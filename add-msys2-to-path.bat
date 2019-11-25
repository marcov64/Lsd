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

where x86_64-w64-mingw32-g++.exe >nul 2>&1

if "%1"=="" (
	if "%ERRORLEVEL%"=="0" (
		echo A C++ 64-bit compiler seems to be already in PATH, aborting
		pause
		goto end
	) else (
		if exist C:\msys64\mingw64\bin\x86_64-w64-mingw32-g++.exe (
			set MSYS_DIR=C:\msys64
			goto install
		)
	)
	echo No FOLDER provided or found, aborting
	pause
	goto end
) else (
	set MSYS_DIR="%1"
)

:install
set MSYS_DIR="%MSYS_DIR%\mingw64\bin"
if not exist "%MSYS_DIR%\" (
	echo Invalid FOLDER selected, aborting
	pause
	goto end
)

setlocal
set ok=0
for /f "skip=2 tokens=3*" %%a in ('reg query HKCU\Environment /v PATH') do if [%%b]==[] ( setx PATH "%%~a;%MSYS_DIR%" && set ok=1 ) else ( setx PATH "%%~a %%~b;%MSYS_DIR%" && set ok=1 )
if "%ok%" == "0" setx PATH "%MSYS_DIR%"
endlocal
:end
