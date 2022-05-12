@ECHO off
rem *************************************************************
rem
rem	 LSD 8.0 - May 2022
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

IF "%1"=="/?" (
	ECHO Add a shortcut to LSD LMM in the desktop
	ECHO Usage: add-shortcut-windows
	GOTO end
)

rem remove existing shortcuts
ERASE /F "%USERPROFILE%\Desktop\LSD.lnk" > NUL 2>&1
ERASE /F "%USERPROFILE%\Desktop\LMM.lnk" > NUL 2>&1
ERASE /F "%APPDATA%\Microsoft\Windows\Start Menu\Programs\LSD.lnk" > NUL 2>&1
ERASE /F "%APPDATA%\Microsoft\Windows\Start Menu\Programs\LMM.lnk" > NUL 2>&1

rem create shortcuts in desktop and the user menu
"%CD%\gnu\bin\Shortcut.exe" /f:"%USERPROFILE%\Desktop\LSD Model Manager.lnk" /a:c /t:"%CD%\LMM.exe" /w:%CD% /r:7 /i:%CD%\src\icons\lmm.ico /d:"LSD Model Manager" > NUL 

COPY "%USERPROFILE%\Desktop\LSD Model Manager.lnk" "%APPDATA%\Microsoft\Windows\Start Menu\Programs" > NUL

:end
