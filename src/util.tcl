#*************************************************************
#
#	LSD 7.3 - December 2020
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
	if { $p < 0 || [ string index $a [ expr $p - 1 ] ] == "_" } {
		return "(invalid system options)"
	}
	
	set p [ expr $p + [ string length "CC=" ] ]
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
# PROGRESSBOX
# Show interruptible progress bar window while 
# slow operations are running
#************************************************
proc progressbox { w tit lab max var { destroy "" } { par . } } {
	
	newtop $w $tit $destroy $par

	ttk::frame $w.main
	ttk::label $w.main.lab -text $lab
	ttk::progressbar $w.main.scale -length 300 -maximum $max -variable {*}$var
	ttk::label $w.main.info
	pack $w.main.lab $w.main.scale $w.main.info -pady 5
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
	
	return $w.main.info
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
	
	return $retVal
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
# INVERT_COLOR
# Invert a color in Tk format
#************************************************

proc invert_color { color } {
	global colorsTheme

	if [ catch { set rgbColor [ winfo rgb . $color ] } ] {
		return $colorsTheme(fg)
	}
	
    set rgbInvert [ list [ expr 65535 - [ lindex $rgbColor 0 ] ] \
						 [ expr 65535 - [ lindex $rgbColor 1 ] ] \
						 [ expr 65535 - [ lindex $rgbColor 2 ] ] ]
						 
    return [ format "#%04x%04x%04x" {*}$rgbInvert ]
}


#************************************************
# CHECK_SYS_OPT
# Check (best guess) if system option configuration is valid for the platform
#************************************************
set winYes [ list "86" "WRC" "windres" "-ltcl" "-ltk" "-lz" "-mthreads" "-mwindows" ]
set winNo  [ list "-framework" "-lpthread" ]
set linuxYes [ list "8.6" "-ltcl" "-ltk" "-lz" "-lpthread" ]
set linuxNo  [ list "WRC" "windres" "-framework" "-mthreads" "-mwindows" ]
set macYes [ list "-framework" "-lz" "-lpthread" ]
set macNo  [ list "86" "8.6" "WRC" "windres" "-mthreads" "-mwindows" ]

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
	.f.hea.cur.col.col2 configure -text [ expr 1 + [ lindex [ split [ .f.t.t index insert ] . ] 1 ] ]
}


#************************************************
# PLOG
# Tcl/Tk version of C "plog" function to 
# show a string in the LSD Log window
#************************************************
proc plog cm {
	.log.text.text.internal insert end $cm
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
# COMP_UND
# Create special sort procedure to keep names 
# starting with underline at the end
#************************************************
proc comp_und { n1 n2 } {

	if [ string equal $n1 $n2 ] {
		return 0
	}
	
	if { ! [ expr { [ string index $n1 0 ] == "_" && [ string index $n2 0 ] == "_" } ] } {
		if { [ string index $n1 0 ] == "_" } {
			return 1
		} else
		if { [ string index $n2 0 ] == "_" } {
			return -1
		}
	}
	
	set listn [ lsort -dictionary [ list $n1 $n2 ] ]
	
	if [ string equal [ lindex $listn 0 ] $n1 ] {
		return -1
	} {
		return 1
	}
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
