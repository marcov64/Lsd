@echo off
rem *************************************************************
rem
rem	 LSD 8.0 - December 2020
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
rem  CYGWIN-IMPORT.BAT
rem  Copy required files from MSYS2 installation including:
rem  - Cygwin libraries
rem  - multitail utility
rem *************************************************************

rem XCOPY options for files and directories
set OPT=/D/Q/Y
set XOPT=%OPT%/S

if "%1"=="/?" (
	echo Import Cygwin developer tools to LSD
	echo Usage: Cygwin-import [CYGWIN FOLDER] [LSD FOLDER]
	goto end
)

if "%1"=="" (
	if exist C:\cygwin64\bin\multitail.exe (
		set CYG_DIR=C:\cygwin64
		goto lsd_path
	)
	echo No CYGWIN FOLDER provided or found, aborting
	pause
	goto end
) else (
	set CYG_DIR="%1"
)

:lsd_path

if "%2"=="" (
	if exist ..\lmm.exe (
		set LSD_DIR=..
		goto import
	)
	echo No LSD FOLDER provided or found, aborting
	pause
	goto end
) else (
	set LSD_DIR="%2"
)

:import

echo Cygwin libraries...
XCOPY %OPT% %CYG_DIR%\bin\cygwin1.dll %LSD_DIR%\gnu\bin\
XCOPY %OPT% %CYG_DIR%\bin\cygiconv-2.dll %LSD_DIR%\gnu\bin\
XCOPY %OPT% %CYG_DIR%\bin\cygintl-8.dll %LSD_DIR%\gnu\bin\
XCOPY %OPT% %CYG_DIR%\bin\cygncursesw-10.dll %LSD_DIR%\gnu\bin\
XCOPY %OPT% %CYG_DIR%\bin\cygpanelw-10.dll %LSD_DIR%\gnu\bin\

echo multitail utility...
XCOPY %OPT% %CYG_DIR%\bin\tail.exe %LSD_DIR%\gnu\bin\
XCOPY %OPT% %CYG_DIR%\bin\multitail.exe %LSD_DIR%\gnu\bin\
XCOPY %XOPT% %CYG_DIR%\usr\share\terminfo\78 %LSD_DIR%\gnu\usr\share\terminfo\78\

echo done copying files

:end
