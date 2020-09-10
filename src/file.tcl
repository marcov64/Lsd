#*************************************************************
#
#	LSD 8.0 - December 2020
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
# FILE.TCL
# Collection of procedures to manage external files.
#*************************************************************

#************************************************
# LSDEXIT
# Remove existing LSD temporary files
#************************************************
proc LsdExit { } {
	global RootLsd
	if { [ file exists $RootLsd/Manual/temp.html ] } {
		file delete -force $RootLsd/Manual/temp.html
	}

	if { [ file exists temp.html ] } {
		file delete -force temp.html
	}
}


#************************************************
# LSDHELP
#************************************************
proc LsdHelp a {
	global HtmlBrowser CurPlatform RootLsd
	set here [ pwd ]
	set f [ open $RootLsd/Manual/temp.html w ]
	puts $f "<meta http-equiv=\"Refresh\" content=\"0;url=$a\">"
	close $f
	set b "[ file nativename $RootLsd/Manual/temp.html ]"
	if { ! [ string equal $CurPlatform windows ] } {
		exec $HtmlBrowser $b &
	} {
		exec cmd.exe /c start $b &
	}
}


#************************************************
# LSDHTML
#************************************************
proc LsdHtml a {
	global HtmlBrowser CurPlatform
	set f [ open temp.html w ]
	puts $f "<meta http-equiv=\"Refresh\" content=\"0;url=$a\">"
	close $f
	set b "temp.html"
	if { ! [ string equal $CurPlatform windows ] } {
		exec $HtmlBrowser $b &
	} {
		exec cmd.exe /c start $b &
	}
}


#************************************************
# FN_SPACES
# Checks is a filename has spaces
# Set 'mult' to one if multiple file names are allowed
#************************************************
proc fn_spaces { fn { par . } { mult 0 } } {
	if $mult {
		set count [ llength $fn ]
	} else {
		set count 1
	}

	for { set i 0 } { $i < $count } { incr i } {
		if $mult {
			set file "[ lindex $fn $i ]"
		} else {
			set file "$fn"
		}
		if { [ string first " " "$file" ] != -1 } {
			ttk::messageBox -parent $par -type ok -title Error -icon error -message "Invalid file name or path" -detail "Invalid file name/path:\n\n'$fn'\n\nLSD files must have no spaces in the file names nor in their directory path. Please rename the file and/or move it to a different directory."
			return true
		} else {
			return false
		}
	}
	return false
}


#************************************************
# SED
# Partial implementation of GNU sed in Tcl
# Code by Emmanuel Frecon (https://wiki.tcl-lang.org/page/sed)
# Extra option "e" (as in extract) and that will 
# extract a particular (sub)group, or the text of 
# the regular expression when no particular index 
# is provided.
#************************************************
proc sed { script input } {
	set sep [ string index $script 1 ]

	foreach { cmd from to flag } [ split $script $sep ] break

	switch -- $cmd {
		"s" {
				set cmd regsub
				if { [ string first "g" $flag ] >= 0 } {
                	lappend cmd -all
            	}

				if { [ string first "i" [ string tolower $flag ] ] >= 0 } {
					lappend cmd -nocase
				}

				set idx [ regsub -all -- {[a-zA-Z]} $flag "" ]
				if { [ string is integer -strict $idx ] } {
					set cmd [ lreplace $cmd 0 0 regexp ]
					lappend cmd -inline -indices -all -- $from $input
					set res [ eval $cmd ]
					set which [ lindex $res $idx ]
					return [ string replace $input [ lindex $which 0 ] [ lindex $which 1 ] $to ]
				}

				# most generic case
				lappend cmd -- $from $input $to
				return [ eval $cmd ]
			}

		"e" {
				set cmd regexp
				if { $to == "" } { 
					set to 0 
				}

				if { ! [ string is integer -strict $to ] } {
					return -code error "No proper group identifier specified for extraction"
            	}

				lappend cmd -inline -- $from $input
				return [ lindex [ eval $cmd ] $to ]
			}

		"y" {
				return [ string map [ list $from $to ] $input ]
			}
	}

	return -code error "Option not yet implemented"
}


