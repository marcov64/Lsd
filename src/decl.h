/*************************************************************

	LSD 7.2 - December 2019
	written by Marco Valente, Universita' dell'Aquila
	and by Marcelo Pereira, University of Campinas

	Copyright Marco Valente and Marcelo Pereira
	LSD is distributed under the GNU General Public License
	
 *************************************************************/
 
/*************************************************************
DECL.H
Global definitions among all LSD C++ modules
*************************************************************/

// LSD compilation options file
#include "choose.h"

// check compiler C++ standard support
#ifndef CPP_DEFAULT
#if __cplusplus >= 201103L 
#define CPP11
#endif
#endif

// standard libraries used
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>
#include <signal.h>
#include <csetjmp>
#include <sys/stat.h>
#include <algorithm>
#include <string>
#include <vector>
#include <list>
#include <new>
#include <map>
#include <set>

#ifdef CPP11
// comment the next line to disable parallel mode (multi-threading)
#define PARALLEL_MODE

// multithreading libraries for C++11
#include <atomic>
#include <thread>
#include <mutex>
#include <exception>
#include <condition_variable>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#endif

// comment the next line to compile without libz
#ifndef CPP_DEFAULT
#define LIBZ 							
#ifdef LIBZ
#include <zlib.h>
#endif
#endif

// Tcl/Tk for graphical version (not no-window version)
#ifndef NO_WINDOW
#include <tk.h>
#endif

// disable code that make gdb debugging harder
#ifdef DEBUG_MODE
#define NO_ERROR_TRAP
#endif

// LSD version strings, for About... boxes and code testing
#define _LSD_MAJOR_ 7
#define _LSD_MINOR_ 2
#define _LSD_VERSION_ "7.2-2"
#define _LSD_DATE_ "February 10 2020"   // __DATE__

// global constants
#define TCL_BUFF_STR 3000				// standard Tcl buffer size (>1000)
#define MAX_PATH_LENGTH 500				// maximum path length (>499)
#define MAX_ELEM_LENGTH 100				// maximum element ( object, variable ) name length (>99)
#define MAX_FILE_SIZE 1000000			// max number of bytes to read from files
#define MAX_FILE_TRY 100000				// max number of lines to read from files
#define MAX_LINE_SIZE 1000				// max size of a text line to read from files (>999)
#define DEF_CONF_FILE "Sim1"			// default new configuration name
#define NOLH_DEF_FILE "NOLH.csv"		// default NOLH file name
#define MAX_SENS_POINTS 999				// default warning threshold for sensitivity analysis
#define MAX_COLS 100					// max numbers of columns in init. editor
#define MAX_PLOTS 1000					// max numbers of plots in analysis
#define MAX_CORES 0						// maximum number of cores to use (0=auto )
#define MAX_WAIT_TIME 10				// maximum wait time for a variable computation ( sec.)
#define MAX_TIMEOUT 100					// maximum timeout for multi-thread scheduler (millisec.)
#define MAX_LEVEL 10					// maximum number of object levels (plotting only)
#define ERR_LIM 5						// maximum number of repeated error messages
#define MARG 0.01						// y-axis % plot clearance margin
#define MARG_CONST 0.1					// y-axis % plot clearance margin for constant series
#define BAR_DONE_SIZE 80				// characters in the percentage done bar
#define SIG_DIG 10						// number of significant digits in data files
#define SIG_MIN 1e-100					// Minimum significant value (different than zero)
#define CSV_SEP ","						// single char string with the .csv format separator
#define SENS_SEP " ,;|/#\t\n"			// sensitivity data valid separators
#define USER_D_VARS 1000				// number of user double variables
#define UPD_PER 0.2						// update period during simulation run in s
#define SRV_MIN_CORES 12				// minimum number of cores to consider a server
#define SRV_MAX_CORES 64				// maximum number of cores to use in a server

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
#define NAN quiet_NaN()
#endif
#define NaN NAN

// access permissions in Linux/Mac
#ifndef ACCESSPERMS
#define ACCESSPERMS 0777 
#endif

// Choose directory/file separator
#define foldersep( dir ) ( dir[ 0 ] == '\0' ? "" : "/" )

// define the base pseudo random number generator
#ifndef RND
double ran1( long *idum_loc = NULL );
#define RND ( ran1( ) )
#endif

// date/time format
#define DATE_FMT "%d %B, %Y"

