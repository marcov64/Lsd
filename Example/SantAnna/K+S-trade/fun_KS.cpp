/******************************************************************************

	MAIN K+S LSD MODEL (including labor and finance extensions)
	------------------

	Written by Marcelo C. Pereira, University of Campinas

	Copyright Marcelo C. Pereira
	Distributed under the GNU General Public License

	VERSION: 5.3.1 - Full LSD version

	This is the topmost code file for the K+S coded in LSD.
	It contains only the scheduling equations 'runCountry' and 'timeStep',
	and the main initialization equation 'init'.
	All remaining equations are loaded from '.h' include files.

 ******************************************************************************/

// disable/enable full logging (FASTMODE=0/1/2)
#define FASTMODE 0

// do not initialize and check LSD pointers
//#define NO_POINTER_INIT


/*======================== ADDITIONAL CODE TO INCLUDE ========================*/

#include <lsd_init.h>							// LSD base definitions
#include "fun_KS_class.h"						// K+S definitions
#include <lsd_head.h>							// LSD main definitions
#include "fun_KS_support.h"						// K+S support C++ functions


/*============================ GENERAL EQUATIONS =============================*/

MODELBEGIN

// The equation files below contains all LSD equations, organized by the
// container object.

#include "fun_KS_world.h"						// world (root) object equations
#include "fun_KS_country.h"						// Country object equations
#include "fun_KS_financial.h"					// Financial object equations
#include "fun_KS_bank.h"						// Bank objects equations
#include "fun_KS_capital.h"						// Capital object equations
#include "fun_KS_firm1.h"						// Firm1 objects equations
#include "fun_KS_consumption.h"					// Consumption object equations
#include "fun_KS_firm2.h"						// Firm2 objects equations
#include "fun_KS_vintage.h"						// Vint objects equations
#include "fun_KS_labor.h"						// Labor object equations
#include "fun_KS_worker.h"						// Worker objects equations
#include "fun_KS_stats.h"						// statistics only equations
#include "fun_KS_test.h"						// test only equations


/*========================= INITIALIZATION EQUATIONS =========================*/

EQUATION( "init" )
/*
Initialize the model, setting up K+S objects for each "Country" object.
Also configures LSD main flags.
*/

PARAMETER;										// execute only once

USE_SAVED;										// allow access to saved vars
USE_ZERO_INSTANCE;								// allow zero-instance objects
//NO_SEARCH;										// don't perform variable search
//NO_SEARCH_UP;
RND_GENERATOR( 2 );								// LSD source of randomness

random_engine.seed( RND_SEED );					// sync seeds between engines

if ( RUN == 1 )									// first run only
{
#ifdef FASTMODE
	if ( FASTMODE == 1 )						// set user selected log mode
		FAST;
	else
		if ( FASTMODE == 2 )
		{
			NO_POINTER_CHECK;					// don't check pointers
			USE_NAN;							// don't check NaN results
			FAST_FULL;
		}
		else
			OBSERVE;
#else
	FAST;										// or disable verbose logging
#endif
}

V( "initWorld" );								// initialize world

RESULT( 1 )


/*===================== SCHEDULING EQUATIONS (TIME LINE) =====================*/

EQUATION( "runCountry" )
/*
Execute the current time-step for all existing countries
*/

V( "init" );									// ensure initialization done

CYCLE( cur, "Country" )
{
	VS( cur, "regChg" );						// do regulatory regime change
	VS( cur, "e" );								// update exchange rates
}

CYCLE( cur, "Country" )
	VS( cur, "timeStep" );

RESULT( T )


EQUATION( "timeStep" )
/*
Execute one time-step of one country in the K+S model
Ensure the high-level scheduling of the model equation by forcing the
computation of key variables in the correct order
In principle, the sequence defined here is embedded in the equations, so this
equation is supposedly unnecessary (but may produce slightly different results)
*/

if ( V( "IDcnt" ) > 1 )							// countries other than first?
	END_EQUATION( T );							// likely triggered by first

i = 1;											// temporary variables index

// central bank updates prime rate and the interest rate structure is adjusted
// accordingly
NEW_VS( v[ i++ ], FINSECL0, "r" );				// prime interest rate
NEW_VS( v[ i++ ], FINSECL0, "rDeb" );			// interest rate on debt
NEW_VS( v[ i++ ], FINSECL0, "rBonds" );			// interest rate on bonds

// consumption-good firms define expected demand, planned production,
// labor demand, and desired investment for sector
NEW_VS( v[ i++ ], CONSECL0, "D2e" );			// expected demand for goods
NEW_VS( v[ i++ ], CONSECL0, "Q2" );				// planned goods production
NEW_VS( v[ i++ ], CONSECL0, "L2d" );			// total desired labor
NEW_VS( v[ i++ ], CONSECL0, "Id" );				// desired investment

