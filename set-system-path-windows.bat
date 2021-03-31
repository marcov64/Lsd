@ECHO off
rem *************************************************************
rem
rem	 LSD 8.0 - March 2021
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
rem  SET-SYSTEM-PATH-WINDOWS.BAT
rem  Add LSD libraries path to the Windows system-level PATH 
rem  This may be required when wrong/old versions of required
rem  libraries are already on the system PATH environment variable.
rem  To run this batch please open an administrative terminal
rem  or run it using the "Run as administrator" option in the
rem  Windows Explorer context menu (right click menu).
rem *************************************************************

IF "%1"=="/?" (
	ECHO Add LSD libraries path to the Windows system-level PATH variable
	ECHO ATTENTION: it must be run with administrator rights
	ECHO Usage: set-system-path-windows [FOLDER]
	GOTO end
)

rem check for admin privilege
fsutil dirty query %systemdrive% > nul

IF NOT %ERRORLEVEL% EQU 0 (
    ECHO This command require to be run as administrator, aborting!
	PAUSE > NUL
	GOTO end
)

if "%1"=="" (
	if exist "%~dp0gnu\bin\tcl86.dll" (
		set GNU_DIR="%~dp0gnu\bin"
		goto install
	)
) else (
	if exist "%1\tcl86.dll" (
		set GNU_DIR="%1"
		goto install
	)
)

echo No FOLDER provided or found, aborting
pause > nul
goto end

:install
setlocal
set ok=0
for /f "skip=2 tokens=3*" %%a in ('reg query "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v PATH') do (
	if [%%b]==[] ( 
		setx PATH "%GNU_DIR%;%%~a" /m > nul && set ok=1 
	) else ( 
		setx PATH "%GNU_DIR%;%%~a %%~b" /m > nul && set ok=1 
	)
)

if "%ok%" == "0" setx PATH "%GNU_DIR%" /m > nul
endlocal
:end
