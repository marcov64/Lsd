#*************************************************************
#
#	LSD 9.0 - January 2026
#	written by Marco Valente, Universita' dell'Aquila
#	and by Marcelo Pereira, University of Campinas
#
#	Copyright Marco Valente and Marcelo Pereira
#	LSD is distributed under the GNU General Public License
#
#	See Readme.txt for copyright information of
#	third parties' code used in LSD
#
#*************************************************************

#*************************************************************
# INDUSTRY_RUN_EXAMPLE.PY
# Simple example of Python interface to LSD.
#*************************************************************

# This script requires the Python-native dynamic-link library
# containing the LSD interface (lsd_api.XXXX.dll/.so/.dylib),
# and the terminal executable dynamic-link library containing
# the model code (liblsd_term.dll/.so/.dylib), both stored
# at the same directory as the script. The DLLs are produced
# LSD Model Manager at the model directory by:
# 1. selecting the menu Model > Model Options, adding the
# line "PYTHON_API=true" (without the quotes), and pressing OK
# 2. selecting menu option Model > Create Terminal Executable
#
# In Windows, it is probably required to use the version of
# Python embedded with LSD to run the script. If no other Python
# version is installed, the LSD version is the default and no
# further action is required. If another version is installed,
# please use the version at:
#  <LSD directory>\gnu\bin\python3.exe
#
# In Windows, it is probably required to use the version of
# Python embedded with LSD to run the script. If no other Python
# version is installed, the LSD version is the default and no
# further action is required. If another version is installed,
# please use the version at:
#  <LSD directory>\gnu\bin\python3.exe

import os, sys
import lsd_api as lsd			# LSD library classes and functions

# function to print the object tree
def printTree( obj ) :

	print( "Object name: %s" % obj.name )

	print( "  '%s' elements: [ " % obj.name, end = "" )
	for elem in obj.elements :
		print( elem.name + " ", end = "" )
	print( "]" )

	for elem in obj.elements :
		print( "    Element name: %s | type: %s" % ( elem.name, elem.type ), end = "" )
		if elem.type == "variable" :
			print( " | lags: %s" % elem.lags, end = "" )
		print( " | values:", elem.values )

	print( "  '%s' sons: [ " % obj.name, end = "" )
	for son in obj.descendants :
		print( son.name + " ", end = "" )
	print( "]" )

	for son in obj.descendants :
		printTree( son )


# main simulation run
lsd.initLSD( __file__ )			# initialize LSD

sim = lsd.Simulation( )			# single LSD simulation terminal instance

if sim.options( zip = False, csv = True ) :	# apply non-default configurations

	if sim.config( "Baseline-Beta.lsd" ) :	# load existing configuration file

		printTree( sim.root )	# print the model structure tree

		print( "Configuration seed: %d" % sim.seed )
		sim.seed = 13			# change pseudo-random number generator seed
		print( "New seed: %d" % sim.seed )

		for t in range( 1, sim.lastT + 1 ) :	# run time steps one at a time

			res = sim.run( untilT = t )			# execute current step

			print( "t=%d curT=%d/%d run=%d/%d running = %d" % ( t, sim.curT, sim.lastT, sim.curRun, sim.lastRun, sim.running ) )
			print( "HHI=%.2f | dS=%.2f | aAvg=%.0f" % ( sim.root.searchElem( "HHI" ).values[ 0 ], sim.root.searchElem( "dS" ).values[ 0 ], sim.root.searchElem( "aAvg" ).values[ 0 ] ) )

			if res == 1 :						# step run ok?
				continue						# next step

			if res != 0 :						# did not finish?
				print( "Simulation error (%d)" % res )	# simulation stopped
				break
	else :
		print( "Could not load configuration" )
		sys.exit( 2 )
else :
	print( "Invalid options" )
	sys.exit( 1 )

# show saved time series
print( "HHI=", sim.root.searchElem( "HHI" ).data )
print( "dS=", sim.root.searchElem( "dS" ).data )
print( "aAvg=", sim.root.searchElem( "aAvg" ).data )

