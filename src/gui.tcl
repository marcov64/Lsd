#*************************************************************
#
#	LSD 9.0 - January 2024
#	written by Marco Valente, Universita' dell'Aquila
#	and by Marcelo Pereira, University of Campinas
#
#	Copyright Marco Valente and Marcelo Pereira
#	LSD is distributed under the GNU General Public License
#
#	See Readme.txt for copyright information of
#	third parties' code used in LSD
#
#*************************************************************

#*************************************************************
# GUI.TCL
# LSD Graphical User Interface code and definitions.
#
# The module sets all the required variables, data structures
# and code required to create and operate the LSD GUI over
# Tk/ttk.
# To ensure smooth operation of LSD GUI, all Tk/ttk new
# windows should be created using the procedures defined
# in WINDOW.TCL.
#*************************************************************

package require Tk 8.6

#************************************************
# INITIAL CONFIGURATION
# The code below is run once when LMM/LSD starts
#************************************************

# load extra code
lappend auto_path "$lsd_root/$lsd_src/themes"

if { ! [ info exists CurPlatform ] } {
	source "$lsd_root/$lsd_src/defaults.tcl" ;	# load LSD defaults if not yet
}

source "$lsd_root/$lsd_src/window.tcl" ;		# load LSD gui management
source "$lsd_root/$lsd_src/theme.tcl" ;			# load LSD gui theming
source "$lsd_root/$lsd_src/tklib/wgtclone.tcl" ;# load LSD widget cloning tools
source "$lsd_root/$lsd_src/tklib/tooltip.tcl" ;	# tklib tootip management
source "$lsd_root/$lsd_src/tklib/dblclick.tcl" ;# enhancements to double-click

# optional development tools
set conWnd		false ;	# enable console window to be opened with CTRL+ALT+J
set logWndFn	false ;	# enable window functions operation logging
set testWnd		false ;	# enable coordinates test window

# register static special, OS-dependent configurations
if [ string equal $CurPlatform mac ] {

	# enable Ctrl+click as replacement for right-shift
	bind all <Control-ButtonPress-1> {
		event generate %W <ButtonPress-2> -x %x -y %y -rootx %X -rooty %Y -button 2
	}

	# ensure homebrew local executables are on PATH
	if { [ string first "/usr/local/bin" "$env(PATH)" ] < 0 } {
		set env(PATH) "/usr/local/bin:$env(PATH)"
	}
} elseif [ string equal $CurPlatform linux ] {

	# use xterm as alternative for missing default/alternative terminals
	if { [ catch { exec which [ lindex $DefaultSysTerm 0 ] } ] } {
		set DefaultSysTerm "xterm -e"
		foreach term $sysTermLinuxAlt {
			if { ! [ catch { exec which [ lindex $term 0 ] } ] } {
				set DefaultSysTerm $term
				break
			}
		}
	}
} elseif [ string equal $CurPlatform windows ] {
	package require registry

	# inherit OS setting
	set mouseWarp [ ismousesnapon $CurPlatform ]

	# Cygwin or MSYS2?
	if { [ catch { exec where cygwin1.dll } ] || [ catch { exec where cygintl-8.dll } ] } {
		if { ! [ catch { exec where $makeWinMingw } ] } {
			set DefaultMakeExe $makeWinMingw
		}
	} else {
		if { ! [ catch { exec where $makeWinCygwin } ] } {
			set DefaultMakeExe $makeWinCygwin
		}
	}

	# Gnuplot on path? if not, try default install folder
	if [ catch { exec where $gnuplotExe } ] {
		if [ file exists "C:/Program Files/gnuplot/bin/$gnuplotExe" ] {
			set gnuplotExe "\"C:/Program Files/gnuplot/bin/$gnuplotExe\""
		}
	}
}

# check old incompatible options and fix with defaults
if { ! [ info exists sys_term ] || ( $CurPlatform in [ list linux windows ] && [ llength $sys_term ] < 2 ) } {
	set sys_term $DefaultSysTerm
}

if { ! [ info exists wish_exe ] || ( $CurPlatform eq "mac" && $wish_exe eq "wish8.6" ) } {
	set wish_exe $DefaultWish
}

if { ! [ info exists html_browser ] || $html_browser eq "open" } {
	set html_browser $DefaultHtmlBrowser
}

# detect and update OS-dependent current/default theme configurations
updateTheme

