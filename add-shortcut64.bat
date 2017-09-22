@echo off
if "%1"=="/?" (
	echo Add a shortcut to Lsd LMM in the desktop
	echo Usage: add-shortcut [32]
	goto end
)
if "%1"=="32" (
	"%CD%\gnu\bin\Shortcut.exe" /f:"%USERPROFILE%\Desktop\LMM.lnk" /a:c /t:"%CD%\run.bat" /w:%CD% /r:7 /i:%CD%\src\icons\lsd.ico /d:"Lsd Model Manager"
) else (
	"%CD%\gnu\bin\Shortcut.exe" /f:"%USERPROFILE%\Desktop\LMM (64-bit).lnk" /a:c /t:"%CD%\run64.bat" /w:%CD% /r:7 /i:%CD%\src\icons\lsd.ico /d:"Lsd Model Manager 64-bit"
)
:end
