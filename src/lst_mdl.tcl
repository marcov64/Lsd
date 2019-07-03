#*************************************************************
#
#	LSD 7.2 - July 2019
#	written by Marco Valente, Universita' dell'Aquila
#	and by Marcelo Pereira, University of Campinas
#
#	Copyright Marco Valente and Marcelo Pereira
#	LSD is distributed under the GNU General Public License
#	
#*************************************************************

#*************************************************************
# LST_MDL.TCL
# Collection of file handling scripts.
#*************************************************************

#************************************************
# LST_MDL
# List models returning the list a exploring directory b
#************************************************
proc lst_mdl { } {
	global lmod ldir lgroup cgroup

	if [ file exists modelinfo.txt ] {
		lappend ldir [ pwd ]
		set f [ open modelinfo.txt r ]
		set info [ gets $f ]
		close $f
		lappend lmod "$info"
		lappend lgroup $cgroup 
	}

	set dirs [ glob -nocomplain -type d * ]

	foreach i $dirs {
		set flag 0
		if [ file isdirectory $i ] {
			cd $i
			if [ file exists groupinfo.txt ] {
				set f [ open groupinfo.txt r ]
				set info [ gets $f ]
				close $f
				if { $cgroup != "." } {
					set cgroup [ file join "$cgroup" "$info" ] 
				} else {
					set cgroup "$info" 
				}
				set flag 1
			}
			 
			lst_mdl
			if { $flag == 1 } {
				set cgroup [ file dirname $cgroup ] 
			}
			cd ..
		} 
	} 
}


#************************************************
# CHS_MDL
#************************************************
proc chs_mdl { } {
	global lmod ldir sd sf d1 d2 f1 f2 lgroup cgroup butWid

	set lmod ""
	set ldir ""
	set sd ""
	set sf ""
	set d1 ""
	set d2 ""
	set f1 ""
	set f2 ""
	set lgroup ""
	set cgroup ""
	set glabel ""

	unset lmod
	unset ldir
	unset sd
	unset sf

	lst_mdl

	newtop .l "LSD Models" { set choice -1; destroytop .l }

	frame .l.l -relief groove -bd 2
	
	label .l.l.tit -text "List of models" -fg red
	scrollbar .l.l.vs -command ".l.l.l yview"
	listbox .l.l.l -height 30 -width 70 -yscroll ".l.l.vs set" -selectmode browse
	mouse_wheel .l.l.l
	
	bind .l.l.l <ButtonRelease> { set glabel [ lindex $lgroup [ .l.l.l curselection ] ]; .l.l.gl configu -text "$glabel" }
	bind .l.l.l <KeyRelease-Up> { set glabel [ lindex $lgroup [ .l.l.l curselection ] ]; .l.l.gl configu -text "$glabel" }
	bind .l.l.l <KeyRelease-Down> { set glabel [ lindex $lgroup [ .l.l.l curselection ] ]; .l.l.gl configu -text "$glabel" }
	
	label .l.l.gt -text "Selected model contained in group:" 
	label .l.l.gl -text "$glabel" 

	frame .l.t -relief groove -bd 2
	frame .l.t.f1 
	label .l.t.f1.tit -text "Selected models" -foreground red
	frame .l.t.f1.m1 -relief groove -bd 2
	label .l.t.f1.m1.l -text "First model"
	entry .l.t.f1.m1.d -width 50 -textvariable d1 -justify center
	entry .l.t.f1.m1.f -width 20 -textvariable f1 -justify center
	
	bind .l.t.f1.m1.f <3> { set tmp [ tk_getOpenFile -parent .l -title "Load LSD File" -initialdir "$d1" ]; if { $tmp != "" && ! [ fn_spaces "$tmp" .l ] } {set f1 [ file tail $tmp ] } }
	
	button .l.t.f1.m1.i -width $butWid -text Insert -command { slct; if { [ info exists sd ] } { set d1 "$sd"; set f1 "$sf" } }
	
	pack .l.t.f1.m1.l -anchor nw
	pack .l.t.f1.m1.d .l.t.f1.m1.f -expand yes -fill x
	
	pack .l.t.f1.m1.i -padx 10 -pady 10 -anchor n

	frame .l.t.f1.m2 -relief groove -bd 2
	label .l.t.f1.m2.l -text "Second model"
	entry .l.t.f1.m2.d -width 50 -textvariable d2 -justify center
	entry .l.t.f1.m2.f -width 20 -textvariable f2 -justify center
	
	bind .l.t.f1.m2.f <3> { set tmp [ tk_getOpenFile -parent .l -title "Load LSD File" -initialdir "$d2" ]; if { $tmp != "" && ! [ fn_spaces "$tmp" .l ] } {set f2 [file tail $tmp] } }
	
	button .l.t.f1.m2.i -width $butWid -text Insert -command {slct; if { [info exists sd]} {set d2 "$sd"; set f2 "$sf"} }
	pack .l.t.f1.m2.l -anchor nw
	pack .l.t.f1.m2.d .l.t.f1.m2.f -expand yes -fill x -anchor nw
	pack .l.t.f1.m2.i -padx 10 -pady 10 -anchor n

	pack .l.t.f1.tit .l.t.f1.m1 .l.t.f1.m2 -expand yes -fill x -anchor n
	pack .l.t.f1 -fill x -anchor n

	frame .l.t.b
	button .l.t.b.cmp -width $butWid -text Compare -command { destroytop .l; set choice 1 }
	button .l.t.b.cnc -width $butWid -text Cancel -command { destroytop .l; set d1 ""; set choice -1 }

	pack .l.t.b.cmp .l.t.b.cnc -padx 10 -pady 10 -side left
	pack .l.t.b -side bottom -anchor e

	pack .l.l.tit

	pack .l.l.vs -side right -fill y
	pack .l.l.l -expand yes -fill both 
	pack .l.l.gt .l.l.gl -side top -expand yes -fill x -anchor w

	pack .l.l .l.t -expand yes -fill both -side left

	set j 0
	foreach i $lmod {
		set k [ lindex $lgroup $j ]
		incr j
		.l.l.l insert end "$i" 
	}
	
	showtop .l centerS
}


