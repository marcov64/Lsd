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

// directory/file separator
#define foldersep( dir ) ( dir[ 0 ] == '\0' ? "" : "/" )

// constant string arrays
#define DESC_KEY_NUM 2
#define DESC_KEY_WORD { "_INIT_", "END_DESCRIPTION" }
#define DESC_TYPE_NUM 5
#define DESC_TYPE_NAME { "Variable", "Parameter", "Function", "", "Object" }
#define ELEM_TYPE_NUM 3
#define ELEM_TYPE_NAME { "variable", "parameter", "function" }
#define LOG_OPS_PAIR { { "==", 0 }, { "=", 0 }, { "EQ", 0 }, { "!=", 1 }, { "=!", 1 }, { "NE", 1 }, { ">", 2 }, { "GT", 2 }, { ">=", 3 }, { "=>", 3 }, { "GE", 3 }, { "<", 4 }, { "LT", 4 }, { "<=", 5 }, { "=<", 5 }, { "LE", 5 } }
#define META_PAR_NUM 3
#define META_PAR_NAME { "_timeSteps_", "_numRuns_", "_rndSeed_" }
#define REG_SIG_NUM 6
#define REG_SIG_CODE { SIGINT, SIGTERM, SIGABRT, SIGFPE, SIGILL, SIGSEGV }
#define REG_SIG_NAME { "Interrupt signal", "Terminate signal", "Abort signal", \
					   "Floating-point exception", "Illegal instruction", \
					   "Segmentation violation" }

// macro functions
#define BROTHER( O ) ( O == NULL ? NULL : O->next )

// classes forward declarations
namespace lsd
{
	class bridge;
	class description;
	class dlliblinkage;
	class netlink;
	class netnode;
	class object;
	class sensitivity;
	class variable;
	class worker;
	struct lattice;
	struct lsdstack;
	struct profile;
}

// special types used for fast equation, object and variable lookup
typedef std::function < double( lsd::object *caller, lsd::variable *var ) > eq_funcT;
typedef std::lock_guard < std::mutex > l_guardT;
typedef std::lock_guard < std::recursive_mutex > rec_lguardT;
typedef std::pair < std::string, lsd::bridge * > b_pairT;
typedef std::pair < double, lsd::object * > o_pairT;
typedef std::pair < long, lsd::object * > n_pairT;
typedef std::pair < std::string, lsd::variable * > v_pairT;
typedef std::vector < double > d_vecT;
typedef std::vector < int > i_vecT;
typedef std::vector < lsd::object * > o_vecT;
typedef std::vector < std::string > s_vecT;
typedef std::unique_lock < std::mutex > uniq_lT;
typedef std::unique_lock < std::recursive_mutex > rec_uniqlT;
typedef std::unordered_map < std::string, eq_funcT > eq_mapT;
typedef std::unordered_map < std::string, lsd::bridge * > b_mapT;
typedef std::unordered_map < double, lsd::object * > o_mapT;
typedef std::unordered_map < long, lsd::object * > n_mapT;
typedef std::unordered_map < std::string, std::string > p_mapT;
typedef std::unordered_map < std::string, lsd::variable * > v_mapT;
typedef std::unordered_set < lsd::object * > o_setT;
typedef pugi::xml_document xml_doc;
typedef pugi::xml_node xml_node;
typedef pugi::xml_attribute xml_attr;


#ifdef _WIN32
typedef HANDLE handleT;
#else
typedef pid_t handleT;
#endif

// LSD library name space
namespace lsd
{
/*************************************************************
 SIMULATION
 *************************************************************/
	class simulation					// simulation container class
	{
		public:

