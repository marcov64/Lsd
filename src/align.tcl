# ****************************************************
# ****************************************************
# LSD 7.0 - August 2015
# written by Marco Valente
# Universita' dell'Aquila
# 
# Copyright Marco Valente
# Lsd is distributed according to the GNU Public License
# 
# Comments and bug reports to marco.valente@univaq.it
# ****************************************************


# ****************************************************
# Procedures to adjust window positioning. 
#	Types of window positioning:
#	centerS: center over the primary display, only available if the parent window center is also in the primary display (if not, falback to centerW)
#	centerW: center over the parent window (in any display)
#	topleftS: put over the top left corner of screen of primary display, only available if the parent window center is also in the primary display (if not, falback to topleftW)
#	topleftW: put over the top left corner of parent window (around menu bar)
#	coverW: cover the parent window (same size & position)
#	overM: over the main window (same top-left position)
#	current: keep current position
#
# ****************************************************

package require Tk 8.5

# Main windows size and margins (must be even numbers)
set hmargin		20	; # horizontal right margin from the screen borders
set vmargin		20	; # vertical margins from the screen borders
set bordsize	2	; # width of windows borders
set tbarsize	55	; # size in pixels of bottom taskbar (exclusion area) - Windows 7+ = 82
set hsizeL		800	; # LMM horizontal size in pixels
set vsizeL		600	; # LMM vertical size in pixels
set hsizeLmin	600	; # LMM minimum horizontal size in pixels
set vsizeLmin	300	; # LMM minimum vertical size in pixels
set hsizeB 		400	; # browser horizontal size in pixels
set vsizeB		620	; # browser vertical size in pixels
set hsizeM 		600	; # model structure horizontal size in pixels
set vsizeM		400	; # model structure vertical size in pixels
set hsizeI 		800	; # initial values editor horizontal size in pixels
set vsizeI		600	; # initial values editor vertical size in pixels
set hsizeN 		350	; # objects numbers editor horizontal size in pixels
set vsizeN		550	; # objects numbers editor vertical size in pixels

# OS specific screen location offset adjustments
set corrXmac	0
set corrYmac	0
set corrXlinux	0
set corrYlinux	-47
set corrXwindows 0
set corrYwindows 0

# list of windows with predefined sizes & positions
set wndLst [ list lsd lmm log str ]

# Enable window functions operation logging
set logWndFn	false


# toolbar buttons style
if [ string equal [ tk windowingsystem ] aqua ] { 
	set bRlf ""
	set ovBrlf "" 
} { 
	set bRlf flat
	set ovBrlf groove 
}

# toolbar images format
if [ string equal [ info tclversion ] 8.6 ] { 
	set iconExt png 
} { 
	set iconExt gif 
}

# register static special configurations
if [ string equal $tcl_platform(platform) unix ] {
	if [ string equal $tcl_platform(os) Darwin ] {
		set corrX $corrXmac
	} {
		set corrX $corrXlinux
	}
} {
	set corrX $corrXwindows
}
if [ string equal $tcl_platform(platform) unix ] {
	if [ string equal $tcl_platform(os) Darwin ] {
		set corrY $corrYmac
	} {
		set corrY $corrYlinux
	}
} {
	set corrY $corrYwindows
}

# current position of structure window
set posXstr 0
set posYstr 0

# images format according to Tk version
if [ string equal [ info tclversion ] 8.6 ] { 
	set iconExt png 
} { 
	set iconExt gif 
}

