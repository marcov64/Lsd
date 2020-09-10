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
rem  REMOVE-TOOLS.BAT
rem  Remove tools which may be alredy installed in MSYS2/Cygwin:
rem  - MSYS2/Cygwin libraries
rem  - make utility (Win32 with embedded linux shell)
rem  - gcc compiler
rem  - gdb debugger
rem  - diff compare tool
rem  - multitail tool
rem *************************************************************

if "%1"=="/?" (
	echo Remove developer tools from LSD
	echo Usage: remove-tools [LSD FOLDER]
	goto end
)

if "%1"=="" (
	if exist ..\lmm.exe (
		set GNU_DIR=%CD%
		goto remove
	)
	echo No LSD FOLDER provided or found, aborting
	pause
	goto end
) else (
	set GNU_DIR="%1\gnu"
)

:remove

set OPT=/F/S/Q

rem preserve files not in MSYS2/Cygwin
MOVE %GNU_DIR%\bin\Shortcut.exe %GNU_DIR%\
MOVE %GNU_DIR%\bin\Tail.exe %GNU_DIR%\
MOVE %GNU_DIR%\bin\tclsh86.exe %GNU_DIR%\
MOVE %GNU_DIR%\bin\wish86.exe %GNU_DIR%\
MOVE %GNU_DIR%\bin\tcl86.dll %GNU_DIR%\
MOVE %GNU_DIR%\bin\tk86.dll %GNU_DIR%\
MOVE %GNU_DIR%\bin\zlib.dll %GNU_DIR%\

rem executables (gcc, gdb, diff, multitail)
ERASE %OPT% %GNU_DIR%\bin\*.*

rem restore files not in MSYS2/Cygwin
MOVE %GNU_DIR%\*.exe %GNU_DIR%\bin\
MOVE %GNU_DIR%\*.dll %GNU_DIR%\bin\

rem include files (gcc)
ERASE %OPT% %GNU_DIR%\include\c++

rem library files (gcc, gdb)
ERASE %OPT% %GNU_DIR%\lib\gcc
ERASE %OPT% %GNU_DIR%\lib\python*.*

rem other files (gcc, gdb, multitail)
ERASE %OPT% %GNU_DIR%\share 
ERASE %OPT% %GNU_DIR%\x86_64-w64-mingw32 
ERASE %OPT% %GNU_DIR%\etc 
ERASE %OPT% %GNU_DIR%\usr

:end
