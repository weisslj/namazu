#
# -*- CPerl -*-
# $Id: gfilter.pl,v 1.3 1999-08-30 03:13:17 satoru Exp $
# Copyright (C) 1999 Satoru Takabayashi ,
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

package gfilter;

# Show debug information for filters
sub show_filter_debug_info ($$$$) {
    my ($contref, $weighted_str, $fields, $headings) = @_;
    util::dprint("-- title --\n$fields->{title}\n");
    util::dprint("-- content --\n$$contref\n");
    util::dprint("-- weighted_str: --\n$$weighted_str\n");
    util::dprint("-- headings --\n$$headings\n");
}

# Adjust white spaces
sub white_space_adjust_filter ($) {
    my ($text) = @_;
    $$text =~ s/^ +//gm;
    $$text =~ s/ +$//gm;
    $$text =~ s/ +/ /g;
    $$text =~ s/\n+/\n/g;
}

# �ե�����̾���饿���ȥ����� (ñ�ʤ�ƥ����ȥե�����ξ��)
sub filename_to_title ($$) {
    my ($cfile, $weighted_str) = @_;

    # for MSWin32's filename using Shift_JIS [1998-09-24]
    if (($mknmz::SYSTEM eq "MSWin32") || ($mknmz::SYSTEM eq "os2")) {
	$cfile = codeconv::shiftjis_to_eucjp($cfile);
    }
    
    $cfile =~ m!^.*/([^/]*)$!;
    my $filename = $1;
    # �ե�����̾�򸵤˥�����ɤ���Ф��Ƥߤ� v1.1.1
    # modified [1998-09-18] 
    my $tmp = $filename;
    $tmp =~ s|/\\_\.-| |g;

    my $weight = $conf::Weight{'html'}->{'title'};
    $$weighted_str .= "\x7f$weight\x7f$tmp\x7f/$weight\x7f\n";

    my $title = $filename . $conf::TEXT_TITLE;
    return $title
}

# uuencode ���ɤ����Ф��롼����ϸ���@��ޥϤ��󤬤�������ޤ�����[1997-09-28]
# �ŤͽŤʹ��դǤ������� BinHex ���ɲä��Ƥ�餤�ޤ��� [1997-11-13]
# �䤬�����ä����Ȥˤ��Х��������Ƥ��������ޤ��� [1998-02-05] Thanks!
sub uuencode_filter ($) {
    my ($content) = @_;
    my @tmp = split(/\n/, $$content);
    $$content = "";
    
    my $uuin = 0;
    while (@tmp) {
	my $line = shift @tmp;
	$line .= "\n";

	# BinHex ���ɤ����Ф�
	# ���ͤ��褯ʬ����ʤ��Τǡ��Ǹ�ޤ����Ф�
	last if $line =~ /^\(This file must be converted with BinHex/; #)

	# uuencode ���ɤ����Ф�
	# ����ʸ�� : SunOS 4.1.4 �� man 5 uuencode
	#            FreeBSD 2.2 �� uuencode.c
        # �����ޥå����Ƥ��ޤä����Υǥ��åȤ򾯤ʤ����뤿��
	# ���ΤΥե����ޥåȥ����å���Ԥʤ�
	#
	# News �ʤɤǥե������ʬ�䤷����Ƥ���Ƥ����Τξ�� begin ���ʤ�
	# ���Ȥ�����ΤǤ�����θ���ޤ� by S.Takabayashi [v1.0.5]
	# �����ޥå����뤳�ȤϤۤȤ�ɤʤ��Ȥϻפ��ޤ�����
	#
	# length �� 62 �� 63 ������ߤ����� [v1.0.5]
	# �⤷��������¾�ˤ�㤤������Τ���
	#
	# ���ͤ���¤�ɽ������ȡ�
	# int((ord($line) - ord(' ') + 2) / 3)
	#     != (length($line) - 2) / 4
	# �Ȥʤ뤬�������ѷ����ơ�
	# 4 * int(ord($line) / 3) != length($line) + $uunumb;

        # SunOS �� uuencode �ϡ�encode �˶����ȤäƤ��롣
        # �������������ǧ���ȡ����̤ιԤ� uuencode �Ԥȸ�ǧ����
        # ��ǽ�����⤯�ʤ롣
        # ����ƤȤ��ơ����Υ�������ǧ��롣
        #     begin �� end �δ�
        #     ���ιԤ� uuencode �Ԥ�Ƚ�Ǥ���ơ�ord �����ιԤ�Ʊ��
	
	# ��Ԥ� 0x20-0x60 ��ʸ���Τߤǹ����������Τ� uuencode 
	# �Ȥߤʤ� v1.1.2.3 (bug fix)

        $uuin = 1, next if $line =~ /^begin [0-7]{3,4} \S+$/;
        if ($line =~ /^end$/){
            $uuin = 0,next if $uuin;
        } else {
            # �����ǡ�ord ���ͤ� 32-95 ���ϰϤ�
	    my $uuord = ord($line);
	    $uuord = 32 if $uuord == 96;

            # uunumb = 38 �ιԤ� loop �γ��˽ФƤ���ȡ�
            # ���̤ιԤ� 63 ʸ���ιԤ����ä����ư��Ƥ��ޤ�
            my $uunumb = (length($line)==63)? 37: 38;

            if ((32 <= $uuord && $uuord < 96) &&
                length($line) <= 63 &&
                (4 * int($uuord / 3) == length($line) + $uunumb)){

                if ($uuin == 1 || $uuin == $uuord){
                    next if $line =~ /^[\x20-\x60]+$/;
                } else {
		    # begin����ϤޤäƤ��ʤ���Τϸ��������褦 [1998-05-22]
                    $uuin = $uuord, next if $line =~ /^M[\x21-\x60]+$/;
                }
            }
        }
        $uuin = 0;
        $$content .= $line;
    }
}

# ��Ƭ�������ζ��򡢥��֡���Ƭ�� > | # ���� (':' ��Ĥ��ä��� by ����)
# ���������ܸ�ǽ������ϲ��ԥ����ɤ���
# ������ʬ�Υ����ɤϸ���@��ޥϤ��󤬤�������ޤ�����[1997-09-15]
# ��ʸ�ϥ��ե��͡������β���ϻ䤬�դ�­���ޤ���
# 40ʸ��̤���ιԤˤĤ��ƹ��������ܸ�Ϣ�������Ԥ�ʤ��褦�ˤ��� v1.1.1
sub line_adjust_filter ($) {
    my ($text) = @_;
    return undef unless defined($$text);

    my @tmp = split(/\n/, $$text);
    for my $line (@tmp) {
	$line .= "\n";
	$line =~ s/^[ \>\|\#\:]+//;
	$line =~ s/ +$//;
	$line =~ s/\n// if (($line =~ /[\xa1-\xfe]\n*$/) &&
			    (length($line) >=40));
	$line =~ s/(��|��)$/$1\n/;
	$line =~ s/([a-z])-\n/$1/;  # for hyphenation.
    }
    $$text = join('', @tmp);
}

# not implimented yet.
sub analize_rcs_stamp()
{
}


1;

