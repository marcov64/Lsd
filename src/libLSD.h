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

// standard libraries used
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

// XML library
#define PUGIXML_NO_XPATH
#define PUGIXML_COMPACT
#include "pugixml/pugixml.hpp"

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
#define CSV_SEP ","						// single char string with the .csv format separator
#define ERR_LIM 5						// maximum number of repeated error messages
#define LEGACY_NO_DESCR "(no description available)" // legacy description (do not change)
#define MAX_BUFF_SIZE 10000				// standard Tcl buffer size (>9999)
#define MAX_CORES 0						// maximum number of cores to use (0=auto )
#define MAX_ELEM_LENGTH 100				// maximum element (object, variable) name length (>99)
#define MAX_FILE_SIZE 1000000			// max number of bytes to read from files
#define MAX_FILE_TRY 100000				// max number of lines to read from files
#define MAX_LINE_SIZE 1000				// max size of a text line to read from files (>999)
#define MAX_OBJ_CHK	10000000			// maximum number of objects to check when searching
#define MAX_PATH_LENGTH 1000			// maximum path length (>999)
#define MAX_STEPS 100					// default number of simulation steps
#define MAX_TIMEOUT 100					// maximum timeout for multi-thread scheduler (millisec.)
#define MAX_WAIT_TIME 10				// maximum wait time for a variable computation ( sec.)
#define NOLH_TABS 7						// number of defined NOLH tables
#define NO_CONF_NAME "(no name)"		// no configuration file name yet
#define NO_DESCR ""						// no description available text
#define SIG_DIG 10						// number of significant digits in data files
#define UPD_PER 0.2						// update period during simulation run in s
#define USER_D_VARS 1000				// number of user double variables
#define T_CLEVS 10						// number of defined t distribution confidence levels
#define Z_CLEVS 7						// number of defined normal distr. confidence levels

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

// set default name space (C++ STL)
using namespace std;

// classes pre-definitions
struct object;
struct variable;
struct bridge;
struct netNode;
struct netLink;

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

// classes definitions
struct object
{
	char *label;
	bool deleting;						// indicate deletion in process
	bool to_compute;
	int acounter;
	int lstCntUpd;						// period of last counter update
	bridge *b;
	object *next;
	object *up;
	variable *v;
	object *hook;
	netNode *node;						// pointer to network node data structure
	void *cext;							// pointer to a C++ object extension to the LSD object
	bool *del_flag;						// address of flag to signal deletion

	o_vecT hooks;
	b_mapT b_map;						// fast lookup map to object bridges
	v_mapT v_map;						// fast lookup map to variables

#ifndef _NP_
	mutex parallel_comp;				// mutex lock for parallel computations
#endif

	bool load_insts( const char *file_name, FILE *f );
	bool load_struct( FILE *f );
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
	int load_xml_insts( xml_node &n, n_mapT &node_map, set < int > &warning );
	int load_xml_struct( xml_node &n, bool quick );
	netLink *add_link_net( object *destPtr, double weight = 0, double probTo = 1 );
	netLink *add_link_net( const char *nodeName, long startNode, long endNode, double weight = 0, double probTo = 1, bool edge = false );
	netLink *draw_link_net( void );
	netLink *search_link_net( long id );
	object *add_n_objects2( const char *lab, int n, int t_update = -1 );
	object *add_n_objects2( const char *lab, int n, object *ex, int t_update = -1 );
	object *add_node_net( long id = -1, const char *nodeName = "", bool silent = false );
	object *add_obj( const char *label, int num = 1, bool propagate = false );
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
	void delete_link_net( netLink *ptr );
	void delete_net( const char *lab );
	void delete_node_net( void );
	void delete_obj( variable *caller = NULL );
	void delete_var( const char *lab );
	void empty( void );
	void init( object *_up, const char *_label, bool _to_compute = true );
	void name_node_net( const char *nodeName );
	void recreate_maps( void );
	void replicate( int num, bool propagate = false );
	void save_insts( FILE *f );
	void save_struct( FILE *f, const char *tab );
	void save_xml_struct( xml_node &pn, long &node_serial, bool quick );
	void search_inst( object *obj, long *pos, long *checked );
	void update( bool recurse, bool user );
};