# load images
image create photo lsdImg -file "$RootLsd/$LsdSrc/icons/lsd.$iconExt"
image create photo lmmImg -file "$RootLsd/$LsdSrc/icons/lmm.$iconExt"
image create photo openImg -file "$RootLsd/$LsdSrc/icons/open.$iconExt"
image create photo saveImg -file "$RootLsd/$LsdSrc/icons/save.$iconExt"
image create photo undoImg -file "$RootLsd/$LsdSrc/icons/undo.$iconExt"
image create photo redoImg -file "$RootLsd/$LsdSrc/icons/redo.$iconExt"
image create photo cutImg -file "$RootLsd/$LsdSrc/icons/cut.$iconExt"
image create photo copyImg -file "$RootLsd/$LsdSrc/icons/copy.$iconExt"
image create photo pasteImg -file "$RootLsd/$LsdSrc/icons/paste.$iconExt"
image create photo findImg -file "$RootLsd/$LsdSrc/icons/find.$iconExt"
image create photo replaceImg -file "$RootLsd/$LsdSrc/icons/replace.$iconExt"
image create photo indentImg -file "$RootLsd/$LsdSrc/icons/indent.$iconExt"
image create photo deindentImg -file "$RootLsd/$LsdSrc/icons/deindent.$iconExt"
image create photo comprunImg -file "$RootLsd/$LsdSrc/icons/comprun.$iconExt"
image create photo compileImg -file "$RootLsd/$LsdSrc/icons/compile.$iconExt"
image create photo infoImg -file "$RootLsd/$LsdSrc/icons/info.$iconExt"
image create photo descrImg -file "$RootLsd/$LsdSrc/icons/descr.$iconExt"
image create photo equationImg -file "$RootLsd/$LsdSrc/icons/equation.$iconExt"
image create photo setImg -file "$RootLsd/$LsdSrc/icons/set.$iconExt"
image create photo hideImg -file "$RootLsd/$LsdSrc/icons/hide.$iconExt"
image create photo helpImg -file "$RootLsd/$LsdSrc/icons/help.$iconExt"
image create photo reloadImg -file "$RootLsd/$LsdSrc/icons/reload.$iconExt"
image create photo structImg -file "$RootLsd/$LsdSrc/icons/struct.$iconExt"
image create photo initImg -file "$RootLsd/$LsdSrc/icons/init.$iconExt"
image create photo numberImg -file "$RootLsd/$LsdSrc/icons/number.$iconExt"
image create photo runImg -file "$RootLsd/$LsdSrc/icons/run.$iconExt"
image create photo dataImg -file "$RootLsd/$LsdSrc/icons/data.$iconExt"
image create photo resultImg -file "$RootLsd/$LsdSrc/icons/result.$iconExt"

# Variable 'alignMode' configure special, per module (LMM, LSD), settings
unset -nocomplain defaultPos defaultFocus
if [ info exists alignMode ] {
	if [ string equal -nocase $alignMode "LMM" ] {
		set defaultPos centerW
		set defaultFocus .f.t.t
	}
}

# lists to hold the windows parents stacks and exceptions to the parent mgmt.
set parWndLst [ list ]
set grabLst [ list ]
set noParLst [ list .log .str .plt .lat ]

# procedures for create, update and destroy top level new windows
proc newtop { w { name "" } { destroy { } } { par "." } } {
	global tcl_platform RootLsd LsdSrc parWndLst grabLst noParLst logWndFn

	if [ winfo exists $w ] { 
		destroytop $w
	}
	toplevel $w
	if { $par != "" } {
		if { $par != "." } {
			if { [ winfo viewable [ winfo toplevel $par ] ] } {
				wm transient $w $par 
			}
		} {
			if { [ lsearch $noParLst [ string range $w 0 3 ] ] < 0 } {
				while { [ llength $parWndLst ] > 0 && ! [ winfo exists [ lindex $parWndLst 0 ] ] } {
						set parWndLst [ lreplace $parWndLst 0 0 ]
					}
				if { [ llength $parWndLst ] > 0 && ! [ string equal [ lindex $parWndLst 0 ] $w ] } {
					if { [ winfo viewable [ winfo toplevel [ lindex $parWndLst 0 ] ] ] } {
						wm transient $w [ lindex $parWndLst 0 ] 
					}
				} {
					wm transient $w .
				}
			} {
				wm transient $w .
			}
		} 
	}
	wm group $w .
	wm title $w $name
	wm protocol $w WM_DELETE_WINDOW $destroy
	if { $logWndFn && [ info procs plog ] != "" } { plog "\nnewtop (w:$w, master:[wm transient $w], parWndLst:$parWndLst, grab:$grabLst)" } 
}

