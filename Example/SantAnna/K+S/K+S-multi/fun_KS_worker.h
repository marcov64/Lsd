/******************************************************************************

	WORKER OBJECT EQUATIONS
	-----------------------

	Written by Marcelo C. Pereira, University of Campinas

	Copyright Marcelo C. Pereira
	Distributed under the GNU General Public License

	Equations that are specific to the worker objects in the K+S LSD model
	are coded below.

 ******************************************************************************/

/*============================== KEY EQUATIONS ===============================*/

EQUATION( "_CdBas" )
/*
Desired consumption (in money terms) of basic consumption-goods by worker
Also updates '_SavLux'
*/

// basic comsumption desired
v[0] = min( V( "_In" ), VLS( PARENT, "InLux", 1 ) );

if ( ! V( "_employed" ) )						// unemployed?
{
	v[1] = V( "_SavLux" );						// current savings for luxury

	if ( v[1] > 0 )								// accumulated savings?
	{
		v[0] += v[2] = v[1] / VS( PARENT, "Tlux" );
		INCR( "_SavLux", - v[2] );				// consume part of it
	}
}

RESULT( v[0] )


EQUATION( "_CdLux" )
/*
Desired consumption (in money terms) of luxury consumption-goods by worker
Also updates '_SavLux'
*/

V( "_SavLux" );									// ensure savings are updated

v[0] = 0;										// start buying nothing
v[1] = INCR( "_SavLux", V( "_In" ) - V( "_CdBas" ) );// update luxury savings

if ( v[1] <= 0 )								// no money for luxury?
	WRITE( "_tLux", T );						// savings period not started
else
	if ( T - V( "_tLux" ) >= VS( PARENT, "Tlux") )// time to buy luxury again?
	{
		dblVecT *weight = & V_EXTS( GRANDPARENT, countryE, ind2wgtd );
		goodMapT *bought = & V_EXT( wrkE, buyLux );
		j = weight->size( );					// number of operating industries
		k = VS( PARENT, "TluxLife" );			// life period of luxury good

		for ( i = 0; i < j; ++i )				// iterations limiter
		{
			// see which industry is in the draw position for accumulated compet.
			// drawing industries with probability proportional to competitiv.
			itd = upper_bound( weight->begin( ), weight->end( ), RND );

			// target industry, product ID and price
			cur = V_EXTS( GRANDPARENT, countryE, ind2ptr[ itd - weight->begin( ) ] );
			h = VS( cur, "ID2" );				// industry/product ID
			v[2] = VS( cur, "p2" );				// average price of good

			// found a good which can be possibly bought?
			if ( v[1] < v[2] )
				continue;						// no, keep searching

			// was it not acquired recently?
			goodMapT::iterator it = bought->find( h );
			if ( it == bought->end( ) || ( k > 0 && it->second <= T - k ) )
			{
				// add to or update the already-bought list
				if ( it == bought->end( ) )
					bought->insert( goodPairT( h, ( int ) T ) );
				else
					it->second = ( int ) T;

				v[0] = v[1];					// total to expend

				WRITE( "_tLux", T );			// restart savings period
				WRITE( "_SavLux", 0 );			// clear savings

				buyOrder order;					// add order to industry
				order.bdgt = v[1];
				order.wrk = THIS;
				EXEC_EXTS( cur, ind2E, buyOrd, push_back, order );

				break;
			}
		}
	}

RESULT( v[0] )


EQUATION( "_In" )
/*
Available income in period to worker, net from tax
Government transfers doesn't pay tax
*/

v[1] = VL( "_Bon", 1 );							// past period bonus, if any

if ( V( "_employed" ) == 0 )					// unemployed?
{
	if ( VS( GRANDPARENT, "flagGovExp" ) < 2 )	// work-or-die + subsistence?
		v[2] = VS( PARENT, "w0min" );			// minimum income
	else
		v[2] = VS( PARENT, "wU" );				// unemployment benefit
}
else
{
	v[1] += V( "_w" );							// this period wage
	v[2] = 0;									// no money from government
}

v[0] = VS( GRANDPARENT, "flagTax" ) > 0 ? 		// apply tax, if required
		( 1 - VS( GRANDPARENT, "tr" ) ) * v[1] : v[1];

RESULT( v[1] + v[2] )


EQUATION( "_SavLux" )
/*
Savings accumulated for luxury goods acquisition
Also updated in '_CdLux', '_CdBas'
*/
RESULT( CURRENT * ( 1 + VS( FINSECL2, "rD" ) ) )// update savings with interest


