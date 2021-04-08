/*************************************************************

	LSD 8.0 - March 2021
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

- void cmd( char *cc );
Standard routine to send the message string cc to the TCL interpreter in order
to execute a command for the graphical interfaces.
It should be enough to make a call to Tcl_Eval. But there are problems due to the
fact that this standard tcl function wants the string cc to be writable. Hence,
it shouldn't work when a constant string is passed. Actually, it worked under windows
but not under unix. Instead, I use Tcl_VarEval, that allows to use pieces
of strings (including the last terminating character NULL) and  it does not
require a writable string.

- void plog( char *m );
print  message string m in the Log screen.

- FILE *search_str( char *name, char *str )
given a string name, returns the file corresponding to name, and the current
position of the file is just after str. Think I don't use any longer.

- FILE *search_data_str( char *name, char *init, char *str )
given a string name, returns the file with that name and the current position
placed immediately after the string str found after the string init. Needed to
not get confused managing the data files, where the same string appears twice,
in the structure definition and in the data section.

- FILE *search_data_ent( char *name, variable *v )
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

#ifndef NP
mutex error;
#endif	


/*********************************
PLOG
The optional tag parameter has to correspond to the log window existing tags
*********************************/
#define NUM_TAGS 6
const char *tags[ NUM_TAGS ] = { "", "highlight", "tabel", "series", "prof1", "prof2" };

void plog( char const *cm, char const *tag, ... )
{
	char buffer[ TCL_BUFF_STR ], *message;
	bool tag_ok = false;
	int i, j;
	va_list argptr;

	for ( int i = 0; i < NUM_TAGS; ++i )
		if ( ! strcmp( tag, tags[ i ] ) )
			tag_ok = true;
	
#ifndef NP
	// abort if not running in main LSD thread
	if ( this_thread::get_id( ) != main_thread )
		return;
#endif

	va_start( argptr, tag );
	int maxSz = TCL_BUFF_STR - 40 - strlen( tag );
	int reqSz = vsnprintf( buffer, maxSz, cm, argptr );
	va_end( argptr );
	
	if ( reqSz >= maxSz )
		plog( "\nWarning: message truncated\n" );
	
	// remove invalid charaters and Tk control characters
	message = new char[ strlen( buffer ) + 1 ];
	for ( i = 0, j = 0; buffer[ i ] != '\0' ; ++i )
		if ( ( isprint( buffer[ i ] ) || buffer[ i ] == '\n' || 
			   buffer[ i ] == '\r' || buffer[ i ] == '\t' ) &&
			 ! ( buffer[ i ] == '\"' || 
				 ( buffer[ i ] == '$' && buffer[ i + 1 ] != '$' ) ) )
			message[ j++ ] = buffer[ i ];
	message[ j ] = '\0';

#ifdef NW 
	printf( "%s", message );
	fflush( stdout );
#else
	if ( ! tk_ok || ! log_ok )
		return;
	
	if ( tag_ok )
	{
		cmd( "set log_ok [ winfo exists .log ]" );
		cmd( "if $log_ok { .log.text.text.internal see [ .log.text.text.internal index insert ] }" );
		cmd( "if $log_ok { catch { .log.text.text.internal insert end \"%s\" %s } }", message, tag );
		cmd( "if $log_ok { .log.text.text.internal see end }" );
	}
	else
		plog( "\nError: invalid tag, message ignored:\n%s\n", "", message );
#endif 
	delete [ ] message;
	
	message_logged = true;
}


