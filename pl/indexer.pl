#
# -*- Perl -*-
# indexer.pl - class for indexing
#
# $Id: indexer.pl,v 1.6 2009-01-29 02:47:53 opengl2772 Exp $
#
# Copyright (C) 2002-2009 Namazu Project All rights reversed.
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either versions 2, or (at your option)
#  any later version.
# 
#  This program is distributed in the hope that it will be useful
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
#  02111-1307, USA


package mknmz::indexer;

my $allow_c = ""; 

sub new {
    my $self = {};
    my $proto = shift @_;
    my $class = ref($proto) || $proto;
    bless($self, $class);

    $self->init(@_);
    return $self;
}

sub init {
    my $self = shift @_;
    $self->{'_keyindex'} = {};
    $self->{'_content'} = shift @_;
    $self->{'_word_leng_max'} = shift @_;
    $self->{'_nosymbol'} = shift @_;
    $self->{'_hook_word'} = undef;
    $allow_c = "[^\x80-\xefa-z_0-9]";
}

sub get_keyindex {
    my $self = shift @_;
    return $self->{'_keyindex'};
}

sub word_hook {
    my $self = shift @_;
    $self->{'_hook_word'} = shift @_;
}

sub noedgesymbol {
    my $self = shift @_;
    $self->word_hook(sub {$_[0] =~ s/^$allow_c*(.*?)$allow_c*$/$1/g; $_[0];});
}

sub count_words {
    my $self = shift @_;

    my $contref = $self->{'_content'};

    my $part1 = "";
    my $part2 = "";
    my $part3 = "";
    my $part3_uniqword = "";

    if ($$contref =~ /\x7f/) {
        $part1 = substr $$contref, 0, index($$contref, "\x7f");
        $part2 = substr $$contref, index($$contref, "\x7f"),
            (rindex($$contref, "\x7f") - index($$contref, "\x7f") +1);
#       $part1 = $PREMATCH;  # $& and friends are not efficient
#       $part2 = $MATCH . $POSTMATCH;
        if ($$contref =~ /\x7f([^\x7f]+)$/) {
            $part3 = $1;
            $part3 = '' if $part3 =~ /^\s+$/;
        }
    } else {
        $part1 = $$contref;
        $part2 = "";
    }

    if ($part3 ne '') {
        $part3_uniqword = substr $part3, (rindex($part3, "\n\.\n") +3);
        if ($part3_uniqword ne "") {
            substr($$contref, rindex($$contref, "\n\.\n")) = "";
        }
    }

    # do scoring
    my $word_count = $self->{'_keyindex'};
    $part2 =~ s!\x7f *(\d+) *\x7f([^\x7f]*)\x7f */ *\d+ *\x7f!
        $self->_wordcount_sub($2, $1, $word_count)!ge;
    $self->_wordcount_sub($part1, 1, $word_count);
    $self->_wordcount_sub($part3_uniqword, 1, $word_count) if $part3_uniqword;
}

sub _wordcount_sub {
    my $self = shift @_;
    my ($text, $weight, $word_count) = @_;

    # Remove all symbols when -K option is specified.
    $text =~ tr/\xa1-\xfea-z0-9/   /c if $self->{'_nosymbol'};

    # Count frequencies of words in a current document.
    # Handle symbols as follows.
    #
    # tcp/ip      ->  tcp/ip,     tcp,      ip
    # (tcp/ip)    ->  (tcp/ip),   tcp/ip,   tcp, ip
    # ((tcpi/ip)) ->  ((tcp/ip)), (tcp/ip), tcp
    #
    # Don't do processing for nested symbols.
    # NOTE: When -K is specified, all symbols are already removed.

    my @words = split /\s+/, $text;
    for my $word (@words) {
        next if ($word eq "" || length($word) > $self->{'_word_leng_max'});
        if (defined $self->{'_hook_word'}) {
            $word = &{$self->{'_hook_word'}}($word);
        }
        $word_count->{$word} = 0 unless defined($word_count->{$word});
        $word_count->{$word} += $weight;
        unless ($self->{'_nosymbol'}) {
            $self->_splitsymbol($word, $weight);
        }
    }
    return "";
}

sub _splitsymbol {
    my $self = shift @_;
    my $word = shift @_;
    my $weight = shift @_;
    my $word_count = $self->{'_keyindex'};
    
    if ($word =~ /^$allow_c(.+)$allow_c$/) {
        $word_count->{$1} = 0 unless defined($word_count->{$1});
        $word_count->{$1} += $weight;
        return unless $1 =~ /$allow_c/;
    } elsif ($word =~ /^$allow_c(.+)/) {
        $word_count->{$1} = 0 unless defined($word_count->{$1});
        $word_count->{$1} += $weight;
        return unless $1 =~ /$allow_c/;
    } elsif ($word =~ /(.+)$allow_c$/) {
        $word_count->{$1} = 0 unless defined($word_count->{$1});
        $word_count->{$1} += $weight;
        return unless $1 =~ /$allow_c/;
    }
    my @words_ = split(/$allow_c+/, $word)
        if $word =~ /$allow_c/;
    for my $tmp (@words_) {
        next if $tmp eq "";
        $word_count->{$tmp} = 0 unless defined($word_count->{$tmp});
        $word_count->{$tmp} += $weight;
    }
}

1;
