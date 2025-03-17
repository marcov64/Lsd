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
 VARIABLE  copy constructor
 ATTENTION: allocation for internal arrays is not duplicated!
 copied variable will share the same allocated arrays
 Useful only to move variable among different data structures
 *************************************************************/
lsd::variable::variable( const variable & v )
{
	copy_state( & v );

	label = v.label;
	val = v.val;
	up = v.up;
	next = v.next;
	dummy = v.dummy;
	under_computation = v.under_computation;
	lab_tit = v.lab_tit;
	data = v.data;
	end = v.end;
	next_update = v.next_update;
	start = v.start;
}


/*************************************************************
 COPY_STATE
 Copy another variable static state, except for allocated
 arrays and simulation/structure position
 *************************************************************/
void lsd::variable::copy_state( const variable *ex )
{
	if ( ex == NULL )
		return;

	initialized = ex->initialized;
	integer = ex->integer;
	observe = ex->observe;
	parallel = ex->parallel;
	plot = ( ! ex->up->sim->running ) ? ex->plot : false;
	save = ex->save;
	savei = ex->savei;

	deb_mode = ex->deb_mode;
	ini_val = ex->ini_val;
	max_val = ex->max_val;
	min_val = ex->min_val;

	delay = ex->delay;
	delay_range = ex->delay_range;
	num_lag = ex->num_lag;
	param = ex->param;
	period = ex->period;
	period_range = ex->period_range;

	deb_cnd_val = ex->deb_cnd_val;

	eq_func = ex->eq_func;

	deb_cond = ex->deb_cond;
	last_update = ex->last_update;
}


/*************************************************************
 ~VARIABLE destructor
 *************************************************************/
lsd::variable::~variable( void )
{
	delete [ ] label;
	delete [ ] val;
	delete [ ] lab_tit;
	free( data );		// use C stdlib to be able to deallocate memory for deleted objects
}


/*************************************************************
 INIT
 *************************************************************/
void lsd::variable::init( object *_up, const char *_label, variable *ex )
{
	// prevent concurrent use by more than one thread
	rec_lguardT lock( var_comp_lck );

	copy_state( ex );

	up = _up;

	if ( _label == NULL && ex != NULL && ex->label != NULL )
		_label = ex->label;

	if ( _label != NULL )
	{
		label = new char[ strlen( _label ) + 1 ];
		strcpy( label, _label );
	}

	if ( ex != NULL && ex->val != NULL )
	{
		val = new double[ num_lag + 1 ];
		for ( int i = 0; i <= num_lag; ++i )
			val[ i ] = ex->val[ i ];
	}
}


/*************************************************************
 EMPTY
 *************************************************************/
void lsd::variable::empty( bool no_lock )
{

	if ( up->sim->running && ! no_lock )
	{
		// prevent concurrent use by more than one thread
		rec_lguardT lock( var_comp_lck );
	}

	if ( up->sim->running && ( label == NULL || val == NULL ) )
	{
		up->sim->error_hard( "internal problem in LSD",
							 "if error persists, please contact developers",
							 true,
							 "failure while deallocating variable %s", label );
		return;
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
		return cur->search_var( NULL, label, true );

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
		if ( integer )
			val = round( val );

		if ( std::isfinite( max_val ) && val > max_val )
			val = max_val;
		else
			if ( std::isfinite( min_val ) && val < min_val )
				val = min_val;
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
	simulation *sim = up->sim;

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
			strncpy( sim->watch_elem, label, MAX_ELEM_LENGTH );
		}

		return val[ 0 ];				// it's a parameter, ignore lags
	}

	// prepare mutex for variables and functions updated in multiple threads
	rec_uniqlT guard( var_comp_lck, std::defer_lock );

	if ( param == 0 )					// it's a variable
	{
		// invalid lag or value not saved yet
		if ( lag > num_lag && ( sim->no_saved || ! ( save || savei ) || sim->t - lag < start ) )
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

			if ( eff_lag > num_lag )	// in principle, invalid lag
			{
				if ( sim->no_saved || ! ( save || savei ) )	// and not saved
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
					strncpy( sim->watch_elem, label, MAX_ELEM_LENGTH );
				}

				return( val[ 0 ] );
			}

			// wait for computation of this variable by other threads
			if ( sim->parallel_mode && ! dummy )
				guard.lock( );

			if ( last_update >= sim->t )// recheck if not computed during lock
				return( val[ 0 ] );
		}
	}
	else								// function
	{
		if ( lag < 0 || lag > num_lag ) // with invalid lag
			goto error;

		if ( lag > 0 )					// lagged value
			return val[ lag - 1 ];

		if ( caller == NULL )			// update or inadequate caller
			return val[ 0 ];

		// wait for computation of this function by other threads
		if ( sim->parallel_mode && ! dummy )
			 guard.lock( );
	}

	// there is a value to be computed

	if ( under_computation )
	{
		sim->error_hard( "deadlock",
						 "check your equation code to prevent this situation\nprobably using the variable lagged value instead",
						 true,
						 "equation for '%s' (object '%s') requested \nits own value while computing its current value", label, up->label );
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
			strcpyn( sim->stack_log->next->label, label, MAX_ELEM_LENGTH );
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
							 label, up->label );
			return 0;
		}

