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
#	topleftW: put over the top left corner of parent window (below menu bar)
#	current: keep current position
#
# Variable 'alignMode' configure special, per module (LMM, LSD), settings

# register static special configurations
unset -nocomplain defaultPos defaultFocus
if [ info exists alignMode ] {
	if [ string equal -nocase $alignMode "LMM" ] {
		set defaultPos centerW
		set defaultFocus .f.t.t
	}
}

# list to hold the windows parents stack
set parWndLst [ list ]

# procedures for create, update and destroy top level new windows
proc newtop { w { name "" } { destroy { } } { par "." } } {
	toplevel $w
	if { $par != "" } {
		if { $par != "." } {
			wm transient $w $par 
		} {
			global parWndLst
			if { [ llength $parWndLst ] > 0 } {
				wm transient $w [ lindex $parWndLst 0 ] 
			} {
				wm transient $w .
			}
		} 
	}
	wm title $w $name
	wm protocol $w WM_DELETE_WINDOW $destroy
#	plog "\nnewtop (w:$w, master:[wm transient $w], parWndLst:$parWndLst)" 
}

proc settop { w { name no } { destroy no } { par no } } {
	if { $par != no } {
		wm transient $w $par 
	}
	if { $name != no } {
		wm title $w $name 
	}
	if { $destroy != no } {
		wm protocol $w WM_DELETE_WINDOW $destroy 
	}
	wm withdraw $w
	update
	wm deiconify $w
	raise $w
	update 
}

# configure the window
proc showtop { w { pos none } { resizeX no } { resizeY no } { grab yes } { sizeX 0 } { sizeY 0 } } {
	if { $sizeX != 0 && $sizeY != 0 } {
		$w configure -width $sizeX -height $sizeY 
	}
	wm withdraw $w
	update idletasks
	# handle different window default position
	global defaultPos parWndLst
	if [ string equal $pos none ] {
		if [ info exists defaultPos ] {
			set pos $defaultPos
		} {
			set pos centerS
		}
	}
	if { ! [ string equal $pos current ] } {
		if { [ string equal $pos centerS ] && ! [ primdisp [ winfo parent $w ] ] } {
			set pos topleftW 
		}
		set x [ getx $w $pos ]
		set y [ gety $w $pos ]
		if { $sizeX != 0 && $sizeY != 0 } {
			wm geom $w ${sizeX}x${sizeY}+$x+$y 
		} {
			wm geom $w +$x+$y
		}
	}
	wm resizable $w $resizeX $resizeY
	set parWndLst [ linsert $parWndLst 0 $w ]
	if $grab {
		global lstGrab
		lappend lstGrab "$w [ grab current $w ]"
		grab set $w 
	}
	wm deiconify $w
	raise $w
	update
#	plog "\nshowtop (w:$w, master:[wm transient $w], parWndLst:$parWndLst, pos:$pos)" 
}

proc destroytop w {
	if [ winfo exists $w ] {
		global lstGrab
		if [ info exists lstGrab ] {
			set igrab [ lsearch -glob $lstGrab "$w *" ]
			if { $igrab >= 0 } {
				grab release $w
				set grabPar [ string range [ lindex $lstGrab $igrab ] [ expr [ string first " " [ lindex $lstGrab $igrab ] ] + 1 ] end ]
				if { $grabPar != "" } {
					grab set $grabPar 
				}
				set lstGrab [ lreplace $lstGrab $igrab $igrab ]
			}
		}
		global parWndLst defaultFocus
		if { [ llength $parWndLst ] > 0 } {
			set parWndLst [ lreplace $parWndLst 0 0 ] 
		}
		# handle different window default focus on destroy
		if [ info exists defaultFocus ] {
			focus -force $defaultFocus
		} {
			focus -force [ winfo parent $w ]
		}
		destroy $w
		update
#		plog "\ndestroytop (w:$w, parWndLst:$parWndLst)"
	}
}

# procedure to save top window size
proc save_top_size { } {
	scan [ wm geometry . ] "%dx%d%*s" w h
	variable wB $w
	variable hB $h 
}

# procedure to restore top window size
proc restore_top_size { } { 
	variable wB
	variable hB
	scan [ wm geometry . ] "%dx%d%*s" w h
	if { $wB != $w || $hB != $h } {
		wm geometry . "[ expr $wB ]x$hB"
	}
}

# alignment of window w1 to the to right side of w2
proc align {w1 w2} {
	global posX
	set a [winfo width $w1]
	set b [winfo height $w1]
	set c [winfo x $w2]
	set d [winfo y $w2]
	set e [winfo width $w2]

	set f [expr $c + $e + $posX ]
	wm geometry $w1 +$f+$d
#	plog "align w1:$w1 w2:$w2 (w1 width:$a, w1 height:$b, w2 x:$c, w2 y:$d, w2 width:$e)"
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
	switch $pos {
		centerS { 
			return [ expr [ winfo screenwidth $w ] / 2 - [ winfo reqwidth $w ] / 2 ]
		}
		centerW { 
			return [ expr [ winfo rootx [ winfo parent $w ] ] + [ winfo width [ winfo parent $w ] ] / 2  - [ winfo reqwidth $w ] / 2 ]
		}
		topleftS { 
			global hmargin
			return $hmargin
		}
		topleftW { 
			return [ expr [ winfo x [ winfo parent $w ] ] + 5 ]
		}
	}
#	plog "\nw:$w (parent:[ winfo parent $w ], x:[ winfo x [ winfo parent $w ] ],"
}

