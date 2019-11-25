/*************************************************************

	LSD 7.2 - December 2019
	written by Marco Valente, Universita' dell'Aquila
	and by Marcelo Pereira, University of Campinas

	Copyright Marco Valente and Marcelo Pereira
	LSD is distributed under the GNU General Public License
	
 *************************************************************/

/*************************************************************
MAIN.CPP 
LSD C++ system entry/exit point.

Sets all OS signal handlers.
*************************************************************/

#include "decl.h"

/*************************************
 MAIN
 *************************************/
int main( int argn, char **argv )
{
	int res = 0;

#ifndef NO_ERROR_TRAP
	// register all signal handlers
	handle_signals( signal_handler );

	try
	{
#endif

		res = lsdmain( argn, argv );
		
#ifndef NO_ERROR_TRAP
	}
	catch ( bad_alloc& )	// out of memory conditions
	{
		signal_handler( SIGMEM );
	}
	catch ( exception& exc )// other known error conditions
	{
		sprintf( msg, "\nSTL exception of type: %s\n", exc.what( ) );
		signal_handler( SIGSTL );
	}
	catch ( ... )				// other unknown error conditions
	{
		abort( );				// raises a SIGABRT exception, tell user & close
	}

#ifndef NO_WINDOW	
	if ( inter != NULL )
	{
		cmd( "if { ! [ catch { package present Tk } ] } { destroy . }" );
		cmd( "LsdExit" );
		Tcl_Finalize( );
	}
#endif
#endif

	return res;
}


/*********************************
 MYEXIT
 *********************************/
// exit LSD finishing 
void myexit( int v )
{
	fflush( stderr );
#ifdef PARALLEL_MODE
	// stop multi-thread workers, if needed
	delete [ ] workers;
#endif

#ifndef NO_WINDOW
	if ( inter != NULL )
	{
		cmd( "if { ! [ catch { package present Tk } ] } { destroy . }" );
		cmd( "LsdExit" );
		Tcl_Finalize( );
	}
#endif
	exit( v );
}


/*********************************
 HANDLE_SIGNALS
 *********************************/
// provide support to the old 32-bit gcc compiler
#ifdef SIGSYS
#define NUM_SIG 11
int signals[ NUM_SIG ] = { SIGINT, SIGQUIT, SIGTERM, SIGWINCH, SIGABRT, SIGFPE, SIGILL, SIGSEGV, SIGBUS, SIGSYS, SIGXFSZ };
#else
#define NUM_SIG 6
int signals[ NUM_SIG ] = { SIGINT, SIGTERM, SIGABRT, SIGFPE, SIGILL, SIGSEGV };
const char *signal_names[ NUM_SIG ] = { "", "", "", "Floating-point exception", "Illegal instruction", "Segmentation violation", };

const char *strsignal( int signum ) 
{ 
	int i;
	for ( i = 0; i < NUM_SIG && signals[ i ] != signum; ++i );
	if ( i == NUM_SIG )
		return "Unknow exception";
	return signal_names[ i ];
}
#endif

void handle_signals( void ( * handler )( int signum ) )
{
	for ( int i = 0; i < NUM_SIG; ++i )
		signal( signals[ i ], handler );  
}


/*********************************
 SIGNAL_HANDLER
 *********************************/