// capital-good firms do R&D, receive orders, define machine production,
// and labor demand for sector
NEW_VS( v[ i++ ], CAPSECL0, "D1" );				// total orders for new machines
NEW_VS( v[ i++ ], CAPSECL0, "Q1" );				// planned machine production
NEW_VS( v[ i++ ], CAPSECL0, "L1d" );			// total desired labor

// workers apply for jobs, firms define open job positions
// and labor market search-and-match define effective employed labor
NEW_VS( v[ i++ ], LABSUPL0, "appl" );			// applications posted by workers
NEW_VS( v[ i++ ], CAPSECL0, "JO1" );			// open job positions in sector 1
NEW_VS( v[ i++ ], CONSECL0, "JO2" );			// open job positions in sector 2
NEW_VS( v[ i++ ], LABSUPL0, "L" );				// effective employed labor

// production is adjusted to labor effectively hired by firms
// and firms set prices
NEW_VS( v[ i++ ], CAPSECL0, "Q1e" );			// effective machine production
NEW_VS( v[ i++ ], CONSECL0, "Q2e" );			// effective goods production
NEW_VS( v[ i++ ], CAPSECL0, "p1avg" );			// average machine prices
NEW_VS( v[ i++ ], CONSECL0, "p2avg" );			// average goods prices

// government decides expenditure and workers set part of income to buy goods,
// demand is matched to supply of goods, producing inventories or forced savings
NEW_VS( v[ i++ ], THIS, "Gcd" );				// desired public consumption
NEW_VS( v[ i++ ], LABSUPL0, "CdW" );			// worker desired consumption
NEW_VS( v[ i++ ], CONSECL0, "D2lD" );			// desired local goods demand
NEW_VS( v[ i++ ], CONSECL0, "D2xD" );			// desired export goods demand
NEW_VS( v[ i++ ], CONSECL0, "D2" );				// fulfilled demand for goods
NEW_VS( v[ i++ ], CONSECL0, "N" );				// accumulated inventories

// workers consolidate consumption and gross income, pay taxes, and save
NEW_VS( v[ i++ ], LABSUPL0, "Cw" );				// workers total consumption
NEW_VS( v[ i++ ], LABSUPL0, "In" );				// worker gross income
NEW_VS( v[ i++ ], LABSUPL0, "TaxW" );			// tax paid by workers
NEW_VS( v[ i++ ], LABSUPL0, "Sav" );			// current savings of workers

// firms determine gross profits and taxes
NEW_VS( v[ i++ ], CAPSECL0, "Pi1" );			// profits of sector 1
NEW_VS( v[ i++ ], CONSECL0, "Pi2" );			// profits of sector 2
NEW_VS( v[ i++ ], FINSECL0, "PiB" );			// profits of banks
NEW_VS( v[ i++ ], CAPSECL0, "Tax1" );			// tax paid by sector 1
NEW_VS( v[ i++ ], CONSECL0, "Tax2" );			// tax paid by sector 2
NEW_VS( v[ i++ ], FINSECL0, "TaxB" );			// tax paid by banks

// government collects taxes and compute public deficit or surplus
NEW_VS( v[ i++ ], THIS, "G" );					// government total expenditure
NEW_VS( v[ i++ ], THIS, "Tax" );				// total tax income
NEW_VS( v[ i++ ], THIS, "Def" );				// public total deficit

// agents' net wealths, savings, or public debt are updated
NEW_VS( v[ i++ ], LABSUPL0, "SavAcc" );			// savings of workers
NEW_VS( v[ i++ ], CAPSECL0, "NW1" );			// net wealth of sector 1
NEW_VS( v[ i++ ], CONSECL0, "NW2" );			// net wealth of sector 2
NEW_VS( v[ i++ ], FINSECL0, "NWb" );			// net wealth of banks
NEW_VS( v[ i++ ], THIS, "Deb" );				// public accumulated debt

// macro variables are computed by aggregation of micro-level data
NEW_VS( v[ i++ ], THIS, "Yreal" );				// real gross domestic product
NEW_VS( v[ i++ ], THIS, "Y" );					// nominal gross domestic product

// exit and entry happens in both sectors and banks update credit scores
// and pecking order of client firms
NEW_VS( v[ i++ ], THIS, "entryExit" );			// net entry of firms

RESULT( T )


MODELEND


/*=========================== GARBAGE COLLECTION =============================*/

CLOSEBEGIN

object *cur, *cur1, *cur2;

CYCLES( ROOT, cur, "Country" )					// scan all country objects
{
	CYCLES( cur, cur1, "Consumption" )
		CYCLES( cur1, cur2, "Firm2" )			// free Firm2 extensions
			DELETE_EXTS( cur2, firm2E );

	DELETE_EXTS( cur, countryE );				// reclaim allocated memory
}

CLOSEEND

