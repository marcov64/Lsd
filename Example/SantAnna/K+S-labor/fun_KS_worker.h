/******************************************************************************

	WORKER OBJECT EQUATIONS
	-----------------------

	Equations that are specific to the worker objects in the K+S LSD model 
	are coded below.
 
 ******************************************************************************/

/*============================== KEY EQUATIONS ===============================*/

EQUATION( "_appl" )
/*
Number of job applications for firms in the period.
Insert candidate in the corresponding sector 1 and 2 firms' queues.
If 0 < omega/omegaU < 1, use the value to draw the probability to apply
at least for one firm.
*/

k = V( "_employed" );							// employment status

// select the correct parameter values for post-change type of firms
i = 1;											// assume pre-change firm
if ( k == 2 )									// employed in sector 2?
{												// handle post-change
	cur = HOOK( FWRK )->up;						// pointer to employer
	if ( VS( cur, "_postChg" ) )				// employer of post-change type?
	{
		v[1] = VS( PARENT, "omega" );			// # firms to queue for employed
		i = 0;									// fix assumption
	}
}
else
	cur = NULL;

if ( i )										// use special pre-change value
	v[1] = VS( PARENT, "omegaPreChg" );			// # firms to queue for employed

v[3] = k ? v[1] : VS( PARENT, "omegaU" );		// max num. of queues to apply
if ( v[3] == 0 )								// no applications to do?
{
	WRITE( "_discouraged", 0 );					// not discouraged
	END_EQUATION( 0 );							// just quit
}

// process specific search mode
switch ( ( int ) VS( GRANDPARENT, "flagSearchMode" ) )
{
	case 0:										// always search
	default:
		v[4] = V( "_searchProb" ) * v[3];		// effective queues to apply
		break;
	case 1:										// search only if unemployed
		if ( ! k )
			v[4] = V( "_searchProb" ) * v[3];
		else
			v[4] = 0;
		break;
	case 2:										// search if wage below average
		if ( VL( "_w", 1 ) < VLS( CONSECL2, "w2oAvg", 1 ) )
			v[4] = V( "_searchProb" ) * v[3];
		else
			v[4] = 0;
		break;
}

if ( v[4] > 0 && v[4] < 1 )						// handle "fractional" number
	h = ( RND < v[4] ) ? 1 : 0;					// by drawing its probability
else
	h = floor( v[4] );

if ( ! k && h <= 0 )							// unemployed and not searching?
	WRITE( "_discouraged", 1 );					// discouraged
else
	WRITE( "_discouraged", 0 );					// not discouraged

if ( h <= 0 )
	END_EQUATION( 0 );

// apply to each queue, until all are done, skipping repeated firms
firmSeT targetFirms;							// set of target firms
firmSeT::iterator it;							// iterator to firm set
dblVecT *weight = & V_EXTS( GRANDPARENT, country, firm2wgtd );// firms weights
i = 0; 											// number of iterations limiter
j = weight->size( );							// number of operating firms
h = min( h, k != 2 ? j : j - 1 );				// can't look for more than all

// select firms to apply in sector 2
while ( ( int ) targetFirms.size( ) < h - 1 )
{
	// see which firm is in that position for accumulated market share
	// in practice, it draws firms with probability proportional to m.s.
	itd = upper_bound( weight->begin( ), weight->end( ), RND );

	// target firm pointer
	cur1 = V_EXTS( GRANDPARENT, country, firm2ptr[ itd - weight->begin( ) ] );
	
	if ( ( k != 2 || cur1 != cur ) && cur1 != NULL )// don't submit to employer		
		targetFirms.insert( cur1 );				// add firm to targets list
		
	++i;										// count iterations
	if ( i > j || weight->at( 0 ) >= 1 )		// probably too few firms?
		break;									// stop searching
}

// set application as a list item
application applData;
applData.w = v[5] = V( "_wR" );
applData.s = v[6] = V( "_s" );
applData.ws = v[5] / v[6]; 
applData.Te = VL( "_Te", 1 );
applData.wrk = p;

// apply to sector 1 queue
EXEC_EXTS( GRANDPARENT, country, firm1appl, push_back, applData );
											
// insert worker application in the selected firms' queues (if any)
for( i = 1, it = targetFirms.begin( ); it != targetFirms.end( ); ++i, ++it )
	// add application to the job queue of corresponding firm in sector 2
	EXEC_EXTS( ( *it ), firm2, appl, push_back, applData );

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
		v[0] = 1;
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
	END_EQUATION( 1 );							// skills = 1
	
