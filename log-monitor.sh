#!/bin/bash
#**************************************************************
#
#	LSD 9.0 - January 2024
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
# LOG-MONITOR.SH
# Open multiple log windows on parallel run folders
#**************************************************************

if [[ "$1" = "-h" ]]; then
	echo "Open multiple log windows on parallel run folders"
	echo "Usage: ./log-monitor.sh [path to log directory] [log name extension]"
	exit 0
fi

if [[ "$1" == "" ]]; then DIR="."; else DIR="$1"; fi
if [[ "$2" == "" ]]; then FILES="*.log"; else FILES="*$2"; fi

if [[ "$OSTYPE" == "darwin" ]]; then
	N=$(find $DIR -maxdepth 1 -name "$FILES" -exec stat -f "." {} \; | wc -l)
else
	N=$(find $DIR -maxdepth 1 -name "$FILES" -printf "." | wc -m)
fi

if [[ $N > 30 ]]; then
	COLS=6
elif [[ $N > 20 ]]; then
	COLS=5
elif [[ $N > 12 ]]; then
	COLS=4
elif [[ $N > 8 ]]; then
	COLS=3
elif [[ $N > 4 ]]; then
	COLS=2
else
	COLS=1
fi

if [[ "$COLS" == "1" ]]; then
	multitail --retry-all --basename -P r -Ec "Finished processing .*" -i $DIR/$FILES
else
	multitail --retry-all --basename -P r -Ec "Finished processing .*" -s $COLS -i $DIR/$FILES
fi
