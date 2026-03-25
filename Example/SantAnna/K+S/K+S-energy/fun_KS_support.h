/******************************************************************************

	SUPPORT C FUNCTIONS
	-------------------

	Written by Marcelo C. Pereira, University of Campinas

	Copyright Marcelo C. Pereira
	Distributed under the GNU General Public License

	Pure C support functions used in the objects in the K+S LSD model are
	coded below.

 ******************************************************************************/

/*======================== GENERAL SUPPORT C FUNCTIONS =======================*/

// calculate the bounded, moving-average growth rate of variable
// if lim is zero, there is no bounding

CFUN_DBL( mov_avg_bound, const char *var, double lim, double per, int lag = 0 )
{
	double prev, g, sum_g;
	int i;

	for ( sum_g = i = 0; i < per; ++i )
	{
		if ( T - i + lag <= 0 )					// just go to t=1
			break;

		prev = VL( var, i + lag + 1 );
		g = ( prev != 0 ) ? VL( var, i + lag ) / prev - 1 : 0;

		if ( lim > 0 )
			g = max( min( g, lim ), - lim );	// apply bounds

		sum_g += g;
	}

	return sum_g / i;
}


// append error messages and increment error counter

CFUN_VOID( check_error, bool cond, const char* errMsg, int errCount, int *errCounter )
{
	if ( ! cond )
		return;

	if ( errCount == 0 )
		LOG( " %s", errMsg );
	else
		LOG( " %s(%d)", errMsg, errCount );

	++( *errCounter );
}


/*====================== FINANCIAL SUPPORT C FUNCTIONS =======================*/

// comparison function for sort method in equation 'cScores', '_cScores'

bool rank_desc_NWtoS( firmRank e1, firmRank e2 )
{
	return e1.NWtoS > e2.NWtoS;
}


// compute the net present value of N equal payments PMT and interest rate r,
// with initial payment optionally deferred by def periods

double npv( double pmt, double r, double n, double def = 0 )
{
	double npv = 0;

	for ( double i = 1; i <= n; ++i )
		npv += pmt / pow( 1 + r, i + def );

	return npv;
}


// set initial bank for entrant in equations 'entry1exit', 'entry2exit',
// 'entryEexit'

const char *bankPar[ ] = { "_bank1", "_bank2", "_bankE" },
		   *CliObj[ ] = { "Cli1", "Cli2", "CliE" },
		   *_IDpar[ ] = { "_ID1","_ID2", "_IDe" },
		   *__IDpar[ ] = { "__ID1","__ID2", "__IDe" };

CFUN_OBJ( set_bank )
{
	int _IDb, sec = strcmp( NAME, "Firm1" ) == 0 ? 0 :
					strcmp( NAME, "Firm2" ) == 0 ? 1 : 2;
	object *bank, *cli, *fin = V_EXTS( GRANDPARENT, countryE, finSec );

	_IDb = VS( fin, "pickBank" );				// draw initial preferred bank
	bank = V_EXTS( GRANDPARENT, countryE, bankPtr[ _IDb - 1 ] );
	WRITE( bankPar[ sec ], _IDb );				// save bank ID
	WRITE_HOOK( BANK, bank );

	cli = ADDOBJS( bank, CliObj[ sec ] );		// add to bank client list
	WRITES( cli, __IDpar[ sec ], V( _IDpar[ sec ] ) );// update object
	WRITE_SHOOKS( cli, THIS );					// pointer back to client
	WRITE_HOOK( BCLIENT, cli );					// pointer to bank client list

	return HOOK( BCLIENT );						// bank client list obj
}


// update firm debt in equations '_Q1', '_Tax1', '_Q2', '_EI', '_SI', '_Tax2',
// '_EIe', '_TaxE'

const char *_CDvar[ ] = { "_CD1", "_CD2", "_CDe", "_CDge" },
		   *_CDcVar[ ] = { "_CD1c", "_CD2c", "_CDeC", "_CDgeC" },
		   *_CSvar[ ] = { "_CS1", "_CS2", "_CSe", "_CSge" },
		   *_DebVar[ ] = { "_Deb1", "_Deb2", "_DebE", "__DebGE" },
		   *_TCfreeVar[ ] = { "_TC1free", "_TC2free", "_TCeFree", "_TCgeFree" };

CFUN_DBL( update_debt, double desired, double loan, object *plant = NULL )
{
	double Deb, TCfree;
	object *bank, *debObj = plant == NULL ? THIS : plant;
	int TfinGE, dest = strcmp( NAME, "Firm1" ) == 0 ? 0 :
					   strcmp( NAME, "Firm2" ) == 0 ? 1 :
					   plant == NULL ? 2 : 3;

	if ( desired > 0 )							// ignore loan repayment
	{
		INCR( _CDvar[ dest ], desired );		// desired credit
		INCR( _CDcVar[ dest ], desired - loan );// credit constraint
		INCR( _CSvar[ dest ], loan );			// supplied credit
	}

	Deb = VS( debObj, _DebVar[ dest ] );

	// take new loan/repay debt from/to bank
	if ( loan != 0 )
	{
		if ( Deb + loan < 0.001 )				// write-off small debt?
			Deb = WRITES( debObj, _DebVar[ dest ], 0 );
		else
			Deb = INCRS( debObj, _DebVar[ dest ], loan );

		if ( dest == 3 )
		{
			TfinGE = min( VS( PARENT, "Tfin" ),
						  VS( PARENT, "etaE" ) );// viable period

			WRITES( plant, "__TfinGE", TfinGE );
			WRITES( plant, "__rGEdeb", V( "_rEdeb" ) );
			WRITES( plant, "__pfinGE", 1 );
		}

		bank = HOOK( BANK );					// firm's bank

		// if credit limit active, adjust bank's available credit
		TCfree = VS( bank, _TCfreeVar[ dest ] );// available credit firm's bank
		if ( TCfree > -0.1 )
			WRITES( bank, _TCfreeVar[ dest ], max( TCfree - loan, 0 ) );
	}

	return Deb;
}


// update firm deposits in equations '_Q1', '_Tax1', '_Q2', '_EI', '_SI',
// '_Tax2', 'EIe', '_TaxE'

const char *_NWvar[ ] = { "_NW1", "_NW2", "_NWe" };

CFUN_DBL( update_depo, double depo, bool incr )
{
	double NW;
	int sec = strcmp( NAME, "Firm1" ) == 0 ? 0 :
			  strcmp( NAME, "Firm2" ) == 0 ? 1 : 2;

	// update total firm net worth (deposits)
	if ( incr )
	{
		NW = V( _NWvar[ sec ] );

		if ( depo != 0 )
			NW = INCR( _NWvar[ sec ], depo );
	}
	else
		NW = WRITE( _NWvar[ sec ], depo );

	return NW;
}


// manage firm cash flow in equations '_Tax1', '_Tax2', '_TaxE'

const char *_CIvar[ ] = { "", "_CI", "_CIe" },
		   *_CSaVar[ ] = { "_CS1a", "_CS2a", "_CSeA" },
		   *_DivVar[ ] = { "_Div1", "_Div2", "_DivE" },
		   *_NWpVar[ ] = { "_NW1p", "_NW2p", "" };

