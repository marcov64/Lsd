#!/bin/bash
if [ "$1" = "-h" ]; then
	echo "Add a shortcut to Lsd LMM in the desktop"
	echo "Usage: ./add-shortcut.sh [full path to desktop directory]"
else
	TARGET=lsd.desktop
	EXEC=lmm
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
	sed 's:$LSDROOT:'"$LSDROOT"':g' "$LSDROOT/$TARGET" > "$DESKTOP"/"$TARGET"
	chmod +x "$DESKTOP"/"$TARGET"
	chmod +x "$LSDROOT/$EXEC"
	
	# also add icon to user window manager configuration
	if [ ! -d "~/.local/share/applications" ]; then
		mkdir -p ~/.local/share/applications
	fi
	cp -f "$DESKTOP"/"$TARGET" ~/.local/share/applications
fi

