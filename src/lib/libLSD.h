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
 LSD dynamic link library (.dll/.so) and LSD terminal executable.

 Relevant macros for conditional compilation (when defined):

 - _FUN_: user model equation file
 - _TERM_: terminal executable
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

#ifdef __APPLE__
	#include <IOKit/pwr_mgt/IOPMLib.h>
#endif

#ifndef _TERM_
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
#define U_FN lsd::equation				// namespace for user functions
#define Z_CLEVS 7						// number of normal distr. confidence levels

#define TAG_NONE 0						// series tags - no tag
#define TAG_UPDT 1						// series from update buffer
#define TAG_ANL 2						// series from DA - analysis
#define TAG_FCT 3						// series from DA - forecast
#define TAG_DAT 4						// series from DA - observation data
#define TAG_FILE 5						// series from files
#define TAG_CALC 6						// series from calculations
#define TAG_MC 7						// series from Monte Carlo experiment

// directory/file separator
#define foldersep( dir ) ( dir[ 0 ] == '\0' ? "" : "/" )

// constant string arrays
#define DA_ALGO_NUM 2
#define DA_ALGO_NAME { "Ensemble Kalman Filter (EnKF)", "Ensemble Transform Particle (ETPF)" }
#define DESC_KEY_NUM 2
#define DESC_KEY_WORD { "_INIT_", "END_DESCRIPTION" }
#define DESC_TYPE_NUM 5
#define DESC_TYPE_NAME { "Variable", "Parameter", "Function", "", "Object" }
#define ELEM_TYPE_NUM 3
#define ELEM_TYPE_NAME { "variable", "parameter", "function" }
#define LOG_OPS_PAIR { { "==", 0 }, { "=", 0 }, { "EQ", 0 }, { "!=", 1 }, { "=!", 1 }, { "NE", 1 }, { ">", 2 }, { "GT", 2 }, { ">=", 3 }, { "=>", 3 }, { "GE", 3 }, { "<", 4 }, { "LT", 4 }, { "<=", 5 }, { "=<", 5 }, { "LE", 5 }, { "NNAN", 6 }, { "NAN", 7 }, { "NINF", 8 }, { "INF", 9 }, { "NFIN", 10 }, { "FIN", 11 } }
#define META_PAR_NUM 3
#define META_PAR_NAME { "_timeSteps_", "_numRuns_", "_rndSeed_" }
#define REG_SIG_NUM 6
#define REG_SIG_CODE { SIGINT, SIGTERM, SIGABRT, SIGFPE, SIGILL, SIGSEGV }
#define REG_SIG_NAME { "Interrupt signal", "Terminate signal", "Abort signal", \
					   "Floating-point exception", "Illegal instruction", \
					   "Segmentation violation" }
#define VAR_TAG_NUM 8
#define VAR_TAG_NAME { "", "U_", "A_", "T_", "D_", "F_", "C_", "MC_" }

// macro functions
#define BROTHER( O ) ( O == NULL ? NULL : O->next )

// general type templates
#ifdef _WIN32
	typedef HANDLE handleT;
#else
	typedef pid_t handleT;
#endif

typedef pugi::xml_document x_docT;
typedef pugi::xml_node x_nodeT;
typedef pugi::xml_attribute x_attrT;
typedef std::atomic < bool > b_atomT;
typedef std::atomic < int > i_atomT;
typedef std::condition_variable cond_vT;
typedef std::list < int > i_listT;
typedef std::lock_guard < std::mutex > l_guardT;
typedef std::lock_guard < std::recursive_mutex > rec_lguardT;
typedef std::map < int, double > d_mapT;
typedef std::mutex mtxT;
typedef std::recursive_mutex rec_mtxT;
typedef std::set < int > i_setT;
typedef std::set < std::string > str_setT;
typedef std::string strT;
typedef std::thread thrT;
typedef std::thread::id thr_idT;
typedef std::vector < bool > b_vecT;
typedef std::vector < double > d_vecT;
typedef std::vector < int > i_vecT;
typedef std::vector < handleT > hand_vecT;
typedef std::vector < long > l_vecT;
typedef std::vector < strT > str_vecT;
typedef std::vector < std::thread > thr_vecT;
typedef std::vector < std::vector < int > > i2_vecT;
typedef std::vector < std::vector < strT > > str2_vecT;
typedef std::vector < std::list < int > > i_list_vecT;
typedef std::unique_lock < std::mutex > uniq_lT;
typedef std::unique_lock < std::recursive_mutex > rec_uniqlT;
typedef std::unordered_map < strT, d_mapT > dm_mapT;
typedef std::unordered_map < strT, int > i_mapT;
typedef std::unordered_map < strT, strT > p_mapT;
typedef Eigen::MatrixXd e_matT;
typedef Eigen::VectorXd e_vecT;

// global namespace functions (legacy)
void close_sim( void );						// legacy user equation closure

// LSD GUI name space
namespace lsd
{
/*************************************************************
 CLASSES
 *************************************************************/
	class assim;
	class assimilation;
	class assinstance;
	class asstatevars;
	class bridge;
	class descr;
	class description;
	class dlliblinkage;
	class equation;
	class lattice;
	class lsdstack;
	class netlink;
	class netnode;
	class objattr;
	class objattributes;
	class object;
	class profile;
	class result;
	class sensitivity;
	class simulation;
	class varattr;
	class varattributes;
	class variable;
	class worker;


/*************************************************************
 TYPE TEMPLATES
 *************************************************************/
	typedef std::function < double( const variable *, object * ) > eq_funcT;
	typedef std::list < assim > ass_listT;
	typedef std::list < assim * > asp_listT;
	typedef std::list < descr > desc_listT;
	typedef std::list < objattr > oatt_listT;
	typedef std::list < varattr > vatt_listT;
	typedef std::vector < assim * > ass_vecT;
	typedef std::map < int, ass_vecT > ia_mapT;
	typedef std::map < strT, profile > prof_mapT;
	typedef std::map < thr_idT, worker * > wrk_mapT;
	typedef std::pair < double, object * > o_pairT;
	typedef std::pair < long, object * > n_pairT;
	typedef std::pair < objattr *, bridge * > b_pairT;
	typedef std::pair < varattr *, variable * > v_pairT;
	typedef std::vector < assinstance > ae_vecT;
	typedef std::vector < object * > o_vecT;
	typedef std::vector < simulation > sim_vecT;
	typedef std::vector < simulation * > simp_vecT;
	typedef std::vector < variable * > v_vecT;
	typedef std::unordered_map < double, object * > o_mapT;
	typedef std::unordered_map < long, object * > n_mapT;
	typedef std::unordered_map < objattr *, bridge * > b_mapT;
	typedef std::unordered_map < strT, ass_listT::iterator > ass_mapT;
	typedef std::unordered_map < strT, desc_listT::iterator > desc_mapT;
	typedef std::unordered_map < strT, oatt_listT::iterator > oatt_mapT;
	typedef std::unordered_map < strT, vatt_listT::iterator > vatt_mapT;
	typedef std::unordered_map < strT, eq_funcT > eq_mapT;
	typedef std::unordered_map < varattr *, variable * > v_mapT;
	typedef std::unordered_set < object * > o_setT;
	typedef ass_listT::iterator ass_list_itT;
	typedef ass_mapT::iterator ass_map_itT;
	typedef const variable c_varT;


/*************************************************************
 EQUATION
 *************************************************************/
	class equation							// simulation model equation class
	{
		protected:
			simulation *_sim_;				// pointer to derived class
#ifndef _TERM_
			double _d_values_[ USER_D_VARS ];// debugger probe variables
			int _i_values_[ 4 ];
			netlink *_n_values_[ 10 ];
			object *_o_values_[ 10 ];
			FILE *_f_values_[ 1 ];
#endif
		public:								// methods used in equations and GUI
			double norm( double mean, double dev );
			double round_digits( double value, int digits );
			double t_star( int df, double cl );
			double uniform( double min, double max );
			double z_star( double cl );