CFUN_DBL( cash_flow, double profit, double tax )
{
	int sec = strcmp( NAME, "Firm1" ) == 0 ? 0 :
			  strcmp( NAME, "Firm2" ) == 0 ? 1 : 2;
	object *fin = V_EXTS( GRANDPARENT, countryE, finSec );

	double dividends = VL( _DivVar[ sec ], 1 );// shareholder dividends
	double amort = sec < 2 ? 0 : V( "_amtGE" );// proj. fin. amortization
	double cashFree = profit - tax - dividends - amort;	// final free cash flow

	if ( sec > 0 )
		V( _CIvar[ sec ] );						// ensure canc. invest. reimbursed

	double provision = ( sec < 2 ) ? V( _NWpVar[ sec ] ) : 0;// prod. cost
	double depo = CFUN( update_depo, provision, true );// current bank deposits

	if ( cashFree < 0 )							// must finance losses?
	{
		if ( depo >= - cashFree )				// deposits cover losses?
			CFUN( update_depo, cashFree, true );// draw from deposits
		else
		{
			double credAvb = V( _CSaVar[ sec ] );// available credit
			double credDes = - cashFree - depo;	// desired credit

			CFUN( update_debt, credDes, credDes );// finance all in any case

			if ( credAvb >= credDes )			// could finance losses?
				CFUN( update_depo, 0, false );	// keep going with zero deposits
			else
				CFUN( update_depo, -1e-6, false );// let negative NW (bankruptcy)
		}
	}
	else										// pay debt with available cash
	{
		double repayDes = V( _DebVar[ sec ] ) * VS( fin, "deltaB" );
												// desired debt repayment
		if ( repayDes > 0 )						// something to repay?
		{
			if ( cashFree > repayDes )			// can repay desired and more
			{
				CFUN( update_debt, 0, - repayDes );// repay up to desired
				CFUN( update_depo, cashFree - repayDes, true );// keep the rest
			}
			else
				CFUN( update_debt, 0, - cashFree );// repay what is possible
		}
		else
			CFUN( update_depo, cashFree, true );// just keep all
	}

	return cashFree;
}


/*================== CAPITAL MANAGEMENT SUPPORT C FUNCTIONS ==================*/

// send machine brochure to consumption-good client firm in equations '_NC',
// '_supplier'

const char *Cli1Obj[ ] = { "", "Cli", "CliEn" },
		   *CliBrochObj[ ] = { "Cli", "Broch", "BrE" },
		   *__IDcPar[ ] = { "", "__IDc", "__IDcE" },
		   *__IDsPar[ ] = { "", "__IDs", "__IDsE" },
		   *__tSelPar[ ] = { "", "__tSel", "__tSelE" };

CFUN_OBJ( send_brochure, object *client )
{
	object *broch, *cli;
	int sec = strcmp( NAMES( client ), "Firm2" ) == 0 ? 1 : 2;

	cli = ADDOBJ( Cli1Obj[ sec ] );				// add object to new client
	WRITES( cli, __IDcPar[ sec ], VS( client, _IDpar[ sec ] ) );// client ID
	WRITES( cli, __tSelPar[ sec ], T );			// update selection time

	broch = ADDOBJS( client, CliBrochObj[ sec ] );// add brochure to client
	WRITES( broch, __IDsPar[ sec ], V( _IDpar[ 0 ] ) );// supplier ID
	WRITE_SHOOKS( broch, cli );					// pointer to supplier client list
	WRITE_SHOOKS( cli, broch );					// pointer to client brochure list

	return broch;
}


// set initial supplier for entrant in equations 'entry2exit', 'entryEexit'

CFUN_OBJ( set_supplier )
{
	object *broch, *suppl,
		   *cap = V_EXTS( GRANDPARENT, countryE, capSec );

	suppl = RNDDRAWS( cap, "Firm1", "_AtauLP" );// draw capital supplier
	broch = CFUNS( suppl, send_brochure, THIS );// get supplier brochure
	WRITE_HOOK( SUPPL, broch );					// pointer to current supplier
	INCRS( suppl, "_NC", 1 );					// update supplier's clients #

	return suppl;
}


// send new machine order in equations '_EI', '_SI', '_EIe'

const char *__nCanPar[ ] = { "", "__nCan", "__nCanE" },
		   *__nOrdPar[ ] = { "", "__nOrd", "__nOrdE" },
		   *__tOrdPar[ ] = { "", "__tOrd", "__tOrdE" };

CFUN_VOID( send_order, double nMach )
{
	int sec = strcmp( NAME, "Firm2" ) == 0 ? 1 : 2;

	// find firm entry on supplier client list
	object *cli = SHOOKS( HOOK( SUPPL ) );

	if ( VS( cli, __tOrdPar[ sec ] ) < T )		// if first order in period
	{
		WRITES( cli, __nOrdPar[ sec ], nMach );	// set new order size
		WRITES( cli, __tOrdPar[ sec ], T );		// set order time
		WRITES( cli, __nCanPar[ sec ], 0 );		// no machine canceled yet
	}
	else
		INCRS( cli, __nOrdPar[ sec ], nMach );	// increase existing order size
}


// perform investment according to available funding in equations '_EI', '_SI'

CFUN_DBL( invest, double desired )
{
	double invest, invCost, loan, loanDes;

	if ( desired <= 0 )
		return 0;

	double m2 = VS( PARENT, "m2" );				// machine output per period
	double _CS2a = V( "_CS2a" );				// available credit supply
	double _NW2 = V( "_NW2" );					// net worth (cash available)
	double _p1 = VS( PARENTS( SHOOKS( HOOK( SUPPL ) ) ), "_p1" );

	invCost = _p1 * desired / m2;				// desired investment cost

	if ( invCost <= _NW2 )						// can invest with own funds?
	{
		invest = desired;						// invest as planned
		_NW2 -= invCost;						// remove machines cost from cash
	}
	else
	{
		if ( invCost <= _NW2 + _CS2a )			// possible to finance all?
		{
			invest = desired;					// invest as planned
			loan = loanDes = invCost - _NW2;	// finance the difference
			_NW2 = 0;							// no cash
		}
		else									// credit constrained firm
		{
			// invest as much as the available finance allows, rounded # machines
			invest = max( floor( ( _NW2 + _CS2a ) / _p1 ) * m2, 0 );
			loanDes = invCost - _NW2;			// desired credit

			if ( invest == 0 )
				loan = 0;						// no finance
			else
			{
				invCost = _p1 * invest / m2;	// reduced investment cost
				if ( invCost <= _NW2 )			// just own funds?
				{
					loan = 0;
					_NW2 -= invCost;			// remove machines cost from cash
				}
				else
				{
					loan = invCost - _NW2;		// finance the difference
					_NW2 = 0;					// no cash
				}
			}
		}

		CFUN( update_debt, loanDes, loan );		// update debt (desired/granted)
	}

	if ( invest > 0 )
	{
		CFUN( update_depo, _NW2, false );		// update the firm net worth
		CFUN( send_order, round( invest / m2 ) );// order to machine supplier
	}

	return invest;
}


// add new vintage to the capital stock of a firm in equation 'K' and 'initCountry'

CFUN_VOID( add_vintage, double nMach, bool newInd )
{
	double __AeeVint, __AefVint, __AlpVint, __pVint, dY0, nInt, nRem;
	int __nVint, __tVint, eta;
	object *cap, *cur, *suppl, *vint;
	intVecT vintUse;

	suppl = PARENTS( SHOOKS( HOOK( SUPPL ) ) );	// current supplier
	nMach = floor( max( nMach, 1 ) );			// integer number of machines
	dY0 = VS( GRANDPARENT, "dGDP0" );			// growth rate at t=0

	// at t=1 firms have a mix of machines: old to new, many suppliers
	if ( newInd )
	{
		cap = V_EXTS( GRANDPARENT, countryE, capSec );
		eta = VS( V_EXTS( GRANDPARENT, countryE, conSec ), "eta" );// tech. life

		__tVint = - eta;						// time of oldest vintage to try
		__AeeVint = CFUN( init_cond, "AtauEE0" );// initial energy efficiency
		__AefVint = CFUN( init_cond, "AtauEF0" );// initial envir. friendliness
		__AlpVint = CFUN( init_cond, "AtauLP0" ) /
					pow( 1 + dY0, eta );		// productivity of oldest vintage
		__pVint = CFUN( init_cond, "p10" );		// initial machine price

		nInt = floor( nMach / ( eta + 1 ) );	// machines per every vintage
		nRem = nMach - nInt * ( eta + 1 );		// remainder machines
		vintUse.resize( eta + 1 );				// list of installed vint. times
		iota( vintUse.begin( ), vintUse.end( ), - eta );// assign vint. times
		shuffle( vintUse.begin( ), vintUse.end( ), random_engine );
		vintUse.resize( nRem );					// random vintages for remainder
	}
	else
	{
		__tVint = T;
		__AeeVint = VS( suppl, "_AtauEE" );
		__AefVint = VS( suppl, "_AtauEF" );
		__AlpVint = VS( suppl, "_AtauLP" );
		__pVint = VS( suppl, "_p1" );
	}

	while ( nMach > 0 )
	{
		// adjust non-integer differences randomly over vintages
		if ( newInd )
		{
			__nVint = nInt;						// allocate uniform part
			if ( std::find( vintUse.begin( ), vintUse.end( ), __tVint ) !=
				 vintUse.end( ) )
				__nVint++;						// allocate remainder part
		}
		else
			__nVint = nMach;

		if ( __nVint > 0 )
		{
			if ( newInd )
			{
				do
					cur = RNDDRAW_FAIRS( cap, "Firm1" );// draw another supplier
				while ( cur == suppl );			// don't use current supplier

				vint = ADDOBJL( "Vint", T - 1 );// recalculate in t=1
			}
			else
			{
				cur = suppl;					// just use current supplier
				vint = ADDOBJ( "Vint" );		// just recalculate in next t
			}

			WRITE_SHOOKS( vint, HOOK( TOPVINT ) );// save previous vintage
			WRITE_HOOK( TOPVINT, vint );	// save pointer to top vintage

			WRITES( vint, "__IDvint", VNT( T, VS( cur, "_ID1" ) ) );// vintage ID
			WRITES( vint, "__AeeVint", __AeeVint );	// vintage energy efficiency
			WRITES( vint, "__AefVint", __AefVint );	// vintage envir. friendliness
			WRITES( vint, "__AlpVint", __AlpVint );	// vintage labor productivity
			WRITES( vint, "__nVint", __nVint );	// number of machines in vintage
			WRITES( vint, "__pVint", __pVint );	// price of machines in vintage
			WRITES( vint, "__tVint", __tVint );	// vintage build time

			nMach -= __nVint;
		}

		__tVint++;
		__AlpVint *= 1 + dY0;
	}
}