#************************************************
# SLCT
#************************************************
proc slct { } {
	global sd sf ldir
	
	set tmp [ .l.l.l curselection ]
	if { $tmp == "" } {
		set sd ""
		set sf ""
		return 
	}

	set sd [ lindex $ldir $tmp ]
	set sf [ file tail [ glob -nocomplain [ file join $sd *.cpp ] ] ]
}


#************************************************
# CHECK_SYS_OPT
# Check (best guess) if system option configuration is valid for the platform
#************************************************
set win32Yes [ list ".exe" "85" "/gnu/" "g++" "-lz" "-mthreads" "-mwindows" ]
set win32No  [ list "/gnu64/" "-framework" "-lpthread" ]
set win64Yes [ list ".exe" "86" "/gnu64/" "x86_64-w64-mingw32-g++" "gnu++14" "-lz" "-mthreads" "-mwindows" ]
set win64No  [ list "/gnu/" "-framework" "-lpthread" ]
set linuxYes [ list "8.6" "g++" "-lz" "-lpthread" "gnu++14" ]
set linuxNo  [ list ".exe" "/gnu64/" "x86_64-w64-mingw32-g++" "-framework" "-mthreads" "-mwindows" ]
set osxYes [ list "g++" "-framework" "-lz" "-lpthread" "gnu++14" ]
set osxNo  [ list ".exe" "/gnu/" "/gnu64/" "x86_64-w64-mingw32-g++" "-mthreads" "-mwindows" ]
set macYes [ list "g++" "-framework" "-lz" "-lpthread" "-DMAC_PKG" "gnu++14" ]
set macNo  [ list ".exe" "/gnu/" "/gnu64/" "x86_64-w64-mingw32-g++" "-mthreads" "-mwindows" ]

