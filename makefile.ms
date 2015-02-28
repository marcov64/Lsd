# Makefile to compile LMM adapt as required. Under MS Windows system with installed Lsd follow these steps:
# 1. Open a dos shell and move in the root of Lsd system, where this makefile is located.
# 2. Run the batch file "run.bat". This will start LMM. Type Esc and quit the program. This will add to the shell
#    the required path
# 3. type make -f makefile.ms. This will create a new LMM program using the source "modman.cpp" in directory "src"

TARGET=lmm
SWITCH_CC=


TCL_VERSION=85
TK_VERSION=85

# For MS Windows systems, using the GNU GCC (cygwin) in the Lsd installation.
LSDROOT=.
DUMMY=-mwindows
PATH_TCL_LIB=$(LSDROOT)/gnu/lib
PATH_TK_LIB=$(LSDROOT)/gnu/lib
PATH_TK_HEADER=$(LSDROOT)/gnu/include
PATH_TCL_HEADER=$(LSDROOT)/gnu/include
PATH_LIB=$(LSDROOT)/gnu/lib
INCLUDE_LIB=-I$(LSDROOT)/gnu/include
TCL_LIBRARY=$(LSDROOT)/gnu/lib/tcl8.5

# For Unix. 
#LSDROOT=.
#DUMMY=
#PATH_TCL_LIB=.
#PATH_TK_LIB=.
#PATH_TK_HEADER=.
#PATH_TCL_HEADER=.
#PATH_LIB=.
#INCLUDE_LIB=.


# from here you should not need to modify anything
CC=gcc
SRC=src

SRC_DIR=$(LSDROOT)/$(SRC)/

# Variable used to prepare the options to search for the tcl and tk headers
# Do not modify
INCLD_TCL_TK=-I$(PATH_TCL_HEADER) -I$(PATH_TK_HEADER) 


# Set the complete option for the libraries to link to the models.
TCL_LIB=-L$(PATH_TCL_LIB) -ltcl$(TCL_VERSION)
TK_LIB=-L$(PATH_TK_LIB) -ltk$(TK_VERSION)

$(TARGET): $(SRC_DIR)modman.o
	$(CC) $(SWITCH_CC)  $(SRC_DIR)modman.o \
	$(TCL_LIB) $(TK_LIB) $(EXTRA_PAR) -L$(PATH_LIB) -lm -lgdi32 -lcomdlg32 -lshell32 -o $(TARGET) $(DUMMY)

$(SRC_DIR)modman.o: $(SRC_DIR)modman.cpp 
	$(CC) $(SWITCH_CC) -c $(SRC_DIR)modman.cpp -o $(SRC_DIR)modman.o $(INCLD_TCL_TK) $(INCLUDE_LIB)










