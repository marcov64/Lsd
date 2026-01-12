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

.PHONY: all

# build all
all:
	cd src && $(MAKE)
	cd lwi && $(MAKE)

# LMM executable
lmm:
	cd src && $(MAKE) lmm

# LSD executable, static and dynamic libraries
lsd:
	cd src && $(MAKE) lsd

# LSD Web Interface executables
lwi:
	if [ -d "lwi" ]; then cd lwi && $(MAKE); fi

# delete all executables, libraries and object files
clean:
	cd src && $(MAKE) clean
	cd lwi && $(MAKE) clean
