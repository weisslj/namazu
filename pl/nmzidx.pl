#
# -*- Perl -*-
# nmzidx.pl - subroutines for accessing Namazu index files (NMZ.*)
#         by furukawa@tcp-ip.or.jp
#

use strict;
use IO::File;
use DirHandle;

package nmzlib;

sub open_db{
    my $par = shift;
    my $ext = shift;
    my $path = $par->{'dir'} . "/NMZ.$ext";
    my $fh;
    if ($par->{'mode'} eq 'w'){
        $fh = new IO::File "$path.$$.tmp", $par->{'mode'};
    }else{
        $fh = new IO::File $path;
    }
    $par->{'dblist'}->{$path} = $fh, binmode $fh if defined $fh;
    return $fh;
}

sub readw{
    my $fh = shift;
    my $ret = '';
    my $c;

    while (read($fh, $c, 1)){
        $ret .= $c;
        last unless 0x80 & ord $c;
    }
    unpack('w', $ret);
}

package nmzfile;

sub new{
    my $class = shift;
    my $self = {};
    bless $self, $class;
    my $par = shift;
    my $ext = shift;
    my $fh = &nmzlib::open_db($par, $ext);
    $self->{'body'} = $fh if defined $fh;
    $self->{'ext'} = $ext;
    $ext .= ($ext =~ /^field/)? '.i': 'i';
    $fh = &nmzlib::open_db($par, $ext);
    $self->{'index'} = $fh if defined $fh;
    $self->{'offset'} = 0;
    if ($par->{'mode'} ne 'w' and defined($self->{'index'})){
        $self->{'size'} = (-s $self->{'index'}) / length(pack('N', 0));
    }
    return $self;
}

sub close{
    my $self = shift;
    $self->{'body'}->close;
    $self->{'index'}->close if defined $self->{'index'};
}

sub seek{
    my $self = shift;
    my $offset = @_? shift: 0;
    my $whence = @_? shift: 0;

    if ($whence == 1){
        $offset += $self->{'offset'};
    }elsif ($whence == 2){
        $offset += $self->{'size'};
    }
    return $offset if $offset == $self->{'offset'};
    return -1 if ($offset < 0 || $offset > $self->{'size'});
    $self->{'offfset'} = $offset;
    $self->{'index'}->seek($offset * length(pack('N', 0)), 0);

    if ($self->{'ext'} ne 'p'){
        if ($offset == $self->{'size'}){
            $self->{'body'}->seek(0, 2);
        }else{
            my $buf;
            $self->{'index'}->read($buf, length pack('N', 0));
            $self->{'body'}->seek(unpack('N', $buf), 0);
        }
    }
    return $offset;
}

sub getline{
    my $self = shift;
    return undef unless defined $self->{'body'};
#    return undef if $self->{'offset'} >= $self->{'size'};
    ++$self->{'offset'};
    return $self->{'body'}->getline;
}

sub getlist{
    my $self = shift;
    return undef unless defined $self->{'body'};
    return undef if $self->{'offset'} >= $self->{'size'};

    if ($self->{'offset'} == $self->{'size'}){
        return ();
    }
    ++$self->{'offset'};

    if ($self->{'ext'} eq 'p'){
        my $buf;
        $self->{'index'}->read($buf, length pack('N', 0));
        return () if $buf eq pack('N', -1);
        $self->{'body'}->seek(unpack('N', $buf), 0);
    }
    $self->{'body'}->read(my $buf, &nmzlib::readw($self->{'body'}));
    return unpack('w*', $buf);
}

sub putline{
    my $self = shift;
    if (@_){
        $self->{'index'}->print(pack('N', $self->{'body'}->tell));
        $self->{'body'}->print(shift);
        ++$self->{'size'};
        ++$self->{'offset'};
    }
}

sub putlist{
    my $self = shift;
    if (@_){
        $self->{'index'}->print(pack('N', $self->{'body'}->tell));
        my $tmp = pack('w*', @_);
        $self->{'body'}->print(pack('w', length $tmp) . $tmp);
    }elsif ($self->{'ext'} eq 'p'){
        $self->{'index'}->print(pack('N', -1));
        ++$self->{'size'};
        ++$self->{'offset'};
    }
}