#ifndef _TERM_
		if ( sim->stack_info >= sim->stack_level && ( ! sim->prof_obs_only || observe ) )
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
		sim->plog( "\n\nAn exception was detected while computing the equation \nfor '%s' requested by object '%s'", label, caller == NULL ? "(none)" : caller->label );
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
			sim->plog( "\n\nAn unknown problem was detected while computing the equation \nfor '%s' requested by object '%s'", label, caller == NULL ? "(none)" : caller->label );
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

	for ( i = 0; i < num_lag; ++i ) // scale down the past values
		val[ num_lag - i ] = val[ num_lag - i - 1 ];

	val[ 0 ] = app;

	last_update = sim->t;

	// choose next update step for special updating variables
	if ( period > 1 || period_range > 0 )
	{
		next_update = sim->t + period;
		if ( period_range > 0 )
			next_update += sim->rnd_int( 0, period_range );
	}

	if ( sim->fast_mode == 0 && ! sim->parallel_mode )
	{
#ifndef _TERM_
		if ( sim->prof_aggr_time )
		{
			pend = clock( );
			time = pend - pstart;

			if ( ( ! sim->prof_obs_only || observe ) && time > sim->prof_min_msecs )
			{
				strT var_name = label;
				sim->prof_times[ var_name ].ticks += time;
				sim->prof_times[ var_name ].comp++;
			}
		}

		tit_updated = false;
		if ( sim->stack_info >= sim->stack_level && ( ! sim->prof_obs_only || observe ) )
		{
			sim->end_profile[ sim->stack_level - 1 ] = sim->prof_aggr_time ? pend : clock( );

			time = 1000 * ( sim->end_profile[ sim->stack_level - 1 ] - sim->start_profile[ sim->stack_level - 1 ] ) / CLOCKS_PER_SEC;

			if ( time >= sim->prof_min_msecs )
			{
				set_lab_tit( );
				tit_updated = true;
				sim->plog_tag( "\n%-12.12s(%-.10s)\t=", "prof1", label, lab_tit );
				sim->plog_tag( "%.4g\t", "highlight", val[ 0 ] );
				sim->plog( "t=" );
				sim->plog_tag( "%d\t", "highlight", sim->t );
				sim->plog( "msecs=" );
				sim->plog_tag( "%d\t", "highlight", time );
				sim->plog( "stack=" );
				sim->plog_tag( "%d\t", "highlight", sim->stack_level );
				sim->plog( "caller=%s%s%s", caller == NULL ? "LSD" : caller->label, caller == NULL ? "" : "\ttrigger=", caller == NULL || sim->stack_log == NULL || sim->stack_log->prev == NULL ? "" : sim->stack_log->prev->label );
			}
		}

		// update debug log file
		if ( sim->log_file_ptr != NULL && sim->t >= sim->log_start && sim->t <= sim->log_stop )
		{
			if ( ! tit_updated )
				set_lab_tit( );

			fprintf( sim->log_file_ptr, "%s (%s)\t= %.4g\t(t=%d sim=%d caller=%s)\n", label, lab_tit, val[ 0 ], sim->t, sim->nsim, caller == NULL ? "LSD" : caller->label );
		}

		// open the debugger if required
		if ( sim->deb_set && sim->t == sim->deb_t && sim->liblnk != NULL && sim->liblnk->debugger != NULL && ( sim->watch_trigger || ( deb_cond == 0 && ( deb_mode == 'd' || deb_mode == 'W' || deb_mode == 'R' ) ) ) )
			( up->*sim->liblnk->debugger )( caller, label, &val[ 0 ], false, "" );
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
						( up->*sim->liblnk->debugger )( caller, label, &val[ 0 ], false, "" );
					break;
				case 2:
					if ( val[ 0 ] > deb_cnd_val )
						( up->*sim->liblnk->debugger )( caller, label, &val[ 0 ], false, "" );
					break;
				case 3:
					if ( val[ 0 ] < deb_cnd_val )
						( up->*sim->liblnk->debugger )( caller, label, &val[ 0 ], false, "" );
					break;
				default:
					sim->error_hard( "internal problem in LSD",
									 "if error persists, please contact developers",
									 true,
									 "conditional debug '%d' in variable '%s'",
									 deb_cond, label );
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
							 label, up->label );
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
						 label, up->label, eff_lag, num_lag, label, eff_lag,
						 label, num_lag, label );
	else
		sim->error_hard( "invalid lag used",
						 "check your code (used lags in equation) to prevent negative lag",
						 false,
						 "variable or function '%s' (object '%s') requested \nwith lag=%d but negative lags are not allowed here\nPossible fix: use positive lag instead", label, up->label, eff_lag );

	return 0;
}


