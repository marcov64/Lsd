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
# UTIL.TCL
# Collection of general utility procedures.
#*************************************************************

#************************************************
# BGERROR
# Handle Tcl background errors (mostly in events)
#************************************************
proc bgerror { message } {
	global errorInfo
	
	log_tcl_error $errorInfo $message
}


#************************************************
# LSDABOUT
# Show LSD About dialog box
#************************************************
proc LsdAbout { ver dat { parWnd "." } } {

	set copyr "written by Marco Valente, Universita' dell'Aquila\nand Marcelo Pereira, University of Campinas\n\nCopyright Marco Valente and Marcelo Pereira\nLSD is distributed under the GNU General Public License\nLSD is free software and comes with ABSOLUTELY NO WARRANTY\n\nSee Readme.txt for copyright information of third parties' code used in LSD"
	
	ttk::messageBox -parent $parWnd -type ok -icon info -title "About LSD" -message "LSD Version $ver ($dat)" -detail "$copyr\n\n[ LsdEnv \n ]"
}


#************************************************
# LSDENV
# Return current LSD working environment
#************************************************
proc LsdEnv { sep } {
	global tcl_platform

	set plat [ string totitle $tcl_platform(platform) ];
	set mach $tcl_platform(machine)
	set os $tcl_platform(os)
	set osV $tcl_platform(osVersion)
	set tclV [ info patch ]
	set gccV [ gccVersion ]
	
	return "Platform: $plat ($mach)${sep}OS: $os ($osV)${sep}Tcl/Tk: $tclV\nCompiler: $gccV"
}


#************************************************
# GCCVERSION
# Get GCC compiler version string
#************************************************
proc gccVersion { } {
	global RootLsd LsdSrc SYSTEM_OPTIONS
	
	if { ! [ file exists "$RootLsd/$LsdSrc/$SYSTEM_OPTIONS" ] } {
		return "(system options missing)"
	}
	
	set f [ open "$RootLsd/$LsdSrc/$SYSTEM_OPTIONS" r ]
	set a [ read -nonewline $f ]
	close $f
	
	set p [ string first "CC=" [ string toupper $a ] ]
	if { $p < 0 || [ string index $a [ expr { $p - 1 } ] ] == "_" } {
		return "(invalid system options)"
	}
	
	set p [ expr { $p + [ string length "CC=" ] } ]
	set e [ string first "\n" $a $p ]
	if { $e < 0 } {
		set e end
	}
	
	set cc [ string trim [ string range $a $p $e ] ]
	if { [ string length $cc ] == 0 } {
		return "(invalid system options)"
	}
	
	if { [ catch { exec $cc --version } r ] && ( ! [ info exists r ] || [ string length $r ] == 0 ) } {
		return "(cannot run compiler)"
	}
	
	set e [ string first "\n" $r ]
	if { $e < 0 } {
		set e end
	}
	
	set v [ string trim [ string range $r 0 $e ] ]
	if { [ string length $v ] == 0 } {
		return "(unknown compiler version)"
	} else {
		return $v
	}
}


#************************************************
# ADD_WIN_PATH
# Add the directory to the permanent
# PATH environment variable in Windows
#************************************************
proc add_win_path { path { prof user } { pos end } } {
	global CurPlatform env

	# only Windows
	if { ! [ string equal $CurPlatform windows ] } {
		return 0
	}
	
	# check if not already in global (user + system) path
	set pathList [ list ]
	foreach part [ split $env(PATH) ";" ] {
		set part [ file normalize $part ]
		if { [ file exists $part ] && [ lsearch -exact $pathList $part ] < 0 } {
			lappend pathList $part
		}
	}

	if { [ catch { set path [ file normalize $path ] } ] || ! [ file exists $path ] } {
		return 0
	} 
	
	set i [ lsearch -exact $pathList $path ]
	if { ( $pos eq "end" && $i >= 0 ) || ( $pos ne "end" && $i == 0 ) } {
		return 1
	}
	
	if { $prof eq "user" } {
		set regPath "HKEY_CURRENT_USER\\Environment"
	} elseif { $prof eq "system" } {
		set regPath "HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment"
	} else {
		return 0
	}
	
	set path [ file nativename $path ]
	if { ! [ catch { set curPath [ registry get $regPath "Path" ] } ] } {
		set curPath [ string trimright $curPath ";" ]

		# add to the end or beginning of the path
		if { $pos eq "end" } {
			set newPath "$curPath;$path;"
		} else {
			set newPath "$path;$curPath;"
		}
		
		if { ! [ catch { registry set $regPath "Path" "$newPath" } ] } {
			registry broadcast $regPath
			return 1
		}
	}
	
	return 0
}


