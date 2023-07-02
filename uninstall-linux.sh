#!/bin/bash
#**************************************************************
#
#	LSD 8.1 - July 2023
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
# UNINSTALL-LINUX.SH
# Remove LSD files, folders and shortcuts.
#**************************************************************

if [[ "$1" = "-h" ]]; then
	echo "Remove LSD files and folders and unregister LSD from OS"
	echo "Usage: ./uninstall-linux.sh  [full path to desktop directory]"
	exit 0
fi

LSDROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd -P )"

read -p "LSD installation at '$LSDROOT' is going to be removed. Proceed [Y,N]?" -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    exit 1
fi

LMMLNK="LMM.desktop"
LMMEXE="LMM"
LSDFOLDERS="Example gnu installer LMM.app lwi Manual Rpkg src"
LSDWORK="Work"
APPMENU="$HOME/.local/share/applications"
if [ -x "$( command -v xdg-user-dir )" ]; then
	DESKTOP="$( xdg-user-dir DESKTOP )"
else
	if [ -z "$1" ]; then
		echo "Warning: cannot find desktop folder, assuming '$HOME/Desktop'"
		DESKTOP="$HOME/Desktop"
	else
		DESKTOP="$1"
	fi
fi

# remove desktop and start menu shortcuts
rm -f "$DESKTOP/$LMMLNK"
rm -f "$APPMENU/$LMMLNK"

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
