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
CLIB_DIR=$(SRC_DIR)clib/

# handle platform-specific stuff
ifeq ($(OS),Windows_NT)
  PLAT=windows
  TMP_DIR=$(TMP)
  EXE_EXT=.exe
  DLL_EXT=.dll
  WRC=windres -D_WIN32
  AR_DIR=$(LSDROOT)/gnu/x86_64-w64-mingw32/bin/
  MANIF_LMM=$(TMP_DIR)/$(TARGET_LMM)/LMM.exe.manifest.o
  MANIF_LSD=$(TMP_DIR)/$(TARGET_LSD)/LSD.exe.manifest.o
  TCL_VERSION=86
  PATH_TCLTK_HEADER=$(LSDROOT)/gnu/include
  PATH_TCLTK_LIB=$(LSDROOT)/gnu/lib
  TCLTK_LIB=-ltcl$(TCL_VERSION) -ltk$(TCL_VERSION)
  SWITCH_CC+=-mthreads -mwindows
  SWITCH_CC_LMM+=-static-libstdc++
else
  ifeq ($(shell uname -s),Darwin)
    PLAT=mac
    TMP_DIR=$(TMPDIR)
    PKG_LMM=$(TARGET_LMM).app/Contents/MacOS/
    PKG_LSD=$(TARGET_LSD).app/Contents/MacOS/
    EXE_EXT=
    DLL_EXT=.dylib
    TOUCH_LMM=touch -c $(LSDROOT)/$(TARGET_LMM).app
    TOUCH_LSD=touch -c $(LSDROOT)/$(TARGET_LSD).app
    PATH_TCL_HEADER=$(SRC_DIR)LSD.app/Contents/Frameworks/Tcl.framework/Headers
    PATH_TK_HEADER=$(SRC_DIR)LSD.app/Contents/Frameworks/Tk.framework/Headers
    PATH_TCLTK_LIB=$(SRC_DIR)LSD.app/Contents/Frameworks
    TCLTK_LIB=-framework Tcl -framework Tk
    SWITCH_CC+=-Wno-unused-result -std=c++14 -mmacosx-version-min=10.10
  else
    PLAT=linux
    TMP_DIR=$(TMPDIR)
    EXE_EXT=
    DLL_EXT=.so
    TCL_VERSION=8.6
    PATH_TCLTK_HEADER=/usr/include/tcl$(TCL_VERSION)
    PATH_TCLTK_LIB=/usr/lib/tcl$(TCL_VERSION)
    TCLTK_LIB=-ltcl$(TCL_VERSION) -ltk$(TCL_VERSION)
    SWITCH_CC+=-Wno-unused-result
  endif
endif

# additional OS-specific locations
INCLUDE=-I$(SRC). -I$(PATH_TCLTK_HEADER)
LIB_GUI=$(SRC_DIR)LSDgui-$(PLAT).a
LIB_BASE=$(SRC_DIR)LSDbase-$(PLAT).a
OBJ_DIR_LMM=$(TMP_DIR)/$(TARGET_LMM)/
OBJ_DIR_LSD=$(TMP_DIR)/$(TARGET_LSD)/
PATH_LIB=.
LIB=-lz

# set general dependencies
DEP_GUI=$(SRC_DIR)LSD.h
DEP_LIB=$(LIB_DIR)libLSD.h

# OS commands
CC=g++$(EXE_EXT)
AR=$(AR_DIR)ar$(EXE_EXT) -rcs
RM=@rm$(EXE_EXT) -rf
MKDIR=@mkdir$(EXE_EXT) -p

# targets
.PHONY: all LMM LSD libdll libgui libbase clean cleanLMM cleanLSD
all: LMM LSD
LMM: $(LSDROOT)/$(TARGET_LMM)$(EXE_EXT)
LSD: $(LSDROOT)/$(TARGET_LSD)$(EXE_EXT)
libdll: $(LSDROOT)/lib$(TARGET_LSD)$(DLL_EXT)
libgui: $(LIB_GUI)
libbase: $(LIB_BASE)

# LMM executable
$(LSDROOT)/$(PKG_LMM)$(TARGET_LMM)$(EXE_EXT): $(OBJ_DIR_LMM)LMM.o $(OBJ_DIR_LMM)common.o $(OBJ_DIR_LMM)commonlib.o $(MANIF_LMM)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LMM) $(OBJ_DIR_LMM)LMM.o $(OBJ_DIR_LMM)common.o $(OBJ_DIR_LMM)commonlib.o $(MANIF_LMM) -L$(PATH_TCLTK_LIB) $(TCLTK_LIB) -L$(PATH_LIB) $(LIB) -o $(LSDROOT)/$(PKG_LMM)$(TARGET_LMM)$(EXE_EXT)
	$(TOUCH_LMM)

