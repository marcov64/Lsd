/*************************************************************

	LSD 9.0 - January 2024
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
LSD dynamic link library (.dll/.so) and LSD No Window.

Relevant macros for conditional compilation (when defined):

- _FUN_: user model equation file
- _NW_: No Window executable
- _NP_: no parallel (multi-task) processing
- _NT_: no signal trapping (better when debugging in GDB)
*************************************************************/

#define LSDLIB

// third-party C++ libraries
#define PUGIXML_NO_XPATH				// XML library
#define PUGIXML_COMPACT
#include "clib/pugixml.hpp"
#include "clib/rapidcsv.h"				// CSV library
#include "clib/eigen.h"					// linear algebra library

// standard libraries
#include <atomic>
#include <cfloat>
#include <cmath>
#include <condition_variable>
#include <csetjmp>
#include <functional>
#include <mutex>
#include <random>
#include <regex>
#include <set>
#include <sys/stat.h>
#include <thread>
#include <unordered_set>
#include <zlib.h>

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#undef DELETE
	#undef THIS
#else
	#include <errno.h>
	#include <signal.h>
	#include <sys/wait.h>
	#include <unistd.h>
	#include <wordexp.h>
#endif

#ifndef _NW_
	#include <tk.h>
#endif

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
#define MAX_BUFF_SIZE 10000				// standard Tcl buffer size (>9999)
#define MAX_CORES 0						// maximum number of cores to use (0=auto )
#define MAX_ELEM_LENGTH 100				// maximum element (obj., var.) name length (>99)
#define MAX_FILE_SIZE 1000000			// max number of bytes to read from files
#define MAX_FILE_TRY 100000				// max number of lines to read from files
#define MAX_LINE_SIZE 1000				// max size of text line to read from file (>999)
#define MAX_OBJ_CHK	10000000			// maximum object number to check when searching
#define MAX_PATH_LENGTH 1000			// maximum path length (>999)
#define MAX_PROF_SIZE 1000				// maximum profile stack size (levels)
#define MAX_SIM_SLEEP 3					// timeout for MT simulation dispatcher (s)
#define MAX_STEPS 100					// default number of simulation steps
#define MAX_STEP_TIMEOUT 60				// timeout for single simulation step (s)
#define MAX_VAR_TIMEOUT 100				// timeout for MT variable scheduler (ms)
#define MAX_WAIT_TIME 10				// maximum time for variable computation (sec.)
#define NOLH_TABS 7						// number of defined NOLH tables
#define NON_AVAILABLE "NA"				// unavailable values text (R default)
#define NO_CONF_NAME "(no name)"		// no configuration file name yet
#define NO_DESCR ""						// no description available text
#define SIG_DIG 10						// number of significant digits in data files
#define UPD_PER 0.2						// update period during simulation run in s
#define USER_D_VARS 1000				// number of user double variables
#define T_CLEVS 10						// number of t distribution confidence levels
#define Z_CLEVS 7						// number of normal distr. confidence levels

// Choose directory/file separator
#define foldersep( dir ) ( dir[ 0 ] == '\0' ? "" : "/" )

// constant string arrays
#define DESC_KEY_NUM 2
#define DESC_KEY_WORDS { "_INIT_", "END_DESCRIPTION" }
#define ELEM_TYPE_NUM 3
#define ELEM_TYPE_NAMES { "variable", "parameter", "function" }
#define REG_SIG_NUM 6
#define REG_SIG_CODE { SIGINT, SIGTERM, SIGABRT, SIGFPE, SIGILL, SIGSEGV }
#define REG_SIG_NAME { "Interrupt signal", "Terminate signal", "Abort signal", \
					   "Floating-point exception", "Illegal instruction", \
					   "Segmentation violation" }

// set default name spaces (C++ STL)
using namespace std;

// classes forward declarations
struct bridge;
struct description;
struct dlliblinkage;
struct lattice;
struct lsdstack;
struct netLink;
struct netNode;
struct object;
struct profile;
struct sensitivity;
struct variable;
struct worker;

// special types used for fast equation, object and variable lookup
typedef function < double( object *caller, variable *var ) > eq_funcT;
typedef pair < string, bridge * > b_pairT;
typedef pair < double, object * > o_pairT;
typedef pair < long, object * > n_pairT;
typedef pair < string, variable * > v_pairT;
typedef vector < object * > o_vecT;
typedef unordered_map < string, eq_funcT > eq_mapT;
typedef unordered_map < string, bridge * > b_mapT;
typedef unordered_map < double, object * > o_mapT;
typedef unordered_map < long, object * > n_mapT;
typedef unordered_map < string, string > p_mapT;
typedef unordered_map < string, variable * > v_mapT;
typedef unordered_set < object * > o_setT;

typedef pugi::xml_document xml_doc;
typedef pugi::xml_node xml_node;
typedef pugi::xml_attribute xml_attr;

#ifndef _NP_
typedef lock_guard < recursive_mutex > rec_lguardT;
typedef unique_lock < recursive_mutex > rec_uniqlT;
#endif

#ifdef _WIN32
typedef HANDLE handleT;
#else
typedef pid_t handleT;
#endif