proc settop { w { name no } { destroy no } { par no } } {
	global parWndLst grabLst logWndFn

	if { $par != no } {
		if { [ winfo viewable [ winfo toplevel $par ] ] } {
			wm transient $w $par 
		}
	}
	if { $name != no } {
		wm title $w $name 
	}
	if { $destroy != no } {
		wm protocol $w WM_DELETE_WINDOW $destroy 
	}
	update
	if { ! [ winfo viewable [ winfo toplevel $w ] ] } {
		wm deiconify $w
	}
	raise $w
	if { [ info exists buttonF ] && [ winfo exists $w.$buttonF.ok ] } {
		focus $w.$buttonF.ok
	} {
		focus $w
	}
	update 
	
	if { $logWndFn && [ info procs plog ] != "" } { plog "\nsettop (w:$w, master:[wm transient $w], pos:([winfo x $w],[winfo y $w]), size:[winfo width $w]x[winfo height $w], parWndLst:$parWndLst, grab:$grabLst)" } 
}

# configure the window
proc showtop { w { pos none } { resizeX no } { resizeY no } { grab yes } { sizeX 0 } { sizeY 0 } { buttonF b } { noMinSize no } } {
	global defaultPos wndLst parWndLst grabLst noParLst logWndFn
	
	#handle main windows differently
	if { [ lsearch $wndLst .$w ] < 0 } {
		# unknown window (not a main one)
		if { ! [ string equal $pos xy ] && $sizeX != 0 } {
			$w configure -width $sizeX 
		}
		if { ! [ string equal $pos xy ] && $sizeY != 0 } {
			$w configure -height $sizeY 
		}
		
		update idletasks
		
		# handle different window default position
		if [ string equal $pos none ] {
			if [ info exists defaultPos ] {
				set pos $defaultPos
			} {
				set pos centerS
			}
		}
		
		if { ! [ string equal $pos current ] } {
			if { [ string equal $pos centerS ] && ! [ primdisp [ winfo parent $w ] ] } {
				if [ info exists defaultPos ] {
					set pos $defaultPos
				} {
					set pos centerS
				}
			}
			
			if { ! [ string equal $pos xy ]	} {
				set x [ getx $w $pos ]
				set y [ gety $w $pos ]
			} {
				set x $sizeX
				set y $sizeY
			}
			
			if { ! [ string equal "" $x ] && ! [ string equal "" $y ] } {
				if { [ string equal $pos coverW ] } {
					set sizeX [ expr [ winfo width [ winfo parent $w ] ] + 10 ]
					set sizeY [ expr [ winfo height [ winfo parent $w ] ] + 30 ]
				}
				if { ! [ string equal $pos xy ]	&& $sizeX != 0 && $sizeY != 0 } {
					wm geom $w ${sizeX}x${sizeY}+$x+$y 
				} {
					wm geom $w +$x+$y
				}
			}
		}
		
		if { ! $noMinSize && ( $resizeX || $resizeY ) } {
			wm minsize $w [ winfo width $w ] [ winfo height $w ]
		}
		
		wm resizable $w $resizeX $resizeY
		if { [ lsearch $noParLst [ string range $w 0 3 ] ] < 0 } {
			set parWndLst [ linsert $parWndLst 0 $w ]
			
			if $grab {
				if { ! [ info exists grabLst ] || [ lsearch -glob $grabLst "$w *" ] < 0 } {
					lappend grabLst "$w [ grab current $w ]"
				}
				grab set $w
			}
		}
	} {
		#known windows - simply apply defaults
		sizetop .$w
	}
	
	if { ! [ winfo viewable [ winfo toplevel $w ] ] } {
		wm deiconify $w
	}
	raise $w
	if { [ info exists buttonF ] && [ winfo exists $w.$buttonF.ok ] } {
		focus $w.$buttonF.ok
	} {
		focus $w
	}
	update
	
	if { $logWndFn && [ info procs plog ] != "" } { plog "\nshowtop (w:$w, master:[wm transient $w], pos:([winfo x $w],[winfo y $w]), size:[winfo width $w]x[winfo height $w], parWndLst:$parWndLst, grab:$grabLst)" } 
}