#*************************************************************
# FINDFILES
# Tcl procedure to list all files with a given pattern
# within a directory subtree.
# Based on code by Joseph Bui (https://stackoverflow.com/a/448573).
# Arguments:
# - directory - the top directory to start looking in
# - pattern - A pattern, as defined by the glob command, 
#	that the files must match
# - tails - option to return just the tail part of path
#  	from the directory chosen
#*************************************************************
proc findfiles { directory pattern { tails "" } } {

	if { $tails != "" } {
		set tails 1
	}
	
    # fix the directory name, this ensures the directory name is in the
    # native format for the platform and contains a final directory separator
    set directory [ string trimright [ file join [ file normalize $directory ] { } ] ]

    # starting with the passed in directory, do a breadth first search for
    # subdirectories. Avoid cycles by normalizing all file paths and checking
    # for duplicates at each level.
    set directories [ list ]
    lappend parents $directory
	
    while { [ llength $parents ] > 0 } {

        # find all the children at the current level
        set children [ list ]
        foreach parent $parents {
            set children [ concat $children [ glob -nocomplain -types { d r } -path $parent * ] ]
		}

        # normalize the children
        set length [ llength $children ]
        for { set i 0 } { $i < $length } { incr i } {
            lset children $i [ string trimright [ file join [ file normalize [ lindex $children $i ] ] { } ] ]
        }

        # make the list of children unique
        set children [ lsort -unique $children ]

        # find the children that are not duplicates, use them for the next level
        set parents [ list ]
        foreach child $children {
            if { [ lsearch -sorted $directories $child ] == -1 } {
                lappend parents $child
            }
        }

        # append the next level directories to the complete list
        set directories [ lsort -unique [ concat $directories $parents ] ]
    }

	lappend directories $directory
	
    # get all the files in the passed in directory and all its subdirectories
    set result [ list ]
	set basLgt [ string length $directory ]
	
    foreach dir $directories {
		set this [ glob -nocomplain -types { f r } -path $dir -- $pattern ]
		if { $tails } {
			set that [ list ]
			
			foreach fil $this {
				if [ string equal $directory "[ string range $fil 0 [ expr $basLgt - 1 ] ]" ] {
					lappend that "[ string range $fil $basLgt end ]"
				} else {
					error "Internal error in 'findfiles'"
				}
			}
			
			set this $that
		}
		
        set result [ concat $result $this ]
    }

    # normalize the filenames
 	if { ! $tails } {
		set length [ llength $result ]
		for { set i 0 } { $i < $length } { incr i } {
			lset result $i [ file normalize [ lindex $result $i ] ]
		}
	}

    # return only unique filenames
    return [ lsort -unique $result ]
}


