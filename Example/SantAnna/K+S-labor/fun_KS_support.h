/******************************************************************************

	SUPPORT C FUNCTIONS
	-------------------

	Pure C support functions used in the objects in the K+S LSD model are 
	coded below.
 
 ******************************************************************************/

/*======================== GENERAL SUPPORT C FUNCTIONS =======================*/

#define MOV_AVG_PER 4							// moving average period

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

void add_vintage( object *firm, double nMach, bool newInd )
{
	double _Avint, _pVint, _tVint;
	object *vint, *suppl, *wrk;
	
	// create object, only recalculate in t if new industry
	if ( newInd )
		vint = ADDOBJLS( firm, "Vint", T - 1 );
	else
		vint = ADDOBJS( firm, "Vint" );
		
	WRITE_SHOOKS( vint, HOOKS( firm, TOPVINT ) );// save previous vintage
	WRITE_HOOKS( firm, TOPVINT, vint );			// save pointer to top vintage
	
	suppl = PARENTS( SHOOKS( HOOKS( firm, SUPPL ) ) );// pointer to supplier
	_Avint = VS( suppl, "_Atau" );
	_pVint = VS( suppl, "_p1" );
	
	// distribute machine ages at t=1 to spread initial technical substitution
	if ( T == 1 )
		_tVint = T - ( uniform_int( 1, VS( PARENTS( firm ), "eta" ) ) + 1 );
	else
		_tVint = T;
		
	WRITES( vint, "_IDvint", VNT( T, VS( suppl, "_ID1" ) ) );// vintage ID
	WRITES( vint, "_Avint", _Avint );			// vintage notional productivity
	WRITES( vint, "_AeVint", _Avint );			// vintage effective product.
	WRITES( vint, "_nVint", nMach );			// number of machines in vintage
	WRITES( vint, "_pVint", _pVint );			// price of machines in vintage
	WRITES( vint, "_tVint", T );				// vintage build time
	WRITELLS( vint, "_AeVint", _Avint, T, 1 );	// lagged value
	
	wrk = SEARCHS( vint, "WrkV" );				// remove empty worker object
	DELETE( wrk );
}

// scrap (remove) vintage from capital stock in equation 'K'
// return -1 if last vintage (not removed but shrank to 1 machine)

