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

// base class extensions
#define SIMULATION_EXT \
	public: \
	int num_sensitivity_variables( void ); \
	long num_sensitivity_points( void ); \
	sensitivity *search_sensitivity( const char *lab, int lag = 0 );

#define OBJECT_EXT \
	public: \
	bool sort_listbox( int box, int order ); \
	int check_affected( int level, int affected[ ] ); \
	int check_label( const char *lab ); \
	int compute_copyfrom( const char *parWnd ); \
	int debugger( object *c, const char *lab, double *res, bool interact = false, const char *hl_var = "" ); \
	object *restore_pos( void ); \
	object *sensitivity_parallel( sensitivity *s ); \
	void clean_debug( void ); \
	void clean_parallel( void ); \
	void clean_plot( void ); \
	void clean_save( void ); \
	void control_to_compute( void ); \
	void count_save( int *count ); \
	void create_par_map( void ); \
	void edit_data( const char *lab ); \
	void find_using( variable *v, FILE *frep, bool *found ); \
	void get_saved( FILE *out, const char *sep, bool all_var = false ); \
	void get_sa_limits( FILE *out, const char *sep ); \
	void insert_data_mem( int *num_v, const char *lab = NULL ); \
	void insert_object( const char *w, bool netOnly = false, object *above = NULL ); \
	void load_elem_lists( void ); \
	void prepare_plot( int id_sim ); \
	void report( void ); \
	void save_pos( void ); \
	void set_all( const char *lab, int lag, const char *parWnd = NULL ); \
	void set_obj_number( void ); \
	void shift_desc( int direction, const char *dlab ); \
	void shift_var( int direction, const char *vlab ); \
	void show_debug( void ); \
	void show_graph( void ); \
	void show_initial( void ); \
	void show_observe( void ); \
	void show_parallel( void ); \
	void show_plot( void ); \
	void show_save( void ); \
	void show_special_updat( void ); \
	void tex_report_end( FILE *f ); \
	void tex_report_head( FILE *f, bool table = true ); \
	void tex_report_init( FILE *f, bool table = true ); \
	void tex_report_initall( FILE *f, bool table = true ); \
	void tex_report_observe( FILE *f, bool table = true ); \
	void tex_report_struct( FILE *f, bool table = true ); \
	void wipe_out( void ); \
	private:\
	int entry_new_objnum( const char *tag ); \
	void ancestors( FILE *f, bool html = true ); \
	void assign_plot_vars( int *idx, const char *lab ); \
	void attach_instance_number( char *outh, char *outv, int outSz ); \
	void count_labels_mem( int *count, const char *lab = NULL ); \
	void count_plot_vars( int *count ); \
	void create_float_list( void ); \
	void create_form( int num, const char *title, const char *prefix, FILE *frep ); \
	void create_initial_values( FILE *frep ); \
	void create_table_init( FILE *frep ); \
	void draw_buttons( void ); \
	void draw_obj( object *sel, int level = 0, int center = 0, int from = 0, bool zeroinst = false ); \
	void edit_str( const char *tag, int *idx, int res, int *done ); \
	void debugger_update( const char *hl_var, int mode ); \
	void fill_list_par( bool show_all ); \
	void fill_list_var( bool show_all, bool lag_only ); \
	void insert_labels_mem( int *num_v, const char *lab = NULL ); \
	void insert_obj_num( const char *tag, const char *ind, int *idx, int *count ); \
	void insert_store_mem( int max_v, int *num_v, const char *lab = NULL ); \
	void link_cells( const char *lab ); \
	void put_line( int x1, int y1, int x2 ); \
	void put_node( int x, int y, const char *str, bool sel ); \
	void put_text( const char *str, const char *n, int x, int y, const char *str2 ); \
	void save_cells( const char *lab ); \
	void search_title( const char *tag, int *idx, const char *lab, int *cols ); \
	void set_title( const char *lab, const char *tag, int *cols ); \
	void show_cells( const char *lab ); \
	void show_neighbors( bool update ); \
	void show_rep_initial( FILE *f, int *begin, FILE *frep ); \
	void show_rep_observe( FILE *f, int *begin, FILE *frep ); \
	void show_tmp_vars( bool update ); \
	void tex_fprintf( FILE *f, char* text ); \
	void unlink_cells( const char *lab ); \
	void write_list( FILE *frep, bool show_all, const char *prefix ); \
	void write_obj( FILE *frep, int *elemDone ); \
	void write_str( FILE *frep, int dep, const char *prefix ); \
	FILE *create_frames( const char *path, const char *fname );

