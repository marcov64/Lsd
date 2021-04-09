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
DECL.H
Global definitions among all LSD C++ modules

Relevant flags (when defined):

- FUN: user model equation file
- NW: No Window executable
- NP: no parallel (multi-task) processing
- NT: no signal trapping (better when debugging in GDB)
*************************************************************/

// common definitions for LMM and LSD
#include "common.h"

// standard libraries used
#include <cstdarg>
#include <cctype>
#include <cfloat>
#include <limits>
#include <algorithm>
#include <random>
#include <chrono>
#include <string>
#include <list>
#include <map>
#include <set>
#include <atomic>
#include <exception>

// global constants
#define DEF_CONF_FILE "Sim1"			// default new configuration name
#define NOLH_DEF_FILE "NOLH.csv"		// default NOLH file name
#define MAX_SENS_POINTS 999				// default warning threshold for sensitivity analysis
#define MAX_COLS 100					// max numbers of columns in init. editor
#define MAX_PLOTS 1000					// max numbers of plots in analysis
#define MAX_PLOT_TABS 10				// max number of plot tabs to show
#define MAX_TAB_LEN 10					// max length of plot tab title
#define MAX_CORES 0						// maximum number of cores to use (0=auto )
#define SRV_MIN_CORES 12				// minimum number of cores to consider a server
#define SRV_MAX_CORES 64				// maximum number of cores to use in a server
#define MAX_WAIT_TIME 10				// maximum wait time for a variable computation ( sec.)
#define MAX_TIMEOUT 100					// maximum timeout for multi-thread scheduler (millisec.)
#define MAX_LEVEL 10					// maximum number of object levels (plotting only)
#define MAX_OBJ_CHK	10000000			// maximum number of objects to check when searching
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

// access permissions in Linux/Mac
#ifndef ACCESSPERMS
#define ACCESSPERMS 0777 
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
double ran1( long *unused = 0 );
double round( double r );
double round_digits( double value, int digits );
double save_lattice( const char fname[ ] = "lattice" );
double student( double n );								// draw from a Student-T distribution
double unifcdf( double a, double b, double x );			// uniform cumulative distribution function
double uniform( double min, double max );
double uniform_int( double min, double max );
double update_lattice( double line, double col, double val = 1 );
double weibull( double a, double b );					// draw from a Weibull distribution
void close_lattice( void );
void deb_log( bool on, int time = 0 );					// control debug mode
void error_hard( const char *logText, const char *boxTitle = "", const char *boxText = "", bool defQuit = false );
void init_random( unsigned seed );						// reset the random number generator seed
void results_alt_path( const char * );  				// change where results are saved.
void set_fast( int level );								// enable fast mode
void *set_random( int gen );							// set random generator


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
extern eq_mapT eq_map;					// map to fast equation look-up
extern int cur_sim;
extern int debug_flag;
extern int max_step;
extern int sim_num;
extern int t;
extern unsigned seed;
extern object *root;

#ifndef NW
extern int i_values[ ];					// user temporary variables copy
extern double d_values[ ];
extern object *o_values[ ];
extern netLink *n_values[ ];
#endif


// prevent exposing internals in users' fun_xxx.cpp
#ifndef FUN

