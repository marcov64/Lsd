@echo off
if "%1"=="/?" (
	echo Open Lsd LMM, 64-bit version
	echo Usage: run64 [LSDROOT] [FILE]
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
path %LSDROOT%\gnu64\bin;%PATH%;%LSDROOT%\gnu\bin
start lmm64.exe %2
:end