#************************************************
# PROGRESSBOX
# Show interruptible progress bar window while 
# slow operations are running
#************************************************
proc progressbox { w tit lab elem1 { max1 1 } { destroy "" } { par . } { elem2 "" } { max2 1 } } {
	
	newtop $w $tit $destroy $par

	ttk::frame $w.main
	ttk::label $w.main.lab -text $lab
	pack $w.main.lab -pady 10
	
	ttk::frame $w.main.p1
	ttk::progressbar $w.main.p1.scale -length 300 -maximum $max1
	ttk::frame $w.main.p1.info
	ttk::label $w.main.p1.info.elem -text "[ string totitle $elem1 ]:"
	ttk::label $w.main.p1.info.val
	pack $w.main.p1.info.elem $w.main.p1.info.val -padx 1 -side left
	pack $w.main.p1.scale $w.main.p1.info -pady 2

	if { $elem1 != "" } {
		pack $w.main.p1 -pady 5
	}
		
	ttk::frame $w.main.p2
	ttk::progressbar $w.main.p2.scale -length 300 -maximum $max2
	ttk::frame $w.main.p2.info
	ttk::label $w.main.p2.info.elem -text "[ string totitle $elem2 ]:"
	ttk::label $w.main.p2.info.val
	pack $w.main.p2.info.elem $w.main.p2.info.val -padx 1 -side left
	pack $w.main.p2.scale $w.main.p2.info -pady 2
	
	if { $elem2 != "" } {
		pack $w.main.p2 -pady 5
	}
	
	pack $w.main -padx 10 -pady 10

	if { $destroy != "" } {
		cancel $w b $destroy
	}

	# handle installer with main window withdrawn
	if { $par == "" } {
		showtop $w centerS no no no 0 0 b no yes
	} else {
		showtop $w centerW
	}
	
	prgboxupdate $w 0 0
	
	if { $elem1 != "" } {
		return $w.main.p1.info.val
	} else {
		return $w.main.p2.info.val
	}
}


#************************************************
# PRGBOXUPDATE
# Updates an existing progressbox
#************************************************
proc prgboxupdate { w last1 { last2 "" } } {
	
	if { ! [ winfo exists $w.main.p1.info.val ] || ! [ winfo exists $w.main.p1.scale ] } {
		return
	}
	
	set max1 [ $w.main.p1.scale cget -maximum ]
	set max2 [ $w.main.p2.scale cget -maximum ]
	
	if { $last1 != "" && [ string is integer -strict $last1 ] } {
		$w.main.p1.scale configure -value $last1
		$w.main.p1.info.val configure -text "[ expr { min( $last1 + 1, $max1 ) } ] of $max1 ([ expr { int( 100 * $last1 / $max1 ) } ]% done)"
	}
	
	if { $last2 != "" && [ string is integer -strict $last2 ] } {
		$w.main.p2.scale configure -value $last2
		$w.main.p2.info.val configure -text "[ expr { min( $last2 + 1, $max2 ) } ] of $max2 ([ expr { int( 100 * $last2 / $max2 ) } ]% done)"
	}
	
	update idletasks
}


