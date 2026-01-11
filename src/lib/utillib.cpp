/*************************************************************

	LSD 9.0 - January 2026
	written by Marco Valente, Universita' dell'Aquila
	and by Marcelo Pereira, University of Campinas

	Copyright Marco Valente and Marcelo Pereira
	LSD is distributed under the GNU General Public License

	See Readme.txt for copyright information of
	third parties' code used in LSD

 *************************************************************/

/*************************************************************
 UTILLIB.CPP
 Contains the basic set of utilities used in DLL or terminal
 executables. The remaining functions are stored in
 UTIL.CPP.

 The main functions contained in this file are:

 - void plog( const char *m, ... );
 print  message string m in the Log screen or the console.

 - void error_hard( const char *boxTitle, const char *boxText,
 				   bool defQuit, const char *logFmt, ... );
 print error messages to the log screen, console and error
 file, recovering LSD configuration to allow for non-crashing
 recovery.
 *************************************************************/

#include "lib/libLSD.h"				// LSD library classes


/*************************************************************
 PLOG
 Print message on the log window,
 if GUI is available, or console
 *************************************************************/
void lsd::simulation::plog( const char *cm, ... )
{
	static va_list argptr;

	va_start( argptr, cm );

	if ( liblnk != NULL )
		liblnk->plog_backend( cm, "", argptr );
	else
		plog_terminal( cm, argptr );

	va_end( argptr );
}


/*************************************************************
 PLOG_MASTER
 *************************************************************/
void lsd::plog_master( const char *cm, ... )
{
	static va_list argptr;

	va_start( argptr, cm );

	if ( sims.size( ) > 0 && sims[ 0 ] != NULL )
	{
		if ( sims[ 0 ]->liblnk != NULL )
			sims[ 0 ]->liblnk->plog_backend( cm, "", argptr );
		else
			sims[ 0 ]->plog_terminal( cm, argptr );
	}

	va_end( argptr );
}


/*************************************************************
 _PLOG_
 Print message in equations according
 to simulation flags
 *************************************************************/
double lsd::equation::_plog_( bool p, const char *cm, ... )
{
	static va_list argptr;

	if ( ( ! p && ! _sim_->fast ) || ( p && _sim_->fast_mode < 2 ) )
	{
		va_start( argptr, cm );

		if ( _sim_->liblnk != NULL )
			_sim_->liblnk->plog_backend( cm, "", argptr );
		else
			_sim_->plog_terminal( cm, argptr );

		va_end( argptr );

		return 1;
	}
	else
		return 0;
}


/*************************************************************
 PLOG_TAG
 The optional tag parameter has to
 correspond to the log window
 existing tags, if GUI is available,
 or console
 *************************************************************/
void lsd::simulation::plog_tag( const char *cm, const char *tag, ... )
{
	static va_list argptr;

	va_start( argptr, tag );

	if ( liblnk != NULL )
		liblnk->plog_backend( cm, tag, argptr );
	else
		plog_terminal( cm, argptr );

	va_end( argptr );
}


/*************************************************************
 PLOG_TAG_MASTER
 *************************************************************/
void lsd::plog_tag_master( const char *cm, const char *tag, ... )
{
	static va_list argptr;

	va_start( argptr, tag );

	if ( sims.size( ) > 0 && sims[ 0 ] != NULL )
	{
		if ( sims[ 0 ]->liblnk != NULL )
			sims[ 0 ]->liblnk->plog_backend( cm, tag, argptr );
		else
			sims[ 0 ]->plog_terminal( cm, argptr );
	}

	va_end( argptr );
}


/*************************************************************
 PLOG_TERMINAL
 Back-end to plog and plog_tag on
 console
 *************************************************************/
