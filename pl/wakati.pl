#
# -*- Perl -*-
# $Id: wakati.pl,v 1.26 2006-08-12 05:45:03 opengl2772 Exp $
# Copyright (C) 1997-1999 Satoru Takabayashi All rights reserved.
# Copyright (C) 2000-2006 Namazu Project All rights reserved.
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

my $kakasi_utf8 = undef;
sub init_text_kakasi () {
    $kakasi_utf8 = Text::Kakasi->new(qw/-iutf-8 -outf-8 -w/);
}

# Unicode: CJK Unified Ideographs(U+4E00 - U+9FAF)
my $kanji = "([\xE4-\xE9][\x80-\xBF][\x80-\xBF])";

# Unicode: Hiragana(U+3041 - U+3093)
my $hiragana = "(\xE3\x81[\x81-\xBF]|\xE3\x82[\x80-\x93])+";

my $noun = "\xE5\x90\x8D\xE8\xA9\x9E";
# noun (meisi) in Japanese "cc be bb ec"

# Do wakatigaki processing for a Japanese text.
sub wakatize_japanese ($) {
    my ($content) = @_;

    my @tmp = ();
    @tmp = wakatize_japanese_sub($content);

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
		$tmp[$ndx] =~ s/$kanji
				$hiragana\s/$1 /gx;
	    }
	    if ($var::Opt{'hiragana'}) {
		$tmp[$ndx] =~ s/ $hiragana //g;
	    }
        }
    }

    # Collect only noun words when -m option is specified.
    if ($var::Opt{'noun'}) {
	$$content = "";
	$$content .= shift(@tmp) =~ /(.+ )$noun/ ? $1 : "" while @tmp; 
    } else {
	$$content = join("\n", @tmp);
    }
    $$content =~ s/^\s+//gm;
    $$content =~ s/\s+$//gm;
    $$content =~ s/ +/ /gm;
    $$content .= "\n";
    my $tmp_cont = $$content;
    codeconv::to_external_encoding(\$tmp_cont);
    util::dprint(_("-- wakatized content --\n")."$tmp_cont\n");
}

sub wakatize_japanese_sub ($) {
    my ($content) = @_;
    my $str = "";
    my @tmp = ();

    if ($conf::WAKATI =~ /^module_(\w+)/) {
	my $module = $1;
	if ($module eq "kakasi") {
	    {
		$str = $kakasi_utf8->get($$content);
		Encode::_utf8_off($str);
		util::dprint(_("wakati: using ")."module_kakasi(utf-8)\n");
	    }
	} elsif ($module eq "chasen") {
	    $str = Text::ChaSen::sparse_tostr_long($$content);
	} elsif ($module eq "mecab") {
	    use vars qw($t);
	    if (!defined $t) {
		require MeCab;
		import MeCab;
                eval '$t = new MeCab::Tagger("-Owakati");' or
		$t = new MeCab::Tagger([qw(mecab -O wakati)]);
	    } 
	    END {
		$t->DESTROY() if defined $t;
	    }; 
	    $str = $t->parse($$content);
        }elsif($module eq "builtin") {
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
