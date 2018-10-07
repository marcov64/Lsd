#*************************************************************
#
#	LSD 7.1 - May 2018
#	written by Marco Valente, Universita' dell'Aquila
#	and by Marcelo Pereira, University of Campinas
#
#	Copyright Marco Valente
#	LSD is distributed under the GNU General Public License
#	
#*************************************************************

# list models returning the list a exploring directory b

proc lst_mdl { } {
	global lmod ldir lgroup cgroup

	if { [file exists modelinfo.txt]==1 } {

	lappend ldir [pwd]
	set f [open modelinfo.txt r]
	set info [gets $f]
	close $f
	lappend lmod "$info"
	lappend lgroup $cgroup }

	set dirs [glob -nocomplain -type d *]

	foreach i $dirs {
		set flag 0
		if { [file isdirectory $i]==1 } {
			cd $i
			if { [file exists groupinfo.txt]==1 } {
				set f [open groupinfo.txt r]
				set info [gets $f]
				close $f
				if {$cgroup != "."} {set cgroup [file join "$cgroup" "$info"] } {set cgroup "$info" }
				set flag 1
			}
			 
			lst_mdl
			if { $flag == 1} {
				set cgroup [file dirname $cgroup] 
			}
			cd ..
		} 
	} 
}

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
	bind .l.l.l <ButtonRelease> {set glabel [lindex $lgroup [.l.l.l curselection]]; .l.l.gl configu -text "$glabel"}
	bind .l.l.l <KeyRelease-Up> {set glabel [lindex $lgroup [.l.l.l curselection]]; .l.l.gl configu -text "$glabel"}
	bind .l.l.l <KeyRelease-Down> {set glabel [lindex $lgroup [.l.l.l curselection]]; .l.l.gl configu -text "$glabel"}
	label .l.l.gt -text "Selected model contained in group:" 
	label .l.l.gl -text "$glabel" 

	frame .l.t -relief groove -bd 2
	frame .l.t.f1 
	label .l.t.f1.tit -text "Selected models" -foreground red
	frame .l.t.f1.m1 -relief groove -bd 2
	label .l.t.f1.m1.l -text "First model"
	entry .l.t.f1.m1.d -width 50 -textvariable d1 -justify center
	entry .l.t.f1.m1.f -width 20 -textvariable f1 -justify center
	bind .l.t.f1.m1.f <3> { set tmp [ tk_getOpenFile -parent .l -title "Load LSD File" -initialdir "$d1" ]; if { $tmp != "" && ! [ fn_spaces "$tmp" .l ] } {set f1 [file tail $tmp] } }
	button .l.t.f1.m1.i -width $butWid -text Insert -command {slct; if { [info exists sd]} {set d1 "$sd"; set f1 "$sf"} }
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
	button .l.t.b.cmp -width $butWid -text Compare -command {destroytop .l; set choice 1}
	button .l.t.b.cnc -width $butWid -text Cancel -command {destroytop .l; set d1 ""; set choice -1}

	pack .l.t.b.cmp .l.t.b.cnc -padx 10 -pady 10 -side left
	pack .l.t.b -side bottom -anchor e

	pack .l.l.tit

	pack .l.l.vs -side right -fill y
	pack .l.l.l -expand yes -fill both 
	pack .l.l.gt .l.l.gl -side top -expand yes -fill x -anchor w

	pack .l.l .l.t -expand yes -fill both -side left

	set j 0
	foreach i $lmod {
		set k [lindex $lgroup $j]
		incr j
		.l.l.l insert end "$i" 
	}
	showtop .l centerS
}

proc slct { } {
	global sd sf ldir
	set tmp [.l.l.l curselection]
	if { $tmp=="" } {
		set sd ""
		set sf ""
		return 
	}

	set sd [lindex $ldir $tmp]
	set sf [file tail [glob -nocomplain [file join $sd *.cpp]]]
}


# check (best guess) if system option configuration is valid for the platform

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
	
	set f [ open $LsdSrc/system_options.txt r ]
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
	
#	if { $noCount > 0 || $yesCount < [ llength $yes ] } {
#		tk_messageBox -message "Missing configuration items: $missingItems\nInvalid configuration items: $invalidItems"
#	}
	
	if { $noCount > 0 } {
		return "Invalid option(s) detected (click 'Default' button to recreate options)"
	}
	
	if { $yesCount < [ llength $yes ] } {
		return "Missing option(s) detected (click 'Default' button to recreate options)"	
	}
	
	return ""
}