// configuration files details
#define LMM_OPTIONS "lmm_options.txt"
#define SYSTEM_OPTIONS "system_options.txt"
#define MODEL_OPTIONS "model_options.txt"
#define GROUP_INFO "groupinfo.txt"
#define MODEL_INFO "modelinfo.txt"
#define DESCRIPTION "description.txt"
#define LMM_OPTIONS_NUM 15
#define MODEL_INFO_NUM 9

using namespace std;

// classes definitions
struct object; 
struct variable;
struct bridge;
struct mnode;
struct netNode;
struct netLink;

// special types used for fast equation, object and variable lookup
typedef pair < string, bridge * > b_pairT;
typedef pair < double, object * > o_pairT;
typedef pair < string, variable * > v_pairT;
typedef vector < object * > o_vecT;
#ifndef CPP11
typedef map < string, bridge * > b_mapT;
typedef map < double, object * > o_mapT;
typedef map < string, variable * > v_mapT;
typedef set < object * > o_setT;
#else
typedef function < double( object *caller, variable *var ) > eq_funcT;
typedef unordered_map < string, eq_funcT > eq_mapT;
typedef unordered_map < string, bridge * > b_mapT;
typedef unordered_map < double, object * > o_mapT;
typedef unordered_map < string, variable * > v_mapT;
typedef unordered_set < object * > o_setT;
#endif

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

#ifdef PARALLEL_MODE
	mutex parallel_comp;				// mutex lock for parallel computations
#endif

	bool load_param( char *file_name, int repl, FILE *f );
	bool load_struct( FILE *f );
	bool under_computation( void );
	bool under_comput_var( char const *lab );
	bridge *search_bridge( char const *lab, bool no_error = false );
	double av( char const *lab, int lag );
	double cal( char const *l, int lag );
	double cal( object *caller, char const *l, int lag );
	double cal( object *caller, char const *l, int lag, bool force_search );
	double count( char const *lab );
	double count_all( char const *lab );
	double increment( char const *lab, double value );
	double initturbo( char const *label, double num );
	double initturbo_cond( char const *label );
	double init_stub_net( char const *lab, const char* gen, long numNodes = 0, long par1 = 0, double par2 = 0.0 );
	double interact( char const *text, double v, double *tv, int i, int j, int h, int k,
		object *cur, object *cur1, object *cur2, object *cur3, object *cur4, object *cur5,
		object *cur6, object *cur7, object *cur8, object *cur9, netLink *curl, netLink *curl1,
		netLink *curl2, netLink *curl3, netLink *curl4, netLink *curl5, netLink *curl6, 
		netLink *curl7, netLink *curl8, netLink *curl9 );
	double last_cal( char const *lab );
	double med( char const *lab, int lag );
	double multiply( char const *lab, double value );
	double overall_max( char const *lab, int lag );
	double overall_min( char const *lab, int lag );
	double perc( char const *lab, int lag, double p );
	double read_file_net( char const *lab, char const *dir = "", char const *base_name = "net", int serial = 1, char const *ext = "net" );
	double recal( char const *l );
	double sd( char const *lab, int lag );
	double search_inst( object *obj = NULL );
	double stat( char const *lab, double *v = NULL );
	double stats_net( char const *lab, double *r );
	double sum( char const *lab, int lag );
	double whg_av( char const *weight, char const *lab, int lag );
	double write( char const *lab, double value, int time, int lag = 0 );
	double write_file_net( char const *lab, char const *dir = "", char const *base_name = "net", int serial = 1, bool append = false );
	int init( object *_up, char const *_label );
	long init_circle_net( char const *lab, long numNodes, long outDeg );
	long init_connect_net( char const *lab, long numNodes );
	long init_discon_net( char const *lab, long numNodes );
	long init_lattice_net( int nRow, int nCol, char const *lab, int eightNeigbr );
	long init_random_dir_net( char const *lab, long numNodes, long numLinks );
	long init_random_undir_net( char const *lab, long numNodes, long numLinks );
	long init_renyi_erdos_net( char const *lab, long numNodes, double linkProb );
	long init_scale_free_net( char const *lab, long numNodes, long outDeg, double expLink );
	long init_small_world_net( char const *lab, long numNodes, long outDeg, double rho );
	long init_star_net( char const *lab, long numNodes );
	long init_uniform_net( char const *lab, long numNodes, long outDeg );
	netLink *add_link_net( object *destPtr, double weight = 0, double probTo = 1 );
	netLink *draw_link_net( void ); 
	netLink *search_link_net( long id ); 
	object *add_n_objects2( char const *lab, int n, int t_update = -1 );
	object *add_n_objects2( char const *lab, int n, object *ex, int t_update = -1 );
	object *add_node_net( long id = -1, char const *nodeName = "", bool silent = false );
	object *draw_node_net( char const *lab ); 
	object *draw_rnd( char const *lo );
	object *draw_rnd( char const *lo, char const *lv, int lag );
	object *draw_rnd( char const *lo, char const *lv, int lag, double tot );
	object *hyper_next( char const *lab );
	object *hyper_next( void );
	object *lat_down( void );
	object *lat_left( void );
	object *lat_right( void );
	object *lat_up( void );
	object *lsdqsort( char const *obj, char const *var, char const *direction );
	object *lsdqsort( char const *obj, char const *var1, char const *var2, char const *direction );
	object *search( char const *lab, bool no_search = false );
	object *search_node_net( char const *lab, long id ); 
	object *search_var_cond( char const *lab, double value, int lag );
	object *shuffle_nodes_net( char const *lab );
	object *turbosearch( char const *label, double tot, double num );
	object *turbosearch_cond( char const *label, double value );
	variable *add_empty_var( char const *str );
	variable *search_var( object *caller, char const *label, bool no_error = false, bool no_search = false, bool search_sons = false );
	variable *search_var_err( object *caller, char const *label, bool no_search, bool search_sons, char const *errmsg );
	void add_obj( char const *label, int num, int propagate );
	void add_var( char const *label, int lag, double *val, int save );
	void add_var_from_example( variable *example );
	void chg_lab( char const *lab );
	void chg_var_lab( char const *old, char const *n );
	void delete_link_net( netLink *ptr );
	void delete_net( char const *lab );
	void delete_node_net( void );
	void delete_obj( void );
	void delete_var( char const *lab );
	void empty( void );
	void emptyturbo( void );			// remove turbo search structure
	void insert_parent_obj( char const *lab );
	void name_node_net( char const *nodeName );
	void recreate_maps( void );
	void replicate( int num, int propagate );
	void save_param( FILE *f );
	void save_struct( FILE *f, char const *tab );
	void search_inst( object *obj, int *pos );
	void sort_asc( object *from, char *l_var );
	void sort_desc( object *from, char *l_var );
	void update( bool recurse, bool user );
};

