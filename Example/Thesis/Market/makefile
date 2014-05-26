# Model compilation options
TARGET=lsd_gnu
FUN=fun_market
SWITCH_CC=-g



# System compilation option
LSDROOT=C:/Lsd61
TCL_VERSION=85
TK_VERSION=85
DUMMY=-mwindows
PATH_TCL_LIB=$(LSDROOT)/gnu/lib
PATH_TK_LIB=$(LSDROOT)/gnu/lib
PATH_TK_HEADER=$(LSDROOT)/gnu/include
PATH_TCL_HEADER=$(LSDROOT)/gnu/include
PATH_LIB=$(LSDROOT)/gnu/lib
INCLUDE_LIB=-I$(LSDROOT)/gnu/include
CC=g++
SRC=src
EXTRA_PAR=-lz
SSWITCH_CC=-g
EXT=.exe

# Set the complete option for the libraries to link to the models.
TCL_LIB=-L$(PATH_TCL_LIB) -ltcl$(TCL_VERSION)
TK_LIB=-L$(PATH_TK_LIB) -ltk$(TK_VERSION)
LSDROOT=C:/Lsd61

# body of the makefile (in src/makefile_base.txt)
# Specify where are the sources of the system. Do not modify unless using a different
# directory structure (why should you?).
SRC_DIR=$(LSDROOT)/$(SRC)/

# Variable used to prepare the options to search for the tcl and tk headers
# Do not modify
INCLD_TCL_TK=-I$(PATH_TCL_HEADER) -I$(PATH_TK_HEADER) 


$(TARGET)$(EXT): $(SRC_DIR)choose.h model_options.txt $(SRC_DIR)system_options.txt $(SRC_DIR)fun_head.h $(SRC_DIR)main_gnuwin.o \
  $(SRC_DIR)file.o $(SRC_DIR)util.o $(SRC_DIR)object.o \
	$(SRC_DIR)variab.o $(SRC_DIR)interf.o $(SRC_DIR)draw.o $(SRC_DIR)runtime.o\
        $(SRC_DIR)debug.o $(SRC_DIR)edit.o $(SRC_DIR)edit_dat.o $(SRC_DIR)set_all.o \
        $(FUN).o $(SRC_DIR)analysis.o $(SRC_DIR)show_eq.o $(SRC_DIR)lsdmain.o $(SRC_DIR)report.o
	$(CC) $(SWITCH_CC) -L$(PATH_LIB) $(SRC_DIR)file.o $(SRC_DIR)main_gnuwin.o $(SRC_DIR)util.o \
	$(SRC_DIR)draw.o \
	$(SRC_DIR)object.o $(SRC_DIR)set_all.o $(SRC_DIR)variab.o $(SRC_DIR)interf.o \
	$(SRC_DIR)show_eq.o $(SRC_DIR)runtime.o \
	$(SRC_DIR)debug.o $(SRC_DIR)edit.o $(SRC_DIR)edit_dat.o $(FUN).o \
	$(SRC_DIR)analysis.o $(SRC_DIR)lsdmain.o $(SRC_DIR)report.o \
	$(TCL_LIB) $(TK_LIB) $(EXTRA_PAR) -L$(PATH_LIB) -lm -o $(TARGET) $(DUMMY)

$(SRC_DIR)file.o: $(SRC_DIR)choose.h $(SRC_DIR)system_options.txt $(SRC_DIR)file.cpp $(SRC_DIR)decl.h
	$(CC) $(SSWITCH_CC) -c $(SRC_DIR)file.cpp -o $(SRC_DIR)file.o $(INCLD_TCL_TK) $(INCLUDE_LIB)

$(SRC_DIR)interf.o: $(SRC_DIR)choose.h $(SRC_DIR)system_options.txt $(SRC_DIR)interf.cpp $(SRC_DIR)decl.h
	$(CC) $(SSWITCH_CC) -c $(SRC_DIR)interf.cpp  -o $(SRC_DIR)interf.o $(INCLD_TCL_TK) $(INCLUDE_LIB)

$(SRC_DIR)main_gnuwin.o: $(SRC_DIR)choose.h $(SRC_DIR)system_options.txt $(SRC_DIR)main_gnuwin.cpp $(SRC_DIR)decl.h
	$(CC) $(SSWITCH_CC) -c $(SRC_DIR)main_gnuwin.cpp -o $(SRC_DIR)main_gnuwin.o $(INCLD_TCL_TK) $(INCLUDE_LIB)

$(SRC_DIR)util.o: $(SRC_DIR)choose.h $(SRC_DIR)system_options.txt $(SRC_DIR)util.cpp $(SRC_DIR)decl.h
	$(CC) $(SSWITCH_CC) -c $(SRC_DIR)util.cpp -o $(SRC_DIR)util.o $(INCLD_TCL_TK) $(INCLUDE_LIB)

$(SRC_DIR)variab.o: $(SRC_DIR)choose.h $(SRC_DIR)system_options.txt $(SRC_DIR)variab.cpp $(SRC_DIR)decl.h
	$(CC) $(SSWITCH_CC) -c $(SRC_DIR)variab.cpp -o $(SRC_DIR)variab.o $(INCLD_TCL_TK) $(INCLUDE_LIB)

