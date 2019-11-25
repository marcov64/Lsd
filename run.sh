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
# RUN.SH
# LMM terminal startup command in Linux and macOS.
#**************************************************************

if [ "$1" = "-h" ]; then
	echo "Open LSD LMM, 64-bit version"
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
