#
# -*- Perl -*-
# $Id: codeconv.pl,v 1.2 1999-10-11 04:25:18 satoru Exp $
# Copyright (C) 1997-1999 Satoru Takabayashi  All rights reserved.
#     This is free software with ABSOLUTELY NO WARRANTY.
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either versions 2, or (at your option)
#  any later version.
# 
#  This program is distributed in the hope that it will be useful
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
#  02111-1307, USA
#
#  This file must be encoded in EUC-JP encoding
#
# package for code conversion
#
#   imported from  Rei FURUKAWA <furukawa@dkv.yamaha.co.jp> san's pnamazu.
#   [1998-09-24]

package codeconv;
use strict;

my @ktoe = (0xA3, 0xD6, 0xD7, 0xA2, 0xA6, 0xF2, 0xA1, 0xA3,
	     0xA5, 0xA7, 0xA9, 0xE3, 0xE5, 0xE7, 0xC3, 0xBC,
	     0xA2, 0xA4, 0xA6, 0xA8, 0xAA, 0xAB, 0xAD, 0xAF,
	     0xB1, 0xB3, 0xB5, 0xB7, 0xB9, 0xBB, 0xBD, 0xBF,
	     0xC1, 0xC4, 0xC6, 0xC8, 0xCA, 0xCB, 0xCC, 0xCD,
	     0xCE, 0xCF, 0xD2, 0xD5, 0xD8, 0xDB, 0xDE, 0xDF,
	     0xE0, 0xE1, 0xE2, 0xE4, 0xE6, 0xE8, 0xE9, 0xEA,
	     0xEB, 0xEC, 0xED, 0xEF, 0xF3, 0xAB, 0xAC, );

# convert JIS X0201 KANA characters to JIS X0208 KANA
sub ktoe {
    my ($c1, $c2) = @_;
    $c1 = ord($c1) & 0x7f;
    my($hi) = ($c1 <= 0x25 || $c1 == 0x30 || 0x5e <= $c1)? "\xa1": "\xa5";
    $c1 -= 0x21;
    my($lo) = $ktoe[$c1];
    if ($c2){
        if ($c1 == 5){
            $lo = 0xdd;
        }else{
            $lo++;
            $lo++ if ord($c2) & 0x7f == 0x5f;
        }
    }
    return $hi . chr($lo);
}

# convert Shift_JIS to EUC-JP
sub stoe ($$) {
    my($c1, $c2) = @_;

    $c1 = ord($c1);
    $c2 = ord($c2);
    $c1 += ($c1 - 0x60) & 0x7f;
    if ($c2 < 0x9f){
        $c1--;
        $c2 += ($c2 < 0x7f) + 0x60;
    }else{
        $c2 += 2;
    }
    return chr($c1) . chr($c2);
}

sub shiftjis_to_eucjp ($){
    my ($str) = @_;
    $str =~ s/([\x81-\x9f\xe0-\xfa])(.)|([\xa1-\xdf])([\xde\xdf]?)/($3? ktoe($3, $4): stoe($1, $2))/ge;
    return $str;
}

sub etos($$) {
    my($c1, $c2) = @_;

    $c1 = ord($c1) & 0x7f;
    $c2 = ord($c2) & 0x7f;

    if ($c1 & 1) {
        $c1 = ($c1 >> 1) + 0x71;
        $c2 += 0x1f;
        $c2++ if $c2 >= 0x7f;
    } else {
        $c1 = ($c1 >> 1) + 0x70;
        $c2 += 0x7e;
    }
    $c1 += 0x40 if $c1 > 0x9f;

    return chr($c1) . chr($c2);
}

sub eucjp_to_shiftjis ($) {
    my ($str) = @_;
    $str =~ s/([\xa1-\xfe])([\xa1-\xfe])/etos($1, $2)/ge;
    return $str;
}

sub toeuc ($) {
    my ($contref) = @_;

    if ($conf::LANGUAGE eq "ja" && ! $var::USE_NKF_MODULE) {
	my $nkftmp = util::tmpnam("NMZ.nkf");
	{
	    my $nh = util::efopen("|$conf::NKF -emXZ1 > $nkftmp");
	    print $nh $$contref;
	}
	{
	    my $nh = util::efopen("< $nkftmp");
	    $$contref = util::readfile($nh);
	}
	unlink($nkftmp);
    } else {
	$$contref = NKF::nkf("-emXZ1", $$contref);
    }
}
1;
