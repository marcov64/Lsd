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
# ADD-SHORTCUT-LINUX.SH
# Add a shortcut to LSD LMM in the Linux Gnome desktop and apps.
#**************************************************************

if [[ "$1" = "-h" ]]; then
	echo "Add a shortcut to LSD LMM in the desktop"
	echo "Usage: ./add-shortcu-linuxt.sh [full path to desktop directory]"
	exit 0
fi

LMMLNK="LMM.desktop"
LMMEXE=LMM
LSDROOT="$( cd "$( dirname "${ BASH_SOURCE[0] }" )" && pwd -P )"
APPMENU="~/.local/share/applications"
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

# remove existing shortcuts
rm -f "$DESKTOP/$LMMLNK" "$DESKTOP/lsd.desktop" "$DESKTOP/LSD.desktop"
rm -f "$APPMENU/$LMMLNK" "$APPMENU/lsd.desktop" "$APPMENU/LSD.desktop"

# create the shortcuts with absolute paths
sed 's:$LSDROOT:'"$LSDROOT"':g' "$LSDROOT/$LMMLNK" > "$DESKTOP/$LMMLNK"
chmod +x "$DESKTOP/$LMMLNK"

# make shortcut clickable in newer GNOME
if command -v gio &> /dev/null; then
	dbus-launch gio set "$DESKTOP/$LMMLNK" "metadata::trusted" true > /dev/null 2>&1
fi

# also add icon to user window manager configuration
if [ ! -d "$APPMENU" ]; then
	mkdir -p "$APPMENU"
fi
cp -f "$DESKTOP/$LMMLNK" "$APPMENU/"

exit 0
