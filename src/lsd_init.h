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
 LSD_INIT.H
 This file contains the macros required to define
 user-created C functions to be used in equations.
 Must be included at the very beginning of the equation file.
 *************************************************************/

#ifdef EQ_USER_CFUNS
	#error User C functions defined before 'lsd_init.h' is included
	#include <stop>
#endif

#define LSDFUNINIT

// macros to define user-defined C functions to be used in equations
#define CFUN_DBL( N, ... ) double N( object *_p_, const variable *_v_, object *_c_ __VA_OPT__( , ) __VA_ARGS__ )
#define CFUN_INT( N, ... ) int N( object *_p_, const variable *_v_, object *_c_ __VA_OPT__( , ) __VA_ARGS__ )
#define CFUN_OBJ( N, ... ) object *N( object *_p_, const variable *_v_, object *_c_ __VA_OPT__( , ) __VA_ARGS__ )
#define CFUN_VOID( N, ... ) void N( object *_p_, const variable *_v_, object *_c_ __VA_OPT__( , ) __VA_ARGS__ )
