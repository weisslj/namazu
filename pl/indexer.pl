#
# -*- Perl -*-
# indexer.pl - class for indexing
#
# $Id: indexer.pl,v 1.1 2002-11-14 10:13:51 knok Exp $
#
# Copyright (C) 2002 Namazu Project All rights reversed.
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

sub new {
    my $self = {};
    my $proto = shift @_;
    my $class = ref($proto) || $proto;
    bless($self);

    $self->init(@_);
    return $self;
}

sub init {
    my $self = shift @_;
    $self->{'KeyIndex'} = {};
    $self->{'content'} = shift @_;
    $self->{'conf::WORD_LENG_MAX'} = shift @_;
    $self->{'conf::nosymbol'} = shift @_;
    $self->{'hook::word'} = undef;
}

sub get_keyindex {
    my $self = shift @_;
    return $self->{'KeyIndex'};
}

sub word_hook {
    my $self = shift @_;
    $self->{'hook::word'} = shift @_;
}

sub noedgesymbol {
    my $self = shift @_;
    $self->word_hook(sub {$_[0] =~ s/^[^\xa1-\xfea-z_0-9]*(.*?)[^\xa1-\xfea-z_0-9]*$/$1/g; $_[0];});
}

sub count_words {
    my $self = shift @_;

    my $contref = $self->{'content'};

    my $part1 = "";
    my $part2 = "";
    if ($$contref =~ /\x7f/) {
        $part1 = substr $$contref, 0, index($$contref, "\x7f");
        $part2 = substr $$contref, index($$contref, "\x7f");
#       $part1 = $PREMATCH;  # $& and friends are not efficient
#       $part2 = $MATCH . $POSTMATCH;
    } else {
        $part1 = $$contref;
        $part2 = "";
    }

    # do scoring
    my $word_count = $self->{'KeyIndex'};
    $part2 =~ s!\x7f *(\d+) *\x7f([^\x7f]*)\x7f */ *\d+ *\x7f!
        $self->wordcount_sub($2, $1, $word_count)!ge;
    $self->wordcount_sub($part1, 1, $word_count);
}

sub wordcount_sub {
    my $self = shift @_;
    my ($text, $weight, $word_count) = @_;

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
        next if ($word eq "" || length($word) > $self->{'conf::WORD_LENG_MAX'});
	if (defined $self->{'hook::word'}) {
	    $word = &{$self->{'hook::word'}}($word);
	}
        $word_count->{$word} = 0 unless defined($word_count->{$word});
        $word_count->{$word} += $weight;
        unless ($self->{'option::nosymbol'}) {
	    $self->splitsymbol($word, $weight);
        }
    }
    return "";
}

sub splitsymbol {
    my $self = shift @_;
    my $word = shift @_;
    my $weight = shift @_;
    my $word_count = $self->{'KeyIndex'};
    if ($word =~ /^[^\xa1-\xfea-z_0-9](.+)[^\xa1-\xfea-z_0-9]$/) {
	$word_count->{$1} = 0 unless defined($word_count->{$1});
	$word_count->{$1} += $weight;
	return unless $1 =~ /[^\xa1-\xfea-z_0-9]/;
    } elsif ($word =~ /^[^\xa1-\xfea-z_0-9](.+)/) {
	$word_count->{$1} = 0 unless defined($word_count->{$1});
	$word_count->{$1} += $weight;
	return unless $1 =~ /[^\xa1-\xfea-z_0-9]/;
    } elsif ($word =~ /(.+)[^\xa1-\xfea-z_0-9]$/) {
	$word_count->{$1} = 0 unless defined($word_count->{$1});
	$word_count->{$1} += $weight;
	return unless $1 =~ /[^\xa1-\xfea-z_0-9]/;
    }
    my @words_ = split(/[^\xa1-\xfea-z_0-9]+/, $word)
      if $word =~ /[^\xa1-\xfea-z_0-9]/;
    for my $tmp (@words_) {
	next if $tmp eq "";
	$word_count->{$tmp} = 0 unless defined($word_count->{$tmp});
	$word_count->{$tmp} += $weight;
    }
}

1;