// scrap (remove) vintage from capital stock in equation 'K'
// return -1 if last vintage (not removed but shrank to 1 machine)

CFUN_DBL( scrap_vintage )
{
	double RS;

	if ( NEXT != NULL )							// don't remove last vintage
	{
		// remove as previous vintage from next vintage
		if ( SHOOKS( NEXT ) == THIS )
			WRITE_SHOOKS( NEXT, NULL );

		RS = abs( V( "__RSvint" ) );
		DELETE( THIS );							// delete vintage
	}
	else
	{
		RS = -1;								// signal last machine
		WRITE( "__nVint", 1 );					// keep just 1 machine
	}

	return RS;
}


// add new power plant to energy firm in equation 'EIe' and 'initCountry'

const char *plantObj[ ] = { "Dirty", "Green" };
const char *__lifeEcycle[ ] = { "__lifeDEcycle", "__lifeGEcycle" };
const char *__tE[ ] = { "__tDE", "__tGE" };
const char *__QeU[ ] = { "__QdeU", "__QgeU" };
const char *__Ke[ ] = { "__Kde", "__Kge" };

CFUN_OBJ( add_plant, int type, double cap, double nMach, bool newInd )
{
	double __Ade, __emDE, __pMach, __uPlant, fGE0, nInt, nRem;
	int __nPlant, __tPlant, etaE, nGE;
	object *plant;
	intVecT plantUse;

	nMach = floor( max( nMach, 1 ) );			// integer number of machines
	__Ade = V( "_AtauDE" );						// plant thermal efficiency
	__emDE = V( "_emTauDE" );					// plant emission coefficient
	__uPlant = 1 / ( 1 + VS( PARENT, "iotaE" ) );// planned utilization

	// at t=1 firms have a mix of newer/older plants with minimum 1 machine each
	if ( newInd )
	{
		etaE = VS( PARENT, "etaE" );
		fGE0 = VS( PARENT, "fGE0" );
		nGE = COUNT( "Green" );

		__tPlant = - etaE;
		__pMach = CFUN( init_cond, "p10" );		// initial machine price

		if ( type == DIRTY && nMach == 1 && fGE0 > 0 && nGE > 0 )
			nMach = floor( nGE / fGE0 );

		cap /= min( nMach, etaE + 1 );			// capacity of each plant
		nInt = floor( nMach / ( etaE + 1 ) );	// machines per every plant
		nRem = nMach - nInt * ( etaE + 1 );		// remainder machines
		plantUse.resize( etaE + 1 );			// list of installed plant times
		iota( plantUse.begin( ), plantUse.end( ), - etaE );// assign plant times
		shuffle( plantUse.begin( ), plantUse.end( ), random_engine );
		plantUse.resize( nRem );				// random plants for remainder
	}
	else
	{
		__tPlant = T;
		__pMach = VS( PARENTS( SHOOKS( HOOK( SUPPL ) ) ), "_p1" );
	}

	while ( nMach > 0 )
	{
		// adjust non-integer differences randomly over plants
		if ( newInd )
		{
			__nPlant = nInt;					// allocate uniform part
			if ( std::find( plantUse.begin( ), plantUse.end( ), __tPlant ) !=
				 plantUse.end( ) )
				__nPlant++;						// allocate remainder part
		}
		else
			__nPlant = nMach;

		// create plant if there are machines to allocate
		if ( __nPlant > 0 )
		{
			if ( newInd )
			{
				plant = ADDOBJL( plantObj[ type ], T - 1 );// recalculate in t=1
				WRITES( plant, __lifeEcycle[ type ], 2 );// already operational
				WRITELLS( plant, __QeU[ type ], __uPlant, T - 1, 1 );
			}
			else
			{
				plant = ADDOBJ( plantObj[ type ] );// just recalculate in next t
				RECALCS( plant, __lifeEcycle[ type ] );// recalculate status
				WRITES( plant, __QeU[ type ], __uPlant );
			}

			WRITES( plant, __tE[ type ], __tPlant );// installation time
			WRITES( plant, __Ke[ type ], cap );	// plant generation capacity

			if ( type == DIRTY )
			{
				WRITES( plant, "__Ade", __Ade );
				WRITES( plant, "__emDE", __emDE );
			}
			else
			{
				WRITE_HOOK( TOPVINT, plant );	// new top green vintage
				WRITES( plant, "__ICge", __pMach * __nPlant );// plant capital cost
				WRITES( plant, "__mGE", cap / __nPlant );// unit power capacity
			}

			nMach -= __nPlant;
		}

		__tPlant++;
	}

	return plant;
}


/*=================== FIRM ENTRY-EXIT SUPPORT C FUNCTIONS ====================*/

// add and configure entrant capital-good firm object(s) and required hooks
// in equations 'entry1exit' and 'initCountry'

