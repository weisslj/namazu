# perl-test
# $Id: 02-extmagic.t,v 1.3 2005-07-22 04:23:50 knok Exp $

use strict;
use Test;

BEGIN { plan tests => 1 };

use File::MMagic;

my $magic = File::MMagic->new();
$magic->addMagicEntry("0\tstring\t#\\ perl-test\tapplication/x-perl-test");
my $ret = $magic->checktype_filename('t/02-extmagic.t');
ok($ret eq 'application/x-perl-test');
