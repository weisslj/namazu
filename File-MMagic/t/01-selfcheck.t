# perl-test
# $Id: 01-selfcheck.t,v 1.4 2006-10-13 16:57:31 opengl2772 Exp $

use strict;
use Test;

BEGIN { plan tests => 1 };

use File::MMagic;

my $magic = File::MMagic->new();
my $ret = $magic->checktype_filename('t/01-selfcheck.t');
ok($ret eq 'text/plain');
