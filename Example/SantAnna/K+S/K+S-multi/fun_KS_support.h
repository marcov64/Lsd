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

CFUN_DBL( mov_avg_bound, const char *var, double lim, double per = 4, int lag = 0 )
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


// order luxury-good orders in equation 'DcLux'

CFUN_VOID( shuffle_orders, buyLisT *orders )
{
	// make a copy of the list into a vector
	buyVecT temp( orders->size( ) );
	copy( orders->begin( ), orders->end( ), temp.begin( ) );

	// shuffle orders to choose buying order
	shuffle( temp.begin( ), temp.end( ), random_engine );

	// and copy it back to a list
	copy( temp.begin( ), temp.end( ), orders->begin( ) );
}


/*====================== FINANCIAL SUPPORT C FUNCTIONS =======================*/

// add and configure new bank object(s) in equation 'initCountry'

CFUN_DBL( entry_bank, int n, bool init )
{
	int _IDb, cliB, cliBacc, firstID;
	object *bank, *cli;

	double alphaB = V( "alphaB" );				// bank size distrib. parameter

	// set initial conditions for adding new bank(s)
	if ( init )
	{
		_IDb = 1;								// start ID from 1 if initializ.
		cliBacc = 0;							// no clients yet
	}
	else
	{
		_IDb = MAX( "_IDb" ) + 1;				// continue from last id
		cliBacc = SUML( "_Cl", 1 );				// existing customers
	}

	// add and configure new bank(s) objects
	for ( firstID = _IDb; _IDb < firstID + n; ++_IDb )
	{
		if ( T == 1 && _IDb == 1 )
			bank = SEARCH( "Bank" );			// first? object instance exists
		else
			bank = ADDOBJL( "Bank", T - 1 );	// recalculate in t

		cliBacc += cliB = pareto( 1, alphaB );	// draw the number of clients

		WRITES( bank, "_IDb", _IDb );
		WRITES( bank, "_fd", cliB );			// desired number of clients

		// remove empty client instances
		cli = SEARCHS( bank, "Cli1" );
		DELETE( cli );
		cli = SEARCHS( bank, "Cli2" );
		DELETE( cli );
	}

	// adjust desired clients to desired market shares
	CYCLE( bank, "Bank" )
		if ( VS( bank, "_IDb" ) >= firstID )
			WRITES( bank, "_fd", VS( bank, "_fd" ) / cliBacc );

	return 0;									// no entry costs till now
}


// initialize the assets of existing bank objects in equation 'initCountry'

CFUN_DBL( init_bank, bool init )
{
	double _Depo, _Loans, _NWb, _fB, NWb = 0;
	int cliB, cliBacc;
	object *bank, *cli;

	double NWb0 = V( "NWb0" );					// initial capital multiple
	double tauB = V( "tauB" );					// capital adequacy rate

	// scan all banks, considering the existing deposits and loans
	cliBacc = 0;
	CYCLE( bank, "Bank" )
	{
		_Loans = _Depo = cliB = 0;

		CYCLES( bank, cli, "Cli1" )
		{
			_Loans += VLS( SHOOKS( cli ), "_Deb1", 1 );// firm debt
			_Depo += VLS( SHOOKS( cli ), "_NW1", 1 );// firm net wealth
			++cliB;
		}

		CYCLES( bank, cli, "Cli2" )
		{
			_Loans += VLS( SHOOKS( cli ), "_Deb2", 1 );// firm debt
			_Depo += VLS( SHOOKS( cli ), "_NW2", 1 );// firm net wealth
			++cliB;
		}

		WRITELLS( bank, "_Loans",  _Loans, T - 1, 1 );// bank loans
		WRITELLS( bank, "_Depo",  _Depo, T - 1, 1 );// bank deposits
		WRITELLS( bank, "_BadDeb",  0, T - 1, 1 );// no bad debt
		WRITELLS( bank, "_Bda",  0, T - 1, 1 );	// no fragility
		WRITELLS( bank, "_fB", cliB, T - 1, 1 );// number of clients

		cliBacc += cliB;
	}

	// scan all banks again, allocating market shares and capital
	CYCLE( bank, "Bank" )
	{
		_fB = VLS( bank, "_fB", 1 ) / cliBacc;	// market share
		_NWb = tauB * VLS( bank, "_Loans", 1 );	// compulsory capital
		_NWb *= init ? NWb0 : 1;				// addt'l initial capital
		NWb += _NWb;

		WRITELLS( bank, "_fB", _fB, T - 1, 1 );
		WRITELLS( bank, "_NWb", _NWb, T - 1, 1 );
	}

	WRITELL( "NWb", NWb, 0, 1 );

	return SUML( "_NWb", 1 );					// entry equity cost
}


// update firm debt in equation 'Q1', '_Tax1'

CFUN_VOID( update_debt1, double desired, double loan )
{
	INCR( "_Deb1", loan );						// increment firm's debt stock

	if ( loan > 0 && desired > loan )			// ignore loan repayment
		INCR( "_cred1c", desired - loan );		// credit constraint

	object *bank = HOOK( BANK );				// firm's bank
	double _TC1free = VS( bank, "_TC1free" );	// available credit firm's bank

	// if credit limit active, adjust bank's available credit
	if ( _TC1free > -0.1 )
		WRITES( bank, "_TC1free", max( _TC1free - loan, 0 ) );
}


// update firm debt in equations '_Q2', '_EI2', '_SI2', '_Tax2'

CFUN_VOID( update_debt2, double desired, double loan )
{
	INCR( "_Deb2", loan );						// increment firm's debt stock

	if ( loan > 0 && desired > loan )			// ignore loan repayment
		INCR( "_cred2c", desired - loan );		// credit constraint

	object *bank = HOOK( BANK );				// firm's bank
	double _TC2free = VS( bank, "_TC2free" );	// available credit at firm's bank

	// if credit limit active, adjust bank's available credit
	if ( _TC2free > -0.1 )
		WRITES( bank, "_TC2free", max( _TC2free - loan, 0 ) );
}


// comparison function for sort method in equation 'cScores'

bool rank_desc_NWtoS( firmRank e1, firmRank e2 )
{
	return e1.NWtoS > e2.NWtoS;
}


/*================== CAPITAL MANAGEMENT SUPPORT C FUNCTIONS ==================*/

// add new generation (technological paradigm) of machines in equations
// 'initCountry' and 'topGen'

CFUN_DBL( add_generation )
{
	double A1top, DeltaG, _A1, _A1g, _B1, _B1g, h, topMix;
	object *cur, *gen, *lastGen = SHOOK;

	if ( lastGen == NULL )						// initial generation?
	{
		gen = SEARCH( "T1" );					// get existing object
		_A1g = _B1g = topMix = INIPROD;			// notional initial productivity
	}
	else
	{
		gen = ADDOBJL( "T1", T - 1 );			// add new object (to update in t)

		// use last generation as minimum reference
		A1top = VS( lastGen, "_A1g" );
		topMix = A1top * VS( lastGen, "_B1g" );

		CYCLE( cur, "Firm1" )					// search best among firms
		{
			_A1 = VLS( cur, "_A1", 1 );
			_B1 = VLS( cur, "_B1", 1 );

			if ( _A1 > A1top )					// absolute maximum productivity
				A1top = _A1;

			if ( _A1 * _B1 > topMix )			// maximum mixed productivity
				topMix = _A1 * _B1;
		}

		// draw new productivities, A1 not worse than last technological generation
		h = V( "h" );							// new generation max jump
		_A1g = uniform( A1top, ( 1 + h ) * A1top );
		// ensure new generation is competitive by limiting minimum B1
		_B1g = topMix / A1top;					// absolute minimum to compete
		_B1g = uniform( _B1g, ( 1 + h ) * _B1g );
	}

	// insert technology in the hook chains and map table
	WRITE_SHOOKS( gen, lastGen );
	WRITE_SHOOK( gen );
	EXEC_EXTS( PARENT, countryE, g1ptr, push_back, gen );
	int _IDg = EXEC_EXTS( PARENT, countryE, g1ptr, size ) - 1;

	// initialize technology parameters
	WRITES( gen, "_IDg", _IDg );
	WRITES( gen, "_tG", T );
	WRITES( gen, "_A1g", _A1g );
	WRITES( gen, "_B1g", _B1g );

	// update the technology gap from previously explored generations
	if ( lastGen != NULL )
	{
		DeltaG = log( _A1g * _B1g / ( VS( lastGen, "_A1g" ) * VS( lastGen, "_B1g" ) ) );
		INCRS( PARENT, "DeltaG", DeltaG );
	}
	else
	{
		DeltaG = 0;
		WRITES( PARENT, "DeltaG", DeltaG );
	}

	LOG( "\n New machine paradigm (t=%g): IDg=%d A1g=%.3g B1g=%.3g A1/B1=%.3g DeltaG=%.3g",
		 T, _IDg, _A1g, _B1g, _A1g / _B1g, DeltaG );

	return _IDg;
}


