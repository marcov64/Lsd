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
 The LSD Browser GUI program entry point.

 This file can be compiled with the command make in the src
 directory.

 Relevant macros for conditional compilation (when defined):

 - _EQ_: user model equation file
 - _TERM_: terminal executable
 - _NT_: no signal trapping (better when debugging in GDB)
 *************************************************************/

#include "LSD.h"


/*************************************************************
 MAIN
 *************************************************************/
int main( int argn, const char **argv )
{
	int res = -1;

#ifndef _NT_

	// register all signal handlers
	lsd::handle_signals( lsd::signal_handler );

	try
	{

#endif
		// start of GUI load
		res = gui::load_gui( argv );

#ifndef _NT_

	}
	catch ( std::bad_alloc& exc )	// out of memory conditions
	{
		lsd::exception_handler( SIGMEM, exc.what( ) );
	}
	catch ( std::exception& exc )	// other known error conditions
	{
		lsd::exception_handler( SIGSTL, exc.what( ) );
	}
	catch ( ... )				// other unknown error conditions
	{
		abort( );				// raises a SIGABRT exception, tell user & close
	}

#endif

	return res;
}