CFUN_DBL( entry_firm1, int n, bool newInd )
{
	double _AtauEE, _AtauEF, _AtauLP, _BtauEE, _BtauEF, _BtauLP, _D10, _Deb1,
		   _Eq1, _L1rd, _NW1, _NW10, _RD0, _c1, _cTau, _f1, _p1, AtauLPmax,
		   BtauLPmax, Deb1, Eq1, NW1, mult;
	int _ID1, _t1ent;
	object *firm, *bank,
		   *cons = V_EXTS( PARENT, countryE, conSec ),
		   *ene = V_EXTS( PARENT, countryE, eneSec ),
		   *lab = V_EXTS( PARENT, countryE, labSup );

	double Deb10ratio = V( "Deb10ratio" );		// bank fin. to equity ratio
	double Phi3 = V( "Phi3" );					// lower support for wealth share
	double Phi4 = V( "Phi4" );					// upper support for wealth share
	double alpha2 = V( "alpha2" );				// lower support for imitation
	double beta2 = V( "beta2" );				// upper support for imitation
	double mu1 = V( "mu1" );					// mark-up in sector 1
	double m1 = V( "m1" );						// worker production scale
	double nu = V( "nu" );						// share of R&D expenses
	double pE = VS( ene, "pE" );				// energy price
	double trCO2 = VS( PARENT, "trCO2" );		// carbon tax rate
	double x5 = V( "x5" );						// entrant upper advantage
	double w = VS( lab, "w" );					// current wage

	if ( newInd )
	{
		_AtauEE = CFUN( init_cond, "AtauEE0" );	// initial products.
		_AtauEF = CFUN( init_cond, "AtauEF0" );
		_AtauLP = AtauLPmax = CFUN( init_cond, "AtauLP0" );
		_BtauEE = CFUN( init_cond, "BtauEE0" );
		_BtauEF = CFUN( init_cond, "BtauEF0" );
		_BtauLP = BtauLPmax = CFUN( init_cond, "BtauLP0" );
		_D10 = CFUN( init_cond, "D10" ) / n;	// steady-state equil. demand
		_NW10 = V( "NW10" );					// initial wealth in sector 1
		_f1 = 1.0 / n;							// fair share
		_t1ent = 0;								// entered before t=1
	}
	else
	{
		_AtauEE = AVE( "_AtauEE" );				// avg. machine energy efficiency
		_AtauEF = AVE( "_AtauEF" );				// avg. machine envir. friendl.
		_BtauEE = AVE( "_BtauEE" );				// avg. energy effic. in sector 1
		_BtauEF = AVE( "_BtauEF" );				// avg. env. friend. in sector 1
		_NW10 = max( WHTAVE( "_NW1", "_f1" ), V( "NW10" ) *
					 V( "PPI" ) / V( "pK0" ) );
		_f1 = 0;								// no market share
		_t1ent = T;								// entered now
		AtauLPmax = MAX( "_AtauLP" );			// best machine lab. productivity
		BtauLPmax = MAX( "_BtauLP" );			// best productivity in sector 1

		// initial demand equal to 1 machine per client under fair share entry
		_D10 = VS( cons, "F2" ) / V( "F1" );
	}

	// add entrant firms (end of period, don't try to sell)
	for ( Deb1 = Eq1 = NW1 = 0; n > 0; --n )
	{
		// create object, only recalculate in t if new industry
		if ( newInd )
			firm = ADDOBJL( "Firm1", T - 1 );
		else
			firm = ADDOBJ( "Firm1" );

		_ID1 = INCR( "lastID1", 1 );			// new firm ID
		WRITES( firm, "_ID1", _ID1 );

		ADDHOOKS( firm, FIRM1HK );				// add object hooks
		DELETE( SEARCHS( firm, "Cli" ) );		// remove empty instances
		DELETE( SEARCHS( firm, "CliEn" ) );

		// select associated bank
		bank = CFUNS( firm, set_bank );

		if ( ! newInd )
		{
			// initial labor productivity (imitation from best firm)
			_AtauLP = beta( alpha2, beta2 );	// draw A from Beta(alpha,beta)
			_AtauLP *= AtauLPmax * ( 1 + x5 );	// fraction of top firm
			_BtauLP = beta( alpha2, beta2 );	// draw B from Beta(alpha,beta)
			_BtauLP *= BtauLPmax * ( 1 + x5 );	// fraction of top firm
		}

		// initial cost, price and net wealth
		mult = newInd ? 1 : uniform( Phi3, Phi4 );// NW multiple
		_c1 = ( w / _BtauLP + ( pE + trCO2 * _BtauEF ) / _BtauEE ) / m1;// unit cost
		_cTau = w / _AtauLP + ( pE + trCO2 * _AtauEF ) / _AtauEE;// u. cost clients
		_p1 = ( 1 + mu1 ) * _c1;				// unit price
		_RD0 = nu * _D10 * _p1;					// R&D expense
		_L1rd = _RD0 / w;						// workers in R&D

		// accumulate capital costs
		NW1 += _NW1 = mult * _NW10;
		Deb1 += _Deb1 = _NW1 * Deb10ratio;
		Eq1 += _Eq1 = _NW1 * ( 1 - Deb10ratio );

		// initialize variables
		WRITES( firm, "_Eq1", _Eq1 );
		WRITES( firm, "_t1ent", _t1ent );
		WRITELLS( firm, "_AtauEE", _AtauEE, _t1ent, 1 );
		WRITELLS( firm, "_AtauEF", _AtauEF, _t1ent, 1 );
		WRITELLS( firm, "_AtauLP", _AtauLP, _t1ent, 1 );
		WRITELLS( firm, "_BtauEE", _BtauEE, _t1ent, 1 );
		WRITELLS( firm, "_BtauEF", _BtauEF, _t1ent, 1 );
		WRITELLS( firm, "_BtauLP", _BtauLP, _t1ent, 1 );
		WRITELLS( firm, "_f1", _f1, _t1ent, 1 );
		WRITELLS( firm, "_p1", _p1, _t1ent, 1 );
		WRITELLS( firm, "_qc1", 4, _t1ent, 1 );

		if ( newInd )
		{
			WRITELLS( firm, "_Deb1", _Deb1, _t1ent, 1 );
			WRITELLS( firm, "_L1rd", _L1rd, _t1ent, 1 );
			WRITELLS( firm, "_NW1", _NW1, _t1ent, 1 );
			WRITELLS( firm, "_RD", _RD0, _t1ent, 1 );
			WRITELLS( firm, "_cTau", w / _cTau, _t1ent, 1 );
		}
		else
		{
			WRITES( firm, "_AtauEE", _AtauEE );
			WRITES( firm, "_AtauEF", _AtauEF );
			WRITES( firm, "_AtauLP", _AtauLP );
			WRITES( firm, "_BtauEE", _BtauEE );
			WRITES( firm, "_BtauEF", _BtauEF );
			WRITES( firm, "_BtauLP", _BtauLP );
			WRITES( firm, "_Deb1", _Deb1 );
			WRITES( firm, "_L1rd", _L1rd );
			WRITES( firm, "_NW1", _NW1 );
			WRITES( firm, "_RD", _RD0 );
			WRITES( firm, "_c1", _c1 );
			WRITES( firm, "_cTau", _cTau );
			WRITES( firm, "_p1", _p1 );

			// compute variables requiring calculation in t
			RECALCS( firm, "_Deb1max" );		// prudential credit limit
			RECALCS( firm, "_NC" );				// set initial clients
			VS( firm, "_CS1a" );				// update credit supply
		}
	}

	if ( newInd )								// set t=0 values
	{
		WRITELL( "Deb1", Deb1, _t1ent, 1 );
		WRITELL( "Eq1", Eq1, _t1ent, 1 );
		WRITELL( "NW1", NW1, _t1ent, 1 );
	}
	else										// just account new equity
	{
		INCR( "Eq1", Eq1 );
		INCR( "cEntry1", Eq1 );
	}

	return Eq1;
}


// add and configure entrant consumer-good firm object(s) and required hooks
// in equations 'entry2exit' and 'initCountry'

