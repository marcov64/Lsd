#!/bin/bash
#**************************************************************
#
#	LSD 8.0 - May 2021
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
# ADD-SHORTCUT-MAC.SH
# Add a shortcut to LSD LMM in the macOS desktop and apps.
# Also fixes some macOS security issues.
#**************************************************************

if [ "$1" = "-h" ]; then
	echo "Add a shortcut to LSD LMM in the desktop"
	echo "Usage: ./add-shortcut-mac.sh"
	exit 0
fi

# remove existing aliases, if any
TARGET="LMM"
rm -f ~/Desktop/"$TARGET" ~/Desktop/"$TARGET.app" ~/Desktop/LSD\ Model\ Manager
rm -f ~/Applications/"$TARGET" ~/Applications/"$TARGET.app" ~/Applications/LSD\ Model\ Manager
	
# disable macOS quarantine of LSD executables
LSDAPP=LSD
LSDROOT="$( cd "$(dirname "${BASH_SOURCE[0]}" )" && pwd -P )"
sudo xattr -rd com.apple.quarantine "$LSDROOT/$TARGET.app"
sudo xattr -rd com.apple.quarantine "$LSDROOT/src/$LSDAPP.app"

# create alias on desktop
osascript >/dev/null <<END_SCRIPT
	tell application "Finder"
		set appHome to path to applications folder from user domain
		make new alias to file (posix file "$LSDROOT/$TARGET.app") at appHome with properties {name:"LSD Model Manager"}
		make new alias to file (posix file "$LSDROOT/$TARGET.app") at desktop with properties {name:"LSD Model Manager"}
	end tell
END_SCRIPT
