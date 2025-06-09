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
 VARIAB.CPP
 The (C++) object variable is devoted to contain numerical
 values of the model. Only double precision floating point
 numbers are used in LSD. Variables are mainly storages for
 information. Actually, all the work is done by LSD objects.

 The most important task of variables is to return their
 value when requested. It is done by comparing the global time
 of the simulation with the time the variable was most
 recently updated. If the value requested (considering the lag)
 is already available, that is returned. Otherwise, the
 variable shifts its lagged values, and calls its equation to
 compute the new value.

 All fields and functions in variables are public, so that
 users may override the default mechanism.

 The fields composing a variables are

 - char *label;
 name of the variable. It needs to be unique in the model

 - object *up;
 address of the object containing the variable

 - variable *next;
 pointer to the next variable contained in the object.
 Variables in an object are organized as a linked chain, and
 can be reached only  via their fields next

 - double *val;
 vector of numerical values. val[ 0 ] is the most recent value
 computed by the equation, that is, computed at time last_update.
 val[1] is the value computed at time last_update - 1; val[2] at
 time last_update - 2 and so on.

 - int num_lag;
 number of lagged values stored for the variable

 - int save;
 flag identifying whether the variable has to be saved or not in
 the result file

 - int plot;
 Flag used to indicate variables that are plotted in the run-
 time graph.

 - char deb_mode;
 flag used to indicate the variables to debug. If this flag is
 equal 'd', when the simulation is run in debug mode it stops
 immediately after the computation of its value.

 - int deb_cond;
 Like the flag deb_mode, but it stops the simulation if the
 attached condition is satisfied. It does not require that the
 simulation is run in debug mode. Its different values represent
 the different conditions for stopping: <, > or ==

 - double deb_cnd_val;
 numerical value used for the conditional stop

 - int under_computation;
 control flag used to avoid infinite recursion of an equation
 calling itself. Used to issue a message of error

 - int last_update;
 contain the global time when it was lastly computed the
 equation for the variable

 - int param;
 Flag set to 1, in case the variable is considered a parameter.
 In case it is, when requested the value it is always returned
 its field val[ 0 ].

 - char initialized;
 flag indicative whether the variable has been initialized with
 numerical values set as default by the system or if they were
 actually chosen by the user. The flag is 0 in case of newly
 created objects and 1 in case the variable's values has been
 at least shown once in the initial values editor window. This
 flag is also saved in the data file, so that this information
 is not lost. The flag prevents to run a simulation if the data
 where not confirmed by users.

 The main methods of the (C++) object variable are:

 - void init( object *_up, char *_label, int_param, int _num_lag, double *_val );
 perform the initialization.

 - double cal( object *caller, int lag );
 it is its main function. Return the numerical value

 	   val[last_update+lag-t]

 if the condition

 	   t-lag<=last_update

 is satisfied. That means that either the variable has already
 been updated, and therefore the requested value is available,
 or that, though the variable has not been still updated in the
 time step, the value requested is a lagged one and therefore
 can be retrieved from the vector of the past values.

 Only in case the lag requested is zero and the variable has
 not been computed at the present time step, the method shifts
 its lagged values and calls the method _fun_ that perform the
 equation computation.

 - void empty( void ) ;
 It is used to free all the memory assigned to the variable.
 Used by object::delete_obj to cancel an object.
 *************************************************************/

#include "lib/libLSD.h"				// LSD library classes


/*************************************************************
 VARATTR constructor
 *************************************************************/
lsd::varattr::varattr( varattributes *_cont, objattr *_par_attr, const char *_label, int _num_lag )
{
	cont = _cont;
	par_attr = _par_attr;
	num_lag = _num_lag;
	label_size = strlen( _label );

	label = new char [ label_size + 1 ];
	strcpy( label, _label );
}


/*************************************************************
 VARATTR copy constructor
 *************************************************************/
lsd::varattr::varattr( const varattr & a )
{
	initialized = a.initialized;
	integer = a.integer;
	observe = a.observe;
	parallel = a.parallel;
	save = a.save;
	savei = a.savei;

	max_val = a.max_val;
	min_val = a.min_val;

	num_lag = a.num_lag;
	delay = a.delay;
	delay_range = a.delay_range;
	period = a.period;
	period_range = a.period_range;
	dummy = a.dummy;
	eq_func = a.eq_func;

	cont = a.cont;
	par_attr = a.par_attr;
	label_size = a.label_size;

	label = new char [ a.label_size + 1 ];
	strcpy( label, a.label );
}


/*************************************************************
 VARATTR destructor
 *************************************************************/
