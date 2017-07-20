#!/bin/bash
if [ "$1" = "-h" ]; then
	echo "Open Lsd LMM, 64-bit version"
	echo "Usage: ./run.sh [LSDROOT] [FILE]"
else
	if [ -n "$1" ]; then
		if [ -n "$LSDROOT" ]; then
			if [ "$LSDROOT" != "$1" ]; then
				echo "LSDROOT is already set to the path '$LSDROOT'. Confirm change [Y/N]?"
				read -n 1 ANS
				if [ "$ANS" = "Y" ] ||  [ "$ANS" = "y" ]; then
					LSDROOT="$1"
					export LSDROOT
				fi
			fi
		else
			LSDROOT="$1"
			export LSDROOT
		fi
	else
		if [ -z "$LSDROOT" ]; then
			LSDROOT="$( cd "$(dirname "${BASH_SOURCE[0]}" )" && pwd -P )"
			export LSDROOT			
		fi
	fi
	"$LSDROOT"/lmm $2
fi