struct variable
{
	char *label;
	char *lab_tit;
	char data_loaded;
	char debug;
	bool dummy;
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
	
#ifdef PARALLEL_MODE
	mutex parallel_comp;				// mutex lock for parallel computation
#endif

#ifdef CPP11
	eq_funcT eq_func;					// pointer to equation function for fast look-up
#endif

	variable( void );					// empty constructor
	variable( const variable &v );		// copy constructor

	double cal( object *caller, int lag );
	double fun( object *caller );
	int init( object *_up, char const *_label, int _num_lag, double *val, int _save );
	void empty( void );
};

struct bridge
{
	char *blabel;
	bool copy;							// just a temporary copy
	bool counter_updated;
	bridge *next;
	mnode *mn;
	object *head;
	char *search_var;					// current initialized search variable 
	
	o_mapT o_map;						// fast lookup map to objects
	
	bridge( const char *lab );			// constructor
	bridge( const bridge &b );			// copy constructor
	~bridge( void );					// destructor
};

struct mnode
{
	long deflev;						// saves the log number objects to allow defaulting
	mnode *son;
	object *pntr;

	void create( double level );
	void empty( void );
	object *fetch( double *n, double level = 0 );
};

struct netNode							// network node data
{
	char *name;							// node textual name (not required )
	double prob;						// assigned node draw probability
	int time;							// time of creation/update
	long id;							// node unique ID number (reorderable )
	long nLinks;						// number of arcs FROM node
	long serNum;						// node serial number (initial order, fixed )
	netLink *first;						// first link in the linked list of links
	netLink *last;						// last link in the linked list of links
	
	netNode( long nodeId = -1, char const nodeName[ ] = "", double nodeProb = 1 );
										// constructor
	~netNode( void );					// destructor
};

struct netLink							// individual outgoing link
{
	double probTo;						// destination node draw probability
	double weight;						// link weight
	int time;							// time of creation/update
	long serTo;							// destination node serial number (fixed )
	netLink *next;						// pointer to next link (NULL if last )
	netLink *prev;						// pointer to previous link (NULL if first )
	object *ptrFrom;					// network node containing the link
	object *ptrTo;						// pointer to destination number
	