// class definitions
struct simulation						// simulation container class
{
	// simulation-class variables (used in equations)
	bool fast;							// safe copy of fast_mode flag
	bool no_saved = true;				// disable usage of saved values as lagged ones
	bool no_search;						// disable standard variable search mechanism
	bool no_search_up;					// disable object up-search mechanism
	bool no_zero_instance = true;		// flag to allow deleting last object instance
	bool parallel_mode;					// parallel mode (multithreading) status
	bool use_nan;						// flag to allow using Not a Number value
	char *conf_name = NULL;				// name of current simulation configuration
	dlliblinkage *liblnk = NULL;		// call-back references for DLL
	eq_mapT eq_map;						// fast equation look-up map
	int deb_set = false;				// debug enable control (bool)
	int fast_mode;						// level of LOG messages & runtime plot
	int last_run = 1;					// total serial simulation runs
	int last_t = MAX_STEPS;				// number of simulation steps
	int no_ptr_chk = false;				// disable user pointer checking
	int quit = 0;						// simulation interruption mode (0=none)
	int run;							// current serial simulation run
	int sim;							// library simulation object index
	int t;								// current time step
	object *root = NULL;				// LSD root object
	o_setT obj_list;					// set with all existing LSD objects
	unsigned seed = 1;					// random number generator initial seed

#ifndef _NP_
	mutex lock_obj_list;				// lock object list for parallel manipulation
#endif

#ifndef _NW_
	// simulation-class debugger temporary probe storage (used in equations)
	double d_values[ USER_D_VARS ];
	int i_values[ 4 ];
	netLink *n_values[ 10 ];
	object *o_values[ 10 ];
	FILE *f_values[ 1 ];
#endif

