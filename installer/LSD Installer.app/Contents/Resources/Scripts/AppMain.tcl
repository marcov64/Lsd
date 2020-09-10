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
# APPMAIN.TCL
# Entry point to execute main script.
#*************************************************************

if { [ string first "-psn" [ lindex $argv 0 ] ] == 0 } {
    set argv [ lrange $argv 1 end ]
}

# set to hide normally or show for debugging
console hide

if [ catch { source [ file join [ file dirname [ info script ] ] src/installer.tcl ] } ] {
    puts $errorInfo
}
