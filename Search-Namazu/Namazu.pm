#
# Search::Namazu.pm
#
# Copyright (C) 1999 NOKUBI Takatsugu All rights reserved.
# This is free software with ABSOLUTELY NO WARRANTY.
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
# 02111-1307, USA
#
# $Id: Namazu.pm,v 1.1 1999-10-29 05:09:52 knok Exp $
#

package Search::Namazu;

=head1 NAME

Search::Namazu - Namazu library module for perl

=head1 SYNOPSIS

  use Search::Namazu;

  $res = Search::Namazu::Search(index => '/usr/local/namazu/index',
				query => 'foo');

=head1 DESCRIPTION

This module is an interface for Namazu library. Namazu is an implement
of full text retrieval search system.
<http://www.namazu.org/>

=head1 COPYRIGHT

Copyright 1999 NOKUBI Takatsugu All rights reserved.
This is free software with ABSOLUTELY NO WARRANTY.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA

=cut

require Exporter;
requite DynaLoader;

@ISA = qw(Exporter DynaLoader);

@EXPORT_OK = qw();
# %EXPORT_TAGS = (all => [qw()]);

$VERSION = '0.01';

bootstrap Search::Namazu $VERSION;

use Carp;

sub Search {
    my %args = @_;
    my $index = $args{'index'};
    my $sortmeth = $args{'sortMethod'};
    my $sortord = $args{'sortOrder'};
    my $config = $args{'config'};
    my $lang = $args{'lang'};
    my $query = $args{'query'};

# initialize libnamazu... (not implemented yet.)

    if (defined $config) {
	load_conf($config);
    }

    if (! defined $index) {
	return NMZ_NOT_SPECIFIED_INDEX;
    }
    my @indices;
    if (ref($index)) {
	for my $idx (@$index) {
	    push @indices, $idx;
	}
    } else {
	@indices = ($index);
    }
    for my $idx (@indices) {
	if (add_index($idx) < 0) {
	    return NMZ_ERR_INDEX;
	}
    }

    
}

package Search::Namazu::HList;

sub new {
    my $self = {};
    $self->{'score'} = -1;
    $self->{'uri'} = '';
    $self->{'date'} = 0;
    $self->{'rank'} = -1;
    bless $self;
    return $self;
}

sub set {
    my $self = shift;
    my $score = shift;
    my $uri = shift;
    my $date = shift;
    my $rank = shift;
    $self->{score} = $score;
    $self->{uri} = $uri;
    $self->{date} = $date;
    $self->{rank} = $rank;
}

sub score {
    my $self = shift;
    if (@_) {
	my $score = shift;
	$self->{'score'} = $score;
    }
    $self->{'score'};
}

sub uri {
    my $self = shift;
    if (@_) {
	my $uri = shift;
	$self->{'uri'} = $uri;
    }
    $self->{'uri'};
}

sub date {
    my $self = shift;
    if (@_) {
	my $date = shift;
	$self->{'date'} = $date;
    }
    $self->{'date'};
}

sub rank {
    my $self = shift;
    if (@_) {
	my $rank = shift;
	$self->{'rank'} = $rank;
    }
    $self->{'rank'};
}

1;
__END__