	// simulation-class methods (used in equations)
	char *no_node_chr( const char *lab, const char *file, int line );
	double alapl( double mu, double alpha1, double alpha2 );// asym. laplace draw
	double alaplcdf( double mu, double alpha1, double alpha2, double x );// asym. laplace cdf
	double bernoulli( double p );		// Bernoulli draw
	double beta( double alpha, double beta );// beta draw
	double betacdf( double alpha, double beta, double x );// beta cdf
	double betacf( double a, double b, double x );// beta distribution function
	double binomial( double p, double t );// binomial draw
	double bpareto( double alpha, double low, double high );// bounded pareto draw
	double bparetocdf( double alpha, double low, double high, double x );
	double build_obj_list( bool set_list );// build object list for pointer checking
	double cauchy( double a, double b );// Cauchy draw
	double chi_squared( double n );		// chi-squared draw
	double exponential( double lambda );// exponential draw
	double fact( double x );			// Factorial function
	double fisher( double m, double n );// Fisher-F draw
	double gamma( double alpha, double beta = 1 );// gamma draw
	double geometric( double p );		// geometric draw
	double init_lattice( int init_color = -0xffffff, double nrow = 100, double ncol = 100, double pixW = 0, double pixH = 0 );
	double init_lattice( double pixW = 0, double pixH = 0, double nrow = 100, double ncol = 100, const char lrow[ ] = "y", const char lcol[ ] = "x", const char lvar[ ] = "", object *p = NULL, int init_color = -0xffffff );
	double ipow( double base, double exp );// integer exponentiation
	double lnorm( double mu, double sigma );// lognormal draw
	double lnormcdf( double mu, double sigma, double x );// lognormal cdf
	double median( vector < double > & v );
	double norm( double mean, double dev );// normal draw
	double normcdf( double mu, double sigma, double x );// normal cdf
	double pareto( double mu, double alpha );// Pareto draw
	double paretocdf( double mu, double alpha, double x );
	double poisson( double m );			// Poisson draw
	double poissoncdf( double lambda, double k );// poisson cdf
	double ran1( long *unused = 0 );	// 0-1 uniform draw
	double read_lattice( double line, double col );
	double round_digits( double value, int digits );
	double save_lattice( const char fname[ ] = "lattice" );
	double student( double n );			// Student-T draw
	double t_star( int df, double cl );	// Student-t distribution statistic
	double uniform( double min, double max );// uniform draw
	double uniform_int( double min, double max );// uniform integer draw
	double unifcdf( double a, double b, double x );// uniform cdf
	double update_lattice( double line, double col, double val = 1 );
	double weibull( double a, double b );// Weibull draw
	double z_star( double cl );			// Standard normal distribution statistic
	inline bool chk_hook( object *ptr, unsigned num );
	inline bool chk_obj( object *ptr );
	inline bool chk_ptr( object *ptr );
	inline char *bad_ptr_chr( object *ptr, const char *file, int line );
	inline double bad_ptr_dbl( object *ptr, const char *file, int line );
	inline double no_node_dbl( const char *lab, const char *file, int line );
	inline double nul_lnk_dbl( const char *file, int line );
	inline eq_funcT chk_eq( const char *lab );
	inline object *cycle_obj( object *parent, const char *label, const char *command );
	inline netLink *bad_ptr_lnk( object *ptr, const char *file, int line );
	inline object *bad_ptr_obj( object *ptr, const char *file, int line );
	inline object *no_hook_obj( object *ptr, unsigned num, const char *file, int line );
	inline object *nul_lnk_obj( const char *file, int line );
	inline void bad_ptr_void( object *ptr, const char *file, int line );
	inline void nul_lnk_void( const char *file, int line );
	void close_lattice( void );
	void close_sim( void );
	void error_hard( const char *boxTitle, const char *boxText, bool defQuit, const char *logFmt, ... );
	void init_map( void );
	void init_random( unsigned seed );	// reset the random number generator seed
	void set_fast( int level );			// enable fast mode
	void *set_random( int gen );		// set random generator engine

#ifdef USER_FUNCS
	USER_FUNCS
#endif

#ifndef _FUN_
	// simulation-class variables (not used in equations)
	bool batch_sequential = false;		// no-window multi configuration job running
	bool batch_loop = false;			// batch multi-config batch loop in process
	bool conf_ok = false;				// a valid configuration file is loaded
	bool error_hard_thread;				// error_hard called in worker thread
	bool grand_total = false;			// produce grand total in batch processing
	bool idle_loop = true;				// main idle loop (no running operation)
	bool message_logged = false;		// new message posted in log window
	bool on_bar;						// indicate bar is being draw in log
	bool parallel_monitor;				// parallel monitor thread status
	bool save_alt = false;				// alternate save path flag
	bool save_ok = true;				// control saving model configuration
	bool user_exception = false;		// indicate exception generated by user code
	bool watch_trigger = false;			// indicate that a watch condition was met
	bool watch_write_mode;				// flag for write-only watch condition
	bool worker_ready;					// parallel worker ready flag
	bool worker_crashed;				// parallel worker crash flag
	char *alt_path = NULL;				// alternative output path
	char *conf_file = NULL;				// name of current configuration file
	char *conf_path = NULL;				// folder where the current configuration is
	char *log_file = NULL;				// name of log file, if any
	char conf_eq_txt[ MAX_FILE_SIZE ] = "";// equations saved in configuration file
	char conf_eq_file[ MAX_PATH_LENGTH ] = "";// equation file name in config. file
	char error_hard_msg1[ MAX_BUFF_SIZE ];// buffer for parallel worker title msg
	char error_hard_msg2[ MAX_BUFF_SIZE ];// buffer for parallel worker log msg
	char error_hard_msg3[ MAX_BUFF_SIZE ];// buffer for parallel worker box msg
	char rep_file[ MAX_PATH_LENGTH ] = "";// documentation report file name
	char res_path[ MAX_PATH_LENGTH ] = "";// path of last used results directory
	char watch_elem[ MAX_ELEM_LENGTH + 1 ] = "";// elem. triggering watch condition
	clock_t start_profile[ MAX_PROF_SIZE ];// profile-level start times
	clock_t end_profile[ MAX_PROF_SIZE ];// profile-level end times
	description *descr = NULL;			// model description structure
	int add_to_tot = false;				// type of totals file generated (bool)
	int deb_t;							// next debug stop time step (0 for none)
	int dobar = false;					// enable progress bar in log/standard output
	int docsv = false;					// produce .csv text results files (bool)
	int dozip = true;					// compressed results file flag (bool)
	int fend;							// last multi configuration job to run
	int findex;							// current multi configuration job
	int last_dispatch_time;				// last time step controlled by dispatcher
	int log_start;						// first period to start logging to file
	int log_stop;						// last period to log to file, if any
	int max_runs;						// maximum number of parallel runs
	int max_threads;					// maximum parallel threads per run
	int no_res = false;					// do not produce .res results files (bool)
	int no_tot = true;					// do not produce .tot totals files (bool)
	int parallel_disable = false;		// flag to control parallel mode
	int prof_aggr_time = false;			// show aggregate profiling times
	int prof_min_msecs = 0;				// profile variables taking more than X msecs.
	int prof_obs_only = false;			// profile only observed variables
	int ran_gen_id = 2;					// ID of initial generator (DO NOT CHANGE)
	int series_saved = 0;				// number of series saved
	int stack_level;					// LSD stack call level
	int stack_info = 0;					// LSD stack control
	int stale_time;						// time passed from last step computation
	lattice *latt = NULL;				// model lattice
	long idum = 0;						// Park-Miller default seed (legacy code)
	long nodesSerial = 1;				// network node serial number counter
	lsdstack *stack_log = NULL;			// LSD stack
	map < string, profile > prof_times;	// set of saved profiling times
	minstd_rand lc1;					// linear congruential generator (internal)
	minstd_rand lc2;					// linear congruential generator (user)
	mt19937 mt32;						// Mersenne-Twister 32 bits generator
	mt19937_64 mt64;					// Mersenne-Twister 64 bits generator
	object *blueprint = NULL;			// LSD blueprint (effective model in use)
	object *wait_delete = NULL;			// LSD object waiting for deletion
	random_device rd;					// simulation random device
	ranlux24 lf24;						// lagged fibonacci 24 bits generator
	ranlux48 lf48;						// lagged fibonacci 48 bits generator
	sensitivity *sens = NULL;			// LSD sensitivity analysis structure
	variable *cemetery = NULL;			// LSD saved data from deleted objects
	variable *last_cemetery = NULL;		// LSD last saved cemetery entry
	vector < string > res_list;			// list of results files last saved
	FILE *log_file_ptr;					// log file pointer, if any

#ifndef _NP_
	// simulation-class conditional variables (not used in equations)
	atomic < bool > running = false;	// single simulation is running
	atomic < bool > running_seq = false;// set of sequential simulations running
	atomic < bool > parallel_ready;		// indicate variable worker is ready
	atomic < int > eff_t = 0;			// number of executed time steps
	atomic < int > alaplErrCnt, bernoErrCnt, betaErrCnt, binomErrCnt, cauchErrCnt,
				   chisqErrCnt, expErrCnt, fishErrCnt, gammaErrCnt, geomErrCnt,
				   lnormErrCnt, normErrCnt, paretErrCnt, poissErrCnt, studErrCnt,
				   weibErrCnt;			// math error count control
	condition_variable upd_workers;		// worker schedule update signal
	mutex draw_lc1_lck;					// locks for random generator operations
	mutex draw_lc2_lck;
	mutex draw_lf24_lck;
	mutex draw_lf48_lck;
	mutex draw_mt32_lck;
	mutex draw_mt64_lck;
	mutex draw_rd_lck;
	mutex error_lck;					// control multiple error_hard calls
	mutex run_logs_lck;					// lock run_logs for parallel updating
	mutex run_pids_lck;					// lock run_pids for parallel updating
	mutex run_status_lck;				// lock run_status for parallel updating
	mutex seq_end_lck;					// lock seq_end for parallel updating
	mutex var_update_lck;				// control worker variable update
	mutex wrk_crash_lck;				// control worker crash handling
	string run_log;						// consolidated runs log
	thread run_monitor;					// thread monitoring parallel instances
	thread sim_thread;					// thread object where simulation is run
	vector < handleT > run_pids;		// parallel running instances process id's
	vector < int > run_status;			// parallel running instances status
	vector < string > run_logs;			// log file list produced in parallel runs
	vector < string > run_results;		// parallel run results files
	vector < thread > run_threads;		// parallel running instances
	worker *workers = NULL;				// multi-thread parallel worker data
#else
	bool running = false;				// single simulation is running
	bool running_seq = false;			// set of sequential simulations running
	int eff_t = 0;						// number of executed time steps
	int alaplErrCnt, bernoErrCnt, betaErrCnt, binomErrCnt, cauchErrCnt,
		chisqErrCnt, expErrCnt, fishErrCnt, gammaErrCnt, geomErrCnt,
		lnormErrCnt, normErrCnt, paretErrCnt, poissErrCnt, studErrCnt,
		weibErrCnt;						// math error count control
#endif

#ifndef _NW_
// library Tcl/Tk specific definitions (for the GUI version only)
	p_mapT par_map;						// variable to parent name map for AoR
	Tcl_Interp *inter;					// Tcl interpreter (for legacy LSD code)
#endif

#endif
	// simulation-class methods (not used in equations)
	simulation( void );					// constructor
	~simulation( void );				// destructor

