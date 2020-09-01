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
# FINDFILES.TCL
# Tcl procedure to list all files with a given pattern
# within a directory subtree.
# Written by Joseph Bui (https://stackoverflow.com/a/448573).
# Arguments:
# - directory - the top directory to start looking in
# - pattern - A pattern, as defined by the glob command, 
#	that the files must match
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
    set parents $directory
	
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

	set directories [ concat $directory $directories ]
	
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
					lappend that "$fil"
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