#************************************************
# CHOOSE_MODELS
# Chose models to compare dialog
#************************************************
proc choose_models { curdir curfile } {
	global d1 d2 f1 f2 lmod ldir lgroup cgroup butWid butPad butSpc darkTheme

	set d1 "$curdir"
	set d2 ""
	set f1 "$curfile"
	set f2 ""
	set lmod ""
	set ldir ""
	set lgroup ""
	set cgroup ""
	
	# recursive search of models and ordering
	list_models
	foreach mod $lmod dir $ldir group $lgroup {
		set moddir($mod) $dir
		set modgroup($mod) $group
	}
	set lmod [ lsort -dictionary $lmod ]
	unset ldir lgroup
	foreach mod $lmod {
		lappend ldir $moddir($mod)
		lappend lgroup $modgroup($mod)
	}

	newtop .l "Compare LSD Models" { set choice -1; destroytop .l }

	ttk::frame .l.t
	
	# 1st column
	ttk::frame .l.t.l

	ttk::label .l.t.l.tit -text "List of models"

	ttk::frame .l.t.l.l
	ttk::scrollbar .l.t.l.l.vs -command ".l.t.l.l.l yview"
	ttk::listbox .l.t.l.l.l -height 13 -width 45 -yscroll ".l.t.l.l.vs set" -dark $darkTheme
	mouse_wheel .l.t.l.l.l
	pack .l.t.l.l.vs -side right -fill y
	pack .l.t.l.l.l -expand yes -fill both

	foreach mod $lmod {
		.l.t.l.l.l insert end "$mod"
	}

	bind .l.t.l.l.l <Double-Button-1> { 
		.l.t.l.gt.t configure -text [ lindex $lgroup [ .l.t.l.l.l curselection ] ]
		select_model .l.t.l.l.l 0
	}
	bind .l.t.l.l.l <Return> {
		if { $d1 != "" && $f1 != "" && [ file exists "$d1/$f1" ] && \
			 $d2 != "" && $f2 != "" && [ file exists "$d2/$f2" ] } {
			.l.b.cmp invoke
		} else {
			select_model .l.t.l.l.l 0
		}
		break
	}
	bind .l.t.l.l.l <KeyRelease> {
		set key %K
		if { [ string length $key ] == 1 && [ string is alpha -strict $key ] } {
			set start [ expr [ .l.t.l.l.l curselection ] + 1 ]
			set first [ lsearch -start $start -nocase $lmod "${key}*" ]
			if { $first == -1 } {
				set first [ lsearch -start 0 -nocase $lmod "${key}*" ]
			}
			if { $first >= 0 } {
				selectinlist .l.t.l.l.l $first
			} 
		}
		.l.t.l.gt.t configure -text [ lindex $lgroup [ .l.t.l.l.l curselection ] ]
	}
	bind .l.t.l.l.l <Up> { .l.t.l.gt.t configure -text [ lindex $lgroup [ .l.t.l.l.l curselection ] ] }
	bind .l.t.l.l.l <Down> { .l.t.l.gt.t configure -text [ lindex $lgroup [ .l.t.l.l.l curselection ] ] }

	ttk::frame .l.t.l.gt
	ttk::label .l.t.l.gt.l -text "Selected model in group"
	ttk::label .l.t.l.gt.t -style hl.TLabel
	pack .l.t.l.gt.l .l.t.l.gt.t

	pack .l.t.l.tit .l.t.l.l .l.t.l.gt -pady 5

	# 2nd column
	ttk::frame .l.t.t

	ttk::label .l.t.t.tit -text "Selected models"

	ttk::frame .l.t.t.f1
	ttk::label .l.t.t.f1.l -text "First model"
	
	ttk::frame .l.t.t.f1.m1
	ttk::entry .l.t.t.f1.m1.d -width 40 -textvariable d1 -justify center
	ttk::entry .l.t.t.f1.m1.f -width 40 -textvariable f1 -justify center

	ttk::frame .l.t.t.f1.m1.i
	ttk::button .l.t.t.f1.m1.i.ins -width $butWid -text Select -command { select_model .l.t.l.l.l 1 }
	ttk::button .l.t.t.f1.m1.i.brw -width $butWid -text "Browse" -command { browse_model 1 }
	pack .l.t.t.f1.m1.i.ins .l.t.t.f1.m1.i.brw -padx $butSpc -pady $butPad -side left

	pack .l.t.t.f1.m1.d .l.t.t.f1.m1.f .l.t.t.f1.m1.i 
	pack .l.t.t.f1.l .l.t.t.f1.m1 -pady 3

	ttk::frame .l.t.t.f2
	ttk::label .l.t.t.f2.l -text "Second model"
	
	ttk::frame .l.t.t.f2.m2
	ttk::entry .l.t.t.f2.m2.d -width 40 -textvariable d2 -justify center
	ttk::entry .l.t.t.f2.m2.f -width 40 -textvariable f2 -justify center

	ttk::frame .l.t.t.f2.m2.i
	ttk::button .l.t.t.f2.m2.i.ins -width $butWid -text Select -command { select_model .l.t.l.l.l 2 }
	ttk::button .l.t.t.f2.m2.i.brw -width $butWid -text "Browse" -command { browse_model 2 }
	pack .l.t.t.f2.m2.i.ins .l.t.t.f2.m2.i.brw -padx $butSpc -pady $butPad -side left

	pack .l.t.t.f2.m2.d .l.t.t.f2.m2.f .l.t.t.f2.m2.i
	pack .l.t.t.f2.l .l.t.t.f2.m2 -pady 3

	pack .l.t.t.tit .l.t.t.f1 .l.t.t.f2 -pady 5
	
	pack .l.t.l .l.t.t -padx 5 -side left
	
	pack .l.t

	ttk::frame .l.b
	ttk::button .l.b.cmp -width $butWid -text Compare -command {
		if { $d1 != "" && $f1 != "" && [ file exists "$d1/$f1" ] && \
			 $d2 != "" && $f2 != "" && [ file exists "$d2/$f2" ] } {
			destroytop .l
			set choice 1
		} else {
			ttk::messageBox -parent .l -type ok -icon error -title Error -message "Model selection incomplete" -detail "Please select two models before comparing."
		}
	}
	ttk::button .l.b.cnc -width $butWid -text Cancel -command { 
		destroytop .l
		set d1 ""
		set f1 ""
		set d2 ""
		set f2 ""
		set choice -1 
	}
	bind .l <KeyPress-Escape> { .l.b.cnc invoke }
	bind .l <KeyPress-Return> { .l.b.cmp invoke }
	pack .l.b.cmp .l.b.cnc -padx $butSpc -side left
	pack .l.b -padx $butPad -pady $butPad -side right

	showtop .l
	pack propagate .l.t.l 0
	focus .l.t.l.l.l
	.l.t.l.l.l selection set 0
	.l.t.l.gt.t configure -text [ lindex $lgroup [ .l.t.l.l.l curselection ] ]
}