	bool load_txt_description( const char *msg, FILE *f );
	bool next_batch( void );
	bool results_alt_path( const char *altPath );
	bool save_txt_configuration( const char *path, const char *rname, const char *ext, const char eq_file[ ], const char eq_txt[ ] = "" );
	bool save_xml_configuration( int findex = 0, const char *dest_path = NULL, bool quick = false, const char mod_nam[ ] = "", const char mod_ver[ ] = "", const char mod_dat[ ] = "", const char eq_file[ ] = "", const char eq_txt[ ] = "" );
	bool stop_parallel( void );
	description *add_description( const char *lab, int type = 4, const char *text = NULL, const char *init = NULL, bool initial = false, bool observe = false );
	description *change_description( const char *lab_old, const char *lab = NULL, int type = -1, const char *text = NULL, const char *init = NULL, int initial = -1, int observe = -1 );
	description *search_description( const char *lab, bool add_missing = true );
	int init_new_run( clock_t & start, clock_t & last_update );
	int init_new_seq( char *bar_done, int & perc_done, int & last_done );
	int load_configuration( bool reload, std::string *warnings, int quick );
	int load_txt_configuration( bool reload, int quick );
	int hyper_count( const char *lab );
	int hyper_count_var( const char *lab );
	int monitor_logs( void );
	int rnd_int( int min, int max );
	int run_parallel( bool nw, const char *exec, const char *simname, int fseed, int runs, int thrrun, int parruns );
	int run_simulation( int until_t = 0, int until_run = 0 );
	int worker_errors( void );
	template < class distr > double draw_gen( distr &d );
	void detach_parallel( void );
	void empty_blueprint( void );
	void empty_cemetery( void );
	void empty_description( void );
	void empty_lattice( void );
	void empty_sensitivity( sensitivity *cs = NULL );
	void empty_stack( void );
	void init_math_error( void );
	void log_parallel( bool nw );
	void monitor_parallel( bool nw );
	void move_obj( const char *lab, const char *dest );
	void reset_blueprint( object *r );
	void run_parallel_exec( bool nw, int id, string cmd );
	void save_results( void );
	void unload_configuration( bool full );
	void update_bar( char *bar, int done, int & last_done, int bar_sz );

#ifndef _NP_
	void parallel_update( variable *v, object* p, object *caller = NULL );
	void warn_distr( atomic < int > & errCnt, bool & stopErr, const char *distr, const char *msg );
#else
	void warn_distr( int & errCnt, bool & stopErr, const char *distr, const char *msg );
#endif

#ifdef SIMULATION_EXT
	SIMULATION_EXT
#endif
};