#************************************************
# WAITBOX
# Show non-interruptible window while slow
# external scripts are running
#************************************************
proc waitbox { w tit msg { steps "" } { timer no } { par . } } {
	global frPadX frPadY
	
	newtop $w "$tit" { } $par 1

	ttk::frame $w.main
	ttk::label $w.main.msg -justify center -text "$msg"
	pack $w.main.msg -pady 10
	
	if { $steps != "" } {
		ttk::frame $w.main.steps -relief solid -borderwidth 1 -padding [ list $frPadX $frPadY ]
		ttk::label $w.main.steps.txt -text $steps
		pack $w.main.steps.txt -padx 5 -pady 5
		pack $w.main.steps -pady 10
	}
	
	if { $timer } {
		ttk::frame $w.main.time
		ttk::label $w.main.time.lab -text "Elapsed time:"
		ttk::label $w.main.time.val -style hl.TLabel -text "00:00"
		pack $w.main.time.lab $w.main.time.val -padx 5 -side left
		pack $w.main.time -pady 10
		set retVal $w.main.time.val
	} else {
		set retVal ""
	}
	
	pack $w.main -padx 20 -pady 20
	
	# handle installer with main window withdrawn
	if { $par == "" } {
		showtop $w centerS no no no 0 0 "" no yes
	} else {
		showtop $w centerW
	}
	
	# try to workaround Mac bug showing black frame
	raise $w
	focus $w.main
	
	return $retVal
}


#************************************************
# INIT_SERIES
# Initialize the main AoR series listbox control 
# data structures
#************************************************
proc init_series { fltcb serlb sern casn sellb seln pltlb pltn } {
	global parAll serPar parChg parFlt serSel serList parList serOrd serNext serParDict serNdict fltCbox serLbox selLbox pltLbox serNlab casNlab selNlab pltNlab DaModElem DaModPar

	set parChg 0
	set parFlt $parAll
	set serPar "$parAll (0)"
	set serSel 0
	set serOrd none
	set fltCbox $fltcb
	set serLbox $serlb
	set selLbox $sellb
	set pltLbox $pltlb
	set serNlab $sern
	set casNlab $casn
	set selNlab $seln
	set pltNlab $pltn
	
	set serList [ list ]
	set parList [ list "$parAll (0)" ]
	set serNext [ list ]
	set DaModElem [ list ]
	set DaModPar [ list $parAll ]
	
	set serParDict [ dict create ]
	set serNdict [ dict create ]
}


#************************************************
# STAT_SERIES
# Updates the main AoR series listboxes stats
#************************************************
proc stat_series { } {
	global fltCbox serLbox selLbox pltLbox serNlab casNlab selNlab pltNlab numc
	
	$serNlab configure -text [ $serLbox size ]
	$casNlab configure -text $numc
	$selNlab configure -text [ $selLbox size ]
	$pltNlab configure -text [ $pltLbox size ]
			
	if { [ $fltCbox current ] < 0 } {
		$fltCbox configure -values [ update_parent ]
	}
				
}


#************************************************
# UPDATE_PARENT
# Generate the AoR drop-down list of parents
#************************************************
proc update_parent { } {
	global parAll serPar parChg parList serSel serParDict serNdict serLbox DaModPar
	
	set serSel [ lindex [ $serLbox curselection ] 0 ]
	
	if { $parChg } {
		set parList [ list ]
		foreach par $DaModPar {
			if { $par eq $parAll } {
				set vars [ dict keys $serParDict ]
			} else {
				set vars [ dict keys [ dict filter $serParDict value $par ] ]
			}
			
			set n 0
			foreach var $vars  {
				incr n [ dict get $serNdict $var ]
			}
				
			lappend parList "$par ($n)"
			
			if { [ lindex $serPar 0 ] eq $par } {
				set serPar "$par ($n)"
			}
		}
		
		set parChg 0
	}
	
	return $parList
}