void lsd::simulation::plog_terminal( const char *cm, va_list arg )
{
	static bool bufdyn;
	static char *buffer, *message, bufstat[ MAX_BUFF_SIZE ], msgstat[ MAX_BUFF_SIZE ];
	static int i, j, reqsz, sz;
	static va_list argcpy;

	buffer = bufstat;
	message = msgstat;
	va_copy( argcpy, arg );

	reqsz = vsnprintf( buffer, MAX_BUFF_SIZE, cm, arg );

	l_guardT lock( plog_term_lck );

	if ( reqsz < 0 )
	{
		fprintf( stderr_ptr, "\nCannot expand message '%s...'\n", cm );
		return;
	}

	// handle very large messages
	if ( reqsz >= MAX_BUFF_SIZE )
	{
		buffer = new char[ reqsz + 1 ];
		sz = vsnprintf( buffer, reqsz + 1, cm, argcpy );

		if ( reqsz < 0 || sz > reqsz )
		{
			fprintf( stderr_ptr, "\nCannot expand message '%s...'\n", cm );
			delete [ ] buffer;
			return;
		}

		message = new char[ reqsz + 1 ];
		bufdyn = true;
	}
	else
		bufdyn = false;

	va_end( argcpy );

	// remove invalid charaters and Tk control characters
	for ( i = 0, j = 0; buffer[ i ] != '\0' && j < reqsz; ++i )
		if ( ( isprint( buffer[ i ] ) || buffer[ i ] == '\n' ||
			   buffer[ i ] == '\r' || buffer[ i ] == '\t' ) &&
			 ! ( buffer[ i ] == '\"' ||
				 ( buffer[ i ] == '$' && buffer[ i + 1 ] != '$' ) ) )
			message[ j++ ] = buffer[ i ];
	message[ j ] = '\0';

	fprintf( stdout_ptr, "%s", message );
	fflush( stdout_ptr );

	if ( sims.size( ) > 0 && sims[ 0 ] != NULL )
		sims[ 0 ]->message_logged = true;

	if ( bufdyn )
	{
		delete [ ] buffer;
		delete [ ] message;
	}
}


/*************************************************************
 ERROR_HARD
 Procedure called when an unrecoverable error occurs.
 Information about the state of the simulation when the error
 occurred is provided. Users can abort the program or analyze
 the results collected up the latest time step available.
 *************************************************************/
void lsd::simulation::error_hard( const char *boxTitle, const char *boxText, bool defQuit, const char *logFmt, ... )
{
	if ( quit == 2 )		// simulation already being stopped
		return;

	static char logText[ MAX_BUFF_SIZE ];
	static va_list argptr;

	va_start( argptr, logFmt );
	vsnprintf( logText, MAX_BUFF_SIZE, logFmt, argptr );
	va_end( argptr );

	// prevent concurrent use by more than one thread
	l_guardT lock( error_lck );

	// abort worker and park message if not running in main LSD thread
	if ( std::this_thread::get_id( ) != main_thread )
	{
		if ( ! error_hard_thread )	// handle just first error
		{
			error_hard_thread = true;
			strcpyn( error_hard_msg1, boxTitle, MAX_BUFF_SIZE );
			strcpyn( error_hard_msg2, logText, MAX_BUFF_SIZE );
			strcpyn( error_hard_msg3, boxText, MAX_BUFF_SIZE );
			throw 1;
		}
		else
			return;
	}

	quit = 2;				// do not continue simulation

	if ( liblnk != NULL && liblnk->error_hard_helper != NULL )
		liblnk->error_hard_helper( boxTitle, boxText, logText, defQuit );
	else
		fprintf( stderr, "\nError: %s\n(%s)\n", boxTitle, logText );

	lsd_exit( 13 );
}


/*************************************************************
 COUNT_SAVE
 *************************************************************/
void lsd::object::count_save( int *count )
{
	object *cur;

	for ( auto cv = v; cv != NULL; cv = cv->next )
		if ( cv->attr->save == 1 || cv->attr->savei == 1 )
			( *count )++;

	for ( auto cb = b; cb != NULL; cb = cb->next )
	{
		if ( cb->head == NULL )
			cur = attr->cont->sim->blueprint->search( cb->attr );
		else
			cur = cb->head;

		cur->count_save( count );
	}
}


/*************************************************************
 GET_SAVED
 Get the set of elements which values are saved
 during simulation run
 *************************************************************/
