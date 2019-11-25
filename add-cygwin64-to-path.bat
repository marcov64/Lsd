@echo off
rem *************************************************************
rem
rem	 LSD 7.2 - December 2019
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
	echo Usage: add-to-path [FOLDER]
	goto end
)

where cygwin1.dll >nul 2>&1

if "%1"=="" (
	if "%ERRORLEVEL%"=="0" (
		echo Cygwin seems to be already in PATH, aborting
		pause
		goto end
	) else (
		if exist C:\cygwin64\bin\ (
			set CYGWIN_DIR=C:\cygwin64
			goto install
		)
		if exist C:\Windows\cygwin64\bin\ (
			set CYGWIN_DIR=C:\Windows\cygwin64
			goto install
		)
		if exist C:\cygwin\bin\ (
			set CYGWIN_DIR=C:\cygwin
			goto install
		)
	)
	echo No FOLDER provided or found, aborting
	pause
	goto end
) else (
	set CYGWIN_DIR="%1"
)

:install
set CYGWIN_DIR="%CYGWIN_DIR%\bin"
if not exist "%CYGWIN_DIR%\" (
	echo Invalid FOLDER selected, aborting
	pause
	goto end
)

setlocal
set ok=0
for /f "skip=2 tokens=3*" %%a in ('reg query HKCU\Environment /v PATH') do if [%%b]==[] ( setx PATH "%%~a;%CYGWIN_DIR%" && set ok=1 ) else ( setx PATH "%%~a %%~b;%CYGWIN_DIR%" && set ok=1 )
if "%ok%" == "0" setx PATH "%CYGWIN_DIR%"
endlocal
:end
