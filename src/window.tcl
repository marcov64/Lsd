#*************************************************************
#
#	LSD 7.2 - December 2019
#	written by Marco Valente, Universita' dell'Aquila
#	and by Marcelo Pereira, University of Campinas
#
#	Copyright Marco Valente and Marcelo Pereira
#	LSD is distributed under the GNU General Public License
#	
#*************************************************************

#*************************************************************
# WINDOW.TCL
# Procedures to adjust window positioning & startup code. 
#
# Types of window positioning:
#
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
#*************************************************************

package require Tk 8.5

# Enable console window to be opened with CTRL+ALT+J
set conWnd		true

# Enable window functions operation logging
set logWndFn	false

# Enable coordinates test window
set testWnd		false

# Warn old Tcl/Tk in macOS
if { [ string equal [ tk windowingsystem ] aqua ] && [ string equal [ info tclversion ] 8.5 ] } {
	tk_messageBox -type ok -icon warning -title Warning \
	-message "Old Tcl/Tk version" \
	-detail "Please update your Tcl/Tk version to version 8.6 or higher.\nSome problems may appear when using an old version."
}

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

# list of windows with predefined sizes & positions
set wndLst [ list .lmm .lsd .log .str .da .deb .lat ]
set wndMenuHeight 0

# register static special, OS-dependent configurations
if [ string equal $tcl_platform(platform) unix ] {
	
	# mac
	if [ string equal $tcl_platform(os) Darwin ] {
		
		if { $MAC_PKG } {
			set CurPlatform mac
			set DefaultExe $exeMacPkg
		} else {
			set CurPlatform osx
			set DefaultExe $exeMacOSX
		}
		
		if { [ string equal [ info tclversion ] 8.6 ] } {	
			if { [ string equal [ info patchlevel ] 8.6.9 ] } {
				set butWid $butMacTk869
			} else {
				set butWid $butMac
			}
			
			set DefaultWish $wishMacTk86
		} else {
			set butWid $butMac
			set DefaultWish $wishMacTk85
		}
		
		set daCwid $daCwidMac
		set corrX $corrXmac
		set corrY $corrYmac
		
		set DefaultSysTerm $sysTermMac
		set systemTerm $sysTermMac
		set gnuplotTerm $gnuplotTermMac
		set DefaultMakeExe $makeMac
		set DefaultDbgExe $dbgMac
		set DefaultHtmlBrowser $browserMac
		set DefaultFont $fontMac
		set DefaultFontSize $fontSizeMac
		set deltaSize $deltaSizeMac
		set bsizeM $bsizeMwin
		set bhstepM $bhstepMwin
		set bvstepM $bvstepMwin
		
	# linux
	} else {

		set CurPlatform linux
		
		set butWid $butLinux
		set daCwid $daCwidLinux
		set corrX $corrXlinux
		set corrY $corrYlinux
		
		set DefaultSysTerm $sysTermLinux
		set systemTerm $sysTermLinux
		set gnuplotTerm $gnuplotTermLinux
		set DefaultExe $exeLinux
		set DefaultMakeExe $makeLinux
		set DefaultWish $wishLinux
		set DefaultDbgExe $dbgLinux
		set DefaultHtmlBrowser $browserLinux
		set DefaultFont $fontLinux
		set DefaultFontSize $fontSizeLinux
		set deltaSize $deltaSizeLinux
		set bsizeM $bsizeMlin
		set bhstepM $bhstepMlin
		set bvstepM $bvstepMlin
	}
	
# Windows
} else {
	
	# Windows 32 or 64-bit?
	if { [ string equal $tcl_platform(machine) intel ] } { 
		set CurPlatform win32
		set DefaultExe $exeWin32
		set DefaultMakeExe $makeWin32
		set DefaultWish $wishWinTk85
	} else {
		set CurPlatform win64
		set DefaultExe $exeWin64
		set DefaultWish $wishWinTk86
		
		# Cygwin or MSYS2?
		if { [ catch { exec where cygwin1.dll } ] || [ catch { exec where cygintl-8.dll } ] } { 
			set DefaultMakeExe $makeWin64mgw 
		} else { 
			set DefaultMakeExe $makeWin64cyg 
		}
	}
	
	set butWid $butWindows
	set daCwid $daCwidWindows
	set corrX $corrXwindows
	set corrY $corrYwindows
	
	set DefaultSysTerm $sysTermWindows
	set systemTerm $sysTermWindows
	set gnuplotTerm $gnuplotTermWindows
	set DefaultDbgExe $dbgWindows
	set DefaultHtmlBrowser $browserWindows
	set DefaultFont $fontWindows
	set DefaultFontSize $fontSizeWindows
	set deltaSize $deltaSizeWindows
	set bsizeM $bsizeMwin
	set bhstepM $bhstepMwin
	set bvstepM $bvstepMwin
}

# text line default canvas height & minimum horizontal border width
set lheightP [ expr int( [ font actual $fontP -size ] * [ tk scaling ] ) + $vtmarginP ]
set hbordsizeP	$hmbordsizeP

# current position of structure window
set posXstr 0
set posYstr 0

# lists to hold the windows parents stacks and exceptions to the parent mgmt.
set parWndLst [ list ]
set grabLst [ list ]
set noParLst [ list .log .str .plt .lat .tst ]

