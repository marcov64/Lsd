**************************************************************

	LSD 8.0 - March 2021
	written by Marco Valente, Universita' dell'Aquila
	and by Marcelo Pereira, University of Campinas

	Copyright Marco Valente and Marcelo Pereira
	LSD is distributed under the GNU General Public License

**************************************************************

***********
Legal stuff
***********

LSD is copyrighted by Marco Valente and Marcelo C. Pereira (version 7.x additions) and is distributed according to the GNU General Public License. That is, as I understand it, you can use, modify and redistribute this code for free, as long as you maintain the same conditions. For legal conditions on gnuplot and the other software used see their legal notices.

LSD includes third party software under the license of the copyright owners:

GCC 10.2 is copyrighted by the Free Software Foundation, Inc. under GPL 3, https://gcc.gnu.org
Tcl/Tk 8.6 is copyrighted by the Regents of the University of California, and other parties under BSD-style license, https://www.tcl.tk
MSYS2 3.1.7, https://www.msys2.org
Cygwin 3.1.7 is copyrighted under GNU GPL and LGPL 3, https://cygwin.com
7-Zip LZMA SDK 19.0 is copyrighted by Igor Pavlov under GNU LGPL 2.1, https://www.7-zip.org
Makeself 2.4.3 is copyrighted by Stephane Peter under GPL2, https://makeself.io
Gnuplot 5.4.1 is copyrighted by Thomas Williams and Colin Kelley, http://www.gnuplot.info
Eigen 3.3.9 is copyrighted by Benoit Jacob and others under MPL2, http://eigen.tuxfamily.org
subbotools 1.3 is copyrighted by Giulio Bottazzi under GPL2, http://cafim.sssup.it/~giulio/software/subbotools
NOLHDesigns_v6 is copyrighted by Susan M. Sanchez under GPL 2.1, http://harvest.nps.edu
NOB_Mixed_512DP_v1 is copyrighted by Helcio Vieira under GPL 2.1, http://harvest.nps.edu
seticon/osxconutils is copyrighted by Sveinbjorn Thordarson, https://sveinbjorn.org/osxiconutils
Shortcut.exe 1.11 is copyrighted by Marty List, http://www.OptimumX.com
Silk icon set 1.3 is copyrighted by Mark James under CCA 2.5 license, http://www.famfamfam.com/lab/icons/silk
awthemes 9.2.2 is copyrighted by Brad Lanam, https://sourceforge.net/projects/tcl-awthemes
BallroomDJ 3.29.20 is copyrighted by Brad Lanam, https://sourceforge.net/projects/ballroomdj
ttkthemes 3.1.0 is copyrighted by RedFantom under GNU GPL 3+, https://github.com/TkinterEP/ttkthemes
tksvg 0.7 is copyrighted by Harald Oehlmann, https://github.com/oehhar/tksvg
dblclick.tcl is copyrighted by Wolf-Dieter Busch under OLL, https://wiki.tcl-lang.org/page/doubleclick
tooltip.tcl 1.4.6 is copyrighted by Jeffrey Hobbs, https://wiki.tcl-lang.org/page/tklib
tkcon.tcl 2.5 is copyrighted by Jeffrey Hobbs, https://wiki.tcl-lang.org/page/Tkcon
tkdiff.tcl 4.2 is copyrighted by John M. Klassa and others GNU GPL 2+, https://sourceforge.net/projects/tkdiff


********
Contents
********

This Readme.txt file contains five sections:

1. Introduction to LSD
2. LSD distribution content
3. Hints on the use of LSD
4. Installing LSD (Windows, macOS and Linux)
5. Optional compilers in Windows (MSYS2 and Cygwin)


***************
1. What is LSD?
***************

LSD is based on the assumption that simulation models are distinguished from simulation programs. Using LSD a user is concerned exclusively with the description of the theoretical model, while all the technicalities of the program implementing the model are automatically generated in an intuitive and computationally efficient way.

