@ECHO off
rem *************************************************************
rem
rem	 LSD 8.0 - May 2022
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
rem  SET-USER-PATH-WINDOWS.BAT
rem  Add LSD libraries path to the Windows user-level PATH 
rem  This may be required when wrong/old versions of required
rem  libraries are already on the user PATH environment variable.
rem  To run this batch please open a terminal
rem  or run it using the "Run" option in the
rem  Windows Explorer context menu (right click menu).
rem *************************************************************

IF "%1"=="/?" (
	ECHO Add LSD libraries path to the Windows user-level PATH variable
	ECHO Usage: set-user-path-windows [FOLDER]
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
for /f "skip=2 tokens=3*" %%a in ('reg query "HKCU\Environment" /v PATH') do (
	if [%%b]==[] ( 
		setx PATH "%GNU_DIR%;%%~a" > nul && set ok=1 
	) else ( 
		setx PATH "%GNU_DIR%;%%~a %%~b" > nul && set ok=1 
	)
)

if "%ok%" == "0" setx PATH "%GNU_DIR%" > nul
endlocal
:end