			double _ran1_( long *unused = 0 );
			void _close_sim_( void );

		protected:							// methods used also by class simulation
			double _fun_( variable *v, object *caller );
			void _close_lattice_( bool destroy_window = false );

			equation( void );				// constructor

		private:							// methods only used in equations
			double alapl( double mu, double alpha1, double alpha2 );
			double alaplcdf( double mu, double alpha1, double alpha2, double x );
			double bernoulli( double p );
			double beta( double alpha, double beta );
			double betacdf( double alpha, double beta, double x );
			double binomial( double p, double t );
			double bpareto( double alpha, double low, double high );
			double bparetocdf( double alpha, double low, double high, double x );
			double cauchy( double a, double b );
			double chi_squared( double n );
			double com( d_vecT & u, d_vecT & v );
			double cov( d_vecT & u, d_vecT & v );
			double exponential( double lambda );
			double fact( double x );
			double fisher( double m, double n );
			double gamma( double alpha, double beta = 1 );
			double geometric( double p );
			double ipow( double base, double exp );
			double lnorm( double mu, double sigma );
			double lnormcdf( double mu, double sigma, double x );
			double mad( d_vecT & v );
			double mean( d_vecT & v );
			double med( d_vecT v );
			double normcdf( double mu, double sigma, double x );
			double pareto( double mu, double alpha );
			double paretocdf( double mu, double alpha, double x );
			double poisson( double m );
			double poissoncdf( double lambda, double k );
			double sd( d_vecT & v );
			double student( double n );
			double uniform_int( double min, double max );
			double unifcdf( double a, double b, double x );
			double weibull( double a, double b );

			const char *_conf_name_( void );
			const char *_conf_path_( void );
			double _current_( const variable *v );
			double _debug_( bool start, int time );
			double _fast_( int new_value );
			double _init_lattice_( int init_color = -0xffffff, double nrow = 100, double ncol = 100, double pixW = 0, double pixH = 0 );
			double _init_lattice_( double pixW = 0, double pixH = 0, double nrow = 100, double ncol = 100, const char lrow[ ] = "y", const char lcol[ ] = "x", const char lvar[ ] = "", object *p = NULL, int init_color = -0xffffff );
			double _last_run_( void );
			double _last_t_( void );
			double _no_saved_( int new_value );
			double _no_search_( int new_value );
			double _no_search_up_( int new_value );
			double _no_zero_inst_( int new_value );
			double _param_( const variable *v, int new_value );
			double _plog_( bool p, const char *cm, ... );
			double _quit_( int new_value );
			double _random_( int new_value );
			double _read_lattice_( double line, double col );
			double _run_( void );
			double _save_lattice_( const char fname[ ] = "lattice" );
			double _seed_( int new_value );
			double _t_( void );
			double _update_lattice_( double line, double col, double val = 1 );
			double _use_nan_( int new_value );
			double _use_pointer_check_( bool new_value );
			eq_mapT _eq_map_;				// fast equation look-up
			inline bool _chk_hook_( object *ptr, unsigned num );
			inline bool _chk_obj_( object *ptr );
			inline bool _chk_ptr_( object *ptr );
			inline char *_bad_ptr_chr_( object *ptr, const char *file, int line );
			inline char *_no_node_chr_( const char *lab, const char *file, int line );
			inline double _bad_ptr_dbl_( object *ptr, const char *file, int line );
			inline double _no_node_dbl_( const char *lab, const char *file, int line );
			inline double _nul_lnk_dbl_( const char *file, int line );
			inline eq_funcT _chk_eq_( const char *lab );
			inline object *_cycle_obj_( object *parent, const char *label, const char *command );
			inline netlink *_bad_ptr_lnk_( object *ptr, const char *file, int line );
			inline object *_bad_ptr_obj_( object *ptr, const char *file, int line );
			inline object *_no_hook_obj_( object *ptr, unsigned num, const char *file, int line );
			inline object *_nul_lnk_obj_( const char *file, int line );
			inline void _bad_ptr_void_( object *ptr, const char *file, int line );
			inline void _nul_lnk_void_( const char *file, int line );
			object *_root_( void );
			void _init_map_( void );
			void _msleep_( unsigned msec );

#ifdef USER_FUNCS
			USER_FUNCS						// user defined equation functions
#endif
	};


/*************************************************************
 OBJATTR
 *************************************************************/
	class objattr							// object static/homogeneous
	{										// attributes class
		friend class object;

		public:								// static public attributes
			char *label;
			int label_size;

		private:							// static private attributes
			objattributes *container;		// attributes container

		public:
			objattr( simulation *sim, const char *_label );// constructor
			objattr( const objattr & a );	// copy constructor
			~objattr( void );				// destructor
			objattr & operator=( const varattr & a ) = delete;// assignment constructor
	};


/*************************************************************
 OBJATTRIBUTES
 *************************************************************/
	class objattributes						// container for object attributes
	{
		public:
			oatt_listT attr;				// element attributes linked-list
			oatt_mapT attr_map;				// map names to element attributes

		private:
			rec_mtxT oattr_lck;				// mutex lock for parallel computation

		public:
			objattr *add( simulation *sim, const char *lab );
			objattr *rename( const char *old_lab, const char *new_lab );
			objattr *search( const char *lab );
	};


/*************************************************************
 OBJECT
 *************************************************************/
	class object							// simulation model object class
	{
		friend class bridge;
		friend class equation;
		friend class netlink;
		friend class netnode;
		friend class simulation;
		friend class variable;
		friend class worker;

		public:
			bool to_compute;
			bridge *b = NULL;
			netnode *node = NULL;			// pointer to network node data structure
			objattr *attr;					// static/homogeneous attributes object
			object *next = NULL;
			object *up;						// parent object
			variable *v = NULL;

		private:
			bool *del_flag = NULL;			// address of flag to signal deletion
			bool deleting = false;			// indicate deletion in process
			b_mapT b_map;					// fast lookup map to object bridges
			int acounter = 0;				// "fail safe" when creating labels
			int lst_cnt_upd = 0;			// period of last counter update
			mtxT obj_comp_lck;				// mutex lock for parallel computations
			object *hook = NULL;
			o_vecT hooks;
			simulation *sim;				// simulation where object is contained
			void *cext = NULL;				// pointer to C++ object extension
			v_mapT v_map;					// fast lookup map to variables

