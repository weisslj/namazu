#
# Makefile.PL for Search::Namazu
#
# $Id: Makefile.PL,v 1.1 1999-10-29 05:09:52 knok Exp $
#

use ExtUtils::MakeMaker;

WriteMakefile(
	      'NAME' => 'Search::Namazu',
	      'VERSION_FROM' => 'Namazu.pm',
#	      'INC' => '-I/usr/local/namazu/include',
	      'LIBS' => ['-lnamazu']
);
