/******************************************************************************

	SUPPORT C FUNCTIONS
	-------------------

	Pure C support functions used in the objects in the K+S LSD model are 
	coded below.
 
 ******************************************************************************/

/*======================== GENERAL SUPPORT C FUNCTIONS =======================*/

#define MOV_AVG_PER 1							// moving average period

// calculate the bounded, moving-average growth rate of variable
// if lim is zero, there is no bounding

double mov_avg_bound( object *obj, const char *var, double lim )
{
	double prev, g, sum_g;
	int i;
	
	for ( sum_g = i = 0; i < MOV_AVG_PER; ++i )
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
		LOG( " %s", errMsg )
	else
		LOG( " %s(%d)", errMsg, errCount )
	
	++( *errCounter );
}


/*====================== FINANCIAL SUPPORT C FUNCTIONS =======================*/

// comparison function for sort method in equation 'cScores'

bool rank_desc_NWtoS( firmRank e1, firmRank e2 ) 
{ 
	return e1.NWtoS > e2.NWtoS; 
}


// update firm debt in equation '_Q1', '_Tax1'

void update_debt1( object *firm, double desired, double loan )
{
	if ( desired > 0 )							// ignore loan repayment
	{
		INCRS( firm, "_CD1", desired );			// desired credit
		INCRS( firm, "_CD1c", desired - loan );	// credit constraint
		INCRS( firm, "_CS1", loan );			// supplied credit
	}
	
	if ( loan != 0 )
	{
		INCRS( firm, "_Deb1", loan );			// increment firm's debt stock

		object *bank = HOOKS( firm, BANK );		// firm's bank
		double TC1free = VS( bank, "_TC1free" );// available credit firm's bank

		// if credit limit active, adjust bank's available credit
		if ( TC1free > -0.1 )
			WRITES( bank, "_TC1free", max( TC1free - loan, 0 ) );
	}
}


// update firm debt in equations '_Q2', '_EI', '_SI', '_Tax2'

void update_debt2( object *firm, double desired, double loan )
{
	if ( desired > 0 )							// ignore loan repayment
	{
		INCRS( firm, "_CD2", desired );			// desired credit
		INCRS( firm, "_CD2c", desired - loan );	// credit constraint
		INCRS( firm, "_CS2", loan );			// supplied credit
	}
	
	if ( loan != 0 )
	{
		INCRS( firm, "_Deb2", loan );			// increment firm's debt stock
	
		object *bank = HOOKS( firm, BANK );		// firm's bank
		double TC2free = VS( bank, "_TC2free" );// available credit at firm's bank
	
		// if credit limit active, adjust bank's available credit
		if ( TC2free > -0.1 )
			WRITES( bank, "_TC2free", max( TC2free - loan, 0 ) );
	}
}


/*================== CAPITAL MANAGEMENT SUPPORT C FUNCTIONS ==================*/

// send machine brochure to consumption-good client firm in equation '_NC'

object *send_brochure( int supplID, object *suppl, int clientID, object *client )
{
	object *broch, *cli;
	
	cli = ADDOBJS( suppl, "Cli" );				// add object to new client
	WRITES( cli, "_IDc", clientID );			// update client ID
	WRITES( cli, "_tSel", T );					// update selection time

	broch = ADDOBJS( client, "Broch" );			// add brochure to client
	WRITES( broch, "_IDs", supplID );			// update supplier ID
	WRITE_SHOOKS( broch, cli );					// pointer to supplier client list
	WRITE_SHOOKS( cli, broch );					// pointer to client brochure list
	
	return broch;
}


// send new machine order in equations '_EI', '_SI'