struct variable
{
	char *label;
	char *lab_tit;
	char deb_mode;
	bool dummy;
	bool initialized;
	bool observe;
	bool parallel;
	bool plot;
	bool save;
	bool savei;
	bool under_computation;
	int deb_cond;
	int delay;
	int delay_range;
	int end;
	int last_update;
	int next_update;
	int num_lag;
	int param;
	int period;
	int period_range;
	int start;
	double *data;
	double *val;
	double deb_cnd_val;
	object *up;
	variable *next;

#ifndef _NP_
	recursive_mutex parallel_comp;		// mutex lock for parallel computation
#endif

	eq_funcT eq_func;					// pointer to equation function for fast look-up

	variable( void );					// empty constructor
	variable( const variable &v );		// copy constructor

	double cal( object *caller, int lag );
	double fun( object *caller );
	void empty( bool no_lock = false );
	void init( object *_up, const char *_label, int _param = -1, int _num_lag = -1, double *_val = NULL );
};

struct bridge
{
	bool copy;							// just a temporary copy
	bool counter_updated;
	char *blabel;
	char *search_var;					// current initialized search variable
	bridge *next;
	object *head;
	n_mapT t_map;						// turbosearch map
	o_mapT o_map;						// fast lookup map to object values

	bridge( const char *lab );			// constructor
	bridge( const bridge &b );			// copy constructor
	~bridge( void );					// destructor
};

struct netNode							// network node data
{
	char *name;							// node textual name (not required)
	double prob;						// assigned node draw probability
	int time;							// time of creation/update
	long id;							// node unique ID number (reorderable)
	long nLinks;						// number of arcs FROM node
	long serNum;						// node serial number (for file save/export)
	netLink *first;						// first link in the linked list of links
	netLink *last;						// last link in the linked list of links

	netNode( long nodeId = -1, const char nodeName[ ] = "", double nodeProb = 1 );
										// constructor
	~netNode( void );					// destructor
};

struct netLink							// individual outgoing link
{
	double probTo;						// destination node draw probability
	double weight;						// link weight
	int time;							// time of creation/update
	netLink *next;						// pointer to next link (NULL if last )
	netLink *prev;						// pointer to previous link (NULL if first )
	object *ptrFrom;					// network node containing the link
	object *ptrTo;						// pointer to destination number

	netLink( object *origNode, object *destNode, double linkWeight = 0, double destProb = 1 );
										// constructor
	~netLink( void );					// destructor
};

struct store
{
	char label[ MAX_ELEM_LENGTH ];
	char tag[ MAX_ELEM_LENGTH ];
	double *data;
	int end;
	int rank;
	int start;
};

struct description
{
	char *init;
	char *label;
	char *text;
	char *type;
	bool initial;
	bool observe;
	description *next;
};

struct sense
{
	bool integer;						// integer element
	char *label;
	double *v;							// values to test sensitivity
	int curv;							// index for value in use in combinations
	int lag;							// lag of initial value
	int numv;							// number of values to test
	int param;							// element type
	sense *next;						// sensitivity analysis chain of elements

	sense( const char *lab, int _param, int _lag, int _numv = 0,
		   vector < double > *_v = NULL, bool _integer = false );// constructor
	~sense( void );						// destructor
	int dataentry( void );

};

struct lattice							// model (visual) lattice data
{
	int **array = NULL;					// lattice data colors array
	int rows = 0;						// lattice size
	int columns = 0;
	int errors = 0;						// error counter
	double width = 0;					// lattice screen size
	double height = 0;
};

struct design							// design of experiment object
{
	int typ, tab, n, k, *par, *lag, *inst;// experiment parameters
	double **hi, **lo, ***doe;
	char **lab;
	bool *intg;

