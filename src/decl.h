/*************************************************************

	LSD 7.1 - May 2018
	written by Marco Valente, Universita' dell'Aquila
	and by Marcelo Pereira, University of Campinas

	Copyright Marco Valente
	LSD is distributed under the GNU General Public License
	
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
#include <sys/stat.h>
#include <string>
#include <list>
#include <new>
#include <map>
#include <vector>
#include <deque>
#include <iterator>

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
#define _LSD_MINOR_ 1
#define _LSD_VERSION_ "7.1"
#define _LSD_DATE_ "Aug 15 2018"        // __DATE__

// global constants
#define TCL_BUFF_STR 3000				// standard Tcl buffer size (>1000)
#define MAX_PATH_LENGTH 500				// maximum path length (>499)
#define MAX_ELEM_LENGTH 100				// maximum element ( object, variable ) name length (>99)
#define MAX_FILE_SIZE 1000000			// max number of bytes to read from files
#define MAX_FILE_TRY 100000				// max number of lines to read from files
#define MAX_LINE_SIZE 1000				// max size of a text line to read from files (>999)
#define NOLH_DEF_FILE "NOLH.csv"		// default NOLH file name
#define MAX_SENS_POINTS 999				// default warning threshold for sensitivity analysis
#define MAX_COLS 100					// max numbers of columns in init. editor
#define MAX_PLOTS 1000					// max numbers of plots in analysis
#define MAX_CORES 0						// maximum number of cores to use (0=auto )
#define MAX_WAIT_TIME 10				// maximum wait time for a variable computation ( sec.)
#define MAX_TIMEOUT 100					// maximum timeout for multi-thread scheduler (millisec.)
#define MAX_LEVEL 10					// maximum number of object levels (for plotting only)
#define ERR_LIM 10						// maximum number of repeated error messages
#define BAR_DONE_SIZE 80				// characters in the percentage done bar
#define SIG_DIG 10						// number of significant digits in data files
#define CSV_SEP ","						// single char string with the .csv format separator

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

using namespace std;

// classes definitions
class object;
class variable;
struct gisMap;

#ifdef CPP11
// special types used for fast equation lookup
typedef function< double( object *caller, variable *var ) > eq_funcT;
typedef map< string, eq_funcT > eq_mapT;
#endif

class variable
{
	public:
	char *label;
	object *up;
	double *val;
	int last_update;
	int num_lag;
	variable *next;
	bool save;
	bool savei;
	bool under_computation;
	bool plot;
	bool parallel;
	bool observe;
	int param;
	char debug;
	int deb_cond;
	double deb_cnd_val;
	char data_loaded;
	char computable;
	double *data;
	char *lab_tit;
	int start;
	int end;
	
#ifdef PARALLEL_MODE
	mutex parallel_comp;		// mutex lock for parallel computation
#endif

#ifdef CPP11
	eq_funcT eq_func = NULL;	// pointer to equation function for fast look-up
#endif

	int init( object *_up, char const *_label, int _num_lag, double *val, int _save );
	double cal( object *caller, int lag );
	double fun( object *caller );
	void empty( void );
};

class mnode
{
	public:
	mnode *son;
	object *pntr;
	long deflev;		// saves the log of number of objects to allow defaulting

	void create( double level);
	void empty( void );
	object *fetch( double *n, double level = 0 );
};

class bridge
{
	public:
	object *head;
	char *blabel;
	bridge *next;
	bool counter_updated;
	mnode *mn;
};

struct store
{
	char label[ MAX_ELEM_LENGTH ];
	int start;
	int end;
	char tag[ MAX_ELEM_LENGTH ];
	double *data;
	int rank;
};

// network data structures
struct netLink		// individual outgoing link
{
	int time;		// time of creation/update
	long serTo;		// destination node serial number (fixed )
	object *ptrTo;	// pointer to destination number
	object *ptrFrom;// network node containing the link
	netLink *prev;	// pointer to previous link (NULL if first )
	netLink *next;	// pointer to next link (NULL if last )
	double weight;	// link weight
	double probTo;	// destination node draw probability
	
	netLink( object *origNode, object *destNode, double linkWeight = 0, double destProb = 1 ); 
					// constructor
	~netLink( void ); // destructor
};

struct netNode		// network node data
{
	long id;		// node unique ID number (reorderable )
	char *name;		// node textual name (not required )
	int time;		// time of creation/update
	long serNum;	// node serial number (initial order, fixed )
	long nLinks;	// number of arcs FROM node
	netLink *first;	// first link in the linked list of links
	netLink *last;	// last link in the linked list of links
	double prob;	// assigned node draw probability
	
	netNode( long nodeId = -1, char const nodeName[ ] = "", double nodeProb = 1 );
					// constructor
	~netNode( void );// destructor
};

/*
//to do: provide advanced attributes for netLink and netNode
struct linkAttributes //network link attributes
{
  netLink *link; //link to netLink object
  object *linkObj; //link to associated object (if any)

  linkAttributes(netLink *link, object *linkObj); //constructor
  ~linkAttributes( void ); //destructor

  double attribute(char const attrName[ ]); //get attribute from assoc. LSD object.
}

struct nodeAttributes //network link attributes
{
  netNode *node; //link to netNode object
  object *nodeObj; //link to associated object


  nodeAttributes(netNode *node, object *nodeObj); //constructor
  ~nodeAttributes( void ); //destructor

  double attribute(char const attrName[ ]); //get attribute from assoc. LSD object.
}
*/

