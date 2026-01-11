#**************************************************************
#
#	LSD 9.0 - January 2026
#	written by Marco Valente, Universita' dell'Aquila
#	and by Marcelo Pereira, University of Campinas
#
#	Copyright Marco Valente and Marcelo Pereira
#	LSD is distributed under the GNU General Public License
#
#**************************************************************

#**************************************************************
# MAKEFILE
# Makefile for LSD Web Interface executables
# In a command prompt (terminal) in LSD root directory, use
#  cd lwi
#  make
#**************************************************************

# LSD options
TARGET_1=lsd_getsaved
TARGET_2=lsd_getlimits
TARGET_3=lsd_confgen
TARGET_4=lsd_mcstats
LSDROOT=..
SRC=src

# Compiler options
SWITCH_CC= -O3
SWITCH_CC_LNK=

# Libraries options
PATH_HEADER=.
PATH_LIB=.
LIB=-lz

# Compiler options
CC=g++
GLOBAL_CC=-march=corei7-avx -Wall -Wextra -Wno-unused-result -Wno-unused-parameter -Wno-implicit-fallthrough

# specify where are the sources of LSD
SRC_DIR=$(LSDROOT)/$(SRC)/
LIB_DIR=$(SRC_DIR)lib/
CLIB_DIR=$(SRC_DIR)clib/
INCLUDE=-I$(LSDROOT)/$(SRC) -I$(LSDROOT)/$(SRC)/lib -I$(PATH_HEADER)

# terminal specific variables
LIB_TERM=$(LIB)
CC_TERM=$(CC)
SWITCH_CC_TERM=$(GLOBAL_CC) $(SWITCH_CC) -D_TERM_ -D_LWI_

# handle platform-specific stuff
ifeq ($(OS),Windows_NT)
  OBJ_DIR=$(TMP)/lwi/
  ALL_FILES=\*
  EXE_EXT=.exe
  EXE_SUF=
else
  ALL_FILES=*
  EXE_EXT=
  ifeq ($(shell uname -s),Darwin)
    OBJ_DIR=$(TMPDIR)lwi/
    EXE_SUF=-mac
    GLOBAL_CC+=-std=gnu++20 -mmacosx-version-min=11.0 -Wno-unused-but-set-variable -Wno-vla-cxx-extension
  else
    OBJ_DIR=/tmp/lwi/
    EXE_SUF=
  endif
endif

# OS commands
RM=@rm$(EXE_EXT) -f
MKDIR=@mkdir$(EXE_EXT) -p

# targets
.PHONY: all
all: $(TARGET_1)$(EXE_EXT) $(TARGET_2)$(EXE_EXT) $(TARGET_3)$(EXE_EXT) $(TARGET_4)$(EXE_EXT)
getsaved: $(TARGET_1)$(EXE_EXT)
getlimits: $(TARGET_2)$(EXE_EXT)
confgen: $(TARGET_3)$(EXE_EXT)
mcstats: $(TARGET_4)$(EXE_EXT)

# terminal executables (single file)
$(TARGET_1)$(EXE_EXT): $(OBJ_DIR)getsaved.o $(OBJ_DIR)libmain.o $(OBJ_DIR)assimlib.o $(OBJ_DIR)commonlib.o $(OBJ_DIR)description.o $(OBJ_DIR)filelib.o $(OBJ_DIR)latticelib.o $(OBJ_DIR)math.o $(OBJ_DIR)nets.o $(OBJ_DIR)object.o $(OBJ_DIR)parallel.o $(OBJ_DIR)run.o $(OBJ_DIR)stattables.o $(OBJ_DIR)utillib.o $(OBJ_DIR)variab.o $(OBJ_DIR)pugixml.o $(OBJ_DIR)lsd_base.o
	$(CC_TERM) $(SWITCH_CC_TERM) $(SWITCH_CC_LNK) $(OBJ_DIR)getsaved.o $(OBJ_DIR)libmain.o $(OBJ_DIR)assimlib.o $(OBJ_DIR)commonlib.o $(OBJ_DIR)description.o $(OBJ_DIR)filelib.o $(OBJ_DIR)latticelib.o $(OBJ_DIR)math.o $(OBJ_DIR)nets.o $(OBJ_DIR)object.o $(OBJ_DIR)parallel.o $(OBJ_DIR)run.o $(OBJ_DIR)stattables.o $(OBJ_DIR)utillib.o $(OBJ_DIR)variab.o $(OBJ_DIR)pugixml.o $(OBJ_DIR)lsd_base.o -L$(PATH_LIB) $(LIB_TERM) -o $(TARGET_1)$(EXE_SUF)$(EXE_EXT)

