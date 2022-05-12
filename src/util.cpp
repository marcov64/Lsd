/*************************************************************

	LSD 8.0 - May 2022
	written by Marco Valente, Universita' dell'Aquila
	and by Marcelo Pereira, University of Campinas

	Copyright Marco Valente and Marcelo Pereira
	LSD is distributed under the GNU General Public License

	See Readme.txt for copyright information of
	third parties' code used in LSD

 *************************************************************/

/*************************************************************
UTIL.CPP
Contains a set of utilities for different parts of the
program.

The main functions contained in this file are:

- void cmd( const char *cc );
Standard routine to send the message string cc to the TCL interpreter in order
to execute a command for the graphical interfaces.
It should be enough to make a call to Tcl_Eval. But there are problems due to the
fact that this standard tcl function wants the string cc to be writable. Hence,
it shouldn't work when a constant string is passed. Actually, it worked under windows
but not under unix. Instead, I use Tcl_VarEval, that allows to use pieces
of strings (including the last terminating character NULL) and  it does not
require a writable string.

- void plog( const char *m );
print  message string m in the Log screen.

- FILE *search_str( const char *name, const char *str )
given a string name, returns the file corresponding to name, and the current
position of the file is just after str. Think I don't use any longer.

- FILE *search_data_str( const char *name, char *init, char *str )
given a string name, returns the file with that name and the current position
placed immediately after the string str found after the string init. Needed to
not get confused managing the data files, where the same string appears twice,
in the structure definition and in the data section.

- FILE *search_data_ent( const char *name, variable *v )
given the file name name, the routine searches for the data line for the variable
(or parameter) v. It is not messed up by same labels for variables and objects.

- other various mathematical routines
*************************************************************/

#include "decl.h"

int **lattice = NULL;					// lattice data colors array
int rows = 0;							// lattice size
int columns = 0;
int error_count;						// error counters
int	normErrCnt, lnormErrCnt, gammaErrCnt, bernoErrCnt, poissErrCnt;
int geomErrCnt, binomErrCnt, cauchErrCnt, chisqErrCnt, expErrCnt;
int fishErrCnt, studErrCnt, weibErrCnt, betaErrCnt, paretErrCnt, alaplErrCnt;
double dimW = 0;						// lattice screen size
double dimH = 0;

#ifndef _NP_
mutex error;
#endif


/*********************************
PLOG
Print message on the log window
*********************************/
void plog( const char *cm, ... )
{
	static va_list argptr;

	va_start( argptr, cm );
	plog_backend( cm, "", argptr );
	va_end( argptr );
}


/*********************************
PLOG_TAG
The optional tag parameter has to correspond to the log window existing tags
*********************************/
void plog_tag( const char *cm, const char *tag, ... )
{
	static va_list argptr;

	va_start( argptr, tag );
	plog_backend( cm, tag, argptr );
	va_end( argptr );
}


/*********************************
PLOG_BACKEND
Back-end to plog and plog_tag
*********************************/
#define NUM_TAGS 7
const char *tags[ NUM_TAGS ] = { "", "highlight", "table", "series", "prof1", "prof2", "bar" };