#************************************************
# FILTER_SERIES
# Filter the main AoR series listbox to show  
# just series from one parent/source
#************************************************
proc filter_series { { par "" } } {
	global parAll serPar parFlt parList serSel serList serOrd serParDict fltCbox serLbox
	
	if { $par eq "" } {
		set par [ lindex $serPar 0 ]
	} else {
		set i 0
		set found 0
		foreach p $parList {
			if { [ lindex $p 0 ] eq $par } {
				set found 1
				set $serPar "$p"
				$fltCbox current $i
				break
			}
			
			incr i
		}
		
		if { ! $found } {
			return
		}
	}
	
	if { $par ne $parFlt } {
		$serLbox delete 0 end
		
		foreach ser $serList {
			if { $par eq $parAll || $par eq [ dict get $serParDict [ lindex $ser 0 ] ] } {
				insert_series $serLbox $ser
			}
		}

		set parFlt $par
		set serSel 0
		set serOrd none
		stat_series
	}

	selectinlist $serLbox $serSel 1
}


#************************************************
# SEARCH_SERIES
# Search for series to the main AoR series listbox, 
# expanding the selection if not in current parent
#************************************************
proc search_series { { text "" } } {
	global parAll parFlt serList serParDict serNext serLbox

	if { [ string length $text ] > 0 } {
		set serNext [ list ]
	}
	
	if { [ llength $serNext ] == 0 && [ string length $text ] > 0 } {
	
		if [ dict exists $serParDict $text ] {
			set matches [ list $text ]
		} else {
			set matches [ lsearch -all -inline [ dict keys $serParDict ] "*$text*" ]
			
			if { [ llength $matches ] == 0 } {
				set matches [ lsearch -all -inline -nocase [ dict keys $serParDict ] "*$text*" ]
			}
		}
		
		if { [ llength $matches ] == 0 } {
			return 0
		}
		
		foreach ser $matches {
			foreach serlin $serList {
				if { $ser eq [ lindex $serlin 0 ] } {
					lappend serNext $serlin
				}
			}
		}
	}
	
	if { [ llength $serNext ] == 0 } {
		return 0
	}
		
	set serlin [ lindex $serNext 0 ]
	set serNext [ lrange $serNext 1 end ]
	set par [ dict get $serParDict [ lindex $serlin 0 ] ]

	if { $parFlt ne $par } {
		filter_series $par
	}
	
	selectinlist $serLbox [ lsearch -exact [ $serLbox get 0 end ] $serlin ] 1
	
	return 1
}


#************************************************
# ADD_SERIES
# Add new series to the main AoR series listbox, 
# updating the lists/dictionary
#************************************************
proc add_series { ser par } {
	global parAll serPar parChg parFlt serList serParDict serNdict serLbox DaModElem DaModPar
	
	if { $parFlt ne $parAll } {
		filter_series $parAll
	}
	
	lappend serList "$ser"
	insert_series $serLbox "$ser"
	
	set nam [ lindex $ser 0 ]
	dict incr serNdict $nam
	
	if { ! [ dict exists $serParDict $nam ] } {
		set par [ file tail $par ]
	
		dict set serParDict $nam $par
		lappend DaModElem $nam
		
		if { [ lsearch -exact $DaModPar $par ] < 0 } {
			lappend DaModPar $par
		}
	}
	
	set parChg 1
}


#************************************************
# INSERT_SERIES
# Append series to an AoR listbox, coloring the 
# entry according to the origin of the series
#************************************************
proc insert_series { lbox ser { pos end } } {
	global colorsTheme

	set orig [ lindex [ split [ lindex [ split $ser ] 1 ] _ ] 0 ]
	switch $orig {
		U {
			set color $colorsTheme(var)
		}
		C {
			set color $colorsTheme(lvar)
		}
		F {
			set color $colorsTheme(fun)
		}
		MC {
			set color $colorsTheme(lfun)
		}
		default {
			set color ""
		}
	}

	$lbox insert $pos "$ser"
	
	if { $color != "" } {
		$lbox itemconfigure $pos -fg $color
	}
}


