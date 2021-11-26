/******************************************************************************

	SUPPORT C FUNCTIONS
	-------------------

	Pure C support functions used in the objects in the K+S LSD model are
	coded below.

 ******************************************************************************/

/*======================== GENERAL SUPPORT C FUNCTIONS =======================*/

// calculate the bounded, moving-average growth rate of variable
// if lim is zero, there is no bounding

double mov_avg_bound( object *obj, const char *var, double lim, double per )
{
	double prev, g, sum_g;
	int i;

	for ( sum_g = i = 0; i < per; ++i )
	{
		if ( t - i <= 0 )						// just go to t=0
			break;

		prev = VLS( obj, var, i + 1 );
		g = ( prev != 0 ) ? VLS( obj, var, i ) / prev - 1 : 0;

		if ( lim > 0 )
			g = max( min( g, lim ), - lim );	// apply bounds

		sum_g += g;
	}

	return sum_g / i;
}


// append error messages and increment error counter

void check_error( bool cond, const char* errMsg, int errCount, int *errCounter )
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
		   *__IDpar[ ] = { "__ID1","__ID2" };

object *set_bank( object *firm, int firmID )
{
	int _IDb, sec = strcmp( NAMES( firm ), "Firm1" ) == 0 ? 0 : 1;
	object *bank, *cli, *fin = SEARCHS( GRANDPARENTS( firm ), "Financial" );

	_IDb = VS( fin, "pickBank" );				// draw initial preferred bank
	bank = V_EXTS( GRANDPARENTS( firm ), countryE, bankPtr[ _IDb - 1 ] );
	WRITES( firm, bankPar[ sec ], _IDb );		// save bank ID
	WRITE_HOOKS( firm, BANK, bank );

	cli = ADDOBJS( bank, CliObj[ sec ] );		// add to bank client list
	WRITES( cli, __IDpar[ sec ], firmID );		// update object
	WRITE_SHOOKS( cli, firm );					// pointer back to client
	WRITE_HOOKS( firm, BCLIENT, cli );			// pointer to bank client list

	return HOOKS( firm, BCLIENT );				// bank client list obj
}


// update firm debt in equations '_Q1', '_Tax1', '_Q2', '_EI', '_SI', '_Tax2'

const char *_CDvar[ ] = { "_CD1", "_CD2" },
		   *_CDcVar[ ] = { "_CD1c", "_CD2c" },
		   *_CSvar[ ] = { "_CS1", "_CS2" },
		   *_DebVar[ ] = { "_Deb1", "_Deb2" },
		   *_NWvar[ ] = { "_NW1", "_NW2" },
		   *_TCfreeVar[ ] = { "_TC1free", "_TC2free" };

double update_debt( object *firm, double desired, double loan )
{
	double Deb, TCfree;
	object *bank;
	int sec = strcmp( NAMES( firm ), "Firm1" ) == 0 ? 0 : 1;

	if ( desired > 0 )							// ignore loan repayment
	{
		INCRS( firm, _CDvar[ sec ], desired );	// desired credit
		INCRS( firm, _CDcVar[ sec ], desired - loan );// credit constraint
		INCRS( firm, _CSvar[ sec ], loan );		// supplied credit
	}

	Deb = VS( firm, _DebVar[ sec ] );

	// take new loan/repay debt from/to bank
	if ( loan != 0 )
	{
		if ( Deb + loan < 0.001 )				// write-off small debt?
			Deb = WRITES( firm, _DebVar[ sec ], 0 );
		else
			Deb = INCRS( firm, _DebVar[ sec ], loan );

		bank = HOOKS( firm, BANK );				// firm's bank

		// if credit limit active, adjust bank's available credit
		TCfree = VS( bank, _TCfreeVar[ sec ] );	// available credit firm's bank
		if ( TCfree > -0.1 )
			WRITES( bank, _TCfreeVar[ sec ], max( TCfree - loan, 0 ) );
	}

	return Deb;
}


// update firm deposits in equations '_Q1', '_Tax1', '_Q2', '_EI', '_SI', '_Tax2'

double update_depo( object *firm, double depo, bool incr )
{
	double NW;
	int sec = strcmp( NAMES( firm ), "Firm1" ) == 0 ? 0 : 1;

	// update total firm net worth (deposits)
	if ( incr )
	{
		NW = VS( firm, _NWvar[ sec ] );

		if ( depo != 0 )
			NW = INCRS( firm, _NWvar[ sec ], depo );
	}
	else
		NW = WRITES( firm, _NWvar[ sec ], depo );

	return NW;
}