struct gisPosition
{
  public:
  gisMap* map;  //link to shared map
  double x;     //x position
  double y;     //y position
  double z;     //z position, if any (default 0, not used in map!
  std::deque<object*> in_radius; //list of objects in range, used by search

  gisPosition (gisMap* map, double x, double y, double z=0) : map(map), x(x), y(y), z(z)  //constructor.
  {
  };

//   ~gisPosition( void ); //destructor. Take care of getting it OUT OF THE MAP
};

struct Wrap
{
    bool noWrap;
    bool left;
    bool right;
    bool top;
    bool bottom;
   /*wrapping: there are 2^4 options. We use a bit-code (0=off):
    0-bit: left     : 0=0 1=1
    1-bit: right    : 0=0 1=2
    2-bit: top      : 0=0 1=4
    3-bit: bottom   : 0=0 1=8
    */
    Wrap(int wrap){
      if (wrap == 0) {
        noWrap = true;
      } else {
        noWrap = false;
        if (wrap>7){bottom=true; wrap-=8;} else {bottom=false; }
        if (wrap>3){top=true;    wrap-=4;} else {top=false;    }
        if (wrap>1){right=true;  wrap-=2;} else {right=false;  }
        if (wrap>0){left=true;           } else {left=false;   }
      }
    };
    Wrap(bool left, bool right, bool top, bool bottom) : left(left),right(right),top(top),bottom(bottom)
    {
      if (left == right == top == bottom == false){
        noWrap = true;
      } else {
        noWrap = false;
      }
    };
//     ~Wrap( void );
};

struct gisMap
{
  int xn;
  int yn;
  Wrap wrap;
  /*
    there are 2^4 options. We use a bit-code (0=off):
    0-bit: left     : 0=0 1=1
    1-bit: right    : 0=0 1=2
    2-bit: top      : 0=0 1=4
    3-bit: bottom   : 0=0 1=8
    Simply sum up the options selected and pass this as argument.
    Examples: 0 = None, 1 = left, 2 = right, 3 (1+2) left-right,
    5 up, 7 down, 12 (5+7) up-down, 15 (1+2+5+7) torus.
  */
  std::vector<std::vector <std::deque<object*> >> elements;
  int nelements; //count number of elements