$(SRC_DIR)object.o: $(SRC_DIR)choose.h $(SRC_DIR)system_options.txt $(SRC_DIR)object.cpp $(SRC_DIR)decl.h
	$(CC) $(SSWITCH_CC) -c $(SRC_DIR)object.cpp -o $(SRC_DIR)object.o $(INCLD_TCL_TK) $(INCLUDE_LIB)

$(SRC_DIR)debug.o: $(SRC_DIR)choose.h $(SRC_DIR)system_options.txt $(SRC_DIR)debug.cpp $(SRC_DIR)decl.h
	$(CC) $(SSWITCH_CC) -c $(SRC_DIR)debug.cpp -o $(SRC_DIR)debug.o $(INCLD_TCL_TK) $(INCLUDE_LIB)

$(SRC_DIR)edit.o: $(SRC_DIR)choose.h $(SRC_DIR)system_options.txt $(SRC_DIR)edit.cpp $(SRC_DIR)decl.h
	$(CC) $(SSWITCH_CC) -c $(SRC_DIR)edit.cpp -o $(SRC_DIR)edit.o $(INCLD_TCL_TK) $(INCLUDE_LIB)

$(SRC_DIR)edit_dat.o: $(SRC_DIR)choose.h $(SRC_DIR)system_options.txt  $(SRC_DIR)edit_dat.cpp $(SRC_DIR)decl.h
	$(CC) $(SSWITCH_CC) -c $(SRC_DIR)edit_dat.cpp -o $(SRC_DIR)edit_dat.o $(INCLD_TCL_TK) $(INCLUDE_LIB)

$(SRC_DIR)set_all.o: $(SRC_DIR)choose.h $(SRC_DIR)system_options.txt $(SRC_DIR)set_all.cpp $(SRC_DIR)decl.h
	$(CC) $(SSWITCH_CC) -c $(SRC_DIR)set_all.cpp -o $(SRC_DIR)set_all.o $(INCLD_TCL_TK) $(INCLUDE_LIB)

$(SRC_DIR)draw.o: $(SRC_DIR)choose.h $(SRC_DIR)system_options.txt $(SRC_DIR)draw.cpp $(SRC_DIR)decl.h
	$(CC) $(SSWITCH_CC) -c $(SRC_DIR)draw.cpp -o $(SRC_DIR)draw.o $(INCLD_TCL_TK) $(INCLUDE_LIB)

$(SRC_DIR)analysis.o: $(SRC_DIR)choose.h $(SRC_DIR)system_options.txt $(SRC_DIR)analysis.cpp $(SRC_DIR)decl.h
	$(CC) $(SSWITCH_CC) -c $(SRC_DIR)analysis.cpp -o $(SRC_DIR)analysis.o $(INCLD_TCL_TK) $(INCLUDE_LIB)

$(SRC_DIR)show_eq.o: $(SRC_DIR)choose.h $(SRC_DIR)system_options.txt $(SRC_DIR)show_eq.cpp $(SRC_DIR)decl.h
	$(CC) $(SSWITCH_CC) -c $(SRC_DIR)show_eq.cpp -o $(SRC_DIR)show_eq.o $(INCLD_TCL_TK) $(INCLUDE_LIB)

$(SRC_DIR)runtime.o: $(SRC_DIR)choose.h $(SRC_DIR)system_options.txt $(SRC_DIR)runtime.cpp $(SRC_DIR)decl.h
	$(CC) $(SSWITCH_CC) -c $(SRC_DIR)runtime.cpp -o $(SRC_DIR)runtime.o $(INCLD_TCL_TK) $(INCLUDE_LIB)

$(SRC_DIR)lsdmain.o: $(SRC_DIR)choose.h $(SRC_DIR)system_options.txt $(SRC_DIR)lsdmain.cpp $(SRC_DIR)decl.h
	$(CC) $(SSWITCH_CC) -c $(SRC_DIR)lsdmain.cpp -o $(SRC_DIR)lsdmain.o $(INCLD_TCL_TK) $(INCLUDE_LIB)

$(SRC_DIR)report.o: $(SRC_DIR)choose.h $(SRC_DIR)system_options.txt $(SRC_DIR)report.cpp $(SRC_DIR)decl.h
	$(CC) $(SSWITCH_CC) -c $(SRC_DIR)report.cpp -o $(SRC_DIR)report.o $(INCLD_TCL_TK) $(INCLUDE_LIB)


$(FUN).o: $(SRC_DIR)choose.h model_options.txt $(SRC_DIR)system_options.txt $(FUN).cpp $(SRC_DIR)decl.h $(SRC_DIR)fun_head.h
	$(CC) $(SWITCH_CC) -c $(FUN).cpp $(INCLD_TCL_TK) $(INCLUDE_LIB) -I$(LSDROOT)/src


clean:
	rm $(SRC_DIR)*.o $(FUN).o $(TARGET)*
