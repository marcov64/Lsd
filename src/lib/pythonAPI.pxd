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
	cdef cppclass simulation :
		char *conf_file			# name of current configuration file
		char *conf_name			# name of first/current simulation configuration
		char *alt_path			# alternative output path
		bint dobar				# enable progress bar in standard output (bool)
		bint docsv				# produce .csv text results files (bool)
		bint dozip				# compressed results file flag (bool)
		bint no_res				# do not produce .res results files (bool)
		bint no_tot				# do not produce .tot totals files (bool)
		bool conf_ok			# a valid configuration file is loaded
		int max_threads			# maximum parallel threads per run

		bool results_alt_path( const char *altPath ) except +
		int load_configuration( bool reload, string *warnings, int quick ) except +
		int run_simulation( int until_t, int until_run, bool da ) except +

	cdef cppclass assimilation :
		bint sav_fct			# save forecast (intermediary) results
		bint sav_obs			# save observational data
		bint sav_dsp			# save data dispersion matrix
		bint disable			# disable data assimilation
		double conf_lev			# confidence interval confidence level (%)

		int count( int what ) except +
		int run_simulation( int until_t ) except +


	# LSD library global variables
	cdef assimilation *da		# data assimilation object container


	# LSD library global variables
	int dispatch_runs( vector[ simulation ] & run_sims, int until_t, int until_run, bool da )
	void init_lib( )
	void set_exec( const char *path, const char *file )
