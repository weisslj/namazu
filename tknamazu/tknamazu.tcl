#!/usr/local/bin/wish4.2jp
#
# Copyright (C) 1998 Ken-ichi Hirose. All rights reserved.
#

# File Name     : tknamazu
# Author        : K.Hirose
# Creation Date : Feb 1998
# Description   : Tcl/Tk Namazu client
# History       : UPDATE        REVISION        Revise detail
#                 98/03/24      1.00            first release
#                 98/03/26      1.01            some bug fix
#                 98/04/01      1.02            .gz bug fix
#                 98/04/07      1.03            DB title bug fix
#                 98/04/07      1.04            kinput2 fix
#                 98/04/09      1.05            man , info , keybind fix
#                 98/05/20      1.06            info fix
#                 98/06/05      1.07            kinput widget, man,info bug fix
#                 98/06/15      1.10            for Namazu 1.2.0
#                 98/06/22      1.11            manpath bug fix
#
#
global version; set version "1.11"

if [info exists env(TKNMZPATH)] {
    set basedir "$env(TKNMZPATH)"
} else {
    set basedir [lindex $argv0 0]
#    regsub -all {\\} $basedir {/} basedir
    regsub {^(.*)/.*$} $basedir {\1} basedir
}

################ Evaluation and Initialize items
global conf
set conf(EXTBROWSER) {/usr/local/netscape/netscape}
set conf(EXTVIEWER) {/usr/local/bin/jless}
set conf(NAMAZU) {/usr/local/bin/namazu}
set conf(BROWSER) {/usr/local/bin/lynx -dump}
set conf(UNCOMPRESS) {/usr/bin/zcat}
set conf(MANPAGE) {/usr/local/bin/groff -man -Tnippon}
set conf(MANPAGEFILTER) {}
set conf(MANPATH) {/usr/local/man/ja_JP.EUC /usr/local/man/ja_JP.ujis /usr/man/ja_JP.ujis /usr/local/man /usr/share/man /usr/bin/man /usr/man /usr/X11R6/man /usr/openwin/man}
set conf(GNUINFO) {/usr/bin/info}
set conf(GNUINFOFILTER) {}
set conf(GNUINFOTMP) {/tmp/.gnuinfotmp}
set conf(MKNMZ) {/usr/local/namazu/bin/mknmz}
set conf(WDNMZ) {/usr/local/namazu/bin/wdnmz}
set conf(DBOUTPUTDIR) {/usr/local/namazu/index}
set conf(WIDTH) {80}
set conf(HEIGHT) {20}
set conf(WIDGETFONT) {-misc-Fixed-Medium-R-Normal-*-*-120-*-*-*-*-*-*}
set conf(TEXTFONT) {-misc-Fixed-Medium-R-Normal-*-*-120-*-*-*-*-*-*}
set conf(WIDGETCOLOR) {gray90}
set conf(LANGUAGE) {JAPANESE}
set conf(OPTIONS) {}
set conf(NMZCONF) {}
set conf(DATABASES) {}
set conf(MKLOCATION) {./}
set conf(MKOPTIONS) {}
set conf(WDLOCATION) {./NMZ.i}

set conf(platform) "$tcl_platform(platform)"
set conf(tclversion) "$tcl_version"
set conf(tkversion) "$tk_version"
set conf(tclpatchlevel) "$tcl_patchLevel"
set conf(tkpatchlevel) "$tk_patchLevel"
set conf(machine) "$tcl_platform(machine)"
set conf(os) "$tcl_platform(os)"
set conf(osversion) "$tcl_platform(osVersion)"
set conf(userhome) "$env(HOME)"
set conf(tknmzhome) "$basedir"