CFUN_DBL( entry_firm2, int n, bool newInd )
{
	double _A2, _D20, _D2e, _Deb2, _E, _Eq2, _K, _L2, _N, _NW2, _NW2f, _NW20,
		   _Q2u, _c2, _dD20, _f2, _life2cycle, _p2, Deb2, Eq2, K, N, NW2, mult;
	int _ID2, _t2ent;
	object *firm, *bank, *suppl,
		   *cap = V_EXTS( PARENT, countryE, capSec ),
		   *ene = V_EXTS( PARENT, countryE, eneSec ),
		   *lab = V_EXTS( PARENT, countryE, labSup );

	double Deb20ratio = V( "Deb20ratio" );		// bank fin. to equity ratio
	double Phi1 = V( "Phi1" );					// lower support for K share
	double Phi2 = V( "Phi2" );					// upper support for K share
	double iota = V( "iota" );					// desired inventories factor
	double mu20 = V( "mu20" );					// initial mark-up in sector 2
	double m2 = V( "m2" );						// machine output per period
	double p10 = VLS( cap, "p1avg", 1 );		// initial machine price
	double pE = VS( ene, "pE" );				// energy price
	double trCO2 = VS( PARENT, "trCO2" );		// carbon tax rate
	double u = V( "u" );						// desired capital utilization
	double w = VS( lab, "w" );					// current wage

	if ( newInd )
	{
		_D20 = CFUN( init_cond, "D20" ) / n;	// steady-state equil. demand
		_E = CFUN( init_cond, "E0" );			// initial competitiveness
		_K = CFUN( init_cond, "K0" ) / n;		// initial capital in sector 2
		_L2 = CFUN( init_cond, "L20" ) / n;		// initial labor in sector 2
		_N = CFUN( init_cond, "N0" ) / n;		// initial inventories
		_NW20 = V( "NW20" );					// initial wealth in sector 2
		_Q2u = 1;								// initial capacity utilization
		_dD20 = VS( PARENT, "dGDP0" );			// pre-initialization growth
		_f2 = 1.0 / n;							// fair share
		_life2cycle = 1;						// start as incumbent
		_t2ent = 0;								// entered before t=1
	}
	else
	{
		_D20 = 0;
		_E = V( "Eavg" );						// average competitiveness
		_K = WHTAVE( "_K", "_f2" );				// w. avg. capital in sector 2
		_L2 = 0;								// labor employed
		_N = 0;									// inventories
		_NW20 = WHTAVE( "_NW2", "_f2" );		// average wealth in sector 2
		_Q2u = V( "Q2u" );						// capacity utilization
		_dD20 = 0;
		_f2 = 0;								// no market share
		_life2cycle = 0;						// start as pre-operat. entrant
		_t2ent = T;								// entered now
	}

	// add entrant firms (end of period, don't try to sell)
	for ( Deb2 = Eq2 = NW2 = K = N = 0; n > 0; --n )
	{
		// create object, only recalculate in t if new industry
		if ( newInd )
			firm = ADDOBJL( "Firm2", T - 1 );
		else
			firm = ADDOBJ( "Firm2" );

		_ID2 = ID( 2, INCR( "lastID2", 1 ) );	// new firm ID
		WRITES( firm, "_ID2", _ID2 );

		ADDHOOKS( firm, FIRM2HK );				// add object hooks
		DELETE( SEARCHS( firm, "Vint" ) );		// remove empty instances
		DELETE( SEARCHS( firm, "Broch" ) );

		// select associated bank
		bank = CFUNS( firm, set_bank );

		// select initial machine supplier
		suppl = CFUNS( firm, set_supplier );

		// initial desired capital/expected demand, rounded to # of machines
		mult = newInd ? 1 : uniform( Phi1, Phi2 );// capital multiple
		K += _K = ceil( max( mult * _K / m2, 1 ) ) * m2;
		_D2e = newInd ? _D20 : u * _K;
		N += _N;

		// define entrant initial free cash (1 period wages or default minimum)
		if ( newInd )
		{
			mult = 1;							// NW multiple
			_A2 = CFUN( init_cond, "AlpIni" );	// initial labor productivity
			_c2 = CFUN( init_cond, "c20" );		// initial unit costs
		}
		else
		{
			mult = uniform( Phi1, Phi2 );
			_A2 = VS( suppl, "_AtauLP" );
			_c2 = VS( suppl, "_cTau" );
		}

		_p2 = ( 1 + mu20 ) * _c2;				// initial price
		_NW2f = ( 1 + iota ) * _D2e * _c2;		// initial free cash
		_NW2f = max( _NW2f, mult * _NW20 );

		// initial equity must pay initial capital and wages
		NW2 += _NW2 = newInd ? _NW2f : VS( suppl, "_p1" ) * _K / m2 + _NW2f;
		Deb2 += _Deb2 = _NW2 * Deb20ratio;
		Eq2 += _Eq2 = _NW2 * ( 1 - Deb20ratio );// accumulated equity (all firms)

		// initialize variables
		WRITES( firm, "_Eq2", _Eq2 );
		WRITES( firm, "_t2ent", _t2ent );
		WRITES( firm, "_life2cycle", _life2cycle );
		WRITELLS( firm, "_A2", _A2, _t2ent, 1 );
		WRITELLS( firm, "_L2", _L2, _t2ent, 1 );
		WRITELLS( firm, "_f2", _f2, _t2ent, 1 );
		WRITELLS( firm, "_f2", _f2, _t2ent, 2 );
		WRITELLS( firm, "_mu2", mu20, _t2ent, 1 );
		WRITELLS( firm, "_p2", _p2, _t2ent, 1 );
		WRITELLS( firm, "_qc2", 4, _t2ent, 1 );

		for ( int i = 1; i <= 4; ++i )
		{
			WRITELLS( firm, "_D2", _D2e * ( 1 - _dD20 * ( i - 1 ) ), _t2ent, i );
			WRITELLS( firm, "_D2d", _D2e * ( 1 - _dD20 * ( i - 1 ) ), _t2ent, i );
		}

		if ( newInd )
		{
			WRITELLS( firm, "_D2e", _D2e, _t2ent, 1 );
			WRITELLS( firm, "_Deb2", _Deb2, _t2ent, 1 );
			WRITELLS( firm, "_K", _K, _t2ent, 1 );
			WRITELLS( firm, "_N", _N, _t2ent, 1 );
			WRITELLS( firm, "_NW2", _NW2, _t2ent, 1 );

			CFUNS( firm, add_vintage, _K / m2, newInd );// first machine vintages
		}
		else
		{
			WRITES( firm, "_A2", _A2 );
			WRITES( firm, "_D2e", _D2e );
			WRITES( firm, "_Deb2", _Deb2 );
			WRITES( firm, "_E", _E );
			WRITES( firm, "_Kd", _K );
			WRITES( firm, "_NW2", _NW2 );
			WRITES( firm, "_Q2u", _Q2u );
			WRITES( firm, "_c2", _c2 );
			WRITES( firm, "_c2e", _c2 );
			WRITES( firm, "_mu2", mu20 );
			WRITES( firm, "_p2", _p2 );

			// compute variables requiring calculation in t
			RECALCS( firm, "_Deb2max" );		// prudential credit limit
			VS( firm, "_CS2a" );				// update credit supply
		}
	}

	if ( newInd )								// set t=0 values
	{
		WRITELL( "Deb2", Deb2, _t2ent, 1 );
		WRITELL( "Eq2", Eq2, _t2ent, 1 );
		WRITELL( "K", K, _t2ent, 1 );
		WRITELL( "N", N, _t2ent, 1 );
		WRITELL( "NW2", NW2, _t2ent, 1 );
	}
	else										// just account new equity
	{
		INCR( "Eq2", Eq2 );
		INCR( "cEntry2", Eq2 );
	}

	return Eq2;
}


// add and configure entrant energy producer firm object(s) and required hooks
// in equations 'entryEexit' and 'initCountry'

