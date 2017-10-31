#!/bin/bash
if [ "$1" = "-h" ]; then
	echo "Add a shortcut to Lsd LMM in the desktop"
	echo "Usage: ./add-shortcut.sh [32]"
else
	if [ "$1" == "32" ]; then
		TARGET=lsd32.desktop
		EXEC=lmm32
	else
		TARGET=lsd.desktop
		EXEC=lmm
	fi
	LSDROOT="$( cd "$(dirname "${BASH_SOURCE[0]}" )" && pwd -P )"
	sed 's:$LSDROOT:'"$LSDROOT"':g' "$LSDROOT/$TARGET" > ~/Desktop/"$TARGET"
	chmod +x ~/Desktop/"$TARGET"
	chmod +x "$LSDROOT/$EXEC"
	
	# also add icon to user window manager configuration
	if [ ! -d "~/.local/share/applications" ]; then
		mkdir -p ~/.local/share/applications
	fi
	cp -f ~/Desktop/"$TARGET" ~/.local/share/applications
fi

