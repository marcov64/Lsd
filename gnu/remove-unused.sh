#!/bin/bash
#**************************************************************
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
#**************************************************************

#**************************************************************
# REMOVE-UNUSED.SH
# Remove unused Windows-only compiler and development tools.
#**************************************************************

if [ "$1" = "-h" ]; then
	echo "Remove unused Windows-only compiler and development tools"
	echo "Usage: ./remove-unused.sh [LSD PATH]"
	exit 0
fi

CUR_DIR="$( cd "$(dirname "${BASH_SOURCE[0]}" )" && pwd -P )"

if [ -z "$1" ]; then
	if [ ! -d "$CUR_DIR/bin" ]; then
		echo "Cannot find directory, aborting"
		exit 1
	else
		GNU_DIR="$CUR_DIR"
	fi
else
	if [ ! -d "$1/gnu/bin" ]; then
		echo "Invalid directory, aborting"
		exit 2
	else
		GNU_DIR="$1/gnu"
	fi
fi

# executables (gcc, Tcl/Tk, gdb, diff, multitail)
rm -f $GNU_DIR/bin

# include files (gcc, Tcl/Tk, gdb)
rm -f -R $GNU_DIR/include

# library files (gcc, Tcl/Tk, gdb)
rm -f -R $GNU_DIR/lib

# other files (gcc, gdb, multitail)
rm -f -R $GNU_DIR/share $GNU_DIR/x86_64-w64-mingw32 $GNU_DIR/etc $GNU_DIR/usr