// standalone internal C functions/procedures (not visible to the users)
FILE *create_frames( const char *path, const char *fname );
FILE *search_data_ent( char *name, variable *v );
FILE *search_data_str( char const *name, char const *init, char const *str );
FILE *search_str( char const *name, char const *str );
bool add_unsaved( int *choice );
bool alloc_save_mem( object *r );
bool alloc_save_var( variable *v );
bool check_cond( double val1, int lopc, double val2 );
bool contains( FILE *f, char *lab, int len );
bool create_maverag( int *choice );
bool create_series( int *choice, bool mc, vector < string > var_names );
bool is_equation_header( char *line, char *var, char *updt_in );
bool load_description( char *msg, FILE *f );
bool load_prev_configuration( void );
bool open_configuration( object *&r, bool reload );
bool save_configuration( int findex = 0 );
bool save_sensitivity( FILE *f );
bool search_parallel( object *r );
bool sensitivity_too_large( long numSaPts, int *choice );
bool sort_listbox( int box, int order, object *r );
bool unsaved_change( bool );
bool unsaved_change( void );
char *NOLH_valid_tables( int k, char* ch );
char *upload_eqfile( void );
description *search_description( char *lab );
double lower_bound( double a, double b, double marg, double marg_eq, int dig = 16 );
double upper_bound( double a, double b, double marg, double marg_eq, int dig = 16 );
double *log_data( double *data, int start, int end, int ser, const char *err_msg );
int browse( object *r, int *choice );
int check_label( char *l, object *r );
int check_affected( object *c, object *pivot, int level, int affected[ ] );
int compute_copyfrom( object *c, int *choice, const char *parWnd );
int entry_new_objnum( object *c, const char *tag, int *choice );
int hyper_count( char const *lab );
int load_configuration( bool reload, bool quick = false );
int load_sensitivity( FILE *f );
int logic_op_code( char const *lop, char const *errmsg );
int min_hborder( int *choice, int pdigits, double miny, double maxy );
int num_sensitivity_variables( sense *rsens );
int rnd_int( int min, int max );
int shrink_gnufile( void );
int uniform_int_0( int max );
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
void add_da_plot_tab( const char *w, int id_plot );
void add_description( char const *lab, char const *type, char const *text );
void add_rt_plot_tab( const char *w, int id_sim );
void analysis( int *choice, bool mc = false );
void ancestors( object *r, FILE *f, bool html = true );
void assign( object *r, int *idx, char *lab );
void attach_instance_number( char *ch, object *r );
void auto_document( int *choice, char const *lab, char const *which, bool append = false );
void autofill_descr( object *o );
void canvas_binds( int n );
void center_plot( void );
void change_descr_lab( char const *lab_old, char const *lab, char const *type, char const *text, char const *init );
void change_descr_lab( char const *lab_old, char const *lab, char const *type, char const *text, char const *init );
void change_descr_text( char *lab );
void change_init_text( char *lab );
void chg_obj_num( object **c, int value, int all, int pippo[ ], int *choice, int cfrom );
void clean_debug( object *n );
void clean_parallel( object *n );
void clean_plot( object *n );
void clean_save( object *n );
void close_sim( void );
void collect_inst( object *r, o_setT &list );
void control_tocompute(object *r, char *ch);
void copy_descendant( object *from, object *to );
void count( object *r, int *i );
void count_save( object *n, int *count );
void cover_browser( const char *text1, const char *text2, bool run );
void create( void );
void create_form( int num, char const *title, char const *prefix, FILE *frep );
void create_initial_values( object *r, FILE *frep );
void create_logwindow( void );
void create_par_map( object *r );
void create_table_init( object *r, FILE *frep );
void dataentry_sensitivity( int *choice, sense *s, int nval = 0 );
void deb_show( object *r, const char *hl_var );
void delete_bridge( object *d );
void disable_plot( void );
void draw_buttons( void );
void draw_obj( object *t, object *sel, int level = 0, int center = 0, int from = 0, bool zeroinst = false );
void edit_data( object *root, int *choice, char *obj_name );
void edit_str( object *r, const char *tag, int *idx, int res, int *choice, int *done );
void eliminate_obj( object **c, int actual, int desired , int *choice );
void empty_blueprint( void );
void empty_cemetery( void );
void empty_description( void );
void empty_lattice( void );
void empty_sensitivity( sense *cs );
void empty_stack( void );
void enable_plot( void );
void file_name( char *name );
void fill_list_par( object *r, int flag_all );
void fill_list_var( object *r, int flag_all, int flag_init );
void find_lags( object *r );
void find_using( object *r, variable *v, FILE *frep, bool *found );
void get_sa_limits( object *r, FILE *out, const char *sep );
void get_saved( object *n, FILE *out, const char *sep, bool all_var = false );
void get_var_descr( char const *lab, char *descr, int descr_len );
void histograms( int *choice );
void histograms_cs( int *choice );
void init_map( void );
void init_math_error( void );
void init_plot( int i, int id_sim );
void insert_data_file( bool gz, int *num_v, vector < string > *var_names, bool keep_vars );
void insert_data_mem( object *r, int *num_v, char *lab = NULL );
void insert_labels_mem( object *r, int *num_v, char *lab = NULL );
void insert_obj_num( object *r, const char *tag, const char *ind, int *idx, int *count );
void insert_object( const char *w, object *r, bool netOnly = false, object *above = NULL );
void insert_store_mem( object *r, int *num_v, char *lab = NULL );
void link_cells( object *root, char *lab );
void move_obj( char const *lab, char const *dest );
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
void put_line( int x1, int y1, int x2 );
void put_node( int x, int y, char *str, bool sel );
void put_text( char *str, char *num, int x, int y, char *str2);
void read_eq_filename( char *s );
void report( int *choice, object *r );
void reset_blueprint( object *r );
void reset_end( object *r );
void reset_plot( void );
void run( void );
void save_cells( object *r, char *lab );
void save_data1( int *choice );
void save_datazip( int *choice );
void save_eqfile( FILE *f );
void save_pos( object * );
void save_single( variable *v );
void scan_used_lab( char *lab, int *choice );
void scan_using_lab( char *lab, int *choice );
void scroll_plot( void );
void search_title( object *r, char *tag, int *idx, char *lab, int *incr );
void sensitivity_created( void );
void sensitivity_doe( int *findex, design *doe );
void sensitivity_sequential( int *findexSens, sense *s, double probSampl = 1.0 );
void sensitivity_undefined( void );
void set_all( int *choice, object *original, char *lab, int lag );
void set_blueprint( object *container, object *r );
void set_buttons_run( bool enable );
void set_cs_data( int *choice );
void set_lab_tit( variable *var );
void set_obj_number( object *r, int *choice );
void set_shortcuts( const char *window );
void set_shortcuts_run( const char *window );
void set_title( object *c, char *lab, char *tag, int *incr );
void shift_desc( int direction, char *dlab, object *r );
void shift_var( int direction, char *vlab, object *r );
void show_cells( object *r, char *lab );
void show_debug( object *n );
void show_descr( char *lab, int *choice );
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
void show_report( int *choice, const char *par_wnd );
void show_save( object *n );
void show_special_updat( object *n );
void show_tmp_vars( object *r, bool update );
void sort_cs_asc( char **s,char **t, double **v, int nv, int nt, int c );
void sort_cs_desc( char **s,char **t, double **v, int nv, int nt, int c );
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
void unlink_cells( object *r, char *lab );
void unset_shortcuts_run( const char *window );
void update_bounds( void );
void update_more_tab( const char *w, bool adding = false );
void warn_distr( int *errCnt, bool *stopErr, const char *distr, const char *msg );
void wipe_out( object *d );
void write_list( FILE *frep, object *root, int flag_all, char const *prefix );
void write_obj( object *r, FILE *frep, int *elemDone );
void write_str( object *r, FILE *frep, int dep, char const *prefix );
void write_var( variable *v, FILE *frep );

