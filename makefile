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
# Makefile for all LSD components
# In a command prompt (terminal) in LSD root directory, use
#  make
#**************************************************************

# LSD file locations
LSDROOT=.
SRC_DIR=$(LSDROOT)/src
LWI_DIR=$(LSDROOT)/lwi


.PHONY: all

# build all
all:
	$(MAKE) -C $(SRC_DIR)
	if [ -d "$(LWI_DIR)" ]; then $(MAKE) -C $(LWI_DIR); fi

# LMM executable
lmm:
	cd $(SRC_DIR) && $(MAKE) lmm

# LSD executable, static and dynamic libraries
lsd:
	cd $(SRC_DIR) && $(MAKE) lsd

# LSD Web Interface executables
lwi:
	if [ -d "$(LWI_DIR)" ]; then $(MAKE) -C $(LWI_DIR); fi

# delete all executables, libraries and object files
clean:
	cd $(SRC_DIR) && $(MAKE) clean
	if [ -d "$(LWI_DIR)" ]; then cd $(LWI_DIR) && $(MAKE) clean; fi