lsd::varattr::~varattr( void )
{
	if ( cont != NULL )
		cont->attr_map.erase( label );

	delete [ ] label;
}


/*************************************************************
 VARATTRIBUTES constructor
 *************************************************************/
lsd::varattributes::varattributes( simulation *_sim )
{
	sim = _sim;
}


/*************************************************************
 VARATTRIBUTES destructor
 *************************************************************/
lsd::varattributes::~varattributes( void )
{
	empty_varattributes( sim );
}


/*************************************************************
 EMPTY_VARATTRIBUTES
 *************************************************************/
void lsd::empty_varattributes( simulation *sim )
{
	if ( sim == NULL && sims.size( ) > 0 )
		sim = sims[ 0 ];

	if ( sim == NULL )
		return;

	sim->va.attr.clear( );
	sim->va.attr_map.clear( );
}


/*************************************************************
 SEARCH
 *************************************************************/
lsd::varattr *lsd::varattributes::search( const char *lab )
{
	if ( lab != NULL && strlen( lab ) > 0 )
	{
		auto d = attr_map.find( lab );
		if ( d != attr_map.end( ) )
			return & ( *( d->second ) );
	}

	return NULL;
}


/*************************************************************
 ADD
 *************************************************************/
lsd::varattr *lsd::varattributes::add( objattr *par_attr, const char *lab, int lags )
{
	auto d = attr_map.find( lab );

	// prevent concurrent use by more than one thread
	rec_lguardT lock( vattr_lck );

	if ( d != attr_map.end( ) )
	{
		d->second->par_attr = par_attr;
		d->second->num_lag = lags;
		return & ( *( d->second ) );
	}

	attr.emplace_back( this, par_attr, lab, lags );
	attr_map.emplace( lab, --attr.end( ) );

	return & attr.back( );
}


/*************************************************************
 RENAME
 *************************************************************/
lsd::varattr *lsd::varattributes::rename( const char *old_lab, const char *new_lab )
{
	auto d = attr_map.find( old_lab );
	if ( d == attr_map.end( ) || strlen( new_lab ) == 0 )// doesn't exist or empty?
		return NULL;

	// prevent concurrent use by more than one thread
	rec_lguardT lock( vattr_lck );

	auto attr = d->second;
	attr->label_size = strlen( new_lab );

	delete [ ] attr->label;
	attr->label = new char [ attr->label_size + 1 ];
	strcpy( attr->label, new_lab );

	attr_map.erase( d );
	attr_map.emplace( new_lab, attr );

	return & ( *attr );
}


/*************************************************************
 VARIABLE constructor
 *************************************************************/
lsd::variable::variable( object *_up, const char *_label, int _param, int _num_lag, bool _plot, char _deb_mode )
{
	varattributes *va = & _up->attr->cont->sim->va;

	if ( _param != 0 )
		_num_lag = 0;

	up = _up;
	param = _param;
	plot = _plot;
	deb_mode = _deb_mode;

	if ( _num_lag >= 0 )
	{
		val = new double [ _num_lag + 1 ];
		for ( int i = 0; i <= _num_lag; ++i )
			val[ i ] = 0;
	}
	else
		val = NULL;

	if ( ( attr = va->search( _label ) ) == NULL )
		attr = va->add( _up->attr, _label, _num_lag );
	else
	{
		attr->par_attr = _up->attr;
		attr->cont = va;
		attr->num_lag = _num_lag;
	}

	if ( ( _param == 0 && _num_lag == 0 ) || _param == 2 )
		attr->initialized = true;
	else
		attr->initialized = false;
}


/*************************************************************
 VARIABLE copy constructor
 *************************************************************/
lsd::variable::variable( const variable & v )
{
	plot = ( ! v.attr->cont->sim->running ) ? v.plot : false;
	ini_val = v.ini_val;
	param = v.param;
	attr = v.attr;
	up = v.up;

	deb_mode = v.deb_mode;
	deb_cnd_val = v.deb_cnd_val;
	deb_cond = v.deb_cond;
	last_update = v.last_update;

	val = new double [ v.attr->num_lag + 1 ];
	for ( int i = 0; i <= v.attr->num_lag; ++i )
		val[ i ] = v.val[ i ];
}


/*************************************************************
 ~VARIABLE destructor
 *************************************************************/
lsd::variable::~variable( void )
{
	delete [ ] val;
	delete [ ] lab_tit;
	free( data );
}


/*************************************************************
 DELETE_VAR
 *************************************************************/