CFUN_DBL( entry_firmE, int n, bool newInd )
{
	double _AtauDE, _DeE, _DebE, _EqE, _ICtauGE, _Kde, _Kge, _KgeD, _NWe, _emTauDE,
		   _fE, _fKge, _muE, _pE, _p1, _rEdeb, AtauDEmax, AtauDEmin, DebE, EqE,
		   ICtauGEmax, ICtauGEmin, Kde, Kge, NWe, NWe0, emTauDEavg, mult;
	int _IDe, _nMach, _tEent;
	object *firm, *bank, *plant, *suppl,
		   *cap = V_EXTS( PARENT, countryE, capSec ),
		   *cons = V_EXTS( PARENT, countryE, conSec ),
		   *fin = V_EXTS( PARENT, countryE, finSec ),
		   *lab = V_EXTS( PARENT, countryE, labSup );

	double DebE0ratio = V( "DebE0ratio" );		// bank fin. to equity ratio
	double Phi5 = V( "Phi5" );					// lower support for wealth share
	double Phi6 = V( "Phi6" );					// upper support for wealth share
	double alpha3 = V( "alpha3" );				// lower support for imitation
	double beta3 = V( "beta3" );				// upper support for imitation
	double bE = V( "bE" );						// required payback period
	double fGE0 = V( "fGE0" );					// initial green energy share
	double iotaE = V( "iotaE" );				// planned reserve capacity
	double kConst = VS( fin, "kConst" );		// debt interest scale factor
	double muE0 = V( "muE0" );					// initial markup wage multiple
	double pF = V( "pF" );						// fossil fuel price
	double x6 = V( "x6" );						// entrant upper advantage
	double w0 = VLS( lab, "w", 1 );				// initial wage
	int flagEnClim = VS( PARENT, "flagEnClim" );// energy enable flag

	int _qc0 = 4;								// start at end of pecking order

	if ( newInd )
	{
		_AtauDE = V( "Ade0" );					// ini. efficiency dirty plant
		_DeE = CFUN( init_cond, "De0" ) / n;	// steady-state equil. demand
		_ICtauGE = V( "ICge0" );				// initial green plant unit cost
		_emTauDE = V( "emDE0" );				// initial emissions dirty plant
		_fE = 1.0 / n;							// fair share
		_fKge = V( "fGE0" );					// initial share of green plants
		_muE = muE0 * w0;						// mark-up floor
		_pE = CFUN( init_cond, "pE0" );			// initial price
		_rEdeb = VLS( fin, "rDeb", 1 ) * ( 1 + ( _qc0 - 1 ) * kConst );// interest
		_tEent = 0;								// entered before t=1
		NWe0 = V( "NWe0" );						// initial wealth in energy sec.
	}
	else
	{
		_DeE = VS( PARENT, "En" ) / V( "Fe" );	// fair share
		_emTauDE = AVE( "_emTauDE" );			// average dirty energy emissions
		_fE = 0;								// no market share
		_fKge = WHTAVE( "_fKge", "_fE" );		// average share of green plants
		_muE = muE0 * VLS( lab, "wReal", 1 );	// mark-up floor
		_pE = WHTAVE( "_pE", "_fE" );			// average power price
		_rEdeb = VS( fin, "rDeb" ) * ( 1 + ( _qc0 - 1 ) * kConst );// interest
		_tEent = T;								// entered now
		AtauDEmax = MAX( "_AtauDE" );			// max dirty energy efficiency
		AtauDEmin = MIN( "_AtauDE" );			// min dirty energy efficiency
		ICtauGEmax = MAX( "_ICtauGE" );			// max green plant cost
		ICtauGEmin = MIN( "_ICtauGE" );			// min green plant cost
		NWe0 = max( WHTAVE( "_NWe", "_fE" ), V( "NWe0" ) );
	}

	// add entrant firms (end of period, don't try to sell)
	for ( DebE = EqE = NWe = Kde = Kge = 0; n > 0; --n )
	{
		// create object, only recalculate in t if new industry
		if ( newInd )
			firm = ADDOBJL( "FirmE", T - 1 );
		else
			firm = ADDOBJ( "FirmE" );

		_IDe = ID( 3, INCR( "lastIDe", 1 ) );// new firm ID
		WRITES( firm, "_IDe", _IDe );

		ADDHOOKS( firm, FIRMEHK );				// add object hooks
		WRITE_HOOKS( firm, TOPVINT, NULL );		// no green plant yet
		DELETE( SEARCHS( firm, "Dirty" ) );		// remove empty instances
		DELETE( SEARCHS( firm, "Green" ) );
		DELETE( SEARCHS( firm, "BrE" ) );

		// select associated bank
		bank = CFUNS( firm, set_bank );

		// select initial machine supplier
		suppl = CFUNS( firm, set_supplier );

		if ( ! newInd )
		{
			// draw initial dirty efficiency/green cost (imitation from best firm)
			_AtauDE = AtauDEmin + ( AtauDEmax - AtauDEmin ) *
								  ( 1 + x6 ) * beta( alpha3, beta3 );
			_ICtauGE = ICtauGEmax - ( ICtauGEmax - ICtauGEmin ) *
									( 1 + x6 ) * beta( alpha3, beta3 );
		}

		// initial capital in power plants
		if ( flagEnClim > 0 )
		{
			// define entrant initial free cash (wages + initial green investment)
			mult = newInd ? 1 : uniform( Phi5, Phi6 );// NW multiple
			_NWe = mult * NWe0;					// add initial free cash

			if ( fGE0 * _DeE >= 1 )
			{
				_KgeD = ( 1 + iotaE ) * fGE0 * _DeE;// desired green plant cap.
				_p1 = VS( suppl, "_p1" );		// capital unit (machine) price
				_nMach = ceil( _ICtauGE * _KgeD / _p1 );// rounded capital units

				if ( newInd )					// install plants now?
					Kge += _Kge = _KgeD;		// installed green capital
				else
				{
					_Kge = 0;					// no green capital yet
					_NWe += _p1 * _nMach;		// cash to finance green plant
				}
			}
			else
				_Kge = _KgeD = 0;				// no green plant

			Kde += _Kde = ( 1 + iotaE ) * _DeE - _KgeD;// dirty capital.

			// initial equity must pay initial capital and wages
			NWe += _NWe;
			DebE += _DebE = _NWe * DebE0ratio;
			EqE += _EqE = _NWe * ( 1 - DebE0ratio );

		}
		else
			_muE = _Kge = _Kde = _NWe = _DebE = _EqE = 0;

		// initialize variables
		WRITES( firm, "_EqE", _EqE );
		WRITES( firm, "_tEent", _tEent );
		WRITELLS( firm, "_AtauDE", _AtauDE, _tEent, 1 );
		WRITELLS( firm, "_DeE", _DeE, _tEent, 1 );
		WRITELLS( firm, "_ICtauGE", _ICtauGE, _tEent, 1 );
		WRITELLS( firm, "_emTauDE", _emTauDE, _tEent, 1 );
		WRITELLS( firm, "_fE", _fE, _tEent, 1 );
		WRITELLS( firm, "_fE", _fE, _tEent, 2 );
		WRITELLS( firm, "_fKge", _fKge, _tEent, 1 );
		WRITELLS( firm, "_muE", _muE, _tEent, 1 );
		WRITELLS( firm, "_pE", _pE, _tEent, 1 );
		WRITELLS( firm, "_qcE", _qc0, _tEent, 1 );

		if ( newInd )
		{
			WRITELLS( firm, "_DebE", _DebE, _tEent, 1 );
			WRITELLS( firm, "_Ke", _Kge + _Kde, _tEent, 1 );
			WRITELLS( firm, "_Kde", _Kde, _tEent, 1 );
			WRITELLS( firm, "_Kge", _Kge, _tEent, 1 );
			WRITELLS( firm, "_NWe", _NWe, _tEent, 1 );

			if ( _Kge > 0 )						// first green plant
				CFUNS( firm, add_plant, GREEN, _Kge, _nMach, true );

			if ( _Kde > 0 )						// first dirty plant
				CFUNS( firm, add_plant, DIRTY, _Kde, 0, true );
		}
		else
		{
			WRITES( firm, "_AtauDE", _AtauDE );
			WRITES( firm, "_Ade", _AtauDE );
			WRITES( firm, "_DeE", _DeE );
			WRITES( firm, "_DebE", _DebE );
			WRITES( firm, "_ICtauGE", _ICtauGE );
			WRITES( firm, "_NWe", _NWe );
			WRITES( firm, "_emTauDE", _emTauDE );
			WRITES( firm, "_fE", _fE );
			WRITES( firm, "_fKge", _fKge );
			WRITES( firm, "_muE", _muE );
			WRITES( firm, "_pE", _pE );
			WRITES( firm, "_rEdeb", _rEdeb );

			// compute variables requiring calculation in t
			RECALCS( firm, "_DebEmax" );		// prudential credit limit
			VS( firm, "_CSeA" );				// update credit supply
		}
	}

	if ( newInd )								// set t=0 values
	{
		WRITELL( "DebE", DebE, _tEent, 1 );
		WRITELL( "EqE", EqE, _tEent, 1 );
		WRITELL( "Ke", Kge + Kde, _tEent, 1 );
		WRITELL( "NWe", NWe, _tEent, 1 );
	}
	else										// just account new equity
	{
		INCR( "EqE", EqE );
		INCR( "cEntryE", EqE );
	}

	return EqE;
}


// remove firm object and existing hooks in equation 'entry1exit', 'entry2exit',
// entryEexit

const char *_BadDebVar[ ] = { "_BadDeb1", "_BadDeb2", "_BadDebE", "_BadDebGE" },
		   *_EqVar[ ] = { "_Eq1", "_Eq2", "_EqE" },
		   *EqVar[ ] = { "Eq1", "Eq2", "EqE" },
		   *cExitVar[ ] = { "cExit1", "cExit2", "cExitE" };

