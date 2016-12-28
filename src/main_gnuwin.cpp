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
	handle_signals( );

	try
	{
		res = lsdmain( argn, argv );
	}
	catch ( std::bad_alloc& )	// out of memory conditions
	{
		signal_handler( SIGMEM );
	}
	catch ( std::exception& exc )// other known error conditions
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
#ifndef NO_WINDOW
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

void handle_signals( void )
{
	for ( int i = 0; i < NUM_SIG; ++i )
		signal( signals[ i ], signal_handler );  
}


/*********************************
 SIGNAL_HANDLER
 *********************************/
// handle critical system signals
void signal_handler(int signum)
{
	char msg2[ MAX_LINE_SIZE ];
	double useless = -1;
	
	switch(signum)
	{
#ifdef SIGQUIT
		case SIGQUIT:
#endif
		case SIGINT:
		case SIGTERM:
			choice = 1;				// regular quit (checking for save)
			return;
#ifdef SIGWINCH
		case SIGWINCH:
			cmd( "sizetop all" );	// readjust windows size/positions
			cmd( "update idletasks" );
			return;
#endif
		case SIGSTL:
			break;
			
		case SIGMEM:
			sprintf( msg, "SIGMEM (Out of memory):\n  Maybe too many series saved?\n  Try to reduce the number of series saved or the number of time steps" );
			break;
			
		case SIGABRT:
			sprintf( msg, "SIGABRT (%s):\n  Maybe an invalid call to library or Tcl/Tk?", strsignal( signum ) );		
			break;

		case SIGFPE:
			sprintf( msg, "SIGFPE (%s):\n  Maybe a division by 0 or similar?", strsignal( signum ) );
		break;
		
		case SIGILL:
			sprintf( msg, "SIGILL (%s):\n  Maybe executing data?", strsignal( signum ) );		
		break;
		
		case SIGSEGV:
			sprintf( msg, "SIGSEGV (%s):\n  Maybe an invalid pointer?\n  Also ensure no group of objects has zero elements.", strsignal( signum ) );		
		break;
#ifdef SIGBUS
		case SIGBUS:
			sprintf( msg, "SIGBUS (%s):\n  Maybe incorrect pointer handling?\n  Also ensure no group of objects has zero elements.", strsignal( signum ) );		
			break;
		
		case SIGSYS:
			sprintf( msg, "SIGSYS (%s):\n  Maybe a system-specific function?", strsignal( signum ) );		
			break;
		
		case SIGXFSZ:
			sprintf( msg, "SIGXFSZ (%s):\n  Maybe a loop while saving data?", strsignal( signum ) );		
			break;
#endif
		default:
			sprintf( msg, strsignal( signum ) );			
	}
	
#ifndef NO_WINDOW
	cmd( "tk_messageBox -parent . -title Error -icon error -type ok -message \"FATAL ERROR\" -detail \"System Signal received:\n\n %s\n\nAdditional information can be obtained running the simulation using the 'Model'/'GDB Debugger' menu option.\n\nAttempting to open the Lsd Debugger (Lsd will close immediately after exiting the Debugger)...\"", msg );
	sprintf( msg2, "Error in equation for '%s'", stacklog->vs->label );
	deb( stacklog->vs->up, NULL, msg2, &useless );
	sprintf( msg2, "System Signal received: %s", msg );
	log_tcl_error( "FATAL ERROR", msg2 );
#else
	fprintf( stderr, "FATAL ERROR: System Signal received:\n %s\n", msg );
#endif
	myexit( -signum );				// abort program
}


