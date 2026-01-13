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
rem  ADD-SHORTCUT-WINDOWS.BAT
rem  Add a shortcut to LSD LMM in the Windows desktop and
rem  Start menu program list, at the user or system level.
rem *************************************************************

IF "%1"=="/?" (
	ECHO Add a shortcut to LSD LMM in the desktop
	ECHO Usage: add-shortcut-windows [/s]
	ECHO Option: /s - install for all users
	GOTO end
)

rem check for admin privilege
SET ADMIN=0
NET session >NUL 2>&1
IF %ERRORLEVEL%==0 SET ADMIN=1

rem get locations
IF /I "%1"=="/s" (
	IF %ADMIN%==0 (
		ECHO This command with /s option requires to be run as administrator, aborting!
		PAUSE
		GOTO end
	)

	SET REGLOC="HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders"
) ELSE (
	IF %ADMIN%==1 (
		ECHO This command cannot be run as administrator without /s option, aborting!
		PAUSE
		GOTO end
	)

	SET REGLOC="HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\User Shell Folders"
)

FOR /F "tokens=2*" %%A IN ('REG QUERY %REGLOC% /v Desktop 2^>^&1^|find "REG_"') DO SET DESKTOP=%%B
FOR /F "tokens=2*" %%A IN ('REG QUERY %REGLOC% /v Programs 2^>^&1^|find "REG_"') DO SET STRTMENU=%%B

rem remove old shortcuts
ERASE /F "%DESKTOP%\LSD*.lnk" > NUL 2>&1
ERASE /F "%DESKTOP%\LMM*.lnk" > NUL 2>&1
ERASE /F "%STRTMENU%\LSD*.lnk" > NUL 2>&1
ERASE /F "%STRTMENU%\LMM*.lnk" > NUL 2>&1

rem create shortcuts in desktop and the user menu
"%CD%\gnu\bin\Shortcut.exe" /f:"%DESKTOP%\LSD Model Manager.lnk" /a:c /t:"%CD%\LMM.exe" /w:%CD% /r:7 /i:%CD%\src\icons\lmm.ico /d:"LSD Model Manager" > NUL

COPY "%DESKTOP%\LSD Model Manager.lnk" "%STRTMENU%" > NUL

:end