proc gety { w pos } {
	switch $pos {
		centerS { 
			return [ expr [ winfo screenheight $w ] / 2 - [ winfo reqheight $w ] / 2 ]
		}
		centerW { 
			return [ expr [ winfo rooty [ winfo parent $w ] ] + [ winfo height [ winfo parent $w ] ] / 2  - [ winfo reqheight $w ] / 2 ]
		}
		topleftS { 
			global vmargin
			return $vmargin
		}
		topleftW { 
			return [ expr [ winfo y [ winfo parent $w ] ] + 50 ]
		}
	} 
#	plog "y:[ winfo y [ winfo parent $w ] ])"
}

# procedures to create standard button sets
proc okhelpcancel { w fr comOk comHelp comCancel } {
	frame $w.$fr
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

proc ok { w fr comOk comHelp } {
	frame $w.$fr
	button $w.$fr.ok -width -9 -text Ok -command $comOk
	button $w.$fr.help -width -9 -text Help -command $comHelp
	bind $w.$fr.ok <KeyPress-Return> "$w.$fr.ok invoke"
	bind $w.$fr.help <KeyPress-Return> "$w.$fr.help invoke"
	bind $w <KeyPress-Escape> "$w.$fr.ok invoke"
	pack $w.$fr.ok $w.$fr.help -padx 10 -pady 10 -side left
	pack $w.$fr -side right 
}

proc okcancel { w fr comOk comCancel } {
	frame $w.$fr
	button $w.$fr.ok -width -9 -text Ok -command $comOk
	button $w.$fr.can -width -9 -text Cancel -command $comCancel
	bind $w.$fr.ok <KeyPress-Return> "$w.$fr.ok invoke"
	bind $w.$fr.can <KeyPress-Return> "$w.$fr.can invoke"
	bind $w <KeyPress-Escape> "$w.$fr.can invoke"
	pack $w.$fr.ok $w.$fr.can -padx 10 -pady 10 -side left
	pack $w.$fr -side right 
}

proc helpcancel { w fr comHelp comCancel } {
	frame $w.$fr
	button $w.$fr.help -width -9 -text Help -command $comHelp
	button $w.$fr.can -width -9 -text Cancel -command $comCancel
	bind $w.$fr.help <KeyPress-Return> "$w.$fr.help invoke"
	bind $w.$fr.can <KeyPress-Return> "$w.$fr.can invoke"
	bind $w <KeyPress-Escape> "$w.$fr.can invoke"
	pack $w.$fr.help $w.$fr.can -padx 10 -pady 10 -side left
	pack $w.$fr -side right 
}

proc ok { w fr comOk } {
	frame $w.$fr
	button $w.$fr.ok -width -9 -text Ok -command $comOk
	bind $w.$fr.ok <KeyPress-Return> "$w.$fr.ok invoke"
	bind $w <KeyPress-Escape> "$w.$fr.ok invoke"
	pack $w.$fr.ok -padx 10 -pady 10 -side left
	pack $w.$fr -side right 
}

proc donehelp { w fr comDone comHelp } {
	frame $w.$fr
	button $w.$fr.ok -width -9 -text Done -command $comDone
	button $w.$fr.help -width -9 -text Help -command $comHelp
	bind $w.$fr.ok <KeyPress-Return> "$w.$fr.ok invoke"
	bind $w.$fr.help <KeyPress-Return> "$w.$fr.help invoke"
	bind $w <KeyPress-Escape> "$w.$fr.ok invoke"
	pack $w.$fr.ok $w.$fr.help -padx 10 -pady 10 -side left
	pack $w.$fr -side right 
}

proc done { w fr comDone } {
	frame $w.$fr
	button $w.$fr.ok -width -9 -text Done -command $comDone
	bind $w.$fr.ok <KeyPress-Return> "$w.$fr.ok invoke"
	bind $w <KeyPress-Escape> "$w.$fr.ok invoke"
	pack $w.$fr.ok -padx 10 -pady 10 -side left
	pack $w.$fr -side right 
}

proc comphelpdone { w fr comComp comHelp comDone } {
	frame $w.$fr
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

proc searchdone { w fr comSearch comDone } {
	frame $w.$fr
	button $w.$fr.search -width -9 -text Search -command $comSearch
	button $w.$fr.ok -width -9 -text Done -command $comDone
	bind $w.$fr.search <KeyPress-Return> "$w.$fr.search invoke"
	bind $w.$fr.ok <KeyPress-Return> "$w.$fr.ok invoke"
	bind $w <KeyPress-Escape> "$w.$fr.ok invoke"
	pack $w.$fr.search $w.$fr.ok -padx 10 -pady 10 -side left
	pack $w.$fr -side right 
}
