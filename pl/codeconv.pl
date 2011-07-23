#
# -*- Perl -*-
# $Id: codeconv.pl,v 1.38 2011-07-23 14:29:46 usu Exp $
# Copyright (C) 1997-1999 Satoru Takabayashi All rights reserved.
# Copyright (C) 2000-2009 Namazu Project All rights reserved.
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

my @utf8_h2z = (
    0x82BF,
    0x8381, 0x8384, 0x8386, 0x8388, 0x838A, 0x838B, 0x838C, 0x838D,
    0x838E, 0x838F, 0x8392, 0x8395, 0x8398, 0x839B, 0x839E, 0x839F,
    0x83A0, 0x83A1, 0x83A2, 0x83A4, 0x83A6, 0x83A8, 0x83A9, 0x83AA,
    0x83AB, 0x83AC, 0x83AD, 0x83AF, 0x83B3, 0x829B, 0x829C,
    0x8080,  # $utf8_h2z[0x20] is not chosen. Filled a gap.
    0x8082, 0x808C, 0x808D, 0x8081, 0x83BB, 0x83B2, 0x82A1, 0x82A3,
    0x82A5, 0x82A7, 0x82A9, 0x83A3, 0x83A5, 0x83A7, 0x8383, 0x83BC,
    0x82A2, 0x82A4, 0x82A6, 0x82A8, 0x82AA, 0x82AB, 0x82AD, 0x82AF,
    0x82B1, 0x82B3, 0x82B5, 0x82B7, 0x82B9, 0x82BB, 0x82BD, );

# convert JIS X0201 KANA characters to JIS X0208 KANA
sub utf8han2zen ($$) {
    my ($c1, $c2) = @_;
    if (util::islang("ja")) {
        my $hi = "\xE3";
        my $dakuten = "";
        $c1 = unpack("n", $c1) & 0x7F;
        my $mid_low = $utf8_h2z[$c1];
        if ($c2) {
            if (unpack("n", $c2) == 0xBE9E) { # dakuten
                if ($c1 == 0) {
                    $mid_low = 0x8380; # "ta" to "da"
                } elsif ($c1<=4 || ($c1>=0x0E && $c1<=0x0E) || $c1>=36) {
                    $mid_low++;
                } else {
                    $dakuten = "\xE3\x82\x9B"; # isolated dakuten
                }
            } elsif (unpack("n", $c2) == 0xBE9F) { # handakuten
                if ($c1>=0x0A && $c1<=0x0E) {
                    $mid_low += 2;
                } else {
                    $dakuten = "\xE3\x82\x9C"; # isolated handakuten
                }
            }
        }
        return $hi . pack("n",$mid_low) . $dakuten;
    }
}

sub utf8_han2zen_kana ($) {
    my ($strref) = @_;
    $$strref =~ s/\xef(\xbd[\xa1-\xbf]|\xbe[\x80-\x9f])
        (\xef(\xbe[\x9e\x9f]))?
        /utf8han2zen($1,$3)/geox;
}

# Remove a garbage utf-8 charactor at the end.
sub chomp_multibytechar ($) {
    my ($str) = @_;
    {
        # Remove a deficient utf-8 charactor at the end.
        # ex. [C0-DF]$, [E0-EF][80-BF]$, [E0-EF]$
        $str =~ s/[\xC0-\xDF]$//;
        $str =~ s/[\xE0-\xEF][\x80-\xBF]?$//;
    }
    return $str;
}

my %nkfopt_f  = ( '7BIT-JIS'    => 'J',    'ISO-2022-JP' => 'J',
                  'UTF-8'       => 'W',    'UTF8'        => 'W',
                  'EUC-JP'      => 'E',
                  'SHIFT_JIS'   => 'S',    'SHIFT-JIS'   => 'S',
                  'SHIFTJIS'    => 'S',    'CP932'       => 'S',
                  'UTF16-BE'    => 'W16B', 'UTF16-LE'    => 'W16L',
                  'UNKNOWN'     => '' );
my %nkfopt_t  = ( '7BIT-JIS'    => 'j',    'ISO-2022-JP' => 'j',
                  'UTF-8'       => 'w',    'UTF8'        => 'w',
                  'EUC-JP'      => 'e',
                  'SHIFT_JIS'   => 's',    'SHIFT-JIS'   => 's',
                  'SHIFTJIS'    => 's',    'CP932'       => 's');

sub from_to_by_nkf_m ($$$) {
    my ($contref, $code_f, $code_t) = @_;

    $code_f = uc($code_f);
    $code_t = uc($code_t);
    my $tmp = $nkfopt_f{$code_f};
    if (!$tmp) {
        $nkfopt_f{$code_f} = '';
    }
    my $nkf_opt = "-". $nkfopt_f{$code_f} . $nkfopt_t{$code_t} . "mXZ1";
    $$contref = NKF::nkf($nkf_opt, $$contref);
}

