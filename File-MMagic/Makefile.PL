# File::MMagic Makefile.PL 
#
# $Id: Makefile.PL,v 1.18 2000-09-12 08:08:47 knok Exp $

use ExtUtils::MakeMaker;

WriteMakefile(
	      'NAME' => 'File::MMagic',
	      'VERSION_FROM' => 'MMagic.pm',
	      ($] ge '5.005') ? 
	      ('AUTHOR' => 'NOKUBI Takatsugu <knok@daionet.gr.jp>',
	       'ABSTRACT' => 'Guess file type from contents',
	      ) : (),
);