#define VARIABLE_EXT \
	public: \
	void plot_runtime( void ); \
	private: \
	void write_var( FILE *frep ); \
	const char *print_constr( char *buf, int buf_sz );

#define SENSITIVITY_EXT \
	public: \
	int dataentry( void );

// definitions from LSD library
#include "lib/libLSD.h"

// standard libraries used
#include <list>

// LSD version strings, for About... boxes and code testing
#define _LSD_MAJOR_ 9
#define _LSD_MINOR_ 0
#define _LSD_VERSION_ "9.0"
#define _LSD_DATE_ "February 6 2024"	 // __DATE__

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
#define DEFAULT_SRC_DIR "src"			// default source files directory
#define FILE_BUF_SIZE 1000000			// buffer size for file reading
#define LOG_FILE "log.txt"				// name of log file
#define MARG 0.01						// y-axis % plot clearance margin
#define MARG_CONST 0.1					// y-axis % plot clearance margin/constant series
#define MAX_CELS 9000					// max number of cells (row x column) init. editor
#define MAX_COLS 100					// max numbers of columns in init. editor
#define MAX_LEVEL 10					// maximum number of object levels (plotting only)
#define MAX_PLOTS 1000					// max numbers of plots in analysis
#define MAX_PLOT_TABS 10				// max number of plot tabs to show
#define MAX_SENS_POINTS 999				// default warning threshold sensitivity analysis
#define MAX_TAB_LEN 10					// max length of plot tab title
#define NOLH_DEF_FILE "NOLH.csv"		// default NOLH file name
#define PROG_SERIES 10000				// AoR progress bar when loading series limit
#define SENS_SEP " ,;|/#\t\n"			// sensitivity data valid separators
#define SIG_MIN 1e-100					// Minimum significant value (different than zero)
#define SRV_MAX_CORES 64				// maximum number of cores to use in a server
#define SRV_MIN_CORES 12				// minimum number of cores to consider a server

// configuration files details
#define DESCRIPTION "description.txt"
#define LMM_TXT_OPTIONS "lmm_options.txt"
#define LSD_XML_OPTIONS "LSD.cfg"
#define GROUP_TXT_INFO "groupinfo.txt"
#define GROUP_XML_INFO "group.cfg"
#define MODEL_TXT_OPTIONS "model_options.txt"
#define MODEL_XML_OPTIONS "model.cfg"
#define MODEL_TXT_INFO "modelinfo.txt"
#define SYSTEM_TXT_OPTIONS "system_options.txt"

// special file names/locations in Windows
#define TCL_LIB_VAR		"TCL_LIBRARY"
#define TCL_LIB_PATH	"gnu/lib/tcl8.6"// must NOT use backslashes
#define TCL_LIB_INIT	"init.tcl"
#define TCL_EXEC_PATH	"gnu\\bin"		// must use (double) backslashes
#define TCL_FIND_EXE	"@where wish86.exe > nul 2>&1"

// constant string arrays
#define LMM_OPTIONS_NUM 16
#define LMM_OPTIONS_NAME { "sys_term", "html_browser", "font_type", \
						   "wish_exe", "lsd_src", "dim_character", \
						   "tab_size", "wrap", "synt_high", \
						   "auto_hide", "file_cmds", "group_new", \
						   "debug_exe", "restore_geom", "lmm_geom", \
						   "lsd_theme" }
#define LMM_OPTIONS_DEFAULT { "$DefaultSysTerm", "$DefaultHtmlBrowser", \
							  "$DefaultFont", "$DefaultWish", DEFAULT_SRC_DIR, \
							  "$DefaultFontSize", "4", "1", "2", "0", "0", \
							  "Work", "$DefaultDbgExe", "1", "#", \
							  "$DefaultTheme" }
