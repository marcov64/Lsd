@echo off
rem *************************************************************
rem
rem	 LSD 7.3 - December 2020
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
rem  RUN.BAT
rem  LMM startup command in Windows.
rem *************************************************************

if "%1"=="/?" (
	echo Open LSD LMM
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
