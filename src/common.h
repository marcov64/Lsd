/*************************************************************

	LSD 8.0 - March 2021
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
 
 - LMM: Model Manager executable
 - FUN: user model equation file
 - NW: No Window executable
 - NP: no parallel (multi-task) processing
 - NT: no signal trapping (better when debugging in GDB)
 *************************************************************/

// LSD version strings, for About... boxes and code testing
#define _LSD_MAJOR_ 8
#define _LSD_MINOR_ 0
#define _LSD_VERSION_ "8.0-beta-2"
#define _LSD_DATE_ "March 22 2020"   // __DATE__

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
#endif

// global constants
#define TCL_BUFF_STR 3000				// standard Tcl buffer size (>1000)
#define MAX_PATH_LENGTH 500				// maximum path length
#define MAX_LINE_SIZE 1000				// max size of a text line to read from files (>999)
#define MAX_ELEM_LENGTH 100				// maximum element ( object, variable ) name length (>99)
#define MAX_FILE_SIZE 1000000			// max number of bytes to read from files
#define MAX_FILE_TRY 100000				// max number of lines to read from files

// platform codes
#define LINUX	1
#define MAC		2
#define WINDOWS	3

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
#define TCL_LIB_VAR 	"TCL_LIBRARY"
#define TCL_LIB_PATH 	"gnu/lib/tcl8.6"// must NOT use backslashes
#define TCL_LIB_INIT 	"init.tcl"
#define TCL_EXEC_PATH 	"gnu\\bin"		// must use (double) backslashes
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
#define MODEL_INFO_NUM 11
#define MODEL_INFO_NAME { "modelName", "modelVersion", "modelDate", \
						  "lsdGeom", "logGeom", "strGeom", \
						  "daGeom", "debGeom", "latGeom", \
						  "pltGeom", "dapGeom" }
#define MODEL_INFO_DEFAULT { "(no name)", "1.0", "[ current_date ]", \
							 "#", "#", "#", \
							 "#", "#", "#", \
							 "#", "#" };
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

#ifndef NP
	mutex parallel_comp;				// mutex lock for parallel computations
#endif

	bool load_param( char *file_name, int repl, FILE *f );
	bool load_struct( FILE *f );
	bool under_computation( void );
	bool under_comput_var( char const *lab );
	bridge *search_bridge( char const *lab, bool no_error = false );
	double av( char const *lab1, int lag = 0, bool cond = false, char const *lab2 = "", char const *lop = "", double value = NAN );
	double cal( char const *l, int lag = 0 );
	double cal( object *caller, char const *l, int lag = 0 );
	double cal( object *caller, char const *l, int lag, bool force_search );
	double count( char const *lab1, int lag = 0, bool cond = false, char const *lab2 = "", char const *lop = "", double value = NAN );
	double count_all( char const *lab1, int lag = 0, bool cond = false, char const *lab2 = "", char const *lop = "", double value = NAN );
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
	double med( char const *lab1, int lag = 0, bool cond = false, char const *lab2 = "", char const *lop = "", double value = NAN );
	double multiply( char const *lab, double value );
	double overall_max( char const *lab1, int lag = 0, bool cond = false, char const *lab2 = "", char const *lop = "", double value = NAN );
	double overall_min( char const *lab1, int lag = 0, bool cond = false, char const *lab2 = "", char const *lop = "", double value = NAN );
	double perc( char const *lab1, double p, int lag = 0, bool cond = false, char const *lab2 = "", char const *lop = "", double value = NAN );
	double read_file_net( char const *lab, char const *dir = "", char const *base_name = "net", int serial = 1, char const *ext = "net" );
	double recal( char const *l );
	double sd( char const *lab1, int lag = 0, bool cond = false, char const *lab2 = "", char const *lop = "", double value = NAN );
	double search_inst( object *obj = NULL, bool fun = true );
	double stat( char const *lab1, double *v = NULL, int lag = 0, bool cond = false, char const *lab2 = "", char const *lop = "", double value = NAN );
	double stats_net( char const *lab, double *r );
	double sum( char const *lab1, int lag = 0, bool cond = false, char const *lab2 = "", char const *lop = "", double value = NAN );
	double to_delete( void );
	double whg_av( char const *lab1, char const *lab2, int lag = 0, bool cond = false, char const *lab3 = "", char const *lop = "", double value = NAN );
	double write( char const *lab, double value, int time, int lag = 0 );
	double write_file_net( char const *lab, char const *dir = "", char const *base_name = "net", int serial = 1, bool append = false );
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
	object *draw_rnd( char const *lo, char const *lv, int lag = 0 );
	object *draw_rnd( char const *lo, char const *lv, int lag, double tot );
	object *hyper_next( char const *lab );
	object *hyper_next( void );
	object *lat_down( void );
	object *lat_left( void );
	object *lat_right( void );
	object *lat_up( void );
	object *lsdqsort( char const *obj, char const *var, char const *direction, int lag = 0 );
	object *lsdqsort( char const *obj, char const *var1, char const *var2, char const *direction, int lag = 0 );
	object *search( char const *lab, bool no_search = false );
	object *search_err( char const *lab, bool no_search, char const *errmsg );
	object *search_node_net( char const *lab, long id ); 
	object *search_var_cond( char const *lab, double value, int lag = 0 );
	object *shuffle_nodes_net( char const *lab );
	object *turbosearch( char const *label, double tot, double num );
	object *turbosearch_cond( char const *label, double value );
	variable *add_empty_var( char const *str );
	variable *search_var( object *caller, char const *label, bool no_error = false, bool no_search = false, bool search_sons = false );
	variable *search_var_err( object *caller, char const *label, bool no_search, bool search_sons, char const *errmsg );
	void add_obj( char const *label, int num, int propagate );
	void add_var_from_example( variable *example );
	void chg_lab( char const *lab );
	void chg_var_lab( char const *old, char const *n );
	void collect_cemetery( variable *caller = NULL );
	void delete_link_net( netLink *ptr );
	void delete_net( char const *lab );
	void delete_node_net( void );
	void delete_obj( variable *caller = NULL );
	void delete_var( char const *lab );
	void empty( void );
	void emptyturbo( void );			// remove turbo search structure
	void init( object *_up, char const *_label, bool _to_compute = true );
	void name_node_net( char const *nodeName );
	void recreate_maps( void );
	void replicate( int num, bool propagate = false );
	void save_param( FILE *f );
	void save_struct( FILE *f, char const *tab );
	void search_inst( object *obj, long *pos, long *checked );
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
	