# LMM executable modules
$(OBJ_DIR_LMM)LMM.o: $(SRC_DIR)LMM.cpp $(DEP_GUI)
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LMM) $(INCLUDE) -c $(SRC_DIR)LMM.cpp -o $(OBJ_DIR_LMM)LMM.o

$(OBJ_DIR_LMM)common.o: $(SRC_DIR)common.cpp $(DEP_GUI)
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LMM) $(INCLUDE) -c $(SRC_DIR)common.cpp -o $(OBJ_DIR_LMM)common.o

$(OBJ_DIR_LMM)commonlib.o: $(LIB_DIR)commonlib.cpp $(DEP_GUI)
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LMM) $(INCLUDE) -c $(LIB_DIR)commonlib.cpp -o $(OBJ_DIR_LMM)commonlib.o

$(MANIF_LMM): $(SRC_DIR)LMM.exe.manifest.rc $(SRC_DIR)LMM.exe.manifest
	$(MKDIR) $(@D)
	$(WRC) $(INCLUDE) -i $(SRC_DIR)LMM.exe.manifest.rc -o $(MANIF_LMM)

# LSD dynamic library (removed after executable is built)
$(LSDROOT)/$(PKG_LSD)lib$(TARGET_LSD)$(DLL_EXT): $(OBJ_DIR_LSD)fun_base.o $(LIB_BASE)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(OBJ_DIR_LSD)fun_base.o -Wl,--whole-archive $(LIB_BASE) -Wl,--no-whole-archive -L$(PATH_LIB) $(LIB) -shared -o $(LSDROOT)/$(PKG_LSD)lib$(TARGET_LSD)$(DLL_EXT)
	$(TOUCH_LSD)

$(OBJ_DIR_LSD)fun_base.o: $(SRC_DIR)fun_base.cpp $(DEP_LIB) $(LIB_DIR)check.h $(SRC_DIR)fun_head.h
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(INCLUDE) -c $(SRC_DIR)fun_base.cpp -o $(OBJ_DIR_LSD)fun_base.o

# base static library for LSD dynamic library
$(LIB_BASE): $(OBJ_DIR_LSD)libmain.o $(OBJ_DIR_LSD)commonlib.o $(OBJ_DIR_LSD)description.o $(OBJ_DIR_LSD)filelib.o $(OBJ_DIR_LSD)latticelib.o $(OBJ_DIR_LSD)math.o $(OBJ_DIR_LSD)nets.o $(OBJ_DIR_LSD)object.o $(OBJ_DIR_LSD)parallel.o $(OBJ_DIR_LSD)run.o $(OBJ_DIR_LSD)stattables.o $(OBJ_DIR_LSD)utillib.o $(OBJ_DIR_LSD)variab.o $(OBJ_DIR_LSD)pugixml.o
	$(AR) $(LIB_BASE) $(OBJ_DIR_LSD)libmain.o $(OBJ_DIR_LSD)commonlib.o $(OBJ_DIR_LSD)description.o $(OBJ_DIR_LSD)filelib.o $(OBJ_DIR_LSD)latticelib.o $(OBJ_DIR_LSD)math.o $(OBJ_DIR_LSD)nets.o $(OBJ_DIR_LSD)object.o $(OBJ_DIR_LSD)parallel.o $(OBJ_DIR_LSD)run.o $(OBJ_DIR_LSD)stattables.o $(OBJ_DIR_LSD)utillib.o $(OBJ_DIR_LSD)variab.o $(OBJ_DIR_LSD)pugixml.o

# LSD base static library modules
$(OBJ_DIR_LSD)libmain.o: $(LIB_DIR)libmain.cpp $(DEP_LIB)
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(LIB_DIR)libmain.cpp -o $(OBJ_DIR_LSD)libmain.o

$(OBJ_DIR_LSD)commonlib.o: $(LIB_DIR)commonlib.cpp $(DEP_LIB)
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(LIB_DIR)commonlib.cpp -o $(OBJ_DIR_LSD)commonlib.o

$(OBJ_DIR_LSD)description.o: $(LIB_DIR)description.cpp $(DEP_LIB)
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(LIB_DIR)description.cpp -o $(OBJ_DIR_LSD)description.o

$(OBJ_DIR_LSD)filelib.o: $(LIB_DIR)filelib.cpp $(DEP_LIB)
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(LIB_DIR)filelib.cpp -o $(OBJ_DIR_LSD)filelib.o

$(OBJ_DIR_LSD)latticelib.o: $(LIB_DIR)latticelib.cpp $(DEP_LIB)
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(LIB_DIR)latticelib.cpp -o $(OBJ_DIR_LSD)latticelib.o

$(OBJ_DIR_LSD)math.o: $(LIB_DIR)math.cpp $(DEP_LIB)
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(LIB_DIR)math.cpp -o $(OBJ_DIR_LSD)math.o