/***********
ERROR_HARD
Procedure called when an unrecoverable error occurs. 
Information about the state of the simulation when the error 
occured is provided. Users can abort the program or analyse 
the results collected up the latest time step available.
*************/
void error_hard( const char *logText, const char *boxTitle, const char *boxText, bool defQuit )
{
	if ( quit == 2 )		// simulation already being stopped
		return;
		
#ifndef NP
	// prevent concurrent use by more than one thread
	lock_guard < mutex > lock( error );
	
	// abort worker and park message if not running in main LSD thread
	if ( this_thread::get_id( ) != main_thread )
	{
		if ( ! error_hard_thread )	// handle just first error
		{
			error_hard_thread = true;
			strcpy( error_hard_msg1, boxTitle );
			strcpy( error_hard_msg2, logText );
			strcpy( error_hard_msg3, boxText );
			throw 1;
		}
		else
			return;
	}
#endif	
		
#ifndef NW
	if ( running )			// handle running events differently
	{
		cmd( "if [ winfo exists .deb ] { destroytop .deb }" );
		deb_log( false );	// close any open debug log file
		reset_plot( );		// show & disable run-time plot
		set_buttons_run( false );

		plog( "\n\nError detected at time %d", "highlight", t );
		plog( "\n\nError: %s\nDetails: %s", "", boxTitle, logText );
		if ( ! parallel_mode && stacklog != NULL && stacklog->vs != NULL )
			plog( "\nOffending code contained in the equation for variable: '%s'", "", stacklog->vs->label );
		plog( "\nSuggestion: %s", "", boxText );
		print_stack( );
		cmd( "focustop .log" );
		cmd( "ttk::messageBox -parent . -title Error -type ok -icon error -message \"[ string totitle {%s} ]\" -detail \"[ string totitle {%s} ].\n\nMore details are available in the Log window.\n\nSimulation cannot continue.\"", boxTitle, boxText  );
	}
	else
	{
		plog( "\n\nError: %s\nDetails: %s", "", boxTitle, logText );
		plog( "\nSuggestion: %s\n", "", boxText );
		cmd( "ttk::messageBox -parent . -title Error -type ok -icon error -message \"[ string totitle {%s} ]\" -detail \"[ string totitle {%s} ].\n\nMore details are available in the Log window.\"", boxTitle, boxText  );
	}
#endif

	if ( ! running )
		return;

	quit = 2;				// do not continue simulation

#ifndef NW
	uncover_browser( );
	cmd( "focustop .log" );

	cmd( "set err %d", defQuit ? 1 : 2 );

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

	choice = 0;
	while ( choice == 0 )
		Tcl_DoOneEvent( 0 );

	cmd( "set choice $err" );
	cmd( "destroytop .cazzo" );
	
	if ( choice == 3 )
	{
		if ( ! parallel_mode && fast_mode == 0 && stacklog != NULL && 
			 stacklog->vs != NULL && stacklog->vs->label != NULL )
		{
			char err_msg[ MAX_LINE_SIZE ];
			double useless = -1;
			sprintf( err_msg, "%s (ERROR)", stacklog->vs->label );
			deb( stacklog->vs->up, NULL, err_msg, &useless );
		}
		
		choice = 2;
	}

	if ( choice == 2 )
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

#ifndef NP
		// stop multi-thread workers
		delete [ ] workers;
		workers = NULL;
#endif	
		throw ( int ) 919293;			// force end of run() (in lsdmain.cpp)
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
		plog( "\n%d\t%s", "", app->ns, app->label );

	plog( "\n\n(the zero-level variable is computed by the simulation manager, \nwhile possible other variables are triggered by the lower level ones\nbecause necessary for completing their computation)\n" );
}


/****************************************************
SEARCH_STR
****************************************************/
FILE *search_str( char const *fname, char const *str )
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
FILE *search_data_str( char const *name, char const *init, char const *str )
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
FILE *search_data_ent( char *name, variable *v )
{
	FILE *f;
	char got[ MAX_LINE_SIZE ];
	char temp[ MAX_LINE_SIZE ];
	char temp1[ MAX_LINE_SIZE ];
	char typ[ 20 ];

	f = fopen( name, "r" );
	if ( f == NULL )
		return NULL;

	fscanf( f, "%999s", got );
	for ( int i = 0; strcmp( got, "DATA" ) && i < MAX_FILE_TRY; ++i )
		if ( fscanf( f, "%999s", got ) == EOF )
			return NULL;

	if ( strcmp( got, "DATA" ) )
		return NULL;

	strcpy( temp, ( v->up )->label );	// search for the section of the Object
	fscanf( f, "%999s", temp1 );
	fscanf( f, "%999s", got );

	for ( int i = 0; ( strcmp( got, temp ) || strcmp( temp1,"Object:" ) ) && i < MAX_FILE_TRY; ++i )
	{
		strcpy( temp1, got );
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
		strcpy( temp1, got );
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
	char app[ 4 * MAX_PATH_LENGTH ], app1[ TCL_BUFF_STR ];
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
			sprintf( app1, "%d_%s", cur->acounter, app );
		else
		{
			first = false;
			sprintf( app1, "%d", cur->acounter );
		}
		strcpy( app, app1 );
	} 

	if ( var->lab_tit != NULL )
		delete [ ] var->lab_tit;
	
	var->lab_tit = new char[ strlen( app ) + 1 ];
	strcpy( var->lab_tit, app );  
}


