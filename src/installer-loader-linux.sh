#!/bin/bash
#**************************************************************
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
#**************************************************************

#**************************************************************
# INSTALLER-LOADER.SH
# Script to load the main LSD installer in Tcl/Tk Wish.
#**************************************************************

TCL_REQ_VER="8.6"

# check if the correct version of Tcl/Tk is properly installed
if [ -x "$( command -v tclsh )" ]; then
	TCL_VER="$( echo 'puts $tcl_version; exit 0' | tclsh )"
	if [ "$TCL_VER" != "$TCL_REQ_VER" ] || [ ! -x "$( command -v wish )" ]; then
		TCL_VER="no"
	fi
else
	TCL_VER="no"
fi

if [ "$TCL_VER" == "no" ]; then
	echo "Cannot install LSD because Tcl/Tk $TCL_REQ_VER is not available"
	echo "Please install it with your package manager"
	
	# detect the package manager to use
	if [ -x "$( command -v apt-get )" ]; then 
		COM="sudo apt-get install tcl tk"
	else
		if [ -x "$( command -v yum )" ]; then 
			COM="sudo yum install tcl tk"
		else 
			if [ -x "$( command -v dnf )" ]; then 
				COM="sudo dnf install tcl tk"
			else
				if [ -x "$( command -v zypper )" ]; then 
					COM="sudo zypper install tcl tk"
				else 
					if [ -x "$( command -v urpmi )" ]; then 
						COM="sudo urpmi tcl tk"
					else 
						if [ -x "$( command -v apk )" ]; then 
							COM="sudo apk add tcl tk"
						else
							COM="no"
						fi
					fi
				fi
			fi
		fi
	fi

	# end the script
	if [ "$COM" != "no" ]; then
		echo "When using the terminal, the install command should be:"
		echo
		echo " $ $COM"
		echo

		read -p "Do you want to try to install Tcl/Tk now? (Y/n)" -n 1 -r
		echo
		if [[ $REPLY =~ ^[Yy]$ ]]; then
			echo "Please enter your password if asked and accept the installation"
			echo
			eval "$COM"
			if [ "$?" == "0" ]; then
				if [ ""$( echo 'puts $tcl_version; exit 0' | tclsh )"" == "$TCL_REQ_VER" ]; then
					exec $( readlink -f "$0" )
				else
					echo "LSD requires Tcl/Tk version $TCL_REQ_VER"
					echo "Check your distribution documentation to see if it is available"
				fi
			else
				echo "Could not install Tcl/Tk"
				echo "Check your distribution documentation on how to do it"
			fi
		fi
	fi
	
	echo "Please retry to run the LSD installer script after installing Tcl/Tk"
	echo "Exiting now"

	exit 1
fi

# launch the main installer
wish ./src/installer.tcl

exit $?

