/******************************************************************************

	K+S LSD MODEL (including finance, variable firms, climate and energy)
	-------------

	VERSION: 0.5.1 - climate model and energy sector with multi power suppliers

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

// LSD and K+S macros and objects definition and support code
#include <fun_head_fast.h>						// LSD definitions
#include "fun_KS_class.h"						// K+S class/macro definitions
#include "fun_KS_support.h"						// K+S support C++ functions


/*============================ GENERAL EQUATIONS =============================*/

MODELBEGIN

// The equation files below contains all LSD equations, organized by the
// container object.

#include "fun_KS_country.h"						// Country object equations
#include "fun_KS_financial.h"					// Financial object equations
#include "fun_KS_bank.h"						// Bank objects equations
#include "fun_KS_energy.h"						// Energy object equations
#include "fun_KS_firme.h"						// Energy firm object equations
#include "fun_KS_dirty.h"						// Dirty objects equations
#include "fun_KS_green.h"						// Green objects equations
#include "fun_KS_climate.h"						// Climate object equations
#include "fun_KS_capital.h"						// Capital object equations
#include "fun_KS_firm1.h"						// Firm1 objects equations
#include "fun_KS_consumption.h"					// Consumption object equations
#include "fun_KS_firm2.h"						// Firm2 objects equations
#include "fun_KS_vintage.h"						// Vint objects equations
#include "fun_KS_labor.h"						// Labor object equations
#include "fun_KS_stats.h"						// statistics only equations
#include "fun_KS_test.h"						// test only equations


/*========================= INITIALIZATION EQUATIONS =========================*/

EQUATION( "init" )
/*
Initialize the model, setting up K+S objects for each "Country" object.
Also configures LSD main flags.
*/

PARAMETER;										// execute only once

DEFAULT_RESULT( NAN );							// default equation result
USE_SAVED;										// allow access to saved vars
USE_ZERO_INSTANCE;								// allow zero-instance objects
NO_SEARCH;										// don't perform variable search

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

CYCLES( root, cur, "Country" )					// scan all country objects
	VS( cur, "initCountry" );					// initialize country

RESULT( 1 )


/*===================== SCHEDULING EQUATIONS (TIME LINE) =====================*/

EQUATION( "runCountry" )
/*
Execute the current time-step for all existing countries
*/

V( "init" );									// ensure initialization done

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

// central bank updates prime rate and the interest rate structure is adjusted
// accordingly
NEW_VS( v[1], FINSECL0, "r" );					// prime interest rate
NEW_VS( v[2], FINSECL0, "rDeb" );				// interest rate on debt
NEW_VS( v[3], FINSECL0, "rBonds" );				// interest rate on bonds

// consumption-good firms define expected demand, planned production,
// labor demand, and desired investment for sector
NEW_VS( v[4], CONSECL0, "D2e" );				// expected demand for goods
NEW_VS( v[5], CONSECL0, "Q2" );					// planned goods production
NEW_VS( v[6], CONSECL0, "L2d" );				// total desired labor
NEW_VS( v[7], CONSECL0, "Id" );					// desired investment

// capital-good firms do R&D, receive orders, define machine production,
// and labor demand for sector
NEW_VS( v[8], CAPSECL0, "D1" );					// orders for new machines
NEW_VS( v[9], CAPSECL0, "Q1" );					// planned machine production
NEW_VS( v[10], CAPSECL0, "L1d" );				// total desired labor

// firms define open job positions and labor market define employed labor
NEW_VS( v[12], CAPSECL0, "JO1" );				// open job positions in sector 1
NEW_VS( v[13], CONSECL0, "JO2" );				// open job positions in sector 2
NEW_VS( v[14], LABSUPL0, "L" );					// effective employed labor

// production is adjusted to labor effectively hired by firms
// and firms set prices
NEW_VS( v[15], CAPSECL0, "Q1e" );				// effective machine production
NEW_VS( v[16], CONSECL0, "Q2e" );				// effective goods production
NEW_VS( v[17], CAPSECL0, "p1avg" );				// average machine prices
NEW_VS( v[18], CONSECL0, "p2avg" );				// average goods prices

// government decides expenditure and workers set all income to buying goods,
// demand is matched to supply of goods, producing inventories or forced savings
NEW_VS( v[19], THIS, "G" );						// public total expenditures
NEW_VS( v[20], CONSECL0, "D2d" );				// desired goods demand
NEW_VS( v[21], CONSECL0, "D2" );				// fulfilled demand for goods
NEW_VS( v[22], CONSECL0, "N" );					// accumulated inventories
NEW_VS( v[23], THIS, "Sav" );					// forced consumer savings

// firms determine profits, taxes, cash flows, and update net wealths
NEW_VS( v[24], CAPSECL0, "Pi1" );				// profits of sector 1
NEW_VS( v[25], CONSECL0, "Pi2" );				// profits of sector 2
NEW_VS( v[26], FINSECL0, "PiB" );				// profits of banks
NEW_VS( v[27], CAPSECL0, "Tax1" );				// tax paid by sector 1
NEW_VS( v[28], CONSECL0, "Tax2" );				// tax paid by sector 2
NEW_VS( v[29], FINSECL0, "TaxB" );				// tax paid by banks
NEW_VS( v[30], CAPSECL0, "NW1" );				// net wealth of sector 1
NEW_VS( v[31], CONSECL0, "NW2" );				// net wealth of sector 2

// government collects taxes, compute public deficit or surplus form,
// and adjust public debt accordingly
NEW_VS( v[32], THIS, "Tax" );					// total tax income
NEW_VS( v[33], THIS, "Def" );					// public total deficit
NEW_VS( v[34], THIS, "Deb" );					// public accumulated debt

// macro variables are computed by aggregation of micro-level data
NEW_VS( v[35], THIS, "GDPreal" );				// real gross domestic product
NEW_VS( v[36], THIS, "GDPnom" );				// nominal gross domestic product

// exit and entry happens in both sectors and banks update credit scores
// and pecking order of client firms
NEW_VS( v[37], THIS, "entryExit" );				// net entry of firms

RESULT( T )


MODELEND


/*=========================== GARBAGE COLLECTION =============================*/

void close_sim( void )
{
	object *cur;

	CYCLES( root, cur, "Country" )				// scan all country objects
		DELETE_EXTS( cur, countryE );			// reclaim allocated memory
}
