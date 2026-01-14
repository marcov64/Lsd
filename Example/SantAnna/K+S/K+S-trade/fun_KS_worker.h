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

EQUATION( "_Bon" )
/*
Bonus received by worker
*/

VS( CONSECL2, "hires2" );						// ensure hiring done

if ( V( "_employed" ) != 2 )					// unemployed or sector 1?
	END_EQUATION( 0 );							// no bonus

v[1] = VS( PARENTS( HOOK( FWRK ) ), "_W2" );	// total wages paid by firm
v[2] = VS( PARENTS( HOOK( FWRK ) ), "_Bon2" );	// total bonuses paid by firm

RESULT( v[1] > 0 ? v[2] * V( "_w" ) / v[1] : 0 )


EQUATION( "_Cd" )
/*
Effective desired consumption (in money terms) of worker
*/

v[1] = V( "_In" ) - V( "_Tax" );				// nominal available income

switch ( ( int ) VS( COUNTRL2, "flagCons" ) )
{
	case 0:
	default:
		v[0] = v[1];							// use just current income
		break;

	case 1:
		v[0] = v[1] + VL( "_SavAcc", 1 );		// use all available wealth
		break;

	case 2:										// use savings up to Crec
		v[2] = VL( "_SavAcc", 1 );				// accumulated savings
		v[3] = v[1] * VS( PARENT, "Crec" );		// max savings usage limit

		if ( v[2] < v[3] )						// fit in limit?
			v[0] = v[1] + v[2];					// yes: use all savings
		else
			v[0] = v[1] + v[3];					// no: spend the limit

		break;

	case 3:										// use behavioral rule to decide
		v[0] = min( V( "_CdReal" ) * VS( PARENT, "CPIexp" ),
					v[1] + VL( "_SavAcc", 1 ) );
}

RESULT( v[0] )


EQUATION( "_CdReal" )
/*
Desired real consumption (in good unit terms) of worker
*/

if ( VS( COUNTRL2, "flagCons" ) < 3 )
	END_EQUATION( 0 );

v[1] = V( "_CrefReal" );						// real consumption reference

RESULT( max( VS( PARENT, "alphaC" ) * ( V( "_In" ) - V( "_Tax" ) ) /
			 VS( PARENT, "CPIexp" ) + VS( PARENT, "betaC" ) * v[1],
			 VS( PARENT, "gammaC" ) * v[1] ) )


EQUATION( "_CrefReal" )
/*
Real consumption reference (in good unit terms) of worker
*/

if ( VS( COUNTRL2, "flagCons" ) < 3 )
	END_EQUATION( 0 );

v[1] = VS( PARENT, "omegaC" );					// weight of time on expectation
k = VS( PARENT, "Texp" );						// time horizon of expectation

for ( v[2] = v[3] = 0, h = 1; h <= k; ++h )
{
	v[3] += v[5] = pow( v[1], h - 1);
	v[2] += v[5] * VL( "_CdReal", h );
}

RESULT( v[2] / v[3] )


EQUATION( "_Gtrf" )
/*
Government transfers received by worker
*/

VS( CONSECL2, "hires2" );						// ensure hiring done

if ( V( "_employed" ) == 0 )					// unemployed?
	v[0] = VS( PARENT, "wU" );					// unemployment income
else
	v[0] = 0;

// other transfers (education & training teacher salaries) uniformly distributed
v[0] += ( VS( PARENT, "Ged" ) + VS( PARENT, "Gtrain" ) ) / VS( PARENT, "Ls" );

RESULT( v[0] )


EQUATION( "_Q" )
/*
Production with worker education, current skills, and vintage in use
*/
RESULT( HOOK( VWRK ) != NULL ?					// disalloc., unempl. or sec. 1?
		V( "_edS" ) * VS( PARENTS( HOOK( VWRK ) ), "__Avint" ) : 0 )


EQUATION( "_age" )
/*
Worker working age
Accumulates age and make worker to reborn after retirement, with new education
Updates '_ed', "_cat"
*/

i = VS( PARENT, "Tr" );							// retirement age
if ( i == 0 || CURRENT < i )					// lives until retirement if any
	v[0] = CURRENT + 1;							// simply gets older
else
{
	v[0] = 1;									// new age is 1 ("reborn")
	CFUN( set_education );						// draw new education
}

RESULT( v[0] )


EQUATION( "_appl" )
/*
Number of job applications for firms in the period
Insert candidate in the corresponding sector 1 and 2 firms' queues.
If 0 < omega/omegaU < 1, use the value to draw the probability to apply
at least for one firm
*/

VS( CONSECL2, "fires2" );						// ensure firing done

if ( V( "_emig") )								// emigrating?
	END_EQUATION( 0 );

