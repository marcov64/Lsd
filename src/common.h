/*************************************************************

	LSD 8.0 - May 2022
	written by Marco Valente, Universita' dell'Aquila
	and by Marcelo Pereira, University of Campinas

	Copyright Marco Valente and Marcelo Pereira
	LSD is distributed under the GNU General Public License

	See Readme.txt for copyright information of
	third parties' code used in LSD

 *************************************************************/

/*************************************************************
 COMMON.H
 Global definitions common between LMM and LSD Browser

 Relevant flags (when defined):

 - _LMM_: Model Manager executable
 - _FUN_: user model equation file
 - _NW_: No Window executable
 - _NP_: no parallel (multi-task) processing
 - _NT_: no signal trapping (better when debugging in GDB)
 *************************************************************/

// LSD version strings, for About... boxes and code testing
#define _LSD_MAJOR_ 8
#define _LSD_MINOR_ 0
#define _LSD_VERSION_ "8.0"
#define _LSD_DATE_ "May 2 2022"	 // __DATE__

// standard libraries used
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <cmath>
#include <ctime>
#include <csignal>
#include <new>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <csetjmp>
#include <sys/stat.h>
#include <zlib.h>

#ifdef _WIN32
#include <windows.h>
#undef DELETE
#undef THIS
#else
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <wordexp.h>
#endif

// global constants
#define MAX_BUFF_SIZE 10000				// standard Tcl buffer size (>9999)
#define MAX_PATH_LENGTH 1000			// maximum path length (>999)
#define MAX_LINE_SIZE 1000				// max size of a text line to read from files (>999)
#define MAX_ELEM_LENGTH 100				// maximum element ( object, variable ) name length (>99)
#define MAX_FILE_SIZE 1000000			// max number of bytes to read from files
#define MAX_FILE_TRY 100000				// max number of lines to read from files

// platform codes
#define _LIN_	1
#define _MAC_	2
#define _WIN_	3

// Choose directory/file separator
#define foldersep( dir ) ( dir[ 0 ] == '\0' ? "" : "/" )

// date/time format
#define DATE_FMT "%d %B, %Y"

// configuration files details
#define LMM_OPTIONS "lmm_options.txt"
#define SYSTEM_OPTIONS "system_options.txt"
#define MODEL_OPTIONS "model_options.txt"
#define GROUP_INFO "groupinfo.txt"
#define MODEL_INFO "modelinfo.txt"
#define DESCRIPTION "description.txt"

// user defined signals
#define SIGMEM NSIG + 1					// out of memory signal
#define SIGSTL NSIG + 2					// standard library exception signal

// Special file names/locations in Windows
#define TCL_LIB_VAR		"TCL_LIBRARY"
#define TCL_LIB_PATH	"gnu/lib/tcl8.6"// must NOT use backslashes
#define TCL_LIB_INIT	"init.tcl"
#define TCL_EXEC_PATH	"gnu\\bin"		// must use (double) backslashes
#define TCL_FIND_EXE	"@where wish86.exe > nul 2>&1"

// Eigen library include command
#define EIGEN "#define EIGENLIB"

// constant string arrays
#define LMM_OPTIONS_NUM 16
#define LMM_OPTIONS_NAME { "sysTerm", "HtmlBrowser", "fonttype", \
						   "wish", "LsdSrc", "dim_character", \
						   "tabsize", "wrap", "shigh", \
						   "autoHide", "showFileCmds", "LsdNew", \
						   "DbgExe", "restoreWin", "lmmGeom", \
						   "lsdTheme" }
#define LMM_OPTIONS_DEFAULT { "$DefaultSysTerm", "$DefaultHtmlBrowser", "$DefaultFont", \
							  "$DefaultWish", "src", "$DefaultFontSize", \
							  "4", "1", "2", \
							  "0", "0", "Work", \
							  "$DefaultDbgExe", "1", "#", \
							  "$DefaultTheme" }
#define MODEL_INFO_NUM 16
#define MODEL_INFO_NAME { "modelName", "modelVersion", "modelDate", \
						  "lsdGeom", "logGeom", "strGeom", \
						  "daGeom", "debGeom", "latGeom", \
						  "pltGeom", "dapGeom", "lastConf", \
						  "lastObj", "lastList", "lastItem", "lastFirst" }
#define MODEL_INFO_DEFAULT { "(no name)", "1.0", "[ current_date ]", \
							 "#", "#", "#", \
							 "#", "#", "#", \
							 "#", "#", "#", \
							 "Root", "1", "0", "0" }
#define LSD_NW_NUM 12
#define LSD_NW_SRC { "lsdmain.cpp", "common.cpp", "file.cpp", "nets.cpp", \
					 "object.cpp", "util.cpp", "variab.cpp", "check.h", \
					 "common.h", "decl.h", "fun_head.h", "fun_head_fast.h" }
