**************************************************************

	LSD 7.1 - December 2018
	written by Marco Valente, Universita' dell'Aquila
	and by Marcelo Pereira, University of Campinas

	Copyright Marco Valente
	LSD is distributed under the GNU General Public License
	
**************************************************************

***********
Legal stuff
***********

LSD is copyrighted by Marco Valente and is distributed according to the GNU General Public License. That is, as I understand it, you can use, modify and redistribute this code for free, as long as you maintain the same conditions. For legal conditions on gnuplot and the other software used see their legal notices.

LSD includes third party software under the license of the copyright owners:

Eigen 3.3.4 is copyrighted by Benoit Jacob and others under a MPL2 license, http://eigen.tuxfamily.org
NOLHDesigns_v6 is copyrighted by Susan M. Sanchez under a GPL 2.1 license, http://harvest.nps.edu
NOB_Mixed_512DP_v1 is copyrighted by Helcio Vieira under a GPL 2.1 license, http://harvest.nps.edu
SetFileIcon 0.1 is copyrighted by HAMSoft Engineering, http://hamsoftengineering.com
Shortcut.exe 1.11 is copyrighted by Marty List, http://www.OptimumX.com
Silk icon set 1.3 is copyrighted by Mark James under a CCA 2.5 license, http://www.famfamfam.com/lab/icons/silk


********
Contents
********

This Readme.txt file contains:
- Brief introduction on LSD
- Description of the installation content
- Instructions for the installation (Windows, MacOS X and Linux)
- Hints on the use of LMM and LSD.


************
What is LSD?
************

LSD is based on the assumption that simulation models are distinguished from simulation programs. Using LSD a user is concerned exclusively with the description of the theoretical model, while all the technicalities of the program implementing the model are automatically generated in an intuitive and computationally efficient way.

LSD can be considered as a modelling language. Using LSD a user is required to describe a model as if it were a system of discrete equations, where each variable is associated to piece of code (usually short and simple), indicating how the generic instance of the variable must compute its value at a generic time step. No other coding is required (e.g. to define the simulation cycle, saving data series, etc.). The required programming skills to use LSD are therefore solely dependent on the complexity of the equations of the model.

The code for the equations is expressed in a highly symbolic form, using a development environment providing assistance and examples. From this code the system automatically generates a program endowed with a complete set of interfaces to exploit the model. With such interfaces the user can: define the elements of the model, run simulations in various modes, inspect in any details events at run time, being informed on errors, analyse the results, generate automatic documentation.

The user interfaces and the skills required to implement simple models can be learned in a few days of training. Being based on C++ a LSD model can both express any computational content, and make use of existing libraries, generating extremely fast and efficient code. LSD is an opensource project and makes use of other open source tools, as the GNU C++ compiler, Tcl/Tk windowing toolkit, gnuplot graphical package. LSD is available natively for Linux, Windows and Mac OS X systems, with minimal requirements.

************************
LSD installation content
************************

This distribution contains:
- LSD source code, for the creation of simulation models
- Several LSD example models
- LSD Model Manager (LMM), a development environment for LSD models
- Documentation, as context dependent help pages 
- (Windows 32-bit version only) GNU C++ compiler and other GNU stuff

LSD source code
The LSD source files are usually managed (e.g. for compilation) automatically, so that users need not to worry for technical issues. Upgrading to new versions of LSD (which always guarantees backward compatibility) necessitates only to replace the source files.

LSD example models
Several models contained in the distribution provide examples on the design of a whole models or can be used to copy-and-past single equations or chunks of code.

LMM
LSD models can be developed using a text editor and pre-defined compilation instructions (i.e. makefile), so that users are allowed to use their preferred coding environment. However, the distribution contains a simple IDE providing all the functionalities required by the (limited) programming needs of a LSD model project. LMM permits to manage easily several modeling projects, develop LSD models' code with extensive assistance, exploit debugging tools.
 
Documentation
All the documentation is accessible through the help menus, presented as HTML pages. In particular, there documentation is made of:
- The LSD manual covers the interfaces for using LSD model programs.
- The LMM manual concerns the use of the LMM environment to develop LSD model programs.
- A few documents describe how to use LSD. See the Documentation entry in the menu Help of LMM.
- The distributed LSD models produce automatically their own documentation.

First time configuration
After LSD installation it may be necessary to configure the system to your operating system. After launching LMM, select "Options..." in menu "File" then press the button "Default", change any configuration if needed and then press "OK". Next select "System Options..." in menu "Model", press the button corresponding to your system and then "OK". Please redo these steps if reinstalling LSD.

