#!/bin/bash
if [ "$1" = "-h" ]; then
	echo "Add a shortcut to Lsd LMM in the desktop"
	echo "Usage: ./add-shortcut-mac.sh"
	exit 0
fi
TARGET="LMM"
LSDROOT="$( cd "$(dirname "${BASH_SOURCE[0]}" )" && pwd -P )"
osascript >/dev/null <<END_SCRIPT
	tell application "Finder"
		make new alias to file (posix file "$LSDROOT/$TARGET.app") at desktop
	end tell
END_SCRIPT
if [ -f ~/Desktop/"$TARGET.app" ]; then
	ALIAS="$TARGET.app"
else
	ALIAS="$TARGET"
fi
"$LSDROOT"/gnu/bin/SetFileIcon -file ~/Desktop/"$ALIAS" -image "$LSDROOT"/src/icons/lsd.icns