/*************************************************************
 CAL_WORKER
 Multi-thread worker for variable computation
 *************************************************************/
void lsd::worker::cal_worker( void )
{
	int i;
	double app;
	simulation *sim = v->up->sim;

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
					snprintf( err_msg2, MAX_BUFF_SIZE, "the equation for '%s' in object '%s' requested its own value\nwhile parallel-computing its current value", v->label, v->up->label );
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
						snprintf( err_msg2, MAX_BUFF_SIZE, "an exception was detected while parallel-computing the equation\nfor '%s' in object '%s'", v->label, v->up->label );
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
				for ( i = 0; i < v->num_lag; ++i )
					v->val[ v->num_lag - i ] = v->val[ v->num_lag - i - 1 ];
				v->val[ 0 ] = app;

				v->last_update = sim->t;

				// choose next update step for special updating variables
				if ( v->period > 1 || v->period_range > 0 )
				{
					v->next_update = sim->t + v->period;
					if ( v->period_range > 0 )
						v->next_update += sim->rnd_int( 0, v->period_range );
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
			snprintf( err_msg2, MAX_BUFF_SIZE, "an exception was detected while parallel-computing the equation\nfor '%s' in object '%s'", v->label, v->up->label );
			snprintf( err_msg3, MAX_BUFF_SIZE, "disable parallel computation for this variable\nor check your code to prevent this situation" );
		}
	}

	stop:

	running = free = false;
}


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

	if ( v != NULL && v->label != NULL	)
		snprintf( err_msg1, MAX_BUFF_SIZE, "\n\n%s: signal received while parallel-computing the equation\nfor '%s' in object '%s'\n(simulation %d). Disable parallel computation for this variable\nor check your code to prevent this situation.", signame, v->label, v->up->label != NULL ? v->up->label : "(none)", v->up->sim->nsim );
	else
		snprintf( err_msg1, MAX_BUFF_SIZE, "\n\n%s: signal received by a parallel worker thread\n(simulation %d).\nDisable parallel computation to prevent this situation.", signame, v->up->sim->nsim );

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

	simulation *sim = v->up->sim;

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
					if ( v != NULL && v->label != NULL )
						sim->error_hard( "parallel computation problem",
										 "disable parallel computation for this variable\nor check your equation code to prevent this situation.\n\nPlease choose 'Quit LSD Browser' in the next dialog box",
										 true,
										 "while computing variable '%s' (object '%s') a multi-threading worker crashed",
										 v->label,
										 v->up->label != NULL ? v->up->label : "(none)" );
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
	cb = p->up->search_bridge( p->label );

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
					"variable '%s' (object '%s') %d parallel worker(s) crashed", v->label, v->up->label, i );
		return;
	}

	// scan all instances of current object under current parent
	for ( co = cb->head; co != NULL; co = co->next )
	{
		cv = co->search_var( co, v->label );

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
									"variable '%s' (object '%s') took more than %d seconds\nwhile computing value for time step %d", cv->label, cv->up->label, MAX_WAIT_TIME, t );
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
							"variable '%s' (object '%s') had a multi-threading inconsistency,\nmaybe a deadlock state", cv->label, cv->up->label );
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
							"variable '%s' (object '%s') took more than %d seconds\nwhile computing value for time step %d", cv != NULL ? cv->up->label : "", cv != NULL ? cv->label : "", MAX_WAIT_TIME, t );
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