sub from_to_by_nkf ($$$) {
    my ($contref, $code_f, $code_t) = @_;

    $code_f = uc($code_f);
    $code_t = uc($code_t);
    my $tmp = $nkfopt_f{$code_f};
    if (!$tmp) {
        $nkfopt_f{$code_f} = '';
    }
    my $nkf_opt = "-". $nkfopt_f{$code_f} . $nkfopt_t{$code_t} . "mXZ1";
    my $nkftmp = util::tmpnam("NMZ.nkf");
    {
        my $nh = util::efopen("|$conf::NKF $nkf_opt > $nkftmp");
        print $nh $$contref;
        util::fclose($nh);
    }
    {
        my $nh = util::efopen("< $nkftmp");
        $$contref = util::readfile($nh);
        util::fclose($nh);
    }
    unlink($nkftmp);
}

sub decode_mime_header ($$) {
    my ($contref, $code_t) = @_;
    my $m_head = '=\?(?:ISO-2022-JP|iso-2022-jp|ISO-8859-1|iso-8859-1)\?[BbQq]\?[A-Za-z0-9\+\/]+=*\?=';
    $$contref =~ s/($m_head)/de_mime_header_by_encode($1,$code_t)/ge;
}

sub de_mime_header_by_encode ($$) {
    my ($str, $code_t) = @_;
    $str = Encode::decode('MIME-Header', $str);
    _utf8_off($str);
    Encode::from_to($str, 'utf8', $code_t);
    return $str;
}