k = V( "_employed" );							// employment status

// select the correct parameter values for post-change type of firms
i = 1;											// assume pre-change firm
if ( k == 2 )									// employed in sector 2?
{												// handle post-change
	cur = PARENTS( HOOK( FWRK ) );				// pointer to employer
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
switch ( ( int ) VS( COUNTRL2, "flagSearchMode" ) )
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
		if ( VL( "_w", 1 ) < VLS( CONSECL2, w2oAvgVar[ ( int ) V( "_cat" ) - 1 ],
								  1 ) )
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
dblVecT *weight = & V_EXTS( COUNTRL2, countryE, firm2wgtd );// firms weights
i = 0;											// number of iterations limiter
j = weight->size( );							// number of operating firms
h = min( h, k != 2 ? j : j - 1 );				// can't look for more than all

// select firms to apply in sector 2
while ( ( int ) targetFirms.size( ) < h - 1 )
{
	// see which firm is in that position for accumulated market share
	// in practice, it draws firms with probability proportional to m.s.
	auto itd = upper_bound( weight->begin( ), weight->end( ), RND );

	// target firm pointer
	cur1 = V_EXTS( COUNTRL2, countryE, firm2ptr[ itd - weight->begin( ) ] );

	if ( ( k != 2 || cur1 != cur ) && cur1 != NULL )// don't submit to employer
		targetFirms.insert( cur1 );				// add firm to targets list

	++i;										// count iterations
	if ( i > j || weight->at( 0 ) >= 1 )		// probably too few firms?
		break;									// stop searching
}

// set application as a list item
application applData;
applData.w = v[5] = V( "_wR" );
applData.edS = v[6] = V( "_edS" );
applData.wEdS = v[5] / v[6];
applData.Te = VL( "_Te", 1 );
applData.cat = V( "_cat" );
applData.wrk = THIS;

// block access to firm2woX from other parallel threads
mtxLckT lock( V_EXTS( COUNTRL2, countryE, firmApplMtx ) );

// apply to sector 1 queue
EXEC_EXTS( COUNTRL2, countryE, firm1appl, push_back, applData );

// insert worker application in the selected firms' queues (if any)
for( i = 1, it = targetFirms.begin( ); it != targetFirms.end( ); ++i, ++it )
	// add application to the job queue of corresponding firm in sector 2
	EXEC_EXTS( ( *it ), firm2E, appl, push_back, applData );

RESULT( i )


EQUATION( "_edS" )
/*
Worker compounded education and skills effect on productivity
*/

if ( VS( COUNTRL2, "flagEduc" ) == 0 )
	END_EQUATION( V( "_s" ) );

v[1] = V( "_ed" );

if ( v[1] == 0 )
	v[0] = V( "_s" );
else
{
	v[2] = VS( PARENT, "alphaEd" );
	v[0] = pow( v[1] / ( 16 * v[2] / ( v[2] + VS( PARENT, "betaEd" ) ) ),
				VS( PARENT, "tauEd" ) ) * V( "_s" );
}

RESULT( v[0] )


EQUATION( "_s" )
/*
Worker compounded skills in last period, due to the worker-level learning from
vintage learning-by-doing (50%) and/or tenure experience (50%), according to
how skills affect productivity (flagWorkerSkProd)
*/

// use correct skills affecting productivity
switch ( ( int ) VS( COUNTRL2, "flagWorkerSkProd" ) )
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
Worker skills in last period, due to tenure learning-by-doing
Tenure skills are >= INISKILL and unbounded
*/

if ( VS( COUNTRL2, "flagWorkerLBU" ) <= 1 )		// no learning-by-tenure mode?
	END_EQUATION( INISKILL );

if ( V( "_age" ) == 1 )							// just "born"?
	END_EQUATION( VLS( PARENT, "sTmin", 1 ) );	// get minimum existing skills

switch ( ( int ) V( "_employed" ) )				// employment status
{
	default:
	case 0:										// not employed
		if ( VS( PARENT, "Gamma" ) > RND )		// under training this period?
			v[0] = CURRENT * ( 1 + VS( PARENT, "tauG" ) );// training increase
		else									// unemployment-decreased skills
			v[0] = CURRENT / ( 1 + VS( PARENT, "tauU" ) );
		break;
	case 1:										// sector 1
		switch ( ( int ) VS( COUNTRL2, "flagLearn1" ) )
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
			v[0] = VLS( PARENTS( HOOK( FWRK ) ), "_sT2min", 1 );// firm minimum skills
		else									// already working, just increase
			v[0] = CURRENT	* ( 1 + VS( PARENT, "tauT" ) );
}

RESULT( max( v[0], VLS( PARENT, "sTmin", 1 ) ) )// minimum skills is current min


EQUATION( "_sV" )
/*
Worker skills in last period, due to technology vintage learning-by-using
Vintage skills are in the [0,INISKILL] range
*/

i = VS( COUNTRL2, "flagWorkerLBU" );			// worker-level learning mode
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
switch ( ( int ) VS( COUNTRL2, "flagSearchDisc" ) )
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
Effective wage received
Adjust employed workers wages according to indexation rules
*/

h = V( "_employed" );							// employment situation

if ( h == 0 )									// unemployed?
	END_EQUATION( 0 );

v[14] = VS( COUNTRL2, "flagHeterWage" );		// heterogeneous wage mode

if ( v[14] == 0 )								// homogeneous wages
	END_EQUATION( VS( PARENT, "wCent" ) );		// single wage centrally defined

// select the correct parameter values for post-change type of firms
i = 1;											// assume pre-change firm
if ( h == 2 )									// employed in sector 2?
{												// handle post-change
	cur = PARENTS( HOOK( FWRK ) );				// pointer to employer in sec. 2

	if ( VS( cur, "_postChg" ) )				// employer of post-change type?
	{
		v[15] = VS( COUNTRL2, "flagIndexWageChg" );// wage indexation mode
		v[16] = VS( COUNTRL2, "flagFireRuleChg" );// firm firing rule
		i = 0;									// correct assumption
	}
}

if ( i )										// use pre-change values
{
	v[15] = VS( COUNTRL2, "flagIndexWage" );	// wage indexation mode
	v[16] = VS( COUNTRL2, "flagFireRule" );		// firm firing rule
}

if ( v[15] == 0 )								// no wage adjustment?
{
	v[0] = CURRENT;
	goto end_wage;								// still check for minimum wage
}

if ( v[15] == 2 && h == 2 )						// homogeneous wages?
	END_EQUATION( VS( cur, _w2oVar[ ( int ) V( "_cat" ) - 1 ] ) );
												// use current offered wage
v[1] = VS( PARENT, "psi1" );					// inflation adjust. parameter
v[2] = VS( PARENT, "psi2" );					// general prod. adjust. param.
v[3] = VS( PARENT, "psi3" );					// unemploym. adjust. parameter
v[4] = VS( PARENT, "psi4" );					// firm prod. adjust. parameter
v[17] = VS( FINSECL2, "piT" );					// expected inflation
v[5] = VLS( CONSECL2, "dCPIb", 1 );				// current inflation
v[6] = VLS( COUNTRL2, "dAb", 1 );				// general productivity variat.
v[7] = VLS( PARENT, "dUeB", 1 );				// unemployment variation

if ( h == 1 )									// worker in sector 1?
{
	k = 4;										// just to silent comp. warning
	v[8] = VLS( CAPSECL2, "dA1b", 1 );			// sector 1 productivity variat.
}
else											// sector 2 workers
{
	k = VS( cur, "_life2cycle" );				// employer status
	if ( k == 0 )								// handle entrants
		v[8] = 0;
	else
		if ( v[14] == 1 )						// how consider productivity?
			v[8] = VLS( cur, "_dA2b", 1 );		// product. variation (firm)
		else
			v[8] = max( VL( "_dQb", 1 ), 0);	// delta pot. prod. (worker)
}

// make sure total productivity effect is bounded to 1
if ( ( v[2] + v[4] ) > 1 )
	v[2] = max( 1 - v[4], 0 );					// adjust general prod. effect

// adjust wage by composite index
v[9] = 1 + v[17] + v[1] * ( v[5] - v[17] ) + v[2] * v[6] + v[3] * v[7] + v[4] * v[8];
v[0] = CURRENT * v[9];

// labor sharing mode? (applicable only in sector 2, for non-entrants)
if ( h == 2 && v[16] == 1 && k > 0 )
{
	v[10] = V( "_wfull" ) * v[9];				// pre sharing wage ceiling
	WRITE( "_wfull", v[10] );					// keep wage ceiling updated

	// adjust wage as utilization changes but not over ceiling
	v[0] *= 1 + VS( PARENT, "rho" ) * VS( cur, "_dQ2d" ) / VLS( cur, "_Q2e", 1 );
	v[0] = max( v[0], v[10] );
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

if ( VS( COUNTRL2, "flagHeterWage" ) == 0 )		// centralized wage setting?
	END_EQUATION( VS( PARENT, "wCent" ) );		// single wage centrally defined

h = V( "_employed" );							// employment situation

if ( ! h )										// unemployed?
	v[0] = V( "_wS" );							// yes: base on satisfacing wage
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


EQUATION( "_wS" )
/*
Satisfacing wage
*/

j = VS( PARENT, "Ts" );							// wage memory
v[1] = VS( PARENT, "wU" );						// unemployment subsidy

if ( j == 0 )									// no memory?
	END_EQUATION( v[1] );

for ( v[0] = 0, i = 1; i <= j; ++i )
	if ( T - i >= 0 )							// just go to t=0
	{
		v[2] = VL( "_w", i );					// past wage

		if ( v[2] > 0 )
			v[0] += v[2];						// sum past wages
		else
			v[0] += VLS( PARENT, "wU", i );		// or unemployment benefit
	}
	else
		break;

RESULT( max( v[1], v[0] / ( i - 1 ) ) )


/*============================ SUPPORT EQUATIONS =============================*/

EQUATION( "_CQ" )
/*
Cumulated production with current technology
*/
RESULT( CURRENT + V( "_Q" ) )


EQUATION( "_Div" )
/*
Dividends received by worker
*/
v[1] = VS( PARENT, "EqW" );
RESULT( v[1] > 0 ? ( VS( CAPSECL2, "Div1w" ) + VS( CONSECL2, "Div2w" ) +
					 VS( FINSECL2, "DivB" ) ) * V( "_Eq" ) / v[1] : 0 )


EQUATION( "_In" )
/*
Total (gross) income of worker
*/
v[1] = V( "_Gtrf" );							// ensure hires are done first
RESULT( V( "_w" ) + VL( "_Bon", 1 ) + VL( "_Div", 1 ) + VL( "_NWexit", 1 ) +
		v[1] + V( "_emigTrf" ) + V( "_iD" ) )


EQUATION( "_iD" )
/*
Interest received from savings by worker
*/
RESULT( VLS( FINSECL2, "rD", 1 ) * VL( "_SavAcc", 1 ) )


EQUATION( "_Sav" )
/*
Worker savings in period
*/
VS( CONSECL2, "D2" );							// ensure shortages allocated
RESULT( V( "_In" ) - V( "_Tax" ) - V( "_C" ) )


EQUATION( "_SavAcc" )
/*
Worker accumulated savings at the end of the period
Also updated in 'entryExit'
*/
RESULT( CURRENT - VL( "_EqEntry", 1 ) + V( "_Sav" ) )


EQUATION( "_Tax" )
/*
Income tax paid by worker
*/
v[1] = VS( COUNTRL2, "sIn" );					// ensure shock is evaluated
RESULT( max( ( VS( COUNTRL2, "trIn" ) - v[1] ) * V( "_In" ), 0 ) )


EQUATION( "_Te" )
/*
Number of periods of employment in current firm (0 if just hired, not calc. here)
*/
RESULT( V( "_employed" ) ? CURRENT + 1 : 0 )


EQUATION( "_Tu" )
/*
Number of periods of unemployment (0 if employed)
*/

VS( CONSECL2, "hires2" );						// ensure hiring done

if ( ! V( "_employed" ) )						// unemployed?
	v[0] = CURRENT + 1;							// yes: one period more
else
	v[0] = 0;									// no: zero periods

RESULT( v[0] )


EQUATION( "_dQb" )
/*
Notional production (bounded) rate of change of worker
Used for wages adjustment only
*/
RESULT( CFUN( mov_avg_bound, "_Q", VS( COUNTRL2, "mLim" ),
		VS( COUNTRL2, "mPer" ) ) )


/*============================= DUMMY EQUATIONS ==============================*/

EQUATION_DUMMY( "_C", "" )
/*
Effective consumption (in money terms) of worker
Updated in 'D2'
*/

EQUATION_DUMMY( "_Eq", "" )
/*
Equity from firms hold by worker
Updated in 'initCountry', 'EqAlloc'
*/

EQUATION_DUMMY( "_EqEntry", "" )
/*
Cost of new equity from firm entries to worker
Updated in 'EqAlloc'
*/

EQUATION_DUMMY( "_NWexit", "" )
/*
Residual net worth from equity hold on exiting firms by worker
Updated in 'EqAlloc'
*/

EQUATION_DUMMY( "_Tc", "" )
/*
Number of periods of employment contract (can't be fired before end of contract)
Updated in 'Ls'
*/

EQUATION_DUMMY( "_discouraged", "_appl" )
/*
Flag to indicate if worker is discouraged to search for a job in the period
Updated in '_appl'
*/

EQUATION_DUMMY( "_emig", "" )
/*
Flag to indicate if worker is emigrating
Updated in 'Ls'
*/

EQUATION_DUMMY( "_emigTrf", "" )
/*
Transfers received from migrants leaving the country
Updated in 'emig'
*/

EQUATION_DUMMY( "_employed", "" )
/*
Flag to indicate if worker is employed at a firm in sector 1 (=1),
sector 2 (=2) or unemployed (=0)
Updated in 'fires1', 'fires2', 'hires1', 'hires2'
*/