LSD can be considered as a modeling language. Using LSD a user is required to describe a model as if it were a system of discrete equations, where each variable is associated to piece of code (usually short and simple), indicating how the generic instance of the variable must compute its value at a generic time step. No other coding is required (e.g. to define the simulation cycle, saving data series, etc.). The required programming skills to use LSD are therefore solely dependent on the complexity of the equations of the model.

The code for the equations is expressed in a highly symbolic form, using a development environment providing assistance and examples. From this code the system automatically generates a program endowed with a complete set of interfaces to exploit the model. With such interfaces the user can: define the elements of the model, run simulations in various modes, inspect in any details events at run time, being informed on errors, analyze the results, generate automatic documentation.

The user interfaces and the skills required to implement simple models can be learned in a few days of training. Being based on C++ a LSD model can both express any computational content, and make use of existing libraries, generating extremely fast and efficient code. LSD is an open source project and makes use of other open source tools, as the GNU C++ compiler, Tcl/Tk windowing toolkit, gnuplot graphical package. LSD is available natively for Windows, macOS and Linux computers, with minimal requirements.


***************************
2. LSD distribution content
***************************

This distribution contains:

- LSD code, for the creation of simulation models
- LSD Model Manager (LMM), a development environment for LSD models
- Several LSD example models
- Comprehensive documentation, organized as context-sensitive help pages

LSD source code: source files are usually managed (e.g., for compilation) automatically, so that users need not to worry for technical issues or command line usage. Upgrading to new versions of LSD (which always guarantees backward compatibility) necessitates only to replace the source files (e.g, extracting the new distribution on the existing location).

LSD example models : several models are available, providing examples on the design of whole models, which can be used to copy-and-past single equations or chunks of code.

LSD Model Manager (LMM): LSD models can be developed using any text editor and standard make files, so users are allowed to use their preferred development environment. However, LSD distribution contains a simple IDE (integrated development environment) providing all the functionalities required by the typical needs of a LSD project. LMM permits to manage easily several modeling projects (small and large), to create new model code with extensive assistance, and to easily debug problematic models.


*********************
3. Use of LMM and LSD
*********************

INSTALLATION: please follow the detailed steps below to install LSD in your Linux, Windows or Mac computer. DO NOT try to use LSD without making sure all the required steps were successfully performed.

LMM (LSD Model Manager) is a program used to manage LSD model programs. LSD (Laboratory for Simulation Development) model programs are stand-alone programs that execute fast and efficiently difference-equation simulation models. For a user to develop a new simulation model it is only requested to specify the equations of the model in a simplified C++ language, with the assistance of automatic help. LSD model programs generate automatically the code necessary to link the equations in a coherent sequence within a simulated time steps, saving and elaborating the result, allowing easy access to initialization values, and many other operations required for fully exploiting the simulation model.

When LMM starts the first operation is to choose a model to work with. Using the LMM's Model Browser you can either select one of the existing models, or create a new empty one (that is, no equations, variables etc.).

After a model is selected in Model Browser, you can ask LMM to:
- compile the model and run it;
- set compilation options
- edit the equations of the model;

ATTENTION: the very first time a model is compiled an error may be caused by the misspecification of the system directories or the operating system. In this case, use menu Model/System Compilation Option and use the default values for your operative system to let LMM adjust automatically the error.

When a model program is successfully compiled and run by LMM, then the user can interact with the LSD Browser interface. This window permits to control every aspect of the simulation run (e.g., setting initial values, observing and saving results, reading the model documentation etc.) except for the modification of the code for the equations, done in LMM. For this latter operation you need to close the LSD model program, tell LMM to show the equation file if necessary, edit the model's equations and compile/run a new LSD model program.

DOCUMENTATION: all LSD documentation is accessible through help menus and presented as HTML pages after installation is finished. The documentation includes from tutorials and a course on LSD to a complete manual covering all LSD details.

MANUAL CONFIGURATION: after LSD installation, or when changing between operating systems or 32 and 64-bit modes, it may be necessary to (re-) configure LSD. To do so, after launching LMM, perform the following steps:

1. Select "Options..." in menu "File" then press the button "Default", change any configuration if desired, and press "OK".

2. Select "System Options..." in menu "Model", press the button "Default", and "OK".