# toolbar buttons style
if [ string equal [ tk windowingsystem ] aqua ] { 
	set bRlf raised
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

#************************************************
# Load icon images
#************************************************
image create photo lsdImg -file "$RootLsd/$LsdSrc/icons/lsd.$iconExt"
image create photo lmmImg -file "$RootLsd/$LsdSrc/icons/lmm.$iconExt"
image create photo newImg -file "$RootLsd/$LsdSrc/icons/new.$iconExt"
image create photo openImg -file "$RootLsd/$LsdSrc/icons/open.$iconExt"
image create photo saveImg -file "$RootLsd/$LsdSrc/icons/save.$iconExt"
image create photo undoImg -file "$RootLsd/$LsdSrc/icons/undo.$iconExt"
image create photo redoImg -file "$RootLsd/$LsdSrc/icons/redo.$iconExt"
image create photo cutImg -file "$RootLsd/$LsdSrc/icons/cut.$iconExt"
image create photo deleteImg -file "$RootLsd/$LsdSrc/icons/delete.$iconExt"
image create photo copyImg -file "$RootLsd/$LsdSrc/icons/copy.$iconExt"
image create photo pasteImg -file "$RootLsd/$LsdSrc/icons/paste.$iconExt"
image create photo editImg -file "$RootLsd/$LsdSrc/icons/edit.$iconExt"
image create photo findImg -file "$RootLsd/$LsdSrc/icons/find.$iconExt"
image create photo replaceImg -file "$RootLsd/$LsdSrc/icons/replace.$iconExt"
image create photo indentImg -file "$RootLsd/$LsdSrc/icons/indent.$iconExt"
image create photo deindentImg -file "$RootLsd/$LsdSrc/icons/deindent.$iconExt"
image create photo wrapImg -file "$RootLsd/$LsdSrc/icons/wrap.$iconExt"
image create photo compileImg -file "$RootLsd/$LsdSrc/icons/compile.$iconExt"
image create photo comprunImg -file "$RootLsd/$LsdSrc/icons/comprun.$iconExt"
image create photo gdbImg -file "$RootLsd/$LsdSrc/icons/gdb.$iconExt"
image create photo infoImg -file "$RootLsd/$LsdSrc/icons/info.$iconExt"
image create photo descrImg -file "$RootLsd/$LsdSrc/icons/descr.$iconExt"
image create photo equationImg -file "$RootLsd/$LsdSrc/icons/equation.$iconExt"
image create photo extraImg -file "$RootLsd/$LsdSrc/icons/extra.$iconExt"
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


#************************************************
# SETGLOBKEYS
# Set global key mappings
#************************************************
proc setglobkeys { w { chkChg 1 } } {
	global conWnd grabLst parWndLst logWndFn
	
	# soft/hard exit (check for unsaved changes or not)
	if { $chkChg } {
		bind $w <Control-Alt-x> { if [ string equal [ discard_change ] ok ] { exit }; break }
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
				set lastGrab [ expr [ llength $grabLst ] - 1 ]
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
# NEWTOP
# Procedure to create top level new windows
#************************************************
proc newtop { w { name "" } { destroy { } } { par "." } } {
	global tcl_platform RootLsd LsdSrc parWndLst grabLst noParLst logWndFn

	if [ winfo exists $w ] { 
		destroytop $w
	}
	toplevel $w
	wm withdraw $w
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
	setglobkeys $w
	
	if { $logWndFn && [ info procs plog ] != "" } { plog "\nnewtop (w:$w, master:[wm transient $w], parWndLst:$parWndLst, grab:$grabLst)" } 
}


#************************************************
# SETTOP
# Show the window
#************************************************
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


#************************************************
# SHOWTOP
# Configure the window
#************************************************
proc showtop { w { pos none } { resizeX no } { resizeY no } { grab yes } { sizeX 0 } { sizeY 0 } { buttonF b } { noMinSize no } } {
	global defaultPos wndLst parWndLst grabLst noParLst logWndFn
	
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
		
		update
		
		if { ! [ string equal $pos current ] } {
		
			# handle different window default position
			if [ string equal $pos none ] {
				if [ info exists defaultPos ] {
					set pos $defaultPos
				} {
					set pos centerW
				}
			}
					
			# don't use screen based positioning if not in primary display
			if { [ string equal $pos centerS ] && ! [ primdisp [ winfo parent $w ] ] } {
				if [ info exists defaultPos ] {
					set pos $defaultPos
				} {
					set pos centerW
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
					set gm ${sizeX}x${sizeY}+$x+$y
				} {
					set gm +$x+$y
				}
				wm geometry $w $gm 
			}
		} {
			if { $sizeX != 0 && $sizeY != 0 } {
				set gm ${sizeX}x${sizeY} 
				wm geometry $w $gm 
			}
		}
		
		if { ! $noMinSize && ( $resizeX || $resizeY ) } {
			update
			wm minsize $w [ winfo width $w ] [ winfo height $w ]
		}
		
		wm resizable $w $resizeX $resizeY
	} {
		#known windows - simply apply defaults if not done before
		if { ! [ string equal $pos current ] } {
			sizetop $w
		}
	}

	if { ! [ winfo viewable [ winfo toplevel $w ] ] } {
		wm deiconify $w
	}

	raise $w
	focus $w
	
	update

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
	
	if { [ info exists buttonF ] && [ winfo exists $w.$buttonF.ok ] } {
		$w.$buttonF.ok configure -default active -state active
		focus $w.$buttonF.ok
	} elseif { [ info exists buttonF ] && [ winfo exists $w.$buttonF.r2.ok ] } {
		$w.$buttonF.r2.ok configure -default active -state active
		focus $w.$buttonF.r2.ok
	} {
		focus $w
	}
	
	if { $logWndFn && [ info procs plog ] != "" } { plog "\nshowtop (w:$w, master:[wm transient $w], pos:([winfo x $w],[winfo y $w]), size:[winfo width $w]x[winfo height $w], minsize:[wm minsize $w], primdisp:[ primdisp [ winfo parent $w ] ], parWndLst:$parWndLst, grab:$grabLst)" } 
}


#************************************************
# DESTROYTOP
#************************************************
proc destroytop w {
	global restoreWin wndLst defaultFocus parWndLst grabLst noParLst logWndFn

	if { ! [ winfo exists $w ] } return
	
	# save main windows sizes/positions
	if { $restoreWin && [ lsearch $wndLst $w ] >= 0 } {
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
				set grabPar [ string range [ lindex $grabLst $igrab ] [ expr [ string first " " [ lindex $grabLst $igrab ] ] + 1 ] end ]
				if { $grabPar != "" } {
					if { ! [ winfo viewable $grabPar ] } {
						wm deiconify $grabPar
					}
					raise $grabPar
					focus $grabPar
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
	set decorationThickness [ expr $contentsLeft - $decorationLeft ]

	# find titlebar plus menubar thickness
	set menubarThickness [ expr $contentsTop - $decorationTop ]
	
	# compute real values
	incr width [ expr 2 * $decorationThickness ]
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
					set realY [ expr $decorationTop + $contentsLeft - $decorationLeft + 8 ]
				}
				.lat {
					set realY [ expr $decorationTop + $contentsLeft - $decorationLeft ]
				}
				default {
					set realY [ expr $decorationTop + $contentsLeft - $decorationLeft - 2 ]
				}
			}
		}
		
		mac -
		osx {
			set realX $decorationLeft
			set realY $decorationTop
			set realHeight [ expr $height + $contentsTop - $decorationTop - $wndMenuHeight ]
		}
		
		win32 -
		win64 {
			set realX $decorationLeft
			set realY $decorationTop
	
			switch $w {
				.da -
				.deb {
					set realHeight [ expr $height + $contentsTop - $decorationTop - ( $wndMenuHeight + 20 ) ]
				}
				default {
					set realHeight [ expr $height + $contentsTop - $decorationTop - $wndMenuHeight ]
				}
			}
		}
	}
	
	if { $w == ".str" } {
		return ${width}x${realHeight}+${realX}+${realY}:${hfactM}+${vfactM}
	} {
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
			set centerX [ expr $decorationLeft + $width / 2 ]
			set centerY [ expr $decorationTop + $height / 2 ]

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
	global wndLst hsizeB vsizeB hsizeL vsizeL hsizeLmin vsizeLmin bordsize hmargin vmargin tbarsize posXstr posYstr hsizeM vsizeM corrX corrY parWndLst grabLst logWndFn lmmGeom lsdGeom logGeom strGeom daGeom debGeom latGeom hfactM vfactM wndMenuHeight

	update
	
	if { [ string equal $w .lsd ] || [ string equal $w .lmm ] } {
	
		set realW .
	
		# save initial height of the top decoration (menu, title bar and border)
		if { $wndMenuHeight == 0 } {
			set curGeom [ wm geometry . ]
			scan $curGeom "%dx%d+%d+%d" width height decorationLeft decorationTop
			set contentsTop [ winfo rooty . ]
			set wndMenuHeight [ expr $contentsTop - $decorationTop ]
		}
	} else {
		set realW $w
	}
	
	set screenWidth [ winfo screenwidth $realW ]
	set screenHeight [ winfo screenheight $realW ]
					
	foreach wnd $wndLst {
		if { ! [ string compare $w all ] || ! [ string compare $w $wnd ] } {
		
			switch $wnd {
			
				.lsd {
					set defGeom "${hsizeB}x$vsizeB+[ getx . topleftS ]+[ gety . topleftS ]"
					wm geometry . [ checkgeom $lsdGeom $defGeom $screenWidth $screenHeight ]
					wm minsize . $hsizeB [ expr $vsizeB / 2 ]
				}
				
				.lmm {
					if { $screenWidth < ( $hsizeL + 2 * $bordsize ) } {
						set W [ expr $screenWidth - 2 * $bordsize ] 
					} {
						set W $hsizeL
					}
					set H [ expr $screenHeight - $tbarsize - 2 * $vmargin - 2 * $bordsize ]
					if { $H < $vsizeL } {
						set H [ expr $screenHeight - $tbarsize - 2 * $bordsize ] 
					}
					if { $screenWidth < ( $hsizeL + 2 * $bordsize + $hmargin ) } {
						set X 0
					} {
						set X [ expr $screenWidth - $hmargin - $bordsize - $W ]
					}
					set Y [ expr ( $screenHeight - $tbarsize ) / 2 - $bordsize - $H / 2 ]
					
					set defGeom "${W}x$H+$X+$Y"

					wm geometry . [ checkgeom $lmmGeom $defGeom $screenWidth $screenHeight ]
					wm minsize . $hsizeLmin $vsizeLmin
				}
				
				.log {
					set defGeom "+[ getx .log bottomrightS ]+[ gety .log bottomrightS ]"
					wm geometry .log [ checkgeom $logGeom $defGeom $screenWidth $screenHeight ]
					wm minsize .log [ winfo width .log ] [ winfo height .log ]
				}
				
				.str {
					set posXstr [ expr [ winfo x . ] + [ winfo width . ] + 2 * $bordsize + $hmargin + $corrX ]
					set posYstr [ expr [ winfo y . ] + $corrY ]
					set defGeom "${hsizeM}x${vsizeM}+${posXstr}+${posYstr}"

					# handle the extra scaling parameters
					set geom [ split [ checkgeom $strGeom $defGeom $screenWidth $screenHeight ] ":" ]
					wm geometry .str [ lindex $geom 0 ]
					if { [ lindex $geom 1 ] != "" } {
						scan [ lindex $geom 1 ] "%f+%f" hfactM vfactM
					}
					
					wm minsize .str [ expr $hsizeM / 2 ] [ expr $vsizeM / 2 ]	
				}
				
				.da {
					set defGeom "+[ getx .da overM ]+[ gety .da overM ]"
					wm geometry .da [ checkgeom $daGeom $defGeom $screenWidth $screenHeight ]
					wm minsize .da [ winfo width .da ] [ winfo height .da ]
					wm resizable .da 0 1
				}
				
				.deb {
					set defGeom "+[ getx .deb topleftW ]+[ gety .deb topleftW ]"
					wm geometry .deb [ checkgeom $debGeom $defGeom $screenWidth $screenHeight ]
					wm minsize .deb [ winfo width .deb ] [ winfo height .deb ]
					wm resizable .deb 0 1
				}

				.lat {
					set defGeom "+[ getx .lat centerS ]+[ gety .lat centerS ]"
					wm geometry .lat [ checkgeom $latGeom $defGeom $screenWidth $screenHeight ]
					wm minsize .lat [ winfo width .lat ] [ winfo height .lat ]
					wm resizable .lat 0 0
				}
			}
		}
	}

	update

	if { $logWndFn && [ info procs plog ] != "" } { plog "\nsizetop (w:$w, master:[wm transient $w], pos:([winfo x $w],[winfo y $w]), size:[winfo width $w]x[winfo height $w], parWndLst:$parWndLst, grab:$grabLst)" } 
}


#************************************************
# RESIZETOP
# Resize the window
#************************************************
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
		if { $sizeX > 0 && $sizeY > 0 } {
			wm geom $w ${sizeX}x${sizeY} 
		}
	}
	update

	if { $logWndFn && [ info procs plog ] != "" } { plog "\nresizetop (w:$w, master:[wm transient $w], pos:([winfo x $w],[winfo y $w]), size:[winfo width $w]x[winfo height $w], parWndLst:$parWndLst, grab:$grabLst)" } 
}