  gisMap(int xn, int yn, int _wrap=0) : xn(xn), yn(yn),wrap(_wrap) //constructor
  {
      nelements = 0;
      elements.resize(xn);
      for (auto& x : elements){
        x.resize(yn); //number of rows, copy
      }
    };

//   ~gisMap( void ) //destructor
//   {
//     //no need to do any thing, this is done in the objects with gisPosition in map.
//   };
};

     //to do: allow doubles
  std::vector<object*> within_radius(object *origin, char const label[], double radius, bool random = true); //give back elements in radius
  std::vector<object*> at_position(char const label[], int x, int y, bool random = true); //give back elements at position
  gisMap* map_from_obj(object* origin); //get mapPointer from an Object situated in a GIS
class object
{
	public:
	char *label;
	object *up;
	object *next;
	object *hook;
	variable *v;
	bridge *b;
	int acounter;
	int lstCntUpd;		// period of last counter update (to avoid multiple updates)
	int to_compute;
	netNode *node;		// pointer to network node data structure
	gisPosition *position; //Pointer to gis data structure
	void *cext;			// pointer to a C++ object extension to the LSD object

	double cal( object *caller,  char const *l, int lag );
	double cal( char const *l, int lag );
	void recal( char const *l );
	variable *search_var( object *caller, char const *label, bool no_error = false );
	object *search_var_cond( char const *lab, double value, int lag );
	double count( char const *lab );
	double count_all( char const *lab );
	double overall_max( char const *lab, int lag );
	double overall_min( char const *lab, int lag );
	double sum( char const *lab, int lag );
	double stat( char const *lab, double *v = NULL );
	double av( char const *lab, int lag );
	double whg_av( char const *lab, char const *lab2, int lag );
	double sd( char const *lab, int lag );
	int init( object *_up, char const *_label );
	void update( void );
	object *hyper_next( char const *lab );
	object *hyper_next( void );
	void add_var( char const *label, int lag, double *val, int save );
	void add_obj( char const *label, int num, int propagate );
	void insert_parent_obj_one( char const *lab );
	object *search( char const *lab );
	void chg_lab( char const *lab );
	void chg_var_lab( char const *old, char const *n );
	variable *add_empty_var( char const *str );
	void add_var_from_example( variable *example );
	void replicate( int num, int propagate );
	bool load_param( char *file_name, int repl, FILE *f );
	bool load_struct( FILE *f );
	void save_param( FILE *f );
	void save_struct( FILE *f, char const *tab );
	void sort_asc( object *from, char *l_var );
	void sort_desc( object *from, char *l_var );
	void sort( char const *obj, char const *var, char *direction );
	void lsdqsort( char const *obj, char const *var, char const *direction );
	void lsdqsort( char const *obj, char const *var1, char const *var2, char const *direction );
	void empty( void );
	void delete_obj( void );
	object *add_n_objects2( char const *lab, int n, object *ex, int t_update );
	object *add_n_objects2( char const *lab, int n, object *ex );
	object *add_n_objects2( char const *lab, int n, int t_update );
	object *add_n_objects2( char const *lab, int n );
	void write( char const *lab, double value, int time );//write value as if computed at time
	void write( char const *lab, double value, int time, int lag );//write value in the lag field
	object *draw_rnd( char const *lo, char const *lv, int lag );
	object *draw_rnd( char const *lo );
	object *draw_rnd( char const *lo, char const *lv, int lag, double tot );
	double increment( char const *lab, double value );
	double multiply( char const *lab, double value );
	object *lat_up( void );
	object *lat_down( void );
	object *lat_left( void );
	object *lat_right( void );
	double interact( char const *text, double v, double *tv );
	void initturbo( char const *label, double num );	// set the structure to use turbo search
	void emptyturbo( void );							// remove turbo search structure
	object *turbosearch( char const *label, double tot, double num );