# define dictionaries of themes available
foreach theme [ array names themeTable ] {
	set themePlatform [ lindex $themeTable($theme) 0 ]
	if { [ string equal $themePlatform $CurPlatform ] || [ string equal $themePlatform all ] } {
		set themeName [ lindex $themeTable($theme) 2 ]
		lappend themeNames $themeName
		dict set themeToName $theme $themeName
		dict set nameToTheme $themeName $theme
	}
}
set themeNames [ lsort $themeNames ]

# try to set tk theme (ttk), falling back to the set default
if { ! [ info exists lsd_theme ] || \
	 [ array names themeTable -exact $lsd_theme ] == "" || \
	 [ catch { package require [ lindex $themeTable($lsd_theme) 1 ] } ] ||
	 [ catch { ttk::style theme use $lsd_theme } ] } {
	if { [ array names themeTable -exact $DefaultTheme ] == "" || \
		 [ catch { package require [ lindex $themeTable($DefaultTheme) 1 ] } ] || \
		 [ catch { ttk::style theme use $DefaultTheme } ] } {
		set lsd_theme [ ttk::style theme use ]
	} else {
		set lsd_theme $DefaultTheme
	}
}

# define dark mode based on theme except on mac system-managed native theme
# also use special aqua theme-automatic colors
if { [ string equal $lsd_theme aqua ] } {
	set darkTheme [ isDarkTheme ]

	set colorsTheme(bg) systemWindowBackgroundColor					; # non-entry light/dark text background
	set colorsTheme(fg) systemTextColor								; # entry/non-entry dark text foreground
	set colorsTheme(dbg) systemTextBackgroundColor					; # entry dark text background
	set colorsTheme(ebg) systemTextBackgroundColor					; # entry light text background
	set colorsTheme(efg) systemTextColor							; # entry light text foreground
	set colorsTheme(sbg) systemSelectedTextBackgroundColor			; # selected text background
	set colorsTheme(sfg) systemSelectedTextColor					; # selected text foreground
} else {
	set darkTheme [ lindex $themeTable($lsd_theme) 3 ]
	set colorsTheme(bg) [ ttk::style lookup . -background ]			; # non-entry light/dark text background
	set colorsTheme(fg) [ ttk::style lookup . -foreground ]			; # entry/non-entry dark text foreground
	set colorsTheme(dbg) [ ttk::style lookup . -troughcolor ]		; # entry dark text background
	set colorsTheme(ebg) [ ttk::style lookup . -fieldbackground ]	; # entry light text background
	set colorsTheme(efg) [ ttk::style lookup . -insertcolor ]		; # entry light text foreground
	set colorsTheme(sbg) [ ttk::style lookup . -selectbackground ]	; # selected text background
	set colorsTheme(sfg) [ ttk::style lookup . -selectforeground ]	; # selected text foreground
}

# get remaining basic colors from set theme
set colorsTheme(dfg) [ ttk::style lookup . -foreground disabled ]
set colorsTheme(isbg) [ ttk::style lookup . -lightcolor ]
set colorsTheme(hc) [ ttk::style lookup . -selectbackground ]

# fix missing colors
if { $colorsTheme(ebg) == "" } {
	set colorsTheme(ebg) [ ttk::style lookup TEntry -selectbackground ]
}
if { $colorsTheme(efg) == "" } {
	set colorsTheme(efg) [ ttk::style lookup TEntry -selectforeground ]
}
if { $colorsTheme(dbg) == "" } {
	set colorsTheme(dbg) [ ttk::style lookup Treeview -background ]
}
if { $colorsTheme(isbg) == "" } {
	set colorsTheme(isbg) [ ttk::style lookup . -background ]
}

foreach color [ array names colorsTheme ] {
	if { $colorsTheme($color) == "" || [ catch { winfo rgb . $colorsTheme($color) } ] } {
		if { $color in [ list sbg ebg dbg isbg ] } {
			set colorsTheme($color) $colorsTheme(bg)
		}
		if { $color in [ list sfg efg dfg hc ] } {
			set colorsTheme($color) $colorsTheme(fg)
		}
		tk_messageBox -icon warning -title Warning -message "Incomplete color palette" -detail "Color '$color' is missing or invalid in current theme '$lsd_theme', replacing with gray shade."
	}
}

