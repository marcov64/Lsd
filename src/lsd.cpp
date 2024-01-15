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
LSD.CPP
The LSD program entry point.
Operates one of the two LSD interfaces: GUI or terminal
- GUI: initializes the Tcl/Tk environment and passes control
to the LSD browser
- terminal: run the simulation(s) defined by the supplied
configuration file(s)

This file can be compiled with the command make in the src
directory.

Relevant macros for conditional compilation (when defined):

- _FUN_: user model equation file
- _NW_: No Window executable
- _NP_: no parallel (multi-task) processing
- _NT_: no signal trapping (better when debugging in GDB)

*************************************************************/

#include <cstdlib>
#include <csignal>
#include <exception>
#include <unistd.h>
#include "libLSD.h"

// user defined signals
#define SIGMEM NSIG + 1					// out of memory signal
#define SIGSTL NSIG + 2					// standard library exception signal



/*************************************
 MAIN
 *************************************/
int main( int argn, const char **argv )
{
	char cwd[ PATH_MAX ];
	int res = -1;

#ifndef _NT_
	// register all signal handlers
	handle_signals( signal_handler );

	try
	{

#endif

		// assume exec path is current path
		getcwd( cwd, PATH_MAX );
		set_exec( cwd, argv[ 0 ] );

#ifndef _NW_

		// start of GUI load
		res = load_gui( argv );

#else

		// terminal load
		if ( ( res = load_term_configuration( argn, argv ) ) != 0 )
			lsd_exit( res );

		// simulation execution
		run( );

#endif

#ifndef _NT_
	}
	catch ( std::bad_alloc& exc )	// out of memory conditions
	{
		exception_handler( SIGMEM, exc.what( ) );
	}
	catch ( std::exception& exc )	// other known error conditions
	{
		exception_handler( SIGSTL, exc.what( ) );
	}
	catch ( ... )				// other unknown error conditions
	{
		abort( );				// raises a SIGABRT exception, tell user & close
	}

#endif

	lsd_exit( res );
	return res;
}