	design( sense *rsens, int typ, const char *fname, const char *dest_path,
			int findex, int samples, int factors = 0, int jump = 2, int trajs = 4 );
										// constructor
	~design( void );					// destructor
	void clear_design( void );
	void load_design_data( sense *rsens, int n );
};

struct lsdstack
{
	char label[ MAX_ELEM_LENGTH ];
	int ns;
	lsdstack *next;
	lsdstack *prev;
	variable *vs;
};


class result							// results file object
{
	FILE *f;							// uncompressed file pointer
	bool docsv;							// comma separated .csv text format
	bool dozip;							// compressed file flag
	bool firstCol;						// flag for first column in line
	gzFile fz;							// compressed file pointer

	void title_recursive( object *r, int i );	// write file header (recursively)
	void data_recursive( object *r, int i );	// save a single time step (recursively)

	public:

	result( const char *fname, const char *fmode, bool dozip = false, bool docsv = false );
										// constructor
	~result( void );					// destructor

	void data( object *root, int initstep, int endtstep = 0 );	// write data
	void title( object *root, int flag );	// write file header
};

struct profile							// profiled variable object
{
	unsigned int comp;
	unsigned long long ticks;

	profile( ) { ticks = 0; comp = 0; };// constructor
};

struct nolh								// near-orthogonal Latin hypercube description
{
	int kMin;
	int kMax;
	int n1;
	int n2;
	int loLevel;
	int hiLevel;
	int *table;
};

struct dlliblinkage						// callback references for dynamic link library
{
	void ( *center_plot ) ( void ) = NULL;
	void ( *cmd_backend ) ( const char *cm, va_list arg ) = NULL;
	void ( *cover_browser ) ( const char *text1, const char *text2,
							  bool run ) = NULL;
	void ( *deb_log ) ( bool on, int time ) = NULL;
	void ( *disable_plot ) ( void ) = NULL;
	void ( *enable_plot ) ( void ) = NULL;
	void ( *error_hard_helper ) ( const char *boxTitle, const char *boxText,
								 const char *logText, bool defQuit ) = NULL;
	void ( *init_lattice_helper ) ( double pixW, double pixH, double nrow, double ncol, int init_color ) = NULL;
	void ( *log_tcl_error ) ( bool show, const char *cm,
							 const char *message, ... ) = NULL;
	void ( *plog_backend ) ( const char *cm, const char *tag,
							 va_list arg ) = NULL;
	void ( *plot_rt ) ( variable *var ) = NULL;
	void ( *prepare_plot ) ( object *r, int id_sim ) = NULL;
	void ( *print_stack ) ( void ) = NULL;
	void ( *reset_plot ) ( void ) = NULL;
	void ( *scroll_plot ) ( void ) = NULL;
	void ( *show_prof_aggr ) ( void ) = NULL;
	void ( *uncover_browser ) ( void ) = NULL;
	double ( *save_lattice_helper ) ( const char *fname ) = NULL;
	double ( *update_lattice_helper ) ( double line, double col, double val,
										int line_int, int col_int,
										int val_int ) = NULL;
	int ( * deb ) ( object *r, object *c, const char *lab, double *res,
					bool interact, const char *hl_var ) = NULL;
};

#ifndef _NP_
struct worker							// multi-thread parallel worker data structure
{
	bool free;
	bool running;
	bool errored;
	bool user_excpt;
	char err_msg1[ MAX_BUFF_SIZE ];
	char err_msg2[ MAX_BUFF_SIZE ];
	char err_msg3[ MAX_BUFF_SIZE ];
	condition_variable run;
	exception_ptr pexcpt;
	int signum;
	jmp_buf env;
	mutex lock;
	thread thr;
	thread::id thr_id;
	variable *var;

	worker( void );						// constructor
	~worker( void );					// destructor

	bool check( void );					// handle worker problems
	static void signal_wrapper( int signun );	// wrapper for signal_handler
	void cal( variable *var );			// start worker calculation
	void cal_worker( void );			// worker thread code
	void signal( int signum );			// signal handler
};
#endif