void lsd::object::get_saved( FILE *out, const char *sep, bool all_var )
{
	int i, sl;
	char *lab;
	object *cur;

	for ( auto cv = v; cv != NULL; cv = cv->next )
		if ( cv->attr->save || all_var )
		{
			// get element description
			auto cd = desc != NULL ? desc->search_descr( cv->attr->label ) : NULL;
			if ( cd != NULL && cd->text != NULL && ( sl = strlen( cd->text ) ) > 0 )
			{
				// select just the first description line
				lab = new char[ sl + 1 ];
				strcpy( lab, cd->text );
				for ( i = 0; i < sl; ++i )
					if ( lab[ i ] == '\n' || lab[ i ] == '\r' )
					{
						lab[ i ] = '\0';
						break;
					}
			}
			else
				lab = NULL;

			fprintf( out, "%s%s%s%s%s%s%s\n", cv->attr->label, sep, cv->param ? "parameter" : "variable", sep, attr->label, sep, lab != NULL ? lab : "" );
		}

	for ( auto cb = b; cb != NULL; cb = cb->next )
	{
		if ( cb->head == NULL )
			cur = attr->cont->sim->blueprint->search( cb->attr );
		else
			cur = cb->head;

		cur->get_saved( out, sep, all_var );
	}
}


/*************************************************************
 GET_SA_LIMITS
 Get the max-min limits used for sensitivity
 analysis of variables
 *************************************************************/
void lsd::object::get_sa_limits( FILE *out, const char *sep, bool meta_par_in[ ] )
{
	int i, sl;
	char *lab, type[ 10 ];

	for ( i = 0; i < META_PAR_NUM; ++i )
		meta_par_in[ i ] = false;

	for ( auto cs = attr->cont->sim->sens; cs != NULL; cs = cs->next )
	{
		// get current value (first object)
		auto cv = search_var( NULL, cs->label );

		// get element description
		auto cd = desc != NULL ? desc->search_descr( cs->label ) : NULL;
		if ( cd != NULL && cd->text != NULL && ( sl = strlen( cd->text ) ) > 0 )
		{
			// select just the first description line
			lab = new char[ sl + 1 ];
			strcpy( lab, cd->text );
			for ( i = 0; i < sl; ++i )
				if ( lab[ i ] == '\n' || lab[ i ] == '\r' )
				{
					lab[ i ] = '\0';
					break;
				}
		}
		else
			lab = NULL;

		// find max and min values
		double min = HUGE_VAL, max = - HUGE_VAL;
		for ( i = 0; cs->val != NULL &&  i < cs->num_val; ++i )
			if ( cs->val[ i ] < min )
				min = cs->val[ i ];
			else
				if ( cs->val[ i ] > max )
					max = cs->val[ i ];

		// check meta-parameters
		if ( cs->param == 1 )
		{
			strcpy( type, "parameter" );

			for ( i = 0; i < META_PAR_NUM; ++i )
				if ( ! strcmp( cs->label, meta_par_names[ i ] ) )
				{
					strcpy( type, "setting" );
					meta_par_in[ i ] = true;
					break;
				}
		}
		else
			strcpy( type, "variable" );

		fprintf( out, "%s%s%s%s%d%s%s%s%g%s%g%s%g%s\"%s\"\n", cs->label, sep, type, sep, cs->param == 1 ? 0 : cs->lag + 1, sep, cs->integer ? "integer" : "real", sep, cv != NULL ? cv->val[ cs->lag ] : NAN, sep, min, sep, max, sep, lab != NULL ? lab : "" );

		delete [ ] lab;
	}
}


/*************************************************************
 SET_LAB_TIT
 Ensure that all objects on top of the variables
 have the counter updated, and then writes the
 lab_tit field.
 lab_tit indicates the position of the object
 containing the variables in the model.
 *************************************************************/
