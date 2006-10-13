# File::MMagic Makefile.PL 
#
# $Id: Makefile.PL,v 1.22 2006-10-13 16:57:31 opengl2772 Exp $

use ExtUtils::MakeMaker;

WriteMakefile(
	      'NAME' => 'File::MMagic',
	      'VERSION_FROM' => 'MMagic.pm',
	      ($] ge '5.005') ? 
	      ('AUTHOR' => 'NOKUBI Takatsugu <knok@daionet.gr.jp>',
	       'ABSTRACT' => 'Guess file type from contents',
	      ) : (),
);