#************************************************
# ICONTOP
# Set window icon
#************************************************
proc icontop { w type } {
	global tcl_platform RootLsd LsdSrc lmmImg lsdImg iconExt
	
	if { $type == "" } {
		wm iconbitmap $w ""
		return
	}
	
	if { $tcl_platform(platform) == "windows" } {
		if [ string equal $w . ] {
			wm iconbitmap $w -default $RootLsd/$LsdSrc/icons/$type.ico
		} {
			wm iconbitmap $w $RootLsd/$LsdSrc/icons/$type.ico
		}
	} 
	
	if { $tcl_platform(platform) == "unix" } { 
		if { $tcl_platform(os) == "Darwin" } {
			
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
}


#************************************************
# ALIGN
# Alignment of window w1 to the to right side of w2
#************************************************
proc align { w1 w2 { side R } } {
	global hmargin corrX corrY logWndFn
	
	set a [ winfo width $w1 ]
	set b [ winfo height $w1 ]
	set c [ expr [ winfo x $w2 ] + $corrX ]
	set d [ expr [ winfo y $w2 ] + $corrY ]
	set e [ winfo width $w1 ]
	set f [ winfo width $w2 ]

	if { $side == "L" } {
		set g [ expr $c - $e - $hmargin ]
	} else {
		set g [ expr $c + $f + $hmargin ]
	}
	wm geometry $w1 +$g+$d
	update

	if { $logWndFn && [ info procs plog ] != "" } { plog "\nalign w1:$w1 w2:$w2 (w1 width:$a, w1 height:$b, w2 x:$c, w2 y:$d, w2 width:$e)" }
}


#************************************************
# PRIMDISP
# check if window center is in primary display
#************************************************
proc primdisp w {
	if { [ winfo rootx $w ] > 0 && [ winfo rootx $w ] < [ winfo screenwidth $w ] && [ winfo rooty $w ] > 0 && [ winfo rooty $w ] < [ winfo screenheight $w ] } {
		return true
	} {
		return false
	}
} 


#************************************************
# GETX
# compute x coordinate of new window according to the types
#************************************************
proc getx { w pos } {
	global corrX hmargin bordsize
	
	switch $pos {
		centerS { 
			set hpos [ expr [ winfo screenwidth $w ] / 2 - [ winfo reqwidth $w ] / 2 ]
		}
		centerW { 
			set hpos [ expr [ winfo x [ winfo parent $w ] ] + $corrX + [ winfo width [ winfo parent $w ] ] / 2  - [ winfo reqwidth $w ] / 2 ]
		}
		topleftS { 
			set hpos [ expr $hmargin + $corrX ]
		}
		topleftW { 
			set hpos [ expr [ winfo x [ winfo parent $w ] ] + $corrX + 10 ]
		}
		overM { 
			set hpos [ expr [ winfo x . ] + $corrX ]
		}
		coverW { 
			set hpos [ expr [ winfo x [ winfo parent $w ] ] + $corrX ]
		}
		bottomrightS {
			set hpos [ expr [ winfo screenwidth $w ] - $hmargin - [ winfo reqwidth $w ] ]
		}
		righttoW {
			set hpos [ expr [ winfo x [ winfo parent $w ] ] + $corrX + $hmargin + [ winfo reqwidth [ winfo parent $w ] ] - 2 * $bordsize ]
		}
		lefttoW {
			set hpos [ expr [ winfo x [ winfo parent $w ] ] + $corrX - $hmargin - [ winfo reqwidth $w ] + 2 * $bordsize ]
		}
		default { 
			set hpos [ expr [ winfo screenwidth $w ] / 2 - [ winfo reqwidth $w ] / 2 ]
		}
	}
		
	if { $hpos < $corrX && [ primdisp [ winfo parent $w ] ] } {
		return $corrX
	} else {
		return $hpos
	}
}


#************************************************
# GETY
# compute y coordinate of new window according to the types
#************************************************
proc gety { w pos } {
	global corrY vmargin tbarsize
	
	switch $pos {
		centerS { 
			set vpos [ expr [ winfo screenheight $w ] / 2 - [ winfo reqheight $w ] / 2 ]
		}
		centerW { 
			set vpos [ expr [ winfo y [ winfo parent $w ] ] + $corrY + [ winfo height [ winfo parent $w ] ] / 2  - [ winfo reqheight $w ] / 2 ]
		}
		topleftS { 
			set vpos [ expr $vmargin + $corrY ]
		}
		topleftW { 
			set vpos [ expr [ winfo y [ winfo parent $w ] ] + $corrY + 30 ]
		}
		overM { 
			set vpos [ expr [ winfo y . ] + $corrY ]
		}
		coverW { 
			set vpos [ expr [ winfo y [ winfo parent $w ] ] + $corrY ]
		}
		bottomrightS {
			set vpos [ expr [ winfo screenheight $w ] - $vmargin - $tbarsize - [ winfo reqheight $w ] ]
		}
		righttoW {
			set vpos [ expr [ winfo y [ winfo parent $w ] ] + $corrY ]
		}
		lefttoW {
			set vpos [ expr [ winfo y [ winfo parent $w ] ] + $corrY ]
		}
		default { 
			set vpos [ expr [ winfo screenheight $w ] / 2 - [ winfo reqheight $w ] / 2 ]
		}
	} 
		
	if { $vpos < $corrY && [ primdisp [ winfo parent $w ] ] } {
		return $corrY
	} else {
		return $vpos
	}
}


#************************************************
# OKHELPCANCEL
# Procedure to create standard button set
#************************************************
# procedures to create standard button sets
proc okhelpcancel { w fr comOk comHelp comCancel } {
	global butWid
	if { ! [ winfo exists $w.$fr ] } { frame $w.$fr }
	button $w.$fr.ok -width $butWid -text OK -command $comOk
	button $w.$fr.help -width $butWid -text Help -command $comHelp
	button $w.$fr.can -width $butWid -text Cancel -command $comCancel
	bind $w.$fr.ok <KeyPress-Return> "$w.$fr.ok invoke"
	bind $w.$fr.help <KeyPress-Return> "$w.$fr.help invoke"
	bind $w.$fr.can <KeyPress-Return> "$w.$fr.can invoke"
	bind $w <KeyPress-Escape> "$w.$fr.can invoke"
	bind $w <F1> "$w.$fr.help invoke"
	pack $w.$fr.ok $w.$fr.help $w.$fr.can -padx 10 -pady 10 -side left
	pack $w.$fr -side right 
}


#************************************************
# OKHELP
# Procedure to create standard button set
#************************************************
proc okhelp { w fr comOk comHelp } {
	global butWid
	if { ! [ winfo exists $w.$fr ] } { frame $w.$fr }
	button $w.$fr.ok -width $butWid -text OK -command $comOk
	button $w.$fr.help -width $butWid -text Help -command $comHelp
	bind $w.$fr.ok <KeyPress-Return> "$w.$fr.ok invoke"
	bind $w.$fr.help <KeyPress-Return> "$w.$fr.help invoke"
	bind $w <KeyPress-Escape> "$w.$fr.ok invoke"
	bind $w <F1> "$w.$fr.help invoke"
	pack $w.$fr.ok $w.$fr.help -padx 10 -pady 10 -side left
	pack $w.$fr -side right 
}


#************************************************
# OKCANCEL
# Procedure to create standard button set
#************************************************
proc okcancel { w fr comOk comCancel } {
	global butWid
	if { ! [ winfo exists $w.$fr ] } { frame $w.$fr }
	button $w.$fr.ok -width $butWid -text OK -command $comOk
	button $w.$fr.can -width $butWid -text Cancel -command $comCancel
	bind $w.$fr.ok <KeyPress-Return> "$w.$fr.ok invoke"
	bind $w.$fr.can <KeyPress-Return> "$w.$fr.can invoke"
	bind $w <KeyPress-Escape> "$w.$fr.can invoke"
	pack $w.$fr.ok $w.$fr.can -padx 10 -pady 10 -side left
	pack $w.$fr -side right 
}


#************************************************
# HELPCANCEL
# Procedure to create standard button set
#************************************************
proc helpcancel { w fr comHelp comCancel } {
	global butWid
	if { ! [ winfo exists $w.$fr ] } { frame $w.$fr }
	button $w.$fr.help -width $butWid -text Help -command $comHelp
	button $w.$fr.can -width $butWid -text Cancel -command $comCancel
	bind $w.$fr.help <KeyPress-Return> "$w.$fr.help invoke"
	bind $w.$fr.can <KeyPress-Return> "$w.$fr.can invoke"
	bind $w <KeyPress-Escape> "$w.$fr.can invoke"
	bind $w <F1> "$w.$fr.help invoke"
	pack $w.$fr.help $w.$fr.can -padx 10 -pady 10 -side left
	pack $w.$fr -side right 
}


#************************************************
# OK
# Procedure to create standard button set
#************************************************
proc ok { w fr comOk } {
	global butWid
	if { ! [ winfo exists $w.$fr ] } { frame $w.$fr }
	button $w.$fr.ok -width $butWid -text OK -command $comOk
	bind $w.$fr.ok <KeyPress-Return> "$w.$fr.ok invoke"
	bind $w <KeyPress-Escape> "$w.$fr.ok invoke"
	pack $w.$fr.ok -padx 10 -pady 10 -side left
	pack $w.$fr -side right 
}


#************************************************
# CANCEL
# Procedure to create standard button set
#************************************************
proc cancel { w fr comCancel } {
	global butWid
	if { ! [ winfo exists $w.$fr ] } { frame $w.$fr }
	button $w.$fr.can -width $butWid -text Cancel -command $comCancel
	bind $w.$fr.can <KeyPress-Return> "$w.$fr.can invoke"
	bind $w <KeyPress-Escape> "$w.$fr.can invoke"
	pack $w.$fr.can -padx 10 -pady 10 -side left
	pack $w.$fr -side right 
}


#************************************************
# XCANCEL
# Procedure to create standard button set
#************************************************
proc Xcancel { w fr nameX comX comCancel } {
	global butWid
	if { ! [ winfo exists $w.$fr ] } { frame $w.$fr }
	if { [ string length "$nameX" ] > $butWid } { 
		set Xwid [ string length "$nameX" ] 
	} else {
		set Xwid $butWid 
	}
	button $w.$fr.ok -width $Xwid -text $nameX -command $comX
	button $w.$fr.can -width $butWid -text Cancel -command $comCancel
	bind $w.$fr.ok <KeyPress-Return> "$w.$fr.ok invoke"
	bind $w.$fr.can <KeyPress-Return> "$w.$fr.can invoke"
	bind $w <KeyPress-Escape> "$w.$fr.can invoke"
	pack $w.$fr.ok $w.$fr.can -padx 10 -pady 10 -side left
	pack $w.$fr -side right 
}


#************************************************
# OKXHELPCANCEL
# Procedure to create standard button set
#************************************************
proc okXhelpcancel { w fr nameX comX comOk comHelp comCancel } {
	global butWid
	if { ! [ winfo exists $w.$fr ] } { frame $w.$fr }
	if { [ string length "$nameX" ] > $butWid } { 
		set Xwid [ string length "$nameX" ] 
	} else {
		set Xwid $butWid 
	}
	button $w.$fr.ok -width $butWid -text OK -command $comOk
	button $w.$fr.x -width $Xwid -text $nameX -command $comX
	button $w.$fr.help -width $butWid -text Help -command $comHelp
	button $w.$fr.can -width $butWid -text Cancel -command $comCancel
	bind $w.$fr.ok <KeyPress-Return> "$w.$fr.ok invoke"
	bind $w.$fr.x <KeyPress-Return> "$w.$fr.x invoke"
	bind $w.$fr.help <KeyPress-Return> "$w.$fr.help invoke"
	bind $w.$fr.can <KeyPress-Return> "$w.$fr.can invoke"
	bind $w <KeyPress-Escape> "$w.$fr.can invoke"
	bind $w <F1> "$w.$fr.help invoke"
	pack $w.$fr.ok $w.$fr.x $w.$fr.help $w.$fr.can -padx 10 -pady 10 -side left
	pack $w.$fr -side right 
}


#************************************************
# XYOKHELPCANCEL
# Procedure to create standard button set
#************************************************
proc XYokhelpcancel { w fr nameX nameY comX comY comOk comHelp comCancel } {
	global butWid
	if { ! [ winfo exists $w.$fr ] } { frame $w.$fr }
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
	frame $w.$fr.r1
	button $w.$fr.r1.x -width $Xwid -text $nameX -command $comX
	button $w.$fr.r1.y -width $Ywid -text $nameY -command $comY
	frame $w.$fr.r2
	button $w.$fr.r2.ok -width $butWid -text OK -command $comOk
	button $w.$fr.r2.help -width $butWid -text Help -command $comHelp
	button $w.$fr.r2.can -width $butWid -text Cancel -command $comCancel
	bind $w.$fr.r1.x <KeyPress-Return> "$w.$fr.r1.x invoke"
	bind $w.$fr.r1.y <KeyPress-Return> "$w.$fr.r1.y invoke"
	bind $w.$fr.r2.ok <KeyPress-Return> "$w.$fr.r2.ok invoke"
	bind $w.$fr.r2.help <KeyPress-Return> "$w.$fr.r2.help invoke"
	bind $w.$fr.r2.can <KeyPress-Return> "$w.$fr.r2.can invoke"
	bind $w <KeyPress-Escape> "$w.$fr.r2.can invoke"
	bind $w <F1> "$w.$fr.r2.help invoke"
	pack $w.$fr.r1.x $w.$fr.r1.y -padx 10 -side left
	pack $w.$fr.r2.ok $w.$fr.r2.help $w.$fr.r2.can -padx 10 -side left
	pack $w.$fr.r1 -anchor w
	pack $w.$fr.r2  -pady 10
	pack $w.$fr -side right 
}


#************************************************
# XYZOKHELPCANCEL
# Procedure to create standard button set
#************************************************
proc XYZokhelpcancel { w fr nameX nameY nameZ comX comY comZ comOk comHelp comCancel } {
	global butWid
	if { ! [ winfo exists $w.$fr ] } { frame $w.$fr }
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
	frame $w.$fr.r1
	button $w.$fr.r1.x -width $Xwid -text $nameX -command $comX
	button $w.$fr.r1.y -width $Ywid -text $nameY -command $comY
	button $w.$fr.r1.z -width $Ywid -text $nameZ -command $comZ
	frame $w.$fr.r2
	button $w.$fr.r2.ok -width $butWid -text OK -command $comOk
	button $w.$fr.r2.help -width $butWid -text Help -command $comHelp
	button $w.$fr.r2.can -width $butWid -text Cancel -command $comCancel
	bind $w.$fr.r1.x <KeyPress-Return> "$w.$fr.r1.x invoke"
	bind $w.$fr.r1.y <KeyPress-Return> "$w.$fr.r1.y invoke"
	bind $w.$fr.r1.z <KeyPress-Return> "$w.$fr.r1.z invoke"
	bind $w.$fr.r2.ok <KeyPress-Return> "$w.$fr.r2.ok invoke"
	bind $w.$fr.r2.help <KeyPress-Return> "$w.$fr.r2.help invoke"
	bind $w.$fr.r2.can <KeyPress-Return> "$w.$fr.r2.can invoke"
	bind $w <KeyPress-Escape> "$w.$fr.r2.can invoke"
	bind $w <F1> "$w.$fr.r2.help invoke"
	pack $w.$fr.r1.x $w.$fr.r1.y $w.$fr.r1.z -padx 10 -side left
	pack $w.$fr.r2.ok $w.$fr.r2.help $w.$fr.r2.can -padx 10 -side left
	pack $w.$fr.r1 -anchor w
	pack $w.$fr.r2  -pady 10
	pack $w.$fr -side right 
}


#************************************************
# DONEHELP
# Procedure to create standard button set
#************************************************
proc donehelp { w fr comDone comHelp } {
	global butWid
	if { ! [ winfo exists $w.$fr ] } { frame $w.$fr }
	button $w.$fr.ok -width $butWid -text Done -command $comDone
	button $w.$fr.help -width $butWid -text Help -command $comHelp
	bind $w.$fr.ok <KeyPress-Return> "$w.$fr.ok invoke"
	bind $w.$fr.help <KeyPress-Return> "$w.$fr.help invoke"
	bind $w <KeyPress-Escape> "$w.$fr.ok invoke"
	bind $w <F1> "$w.$fr.help invoke"
	pack $w.$fr.ok $w.$fr.help -padx 10 -pady 10 -side left
	pack $w.$fr -side right 
}


#************************************************
# DONE
# Procedure to create standard button set
#************************************************
proc done { w fr comDone } {
	global butWid
	if { ! [ winfo exists $w.$fr ] } { frame $w.$fr }
	button $w.$fr.ok -width $butWid -text Done -command $comDone
	bind $w.$fr.ok <KeyPress-Return> "$w.$fr.ok invoke"
	bind $w <KeyPress-Escape> "$w.$fr.ok invoke"
	pack $w.$fr.ok -padx 10 -pady 10 -side left
	pack $w.$fr -side right 
}


#************************************************
# COMPHELPDONE
# Procedure to create standard button set
#************************************************
proc comphelpdone { w fr comComp comHelp comDone } {
	global butWid
	if { ! [ winfo exists $w.$fr ] } { frame $w.$fr }
	button $w.$fr.com -width $butWid -text Compute -command $comComp
	button $w.$fr.help -width $butWid -text Help -command $comHelp
	button $w.$fr.ok -width $butWid -text Done -command $comDone
	bind $w.$fr.com <KeyPress-Return> "$w.$fr.com invoke"
	bind $w.$fr.help <KeyPress-Return> "$w.$fr.help invoke"
	bind $w.$fr.ok <KeyPress-Return> "$w.$fr.ok invoke"
	bind $w <KeyPress-Escape> "$w.$fr.ok invoke"
	bind $w <F1> "$w.$fr.help invoke"
	pack $w.$fr.com $w.$fr.help $w.$fr.ok -padx 10 -pady 10 -side left
	pack $w.$fr -side right 
}


#************************************************
# FINDHELPDONE
# Procedure to create standard button set
#************************************************
proc findhelpdone { w fr comFind comHelp comDone } {
	global butWid
	if { ! [ winfo exists $w.$fr ] } { frame $w.$fr }
	button $w.$fr.search -width $butWid -text Find -command $comFind
	button $w.$fr.help -width $butWid -text Help -command $comHelp
	button $w.$fr.ok -width $butWid -text Done -command $comDone
	bind $w.$fr.search <KeyPress-Return> "$w.$fr.search invoke"
	bind $w.$fr.help <KeyPress-Return> "$w.$fr.help invoke"
	bind $w.$fr.ok <KeyPress-Return> "$w.$fr.ok invoke"
	bind $w <KeyPress-Escape> "$w.$fr.ok invoke"
	bind $w <F1> "$w.$fr.help invoke"
	pack $w.$fr.search $w.$fr.help $w.$fr.ok -padx 10 -pady 10 -side left
	pack $w.$fr -side right 
}


#************************************************
# SAVE
# Procedure to create standard button set
#************************************************
proc save { w fr comSave } {
	global butWid
	if { ! [ winfo exists $w.$fr ] } { frame $w.$fr }
	button $w.$fr.ok -width $butWid -text Save -command $comSave
	bind $w.$fr.ok <KeyPress-Return> "$w.$fr.ok invoke"
	pack $w.$fr.ok -padx 10 -pady 10 -side left
	pack $w.$fr -side right 
}


#************************************************
# DISABLE_WINDOW
# Command to disable windows in cases where grab is inappropriate (only menus if not TK8.6)
# call parameters are: container window, menu name, widgets names
#************************************************
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
	

#************************************************
# ENABLE_WINDOW
# Command to enable windows in cases where grab is inappropriate (only menus if not TK8.6)
# call parameters are: container window, menu name, widgets names
#************************************************
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
	

#************************************************
# WRITE_ANY
# Read any entry widget (normal or disabled)
#************************************************
proc write_any { w val } {
	if [ string equal [ $w cget -state ] disabled ] {
		write_disabled $w $val
	} {
		$w delete 0 end
		$w insert 0 $val
	}
}


#************************************************
# WRITE_DISABLED
# Update a disabled entry widget (do nothing if normal state)
#************************************************
proc write_disabled { w val } {
	if [ string equal [ $w cget -state ] disabled ] {
		$w conf -state normal
		write_any $w $val
		$w conf -state disabled
	}
}


#************************************************
# SETTAB
# Procedure to adjust tab size according to font type and size
#************************************************
proc settab { w size font } { 
	set tabwidth "[ expr { $size * [ font measure "$font" 0 ] } ] left"
	$w conf -font "$font" -tabs $tabwidth -tabstyle wordprocessor 
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
# bind the mouse wheel to the y scrollbar
#************************************************
proc mouse_wheel { w } {
	global tcl_platform sfmwheel winmwscale
	
	if [ string equal $tcl_platform(platform) windows ] {
		bind $w <MouseWheel> { if { [ expr abs( %D ) ] < $winmwscale } { set winmwscale [ expr abs( %D ) ]; if { $winmwscale <= 0 } { set winmwscale 1 } }; %W yview scroll [ expr -1 * $sfmwheel * %D / $winmwscale ] units }
		bind $w <Shift-MouseWheel> { if { [ expr abs( %D ) ] < $winmwscale } { set winmwscale [ expr abs( %D ) ]; if { $winmwscale <= 0 } { set winmwscale 1 } }; %W xview scroll [ expr -1 * $sfmwheel * %D / $winmwscale ] units }
		bind $w <Control-MouseWheel> { if { [ expr abs( %D ) ] < $winmwscale } { set winmwscale [ expr abs( %D ) ]; if { $winmwscale <= 0 } { set winmwscale 1 } }; %W xview scroll [ expr -1 * $sfmwheel * %D / $winmwscale ] units }
		bind $w <Alt-MouseWheel> { if { [ expr abs( %D ) ] < $winmwscale } { set winmwscale [ expr abs( %D ) ]; if { $winmwscale <= 0 } { set winmwscale 1 } }; %W xview scroll [ expr -1 * $sfmwheel * %D / $winmwscale ] units }
	} {
		if [ string equal $tcl_platform(os) Darwin ] {
			bind $w <MouseWheel> { %W yview scroll [ expr -1 * $sfmwheel * %D ] units }
			bind $w <Shift-MouseWheel> { %W xview scroll [ expr -1 * $sfmwheel * %D ] units }
			bind $w <Control-MouseWheel> { %W xview scroll [ expr -1 * $sfmwheel * %D ] units }
		} {
			bind $w <4> { %W yview scroll [ expr -1 * $sfmwheel ] units }
			bind $w <5> { %W yview scroll $sfmwheel units }
			bind $w <Shift-4> { %W xview scroll [ expr -1 * $sfmwheel ] units }
			bind $w <Shift-5> { %W xview scroll $sfmwheel units }
			bind $w <Control-4> { %W xview scroll [ expr -1 * $sfmwheel ] units }
			bind $w <Control-5> { %W xview scroll $sfmwheel units }
		}
	}
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
        set ratio [ expr sqrt( 2.0 ) ]
    } else {
        set ratio [ expr 1.0 / sqrt( 2.0 ) ]
    }  
	
	set sro [ $c cget -scrollregion ]
	
	set newH [ expr ( [ lindex $sro 3 ] - [ lindex $sro 1 ] ) * $ratio ]
	set origH [ expr $vsizeP + $tbordsizeP + $bbordsizeP ]
	set newRatio [ expr $newH / $origH ]
	
	if { $newRatio < $maxzoomP && $newRatio > $minzoomP } {
		set srn [ list ]
		for { set i 0 } { $i < 4 } { incr i } {
			lappend srn [ expr round( [ lindex $sro $i ] * $ratio ) ]
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
proc canvas_axis { c type grid { y2 0 } } {
	global hsizeP vsizeP hbordsizeP tbordsizeP hticksP vticksP axcolorP grcolorP 
	
	# x-axis, ticks & x-grid
	$c create line $hbordsizeP $tbordsizeP [ expr $hbordsizeP + $hsizeP ] $tbordsizeP -width 1 -fill $axcolorP -tag p
	$c create line $hbordsizeP [ expr $tbordsizeP + $vsizeP ] [ expr $hbordsizeP + $hsizeP ] [ expr $tbordsizeP + $vsizeP ] -width 1 -fill $axcolorP -tag p

	if { $type == 0 || $type == 4 || $type == 5 } {
		for { set i 0 } { $i < [ expr $hticksP + 2 ] } { incr i } {
			if { $grid && $i > 0 && $i < [ expr $hticksP + 1 ] } {
				$c create line [ expr $hbordsizeP + round( $i * $hsizeP / ( $hticksP + 1 ) ) ] [ expr $tbordsizeP + 1 ] [ expr $hbordsizeP + round( $i * $hsizeP / ( $hticksP + 1 ) ) ] [ expr $tbordsizeP + $vsizeP - 1 ] -fill $grcolorP -width 1 -tags {g p} 
			}
			$c create line [ expr $hbordsizeP + round( $i * $hsizeP / ( $hticksP + 1 ) ) ] [ expr $tbordsizeP + $vsizeP ] [ expr $hbordsizeP + round( $i * $hsizeP / ( $hticksP + 1 ) ) ] [ expr $tbordsizeP + $vsizeP + 5 ] -fill $axcolorP -width 1 -tags p
		}
	}
	
	# y-axis, ticks & y-grid
	$c create line $hbordsizeP $tbordsizeP $hbordsizeP [ expr $tbordsizeP + $vsizeP ] -width 1 -fill $axcolorP -tag p
	$c create line [ expr $hbordsizeP + $hsizeP ]  $tbordsizeP [ expr $hbordsizeP + $hsizeP ] [ expr $tbordsizeP + $vsizeP ] -width 1 -fill $axcolorP -tag p

	for { set i 0 } { $i < [ expr $vticksP + 2 ] } { incr i } {
		if { $grid && $i > 0 && $i < [ expr $vticksP + 1 ] } {
			$c create line [ expr $hbordsizeP + 1 ] [ expr $tbordsizeP + round( $i * $vsizeP / ( $vticksP + 1 ) ) ] [ expr $hbordsizeP + $hsizeP - 1 ] [ expr $tbordsizeP + round( $i * $vsizeP / ( $vticksP + 1 ) ) ] -fill $grcolorP -width 1 -tags {g p}
		}
		$c create line [ expr $hbordsizeP - 5 ] [ expr $tbordsizeP + round( $i * $vsizeP / ( $vticksP + 1 ) ) ] $hbordsizeP [ expr $tbordsizeP + round( $i * $vsizeP / ( $vticksP + 1 ) ) ] -fill $axcolorP -width 1 -tag p
		if $y2 {
			$c create line [ expr $hbordsizeP + $hsizeP ] [ expr $tbordsizeP + round( $i * $vsizeP / ( $vticksP + 1 ) ) ] [ expr $hbordsizeP + $hsizeP + 5 ] [ expr $tbordsizeP + round( $i * $vsizeP / ( $vticksP + 1 ) ) ] -fill $axcolorP -width 1 -tag p
		}
	}
}


#************************************************
# PLOT_BARS
# plot color filled bars
#************************************************
proc plot_bars { c x1 y1 x2 y2 { tags "" } { fill white } { width 1 } } {

	set size [ expr min( [ llength $x1 ], [ llength $y1 ], [ llength $x2 ], [ llength $y2 ]  ) ]
	lappend tags bar series
	
	for { set i 0 } { $i < $size } { incr i } {
		# valid point?
		set x1i [ lindex $x1 $i ]
		set y1i [ lindex $y1 $i ]
		set x2i [ lindex $x2 $i ]
		set y2i [ lindex $y2 $i ]
		if { $x1i != $x2i && $y1i != $y2i } {
			# plot bar
			$c create rect $x1i $y1i $x2i $y2i -width $width -fill $fill -tags $tags
		}
	}
}


#************************************************
# PLOT_LINE
# plot one series as a line on canvas (may be discontinuous)
#************************************************
proc plot_line { c x y { tags "" } { fill c0 } { width 1 } } {
	global smoothP splstepsP

	set size [ expr min( [ llength $x ], [ llength $y ] ) ]
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
		} { # last point in segment?
			set lenxy [ llength $xy ]
			if { $lenxy > 0 } {
				# plot line segment/single point till now
				if { $lenxy >= 4 } {
					$c create line $xy -width $width -fill $fill -smooth $smoothP \
						-splinesteps $splstepsP -tags $tags
				} {
					if { $lenxy == 2 } {
						set xi [ lindex $xy 0 ]
						set yi [ lindex $xy 1 ]
						$c create oval [ expr $xi - $width / 2 ] \
							[ expr $yi - $width / 2 ] [ expr $xi + $width / 2 ] \
							[ expr $yi + $width / 2 ] -fill $fill \
							-width 0 -outline white -tags $tagsdots
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
	} {
		if { [ llength $xy ] == 2 } {
			set xi [ lindex $xy 0 ]
			set yi [ lindex $xy 1 ]
			$c create oval [ expr $xi - $width / 2 ] [ expr $yi - $width / 2 ] \
				[ expr $xi + $width / 2 ] [ expr $yi + $width / 2 ] -fill $fill \
				-width 0 -outline white -tags $tagsdots
		}
	}
}


#************************************************
# PLOT_POINTS
# plot one series as a set of points on canvas
#************************************************
proc plot_points { c x y { tagsdots "" } { fill c0 } { width 1 } } {
	lappend tagsdots dots series

	set size [ expr min( [ llength $x ], [ llength $y ] ) ]
	
	for { set i 0 } { $i < $size } { incr i } {
		set xi [ lindex $x $i ]
		set yi [ lindex $y $i ]
		if { $xi >= 0 && $yi >= 0 } {
			if { $width == 1 } {
				# +
				$c create line [ expr $xi + 2 ] $yi [ expr $xi - 3 ] $yi \
					-width 1 -fill $fill -tags $tagsdots
				$c create line $xi [ expr $yi + 2 ] $xi [ expr $yi - 3 ] \
					-width 1 -fill $fill -tags $tagsdots
			} elseif { $width == 2 } {
				# x
				$c create line [ expr $xi + 2 ] [ expr $yi + 2 ] \
					[ expr $xi - 3 ] [ expr $yi - 3 ] \
					-width 1 -fill $fill -tags $tagsdots
				$c create line [ expr $xi + 2 ] [ expr $yi - 2 ] \
					[ expr $xi - 3 ] [ expr $yi + 3 ] \
					-width 1 -fill $fill -tags $tagsdots
			} elseif { $width < 1 } {
				# small point
				$c create oval [ expr $xi - 1 ] [ expr $yi - 1 ] \
					[ expr $xi + 1 ] [ expr $yi + 1 ] -fill $fill \
					-width 0 -outline white -tags $tagsdots
			} else {
				# filled circle
				$c create oval [ expr $xi - $width / 2 ] [ expr $yi - $width / 2 ] \
					[ expr $xi + $width / 2 ] [ expr $yi + $width / 2 ] -fill $fill \
					-width 0 -outline white -tags $tagsdots
			}
		}
	}
}


#************************************************
# GET_SERIES
# Set a byte array to hold data series that can be accessed from C
# Based on code by Arjen Markus (http://wiki.tcl.tk/4179) 
#************************************************
proc get_series { size data } {
	upvar $data _data

	set _data [ list ]
	# Create a list with the correct number of integer elements
	for { set i 0 } { $i < $size } { incr i } {
	   lappend _data 0
	}

	# Convert the list to a byte array
	set c_data [ intsToByteArray $_data ]

	# Call the C routine - that will fill the byte array
	upload_series $size $c_data

	# Convert the byte array into an ordinary list
	set _data [ byteArrayToInts $c_data ]
}


#************************************************
# LISTTOBYTEARRAY
# Generic routine to convert a list into a bytearray
#************************************************
proc listToByteArray { valuetype list { elemsize 0 } } {

	if { $valuetype == "i" || $valuetype == "I" } {
		if { $::tcl_platform(byteOrder) == "littleEndian" } {
			set valuetype "i"
		} {
			set valuetype "I"
		}
	}

	switch -- $valuetype {
		f - d - i - I {
		   set result [ binary format ${valuetype}* $list ]
		}
		s {
			set result { }
			foreach elem $list {
				append result [ binary format a$elemsize $elem ]
			}
		}
		default {
			error "Unknown value type: $valuetype"
		}
	}
	
	return $result
}

interp alias { } stringsToByteArray { } listToByteArray s
interp alias { } intsToByteArray    { } listToByteArray i
interp alias { } floatsToByteArray  { } listToByteArray f
interp alias { } doublesToByteArray { } listToByteArray d


#************************************************
# BYTEARRAYTOLIST
# Generic routine to convert a bytearray into a list
#************************************************
proc byteArrayToList { valuetype bytearray { elemsize 0 } } {

	if { $valuetype == "i" || $valuetype == "I" } {
		if { $::tcl_platform(byteOrder) == "littleEndian" } {
			set valuetype "i"
		} else {
			set valuetype "I"
		}
	}

	switch -- $valuetype {
		f - d - i - I {
		   binary scan $bytearray ${valuetype}* result
		}
		s {
			set result  { }
			set length  [ string length $bytearray ]
			set noelems [ expr { $length / $elemsize } ]
			for { set i 0 } { $i < $noelems } { incr i } {
				set elem [ string range $bytearray \
						[ expr { $i * $elemsize } ] \
						[ expr { ( $i + 1 ) * $elemsize - 1 } ] ]
				set posnull [ string first "\000" $elem ]
				if { $posnull != -1 } {
					set elem [ string range $elem 0 [ expr { $posnull - 1 } ] ]
				}
				lappend result $elem
			}
		}
		default {
			error "Unknown value type: $valuetype"
		}
	}
	
	return $result
}

interp alias { } byteArrayToStrings { } byteArrayToList s
interp alias { } byteArrayToInts    { } byteArrayToList i
interp alias { } byteArrayToFloats  { } byteArrayToList f
interp alias { } byteArrayToDoubles { } byteArrayToList d


#************************************************
# INIT_CANVAS_COLORS
# Initialize Tk canvas colors
#************************************************
# list of all Tk named colors
set allcolors { 
	snow {ghost white} {white smoke} gainsboro {floral white}
	{old lace} linen {antique white} {papaya whip} {blanched almond}
	bisque {peach puff} {navajo white} moccasin cornsilk ivory
	{lemon chiffon} seashell honeydew {mint cream} azure {alice blue}
	lavender {lavender blush} {misty rose} white black {dark slate gray}
	{dim gray} {slate gray} {light slate gray} gray {light grey}
	{midnight blue} navy {cornflower blue} {dark slate blue} {slate blue}
	{medium slate blue} {light slate blue} {medium blue} {royal blue}
	blue {dodger blue} {deep sky blue} {sky blue} {light sky blue}
	{steel blue} {light steel blue} {light blue} {powder blue}
	{pale turquoise} {dark turquoise} {medium turquoise} turquoise
	cyan {light cyan} {cadet blue} {medium aquamarine} aquamarine
	{dark green} {dark olive green} {dark sea green} {sea green}
	{medium sea green} {light sea green} {pale green} {spring green}
	{lawn green} green chartreuse {medium spring green} {green yellow}
	{lime green} {yellow green} {forest green} {olive drab} {dark khaki}
	khaki {pale goldenrod} {light goldenrod yellow} {light yellow} yellow
	gold {light goldenrod} goldenrod {dark goldenrod} {rosy brown}
	{indian red} {saddle brown} sienna peru burlywood beige wheat
	{sandy brown} tan chocolate firebrick brown {dark salmon} salmon
	{light salmon} orange {dark orange} coral {light coral} tomato
	{orange red} red {hot pink} {deep pink} pink {light pink}
	{pale violet red} maroon {medium violet red} {violet red}
	magenta violet plum orchid {medium orchid} {dark orchid} {dark violet}
	{blue violet} purple {medium purple} thistle snow2 snow3
	snow4 seashell2 seashell3 seashell4 AntiqueWhite1 AntiqueWhite2
	AntiqueWhite3 AntiqueWhite4 bisque2 bisque3 bisque4 PeachPuff2
	PeachPuff3 PeachPuff4 NavajoWhite2 NavajoWhite3 NavajoWhite4
	LemonChiffon2 LemonChiffon3 LemonChiffon4 cornsilk2 cornsilk3
	cornsilk4 ivory2 ivory3 ivory4 honeydew2 honeydew3 honeydew4
	LavenderBlush2 LavenderBlush3 LavenderBlush4 MistyRose2 MistyRose3
	MistyRose4 azure2 azure3 azure4 SlateBlue1 SlateBlue2 SlateBlue3
	SlateBlue4 RoyalBlue1 RoyalBlue2 RoyalBlue3 RoyalBlue4 blue2 blue4
	DodgerBlue2 DodgerBlue3 DodgerBlue4 SteelBlue1 SteelBlue2
	SteelBlue3 SteelBlue4 DeepSkyBlue2 DeepSkyBlue3 DeepSkyBlue4
	SkyBlue1 SkyBlue2 SkyBlue3 SkyBlue4 LightSkyBlue1 LightSkyBlue2
	LightSkyBlue3 LightSkyBlue4 SlateGray1 SlateGray2 SlateGray3
	SlateGray4 LightSteelBlue1 LightSteelBlue2 LightSteelBlue3
	LightSteelBlue4 LightBlue1 LightBlue2 LightBlue3 LightBlue4
	LightCyan2 LightCyan3 LightCyan4 PaleTurquoise1 PaleTurquoise2
	PaleTurquoise3 PaleTurquoise4 CadetBlue1 CadetBlue2 CadetBlue3
	CadetBlue4 turquoise1 turquoise2 turquoise3 turquoise4 cyan2 cyan3
	cyan4 DarkSlateGray1 DarkSlateGray2 DarkSlateGray3 DarkSlateGray4
	aquamarine2 aquamarine4 DarkSeaGreen1 DarkSeaGreen2 DarkSeaGreen3
	DarkSeaGreen4 SeaGreen1 SeaGreen2 SeaGreen3 PaleGreen1 PaleGreen2
	PaleGreen3 PaleGreen4 SpringGreen2 SpringGreen3 SpringGreen4
	green2 green3 green4 chartreuse2 chartreuse3 chartreuse4
	OliveDrab1 OliveDrab2 OliveDrab4 DarkOliveGreen1 DarkOliveGreen2
	DarkOliveGreen3 DarkOliveGreen4 khaki1 khaki2 khaki3 khaki4
	LightGoldenrod1 LightGoldenrod2 LightGoldenrod3 LightGoldenrod4
	LightYellow2 LightYellow3 LightYellow4 yellow2 yellow3 yellow4
	gold2 gold3 gold4 goldenrod1 goldenrod2 goldenrod3 goldenrod4
	DarkGoldenrod1 DarkGoldenrod2 DarkGoldenrod3 DarkGoldenrod4
	RosyBrown1 RosyBrown2 RosyBrown3 RosyBrown4 IndianRed1 IndianRed2
	IndianRed3 IndianRed4 sienna1 sienna2 sienna3 sienna4 burlywood1
	burlywood2 burlywood3 burlywood4 wheat1 wheat2 wheat3 wheat4 tan1
	tan2 tan4 chocolate1 chocolate2 chocolate3 firebrick1 firebrick2
	firebrick3 firebrick4 brown1 brown2 brown3 brown4 salmon1 salmon2
	salmon3 salmon4 LightSalmon2 LightSalmon3 LightSalmon4 orange2
	orange3 orange4 DarkOrange1 DarkOrange2 DarkOrange3 DarkOrange4
	coral1 coral2 coral3 coral4 tomato2 tomato3 tomato4 OrangeRed2
	OrangeRed3 OrangeRed4 red2 red3 red4 DeepPink2 DeepPink3 DeepPink4
	HotPink1 HotPink2 HotPink3 HotPink4 pink1 pink2 pink3 pink4
	LightPink1 LightPink2 LightPink3 LightPink4 PaleVioletRed1
	PaleVioletRed2 PaleVioletRed3 PaleVioletRed4 maroon1 maroon2
	maroon3 maroon4 VioletRed1 VioletRed2 VioletRed3 VioletRed4
	magenta2 magenta3 magenta4 orchid1 orchid2 orchid3 orchid4 plum1
	plum2 plum3 plum4 MediumOrchid1 MediumOrchid2 MediumOrchid3
	MediumOrchid4 DarkOrchid1 DarkOrchid2 DarkOrchid3 DarkOrchid4
	purple1 purple2 purple3 purple4 MediumPurple1 MediumPurple2
	MediumPurple3 MediumPurple4 thistle1 thistle2 thistle3 thistle4 
}

proc init_canvas_colors { } {
	global defcolors allcolors
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
	set n [ expr $i + [ llength $unusedcolors ] ]
	for { } { $i < $n } { incr i } {
		# pick random color
		set m [ llength $unusedcolors ]
		set j [ expr min( int( rand( ) * $m ), $m - 1 ) ]
		set color [ lindex $unusedcolors $j ]
		set ::c$i $color
		
		# remove from list
		set pos [ lsearch -sorted $unusedcolors $color ]
		set unusedcolors [ lreplace $unusedcolors $pos $pos ]
	}

	# fill the rest till color 1000
	if { $i < 1000 } {
		for { } { $i < 1000 } { incr i } {
			set ::c$i black
		}
	}
	set ::c$i white
	
	# fill the colors 1000-1099 with gray shades
	for { set j 0 } { $j < 10 } { incr j } {
		for { set k 0 } { $k < 10 } { incr k; incr i } {
			if { ! ( $j == 0 && $k == 0 ) } {
				if { $k == 0 } {
					set ::c$i gray$j
				} {
					set ::c$i gray$k$j
				}
			}			
		}
	}
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
# ROUND_N
# Round float to N decimal positions
#************************************************
proc round_N { float N } {
	return [ expr round( $float * pow( 10, $N ) ) / pow( 10, $N ) ]
}


#************************************************
# CURRENT_DATE
# Current date in the default date format
#************************************************
proc current_date { } {
	global DATE_FMT
	
	return [ clock format [ clock seconds ] -format $DATE_FMT ]
}


#************************************************
# OPEN_GNUPLOT
# Open gnuplot window
#************************************************
proc open_gnuplot { { script "" } { errmsg "" } { wait false } { par ".da" } } {
	global tcl_platform sysTerm
	
	if [ string equal $script "" ] { 
		set args "" 
	} else {
		set args "-p $script"
	}
	
	if [ string equal $tcl_platform(platform) unix ] {
		if [ string equal $tcl_platform(os) Darwin ] {
			if { $wait } {
				set ret [ catch { exec osascript -e "tell application \"$sysTerm\" to do script \"cd [ pwd ]; gnuplot $script; exit\"" } ]
			} else {
				set ret [ catch { exec osascript -e "tell application \"$sysTerm\" to do script \"cd [ pwd ]; gnuplot $args; exit\"" & } ]
			}
		} else {
			if { $wait } {
				set ret [ catch { exec $sysTerm -e "gnuplot $script; exit" } ]
			} else {
				set ret [ catch { exec $sysTerm -e "gnuplot $args; exit" & } ]
			}
		}
	} else {
		if [ string equal $script "" ] { 
			set ret [ catch { exec wgnuplot.exe & } ]
		} else {
			if { $wait } {
				set ret [ catch { exec wgnuplot.exe $script } ]
			} else {
				set ret [ catch { exec wgnuplot.exe -p $script & } ]
			}
		}
	}

	if { $ret != 0 } {
		if [ string equal $errmsg "" ] { 
			set errmsg "Please check if Gnuplot is installed and set up properly." 
		}
		
		tk_messageBox -parent $par -type ok -icon error -title Error -message "Gnuplot failed to launch" -detail "Gnuplot returned error '$ret'.\n$errmsg" 
	}			

	return $ret
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


#************************************************
# CONWND
# load and set console configuration
#************************************************
if { $conWnd } {
	set msg "File(s) missing or corrupted"
	set det "Tcl/Tk console file 'tkcon.tcl' is missing or corrupted.\nPlease check your installation and reinstall LSD if the problem persists.\n\nLSD is continuing without console support."
	if [ file exists "$RootLsd/$LsdSrc/tkcon.tcl" ] {
		if { [ catch { source "$RootLsd/$LsdSrc/tkcon.tcl" } ] == 0 } {
			set tkcon::PRIV(showOnStartup) 0
			set tkcon::PRIV(root) .console
			set tkcon::PRIV(protocol) { tkcon hide }
			set tkcon::OPT(exec) ""
		} else {
			set conWnd false
			tk_messageBox -type ok -icon warning -title Warning -message $msg -detail $det
		}
	} else {
		set conWnd false
			tk_messageBox -type ok -icon warning -title Warning -message $msg -detail $det
	}
}


#************************************************
# TESTWND
# Open test window if enabled
#************************************************
if $testWnd {
	newtop .tst "LSD Coordinates Test Window" { destroytop .tst } ""
	
	frame .tst.xy
	label .tst.xy.l1 -anchor e -text "X:"
	label .tst.xy.v1 -anchor w -fg red
	label .tst.xy.l2 -anchor e -text "   Y:"
	label .tst.xy.v2 -anchor w -fg red
	pack .tst.xy.l1 .tst.xy.v1 .tst.xy.l2 .tst.xy.v2 -side left -padx 2 -pady 2
	
	frame .tst.r
	label .tst.r.l1 -anchor e -text "rootx:"
	label .tst.r.v1 -anchor w -fg red
	label .tst.r.l2 -anchor e -text "   rooty:"
	label .tst.r.v2 -anchor w -fg red
	pack .tst.r.l1 .tst.r.v1 .tst.r.l2 .tst.r.v2 -side left -padx 2 -pady 2
	
	frame .tst.v
	label .tst.v.l1 -anchor e -text "vrootx:"
	label .tst.v.v1 -anchor w -fg red
	label .tst.v.l2 -anchor e -text "   vrooty:"
	label .tst.v.v2 -anchor w -fg red
	pack .tst.v.l1 .tst.v.v1 .tst.v.l2 .tst.v.v2 -side left -padx 2 -pady 2
	
	frame .tst.s
	label .tst.s.l1 -anchor e -text "screenwidth:"
	label .tst.s.v1 -anchor w -fg red
	label .tst.s.l2 -anchor e -text "   screenheight:"
	label .tst.s.v2 -anchor w -fg red
	pack .tst.s.l1 .tst.s.v1 .tst.s.l2 .tst.s.v2 -side left -padx 2 -pady 2
	
	frame .tst.t
	label .tst.t.l1 -anchor e -text "vrootwidth:"
	label .tst.t.v1 -anchor w -fg red
	label .tst.t.l2 -anchor e -text "   vrootheight:"
	label .tst.t.v2 -anchor w -fg red
	pack .tst.t.l1 .tst.t.v1 .tst.t.l2 .tst.t.v2 -side left -padx 2 -pady 2
	
	frame .tst.m
	label .tst.m.l1 -anchor e -text "maxwidth:"
	label .tst.m.v1 -anchor w -fg red
	label .tst.m.l2 -anchor e -text "   maxheight:"
	label .tst.m.v2 -anchor w -fg red
	pack .tst.m.l1 .tst.m.v1 .tst.m.l2 .tst.m.v2 -side left -padx 2 -pady 2
	
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
