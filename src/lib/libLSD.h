/*************************************************************

	LSD 9.0 - January 2026
	written by Marco Valente, Universita' dell'Aquila
	and by Marcelo Pereira, University of Campinas

	Copyright Marco Valente and Marcelo Pereira
	LSD is distributed under the GNU General Public License

	See Readme.txt for copyright information of
	third parties' code used in LSD

 *************************************************************/

/*************************************************************
 LIBLSD.H
 Global definitions shared by all LSD modules contained in
 LSD dynamic link library (.dll/.so) and LSD terminal executable.

 Relevant macros for conditional compilation (when defined):

 - _EQ_: user model equation file
 - _TERM_: terminal executable
 - _LMM_: LMM executable
 - _LWI_: LSD Web Interface executables
 - _NT_: no signal trapping (better when debugging in GDB)
 *************************************************************/

#define LSDLIB


#ifndef LSDLIBCLASSES
	#include "libclasses.h"
#endif


// standard libraries
#include <cfloat>
#include <cmath>
#include <functional>
#include <regex>
#include <sys/stat.h>

// third-party C++ libraries
#include "clib/rapidcsv.h"				// CSV library

// user defined signals
#define SIGMEM NSIG + 1					// out of memory signal
#define SIGSTL NSIG + 2					// standard library exception signal

// define PI for C++11
#ifndef M_PI
	#define M_PI (3.14159265358979323846)
#endif

// redefine NAN to use faster non-signaling NaNs
#if has_quiet_NaN
	#undef NAN
#define NAN quiet_NaN( )
	#endif
#define NaN NAN

// hardware random generator present?
#ifdef __RDSEED__
	#define HW_RAND_GEN true
#else
	#define HW_RAND_GEN false
#endif

// global constants
#define BAR_DONE_SIZE 80				// characters in the percentage done bar
#define CSV_SEP ","						// single char with the .csv format separator
#define ERR_LIM 5						// maximum number of repeated error messages
#define DEF_CONF_FILE "Sim1"			// default new configuration name
#define LEGACY_NO_DESCR "(no description available)" // legacy description (do not change)
#define MAX_CORES 0						// maximum number of cores to use (0=auto )
#define MAX_FILE_TRY 100000				// max number of lines to read from files
#define MAX_LINE_SIZE 1000				// max size of text line to read from file (>999)
#define MAX_OBJ_CHK	10000000			// maximum object number to check when searching
#define MAX_SIM_SLEEP 3					// timeout for MT simulation dispatcher (s)
#define MAX_STEP_TIMEOUT 60				// timeout for single simulation step (s)
#define MAX_VAR_TIMEOUT 100				// timeout for MT variable scheduler (ms)
#define MAX_WAIT_TIME 10				// maximum time for variable computation (sec.)
#define NOLH_TABS 7						// number of defined NOLH tables
#define NON_AVAILABLE "NA"				// unavailable values text (R default)
#define NO_CONF_NAME "(no name)"		// no configuration file name yet
#define NO_DESCR ""						// no description available text
#define ROOT_NAME "Root"				// name of root element
#define SIG_DIG 10						// number of significant digits in data files
#define UPD_PER 0.2						// update period during simulation run in s
#define T_CLEVS 10						// number of t distribution confidence levels
#define Z_CLEVS 7						// number of normal distr. confidence levels

#define TAG_NONE 0						// series tags - no tag
#define TAG_UPDT 1						// series from update buffer
#define TAG_ANL 2						// series from DA - analysis
#define TAG_FCT 3						// series from DA - forecast
#define TAG_OBS 4						// series from DA - observation data
#define TAG_FILE 5						// series from files
#define TAG_CALC 6						// series from calculations
#define TAG_MC 7						// series from Monte Carlo experiment

// directory/file separator
#define foldersep( dir ) ( dir[ 0 ] == '\0' ? "" : "/" )

// constant string arrays
#define DESC_KEY_NUM 2
#define DESC_KEY_WORD { "_INIT_", "END_DESCRIPTION" }
#define DESC_TYPE_NUM 5
#define DESC_TYPE_NAME { "Variable", "Parameter", "Function", "", "Object" }
#define ELEM_TYPE_NUM 3
#define ELEM_TYPE_NAME { "variable", "parameter", "function" }
#define LOG_OPS_PAIR { { "==", 0 }, { "=", 0 }, { "EQ", 0 }, { "!=", 1 }, { "=!", 1 }, { "NE", 1 }, { ">", 2 }, { "GT", 2 }, { ">=", 3 }, { "=>", 3 }, { "GE", 3 }, { "<", 4 }, { "LT", 4 }, { "<=", 5 }, { "=<", 5 }, { "LE", 5 }, { "NNAN", 6 }, { "NAN", 7 }, { "NINF", 8 }, { "INF", 9 }, { "NFIN", 10 }, { "FIN", 11 } }
#define META_PAR_NUM 3
#define META_PAR_NAME { "_timeSteps_", "_numRuns_", "_rndSeed_" }
#define REG_SIG_NUM 6
#define REG_SIG_CODE { SIGINT, SIGTERM, SIGABRT, SIGFPE, SIGILL, SIGSEGV }
#define REG_SIG_NAME { "Interrupt signal", "Terminate signal", "Abort signal", \
					   "Floating-point exception", "Illegal instruction", \
					   "Segmentation violation" }