WARNING: LSD operates by creating and modifying executable (.exe) files for your simulation models. This kind of procedure may be INCORRECTLY detected as suspicious by some anti-virus software and incorrect operation of LSD may arise, particularly when you try to compile and/or run your model. In this case, you may try to reconfigure your anti-virus to allow LSD operation or try to TEMPORARILY disable it entirely. Windows standard anti-virus software (included and enabled by default in Windows 10) is known to operate properly with LSD and offers a good level of protection for most users.

For persisting problems email us: valente@ec.univaq.it or mcper@unicamp.br


**************************************
4.1 Windows installation (64-bit ONLY)
**************************************

IMPORTANT: from version 8.0 and higher, LSD supports only Windows 64-bit. If you must run LSD in a Windows 32-bit computer, please use the 7.2 version which can be downloaded from the same site indicated below. If you are unsure of the version of Windows in your computer, please check in "Windows > Settings > System > About > System type".

ANTIVIRUS BAD BEHAVIOR: many third-party anti-virus software consider LSD operation suspicious because it creates executable files and deals with tools, like compilers, which do not belong to the regular user universe. This wrong assumption can break LSD operation in many aspects and usually cannot be fixed reliably, so using LSD together with these tools is NOT SUPPORTED. LSD is tested and fully works with the embedded Windows Security anti-virus, which is unfortunately disabled when a third-party anti-virus is installed by the user. According to specialized media tests, Windows anti-virus is considered an excellent option, in particular when compared to the free alternatives, so using it instead should not create any problem. However, if you cannot remove your current third-party anti-virus (and enable Windows Security), you still have the option to run LSD inside a virtual machine like VirtualBox or VMware (but with reduced performance).

To install LSD, the simplest alternative is to use the installer executable (e.g. LSD-installer-windows-8-0-stable-1.exe). Download it, double-click on the installer file, and follow the instructions. The installer executable can be deleted after the installation. According to your configuration, express authorization must be provided before downloading and running the LSD installer. This is normal.
 
Alternatively, or in the case of problems using the installer, it is also possible to simply unzip a LSD distribution file archive on the desired folder. In most cases "C:\" (the root of the hard disk) is a good option. The steps to perform a manual installation are:

1. In an internet browser, open the site https://github.com/marcov64/Lsd/releases , choose the desired release, and click on the respective "Source code (zip)" link to download the LSD distribution file.

2. Open Windows Explorer and double-click on the downloaded LSD distribution file to open it (e.g., LSD-8.0-master.zip).

3. Drag the single folder inside the distribution file (e.g., LSD-8.0-master) to the desired location. The recommended is "C:\", normally labeled as "Local Disk (C:)" in Windows Explorer.

This will create the whole LSD folder structure. The distribution file may be deleted after installation.

