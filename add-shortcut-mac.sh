#!/bin/bash
#**************************************************************
#
#	LSD 8.0 - May 2022
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

if [[ "$1" = "-h" ]]; then
	echo "Add a shortcut to LSD LMM in the desktop"
	echo "Usage: ./add-shortcut-mac.sh"
	exit 0
fi

LMMLNK="LSD Model Manager"
LSDROOT="$( cd "$( dirname "${ BASH_SOURCE[0] }" )" && pwd -P )"
LMMAPP=LMM
LSDAPP=LSD
DESKTOP="$( osascript \
             -e 'tell application "System Events"' \
             -e 'get POSIX path of (path to desktop folder from user domain)' \
             -e 'end tell' )"
APPHOME="$( osascript \
             -e 'tell application "System Events"' \
             -e 'get POSIX path of (path to applications folder from user domain)' \
             -e 'end tell' )"


# remove existing aliases, if any
rm -f "$DESKTOP/$LMMAPP" "$DESKTOP/$LMMAPP.app" "$DESKTOP/$LMMLNK"
rm -f "$APPHOME/$LMMAPP" "$APPHOME/$LMMAPP.app" "$APPHOME/$LMMLNK"

# disable macOS quarantine of LSD executables
sudo xattr -rd com.apple.quarantine "$LSDROOT/$LMMAPP.app"
sudo xattr -rd com.apple.quarantine "$LSDROOT/src/$LSDAPP.app"

# create alias on desktop
osascript >/dev/null <<END_SCRIPT
	tell application "Finder"
		set appHome to path to applications folder from user domain
		make new alias to file (posix file "$LSDROOT/$LMMAPP.app") at appHome with properties {name:"$LMMLNK"}
		make new alias to file (posix file "$LSDROOT/$LMMAPP.app") at desktop with properties {name:"$LMMLNK"}
	end tell
END_SCRIPT

exit 0
