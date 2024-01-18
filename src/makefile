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
# Makefile for LMM and LSD GUI modules
# In a command prompt (terminal) in LSD root directory, use
#  cd src
#  make
#**************************************************************

# general settings
TARGET_LMM=LMM
TARGET_LSD=LSD

SWITCH_CC=-O3 -Wall -Wextra -Wno-unused-parameter -Wno-implicit-fallthrough
SWITCH_CC_LMM=-D_LMM_ -D_NP_
SWITCH_CC_LSD=

# LSD file locations
LSDROOT=..
SRC_DIR=$(LSDROOT)/src/
LIB_DIR=$(SRC_DIR)lib/

# handle platform-specific stuff
ifeq ($(OS),Windows_NT)
	TMP_DIR=$(TMP)
	EXE_EXT=.exe
	DLL_EXT=.dll
	WRC=windres -D_WIN32
	AR_DIR=$(LSDROOT)/gnu/x86_64-w64-mingw32/bin/
	MANIF_LMM=$(TMP_DIR)/$(TARGET_LMM)/lmm_manifest.o
	MANIF_LSD=$(TMP_DIR)/$(TARGET_LSD)/lsd_manifest.o
	TCL_VERSION=86
	PATH_TCLTK_HEADER=$(LSDROOT)/gnu/include
	PATH_TCLTK_LIB=$(LSDROOT)/gnu/lib
	TCLTK_LIB=-ltcl$(TCL_VERSION) -ltk$(TCL_VERSION)
	SWITCH_CC+=-mthreads -mwindows
	SWITCH_CC_LMM+=-static-libstdc++
else
	ifeq ($(shell uname -s),Darwin)
		TMP_DIR=$(TMPDIR)
		PKG=$(TARGET_LMM).app/Contents/MacOS/
		DLL_EXT=.so
		TOUCH_LMM=touch -c $(LSDROOT)/$(TARGET_LMM).app
		TOUCH_LSD=touch -c $(LSDROOT)/$(TARGET_LSD).app
		PATH_TCL_HEADER=$(SRC_DIR)LSD.app/Contents/Frameworks/Tcl.framework/Headers
		PATH_TK_HEADER=$(SRC_DIR)LSD.app/Contents/Frameworks/Tk.framework/Headers
		PATH_TCLTK_LIB=$(SRC_DIR)LSD.app/Contents/Frameworks
		TCLTK_LIB=-framework Tcl -framework Tk
		SWITCH_CC+=-Wno-unused-result -std=c++14 -mmacosx-version-min=10.10
	else
		TMP_DIR=$(TMPDIR)
		DLL_EXT=.so
		TCL_VERSION=8.6
		PATH_TCLTK_HEADER=/usr/include/tcl$(TCL_VERSION)
		PATH_TCLTK_LIB=/usr/lib/tcl$(TCL_VERSION)
		TCLTK_LIB=-ltcl$(TCL_VERSION) -ltk$(TCL_VERSION)
		SWITCH_CC+=-Wno-unused-result
	endif
endif

# additional OS-specific locations
OBJ_DIR_LMM=$(TMP_DIR)/$(TARGET_LMM)/
OBJ_DIR_LSD=$(TMP_DIR)/$(TARGET_LSD)/
INCLUDE=-I$(SRC). -I$(PATH_TCLTK_HEADER)
PATH_LIB=.
LIB=-lz


# OS-specific commands and definitions
CC=g++$(EXE_EXT)
AR=$(AR_DIR)ar$(EXE_EXT) -rcs
RM=@rm$(EXE_EXT) -f
MKDIR=@mkdir$(EXE_EXT) -p

# targets
.PHONY: all lmm lsd libdll libgui libbase clean cleanlmm cleanlsd
all: lmm lsd
lmm: $(LSDROOT)/$(TARGET_LMM)$(EXE_EXT)
lsd: $(LSDROOT)/$(TARGET_LSD)$(EXE_EXT)
libdll: $(LSDROOT)/lib$(TARGET_LSD)$(DLL_EXT)
libgui: $(SRC_DIR)libLSDgui.a
libbase: $(SRC_DIR)libLSDbase.a