void send_order( object *firm, double nMach )
{
	// find firm entry on supplier client list
	object *cli = SHOOKS( HOOKS( firm, SUPPL ) );
	
	if ( VS( cli, "_tOrd" ) < T )				// if first order in period
	{
		WRITES( cli, "_nOrd", nMach );			// set new order size
		WRITES( cli, "_tOrd", T );				// set order time
		WRITES( cli, "_nCan", 0 );				// no machine canceled yet
	}
	else
		INCRS( cli, "_nOrd", nMach );			// increase existing order size
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

	if ( invCost <= _NW2 - 1 )					// can invest with own funds?
	{
		invest = desired;						// invest as planned
		_NW2 -= invCost;						// remove machines cost from cash
	}
	else
	{
		if ( invCost <= _NW2 - 1 + _CS2a )		// possible to finance all?
		{
			invest = desired;					// invest as planned
			loan = loanDes = invCost - _NW2 + 1;// finance the difference
			_NW2 = 1;							// keep minimum cash
		}
		else									// credit constrained firm
		{
			// invest as much as the available finance allows, rounded # machines
			invest = max( floor( ( _NW2 - 1 + _CS2a ) / _p1 ) * m2, 0 );
			loanDes = invCost - _NW2 + 1;		// desired credit
			
			if ( invest == 0 )
				loan = 0;						// no finance
			else
			{
				invCost = _p1 * invest / m2;	// reduced investment cost
				loan = invCost - _NW2 + 1;		// finance the difference
				_NW2 = 1;						// keep minimum cash
			}
		}

		update_debt2( firm, loanDes, loan );	// update debt (desired/granted)
	}

	if ( invest > 0 )
	{
		WRITES( firm, "_NW2", _NW2 );			// update the firm net worth
		send_order( firm, round( invest / m2 ) );// order to machine supplier
	}

	return invest;
}


// add new vintage to the capital stock of a firm in equation 'K' and 'initCountry'

void add_vintage( object *firm, double nMach, bool newInd )
{
	double _Avint, _pVint;
	int _ageVint, _nMach, _nVint;
	object *cap, *cons, *cur, *suppl, *vint;
	
	suppl = PARENTS( SHOOKS( HOOKS( firm, SUPPL ) ) );// current supplier
	_nMach = floor( nMach );					// integer number of machines
	
	// at t=1 firms have a mix of machines: old to new, many suppliers
	if ( newInd )
	{
		cap = SEARCHS( GRANDPARENTS( firm ), "Capital" ); 
		cons = SEARCHS( GRANDPARENTS( firm ), "Consumption" );
		
		_ageVint = VS( cons, "eta" ) + 1;		// age of oldest machine
		_nVint = ceil( nMach / _ageVint );		// machines per vintage
		_Avint = INIPROD;						// initial product. in sector 2
		_pVint = VLS( cap, "p1avg", 1 );		// initial machine price
	}
	else
	{
		_ageVint = 1 - T;
		_nVint = _nMach;
		_Avint = VS( suppl, "_Atau" );
		_pVint = VS( suppl, "_p1" );
	}

	while ( _nMach > 0 )
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
	
		WRITES( vint, "_IDvint", VNT( T, VS( cur, "_ID1" ) ) );// vintage ID
		WRITES( vint, "_Avint", _Avint );		// vintage productivity
		WRITES( vint, "_nVint", _nVint );		// number of machines in vintage
		WRITES( vint, "_pVint", _pVint );		// price of machines in vintage
		WRITES( vint, "_tVint", 1 - _ageVint );	// vintage build time
		
		_nMach -= _nVint;
		--_ageVint;
	
		if ( _ageVint > 0 && _nMach % _ageVint == 0 )// exact ratio missing?
			_nVint = _nMach / _ageVint;			// adjust machines per vintage
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
		
		RS = abs( VS( vint, "_RSvint" ) );					
		DELETE( vint );							// delete vintage
	}
	else
	{
		RS = -1;								// signal last machine
		WRITES( vint, "_nVint", 1 );			// keep just 1 machine
	}
	
	return RS;
}


/*=================== FIRM ENTRY-EXIT SUPPORT C FUNCTIONS ====================*/

// add and configure entrant capital-good firm object(s) and required hooks 
// in equations 'entry1exit' and 'initCountry'