CFUN_DBL( exit_firm )
{
	double liqEq, liqVal, DebGE;
	object *bank, *cli;
	int sec = strcmp( NAME, "Firm1" ) == 0 ? 0 :
			  strcmp( NAME, "Firm2" ) == 0 ? 1 : 2;

	// remove equity from sector total
	INCRS( PARENT, EqVar[ sec ], - V( _EqVar[ sec ] ) );

	// account liquidation equity credit of shareholder or bad debt cost of bank
	liqVal = V( _NWvar[ sec ] ) - V( _DebVar[ sec ] );
	DebGE = sec == 2 ? V( "_DebGE" ) : 0;// green project finance

	if ( liqVal - DebGE < 0 )					// account bank losses, if any
	{
		liqEq = 0;								// no liquidation equity
		bank = HOOK( BANK );					// exiting firm bank
		VS( bank, _BadDebVar[ sec ] );			// ensure reset in t
		INCRS( bank, _BadDebVar[ sec ], - liqVal );// accumulate bank losses

		if ( sec == 2 )
		{
			VS( bank, _BadDebVar[ 3 ] );		// account project finance
			INCRS( bank, _BadDebVar[ 3 ], - DebGE );// separately
		}
	}
	else
	{
		liqEq = ROUND( liqVal - DebGE, 0, 0.01 );// liquidation equity credit
		INCRS( PARENT, cExitVar[ sec ], liqEq );
	}

	DELETE( HOOK( BCLIENT ) );					// leave client list of bank

	CYCLE( cli, CliBrochObj[ sec ] )			// leave 1st counterpart list
		DELETE( SHOOKS( cli ) );				// delete from counterpart lists

	if ( sec == 0 )
		CYCLE( cli, "CliEn" )					// leave 2nd counterpart list
			DELETE( SHOOKS( cli ) );			// delete from counterpart lists

	if ( sec == 1 )
		// update firm map before removing LSD object in consumption sector
		EXEC_EXTS( GRANDPARENT, countryE, firm2map, erase,
				   ( int ) V( _IDpar[ sec ] ) );

	if ( sec == 2 )
		// update firm map before removing LSD object in energy sector
		EXEC_EXTS( GRANDPARENT, countryE, firmEmap, erase,
				   ( int ) V( _IDpar[ sec ] ) );

	DELETE( THIS );

	return liqEq;
}


// steady-state initial conditions constants