void lsd::variable::delete_var( bool no_lock )
{

	if ( attr->cont->sim->running && ! no_lock )
	{
		// prevent concurrent use by more than one thread
		rec_lguardT lock( var_comp_lck );
	}

	delete this;
}


/*************************************************************
 HYPER_NEXT
 Find the next instance of variable anywhere in the structure
 *************************************************************/
lsd::variable *lsd::variable::hyper_next( void )
{
	object *cur;

	if ( ( cur = up->hyper_next( ) ) != NULL )
		return cur->search_var( NULL, attr, true );

	return NULL;
}


/*************************************************************
 CHK_VAL
 Adjust value for considering
 variable constraints
 *************************************************************/
double lsd::variable::chk_val( double val )
{
	if ( std::isfinite( val ) )
	{
		if ( attr->integer )
			val = round( val );

		if ( std::isfinite( attr->max_val ) && val > attr->max_val )
			val = attr->max_val;
		else
			if ( std::isfinite( attr->min_val ) && val < attr->min_val )
				val = attr->min_val;
	}
	else
		val = NAN;

	return val;
}


/*************************************************************
 CAL
 Standard version (non parallel computation)
 *************************************************************/
double lsd::variable::cal( object *caller, int lag )
{
	int i, eff_lag;
	double app;
	simulation *sim = attr->cont->sim;

#ifndef _TERM_
	bool tit_updated;
	int time;
	clock_t pstart = 0, pend = 0;
#endif

	if ( param == 1 )
	{
		if ( sim->deb_set && sim->t == sim->deb_t && ( deb_mode == 'w' || deb_mode == 'W' ) )
		{
			sim->watch_trigger = true;
			sim->watch_write_mode = false;
			strncpy( sim->watch_elem, attr->label, MAX_ELEM_LENGTH );
		}

		return val[ 0 ];				// it's a parameter, ignore lags
	}

	// prepare mutex for variables and functions updated in multiple threads
	rec_uniqlT guard( var_comp_lck, std::defer_lock );

	if ( param == 0 )					// it's a variable
	{
		// invalid lag or value not saved yet
		if ( lag > attr->num_lag && ( sim->no_saved || ! ( attr->save || attr->savei ) || sim->t - lag < start ) )
		{
			eff_lag = lag;
			goto error;
		}

		// effective lag for variables (compatible with older versions)
		eff_lag = ( last_update < sim->t ) ? lag - 1 : lag;

		// check lag error and return past value if available
		if ( lag != 0 )
		{
			if ( eff_lag < 0 )			// with negative lag
				goto error;

			if ( eff_lag > attr->num_lag )	// in principle, invalid lag
			{
				if ( sim->no_saved || ! ( attr->save || attr->savei ) )	// and not saved
					goto error;
				else
					if ( lag > sim->t - start )	// or before there are saved values
						goto error;

				return data[ sim->t - lag - start ]; // use saved past value
			}
			else
				return val[ eff_lag ];	// use regular past value
		}
		else
		{
			// already calculated this time step or not to be calculated this time step
			if ( last_update >= sim->t || sim->t < next_update )
			{
				if ( sim->deb_set && sim->t == sim->deb_t && ( deb_mode == 'w' || deb_mode == 'W' ) )
				{
					sim->watch_trigger = true;
					sim->watch_write_mode = false;
					strncpy( sim->watch_elem, attr->label, MAX_ELEM_LENGTH );
				}

				return( val[ 0 ] );
			}

			// wait for computation of this variable by other threads
			if ( sim->parallel_mode && ! attr->dummy )
				guard.lock( );

			if ( last_update >= sim->t )// recheck if not computed during lock
				return( val[ 0 ] );
		}
	}
	else								// function
	{
		if ( lag < 0 || lag > attr->num_lag ) // with invalid lag
			goto error;

		if ( lag > 0 )					// lagged value
			return val[ lag - 1 ];

		if ( caller == NULL )			// update or inadequate caller
			return val[ 0 ];

		// wait for computation of this function by other threads
		if ( sim->parallel_mode && ! attr->dummy )
			 guard.lock( );
	}

	// there is a value to be computed

	if ( under_computation )
	{
		sim->error_hard( "deadlock",
						 "check your equation code to prevent this situation\nprobably using the variable lagged value instead",
						 true,
						 "equation for '%s' (object '%s') requested \nits own value while computing its current value", attr->label, up->attr->label );
		return 0;
	}

	under_computation = true;

	if ( sim->fast_mode == 0 && ! sim->parallel_mode )
	{
		// add the Variable to the stack
		if ( sim->stack_log != NULL && sim->stack_log->next == NULL )
		{
			sim->stack_level++;
			sim->stack_log->next = new lsdstack;
			sim->stack_log->next->prev = sim->stack_log;
			strcpyn( sim->stack_log->next->label, attr->label, MAX_ELEM_LENGTH );
			sim->stack_log->next->n = sim->stack_level;
			sim->stack_log->next->v = this;
			sim->stack_log = sim->stack_log->next;
		}
		else
		{
			sim->error_hard( "internal problem in LSD",
							 "if error persists, please contact developers",
							 true,
							 "failure while pushing '%s' (object '%s')",
							 attr->label, up->attr->label );
			return 0;
		}

#ifndef _TERM_
		if ( sim->stack_info >= sim->stack_level && ( ! sim->prof_obs_only || attr->observe ) )
			sim->start_profile[ sim->stack_level - 1 ] = pstart = clock( );
		else
			if ( sim->prof_aggr_time )
				pstart = clock( );
#endif
	}
#ifndef _TERM_
	else
		if ( sim->prof_aggr_time )
			pstart = clock( );
#endif

	// Compute the Variable's equation
	sim->user_exception = true;		// allow distinguishing among internal & user exceptions
	try								// do it while catching exceptions to avoid obscure aborts
	{
		app = sim->_fun_( this, caller );
	}
	catch ( std::exception& exc )
	{
		sim->plog( "\n\nAn exception was detected while computing the equation \nfor '%s' requested by object '%s'", attr->label, caller == NULL ? "(none)" : caller->attr->label );
		sim->quit = 2;
		throw;
	}
	catch ( int p )					// avoid general catch of error_hard throwing to lsdmain
	{
		throw p;
	}
	catch ( ... )
	{
		if ( sim->quit != 2 )		// error message not already presented?
		{
			sim->plog( "\n\nAn unknown problem was detected while computing the equation \nfor '%s' requested by object '%s'", attr->label, caller == NULL ? "(none)" : caller->attr->label );
			sim->quit = 2;
			throw;
		}
		else
		{
			app = NAN;				// mark result as invalid
			sim->use_nan = true;	// and allow propagation
		}
	}
	sim->user_exception = false;

	for ( i = 0; i < attr->num_lag; ++i ) // scale down the past values
		val[ attr->num_lag - i ] = val[ attr->num_lag - i - 1 ];

	val[ 0 ] = app;

	last_update = sim->t;

	// choose next update step for special updating variables
	if ( attr->period > 1 || attr->period_range > 0 )
	{
		next_update = sim->t + attr->period;
		if ( attr->period_range > 0 )
			next_update += sim->rnd_int( 0, attr->period_range );
	}

	if ( sim->fast_mode == 0 && ! sim->parallel_mode )
	{
#ifndef _TERM_
		if ( sim->prof_aggr_time )
		{
			pend = clock( );
			time = pend - pstart;

			if ( ( ! sim->prof_obs_only || attr->observe ) && time > sim->prof_min_msecs )
			{
				strT var_name = attr->label;
				sim->prof_times[ var_name ].ticks += time;
				sim->prof_times[ var_name ].comp++;
			}
		}

		tit_updated = false;
		if ( sim->stack_info >= sim->stack_level && ( ! sim->prof_obs_only || attr->observe ) )
		{
			sim->end_profile[ sim->stack_level - 1 ] = sim->prof_aggr_time ? pend : clock( );

			time = 1000 * ( sim->end_profile[ sim->stack_level - 1 ] - sim->start_profile[ sim->stack_level - 1 ] ) / CLOCKS_PER_SEC;

			if ( time >= sim->prof_min_msecs )
			{
				set_lab_tit( );
				tit_updated = true;
				sim->plog_tag( "\n%-12.12s(%-.10s)\t=", "prof1", attr->label, lab_tit );
				sim->plog_tag( "%.4g\t", "highlight", val[ 0 ] );
				sim->plog( "t=" );
				sim->plog_tag( "%d\t", "highlight", sim->t );
				sim->plog( "msecs=" );
				sim->plog_tag( "%d\t", "highlight", time );
				sim->plog( "stack=" );
				sim->plog_tag( "%d\t", "highlight", sim->stack_level );
				sim->plog( "caller=%s%s%s", caller == NULL ? "LSD" : caller->attr->label, caller == NULL ? "" : "\ttrigger=", caller == NULL || sim->stack_log == NULL || sim->stack_log->prev == NULL ? "" : sim->stack_log->prev->label );
			}
		}

		// update debug log file
		if ( sim->log_file_ptr != NULL && sim->t >= sim->log_start && sim->t <= sim->log_stop )
		{
			if ( ! tit_updated )
				set_lab_tit( );

			fprintf( sim->log_file_ptr, "%s (%s)\t= %.4g\t(t=%d sim=%d caller=%s)\n", attr->label, lab_tit, val[ 0 ], sim->t, sim->nsim, caller == NULL ? "LSD" : caller->attr->label );
		}

		// open the debugger if required
		if ( sim->deb_set && sim->t == sim->deb_t && sim->liblnk != NULL && sim->liblnk->debugger != NULL && ( sim->watch_trigger || ( deb_cond == 0 && ( deb_mode == 'd' || deb_mode == 'W' || deb_mode == 'R' ) ) ) )
			( up->*sim->liblnk->debugger )( caller, attr->label, &val[ 0 ], false, "" );
		else
		{
			if ( ( sim->liblnk == NULL || sim->liblnk->debugger == NULL ) && deb_cond >= 1 && deb_cond <= 3 )
				deb_cond = -1;

			switch ( deb_cond )
			{
				case 0:
					break;
				case 1:
					if ( val[ 0 ] == deb_cnd_val )
						( up->*sim->liblnk->debugger )( caller, attr->label, &val[ 0 ], false, "" );
					break;
				case 2:
					if ( val[ 0 ] > deb_cnd_val )
						( up->*sim->liblnk->debugger )( caller, attr->label, &val[ 0 ], false, "" );
					break;
				case 3:
					if ( val[ 0 ] < deb_cnd_val )
						( up->*sim->liblnk->debugger )( caller, attr->label, &val[ 0 ], false, "" );
					break;
				default:
					sim->error_hard( "internal problem in LSD",
									 "if error persists, please contact developers",
									 true,
									 "conditional debug '%d' in variable '%s'",
									 deb_cond, attr->label );
					return -1;
			}
		}
#endif
		// remove the element from the stack
		if ( sim->stack_log != NULL && sim->stack_log->prev != NULL )
		{
			sim->stack_log = sim->stack_log->prev;
			delete sim->stack_log->next;
			sim->stack_log->next = NULL;
			sim->stack_level--;
		}
		else
		{
			sim->error_hard( "internal problem in LSD",
							 "if error persists, please contact developers",
							 true,
							 "failure while poping '%s' (in object '%s')",
							 attr->label, up->attr->label );
			return 0;
		}
	}

	under_computation = false;

	// if there is a pending deletion, try to do it now
	if ( sim->wait_delete != NULL )
	{
		if ( guard.owns_lock( ) )
			guard.unlock( );					// release lock

		sim->wait_delete->delete_obj( this );
	}

	return app; // by default the requested value is the last one, not yet computed

	error:

	eff_lag = ( param == 0 ) ? eff_lag : lag;

	if ( eff_lag > 0 )
		sim->error_hard( "invalid lag used",
						 "check your configuration (variable max lag) or\ncode (used lags in equation) to prevent this situation",
						 false,
						 "variable or function '%s' (object '%s') requested \nwith lag=%d but declared with lag=%d\nPossible fixes:\n- change the model configuration, declaring '%s' with at least lag=%d,\n- change the offender equation to request the value of '%s' with lag=%d maximum, or\n- enable USE_SAVED and mark '%s' to be saved (variables only)",
						 attr->label, up->attr->label, eff_lag, attr->num_lag, attr->label, eff_lag, attr->label, attr->num_lag, attr->label );
	else
		sim->error_hard( "invalid lag used",
						 "check your code (used lags in equation) to prevent negative lag",
						 false,
						 "variable or function '%s' (object '%s') requested \nwith lag=%d but negative lags are not allowed here\nPossible fix: use positive lag instead", attr->label, up->attr->label, eff_lag );

	return 0;
}


