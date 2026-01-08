/******************************************************************************

	Multi-Industry K+S LSD MODEL
	----------------------------

	Written by Marcelo C. Pereira, University of Campinas

	Copyright Marcelo C. Pereira
	Distributed under the GNU General Public License

	VERSION: 6.0 - Full LSD version

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

CYCLES( ROOT, cur, "Country" )					// scan all country objects
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

// sector 2 firms define market expectations, define new capital desired, and
// incorporate new and scrap old machines

v[1] = SUM( "D2e" );							// expected demand for goods

// sector 1 firms do R&D, set prices and send machine brochures, and sector 2
// firms choose suppliers

v[2] = SUM( "Id" );								// desired investment sector 2

// sector 1 define machine production, total unbounded labor demand and
// required credit

v[3] = SUM( "D1" );								// total demand for new machines

// define expected demand, desired and planned production and capital, labor
// demand, investment and credit required for sector 2 firms

v[4] = SUM( "Q1" );								// planned production sector 1

// adjust labor demand, investment and production in all sectors according to
// the effective labor available to firms

v[5] = VS( LABSUPL0, "L" );						// employed labor force

// adjust investment, capital and number of employed machines in sector 2
// according to the finance and labor available and update productivity and
// cost structures

v[6] = SUM( "Q1e" );							// effective production sector 1
v[7] = SUM( "Q2e" );							// effective production sector 2

// sector 2 set prices, consumers choose suppliers and existing demand is
// matched to available supply of goods, producing inventories or forced savings

v[8] = SUM( "D2" );								// fulfilled demand in sector 2

// firms determine profits, cash flows, taxes and net wealths, using credit
// if possible and required

v[9] = SUM( "Tax1" );							// tax paid by sector 1
v[10] = SUM( "Tax2" );							// tax paid by sector 2

// banks compute bad debt, profits, credit available and

v[11] = VS( FINSECL0, "NWb" );					// liquid assets of banks

// government collects taxes and decide about expenditure, public deficit or
// superavit form and its financed by public debt

v[12] = V( "Def" );								// public deficit

// macro variables are computed by simple aggregation of micro-level data

v[13] = V( "GDPreal" );							// real gross domestic product

// central bank updates prime rate and the interest rate structure is adjusted
// accordingly

v[14] = VS( FINSECL0, "rDeb" );					// interest rate on debt

// entry and exit happens in both sectors, entries are not related to exits,
// and banks adjust credit scores of firms and define the credit pecking order

v[15] = V( "entryExit" );						// net entry of firms

RESULT( T )


MODELEND


/*=========================== GARBAGE COLLECTION =============================*/

CLOSEBEGIN

object *cur, *cur1, *cur2;

CYCLES( ROOT, cur, "Country" )					// scan all country objects
{
	CYCLES( cur, cur1, "Consumption" )
	{
		CYCLES( cur1, cur2, "Firm2" )			// free Firm2 extensions
			DELETE_EXTS( cur2, firm2E );

		DELETE_EXTS( cur1, ind2E );				// free Consumption extensions
	}

	cur1 = SEARCHS( cur, "Labor" );
	CYCLES( cur1, cur2, "Worker" )				// free Worker extensions
		DELETE_EXTS( cur2, wrkE );

	DELETE_EXTS( cur, countryE );				// free Country extensions
}

CLOSEEND