double entry_firm1( variable *var, object *sector, int n, bool newInd )
{
	double Atau, AtauMax, Btau, BtauMax, D10, NW1, NW10, RD0, c1, f1, p1, mult, 
		   equity = 0;
	int ID1, IDb, t1ent;
	object *firm, *bank, *cli, 
		   *cons = SEARCHS( PARENTS( sector ), "Consumption" ), 
		   *fin = SEARCHS( PARENTS( sector ), "Financial" ),
		   *lab = SEARCHS( PARENTS( sector ), "Labor" );
	
	double Deb10ratio = VS( sector, "Deb10ratio" );// bank fin. to equity ratio
	double Phi3 = VS( sector, "Phi3" );			// lower support for wealth share
	double Phi4 = VS( sector, "Phi4" );			// upper support for wealth share
	double alpha2 = VS( sector, "alpha2" );		// lower support for imitation
	double beta2 = VS( sector, "beta2" );		// upper support for imitation
	double mu1 = VS( sector, "mu1" );			// mark-up in sector 1
	double m1 = VS( sector, "m1" );				// worker production scale
	double nu = VS( sector, "nu" );				// share of R&D expenses
	double x5 = VS( sector, "x5" );				// entrant upper advantage
	double w = VS( lab, "w" );					// current wage
	
	if ( newInd )
	{
		double F2 = VS( cons, "F2" );
		double m2 = VS( cons, "m2" );			// machine output per period

		Atau = AtauMax = INIPROD;				// initial productivities to use
		Btau = BtauMax = ( 1 + mu1 ) * Atau / ( m1 * m2 );// and build machines
		NW10 = VS( sector, "NW10" ); 			// initial wealth in sector 1
		f1 = 1.0 / n;							// fair share
		t1ent = 0;								// entered before t=1
		
		// initial demand expectation, assuming all sector 2 firms, 1/eta 
		// replacement factor and fair share in sector 1 and full employment
		double p20 = VLS( cons, "CPI", 1 );
		double K0 = ceil( VS( lab, "Ls0" ) * INIWAGE / p20 / F2 / m2 ) * m2;
		
		D10 = F2 * K0 / m2 / VS( cons, "eta" ) / n;
	}
	else
	{
		AtauMax = MAXS( sector, "_Atau" );		// best machine productivity
		BtauMax = MAXS( sector, "_Btau" );		// best productivity in sector 1
		NW10 = max( WHTAVES( sector, "_NW1", "_f1" ), VS( sector, "NW10" ) * 
					VS( sector, "PPI" ) / VS( sector, "PPI0" ) );
		f1 = 0;									// no market share
		t1ent = T;								// entered now
		
		// initial demand equal to 1 machine per client under fair share entry
		D10 = VS( cons, "F2" ) / VS( sector, "F1" );
	}
	
	// add entrant firms (end of period, don't try to sell)
	for ( ; n > 0; --n )
	{
		ID1 = INCRS( sector, "lastID1", 1 );	// new firm ID
		
		// create object, only recalculate in t if new industry
		if ( newInd )
			firm = ADDOBJLS( sector, "Firm1", T - 1 );
		else
			firm = ADDOBJS( sector, "Firm1" );
		
		ADDHOOKS( firm, FIRM1HK );				// add object hooks
		
		// select associated bank and create hooks to/from it
		IDb = VS( fin, "pickBank" );			// draw bank
		bank = V_EXTS( PARENTS( sector ), country, bankPtr [ IDb - 1 ] );// bank object
		WRITES( firm, "_bank1", IDb );
		WRITE_HOOKS( firm, BANK, bank );		
		cli = ADDOBJS( bank, "Cli1" );			// add to bank client list
		WRITES( cli, "_IDc1", ID1 );
		WRITE_SHOOKS( cli, firm );				// pointer back to client
		WRITE_HOOKS( firm, BCLIENT, cli );		// pointer to bank client list
		
		// remove empty client instance
		cli = SEARCHS( firm, "Cli" );
		DELETE( cli );

		if ( ! newInd )
		{
			// initial labor productivity (imitation from best firm)
			Atau = beta( alpha2, beta2 );		// draw A from Beta(alpha,beta)
			Atau *= AtauMax * ( 1 + x5 );		// fraction of top firm
			Btau = beta( alpha2, beta2 );		// draw B from Beta(alpha,beta)
			Btau *= BtauMax * ( 1 + x5 );		// fraction of top firm
		}
		
		// initial cost, price and net wealth
		mult = newInd ? 1 : uniform( Phi3, Phi4 );// NW multiple
		c1 = w / ( Btau * m1 );					// unit cost
		p1 = ( 1 + mu1 ) * c1;					// unit price
		RD0 = nu * D10 * p1;					// R&D expense
		NW1 = mult * NW10;						// initial free cash
		equity += NW1 * ( 1 - Deb10ratio );		// accumulated equity (all firms)

		// initialize variables
		WRITES( firm, "_ID1", ID1 );
		WRITES( firm, "_t1ent", t1ent );
		WRITELLS( firm, "_Atau", Atau, t1ent, 1 );
		WRITELLS( firm, "_Btau", Btau, t1ent, 1 );
		WRITELLS( firm, "_Deb1", NW1 * Deb10ratio, t1ent, 1 );
		WRITELLS( firm, "_L1rd", RD0 / w, t1ent, 1 );
		WRITELLS( firm, "_NW1", NW1, t1ent, 1 );
		WRITELLS( firm, "_RD", RD0, t1ent, 1 );
		WRITELLS( firm, "_cTau", w / Atau, t1ent, 1 );
		WRITELLS( firm, "_f1", f1, t1ent, 1 );
		WRITELLS( firm, "_p1", p1, t1ent, 1 );
		WRITELLS( firm, "_qc1", 4, t1ent, 1 );
		
		if ( ! newInd )
		{
			WRITES( firm, "_Atau", Atau );
			WRITES( firm, "_Btau", Btau );
			WRITES( firm, "_Deb1", NW1 * Deb10ratio );
			WRITES( firm, "_L1rd", RD0 / w );
			WRITES( firm, "_NW1", NW1 );
			WRITES( firm, "_RD", RD0 );
			WRITES( firm, "_c1", c1 );
			WRITES( firm, "_cTau", w / Atau );
			WRITES( firm, "_p1", p1 );
			
			// compute variables requiring calculation in t 
			RECALCS( firm, "_Deb1max" );		// prudential credit limit
			RECALCS( firm, "_NC" );				// set initial clients
			VS( firm, "_CS1a" );				// update credit supply
		}
	}
	
	return equity;								// equity cost of entry(ies)
}