void lsd::variable::set_lab_tit( void )
{
	bool first = true;
	char app[ MAX_LINE_SIZE ], app1[ 2 * MAX_LINE_SIZE ];
	object *cur;

	if ( up->up == NULL )
	{
		// this is the root of the model
		if ( lab_tit != NULL )
			return;						// already done in the past

		lab_tit = new char[ strlen( "R" ) + 1 ];
		strcpy( lab_tit, "R" );

		return;
	}

	for ( cur = up; cur->up != NULL; cur = cur->up )
	{
		// find the bridge containing the variable
		cur->set_tit_counter( );

		if ( ! first )
			snprintf( app1, 2 * MAX_LINE_SIZE, "%d_%s", cur->acounter, app );
		else
		{
			first = false;
			snprintf( app1, 2 * MAX_LINE_SIZE, "%d", cur->acounter );
		}

		strcpyn( app, app1, MAX_LINE_SIZE );
	}

	delete [ ] lab_tit;
	lab_tit = new char[ strlen( app ) + 1 ];
	strcpy( lab_tit, app );
}


/*************************************************************
 SET_TIT_COUNTER
 *************************************************************/
void lsd::object::set_tit_counter( void )
{
	int i, t;
	bridge *cb;
	object *cur;

	if ( up == NULL )
		return;

	up->set_tit_counter( );

	// find the bridge which contains the object
	cb = up->search_bridge( attr );

	if ( cb->counter_updated )
		return;

	t = attr->cont->sim->t;
	for ( cur = cb->head, i = 1; cur != NULL; cur = cur->next, ++i )
		if ( cur->lst_cnt_upd < t )		// don't update more than once per period
		{								// to avoid deletions to change counters
			cur->acounter = i;
			cur->lst_cnt_upd = t;
		}

	cb->counter_updated = true;
}


/*************************************************************
 SET_BLUEPRINT
 copy the naked structure of the model into another object,
 called blueprint, to be used for adding objects without
 example
 *************************************************************/
void lsd::object::set_blueprint( object *container )
{
	bridge *cb, *cb1;
	object *cur, *cur1;

	if ( container == NULL )
		return;

	container->attr = attr;

	for ( auto cv = v; cv != NULL; cv = cv->next )
		container->add_var( cv );

	for ( cb = b; cb != NULL; cb = cb->next )
	{
		if ( cb->head == NULL )
			continue;

		cur1 = cb->head;
		container->add_obj( cur1->attr->label );

		for ( cb1 = container->b; cb1->attr != cb->attr; cb1 = cb1->next );

		cur = cb1->head;
		cur1->set_blueprint( cur );
	}
}


/*************************************************************
 EMPTY_BLUEPRINT
 remove the current blueprint
 *************************************************************/
void lsd::simulation::empty_blueprint( void )
{
	if ( blueprint == NULL )
		return;

	blueprint->delete_obj( );
	blueprint = NULL;
}


/*************************************************************
 RESET_BLUEPRINT
 reset the current blueprint
 *************************************************************/
void lsd::simulation::reset_blueprint( object *r )
{
	empty_blueprint( );
	blueprint = new object ( NULL, ROOT_NAME, true, this );

	if ( r != NULL )
		r->set_blueprint( blueprint );
}


/*************************************************************
 SEARCH_PARALLEL
 *************************************************************/
bool lsd::object::search_parallel( void )
{
	// search among the variables
	for ( auto cv = v; cv != NULL; cv = cv->next )
		if ( cv->attr->parallel )
			return true;

	// search among descendants
	for ( auto cb = b; cb != NULL; cb = cb->next )
		if ( cb->head != NULL )
			if ( cb->head->search_parallel( ) )
				return true;

	return false;
}


/*************************************************************
 _QUIT_ (*)
 *************************************************************/
double lsd::equation::_quit_( int new_value )
{
	if ( new_value >= 0 && new_value <= 2 )
		return ( _sim_->quit = new_value );
	else
		return _sim_->quit;
}


/*************************************************************
 _FAST_ (*)
 *************************************************************/
double lsd::equation::_fast_( int new_value )
{
	if ( new_value >= 0 && new_value <= 2 )
		return ( _sim_->fast = new_value );
	else
		return _sim_->fast;
}


/*************************************************************
 _PARAM_ (*)
 *************************************************************/
double lsd::equation::_param_( const variable *v, int new_value )
{
	if ( v == NULL )
		return -1;

	if ( new_value >= 0 && new_value <= 2 )
		return ( ( ( variable * ) v )->param = new_value );
	else
		return v->param;
}


