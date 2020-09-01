@echo off
rem *************************************************************
rem
rem	 LSD 7.3 - December 2020
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
rem  CREATE-INSTALLER-WINDOWS.BAT
rem  Create LSD installer for Windows.
rem *************************************************************

set LSD_VER=7-3-beta

if "%1"=="/?" (
	echo Create LSD installer for Windows
	echo Usage: create-installer-windows [LSD FOLDER]
	goto end
)

if "%1"=="" (
	if exist ..\lmm.exe (
		set LSD_DIR=..
		goto create
	)
	echo No LSD FOLDER provided or found, aborting
	pause
	goto end
) else (
	if exist "%1"\lmm.exe (
		set LSD_DIR="%1"
		goto create
	)
	echo Provided LSD FOLDER not found, aborting
	pause
	goto end
)

:create

set INST_DIR=%LSD_DIR%\installer

del %INST_DIR%\LSD-archive-windows.7z >nul 2>&1
del %INST_DIR%\LSD-installer-windows-%LSD_VER%.exe >nul 2>&1

%INST_DIR%\7zr.exe a %INST_DIR%\LSD-archive-windows.7z %LSD_DIR%\* -x@exclude-installer-windows.txt -r -mx -mf=BCJ2

copy /b %INST_DIR%\7zSD.sfx + %INST_DIR%\config-installer-windows.txt + %INST_DIR%\LSD-archive-windows.7z %INST_DIR%\LSD-installer-windows-%LSD_VER%.exe  >nul 2>&1

del %INST_DIR%\LSD-archive-windows.7z >nul 2>&1
echo .

if exist %INST_DIR%\LSD-installer-windows-%LSD_VER%.exe (
	echo Self-extracting LSD package created: %INST_DIR%\LSD-installer-windows-%LSD_VER%.exe
) else (
	echo Error creating LSD installer
)

:end