EQUATION( "_appl" )
/*
Number of job applications for firms in the period
Insert candidate in the corresponding sector 1 and 2 firms' queues
If 0 < omega/omegaU < 1, use the value to draw the probability to apply
at least for one firm
*/

k = V( "_employed" );							// employment status

// select the correct parameter values for each type of worker
if ( k == 0 )									// unemployed?
	v[1] = VS( PARENT, "omegaU" );				// max num. of queues to apply
else
{
	cur = PARENTS( HOOK( FWRK ) );				// pointer to employer

	if ( k == 2 )								// employed in sector 2?
		if ( VS( cur, "_post2chg" ) )			// is employer post-change?
			v[1] = VS( PARENT, "omega" );		// # firms to apply
		else
			v[1] = VS( PARENT, "omegaPreChg" );
	else
		v[1] = VS( PARENT, "omegaPreChg" );		// sector 1
}

if ( v[1] == 0 )								// no applications to do?
{
	WRITE( "_discouraged", 0 );					// not discouraged
	END_EQUATION( 0 );							// just quit
}

// process specific search mode
switch ( ( int ) VS( GRANDPARENT, "flagSearchMode" ) )
{
	case 0:										// always search
	default:
		v[2] = V( "_searchProb" ) * v[1];		// effective queues to apply
		break;
	case 1:										// search only if unemployed
		if ( k == 0 )
			v[2] = V( "_searchProb" ) * v[1];
		else
			v[2] = 0;
		break;
	case 2:										// search if wage below average
		if ( k == 0 || VL( "_w", 1 ) < VLS( PARENT, "woAvg", 1 ) )
			v[2] = V( "_searchProb" ) * v[1];
		else
			v[2] = 0;
}

if ( v[2] > 0 && v[2] < 1 )						// handle "fractional" number
	h = ( RND < v[2] ) ? 1 : 0;					// by drawing its probability
else
	h = round( v[2] );

if ( k == 0 && h <= 0 )							// unemployed and not searching?
	WRITE( "_discouraged", 1 );					// discouraged
else
	WRITE( "_discouraged", 0 );					// not discouraged

if ( h <= 0 )
	END_EQUATION( 0 );

// apply to each queue, until all are done, skipping repeated firms
firmSeT targetFirms;							// set of target firms
firmSeT::iterator it;							// iterator to firm set
dblVecT *weight = & V_EXTS( GRANDPARENT, countryE, firm2wgtd );// firms weights
i = 0; 											// number of iterations limiter
j = weight->size( );							// number of operating firms
h = min( h, k != 2 ? j : j - 1 );				// can't look for more than all

// select firms to apply in sector 2
while ( ( int ) targetFirms.size( ) < h - 1 )
{
	// see which firm is in the drawn position for accumulated market share
	// in practice, it draws firms with probability proportional to m.s.
	itd = upper_bound( weight->begin( ), weight->end( ), RND );

	// target firm pointer
	cur1 = V_EXTS( GRANDPARENT, countryE, firm2ptr[ itd - weight->begin( ) ] );

	if ( cur1 != NULL && ( k != 2 || cur1 != cur ) )// don't submit to employer
		targetFirms.insert( cur1 );				// add firm to targets list

	++i;										// count iterations
	if ( i > j || weight->at( 0 ) >= 1 )		// probably too few firms?
		break;									// stop searching
}

// set application as a list item
application applData;
applData.w = V( "_wR" );
applData.s = V( "_s" );
applData.ws = applData.w / applData.s;
applData.Te = VL( "_Te", 1 );
applData.wrk = THIS;

// apply to sector 1 queue
EXEC_EXTS( GRANDPARENT, countryE, firm1appl, push_back, applData );

// insert worker application in the selected firms' queues (if any)
for( i = 1, it = targetFirms.begin( ); it != targetFirms.end( ); ++i, ++it )
	// add application to the job queue of corresponding firm in sector 2
	EXEC_EXTS( ( *it ), firm2E, appl, push_back, applData );

RESULT( i )


EQUATION( "_s" )
/*
Worker compounded skills in last period, due to the worker-level learning from
vintage learning-by-doing (50%) and/or tenure experience (50%), according to
how skills affect productivity (flagWorkerSkProd)
*/

