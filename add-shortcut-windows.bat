@echo off
rem *************************************************************
rem
rem	 LSD 8.0 - December 2020
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
rem  ADD-SHORTCUT-WINDOWS.BAT
rem  Add a shortcut to LSD LMM in the Windows desktop and
rem  Start menu program list.
rem *************************************************************

if "%1"=="/?" (
	echo Add a shortcut to LSD LMM in the desktop
	echo Usage: add-shortcut-windows
	goto end
)

"%CD%\gnu\bin\Shortcut.exe" /f:"%USERPROFILE%\Desktop\LSD.lnk" /a:c /t:"%CD%\LMM.exe" /w:%CD% /r:7 /i:%CD%\src\icons\lmm.ico /d:"LSD Model Manager" > nul

copy "%USERPROFILE%\Desktop\LSD.lnk" "%AppData%\Microsoft\Windows\Start Menu\Programs" > nul

:end
