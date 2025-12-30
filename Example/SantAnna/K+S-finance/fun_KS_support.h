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


// set initial bank for entrant in equations 'entry1exit', 'entry2exit'

const char *bankPar[ ] = { "_bank1", "_bank2" },
		   *CliObj[ ] = { "Cli1", "Cli2" },
		   *_IDpar[ ] = { "_ID1","_ID2" },
		   *__IDpar[ ] = { "__ID1","__ID2" };

CFUN_OBJ( set_bank )
{
	int _IDb, sec = strcmp( NAME, "Firm1" ) == 0 ? 0 : 1;
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


// update firm debt in equations '_Q1', '_Tax1', '_Q2', '_EI', '_SI', '_Tax2'

const char *_CDvar[ ] = { "_CD1", "_CD2" },
		   *_CDcVar[ ] = { "_CD1c", "_CD2c" },
		   *_CSvar[ ] = { "_CS1", "_CS2" },
		   *_DebVar[ ] = { "_Deb1", "_Deb2" },
		   *_NWvar[ ] = { "_NW1", "_NW2" },
		   *_TCfreeVar[ ] = { "_TC1free", "_TC2free" };

CFUN_DBL( update_debt, double desired, double loan )
{
	double Deb, TCfree;
	object *bank;
	int sec = strcmp( NAME, "Firm1" ) == 0 ? 0 : 1;

	if ( desired > 0 )							// ignore loan repayment
	{
		INCR( _CDvar[ sec ], desired );			// desired credit
		INCR( _CDcVar[ sec ], desired - loan );	// credit constraint
		INCR( _CSvar[ sec ], loan );			// supplied credit
	}

	Deb = V( _DebVar[ sec ] );

	// take new loan/repay debt from/to bank
	if ( loan != 0 )
	{
		if ( Deb + loan < 0.001 )				// write-off small debt?
			Deb = WRITE( _DebVar[ sec ], 0 );
		else
			Deb = INCR( _DebVar[ sec ], loan );

		bank = HOOK( BANK );					// firm's bank

		// if credit limit active, adjust bank's available credit
		TCfree = VS( bank, _TCfreeVar[ sec ] );	// available credit firm's bank
		if ( TCfree > -0.1 )
			WRITES( bank, _TCfreeVar[ sec ], max( TCfree - loan, 0 ) );
	}

	return Deb;
}


// update firm deposits in equations '_Q1', '_Tax1', '_Q2', '_EI', '_SI', '_Tax2'

CFUN_DBL( update_depo, double depo, bool incr )
{
	double NW;
	int sec = strcmp( NAME, "Firm1" ) == 0 ? 0 : 1;

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


// manage firm cash flow in equations '_Tax1', '_Tax2'

const char *_CIvar[ ] = { "", "_CI" },
		   *_CSaVar[ ] = { "_CS1a", "_CS2a" },
		   *_DivVar[ ] = { "_Div1", "_Div2" },
		   *_NWpVar[ ] = { "_NW1p", "_NW2p" };

CFUN_DBL( cash_flow, double profit, double tax )
{
	int sec = strcmp( NAME, "Firm1" ) == 0 ? 0 : 1;
	object *fin = V_EXTS( GRANDPARENT, countryE, finSec );

	double dividends = VL( _DivVar[ sec ], 1 );// shareholder dividends
	double cashFree = profit - tax - dividends;	// final free cash flow

	if ( sec > 0 )
		V( _CIvar[ sec ] );						// ensure canc. invest. reimbursed

	double provision = V( _NWpVar[ sec ] );		// prod. cost
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

CFUN_OBJ( send_brochure, object *client )
{
	object *broch, *cli;

	cli = ADDOBJ( "Cli" );						// add object to new client
	WRITES( cli, "__IDc", VS( client, "_ID2" ) );// client ID
	WRITES( cli, "__tSel", T );					// update selection time

	broch = ADDOBJS( client, "Broch" );			// add brochure to client
	WRITES( broch, "__IDs", V( "_ID1" ) );		// supplier ID
	WRITE_SHOOKS( broch, cli );					// pointer to supplier client list
	WRITE_SHOOKS( cli, broch );					// pointer to client brochure list

	return broch;
}


// set initial supplier for entrant in equations 'entry2exit'

CFUN_OBJ( set_supplier )
{
	object *broch, *suppl,
		   *cap = V_EXTS( GRANDPARENT, countryE, capSec );

	suppl = RNDDRAWS( cap, "Firm1", "_Atau" );	// draw capital supplier
	broch = CFUNS( suppl, send_brochure, THIS );// get supplier brochure
	WRITE_HOOK( SUPPL, broch );					// pointer to current supplier
	INCRS( suppl, "_NC", 1 );					// update supplier's clients #

	return suppl;
}


// send new machine order in equations '_EI', '_SI'

CFUN_VOID( send_order, double nMach )
{
	// find firm entry on supplier client list
	object *cli = SHOOKS( HOOK( SUPPL ) );

	if ( VS( cli, "__tOrd" ) < T )				// if first order in period
	{
		WRITES( cli, "__nOrd", nMach );			// set new order size
		WRITES( cli, "__tOrd", T );				// set order time
		WRITES( cli, "__nCan", 0 );				// no machine canceled yet
	}
	else
		INCRS( cli, "__nOrd", nMach );			// increase existing order size
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
	double __Avint, __pVint;
	int __ageVint, __nMach, __nVint;
	object *cap, *cons, *cur, *suppl, *vint;

	suppl = PARENTS( SHOOKS( HOOK( SUPPL ) ) );	// current supplier
	__nMach = floor( nMach );					// integer number of machines

	// at t=1 firms have a mix of machines: old to new, many suppliers
	if ( newInd )
	{
		cap = V_EXTS( GRANDPARENT, countryE, capSec );
		cons = V_EXTS( GRANDPARENT, countryE, conSec );

		__ageVint = VS( cons, "eta" ) + 1;		// age of oldest machine
		__nVint = ceil( nMach / __ageVint );	// machines per vintage
		__Avint = INIPROD;						// initial product. in sector 2
		__pVint = VLS( cap, "p1avg", 1 );		// initial machine price
	}
	else
	{
		__ageVint = 1 - T;
		__nVint = __nMach;
		__Avint = VS( suppl, "_Atau" );
		__pVint = VS( suppl, "_p1" );
	}

	while ( __nMach > 0 )
	{
		if ( newInd )
		{
			cur = RNDDRAW_FAIRS( cap, "Firm1" );// draw another supplier
			if ( cur == suppl )					// don't use current supplier
				continue;

			vint = ADDOBJL( "Vint", T - 1 );	// recalculate in t=1
		}
		else
		{
			cur = suppl;						// just use current supplier
			vint = ADDOBJ( "Vint" );			// just recalculate in next t
		}

		WRITE_SHOOKS( vint, HOOK( TOPVINT ) );	// save previous vintage
		WRITE_HOOK( TOPVINT, vint );			// save pointer to top vintage

		WRITES( vint, "__IDvint", VNT( T, VS( cur, "_ID1" ) ) );// vintage ID
		WRITES( vint, "__Avint", __Avint );		// vintage productivity
		WRITES( vint, "__nVint", __nVint );		// number of machines in vintage
		WRITES( vint, "__pVint", __pVint );		// price of machines in vintage
		WRITES( vint, "__tVint", 1 - __ageVint );// vintage build time

		__nMach -= __nVint;
		--__ageVint;

		if ( __ageVint > 0 && __nMach % __ageVint == 0 )// exact ratio missing?
			__nVint = __nMach / __ageVint;		// adjust machines per vintage
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


/*=================== FIRM ENTRY-EXIT SUPPORT C FUNCTIONS ====================*/

// add and configure entrant capital-good firm object(s) and required hooks
// in equations 'entry1exit' and 'initCountry'

CFUN_DBL( entry_firm1, int n, bool newInd )
{
	double _Atau, _Btau, _D10, _Deb1, _Eq1, _L1rd, _NW1, _NW10, _RD0, _c1, _f1,
		   _p1, AtauMax, BtauMax, Deb1, Eq1, NW1, mult;
	int _ID1, _t1ent;
	object *firm, *bank,
		   *cons = V_EXTS( PARENT, countryE, conSec ),
		   *lab = V_EXTS( PARENT, countryE, labSup );

	double Deb10ratio = V( "Deb10ratio" );		// bank fin. to equity ratio
	double Phi3 = V( "Phi3" );					// lower support for wealth share
	double Phi4 = V( "Phi4" );					// upper support for wealth share
	double alpha2 = V( "alpha2" );				// lower support for imitation
	double beta2 = V( "beta2" );				// upper support for imitation
	double mu1 = V( "mu1" );					// mark-up in sector 1
	double m1 = V( "m1" );						// worker production scale
	double nu = V( "nu" );						// share of R&D expenses
	double x5 = V( "x5" );						// entrant upper advantage
	double w = VS( lab, "w" );					// current wage

	if ( newInd )
	{
		double F2 = VS( cons, "F2" );
		double m2 = VS( cons, "m2" );			// machine output per period

		_Atau = AtauMax = INIPROD;				// initial productivities to use
		_Btau = BtauMax = ( 1 + mu1 ) * _Atau / ( m1 * m2 * VS( cons, "b" ) );
												// and build machines (s. s.)
		_NW10 = V( "NW10" );					// initial wealth in sector 1
		_f1 = 1.0 / n;							// fair share
		_t1ent = 0;								// entered before t=1

		// initial demand expectation, assuming all sector 2 firms, 1/eta
		// replacement factor and fair share in sector 1 and full employment
		double p20 = VLS( cons, "CPI", 1 );
		double K0 = ceil( VS( lab, "Ls0" ) * INIWAGE / p20 / F2 / m2 ) * m2;

		_D10 = F2 * K0 / m2 / VS( cons, "eta" ) / n;
	}
	else
	{
		_NW10 = max( WHTAVE( "_NW1", "_f1" ), V( "NW10" ) *
					 V( "PPI" ) / V( "pK0" ) );
		_f1 = 0;								// no market share
		_t1ent = T;								// entered now
		AtauMax = MAX( "_Atau" );				// best machine productivity
		BtauMax = MAX( "_Btau" );				// best productivity in sector 1

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

		// select associated bank
		bank = CFUNS( firm, set_bank );

		if ( ! newInd )
		{
			// initial labor productivity (imitation from best firm)
			_Atau = beta( alpha2, beta2 );		// draw A from Beta(alpha,beta)
			_Atau *= AtauMax * ( 1 + x5 );		// fraction of top firm
			_Btau = beta( alpha2, beta2 );		// draw B from Beta(alpha,beta)
			_Btau *= BtauMax * ( 1 + x5 );		// fraction of top firm
		}

		// initial cost, price and net wealth
		mult = newInd ? 1 : uniform( Phi3, Phi4 );// NW multiple
		_c1 = w / ( _Btau * m1 );				// unit cost
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
		WRITELLS( firm, "_Atau", _Atau, _t1ent, 1 );
		WRITELLS( firm, "_Btau", _Btau, _t1ent, 1 );
		WRITELLS( firm, "_f1", _f1, _t1ent, 1 );
		WRITELLS( firm, "_p1", _p1, _t1ent, 1 );
		WRITELLS( firm, "_qc1", 4, _t1ent, 1 );

		if ( newInd )
		{
			WRITELLS( firm, "_Deb1", _Deb1, _t1ent, 1 );
			WRITELLS( firm, "_L1rd", _L1rd, _t1ent, 1 );
			WRITELLS( firm, "_NW1", _NW1, _t1ent, 1 );
			WRITELLS( firm, "_RD", _RD0, _t1ent, 1 );
			WRITELLS( firm, "_cTau", w / _Atau, _t1ent, 1 );
		}
		else
		{
			WRITES( firm, "_Atau", _Atau );
			WRITES( firm, "_Btau", _Btau );
			WRITES( firm, "_Deb1", _Deb1 );
			WRITES( firm, "_L1rd", _L1rd );
			WRITES( firm, "_NW1", _NW1 );
			WRITES( firm, "_RD", _RD0 );
			WRITES( firm, "_c1", _c1 );
			WRITES( firm, "_cTau", w / _Atau );
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
	double _A2, _D20, _D2e, _Deb2, _E, _Eq2, _K, _N, _NW2, _NW2f, _NW20, _Q2u,
		   _c2, _f2, _life2cycle, _p2, Deb2, Eq2, K, N, NW2, mult;
	int _ID2, _t2ent;
	object *firm, *bank, *suppl,
		   *cap = V_EXTS( PARENT, countryE, capSec ),
		   *lab = V_EXTS( PARENT, countryE, labSup );

	double Deb20ratio = V( "Deb20ratio" );		// bank fin. to equity ratio
	double Phi1 = V( "Phi1" );					// lower support for K share
	double Phi2 = V( "Phi2" );					// upper support for K share
	double iota = V( "iota" );					// desired inventories factor
	double mu20 = V( "mu20" );					// initial mark-up in sector 2
	double m2 = V( "m2" );						// machine output per period
	double p10 = VLS( cap, "p1avg", 1 );		// initial machine price
	double u = V( "u" );						// desired capital utilization
	double w = VS( lab, "w" );					// current wage

	if ( newInd )
	{
		double phi = VS( lab, "phi" );			// unemployment benefit rate
		double c10 = p10 / ( 1 + VS( cap, "mu1" ) );// initial unit cost sec. 1
		double c20 = INIWAGE / INIPROD;			// initial unit cost sec. 2
		double p20 = ( 1 + mu20 ) * c20;		// initial consumer-good price
		double trW = VS( PARENT, "flagTax" ) > 0 ?
					 VS( PARENT, "tr" ) : 0;	// tax rate on wages
		double K0 = ceil( VS( lab, "Ls0" ) * INIWAGE /
						  p20 / n / m2 ) * m2;	// full employment K required
		double SIr0 = n * K0 / m2 / V( "eta" );	// substit. real invest.
		double RD0 = VS( cap, "nu" ) * SIr0 * p10;// initial R&D expense

		// initial steady state demand under fair share
		_D20 = ( ( SIr0 * c10 + RD0 ) * ( 1 - phi - trW ) +
				VS( lab, "Ls0" ) * INIWAGE * phi ) /
			  ( mu20 + phi + trW ) * c20 / n;
		_E = VL( "Eavg", 1 );					// initial competitiveness
		_K = K0;								// initial capital in sector 2
		_N = iota * _D20;						// initial inventories
		_NW20 = V( "NW20" );					// initial wealth in sector 2
		_Q2u = 1;								// initial capacity utilization
		_f2 = 1.0 / n;							// fair share
		_life2cycle = 1;						// start as incumbent
		_t2ent = 0;								// entered before t=1
	}
	else
	{
		_D20 = 0;
		_E = V( "Eavg" );						// average competitiveness
		_K = WHTAVE( "_K", "_f2" );				// w. avg. capital in sector 2
		_N = 0;									// inventories
		_NW20 = WHTAVE( "_NW2", "_f2" );		// average wealth in sector 2
		_Q2u = V( "Q2u" );						// capacity utilization
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

		_ID2 = INCR( "lastID2", 1 );			// new firm ID
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
		mult = newInd ? 1 : uniform( Phi1, Phi2 );// NW multiple
		_A2 = VS( suppl, "_Atau" );				// initial productivity
		_c2 = w / _A2;							// initial unit costs
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
		WRITELLS( firm, "_f2", _f2, _t2ent, 1 );
		WRITELLS( firm, "_f2", _f2, _t2ent, 2 );
		WRITELLS( firm, "_mu2", mu20, _t2ent, 1 );
		WRITELLS( firm, "_p2", _p2, _t2ent, 1 );
		WRITELLS( firm, "_qc2", 4, _t2ent, 1 );

		for ( int i = 1; i <= 4; ++i )
		{
			WRITELLS( firm, "_D2", _D2e, _t2ent, i );
			WRITELLS( firm, "_D2d", _D2e, _t2ent, i );
		}

		if ( newInd )
		{
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


// remove firm object and existing hooks in equation 'entry1exit', 'entry2exit'

const char *_BadDebVar[ ] = { "_BadDeb1", "_BadDeb2" },
		   *_EqVar[ ] = { "_Eq1", "_Eq2" },
		   *EqVar[ ] = { "Eq1", "Eq2" },
		   *cExitVar[ ] = { "cExit1", "cExit2" },
		   *CliBrochObj[ ] = { "Cli", "Broch" };

CFUN_DBL( exit_firm )
{
	double liqEq, liqVal;
	object *bank, *cli;
	int sec = strcmp( NAME, "Firm1" ) == 0 ? 0 : 1;

	// remove equity from sector total
	INCRS( PARENT, EqVar[ sec ], - V( _EqVar[ sec ] ) );

	// account liquidation equity credit of shareholder or bad debt cost of bank
	liqVal = V( _NWvar[ sec ] ) - V( _DebVar[ sec ] );

	if ( liqVal < 0 )							// account bank losses, if any
	{
		liqEq = 0;								// no liquidation equity
		bank = HOOK( BANK );					// exiting firm bank
		VS( bank, _BadDebVar[ sec ] );			// ensure reset in t
		INCRS( bank, _BadDebVar[ sec ], - liqVal );// accumulate bank losses
	}
	else
	{
		liqEq = ROUND( liqVal, 0, 0.01 );		// no liquidation equity credit
		INCRS( PARENT, cExitVar[ sec ], liqEq );
	}

	DELETE( HOOK( BCLIENT ) );					// leave client list of bank

	CYCLE( cli, CliBrochObj[ sec ] )			// leave counterpart list
		DELETE( SHOOKS( cli ) );				// delete from counterpart list

	if ( sec == 1 )
		// update firm map before removing LSD object in consumption sector
		EXEC_EXTS( GRANDPARENT, countryE, firm2map, erase,
				   ( int ) V( "_ID2" ) );

	DELETE( THIS );

	return liqEq;
}