	// set the network handling methods
	netNode *add_node_net( long id = -1, char const *nodeName = "", bool silent = false );
	void delete_node_net( void );
	void name_node_net( char const *nodeName );
	void stats_net( char const *lab, double *r );
	object *search_node_net( char const *lab, long id ); 
	object *draw_node_net( char const *lab ); 
	object *shuffle_nodes_net( char const *lab );
	netLink *add_link_net( object *destPtr, double weight = 0, double probTo = 1 );
	void delete_link_net( netLink *ptr );
	netLink *search_link_net( long id ); 
	netLink *draw_link_net( void ); 
	long read_file_net( char const *lab, char const *dir = "", char const *base_name = "net", int serial = 1, char const *ext = "net" );
	long write_file_net( char const *lab, char const *dir = "", char const *base_name = "net", int serial = 1, bool append = false );
	long init_stub_net( char const *lab, const char* gen, long numNodes, long par1 = 0, double par2 = 0.0 );
	long init_discon_net( char const *lab, long numNodes );
	long init_random_dir_net( char const *lab, long numNodes, long numLinks );
	long init_random_undir_net( char const *lab, long numNodes, long numLinks );
	long init_uniform_net( char const *lab, long numNodes, long outDeg );
	long init_star_net( char const *lab, long numNodes );
	long init_circle_net( char const *lab, long numNodes, long outDeg );
	long init_renyi_erdos_net( char const *lab, long numNodes, double linkProb );
	long init_small_world_net( char const *lab, long numNodes, long outDeg, double rho );
	long init_scale_free_net( char const *lab, long numNodes, long outDeg, double expLink );
	long init_lattice_net( int nRow, int nCol, char const *lab, int eightNeigbr );
	void delete_net( char const *lab );

	//set the new GIS handling methods
	double distance(object* other); //distance to other object
	double pseudo_distance(object* other); //pseudo distance to other object
	std::deque<object*>::iterator it_in_radius(char const lab[], double radius, bool random); //for the cycle in radius

	bool register_position(double _x, double _y);
	bool unregister_position(bool move);
	bool change_position(double _x, double _y);

	bool register_at_map(gisMap* map, double _x, double _y);
	bool unregister_from_gis();


	gisMap* ptr_map(); //get ptr to map, if gis object

	gisMap* init_map(int xn, int yn, int _wrap=0); //initialise a new map and register the ob to it.
	bool delete_map(); //delete the map, unregistering all gis-objects but leaving them otherwise untouched.

	bool init_gis_singleObj(object* gisObj, double _x, double _y, int xn, int yn, int _wrap=0); //Create a gis and add the object to it
	bool init_gis_regularGrid(char const lab[], int xn, int yn, int _wrap = 0); //Create a gis and add the objects to it, creating new ones if necessary.

};

struct lsdstack
{
	lsdstack *prev;
	lsdstack *next;
	char label[ MAX_ELEM_LENGTH ];
	int ns;
	variable *vs;
};

struct description
{
	char *label;
	char *type;
	char *text;
	char *init;
	char initial;
	char observe;
	description *next;
};

struct sense
{
	char *label;
	int param;						// save element type/lag to allow
	int lag;						// handling lags > 1
	int nvalues;
	int i;
	double *v;
	sense *next;
	bool entryOk;					// flag valid data entered
	bool integer;					// integer element
};

// design of experiment object
struct design 
{ 
	int typ, tab, n, k, *par, *lag;	// experiment parameters
	double *hi, *lo, **ptr; 
	char **lab;
	bool *intg;

	design( sense *rsens, int typ = 1, char const *fname = "", int findex = 1, 
			int samples = 0, int factors = 0, int jump = 2, int trajs = 4 );	// constructor
	~design( void );			// destructor
};

// results file object
class result
{
	FILE *f;					// uncompressed file pointer
#ifdef LIBZ
	gzFile fz;					// compressed file pointer
#endif
	bool dozip;					// compressed file flag
	bool docsv;					// comma separated .csv text format
	bool firstCol;				// flag for first column in line

	void title_recursive( object *r, int i );	// write file header (recursively)
	void data_recursive( object *r, int i );	// save a single time step (recursively)

public:
	result( char const *fname, char const *fmode, bool dozip = false, bool docsv = false );	// constructor
	~result( void );							// destructor
	void title( object *root, int flag );		// write file header
	void data( object *root, int initstep, int endtstep = 0 );	// write data
};

// profiled variable object
struct profile
{
	unsigned long long ticks;
	unsigned int comp;
	
	profile( ) { ticks = 0; comp = 0; };	// constructor
};