$(TARGET_2)$(EXE_EXT): $(OBJ_DIR)getlimits.o $(OBJ_DIR)libmain.o $(OBJ_DIR)assimlib.o $(OBJ_DIR)commonlib.o $(OBJ_DIR)description.o $(OBJ_DIR)filelib.o $(OBJ_DIR)latticelib.o $(OBJ_DIR)math.o $(OBJ_DIR)nets.o $(OBJ_DIR)object.o $(OBJ_DIR)parallel.o $(OBJ_DIR)run.o $(OBJ_DIR)stattables.o $(OBJ_DIR)utillib.o $(OBJ_DIR)variab.o $(OBJ_DIR)pugixml.o $(OBJ_DIR)lsd_base.o
	$(CC_TERM) $(SWITCH_CC_TERM) $(SWITCH_CC_LNK) $(OBJ_DIR)getlimits.o $(OBJ_DIR)libmain.o $(OBJ_DIR)assimlib.o $(OBJ_DIR)commonlib.o $(OBJ_DIR)description.o $(OBJ_DIR)filelib.o $(OBJ_DIR)latticelib.o $(OBJ_DIR)math.o $(OBJ_DIR)nets.o $(OBJ_DIR)object.o $(OBJ_DIR)parallel.o $(OBJ_DIR)run.o $(OBJ_DIR)stattables.o $(OBJ_DIR)utillib.o $(OBJ_DIR)variab.o $(OBJ_DIR)pugixml.o $(OBJ_DIR)lsd_base.o -L$(PATH_LIB) $(LIB_TERM) -o $(TARGET_2)$(EXE_SUF)$(EXE_EXT)

$(TARGET_3)$(EXE_EXT): $(OBJ_DIR)confgen.o $(OBJ_DIR)libmain.o $(OBJ_DIR)assimlib.o $(OBJ_DIR)commonlib.o $(OBJ_DIR)description.o $(OBJ_DIR)filelib.o $(OBJ_DIR)latticelib.o $(OBJ_DIR)math.o $(OBJ_DIR)nets.o $(OBJ_DIR)object.o $(OBJ_DIR)parallel.o $(OBJ_DIR)run.o $(OBJ_DIR)stattables.o $(OBJ_DIR)utillib.o $(OBJ_DIR)variab.o $(OBJ_DIR)pugixml.o $(OBJ_DIR)lsd_base.o
	$(CC_TERM) $(SWITCH_CC_TERM) $(SWITCH_CC_LNK) $(OBJ_DIR)confgen.o $(OBJ_DIR)libmain.o $(OBJ_DIR)assimlib.o $(OBJ_DIR)commonlib.o $(OBJ_DIR)description.o $(OBJ_DIR)filelib.o $(OBJ_DIR)latticelib.o $(OBJ_DIR)math.o $(OBJ_DIR)nets.o $(OBJ_DIR)object.o $(OBJ_DIR)parallel.o $(OBJ_DIR)run.o $(OBJ_DIR)stattables.o $(OBJ_DIR)utillib.o $(OBJ_DIR)variab.o $(OBJ_DIR)pugixml.o $(OBJ_DIR)lsd_base.o -L$(PATH_LIB) $(LIB_TERM) -o $(TARGET_3)$(EXE_SUF)$(EXE_EXT)

