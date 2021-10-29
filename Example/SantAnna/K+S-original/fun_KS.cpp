/******************************************************************************

	K+S LSD MODEL (SFC version)
	-------------
	
	VERSION: 0.1.1 - original JEDC2010 model plus stock-flow consistency (SFC)

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

// consumption-good firms define expected demand, planned production, 
// and labor demand for sector
v[1] = VS( CONSECL0, "D2e" );					// expected demand for goods
v[2] = VS( CONSECL0, "Q2" );					// planned goods production
v[3] = VS( CONSECL0, "L2d" );					// desired labor

// capital-good firms do R&D, set prices, and search for new clients
// and consumption-good firms define desired investment 
v[4] = VS( CAPSECL0, "p1avg" );					// average machine prices
v[5] = VS( CONSECL0, "Id" );					// desired investment

// capital-good firms receive orders, define machine production, 
// and labor demand for sector
v[6] = VS( CAPSECL0, "D1" );					// orders for new machines
v[7] = VS( CAPSECL0, "Q1" );					// planned machine production
v[8] = VS( CAPSECL0, "L1d" );					// desired labor

// labor market define effective employed labor, government decides expenditure 
// investment and production is adjusted to labor available to firms
v[9] = VS( LABSUPL0, "L" );					// employed labor
v[10] = V( "G" );								// public total expenditures
v[11] = VS( CAPSECL0, "Q1e" );					// effective machine production
v[12] = VS( CONSECL0, "Q2e" );					// effective goods production

// consumption-good firms set prices, existing demand is matched to 
// available supply of goods, producing inventories or forced savings
v[13] = VS( CONSECL0, "p2avg" );				// average goods prices
v[14] = VS( CONSECL0, "D2" );					// fulfilled demand for goods
v[15] = VS( CONSECL0, "N" );					// accumulated inventories
v[16] = V( "Sav" );								// forced consumer savings

// firms determine profits, taxes, cash flows, and update net wealths
v[17] = VS( CAPSECL0, "Pi1" );					// profits of sector 1
v[18] = VS( CONSECL0, "Pi2" );					// profits of sector 2
v[19] = VS( CAPSECL0, "Tax1" );					// tax paid by sector 1
v[20] = VS( CONSECL0, "Tax2" );					// tax paid by sector 2
v[21] = VS( CAPSECL0, "NW1" );					// net wealth of sector 1
v[22] = VS( CONSECL0, "NW2" );					// net wealth of sector 2

// government collects taxes, compute public deficit or surplus form,
// and adjust public debt accordingly
v[23] = V( "Tax" );								// total tax income
v[24] = V( "Def" );								// public total deficit
v[25] = V( "Deb" );								// public accumulated debt

// macro variables are computed by aggregation of micro-level data
v[26] = V( "GDP" );								// real gross domestic product
v[27] = V( "GDPnom" );							// nominal gross domestic product

// exit and entry happens in both sectors and banks update credit scores 
// and pecking order of client firms, and account bad debt losses
v[28] = V( "entryExit" );						// net entry of firms

RESULT( T )


MODELEND


/*=========================== GARBAGE COLLECTION =============================*/

void close_sim( void )
{
	object *cur;
	
	CYCLES( root, cur, "Country" )				// scan all country objects
		DELETE_EXTS( cur, country );			// reclaim allocated memory
}