struct object							// simulation model object class
{
	bool *del_flag = NULL;				// address of flag to signal deletion
	bool deleting = false;				// indicate deletion in process
	bool to_compute;
	b_mapT b_map;						// fast lookup map to object bridges
	char *label;
	int acounter = 0;					// "fail safe" when creating labels
	int lstCntUpd = 0;					// period of last counter update
	bridge *b = NULL;
	netNode *node = NULL;				// pointer to network node data structure
	object *hook = NULL;
	object *next = NULL;
	object *up;							// parent object
	o_vecT hooks;
	simulation *sim;					// simulation where object is contained
	variable *v = NULL;
	void *cext = NULL;					// pointer to C++ object extension
	v_mapT v_map;						// fast lookup map to variables

#ifndef _NP_
	mutex obj_comp_lck;					// mutex lock for parallel computations
#endif

	// object-class methods
	bool alloc_save_mem( void );
	bool check_cond( double val1, int lopc, double val2 );
	bool load_txt_insts( const char *file_name, FILE *f );
	bool load_txt_struct( FILE *f );
	bool search_parallel( void );
	bool under_computation( void );
	bool under_comput_var( const char *lab );
	bridge *search_bridge( const char *lab, bool no_error = false );
	double av( const char *lab1, int lag = 0, bool cond = false, const char *lab2 = "", const char *lop = "", double value = NAN );
	double cal( const char *l, int lag = 0 );
	double cal( object *caller, const char *l, int lag = 0 );
	double cal( object *caller, const char *l, int lag, bool force_search );
	double count( const char *lab1, int lag = 0, bool cond = false, const char *lab2 = "", const char *lop = "", double value = NAN );
	double count_all( const char *lab1, int lag = 0, bool cond = false, const char *lab2 = "", const char *lop = "", double value = NAN );
	double increment( const char *lab, double value );
	double initturbo( const char *lab );
	double initturbo( const char *lab, double tot );
	double initturbo_cond( const char *label );
	double init_stub_net( const char *lab, const char* gen, long numNodes = 0, long par1 = 0, double par2 = 0.0 );
	double interact( const char *text, double v, double *tv, int i, int j, int h, int k,
		object *cur, object *cur1, object *cur2, object *cur3, object *cur4, object *cur5,
		object *cur6, object *cur7, object *cur8, object *cur9, netLink *curl, netLink *curl1,
		netLink *curl2, netLink *curl3, netLink *curl4, netLink *curl5, netLink *curl6,
		netLink *curl7, netLink *curl8, netLink *curl9 );
	double last_cal( const char *lab );
	double mav( object *caller, const char *lab, double per, int lag = 0 );
	double mav( object *caller, const char *lab, double per, const double weight[ ] = NULL, int lag = 0 );
	double med( const char *lab1, int lag = 0, bool cond = false, const char *lab2 = "", const char *lop = "", double value = NAN );
	double multiply( const char *lab, double value );
	double overall_max( const char *lab1, int lag = 0, bool cond = false, const char *lab2 = "", const char *lop = "", double value = NAN );
	double overall_min( const char *lab1, int lag = 0, bool cond = false, const char *lab2 = "", const char *lop = "", double value = NAN );
	double perc( const char *lab1, double p, int lag = 0, bool cond = false, const char *lab2 = "", const char *lop = "", double value = NAN );
	double read_file_net( const char *lab, const char *dir = "", const char *base_name = "net", int serial = 1, const char *ext = "net" );
	double recal( const char *l );
	double sd( const char *lab1, int lag = 0, bool cond = false, const char *lab2 = "", const char *lop = "", double value = NAN );
	double search_inst( object *obj = NULL, bool fun = true );
	double stat( const char *lab1, double *v = NULL, int lag = 0, bool cond = false, const char *lab2 = "", const char *lop = "", double value = NAN );
	double stats_net( const char *lab, double *r );
	double sum( const char *lab1, int lag = 0, bool cond = false, const char *lab2 = "", const char *lop = "", double value = NAN );
	double to_delete( void );
	double turboset( const char *lab );
	double turboset_cond( const char *lab );
	double whg_av( const char *lab1, const char *lab2, int lag = 0, bool cond = false, const char *lab3 = "", const char *lop = "", double value = NAN );
	double write( const char *lab, double value, int time, int lag = 0 );
	double write_file_net( const char *lab, const char *dir = "", const char *base_name = "net", int serial = 1, bool append = false );
	int load_xml_insts( xml_node &n, n_mapT &node_map, set < int > &warning );
	int load_xml_struct( xml_node &n, bool quick );
	int logic_op_code( const char *lop, const char *errmsg );
	long init_circle_net( const char *lab, long numNodes, long outDeg );
	long init_connect_net( const char *lab, long numNodes );
	long init_discon_net( const char *lab, long numNodes );
	long init_lattice_net( int nRow, int nCol, const char *lab, int eightNeigbr );
	long init_random_dir_net( const char *lab, long numNodes, long numLinks );
	long init_random_undir_net( const char *lab, long numNodes, long numLinks );
	long init_renyi_erdos_net( const char *lab, long numNodes, double linkProb );
	long init_scale_free_net( const char *lab, long numNodes, long outDeg, double expLink );
	long init_small_world_net( const char *lab, long numNodes, long outDeg, double rho );
	long init_star_net( const char *lab, long numNodes );
	long init_uniform_net( const char *lab, long numNodes, long outDeg );
	long nodes2create( const char *lab, long numNodes );
	netLink *add_link_net( object *destPtr, double weight = 0, double probTo = 1 );
	netLink *add_link_net( const char *nodeName, long startNode, long endNode, double weight = 0, double probTo = 1, bool edge = false );
	netLink *draw_link_net( void );
	netLink *search_link_net( long id );
	object *add_n_objects2( const char *lab, int n, int t_update = -1 );
	object *add_n_objects2( const char *lab, int n, object *ex, int t_update = -1 );
	object *add_node_net( long id = -1, const char *nodeName = "", bool silent = false );
	object *add_obj( const char *label, int num = 1, bool propagate = false );
	object *check_net_struct( const char *nodeLab, bool noErr = false );
	object *draw_node_net( const char *lab );
	object *draw_rnd( const char *lo );
	object *draw_rnd( const char *lo, const char *lv, int lag = 0 );
	object *draw_rnd( const char *lo, const char *lv, int lag, double tot );
	object *hyper_next( const char *lab );
	object *hyper_next( void );
	object *lat_down( void );
	object *lat_left( void );
	object *lat_right( void );
	object *lat_up( void );
	object *lsdqsort( const char *obj, const char *var, const char *direction, int lag = 0 );
	object *lsdqsort( const char *obj, const char *var1, const char *var2, const char *direction, int lag = 0 );
	object *search( const char *lab, bool no_search = false, bool no_search_up = true );
	object *search_err( const char *lab, bool no_search, bool no_search_up, const char *errmsg );
	object *search_node_net( const char *lab, long id );
	object *search_var_cond( const char *lab, double value, int lag = 0 );
	object *shuffle_nodes_net( const char *lab );
	object *turbosearch( const char *label, double num );
	object *turbosearch( const char *label, double tot, double num );
	object *turbosearch_cond( const char *label, double value );
	variable *add_empty_var( const char *str );
	variable *add_var_from_example( variable *example );
	variable *search_var( object *caller, const char *label, bool no_error = false, bool no_search = false, bool no_search_up = false, bool search_sons = false );
	variable *search_var_err( object *caller, const char *label, bool no_search, bool no_search_up, bool search_sons, const char *errmsg );
	void chg_lab( const char *lab );
	void chg_var_lab( const char *old, const char *n );
	void collect_cemetery( variable *caller = NULL );
	void collect_inst( o_setT &list );
	void copy_descendant( object *to );
	void delete_bridge( void );
	void delete_link_net( netLink *ptr );
	void delete_net( const char *lab );
	void delete_node_net( void );
	void delete_obj( variable *caller = NULL );
	void delete_var( const char *lab );
	void empty( void );
	void init( object *_up, simulation *_sim, const char *_label, bool _to_compute = true );
	void name_node_net( const char *nodeName );
	void recreate_maps( void );
	void replicate( int num, bool propagate = false );
	void reset_description( void );
	void reset_end( void );
	void save_txt_description( FILE *f );
	void save_txt_insts( FILE *f );
	void save_txt_struct( FILE *f, const char *tab );
	void save_xml_struct( xml_node &pn, long &node_serial, bool quick );
	void search_inst( object *obj, long *pos, long *checked );
	void set_blueprint( object *container );
	void set_tit_counter( void );
	void update( bool recurse, bool user );
	FILE *search_txt_data( const char *name, const char *init, const char *str );

#ifdef OBJECT_EXT
	OBJECT_EXT
#endif
};