#************************************************
# SORT_SERIES
# Sort series to an AoR listbox, according to
# the selected criterion, if different
#************************************************
proc sort_series { lbox ord } {
	global parAll parFlt serList serOrd serLbox selLbox
	
	if { ( $lbox eq $serLbox && $serOrd ne $ord ) || $lbox eq $selLbox } {
		set ss [ $lbox get 0 end ]
		if { [ llength $ss ] > 1 } {
			switch -glob $ord {
				inc* {
					set ss [ lsort -command comp_und_inc $ss ]
				}
				dec* {
					set ss [ lsort -command comp_und_dec $ss ]
				}
				rev* {
					set ss [ lreverse $ss ]
				}
				nice {
					set ss [ lsort -command comp_nice $ss ]
				}
				none -
				default {
					if { $lbox eq $serLbox && $parFlt eq $parAll } {
						set ss $serList
					} else { 
						set ss [ lsort -command comp_rank $ss ]
					}
					
					set ord none
				}
			}
			
			$lbox delete 0 end
			foreach s $ss {
				insert_series $lbox "$s"
			}
			
			if { $lbox eq $serLbox } {
				set serOrd $ord
				set serSel 0
				selectinlist $serLbox $serSel 1
			}
		}
	}
	
	focus $lbox
}


#************************************************
# COMP_RANK
# Special sort procedure to sort according 
# series unique rank (serial creation number)
#************************************************
proc comp_rank { a b } {
	scan $a "%*s %*s %*s #%d" ar
	scan $b "%*s %*s %*s #%d" br
if { ! [ info exists ar ] } { tk_messageBox -message "$a\n$b" }
	return [ expr { $ar - $br } ]
}


#************************************************
# COMP_NICE
# Special sort procedure to sort according to:
# 1) name (increasing: A first z last, underscored at end)
# 2) time of last occurrence (decreasing: last updated first)
# 3) time of first occurrence (increasing: older first)
# 4) rank (increasing)
#************************************************
proc comp_nice { a b } {

	scan $a "%s %*s (%d-%d) #%d" an ab ae ar
	scan $b "%s %*s (%d-%d) #%d" bn bb be br
	
	set d [ comp_und_inc $an $bn ]
	if { $d != 0 } {
		return $d
	}
	
	if { $ae != $be } {
		return [ expr { $be - $ae } ]
	}
	
	if { $ab != $bb } {
		return [ expr { $ab - $bb } ]
	}
	
	return [ expr { $ar - $br } ]
}


#************************************************
# COMP_UND_INC
# Special increasing sort procedure to keep names 
# starting with underline(s) at the end
#************************************************
proc comp_und_inc { a b } {

	if { [ string index $a 0 ] ne "_" } {
		if { [ string index $b 0 ] ne "_" } {
			return [ str_comp_dict $a $b ]
		} else {
			return -1
		}
	} 
	
	if { [ string index $b 0 ] eq "_" } {
		return [ comp_und_inc [ string range $a 1 end ] [ string range $b 1 end ] ]
	}
		
	return 1
}


#************************************************
# COMP_UND_DEC
# Special decreasing sort procedure to keep names 
# starting with underline(s) at the end
#************************************************
proc comp_und_dec { a b } {

	if { $a eq $b } {
		return 0
	}
	
	if { [ string index $a 0 ] ne "_" } {
		if { [ string index $b 0 ] ne "_" } {
			return [ expr { - [ str_comp_dict $a $b ] } ]
		} else {
			return -1
		}
	} 
	
	if { [ string index $b 0 ] eq "_" } {
		return [ comp_und_dec [ string range $a 1 end ] [ string range $b 1 end ] ]
	}
		
	return 1
}


#************************************************
# STR_COMP_DICT
# Compare two strings as in lsort -dictionary
# in increasing order
#************************************************
proc str_comp_dict { a b } {
	dict get {1 0  {0 1} -1  {1 0} 1} [ lsort -indices -dictionary -unique [ list $a $b ] ]
}