proc destroytop w {
	global defaultFocus parWndLst grabLst noParLst logWndFn

	if { ! [ winfo exists $w ] } return
	
	if { [ lsearch $noParLst [ string range $w 0 3 ] ] < 0 } {
		if [ info exists grabLst ] {
			set igrab [ lsearch -glob $grabLst "$w *" ]
			if { $igrab >= 0 } {
				grab release $w
				set grabPar [ string range [ lindex $grabLst $igrab ] [ expr [ string first " " [ lindex $grabLst $igrab ] ] + 1 ] end ]
				if { $grabPar != "" } {
					grab set $grabPar 
				}
				set grabLst [ lreplace $grabLst $igrab $igrab ]
			}
		}
		if { [ llength $parWndLst ] > 0 } {
			set parWndLst [ lreplace $parWndLst 0 0 ] 
		}
	}
	# handle different window default focus on destroy
	if [ info exists defaultFocus ] {
		focus $defaultFocus
	} {
		focus [ winfo parent $w ]
	}
	destroy $w
	update

	if { $logWndFn && [ info procs plog ] != "" } { plog "\ndestroytop (w:$w, parWndLst:$parWndLst, grab:$grabLst)" }
}

# size the window to default size & positions
proc sizetop { w } {
	global wndLst hsizeB vsizeB hsizeL vsizeL hsizeLmin vsizeLmin bordsize hmargin vmargin tbarsize posXstr posYstr hsizeM vsizeM logWndFn

	update idletasks
	
	foreach wnd $wndLst {
		if { ! [ string compare $w all ] || ! [ string compare $w $wnd ] } {
		
			switch $wnd {
				lsd {
					wm geometry . "${hsizeB}x$vsizeB+[ getx . topleftS ]+[ gety . topleftS ]"
					wm minsize . $hsizeB [ expr $vsizeB / 2 ]
				}
				lmm {
					if { [ expr [ winfo screenwidth . ] ] < ( $hsizeL + 2 * $bordsize ) } {
						set W [ expr [winfo screenwidth . ] - 2 * $bordsize ] 
					} {
						set W $hsizeL
					}
					set H [ expr [ winfo screenheight . ] - $tbarsize - 2 * $vmargin - 2 * $bordsize ]
					if { $H < $vsizeL } {
						set H [ expr [ winfo screenheight . ] - $tbarsize - 2 * $bordsize ] 
					}
					if { [ expr [ winfo screenwidth . ] ] < ( $hsizeL + 2 * $bordsize + $hmargin ) } {
						set X 0
					} {
						set X [ expr [ winfo screenwidth . ] - $hmargin - $bordsize - $W ]
					}
					set Y [ expr ( [ winfo screenheight . ] - $tbarsize ) / 2 - $bordsize - $H / 2]
					wm geom . "${W}x$H+$X+$Y"
					wm minsize . $hsizeLmin $vsizeLmin
				}
				log {
					set X [ getx .log bottomrightS ]
					set Y [ gety .log bottomrightS ]
					wm geom .log +$X+$Y
					wm minsize .log [ winfo width .log ] [ winfo height .log ]
				}
				str {
					set posXstr [ expr [ getx . topleftS ] + [ winfo width . ] + $hmargin ]
					set posYstr [ gety . topleftS ]
					wm geometry .str ${hsizeM}x${vsizeM}+${posXstr}+${posYstr}	
					wm minsize .str [ expr $hsizeM / 2 ] [ expr $vsizeM / 2 ]	
				}
			}
		}
	}

	update

	if { $logWndFn && [ info procs plog ] != "" } { plog "\nizetop (w:$w, master:[wm transient $w], pos:([winfo x $w],[winfo y $w]), size:[winfo width $w]x[winfo height $w], parWndLst:$parWndLst, grab:$grabLst)" } 
}

