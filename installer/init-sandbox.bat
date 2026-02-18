@echo off
rem *************************************************************
rem
rem	 LSD 9.0 - January 2026
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
rem  INIT-SANDBOX.BAT
rem  Initialize Windows Sandbox and launch LSD installer.
rem *************************************************************

set LSD_FILE_TAG=9-0-beta-2
set INSTALL_BAT=test-install.bat
set TEST_USER=test
set USER_PWD=pwd

net user %TEST_USER% %USER_PWD% /add
echo Password for %TEST_USER%: %USER_PWD%
runas /user:test C:\LSD-host\installer\%INSTALL_BAT%
