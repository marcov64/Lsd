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
# UNINSTALL-MAC.SH
# Remove LSD files, folders and shortcuts.
#**************************************************************

if [[ "$1" = "-h" ]]; then
	echo "Remove LSD files and folders and unregister LSD from OS"
	echo "Usage: ./uninstall-mac.sh"
	exit 0
fi

LSDROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd -P )"

read -p "LSD installation at '$LSDROOT' is going to be removed. Proceed [Y,N]?" -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    exit 1
fi

LMMLNK="LSD Model Manager"
LMMAPP=LMM
LMMEXE="$LMMAPP.app/Contents/MacOS/LMM"
LSDAPP=LSD
LSDFOLDERS="Example gnu installer LMM.app lwi Manual Rpkg src"
LSDWORK=Work
DESKTOP="$( osascript \
             -e 'tell application "System Events"' \
             -e 'get POSIX path of (path to desktop folder from user domain)' \
             -e 'end tell' )"
APPHOME="$( osascript \
             -e 'tell application "System Events"' \
             -e 'get POSIX path of (path to applications folder from user domain)' \
             -e 'end tell' )"
			 
# remove desktop and start menu aliases
rm -f "$DESKTOP/$LMMLNK"
rm -f "$APPHOME/$LMMLNK"

# do not proceed if wrong directory
if [[ ! -f "$LSDROOT/$LMMEXE" ]]; then
	echo "LSD installation not found or corrupt, aborting!"
	read -p "Press any key to continue . . ."
	exit 2
fi

# remove folders
for f in $LSDFOLDERS; do
	if [[ -d "$LSDROOT/$f" ]]; then
		rm -rf "$LSDROOT/$f"
	fi
done

# remove Work if empty
if [[ -d "$LSDROOT/$LSDWORK" ]]; then
	COUNT=$( ls -l "$LSDROOT/$LSDWORK" | grep -c ^d )
else
	COUNT=1
fi

if [[ $COUNT = 0 ]]; then
	rm -rf "$LSDROOT/$LSDWORK"
fi

# remove main directory if empty or just files otherwise
COUNT=$( ls -l "$LSDROOT" | grep -c ^d )
if [[ $COUNT = 0 ]]; then
	rm -rf "${LSDROOT:-/tmp/__UNDEFINED__}"
else
	rm -f $( find "$LSDROOT" -maxdepth 1 -type f )
fi

exit 0