#ifndef NP
	mutex parallel_comp;				// mutex lock for parallel computation
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

struct store
{
	char label[ MAX_ELEM_LENGTH + 1 ];
	char tag[ MAX_ELEM_LENGTH + 1 ];
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

struct lsdstack
{
	char label[ MAX_ELEM_LENGTH + 1 ];
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

#ifndef NP
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
void msleep( unsigned msec = 1000 );	// sleep process for milliseconds
void plog( char const *msg, char const *tag = "", ... );	// write on log window


// common global variables (visible to the users)
extern int fast_mode;					// execution speed control flag
extern int platform;					// OS platform (1=Linux, 2=Mac, 3=Windows)
extern int quit;						// simulation termination control flag


// prevent exposing internals in users' fun_xxx.cpp
#ifndef FUN

// common standalone internal C functions/procedures (not visible to the users)
bool get_bool( const char *tcl_var, bool *var = NULL );
bool load_lmm_options( void );
bool load_model_info( const char *path );
bool set_env( bool set );
bool valid_label( const char *lab );
char *clean_file( char *file );
char *clean_path( char *path );
char *get_str( const char *tcl_var, char *var = NULL, int var_size = 0 );
char *search_lsd_root( char *start_path );
char *str_upr( char *s );
const char *signal_name( int signum );
double get_double( const char *tcl_var, double *var = NULL );
int deb( object *r, object *c, char const *lab, double *res, bool interact = false, const char *hl_var = "" );
int get_int( const char *tcl_var, int *var = NULL );
int lsdmain( int argn, char **argv );
long get_long( const char *tcl_var, long *var = NULL );
string win_path( string filepath );
void check_option_files( bool sys );
void clean_spaces( char *s );
void cmd( const char *cm, ... );
void handle_signals( void ( * handler )( int signum ) );
void init_tcl_tk( const char *exec, const char *tcl_app_name );
void clean_newlines( char *s );
void log_tcl_error( const char *cm, const char *message );
void myexit( int v );
void print_stack( void );
void signal_handler( int signum );
void update_lmm_options( bool justLmmGeom = false );
void update_model_info( void );

#ifdef LMM
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
extern char msg[ ];						// auxiliary Tcl buffer
extern int choice;						// Tcl menu control variable (main window)
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
#ifndef NP
extern thread::id main_thread;			// LSD main thread ID
extern worker *workers;					// multi-thread parallel worker data
#endif

// Tcl/Tk specific definitions (for the windowed version only)
#ifndef NW

#include <tk.h>

extern Tcl_Interp *inter;				// Tcl standard interpreter pointer

// C to TCL interface functions
int Tcl_discard_change( ClientData cdata, Tcl_Interp *inter, int argc, const char *argv[ ] );
int Tcl_log_tcl_error( ClientData cdata, Tcl_Interp *inter, int argc, const char *argv[ ] );

#endif

#endif