#************************************************
# LIST_MODELS
# List models returning the list a exploring directory b
# Support procedure to choose_models
#************************************************
proc list_models { } {
	global lmod ldir lgroup cgroup GROUP_INFO MODEL_INFO

	if [ file exists $MODEL_INFO ] {
		lappend ldir [ pwd ]
		set f [ open $MODEL_INFO r ]
		set mod [ gets $f ]
		set ver [ gets $f ]
		close $f
		if { $ver != "" } {
			set mod "$mod (v. $ver)"
		}
		if { [ lsearch $lmod $mod ] >= 0 } {
			set mod "$mod                                                                                                           #[ expr int( rand( ) * 1000 ) ]"
		}
		lappend lmod "$mod"
		lappend lgroup $cgroup
	}

	set dirs [ glob -nocomplain -type d * ]

	foreach i $dirs {
		set flag 0
		if [ file isdirectory $i ] {
			cd $i
			if [ file exists $GROUP_INFO ] {
				set f [ open $GROUP_INFO r ]
				set group [ gets $f ]
				close $f
				if { $cgroup != "." } {
					set cgroup [ file join "$cgroup" "$group" ]
				} else {
					set cgroup "$group"
				}
				set flag 1
			}

			list_models
			if { $flag == 1 } {
				set cgroup [ file dirname $cgroup ]
			}

			cd ..
		}
	}
}


#************************************************
# SELECT_MODEL
# Select current model and insert it into a panel
# Support procedure to choose_models
#************************************************
proc select_model { w panel } {
	global d1 d2 f1 f2 ldir
	
	set posModel [ $w curselection ]
	if { $posModel != "" } {
		set sd [ lindex $ldir $posModel ]
		set sf [ file tail [ glob -nocomplain [ file join $sd *.cpp ] ] ]
		
		if { $sd != "" && $sf != "" && [ file exists "$sd/$sf" ] } {
			if { $panel == 2 || ( $panel != 1 && $d1 != "" && $f1 != "" && [ file exists "$d1/$f1" ] ) } {
				set d2 "$sd"
				set f2 "$sf"
			} else {
				set d1 "$sd"
				set f1 "$sf"
			}
		} else {
			ttk::messageBox -parent .l -type ok -icon error -title Error -message "Equation file missing" -detail "Check if the model equation file exists or create one if required."
		}
	}
}