#define LSD_DIR_NUM 8
#define LSD_DIR_NAME { "src", "gnu", "installer", "Manual", "LMM.app", "Rpkg", "lwi", "___" }
#define LSD_MIN_NUM 3
#define LSD_MIN_FILES { "src/icons", "src/themes", "src/interf.cpp", "src/analysis.cpp" }
#define WIN_COMP_NUM 2
#define WIN_COMP_PATH { "mingw64\\bin", "cygwin64\\bin" }	// must use (double) backslashes
#define LSD_WIN_NUM MODEL_INFO_NUM - 3
#define LSD_WIN_NAME { "lsd", "log", "str", "da", "deb", "lat", "plt", "dap" }
#define REG_SIG_NUM 6
#define REG_SIG_CODE { SIGINT, SIGTERM, SIGABRT, SIGFPE, SIGILL, SIGSEGV }
#define REG_SIG_NAME { "Interrupt signal", "Terminate signal", "Abort signal", \
					   "Floating-point exception", "Illegal instruction", "Segmentation violation" }

using namespace std;

// classes pre-definitions
struct object;
struct variable;
struct bridge;
struct mnode;
struct netNode;
struct netLink;

// special types used for fast equation, object and variable lookup
typedef function < double( object *caller, variable *var ) > eq_funcT;
typedef pair < string, bridge * > b_pairT;
typedef pair < double, object * > o_pairT;
typedef pair < string, variable * > v_pairT;
typedef vector < object * > o_vecT;
typedef unordered_map < string, eq_funcT > eq_mapT;
typedef unordered_map < string, bridge * > b_mapT;
typedef unordered_map < double, object * > o_mapT;
typedef unordered_map < string, string > p_mapT;
typedef unordered_map < string, variable * > v_mapT;
typedef unordered_set < object * > o_setT;

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

	bool load_param( const char *file_name, int repl, FILE *f );
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
	double initturbo( const char *label, double num );
	double initturbo_cond( const char *label );
	double init_stub_net( const char *lab, const char* gen, long numNodes = 0, long par1 = 0, double par2 = 0.0 );
	double interact( const char *text, double v, double *tv, int i, int j, int h, int k,
		object *cur, object *cur1, object *cur2, object *cur3, object *cur4, object *cur5,
		object *cur6, object *cur7, object *cur8, object *cur9, netLink *curl, netLink *curl1,
		netLink *curl2, netLink *curl3, netLink *curl4, netLink *curl5, netLink *curl6,
		netLink *curl7, netLink *curl8, netLink *curl9 );
	double last_cal( const char *lab );
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
	netLink *add_link_net( object *destPtr, double weight = 0, double probTo = 1 );
	netLink *draw_link_net( void );
	netLink *search_link_net( long id );
	object *add_n_objects2( const char *lab, int n, int t_update = -1 );
	object *add_n_objects2( const char *lab, int n, object *ex, int t_update = -1 );
	object *add_node_net( long id = -1, const char *nodeName = "", bool silent = false );
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
	object *search( const char *lab, bool no_search = false );
	object *search_err( const char *lab, bool no_search, const char *errmsg );
	object *search_node_net( const char *lab, long id );
	object *search_var_cond( const char *lab, double value, int lag = 0 );
	object *shuffle_nodes_net( const char *lab );
	object *turbosearch( const char *label, double tot, double num );
	object *turbosearch_cond( const char *label, double value );
	variable *add_empty_var( const char *str );
	variable *search_var( object *caller, const char *label, bool no_error = false, bool no_search = false, bool search_sons = false );
	variable *search_var_err( object *caller, const char *label, bool no_search, bool search_sons, const char *errmsg );
	void add_obj( const char *label, int num, int propagate );
	void add_var_from_example( variable *example );
	void chg_lab( const char *lab );
	void chg_var_lab( const char *old, const char *n );
	void collect_cemetery( variable *caller = NULL );
	void delete_link_net( netLink *ptr );
	void delete_net( const char *lab );
	void delete_node_net( void );
	void delete_obj( variable *caller = NULL );
	void delete_var( const char *lab );
	void empty( void );
	void emptyturbo( void );			// remove turbo search structure
	void init( object *_up, const char *_label, bool _to_compute = true );
	void name_node_net( const char *nodeName );
	void recreate_maps( void );
	void replicate( int num, bool propagate = false );
	void save_param( FILE *f );
	void save_struct( FILE *f, const char *tab );
	void search_inst( object *obj, long *pos, long *checked );
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

#ifndef _NP_
	recursive_mutex parallel_comp;		// mutex lock for parallel computation
#endif

	eq_funcT eq_func;					// pointer to equation function for fast look-up

	variable( void );					// empty constructor
	variable( const variable &v );		// copy constructor

	double cal( object *caller, int lag );
	double fun( object *caller );
	void empty( bool no_lock = false );
	void init( object *_up, const char *_label, int _num_lag, double *val, int _save );
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

	netNode( long nodeId = -1, const char nodeName[ ] = "", double nodeProb = 1 );
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


