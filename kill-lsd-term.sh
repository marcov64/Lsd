#!/bin/bash
#**************************************************************
#
#	LSD 9.0 - January 2026
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
# KILL-LSD-TERM.SH
# Kill all LSD terminal processes running in background
#**************************************************************

DEFAULT_PROC_NAME=lsd_term

if [[ "$1" == "-h" ]]; then
	echo "Kill all LSD terminal processes running in background"
	echo "Usage: ./kill-lsd-term.sh [process name with extension]"
	exit 0
fi

if [[ "$1" == "" ]]; then PROC="$DEFAULT_PROC_NAME"; else PROC="$1"; fi

PIDS=$(ps -A | grep $PROC | awk '{print $1}')

if [[ "$PIDS" == "" ]]; then
	echo "No process named '$PIDS' running"
else
	kill -9 $PIDS
fi
