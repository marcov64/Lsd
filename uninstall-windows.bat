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
rem  UNINSTALL-WINDOWS.BAT
rem  Remove LSD files, folders and shortcuts
rem  and unregister LSD from OS.
rem *************************************************************

IF "%1"=="/?" (
	ECHO Remove LSD files and folders and unregister LSD from OS
	ECHO Usage: uninstall-windows [/s]
	ECHO Option: /s - silent uninstall
	GOTO end
)

SET LSDROOT=%~dp0
SET LSDROOT=%LSDROOT:~0,-1%
SET GETFOLDER=%LSDROOT%\src\get-dir-windows.vbs
SET LSDFOLDERS=Example gnu installer LMM.app lwi Manual Rpkg src tmp
SET LSDWORK=Work

IF /I "%1"=="/s" GOTO nocheck

CHOICE /M "LSD installation at '%LSDROOT%' is going to be removed. Proceed"
IF NOT %ERRORLEVEL%==1 GOTO end

:nocheck

rem check for admin privilege
SET ADMIN=0
NET session >NUL 2>&1
IF %ERRORLEVEL%==0 SET ADMIN=1

rem remove LSD from system PATH
SET REGLOC="HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment"
FOR /F "tokens=2*" %%A IN ('REG QUERY %REGLOC% /v PATH 2^>^&1^|find "REG_"') DO SET SYSPATH=%%B

CALL SET NEWPATH=%%SYSPATH:%LSDROOT%\gnu\bin=%%
IF NOT "%NEWPATH%"=="%SYSPATH%" (

	IF %ADMIN%==0 (
		ECHO This command requires to be run as administrator, aborting!
		PAUSE
		GOTO end
	)

	SETX /M PATH "%NEWPATH%" > nul
)

rem remove LSD from user PATH
SET REGLOC="HKCU\Environment"
FOR /F "tokens=2*" %%A IN ('REG QUERY %REGLOC% /v PATH 2^>^&1^|find "REG_"') DO SET USRPATH=%%B

CALL SET NEWPATH=%%USRPATH:%LSDROOT%\gnu\bin=%%
IF NOT "%NEWPATH%"=="%USRPATH%" SETX PATH "%NEWPATH%" > nul

rem remove desktop and start menu links, unregister from Windows
SET REGLOC="HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\User Shell Folders"
FOR /F "tokens=2*" %%A IN ('REG QUERY %REGLOC% /v Desktop 2^>^&1^|find "REG_"') DO SET DESKTOP=%%B
FOR /F "tokens=2*" %%A IN ('REG QUERY %REGLOC% /v Programs 2^>^&1^|find "REG_"') DO SET STRTMENU=%%B
ERASE /F "%DESKTOP%\LSD Model Manager.lnk" > NUL 2>&1
ERASE /F "%STRTMENU%\LSD Model Manager.lnk" > NUL 2>&1
REG DELETE "HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\LSD" /f > NUL 2>&1

SET REGLOC="HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders"
FOR /F "tokens=2*" %%A IN ('REG QUERY %REGLOC% /v Desktop 2^>^&1^|find "REG_"') DO SET DESKTOP=%%B
FOR /F "tokens=2*" %%A IN ('REG QUERY %REGLOC% /v Programs 2^>^&1^|find "REG_"') DO SET STRTMENU=%%B

IF %ADMIN%==1 (
	ERASE /F "%DESKTOP%\LSD Model Manager.lnk" > NUL 2>&1
	ERASE /F "%STRTMENU%\LSD Model Manager.lnk" > NUL 2>&1
	REG DELETE "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\LSD" /f > NUL 2>&1
)

rem do not proceed if wrong directory
IF NOT EXIST "%LSDROOT%\LMM.exe" (
	ECHO LSD installation not found or corrupt, aborting!
	PAUSE
	GOTO end
)

rem remove folders
FOR %%f IN (%LSDFOLDERS%) DO IF EXIST "%LSDROOT%\%%f" RMDIR /S /Q "%LSDROOT%\%%f"

rem remove Work if empty
IF EXIST "%LSDROOT%\%LSDWORK%" (
	FOR /F %%f IN ('DIR /b /ad "%LSDROOT%\%LSDWORK%"^|find /c /v "" ') DO SET COUNT=%%f
) ELSE (
	SET COUNT=-1
)

IF %COUNT%==0 RMDIR /S /Q "%LSDROOT%\%LSDWORK%"

rem remove main directory if empty or just files otherwise
FOR /F %%f IN ('DIR /b /ad "%LSDROOT%"^|find /c /v "" ') DO SET COUNT=%%f
IF %COUNT%==0 (
	IF /I "%LSDROOT%"=="%CD%" CD ..
	(GOTO) 2>NUL & RMDIR /S /Q "%LSDROOT%"
) ELSE (
	(GOTO) 2>NUL & ERASE /F /Q "%LSDROOT%\*.*"
)

:end