// standalone C functions (visible to the users)
bool is_finite( double x );
bool is_inf( double x );
bool is_nan( double x );
bool results_alt_path( const char * );					// change where results are saved.
double _abs( double a );
double alapl( double mu, double alpha1, double alpha2 );// draw from an asymmetric laplace distribution
double alaplcdf( double mu, double alpha1, double alpha2, double x );	// asymmetric laplace cdf
double bernoulli( double p );							// draw from a Bernoulli distribution
double beta( double alpha, double beta );				// draw from a beta distribution
double betacdf( double alpha, double beta, double x );	// beta cumulative distribution function
double betacf( double a, double b, double x );			// beta distribution function
double binomial( double p, double t );					// draw from a binomial distribution
double bpareto( double alpha, double low, double high );// draw from bounded pareto
double bparetocdf( double alpha, double low, double high, double x );
double build_obj_list( bool set_list );					// build the object list for pointer checking
double cauchy( double a, double b );					// draw from a Cauchy distribution
double chi_squared( double n );							// draw from a chi-squared distribution
double exponential( double lambda );					// draw from an exponential distribution
double fact( double x );								// Factorial function
double fisher( double m, double n );					// draw from a Fisher-F distribution
double gamma( double alpha, double beta = 1 );			// draw from a gamma distribution
double geometric( double p );							// draw from a geometric distribution
double init_lattice( int init_color = -0xffffff, double nrow = 100, double ncol = 100, double pixW = 0, double pixH = 0 );
double ipow( double base, double exp );					// integer exponentiation
double lnorm( double mu, double sigma );				// draw from a lognormal distribution
double lnormcdf( double mu, double sigma, double x );	// lognormal cumulative distribution function
double max( double a, double b );
double median( vector < double > & v );
double min( double a, double b );
double norm( double mean, double dev );
double normcdf( double mu, double sigma, double x );	// normal cumulative distribution function
double pareto( double mu, double alpha );
double paretocdf( double mu, double alpha, double x );
double poisson( double m );
double poissoncdf( double lambda, double k );			// poisson cumulative distribution function
double read_lattice( double line, double col );
double ran1( long *unused = 0 );
double round( double r );
double round_digits( double value, int digits );
double save_lattice( const char fname[ ] = "lattice" );
double student( double n );								// draw from a Student-T distribution
double t_star( int df, double cl );						// Student-t distribution statistic
double z_star( double cl );								// Standard normal distribution statistic
double unifcdf( double a, double b, double x );			// uniform cumulative distribution function
double uniform( double min, double max );
double uniform_int( double min, double max );
double update_lattice( double line, double col, double val = 1 );
double weibull( double a, double b );					// draw from a Weibull distribution
void close_lattice( void );
void deb_log( bool on, int time );						// control debug mode
void error_hard( const char *boxTitle, const char *boxText, bool defQuit, const char *logFmt, ... );
void error_hard_helper( const char *boxTitle, const char *boxText, const char *logText, bool defQuit );
void init_random( unsigned seed );						// reset the random number generator seed
void msleep( unsigned msec = 1000 );					// sleep process for milliseconds
void plog( const char *msg, ... );						// write on log window
void set_fast( int level );								// enable fast mode
void *set_random( int gen );							// set random generator

// global variables (visible to the users)
extern bool fast;						// flag to hide LOG messages & runtime (read-only)
extern bool no_saved;					// disable the usage of saved values as lagged ones
extern bool no_search;					// disable the standard variable search mechanism
extern bool no_search_up;				// disable the object up-search mechanism
extern bool no_zero_instance;			// flag to allow deleting last object instance
extern bool use_nan;					// flag to allow using Not a Number value
extern char *simul_name;				// configuration name being run (for saving networks)
extern const bool fast_lookup;			// flag for fast equation look-up mode
extern const bool no_pointer_check;		// user pointer checking static disable
extern double def_res;					// default equation result
extern eq_mapT eq_map;					// map to fast equation look-up
extern int cur_sim;
extern int debug_flag;
extern int fast_mode;					// execution speed control flag
extern int max_step;
extern int no_ptr_chk;					// dynamic disable user pointer checking
extern int platform;					// OS platform (1=Linux, 2=Mac, 3=Windows)
extern int quit;						// simulation termination control flag
extern int sim_num;
extern int t;
extern unsigned seed;
extern object *root;