		// public variables used in equations
		bool fast;						// safe copy of fast_mode flag
		bool no_saved = true;			// disable usage of saved values as lagged ones
		bool no_search;					// disable standard variable search mechanism
		bool no_search_up;				// disable object up-search mechanism
		bool no_zero_instance = true;	// flag to allow deleting last object instance
		bool parallel_mode;				// parallel mode (multithreading) status
		bool use_nan;					// flag to allow using Not a Number value
		char *conf_name = NULL;			// name of current simulation configuration
		char *conf_path = NULL;			// folder where the current configuration is
		dlliblinkage *liblnk = NULL;	// call-back references for DLL
		eq_mapT eq_map;					// fast equation look-up map
		int deb_set = false;			// debug enable control (bool)
		int fast_mode;					// level of LOG messages & runtime plot
		int last_run = 1;				// total serial simulation runs
		int last_t = MAX_STEPS;			// number of simulation steps
		int no_ptr_chk = false;			// disable user pointer checking
		int quit = 0;					// simulation interruption mode (0=none)
		int run;						// current serial simulation run
		int sim;						// library simulation object index
		int t;							// current time step
		object *root = NULL;			// LSD root object
		o_setT obj_list;				// set with all existing LSD objects
		std::mutex lock_obj_list;		// lock object list for parallel manipulation
		unsigned seed = 1;				// random number generator initial seed
#ifndef _NW_
		double d_values[ USER_D_VARS ];
		int i_values[ 4 ];
		netlink *n_values[ 10 ];
		object *o_values[ 10 ];
		FILE *f_values[ 1 ];
#endif
#ifndef _FUN_
		// public variables not used in equations
		bool batch_sequential = false;	// no-window multi configuration job running
		bool conf_ok = false;			// a valid configuration file is loaded
		bool error_hard_thread;			// error_hard called in worker thread
		bool grand_total = false;		// produce grand total in batch processing
		bool idle_loop = true;			// main idle loop (no running operation)
		bool message_logged = false;	// new message posted in log window
		bool on_bar;					// indicate bar is being draw in log
		bool parallel_monitor;			// parallel monitor thread status
		bool save_alt = false;			// alternate save path flag
		bool save_ok = true;			// control saving model configuration
		bool user_exception = false;	// indicate exception generated by user code
		bool watch_trigger = false;		// indicate that a watch condition was met
		bool watch_write_mode;			// flag for write-only watch condition
		bool worker_crashed;			// parallel worker crash flag
		bool worker_ready;				// parallel worker ready flag
		char *alt_path = NULL;			// alternative output path
		char *conf_file = NULL;			// name of current configuration file
		char *log_file = NULL;			// name of log file, if any
		char conf_eq_txt[ MAX_FILE_SIZE ] = "";// equations saved in configuration file
		char error_hard_msg1[ MAX_BUFF_SIZE ];// buffer for parallel worker title msg
		char error_hard_msg2[ MAX_BUFF_SIZE ];// buffer for parallel worker log msg
		char error_hard_msg3[ MAX_BUFF_SIZE ];// buffer for parallel worker box msg
		char rep_file[ MAX_PATH_LENGTH ] = "";// documentation report file name
		char res_path[ MAX_PATH_LENGTH ] = "";// path of last used results directory
		char watch_elem[ MAX_ELEM_LENGTH + 1 ] = "";// elem. triggering watch condition
		clock_t start_profile[ MAX_PROF_SIZE ];// profile-level start times
		clock_t end_profile[ MAX_PROF_SIZE ];// profile-level end times
		description *descr = NULL;		// model description structure
		int add_to_tot = false;			// type of totals file generated (bool)
		int deb_t;						// next debug stop time step (0 for none)
		int dobar = false;				// enable progress bar in log/standard output
		int docsv = false;				// produce .csv text results files (bool)
		int dozip = true;				// compressed results file flag (bool)
		int findex;						// current multi configuration job
		int fend;						// last multi configuration job to run
		int last_dispatch_time;			// last time step controlled by dispatcher
		int log_start;					// first period to start logging to file
		int log_stop;					// last period to log to file, if any
		int max_runs;					// maximum number of parallel runs
		int max_threads;				// maximum parallel threads per run
		int no_res = false;				// do not produce .res results files (bool)
		int no_tot = true;				// do not produce .tot totals files (bool)
		int parallel_disable = false;	// flag to control parallel mode
		int prof_aggr_time = false;		// show aggregate profiling times
		int prof_min_msecs = 0;			// profile variables taking more than X msecs.
		int prof_obs_only = false;		// profile only observed variables
		int series_saved = 0;			// number of series saved
		int stack_info = 0;				// LSD stack control
		int stack_level;				// LSD stack call level
		int stale_time;					// time passed from last step computation
		lattice *latt = NULL;			// model lattice
		long nodesSerial = 1;			// network node serial number counter
		lsdstack *stack_log = NULL;		// LSD stack
		object *blueprint = NULL;		// LSD blueprint (effective model in use)
		object *wait_delete = NULL;		// LSD object waiting for deletion
		sensitivity *sens = NULL;		// LSD sensitivity analysis structure
		std::atomic < bool > parallel_ready;// indicate variable worker is ready
		std::atomic < bool > running = false;// single simulation is running
		std::atomic < bool > running_seq = false;// set of sequential simulations running
		std::atomic < int > eff_t = 0;	// number of executed time steps
		std::condition_variable upd_workers;// worker schedule update signal
		std::map < std::string, profile > prof_times;// set of saved profiling times
		std::minstd_rand lc1;			// linear congruential generator (internal)
		std::minstd_rand lc2;			// linear congruential generator (user)
		std::mt19937 mt32;				// Mersenne-Twister 32 bits generator
		std::mt19937_64 mt64;			// Mersenne-Twister 64 bits generator
		std::mutex draw_lc1_lck;		// locks for random generator operations
		std::mutex draw_lc2_lck;
		std::mutex draw_lf24_lck;
		std::mutex draw_lf48_lck;
		std::mutex draw_mt32_lck;
		std::mutex draw_mt64_lck;
		std::mutex draw_rd_lck;
		std::mutex run_logs_lck;		// lock run_logs for parallel updating
		std::mutex run_pids_lck;		// lock run_pids for parallel updating
		std::mutex var_update_lck;		// control worker variable update
		std::mutex wrk_crash_lck;		// control worker crash handling
		std::random_device rd;			// simulation random device
		std::ranlux24 lf24;				// lagged fibonacci 24 bits generator
		std::ranlux48 lf48;				// lagged fibonacci 48 bits generator
		std::string run_log;			// consolidated runs log
		std::vector < handleT > run_pids;// parallel running instances process id's
		std::vector < std::string > res_list;// list of results files last saved
		std::thread run_monitor;		// thread monitoring parallel instances
		std::thread sim_thread;			// thread object where simulation is run
		s_vecT run_logs;				// log file list produced in parallel runs
		variable *cemetery = NULL;		// LSD saved data from deleted objects
		variable *last_cemetery = NULL;	// LSD last saved cemetery entry
		worker *workers = NULL;			// multi-thread parallel worker data
		FILE *log_file_ptr;				// log file pointer, if any
#ifndef _NW_
		p_mapT par_map;					// variable to parent name map for AoR
		Tcl_Interp *inter;				// Tcl interpreter (for legacy LSD code)
#endif
		private:

