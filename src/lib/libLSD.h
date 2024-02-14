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

#ifndef _NW_
	#include <tk.h>
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

// classes forward declarations
struct bridge;
struct description;
struct lattice;
struct lsdstack;
struct netLink;
struct netNode;
struct object;
struct profile;
struct sensitivity;
struct variable;
struct workerVar;

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
	// simulation-class variables setting defaults (used in equations)
	int last_t = MAX_STEPS;				// number of simulation steps
	unsigned seed = 1;					// random number generator initial seed

	// simulation-class variables (used in equations)
	bool fast;							// safe copy of fast_mode flag
	bool no_saved = true;				// disable usage of saved values as lagged ones
	bool no_search;						// disable standard variable search mechanism
	bool no_search_up;					// disable object up-search mechanism
	bool no_zero_instance = true;		// flag to allow deleting last object instance
	bool parallel_mode;					// parallel mode (multithreading) status
	bool use_nan;						// flag to allow using Not a Number value
	char *conf_name = NULL;				// name of current simulation configuration
	eq_mapT eq_map;						// fast equation look-up map
	int fast_mode;						// level of LOG messages & runtime plot
	int last_run = 1;					// total serial simulation runs
	int no_ptr_chk = false;				// disable user pointer checking
	int quit = 0;						// simulation interruption mode (0=none)
	int run;							// current serial simulation run
	int sim;							// library simulation object index
	int t;								// current time step
	object *root = NULL;				// LSD root object
	o_setT obj_list;					// set with all existing LSD objects

#ifndef _NP_
	// simulation-class conditional variables (used in equations)
	mutex lock_obj_list;				// lock object list for parallel manipulation
#endif

#ifndef _NW_
	// simulation-class debugger temporary probe storage (used in equations)
	double d_values[ USER_D_VARS ];
	int i_values[ 4 ];					// user temporary variables copy
	netLink *n_values[ 10 ];
	object *o_values[ 10 ];
	FILE *f_values[ 1 ];
#endif

	// simulation-class methods (used in equations)
	char *no_node_chr( const char *lab, const char *file, int line );
	double build_obj_list( bool set_list );// build object list for pointer checking
	double init_lattice( int init_color = -0xffffff, double nrow = 100, double ncol = 100, double pixW = 0, double pixH = 0 );
	double init_lattice( double pixW = 0, double pixH = 0, double nrow = 100, double ncol = 100, const char lrow[ ] = "y", const char lcol[ ] = "x", const char lvar[ ] = "", object *p = NULL, int init_color = -0xffffff );
	double read_lattice( double line, double col );
	double save_lattice( const char fname[ ] = "lattice" );
	double update_lattice( double line, double col, double val = 1 );
	inline bool chk_hook( object *ptr, unsigned num );
	inline bool chk_obj( object *ptr );
	inline bool chk_ptr( object *ptr );
	inline char *bad_ptr_chr( object *ptr, const char *file, int line );
	inline double bad_ptr_dbl( object *ptr, const char *file, int line );
	inline double chk_res( double res, const char *lab );
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
	void error_hard( const char *boxTitle, const char *boxText, bool defQuit, const char *logFmt, ... );
	void init_map( void );
	void set_fast( int level );			// enable fast mode