// send machine brochure to consumption-good client firm in equation '_NC1'

CFUN_OBJ( send_brochure, object *client )
{
	object *broch, *cli;

	cli = ADDOBJ( "Cli" );						// add object to new client
	WRITES( cli, "_IDc", VS( client, "_ID2" ) );// client ID
	WRITES( cli, "_tSel", T );					// update selection time

	broch = ADDOBJS( client, "Broch" );			// add brochure to client
	WRITES( broch, "_IDs", V( "_ID1" ) );		// supplier ID
	WRITE_SHOOKS( broch, cli );					// pointer to supplier client list
	WRITE_SHOOKS( cli, broch );					// pointer to client brochure list

	return broch;
}


// send new machine order in equations '_EI2', '_SI2'

CFUN_VOID( send_order, double nMach )
{
	// find firm entry on supplier client list
	object *cli = SHOOKS( HOOK( SUPPL ) );

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

CFUN_VOID( add_vintage, double nMach, bool newInd )
{
	double _Avint, _tVint;
	object *vint;

	object *suppl = PARENTS( SHOOKS( HOOK( SUPPL ) ) );// supplier
	double _pVint = VS( suppl, "_p1" );			// machine price
	int _g1 = VS( suppl, "_g1" );				// machine tech. generation
	object *gen = V_EXTS( GRANDPARENT, countryE, g1ptr[ _g1 ] );

	// create object, only recalculate in t if new industry
	if ( newInd )
		vint = ADDOBJL( "Vint", T - 1 );
	else
		vint = ADDOBJ( "Vint" );

	ADDHOOKS( vint, VINTHK );					// add worker hooks
	WRITE_SHOOKS( vint, HOOK( TOPVINT ) );// save previous vintage
	WRITE_HOOKS( vint, TGEN, gen );				// save technological generation
	WRITE_HOOK( TOPVINT, vint );			// save pointer to top vintage

	// distribute machine ages at t=1 to spread initial technical substitution
	if ( T == 1 )
		_tVint = T - ( uniform_int( 1, VS( PARENT, "eta" ) ) + 1 );
	else
		_tVint = T;

	WRITES( vint, "_IDvint", VNT( T, VS( suppl, "_ID1" ) ) );// vintage ID
	WRITES( vint, "_nVint", nMach );			// number of machines in vintage
	WRITES( vint, "_pVint", _pVint );			// price of machines in vintage
	WRITES( vint, "_tVint", _tVint );			// vintage install time

	_Avint = VS( suppl, "_A1" );
	WRITELLS( vint, "_AeVint", _Avint, T - 1, 1 );// lagged value
	WRITES( vint, "_AeVint", _Avint );			// vintage effective product.
	WRITES( vint, "_Avint", _Avint );			// vintage notional productivity

	DELETE( SEARCHS( vint, "WrkV" ) );			// remove empty worker object
}


// scrap (remove) vintage from capital stock in equation 'K'
// return -1 if last vintage (not removed but shrank to 1 machine)

CFUN_DBL( scrap_vintage )
{
	double RS;
	object *wrk;

	if ( NEXT != NULL )							// don't remove last vintage
	{
		// move all workers out from this vintage
		CYCLE( wrk, "WrkV" )
			WRITE_HOOKS( SHOOKS( wrk ), VWRK, NULL );

		// remove as previous vintage from next vintage
		if ( SHOOKS( NEXT ) == THIS )
			WRITE_SHOOKS( NEXT, NULL );

		RS = abs( V( "_RSvint" ) );
		DELETE( THIS );							// delete vintage
	}
	else
	{
		RS = -1;								// signal last machine
		WRITE( "_nVint", 1 );					// keep just 1 machine
	}

	return RS;
}


/*================== LABOR MANAGEMENT SUPPORT C FUNCTIONS ====================*/

// add and configure new worker object(s) in equations 'initCountry' and 'Ls'

CFUN_DBL( entry_worker, int n, bool init )
{
	double _SavLux, _sV;
	int _ID, _age, _tLux, firstID, lag, wrkObj;
	object *wrk;

	double wU = V( "wU" );						// unemployed wage (benefit)
	int Tc = V( "Tc" );							// work-contract term
	int Tlux = V( "Tlux" );						// luxury goods acquisit. period
	int Tr = V( "Tr" );							// work-life duration
	int LBU = VS( PARENT, "flagWorkerLBU" );	// learning mode

	wrkObj = ceil( n / V( "Lscale" ) );			// number of object instances
	_sV = ( LBU == 1 || LBU == 3 ) ? V( "sigma" ) : 1;// initial skills

	// set initial ID for adding new worker(s)
	if ( init )
	{
		_ID = 1;								// start ID from 1 if initializ.
		_SavLux = V( "Sav0lux" ) * INIWAGE;		// initial luxury savings
	}
	else
	{
		_ID = MAX( "_ID" ) + 1;					// continue from last id
		_SavLux = 0;							// no initial luxury savings
	}

	// add and configure new worker(s) objects
	for ( firstID = _ID; _ID < firstID + wrkObj; ++_ID )
	{
		if ( T == 1 && _ID == 1 )
			wrk = SEARCH( "Worker" );			// first? object instance exists
		else
			wrk = ADDOBJL( "Worker", T - 1 );	// recalculate in t

		ADDEXTS( wrk, wrkE );					// allocate extended data
		ADDHOOKS( wrk, WORKERHK );				// add worker hooks

		_age = uniform_int( 1, Tr );			// draw worker age in [1, Tr]
		_tLux = uniform_int( T - Tlux, T - 1 );	// draw last luxury acquisition

		WRITES( wrk, "_ID", _ID );
		WRITES( wrk, "_Tc", Tc );
		WRITES( wrk, "_age", _age );
		WRITES( wrk, "_employed", 0 );
		WRITES( wrk, "_sV", _sV );
		WRITES( wrk, "_tLux", _tLux );
		WRITES( wrk, "_wRes", wU );
		WRITELLS( wrk, "_SavLux", _SavLux, T - 1, 1 );
	}

	return 0;									// no entry costs till now
}


// update a worker after hiring in equations 'hire1', 'hire2'

CFUN_VOID( hire_worker, int sec, object *firm, double wage )
{
	int flagWorkerLBU;
	object *wrk;

	int _ID = V( "_ID" );
	WRITE( "_employed", sec );
	WRITE( "_Te", 0 );
	WRITE( "_CQ", 0 );							// no cumulated production yet
	WRITE( "_w", wage );

	flagWorkerLBU = VS( GRANDPARENT, "flagWorkerLBU" );
	if ( flagWorkerLBU != 0 && flagWorkerLBU != 2 )
		WRITE( "_sV", VS( PARENT, "sigma" ) );	// public skills
	else
		WRITE( "_sV", INISKILL );

	// then handle the case at hand, setting vintage and bridge objects
	if ( sec < 2 )								// unemployed or sector 1?
	{
		wrk = NULL;								// no Firm2 in use

		if ( sec == 1 )
		{
			// add bridge object between 'Wrk1' (in Capital) to 'Worker'
			wrk = ADDOBJS( firm, "Wrk1" );
			WRITE_SHOOKS( wrk, THIS );			// pointer to worker from firm
			WRITES( wrk, "_IDw1", _ID );
		}
	}
	else
	{
		// add bridge-object between 'Wrk2' (in Firm2) to 'Worker'
		wrk = ADDOBJS( firm, "Wrk2" );
		WRITE_SHOOKS( wrk, THIS );				// pointer to worker from firm
		WRITES( wrk, "_IDw2", _ID );			// register worker ID
	}

	WRITE_HOOK( FWRK, wrk );					// pointer to firm from worker
}


// update a worker after firing in equations 'fires1', '_fires2', 'entry2exit',
// 'quits1', 'retires1', '_quits2', '_retires2'

CFUN_VOID( fire_worker )
{
	WRITE( "_employed", 0 );					// register fire
	WRITE( "_Te", 0 );
	WRITE( "_w", 0 );							// reset wage
	RECALC( "_In" );							// recalc. income if already done

	// if already has a bridge object, destroy it first
	if ( HOOK( FWRK ) != NULL )
	{
		DELETE( HOOK( FWRK ) );
		WRITE_HOOK( FWRK, NULL );

		// and also destroy vintage bridge object
		if ( HOOK( VWRK ) != NULL )
		{
			DELETE( HOOK( VWRK ) );
			WRITE_HOOK( VWRK, NULL );
		}
	}
}


// perform worker quitting from current firm in equations 'hires1', 'hires'

CFUN_VOID( quit_worker )
{
	double Lscale = VS( PARENT, "Lscale" );		// labor scaling

	if ( V( "_employed" ) == 1 )				// sector 1?
		INCRS( PARENTS( HOOK( FWRK ) ), "quits1", Lscale );
	else										// no: assume sector 2
		INCRS( PARENTS( HOOK( FWRK ) ), "_quits2", Lscale );

	CFUN( fire_worker );						// register fire
}


// move worker to a different vintage in equation 'alloc2'

CFUN_VOID( move_worker, object *vint, bool vint_learn )
{
	double _sV;
	int _IDv;
	object *wrkV;

	if ( vint_learn )							// learning-by-vintage mode?
	{											// worker has public skills
		_IDv = VS( vint, "_IDvint" );
		_sV = V_EXTS( GRANDPARENTS( vint ), countryE, vintProd[ _IDv ].sVp );
	}
	else
		_sV = INISKILL;

	wrkV = ADDOBJS( vint, "WrkV" );				// add worker-bridge object
	WRITE_SHOOKS( wrkV, THIS );					// save pointer to work object
	WRITE_HOOK( VWRK, wrkV );					// register vint. in worker

	WRITES( wrkV, "_IDwV", V( "_ID" ) );
	WRITE( "_sV", _sV );							// set vintage skills
	WRITE( "_CQ", 0 );							// no cumulated production yet
}


// order wage offers in equation 'hires'

bool wo_asc_wrk( wageOffer e1, wageOffer e2 ) { return e1.workers < e2.workers; };
bool wo_desc_off( wageOffer e1, wageOffer e2 ) { return e1.offer > e2.offer; };

CFUN_VOID( shuffle_offers, woLisT *offers )
{
	// make a copy of the workers list into a vector
	woVecT temp( offers->size( ) );
	copy( offers->begin( ), offers->end( ), temp.begin( ) );

	// shuffle firms to choose hiring order
	shuffle( temp.begin( ), temp.end( ), random_engine );

	// and copy it back to a list
	copy( temp.begin( ), temp.end( ), offers->begin( ) );
}

CFUN_VOID( order_offers, int order, woLisT *offers )
{
	int i;
	woLisT noWorker;
	woLisT::iterator it;

	// always shuffle orders to prevent industry preference when same wages
	CFUN( shuffle_offers, offers );

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
			CFUN( shuffle_offers, & noWorker );	// shuffle noWorker firm(s)
			CFUN( shuffle_offers, offers );		// shuffle the rest of the list

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


// sort job applications in equations 'hires1', 'hires'

bool appl_asc_w( application e1, application e2 ) { return e1.w < e2.w; };
bool appl_desc_w( application e1, application e2 ) { return e1.w > e2.w; };
bool appl_asc_s( application e1, application e2 ) { return e1.s < e2.s; };
bool appl_desc_s( application e1, application e2 ) { return e1.s > e2.s; };
bool appl_asc_ws( application e1, application e2 ) { return e1.ws < e2.ws; };
bool appl_desc_ws( application e1, application e2 ) { return e1.ws > e2.ws; };
bool appl_asc_Te( application e1, application e2 ) { return e1.Te < e2.Te; };
bool appl_desc_Te( application e1, application e2 ) { return e1.Te > e2.Te; };

CFUN_VOID( order_applications, int order, appLisT *appl )
{
	if ( appl->size( ) == 0 )					// prevent empty lists
		return;

	appVecT temp( appl->size( ) );

	switch ( order )
	{
		default:
		case 0:									// random order
			// make a copy of list to a vector, shuffle, and copy back
			copy( appl->begin( ), appl->end( ), temp.begin( ) );
			shuffle( temp.begin( ), temp.end( ), random_engine );
			copy( temp.begin( ), temp.end( ), appl->begin( ) );
			break;
		case 1:									// higher wage first order
			appl->sort( appl_desc_w );
			break;
		case 2:									// lower wage first order
			appl->sort( appl_asc_w );
			break;
		case 3:									// higher skills first order
			appl->sort( appl_desc_s );
			break;
		case 4:									// lower skills first order
			appl->sort( appl_asc_s );
			break;
		case 5:									// higher payback first order
			appl->sort( appl_desc_ws );
			break;
		case 6:									// lower payback first order
			appl->sort( appl_asc_ws );
			break;
		case 7:									// old hires first order
			appl->sort( appl_desc_Te );
			break;
		case 8:									// recent hires first order
			appl->sort( appl_asc_Te );
			break;
	}
}


// sort worker bridge objects in equation 'fires1', 'fires2'

#define OBJ_WRK1 0								// operate on sector 1 workers
#define OBJ_WRK2 1								// operate on sector 2 workers

const char *wrkName[ ] = { "Wrk1", "Wrk2" },
		   *keyName[ ] = { "_key1", "_key2" };

CFUN_VOID( order_workers, int order, int obj )
{
	char keyN[ 4 ], dir[ 5 ];
	double keyV;
	object *wrk;

	switch ( order )							// handle selected sort scheme
	{
		default:
		case 0:									// random order
		case 6:									// lower payback first order
			strcpy( dir, "UP" );
			break;
		case 5:									// higher payback first order
			strcpy( dir, "DOWN" );
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
		case 7:									// old hires first order
			strcpy( keyN, "_Te" );
			strcpy( dir, "DOWN" );
			break;
		case 8:									// recent hires first order
			strcpy( keyN, "_Te" );
			strcpy( dir, "UP" );
	}

	CYCLE( wrk, wrkName[ obj ] )				// update all employees
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

	SORT( wrkName[ obj ], keyName[ obj ], dir );// sort the bridge objects
}


// realize firing for firm in equation 'fires2'

#define MODE_ALL 1								// fire all workers
#define MODE_ADJ 2								// fire only for adjustment
#define MODE_PBACK 3							// fire negative paybacks
#define MODE_IPROT 4							// fire non protected workers
#define MODE_EXIT 5								// fire all when firm exiting

CFUN_DBL( fire_workers, int mode, double xsCap, double *redCap )
{
	bool fire;
	int _Te;
	object *cyccur, *wrk, *worker;

	object *cnt = GRANDPARENT;					// pointers to objects
	object *lab = V_EXTS( cnt, countryE, labSup );

	int i = 0;									// fired workers counter
	int Tp = VS( lab, "Tp" );					// time for protected workers
	double w2avg = VLS( PARENT, "w2avg", 1 );	// average wage
	double Lscale = VS( lab, "Lscale" );		// labor scale

	*redCap = 0;								// reduced capacity accumulator
	xsCap *= 1 - VS( lab, "theta" );			// create slack (extra workers)

	// order workers to fire according firm preference
	int fOrder = V( "_post2chg" ) ? VS( cnt, "flagFireOrder2Chg" ) :
									VS( cnt, "flagFireOrder2" );
	if ( mode == MODE_PBACK )					// explicit payback firing?
		fOrder = 0;								// ignore order set

	// create sorted list of workers according to the chosen attributes
	CFUN( order_workers, fOrder, OBJ_WRK2 );	// sort bridge objects

	// check firing worker by worker: firm desired adjustments
	CYCLE( wrk, "Wrk2" )
	{
		fire = false;
		worker = SHOOKS( wrk );
		_Te = VLS( worker, "_Te", 1 ) + 1;

		// contract not finished and firm not exiting market?
		if ( _Te < VS( worker, "_Tc" ) && mode != MODE_EXIT )
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
					if ( _Te <= Tp )			// is worker yet unprotected
						fire = true;

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
			CFUNS( worker, fire_worker );		// register fire
			*redCap += VLS( worker, "_Q", 1 ) * Lscale;// pot. fired capacity
			++i;								// scaled equivalent fires
		}
	}

	return i * Lscale;
}


/*=================== FIRM ENTRY-EXIT SUPPORT C FUNCTIONS ====================*/

// assign a random bank to entrant firm, preserving bank size distribution,
// in equations 'entry1exit',  'entry2exit' and 'initCountry'

const char *bankPar[ ] = { "_bank1", "_bank2" },
		   *CliObj[ ] = { "Cli1", "Cli2" },
		   *_IDpar[ ] = { "_ID1","_ID2" },
		   *__IDpar[ ] = { "_IDc1", "_IDc2" };

CFUN_OBJ( set_bank )
{
	int _IDb, sec = strcmp( NAME, "Firm1" ) == 0 ? 0 : 1;
	object *bank, *cli, *fin = SEARCHS( GRANDPARENT, "Financial" );

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


// add and configure entrant capital-good firm object(s) and required hooks
// in equations 'entry1exit',  'entry2exit' and 'initCountry'

CFUN_DBL( entry_firm1, int n, bool newInd )
{
	double A1best, B1best, _A1, _B1, _D10u, _NW1, _NW10, _RD10, _c1, _f1, _g1,
		   _p1, _sV;
	double equity, w1avg, mult;
	int _ID1, tComp;
	object *cur, *firm,
		   *cons = SEARCHS( PARENT, "Consumption" ),
		   *lab = SEARCHS( PARENT, "Labor" );

	double Deb10ratio = V( "Deb10ratio" );		// bank fin. to equity ratio
	double NW10 = V( "NW10" );					// initial net worth ratio
	double Phi3 = V( "Phi3" );					// lower support for wealth share
	double Phi4 = V( "Phi4" );					// upper support for wealth share
	double alpha2 = V( "alpha2" );				// lower support for imitation
	double beta2 = V( "beta2" );				// upper support for imitation
	double eta = VS( cons, "eta" ); 			// machine technical life
	double mu1 = V( "mu1" );					// mark-up in sector 1
	double m1 = V( "m1" );						// worker production scale
	double nu = V( "nu" );						// share of R&D expenses
	double x5 = V( "x5" );						// entrant upper advantage

	if ( newInd && T == 1 )
	{
		double m2 = VS( cons, "m2" );			// machine output per period
		double c10 = INIWAGE / ( m1 * INIPROD );// machine initial cost
		double c20 = INICPLX * INIWAGE / ( m2 * INIPROD );// good initial cost
		double p10 = ( 1 + mu1 ) * c10;			// machine initial price
		double p20 = ( 1 + VS( PARENT, "mu0bas" ) ) * c20;// price s. 2
		double D20 = INIWAGE * VS( lab, "Ls0" );// initial demand (full employm.)

		// initial full employment demand (machines) expectation, all sector 2
		double K20 = ( 1 + VS( cons, "iota" ) ) * ( D20 / p20 ) /
					 ( m2 / INICPLX ) / VS( cons, "u" );

		// 1/eta replacement factor over all sector 2 and fair share in sector 1
		_D10u = K20 / eta / n;

		 // expected first period costs (production and R&D)
		_NW10 = _D10u * c10 + max( nu * _D10u * p10, INIWAGE );

		_A1 = _B1 = A1best = B1best = INIPROD;	// initial productivities
		_f1 = 1.0 / n;							// fair share
		_g1 = 0;								// initial machine generation
		_sV = VS( lab, "sAvg" );				// initial worker skills
		tComp = 0;								// time of last computation
		w1avg = INIWAGE;						// initial wage
	}
	else
	{
		// initial demand is the expected machine replacement under fair share
		_D10u = SUMS( PARENT, "K2" ) / eta / ( V( "F1" ) + n );

		_NW10 = WHTAVE( "_NW1", "_f1" );		// initial net worth

		// find best firm productivities (mix) and machine generation
		A1best = B1best = 0;
		CYCLE( cur, "Firm1" )
		{
			_A1 = VS( cur, "_A1" );
			_B1 = VS( cur, "_B1" );

			if ( _A1 * _B1 > A1best * B1best )
			{
				A1best = _A1;					// best productivity so far
				B1best = _B1;
			}
		}

		_f1 = 0;								// no market share
		_g1 = 0;								// machine generation (0=none)
		_sV = INISKILL;							// worker skills
		tComp = T;								// time of last computation
		w1avg = V( "w1avg" );					// average wage in sector 1
	}

	// add entrant firms (end of period, don't try to sell)
	for ( equity = 0; n > 0; --n )
	{
		// create object, only recalculate in t if new industry
		if ( newInd && T == 1 )
			firm = ADDOBJL( "Firm1", T - 1 );
		else
			firm = ADDOBJ( "Firm1" );

		_ID1 = INCR( "last_ID1", 1 );			// new firm ID
		WRITES( firm, "_ID1", _ID1 );

		ADDHOOKS( firm, FIRM1HK );				// add object hooks
		DELETE( SEARCHS( firm, "Cli" ) );		// remove empty instances

		// select associated bank
		CFUNS( firm, set_bank );

		if ( ! newInd || T > 1 )
		{
			// initial technological (imitation from best firm)
			_A1 = beta( alpha2, beta2 );		// draw A from Beta(alpha,beta)
			_A1 *= A1best * ( 1 + x5 );			// fraction of top firm
			_B1 = beta( alpha2, beta2 );		// draw B from Beta(alpha,beta)
			_B1 *= B1best * ( 1 + x5 );			// fraction of top firm
		}

		// initial cost, price and net wealth to pay at least for initial R&D
		_c1 = w1avg / ( m1 * _B1 );				// unit cost
		_p1 = ( 1 + mu1 ) * _c1;				// unit price
		_RD10 = max( nu * _D10u * _p1, w1avg );	// R&D expense

		// initial net worth based on existing firms with a floor
		mult = ( newInd && T == 1 ) ? 1 : uniform( Phi3, Phi4 );// NW multiple
		_NW1 = _D10u * _c1 + _RD10;				// NW floor
		_NW1 *= NW10;							// NW floor multiple
		_NW1 = max( _NW1, mult * _NW10 );

		equity += _NW1 * ( 1 - Deb10ratio );	// account public entry equity

		// initialize variables
		WRITES( firm, "_t1ent", tComp );
		WRITELLS( firm, "_A1", _A1, tComp, 1 );
		WRITELLS( firm, "_B1", _B1, tComp, 1 );
		WRITELLS( firm, "_Deb1", _NW1 * Deb10ratio, tComp, 1 );
		WRITELLS( firm, "_NW1", _NW1, tComp, 1 );
		WRITELLS( firm, "_RD1", _RD10, tComp, 1 );
		WRITELLS( firm, "_f1", _f1, tComp, 1 );
		WRITELLS( firm, "_g1", _g1, tComp, 1 );
		WRITELLS( firm, "_qc1", 1, tComp, 1 );

		if ( newInd && T == 1 )
		{
			// initialize the map of vintage productivity and skills
			WRITE_EXTS( PARENT, countryE, vintProd[ VNT( T - 1, _ID1 ) ].sVp, _sV );
			WRITE_EXTS( PARENT, countryE, vintProd[ VNT( T - 1, _ID1 ) ].sVavg, _sV );
		}
		else
		{
			WRITES( firm, "_A1", _A1 );
			WRITES( firm, "_B1", _B1 );
			WRITES( firm, "_Deb1", _NW1 * Deb10ratio );
			WRITES( firm, "_NW1", _NW1 );
			WRITES( firm, "_RD1", _RD10 );
			WRITES( firm, "_c1", _c1 );
			WRITES( firm, "_g1", _g1 );
			WRITES( firm, "_p1", _p1 );

			// compute variables requiring calculation still in t
			RECALCS( firm, "_Deb1max" );		// prudential credit limit
			RECALCS( firm, "_NC1" );			// set initial clients
			VS( firm, "_cred1" );				// update available credit
		}
	}

	return equity;								// equity cost of entry(ies)
}


// add and configure entrant consumer-good firm object(s) and required hooks
// in equations 'entry1exit',  'entry2exit' and 'initCountry'

CFUN_DBL( entry_firm2, int n, bool newInd )
{
	bool _post2chg;
	double _A2, _D20, _D2e, _E2, _K2, _K2nom, _K2d, _N2, _NW2, _NW20, _Q2u, _c2,
		   _f2, _life2cycle, _p2, _q2, _s2avg, _w2avg, _w2o;
	double F2avg, K20, K2avg, cash, equity, f2sum, k2avg, mult, w2avg,
		   w2oAvg;
	int _ID2, tComp;
	object *firm, *cli, *suppl, *broch, *cur,
		   *cap = SEARCHS( PARENT, "Capital" ),
		   *fin = SEARCHS( PARENT, "Financial" ),
		   *lab = SEARCHS( PARENT, "Labor" );

	bool AllFirmsChg = VS( PARENT, "flagAllFirmsChg" );// change at once?
	bool f2critChg = V( "f2critChg" );			// critical change thresh. met?
	double Deb20ratio = V( "Deb20ratio" );		// bank fin. to equity ratio
	double NW20 = V( "NW20" );					// initial net worth ratio
	double Phi1 = V( "Phi1" );					// lower support for K share
	double Phi2 = V( "Phi2" );					// upper support for K share
	double ent2HldShr = V( "ent2HldShr" );		// hold share post-chg firms
	double f2minPosChg = V( "f2minPosChg" );	// min m.s. post-chg firms
	double f2posChg = V( "f2posChg" );			// current m.s. post-chg firms
	double iota = V( "iota" );					// desired inventories factor
	double k2 = V( "k2" );						// industry complexity degree
	double mu20 = V( "mu20" );					// initial mark-up in sector 2
	double m2 = V( "m2" );						// machine output per period
	double u = V( "u" );						// desired capital utilization
	int ID2 = V( "ID2" );						// industry ID
	int TregChg = VS( PARENT, "TregChg" );		// time for regime change
	int g1front = VS( cap, "g1front" );			// tech. frontier generation
	int type2 = V( "type2" );					// type of industry firm is in

	if ( newInd )
	{
		_E2 = VL( "E2avg", 1 );					// initial competitiveness
		_NW20 = 0; 								// compute later
		_Q2u = u;								// initial capacity utilization
		_f2 = 1.0 / n;							// fair share
		_q2 = VL( "q2", 1 );					// initial quality
		_s2avg = VL( "s2avg", 1 );				// initial skills

		if ( T == 1 )							// handle initial configuration
		{
			double Ls0 = VS( lab, "Ls0" );		// initial number of workers
			double c20 = k2 * INIWAGE / ( m2 * INIPROD );// good initial cost
			double p20 = ( 1 + mu20 ) * c20;	// consumer good initial price

			if ( type2 == 0 )
				// fair shares of initial full employment demand (monetary terms)
				_D20 = INIWAGE * Ls0 * ( 1 + VS( lab, "Sav0bas" ) ) /
					   ( n * VS( PARENT, "Fc0bas" ) );
			else
				_D20 = INIWAGE * Ls0 * VS( lab, "Sav0lux" ) / VS( lab, "Tlux" ) /
					   ( n * VS( PARENT, "Fc0lux" ) );

			_K2 = ( 1 + iota ) * ( _D20 / p20 ) / ( m2 / k2 ) / u;
			_N2 = iota * _D20 / p20;			// initial inventories
			_life2cycle = 1;					// start as operating entrant
			suppl = NULL;						// no supplier preselected
			tComp = 0;							// time of last computation
			w2avg = w2oAvg = INIWAGE;			// initial average wage
		}
		else									// regular industry emergence
		{
			k2avg = type2 == 0 ? VLS( PARENT, "kCavgBas", 1 ) :
								 VLS( PARENT, "kCavgLux", 1 );
			if ( is_nan( k2avg ) )
				k2avg = VLS( PARENT, "kCavg", 1 );

			f2sum = SUM_CNDLS( PARENT, "f2", "type2", "==", type2, 1 );
			if ( f2sum > 0 )					// can use same subsector only?
			{
				K2avg = WHTAVE_CNDLS( PARENT, "K2", "f2",
									  "type2", "==", type2, 1 ) / f2sum;
				F2avg = WHTAVE_CNDLS( PARENT, "F2", "f2",
									  "type2", "==", type2, 1 ) / f2sum;
			}
			else								// no industry of same type
			{
				f2sum = SUMLS( PARENT, "f2", 1 );
				K2avg = WHTAVELS( PARENT, "K2", "f2", 1 ) / f2sum;
				F2avg = WHTAVELS( PARENT, "F2", "f2", 1 ) / f2sum;
			}

			_D20 = 0;							// compute later
			_K2 = ( k2 / k2avg ) * ( K2avg / F2avg );// initial capital in ind.
			_N2 = 0;							// inventories
			_life2cycle = 0;					// start pre-operational entrant
			tComp = T;							// time of last computation
			w2avg = VL( "w2avg", 1 );			// initial wages
			w2oAvg = VL( "w2oAvg", 1 );			// initial offered wages

			// find top supplier in latest available machine generation
			double A1best = 0, g1dist = DBL_MAX;
			CYCLES( cap, cur, "Firm1" )
			{
				double _A1 = VS( cur, "_A1" );
				double _g1 = VS( cur, "_g1" );

				// pick the best supplier among the ones with the best tech.
				if ( g1front - _g1 < g1dist || ( g1front - _g1 == g1dist && _A1 > A1best ) )
				{
					g1dist = g1front - _g1;
					A1best = _A1;
					suppl = cur;
				}
			}
		}
	}
	else										// existing industry
	{
		_D20 = 0;								// compute later
		_E2 = V( "E2avg" );
		_K2 = WHTAVE( "_K2", "_f2" );
		_N2 = 0;								// inventories
		_NW20 = WHTAVE( "_NW2", "_f2" );		// average wealth in industry
		_Q2u = V( "Q2u" );
		_f2 = 0;								// no market share
		_life2cycle = 0;						// start as pre-operat. entrant
		_q2 = V( "q2" );
		_s2avg = VS( lab, "sAvg" );				// average skills
		suppl = NULL;
		tComp = T;
		w2avg = V( "w2avg" );					// average industry wages
		w2oAvg = V( "w2oAvg" );
	}

	// add entrant firms (end of period, don't try to produce except on startup)
	for ( equity = 0; n > 0; --n )
	{
		// create object, only recalculate in t if new industry
		if ( newInd && T == 1 )
			firm = ADDOBJL( "Firm2", T - 1 );
		else
			firm = ADDOBJ( "Firm2" );

		_ID2 = ID( ID2, INCR( "last_ID2", 1 ) );// new firm ID
		WRITES( firm, "_ID2", _ID2 );
		ADDHOOKS( firm, FIRM2HK );				// add object hooks
		ADDEXTS( firm, firm2E );				// allocate extended data
		DELETE( SEARCHS( firm, "Vint" ) );		// remove empty instances
		DELETE( SEARCHS( firm, "Wrk2" ) );

		// select associated bank
		CFUNS( firm, set_bank );

		// select current machine supplier and create hooks to/from it
		if ( suppl == NULL )
			suppl = RNDDRAW_FAIRS( cap, "Firm1" );// pick random supplier if none

		INCRS( suppl, "_NC1", 1 );
		cli = ADDOBJS( suppl, "Cli" );			// add to supplier client list
		WRITES( cli, "_IDc", _ID2 );			// update object
		WRITES( cli, "_tSel", T );
		broch = SEARCHS( firm, "Broch" );		// add to firm brochure list
		WRITES( broch, "_IDs", VS( suppl, "_ID1" ) );// update object
		WRITE_SHOOKS( broch, cli );				// pointer to supplier cli. list
		WRITE_SHOOKS( cli, broch );				// pointer to client broch. list
		WRITE_HOOKS( firm, SUPPL, broch );		// pointer to current supplier

		// choose firm type (pre/post-change)
		if ( TregChg <= 0 || T < TregChg )		// before regime change?
			_post2chg = false;					// it's pre-change type
		else
			if ( AllFirmsChg || newInd )		// all change at once or new ind.?
				_post2chg = true;				// it's post-change type
			else
				if ( ! f2critChg )				// critical threshold not met?
					// fixed type proportion
					_post2chg = ( RND < ent2HldShr ) ? true : false;
				else
					// draw type according shares
					_post2chg = ( RND < max( f2posChg, f2minPosChg ) ) ? true : false;

		// initial desired capital/expected demand, rounded to # of machines
		mult = ( newInd && T == 1 ) ? 1 : uniform( Phi1, Phi2 );// capital multiple
		_K2d = max( mult * _K2, 1 );
		_K2nom = _K2d * VS( suppl, "_p1" );

		// initial wage, productivity, cost and price
		mult = ( newInd && T == 1 ) ? 1 + uniform( - mu20 / 2, mu20 / 2 ) : 1;
		_w2avg = mult * w2avg;					// initial wage dispersion
		_w2o = mult * w2oAvg;
		_A2 = VS( suppl, "_A1" );				// initial notional productivity
		_c2 =  k2 * _w2avg / ( m2 * _A2 );		// initial unit costs
		_p2 = ( 1 + mu20 ) * _c2;				// initial price

		// handle special case of initial demand
		_D2e = ( newInd && T == 1 ) ? _D20 : u * _K2d * ( m2 / k2 ) * _p2;

		// initial net worth based on existing firms with a floor
		mult = uniform( Phi1, Phi2 );			// NW multiple
		_NW2 = ( 1 + iota ) * ( _D2e / _p2 ) * _c2;// NW floor
		_NW2 *= NW20;							// NW floor multiple
		_NW2 = max( _NW2, mult * _NW20 );

		// initial equity must pay initial capital and wages
		cash = _K2nom + _NW2;
		equity += cash * ( 1 - Deb20ratio );	// accumulated equity

		// initialize variables
		WRITES( firm, "_t2ent", tComp );
		WRITES( firm, "_life2cycle", _life2cycle );
		WRITES( firm, "_post2chg", _post2chg );
		WRITELLS( firm, "_A2", _A2, tComp, 1 );
		WRITELLS( firm, "_Deb2", cash * Deb20ratio, tComp, 1 );
		WRITELLS( firm, "_E2", _E2, tComp, 1 );
		WRITELLS( firm, "_f2", _f2, tComp, 1 );
		WRITELLS( firm, "_f2", _f2, tComp, 2 );
		WRITELLS( firm, "_mu2", mu20, tComp, 1 );
		WRITELLS( firm, "_p2", _p2, tComp, 1 );
		WRITELLS( firm, "_qc2", 1, tComp, 1 );
		WRITELLS( firm, "_sT2min", 1, tComp, 1 );
		WRITELLS( firm, "_w2avg", _w2avg, tComp, 1 );
		WRITELLS( firm, "_w2o", _w2o, tComp, 1 );

		for ( int i = 1; i <= 4; ++i )
		{
			WRITELLS( firm, "_D2", _D2e, tComp, i );
			WRITELLS( firm, "_D2d", _D2e, tComp, i );
		}

		if ( newInd && T == 1 )
		{
			WRITELLS( firm, "_K2", _K2d, tComp, 1 );
			WRITELLS( firm, "_K2nom", _K2nom, tComp, 1 );
			WRITELLS( firm, "_N2", _N2, tComp, 1 );
			WRITELLS( firm, "_NW2", _NW2, tComp, 1 );

			// add first machine vintage
			CFUNS( firm, add_vintage, _K2d, newInd );
		}
		else
		{
			WRITES( firm, "_A2", _A2 );
			WRITES( firm, "_A2p", _A2 );
			WRITES( firm, "_D2e", _D2e );
			WRITES( firm, "_Deb2", cash * Deb20ratio );
			WRITES( firm, "_E2", _E2 );
			WRITES( firm, "_K2d", _K2d );
			WRITES( firm, "_NW2", cash );
			WRITES( firm, "_Q2u", _Q2u );
			WRITES( firm, "_c2", _c2 );
			WRITES( firm, "_c2e", _c2 );
			WRITES( firm, "_f2", _f2 );
			WRITES( firm, "_mu2", mu20 );
			WRITES( firm, "_old2vint", VNT( T, 0 ) );
			WRITES( firm, "_p2", _p2 );
			WRITES( firm, "_q2", _q2 );
			WRITES( firm, "_s2avg", _s2avg );
			WRITES( firm, "_w2avg", _w2avg );
			WRITES( firm, "_w2o", _w2o );
			WRITELLS( firm, "_NW2", cash, tComp, 1 );

			// compute variables requiring calculation still in t
			RECALCS( firm, "_Deb2max" );		// prudential credit limit
			VS( firm, "_cred2" );				// update available credit

			// statistics only
			RECALCS( firm, "_w2realAvg" );
		}
	}

	return equity;								// equity cost of entry(ies)
}


// remove capital-good firm objects and exiting hooks in equation 'entry1exit'

CFUN_DBL( exit_firm1, double *cEntry, double *cExit, double *nFail, bool all )
{
	double equity, liqVal, shareAcc, shareBest, _NW1;
	int i, j, best, exits;
	object *bank, *firm1, *firm2, *cyccur;

	double n1 = V( "n1" );						// market participation period
	int F1 = COUNT( "Firm1" );					// current number of firms

	boolVecT quit( F1 );					// vector of firms' quit status

	// mark bankrupt and market-share-irrelevant firms to exit
	shareBest = best = i = 0;
	CYCLE( firm1, "Firm1" )
	{
		_NW1 = VS( firm1, "_NW1" );				// current net wealth

		// all leaving, bankrupt or incumbent?
		if ( all || _NW1 < 0 || T >= VS( firm1, "_t1ent" ) + n1 )
		{
			for ( shareAcc = j = 0; j < n1; ++j )
				shareAcc += VLS( firm1, "_BC1", j );// n1 periods customer number

			if ( all || _NW1 < 0 || shareAcc <= 0 )
			{
				quit[ i ] = true;				// mark for likely exit
				--F1;							// one less firm

				if ( shareAcc > shareBest )		// best firm so far?
				{
					best = i;					// save firm index
					shareBest = shareAcc;		// and customer number
				}
			}
		}

		++i;
	}

	// quit candidate firms exit, except the best one if all going to exit
	*nFail = exits = i = 0;
	CYCLE( firm1, "Firm1" )
	{
		if ( quit[ i ] )
		{
			if ( all || F1 > 0 || i != best )	// firm must exit?
			{
				// account liquidation value (bank bad debt or public equity)
				_NW1 = VS( firm1, "_NW1" );		// exit net wealth
				liqVal = _NW1 - VS( firm1, "_Deb1" );// liquidation value
				if ( liqVal < 0 )				// account bank losses, if any
				{
					bank = HOOKS( firm1, BANK );// exiting firm bank
					VS( bank, "_BadDeb" );		// ensure reset in t
					INCRS( bank, "_BadDeb", - liqVal );// accumulate bank losses
				}
				else
					*cExit += liqVal;			// liquidation credit, if any

				DELETE( HOOKS( firm1, BCLIENT ) );// leave client list of bank

				CYCLES( firm1, firm2, "Cli" )	// leave supplier lists of s. 2
					DELETE( SHOOKS( firm2 ) );	// delete from firm broch. list

				DELETE( firm1 );

				++exits;						// count exits
				if ( _NW1 < 0 )					// count bankruptcies
					++( *nFail );
			}
			else
				if ( F1 == 0 && i == best )		// best firm must get new equity
				{
					// compute new net worth after bail-out (all market)
					double eta = VS( SEARCHS( PARENT, "Consumption" ),
									 "eta" );
					double w1avg = V( "w1avg" );
					double _D1u = SUMS( PARENT, "K2" ) / eta;
					double _RD1 = max( V( "nu" ) * _D1u * V( "p1" ), w1avg );
					double _NW1 = _D1u * w1avg / ( V( "m1" ) * V( "B1" ) ) + _RD1;

					// new equity required
					equity = _NW1 + VS( firm1, "_Deb1" ) - VS( firm1, "_NW1" );
					*cEntry += equity;			// accumulate "entry" equity cost

					WRITES( firm1, "_Deb1", 0 );// reset debt
					INCRS( firm1, "_NW1", equity );// add new equity
				}
		}

		++i;
	}

	return exits;
}


// remove consumer-good firm objects and exiting hooks in equations 'entry2exit'
// and 'entryExit'

CFUN_DBL( exit_firm2, double *cEntry, double *cExit, double *nFail, bool all )
{
	double equity, fires, firesAcc, liqVal, shareAcc, shareBest, _NW2;
	int i, j, best, exits;
	object *bank, *firm1, *firm2, *cyccur;

	double f2min = V( "f2min" );				// min market share in industry
	double n2 = V( "n2" );						// market participation period
	int F2 = COUNT( "Firm2" );					// current number of firms

	boolVecT quit( F2 );						// vector of firms' quit status

	// mark bankrupt and market-share-irrelevant incumbent firms to exit
	shareBest = best = i = 0;
	CYCLE( firm2, "Firm2" )
	{
		_NW2 = VS( firm2, "_NW2" );				// current net wealth

		// all leaving, bankrupt or incumbent?
		if ( all || _NW2 < 0 || VS( firm2, "_life2cycle" ) > 1 )
		{
			for ( shareAcc = j = 0; j < n2; ++j )
				shareAcc += VLS( firm2, "_f2", j ) / n2;// n2 periods avg. share

			if ( all || _NW2 < 0 || shareAcc < f2min )
			{
				quit[ i ] = true;				// mark for likely exit
				--F2;							// one less firm

				if ( shareAcc > shareBest )		// best firm so far?
				{
					best = i;					// save firm index
					shareBest = shareAcc;		// and market share
				}
			}
			else
				quit[ i ] = false;
		}

		++i;
	}

	// quit candidate firms exit, except the best one if all going to exit
	*nFail = exits = firesAcc = i = 0;
	CYCLE( firm2, "Firm2" )
	{
		if ( quit[ i ] )
		{
			if ( all || F2 > 0 || i != best )	// firm must exit?
			{
				WRITES( firm2, "_life2cycle", 4 );// mark as exiting firm

				// fire all workers
				firesAcc += fires = CFUNS( firm2, fire_workers, MODE_EXIT, 0,
										   &fires );
				INCRS( firm2, "_fires2", fires );

				// account liquidation value (bank bad debt or public equity)
				_NW2 = VS( firm2, "_NW2" );		// exit net wealth
				liqVal = _NW2 - VS( firm2, "_Deb2" );// liquidation value
				if ( liqVal < 0 )				// account bank losses, if any
				{
					bank = HOOKS( firm2, BANK );// exiting firm bank
					VS( bank, "_BadDeb" );		// ensure reset in t
					INCRS( bank, "_BadDeb", - liqVal );// accumulate bank losses
				}
				else
					*cExit += liqVal;			// liquidation credit, if any

				DELETE( HOOKS( firm2, BCLIENT ) );// leave client list of bank

				CYCLES( firm2, firm1, "Broch" )	// leave client lists of s. 1
					DELETE( SHOOKS( firm1 ) );	// delete from firm client list

				// update firm map before removing LSD object
				EXEC_EXTS( PARENT, countryE, firm2map, erase,
						   ( int ) VS( firm2, "_ID2" ) );

				DELETE_EXTS( firm2, firm2E );
				DELETE( firm2 );

				++exits;						// count exits
				if ( _NW2 < 0 )					// count bankruptcies
					++( *nFail );
			}
			else
				if ( F2 == 0 && i == best )		// best firm must get new equity
				{
					// compute new net worth after bail-out (all market)
					double _NW2 = ( 1 + V( "iota" ) ) *
								  ( V( "D2d" ) / V( "p2" ) ) * V( "c2avg" );

					// new equity required
					equity = _NW2 + VS( firm2, "_Deb2" ) - VS( firm2, "_NW2" );
					*cEntry += equity;			// accumulate "entry" equity cost

					WRITES( firm2, "_Deb2", 0 );// reset debt
					INCRS( firm2, "_NW2", equity );// add new equity
				}
		}

		++i;
	}

	INCR( "fires2", firesAcc );					// update industry fires

	return exits;
}


/*================== INDUSTRY ENTRY-EXIT SUPPORT C FUNCTIONS =================*/

// add and configure new consumer-good industries and associated entrant firm
// objects in equations 'entryExit' and 'initCountry'

CFUN_DBL( entry_consumption, int n, bool basic, double f20, double *F2acc )
{
	double A2avg, E2avg, E2avgAvg, c2, equity, f2, f2sum, gamma, k2, k2avg, p2,
		   piRnd, q2avg, sAvg, wAvg, w2oMax, x3inf, x3sup;
	int i, F20, ID2, g2base;
	object *cur, *ind, *firm, *cap = SEARCH( "Capital" ),
							  *lab = SEARCH( "Labor" );

	double mu20 = basic ? V( "mu0bas" ) : V( "mu0lux" );// initial mark-up at t=1
	int TregChg = V( "TregChg" );				// time for regime change
	int type2 = basic ? 0 : 1;					// industry type

	for ( equity = 0, i = 0; i < n; ++i )
	{
		ID2 = INCR( "lastID2", 1 );				// new industry ID

		// add new industry object, if not the first industry
		if ( ID2 == 1 )							// first? object instance exists
			ind = SEARCH( "Consumption" );
		else
			if ( T == 1 )						// if startup, recompute in t=1
				ind = ADDOBJL( "Consumption", T - 1 );
			else
				ind = ADDOBJ( "Consumption" );

		ADDEXTS( ind, ind2E );					// allocate extended data

		// initial number of firms and complexity (in startup or later)
		if ( T == 1 )
		{
			A2avg = INIPROD;					// initial productivity
			F20 = VS( ind, "F20" );				// initial number of firms
			E2avg = ( V( "delta1" ) + V( "delta2" ) + V( "delta3" ) +
					  V( "delta4" ) ) / 2;		// initial industry compet.
			E2avgAvg = ( VS( ind, "omega1" ) +
						 VS( ind, "omega2" ) +
						 VS( ind, "omega3" ) ) / 2;// initial firm compet.
			g2base = 0;							// base tech. machine generation
			k2 = INICPLX;						// initial complexity
			q2avg = 1;							// initial quality
			sAvg = INISKILL;					// average worker skills
			wAvg = INIWAGE;						// initial wage
			w2oMax = INIWAGE;					// initial top offer wage
		}
		else
		{
			F20 = VS( ind, "F2min" );
			sAvg = VS( lab, "sAvg" );
			wAvg = VS( lab, "wAvg" );
			w2oMax = MAX( "w2oMax" );

			if ( basic )
			{
				A2avg = VL( "AcBas", 1 );
				g2base = 0;
				gamma = V( "gammaBas" );		// technology intensity
				k2avg = VL( "kCavgBas", 1 );	// average complexity
			}
			else
			{
				A2avg = VL( "AcLux", 1 );
				g2base = MAX( "g1front" );
				gamma = V( "gammaLux" );		// technology intensity
				k2avg = VL( "kCavgLux", 1 );	// average complexity
			}

			// compute weighted averages using only same type industries, if any
			E2avg = E2avgAvg = f2sum = q2avg = 0;
			CYCLE( cur, "Consumption" )
				if ( VS( cur, "type2" ) == type2 )
				{
					f2sum += f2 = VLS( cur, "f2", 1 );
					E2avg += VLS( cur, "E2", 1 ) * f2;
					E2avgAvg += VLS( cur, "E2avg", 1 ) * f2;
					q2avg += VLS( cur, "q2", 1 ) * f2;
				}

			if ( f2sum > 0 )
			{
				E2avg /= f2sum;
				E2avgAvg /= f2sum;
				q2avg /= f2sum;
			}
			else								// if no industry of same type
			{
				E2avg = WHTAVEL( "E2", "f2", 1 );
				E2avgAvg = WHTAVEL( "E2avg", "f2", 1 );
				q2avg = WHTAVEL( "q2", "f2", 1 );
			}

			// draw product complexity from current subsector avg.
			double DeltaG = V( "DeltaG" );		// last mach. gen. improv.
			double alpha3 = V( "alpha3" );		// beta distrib. alpha par.
			double beta3 = V( "beta3" );		// beta distrib. beta par.
			double x3inf = V( "x3inf" );		// lower beta draw support
			double x3sup = V( "x3sup" );		// upper beta draw support

			piRnd =  x3inf + ( x3sup - x3inf ) * beta( alpha3, beta3 );
			k2 = k2avg * ( 1 + piRnd ) * pow( 1 + DeltaG, gamma );
		}

		// set machine output, cost and price from adopted technology/complexity
		c2 = k2 * wAvg / ( VS( ind, "m2" ) * A2avg );
		p2 = ( 1 + mu20 ) * c2;

		// initialize industry parameters and variables
		WRITELLS( ind, "A2", A2avg, T - 1, 1 );
		WRITELLS( ind, "A2p", A2avg, T - 1, 1 );
		WRITELLS( ind, "E2", E2avg, T - 1, 1 );
		WRITELLS( ind, "E2avg", E2avgAvg, T - 1, 1 );
		WRITELLS( ind, "F2", F20, T - 1, 1 );
		WRITELLS( ind, "c2avg", c2, T - 1, 1 );
		WRITELLS( ind, "c2eAvg", c2, T - 1, 1 );
		WRITELLS( ind, "f2", f20, T - 1, 1 );
		WRITELLS( ind, "f2e", f20, T - 1, 1 );
		WRITELLS( ind, "k2", k2, T - 1, 1 );
		WRITELLS( ind, "old2vint", VNT( T - 1, 0 ), T - 1, 1 );
		WRITELLS( ind, "p2", p2, T - 1, 1 );
		WRITELLS( ind, "q2", q2avg, T - 1, 1 );
		WRITELLS( ind, "s2avg", sAvg, T - 1, 1 );
		WRITELLS( ind, "w2avg", wAvg, T - 1, 1 );
		WRITELLS( ind, "w2oAvg", w2oMax, T - 1, 1 );

		WRITES( ind, "ID2", ID2 );
		WRITES( ind, "t2ent", T );
		WRITES( ind, "type2", type2 );
		WRITES( ind, "k2", k2 );
		WRITES( ind, "f2", f20 );
		WRITES( ind, "f2e", f20 );
		WRITES( ind, "f2critChg", TregChg <= 0 || T < TregChg ? false : true );
		WRITES( ind, "f2posChg", TregChg <= 0 || T < TregChg ? 0 : 1 );
		WRITES( ind, "g2base", g2base );
		WRITES( ind, "mu20", mu20 );

		// remove empty firm instance on industry
		firm = SEARCHS( ind, "Firm2" );
		DELETE( firm );

		// add firms to industry
		equity += CFUNS( ind, entry_firm2, F20, true );
		*F2acc += F20;

		// initialize variables depending on firms
		if ( T == 1 )
			WRITELLS( ind, "K2", SUMLS( ind, "_K2", 1 ), 0, 1 );
		else
		{	// update variables still in t for not breaking statistics
			RECALCS( ind, "A2" ); RECALCS( ind, "A2p" ); RECALCS( ind, "D2e" );
			RECALCS( ind, "Deb2" ); RECALCS( ind, "F2" ); RECALCS( ind, "HH2" );
			RECALCS( ind, "HP2" ); RECALCS( ind, "K2d" ); RECALCS( ind, "NW2" );
			RECALCS( ind, "Q2u" ); RECALCS( ind, "c2avg" ); RECALCS( ind, "c2eAvg" );
			RECALCS( ind, "f2" ); RECALCS( ind, "mu2avg" );
			RECALCS( ind, "p2max" ); RECALCS( ind, "p2min" );
			RECALCS( ind, "q2max" ); RECALCS( ind, "q2min" );
			RECALCS( ind, "w2oMax" );

			// statistics only
			RECALCS( ind, "w2realAvg" );
		}

		// update country level variables
		INCR( "Fc", 1 );
		if ( basic )
			INCR( "FcBas", 1 );
		else
		{
			INCR( "FcLux", 1 );
			WRITE( "DeltaG", 0 );				// only reset if luxury industry
		}

		LOG( "\n %s industry started (t=%g): ID2=%d g2base=%d k2=%.3g FcBas=%g FcLux=%g",
			 basic ? "Basic" : "Luxury", T, ID2, g2base, k2, V( "FcBas" ), V( "FcLux" ) );
	}

	return equity;
}


// close under-performing consumption-goods industries, shutting-down their
// firms and accounting exit finance (bank bad debt and equity credit)
// in equation 'entryExit'

CFUN_DBL( exit_consumption, double *firmExits )
{
	bool toExitCbas, toExitClux;
	double NW2min, f2max, cEntry, cExit, nFail;
	int type2;
	object *ind, *cyccur;

	double fCmin = V( "fCmin" );				// minimum share to stay in s.2
	double nC = V( "nC" );						// sector 2 evaluation period
	int FcBas = V( "FcBas" );					// number of basic industries
	int FcBasMin = V( "FcBasMin" );				// min basic industries
	int FcLux = V( "FcLux" );					// number of luxury industries
	int FcLuxMin = V( "FcLuxMin" );				// min luxury industries

	// minimum wealth to stay in s.2
	NW2min = SUM( "NW2" ) * fCmin / ( FcBas + FcLux );

	// delete under-performing industries, saving exits proceeds
	cEntry = cExit = nFail = 0;
	toExitCbas = toExitClux = false;
	CYCLE( ind, "Consumption" )					// check industries to exit
	{
		f2max = max( VS( ind, "f2" ), VS( ind, "f2e" ) );
		type2 = VS( ind, "type2" );				// industry type (0=basic)

		// close industry if irrelevant in wealth or wallet share
		// but don't remove too young or to few industries
		if ( ( VS( ind, "NW2" ) < NW2min || f2max < fCmin ) &&
			 T >= VS( ind, "t2ent" ) + nC )
		{
			if ( ! ( type2 == 0 && FcBas <= FcBasMin ) &&
				 ! ( type2 > 0 && FcLux <= FcLuxMin ) )
			{
				// account exiting of all firms firms
				*firmExits -= CFUNS( ind, exit_firm2, & cEntry, & cExit,
									 & nFail, true );

				if ( type2 == 0 )
					--FcBas;
				else
					--FcLux;

				LOG( "\n %s industry closed (t=%g): (%s) ID2=%g age2=%g k2=%.3g FcBas=%d FcLux=%d",
					 VS( ind, "type2" ) == 0 ? "Basic" : "Luxury", T,
					 f2max < fCmin ? "m.s." : "bank", VS( ind, "ID2" ),
					 T - VS( ind, "t2ent" ), VS( ind, "k2" ), FcBas, FcLux );

				DELETE_EXTS( ind, ind2E );
				DELETE( ind );
			}
			else
				if ( type2 == 0 && FcBas <= FcBasMin )
					toExitCbas = true;
				else
					if ( type2 > 0 && FcLux <= FcLuxMin )
						toExitClux = true;
		}
	}

	WRITE( "FcBas", FcBas );
	WRITE( "FcLux", FcLux );
	WRITE( "Fc", FcBas + FcLux );
	WRITE( "toExitCbas", toExitCbas );
	WRITE( "toExitClux", toExitClux );

	return cExit;								// exit equity credits
}