/***************************************************
SEARCH_DESCRIPTION
***************************************************/
description *search_description( char *lab )
{
	description *cur;
	variable *cv;

	for ( cur = descr; cur != NULL; cur = cur->next )
	{
		if ( ! strcmp( cur->label, lab ) )
			return cur;
	}
	
	cv = root->search_var( NULL, lab );
	
	if ( cv == NULL )
		return NULL;
	
	if ( cv->param == 0 )
		add_description( lab, "Variable", "(no description available )" );
	if ( cv->param == 1 )
		add_description( lab, "Parameter", "(no description available )" );  
	if ( cv->param == 2 )
		add_description( lab, "Function", "(no description available )" );  

	return search_description( lab );
} 


/***************************************************
AUTOFILL_DESCR
generate recur. the descriptions of the model as it is
***************************************************/
void autofill_descr( object *o )
{
	int i;
	bridge *cb;
	description *cur;
	variable *cv;

	cur = search_description( o->label );
	if ( cur == NULL )
		add_description( o->label, "Object", "(no description available)" );

	for ( cv = o->v; cv != NULL; cv = cv->next)
	{
		cur = search_description(cv->label);
		if ( cur == NULL )
		{
			i = cv->param;
			if ( i == 1 )
				add_description( cv->label, "Parameter", "(no description available)" );
			if ( i == 0 )
				add_description( cv->label, "Variable", "(no description available)" );
			if ( i == 2 )
				add_description( cv->label, "Function", "(no description available)" );
		} 
	} 
	 
	for ( cb = o->b; cb != NULL; cb = cb->next )
		if ( cb->head != NULL )
			autofill_descr( cb->head );
}


/***************************************************
CHANGE_DESCR_LAB
***************************************************/
void change_descr_lab( char const *lab_old, char const *lab, char const *type, char const *text, char const *init )
{
	description *cur, *cur1;

	for ( cur = descr; cur != NULL; cur = cur->next )
	{
		if ( ! strcmp( cur->label, lab_old ) )
		{

			if ( ! strcmp( lab, "" ) && ! strcmp( type, "" ) && ! strcmp( text, "" ) && ! strcmp(init, "" ) )
			{
				delete [ ] cur->label;
				delete [ ] cur->type;
				delete [ ] cur->text;
				delete [ ] cur->init;
		  
				if ( cur == descr )
				{
					descr = cur->next;	
					delete cur;
				}
				else
				{
					for ( cur1 = descr; cur1->next != cur; cur1 = cur1->next );
					
					cur1->next = cur->next;
					delete cur;
				} 
			}
			
			if ( strcmp( lab, "" ) )
			{
				delete [ ] cur->label;
				cur->label = new char [ strlen( lab ) + 1 ];
				strcpy( cur->label, lab );
			}
			
			if ( strcmp( type, "" ) )
			{
				delete [ ] cur->type;
				cur->type = new char [ strlen( type ) + 1 ];
				strcpy( cur->type, type );
			}
			
			if ( strcmp( text, "" ) )
			{
				delete [ ] cur->text;
				cur->text = new char [ strlen( text ) + 1 ];
				strcpy( cur->text, text );
			} 
			
			if ( strcmp( init, "" ) )
			{
				delete [ ] cur->init;
				cur->init = new char [ strlen( init ) + 1 ];
				strcpy( cur->init, init );
			} 

		   return;
		}
	}
}


/***************************************************
ADD_DESCRIPTION
***************************************************/
void add_description( char const *lab, char const *type, char const *text )
{
	description *cur;

	if ( descr == NULL )
		cur = descr = new description;
	else
	{ 
		for ( cur = descr; cur->next != NULL; cur = cur->next );
	  
		cur->next = new description;
		cur = cur->next;
	}  

	cur->next = NULL;
	cur->label = new char [ strlen( lab ) + 1 ];
	strcpy( cur->label, lab );
	cur->type = new char [ strlen( type ) + 1 ];
	strcpy( cur->type, type );
	if ( text != NULL && strlen( text ) != 0 )
	{
		cur->text = new char [ strlen( text ) + 1 ]; 
		strcpy( cur->text, text );
	}
	else
	{
		cur->text = new char[ 29 ]; 
		strcpy( cur->text, "(no description available)" );
	}
	   
	cur->init = NULL;
}


#ifndef NW