IMPORTANT: LSD cannot be installed within a folder (or having a parent folder) containing spaces in its name. For example, the directory "C:\Program Files\Lsd" cannot be used. If you installed LSD in such a location, simply move (or drag) the whole LSD folder structure to an adequate place ("C:\" recommended).

To create a desktop link (icon) to run LSD/LMM, as well as an entry to the Windows Start Menu, you can simply double-click in Windows Explorer the following batch file available in the installation directory:

 add-shortcut-windows[.bat]

After the desktop icon is created, double-clicking it opens LMM (LSD Model Manager) which allows to create new models, or select existing models. If you cannot create the LSD icon on your desktop, or an entry in Start Menu, following the steps above, you may still run LSD by double-clicking the file "LMM.exe" inside the LSD installation directory.

GNUPLOT INSTALLATION:

It is recommended, but not required, to install the Gnuplot graphical plotting application. Gnuplot allows LSD to produce X-Y and other more elaborated plots but it is not otherwise needed to run LSD simulations. LSD installer usually detects when Gnuplot is missing and tries to install it automatically. If this is not possible, Gnuplot can be also installed manually. The required Gnuplot installer (64-bit) can be downloaded from http://www.gnuplot.info . The step-by-step instructions to install Gnuplot are:

1. In an internet browser, open the site http://www.gnuplot.info, choose the desired version (the current stable one is recommended), click on the respective Release link, and on the files list choose gpXXX-win64-mingw.exe, where XXX is the current version number (currently, gp528-win64-mingw.exe), ensuring the 64-bit Windows version is selected.

2. Open Windows Explorer and double-click on the downloaded Gnuplot installer (e.g., gp528-win64-mingw.exe).

3. Accept the agreement, press "Next" twice, agree with the default installation folder, press "Next", accept the proposed components to install, press "Next", accept the creation of a Start Menu item, and press "Next" again.

4. In the "Select Additional Tasks", set the terminal type to "wxt", keep the proposed file associations, and MAKE SURE you select the "Add application directory to your PATH environment variable". If this option is not visible, you need to scroll down in the window to see it. Press "Next" and then "Install". When the installation is completed, press "Next" and "Finish".

ATTENTION: If Gnuplot is not included in the Windows PATH environment variable, LSD may not be able to use Gnuplot. If this option is missing, Gnuplot must be installed again using the correct options as described above.

FIXING COMPILATION ERRORS:

Some free software packages have the bad habit of installing old copies of libraries required by LSD on the system PATH, which may prevent successful compilation and operation of LSD. The installer tries to detect and inform about possible sources of problems, which may have to be fixed before LSD can be run reliably. Alternatively, the user may force LSD libraries into the system PATH, which is NOT done by the installer. To do so, open Windows Explorer, navigate to the folder where LSD was installed, right-click on the file:

 set-system-path-windows[.bat]
 
and choose the "Run as administrator" option. This will force LSD libraries to be used system-wide, which may break other software which relies on older versions of the libraries.

USING DIFFERENT COMPILERS (optional):

It is possible to use LSD with a C++ compiler already installed in your computer. However, in this case the user must install and configure the compiler to ensure it has all the required optional libraries. LSD supports any version of GNU 64-bit C++ compiler (GCC) supporting C++14 (or more recent) standard. Cygwin and MSYS2 mingw-w64-x86_64 versions are both fine, but they require Tcl/Tk 8.6 and zlib 1.2 libraries to be installed. Cygwin compiler is sometwhat easier to install as it does not require the user to deal with a command prompt. MSYS2 compiler usually releases new versions earlier. Instructions for installing both are available at the end of this document. Even if they are already installed, the instructions can be also followed to make sure the installation is complete and LSD is configured to use it.


************************************
4.2 macOS installation (10.10+ ONLY)
************************************

To install LSD, the simplest alternative is to use the installer package (e.g. LSD-installer-mac-8-0-stable-1.dmg). Download it, double-click on the package file to mount it, double-click on the LSD Installer application, and follow the instructions. The installer package can be unmounted and deleted after the installation.
 
IMPORTANT: please pay attention to the Terminal windows opened by the installer. They may require your interaction, according to the instructions of the installer. Do not close any Terminal window which is not inactive. If the Terminal window is closed or interrupted by accident, please cancel the installation and restart.

To manually install LSD, simply unzip a LSD distribution file (e.g., LSD-8.0-master.zip) to the chosen directory (in most cases  "~/", the user home directory, is recommended) using Finder. This will create the whole LSD folder structure. Take note of the name of the main (topmost) folder where LSD is installed. The distribution file may be deleted after the extraction.

Next, open the Terminal application (located inside the Utilities app folder), and type the following commands (each line must be completed by pressing "Enter"):

 cd ~/Lsd-8.0-master
 ./add-shortcut-mac.sh

After a successful installation, a desktop shortcut (icon) will be available for using LMM/LSD. A shortcut is also created in the computer Applications folder.

Users of macOS Sierra (10.12) or newer MUST yet manually install the Apple Command Line Tools package (the full Xcode package is NOT required) to make the compiler and other required command line tools available in macOS. To install it, open the Terminal  and enter the following command:

 xcode-select --install

Then, in the opened window, click on the "Install" button (do NOT click on the "Get Xcode" button), accept the license and wait the installation to complete (you may have to reboot to finish the installation).

It is recommended, but not required, to install multitail and Gnuplot applications. Gnuplot allows LSD to produce X-Y and other more elaborated plots but it is not otherwise needed to run LSD simulations. The easiest way to install Gnuplot is to use the Homebrew package manager for the installation. If you do not have Homebrew installed, at the terminal prompt, paste the following command and press ENTER:

 /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install.sh)"

