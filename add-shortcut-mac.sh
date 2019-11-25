#!/bin/bash
#**************************************************************
#
#	LSD 7.2 - December 2019
#	written by Marco Valente, Universita' dell'Aquila
#	and by Marcelo Pereira, University of Campinas
#
#	Copyright Marco Valente and Marcelo Pereira
#	LSD is distributed under the GNU General Public License
#	
#**************************************************************

#**************************************************************
# ADD-SHORTCUT-MAC.SH
# Add a shortcut to LSD LMM in the macOS desktop.
# Also fixes some macOS security issues.
#**************************************************************

if [ "$1" = "-h" ]; then
	echo "Add a shortcut to LSD LMM in the desktop"
	echo "Usage: ./add-shortcut-mac.sh"
	exit 0
fi

# remove existing alias, if any
TARGET="LMM"
rm -f ~/Desktop/"$TARGET" ~/Desktop/"$TARGET.app" 
	
# disable macOS quarantine of LSD executables
LSDROOT="$( cd "$(dirname "${BASH_SOURCE[0]}" )" && pwd -P )"
sudo xattr -rd com.apple.quarantine "$LSDROOT/$TARGET.app"
sudo xattr -rd com.apple.quarantine "$LSDROOT/src/LSD.app"

# create alias on desktop
osascript >/dev/null <<END_SCRIPT
	tell application "Finder"
		make new alias to file (posix file "$LSDROOT/$TARGET.app") at desktop
	end tell
END_SCRIPT

# name of created alias
if [ -f ~/Desktop/"$TARGET.app" ]; then
	ALIAS="$TARGET.app"
else
	ALIAS="$TARGET"
fi

# fix alias icon
"$LSDROOT"/gnu/bin/seticon "$LSDROOT"/src/icons/lsd.icns ~/Desktop/"$ALIAS"