package nmzfield;
sub new{
    my $class = shift;
    my $self = {};
    bless $self, $class;
    $self->open(@_) if @_;
    return $self;
}

sub open{
    my $self = shift;
    my $par = shift;
    my $ext = shift;

    $self->{$ext} = new nmzfile($par, "field." . $ext);
}

sub open_all{
    my $self = shift;
    my $par = shift;
    my $dh = new DirHandle($par->{'dir'});
    while (defined(my $ent = $dh->read)){
        if ($ent =~ /^NMZ\.field\.([^\.]+)$/){
            $self->{$1} = new nmzfile($par, "field." . $1);
        }
    }
    $dh->close;
}

sub close{
    my $self = shift;
    for my $key (keys %$self){
        $self->{$key}->close;
    }
}


package nmzflist;
sub new{
    my $class = shift;
    my $self = {};
    my $par = shift;
    bless $self, $class;

    $self->{'t'} = &nmzlib::open_db($par, 't');
    $self->{'r'} = &nmzlib::open_db($par, 'r');
    $self->{'field'} = new nmzfield;
    $self->{'field'}->open_all($par);
    $self->{'offset'} = 0;
    $self->{'size'} = (-s $self->{'t'}) / length(pack('N', 0));
    return $self;
}

sub close{
    my $self = shift;
    $self->{'t'}->close;
    $self->{'r'}->close;
    $self->{'field'}->close;
}