# LMM executable
$(LSDROOT)/$(PKG)$(TARGET_LMM)$(EXE_EXT): $(OBJ_DIR_LMM)lmm.o $(OBJ_DIR_LMM)common.o $(OBJ_DIR_LMM)commonlib.o $(MANIF_LMM)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LMM) $(OBJ_DIR_LMM)lmm.o $(OBJ_DIR_LMM)common.o $(OBJ_DIR_LMM)commonlib.o $(MANIF_LMM) -L$(PATH_TCLTK_LIB) $(TCLTK_LIB) -L$(PATH_LIB) $(LIB) -o $(LSDROOT)/$(PKG)$(TARGET_LMM)$(EXE_EXT)
	$(TOUCH)

# LSD executable
$(LSDROOT)/$(TARGET_LSD)$(EXE_EXT): $(MANIF_LSD) $(SRC_DIR)libLSDgui.a $(LSDROOT)/lib$(TARGET_LSD)$(DLL_EXT)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(MANIF_LSD) -L$(LSDROOT) -l$(TARGET_LSD) -L$(SRC_DIR) -lLSDgui -L$(PATH_TCLTK_LIB) $(TCLTK_LIB) -L$(PATH_LIB) $(LIB) -o $(LSDROOT)/$(TARGET_LSD)$(EXE_EXT)

# LSD dynamic library
$(LSDROOT)/lib$(TARGET_LSD)$(DLL_EXT): $(OBJ_DIR_LSD)fun_dummy.o $(SRC_DIR)libLSDbase.a
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(OBJ_DIR_LSD)fun_dummy.o -L$(SRC_DIR) -Wl,--whole-archive -lLSDbase -Wl,--no-whole-archive -L$(PATH_LIB) $(LIB) -shared -o $(LSDROOT)/lib$(TARGET_LSD)$(DLL_EXT)

# LSD GUI static library
$(SRC_DIR)libLSDgui.a: $(OBJ_DIR_LSD)lsd.o $(OBJ_DIR_LSD)lsdmain.o $(OBJ_DIR_LSD)analysis.o $(OBJ_DIR_LSD)browser.o $(OBJ_DIR_LSD)common.o $(OBJ_DIR_LSD)debug.o $(OBJ_DIR_LSD)draw.o $(OBJ_DIR_LSD)edit.o $(OBJ_DIR_LSD)edit_dat.o $(OBJ_DIR_LSD)file.o $(OBJ_DIR_LSD)interf.o $(OBJ_DIR_LSD)lattice.o $(OBJ_DIR_LSD)log.o $(OBJ_DIR_LSD)nolhtables.o $(OBJ_DIR_LSD)report.o $(OBJ_DIR_LSD)runtime.o $(OBJ_DIR_LSD)set_all.o $(OBJ_DIR_LSD)sensitivity.o $(OBJ_DIR_LSD)show_eq.o $(OBJ_DIR_LSD)util.o
	$(AR) $(SRC_DIR)libLSDgui.a $(OBJ_DIR_LSD)lsd.o $(OBJ_DIR_LSD)lsdmain.o $(OBJ_DIR_LSD)analysis.o $(OBJ_DIR_LSD)browser.o $(OBJ_DIR_LSD)common.o $(OBJ_DIR_LSD)debug.o $(OBJ_DIR_LSD)draw.o $(OBJ_DIR_LSD)edit.o $(OBJ_DIR_LSD)edit_dat.o $(OBJ_DIR_LSD)file.o $(OBJ_DIR_LSD)interf.o $(OBJ_DIR_LSD)lattice.o $(OBJ_DIR_LSD)log.o $(OBJ_DIR_LSD)nolhtables.o $(OBJ_DIR_LSD)report.o $(OBJ_DIR_LSD)runtime.o $(OBJ_DIR_LSD)set_all.o $(OBJ_DIR_LSD)sensitivity.o $(OBJ_DIR_LSD)show_eq.o $(OBJ_DIR_LSD)util.o

