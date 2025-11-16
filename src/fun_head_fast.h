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
FUN_HEAD_FAST.H
Header file to enable just current LSD syntax. Use fun_head.h
to keep compatibility with legacy LSD code.
*************************************************************/

#ifdef EQ_USER_CFUNS
	#ifndef LSDFUNINIT
		#error User C functions defined but 'fun_init.h' not included
		#include <stop>
	#endif
#endif

#ifdef LEGACY_CODE
	#undef LEGACY_CODE
#endif

#ifndef LSDEQUATION
	#include "lib/lsdequation.h"
#endif