// add and configure entrant consumer-good firm object(s) and required hooks 
// in equations 'entry2exit' and 'initCountry'

double entry_firm2( variable *var, object *sector, int n, bool newInd )
{
	double A2, D20, D2e, Eavg, K, Kd, N, NW2, NW2f, NW20, Q2u, c2, f2, f2posChg, 
		   life2cycle, p2, mult, equity = 0;
	int ID2, IDb, t2ent;
	object *firm, *bank, *cli, *suppl, *broch, *vint,
		   *cap = SEARCHS( PARENTS( sector ), "Capital" ), 
		   *fin = SEARCHS( PARENTS( sector ), "Financial" ),
		   *lab = SEARCHS( PARENTS( sector ), "Labor" );

	double Deb20ratio = VS( sector, "Deb20ratio" );// bank fin. to equity ratio
	double Phi1 = VS( sector, "Phi1" );			// lower support for K share
	double Phi2 = VS( sector, "Phi2" );			// upper support for K share
	double iota = VS( sector, "iota" );			// desired inventories factor
	double mu20 = VS( sector, "mu20" );			// initial mark-up in sector 2
	double m2 = VS( sector, "m2" );				// machine output per period
	double p10 = VLS( cap, "p1avg", 1 );		// initial machine price
	double u = VS( sector, "u" );				// desired capital utilization
	double w = VS( lab, "w" );					// current wage

	if ( newInd )
	{
		double phi = VS( lab, "phi" );			// unemployment benefit rate
		double c10 = p10 / ( 1 + VS( cap, "mu1" ) );// initial unit cost sec. 1
		double c20 = INIWAGE / INIPROD;			// initial unit cost sec. 2
		double p20 = ( 1 + mu20 ) * c20;		// initial consumer-good price
		double trW = VS( PARENTS( sector ), "flagTax" ) > 0 ? 
					 VS( PARENTS( sector ), "tr" ) : 0;// tax rate on wages
		double K0 = ceil( VS( lab, "Ls0" ) * INIWAGE / 
						  p20 / n / m2 ) * m2;	// full employment K required
		double SIr0 = n * K0 / m2 / VS( sector, "eta" );// substit. real invest.
		double RD0 = VS( cap, "nu" ) * SIr0 * p10;// initial R&D expense
		
		// initial steady state demand under fair share
		D20 = ( ( SIr0 * c10 + RD0 ) * ( 1 - phi - trW ) + 
				VS( lab, "Ls0" ) * INIWAGE * phi ) / 
			  ( mu20 + phi + trW ) * c20 / n;
		Eavg = VLS( sector, "Eavg", 1 );		// initial competitiveness
		K = K0;									// initial capital in sector 2
		N = iota * D20;							// initial inventories
		NW20 = VS( sector, "NW20" );			// initial wealth in sector 2
		Q2u = 1;								// initial capacity utilization
		f2 = 1.0 / n;							// fair share
		f2posChg = 0;							// m.s. of post-change firms
		life2cycle = 1;							// start as incumbent
		t2ent = 0;								// entered before t=1
	}
	else
	{
		D20 = 0;
		Eavg = VS( sector, "Eavg" );			// average competitiveness
		K = WHTAVES( sector, "_K", "_f2" );		// w. avg. capital in sector 2
		N = 0;									// inventories
		NW20 = WHTAVES( sector, "_NW2", "_f2" );// average wealth in sector 2
		Q2u = VS( sector, "Q2u" );				// capacity utilization
		f2 = 0;									// no market share
		life2cycle = 0;							// start as pre-operat. entrant
		t2ent = T;								// entered now
	}

	// add entrant firms (end of period, don't try to sell)
	for ( ; n > 0; --n )
	{
		ID2 = INCRS( sector, "lastID2", 1 );	// new firm ID
		
		// create object, only recalculate in t if new industry
		if ( newInd )
			firm = ADDOBJLS( sector, "Firm2", T - 1 );
		else
			firm = ADDOBJS( sector, "Firm2" );
		
		ADDHOOKS( firm, FIRM2HK );				// add object hooks
		vint = SEARCHS( firm, "Vint" );			// remove empty vintage instance		
		DELETE( vint );
		
		// select associated bank and create hooks to/from it
		IDb = VS( fin, "pickBank" );			// draw bank
		bank = V_EXTS( PARENTS( sector ), country, bankPtr[ IDb - 1 ] );// bank object
		WRITES( firm, "_bank2", IDb );
		WRITE_HOOKS( firm, BANK, bank );
		cli = ADDOBJS( bank, "Cli2" );			// add to bank client list
		WRITES( cli, "_IDc2", ID2 );			// update object
		WRITE_SHOOKS( cli, firm );				// pointer back to client
		WRITE_HOOKS( firm, BCLIENT, cli );		// pointer to bank client list
		
		// draw current machine supplier and create hooks to/from it
		suppl = RNDDRAWS( cap, "Firm1", "_Atau" );// try to draw good supplier
		INCRS( suppl, "_NC", 1 );
		cli = ADDOBJS( suppl, "Cli" );			// add to supplier client list
		WRITES( cli, "_IDc", ID2 );				// update object
		WRITES( cli, "_tSel", T );
		broch = SEARCHS( firm, "Broch" );		// add to firm brochure list
		WRITES( broch, "_IDs", VS( suppl, "_ID1" ) );// update object
		WRITE_SHOOKS( broch, cli );				// pointer to supplier cli. list
		WRITE_SHOOKS( cli, broch );				// pointer to client broch. list
		WRITE_HOOKS( firm, SUPPL, broch );		// pointer to current supplier
			
		// initial desired capital/expected demand, rounded to # of machines
		mult = newInd ? 1 : uniform( Phi1, Phi2 );// capital multiple
		Kd = ceil( max( mult * K / m2, 1 ) ) * m2; 
		D2e = newInd ? D20 : u * Kd;
		
		// define entrant initial free cash (1 period wages or default minimum)
		mult = newInd ? 1 : uniform( Phi1, Phi2 );// NW multiple
		A2 = VS( suppl, "_Atau" );				// initial productivity
		c2 = w / A2;							// initial unit costs
		p2 = ( 1 + mu20 ) * c2;					// initial price
		NW2f = ( 1 + iota ) * D2e * c2;			// initial free cash
		NW2f = max( NW2f, mult * NW20 );
		
		// initial equity must pay initial capital and wages
		NW2 = newInd ? NW2f : VS( suppl, "_p1" ) * Kd / m2 + NW2f;
		equity += NW2 * ( 1 - Deb20ratio );		// accumulated equity (all firms)
		
		// initialize variables
		WRITES( firm, "_ID2", ID2 );
		WRITES( firm, "_t2ent", t2ent );
		WRITES( firm, "_life2cycle", life2cycle );		
		WRITELLS( firm, "_A2", A2, t2ent, 1 );
		WRITELLS( firm, "_Deb2", NW2 * Deb20ratio, t2ent, 1 );
		WRITELLS( firm, "_f2", f2, t2ent, 1 );
		WRITELLS( firm, "_f2", f2, t2ent, 2 );
		WRITELLS( firm, "_mu2", mu20, t2ent, 1 );
		WRITELLS( firm, "_p2", p2, t2ent, 1 );
		WRITELLS( firm, "_qc2", 4, t2ent, 1 );
		
		for ( int i = 1; i <= 4; ++i )
		{
			WRITELLS( firm, "_D2", D2e, t2ent, i );
			WRITELLS( firm, "_D2d", D2e, t2ent, i );
		}
		
		if ( newInd )
		{
			WRITELLS( firm, "_K", Kd, t2ent, 1 );
			WRITELLS( firm, "_N", N, t2ent, 1 );
			WRITELLS( firm, "_NW2", NW2, t2ent, 1 );
			
			add_vintage( firm, Kd / m2, newInd );// first machine vintages
		}
		else
		{
			WRITES( firm, "_A2", A2 );
			WRITES( firm, "_D2e", D2e ); 
			WRITES( firm, "_Deb2", NW2 * Deb20ratio );
			WRITES( firm, "_E", Eavg );
			WRITES( firm, "_Kd", Kd );
			WRITES( firm, "_NW2", NW2 );
			WRITES( firm, "_Q2u", Q2u );
			WRITES( firm, "_c2", c2 );
			WRITES( firm, "_c2e", c2 );
			WRITES( firm, "_mu2", mu20 );
			WRITES( firm, "_p2", p2 );

			// compute variables requiring calculation in t 
			RECALCS( firm, "_Deb2max" );		// prudential credit limit
			VS( firm, "_CS2a" );				// update credit supply
		}
	}

	return equity;								// equity cost of entry(ies)
}