sub read{
    my $self = shift;
    my $list = shift;
    %$list = ();

    my $fh = $self->{'t'};
    $fh->read(my $pindex, length pack('N', 0));
    $list->{'t'} = ($pindex eq pack('N', -1))? -1: unpack('N', $pindex);

    $fh = $self->{'r'};
    $list->{'r'} = $fh->getline;
    $list->{'r'} = $fh->getline while (defined($list->{'r'}) && $list->{'r'} =~ /^[\#\r\n]/);

    if (defined $list->{'r'}){
        chomp $list->{'r'};
        my $field = $self->{'field'};
        for my $key (keys %$field){
            $fh = $field->{$key};
            my $line = $fh->getline;
            $line = '' unless defined $line;
            chomp $line;
            $list->{'field'}->{$key} = $line;
        }
        ++$self->{'offset'};
    }
    return $list->{'r'}
}

sub write{
    my $self = shift;
    my $list = shift;

    my $fh = $self->{'t'};
    $fh->print(pack('N', $list->{'t'}));

    $fh = $self->{'r'};
    $fh->print($list->{'r'} . "\n");

    my $field = $self->{'field'};
    for my $key (keys %$field){
        $field->{$key}->putline($list->{'field'}->{$key} . "\n")
    }
    ++$self->{'offset'};
}


package nmzword;
sub new{
    my $class = shift;
    my $par = shift;
    my $self = {};
    bless $self, $class;

    $self->{'i'} = new nmzfile($par, 'i');
    $self->{'w'} = new nmzfile($par, 'w');
    $self->{'offset'} = 0;
    $self->{'size'} = $self->{'i'}->{'size'};
    return $self;
}

sub close{
    my $self = shift;
    $self->{'i'}->close;
    $self->{'w'}->close;
}

sub read{
    my $self = shift;
    my $word = shift;
    my $list = shift;
    %$list = ();

    return unless defined($$word = $self->{'w'}->getline);
    chomp $$word;

    my $key = 0;
    my @tmp = $self->{'i'}->getlist;
    $key += shift @tmp, $list->{$key} = shift @tmp while @tmp;
    ++$self->{'offset'};
    return $$word;
}

sub write{
    my $self = shift;
    my $word = shift;
    my $list = shift;

    if (length $word and scalar keys %$list){
        $self->{'w'}->putline($word . "\n");

        my @tmp = ();
        my $ndx = 0;
        for my $key (sort {$a <=> $b} keys %$list){
            push(@tmp, $key - $ndx);
            push(@tmp, $list->{$key});
            $ndx = $key;
        }
        $self->{'i'}->putlist(@tmp);
        ++$self->{'offset'};
    }
}

package nmzphrase;
sub new{
    my $class = shift;
    my $par = shift;
    my $self = {};
    bless $self, $class;

    $self->{'p'} = new nmzfile($par, 'p');
    $self->{'offset'} = 0;
    $self->{'size'} = 0x10000;
    return $self;
}

sub close{
    my $self = shift;
    $self->{'p'}->close;
}

sub read{
    my $self = shift;
    my $list = shift;

    @$list = ();
    my $ndx = 0;
    my @tmp = $self->{'p'}->getlist;
    push(@$list, $ndx += shift @tmp) while @tmp;
    ++$self->{'offset'};
    return scalar @$list;
}

sub write{
    my $self = shift;
    my $list = shift;

    my $fh_p = $self->{'p'};
    my $fh_pi = $self->{'pi'};
    my @tmp = ();
    my $ndx = 0;

    for my $key (@$list){
        push(@tmp, $key - $ndx);
        $ndx = $key;
    }
    $self->{'p'}->putlist(@tmp);
    ++$self->{'offset'};
}


package nmzidx;
sub new{
    my $class = shift;
    my $dir = @_? shift: '.';
    my $mode = @_? shift: 'r';

    if ($mode ne 'w'){
        return undef if -f "$dir/NMZ.lock";
        my $fh = new IO::File ">$dir/NMZ.lock2";
        $fh->print($$);
        $fh->close;
    }

    my $self = {};
    bless $self, $class;

    $self->{'dir'} = $dir;
    $self->{'mode'} = $mode;
    return $self;
}

sub close{
    my $self = shift;
    unlink ($self->{'dir'} . "/NMZ.lock2") if $self->{'mode'} ne 'w';
}

sub open_field{
    my $self = shift;

    $self->{'field'} = new nmzfield() unless $self->{'field'};
    $self->{'field'}->open($self, @_);
    return $self->{'field'};
}

sub open_flist{
    my $self = shift;
    $self->{'flist'} = new nmzflist($self);
    return $self->{'flist'};
}

sub open_word{
    my $self = shift;
    $self->{'word'} = new nmzword($self);
    return $self->{'word'};
}

sub open_phrase{
    my $self = shift;
    return $self->{'phrase'} = new nmzphrase($self);
}

sub replace_db{
    my $self = shift;
    my $bak = @_? shift : 0;
    my $lock = $self->{'dir'} . "/NMZ.lock";
    my $fh = new IO::File($lock, 'w');
    $fh->close;

    for my $path (keys %{$self->{'dblist'}}){
        $self->{'dblist'}->{$path}->close;
        rename $path, "$path.BAK" if $bak;
        rename "$path.$$.tmp", $path;
    }
    unlink $lock;
}

sub write_status{
    my $self = shift;
    my $in = shift;
    my $key = $self->{'word'}->{'offset'};
    my $file = $self->{'flist'}->{'offset'};
    if ($self->{'mode'} eq 'w'){
        my $fi = &nmzlib::open_db($in, 'status');
        my $fo = &nmzlib::open_db($self, 'status');
        while (defined(my $line = $fi->getline)){
            $line = "file $file\n" if $line =~ /^file /;
            $line = "key $key\n" if $line =~ /^key /;
            $fo->print($line);
        }
        my $dh = new DirHandle($in->{'dir'});
        while (defined(my $ent = $dh->read)){
            if ($ent =~ /^NMZ\.(head\.[^\.]+)$/){
                $fi = &nmzlib::open_db($in, $1);
                $fo = &nmzlib::open_db($self, $1);

                while (defined(my $line = $fi->getline)){
                    $line =~ s/(\<\!-- FILE --\>).*?\1/$1 $file $1/;
                    $line =~ s/(\<\!-- KEY --\>).*?\1/$1 $key $1/;
                    $fo->print($line);
                }
            }
        }
        undef $dh;
    }
}

sub log_open{
    my $self = shift;
    my $tag = shift;
    my $path = $self->{'dir'} . "/NMZ.log";
    my $fh = new IO::File ">>$path";
    if (defined $fh){
        $fh->print("$tag\n") if defined $tag;
        $fh->print("Date: " . localtime($^T) . "\n");
    }
    return $self->{'log'} = $fh;
}

sub log_close{
    my $self = shift;
    if (defined $self->{'log'}){
        $self->{'log'}->print("Perl: $]\n");
        $self->{'log'}->print("System: $^O\n");
        $self->{'log'}->printf("Time: %d sec.\n\n", time - $^T);
        $self->{'log'}->close;
    }
}
1;
