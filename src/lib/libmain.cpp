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
// regular library global variables
bool idle_loop = true;			// in main idle loop (no running operation)
bool message_logged = false;	// new message posted in log window
char *exec_file = NULL;			// name of executable file
char *exec_path = NULL;			// path of executable file
char *lib_file = NULL;			// name of shared library, if any
char *lib_path = NULL;			// path of shared library, if any
char *model_path = NULL;		// folder where the model files are
char *rootLsd = NULL;			// path of LSD root directory
dlliblinkage liblnk;			// call-back references for DLL
int choice;						// Tcl menu control variable (main window)
vector < simulation * > sims;	// vector holding existing simulations
FILE *stderr_ptr;				// main thread standard error file pointer
FILE *stdout_ptr;				// main thread standard output file pointer

// constant arrays
const char nonavail[ ] = NON_AVAILABLE;// unavailable values text (R default)
const char *desc_key_words[ DESC_KEY_NUM ] = DESC_KEY_WORDS;
const char *elem_type_names[ ELEM_TYPE_NUM ] = ELEM_TYPE_NAMES;
const char *signal_names[ REG_SIG_NUM ] = REG_SIG_NAME;
const int signals[ REG_SIG_NUM ] = REG_SIG_CODE;

#ifndef _NP_
// conditional variables
condition_variable seq_end;		// variable to signal simulation sequence end
map < thread::id, worker * > worker_thread_ptr;// worker thread pointers
mutex init_sim_lck;				// lock simulation constructor
mutex plog_term_lck;			// lock plog_terminal for parallel updating
mutex wrk_thr_ptr_lck;			// lock worker_thread_ptr for parallel updating
thread::id main_thread;			// LSD main thread ID
#endif


/*********************************
 LIB_CONSTRUCTOR
 *********************************/
void __attribute__( ( constructor ) ) lib_constructor( )
{
#ifndef _NP_
	main_thread = this_thread::get_id( );
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

	stderr_ptr = stderr;			// capture main thread standard streams
	stdout_ptr = stdout;
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

#ifndef _NP_
	max_threads = ( MAX_CORES <= 0 ) ? thread::hardware_concurrency( ) : MAX_CORES;
#else
	max_threads = ( MAX_CORES <= 0 ) ? 4 : MAX_CORES;
#endif

	conf_name = new char[ strlen( "" ) + 1 ];
	conf_path = new char[ strlen( "" ) + 1 ];
	strcpy( conf_name, "" );
	strcpy( conf_path, "" );

	stack_level = 0;
	stack_log = new lsdstack;
	strcpy( stack_log->label, "LSD Simulation Manager" );

#ifndef _NP_
	parallel_ready = true;
	lock_guard < mutex > lock( init_sim_lck );// parallel semaphore
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

	if ( log_file_ptr != NULL )
		fclose( log_file_ptr );

	delete latt;
	delete stack_log;
	delete [ ] conf_file;
	delete [ ] conf_name;
	delete [ ] conf_path;
	delete [ ] log_file;
}
