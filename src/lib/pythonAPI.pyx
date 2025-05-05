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

# Simulation: main LSD class containing both configuration and data
cdef class Simulation :
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
	#  untilT: time step to stop the simulation (0=end)
	#  untilRun: serial simulation run to stop the simulation (0=all)
	def run( self, untilT = 0, untilRun = 0 ) :
		if self.sim.conf_ok :
			res = self.sim.run_simulation( untilT, untilRun, False )

			if res == -2 :
				return 0		# simulation paused
			elif res == 0 :
				return 1		# simulation finished
			else :
				return res		# error
		else :
			return -1

	# searchObj: return the root object of the structure tree
	@property
	def root( self ) :
		return Object.fromPtr( self.sim.root )

	# running: True if simulation is running (started but not yet finished)
	@property
	def running( self ) :
		return self.sim.running

	# curT: the current (completed) time steps of simulation run(s)
	@property
	def curT( self ) :
		return self.sim.eff_t

	# curRun: the number of serial simulation runs to perform
	@property
	def curRun( self ) :
		return self.sim.run

	# lastT: the number of time steps per simulation run
	@property
	def lastT( self ) :
		return self.sim.last_t

	# lastRun: the number of serial simulation runs to perform
	@property
	def lastRun( self ) :
		return self.sim.last_run

	# destructor: garbage collection
	def __dealloc__( self ) :
		PyMem_Free( self.sim.conf_name )
		PyMem_Free( self.sim.conf_file )
		self.sim.conf_name = NULL
		self.sim.conf_file = NULL


# Object: class containing a node element in structure tree
cdef class Object :
	cdef lsd.cobject *objPtr

	# searchObj: search the structure tree for an object instance and return it
	#  name: name of object below this object
	def searchObj( self, name : str ) :
		if self.objPtr is not NULL :
			return Object.fromPtr( self.objPtr.search( name.encode( ), False, True ) )
		else :
			return None

	# searchElem: search the structure tree for an element instance and return it
	#  name: name of element (variable/parameter) below this object
	def searchElem( self, name : str ) :
		if self.objPtr is not NULL :
			return Element.fromPtr( self.objPtr.search_var( NULL, name.encode( ), True, False, True, True ) )
		else :
			return None

	# name: get the name of object
	@property
	def name( self ) :
		if self.objPtr is not NULL :
			return self.objPtr.attr.label.decode( )
		else :
			return None

	# descendants: get a list with descending (son) objects in object
	@property
	def descendants( self ) :
		bridge = [ ]

		if self.objPtr is not NULL :
			cb = self.objPtr.b
			while cb is not NULL :
				bridge.append( Object.fromPtr( cb.head ) )
				cb = cb.next

		return bridge

	# elements: get a list with elements (variables and parameters) in object
	@property
	def elements( self ) :
		elem = [ ]

		if self.objPtr is not NULL :
			cv = self.objPtr.v
			while cv is not NULL :
				elem.append( Element.fromPtr( cv ) )
				cv = cv.next

		return elem

	# next: get the next object in the chain of siblings
	@property
	def next( self ) :
		if self.objPtr is not NULL :
			return Object.fromPtr( self.objPtr.next )
		else :
			return None

	# fromPtr: factory function to create Object instances
	@staticmethod
	cdef Object fromPtr( lsd.cobject *objPtr ) :
		cdef Object wrapper = Object.__new__( Object )
		if objPtr is not NULL :
			wrapper.objPtr = objPtr
			return wrapper
		else :
			return None

	# constructor: prevent instantiation from Python
	def __init__( self ) :
		raise TypeError( "Cannot be instantiated directly" )

# Element: class containing a node element in structure tree
cdef class Element :
	cdef lsd.variable *varPtr

	# name: get the name of element
	@property
	def name( self ) :
		if self.varPtr is not NULL :
			return self.varPtr.attr.label.decode( )
		else :
			return None

	# type: get the type of element (0=parameter / 1=variable / 2=function)
	@property
	def type( self ) :
		if self.varPtr is not NULL :
			return self.varPtr.param
		else :
			return None

	# lags: get the number of lags of variable element
	@property
	def lags( self ) :
		if self.varPtr is not NULL :
			return self.varPtr.attr.num_lag
		else :
			return None

	# values: get a list with short-term values stored in element
	@property
	def values( self ) :
		val = [ ]

		if self.varPtr is not NULL :
			for i in range( self.varPtr.attr.num_lag + 1 ) :
				val.append( self.varPtr.val[ i ] )

		return val

	# data: get a dictionary with long-term values stored in element with times
	@property
	def data( self ) :
		data = { }

		if self.varPtr is not NULL and self.varPtr.data is not NULL :
			for i in range( self.varPtr.end - self.varPtr.start + 1 ) :
				data[ self.varPtr.start + i ] = self.varPtr.data[ i ]

		return data

	# next: get the next element in the chain of siblings
	@property
	def next( self ) :
		if self.varPtr is not NULL :
			return Element.fromPtr( self.varPtr.next )
		else :
			return None

	# fromPtr: factory function to create Element instances
	@staticmethod
	cdef Element fromPtr( lsd.variable *varPtr ) :
		cdef Element wrapper = Element.__new__( Element )
		if varPtr is not NULL :
			wrapper.varPtr = varPtr
			return wrapper
		else :
			return None

	# constructor: prevent instantiation from Python
	def __init__( self ) :
		raise TypeError( "Cannot be instantiated directly" )


# Assimilation: class containing the data assimilation configuration and data
cdef class Assimilation :
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
			return -1

	# constructor: register at library
	def __cinit__( self ) :
		lsd.da = & self.da


# wrapped C++ functions

# init.LSD: initialize LSD
#  fileScript: name of script, including the full path if pathScript is None
def initLSD( fileScript : str ) :
	lsd.init_lib( )								# initialize LSD library
	lsd.set_exec( NULL, fileScript.encode( ) )	# assume script path is included in file name
