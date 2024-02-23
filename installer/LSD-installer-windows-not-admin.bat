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
rem  LSD-INSTALLER-WINDOWS-NOT-ADMIN.BAT
rem  Run the LSD installer as a non-administrative user,
rem  preventing the need of admin rights.
rem *************************************************************

SET VERSION=8-1-stable-3

IF "%1"=="/?" (
	ECHO Install LSD as non-admin user
	ECHO Usage: windows-run-non-admin [NAME_OF_INSTALLER]
	GOTO end
)

IF "%1"=="" (
	SET INSTALLER=LSD-installer-windows-%VERSION%.exe
) ELSE (
	SET INSTALLER="%1"
)

CMD /MIN /C "set __COMPAT_LAYER=RUNASINVOKER && start "" %INSTALLER%"

:end