Summary: Namazu is a search engine intented for easy use
Name: namazu
Version: 1.9.6
Release: 1
Copyright: GPL
Group: Utilities/Text
Requires: perl >= 5.004, kakasi, nkf
Source0: namazu-1.9.6.tar.gz
URL: http://openlab.ring.gr.jp/namazu/
BuildRoot: /var/tmp/namazu

%description
Namazu is a search engine software intended for easy use.  Not
only it works as CGI program for small or medium scale WWW search
engine, but also works as personal use such as search system for
local HDD.  Now, search clients for Mule and Tcl/Tk, JAVA and
Win32 are available.

%changelog
* Tue Oct 12 1999 Ryuji Abe <raeva@t3.rim.or.jp>
- Fixed correctly executables entry at %files.
- Added missing /usr/share/locale entry at %files.
 
* Thu Aug 26 1999 Ryuji Abe <raeva@t3.rim.or.jp>
- Requires perl >= 5.004.
- Delete Packager tag.
- Clean up at %prep.
- Use CFLAGS="$RPM_OPT_FLAGS" at %build.
- Use $RPM_BUILD_ROOT variables at %install.
- Change configure option at %build and %files for new namazu directory structure.

* Sun May 23 1999 Taku Kudoh <taku@TAHOO.ORG>
- 

%prep 
rm -rf $RPM_BUILD_ROOT

%setup

%build
CFLAGS="$RPM_OPT_FLAGS" ./configure --prefix=/usr
make

%install
make prefix=$RPM_BUILD_ROOT/usr install

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-, root, root)

%doc AUTHORS ChangeLog COPYING NEWS README README-ja
/usr/bin/namazu
/usr/bin/*nmz
/usr/bin/mailutime
#/usr/share/emacs/site-lisp/*
/usr/share/locale/*
/usr/share/namazu/*