sub decide_encode ($$) {
    my ($enc, $contref) = @_;
    my @enc = split(/ /, $enc);
    my $maxct = 1024;
    my $minres = $maxct;
    my $testdata = substr($$contref, 0, $maxct);

    my $encode = "ascii";
    while(my $enc = shift(@enc)) {
        next if ($enc eq 'or');
        if ($enc eq 'shiftjis') {
            my $tmp = $testdata;
            my $ct = 0;
            $tmp =~ s/([\x00-\x7f]|[\x81-\x9f\xe0-\xfc][\x40-\x7e\x80-\xfc])/$ct++; '';/egs;
            my $res = length($tmp);
            if ($res < $minres || ($res == $minres && $ct < $maxct)) {
                $maxct = $ct;
                $minres = $res;
                $encode = 'shiftjis';
            }
        } elsif ($enc eq 'euc-jp') {
            my $tmp = $testdata;
            my $ct = 0;
            $tmp =~ s/([\x00-\x7f]|[\x8e\xa1-\xfe][\xa1-\xfe]|\x8f[\xa1-\xfe][\xa1-\xfe])/$ct++; '';/egs;
            my $res = length($tmp);
            if ($res < $minres || ($res == $minres && $ct < $maxct)) {
                $maxct = $ct;
                $minres = $res;
                $encode = 'euc-jp';
            }
        } elsif ($enc =~ /7bit-jis/) {
            my $tmp = $testdata;
            my $ct = ($tmp =~ s/(\x1b\$B([\x21-\x7e]{2})+\x1b\(B)//gs);
            $ct +=  ($tmp =~ s/([\x0d\x0a\x20-\x7e])//gs);
            my $res = length($tmp);
            if ($res < $minres || ($res == $minres && $ct < $maxct)) {
                $maxct = $ct;
                $minres = $res;
                $encode = '7bit-jis';
            }
        } elsif ($enc =~ /utf-?8/) {
            my $tmp = $testdata;
            my $ct = ($tmp =~ s/([\x00-\x7f]|[\xc0-\xdf][\x80-\xbf]|[\xe0-\xef][\x80-\xbf]{2}|[\xf0-\xf7][\x80-\xbf]{3})//gs);
            my $res = length($tmp);
            if ($ct > 0 && (($res < $minres || ($res == $minres && $ct < $maxct)))) {
                $maxct = $ct;
                $minres = $res;
                $encode = 'utf8';
            }
        }
    }
    return $encode;
}

sub encode_from_to ($$$) {
    my ($contref, $code_f, $code_t) = @_;
    return if ($code_f eq $code_t);
    if ($conf::NKF eq 'module_iconv') {
        # FIXME:
    } elsif ($conf::NKF eq 'module_encode') {
        if ($code_f eq 'unknown') {
            #$Encode::Guess::DEBUG=1;
            my $enc = Encode::Guess::guess_encoding($$contref);
            if (ref $enc) {
                $code_f = $enc->name;
                util::dprint("Encode guessed : $code_f\n");
            } elsif (!defined $enc) {
                $$contref = "";
                util::dprint("Encode::Guess couldn't find coding name");
                return "Encode::Guess couldn't find encoding";
            } else {
                util::dprint("Encode guessed : $enc\n");
                $code_f = decide_encode($enc, $contref);
                util::dprint("Encode decided : $code_f\n");
            }
        }
        Encode::from_to($$contref, $code_f ,$code_t);
        util::dprint("Encode from $code_f to $code_t via encode_module");
    } else {
        # FIXME: nkf is a japanese limitation.
        # In the future, code conversion functions will be replaced with
        # Text::Iconv.

        # FIXME: unnecessary since nkf 2.06.
        #
        # # nkf need UTF-16 BOM.
        # if ($code_f eq 'UTF-16BE') {
        #     $$contref = "\xfe\xff" . $$contref;
        # } elsif ($code_f eq 'UTF-16LE') {
        #     $$contref = "\xff\xfe" . $$contref;
        # }

        if ($conf::NKF eq 'module_nkf') {
            from_to_by_nkf_m($contref, $code_f, $code_t);
            util::dprint("Encode from $code_f to $code_t via nkf_module");
        } elsif ($conf::NKF ne 'no') {
            from_to_by_nkf($contref, $code_f, $code_t);
            util::dprint("Encode from $code_f to $code_t via nkf");
        }
    }
}

sub decode_filename ($$) {
    my ($contref, $code_f) = @_;
    my $code_to = 'utf-8';
    my $err = undef;
    if (($code_f eq 'CAP') || ($code_f eq 'HEX')) {
        $$contref =~ s/:(\w\w)/chr(hex($1))/eg;
        $err = encode_from_to($contref, 'cp932', $code_to);
    } else {
        $err = encode_from_to($contref, $code_f, $code_to);
    }
}

sub to_inner_encoding ($$) {
    my ($contref, $code_from) = @_;
    my $err = undef;
    my $code_to = 'utf-8';
    if (!($code_from)) {
        $code_from = 'unknown';
    }
    if (util::islang("ja")) {
        $err = encode_from_to($contref, $code_from, $code_to);
        if ($conf::NKF eq 'module_encode') {
            decode_mime_header($contref, $code_to);
            normalize_jp($contref);
        }
    }
    return $err;
}

sub to_external_encoding ($) {
    my ($contref) = @_;

    my $err;
    if ($util::EXT_ENCODE ne 'utf-8') {
        $err = encode_from_to($contref, 'utf-8', $util::EXT_ENCODE);
    }
    return $err;
}

sub load_encode {
    if ($conf::NKF =~ /^module_iconv/) {
        # FIXME:
    } elsif (($conf::NKF =~ /^module_encode/) && ($] >= 5.008)) {
        eval 'use Encode qw/ from_to decode _utf8_off /;';
        if ($@) {return $@};
        eval 'use Encode::Guess qw/ euc-jp shiftjis 7bit-jis utf-8 /;';
        if ($@) {return $@};
        return undef;
    } elsif (util::islang("ja") || util::islang_msg("ja")) {
        if ($conf::NKF =~ /^module_nkf/) {
            eval 'use NKF 2.06;';
            if ($@) {return 'unable to require "NKF": ' . $@};
        }
    }
    return undef;
}

sub utf8_zen2han_ascii ($) {
    my ($strref) = @_;
    if (util::islang("ja")) {
        # !#$%&()*+,./
        # 0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_
        $$strref =~ s/\xEF\xBC([\x81-\xBF])/chr(ord($1)-0x60)/ge;
        # `abcdefghijklmnopqrstuvwxyz{|}
        $$strref =~ s/\xEF\xBD([\x80-\x9D])/chr(ord($1)-0x20)/ge;
        # space
        $$strref =~ s/\xE3\x80\x80/ /g;
    }
}

sub normalize_jp ($) {
    my ($contref) = @_;
    if (util::islang("ja")) {
        {
            codeconv::utf8_han2zen_kana($contref);
            codeconv::utf8_zen2han_ascii($contref);
        }
    }
    $contref;
}

sub normalize_nl ($) {
    my ($conts) = @_;

    $$conts =~ s/\x0d\x0a/\x0a/g;  # Windows
    $$conts =~ s/\x0d/\x0a/g;      # Mac
    $$conts =~ s/\x0a/\n/g;
    $$conts;
}

sub remove_control_char ($) {
    my ($textref) = @_;
    $$textref =~ tr/\x01-\x08\x0b-\x0c\x0e-\x1f\x7f/ /; # Remove control char.
}

sub normalize_document ($) {
    my ($textref) = @_;
    codeconv::normalize_nl($textref);
    codeconv::remove_control_char($textref);
}

sub codeconv_document ($) {
    my ($textref) = @_;
    codeconv::to_inner_encoding($textref, 'unknown');
    codeconv::normalize_document($textref);
}

sub normalize_jp_document ($) {
    my ($textref) = @_;
    codeconv::normalize_jp($textref);
    codeconv::normalize_document($textref);
}

sub tousascii ($) {
    my ($contref) = @_;

    $$contref =~ s/[\x80-\xFF]/#/g;

    return $$contref;
}

1;
