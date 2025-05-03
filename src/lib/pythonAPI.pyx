#*************************************************************
#
#	LSD 9.0 - January 2024
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
# PYTHONAPI.PYX
# Cython API for LSD models.
#*************************************************************

import os, re
cimport pythonAPI as lsd
from cpython.mem cimport PyMem_Malloc, PyMem_Free
from libc.string cimport strcpy

# wrapped C++ classes

cdef class simulation :
	cdef lsd.simulation sim

	# options: set simulation options
	#  noResults: if True, a results file is not produced
	#  noTotals: if False, a totals file is produced
	#  zip: if False, results file is not gzip compressed
	#  csv: if True, comma-separated values (CSV) format is used for files
	#  progressBar: show text progress bar in standard output
	#  maxThreads: maximum parallel threads to use
	#  altPath: path to save results, if different from configuration file
	def options( self, noResults = False, noTotals = True, zip = True, csv = False, progressBar = False, maxThreads = 0, altPath : str = None ) :
		self.sim.no_res = noResults
		self.sim.no_tot = noTotals
		self.sim.dozip = zip
		self.sim.docsv = csv
		self.sim.dobar = progressBar

		if maxThreads > 0 :
			self.sim.max_threads = maxThreads

		if altPath is None :
			return True
		else :
			return self.sim.results_alt_path( altPath.encode( ) )

	# config: define LSD configuration file(s) to load
	#  confFile: name of LSD configuration file
	def config( self, confFile : str ) :
		confName = re.sub( ".lsd", "", confFile, flags = re.IGNORECASE ).encode( )
		fileName = confName + b".lsd"

		if not os.path.isfile( fileName ) :
			return False

		PyMem_Free( self.sim.conf_name )
		PyMem_Free( self.sim.conf_file )
		self.sim.conf_name = < char * > PyMem_Malloc( len( confName ) + 1 )
		self.sim.conf_file = < char * > PyMem_Malloc( len( fileName ) + 1 )

		if not self.sim.conf_name or not self.sim.conf_file :
			raise MemoryError( )

		strcpy( self.sim.conf_name, confName )
		strcpy( self.sim.conf_file, fileName )

		return self.sim.load_configuration( True, NULL, 1 ) == 0

	# run: execute the loaded configuration
	#  until: time step to stop the simulation (0=end)
	def run( self, until = 0 ) :
		if self.sim.conf_ok :
			return self.sim.run_simulation( until, 0, False )
		else :
			return 1

	# destructor: garbage collection
	def __del__( self ) :
		PyMem_Free( self.sim.conf_name )
		PyMem_Free( self.sim.conf_file )
		self.sim.conf_name = NULL
		self.sim.conf_file = NULL


cdef class assimilation :
	cdef lsd.assimilation da

	# options: set data assimilation options
	#  confLevel: confidence interval level (percentage)
	#  saveForecasts: if True, DA forecasts are saved
	#  saveObservations: if True, DA observation data is saved
	#  saveCovMatrix: if True, DA covariance/comedian matrix is saved
	def options( self, confLevel = 95, saveForecasts = False, saveObservations = False, saveCovMatrix = False ) :
		self.da.conf_lev = confLevel
		self.da.sav_fct = saveForecasts
		self.da.sav_obs = saveObservations
		self.da.sav_dsp = saveCovMatrix

	# enabled: check if data assimilation is enabled (True)
	def enabled( self ) :
		return not self.da.disable and self.da.count( 4 ) > 0

	# run: execute the loaded data assimilation configuration
	#  until: time step to stop the simulation (0=end)
	def run( self, until = 0 ) :
		if self.sim.conf_ok :
			return self.da.run_simulation( until )
		else :
			return 1

	# constructor: register at library
	def __init__( self ) :
		lsd.da = & self.da


# wrapped C++ functions

# init.LSD: initialize LSD
#  fileScript: name of script, including the full path if pathScript is None
def initLSD( fileScript : str ) :
	lsd.init_lib( )								# initialize LSD library
	lsd.set_exec( NULL, fileScript.encode( ) )	# assume script path is included in file name
