Laboratory for Simulation Development - Lsd
and
Lsd Model Manager - LMM
August 2015

Version 7.0 alpha
by Marco Valente - marco.valente@univaq.it

This Readme.txt file contains:
- Brief introduction on Lsd
- Description of the installation content
- Instructions for the installation (Windows, MacOS X and Linux)
- Hints on the use of LMM and Lsd.


************
What is Lsd?
************

Lsd is based on the assumption that simulation models are distinguished from simulation programs. Using Lsd a user is concerned exclusively with the description of the theoretical model, while all the technicalities of the program implementing the model are automatically generated in an intuitive and computationally efficient way.

Lsd can be considered as a modelling language. Using Lsd a user is required to describe a model as if it were a system of discrete equations, where each variable is associated to piece of code (usually short and simple), indicating how the generic instance of the variable must compute its value at a generic time step. No other coding is required (e.g. to define the simulation cycle, saving data series, etc.). The required programming skills to use Lsd are therefore solely dependent on the complexity of the equations of the model.

The code for the equations is expressed in a highly symbolic form, using a development environment providing assistance and examples. From this code the system automatically generates a program endowed with a complete set of interfaces to exploit the model. With such interfaces the user can: define the elements of the model, run simulations in various modes, inspect in any details events at run time, being informed on errors, analyse the results, generate automatic documentation.

The user interfaces and the skills required to implement simple models can be learned in a few days of training. Being based on C++ a Lsd model can both express any computational content, and make use of existing libraries, generating extremely fast and efficient code. Lsd is an opensource project and makes use of other open source tools, as the GNU C++ compiler, Tcl/Tk windowing toolkit, gnuplot graphical package. Lsd is available natively for Linux, Windows and Mac OS X systems, with minimal requirements.

************************
Lsd installation content
************************

This distribution contains:
- Lsd source code, for the creation of simulation models
- Several Lsd example models
- Lsd Model Manager (LMM), a developing environment for Lsd models
- Documentation, as context dependent help pages 
- (Windows 32-bit version only) GNU C++ compiler and other GNU stuff. 

Lsd source code
The Lsd source files are usually managed (e.g. for compilation) automatically, so that users need not to worry for technical issues. Upgrading to new versions of Lsd (which always guarantees backward compatibility) necessitates only to replace the source files.

Lsd example models
Several models contained in the distribution provide examples on the design of a whole models or can be used to copy-and-past single equations or chunks of code.

LMM
Lsd models can be developed using a text editor and pre-defined compilation instructions (i.e. makefile), so that users are allowed to use their preferred coding environment. However, the distribution contains a simple IDE providing all the functionalities required by the (limited) programming needs of a Lsd model project. LMM permits to manage easily several modelling projects, develop Lsd models' code with extensive assistance, exploit debugging tools.
 
Documentation
All the documentation is accessible through the help menus, presented as HTML pages. In particular, there documentation is made of:
- The Lsd manual covers the interfaces for using Lsd model programs.
- The LMM manual concerns the use of the LMM environment to develop Lsd model programs.
- A few documents describe how to use Lsd. See the Documentation entry in the menu Help of LMM.
- The distributed Lsd models produce automatically their own documentation.

MS Windows users
Contrary to Linux and Mac systems, MS Windows systems normally do not contain C++ programming tools (compiler, libraries, debugger, etc.). The Windows distribution of Lsd contains a selection of the MinGW 32-bit package with all (and only) the packages required to use Lsd, so that no extra software must be installed to use Lsd on such systems. 

Optionally, Windows x64 users may use LMM/Lsd under native 64-bit support. This is not required, as the standard 32-bit version also works in the 64-bit versions of Windows (Vista/7/8/10). However, the 64-bit version of Lsd is reasonably faster and supports models accessing far more memory (in practice, limited only by the amount of available RAM).


***********************************
MS Windows Installation (32/64-bit)
***********************************
The installation consists simply in unpacking the Lsd files that are structured in a root directory (e.g. C:\Lsd) and several subdirectories for the models, manuals and source code. 

*** IMPORTANT ***
Lsd cannot be installed within a directory (or having an ancestor directory) containing a space in its name. For example, the directory:
C:\Documents and Settings\Lsd
cannot work. If you installed the system in such a wrong directory, simply move the whole Lsd directory in another location.
*****************

When the installation is completed, run this file inside the Lsd directory (double clicking in Windows Explorer is fine):

> run.bat

in the installation directory. This will run LMM (Lsd Model Manager) that allows to create new models, or select existing models. Never invoke the file "lmm.exe" directly, because it needs some environment variables to be set by "run.bat".

