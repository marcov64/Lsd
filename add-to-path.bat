@echo off
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
