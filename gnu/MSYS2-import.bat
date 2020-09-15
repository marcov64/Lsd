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
rem  MSYS2-IMPORT.BAT
rem  Copy required files from MSYS2 installation including:
rem  - MSYS2 libraries
rem  - make utility (Win32 with embedded linux shell)
rem  - gcc compiler
rem  - Tcl/Tk framework
rem  - gdb debugger
rem  - diff compare tool
rem *************************************************************

if "%1"=="/?" (
	echo Import MSYS2 developer tools to LSD
	echo Usage: MSYS2-import [MSYS2 FOLDER] [LSD FOLDER]
	goto end
)

if "%1"=="" (
	if exist C:\msys64\mingw64\bin\x86_64-w64-mingw32-g++.exe (
		set MSYS_DIR=C:\msys64
		goto lsd_path
	)
	echo No MSYS2 FOLDER provided or found, aborting
	pause
	goto end
) else (
	set MSYS_DIR="%1"
)

:lsd_path

if "%2"=="" (
	if exist ..\lmm.exe (
		set LSD_DIR=..
		goto import
	)
	echo No LSD FOLDER provided or found, aborting
	pause
	goto end
) else (
	set LSD_DIR="%2"
)

:import

set GCC_VER=10.2.0
set OPT=/D/Q/Y
set XOPT=%OPT%/S

rem MSYS2 libraries and make utility
XCOPY %OPT% %MSYS_DIR%\usr\bin\make.exe %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\usr\bin\rm.exe %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\usr\bin\msys-2.0.dll %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\usr\bin\msys-intl-8.dll %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\usr\bin\msys-iconv-2.dll %LSD_DIR%\gnu\bin\

rem g++ compiler
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\gcc.exe %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\g++.exe %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\windres.exe %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\libwinpthread-1.dll %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\libgcc_s_seh-1.dll %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\libgmp-10.dll %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\libstdc++-6.dll %LSD_DIR%\gnu\bin\
XCOPY %XOPT% %MSYS_DIR%\mingw64\include\c++ %LSD_DIR%\gnu\include\c++\
XCOPY %XOPT% %MSYS_DIR%\mingw64\share\gcc-%GCC_VER% %LSD_DIR%\gnu\share\gcc-%GCC_VER%\
XCOPY %XOPT% %MSYS_DIR%\mingw64\x86_64-w64-mingw32 %LSD_DIR%\gnu\x86_64-w64-mingw32\

rem zlib library
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\libzstd.dll %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\zlib1.dll %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\mingw64\include\zlib.h %LSD_DIR%\gnu\include\
XCOPY %OPT% %MSYS_DIR%\mingw64\include\zconf.h %LSD_DIR%\gnu\include\
XCOPY %OPT% %MSYS_DIR%\mingw64\lib\libz.* %LSD_DIR%\gnu\lib\

rem Tcl/Tk framework
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\tclsh86.exe %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\wish86.exe %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\tcl86.dll %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\tk86.dll %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\mingw64\include\tcl*.* %LSD_DIR%\gnu\include\
XCOPY %OPT% %MSYS_DIR%\mingw64\include\tk*.* %LSD_DIR%\gnu\include\
XCOPY %XOPT% %MSYS_DIR%\mingw64\include\X11 %LSD_DIR%\gnu\include\X11\
XCOPY %OPT% %MSYS_DIR%\mingw64\lib\libtcl*.* %LSD_DIR%\gnu\lib\
XCOPY %OPT% %MSYS_DIR%\mingw64\lib\libtk*.* %LSD_DIR%\gnu\lib\
XCOPY %OPT% %MSYS_DIR%\mingw64\lib\tcl*.* %LSD_DIR%\gnu\lib\
XCOPY %OPT% %MSYS_DIR%\mingw64\lib\tk*.* %LSD_DIR%\gnu\lib\
XCOPY %XOPT% %MSYS_DIR%\mingw64\lib\tcl8 %LSD_DIR%\gnu\lib\tcl8\
XCOPY %XOPT% %MSYS_DIR%\mingw64\lib\tcl8.6 %LSD_DIR%\gnu\lib\tcl8.6\
XCOPY %XOPT% %MSYS_DIR%\mingw64\lib\tk8.6 %LSD_DIR%\gnu\lib\tk8.6\
XCOPY %XOPT% %MSYS_DIR%\mingw64\lib\dde1.4 %LSD_DIR%\gnu\lib\dde1.4\
XCOPY %XOPT% %MSYS_DIR%\mingw64\lib\reg1.3 %LSD_DIR%\gnu\lib\reg1.3\
XCOPY %XOPT% %MSYS_DIR%\mingw64\lib\thread2.8.5 %LSD_DIR%\gnu\lib\thread2.8.5\

rem gdb debugger
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\gdb.exe %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\python3.8-config %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\libpython3.8.dll %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\libreadline8.dll %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\libtermcap-0.dll %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\libxxhash.dll %LSD_DIR%\gnu\bin\
XCOPY %XOPT% %MSYS_DIR%\mingw64\lib\gcc %LSD_DIR%\gnu\lib\gcc\
XCOPY %XOPT% %MSYS_DIR%\mingw64\lib\python3.8 %LSD_DIR%\gnu\lib\python3.8\
XCOPY %XOPT% %MSYS_DIR%\mingw64\share\gdb %LSD_DIR%\gnu\share\gdb\
XCOPY %OPT% %MSYS_DIR%\mingw64\etc\gdbinit %LSD_DIR%\gnu\etc\

rem diff compare tool
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\diff.exe %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\libiconv-2.dll %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\libintl-8.dll %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\libsigsegv-2.dll %LSD_DIR%\gnu\bin\

rem remove unneeded python cache files (requires python 3.5+ on path)
python3 -Bc "for p in __import__( 'pathlib' ).Path( '%LSD_DIR%\gnu' ).rglob( '*.py[co]' ) : p.unlink( )" > nul 2>&1
python3 -Bc "for p in __import__( 'pathlib' ).Path( '%LSD_DIR%\gnu' ).rglob( '__pycache__' ) : p.rmdir( )" > nul 2>&1

:end
