#
# -*- Perl -*-
# $Id: codeconv.pl,v 1.16 2003-08-01 01:45:10 makoto Exp $
# Copyright (C) 1997-1999 Satoru Takabayashi All rights reserved.
# Copyright (C) 2000 Namazu Project All rights reserved.
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
sub ktoe ($$) {
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
            $lo++ if (ord($c2) == 0xdf);
        }
    }
    return $hi . chr($lo);
}

sub eucjp_han2zen_kana ($) {
    my ($str) = @_;
    if (util::islang("ja")) {
	$str =~ s/\x8e([\xa1-\xdf])(\x8e([\xde\xdf]))?/&ktoe($1,$3)/geo;
    }
    $str;
}

# convert Shift_JIS to EUC-JP
sub stoe ($) {
    my ($c1, $c2) = unpack('CC', shift);

    if (0xa1 <= $c1 && $c1 <= 0xdf) {
	$c2 = $c1;
	$c1 = 0x8e;
    } elsif (0x9f <= $c2) {
	$c1 = $c1 * 2 - ($c1 >= 0xe0 ? 0xe0 : 0x60);
	$c2 += 2;
    } else {
	$c1 = $c1 * 2 - ($c1 >= 0xe0 ? 0xe1 : 0x61);
	$c2 += 0x60 + ($c2 < 0x7f);
    }
    return chr($c1) . chr($c2);
}

sub shiftjis_to_eucjp ($) {
    my ($str) = @_;
    if (util::islang("ja")) {
	$str =~ s/([\x81-\x9f\xe0-\xfc][\x40-\x7e\x80-\xfc]|[\xa1-\xdf])/&stoe($1)/geo;
    }
    return $str;
}

# convert EUC-JP to Shift_JIS
sub etos ($) {
    my ($c1, $c2) = unpack('CC', shift);

    if ($c1 == 0x8e) {      # JIS X 0201 KATAKANA
	return chr($c2);
    } elsif ($c1 == 0x8f) { # JIS X 0212 HOJO KANJI
	return "\x81\xac";
    } elsif ($c1 % 2) {
	$c1 = ($c1>>1) + ($c1 < 0xdf ? 0x31 : 0x71);
	$c2 -= 0x60 + ($c2 < 0xe0);
    } else {
	$c1 = ($c1>>1) + ($c1 < 0xdf ? 0x30 : 0x70);
	$c2 -= 2;
    }
    return chr($c1) . chr($c2);
}

sub eucjp_to_shiftjis ($) {
    my ($str) = @_;
    if (util::islang("ja")) {
	$str =~ s/([\xa1-\xfe][\xa1-\xfe]|\x8e[\xa1-\xdf]|\x8f[\xa1-\xfe][\xa1-\xfe])/&etos($1)/ge;
    }
    return $str;
}

# Remove a garbage EUC-JP 1st charactor at the end.
sub chomp_eucjp ($) {
    my ($str) = @_;
    if ($str =~ /\x8f$/ or $str =~ tr/\x8e\xa1-\xfe// % 2) {
	chop($str);
	chop($str) if ($str =~ /\x8f$/);
    }
    return $str;
}

# convert to EUC-JP by using NKF
sub toeuc ($) {
    my ($contref, $opt) = @_;

    if (util::islang("ja")) {
	my $nkf_opt = "-emXZ1";

	if ($var::USE_NKF_MODULE) {
	    $$contref = NKF::nkf($nkf_opt, $$contref); # namazu-devel-ja #3152 -> backed out, #3181
	} else {
	    my $nkftmp = util::tmpnam("NMZ.nkf");
	    {
		my $nh = util::efopen("|$conf::NKF $nkf_opt > $nkftmp");
		print $nh $$contref;
	    }
	    {
		my $nh = util::efopen("< $nkftmp");
		$$contref = util::readfile($nh);
	    }
	    unlink($nkftmp);
	}
    }
}

sub eucjp_zen2han_ascii ($) {
    my ($str) = @_;
    if (util::islang("ja")) {         
        $str =~ s/\xa3([\xb0-\xb9\xc1-\xda\xe1-\xfa])/pack("C",unpack("C",$1)-0x80)/ge;
    }
    $str;
}
 
sub normalize_eucjp ($) {
    my ($contref) = @_;
    if (util::islang("ja")) {
        $$contref = codeconv::eucjp_han2zen_kana($$contref);
        $$contref = codeconv::eucjp_zen2han_ascii($$contref);
    }
    $contref;
}

1;