# base static library for LSD dynamic library
$(SRC_DIR)libLSDbase.a: $(OBJ_DIR_LSD)libmain.o $(OBJ_DIR_LSD)commonlib.o $(OBJ_DIR_LSD)description.o $(OBJ_DIR_LSD)filelib.o $(OBJ_DIR_LSD)latticelib.o $(OBJ_DIR_LSD)math.o $(OBJ_DIR_LSD)nets.o $(OBJ_DIR_LSD)object.o $(OBJ_DIR_LSD)parallel.o $(OBJ_DIR_LSD)run.o $(OBJ_DIR_LSD)stattables.o $(OBJ_DIR_LSD)utillib.o $(OBJ_DIR_LSD)variab.o $(OBJ_DIR_LSD)pugixml.o
	$(AR) $(SRC_DIR)libLSDbase.a $(OBJ_DIR_LSD)libmain.o $(OBJ_DIR_LSD)commonlib.o $(OBJ_DIR_LSD)description.o $(OBJ_DIR_LSD)filelib.o $(OBJ_DIR_LSD)latticelib.o $(OBJ_DIR_LSD)math.o $(OBJ_DIR_LSD)nets.o $(OBJ_DIR_LSD)object.o $(OBJ_DIR_LSD)parallel.o $(OBJ_DIR_LSD)run.o $(OBJ_DIR_LSD)stattables.o $(OBJ_DIR_LSD)utillib.o $(OBJ_DIR_LSD)variab.o $(OBJ_DIR_LSD)pugixml.o

# LMM executable modules
$(OBJ_DIR_LMM)lmm.o: $(SRC_DIR)lmm.cpp $(SRC_DIR)LSD.h
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LMM) $(INCLUDE) -c $(SRC_DIR)lmm.cpp -o $(OBJ_DIR_LMM)lmm.o

$(OBJ_DIR_LMM)common.o: $(SRC_DIR)common.cpp $(SRC_DIR)LSD.h
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LMM) $(INCLUDE) -c $(SRC_DIR)common.cpp -o $(OBJ_DIR_LMM)common.o

$(OBJ_DIR_LMM)commonlib.o: $(LIB_DIR)commonlib.cpp $(SRC_DIR)LSD.h
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LMM) $(INCLUDE) -c $(LIB_DIR)commonlib.cpp -o $(OBJ_DIR_LMM)commonlib.o

$(OBJ_DIR_LMM)lmm_manifest.o: $(SRC_DIR)lmm_manifest.rc $(SRC_DIR)lmm.exe.manifest
	$(MKDIR) $(@D)
	$(WRC) $(INCLUDE) -i $(SRC_DIR)lmm_manifest.rc -o $(OBJ_DIR_LMM)lmm_manifest.o

# LSD dynamic library modules
$(OBJ_DIR_LSD)fun_dummy.o: $(SRC_DIR)fun_dummy.cpp $(SRC_DIR)libLSD.h $(SRC_DIR)check.h $(SRC_DIR)fun_head.h
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(INCLUDE) -c $(SRC_DIR)fun_dummy.cpp -o $(OBJ_DIR_LSD)fun_dummy.o

$(OBJ_DIR_LSD)libmain.o: $(LIB_DIR)libmain.cpp $(SRC_DIR)libLSD.h
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(LIB_DIR)libmain.cpp -o $(OBJ_DIR_LSD)libmain.o

$(OBJ_DIR_LSD)commonlib.o: $(LIB_DIR)commonlib.cpp $(SRC_DIR)libLSD.h
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(LIB_DIR)commonlib.cpp -o $(OBJ_DIR_LSD)commonlib.o

$(OBJ_DIR_LSD)description.o: $(LIB_DIR)description.cpp $(SRC_DIR)libLSD.h
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(LIB_DIR)description.cpp -o $(OBJ_DIR_LSD)description.o

$(OBJ_DIR_LSD)filelib.o: $(LIB_DIR)filelib.cpp $(SRC_DIR)libLSD.h
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(LIB_DIR)filelib.cpp -o $(OBJ_DIR_LSD)filelib.o

