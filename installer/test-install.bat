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
rem  TEST-INSTALL.BAT
rem  Try to install LSD in Windows.
rem *************************************************************

set LSD_FILE_TAG=9-0-beta-2

cd %USERPROFILE%
mkdir tmp
cd tmp
tar -xf C:\LSD-host\installer\LSD-installer-windows-%LSD_FILE_TAG%.zip
LSD-installer-windows-%LSD_FILE_TAG%.exe