// use correct skills affecting productivity
switch ( ( int ) VS( GRANDPARENT, "flagWorkerSkProd" ) )
{
	case 0:										// skills don't affect product.
	default:
		v[0] = INISKILL;
		break;

	case 1:										// only vintage skills count
		v[0] = V( "_sV" );
		break;

	case 2:										// only tenure skills count
		v[0] = V( "_sT" ) / VLS( PARENT, "sTavg", 1 );// normalized tenure skills
		break;

	case 3:										// both skills count
		v[0] = V( "_sV" ) * V( "_sT" ) / VLS( PARENT, "sTavg", 1 );
		break;
}

RESULT( v[0] )


EQUATION( "_sT" )
/*
Worker skills in last period, due to tenure learning-by-doing.
Tenure skills are >= 1 and unbounded.
*/

i = VS( GRANDPARENT, "flagWorkerLBU" );			// worker-level learning mode
if ( i == 0 || i == 1 )							// no learning-by-tenure mode?
	END_EQUATION( INISKILL );

v[1] = VLS( PARENT, "sTmin", 1 );				// current minimum skills

if ( V( "_age" ) == 1 )							// just "born"?
	END_EQUATION( v[1] );						// get minimum existing skills

switch ( ( int ) V( "_employed" ) )				// employment status
{
	case 0:										// not employed
	default:
		if ( VS( PARENT, "Gamma" ) > RND )		// under training this period?
			v[0] = CURRENT * ( 1 + VS( PARENT, "tauG" ) );// training increase
		else									// unemployment-decreased skills
			v[0] = CURRENT / ( 1 + VS( PARENT, "tauU" ) );
		break;
	case 1:										// sector 1
		switch ( ( int ) VS( GRANDPARENT, "flagLearn1" ) )
		{
			case 0:
			default:
				v[0] = CURRENT;					// keep skills
				break;
			case 1:								// decrease skills as unemployed
				v[0] = CURRENT / ( 1 + VS( PARENT, "tauU" ) );
				break;
			case 2:								// increase skills
				v[0] = CURRENT * ( 1 + VS( PARENT, "tauT" ) );
				break;
			case 3:
				v[0] = v[1];					// minimum skills
				break;
		}
		break;
	case 2:										// sector 2
		if ( V( "_Te" ) == 0 )					// just hired in firm?
			v[0] = VLS( PARENTS( HOOK( FWRK ) ), "_sT2min", 1 );// firm minimum skills
		else									// already working, just increase
			v[0] = CURRENT  * ( 1 + VS( PARENT, "tauT" ) );
}

RESULT( max( v[0], v[1] ) )						// minimum skills is current min


EQUATION( "_sV" )
/*
Worker skills in last period, due to technology vintage learning-by-using.
Vintage skills are in the [0,1] range.
*/

i = VS( GRANDPARENT, "flagWorkerLBU" );			// worker-level learning mode
if ( i == 0 || i == 2 )							// no learning-by-vintage mode?
	END_EQUATION( INISKILL );

if ( HOOK( FWRK ) == NULL || V( "_age" ) == 1 )	// discard unempl./sect.1
	END_EQUATION( VS( PARENT, "sigma" ) );		// skills = public skills

RESULT( CURRENT + VS( PARENT, "sigma" ) * ( VL( "_Q", 1 ) / VL( "_CQ", 1 ) ) *
		CURRENT * ( 1 - CURRENT ) )


EQUATION( "_searchProb" )
/*
Probability of searching for job in period (individual)
*/

// handle discouragement mode
switch ( ( int ) VS( GRANDPARENT, "flagSearchDisc" ) )
{
	case 0:										// always search
	default:
		v[0] = 1;
		break;

	case 1:										// global search
		v[0] = VS( PARENT, "searchProb" );
		break;

	case 2:										// individual search
		v[1] = VS( PARENT, "lambda" );
		v[2] = V( "_Tu" );						// periods unemployed
		v[0] = v[1] * exp( - v[1] * v[2] );		// compute individual prob.
		break;
}

RESULT( min( v[0], 1 ) )


EQUATION( "_w" )
/*
Wage received by worker (excluding bonus and unemployment benefit)
*/

double dAfirm, lifeCycle;
int indexWage, fireRule;

int employed = V( "_employed" );				// employment situation

if ( employed == 0 )							// unemployed?
	END_EQUATION( 0 );

int heterWage = VS( GRANDPARENT, "flagHeterWage" );// heterogeneous wage mode

if ( heterWage == 0 )							// homogeneous wages
	END_EQUATION( VS( PARENT, "wCent" ) );		// single wage centrally defined