# resize the window
proc resizetop { w sizeX { sizeY 0 } } {
	global parWndLst grabLst logWndFn

	if { $sizeX <= 0 } {
		set sizeX [ winfo width $w ]
	}
	if { $sizeY <= 0 } {
		set sizeY [ winfo height $w ]
	}
	if { $sizeX > [ expr [ winfo screenwidth $w ] - [ winfo rootx $w ] ] } {
		set sizeX [ expr [ winfo screenwidth $w ] - [ winfo rootx $w ] ]
	}
	if { $sizeY > [ expr [ winfo screenheight $w ] - [ winfo rooty $w ] ] } {
		set sizeY [ expr [ winfo screenheight $w ] - [ winfo rooty $w ] ]
	}
	set newMinX [ expr min( [ lindex [ wm minsize $w ] 0 ], $sizeX ) ]
	set newMinY [ expr min( [ lindex [ wm minsize $w ] 1 ], $sizeY ) ]
	if { $newMinX != [ lindex [ wm minsize $w ] 0 ] || $newMinY != [ lindex [ wm minsize $w ] 1 ] } {
		wm minsize $w $newMinX $newMinY
	}
	if { $sizeX != [ winfo width $w ] || $sizeY != [ winfo height $w ] } {
		wm geom $w ${sizeX}x${sizeY} 
	}
	update

	if { $logWndFn && [ info procs plog ] != "" } { plog "\nresizetop (w:$w, master:[wm transient $w], pos:([winfo x $w],[winfo y $w]), size:[winfo width $w]x[winfo height $w], parWndLst:$parWndLst, grab:$grabLst)" } 
}

# set window icon
proc icontop { w { type lsd } } {
	global tcl_platform RootLsd LsdSrc lmmImg lsdImg iconExt
	
	if { $tcl_platform(platform) == "windows" } {
		if [ string equal $w . ] {
			wm iconbitmap $w -default $RootLsd/$LsdSrc/icons/$type.ico
		} {
			wm iconbitmap $w $RootLsd/$LsdSrc/icons/$type.ico
		}
	} {
		if [ string equal $w . ] {
			wm iconphoto $w -default ${type}Img
			wm iconbitmap $w @$RootLsd/$LsdSrc/icons/$type.xbm
		} {
			wm iconphoto $w ${type}Img
			wm iconbitmap $w @$RootLsd/$LsdSrc/icons/$type.xbm
		}
	}
}

# alignment of window w1 to the to right side of w2
proc align {w1 w2} {
	global hmargin corrX corrY logWndFn
	
	set a [ winfo width $w1 ]
	set b [ winfo height $w1 ]
	set c [ expr [ winfo x $w2 ] + $corrX ]
	set d [ expr [ winfo y $w2 ] + $corrY ]
	set e [ winfo width $w2 ]

	set f [ expr $c + $e + $hmargin ]
	wm geometry $w1 +$f+$d
	update

	if { $logWndFn && [ info procs plog ] != "" } { plog "\nalign w1:$w1 w2:$w2 (w1 width:$a, w1 height:$b, w2 x:$c, w2 y:$d, w2 width:$e)" }
}