// standalone C functions/procedures (visible to the users)
void msleep( unsigned msec = 1000 );	// sleep process for milliseconds
void plog( const char *msg, ... );		// write on log window


// common global variables (visible to the users)
extern int fast_mode;					// execution speed control flag
extern int platform;					// OS platform (1=Linux, 2=Mac, 3=Windows)
extern int quit;						// simulation termination control flag


// prevent exposing internals in users' fun_xxx.cpp
#ifndef _FUN_

// common standalone internal C functions/procedures (not visible to the users)
bool compile_run( int run_mode, bool nw = false );
bool expr_eq( const char *tcl_exp, const char *c_str );
bool eval_bool( const char *tcl_exp );
bool exists_var( const char *lab );
bool exists_window( const char *lab );
bool get_bool( const char *tcl_var, bool *var = NULL );
bool load_lmm_options( void );
bool load_model_info( const char *path );
bool make_no_window( void );
bool set_env( bool set );
bool strwsp( const char *str );
bool use_eigen( void );
bool valid_label( const char *lab );
char *clean_file( const char *file );
char *clean_path( char *path );
char *eval_str( const char *tcl_exp, char *var, int var_size );
char *get_str( const char *tcl_var, char *var, int var_size );
char *search_lsd_root( char *start_path );
char *strcatn( char *d, const char *s, size_t dSz );
char *strcpyn( char *d, const char *s, size_t dSz );
char *strtcl( char *out, const char *text, int outSz );
char *strupr( char *s );
const char *eval_str( const char *tcl_exp );
const char *get_fun_name( char *str, int str_sz, bool nw = false );
const char *get_str( const char *tcl_var );
const char *signal_name( int signum );
double eval_double( const char *tcl_exp );
double get_double( const char *tcl_var, double *var = NULL );
int deb( object *r, object *c, const char *lab, double *res, bool interact = false, const char *hl_var = "" );
int eval_int( const char *tcl_exp );
int get_int( const char *tcl_var, int *var = NULL );
int kill_system( int id );
int lsdmain( int argn, const char **argv );
int strcln( char *out, const char *str, int outSz );
int strlf( char *out, const char *str, int outSz );
int strtrim( char *out, const char *str, int outSz );
int strwrap( char *out, const char *str, int outSz, int wid );
int run_system( const char *cmd, int id = -1 );
int worker_errors( void );
long eval_long( const char *tcl_exp );
long get_long( const char *tcl_var, long *var = NULL );
string win_path( string filepath );
void check_option_files( bool sys = false );
void clean_spaces( char *s );
void cmd( const char *cm, ... );
void exception_handler( int signum, const char *what = NULL );
void handle_signals( void ( * handler ) ( int signum ) );
void init_tcl_tk( const char *exec, const char *tcl_app_name );
void log_tcl_error( bool show, const char *cm, const char *message, ... );
void make_makefile( bool nw = false );
void myexit( int v );
void print_stack( void );
void show_comp_result( bool nw = false );
void show_tcl_error( const char *boxTitle, const char *errMsg, ... );
void signal_handler( int signum );
void update_lmm_options( bool justLmmGeom = false );
void update_model_info( bool fix = false );

#ifdef _LMM_
bool discard_change( void );
#else
bool discard_change( bool checkSense = true, bool senseOnly = false, const char title[ ] = "" );
#endif

// commonglobal internal variables (not visible to the users)
extern bool parallel_mode;				// parallel mode (multithreading) status
extern bool tk_ok;						// control for tk_ready to operate
extern bool user_exception;				// flag indicating exception was generated by user code
extern char *exec_path;					// path of executable file
extern char *rootLsd;					// path of LSD root directory
extern char equation_name[ ];			// equation file name
extern char err_file[ ];				// error log file name
extern int stop;						// activity interruption flag (Tcl boolean)
extern lsdstack *stacklog;				// LSD stack

// common constant string arrays (not visible to the users)
extern const char *lmm_options[ ];
extern const char *lmm_defaults[ ];
extern const char *model_info[ ];
extern const char *model_defaults[ ];
extern const char *signal_names[ ];
extern const char *wnd_names[ ];		// LSD main windows' names
extern const int signals[ ];			// handled system signal numbers

// multi-threading control
#ifndef _NP_
extern mutex lock_run_pids;				// lock run_pids for parallel updating
extern thread::id main_thread;			// LSD main thread ID
extern vector < handleT > run_pids;		// parallel running instances process id's
extern worker *workers;					// multi-thread parallel worker data
#endif

// Tcl/Tk specific definitions (for the windowed version only)
#ifndef _NW_

#include <tk.h>

extern Tcl_Interp *inter;				// Tcl standard interpreter pointer

// C to TCL interface functions
int Tcl_discard_change( ClientData cdata, Tcl_Interp *inter, int argc, const char *argv[ ] );
int Tcl_log_tcl_error( ClientData cdata, Tcl_Interp *inter, int argc, const char *argv[ ] );

#endif

#endif
