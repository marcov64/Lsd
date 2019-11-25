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
# LS2HTML.TCL
# Collection of procedures to manage HTML and other external files.
#*************************************************************

#************************************************
# LSDABOUT
# Show LSD About dialog box
#************************************************
proc LsdAbout { ver dat { parWnd "." } } {
	global tcl_platform

	set tit "About LSD"
	set plat [ string totitle $tcl_platform(platform) ];
	set mach $tcl_platform(machine)
	set os $tcl_platform(os)
	set osV $tcl_platform(osVersion)
	set tclV [ info patch ]
	set gccV [ gccVersion ]
	set copyr "written by Marco Valente, Universita' dell'Aquila\nand by Marcelo Pereira, University of Campinas\n\nCopyright Marco Valente and Marcelo Pereira\nLSD is distributed under the GNU General Public License"
	
	tk_messageBox -parent $parWnd -type ok -icon info -title $tit -message "Version $ver ($dat)" -detail "Platform: $plat ($mach)\nOS: $os ($osV)\nTcl/Tk: $tclV\nGCC: $gccV\n\n$copyr"
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
	global HtmlBrowser tcl_platform RootLsd
	set here [ pwd ]
	set f [ open $RootLsd/Manual/temp.html w ]
	puts $f "<meta http-equiv=\"Refresh\" content=\"0;url=$a\">"
	close $f
	set b "[file nativename $RootLsd/Manual/temp.html]"
	if { $tcl_platform(platform) == "unix" } {
		exec $HtmlBrowser $b &
	} {
		exec cmd.exe /c start $b &
	}
}


#************************************************
# LSDHTML
#************************************************
proc LsdHtml a {
	global HtmlBrowser tcl_platform
	set f [ open temp.html w ]
	puts $f "<meta http-equiv=\"Refresh\" content=\"0;url=$a\">"
	close $f
	set b "temp.html"
	if { $tcl_platform(platform) == "unix" } {
		exec $HtmlBrowser $b &
	} {
		exec cmd.exe /c start $b &
	}
}


#************************************************
# LSDTKDIFF
#************************************************
proc LsdTkDiff { a b { c "" } { d "" } } {
	global tcl_platform RootLsd wish LsdSrc
	if { $tcl_platform(platform) == "unix" } {
		exec $wish $RootLsd/$LsdSrc/tkdiff.tcl -L "$c" -L "$d" -lsd $a $b &
	} {
		exec $wish $RootLsd/$LsdSrc/tkdiff.tcl -L "$c" -L "$d" -lsd $a $b &
	}
}


#************************************************
# LS2HTML
# Procedure used to allocate a file 'index.html' in each directory
# containing links to each file
# the "from" parameter is the directory root where to start inserting the index.html files
# the "chop" parameter is the number of characters to chop from the beginning of the directory.
# For example, if the directory is C:/Lsd5.1, setting chop=3 will make appear the directory names as Lsd5.1
#************************************************
proc ls2html {from chop} {

	catch [set list [glob *]]
	if { [info exists list] == 0 } {return }

	foreach i $list {
		if { [file isdirectory $i] == 1 } {lappend ldir $i} {lappend lfile $i } 
	}

	if { [info exists ldir] == 1 } {
		set sortedlist [lsort -dictionary $ldir]
		foreach i $sortedlist {
			cd $i; ls2html $from $chop; cd ..;
		} 
	}

	set f [open index.html w]
	puts $f "<B><font size=+3><U>Directory: [string range [pwd] $chop end ]</U></font></B>"
	if { [pwd] != $from } {
		puts $f "\n<br><a href=\"../index.html\">Return</a> to UP directory" 
	}

	puts $f "\n<br>"

	if { [info exists ldir] == 1 } {
		puts $f "\n<br><hr style=\"width: 100%; height: 2px;\">"
		puts $f "<B><font size=+2>List of directories</B></font>\n<br>"
		foreach i $sortedlist {
			puts $f "\n<br><B>Dir: </B><a href=\"$i/index.html\">$i</a>" 
		} 
	} 

	if { [info exists lfile] == 1 } {
		set sortedlist [lsort -dictionary $lfile]
		puts $f "\n<br><hr style=\"width: 100%; height: 2px;\">"
		puts $f "<B><font size=+2>List of files</font></B>\n<br>"
		puts $f "<br><b><font face=\"Courier New,Courier\">File name...................Size, &nbsp Date</font></b><br>\n"

		foreach i $sortedlist {
			if { [string compare $i index.html] } {
				puts $f "<font face=\"Courier New,Courier\"><a href=\"$i\">$i</a>" 
				set len [string length $i]
				set np [expr 30 -$len]
				set np [expr $np - [string length [file size $i]]]
				set fill [string repeat . $np]
				puts $f "$fill [file size $i], "
				set fdate [clock format [file mtime $i] -format "%e %h %Y"]
				puts $f "$fdate </font>\n<br>"
			}
		}
	}

	if { [file exist description.txt] } {
		puts $f "<hr style=\"width: 100%; height: 2px;\">"
		puts $f "<B><font size=+2>Description</font></B>\n<br>\n<br>"
		set desc [open description.txt r]
		while { ![eof $desc] } {
			puts $f [gets $desc]
			puts $f "<br>"
		}   
		close $desc
	}
	close $f
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
			tk_messageBox -parent $par -type ok -title Error -icon error -message "Invalid file name or path" -detail "Invalid file name/path:\n\n'$fn'\n\nLSD files must have no spaces in the file names nor in their directory path. Please rename the file and/or move it to a different directory."
			return true
		} else {
			return false
		}
	}
	return false
}