$(TARGET_4)$(EXE_EXT): $(OBJ_DIR)mcstats.o $(OBJ_DIR)libmain.o $(OBJ_DIR)assimlib.o $(OBJ_DIR)commonlib.o $(OBJ_DIR)description.o $(OBJ_DIR)filelib.o $(OBJ_DIR)latticelib.o $(OBJ_DIR)math.o $(OBJ_DIR)nets.o $(OBJ_DIR)object.o $(OBJ_DIR)parallel.o $(OBJ_DIR)run.o $(OBJ_DIR)stattables.o $(OBJ_DIR)utillib.o $(OBJ_DIR)variab.o $(OBJ_DIR)pugixml.o $(OBJ_DIR)lsd_base.o
	$(CC_TERM) $(SWITCH_CC_TERM) $(SWITCH_CC_LNK) $(OBJ_DIR)mcstats.o $(OBJ_DIR)libmain.o $(OBJ_DIR)assimlib.o $(OBJ_DIR)commonlib.o $(OBJ_DIR)description.o $(OBJ_DIR)filelib.o $(OBJ_DIR)latticelib.o $(OBJ_DIR)math.o $(OBJ_DIR)nets.o $(OBJ_DIR)object.o $(OBJ_DIR)parallel.o $(OBJ_DIR)run.o $(OBJ_DIR)stattables.o $(OBJ_DIR)utillib.o $(OBJ_DIR)variab.o $(OBJ_DIR)pugixml.o $(OBJ_DIR)lsd_base.o -L$(PATH_LIB) $(LIB_TERM) -o $(TARGET_4)$(EXE_SUF)$(EXE_EXT)

# executable/DLL modules
$(OBJ_DIR)getsaved.o: $(SRC_DIR)getsaved.cpp $(LIB_DIR)libLSD.h
	$(MKDIR) $(@D)
	$(CC_TERM) $(SWITCH_CC_TERM) $(INCLUDE) -c $(SRC_DIR)getsaved.cpp -o $(OBJ_DIR)getsaved.o

$(OBJ_DIR)getlimits.o: $(SRC_DIR)getlimits.cpp $(LIB_DIR)libLSD.h
	$(MKDIR) $(@D)
	$(CC_TERM) $(SWITCH_CC_TERM) $(INCLUDE) -c $(SRC_DIR)getlimits.cpp -o $(OBJ_DIR)getlimits.o

$(OBJ_DIR)confgen.o: $(SRC_DIR)confgen.cpp $(LIB_DIR)libLSD.h
	$(MKDIR) $(@D)
	$(CC_TERM) $(SWITCH_CC_TERM) $(INCLUDE) -c $(SRC_DIR)confgen.cpp -o $(OBJ_DIR)confgen.o

$(OBJ_DIR)mcstats.o: $(SRC_DIR)mcstats.cpp $(LIB_DIR)libLSD.h
	$(MKDIR) $(@D)
	$(CC_TERM) $(SWITCH_CC_TERM) $(INCLUDE) -c $(SRC_DIR)mcstats.cpp -o $(OBJ_DIR)mcstats.o

$(OBJ_DIR)lsd_base.o: $(SRC_DIR)lsd_base.cpp $(LIB_DIR)libLSD.h
	$(MKDIR) $(@D)
	$(CC_TERM) $(SWITCH_CC_TERM) $(INCLUDE) -c $(SRC_DIR)lsd_base.cpp -o $(OBJ_DIR)lsd_base.o

$(OBJ_DIR)libmain.o: $(LIB_DIR)libmain.cpp $(LIB_DIR)libLSD.h
	$(MKDIR) $(@D)
	$(CC_TERM) $(SWITCH_CC_TERM) $(INCLUDE) -c $(LIB_DIR)libmain.cpp -o $(OBJ_DIR)libmain.o

$(OBJ_DIR)assimlib.o: $(LIB_DIR)assimlib.cpp $(LIB_DIR)libLSD.h
	$(MKDIR) $(@D)
	$(CC_TERM) $(SWITCH_CC_TERM) $(INCLUDE) -c $(LIB_DIR)assimlib.cpp -o $(OBJ_DIR)assimlib.o

$(OBJ_DIR)commonlib.o: $(LIB_DIR)commonlib.cpp $(LIB_DIR)libLSD.h
	$(MKDIR) $(@D)
	$(CC_TERM) $(SWITCH_CC_TERM) $(INCLUDE) -c $(LIB_DIR)commonlib.cpp -o $(OBJ_DIR)commonlib.o

$(OBJ_DIR)description.o: $(LIB_DIR)description.cpp $(LIB_DIR)libLSD.h
	$(MKDIR) $(@D)
	$(CC_TERM) $(SWITCH_CC_TERM) $(INCLUDE) -c $(LIB_DIR)description.cpp -o $(OBJ_DIR)description.o