# check if window center is in primary display
proc primdisp w {
	if { [ winfo rootx $w ] > 0 && [ winfo rootx $w ] < [ winfo screenwidth $w ] && [ winfo rooty $w ] > 0 && [ winfo rooty $w ] < [ winfo screenheight $w ] } {
		return true
	} {
		return false
	}
} 

# compute x and y coordinates of new window according to the types
proc getx { w pos } {
	global corrX hmargin bordsize
	
	switch $pos {
		centerS { 
			return [ expr [ winfo screenwidth $w ] / 2 - [ winfo reqwidth $w ] / 2 ]
		}
		centerW { 
			return [ expr [ winfo x [ winfo parent $w ] ] + $corrX + [ winfo width [ winfo parent $w ] ] / 2  - [ winfo reqwidth $w ] / 2 ]
		}
		topleftS { 
			return [ expr $hmargin + $corrX ]
		}
		topleftW { 
			return [ expr [ winfo x [ winfo parent $w ] ] + $corrX + 10 ]
		}
		overM { 
			return [ expr [ winfo x . ] + $corrX ]
		}
		coverW { 
			return [ expr [ winfo x [ winfo parent $w ] ] + $corrX ]
		}
		bottomrightS {
			return [ expr [ winfo screenwidth $w ] - $hmargin - [ winfo reqwidth $w ] ]
		}
		righttoW {
			return [ expr [ winfo x [ winfo parent $w ] ] + $corrX + $hmargin + [ winfo reqwidth [ winfo parent $w ] ] - 2 * $bordsize ]
		}
	}
}

proc gety { w pos } {
	global corrY vmargin tbarsize
	
	switch $pos {
		centerS { 
			return [ expr [ winfo screenheight $w ] / 2 - [ winfo reqheight $w ] / 2 ]
		}
		centerW { 
			return [ expr [ winfo y [ winfo parent $w ] ] + $corrY + [ winfo height [ winfo parent $w ] ] / 2  - [ winfo reqheight $w ] / 2 ]
		}
		topleftS { 
			return [ expr $vmargin + $corrY ]
		}
		topleftW { 
			return [ expr [ winfo y [ winfo parent $w ] ] + $corrY + 30 ]
		}
		overM { 
			return [ expr [ winfo y . ] + $corrY ]
		}
		coverW { 
			return [ expr [ winfo y [ winfo parent $w ] ] + $corrY ]
		}
		bottomrightS {
			return [ expr [ winfo screenheight $w ] - $vmargin - $tbarsize - [ winfo reqheight $w ] ]
		}
		righttoW {
			return [ expr [ winfo y [ winfo parent $w ] ] + $corrY ]
		}
	} 
}

# procedures to create standard button sets
proc okhelpcancel { w fr comOk comHelp comCancel } {
	if { ! [ winfo exists $w.$fr ] } { frame $w.$fr }
	button $w.$fr.ok -width -9 -text Ok -command $comOk
	button $w.$fr.help -width -9 -text Help -command $comHelp
	button $w.$fr.can -width -9 -text Cancel -command $comCancel
	bind $w.$fr.ok <KeyPress-Return> "$w.$fr.ok invoke"
	bind $w.$fr.help <KeyPress-Return> "$w.$fr.help invoke"
	bind $w.$fr.can <KeyPress-Return> "$w.$fr.can invoke"
	bind $w <KeyPress-Escape> "$w.$fr.can invoke"
	pack $w.$fr.ok $w.$fr.help $w.$fr.can -padx 10 -pady 10 -side left
	pack $w.$fr -side right 
}

proc okhelp { w fr comOk comHelp } {
	if { ! [ winfo exists $w.$fr ] } { frame $w.$fr }
	button $w.$fr.ok -width -9 -text Ok -command $comOk
	button $w.$fr.help -width -9 -text Help -command $comHelp
	bind $w.$fr.ok <KeyPress-Return> "$w.$fr.ok invoke"
	bind $w.$fr.help <KeyPress-Return> "$w.$fr.help invoke"
	bind $w <KeyPress-Escape> "$w.$fr.ok invoke"
	pack $w.$fr.ok $w.$fr.help -padx 10 -pady 10 -side left
	pack $w.$fr -side right 
}

