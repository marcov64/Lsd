@ECHO off
rem *************************************************************
rem
rem	 LSD 9.0 - January 2026
rem	 written by Marco Valente, Universita' dell'Aquila
rem	 and by Marcelo Pereira, University of Campinas
rem
rem	 Copyright Marco Valente and Marcelo Pereira
rem	 LSD is distributed under the GNU General Public License
rem
rem	See Readme.txt for copyright information of
rem	third parties' code used in LSD
rem
rem *************************************************************

rem *************************************************************
rem KILL-LSD-TERM.BAT
rem Kill all LSD terminal processes running in background
rem *************************************************************

SET DEFAULT_PROC_NAME=lsd_term

IF "%1"=="/?" (
	ECHO Kill all LSD terminal processes running in background
	ECHO Usage: kill-lsd-term [process name with extension]
	GOTO END
)

IF "%1"=="" (SET PROC=%DEFAULT_PROC_NAME%.exe) else (SET PROC=%1)

TASKKILL /F /IM %PROC% > NUL

:END