#************************************************
# BROWSE_MODEL
# Browse a model on disk and insert it into a panel
# Support procedure to choose_models
#************************************************
proc browse_model { panel } {
	global d1 d2 f1 f2
	
	if { $panel == 1 } {
		set dir $d1
	} elseif { $panel == 2 } {
		set dir $d2
	} else {
		return
	}
	
	set filename [ tk_getOpenFile -parent .l -title "Load LSD Equation File" -initialdir "$dir" -filetypes { { {LSD equation files} {.cpp} } { {All files} {*} } } ]
	
	if { $filename != "" && ! [ fn_spaces "$filename" .l ] } {
		if { $panel == 1 } {
			set f1 [ file tail $filename ]
			set d1 [ file dirname $filename ]
		} elseif { $panel == 2 } {
			set f2 [ file tail $filename ]
			set d2 [ file dirname $filename ]
		}
	}
}


#************************************************
# OPEN_DIFF
#************************************************
proc open_diff { file1 file2 { file1name "" } { file2name "" } } {
	global CurPlatform wish sysTerm RootLsd LsdSrc diffApp diffAppType diffFile1name diffFile2name diffFile1 diffFile2 diffOptions

	set cmdline "$RootLsd/$LsdSrc/$diffApp $diffFile1 $file1 $diffFile2 $file2 $diffOptions $diffFile1name $file1name $diffFile2name $file2name"

	if { $diffAppType == 0 } {
		set cmdline [ concat $wish $cmdline ]
	} elseif { $diffAppType == 1 } {
		switch $CurPlatform {
			linux {
				set cmdline [ concat $sysTerm "-e" $cmdline ]
			}
			mac {
				set cmdline [ concat "osascript -e tell application \"$sysTerm\" to do script \"cd [ pwd ]; $cmdline; exit\"" $cmdline ]
			}
			windows {
				set cmdline [ concat $sysTerm "/c" $cmdline ]
			}
		}
	}

	set error [ catch { exec -- {*}$cmdline & } result ]

	if { $error } {
		ttk::messageBox -parent $par -type ok -icon error -title Error -message "Diff failed to launch" -detail "Diff returned error '$error'.\nDetail:\n$result\n\nPlease check if the diff appplication is set up properly and reinstall LSD if the problem persists."
	}

	return $error
}