# adjust between dark and light desktop modes
if { $darkTheme } {
	set defcolors $defcolorsD
	set colorsTheme(hl) $hlcolorD
	set colorsTheme(dhl) $dhlcolorD
	set colorsTheme(comm) $commcolorD
	set colorsTheme(str) $strcolorD
	set colorsTheme(prep) $prepcolorD
	set colorsTheme(type) $typecolorD
	set colorsTheme(kwrd) $kwrdcolorD
	set colorsTheme(vlsd) $vlsdcolorD
	set colorsTheme(mlsd) $mlsdcolorD
	set colorsTheme(par) $parcolorD
	set colorsTheme(var) $varcolorD
	set colorsTheme(lvar) $lvarcolorD
	set colorsTheme(fun) $funcolorD
	set colorsTheme(lfun) $lfuncolorD
	set colorsTheme(obj) $objcolorD
	set colorsTheme(grp) $grpcolorD
	set colorsTheme(mod) $modcolorD
	set colorsTheme(ttip) $ttipcolorD
} else {
	set defcolors $defcolorsL
	set colorsTheme(hl) $hlcolorL
	set colorsTheme(dhl) $dhlcolorL
	set colorsTheme(comm) $commcolorL
	set colorsTheme(str) $strcolorL
	set colorsTheme(prep) $prepcolorL
	set colorsTheme(type) $typecolorL
	set colorsTheme(kwrd) $kwrdcolorL
	set colorsTheme(vlsd) $vlsdcolorL
	set colorsTheme(mlsd) $mlsdcolorL
	set colorsTheme(par) $parcolorL
	set colorsTheme(var) $varcolorL
	set colorsTheme(lvar) $lvarcolorL
	set colorsTheme(fun) $funcolorL
	set colorsTheme(lfun) $lfuncolorL
	set colorsTheme(obj) $objcolorL
	set colorsTheme(grp) $grpcolorL
	set colorsTheme(mod) $modcolorL
	set colorsTheme(ttip) $ttipcolorL
}

# set tool tips (balloons)
if { [ catch { set ttfam [ font actual [ ttk::style lookup TLabel -font ] -family ] } ] || [ catch { set ttsize [ expr { [ font actual [ ttk::style lookup TLabel -font ] -size ] - 1 } ] } ] } {
	set ttfam [ font actual TkDefaultFont -family ]
	set ttsize [ expr { [ font actual TkDefaultFont -size ] - 1 } ]
}
set ttfont [ font create -family "$ttfam" -size $ttsize ]
set ttfontB [ font create -family "$ttfam" -size $ttsize -weight bold ]
set tooltip::labelOpts [ list -background $colorsTheme(ttip) -foreground $colorsTheme(fg) \
	 -borderwidth 0 -highlightthickness 1 -highlightbackground $colorsTheme(fg) ]
tooltip::tooltip delay $ttipdelay
tooltip::tooltip fade $ttipfade

# Variable 'alignMode' configure special, per module (LMM, LSD), settings
unset -nocomplain defaultPos defaultFocus
if [ info exists alignMode ] {
	if [ string equal -nocase $alignMode "LMM" ] {
		set defaultPos centerW
		set defaultFocus .f.t.t
	} else {
		set defaultPos centerW
	}
}

# lists to hold the windows parents stacks and exceptions to the parent mgmt.
set parWndLst [ list ]
set grabLst [ list ]
set noParLst [ list .log .str .deb .lat .plt .dap .tst ]

# list of windows with predefined sizes & positions
set wndLst [ list .lmm .lsd .log .str .da .deb .lat .plt .dap ]
set wndMenuHeight 0

# text line default canvas height & minimum horizontal border width
set lheightP [ expr { int( [ font actual $fontP -size ] * [ tk scaling ] ) + $vtmarginP } ]
set hbordsizeP	$hmbordsizeP

# current position of structure window
set posXstr 0
set posYstr 0

# load and set console configuration
if $conWnd {
	set msg "File(s) missing or corrupted"
	set det "Tcl/Tk console file 'tkcon.tcl' is missing or corrupted.\nPlease check your installation and reinstall LSD if the problem persists.\n\nLSD is continuing without console support."
	if [ file exists "$lsd_root/$lsd_src/tklib/tkcon.tcl" ] {
		if { [ catch { source "$lsd_root/$lsd_src/tklib/tkcon.tcl" } ] == 0 } {
			set tkcon::PRIV(showOnStartup) 0
			set tkcon::PRIV(root) .console
			set tkcon::PRIV(protocol) { tkcon hide }
			set tkcon::OPT(exec) ""
		} else {
			set conWnd false
			ttk::messageBox -type ok -icon warning -title Warning -message $msg -detail $det
		}
	} else {
		set conWnd false
			ttk::messageBox -type ok -icon warning -title Warning -message $msg -detail $det
	}
}