// manage firm cash flow in equations '_Tax1', '_Tax2'

const char *_CIvar[ ] = { "", "_CI" },
		   *_CSaVar[ ] = { "_CS1a", "_CS2a" },
		   *_DivVar[ ] = { "_Div1", "_Div2" },
		   *_NWpVar[ ] = { "_NW1p", "_NW2p" };

double cash_flow( object *firm, double profit, double tax )
{
	int sec = strcmp( NAMES( firm ), "Firm1" ) == 0 ? 0 :
			  strcmp( NAMES( firm ), "Firm2" ) == 0 ? 1 : 2;
	object *fin = SEARCHS( GRANDPARENTS( firm ), "Financial" );

	double dividends = VLS( firm, _DivVar[ sec ], 1 );// shareholder dividends
	double cashFree = profit - tax - dividends;	// final free cash flow

	if ( sec > 0 )
		VS( firm, _CIvar[ sec ] );				// ensure canc. invest. reimbursed

	double provision = ( sec < 2 ) ? VS( firm, _NWpVar[ sec ] ) : 0;// prod. cost
	double depo = update_depo( firm, provision, true );// current bank deposits

	if ( cashFree < 0 )							// must finance losses?
	{
		if ( depo >= - cashFree )				// deposits cover losses?
			update_depo( firm, cashFree, true );// draw from deposits
		else
		{
			double credAvb = VS( firm, _CSaVar[ sec ] );// available credit
			double credDes = - cashFree - depo;	// desired credit

			update_debt( firm, credDes, credDes );// finance all in any case

			if ( credAvb >= credDes )			// could finance losses?
				update_depo( firm, 0, false );	// keep going with zero deposits
			else
				update_depo( firm, -1e-6, false );// let negative NW (bankruptcy)
		}
	}
	else										// pay debt with available cash
	{
		double repayDes = VS( firm, _DebVar[ sec ] ) * VS( fin, "deltaB" );
												// desired debt repayment
		if ( repayDes > 0 )						// something to repay?
		{
			if ( cashFree > repayDes )			// can repay desired and more
			{
				update_debt( firm, 0, - repayDes );// repay up to desired
				update_depo( firm, cashFree - repayDes, true );// keep the rest
			}
			else
				update_debt( firm, 0, - cashFree );// repay what is possible
		}
		else
			update_depo( firm, cashFree, true );// just keep all
	}

	return cashFree;
}


/*================== CAPITAL MANAGEMENT SUPPORT C FUNCTIONS ==================*/

// send machine brochure to consumption-good client firm in equations '_NC',
// '_supplier'

object *send_brochure( object *suppl, object *client )
{
	object *broch, *cli;

	cli = ADDOBJS( suppl, "Cli" );				// add object to new client
	WRITES( cli, "__IDc", VS( client, "_ID2" ) );// update client ID
	WRITES( cli, "__tSel", T );					// update selection time

	broch = ADDOBJS( client, "Broch" );			// add brochure to client
	WRITES( broch, "__IDs", VS( suppl, "_ID1" ) );// update supplier ID
	WRITE_SHOOKS( broch, cli );					// pointer to supplier client list
	WRITE_SHOOKS( cli, broch );					// pointer to client brochure list

	return broch;
}


// set initial supplier for entrant in equations 'entry2exit'

object *set_supplier( object *firm, int firmID )
{
	object *broch, *suppl, *cap = SEARCHS( GRANDPARENTS( firm ), "Capital" );

	suppl = RNDDRAWS( cap, "Firm1", "_AtauLP" );// draw capital supplier
	broch = send_brochure( suppl, firm );		// get supplier brochure
	WRITE_HOOKS( firm, SUPPL, broch );			// pointer to current supplier
	INCRS( suppl, "_NC", 1 );					// update supplier's clients #

	return suppl;
}


// send new machine order in equations '_EI', '_SI'