		public:
			bool search_parallel( void );
			bool under_comput_var( const char *lab );
			double cal( object *caller, const char *l, int lag, bool force_search );
			double read_file_net( const char *lab, const char *dir = "", const char *base_name = "net", int serial = 1, const char *ext = "net" );
			double write_file_net( const char *lab, const char *dir = "", const char *base_name = "net", int serial = 1, bool append = false );
			object *add_n_objects2( const char *lab, int n, int t_update = -1 );
			object *add_n_objects2( const char *lab, int n, object *ex, int t_update = -1 );
			object *add_obj( const char *label, int num = 1, bool propagate = false );
			object *hyper_next( const char *lab );
			object *hyper_next( objattr *at );
			object *hyper_next( void );
			object *next_count( object *obj, int *count );
			object *search( const char *lab, bool no_search = false, bool no_search_up = true );
			object *search( objattr *at, bool no_search = false, bool no_search_up = true );
			object *search_err( const char *lab, bool no_search, bool no_search_up, const char *errmsg );
			variable *add_var( const char *str, int par, int lags = -1, bool plot = false, char deb = 'n' );
			variable *add_var( variable *example );
			variable *search_var( object *caller, const char *label, bool no_error = false, bool no_search = false, bool no_search_up = false, bool search_sons = false );
			variable *search_var( object *caller, varattr *at, bool no_error = false, bool no_search = false, bool no_search_up = false, bool search_sons = false );
			void chg_lab( const char *lab );
			void chg_var_lab( const char *old, const char *n );
			void delete_net( const char *lab );
			void delete_obj( const variable *caller = NULL );
			void delete_var( const char *lab );
			void move( const char *dest );
			void reset_end( void );

			object( object *_up, simulation *_sim, const char *_label, bool _to_compute = true );	// constructor
			~object( void );					// destructor
			object( object & o ) = delete;		// copy constructor
			object & operator=( const object & o ) = delete;// assignment constructor

		private:
			bool alloc_save_mem( void );
			bool check_cond( double val1, int lopc, double val2 );
			bool load_txt_insts( const char *file_name, FILE *f );
			bool load_txt_struct( FILE *f );
			bool sort_down_1( object *a, object *b, const char *var, int lag );
			bool sort_down_2( object *a, object *b, const char *var1, const char *var2, int lag );
			bool sort_up_1( object *a, object *b, const char *var, int lag );
			bool sort_up_2( object *a, object *b, const char *var1, const char *var2, int lag );
			bool under_computation( void );
			bridge *search_bridge( const char *lab, bool no_error = false );
			bridge *search_bridge( objattr *at, bool no_error = false );
			double av( const char *lab1, int lag = 0, bool cond = false, const char *lab2 = "", const char *lop = "", double value = NAN );
			double cal( const char *l, int lag = 0 );
			double cal( object *caller, const char *l, int lag = 0 );
			double count( const char *lab1, int lag = 0, bool cond = false, const char *lab2 = "", const char *lop = "", double value = NAN );
			double count_all( const char *lab1, int lag = 0, bool cond = false, const char *lab2 = "", const char *lop = "", double value = NAN );
			double increment( const char *lab, double value );
			double initturbo( const char *lab );
			double initturbo( const char *lab, double tot );
			double initturbo_cond( const char *label );
			double init_stub_net( const char *lab, const char gen[ ] = "DISCONNECTED", long numNodes = 0, long par1 = 0, double par2 = 0.0 );
			double interact( const char *text, double v, double *tv, int i, int j, int h, int k, object *cur, object *cur1, object *cur2, object *cur3, object *cur4, object *cur5, object *cur6, object *cur7, object *cur8, object *cur9, netlink *curl, netlink *curl1, netlink *curl2, netlink *curl3, netlink *curl4, netlink *curl5, netlink *curl6, netlink *curl7, netlink *curl8, netlink *curl9, FILE *f );
			double last_cal( const char *lab );
			double mav( object *caller, const char *lab, double per, int lag = 0 );
			double mav( object *caller, const char *lab, double per, const double weight[ ] = NULL, int lag = 0 );
			double med( const char *lab1, int lag = 0, bool cond = false, const char *lab2 = "", const char *lop = "", double value = NAN );
			double multiply( const char *lab, double value );
			double overall_max( const char *lab1, int lag = 0, bool cond = false, const char *lab2 = "", const char *lop = "", double value = NAN );
			double overall_min( const char *lab1, int lag = 0, bool cond = false, const char *lab2 = "", const char *lop = "", double value = NAN );
			double perc( const char *lab1, double p, int lag = 0, bool cond = false, const char *lab2 = "", const char *lop = "", double value = NAN );
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
			int load_xml_insts( x_nodeT &n, n_mapT &node_map, i_setT &warning );
			int load_xml_struct( x_nodeT &n, bool quick );
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
			netlink *add_link_net( object *destPtr, double weight = 0, double probTo = 1 );
			netlink *add_link_net( const char *nodeName, long startNode, long endNode, double weight = 0, double probTo = 1, bool edge = false );
			netlink *draw_link_net( void );
			netlink *search_link_net( long id );
			object *add_node_net( long id = -1, const char *nodeName = "", bool silent = false );
			object *check_net_struct( const char *nodeLab, bool noErr = false );
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
			object *next_obj( object *obj );
			object *search_node_net( const char *lab, long id );
			object *search_var_cond( const char *lab, double value, int lag = 0 );
			object *shuffle_nodes_net( const char *lab );
			object *turbosearch( const char *label, double num );
			object *turbosearch( const char *label, double tot, double num );
			object *turbosearch_cond( const char *label, double value );
			variable *search_var_err( object *caller, const char *label, bool no_search, bool no_search_up, bool search_sons, const char *errmsg );
			void collect_cemetery( const variable *caller = NULL );
			void collect_inst( o_setT &list );
			void copy_descendant( object *to );
			void delete_link_net( netlink *ptr );
			void delete_node_net( void );
			void get_line( char *lBuffer, FILE *fPtr );
			void name_node_net( const char *nodeName );
			void recreate_maps( void );
			void replicate( int num, bool propagate = false );
			void save_txt_descr( FILE *f );
			void save_txt_insts( FILE *f );
			void save_txt_struct( FILE *f, const char *tab );
			void save_xml_struct( x_nodeT &pn, long &node_serial, bool quick );
			void search_inst( object *obj, long *pos, long *checked );
			void set_blueprint( object *container );
			void set_tit_counter( void );
			void update( bool recurse, bool user );
			FILE *search_txt_data( const char *name, const char *init, const char *str );

#ifdef OBJECT_EXT
		OBJECT_EXT
#endif
	};


/*************************************************************
 BRIDGE
 *************************************************************/
	class bridge							// descendant-object container class
	{
		friend class object;
		friend class result;
		friend class simulation;

		public:
			bridge *next = NULL;
			objattr *attr;					// static/homogeneous attributes object
			object *head = NULL;

		private:
			bool counter_updated = false;
			char *search_var = NULL;		// current initialized search variable
			n_mapT t_map;					// turbosearch map
			o_mapT o_map;					// fast lookup map to object values

