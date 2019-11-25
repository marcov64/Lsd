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
rem  ADD-SHORTCUT64.BAT
rem  Add a shortcut to LSD LMM in the Windows 64-bit desktop.
rem *************************************************************

if "%1"=="/?" (
	echo Add a shortcut to LSD LMM in the desktop
	echo Usage: add-shortcut [32]
	goto end
)
if "%1"=="32" (
	"%CD%\gnu\bin\Shortcut.exe" /f:"%USERPROFILE%\Desktop\LMM.lnk" /a:c /t:"%CD%\run.bat" /w:%CD% /r:7 /i:%CD%\src\icons\lsd.ico /d:"Lsd Model Manager"
) else (
	"%CD%\gnu\bin\Shortcut.exe" /f:"%USERPROFILE%\Desktop\LMM (64-bit).lnk" /a:c /t:"%CD%\run64.bat" /w:%CD% /r:7 /i:%CD%\src\icons\lsd.ico /d:"Lsd Model Manager 64-bit"
)
:end