/*************************************************************
 _USE_NAN_ (*)
 *************************************************************/
double lsd::equation::_use_nan_( int new_value )
{
	if ( new_value == 0 || new_value == 1 )
		return ( _sim_->use_nan = new_value == 1 ? true : false );
	else
		return _sim_->use_nan;
}


/*************************************************************
 _USE_POINTER_CHECK_ (*)
 *************************************************************/
double lsd::equation::_use_pointer_check_( bool new_value )
{
	return _sim_->build_obj_list( new_value );
}


/*************************************************************
 _NO_SAVED_ (*)
 *************************************************************/
double lsd::equation::_no_saved_( int new_value )
{
	if ( new_value == 0 || new_value == 1 )
		return ( _sim_->no_saved = new_value == 1 ? true : false );
	else
		return _sim_->no_saved;
}


/*************************************************************
 _NO_SEARCH_ (*)
 *************************************************************/
double lsd::equation::_no_search_( int new_value )
{
	if ( new_value == 0 || new_value == 1 )
		return ( _sim_->no_search = new_value == 1 ? true : false );
	else
		return _sim_->no_search;
}


/*************************************************************
 _NO_SEARCH_UP_ (*)
 *************************************************************/
double lsd::equation::_no_search_up_( int new_value )
{
	if ( new_value == 0 || new_value == 1 )
		return ( _sim_->no_search_up = new_value == 1 ? true : false );
	else
		return _sim_->no_search_up;
}


/*************************************************************
 _NO_ZERO_INST_ (*)
 *************************************************************/
double lsd::equation::_no_zero_inst_( int new_value )
{
	if ( new_value == 0 || new_value == 1 )
		return ( _sim_->no_zero_instance = new_value == 1 ? true : false );
	else
		return _sim_->no_zero_instance;
}


/*************************************************************
 _SEED_ (*)
 *************************************************************/
double lsd::equation::_seed_( int new_value )
{
	if ( new_value >= 0 )
	{
		_sim_->seed = ( unsigned ) new_value;
		_sim_->init_random( _sim_->seed );
		return _sim_->seed;
	}
	else
		return _sim_->seed - 1;
}


/*************************************************************
 _RANDOM_ (*)
 *************************************************************/
double lsd::equation::_random_( int new_value )
{
	if ( new_value >= 0 && new_value <= 7 )
		_sim_->set_random( ( unsigned ) new_value );

	return _sim_->ran_gen_id;
}


/*************************************************************
 _DEBUG_ (*)
 *************************************************************/
double lsd::equation::_debug_( bool start, int time )
{
	if ( time >= 0 && _sim_->liblnk != NULL )
		_sim_->liblnk->deb_log( start, time );

	if ( start )
		return _sim_->deb_t;
	else
		return _sim_->log_stop;
}


/*************************************************************
 _ROOT_ (*)
 *************************************************************/
lsd::object *lsd::equation::_root_( void )
{
	return _sim_->root;
}


/*************************************************************
  _CONF_NAME_ (*)
 *************************************************************/
const char *lsd::equation::_conf_name_( void )
{
	return _sim_->conf_name;
}


/*************************************************************
 _CONF_PATH_ (*)
 *************************************************************/
const char *lsd::equation::_conf_path_( void )
{
	return _sim_->conf_path;
}


/*************************************************************
 _CURRENT_ (*)
 *************************************************************/
double lsd::equation::_current_( const variable *v )
{
	if ( v != NULL && v->val != NULL )
		return v->val[ 0 ];
	else
		return NAN;
}


/*************************************************************
 _T_ (*)
 *************************************************************/
double lsd::equation::_t_( void )
{
	return _sim_->t;
}


/*************************************************************
 _LAST_ (*)
 *************************************************************/
double lsd::equation::_last_t_( void )
{
	return _sim_->last_t;
}


/*************************************************************
 _RUN_ (*)
 *************************************************************/
double lsd::equation::_run_( void )
{
	return _sim_->run;
}


/*************************************************************
 _LAST_RUN_ (*)
 *************************************************************/
double lsd::equation::_last_run_( void )
{
	return _sim_->last_run;
}