#ifndef _NW_
extern int i_values[ ];					// user temporary variables copy
extern double d_values[ ];
extern object *o_values[ ];
extern netLink *n_values[ ];
extern FILE *f_values[ ];
#endif

/// prevent exposing internals in users' fun_xxx.cpp
#ifndef _FUN_

// standalone internal C functions/procedures (not visible to the users)
bool alloc_save_mem( object *r );
bool alloc_save_var( variable *v );
bool check_cond( double val1, int lopc, double val2 );
bool has_descr_text( description *d );
bool load_description( const char *msg, FILE *f );
bool results_alt_path( const char * );
bool search_parallel( object *r );
bool stop_parallel( void );
bool strwsp( const char *str );
bool valid_label( const char *lab );
bool valid_xml_string( const char *lab );
char *clean_file( const char *file );
char *clean_path( const char *path );
char *strcatn( char *d, const char *s, size_t dSz );
char *strcpyn( char *d, const char *s, size_t dSz );
char *strdecdata( char *out, const char *in, int outSz = 0 );
const char *signal_name( int signum );
int hyper_count( const char *lab );
int hyper_count_var( const char *lab );
int kill_system( int id );
int load_configuration( bool reload, std::string *warnings, int quick );
int logic_op_code( const char *lop, const char *errmsg );
int monitor_logs( void );
int rnd_int( int min, int max );
int run( void );
int run_parallel( bool nw, const char *exec, const char *simname, int fseed, int runs, int thrrun, int parruns );
int run_system( const char *cmd, int id = -1 );
int strcln( char *out, const char *str, int outSz );
int strlf( char *out, const char *str, int outSz );
int strtrim( char *out, const char *str, int outSz );
int strtrimin( char *out, const char *str, int outSz );
int worker_errors( void );
description *add_description( const char *lab, int type = 4, const char *text = NULL, const char *init = NULL, bool initial = false, bool observe = false );
description *change_description( const char *lab_old, const char *lab = NULL, int type = -1, const char *text = NULL, const char *init = NULL, int initial = -1, int observe = -1 );
description *search_description( const char *lab, bool add_missing = true );
long strtol( const char *in, char** endptr, int base, long inv );
object *check_net_struct( object *caller, const char *nodeLab, bool noErr = false );
object *go_brother( object *c );
object *skip_next_obj( object *t );
object *skip_next_obj( object *t, int *count );
vector < double > strtodsplit( const char *in, char sep, double inv = 0. );
vector < long > strtolsplit( const char *in, char sep, long inv = 0 );
vector < string > strtostrsplit( const char *in, char sep, bool remQuotes = false );
void add_cemetery( variable *v );
void close_sim( void );
void cmd_gui( const char *cm, ... );
void collect_inst( object *r, o_setT &list );
void copy_descendant( object *from, object *to );
void delete_bridge( object *d );
void detach_parallel( void );
void empty_blueprint( void );
void empty_cemetery( void );
void empty_description( void );
void empty_lattice( void );
void empty_sensitivity( sense *cs = NULL );
void empty_stack( void );
void exception_handler( int signum, const char *what );
void handle_signals( void ( * handler ) ( int signum ) );
void init_map( void );
void init_math_error( void );
void log_parallel( bool nw );
void lsd_exit( int v );
void monitor_parallel( bool nw );
void move_obj( const char *lab, const char *dest );
void plog_tag( const char *cm, const char *tag, ... );
void plog_terminal( const char *cm, va_list arg );
void reset_blueprint( object *r );
void reset_description( object *r );
void reset_end( object *r );
void run_parallel_exec( bool nw, int id, string cmd );
void save_single( variable *v );
void set_blueprint( object *container, object *r );
void set_exec( const char *path, const char *file );
void set_lab_tit( variable *var );
void set_tit_counter( object *o );
void signal_handler( int signum );
void unload_configuration( bool full );
void update_bar( char *bar, int done, int & last_done, int bar_sz );
void warn_distr( int *errCnt, bool *stopErr, const char *distr, const char *msg );
FILE *search_data_str( const char *name, const char *init, const char *str );