		bool batch_loop = false;		// batch multi-config batch loop in process
		bool parallel_abort;			// indicate parallel threads were aborted
		char conf_eq_file[ MAX_PATH_LENGTH ] = "";// equation file name in config. file
		int ran_gen_id = 2;				// ID of initial generator (DO NOT CHANGE)
		i_vecT run_status;				// parallel running instances status
		long idum = 0;					// Park-Miller default seed (legacy code)
		std::atomic < int > alaplErrCnt, bernoErrCnt, betaErrCnt, binomErrCnt,
							cauchErrCnt, chisqErrCnt, expErrCnt, fishErrCnt,
							gammaErrCnt, geomErrCnt, lnormErrCnt, normErrCnt,
							paretErrCnt, poissErrCnt, studErrCnt, weibErrCnt;
										// math error count control
		std::mutex error_lck;			// control multiple error_hard calls
		std::mutex run_status_lck;		// lock run_status for parallel updating
		std::mutex seq_end_lck;			// lock seq_end for parallel updating
		std::vector < std::thread > run_threads;// parallel running instances
		s_vecT run_results;				// parallel run results files
#endif
		public:

		// public methods used in equations
		char *no_node_chr( const char *lab, const char *file, int line );
		double alapl( double mu, double alpha1, double alpha2 );// asym. laplace draw
		double alaplcdf( double mu, double alpha1, double alpha2, double x );// asym. laplace cdf
		double bernoulli( double p );	// Bernoulli draw
		double beta( double alpha, double beta );// beta draw
		double betacdf( double alpha, double beta, double x );// beta cdf
		double binomial( double p, double t );// binomial draw
		double bpareto( double alpha, double low, double high );// bounded pareto draw
		double bparetocdf( double alpha, double low, double high, double x );
		double build_obj_list( bool set_list );// build object list for pointer checking
		double cauchy( double a, double b );// Cauchy draw
		double chi_squared( double n );		// chi-squared draw
		double exponential( double lambda );// exponential draw
		double fact( double x );		// Factorial function
		double fisher( double m, double n );// Fisher-F draw
		double gamma( double alpha, double beta = 1 );// gamma draw
		double geometric( double p );	// geometric draw
		double init_lattice( int init_color = -0xffffff, double nrow = 100, double ncol = 100, double pixW = 0, double pixH = 0 );
		double init_lattice( double pixW = 0, double pixH = 0, double nrow = 100, double ncol = 100, const char lrow[ ] = "y", const char lcol[ ] = "x", const char lvar[ ] = "", object *p = NULL, int init_color = -0xffffff );
		double ipow( double base, double exp );// integer exponentiation
		double lnorm( double mu, double sigma );// lognormal draw
		double lnormcdf( double mu, double sigma, double x );// lognormal cdf
		double norm( double mean, double dev );// normal draw
		double normcdf( double mu, double sigma, double x );// normal cdf
		double pareto( double mu, double alpha );// Pareto draw
		double paretocdf( double mu, double alpha, double x );
		double poisson( double m );		// Poisson draw
		double poissoncdf( double lambda, double k );// poisson cdf
		double ran1( long *unused = 0 );// 0-1 uniform draw
		double read_lattice( double line, double col );
		double round_digits( double value, int digits );
		double save_lattice( const char fname[ ] = "lattice" );
		double student( double n );		// Student-T draw
		double t_star( int df, double cl );// Student-t distribution statistic
		double uniform( double min, double max );// uniform draw
		double uniform_int( double min, double max );// uniform integer draw
		double unifcdf( double a, double b, double x );// uniform cdf
		double update_lattice( double line, double col, double val = 1 );
		double weibull( double a, double b );// Weibull draw
		double z_star( double cl );		// Standard normal distribution statistic
		inline bool chk_hook( object *ptr, unsigned num );
		inline bool chk_obj( object *ptr );
		inline bool chk_ptr( object *ptr );
		inline char *bad_ptr_chr( object *ptr, const char *file, int line );
		inline double bad_ptr_dbl( object *ptr, const char *file, int line );
		inline double no_node_dbl( const char *lab, const char *file, int line );
		inline double nul_lnk_dbl( const char *file, int line );
		inline eq_funcT chk_eq( const char *lab );
		inline object *cycle_obj( object *parent, const char *label, const char *command );
		inline netlink *bad_ptr_lnk( object *ptr, const char *file, int line );
		inline object *bad_ptr_obj( object *ptr, const char *file, int line );
		inline object *no_hook_obj( object *ptr, unsigned num, const char *file, int line );
		inline object *nul_lnk_obj( const char *file, int line );
		inline void bad_ptr_void( object *ptr, const char *file, int line );
		inline void nul_lnk_void( const char *file, int line );
		void close_lattice( void );
		void close_sim( void );
		void error_hard( const char *boxTitle, const char *boxText, bool defQuit, const char *logFmt, ... );
		void init_map( void );
		void init_random( unsigned seed );// reset the random number generator seed
		void plog( const char *msg, ... );// write on log window or terminal
		void set_fast( int level );		// enable fast mode
		void *set_random( int gen );	// set random generator engine

#ifdef USER_FUNCS
		USER_FUNCS
#endif
#ifndef _FUN_
		// public methods not used in equations
		bool results_alt_path( const char *altPath );
		bool save_txt_configuration( const char *path, const char *rname, const char *ext, const char eq_file[ ], const char eq_txt[ ] = "" );
		bool save_xml_configuration( int findex = 0, const char *dest_path = NULL, bool quick = false, const char mod_nam[ ] = "", const char mod_ver[ ] = "", const char mod_dat[ ] = "", const char eq_file[ ] = "", const char eq_txt[ ] = "" );
		bool stop_parallel( void );
		description *add_description( const char *lab, int type = 4, const char *text = NULL, const char *init = NULL, bool initial = false, bool observe = false );
		description *change_description( const char *lab_old, const char *lab = NULL, int type = -1, const char *text = NULL, const char *init = NULL, int initial = -1, int observe = -1 );
		description *search_description( const char *lab, bool add_missing = true );
		double median( d_vecT & v );
		int hyper_count( const char *lab );
		int hyper_count_var( const char *lab );
		int init_new_run( clock_t & start, clock_t & last_update );
		int load_configuration( bool reload, std::string *warnings, int quick );
		int rnd_int( int min, int max );
		int run_parallel( bool nw, const char *exec, const char *simname, int fseed, int runs, int thrrun, int parruns );
		int run_simulation( int until_t = 0, int until_run = 0 );
		int worker_errors( void );
		void detach_parallel( void );
		void empty_sensitivity( sensitivity *cs = NULL );
		void empty_stack( void );
		void move_obj( const char *lab, const char *dest );
		void parallel_update( variable *v, object* p, object *caller = NULL );
		void plog_tag( const char *cm, const char *tag, ... );
		void reset_blueprint( object *r );
		void unload_configuration( bool full );