If you have the full Cygwin or MinGW 32-bit distribution installed and want to use it instead of the minimum version installed with Lsd, please make sure you have it minimally configured with the packages "gcc-core", "gcc-g++" (C++ compiler), "zlib-devel" (library), "make" (make tool) and "gdb". Then execute "config-cygwin32.bat" to remove the version of these tools that comes with Lsd. Please use the guide below (for 64-bit) as a reference to the process of setting up Cygwin 32-bit. Don't forget to have your preferred tools included in your PATH environment variable.

In some cases, the included minimum tool set may not work properly with your particular Windows configuration (double-clicking not working, graphical glitches, compiler errors). In this case, you may need to perform a full installation of Cygwin 32-bits. Please use the guide below (for 64-bit) as a reference to the process of setting it up.


*************************************
MS Windows Installation (64-bit only)
*************************************

To use the 64-bit of Lsd, additionally, you need the Cygwin 64-bit version installed plus some optional packages (or another compatible GNU gcc 4.9+ compiler). The installer can be downloaded at http://www.cygwin.com/ (make sure you download the "setup-x86_64.exe" file). Install it to any appropriate directory (the same restrictions mentioned above, about folder names with spaces, also apply to Cygwin). Verify in the installer that "gcc-core", "gcc-g++" (C++ compiler), "zlib-devel" (library), "make" (make tool) and "gdb" (debugger), all 64-bit versions (x86_64) were selected for installation in the "Select Packages" window of Cygwin Setup. Alternatively, MinGW gcc-g++ 64-bit (plus zlib, make and gdb) can also be used. You don't have to install Tcl/Tk or gnuplot in Cygwin, the required versions are already included with Lsd.

After installation check that Cygwin (or MinGW) was added to the PATH environment variable. To do that, in Windows open "Control Panel", sequentially select "System and Security", "System", "Advanced system settings", "Advanced" tab and then "Environment Variables...". In the "System variables" list, select "Path" and press "Edit..." (be carefull to NOT DELETE the existing text). Run across the line to see if your Cygwin (or MinGW) bin folder, i.e. "C:\cygwin64\bin", is already there. If Yes, just press "Cancel" 3 times. If not, at the end of the "Variable value" field type ";" (next to the existing text) and add your Cygwin bin folder. Press "Ok" 3 times and you are done. Make sure you don't have any older version of gcc ahead of your Cygwin/MinGW bin folder in PATH.

When the installation is completed, run this file inside the Lsd directory (double clicking in Windows Explorer is fine):

> run64.bat

in the installation directory. This will run LMM (Lsd Model Manager) that allows to create new models, or select existing models. Never invoke the file "lmm.exe" or "lmm64.exe" directly, because they need some environment variables to be set by "run.bat"/"run64.bat".

If you switch between the 32 and 64-bit versions, make sure you adjust the "System Compilation Options" (in the "Model" menu in LMM), at least by clicking on the button "Default Windows[64]".

Please note that you need to FULLY install Cygwin64/MinGW64 to use Lsd 64-bit. Just copying the minimum required .dll files is not enough because full compiler support is needed.

 
*****************
Unix Installation
*****************
To use the Lsd system it is necessary to have a GNU GCC compiler with the standard libraries and the Tcl/Tk 8.5 package; you likely have already Tcl/Tk installed on your system, but you also need the "development" package for your Tcl/Tk version. Use your preferred package manager to get the 'dev' package. In Debian or Ubuntu, you can use "sudo apt-get install tcl8.5-dev" and  "sudo apt-get install tk8.5-dev". Remember you need the packages to match the architecture (32 or 64-bit) you are using for Lsd (apt-get manages that transparently).

Though not striclty necessary, it is also suggested to have the GDB debugger (for low-level inspection of a simulation working) and the gnuplot graphical package. 

To unpack the file, e.g. LsdXX.zip, simple unzip it in the chosen directory.
This will create the whole directory structure. 

Optionally, you can recompile LMM, in particular if the included pre-compiled version has problems with your Unix setup. Move in the new Lsd directory and use the makefile "makefile.ln" (64/32-bit, native) or "makefile32.ln" for creating a 32-bit version in a 64-bit machine:

# make -f makefile.ln

To run LMM, please open a terminal in the chosen installation directory (or use your graphical file browser) and execute:

# ./lmm

Lsd includes two versions of LMM precompiled for Linux systems. "lmm" is the 32-bit version and should run in any machine (32 or 64-bit) if the appropriate 32-bit tools and libraries are installed. "lmm64" is the 64-bit only version and requires 64-bit tools and libraries. In Debian/Ubuntu, apt-get selects packages automatically according to the installed architecture of Linux (32 or 64-bit), so you should use "lmm" if using Linux 32-bit or "lmm64" for 64-bit OS installations.