/*************************************************************
 CAL_WORKER
 Multi-thread worker for variable computation
 *************************************************************/
void lsd::worker::cal_worker( simulation *sim )
{
	int i;
	double app;

	// create try-catch block to capture exceptions in thread and reroute to main thread
	try
	{
		running = true;
		errored = false;

		// update object map and register all signal handlers
		uniq_lT lock_map( wrk_thr_ptr_lck );
		thread_id = std::this_thread::get_id( );
		worker_thread_ptr[ thread_id ] = this;
		lock_map.unlock( );
		handle_signals( signal_wrapper );

		free = true;

		while ( running )
		{
			// wait for variable calculation message
			uniq_lT lock_worker( worker_lck );
			run.wait( lock_worker, [ this ]{ return ! free; }  );

			// exit if shutdown or continue if already updated
			if ( running && v != NULL && v->last_update < sim->t )
			{	// prevent parallel computation of the same variable
				rec_uniqlT guard_var( v->var_comp_lck );

				// recheck if not computed during lock
				if ( v->last_update >= sim->t )
					goto end;

				if ( v->under_computation )
				{
					snprintf( err_msg1, MAX_BUFF_SIZE, "deadlock during parallel computation" );
					snprintf( err_msg2, MAX_BUFF_SIZE, "the equation for '%s' in object '%s' requested its own value\nwhile parallel-computing its current value", v->attr->label, v->up->attr->label );
					snprintf( err_msg3, MAX_BUFF_SIZE, "check your code to prevent this situation" );
					user_excpt = true;

					if ( sim->worker_errors( ) == 0 )
					{
						errored = true;
						throw;
					}
					else
					{
						errored = true;
						goto stop;
					}
				}

				v->under_computation = true;

				// compute the Variable's equation
				user_excpt = true;			// allow distinguishing among internal & user exceptions

#ifndef _TERM_
				if ( setjmp( env ) )		// allow recovering from signals
					return;
#endif
				try							// do it while catching exceptions to avoid obscure aborts
				{
					app = sim->_fun_( v, NULL );
				}
				catch ( ... )
				{
					if ( sim->error_hard_thread )
						pexcpt = nullptr;
					else
					{
						pexcpt = std::current_exception( );
						snprintf( err_msg1, MAX_BUFF_SIZE, "equation error" );
						snprintf( err_msg2, MAX_BUFF_SIZE, "an exception was detected while parallel-computing the equation\nfor '%s' in object '%s'", v->attr->label, v->up->attr->label );
						snprintf( err_msg3, MAX_BUFF_SIZE, "check your code to prevent this situation" );
					}

					if ( sim->worker_errors( ) == 0 )
					{
						errored = true;
						throw;
					}
					else
					{
						errored = true;
						goto stop;
					}
				}

				user_excpt = errored = false;

				// scale down the past values
				for ( i = 0; i < v->attr->num_lag; ++i )
					v->val[ v->attr->num_lag - i ] = v->val[ v->attr->num_lag - i - 1 ];
				v->val[ 0 ] = app;

				v->last_update = sim->t;

				// choose next update step for special updating variables
				if ( v->attr->period > 1 || v->attr->period_range > 0 )
				{
					v->next_update = sim->t + v->attr->period;
					if ( v->attr->period_range > 0 )
						v->next_update += sim->rnd_int( 0, v->attr->period_range );
				}

				v->under_computation = false;

				// if there is a pending object deletion, try to do it now
				if ( sim->wait_delete != NULL )
				{
					guard_var.unlock( );					// release lock
					sim->wait_delete->delete_obj( v );
				}
			}

		end:
			v = NULL;
			free = true;
			// create context to send signal to update scheduler if needed
			if ( ! sim->worker_ready )
			{
				uniq_lT lock_update( sim->var_update_lck );
				// recheck if still needed
				if ( ! sim->worker_ready )
				{
					sim->worker_ready = true;
					sim->upd_workers.notify_one( );
				}
			}
		}
	}
	catch ( ... )
	{
		// only capture exception if not already done
		if ( ! sim->error_hard_thread && pexcpt != nullptr )
		{
			pexcpt = std::current_exception( );
			snprintf( err_msg1, MAX_BUFF_SIZE, "parallel computation problem" );
			snprintf( err_msg2, MAX_BUFF_SIZE, "an exception was detected while parallel-computing the equation\nfor '%s' in object '%s'", v->attr->label, v->up->attr->label );
			snprintf( err_msg3, MAX_BUFF_SIZE, "disable parallel computation for this variable\nor check your code to prevent this situation" );
		}
	}

	stop:

	running = free = false;
}