		simulation( void );				// constructor
		~simulation( void );			// destructor

		private:

		bool load_txt_description( const char *msg, FILE *f );
		bool next_batch( void );
		double betacf( double a, double b, double x );
		int init_new_seq( char *bar_done, int & perc_done, int & last_done );
		int load_txt_configuration( bool reload, int quick );
		int monitor_logs( void );
		template < class distr > double draw_gen( distr &d );
		void empty_blueprint( void );
		void empty_cemetery( void );
		void empty_description( void );
		void empty_lattice( void );
		void init_math_error( void );
		void log_parallel( bool nw );
		void monitor_parallel( bool nw );
		void plog_terminal( const char *cm, va_list arg );
		void run_parallel_exec( bool nw, int id, std::string cmd );
		void save_results( void );
		void update_bar( char *bar, int done, int & last_done, int bar_sz );
		void warn_distr( std::atomic < int > & errCnt, bool & stopErr, const char *distr, const char *msg );

#ifdef SIMULATION_EXT
		SIMULATION_EXT
#endif
#endif
	};


/*************************************************************
 OBJECT
 *************************************************************/
	class object						// simulation model object class
	{
		public:

		// public variables used in equations
		char *label;
		netnode *node = NULL;			// pointer to network node data structure
		object *hook = NULL;
		object *next = NULL;
		object *up;						// parent object
		o_vecT hooks;
		void *cext = NULL;				// pointer to C++ object extension

#ifndef _FUN_
		// public variables not used in equations
		bool to_compute;
		b_mapT b_map;					// fast lookup map to object bridges
		bridge *b = NULL;
		int acounter = 0;				// "fail safe" when creating labels
		simulation *sim;				// simulation where object is contained
		variable *v = NULL;
#endif
		private:

		bool *del_flag = NULL;			// address of flag to signal deletion
		bool deleting = false;			// indicate deletion in process
		int lst_cnt_upd = 0;			// period of last counter update
		std::mutex obj_comp_lck;		// mutex lock for parallel computations
		v_mapT v_map;					// fast lookup map to variables

		public:

		// public methods used in equations
		bool under_comput_var( const char *lab );
		double av( const char *lab1, int lag = 0, bool cond = false, const char *lab2 = "", const char *lop = "", double value = NAN );
		double cal( object *caller, const char *l, int lag = 0 );
		double cal( object *caller, const char *l, int lag, bool force_search );
		double count( const char *lab1, int lag = 0, bool cond = false, const char *lab2 = "", const char *lop = "", double value = NAN );
		double count_all( const char *lab1, int lag = 0, bool cond = false, const char *lab2 = "", const char *lop = "", double value = NAN );
		double increment( const char *lab, double value );
		double initturbo( const char *lab );
		double initturbo( const char *lab, double tot );
		double initturbo_cond( const char *label );
		double init_stub_net( const char *lab, const char gen[ ] = "DISCONNECTED", long numNodes = 0, long par1 = 0, double par2 = 0.0 );
		double interact( const char *text, double v, double *tv, int i, int j, int h, int k,
			object *cur, object *cur1, object *cur2, object *cur3, object *cur4, object *cur5,
			object *cur6, object *cur7, object *cur8, object *cur9, netlink *curl, netlink *curl1,
			netlink *curl2, netlink *curl3, netlink *curl4, netlink *curl5, netlink *curl6,
			netlink *curl7, netlink *curl8, netlink *curl9 );
		double last_cal( const char *lab );
		double mav( object *caller, const char *lab, double per, int lag = 0 );
		double mav( object *caller, const char *lab, double per, const double weight[ ] = NULL, int lag = 0 );
		double med( const char *lab1, int lag = 0, bool cond = false, const char *lab2 = "", const char *lop = "", double value = NAN );
		double multiply( const char *lab, double value );
		double overall_max( const char *lab1, int lag = 0, bool cond = false, const char *lab2 = "", const char *lop = "", double value = NAN );
		double overall_min( const char *lab1, int lag = 0, bool cond = false, const char *lab2 = "", const char *lop = "", double value = NAN );
		double perc( const char *lab1, double p, int lag = 0, bool cond = false, const char *lab2 = "", const char *lop = "", double value = NAN );
		double recal( const char *l );
		double read_file_net( const char *lab, const char *dir = "", const char *base_name = "net", int serial = 1, const char *ext = "net" );
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
		netlink *add_link_net( object *destPtr, double weight = 0, double probTo = 1 );
		netlink *add_link_net( const char *nodeName, long startNode, long endNode, double weight = 0, double probTo = 1, bool edge = false );
		netlink *draw_link_net( void );
		netlink *search_link_net( long id );
		object *add_node_net( long id = -1, const char *nodeName = "", bool silent = false );
		object *add_n_objects2( const char *lab, int n, int t_update = -1 );
		object *add_n_objects2( const char *lab, int n, object *ex, int t_update = -1 );
		object *draw_node_net( const char *lab );
		object *draw_rnd( const char *lo );
		object *draw_rnd( const char *lo, const char *lv, int lag = 0 );
		object *draw_rnd( const char *lo, const char *lv, int lag, double tot );
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
		variable *search_var( object *caller, const char *label, bool no_error = false, bool no_search = false, bool no_search_up = false, bool search_sons = false );
		void delete_link_net( netlink *ptr );
		void delete_net( const char *lab );
		void delete_node_net( void );
		void delete_obj( variable *caller = NULL );
		void name_node_net( const char *nodeName );
		void update( bool recurse, bool user );

#ifndef _FUN_
		// public methods not used in equations
		bool alloc_save_mem( void );
		bool load_txt_insts( const char *file_name, FILE *f );
		bool load_txt_struct( FILE *f );
		bool search_parallel( void );
		bridge *search_bridge( const char *lab, bool no_error = false );
		double cal( const char *l, int lag = 0 );
		int load_xml_insts( xml_node &n, n_mapT &node_map, std::set < int > &warning );
		int load_xml_struct( xml_node &n, bool quick );
		object *add_obj( const char *label, int num = 1, bool propagate = false );
		object *hyper_next( void );
		object *hyper_next( const char *lab );
		object *next_count( object *obj, int *count );
		variable *add_empty_var( const char *str );
		variable *add_var_from_example( variable *example );
		void chg_lab( const char *lab );
		void chg_var_lab( const char *old, const char *n );
		void collect_cemetery( variable *caller = NULL );
		void collect_inst( o_setT &list );
		void copy_descendant( object *to );
		void delete_var( const char *lab );
		void empty( void );
		void init( object *_up, simulation *_sim, const char *_label, bool _to_compute = true );
		void reset_description( void );
		void reset_end( void );
		void save_txt_description( FILE *f );
		void save_txt_insts( FILE *f );
		void save_txt_struct( FILE *f, const char *tab );
		void save_xml_struct( xml_node &pn, long &node_serial, bool quick );
		void set_blueprint( object *container );
		void set_tit_counter( void );

