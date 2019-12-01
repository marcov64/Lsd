**************************************************************

	LSD 7.2 - December 2019
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

Eigen 3.3.4 is copyrighted by Benoit Jacob and others under a MPL2 license, http://eigen.tuxfamily.org
NOLHDesigns_v6 is copyrighted by Susan M. Sanchez under a GPL 2.1 license, http://harvest.nps.edu
NOB_Mixed_512DP_v1 is copyrighted by Helcio Vieira under a GPL 2.1 license, http://harvest.nps.edu
seticon/osxconutils is copyrighted by Sveinbjorn Thordarson, https://sveinbjorn.org/osxiconutils
Shortcut.exe 1.11 is copyrighted by Marty List, http://www.OptimumX.com
Silk icon set 1.3 is copyrighted by Mark James under a CCA 2.5 license, http://www.famfamfam.com/lab/icons/silk


********
Contents
********

This Readme.txt file contains five sections:

- Introduction to LSD
- LSD distribution content
- Installing LSD (Windows, MacOS X and Linux)
- Manually configuring LSD
- Hints on the use of LSD


************
What is LSD?
************

LSD is based on the assumption that simulation models are distinguished from simulation programs. Using LSD a user is concerned exclusively with the description of the theoretical model, while all the technicalities of the program implementing the model are automatically generated in an intuitive and computationally efficient way.

LSD can be considered as a modeling language. Using LSD a user is required to describe a model as if it were a system of discrete equations, where each variable is associated to piece of code (usually short and simple), indicating how the generic instance of the variable must compute its value at a generic time step. No other coding is required (e.g. to define the simulation cycle, saving data series, etc.). The required programming skills to use LSD are therefore solely dependent on the complexity of the equations of the model.

The code for the equations is expressed in a highly symbolic form, using a development environment providing assistance and examples. From this code the system automatically generates a program endowed with a complete set of interfaces to exploit the model. With such interfaces the user can: define the elements of the model, run simulations in various modes, inspect in any details events at run time, being informed on errors, analyze the results, generate automatic documentation.

The user interfaces and the skills required to implement simple models can be learned in a few days of training. Being based on C++ a LSD model can both express any computational content, and make use of existing libraries, generating extremely fast and efficient code. LSD is an open source project and makes use of other open source tools, as the GNU C++ compiler, Tcl/Tk windowing toolkit, gnuplot graphical package. LSD is available natively for Linux, Windows and Mac OS X systems, with minimal requirements.


************************
LSD distribution content
************************

This distribution contains:

- LSD code, for the creation of simulation models
- LSD Model Manager (LMM), a development environment for LSD models
- Several LSD example models
- Comprehensive documentation, organized as context-sensitive help pages 

LSD source code: source files are usually managed (e.g., for compilation) automatically, so that users need not to worry for technical issues or command line usage. Upgrading to new versions of LSD (which always guarantees backward compatibility) necessitates only to replace the source files (e.g, extracting the new distribution on the existing location).

LSD example models : several models are available, providing examples on the design of whole models, which can be used to copy-and-past single equations or chunks of code.

LSD Model Manager (LMM): LSD models can be developed using any text editor and standard make files, so users are allowed to use their preferred development environment. However, LSD distribution contains a simple IDE (integrated development environment) providing all the functionalities required by the typical needs of a LSD project. LMM permits to manage easily several modeling projects (small and large), to create new model code with extensive assistance, and to easily debug problematic models.
 
Documentation: all LSD documentation is accessible through help menus and presented as HTML pages.


********************************
Windows installation (32/64-bit)
********************************

To install LSD, simply unzip the LSD distribution file on the desired folder. In most cases "C:\" (the root of the hard disk) is the best option. The simplest way of doing is:

1. In an internet browser, open the site https://github.com/marcov64/Lsd/releases , choose the desired release, and click on the respective "Source code (zip)" link to download the LSD distribution file.

2. Open Windows Explorer and double-click on the downloaded LSD distribution file to open it (e.g., Lsd-7.2-master.zip).

3. Drag the single folder inside the distribution file (e.g., Lsd-7.2-master) to the desired location. The recommended is "C:\", normally labeled as "Local Disk (C:)" in Windows Explorer.

