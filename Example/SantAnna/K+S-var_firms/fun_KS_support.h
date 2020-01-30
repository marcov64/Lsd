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


// update firm debt in equation 'Q1', '_Tax1'

void update_debt1( object *firm, double desired, double loan )
{
	INCRS( firm, "_Deb1", loan );				// increment firm's debt stock

	if ( loan > 0 && desired > loan )			// ignore loan repayment
		INCRS( firm, "_cred1c", desired - loan );// credit constraint
	
	object *bank = HOOKS( firm, BANK );			// firm's bank
	double TC1free = VS( bank, "_TC1free" );	// available credit firm's bank

	// if credit limit active, adjust bank's available credit
	if ( TC1free > -0.1 )
		WRITES( bank, "_TC1free", max( TC1free - loan, 0 ) );
}


// update firm debt in equations '_Q2', '_EI', '_SI', '_Tax2'

void update_debt2( object *firm, double desired, double loan )
{
	INCRS( firm, "_Deb2", loan );				// increment firm's debt stock
	
	if ( loan > 0 && desired > loan )			// ignore loan repayment
		INCRS( firm, "_cred2c", desired - loan );// credit constraint
	
	object *bank = HOOKS( firm, BANK );			// firm's bank
	double TC2free = VS( bank, "_TC2free" );	// available credit at firm's bank
	
	// if credit limit active, adjust bank's available credit
	if ( TC2free > -0.1 )
		WRITES( bank, "_TC2free", max( TC2free - loan, 0 ) );
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


// add new vintage to the capital stock of a firm in equation 'K' and 'initCountry'

object *add_vintage( object *firm, double nMach, object *suppl, bool newInd )
{
	double _Avint, _pVint;
	object *vint, *wrk;
	
	// create object, only recalculate in t if new industry
	if ( newInd )
		vint = ADDOBJLS( firm, "Vint", T - 1 );
	else
		vint = ADDOBJS( firm, "Vint" );
		
	WRITE_SHOOKS( vint, HOOKS( firm, TOPVINT ) );// save previous vintage
	WRITE_HOOKS( firm, TOPVINT, vint );			// save pointer to top vintage		
	
	_Avint = VS( suppl, "_Atau" );
	_pVint = VS( suppl, "_p1" );
	WRITES( vint, "_IDvint", VNT( T, VS( suppl, "_ID1" ) ) );// vintage ID
	WRITES( vint, "_Avint", _Avint );			// vintage productivity
	WRITES( vint, "_nVint", nMach );			// number of machines in vintage
	WRITES( vint, "_pVint", _pVint );			// price of machines in vintage
	WRITES( vint, "_tVint", T );				// vintage build time
	
	return vint;
}


// scrap (remove) vintage from capital stock in equation 'K'
// return -1 if last vintage (not removed but shrank to 1 machine)

double scrap_vintage( object *vint )
{
	double RS;
	
	if ( vint->next != NULL )					// don't remove last vintage
	{
		// remove as previous vintage from next vintage
		if ( SHOOKS( vint->next ) == vint )
			WRITE_SHOOKS( vint->next, NULL );
		
		RS = abs( VS( vint, "_RS" ) );					
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

double entry_firm1( object *sector, int n, bool newInd )
{
	double Atau, AtauMax, Btau, BtauMax, D10, NW1, NW10, RD0, c1, f1, p1, mult, 
		   equity = 0;
	int ID1, IDb;
	object *firm, *bank, *cli, 
		   *cons = SEARCHS( sector->up, "Consumption" ), 
		   *fin = SEARCHS( sector->up, "Financial" ),
		   *lab = SEARCHS( sector->up, "Labor" );
	
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
		Atau = Btau = AtauMax = BtauMax = 1;
		NW10 = VS( sector, "NW10" ); 			// initial wealth in sector 1
		f1 = 1.0 / n;							// fair share
		
		// initial demand expectation, assuming all sector 2 firms, 
		// 1/eta replacement factor and fair share in sector 1
		D10 = ceil( VS( cons, "K0" ) / VS( cons, "m2" ) ) / VS( cons, "eta" ) * 
			  VS( cons, "F20" ) / n;
	}
	else
	{
		AtauMax = MAXS( sector, "_Atau" );		// best machine productivity
		BtauMax = MAXS( sector, "_Btau" );		// best productivity in sector 1
		NW10 = max( WHTAVES( sector, "_NW1", "_f1" ), VS( sector, "NW10" ) * 
					VS( sector, "PPI" ) / VS( sector, "PPI0" ) );
		f1 = 0;									// no market share
		
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
		bank = V_EXTS( sector->up, country, bankPtr [ IDb - 1 ] );// bank object
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
			// initial technological (imitation from best firm)
			Atau = beta( alpha2, beta2 );		// draw A from Beta(alpha,beta)
			Atau *= AtauMax * ( 1 + x5 );		// fraction of top firm
			Btau = beta( alpha2, beta2 );		// draw B from Beta(alpha,beta)
			Btau *= BtauMax * ( 1 + x5 );		// fraction of top firm
		}
		
		// initial cost, price and net wealth to pay at least for initial R&D
		c1 = w / ( Btau * m1 );					// unit cost
		p1 = ( 1 + mu1 ) * c1;					// unit price
		RD0 = nu * D10 * p1;					// R&D expense
		mult = newInd ? 1 : uniform( Phi3, Phi4 );// NW multiple
		NW1 = mult * NW10;						// initial cash
		equity += NW1 * ( 1 - Deb10ratio );		// account public entry equity

		// initialize variables
		WRITES( firm, "_ID1", ID1 );
		WRITES( firm, "_t1ent", T );
		
		if ( newInd )
		{
			WRITELS( firm, "_Deb1", NW1 * Deb10ratio, -1 );
			WRITELS( firm, "_L1rd", RD0 / w, -1 );
			WRITELS( firm, "_NW1", NW1, -1 );
			WRITELS( firm, "_RD", RD0, -1 );
			WRITELS( firm, "_f1", f1, -1 );
			WRITELS( firm, "_qc1", 1, -1 );
		}
		else
		{
			WRITES( firm, "_Atau", Atau );
			WRITES( firm, "_Btau", Btau );
			WRITES( firm, "_Deb1", NW1 * Deb10ratio );
			WRITES( firm, "_L1rd", RD0 / w );
			WRITES( firm, "_NW1", NW1 );
			WRITES( firm, "_RD", RD0 );
			WRITES( firm, "_c1", c1 );
			WRITES( firm, "_p1", p1 );
			
			// compute variables requiring calculation in t 
			RECALCS( firm, "_Deb1max" );		// prudential credit limit
			RECALCS( firm, "_NC" );				// set initial clients
			VS( firm, "_cred1" );				// update available credit
		}
	}
	
	return equity;								// equity cost of entry(ies)
}


// add and configure entrant consumer-good firm object(s) and required hooks 
// in equations 'entry2exit' and 'initCountry'

double entry_firm2( object *sector, int n, bool newInd )
{
	double A2, D20, D2e, Eavg, Inom, K, Kd, N, NW2, NW2f, NW20, Q2u, c2, f2, 
		   f2posChg, life2cycle, p2, mult, equity = 0;
	int ID2, IDb, nMach, nVint, tVint, t2ent;
	object *firm, *bank, *cli, *cur, *suppl, *broch, *vint,
		   *cap = SEARCHS( sector->up, "Capital" ), 
		   *fin = SEARCHS( sector->up, "Financial" ),
		   *lab = SEARCHS( sector->up, "Labor" );

	double Deb20ratio = VS( sector, "Deb20ratio" );// bank fin. to equity ratio
	double Phi1 = VS( sector, "Phi1" );			// lower support for K share
	double Phi2 = VS( sector, "Phi2" );			// upper support for K share
	double iota = VS( sector, "iota" );			// desired inventories factor
	double mu1 = VS( cap, "mu1" );				// mark-up in sector 1
	double mu20 = VS( sector, "mu20" );			// initial mark-up in sector 2
	double m1 = VS( cap, "m1" );				// worker output per period
	double m2 = VS( sector, "m2" );				// machine output per period
	double p10 = ( 1 + mu1 ) / m1;				// initial price of machines
	double u = VS( sector, "u" );				// desired capital utilization
	double w = VS( lab, "w" );					// current wage
	int eta = VS( sector, "eta" );				// technical life of machines

	if ( newInd )
	{
		// fair share of initial steady state demand
		double K0 = VS( sector, "K0" );			// initial capital in sector 2
		double phi = VS( fin, "phi" );			// unemployment benefit rate
		double SIr0 = n * ceil( K0 / m2 ) / eta;// substitution real investment
		double RD0 = VS( cap, "nu" ) * SIr0 * p10;// initial R&D expense
		D20 = ( ( SIr0 / m1 + RD0 ) * ( 1 - phi ) + VS( lab, "Ls0" ) * phi ) / 
			  ( mu20 + phi ) / n;
		
		Eavg = 1;								// initial competitiveness
		K = K0;									// initial capital in sector 2
		N = iota * D20;							// initial inventories
		NW20 = VS( sector, "NW20" );			// initial wealth in sector 2
		Q2u = u;								// initial capacity utilization
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
		bank = V_EXTS( sector->up, country, bankPtr[ IDb - 1 ] );// bank object
		WRITES( firm, "_bank2", IDb );
		WRITE_HOOKS( firm, BANK, bank );
		cli = ADDOBJS( bank, "Cli2" );			// add to bank client list
		WRITES( cli, "_IDc2", ID2 );			// update object
		WRITE_SHOOKS( cli, firm );				// pointer back to client
		WRITE_HOOKS( firm, BCLIENT, cli );		// pointer to bank client list
		
		// draw current machine supplier and create hooks to/from it
		suppl = RNDDRAW_FAIRS( cap, "Firm1" );	// draw a supplier
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
		NW2f = ( 1 + iota ) * D2e * c2;
		NW2f = max( NW2f, mult * NW20 );
		
		if ( newInd )
		{	// at t=1 firms have a mix of machines: old to new, many suppliers
			nMach = Kd / m2;					// number of machines
			tVint = eta + 1;					// age of oldest machine
			nVint = ceil( Kd / m2 / tVint );	// initial machines per vintage
			Inom = 0;							// nominal firm investment acc.
			while ( nMach > 0 )
			{
				cur = RNDDRAW_FAIRS( cap, "Firm1" );// draw a supplier
				
				if ( cur == suppl )				// don't use current supplier
					continue;
					
				vint = add_vintage( firm, nVint, cur, newInd );// add vintage
				WRITES( vint, "_Avint", 1 );	// adjust vintage productivity
				WRITES( vint, "_pVint", p10 );	// adjust vintage price
				WRITES( vint, "_tVint", 1 - tVint );// and age
				Inom += VS( cur, "_p1" ) * nVint;// investment required
				nMach -= nVint;
				--tVint;
				
				if ( tVint > 0 && nMach % tVint == 0 )// exact ratio missing?
					nVint = nMach / tVint;		// reduce machines per vintage
			}
			
			NW2 = NW2f;							// operational (wage) cash only
			equity += ( Inom + NW2 ) * ( 1 - Deb20ratio );
		}
		else
		{	// initial net worth allows financing initial capital and pay wages
			NW2 = VS( suppl, "_p1" ) * Kd / m2 + NW2f;
			equity += NW2 * ( 1 - Deb20ratio );	// accumulated equity
		}
		
		// initialize variables
		WRITES( firm, "_ID2", ID2 );
		WRITES( firm, "_t2ent", t2ent );
		WRITES( firm, "_life2cycle", life2cycle );		
		
		if ( newInd )
		{
			WRITELS( firm, "_Deb2", NW2 * Deb20ratio, -1 );
			WRITELS( firm, "_K", Kd, -1 );
			WRITELS( firm, "_N", N, -1 );
			WRITELS( firm, "_NW2", NW2f, -1 );
			WRITELS( firm, "_f2", f2, -1 );
			WRITELS( firm, "_f2", f2, -1 );
			WRITELS( firm, "_mu2", mu20, -1 );
			WRITELS( firm, "_p2", p2, -1 );
			WRITELS( firm, "_qc2", 1, -1 );
			
			for ( int i = 1; i <= 4; ++i )
			{
				WRITELS( firm, "_D2", D2e, - i );
				WRITELS( firm, "_D2d", D2e, - i );
			}
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
			VS( firm, "_cred2" );				// update available credit
		}
	}

	return equity;								// equity cost of entry(ies)
}


// remove capital-good firm object and exiting hooks in equation 'entry1exit'

double exit_firm1( object *firm )
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

double exit_firm2( object *firm, double *firesAcc )
{
	double fires, liqVal = VS( firm, "_NW2" ) - VS( firm, "_Deb2" );
	object *bank, *firm1;

	// fire all workers
	*firesAcc += fires = VS( firm, "_L2" );
	
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
	EXEC_EXTS( firm->up->up, country, firm2map, erase, ( int ) VS( firm, "_ID2" ) );
		
	DELETE( firm );

	return max( liqVal, 0 );					// liquidation credit, if any
}
