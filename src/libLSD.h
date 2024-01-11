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

// define LIBLSD_EXPORTS when building the dynamic library
#ifdef _WIN32
	#ifdef _LIBLSD_EXPORTS
		#define LSD_API __declspec( dllexport )
		#define LSD_API_V LSD_API
	#else
		#define LSD_API __declspec( dllimport )
		#define LSD_API_V LSD_API extern
	#endif
#else
	#define LSD_API
	#ifdef _LIBLSD_EXPORTS
		#define LSD_API_V
	#else
		#define LSD_API_V extern
	#endif
#endif
	

// LSD API functions
LSD_API int lsdmain( int argn, const char **argv );
LSD_API void myprint( void );
LSD_API void settest( int );

/*
clean_path
clean_file
*/

// LSD API variables
LSD_API_V int mytest;

/*
LSD_API_V exec_path
LSD_API_V exec_file
LSD_API_V
LSD_API_V
LSD_API_V
LSD_API_V
LSD_API_V
*/