$(OBJ_DIR_LSD)latticelib.o: $(LIB_DIR)latticelib.cpp $(SRC_DIR)libLSD.h
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(LIB_DIR)latticelib.cpp -o $(OBJ_DIR_LSD)latticelib.o

$(OBJ_DIR_LSD)math.o: $(LIB_DIR)math.cpp $(SRC_DIR)libLSD.h
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(LIB_DIR)math.cpp -o $(OBJ_DIR_LSD)math.o

$(OBJ_DIR_LSD)nets.o: $(LIB_DIR)nets.cpp $(SRC_DIR)libLSD.h
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(LIB_DIR)nets.cpp -o $(OBJ_DIR_LSD)nets.o

$(OBJ_DIR_LSD)object.o: $(LIB_DIR)object.cpp $(SRC_DIR)libLSD.h
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(LIB_DIR)object.cpp -o $(OBJ_DIR_LSD)object.o

$(OBJ_DIR_LSD)parallel.o: $(LIB_DIR)parallel.cpp $(SRC_DIR)libLSD.h
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(LIB_DIR)parallel.cpp -o $(OBJ_DIR_LSD)parallel.o

$(OBJ_DIR_LSD)run.o: $(LIB_DIR)run.cpp $(SRC_DIR)libLSD.h
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(LIB_DIR)run.cpp -o $(OBJ_DIR_LSD)run.o

$(OBJ_DIR_LSD)stattables.o: $(LIB_DIR)stattables.cpp $(SRC_DIR)libLSD.h
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(LIB_DIR)stattables.cpp -o $(OBJ_DIR_LSD)stattables.o

$(OBJ_DIR_LSD)utillib.o: $(LIB_DIR)utillib.cpp $(SRC_DIR)libLSD.h
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(LIB_DIR)utillib.cpp -o $(OBJ_DIR_LSD)utillib.o

$(OBJ_DIR_LSD)variab.o: $(LIB_DIR)variab.cpp $(SRC_DIR)libLSD.h
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(LIB_DIR)variab.cpp -o $(OBJ_DIR_LSD)variab.o

$(OBJ_DIR_LSD)pugixml.o: $(SRC_DIR)pugixml/pugixml.cpp $(SRC_DIR)pugixml/pugixml.hpp $(SRC_DIR)pugixml/pugiconfig.hpp
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(SRC_DIR)pugixml/pugixml.cpp -o $(OBJ_DIR_LSD)pugixml.o

# LSD executable modules
$(OBJ_DIR_LSD)lsd.o: $(SRC_DIR)lsd.cpp $(SRC_DIR)LSD.h
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(SRC_DIR)lsd.cpp -o $(OBJ_DIR_LSD)lsd.o

$(OBJ_DIR_LSD)lsdmain.o: $(SRC_DIR)lsdmain.cpp $(SRC_DIR)LSD.h
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(SRC_DIR)lsdmain.cpp -o $(OBJ_DIR_LSD)lsdmain.o

$(OBJ_DIR_LSD)analysis.o: $(SRC_DIR)analysis.cpp $(SRC_DIR)LSD.h
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(SRC_DIR)analysis.cpp -o $(OBJ_DIR_LSD)analysis.o

$(OBJ_DIR_LSD)browser.o: $(SRC_DIR)browser.cpp $(SRC_DIR)LSD.h
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(SRC_DIR)browser.cpp -o $(OBJ_DIR_LSD)browser.o

$(OBJ_DIR_LSD)common.o: $(SRC_DIR)common.cpp $(SRC_DIR)LSD.h
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(SRC_DIR)common.cpp -o $(OBJ_DIR_LSD)common.o

$(OBJ_DIR_LSD)debug.o: $(SRC_DIR)debug.cpp $(SRC_DIR)LSD.h
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(SRC_DIR)debug.cpp -o $(OBJ_DIR_LSD)debug.o

$(OBJ_DIR_LSD)draw.o: $(SRC_DIR)draw.cpp $(SRC_DIR)LSD.h
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(SRC_DIR)draw.cpp -o $(OBJ_DIR_LSD)draw.o

