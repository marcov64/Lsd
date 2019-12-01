/******************************************************************************

	MAIN K+S LSD MODEL (including labor and finance extensions)
	------------------
	
	VERSION: 5.1 - Full LSD version

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

// define variables to use in equations
#define EQ_USER_VARS dblVecT::iterator itd, itd1; \
					 objVecT::iterator ito, ito1; \
					 appLisT::iterator its, its1; \
					 woLisT::iterator itw, itw1;// general purpose iterators

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

DEFAULT_RESULT( NAN );							// default equation result
USE_ZERO_INSTANCE;								// allow zero-instance objects
NO_SEARCH;										// don't perform variable search

if ( cur_sim == 1 )								// first run only
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

CYCLE( cur, "Country" )
	VS( cur, "timeStep" );

RESULT( t )


EQUATION( "timeStep" )
/*
Execute one time-step of one country in the K+S model
Ensure the high-level scheduling of the model equation by forcing the 
computation of key variables in the correct order
In principle, the sequence defined here is embedded in the equations, so
this equation is likely unnecessary
*/

// sector 2 firms define market expectations, define new capital desired, and 
// incorporate new and scrap old machines

v[1] = VS( CONSECL0, "D2e" );					// expected demand for goods

// define expected demand, desired and planned production and capital, labor 
// demand, investment and credit required for sector 2 firms

v[2] = VS( CONSECL0, "Id" );					// desired investment sector 2

// sector 1 firms do R&D, set prices and send machine brochures, and sector 2  
// firms choose suppliers

v[3] = VS( CAPSECL0, "D1" );					// total demand for new machines

// sector 1 define machine production, total unbounded labor demand and
// required credit

v[4] = VS( CAPSECL0, "Q1" );					// planned production sector 1

// adjust labor demand, investment and production in all sectors according to 
// the effective labor available to firms

v[5] = VS( LABSUPL0, "L" );						// employed labor force

// adjust investment, capital and number of employed machines in sector 2 
// according to the finance and labor available and update productivity and 
// cost structures

v[6] = VS( CAPSECL0, "Q1e" );					// effective production sector 1
v[7] = VS( CONSECL0, "Q2e" );					// effective production sector 2

// sector 2 set prices, consumers choose suppliers and existing demand is
// matched to available supply of goods, producing inventories or forced savings

v[8] = VS( CONSECL0, "D2" );					// fulfilled demand in sector 2

// firms determine profits, cash flows, taxes and net wealths, using credit
// if possible and required

v[9] = VS( CAPSECL0, "Tax1" );					// tax paid by sector 1
v[10] = VS( CONSECL0, "Tax2" );					// tax paid by sector 2

// government collects taxes and decide about expenditure, public deficit or
// superavit form and its financed by public debt

v[11] = V( "Def" );								// public deficit

// macro variables are computed by simple aggregation of micro-level data

v[12] = V( "GDPnom" );							// nominal gross domestic product

// entry and exit happens in both sectors, entries are not related to exits,
// and banks adjust credit scores of firms and define the credit pecking order

v[13] = V( "entryExit" );						// net entry of firms

// banks compute bad debt, profits, credit available and

v[14] = VS( FINSECL0, "NWb" );					// liquid assets of banks

// central bank updates prime rate and the interest rate structure is adjusted
// accordingly

v[15] = VS( FINSECL0, "rDeb" );					// interest rate on debt

RESULT( t )


MODELEND


/*=========================== GARBAGE COLLECTION =============================*/

void close_sim( void )
{
	object *cur, *cur1, *cur2;
	
	CYCLES( root, cur, "Country" )				// scan all country objects
	{
		CYCLES( cur, cur1, "Consumption" )
			CYCLES( cur1, cur2, "Firm2" )		// free Firm2 extensions
				DELETE_EXTS( cur2, firm2 );

		DELETE_EXTS( cur, country );			// reclaim allocated memory
	}
}