struct bridge							// descendant-object container class
{
	bool copy = false;					// just a temporary copy
	bool counter_updated = false;
	bridge *next = NULL;
	char *blabel;						// bridge label (same as parent)
	char *search_var = NULL;			// current initialized search variable
	n_mapT t_map;						// turbosearch map
	object *head = NULL;
	o_mapT o_map;						// fast lookup map to object values

	bridge( const char *lab );			// constructor
	bridge( const bridge &b );			// copy constructor
	~bridge( void );					// destructor
};

struct variable							// model numeric element (variable,
{										// parameter, or function) class
	char *label = NULL;
	char *lab_tit = NULL;
	char deb_mode = 'n';
	bool dummy = false;
	bool initialized = false;
	bool integer = false;				// variable must be rounded to integer
	bool observe = false;
	bool parallel = false;
	bool plot = false;
	bool save = false;
	bool savei = false;
	bool under_computation = false;
	double *data = NULL;
	double *val = NULL;
	double deb_cnd_val = 0;
	double max_val = NAN;				// maximum limit for variable
	double min_val = NAN;				// minimum limit (NAN = no limit)
	eq_funcT eq_func = NULL;			// pointer to equation function
	int deb_cond = 0;
	int delay = 0;
	int delay_range = 0;
	int end = 0;
	int last_update = 0;
	int next_update = 0;
	int num_lag = 0;
	int param = 0;
	int period = 1;
	int period_range = 0;
	int start = 0;
	object *up = NULL;
	simulation *sim = NULL;				// simulation where object is contained
	variable *next = NULL;

#ifndef _NP_
	recursive_mutex var_comp_lck;		// mutex lock for parallel computation
#endif

	variable( void ) { };				// constructor (empty)
	variable( const variable &v );		// copy constructor
	~variable( void );					// destructor

