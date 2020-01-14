/*************************************************************

	LSD 7.2 - December 2019
	written by Marco Valente, Universita' dell'Aquila
	and by Marcelo Pereira, University of Campinas

	Copyright Marco Valente and Marcelo Pereira
	LSD is distributed under the GNU General Public License
	
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

- int my_strcmp( char *a, char *b )
It is a normal strcmp, but it catches the possibility of both strings being
NULL

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

#ifdef CPP11
#include <random>						// use new random libraries if possible
#endif


int **lattice = NULL;					// lattice data colors array
int rows = 0;							// lattice size
int columns = 0;
int error_count;						// error counters
int	normErrCnt, lnormErrCnt, gammaErrCnt, bernoErrCnt, poissErrCnt;
int geomErrCnt, binomErrCnt, cauchErrCnt, chisqErrCnt, expErrCnt;
int fishErrCnt, studErrCnt, weibErrCnt, betaErrCnt, paretErrCnt, alaplErrCnt;
double dimW = 0;						// lattice screen size
double dimH = 0;

#ifdef PARALLEL_MODE
mutex error;
#endif	


#ifndef NO_WINDOW

/****************************************************
CMD
****************************************************/
bool firstCall = true;

void cmd( const char *cm, ... )
{
	char message[ TCL_BUFF_STR ];
	
	// abort if Tcl interpreter not initialized
	if ( inter == NULL )
	{
		printf( "\nTcl interpreter not initialized. Quitting LSD now.\n" );
		myexit( 24 );
	}
	
#ifdef PARALLEL_MODE
	// abort if not running in main LSD thread
	if ( this_thread::get_id( ) != main_thread )
		return;
#endif
	
	if ( strlen( cm ) >= TCL_BUFF_STR )
	{
		sprintf( message, "Tcl buffer overrun. Please increase TCL_BUFF_STR in 'decl.h' to at least %lu bytes.", ( long unsigned int ) strlen( cm ) + 1 );
		log_tcl_error( cm, message );
		if ( tk_ok )
			cmd( "tk_messageBox -type ok -title Error -icon error -message \"Tcl buffer overrun (memory corrupted!)\" -detail \"LSD will close immediately after pressing 'OK'.\"" );
		myexit( 24 );
	}

	char buffer[ TCL_BUFF_STR ];
	va_list argptr;
	
	va_start( argptr, cm );
	int reqSz = vsnprintf( buffer, TCL_BUFF_STR, cm, argptr );
	va_end( argptr );
	
	if ( reqSz >= TCL_BUFF_STR )
	{
		sprintf( message, "Tcl buffer too small. Please increase TCL_BUFF_STR in 'decl.h' to at least %d bytes.", reqSz + 1 );
		log_tcl_error( cm, message );
		if ( tk_ok )
			cmd( "tk_messageBox -type ok -title Error -icon error -message \"Tcl buffer too small\" -detail \"Tcl/Tk command was canceled.\"" );
	}
	else
	{
		int code = Tcl_Eval( inter, buffer );

		if ( code != TCL_OK )
			log_tcl_error( cm, Tcl_GetStringResult( inter ) );
	}
}


/****************************************************
LOG_TCL_ERROR
****************************************************/
void log_tcl_error( const char *cm, const char *message )
{
	FILE *f;
	char fname[ MAX_PATH_LENGTH ];
	time_t rawtime;
	struct tm *timeinfo;
	char ftime[ 80 ];

	if ( strlen( exec_path ) > 0 )
		sprintf( fname, "%s/LSD.err", exec_path );
	else
		sprintf( fname, "LSD.err" );

	f = fopen( fname,"a" );
	if ( f == NULL )
	{
		plog( "\nCannot write log file to disk. Check write permissions\n" );
		return;
	}

	time( &rawtime );
	timeinfo = localtime( &rawtime );
	strftime ( ftime, 80, "%x %X", timeinfo );

	if ( firstCall )
	{
		firstCall = false;
		fprintf( f,"\n\n====================> NEW TCL SESSION\n" );
	}
	fprintf( f, "\n(%s)\nCommand:\n%s\nMessage:\n%s\n-----\n", ftime, cm, message );
	fclose( f );
	
	plog( "\nInternal LSD error. See file '%s'\n", "", fname );
}

#else

void cmd( const char *cm, ... )
{
}
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
	
	// handle the "bar" pseudo tag
	if ( ! strcmp( tag, "bar" ) )
		tag_ok = true;
	else
		on_bar = false;
	
#ifdef PARALLEL_MODE
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

#ifdef NO_WINDOW 
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
		
#ifdef PARALLEL_MODE
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
		
#ifndef NO_WINDOW
	if ( running )			// handle running events differently
	{
		cmd( "if [ winfo exists .deb ] { destroytop .deb }" );
		deb_log( false );	// close any open debug log file
		reset_plot( cur_sim );	// allow closing run-time plot
		set_buttons_log( false );

		plog( "\n\nError detected at time %d", "highlight", t );
		plog( "\n\nError: %s\nDetails: %s", "", boxTitle, logText );
		if ( ! parallel_mode && stacklog != NULL && stacklog->vs != NULL )
			plog( "\nOffending code contained in the equation for variable: '%s'", "", stacklog->vs->label );
		plog( "\nSuggestion: %s", "", boxText );
		print_stack( );
		cmd( "wm deiconify .log; raise .log; focus -force .log" );
		cmd( "tk_messageBox -parent . -title Error -type ok -icon error -message \"[ string totitle {%s} ]\" -detail \"[ string totitle {%s} ].\n\nMore details are available in the Log window.\n\nSimulation cannot continue.\"", boxTitle, boxText  );
	}
	else
	{
		plog( "\n\nError: %s\nDetails: %s", "", boxTitle, logText );
		plog( "\nSuggestion: %s\n", "", boxText );
		cmd( "tk_messageBox -parent . -title Error -type ok -icon error -message \"[ string totitle {%s} ]\" -detail \"[ string totitle {%s} ].\n\nMore details are available in the Log window.\"", boxTitle, boxText  );
	}
#endif

	if ( ! running )
		return;

	quit = 2;				// do not continue simulation

#ifndef NO_WINDOW
	uncover_browser( );
	cmd( "wm deiconify .; wm deiconify .log; raise .log; focus -force .log" );

	cmd( "set err %d", defQuit ? 1 : 2 );

	cmd( "newtop .cazzo Error" );

	cmd( "frame .cazzo.t" );
	cmd( "label .cazzo.t.l -fg red -text \"An error occurred during the simulation\"" );
	cmd( "pack .cazzo.t.l -pady 10" );
	cmd( "label .cazzo.t.l1 -text \"Information about the error\nis reported in the log window.\nResults are available in the LSD browser.\"" );
	cmd( "pack .cazzo.t.l1" );

	cmd( "frame .cazzo.e" );
	cmd( "label .cazzo.e.l -text \"Choose one option to continue\"" );

	cmd( "frame .cazzo.e.b -relief groove -bd 2" );
	cmd( "radiobutton .cazzo.e.b.r -variable err -value 2 -text \"Return to LSD Browser to edit the model configuration\"" );
	cmd( "radiobutton .cazzo.e.b.d -variable err -value 3 -text \"Open LSD Debugger on the offending variable and object instance\"" );
	cmd( "radiobutton .cazzo.e.b.e -variable err -value 1 -text \"Quit LSD Browser to edit the model equations' code in LMM\"" );

	cmd( "pack .cazzo.e.b.r .cazzo.e.b.d .cazzo.e.b.e -anchor w" );

	cmd( "pack .cazzo.e.l .cazzo.e.b" );

	cmd( "pack .cazzo.t .cazzo.e -padx 5 -pady 5" );

	cmd( "okhelp .cazzo b { set choice 1 }  { LsdHelp debug.html#crash }" );

	cmd( "showtop .cazzo centerS" );
	
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
		unwind_stack( );
		actual_steps = t;
		unsavedData = true;				// flag unsaved simulation results
		running = false;
		
		// run user closing function, reporting error appropriately
		user_exception = true;
		close_sim( );
		user_exception = false;
		
		reset_end( root );
		root->emptyturbo( );
		uncover_browser( );

