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
# ADD-SHORTCUT-LINUX.SH
# Add a shortcut to LSD LMM in the Linux Gnome desktop and apps.
#**************************************************************

if [ "$1" = "-h" ]; then
	echo "Add a shortcut to LSD LMM in the desktop"
	echo "Usage: ./add-shortcut.sh [full path to desktop directory]"
else
	TARGET=LSD.desktop
	EXEC=LMM
	LSDROOT="$( cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P )"
	if [ -x "$(command -v xdg-user-dir)" ]; then
  		DESKTOP="$(xdg-user-dir DESKTOP)"
  	else
  		if [ -z "$1" ]; then
  			echo "Warning: cannot find desktop folder, assuming '$HOME/Desktop'"
  			DESKTOP="$HOME/Desktop"
  		else
			DESKTOP="$1"
  		fi
	fi
	
	# create the shortcuts with absolute paths
	sed 's:$LSDROOT:'"$LSDROOT"':g' "$LSDROOT/$TARGET" > "$DESKTOP"/"$TARGET"
	chmod +x "$DESKTOP"/"$TARGET"
	chmod +x "$LSDROOT/$EXEC"
	
	# also add icon to user window manager configuration
	if [ ! -d "~/.local/share/applications" ]; then
		mkdir -p ~/.local/share/applications
	fi
	cp -f "$DESKTOP"/"$TARGET" ~/.local/share/applications
fi