proc okcancel { w fr comOk comCancel } {
	if { ! [ winfo exists $w.$fr ] } { frame $w.$fr }
	button $w.$fr.ok -width -9 -text Ok -command $comOk
	button $w.$fr.can -width -9 -text Cancel -command $comCancel
	bind $w.$fr.ok <KeyPress-Return> "$w.$fr.ok invoke"
	bind $w.$fr.can <KeyPress-Return> "$w.$fr.can invoke"
	bind $w <KeyPress-Escape> "$w.$fr.can invoke"
	pack $w.$fr.ok $w.$fr.can -padx 10 -pady 10 -side left
	pack $w.$fr -side right 
}

proc helpcancel { w fr comHelp comCancel } {
	if { ! [ winfo exists $w.$fr ] } { frame $w.$fr }
	button $w.$fr.help -width -9 -text Help -command $comHelp
	button $w.$fr.can -width -9 -text Cancel -command $comCancel
	bind $w.$fr.help <KeyPress-Return> "$w.$fr.help invoke"
	bind $w.$fr.can <KeyPress-Return> "$w.$fr.can invoke"
	bind $w <KeyPress-Escape> "$w.$fr.can invoke"
	pack $w.$fr.help $w.$fr.can -padx 10 -pady 10 -side left
	pack $w.$fr -side right 
}

proc ok { w fr comOk } {
	if { ! [ winfo exists $w.$fr ] } { frame $w.$fr }
	button $w.$fr.ok -width -9 -text Ok -command $comOk
	bind $w.$fr.ok <KeyPress-Return> "$w.$fr.ok invoke"
	bind $w <KeyPress-Escape> "$w.$fr.ok invoke"
	pack $w.$fr.ok -padx 10 -pady 10 -side left
	pack $w.$fr -side right 
}

proc okXhelpcancel { w fr nameX comX comOk comHelp comCancel } {
	if { ! [ winfo exists $w.$fr ] } { frame $w.$fr }
	button $w.$fr.ok -width -9 -text Ok -command $comOk
	button $w.$fr.x -width -9 -text $nameX -command $comX
	button $w.$fr.help -width -9 -text Help -command $comHelp
	button $w.$fr.can -width -9 -text Cancel -command $comCancel
	bind $w.$fr.ok <KeyPress-Return> "$w.$fr.ok invoke"
	bind $w.$fr.x <KeyPress-Return> "$w.$fr.x invoke"
	bind $w.$fr.help <KeyPress-Return> "$w.$fr.help invoke"
	bind $w.$fr.can <KeyPress-Return> "$w.$fr.can invoke"
	bind $w <KeyPress-Escape> "$w.$fr.can invoke"
	pack $w.$fr.ok $w.$fr.x $w.$fr.help $w.$fr.can -padx 10 -pady 10 -side left
	pack $w.$fr -side right 
}

proc donehelp { w fr comDone comHelp } {
	if { ! [ winfo exists $w.$fr ] } { frame $w.$fr }
	button $w.$fr.ok -width -9 -text Done -command $comDone
	button $w.$fr.help -width -9 -text Help -command $comHelp
	bind $w.$fr.ok <KeyPress-Return> "$w.$fr.ok invoke"
	bind $w.$fr.help <KeyPress-Return> "$w.$fr.help invoke"
	bind $w <KeyPress-Escape> "$w.$fr.ok invoke"
	pack $w.$fr.ok $w.$fr.help -padx 10 -pady 10 -side left
	pack $w.$fr -side right 
}