CFUN_DBL( init_cond, const char *var )
{
	static bool computed = false;
	static strMapT v;

	if ( var == NULL )							// simulation reset?
		computed = false;

	if ( ! computed )
	{
		computed = true;

		// model parameters
		bool flagEnClim = V( "flagEnClim" );	// energy sector enabled?
		double Ade0 = VS( ENESECL0, "Ade0" );	// initial efficiency dirty plant
		double Deb10ratio = VS( CAPSECL0, "Deb10ratio" );// initial debt-to-equity
		double Deb20ratio = VS( CONSECL0, "Deb20ratio" );
		double DebE0ratio = VS( ENESECL0, "DebE0ratio" );
		double F10 = VS( CAPSECL0, "F10" );		// initial number of firms
		double F20 = VS( CONSECL0, "F20" );
		double Fe0 = VS( ENESECL0, "Fe0" );
		double Ls0 = VS( LABSUPL0, "Ls0" );		// initial labor supply
		double NW10 = VS( CAPSECL0, "NW10" );	// initial net worth (cash)
		double NW20 = VS( CONSECL0, "NW20" );
		double NWe0 = VS( ENESECL0, "NWe0" );
		double b = VS( CONSECL0, "b" );			// investment payback period
		double bE = VS( ENESECL0, "bE" );		// energy invest. payback period
		double dY0 = V( "dGDP0" );				// steady-state initial growth
		double emDE0 = VS( ENESECL0, "emDE0" );	// initial emissions coefficient
		double eta = VS( CONSECL0, "eta" );		// technical machine lifetime
		double etaE = VS( ENESECL0, "etaE" );	// technical power plant lifetime
		double fGE0 = VS( ENESECL0, "fGE0" );	// initial share of green energy
		double iota = VS( CONSECL0, "iota" );	// desired inventory factor
		double iotaE = VS( ENESECL0, "iotaE" );	// reserve plant capacity factor
		double m1 = VS( CAPSECL0, "m1" );		// output factor in capital sec.
		double m2 = VS( CONSECL0, "m2" );		// output factor in cons. sec.
		double mDE = VS( ENESECL0, "mDE" );		// labor intensity dirty energy
		double mGE = VS( ENESECL0, "mGE" );		// labor intensity green energy
		double mu1 = VS( CAPSECL0, "mu1" );		// mark-up in capital sector
		double mu20 = VS( CONSECL0, "mu20" );	// initial mark-up in cons. sec.
		double muE0 = VS( ENESECL0, "muE0" );	// initial mark-up in energy sec.
		double muBonds = VS( FINSECL0, "muBonds" );// mark-down interest on bonds
		double muD = VS( FINSECL0, "muD" );		// mark-down interest on deposits
		double muDeb = VS( FINSECL0, "muDeb" );	// mark-up interest on loan debt
		double muRes = VS( FINSECL0, "muRes" );	// mark-down interest on reserves
		double nu = VS( CAPSECL0, "nu" );		// R&D to sales ratio
		double nuE = VS( ENESECL0, "nuE" );		// R&D to energy sales ratio
		double omega1 = VS( CONSECL0, "omega1" );// price weight on competitiv.
		double omega2 = VS( CONSECL0, "omega2" );// delivery weight on competitiv.
		double pF0 = VS( ENESECL0, "pF0" );		// initial price of fossil fuel
		double phi = VS( LABSUPL0, "phi" );		// unemployment benefit rate
		double rT = VS( FINSECL0, "rT" );		// target prime interest rate
		double tr = V( "tr" );					// tax rate
		double trIn = V( "flagTax" ) > 0 ? tr : 0;// tax rate on income (wages)
		double trCO2 = V( "trCO2" );			// carbon tax rate
		double trCO2e = VS( ENESECL0, "trCO2e" );// carbon tax rate energy sec.
		double tA0 = VS( CLIMATL0, "tA0" );		// energy-source competition time
		double u = VS( CONSECL0, "u" );			// planned capital utilization

		// macro prices
		v[ "w0" ] = INIWAGE;
		v[ "r0" ] = rT;
		v[ "rBonds0" ] = ( 1 - muBonds ) * v[ "r0" ];
		v[ "rD0" ] = ( 1 - muD ) * v[ "r0" ];
		v[ "rDeb0" ] = ( 1 + muDeb ) * v[ "r0" ];
		v[ "rRes0" ] = ( 1 - muRes ) * v[ "r0" ];

		// technology and productivities
		v[ "AtauLP0" ] = INIPROD;
		v[ "AtauEE0" ] = INIEEFF * v[ "AtauLP0" ];
		v[ "AtauEF0" ] = INIEFRI * v[ "AtauLP0" ];
		v[ "BtauLP0" ] = v[ "AtauLP0" ] * ( 1 + mu1 ) / ( m1 * m2 * b );
		v[ "BtauEE0" ] = INIEEFF * v[ "AtauLP0" ];
		v[ "BtauEF0" ] = INIEFRI * v[ "AtauLP0" ];
		v[ "ICge0" ] = ( 1 + log( tA0 + 1 ) ) * bE * pF0 / Ade0;
		v[ "Tk0" ] = min( ceil( eta * u * ( 1 + dY0 ) ), eta );

		if ( dY0 == 0 )
			v[ "AlpIni" ] = v[ "AtauLP0" ];
		else
			v[ "AlpIni" ] = v[ "AtauLP0" ] / ( v[ "Tk0" ] + 1 ) *
							( 1 + ( 1 + dY0 ) *
							( 1 - pow( 1 + dY0, - v[ "Tk0" ] ) ) / dY0 );

		// costs and prices
		if ( flagEnClim )
		{
			v[ "cE0" ] = fGE0 * mGE * v[ "w0" ] + ( 1 - fGE0 ) *
												  ( pF0 / Ade0 + mDE * v[ "w0" ] );
			if ( fGE0 == 1 )
				v[ "pE0" ] = ( mGE + muE0 ) * v[ "w0" ];
			else
				v[ "pE0" ] = pF0 / Ade0 + ( mDE + muE0 ) * v[ "w0" ];
		}
		else
			v[ "cE0" ] = v[ "pE0" ] = 0;

		v[ "c10" ] = ( 1 / m1 ) * ( v[ "w0" ] / v[ "BtauLP0" ] +
									( v[ "pE0" ] + trCO2 * v[ "BtauEF0" ] ) /
									v[ "BtauEE0" ] );
		v[ "c20" ] = v[ "w0" ] / v[ "AlpIni" ] +
					 ( v[ "pE0" ] + trCO2 * v[ "AtauEF0" ] ) / v[ "AtauEE0" ];
		v[ "p10" ] = ( 1 + mu1 ) * v[ "c10" ];
		v[ "p20" ] = ( 1 + mu20 ) * v[ "c20" ];
		v[ "mE0" ] = v[ "p10" ] / v[ "ICge0" ];

		// steady state capital, demand, production and inventory
		v[ "delta20" ] = iota * ( 1 - 1 / ( 1 + dY0 ) ) + 1;
		v[ "rho20" ] = ( 1 + dY0) * u / v[ "AlpIni" ];

		if ( flagEnClim )
			v[ "thetaE0" ] = etaE * v[ "mE0" ] * ( 1 + iotaE ) *
							 ( v[ "delta20" ] * v[ "AtauEE0" ] +
							   ( 1 + dY0 ) * u * eta * m1 * m2 * v[ "BtauEE0" ] ) /
							 ( eta * m2 * v[ "delta20" ] * v[ "AtauEE0" ] *
							   ( etaE * m1 * v[ "mE0" ] * v[ "BtauEE0" ] -
								 ( 1 + iotaE ) * fGE0 ) );
		else
			v[ "thetaE0" ] = 0;

		v[ "psi10" ] = 1 / ( eta * m2 ) +
					   v[ "thetaE0" ] * fGE0 / ( etaE * v[ "mE0" ] );
		v[ "kappa10" ] = nu * v[ "p10" ] / v[ "w0" ] +
						 1 / ( m1 * v[ "BtauLP0" ] );
		v[ "lambdaE0" ] = ( 1 + dY0 ) *
						  ( nuE * v[ "pE0" ] / ( v[ "w0" ] * ( 1 + iotaE ) ) +
							mDE * ( 1 - fGE0 ) + mGE * fGE0 );
		v[ "K0" ] = phi * Ls0 / ( ( 1 + dY0 ) * u * v[ "p20" ] /
									( v[ "w0" ] * v[ "delta20" ] ) -
								  ( 1 - phi - trIn ) *
								  ( v[ "kappa10" ] * v[ "psi10" ] + v[ "rho20" ] +
									v[ "thetaE0" ] * v[ "lambdaE0" ] ) );
		v[ "Ke0" ] = v[ "thetaE0" ] * v[ "K0" ];
		v[ "De0" ] = ( 1 + dY0 ) / ( 1 + iotaE ) * v[ "Ke0" ];
		v[ "Df0" ] = ( 1 - fGE0 ) / Ade0 * v[ "De0" ];
		v[ "D10" ] = v[ "Q10" ] = v[ "psi10" ] * v[ "K0" ];
		v[ "D20" ] = u * ( 1 + dY0 ) / v[ "delta20" ] * v[ "K0" ];
		v[ "Qe0" ] = ( 1 + iotaE ) * v[ "De0" ];
		v[ "Q20" ] = v[ "delta20" ] * v[ "D20" ];
		v[ "N0" ] = iota * v[ "D20" ];

		// labor employed and real wage
		v[ "L0" ] = ( v[ "kappa10" ] * v[ "psi10" ] + v[ "rho20" ] +
					  v[ "lambdaE0" ] * v[ "thetaE0" ] ) * v[ "K0" ];
		v[ "Le0" ] = v[ "lambdaE0" ] * v[ "thetaE0" ] * v[ "K0" ];
		v[ "L10" ] = v[ "kappa10" ] * v[ "psi10" ] * v[ "K0" ];
		v[ "L20" ] = v[ "rho20" ] * v[ "K0" ];
		v[ "U0" ] = 1 - min( v[ "L0" ] / Ls0, 1 );
		v[ "wReal0" ] = v[ "w0" ] / v[ "p20" ];

		// energy consumption and CO2 emissions
		v[ "En10" ] = v[ "D10" ] / ( m1 * v[ "BtauEE0" ] );
		v[ "En20" ] = v[ "D20" ] / v[ "AtauEE0" ];
		v[ "En0" ] = v[ "En10" ] + v[ "En20" ];
		v[ "Em10" ] = v[ "En10" ] * v[ "BtauEF0" ];
		v[ "Em20" ] = v[ "En20" ] * v[ "AtauEF0" ];
		v[ "EmE0" ] = v[ "Df0" ] * emDE0;
		v[ "Em0" ] = v[ "Em10" ] + v[ "Em20" ] + v[ "EmE0" ];

		// firm net worth, loan debt and profits
		v[ "NW10" ] = NW10 * F10;
		v[ "NW20" ] = NW20 * F20;
		v[ "NWe0" ] = NWe0 * Fe0;
		v[ "Deb10" ] = v[ "NW10" ] * Deb10ratio;
		v[ "Deb20" ] = v[ "NW20" ] * Deb20ratio;
		v[ "DebE0" ] = v[ "NWe0" ] * DebE0ratio;
		v[ "Pi10" ] = v[ "p10" ] * v[ "D10" ] - v[ "w0" ] * v[ "L10" ] -
					  v[ "pE0" ] * v[ "En10" ] - trCO2 * v[ "Em10" ] +
					  v[ "rD0" ] * v[ "NW10" ] - v[ "rDeb0" ] * v[ "Deb10" ];
		v[ "Pi20" ] = v[ "p20" ] * v[ "D20" ] - v[ "w0" ] * v[ "L20" ] -
					  v[ "pE0" ] * v[ "En20" ] - trCO2 * v[ "Em20" ] +
					  v[ "rD0" ] * v[ "NW20" ] - v[ "rDeb0" ] * v[ "Deb20" ];
		v[ "PiE0" ] = v[ "pE0" ] * v[ "De0" ] - v[ "w0" ] * v[ "Le0" ] -
					  pF0 * v[ "Df0" ] - trCO2e * v[ "EmE0" ] +
					  v[ "rD0" ] * v[ "NWe0" ] - v[ "rDeb0" ] * v[ "DebE0" ];
		v[ "PiB0" ] = ( v[ "Deb10" ] + v[ "Deb20" ] + v[ "DebE0" ] ) * v[ "rDeb0" ] -
					 ( v[ "NW10" ] + v[ "NW20" ] + v[ "NWe0" ] ) * v[ "rD0" ];

		// investment, consumption and GDP
		v[ "InomE0" ] = v[ "ICge0" ] * fGE0 * v[ "Ke0" ] / etaE;
		v[ "Inom0" ] = v[ "p10" ] * v[ "D10" ];
		v[ "C0" ] = v[ "p20" ] * v[ "D20" ];
		v[ "GDPnom0" ] = v[ "C0" ] + v[ "Inom0" ];

		// government expenditure, tax revenue and budget deficit
		v[ "G0" ] = phi * v[ "w0" ] * Ls0 * v[ "U0" ];
		v[ "Tax0" ] = ( v[ "Pi10" ] + v[ "Pi20" ] + v[ "PiE0" ] + v[ "PiB0" ] ) * tr;
		v[ "Def0" ] = v[ "G0" ] - v[ "Tax0" ];

		// productivity and competitiveness
		v[ "A0" ] = v[ "GDPnom0" ] / ( v[ "L10" ] + v[ "L20" ] );
		v[ "Ae0" ] = v[ "Le0" ] > 0 ? v[ "De0" ] / v[ "Le0" ] :
									  fGE0 / mDE + ( 1 - fGE0 ) / mGE;
		v[ "E0" ] = - omega1 - omega2;
	}

	if ( var != NULL && v.find( var ) != v.end( ) )
		return v[ var ];
	else
		return NAN;
}