#ifdef PARALLEL_MODE
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
MSLEEP
****************************************************/
#ifdef _WIN32
#include <windows.h>
void msleep( unsigned msec )
{
	Sleep( msec );
	return;
}
#else
#include <unistd.h>	
void msleep( unsigned msec )
{
	usleep( msec * 1000 );
	return;
}
#endif


/****************************************************
SEARCH_STR
****************************************************/
FILE *search_str( char const *name, char const *str )
{
	FILE *f;
	char got[ MAX_LINE_SIZE ];

	f = fopen( name, "r" );
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

	for ( cur = descr; cur != NULL; cur = cur->next )
	{
		if ( ! strcmp( cur->label, lab ) )
			return cur;
	}
	return NULL;
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
void change_descr_lab(char const *lab_old, char const *lab, char const *type, char const *text, char const *init)
{
	description *cur, *cur1;

	for (cur=descr; cur!=NULL; cur=cur->next)
	 {
	  if (!strcmp(cur->label, lab_old) )
	   {

	   if (!strcmp(lab, "" ) && !strcmp(type, "" ) && !strcmp(text, "" ) && !strcmp(init, "" ) )
		{
		 delete [ ] cur->label;
		 delete [ ] cur->type;
		 delete [ ] cur->text;
		 if (cur->init != NULL )
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
	   if (strcmp(lab, "" ) )
		{
		 delete [ ] cur->label;
		 cur->label=new char[strlen(lab)+1];
		 strcpy(cur->label, lab);
		} 
	   if (strcmp(type, "" ) )
		{
		 delete [ ] cur->type;
		 cur->type=new char[strlen(type)+1];
		 strcpy(cur->type, type);
		}
	   if (strcmp(text, "" ) )
		{
		 delete [ ] cur->text;
		 cur->text=new char[strlen(text)+1];
		 strcpy(cur->text, text);
		} 
	   if (strcmp(init, "" ) )
		{
		 if (cur->init != NULL )
		   delete [ ] cur->init;
		 cur->init=new char[strlen(init)+1];
		 strcpy(cur->init, init);
		} 

	   return;
	  
	   }
	 }
}


/***************************************************
ADD_DESCRIPTION
***************************************************/
void add_description(char const *lab, char const *type, char const *text)
{
	description *cur;

	if (descr == NULL )
		cur=descr=new description;
	else
	{ 
		for (cur=descr; cur->next!=NULL; cur=cur->next);
	  
		cur->next=new description;
		cur=cur->next;
	}  

	cur->next=NULL;
	cur->label=new char[strlen(lab)+1];
	strcpy(cur->label, lab);
	cur->type=new char[strlen(type)+1];
	strcpy(cur->type, type);
	if (text!=NULL && strlen(text) != 0 )
	{
		cur->text=new char[strlen(text)+1]; 
		strcpy(cur->text, text);
	}
	else
	{
		cur->text=new char[29]; 
		strcpy(cur->text, "(no description available)");
	}
	   
	cur->init=NULL;
}


#ifndef NO_WINDOW

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
	get_int( "res", & nfiles );
	
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


/***************************************************
GET_BOOL
***************************************************/
bool get_bool( const char *tcl_var, bool *var )
{
	int intvar;
	sscanf( ( char * ) Tcl_GetVar( inter, tcl_var, 0 ), "%d", & intvar );
	if ( var != NULL )
		*var = intvar ? true : false;
	return ( intvar ? true : false );
}


/***************************************************
GET_INT
***************************************************/
int get_int( const char *tcl_var, int *var )
{
	int intvar;
	sscanf( ( char * ) Tcl_GetVar( inter, tcl_var, 0 ), "%d", & intvar );
	if ( var != NULL )
		*var = intvar;
	return intvar;
}


/***************************************************
GET_LONG
***************************************************/
long get_long( const char *tcl_var, long *var )
{
	long longvar;
	sscanf( ( char * ) Tcl_GetVar( inter, tcl_var, 0 ), "%ld", & longvar );
	if ( var != NULL )
		*var = longvar;
	return longvar;
}


/***************************************************
GET_DOUBLE
***************************************************/
double get_double( const char *tcl_var, double *var )
{
	double dblvar;
	sscanf( ( char * ) Tcl_GetVar( inter, tcl_var, 0 ), "%lf", & dblvar );
	if ( var != NULL )
		*var = dblvar;
	return dblvar;
}

#endif


/***************************************************
ADD_CEMETERY
Store the variable in a list of variables in objects deleted
but to be used for analysis.
***************************************************/
void add_cemetery( variable *v )
{
	variable *cv;
	
	if ( v->savei )
		save_single( v );
	
	if ( cemetery == NULL )
	{
		cemetery = v;
		v->next = NULL;
	}
	else
	{
		for ( cv = cemetery; cv->next != NULL; cv = cv->next );
	  
		cv->next = v;
		v->next = NULL;
	}
}


/***************************************************
COLLECT_CEMETERY
Processes variables from an object required to go to cemetery 
***************************************************/
void collect_cemetery( object *o )
{
	variable *cv, *nv;
	
	for ( cv = o->v; cv != NULL; cv = nv )	// scan all variables
	{
		nv = cv->next;						// pointer to next variable
		
		// need to save?
		if ( running && ( cv->save == true || cv->savei == true ) )
		{
			cv->end = t;					// define last period,
			cv->data[ t ] = cv->val[ 0 ];	// last value
			set_lab_tit( cv );				// and last lab_tit
			add_cemetery( cv );				// put in cemetery (destroy cv->next)
		}
	}
}


/***************************************************
EMPTY_CEMETERY
***************************************************/
void empty_cemetery( void )
{
	variable *cv, *cv1;
	
	for ( cv = cemetery; cv !=NULL ; )
	{
		cv1 = cv->next;
		cv->empty( );
		delete cv;
		cv = cv1;
	}
	
	cemetery = NULL;
}


/****************************************************
MY_STRCMP
****************************************************/
int my_strcmp( char *a, char *b )
{
	int res;
	if ( a == NULL && b == NULL )
		return 0;

	res = strcmp( a, b );
	return res;
}


/***************************************************
KILL_INITIAL_NEWLINE
***************************************************/
void kill_initial_newline( char *s )
{
	char *d;
	int i, j;
	
	j = strlen( s );

	d = new char[ j + 1 ];

	for ( i = 0; i < j; ++i )
		if ( s[ i ] != '\n' )
			break;

	strcpy( d, s + i );
	strcpy( s, d );
	delete [ ] d;
}


/***************************************************
KILL_TRAILING_NEWLINE
***************************************************/
void kill_trailing_newline( char *s )
{
	int i, done = 0;
	
	kill_initial_newline( s );

	while ( done == 0 )
	{ 
		done = 1;
		for ( i = 0; s[ i ] != '\0'; ++i )
			if ( s[ i ] == '\n' && s[ i + 1 ] == '\0' )
			{
				s[ i ] = '\0';
				done = 0;
			} 
	}
}


/***************************************************
CLEAN_SPACES
***************************************************/
void clean_spaces( char *s )
{
	int i, j;
	char app[ MAX_LINE_SIZE ];

	app[ MAX_LINE_SIZE - 1 ] = '\0';
	for ( j = 0, i = 0; s[ i ] != '\0' && i < MAX_LINE_SIZE - 1; ++i )
		switch ( s[ i ] )
		{
			case ' ':
			case '\t':
				break;
				
			default: 
				app[ j++ ] = s[ i ];
				break;
		}
		
	app[ j ] = '\0';
	strcpy( s, app );
}


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


#ifndef NO_WINDOW

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
		cmd( "tk_messageBox -parent . -title Error -icon error -type ok -message \"File not found\" -detail \"File '$MODEL_OPTIONS' missing, cannot upload the equation file.\nYou may have to recreate your model configuration.\"" );
		return;
	}
	
	fscanf( f, "%499s", lab );
	for ( int i = 0; strncmp( lab, "FUN=", 4 ) && fscanf( f, "%499s", lab ) != EOF && i < MAX_FILE_TRY; ++i );    
	fclose( f );
	if ( strncmp( lab, "FUN=", 4 ) != 0 )
	{
		cmd( "tk_messageBox -parent . -type ok -title -title Error -icon error -message \"File corrupted\" -detail \"File '$MODEL_OPTIONS' has invalid contents, cannot upload the equation file.\nYou may have to recreate your model configuration.\"" );
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
		
		if ( dozip )				// and change line
		{
#ifdef LIBZ
			gzprintf( fz, "\n" );
#endif
		}
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
			if ( cv->start <= i && cv->end >= i && ! is_nan( cv->data[ i ] ) )
			{
				if ( dozip )
				{
#ifdef LIBZ
					if ( docsv )
						gzprintf( fz, "%s%.*G", firstCol ? "" : CSV_SEP, SIG_DIG, cv->data[ i ] );
					else
						gzprintf( fz, "%.*G\t", SIG_DIG, cv->data[ i ] );
#endif
				}
				else
				{
					if ( docsv )
						fprintf( f, "%s%.*G", firstCol ? "" : CSV_SEP, SIG_DIG, cv->data[ i ] );
					else
						fprintf( f, "%.*G\t", SIG_DIG, cv->data[ i ] );
				}
			}
			else
			{
				if ( dozip )		// save NaN as n/a
				{
#ifdef LIBZ
					if ( docsv )
						gzprintf( fz, "%s%s", firstCol ? "" : CSV_SEP, nonavail );
					else
						gzprintf( fz, "%s\t", nonavail );
#endif
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
			if ( cv->start <=i && cv->end >= i && ! is_nan( cv->data[ i ] ) )
			{
				if ( dozip )
				{
#ifdef LIBZ
					if ( docsv )
						gzprintf( fz, "%s%.*G", firstCol ? "" : CSV_SEP, SIG_DIG, cv->data[ i ] );
					else
						gzprintf( fz, "%.*G\t", SIG_DIG, cv->data[ i ] );
#endif
				}
				else
				{
					if ( docsv )
						fprintf( f, "%s%.*G", firstCol ? "" : CSV_SEP, SIG_DIG, cv->data[ i ] );
					else
						fprintf( f, "%.*G\t", SIG_DIG, cv->data[ i ] );
				}
			}
			else					// save NaN as n/a
			{
				if ( dozip )
				{
#ifdef LIBZ
					if ( docsv )
						gzprintf( fz, "%s%s", firstCol ? "" : CSV_SEP, nonavail );
					else
						gzprintf( fz, "%s\t", nonavail );
#endif
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
	{
#ifdef LIBZ
		gzprintf( fz, "\n" );
#endif
	}
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
#ifdef LIBZ
					if ( docsv )
						gzprintf( fz, "%s%s%s%s", firstCol ? "" : CSV_SEP, cv->label, single ? "" : "_", single ? "" : cv->lab_tit );
					else
						gzprintf( fz, "%s %s (%d %d)\t", cv->label, cv->lab_tit, cv->start, cv->end );
#endif
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
#ifdef LIBZ
					if ( docsv )
						gzprintf( fz, "%s%s%s%s", firstCol ? "" : CSV_SEP, cv->label, single ? "" : "_", single ? "" : cv->lab_tit );
					else
						gzprintf( fz, "%s %s (-1 -1)\t", cv->label, cv->lab_tit );
#endif
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
#ifdef LIBZ
				if ( docsv )
					gzprintf( fz, "%s%s%s%s", firstCol ? "" : CSV_SEP, cv->label, single ? "" : "_", single ? "" : cv->lab_tit );
				else
					gzprintf( fz, "%s %s (%d %d)\t", cv->label, cv->lab_tit, cv->start, cv->end );
#endif
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
#ifndef LIBZ
	dozip = false;				// disable zip if libraries not available
#endif
	this->docsv = docsv;
	this->dozip = dozip;		// save local class flag
	if ( dozip )
	{
#ifdef LIBZ
			char *fnamez = new char[ strlen( fname ) + 4 ];	// append .gz to the file name
			strcpy( fnamez, fname );
			strcat( fnamez, ".gz");
			fz = gzopen( fnamez, fmode );
			delete [ ] fnamez;
#endif
	}
	else
		f = fopen( fname, fmode );
}

// close the appropriate results file (destructor)
result::~result( void )
{
	if ( dozip )
	{
#ifdef LIBZ
		gzclose( fz );
#endif
	}
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
		
#ifndef NO_WINDOW

	get_int( "hsizeLat", & hsize );			// 400
	get_int( "vsizeLat", & vsize );			// 400
	get_int( "hsizeLatMax", & hsizeMax );	// 1024
	get_int( "vsizeLatMax", & vsizeMax );	// 1024

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
		// create (white) pallete entry if invalid palette in init_color
		cmd( "if { ! [ info exist c%d ] } { set c%d white }", init_color, init_color  );
	}
			
	if ( init_color == 1001 )
		cmd( "canvas .lat.c -height %d -width %d -bg white", ( unsigned int ) pixH, ( unsigned int ) pixW );
	else
		cmd( "canvas .lat.c -height %d -width %d -bg %s", ( unsigned int ) pixH, ( unsigned int ) pixW, init_color_string );

	cmd( "pack .lat.c" );

	cmd( "save .lat b { set b \"%s.eps\"; set a [ tk_getSaveFile -parent .lat -title \"Save Lattice Image File\" -defaultextension .eps -initialfile $b -initialdir \"%s\" -filetypes { { {Encapsulated Postscript files} {.eps} } { {All files} {*} } } ]; if { $a != \"\" } { .lat.c postscript -colormode color -file \"$a\" } }", simul_name, path );

	cmd( "set rows %d", rows );
	cmd( "set columns %d", columns );
	cmd( "set dimH %.6g", dimH );
	cmd( "set dimW %.6g", dimW );
	
	cmd( "for { set i 1 } { $i <= $rows } { incr i } { \
			for { set j 1 } { $j <= $columns } { incr j } { \
				set x1 [ expr ( $j - 1 ) * $dimW ]; \
				set y1 [ expr ( $i - 1 ) * $dimH ]; \
				set x2 [ expr $j * $dimW ]; \
				set y2 [ expr $i * $dimH ]; \
				.lat.c create rectangle $x1 $y1 $x2 $y2 -outline \"\" -tags c${i}_${j} \
			} \
		}" );

	cmd( "showtop .lat centerS no no no" );
	set_shortcuts_log( ".lat", "lattice.html" );

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
	
#ifndef NO_WINDOW
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
#ifndef NO_WINDOW

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
		// create (white) pallete entry if invalid palette in val
		cmd( "if { ! [ info exist c%d ] } { set c%d white }", val_int, val_int  );
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

#ifndef NO_WINDOW

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
STR_UPR
function may be missing in some compiler libraries
****************************************************/
char *str_upr( char *str )
{
	unsigned char *p = ( unsigned char * ) str;

	while ( *p )
	{
		*p = toupper( *p );
		++p;
	}

	return str;
}

/****************************************************
C++11 Standard Library <random> functions
If C++11 or higher is not available, use the old
LSD functions
****************************************************/

#ifdef CPP11

/****************************************************
INIT_RANDOM
Set seed to all random generators
Pseudo-random number generator to extract draws
ran_gen =	1 : Linear congruential in (0,1)
ran_gen =	2 : Mersenne-Twister in (0,1)
ran_gen =	3 : Linear congruential in [0,1)
ran_gen =	4 : Mersenne-Twister in [0,1)
ran_gen =	5 : Mersenne-Twister with 64 bits resolution in [0,1)
ran_gen =	6 : Lagged fibonacci with 24 bits resolution in [0,1)
ran_gen =	7 : Lagged fibonacci with 48 bits resolution in [0,1)
****************************************************/
int ran_gen = 2;					// default pseudo-random number generator

#ifdef PARALLEL_MODE
mutex parallel_lc1;					// mutex locks for random generator operations
mutex parallel_lc2;
mutex parallel_mt32;
mutex parallel_mt64;
mutex parallel_lf24;
mutex parallel_lf48;
#endif	

minstd_rand lc1;					// linear congruential generator (internal)
minstd_rand lc2;					// linear congruential generator (user)
mt19937 mt32;						// Mersenne-Twister 32 bits generator
mt19937_64 mt64;					// Mersenne-Twister 64 bits generator
ranlux24 lf24;						// lagged fibonacci 24 bits generator
ranlux48 lf48;						// lagged fibonacci 48 bits generator

void init_random( unsigned seed )
{
	lc1.seed( seed );				// linear congruential (internal)
	lc2.seed( seed );				// linear congruential (user)
	mt32.seed( seed );				// Mersenne-Twister 32 bits
	mt64.seed( seed );				// Mersenne-Twister 64 bits
	lf24.seed( seed );				// lagged fibonacci 24 bits
	lf48.seed( seed );				// lagged fibonacci 48 bits
}

template < class distr > double draw_lc1( distr &d )
{
#ifdef PARALLEL_MODE
	// prevent concurrent draw by more than one thread
	lock_guard < mutex > lock( parallel_lc1 );
#endif	
	return d( lc1 );
}

template < class distr > double draw_lc2( distr &d )
{
#ifdef PARALLEL_MODE
	// prevent concurrent draw by more than one thread
	lock_guard < mutex > lock( parallel_lc2 );
#endif	
	return d( lc2 );
}

template < class distr > double draw_mt32( distr &d )
{
#ifdef PARALLEL_MODE
	// prevent concurrent draw by more than one thread
	lock_guard < mutex > lock( parallel_mt32 );
#endif	
	return d( mt32 );
}

template < class distr > double draw_mt64( distr &d )
{
#ifdef PARALLEL_MODE
	// prevent concurrent draw by more than one thread
	lock_guard < mutex > lock( parallel_mt64 );
#endif	
	return d( mt64 );
}

template < class distr > double draw_lf24( distr &d )
{
#ifdef PARALLEL_MODE
	// prevent concurrent draw by more than one thread
	lock_guard < mutex > lock( parallel_lf24 );
#endif	
	return d( lf24 );
}

template < class distr > double draw_lf48( distr &d )
{
#ifdef PARALLEL_MODE
	// prevent concurrent draw by more than one thread
	lock_guard < mutex > lock( parallel_lf48 );
#endif	
	return d( lf48 );
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
CUR_GEN
Generate the draw using current generator object
***************************************************/
template < class distr > double cur_gen( distr &d )
{
	switch ( ran_gen )
	{
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
RAN1
Call the preset pseudo-random number generator
***************************************************/
double ran1( long *unused )
{
	double ran;
	uniform_real_distribution< double > distr( 0, 1 );
	
	do
		ran = cur_gen( distr );
	while ( ran_gen < 3 && ran == 0.0 );

	return ran;
}


/****************************************************
UNIFORM
****************************************************/
double uniform( double min, double max )
{
	uniform_real_distribution< double > distr( min, max );
	return cur_gen( distr );
}


/****************************************************
UNIFORM_INT
****************************************************/
double uniform_int( double min, double max )
{
	uniform_int_distribution< int > distr( ( long ) min, ( long ) max );
	return cur_gen( distr );
}


/***************************************************
NORM
***************************************************/
double norm( double mean, double dev )
{
	static bool normStopErr;
	
	if ( dev < 0 )	
	{
		if ( ++normErrCnt < ERR_LIM )	// prevent slow down due to I/O
		{
			plog( "\nWarning: negative standard deviation in function 'norm'" );
			normStopErr = false;
		}
		else
			if ( ! normStopErr )
			{
				plog( "\nWarning: too many negative standard deviation errors in function 'norm', stop reporting...\n" );
				normStopErr = true;
			}

		return mean;
	}

	normal_distribution< double > distr( mean, dev );
	return cur_gen( distr );
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
		if ( ++lnormErrCnt < ERR_LIM )	// prevent slow down due to I/O
		{
			plog( "\nWarning: negative standard deviation in function 'lnorm'" );
			lnormStopErr = false;
		}
		else
			if ( ! lnormStopErr )
			{
				plog( "\nWarning: too many negative standard deviation errors in function 'lnorm', stop reporting...\n" );
				lnormStopErr = true;
			}
			
		return exp( mean );
	}

	lognormal_distribution< double > distr( mean, dev );
	return cur_gen( distr );
}


/****************************************************
GAMMA
****************************************************/
double gamma( double alpha, double beta )
{
	static bool gammaStopErr;
	
	if ( alpha <= 0 || beta <= 0 )
	{
		if ( ++gammaErrCnt < ERR_LIM )	// prevent slow down due to I/O
		{
			plog( "\nWarning: non-positive alpha or beta parameters in function 'gamma'" );
			gammaStopErr = false;
		}
		else
			if ( ! gammaStopErr )
			{
				plog( "\nWarning: too many non-positive parameter errors in function 'gamma', stop reporting...\n" );
				gammaStopErr = true;
			}
			
		return 0.0;
	}

	gamma_distribution< double > distr( alpha, beta );
	return cur_gen( distr );
}


/****************************************************
BERNOULLI
****************************************************/
double bernoulli( double p )
{
	static bool bernoStopErr;
	
	if ( p < 0 || p > 1 )
	{
		if ( ++bernoErrCnt < ERR_LIM )	// prevent slow down due to I/O
		{
			plog( "\nWarning: probability out of [0, 1] in function 'bernoulli'" );
			bernoStopErr = false;
		}
		else
			if ( ! bernoStopErr )
			{
				plog( "\nWarning: too many invalid probability errors in function 'bernoulli', stop reporting...\n" );
				bernoStopErr = true;
			}
			
		if ( p < 0 )
			return 0.0;
		else
			return 1.0;
	}

	bernoulli_distribution distr( p );
	return cur_gen( distr );
}


/****************************************************
POISSON
****************************************************/
double poisson( double mean )
{
	static bool poissStopErr;
	
	if ( mean < 0 )
	{
		if ( ++poissErrCnt < ERR_LIM )	// prevent slow down due to I/O
		{
			plog( "\nWarning: negative mean in function 'poisson'" );
			poissStopErr = false;
		}
		else
			if ( ! poissStopErr )
			{
				plog( "\nWarning: too many negative mean errors in function 'poisson', stop reporting...\n" );
				poissStopErr = true;
			}
			
		return 0.0;
	}

	poisson_distribution< int > distr( mean );
	return cur_gen( distr );
}


/****************************************************
GEOMETRIC
****************************************************/
double geometric( double p )
{
	static bool geomStopErr;
	
	if ( p < 0 || p > 1 )
	{
		if ( ++geomErrCnt < ERR_LIM )	// prevent slow down due to I/O
		{
			plog( "\nWarning: probability out of [0, 1] in function 'geometric'" );
			geomStopErr = false;
		}
		else
			if ( ! geomStopErr )
			{
				plog( "\nWarning: too many invalid probability errors in function 'geometric', stop reporting...\n" );
				geomStopErr = true;
			}
			
		if ( p < 0 )
			return 0.0;
		else
			return 1.0;
	}

	geometric_distribution< int > distr( p );
	return cur_gen( distr );
}


/****************************************************
BINOMIAL
****************************************************/
double binomial( double p, double t )
{
	static bool binomStopErr;
	
	if ( p < 0 || p > 1 || t <= 0 )
	{
		if ( ++binomErrCnt < ERR_LIM )	// prevent slow down due to I/O
		{
			plog( "\nWarning: invalid parameters in function 'binomial'" );
			binomStopErr = false;
		}
		else
			if ( ! binomStopErr )
			{
				plog( "\nWarning: too many invalid parameter errors in function 'binomial', stop reporting...\n" );
				binomStopErr = true;
			}
			
		if ( p < 0 || t <= 0 )
			return 0.0;
		else
			return 1.0;
	}

	binomial_distribution< int > distr( t, p );
	return cur_gen( distr );
}


/***************************************************
CAUCHY
***************************************************/
double cauchy( double a, double b )
{
	static bool cauchStopErr;
	
	if ( b <= 0 )
	{
		if ( ++cauchErrCnt < ERR_LIM )	// prevent slow down due to I/O
		{
			plog( "\nWarning: non-positive b parameter in function 'cauchy'" );
			cauchStopErr = false;
		}
		else
			if ( ! cauchStopErr )
			{
				plog( "\nWarning: too many non-positive parameter errors in function 'cauchy', stop reporting...\n" );
				cauchStopErr = true;
			}
			
		return a;
	}

	cauchy_distribution< double > distr( a, b );
	return cur_gen( distr );
}


/***************************************************
CHI_SQUARED
***************************************************/
double chi_squared( double n )
{
	static bool chisqStopErr;
	
	if ( n <= 0 )
	{
		if ( ++chisqErrCnt < ERR_LIM )	// prevent slow down due to I/O
		{
			plog( "\nWarning: non-positive n parameter in function 'chi_squared'" );
			chisqStopErr = false;
		}
		else
			if ( ! chisqStopErr )
			{
				plog( "\nWarning: too many non-positive parameter errors in function 'chi_squared', stop reporting...\n" );
				chisqStopErr = true;
			}
			
		return 0.0;
	}

	chi_squared_distribution< double > distr( n );
	return cur_gen( distr );
}


/***************************************************
EXPONENTIAL
***************************************************/
double exponential( double lambda )
{
	static bool expStopErr;
	
	if ( lambda <= 0 )
	{
		if ( ++expErrCnt < ERR_LIM )	// prevent slow down due to I/O
		{
			plog( "\nWarning: non-positive lambda parameter in function 'exponential'" );
			expStopErr = false;
		}
		else
			if ( ! expStopErr )
			{
				plog( "\nWarning: too many non-positive parameter errors in function 'exponential', stop reporting...\n" );
				expStopErr = true;
			}
			
		return 0.0;
	}

	exponential_distribution< double > distr( lambda );
	return cur_gen( distr );
}


/***************************************************
FISHER
***************************************************/
double fisher( double m, double n )
{
	static bool fishStopErr;
	
	if ( m <= 0 || n <= 0 )
	{
		if ( ++fishErrCnt < ERR_LIM )	// prevent slow down due to I/O
		{
			plog( "\nWarning: invalid parameters in function 'fisher'" );
			fishStopErr = false;
		}
		else
			if ( ! fishStopErr )
			{
				plog( "\nWarning: too many invalid parameter errors in function 'fisher', stop reporting...\n" );
				fishStopErr = true;
			}
			
		return 0.0;
	}

	fisher_f_distribution< double > distr( m, n );
	return cur_gen( distr );
}


/***************************************************
STUDENT
***************************************************/
double student( double n )
{
	static bool studStopErr;
	
	if ( n <= 0 )
	{
		if ( ++studErrCnt < ERR_LIM )	// prevent slow down due to I/O
		{
			plog( "\nWarning: non-positive n parameter in function 'student'" );
			studStopErr = false;
		}
		else
			if ( ! studStopErr )
			{
				plog( "\nWarning: too many non-positive parameter errors in function 'student', stop reporting...\n" );
				studStopErr = true;
			}
			
		return 0.0;
	}

	student_t_distribution< double > distr( n );
	return cur_gen( distr );
}


/***************************************************
WEIBULL
***************************************************/
double weibull( double a, double b )
{
	static bool weibStopErr;
	
	if ( a <= 0 || b <= 0 )
	{
		if ( ++weibErrCnt < ERR_LIM )	// prevent slow down due to I/O
		{
			plog( "\nWarning: non-positive a or b parameters in function 'weibull'" );
			weibStopErr = false;
		}
		else
			if ( ! weibStopErr )
			{
				plog( "\nWarning: too many non-positive parameter errors in function 'weibull', stop reporting...\n" );
				weibStopErr = true;
			}
			
		return 0.0;
	}

	weibull_distribution< double > distr( a, b );
	return cur_gen( distr );
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
		if ( ++betaErrCnt < ERR_LIM )	// prevent slow down due to I/O
		{
			plog( "\nWarning: non-positive alpha or beta parameters in function 'beta'" );
			betaStopErr = false;
		}
		else
			if ( ! betaStopErr )
			{
				plog( "\nWarning: too many non-positive parameter errors in function 'beta', stop reporting...\n" );
				betaStopErr = true;
			}
			
		if ( alpha < beta )
			return 0.0;
		else
			return 1.0;
	}

	gamma_distribution< double > distr1( alpha, 1.0 ), distr2( beta, 1.0 );
	double draw = cur_gen( distr1 );
	return draw / ( draw + cur_gen( distr2 ) );
}

#else

/***************************************************
PMRAND_OPEN
Park-Miller pseudo-random number generator with Bays-Durham shuffling
and Press et al. (1992) protections with period 2^31 - 1 in (0,1)
***************************************************/
#define IA 16807
#define IM 2147483647
#define AM (1.0/IM)
#define IQ 127773
#define IR 2836
#define NTAB 32
#define NDIV (1+(IM-1)/NTAB)
#define EPS 1.2e-7
#define RNMX (1.0-EPS)

double PMRand_open( long *idum )
{
	int j;
	long k;
	static long iy = 0, iv[ NTAB ];
	double temp;

	if ( *idum <= 0 || ! iy ) 
	{
		if ( - ( *idum ) < 1 ) 
			*idum = 1;
		else 
			*idum = - ( *idum );
		for ( j = NTAB + 7; j >= 0; --j ) 
		{
			k = ( *idum ) / IQ;
			*idum = IA * ( *idum - k * IQ ) - IR * k;
			if ( *idum < 0 ) 
				*idum += IM;
			if ( j < NTAB ) 
				iv[ j ] = *idum;
		}
		
		iy = iv[ 0 ];
	}
	
	k = ( *idum ) / IQ;
	*idum = IA * ( *idum - k * IQ ) - IR * k;
	if ( *idum < 0 ) 
		*idum += IM;
	
	j = iy / NDIV;
	iy = iv[ j ];
	iv[ j ] = *idum;
	if ( ( temp = AM * iy ) > RNMX ) 
		return RNMX;
	else 
		return temp;
}
   

/***************************************************
MTRAND
Mersenne Twister pseudo-random number generator
with period 2^19937 - 1 with improved initialization scheme,
modified on 2002/1/26 by Takuji Nishimura and Makoto Matsumoto.
The generators returning floating point numbers are based on
a version by Isaku Wada, 2002/01/09
This version is a port from the original C-code to C++ by
Jasper Bedaux (http://www.bedaux.net/mtrand/).
Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
3. The names of its contributors may not be used to endorse or promote
products derived from this software without specific prior written
permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***************************************************/

class MTRand_int32 { // Mersenne Twister random number generator
public:
// default constructor: uses default seed only if this is the first instance
  MTRand_int32() { if (!init) seed(5489UL); init = true; }
// constructor with 32 bit int as seed
  MTRand_int32(unsigned long s) { seed(s); init = true; }
// constructor with array of size 32 bit ints as seed
  MTRand_int32(const unsigned long* array, int size) { seed(array, size); init = true; }
// the two seed functions
  void seed(unsigned long); // seed with 32 bit integer
  void seed(const unsigned long*, int size); // seed with array
// overload operator() to make this a generator (functor)
  unsigned long operator()() { return rand_int32(); }
// 2007-02-11: made the destructor virtual; thanks "double more" for pointing this out
  virtual ~MTRand_int32() {} // destructor
protected: // used by derived classes, otherwise not accessible; use the ()-operator
  unsigned long rand_int32(); // generate 32 bit random integer
private:
  static const int n = 624, m = 397; // compile time constants
// the variables below are static (no duplicates can exist)
  static unsigned long state[n]; // state vector array
  static int p; // position in state array
  static bool init; // true if init function is called
// private functions used to generate the pseudo random numbers
  unsigned long twiddle(unsigned long, unsigned long); // used by gen_state()
  void gen_state(); // generate new state
// make copy constructor and assignment operator unavailable, they don't make sense
  MTRand_int32(const MTRand_int32&); // copy constructor not defined
  void operator=(const MTRand_int32&); // assignment operator not defined
};

// inline for speed, must therefore reside in header file
inline unsigned long MTRand_int32::twiddle(unsigned long u, unsigned long v) {
  return (((u & 0x80000000UL) | (v & 0x7FFFFFFFUL)) >> 1)
    ^ ((v & 1UL) * 0x9908B0DFUL);
// 2013-07-22: line above modified for performance according to http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/Ierymenko.html
// thanks Vitaliy FEOKTISTOV for pointing this out
}

inline unsigned long MTRand_int32::rand_int32() { // generate 32 bit random int
  if (p == n) gen_state(); // new state vector needed
// gen_state() is split off to be non-inline, because it is only called once
// in every 624 calls and otherwise irand() would become too big to get inlined
  unsigned long x = state[p++];
  x ^= (x >> 11);
  x ^= (x << 7) & 0x9D2C5680UL;
  x ^= (x << 15) & 0xEFC60000UL;
  return x ^ (x >> 18);
}

// generates double floating point numbers in the half-open interval [0, 1)
class MTRand : public MTRand_int32 {
public:
  MTRand() : MTRand_int32() {}
  MTRand(unsigned long seed) : MTRand_int32(seed) {}
  MTRand(const unsigned long* seed, int size) : MTRand_int32(seed, size) {}
  ~MTRand() {}
  double operator()() {
    return static_cast<double>(rand_int32()) * (1. / 4294967296.); } // divided by 2^32
private:
  MTRand(const MTRand&); // copy constructor not defined
  void operator=(const MTRand&); // assignment operator not defined
};

// generates double floating point numbers in the closed interval [0, 1]
class MTRand_closed : public MTRand_int32 {
public:
  MTRand_closed() : MTRand_int32() {}
  MTRand_closed(unsigned long seed) : MTRand_int32(seed) {}
  MTRand_closed(const unsigned long* seed, int size) : MTRand_int32(seed, size) {}
  ~MTRand_closed() {}
  double operator()() {
    return static_cast<double>(rand_int32()) * (1. / 4294967295.); } // divided by 2^32 - 1
private:
  MTRand_closed(const MTRand_closed&); // copy constructor not defined
  void operator=(const MTRand_closed&); // assignment operator not defined
};

// generates double floating point numbers in the open interval (0, 1)
class MTRand_open : public MTRand_int32 {
public:
  MTRand_open() : MTRand_int32() {}
  MTRand_open(unsigned long seed) : MTRand_int32(seed) {}
  MTRand_open(const unsigned long* seed, int size) : MTRand_int32(seed, size) {}
  ~MTRand_open() {}
  double operator()() {
    return (static_cast<double>(rand_int32()) + .5) * (1. / 4294967296.); } // divided by 2^32
private:
  MTRand_open(const MTRand_open&); // copy constructor not defined
  void operator=(const MTRand_open&); // assignment operator not defined
};

// generates 53 bit resolution doubles in the half-open interval [0, 1)
class MTRand53 : public MTRand_int32 {
public:
  MTRand53() : MTRand_int32() {}
  MTRand53(unsigned long seed) : MTRand_int32(seed) {}
  MTRand53(const unsigned long* seed, int size) : MTRand_int32(seed, size) {}
  ~MTRand53() {}
  double operator()() {
    return (static_cast<double>(rand_int32() >> 5) * 67108864. + 
      static_cast<double>(rand_int32() >> 6)) * (1. / 9007199254740992.); }
private:
  MTRand53(const MTRand53&); // copy constructor not defined
  void operator=(const MTRand53&); // assignment operator not defined
};

// initialization of static private members
unsigned long MTRand_int32::state[n] = {0x0UL};
int MTRand_int32::p = 0;
bool MTRand_int32::init = false;

void MTRand_int32::gen_state() { // generate new state vector
  for (int i = 0; i < (n - m); ++i)
    state[ i ] = state[i + m] ^ twiddle(state[ i ], state[i + 1]);
  for (int i = n - m; i < (n - 1); ++i)
    state[ i ] = state[i + m - n] ^ twiddle(state[ i ], state[i + 1]);
  state[n - 1] = state[m - 1] ^ twiddle(state[n - 1], state[ 0 ]);
  p = 0; // reset position
}

void MTRand_int32::seed(unsigned long s) {  // init by 32 bit seed
  state[ 0 ] = s & 0xFFFFFFFFUL; // for > 32 bit machines
  for (int i = 1; i < n; ++i) {
    state[ i ] = 1812433253UL * (state[i - 1] ^ (state[i - 1] >> 30)) + i;
// see Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier
// in the previous versions, MSBs of the seed affect only MSBs of the array state
// 2002/01/09 modified by Makoto Matsumoto
    state[ i ] &= 0xFFFFFFFFUL; // for > 32 bit machines
  }
  p = n; // force gen_state() to be called for next random number
}

void MTRand_int32::seed(const unsigned long* array, int size) { // init by array
  seed(19650218UL);
  int i = 1, j = 0;
  for (int k = ((n > size) ? n : size); k; --k) {
    state[ i ] = (state[ i ] ^ ((state[i - 1] ^ (state[i - 1] >> 30)) * 1664525UL))
      + array[ j ] + j; // non linear
    state[ i ] &= 0xFFFFFFFFUL; // for > 32 bit machines
    ++j; j %= size;
    if ((++i) == n) { state[ 0 ] = state[n - 1]; i = 1; }
  }
  for (int k = n - 1; k; --k) {
    state[ i ] = (state[ i ] ^ ((state[i - 1] ^ (state[i - 1] >> 30)) * 1566083941UL)) - i;
    state[ i ] &= 0xFFFFFFFFUL; // for > 32 bit machines
    if ((++i) == n) { state[ 0 ] = state[n - 1]; i = 1; }
  }
  state[ 0 ] = 0x80000000UL; // MSB is 1; assuring non-zero initial array
  p = n; // force gen_state() to be called for next random number
}


/****************************************************
INIT_RANDOM
Pseudo-random number generator to extract draws
ran_gen =	1 : Park-Miller in (0,1)
ran_gen =	2 : Mersenne-Twister in (0,1)
ran_gen =	3 : Mersenne-Twister in [0,1]
ran_gen =	4 : Mersenne-Twister in [0,1)
ran_gen =	5 : Mersenne-Twister with 53 bits resolution in [0,1)
****************************************************/
int ran_gen = 1;	// default pseudo-random number generator
int dum = 0;
long idum = 0;		// Park-Miller sequential control (default seed)

MTRand_open mt2;	// Mersenne-Twister object in (0,1)
MTRand_closed mt3;	// Mersenne-Twister object in [0,1]
MTRand mt4; 		// Mersenne-Twister object in [0,1)
MTRand53 mt5;		// Mersenne-Twister object with 53 bits resolution in [0,1)

// Set seed to all random generators
void init_random( unsigned seed )
{
	dum = 0;
	idum = -seed;
	PMRand_open( &idum );	// PM in (0,1)
	mt2.seed( seed );		// MT in (0,1)
	mt3.seed( seed );		// MT in [0,1]
	mt4.seed( seed );		// MT in [0,1)
	mt5.seed( seed );		// MT with 53 bits resolution in [0,1)
}


/***************************************************
RAN1
Call the preset pseudo-random number generator
***************************************************/
double ran1( long *idum_loc )
{
	// Manage default (internal) number sequence
	// ONLY FOR PARK-MILER generator
	if ( idum_loc == NULL )
		idum_loc = & idum;

	switch ( ran_gen )
	{
		case 2:
			return mt2( );			// in (0,1)
			break;
		case 3:
			return mt3( );			// in [0,1]
			break;
		case 4:
			return mt4( );			// in [0,1)
			break;
		case 5:
			return mt5( );			// 53 bits resolution in [0,1]
			break;
		case 1:
		default:
			return PMRand_open( idum_loc );
	}
}


/****************************************************
RND_INT
****************************************************/
int rnd_int( int min, int max )
{
	return ( floor( min + ran1( ) * ( max + 1 - min ) ) );
}


/****************************************************
UNIFORM
****************************************************/
double uniform( double min, double max )
{
	if ( min > max )
		return 0;
	return ( min + ran1( ) * ( max - min ) );
}


/****************************************************
UNIFORM_INT
****************************************************/
double uniform_int( double min, double max )
{
	if ( min > max )
		return 0;
	return ( floor( min + ran1( ) * ( max + 1 - min ) ) );
}


/***************************************************
NORM
***************************************************/
double norm( double mean, double dev )
{
	static bool normStopErr;
	double gasdev, R, v1, v2, fac;
	static double gset;
	int boh = 1;
	
	if ( dev < 0 )
	{
		if ( ++normErrCnt < ERR_LIM )	// prevent slow down due to I/O
		{
			plog( "\nWarning: negative standard deviation in function 'norm'" );
			normStopErr = false;
		}
		else
			if ( ! normStopErr )
			{
				plog( "\nWarning: too many negative standard deviation errors in function 'norm', stop reporting...\n" );
				normStopErr = true;
			}

		return mean;
	}

	if ( dum == 0 )
	{ 
		do
		{ 
			v1 = 2.0 * ran1( ) - 1;
			boh = 1;
			v2 = 2.0 * ran1( ) - 1;
			R = v1 * v1 + v2 * v2;
		}
		while ( R >= 1 );
		
		fac = log( R );
		fac = fac / R;
		fac = fac * ( -2.0 );
		fac = sqrt( fac );
		gset = v1 * fac;
		gasdev = v2 * fac;
		dum = 1;
	}
	else
	{
		gasdev = gset;
		dum = 0;
	}
	gasdev = gasdev * dev + mean;
	
	return gasdev;
}


/***************************************************
LNORM
Return a draw from a lognormal distribution
***************************************************/
double lnorm( double mean, double dev )
{
	static bool lnormStopErr;
	
	if ( dev <= 0 )
	{
		if ( ++lnormErrCnt < ERR_LIM )	// prevent slow down due to I/O
		{
			plog( "\nWarning: negative standard deviation in function 'lnorm'" );
			lnormStopErr = false;
		}
		else
			if ( ! lnormStopErr )
			{
				plog( "\nWarning: too many negative standard deviation errors in function 'lnorm', stop reporting...\n" );
				lnormStopErr = true;
			}
			
		return exp( mean );
	}

	return exp( norm( mean, dev ) );
}


/****************************************************
GAMMA
****************************************************/
extern int quit;

// support function to gamma
double gamdev( int ia, long *idum_loc = NULL )
{
	int j;
	double am, e, s, v1, v2, x, y;

	// Manage default (internal) number sequence
	// WORKS ONLY FOR PARK-MILER generator
	if ( idum_loc == NULL )
		idum_loc = & idum;

	if ( ia < 1 ) 
	{
		error_hard( "inconsistent state in gamma function",
					"internal problem in LSD", 
					"if error persists, please contact developers", 
					true );
		quit = 1;
		return 0;
	} 
	if ( ia < 6 )
	{
		x = 1.0;
		for ( j = 1; j <= ia; ++j )
			x *= ran1( idum_loc );
		x = - log( x );
	}
	else
	{
		do
		{
			do
			{
				do
				{ 
					v1 = ran1( idum_loc );
					v2 = 2 * ran1( idum_loc ) - 1.0;
				}
				while ( v1 * v1 + v2 * v2 > 1.0 );
				
				y = v2 / v1;
				am = ia - 1;
				s = sqrt( 2.0 * am + 1.0 );
				x = s * y + am;
			}
			while ( x <= 0 );
			e = ( 1 + y * y ) * exp( am * log( x / am ) - s * y );
		}
		while ( ran1( idum_loc ) > e );
	} 
	return x;
}

double gamma( double alpha, double beta )
{
	static bool gammaStopErr;
	
	if ( alpha <= 0 || beta <= 0 )
	{
		if ( ++gammaErrCnt < ERR_LIM )	// prevent slow down due to I/O
		{
			plog( "\nWarning: non-positive alpha or beta parameters in function 'gamma'" );
			gammaStopErr = false;
		}
		else
			if ( ! gammaStopErr )
			{
				plog( "\nWarning: too many non-positive parameter errors in function 'gamma', stop reporting...\n" );
				gammaStopErr = true;
			}
			
		return 0.0;
	}

	return gamdev( ( int ) round( alpha ), & idum );
}


/****************************************************
BERNOULLI
****************************************************/
double bernoulli( double p )
{
	static bool bernoStopErr;
	
	if ( p < 0 || p > 1 )
	{
		if ( ++bernoErrCnt < ERR_LIM )	// prevent slow down due to I/O
		{
			plog( "\nWarning: probability out of [0, 1] in function 'bernoulli'" );
			bernoStopErr = false;
		}
		else
			if ( ! bernoStopErr )
			{
				plog( "\nWarning: too many invalid probability errors in function 'bernoulli', stop reporting...\n" );
				bernoStopErr = true;
			}
			
		if ( p < 0 )
			return 0.0;
		else
			return 1.0;
	}

	double dice = ran1( );
	if ( dice < p )
		return 1;
	return 0;
}


/****************************************************
POISSON
****************************************************/

// support function to poidev
double gammaln( double xx )
{
	double x, y, tmp, ser;
	static double cof[ 6 ]={ 76.18009172947146, -86.50532032941677, 24.01409824083091, -1.231739572450155, 0.1208650973866179e-2, -0.5395239384953e-5};
	int j;
	
	y = x = xx;
	tmp = x + 5.5;
	tmp -= ( x + 0.5 ) * log( tmp );
	ser = 1.000000000190015;
	
	for ( j = 0; j <= 5; ++j ) 
		ser += cof[ j ] / ++y;
	
	return -tmp + log( 2.5066282746310005 * ser / x );
}

// support function to poisson
double poidev( double xm, long *idum_loc = NULL )
{
	double em, t, y;
	static double sq , alxm, g, oldm = ( -1.0 );

	// Manage default (internal) number sequence
	// WORKS ONLY FOR PARK-MILER generator
	if ( idum_loc == NULL )
		idum_loc = & idum;

	if ( xm < 12.0 ) 
	{
		if ( xm != oldm ) 
		{
			oldm = xm;
			g = exp( - xm );
		}
		
		em = -1;
		t = 1.0;
		
		do 
		{
			++em;
			t *= ran1( idum_loc );
		} 
		while ( t > g );
	} 
	else 
	{
		if ( xm != oldm ) 
		{
			oldm = xm;
			sq = sqrt( 2.0 * xm );
			alxm = log( xm );
			g = xm * alxm - gammaln( xm + 1.0 );
		}
		do 
		{
			do 
			{
				y = tan( M_PI * ran1( idum_loc ) );
				em = sq * y + xm;
			} 
			while ( em < 0.0 );   
			
			em = floor( em );
			t = 0.9 * ( 1.0 + y * y ) * exp( em * alxm - gammaln( em + 1.0 ) - g );
		} 
		while ( ran1( idum_loc ) > t );
	}
	return em;
}

double poisson( double mean )
{
	static bool poissStopErr;
	
	if ( mean < 0 )
	{
		if ( ++poissErrCnt < ERR_LIM )	// prevent slow down due to I/O
		{
			plog( "\nWarning: negative mean in function 'poisson'" );
			poissStopErr = false;
		}
		else
			if ( ! poissStopErr )
			{
				plog( "\nWarning: too many negative mean errors in function 'poisson', stop reporting...\n" );
				poissStopErr = true;
			}
			
		return 0.0;
	}

	return poidev( mean, & idum );
}


/***************************************************
BETA
Return a draw from a Beta(alfa,beta) distribution
Dosi et al. (2010) K+S
***************************************************/
double beta( double alpha, double beta )
{
	static bool betaStopErr;
	
	if ( alpha <= 0 || beta <= 0 )
	{
		if ( ++betaErrCnt < ERR_LIM )	// prevent slow down due to I/O
		{
			plog( "\nWarning: non-positive alpha or beta parameters in function 'beta'" );
			betaStopErr = false;
		}
		else
			if ( ! betaStopErr )
			{
				plog( "\nWarning: too many non-positive parameter errors in function 'beta', stop reporting...\n" );
				betaStopErr = true;
			}
			
		if ( alpha < beta )
			return 0.0;
		else
			return 1.0;
	}

	double U, V, den;
	U = ran1( );
	V = ran1( ); 
	den = pow( U, ( 1 / alpha ) ) + pow( V, ( 1 / beta ) );
	
	while ( den <= 0 || den > 1)
	{
		U = ran1( );
		V = ran1( );
		den = pow( U,( 1 / alpha ) ) + pow( V, ( 1 / beta ) );
	}

	return pow( U , ( 1 / alpha ) ) / den;
}

#endif


/****************************************************
PARETO
****************************************************/
double pareto( double mu, double alpha )
{
	static bool paretStopErr;
	
	if ( mu <= 0 || alpha <= 0 )
	{
		if ( ++paretErrCnt < ERR_LIM )	// prevent slow down due to I/O
		{
			plog( "\nWarning: non-positive mu or alpha parameters in function 'pareto'" );
			paretStopErr = false;
		}
		else
			if ( ! paretStopErr )
			{
				plog( "\nWarning: too many non-positive parameter errors in function 'pareto', stop reporting...\n" );
				paretStopErr = true;
			}
			
		return mu;
	}

	return mu / pow( 1 - ran1( ), 1 / alpha );
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
		if ( ++alaplErrCnt < ERR_LIM )	// prevent slow down due to I/O
		{
			plog( "\nWarning: non-positive alpha1 or alpha2 parameters in function 'alapl'" );
			alaplStopErr = false;
		}
		else
			if ( ! alaplStopErr )
			{
				plog( "\nWarning: too many non-positive parameter errors in function 'alapl', stop reporting...\n" );
				alaplStopErr = true;
			}
			
		return mu;
	}

	double draw = ran1( );
	if ( draw < ( alpha1 / ( alpha1 + alpha2 ) ) )
		return mu + alpha1 * log( draw * ( 1 + alpha1 / alpha2 ) );
	else 
		return mu - alpha2 * log( ( 1 - draw ) * ( 1 + alpha1 / alpha2 ) );
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