		public:
			bridge( objattr *_attr );		// constructor
			bridge( bridge && b );			// move constructor
			~bridge( void );				// destructor
			bridge( const bridge & b ) = delete;// copy constructor
			bridge & operator=( const bridge & b ) = delete;// assignment constructor
	};


/*************************************************************
 VARATTR
 *************************************************************/
	class varattr							// element (variable, parameter,
	{										// or function) static/homogeneous
		friend class equation;				// attributes class
		friend class object;
		friend class variable;

		public:								// static public attributes
			bool initialized = false;
			bool integer = false;			// variable must be rounded to integer
			bool observe = false;
			bool parallel = false;
			bool save = false;
			bool savei = false;
			char *label;
			double max_val = NAN;			// maximum limit for variable
			double min_val = NAN;			// minimum limit (NAN = no limit)
			int delay = 0;
			int delay_range = 0;
			int label_size;
			int num_lag;
			int period = 1;
			int period_range = 0;

		private:							// static private attributes
			bool dummy = false;
			eq_funcT eq_func = NULL;		// pointer to equation function
			varattributes *container;		// attributes container

		public:
			varattr( simulation *sim, const char *_label, int _num_lag = -1 );// constructor
			varattr( const varattr & a );	// copy constructor
			~varattr( void );				// destructor
			varattr & operator=( const varattr & a ) = delete;// assignment constructor
	};


/*************************************************************
 VARATTRIBUTES
 *************************************************************/
	class varattributes						// container for element (variable,
	{										// parameter, or function) attributes
		public:
			vatt_listT attr;				// element attributes linked-list
			vatt_mapT attr_map;				// map names to element attributes

		private:
			rec_mtxT vattr_lck;				// mutex lock for parallel computation

		public:
			varattr *add( simulation *sim, const char *lab, int lags = -1 );
			varattr *rename( const char *old_lab, const char *new_lab );
			varattr *search( const char *lab );
	};


/*************************************************************
 VARIABLE
 *************************************************************/
	class variable							// model numeric element (variable,
	{										// parameter, or function) class
		friend class assimilation;
		friend class equation;
		friend class object;
		friend class result;
		friend class simulation;
		friend class worker;

		public:								// dynamic public attributes
			bool plot = false;
			char deb_mode = 'n';
			double *val;
			double ini_val = NAN;			// initial for DA parameter estimation
			int param = 0;
			object *up;
			varattr *attr;					// static/homogeneous attributes object
			variable *next = NULL;			// sibling variable under same object

		private:							// dynamic private attributes
			bool under_computation = false;
			char *lab_tit = NULL;
			double deb_cnd_val = 0;
			double *data = NULL;
			int deb_cond = 0;
			int end = 0;
			int last_update = 0;
			int next_update = 0;
			int start = 0;
			rec_mtxT var_comp_lck;			// mutex lock for parallel computation

		public:
			double chk_val( double val );
			variable *hyper_next( void );

			variable( object *_up, const char *_label, int _param, int _num_lag, bool plot, char _deb_mode );	// constructor
			variable( const variable & v );	// copy constructor
			~variable( void );				// destructor
			variable & operator=( const variable & v ) = delete;// assignment constructor

		private:
			bool alloc_save_var( void );
			double cal( object *caller, int lag );
			inline double chk_dummy( const char *lab );
			inline double chk_res( double res );
			void add_cemetery( void );
			void delete_var( bool no_lock = false );
			void save_single( void );
			void set_lab_tit( void );

#ifdef VARIABLE_EXT
			VARIABLE_EXT
#endif
	};


/*************************************************************
 NETNODE
 *************************************************************/
	class netnode							// network node data class
	{
		friend class equation;
		friend class netlink;
		friend class object;

		private:
			char *name = NULL;				// node textual name (not required)
			double prob;					// assigned node draw probability
			int time;						// time of creation/update
			long id;						// node unique ID number (reorderable)
			long nlinks = 0;				// number of arcs FROM node
			long serial;					// node serial number (for file save/export)
			netlink *first = NULL;			// first link in the linked list of links
			netlink *last = NULL;			// last link in the linked list of links
			object *up;						// object containing node

			netnode( object *_up, long nodeId = -1, const char nodeName[ ] = "",
					 double nodeProb = 1 );	// constructor
			~netnode( void );				// destructor
			netnode( const netnode & n ) = delete;	// copy constructor
			netnode & operator=( const netnode & n ) = delete;// assignment constructor
	};


/*************************************************************
 NETLINK
 *************************************************************/
	class netlink							// individual outgoing network link class
	{
		friend class equation;
		friend class netnode;
		friend class object;

		private:
			double probTo;					// destination node draw probability
			double weight;					// link weight
			int time;						// time of creation/update
			netlink *next = NULL;			// pointer to next link (NULL if last )
			netlink *prev;					// pointer to previous link (NULL if first )
			object *from;					// network node containing the link
			object *to;						// pointer to destination number

			netlink( object *origNode, object *destNode, double linkWeight = 0, double destProb = 1 );
											// constructor
			~netlink( void );				// destructor
			netlink( const netlink & n ) = delete;	// copy constructor
			netlink & operator=( const netlink & n ) = delete;// assignment constructor
	};


/*************************************************************
 DLLIBLINKAGE
 *************************************************************/
	class dlliblinkage						// callback references for dynamic link library
	{
		friend class equation;
		friend class object;
		friend class simulation;

		public:
			int *choice;

			bool ( *runtime_step ) ( bool da_en ) = NULL;
			double ( *save_lattice_helper ) ( const char *fname ) = NULL;
			double ( *update_lattice_helper ) ( double line, double col, double val, int line_int, int col_int, int val_int ) = NULL;
			int ( object::*debugger ) ( object *c, const char *lab, double *res, bool interact, const char *hl_var ) = NULL;
			int ( *runtime_buttons ) ( void ) = NULL;
			void ( *cmd_backend ) ( const char *cm, va_list arg ) = NULL;
			void ( *cover_browser ) ( const char *text1, const char *text2, bool run, bool da_en ) = NULL;
			void ( *deb_log ) ( bool on, int time ) = NULL;
			void ( *disable_plot ) ( void ) = NULL;
			void ( *enable_plot ) ( void ) = NULL;
			void ( *error_hard_helper ) ( const char *boxTitle, const char *boxText, const char *logText, bool defQuit ) = NULL;
			void ( *init_lattice_helper ) ( double pixW, double pixH, double nrow, double ncol, int init_color ) = NULL;
			void ( *log_tcl_error ) ( bool show, const char *cm, const char *message, ... ) = NULL;
			void ( *plog_backend ) ( const char *cm, const char *tag, va_list arg ) = NULL;
			void ( *plot_runtime ) ( int *idx, int t, double cur_val, double last_val ) = NULL;
			void ( *print_stack ) ( void ) = NULL;
			void ( *progress_bar ) ( int cur_t, clock_t & last_update ) = NULL;
			void ( *runtime_end ) ( void ) = NULL;
			void ( *runtime_run_end ) ( void ) = NULL;
			void ( *runtime_run_start ) ( bool da_en ) = NULL;
			void ( *runtime_start ) ( bool da_en ) = NULL;
	};


/*************************************************************
 SENSITIVITY
 *************************************************************/
	class sensitivity						// sensitivity analysis container class
	{
		friend class object;
		friend class simulation;

