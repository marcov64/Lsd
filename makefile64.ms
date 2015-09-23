# Makefile to compile LMM adapt as required. Under MS Windows system with installed Lsd follow these steps:
# 1. Open a dos shell and move in the root of Lsd system, where this makefile is located.
# 2. Run the batch file "run64.bat". This will start LMM. Type Esc and quit the program. This will add to the shell
#    the required path
# 3. type make -f makefile64.ms. This will create a new LMM program using the source "modman.cpp" in directory "src"

TARGET=lmm64
SWITCH_CC=

TCL_VERSION=86
TK_VERSION=86

# For MS Windows 64-bit systems ONLY, using the GNU 64-bit GCC (cygwin64/mingw-w64) in the Lsd installation.
LSDROOT=.
DUMMY=-mwindows
PATH_TCL_LIB=$(LSDROOT)/gnu64/lib
PATH_TK_LIB=$(LSDROOT)/gnu64/lib
PATH_TK_HEADER=$(LSDROOT)/gnu64/include
PATH_TCL_HEADER=$(LSDROOT)/gnu64/include
PATH_LIB=$(LSDROOT)/gnu64/lib
INCLUDE_LIB=-I$(LSDROOT)/gnu64/include

# from here you should not need to modify anything
CC=gcc
RM=rm -f
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
	$(TCL_LIB) $(TK_LIB) $(EXTRA_PAR) -L$(PATH_LIB) -o $(TARGET) $(DUMMY)

$(SRC_DIR)modman.o: $(SRC_DIR)modman.cpp 
	$(CC) $(SWITCH_CC) -c $(SRC_DIR)modman.cpp -o $(SRC_DIR)modman.o $(INCLD_TCL_TK) $(INCLUDE_LIB)

# Clean object files for building for another distribution
clean:
	$(RM) $(SRC_DIR)modman.o
