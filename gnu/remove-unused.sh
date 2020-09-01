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

# MSYS2 libraries and make utility
rm -f "$GNU_DIR/bin/make.exe" "$GNU_DIR/bin/rm.exe" "$GNU_DIR/bin/msys-2.0.dll" "$GNU_DIR/bin/msys-intl-8.dll" "$GNU_DIR/bin/msys-iconv-2.dll"

# g++ compiler
rm -f "$GNU_DIR/bin/gcc.exe" "$GNU_DIR/bin/g++.exe" "$GNU_DIR/bin/windres.exe" "$GNU_DIR/bin/libwinpthread-1.dll" "$GNU_DIR/bin/libgcc_s_seh-1.dll" "$GNU_DIR/bin/libgmp-10.dll" "$GNU_DIR/bin/libstdc++-6.dll" "$GNU_DIR/bin/libzstd.dll" "$GNU_DIR/bin/zlib1.dll"

# Tcl/Tk framework
rm -f "$GNU_DIR/bin/tclsh86.exe" "$GNU_DIR/bin/wish86.exe" "$GNU_DIR/bin/tcl86.dll" "$GNU_DIR/bin/tk86.dll"

# gdb debugger
rm -f "$GNU_DIR/bin/gdb.exe" "$GNU_DIR/bin/python3.8-config" "$GNU_DIR/bin/libpython3.8.dll" "$GNU_DIR/bin/libreadline8.dll" "$GNU_DIR/bin/libtermcap-0.dll" "$GNU_DIR/bin/libxxhash.dll"

# diff compare tool
rm -f "$GNU_DIR/bin/diff.exe" "$GNU_DIR/bin/libiconv-2.dll" "$GNU_DIR/bin/libintl-8.dll" "$GNU_DIR/bin/libsigsegv-2.dll"

# include files (gcc and Tcl/Tk)
rm -f -R "$GNU_DIR/include"

# library files (gcc and Tcl/Tk)
rm -f -R "$GNU_DIR/lib"

# other files (gcc and gdb)
rm -f -R "$GNU_DIR/share" "$GNU_DIR/x86_64-w64-mingw32" "$GNU_DIR/etc"

# Cygwin libraries
rm -f "$GNU_DIR/bin/cygwin1.dll" "$GNU_DIR/bin/cygncursesw-10.dll" "$GNU_DIR/bin/cygpanelw-10.dll"

# multitail utility
rm -f "$GNU_DIR/bin/multitail.exe" 
rm -f -R "$GNU_DIR/usr"

# 7-Zip compressing tool
rm -f "$GNU_DIR/bin/7zr.exe" "$GNU_DIR/bin/7zSD.sfx"