proc done { w fr comDone } {
	if { ! [ winfo exists $w.$fr ] } { frame $w.$fr }
	button $w.$fr.ok -width -9 -text Done -command $comDone
	bind $w.$fr.ok <KeyPress-Return> "$w.$fr.ok invoke"
	bind $w <KeyPress-Escape> "$w.$fr.ok invoke"
	pack $w.$fr.ok -padx 10 -pady 10 -side left
	pack $w.$fr -side right 
}

proc comphelpdone { w fr comComp comHelp comDone } {
	if { ! [ winfo exists $w.$fr ] } { frame $w.$fr }
	button $w.$fr.com -width -9 -text Compute -command $comComp
	button $w.$fr.help -width -9 -text Help -command $comHelp
	button $w.$fr.ok -width -9 -text Done -command $comDone
	bind $w.$fr.com <KeyPress-Return> "$w.$fr.com invoke"
	bind $w.$fr.help <KeyPress-Return> "$w.$fr.help invoke"
	bind $w.$fr.ok <KeyPress-Return> "$w.$fr.ok invoke"
	bind $w <KeyPress-Escape> "$w.$fr.ok invoke"
	pack $w.$fr.com $w.$fr.help $w.$fr.ok -padx 10 -pady 10 -side left
	pack $w.$fr -side right 
}

proc finddone { w fr comFind comDone } {
	if { ! [ winfo exists $w.$fr ] } { frame $w.$fr }
	button $w.$fr.search -width -9 -text Find -command $comFind
	button $w.$fr.ok -width -9 -text Done -command $comDone
	bind $w.$fr.search <KeyPress-Return> "$w.$fr.search invoke"
	bind $w.$fr.ok <KeyPress-Return> "$w.$fr.ok invoke"
	bind $w <KeyPress-Escape> "$w.$fr.ok invoke"
	pack $w.$fr.search $w.$fr.ok -padx 10 -pady 10 -side left
	pack $w.$fr -side right 
}


# commands to disable/enable windows in cases where grab is inappropriate (only menus if not TK8.6)
# call parameters are: container window, menu name, widgets names
proc disable_window { w m { args "" } } {
	if [ winfo exist $w.$m ] {
		for { set i 0 } { $i <= [ $w.$m index last ] } { incr i } {
			$w.$m entryconfig $i -state disabled
		}
	}
	if [ string equal [ info tclversion ] "8.6" ] {
		foreach i $args {
			if [ winfo exists $w.$i ] {
				tk busy hold $w.$i
			}
		}
	}
	update 
}
	
proc enable_window { w m { args "" } } {
	if [ winfo exist $w.$m ] {
		for { set i 0 } { $i <= [ $w.$m index last ] } { incr i } {
			$w.$m entryconfig $i -state normal
		}
	}
	if [ string equal [ info tclversion ] "8.6" ] {
		foreach i $args {
			if [ winfo exists $w.$i ] {
				tk busy forget $w.$i
			}
		}
	}
	update
}
	
	
# read any entry widget (normal or disabled)
proc write_any { w val } {
	if [ string equal [ $w cget -state ] disabled ] {
		write_disabled $w $val
	} {
		$w delete 0 end
		$w insert 0 $val
	}
}


# update a disabled entry widget (do nothing if normal state)
proc write_disabled { w val } {
	if [ string equal [ $w cget -state ] disabled ] {
		$w conf -state normal
		write_any $w $val
		$w conf -state disabled
	}
}


# bind the mouse wheel to the y scrollbar
proc mouse_wheel { w } {
	global tcl_platform
	if [ string equal $tcl_platform(platform) windows ] {
		bind $w <MouseWheel> { %W yview scroll [ expr { -%D / 40 } ] units }
	} {
		if [ string equal $tcl_platform(os) Darwin ] {
			bind $w <MouseWheel> { %W yview scroll [ expr { -%D } ] units }
		} {
			bind $w <4> { %W yview scroll -1 units }
			bind $w <5> { %W yview scroll 1 units }
		}
	}
}
