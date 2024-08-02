@ECHO off
rem *************************************************************
rem
rem	 LSD 9.0 - January 2024
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
rem LOG-MONITOR.SH
rem Open multiple log windows
rem *************************************************************

IF "%1"=="/?" (
	ECHO Open multiple log windows on parallel run folders
	ECHO Usage: log-monitor [path to log directory] [log name extension]
	GOTO end
)

IF "%1"=="" (SET DIR=.) else (SET DIR=%1)
IF "%2"=="" (SET FILES=*.log) else (SET FILES=*%2)

FOR /F "tokens=* USEBACKQ" %%F IN (`DIR /a-d "%DIR%\%FILES%" ^| FIND /C "/"`) DO (
	SET N=%%F
)

IF %N% GTR 30 (
	SET COLS=6
) ELSE (
	IF %N% GTR 20 (
		SET COLS=5
	) ELSE (
		IF %N% GTR 12 (
			SET COLS=4
		) ELSE (
			IF %N% GTR 8 (
				SET COLS=3
			) ELSE (
				IF %N% GTR 4 (
					SET COLS=2
				) ELSE (
					SET COLS=1
				)
			)
		)
	)
)

IF "%COLS%"=="1" (
	multitail --basename -P r -Ec "Finished processing .*" -i %DIR%/%FILES%
) else (
	multitail --basename -P r -Ec "Finished processing .*" -s %COLS% -i %DIR%/%FILES%
)