/****************************************************
SEARCH_ALL_SOURCES
****************************************************/
FILE *search_all_sources( char *str )
{
	char *fname, got[ MAX_LINE_SIZE ];
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
		fname = ( char * ) Tcl_GetVar( inter, "brr", 0 );
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
void return_where_used( char *lab, char s[ ] ) 
{
	char *r; 
	int choice = -1;

	scan_used_lab( lab, &choice );
	r = ( char * ) Tcl_GetVar( inter, "list_used", 0 );
	if ( r != NULL )
		strcpy( s, r);
	else
		strcpy( s, "" );
}


/***************************************************
CHANGE_DESCR_TEXT
***************************************************/
void change_descr_text( char *lab )
{
	description *cur;
	char *lab1;

	for ( cur = descr; cur != NULL; cur = cur->next )
	{
		if ( ! strcmp( cur->label, lab ) )
		{
			delete [ ] cur->text;
			lab1 = ( char * ) Tcl_GetVar( inter, "text_description", 0 );
			cur->text = new char[ strlen( lab1 ) + 1 ];
			strcpy( cur->text, lab1 );
			return;
	   }
	}
}


/***************************************************
CHANGE_INIT_TEXT
***************************************************/
void change_init_text( char *lab )
{
	description *cur;
	char *lab1;

	for ( cur = descr; cur != NULL; cur = cur->next )
	{
		if ( ! strcmp( cur->label, lab ) )
		{
			lab1 = ( char * ) Tcl_GetVar( inter, "text_description", 0 );
			if ( strlen( lab1 ) > 0 )
			{
				if ( cur->init != NULL )
				delete [ ] cur->init;
				cur->init = new char[ strlen( lab1 ) + 1 ];
				strcpy( cur->init, lab1 );
			}
			return;
		}
	}
}


/***************************************************
GET_VAR_DESCR
***************************************************/
void get_var_descr( char const *lab, char *descr, int descr_len )
{
	char str[ 2 * MAX_ELEM_LENGTH ], str1[ MAX_LINE_SIZE ];
	int i, j = 0, done = -1;
	FILE *f;
	
	sprintf( str, "EQUATION(\"%s\")", lab );
	f = search_all_sources( str );
	
	if ( f == NULL )
	{
		sprintf( str, "EQUATION_DUMMY(\"%s\",", lab );
		f = search_all_sources( str );
	}
	
	if ( f == NULL )
	{
		sprintf( str, "FUNCTION(\"%s\")", lab );
		f = search_all_sources( str );
	}
	
	if ( f == NULL )
	{
		sprintf( str, "if (!strcmp(label,\"%s\"))", lab );
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
						descr[ j++ ] = str1[ i ];

				if ( done == 2 && str1[ i ] == '\n' )
					done = -1; 
			 
				if ( j >= descr_len - 2 )
					done = 1;
			}
		}
		
		fclose( f );
	}
	descr[ j ] = '\0';
}


