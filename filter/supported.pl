#
# -*- Perl -*-
# $Id: supported.pl,v 1.3 1999-08-28 00:46:51 satoru Exp $
#
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
package supported;
my %supported = 
    (
     # The following documents don't need any filters.
     # (mknmz has built-in filters for them)
     "text/plain"                => "yes",
     "text/html"                 => "yes",
     "text/plain; x-type=rfc"    => "yes",
     "text/html; x-type=mhonarc" => "yes",
     "message/rfc822"            => "yes",
     "message/news"              => "yes",

     # The following documents need external filters.
     "application/pdf"           => "yes",
     "application/msword"        => "yes",
     "application/x-gzip"        => "yes",
     "application/x-bzip2"       => "yes",
     "application/x-compress"    => "yes",
    );
