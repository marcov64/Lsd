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
rem  RUN.BAT
rem  LMM startup command in Windows 32-bit.
rem *************************************************************

if "%1"=="/?" (
	echo Open LSD LMM, 32-bit version
	echo Usage: run [LSDROOT] [FILE]
	goto end
)
if not "%1"=="" (
	if defined LSDROOT (
		if not "%LSDROOT%"=="%1" (
			echo LSDROOT is already set to the path '%LSDROOT%'. Confirm change [Y/N]? 
			choice /c yn /n
			if ERRORLEVEL==2 goto end
		)
	)
	set LSDROOT=%1
	setx LSDROOT %1 > NUL
) else (
	set LSDROOT=%CD%
)
path %LSDROOT%\gnu\bin;%PATH%
start lmm.exe %2
:end