		public:
			bool integer;					// variable must be rounded to integer
			char *label = NULL;				// variable name
			double *val = NULL;				// values to test sensitivity
			int cur_val = 0;				// index for value in use in combinations
			int lag;						// lag of initial value
			int num_val = 0;				// number of values to test
			int param;						// element type
			sensitivity *next = NULL;		// sensitivity analysis chain of elements

			sensitivity( const char *lab, simulation *_sim, int _param, int _lag, bool _integer, int _num_val = 0, d_vecT *_val = NULL );	// constructor
			~sensitivity( void );			// destructor
			sensitivity( const sensitivity & s ) = delete;	// copy constructor
			sensitivity & operator=( const sensitivity & s ) = delete;// assignment constructor

		private:
			simulation *sim;				// simulation where object is contained

#ifdef SENSITIVITY_EXT
			SENSITIVITY_EXT
#endif
	};


/*************************************************************
 WORKER
 *************************************************************/
	class worker							// multi-thread variable worker data
	{
		friend class simulation;

		public:
			worker( void );					// constructor
			~worker( void );				// destructor
			worker( const worker & w ) = delete;	// copy constructor
			worker & operator=( const worker & w ) = delete;// assignment constructor

		private:
			bool errored;
			bool free = false;
			bool running = false;
			bool user_excpt;
			char err_msg1[ MAX_BUFF_SIZE ] = "";
			char err_msg2[ MAX_BUFF_SIZE ] = "";
			char err_msg3[ MAX_BUFF_SIZE ] = "";
			cond_vT run;
			int signum = -1;
			jmp_buf env;
			mtxT worker_lck;
			std::exception_ptr pexcpt = nullptr;
			thrT worker_thread;
			thr_idT thread_id;
			variable *v = NULL;

			bool check( void );				// handle worker problems
			static void signal_wrapper( int signun );// wrapper for signal_handler
			void cal( variable *_v );		// start worker calculation
			void cal_worker( void );		// worker thread code
			void signal( int signum );		// signal handler
	};


/*************************************************************
 DESCR
 *************************************************************/
	class descr								// model-element description class
	{
		friend class object;
		friend class result;
		friend class simulation;

		public:
			bool initial = false;
			bool observe = false;
			char *init = NULL;
			char *label;
			char *text;
			char *type;

		private:
			description *container = NULL;

		public:
			bool has_descr_text( void );

			descr( description *_container, const char *_label, int _type, const char *_text, const char *_init, bool _initial, bool _observe );	// constructor
			~descr( void );					// destructor
			descr( const descr & d ) = delete;	// copy constructor
			descr & operator=( const descr & d ) = delete;// assignment constructor
	};


/*************************************************************
 DESCRIPTION
 *************************************************************/
	class description						// description container class
	{
		public:
			desc_listT elem;				// description elements linked-list
			desc_mapT elem_map;				// map names to description elements

		public:
			descr *add_descr( const char *lab, int type = 4, const char *text = NULL, const char *init = NULL, bool initial = false, bool observe = false );
			descr *change_descr( const char *lab_old, const char *lab = NULL, int type = -1, const char *text = NULL, const char *init = NULL, int initial = -1, int observe = -1 );
			descr *search_descr( const char *lab, bool add_missing = false );
			void reset_descr( object *r );

			description( void );			// constructor
			~description( void );			// destructor
			description( const description & d ) = delete;	// copy constructor
			description & operator=( const description & d ) = delete;// assignment constructor
	};


/*************************************************************
 RESULT
 *************************************************************/
	class result							// results file container class
	{
		friend class simulation;

		public:
			void data( object *root, int initstep, int endtstep = 0 );	// write data
			void title( object *root, int flag );// write file header

			result( const char *fname, const char *fmode, simulation *_sim,
					bool _dozip = false, bool _docsv = false );// constructor
			~result( void );				// destructor
			result( const result & r ) = delete;	// copy constructor
			result & operator=( const result & r ) = delete;// assignment

		private:
			bool da_res = false;			// data assimilation results
			bool docsv;						// comma separated .csv text format
			bool dozip;						// compressed file flag
			bool first_col;					// flag for first column in line
			gzFile fz = NULL;				// compressed file pointer
			simulation *sim;				// simulation where object is contained
			FILE *f = NULL;					// uncompressed file pointer

			void data_recursive( object *r, int t );
			void title_recursive( object *r, bool header );
			void write_data( double *data, int t, int start, int end );
			void write_title( const char *lab, const char *lab_tit, object *par, int tag, bool header = false, int start = -1, int end = -1 );
	};


/*************************************************************
 LATTICE
 *************************************************************/
	class lattice							// model (visual) lattice class
	{
		friend class equation;

		public:
			double height = 0;
			double width = 0;				// lattice screen size
			int columns = 0;
			int rows = 0;					// lattice size

		private:
			int errors = 0;					// error counter
			int **array = NULL;				// lattice data colors array
	};


/*************************************************************
 LSDSTACK
 *************************************************************/
	class lsdstack							// simulation-stack element class
	{
		friend class object;
		friend class simulation;
		friend class variable;

		public:
			char label[ MAX_ELEM_LENGTH ] = "";
			int n = 0;
			lsdstack *prev = NULL;
			variable *v = NULL;

		private:
			lsdstack *next = NULL;
	};


/*************************************************************
 PROFILE
 *************************************************************/
	class profile							// profiled variable class
	{
		friend class variable;

		public:
			unsigned int comp = 0;
			unsigned long long ticks = 0;
	};


/*************************************************************
 ASSTATEVARS
 *************************************************************/
	class asstatevars						// DA state variables class
	{
		friend class assimilation;
		friend class simulation;

		private:
			size_t idx;						// current position during analysis
			v_vecT st_vec;					// current state variable vector for DA

		private:
			void save_state_vars( object *r );
	};


/*************************************************************
 ASSINSTANCE
 *************************************************************/
	class assinstance						// DA element instance collection class
	{
		friend class assim;
		friend class assimilation;
		friend class object;
		friend class result;

		private:
			bool saved = false;				// data saved in current run
			d_vecT anl;						// analysis data produced by DA
			d_vecT anl_hi;					// analysis data upper bound
			d_vecT anl_lo;					// analysis data lower bound
			d_vecT fct;						// forecast data produced by DA
			d_vecT fct_hi;					// forecast data upper bound
			d_vecT fct_lo;					// forecast data lower bound
			d_vecT dat;						// observational data used during DA
			d_vecT dat_hi;					// observational data upper bound
			d_vecT dat_lo;					// observational data lower bound
			int	cur_t;						// last update time of data
			int	end;						// last valid data period
			int start;						// first valid data period

		public:
			assinstance( int _start, int _end, bool fct, bool dat );// constructor
	};


/*************************************************************
 ASSIM
 *************************************************************/
	class assim								// data assimilation container class
	{
		friend class assimilation;
		friend class object;
		friend class result;
		friend class asstatevars;