proc check_sys_opt { } {
	global LsdSrc CurPlatform win32Yes win32No win64Yes win64No linuxYes linuxNo osxYes osxNo macYes macNo
	
	if { ! [ file exists "$LsdSrc/system_options.txt" ] } { 
		return "File 'system_options.txt' not found (click 'Default' button to recreate it)"
	}
	
	set f [ open "$LsdSrc/system_options.txt" r ]
	set options [ read -nonewline $f ]
	close $f
	
	switch $CurPlatform {
		win32 {
			set yes $win32Yes
			set no $win32No
		}
		win64 {
			set yes $win64Yes
			set no $win64No
		}
		linux {
			set yes $linuxYes
			set no $linuxNo
		}
		osx {
			set yes $osxYes
			set no $osxNo
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
# MAKE_WAIT
# Waits for a makefile background task to finish
# Set 'res' to 1 if compilation succeeds and 0 otherwise
#************************************************
proc make_wait { } {
	global targetExe iniTime makePipe exeTime res
	
	if { [ eof $makePipe ] } {
		fileevent $makePipe readable ""

		if [ file exists make.bat ] {
			file delete make.bat
		}
		
		# check if the executable is newer than the compilation command, implying just warnings
		if [ file exist "$targetExe" ] { 
			set exeTime [ file mtime "$targetExe" ] 
		} else { 
			set exeTime 0 
		};

		if { [ file exists makemessage.txt ] && [ file size makemessage.txt ] == 0 } {
			file delete makemessage.txt
			set res 1
		} elseif { $iniTime <= $exeTime } { 
			set res 1 
		} else {
			set res 0
		}
		
		return
	}
	
	set data [ read $makePipe ]
}


#************************************************
# MAKE_BACKGROUND
# Start a makefile as a background task
#************************************************
proc make_background { target threads nw macPkg } {
	global tcl_platform MakeExe RootLsd LsdGnu targetExe iniTime makePipe
	
	if { $nw } {
		set makeSuffix "NW"
	} else {
		set makeSuffix ""
	};
	
	if [ string equal $tcl_platform(platform) windows ] {
		set exeSuffix ".exe"
	} else {
		set exeSuffix ""
	}
	
	if { ! $nw && $macPkg && [ string equal $tcl_platform(os) Darwin ] } {
		set targetExe "$target.app/Contents/MacOS/$target"
	} else {
		set targetExe "$target$exeSuffix"
	};
	
	set iniTime [ clock seconds ]
	
	# handle Windows access to open executable and empty compilation windows
	if [ string equal $tcl_platform(platform) windows ] {
	
		if [ file exists "$target$exeSuffix" ] {
			if [ string equal [ info tclversion ] 8.6 ] {
				close [ file tempfile targetTemp ]
				file delete $targetTemp
				set targetDir [ file dirname $targetTemp ]
				file mkdir "$targetDir"
				set targetTemp "$targetDir/$target.bak"
			} else {
				set targetTemp "$target.bak"
			}
			
			file rename -force "$target$exeSuffix" "$targetTemp"
			file copy -force "$targetTemp" "$target$exeSuffix"
		}
		
		set file [ open make.bat w ]
		puts -nonewline $file "$MakeExe -j $threads -f makefile$makeSuffix 2> makemessage.txt\n"
		close $file

		set makePipe [ open "| make.bat" r ]
	} else {
		set makePipe [ open "| $MakeExe -j $threads -f makefile$makeSuffix 2> makemessage.txt" r ]
	}
	
	fconfigure $makePipe -blocking 0
	fileevent $makePipe readable make_wait
}


#************************************************
# GET_SOURCE_FILES
# Get the list of source files, including the main and extra files
#************************************************
proc get_source_files { path } {

	if { ! [ file exists "$path/model_options.txt" ] } { 
		return [ list ]
	}

	set f [ open "$path/model_options.txt" r ]
	set options [ read -nonewline $f ]
	close $f

	set ini [ expr [ string first "FUN=" "$options" ] + 4 ]
	set end [ expr $ini + [ string first "\n" [ string range "$options" $ini end ] ] - 1 ]
	set files [ list "[ string trim [ lindex [ split [ string range "$options" $ini $end ] ] 0 ] ].cpp" ]
	
	if { [ llength $files ] != 1 } {
		return [ list ]
	}
	
	set ini [ expr [ string first "FUN_EXTRA=" "$options" ] + 10 ]
	if { $ini != -1 } {
		set end [ expr $ini + [ string first "\n" [ string range "$options" $ini end ] ] - 1 ]
		set extra [ string trim [ string range "$options" $ini $end ] ]
		regsub -all { +} $extra { } extra
		set extra [ split $extra " \t" ]
		
		foreach x $extra { 
			if { [ file exists $x ] || [ file exists "$path/$x" ] } {
				lappend files $x
			}
		}
	}

	return $files
}


#************************************************
# CREATE_ELEM_FILE
# Produce the element list file (elements.txt) to be used in LSD browser
#************************************************
# list of commands to search for parameters (X=number of macro arguments, Y=position of parameter)
set cmds_1_1 [ list V SUM MAX MIN AVE SD STAT RECALC ]
set cmds_2_1 [ list WHTAVE SEARCH_CND WRITE INCR MULT ]
set cmds_2_2 [ list VS SUMS MAXS MINS AVES WHTAVE SDS STATS RNDDRAW RECALCS ]
set cmds_3_1 [ list WRITEL ]
set cmds_3_2 [ list WHTAVES SEARCH_CNDS RNDDRAW_TOT WRITES INCRS MULTS SORT ]
set cmds_3_3 [ list WHTAVES RNDDRAWS ]
set cmds_4_2 [ list WRITELS SORT2 ]
set cmds_4_3 [ list RNDDRAW_TOTS SORTS SORT2 ]
set cmds_5_3 [ list SORT2S ]
set cmds_5_4 [ list SORT2S ]

proc create_elem_file { path } {
	global exeTime cmds_1_1 cmds_2_1 cmds_2_2 cmds_3_1 cmds_3_2 cmds_3_3 cmds_4_2 cmds_4_3 cmds_5_3 cmds_5_4
	
	# don't recreate if executable file was not changed
	if { [ file exists "$path/elements.txt" ] && [ info exists exeTime ] } {
		if { [ file mtime "$path/elements.txt" ] >= $exeTime } {
			return
		}
	} 
	
	set files [ get_source_files $path ]
	
	set nFiles [ llength $files ]
	if { $nFiles == 0 } {
		return
	}
	
	set vars [ list ]
	set pars [ list ]
	
	foreach fname $files {
		if { ! [ file exists "$path/$fname" ] } {
			continue
		}
		
		set f [ open "$path/$fname" r ]
		set text [ read -nonewline $f ]
		close $f

		set eqs [ regexp -all -inline -- {EQUATION[ \t]*\([ \t]*\"(\w+)\"[ \t]*\)} $text ]
		foreach { eq var } $eqs {
			lappend vars $var
		}
		
		set eqs [ regexp -all -inline -- {EQUATION_DUMMY[ \t]*\([ \t]*\"(\w+)\"[ \t]*,[ \t]*\"\w+\"[ \t]*\)} $text ]
		foreach { eq var } $eqs {
			lappend vars $var
		}
		
		foreach cmd $cmds_1_1 {
			set calls [ regexp -all -inline -- [ subst -nocommands -nobackslashes {$cmd[ \t]*\([ \t]*\"(\w+)\"[ \t]*\)} ] $text ]
			foreach { call par } $calls {
				lappend pars $par
			}
		}
		foreach cmd $cmds_2_1 {
			set calls [ regexp -all -inline -- [ subst -nocommands -nobackslashes {$cmd[ \t]*\([ \t]*\"(\w+)\"[ \t]*,[^;]+\)} ] $text ]
			foreach { call par } $calls {
				lappend pars $par
			}
		}
		foreach cmd $cmds_2_2 {
			set calls [ regexp -all -inline -- [ subst -nocommands -nobackslashes {$cmd[ \t]*\([^;]+,[ \t]*\"(\w+)\"[ \t]*\)} ] $text ]
			foreach { call par } $calls {
				lappend pars $par
			}
		}
		foreach cmd $cmds_3_1 {
			set calls [ regexp -all -inline -- [ subst -nocommands -nobackslashes {$cmd[ \t]*\([ \t]*\"(\w+)\"[ \t]*,[^;]+,[^;]+\)} ] $text ]
			foreach { call par } $calls {
				lappend pars $par
			}
		}
		foreach cmd $cmds_3_2 {
			set calls [ regexp -all -inline -- [ subst -nocommands -nobackslashes {$cmd[ \t]*\([^;]+,[ \t]*\"(\w+)\"[ \t]*,[^;]+\)} ] $text ]
			foreach { call par } $calls {
				lappend pars $par
			}
		}
		foreach cmd $cmds_3_3 {
			set calls [ regexp -all -inline -- [ subst -nocommands -nobackslashes {$cmd[ \t]*\([^;]+,[^;]+,[ \t]*\"(\w+)\"[ \t]*\)} ] $text ]
			foreach { call par } $calls {
				lappend pars $par
			}
		}
		foreach cmd $cmds_4_2 {
			set calls [ regexp -all -inline -- [ subst -nocommands -nobackslashes {$cmd[ \t]*\([^;]+,[ \t]*\"(\w+)\"[ \t]*,[^;]+,[^;]+\)} ] $text ]
			foreach { call par } $calls {
				lappend pars $par
			}
		}
		foreach cmd $cmds_4_3 {
			set calls [ regexp -all -inline -- [ subst -nocommands -nobackslashes {$cmd[ \t]*\([^;]+,[^;]+,[ \t]*\"(\w+)\"[ \t]*,[^;]+\)} ] $text ]
			foreach { call par } $calls {
				lappend pars $par
			}
		}
		foreach cmd $cmds_5_3 {
			set calls [ regexp -all -inline -- [ subst -nocommands -nobackslashes {$cmd[ \t]*\([^;]+,[^;]+,[ \t]*\"(\w+)\"[ \t]*,[^;]+,[^;]+\)} ] $text ]
			foreach { call par } $calls {
				lappend pars $par
			}
		}
		foreach cmd $cmds_5_4 {
			set calls [ regexp -all -inline -- [ subst -nocommands -nobackslashes {$cmd[ \t]*\([^;]+,[^;]+,[^;]+,[ \t]*\"(\w+)\"[ \t]*,[^;]+\)} ] $text ]
			foreach { call par } $calls {
				lappend pars $par
			}
		}
	}
	
	set vars [ lsort -dictionary -unique $vars ]
	set pars [ remove_elem $pars $vars ]
	set pars [ lsort -dictionary $pars ]
	
	set f [ open "$path/elements.txt" w ]
	puts -nonewline $f "$vars\n$pars"
	close $f
}


#************************************************
# READ_ELEM_FILE
# Read the element list file (elements.txt) from dist to use in LSD browser
#************************************************
# Lists to hold the elements in model program and the ones missing in model
set progVar [ list ]
set progPar [ list ]
set missVar [ list ]
set missPar [ list ]

proc read_elem_file { path } {
	global progVar progPar

	if { ! [ file exists "$path/elements.txt" ] } { 
		set progVar [ list ]
		set progPar [ list ]
		return
	}

	set f [ open "$path/elements.txt" r ]
	set progVar [ split [ gets $f ] ]
	set progPar [ split [ gets $f ] ]	
	close $f
	
	upd_miss_elem
}


#************************************************
# UPD_MISS_ELEM
# Update the missing elements list, considering the elements already in the model structure
#************************************************
proc upd_miss_elem { } {
	global modElem progVar progPar missPar missVar
	
	if [ info exists modElem ] {
		set missVar [ lsort -dictionary [ remove_elem $progVar $modElem ] ]
		set missPar [ lsort -dictionary [ remove_elem $progPar $modElem ] ]
	} else {
		set missVar $progVar
		set missPar $progPar
	}
}


#************************************************
# REMOVE_ELEM
# Remove duplicated and elements contained in toRemove from origList and sort it
#************************************************
proc remove_elem { origList toRemove } {

	set origList [ lsort -unique $origList ]
	
	foreach elem $toRemove {
		set idx [ lsearch -sorted -exact $origList $elem ]
		if { $idx >= 0 } {
			set origList [ lreplace $origList $idx $idx ]
		}
	}
	
	return $origList
}
	