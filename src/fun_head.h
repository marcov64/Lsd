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
FUN_HEAD.H
Header file to enable deprecated LSD syntax. Use instead
lsd_head.h to ensure just current and faster LSD code is used.
*************************************************************/

#ifndef LEGACY_CODE
	#define LEGACY_CODE
#endif

#ifndef LSDEQUATION
	#include "lib/lsdequation.h"
#endif