if ( V( "_age" ) == 1 )							// just "born"?
	END_EQUATION( VLS( PARENT, "sTmin", 1 ) );	// get minimum existing skills

switch ( ( int ) V( "_employed" ) )				// employment status
{
	case 0:										// not employed
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
				v[0] = VLS( CAPSECL2, "sT1min", 1 );// minimum skills in sector 1
				break;
		}
		break;
	case 2:										// sector 2
		if ( V( "_Te" ) == 0 )					// just hired in firm?
			v[0] = VLS( HOOK( FWRK )->up, "_sT2min", 1 );// firm minimum skills
		else									// already working, just increase
			v[0] = CURRENT  * ( 1 + VS( PARENT, "tauT" ) );
}

RESULT( max( v[0], VLS( PARENT, "sTmin", 1 ) ) )// minimum skills is current min


EQUATION( "_sV" )
/*
Worker skills in last period, due to technology vintage learning-by-using.
Vintage skills are in the [0,1] range.
*/

i = VS( GRANDPARENT, "flagWorkerLBU" );			// worker-level learning mode
if ( i == 0 || i == 2 )							// no learning-by-vintage mode?
	END_EQUATION( 1 );							// skills = 1

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
Effective wage received.
Adjust employed workers wages according to indexation rules.
*/

h = V( "_employed" );							// employment situation
v[14] = VS( GRANDPARENT, "flagHeterWage" );		// heterogeneous wage mode

// select the correct parameter values for post-change type of firms
i = 1;											// assume pre-change firm
if ( h == 2 )									// employed in sector 2?
{												// handle post-change
	cur = HOOK( FWRK )->up;						// pointer to employer in sec. 2
	
	if ( VS( cur, "_postChg" ) )				// employer of post-change type?
	{
		v[15] = VS( GRANDPARENT, "flagIndexWageChg" );// wage indexation mode
		v[16] = VS( GRANDPARENT, "flagFireRuleChg" );// firm firing rule
		i = 0;									// correct assumption
	}
}

if ( i )										// use pre-change values
{
	v[15] = VS( GRANDPARENT, "flagIndexWage" );	// wage indexation mode
	v[16] = VS( GRANDPARENT, "flagFireRule" );	// firm firing rule
}

if ( h == 0 )									// unemployed?
{	// unemployment wage available if flagGovExp == 2, 3
	if ( ( int ) VS( GRANDPARENT, "flagGovExp" ) > 1 )
		v[0] = VS( PARENT, "wU" );				// use benefit
	else
		v[0] = VS( PARENT, "w0min" );			// minimum subsistence

	END_EQUATION( v[0] );
}
else
{
	if ( v[15] == 0 )							// no wage adjustment?
	{
		v[0] = CURRENT;
		goto end_wage;							// still check for minimum wage
	}

	if ( v[14] == 0 )							// homogeneous wages
		END_EQUATION( VS( PARENT, "wCent" ) );	// single wage centrally defined

	if ( v[15] == 2 && h == 2 )					// homogeneous wages?
		END_EQUATION( VS( cur, "_w2o" ) );		// use current offered wage
	
	v[1] = VS( PARENT, "psi1" );				// inflation adjust. parameter
	v[2] = VS( PARENT, "psi2" );				// general prod. adjust. param.
	v[3] = VS( PARENT, "psi3" );				// unemploym. adjust. parameter
	v[4] = VS( PARENT, "psi4" );				// firm prod. adjust. parameter
	v[5] = VLS( CONSECL2, "dCPIb", 1 );			// inflation variation
	v[6] = VLS( GRANDPARENT, "dAb", 1 );		// general productivity variat.
	v[7] = VLS( PARENT, "dUeB", 1 );			// unemployment variation
	
	if ( h == 1 )								// worker in sector 1?
	{
		k = 4;									// just to silent comp. warning
		v[8] = VLS( CAPSECL2, "dA1b", 1 );		// sector 1 productivity variat.
	}
	else										// sector 2 workers
	{ 	
		k = VS( cur, "_life2cycle" );			// employer status
		if ( k == 0 )							// handle entrants
			v[8] = 0;
		else
			if ( v[14] == 1 )					// how consider productivity?
				v[8] = VLS( cur, "_dA2b", 1 );	// product. variation (firm)
			else
				v[8] = max( VL( "_dQb", 1 ), 0);// delta pot. prod. (worker)
	}
	
	if ( VS( GRANDPARENT, "flagNegProdWage" ) == 1 ) // negative product. floor?
		v[8] = max( 0, v[8] );					// apply limit if enabled

	// make sure total productivity effect is bounded to 1
	if ( ( v[2] + v[4] ) > 1 )
		v[2] = max( 1 - v[4], 0 );				// adjust general prod. effect

	// adjust wage by composite index
	v[9] = 1 + v[1] * v[5] + v[2] * v[6] + v[3] * v[7] + v[4] * v[8];
	v[0] = CURRENT * v[9];
	
	// labor sharing mode? (applicable only in sector 2, for non-entrants)
	if ( h == 2 && v[16] == 1 && k > 0 )
	{
		v[10] = V( "_wfull" ) * v[9];			// pre sharing wage ceiling
		WRITE( "_wfull", v[10] );				// keep wage ceiling updated
		// adjust wage as utilization changes but not over ceiling
		v[0] *= 1 + VS( PARENT, "rho" ) * VS( cur, "_dQ2d" ) / VLS( cur, "_Q2e", 1 );
		v[0] = max( v[0], v[10] );
	}
}