		private:

		// object-class methods
		bool check_cond( double val1, int lopc, double val2 );
		bool under_computation( void );
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
		object *check_net_struct( const char *nodeLab, bool noErr = false );
		object *next_obj( object *obj );
		variable *search_var_err( object *caller, const char *label, bool no_search, bool no_search_up, bool search_sons, const char *errmsg );
		void delete_bridge( void );
		void recreate_maps( void );
		void replicate( int num, bool propagate = false );
		void search_inst( object *obj, long *pos, long *checked );
		FILE *search_txt_data( const char *name, const char *init, const char *str );

#ifdef OBJECT_EXT
		OBJECT_EXT
#endif
#endif
	};


/*************************************************************
 BRIDGE
 *************************************************************/
	class bridge						// descendant-object container class
	{
		public:

		bool copy = false;				// just a temporary copy
		bool counter_updated = false;
		bridge *next = NULL;
		char *label;					// bridge label (same as parent)
		char *search_var = NULL;		// current initialized search variable
		n_mapT t_map;					// turbosearch map
		object *head = NULL;
		o_mapT o_map;					// fast lookup map to object values

		bridge( const char *lab );		// constructor
		bridge( const bridge &b );		// copy constructor
		~bridge( void );				// destructor
	};


/*************************************************************
 VARIABLE
 *************************************************************/
	class variable						// model numeric element (variable,
	{									// parameter, or function) class
		public:

		// public variables used in equations
		bool dummy = false;
		bool integer = false;			// variable must be rounded to integer
		char *label = NULL;
		int param = 0;
		double max_val = NAN;			// maximum limit for variable
		double min_val = NAN;			// minimum limit (NAN = no limit)
		double *val = NULL;
		object *up = NULL;

#ifndef _FUN_
		// public variables not used in equations
		bool initialized = false;
		bool observe = false;
		bool parallel = false;
		bool plot = false;
		bool save = false;
		bool savei = false;
		bool under_computation = false;
		char deb_mode = 'n';
		char *lab_tit = NULL;
		double deb_cnd_val = 0;
		double *data = NULL;
		int deb_cond = 0;
		int delay = 0;
		int delay_range = 0;
		int end = 0;
		int last_update = 0;
		int next_update = 0;
		int num_lag = 0;
		int period = 1;
		int period_range = 0;
		int start = 0;
		std::recursive_mutex var_comp_lck;// mutex lock for parallel computation
		variable *next = NULL;
#endif
		private:

		eq_funcT eq_func = NULL;		// pointer to equation function
		simulation *sim = NULL;			// simulation where object is contained

		public:

		// public methods used in equations
		double fun( object *caller );
		inline double chk_dummy( const char *lab );
		inline double chk_res( double res );

#ifndef _FUN_
		// public methods not used in equations
		bool alloc_save_var( void );
		double cal( object *caller, int lag );
		double chk_val( double val );
		void add_cemetery( void );
		void empty( bool no_lock = false );
		void init( object *_up, simulation *_sim, const char *_label, int _param = -1,
				   int _num_lag = -1, double *_val = NULL );
		void save_single( void );
		void set_lab_tit( void );