After Homebrew installation finishes (details at http://brew.sh), you can install multitail and Gnuplot using the following command in Terminal (Qt framework will be automatically installed too):

 brew install multitail gnuplot

INACTIVE TERMINAL WINDOWS:

As LSD accesses external programs, inactive Terminal windows may be left open in the computer desktop. To prevent this default behavior, the user can change the Terminal aplication configuration. In Terminal, open menu "Preferences > Profiles > Shell" and set options "When the shell exists" to "Close if the shell exited cleanly", and "Ask before closing" to "Never".

IN CASE OF PROBLEMS:

LSD is only supported in mac computers as a native macOS (Aqua) application. There is no longer support for X11.

If using macOS 10.12 (Sierra) or newer, you MUST use the "add-shortcut-mac.sh" command above to remove LMM and LSD from the system quarantine. Failing to do so will prevent from using LSD as a native macOS application.

If you delete or cannot create the LSD shortcuts, you can still run the app named LMM located inside the LSD installation directory (double click it in Finder) to open LMM/LSD, or try to rerun the "add-shortcut-mac.sh" script to recreate the shortcut.

AUTOMATION ACCESS:

LSD needs to control external programs to automate operation. The first time it needs automation access, a macOS (10.14+) popup will request the user to allow it, please click on OK. If user denies automation access once, macOS will not ask again for authorization and will silently block all further LSD operations, curtailing LSD capabilities and preventing some operations. To force the system to ask again for authorization (all programs) please use the following command on the Terminal:

 tccutil reset AppleEvents

Authorized programs can be checked at System Preferences > Security & Privacy > Privacy > Automation.


**********************
4.3 Linux installation
**********************

To install LSD, the simplest alternative is to use the installer script package (e.g. LSD-installer-linux-8-0-stable-1.zip). Download, extract and execute it. If your file manager does not support extracting compressed files or executing scripts directly, open a terminal, and type (replacing the X's with the actual values):

 cd ~/Downloads
 unzip LSD-installer-linux-8-X-xxxx-X.sh
 ./LSD-installer-linux-8-X-xxxx-X.sh

You may have to adjust the download directory according to your computer. When the LSD installer program opens, follow the instructions to install LSD and the required packages. The installer requires Tcl/Tk to be installed. This is the default in most distributions. The installer script will ask you to install Tcl/Tk if this is not the case. The installer package and script can be deleted after the installation.

Alternatively, or in the case of problems using the installer script, it is also possible to simply unpack the LSD distribution file (e.g., Lsd-8.0-master.tar.gz) to the chosen directory (in most cases  "~/", the user home directory, is recommended) using your file manager or command line. This will create the whole directory structure. In terminal, you may use the command (".tar.gz" extension files only):

 tar -xzf Lsd-8.0-master.tar.gz

The distribution file may be deleted after installation.

If you want to create a desktop link (icon) to run LSD/LMM, you can use the script available in the installation directory executing the command (may work only in Gnome-based distributions):

 ./add-shortcut-linux.sh

To use the LSD in Linux it is necessary to have the GNU gcc/g++ compiler (version 6.4+) with the standard packages, including zlib, Tcl/Tk 8.6, and GDB. Likely, you already have those installed on your computer but you may need to install the development version of these packages. Usually, you can use your distribution package manager to get the appropriate 'dev' package versions to your installation. Though not strictly necessary, it is also suggested to have the Gnuplot graphical package (for advanced graphics), preferably using Qt.

In Debian or Ubuntu, to make sure you have the correct libraries you can use:

 sudo apt-get install build-essential gdb gnuplot-qt multitail zlib1g-dev tcl-dev tk-dev

In Fedora, CentOS or Red Hat, the equivalent command is:

 sudo yum install gcc-c++ make gdb gnuplot multitail zlib-devel tcl tk tcl-devel tk-devel

In Mandriva or Mageia:

 sudo urpmi gcc-c++ make gdb gnuplot multitail lib64z-devel lib64tcl-devel lib64tk-devel

IN CASE OF PROBLEMS:

Please check your configuration has at least g++ version 6.4 installed (you may check it by issuing the command "g++ -v" in terminal). Ubuntu minimum supported version is 18.04 (older versions can be updated to use g++ 6.4 or newer but this is not the default configuration).

Also make sure if Tcl/Tk version 8.6 or newer is present (use the command "echo 'puts $tcl_version;exit 0' | tclsh" to get the installed version).

To run LMM from a system shell, please open a terminal in the installation directory (or use your graphical file browser) and execute:

 ./run.sh

If you get an error when trying to execute any of the above commands, please make sure the respective files are set as executable (use terminal command "chmod +x FILENAME" in the installation directory, replacing FILENAME by the name of the corresponding file).

You may need to recompile LMM if the included pre-compiled versions have problems with your Linux setup. Move in the LSD installation directory and use the commands:

 cd src
 make -f makefile.linux

If the compilation fails, the most likely reason is the mis-specification of the locations of the files required for the compilation. The major problem is that Tcl/Tk may be installed in your systems in several different locations. The makefile contains a list of variables for the directory needed for the Tcl/Tk libraries and include files. For example, on some systems you have the Tcl/Tk library located in /usr/lib, or /usr/local/lib, or usr/share/lib, etc. Similarly, the include files may be located in different directories. The makefile lists the files you need to identify; check the location for those files and edit the makefile as appropriate for your system.

It is also possible that Tcl/Tk requires further libraries besides those specified in the makefile. If you have errors even after having specified the correct path to the Tcl/Tk libraries, then find out where the file wish is located (using the command "whereis wish"), and then find out which libraries are used with the command:

 ldd /usr/bin/wish

If the system lists further libraries, add the appropriate option to the linker (e.g. -lieee to add the library libieee.a) in the makefile to the variable DUMMY.

IMPORTANT: if you modified the makefile to compile LMM, the same changes need to be made to the makefiles used to generate the LSD Model programs. You need to make these changes only once using LMM. Use the menu item System Compilation Options in menu Model. You will have to fill the same variables as in the makefile used to compile LMM, which must be set to the same values.


******************************************
5.1 MSYS2 compiler installation (optional)
******************************************

Please note that MSYS2 compiler is NOT required (or recommended) if the above instructions for installing LSD were already performed, as LSD already embeds an up-to-date MSYS2 Windows compiler. If performing an update over an existing MSYS2 installation, it is recommended to uninstall the previous version before proceeding. If an on-place update is performed, without removing the existing version, start on step 2 below.

Step-by-step procedure to install the MSYS2 64-bit compiler (an internet connection is required):

1. Download and execute the MSYS2 64-bit installer from https://www.msys2.org.

2. Open the MSYS2 shell (icon on Start Menu) and update MSYS2 using the command:

 pacman -Syu

3. When instructed, close and re-open the MSYS2 shell, and update again:

 pacman -Su

4. Still in the shell, install the compiler and required tools:

 pacman -S mingw-w64-x86_64-make mingw-w64-x86_64-gcc mingw-w64-x86_64-gdb

5. Press "Windows" and "R" keys together, type the command "sysdm.cpl" in the "Run" dialog box and press "Enter". In the "System Properties" window, go to the "Advanced" tab and click on button "Environment Variables...". In the "System variables" list, select "Path" and press "Edit...". Run across the lines to see if your MSYS2 MinGW64 bin folder, i.e. "C:\msys64\mingw64\bin", is already there. If Yes, just press "Cancel" 3 times. If not, click on "New" and type MSYS2 MinGW64 binary folder, i.e. "C:\msys64\mingw64\bin". Press "Ok" 3 times and you are done. Make sure you don't have any older version of gcc ahead of your MSYS2 MinGW64 bin folder in PATH.

6. Open LSD LMM and choose menu "Model" > "System Options...". Edit the following entries to the values shown bellow:

 PATH_HEADER=.
 PATH_LIB=.


SLOW GDB OPERATION: the GDB debugger can become very slow in Windows when the models are large. This is due to the Windows special handling of the heap (dynamic memory allocation) for programs being debugged. It is normally safe to disable this special handling by defining the folowing environment variable:

 _NO_DEBUG_HEAP=1

To define it, open "Control Panel", sequentially select "System and Security", "System", "Advanced system settings", "Advanced" tab and then "Environment Variables...". In the "System variables" list, press "New...", type "_NO_DEBUG_HEAP" as the variable name, and "1" as its value. Press "Ok" 3 times and you are done.


*******************************************
5.2 Cygwin compiler installation (optional)
*******************************************

Please note that Cygwin compiler is NOT required (or recommended) if the above instructions for installing LSD were already performed, as LSD already embeds an up-to-date compiler for Windows. 

The Windows Cygwin installer can be downloaded at http://www.cygwin.com (make sure you download the "setup-x86_64.exe" file). Preferably, install Cygwin to the default directory ("C:\cygwin64") or to the existing Cygwin directory in case of an update. If installing to a different directory please note that the same restrictions mentioned above, about folder using names WITHOUT spaces, also apply to Cygwin. Additionally to the packages automatically installed with Cygwin, five non-default packages are REQUIRED by LSD (mingw64-x86_64-gcc-g++, mingw64-x86_64-zlib, make, gdb and multitail). Alternatively, the pure-Cygwin, non-MinGW 64-bit gcc compiler can be also used, if matched with the corresponding Tcl/Tk and zlib libraries. Lastly, Cygwin main and compiler binary subdirectories must be added to the PATH environment variable (not done by the installer).

Step-by-step procedure to install the Cygwing 64-bit compiler (an internet connection is required):

1. In an internet browser, open the site https://www.cygwin.com and find the link to download the Cygwin installer file for Windows 64-bit (setup-x86_64.exe).

2. In Windows Explorer, double click the downloaded "setup-x86_64.exe" installer file.

3. When the installer opens, choose "Install from Internet", accept the proposed root install directory (C:\cygwin64), press "Next", opt for installing for "All Users", press "Next", accept the proposed local package directory (C:\cygwin64), "Next" again, select "Direct Connection", press "Next", choose any of the offered download sites (one closer to your location will just provide faster downloads), and "Next".

4. When the "Select Packages" window opens, click on the View option list, choose "Full" (to see the list with all packages), type the name of each required package in the Search box, find the EXACT name in the packages list (do not use partial matches), and on the "New" column change the option from "Skip" to the latest available version (the one with the higher number). Repeat this process for each of the five required packages: "mingw64-x86_64-gcc-g++", "mingw64-x86_64-zlib", "make", "gdb", and "multitail".

5. When all the required packages are marked for download, choose "Next", accept the proposed changes and proceed to download and install Cygwin. Please wait until the installer finishes (it may take a while) and close the installer when requested.

6. Press "Windows" and "R" keys together, type the command "sysdm.cpl" in the "Run" dialog box and press "Enter". In the "System Properties" window, go to the "Advanced" tab and click on button "Environment Variables...". In the "System variables" list, select "Path" and press "Edit...". Run across the lines to see if your Cygwin two bin folders, i.e. "C:\cygwin64\bin" and "C:\cygwin64\usr\x86_64-w64-mingw32\sys-root\mingw\bin", are already there. If Yes, just press "Cancel" 3 times. If not, click on "New" and type Cygwin main binary folder, i.e. "C:\cygwin64\bin", then press "New" again and type the compiler binary folder, i.e."C:\cygwin64\usr\x86_64-w64-mingw32\sys-root\mingw\bin". Press "Ok" 3 times and you are done. Make sure you don't have any older version of gcc ahead of your Cygwin bin folder in PATH.

7. Open LSD LMM and choose menu "Model" > "System Options...". Edit the following entries to the values shown bellow:

 PATH_HEADER=.
 PATH_LIB=.
 WRC=x86_64-w64-mingw32-windres
 CC=x86_64-w64-mingw32-g++