		public:
			bool disable = false;			// element disabled for assimilation
			strT label;						// element name

		private:
			ae_vecT da_data;				// data produced during assimilation
			bool data_obs = false;			// element has data obs. to assimilate
			bool no_data = true;			// no data retrieved?
			bool param = false;				// element is a parameter (not variable)
			bool plot = false;				// element marked for run-time plot
			bool save = false;				// element market to be saved
			bool update = false;			// element to be updated by assimilation
			double par_n_sd = 0;			// parameter normal standard deviation
			double par_u_low = 0;			// parameter uniform distribution delta -
			double par_u_upp = 0;			// parameter uniform distribution delta +
			int cov_idx = -1;				// index (row+col) in covariance matrix
			int data_col_num = 0;			// number of data value column
			int inst_idx = -1;				// index to last used element instance
			int inst_ini = 0;				// number of instances at t=0
			int par_dist = 0;				// parameter distribution (0:N/1:U)
			int t_col_num = 0;				// number of time value column
			strT data_file;					// name of source data CSV file
			strT data_col_name;				// name of data value column
			strT parent;					// element parent name
			strT t_col_name;				// name of time value column

		public:
			assim( const strT & _label, bool _param = false, bool _disable = false, bool _update = false, bool _data_obs = false, const strT & _data_file = "", const strT & _data_col_name = "", const strT & _t_col_name = "", int _data_col_num = 0, int _t_col_num = 0, int _par_dist = 0, double _par_n_sd = 0, double _par_u_upp = 0, double _par_u_low = 0 );
											// constructor

		private:
			bool init( void );
			void finish( void );
			void update_param( variable *v );

#ifdef ASSIM_EXT
			ASSIM_EXT
#endif
	};


/*************************************************************
 ASSIMILATION
 *************************************************************/
	class assimilation						// assimilation container class
	{
		friend class assim;
		friend class asstatevars;
		friend class object;
		friend class result;
		friend class simulation;

		public:
			ass_listT ass_elem;				// assimilation elements linked-list head
			ass_mapT elem_map;				// map names to assimilation elements
			double conf_lev = 95.;			// confidence interval confidence level (%)
			double infl_fac = 1.;			// ensemble inflation factor
			int algorithm = 0;				// algorithm to use in DA (0=EnKF,1=ETPF)
			int align_trim = false;			// trim extra instances for forecast alignment
			int ens_infl = false;			// use ensemble inflation
			int infl_time = 2;				// ensemble inflation start time
			int use_dsp_file = false;		// use data dispersion/virtual obs.
			int disable = false;			// disable data assimilation
			int med_stats = false;			// use median/MAD statistics (vs mean/SD)
			int sav_ci = false;				// save confidence interval for DA series
			int sav_dat = false;			// save observational data
			int sav_dsp = false;			// save data dispersion matrix
			int sav_fct = false;			// save forecast (intermediary) results
			strT dsp_file;					// data assimilation dispersion CSV file

			const char *algo_names[ DA_ALGO_NUM ] = DA_ALGO_NAME;

		private:
			i_vecT miss_inst;				// # of missing instances per mat. column
			dm_mapT var_data;				// assimilation observational data map
			e_matT dsp_mat;					// data assimilation dispersion matrix
			e_matT dsp_chol;				// Cholesky decomposition of dispersion matrix
			ia_mapT time_var;				// list of times and variables for assimilation
			int next_t = 0;					// next da assimilation time
			simulation *ref_sim = NULL;		// reference simulation for data assimilation
			sim_vecT run_sims;				// vector of running assimilation simulations
			str_vecT data_lab;				// observational data variable labels
			str_vecT fctd_labs;				// forecasted variable labels

		public:
			ass_list_itT search( const char *lab );
			int count( int what );
			int run_simulation( int until_t );
			void show( void );

		private:
			ass_list_itT search( const strT & lab );
			bool init( simulation *ref );
			bool load_files( simulation *sim, int last_t );
			const e_matT & dsp_stat( const e_matT & x, const e_vecT & x_bar );
			const e_matT & ensemble_forecast( void );
			const e_matT & ensemble_inflation( const e_matT & x, const e_vecT & x_bar );
			const e_matT & virtual_obs( const e_vecT & z, int nobs );
			const e_matT & forward_matrix( const ass_vecT & dvars );
			const e_vecT & data_obs( const ass_vecT & dvars, int cur_t );
			e_matT ci_stat( const e_matT & x_e, const e_vecT & x_bar );
			e_vecT loc_stat( const e_matT & x );
			int analysis( const ass_vecT & dvars, int cur_t );
			int calc_dsp_mat( void );
			int load_dsp_mat( simulation *sim, strT & missing );
			int load_obs_data( int last_t );
			template < class T > double median( T begin, T end );
			void align_state_vars( void );
			void finish( void );
			void reset_insts( void );
			void save_param( object *r );
			void update_assim_vars( const e_vecT & x_a, const e_vecT & x_f, const e_vecT & z, const e_matT & x_a_ci, const e_matT & x_f_ci, const e_matT & z_ci, int t );
			void update_runtime_plot( int cur_t );
			void update_state_vars( const e_matT & x_a_e );

#ifdef ASSIMILATION_EXT
			ASSIMILATION_EXT
#endif
	};


/*************************************************************
 SIMULATION
 *************************************************************/
	class simulation : public equation		// simulation container class
	{
		friend class assimilation;
		friend class equation;
		friend class netnode;
		friend class object;
		friend class variable;
		friend class worker;

