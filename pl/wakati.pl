#
# -*- Perl -*-
# $Id: wakati.pl,v 1.16 2005-09-03 08:41:07 usu Exp $
# Copyright (C) 1997-1999 Satoru Takabayashi All rights reserved.
# Copyright (C) 2000-2005 Namazu Project All rights reserved.
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

# Do wakatigaki processing for a Japanese text.
sub wakatize_japanese ($) {
    my ($content) = @_;

    my @tmp = wakatize_japanese_sub($content);

    # Remove words consists of only Hiragana characters 
    # when -H option is specified.
    # Original of this code was contributed by <furukawa@tcp-ip.or.jp>. 
    # [1997-11-13]
    # And do Okurigana processing. [1998-04-24]
    if ($var::Opt{'hiragana'} || $var::Opt{'okurigana'}){
        for (my $ndx = 0; $ndx <= $#tmp; ++$ndx){
	    $tmp[$ndx] =~ s/(\s)/ $1/g;
	    $tmp[$ndx] = ' ' . $tmp[$ndx] . ' ';
            $tmp[$ndx] =~ s!\x7f *(\d+) *\x7f([^\x7f]*)\x7f */ *\d+ *\x7f!\x7f$1\x7f $2 \x7f/$1\x7f!g;
	    if ($var::Opt{'okurigana'}) {
		$tmp[$ndx] =~ s/((?:[^\xa4][\xa1-\xfe])+)(?:\xa4[\xa1-\xf3])+ /$1 /g;
	    }
	    if ($var::Opt{'hiragana'}) {
		$tmp[$ndx] =~ s/ (?:\xa4[\xa1-\xf3])+ //g;
	    }
        }
    }

    # Collect only noun words when -m option is specified.
    if ($var::Opt{'noun'}) {
	$$content = "";
	$$content .= shift(@tmp) =~ /(.+ )\xcc\xbe\xbb\xec/ ? $1 : "" while @tmp; 
	# noun (meisi) in Japanese "cc be bb ec"
    } else {
	$$content = join("\n", @tmp);
    }
    util::dprint(_("-- wakatized content --\n")."$$content\n");
}

sub wakatize_japanese_sub ($) {
    my ($content) = @_;
    my $str = "";
    my @tmp = ();

    if ($conf::WAKATI =~ /^module_(\w+)/) {
	my $module = $1;
	if ($module eq "kakasi") {
	    $str = Text::Kakasi::do_kakasi($$content);
	} elsif ($module eq "chasen") {
	    $str = Text::ChaSen::sparse_tostr_long($$content);
	} elsif ($module eq "mecab") {
	    use vars qw($t);
	    if (!defined $t) {
		require MeCab;
		import MeCab;
		$t = new MeCab::Tagger([qw(mecab -O wakati)]);
	    } 
	    END {
		$t->DESTROY() if defined $t;
	    }; 
	    $str = $t->parse($$content);
	} elsif ($module eq "builtin") {
	    $str = builtinwakati::wakati($content);
	} else {
	    util::cdie(_("invalid wakati module: ")."$module\n");
	}
        util::dprint(_("-- wakatized bare content --\n")."$str\n\n");
	@tmp = split('\n', $str);
    } else {
	my $tmpfile = util::tmpnam("NMZ.wakati");
        util::dprint(_("wakati: using ")."$conf::WAKATI\n");
	# Don't use IPC::Open2 because it's not efficent.
	{
	    my $fh_wakati = util::efopen("|$conf::WAKATI > $tmpfile");
	    print $fh_wakati $$content;
            util::fclose($fh_wakati);
	}
	{
	    my $fh_wakati = util::efopen($tmpfile);
	    @tmp = <$fh_wakati>;
	    chomp @tmp;
            util::fclose($fh_wakati);
	}
	unlink $tmpfile;
    }

    return @tmp;
}

1;