// check for abnormal change
v[11] = VS( PARENT, "wCap" );					// wage cap multiplier
if ( v[11] > 0 )
{
	v[12] = v[0] / CURRENT;						// calculate multiple
	v[13] = v[12] < 1 ? 1 / v[12] : v[12];
	if ( v[13] > v[11] )						// explosive change?
	{
		v[14] = v[0];
		v[0] = v[12] < 1 ? CURRENT / v[11] : CURRENT * v[11];
	}
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

h = V( "_employed" );							// employment situation

if ( ! h )										// unemployed?
	v[0] = max( V( "_wRes" ), V( "_wS" ) );		// yes: base on satisfacing wage
else
	v[0] = V( "_w" ) * ( 1 + VS( PARENT, "epsilon" ) );	// no: base on last wage

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
RESULT( VS( PARENT, "wU" ) )					// unemployed wage (benefit)


EQUATION( "_wS" )
/*
Satisfacing wage
*/

j = VS( PARENT, "Ts" );							// wage memory

if ( j == 0 )									// no memory?
	END_EQUATION( V( "_wRes" ) )

v[0] = 0;
for ( i = 1; i <= j; ++i )
	v[0] += VL( "_w", i );						// sum past wages

RESULT( v[0] / j )


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "_B" )
/*
Bonus received in the period by worker
*/

if ( V( "_employed" ) != 2 )					// unemployed or sector 1?
	END_EQUATION( 0 );							// no bonus

VS( HOOK( FWRK )->up, "_Tax2" );				// ensure bonus is computed

v[1] = VS( HOOK( FWRK )->up, "_W2" );			// total wages paid by firm
v[2] = VS( HOOK( FWRK )->up, "_B2" );			// total bonuses paid by firm

RESULT( v[1] > 0 ? V( "_w" ) * v[2] / v[1] : 0 )// bonus share


EQUATION( "_CQ" )
/*
Cumulated production with current technology
*/
RESULT( CURRENT + V( "_Q" ) )


EQUATION( "_Q" )
/*
Production with current worker skills and vintage
*/

if ( HOOK( VWRK ) == NULL )						// disalloc., unempl. or sec. 1?
    END_EQUATION( 0 );							// no production
	
RESULT( V( "_s" ) * VS( HOOK( VWRK )->up, "_Avint" ) )


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
RESULT( mov_avg_bound( p, "_Q", VS( GRANDPARENT, "mLim" ) ) )


/*============================= DUMMY EQUATIONS ==============================*/

EQUATION_DUMMY( "_Tc", "" )
/*
Number of periods of employment contract (can't be fired before end of contract).
Updated in 'Ls'. 
*/

EQUATION_DUMMY( "_discouraged", "_appl" )
/*
Flag to indicate if worker is discouraged to search for a job in the period.
Updated in '_appl'. 
*/

EQUATION_DUMMY( "_employed", "" )
/*
Flag to indicate if worker is employed at a firm in sector 1 (=1), 
sector 2 (=2) or unemployed (=0).
Updated in 'firing' and 'hiring'. 
*/