/*************************************************************
 WORKER constructor
 *************************************************************/
lsd::worker::worker( void ) { }


/*************************************************************
 WORKER destructor
 *************************************************************/
lsd::worker::~worker( void )
{
	// command thread shutdown if running
	if ( running && ! errored )
	{
		uniq_lT lock_worker( worker_lck );
		running = free = false;
		run.notify_one( );
	}

	// wait for shutdown and check exception
	if ( worker_thread.joinable( ) && ! errored )
		worker_thread.join( );

	// remove thread id from threads map
	uniq_lT lock_map( wrk_thr_ptr_lck );
	worker_thread_ptr.erase( thread_id );
}


/*************************************************************
 SIGNAL
 Handle system signals in worker
 *************************************************************/
void lsd::worker::signal( int sig )
{
	char signame[ 16 ];

	switch ( sig )
	{
		case SIGMEM:
			strcpy( signame, "SIGMEM" );
			break;

		case SIGABRT:
			strcpy( signame, "SIGABRT" );
			break;

		case SIGFPE:
			strcpy( signame, "SIGFPE" );
			break;

		case SIGILL:
			strcpy( signame, "SIGILL" );
			break;

		case SIGSEGV:
			strcpy( signame, "SIGSEGV" );
			break;

		default:
			strcpy( signame, "Unknown signal" );
	}

	if ( v != NULL && v->attr != NULL && v->attr->label != NULL )
		snprintf( err_msg1, MAX_BUFF_SIZE, "\n\n%s: signal received while parallel-computing the equation\nfor '%s' in object '%s'\n(simulation %d). Disable parallel computation for this variable\nor check your code to prevent this situation.", signame, v->attr->label, v->up->attr->label, v->attr->cont->sim->nsim );
	else
		snprintf( err_msg1, MAX_BUFF_SIZE, "\n\n%s: signal received by a parallel worker thread\n(simulation %d).\nDisable parallel computation to prevent this situation.", signame, v->attr->cont->sim->nsim );

	// signal & kill thread
	signum = sig;
	free = false;
	running = false;

#ifndef _TERM_
	longjmp( env, 1 );				// recover from crash on user code
#endif
}


