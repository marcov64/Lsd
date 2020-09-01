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
# SETUP.TCL
# Tcl scripts used to install LSD.
#*************************************************************

wm withdraw .

source ./installer/findfiles.tcl

set lsdDir LSD
set tmpDir [ file normalize [ pwd ] ]
set homeDir [ file normalize "~/." ]
set gnuplot 0

set winRoot "C:/"
set winGnuplot "gp528-win64-mingw.exe"

set notInstall [ list .git/* installer/* lwi/* R/* Manual/src/* ]

if [ string equal $tcl_platform(platform) unix ] {
	if [ string equal $tcl_platform(os) Darwin ] {
		set CurPlatform mac
		set notInstall [ concat $notInstall *.exe *.dll *.bat gnu/etc/* gnu/include/* gnu/lib/* gnu/share/* gnu/usr/* gnu/x86_64-w64-mingw32/* ]
	} else {
		set CurPlatform linux
		set notInstall [ concat $notInstall *.exe *.dll *.bat LMM.app/* src/LSD.app/* gnu/etc/* gnu/include/* gnu/lib/* gnu/share/* gnu/usr/* gnu/x86_64-w64-mingw32/* ]

		image create photo lsdImg -file "./src/icons/lsd.png"
		wm iconphoto . -default lsdImg
		wm iconbitmap . @./src/icons/lsd.xbm
	}
	
	if { [ string first " " "$homeDir" ] < 0 } { 
		set lsdRoot "~/$lsdDir"
	} else {
		tk_messageBox -type ok -title Warning -icon warning -message "Home directory includes space(s)" -detail "The system directory '$homeDir' is invalid for installing LSD.\nLSD subdirectory must be located in a directory with no spaces in the full path name.\n\nYou may use another directory if you have write permissions to it."
		set homeDir "/"
		set lsdRoot "/$lsdDir"
	}
} else {
	if { [ string equal $tcl_platform(platform) windows ] && [ string equal $tcl_platform(machine) amd64 ] } {
		set CurPlatform windows
		set notInstall [ concat $notInstall *.sh LMM.app/* src/LSD.app/* ]
		
		wm iconbitmap . -default ./src/icons/lsd.ico
		
		if { [ catch { exec where wgnuplot.exe } ] && ! [ file exists "C:/Program Files/gnuplot/bin/wgnuplot.exe" ] } {
			set gnuplot 1
		}
		
		if { [ string first " " "$homeDir" ] < 0 } { 
			set lsdRoot [ file normalize "~/$lsdDir" ]
		} elseif [ file exists $winRoot ] {
			set homeDir "$winRoot"
			set lsdRoot "${winRoot}$lsdDir"
		} else {
			set homeDir "/"
			set lsdRoot "/$lsdDir"
		}
	} else {
		tk_messageBox -type ok -title Error -icon error -message "Invalid platform" -detail "LSD cannot be installed in this computer.\nExiting now."
		destroy .
		exit 0
	}
}

# select files to install
set files [ findfiles "$tmpDir" * 1 ]
foreach rem $notInstall {
	set toRemove [ lsearch -glob -all -inline $files $rem ]
	foreach f $toRemove {
		set pos [ lsearch -exact $files $f ]
		set files [ lreplace $files $pos $pos ]
	}
}

# check if bare minimum is there
if { [ llength $files ] < 100 || [ string first Readme.txt $files ] < 0 || [ string first lmm $files ] < 0 || [ string first lsdmain.cpp $files ] < 0 || [ string first groupinfo.txt $files ] < 0 } {
	tk_messageBox -type ok -title Error -icon error -message "Corrupt installation package" -detail "The installation package does not contain the required files to install LSD.\n\nPlease try to download again the installation package."
	destroy .
	exit 0
}

# install directory window
ttk::frame .dir
ttk::frame .dir.choice
ttk::frame .dir.choice.blk
ttk::label .dir.choice.blk.lab -text "Directory to install LSD"
ttk::entry .dir.choice.blk.where -textvariable lsdRoot -width 40 -justify center
pack .dir.choice.blk.lab .dir.choice.blk.where
bind .dir.choice.blk.where <Return> { .b.ok invoke }
ttk::frame .dir.choice.but
ttk::label .dir.choice.but.lab
ttk::button .dir.choice.but.browse -text Browse -width -1 -command {
	set dir [ tk_chooseDirectory -initialdir "$homeDir" -title "Choose a directory" ]
	if { $dir != "" } {
		set lsdRoot "$dir"
	}
	.dir.choice.blk.where selection range 0 end 
	focus .dir.choice.blk.where
}
pack .dir.choice.but.lab .dir.choice.but.browse
pack .dir.choice.blk .dir.choice.but -padx 5 -side left
ttk::label .dir.obs1 -text "If LSD is already installed in the\nselected directory, it will be updated" -justify center
if { $gnuplot } {
	ttk::label .dir.obs2 -text "Gnuplot does not seem to be installed,\nit will be installed in the standard directory" -justify center
	pack .dir.choice .dir.obs1 .dir.obs2 -pady 5
} else {
	pack .dir.choice .dir.obs1 -pady 5
}
pack .dir -padx 10 -pady 10

ttk::frame .b
ttk::button .b.ok -width 9 -text OK -command { set done 1 }
ttk::button .b.cancel -width 9 -text Cancel -command { set done 2 }
bind .b.ok <Return> { .b.ok invoke }
bind .b.cancel <Return> { .b.cancel invoke }
bind . <Escape> { .b.cancel invoke }
pack .b.ok .b.cancel -padx 10 -pady 10 -side left
pack .b -side right

set centerX [ expr [ winfo screenwidth . ] / 2 - [ winfo reqwidth . ] / 2 ]
set centerY [ expr [ winfo screenheight . ] / 2 - [ winfo reqheight . ] / 2 ]
wm geometry . +$centerX+$centerY
wm protocol . WM_DELETE_WINDOW { set done 2 }
wm title . "LSD Installer"
wm deiconify .
raise .
update

# check user option
while 1 {
	while 1 {
		.dir.choice.blk.where selection range 0 end 
		focus .dir.choice.blk.where

		set done 0
		tkwait variable done

		if { $done == 2 } { 
			break
		}
		
		if { [ catch { set lsdRoot [ file normalize $lsdRoot ] } ] || ! [ file exists [ file dirname "$lsdRoot" ] ] } {
			tk_messageBox -type ok -title Error -icon error -message "Invalid directory path/name" -detail "The directory path '$lsdRoot' is invalid.\nA valid directory path and names must be supplied. Only valid characters for directory names are accepted. The parent directory to the LSD subdirectory must exist.\n\nPlease choose another path/name."
			continue
		}
		
		if { [ string first " " "$lsdRoot" ] >= 0 } { 
			tk_messageBox -type ok -title Error -icon error -message "Directory includes space(s)" -detail "The chosen directory '$lsdRoot' is invalid for installing LSD.\nLSD subdirectory must be located in a directory with no spaces in the full path name.\n\nPlease choose another directory."
			continue
		}
		
		if { ! [ file writable [ file dirname "$lsdRoot" ] ] } {
			tk_messageBox -type ok -title Error -icon error -message "Directory not writable" -detail "The chosen directory '[ file dirname "$lsdRoot" ]' is invalid for installing LSD.\nLSD subdirectory must be located in a directory where the user has write permission.\n\nPlease choose another directory."
		} else {
			break
		}
	}

	if { $done == 1 && [ file exists $lsdRoot ] } {
		if { ! [ string equal [ tk_messageBox -type okcancel -title Warning -icon warning -default ok -message "Directory already exists" -detail "Directory '$lsdRoot' already exists.\n\nPress 'OK' to continue and update installed files or 'Cancel' to abort installation." ] ok ] } {
			continue
		}
	}

	if { $done == 2 } {
		if [ string equal [ tk_messageBox -type okcancel -title "Exit?" -icon info -default ok -message "Exit installation?" -detail "LSD installation is not complete.\n\nPress 'OK' to confirm exit." ] ok ] {
			destroy .
			exit 0
		}
	} else {
		break
	}
}

if { ! [ file exists "$lsdRoot" ] && [ catch { file mkdir "$lsdRoot" } ] } {
	tk_messageBox -type ok -title Error -icon error -message "Cannot create LSD directory" -detail "The chosen directory '$lsdRoot' could not be created.\nLSD subdirectory must be located in a directory where the user has write permission.\n\nPress 'OK' to abort installation."
	destroy .
	exit 0
}

wm withdraw .

# copy progress bar window
set n 0
set nFiles [ llength $files ]

toplevel .inst
ttk::frame .inst.main
ttk::label .inst.main.lab -text "Installation progress"
ttk::progressbar .inst.main.scale -length 300 -maximum $nFiles -variable n
ttk::label .inst.main.info
pack .inst.main.lab .inst.main.scale .inst.main.info -pady 5
pack .inst.main -padx 10 -pady 10

ttk::frame .inst.b
ttk::button .inst.b.cancel -width 9 -text Cancel -command { set done 2 }
bind .inst.b.cancel <Return> { .inst.b.cancel invoke }
bind .inst <Escape> { .inst.b.cancel invoke }
pack .inst.b.cancel -padx 10 -pady 10 -side left
pack .inst.b -side right

set centerX [ expr [ winfo screenwidth .inst ] / 2 - [ winfo reqwidth .inst ] / 2 ]
set centerY [ expr [ winfo screenheight .inst ] / 2 - [ winfo reqheight .inst ] / 2 ]
wm geometry .inst +$centerX+$centerY
wm protocol .inst WM_DELETE_WINDOW { set done 2 }
wm title .inst "LSD Installation"
raise .inst
grab .inst

# copy files
foreach f $files {
	if { ! [ file exists [ file dirname "$lsdRoot/$f" ] ] && [ catch { file mkdir [ file dirname "$lsdRoot/$f" ] } ] } {
		break
	}
	
	if [ catch { file copy -force "$tmpDir/$f" "$lsdRoot/$f" } ] {
		break
	}
	
	incr n
	if { $n % 20 == 0 } {
		.inst.main.info configure -text "$n of $nFiles files installed ([ expr int( 100 * $n / $nFiles ) ]%)"
	}
	
	focus .inst.b.cancel
	update
	
	if { $done == 2 } {
		if [ string equal [ tk_messageBox -type okcancel -title "Exit?" -icon info -default ok -message "Exit installation?" -detail "LSD installation is not complete.\n\nPress 'OK' to confirm exit." ] ok ] {
			destroy .
			exit 0
		}
	}
}

destroy .inst

if { $n != $nFiles } {
	tk_messageBox -type ok -title Error -icon error -message "Incomplete installation" -detail "The installation could not install the required files to run LSD ($n out of $nFiles installed).\n\nPlease try reinstalling or download again the installation package."
	destroy .
	exit 0
}

# add icons to desktop and program menu
cd $lsdRoot
if [ string equal $CurPlatform windows ] {
	set res [ catch { exec add-shortcut-windows.bat } result ]
} elseif [ string equal $CurPlatform linux ] {
	set res [ catch { exec add-shortcut-linux.sh } result ]
} else {
	set res [ catch { exec add-shortcut-mac.sh } result ]
}

if { $res } {
	tk_messageBox -type ok -title Error -icon error -message "Error creating shortcuts" -detail "The creation of LSD shortcuts failed ($result).\n\nPlease try again using the instructions steps described in 'Readme.txt'."
}

# install gnuplot if required
if { $gnuplot } {
	if [ string equal $CurPlatform windows ] {
		if [ catch { exec $tmpDir/installer/$winGnuplot /SILENT /LOADINF=wgnuplot.inf /LOG=D:\wgnuplot.log } result ] {
		tk_messageBox -type ok -title Error -icon error -message "Error installing Gnuplot" -detail "The installation of Gnuplot failed ($result).\n\nPlease try again using the recommended Gnuplot installation steps described in 'Readme.txt'."
		destroy .
		exit 0
		}
	}
}

# finish installation
destroy .dir .b

ttk::frame .end
ttk::label .end.msg1 -text "LSD installation completed successfully"
ttk::label .end.msg2 -text "The installation directory is: $lsdRoot"

ttk::label .end.msg3 -text "LSD/LMM can be run using the created desktop icon,\nor using the computer program menu"  -justify center
pack .end.msg1 .end.msg2 .end.msg3 -pady 5
pack .end -padx 10 -pady 10

ttk::frame .b
ttk::button .b.run -width 9 -text "Run Now" -command { set done 1 }
ttk::button .b.finish -width 9 -text Finish -command { set done 2 }
bind .b.run <Return> { .b.run invoke }
bind .b.finish <Return> { .b.finish invoke }
bind . <Escape> { .b.finish invoke }
pack .b.run .b.finish -padx 10 -pady 10 -side left
pack .b -side right

wm deiconify .
raise .
focus .b.run
update

set done 0
tkwait variable done

# open LMM
if { $done == 1 } {
	if [ string equal $CurPlatform windows ] {
		catch { exec run.bat & }
	} elseif [ string equal $CurPlatform linux ] {
		catch { exec run.sh & }
	} else {
		catch { exec open -F -n LMM.app & }
	}
}

destroy .
exit 0