		public:
			bool batch_sequential = false;	// terminal multi configuration job running
			bool conf_ok = false;			// a valid configuration file is loaded
			bool fast;						// safe copy of fast_mode flag
			bool grand_total = false;		// produce grand total in batch processing
			bool idle_loop = true;			// main idle loop (no running operation)
			bool message_logged = false;	// new message posted in log window
			bool no_search;					// disable standard variable search mechanism
			bool no_search_up;				// disable object up-search mechanism
			bool on_bar;					// indicate bar is being draw in log
			bool parallel_mode;				// parallel mode (multithreading) status
			bool parallel_monitor;			// parallel monitor thread status
			bool save_alt = false;			// alternate save path flag
			bool save_ok = true;			// control saving model configuration
			bool use_nan;					// flag to allow using Not a Number value
			bool user_exception = false;	// indicate exception generated by user code
			b_atomT running = false;		// single simulation is running
			b_atomT running_seq = false;	// set of sequential simulations running
			char *alt_path = NULL;			// alternative output path
			char *conf_file = NULL;			// name of current configuration file
			char *conf_name = NULL;			// name of current simulation configuration
			char *conf_path = NULL;			// folder where the current configuration is
			char *log_file = NULL;			// name of log file, if any
			char conf_eq_txt[ MAX_FILE_SIZE ] = "";// equations saved in configuration file
			char rep_file[ MAX_PATH_LENGTH ] = "";// documentation report file name
			char res_path[ MAX_PATH_LENGTH ] = "";// path of last used results directory
			dlliblinkage *liblnk = NULL;	// call-back references for DLL
			hand_vecT run_pids;				// parallel running instances process id's
			int add_to_tot = false;			// type of totals file generated (bool)
			int deb_set = false;			// debug enable control (bool)
			int deb_t;						// next debug stop time step (0 for none)
			int dobar = false;				// enable progress bar in log/standard output
			int docsv = false;				// produce .csv text results files (bool)
			int dozip = true;				// compressed results file flag (bool)
			int fast_mode = 2;				// level of LOG messages & runtime plot
			int fend;						// last multi configuration job to run
			int findex;						// current multi configuration job
			int last_dispatch_time;			// last time step controlled by dispatcher
			int last_run = 1;				// total serial simulation runs
			int last_t = MAX_STEPS;			// number of simulation steps
			int log_start;					// first period to start logging to file
			int log_stop;					// last period to log to file, if any
			int max_runs;					// maximum number of parallel runs
			int max_threads;				// maximum parallel threads per run
			int no_ptr_chk = false;			// disable user pointer checking
			int no_res = false;				// do not produce .res results files (bool)
			int no_tot = true;				// do not produce .tot totals files (bool)
			int parallel_disable = false;	// flag to control parallel mode
			int prof_aggr_time = false;		// show aggregate profiling times
			int prof_min_msecs = 0;			// profile variables taking more than X msecs.
			int prof_obs_only = false;		// profile only observed variables
			int quit = 0;					// simulation interruption mode (0=none)
			int run = 0;					// current serial simulation run
			int series_saved = 0;			// number of series saved
			int stack_info = 0;				// LSD stack control
			int stale_time;					// time passed from last step computation
			int t = 0;						// current time step
			i_atomT eff_t = 0;				// number of executed time steps
			lattice *latt = NULL;			// model lattice
			lsdstack *stack_log = NULL;		// LSD stack
			mtxT obj_list_lck;				// lock object list for parallel manip.
			mtxT run_logs_lck;				// lock run_logs for parallel updating
			mtxT run_pids_lck;				// lock run_pids for parallel updating
			objattributes oa;				// static object attributes container
			object *blueprint = NULL;		// LSD blueprint (effective model in use)
			object *root = NULL;			// LSD root object
			o_setT obj_list;				// set with all existing LSD objects
			prof_mapT prof_times;			// set of saved profiling times
			sensitivity *sens = NULL;		// sensitivity analysis linked-list head
			std::mt19937 mt32;				// Mersenne-Twister 32 bits generator
			strT run_log;					// consolidated runs log
			str_vecT res_list;				// list of results files last saved
			str_vecT run_logs;				// log file list produced in parallel runs
			thrT run_monitor;				// thread monitoring parallel instances
			thrT sim_thread;				// thread object where simulation is run
			unsigned seed = 1;				// random number generator initial seed
			varattributes va;				// static variable attributes container
			variable *cemetery = NULL;		// LSD saved data from deleted objects
			worker *workers = NULL;			// multi-thread parallel worker data
			FILE *log_file_ptr = NULL;		// log file pointer, if any
#ifndef _TERM_
			p_mapT par_map;					// variable to parent name map for AoR
			Tcl_Interp *inter;				// Tcl interpreter (for legacy LSD code)
#endif
		private:
			asstatevars da_svars;			// state variable vector for DA
			bool batch_loop = false;		// batch multi-config batch loop in process
			bool error_hard_thread;			// error_hard called in worker thread
			bool no_saved = true;			// disable usage of saved values as lagged ones
			bool no_zero_instance = true;	// flag to allow deleting last object instance
			bool parallel_abort;			// indicate parallel threads were aborted
			bool watch_trigger = false;		// indicate that a watch condition was met
			bool watch_write_mode;			// flag for write-only watch condition
			bool worker_crashed;			// parallel worker crash flag
			bool worker_ready;				// parallel worker ready flag
			b_atomT parallel_ready;			// indicate variable worker is ready
			char conf_eq_file[ MAX_PATH_LENGTH ] = "";// equation file name in config. file
			char error_hard_msg1[ MAX_BUFF_SIZE ];// buffer for parallel worker title msg
			char error_hard_msg2[ MAX_BUFF_SIZE ];// buffer for parallel worker log msg
			char error_hard_msg3[ MAX_BUFF_SIZE ];// buffer for parallel worker box msg
			char watch_elem[ MAX_ELEM_LENGTH + 1 ] = "";// elem. triggering watch condition
			clock_t start_profile[ MAX_PROF_SIZE ];// profile-level start times
			clock_t end_profile[ MAX_PROF_SIZE ];// profile-level end times
			cond_vT upd_workers;			// worker schedule update signal
			int nsim;						// library simulation object index
			int ran_gen_id = 2;				// ID of initial generator (DO NOT CHANGE)
			int stack_level;				// LSD stack call level
			i_atomT alaplErrCnt, bernoErrCnt, betaErrCnt, binomErrCnt, cauchErrCnt, chisqErrCnt, expErrCnt, fishErrCnt, gammaErrCnt, geomErrCnt, lnormErrCnt, normErrCnt, paretErrCnt, poissErrCnt, studErrCnt, weibErrCnt;
			i_vecT run_status;				// parallel running instances status
			long idum = 0;					// Park-Miller default seed (legacy code)
			long nodesSerial = 1;			// network node serial number counter
			object *wait_delete = NULL;		// LSD object waiting for deletion
			mtxT draw_lc1_lck;				// locks for random generator operations
			mtxT draw_lc2_lck;
			mtxT draw_lf24_lck;
			mtxT draw_lf48_lck;
			mtxT draw_mt32_lck;
			mtxT draw_mt64_lck;
			mtxT draw_rd_lck;
			mtxT error_lck;					// control multiple error_hard calls
			mtxT run_status_lck;			// lock run_status for parallel updating
			mtxT seq_end_lck;				// lock seq_end for parallel updating
			mtxT var_update_lck;			// control worker variable update
			mtxT wrk_crash_lck;				// control worker crash handling
			std::minstd_rand lc1;			// linear congruential generator (internal)
			std::minstd_rand lc2;			// linear congruential generator (user)
			std::mt19937_64 mt64;			// Mersenne-Twister 64 bits generator
			std::random_device rd;			// simulation random device
			std::ranlux24 lf24;				// lagged fibonacci 24 bits generator
			std::ranlux48 lf48;				// lagged fibonacci 48 bits generator
			str_vecT run_results;			// parallel run results files
			thr_vecT run_threads;			// parallel running instances
			variable *last_cemetery = NULL;	// LSD last saved cemetery entry

