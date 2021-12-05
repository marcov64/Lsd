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


// set initial bank for entrant in equations 'entry1exit', 'entry2exit',
// 'entryEexit'

const char *bankPar[ ] = { "_bank1", "_bank2", "_bankE" },
		   *CliObj[ ] = { "Cli1", "Cli2", "CliE" },
		   *__IDpar[ ] = { "__ID1","__ID2", "__IDe" };

object *set_bank( object *firm, int firmID )
{
	int _IDb, sec = strcmp( NAMES( firm ), "Firm1" ) == 0 ? 0 :
					strcmp( NAMES( firm ), "Firm2" ) == 0 ? 1 : 2;
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


// update firm debt in equations '_Q1', '_Tax1', '_Q2', '_EI', '_SI', '_Tax2',
// '_EIe', '_TaxE'

const char *_CDvar[ ] = { "_CD1", "_CD2", "_CDe" },
		   *_CDcVar[ ] = { "_CD1c", "_CD2c", "_CDeC" },
		   *_CSvar[ ] = { "_CS1", "_CS2", "_CSe" },
		   *_DebVar[ ] = { "_Deb1", "_Deb2", "_DebE" },
		   *_NWvar[ ] = { "_NW1", "_NW2", "_NWe" },
		   *_TCfreeVar[ ] = { "_TC1free", "_TC2free", "_TCeFree" };

double update_debt( object *firm, double desired, double loan )
{
	double Deb, TCfree;
	object *bank;
	int sec = strcmp( NAMES( firm ), "Firm1" ) == 0 ? 0 :
			  strcmp( NAMES( firm ), "Firm2" ) == 0 ? 1 : 2;

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


// update firm deposits in equations '_Q1', '_Tax1', '_Q2', '_EI', '_SI',
// '_Tax2', 'EIe', '_TaxE'

double update_depo( object *firm, double depo, bool incr )
{
	double NW;
	int sec = strcmp( NAMES( firm ), "Firm1" ) == 0 ? 0 :
			  strcmp( NAMES( firm ), "Firm2" ) == 0 ? 1 : 2;

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


// manage firm cash flow in equations '_Tax1', '_Tax2', '_TaxE'

const char *_CIvar[ ] = { "", "_CI", "_CIe" },
		   *_CSaVar[ ] = { "_CS1a", "_CS2a", "_CSeA" },
		   *_DivVar[ ] = { "_Div1", "_Div2", "_DivE" },
		   *_NWpVar[ ] = { "_NW1p", "_NW2p", "" };

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

const char *Cli1Obj[ ] = { "", "Cli", "CliEn" },
		   *CliBrochObj[ ] = { "Cli", "Broch", "BrE" },
		   *_IDpar[ ] = { "_ID1", "_ID2", "_IDe" },
		   *__IDcPar[ ] = { "", "__IDc", "__IDcE" },
		   *__IDsPar[ ] = { "", "__IDs", "__IDsE" },
		   *__tSelPar[ ] = { "", "__tSel", "__tSelE" };

object *send_brochure( object *suppl, object *client )
{
	object *broch, *cli;
	int sec = strcmp( NAMES( client ), "Firm2" ) == 0 ? 1 : 2;

	cli = ADDOBJS( suppl, Cli1Obj[ sec ] );		// add object to new client
	WRITES( cli, __IDcPar[ sec ], VS( client, _IDpar[ sec ] ) );
	WRITES( cli, __tSelPar[ sec ], T );			// update selection time

	broch = ADDOBJS( client, CliBrochObj[ sec ] );// add brochure to client
	WRITES( broch, __IDsPar[ sec ], VS( suppl, _IDpar[ 0 ] ) );// supplier ID
	WRITE_SHOOKS( broch, cli );					// pointer to supplier client list
	WRITE_SHOOKS( cli, broch );					// pointer to client brochure list

	return broch;
}


// set initial supplier for entrant in equations 'entry2exit', 'entryEexit'

object *set_supplier( object *firm, int firmID )
{
	object *broch, *suppl, *cap = SEARCHS( GRANDPARENTS( firm ), "Capital" );

	suppl = RNDDRAWS( cap, "Firm1", "_AtauLP" );// draw capital supplier
	broch = send_brochure( suppl, firm );		// get supplier brochure
	WRITE_HOOKS( firm, SUPPL, broch );			// pointer to current supplier
	INCRS( suppl, "_NC", 1 );					// update supplier's clients #

	return suppl;
}


// send new machine order in equations '_EI', '_SI', '_EIe'

const char *__nCanPar[ ] = { "", "__nCan", "__nCanE" },
		   *__nOrdPar[ ] = { "", "__nOrd", "__nOrdE" },
		   *__tOrdPar[ ] = { "", "__tOrd", "__tOrdE" };

void send_order( object *firm, double nMach )
{
	int sec = strcmp( NAMES( firm ), "Firm2" ) == 0 ? 1 : 2;

	// find firm entry on supplier client list
	object *cli = SHOOKS( HOOKS( firm, SUPPL ) );

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


// add new green power plant to energy firm in equation 'EIe' and 'initCountry'

void add_green_plant( object *firm, double cap, double nMach, bool newInd )
{
	object *plant;
	double u = 1 / ( 1 + VS( PARENTS( firm ), "iotaE" ) );
	double p1 = VS( PARENTS( SHOOKS( HOOKS( firm, SUPPL ) ) ), "_p1" );

	if ( newInd )
	{
		plant = ADDOBJLS( firm, "Green", T - 1 );// recalculate in t
		WRITES( plant, "__tGE", T - 1 );		// installation time
		WRITELS( plant, "__QgeU", u, 1 );		// planned utilization
	}
	else
	{
		plant = ADDOBJS( firm, "Green" );		// recalculate only in t+1
		WRITES( plant, "__tGE", T );
		WRITES( plant, "__QgeU", u );
	}

	WRITES( plant, "__Kge", cap );				// plant generation capacity
	WRITES( plant, "__ICge", p1 * nMach );		// plant capital cost
	WRITES( plant, "__mGE", cap / nMach );		// unit (machine) power capacity

	WRITE_HOOKS( firm, TOPVINT, plant );		// new top green vintage
}


// add new dirty power plant to energy firm in equation 'EIe' and 'initCountry'

void add_dirty_plant( object *firm, double cap, bool newInd )
{
	object *plant;
	double u = 1 / ( 1 + VS( PARENTS( firm ), "iotaE" ) );

	if ( newInd )
	{
		plant = ADDOBJLS( firm, "Dirty", T - 1 );// recalculate in t
		WRITES( plant, "__tDE", T - 1 );		// installation time
		WRITELS( plant, "__QdeU", u, 1 );		// planned utilization
	}
	else
	{
		plant = ADDOBJS( firm, "Dirty" );		// recalculate only in t+1
		WRITES( plant, "__tDE", T );
		WRITES( plant, "__QdeU", u );
	}

	WRITES( plant, "__Kde", cap );				// plant generation capacity
	WRITES( plant, "__Ade", VS( firm, "_AtauDE" ) );// plant thermal efficiency
	WRITES( plant, "__emDE", VS( firm, "_emTauDE" ) );// plant emissions
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
		DELETE( SEARCHS( firm, "CliEn" ) );

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
		_ID2 = ID( 2, INCRS( sector, "lastID2", 1 ) );// new firm ID

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


// add and configure entrant energy producer firm object(s) and required hooks
// in equations 'entryEexit' and 'initCountry'

double entry_firmE( variable *var, object *sector, int n, bool newInd )
{
	double _AtauDE, _DeE, _DebE, _EqE, _ICtauGE, _Kde, _Kge, _KgeD, _NWe,
		   _emTauDE, _fE, _fKge, _pE, _p1, AtauDEmax, AtauDEmin, DebE, EqE,
		   ICtauGEmax, ICtauGEmin, Kde, Kge, NWe, NWe0, emTauDEavg, mult;
	int _IDe, _nMach, _tEent;
	object *firm, *bank, *plant, *suppl,
		   *cap = SEARCHS( PARENTS( sector ), "Capital" ),
		   *cons = SEARCHS( PARENTS( sector ), "Consumption" ),
		   *lab = SEARCHS( PARENTS( sector ), "Labor" );

	double DebE0ratio = VS( sector, "DebE0ratio" );// bank fin. to equity ratio
	double Phi5 = VS( sector, "Phi5" );			// lower support for wealth share
	double Phi6 = VS( sector, "Phi6" );			// upper support for wealth share
	double alpha3 = VS( sector, "alpha3" );		// lower support for imitation
	double beta3 = VS( sector, "beta3" );		// upper support for imitation
	double bE = VS( sector, "bE" );				// required payback period
	double fGE0 = VS( sector, "fGE0" );			// initial green energy share
	double iotaE = VS( sector, "iotaE" );		// planned reserve capacity
	double pF = VS( sector, "pF" );				// fossil fuel price
	double x6 = VS( sector, "x6" );				// entrant upper advantage
	int flagEnClim = VS( PARENTS( sector ), "flagEnClim" );// energy enable flag

	double _muE = VS( sector, "muE0" ) * VLS( lab, "wReal", 1 );// mark-up floor

	if ( newInd )
	{
		double p20 = VLS( cons, "CPI", 1 );
		double D10 = VS( lab, "Ls0" ) * INIWAGE / p20 / VS( cons, "m2" ) /
					 VS( cons, "eta" );			// initial demand for sector 1
		double D20 = SUMLS( cons, "_D2", 1 ) ;	// initial demand for sector 2

		// initial steady state expected demand under fair share
		_DeE = ( D10 / VS( cap, "m1" ) + D20 ) / INIEEFF / n;

		_AtauDE = VS( sector, "Ade0" );			// ini. efficiency dirty plant
		_ICtauGE = VS( sector, "ICge0" );		// initial green plant unit cost
		_emTauDE = VS( sector, "emDE0" );		// initial emissions dirty plant
		_fE = 1.0 / n;							// fair share
		_fKge = VS( sector, "fGE0" );			// initial share of green plants
		_pE = _muE + ( _fKge == 1 ? 0 : pF / _AtauDE );// initial price
		_tEent = 0;								// entered before t=1
		NWe0 = VS( sector, "NWe0" );			// initial wealth in energy sec.
	}
	else
	{
		_DeE = VS( PARENTS( sector ), "En" ) / VS( sector, "Fe" );// fair share
		_emTauDE = AVES( sector, "_emTauDE" );	// average dirty energy emissions
		_fE = 0;								// no market share
		_fKge = WHTAVES( sector, "_fKge", "_fE" );// average share of green plants
		_pE = WHTAVES( sector, "_pE", "_fE" );	// average power price
		_tEent = T;								// entered now
		AtauDEmax = MAXS( sector, "_AtauDE" );	// max dirty energy efficiency
		AtauDEmin = MINS( sector, "_AtauDE" );	// min dirty energy efficiency
		ICtauGEmax = MAXS( sector, "_ICtauGE" );// max green plant cost
		ICtauGEmin = MINS( sector, "_ICtauGE" );// min green plant cost
		NWe0 = max( WHTAVES( sector, "_NWe", "_fE" ), VS( sector, "NWe0" ) );
	}

	// add entrant firms (end of period, don't try to sell)
	for ( DebE = EqE = NWe = Kde = Kge = 0; n > 0; --n )
	{
		_IDe = ID( 3, INCRS( sector, "lastIDe", 1 ) );// new firm ID

		// create object, only recalculate in t if new industry
		if ( newInd )
			firm = ADDOBJLS( sector, "FirmE", T - 1 );
		else
			firm = ADDOBJS( sector, "FirmE" );

		ADDHOOKS( firm, FIRMEHK );				// add object hooks
		WRITE_HOOKS( firm, TOPVINT, NULL );		// no green plant yet
		DELETE( SEARCHS( firm, "Dirty" ) );		// remove empty instances
		DELETE( SEARCHS( firm, "Green" ) );
		DELETE( SEARCHS( firm, "BrE" ) );

		// select associated bank
		bank = set_bank( firm, _IDe );

		// select initial machine supplier
		suppl = set_supplier( firm, _IDe );

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
		WRITES( firm, "_IDe", _IDe );
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
		WRITELLS( firm, "_qcE", 4, _tEent, 1 );

		if ( newInd )
		{
			WRITELLS( firm, "_DebE", _DebE, _tEent, 1 );
			WRITELLS( firm, "_Ke", _Kge + _Kde, _tEent, 1 );
			WRITELLS( firm, "_Kde", _Kde, _tEent, 1 );
			WRITELLS( firm, "_Kge", _Kge, _tEent, 1 );
			WRITELLS( firm, "_NWe", _NWe, _tEent, 1 );

			if ( _Kge > 0 )						// first green plant
				add_green_plant( firm, _Kge, _nMach, true );

			if ( _Kde > 0 )						// first dirty plant
				add_dirty_plant( firm, _Kde, true );
		}
		else
		{
			WRITES( firm, "_AtauDE", _AtauDE );
			WRITES( firm, "_DeE", _DeE );
			WRITES( firm, "_DebE", _DebE );
			WRITES( firm, "_ICtauGE", _ICtauGE );
			WRITES( firm, "_NWe", _NWe );
			WRITES( firm, "_emTauDE", _emTauDE );
			WRITES( firm, "_fE", _fE );
			WRITES( firm, "_fKge", _fKge );
			WRITES( firm, "_muE", _muE );
			WRITES( firm, "_pE", _pE );

			// compute variables requiring calculation in t
			RECALCS( firm, "_DebEmax" );		// prudential credit limit
			VS( firm, "_CSeA" );				// update credit supply
		}
	}

	if ( newInd )								// set t=0 values
	{
		WRITELLS( sector, "DebE", DebE, _tEent, 1 );
		WRITELLS( sector, "EqE", EqE, _tEent, 1 );
		WRITELLS( sector, "Ke", Kge + Kde, _tEent, 1 );
		WRITELLS( sector, "NWe", NWe, _tEent, 1 );
	}
	else										// just account new equity
	{
		INCRS( sector, "EqE", EqE );
		INCRS( sector, "cEntryE", EqE );
	}

	return EqE;
}


// remove firm object and existing hooks in equation 'entry1exit', 'entry2exit',
// entryEexit

const char *_BadDebVar[ ] = { "_BadDeb1", "_BadDeb2", "_BadDebE" },
		   *_EqVar[ ] = { "_Eq1", "_Eq2", "_EqE" },
		   *EqVar[ ] = { "Eq1", "Eq2", "EqE" },
		   *cExitVar[ ] = { "cExit1", "cExit2", "cExitE" };

double exit_firm( variable *var, object *firm )
{
	double liqEq, liqVal;
	object *bank, *cli;
	int sec = strcmp( NAMES( firm ), "Firm1" ) == 0 ? 0 :
			  strcmp( NAMES( firm ), "Firm2" ) == 0 ? 1 : 2;

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

	CYCLES( firm, cli, CliBrochObj[ sec ] )		// leave 1st counterpart list
		DELETE( SHOOKS( cli ) );				// delete from counterpart lists

	if ( sec == 0 )
		CYCLES( firm, cli, "CliEn" )			// leave 2nd counterpart list
			DELETE( SHOOKS( cli ) );			// delete from counterpart lists

	if ( sec == 1 )
		// update firm map before removing LSD object in consumption sector
		EXEC_EXTS( GRANDPARENTS( firm ), countryE, firm2map, erase,
				   ( int ) VS( firm, _IDpar[ sec ] ) );

	if ( sec == 2 )
		// update firm map before removing LSD object in energy sector
		EXEC_EXTS( GRANDPARENTS( firm ), countryE, firmEmap, erase,
				   ( int ) VS( firm, _IDpar[ sec ] ) );

	DELETE( firm );

	return liqEq;
}
