# File::MMagic Makefile.PL 
#
# $Id: Makefile.PL,v 1.10 2000-02-28 06:13:19 knok Exp $

use ExtUtils::MakeMaker;

WriteMakefile(
	      'NAME' => 'File::MMagic',
	      'VERSION_FROM' => 'MMagic.pm',
	      ($] ge '5.005') ? 
	      ('AUTHOR' => 'NOKUBI Takatsugu <knok@daionet.gr.jp>',
	       'ABSTRACT' => 'Guess file type from contents',
	      ) : (),
);