// handle critical system signals
void signal_handler( int signum )
{
	char msg2[ MAX_LINE_SIZE ], msg3[ MAX_LINE_SIZE ];
	double useless = -1;
	
	switch ( signum )
	{
#ifdef SIGQUIT
		case SIGQUIT:
#endif
		case SIGINT:
		case SIGTERM:
#ifdef NO_WINDOW
			sprintf( msg, "SIGINT/SIGTERM (%s)", strsignal( signum ) );
			break;
#else
			choice = 1;				// regular quit (checking for save)
			return;
#endif
#ifdef SIGWINCH
		case SIGWINCH:
#ifndef NO_WINDOW
			cmd( "sizetop all" );	// readjust windows size/positions
			cmd( "update" );
#endif
			return;
#endif
		case SIGSTL:
			break;
			
		case SIGMEM:
			sprintf( msg, "SIGMEM (Out of memory)" );
			strcpy( msg2, "Maybe too many series saved?\n  Try to reduce the number of series saved or the number of time steps" );
			break;
			
		case SIGABRT:
			sprintf( msg, "SIGABRT (%s)", strsignal( signum ) );
			strcpy( msg2, "Maybe an invalid call to library or Tcl/Tk?" );		
			break;

		case SIGFPE:
			sprintf( msg, "SIGFPE (%s)", strsignal( signum ) );
			strcpy( msg2, "Maybe a division by 0 or similar?" );
		break;
		
		case SIGILL:
			sprintf( msg, "SIGILL (%s)", strsignal( signum ) );
			strcpy( msg2, "Maybe executing data?" );		
		break;
		
		case SIGSEGV:
			sprintf( msg, "SIGSEGV (%s)", strsignal( signum ) );
			strcpy( msg2, "Maybe an invalid pointer?\n  Also ensure no group of objects has zero elements." );		
		break;
#ifdef SIGBUS
		case SIGBUS:
			sprintf( msg, "SIGBUS (%s)", strsignal( signum ) );
			strcpy( msg2, "Maybe incorrect pointer handling?\n  Also ensure no group of objects has zero elements." );		
			break;
		
		case SIGSYS:
			sprintf( msg, "SIGSYS (%s)", strsignal( signum ) );
			strcpy( msg2, "Maybe a system-specific function?" );		
			break;
		
		case SIGXFSZ:
			sprintf( msg, "SIGXFSZ (%s)", strsignal( signum ) );
			strcpy( msg2, "Maybe a loop while saving data?" );		
			break;
#endif
		default:
			sprintf( msg, "Unknown signal (%s)", strsignal( signum ) );
			strcpy( msg2, "" );			
	}
	
#ifndef NO_WINDOW
	if ( ! user_exception )
	{
		strcpy( msg2, "There is an internal LSD error\n  If error persists, please contact developers" );
		strcpy( msg3, "LSD will close now..." );
	}
	else
	{
		strcpy( msg3, "Additional information may be obtained running the simulation using the 'Model'/'GDB Debugger' menu option" );
		if ( quit != 2 )
		{
			if ( ! parallel_mode && fast_mode == 0 && stacklog != NULL && 
				 stacklog->vs != NULL && stacklog->vs->label != NULL )
			{
				strcat( msg3, "\n\nAttempting to open the LSD Debugger.\n\nLSD will close immediately after exiting the Debugger." );
				plog( "\n\nAn unknown problem was detected while computing the equation \nfor '%s'", "", stacklog->vs->label );
				print_stack( );				
			}
			else
			{
				strcat( msg3, "\n\nPlease disable fast mode and parallel processing to get more information about the error.\n\nLSD will close now." );
				plog( "\n\nAn unknown problem was detected while executing user's equations code" );
				plog( "\n\nWarning: %s active, cannot open LSD Debugger", "", parallel_mode ? "parallel preocessing" : "fast mode" );
			}
				
			quit = 2;
		}
	}
	
	cmd( "tk_messageBox -parent . -title Error -icon error -type ok -message \"FATAL ERROR\" -detail \"System Signal received:\n\n %s:\n  %s\n\n%s\"", msg, msg2, msg3 );
	
	if ( user_exception )
	{
		if ( ! parallel_mode && fast_mode == 0 && stacklog != NULL && 
			 stacklog->vs != NULL && stacklog->vs->label != NULL )
		{
			sprintf( msg3, "%s (ERROR)", stacklog->vs->label );
			deb( stacklog->vs->up, NULL, msg3, &useless );
		}
	}
	else
	{
		sprintf( msg3, "System Signal received: %s", msg );
		log_tcl_error( "FATAL ERROR", msg3 );
	}
#else
	fprintf( stderr, "\nFATAL ERROR: System Signal received: %s\n", msg );
#endif
	myexit( -signum );				// abort program
}