#define VAR_TAG_NUM 8
#define VAR_TAG_NAME { "", "U_", "A_", "T_", "O_", "F_", "C_", "MC_" }

// macro functions
#define BROTHER( O ) ( O == NULL ? NULL : O->next )


// global namespace functions (legacy)
void close_sim( void );						// legacy user equation closure


namespace lsd
{
/*************************************************************
 LIBRARY GLOBAL VARIABLES
 *************************************************************/
	extern const bool no_pointer_check;		// user pointer check static disable
	extern const bool no_pointer_init;		// user pointer init. static disable

#ifndef _EQ_
	extern assimilation *da;				// data assimilation object container
	extern char *exec_file;					// name of executable file
	extern char *exec_path;					// path of executable file
	extern char *lib_file;					// name of shared library, if any
	extern char *lib_path;					// path of shared library, if any
	extern char *model_path;				// folder where the model files are
	extern char *root_lsd;					// path of LSD root directory
	extern const char nonavail[ ];			// string for unavailable values
	extern const char *desc_key_words[ ];	// library constant string arrays
	extern const char *desc_type_names[ ];
	extern const char *elem_type_names[ ];
	extern const char *meta_par_names[ ];
	extern const char *signal_names[ ];
	extern const char *tag_pref[ ];
	extern const double t_dist_cl[ T_CLEVS ];// t-distribution table confidence
	extern const double t_dist_st[ T_CLEVS ][ 36 ];// t-distribution table statistics
	extern const double z_dist_cl[ Z_CLEVS ];// normal distribution table confidence
	extern const double z_dist_st[ Z_CLEVS ];// normal distribution table statistics
	extern const int signals[ ];			// handled system signal numbers
	extern const i_mapT logic_ops_map;		// conditional operators
	extern cond_vT seq_end;					// signal simulation sequence end
	extern description *desc;				// element description object container
	extern mtxT plog_term_lck;				// lock plog_terminal for parallel upd.
	extern mtxT wrk_thr_ptr_lck;			// lock worker_thread_ptr for par. upd.
	extern simp_vecT sims;					// vector holding existing simulations
	extern std::mt19937 lib_prng;			// internal pseudo-random number generator
	extern thr_idT main_thread;				// LSD main thread ID
	extern wrk_mapT worker_thread_ptr;		// worker thread pointer map
	extern FILE *stderr_ptr;				// main thread standard error pointer
	extern FILE *stdout_ptr;				// main thread standard output pointer

#ifdef __APPLE__
	extern IOPMAssertionID mac_pwr_assert;	// mac sleep control
#endif


/*************************************************************
 LIBRARY GLOBAL FUNCTIONS
 *************************************************************/
	bool strwsp( const char *str );
	bool valid_label( const char *lab );
	bool valid_xml_string( const char *lab );
	char *clean_file( const char *file );
	char *clean_path( const char *path );
	char *get_path( const char *file );
	char *strcatn( char *d, const char *s, size_t dSz );
	char *strcpyn( char *d, const char *s, size_t dSz );
	char *strdecdata( char *out, const char *in, int outSz = 0 );
	char *strencdata( char *out, const char *in, int outSz = 0 );
	char *strupr( char *s );
	const char *get_str( const char *tcl_var );
	const char *signal_name( int signum );
	double median( d_vecT & v );
	double strtod( const char *in, char** endptr, double inv );
	d_vecT strtodsplit( const char *in, char sep, double inv = 0. );
	int dispatch_runs( sim_vecT & run_sims, int until_t, int until_run, bool da );
	int kill_system( simulation *sim, int id );
	int run_system( const char *cmd, simulation *sim = NULL, int id = -1 );
	int strcln( char *out, const char *str, int outSz );
	int strlf( char *out, const char *str, int outSz );
	int strtrim( char *out, const char *str, int outSz );
	int strtrimin( char *out, const char *str, int outSz );
	int strwrap( char *out, const char *str, int outSz, int wid );
	int strwrds( const char *s );
	long strtol( const char *in, char** endptr, int base, long inv );
	l_vecT strtolsplit( const char *in, char sep, long inv = 0 );
	strT to_string( const char *fmt, ... );
	str_vecT strtostrsplit( const char *in, char sep, bool remQuotes = false );
	void cmd( const char *cm, ... );
	void debug_break( void );
	void empty_assimilation( void );
	void empty_description( description *d = NULL );
	void empty_objattributes( simulation *sim = NULL );
	void empty_varattributes( simulation *sim = NULL );
	void exception_handler( int signum, const char *what );
	void finish_lib( void );
	void handle_signals( void ( * handler ) ( int signum ) );
	void inhibit_system_sleep( void );
	void init_lib( description *_desc = NULL, assimilation *_da = NULL );
	void lsd_exit( int v, bool clean = false );
	void msleep( unsigned msec = 1000 );
	void plog_master( const char *cm, ... );
	void plog_tag_master( const char *cm, const char *tag, ... );
	void restore_system_sleep( void );
	void set_exec( const char *path, const char *file );
	void signal_handler( int signum );
#endif
}
