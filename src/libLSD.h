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
LIBLSD.H
This file contains the list of LSD functions to be exported
by a model dynamic link library (.dll/.so).
*************************************************************/

// DLL_EXPORT must be defined when building the dynamic library
#if defined _WIN32 && defined _DLL_
	#ifdef DLL_EXPORT
		#define API __declspec( dllexport )
	#else
		#define API __declspec( dllimport )
	#endif
#else
	#define API
#endif

// LSD API functions
API int load_gui( const char **argv );
API int load_term_configuration( int argn, const char **argv );
API void exception_handler( int signum, const char *what = NULL );
API void handle_signals( void ( * handler ) ( int signum ) );
API void myexit( int v );
API void run( void );
API void set_exec( const char *path, const char *file );
API void signal_handler( int signum );
