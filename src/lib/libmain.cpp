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
 LIBMAIN.CPP
 Contains the LSD library global variables and the library
 start-up and shut-down functions.

 The main functions contained here are:

 - lsd_constructor( ), lsd_destructor( )
 Allocates and initializes the global variables on program
 startup, and destroys allocated global variables on program
 exit.
*************************************************************/

#include "lib/libLSD.h"			// LSD library classes


/*********************************
 GLOBAL VARIABLES
 *********************************/
// global variables setting library defaults
char nonavail[ ] = "NA";		// string for unavailable values (use R default)
int add_to_tot = false;			// append results to existing totals file (bool)
int dobar = false;				// output progress bar to the log/standard output
int docsv = false;				// produce .csv text results files (bool)
int dozip = true;				// compressed results file flag (bool)

// regular library global variables
bool batch_sequential = false;	// no-window multi configuration job running
bool grandTotal = false;		// produce grand total in batch processing
bool idle_loop = true;			// in main idle loop (no running operation)
bool message_logged = false;	// new message posted in log window
bool on_bar;					// flag to indicate bar is being draw in log
bool parallel_monitor;			// parallel monitor thread status
char *exec_file = NULL;			// name of executable file
char *exec_path = NULL;			// path of executable file
char *lib_file = NULL;			// name of shared library, if any
char *lib_path = NULL;			// path of shared library, if any
char *model_path = NULL;		// folder where the model files are
char *rootLsd = NULL;			// path of LSD root directory
dlliblinkage liblnk;			// call-back references for DLL
int choice;						// Tcl menu control variable (main window)
int deb_set = false;			// debug enable control (bool)
int fend;						// last multi configuration job to run
int findex;						// current multi configuration job
int log_start;					// first period to start logging to file
int log_stop;					// last period to log to file, if any
int max_runs;					// maximum number of terminal parallel runs
int max_threads;				// maximum parallel threads per run
int no_res = false;				// do not produce .res results files (bool)
int no_tot = true;				// do not produce .tot totals files (bool)
vector < simulation * > sims;	// vector holding existing simulations
vector < string > res_list;		// list of results files last saved
FILE *log_file_ptr = NULL;		// log file pointer, if any

// constant arrays
const char *desc_key_words[ DESC_KEY_NUM ] = DESC_KEY_WORDS;
const char *elem_type_names[ ELEM_TYPE_NUM ] = ELEM_TYPE_NAMES;
const char *signal_names[ REG_SIG_NUM ] = REG_SIG_NAME;
const int signals[ REG_SIG_NUM ] = REG_SIG_CODE;

#ifndef _NP_
// conditional variables
map < thread::id, workerVar * > thr_ptr;// variable worker thread pointers
mutex lock_init_sim;			// lock simulation constructor
mutex lock_run_logs;			// lock run_logs for parallel updating
mutex lock_run_pids;			// lock run_pids for parallel updating
mutex lock_run_status;			// lock run_status for parallel updating
string run_log;					// consolidated runs log
thread run_monitor;				// thread monitoring parallel instances
thread::id main_thread;			// LSD main thread ID
vector < handleT > run_pids;	// parallel running instances process id's
vector < int > run_status;		// parallel running instances status
vector < string > run_logs;		// log file list produced in parallel runs
vector < string > run_results;	// parallel run results files
vector < thread > run_threads;	// parallel running instances
#endif

// Tcl/Tk specific definitions (for the GUI version only)
#ifndef _NW_
p_mapT par_map;					// variable to parent name map for AoR
Tcl_Interp *inter;				// Tcl interpreter in GUI (for legacy LSD code)
#endif

/*********************************
 LIB_CONSTRUCTOR
 *********************************/
void __attribute__( ( constructor ) ) lib_constructor( )
{
#ifndef _NP_
	main_thread = this_thread::get_id( );
	max_threads = ( MAX_CORES <= 0 ) ? thread::hardware_concurrency( ) : MAX_CORES;
#else
	max_threads = ( MAX_CORES <= 0 ) ? 4 : MAX_CORES;
#endif

	exec_file = new char[ strlen( "" ) + 1 ];
	exec_path = new char[ strlen( "" ) + 1 ];
	lib_file = new char[ strlen( "" ) + 1 ];
	lib_path = new char[ strlen( "" ) + 1 ];
	model_path = new char[ strlen( "" ) + 1 ];
	strcpy( exec_file, "" );
	strcpy( exec_path, "" );
	strcpy( lib_file, "" );
	strcpy( lib_path, "" );
	strcpy( model_path, "" );
}


/*********************************
 LIB_DESTRUCTOR
 *********************************/
void __attribute__( ( destructor ) ) lib_destructor( )
{
	delete [ ] exec_file;
	delete [ ] exec_path;
	delete [ ] lib_file;
	delete [ ] lib_path;
	delete [ ] model_path;
	delete [ ] rootLsd;
}


/*********************************
 SIMULATION_CONSTRUCTOR
 *********************************/
simulation::simulation( void )
{
	root = new object;
	root->init( NULL, this, "Root" );
	add_description( "Root" );
	latt = new lattice;
	reset_blueprint( NULL );
	init_map( );					// set equation look-up map

	conf_name = new char[ strlen( "" ) + 1 ];
	conf_path = new char[ strlen( "" ) + 1 ];
	strcpy( conf_name, "" );
	strcpy( conf_path, "" );

	stack_log = new lsdstack;
	stack_log->prev = NULL;
	stack_log->next = NULL;
	stack_log->ns = 0;
	stack_log->vs = NULL;
	strcpy( stack_log->label, "LSD Simulation Manager" );
	stack_level = 0;

#ifndef _NP_
	parallel_ready = true;
	lock_guard < mutex > lock( lock_init_sim );// parallel semaphore
#endif

	sim = sims.size( );				// index por this sim
	sims.push_back( this );			// add to list of existing simulations
}


/*********************************
 SIMULATION_DESTRUCTOR
 *********************************/
simulation::~simulation( void )
{
	sims.erase( find( sims.begin( ), sims.end( ), this ) );

	empty_sensitivity( );
	empty_stack( );
	empty_cemetery( );
	empty_blueprint( );
	empty_lattice( );
	empty_description( );
	root->delete_obj( );

	delete latt;
	delete stack_log;
	delete [ ] conf_file;
	delete [ ] conf_name;
	delete [ ] conf_path;
	delete [ ] log_file;
}