# open test window if enabled
if $testWnd {
	newtop .tst "LSD Coordinates Test Window" { destroytop .tst } ""

	ttk::frame .tst.xy
	ttk::label .tst.xy.l1 -anchor e -text "X:"
	ttk::label .tst.xy.v1 -anchor w -style hl.TLabel
	ttk::label .tst.xy.l2 -anchor e -text "   Y:"
	ttk::label .tst.xy.v2 -anchor w -style hl.TLabel
	pack .tst.xy.l1 .tst.xy.v1 .tst.xy.l2 .tst.xy.v2 -side left -padx $::_2 -pady $::_2

	ttk::frame .tst.r
	ttk::label .tst.r.l1 -anchor e -text "rootx:"
	ttk::label .tst.r.v1 -anchor w -style hl.TLabel
	ttk::label .tst.r.l2 -anchor e -text "   rooty:"
	ttk::label .tst.r.v2 -anchor w -style hl.TLabel
	pack .tst.r.l1 .tst.r.v1 .tst.r.l2 .tst.r.v2 -side left -padx $::_2 -pady $::_2

	ttk::frame .tst.v
	ttk::label .tst.v.l1 -anchor e -text "vrootx:"
	ttk::label .tst.v.v1 -anchor w -style hl.TLabel
	ttk::label .tst.v.l2 -anchor e -text "   vrooty:"
	ttk::label .tst.v.v2 -anchor w -style hl.TLabel
	pack .tst.v.l1 .tst.v.v1 .tst.v.l2 .tst.v.v2 -side left -padx $::_2 -pady $::_2

	ttk::frame .tst.s
	ttk::label .tst.s.l1 -anchor e -text "screenwidth:"
	ttk::label .tst.s.v1 -anchor w -style hl.TLabel
	ttk::label .tst.s.l2 -anchor e -text "   screenheight:"
	ttk::label .tst.s.v2 -anchor w -style hl.TLabel
	pack .tst.s.l1 .tst.s.v1 .tst.s.l2 .tst.s.v2 -side left -padx $::_2 -pady $::_2

	ttk::frame .tst.t
	ttk::label .tst.t.l1 -anchor e -text "vrootwidth:"
	ttk::label .tst.t.v1 -anchor w -style hl.TLabel
	ttk::label .tst.t.l2 -anchor e -text "   vrootheight:"
	ttk::label .tst.t.v2 -anchor w -style hl.TLabel
	pack .tst.t.l1 .tst.t.v1 .tst.t.l2 .tst.t.v2 -side left -padx $::_2 -pady $::_2

	ttk::frame .tst.m
	ttk::label .tst.m.l1 -anchor e -text "maxwidth:"
	ttk::label .tst.m.v1 -anchor w -style hl.TLabel
	ttk::label .tst.m.l2 -anchor e -text "   maxheight:"
	ttk::label .tst.m.v2 -anchor w -style hl.TLabel
	pack .tst.m.l1 .tst.m.v1 .tst.m.l2 .tst.m.v2 -side left -padx $::_2 -pady $::_2

	pack .tst.xy .tst.r .tst.v .tst.s .tst.t .tst.m

	bind .tst <Motion> {
		.tst.xy.v1 configure -text %X
		.tst.xy.v2 configure -text %Y
		.tst.r.v1 configure -text [ winfo rootx .tst ]
		.tst.r.v2 configure -text [ winfo rooty .tst ]
		.tst.v.v1 configure -text [ winfo vrootx .tst ]
		.tst.v.v2 configure -text [ winfo vrooty .tst ]
		.tst.s.v1 configure -text [ winfo screenwidth .tst ]
		.tst.s.v2 configure -text [ winfo screenheight .tst ]
		.tst.t.v1 configure -text [ winfo vrootwidth .tst ]
		.tst.t.v2 configure -text [ winfo vrootheight .tst ]
		.tst.m.v1 configure -text [ lindex [ wm maxsize .tst ] 0 ]
		.tst.m.v2 configure -text [ lindex [ wm maxsize .tst ] 1 ]
	}

	showtop .tst current yes yes no
}


#		ttk::messageBox -message "lsd_theme:$lsd_theme x [ ttk::style theme use ]\n\nThemes:\n[ ttk::style theme names ]\n\nthemeList:\n[ array names themeTable ]\n\npackages:\n[ package names ]\n\nColors:\n[ array get colorsTheme ]\n\nStyles:\n[ ttk::style element names ]"