/*************************************************************
 SIGNAL_WRAPPER
 Reformat signal function format to comply with OS
 *************************************************************/
void lsd::worker::signal_wrapper( int signum )
{
	// call the appropriate worker object member function to handle signal
	worker_thread_ptr[ std::this_thread::get_id( ) ]->signal( signum );
}


/*************************************************************
 CAL
 Multi-thread CAL version (parallel computation)
 *************************************************************/
void lsd::worker::cal( variable *_v )
{
	uniq_lT worker_lock( worker_lck );
	v = _v;
	free = false;
	run.notify_one( );
}


/*************************************************************
 CHECK
 Check if worker is running and handle problems
 *************************************************************/
bool lsd::worker::check( void )
{
	if ( running && ! errored )				// nothing to do?
		return true;

	simulation *sim = v->attr->cont->sim;

	// only process first worker crash
	l_guardT lock_crash( sim->wrk_crash_lck );
	if ( ! sim->worker_crashed )
	{
		sim->worker_crashed = true;
		sim->user_exception = user_excpt;

		if ( signum >= 0 )
		{
			sim->plog( err_msg1 );
			signal_handler( signum );
		}
		else
		{
			if ( sim->error_hard_thread )
				sim->error_hard( sim->error_hard_msg1, sim->error_hard_msg3, true,
								 sim->error_hard_msg2 );
			else
			{
				if ( pexcpt != nullptr )
				{
					sim->error_hard( err_msg1, err_msg3, true, err_msg2 );
					rethrow_exception( pexcpt );
				}
				else
				{
					if ( v != NULL && v->attr != NULL && v->attr->label != NULL )
						sim->error_hard( "parallel computation problem",
										 "disable parallel computation for this variable\nor check your equation code to prevent this situation.\n\nPlease choose 'Quit LSD Browser' in the next dialog box",
										 true,
										 "while computing variable '%s' (object '%s') a multi-threading worker crashed",
										 v->attr->label, v->up->attr->label );
					else
						sim->error_hard( "parallel computation problem",
										 "disable parallel computation for this variable\nor check your equation code to prevent this situation.\n\nPlease choose 'Quit LSD Browser' in the next dialog box",
										 true,
										 "multi-threading worker crashed" );
				}
			}
		}
	}

	return false;
}