MS Windows users
Contrary to Linux and Mac systems, MS Windows systems normally do not contain C++ programming tools (compiler, libraries, debugger, etc.). The Windows distribution of LSD contains a selection of the MinGW 32-bit package with all (and only) the packages required to use LSD, so that no extra software must be installed to use LSD on such systems. 

Optionally, Windows x64 users may use LMM/LSD under native 64-bit support. This is not required, as the standard 32-bit version also works in the 64-bit versions of Windows (Vista/7/8/10). However, the 64-bit version of LSD is reasonably faster and supports models accessing far more memory (in practice, limited only by the amount of available RAM).


***********************************
MS Windows Installation (32/64-bit)
***********************************
To unpack the LSD distribution file, e.g., Lsd-7.1-master.zip, simply unzip it in the chosen directory (in most cases  "C:\" is the best option) using Windows Explorer. This will create the whole directory structure. The distribution file may be deleted after installation.


*** IMPORTANT ***
LSD cannot be installed within a directory (or having an ancestor directory) containing a space in its name. For example, the directory:
C:\Documents and Settings\Lsd
cannot work. If you installed the system in such a wrong directory, simply move the whole LSD directory structure in another location (drag and drop in Windows Explorer).
*****************

*** WARNING ***
LSD operates by creating and modifying executable (.exe) files for your simulation models. This kind of procedure may be INCORRECTLY detected as suspicious by some antivirus software and incorrect operation of LSD may arise, particularly when you try to compile and/or run your model. In this case, you may try to reconfigure your antivirus to allow LSD operation or try to TEMPORARILY disable it entirely. Microsoft standard antivirus software (included with Windows 8/10 and available as a download for Vista/7) is known to operate properly with LSD.
***************

If you want to create a desktop link (icon) to run LSD/LMM, you can use the batch file available in the installation directory executing the command (or double-clicking it in Windows Explorer):

add-shortcut.bat

Double-clicking the created desktop icon will run LMM (LSD Model Manager) in 32-bit mode, which allows to create new models, or select existing models. Never invoke the file "lmm.exe" directly, because it needs some environment variables to be set by "run.bat".

Alternatively, you may run this file inside the LSD directory (double-clicking in Windows Explorer is fine too) to run LMM:

run.bat

If you have the full Cygwin or MinGW 32-bit distribution installed and want to use it instead of the minimum version installed with LSD, please make sure you have it minimally configured with the packages "gcc-core", "gcc-g++" (C++ compiler), "zlib-devel" (library), "make" (make tool) and "gdb". Then execute "config-cygwin32.bat" to remove the version of these tools that comes with LSD. Please use the guide below (for 64-bit) as a reference to the process of setting up Cygwin 32-bit. Don't forget to have your preferred tools included in your PATH environment variable.

In some rare cases, the included minimum tool set may not work properly with your particular Windows configuration (double-clicking not working, graphical glitches, compiler errors). In this case, you may need to perform a full installation of Cygwin 32-bits. Please use the guide below (for 64-bit) as a reference to the process of setting it up.

You may have to install Gnuplot if you want to produce more elaborated plots in LSD. The required installer (64-bit preferred, even for LSD 32-bit installation) can be downloaded from:

http://www.gnuplot.info

When download finishes, simply run the downloaded installer, accept defaults EXCEPT the option "Add application directory to your PATH environment variable" which MUST be checked.


*************************************
MS Windows Installation (64-bit only)
*************************************

To use the 64-bit of LSD, additionally, you need the GNU 64-bit C++ compiler installed (cygwin64+mingw or mingw-w64 versions are fine). The Cygwin installer can be downloaded at http://www.cygwin.com/ (make sure you download the "setup-x86_64.exe" file). Preferably, install Cygwin to the default directory ("C:\cygwin64"). If installing to a different directory please note that the same restrictions mentioned above, about folder names with spaces, also apply to Cygwin. Additionally, check the packages "mingw64-x86_64-gcc-g++" (C++ compiler) and "mingw64-x86_64-zlib" (compression library), which are not selected in default installation, in the "Select Packages" window of Cygwin Setup. You don't have to install Tcl/Tk, make, gdb or gnuplot in Cygwin, the required versions are already included with LSD.

After installation you MUST add Cygwin to the PATH environment variable. To do that, open a Windows Command Prompt in the LSD installation directory (shift-right-click in Explorer, then select "Open command prompt here") and use the command (if not using the default, replace "C:\cygwin64" with your Cygwin installation directory):

add-to-path C:\cygwin64

Alternatively, or if you have problems with the command above, you can use Windows GUI to add Cygwin to the PATH. In Windows open "Control Panel", sequentially select "System and Security", "System", "Advanced system settings", "Advanced" tab and then "Environment Variables...". In the "System variables" list, select "Path" and press "Edit..." (be carefull to NOT DELETE the existing text). Run across the lines to see if your Cygwin bin folder, i.e. "C:\cygwin64\bin", is already there. If Yes, just press "Cancel" 3 times. If not, at the end of the "Variable value" field type ";" (next to the existing text) and add your Cygwin bin folder. Press "Ok" 3 times and you are done. Make sure you don't have any older version of gcc ahead of your Cygwin bin folder in PATH.

When the installation is completed, you can use the batch file available in the installation directory executing the command (or double-clicking it in Windows Explorer):

add-shortcut64.bat

Double-clicking the created desktop icon will run LMM (LSD Model Manager) in 64-bit mode, which allows to create new models, or select existing models.  Never invoke the file "lmm.exe" or "lmm64.exe" directly, because they need some environment variables to be set by "run.bat"/"run64.bat".

Alternatively, you may run this file inside the LSD directory (double-clicking in Windows Explorer is fine) to run LMM:

run64.bat

If you switch between the 32 and 64-bit versions, make sure you adjust the "System Compilation Options" (in the "Model" menu in LMM), at least by clicking on the button "Default Windows[32/64]".

Please note that you need a FULLY installed Cygwin64 to use LSD 64-bit. Just copying the minimum required .dll files is not enough because full compiler support is needed.

 
******************
Linux Installation
******************

To use the LSD system it is necessary to have the GNU gcc/g++ compiler (version 4.9+) with the standard libraries, including zlib and Tcl/Tk 8.6 packages; you likely have zlib/Tcl/Tk already installed on your system but you may need the development packages. Use your preferred package manager to get the 'dev' package versions and beware of 32/64-bit variants according to your architecture. Though not strictly necessary, it is also suggested to have the gdb debugger (for low-level inspection of a simulation) and the gnuplot graphical package (for advanced graphics), preferably using Qt. 

In Debian or Ubuntu, to make sure you have the correct libraries you can use:

sudo apt-get install build-essential gdb gnuplot-qt zlib1g-dev tcl8.6-dev tk8.6-dev

In Fedora or CentOS, the equivalent command is:

sudo yum install gcc-c++ make gdb gnuplot zlib-devel tcl tk tcl-devel tk-devel

Please check your configuration has at least g++ version 4.9 installed (you may check it by issuing the command "g++ -v" in terminal). Ubuntu minimum supported version is 15.04 (older versions can be updated to use g++ 4.9 but this is not the default configuration). If only g++ version 4.8 is available, it is usually possible to use it if the user change the "-std=gnu++14" directive to "-std=gnu++11" in the GLOBAL_CC parameter in LMM menu "Model>System Options". 

Also check if Tcl/Tk version 8.6 is present (use the command "echo 'puts $tcl_version;exit 0' | tclsh" to get the installed version). If a different version is present, but at least version 8.5, the user must change the TCL_VERSION parameter in LMM menu "Model>System Options" to the appropriate value.

To unpack the LSD distribution file, e.g., Lsd-7.1-master.tar.gz, simply copy it to the chosen directory (in most cases  "~/", the user home directory, is the best option) and unpack using your file manager. This will create the whole directory structure. Alternatively, to unpack the distribution file using the terminal can be done using (for ".tar.gz" extension files only):

tar -xzf Lsd-7.1-master.tar.gz

The distribution file may be deleted after installation.

If you want to create a desktop link (icon) to run LSD/LMM, you can use the script available in the installation directory executing the command (Ubuntu and Gnome based distributions):

./add-shortcut-linux.sh

To run LMM from a system shell, please open a terminal in the installation directory (or use your graphical file browser) and execute:

./lmm

If you get an error when trying to execute any of the above commands, please make sure the respective files are set as executable (use terminal command "chmod +x FILENAME" in the installation directory, replacing FILENAME by the name of the corresponding file).

You may need to recompile LMM if the included pre-compiled versions have problems with your Linux setup (32-bit distribution, for instance). Move in the new LSD directory and use the makefile "makefile.ln":

make -f makefile.ln

If the compilation fails, the most likely reason is the mis-specification of the locations of the files required for the compilation. The major problem is that Tcl/Tk may be installed in your systems in several different locations. The makefile contains a list of variables for the directory needed for the Tcl/Tk libraries and include files. For example, on some systems you have the Tcl/Tk library located in /usr/lib, or /usr/local/lib, or usr/share/lib, etc. Similary, the include files may be located in different directories. The makefile lists the files you need to identify; check the location for those files and edit the makefile as appropriate for your system. 

It is also possible that Tcl/Tk requires further libraries besides those specified in the makefile. If you have errors even after having specified the correct path to the Tcl/Tk libraries, then find out where the file wish is located (using the command "whereis wish"), and then find out which libraries are used with the command:

ldd /usr/bin/wish

If the system lists further libraries, add the appropriate option to the linker (e.g. -lieee to add the library libieee.a) in the makefile to the variable DUMMY.

!!!!!!!! IMPORTANT !!!!!!!!!!!!
If you modified the makefile to compile LMM, the same changes need to be made to the makefiles used to generate the LSD Model Programs. You need to make these changes only once using a command in LMM. Use the menu item System Compilation Options in menu Model. You will have the same variables as in the makefile used to compile LMM that must be set to the same values.


***************************
macOS (10.10+) Installation
***************************

Mac users have two installation options. Most users should use the recommended native application for macOS (Aqua), as presented here. 

To unpack the LSD distribution file, e.g., Lsd-7.1-master.zip, simply extract it to the chosen directory (in most cases  "~/", the user home directory, is the best option) using Finder. This will create the whole LSD folder structure. Take note of the name of the main (topmost) folder where LSD is installed. The distribution file may be deleted after the extraction.

Next, open the Terminal application (located inside the app folder Utilities), go to the main folder where LSD was extracted (replace "Lsd-7.1-master" with the correct name), and execute the installation script available there:

cd ~/Lsd-7.1-master
./add-shortcut-mac.sh

If using macOS 10.12 (Sierra) or newer, you MUST use the command above to remove LMM and LSD from the system quarantine. Failing to do so will prevent the usage of LSD as a native application. After the initial configuration, a desktop shortcut (icon) will be available for using LMM/LSD. If you delete the created shortcut, you can still run the app named LMM located inside the LSD installation directory (double click it in Finder) to open LMM/LSD, or rerun the add-shortcut-mac.sh script to recreate the shortcut.

Users of macOS Sierra (10.12) or newer MUST yet manually install the Apple Command Line Tools package (the full Xcode package is NOT required) to make the compiler and other required command line tools available in macOS. To install it, open the Terminal  and enter the following command:

xcode-select --install

Then, in the opened window, click on the Install button (do NOT click on the Get Xcode button), accept the license and wait the installation to complete (you may have to reboot to finish the installation).

You may have to install Gnuplot if you want to produce more elaborated plots in LSD. The easiest way is to use the Homebrew application for the installation. If you do not have Homebrew installed, at the terminal prompt, paste the following command and press ENTER:

/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"

When Homebrew installation finishes (details at http://brew.sh), you can install Gnuplot using the following command in Terminal (Qt framework will be automatically installed too):

brew install gnuplot --with-qt


*************************
macOS Legacy Installation
*************************

It is possible, but not required or recommended in most cases, to compile LSD as a Unix system. To do so you need to install the X11/XQuartz package (see the help on your Mac documentation or http://xquartz.macosforge.org), update your Tcl/Tk package to version 8.6+ (the embedded version 8.5 in macOS is not adequate) (see instructions on https://www.tcl.tk), and make sure the Apple Command Line Tools package (or Xcode) is installed (see the instructions above).

When all prerequisites are installed, download the LSD distribution file, e.g., Lsd-7.1-master.zip, to your home folder ("~/") and extract it in Terminal:

unzip Lsd-7.1-master.zip

Next, still in Terminal, change to the folder where LSD was installed (replace "Lsd-7.1-master" with the correct name), and compile LMM:

cd Lsd-7.1-master
make -f makefile.mac-legacy

Now, you can run LMM from the terminal executing (you must be in LSD installation folder):

./lmmOSX

You still have to install Gnuplot if you want to produce more elaborated plots in LSD. See http://www.gnuplot.info for the details or use Homebrew to do it in a simpler way (see the instructions above).

Please note that models created with lmmOSX have slightly different configurations than models produced with the LMM native macOS application (see above). So, if changing from one version to the other, the user MUST manually update both the LSD system and model options. Reapplying the defaults for each case is susally fine.


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
