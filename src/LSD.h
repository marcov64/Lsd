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
LSD.H
Global definitions shared by all LSD GUI modules.
*************************************************************/

// definitions from LSD library
#include "libLSD.h"

// standard libraries used
#include <list>

// LSD version strings, for About... boxes and code testing
#define _LSD_MAJOR_ 9
#define _LSD_MINOR_ 0
#define _LSD_VERSION_ "9.0"
#define _LSD_DATE_ "January 2 2024"	 // __DATE__

// platform codes
#define _LIN_	1
#define _MAC_	2
#define _WIN_	3

// date/time format
#define DATE_FMT "%d %B, %Y"

// access permissions in Linux/Mac
#ifndef ACCESSPERMS
#define ACCESSPERMS 0777
#endif

// global constants
#define DEF_CONF_FILE "Sim1"			// default new configuration name
#define FILE_BUF_SIZE 1000000			// buffer size for file reading
#define MARG 0.01						// y-axis % plot clearance margin
#define MARG_CONST 0.1					// y-axis % plot clearance margin for constant series
#define MAX_CELS 9000					// max number of cells (rows x columns) in init. editor
#define MAX_COLS 100					// max numbers of columns in init. editor
#define MAX_LEVEL 10					// maximum number of object levels (plotting only)
#define MAX_PLOTS 1000					// max numbers of plots in analysis
#define MAX_PLOT_TABS 10				// max number of plot tabs to show
#define MAX_SENS_POINTS 999				// default warning threshold for sensitivity analysis
#define MAX_TAB_LEN 10					// max length of plot tab title
#define NOLH_DEF_FILE "NOLH.csv"		// default NOLH file name
#define PROG_SERIES 10000				// AoR progress bar when loading series limit
#define SENS_SEP " ,;|/#\t\n"			// sensitivity data valid separators
#define SIG_MIN 1e-100					// Minimum significant value (different than zero)
#define SRV_MAX_CORES 64				// maximum number of cores to use in a server
#define SRV_MIN_CORES 12				// minimum number of cores to consider a server

// configuration files details
#define LMM_OPTIONS "lmm_options.txt"
#define SYSTEM_OPTIONS "system_options.txt"
#define MODEL_OPTIONS "model_options.txt"
#define GROUP_INFO "groupinfo.txt"
#define MODEL_INFO "modelinfo.txt"
#define DESCRIPTION "description.txt"

// Special file names/locations in Windows
#define TCL_LIB_VAR		"TCL_LIBRARY"
#define TCL_LIB_PATH	"gnu/lib/tcl8.6"// must NOT use backslashes
#define TCL_LIB_INIT	"init.tcl"
#define TCL_EXEC_PATH	"gnu\\bin"		// must use (double) backslashes
#define TCL_FIND_EXE	"@where wish86.exe > nul 2>&1"

// Eigen library include command
#define EIGEN "#define EIGENLIB"

// define meta-parameter names for LWI getlimits
#define META_PAR_NUM 3
#define META_PAR_NAME { "_timeSteps_", "_numRuns_", "_rndSeed_" }

// constant string arrays
#define LMM_OPTIONS_NUM 16
#define LMM_OPTIONS_NAME { "sysTerm", "HtmlBrowser", "fonttype", \
						   "wish", "LsdSrc", "dim_character", \
						   "tabsize", "wrap", "shigh", \
						   "autoHide", "showFileCmds", "LsdNew", \
						   "DbgExe", "restoreWin", "lmmGeom", \
						   "lsdTheme" }
#define LMM_OPTIONS_DEFAULT { "$DefaultSysTerm", "$DefaultHtmlBrowser", \
							  "$DefaultFont", "$DefaultWish", "src", \
							  "$DefaultFontSize", "4", "1", "2", "0", "0", \
							  "Work", "$DefaultDbgExe", "1", "#", \
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
#define LSD_NW_NUM 5
#define LSD_NW_SRC { "lsdnw.cpp", "libLSD.h", "check.h", "fun_head.h", \
					 "fun_head_fast.h" }