$(OBJ_DIR_LSD)nets.o: $(LIB_DIR)nets.cpp $(DEP_LIB)
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(LIB_DIR)nets.cpp -o $(OBJ_DIR_LSD)nets.o

$(OBJ_DIR_LSD)object.o: $(LIB_DIR)object.cpp $(DEP_LIB)
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(LIB_DIR)object.cpp -o $(OBJ_DIR_LSD)object.o

$(OBJ_DIR_LSD)parallel.o: $(LIB_DIR)parallel.cpp $(DEP_LIB)
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(LIB_DIR)parallel.cpp -o $(OBJ_DIR_LSD)parallel.o

$(OBJ_DIR_LSD)run.o: $(LIB_DIR)run.cpp $(DEP_LIB)
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(LIB_DIR)run.cpp -o $(OBJ_DIR_LSD)run.o

$(OBJ_DIR_LSD)stattables.o: $(LIB_DIR)stattables.cpp $(DEP_LIB)
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(LIB_DIR)stattables.cpp -o $(OBJ_DIR_LSD)stattables.o

$(OBJ_DIR_LSD)utillib.o: $(LIB_DIR)utillib.cpp $(DEP_LIB)
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(LIB_DIR)utillib.cpp -o $(OBJ_DIR_LSD)utillib.o

$(OBJ_DIR_LSD)variab.o: $(LIB_DIR)variab.cpp $(DEP_LIB)
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(LIB_DIR)variab.cpp -o $(OBJ_DIR_LSD)variab.o

$(OBJ_DIR_LSD)pugixml.o: $(CLIB_DIR)pugixml.cpp $(CLIB_DIR)pugixml.hpp $(CLIB_DIR)pugiconfig.hpp
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(CLIB_DIR)pugixml.cpp -o $(OBJ_DIR_LSD)pugixml.o

# LSD executable
$(LSDROOT)/$(PKG_LSD)$(TARGET_LSD)$(EXE_EXT): $(MANIF_LSD) $(LIB_GUI) $(LSDROOT)/lib$(TARGET_LSD)$(DLL_EXT)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(MANIF_LSD) $(LIB_GUI) -L$(LSDROOT) -l$(TARGET_LSD) -L$(PATH_TCLTK_LIB) $(TCLTK_LIB) -L$(PATH_LIB) $(LIB) -o $(LSDROOT)/$(PKG_LSD)$(TARGET_LSD)$(EXE_EXT)
	$(RM) $(LSDROOT)/lib$(TARGET_LSD)$(DLL_EXT)
	$(TOUCH_LSD)

# LSD executable static library
$(LIB_GUI): $(OBJ_DIR_LSD)LSD.o $(OBJ_DIR_LSD)LSDmain.o $(OBJ_DIR_LSD)analysis.o $(OBJ_DIR_LSD)browser.o $(OBJ_DIR_LSD)common.o $(OBJ_DIR_LSD)debug.o $(OBJ_DIR_LSD)draw.o $(OBJ_DIR_LSD)edit.o $(OBJ_DIR_LSD)edit_dat.o $(OBJ_DIR_LSD)file.o $(OBJ_DIR_LSD)interf.o $(OBJ_DIR_LSD)lattice.o $(OBJ_DIR_LSD)log.o $(OBJ_DIR_LSD)report.o $(OBJ_DIR_LSD)runtime.o $(OBJ_DIR_LSD)sensitivity.o $(OBJ_DIR_LSD)set_all.o $(OBJ_DIR_LSD)show_eq.o $(OBJ_DIR_LSD)util.o
	$(AR) $(LIB_GUI) $(OBJ_DIR_LSD)LSD.o $(OBJ_DIR_LSD)LSDmain.o $(OBJ_DIR_LSD)analysis.o $(OBJ_DIR_LSD)browser.o $(OBJ_DIR_LSD)common.o $(OBJ_DIR_LSD)debug.o $(OBJ_DIR_LSD)draw.o $(OBJ_DIR_LSD)edit.o $(OBJ_DIR_LSD)edit_dat.o $(OBJ_DIR_LSD)file.o $(OBJ_DIR_LSD)interf.o $(OBJ_DIR_LSD)lattice.o $(OBJ_DIR_LSD)log.o $(OBJ_DIR_LSD)report.o $(OBJ_DIR_LSD)runtime.o $(OBJ_DIR_LSD)sensitivity.o $(OBJ_DIR_LSD)set_all.o $(OBJ_DIR_LSD)show_eq.o $(OBJ_DIR_LSD)util.o

# LSD executable static library modules
$(OBJ_DIR_LSD)LSD.o: $(SRC_DIR)LSD.cpp $(DEP_GUI)
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(SRC_DIR)LSD.cpp -o $(OBJ_DIR_LSD)LSD.o