#************************************************
# OPEN_GNUPLOT
# Open external gnuplot application
#************************************************
proc open_gnuplot { { script "" } { errmsg "" } { wait false } { par ".da" } } {
	global CurPlatform sysTerm gnuplotExe

	if [ string equal $script "" ] {
		set args ""
	} else {
		set args "-p $script"
	}

	switch $CurPlatform {
		mac {
			if { $wait } {
				set error [ catch { exec osascript -e "tell application \"$sysTerm\" to do script \"cd [ pwd ]; gnuplot $script; exit\"" } result ]
			} else {
				set error [ catch { exec osascript -e "tell application \"$sysTerm\" to do script \"cd [ pwd ]; gnuplot $args; exit\"" & } result ]
			}
		}
		linux {
			if { $wait } {
				set error [ catch { exec $sysTerm -e "gnuplot $script; exit" } result ]
			} else {
				set error [ catch { exec $sysTerm -e "gnuplot $args; exit" & } result ]
			}
		}
		windows {
			if [ string equal $script "" ] {
				set error [ catch { exec $gnuplotExe & } result ]
			} else {
				if { $wait } {
					set error [ catch { exec $gnuplotExe $script } result ]
				} else {
					set error [ catch { exec $gnuplotExe -p $script & } result ]
				}
			}
		}
	}

	if { $error != 0 } {
		if [ string equal $errmsg "" ] {
			set errmsg "Please check if Gnuplot is installed and set up properly."
		}

		ttk::messageBox -parent $par -type ok -icon error -title Error -message "Gnuplot failed to launch" -detail "Gnuplot returned error '$error'.\nDetail:\n$result\n\n$errmsg"
	}

	return $error
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
	global CurPlatform MakeExe RootLsd LsdGnu targetExe iniTime makePipe

	if { $nw } {
		set makeSuffix "NW"
	} else {
		set makeSuffix ""
	};

	if [ string equal $CurPlatform windows ] {
		set exeSuffix ".exe"
	} else {
		set exeSuffix ""
	}

	if { ! $nw && $macPkg && [ string equal $CurPlatform mac ] } {
		set targetExe "$target.app/Contents/MacOS/$target"
	} else {
		set targetExe "$target$exeSuffix"
	};

	set iniTime [ clock seconds ]

	# handle Windows access to open executable and empty compilation windows
	if [ string equal $CurPlatform windows ] {

		if [ file exists "$target$exeSuffix" ] {
			close [ file tempfile targetTemp ]
			file delete $targetTemp
			set targetDir [ file dirname $targetTemp ]
			file mkdir "$targetDir"
			set targetTemp "$targetDir/$target.bak"

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
	global MODEL_OPTIONS

	if { ! [ file exists "$path/$MODEL_OPTIONS" ] } {
		return [ list ]
	}

	set f [ open "$path/$MODEL_OPTIONS" r ]
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
set cmds_1_1 [ list V SUM MAX MIN AVE MED SD STAT RECALC LAST_CALC INIT_TSEARCH_CND ]
set cmds_2_1 [ list VL SUML MAXL MINL AVEL MEDL WHTAVE SDL SEARCH_CND TSEARCH_CND WRITE INCR MULT V_CHEAT ]
set cmds_2_2 [ list VS SUMS MAXS MINS AVES MEDS WHTAVE SDS STATS RNDDRAW RECALCS LAST_CALCS INIT_TSEARCH_CNDS ]
set cmds_3_1 [ list WRITEL SEARCH_CNDL V_CHEATL ]
set cmds_3_2 [ list VLS SUMLS MAXLS MINLS AVELS MEDLS WHTAVES SDLS SEARCH_CNDS TSEARCH_CNDS RNDDRAWL RNDDRAW_TOT WRITES INCRS MULTS SORT V_CHEATS ]
set cmds_3_3 [ list WHTAVES RNDDRAWS ]
set cmds_4_1 [ list WRITELL ]
set cmds_4_2 [ list WHTAVELS WRITELS SEARCH_CNDLS RNDDRAW_TOTL SORT2 V_CHEATLS ]
set cmds_4_3 [ list WHTAVELS RNDDRAWLS RNDDRAW_TOTS SORTS SORT2 ]
set cmds_5_2 [ list WRITELLS ]
set cmds_5_3 [ list RNDDRAW_TOTLS SORT2S ]
set cmds_5_4 [ list SORT2S ]

proc create_elem_file { path } {
	global exeTime cmds_1_1 cmds_2_1 cmds_2_2 cmds_3_1 cmds_3_2 cmds_3_3 cmds_4_1 cmds_4_2 cmds_4_3 cmds_5_2 cmds_5_3 cmds_5_4

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
		foreach cmd $cmds_4_1 {
			set calls [ regexp -all -inline -- [ subst -nocommands -nobackslashes {$cmd[ \t]*\([ \t]*\"(\w+)\"[ \t]*,[^;]+,[^;]+,[^;]+\)} ] $text ]
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
		foreach cmd $cmds_5_2 {
			set calls [ regexp -all -inline -- [ subst -nocommands -nobackslashes {$cmd[ \t]*\([^;]+,[ \t]*\"(\w+)\"[ \t]*,[^;]+,[^;]+,[^;]+\)} ] $text ]
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
	global modObj modElem progVar progPar missPar missVar

	if [ info exists modElem ] {
		set missVar [ remove_elem $progVar $modElem ]
		set missPar [ remove_elem $progPar $modElem ]
	} else {
		set missVar $progVar
		set missPar $progPar
	}

	if [ info exists modObj ] {
		set missVar [ remove_elem $missVar $modObj ]
		set missPar [ remove_elem $missPar $modObj ]
	}

	set missVar [ lsort -dictionary $missVar ]
	set missPar [ lsort -dictionary $missPar ]
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
