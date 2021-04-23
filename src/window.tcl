#*************************************************************
#
#	LSD 8.0 - March 2021
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
# WINDOW.TCL
# Collection of procedures to manage LSD windows.
#*************************************************************

#************************************************
# NEWTOP
# Procedure to create top level new windows
#************************************************
proc newtop { w { name "" } { destroy { } } { par "." } { noglobkeys 0 } } {
	global CurPlatform RootLsd parWndLst grabLst noParLst logWndFn colorsTheme activeplot

	destroytop $w
	toplevel $w -background $colorsTheme(bg)

	# workaround for bug in Tk 8.6.11 (must update before withdrawing)
	if { [ string equal $CurPlatform mac ] } { 
		wm attributes $w -alpha 0
		update idletasks
	}

	wm withdraw $w
	update idletasks

	if { $par != "" } {
		if { $par != "." } {
			if { [ winfo viewable [ winfo toplevel $par ] ] } {
				wm transient $w $par
			}
		} {
			# attribute transient only if not in the no parent list and window is visible
			if { [ lsearch $noParLst [ string range $w 0 3 ] ] < 0 } {
				# remove non-existing windows in the beginning of the parents list
				while { [ llength $parWndLst ] > 0 && ! [ winfo exists [ lindex $parWndLst 0 ] ] } {
					set parWndLst [ lreplace $parWndLst 0 0 ]
				}
				# avoid setting the window transient to itself
				if { [ llength $parWndLst ] > 0 && ! [ string equal [ lindex $parWndLst 0 ] $w ] } {
					# set transient only if parent's parent is visible
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
	wm attributes $w -alpha 1
	
	if { ! $noglobkeys } {
		setglobkeys $w
	}

	if { $logWndFn && [ info procs plog ] != "" } { 
		plog "\nnewtop (w:$w, master:[ wm transient $w ], parWndLst:$parWndLst, grab:$grabLst)"
	}
}


#************************************************
# SETTOP
# Show the window
#************************************************
proc settop { w { name no } { destroy no } { par no } { force no } } {
	global parWndLst grabLst logWndFn

	if { $par != no && $par != "" } {
		if { [ winfo viewable [ winfo toplevel $par ] ] } {
			wm transient $w $par
		}
	}
	
	if { $name != no && $name != "" } {
		wm title $w $name
	}
	
	if { $destroy != no && $destroy != "" } {
		wm protocol $w WM_DELETE_WINDOW $destroy
	}
	
	deiconifytop $w $force
	raise $w
	focustop $w "" $force
	update idletasks

	if { $logWndFn && [ info procs plog ] != "" } { 
		plog "\nsettop (w:$w, master:[ wm transient $w ], pos:([ winfo x $w ],[ winfo y $w ]), size:[ winfo width $w ]x[ winfo height $w ], parWndLst:$parWndLst, grab:$grabLst)" 
	}
}


#************************************************
# SHOWTOP
# Configure the window
#
# Types of window positioning:
# centerS: center over the primary display, only available if the parent window
#	center is also in the primary display (if not, falback to centerW)
# centerW: center over the parent window (in any display)
# topleftS: put over the top left corner of screen of primary display, only
#	available if the parent window center is also in the primary display (if not,
#	falback to topleftW)
# topleftW: put over the top left corner of parent window (around menu bar)
# coverW: cover the parent window (same size & position)
# overM: over the main window (same top-left position)
# current: keep current position
#************************************************
proc showtop { w { pos none } { resizeX no } { resizeY no } { grab yes } { sizeX 0 } { sizeY 0 } { buttonF b } { noMinSize no } { force no } } {
	global defaultPos hmargin vmargin bordsize tbarsize wndLst parWndLst grabLst noParLst logWndFn

	# copy of applied geometry, if any
	set gm ""

	#handle main windows differently
	if { [ lsearch $wndLst $w ] < 0 } {

		# unknown window (not a main one)
		if { ! [ string equal $pos xy ] && $sizeX != 0 } {
			$w configure -width $sizeX
		}
		if { ! [ string equal $pos xy ] && $sizeY != 0 } {
			$w configure -height $sizeY
		}

		update idletasks

		if { ! [ string equal $pos current ] } {

			# handle different window default position
			if [ string equal $pos none ] {
				if [ info exists defaultPos ] {
					set pos $defaultPos
				} else {
					if [ winfo viewable [ winfo parent $w ] ] {
						set pos centerW
					} else {
						set pos centerS
					}
				}
			}

			# don't use screen based positioning if not in primary display
			if { [ string equal $pos centerS ] && ! [ primdisp [ winfo parent $w ] ] } {
				if [ info exists defaultPos ] {
					set pos $defaultPos
				} else {
					if [ winfo viewable [ winfo parent $w ] ] {
						set pos centerW
					} else {
						set pos centerS
					}
				}
			}

			if { ! [ string equal $pos xy ]	} {
				set x [ getx $w $pos ]
				set y [ gety $w $pos ]

				set maxWid [ expr { [ winfo vrootwidth $w ] - $x - 2 * $bordsize - $hmargin } ]
				set maxHgt [ expr { [ winfo vrootheight $w ] - $y- 2 * $bordsize - $vmargin - $tbarsize } ]
					
				if { $maxWid > 0 && $sizeX > $maxWid } {
					set sizeX $maxWid
					$w configure -width $sizeX
				}
				if { $maxHgt > 0 && $sizeY > $maxHgt } {
					set sizeY $maxHgt
					$w configure -height $sizeY
				}
			} else {
				set x $sizeX
				set y $sizeY
			}

			if { ! [ string equal "" $x ] && ! [ string equal "" $y ] } {

				if { [ string equal $pos coverW ] } {
					set sizeX [ expr { [ winfo width [ winfo parent $w ] ] + 10 } ]
					set sizeY [ expr { [ winfo height [ winfo parent $w ] ] + 30 } ]
				}

				if { ! [ string equal $pos xy ]	&& $sizeX != 0 && $sizeY != 0 } {
					set gm ${sizeX}x${sizeY}+$x+$y
				} else {
					set gm +$x+$y
				}
				
				wm geometry $w $gm
			}
		} else {
			if { $sizeX != 0 && $sizeY != 0 } {
				set gm ${sizeX}x${sizeY}
				wm geometry $w $gm
			}
		}

		if { ! $noMinSize && ( $resizeX || $resizeY ) } {
			update idletasks
			wm minsize $w [ winfo reqwidth $w ] [ winfo reqheight $w ]
		}

		wm resizable $w $resizeX $resizeY
	} else {
		#known windows - simply apply defaults if not done before
		if { ! [ string equal $pos current ] } {
			sizetop $w
		}
	}

	wm maxsize $w [ winfo vrootwidth $w ] [ winfo vrootheight $w ]
	focustop $w "" $force
	update idletasks

	# grab focus, if required, updating the grabbing list
	if { $grab && $w != "." && [ lsearch $noParLst [ string range $w 0 3 ] ] < 0 } {

		set parWndLst [ linsert $parWndLst 0 $w ]

		if { ! [ info exists grabLst ] || [ lsearch -glob $grabLst "$w *" ] < 0 } {
			lappend grabLst "$w [ grab current $w ]"
		}

		grab set $w

		# reposition window because of macOS bug when grabbing
		if { [ string equal [ tk windowingsystem ] aqua ] && $gm != "" } {
			wm geometry $w $gm
		}

		raise $w
	}

	# because of macOS bug when not grabbing scroll bar doesn't work...
	if { ! $grab && [ string equal [ tk windowingsystem ] aqua ] } {
		grab set $w
		grab release $w
	}

	if { [ string length buttonF ] > 0 && [ winfo exists $w.$buttonF.ok ] } {
		$w.$buttonF.ok configure -default active -state active
		focus $w.$buttonF.ok
	} elseif { [ string length buttonF ] > 0 && [ winfo exists $w.$buttonF.r2.ok ] } {
		$w.$buttonF.r2.ok configure -default active -state active
		focus $w.$buttonF.r2.ok
	} else {
		focus $w
	}

	update idletasks
	
	if { $logWndFn && [ info procs plog ] != "" } { 
		plog "\nshowtop (w:$w, master:[ wm transient $w ], pos:([ winfo x $w ],[ winfo y $w ]), size:[ winfo width $w ]x[ winfo height $w ], minsize:[ wm minsize $w ], primdisp:[ primdisp [ winfo parent $w ] ], parWndLst:$parWndLst, grab:$grabLst)" 
	}
}


#************************************************
# DESTROYTOP
# Destroy window, if it exists
#************************************************
proc destroytop w {
	global restoreWin wndLst defaultFocus parWndLst grabLst noParLst logWndFn

	if { $w == "" || ! [ winfo exists $w ] } {
		return
	}

	# save main windows sizes/positions
	if { [ info exists restoreWin ] && $restoreWin && [ lsearch $wndLst $w ] >= 0 } {
		set curGeom [ geomtosave $w ]

		if { $curGeom != "" } {
			set wName [ string range $w 1 3 ]
			set ::${wName}Geom $curGeom
		}
	}

	if { [ lsearch $noParLst [ string range $w 0 3 ] ] < 0 } {
		if [ info exists grabLst ] {
			set igrab [ lsearch -glob $grabLst "$w *" ]
			if { $igrab >= 0 } {
				grab release $w
				set grabPar [ string range [ lindex $grabLst $igrab ] [ expr { [ string first " " [ lindex $grabLst $igrab ] ] + 1 } ] end ]
				if { $grabPar != "" } {
					focustop $grabPar
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
	} else {
		focus [ winfo parent $w ]
	}
	
	destroy $w
	update idletasks

	if { $logWndFn && [ info procs plog ] != "" } { 
		plog "\ndestroytop (w:$w, parWndLst:$parWndLst, grab:$grabLst)" 
	}
}


#************************************************
# GEOMTOP
# Return REAL size & positions of a top window
#************************************************
proc geomtop { { w . } } {

	# extract info from Tk/window manager
	set geom [ wm geometry $w ]
	scan $geom "%dx%d+%d+%d" width height decorationLeft decorationTop
	set contentsTop [ winfo rooty $w ]
	set contentsLeft [ winfo rootx $w ]

	# measure left edge, and assume all edges except top are the same thickness
	set decorationThickness [ expr { $contentsLeft - $decorationLeft } ]

	# find titlebar plus menubar thickness
	set menubarThickness [ expr { $contentsTop - $decorationTop } ]

	# compute real values
	incr width [ expr { 2 * $decorationThickness } ]
	incr height $decorationThickness
	incr height $menubarThickness

	return [ list $width $height $decorationLeft $decorationTop ]
}


#************************************************
# GEOMTOSAVE
# Return size & position of a window to be saved
# and later reopened, at the same place/size
#************************************************
proc geomtosave { { w . } } {
	global CurPlatform wndMenuHeight hfactM vfactM

	# handle virtual top windows names
	if { [ string equal $w .lmm ] || [ string equal $w .lsd ] } {
		set realW .
	} else {
		set realW $w
	}

	if { ! [ winfo exists $realW ] } {
		return ""
	}

	set geom [ wm geometry $realW ]
	scan $geom "%dx%d+%d+%d" width height decorationLeft decorationTop
	set contentsLeft [ winfo rootx $realW ]
	set contentsTop [ winfo rooty $realW ]

	# handle windows with incorrect size/position because of Tk ugly bugs in each platform
	switch $CurPlatform {
		linux {
			set realHeight $height
			set realX $contentsLeft

			switch $w {
				.da -
				.deb {
					set realY [ expr { $decorationTop + $contentsLeft - $decorationLeft + 8 } ]
				}
				.lat {
					set realY [ expr { $decorationTop + $contentsLeft - $decorationLeft } ]
				}
				default {
					set realY [ expr { $decorationTop + $contentsLeft - $decorationLeft - 2 } ]
				}
			}
		}

		mac {
			set realX $decorationLeft
			set realY $decorationTop
			set realHeight [ expr { $height + $contentsTop - $decorationTop - $wndMenuHeight } ]
		}

		windows {
			set realX $decorationLeft
			set realY $decorationTop

			switch $w {
				.da -
				.deb {
					set realHeight [ expr { $height + $contentsTop - $decorationTop - ( $wndMenuHeight + 20 ) } ]
				}
				.plt -
				.dap {
					set realHeight [ expr { $height } ]
				}
				default {
					set realHeight [ expr { $height + $contentsTop - $decorationTop - $wndMenuHeight } ]
				}
			}
		}
	}

	if { $w == ".str" } {
		return ${width}x${realHeight}+${realX}+${realY}:${hfactM}+${vfactM}
	} else {
		return ${width}x${realHeight}+${realX}+${realY}
	}

}


#************************************************
# CHECKGEOM
# check for window mostly out of the (main)
# screen or invalid and use the default if needed
#************************************************
proc checkgeom { geom defGeom screenWidth screenHeight } {
	global restoreWin hfactMmin vfactMmin

	if { ! $restoreWin || $geom == "#" } {
		return $defGeom
	} else {
		set n [ scan $geom "%dx%d+%d+%d:%f+%f" width height decorationLeft decorationTop hScale vScale ]

		if { $n < 4 } {
			return $defGeom
		} else {
			set centerX [ expr { $decorationLeft + $width / 2 } ]
			set centerY [ expr { $decorationTop + $height / 2 } ]

			if { $centerX < 0 || $centerX > $screenWidth || $centerY < 0 || $centerY > $screenHeight } {
				return $defGeom
			}

			if { $n == 6 && ( $hScale < $hfactMmin || $vScale < $vfactMmin ) } {
				return $defGeom
			}
		}
	}

	return $geom
}


#************************************************
# SIZETOP
# Adjust main windows to default size & positions
#************************************************
proc sizetop { { w all } } {
	global wndLst hsizeBmin vsizeBmin hsizeL vsizeL hsizeLmin vsizeLmin hsizeGmin vsizeGmin hsizeAmin vsizeAmin hsizePmin vsizePmin hsizeDmin vsizeDmin bordsize hmargin vmargin tbarsize posXstr posYstr hsizeM vsizeM corrX corrY parWndLst grabLst logWndFn lmmGeom lsdGeom logGeom strGeom daGeom debGeom latGeom pltGeom dapGeom hfactM vfactM wndMenuHeight

	update idletasks

	if { [ string equal $w .lsd ] || [ string equal $w .lmm ] } {

		set realW .

		# save initial height of the top decoration (menu, title bar and border)
		if { $wndMenuHeight == 0 } {
			set curGeom [ wm geometry . ]
			scan $curGeom "%dx%d+%d+%d" width height decorationLeft decorationTop
			set contentsTop [ winfo rooty . ]
			set wndMenuHeight [ expr { $contentsTop - $decorationTop } ]
		}
	} else {
		set realW $w
	}

	set screenWidth [ winfo vrootwidth $realW ]
	set screenHeight [ winfo vrootheight $realW ]

	foreach wnd $wndLst {
		if { ! [ string compare $w all ] || ! [ string compare $w $wnd ] } {

			switch $wnd {

				.lsd {
					set defGeom "${hsizeBmin}x${vsizeBmin}+[ getx . topleftS ]+[ gety . topleftS ]"
					wm geometry . [ checkgeom $lsdGeom $defGeom $screenWidth $screenHeight ]
					wm minsize . $hsizeBmin $vsizeBmin
					wm maxsize . [ winfo vrootwidth . ] [ winfo vrootheight . ]
				}

				.lmm {
					if { $screenWidth < ( $hsizeL + 2 * $bordsize ) } {
						set W [ expr { $screenWidth - 2 * $bordsize } ]
					} else {
						set W $hsizeL
					}
					set H [ expr { $screenHeight - $tbarsize - 2 * $vmargin - 2 * $bordsize } ]
					if { $H < $vsizeL } {
						set H [ expr { $screenHeight - $tbarsize - 2 * $bordsize } ]
					}
					if { $screenWidth < ( $hsizeL + 2 * $bordsize + $hmargin ) } {
						set X 0
					} else {
						set X [ expr { $screenWidth - $hmargin - $bordsize - $W } ]
					}
					set Y [ expr { ( $screenHeight - $tbarsize ) / 2 - $bordsize - $H / 2 } ]

					set defGeom "${W}x$H+$X+$Y"

					wm geometry . [ checkgeom $lmmGeom $defGeom $screenWidth $screenHeight ]
					wm minsize . $hsizeLmin $vsizeLmin
					wm maxsize . [ winfo vrootwidth . ] [ winfo vrootheight . ]
				}

				.log {
					set defGeom "+[ expr { $screenWidth - $hmargin - $bordsize - $hsizeGmin } ]+[ gety .log bottomrightS ]"
					wm geometry .log [ checkgeom $logGeom $defGeom $screenWidth $screenHeight ]
					wm minsize .log $hsizeGmin $vsizeGmin
					wm maxsize .log [ winfo vrootwidth .log ] [ winfo vrootheight .log ]
				}

				.str {
					set posXstr [ expr { [ winfo x . ] + $corrX + $hmargin + [ winfo width . ] - 2 * $bordsize } ]
					set posYstr [ expr { [ winfo y . ] + $corrY } ]
					set defGeom "${hsizeM}x${vsizeM}+${posXstr}+${posYstr}"

					# handle the extra scaling parameters
					set geom [ split [ checkgeom $strGeom $defGeom $screenWidth $screenHeight ] ":" ]
					wm geometry .str [ lindex $geom 0 ]
					if { [ lindex $geom 1 ] != "" } {
						scan [ lindex $geom 1 ] "%f+%f" hfactM vfactM
					}

					wm minsize .str [ expr { $hsizeM / 2 } ] [ expr { $vsizeM / 2 } ]
					wm maxsize .str [ winfo vrootwidth .str ] [ winfo vrootheight .str ]
				}

				.da {
					set defGeom "+[ getx .da overM ]+[ gety .da overM ]"
					wm geometry .da [ checkgeom $daGeom $defGeom $screenWidth $screenHeight ]
					wm minsize .da $hsizeAmin $vsizeAmin
					wm maxsize .da [ winfo vrootwidth .da ] [ winfo vrootheight .da ]
					wm resizable .da 1 1
				}

				.deb {
					set defGeom "${hsizeDmin}x${vsizeDmin}+[ getx .deb topleftW ]+[ gety .deb topleftW ]"
					set n [ scan $debGeom "%dx%d+%d+%d" width height x y ]
					if { $n < 4 } {
						set debGeom $defGeom
					} else {
						set debGeom "${hsizeDmin}x[ expr { max ( $height, $vsizeDmin ) } ]+${x}+${y}"
					}
				
					wm geometry .deb [ checkgeom $debGeom $defGeom $screenWidth $screenHeight ]
					wm minsize .deb $hsizeDmin $vsizeDmin
					wm maxsize .deb [ winfo vrootwidth .deb ] [ winfo vrootheight .deb ]
					wm resizable .deb 0 1
				}

				.lat {
					set defGeom "+[ getx .lat centerS ]+[ gety .lat centerS ]"
					wm geometry .lat [ checkgeom $latGeom $defGeom $screenWidth $screenHeight ]
					wm minsize .lat [ winfo reqwidth .lat ] [ winfo reqheight .lat ]
					wm maxsize .lat [ winfo vrootwidth .lat ] [ winfo vrootheight .lat ]
					wm resizable .lat 0 0
				}
				
				.plt {
					set defGeom "+[ getx .plt righttoM ]+[ gety .plt righttoM ]"
					wm geometry .plt [ checkgeom $pltGeom $defGeom $screenWidth $screenHeight ]
					wm minsize .plt [ winfo reqwidth .plt ] [ winfo reqheight .plt ]
					wm maxsize .plt [ winfo vrootwidth .plt ] [ winfo vrootheight .plt ]
					wm resizable .plt 0 0
				}
				
				.dap {
					set defGeom "+[ getx .dap centerS ]+[ gety .dap centerS ]"
					wm geometry .dap [ checkgeom $dapGeom $defGeom $screenWidth $screenHeight ]
					wm minsize .dap $hsizePmin $vsizePmin
					wm maxsize .dap [ winfo vrootwidth .dap ] [ winfo vrootheight .dap ]
					wm resizable .dap 1 1
				}
			}
		}
	}

	update idletasks

	if { $logWndFn && [ info procs plog ] != "" } { 
		plog "\nsizetop (w:$w, master:[ wm transient $w ], pos:([ winfo x $w ],[ winfo y $w ]), size:[ winfo width $w ]x[ winfo height $w ], parWndLst:$parWndLst, grab:$grabLst)"
	}
}


#************************************************
# RESIZETOP
# Resize the window
#************************************************
proc resizetop { w sizeX { sizeY 0 } } {
	global hmargin vmargin bordsize tbarsize parWndLst grabLst logWndFn

	if { $sizeX <= 0 } {
		set sizeX [ winfo width $w ]
	}
	if { $sizeY <= 0 } {
		set sizeY [ winfo height $w ]
	}
	
	set sizeX [ expr { min( $sizeX, [ winfo vrootwidth $w ] - [ winfo rootx $w ] - 2 * $bordsize - $hmargin ) } ]
	set sizeY [ expr { min( $sizeY, [ winfo vrootheight $w ] - [ winfo rooty $w ] - 2 * $bordsize - $vmargin - $tbarsize ) } ]
	
	set newMinX [ expr { min( [ lindex [ wm minsize $w ] 0 ], $sizeX ) } ]
	set newMinY [ expr { min( [ lindex [ wm minsize $w ] 1 ], $sizeY ) } ]
	if { $newMinX != [ lindex [ wm minsize $w ] 0 ] || $newMinY != [ lindex [ wm minsize $w ] 1 ] } {
		wm minsize $w $newMinX $newMinY
	}
	if { $sizeX != [ winfo width $w ] || $sizeY != [ winfo height $w ] } {
		if { $sizeX > 0 && $sizeY > 0 } {
			wm geom $w ${sizeX}x${sizeY}
		}
	}
	
	update idletasks

	if { $logWndFn && [ info procs plog ] != "" } { 
		plog "\nresizetop (w:$w, master:[ wm transient $w ], pos:([ winfo x $w ],[ winfo y $w ]), size:[ winfo width $w ]x[ winfo height $w ], parWndLst:$parWndLst, grab:$grabLst)"
	}
}


#************************************************
# FOCUSTOP
# Set focus to the given window only if a
# LSD window has currently the system focus
#************************************************
proc focustop { w1 { w2 "" } { force no } } {

	if [ winfo exists $w1 ] {
		update idletasks
		
		set t1 [ winfo toplevel $w1 ]
		deiconifytop $t1 $force
		
		if { $w2 != "" && [ winfo exists $w2 ] && [ winfo toplevel $w2 ] != $t1 } {
			raise $t1 [ winfo toplevel $w2 ]
		} else {
			raise $t1
		}
		
		if { $force } {
			focus -force $w1
		} else {
			focus $w1
		}
	}
	
	update idletasks
}


#************************************************
# DEICONIFYTOP
# Deiconify/map window if not yet viewable
#************************************************
proc deiconifytop { w { force no } } {

	if { $force || ! [ winfo viewable $w ] } {
		wm deiconify $w
	}
	
	update idletasks
}


#************************************************
# ICONTOP
# Set window icon
#************************************************
proc icontop { w type } {
	global CurPlatform RootLsd LsdSrc lmmImg lsdImg iconExt

	if { $type == "" } {
		wm iconbitmap $w ""
		return
	}

	if [ string equal $CurPlatform windows ] {
		if [ string equal $w . ] {
			wm iconbitmap $w -default $RootLsd/$LsdSrc/icons/$type.ico
		} else {
			wm iconbitmap $w $RootLsd/$LsdSrc/icons/$type.ico
		}
	} elseif [ string equal $CurPlatform linux ] {
		if [ string equal $w . ] {
			wm iconphoto $w -default ${type}Img
			wm iconbitmap $w @$RootLsd/$LsdSrc/icons/$type.xbm
		} else {
			wm iconphoto $w ${type}Img
			wm iconbitmap $w @$RootLsd/$LsdSrc/icons/$type.xbm
		}
	}
}


#************************************************
# PLACELINE
# Place a line of slaves in a master
#************************************************
proc placeline { slvLst widLst y hgt { relX 1 } { relY 0 } } {

	if { [ llength $slvLst ] != [ llength $widLst ] } {
		error "\nplaceline: slave and width lists have different size"
		return
	}
	
	if { $relX } {
		set xCmd -relx
		set widCmd -relwidth
	} else {
		set xCmd -x
		set widCmd -width
	}
	
	if { $relY } {
		set yCmd "-rely $y"
		set hgtCmd "-relheight $hgt"
	} else {
		set yCmd "-y $y"
		set hgtCmd "-height $hgt"
	}
	
	set curX 0
	
	foreach w $slvLst wid $widLst {
		place $w {*}$xCmd $curX {*}$widCmd $wid {*}$yCmd {*}$hgtCmd
		
		set curX [ expr { $curX + $wid } ]
	}
}


#************************************************
# ALIGN
# Alignment of window w1 to the to right side of w2
#************************************************
proc align { w1 w2 { side R } } {
	global hmargin corrX corrY logWndFn

	set a [ winfo width $w1 ]
	set b [ winfo height $w1 ]
	set c [ expr { [ winfo x $w2 ] + $corrX } ]
	set d [ expr { [ winfo y $w2 ] + $corrY } ]
	set e [ winfo width $w1 ]
	set f [ winfo width $w2 ]

	if { $side == "L" } {
		set g [ expr { $c - $e - $hmargin } ]
	} else {
		set g [ expr { $c + $f + $hmargin } ]
	}
	
	wm geometry $w1 +$g+$d
	update idletasks

	if { $logWndFn && [ info procs plog ] != "" } { 
		plog "\nalign w1:$w1 w2:$w2 (w1 width:$a, w1 height:$b, w2 x:$c, w2 y:$d, w2 width:$e)"
	}
}


#************************************************
# PRIMDISP
# check if window center is in primary display
#************************************************
proc primdisp w {
	if { [ winfo rootx $w ] > 0 && [ winfo rootx $w ] < [ winfo screenwidth $w ] && [ winfo rooty $w ] > 0 && [ winfo rooty $w ] < [ winfo screenheight $w ] } {
		return true
	} else {
		return false
	}
}


#************************************************
# GETX
# compute x coordinate of new window according to the types
#************************************************
proc getx { w pos } {
	global corrX hmargin bordsize shiftW

	set par [ winfo parent $w ]
	if { $par == "" } {
		set par .
	}

	switch $pos {
		centerS {
			set hpos [ expr { [ winfo screenwidth $w ] / 2 + $corrX - [ winfo reqwidth $w ] / 2 } ]
		}
		centerW {
			set hpos [ expr { [ winfo x $par ] + $corrX + [ winfo width $par ] / 2  - [ winfo reqwidth $w ] / 2 } ]
		}
		topleftS {
			set hpos [ expr { $hmargin + $corrX } ]
		}
		topleftW {
			set hpos [ expr { [ winfo x $par ] + $corrX + 10 } ]
		}
		overM {
			set hpos [ expr { [ winfo x . ] + $corrX } ]
		}
		coverW {
			set hpos [ expr { [ winfo x $par ] + $corrX } ]
		}
		bottomrightS {
			set hpos [ expr { [ winfo screenwidth $w ] - $hmargin - 2 * $bordsize - [ winfo reqwidth $w ] } ]
		}
		righttoW {
			set hpos [ expr { [ winfo x $par ] + $corrX + $hmargin + [ winfo width $par ] + 2 * $bordsize } ]
		}
		lefttoW {
			set hpos [ expr { [ winfo x $par ] + $corrX - $hmargin - [ winfo reqwidth $w ] + 2 * $bordsize } ]
		}
		righttoM {
			set hpos [ expr { [ winfo x . ] + $corrX + $hmargin + [ winfo width . ] - 2 * $bordsize } ]
		}
		default {
			set hpos [ expr { [ winfo screenwidth $w ] / 2 - [ winfo reqwidth $w ] / 2 } ]
		}
	}

	if [ primdisp $par ] {
		if { $hpos < $corrX } {
			return $corrX
		}
		if { $hpos > [ expr { [ winfo screenwidth $par ] - [ winfo reqwidth $w ] / 2 } ] } {
			return [ expr { [ winfo x $par ] + $corrX + $hmargin + 2 * $bordsize } ]
		}
	}

	return $hpos
}


#************************************************
# GETY
# compute y coordinate of new window according to the types
#************************************************
proc gety { w pos } {
	global corrY vmargin tbarsize shiftW

	set par [ winfo parent $w ]
	if { $par == "" } {
		set par .
	}

	switch $pos {
		centerS {
			set vpos [ expr { [ winfo screenheight $w ] / 2 + $corrY - [ winfo reqheight $w ] / 2 } ]
		}
		centerW {
			set vpos [ expr { [ winfo y $par ] + $corrY + [ winfo height $par ] / 2  - [ winfo reqheight $w ] / 2 } ]
		}
		topleftS {
			set vpos [ expr { $vmargin + $corrY } ]
		}
		topleftW {
			set vpos [ expr { [ winfo y $par ] + $corrY + 30 } ]
		}
		overM {
			set vpos [ expr { [ winfo y . ] + $corrY } ]
		}
		coverW {
			set vpos [ expr { [ winfo y $par ] + $corrY } ]
		}
		bottomrightS {
			set vpos [ expr { [ winfo screenheight $w ] - $vmargin - $tbarsize - [ winfo reqheight $w ] } ]
		}
		righttoW {
			set vpos [ expr { [ winfo y $par ] + $corrY } ]
		}
		lefttoW {
			set vpos [ expr { [ winfo y $par ] + $corrY } ]
		}
		righttoM {
			set vpos [ expr { [ winfo y . ] + $corrY } ]
		}
		default {
			set vpos [ expr { [ winfo screenheight $w ] / 2 - [ winfo reqheight $w ] / 2 } ]
		}
	}

	if [ primdisp $par ] {
		if { $vpos < $corrY } {
			return $corrY
		}
		if { $vpos > [ expr { [ winfo screenheight $par ] - [ winfo reqheight $w ] / 2 } ] } {
			return [ expr { [ winfo y $par ] + $corrY } ]
		}
	}

	return $vpos
}


#************************************************
# DISABLE_WINDOW
# Command to disable windows in cases where grab is inappropriate
# call parameters are: container window, menu name, widgets names
#************************************************
proc disable_window { w m { args "" } } {
	
	if [ winfo exist $w.$m ] {
		for { set i 0 } { $i <= [ $w.$m index last ] } { incr i } {
			$w.$m entryconfig $i -state disabled
		}
	}
	
	foreach i $args {
		if [ winfo exists $w.$i ] {
			tk busy hold $w.$i
		}
	}
	
	update idletasks
}


#************************************************
# ENABLE_WINDOW
# Command to enable windows in cases where grab is inappropriate
# call parameters are: container window, menu name, widgets names
#************************************************
proc enable_window { w m { args "" } } {
	
	if [ winfo exist $w.$m ] {
		for { set i 0 } { $i <= [ $w.$m index last ] } { incr i } {
			$w.$m entryconfig $i -state normal
		}
	}
	
	foreach i $args {
		if [ winfo exists $w.$i ] {
			tk busy forget $w.$i
		}
	}
	
	update idletasks
}


#************************************************
# SETGLOBKEYS
# Set global key mappings in all windows
#************************************************
proc setglobkeys { w { chkChg 1 } } {
	global conWnd grabLst

	# soft/hard exit (check for unsaved changes or not)
	if { $chkChg } {
		bind $w <Control-Alt-x> { 
			if [ string equal [ discard_change ] ok ] { 
				exit 
			}
			break
		}
		bind $w <Control-Alt-X> { event generate . <Control-Alt-x> }
	} else {
		bind $w <Control-Alt-x> { exit }
		bind $w <Control-Alt-X> { event generate . <Control-Alt-x> }
	}
	# open Tcl/Tk console
	if { $conWnd } {
		bind $w <Control-Alt-j> {
			# remove existing grabs
			if [ info exists grabLst ] {
				set lastGrab [ expr { [ llength $grabLst ] - 1 } ]
				if { $lastGrab >= 0 } {
					grab release [ lindex [ lindex $grabLst $lastGrab ] 0 ]
					set grabLst [ list ]
				}
			}
			tk_console
			break
		}
		bind $w <Control-Alt-J> { event generate . <Control-Alt-j> }
	}
}


#************************************************
# SELECTINLIST
# Select, activate and show a listbox item
#************************************************
proc selectinlist { w pos { foc 0 } } {
	if { [ winfo exists $w ] && [ string equal -nocase [ winfo class $w ] listbox ] && [ $w size ] > 0 } {
		if [ string is integer -strict $pos ] {
			set pos [ expr { max( min( $pos, [ $w size ] - 1 ), 0 ) } ]
		} elseif { ! ( $pos in [ list active end ] ) } {
			return
		}
		
		$w selection clear 0 end
		$w selection set $pos
		$w activate $pos
		$w see $pos
		$w xview moveto 0
		
		if { $foc } {
			focus $w
		}
		
		update idletasks
	}
}


#************************************************
# ADDTOLIST
# Procedure to add a new plot to a listbox,
# selecting it
#************************************************
proc addtolist { w text } {
	$w insert end "$text"
	tooltip::tooltip $w -item [ expr { [ $w index end ] - 1 } ] "Double-click to show\nRight-click to delete"
	selectinlist $w end
}


#************************************************
# CANVASSEE
# Change the canvas scrollbars' positions to see
# completely an window item in canvas
#************************************************
proc canvassee { c w } {
	set bb [ $c bbox all ]
	set xv [ $c xview ]
	set yv [ $c yview ]
	set cw [ expr { [ lindex $bb 2 ] - [ lindex $bb 0 ] } ]
	set ch [ expr { [ lindex $bb 3 ] - [ lindex $bb 1 ] } ]
	set xmi [ expr { [ lindex $bb 0 ] + [ lindex $xv 0 ] * $cw } ]
	set xma [ expr { [ lindex $bb 0 ] + [ lindex $xv 1 ] * $cw } ]
	set ymi [ expr { [ lindex $bb 1 ] + [ lindex $yv 0 ] * $ch } ]
	set yma [ expr { [ lindex $bb 1 ] + [ lindex $yv 1 ] * $ch } ]
	set wx [ expr { [ $c canvasx [ winfo x $w ] ] - $xmi } ]
	set wy [ expr { [ $c canvasy [ winfo y $w ] ] - $ymi } ]
	set ww [ winfo width $w ]
	set wh [ winfo height $w ]
	if { $wx < $xmi } {
		$c xview moveto [ expr { [ lindex $xv 0 ] + ( $wx - $xmi ) / $cw } ]
	} elseif { [ expr { $wx + $ww } ] > $xma } {
		$c xview moveto [ expr { [ lindex $xv 0 ] + ( $wx + $ww - $xma ) / $cw } ]
	}
	if { $wy < $ymi } {
		$c yview moveto [ expr { [ lindex $yv 0 ] + ( $wy - $ymi ) / $ch } ]
	} elseif { [ expr { $wy + $wh } ] > $yma } {
		$c yview moveto [ expr { [ lindex $yv 0 ] + ( $wy + $wh - $yma ) / $ch } ]
	}
}


#************************************************
# INISELCELL
# Scroll table in canvas to show selected
# text cell, selecting its content
#************************************************
proc selectcell { can cell } {
	focus $cell
	$cell selection range 0 end
	canvassee $can $cell
}


#************************************************
# OKHELPCANCEL
# Procedure to create standard button set
#************************************************
proc okhelpcancel { w fr comOk comHelp comCancel } {
	global butWid butSpc butPad
	if [ string equal $w . ] { set w "" }
	if { ! [ winfo exists $w.$fr ] } { ttk::frame $w.$fr }
	ttk::button $w.$fr.ok -width $butWid -text OK -command $comOk
	ttk::button $w.$fr.help -width $butWid -text Help -command $comHelp
	ttk::button $w.$fr.cancel -width $butWid -text Cancel -command $comCancel
	bind $w.$fr.ok <KeyPress-Return> "$w.$fr.ok invoke"
	bind $w.$fr.help <KeyPress-Return> "$w.$fr.help invoke"
	bind $w.$fr.cancel <KeyPress-Return> "$w.$fr.cancel invoke"
	bind $w <KeyPress-Escape> "$w.$fr.cancel invoke"
	bind $w <F1> "$w.$fr.help invoke"
	pack $w.$fr.ok $w.$fr.help $w.$fr.cancel -padx $butSpc -side left
	pack $w.$fr -padx $butPad -pady $butPad -side right
}


#************************************************
# OKHELP
# Procedure to create standard button set
#************************************************
proc okhelp { w fr comOk comHelp } {
	global butWid butSpc butPad
	if [ string equal $w . ] { set w "" }
	if { ! [ winfo exists $w.$fr ] } { ttk::frame $w.$fr }
	ttk::button $w.$fr.ok -width $butWid -text OK -command $comOk
	ttk::button $w.$fr.help -width $butWid -text Help -command $comHelp
	bind $w.$fr.ok <KeyPress-Return> "$w.$fr.ok invoke"
	bind $w.$fr.help <KeyPress-Return> "$w.$fr.help invoke"
	bind $w <KeyPress-Escape> "$w.$fr.ok invoke"
	bind $w <F1> "$w.$fr.help invoke"
	pack $w.$fr.ok $w.$fr.help -padx $butSpc -side left
	pack $w.$fr -padx $butPad -pady $butPad -side right
}


#************************************************
# OKCANCEL
# Procedure to create standard button set
#************************************************
proc okcancel { w fr comOk comCancel } {
	global butWid butSpc butPad
	if [ string equal $w . ] { set w "" }
	if { ! [ winfo exists $w.$fr ] } { ttk::frame $w.$fr }
	ttk::button $w.$fr.ok -width $butWid -text OK -command $comOk
	ttk::button $w.$fr.cancel -width $butWid -text Cancel -command $comCancel
	bind $w.$fr.ok <KeyPress-Return> "$w.$fr.ok invoke"
	bind $w.$fr.cancel <KeyPress-Return> "$w.$fr.cancel invoke"
	bind $w <KeyPress-Escape> "$w.$fr.cancel invoke"
	pack $w.$fr.ok $w.$fr.cancel -padx $butSpc -side left
	pack $w.$fr -padx $butPad -pady $butPad -side right
}


#************************************************
# HELPCANCEL
# Procedure to create standard button set
#************************************************
proc helpcancel { w fr comHelp comCancel } {
	global butWid butSpc butPad
	if [ string equal $w . ] { set w "" }
	if { ! [ winfo exists $w.$fr ] } { ttk::frame $w.$fr }
	ttk::button $w.$fr.help -width $butWid -text Help -command $comHelp
	ttk::button $w.$fr.cancel -width $butWid -text Cancel -command $comCancel
	bind $w.$fr.help <KeyPress-Return> "$w.$fr.help invoke"
	bind $w.$fr.cancel <KeyPress-Return> "$w.$fr.cancel invoke"
	bind $w <KeyPress-Escape> "$w.$fr.cancel invoke"
	bind $w <F1> "$w.$fr.help invoke"
	pack $w.$fr.help $w.$fr.cancel -padx $butSpc -side left
	pack $w.$fr -padx $butPad -pady $butPad -side right
}


#************************************************
# OK
# Procedure to create standard button set
#************************************************
proc ok { w fr comOk } {
	global butWid butSpc butPad
	if [ string equal $w . ] { set w "" }
	if { ! [ winfo exists $w.$fr ] } { ttk::frame $w.$fr }
	ttk::button $w.$fr.ok -width $butWid -text OK -command $comOk
	bind $w.$fr.ok <KeyPress-Return> "$w.$fr.ok invoke"
	bind $w <KeyPress-Escape> "$w.$fr.ok invoke"
	pack $w.$fr.ok -padx $butSpc -side left
	pack $w.$fr -padx $butPad -pady $butPad -side right
}


#************************************************
# CANCEL
# Procedure to create standard button set
#************************************************
proc cancel { w fr comCancel } {
	global butWid butSpc butPad
	if [ string equal $w . ] { set w "" }
	if { ! [ winfo exists $w.$fr ] } { ttk::frame $w.$fr }
	ttk::button $w.$fr.cancel -width $butWid -text Cancel -command $comCancel
	bind $w.$fr.cancel <KeyPress-Return> "$w.$fr.cancel invoke"
	bind $w <KeyPress-Escape> "$w.$fr.cancel invoke"
	pack $w.$fr.cancel -padx $butSpc -side left
	pack $w.$fr -padx $butPad -pady $butPad -side right
}


#************************************************
# XCANCEL
# Procedure to create standard button set
#************************************************
proc Xcancel { w fr nameX comX comCancel } {
	global butWid butSpc butPad
	if [ string equal $w . ] { set w "" }
	if { ! [ winfo exists $w.$fr ] } { ttk::frame $w.$fr }
	if { [ string length "$nameX" ] > $butWid } {
		set Xwid [ string length "$nameX" ]
	} else {
		set Xwid $butWid
	}
	ttk::button $w.$fr.ok -width $Xwid -text $nameX -command $comX
	ttk::button $w.$fr.cancel -width $butWid -text Cancel -command $comCancel
	bind $w.$fr.ok <KeyPress-Return> "$w.$fr.ok invoke"
	bind $w.$fr.cancel <KeyPress-Return> "$w.$fr.cancel invoke"
	bind $w <KeyPress-Escape> "$w.$fr.cancel invoke"
	pack $w.$fr.ok $w.$fr.cancel -padx $butSpc -side left
	pack $w.$fr -padx $butPad -pady $butPad -side right
}


#************************************************
# OKXHELPCANCEL
# Procedure to create standard button set
#************************************************
proc okXhelpcancel { w fr nameX comX comOk comHelp comCancel } {
	global butWid butSpc butPad
	if [ string equal $w . ] { set w "" }
	if { ! [ winfo exists $w.$fr ] } { ttk::frame $w.$fr }
	if { [ string length "$nameX" ] > $butWid } {
		set Xwid [ string length "$nameX" ]
	} else {
		set Xwid $butWid
	}
	ttk::button $w.$fr.ok -width $butWid -text OK -command $comOk
	ttk::button $w.$fr.x -width $Xwid -text $nameX -command $comX
	ttk::button $w.$fr.help -width $butWid -text Help -command $comHelp
	ttk::button $w.$fr.cancel -width $butWid -text Cancel -command $comCancel
	bind $w.$fr.ok <KeyPress-Return> "$w.$fr.ok invoke"
	bind $w.$fr.x <KeyPress-Return> "$w.$fr.x invoke"
	bind $w.$fr.help <KeyPress-Return> "$w.$fr.help invoke"
	bind $w.$fr.cancel <KeyPress-Return> "$w.$fr.cancel invoke"
	bind $w <KeyPress-Escape> "$w.$fr.cancel invoke"
	bind $w <F1> "$w.$fr.help invoke"
	pack $w.$fr.ok $w.$fr.x $w.$fr.help $w.$fr.cancel -padx $butSpc -side left
	pack $w.$fr -padx $butPad -pady $butPad -side right
}


#************************************************
# XYOKHELPCANCEL
# Procedure to create standard button set
#************************************************
proc XYokhelpcancel { w fr nameX nameY comX comY comOk comHelp comCancel } {
	global butWid butSpc butPad
	if [ string equal $w . ] { set w "" }
	if { ! [ winfo exists $w.$fr ] } { ttk::frame $w.$fr }
	if { [ string length "$nameX" ] > $butWid } {
		set Xwid [ string length "$nameX" ]
	} else {
		set Xwid $butWid
	}
	if { [ string length "$nameY" ] > $butWid } {
		set Ywid [ string length "$nameY" ]
	} else {
		set Ywid $butWid
	}
	ttk::frame $w.$fr.r1
	ttk::button $w.$fr.r1.x -width $Xwid -text $nameX -command $comX
	ttk::button $w.$fr.r1.y -width $Ywid -text $nameY -command $comY
	ttk::frame $w.$fr.r2
	ttk::button $w.$fr.r2.ok -width $butWid -text OK -command $comOk
	ttk::button $w.$fr.r2.help -width $butWid -text Help -command $comHelp
	ttk::button $w.$fr.r2.cancel -width $butWid -text Cancel -command $comCancel
	bind $w.$fr.r1.x <KeyPress-Return> "$w.$fr.r1.x invoke"
	bind $w.$fr.r1.y <KeyPress-Return> "$w.$fr.r1.y invoke"
	bind $w.$fr.r2.ok <KeyPress-Return> "$w.$fr.r2.ok invoke"
	bind $w.$fr.r2.help <KeyPress-Return> "$w.$fr.r2.help invoke"
	bind $w.$fr.r2.cancel <KeyPress-Return> "$w.$fr.r2.cancel invoke"
	bind $w <KeyPress-Escape> "$w.$fr.r2.cancel invoke"
	bind $w <F1> "$w.$fr.r2.help invoke"
	pack $w.$fr.r1.x $w.$fr.r1.y -padx $butSpc -side left
	pack $w.$fr.r2.ok $w.$fr.r2.help $w.$fr.r2.cancel -padx $butSpc -side left
	pack $w.$fr.r1 -pady $butSpc -anchor e
	pack $w.$fr.r2 -pady $butSpc -anchor w
	pack $w.$fr -padx $butPad -pady $butPad -side right
}


#************************************************
# XYZOKHELPCANCEL
# Procedure to create standard button set
#************************************************
proc XYZokhelpcancel { w fr nameX nameY nameZ comX comY comZ comOk comHelp comCancel } {
	global butWid butSpc butPad
	if [ string equal $w . ] { set w "" }
	if { ! [ winfo exists $w.$fr ] } { ttk::frame $w.$fr }
	if { [ string length "$nameX" ] > $butWid } {
		set Xwid [ string length "$nameX" ]
	} else {
		set Xwid $butWid
	}
	if { [ string length "$nameY" ] > $butWid } {
		set Ywid [ string length "$nameY" ]
	} else {
		set Ywid $butWid
	}
	ttk::frame $w.$fr.r1
	ttk::button $w.$fr.r1.x -width $Xwid -text $nameX -command $comX
	ttk::button $w.$fr.r1.y -width $Ywid -text $nameY -command $comY
	ttk::button $w.$fr.r1.z -width $Ywid -text $nameZ -command $comZ
	ttk::frame $w.$fr.r2
	ttk::button $w.$fr.r2.ok -width $butWid -text OK -command $comOk
	ttk::button $w.$fr.r2.help -width $butWid -text Help -command $comHelp
	ttk::button $w.$fr.r2.cancel -width $butWid -text Cancel -command $comCancel
	bind $w.$fr.r1.x <KeyPress-Return> "$w.$fr.r1.x invoke"
	bind $w.$fr.r1.y <KeyPress-Return> "$w.$fr.r1.y invoke"
	bind $w.$fr.r1.z <KeyPress-Return> "$w.$fr.r1.z invoke"
	bind $w.$fr.r2.ok <KeyPress-Return> "$w.$fr.r2.ok invoke"
	bind $w.$fr.r2.help <KeyPress-Return> "$w.$fr.r2.help invoke"
	bind $w.$fr.r2.cancel <KeyPress-Return> "$w.$fr.r2.cancel invoke"
	bind $w <KeyPress-Escape> "$w.$fr.r2.cancel invoke"
	bind $w <F1> "$w.$fr.r2.help invoke"
	pack $w.$fr.r1.x $w.$fr.r1.y $w.$fr.r1.z -padx $butSpc -side left
	pack $w.$fr.r2.ok $w.$fr.r2.help $w.$fr.r2.cancel -padx $butSpc -side left
	pack $w.$fr.r1 -pady $butSpc -anchor e
	pack $w.$fr.r2 -pady $butSpc -anchor w
	pack $w.$fr -padx $butPad -pady $butPad -side right
}


#************************************************
# DONEHELP
# Procedure to create standard button set
#************************************************
proc donehelp { w fr comDone comHelp } {
	global butWid butSpc butPad
	if [ string equal $w . ] { set w "" }
	if { ! [ winfo exists $w.$fr ] } { ttk::frame $w.$fr }
	ttk::button $w.$fr.ok -width $butWid -text Done -command $comDone
	ttk::button $w.$fr.help -width $butWid -text Help -command $comHelp
	bind $w.$fr.ok <KeyPress-Return> "$w.$fr.ok invoke"
	bind $w.$fr.help <KeyPress-Return> "$w.$fr.help invoke"
	bind $w <KeyPress-Escape> "$w.$fr.ok invoke"
	bind $w <F1> "$w.$fr.help invoke"
	pack $w.$fr.ok $w.$fr.help -padx $butSpc -side left
	pack $w.$fr -padx $butPad -pady $butPad -side right
}


#************************************************
# DONE
# Procedure to create standard button set
#************************************************
proc done { w fr comDone } {
	global butWid butSpc butPad
	if [ string equal $w . ] { set w "" }
	if { ! [ winfo exists $w.$fr ] } { ttk::frame $w.$fr }
	ttk::button $w.$fr.ok -width $butWid -text Done -command $comDone
	bind $w.$fr.ok <KeyPress-Return> "$w.$fr.ok invoke"
	bind $w <KeyPress-Escape> "$w.$fr.ok invoke"
	pack $w.$fr.ok -padx $butSpc -side left
	pack $w.$fr -padx $butPad -pady $butPad -side right
}


#************************************************
# COMPHELPDONE
# Procedure to create standard button set
#************************************************
proc comphelpdone { w fr comComp comHelp comDone } {
	global butWid butSpc butPad
	if [ string equal $w . ] { set w "" }
	if { ! [ winfo exists $w.$fr ] } { ttk::frame $w.$fr }
	ttk::button $w.$fr.comp -width $butWid -text Compute -command $comComp
	ttk::button $w.$fr.help -width $butWid -text Help -command $comHelp
	ttk::button $w.$fr.ok -width $butWid -text Done -command $comDone
	bind $w.$fr.comp <KeyPress-Return> "$w.$fr.comp invoke"
	bind $w.$fr.help <KeyPress-Return> "$w.$fr.help invoke"
	bind $w.$fr.ok <KeyPress-Return> "$w.$fr.ok invoke"
	bind $w <KeyPress-Escape> "$w.$fr.ok invoke"
	bind $w <F1> "$w.$fr.help invoke"
	pack $w.$fr.comp $w.$fr.help $w.$fr.ok -padx $butSpc -side left
	pack $w.$fr -padx $butPad -pady $butPad -side right
}


#************************************************
# FINDHELPDONE
# Procedure to create standard button set
#************************************************
proc findhelpdone { w fr comFind comHelp comDone } {
	global butWid butSpc butPad
	if [ string equal $w . ] { set w "" }
	if { ! [ winfo exists $w.$fr ] } { ttk::frame $w.$fr }
	ttk::button $w.$fr.search -width $butWid -text Find -command $comFind
	ttk::button $w.$fr.help -width $butWid -text Help -command $comHelp
	ttk::button $w.$fr.ok -width $butWid -text Done -command $comDone
	bind $w.$fr.search <KeyPress-Return> "$w.$fr.search invoke"
	bind $w.$fr.help <KeyPress-Return> "$w.$fr.help invoke"
	bind $w.$fr.ok <KeyPress-Return> "$w.$fr.ok invoke"
	bind $w <KeyPress-Escape> "$w.$fr.ok invoke"
	bind $w <F1> "$w.$fr.help invoke"
	pack $w.$fr.search $w.$fr.help $w.$fr.ok -padx $butSpc -side left
	pack $w.$fr -padx $butPad -pady $butPad -side right
}


#************************************************
# SAVE
# Procedure to create standard button set
#************************************************
proc save { w fr comSave } {
	global butWid butSpc butPad
	if [ string equal $w . ] { set w "" }
	if { ! [ winfo exists $w.$fr ] } { ttk::frame $w.$fr }
	ttk::button $w.$fr.ok -width $butWid -text Save -command $comSave
	bind $w.$fr.ok <KeyPress-Return> "$w.$fr.ok invoke"
	pack $w.$fr.ok -padx $butSpc -side left
	pack $w.$fr -padx $butPad -pady $butPad -side right
}


#************************************************
# YESNO
# Procedure to create standard button set
#************************************************
# procedures to create standard button sets
proc yesno { w fr comYes comNo } {
	global butWid butSpc butPad
	if [ string equal $w . ] { set w "" }
	if { ! [ winfo exists $w.$fr ] } { ttk::frame $w.$fr }
	ttk::button $w.$fr.yes -width $butWid -text Yes -command $comYes
	ttk::button $w.$fr.no -width $butWid -text No -command $comNo
	bind $w.$fr.yes <KeyPress-Return> "$w.$fr.yes invoke"
	bind $w.$fr.no <KeyPress-Return> "$w.$fr.no invoke"
	pack $w.$fr.yes $w.$fr.no -padx $butSpc -side left
	pack $w.$fr -padx $butPad -pady $butPad -side right
}


#************************************************
# YESNOCANCEL
# Procedure to create standard button set
#************************************************
# procedures to create standard button sets
proc yesnocancel { w fr comYes comNo comCancel } {
	global butWid butSpc butPad
	if [ string equal $w . ] { set w "" }
	if { ! [ winfo exists $w.$fr ] } { ttk::frame $w.$fr }
	ttk::button $w.$fr.yes -width $butWid -text Yes -command $comYes
	ttk::button $w.$fr.no -width $butWid -text No -command $comNo
	ttk::button $w.$fr.cancel -width $butWid -text Cancel -command $comCancel
	bind $w.$fr.yes <KeyPress-Return> "$w.$fr.yes invoke"
	bind $w.$fr.no <KeyPress-Return> "$w.$fr.no invoke"
	bind $w.$fr.cancel <KeyPress-Return> "$w.$fr.cancel invoke"
	bind $w <KeyPress-Escape> "$w.$fr.cancel invoke"
	pack $w.$fr.yes $w.$fr.no $w.$fr.cancel -padx $butSpc -side left
	pack $w.$fr -padx $butPad -pady $butPad -side right
}


#************************************************
# RETRYCANCEL
# Procedure to create standard button set
#************************************************
proc retrycancel { w fr comRetry comCancel } {
	global butWid butSpc butPad
	if [ string equal $w . ] { set w "" }
	if { ! [ winfo exists $w.$fr ] } { ttk::frame $w.$fr }
	ttk::button $w.$fr.retry -width $butWid -text Retry -command $comRetry
	ttk::button $w.$fr.cancel -width $butWid -text Cancel -command $comCancel
	bind $w.$fr.retry <KeyPress-Return> "$w.$fr.retry invoke"
	bind $w.$fr.cancel <KeyPress-Return> "$w.$fr.cancel invoke"
	bind $w <KeyPress-Escape> "$w.$fr.cancel invoke"
	pack $w.$fr.retry $w.$fr.cancel -padx $butSpc -side left
	pack $w.$fr -padx $butPad -pady $butPad -side right
}


#************************************************
# ABORTRETRYIGNORE
# Procedure to create standard button set
#************************************************
# procedures to create standard button sets
proc abortretryignore { w fr comAbort comRetry comIgnore } {
	global butWid butSpc butPad
	if [ string equal $w . ] { set w "" }
	if { ! [ winfo exists $w.$fr ] } { ttk::frame $w.$fr }
	ttk::button $w.$fr.abort -width $butWid -text Abort -command $comAbort
	ttk::button $w.$fr.retry -width $butWid -text Retry -command $comRetry
	ttk::button $w.$fr.ignore -width $butWid -text Ignore -command $comIgnore
	bind $w.$fr.abort <KeyPress-Return> "$w.$fr.abort invoke"
	bind $w.$fr.retry <KeyPress-Return> "$w.$fr.retry invoke"
	bind $w.$fr.ignore <KeyPress-Return> "$w.$fr.ignore invoke"
	bind $w <KeyPress-Escape> "$w.$fr.ignore invoke"
	pack $w.$fr.abort $w.$fr.retry $w.$fr.ignore -padx $butSpc -side left
	pack $w.$fr -padx $butPad -pady $butPad -side right
}


#************************************************
# MOUSEWARPTO
# Move the mouse pointer to widget (button) w
# If disableMouseWarp is 1, does nothing
#************************************************
proc mousewarpto w {
	global mouseWarp curX curY
	
	update
	
	if { $mouseWarp && [ winfo exists $w ] && [ winfo viewable $w ] } {
		set wX [ expr { [ winfo width $w ] / 2 } ]
		set wY [ expr { [ winfo height $w ] / 2 } ]
		set curX 0
		set curY 0
		
		bind $w <Motion> {
			set curX %x
			set curY %y
		}
		
		after 100
		focus $w
		
		# first move pointer to toplevel to bypass Tk bug
		set t [ winfo toplevel $w ]
		event generate $t <Motion> -warp 1 -x [ expr { [ winfo width $t ] / 2 } ] -y [ expr { [ winfo height $t ] / 2 } ]
		update idletasks

		# do it as required to bypass Tk bug (first warps just go to the dialog not the button)
		for { set tries 0 } { ( $curX != $wX || $curY != $wY ) && $tries < 10 } { incr tries } {
			event generate $w <Motion> -warp 1 -x $wX -y $wY
			update idletasks
		}
		
		bind $w <Motion> { }
		unset curX curY
	}
	
	update idletasks
}


#************************************************
# ISMOUSESNAPON
# Check for a system mouse auto-snap mode
#************************************************
proc ismousesnapon { platform } {
	if [ string equal $platform mac ] {
		return 0
	} elseif [ string equal $platform linux ] {
		return 0
	} elseif [ string equal $platform windows ] {
		package require registry
		if { ! [ catch { set SnapToDefaultButton [ registry get "HKEY_CURRENT_USER\\Control Panel\\Mouse" SnapToDefaultButton ] } ] } {
			if { $SnapToDefaultButton } {
				return 1
			}
		}
	}
	return 0
}


#************************************************
# WRITE_ANY
# Read any entry widget (normal or disabled)
#************************************************
proc write_any { w val } {
	if { ! [ winfo exists $w ] } {
		return 0
	}
	
	if [ string equal [ $w cget -state ] disabled ] {
		return [ write_disabled $w $val ]
	} else {
		if [ catch {
			$w delete 0 end
			$w insert 0 $val
		} ] {
			return 0
		}
	}
	
	return 1
}


#************************************************
# WRITE_DISABLED
# Update a disabled entry widget (do nothing if normal state)
#************************************************
proc write_disabled { w val } {
	if { ! [ winfo exists $w ] } {
		return 0
	}
	
	if [ string equal [ $w cget -state ] disabled ] {
		$w conf -state normal
		write_any $w $val
		$w conf -state disabled
	} else {
		return 0
	}
	
	return 1
}


#************************************************
# UPDATE_TITLE_BAR
# Update LMM main window title bar according to file save status
#************************************************
proc update_title_bar { } {
	global tosave before filename

	if [ winfo exists .f.t.t ] {
		set after [ .f.t.t get 1.0 end ]
	} else {
		set after $before
	}
	if [ string compare $before $after ] {
		set tosave 1
		wm title . "*$filename - LMM"
	} else {
		set tosave 0
		wm title . "  $filename - LMM"
	}
}


#************************************************
# SETWRAP
# Procedure to adjust text wrapping
#************************************************
proc setwrap { w wrap } {
	if { $wrap == 1 } {
		$w conf -wrap word
	} else {
		$w conf -wrap none }
}


#************************************************
# MOUSE_WHEEL
# Bind the mouse wheel to the y scrollbar of 
# the window w or the first (grand) parent with 
# a vertical scrollbar
#************************************************
proc mouse_wheel { w } {
	global CurPlatform

	if [ string equal $CurPlatform windows ] {
		bind $w <MouseWheel> { scroll_wheel_windows %D %W }
		bind $w <Shift-MouseWheel> { scroll_wheel_windows %D %W }
		bind $w <Control-MouseWheel> { scroll_wheel_windows %D %W }
		bind $w <Alt-MouseWheel> { scroll_wheel_windows %D %W }
	} elseif [ string equal $CurPlatform mac ] {
		bind $w <MouseWheel> { scroll_wheel_mac %D %W }
		bind $w <Shift-MouseWheel> { scroll_wheel_mac %D %W }
		bind $w <Control-MouseWheel> { scroll_wheel_mac %D %W }
	} else {
		bind $w <4> { scroll_wheel_linux %D %W -1 }
		bind $w <5> { scroll_wheel_linux %D %W 1 }
		bind $w <Shift-4> { scroll_wheel_linux %D %W -1 }
		bind $w <Shift-5> { scroll_wheel_linux %D %W 1 }
		bind $w <Control-4> { scroll_wheel_linux %D %W -1 }
		bind $w <Control-5> { scroll_wheel_linux %D %W 1 }
	}
}


#************************************************
# SCROLL_WHEEL_WINDOWS
# Handle scroll wheel event in Windows
#************************************************
proc scroll_wheel_windows { delta w } {
	global winmwscale sfmwheel
	
	if { [ expr { abs( $delta ) } ] < $winmwscale } { 
		set winmwscale [ expr { abs( $delta ) } ]
		if { $winmwscale <= 0 } { 
			set winmwscale 1
		}
	}
	
	set scrW [ find_scrollable $w ]
	if { $scrW != "" } {
		set wPos [ $scrW yview ]
		set top [ lindex $wPos 0 ]
		set bot [ lindex $wPos 1 ]
		if { ( ! ( $top == 0.0 && $bot == 1.0 ) ) && \
			 ( ( $delta > 0 && $top > 0.0 ) || ( $delta < 0 && $bot < 1.0 ) ) } {
			$scrW yview scroll [ expr { -1 * $sfmwheel * $delta / $winmwscale } ] units
		}
	}
}


#************************************************
# SCROLL_WHEEL_MAC
# Handle scroll wheel event in macOS
#************************************************
proc scroll_wheel_mac { delta w } {
	global sfmwheel
	
	set scrW [ find_scrollable $w ]
	if { $scrW != "" } {
		set wPos [ $scrW yview ]
		set top [ lindex $wPos 0 ]
		set bot [ lindex $wPos 1 ]
		if { ( ! ( $top == 0.0 && $bot == 1.0 ) ) && \
			 ( ( $delta > 0 && $top > 0.0 ) || ( $delta < 0 && $bot < 1.0 ) ) } {
			$scrW yview scroll [ expr { -1 * $sfmwheel * $delta } ] units
		}
	}
}


#************************************************
# SCROLL_WHEEL_LINUX
# Handle scroll wheel event in Linux
#************************************************
proc scroll_wheel_linux { delta w dir } {
	global sfmwheel
	
	set scrW [ find_scrollable $w ]
	if { $scrW != "" } {
		set wPos [ $scrW yview ]
		set top [ lindex $wPos 0 ]
		set bot [ lindex $wPos 1 ]
		if { ( ! ( $top == 0.0 && $bot == 1.0 ) ) && \
			 ( ( $dir < 0 && $top > 0.0 ) || ( $dir > 0 && $bot < 1.0 ) ) } {
			$scrW yview scroll [ expr { $dir * $sfmwheel } ] units
		}
	}
}


#************************************************
# FIND_SCROLLABLE
# Find the first window over w, including itself
# which has a vertical scrollbar
#************************************************
proc find_scrollable { w } {
	for { set scrW $w } \
		{ [ winfo exists $scrW ] && [ winfo class $scrW ] != "Toplevel" } \
		{ set scrW [ winfo parent $scrW ] } {
		if { [ winfo class $scrW ] in { Text Canvas Listbox } && \
			 [ $scrW cget -yscrollcommand ] != "" } {
			return $scrW
		}
	}
	return ""
}


#************************************************
# MOVE_CANVAS
# Move all items in canvas to point (x,y) (from (x0,y0))
#************************************************
proc move_canvas { c x y { x0 -1 } { y0 -1 } } {
	global hereX hereY

	if { $x0 > 0 } { set hereX $x0 }
	if { $y0 > 0 } { set hereY $y0 }

	if { [ info exists hereX ] && [ info exists hereY ] } {
		$c move all [ expr { $x - $hereX } ] [ expr { $y - $hereY } ]
    }
    set hereX $x
    set hereY $y
}


#************************************************
# SCALE_CANVAS
# Scale all items in canvas around point (0,0), including the scroll region
#************************************************
proc scale_canvas { c type ratio } {
	global vsizeP tbordsizeP bbordsizeP maxzoomP minzoomP
	upvar $ratio finalRatio

    if { $type == "+" } {
        set ratio [ expr { sqrt( 2.0 ) } ]
    } else {
        set ratio [ expr { 1.0 / sqrt( 2.0 ) } ]
    }

	set sro [ $c cget -scrollregion ]

	set newH [ expr { ( [ lindex $sro 3 ] - [ lindex $sro 1 ] ) * $ratio } ]
	set origH [ expr { $vsizeP + $tbordsizeP + $bbordsizeP } ]
	set newRatio [ expr { $newH / $origH } ]

	if { $newRatio < $maxzoomP && $newRatio > $minzoomP } {
		set srn [ list ]
		for { set i 0 } { $i < 4 } { incr i } {
			lappend srn [ expr { round( [ lindex $sro $i ] * $ratio ) } ]
		}
		$c configure -scrollregion $srn
		$c scale all 0 0 $ratio $ratio
		set finalRatio $newRatio
	}
}


#************************************************
# CANVAS_AXIS
# Create the canvas plotting axes and the optional grid
#************************************************
proc canvas_axis { c type grid hticks { y2 0 } } {
	global hsizeP vsizeP hbordsizeP tbordsizeP vticksP colorsTheme

	# x-axis, ticks & x-grid
	$c create line $hbordsizeP $tbordsizeP [ expr { $hbordsizeP + $hsizeP } ] $tbordsizeP -width 1 -fill $colorsTheme(dfg) -tag p
	$c create line $hbordsizeP [ expr { $tbordsizeP + $vsizeP } ] [ expr { $hbordsizeP + $hsizeP } ] [ expr { $tbordsizeP + $vsizeP } ] -width 1 -fill $colorsTheme(dfg) -tag p

	if { $type == 0 || $type == 4 || $type == 5 } {
		for { set i 0 } { $i < [ expr { $hticks + 2 } ] } { incr i } {
			if { $grid && $i > 0 && $i < [ expr { $hticks + 1 } ] } {
				$c create line [ expr { $hbordsizeP + round( $i * $hsizeP / ( $hticks + 1 ) ) } ] [ expr { $tbordsizeP + 1 } ] [ expr { $hbordsizeP + round( $i * $hsizeP / ( $hticks + 1 ) ) } ] [ expr { $tbordsizeP + $vsizeP - 1 } ] -fill $colorsTheme(bg) -width 1 -tags {g p}
			}
			$c create line [ expr { $hbordsizeP + round( $i * $hsizeP / ( $hticks + 1 ) ) } ] [ expr { $tbordsizeP + $vsizeP } ] [ expr { $hbordsizeP + round( $i * $hsizeP / ( $hticks + 1 ) ) } ] [ expr { $tbordsizeP + $vsizeP + 5 } ] -fill $colorsTheme(dfg) -width 1 -tags p
		}
	}

	# y-axis, ticks & y-grid
	$c create line $hbordsizeP $tbordsizeP $hbordsizeP [ expr { $tbordsizeP + $vsizeP } ] -width 1 -fill $colorsTheme(dfg) -tag p
	$c create line [ expr { $hbordsizeP + $hsizeP } ]  $tbordsizeP [ expr { $hbordsizeP + $hsizeP } ] [ expr { $tbordsizeP + $vsizeP } ] -width 1 -fill $colorsTheme(dfg) -tag p

	for { set i 0 } { $i < [ expr { $vticksP + 2 } ] } { incr i } {
		if { $grid && $i > 0 && $i < [ expr { $vticksP + 1 } ] } {
			$c create line [ expr { $hbordsizeP + 1 } ] [ expr { $tbordsizeP + round( $i * $vsizeP / ( $vticksP + 1 ) ) } ] [ expr { $hbordsizeP + $hsizeP - 1 } ] [ expr { $tbordsizeP + round( $i * $vsizeP / ( $vticksP + 1 ) ) } ] -fill $colorsTheme(bg) -width 1 -tags {g p}
		}
		$c create line [ expr { $hbordsizeP - 5 } ] [ expr { $tbordsizeP + round( $i * $vsizeP / ( $vticksP + 1 ) ) } ] $hbordsizeP [ expr { $tbordsizeP + round( $i * $vsizeP / ( $vticksP + 1 ) ) } ] -fill $colorsTheme(dfg) -width 1 -tag p
		if $y2 {
			$c create line [ expr { $hbordsizeP + $hsizeP } ] [ expr { $tbordsizeP + round( $i * $vsizeP / ( $vticksP + 1 ) ) } ] [ expr { $hbordsizeP + $hsizeP + 5 } ] [ expr { $tbordsizeP + round( $i * $vsizeP / ( $vticksP + 1 ) ) } ] -fill $colorsTheme(dfg) -width 1 -tag p
		}
	}
}


#************************************************
# PLOT_BARS
# plot color filled bars
#************************************************
proc plot_bars { c x1 y1 x2 y2 { tags "" } { fill "" } { width 1 } { outline "" } } {
	global colorsTheme
	
	if { $fill == "" } {
		set fill $colorsTheme(bg)
	}
	
	if { $outline == "" } {
		set outline $colorsTheme(fg)
	}
	
	set size [ expr { min( [ llength $x1 ], [ llength $y1 ], [ llength $x2 ], [ llength $y2 ]  ) } ]
	lappend tags bar series

	for { set i 0 } { $i < $size } { incr i } {
		# valid point?
		set x1i [ lindex $x1 $i ]
		set y1i [ lindex $y1 $i ]
		set x2i [ lindex $x2 $i ]
		set y2i [ lindex $y2 $i ]
		if { $x1i != $x2i && $y1i != $y2i } {
			# plot bar
			$c create rectangle $x1i $y1i $x2i $y2i -fill $fill -width $width -outline $outline -tags $tags
		}
	}
}


#************************************************
# PLOT_LINE
# plot one series as a line on canvas (may be discontinuous)
#************************************************
proc plot_line { c x y { tags "" } { fill c0 } { width 1 } } {
	global smoothP splstepsP

	set size [ expr { min( [ llength $x ], [ llength $y ] ) } ]
	set xy [ list ]
	set tagsdots $tags
	lappend tags line series
	lappend tagsdots dots series

	for { set i 0 } { $i < $size } { incr i } {
		# valid point?
		set xi [ lindex $x $i ]
		set yi [ lindex $y $i ]
		if { $xi >= 0 && $yi >= 0 } {
			# add one more segment
			lappend xy $xi $yi
		} else { # last point in segment?
			set lenxy [ llength $xy ]
			if { $lenxy > 0 } {
				# plot line segment/single point till now
				if { $lenxy >= 4 } {
					$c create line $xy -width $width -fill $fill -smooth $smoothP \
						-splinesteps $splstepsP -tags $tags
				} else {
					if { $lenxy == 2 } {
						set xi [ lindex $xy 0 ]
						set yi [ lindex $xy 1 ]
						# x
						$c create line [ expr { $xi + 2 } ] [ expr { $yi + 2 } ] \
							[ expr { $xi - 3 } ] [ expr { $yi - 3 } ] \
							-width 1 -fill $fill -tags $tagsdots
						$c create line [ expr { $xi + 2 } ] [ expr { $yi - 2 } ] \
							[ expr { $xi - 3 } ] [ expr { $yi + 3 } ] \
							-width 1 -fill $fill -tags $tagsdots
					}
				}
				# restart line
				set xy [ list ]
			}
		}
	}

	# plot last segment, if any
	if { [ llength $xy ] >= 4 } {
		$c create line $xy -width $width -fill $fill -tags $tags
	} else {
		if { [ llength $xy ] == 2 } {
			set xi [ lindex $xy 0 ]
			set yi [ lindex $xy 1 ]
			# x
			$c create line [ expr { $xi + 2 } ] [ expr { $yi + 2 } ] \
				[ expr { $xi - 3 } ] [ expr { $yi - 3 } ] \
				-width 1 -fill $fill -tags $tagsdots
			$c create line [ expr { $xi + 2 } ] [ expr { $yi - 2 } ] \
				[ expr { $xi - 3 } ] [ expr { $yi + 3 } ] \
				-width 1 -fill $fill -tags $tagsdots
		}
	}
}


#************************************************
# PLOT_POINTS
# plot one series as a set of points on canvas
#************************************************
proc plot_points { c x y { tagsdots "" } { fill c0 } { width 1 } } {
	
	lappend tagsdots dots series

	set size [ expr { min( [ llength $x ], [ llength $y ] ) } ]

	for { set i 0 } { $i < $size } { incr i } {
		set xi [ lindex $x $i ]
		set yi [ lindex $y $i ]
		if { $xi >= 0 && $yi >= 0 } {
			if { $width < 1 } {
				# small point
				$c create oval [ expr { $xi - 1 } ] [ expr { $yi - 1 } ] \
					[ expr { $xi + 1 } ] [ expr { $yi + 1 } ] -fill $fill \
					-width 0 -outline [ $c cget -background ] -tags $tagsdots
			} elseif { $width == 1 } {
				# x
				$c create line [ expr { $xi + 2 } ] [ expr { $yi + 2 } ] \
					[ expr { $xi - 3 } ] [ expr { $yi - 3 } ] \
					-width 1 -fill $fill -tags $tagsdots
				$c create line [ expr { $xi + 2 } ] [ expr { $yi - 2 } ] \
					[ expr { $xi - 3 } ] [ expr { $yi + 3 } ] \
					-width 1 -fill $fill -tags $tagsdots
			} elseif { $width <= 2 } {
				# +
				$c create line [ expr { $xi + 3 } ] $yi [ expr { $xi - 4 } ] $yi \
					-width 1 -fill $fill -tags $tagsdots
				$c create line $xi [ expr { $yi + 3 } ] $xi [ expr { $yi - 4 } ] \
					-width 1 -fill $fill -tags $tagsdots
			} else {
				# filled circle
				$c create oval [ expr { $xi - $width / 2 } ] [ expr { $yi - $width / 2 } ] \
					[ expr { $xi + $width / 2 } ] [ expr { $yi + $width / 2 } ] -fill $fill \
					-width 0 -outline [ $c cget -background ] -tags $tagsdots
			}
		}
	}
}


#************************************************
# INIT_CANVAS_COLORS
# Initialize Tk canvas colors
#************************************************
proc init_canvas_colors { } {
	global defcolors allcolors colorsTheme
	set unusedcolors [ lsort $allcolors ]

	# load default colors
	for { set i 0 } { $i < [ llength $defcolors ] } { incr i } {
		set color [ lindex $defcolors $i ]
		set ::c$i $color

		# remove from list
		set pos [ lsearch -sorted $unusedcolors $color ]
		set unusedcolors [ lreplace $unusedcolors $pos $pos ]
	}

	# load remaining colors
	set n [ expr { $i + [ llength $unusedcolors ] } ]
	for { } { $i < $n } { incr i } {
		# pick random color
		set m [ llength $unusedcolors ]
		set j [ expr { min( int( rand( ) * $m ), $m - 1 ) } ]
		set color [ lindex $unusedcolors $j ]
		set ::c$i $color

		# remove from list
		set pos [ lsearch -sorted $unusedcolors $color ]
		set unusedcolors [ lreplace $unusedcolors $pos $pos ]
	}

	# fill the rest till color 1000
	if { $i < 1000 } {
		for { } { $i < 1000 } { incr i } {
			set ::c$i $colorsTheme(fg)
		}
	}
	set ::c$i $colorsTheme(bg)

	# fill the colors 1000-1099 with gray shades
	for { set j 0 } { $j < 10 } { incr j } {
		for { set k 0 } { $k < 10 } { incr k; incr i } {
			if { ! ( $j == 0 && $k == 0 ) } {
				if { $k == 0 } {
					set ::c$i gray$j
				} else {
					set ::c$i gray$k$j
				}
			}
		}
	}
}


#************************************************
# DETACH_TAB
# Detach/reattach a plot tab/window
#************************************************
proc detach_tab { nb tab but1 but2 da maxLen } {

	if { [ $nb.$tab.$but1 cget -text ] eq "Attach" } {
		
		set tt [ wm title $nb.$tab ]
		
		destroy .tmp
		ttk::frame .tmp
		set tmp [ clone_widget $nb.$tab.c .tmp ]
		update idletasks
		destroytop $nb.$tab
		
		ttk::labelframe $nb.$tab -text "$tt"
		pack $nb.$tab
		set new [ clone_widget .tmp.c $nb.$tab ]
		update idletasks
		destroy .tmp
		
		$nb.$tab.$but1 configure -text Detach
		tooltip::tooltip $nb.$tab.$but1 "Move to independent window"
		tooltip::tooltip $nb.$tab.$but2 "Save plot to file"
		
		pack $new -expand yes -fill both
		$nb add $nb.$tab -text [ string range [ lindex [ split $tt ] 1 ] 0 $maxLen ]
		$nb select $nb.$tab 
		focustop $nb
		
	} else {
	
		set tt [ $nb.$tab cget -text ]
		
		$nb forget $nb.$tab
		
		if { [ $nb index end ] == 0 } {
			wm withdraw [ winfo toplevel $nb ]
		}
			
		destroy .tmp
		ttk::frame .tmp
		set tmp [ clone_widget $nb.$tab.c .tmp ]
		update idletasks
		destroy $nb.$tab
		
		newtop $nb.$tab "$tt" "$nb.$tab.$but1 invoke" ""
		wm transient $nb.$tab $da
		set new [ clone_widget $tmp $nb.$tab ]
		update idletasks
		pack $new -expand yes -fill both
		destroy .tmp
		
		$nb.$tab.$but1 configure -text Attach
		tooltip::tooltip $nb.$tab.$but1 "Move back to main plot window"
		tooltip::tooltip $nb.$tab.$but2 "Save plot to file"
		
		showtop $nb.$tab current yes yes no
		bind $nb.$tab <F1> { LsdHelp menudata_res.html#graph }
		bind $nb.$tab <Escape> "$nb.$tab.$but1 invoke"
	}
}


#************************************************
# ISTOPLEVEL
# Check if widget is a toplevel window
#************************************************
proc istoplevel { w } {
	return [ string equal $w [ winfo toplevel $w ] ]
}


#************************************************
# TK_CONSOLE
# Open Tk console window
#************************************************
proc tk_console { } {
	global conWnd

	if { ! $conWnd } {
		return
	}

	if { ! [ winfo exists .console ] } {
		tkcon::Init
		tkcon title "Tcl/Tk Debug Console"
	}

	tkcon show
}