#ifndef _FUN_
	// simulation-class variables (not used in equations)
	bool conf_ok = false;				// a valid configuration file is loaded
	bool error_hard_thread;				// flag to error_hard called in worker thread
	bool running = false;				// single simulation is running
	bool running_seq = false;			// set of sequential simulation runs is running
	bool save_alt = false;				// alternate save path flag
	bool save_ok = true;				// control saving model configuration possible
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
	char watch_elem[ MAX_ELEM_LENGTH + 1 ] = "";// element triggering watch condition
	description *descr = NULL;			// model description structure
	int deb_t;							// next debug stop time step (0 for none)
	int eff_t = 0;						// number of executed time steps
	int parallel_disable = false;		// flag to control parallel mode
	int prof_aggr_time = false;			// show aggregate profiling times
	int prof_min_msecs = 0;				// profile variables taking more than X msecs.
	int prof_obs_only = false;			// profile only observed variables
	int series_saved = 0;				// number of series saved
	int stack_level;					// LSD stack call level
	int stack_info = 0;					// LSD stack control
	lattice *latt = NULL;				// model lattice
	long nodesSerial = 1;				// network node serial number counter
	lsdstack *stack_log = NULL;			// LSD stack
	map < string, profile > prof_times;	// set of saved profiling times
	object *blueprint = NULL;			// LSD blueprint (effective model in use)
	object *wait_delete = NULL;			// LSD object waiting for deletion
	sensitivity *sens = NULL;			// LSD sensitivity analysis structure
	variable *cemetery = NULL;			// LSD saved data from deleted objects
	variable *last_cemetery = NULL;		// LSD last saved cemetery entry

#ifndef _NP_
	// simulation-class conditional variables (not used in equations)
	atomic < bool > parallel_ready;		// indicate variable worker is ready
	workerVar *workers = NULL;			// multi-thread parallel worker data
#endif

#endif
	// simulation-class methods (not used in equations)
	simulation( void );					// constructor
	~simulation( void );				// destructor

	bool load_description( const char *msg, FILE *f );
	bool next_batch( void );
	bool results_alt_path( const char *altPath );
	description *add_description( const char *lab, int type = 4, const char *text = NULL, const char *init = NULL, bool initial = false, bool observe = false );
	description *change_description( const char *lab_old, const char *lab = NULL, int type = -1, const char *text = NULL, const char *init = NULL, int initial = -1, int observe = -1 );
	description *search_description( const char *lab, bool add_missing = true );
	int load_configuration( bool reload, std::string *warnings, int quick );
	int hyper_count( const char *lab );
	int hyper_count_var( const char *lab );
	int run_simulation( int until_t = 0, int until_run = 0 );
	int worker_errors( void );
	void empty_blueprint( void );
	void empty_cemetery( void );
	void empty_description( void );
	void empty_lattice( void );
	void empty_sensitivity( sensitivity *cs = NULL );
	void empty_stack( void );
	void move_obj( const char *lab, const char *dest );
	void reset_blueprint( object *r );
	void save_results( void );
	void unload_configuration( bool full );
	void update_bar( char *bar, int done, int & last_done, int bar_sz );

#ifndef _NP_
	void parallel_update( variable *v, object* p, object *caller = NULL );
#endif

#ifdef SIMULATION_EXT
	SIMULATION_EXT
#endif
};

struct object							// simulation model object class
{
	bool *del_flag;						// address of flag to signal deletion
	bool deleting;						// indicate deletion in process
	bool to_compute;
	b_mapT b_map;						// fast lookup map to object bridges
	char *label;
	int acounter;
	int lstCntUpd;						// period of last counter update
	bridge *b;
	netNode *node;						// pointer to network node data structure
	object *hook;
	object *next;
	object *up;
	o_vecT hooks;
	simulation *sim;					// simulation where object is contained
	variable *v;
	void *cext;							// pointer to C++ object extension
	v_mapT v_map;						// fast lookup map to variables

#ifndef _NP_
	mutex parallel_comp;				// mutex lock for parallel computations
#endif

	// object-class methods
	bool alloc_save_mem( void );
	bool load_insts( const char *file_name, FILE *f );
	bool load_struct( FILE *f );
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
	int load_xml_insts( xml_node &n, n_mapT &node_map, set < int > &warning );
	int load_xml_struct( xml_node &n, bool quick );
	int logic_op_code( const char *lop, const char *errmsg );
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
	void search_inst( object *obj, long *pos, long *checked );
	void set_blueprint( object *container );
	void set_tit_counter( void );
	void update( bool recurse, bool user );

#ifdef OBJECT_EXT
	OBJECT_EXT
#endif
};