	bool alloc_save_var( void );
	double cal( object *caller, int lag );
	double chk_val( double val );
	double fun( object *caller );
	inline double chk_dummy( const char *lab );
	inline double chk_res( double res );
	void add_cemetery( void );
	void empty( bool no_lock = false );
	void init( object *_up, simulation *_sim, const char *_label, int _param = -1,
			   int _num_lag = -1, double *_val = NULL );
	void save_single( void );
	void set_lab_tit( void );

#ifdef VARIABLE_EXT
	VARIABLE_EXT
#endif
};

struct sensitivity						// sensitivity analysis container class
{
	bool integer;						// variable must be rounded to integer
	char *label = NULL;					// variable name
	double *val = NULL;					// values to test sensitivity
	int cur_val = 0;					// index for value in use in combinations
	int lag;							// lag of initial value
	int num_val = 0;					// number of values to test
	int param;							// element type
	sensitivity *next = NULL;			// sensitivity analysis chain of elements
	simulation *sim;					// simulation where object is contained

	sensitivity( const char *lab, simulation *_sim, int _param, int _lag,
				 bool _integer, int _num_val = 0, vector < double > *_val = NULL );
										// constructor
	~sensitivity( void );				// destructor

#ifdef SENSITIVITY_EXT
	SENSITIVITY_EXT
#endif
};

struct description						// model-element description class
{
	char *init = NULL;
	char *label;
	char *text;
	char *type;
	bool initial = false;
	bool observe = false;
	description *next = NULL;

	description( const char *_label, int _type, const char *_text,
				 const char *_init, bool _initial, bool _observe );// constructor
	~description( void );

	bool has_descr_text( void );
};

struct netNode							// network node data class
{
	char *name = NULL;					// node textual name (not required)
	double prob;						// assigned node draw probability
	int time;							// time of creation/update
	long id;							// node unique ID number (reorderable)
	long nLinks = 0;					// number of arcs FROM node
	long serNum;						// node serial number (for file save/export)
	netLink *first = NULL;				// first link in the linked list of links
	netLink *last = NULL;				// last link in the linked list of links
	object *up;							// object containing node

	netNode( object *_up, long nodeId = -1, const char nodeName[ ] = "",
			 double nodeProb = 1 );		// constructor
	~netNode( void );					// destructor
};

struct netLink							// individual outgoing network link class
{
	double probTo;						// destination node draw probability
	double weight;						// link weight
	int time;							// time of creation/update
	netLink *next = NULL;				// pointer to next link (NULL if last )
	netLink *prev;						// pointer to previous link (NULL if first )
	object *from;						// network node containing the link
	object *to;							// pointer to destination number

	netLink( object *origNode, object *destNode, double linkWeight = 0, double destProb = 1 );
										// constructor
	~netLink( void );					// destructor
};

struct lattice							// model (visual) lattice data class
{
	int **array = NULL;					// lattice data colors array
	int rows = 0;						// lattice size
	int columns = 0;
	int errors = 0;						// error counter
	double width = 0;					// lattice screen size
	double height = 0;
};

#ifndef _NP_
struct worker							// multi-thread variable worker data
{
	bool free = false;
	bool running = false;
	bool errored;
	bool user_excpt;
	char err_msg1[ MAX_BUFF_SIZE ] = "";
	char err_msg2[ MAX_BUFF_SIZE ] = "";
	char err_msg3[ MAX_BUFF_SIZE ] = "";
	condition_variable run;
	exception_ptr pexcpt = nullptr;
	int signum = -1;
	jmp_buf env;
	mutex worker_lck;
	simulation *sim = NULL;				// simulation where object is contained
	thread worker_thread;
	thread::id thread_id;
	variable *v = NULL;

	~worker( void );					// destructor

	bool check( void );					// handle worker problems
	static void signal_wrapper( int signun );// wrapper for signal_handler
	void cal( variable *_v );			// start worker calculation
	void cal_worker( void );			// worker thread code
	void signal( int signum );			// signal handler
};
#endif

struct result							// results file container class
{
	bool docsv;							// comma separated .csv text format
	bool dozip;							// compressed file flag
	bool firstCol;						// flag for first column in line
	gzFile fz = NULL;					// compressed file pointer
	simulation *sim;					// simulation where object is contained
	FILE *f = NULL;						// uncompressed file pointer

	result( const char *fname, const char *fmode, simulation *_sim,
			bool _dozip = false, bool _docsv = false );// constructor
	~result( void );					// destructor

	void data( object *root, int initstep, int endtstep = 0 );	// write data
	void data_recursive( object *r, int i );	// save a single time step (recursively)
	void title( object *root, int flag );// write file header
	void title_recursive( object *r, int i );	// write file header (recursively)
};

struct lsdstack							// simulation-stack element class
{
	char label[ MAX_ELEM_LENGTH ] = "";
	int n = 0;
	lsdstack *next = NULL;
	lsdstack *prev = NULL;
	variable *v = NULL;
};

struct profile							// profiled variable class
{
	unsigned int comp = 0;
	unsigned long long ticks = 0;
};

