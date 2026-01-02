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

	double bonus = ( sec == 1 ) ? VL( "_Bon2", 1 ) : 0;// worker bonus
	double dividends = VL( _DivVar[ sec ], 1 );// shareholder dividends
	double cashFree = profit - tax - bonus - dividends;// final free cash flow

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
	int i, F1w;
	double _p1;
	object *broch, *cnt, *suppl;

	int flagTradeK = VS( GRANDPARENT, "flagTradeK" );// trade mode

	if ( flagTradeK == 1 || flagTradeK == 2 )		// int'l machine trade available?
	{
		object *cheap, *wrld = PARENTS( GRANDPARENT );
		double pKavg = VLS( wrld, "pKavgW", 1 ) *
					   VLS( GRANDPARENT, "e", 1 );	// world avg. price

		F1w = 0;
		CYCLES( wrld, cnt, "Country" )			// find total suppliers in world
			F1w += SUMS( V_EXTS( cnt, countryE, capSec ), "F1" );

		for ( i = 0, cheap = suppl = NULL; i < F1w && suppl == NULL; ++i )
		{
			// draw machine supplier worldwide
			cnt = RNDDRAW_FAIRS( wrld, "Country" );// draw supplier country
			suppl = RNDDRAWS( V_EXTS( cnt, countryE, capSec ), "Firm1", "_Atau" );
			_p1 = VS( suppl, "_p1" );			// machine FOB price

			if ( GRANDPARENTS( suppl ) != GRANDPARENT )// foreign? CIF
				_p1 *= ( VS( GRANDPARENT, "e" ) /
						 VS( GRANDPARENTS( suppl ), "e" ) ) /
					   ( ( 1 - VS( GRANDPARENT, "trMK" ) ) *
						 ( 1 - VS( PARENTS( suppl ), "trX1" ) ) );

			if ( _p1 > pKavg )					// over the world average?
			{
				if ( cheap == NULL || _p1 < VS( cheap, "_p1" ) )
					cheap = suppl;				// save cheapest so far

				suppl = NULL;					// keep searching
			}
		}

		if ( suppl == NULL )					// none below average?
			suppl = cheap;						// use cheapest
	}
	else
		suppl = RNDDRAWS( V_EXTS( GRANDPARENT, countryE, capSec ),
						  "Firm1", "_Atau" );	// draw domestic mach. supplier

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

	object *suppl = PARENTS( SHOOKS( HOOK( SUPPL ) ) );// current supplier
	double m2 = VS( PARENT, "m2" );				// machine output per period
	double _CS2a = V( "_CS2a" );				// available credit supply
	double _NW2 = V( "_NW2" );					// net worth (cash available)
	double _p1 = VS( suppl, "_p1" );			// machine FOB price

	if ( GRANDPARENTS( suppl ) != GRANDPARENT )	// foreign? CIF price
		_p1 *= ( VS( GRANDPARENT, "e" ) / VS( GRANDPARENTS( suppl ), "e" ) ) /
			   ( ( 1 - VS( GRANDPARENT, "trMK" ) ) *
				 ( 1 - VS( PARENTS( suppl ), "trX1" ) ) );

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
	double __Avint, __TaxMvint, __pVint, pDutyF;
	int __ageVint, __nMach, __nVint;
	object *cons, *cur, *suppl, *vint,
		   *cap = V_EXTS( GRANDPARENT, countryE, capSec );

	suppl = PARENTS( SHOOKS( HOOK( SUPPL ) ) );	// current supplier
	__nMach = floor( nMach );					// integer number of machines

	// at t=1 firms have a mix of machines: old to new, many suppliers
	if ( newInd )
	{
		cons = V_EXTS( GRANDPARENT, countryE, conSec );

		__ageVint = VS( cons, "eta" ) + 1;		// age of oldest machine
		__nVint = ceil( nMach / __ageVint );	// machines per vintage
		__Avint = INIPROD;						// initial product. in sector 2
		__pVint = VLS( cap, "p1avg", 1 );		// initial machine price
		__TaxMvint = 0;							// local supplier, no import tax
	}
	else
	{
		__ageVint = 1 - T;
		__nVint = __nMach;
		__Avint = VS( suppl, "_Atau" );
		__pVint = VS( suppl, "_p1" );			// machine FOB price

		if ( GRANDPARENTS( suppl ) != GRANDPARENT )// foreign? CIF price
		{
			pDutyF= __pVint * VS( GRANDPARENT, "e" ) /
							  VS( GRANDPARENTS( suppl ), "e" );// FOB in local $
			pDutyF /= 1 - VS( PARENTS( suppl ), "trX1" );// add foreign duty
			__pVint = pDutyF / ( 1 - VS( GRANDPARENT, "trMK" ) );
												// final CIF price in local $
			__TaxMvint = ( __pVint - pDutyF ) * __nVint;// local duty paid
		}
		else
			__TaxMvint = 0;						// local supplier, no import tax
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
		WRITES( vint, "__AeVint", __Avint );	// vintage effective product.
		WRITES( vint, "__TaxMvint", __TaxMvint );// import tax paid in vintage
		WRITES( vint, "__nVint", __nVint );		// number of machines in vintage
		WRITES( vint, "__pVint", __pVint );		// price of machines in vintage
		WRITES( vint, "__tVint", 1 - __ageVint );// vintage build time
		WRITELLS( vint, "__AeVint", __Avint, T, 1 );// lagged value

		DELETE( SEARCHS( vint, "WrkV" ) );		// remove empty worker object

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
	object *wrk;

	if ( NEXT != NULL )							// don't remove last vintage
	{
		// move all workers out from this vintage
		CYCLE( wrk, "WrkV" )
			WRITE_HOOKS( SHOOKS( wrk ), VWRK, NULL );

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


/*================== LABOR MANAGEMENT SUPPORT C FUNCTIONS ====================*/

// define worker education level and category in equations 'initCountry', '_age'

#define EDU_MIN 0.1								// minimum schooling year
#define EDU_MAX	16								// maximum schooling years
#define EDU_SEC	9								// start year of secondary education
#define EDU_TER	13								// start year of tertiary education

CFUN_VOID( set_education )
{
	if ( VS( GRANDPARENT, "flagEduc" ) == 0 )
	{
		WRITE( "_cat", 1 );
		return;
	}

	object *lab = PARENT;
	double g = pow( VS( lab, "epsilonEd" ) / VS( lab, "epsilonAd" ),
					VS( lab, "varthetaEd" ) );	// gov. expenditure effect
	double _ed = max( EDU_MAX * beta( g * VS( lab, "alphaEd" ),// years schooling
									  VS( lab, "betaEd" ) / g ), EDU_MIN );
	double _cat = _ed < EDU_SEC ? 1 : ( _ed < EDU_TER ? 2 : 3 );// edu. category

	WRITE( "_ed", _ed );
	WRITE( "_cat", _cat );
}


// count firm's workers of given category in equations '_L21', '_L22', '_L23'

CFUN_DBL( count_workers, int cat )
{
	object *wrk;

	int n = 0;
	CYCLE( wrk, "Wrk2" )						// count current workers in cat.
		if ( VS( SHOOKS( wrk ), "_cat" ) == cat )
			++n;

	return n * VS( V_EXTS( GRANDPARENT, countryE, labSup ), "Lscale" );
}


// compute the number of job positions to open to reach desired labor in category
// in equations '_JO21', '_JO22', '_JO23'

const char *_L2dVar[ ] = { "_L2d1", "_L2d2", "_L2d3" };

CFUN_DBL( open_positions, int cat )
{
	object *wrk, *country = GRANDPARENT,
		   *lab = V_EXTS( country, countryE, labSup );

	VS( V_EXTS( country, countryE, capSec ), "hires1" );// ensure sector 1 done
	V( "_fires2" );								// and own fires also done

	double _L2net = V( _L2dVar[ cat - 1 ] ) - CFUN( count_workers, cat );

	return max( ceil( ( 1 + VS( lab, "theta" ) ) * _L2net ), 0 );
}


// update a worker after firing in equations 'fires1', '_fires2', 'entry2exit',
// 'quits1', 'retires1', '_quits2', '_retires2'

CFUN_VOID( fire_worker )
{
	WRITE( "_employed", 0 );					// register fire
	WRITE( "_Te", 0 );

	if ( LAST_CALC( "_In" ) != T )				// early fire?
		WRITE( "_w", 0 );						// no wage

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


// update a worker after hiring in equations 'hire1', 'hire2'

CFUN_VOID( hire_worker, int sec, object *firm, double wage )
{
	int flagWorkerLBU;
	object *wrk;

	int _ID = V( "_ID" );
	int _employed = V( "_employed" );

	if ( _employed )							// worker must quit first?
	{
		double Lscale = VS( PARENT, "Lscale" );	// labor scaling

		if ( _employed == 1 )					// sector 1?
			INCRS( V_EXTS( GRANDPARENT, countryE, capSec ), "quits1", Lscale );
		else									// no: assume sector 2
			INCRS( PARENTS( HOOK( FWRK ) ), "_quits2", Lscale );

		CFUN( fire_worker );					// register fire
	}

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
		WRITES( wrk, "__IDw2", _ID );			// register worker ID ID
	}

	WRITE_HOOK( FWRK, wrk );					// pointer to firm from worker
}


// move worker to a different vintage in equation 'alloc2'

CFUN_VOID( move_worker, object *vint, bool vint_learn )
{
	double sV;
	int IDv;
	object *wrkV;

	if ( vint_learn )							// learning-by-vintage mode?
	{											// worker has public skills
		IDv = VS( vint, "__IDvint" );
		sV = V_EXTS( GRANDPARENTS( vint ), countryE, vintProd[ IDv ].sVp );
	}
	else
		sV = INISKILL;

	wrkV = ADDOBJS( vint, "WrkV" );				// add worker-bridge object
	WRITE_SHOOKS( wrkV, THIS );					// save pointer to work object
	WRITE_HOOK( VWRK, wrkV );					// register vint. in worker

	WRITES( wrkV, "___IDwV", V( "_ID" ) );
	WRITE( "_sV", sV );							// set vintage skills
	WRITE( "_CQ", 0 );							// no cumulated production yet
}


// order wage offers in equation 'hires2'

bool wo_asc_wrk( wageOffer e1, wageOffer e2 ) { return e1.workers < e2.workers; };
bool wo_desc_off( wageOffer e1, wageOffer e2 ) { return e1.offer > e2.offer; };

CFUN_VOID( shuffle_offers, woLisT *offers )
{
	// make a copy of the workers list into a vector
	vector < wageOffer > temp( offers->size( ) );
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

	// always shuffle orders to prevent preference when same wages are offered
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


// sort job applications in equations 'hires1', 'hires2'

bool appl_asc_w( application e1, application e2 ) { return e1.w < e2.w; };
bool appl_desc_w( application e1, application e2 ) { return e1.w > e2.w; };
bool appl_asc_edS( application e1, application e2 ) { return e1.edS < e2.edS; };
bool appl_desc_edS( application e1, application e2 ) { return e1.edS > e2.edS; };
bool appl_asc_wEdS( application e1, application e2 ) { return e1.wEdS < e2.wEdS; };
bool appl_desc_wEdS( application e1, application e2 ) { return e1.wEdS > e2.wEdS; };
bool appl_asc_Te( application e1, application e2 ) { return e1.Te < e2.Te; };
bool appl_desc_Te( application e1, application e2 ) { return e1.Te > e2.Te; };

CFUN_VOID( order_applications, int order, appLisT *appl )
{
	if ( appl->size( ) == 0 )					// prevent empty lists
		return;

	vector < application > temp( appl->size( ) );

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
		case 3:									// higher edu+skills first order
			appl->sort( appl_desc_edS );
			break;
		case 4:									// lower edu+skills first order
			appl->sort( appl_asc_edS );
			break;
		case 5:									// higher payback first order
			appl->sort( appl_desc_wEdS );
			break;
		case 6:									// lower payback first order
			appl->sort( appl_asc_wEdS );
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
		   *keyName[ ] = { "_key1", "__key2" };

CFUN_VOID( order_workers, int order, int obj )
{
	char keyN[ 4 ], dir[ 5 ];
	double edS, keyV;
	object *wrk, *country;

	if ( strcmp( NAME, "Capital" ) == 0 )
		country = PARENT;
	else
		country = GRANDPARENT;

	switch ( order )							// handle selected sort scheme
	{
		default:
		case 0:									// random order
		case 4:									// lower edu+skills first order
		case 6:									// lower payback first order
			strcpy( dir, "UP" );
			break;
		case 3:									// higher edu+skills first order
		case 5:									// higher payback first order
			strcpy( dir, "DOWN" );
			break;
		case 1:									// higher wage first order
			strcpy( keyN, "_w" );
			strcpy( dir, "DOWN" );
			break;
		case 2:									// lower wage first order
			strcpy( keyN, "_w" );
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
			if ( order == 1 || order == 2 || order == 7 || order == 8 )
				keyV = VLS( SHOOKS( wrk ), keyN, 1 );
			else
			{
				keyV = VLS( SHOOKS( wrk ), "_edS", 1 );

				if ( order == 5 || order == 6 )
					keyV = VLS( SHOOKS( wrk ), "_w", 1 ) / keyV;
			}

		WRITES( wrk, keyName[ obj ], keyV );	// copy key to bridge obj
	}

	SORT( wrkName[ obj ], keyName[ obj ], dir );// sort the bridge objects
}


// compute wage offer according to worker category in '_w2o1', '_w2o2', '_w2o3'

const char *_L2var[ ] = { "_L21", "_L22", "_L23" },
		   *_w2oVar[ ] = { "_w2o1", "_w2o2", "_w2o3" },
		   *phiPar[ ] = { "", "phi2", "phi3" },
		   *phiGpar[ ] = { "", "phi2g", "phi3g" },
		   *w2oAvgVar[ ] = { "w2o1avg", "w2o2avg", "w2o3avg" };

CFUN_DBL( wage_offer, int cat )
{
	double _L2_1, _L2vac_1, _dA2b, _w2o, _w2o_1, dAb_1, dCPIb_1, dUeB_1,
		   invMult, maxW, mult, phi, psi1, psi2, psi3, psi4, psi5, w2oAvg_1,
		   wCap;
	int WageOffer, hOrder, i, n, v, _life2cycle;
	object *cons = PARENT, *country = PARENTS( cons ),
		   *lab = V_EXTS( country, countryE, labSup );

	if ( VS( country, "flagEduc" ) == 0 && cat != 1 )
		return 0;								// only category 1 workers?

	v = cat - 1;								// vars start from zero
	phi = v > 0 ? ( V( "_own2" ) == 1 ? VS( lab, phiGpar[ v ] ) :
										VS( lab, phiPar[ v ] ) ) : 0;
												// category wage premium
	_L2_1 = VL( _L2var[ v ], 1 );				// existing workers in category

	if ( VS( country, "flagHeterWage" ) == 0 )	// centralized wage setting?
	{
		_w2o = VS( lab, "wCent" );				// single wage centrally defined
		goto end_offer;
	}

	_life2cycle = V( "_life2cycle" );			// firm status
	_w2o_1 = VL( _w2oVar[ v ], 1 );				// current wage offer in category
	w2oAvg_1 = VLS( cons, w2oAvgVar[ v ], 1 );	// average market offer
	wCap = VS( lab, "wCap" );					// wage cap multiplier
	WageOffer = V( "_postChg" ) ? VS( country, "flagWageOfferChg" ) :
								  VS( country, "flagWageOffer" );

	if ( WageOffer == 0 )						// wage premium mode?
	{
		if ( _life2cycle == 0 )					// if entrant
			_w2o = w2oAvg_1;					// use market average as base
		else
			_w2o = _w2o_1;

		switch ( ( int ) VS( country, "flagWagePremium" ) )
		{										// define wage premium type
			case 0:								// no premium
			default:
				break;

			case 1:								// indexed premium (WP1)
				psi1 = VS( lab, "psi1" );		// inflation adjust. parameter
				psi2 = VS( lab, "psi2" );		// general prod. adjust. param.
				psi3 = VS( lab, "psi3" );		// unemploym. adjust. parameter
				psi4 = VS( lab, "psi4" );		// firm prod. adjust. parameter
				psi5 = VS( lab, "psi5" );		// firm vacancy booster param.
				dCPIb_1 = VLS( cons, "dCPIb", 1 );// inflation variation
				dAb_1 = VLS( country, "dAb", 1 );// general productivity var.
				dUeB_1 = VLS( lab, "dUeB", 1 );	// unemployment variation
				_L2vac_1 = VL( "_L2vac", 1 );	// previous vacancy rate

				// notional productivity variation (firm), consider entrants
				_dA2b = ( _life2cycle == 0 ) ? 0 : VL( "_dA2b", 1 );

				// make sure total productivity effect is bounded to 1
				if ( ( psi2 + psi4 ) > 1 )
					psi2 = max( 1 - psi4, 0 );	// adjust general prod. effect

				_w2o *= 1 + psi1 * dCPIb_1 + psi2 * dAb_1 + psi3 * dUeB_1 +
						psi4 * _dA2b + psi5 * _L2vac_1;
				break;

			case 2:								// endogenous mechanism (WP2)
				if ( _w2o != 0 )				// valid  offer last period?
					_w2o *= 1 + max( w2oAvg_1 / _w2o - 1, 0 );
				else							// no: use market average
					_w2o = w2oAvg_1;
		}
	}
	else
	{											// lowest wage mode
		VS( lab, "appl" );						// ensure applications are done
		n = ceil( ( V( _L2dVar[ v ] ) - _L2_1 ) *
				  ( 1 + VS( lab, "theta" ) ) / VS( lab, "Lscale" ) );
												// number of workers (scaled)
		n = max( n, 1 );						// minimum one worker for calc.

		// sort firm's candidate list according to the defined strategy
		hOrder = V( "_own2" ) == 1 ? VS( country, "flagHireOrder2g" ) :
				 V( "_postChg" ) ? VS( country, "flagHireOrder2Chg" ) :
								   VS( country, "flagHireOrder2" );
		CFUN( order_applications, hOrder, & V_EXT( firm2E, appl ) );

		// search applications set (increasing wage requests) for enough workers
		i = 0;									// workers counter
		_w2o = 0;								// highest wage found
		appLisT::iterator its;
		CYCLE_EXT( its, firm2E, appl )			// run over enough applications
		{
			if ( its->cat != cat )				// ignore other categories
				continue;

			if ( its->w > _w2o )				// new high wage request?
				_w2o = its->w;					// i-th worker wage

			if ( ++i >= n )
				break;							// stop when enough workers
		}

		if ( _w2o == 0 )						// no worker in sub-queue
			_w2o = _w2o_1;						// keep current offer
	}

	// check for abnormal change
	if ( _life2cycle > 0 && wCap > 0 )
	{
		mult = _w2o / _w2o_1;					// calculate multiple
		invMult = mult < 1 ? 1 / mult : mult;

		if ( invMult > wCap )					// explosive change?
			_w2o = mult < 1 ? _w2o_1 / wCap : _w2o_1 * wCap;
	}

	// check if non-entrant firm is able to pay wage
	if ( _life2cycle > 0 )
	{
		maxW = VL( "_p2", 1 ) * VL( "_A2", 1 );	// max wage for minimum markup
		if ( maxW > 0 && _w2o > maxW )			// over max?
			_w2o = maxW;
		else
			if ( maxW <= 0 )					// max can't be calculated?
				_w2o = min( _w2o, _w2o_1 );		// limit to current
	}

	// under unemployment benefit or minimum wage? Adjust if necessary
	_w2o = max( _w2o, max( VS( lab, "wU" ), VS( lab, "wMinPol" ) ) );

	end_offer:

	// consider category relative floor
	if ( cat >= 2 )
		_w2o = max( _w2o, ( 1 + phi ) * V( _w2oVar[ v - 1 ] ) );

	// save offer in global offers sets
	wageOffer woData;
	woData.offer = _w2o;
	woData.workers = _L2_1;
	woData.firm = THIS;

	// block access to firm2woX from other parallel threads
	lock_guard < mutex > lock( V_EXTS( country, countryE, firm2woMtx ) );

	if ( cat == 1 )
		EXEC_EXTS( country, countryE, firm2wo1, push_back, woData );
	else
		if ( cat == 2 )
			EXEC_EXTS( country, countryE, firm2wo2, push_back, woData );
		else
			EXEC_EXTS( country, countryE, firm2wo3, push_back, woData );

	return _w2o;
}


// do hiring in equation 'hires21', 'hires22', 'hires23'

const char *_JO2var[ ] = { "_JO21", "_JO22", "_JO23" };

CFUN_DBL( hire_workers, int cat )
{
	double wMin;
	int hires, open, hired = 0;
	object *wrk, *country = PARENT,
		   *lab = V_EXTS( country, countryE, labSup );
	appLisT *appl;
	woLisT *offers;

	double Lscale = VS( lab, "Lscale" );		// labor scaling
	int hSeq = VS( country, "flagHeterWage" ) == 0 ?
			   0 : VS( country, "flagHireSeq" );// firm hiring order

	// create pointer and sort wage offers list
	if ( cat == 1 )
		offers = & V_EXTS( country, countryE, firm2wo1 );
	else
		if ( cat == 2 )
			offers = & V_EXTS( country, countryE, firm2wo2 );
		else
			offers = & V_EXTS( country, countryE, firm2wo3 );

	CFUN( order_offers, hSeq, offers );

	// firms hire employees according to the selected hiring order
	for ( auto ito = offers->begin( ); ito != offers->end( ); ++ito )
	{
		// hire the ordered applications until queue is exhausted for category
		open = ceil( VS( ito->firm, _JO2var[ cat - 1 ] ) / Lscale );
												// firm's jobs open (scaled)
		hires = 0;								// firm hiring counter
		wrk = NULL;								// next cheaper worker applying
		wMin = DBL_MAX;							// next lower wage requested
		appl = & V_EXTS( ito->firm, firm2E, appl );// applications list
		auto ita = appl->begin( );				// first application

		// run through the applications till all positions filled or list over
		while( open - hires > 0 && ita != appl->end( ) )
			if ( VS( ita->wrk, "_cat" ) != cat )
				++ita;							// ignore other worker categories
			else
			{
				// candidate not yet hired in this period and offered wage ok?
				if ( ! ( VS( ita->wrk, "_employed" ) != 0 &&
						 VS( ita->wrk, "_Te" ) == 0 ) )
				{
					if ( ROUND( ita->w, ito->offer, 0.01 ) <= ito->offer )
					{
						// flag hiring and set wage, employer & vintage of worker
						CFUNS( ita->wrk, hire_worker, 2, ito->firm, ito->offer );
						++hires;				// scaled count hire (firm)
					}
					else
						if ( ita->w < wMin )
						{
							wMin = ita->w;
							wrk = ita->wrk;
						}
				}

				ita = appl->erase( ita );		// remove worker from list
			}

		// try to hire at least one worker, at any wage
		if ( open - hires > 0 && hires == 0 && wrk != NULL )
		{
			CFUNS( wrk, hire_worker, 2, ito->firm, wMin );// pay requested wage
			++hires;
		}

		// adjust lower categories demand to account for unfilled positions
		if ( open - hires > 0 && cat > 1 )
			INCRS( ito->firm, _L2dVar[ cat - 2 ], open - hires );

		INCRS( ito->firm, "_hires2", hires * Lscale );// update firm hires
		hired += hires;							// total hired in sector
	}

	offers->clear( );							// clear offers set

	return hired * Lscale;
}


// do firing for firm in equation 'fires2'

#define MODE_ALL 1								// fire all workers
#define MODE_ADJ 2								// fire only for adjustment
#define MODE_PBACK 3							// fire negative paybacks
#define MODE_IPROT 4							// fire non protected workers
#define MODE_EXIT 5								// fire all when firm exiting

CFUN_DBL( fire_workers, int mode, double xsCap, double *redCap )
{
	bool fire;
	int Te, i;
	object *cyccur, *wrk, *worker;

	object *country = GRANDPARENT;				// pointers to objects
	object *lab = V_EXTS( country, countryE, labSup );

	double Lscale = VS( lab, "Lscale" );		// labor scale
	double w2avg = VLS( PARENT, "w2avg", 1 );	// average wage
	int Tp = VS( lab, "Tp" );					// time for protected workers

	i = 0;										// fired workers counter
	*redCap = 0;								// reduced capacity accumulator
	xsCap *= 1 - VS( lab, "theta" );			// create slack (extra workers)

	// order workers to fire according firm preference
	int fOrder = V( "_own2" ) == 1 ? VS( country, "flagFireOrder2g" ) :
				 V( "_postChg" ) ? VS( country, "flagFireOrder2Chg" ) :
								   VS( country, "flagFireOrder2" );
	if ( mode == MODE_PBACK )					// explicit payback firing?
		fOrder = 0;								// ignore order set

	// create sorted list of workers according to the chosen attributes
	CFUN( order_workers, fOrder, OBJ_WRK2 );	// sort bridge objects

	// check firing worker by worker: firm desired adjustments
	CYCLE_SAFE( wrk, "Wrk2" )
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
					if ( Te <= Tp )				// is worker yet unprotected
						fire = true;

				break;

			case MODE_PBACK:					// fire negative paybacks
				// insufficient payback
				if ( VLS( worker, "_w", 1 ) / w2avg / VLS( worker, "_edS", 1 ) > 1 )
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

// find best machine technologies (domestic or int'l) in 'entry_firm1' and
// 'rescue_firm'

CFUN_VOID( best_tech, double & AtauMax, double & BtauMax )
{
	object *cap, *cnt;

	if ( VS( PARENT, "flagTradeK" ) > 1 )		// int'l imitation?
	{
		// find best technologies worldwide
		AtauMax = BtauMax = 0;
		CYCLES( GRANDPARENT, cnt, "Country" )
		{
			cap = V_EXTS( cnt, countryE, capSec );// country's capital sector
			AtauMax = max( AtauMax, MAXS( cap, "_Atau" ) );
			BtauMax = max( BtauMax, MAXS( cap, "_Btau" ) );
		}
	}
	else
	{
		AtauMax = MAX( "_Atau" );				// best machine productivity
		BtauMax = MAX( "_Btau" );				// best productivity in sector 1
	}
}


// add and configure entrant capital-good firm object(s) and required hooks
// in equations 'entry1exit' and 'initCountry'

CFUN_DBL( entry_firm1, int n, bool newInd )
{
	double _Atau, _Btau, _D10, _Deb1, _Eq1, _L1rd, _NW1, _NW10, _RD0, _c1, _f1,
		   _p1, _sV, AtauMax, BtauMax, Deb1, Eq1, NW1, w1avg, mult;
	int _ID1, _t1ent;
	object *firm, *bank,
		   *cons = V_EXTS( PARENT, countryE, conSec ),
		   *lab = V_EXTS( PARENT, countryE, labSup );

	double Deb10 = V( "Deb10" );				// bank fin. to equity ratio
	double Phi3 = V( "Phi3" );					// lower support for wealth share
	double Phi4 = V( "Phi4" );					// upper support for wealth share
	double alpha2 = V( "alpha2" );				// lower support for imitation
	double beta2 = V( "beta2" );				// upper support for imitation
	double mu1 = V( "mu1" );					// mark-up in sector 1
	double m1 = V( "m1" );						// worker production scale
	double nu = V( "nu" );						// share of R&D expenses
	double x5 = V( "x5" );						// entrant upper advantage
	int IDcnt = VS( PARENT, "IDcnt" );			// country ID

	if ( newInd )
	{
		double F20 = VS( cons, "F20" );
		double m2 = VS( cons, "m2" );			// machine output per period

		_Atau = AtauMax = INIPROD;				// initial productivities to use
		_Btau = BtauMax = ( 1 + mu1 ) * _Atau / ( m1 * m2 * VS( cons, "b" ) );
												// and build machines (s. s.)
		_NW10 = V( "NW10" );					// initial wealth in sector 1
		_f1 = 1.0 / n;							// fair share
		_sV = VLS( lab, "sAvg", 1 );			// initial worker vintage skills
		_t1ent = 0;								// entered before t=1
		w1avg = VLS( lab, "wAvg", 1 );			// initial average wage

		// initial demand expectation, assuming all sector 2 firms, 1/eta
		// replacement factor and fair share in sector 1 and full employment
		double p20 = VLS( cons, "CPI", 1 );
		double K0 = ceil( VS( lab, "Ls0" ) * w1avg / p20 / F20 / m2 ) * m2;

		_D10 = F20 * K0 / m2 / VS( cons, "eta" ) / n;
	}
	else
	{
		_NW10 = max( WHTAVE( "_NW1", "_f1" ), V( "NW10" ) *
					 V( "PPI" ) / V( "pK0" ) );
		_f1 = 0;								// no market share
		_sV = INISKILL;							// worker vintage skills
		_t1ent = T;								// entered now
		w1avg = V( "w1avg" );					// average wage in sector 1

		// initial demand equal to 1 machine per client under fair share entry
		_D10 = VS( cons, "F2" ) / V( "F1" );

		// find technological frontier reference
		CFUN( best_tech, AtauMax, BtauMax );
	}

	// add entrant firms (end of period, don't try to sell)
	for ( Deb1 = Eq1 = NW1 = 0; n > 0; --n )
	{
		// create object, only recalculate in t if new industry
		if ( newInd )
			firm = ADDOBJL( "Firm1", T - 1 );
		else
			firm = ADDOBJ( "Firm1" );

		_ID1 = ID( IDcnt, 1, INCR( "lastID1", 1 ) );// new firm ID
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
		_c1 = w1avg / ( _Btau * m1 );			// unit cost
		_p1 = ( 1 + mu1 ) * _c1;				// unit price
		_RD0 = max( nu * _D10 * _p1, w1avg );	// R&D expense
		_L1rd = floor( _RD0 / w1avg );			// workers in R&D

		// accumulate capital costs
		NW1 += _NW1 = mult * _NW10;
		Deb1 += _Deb1 = _NW1 * Deb10;
		Eq1 += _Eq1 = _NW1 * ( 1 - Deb10 );

		// initialize variables
		WRITES( firm, "_Eq1", _Eq1 );
		WRITES( firm, "_t1ent", _t1ent );
		WRITES( firm, "_own1", 0 );
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

			// initialize the map of vintage productivity and skills
			WRITE_EXTS( PARENT, countryE, vintProd[ VNT( T - 1, _ID1 ) ].sVp, _sV );
			WRITE_EXTS( PARENT, countryE, vintProd[ VNT( T - 1, _ID1 ) ].sVavg, _sV );
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
		INCR( "Eq1entryW", Eq1 );
	}

	return Eq1;
}


// add and configure entrant consumer-good firm object(s) and required hooks
// in equations 'entry2exit' and 'initCountry'

CFUN_DBL( entry_firm2, int n, bool newInd )
{
	bool _postChg;
	double _A2, _D20, _D2e, _Deb2, _E2, _Eq2, _K, _N, _NW2, _NW2f, _NW20, _Q2u,
		   _c2, _f2, _life2cycle, _p1, _p2, _q2, Deb2, Eq2, K, N, NW2, f2posChg,
		   w2avg, w2o1avg, w2o2avg, w2o3avg, w2realAvg, mult;
	int _ID2, _t2ent;
	object *firm, *bank, *suppl,
		   *cap = V_EXTS( PARENT, countryE, capSec ),
		   *lab = V_EXTS( PARENT, countryE, labSup );

	bool AllFirmsChg = VS( PARENT, "flagAllFirmsChg" );// change at once?
	bool f2critChg = V( "f2critChg" );			// critical change thresh. met?
	double Deb20 = V( "Deb20" );				// bank fin. to equity ratio
	double Phi1 = V( "Phi1" );					// lower support for K share
	double Phi2 = V( "Phi2" );					// upper support for K share
	double e = VS( PARENT, "e" );				// exchange rate
	double ent2HldShr = V( "ent2HldShr" );		// hold share post-chg firms
	double f2minPosChg = V( "f2minPosChg" );	// min m.s. post-chg firms
	double iota = V( "iota" );					// desired inventories factor
	double mu20 = V( "mu20" );					// initial mark-up in sector 2
	double m2 = V( "m2" );						// machine output per period
	double p10 = VLS( cap, "p1avg", 1 );		// initial machine price
	double u = V( "u" );						// desired capital utilization
	double sAvg = VLS( lab, "sAvg", 1 );		// initial worker compound skills
	double trMK = VS( PARENT, "trMK" );			// import tax in country
	int IDcnt = VS( PARENT, "IDcnt" );			// country ID
	int TregChg = VS( PARENT, "TregChg" );		// time for regime change

	if ( newInd )
	{
		double phi = VS( lab, "phi" );			// unemployment benefit rate
		double wAvg = VLS( lab, "wAvg", 1 );	// initial average wage
		double c10 = p10 / ( 1 + VS( cap, "mu1" ) );// initial unit cost sec. 1
		double c20 = wAvg / INIPROD;			// initial unit cost sec. 2
		double p20 = ( 1 + mu20 ) * c20;		// initial consumer-good price
		double trIn = VS( PARENT, "trIn" );		// tax rate on income
		double K0 = ceil( VS( lab, "Ls0" ) * wAvg /
						  p20 / n / m2 ) * m2;	// full employment K required
		double SIr0 = n * K0 / m2 / V( "eta" );	// substit. real invest.
		double RD0 = VS( cap, "nu" ) * SIr0 * p10;// initial R&D expense

		// initial steady state demand under fair share
		_D20 = ( ( SIr0 * c10 + RD0 ) * ( 1 - phi - trIn ) +
				VS( lab, "Ls0" ) * wAvg * phi ) /
			  ( mu20 + phi + trIn ) * c20 / n;
		_E2 = VL( "E2avg", 1 );					// initial competitiveness
		_K = K0;								// initial capital in sector 2
		_N = iota * _D20;						// initial inventories
		_NW20 = V( "NW20" );					// initial wealth in sector 2
		_Q2u = 1;								// initial capacity utilization
		_f2 = 1.0 / n;							// fair share
		_life2cycle = 3;						// start as incumbent
		_q2 = 1;								// initial quality
		_t2ent = 0;								// entered before t=1
		f2posChg = 0;							// m.s. of post-change firms
		w2avg = w2realAvg = wAvg;				// initial average wage
		w2o1avg = INIWAGE;						// initial offered wages
		w2o2avg = VL( "w2o2avg", 1 );
		w2o3avg = VL( "w2o3avg", 1 );
	}
	else
	{
		_D20 = 0;
		_E2 = V( "E2avg" );						// average competitiveness
		_K = WHTAVE( "_K", "_f2" );				// w. avg. capital in sector 2
		_N = 0;									// inventories
		_NW20 = WHTAVE( "_NW2", "_f2" );		// average wealth in sector 2
		_Q2u = V( "Q2u" );						// capacity utilization
		_f2 = 0;								// no market share
		_life2cycle = 0;						// start as pre-operat. entrant
		_q2 = V( "q2avg" );						// average quality
		_t2ent = T;								// entered now
		f2posChg = V( "f2posChg" );				// m.s. of post-change firms
		w2avg = V( "w2avg" );					// average wage in sector 2
		w2o1avg = V( "w2o1avg" );				// average cat. 1 wage offer
		w2o2avg = V( "w2o2avg" );				// average cat. 2 wage offer
		w2o3avg = V( "w2o3avg" );				// average cat. 3 wage offer
		w2realAvg = V( "w2realAvg" );			// average real wage in s. 2
	}

	// add entrant firms (end of period, don't try to sell)
	for ( Deb2 = Eq2 = NW2 = K = N = 0; n > 0; --n )
	{
		// create object, only recalculate in t if new industry
		if ( newInd )
			firm = ADDOBJL( "Firm2", T - 1 );
		else
			firm = ADDOBJ( "Firm2" );

		_ID2 = ID( IDcnt, 2, INCR( "lastID2", 1 ) );// new firm ID
		WRITES( firm, "_ID2", _ID2 );

		ADDHOOKS( firm, FIRM2HK );				// add object hooks
		ADDEXTS( firm, firm2E );				// allocate extended data
		DELETE( SEARCHS( firm, "Vint" ) );		// remove empty instances
		DELETE( SEARCHS( firm, "Broch" ) );
		DELETE( SEARCHS( firm, "Wrk2" ) );

		// select associated bank
		bank = CFUNS( firm, set_bank );

		// select initial machine supplier
		suppl = CFUNS( firm, set_supplier );

		// choose firm type (pre/post-change)
		if ( TregChg <= 0 || T < TregChg )		// before regime change?
			_postChg = false;					// it's pre-change type
		else
			if ( AllFirmsChg )					// all firms change at once?
				_postChg = true;				// it's post-change type
			else
				if ( ! f2critChg || newInd )	// critical threshold not met?
					// fixed type proportion
					_postChg = ( RND < ent2HldShr ) ? true : false;
				else
					// draw type according shares
					_postChg = ( RND < max( f2posChg, f2minPosChg ) ) ? true : false;

		// initial desired capital/expected demand, rounded to # of machines
		mult = newInd ? 1 : uniform( Phi1, Phi2 );// capital multiple
		K += _K = ceil( max( mult * _K / m2, 1 ) ) * m2;
		_D2e = newInd ? _D20 : u * _K;
		N += _N;

		// define entrant initial free cash (1 period wages or default minimum)
		mult = newInd ? 1 : uniform( Phi1, Phi2 );// NW multiple
		_A2 = VS( suppl, "_Atau" );				// initial productivity
		_c2 = w2avg / _A2;						// initial unit costs
		_p2 = ( 1 + mu20 ) * _c2;				// initial price
		_NW2f = ( 1 + iota ) * _D2e * _c2;		// initial free cash
		_NW2f = max( _NW2f, mult * _NW20 );

		// initial equity must pay initial capital and wages
		_p1 = VS( suppl, "_p1" );				// machine FOB price

		if ( GRANDPARENTS( suppl ) != PARENT )	// foreign? CIF price
			_p1 *= ( e / VS( GRANDPARENTS( suppl ), "e" ) ) /
				   ( ( 1 - trMK ) * ( 1 - VS( PARENTS( suppl ), "trX1" ) ) );

		NW2 += _NW2 = newInd ? _NW2f : _p1 * _K / m2 + _NW2f;
		Deb2 += _Deb2 = _NW2 * Deb20;
		Eq2 += _Eq2 = _NW2 * ( 1 - Deb20 );		// accumulated equity (all firms)

		// initialize variables
		WRITES( firm, "_Eq2", _Eq2 );
		WRITES( firm, "_own2", 0 );
		WRITES( firm, "_t2ent", _t2ent );
		WRITES( firm, "_life2cycle", _life2cycle );
		WRITES( firm, "_postChg", _postChg );
		WRITELLS( firm, "_A2", _A2, _t2ent, 1 );
		WRITELLS( firm, "_f2", _f2, _t2ent, 1 );
		WRITELLS( firm, "_f2", _f2, _t2ent, 2 );
		WRITELLS( firm, "_mu2", mu20, _t2ent, 1 );
		WRITELLS( firm, "_p2", _p2, _t2ent, 1 );
		WRITELLS( firm, "_qc2", 4, _t2ent, 1 );
		WRITELLS( firm, "_s2avg", sAvg, _t2ent, 1 );
		WRITELLS( firm, "_sT2min", INISKILL, _t2ent, 1 );
		WRITELLS( firm, "_w2avg", w2avg, _t2ent, 1 );
		WRITELLS( firm, "_w2o1", w2o1avg, _t2ent, 1 );
		WRITELLS( firm, "_w2o2", w2o2avg, _t2ent, 1 );
		WRITELLS( firm, "_w2o3", w2o3avg, _t2ent, 1 );

		for ( int i = 1; i <= 4; ++i )
		{
			WRITELLS( firm, "_D2l", _D2e, _t2ent, i );
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
			WRITES( firm, "_A2p", _A2 );
			WRITES( firm, "_D2e", _D2e );
			WRITES( firm, "_Deb2", _Deb2 );
			WRITES( firm, "_E2", _E2 );
			WRITES( firm, "_Kd", _K );
			WRITES( firm, "_NW2", _NW2 );
			WRITES( firm, "_Q2u", _Q2u );
			WRITES( firm, "_c2", _c2 );
			WRITES( firm, "_c2e", _c2 );
			WRITES( firm, "_mu2", mu20 );
			WRITES( firm, "_p2", _p2 );
			WRITES( firm, "_q2", _q2 );
			WRITES( firm, "_s2avg", sAvg );
			WRITES( firm, "_w2avg", w2avg );
			WRITES( firm, "_w2o1", w2o1avg );
			WRITES( firm, "_w2o2", w2o2avg );
			WRITES( firm, "_w2o3", w2o3avg );
			WRITES( firm, "_w2realAvg", w2realAvg );

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
		INCR( "Eq2entryW", Eq2 );
	}

	return Eq2;
}


// rescue firm and allocate new required in equation 'entry1exit', 'entry2exit'

const char *_EqVar[ ] = { "_Eq1", "_Eq2" },
		   *_ownVar[ ] = { "_own1", "_own2" },
		   *EqVar[ ] = { "Eq1", "Eq2" },
		   *EqEntryGvar[ ] = { "Eq1entryG", "Eq2entryG" },
		   *EqEntryWvar[ ] = { "Eq1entryW", "Eq2entryW" },
		   *EqExitGVar[ ] = { "Eq1exitG", "Eq2exitG" },
		   *EqExitWVar[ ] = { "Eq1exitW", "Eq2exitW" },
		   *NW0var[ ] = { "NW10", "NW20" };

CFUN_DBL( rescue_firm, bool stat )
{
	double _NW0, _newEq, _oldEq, AtauMax, BtauMax, x6;
	int sec = strcmp( NAME, "Firm1" ) == 0 ? 0 : 1;
	object *cap = V_EXTS( GRANDPARENT, countryE, capSec );

	_NW0 = VS( PARENT, NW0var[ sec ] ) * 		// deposits of new firms
		   VS( cap, "PPI" ) / VS( cap, "pK0" );

	_oldEq = V( _EqVar[ sec ] );				// firm current equity
	_newEq = _NW0 + V( _DebVar[ sec ] ) - V( _NWvar[ sec ] );
												// new equity required
	WRITE( _DebVar[ sec ], 0 );					// reset debt
	WRITE( _NWvar[ sec ], _NW0 );				// update deposits
	INCR( _EqVar[ sec ], _newEq );				// issue new equity

	if ( stat )
	{
		WRITE( _ownVar[ sec ], 1 );
		INCRS( PARENT, EqExitWVar[ sec ], _oldEq );// shareholder equity
		INCRS( PARENT, EqEntryGvar[ sec ], _newEq );// gov. new equity
		INCRS( PARENT, EqVar[ sec ], - _oldEq + _newEq );
												// change in sectoral equity
		if ( sec == 0 )							// capital-good firm?
		{
			x6 = VS( PARENT, "x6" );			// technological frontier gap
			if ( x6 < 1 )						// don't apply if invalid
			{
				// endow rescued firm with new technology
				CFUNS( PARENT, best_tech, AtauMax, BtauMax );
				WRITE( "_Atau", AtauMax * ( 1 - x6 ) );
				WRITE( "_Btau", BtauMax * ( 1 - x6 ) );
			}
		}
	}
	else
		INCRS( PARENT, EqEntryWvar[ sec ], _newEq );// shareh. new eq.

	return _newEq;
}


// remove firm object and existing hooks in equation 'entry1exit', 'entry2exit'

const char *_BadDebVar[ ] = { "_BadDeb1", "_BadDeb2" },
		   *NWexitGVar[ ] = { "NW1exitG", "NW2exitG" },
		   *NWexitWVar[ ] = { "NW1exitW", "NW2exitW" },
		   *CliBrochObj[ ] = { "Cli", "Broch" };

CFUN_DBL( exit_firm, double *firesAcc )
{
	double  _Eq, fires, liqVal;
	object *bank, *cli;
	int sec = strcmp( NAME, "Firm1" ) == 0 ? 0 : 1,
		_own = V( _ownVar[ sec ] ) ;

	// remove equity from sector total
	_Eq = V( _EqVar[ sec ] );						// firm equity
	INCRS( PARENT, EqVar[ sec ], - _Eq );

	if ( _own == 1 )
		INCRS( PARENT, EqExitGVar[ sec ], _Eq );	// government equity
	else
		INCRS( PARENT, EqExitWVar[ sec ], _Eq );	// shareholder equity

	// account liquidation equity credit of shareholder or bad debt cost of bank
	liqVal = ROUND( V( _NWvar[ sec ] ) - V( _DebVar[ sec ] ), 0, 0.01 );

	// government always repays his firms debts, bad debt only from private ones
	if ( _own == 1 )
		INCRS( PARENT, NWexitGVar[ sec ], liqVal );	// + or -
	else
		if ( liqVal < 0 )						// account bank losses, if any
		{
			bank = HOOK( BANK );				// exiting firm bank
			VS( bank, _BadDebVar[ sec ] );		// ensure reset in t
			INCRS( bank, _BadDebVar[ sec ], - liqVal );// accumulate bank losses
		}
		else
			INCRS( PARENT, NWexitWVar[ sec ], liqVal );// return $

	DELETE( HOOK( BCLIENT ) );					// leave client list of bank

	CYCLE( cli, CliBrochObj[ sec ] )			// leave counterpart list
		DELETE( SHOOKS( cli ) );				// delete from counterpart lists

	if ( sec == 1 )
	{
		WRITE( "_life2cycle", 4 );				// mark as exiting firm

		// fire all workers
		*firesAcc += fires = CFUN( fire_workers, MODE_EXIT, 0, & fires );
		INCR( "_fires2", fires );

		// update firm map before removing LSD object in consumption sector
		EXEC_EXTS( GRANDPARENT, countryE, firm2map, erase,
				   ( int ) V( "_ID2" ) );
		DELETE_EXT( firm2E );
	}

	DELETE( THIS );

	return _Eq;
}