#ifndef NP
void parallel_update( variable *v, object* p, object *caller = NULL );
#endif

// global internal variables (not visible to the users)
extern FILE *log_file;			// log file, if any
extern bool brCovered;			// browser cover currently covered
extern bool eq_dum;				// current equation is dummy
extern bool error_hard_thread;	// flag to error_hard() called in worker thread
extern bool ignore_eq_file;		// control of configuration files equation updating
extern bool iniShowOnce;		// prevent repeating warning on # of columns
extern bool log_ok;				// control for log window available
extern bool message_logged;		// new message posted in log window
extern bool non_var;			// flag to indicate INTERACT macro condition
extern bool redrawRoot;			// control for redrawing root window (.)
extern bool redrawStruc;		// control for redrawing model structure window
extern bool running;			// simulation is running
extern bool scrollB;			// scroll check box state in current runtime plot
extern bool struct_loaded;		// a valid configuration file is loaded
extern bool unsavedData;		// control for unsaved simulation results
extern bool unsavedSense;		// control for unsaved changes in sensitivity data
extern bool worker_ready;		// parallel worker ready flag
extern bool worker_crashed;		// parallel worker crash flag
extern char *eq_file;			// equation file content
extern char *exec_file;			// name of executable file
extern char *sens_file;			// current sensitivity analysis file
extern char *struct_file;		// name of current configuration file
extern char error_hard_msg1[ ];	// buffer for parallel worker title msg
extern char error_hard_msg2[ ];	// buffer for parallel worker log msg
extern char error_hard_msg3[ ];	// buffer for parallel worker box msg
extern char lastObj[ ];			// last shown object for quick reload
extern char lsd_eq_file[ ];		// equations saved in configuration file
extern char name_rep[ ];		// documentation report file name
extern char nonavail[ ];		// string for unavailable values
extern description *descr;		// model description structure
extern double ymax;				// runtime plot max limit
extern double ymin;				// runtime plot min limit
extern int actual_steps;		// number of executed time steps
extern int add_to_tot;			// type of totals file generated (bool)
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
extern map< string, profile > prof;// set of saved profiling times
extern mt19937 mt32;			// Mersenne-Twister 32 bits generator
extern object *blueprint;   	// LSD blueprint (effective model in use )
extern object *currObj;			// pointer to current object in browser
extern object *wait_delete;		// LSD object waiting for deletion
extern o_setT obj_list;			// list with all existing LSD objects
extern sense *rsense;       	// LSD sensitivity analysis structure
extern variable *cemetery;  	// LSD saved data from deleted objects
extern variable *last_cemetery;	// LSD last saved data from deleted objects
extern void *random_engine;		// current random number generator engine

// multi-threading control 
#ifndef NP
extern atomic< bool > parallel_ready;	// flag to indicate multitasking is available
extern map< thread::id, worker * > thr_ptr;	// worker thread pointers
#endif

// Tcl/Tk specific definitions (for the windowed version only)
#ifndef NW

extern p_mapT par_map;			// variable to parent name map for AoR

// C to TCL interface functions
int Tcl_get_obj_conf( ClientData cdata, Tcl_Interp *inter, int argc, const char *argv[ ] );
int Tcl_set_obj_conf( ClientData cdata, Tcl_Interp *inter, int argc, const char *argv[ ] );
int Tcl_get_var_conf( ClientData cdata, Tcl_Interp *inter, int argc, const char *argv[ ] );
int Tcl_set_var_conf( ClientData cdata, Tcl_Interp *inter, int argc, const char *argv[ ] );
int Tcl_set_c_var( ClientData cdata, Tcl_Interp *inter, int argc, const char *argv[ ] );
int Tcl_get_var_descr( ClientData cdata, Tcl_Interp *inter, int argc, const char *argv[ ] );
int Tcl_upload_series( ClientData cd, Tcl_Interp *inter, int oc, Tcl_Obj *CONST ov[ ] );

#endif

#endif