	netLink( object *origNode, object *destNode, double linkWeight = 0, double destProb = 1 ); 
										// constructor
	~netLink( void ); 					// destructor
};

struct lsdstack
{
	char label[ MAX_ELEM_LENGTH ];
	int ns;
	lsdstack *next;
	lsdstack *prev;
	variable *vs;
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
	char initial;
	char observe;
	description *next;
};

struct sense
{
	bool entryOk;						// flag valid data entered
	bool integer;						// integer element
	char *label;
	double *v;
	int i;
	int lag;							// handling lags > 1
	int nvalues;
	int param;							// save element type/lag to allow
	sense *next;
};

struct design 							// design of experiment object
{ 
	int typ, tab, n, k, *par, *lag;		// experiment parameters
	double *hi, *lo, **ptr; 
	char **lab;
	bool *intg;

	design( sense *rsens, int typ = 1, char const *fname = "", int findex = 1, 
			int samples = 0, int factors = 0, int jump = 2, int trajs = 4 );	
										// constructor
	~design( void );					// destructor
};

class result							// results file object
{
	FILE *f;							// uncompressed file pointer
	bool docsv;							// comma separated .csv text format
	bool dozip;							// compressed file flag
	bool firstCol;						// flag for first column in line

#ifdef LIBZ
	gzFile fz;							// compressed file pointer
#endif

	void title_recursive( object *r, int i );	// write file header (recursively)
	void data_recursive( object *r, int i );	// save a single time step (recursively)
	
	public:
	
	result( char const *fname, char const *fmode, bool dozip = false, bool docsv = false );
										// constructor
	~result( void );					// destructor
	
	void data( object *root, int initstep, int endtstep = 0 );	// write data
	void title( object *root, int flag );	// write file header
};

struct profile							// profiled variable object
{
	unsigned int comp;
	unsigned long long ticks;
	
	profile( ) { ticks = 0; comp = 0; };	// constructor
};