#define LSD_DIR_NUM 8
#define LSD_DIR_NAME { "src", "gnu", "installer", "Manual", "LMM.app", "Rpkg", \
					   "lwi", "___" }
#define LSD_MIN_NUM 3
#define LSD_MIN_FILES { "src/icons", "src/themes", "src/interf.cpp", \
						"src/analysis.cpp" }
#define WIN_COMP_NUM 2
#define WIN_COMP_PATH { "mingw64\\bin", "cygwin64\\bin" }	// must use (double) backslashes
#define LSD_WIN_NUM MODEL_INFO_NUM - 3
#define LSD_WIN_NAME { "lsd", "log", "str", "da", "deb", "lat", "plt", "dap" }

// standalone internal C functions (not visible to the users)
bool abort_run_threads( void );
bool add_rt_plot_tab( const char *w, int id_sim );
bool add_unsaved( void );
bool check_res_dir( const char *path, const char *sim_name = NULL );
bool compile_run( int run_mode, bool nw = false );
bool contains( FILE *f, const char *lab, int len );
bool create_maverag( void );
bool create_res_dir( const char *path );
bool create_series( bool mc, vector < string > var_names );
bool eval_bool( const char *tcl_exp );
bool exists_var( const char *lab );
bool exists_window( const char *lab );
bool expr_eq( const char *tcl_exp, const char *c_str );
bool get_bool( const char *tcl_var, bool *var = NULL );
bool is_equation_header( const char *line, char *var, char *updt_in );
bool load_lmm_options( void );
bool load_model_info( const char *path );
bool load_prev_configuration( void );
bool make_no_window( void );
bool need_res_dir( const char *path, const char *sim_name, char *buf, int buf_sz );
bool open_configuration( object *&r, bool reload );
bool save_configuration( const char *path, const char *rname, const char *ext );
bool save_sensitivity( FILE *f );
bool save_xml_configuration( int findex = 0, const char *dest_path = NULL, bool quick = false );
bool sensitivity_clean_dir( const char *path );
bool sensitivity_too_large( long numSaPts );
bool set_env( bool set );
bool sort_listbox( int box, int order, object *r );
bool unsaved_change( bool );
bool unsaved_change( void );
bool use_eigen( void );
char *eval_str( const char *tcl_exp, char *var, int var_size );
char *fmt_ttip_descr( char *out, description *d, int outSz, bool init = true );
char *get_str( const char *tcl_var, char *var, int var_size );
char *load_eqfile( void );
char *search_lsd_root( char *buf, int bufSz );
char *strencdata( char *out, const char *in, int outSz = 0 );
char *strtcl( char *out, const char *text, int outSz );
char *strupr( char *s );
char *NOLH_valid_tables( int k, char *out, int sz );
const char *eval_str( const char *tcl_exp );
const char *get_fun_name( char *str, int str_sz, bool nw = false );
const char *get_str( const char *tcl_var );
const char *get_target_name( char *str, int str_sz, bool nw = false );
double eval_double( const char *tcl_exp );
double get_double( const char *tcl_var, double *var = NULL );
double lower_bound( double a, double b, double marg, double marg_eq, int dig = 16 );
double save_lattice_helper( const char *fname );
double strtod( const char *in, char** endptr, double inv );
double update_lattice_helper( double line, double col, double val, int line_int, int col_int, int val_int );
double upper_bound( double a, double b, double marg, double marg_eq, int dig = 16 );
double *log_data( double *data, int start, int end, int ser, const char *err_msg );
int browse( object *r );
int check_affected( object *c, int level, int affected[ ] );
int check_label( const char *lab, object *r );
int compute_copyfrom( object *c, const char *parWnd );
int count_lines( const char *fname, bool dozip = false );
int deb( object *r, object *c, const char *lab, double *res, bool interact = false, const char *hl_var = "" );
int entry_new_objnum( object *c, const char *tag );
int eval_int( const char *tcl_exp );
int get_int( const char *tcl_var, int *var = NULL );
int load_gui( const char **argv );
int load_sensitivity( FILE *f );
int min_hborder( int pdigits, double miny, double maxy );
int modman( int argn, const char **argv );
int num_sensitivity_variables( void );
int shrink_gnufile( void );
int strwrap( char *out, const char *str, int outSz, int wid );
int uniform_int_0( int max );
int Tcl_abort_run_threads( ClientData cdata, Tcl_Interp *interp, int argc, const char *argv[ ] );
int Tcl_discard_change( ClientData cdata, Tcl_Interp *interp, int argc, const char *argv[ ] );
int Tcl_get_obj_conf( ClientData cdata, Tcl_Interp *interp, int argc, const char *argv[ ] );
int Tcl_get_var_conf( ClientData cdata, Tcl_Interp *interp, int argc, const char *argv[ ] );
int Tcl_get_var_descr( ClientData cdata, Tcl_Interp *interp, int argc, const char *argv[ ] );
int Tcl_log_tcl_error( ClientData cdata, Tcl_Interp *interp, int argc, const char *argv[ ] );
int Tcl_set_c_var( ClientData cdata, Tcl_Interp *interp, int argc, const char *argv[ ] );
int Tcl_set_obj_conf( ClientData cdata, Tcl_Interp *interp, int argc, const char *argv[ ] );
int Tcl_set_ttip_descr( ClientData cdata, Tcl_Interp *interp, int argc, const char *argv[ ] );
int Tcl_set_var_conf( ClientData cdata, Tcl_Interp *interp, int argc, const char *argv[ ] );
int Tcl_upload_series( ClientData cd, Tcl_Interp *interp, int oc, Tcl_Obj *CONST ov[ ] );
long eval_long( const char *tcl_exp );
long get_long( const char *tcl_var, long *var = NULL );
long num_sensitivity_points( void );
object *operate( object *r );
object *restore_pos( object * );
object *sensitivity_parallel( object *o, sense *s );
sense *search_sensitivity( const char *lab, int lag = 0 );
string to_string( const char *fmt, double val );
string win_path( string filepath );
void add_da_plot_tab( const char *w, int id_plot );
void analysis( bool mc = false );
void ancestors( object *r, FILE *f, bool html = true );
void assign( object *r, int *idx, const char *lab );
void attach_instance_number( char *outh, char *outv, object *r, int outSz );
void auto_document( const char *lab, const char *which, bool append = false );
void canvas_binds( int n );
void center_plot( void );
void check_option_files( bool sys = false );
void chg_obj_num( object **c, int value, int all, int pippo[ ], int cfrom );
void clean_debug( object *n );
void clean_parallel( object *n );
void clean_plot( object *n );
void clean_res_dir( const char *path, const char *sim_name = NULL );
void clean_save( object *n );
void clean_spaces( char *s );
void cmd( const char *cm, ... );
void cmd_backend( const char *cm, va_list arg );
void cmd_gui( const char *cm, ... );
void control_to_compute( object *r, const char *lab );
void count( object *r, int *i );
void count_labels_mem( object *r, int *count, const char *lab = NULL );
void count_save( object *n, int *count );
void cover_browser( const char *text1, const char *text2, bool run );
void create( void );
void create_float_list( object *t );
void create_form( int num, const char *title, const char *prefix, FILE *frep );
void create_initial_values( object *r, FILE *frep );
void create_logwindow( void );
void create_par_map( object *r );
void create_table_init( object *r, FILE *frep );
void deb_show( object *r, const char *hl_var, int mode );
void disable_plot( void );
void draw_buttons( void );
void draw_obj( object *t, object *sel, int level = 0, int center = 0, int from = 0, bool zeroinst = false );
void edit_data( object *root, const char *obj_name );
void edit_str( object *r, const char *tag, int *idx, int res, int *done );
void eliminate_obj( object **c, int actual, int desired );
void enable_plot( void );
void fill_list_par( object *r, bool show_all );
void fill_list_var( object *r, bool show_all, bool lag_only );
void find_lags( object *r );
void find_using( object *r, variable *v, FILE *frep, bool *found );
void get_sa_limits( object *r, FILE *out, const char *sep );
void get_saved( object *n, FILE *out, const char *sep, bool all_var = false );
void get_var_descr( const char *lab, char *desc, int descr_len );
void histograms( void );
void histograms_cs( void );
void init_lattice_helper( double pixW, double pixH, double nrow, double ncol, int init_color );
void init_plot( int i );
void init_tcl_tk( const char *exec, const char *tcl_app_name );
void insert_data_file( bool gz, int *num_v, vector < string > *var_names, bool keep_vars );
void insert_data_mem( object *r, int *num_v, const char *lab = NULL );
void insert_labels_mem( object *r, int *num_v, const char *lab = NULL );
void insert_obj_num( object *r, const char *tag, const char *ind, int *idx, int *count );
void insert_object( const char *w, object *r, bool netOnly = false, object *above = NULL );
void insert_store_mem( object *r, int max_v, int *num_v, const char *lab = NULL );
void link_cells( object *root, const char *lab );
void load_elem_lists( object *r );
void log_tcl_error( bool show, const char *cm, const char *message, ... );
void lsd_exit_gui( int v );
void make_makefile( bool nw = false );
void plog_backend( const char *cm, const char *tag, va_list arg );
void plog_series( void );
void plot( int type, const int *start, const int *end, char **str, char **tag, bool norm );
void plot( int type, int nv, double **data, const int *start, const int *end, const int *id, char **str, char **tag );
void plot_canvas( int type, int nv, const int *start, const int *end, char **str, char **tag );
void plot_cross( void );
void plot_cs_xy( void );
void plot_gnu( void );
void plot_lattice( void );
void plot_phase_diagram( void );
void plot_rt( variable *var );
void plot_tseries( void );
void prepare_plot( object *r, int id_sim );
void print_stack( void );
void put_line( int x1, int y1, int x2 );
void put_node( int x, int y, const char *str, bool sel );
void put_text( const char *str, const char *num, int x, int y, const char *str2 );
void read_eqfile_name( char *s, int sz );
void report( object *r );
void reset_plot( void );
void return_where_used( char *lab, char *s, int sz );
void save_cells( object *r, const char *lab );
void save_data1( void );
void save_datazip( void );
void save_description( object *r, FILE *f );
void save_eqfile( FILE *f );
void save_pos( object * );
void scan_used_lab( const char *lab, const char *parWnd = NULL );
void scan_using_lab( const char *lab, const char *parWnd = NULL );
void scroll_plot( void );
void search_title( object *r, const char *tag, int *idx, const char *lab, int *cols );
void sensitivity_created( const char *path, const char *sim_name, int findex );
void sensitivity_doe( int *findex, design *doe, const char *dest_path );
void sensitivity_sequential( int *findexSens, sense *s, double probSampl, const char *dest_path );
void sensitivity_undefined( void );
void set_all( object *original, const char *lab, int lag, const char *parWnd = NULL );
void set_buttons_run( bool enable );
void set_cs_data( void );
void set_obj_number( object *r );
void set_shortcuts( const char *window );
void set_shortcuts_run( const char *window );
void set_title( object *c, const char *lab, const char *tag, int *cols );
void set_ttip_descr( const char *w, const char *lab, int it = -1, bool init = true );
void shift_desc( int direction, const char *dlab, object *r );
void shift_var( int direction, const char *vlab, object *r );
void show_cells( object *r, const char *lab );
void show_comp_result( bool nw = false );
void show_debug( object *n );
void show_descr( const char *lab, const char *parWnd = NULL );
void show_eq( const char *lab, const char *parWnd = NULL );
void show_graph( object *t = NULL );
void show_initial( object *n );
void show_logs( const char *path, vector < string > & logs, bool par_cntl = false );
void show_neighbors( object *r, bool update );
void show_observe( object *n );
void show_parallel( object *n );
void show_plot( object *n );
void show_plot_gnu( int n, int type, char **str, char **tag );
void show_prof_aggr( void );
void show_rep_initial( FILE *f, object *n, int *begin, FILE *frep );
void show_rep_observe( FILE *f, object *n, int *begin, FILE *frep );
void show_report( const char *par_wnd );
void show_save( object *n );
void show_special_updat( object *n );
void show_tcl_error( const char *boxTitle, const char *errMsg, ... );
void show_tmp_vars( object *r, bool update );
void sort_cs_asc( char **s, char **t, double **v, int nv, int nt, int c );
void sort_cs_desc( char **s, char **t, double **v, int nv, int nt, int c );
void statistics( void );
void statistics_cross( void );
void tex_report_end( FILE *f );
void tex_report_head( FILE *f, bool table = true );
void tex_report_init( object *r, FILE *f, bool table = true );
void tex_report_initall( object *r, FILE *f, bool table = true );
void tex_report_observe( object *r, FILE *f, bool table = true );
void tex_report_struct( object *r, FILE *f, bool table = true );
void uncover_browser( void );
void unlink_cells( object *r, const char *lab );
void unload_configuration_gui( bool full );
void unset_shortcuts_run( const char *window );
void update_bounds( void );
void update_descr_dict( void );
void update_lmm_options( bool justLmmGeom = false );
void update_model_info( bool fix = false );
void update_more_tab( bool adding = false );
void wipe_out( object *d );
void write_list( FILE *frep, object *root, bool show_all, const char *prefix );
void write_obj( object *r, FILE *frep, int *elemDone );
void write_str( object *r, FILE *frep, int dep, const char *prefix );
void write_var( object *r, variable *v, FILE *frep );
void NOLH_clear( void );
FILE *create_frames( const char *path, const char *fname );
FILE *search_all_sources( char *str );

