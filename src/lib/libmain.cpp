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

namespace lsd
{
/*************************************************************
 GLOBAL VARIABLES
 *************************************************************/

	// regular library global variables
	assimilation da;				// data assimilation object
	char *exec_file = NULL;			// name of executable file
	char *exec_path = NULL;			// path of executable file
	char *lib_file = NULL;			// name of shared library, if any
	char *lib_path = NULL;			// path of shared library, if any
	char *model_path = NULL;		// folder where the model files are
	char *root_lsd = NULL;			// path of LSD root directory
	cond_vT seq_end;				// variable to signal simulation sequence end
	mtxT init_sim_lck;				// lock simulation constructor
	mtxT plog_term_lck;				// lock plog_terminal for parallel updating
	mtxT wrk_thr_ptr_lck;			// lock worker_thread_ptr for parallel updating
	sim_vecT sims;					// vector holding existing simulations
	thr_idT main_thread;			// LSD main thread ID
	wrk_mapT worker_thread_ptr;		// worker thread pointers
	FILE *stderr_ptr;				// main thread standard error file pointer
	FILE *stdout_ptr;				// main thread standard output file pointer

#ifdef __APPLE__
	IOPMAssertionID mac_pwr_assert = kIOPMNullAssertionID;// mac sleep control
#endif

	// constant arrays
	const char nonavail[ ] = NON_AVAILABLE;// unavailable values text (R default)
	const char *desc_key_words[ DESC_KEY_NUM ] = DESC_KEY_WORD;
	const char *desc_type_names[ DESC_TYPE_NUM ] = DESC_TYPE_NAME;
	const char *elem_type_names[ ELEM_TYPE_NUM ] = ELEM_TYPE_NAME;
	const char *meta_par_names[ META_PAR_NUM ] = META_PAR_NAME;
	const char *signal_names[ REG_SIG_NUM ] = REG_SIG_NAME;
	const int signals[ REG_SIG_NUM ] = REG_SIG_CODE;
	const i_mapT logic_ops_map = LOG_OPS_PAIR;
}


/*************************************************************
 LIB CONSTRUCTOR
 *************************************************************/
void __attribute__( ( constructor ) ) lib_constructor( )
{
	lsd::main_thread = std::this_thread::get_id( );
	lsd::exec_file = new char[ strlen( "" ) + 1 ];
	lsd::exec_path = new char[ strlen( "" ) + 1 ];
	lsd::lib_file = new char[ strlen( "" ) + 1 ];
	lsd::lib_path = new char[ strlen( "" ) + 1 ];
	lsd::model_path = new char[ strlen( "" ) + 1 ];
	strcpy( lsd::exec_file, "" );
	strcpy( lsd::exec_path, "" );
	strcpy( lsd::lib_file, "" );
	strcpy( lsd::lib_path, "" );
	strcpy( lsd::model_path, "" );

	lsd::stderr_ptr = stderr;		// capture main thread standard streams
	lsd::stdout_ptr = stdout;
}


/*************************************************************
 LIB DESTRUCTOR
 *************************************************************/
void __attribute__( ( destructor ) ) lib_destructor( )
{
	delete [ ] lsd::exec_file;
	delete [ ] lsd::exec_path;
	delete [ ] lsd::lib_file;
	delete [ ] lsd::lib_path;
	delete [ ] lsd::model_path;
	delete [ ] lsd::root_lsd;
}


/*************************************************************
 EQUATION CONSTRUCTOR
 *************************************************************/
lsd::equation::equation( void )
{
	_init_map_( );				// set equation look-up map
}


/*************************************************************
 SIMULATION CONSTRUCTOR
 *************************************************************/
lsd::simulation::simulation( void )
{
	root = new object;
	root->init( NULL, this, "Root" );
	add_description( "Root" );
	latt = new lattice;
	reset_blueprint( NULL );

	max_threads = ( MAX_CORES <= 0 ) ? thrT::hardware_concurrency( ) : MAX_CORES;

	conf_name = new char[ strlen( "" ) + 1 ];
	conf_path = new char[ strlen( "" ) + 1 ];
	strcpy( conf_name, "" );
	strcpy( conf_path, "" );

	stack_level = 0;
	stack_log = new lsdstack;
	strcpy( stack_log->label, "LSD Simulation Manager" );

	parallel_ready = true;
	l_guardT lock( init_sim_lck );// parallel semaphore

	_sim_ = this;				// register pointer to base equation class
	nsim = sims.size( );		// index por this sim
	sims.push_back( this );		// add to list of existing simulations
}


/*************************************************************
 SIMULATION DESTRUCTOR
 *************************************************************/
lsd::simulation::~simulation( void )
{
	sims.erase( find( sims.begin( ), sims.end( ), this ) );

	empty_sensitivity( );
	empty_stack( );
	empty_cemetery( );
	empty_blueprint( );
	empty_description( );
	_close_lattice_( );
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