$(OBJ_DIR)filelib.o: $(LIB_DIR)filelib.cpp $(LIB_DIR)libLSD.h
	$(MKDIR) $(@D)
	$(CC_TERM) $(SWITCH_CC_TERM) $(INCLUDE) -c $(LIB_DIR)filelib.cpp -o $(OBJ_DIR)filelib.o

$(OBJ_DIR)latticelib.o: $(LIB_DIR)latticelib.cpp $(LIB_DIR)libLSD.h
	$(MKDIR) $(@D)
	$(CC_TERM) $(SWITCH_CC_TERM) $(INCLUDE) -c $(LIB_DIR)latticelib.cpp -o $(OBJ_DIR)latticelib.o

$(OBJ_DIR)math.o: $(LIB_DIR)math.cpp $(LIB_DIR)libLSD.h
	$(MKDIR) $(@D)
	$(CC_TERM) $(SWITCH_CC_TERM) $(INCLUDE) -c $(LIB_DIR)math.cpp -o $(OBJ_DIR)math.o

$(OBJ_DIR)nets.o: $(LIB_DIR)nets.cpp $(LIB_DIR)libLSD.h
	$(MKDIR) $(@D)
	$(CC_TERM) $(SWITCH_CC_TERM) $(INCLUDE) -c $(LIB_DIR)nets.cpp -o $(OBJ_DIR)nets.o

$(OBJ_DIR)object.o: $(LIB_DIR)object.cpp $(LIB_DIR)libLSD.h
	$(MKDIR) $(@D)
	$(CC_TERM) $(SWITCH_CC_TERM) $(INCLUDE) -c $(LIB_DIR)object.cpp -o $(OBJ_DIR)object.o

$(OBJ_DIR)parallel.o: $(LIB_DIR)parallel.cpp $(LIB_DIR)libLSD.h
	$(MKDIR) $(@D)
	$(CC_TERM) $(SWITCH_CC_TERM) $(INCLUDE) -c $(LIB_DIR)parallel.cpp -o $(OBJ_DIR)parallel.o

$(OBJ_DIR)run.o: $(LIB_DIR)run.cpp $(LIB_DIR)libLSD.h
	$(MKDIR) $(@D)
	$(CC_TERM) $(SWITCH_CC_TERM) $(INCLUDE) -c $(LIB_DIR)run.cpp -o $(OBJ_DIR)run.o

$(OBJ_DIR)stattables.o: $(LIB_DIR)stattables.cpp $(LIB_DIR)libLSD.h
	$(MKDIR) $(@D)
	$(CC_TERM) $(SWITCH_CC_TERM) $(INCLUDE) -c $(LIB_DIR)stattables.cpp -o $(OBJ_DIR)stattables.o

$(OBJ_DIR)utillib.o: $(LIB_DIR)utillib.cpp $(LIB_DIR)libLSD.h
	$(MKDIR) $(@D)
	$(CC_TERM) $(SWITCH_CC_TERM) $(INCLUDE) -c $(LIB_DIR)utillib.cpp -o $(OBJ_DIR)utillib.o

$(OBJ_DIR)variab.o: $(LIB_DIR)variab.cpp $(LIB_DIR)libLSD.h
	$(MKDIR) $(@D)
	$(CC_TERM) $(SWITCH_CC_TERM) $(INCLUDE) -c $(LIB_DIR)variab.cpp -o $(OBJ_DIR)variab.o

$(OBJ_DIR)pugixml.o: $(CLIB_DIR)pugixml.cpp $(CLIB_DIR)pugixml.hpp $(CLIB_DIR)pugiconfig.hpp
	$(MKDIR) $(@D)
	$(CC_TERM) $(SWITCH_CC_TERM) $(INCLUDE) -c $(CLIB_DIR)pugixml.cpp -o $(OBJ_DIR)pugixml.o

# remove compiled files
clean:
	$(RM) $(OBJ_DIR)$(ALL_FILES) $(TARGET_1)$(EXE_SUF)$(EXE_EXT) $(TARGET_2)$(EXE_SUF)$(EXE_EXT) $(TARGET_3)$(EXE_SUF)$(EXE_EXT) $(TARGET_4)$(EXE_SUF)$(EXE_EXT)
