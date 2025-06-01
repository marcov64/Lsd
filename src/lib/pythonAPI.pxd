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
# PYTHONAPI.PXD
# Cython declarations for LSD library.
#*************************************************************

from libcpp cimport bool
from libcpp.string cimport string
from libcpp.vector cimport vector

cdef extern from "libLSD.h" namespace "lsd" :

	# LSD library classes
	cdef cppclass simulation :	# main simulation class
		char *alt_path			# alternative output path
		char *conf_file			# name of current configuration file
		char *conf_name			# name of first/current simulation configuration
		bint dobar				# enable progress bar in standard output (bool)
		bint docsv				# produce .csv text results files (bool)
		bint dozip				# compressed results file flag (bool)
		bint no_res				# do not produce .res results files (bool)
		bint no_tot				# do not produce .tot totals files (bool)
		bool conf_ok			# a valid configuration file is loaded
		bool running			# simulation is running
		int last_run			# total serial simulation runs
		int last_t				# number of simulation steps
		int max_threads			# maximum parallel threads per run
		int run					# current serial simulation run
		int eff_t				# number of executed time steps
		unsigned int seed		# random number generator initial seed
		cobject *root			# LSD root object of structure-tree

		simulation( ) except +

		bool results_alt_path( const char *altPath ) except +
		int load_configuration( bool reload, string *warnings, int quick ) except +
		int run_simulation( int until_t, int until_run, bool da ) except +

	cdef cppclass cobject "lsd::object" :# structure-tree object node element
								# avoid collision with Cython "object" built-in
		bridge *b				# head of list of son-object instances
		objattr *attr			# static/homogeneous attributes object
		cobject *next			# next sibling object
		cobject *up				# parent object
		variable *v				# head of list of contained variables

		cobject *add_n_objects2( const char *lab, int n, int t_update ) except +
		cobject *search( const char *lab, bool no_search, bool no_search_up )
		variable *search_var( cobject *caller, const char *label, bool no_error, bool no_search, bool no_search_up, bool search_sons )
		void delete_obj( const variable *caller = NULL )

	cdef cppclass objattr :		# object static/shared attributes
		char *label				# object name

	cdef cppclass bridge :		# instance in list of descendant objects
		bridge *next			# next son-object type
		cobject *head			# first instance of this object type

	cdef cppclass variable :	# structure-tree variable/parameter element
		double *data			# variable long-term value array
		double *val				# variable short-term value array
		int end					# last time allocated in value array
		int last_update			# last period variable was updated
		int next_update			# next period variable will be updated
		int param				# variable type (0=var/1=var/2=func)
		int start				# first time allocated in value array
		cobject *up				# parent object
		varattr *attr			# static/homogeneous attributes object
		variable *next			# sibling variable under same object

	cdef cppclass varattr :		# variable/parameter static/shared attributes
		bool integer			# variable must be rounded to integer
		bool parallel			# may be executed in parallel
		bool save				# time series to be saved
		bool savei				# time series to sade individually
		char *label				# variable name
		double max_val			# maximum limit for variable
		double min_val			# minimum limit (NAN = no limit)
		int delay				# time from t=0 to start computation
		int delay_range			# maximum range for random delay
		int num_lag				# number of lags kept in value array
		int period				# period between updates
		int period_range		# maximum range for random updates

	cdef cppclass assimilation :
		bint sav_fct			# save forecast (intermediary) results
		bint sav_obs			# save observational data
		bint sav_dsp			# save data dispersion matrix
		bint disable			# disable data assimilation
		double conf_lev			# confidence interval confidence level (%)

		assimilation( )			# constructor
		int count( int what )
		int run_simulation( int until_t ) except +


	# LSD library global variables
	cdef assimilation *da		# data assimilation object container


	# LSD library global variables
	int dispatch_runs( vector[ simulation ] & run_sims, int until_t, int until_run, bool da )
	void init_lib( assimilation *da )
	void set_exec( const char *path, const char *file )
