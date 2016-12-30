@echo off
if "%1"=="/?" (
	echo Open Lsd LMM, 32-bit version
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