#ifdef _LMM_
bool discard_change( void );
#else
bool discard_change( bool checkSense = true, bool senseOnly = false, const char title[ ] = "" );
#endif

// global internal variables (not visible to the users)
extern bool brCovered;			// browser cover currently covered
extern bool check_save;			// control saving message inside disabled objects
extern bool eq_dum;				// current equation is dummy
extern bool ignore_eq_file;		// control of configuration files equation updating
extern bool log_ok;				// control for log window available
extern bool meta_par_in[ ];		// flag meta variables for simulation settings found
extern bool redrawRoot;			// control for redrawing root window (.)
extern bool redrawStruc;		// control for redrawing model structure window
extern bool redrawReq;			// flag for asynchronous window redraw request
extern bool tk_ok;				// control for tk_ready to operate
extern char *rootLsd;			// path of LSD root directory
extern char err_file[ ];		// error log file name
extern char path_sens[ ];		// path of last used sensitivity directory
extern char tcl_dir[ ];			// Tcl/Tk directory
extern const char *res_g;		// structure window result variable
extern int choice_g;			// Tcl menu control variable ( structure window)
extern int doover;				// overwrite results folder (bool)
extern int elem_count;			// recursive element counter for show elements menu
extern int macro;				// equations style (macros or C++) (bool)
extern int overwConf;			// overwrite current configuration file on run (bool)
extern int saveConf;			// save configuration on results saving (bool)
extern int stop;				// activity interruption flag (Tcl boolean)
extern int strWindowOn;			// control the presentation of the model structure window (bool)
extern object *currObj;			// pointer to current object in browser
extern Tcl_Interp *interp;		// Tcl standard interpreter pointer

// common constant string arrays (not visible to the users)
extern const char *lmm_options[ ];
extern const char *lmm_defaults[ ];
extern const char *model_info[ ];
extern const char *model_defaults[ ];
extern const char *wnd_names[ ];// LSD main windows' names