struct dlliblinkage						// callback references for dynamic link library
{
	bool ( *runtime_step ) ( void ) = NULL;
	double ( *save_lattice_helper ) ( const char *fname ) = NULL;
	double ( *update_lattice_helper ) ( double line, double col, double val,
										int line_int, int col_int,
										int val_int ) = NULL;
	int ( object::*debugger ) ( object *c, const char *lab, double *res,
								bool interact, const char *hl_var ) = NULL;
	void ( *cmd_backend ) ( const char *cm, va_list arg ) = NULL;
	void ( *cover_browser ) ( const char *text1, const char *text2,
							  bool run ) = NULL;
	void ( *deb_log ) ( bool on, int time ) = NULL;
	void ( *disable_plot ) ( void ) = NULL;
	void ( *enable_plot ) ( void ) = NULL;
	void ( *error_hard_helper ) ( const char *boxTitle, const char *boxText,
								  const char *logText, bool defQuit ) = NULL;
	void ( *init_lattice_helper ) ( double pixW, double pixH, double nrow,
									double ncol, int init_color ) = NULL;
	void ( *log_tcl_error ) ( bool show, const char *cm,
							 const char *message, ... ) = NULL;
	void ( *plog_backend ) ( const char *cm, const char *tag,
							 va_list arg ) = NULL;
	void ( variable::*plot_runtime ) ( void ) = NULL;
	void ( *print_stack ) ( void ) = NULL;
	void ( *runtime_buttons ) ( clock_t &last_update ) = NULL;
	void ( *runtime_end ) ( void ) = NULL;
	void ( *runtime_run_end ) ( void ) = NULL;
	void ( *runtime_run_start ) ( void ) = NULL;
	void ( *runtime_start ) ( void ) = NULL;
};

// library global variables (used in equations)
extern const bool no_pointer_check;		// user pointer checking static disable
extern const bool no_pointer_init;		// user pointer initialization disable
extern int platform;					// OS platform (1=Linux, 2=Mac, 3=Windows)

// library C++ functions (used in equations)
void msleep( unsigned msec = 1000 );	// sleep process for milliseconds
void plog( const char *msg, ... );		// write on log window

#ifndef _FUN_

// library global variables (not used in equations)
extern char *exec_file;					// name of executable file
extern char *exec_path;					// path of executable file
extern char *lib_file;					// name of shared library, if any
extern char *lib_path;					// path of shared library, if any
extern char *model_path;				// folder where the model files are
extern char *rootLsd;					// path of LSD root directory
extern const char nonavail[ ];			// string for unavailable values
extern const char *desc_key_words[ ];	// library constant string arrays
extern const char *elem_type_names[ ];
extern const char *signal_names[ ];
extern const double t_dist_cl[ T_CLEVS ];// t-distribution table confidence
extern const double t_dist_st[ T_CLEVS ][ 36 ];// t-distribution table statistics
extern const double z_dist_cl[ Z_CLEVS ];// normal distribution table confidence
extern const double z_dist_st[ Z_CLEVS ];// normal distribution table statistics
extern const int signals[ ];			// handled system signal numbers
extern int choice;						// Tcl menu control (main window)
extern vector < simulation * > sims;	// vector holding existing simulations
extern FILE *stderr_ptr;				// main thread standard error pointer
extern FILE *stdout_ptr;				// main thread standard output pointer

#ifndef _NP_
// library conditional variables (not used in equations)
extern condition_variable seq_end;		// signal simulation sequence end
extern map < thread::id, worker * > worker_thread_ptr;// worker thread pointers
extern mutex plog_term_lck;				// lock plog_terminal for parallel upd.
extern mutex wrk_thr_ptr_lck;			// lock worker_thread_ptr for par. upd.
extern thread::id main_thread;			// LSD main thread ID
#endif

// library C++ functions (not used in equations)
bool strwsp( const char *str );
bool valid_label( const char *lab );
bool valid_xml_string( const char *lab );
char *clean_file( const char *file );
char *clean_path( const char *path );
char *strcatn( char *d, const char *s, size_t dSz );
char *strcpyn( char *d, const char *s, size_t dSz );
char *strdecdata( char *out, const char *in, int outSz = 0 );
char *strencdata( char *out, const char *in, int outSz = 0 );
char *strupr( char *s );
const char *signal_name( int signum );
int dispatch_runs( int until_t = 0, int until_run = 0 );
int kill_system( simulation *sim, int id );
int run_system( const char *cmd, simulation *sim = NULL, int id = -1 );
int strcln( char *out, const char *str, int outSz );
int strlf( char *out, const char *str, int outSz );
int strtrim( char *out, const char *str, int outSz );
int strtrimin( char *out, const char *str, int outSz );
int strwrap( char *out, const char *str, int outSz, int wid );
long strtol( const char *in, char** endptr, int base, long inv );
object *go_brother( object *c );
object *skip_next_obj( object *t );
object *skip_next_obj( object *t, int *count );
string to_string( const char *fmt, double val );
vector < double > strtodsplit( const char *in, char sep, double inv = 0. );
vector < long > strtolsplit( const char *in, char sep, long inv = 0 );
vector < string > strtostrsplit( const char *in, char sep, bool remQuotes = false );
void close_sim( void );
void cmd_gui( const char *cm, ... );
void exception_handler( int signum, const char *what );
void handle_signals( void ( * handler ) ( int signum ) );
void init_map( void );
void lsd_exit( int v );
void plog_tag( const char *cm, const char *tag, ... );
void plog_terminal( const char *cm, va_list arg );
void set_exec( const char *path, const char *file );
void signal_handler( int signum );

#endif