#************************************************
# ROUND_N
# Round float to N decimal positions
#************************************************
proc round_N { float N } {
	return [ expr { round( $float * pow( 10, $N ) ) / pow( 10, $N ) } ]
}


#************************************************
# FORMATFLOAT
# Nice format of floats with N significant 
# digits without losing precision
#************************************************
proc formatfloat { float { N 6 } } {
	set prec 1e-12
	set fmt "%.${N}g"
	
	set fmtFlt [ format $fmt $float ]
	if { abs( $fmtFlt - $float ) < $prec } { 
		return $fmtFlt
	} elseif { abs( $float - int( $float ) ) < $prec } {
		return [ expr { int( $float ) } ]
	} else {
		return $float
	}
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
# INVERT_COLOR
# Invert a color in Tk format
#************************************************

proc invert_color { color } {
	global colorsTheme

	if [ catch { set rgbColor [ winfo rgb . $color ] } ] {
		return $colorsTheme(fg)
	}
	
	set rgbInvert [ list [ expr { 65535 - [ lindex $rgbColor 0 ] } ] \
						 [ expr { 65535 - [ lindex $rgbColor 1 ] } ] \
						 [ expr { 65535 - [ lindex $rgbColor 2 ] } ] ]
						 
	return [ format "#%04x%04x%04x" {*}$rgbInvert ]
}


#************************************************
# CHECK_SYS_OPT
# Check (best guess) if system option configuration is valid for the platform
#************************************************
set winYes [ list "86" "-ltcl" "-ltk" "-lz" "-mthreads" "-mwindows" "LSDROOT" "SRC" "PATH_TCLTK_HEADER" "PATH_TCLTK_LIB" "TCLTK_LIB" "PATH_HEADER" "PATH_LIB" "LIB" "WRC" "CC" "GLOBAL_CC" "SSWITCH_CC" ]
set winNo  [ list "-framework" "-lpthread" "EXT" "PATH_TCL_HEADER" "PATH_TK_HEADER" "PATH_TCL_LIB" "PATH_TK_LIB" "TCL_LIB" "LIBS" ]
set linuxYes [ list "8.6" "-ltcl" "-ltk" "-lz" "-lpthread" "LSDROOT" "SRC" "PATH_TCLTK_HEADER" "PATH_TCLTK_LIB" "TCLTK_LIB" "PATH_HEADER" "PATH_LIB" "LIB" "CC" "GLOBAL_CC" "SSWITCH_CC" ]
set linuxNo  [ list "windres" "-framework" "-mthreads" "-mwindows" "PATH_TCL_HEADER" "PATH_TK_HEADER" "PATH_TCL_LIB" "PATH_TK_LIB" "TCL_LIB" "LIBS" "WRC" ]
set macYes [ list "-framework" "-lz" "-lpthread" "LSDROOT" "SRC" "PATH_TCL_HEADER" "PATH_TK_HEADER" "PATH_TCLTK_LIB" "TCLTK_LIB" "PATH_HEADER" "PATH_LIB" "LIB" "CC" "GLOBAL_CC" "SSWITCH_CC" ]
set macNo  [ list "86" "8.6" "windres" "-mthreads" "-mwindows" "PATH_TCLTK_HEADER" "PATH_TCL_LIB" "PATH_TK_LIB" "TCL_LIB" "LIBS" "WRC" ]

proc check_sys_opt { } {
	global RootLsd LsdSrc CurPlatform winYes winNo winYes winNo linuxYes linuxNo macYes macNo SYSTEM_OPTIONS
	
	if { ! [ file exists "$RootLsd/$LsdSrc/$SYSTEM_OPTIONS" ] } { 
		return "File '$SYSTEM_OPTIONS' not found (click 'Default' button to recreate it)"
	}
	
	set f [ open "$RootLsd/$LsdSrc/$SYSTEM_OPTIONS" r ]
	set options [ read -nonewline $f ]
	close $f
	
	switch $CurPlatform {
		windows {
			set yes $winYes
			set no $winNo
		}
		linux {
			set yes $linuxYes
			set no $linuxNo
		}
		mac {
			set yes $macYes
			set no $macNo
		}
	}
	
	set missingItems ""
	set yesCount 0
	foreach yesItem $yes {
		if { [ string first $yesItem $options ] >= 0 } {
			incr yesCount
		} else {
			lappend missingItems $yesItem
		}
	}
		
	set invalidItems ""
	set noCount 0
	foreach noItem $no {
		if { [ string first $noItem $options ] >= 0 } {
			incr noCount
			lappend invalidItems $noItem
		}
	}
	
	if { $noCount > 0 } {
		return "Invalid option(s) detected (click 'Default' button to recreate options)"
	}
	
	if { $yesCount < [ llength $yes ] } {
		return "Missing option(s) detected (click 'Default' button to recreate options)"	
	}
	
	return ""
}


#************************************************
# SAV_CUR_INI
# Save LMM cursor environment before
# changes in text window for syntax coloring
#************************************************
proc sav_cur_ini { } {
	global curSelIni curPosIni
	
	set curSelIni [ .f.t.t tag nextrange sel 1.0 ]
	set curPosIni [ .f.t.t index insert ]
	.f.t.t edit modified false
}


#************************************************
# SAV_CUR_END
# Save LMM cursor environment after 
# changes in text window for syntax coloring
#************************************************
proc sav_cur_end { } {
	global curSelFin curPosFin
	
	set curSelFin [ .f.t.t tag nextrange sel 1.0 ]
	set curPosFin [ .f.t.t index insert ]
	.f.t.t edit modified false
}


#************************************************
# UPD_COLOR
# Update LMM text window syntax coloring 
#************************************************
proc upd_color { { force 0 } } {
	global choice
	
	if { $force || [ .f.t.t edit modified ] } {
		sav_cur_end
		set choice 23
	}
	
	upd_cursor
}


#************************************************
# UPD_CURSOR
# Update LMM cursor coordinates window 
#************************************************
proc upd_cursor { } {
	.f.hea.cur.line.ln2 configure -text [ lindex [ split [ .f.t.t index insert ] . ] 0 ]
	.f.hea.cur.col.col2 configure -text [ expr { 1 + [ lindex [ split [ .f.t.t index insert ] . ] 1 ] } ]
}


#************************************************
# PLOG
# Tcl/Tk version of C "plog" function to 
# show a string in the LSD Log window
#************************************************
proc plog { cm { tag "" } } {
	.log.text.text.internal insert end $cm $tag
	.log.text.text.internal see end
}

	
#************************************************
# UPD_MENU_VISIB
# Update active menu options in LSD Model Browser
# according to panel in use
#************************************************
proc upd_menu_visib { } {
	global listfocus prevlistfocus
	
	if { $listfocus == 1 && $prevlistfocus != 1 } {
		.m.model.sort entryconfig 2 -state normal
		.m.model.sort entryconfig 3 -state normal
		.m.model.sort entryconfig 4 -state normal
		.m.model.sort entryconfig 5 -state normal
	}
	
	if { $listfocus == 2  && $prevlistfocus != 2 } {
		.m.model.sort entryconfig 2 -state disabled
		.m.model.sort entryconfig 3 -state disabled
		.m.model.sort entryconfig 4 -state disabled
		.m.model.sort entryconfig 5 -state disabled
	}
	
	set prevlistfocus $listfocus
}


#************************************************
# LISTTOBYTEARRAY
# Generic routine to convert a list into a bytearray
#************************************************
proc listToByteArray { valuetype list { elemsize 0 } } {
	global tcl_platform
	
	if { $valuetype == "i" || $valuetype == "I" } {
		if [ string equal $tcl_platform(byteOrder) littleEndian ] {
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
	global tcl_platform

	if { $valuetype == "i" || $valuetype == "I" } {
		if [ string equal $tcl_platform(byteOrder) littleEndian ] {
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
