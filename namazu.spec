Summary: Nmazu is a search engine intented for easy use
Name: namazu
Version: 1.4.0.0-beta-8
Release: 1
Copyright: GPL
Group: Utilities/Text
Requires: perl >= 5.0, kakasi, nkf
Source0: namazu-1.4.0.0-beta-8.tar.gz
URL: http://openlab.ring.gr.jp/namazu/
Packager: Taku Kudoh <taku@TAHOO.ORG>
BuildRoot: /var/tmp/namazu

%description
Namazu is a search engine software intended for easy use.  Not
only it works as CGI program for small or medium scale WWW search
engine, but also works as personal use such as search system for
local HDD.  Now, search clients for Mule and Tcl/Tk, JAVA and
Win32 are available.

%changelog 
* Sun May 23 1999 Taku Kudoh <taku@TAHOO.ORG>
- 

%prep 
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/bin
mkdir -p $RPM_BUILD_ROOT/usr/lib/namazu

%setup

%build
./configure --bindir=$RPM_BUILD_ROOT/usr/bin --libdir=$RPM_BUILD_ROOT/usr/lib/namazu
make

%install
make install

%clean
rm -rf $RPM_BUILD_ROOT

%files
%attr(-,root,root) /usr/bin/*
%attr(-,root,root) /usr/lib/namazu/*