struct bridge							// descendant-object container class
{
	bool copy;							// just a temporary copy
	bool counter_updated;
	bridge *next;
	char *blabel;
	char *search_var;					// current initialized search variable
	n_mapT t_map;						// turbosearch map
	object *head;
	o_mapT o_map;						// fast lookup map to object values

	bridge( const char *lab );			// constructor
	bridge( const bridge &b );			// copy constructor
	~bridge( void );					// destructor
};

struct variable							// model numeric element (variable,
{										// parameter, or function) class
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
	double *data;
	double *val;
	double deb_cnd_val;
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
	object *up;
	simulation *sim;					// simulation where object is contained
	variable *next;

#ifndef _NP_
	recursive_mutex parallel_comp;		// mutex lock for parallel computation
#endif

	eq_funcT eq_func;					// pointer to equation function for fast look-up

	variable( void );					// empty constructor
	variable( const variable &v );		// copy constructor
	~variable( void );					// destructor

	bool alloc_save_var( void );
	double cal( object *caller, int lag );
	double fun( object *caller );
	inline double chk_dummy( const char *lab );
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
	bool integer;						// integer element
	char *label;
	double *v;							// values to test sensitivity
	int curv;							// index for value in use in combinations
	int lag;							// lag of initial value
	int numv;							// number of values to test
	int param;							// element type
	sensitivity *next;					// sensitivity analysis chain of elements
	simulation *sim;					// simulation where object is contained

	sensitivity( const char *lab, simulation *_sim, int _param, int _lag,
				 int _numv = 0, vector < double > *_v = NULL,
				 bool _integer = false );// constructor
	~sensitivity( void );				// destructor

#ifdef SENSITIVITY_EXT
	SENSITIVITY_EXT
#endif
};

struct description						// model-element description class
{
	char *init;
	char *label;
	char *text;
	char *type;
	bool initial;
	bool observe;
	description *next;

	description( const char *_label, int _type, const char *_text,
				 const char *_init, bool _initial, bool _observe );// constructor
	~description( void );

	bool has_descr_text( void );
};

struct netNode							// network node data class
{
	char *name;							// node textual name (not required)
	double prob;						// assigned node draw probability
	int time;							// time of creation/update
	long id;							// node unique ID number (reorderable)
	long nLinks;						// number of arcs FROM node
	long serNum;						// node serial number (for file save/export)
	netLink *first;						// first link in the linked list of links
	netLink *last;						// last link in the linked list of links
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
	netLink *next;						// pointer to next link (NULL if last )
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
struct workerVar						// multi-thread variable worker data structure
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
	simulation *sim;					// simulation where object is contained
	thread thr;
	thread::id thr_id;
	variable *var;

	workerVar( void );					// constructor
	~workerVar( void );					// destructor

	bool check( void );					// handle worker problems
	static void signal_wrapper( int signun );// wrapper for signal_handler
	void cal( variable *var );			// start worker calculation
	void cal_worker( void );			// worker thread code
	void signal( int signum );			// signal handler
};
#endif

struct result							// results file container class
{
	bool docsv;							// comma separated .csv text format
	bool dozip;							// compressed file flag
	bool firstCol;						// flag for first column in line
	gzFile fz;							// compressed file pointer
	simulation *sim;					// simulation where object is contained
	FILE *f;							// uncompressed file pointer

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
	char label[ MAX_ELEM_LENGTH ];
	int ns;
	lsdstack *next;
	lsdstack *prev;
	variable *vs;
};

struct profile							// profiled variable class
{
	unsigned int comp;
	unsigned long long ticks;

