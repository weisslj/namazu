#
# Makefile.PL for Search::Namazu
#
# $Id: Makefile.PL,v 1.2 1999-11-09 08:34:36 knok Exp $
#

use ExtUtils::MakeMaker;

WriteMakefile(
	      'NAME' => 'Search::Namazu',
	      'VERSION_FROM' => 'Namazu.pm',
	      'INC' => '-I../nmz',
	      'LIBS' => ['-L../nmz/.libs -lnmz']
);