// remove capital-good firm object and exiting hooks in equation 'entry1exit'

double exit_firm1( variable *var, object *firm )
{
	double liqVal = VS( firm, "_NW1" ) - VS( firm, "_Deb1" );
	object *bank, *firm2;
	
	if ( liqVal < 0 )							// account bank losses, if any
	{
		bank = HOOKS( firm, BANK );				// exiting firm bank
		VS( bank, "_BadDeb" );					// ensure reset in t
		INCRS( bank, "_BadDeb", - liqVal );		// accumulate bank losses
	}
	
	DELETE( HOOKS( firm, BCLIENT ) );			// leave client list of bank
	
	CYCLES( firm, firm2, "Cli" )				// leave supplier lists of s. 2
		DELETE( SHOOKS( firm2 ) );				// delete from firm broch. list
		
	DELETE( firm );
	
	return max( liqVal, 0 );					// liquidation credit, if any
}


// remove consumer-good firm object and exiting hooks in equation 'entry2exit'

double exit_firm2( variable *var, object *firm )
{
	double fires, liqVal = VS( firm, "_NW2" ) - VS( firm, "_Deb2" );
	object *bank, *firm1;

	
	if ( liqVal < 0 )							// account bank losses, if any
	{
		bank = HOOKS( firm, BANK );				// exiting firm bank
		VS( bank, "_BadDeb" );					// ensure reset in t
		INCRS( bank, "_BadDeb", - liqVal );		// accumulate bank losses
	}
	
	DELETE( HOOKS( firm, BCLIENT ) );			// leave client list of bank
	
	CYCLES( firm, firm1, "Broch" )				// leave client lists of s. 1
		DELETE( SHOOKS( firm1 ) );				// delete from firm client list
	
	// update firm map before removing LSD object
	EXEC_EXTS( GRANDPARENTS( firm ), country, firm2map, erase, ( int ) VS( firm, "_ID2" ) );
		
	DELETE( firm );

	return max( liqVal, 0 );					// liquidation credit, if any
}