	profile( ) { ticks = 0; comp = 0; };// constructor
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
extern int deb_set;						// debug enable control (bool)
extern int platform;					// OS platform (1=Linux, 2=Mac, 3=Windows)

// library C++ functions (used in equations)
bool is_finite( double x );
bool is_inf( double x );
bool is_nan( double x );
double _abs( double a );
double alapl( double mu, double alpha1, double alpha2 );// draw from asym. laplace distr.
double alaplcdf( double mu, double alpha1, double alpha2, double x );// asym. laplace cdf
double bernoulli( double p );			// draw from a Bernoulli distribution
double beta( double alpha, double beta );// draw from a beta distribution
double betacdf( double alpha, double beta, double x );// beta cdf
double betacf( double a, double b, double x );// beta distribution function
double binomial( double p, double t );	// draw from a binomial distribution
double bpareto( double alpha, double low, double high );// draw from bounded pareto
double bparetocdf( double alpha, double low, double high, double x );
double cauchy( double a, double b );	// draw from Cauchy distribution
double chi_squared( double n );			// draw from chi-squared distribution
double exponential( double lambda );	// draw from exponential distribution
double fact( double x );				// Factorial function
double fisher( double m, double n );	// draw from Fisher-F distribution
double gamma( double alpha, double beta = 1 );// draw from a gamma distribution
double geometric( double p );			// draw from geometric distribution
double ipow( double base, double exp );	// integer exponentiation
double lnorm( double mu, double sigma );// draw from lognormal distribution
double lnormcdf( double mu, double sigma, double x );// lognormal cdf
double max( double a, double b );
double median( vector < double > & v );
double min( double a, double b );
double norm( double mean, double dev );
double normcdf( double mu, double sigma, double x );// normal cdf
double pareto( double mu, double alpha );
double paretocdf( double mu, double alpha, double x );
double poisson( double m );
double poissoncdf( double lambda, double k );// poisson cdf
double ran1( long *unused = 0 );
double round( double r );
double round_digits( double value, int digits );
double student( double n );				// draw from Student-T distribution
double t_star( int df, double cl );		// Student-t distribution statistic
double z_star( double cl );				// Standard normal distribution statistic
double unifcdf( double a, double b, double x );// uniform cdf
double uniform( double min, double max );
double uniform_int( double min, double max );
double weibull( double a, double b );	// draw from Weibull distribution
void deb_log( bool on, int time );		// control debug mode
void error_hard_helper( const char *boxTitle, const char *boxText, const char *logText, bool defQuit );
void init_random( unsigned seed );		// reset the random number generator seed
void msleep( unsigned msec = 1000 );	// sleep process for milliseconds
void plog( const char *msg, ... );		// write on log window
void *set_random( int gen );			// set random generator

#ifndef _FUN_

// library global variables (not used in equations)
extern bool batch_sequential;	// no-window multi configuration job running
extern bool batch_loop;			// batch multi-config batch loop in process
extern bool grandTotal;			// flag to produce grand total in batch processing
extern bool idle_loop;			// indicates in main idle loop (no running operation)
extern bool message_logged;		// new message posted in log window
extern bool on_bar;				// flag to indicate bar is being draw in log
extern bool parallel_monitor;	// parallel monitor thread status
extern char *exec_file;			// name of executable file
extern char *exec_path;			// path of executable file
extern char *lib_file;			// name of shared library, if any
extern char *lib_path;			// path of shared library, if any
extern char *model_path;		// folder where the model files are
extern char *rootLsd;			// path of LSD root directory
extern char nonavail[ ];		// string for unavailable values
extern dlliblinkage liblnk;		// call-back references for DLL
extern double t_dist_cl[ T_CLEVS ];// t-distribution table confidence levels
extern double t_dist_st[ T_CLEVS ][ 36 ];// t-distribution table statistics
extern double z_dist_cl[ Z_CLEVS ];// normal distribution table confidence levels
extern double z_dist_st[ Z_CLEVS ];// normal distribution table statistics
extern int add_to_tot;			// type of totals file generated (bool)
extern int choice;				// Tcl menu control variable (main window)
extern int dobar;				// output a progress bar to the log/standard output
extern int docsv;				// produce .csv text results files (bool)
extern int dozip;				// compressed results file flag (bool)
extern int fend;				// last multi configuration job to run
extern int findex;				// current multi configuration job
extern int log_start;			// first period to start logging to file, if any
extern int log_stop;			// last period to log to file, if any
extern int max_runs;			// maximum number of parallel runs
extern int max_threads;			// maximum parallel threads per run
extern int no_res;				// do not produce .res results files (bool)
extern int no_tot;				// do not produce .tot totals files (bool)
extern mt19937 mt32;			// Mersenne-Twister 32 bits generator
extern vector < simulation * > sims;// vector holding existing simulations
extern vector < string > res_list;// list of results files last saved
extern FILE *log_file_ptr;		// log file pointer, if any

extern void *random_engine;		// current random number generator engine

// library constant string arrays (not visible to the users)
extern const char *desc_key_words[ ];
extern const char *elem_type_names[ ];
extern const char *signal_names[ ];
extern const int signals[ ];			// handled system signal numbers

#ifndef _NP_
// library conditional variables
extern map < thread::id, workerVar * > thr_ptr;// worker thread pointers
extern mutex lock_run_logs;		// lock run_logs for parallel updating
extern mutex lock_run_pids;		// lock run_pids for parallel updating
extern mutex lock_run_status;	// lock run_status for parallel updating
extern string run_log;			// consolidated runs log
extern thread run_monitor;		// thread monitoring parallel instances
extern thread::id main_thread;	// LSD main thread ID
extern vector < handleT > run_pids;// parallel running instances process id's
extern vector < int > run_status;// parallel running instances status
extern vector < string > run_logs;// log file list produced in parallel runs
extern vector < string > run_results;// parallel run results files
extern vector < thread > run_threads;// parallel running instances
#endif

#ifndef _NW_
// library Tcl/Tk specific definitions (for the GUI version only)
extern p_mapT par_map;			// variable to parent name map for AoR
extern Tcl_Interp *inter;		// Tcl interpreter in GUI (for legacy LSD code)
#endif

// library C++ functions (not used in equations)
bool check_cond( double val1, int lopc, double val2 );
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
int kill_system( int id );
int monitor_logs( void );
int rnd_int( int min, int max );
int run_parallel( bool nw, const char *exec, const char *simname, int fseed, int runs, int thrrun, int parruns );
int run_system( const char *cmd, int id = -1 );
int strcln( char *out, const char *str, int outSz );
int strlf( char *out, const char *str, int outSz );
int strtrim( char *out, const char *str, int outSz );
int strtrimin( char *out, const char *str, int outSz );
long strtol( const char *in, char** endptr, int base, long inv );
object *go_brother( object *c );
object *skip_next_obj( object *t );
object *skip_next_obj( object *t, int *count );
vector < double > strtodsplit( const char *in, char sep, double inv = 0. );
vector < long > strtolsplit( const char *in, char sep, long inv = 0 );
vector < string > strtostrsplit( const char *in, char sep, bool remQuotes = false );
void close_sim( void );
void cmd_gui( const char *cm, ... );
void detach_parallel( void );
void exception_handler( int signum, const char *what );
void handle_signals( void ( * handler ) ( int signum ) );
void init_map( void );
void init_math_error( void );
void log_parallel( bool nw );
void lsd_exit( int v );
void monitor_parallel( bool nw );
void plog_tag( const char *cm, const char *tag, ... );
void plog_terminal( const char *cm, va_list arg );
void run_parallel_exec( bool nw, int id, string cmd );
void set_exec( const char *path, const char *file );
void signal_handler( int signum );
void warn_distr( int *errCnt, bool *stopErr, const char *distr, const char *msg );
FILE *search_data_str( const char *name, const char *init, const char *str );

#endif