// select the correct parameter values for post-change type of firms
i = true;										// assume pre-change firm
if ( employed == 2 )							// employed in sector 2?
{												// handle post-change
	cur = PARENTS( HOOK( FWRK ) );				// pointer to employer in sec. 2

	if ( VS( cur, "_post2chg" ) )				// employer of post-change type?
	{
		indexWage = VS( GRANDPARENT, "flagIndexWageChg" );// wage indexation mode
		fireRule = VS( GRANDPARENT, "flagFireRuleChg" );// firm firing rule
		i = false;								// correct assumption
	}
}

if ( i )										// use pre-change values
{
	indexWage = VS( GRANDPARENT, "flagIndexWage" );// wage indexation mode
	fireRule = VS( GRANDPARENT, "flagFireRule" );// firm firing rule
}

double psi1 = VS( PARENT, "psi1" );				// inflation adjust. param.
double psi2 = VS( PARENT, "psi2" );				// general prod. adjust. param.
double psi3 = VS( PARENT, "psi3" );				// unemployment adjust. param.
double psi4 = VS( PARENT, "psi4" );				// firm prod. adjust. parameter
double piT = VS( FINSECL2, "piT" );				// expected inflation
double dCPIb = VLS( GRANDPARENT, "dCPIb", 1 );	// current inflation
double dAb = VLS( GRANDPARENT, "dAb", 1 );		// general productivity variat.
double dUeB = VLS( PARENT, "dUeB", 1 );			// current unemployment change

if ( indexWage == 0 )							// no wage adjustment?
{
	v[0] = CURRENT;
	goto end_wage;								// still check for minimum wage
}

if ( indexWage == 2 && employed == 2 )			// homogeneous wages?
{
	v[0] = VS( cur, "_w2o" );					// use current offered wage
	goto end_wage;								// still check for minimum wage
}

if ( employed == 1 )							// worker in sector 1?
{
	lifeCycle = 3;								// always incumbent
	dAfirm = AVELS( GRANDPARENT, "dB1b", 1 );	// sector 1 productivity var.
}
else											// sector 2 workers
{
	lifeCycle = VS( cur, "_life2cycle" );		// employer status
	if ( lifeCycle < 2 )						// handle entrants
		dAfirm = dAb;
	else
		if ( heterWage == 1 )					// how consider productivity?
			dAfirm = VLS( cur, "_dA2b", 1 );	// product. variation (firm)
		else
			dAfirm = max( VL( "_dQb", 1 ), 0 );	// delta pot. prod. (worker)
}

// make sure total productivity effect is bounded to 1
if ( ( psi2 + psi4 ) > 1 )
	psi2 = max( 1 - psi4, 0 );					// adjust general prod. effect

// adjust wage by composite index
v[1] = 1 + piT + psi1 * ( dCPIb - piT ) + psi2 * dAb + psi3 * dUeB + psi4 * dAfirm;
v[0] = CURRENT * v[1];

// labor sharing mode? (applicable only in sector 2, for non-entrants)
if ( employed == 2 && fireRule == 1 && lifeCycle > 0 )
{
	v[2] = V( "_wfull" ) * v[1];				// pre sharing wage ceiling
	WRITE( "_wfull", v[2] );					// keep wage ceiling updated

	// adjust wage as utilization changes but not over ceiling
	v[0] *= 1 + VS( PARENT, "rho" ) * VS( cur, "_dQ2d" ) / VLS( cur, "_Q2e", 1 );
	v[0] = max( v[0], v[2] );
}

// check for and bound abnormal change
v[3] = VS( PARENT, "wCap" );					// wage cap multiplier
if ( v[3] > 0 )
{
	v[4] = v[0] / CURRENT;						// calculate multiple
	v[5] = v[4] < 1 ? 1 / v[4] : v[4];			// handle decreases/increases
	if ( v[5] > v[3] )							// explosive change?
		v[0] = v[4] < 1 ? CURRENT / v[3] : CURRENT * v[3];
}

end_wage:
// under minimum wage?
v[0] = max( v[0], VS( PARENT, "wMinPol" ) );	// adjust if necessary

RESULT( v[0] )


EQUATION( "_wR" )
/*
Requested/required wage to accept job offer
*/

if ( VS( GRANDPARENT, "flagHeterWage" ) == 0 )	// centralized wage setting?
	END_EQUATION( VS( PARENT, "wCent" ) );		// single wage centrally defined

if ( ! V( "_employed" ) )						// unemployed?
	v[0] = max( V( "_wRes" ), V( "_wS" ) );		// yes: base on satisfacing wage