#define LMM_OPTIONS_TYPE { 'p', 'p', 'p', 'p', 'p', 'a', 'a', 'a', 'a', 'a', \
						   'a', 'p', 'p', 'a', 'g', 'p' }
#define LSD_NW_NUM 3
#define LSD_NW_SRC { "lsdnw.cpp", "fun_head.h", "fun_head_fast.h" }
#define LSD_DIR_NUM 8
#define LSD_DIR_NAME { DEFAULT_SRC_DIR, "gnu", "installer", "Manual", \
					   "LMM.app", "Rpkg", "lwi", "___" }
#define LSD_MIN_NUM 5
#define LSD_MIN_FILES { "icons", "themes", "LSD.h", "interf.cpp", "analysis.cpp" }
#define LSD_WIN_NUM MODEL_OPTIONS_NUM - 3
#define LSD_WIN_NAME { "lsd", "log", "str", "da", "deb", "lat", "plt", "dap" }
#define MODEL_OPTIONS_NUM 16
#define MODEL_OPTIONS_NAME { "model_name", "model_version", "model_date", \
							 "lsd_geom", "log_geom", "str_geom", \
							 "da_geom", "deb_geom", "lat_geom", \
							 "plt_geom", "dap_geom", "last_conf", \
							 "last_obj", "last_list", "last_item", "last_first" }
#define MODEL_OPTIONS_DEFAULT { "(no name)", "1.0", "[ current_date ]", \
								"#", "#", "#", \
								"#", "#", "#", \
								"#", "#", "#", \
								"Root", "1", "0", "0" }
#define TK_WIN_NUM 10
#define TK_WIN_NAME { ".", ".log", ".str", ".inid", ".inin", ".da", ".deb", ".lat", ".plt", ".dap" }
#define WIN_COMP_NUM 2
#define WIN_COMP_PATH { "mingw64\\bin", "cygwin64\\bin" }// must use (double) backslashes