double scrap_vintage( object *vint )
{
	double RS;
	object *cur;
	
	if ( NEXTS( vint ) != NULL )				// don't remove last vintage
	{
		// move all workers out from this vintage
		CYCLES( vint, cur, "WrkV" )
			WRITE_HOOKS( SHOOKS( cur ), VWRK, NULL );
		
		// remove as previous vintage from next vintage
		if ( SHOOKS( NEXTS( vint ) ) == vint )
			WRITE_SHOOKS( NEXTS( vint ), NULL );
		
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


/*================== LABOR MANAGEMENT SUPPORT C FUNCTIONS ====================*/

// update a worker after hiring in equations 'hire1', 'hire2'

void hire_worker( object *worker, int sec, object *firm, double wage )
{
	int flagWorkerLBU, ID = VS( worker, "_ID" );
	object *wrk;
	
	WRITES( worker, "_employed", sec );
	WRITES( worker, "_Te", 0 );
	WRITES( worker, "_CQ", 0 );					// no cumulated production yet
	WRITES( worker, "_w", wage );	
	
	flagWorkerLBU = VS( GRANDPARENTS( worker ), "flagWorkerLBU" );
	if ( flagWorkerLBU != 0 && flagWorkerLBU != 2 )
		WRITES( worker, "_sV", VS( PARENTS( worker ), "sigma" ) );// public skills
	else
		WRITES( worker, "_sV", 1 );

	// then handle the case at hand, setting vintage and bridge objects
	if ( sec < 2 )								// unemployed or sector 1?
	{
		wrk = NULL;								// no Firm2 in use
		
		if ( sec == 1 )
		{
			// add bridge object between 'Wrk1' (in Capital) to 'Worker'
			wrk = ADDOBJS( firm, "Wrk1" );
			WRITE_SHOOKS( wrk, worker );		// pointer to worker from firm
			WRITES( wrk, "_IDw1", ID );
		}
	}
	else
	{
		// add bridge-object between 'Wrk2' (in Firm2) to 'Worker'
		wrk = ADDOBJS( firm, "Wrk2" );
		WRITE_SHOOKS( wrk, worker );			// pointer to worker from firm
		WRITES( wrk, "_IDw2", ID );				// register worker ID ID
	}
	
	WRITE_HOOKS( worker, FWRK, wrk );			// pointer to firm from worker
}


// update a worker after firing in equations 'fires1', '_fires2', 'entry2exit', 
// 'quits1', 'retires1', '_quits2', '_retires2' 

void fire_worker( object *worker )
{
	WRITES( worker, "_employed", 0 );			// register fire
	WRITES( worker, "_Te", 0 );
	RECALCS( worker, "_w" );					// recalc. wage if already done
	
	// if already has a bridge object, destroy it first
	if ( HOOKS( worker, FWRK ) != NULL )
	{
		DELETE( HOOKS( worker, FWRK ) );
		WRITE_HOOKS( worker, FWRK, NULL );
		
		// and also destroy vintage bridge object 
		if ( HOOKS( worker, VWRK ) != NULL )
		{
			DELETE( HOOKS( worker, VWRK ) );
			WRITE_HOOKS( worker, VWRK, NULL );
		}
	}
}


// perform worker quitting from current firm in equations 'hires1', 'hires2'

void quit_worker( object *worker )
{
	double Lscale = VS( PARENTS( worker ), "Lscale" );// labor scaling
	
	if ( VS( worker, "_employed" ) == 1 )		// sector 1?
		INCRS( V_EXTS( GRANDPARENTS( worker ), countryE, capSec ), "quits1", Lscale );
	else										// no: assume sector 2
		INCRS( PARENTS( HOOKS( worker, FWRK ) ), "_quits2", Lscale );
	
	fire_worker( worker );						// register fire
}


// move worker to a different vintage in equation 'alloc2'

void move_worker( object *worker, object *vint, bool vint_learn )
{
	double sV;
	int IDv;
	object *wrkV;
	
	if ( vint_learn )							// learning-by-vintage mode?
	{											// worker has public skills
		IDv = VS( vint, "_IDvint" );
		sV = V_EXTS( GRANDPARENTS( vint ), countryE, vintProd[ IDv ].sVp );
	}
	else
		sV = 1;
	
	wrkV = ADDOBJS( vint, "WrkV" );				// add worker-bridge object
	WRITE_SHOOKS( wrkV, worker );				// save pointer to work object
	WRITE_HOOKS( worker, VWRK, wrkV );			// register vint. in worker
	
	WRITES( wrkV, "_IDwV", VS( worker, "_ID" ) );
	WRITES( worker, "_sV", sV );				// set vintage skills
	WRITES( worker, "_CQ", 0 );					// no cumulated production yet
}


// order wage offers in equation 'hires2'

bool wo_asc_wrk( wageOffer e1, wageOffer e2 ) { return e1.workers < e2.workers; };
bool wo_desc_off( wageOffer e1, wageOffer e2 ) { return e1.offer > e2.offer; };

void shuffle_offers( woLisT *offers )
{
	// make a copy of the workers list into a vector 
	vector < wageOffer > temp( offers->size( ) );
	copy( offers->begin( ), offers->end( ), temp.begin( ) );
	
	// shuffle firms to choose hiring order
	shuffle( temp.begin( ), temp.end( ), random_engine );
	
	// and copy it back to a list
	copy( temp.begin( ), temp.end( ), offers->begin( ) );
}

void order_offers( int order, woLisT *offers )
{
	int i;
	woLisT noWorker;
	woLisT::iterator it;
	
	// always shuffle orders to prevent preference when same wages are offered
	shuffle_offers( offers );

	if ( order == 2 || order == 3 )				// no worker firms priority?
	{
		offers->sort( wo_asc_wrk );				// sort by number of workers
		
		// find first firm with employers
		for ( i = 0, it = offers->begin( ); 
			  it->workers == 0 && it != offers->end( ); ++i, ++it );
			  
		if ( i > 0 && it != offers->end( ) )	// at least one no worker firm?
			// move no worker firm(s) to separate list
			noWorker.splice( noWorker.begin( ), *offers, offers->begin( ), it );
	}

	switch( order )
	{
		default:
		case 0:									// random order (already none)
			break;
			
		case 1:									// higher offers first
			offers->sort( wo_desc_off );		// sort by higher offers first
			break;
			
		case 2:									// no worker first, all random
			shuffle_offers( & noWorker );		// shuffle noWorker firm(s)
			shuffle_offers( offers );			// shuffle the rest of the list
			
			// insert the shuffled list of no worker firm(s) at the beginning
			offers->splice( offers->begin(), noWorker, 
							noWorker.begin(), noWorker.end() );
			break;
			
		case 3:									// no worker first + higher off.
			noWorker.sort( wo_desc_off );		// sort noWorker firm(s)
			offers->sort( wo_desc_off );		// sort the rest of the list
			
			// insert the shuffled list of no worker firm(s) at the beginning
			offers->splice( offers->begin( ), noWorker, 
							noWorker.begin( ), noWorker.end( ) );
	}
}


// sort job applications in equations 'hires1', 'hires2'

bool appl_asc_w( application e1, application e2 ) { return e1.w < e2.w; };
bool appl_desc_w( application e1, application e2 ) { return e1.w > e2.w; };
bool appl_asc_s( application e1, application e2 ) { return e1.s < e2.s; };
bool appl_desc_s( application e1, application e2 ) { return e1.s > e2.s; };
bool appl_asc_ws( application e1, application e2 ) { return e1.ws < e2.ws; };
bool appl_desc_ws( application e1, application e2 ) { return e1.ws > e2.ws; };
bool appl_asc_Te( application e1, application e2 ) { return e1.Te < e2.Te; };
bool appl_desc_Te( application e1, application e2 ) { return e1.Te > e2.Te; };

void order_applications( int order, appLisT *appls )
{
	if ( appls->size( ) == 0 )					// prevent empty lists
		return;
				
	vector < application > temp( appls->size( ) );
	
	switch ( order )
	{
		default:
		case 0:									// random order
			// make a copy of list to a vector, shuffle, and copy back
			copy( appls->begin( ), appls->end( ), temp.begin( ) );
			shuffle( temp.begin( ), temp.end( ), random_engine );
			copy( temp.begin( ), temp.end( ), appls->begin( ) );
			break;
		case 1:									// higher wage first order
			appls->sort( appl_desc_w );
			break;
		case 2:									// lower wage first order
			appls->sort( appl_asc_w );
			break;
		case 3:									// higher skills first order
			appls->sort( appl_desc_s );
			break;
		case 4:									// lower skills first order
			appls->sort( appl_asc_s );
			break;
		case 5: 								// higher payback first order
			appls->sort( appl_desc_ws );
			break;
		case 6: 								// lower payback first order
			appls->sort( appl_asc_ws );
			break;
		case 7:									// old hires first order
			appls->sort( appl_desc_Te );
			break;
		case 8:									// recent hires first order
			appls->sort( appl_asc_Te );
			break;
	}
}


// sort worker bridge objects in equation 'fires1', 'fires2'

#define OBJ_WRK1 0								// operate on sector 1 workers
#define OBJ_WRK2 1								// operate on sector 2 workers

const char *wrkName[ ] = { "Wrk1", "Wrk2" }, 
		   *keyName[ ] = { "_key1", "_key2" };
		   
void order_workers( int order, int obj, object *firm )
{
	char keyN[ 4 ], dir[ 5 ];
	double keyV;
	object *wrk;
	
	switch ( order )							// handle selected sort scheme
	{
		case 0:									// random order
			strcpy( dir, "UP" );
			break;
		case 1:									// higher wage first order
			strcpy( keyN, "_wR" );
			strcpy( dir, "DOWN" );
			break;
		case 2:									// lower wage first order
			strcpy( keyN, "_wR" );
			strcpy( dir, "UP" );
			break;
		case 3:									// higher skills first order
			strcpy( keyN, "_s" );
			strcpy( dir, "DOWN" );
			break;
		case 4:									// lower skills first order
			strcpy( keyN, "_s" );
			strcpy( dir, "UP" );
			break;
		case 5: 								// higher payback first order
			strcpy( dir, "DOWN" );
			break;
		case 6: 								// lower payback first order
			strcpy( dir, "UP" );
			break;
		case 7:									// old hires first order
			strcpy( keyN, "_Te" );
			strcpy( dir, "DOWN" );
			break;
		case 8:									// recent hires first order
			strcpy( keyN, "_Te" );
			strcpy( dir, "UP" );
	}
	
	CYCLES( firm, wrk, wrkName[ obj ] )			// update all employees
	{
		if ( order == 0 )						// random order?
			keyV = RND;
		else
			if ( order == 5 || order == 6 )
				keyV = VS( SHOOKS( wrk ), "_wR" ) /
					   VS( SHOOKS( wrk ), "_s" );
			else
				keyV = VS( SHOOKS( wrk ), keyN );
		
		WRITES( wrk, keyName[ obj ], keyV );	// copy key to bridge obj
	}
	
	SORTS( firm, wrkName[ obj ], keyName[ obj ], dir );// sort the bridge objects
}


// realize firing for firm in equation 'fires2'

#define MODE_ALL 1								// fire all workers
#define MODE_ADJ 2								// fire only for adjustment
#define MODE_PBACK 3							// fire negative paybacks
#define MODE_IPROT 4							// fire non protected workers
#define MODE_EXIT 5								// fire all when firm exiting

double fire_workers( object *firm, int mode, double xsCap, double *redCap )
{
	bool fire;
	int Te;
	object *cyccur, *wrk, *worker;
	
	object *cnt = GRANDPARENTS( firm );			// pointers to objects
	object *lab = V_EXTS( cnt, countryE, labSup );
	
	int i = 0;									// fired workers counter
	int Tp = VS( lab, "Tp" );					// time for protected workers
	double w2avg = VLS( PARENTS( firm ), "w2avg", 1 );// average wage
	double Lscale = VS( lab, "Lscale" );		// labor scale
	
	*redCap = 0;								// reduced capacity accumulator
	xsCap *= 1 - VS( lab, "theta" );			// create slack (extra workers)
	
	// order workers to fire according firm preference
	int fOrder = VS( firm, "_postChg" ) ? VS( cnt, "flagFireOrder2Chg" ) : 
										  VS( cnt, "flagFireOrder2" );
	if ( mode == MODE_PBACK )					// explicit payback firing?
		fOrder = 0;								// ignore order set
		
	// create sorted list of workers according to the chosen attributes
	order_workers( fOrder, OBJ_WRK2, firm );	// sort bridge objects
	
	// check firing worker by worker: firm desired adjustments
	CYCLE_SAFES( firm, wrk, "Wrk2" )
	{
		fire = false;
		worker = SHOOKS( wrk );
		Te = VLS( worker, "_Te", 1 ) + 1;
		
		// contract not finished and firm not exiting market?
		if ( Te < VS( worker, "_Tc" ) && mode != MODE_EXIT )
			continue;							// go to next worker
			
		switch ( mode )							// handle different modes
		{
			case MODE_ALL:						// fire all workers
			case MODE_EXIT:						// firm exiting market
				fire = true;					// simply fire
				break;
				
			case MODE_IPROT:					// fire only unprotected
				// vintage-unallocated worker or not enough fires?
				if ( HOOKS( worker, VWRK ) == NULL || *redCap < xsCap )
				{
					if ( Te <= Tp )				// is worker yet unprotected
						fire = true;
				}
				
				break;
				
			case MODE_PBACK:					// fire negative paybacks
				// insufficient payback
				if ( VS( worker, "_wR" ) / w2avg / VS( worker, "_s" ) > 1 )
					fire = true;
				// no 'break' here, even if payback is ok, fire if excess
				
			case MODE_ADJ:						// fire only for adjustment
				// vintage-unallocated worker or not enough fires?
				if ( HOOKS( worker, VWRK ) == NULL || *redCap < xsCap )
					fire = true;
				
				break;
				
			default:							// all other cases
				// just fire vintage-unallocated workers?
				if ( HOOKS( worker, VWRK ) == NULL )
					fire = true;
		}

		if ( fire )								// if marked, process firing
		{
			fire_worker( worker );				// register fire
			*redCap += VLS( worker, "_Q", 1 ) * Lscale;// pot. fired capacity
			++i;								// scaled equivalent fires
		}
	}

	return i * Lscale;
}


/*=================== FIRM ENTRY-EXIT SUPPORT C FUNCTIONS ====================*/

// add and configure entrant capital-good firm object(s) and required hooks 
// in equations 'entry1exit' and 'initCountry'

double entry_firm1( object *sector, int n, bool newInd )
{
	double Atau, AtauMax, Btau, BtauMax, D10, L1rd, NW1, NW10, RD0, c1, f1, p1, 
		   sV, w1avg, mult, equity = 0;
	int ID1, IDb;
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
	
	if ( newInd )
	{
		Atau = Btau = AtauMax = BtauMax = INIPROD;// initial productivities
		NW10 = VS( sector, "NW10" ); 			// initial wealth in sector 1
		f1 = 1.0 / n;							// fair share
		sV = VS( lab, "sAvg" );					// initial worker skills
		w1avg = INIWAGE;						// initial notional wage
		
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
		sV = 1;									// worker skills
		w1avg = VS( sector, "w1avg" );			// average wage in sector 1

		// initial demand is the expected machine replacement under fair share
		D10 = VS( cons, "K" ) / VS( cons, "m2" ) / VS( cons, "eta" ) / 
			  VS( sector, "F1" );
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
		bank = V_EXTS( PARENTS( sector ), countryE, bankPtr [ IDb - 1 ] );// bank object
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
		c1 = w1avg / ( Btau * m1 );				// unit cost
		p1 = ( 1 + mu1 ) * c1;					// unit price
		RD0 = max( nu * D10 * p1, w1avg );		// R&D expense
		L1rd = floor( RD0 / w1avg );			// workers in R&D
		mult = newInd ? 1 : uniform( Phi3, Phi4 );// NW multiple
		NW1 = mult * NW10;						// initial cash
		equity += NW1 * ( 1 - Deb10ratio );		// account public entry equity

		// initialize variables
		WRITES( firm, "_ID1", ID1 );
		WRITES( firm, "_t1ent", T );
		
		if ( newInd )
		{
			WRITELS( firm, "_Deb1", NW1 * Deb10ratio, -1 );
			WRITELS( firm, "_L1rd", L1rd, -1 );
			WRITELS( firm, "_NW1", NW1, -1 );
			WRITELS( firm, "_RD", RD0, -1 );
			WRITELS( firm, "_f1", f1, -1 );
			WRITELS( firm, "_qc1", 1, -1 );
			
			// initialize the map of vintage productivity and skills
			WRITE_EXTS( PARENTS( sector ), countryE, vintProd[ VNT( T - 1, ID1 ) ].sVp, sV );
			WRITE_EXTS( PARENTS( sector ), countryE, vintProd[ VNT( T - 1, ID1 ) ].sVavg, sV );
		}
		else
		{
			WRITES( firm, "_Atau", Atau );
			WRITES( firm, "_Btau", Btau );
			WRITES( firm, "_Deb1", NW1 * Deb10ratio );
			WRITES( firm, "_L1rd", L1rd );
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
	bool postChg;
	double A2, D20, D2e, Eavg, K, Kd, N, NW2, NW20, Q2u, c2, f2, f2posChg, 
		   life2cycle, p2, q2, w2avg, w2realAvg, cash, mult, equity = 0;
	int ID2, IDb, t2ent;
	object *firm, *bank, *cli, *suppl, *broch, *vint, *wrk,
		   *cap = SEARCHS( PARENTS( sector ), "Capital" ), 
		   *fin = SEARCHS( PARENTS( sector ), "Financial" ),
		   *lab = SEARCHS( PARENTS( sector ), "Labor" );

	bool AllFirmsChg = VS( PARENTS( sector ), "flagAllFirmsChg" );// change at once?
	bool f2critChg = VS( sector, "f2critChg" );	// critical change thresh. met?
	double Deb20ratio = VS( sector, "Deb20ratio" );// bank fin. to equity ratio
	double Phi1 = VS( sector, "Phi1" );			// lower support for K share
	double Phi2 = VS( sector, "Phi2" );			// upper support for K share
	double ent2HldShr = VS( sector, "ent2HldShr" );// hold share post-chg firms
	double f2minPosChg = VS( sector, "f2minPosChg" );// min m.s. post-chg firms
	double iota = VS( sector, "iota" );			// desired inventories factor
	double mu20 = VS( sector, "mu20" );			// initial mark-up in sector 2
	double m2 = VS( sector, "m2" );				// machine output per period
	double u = VS( sector, "u" );				// desired capital utilization
	double sV = VS( lab, "sAvg" );				// initial worker skills
	int TregChg = VS( PARENTS( sector ), "TregChg" );// time for regime change
	int eta = VS( sector, "eta" );				// technical life of machines

	if ( newInd )
	{
		double K0 = VS( sector, "K0" );			// initial capital in sector 2
		double mu1 = VS( cap, "mu1" );			// mark-up in sector 1
		double m1 = VS( cap, "m1" );			// worker output per period
		double p10 = ( 1 + mu1 ) * INIWAGE / ( INIPROD * m1 );// mach. price
		double phi = VS( fin, "phi" );			// unemployment benefit rate
		double SIr0 = n * ceil( K0 / m2 ) / eta;// substitution real investment
		double RD0 = VS( cap, "nu" ) * SIr0 * p10;// initial R&D expense

		D20 = ( ( SIr0 * INIWAGE / ( INIPROD * m1 ) + RD0 ) * ( 1 - phi ) + 
				VS( lab, "Ls0" ) * INIWAGE * phi ) / 
			  ( mu20 + phi ) * ( INIWAGE / INIPROD ) / n;// share of s.s. demand
		Eavg = ( VS( sector, "omega1" ) + VS( sector, "omega2" ) + 
				 VS( sector, "omega3" ) ) / 2;	// initial competitiveness
		K = K0;									// initial capital in sector 2
		N = iota * D20;							// initial inventories
		NW20 = VS( sector, "NW20" );			// initial wealth in sector 2
		Q2u = u;								// initial capacity utilization
		f2 = 1.0 / n;							// fair share
		f2posChg = 0;							// m.s. of post-change firms
		life2cycle = 3;							// start as incumbent
		q2 = 1;									// initial quality
		t2ent = 0;								// entered before t=1
		w2avg = w2realAvg = INIWAGE;			// initial notional wage
	}
	else
	{
		D20 = 0;								// compute later
		Eavg = VS( sector, "Eavg" );			// average competitiveness
		K = WHTAVES( sector, "_K", "_f2" );		// w. avg. capital in sector 2
		N = 0;									// inventories
		NW20 = WHTAVES( sector, "_NW2", "_f2" );// average wealth in sector 2
		Q2u = VS( sector, "Q2u" );				// capacity utilization
		f2 = 0;									// no market share
		f2posChg = VS( sector, "f2posChg" );	// m.s. of post-change firms
		life2cycle = 0;							// start as pre-operat. entrant
		q2 = VS( sector, "q2avg" );				// average quality
		t2ent = T;								// entered now
		w2avg = VS( sector, "w2avg" );			// average wage in sector 2
		w2realAvg = VS( sector, "w2realAvg" );	// average real wage in s. 2
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
		ADDEXTS( firm, firm2E );				// allocate extended data
		vint = SEARCHS( firm, "Vint" );			// remove empty vintage instance		
		DELETE( vint );
		
		// select associated bank and create hooks to/from it
		IDb = VS( fin, "pickBank" );			// draw bank
		bank = V_EXTS( PARENTS( sector ), countryE, bankPtr[ IDb - 1 ] );// bank object
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
			
		// remove empty worker instance
		wrk = SEARCHS( firm, "Wrk2" );			
		DELETE( wrk );
		
		// choose firm type (pre/post-change)
		if ( TregChg <= 0 || T < TregChg )		// before regime change?
			postChg = false;					// it's pre-change type
		else
			if ( AllFirmsChg )					// all firms change at once?
				postChg = true;					// it's post-change type
			else
				if ( ! f2critChg || newInd )	// critical threshold not met?
					// fixed type proportion
					postChg = ( RND < ent2HldShr ) ? true : false;
				else
					// draw type according shares
					postChg = ( RND < max( f2posChg, f2minPosChg ) ) ? true : false;
					
		// initial desired capital/expected demand, rounded to # of machines
		mult = newInd ? 1 : uniform( Phi1, Phi2 );// capital multiple
		Kd = ceil( max( mult * K / m2, 1 ) ) * m2; 
		D2e = newInd ? D20 : u * Kd;
		
		// define entrant initial free cash (1 period wages or default minimum)
		mult = newInd ? 1 : uniform( Phi1, Phi2 );// NW multiple
		A2 = VS( suppl, "_Atau" );				// initial productivity
		c2 = w2avg / A2;						// initial unit costs
		p2 = ( 1 + mu20 ) * c2;					// initial price
		NW2 = ( 1 + iota ) * D2e * c2;
		NW2 = max( NW2, mult * NW20 );
		
		// initial equity must pay initial capital and wages
		cash = VS( suppl, "_p1" ) * Kd / m2 + NW2;
		equity += cash * ( 1 - Deb20ratio );	// accumulated equity
			
		// initialize variables
		WRITES( firm, "_ID2", ID2 );
		WRITES( firm, "_t2ent", t2ent );
		WRITES( firm, "_life2cycle", life2cycle );		
		WRITES( firm, "_postChg", postChg );
		
		if ( newInd )
		{
			WRITELS( firm, "_Deb2", cash * Deb20ratio, -1 );
			WRITELS( firm, "_K", Kd, -1 );
			WRITELS( firm, "_N", N, -1 );
			WRITELS( firm, "_NW2", NW2, -1 );
			WRITELS( firm, "_f2", f2, -1 );
			WRITELS( firm, "_f2", f2, -1 );
			WRITELS( firm, "_mu2", mu20, -1 );
			WRITELS( firm, "_p2", p2, -1 );
			WRITELS( firm, "_qc2", 1, -1 );
			WRITELS( firm, "_s2avg", sV, -1 );
			
			for ( int i = 1; i <= 4; ++i )
			{
				WRITELS( firm, "_D2", D2e, - i );
				WRITELS( firm, "_D2d", D2e, - i );
			}
	
			// add first machine vintage
			add_vintage( firm, Kd / m2, newInd );
		}
		else
		{
			WRITES( firm, "_A2", A2 );
			WRITES( firm, "_A2p", A2 );
			WRITES( firm, "_D2e", D2e ); 
			WRITES( firm, "_Deb2", cash * Deb20ratio );
			WRITES( firm, "_E", Eavg );
			WRITES( firm, "_Kd", Kd );
			WRITES( firm, "_NW2", cash );
			WRITES( firm, "_Q2u", Q2u );
			WRITES( firm, "_c2", c2 );
			WRITES( firm, "_c2e", c2 );
			WRITES( firm, "_mu2", mu20 );
			WRITES( firm, "_p2", p2 );
			WRITES( firm, "_q2", q2 );
			WRITES( firm, "_w2avg", w2avg );
			WRITES( firm, "_w2realAvg", w2realAvg );

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

	WRITES( firm, "_life2cycle", 4 );	// mark as exiting firm
	
	// fire all workers
	*firesAcc += fires = fire_workers( firm, MODE_EXIT, 0, &fires );
	INCRS( firm, "_fires2", fires );
	
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
	EXEC_EXTS( GRANDPARENTS( firm ), countryE, firm2map, erase, ( int ) VS( firm, "_ID2" ) );
	
	DELETE_EXTS( firm, firm2E );
	DELETE( firm );

	return max( liqVal, 0 );					// liquidation credit, if any
}
