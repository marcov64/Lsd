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
# Tcl script used to install LSD in all platforms.
#*************************************************************

wm withdraw .

set LsdDir LSD
set LsdSrc src
set winGnuplot "gp528-win64-mingw.exe"
set winRoot "C:/"
set notInstall [ list .git/* installer/* lwi/* R/* Manual/src/* ]

# set absolute reference paths
set homeDir [ file normalize "~/." ]
set scptDir [ file normalize "[ file dirname [ info script ] ]/.." ]
set macZip [ file normalize "$scptDir/../Package/LSD-archive-mac.zip" ]

# load support tools
set RootLsd "$scptDir"
source "$RootLsd/$LsdSrc/gui.tcl"
source "$RootLsd/$LsdSrc/file.tcl"
source "$RootLsd/$LsdSrc/util.tcl"

# set main window
set fonttype $DefaultFontSize
set dim_character $DefaultFontSize
set small_character [ expr $dim_character - $deltaSize ]
setstyles
icontop . lsd

# adjust platform-specific defaults
if [ string equal $CurPlatform mac ] {
	set notInstall [ concat $notInstall *.exe *.dll *.bat gnu/etc/* gnu/include/* gnu/lib/* gnu/share/* gnu/usr/* gnu/x86_64-w64-mingw32/* ]
	
	# make sure PATH is complete
	if { [ string first "/usr/local/bin" env(PATH) ] < 0 } {
		set env(PATH) "/usr/local/bin:$env(PATH)"
	}
	
	# create temporary folder
	try {
		close [ file tempfile temp ]
		file delete -force $temp
		set filesDir [ file normalize [ file join [ file dirname $temp ] [ file tail $temp ] ] ]
		file mkdir "$filesDir"
	} on error result {
		ttk::messageBox -type ok -title Error -icon error -message "Cannot create temporary files" -detail "Please check your file system and try again.\n\nExiting now."
		exit 1
	}
	
	# detect missing components
	if [ catch { exec which g++ } ] {
		set xcode 1
	}
	if [ catch { exec which gnuplot } ] {
		set gnuplot 1
	}	
	if [ catch { exec which multitail } ] {
		set multitail 1
	}	
	
} elseif [ string equal $CurPlatform linux ] {
	set notInstall [ concat $notInstall *.exe *.dll *.bat LMM.app/* $LsdSrc/LSD.app/* gnu/etc/* gnu/include/* gnu/lib/* gnu/share/* gnu/usr/* gnu/x86_64-w64-mingw32/* ]
	set filesDir [ file normalize [ pwd ] ]
	
} elseif [ string equal $CurPlatform windows ] {
	set notInstall [ concat $notInstall *.sh LMM.app/* $LsdSrc/LSD.app/* ]
	set filesDir [ file normalize [ pwd ] ]
	
	# detect missing components
	if { [ catch { exec where wgnuplot.exe } ] && ! [ file exists "C:/Program Files/gnuplot/bin/wgnuplot.exe" ] } {
		set gnuplot 1
	}
	
} else {
	ttk::messageBox -type ok -title Error -icon error -message "Invalid platform" -detail "LSD cannot be installed in this computer.\n\nExiting now."
	exit 2
}
		
# check default LSD directory location
if { [ string equal $CurPlatform mac ] || [ string equal $CurPlatform linux ] } {
	if { [ string first " " "$homeDir" ] < 0 } { 
		set LsdRoot "~/$LsdDir"
	} else {
		ttk::messageBox -type ok -title Warning -icon warning -message "Home directory includes space(s)" -detail "The system directory '$homeDir' is invalid for installing LSD.\nLSD subdirectory must be located in a directory with no spaces in the full path name.\n\nYou may use another directory if you have write permissions to it.\n\nExiting now."
		set homeDir "/"
		set LsdRoot "/$LsdDir"
	}
	
} else {
	if { [ string first " " "$homeDir" ] < 0 } { 
		set LsdRoot [ file normalize "~/$LsdDir" ]
	} elseif [ file exists $winRoot ] {
		set homeDir "$winRoot"
		set LsdRoot "${winRoot}$LsdDir"
	} else {
		set homeDir "/"
		set LsdRoot "/$LsdDir"
	}
}

# unzip files if not done yet
if [ string equal $CurPlatform mac ] {
	waitbox .wait "Decompressing..." "Decompressing files.\n\nIt may take a while, please wait..."
	try {
		set result [ exec unzip -oqq $macZip -d $filesDir ]
		destroytop .wait
	} on error result {
		destroytop .wait
		ttk::messageBox -type ok -title Error -icon error -message "Disk full or corrupt package" -detail "The computer storage is full or the installation package does not contain the required files to install LSD.\n\nPlease check your disk space or try to download again the installation package.\n\nExiting now."
		exit 3
	}
}

# select files to install
set files [ findfiles "$filesDir" * 1 ]
foreach rem $notInstall {
	set toRemove [ lsearch -glob -all -inline $files $rem ]
	foreach f $toRemove {
		set pos [ lsearch -exact $files $f ]
		set files [ lreplace $files $pos $pos ]
	}
}

# check if bare minimum is there
if { [ llength $files ] < 100 || [ string first Readme.txt $files ] < 0 || [ string first lsdmain.cpp $files ] < 0 || [ string first groupinfo.txt $files ] < 0 } {
	ttk::messageBox -type ok -title Error -icon error -message "Corrupt installation package" -detail "The installation package does not contain the required files to install LSD.\n\nPlease try to download again the installation package."
	exit 4
}

# main (select directory) window
ttk::frame .dir
ttk::frame .dir.choice
ttk::frame .dir.choice.blk
ttk::label .dir.choice.blk.lab -text "Directory to install LSD"
ttk::entry .dir.choice.blk.where -textvariable LsdRoot -width 40 -justify center
pack .dir.choice.blk.lab .dir.choice.blk.where
bind .dir.choice.blk.where <Return> { .b.ok invoke }
ttk::frame .dir.choice.but
ttk::label .dir.choice.but.lab
ttk::button .dir.choice.but.browse -text Browse -width -1 -command {
	set dir [ tk_chooseDirectory -initialdir "$homeDir" -title "Choose a directory" ]
	if { $dir != "" } {
		set LsdRoot "$dir"
	}
	.dir.choice.blk.where selection range 0 end 
	focus .dir.choice.blk.where
}
pack .dir.choice.but.lab .dir.choice.but.browse
pack .dir.choice.blk .dir.choice.but -padx 5 -side left
ttk::label .dir.obs -text "If LSD is already installed in the\nselected directory, it will be updated" -justify center
pack .dir.choice .dir.obs -pady 5
if { [ info exists xcode ] && [ info exists gnuplot ] } {
	ttk::label .dir.extra -text "Xcode command line tools and\nGnuplot graphical terminal\nare not available and will be installed" -justify center
	pack .dir.extra -pady 5
} else {
	if [ info exists xcode ] {
		ttk::label .dir.extra -text "Xcode command line tools are\nnot available and will be installed" -justify center
		pack .dir.extra -pady 5
	} elseif [ info exists gnuplot ] {
		ttk::label .dir.extra -text "Gnuplot graphical terminal seems\nunavailable and will be installed" -justify center
		pack .dir.extra -pady 5
	}
}
ttk::label .dir.lic -text "LSD is free software and comes\nwith ABSOLUTELY NO WARRANTY\nSee Readme.txt for copyright information" -justify center
pack .dir.lic -pady 5
pack .dir -padx 10 -pady 10

okcancel . b { set done 1 } { set done 2 }

wm geometry . +[ getx . centerS ]+[ gety . centerS ]
settop . "LSD Installer" { set done 2 }

set newInst 1

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
		
		if { [ catch { set LsdRoot [ file normalize $LsdRoot ] } ] || ! [ file exists [ file dirname "$LsdRoot" ] ] } {
			ttk::messageBox -type ok -title Error -icon error -message "Invalid directory path/name" -detail "The directory path '$LsdRoot' is invalid.\nA valid directory path and names must be supplied. Only valid characters for directory names are accepted. The parent directory to the LSD subdirectory must exist.\n\nPlease choose another path/name."
			continue
		}
		
		if { [ string first " " "$LsdRoot" ] >= 0 } { 
			ttk::messageBox -type ok -title Error -icon error -message "Directory includes space(s)" -detail "The chosen directory '$LsdRoot' is invalid for installing LSD.\nLSD subdirectory must be located in a directory with no spaces in the full path name.\n\nPlease choose another directory."
			continue
		}
		
		if { ! [ file writable [ file dirname "$LsdRoot" ] ] } {
			ttk::messageBox -type ok -title Error -icon error -message "Directory not writable" -detail "The chosen directory '[ file dirname "$LsdRoot" ]' is invalid for installing LSD.\nLSD subdirectory must be located in a directory where the user has write permission.\n\nPlease choose another directory."
		} else {
			break
		}
	}

	if { $done == 1 && [ file exists $LsdRoot ] } {
		if { ! [ string equal [ ttk::messageBox -type okcancel -title Warning -icon warning -default ok -message "Directory already exists" -detail "Directory '$LsdRoot' already exists.\n\nPress 'OK' to continue and update installed files or 'Cancel' to abort installation." ] ok ] } {
			continue
		} else {
			set newInst 0
			catch { file delete -force "$LsdRoot/$LsdSrc/system_options.txt" }
		}
	}

	if { $done == 2 } {
		if [ string equal [ ttk::messageBox -type okcancel -title "Exit?" -icon info -default ok -message "Exit installation?" -detail "LSD installation is not complete.\n\nPress 'OK' to confirm exit." ] ok ] {
			exit 5
		}
	} else {
		break
	}
}

if { ! [ file exists "$LsdRoot" ] && [ catch { file mkdir "$LsdRoot" } ] } {
	ttk::messageBox -type ok -title Error -icon error -message "Cannot create LSD directory" -detail "The chosen directory '$LsdRoot' could not be created.\nLSD subdirectory must be located in a directory where the user has write permission.\n\nPress 'OK' to abort installation."
	exit 6
}

wm withdraw .

# create progress bar window
set n 0
set nFiles [ llength $files ]
set inst [ progressbox .inst "Installation progress" $nFiles n { set done 2 } ]

# copy files
foreach f $files {
	try {
		file mkdir [ file dirname "$LsdRoot/$f" ]
		file copy -force "$filesDir/$f" "$LsdRoot/$f" 
	} on error result {
		break
	}
	
	incr n
	if { $n % 20 == 0 } {
		$inst configure -text "$n of $nFiles files installed ([ expr int( 100 * $n / $nFiles ) ]%)"
	}
	
	focus .inst.b.cancel
	update
	
	if { $done == 2 } {
		if [ string equal [ ttk::messageBox -type okcancel -title "Exit?" -icon info -default ok -message "Exit installation?" -detail "LSD installation is not complete.\n\nPress 'OK' to confirm exit." ] ok ] {
			exit 7
		}
	}
}

# remove temporary files
if [ string equal $CurPlatform mac ] {
	$inst configure -text "Removing temporary files..."
	catch { file delete -force $filesDir }
}

destroytop .inst

if { $n != $nFiles } {
	if { ! [ info exists result ] } {
		set result "(no information)"
	}
	ttk::messageBox -type ok -title Error -icon error -message "Incomplete installation" -detail "The installation could not install the required files to run LSD ($n out of $nFiles installed).\n\nError detail:\n$result\n\nPlease try reinstalling or download again the installation package."
}

# add icons to desktop and program menu
cd $LsdRoot
if [ string equal $CurPlatform windows ] {
	set res [ catch { exec $LsdRoot/add-shortcut-windows.bat } result ]
} elseif [ string equal $CurPlatform linux ] {
	set res [ catch { exec $LsdRoot/add-shortcut-linux.sh } result ]
} else {
	ttk::messageBox -type ok -title "Password" -icon info -message "Password required" -detail "The next step of installation will require the password of a user with administrator rights.\n\nThis is required so LSD can be installed out of the macOS quarantine zone for new executable files."
	update
	set res [ catch { exec osascript -e "do shell script \"$LsdRoot/add-shortcut-mac.sh 2>&1 /dev/nul\" with administrator privileges" } result ]
}

if { $res } {
	ttk::messageBox -type ok -title Error -icon error -message "Error configuring computer" -detail "The configuration of LSD installation failed ($result).\n\nYou may try to repeat the installation or do a manual install following the instructions steps described in 'Readme.txt'.\n\nExiting now."
	if { $newInst } {
		catch { file delete -force $LsdRoot }
	}
	exit 8
}

# install Xcode command line tools if required
if [ info exists xcode ] {
	ttk::messageBox -type ok -title "Xcode Installation" -icon info -message "User interaction required" -detail "The next step of installation will require the user to confirm Xcode command line tools installation.\n\nThis is required to install the C++ compiler and development tools."
	waitbox .wait "Installing..." "Installing Xcode command line tools.\nAn internet connection is required.\nPlease perform the steps below.\nIt may take a while, please wait..." "1. if required, allow the Terminal access\n2. click on 'Install'\n3. agree with the license agreement\n4. wait for the download\n5. click on 'Done'"
	set res [ catch { exec xcode-select --install } result ]
	destroytop .wait
	if { $res } {
		ttk::messageBox -type ok -title Error -icon error -message "Error installing Xcode" -detail "The installation of Xcode command line tools failed ($result).\n\nYou may try to repeat the installation or do a manual install following the instructions steps described in 'Readme.txt'."
	}
}

# install Gnuplot/multitail if required
if { [ info exists gnuplot ] || [ info exists multitail ] } {
	if [ string equal $CurPlatform windows ] {
		set res [ catch { exec $filesDir/installer/$winGnuplot /SILENT /LOADINF=wgnuplot.inf /LOG=D:\wgnuplot.log } result ]
	}
	
	if [ string equal $CurPlatform mac ] {
		# check if Homebrew is installed and install if not
		set res 0
		if [ catch { exec which brewXX } ] {
			set brewInsta "/bin/bash -c \\\"\$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install.sh)\\\"; "
			set brewInstr "Homebrew package manager, \\n"
			set brewSteps "in Terminal type your password and press <Return> twice\n3. "
			set brewMsg1 "Homebrew, "
			set brewMsg2 "\\nType your password and press <Return> twice:"
		} else {
			set brewInsta ""
			set brewInstr ""
			set brewSteps ""
			set brewMsg1 ""
			set brewMsg2 ""
		}
		
		if { [ info exists gnuplot ] && [ info exists multitail ] } {
			set pkgInsta "brew install multitail gnuplot; "
		} elseif { [ info exists gnuplot ] } {
			set pkgInsta "brew install gnuplot; "
		} else {
			set pkgInsta "brew install multitail; "
		}
		
		ttk::messageBox -type ok -title "Tools Installation" -icon info -message "User interaction required" -detail "The next step of installation will require the user to confirm installation of ${brewInstr}Gnuplot graphical terminal and/or multitail tool.\n\nA Terminal window will open and the interaction must be performed there."
		set wait [ waitbox .wait "Installing..." "Installing ${brewInstr} Gnuplot graphical terminal and/or multitail tool.\nAn internet connection is required.\nPlease perform the steps below.\nIt may take a while, please wait..." "1. if required, allow the Terminal access\n2. ${brewSteps}Terminal window will close/disable when done\n" 1 ]

		set scpt [ open "$env(TMPDIR)/install_homebrew.as" w ]
		puts $scpt "tell application \"Terminal\""
		set openMsg "clear; echo \\\"Installing ${brewMsg1}Gnuplot and/or multitail\\nPlease wait for this window to close/deactivate automatically.${brewMsg2}\\\"; "
		set closeMsg "${pkgInsta}touch \$TMPDIR/brew-done.tmp; exit"
		puts $scpt "\tdo script \"${openMsg}${brewInsta}${closeMsg}\""
		puts $scpt "end tell"
		close $scpt
		exec chmod +x "$env(TMPDIR)/install_homebrew.as"

		file delete -force "$env(TMPDIR)/brew-done.tmp"
		set res [ catch { exec osascript "$env(TMPDIR)/install_homebrew.as" } ]
		
		if { ! $res } {
			set timeout 3600
			set elapsed 0
			while { ! [ file exists "$env(TMPDIR)/brew-done.tmp" ] && $elapsed < $timeout } {
				$wait configure -text [ format "Time elapsed: %02d:%02d" [ expr int( $elapsed / 60 ) ] [ expr $elapsed % 60 ] ]
				update
				after 1000
				incr elapsed
			}
			
			if { $elapsed >= $timeout } {
				set res 1
			}
		}
		
		destroytop .wait
	}
	
	if { $res } {
		ttk::messageBox -type ok -title Error -icon error -message "Error installing Gnuplot" -detail "The installation of Gnuplot graphical terminal (and/or other associated tools) failed ($result).\n\nYou may try to repeat the installation or do a manual install following the instructions steps described in 'Readme.txt'."
	}
}

# verify if all tools are installed in linux
if [ string equal $CurPlatform linux ] {
	waitbox .wait "Checking Tools..." "Checking if the required development tools\nare available and installing as necessary.\nAn internet connection is required.\n\nIt may take a while, please wait..." ]
	set res [ catch { exec $LsdRoot/$LsdSrc/check-dev-tools-linux.sh } result ]
	destroytop .wait
	if { $res } {
		ttk::messageBox -type ok -title Error -icon error -message "Error checking tools" -detail "The check of required development tools failed ($result).\n\nYou may try to repeat the installation or do a manual install following the instructions steps described in 'Readme.txt'."
	}
}

# finish installation
destroy .dir .b

ttk::frame .end
ttk::label .end.msg1 -text "LSD installation completed successfully"
ttk::label .end.msg2 -text "The installation directory is: $LsdRoot"

ttk::label .end.msg3 -text "LSD/LMM can be run using the created desktop icon,\nor using the computer program menu"  -justify center
pack .end.msg1 .end.msg2 .end.msg3 -pady 5
pack .end -padx 10 -pady 10

ttk::frame .b
ttk::button .b.run -width $butWid -text "Run Now" -command { set done 1 }
ttk::button .b.finish -width $butWid -text Finish -command { set done 2 }
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
		catch { exec $LsdRoot/run.bat & }
	} elseif [ string equal $CurPlatform linux ] {
		catch { exec $LsdRoot/run.sh & }
	} else {
		catch { exec open -F -n $LsdRoot/LMM.app & }
	}
}

exit 0