$(OBJ_DIR_LSD)LSDmain.o: $(SRC_DIR)LSDmain.cpp $(DEP_GUI)
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(SRC_DIR)LSDmain.cpp -o $(OBJ_DIR_LSD)LSDmain.o

$(OBJ_DIR_LSD)analysis.o: $(SRC_DIR)analysis.cpp $(DEP_GUI)
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(SRC_DIR)analysis.cpp -o $(OBJ_DIR_LSD)analysis.o

$(OBJ_DIR_LSD)browser.o: $(SRC_DIR)browser.cpp $(DEP_GUI)
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(SRC_DIR)browser.cpp -o $(OBJ_DIR_LSD)browser.o

$(OBJ_DIR_LSD)common.o: $(SRC_DIR)common.cpp $(DEP_GUI)
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(SRC_DIR)common.cpp -o $(OBJ_DIR_LSD)common.o

$(OBJ_DIR_LSD)debug.o: $(SRC_DIR)debug.cpp $(DEP_GUI)
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(SRC_DIR)debug.cpp -o $(OBJ_DIR_LSD)debug.o

$(OBJ_DIR_LSD)draw.o: $(SRC_DIR)draw.cpp $(DEP_GUI)
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(SRC_DIR)draw.cpp -o $(OBJ_DIR_LSD)draw.o

$(OBJ_DIR_LSD)edit.o: $(SRC_DIR)edit.cpp $(DEP_GUI)
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(SRC_DIR)edit.cpp -o $(OBJ_DIR_LSD)edit.o

$(OBJ_DIR_LSD)edit_dat.o: $(SRC_DIR)edit_dat.cpp $(DEP_GUI)
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(SRC_DIR)edit_dat.cpp -o $(OBJ_DIR_LSD)edit_dat.o

$(OBJ_DIR_LSD)file.o: $(SRC_DIR)file.cpp $(DEP_GUI)
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(SRC_DIR)file.cpp -o $(OBJ_DIR_LSD)file.o

$(OBJ_DIR_LSD)interf.o: $(SRC_DIR)interf.cpp $(DEP_GUI)
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(SRC_DIR)interf.cpp -o $(OBJ_DIR_LSD)interf.o

$(OBJ_DIR_LSD)lattice.o: $(SRC_DIR)lattice.cpp $(DEP_GUI)
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(SRC_DIR)lattice.cpp -o $(OBJ_DIR_LSD)lattice.o

$(OBJ_DIR_LSD)log.o: $(SRC_DIR)log.cpp $(DEP_GUI)
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(SRC_DIR)log.cpp -o $(OBJ_DIR_LSD)log.o

$(OBJ_DIR_LSD)report.o: $(SRC_DIR)report.cpp $(DEP_GUI)
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(SRC_DIR)report.cpp -o $(OBJ_DIR_LSD)report.o

$(OBJ_DIR_LSD)runtime.o: $(SRC_DIR)runtime.cpp $(DEP_GUI)
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(SRC_DIR)runtime.cpp -o $(OBJ_DIR_LSD)runtime.o

$(OBJ_DIR_LSD)sensitivity.o: $(SRC_DIR)sensitivity.cpp $(SRC_DIR)nolhtables.h $(DEP_GUI)
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(SRC_DIR)sensitivity.cpp -o $(OBJ_DIR_LSD)sensitivity.o

$(OBJ_DIR_LSD)set_all.o: $(SRC_DIR)set_all.cpp $(DEP_GUI)
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(SRC_DIR)set_all.cpp -o $(OBJ_DIR_LSD)set_all.o

$(OBJ_DIR_LSD)show_eq.o: $(SRC_DIR)show_eq.cpp $(DEP_GUI)
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(SRC_DIR)show_eq.cpp -o $(OBJ_DIR_LSD)show_eq.o

$(OBJ_DIR_LSD)util.o: $(SRC_DIR)util.cpp $(DEP_GUI)
	$(MKDIR) $(@D)
	$(CC) $(SWITCH_CC) $(SWITCH_CC_LSD) $(INCLUDE) -c $(SRC_DIR)util.cpp -o $(OBJ_DIR_LSD)util.o

$(MANIF_LSD): $(SRC_DIR)LSD.exe.manifest.rc $(SRC_DIR)LSD.exe.manifest
	$(MKDIR) $(@D)
	$(WRC) -i $(SRC_DIR)LSD.exe.manifest.rc -o $(MANIF_LSD) $(INCLUDE)

# delete object files
clean: cleanLMM cleanLSD

cleanLMM:
	$(RM) $(OBJ_DIR_LMM)* $(LSDROOT)/$(TARGET_LMM)$(EXE_EXT)

cleanLSD:
	$(RM) $(OBJ_DIR_LSD)* $(LIB_BASE) $(LIB_GUI) $(LSDROOT)/$(TARGET_LSD)$(EXE_EXT)