#ifndef _NP_
void parallel_update( variable *v, object* p, object *caller = NULL );
#endif

// global internal variables (not visible to the users)
extern bool batch_sequential;// no-window multi configuration job running
extern bool error_hard_thread;	// flag to error_hard() called in worker thread
extern bool grandTotal;			// flag to produce grand total in batch processing
extern bool idle_loop;			// indicates in main idle loop (no running operation)
extern bool iniShowOnce;		// prevent repeating warning on # of columns
extern bool message_logged;		// new message posted in log window
extern bool no_more_memory;		// memory overflow when setting data save structure
extern bool on_bar;				// flag to indicate bar is being draw in log window
extern bool parallel_abort;		// indicate parallel threads were aborted
extern bool parallel_mode;		// parallel mode (multithreading) status
extern bool parallel_monitor;	// parallel monitor thread status
extern bool pause_run;			// pause running simulation
extern bool running;			// simulation is running
extern bool save_alt_path;		// alternate save path flag
extern bool save_ok;			// control if saving model configuration is possible
extern bool scrollB;			// scroll check box state in current runtime plot
extern bool struct_loaded;		// a valid configuration file is loaded
extern bool unsavedData;		// control for unsaved simulation results
extern bool unsavedSense;		// control for unsaved changes in sensitivity data
extern bool user_exception;		// flag indicating exception was generated by user code
extern bool watch_trigger;		// indicate that a watch condition was met
extern bool watch_write_mode;	// flag for write-only watch condition
extern bool worker_ready;		// parallel worker ready flag
extern bool worker_crashed;		// parallel worker crash flag
extern char *alt_path;			// alternative output path
extern char *conf_path;			// folder where the current configuration is
extern char *eq_file;			// equation file content
extern char *exec_file;			// name of executable file
extern char *exec_path;			// path of executable file
extern char *log_filename;		// name of log file, if any
extern char *lib_file;			// name of shared library, if any
extern char *lib_path;			// path of shared library, if any
extern char *model_path;		// folder where the model files are
extern char *rootLsd;			// path of LSD root directory
extern char *sens_file;			// current sensitivity analysis file
extern char *struct_file;		// name of current configuration file
extern char equation_name[ ];	// equation file name
extern char error_hard_msg1[ ];	// buffer for parallel worker title msg
extern char error_hard_msg2[ ];	// buffer for parallel worker log msg
extern char error_hard_msg3[ ];	// buffer for parallel worker box msg
extern char lsd_eq_file[ ];		// equations saved in configuration file
extern char name_rep[ ];		// documentation report file name
extern char nonavail[ ];		// string for unavailable values
extern char path_res[ ];		// path of last used results directory
extern char watch_elem[ ];		// name of element triggering watch condition
extern description *descr;		// model description structure
extern dlliblinkage liblnk;		// call-back references for DLL
extern double t_dist_cl[ T_CLEVS ];// t-distribution table confidence levels
extern double t_dist_st[ T_CLEVS ][ 36 ];// t-distribution table statistics
extern double z_dist_cl[ Z_CLEVS ];// normal distribution table confidence levels
extern double z_dist_st[ Z_CLEVS ];// normal distribution table statistics
extern double ymax;				// runtime plot max limit
extern double ymin;				// runtime plot min limit
extern int NOLH_1[ ][ 7 ];		// near-orthogonal Latin hypercube tables
extern int NOLH_2[ ][ 11 ];
extern int NOLH_3[ ][ 16 ];
extern int NOLH_4[ ][ 22 ];
extern int NOLH_5[ ][ 29 ];
extern int NOLH_6[ ][ 100 ];
extern int actual_steps;		// number of executed time steps
extern int add_to_tot;			// type of totals file generated (bool)
extern int choice;				// Tcl menu control variable (main window)
extern int cur_plt;				// current graph plot number
extern int dobar;				// output a progress bar to the log/standard output
extern int docsv;				// produce .csv text results files (bool)
extern int done_in;				// Tcl menu control variable (log window)
extern int dozip;				// compressed results file flag (bool)
extern int fend;				// last multi configuration job to run
extern int findex;				// current multi configuration job
extern int findexSens;			// index to sequential sensitivity configuration filenames
extern int log_start;			// first period to start logging to file, if any
extern int log_stop;			// last period to log to file, if any
extern int max_runs;			// maximum number of parallel runs
extern int max_threads;			// maximum number of parallel threads per run
extern int no_res;				// do not produce .res results files (bool)
extern int no_tot;				// do not produce .tot totals files (bool)
extern int parallel_disable;	// flag to control parallel mode
extern int prof_aggr_time;		// show aggregate profiling times
extern int prof_min_msecs;		// profile only variables taking more than X msecs.
extern int prof_obs_only;		// profile only observed variables
extern int series_saved;		// number of series saved
extern int stack_level;			// LSD stack call level
extern int stack_info;			// LSD stack control
extern int watch;				// allow for graph generation interruption (bool)
extern int when_debug;			// next debug stop time step (0 for none )
extern int wr_warn_cnt;			// invalid write operations warning counter
extern lattice latt;			// model lattice
extern long nodesSerial;		// network node serial number global counter
extern lsdstack *stack_log;		// LSD stack
extern map< string, profile > prof;// set of saved profiling times
extern mt19937 mt32;			// Mersenne-Twister 32 bits generator
extern nolh NOLH[ NOLH_TABS ];	// characteristics of NOLH tables
extern object *blueprint;		// LSD blueprint (effective model in use )
extern object *wait_delete;		// LSD object waiting for deletion
extern o_setT obj_list;			// list with all existing LSD objects
extern sense *rsense;			// LSD sensitivity analysis structure
extern variable *cemetery;		// LSD saved data from deleted objects
extern variable *last_cemetery;	// LSD last saved data from deleted objects
extern vector < string > res_list;// list of results files last saved
extern void *random_engine;		// current random number generator engine
extern FILE *log_file;			// log file, if any