void send_order( object *firm, double nMach )
{
	// find firm entry on supplier client list
	object *cli = SHOOKS( HOOKS( firm, SUPPL ) );

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

double invest( object *firm, double desired )
{
	double invest, invCost, loan, loanDes;

	if ( desired <= 0 )
		return 0;

	double m2 = VS( PARENTS( firm ), "m2" );	// machine output per period
	double _CS2a = VS( firm, "_CS2a" );			// available credit supply
	double _NW2 = VS( firm, "_NW2" );			// net worth (cash available)
	double _p1 = VS( PARENTS( SHOOKS( HOOKS( firm, SUPPL ) ) ), "_p1" );

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

		update_debt( firm, loanDes, loan );		// update debt (desired/granted)
	}

	if ( invest > 0 )
	{
		update_depo( firm, _NW2, false );		// update the firm net worth
		send_order( firm, round( invest / m2 ) );// order to machine supplier
	}

	return invest;
}


// add new vintage to the capital stock of a firm in equation 'K' and 'initCountry'

void add_vintage( object *firm, double nMach, bool newInd )
{
	double __AeeVint, __AefVint, __AlpVint, __pVint;
	int __ageVint, __nMach, __nVint;
	object *cap, *cons, *cur, *suppl, *vint;

	suppl = PARENTS( SHOOKS( HOOKS( firm, SUPPL ) ) );// current supplier
	__nMach = floor( nMach );					// integer number of machines

	// at t=1 firms have a mix of machines: old to new, many suppliers
	if ( newInd )
	{
		cap = SEARCHS( GRANDPARENTS( firm ), "Capital" );
		cons = SEARCHS( GRANDPARENTS( firm ), "Consumption" );

		__ageVint = VS( cons, "eta" ) + 1;		// age of oldest machine
		__nVint = ceil( nMach / __ageVint );	// machines per vintage
		__AeeVint = INIEEFF;					// initial energy efficiency
		__AefVint = INIEFRI;					// initial envir. friendliness
		__AlpVint = INIPROD;					// initial labor productivity
		__pVint = VLS( cap, "p1avg", 1 );		// initial machine price
	}
	else
	{
		__ageVint = 1 - T;
		__nVint = __nMach;
		__AeeVint = VS( suppl, "_AtauEE" );
		__AefVint = VS( suppl, "_AtauEF" );
		__AlpVint = VS( suppl, "_AtauLP" );
		__pVint = VS( suppl, "_p1" );
	}

	while ( __nMach > 0 )
	{
		if ( newInd )
		{
			cur = RNDDRAW_FAIRS( cap, "Firm1" );// draw another supplier
			if ( cur == suppl )					// don't use current supplier
				continue;

			vint = ADDOBJLS( firm, "Vint", T - 1 );// recalculate in t=1
		}
		else
		{
			cur = suppl;						// just use current supplier
			vint = ADDOBJS( firm, "Vint" );		// just recalculate in next t
		}

		WRITE_SHOOKS( vint, HOOKS( firm, TOPVINT ) );// save previous vintage
		WRITE_HOOKS( firm, TOPVINT, vint );		// save pointer to top vintage

		WRITES( vint, "__IDvint", VNT( T, VS( cur, "_ID1" ) ) );// vintage ID
		WRITES( vint, "__AeeVint", __AeeVint );	// vintage energy efficiency
		WRITES( vint, "__AefVint", __AefVint );	// vintage envir. friendliness
		WRITES( vint, "__AlpVint", __AlpVint );	// vintage labor productivity
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

double scrap_vintage( variable *var, object *vint )
{
	double RS;

	if ( NEXTS( vint ) != NULL )				// don't remove last vintage
	{
		// remove as previous vintage from next vintage
		if ( SHOOKS( NEXTS( vint ) ) == vint )
			WRITE_SHOOKS( NEXTS( vint ), NULL );

		RS = abs( VS( vint, "__RSvint" ) );
		DELETE( vint );							// delete vintage
	}
	else
	{
		RS = -1;								// signal last machine
		WRITES( vint, "__nVint", 1 );			// keep just 1 machine
	}

	return RS;
}


/*=================== FIRM ENTRY-EXIT SUPPORT C FUNCTIONS ====================*/

// add and configure entrant capital-good firm object(s) and required hooks
// in equations 'entry1exit' and 'initCountry'

double entry_firm1( variable *var, object *sector, int n, bool newInd )
{
	double _AtauEE, _AtauEF, _AtauLP, _BtauEE, _BtauEF, _BtauLP, _D10, _Deb1,
		   _Eq1, _L1rd, _NW1, _NW10, _RD0, _c1, _cTau, _f1, _p1, AtauLPmax,
		   BtauLPmax, Deb1, Eq1, NW1, mult;
	int _ID1, _t1ent;
	object *firm, *bank,
		   *cons = SEARCHS( PARENTS( sector ), "Consumption" ),
		   *ene = SEARCHS( PARENTS( sector ), "Energy" ),
		   *lab = SEARCHS( PARENTS( sector ), "Labor" );

	double Deb10ratio = VS( sector, "Deb10ratio" );// bank fin. to equity ratio
	double Phi3 = VS( sector, "Phi3" );			// lower support for wealth share
	double Phi4 = VS( sector, "Phi4" );			// upper support for wealth share
	double alpha2 = VS( sector, "alpha2" );		// lower support for imitation
	double beta2 = VS( sector, "beta2" );		// upper support for imitation
	double mu1 = VS( sector, "mu1" );			// mark-up in sector 1
	double m1 = VS( sector, "m1" );				// worker production scale
	double nu = VS( sector, "nu" );				// share of R&D expenses
	double pE = VS( ene, "pE" );				// energy price
	double trCO2 = VS( PARENTS( sector ), "trCO2" );// carbon tax rate
	double x5 = VS( sector, "x5" );				// entrant upper advantage
	double w = VS( lab, "w" );					// current wage

	if ( newInd )
	{
		double F20 = VS( cons, "F20" );
		double m2 = VS( cons, "m2" );			// machine output per period

		_AtauEE = _BtauEE = INIEEFF;			// initial products.
		_AtauEF = _BtauEF = INIEFRI;
		_AtauLP = AtauLPmax = INIPROD;
		_BtauLP = BtauLPmax = ( 1 + mu1 ) * _AtauLP /
							  ( m1 * m2 * VS( cons, "b" ) );
		_NW10 = VS( sector, "NW10" );			// initial wealth in sector 1
		_f1 = 1.0 / n;							// fair share
		_t1ent = 0;								// entered before t=1

		// initial demand expectation, assuming all sector 2 firms, 1/eta
		// replacement factor and fair share in sector 1 and full employment
		double p20 = VLS( cons, "CPI", 1 );
		double K0 = ceil( VS( lab, "Ls0" ) * INIWAGE / p20 / F20 / m2 ) * m2;

		_D10 = F20 * K0 / m2 / VS( cons, "eta" ) / n;
	}
	else
	{
		_AtauEE = AVES( sector, "_AtauEE" );	// avg. machine energy efficiency
		_AtauEF = AVES( sector, "_AtauEF" );	// avg. machine envir. friendl.
		_BtauEE = AVES( sector, "_BtauEE" );	// avg. energy effic. in sector 1
		_BtauEF = AVES( sector, "_BtauEF" );	// avg. env. friend. in sector 1
		_NW10 = max( WHTAVES( sector, "_NW1", "_f1" ), VS( sector, "NW10" ) *
					 VS( sector, "PPI" ) / VS( sector, "pK0" ) );
		_f1 = 0;								// no market share
		_t1ent = T;								// entered now
		AtauLPmax = MAXS( sector, "_AtauLP" );	// best machine lab. productivity
		BtauLPmax = MAXS( sector, "_BtauLP" );	// best productivity in sector 1

		// initial demand equal to 1 machine per client under fair share entry
		_D10 = VS( cons, "F2" ) / VS( sector, "F1" );
	}

	// add entrant firms (end of period, don't try to sell)
	for ( Deb1 = Eq1 = NW1 = 0; n > 0; --n )
	{
		_ID1 = INCRS( sector, "lastID1", 1 );	// new firm ID

		// create object, only recalculate in t if new industry
		if ( newInd )
			firm = ADDOBJLS( sector, "Firm1", T - 1 );
		else
			firm = ADDOBJS( sector, "Firm1" );

		ADDHOOKS( firm, FIRM1HK );				// add object hooks
		DELETE( SEARCHS( firm, "Cli" ) );		// remove empty instances

		// select associated bank
		bank = set_bank( firm, _ID1 );

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
		WRITES( firm, "_ID1", _ID1 );
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
		WRITELLS( sector, "Deb1", Deb1, _t1ent, 1 );
		WRITELLS( sector, "Eq1", Eq1, _t1ent, 1 );
		WRITELLS( sector, "NW1", NW1, _t1ent, 1 );
	}
	else										// just account new equity
	{
		INCRS( sector, "Eq1", Eq1 );
		INCRS( sector, "cEntry1", Eq1 );
	}

	return Eq1;
}


// add and configure entrant consumer-good firm object(s) and required hooks
// in equations 'entry2exit' and 'initCountry'

double entry_firm2( variable *var, object *sector, int n, bool newInd )
{
	double _A2, _D20, _D2e, _Deb2, _E, _Eq2, _K, _N, _NW2, _NW2f, _NW20, _Q2u,
		   _c2, _f2, _life2cycle, _p2, Deb2, Eq2, K, N, NW2, mult;
	int _ID2, _t2ent;
	object *firm, *bank, *suppl,
		   *cap = SEARCHS( PARENTS( sector ), "Capital" ),
		   *ene = SEARCHS( PARENTS( sector ), "Energy" ),
		   *lab = SEARCHS( PARENTS( sector ), "Labor" );

	double Deb20ratio = VS( sector, "Deb20ratio" );// bank fin. to equity ratio
	double Phi1 = VS( sector, "Phi1" );			// lower support for K share
	double Phi2 = VS( sector, "Phi2" );			// upper support for K share
	double iota = VS( sector, "iota" );			// desired inventories factor
	double mu20 = VS( sector, "mu20" );			// initial mark-up in sector 2
	double m2 = VS( sector, "m2" );				// machine output per period
	double p10 = VLS( cap, "p1avg", 1 );		// initial machine price
	double pE = VS( ene, "pE" );				// energy price
	double trCO2 = VS( PARENTS( sector ), "trCO2" );// carbon tax rate
	double u = VS( sector, "u" );				// desired capital utilization
	double w = VS( lab, "w" );					// current wage

	if ( newInd )
	{
		double phi = VS( lab, "phi" );			// unemployment benefit rate
		double c10 = p10 / ( 1 + VS( cap, "mu1" ) );// initial unit cost sec. 1
		double c20 = INIWAGE / INIPROD +		// initial unit cost sec. 2
					 ( pE + trCO2 * INIEFRI ) / INIEEFF;
		double p20 = ( 1 + mu20 ) * c20;		// initial consumer-good price
		double trW = VS( PARENTS( sector ), "flagTax" ) > 0 ?
					 VS( PARENTS( sector ), "tr" ) : 0;// tax rate on wages
		double K0 = ceil( VS( lab, "Ls0" ) * INIWAGE /
						  p20 / n / m2 ) * m2;	// full employment K required
		double SIr0 = n * K0 / m2 / VS( sector, "eta" );// substit. real invest.
		double RD0 = VS( cap, "nu" ) * SIr0 * p10;// initial R&D expense

		// initial steady state demand under fair share
		_D20 = ( ( SIr0 * c10 + RD0 ) * ( 1 - phi - trW ) +
				VS( lab, "Ls0" ) * INIWAGE * phi ) /
			  ( mu20 + phi + trW ) * c20 / n;
		_E = VLS( sector, "Eavg", 1 );			// initial competitiveness
		_K = K0;								// initial capital in sector 2
		_N = iota * _D20;						// initial inventories
		_NW20 = VS( sector, "NW20" );			// initial wealth in sector 2
		_Q2u = 1;								// initial capacity utilization
		_f2 = 1.0 / n;							// fair share
		_life2cycle = 1;						// start as incumbent
		_t2ent = 0;								// entered before t=1
	}
	else
	{
		_D20 = 0;
		_E = VS( sector, "Eavg" );				// average competitiveness
		_K = WHTAVES( sector, "_K", "_f2" );	// w. avg. capital in sector 2
		_N = 0;									// inventories
		_NW20 = WHTAVES( sector, "_NW2", "_f2" );// average wealth in sector 2
		_Q2u = VS( sector, "Q2u" );				// capacity utilization
		_f2 = 0;								// no market share
		_life2cycle = 0;						// start as pre-operat. entrant
		_t2ent = T;								// entered now
	}

	// add entrant firms (end of period, don't try to sell)
	for ( Deb2 = Eq2 = NW2 = K = N = 0; n > 0; --n )
	{
		_ID2 = INCRS( sector, "lastID2", 1 );	// new firm ID

		// create object, only recalculate in t if new industry
		if ( newInd )
			firm = ADDOBJLS( sector, "Firm2", T - 1 );
		else
			firm = ADDOBJS( sector, "Firm2" );

		ADDHOOKS( firm, FIRM2HK );				// add object hooks
		DELETE( SEARCHS( firm, "Vint" ) );		// remove empty instances
		DELETE( SEARCHS( firm, "Broch" ) );

		// select associated bank
		bank = set_bank( firm, _ID2 );

		// select initial machine supplier
		suppl = set_supplier( firm, _ID2 );

		// initial desired capital/expected demand, rounded to # of machines
		mult = newInd ? 1 : uniform( Phi1, Phi2 );// capital multiple
		K += _K = ceil( max( mult * _K / m2, 1 ) ) * m2;
		_D2e = newInd ? _D20 : u * _K;
		N += _N;

		// define entrant initial free cash (1 period wages or default minimum)
		mult = newInd ? 1 : uniform( Phi1, Phi2 );// NW multiple
		_A2 = VS( suppl, "_AtauLP" );			// initial labor productivity
		_c2 = VS( suppl, "_cTau" );				// initial unit costs
		_p2 = ( 1 + mu20 ) * _c2;				// initial price
		_NW2f = ( 1 + iota ) * _D2e * _c2;		// initial free cash
		_NW2f = max( _NW2f, mult * _NW20 );

		// initial equity must pay initial capital and wages
		NW2 += _NW2 = newInd ? _NW2f : VS( suppl, "_p1" ) * _K / m2 + _NW2f;
		Deb2 += _Deb2 = _NW2 * Deb20ratio;
		Eq2 += _Eq2 = _NW2 * ( 1 - Deb20ratio );// accumulated equity (all firms)

		// initialize variables
		WRITES( firm, "_Eq2", _Eq2 );
		WRITES( firm, "_ID2", _ID2 );
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

			add_vintage( firm, _K / m2, newInd );// first machine vintages
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
		WRITELLS( sector, "Deb2", Deb2, _t2ent, 1 );
		WRITELLS( sector, "Eq2", Eq2, _t2ent, 1 );
		WRITELLS( sector, "K", K, _t2ent, 1 );
		WRITELLS( sector, "N", N, _t2ent, 1 );
		WRITELLS( sector, "NW2", NW2, _t2ent, 1 );
	}
	else										// just account new equity
	{
		INCRS( sector, "Eq2", Eq2 );
		INCRS( sector, "cEntry2", Eq2 );
	}

	return Eq2;
}


// remove firm object and existing hooks in equation 'entry1exit', 'entry2exit'

const char *_BadDebVar[ ] = { "_BadDeb1", "_BadDeb2" },
		   *_EqVar[ ] = { "_Eq1", "_Eq2" },
		   *EqVar[ ] = { "Eq1", "Eq2" },
		   *cExitVar[ ] = { "cExit1", "cExit2" },
		   *CliBrochObj[ ] = { "Cli", "Broch" };

double exit_firm( variable *var, object *firm )
{
	double liqEq, liqVal;
	object *bank, *cli;
	int sec = strcmp( NAMES( firm ), "Firm1" ) == 0 ? 0 : 1;

	// remove equity from sector total
	INCRS( PARENTS( firm ), EqVar[ sec ], - VS( firm, _EqVar[ sec ] ) );

	// account liquidation equity credit of shareholder or bad debt cost of bank
	liqVal = VS( firm, _NWvar[ sec ] ) - VS( firm, _DebVar[ sec ] );

	if ( liqVal < 0 )							// account bank losses, if any
	{
		liqEq = 0;								// no liquidation equity
		bank = HOOKS( firm, BANK );				// exiting firm bank
		VS( bank, _BadDebVar[ sec ] );			// ensure reset in t
		INCRS( bank, _BadDebVar[ sec ], - liqVal );// accumulate bank losses
	}
	else
	{
		liqEq = ROUND( liqVal, 0, 0.01 );		// no liquidation equity credit
		INCRS( PARENTS( firm ), cExitVar[ sec ], liqEq );
	}

	DELETE( HOOKS( firm, BCLIENT ) );			// leave client list of bank

	CYCLES( firm, cli, CliBrochObj[ sec ] )		// leave counterpart lists
		DELETE( SHOOKS( cli ) );				// delete from counterpart list

	if ( sec == 1 )
		// update firm map before removing LSD object in consumption sector
		EXEC_EXTS( GRANDPARENTS( firm ), countryE, firm2map, erase,
				   ( int ) VS( firm, "_ID2" ) );

	DELETE( firm );

	return liqEq;
}