// LSD GUI name space
namespace gui
{
/*************************************************************
 CLASSES
 *************************************************************/
	struct design;
	struct nolh;


/*************************************************************
 GLOBAL VARIABLES
 *************************************************************/
	extern bool brCovered;				// browser cover currently covered
	extern bool check_save;				// saving message inside disabled objects
	extern bool eq_dum;					// current equation is dummy
	extern bool ignore_eq_file;			// configuration files equation updating
	extern bool log_ok;					// control for log window available
	extern bool meta_par_in[ ];			// meta variables for simulation settings found
	extern bool pause_run;				// pause running simulation
	extern bool redrawReq;				// flag for asynchronous window redraw request
	extern bool redrawRoot;				// control for redrawing root window (.)
	extern bool redrawStruc;			// control for redrawing model structure window
	extern bool scrollB;				// scroll box state in current runtime plot
	extern bool tk_ok;					// control for tk_ready to operate
	extern bool unsavedChange;			// control unsaved changes in configuration
	extern bool unsavedData; 			// flag unsaved simulation configurations
	extern bool unsavedSense;			// control for unsaved sensitivity data
	extern char *eq_txt;				// equation file content
	extern char *mod_options;			// model makefile options
	extern char *sens_file;				// current sensitivity analysis file
	extern char *sys_options;			// system makefile options
	extern char cfg_path[ ];			// path of LSD configuration file
	extern char eq_file[ ];				// equation file name
	extern char err_file[ ];			// error log file name
	extern char sens_path[ ];			// path of last used sensitivity directory
	extern const char *lmm_defaults[ ];	// default values for LMM parameter list
	extern const char *lmm_options[ ];	// LMM save parameter list
	extern const char *lsd_nw_src[ ];
	extern const char *model_defaults[ ];
	extern const char *model_info[ ];
	extern const char *res_g;			// structure window result variable
	extern const char *tk_wnd_names[ ];	// Tk names of main windows
	extern const char *wnd_names[ ];	// LSD main windows' names
	extern const char lmm_types[ ];		// types of LMM parameters
	extern const int NOLH_1[ ][ 7 ];	// near-orthogonal Latin hypercube tables
	extern const int NOLH_2[ ][ 11 ];
	extern const int NOLH_3[ ][ 16 ];
	extern const int NOLH_4[ ][ 22 ];
	extern const int NOLH_5[ ][ 29 ];
	extern const int NOLH_6[ ][ 100 ];
	extern int choice;					// Tcl menu control (main window)
	extern int choice_g;				// Tcl menu control variable ( structure window)
	extern int cur_plt;					// current graph plot number
	extern int done_in;					// Tcl menu control variable (log window)
	extern int doover;					// overwrite results folder (bool)
	extern int elem_count;				// recursive element counter for show elements menu
	extern int findexSens;				// sequential sensitivity index to filenames
	extern int macro;					// equations style (macros or C++) (bool)
	extern int overwConf;				// overwrite current config.file on run (bool)
	extern int platform;				// OS platform (1=Linux, 2=Mac, 3=Windows)
	extern int saveConf;				// save configuration on results saving (bool)
	extern int stop;					// activity interruption flag (Tcl boolean)
	extern int strWindowOn;				// presentation of the model structure window (bool)
	extern int watch;					// allow for graph generation interruption (bool)
	extern lsd::object *curr_obj;		// pointer to current object in browser
	extern lsd::object *last_obj;		// pointer to last selected object in structure
	extern lsd::simulation sim;			// the single GUI simulation object
	extern nolh NOLH[ ];				// characteristics of NOLH tables
	extern Tcl_Interp *interp;			// Tcl standard interpreter pointer


/*************************************************************
 GLOBAL FUNCTIONS
 *************************************************************/
	bool abort_run_threads( void );
	bool add_rt_plot_tab( const char *w, int id_sim );
	bool add_unsaved( void );
	bool check_nw_exec( const char *nw_exe );
	bool check_res_dir( const char *path, const char *sim_name = NULL );
	bool compile_run( int run_mode, bool nw = false );
	bool create_maverag( void );
	bool create_res_dir( const char *path );
	bool create_series( bool mc, str_vecT var_names );
	bool discard_change( bool checkSense = true, bool senseOnly = false, const char title[ ] = "" );
	bool eq_contains( FILE *f, const char *lab, int len );
	bool eq_header( const char *line, char *var, char *updt_in );
	bool eval_bool( const char *tcl_exp );
	bool exists_var( const char *lab );
	bool exists_window( const char *lab );
	bool expr_eq( const char *tcl_exp, const char *c_str );
	bool get_bool( const char *tcl_var, bool *var = NULL );
	bool get_precompiled_flag( const char *exec, bool nw = false );
	bool load_model_options( const char *path );
	bool load_prev_configuration( void );
	bool make_no_window( void );
	bool need_res_dir( const char *path, const char *sim_name, char *buf, int buf_sz );
	bool open_configuration( lsd::object *&r, bool reload );
	bool runtime_step( void );
	bool save_sensitivity( FILE *f );
	bool save_xml_configuration_gui( int findex = 0, const char *dest_path = NULL, bool quick = false );
	bool sensitivity_clean_dir( const char *path );
	bool sensitivity_too_large( long numSaPts );
	bool set_env( bool set );
	bool unsaved_change( bool );
	bool unsaved_change( void );
	bool NOLH_load( const char baseName[ ] = NOLH_DEF_FILE, bool force = false );
	char *eval_str( const char *tcl_exp, char *var, int var_size );
	char *fmt_ttip_descr( char *out, lsd::description *d, int outSz, bool init = true );
	char *get_str( const char *tcl_var, char *var, int var_size );
	char *load_eqfile( void );
	char *search_lsdroot( char *buf, int bufSz );
	char *strtcl( char *out, const char *text, int outSz );
	char *NOLH_valid_tables( int k, char *out, int sz );
	const char *eval_str( const char *tcl_exp );
	const char *get_fun_name( char *str, int str_sz, bool nw = false );
	const char *get_str( const char *tcl_var );
	const char *get_target_name( char *str, int str_sz, bool nw = false );
	double eval_double( const char *tcl_exp );
	double get_double( const char *tcl_var, double *var = NULL, bool no_error = false );
	double lower_bound( double a, double b, double marg, double marg_eq, int dig = 16 );
	double mat_sum_dists( double **a, int m, int n, double **b );
	double save_lattice_helper( const char *fname );
	double sum_distances( i_listT indices, double **DM );
	double update_lattice_helper( double line, double col, double val, int line_int, int col_int, int val_int );
	double upper_bound( double a, double b, double marg, double marg_eq, int dig = 16 );
	double *log_data( double *data, int start, int end, int ser, const char *err_msg );
	double **compute_distance_matrix( double **sample, int M, int k, double **DM );
	double **mat_add_mat( double **a, int m, int n, double **b, double **c );
	double **mat_add_scal( double **a, int m, int n, double b, double **c );
	double **mat_copy_mat( double **a, int m, int n, double **b );
	double **mat_copy_scal( double **a, int m, int n, double b );
	double **mat_ext_mat( double **a, int m, int n, double **b, int o, int p, int lpos );
	double **mat_ins_mat( double **a, int m, int n, double **b, int o, int p, int lpos );
	double **mat_mult_mat( double **a, int m, int n, double **b, int o, int p, double **c );
	double **mat_mult_scal( double **a, int m, int n, double b, double **c );
	double **morris_oat( int k, int r, int p, int jump, double **X );
	double **mat_new( int m, int n );
	double **opt_trajectories( int k, double **pool, int M, int r, double **X );
	int browse( lsd::object *r );
	int count_lines( const char *fname, bool dozip = false );
	int eval_int( const char *tcl_exp );
	int get_int( const char *tcl_var, int *var = NULL );
	int init_lsd_env( const char **argv );
	int intmin_hborder( int pdigits, double miny, double maxy );
	int load_configuration_gui( bool reload, strT *warnings, int quick );
	int load_gui( const char **argv );
	int load_sensitivity( FILE *f );
	int min_hborder( int pdigits, double miny, double maxy );
	int set_platform( void );
	int shrink_gnufile( void );
	int store_gnufile( struct node *c, int x4 );
	int store_gnufile( struct node *c, int x3, int x4 );
	int store_gnufile( struct node *c, int x2, int x3, int x4 );
	int store_gnufile( int x1, int x2, int x3, int x4 );
	int uniform_int_0( int max );
	int NOLH_table( int k );
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
	i_listT get_max_sum_ind( i_list_vecT indices_list, d_vecT row_maxima_i );
	i_listT top_idx( double *a, int n, int i );
	i_list_vecT add_indices( i_listT m_max_ind, int M );
	i2_vecT combinations( i_listT indices, int r );
	long eval_long( const char *tcl_exp );
	long get_long( const char *tcl_var, long *var = NULL );
	lsd::object *operate( lsd::object *r ); \
	strT win_path( strT filepath );
	void add_da_plot_tab( const char *w, int id_plot );
	void analysis( bool mc = false );
	void auto_document( const char *lab, const char *which, bool append = false );
	void canvas_binds( int n );
	void center_plot( void );
	void change_obj_number( lsd::object *&c, int value, int all, int pippo[ ], int cfrom );
	void reset_make_options( int which = 0 );
	void clean_res_dir( const char *path, const char *sim_name = NULL );
	void clean_spaces( char *s );
	void cmd( const char *cm, ... );
	void cmd_backend( const char *cm, va_list arg );
	void cover_browser( const char *text1, const char *text2, bool run );
	void create( void );
	void create_logwindow( void );
	void deb_log( bool on, int time );
	void disable_plot( void );
	void draw_buttons( void );
	void eliminate_obj( lsd::object *&c, int actual, int desired );
	void enable_plot( void );
	void error_hard_helper( const char *boxTitle, const char *boxText, const char *logText, bool defQuit );
	void free_gnufile_storage( struct node *n );
	void get_var_descr( const char *lab, char *desc, int descr_len );
	void histograms( void );
	void histograms_cs( void );
	void init_lattice_helper( double pixW, double pixH, double nrow, double ncol, int init_color );
	void init_plot( int i );
	void init_tcl_tk( const char *exec, const char *tcl_app_name );
	void insert_data_file( bool gz, int *num_v, str_vecT *var_names, bool keep_vars );
	void load_lsd_options( void );
	void log_tcl_error( bool show, const char *cm, const char *message, ... );
	void lsd_exit_gui( int v );
	void make_makefile( bool nw = false );
	void mat_del( double **a, int m );
	void plog( const char *msg, ... );
	void plog_backend( const char *cm, const char *tag, va_list arg );
	void plog_series( void );
	void plog_tag( const char *cm, const char *tag, ... );
	void plog_terminal( const char *cm, va_list arg );
	void plot( int type, const int *start, const int *end, char **str, char **tag, bool norm );
	void plot( int type, int nv, double **data, const int *start, const int *end, const int *id, char **str, char **tag );
	void plot_canvas( int type, int nv, const int *start, const int *end, char **str, char **tag );
	void plot_cross( void );
	void plot_cs_xy( void );
	void plot_gnu( void );
	void plot_lattice( void );
	void plot_phase_diagram( void );
	void plot_tseries( void );
	void print_stack( void );
	void put_line( int x1, int y1, int x2 );
	void put_node( int x, int y, const char *str, bool sel );
	void put_text( const char *str, const char *num, int x, int y, const char *str2 );
	void read_eqfile_name( char *s, int sz );
	void reset_configuration_gui( void );
	void reset_plot( void );
	void return_where_used( char *lab, char *s, int sz );
	void runtime_buttons( clock_t &last_update );
	void runtime_end( void );
	void runtime_run_start( void );
	void runtime_run_end( void );
	void runtime_start( void );
	void save_data1( void );
	void save_datazip( void );
	void scan_used_lab( const char *lab, const char *parWnd = NULL );
	void scan_using_lab( const char *lab, const char *parWnd = NULL );
	void scroll_plot( void );
	void sensitivity_created( const char *path, const char *sim_name, int findex );
	void sensitivity_doe( int *findex, design *doe, const char *dest_path );
	void sensitivity_sequential( int *findexSens, lsd::sensitivity *s, double probSampl, const char *dest_path );
	void sensitivity_undefined( void );
	void set_buttons_run( bool enable );
	void set_cs_data( void );
	void set_shortcuts( const char *window );
	void set_shortcuts_run( const char *window );
	void set_ttip_descr( const char *w, const char *lab, int it = -1, bool init = true );
	void show_comp_result( bool nw = false );
	void show_descr( const char *lab, const char *parWnd = NULL );
	void show_eq( const char *lab, const char *parWnd = NULL );
	void show_logs( const char *path, str_vecT & logs, bool par_cntl = false );
	void show_plot_gnu( int n, int type, char **str, char **tag );
	void show_prof_aggr( void );
	void show_report( const char *par_wnd );
	void show_tcl_error( const char *boxTitle, const char *errMsg, ... );
	void sort_cs_asc( char **s, char **t, double **v, int nv, int nt, int c );
	void sort_cs_desc( char **s, char **t, double **v, int nv, int nt, int c );
	void statistics( void );
	void statistics_cross( void );
	void uncover_browser( void );
	void unload_configuration_gui( bool full );
	void unset_shortcuts_run( const char *window );
	void update_bounds( void );
	void update_descr_dict( void );
	void update_lsd_options( bool save_settings = true );
	void update_model_options( bool fix = false );
	void update_more_tab( bool adding = false );
	void NOLH_clear( void );
	FILE *search_all_sources( char *str );
}


/*************************************************************
 DESIGN
 *************************************************************/
struct gui::design						// design of experiment container class
{
	int typ, tab, n, k, *par, *lag, *inst;// experiment parameters
	double **hi, **lo, ***doe;
	char **lab;
	bool *intg;

	design( lsd::sensitivity *rsens, int typ, const char *fname, const char *dest_path,
			int findex, int samples, int factors = 0, int jump = 2, int trajs = 4 );
										// constructor
	~design( void );					// destructor

	void clear_design( void );
	void load_design_data( lsd::sensitivity *rsens, int n );
};


/*************************************************************
 NOLH
 *************************************************************/
struct gui::nolh						// near-orthogonal Latin hypercube class
{
	int kMin;
	int kMax;
	int n1;
	int n2;
	int loLevel;
	int hiLevel;
	const int *table;
};
