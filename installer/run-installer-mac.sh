#!/bin/bash
#**************************************************************
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
#**************************************************************

#**************************************************************
# RUN-INSTALLER-MAC.SH
# Disable translocation before running installer.
#**************************************************************

APPNAME="LSD Installer"
LSD_VER_NUM="9.0"
LSD_VER_TAG="beta-3"

if [[ "$1" == "-h" ]]; then
	echo "Run LSD installer from Terminal"
	echo "Usage: ./run-installer-mac.sh [LSD INSTALLER FILE NAME][.app]"
	exit 0
fi

if [[ "$1" == "" ]]; then
	INSTAPP="$APPNAME ($LSD_VER_NUM-$LSD_VER_TAG).app"
else
	if [[ "$1" == *".app" ]]; then
		INSTAPP="$1"
	else
		INSTAPP="$1.app"		
	fi
fi

if [ ! -d "$INSTAPP" ]; then
	INSTROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd -P )"
	if [ -d "$INSTROOT/$INSTAPP" ]; then
		INSTAPP="$INSTROOT/$INSTAPP"
	else
		echo "Could not locate installer $INSTAPP, aborting"
		exit 1
	fi
fi

if [ ! -f "$INSTAPP/Contents/MacOS/Wish" ]; then
	echo "Invalid installer $INSTAPP, aborting"
	exit 2
fi

INSTTMP="LSD.tmp"

# copy installer to temporary location
rm -f -R ~/"$INSTTMP"
mkdir ~/"$INSTTMP"
cp -f -R "$INSTAPP" ~/"$INSTTMP/"

# reset automation authorization, to force asking again
tccutil reset All com.marcov64.lsdi >/dev/null 2>&1
tccutil reset AppleEvents com.apple.terminal >/dev/null 2>&1

# disable macOS quarantine of LSD executable
sudo xattr -rd com.apple.quarantine ~/"$INSTTMP/$INSTAPP"

# execute application installer
open -W ~/"$INSTTMP/$INSTAPP"
rm -f -R ~/"$INSTTMP"

exit 0