This will create the whole LSD folder structure. The distribution file may be deleted after installation.

IMPORTANT: LSD cannot be installed within a folder (or having a parent folder) containing spaces in its name. For example, the directory "C:\Program Files\Lsd" cannot be used. If you installed LSD in such a location, simply move (drag) the whole LSD folder structure to an adequate place ("C:\" recommended).

LSD operates in two modes: 32 and 64-bit. Most recent Windows computers come enabled to the newer 64-bit mode. So, the recommended configuration for most users is to use the more capable 64-bit version of LSD. It can handle much larger models and access all the memory available in your computer (the 32-bit version is limited to 2 Gbytes). However, to use the 64-bit version the user has to manually install a compatible 64-bit compiler, if one is not already present (usually not), as detailed in the next section.

To create a desktop link (icon) to run LSD/LMM, you simply double-click in Windows Explorer one of the following batch files available in the installation directory:

LSD 32-bit: add-shortcut.bat 
LSD 64-bit: add-shortcut64.bat   (requires the installation of a 64-bit compiler, see below)
 
After the desktop icon is created, double-clicking it opens LMM (LSD Model Manager) in the selected mode (32-bit or 64-bit), which allows to create new models, or select existing models. Never invoke the files "lmm.exe" or "lmm64.exe" directly, because they need some environment variables to be set by "run.bat" or "run64.bat". 

If you create a model using a mode (say, 32-bit) and later decides to use it under a different one (say, 64-bit), you must manually reconfigure LSD following the instructions in the "Manually configuring LSD" section below.

GNUPLOT INSTALLATION:

It is recommended, but not required, to install the Gnuplot graphical plotting application. Gnuplot allows LSD to produce X-Y and other more elaborated plots but it is not otherwise needed to run LSD simulations. The required Gnuplot installer (64-bit preferred, except for old Windows 32-bit computers) can be downloaded from http://www.gnuplot.info . The step-by-step instructions to install Gnuplot are:

1. In an internet browser, open the site http://www.gnuplot.info, choose the desired version (the current stable one is recommended), click on the respective Release link, and on the files list choose gpXXX-winYY-mingw.exe, where XXX is the current version number an YY is the Windows type (currently, gp527-win64-mingw.exe, for 64-bit Windows).

2. Open Windows Explorer and double-click on the downloaded Gnuplot installer (e.g., gp527-win64-mingw.exe).

3. Accept the agreement, press "Next" twice, agree with the default installation folder, press "Next", accept the proposed components to install, press "Next", accept the creation of a Start Menu item, and press "Next" again.

4. In the "Select Additional Tasks", set the terminal type to "wxt", keep the proposed file associations, and MAKE SURE you select the "Add application directory to your PATH environment variable". If this option is not visible, you need to scroll down in the window to see it. Press "Next" and then "Install". When the installation is completed, press "Next" and "Finish".

ATTENTION: If Gnuplot is not included in the Windows PATH environment variable, LSD will not be able to use Gnuplot. If this option is missing, Gnuplot must be installed again using the correct options as described above.


**********************************
Windows installation (64-bit only)
**********************************

To use the 64-bit mode of LSD, you need the GNU 64-bit C++ compiler installed (Cygwin or MSYS2 mingw-w64-x86_64 versions are both fine). The Cygwin installer can be downloaded at http://www.cygwin.com (make sure you download the "setup-x86_64.exe" file). Preferably, install Cygwin to the default directory ("C:\cygwin64"). If installing to a different directory please note that the same restrictions mentioned above, about folder using names WITHOUT spaces, also apply to Cygwin. Additionally to the packages automatically installed with Cygwin, four non-default packages are REQUIRED by LSD (mingw64-x86_64-gcc-g++, mingw64-x86_64-zlib, make, gdb and multitail). Lastly, Cygwin must be added to the PATH environment variable (not done by the installer).

If you prefer to use MSYS2 instead of Cygwin (only recommended for advanced users), see instructions at the end of this section.

Step-by-step procedure to enable LSD 64-bit mode (an internet connection is required):

1. In an internet browser, open the site http://www.cygwin.com and find the link to download the Cygwin installer file for Windows 64-bit (setup-x86_64.exe).

2. In Windows Explorer, double click the downloaded "setup-x86_64.exe" installer file.

3. When the installer opens, choose "Install from Internet", accept the proposed root install directory (C:\cygwin64), press "Next", opt for installing for "All Users", press "Next", accept the proposed local package directory (C:\cygwin64), "Next" again, select "Direct Connection", press "Next", choose any of the offered download sites (one closer to your location will just provide faster downloads), and "Next".

4. When the "Select Packages" window opens, click on the View option list, choose "Full" (to see the list with all packages), type the name of each required package in the Search box, find the EXACT name in the packages list (do not use partial matches), and on the "New" column change the option from "Skip" to the latest available version (the one with the higher number). Repeat this process for each of the five required packages: "mingw64-x86_64-gcc-g++", "mingw64-x86_64-zlib", "make", "gdb", and "multitail".

5. When all the required packages are marked for download, choose "Next", accept the proposed changes and proceed to download and install Cygwin. Please wait until the installer finishes (it may take a while) and close the installer when requested.

6. In Windows Explorer, double click in the "add-cygwin64-to-path.bat" file (in LSD installation folder). A black empty window should briefly open and close, and the Windows 64-bit configuration is done. Otherwise, if there is an error message and the black window remains open, see the alternative steps below to fix the problem.

IN CASE OF PROBLEMS:

LSD requires Cygwin to be added to the Windows PATH environment variable. If the above instructions fail, please open a Windows Command Prompt in the LSD installation directory. To do so, press and hold shift while right clicking LSD folder in Windows Explorer, then select "Open command prompt here". On the Command Line, please type the command below and press "Enter". If the installation was done to a different path, please adjust the command to the correct Cygwin installation directory:

add-cygwin64-to-path C:\cygwin64

Alternatively, or if you have problems with the command above, you can use Windows GUI to add Cygwin to the PATH. In Windows open "Control Panel", sequentially select "System and Security", "System", "Advanced system settings", "Advanced" tab and then "Environment Variables...". In the "System variables" list, select "Path" and press "Edit..." (be carefull to NOT DELETE the existing text). Run across the lines to see if your Cygwin bin folder, i.e. "C:\cygwin64\bin", is already there. If Yes, just press "Cancel" 3 times. If not, at the end of the "Variable value" field type ";" (next to the existing text) and add your Cygwin bin folder. Press "Ok" 3 times and you are done. Make sure you don't have any older version of gcc ahead of your Cygwin bin folder in PATH.

If you cannot create the LSD icon on your desktop following the steps above, you may still run LSD by double-clicking the file "run64.bat" inside the LSD installation directory.

WARNING: LSD operates by creating and modifying executable (.exe) files for your simulation models. This kind of procedure may be INCORRECTLY detected as suspicious by some anti-virus software and incorrect operation of LSD may arise, particularly when you try to compile and/or run your model. In this case, you may try to reconfigure your anti-virus to allow LSD operation or try to TEMPORARILY disable it entirely. Microsoft standard anti-virus software (included and enabled by default in Windows 10) is known to operate properly with LSD and offers a good level of protection for most users.

MSYS2: Please note that MSYS2 is NOT required if the above instructions for installing Cygwin were already performed. Steps to configure MSYS2:

1. Download and install the MSYS2 64-bit installer from http://www.msys2.org.

2. Open the MSYS2 shell and update MSYS2 using the command:

pacman -Syu

3. When instructed, close and re-open the MSYS2 shell, and update again:

pacman -Su

4. Still in the shell, install the compiler and required tools:

pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-gdb

5. Add the compiler to path, double clicking on "add-msys2-to-path.bat" file (in LSD installation folder) if MSYS2 was installed on the default path (C:\msys64). Otherwise, use the command below in the LSD installation folder (replacing PATH with the chosen installation folder):

add-msys2-to-path PATH

SLOW GDB OPERATION: the GDB debugger can become very slow in Windows when the models are large. This is due to the Windows special handling of the heap (dynamic memory allocation) for programs being debugged. It is normally safe to disable this special handling by defining the folowing environment variable:

_NO_DEBUG_HEAP=1

To define it, open "Control Panel", sequentially select "System and Security", "System", "Advanced system settings", "Advanced" tab and then "Environment Variables...". In the "System variables" list, press "New...", type "_NO_DEBUG_HEAP" as the variable name, and "1" as its value. Press "Ok" 3 times and you are done.


******************
Linux Installation
******************

To install LSD, simply unpack the LSD distribution file (e.g., Lsd-7.2-master.tar.gz) to the chosen directory (in most cases  "~/", the user home directory, is recommended) using your file manager. This will create the whole directory structure. 

Alternatively, it is also possible to unpack the distribution file using the terminal using the command (".tar.gz" extension files only):

tar -xzf Lsd-7.2-master.tar.gz

The distribution file may be deleted after installation.

If you want to create a desktop link (icon) to run LSD/LMM, you can use the script available in the installation directory executing the command (Ubuntu and Gnome based distributions):

./add-shortcut-linux.sh

To use the LSD in Linux it is necessary to have the GNU gcc/g++ compiler (version 4.9+) with the standard packages, including zlib, Tcl/Tk 8.6, and GDB. Likely, you already have those installed on your computer but you may need to install the development version of these packages. Usually, you can use your distribution package manager to get the appropriate 'dev' package versions to your installation. Though not strictly necessary, it is also suggested to have the Gnuplot graphical package (for advanced graphics), preferably using Qt. 

In Debian or Ubuntu, to make sure you have the correct libraries you can use:

sudo apt-get install build-essential gdb gnuplot-qt zlib1g-dev tcl8.6-dev tk8.6-dev multitail

In Fedora, CentOS or Red Hat, the equivalent command is:

sudo yum install gcc-c++ make gdb gnuplot zlib-devel tcl tk tcl-devel tk-devel multitail

IN CASE OF PROBLEMS:

Please check your configuration has at least g++ version 4.9 installed (you may check it by issuing the command "g++ -v" in terminal). Ubuntu minimum supported version is 15.04 (older versions can be updated to use g++ 4.9 but this is not the default configuration). If only g++ version 4.8 is available, it is usually possible to use it if the user change the "-std=gnu++14" directive to "-std=gnu++11" in the GLOBAL_CC parameter in LMM menu "Model>System Options". 

Also make sure if Tcl/Tk version 8.6 is present (use the command "echo 'puts $tcl_version;exit 0' | tclsh" to get the installed version). If a different version is present, but at least version 8.5, the user must change the TCL_VERSION parameter in LMM menu "Model>System Options" to the appropriate value.

To run LMM from a system shell, please open a terminal in the installation directory (or use your graphical file browser) and execute:

./lmm

If you get an error when trying to execute any of the above commands, please make sure the respective files are set as executable (use terminal command "chmod +x FILENAME" in the installation directory, replacing FILENAME by the name of the corresponding file).

You may need to recompile LMM if the included pre-compiled versions have problems with your Linux setup (a 32-bit distribution, for instance). Move in the LSD installation directory and use the command:

make -f makefile.linux

If the compilation fails, the most likely reason is the mis-specification of the locations of the files required for the compilation. The major problem is that Tcl/Tk may be installed in your systems in several different locations. The makefile contains a list of variables for the directory needed for the Tcl/Tk libraries and include files. For example, on some systems you have the Tcl/Tk library located in /usr/lib, or /usr/local/lib, or usr/share/lib, etc. Similarly, the include files may be located in different directories. The makefile lists the files you need to identify; check the location for those files and edit the makefile as appropriate for your system. 

It is also possible that Tcl/Tk requires further libraries besides those specified in the makefile. If you have errors even after having specified the correct path to the Tcl/Tk libraries, then find out where the file wish is located (using the command "whereis wish"), and then find out which libraries are used with the command:

ldd /usr/bin/wish

If the system lists further libraries, add the appropriate option to the linker (e.g. -lieee to add the library libieee.a) in the makefile to the variable DUMMY.

IMPORTANT: if you modified the makefile to compile LMM, the same changes need to be made to the makefiles used to generate the LSD Model programs. You need to make these changes only once using LMM. Use the menu item System Compilation Options in menu Model. You will have to fill the same variables as in the makefile used to compile LMM, which must be set to the same values.


***************************
macOS (10.10+) Installation
***************************

LSD is only supported in mac computers as a native macOS (Aqua) application. 

To install LSD, simply unzip the LSD distribution file (e.g., Lsd-7.2-master.zip) to the chosen directory (in most cases  "~/", the user home directory, is recommended) using Finder. This will create the whole LSD folder structure. Take note of the name of the main (topmost) folder where LSD is installed. The distribution file may be deleted after the extraction.

Next, open the Terminal application (located inside the Utilities app folder), and type the following commands (each line must be completed by pressing "Enter"):

cd ~/Lsd-7.2-master
./add-shortcut-mac.sh

After a successful installation, a desktop shortcut (icon) will be available for using LMM/LSD.
 
Users of macOS Sierra (10.12) or newer MUST yet manually install the Apple Command Line Tools package (the full Xcode package is NOT required) to make the compiler and other required command line tools available in macOS. To install it, open the Terminal  and enter the following command:

xcode-select --install

Then, in the opened window, click on the "Install" button (do NOT click on the "Get Xcode" button), accept the license and wait the installation to complete (you may have to reboot to finish the installation).

It is recommended, but not required, to install multitail and Gnuplot applications. Gnuplot allows LSD to produce X-Y and other more elaborated plots but it is not otherwise needed to run LSD simulations. The easiest way to install Gnuplot is to use the Homebrew package manager for the installation. If you do not have Homebrew installed, at the terminal prompt, paste the following command and press ENTER:

ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"

After Homebrew installation finishes (details at http://brew.sh), you can install Gnuplot using the following command in Terminal (Qt framework will be automatically installed too):

brew install multitail gnuplot

IN CASE OF PROBLEMS:

If using macOS 10.12 (Sierra) or newer, you MUST use the "add-shortcut-mac.sh" command above to remove LMM and LSD from the system quarantine. Failing to do so will prevent from using LSD as a native macOS application.

If you delete or cannot create the LSD desktop shortcut, you can still run the app named LMM located inside the LSD installation directory (double click it in Finder) to open LMM/LSD, or try to rerun the "add-shortcut-mac.sh" script to recreate the shortcut.


************************
Manually configuring LSD
************************

After LSD installation, or when changing between operating systems or 32 and 64-bit modes, it may be necessary to (re-) configure LSD. To do so, after launching LMM, perform the following steps:

1. Select "Options..." in menu "File" then press the button "Default", change any configuration if desired, and press "OK". 

2. Select "System Options..." in menu "Model", press the button "Default", and "OK".


******************
Use of LMM and LSD
******************

LMM (LSD Model Manager) is a program used to manage LSD model programs. LSD (Laboratory for Simulation Development) model programs are stand-alone programs that execute fast and efficiently difference-equation simulation models. For a user to develop a new simulation model it is only requested to specify the equations of the model in a simplified C++ language, with the assistance of automatic help. LSD model programs generate automatically the code necessary to link the equations in a coherent sequence within a simulated time steps, saving and elaborating the result, allowing easy access to initialization values, and many other operations required for fully exploiting the simulation model.

When LMM starts the first operation is to choose a model to work with. Using the LMM's Model Browser you can either select one of the existing models, or create a new empty one (that is, no equations, variables etc.).

After a model is selected in Model Browser, you can ask LMM to:
- compile the model and run it;
- set compilation options
- edit the equations of the model;

ATTENTION: the very first time a model is compiled an error may be caused by the misspecification of the system directories or the operating system. In this case, use menu Model/System Compilation Option and use the default values for your operative system to let LMM adjust automatically the error.

When a model program is successfully compiled and run by LMM, then the user can interact with the LSD Browser interface. This window permits to control every aspect of the simulation run (e.g., setting initial values, observing and saving results, reading the model documentation etc.) except for the modification of the code for the equations, done in LMM. For this latter operation you need to close the LSD model program, tell LMM to show the equation file if necessary, edit the model's equations and compile/run a new LSD model program.

Please check LSD documentation using the menu Help at any time. The documentation includes from tutorials and a course on LSD to a complete manual covering all LSD details.

For persisting problems email us: valente@ec.univaq.it or mcper@unicamp.br