$(OBJ_DIR_LSD)edit.o: $(SRC_DIR)edit.cpp $(SRC_DIR)LSD.h
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(SRC_DIR)edit.cpp -o $(OBJ_DIR_LSD)edit.o

$(OBJ_DIR_LSD)edit_dat.o: $(SRC_DIR)edit_dat.cpp $(SRC_DIR)LSD.h
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(SRC_DIR)edit_dat.cpp -o $(OBJ_DIR_LSD)edit_dat.o

$(OBJ_DIR_LSD)file.o: $(SRC_DIR)file.cpp $(SRC_DIR)LSD.h
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(SRC_DIR)file.cpp -o $(OBJ_DIR_LSD)file.o

$(OBJ_DIR_LSD)interf.o: $(SRC_DIR)interf.cpp $(SRC_DIR)LSD.h
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(SRC_DIR)interf.cpp -o $(OBJ_DIR_LSD)interf.o

$(OBJ_DIR_LSD)lattice.o: $(SRC_DIR)lattice.cpp $(SRC_DIR)LSD.h
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(SRC_DIR)lattice.cpp -o $(OBJ_DIR_LSD)lattice.o

$(OBJ_DIR_LSD)log.o: $(SRC_DIR)log.cpp $(SRC_DIR)LSD.h
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(SRC_DIR)log.cpp -o $(OBJ_DIR_LSD)log.o

$(OBJ_DIR_LSD)nolhtables.o: $(SRC_DIR)nolhtables.cpp $(SRC_DIR)LSD.h
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(SRC_DIR)nolhtables.cpp -o $(OBJ_DIR_LSD)nolhtables.o

$(OBJ_DIR_LSD)report.o: $(SRC_DIR)report.cpp $(SRC_DIR)LSD.h
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(SRC_DIR)report.cpp -o $(OBJ_DIR_LSD)report.o

$(OBJ_DIR_LSD)runtime.o: $(SRC_DIR)runtime.cpp $(SRC_DIR)LSD.h
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(SRC_DIR)runtime.cpp -o $(OBJ_DIR_LSD)runtime.o

$(OBJ_DIR_LSD)sensitivity.o: $(SRC_DIR)sensitivity.cpp $(SRC_DIR)LSD.h
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(SRC_DIR)sensitivity.cpp -o $(OBJ_DIR_LSD)sensitivity.o

$(OBJ_DIR_LSD)set_all.o: $(SRC_DIR)set_all.cpp $(SRC_DIR)LSD.h
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(SRC_DIR)set_all.cpp -o $(OBJ_DIR_LSD)set_all.o

$(OBJ_DIR_LSD)show_eq.o: $(SRC_DIR)show_eq.cpp $(SRC_DIR)LSD.h
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(SRC_DIR)show_eq.cpp -o $(OBJ_DIR_LSD)show_eq.o

$(OBJ_DIR_LSD)util.o: $(SRC_DIR)util.cpp $(SRC_DIR)LSD.h
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(SRC_DIR)util.cpp -o $(OBJ_DIR_LSD)util.o

$(OBJ_DIR_LSD)lsd_manifest.o: $(SRC_DIR)lsd_manifest.rc $(SRC_DIR)lsd.exe.manifest
	$(MKDIR) $(@D)
	$(WRC) -i $(SRC_DIR)lsd_manifest.rc -o $(OBJ_DIR_LSD)lsd_manifest.o $(INCLUDE)

# delete object files
clean: cleanlmm cleanlsd

cleanlmm:
	$(RM) $(OBJ_DIR_LMM)* $(LSDROOT)/$(TARGET_LMM)$(EXE_EXT)

cleanlsd:
	$(RM) $(OBJ_DIR_LSD)* $(SRC_DIR)libLSDbase.a $(LSDROOT)/lib$(TARGET_LSD)$(DLL_EXT) $(SRC_DIR)libLSDgui.a $(LSDROOT)/$(TARGET_LSD)$(EXE_EXT)