If the compilation fails the most likely reason is the mis-specification of the locations of the files required for the compilation. The major problem is that Tcl/Tk may be installed in your systems in several different locations. The makefile contains a list of variables for the directory needed for the Tcl/Tk libraries and include files. For example, on some systems you have the Tcl/Tk library located in /usr/lib, or /usr/local/lib, or usr/share/lib, etc. Similary, the include files may be located in different directories. The makefile lists the files you need to identify; check the location for those files and edit the makefile as appropriate for your system. 

It is also possible that Tcl/Tk requires further libraries besides those specified in the makefile. If you have errors even after having specified the correct path to the Tcl/Tk libraries, then find out where the file wish is located (using the command "whereis wish"), and then find out which libraries are used with the command:

# ldd /usr/bin/wish

If the system lists further libraries, add the appropriate option to the linker (e.g. -lieee to add the library libieee.a) in the makefile to the variable DUMMY>

!!!!!!!! IMPORTANT !!!!!!!!!!!!
If you modified the makefile to compile LMM, the same changes will need to be made to the makefiles used to generate the Lsd Model Programs. You need to make these changes only once using a command in LMM. Use the menu item System Compilation Options in menu Model. You will have the same variables as in the makefile used to compile LMM that must be set to the same values.


*********************************
Mac OS X (Mavericks) Installation
*********************************
Mac users have two options. Either compile Lsd as a Unix system, or use the package native for Mac OS X. In both cases you need to install the X11/XQuartz package (see the help on your Mac documentation or http://xquartz.macosforge.org). 

In the first case you also need to install the Tcl/Tk 8.5 developer toolkit, as described above. Then, open a terminal and follow the remaining instructions for installing Lsd on Unix systems. For updates and tips see:

http://andre.lorentz.pagesperso-orange.fr/Site/%5BLSD_on_a_Mac%5D.html

For a native Mac look&feel you need to install the ActiveState ActiveTcl distribution of Tcl/Tk 8.5 (http://www.activestate.com/activetcl/downloads). Unpack the Lsd distribution on your preferred location. 

Optionally, you can recompile LMM, in particular if the included pre-compiled version has problems with your Mac setup. Open a terminal and go to the directory of Lsd installation. Then use the command:

> make -f makefile.osx

To run LMM, please open a terminal in the chosen installation directory and execute:

> ./lmmOSX

or simply click on its icon from the Finder.


******************
Use of LMM and Lsd
******************

LMM (Lsd Model Manager) is a program used to manage Lsd model programs. Lsd (Laboratory for Simulation Development) model programs are stand-alone programs that execute fast and efficiently difference-equation simulation models. For a user to develop a new simulation model it is only requested to specify the equations of the model in a simplified C++ language, with the assistance of automatic help. Lsd model programs generate automatically the code necessary to link the equations in a coherent sequence within a simulated time steps, saving and elaborating the result, allowing easy access to initialization values, and many other operations required for fully exploiting the simulation model. Both LMM and Lsd offer extensive manual pages for each operation available.

When LMM starts the first operation is to choose a model to work with. Using the LMM's Models' Browser you can either select one of the existing models, or create a new empty one  (that is, no equations, variables etc.). When a model is selected you can ask LMM to:
- compile the model and run it;
- set compilation options
- edit the equations of the model;
- debug the code of the equations with gdb;

Besides this model-specific operations, LMM is also a standard editor for creating and editing text files.

ATTENTION: the very first time a model is compiled an error can be caused by the misspecification of the system directories. In this case, use menu Model/System Compilation Option and use the default values for your operative system to let LMM adjust automatically the error.

When a model program is successfully compiled and run by LMM it can interact with the user with the Lsd interfaces. These permit to control every aspect of the simulation runs (e.g. setting initial values, observing and saving results, reading the model documentation etc.) but for the modification of the code for the equations. For this latter operation you need to close the Lsd model program, tell LMM to show the equation file, edit it and re-compile a new Lsd model program.


For persisting problems email us: valente@ec.univaq.it or marcelocpereira@uol.com.br
(is anybody willing to write a stupid configure for Lsd?)


Legal stuff
Lsd is copyright by Marco Valente and is distributed according to the GNU General Public License. That is, as I understand it, you can use, modify and redistribute this code for free, as long as you maintain the same conditions. For legal conditions on gnuplot and the other software used see their legal notices