if {[catch {open "$conf(userhome)/.tknmzrc"} input] \
    && [catch {open "$conf(tknmzhome)/.tknmzrc"} input]} {
    puts "I can not read resource file ... (T_T)\n"
} else {
    foreach line [split [read $input] \n] {
        if [regexp {^([^#][0-9A-Z]*)	+(.*)$} $line match item value] {
            set conf($item) "$value"
        }
    }
}

append conf(GNUINFOTMP) ".[pid]"
set conf(viewer) {}
set conf(manpagedir) {}
set conf(gnuinfofile) {}
set conf(commandline) {}
set keyword {}
set lynxauth {}
set urlforext {}

################ proc routine
proc Exec { keyword } {
    global log but conf

    MakeOptions
    $log config -state normal
    $log delete 1.0 end
    $log config -cursor watch
    regsub -all {\"([^\"]+)\"} $keyword {{\1}} keyword
    set conf(commandline) "%$conf(NAMAZU) $conf(OPTIONS) \"$keyword\" $conf(DATABASES)"
    $log insert end "$conf(commandline)\n\n"
    if [catch {open "| $conf(NAMAZU) $conf(OPTIONS) \"$keyword\" $conf(DATABASES)"} input] {
        $log insert end "$input\n"
    } else {
        set i 0
        foreach line [split [read $input] \n] {
            if [regexp {^(/.*|[A-Za-z]:/.*|http://.*|file://.*|ftp://.*)( size \(.*\))$} \
                $line match url tail ] {
                incr i
                $log insert end "$url" kicker$i
                $log insert end "$tail\n"
                $log tag configure kicker$i -underline true -foreground blue
                $log tag bind kicker$i <Enter> {%W config -cursor hand2}
                $log tag bind kicker$i <Leave> {%W config -cursor {} }
                $log tag bind kicker$i <Button-1> {Browse {}}
            } else {
                $log insert end "$line\n"
            }
        }
    }
    close $input
    MarkIt $keyword
    $but config -text {Search} -command {Exec $keyword}
    $log see 1.0
    $log config -cursor {}
    $log config -relief raised -state disabled
    focus .tknmz.result.scroll
}

proc Kick { url } {
    global log conf

    if {![string compare $url ""]} {
        if [regexp {(kicker[0-9]+)} [$log tag names [$log index current]] match urltag] {
            set url [$log get $urltag.first $urltag.last]
        } else {
            return
        }
    }

    after 1000
    if [regexp {(\.(htm|HTM).*$)|(^http://)|(^ftp://)|(^file://)} $url] {
        if {![catch {file readlink $conf(userhome)/.netscape/lock} input]} {
            catch {exec $conf(EXTBROWSER) -remote openURL($url) &} input
        } else {
            catch {exec $conf(EXTBROWSER) $url &} input
        }
    } else {
        regsub {^/([A-z])\|(/.*)} $url {\1:\2} url
        catch {exec $conf(EXTVIEWER) $url &} input
    }
}

proc Browse { url } {
    global conf log keyword but

    if {![string compare $url ""]} {
        regexp {(kicker[0-9]+)} [$log tag names [$log index current]] match urltag
        regexp {(node[0-9]+)} [$log tag names [$log index current]] match urltag
        set url [$log get $urltag.first $urltag.last]
    }

    $but config -text {Back to result} -command {Exec $keyword}
    switch -regexp $url {
        {(\.(htm|HTM).*$)|(^http://)|(^ftp://)|(^file://)} {
            set conf(viewer) {lynx}
#            regsub {^/([A-z])\|(/.*)} $url {//\1\2} url  #for lynx by cygwin32
            Browser $url
        }
        {(\*.*::)|(^\*.*:.*\.)|((Next|Prev|Up): [^,]*,)|((Next|Prev|Up): [^,]*$)} {
            set conf(viewer) {gnuinfo}
            regsub -all {\* *(.*)::} $url {\1} url
            regsub -all {^\*.*:(.*)\.} $url {\1} url
            regsub -all -nocase {^(Note|Next|Prev|Up):* (.*)} $url {\2} url
            regsub -all {(.*)\. +([0-9]+$)|,$} $url {\1} url
            regsub -all {^ *(.*) *$} $url {\1} url
            if [regexp {\((.+)\).*} $url match file] {
                regexp {(.*/info/)[^/]*$} $conf(gnuinfofile) match basedir
                set conf(gnuinfofile) "$basedir$file"
                set url {}
            }
            InfoViewer $conf(gnuinfofile) $url
        }
        {^[^/]([a-z0-9_\-\.]+)\(([1-9nl])\)} {
            set conf(viewer) {manpage}
            regexp {([a-z0-9_\-\.]+)\(([1-9nl])\)} $url match manual section
            regexp {(.*)/man[1-9nl]/[^/]*$} $conf(manpagedir) match basedir
            set url "$basedir/man$section/$manual.$section"
            foreach manpath [split $conf(MANPATH)] {
                set url "$manpath/man$section/$manual.$section"
                if {[file exists $url] || [file exists $url.gz]} {
                    break
                } else {
                    set url "$basedir/man$section/$manual.$section"
                }
            }
            ManViewer $url
        }
        {.*/info/[^/]*$} {
            set conf(viewer) {gnuinfo}
            regsub {^/([A-z])\|(/.*)} $url {\1:\2} url
            regsub -nocase {(,*)(-[1-9]+)(.*)} $url {\1\3} url
            InfoViewer $url {}
        }
        {.*/man[1-9nl]/[^/]*$} {
            set conf(viewer) {manpage}
            regsub {^/([A-z])\|(/.*)} $url {\1:\2} url
            ManViewer $url
        }
        default {
            set conf(viewer) {}
            regsub {^/([A-z])\|(/.*)} $url {\1:\2} url
            Viewer $url
        }
    }
    focus .tknmz.result.scroll
}

proc Viewer { url } {
    global conf log keyword

    if {![file exists $url]} {
        tk_messageBox -icon info -type ok -title "Error occurred" -message "$url: File not found."
        return
    }
    $log config -state normal
    $log delete 1.0 end
    $log config -cursor watch
    if [regexp {.*\.gz$} $url ] {
        set conf(commandline) "%$conf(UNCOMPRESS) $url"
        set url "| $conf(UNCOMPRESS) $url"
    } else {
        set conf(commandline) "%$url"
    }
    if [catch {open "$url"} input] {
        $log insert end "$conf(commandline)\n"
        $log insert end $input
    } else {
        $log insert end "$conf(commandline)\n"
        foreach line [split [read $input] \n] {
            regsub -all {_|..|.} $line {} line
            $log insert end "$line\n"
        }
    }
    close $input
    RichEditControl
    MarkIt $keyword
    $log see 1.0
    $log config -cursor {}
    $log config -relief raised -state disabled
}

proc Browser { url } {
    global conf log keyword lynxauth
    if {![string compare $lynxauth ""]} {
        set browser $conf(BROWSER)
    } else {
        set browser "$conf(BROWSER) -auth=$lynxauth"
    }

    $log config -state normal
    $log delete 1.0 end
    $log config -cursor watch
    set conf(commandline) "%$conf(BROWSER) $url"
    if [catch {open "| $browser $url"} input] {
        $log insert end "$conf(commandline)\n"
        $log insert end $input
    } else {
        $log insert end "$conf(commandline)\n"
        $log insert end [read $input]
    }
    close $input
    RichEditControl
    MarkIt $keyword
    $log see 1.0
    $log config -cursor {}
    $log config -relief raised -state disabled
}

proc InfoViewer { url node } {
    global conf log but keyword
    set conf(gnuinfofile) "$url"
    if {![string compare $node ""]} {
        set node {Top}
    }

    if {![file exists $url]} {
        if {![file exists $url.gz]} {
            tk_messageBox -icon info -type ok -title "Error occurred" -message "$url: File not found."
            return
        } else {
            append url ".gz"
        }
    }
    $log config -state normal
    $log delete 1.0 end
    $log config -cursor watch
    set conf(commandline) "%$conf(GNUINFO) -f $url -o $conf(GNUINFOTMP) -n \"$node\""
    set incode [kanji code $node]
    if {[string compare $incode "ANY"]} {
        set node [kanji conversion $incode JIS $node]
    }
    catch {exec $conf(GNUINFO) -f $url -o $conf(GNUINFOTMP) -n "$node"} input
    if {![file exists $conf(GNUINFOTMP)]} {
        $log insert end "$conf(commandline)\n"
        $log insert end $input
        $log config -cursor {}
        $log config -relief raised -state disabled
        return
    }
    if [catch {open "$conf(GNUINFOTMP)"} input] {
        $log insert end "$conf(commandline)\n"
        $log insert end $input
    } else {
        $log insert end "$conf(commandline)\n"
        $log insert end [read $input]
    }
    close $input
    file delete -force -- $conf(GNUINFOTMP)
    RichEditControl
    MarkIt $keyword
    $log see 1.0
    $log config -cursor {}
    $log config -relief raised -state disabled
}
proc ManViewer { url } {
    global conf log but keyword
    set conf(manpagedir) "$url"

    if {![file exists $url]} {
        if {![file exists $url.gz]} {
            tk_messageBox -icon info -type ok -title "Error occurred" -message "$url: File not found."
            return
        } else {
            append url ".gz"
        }
    }
    $log config -state normal
    $log delete 1.0 end
    $log config -cursor watch
    switch -regexp $url {
        {.*/man[1-9n]/[^/]*\.gz$} {
            set conf(commandline) "%$conf(UNCOMPRESS) $url | $conf(MANPAGE) $conf(MANPAGEFILTER)"
            set url "| $conf(UNCOMPRESS) $url | $conf(MANPAGE) $conf(MANPAGEFILTER)"
        }
        {.*/man[1-9n]/[^/]*$} {
            set conf(commandline) "%$conf(MANPAGE) $url $conf(MANPAGEFILTER)"
            set url "| $conf(MANPAGE) $url $conf(MANPAGEFILTER)"
        }
    }
    if [catch {open "$url"} input] {
        $log insert end "$conf(commandline)\n"
        $log insert end $input
    } else {
        $log insert end "$conf(commandline)\n"
        foreach line [split [read $input] \n] {
            regsub -all {_|..|.} $line {} line
            $log insert end "$line\n"
        }
    }
    close $input
    RichEditControl
    MarkIt $keyword
    $log see 1.0
    $log config -cursor {}
    $log config -relief raised -state disabled
}

proc RichEditControl {} {
    global conf log

    set cur 1.0
    set i 4096
    switch $conf(viewer) {
        {gnuinfo} {
            set pattern {http://[^ >"`']*|file://[^ >"`']*|ftp://[^ >"`']*|\*.*::|^\*.*:.*\.|(Next|Prev|Up): [^,]*,|(Next|Prev|Up): [^,]*$}
        }
        {manpage} {
            set pattern {http://[^ >"`']*|file://[^ >"`']*|ftp://[^ >"`']*|[a-z0-9_\-\.]+\([1-9nl]\)}
        }
        {lynx} {
            set pattern {http://[^ >"`']*|file://[^ >"`']*|ftp://[^ >"`']*|\[[0-9]+\]}
        }
        default {
            set pattern {http://[^ >"`']*|file://[^ >"`']*|ftp://[^ >"`']*}
        }
    }
    while 1 {
        set cur [$log search -regexp -count length -- "$pattern" $cur end]
        if {$cur == ""} {
            break
        }
        incr i
        set link [$log get $cur "$cur + $length char"]
        if {($conf(viewer) == "lynx") && [regexp {^\[} $link]} {
            $log tag add jumper$i $cur "$cur + $length char"
            $log tag configure jumper$i -underline true -foreground green3
            $log tag bind jumper$i <Enter> {%W config -cursor hand2}
            $log tag bind jumper$i <Leave> {%W config -cursor {} }
            $log tag bind jumper$i <Button-1> {Jump {}}
        }
        if {($conf(viewer) == "gnuinfo") && [regexp {^\*|^Next|^Prev|^Up} $link]} {
            $log tag add node$i $cur "$cur + $length char"
            $log tag configure node$i -underline true -foreground blue
            $log tag bind node$i <Enter> {%W config -cursor hand2}
            $log tag bind node$i <Leave> {%W config -cursor {} }
            $log tag bind node$i <Button-1> {Browse {}}
        }
        if {($conf(viewer) == "manpage") && [regexp {^[a-z0-9_\-\.]+\(} $link]} {
            $log tag add node$i $cur "$cur + $length char"
            $log tag configure node$i -underline true -foreground blue
            $log tag bind node$i <Enter> {%W config -cursor hand2}
            $log tag bind node$i <Leave> {%W config -cursor {} }
            $log tag bind node$i <Button-1> {Browse {}}
        }
        if {[regexp {^http:|^ftp:|^file:} $link]} {
            $log tag add kicker$i $cur "$cur + $length char"
            $log tag configure kicker$i -underline true -foreground blue
            $log tag bind kicker$i <Enter> {%W config -cursor hand2}
            $log tag bind kicker$i <Leave> {%W config -cursor {} }
            $log tag bind kicker$i <Button-1> {Browse {}}
        }
        set cur [$log index "$cur + $length char"]
    }
}

proc MarkIt { markword } {
    global conf log

    regsub -nocase -all { \& | \| | \! |\( | \)| and | or | not } $markword { } markword
    regsub -all {\{|\}|\"|\/|\[|\]|\^|\$|\.|\?|\+|\*} $markword {} markword
    regsub -all { +} $markword { } markword
    regsub -all {^ | $} $markword {} markword
    foreach find [split $markword] {
        if {$find == ""} {
            return
        }
        set cur 1.0
        while 1 {
            set cur [$log search -nocase -count length -- "$find" $cur end]
            if {$cur == ""} {
                break
            }
            $log tag add find $cur "$cur + $length char"
            set cur [$log index "$cur + $length char"]
        }
        $log tag configure find -foreground red
    }
}

proc Jump { ref } {
    global log

    if {![string compare $ref ""]} {
       regexp {(jumper[0-9]+)} [$log tag names [$log index current]] match reftag
       set ref [$log get $reftag.first $reftag.last]
       regsub {\[([0-9]+)\]} $ref {\1} ref
    }

    if {[$log search -regexp -- "^ *$ref\. .*" 1.0 end] == ""} {
        tk_messageBox -icon info -type ok -title "Error occurred" -message "Referenses not found. ;-("
        focus .tknmz.result.scroll
        return
    }
    $log tag remove references 0.0 end
    set cur 1.0
    while 1 {
        set cur [$log search -regexp -count length -- "^ *$ref\. .*" $cur end]
        if {$cur == ""} {
            break
        }
        $log tag add references $cur "$cur + $length char"
        set cur [$log index "$cur + $length char"]
        $log tag configure references -foreground green3
        $log tag raise references
        $log see $cur
    }
}

proc MakeOptions {} {
    global conf seldb opt

    regsub -all { \-a| \-n [0-9]+| \-s} $conf(OPTIONS) {} conf(OPTIONS)
    if {$opt(n) == "ALL"} {
        append conf(OPTIONS) " -a"
    } else {
        append conf(OPTIONS) " -n $opt(n)"
    }
    if {$opt(s) == "ON"} {
        append conf(OPTIONS) ""
    } else {
        append conf(OPTIONS) " -s"
    }

    if {$conf(NMZCONF) != ""} {
        regsub -all { \-f [^ ]+} $conf(OPTIONS) {} conf(OPTIONS)
        append conf(OPTIONS) " -f $conf(NMZCONF)"
    }

    if {$conf(DBOUTPUTDIR) != ""} {
        regsub -all { \-O [^ ]+} $conf(MKOPTIONS) {} conf(MKOPTIONS)
        append conf(MKOPTIONS) " -O $conf(DBOUTPUTDIR)"
    }

    set conf(initflag) 1
    foreach dbs [array names seldb] {
        if {$seldb($dbs)} {
            if {$conf(initflag)} {
                set conf(DATABASES) {}
                set conf(initflag) 0
            }
            regexp {^([^ ]+) +(.*)$} $conf($dbs) match title value
            append conf(DATABASES) " $value"
        }
    }
    set conf(initflag) 1
}

proc OpenViewer {} {
    global conf

    set openlocation [tk_getOpenFile -filetypes {{ALL {*}}}]
    if {![string compare $openlocation ""]} {
        return
    }
    Browse $openlocation
}

proc OpenURL {} {
    global conf openurl urllocation

    if [winfo exists .openurl] {
        focus .openurl.entry
        return
    } else {
        toplevel .openurl -borderwidth 10
        wm title .openurl "Open URL Dialog"
    }

    entry .openurl.entry -textvariable urllocation -width 70
    frame .openurl.buttons
    pack .openurl.entry .openurl.buttons -side top -fill x
    button .openurl.buttons.ok -text OK -command {set openurl(ok) 1} -underline 0
    button .openurl.buttons.cancel -text Cancel -command {set openurl(ok) 0} -underline 0
    pack .openurl.buttons.cancel .openurl.buttons.ok -side right

    bind .openurl <Alt-o> "focus .openurl.buttons.ok ; break"
    bind .openurl <Alt-c> "focus .openurl.buttons.cancel ; break"
    bind .openurl <Alt-Key> break
    bind .openurl <Return> {set openurl(ok) 1}
    bind .openurl <Escape> {set openurl(ok) 0}
    bind .openurl <Button-3> {tk_popup .popupmenu %X %Y}

    SetColor {.openurl}

    while 1 {
        focus .openurl.entry
        tkwait variable openurl(ok)
        if {$openurl(ok)} {
            Browser $urllocation
        } else {
            break
        }
    }
    focus .tknmz.result.scroll
    destroy .openurl
}

proc SaveFile {} {
    global conf log

    set openlocation [tk_getSaveFile -filetypes {{ALL {*}}}]
    if {![string compare $openlocation ""]} {
        return
    }
    set savefile [ open $openlocation "w" ]
    puts $savefile [ $log get 1.0 end ]
    close $savefile
}

proc MakeDBLuncher {} {
    global conf seldb

    menu .tknmz.file.menu.seldb
    foreach dbs [array names conf] {
        if [regexp {DATABASES([0-9]+)} $dbs ] {
            regexp {^([^ ]+) +(.*)$} $conf($dbs) match title value
            .tknmz.file.menu.seldb add check -label $title -variable seldb($dbs) -selectcolor blue
        }
    } 
    .tknmz.file.menu.seldb add separator
    .tknmz.file.menu.seldb add command -label {Select All} -underline 0 -command {SelectAll}
    .tknmz.file.menu.seldb add command -label {Clear All} -underline 0 -command {ClearAll}
}

proc MakeBMLuncher {} {
    global conf bookmarks

    menu .tknmz.go.menu.bookmark
    foreach dbs [array names conf] {
        if [regexp {BOOKMARKS([0-9]+)} $dbs ] {
            regexp {^([^ ]+) +(.*)$} $conf($dbs) match title value
            .tknmz.go.menu.bookmark add command -label $title -command [list Browse $value]
        }
    } 
}

proc SelectAll {} {
    global seldb

    foreach dbs [array names seldb] {
        set seldb($dbs) 1
    }
}

proc ClearAll {} {
    global seldb

    foreach dbs [array names seldb] {
        set seldb($dbs) 0
    }
}

proc AddDB {} {
    global conf seldb

    set openlocation [tk_getOpenFile -filetypes {{NMZ.i {.i}} {ALL {*}}}]
    regsub {^(.*)/NMZ.i} $openlocation {\1} openlocation
    set max 1
    foreach dbs [array names seldb] {
        regexp {DATABASES([0-9]+)} $dbs match i
        if {$i > $max} {
            set max $i
        }
    }
    incr max
    set conf(DATABASES$max) "OneTimeDB$max $openlocation"
    destroy .tknmz.file.menu.seldb
    MakeDBLuncher
}

proc LoadConf {} {
    global conf

    set conf(NMZCONF) [tk_getOpenFile -filetypes {{NAMAZU {.conf}} {ALL {*}}}]
}

proc SetAuthorization {} {
    global auth conf lynxauth

    if [winfo exists .auth] {
        focus .auth.entry
        return
    } else {
        toplevel .auth -borderwidth 10
        wm title .auth "Authorization Dialog"
    }

    label .auth.msg -borderwidth 10 -text {Please input LOGIN:PASSWORD}
    entry .auth.entry -textvariable lynxauth -width 17
    frame .auth.buttons -bd 10
    pack .auth.msg .auth.entry .auth.buttons -side top -fill x
    button .auth.buttons.ok -text OK -command {set auth(ok) 1} -underline 0
    button .auth.buttons.cancel -text Cancel -command {set auth(ok) 0} -underline 0
    pack .auth.buttons.cancel .auth.buttons.ok -side right

    bind .auth <Alt-o> "focus .auth.buttons.ok ; break"
    bind .auth <Alt-c> "focus .auth.buttons.cancel ; break"
    bind .auth <Alt-Key> break
    bind .auth <Return> {set auth(ok) 1}
    bind .auth <Escape> {set auth(ok) 0}
    bind .auth <Button-3> {tk_popup .popupmenu %X %Y}

    SetColor {.auth}

    while 1 {
        focus .auth.entry
        tkwait variable auth(ok)
        if {$auth(ok)} {
            break
        } else {
            set lynxauth {}
            tk_messageBox -icon info -type ok -title "Clear" -message "Clear input authorization."
            break
        }
    }
    focus .tknmz.result.scroll
    destroy .auth
}

proc FindWord {} {
    global conf log prompt keyword findword

    if [winfo exists .prompt] {
        focus .prompt.entry
        return
    } else {
        toplevel .prompt -borderwidth 10
        wm title .prompt "Find Dialog"
    }

    label .prompt.msg -borderwidth 10 -text {Please input pattern.}
    entry .prompt.entry -textvariable findword -width 32
    frame .prompt.buttons -bd 10
    pack .prompt.msg .prompt.entry .prompt.buttons -side top -fill x
    button .prompt.buttons.ok -text OK -command {set prompt(ok) 1} -underline 0
    button .prompt.buttons.cancel -text Cancel -command {set prompt(ok) 0} -underline 0
    pack .prompt.buttons.cancel .prompt.buttons.ok -side right
    if {![catch {selection get} sel]} {
        .prompt.entry delete 0 end
        .prompt.entry insert insert [ selection get -selection PRIMARY]
    } else {
        .prompt.entry delete 0 end
        set findword $keyword
        regsub -nocase -all { \& | \| | \! |\( | \)| and | or | not } $findword { } findword
        regsub -all {\{|\}|\"|\/|\[|\]|\^|\$|\.|\?|\+|\*} $findword {} findword
        regsub -all { +} $findword { } findword
        regsub -all {^ | $} $findword {} findword
    }

    bind .prompt <Alt-o> "focus .prompt.buttons.ok ; break"
    bind .prompt <Alt-c> "focus .prompt.buttons.cancel ; break"
    bind .prompt <Alt-Key> break
    bind .prompt <Return> {set prompt(ok) 1}
    bind .prompt <Escape> {set prompt(ok) 0}
    bind .prompt <Button-3> {tk_popup .popupmenu %X %Y}

    SetColor {.prompt}

    set cur 1.0
    while 1 {
        focus .prompt.entry
        tkwait variable prompt(ok)
        if {$prompt(ok)} {
            if {[$log search -nocase -- $findword 1.0 end] == ""} {
                tk_messageBox -icon info -type ok -title "Error occurred" -message "Pattern not found. ;-("
                break
            }
            $log tag remove find 0.0 end
            set cur [$log search -nocase -count length -- $findword $cur end]
            if {$cur == ""} {
                set cur 1.0
                set cur [$log search -nocase -count length -- $findword $cur end]
                tk_messageBox -icon info -type ok -title "Last line" -message "I scanned all text."
            }
            $log tag add find $cur "$cur + $length char"
            set cur [$log index "$cur + $length char"]
            $log tag configure find -foreground red
            $log tag raise find
            $log see $cur
        } else {
            break
        }
    }
    focus .tknmz.result.scroll
    destroy .prompt
    MarkIt $keyword
}

proc LoadTknmzrc {} {
    global log conf seldb
    set tknmzrc [tk_getOpenFile -filetypes {{ALL {*}}}]

    if [catch {open "$tknmzrc"} input] {
        return
    } else {
        $log config -state normal
        $log delete 1.0 end
        foreach line [split [read $input] \n] {
            if [regexp {^([^#][0-9A-Z]*)	+(.*)$} $line match item value] {
                $log insert end "$item : $value\n"
            }
        }
        $log config -relief raised -state disabled
    }

    set answer [tk_dialog .quit {Make sure} "Are you load this parameter? " question 1 {Yes} {No} ]
    if {$answer == 1} {
        return
    }

    if [catch {open "$tknmzrc"} input] {
            $log insert end $input
            return
    } else {
        unset conf
        unset seldb
        foreach line [split [read $input] \n] {
            if [regexp {^([^#][0-9A-Z]*)	+(.*)$} $line match item value] {
                set conf($item) "$value"
            }
        }
        destroy .tknmz.file.menu.seldb
        MakeDBLuncher
    }
}
proc DisplayConfiguration {} {
    global log conf version env

    $log config -state normal
    $log delete 1.0 end
    $log insert end "# TkNamazu version $version\n"
    $log insert end "# Date : [clock format [clock seconds]]\n"
    foreach item [array names conf] {
        $log insert end "$item		$conf($item)\n"
    }

    if [catch {open "| $conf(BROWSER) -version"} input] {
        $log insert end "#$conf(BROWSER):Command not found.\n"
    } else {
        foreach line [split [read $input] \n] {
            $log insert end "#$line\n"
        }
    }
    close $input
    if [catch {open "| $conf(GNUINFO) --version"} input] {
        $log insert end "#$conf(GNUINFO):Command not found.\n"
    } else {
        foreach line [split [read $input] \n] {
            $log insert end "#$line\n"
        }
    }
    close $input
    $log config -relief raised -state disabled
}

proc CopyString {} {
    if [catch {selection get} sel] {
        return
    }
    if { [ selection own ] != "" } {
        clipboard clear
        clipboard append [selection get ]
    }
}

proc CopyURL {} {
    global urlforext

    if { $urlforext != "" } {
        clipboard clear
        clipboard append $urlforext
    }
}

proc CutString {} {
    set widget [focus]

    if {[catch {selection get} sel] || ![regexp {\.tknmz\.result\.log|\.tknmz\.keyword} $widget]} {
        return
    }
    if { [ selection own ] != "" } {
        clipboard clear
        clipboard append [ selection get ]
        $widget delete sel.first sel.last
    }
}

proc PasteString { widget } {
    if {![string compare $widget ""]} {
        set widget [focus]
    }
    if {![winfo exists $widget] || ![regexp {\.tknmz\.result\.log|\.tknmz\.keyword|\.prompt\.entry|\.openurl\.entry|\.auth\.entry} $widget]} {
        return
    }

    if [catch {selection get} sel] {
        if [catch {selection get -selection CLIPBOARD} sel] {
            return
        }
    }
    $widget insert insert $sel
}

proc FastExec {} {
    global keyword

    if [catch {selection get} sel] {
        return
    }
    set keyword [ selection get ]
    Exec [ selection get ]
}

proc SetColor { w } {
    global conf

    $w config -background $conf(WIDGETCOLOR)
    foreach child [winfo children $w] {
        SetColor $child
    }
}

proc ScrollDownResult {number unit} {
    .tknmz.result.log yview scroll $number $unit;
}

proc PopupMenu { X Y } {
    global log urlforext
    set urlforext {}

    if [regexp {(kicker[0-9]+)} [$log tag names [$log index current]] match urltag] {
        set urlforext [$log get $urltag.first $urltag.last]
    }

    tk_popup .popupmenu $X $Y
}

proc Confirmation {} {
    set answer [tk_dialog .confirmation {Quit?} "Really? ;-( " question 1 {Yes} {No} ]

    if {$answer == 0} {
        exit
    }
}

proc HelpView {} {
    global conf log keyword

    set conf(viewer) {lynx}
    Viewer "$conf(tknmzhome)/tknamazu.hlp"
    MarkIt {TKNAMAZU 必ず１つ以上の<TAB>で区切って 大文字 事前に}
}

proc AboutTknamazu {} {
    global version conf

    if [winfo exists .about] {
        focus .about.ok.b
        return
    } else {
        toplevel .about
        wm title .about "About TkNamazu"
    }

    frame .about.msg1 -relief flat -borderwidth 2 -bg white
    pack .about.msg1 -fill both
    label .about.msg1.title1 -text {Tcl/Tk Namazu client} -font $conf(WIDGETFONT) -background white
    label .about.msg1.title2 -text "version $version" -font $conf(WIDGETFONT) -background white
    label .about.msg1.title3 -image logo -background white
    label .about.msg1.title4 -text {- GPL2 software -} -font $conf(WIDGETFONT) -background white
    label .about.msg1.title5 -text {Copyright (C) 1998} -font $conf(WIDGETFONT) -background white
    label .about.msg1.title6 -text {Ken-ichi Hirose.} -font $conf(WIDGETFONT) -background white
    label .about.msg1.title7 -text {All rights reserved.} -font $conf(WIDGETFONT) -background white

    pack .about.msg1.title1 .about.msg1.title2
    pack .about.msg1.title3
    pack .about.msg1.title4 .about.msg1.title5 .about.msg1.title6 .about.msg1.title7

    frame .about.info -bg white
    pack .about.info -fill both
    set m1 {.about.info.list1}
    set m2 {.about.info.list2}

    listbox $m1 -bg white -height 7 -width 12 -font $conf(WIDGETFONT) -borderwidth 0
    $m1 insert end "TKNMZPATH"
    $m1 insert end "HOME"
    $m1 insert end "Tcl/Tk"
    $m1 insert end "PatchLevel"
    $m1 insert end "OS"
    $m1 insert end "Machine"
    $m1 insert end "Platform"

    listbox $m2 -bg white -height 7 -width 28 -font $conf(WIDGETFONT) -borderwidth 0
    $m2 insert end $conf(tknmzhome)
    $m2 insert end $conf(userhome)
    $m2 insert end "$conf(tclversion) / $conf(tkversion)"
    $m2 insert end "$conf(tclpatchlevel) / $conf(tkpatchlevel)"
    $m2 insert end "$conf(os) $conf(osversion)"
    $m2 insert end $conf(machine)
    $m2 insert end $conf(platform)

    pack $m1 $m2 -side left -expand yes -fill both

    
    frame .about.ok -bg white
    pack .about.ok -fill both
    button .about.ok.b -text { OK } -font $conf(WIDGETFONT) -command { destroy .about }
    pack .about.ok.b -expand yes
    focus .about.ok.b
}

################ helper proc routine
proc MknmzHelper {} {
    global mklog mkbut conf

    if [winfo exists .tkmknmz] {
        focus .tkmknmz.input.location
        return
    } else {
        toplevel .tkmknmz
        wm title .tkmknmz "mknmz"
    }

    frame .tkmknmz.input -borderwidth 1
    pack .tkmknmz.input -side top -fill x
    label .tkmknmz.input.l -text {Location:} -padx 0 -font $conf(WIDGETFONT)
    entry .tkmknmz.input.location -width 20 \
        -relief sunken -textvariable conf(MKLOCATION)
    pack .tkmknmz.input.l -side left
    pack .tkmknmz.input.location -side left -fill x -expand true
    set mkbut [button .tkmknmz.input.execute -text {Execute} -font $conf(WIDGETFONT) \
        -underline 1 -command {ExecMknmz}]
    button .tkmknmz.input.browse -text {Browse} -font $conf(WIDGETFONT) \
        -underline 0 -command {DirectoryExplorer}
    pack .tkmknmz.input.browse .tkmknmz.input.execute -side right
    bind .tkmknmz.input.location <Return> {ExecMknmz}
    bind .tkmknmz.input.location <Control-c> {Break}
    bind .tkmknmz <Alt-x> {focus .tkmknmz.input.execute ; break}
    bind .tkmknmz <Alt-b> {focus .tkmknmz.input.browse ; break}

    frame .tkmknmz.result
    pack .tkmknmz.result -side top -fill both -expand true
    set mklog [text .tkmknmz.result.log \
        -width $conf(WIDTH) -height [expr $conf(HEIGHT) / 2] -font $conf(WIDGETFONT) \
        -borderwidth 2 -relief sunken -setgrid true -state normal \
        -yscrollcommand {.tkmknmz.result.scroll set}]
    scrollbar .tkmknmz.result.scroll -command {.tkmknmz.result.log yview}
    pack .tkmknmz.result.scroll -side right -fill y
    pack .tkmknmz.result.log -side left -fill both -expand true

    focus .tkmknmz.input.location

    SetColor {.tkmknmz}
}

proc ExecMknmz {} {
    global mklog mkinput mkbut conf

    MakeOptions
    $mklog config -cursor watch
    $mklog delete 1.0 end
    if [catch {open "| $conf(MKNMZ) $conf(MKOPTIONS) \"$conf(MKLOCATION)\""} mkinput] {
        $mklog insert end "$mkinput\n"
    } else {
        fileevent $mkinput readable {
            if [eof $mkinput] {
                BreakMknmz
            } else {
                gets $mkinput mkline
                $mklog insert end "$mkline\n"
                $mklog see end
            }
        }
        set conf(commandline) "%$conf(MKNMZ) $conf(MKOPTIONS) $conf(MKLOCATION)"
        $mklog insert end "$conf(commandline)\n"
        $mkbut config -text {Stop} -command {BreakMknmz}
    }
    $mklog config -cursor {}
}

proc BreakMknmz {} {
    global mkinput mkbut

    catch {close $mkinput}
    $mkbut config -text {Execute} -command {ExecMknmz}
}

proc DirectoryExplorer {} {
    global conf

    set conf(MKLOCATION) [tk_getOpenFile -filetypes {{ALL {*}}}]
    regsub {^(.*/).*} $conf(MKLOCATION) {\1} conf(MKLOCATION)
}

proc WdnmzHelper {} {
    global wdlog wdbut conf

    if [winfo exists .tkwdnmz] {
        focus .tkwdnmz.input.location
        return
    } else {
        toplevel .tkwdnmz
        wm title .tkwdnmz "wdnmz"
    }

    frame .tkwdnmz.input -borderwidth 1
    pack .tkwdnmz.input -side top -fill x
    label .tkwdnmz.input.l -text {Location:} -padx 0 -font $conf(WIDGETFONT)
    entry .tkwdnmz.input.location -width 20 \
        -relief sunken -textvariable conf(WDLOCATION)
    pack .tkwdnmz.input.l -side left
    pack .tkwdnmz.input.location -side left -fill x -expand true
    set wdbut [button .tkwdnmz.input.execute -text {Execute} -font $conf(WIDGETFONT) \
        -underline 1 -command {ExecWdnmz}]
    button .tkwdnmz.input.browse -text {Browse} -font $conf(WIDGETFONT) \
        -underline 0 -command {set conf(WDLOCATION) [tk_getOpenFile \
        -filetypes {{NMZ.i {.i}} {ALL {*}}}]}
    pack .tkwdnmz.input.browse .tkwdnmz.input.execute -side right
    bind .tkwdnmz.input.location <Return> {ExecWdnmz}
    bind .tkwdnmz.input.location <Control-c> {Break}
    bind .tkwdnmz <Alt-x> {focus .tkwdnmz.input.execute ; break}
    bind .tkwdnmz <Alt-b> {focus .tkwdnmz.input.browse ; break}

    frame .tkwdnmz.result
    pack .tkwdnmz.result -side top -fill both -expand true
    set wdlog [text .tkwdnmz.result.log \
        -width [expr $conf(WIDTH) / 2] -height $conf(HEIGHT) -font $conf(WIDGETFONT) \
        -borderwidth 2 -relief sunken -setgrid true -state normal \
        -yscrollcommand {.tkwdnmz.result.scroll set}]
    scrollbar .tkwdnmz.result.scroll -command {.tkwdnmz.result.log yview}
    pack .tkwdnmz.result.scroll -side right -fill y
    pack .tkwdnmz.result.log -side left -fill both -expand true
    focus .tkwdnmz.input.location

    SetColor {.tkwdnmz}
}

proc ExecWdnmz {} {
    global wdlog wdinput wdbut conf

    MakeOptions
    $wdlog config -cursor watch
    $wdlog delete 1.0 end
    if [catch {open "| $conf(WDNMZ) \"$conf(WDLOCATION)\""} wdinput] {
        $wdlog insert end "$wdinput\n"
    } else {
        fileevent $wdinput readable {
            if [eof $wdinput] {
                BreakWdnmz
            } else {
                gets $wdinput wdline
                $wdlog insert end "$wdline\n"
                $wdlog see end
            }
        }
        set conf(commandline) "%$conf(WDNMZ) $conf(WDLOCATION)"
        $wdlog insert end "$conf(commandline)\n"
        $wdbut config -text {Stop} -command {BreakWdnmz}
    }
    $wdlog config -cursor {}
}

proc BreakWdnmz {} {
    global wdinput wdbut

    catch {close $wdinput}
    $wdbut config -text {Execute} -command {ExecWdnmz}
}

################ main routine and .tknmz widget
wm title . "TkNamazu version $version"
wm minsize . 80 5

frame .tknmz -borderwidth 1
pack .tknmz -side top -fill both -expand true

frame .tknmz.menubar
pack .tknmz.menubar -side top -fill x
    menubutton .tknmz.file -text {File} -menu .tknmz.file.menu -underline 0 \
        -font $conf(WIDGETFONT)
    menu .tknmz.file.menu -tearoff no
    .tknmz.file.menu add command -label {Open} -underline 0 \
        -font $conf(WIDGETFONT) -command {OpenViewer}
    .tknmz.file.menu add command -label {Open URL} -underline 5 \
        -font $conf(WIDGETFONT) -command {OpenURL}
    .tknmz.file.menu add command -label {Save} -underline 2 \
        -font $conf(WIDGETFONT) -command {SaveFile}
    .tknmz.file.menu add separator
    .tknmz.file.menu add command -label {Load namazu.conf} -underline 0 \
        -font $conf(WIDGETFONT) -command {LoadConf}
    .tknmz.file.menu add separator
    .tknmz.file.menu add cascade -label {Select DB} -underline 0 \
        -menu .tknmz.file.menu.seldb -font $conf(WIDGETFONT)
    .tknmz.file.menu add command -label {Add DB} -underline 0 \
        -font $conf(WIDGETFONT) -command {AddDB}
    MakeDBLuncher
    .tknmz.file.menu add separator
    .tknmz.file.menu add command -label {Quit} -underline 0 \
        -font $conf(WIDGETFONT) -command {Confirmation}

    menubutton .tknmz.edit -text {Edit} -menu .tknmz.edit.menu -underline 0 \
        -font $conf(WIDGETFONT)
    menu .tknmz.edit.menu -tearoff no
    .tknmz.edit.menu add command -label {Editable} -underline 0 \
        -font $conf(WIDGETFONT) -command {$log config -state normal -relief sunken }
    .tknmz.edit.menu add command -label {Copy} -underline 0 \
        -font $conf(WIDGETFONT) -command {CopyString}
    .tknmz.edit.menu add command -label {Cut} -underline 1 \
        -font $conf(WIDGETFONT) -command {CutString}
    .tknmz.edit.menu add command -label {Paste} -underline 0 \
        -font $conf(WIDGETFONT) -command {PasteString {}}
    .tknmz.edit.menu add separator
    .tknmz.edit.menu add command -label {Find} -underline 0 \
        -font $conf(WIDGETFONT) -command {FindWord}
    .tknmz.edit.menu add separator
    .tknmz.edit.menu add command -label {Set authorization} -underline 4 \
        -font $conf(WIDGETFONT) -command {SetAuthorization}
    .tknmz.edit.menu add separator
    .tknmz.edit.menu add command -label {Display configuration} -underline 0 \
        -font $conf(WIDGETFONT) -command {DisplayConfiguration}
    .tknmz.edit.menu add command -label {Load tknmzrc} -underline 0 \
        -font $conf(WIDGETFONT) -command {LoadTknmzrc}
#    .tknmz.edit.menu add command -label {Option} -underline 0 \
#        -font $conf(WIDGETFONT) -command {TknmzrcHelper}

    menubutton .tknmz.go -text {Go} -menu .tknmz.go.menu -underline 0 \
        -font $conf(WIDGETFONT)
    menu .tknmz.go.menu -tearoff no
    .tknmz.go.menu add command -label {To Head} -underline 3 \
        -font $conf(WIDGETFONT) -command {$log see 1.0}
    .tknmz.go.menu add command -label {To Tail} -underline 3 \
        -font $conf(WIDGETFONT) -command {$log see end}
    .tknmz.go.menu add separator
    .tknmz.go.menu add cascade -label {Bookmark} -underline 0 \
        -menu .tknmz.go.menu.bookmark -font $conf(WIDGETFONT)
    MakeBMLuncher
#    .tknmz.go.menu add separator
#    .tknmz.go.menu add cascade -label {History} -underline 1 \
#        -font $conf(WIDGETFONT) -command {}

    menubutton .tknmz.mknmz -text {Mknmz} -menu .tknmz.mknmz.menu -underline 0 \
        -font $conf(WIDGETFONT)
    menu .tknmz.mknmz.menu -tearoff no
    .tknmz.mknmz.menu add command -label {Execute mknmz} -underline 0 \
        -font $conf(WIDGETFONT) -command {MknmzHelper}

    menubutton .tknmz.wdnmz -text {Wdnmz} -menu .tknmz.wdnmz.menu -underline 0 \
        -font $conf(WIDGETFONT)
    menu .tknmz.wdnmz.menu -tearoff no
    .tknmz.wdnmz.menu add command -label {Execute wdnmz} -underline 0 \
        -font $conf(WIDGETFONT) -command {WdnmzHelper}
    pack .tknmz.file .tknmz.edit .tknmz.go .tknmz.mknmz .tknmz.wdnmz \
        -side left -in .tknmz.menubar

    menubutton .tknmz.help -text {Help} -menu .tknmz.help.menu -underline 0 \
        -font $conf(WIDGETFONT)
    menu .tknmz.help.menu -tearoff no
    .tknmz.help.menu add command -label {Help} -underline 0 \
        -font $conf(WIDGETFONT) -command {HelpView}
    .tknmz.help.menu add command -label {Version} -underline 0 \
        -font $conf(WIDGETFONT) -command {AboutTknamazu}
    pack .tknmz.help -side right -in .tknmz.menubar

frame .tknmz.input
pack .tknmz.input -side top -fill x
label .tknmz.input.l -text {Keyword:} -padx 0 -font $conf(WIDGETFONT) -underline 0
entry .tknmz.keyword -width 20 -relief sunken -textvariable keyword
pack .tknmz.input.l -side left
pack .tknmz.keyword -side left -fill x -expand true -in .tknmz.input
set but [button .tknmz.input.search -text {Search} -font $conf(WIDGETFONT) \
     -underline 0 -command {Exec $keyword}]
button .tknmz.input.quit -text {Quit} -font $conf(WIDGETFONT) \
     -underline 0 -command {Confirmation}
pack .tknmz.input.quit .tknmz.input.search -side right

frame .tknmz.option
pack .tknmz.option -side top -fill x
image create photo logo -file "$conf(tknmzhome)/tknamazu.gif"
button .tknmz.option.logo -image logo -command {HelpView ; Jump {0} }
pack .tknmz.option.logo -side right
label .tknmz.option.dlabel -text {Display Count:} -font $conf(WIDGETFONT) -underline 8
tk_optionMenu .tknmz.option.display opt(n) 10 20 30 50 100 ALL
pack .tknmz.option.dlabel .tknmz.option.display -side left -fill x
label .tknmz.option.slabel -text {Summary:} -font $conf(WIDGETFONT) -underline 1
tk_optionMenu .tknmz.option.summary opt(s) ON OFF
pack .tknmz.option.slabel .tknmz.option.summary -side left -fill x

frame .tknmz.result
pack .tknmz.result -side top -fill both -expand true
set log [text .tknmz.result.log -width $conf(WIDTH) -height $conf(HEIGHT) \
    -borderwidth 1 -relief raised -setgrid true -font $conf(TEXTFONT) \
    -state disabled -yscrollcommand {.tknmz.result.scroll set}]
scrollbar .tknmz.result.scroll -command {.tknmz.result.log yview} 
pack .tknmz.result.scroll -side right -fill y
pack .tknmz.result.log -side left -fill both -expand true

menu .popupmenu -tearoff no
.popupmenu add command -label {Search this} -underline 0 \
    -command {FastExec} -font $conf(WIDGETFONT)
.popupmenu add separator
.popupmenu add command -label {Copy and Paste to Keyword} -underline 18 \
    -command {CopyString ; PasteString {.tknmz.keyword}} -font $conf(WIDGETFONT)
.popupmenu add command -label {Copy and Paste to Findword} -underline 18 \
    -command {FindWord ; CopyString ; PasteString {.prompt.entry}} -font $conf(WIDGETFONT)
.popupmenu add command -label {Copy this URL} -underline 11 \
    -command {CopyURL} -font $conf(WIDGETFONT)
.popupmenu add separator
.popupmenu add command -label {Copy} -underline 0 \
    -command {CopyString} -font $conf(WIDGETFONT)
.popupmenu add command -label {Paste} -underline 0 \
    -command {PasteString {}} -font $conf(WIDGETFONT)
.popupmenu add command -label {Cut} -underline 1 \
    -command {CutString} -font $conf(WIDGETFONT)
.popupmenu add separator
.popupmenu add command -label {External Browser or Viewer} -underline 1 \
    -command {Kick $urlforext} -font $conf(WIDGETFONT)
.popupmenu add separator
.popupmenu add command -label {To Head} -underline 3 \
    -command {$log see 1.0} -font $conf(WIDGETFONT)
.popupmenu add command -label {To Tail} -underline 3 \
    -command {$log see end} -font $conf(WIDGETFONT)
.popupmenu add command -label {Editable} -underline 0 \
    -command {$log config -state normal -relief sunken } -font $conf(WIDGETFONT)
.popupmenu add separator
.popupmenu add command -label {Quit} -underline 0 \
   -command {Confirmation} -font $conf(WIDGETFONT)

HelpView
focus .tknmz.keyword

SetColor {.tknmz}

bind . <Control-f> {FindWord}
bind . <Alt-k> {focus .tknmz.keyword ; break}
bind . <Alt-s> {focus .tknmz.input.search ; break}
bind . <Alt-b> {focus .tknmz.input.search ; break}
bind . <Alt-q> {focus .tknmz.input.quit ; break}
bind . <Alt-c> {focus .tknmz.option.display ; break}
bind . <Alt-u> {focus .tknmz.option.summary ; break}
bind . <Button-2> {Kick {}}
bind . <Button-3> {PopupMenu %X %Y}
bind . <Control-u> {PopupMenu %X %Y}
bind . <Control-n> {ScrollDownResult 1 unit}
bind . <Control-p> {ScrollDownResult -1 unit}
bind . <Control-v> {ScrollDownResult 1 page}
bind . <Control-z> {ScrollDownResult -1 page}

if {![string compare $conf(platform) "unix"] && ![string compare $conf(LANGUAGE) "JAPANESE"] \
        && ($conf(tkversion) < 8.1) && ![regexp -nocase {\-L ENGLISH} $conf(OPTIONS)]} {
    bind Entry <Control-o> {kinput_start %W over}
    bind Entry <Shift-space> {kinput_start %W over}
}

bind .tknmz.keyword <Return> {Exec $keyword}
bind .tknmz.result.log <Button-1> {focus .tknmz.result.scroll}
bind .tknmz.result.scroll <space> {ScrollDownResult 1 page}
bind .tknmz.result.scroll <BackSpace> {ScrollDownResult -1 page}
bind .tknmz.result.scroll <Return> {ScrollDownResult 1 unit}

if {$tk_version < 4.0} {
    puts \
"Warning: the script are based on Tk 8.1a2.  You have
Tk $tk_version, so script may not work.

Please see <URL:http://sunscript.sun.com/TclTkCore/>
"
}
