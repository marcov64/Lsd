#*************************************************************
#
#	LSD 9.0 - January 2026
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

if [ catch { source [ file join [ file dirname [ info script ] ] src/installer.tcl ] } ] {
    puts $errorInfo
}