/***************************************************
AUTO_DOCUMENT
***************************************************/
void auto_document( int *choice, char const *lab, char const *which, bool append )
{
	bool var;
	char str1[ MAX_LINE_SIZE ], app[ 10 * MAX_LINE_SIZE ];
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
	  
			return_where_used( cd->label, str1 ); 
			if ( ( append || ! var ) && ! strstr( cd->text, "(no description available)" ) )
				if ( strlen( cd->text ) == 0 || ! strcmp( cd->text, "\n" ) )
					sprintf( msg, "%s\n'%s' appears in the equation for: %s", app, cd->label, str1 );
				else
					sprintf( msg, "%s\n%s\n'%s' appears in the equation for: %s", cd->text, app, cd->label, str1 );
			else
				sprintf( msg, "%s\n'%s' appears in the equation for: %s", app, cd->label, str1 );

			delete [ ] cd->text;
			cd->text = new char[ strlen( msg ) + 1 ];
			strcpy( cd->text, msg );
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


/****************************************************
GET_SAVED
****************************************************/
void get_saved( object *n, FILE *out, const char *sep, bool all_var )
{
	int i, sl;
	char *lab;
	bridge *cb;
	description *cd;
	object *co;
	variable *cv;

	for ( cv = n->v; cv != NULL; cv = cv->next )
		if ( cv->save || all_var )
		{
			// get element description
			cd = search_description( cv->label );
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
		
			fprintf( out, "%s%s%s%s%s%s%s\n", cv->label, sep, cv->param ? "parameter" : "variable", sep, n->label, sep, lab != NULL ? lab : "" );
		}

	for ( cb = n->b; cb != NULL; cb = cb->next )
	{
		if ( cb->head == NULL )
			co = blueprint->search( cb->blabel );
		else
			co = cb->head; 
		get_saved( co, out, sep, all_var );
	}
}


/****************************************************
GET_SA_LIMITS
****************************************************/
void get_sa_limits( object *r, FILE *out, const char *sep )
{
	int i, sl;
	char *lab;
	variable *cv;
	description *cd;
	sense *cs;
	
	for ( cs = rsense; cs != NULL; cs = cs->next )
	{
		// get current value (first object)
		cv = r->search_var( NULL, cs->label );
		
		// get element description
		cd = search_description( cs->label );
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
		for ( i = 0; cs->v != NULL &&  i < cs->nvalues; ++i )
			if ( cs->v[ i ] < min )
				min = cs->v[ i ];
			else
				if ( cs->v[ i ] > max )
					max = cs->v[ i ];

		fprintf( out, "%s%s%s%s%d%s%s%s%g%s%g%s%g%s\"%s\"\n", cs->label, sep, cs->param == 1 ? "parameter" : "variable", sep, cs->param == 1 ? 0 : cs->lag + 1, sep, cs->integer ? "integer" : "real", sep, cv != NULL ? cv->val[ cs->lag ] : NAN, sep, min, sep, max, sep, lab != NULL ? lab : "" );	
		
		delete [ ] lab;
	}
}


/***************************************************
SAVE_EQFILE
***************************************************/
void save_eqfile( FILE *f )
{
	if ( strlen( lsd_eq_file ) == 0 )
		strcpy( lsd_eq_file, eq_file );
	 
	fprintf( f, "\nEQ_FILE\n" );
	fprintf( f, "%s", lsd_eq_file );
	fprintf( f, "\nEND_EQ_FILE\n" );
}


#ifndef NW

/***************************************************
READ_EQ_FILENAME
***************************************************/
void read_eq_filename( char *s )
{
	char lab[ MAX_PATH_LENGTH ];
	FILE *f;

	sprintf( lab, "%s/%s", exec_path, MODEL_OPTIONS );
	f = fopen( lab, "r" );
	
	if ( f == NULL )
	{
		cmd( "ttk::messageBox -parent . -title Error -icon error -type ok -message \"File not found\" -detail \"File '$MODEL_OPTIONS' missing, cannot upload the equation file.\nYou may have to recreate your model configuration.\"" );
		return;
	}
	
	fscanf( f, "%499s", lab );
	for ( int i = 0; strncmp( lab, "FUN=", 4 ) && fscanf( f, "%499s", lab ) != EOF && i < MAX_FILE_TRY; ++i );    
	fclose( f );
	if ( strncmp( lab, "FUN=", 4 ) != 0 )
	{
		cmd( "ttk::messageBox -parent . -type ok -title -title Error -icon error -message \"File corrupted\" -detail \"File '$MODEL_OPTIONS' has invalid contents, cannot upload the equation file.\nYou may have to recreate your model configuration.\"" );
		return;
	}

	strcpy( s, lab + 4 );
	strcat( s, ".cpp" );

	return;
}


/***************************************************
COMPARE_EQFILE
***************************************************/
int compare_eqfile( void )
{
	char *s, lab[ MAX_PATH_LENGTH + 1 ];
	int i = MAX_FILE_SIZE;
	FILE *f;

	read_eq_filename( lab );
	f = fopen( lab, "r" );
	s = new char[ i + 1 ];
	while ( fgets( msg, MAX_LINE_SIZE, f ) != NULL )
	{
		i -= strlen( msg );
		if ( i < 0 )
			break;
		strcat( s, msg );
	}
	fclose( f );  
	
	if ( strcmp( s, lsd_eq_file ) == 0 )
		i = 0;
	else
		i = 1;
	delete [ ] s;

	return i;
}


/***************************************************
UPLOAD_EQFILE
***************************************************/
char *upload_eqfile( void )
{
	//load into the string eq_file the equation file
	char s[ MAX_PATH_LENGTH + 1 ], *eq;
	int i;
	FILE *f;

	Tcl_LinkVar( inter, "eqfiledim", ( char * ) &i, TCL_LINK_INT );

	read_eq_filename( s );
	cmd( "set eqfiledim [ file size %s ]", s );

	Tcl_UnlinkVar( inter, "eqfiledim" );

	eq = new char[ i + 1 ];
	eq[ 0 ] = '\0';
	f = fopen( s, "r");
	while ( fgets( msg, MAX_LINE_SIZE, f ) != NULL )
	{
		i -= strlen( msg );
		if ( i < 0 )
			break;
		strcat( eq, msg );
	}
	
	fclose( f );
	return eq;
}

#endif


/***************************************************
RESULT
Methods for results file saving (class result)
***************************************************/

// saves data to file in the specified period
void result::data( object *root, int initstep, int endtstep )
{
	// don't include initialization (t=0) in .csv format
	initstep = ( docsv && initstep < 1 ) ? 1 : initstep;
	// adjust for 1 time step if needed
	endtstep = ( endtstep == 0 ) ? initstep : endtstep;
	
	for ( int i = initstep; i <= endtstep; i++ )
	{
		firstCol = true;
		
		data_recursive( root, i );		// output one data line
		
		if ( dozip )					// and change line
			gzprintf( fz, "\n" );
		else
			fprintf( f, "\n" );
	}
}

void result::data_recursive( object *r, int i )
{
	bridge *cb;
	object *cur;
	variable *cv;

	for ( cv = r->v; cv != NULL; cv = cv->next )
	{
		if ( cv->save == 1 )
		{
			if ( cv->start <= i && cv->end >= i && ! is_nan( cv->data[ i - cv->start ] ) )
			{
				if ( dozip )
				{
					if ( docsv )
						gzprintf( fz, "%s%.*G", firstCol ? "" : CSV_SEP, SIG_DIG, cv->data[ i - cv->start ] );
					else
						gzprintf( fz, "%.*G\t", SIG_DIG, cv->data[ i - cv->start ] );
				}
				else
				{
					if ( docsv )
						fprintf( f, "%s%.*G", firstCol ? "" : CSV_SEP, SIG_DIG, cv->data[ i - cv->start ] );
					else
						fprintf( f, "%.*G\t", SIG_DIG, cv->data[ i - cv->start ] );
				}
			}
			else
			{
				if ( dozip )		// save NaN as n/a
				{
					if ( docsv )
						gzprintf( fz, "%s%s", firstCol ? "" : CSV_SEP, nonavail );
					else
						gzprintf( fz, "%s\t", nonavail );
				}
				else
				{
					if ( docsv )
						fprintf( f, "%s%s", firstCol ? "" : CSV_SEP, nonavail );
					else
						fprintf( f, "%s\t", nonavail );
				}
			}
			
			firstCol = false;
		}
	}
	 
	for ( cb = r->b; cb != NULL; cb = cb->next )
	{
		if ( cb->head == NULL )
			continue;
		
		cur = cb->head;
		if ( cur->to_compute )
			for ( ; cur != NULL; cur = cur->next )
				data_recursive( cur, i );
	}

	if ( r->up == NULL )
	{
		for ( cv = cemetery; cv != NULL; cv = cv->next )
		{
			if ( cv->start <= i && cv->end >= i && ! is_nan( cv->data[ i - cv->start ] ) )
			{
				if ( dozip )
				{
					if ( docsv )
						gzprintf( fz, "%s%.*G", firstCol ? "" : CSV_SEP, SIG_DIG, cv->data[ i - cv->start ] );
					else
						gzprintf( fz, "%.*G\t", SIG_DIG, cv->data[ i - cv->start ] );
				}
				else
				{
					if ( docsv )
						fprintf( f, "%s%.*G", firstCol ? "" : CSV_SEP, SIG_DIG, cv->data[ i - cv->start ] );
					else
						fprintf( f, "%.*G\t", SIG_DIG, cv->data[ i - cv->start ] );
				}
			}
			else					// save NaN as n/a
			{
				if ( dozip )
				{
					if ( docsv )
						gzprintf( fz, "%s%s", firstCol ? "" : CSV_SEP, nonavail );
					else
						gzprintf( fz, "%s\t", nonavail );
				}
				else
				{
					if ( docsv )
						fprintf( f, "%s%s", firstCol ? "" : CSV_SEP, nonavail );
					else
						fprintf(f, "%s\t", nonavail );
				}
			}
						
			firstCol = false;
		}
	}
}

// saves header to file
void result::title( object *root, int flag )
{
	firstCol = true;
	
	title_recursive( root, flag );		// output header
		
	if ( dozip )						// and change line
		gzprintf( fz, "\n" );
	else
		fprintf( f, "\n" );
}

void result::title_recursive( object *r, int header )
{
	bool single = false;
	bridge *cb;
	object *cur;
	variable *cv;

	for ( cv = r->v; cv != NULL; cv = cv->next )
	{
		if ( cv->save == 1 )
		{
			set_lab_tit( cv );
			if ( ( ! strcmp( cv->lab_tit, "1" ) || ! strcmp( cv->lab_tit, "1_1" ) || ! strcmp( cv->lab_tit, "1_1_1" ) || ! strcmp( cv->lab_tit, "1_1_1_1" ) ) && cv->up->hyper_next( ) == NULL )
				single = true;					// prevent adding suffix to single objects
			
			if ( header )
			{
				if ( dozip )
				{
					if ( docsv )
						gzprintf( fz, "%s%s%s%s", firstCol ? "" : CSV_SEP, cv->label, single ? "" : "_", single ? "" : cv->lab_tit );
					else
						gzprintf( fz, "%s %s (%d %d)\t", cv->label, cv->lab_tit, cv->start, cv->end );
				}
				else
				{
					if ( docsv )
						fprintf( f, "%s%s%s%s", firstCol ? "" : CSV_SEP, cv->label, single ? "" : "_", single ? "" : cv->lab_tit );
					else
						fprintf( f, "%s %s (%d %d)\t", cv->label, cv->lab_tit, cv->start, cv->end );
				}
			}
			else
			{
				if ( dozip )
				{
					if ( docsv )
						gzprintf( fz, "%s%s%s%s", firstCol ? "" : CSV_SEP, cv->label, single ? "" : "_", single ? "" : cv->lab_tit );
					else
						gzprintf( fz, "%s %s (-1 -1)\t", cv->label, cv->lab_tit );
				}
				else
				{
					if ( docsv )
						fprintf( f, "%s%s%s%s", firstCol ? "" : CSV_SEP, cv->label, single ? "" : "_", single ? "" : cv->lab_tit );
					else
						fprintf( f, "%s %s (-1 -1)\t", cv->label, cv->lab_tit );
				}
			}
			
			firstCol = false;
		}
	}
	 
	for ( cb = r->b; cb != NULL; cb = cb->next )
	{
		if ( cb->head == NULL )
			continue;
		
		cur = cb->head;
		if ( cur->to_compute )
		{
			for ( ; cur != NULL; cur = cur->next )
			title_recursive( cur, header );
		} 
	} 

	if ( r->up == NULL )
	{
		for ( cv = cemetery; cv != NULL; cv = cv->next )
		{
			if ( dozip )
			{
				if ( docsv )
					gzprintf( fz, "%s%s%s%s", firstCol ? "" : CSV_SEP, cv->label, single ? "" : "_", single ? "" : cv->lab_tit );
				else
					gzprintf( fz, "%s %s (%d %d)\t", cv->label, cv->lab_tit, cv->start, cv->end );
			}
			else
			{
				if ( docsv )
					fprintf( f, "%s%s%s%s", firstCol ? "" : CSV_SEP, cv->label, single ? "" : "_", single ? "" : cv->lab_tit );
				else
					fprintf( f, "%s %s (%d %d)\t", cv->label, cv->lab_tit, cv->start, cv->end );
			}
			
			firstCol = false;
		}
	}
}

// open the appropriate file for saving the results (constructor)
result::result( char const *fname, char const *fmode, bool dozip, bool docsv )
{
	this->docsv = docsv;
	this->dozip = dozip;		// save local class flag
	if ( dozip )
	{
		char *fnamez = new char[ strlen( fname ) + 4 ];	// append .gz to the file name
		strcpy( fnamez, fname );
		strcat( fnamez, ".gz");
		fz = gzopen( fnamez, fmode );
		delete [ ] fnamez;
	}
	else
		f = fopen( fname, fmode );
}

// close the appropriate results file (destructor)
result::~result( void )
{
	if ( dozip )
		gzclose( fz );
	else
		fclose( f );
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
double init_lattice( double pixW, double pixH, double nrow, double ncol, char const lrow[ ], char const lcol[ ], char const lvar[ ], object *p, int init_color )
{
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
		
#ifndef NW

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

	// create the window with the lattice, roughly 600 pixels as maximum dimension
	cmd( "newtop .lat \"%s%s - LSD Lattice (%.0lf x %.0lf)\" { destroytop .lat } \"\"", unsaved_change() ? "*" : " ", simul_name, nrow, ncol );

	cmd( "bind .lat <Button-2> { .lat.b.ok invoke }" );
	cmd( "bind .lat <Button-3> { event generate .lat <Button-2> -x %%x -y %%y }" );

	char init_color_string[ 32 ];		// the final string to be used to define tk color to use

	if ( init_color < 0 && ( - init_color ) <= 0xffffff )		// RGB mode selected?
		sprintf( init_color_string, "#%06x", - init_color );	// yes: just use the positive RGB value
	else
	{
		sprintf( init_color_string, "$c%d", init_color );		// no: use the positive RGB value
		// create (background color) pallete entry if invalid palette in init_color
		cmd( "if { ! [ info exist c%d ] } { set c%d $colorsTheme(bg) }", init_color, init_color  );
	}
			
	cmd( "ttk::canvas .lat.c -height %d -width %d -entry 0 -dark $darkTheme", ( unsigned int ) pixH, ( unsigned int ) pixW );
	
	if ( init_color != 1001 )
		cmd( ".lat.c configure -background %s", init_color_string );

	cmd( "pack .lat.c" );

	cmd( "save .lat b { set b \"%s.eps\"; set a [ tk_getSaveFile -parent .lat -title \"Save Lattice Image File\" -defaultextension .eps -initialfile $b -initialdir \"%s\" -filetypes { { {Encapsulated Postscript files} {.eps} } { {All files} {*} } } ]; if { $a != \"\" } { .lat.c postscript -colormode color -file \"$a\" } }", simul_name, path );

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
	
#ifndef NW
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
	char *latcanv, val_string[ 32 ];		// the final string to be used to define tk color to use
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
#ifndef NW

	// avoid operation if canvas was closed
	cmd( "if [ winfo exists .lat.c ] { set latcanv 1 } { set latcanv 0 }" );
	latcanv = ( char * ) Tcl_GetVar( inter, "latcanv", 0 );
	if ( ! strcmp( latcanv, "0" ) )
		return -1;
	
	if ( val < 0 && ( - ( int )  val ) <= 0xffffff )	// RGB mode selected?
		sprintf( val_string, "#%06x", - ( int ) val );	// yes: just use the positive RGB value
	else
	{
		sprintf( val_string, "$c%d", val_int );			// no: use the predefined Tk color
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
	char *latcanv;

#ifndef NW

	// avoid operation if no canvas or no file name
	cmd( "if [ winfo exists .lat.c ] { set latcanv \"1\" } { set latcanv \"0\" }" );
	latcanv = ( char * ) Tcl_GetVar( inter, "latcanv", 0 );
	if ( latcanv == NULL || strlen( fname ) == 0 )
		return -1;
	
	Tcl_SetVar( inter, "latname", fname, 0 );
	cmd( "append latname \".eps\"; .lat.c postscript -colormode color -file $latname" );

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

#ifndef NP
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
#ifndef NP
	// prevent concurrent draw by more than one thread
	lock_guard < mutex > lock( parallel_rd );
#endif	
	return d( rd );
}

template < class distr > double draw_lc1( distr &d )
{
#ifndef NP
	// prevent concurrent draw by more than one thread
	lock_guard < mutex > lock( parallel_lc1 );
#endif	
	return d( lc1 );
}

template < class distr > double draw_lc2( distr &d )
{
#ifndef NP
	// prevent concurrent draw by more than one thread
	lock_guard < mutex > lock( parallel_lc2 );
#endif	
	return d( lc2 );
}

template < class distr > double draw_mt32( distr &d )
{
#ifndef NP
	// prevent concurrent draw by more than one thread
	lock_guard < mutex > lock( parallel_mt32 );
#endif	
	return d( mt32 );
}

template < class distr > double draw_mt64( distr &d )
{
#ifndef NP
	// prevent concurrent draw by more than one thread
	lock_guard < mutex > lock( parallel_mt64 );
#endif	
	return d( mt64 );
}

template < class distr > double draw_lf24( distr &d )
{
#ifndef NP
	// prevent concurrent draw by more than one thread
	lock_guard < mutex > lock( parallel_lf24 );
#endif	
	return d( lf24 );
}

template < class distr > double draw_lf48( distr &d )
{
#ifndef NP
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
		plog( "\nWarning: %s in function '%s'", "", msg, distr );
		*stopErr = false;
	}
	else
		if ( ! *stopErr )
		{
			plog( "\nWarning: too many warnings in function '%s', stop reporting...\n", "", distr );
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