/*************************************************************
 PARALLEL_UPDATE
 Multi-thread scheduler for parallel updating
 *************************************************************/
void lsd::simulation::parallel_update( variable *v, object* p, object *caller )
{
	bool ready[ max_threads ], wait = false;
	int i, nt, wait_time;
	clock_t pstart = 0;
	bridge *cb;
	object *co;
	variable *cv = NULL;

	// prevent concurrent parallel update and multi-threading in a single core
	if ( parallel_ready && max_threads > 1 )
		parallel_ready = false;
	else
	{
		v->cal( caller, 0 );
		return;
	}

	// find the beginning of the linked list chain for current object
	cb = p->up->search_bridge( p->attr );

	// if single instanced object, update as usual
	if ( cb->head == NULL || cb->head->next == NULL )
	{
		v->cal( caller, 0 );
		return;
	}

	// set ready worker threads
	for ( nt = 0, i = 0; i < max_threads; ++i )
	{
		ready[ i ] = workers[ i ].free;
		if ( ! ready[ i ] )
			++nt;
	}

	if ( nt > 0 )
	{
		error_hard( "parallel computation problem",
					"disable parallel computation for this variable or check your equation code to prevent this situation.\n\nPlease choose 'Quit LSD Browser' in the next dialog box",
					true,
					"variable '%s' (object '%s') %d parallel worker(s) crashed", v->attr->label, v->up->attr->label, i );
		return;
	}

	// scan all instances of current object under current parent
	for ( co = cb->head; co != NULL; co = co->next )
	{
		cv = co->search_var( co, v->attr );

		// compute only if not updated
		if ( cv != NULL && cv->last_update < t && t >= cv->next_update )
		{
			// if no worker available, wait to free existing ones
			while ( nt >= max_threads )
			{
				// if starting wait, reset chronometer
				if ( ! wait )
				{
					wait = true;
					pstart = clock( );
				}
				else		// already waiting
				{
					wait_time = ( clock( ) - pstart ) / CLOCKS_PER_SEC;
					if ( wait_time > MAX_WAIT_TIME )
					{
						error_hard( "deadlock during parallel computation",
									"disable parallel computation for this variable or check your equation code to prevent this situation.\n\nPlease choose 'Quit LSD Browser' in the next dialog box",
									true,
									"variable '%s' (object '%s') took more than %d seconds\nwhile computing value for time step %d", cv->attr->label, cv->up->attr->label, MAX_WAIT_TIME, t );
						return;
					}
				}

				// look for free and stopped workers
				for ( i = 0; i < max_threads; ++i )
				{
					workers[ i ].check( );

					if ( ! ready[ i ] && workers[ i ].free )
					{
						--nt;
						ready[ i ] = true;
						wait = false;
					}
				}

				// sleep process until first worker is free
				if ( nt >= max_threads )
				{
					uniq_lT lock_update( var_update_lck );
					worker_ready = false;
					if ( ! upd_workers.wait_for ( lock_update, std::chrono::milliseconds( MAX_VAR_TIMEOUT ), [ & ]{ return ! worker_ready; } ) )
						{
							worker_ready = true;
							plog( "\nWarning: workers timeout (%d millisecs.), continuing...", MAX_VAR_TIMEOUT );
							break;
						}
				}

				// recheck running workers
				for ( i = 0; i < max_threads; ++i )
					workers[ i ].check( );
			}

			// find first free worker
			for ( i = 0; ! ready[ i ] && i < max_threads; ++i );
			// if something go wrong, wait fist worker (always there)
			if ( i >= max_threads )
			{
				error_hard( "parallel computation problem",
							"disable parallel computation for this variable or check your equation code to prevent this situation.\n\nPlease choose 'Quit LSD Browser' in the next dialog box",
							true,
							"variable '%s' (object '%s') had a multi-threading inconsistency,\nmaybe a deadlock state", cv->attr->label, cv->up->attr->label );
				return;
			}
			else
			{	// start the computation of the current variable instance in the worker
				++nt;
				ready[ i ] = false;
				workers[ i ].cal( cv );
			}
		}
	}

	// wait last threads finish processing
	while ( nt > 0 )
	{	// if starting wait, reset chronometer
		if ( ! wait )
		{
			wait = true;
			pstart = clock( );
		}
		else		// already waiting
		{
			wait_time = ( clock( ) - pstart ) / CLOCKS_PER_SEC;
			if ( wait_time > MAX_WAIT_TIME )
			{
				error_hard( "deadlock during parallel computation",
							"disable parallel computation for this variable or check your equation code to prevent this situation.\n\nPlease choose 'Quit LSD Browser' in the next dialog box",
							true,
							"variable '%s' (object '%s') took more than %d seconds\nwhile computing value for time step %d", cv != NULL ? cv->up->attr->label : "", cv != NULL ? cv->attr->label : "(none)", MAX_WAIT_TIME, t );
				return;
			}
		}

		// wait till each thread finishes
		for ( i = 0; i < max_threads; ++i )
		{
			if ( ! ready[ i ] && workers[ i ].free )
			{
				--nt;
				ready[ i ] = true;
				wait = false;
			}

			// check worker problem
			workers[ i ].check( );
		}
	}

	// re-enable concurrent parallel update
	parallel_ready = true;
}


/*************************************************************
 WORKER_ERRORS
 Check how many workers are in error condition
 *************************************************************/
int lsd::simulation::worker_errors( void )
{
	int i, count;

	if ( workers == NULL )
		return 0;

	for ( count = i = 0; i < max_threads; ++i )
		if ( workers[ i ].errored )
			++count;

	return count;
}