void plog_backend( const char *cm, const char *tag, va_list arg )
{
	static bool bufdyn;
	static char *buffer, *message, bufstat[ MAX_BUFF_SIZE ], msgstat[ MAX_BUFF_SIZE ];
	static bool tag_ok;
	static int i, j, reqsz, sz;
	static va_list argcpy;

#ifndef _NW_
	if ( ! tk_ok || ! log_ok )
		return;
#endif

#ifndef _NP_
	// abort if not running in main LSD thread
	if ( this_thread::get_id( ) != main_thread )
		return;
#endif

	buffer = bufstat;
	message = msgstat;
	va_copy( argcpy, arg );

	reqsz = vsnprintf( buffer, MAX_BUFF_SIZE, cm, arg );

	if ( reqsz < 0 )
	{
#ifndef _NW_
		log_tcl_error( true, "Invalid text message", "Cannot expand message '%s...'", cm );
#else
		fprintf( stderr, "\nCannot expand message '%s...'\n", cm );
#endif
		return;
	}

	// handle very large messages
	if ( reqsz >= MAX_BUFF_SIZE )
	{
		buffer = new char[ reqsz + 1 ];
		sz = vsnprintf( buffer, reqsz + 1, cm, argcpy );

		if ( reqsz < 0 || sz > reqsz )
		{
#ifndef _NW_
			log_tcl_error( true, "Invalid text message", "Cannot expand message '%s...'", cm );
#else
			fprintf( stderr, "\nCannot expand message '%s...'\n", cm );
#endif
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

#ifdef _NW_
	printf( "%s", message );
	fflush( stdout );
#else
	for ( tag_ok = false, i = 0; i < NUM_TAGS; ++i )
		if ( ! strcmp( tag, tags[ i ] ) )
			tag_ok = true;

	// handle the "bar" pseudo tag
	if ( strcmp( tag, "bar" ) )
		on_bar = false;

	if ( tag_ok )
	{
		cmd( "set log_ok 0" );
		cmd( "if { ! [ catch { package present Tk 8.6 } ] && ! [ catch { set tk_ok [ winfo exists . ] } ] && $tk_ok } { \
				catch { set log_ok [ winfo exists .log ] } \
			}" );
		cmd( "if $log_ok { .log.text.text.internal see [ .log.text.text.internal index insert ] }" );
		cmd( "if $log_ok { catch { .log.text.text.internal insert end \"%s\" %s } }", message, tag );
		cmd( "if $log_ok { .log.text.text.internal see end }" );
	}
	else
		plog( "\nError: invalid tag, message ignored:\n%s\n", message );
#endif

	message_logged = true;

	if ( bufdyn )
	{
		delete [ ] buffer;
		delete [ ] message;
	}
}


/***********
ERROR_HARD
Procedure called when an unrecoverable error occurs.
Information about the state of the simulation when the error
occurred is provided. Users can abort the program or analyze
the results collected up the latest time step available.
*************/
void error_hard( const char *boxTitle, const char *boxText, bool defQuit, const char *logFmt, ... )
{
	if ( quit == 2 )		// simulation already being stopped
		return;

	static char logText[ MAX_BUFF_SIZE ];
	static va_list argptr;

	va_start( argptr, logFmt );
	vsnprintf( logText, MAX_BUFF_SIZE, logFmt, argptr );
	va_end( argptr );

#ifndef _NP_
	// prevent concurrent use by more than one thread
	lock_guard < mutex > lock( error );

	// abort worker and park message if not running in main LSD thread
	if ( this_thread::get_id( ) != main_thread )
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
#endif

#ifndef _NW_
	if ( running )			// handle running events differently
	{
		cmd( "if [ winfo exists .deb ] { destroytop .deb }" );
		deb_log( false );	// close any open debug log file
		reset_plot( );		// show & disable run-time plot
		set_buttons_run( false );

		plog_tag( "\n\nError detected at case (time step): %d", "highlight", t );
		plog( "\n\nError: %s\nDetails: %s", boxTitle, logText );
		if ( ! parallel_mode && stacklog != NULL && stacklog->vs != NULL )
			plog( "\nOffending code contained in the equation for variable: '%s'", stacklog->vs->label );
		plog( "\nSuggestion: %s", boxText );
		print_stack( );
		cmd( "focustop .log" );
		cmd( "ttk::messageBox -parent . -title Error -type ok -icon error -message \"[ string totitle {%s} ]\" -detail \"[ string totitle {%s} ].\n\nMore details are available in the Log window.\n\nSimulation cannot continue.\"", boxTitle, boxText  );
	}
	else
	{
		plog( "\n\nError: %s\nDetails: %s", boxTitle, logText );
		plog( "\nSuggestion: %s\n", boxText );
		cmd( "ttk::messageBox -parent . -title Error -type ok -icon error -message \"[ string totitle {%s} ]\" -detail \"[ string totitle {%s} ].\n\nMore details are available in the Log window.\"", boxTitle, boxText  );
	}
#endif

	if ( ! running )
		return;

	quit = 2;				// do not continue simulation

#ifndef _NW_
	uncover_browser( );
	cmd( "focustop .log" );

	cmd( "set err %d", ( defQuit || worker_errors( ) ) > 0 ? 1 : 2 );

	cmd( "newtop .cazzo Error" );

	cmd( "ttk::frame .cazzo.t" );
	cmd( "ttk::label .cazzo.t.l -style hl.TLabel -text \"An error occurred during the simulation\"" );
	cmd( "pack .cazzo.t.l -pady 10" );
	cmd( "ttk::label .cazzo.t.l1 -justify center -text \"Information about the error is reported in the log window.\nPartial results are available in the LSD browser.\"" );
	cmd( "pack .cazzo.t.l1" );

	cmd( "ttk::frame .cazzo.e" );
	cmd( "ttk::label .cazzo.e.l -text \"Choose one option to continue\"" );

	cmd( "ttk::frame .cazzo.e.b -relief solid -borderwidth 1 -padding [ list $frPadX $frPadY ]" );
	cmd( "ttk::radiobutton .cazzo.e.b.r -variable err -value 2 -text \"Return to LSD Browser to edit the model configuration\"" );
	cmd( "ttk::radiobutton .cazzo.e.b.d -variable err -value 3 -text \"Open LSD Debugger on the offending variable and object instance\"" );
	cmd( "ttk::radiobutton .cazzo.e.b.e -variable err -value 1 -text \"Quit LSD Browser to edit the model equations' code in LMM\"" );
	cmd( "pack .cazzo.e.b.r .cazzo.e.b.d .cazzo.e.b.e -anchor w" );

	cmd( "pack .cazzo.e.l .cazzo.e.b" );

	cmd( "pack .cazzo.t .cazzo.e -padx 5 -pady 5" );

	cmd( "okhelp .cazzo b { set choice 1 }  { LsdHelp debug.html#crash }" );

	cmd( "showtop .cazzo centerW" );
	cmd( "mousewarpto .cazzo.b.ok" );

	if ( parallel_mode || fast_mode != 0 )
		cmd( ".cazzo.e.b.d configure -state disabled" );

	if ( worker_errors( ) > 0 )
		cmd( ".cazzo.e.b.r configure -state disabled" );

	choice = 0;
	while ( choice == 0 )
		Tcl_DoOneEvent( 0 );

	cmd( "destroytop .cazzo" );

	int err = get_int( "err" );

	if ( err == 3 )
	{
		if ( ! parallel_mode && fast_mode == 0 && stacklog != NULL &&
			 stacklog->vs != NULL && stacklog->vs->label != NULL )
		{
			char err_msg[ MAX_LINE_SIZE ];
			double useless = -1;
			snprintf( err_msg, MAX_LINE_SIZE, "%s (ERROR)", stacklog->vs->label );
			deb( stacklog->vs->up, NULL, err_msg, & useless );
		}

		err = 2;
	}

	if ( err == 2 )
	{
		// do run( ) cleanup
		empty_stack( );
		unsavedData = true;				// flag unsaved simulation results
		running = false;

		// run user closing function, reporting error appropriately
		user_exception = true;
		close_sim( );
		user_exception = false;

		reset_end( root );
		root->emptyturbo( );
		uncover_browser( );

#ifndef _NP_
		// stop multi-thread workers
		delete [ ] workers;
		workers = NULL;
#endif
		throw ( int ) 919293;			// force end of run() (in lsdmain.cpp)
	}

	if ( err == 1 )
	{
		save_pos( currObj );			// save browser position in structure
		update_model_info( );			// save windows positions if appropriate
	}

#else

	fprintf( stderr, "\nError: %s\n(%s)\n", boxTitle, logText );
#endif

	myexit( 13 );
}


/****************************
PRINT_STACK
Print the state of the stack in the log window.
This tells the user which variable is computed
because of other equations' request.
*****************************/
void print_stack( void )
{
	lsdstack *app;

	if ( parallel_mode )
	{
		plog( "\n\nRunning in parallel mode, list of variables under computation not available\n(You may disable parallel computation in menu 'Run', 'Simulation Settings')\n" );
		return;
	}

	if ( fast_mode > 0 )
	{
		plog( "\n\nRunning in fast mode, list of variables under computation not available\n(You may temporarily not use fast mode to get additional information)\n" );
		return;
	}

	plog( "\n\nList of variables currently under computation" );
	plog( "\n\nLevel\tVariable Label" );

	for ( app = stacklog; app != NULL; app = app->prev )
		plog( "\n%d\t%s", app->ns, app->label );

	plog( "\n\n(the zero-level variable is computed by the simulation manager, \nwhile possible other variables are triggered by the lower level ones\nbecause necessary for completing their computation)\n" );
}


/****************************************************
SEARCH_STR
****************************************************/
FILE *search_str( const char *fname, const char *str )
{
	FILE *f;
	char got[ MAX_LINE_SIZE ];

	f = fopen( fname, "r" );
	if ( f == NULL )
		return NULL;

	fscanf( f, "%999s", got );
	for ( int i = 0; strcmp( got, str ) && i < MAX_FILE_TRY; ++i )
		if ( fscanf( f, "%999s", got ) == EOF )
			return NULL;

	if ( ! strcmp( got, str ) )
		return f;
	else
		return NULL;
}


/****************************************************
SEARCH_DATA_STR
****************************************************/
FILE *search_data_str( const char *name, const char *init, const char *str )
{
	FILE *f;
	char got[ MAX_LINE_SIZE ];

	f = fopen( name, "r" );
	if ( f == NULL )
		return NULL;

	fscanf( f, "%999s", got );
	for ( int i = 0; strcmp( got, init ) && i < MAX_FILE_TRY; ++i )
		if ( fscanf( f, "%999s", got ) == EOF )
			return NULL;

	if ( strcmp( got, init ) )
		return NULL;

	for ( int i = 0; strcmp( got, str ) && i < MAX_FILE_TRY; ++i )
		if ( fscanf( f, "%999s", got ) == EOF )
			return NULL;

	if ( ! strcmp( got, str ) )
		return f;
	else
		return NULL;
}


/****************************************************
SEARCH_DATA_ENT
****************************************************/
FILE *search_data_ent( const char *name, variable *v )
{
	FILE *f;
	char got[ MAX_LINE_SIZE ];
	char temp[ MAX_ELEM_LENGTH ];
	char temp1[ MAX_LINE_SIZE ];
	char typ[ MAX_ELEM_LENGTH ];

	f = fopen( name, "r" );
	if ( f == NULL )
		return NULL;

	fscanf( f, "%999s", got );
	for ( int i = 0; strcmp( got, "DATA" ) && i < MAX_FILE_TRY; ++i )
		if ( fscanf( f, "%999s", got ) == EOF )
			return NULL;

	if ( strcmp( got, "DATA" ) )
		return NULL;

	strcpyn( temp, ( v->up )->label, MAX_ELEM_LENGTH );	// search for the section of the Object
	fscanf( f, "%999s", temp1 );
	fscanf( f, "%999s", got );

	for ( int i = 0; ( strcmp( got, temp ) || strcmp( temp1,"Object:" ) ) && i < MAX_FILE_TRY; ++i )
	{
		strcpyn( temp1, got, MAX_LINE_SIZE );
		if ( fscanf( f, "%999s", got ) == EOF )
			return NULL;
	}

	if ( strcmp( got, temp ) || strcmp( temp1,"Object:" ) )
		return NULL;

	// hopefully, we are at the beginning of the vars in the correct object
	if ( v->param == 1 )
		strcpy( typ, "Param:" );
	else
		if ( v->param == 2 )
			strcpy( typ, "Func:" );
		else
			strcpy( typ, "Var:" );

	fscanf( f, "%999s", temp1 );		// search for the line of the var
	fscanf( f, "%999s", got );

	for ( int i = 0; ( strcmp( got, v->label ) || strcmp( temp1, typ ) ) && i < MAX_FILE_TRY; ++i )
	{
		strcpyn( temp1, got, MAX_LINE_SIZE );
		if ( fscanf( f, "%999s", got ) == EOF )
			return NULL;
	}

	if ( strcmp( got, v->label ) || strcmp( temp1, typ ) )
		return NULL;
	else
		return f;
}


/***************************************************
SET_COUNTER
***************************************************/
void set_counter( object *o )
{
	int i;
	bridge *cb;
	object *cur;

	if ( o->up == NULL )
		return;

	set_counter( o->up );

	// find the bridge which contains the object
	cb = o->up->search_bridge( o->label );

	if ( cb->counter_updated )
		return;

	for ( cur = cb->head, i = 1; cur != NULL; cur = cur->next, ++i )
		if ( cur->lstCntUpd < t )		// don't update more than once per period
		{								// to avoid deletions to change counters
			cur->acounter = i;
			cur->lstCntUpd = t;
		}

	cb->counter_updated = true;
}


/***************************************************
SET_LAB_TIT
Ensure that all objects on top of the variables have the counter updated,
and then writes the lab_tit field.
lab_tit indicates the position of the object containing the variables in the model.
***************************************************/
void set_lab_tit( variable *var )
{
	bool first = true;
	char app[ MAX_LINE_SIZE ], app1[ 2 * MAX_LINE_SIZE ];
	object *cur;

	if ( var->up->up == NULL )
	{
		// this is the Root of the model
		if ( var->lab_tit != NULL )
			return; 					// already done in the past

		var->lab_tit = new char[ strlen( "R" ) + 1 ];
		strcpy( var->lab_tit, "R" );

		return;
	}

	for ( cur = var->up; cur->up != NULL; cur = cur->up )
	{
		// find the bridge containing the variable
		set_counter( cur );
		if ( ! first )
			snprintf( app1, 2 * MAX_LINE_SIZE, "%d_%s", cur->acounter, app );
		else
		{
			first = false;
			snprintf( app1, 2 * MAX_LINE_SIZE, "%d", cur->acounter );
		}

		strcpyn( app, app1, MAX_LINE_SIZE );
	}

	if ( var->lab_tit != NULL )
		delete [ ] var->lab_tit;

	var->lab_tit = new char[ strlen( app ) + 1 ];
	strcpy( var->lab_tit, app );
}


/*****************************************************************************
SET_BLUEPRINT
copy the naked structure of the model into another object, called blueprint,
to be used for adding objects without example
******************************************************************************/
void set_blueprint( object *container, object *r )
{
	bridge *cb, *cb1;
	object *cur, *cur1;
	variable *cv;

	if ( r == NULL )
		return;

	for ( cv = r->v; cv != NULL; cv = cv->next )
		container->add_var_from_example( cv );

	delete [ ] container->label;

	container->label = new char[ strlen( r->label ) + 1 ];
	strcpy( container->label, r->label );

	for ( cb = r->b; cb != NULL; cb = cb->next )
	{
		if ( cb->head == NULL )
			continue;

		cur1 = cb->head;
		container->add_obj( cur1->label, 1, 0 );

		for ( cb1 = container->b; strcmp( cb1->blabel, cb->blabel ); cb1 = cb1->next );

		cur = cb1->head;
		set_blueprint( cur, cur1 );
	}
}


/*****************************************************************************
EMPTY_BLUEPRINT
remove the current blueprint
******************************************************************************/
void empty_blueprint( void )
{
	if ( blueprint == NULL )
		return;

	blueprint->empty( );
	blueprint->delete_obj( );
	blueprint = NULL;
}


/*****************************************************************************
RESET_BLUEPRINT
reset the current blueprint
******************************************************************************/
void reset_blueprint( object *r )
{
	empty_blueprint( );
	blueprint = new object;
	blueprint->init( NULL, "Root" );
	set_blueprint( blueprint, r );
}


/***************************************************
SEARCH_DESCRIPTION
***************************************************/
description *search_description( const char *lab, bool add_missing )
{
	description *cd;
	variable *cv;

	for ( cd = descr; cd != NULL; cd = cd->next )
		if ( ! strcmp( cd->label, lab ) )
			return cd;

	if ( ! add_missing )
		return NULL;

	if ( root->search( lab ) != NULL )
		return add_description( lab );

	cv = root->search_var( NULL, lab );
	if ( cv != NULL )
		return add_description( lab, cv->param );

	return NULL;
}


/***************************************************
ADD_DESCRIPTION
***************************************************/
const char *kwords[ ] = { BEG_INIT, END_DESCR };

description *add_description( const char *lab, int type, const char *text, const char *init, char initial, char observe )
{
	char *str, ltype[ MAX_ELEM_LENGTH ];
	int i, j;
	description *cd;

	if ( search_description( lab, false ) != NULL )	// already exists?
		return change_description( lab, NULL, type, text, init, initial, observe );

	if ( descr == NULL )
		cd = descr = new description;
	else
	{
		for ( cd = descr; cd->next != NULL; cd = cd->next );
		cd->next = new description;
		cd = cd->next;
	}

	cd->next = NULL;
	cd->label = new char [ strlen( lab ) + 1 ];
	strcln( cd->label, lab, strlen( lab ) + 1 );

	switch ( type )
	{
		case 0:
		default:
			strcpy( ltype, "Variable" );
			break;
		case 1:
			strcpy( ltype, "Parameter" );
			break;
		case 2:
			strcpy( ltype, "Function" );
			break;
		case 4:
			strcpy( ltype, "Object" );
	}

	cd->type = new char [ strlen( ltype ) + 1 ];
	strcpy( cd->type, ltype );

	if ( ! strwsp( text ) && strstr( text, LEGACY_NO_DESCR ) == NULL && ( strlen( NO_DESCR ) == 0 || strstr( text, NO_DESCR ) == NULL ) )
	{
		for ( i = 0; i < 2; ++i )
		{
			str = ( char * ) strstr( text, kwords[ i ] );
			if ( str != NULL )
				for( j = 0; j < ( int ) strlen( kwords[ i ] ); ++j, ++str )
					*str = tolower( *str );
		}

		cd->text = new char [ strlen( text ) + 1 ];
		strcln( cd->text, text, strlen( text ) + 1 );
	}
	else
	{
		cd->text = new char[ strlen( NO_DESCR ) + 1 ];
		strcln( cd->text, NO_DESCR, strlen( NO_DESCR ) + 1 );
	}

	if ( ! strwsp( init ) )
	{
		str = ( char * ) strstr( init, kwords[ 1 ] );
		if ( str != NULL )
			for( j = 0; j < ( int ) strlen( kwords[ 1 ] ); ++j, ++str )
				*str = tolower( *str );

		cd->init = new char [ strlen( init ) + 1 ];
		strcln( cd->init, init, strlen( init ) + 1 );
	}
	else
		cd->init = NULL;

	cd->initial = initial;
	cd->observe = observe;

	return cd;
}


/***************************************************
CHANGE_DESCRIPTION
***************************************************/
description *change_description( const char *lab_old, const char *lab, int type, const char *text, const char *init, char initial, char observe )
{
	char *str, ltype[ MAX_ELEM_LENGTH ];
	int i, j;
	description *cd, *cd1;

	for ( cd = descr; cd != NULL; cd = cd->next )
	{
		if ( ! strcmp( cd->label, lab_old ) )
		{

			if ( lab == NULL && type < 0 && text == NULL && init == NULL && initial == '\0' && observe == '\0' )
			{
				delete [ ] cd->label;
				delete [ ] cd->type;
				delete [ ] cd->text;
				delete [ ] cd->init;

				if ( cd == descr )
					descr = cd->next;
				else
				{
					for ( cd1 = descr; cd1->next != cd; cd1 = cd1->next );
					cd1->next = cd->next;
				}

				delete cd;

				return NULL;
			}

			if ( lab != NULL )
			{
				delete [ ] cd->label;
				cd->label = new char [ strlen( lab ) + 1 ];
				strcln( cd->label, lab, strlen( lab ) + 1 );
			}

			if ( type >= 0 )
			{
				delete [ ] cd->type;

				switch ( type )
				{
					case 0:
						strcpy( ltype, "Variable" );
						break;
					case 1:
						strcpy( ltype, "Parameter" );
						break;
					case 2:
						strcpy( ltype, "Function" );
						break;
					case 4:
					default:
						strcpy( ltype, "Object" );
				}

				cd->type = new char [ strlen( ltype ) + 1 ];
				strcpy( cd->type, ltype );
			}

			if ( text != NULL )
			{
				delete [ ] cd->text;

				if ( ! strwsp( text ) && strstr( text, LEGACY_NO_DESCR ) == NULL && ( strlen( NO_DESCR ) == 0 || strstr( text, NO_DESCR ) == NULL ) )
				{
					for ( i = 0; i < 2; ++i )
					{
						str = ( char * ) strstr( text, kwords[ i ] );
						if ( str != NULL )
							for( j = 0; j < ( int ) strlen( kwords[ i ] ); ++j, ++str )
								*str = tolower( *str );
					}

					cd->text = new char [ strlen( text ) + 1 ];
					strcln( cd->text, text, strlen( text ) + 1 );
				}
				else
				{
					cd->text = new char[ strlen( NO_DESCR ) + 1 ];
					strcln( cd->text, NO_DESCR, strlen( NO_DESCR ) + 1 );
				}
			}

			if ( init != NULL )
			{
				delete [ ] cd->init;

				if ( ! strwsp( init ) )
				{
					str = ( char * ) strstr( init, kwords[ 1 ] );
					if ( str != NULL )
						for( j = 0; j < ( int ) strlen( kwords[ 1 ] ); ++j, ++str )
							*str = tolower( *str );

					cd->init = new char [ strlen( init ) + 1 ];
					strcln( cd->init, init, strlen( init ) + 1 );
				}
				else
					cd->init = NULL;
			}

			if ( initial != '\0' )
				cd->initial = initial;

			if ( observe != '\0' )
				cd->observe = observe;

			return cd;
		}
	}

	return NULL;
}


/*****************************************************************************
EMPTY_DESCRIPTION
******************************************************************************/
void empty_description( void )
{
	description *cd, *cd1;

	for ( cd = descr; cd != NULL; cd = cd1 )
	{
		cd1 = cd->next;
		delete [ ] cd->label;
		delete [ ] cd->type;
		delete [ ] cd->text;
		delete [ ] cd->init;
		delete cd;
	}

	descr = NULL;
}


/***************************************************
RESET_DESCRIPTION
regenerate recur. the descriptions of the model as it is
***************************************************/
void reset_description( object *r )
{
	bridge *cb;
	variable *cv;

	if ( r == NULL )
		return;

	search_description( r->label );

	for ( cv = r->v; cv != NULL; cv = cv->next )
		search_description( cv->label );

	for ( cb = r->b; cb != NULL; cb = cb->next )
		if ( cb->head != NULL )
			reset_description( cb->head );
}


/***************************************************
HAS_DESCR_TEXT
***************************************************/
bool has_descr_text( description *d )
{
	if ( d != NULL && d->text != NULL && strlen( d->text ) > 0 && strstr( d->text, LEGACY_NO_DESCR ) == NULL && ( strlen( NO_DESCR ) == 0 || strstr( d->text, NO_DESCR ) == NULL ) )
		return true;

	return false;
}


#ifndef _NW_

/***************************************************
FMT_TTIP_DESCR
***************************************************/
char *fmt_ttip_descr( char *out, description *d, int outSz, bool init )
{
	char out1[ outSz ];

	if ( out == NULL || outSz <= 0 )
		return NULL;

	if ( has_descr_text ( d ) )
		strcln( out, d->text, outSz );
	else
		out[ 0 ] = '\0';

	if ( init && d != NULL && d->init != NULL && strlen( d->init ) > 0 )
	{
		if ( strlen( out ) > 0 )
			strcatn( out, "\n\u2500\u2500\u2500\n", outSz );

		strcln( out1, d->init, outSz );
		strcatn( out, out1, outSz );
	}

	if ( strlen( out ) > 0 )
	{
		strwrap( out1, out, outSz - 1, 60 );
		strtcl( out, out1, outSz - 1 );
	}

	return out;
}


/***************************************************
SET_TTIP_DESCR
***************************************************/
void set_ttip_descr( const char *w, const char *lab, int it, bool init )
{
	char desc[ MAX_LINE_SIZE + 1 ];
	description *cd;

	// add tooltip only if element has description
	cd = search_description( lab, false );
	if ( cd != NULL && strlen( fmt_ttip_descr( desc, cd, MAX_LINE_SIZE + 1, init ) ) > 0 )
	{
		if ( it >= 0 )			// listbox/canvas?
			cmd( "tooltip::tooltip %s -item %d \"%s\"", w, it, desc );
		else
			cmd( "tooltip::tooltip %s \"%s\"", w, desc );
	}
}


/***************************************************
TCL_SET_TTIP_DESCR
***************************************************/
int Tcl_set_ttip_descr( ClientData cdata, Tcl_Interp *inter, int argc, const char *argv[ ] )
{
	int it, init;

	if ( argc < 3 || argc > 5 )		// require 4 parameters: widget name, variable name text, item number (opt) and init text flag (opt)
		return TCL_ERROR;

	if ( argv[ 1 ] == NULL || argv[ 2 ] == NULL ||
		 ! strcmp( argv[ 1 ], "" ) || ! strcmp( argv[ 2 ], "" ) )
		return TCL_ERROR;

	if ( argc < 4 || argv[ 3 ] == NULL || sscanf( argv[ 3 ], "%d", & it ) == 0 )
		it = -1;

	if ( argc < 5 || argv[ 4 ] == NULL || sscanf( argv[ 4 ], "%d", & init ) == 0 || init < 0 || init > 1 )
		init = 1;

	set_ttip_descr( argv[ 1 ], argv[ 2 ], it, init ? true : false );

	return TCL_OK;
}


/****************************************************
SEARCH_ALL_SOURCES
****************************************************/
FILE *search_all_sources( char *str )
{
	char got[ MAX_LINE_SIZE ];
	const char *fname;
	int i, j, nfiles;
	FILE *f;

	// search in all source files
	cmd( "set source_files [ get_source_files \"%s\" ]", exec_path );
	cmd( "if { [ lsearch -exact $source_files \"%s\" ] == -1 } { lappend source_files \"%s\" }", equation_name, equation_name );
	cmd( "set res [ llength $source_files ]" );
	nfiles = get_int( "res" );

	for ( i = 0; i < nfiles; ++i )
	{
		cmd( "set brr [ lindex $source_files %d ]", i );
		cmd( "if { ! [ file exists $brr ] && [ file exists \"%s/$brr\" ] } { set brr \"%s/$brr\" }", exec_path, exec_path );
		fname = get_str( "brr" );
		if ( ( f = fopen( fname, "r" ) ) == NULL )
			continue;

		fgets( got, MAX_LINE_SIZE, f );
		clean_spaces( got );
		for ( j = 0; strncmp( got, str, strlen( str ) ) && j < MAX_FILE_TRY; ++j )
		{
			if ( fgets( got, MAX_LINE_SIZE, f ) == NULL )
				break;
			clean_spaces( got );
		}

		if ( ! strncmp( got, str, strlen( str ) ) )
			return f;

		fclose( f );
	}

	return NULL;
}


/***************************************************
RETURN_WHERE_USED
***************************************************/
void return_where_used( char *lab, char *s, int sz )
{
	const char *app;

	scan_used_lab( lab, "" );	// make scan without window
	app = get_str( "list_used" );

	if ( app != NULL )
		strcpyn( s, app, sz );
	else
		strcpy( s, "" );
}


/***************************************************
GET_VAR_DESCR
***************************************************/
void get_var_descr( const char *lab, char *desc, int descr_len )
{
	char str[ 2 * MAX_ELEM_LENGTH ], str1[ MAX_LINE_SIZE ], str2[ descr_len ];
	int i, j = 0, done = -1;
	FILE *f;

	snprintf( str, 2 * MAX_ELEM_LENGTH, "EQUATION(\"%s\")", lab );
	f = search_all_sources( str );

	if ( f == NULL )
	{
		snprintf( str, 2 * MAX_ELEM_LENGTH, "EQUATION_DUMMY(\"%s\",", lab );
		f = search_all_sources( str );
	}

	if ( f == NULL )
	{
		snprintf( str, 2 * MAX_ELEM_LENGTH, "FUNCTION(\"%s\")", lab );
		f = search_all_sources( str );
	}

	if ( f == NULL )
	{
		snprintf( str, 2 * MAX_ELEM_LENGTH, "if (!strcmp(label,\"%s\"))", lab );
		f = search_all_sources( str );
	}

	if ( f != NULL )
	{
		while ( done != 1 )
		{
			fgets( str1, MAX_LINE_SIZE, f );

			for ( i = 0; str1[ i ] != '\0' && done != 1; ++i )
			{
				if ( done == -1 ) 		// no comment found yet
				{
					if ( isalpha( str1[ i ]) != 0 ) 	// no comment exists
						done = 1;

					if ( str1[ i ] == '/' && str1[ i + 1 ] == '*' )
					{
						done = 0; 		// beginning of a multiline comment
						i += 2;

						// discard initial empty line
						while ( str1[ i ] == '\r' && str1[ i + 1 ] == '\n' )
							i += 2;
						while ( str1[ i ] == '\n' )
							++i;
						if ( str1[ i ] == '\0' )
							break;
					}

					if ( str1[ i ] == '/' && str1[ i + 1 ] == '/' )
					{
						done = 2; 		// beginning of a single line comment
						i += 2;
					}
				}

				if ( done == 0 ) 		// we are in a comment
					if ( str1[ i ] == '*' && str1[ i + 1 ] == '/' )
						done = 1;

				if ( done == 0 || done == 2 )
					if ( str1[ i ] != '\r' )
						str2[ j++ ] = str1[ i ];

				if ( done == 2 && str1[ i ] == '\n' )
					done = -1;

				if ( j >= descr_len - 2 )
					done = 1;
			}
		}

		fclose( f );
	}

	str2[ j ] = '\0';
	strcln( desc, str2, descr_len );
}


/***************************************************
AUTO_DOCUMENT
***************************************************/
void auto_document( const char *lab, const char *which, bool append )
{
	bool var;
	char str1[ MAX_LINE_SIZE ], app[ 10 * MAX_LINE_SIZE ], text[ 2 * MAX_BUFF_SIZE ];
	description *cd;

	for ( cd = descr; cd != NULL; cd = cd->next )
	{
		app[ 0 ] = '\0';
		if ( ( lab == NULL && ( ! strcmp( which, "ALL" ) || ! strcmp( cd->type, "Variable" ) || ! strcmp( cd->type, "Function" ) ) ) || ( lab != NULL && ! strcmp( lab, cd->label ) ) )
		{	// for each description
			if ( ( ! strcmp( cd->type, "Variable") ) == 1 || ( ! strcmp( cd->type, "Function" ) ) == 1 )
			{ 	// if it is a Variable
				var = true;
				get_var_descr( cd->label, app, 10 * MAX_LINE_SIZE );
			}
			else
				var = false;

			return_where_used( cd->label, str1, MAX_LINE_SIZE );
			if ( ( append || ! var ) && has_descr_text ( cd ) )
				if ( strwsp( cd->text ) )
					snprintf( text, 2 * MAX_BUFF_SIZE, "%s\n'%s' appears in the equation for: %s", app, cd->label, str1 );
				else
					snprintf( text, 2 * MAX_BUFF_SIZE, "%s\n%s\n'%s' appears in the equation for: %s", cd->text, app, cd->label, str1 );
			else
				snprintf( text, 2 * MAX_BUFF_SIZE, "%s\n'%s' appears in the equation for: %s", app, cd->label, str1 );

			delete [ ] cd->text;
			cd->text = new char[ strlen( text ) + 1 ];
			strcpy( cd->text, text );
		} 					// end of the label to document
	}						// end of the for (desc)
}

#endif


/****************************************************
COUNT_SAVE
****************************************************/
void count_save( object *n, int *count )
{
	bridge *cb;
	object *co;
	variable *cv;

	for ( cv = n->v; cv != NULL; cv = cv->next )
		if ( cv->save == 1 || cv->savei == 1 )
			( *count )++;

	for ( cb = n->b; cb != NULL; cb = cb->next )
	{
		if ( cb->head == NULL )
			co = blueprint->search( cb->blabel );
		else
			co = cb->head;
		count_save( co, count );
	}
}


/***************************************************
INIT_LATTICE
Create a new run time lattice having:
- pix= maximum pixel (600 should fit in typical screens, 0=default size)
- nrow= number of rows
- ncol= number of columns
- lrow= label of variable or parameter indicating the row value
- lcol= label of variable or parameter indicating the column value
- lvar= label of variable or parameter from which to read the color of the cell
- p= pointer of the object containing the initial color of the cell (if flag==-1)
- init_color= indicate the type of initialization.
  If init_color < 0, the (positive) RGB equivalent to init_color is used.
  Otherwise, the lattice is homogeneously initialized to the palette color specified by init_color.
***************************************************/
double init_lattice( double pixW, double pixH, double nrow, double ncol, const char lrow[ ], const char lcol[ ], const char lvar[ ], object *p, int init_color )
{
	char init_color_string[ 32 ];	// the final string to be used to define tk color to use
	int i, j, hsize, vsize, hsizeMax, vsizeMax;

	// ignore invalid values
	if ( ( int ) nrow < 1 || ( int ) ncol < 1 || ( int ) nrow > INT_MAX || ( int ) ncol > INT_MAX )
	{
		plog( "\nError: invalid lattice initialization values, ignoring.\n");
		return -1;
	}

	init_color = min( init_color, 1099 );	// limit to valid palette

	// reset the LSD lattice, if any
	close_lattice( );
	rows = ( int ) max( 0, floor( nrow ) );
	columns = ( int ) max( 0, floor( ncol ) );
	error_count = 0;

	// create the color data matrix
	lattice = new int *[ rows ];
	for ( i = 0; i < rows; ++i )
		lattice[ i ] = new int [ columns ];

	for ( i = 0; i < rows; ++i )
		for ( j = 0; j < columns; ++j )
			lattice[ i ][ j ] = init_color;

#ifndef _NW_

	hsize = get_int( "hsizeLat" );			// 400
	vsize = get_int( "vsizeLat" );			// 400
	hsizeMax = get_int( "hsizeLatMax" );	// 1024
	vsizeMax = get_int( "vsizeLatMax" );	// 1024

	pixW = floor( pixW ) > 0 ? floor( pixW ) : hsize;
	pixH = floor( pixH ) > 0 ? floor( pixH ) : vsize;
	pixW = min( pixW, hsizeMax );
	pixH = min( pixH, vsizeMax );

	dimH = pixH / rows;
	dimW = pixW / columns;

	if ( init_color < 0 && ( - init_color ) <= 0xffffff )		// RGB mode selected?
		snprintf( init_color_string, 32, "#%06x", - init_color );	// yes: just use the positive RGB value
	else
	{
		snprintf( init_color_string, 32, "$c%d", init_color );		// no: use the positive RGB value
		// create (background color) pallete entry if invalid palette in init_color
		cmd( "if { ! [ info exist c%d ] } { set c%d $colorsTheme(bg) }", init_color, init_color  );
	}

	// create the window with the lattice, roughly 600 pixels as maximum dimension
	cmd( "newtop .lat \"%s%s - LSD Lattice (%.0lf x %.0lf)\" { destroytop .lat } \"\"", unsaved_change() ? "*" : " ", strlen( simul_name ) > 0 ? simul_name : NO_CONF_NAME, nrow, ncol );

	cmd( "ttk::canvas .lat.c -height %d -width %d -entry 0 -dark $darkTheme", ( unsigned int ) pixH, ( unsigned int ) pixW );

	if ( init_color != 1001 )
		cmd( ".lat.c configure -background %s", init_color_string );

	cmd( "pack .lat.c" );

	cmd( "save .lat b { \
			if { ! [ info exists pltSavFmt ] } { \
				set pltSavFmt svg \
			}; \
			if { [ string equal $pltSavFmt eps ] } { \
				set c \"Encapsulated Postscript\" \
			} else { \
				set c \"Scalable Vector Graphics\" \
			}; \
			set a [ tk_getSaveFile -parent .lat -title \"Save Lattice to File\" -defaultextension .$pltSavFmt -initialfile %s.$pltSavFmt -initialdir \"%s\" -filetypes { { {Scalable Vector Graphics} {.svg} } { {Encapsulated Postscript} {.eps} } { {All files} {*} } } -typevariable c ]; \
			if { [ string length $a ] != 0 } { \
				set a [ file nativename $a ]; \
				set b [ string trimleft [ file extension $a ] . ]; \
				if { $b in [ list svg eps ] } { \
					set pltSavFmt $b \
				}; \
				if [ string equal $pltSavFmt eps ] { \
					.lat.c postscript -colormode color -file \"$a\" \
				} else { \
					canvas2svg .lat.c \"$a\" \
				}; \
				plog \"\nPlot saved: $a\n\" \
			} \
		}", strlen( simul_name ) > 0 ? simul_name : "plot", path );

	cmd( "set rows %d", rows );
	cmd( "set columns %d", columns );
	cmd( "set dimH %.6g", dimH );
	cmd( "set dimW %.6g", dimW );

	cmd( "for { set i 1 } { $i <= $rows } { incr i } { \
			for { set j 1 } { $j <= $columns } { incr j } { \
				set x1 [ expr { ( $j - 1 ) * $dimW } ]; \
				set y1 [ expr { ( $i - 1 ) * $dimH } ]; \
				set x2 [ expr { $j * $dimW } ]; \
				set y2 [ expr { $i * $dimH } ]; \
				.lat.c create rectangle $x1 $y1 $x2 $y2 -outline \"\" -tags c${i}_${j} \
			} \
		}" );

	cmd( "showtop .lat centerS no no no" );

	cmd( "tooltip::tooltip .lat.b.ok \"Save plot to file\"" );

	cmd( "bind .lat <Button-2> { .lat.b.ok invoke }" );
	cmd( "bind .lat <Button-3> { event generate .lat <Button-2> -x %%x -y %%y }" );
	cmd( "bind .lat <F1> { LsdHelp lattice.html }" );
	set_shortcuts_run( ".lat" );

#endif

	return 0;
}

// call for macro
double init_lattice( int init_color, double nrow, double ncol, double pixW, double pixH )
{
	return init_lattice( pixW, pixH, nrow, ncol, "y", "x", "", NULL, init_color );
}


/***************************************************
EMPTY_LATTICE
***************************************************/
void empty_lattice( void )
{
	if ( lattice != NULL && rows > 0 )
	{
		for ( int i = 0; i < rows; ++i )
			delete [ ] lattice[ i ];

		delete [ ] lattice;
	}

	lattice = NULL;
	rows = columns = 0;
}


/***************************************************
CLOSE_LATTICE
***************************************************/
void close_lattice( void )
{
	empty_lattice( );

#ifndef _NW_
	cmd( "destroytop .lat" );
#endif
}


/***************************************************
UPDATE_LATTICE
update the cell line.col to the color val (1 to 21 as set in default.tcl palette)
negative values of val prompt for the use of the (positive) RGB equivalent
***************************************************/
double update_lattice( double line, double col, double val )
{
	char val_string[ 32 ];		// the final string to be used to define tk color to use
	int line_int, col_int, val_int;

	line_int = line - 1;
	col_int = col - 1;
	val_int = max( 0, floor( val ) );

	// ignore invalid values
	if ( line_int < 0 || col_int < 0 || line_int >= rows ||
		 col_int >= columns || ( int ) fabs( val ) > INT_MAX )
	{
		if ( error_count == ERR_LIM )
			plog( "\nWarning: too many lattice parameter errors, messages suppressed.\n");
		else
			if ( error_count < ERR_LIM )
				plog( "\nError: invalid lattice update values, ignoring." );

		++error_count;

		return -1;
	}

	// save lattice color data

	if ( lattice != NULL && rows > 0 && columns > 0 )
	{
		if ( val >= 0 && lattice[ line_int ][ col_int ] == val_int )
			return 0;
		else
			lattice[ line_int ][ col_int ] = val_int;
	}
#ifndef _NW_

	// avoid operation if canvas was closed
	if ( ! exists_window( ".lat.c" ) )
		return -1;

	if ( val < 0 && ( - ( int )  val ) <= 0xffffff )	// RGB mode selected?
		snprintf( val_string, 32, "#%06x", - ( int ) val );	// yes: just use the positive RGB value
	else
	{
		snprintf( val_string, 32, "$c%d", val_int );			// no: use the predefined Tk color
		// create (background color) pallete entry if invalid palette in val
		cmd( "if { ! [ info exist c%d ] } { set c%d $colorsTheme(bg) }", val_int, val_int  );
	}

	cmd( ".lat.c itemconfigure c%d_%d -fill %s", line_int + 1, col_int + 1, val_string );

#endif

	return 0;
}


/***************************************************
READ_LATTICE
read the cell line.col color val (1 to 21 as set in default.tcl palette)
negative values of val mean the use of the (positive) RGB equivalent
***************************************************/
double read_lattice( double line, double col )
{
	// ignore invalid values
	if ( ( int ) line <= 0 || ( int ) col <= 0 || ( int ) line > rows || ( int ) col > columns )
	{
		if ( error_count == ERR_LIM )
			plog( "\nWarning: too many lattice parameter errors, messages suppressed.\n");
		else
			if ( error_count < ERR_LIM )
				plog( "\nError: invalid lattice update values, ignoring." );

		++error_count;

		return -1;
	}

	if ( lattice != NULL && rows > 0 && columns > 0 )
		return lattice[ ( int ) line - 1 ][ ( int ) col - 1 ];
	else
		return 0;
}


/***************************************************
SAVE_LATTICE
Save the existing lattice (if any) to the specified file name.
***************************************************/
double save_lattice( const char *fname )
{
#ifndef _NW_
	// avoid operation if no canvas or no file name
	if ( ! exists_window( ".lat.c" ) || fname == NULL || strlen( fname ) == 0 )
		return -1;

	cmd( "set latname \"%s\"", fname );
	cmd( "append latname .eps" );
	cmd( ".lat.c postscript -colormode color -file $latname" );
#endif
	return 0;
}


/****************************************************
_ABS
****************************************************/
double _abs( double a )
{
	if ( a > 0 )
		return a;
	else
		return ( -1 * a );
};


/****************************************************
MAX
****************************************************/
double max( double a, double b )
{
	if ( a > b )
		return a;
	return b;
}


/****************************************************
MIN
****************************************************/
double min ( double a, double b )
{
	if ( a < b )
		return a;
	return b;
}


/****************************************************
MEDIAN
****************************************************/
double median( vector < double > & v )
{
	int mid;
	double midVal;

	if ( v.empty( ) )
		return NAN;

	mid = v.size( ) / 2;

	auto midPos = v.begin( ) + mid;
	nth_element( v.begin( ), midPos, v.end( ) );
	midVal = v[ mid ];

	if ( v.size( ) % 2 != 0 )
		return midVal;
	else
		return ( * max_element( v.begin( ), midPos ) + midVal ) / 2;
}


/***************************************************
FACT
Factorial function
***************************************************/
double fact( double x )
{
	x = floor( x );
	if ( x < 0.0 )
	{
		plog( "\nWarning: bad x in function: fact" );
		return 0.0;
	}

	double fact = 1.0;
	long i = 1;
	while (i <= x)
		fact *= i++;

	return fact;
}


/****************************************************
ROUND
****************************************************/
double round( double x )
{
	if ( ( x - floor( x ) ) > ( ceil( x ) - x ) )
		return ceil( x );

	return floor( x );
}


/****************************************************
ROUND_DIGITS
****************************************************/
double round_digits( double value, int digits )
{
	if ( value == 0.0 )
		return 0.0;

	double factor = pow( 10.0, digits - ceil( log10( fabs( value ) ) ) );

	return round( value * factor ) / factor;
}


/****************************************************
LOWER_BOUND
****************************************************/
double lower_bound( double a, double b, double marg, double marg_eq, int dig )
{
	double rmin = round_digits( a, dig );
	double rmax = round_digits( b, dig );

	if ( rmin > rmax )
	{
		double temp = rmin;
		rmin = rmax;
		rmax = temp;
	}

	if ( rmin == rmax )
	{
		if ( rmin == 0.0 )
			return round_digits( - marg_eq, dig );
		else
			if ( rmin > 0 )
				return round_digits( rmin * ( 1 - marg_eq ), dig );
			else
				return round_digits( rmin * ( 1 + marg_eq ), dig );
	}

	if ( rmin == 0.0 )
		return round_digits( - marg, dig );
	else
		if ( rmin > 0 )
			return round_digits( rmin * ( 1 - marg ), dig );
		else
			return round_digits( rmin * ( 1 + marg ), dig );
}


/****************************************************
UPPER_BOUND
****************************************************/
double upper_bound( double a, double b, double marg, double marg_eq, int dig )
{
	double rmin = round_digits( a, dig );
	double rmax = round_digits( b, dig );

	if ( rmin > rmax )
	{
		double temp = rmin;
		rmin = rmax;
		rmax = temp;
	}

	if ( rmin == rmax )
	{
		if ( rmax == 0.0 )
			return round_digits( marg_eq, dig );
		else
			if ( rmax > 0 )
				return round_digits( rmax * ( 1 + marg_eq ), dig );
			else
				return round_digits( rmax * ( 1 - marg_eq ), dig );
	}

	if ( rmax == 0.0 )
		return round_digits( marg, dig );
	else
		if ( rmax > 0 )
			return round_digits( rmax * ( 1 + marg ), dig );
		else
			return round_digits( rmax * ( 1 - marg ), dig );
}


/***************************************************
UNIFCDF
Uniform cumulative distribution function
***************************************************/

double unifcdf( double a, double b, double x )
{
	if ( a >= b )
	{
		plog( "\nWarning: bad a or b in function: uniformcdf" );
		return 0.0;
	}

	if ( x <= a )
		return 0.0;
	if ( x >= b )
		return 1.0;

	return ( x - a ) / ( b - a );
}


/***************************************************
POISSONCDF
Poisson cumulative distribution function
***************************************************/
double poissoncdf( double lambda, double k )
{
	k = floor( k );
	if ( lambda <= 0.0 || k < 0.0 )
	{
		plog( "\nWarning: bad lambda or k in function: poissoncdf" );
		return 0.0;
	}

	double sum = 0.0;
	long i;
	for ( i = 0; i <= k; i++ )
		sum += pow( lambda, i ) / fact( i );

	return exp( -lambda ) * sum;
}


/***************************************************
PARETOCDF
Pareto cumulative distribution function
***************************************************/
double paretocdf( double mu, double alpha, double x )
{
	if ( mu <= 0 || alpha <= 0 )
	{
		plog( "\nWarning: bad mu, alpha in function: paretocdf" );
		return 0.0;
	}

	if ( x < mu )
		return 0.0;
	else
		return 1.0 - pow( mu / x, alpha );
}


/***************************************************
BPARETOCDF
Bounded Pareto cumulative distribution function
***************************************************/
double bparetocdf( double alpha, double low, double high, double x )
{
	if ( alpha <= 0 || low <= 0 || low >= high )
	{
		plog( "\nWarning: bad alpha, low or high in function: bparetocdf" );
		return 0.0;
	}

	if ( x < low )
		return 0.0;
	else
		return ( 1 - pow( low, alpha ) * pow( x, - alpha ) ) /
			   ( 1 - pow( low / high, alpha ) ) ;
}


/***************************************************
NORMCDF
Normal cumulative distribution function
***************************************************/
double normcdf( double mu, double sigma, double x )
{
	if ( sigma <= 0.0 )
	{
		plog( "\nWarning: bad sigma in function: normalcdf" );
		return 0.0;
	}

	return 0.5 * ( 1 + erf( ( x - mu ) / ( sigma * sqrt( 2.0 ) ) ) );
}


/***************************************************
LNORMCDF
Lognormal cumulative distribution function
***************************************************/
double lnormcdf( double mu, double sigma, double x )
{
	if ( sigma <= 0.0 || x <= 0.0 )
	{
		plog( "\nWarning: bad sigma or x in function: lnormalcdf" );
		return 0.0;
	}

	return 0.5 + 0.5 * erf( ( log( x ) - mu ) / ( sigma * sqrt( 2.0 ) ) );
}


/***************************************************
ALAPLCDF
Asymmetric laplace cumulative distribution function
***************************************************/
double alaplcdf( double mu, double alpha1, double alpha2, double x )
{
	if ( alpha1 <= 0.0 || alpha2 <= 0.0 )
	{
		plog( "\nWarning: bad alpha in function: alaplcdf" );
		return 0.0;
	}

	if ( x < mu )									// cdf up to upper bound
		return 0.5 * exp( ( x - mu ) / alpha1 );
	else
		return 1 - 0.5 * exp( -( x - mu ) / alpha2 );
}


/***************************************************
BETACF
Beta distribution: continued fraction evaluation function
Press et al. (1992) Numerical Recipes in C, 2nd Ed.
***************************************************/
#define MAXIT 100
#define BEPS 3.0e-7
#define FPMIN 1.0e-30

double betacf( double a, double b, double x )
{
	void nrerror(char error_text[ ]);
	int m, m2;
	double aa, c, d, del, h, qab, qam, qap;

	qab = a + b;
	qap = a + 1.0;
	qam = a - 1.0;
	c = 1.0;
	d = 1.0 - qab * x / qap;

	if ( fabs( d ) < FPMIN )
		d = FPMIN;
	d = 1.0 / d;
	h = d;

	for ( m = 1; m <= MAXIT; m++ )
	{
		m2 = 2 * m;
		aa = m * ( b - m ) * x / ( ( qam + m2 ) * ( a + m2 ) );
		d = 1.0 + aa * d;
		if ( fabs( d ) < FPMIN)
			d = FPMIN;

		c = 1.0 + aa / c;
		if ( fabs( c ) < FPMIN )
			c=FPMIN;

		d = 1.0 / d;
		h *= d * c;
		aa = -( a + m ) * ( qab + m ) * x / ( ( a + m2 ) * ( qap + m2 ) );
		d = 1.0 + aa * d;
		if ( fabs( d ) < FPMIN )
			d = FPMIN;

		c = 1.0 + aa / c;
		if ( fabs( c ) < FPMIN )
			c = FPMIN;

		d = 1.0 / d;
		del = d * c;
		h *= del;
		if ( fabs( del - 1.0) < BEPS )
			break;
	}

	if ( m > MAXIT )
		plog( "\nWarning: a or b too big (or MAXIT too small) in function: betacf");

	return h;
}


/***************************************************
BETACDF
Beta cumulative distribution function: incomplete beta function
Press et al. (1992) Numerical Recipes in C, 2nd Ed.
***************************************************/
double betacdf( double alpha, double beta, double x )
{
	double bt;

	if ( alpha <= 0.0 || beta <= 0.0 || x < 0.0 || x > 1.0 )
	{
		plog( "\nWarning: bad alpha, beta or x in function: betacdf" );
		return 0.0;
	}

	if ( x == 0.0 || x == 1.0 )
		bt = 0.0;
	else
		bt = exp( lgamma( alpha + beta ) - lgamma( alpha ) - lgamma( beta )
				 + alpha * log( x ) + beta * log( 1.0 - x ) );

	if ( x < ( alpha + 1.0 ) / ( alpha + beta + 2.0 ) )
		return bt * betacf( alpha, beta, x ) / alpha;
	else
		return 1.0 - bt * betacf( beta, alpha, 1.0 - x ) / beta;
}


#ifndef _NW_

/****************************************************
T_STAR
Student t distribution  statistic for given
degrees of freedom and confidence level (in %)
****************************************************/
double t_star( int df, double cl )
{
	int i;

	for ( i = 0; i < T_CLEVS - 1; ++i )
		if ( cl <= 100 * t_dist_cl[ i ] )
			break;

	if ( df <= 30 )
		return t_dist_st[ i ][ df - 1 ];

	if ( df <= 40 )
		return t_dist_st[ i ][ 30 ];

	if ( df <= 60 )
		return t_dist_st[ i ][ 31 ];

	if ( df <= 80 )
		return t_dist_st[ i ][ 32 ];

	if ( df <= 100 )
		return t_dist_st[ i ][ 33 ];

	if ( df <= 1000 )
		return t_dist_st[ i ][ 34 ];

	return t_dist_st[ i ][ 35 ];
}


/****************************************************
Z_STAR
Standard normal distribution statistic for given
confidence level (in %)
****************************************************/
double z_star( double cl )
{
	int i;

	for ( i = 0; i < Z_CLEVS - 1; ++i )
		if ( cl <= 100 * z_dist_cl[ i ] )
			break;

	return z_dist_st[ i ];
}

#endif


/****************************************************
IS_FINITE
function redefinition to handle GCC standard library bugs
****************************************************/
bool is_finite( double x )
{
#if __GNUC__ > 3
	return __builtin_isfinite( x );
#else
	return isfinite( x );
#endif
}


/****************************************************
IS_INF
function redefinition to handle GCC standard library bugs
****************************************************/
bool is_inf( double x )
{
#if __GNUC__ > 3
	return __builtin_isinf( x );
#else
	return isinf( x );
#endif
}


/****************************************************
IS_NAN
function redefinition to handle GCC standard library bugs
****************************************************/
bool is_nan( double x )
{
#if __GNUC__ > 3
	return __builtin_isnan( x );
#else
	return isnan( x );
#endif
}


/****************************************************
INIT_RANDOM
Set seed to all random generators
Pseudo-random number generator to extract draws
ran_gen_id = 0 : system (not pseudo) random device in (0,1)
ran_gen_id = 1 : Linear congruential in (0,1)
ran_gen_id = 2 : Mersenne-Twister in (0,1)
ran_gen_id = 3 : Linear congruential in [0,1)
ran_gen_id = 4 : Mersenne-Twister in [0,1)
ran_gen_id = 5 : Mersenne-Twister with 64 bits resolution in [0,1)
ran_gen_id = 6 : Lagged fibonacci with 24 bits resolution in [0,1)
ran_gen_id = 7 : Lagged fibonacci with 48 bits resolution in [0,1)
****************************************************/
int ran_gen_id = 2;					// ID of initial generator (DO NOT CHANGE)
long idum = 0;						// Park-Miller default seed (legacy code only)

#ifndef _NP_
mutex parallel_rd;					// mutex locks for random generator operations
mutex parallel_lc1;
mutex parallel_lc2;
mutex parallel_mt32;
mutex parallel_mt64;
mutex parallel_lf24;
mutex parallel_lf48;
#endif

random_device rd;					// system random device
minstd_rand lc1;					// linear congruential generator (internal)
minstd_rand lc2;					// linear congruential generator (user)
mt19937 mt32;						// Mersenne-Twister 32 bits generator
mt19937_64 mt64;					// Mersenne-Twister 64 bits generator
ranlux24 lf24;						// lagged fibonacci 24 bits generator
ranlux48 lf48;						// lagged fibonacci 48 bits generator

void init_random( unsigned seed )
{
	idum = -seed;					// unused (legacy code only)
	lc1.seed( seed );				// linear congruential (internal)
	lc2.seed( seed );				// linear congruential (user)
	mt32.seed( seed );				// Mersenne-Twister 32 bits
	mt64.seed( seed );				// Mersenne-Twister 64 bits
	lf24.seed( seed );				// lagged fibonacci 24 bits
	lf48.seed( seed );				// lagged fibonacci 48 bits
}

template < class distr > double draw_rd( distr &d )
{
#ifndef _NP_
	// prevent concurrent draw by more than one thread
	lock_guard < mutex > lock( parallel_rd );
#endif
	return d( rd );
}

template < class distr > double draw_lc1( distr &d )
{
#ifndef _NP_
	// prevent concurrent draw by more than one thread
	lock_guard < mutex > lock( parallel_lc1 );
#endif
	return d( lc1 );
}

template < class distr > double draw_lc2( distr &d )
{
#ifndef _NP_
	// prevent concurrent draw by more than one thread
	lock_guard < mutex > lock( parallel_lc2 );
#endif
	return d( lc2 );
}

template < class distr > double draw_mt32( distr &d )
{
#ifndef _NP_
	// prevent concurrent draw by more than one thread
	lock_guard < mutex > lock( parallel_mt32 );
#endif
	return d( mt32 );
}

template < class distr > double draw_mt64( distr &d )
{
#ifndef _NP_
	// prevent concurrent draw by more than one thread
	lock_guard < mutex > lock( parallel_mt64 );
#endif
	return d( mt64 );
}

template < class distr > double draw_lf24( distr &d )
{
#ifndef _NP_
	// prevent concurrent draw by more than one thread
	lock_guard < mutex > lock( parallel_lf24 );
#endif
	return d( lf24 );
}

template < class distr > double draw_lf48( distr &d )
{
#ifndef _NP_
	// prevent concurrent draw by more than one thread
	lock_guard < mutex > lock( parallel_lf48 );
#endif
	return d( lf48 );
}


/***************************************************
DRAW_GEN
Generate the draw using current generator object
***************************************************/
template < class distr > double draw_gen( distr &d )
{
	switch ( ran_gen_id )
	{
		case 0:						// system (not pseudo) random generator
			return draw_rd( d );
		case 1:						// linear congruential in (0,1)
		case 3:						// linear congruential in [0,1)
		default:
			return draw_lc2( d );

		case 2:						// Mersenne-Twister 32 bits in (0,1)
		case 4:						// Mersenne-Twister 32 bits in [0,1)
			return draw_mt32( d );

		case 5:						// Mersenne-Twister 64 bits in [0,1)
			return draw_mt64( d );

		case 6:						// lagged fibonacci 24 bits in [0,1)
			return draw_lf24( d );

		case 7:						// lagged fibonacci 48 bits in [0,1)
			return draw_lf48( d );
	}
}


/***************************************************
SET_RANDOM
Set the generator object to be used in draws
***************************************************/
void *set_random( int gen )
{
	if ( gen >= 0 && gen <= 7 )
	{
		ran_gen_id = gen;

		switch ( ran_gen_id )
		{
			case 0:						// system (not pseudo) random generator
				if ( ! HW_RAND_GEN )
					plog( "\nWarning: true random generator not available\n" );
				return ( ( void * ) & rd );

			case 1:						// linear congruential in (0,1)
			case 3:						// linear congruential in [0,1)
				return ( ( void * ) & lc2 );

			case 2:						// Mersenne-Twister 32 bits in (0,1)
			case 4:						// Mersenne-Twister 32 bits in [0,1)
				return ( ( void * ) & mt32 );

			case 5:						// Mersenne-Twister 64 bits in [0,1)
				return ( ( void * ) & mt64 );

			case 6:						// lagged fibonacci 24 bits in [0,1)
				return ( ( void * ) & lf24 );
				break;
			case 7:						// lagged fibonacci 48 bits in [0,1)
				return ( ( void * ) & lf48 );
		}
	}

	return NULL;
}


/****************************************************
RND_INT
****************************************************/
int rnd_int( int min, int max )
{
	uniform_int_distribution< int > distr( min, max );
	return draw_lc1( distr );
}


/***************************************************
RAN1
Call the preset pseudo-random number generator
Just generates numbers > 0 and < 1
***************************************************/
double ran1( long *unused )
{
	double ran;
	uniform_real_distribution< double > distr( 0, 1 );

	do
		ran = draw_gen( distr );
	while ( ran == 0.0 && ran_gen_id < 3 );

	return ran;
}


/****************************************************
UNIFORM
****************************************************/
double uniform( double min, double max )
{
	uniform_real_distribution< double > distr( min, max );
	return draw_gen( distr );
}


/****************************************************
UNIFORM_INT
****************************************************/
double uniform_int( double min, double max )
{
	uniform_int_distribution< int > distr( ( long ) min, ( long ) max );
	return draw_gen( distr );
}


/***************************************************
NORM
***************************************************/
double norm( double mean, double dev )
{
	static bool normStopErr;

	if ( dev < 0 )
	{
		warn_distr( & normErrCnt, & normStopErr, "norm", "negative standard deviation" );
		return mean;
	}

	normal_distribution< double > distr( mean, dev );
	return draw_gen( distr );
}


/***************************************************
LNORM
Return a draw from a lognormal distribution
***************************************************/
double lnorm( double mean, double dev )
{
	static bool lnormStopErr;

	if ( dev < 0 )
	{
		warn_distr( & lnormErrCnt, & lnormStopErr, "lnorm", "negative standard deviation" );
		return exp( mean );
	}

	lognormal_distribution< double > distr( mean, dev );
	return draw_gen( distr );
}


/****************************************************
GAMMA
****************************************************/
double gamma( double alpha, double beta )
{
	static bool gammaStopErr;

	if ( alpha <= 0 || beta <= 0 )
	{
		warn_distr( & gammaErrCnt, & gammaStopErr, "gamma", "non-positive alpha or beta parameter" );
		return 0.0;
	}

	gamma_distribution< double > distr( alpha, beta );
	return draw_gen( distr );
}


/****************************************************
BERNOULLI
****************************************************/
double bernoulli( double p )
{
	static bool bernoStopErr;

	if ( p < 0 || p > 1 )
	{
		warn_distr( & bernoErrCnt, & bernoStopErr, "bernoulli", "probability out of \\[0, 1\\]" );

		if ( p < 0 )
			return 0.0;
		else
			return 1.0;
	}

	bernoulli_distribution distr( p );
	return draw_gen( distr );
}


/****************************************************
POISSON
****************************************************/
double poisson( double mean )
{
	static bool poissStopErr;

	if ( mean < 0 )
	{
		warn_distr( & poissErrCnt, & poissStopErr, "poisson", "negative mean" );
		return 0.0;
	}

	poisson_distribution< int > distr( mean );
	return draw_gen( distr );
}


/****************************************************
GEOMETRIC
****************************************************/
double geometric( double p )
{
	static bool geomStopErr;

	if ( p < 0 || p > 1 )
	{
		warn_distr( & geomErrCnt, & geomStopErr, "geometric", "probability out of \\[0, 1\\]" );

		if ( p < 0 )
			return 0.0;
		else
			return 1.0;
	}

	geometric_distribution< int > distr( p );
	return draw_gen( distr );
}


/****************************************************
BINOMIAL
****************************************************/
double binomial( double p, double t )
{
	static bool binomStopErr;

	if ( p < 0 || p > 1 || t <= 0 )
	{
		warn_distr( & binomErrCnt, & binomStopErr, "binomial", "invalid parameter" );

		if ( p < 0 || t <= 0 )
			return 0.0;
		else
			return 1.0;
	}

	binomial_distribution< int > distr( t, p );
	return draw_gen( distr );
}


/***************************************************
CAUCHY
***************************************************/
double cauchy( double a, double b )
{
	static bool cauchStopErr;

	if ( b <= 0 )
	{
		warn_distr( & cauchErrCnt, & cauchStopErr, "cauchy", "non-positive b parameter" );
		return a;
	}

	cauchy_distribution< double > distr( a, b );
	return draw_gen( distr );
}


/***************************************************
CHI_SQUARED
***************************************************/
double chi_squared( double n )
{
	static bool chisqStopErr;

	if ( n <= 0 )
	{
		warn_distr( & chisqErrCnt, & chisqStopErr, "chi_squared", "non-positive n parameter" );
		return 0.0;
	}

	chi_squared_distribution< double > distr( n );
	return draw_gen( distr );
}


/***************************************************
EXPONENTIAL
***************************************************/
double exponential( double lambda )
{
	static bool expStopErr;

	if ( lambda <= 0 )
	{
		warn_distr( & expErrCnt, & expStopErr, "exponential", "non-positive lambda parameter" );
		return 0.0;
	}

	exponential_distribution< double > distr( lambda );
	return draw_gen( distr );
}


/***************************************************
FISHER
***************************************************/
double fisher( double m, double n )
{
	static bool fishStopErr;

	if ( m <= 0 || n <= 0 )
	{
		warn_distr( & fishErrCnt, & fishStopErr, "fisher", "invalid parameter" );
		return 0.0;
	}

	fisher_f_distribution< double > distr( m, n );
	return draw_gen( distr );
}


/***************************************************
STUDENT
***************************************************/
double student( double n )
{
	static bool studStopErr;

	if ( n <= 0 )
	{
		warn_distr( & studErrCnt, & studStopErr, "student", "non-positive n parameter" );
		return 0.0;
	}

	student_t_distribution< double > distr( n );
	return draw_gen( distr );
}


/***************************************************
WEIBULL
***************************************************/
double weibull( double a, double b )
{
	static bool weibStopErr;

	if ( a <= 0 || b <= 0 )
	{
		warn_distr( & weibErrCnt, & weibStopErr, "weibull", "non-positive a or b parameter" );
		return 0.0;
	}

	weibull_distribution< double > distr( a, b );
	return draw_gen( distr );
}


/***************************************************
BETA
Return a draw from a Beta(alfa,beta) distribution
***************************************************/
double beta( double alpha, double beta )
{
	static bool betaStopErr;

	if ( alpha <= 0 || beta <= 0 )
	{
		warn_distr( & betaErrCnt, & betaStopErr, "beta", "non-positive alpha or beta parameter" );

		if ( alpha < beta )
			return 0.0;
		else
			return 1.0;
	}

	gamma_distribution< double > distr1( alpha, 1.0 ), distr2( beta, 1.0 );
	double draw = draw_gen( distr1 );
	return draw / ( draw + draw_gen( distr2 ) );
}


/****************************************************
PARETO
****************************************************/
double pareto( double mu, double alpha )
{
	static bool paretStopErr;

	if ( mu <= 0 || alpha <= 0 )
	{
		warn_distr( & paretErrCnt, & paretStopErr, "pareto", "non-positive mu or alpha parameter" );
		return mu;
	}

	return mu / pow( 1 - ran1( ), 1 / alpha );
}


/****************************************************
BPARETO
****************************************************/
double bpareto( double alpha, double low, double high )
{
	static bool paretStopErr;

	if ( alpha <= 0 || low <= 0 || low >= high )
	{
		warn_distr( & paretErrCnt, & paretStopErr, "bpareto", "non-positive alpha parameter or bounds or invalid bounds" );
		return max( low, 0 );
	}

	return pow( pow( low, alpha ) /
				( ran1( ) * ( pow( low / high, alpha ) - 1 ) + 1 ),
				1 / alpha );
}


/***************************************************
ALAPL
Return a draw from an asymmetric laplace distribution
***************************************************/
double alapl( double mu, double alpha1, double alpha2 )
{
	static bool alaplStopErr;

	if ( alpha1 <= 0 || alpha2 <= 0 )
	{
		warn_distr( & alaplErrCnt, & alaplStopErr, "alapl", "non-positive alpha1 or alpha2 parameter" );
		return mu;
	}

	double draw = ran1( );
	if ( draw < ( alpha1 / ( alpha1 + alpha2 ) ) )
		return mu + alpha1 * log( draw * ( 1 + alpha1 / alpha2 ) );
	else
		return mu - alpha2 * log( ( 1 - draw ) * ( 1 + alpha1 / alpha2 ) );
}


/****************************************************
WARN_DISTR
****************************************************/
void warn_distr( int *errCnt, bool *stopErr, const char *distr, const char *msg )
{
	if ( ++( *errCnt ) < ERR_LIM )	// prevent slow down due to I/O
	{
		plog( "\nWarning: %s in function '%s'", msg, distr );
		*stopErr = false;
	}
	else
		if ( ! *stopErr )
		{
			plog( "\nWarning: too many warnings in function '%s', stop reporting...\n", distr );
			*stopErr = true;
		}
}


/***************************************************
INIT_MATH_ERROR
Initialize the math functions error controls
***************************************************/
void init_math_error( void )
{
	normErrCnt = lnormErrCnt = gammaErrCnt = bernoErrCnt = poissErrCnt = 0;
	geomErrCnt = binomErrCnt = cauchErrCnt = chisqErrCnt = expErrCnt = 0;
	fishErrCnt = studErrCnt = weibErrCnt = betaErrCnt = paretErrCnt = alaplErrCnt = 0;
}
