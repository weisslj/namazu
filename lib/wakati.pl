#
# -*- Perl -*-
# $Id: wakati.pl,v 1.11 1999-09-05 03:14:10 satoru Exp $
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

package wakati;
use strict;

# �狼���񤭤ˤ�����ܸ��ñ��ζ��ڤ�Ф�
sub wakatize_japanese ($) {
    my ($content) = @_;

    my @tmp = wakatize_japanese_sub($content);

    # �Ҥ餬�ʤ����θ�Ϻ������ -H ���ץ�����
    # ���Υ����ɤϸ���@��ޥϤ��󤬤�������ޤ�����[1997-11-13]
    # ���겾̾�ˤĤ��Ƥ��б� (�����Υ����ɤ��) [1998-04-24]
    if ($var::Opt{'Hiragana'} || $var::Opt{'Okurigana'}){
        for (my $ndx = 0; $ndx <= $#tmp; ++$ndx){
	    $tmp[$ndx] =~ s/(\s)/ $1/g;
	    $tmp[$ndx] = ' ' . $tmp[$ndx];
	    if ($var::Opt{'Okurigana'}) {
		$tmp[$ndx] =~ s/([^\xa4][\xa1-\xfe])+(\xa4[\xa1-\xf3])+ /$1 /g;
	    }
	    if ($var::Opt{'Hiragana'}) {
		$tmp[$ndx] =~ s/ (\xa4[\xa1-\xf3])+ //g;
	    }
        }
    }

    # �ʻ����򸵤�̾��Τߤ���Ͽ���� -m ���ץ�����
    if ($var::Opt{'Morph'}) {
	$$content = "";
	$$content .= shift(@tmp) =~ /(.+ )̾��/ ? $1 : "" while @tmp; 
    } else {
	$$content = join("\n", @tmp);
    }
    util::dprint("-- wakatized content --\n$$content\n");
}

sub wakatize_japanese_sub ($) {
    my ($content) = @_;
    my $str = "";
    my @tmp = ();

    if ($conf::WAKATI =~ /^module_(\w+)/) {
	my $module = $1;
	if ($module eq "kakasi") {
	    $str = Text::Kakasi::do_kakasi($$content);
	} elsif ($module eq "chasen1") {
	    $str = Text::ChaSen1::sparse_tostr_long($$content);
	} else {
	    util::cdie("invalid wakati module: $module\n");
	}
        util::dprint("-- wakatized bare content --\n$str\n\n");
	@tmp = split('\n', $str);
    } else {
	my $tmpfile = util::tmpnam("NMZ.wakati");
        util::dprint("wakati: using $conf::WAKATI\n");
	# IPC::Open2 �⤢�뤱�ɻ�������ä��ѤǤ������٤��ä�
	{
	    my $fh_wakati = util::efopen("|$conf::WAKATI > $tmpfile");
	    print $fh_wakati $$content;
	}
	{
	    my $fh_wakati = util::efopen($tmpfile);
	    @tmp = <$fh_wakati>;
	    chomp @tmp;
	}
	unlink $tmpfile;
    }

    return @tmp;
}

1;
