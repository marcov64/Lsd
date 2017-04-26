/***************************************************
****************************************************
LSD 7.0 - August 2015
written by Marco Valente
Universita' dell'Aquila

Copyright Marco Valente
Lsd is distributed according to the GNU Public License

Comments and bug reports to marco.valente@univaq.it
****************************************************
****************************************************/

#include "decl.h"


/*************************************
 MAIN
 *************************************/
int main( int argn, char **argv )
{
	int res;

	// register all signal handlers
	handle_signals( signal_handler );

	try
	{
		res = lsdmain( argn, argv );
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
	Tcl_Exit( res );
#endif
	return res;
}


/*********************************
 MYEXIT
 *********************************/
// exit Lsd finishing 
void myexit( int v )
{
#ifdef PARALLEL_MODE
// stop multi-thread workers, if needed
delete [ ] workers;
#endif

#ifndef NO_WINDOW
	cmd( "LsdExit" );
	Tcl_Exit( v );
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
	
	switch(signum)
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
			cmd( "sizetop all" );	// readjust windows size/positions
			cmd( "update idletasks" );
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
		strcpy( msg2, "There is an internal Lsd error\n  If error persists, please contact developers" );
		strcpy( msg3, "Lsd will close now..." );
	}
	else
	{
		strcpy( msg3, "Additional information can be obtained running the simulation using the 'Model'/'GDB Debugger' menu option" );
		if ( quit != 2 )
		{
			if ( ! parallel_mode && stacklog->vs != NULL )
			{
				strcat( msg3, "\n\nAttempting to open the Lsd Debugger (Lsd will close immediately after exiting the Debugger)..." );
				plog( "\n\nAn unknown problem was detected while computing the equation \nfor '%s'", "", stacklog->vs->label );
				print_stack( );				
			}
			else
				plog( "\n\nAn unknown problem was detected while executing user's equations code" );
				
			quit = 2;
		}
	}
	
	cmd( "tk_messageBox -parent . -title Error -icon error -type ok -message \"FATAL ERROR\" -detail \"System Signal received:\n\n %s:\n  %s\n\n%s\"", msg, msg2, msg3 );
	
	if ( user_exception )
	{
		if ( ! parallel_mode && stacklog->vs != NULL )
		{
			sprintf( msg3, "%s (equation error)", stacklog->vs->label );
			deb( stacklog->vs == NULL ? root : stacklog->vs->up, NULL, msg3, &useless );
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
