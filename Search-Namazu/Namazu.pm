#
# Search::Namazu.pm
#
# Copyright (C) 1999, 2000 NOKUBI Takatsugu All rights reserved.
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
# $Id: Namazu.pm,v 1.12 2001-01-17 09:45:52 knok Exp $
#

package Search::Namazu;

=head1 NAME

Search::Namazu - Namazu library module for perl

=head1 SYNOPSIS

  use Search::Namazu;

  @hlists = Search::Namazu::Search(index => '/usr/local/namazu/index',
				query => 'foo');

  foreach my $hlist (@hlists) {
      print ($hlist->score, $hlist->uri, $hlist->date, $hlist->rank);
  }

=head1 DESCRIPTION

This module is an interface for Namazu library. Namazu is an implement
of full text retrieval search system.
<http://www.namazu.org/>

=head1 COPYRIGHT

Copyright 1999, 2000 NOKUBI Takatsugu All rights reserved.
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
require DynaLoader;

@ISA = qw(Exporter DynaLoader);

@EXPORT = qw(NMZ_SORTBYDATE NMZ_SORTBYSCORE NMZ_SORTBYFIELD
NMZ_ASCENDSORT NMZ_DESCENDSORT
NMZ_NOT_SPECIFIED_INDEX NMZ_ERR_INDEX);
# %EXPORT_TAGS = (all => [qw()]);

$VERSION = '0.12';

bootstrap Search::Namazu $VERSION;

use Carp;

sub NMZ_SORTBYDATE { return 1; }
sub NMZ_SORTBYSCORE { return 2; }
sub NMZ_SORTBYFIELD { return 3; }
sub NMZ_ASCENDSORT { return 16; }
sub NMZ_DESCENDSORT { return 32; }

sub NMZ_NOT_SPECIFIED_INDEX { return -1; }
sub NMZ_ERR_INDEX { return -2; }

sub Search {
    my %args = @_;
    my $index = $args{'index'};
    my $sortmeth = $args{'sortMethod'};
    my $sortord = $args{'sortOrder'};
    my $lang = $args{'lang'};
    my $query = $args{'query'};

# initialize

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
	if (nmz_addindex($idx) < 0) {
	    return NMZ_ERR_INDEX;
	}
    }

# set paramater

    if (!defined $sortmeth) {
	nmz_sortbydate();
    } elsif ( $sortmeth == NMZ_SORTBYDATE) {
	nmz_sortbydate();
    } elsif ($sortmeth == NMZ_SORTBYSCORE) {
	nmz_sortbyscore();
    } elsif ($sortmeth == NMZ_SORTBYFIELD) {
	nmz_sortbyfield();
    } else {
	nmz_sortbydate();
    }

    if (!defined $sortord) {
	nmz_ascendingsort();
    } elsif ($sortord == NMZ_DESCENDSORT) {
	nmz_descendingsort();
    } elsif ($sortord == NMZ_ASCENDSORT) {
	nmz_ascendingsort();
    } else {
	nmz_descendingsort();
    }

    if (defined $config) {
	if (nmz_setconfig($config)) {
	  return ();
      }
    }

    if (defined $lang) {
	nmz_setlang($lang);
    }

# query and get hlist

    if (! defined $query) {
	return ();
    }

# create Search::Namazu::Result object

    my @hlists = call_search_main($query);
    pop @hlists;

# return objects
    return @hlists;
}

package Search::Namazu::Result;

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

