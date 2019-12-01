/******************************************************************************

	SUPPORT C FUNCTIONS
	-------------------

	Pure C support functions used in the objects in the K+S LSD model are 
	coded below.
 
 ******************************************************************************/

/*======================== GENERAL SUPPORT C FUNCTIONS =======================*/

// calculate the bounded, 4-period-moving-average growth rate of variable
// if lim is zero, there is no bounding

double mov_avg_bound( object *obj, const char *var, double lim )
{
	double prev, g, sum_g;
	int i;
	
	for ( sum_g = i = 0; i < 4; ++i )
	{
		prev = VLS( obj, var, i + 1 );
		g = ( prev != 0 ) ? VLS( obj, var, i ) / prev - 1 : 0;
		
		if ( lim > 0 )
		{
			g = min( g, lim );					// apply bounds
			g = max( g, - lim );
		}
		
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

// send new machine order in equations '_EI', '_SI'

void send_order( object *firm, double nMach )
{
	// find firm entry on supplier client list
	object *cli = SHOOKS( HOOKS( firm, SUPPL ) );
	
	if ( VS( cli, "_tOrd" ) < t )				// if first order in period
	{
		WRITES( cli, "_nOrd", nMach );			// set new order size
		WRITES( cli, "_tOrd", t );				// set order time
		WRITES( cli, "_nCan", 0 );				// no machine canceled yet
	}
	else
		INCRS( cli, "_nOrd", nMach );			// increase existing order size
}


// add new vintage to the capital stock of a firm in equation 'K'

void add_vintage( object *firm, double nMach )
{
	double Atau;
	object *vint, *suppl, *wrk;
	
	vint = ADDOBJS( firm, "Vint" );				// create new vintage
	WRITE_SHOOKS( vint, HOOKS( firm, TOPVINT ) );// save previous vintage
	WRITE_HOOKS( firm, TOPVINT, vint );			// save pointer to top vintage
	
	suppl = SHOOKS( HOOKS( firm, SUPPL ) )->up;	// pointer to supplier
	Atau = VS( suppl, "_Atau" );
	WRITES( vint, "_IDvint", VNT( t, VS( suppl, "_ID1" ) ) );// vintage ID
	WRITES( vint, "_Avint", Atau );				// vintage notional productivity
	WRITES( vint, "_AeVint", Atau );			// vintage effective product.
	WRITES( vint, "_nVint", nMach );			// number of machines in vintage
	WRITES( vint, "_tVint", t );				// vintage build time
	WRITELLS( vint, "_AeVint", Atau, t, 1 );	// lagged value
	
	wrk = SEARCHS( vint, "WrkV" );				// remove empty worker object
	DELETE( wrk );
}

// scrap (remove) vintage from capital stock in equation 'K'
// return -1 if last vintage (not removed but shrank to 1 machine)

double scrap_vintage( object *vint )
{
	double RS;
	object *cur;
	
	if ( vint->next != NULL )					// don't remove last vintage
	{
		// move all workers out from this vintage
		CYCLES( vint, cur, "WrkV" )
			WRITE_HOOKS( SHOOKS( cur ), VWRK, NULL );
		
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
	
	flagWorkerLBU = VS( worker->up->up, "flagWorkerLBU" );
	if ( flagWorkerLBU != 0 && flagWorkerLBU != 2 )
		WRITES( worker, "_sV", VS( worker->up, "sigma" ) );// public skills
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
	double Lscale = VS( worker->up, "Lscale" );	// labor scaling
	
	if ( VS( worker, "_employed" ) == 1 )		// sector 1?
		INCRS( V_EXTS( worker->up->up, country, capSec ), "quits1", Lscale );
	else										// no: assume sector 2
		INCRS( HOOKS( worker, FWRK )->up, "_quits2", Lscale );
	
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
		sV = V_EXTS( vint->up->up, country, vintProd[ IDv ].sVp );
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
int rand_int( int max ) { return uniform_int( 0, max - 1 ); }

void shuffle_offers( woLisT *offers )
{
	// make a copy of the workers list into a vector 
	vector < wageOffer > temp( offers->size( ) );
	copy( offers->begin( ), offers->end( ), temp.begin( ) );
	
	// shuffle firms to choose hiring order
	random_shuffle( temp.begin( ), temp.end( ), rand_int );
	
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
			random_shuffle( temp.begin( ), temp.end( ), rand_int );
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
	
	object *cnt = firm->up->up;					// pointers to objects
	object *lab = V_EXTS( cnt, country, labSup );
	
	int i = 0;									// fired workers counter
	int Tp = VS( lab, "Tp" );					// time for protected workers
	double w2avg = VLS( firm->up, "w2avg", 1 );	// average wage
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
	double Atau, AtauMax, AtauMin, Btau, BtauMax, BtauMin, F1, F2, NW1, NW10u, 
		   RD, c1, f1, p1, sV, w1avg, mult, equity = 0;
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
	double nu = VS( sector, "nu" );				// R&D share of sales
	double n1 = VS( sector, "n1" );				// maximum time without clients
	double x5 = VS( sector, "x5" );				// entrant upper advantage
	
	if ( newInd )
	{
		Atau = Btau = AtauMax = AtauMin = BtauMax = BtauMin = 1;
		F1 = n;									// initial number of firms
		F2 = VS( cons, "F20" );					// initial of firms in sector 2
		NW10u = VS( sector, "NW10" ); 			// initial wealth in sector 1
		f1 = 1.0 / n;							// fair share
		sV = VS( lab, "sAvg" );					// initial worker skills
		w1avg = 1;								// initial wage
	}
	else
	{
		AtauMax = MAXS( sector, "_Atau" );		// best machine productivity
		AtauMin = MINS( sector, "_Atau" );		// worse machine productivity
		BtauMax = MAXS( sector, "_Btau" );		// best productivity in sector 1
		BtauMin = MINS( sector, "_Btau" );		// worse productivity in s. 1
		F1 = VS( sector, "F1" );				// current number of firms
		F2 = VS( cons, "F2" );					// number of firms in sector 2
		NW10u = VS( sector, "NW10" ) * 			// minimum wealth in s. 1
				VS( sector, "PPI" ) / VS( sector, "PPI0" );
		f1 = 0;									// no market share
		sV = 1;									// worker skills
		w1avg = VS( sector, "w1avg" );			// average wage in sector 1
		mult = 1;								// no NW multiple
	}
	
	// add entrant firms (end of period, don't try to sell)
	for ( ; n > 0; --n )
	{
		ID1 = INCRS( sector, "lastID1", 1 );	// new firm ID
		
		// create object, only recalculate in t if new industry
		if ( newInd )
			firm = ADDOBJLS( sector, "Firm1", t - 1 );
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

		if ( newInd )
			mult = uniform( Phi3, Phi4 );		// NW multiple
		else
		{
			// initial technological (imitation from best firm)
			Atau = beta( alpha2, beta2 );		// draw A from Beta(alpha,beta)
			Atau *= AtauMax * ( 1 + x5 );		// fraction of top firm
			Atau = max( Atau, AtauMin );		// bounded by the worse firm
			Btau = beta( alpha2, beta2 );		// draw B from Beta(alpha,beta)
			Btau *= BtauMax * ( 1 + x5 );		// fraction of top firm
			Btau = max( Btau, BtauMin );		// bounded by the worse firm
		}
		
		// initial cost, price and net wealth to pay at least for initial R&D
		c1 = w1avg / ( Btau * m1 );				// unit cost
		p1 = ( 1 + mu1 ) * c1;					// unit price
		RD = max( n1 * nu * p1 * F2 / F1, w1avg );// expected R&D initial cost 
		NW1 = max( RD, mult * NW10u );			// initial cash
		equity += NW1 * ( 1 - Deb10ratio );		// account public entry equity

		// initialize variables
		WRITES( firm, "_ID1", ID1 );
		WRITES( firm, "_t1ent", t );
		
		if ( newInd )
		{
			WRITELS( firm, "_Deb1", NW1 * Deb10ratio, -1 );
			WRITELS( firm, "_NW1", NW1, -1 );
			WRITELS( firm, "_RD", RD, -1 );
			WRITELS( firm, "_f1", f1, -1 );
			WRITELS( firm, "_qc1", 1, -1 );
			
			// initialize the map of vintage productivity and skills
			WRITE_EXTS( sector->up, country, vintProd[ VNT( t - 1, ID1 ) ].sVp, sV );
			WRITE_EXTS( sector->up, country, vintProd[ VNT( t - 1, ID1 ) ].sVavg, sV );
		}
		else
		{
			WRITES( firm, "_Atau", Atau );
			WRITES( firm, "_Btau", Btau );
			WRITES( firm, "_Deb1", NW1 * Deb10ratio );
			WRITES( firm, "_NW1", NW1 );
			WRITES( firm, "_RD", RD );
			WRITES( firm, "_c1", c1 );
			WRITES( firm, "_p1", p1 );
			
			// compute variables requiring calculation in t 
			RECALCS( firm, "_Deb1max" );		// prudential credit limit
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
	double A2, D2e, Eavg, K, Kd, NW2, NW2f, NW20u, Q2u, c2, f2, 
		   f2posChg, life2cycle, p2, q2, w2avg, w2realAvg, mult, equity = 0;
	int ID2, IDb;
	object *firm, *bank, *cli, *suppl, *broch, *vint, *wrk,
		   *cap = SEARCHS( sector->up, "Capital" ), 
		   *fin = SEARCHS( sector->up, "Financial" ),
		   *lab = SEARCHS( sector->up, "Labor" );

	bool AllFirmsChg = VS( sector->up, "flagAllFirmsChg" );// change at once?
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
	int TregChg = VS( sector->up, "TregChg" ); 	// time for regime change

	if ( newInd )
	{
		Eavg = 1;								// initial competitiveness
		K = VS( sector, "K0" );					// initial capital in sector 2
		NW20u = VS( sector, "NW20" );			// initial wealth in sector 2
		Q2u = u;								// initial capacity utilization
		f2 = 1.0 / n;							// fair share
		f2posChg = 0;							// m.s. of post-change firms
		life2cycle = 3;							// start as incumbent
		q2 = 1;									// initial quality
		w2avg = w2realAvg = 1;					// initial wage
	}
	else
	{
		Eavg = VS( sector, "Eavg" );			// average competitiveness
		K = WHTAVES( sector, "_K", "_f2" );		// w. avg. capital in sector 2
		NW20u = VS( sector, "NW20" ) * VS( cap, "PPI" ) / VS( cap, "PPI0" );
												// minimum wealth in sector 2
		Q2u = VS( sector, "Q2u" );				// capacity utilization
		f2 = 0;									// no market share
		f2posChg = VS( sector, "f2posChg" );	// m.s. of post-change firms
		life2cycle = 0;							// start as pre-operat. entrant
		q2 = VS( sector, "q2avg" );				// average quality
		w2avg = VS( sector, "w2avg" );			// average wage in sector 2
		w2realAvg = VS( sector, "w2realAvg" );	// average real wage in s. 2
	}

	// add entrant firms (end of period, don't try to sell)
	for ( ; n > 0; --n )
	{
		ID2 = INCRS( sector, "lastID2", 1 );	// new firm ID
		
		// create object, only recalculate in t if new industry
		if ( newInd )
			firm = ADDOBJLS( sector, "Firm2", t - 1 );
		else
			firm = ADDOBJS( sector, "Firm2" );
		
		ADDHOOKS( firm, FIRM2HK );				// add object hooks
		ADDEXTS( firm, firm2 );					// allocate extended data
		
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
		WRITES( cli, "_tSel", t );
		broch = SEARCHS( firm, "Broch" );		// add to firm brochure list
		WRITES( broch, "_IDs", VS( suppl, "_ID1" ) );// update object
		WRITE_SHOOKS( broch, cli );				// pointer to supplier cli. list
		WRITE_SHOOKS( cli, broch );				// pointer to client broch. list
		WRITE_HOOKS( firm, SUPPL, broch );		// pointer to current supplier
			
		// remove empty worker instance
		wrk = SEARCHS( firm, "Wrk2" );			
		DELETE( wrk );
		
		// choose firm type (pre/post-change)
		if ( TregChg <= 0 || t < TregChg )		// before regime change?
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
		mult = uniform( Phi1, Phi2 );
		Kd = ceil( max( mult * K / m2, 1 ) ) * m2; 
		D2e = u * Kd;
		
		// define entrant initial free cash (1 period wages or default minimum)
		A2 = VS( suppl, "_Atau" );				// initial productivity
		c2 = w2avg / A2;						// initial unit costs
		p2 = ( 1 + mu20 ) * c2;					// initial price
		NW2f = ( 1 + iota ) * D2e * c2;
		NW2f = max( NW2f, ( newInd ? mult : 1 ) * NW20u );
		
		// initial net worth allows financing initial capital and pay wages
		NW2 = VS( suppl, "_p1" ) * Kd / m2 + NW2f;
		equity += NW2 * ( 1 - Deb20ratio );		// accumulated equity
		
		// initialize variables
		WRITES( firm, "_ID2", ID2 );
		WRITES( firm, "_t2ent", t );
		WRITES( firm, "_life2cycle", life2cycle );		
		WRITES( firm, "_postChg", postChg );
		
		if ( newInd )
		{
			WRITELS( firm, "_Deb2", NW2 * Deb20ratio, -1 );
			WRITELS( firm, "_K", Kd, -1 );
			WRITELS( firm, "_NW2", NW2f, -1 );
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
	
			// set first machine vintage
			vint = SEARCHS( firm, "Vint" );
			WRITES( vint, "_IDvint", VNT( t - 1, VS( suppl, "_ID1" ) ) );
			WRITES( vint, "_tVint", t );		// build time
			WRITES( vint, "_nVint", Kd / m2 );	// number of machines
			WRITES( vint, "_Avint", A2 );
			WRITELS( vint, "_AeVint", A2, -1 );
			WRITE_HOOKS( firm, TOPVINT, vint );	// save pointer to top vintage
			
			// remove empty worker instance
			wrk = SEARCHS( vint, "WrkV" );			
			DELETE( wrk );
		}
		else
		{
			WRITES( firm, "_A2", A2 );
			WRITES( firm, "_A2p", A2 );
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
			WRITES( firm, "_q2", q2 );
			WRITES( firm, "_w2avg", w2avg );
			WRITES( firm, "_w2realAvg", w2realAvg );

			// compute variables requiring calculation in t 
			RECALCS( firm, "_Deb2max" );		// prudential credit limit
			VS( firm, "_cred2" );				// update available credit
			
			// remove empty vintage instance
			vint = SEARCHS( firm, "Vint" );			
			DELETE( vint );
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
	EXEC_EXTS( firm->up->up, country, firm2map, erase, ( int ) VS( firm, "_ID2" ) );
		
	DELETE( firm );

	return max( liqVal, 0 );					// liquidation credit, if any
}