#ifdef PARALLEL_MODE
// multi-thread parallel worker data structure
struct worker
{
	bool running;
	bool free;
	bool user_excpt;
	char err_msg1[ TCL_BUFF_STR ];
	char err_msg2[ TCL_BUFF_STR ];
	char err_msg3[ TCL_BUFF_STR ];
	condition_variable run;
	exception_ptr pexcpt;
	int signum;
	mutex lock;
	thread thr;
	thread::id thr_id;
	variable *var;
	
	worker( void );								// constructor
	~worker( void );							// destructor
	bool check( void );							// handle worker problems
	void cal( variable *var );					// start worker calculation
	void cal_worker( void );					// worker thread code
	void signal( int signum );					// signal handler
	static void signal_wrapper( int signun );	// wrapper for signal_handler
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
double round( double r );
double save_lattice( const char fname[ ] = "lattice" );
double unifcdf( double a, double b, double x );			// uniform cumulative distribution function
double uniform( double min, double max );
double uniform_int( double min, double max );
double read_lattice( double line, double col );
double update_lattice( double line, double col, double val = 1 );
object *get_cycle_obj( object *c, char const *label, char const *command );
object *go_brother( object *c );
void close_lattice( void );
void deb_log( bool on, int time = 0 );					// control debug mode
void error_hard( const char *logText, const char *boxTitle = "", const char *boxText = "" );
void init_random( int seed );							// reset the random number generator seed
void msleep( unsigned msec = 1000 );					// sleep process for milliseconds
void nop( void );										// no operation
void plog( char const *msg, char const *tag = "", ... );
void results_alt_path( const char * );  				// change where results are saved.
void set_fast( int level );								// enable fast mode
long nodes2create( object *parent, char const *lab, long numNodes ); //calculate nodes missing

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
extern bool fast;				// flag to hide LOG messages & runtime (read-only)
extern bool invalidHooks;		// flag to invalid hooks pointers (set by simulation)
extern bool use_nan;			// flag to allow using Not a Number value
extern char *path;				// folder where the configuration is
extern char *simul_name;		// configuration name being run (for saving networks)
extern double def_res;			// default equation result
extern int cur_sim;
extern int debug_flag;
extern int fast_mode;
extern int max_step;
extern int quit;
extern int ran_gen;				// pseudo-random number generator to use (1-5) )
extern int seed;
extern int sim_num;
extern int t;
extern object *root;

#ifdef CPP11
extern eq_mapT eq_map;			// map to fast equation look-up
#endif

#ifndef NO_WINDOW
extern double i_values[ ];		// user temporary variables copy
extern Tcl_Interp *inter;		// Tcl standard interpreter pointer
#endif


// prevent exposing internals in users' fun_xxx.cpp
#ifndef FUN

// standalone internal C functions/procedures (not visible to the users)

FILE *create_frames( char *t );
FILE *search_data_ent( char *name, variable *v );
FILE *search_data_str( char const *name, char const *init, char const *str );
FILE *search_str( char const *name, char const *str );
bool alloc_save_mem( object *root );
bool discard_change( bool checkSense = true, bool senseOnly = false );	// ask before discarding unsaved changes
bool get_bool( const char *tcl_var, bool *var = NULL );
bool load_description( char *msg, FILE *f );
bool save_configuration( object *r, int findex = 0 );
bool save_sensitivity( FILE *f );
bool search_parallel( object *r );
bool unsaved_change( void );					// control for unsaved changes in configuration
bool unsaved_change( bool );
char *clean_file( char * );
char *clean_path( char * );
char *NOLH_valid_tables( int k, char* ch );
char *upload_eqfile( void );
description *search_description( char *lab );
double get_double( const char *tcl_var, double *var = NULL );
int browse( object *r, int *choice );
int check_label( char *l, object *r );
int compute_copyfrom( object *c, int *choice );
int contains ( FILE *f, char *lab, int len );
int deb( object *r, object *c, char const *lab, double *res, bool interact = false );
int get_int( const char *tcl_var, int *var = NULL );
int is_equation_header( char *line, char *var );
int load_configuration( object *, bool reload = false );
int load_sensitivity( object *r, FILE *f );
int lsdmain( int argn, char **argv );
int min_hborder( int *choice, int pdigits, double miny, double maxy );
int my_strcmp( char *a, char *b );
int num_sensitivity_variables( sense *rsens );	// calculates the number of variables to test
int reset_bridges( object *r );
int shrink_gnufile( void );
int sort_function_down( const void *a, const void *b );
int sort_function_down_two( const void *a, const void *b );
int sort_function_up( const void *a, const void *b );
int sort_function_up_two( const void *a, const void *b );
int sort_labels_down( const void *a, const void *b );
long get_long( const char *tcl_var, long *var = NULL );
long num_sensitivity_points( sense *rsens );	// calculates the sensitivity space size
object *check_net_struct( object *caller, char const *nodeLab, bool noErr = false );
object *create( object *r );
object *operate( int *choice, object *r );
object *restore_pos( object * );
object *sensitivity_parallel( object *o, sense *s );
object *skip_next_obj( object *t );
object *skip_next_obj( object *t, int *count );
void NOLH_clear( void );						// external DoE	cleanup
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
void collect_cemetery( object *o );				// collect variables from object before deletion
void control_tocompute(object *r, char *ch);
void copy_descendant( object *from, object *to );
void count( object *r, int *i );
void count_save( object *n, int *count );
void cover_browser( const char *, const char *, const char * );
void create_form( int num, char const *title, char const *prefix );
void create_initial_values( object *r );
void create_logwindow( void );
void create_maverag( int *choice );
void create_series( int *choice );
void create_table_init( object *r );
void dataentry_sensitivity( int *choice, sense *s, int nval );
void deb_show( object *r );
void delete_bridge( object *d );
void draw_obj( object *t, int level, int center, int from );
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
void go_next( object **t );
void handle_signals( void ( * handler )( int signum ) );
void histograms( int *choice );
void histograms_cs( int *choice );
void init_map( void );
void init_plot( int i, int id_sim );
void insert_data_file( bool gz, int *num_v, int *num_c );
void insert_data_mem( object *r, int *num_v, int *num_c );
void insert_labels_mem( object *r, int *num_v, int *num_c );
void insert_object( const char *w, object *r, bool netOnly = false );
void insert_obj_num( object *root, char const *tag, char const *indent, int counter, int *i, int *value );
void insert_store_mem( object *r, int *num_v );
void kill_trailing_newline( char *s );
void link_data( object *root, char *lab );
void load_configuration_failed( void );
void log_tcl_error( const char *cm, const char *message );
void myexit( int v );
void plog_series( int *choice );
void plot( int type, int *start, int *end, char **str, char **tag, int *choice, bool norm );
void plot( int type, int nv, double **data, int *start, int *end, char **str, char **tag, int *choice );
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
void put_line( int x1, int y1, int x2, int y2);
void put_node( int x1, int y1, int x2, int y2, char *str );
void put_text( char *str, char *num, int x, int y, char *str2);
void read_data( int *choice );
void read_eq_filename( char *s );
void report( int *choice, object *r );
void reset_end( object *r );
void reset_plot( int run );
void run( object *r );
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
void show_rep_initial( FILE *f, object *n, int *begin );
void show_rep_observe( FILE *f, object *n, int *begin );
void show_save( object *n );
void show_tmp_vars( bool update );
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
void unwind_stack( void );
void wipe_out( object *d );
void write_list( FILE *frep, object *root, int flag_all, char const *prefix );
void write_obj( object *r, FILE *frep );
void write_str( object *r, FILE *frep, int dep, char const *prefix );
void write_var( variable *v, FILE *frep );

#ifdef PARALLEL_MODE
void parallel_update( variable *v, object* p, object *caller = NULL );
#endif

// global internal variables (not visible to the users)

extern bool fast_lookup;	// flag for fast look-up mode
extern bool ignore_eq_file;	// control of configuration files equation updating
extern bool iniShowOnce;	// prevent repeating warning on # of columns
extern bool in_edit_data;	// in initial settings mode
extern bool in_set_obj;		// in setting number of objects mode
extern bool log_ok;			// control for log window available
extern bool message_logged;	// new message posted in log window
extern bool non_var;		// flag to indicate INTERACT macro condition
extern bool on_bar;			// flag to indicate bar is being draw in log window
extern bool parallel_mode;	// parallel mode (multithreading) status
extern bool redrawRoot;		// control for redrawing root window (.)
extern bool running;		// simulation is running
extern bool struct_loaded;	// a valid configuration file is loaded
extern bool unsavedData;	// control for unsaved simulation results
extern bool unsavedSense;	// control for unsaved changes in sensitivity data
extern bool user_exception;	// flag indicating exception was generated by user code
extern bool tk_ok;			// control for tk_ready to operate
extern char *eq_file;		// equation file content
extern char *exec_file;		// name of executable file
extern char *exec_path;		// path of executable file
extern char *sens_file;		// current sensitivity analysis file
extern char *struct_file;	// name of current configuration file
extern char equation_name[ ];// equation file name
extern char lsd_eq_file[ ];	// equations saved in configuration file
extern char msg[ ];			// auxiliary Tcl buffer
extern char name_rep[ ];	// documentation report file name
extern char nonavail[ ];	// string for unavailable values
extern description *descr;	// model description structure
extern double ymax;			// runtime plot max limit
extern double ymin;			// runtime plot min limit
extern int actual_steps;	// number of executed time steps
extern int add_to_tot;		// type of totals file generated (bool)
extern int choice;			// Tcl menu control variable (main window)
extern int choice_g;		// Tcl menu control variable ( structure window)
extern int cur_plt;			// current graph plot number
extern int docsv;			// produce .csv text results files (bool)
extern int dozip;			// compressed results file flag (bool)
extern int findexSens;		// index to sequential sensitivity configuration filenames
extern int log_start;		// first period to start logging to file, if any
extern int log_stop;		// last period to log to file, if any
extern int macro;			// equations style (macros or C++) (bool)
extern int max_threads;		// suggested maximum number of parallel threads 
extern int no_res;			// do not produce .res results files (bool)
extern int overwConf;		// overwrite current configuration file on run (bool)
extern int parallel_disable;// flag to control parallel mode
extern int prof_aggr_time;	// show aggregate profiling times
extern int prof_min_msecs;	// profile only variables taking more than X msecs.
extern int prof_obs_only;	// profile only observed variables
extern int saveConf;		// save configuration on results saving (bool)
extern int series_saved;	// number of series saved
extern int stack;			// LSD stack call level
extern int stack_info; 		// LSD stack control
extern int strWindowOn;		// control the presentation of the model structure window (bool)
extern int total_obj;		// total objects in model
extern int total_var;       // total variables/parameters in model
extern int watch;			// allow for graph generation interruption (bool)
extern int when_debug;      // next debug stop time step (0 for none )
extern int wr_warn_cnt;		// invalid write operations warning counter
extern long nodesSerial;	// network node serial number global counter
extern lsdstack *stacklog;	// LSD stack
extern object *blueprint;   // LSD blueprint (effective model in use )
extern sense *rsense;       // LSD sensitivity analysis structure
extern variable *cemetery;  // LSD saved data series (from last simulation run )
extern map< string, profile > prof;// set of saved profiling times
extern FILE *log_file;		// log file, if any

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
int Tcl_get_var_conf( ClientData cdata, Tcl_Interp *inter, int argc, const char *argv[ ] );
int Tcl_set_var_conf( ClientData cdata, Tcl_Interp *inter, int argc, const char *argv[ ] );
int Tcl_set_c_var( ClientData cdata, Tcl_Interp *inter, int argc, const char *argv[ ] );
int Tcl_upload_series( ClientData cd, Tcl_Interp *inter, int oc, Tcl_Obj *CONST ov[ ] );
#endif						// NO_WINDOW

#endif						// FUN