		variable( void ) { };			// constructor (empty)
		variable( const variable &v );	// copy constructor
		~variable( void );				// destructor

#ifdef VARIABLE_EXT
		VARIABLE_EXT
#endif
#endif
	};


/*************************************************************
 NETNODE
 *************************************************************/
	class netnode						// network node data class
	{
		public:

		// public variables used in equations
		char *name = NULL;				// node textual name (not required)
		double prob;					// assigned node draw probability
		long id;						// node unique ID number (reorderable)
		long nlinks = 0;				// number of arcs FROM node
		netlink *first = NULL;			// first link in the linked list of links

#ifndef _FUN_
		// public variables not used in equations
		int time;						// time of creation/update
		long serial;					// node serial number (for file save/export)
		netlink *last = NULL;			// last link in the linked list of links

		netnode( object *_up, long nodeId = -1, const char nodeName[ ] = "",
				 double nodeProb = 1 );	// constructor
		~netnode( void );				// destructor
#endif
		private:

		object *up;						// object containing node
	};


/*************************************************************
 NETLINK
 *************************************************************/
	class netlink						// individual outgoing network link class
	{
		public:
		// public variables used in equations
		double probTo;					// destination node draw probability
		double weight;					// link weight
		netlink *next = NULL;			// pointer to next link (NULL if last )
		object *from;					// network node containing the link
		object *to;						// pointer to destination number

#ifndef _FUN_
		// public variables not used in equations
		int time;						// time of creation/update
		netlink *prev;					// pointer to previous link (NULL if first )

		netlink( object *origNode, object *destNode, double linkWeight = 0, double destProb = 1 );
										// constructor
		~netlink( void );				// destructor
#endif
	};


/*************************************************************
 DLLIBLINKAGE
 *************************************************************/
	class dlliblinkage					// callback references for dynamic link library
	{
		public:

		// public methods used in equations
		void ( *deb_log ) ( bool on, int time ) = NULL;

#ifndef _FUN_
		// public variable and methods not used in equations
		int *choice;

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
#endif
	};


#ifndef _FUN_
/*************************************************************
 SENSITIVITY
 *************************************************************/
	class sensitivity					// sensitivity analysis container class
	{
		public:

		bool integer;					// variable must be rounded to integer
		char *label = NULL;				// variable name
		double *val = NULL;				// values to test sensitivity
		int lag;						// lag of initial value
		int cur_val = 0;				// index for value in use in combinations
		int num_val = 0;				// number of values to test
		int param;						// element type
		sensitivity *next = NULL;		// sensitivity analysis chain of elements

		sensitivity( const char *lab, simulation *_sim, int _param, int _lag,
					 bool _integer, int _num_val = 0, d_vecT *_val = NULL );
										// constructor
		~sensitivity( void );			// destructor

		private:

		simulation *sim;				// simulation where object is contained

#ifdef SENSITIVITY_EXT
		SENSITIVITY_EXT
#endif
	};


/*************************************************************
 WORKER
 *************************************************************/
	class worker						// multi-thread variable worker data
	{
		public:

		bool errored;
		bool free = false;
		simulation *sim = NULL;			// simulation where object is contained
		std::thread worker_thread;
		void cal_worker( void );		// worker thread code

		~worker( void );				// destructor

		bool check( void );				// handle worker problems
		void cal( variable *_v );		// start worker calculation

		private:

		bool running = false;
		bool user_excpt;
		char err_msg1[ MAX_BUFF_SIZE ] = "";
		char err_msg2[ MAX_BUFF_SIZE ] = "";
		char err_msg3[ MAX_BUFF_SIZE ] = "";
		int signum = -1;
		jmp_buf env;
		std::condition_variable run;
		std::exception_ptr pexcpt = nullptr;
		std::mutex worker_lck;
		std::thread::id thread_id;
		variable *v = NULL;

		static void signal_wrapper( int signun );// wrapper for signal_handler
		void signal( int signum );		// signal handler
	};


/*************************************************************
 DESCRIPTION
 *************************************************************/
	class description					// model-element description class
	{
		public:

		bool initial = false;
		bool observe = false;
		char *init = NULL;
		char *label;
		char *text;
		char *type;
		description *next = NULL;

		description( const char *_label, int _type, const char *_text,
					 const char *_init, bool _initial, bool _observe );// constructor
		~description( void );

		bool has_descr_text( void );
	};


/*************************************************************
 RESULT
 *************************************************************/
	class result						// results file container class
	{
		public:

		void title( object *root, int flag );// write file header
		void data( object *root, int initstep, int endtstep = 0 );	// write data

		result( const char *fname, const char *fmode, simulation *_sim,
				bool _dozip = false, bool _docsv = false );// constructor
		~result( void );				// destructor

		private:

		bool docsv;						// comma separated .csv text format
		bool dozip;						// compressed file flag
		bool firstCol;					// flag for first column in line
		gzFile fz = NULL;				// compressed file pointer
		simulation *sim;				// simulation where object is contained
		FILE *f = NULL;					// uncompressed file pointer

		void data_recursive( object *r, int i );	// save a single time step (recursively)
		void title_recursive( object *r, int i );	// write file header (recursively)
	};


/*************************************************************
 LATTICE
 *************************************************************/
	struct lattice						// model (visual) lattice data structure
	{
		double height = 0;
		double width = 0;				// lattice screen size
		int columns = 0;
		int errors = 0;					// error counter
		int rows = 0;					// lattice size
		int **array = NULL;				// lattice data colors array
	};


/*************************************************************
 LSDSTACK
 *************************************************************/
	struct lsdstack						// simulation-stack element structure
	{
		char label[ MAX_ELEM_LENGTH ] = "";
		int n = 0;
		lsdstack *next = NULL;
		lsdstack *prev = NULL;
		variable *v = NULL;
	};


/*************************************************************
 PROFILE
 *************************************************************/
	struct profile						// profiled variable structure
	{
		unsigned int comp = 0;
		unsigned long long ticks = 0;
	};
#endif


/*************************************************************
 GLOBAL VARIABLES
 *************************************************************/
	// variables used in equations
	extern const bool no_pointer_check;	// user pointer checking static disable
	extern const bool no_pointer_init;	// user pointer initialization disable

#ifndef _FUN_
	// variables not used in equations
	extern char *exec_file;				// name of executable file
	extern char *exec_path;				// path of executable file
	extern char *lib_file;				// name of shared library, if any
	extern char *lib_path;				// path of shared library, if any
	extern char *model_path;			// folder where the model files are
	extern char *root_lsd;				// path of LSD root directory
	extern const char nonavail[ ];		// string for unavailable values
	extern const char *desc_key_words[ ];// library constant string arrays
	extern const char *desc_type_names[ ];
	extern const char *elem_type_names[ ];
	extern const char *meta_par_names[ ];
	extern const char *signal_names[ ];
	extern const double t_dist_cl[ T_CLEVS ];// t-distribution table confidence
	extern const double t_dist_st[ T_CLEVS ][ 36 ];// t-distribution table statistics
	extern const double z_dist_cl[ Z_CLEVS ];// normal distribution table confidence
	extern const double z_dist_st[ Z_CLEVS ];// normal distribution table statistics
	extern const int signals[ ];		// handled system signal numbers
	extern const std::unordered_map < std::string, int > logic_ops_map;// cond. ops.
	extern std::condition_variable seq_end;	// signal simulation sequence end
	extern std::map < std::thread::id, worker * > worker_thread_ptr;// worker thr ptr
	extern std::mutex plog_term_lck;	// lock plog_terminal for parallel upd.
	extern std::mutex wrk_thr_ptr_lck;	// lock worker_thread_ptr for par. upd.
	extern std::vector < simulation * > sims;// vector holding existing simulations
	extern std::thread::id main_thread;	// LSD main thread ID
	extern FILE *stderr_ptr;			// main thread standard error pointer
	extern FILE *stdout_ptr;			// main thread standard output pointer
#endif


/*************************************************************
 GLOBAL FUNCTIONS
 *************************************************************/
	// functions used in equations
	void msleep( unsigned msec = 1000 );// sleep process for milliseconds

#ifndef _FUN_
	// functions not used in equations
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
	double strtod( const char *in, char** endptr, double inv );
	d_vecT strtodsplit( const char *in, char sep, double inv = 0. );
	int dispatch_runs( int until_t = 0, int until_run = 0 );
	int kill_system( simulation *sim, int id );
	int run_system( const char *cmd, simulation *sim = NULL, int id = -1 );
	int strcln( char *out, const char *str, int outSz );
	int strlf( char *out, const char *str, int outSz );
	int strtrim( char *out, const char *str, int outSz );
	int strtrimin( char *out, const char *str, int outSz );
	int strwrap( char *out, const char *str, int outSz, int wid );
	int strwrds( const char *s );
	long strtol( const char *in, char** endptr, int base, long inv );
	std::string to_string( const char *fmt, double val );
	std::vector < long > strtolsplit( const char *in, char sep, long inv = 0 );
	s_vecT strtostrsplit( const char *in, char sep, bool remQuotes = false );
	void cmd( const char *cm, ... );
	void exception_handler( int signum, const char *what );
	void handle_signals( void ( * handler ) ( int signum ) );
	void init_map( void );
	void lsd_exit( int v );
	void set_exec( const char *path, const char *file );
	void signal_handler( int signum );
#endif
}

void close_sim( void );				// legacy user equation closure
