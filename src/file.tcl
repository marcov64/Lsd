#*************************************************************
#
#	LSD 7.3 - December 2020
#	written by Marco Valente, Universita' dell'Aquila
#	and by Marcelo Pereira, University of Campinas
#
#	Copyright Marco Valente and Marcelo Pereira
#	LSD is distributed under the GNU General Public License
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
# LSDTKDIFF
#************************************************
proc open_diff { file1 file2 { file1name "" } { file2name "" } } {
	global CurPlatform tkApp wish sysTerm RootLsd LsdSrc diffApp diffFile1name diffFile2name diffFile1 diffFile2
	
	set cmdline "$RootLsd/$LsdSrc/$diffApp $diffFile1name $file1name $diffFile2name $file2name $diffFile1 $file1 $diffFile2 $file2"
	
	set optLinux ""
	set optMac ""
	set optWindows ""
	
	if { $tkApp == 0 } {
		set shell $wish
	} elseif { $tkApp == 1 } {
		set shell $sysTerm
		set optLinux "-e"
		set optMac "-e"
		set optWindows "/c"
	} else {
		set shell ""
	} 
	
	switch $CurPlatform {
		linux {
			set error [ catch { exec $shell $optLinux $cmdline & } result ]
		}
		mac {
			if { $tkApp == 1 } {
				set error [ catch { exec osascript $optMac "tell application \"$shell\" to do script \"cd [ pwd ]; $cmdline; exit\"" & } result ]
			} else {
				set error [ catch { exec $shell $optMac $cmdline & } result ]
			}
		}
		windows {
			set error [ catch { exec $shell $optWindows $cmdline & } result ]
		}
	}
	
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
	global CurPlatform sysTerm

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
				set error [ catch { exec wgnuplot.exe & } result ]
			} else {
				if { $wait } {
					set error [ catch { exec wgnuplot.exe $script } result ]
				} else {
					set error [ catch { exec wgnuplot.exe -p $script & } result ]
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
