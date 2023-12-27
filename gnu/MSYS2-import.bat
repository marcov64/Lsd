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

rem component versions
set GCC_VER=13.2.0
set PYTHON_VER=3.11

rem XCOPY options for files and directories
set OPT=/D/Q/Y
set XOPT=%OPT%/S

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

if not exist %MSYS_DIR%\mingw64\share\gcc-%GCC_VER% (
	echo Please update the GCC version in this batch file
	goto end
)

if not exist %MSYS_DIR%\mingw64\lib\python%PYTHON_VER% (
	echo Please update the PYTHON version in this batch file
	goto end
)

echo MSYS2 libraries and make utility...
XCOPY %OPT% %MSYS_DIR%\usr\bin\rm.exe %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\usr\bin\msys-2.0.dll %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\usr\bin\msys-intl-8.dll %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\usr\bin\msys-iconv-2.dll %LSD_DIR%\gnu\bin\

echo make utility...
XCOPY %OPT% /-I %MSYS_DIR%\mingw64\bin\mingw32-make.exe %LSD_DIR%\gnu\bin\make.exe

echo g++ compiler...
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\gcc.exe %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\g++.exe %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\windres.exe %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\libwinpthread-1.dll %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\libgcc_s_seh-1.dll %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\libgmp-10.dll %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\libisl-23.dll %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\libmpfr-6.dll %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\libmpc-3.dll %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\libstdc++-6.dll %LSD_DIR%\gnu\bin\
XCOPY %XOPT% %MSYS_DIR%\mingw64\include %LSD_DIR%\gnu\include\
XCOPY %XOPT% %MSYS_DIR%\mingw64\lib %LSD_DIR%\gnu\lib\
XCOPY %XOPT% %MSYS_DIR%\mingw64\share\gcc-%GCC_VER% %LSD_DIR%\gnu\share\gcc-%GCC_VER%\
XCOPY %XOPT% %MSYS_DIR%\mingw64\x86_64-w64-mingw32 %LSD_DIR%\gnu\x86_64-w64-mingw32\

echo zlib library...
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\libzstd.dll %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\zlib1.dll %LSD_DIR%\gnu\bin\

echo Tcl/Tk framework...
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\tclsh*.exe %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\wish*.exe %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\tcl*.dll %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\tk*.dll %LSD_DIR%\gnu\bin\

echo gdb debugger...
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\gdb.exe %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\libncursesw6.dll %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\python%PYTHON_VER%-config %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\libpython%PYTHON_VER%.dll %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\libreadline8.dll %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\libtermcap-0.dll %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\libxxhash.dll %LSD_DIR%\gnu\bin\
XCOPY %XOPT% %MSYS_DIR%\mingw64\share\gdb %LSD_DIR%\gnu\share\gdb\
XCOPY %OPT% %MSYS_DIR%\mingw64\etc\gdbinit %LSD_DIR%\gnu\etc\

echo diff compare tool...
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\diff.exe %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\libiconv-2.dll %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\libintl-8.dll %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\libsigsegv-2.dll %LSD_DIR%\gnu\bin\

echo subbotools required libraries...
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\libgsl-27.dll %LSD_DIR%\gnu\bin\
XCOPY %OPT% %MSYS_DIR%\mingw64\bin\libgslcblas-0.dll %LSD_DIR%\gnu\bin\

echo done copying files

echo remove python cache files...
rem requires python 3.5+ on path!
python3 -Bc "for p in __import__( 'pathlib' ).Path( '%LSD_DIR%\gnu' ).rglob( '*.py[co]' ) : p.unlink( )" > nul 2>&1
python3 -Bc "for p in __import__( 'pathlib' ).Path( '%LSD_DIR%\gnu' ).rglob( '__pycache__' ) : p.rmdir( )" > nul 2>&1

echo done removing cache

echo checking missing files

if not exist %LSD_DIR%\gnu\bin\tail.exe echo missing Cygwin tail.exe
if not exist %LSD_DIR%\gnu\bin\multitail.exe echo missing Cygwin multitail.exe

if not exist %LSD_DIR%\gnu\bin\laplafit.exe echo missing Subbotools laplafit.exe
if not exist %LSD_DIR%\gnu\bin\laplaafit.exe echo missing Subbotools laplaafit.exe
if not exist %LSD_DIR%\gnu\bin\sepfit.exe echo missing Subbotools sepfit.exe
if not exist %LSD_DIR%\gnu\bin\subboafish.exe echo missing Subbotools subboafish.exe
if not exist %LSD_DIR%\gnu\bin\subboafit.exe echo missing Subbotools subboafit.exe
if not exist %LSD_DIR%\gnu\bin\subboagen.exe echo missing Subbotools subboagen.exe
if not exist %LSD_DIR%\gnu\bin\subbofish.exe echo missing Subbotools subbofish.exe
if not exist %LSD_DIR%\gnu\bin\subbofit.exe echo missing Subbotools subbofit.exe
if not exist %LSD_DIR%\gnu\bin\subbogen.exe echo missing Subbotools subbogen.exe
if not exist %LSD_DIR%\gnu\bin\subbolafit.exe echo missing Subbotools subbolafit.exe
if not exist %LSD_DIR%\gnu\bin\subboshow.exe echo missing Subbotools subboshow.exe

if not exist %LSD_DIR%\gnu\bin\LMM.assembly.manifest echo missing LMM.assembly.manifest
if not exist %LSD_DIR%\gnu\bin\Shortcut.exe echo missing Shortcut.exe
if not exist %LSD_DIR%\gnu\bin\TailWin.exe echo missing TailWin.exe

echo done

:end
