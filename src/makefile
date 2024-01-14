#**************************************************************
#
#	LSD 9.0 - January 2024
#	written by Marco Valente, Universita' dell'Aquila
#	and by Marcelo Pereira, University of Campinas
#
#	Copyright Marco Valente and Marcelo Pereira
#	LSD is distributed under the GNU General Public License
#
#**************************************************************

#**************************************************************
# MAKEFILE
# Makefile for LMM
# In a command prompt (terminal) in LSD root directory, use
#  cd src
#  make
#**************************************************************

CC=g++
TARGET=LMM
SWITCH_CC=-D_LMM_ -D_NP_ -O3 -Wall \
-Wextra -Wno-unused-parameter -Wno-implicit-fallthrough

# from here you should not need to modify anything

# using GCC and Tcl/Tk in the Lsd installation
LSDROOT=..
SRC_DIR=

# handle platform-specific stuff
ifeq ($(OS),Windows_NT)
	OBJ_DIR=$(TMP)\$(TARGET)/
	PKG=
	EXT=.exe
	WRC=windres -D_WIN32
	MANIF=$(OBJ_DIR)lmm_manifest.o
	TOUCH=
	TCL_VERSION=86
	PATH_TCLTK_HEADER=$(LSDROOT)/gnu/include
	PATH_TCLTK_LIB=-L $(LSDROOT)/gnu/lib
	TCLTK_LIB=-ltcl$(TCL_VERSION) -ltk$(TCL_VERSION)
	SWITCH_CC+=-mthreads -mwindows -static-libstdc++
else
	ifeq ($(shell uname -s),Darwin)
		OBJ_DIR=$(TMPDIR)/$(TARGET)/
		PKG=$(TARGET).app/Contents/MacOS/
		EXT=
		MANIF=
		TOUCH=touch -c $(LSDROOT)/$(TARGET).app
		PATH_TCL_HEADER=$(SRC_DIR)LSD.app/Contents/Frameworks/Tcl.framework/Headers
		PATH_TK_HEADER=$(SRC_DIR)LSD.app/Contents/Frameworks/Tk.framework/Headers
		PATH_TCLTK_LIB=-F $(SRC_DIR)LSD.app/Contents/Frameworks
		TCLTK_LIB=-framework Tcl -framework Tk
		SWITCH_CC+=-Wno-unused-result -std=c++14 -mmacosx-version-min=10.10
	else
		OBJ_DIR=$(TMPDIR)/$(TARGET)/
		PKG=
		EXT=
		MANIF=
		TOUCH=
		TCL_VERSION=8.6
		PATH_TCLTK_HEADER=/usr/include/tcl$(TCL_VERSION)
		PATH_TCLTK_LIB=-L /usr/lib/tcl$(TCL_VERSION)
		TCLTK_LIB=-ltcl$(TCL_VERSION) -ltk$(TCL_VERSION)
		SWITCH_CC+=-Wno-unused-result
	endif
endif

# variable to search for the tcl and tk headers
INCLUDE=-I$(PATH_TCLTK_HEADER)

# OS commands to delete files and create directories
RM=@rm$(EXT) -f
MKDIR=@mkdir$(EXT) -p

# make rules
all: $(LSDROOT)/$(TARGET)$(EXT)

$(LSDROOT)/$(PKG)$(TARGET)$(EXT): $(OBJ_DIR)modman.o $(OBJ_DIR)common.o $(MANIF)
	$(CC) $(SWITCH_CC) $(OBJ_DIR)modman.o $(OBJ_DIR)common.o $(MANIF) $(PATH_TCLTK_LIB) $(TCLTK_LIB) -o $(LSDROOT)/$(PKG)$(TARGET)$(EXT)
	$(TOUCH)

$(OBJ_DIR)modman.o: $(SRC_DIR)modman.cpp $(SRC_DIR)common.h
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(INCLUDE) -c $(SRC_DIR)modman.cpp -o $(OBJ_DIR)modman.o

$(OBJ_DIR)common.o: $(SRC_DIR)common.cpp $(SRC_DIR)common.h
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(INCLUDE) -c $(SRC_DIR)common.cpp -o $(OBJ_DIR)common.o

$(OBJ_DIR)lmm_manifest.o: $(SRC_DIR)lmm_manifest.rc $(SRC_DIR)lmm.exe.manifest
	$(MKDIR) $(@D)
	$(WRC) $(INCLUDE) -i $(SRC_DIR)lmm_manifest.rc -o $(OBJ_DIR)lmm_manifest.o

# delete object files
clean:
	$(RM) $(OBJ_DIR)modman.o $(OBJ_DIR)common.o $(MANIF)