#ifdef PARALLEL_MODE
struct worker							// multi-thread parallel worker data structure
{
	bool free;
	bool running;
	bool user_excpt;
	char err_msg1[ TCL_BUFF_STR ];
	char err_msg2[ TCL_BUFF_STR ];
	char err_msg3[ TCL_BUFF_STR ];
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


// standalone C functions/procedures (visible to the users)

bool is_finite( double x );
bool is_inf( double x );
bool is_nan( double x );
double _abs( double a );
double alapl( double mu, double alpha1, double alpha2 );// draw from an asymmetric laplace distribution
double alaplcdf( double mu, double alpha1, double alpha2, double x );	// asymmetric laplace cdf
double bernoulli( double p );							// draw from a Bernoulli distribution
double beta( double alpha, double beta );				// draw from a beta distribution
double betacdf( double alpha, double beta, double x );	// beta cumulative distribution function
double betacf( double a, double b, double x );			// beta distribution function
double build_obj_list( bool set_list );					// build the object list for pointer checking
double fact( double x );								// Factorial function
double gamma( double alpha, double beta = 1 );			// draw from a gamma distribution
double init_lattice( int init_color = -0xffffff, double nrow = 100, double ncol = 100, double pixW = 0, double pixH = 0 );
double lnorm( double mu, double sigma );				// draw from a lognormal distribution
double lnormcdf( double mu, double sigma, double x );	// lognormal cumulative distribution function
double max( double a, double b );
double min( double a, double b );
double norm( double mean, double dev );
double normcdf( double mu, double sigma, double x );	// normal cumulative distribution function
double pareto( double mu, double alpha );
double paretocdf( double mu, double alpha, double x );
double poisson( double m );
double poissoncdf( double lambda, double k );			// poisson cumulative distribution function
double read_lattice( double line, double col );
double round( double r );
double round_digits( double value, int digits );
double save_lattice( const char fname[ ] = "lattice" );
double unifcdf( double a, double b, double x );			// uniform cumulative distribution function
double uniform( double min, double max );
double uniform_int( double min, double max );
double update_lattice( double line, double col, double val = 1 );
void close_lattice( void );
void deb_log( bool on, int time = 0 );					// control debug mode
void error_hard( const char *logText, const char *boxTitle = "", const char *boxText = "", bool defQuit = false );
void init_random( unsigned seed );						// reset the random number generator seed
void msleep( unsigned msec = 1000 );					// sleep process for milliseconds
void plog( char const *msg, char const *tag = "", ... );
void results_alt_path( const char * );  				// change where results are saved.
void set_fast( int level );								// enable fast mode

#ifdef CPP11
double binomial( double p, double t );					// draw from a binomial distribution
double cauchy( double a, double b );					// draw from a Cauchy distribution
double chi_squared( double n );							// draw from a chi-squared distribution
double exponential( double lambda );					// draw from an exponential distribution
double fisher( double m, double n );					// draw from a Fisher-F distribution
double geometric( double p );							// draw from a geometric distribution
double student( double n );								// draw from a Student-T distribution
double weibull( double a, double b );					// draw from a Weibull distribution
#endif


// global variables (visible to the users)
extern bool fast;						// flag to hide LOG messages & runtime (read-only)
extern bool fast_lookup;				// flag for fast equation look-up mode
extern bool no_ptr_chk;					// disable user pointer checking
extern bool no_saved;					// disable the usage of saved values as lagged ones
extern bool no_search;					// disable the standard variable search mechanism
extern bool no_zero_instance;			// flag to allow deleting last object instance
extern bool use_nan;					// flag to allow using Not a Number value
extern char *path;						// folder where the configuration is
extern char *simul_name;				// configuration name being run (for saving networks)
extern double def_res;					// default equation result
extern int cur_sim;
extern int debug_flag;
extern int fast_mode;
extern int max_step;
extern int quit;
extern int ran_gen;						// pseudo-random number generator to use (1-5) )
extern int sim_num;
extern int t;
extern unsigned seed;
extern object *root;

#ifdef CPP11
extern eq_mapT eq_map;					// map to fast equation look-up
#endif

#ifndef NO_WINDOW
extern int i_values[ ];					// user temporary variables copy
extern double d_values[ ];
extern object *o_values[ ];
extern netLink *n_values[ ];
extern Tcl_Interp *inter;				// Tcl standard interpreter pointer
#endif


// prevent exposing internals in users' fun_xxx.cpp
#ifndef FUN

// standalone internal C functions/procedures (not visible to the users)
FILE *create_frames( char *t );
FILE *search_data_ent( char *name, variable *v );
FILE *search_data_str( char const *name, char const *init, char const *str );
FILE *search_str( char const *name, char const *str );
bool alloc_save_mem( object *r );
bool contains( FILE *f, char *lab, int len );
bool discard_change( bool checkSense = true, bool senseOnly = false, const char title[ ] = "" );
bool get_bool( const char *tcl_var, bool *var = NULL );
bool is_equation_header( char *line, char *var, char *updt_in );
bool load_description( char *msg, FILE *f );
bool load_lmm_options( void );
bool load_model_info( void );
bool load_prev_configuration( void );
bool open_configuration( object *&r, bool reload );
bool save_configuration( int findex = 0 );
bool save_sensitivity( FILE *f );
bool search_parallel( object *r );
bool sort_listbox( int box, int order, object *r );
bool unsaved_change( bool );
bool unsaved_change( void );
char *NOLH_valid_tables( int k, char* ch );
char *clean_file( char * );
char *clean_path( char * );
char *str_upr( char * );
char *upload_eqfile( void );
description *search_description( char *lab );
double get_double( const char *tcl_var, double *var = NULL );
double lower_bound( double a, double b, double marg, double marg_eq, int dig = 16 );
double upper_bound( double a, double b, double marg, double marg_eq, int dig = 16 );
int browse( object *r, int *choice );
int check_label( char *l, object *r );
int compute_copyfrom( object *c, int *choice );
int deb( object *r, object *c, char const *lab, double *res, bool interact = false );
int get_int( const char *tcl_var, int *var = NULL );
int load_configuration( bool reload, bool quick = false );
int load_sensitivity( FILE *f );
int lsdmain( int argn, char **argv );
int min_hborder( int *choice, int pdigits, double miny, double maxy );
int my_strcmp( char *a, char *b );
int num_sensitivity_variables( sense *rsens );
int rnd_int( int min, int max );
int shrink_gnufile( void );
long get_long( const char *tcl_var, long *var = NULL );
long num_sensitivity_points( sense *rsens );
object *check_net_struct( object *caller, char const *nodeLab, bool noErr = false );
object *go_brother( object *c );
object *operate( object *r, int *choice );
object *restore_pos( object * );
object *sensitivity_parallel( object *o, sense *s );
object *skip_next_obj( object *t );
object *skip_next_obj( object *t, int *count );
void NOLH_clear( void );
void add_cemetery( variable *v );
void add_description( char const *lab, char const *type, char const *text );
void analysis( int *choice );
void ancestors( object *r, FILE *f, bool html = true );
void assign( object *r, int *i, char *lab );
void attach_instance_number( char *ch, object *r );
void auto_document( int *choice, char const *lab, char const *which, bool append = false );
void autofill_descr( object *o );
void change_descr_lab( char const *lab_old, char const *lab, char const *type, char const *text, char const *init );
void change_descr_lab( char const *lab_old, char const *lab, char const *type, char const *text, char const *init );
void change_descr_text( char *lab );
void change_init_text( char *lab );
void chg_obj_num( object **c, int value, int all, int pippo[ ], int *choice, int cfrom );
void clean_cell( object *root, char *tag, char *lab );
void clean_debug( object *n );
void clean_parallel( object *n );
void clean_plot( object *n );
void clean_save( object *n );
void clean_spaces( char *s );
void close_sim( void );
void cmd( const char *cc, ... );
void collect_cemetery( object *o );
void collect_inst( object *r, o_setT &list );
void control_tocompute(object *r, char *ch);
void copy_descendant( object *from, object *to );
void count( object *r, int *i );
void count_save( object *n, int *count );
void cover_browser( const char *, const char *, const char * );
void create( void );
void create_form( int num, char const *title, char const *prefix, FILE *frep );
void create_initial_values( object *r, FILE *frep );
void create_logwindow( void );
void create_maverag( int *choice );
void create_series( int *choice, bool mc, vector < string > var_names );
void create_table_init( object *r, FILE *frep );
void dataentry_sensitivity( int *choice, sense *s, int nval = 0 );
void deb_show( object *r );
void delete_bridge( object *d );
void draw_buttons( void );
void draw_obj( object *t, object *sel, int level = 0, int center = 0, int from = 0, bool zeroinst = false );
void edit_data( object *root, int *choice, char *obj_name );
void edit_str( object *root, char *tag, int counter, int *i, int res, int *num, int *choice, int *done );
void eliminate_obj( object **r, int actual, int desired , int *choice );
void empty_cemetery( void );
void empty_description( void );
void empty_lattice( void );
void empty_sensitivity( sense *cs );
void entry_new_objnum( object *c, int *choice, char const *tag );
void file_name( char *name );
void fill_list_par( object *r, int flag_all );
void fill_list_var( object *r, int flag_all, int flag_init );
void find_lags( object *r );
void find_using( object *r, variable *v, FILE *frep );
void get_sa_limits( object *r, FILE *out, const char *sep );
void get_saved( object *n, FILE *out, const char *sep, bool all_var = false );
void get_var_descr( char const *lab, char *descr, int descr_len );
void handle_signals( void ( * handler )( int signum ) );
void histograms( int *choice );
void histograms_cs( int *choice );
void init_map( void );
void init_math_error( void );
void init_plot( int i, int id_sim );
void insert_data_file( bool gz, int *num_v, int *num_c, vector < string > *var_names, bool keep_vars );
void insert_data_mem( object *r, int *num_v, int *num_c );
void insert_labels_mem( object *r, int *num_v, int *num_c );
void insert_obj_num( object *root, char const *tag, char const *indent, int counter, int *i, int *value );
void insert_object( const char *w, object *r, bool netOnly = false );
void insert_store_mem( object *r, int *num_v );
void kill_trailing_newline( char *s );
void link_data( object *root, char *lab );
void log_tcl_error( const char *cm, const char *message );
void myexit( int v );
void plog_series( int *choice );
void plot( int type, int *start, int *end, char **str, char **tag, int *choice, bool norm );
void plot( int type, int nv, double **data, int *start, int *end, int *id, char **str, char **tag, int *choice );
void plot_canvas( int type, int nv, int *start, int *end, char **str, char **tag, int *choice );
void plot_cross( int *choice );
void plot_cs_xy( int *choice );
void plot_gnu( int *choice );
void plot_lattice( int *choice );
void plot_phase_diagram( int *choice );
void plot_rt( variable *var );
void plot_tseries( int *choice );
void prepare_plot( object *r, int id_sim );
void print_stack( void );
void put_line( int x1, int y1, int x2 );
void put_node( int x, int y, char *str, bool sel );
void put_text( char *str, char *num, int x, int y, char *str2);
void read_data( int *choice );
void read_eq_filename( char *s );
void report( int *choice, object *r );
void reset_end( object *r );
void reset_plot( int run );
void run( void );
void save_data1( int *choice );
void save_datazip( int *choice );
void save_eqfile( FILE *f );
void save_pos( object * );
void save_single( variable *vcv );
void scan_used_lab( char *lab, int *choice );
void scan_using_lab( char *lab, int *choice );
void search_title( object *root, char *tag, int *i, char *lab, int *incr );
void sensitivity_created( void );
void sensitivity_doe( int *findex, design *doe );
void sensitivity_sequential( int *findexSens, sense *s, double probSampl = 1.0 );
void sensitivity_too_large( void );
void sensitivity_undefined( void );
void set_all( int *choice, object *original, char *lab, int lag );
void set_blueprint( object *container, object *r );
void set_buttons_log( bool on );
void set_cs_data( int *choice );
void set_lab_tit( variable *var );
void set_obj_number( object *r, int *choice );
void set_shortcuts( const char *window, const char *help );
void set_shortcuts_log( const char *window, const char *help );
void set_title( object *c, char *lab, char *tag, int *incr );
void shift_desc( int direction, char *dlab, object *r );
void shift_var( int direction, char *vlab, object *r );
void show_debug( object *n );
void show_eq( char *lab, int *choice );
void show_graph( object *t );
void show_initial( object *n );
void show_neighbors( object *r, bool update );
void show_observe( object *n );
void show_parallel( object *n );
void show_plot( object *n );
void show_plot_gnu( int n, int *choice, int type, char **str, char **tag );
void show_prof_aggr( void );
void show_rep_initial( FILE *f, object *n, int *begin, FILE *frep );
void show_rep_observe( FILE *f, object *n, int *begin, FILE *frep );
void show_save( object *n );
void show_special_updat( object *n );
void show_tmp_vars( object *r, bool update );
void signal_handler( int );
void sort_cs_asc( char **s,char **t, double **v, int nv, int nt, int c );
void sort_cs_desc( char **s,char **t, double **v, int nv, int nt, int c );
void sort_on_end( store *app );
void statistics( int *choice );
void statistics_cross( int *choice );
void tex_report_end( FILE *f );
void tex_report_head( FILE *f, bool table = true );
void tex_report_init( object *r, FILE *f, bool table = true );
void tex_report_initall( object *r, FILE *f, bool table = true );
void tex_report_observe( object *r, FILE *f, bool table = true );
void tex_report_struct( object *r, FILE *f, bool table = true );
void uncover_browser( void );
void unload_configuration ( bool full );
void unwind_stack( void );
void update_bounds( void );
void update_lmm_options( void );
void update_model_info( void );
void wipe_out( object *d );
void write_list( FILE *frep, object *root, int flag_all, char const *prefix );
void write_obj( object *r, FILE *frep );
void write_str( object *r, FILE *frep, int dep, char const *prefix );
void write_var( variable *v, FILE *frep );

#ifdef PARALLEL_MODE
void parallel_update( variable *v, object* p, object *caller = NULL );
#endif

// global internal variables (not visible to the users)
extern FILE *log_file;			// log file, if any
extern bool brCovered;			// browser cover currently covered
extern bool eq_dum;				// current equation is dummy
extern bool error_hard_thread;	// flag to error_hard() called in worker thread
extern bool ignore_eq_file;		// control of configuration files equation updating
extern bool in_edit_data;		// in initial settings mode
extern bool in_set_obj;			// in setting number of objects mode
extern bool iniShowOnce;		// prevent repeating warning on # of columns
extern bool log_ok;				// control for log window available
extern bool message_logged;		// new message posted in log window
extern bool non_var;			// flag to indicate INTERACT macro condition
extern bool on_bar;				// flag to indicate bar is being draw in log window
extern bool parallel_mode;		// parallel mode (multithreading) status
extern bool redrawRoot;			// control for redrawing root window (.)
extern bool running;			// simulation is running
extern bool scrollB;			// scroll check box state in current runtime plot
extern bool struct_loaded;		// a valid configuration file is loaded
extern bool tk_ok;				// control for tk_ready to operate
extern bool unsavedData;		// control for unsaved simulation results
extern bool unsavedSense;		// control for unsaved changes in sensitivity data
extern bool user_exception;		// flag indicating exception was generated by user code
extern bool worker_ready;		// parallel worker ready flag
extern bool worker_crashed;		// parallel worker crash flag
extern char *eq_file;			// equation file content
extern char *exec_file;			// name of executable file
extern char *exec_path;			// path of executable file
extern char *sens_file;			// current sensitivity analysis file
extern char *struct_file;		// name of current configuration file
extern char equation_name[ ];	// equation file name
extern char error_hard_msg1[ ];	// buffer for parallel worker title msg
extern char error_hard_msg2[ ];	// buffer for parallel worker log msg
extern char error_hard_msg3[ ];	// buffer for parallel worker box msg
extern char lastObj[ ];			// last shown object for quick reload
extern char lsd_eq_file[ ];		// equations saved in configuration file
extern char msg[ ];				// auxiliary Tcl buffer
extern char name_rep[ ];		// documentation report file name
extern char nonavail[ ];		// string for unavailable values
extern description *descr;		// model description structure
extern double ymax;				// runtime plot max limit
extern double ymin;				// runtime plot min limit
extern int actual_steps;		// number of executed time steps
extern int add_to_tot;			// type of totals file generated (bool)
extern int choice;				// Tcl menu control variable (main window)
extern int choice_g;			// Tcl menu control variable ( structure window)
extern int cur_plt;				// current graph plot number
extern int docsv;				// produce .csv text results files (bool)
extern int dozip;				// compressed results file flag (bool)
extern int findexSens;			// index to sequential sensitivity configuration filenames
extern int log_start;			// first period to start logging to file, if any
extern int log_stop;			// last period to log to file, if any
extern int macro;				// equations style (macros or C++) (bool)
extern int max_threads;			// suggested maximum number of parallel threads 
extern int no_res;				// do not produce .res results files (bool)
extern int overwConf;			// overwrite current configuration file on run (bool)
extern int parallel_disable;	// flag to control parallel mode
extern int prof_aggr_time;		// show aggregate profiling times
extern int prof_min_msecs;		// profile only variables taking more than X msecs.
extern int prof_obs_only;		// profile only observed variables
extern int saveConf;			// save configuration on results saving (bool)
extern int series_saved;		// number of series saved
extern int stack;				// LSD stack call level
extern int stack_info; 			// LSD stack control
extern int strWindowOn;			// control the presentation of the model structure window (bool)
extern int watch;				// allow for graph generation interruption (bool)
extern int when_debug;      	// next debug stop time step (0 for none )
extern int wr_warn_cnt;			// invalid write operations warning counter
extern long nodesSerial;		// network node serial number global counter
extern lsdstack *stacklog;		// LSD stack
extern map< string, profile > prof;// set of saved profiling times
extern object *blueprint;   	// LSD blueprint (effective model in use )
extern object *currObj;			// pointer to current object in browser
extern object *wait_delete;		// LSD object waiting for deletion
extern o_setT obj_list;			// list with all existing LSD objects
extern sense *rsense;       	// LSD sensitivity analysis structure
extern variable *cemetery;  	// LSD saved data series (from last simulation run )

// constant string arrays
extern const char *lmm_options[ ];
extern const char *lmm_defaults[ ];
extern const char *model_info[ ];
extern const char *model_defaults[ ];

// multi-threading control 
#ifdef PARALLEL_MODE
extern atomic< bool > parallel_ready;	// flag to indicate multitasking is available
extern map< thread::id, worker * > thr_ptr;	// worker thread pointers
extern thread::id main_thread;			// LSD main thread ID
extern worker *workers;					// multi-thread parallel worker data
#endif

// Tcl/Tk specific definitions (for the windowed version only)
#ifndef NO_WINDOW
int Tcl_discard_change( ClientData, Tcl_Interp *, int, const char *[ ] );	// ask before discarding unsaved changes
int Tcl_get_obj_conf( ClientData cdata, Tcl_Interp *inter, int argc, const char *argv[ ] );
int Tcl_set_obj_conf( ClientData cdata, Tcl_Interp *inter, int argc, const char *argv[ ] );
int Tcl_get_var_conf( ClientData cdata, Tcl_Interp *inter, int argc, const char *argv[ ] );
int Tcl_set_var_conf( ClientData cdata, Tcl_Interp *inter, int argc, const char *argv[ ] );
int Tcl_set_c_var( ClientData cdata, Tcl_Interp *inter, int argc, const char *argv[ ] );
int Tcl_get_var_descr( ClientData cdata, Tcl_Interp *inter, int argc, const char *argv[ ] );
int Tcl_upload_series( ClientData cd, Tcl_Interp *inter, int oc, Tcl_Obj *CONST ov[ ] );
#endif									// NO_WINDOW

#endif									// FUN