// common constant string arrays (not visible to the users)
extern const char *desc_key_words[ ];
extern const char *elem_type_names[ ];
extern const char *signal_names[ ];
extern const int signals[ ];			// handled system signal numbers

// multi-threading control
#ifndef _NP_
extern atomic < bool > parallel_ready;// flag to indicate multitasking is available
extern map< thread::id, worker * > thr_ptr;// worker thread pointers
extern mutex lock_obj_list;		// lock for object list for parallel manipulation
extern mutex lock_run_logs;		// lock run_logs for parallel updating
extern mutex lock_run_pids;		// lock run_pids for parallel updating
extern mutex lock_run_status;	// lock run_status for parallel updating
extern string run_log;			// consolidated runs log
extern thread run_monitor;		// thread monitoring parallel instances
extern thread::id main_thread;			// LSD main thread ID
extern vector < int > run_status;// parallel running instances status
extern vector < handleT > run_pids;// parallel running instances process id's
extern vector < string > run_results;// parallel run results files
extern vector < string > run_logs;// list of log files produced in parallel run
extern vector < thread > run_threads;// parallel running instances
extern worker *workers;			// multi-thread parallel worker data
#endif

// Tcl/Tk specific definitions (for the windowed version only)
#ifndef _NW_
#include <tk.h>
extern p_mapT par_map;			// variable to parent name map for AoR
extern Tcl_Interp *inter;		// Tcl interpreter in GUI (for legacy LSD code)
#endif

#endif
