/*************************************************************

	LSD 7.3 - December 2020
	written by Marco Valente, Universita' dell'Aquila
	and by Marcelo Pereira, University of Campinas

	Copyright Marco Valente and Marcelo Pereira
	LSD is distributed under the GNU General Public License
	
	See Readme.txt for copyright information of
	third parties' code used in LSD
	
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

#endif

	myexit( res );
	return res;
}