		public:
			bool results_alt_path( const char *altPath );
			bool save_txt_configuration( const char *path, const char *rname, const char *ext, const char eq_file[ ], const char eq_txt[ ] = "" );
			bool save_xml_configuration( int findex = 0, const char *dest_path = NULL, bool quick = false, const char mod_nam[ ] = "", const char mod_ver[ ] = "", const char mod_dat[ ] = "", const char eq_file[ ] = "", const char eq_txt[ ] = "" );
			bool stop_parallel( void );
			int hyper_count( const char *lab );
			int hyper_count_var( const char *lab );
			int load_configuration( bool reload, strT *warnings, int quick );
			int rnd_int( int min, int max );
			int run_parallel( bool term, const char *exec, const char *simname, int fseed, int runs, int thrrun, int parruns );
			int run_simulation( int until_t, int until_run, bool da );
			int worker_errors( void );
			void detach_parallel( void );
			void empty_sensitivity( sensitivity *cs = NULL );
			void empty_stack( void );
			void error_hard( const char *boxTitle, const char *boxText, bool defQuit, const char *logFmt, ... );
			void init_random( unsigned seed );
			void plog( const char *msg, ... );
			void plog_tag( const char *cm, const char *tag, ... );
			void plog_terminal( const char *cm, va_list arg );
			void reset_blueprint( object *r );
			void set_fast( int level );
			void unload_configuration( bool full );

			simulation( const char fname[ ] = "", const char path[ ] = "", int quick = 0 );// constructor
			simulation( simulation && src ) { }// move constructor
			~simulation( void );			// destructor
			simulation( const simulation & s ) = delete;	// copy constructor
			simulation & operator=( const simulation & s ) = delete;// assignment

		private:
			bool load_txt_descr( const char *msg, FILE *f );
			bool next_batch( void );
			double betacf( double a, double b, double x );
			double build_obj_list( bool set_list );
			int init_new_run( clock_t & start, clock_t & last_update, bool da_en = false );
			int init_new_seq( clock_t & start, char *bar_done, int & perc_done, int & last_done, bool da_en = false );
			int load_txt_configuration( bool reload, int quick );
			int monitor_logs( void );
			template < class distr > double draw_gen( distr &d );
			template < class distr > double draw_lc1( distr &d );
			template < class distr > double draw_lc2( distr &d );
			template < class distr > double draw_lf24( distr &d );
			template < class distr > double draw_lf48( distr &d );
			template < class distr > double draw_mt32( distr &d );
			template < class distr > double draw_mt64( distr &d );
			template < class distr > double draw_rd( distr &d );
			void *set_random( int gen );
			void empty_blueprint( void );
			void empty_cemetery( void );
			void empty_lattice( void );
			void init_math_error( void );
			void log_parallel( bool term );
			void monitor_parallel( bool term );
			void parallel_update( variable *v, object* p, object *caller = NULL );
			void run_parallel_exec( bool term, int id, strT cmd );
			void save_results( bool da_en = false );
			void update_bar( char *bar, int done, int & last_done, int bar_sz );
			void warn_distr( i_atomT & errCnt, bool & stopErr, const char *distr, const char *msg );

#ifdef SIMULATION_EXT
			SIMULATION_EXT
#endif
	};


/*************************************************************
 GLOBAL VARIABLES
 *************************************************************/
	extern const bool no_pointer_check;		// user pointer check static disable
	extern const bool no_pointer_init;		// user pointer init. static disable

#ifndef _FUN_
	extern assimilation *da;				// data assimilation object container
	extern char *exec_file;					// name of executable file
	extern char *exec_path;					// path of executable file
	extern char *lib_file;					// name of shared library, if any
	extern char *lib_path;					// path of shared library, if any
	extern char *model_path;				// folder where the model files are
	extern char *root_lsd;					// path of LSD root directory
	extern const char nonavail[ ];			// string for unavailable values
	extern const char *desc_key_words[ ];	// library constant string arrays
	extern const char *desc_type_names[ ];
	extern const char *elem_type_names[ ];
	extern const char *meta_par_names[ ];
	extern const char *signal_names[ ];
	extern const char *tag_pref[ ];
	extern const double t_dist_cl[ T_CLEVS ];// t-distribution table confidence
	extern const double t_dist_st[ T_CLEVS ][ 36 ];// t-distribution table statistics
	extern const double z_dist_cl[ Z_CLEVS ];// normal distribution table confidence
	extern const double z_dist_st[ Z_CLEVS ];// normal distribution table statistics
	extern const int signals[ ];			// handled system signal numbers
	extern const i_mapT logic_ops_map;		// conditional operators
	extern cond_vT seq_end;					// signal simulation sequence end
	extern description *desc;				// element description object container
	extern mtxT plog_term_lck;				// lock plog_terminal for parallel upd.
	extern mtxT wrk_thr_ptr_lck;			// lock worker_thread_ptr for par. upd.
	extern simp_vecT sims;					// vector holding existing simulations
	extern std::mt19937 lib_prng;			// internal pseudo-random number generator
	extern thr_idT main_thread;				// LSD main thread ID
	extern wrk_mapT worker_thread_ptr;		// worker thread pointer map
	extern FILE *stderr_ptr;				// main thread standard error pointer
	extern FILE *stdout_ptr;				// main thread standard output pointer

#ifdef __APPLE__
	extern IOPMAssertionID mac_pwr_assert;	// mac sleep control
#endif


/*************************************************************
 GLOBAL FUNCTIONS
 *************************************************************/
	bool strwsp( const char *str );
	bool valid_label( const char *lab );
	bool valid_xml_string( const char *lab );
	char *clean_file( const char *file );
	char *clean_path( const char *path );
	char *get_path( const char *file );
	char *strcatn( char *d, const char *s, size_t dSz );
	char *strcpyn( char *d, const char *s, size_t dSz );
	char *strdecdata( char *out, const char *in, int outSz = 0 );
	char *strencdata( char *out, const char *in, int outSz = 0 );
	char *strupr( char *s );
	const char *signal_name( int signum );
	double median( d_vecT & v );
	double strtod( const char *in, char** endptr, double inv );
	d_vecT strtodsplit( const char *in, char sep, double inv = 0. );
	int dispatch_runs( sim_vecT & run_sims, int until_t, int until_run, bool da );
	int kill_system( simulation *sim, int id );
	int run_system( const char *cmd, simulation *sim = NULL, int id = -1 );
	int strcln( char *out, const char *str, int outSz );
	int strlf( char *out, const char *str, int outSz );
	int strtrim( char *out, const char *str, int outSz );
	int strtrimin( char *out, const char *str, int outSz );
	int strwrap( char *out, const char *str, int outSz, int wid );
	int strwrds( const char *s );
	long strtol( const char *in, char** endptr, int base, long inv );
	l_vecT strtolsplit( const char *in, char sep, long inv = 0 );
	strT to_string( const char *fmt, ... );
	str_vecT strtostrsplit( const char *in, char sep, bool remQuotes = false );
	void cmd( const char *cm, ... );
	void debug_break( void );
	void empty_assimilation( void );
	void empty_description( description *d = NULL );
	void empty_objattributes( simulation *sim = NULL );
	void empty_varattributes( simulation *sim = NULL );
	void exception_handler( int signum, const char *what );
	void finish_lib( void );
	void handle_signals( void ( * handler ) ( int signum ) );
	void inhibit_system_sleep( void );
	void init_lib( void );
	void lsd_exit( int v, bool clean = false );
	void msleep( unsigned msec = 1000 );
	void plog_master( const char *cm, ... );
	void plog_tag_master( const char *cm, const char *tag, ... );
	void restore_system_sleep( void );
	void set_exec( const char *path, const char *file );
	void signal_handler( int signum );
#endif
}

