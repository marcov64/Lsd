@echo off
rem *************************************************************
rem
rem	 LSD 8.1 - July 2023
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

set LSD_FILE_TAG=8-1-stable-3

if "%1"=="/?" (
	echo Create LSD installer for Windows
	echo Usage: create-installer-windows [LSD FOLDER]
	goto end
)

if "%1"=="" (
	if exist ..\LMM.exe (
		set LSD_DIR=..
		goto create
	)
	echo No LSD FOLDER provided or found, aborting
	pause > nul
	goto end
) else (
	if exist "%1"\LMM.exe (
		set LSD_DIR="%1"
		goto create
	)
	echo Provided LSD FOLDER not found, aborting
	pause > nul
	goto end
)

:create

set README=%LSD_DIR%\Readme.txt
set INST_DIR=%LSD_DIR%\installer
set FILENAME=%INST_DIR%\LSD-installer-windows

rem create installer executable
del %FILENAME%.7z >nul 2>&1
del %FILENAME%-%LSD_FILE_TAG%.exe >nul 2>&1
7z a -r -mx=9 %FILENAME%.7z %LSD_DIR%\* -x@exclude-installer-windows.txt
copy /b %INST_DIR%\7zSD.sfx + %INST_DIR%\config-installer-windows.txt + %FILENAME%.7z %FILENAME%-%LSD_FILE_TAG%.exe  >nul 2>&1

rem create compressed distribution file
del %FILENAME%-%LSD_FILE_TAG%.zip >nul 2>&1
7z a -tzip -mx=1 %FILENAME%-%LSD_FILE_TAG%.zip %README% %FILENAME%-%LSD_FILE_TAG%.exe

rem cleanup
del %FILENAME%.7z >nul 2>&1
del %FILENAME%-%LSD_FILE_TAG%.exe >nul 2>&1
echo .

if exist %FILENAME%-%LSD_FILE_TAG%.zip (
	echo LSD installer package created: %FILENAME%-%LSD_FILE_TAG%.zip
) else (
	echo Error creating LSD installer
)

:end
