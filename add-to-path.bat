@echo off
rem *************************************************************
rem
rem	 LSD 7.1 - December 2018
rem	 written by Marco Valente, Universita' dell'Aquila
rem	 and by Marcelo Pereira, University of Campinas
rem
rem	 Copyright Marco Valente and Marcelo Pereira
rem	 LSD is distributed under the GNU General Public License
rem	
rem *************************************************************

rem *************************************************************
rem  ADD-TO-PATH.BAT
rem  Add a folder to the user PATH environment variable.
rem *************************************************************

if "%1"=="/?" (
	echo Add a folder to the user PATH environment variable
	echo Usage: add-to-path FOLDER
	goto end
)

if "%1"=="" (
	echo No FOLDER provided, aborting
	goto end
)

set CYGWIN_DIR=%1\bin
if not exist "%CYGWIN_DIR%" (
	echo Invalid FOLDER selected, aborting
	goto end
)

setlocal
set ok=0
for /f "skip=2 tokens=3*" %%a in ('reg query HKCU\Environment /v PATH') do if [%%b]==[] ( setx PATH "%%~a;%CYGWIN_DIR%" && set ok=1 ) else ( setx PATH "%%~a %%~b;%CYGWIN_DIR%" && set ok=1 )
if "%ok%" == "0" setx PATH "%CYGWIN_DIR%"
endlocal
:end