else
	v[0] = V( "_w" ) * ( 1 + VS( PARENT, "epsilon" ) );// no: base on last wage

v[0] = max( v[0], VS( PARENT, "wMinPol" ) );	// adjust to minimum if needed

// check for abnormal change
v[1] = VS( PARENT, "wCap" );					// wage cap multiplier
if ( CURRENT > 0 && v[1] > 0 )
{
	v[2] = v[0] / CURRENT;						// calculate multiple
	v[3] = v[2] < 1 ? 1 / v[2] : v[2];
	if ( v[3] > v[1] )							// explosive change?
	{
		v[4] = v[0];
		v[0] = v[2] < 1 ? CURRENT / v[1] : CURRENT * v[1];
	}
}

RESULT( v[0] )


EQUATION( "_wRes" )
/*
Reservation wage
*/
RESULT( VS( GRANDPARENT, "flagGovExp" ) < 2 ? 	// minimum assured income
		VS( PARENT, "w0min" ) : VS( PARENT, "wU" ) )


EQUATION( "_wS" )
/*
Satisfacing wage
*/

j = VS( PARENT, "Ts" );							// wage memory
v[1] = V( "_wRes" );							// reservation wage (min income)

for ( v[0] = 0, i = 1; i <= j; ++i )
{
	v[2] = VL( "_w", i );
	v[0] += ( v[2] == 0 ) ? v[1] : v[2];		// sum past wages
}

RESULT( j > 0 ? v[0] / j : v[1] )


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "_Bon" )
/*
Bonus received in the period by worker
*/

if ( V( "_employed" ) != 2 )					// unemployed or sector 1?
	END_EQUATION( 0 );							// no bonus

VS( PARENTS( HOOK( FWRK ) ), "_Tax2" );			// ensure bonus is computed

v[1] = VS( PARENTS( HOOK( FWRK ) ), "_W2" );	// total wages paid by firm
v[2] = VS( PARENTS( HOOK( FWRK ) ), "_Bon2" );	// total bonuses paid by firm

RESULT( v[1] > 0 ? V( "_w" ) * v[2] / v[1] : 0 )// bonus share


EQUATION( "_CQ" )
/*
Cumulated production with current technology
*/
RESULT( CURRENT + V( "_Q" ) )


EQUATION( "_Q" )
/*
Production (in output units) with current worker skills and vintage
*/
RESULT( HOOK( VWRK ) != NULL ? V( "_s" ) *
		VS( PARENTS( GRANDPARENTS( HOOK( VWRK ) ) ), "m2" ) *
		VS( PARENTS( HOOK( VWRK ) ), "_Avint" ) /
		VS( PARENTS( GRANDPARENTS( HOOK( VWRK ) ) ), "k2" ) : 0 )


EQUATION( "_Te" )
/*
Number of periods of employment in current firm (0 if just hired, not calc. here)
*/
RESULT( V( "_employed" ) ? CURRENT + 1 : 0 )


EQUATION( "_Tu" )
/*
Number of periods of unemployment (0 if employed)
*/

if ( ! V( "_employed" ) )						// unemployed?
	v[0] = CURRENT + 1;							// yes: one period more
else
	v[0] = 0;									// no: zero periods

RESULT( v[0] )


EQUATION( "_age" )
/*
Worker working age. Accumulates age and make worker to reborn after retirement.
New "born" worker goes immediately to the labor market under minimum skills.
*/

i = VS( PARENT, "Tr" );							// retirement age
if ( i == 0 || CURRENT < i )					// lives until retirement if any
	v[0] = CURRENT + 1;							// simply gets older
else
	v[0] = 1;									// new age is 1 ("reborn")

RESULT( v[0] )


EQUATION( "_dQb" )
/*
Notional production (bounded) rate of change of worker
Used for wages adjustment only
*/
RESULT( CFUN( mov_avg_bound, "_Q", VS( GRANDPARENT, "mLim" ) ) )


/*============================= DUMMY EQUATIONS ==============================*/

EQUATION_DUMMY( "_Tc", "Ls" )
/*
Number of periods of employment contract (can't be fired before end of contract)
Updated in 'Ls'
*/

EQUATION_DUMMY( "_discouraged", "_appl" )
/*
Flag to indicate if worker is discouraged to search for a job in the period
Updated in '_appl'
*/

EQUATION_DUMMY( "_employed", "" )
/*
Flag to indicate if worker is employed at a firm in sector 1 (=1),
sector 2 (=2) or unemployed (=0)
Updated in 'firing' and 'hiring'
*/
