/******************************************************************************

	STATISTICS EQUATIONS
	--------------------

	Written by Marcelo C. Pereira, University of Campinas

	Copyright Marcelo C. Pereira
	Distributed under the GNU General Public License

	Equations that are not required for the model to run but may produce
	useful country- or sector-level statistics for analysis.

 ******************************************************************************/

/*========================= COUNTRY-LEVEL STATS ==============================*/

EQUATION( "CI" )
/*
Total canceled investment (in machine-number terms)
*/
RESULT( SUMS( GRANDPARENT, "CI2" ) )


EQUATION( "Cd" )
/*
Nominal (in money terms) desired aggregated consumption
Doesn't include desired government demand 'Gd'
*/
RESULT( VS( GRANDPARENT, "CdBas" ) + VS( GRANDPARENT, "CdLux" ) )


EQUATION( "DebF" )
/*
Total debt of firms
*/
RESULT( SUMS( GRANDPARENT, "Deb1" ) + SUMS( GRANDPARENT, "Deb2" ) )


EQUATION( "DebGDP" )
/*
Government debt on GDP ratio
*/
RESULT( VS( GRANDPARENT, "Deb" ) / VS( GRANDPARENT, "GDPnom" ) )


EQUATION( "DefGDP" )
/*
Government deficit on GDP ratio
*/
RESULT( VS( GRANDPARENT, "Def" ) / VS( GRANDPARENT, "GDPnom" ) )


EQUATION( "EId" )
/*
Aggregated desired expansion investment (in machine-number terms)
*/
RESULT( SUMS( GRANDPARENT, "EI2d" ) )


EQUATION( "GDPdefl" )
/*
GDP (complexity-adjusted) price deflator
*/
RESULT( VS( GRANDPARENT, "GDPnom" ) / VS( GRANDPARENT, "GDPreal" ) )


EQUATION( "GDI" )
/*
Gross domestic income (in money terms)
*/
RESULT( VS( LABSUPL2, "W" ) + VS( LABSUPL2, "Bon" ) + V( "Pi" ) +
		VS( GRANDPARENT, "Div" ) )


EQUATION( "Kd" )
/*
Total desired capital stock (in machine-number terms)
*/
RESULT( SUMS( GRANDPARENT, "K2d" ) )


EQUATION( "Kinst" )
/*
Total capital installed (in machine-number terms)
Should be equal to 'K', used to update number of machines per generation
Updates '_nG'
*/

CYCLES( SEARCHS( GRANDPARENT, "Capital" ), cur, "T1" )
	WRITES( cur, "_nG", 0 );

v[0] = 0;
CYCLES( GRANDPARENT, cur, "Consumption" )
	CYCLES( cur, cur1, "Firm2" )
		CYCLES( cur1, cur2, "Vint" )
		{
			v[0] += v[1] = VS( cur2, "_nVint" );
			INCRS( HOOKS( cur2, TGEN ), "_nG", v[1] );
		}

RESULT( v[0] )


EQUATION( "NW" )
/*
Total net wealth (free cash)
*/
RESULT( SUMS( GRANDPARENT, "NW1" ) + SUMS( GRANDPARENT, "NW2" ) +
		VS( FINSECL2, "NWb" ) )


EQUATION( "Nnom" )
/*
Total inventories (in money terms)
*/
RESULT( WHTAVES( GRANDPARENT, "N2", "p2" ) )	// sum of the product


EQUATION( "Pi" )
/*
Total profits
*/
RESULT( SUMS( GRANDPARENT, "Pi1" ) + SUMS( GRANDPARENT, "Pi2" ) +
		VS( FINSECL2, "PiB" ) )


EQUATION( "S" )
/*
Total sales (in money terms)
*/
RESULT( SUMS( GRANDPARENT, "S1" ) + SUMS( GRANDPARENT, "S2" ) )


EQUATION( "SId" )
/*
Aggregated desired substitution investment (in machine-number terms)
*/
RESULT( SUMS( GRANDPARENT, "SI2d" ) )


EQUATION( "TC" )
/*
Total credit supply provided by banks to firms.
Negative value (-1) means unlimited credit.
*/
RESULT( VS( GRANDPARENT, "flagCreditRule" ) == 1 ? SUMS( FINSECL2, "_TC" ) : -1 )


EQUATION( "dA" )
/*
Overall labor productivity growth rate
*/
v[1] = VLS( GRANDPARENT, "A", 1 );
RESULT( v[1] > 0 ? VS( GRANDPARENT, "A" ) / v[1] - 1 : 0 )


EQUATION( "dCPI" )
/*
Consumer price index inflation (change) rate
*/
RESULT( VS( GRANDPARENT, "CPI" ) / VLS( GRANDPARENT, "CPI", 1 ) - 1 )


EQUATION( "dGDPnom" )
/*
Nominal gross domestic product (log) growth rate
*/
RESULT( T > 1 ? log( VS( GRANDPARENT, "GDPnom" ) ) -
				log( VLS( GRANDPARENT, "GDPnom", 1 ) ) : 0 )


EQUATION( "iNetGDP" )
/*
Net interest expenses (income) on GDP ratio
*/
RESULT( ( SUMS( GRANDPARENT, "i1net" ) + VS( SECSTAL2, "iCnet" ) ) /
		  VS( GRANDPARENT, "GDPnom" ) )


/*========================= FINANCIAL SECTOR STATS ===========================*/

EQUATION( "Bda" )
/*
Firms financial fragility defined as the ratio between accumulated bad debt (loans
in default) and total bank assets (stock of loans)
*/
RESULT( SUMS( FINSECL2, "_Bda" ) )


EQUATION( "Bfail" )
/*
Rate of failing banks
*/
VS( FINSECL2, "NWb" );							// ensure _Gbail it is updated
RESULT( COUNT_CNDS( FINSECL2, "Bank", "_Gbail", ">", 0 ) /
		COUNTS( FINSECL2, "Bank" ) )


EQUATION( "HHb" )
/*
Normalized Herfindahl-Hirschman index for banking sector
*/
i = COUNTS( FINSECL2, "Bank" );
RESULT( i > 1 ? max( 0, ( WHTAVES( FINSECL2, "_fB", "_fB" ) - 1.0 / i ) /
						( 1 - 1.0 / i ) ) : 1 )


EQUATION( "HPb" )
/*
Hymer-Pashigian index for banking sector
*/

v[0] = 0;										// index accumulator
CYCLES( FINSECL2, cur, "Bank" )
	v[0] += fabs( VLS( cur, "_fB", 1 ) - VS( cur, "_fB" ) );// sum share changes

RESULT( v[0] )


/*======================= CAPITAL-GOOD SECTOR STATS ==========================*/

EQUATION( "ABgFront" )
/*
Combined notional labor productivity of machine generation (paradigm) at
technological frontier
*/
cur = SHOOKS( SEARCHS( GRANDPARENT, "Capital" ) );
RESULT( cur != NULL ? VS( cur, "_A1g" ) * VS( cur, "_B1g" ) : 1 )


EQUATION( "Ak" )
/*
Weighted-average labor productivity of firms in capital-good sector
*/

v[1] = v[2] = 0;								// productivity/labor accum.
CYCLES( GRANDPARENT, cur, "Capital" )
{
	v[1] += WHTAVES( cur, "B1", "L1" );
	v[2] += SUMS( cur, "L1" );
}

RESULT( v[2] > 0 ? v[1] / v[2] : 0 )


EQUATION( "EXk" )
/*
Entry-exit index for capital-good sector
*/

v[1] = v[2] = 0;
CYCLES( GRANDPARENT, cur, "Capital" )
{
	v[1] += WHTAVES( cur, "entry1", "F1" );
	v[2] += WHTAVES( cur, "exit1", "F1" );
}

RESULT( v[1] + v[2] > 0 ? 1 - abs( v[1] - v[2] ) / ( v[1] + v[2] ) : 0 )


EQUATION( "HCkAvg" )
/*
Number of historical clients of capital-good firms
*/

v[0] = i = 0;
CYCLES( GRANDPARENT, cur, "Capital" )
{
	v[0] += SUMS( cur, "_HC1" );
	i += COUNTS( cur, "Firm1" );
}

RESULT( v[0] / i )


EQUATION( "NCkAvg" )
/*
Number of new clients of capital-good firms
*/

v[0] = i = 0;
CYCLES( GRANDPARENT, cur, "Capital" )
{
	v[0] += SUMS( cur, "_NC1" );
	i += COUNTS( cur, "Firm1" );
}

RESULT( v[0] / i )


/*======================= CONSUMER-GOOD SECTOR STATS =========================*/

EQUATION( "Ac" )
/*
Weighted-average (single-stage) labor productivity of firms in
consumption-goods sector
Machine-level productivity is notional (single production stage) and
DO NOT directly represent firm final labor productivity
Also updates 'AcPreChg', 'AcPosChg'
*/

v[1] = v[2] = v[3] = v[4] = 0;					// productivity/share accum.
CYCLES( GRANDPARENT, cur, "Consumption" )
{
	CYCLES( cur, cur1, "Firm2" )
	{
		v[5] = VS( cur1, "_L2" );

		if ( ! VS( cur1, "_post2chg" ) )		// pre-change?
		{
			v[1] += VS( cur1, "_A2" ) * v[5];
			v[2] += v[5];
		}
		else
		{
			v[3] += VS( cur1, "_A2" ) * v[5];
			v[4] += v[5];
		}
	}
}

WRITE( "AcPreChg", v[2] > 0 ? v[1] / v[2] : 0 );
WRITE( "AcPosChg", v[4] > 0 ? v[3] / v[4] : 0 );

RESULT( v[2] + v[4] > 0 ? ( v[1] + v[3] ) / ( v[2] + v[4] ) : 0 )


EQUATION( "AsdC" )
/*
Standard deviation of (single-stage) log labor productivity of firms in
consumption-good sector
Also updates 'AsdCpreChg', 'AsdCposChg'
*/

v[1] = V( "Ac" );								// average productivities
v[2] = V( "AcPreChg" );
v[3] = V( "AcPosChg" );

v[1] = ( v[1] >= 0 ) ? log( v[1] + 1 ) : 0;		// average log productivities
v[2] = ( v[2] >= 0 ) ? log( v[2] + 1 ) : 0;
v[3] = ( v[3] >= 0 ) ? log( v[3] + 1 ) : 0;

v[0] = v[4] = v[5] = i = j = k = 0;
CYCLES( GRANDPARENT, cur, "Consumption" )
	CYCLES( cur, cur1, "Firm2" )
	{
		v[6] = VS( cur1, "_A2" );
		if ( v[6] < 0 )
			continue;

		v[0] += pow( log( v[6] + 1 ) - v[1], 2 );
		++i;

		if ( ! VS( cur1, "_post2chg" ) )		// pre-change?
		{
			v[4] += pow( log( v[6] + 1 ) - v[2], 2 );
			++j;
		}
		else
		{
			v[5] += pow( log( v[6] + 1 ) - v[3], 2 );
			++k;
		}
	}

WRITE( "AsdCpreChg", j > 0 ? sqrt( v[4] / j ) : 0 );
WRITE( "AsdCposChg", k > 0 ? sqrt( v[5] / k ) : 0 );

RESULT( i > 0 ? sqrt( v[0] / i ) : 0 )


EQUATION( "BonC" )
/*
Total bonuses paid in consumption-good sector
*/
RESULT( SUMS( GRANDPARENT, "Bon2" ) )


EQUATION( "Dc" )
/*
Demand (in money terms) fulfilled by consumption-good sector
*/
RESULT( SUMS( GRANDPARENT, "D2" ) )


EQUATION( "DcA" )
/*
Demand (in money terms) allocated to consumption-good sector
*/
RESULT( SUMS( GRANDPARENT, "D2a" ) )


EQUATION( "DcAbas" )
/*
Demand (in money terms) allocated to basic consumption-good subsector
*/
RESULT( SUM_CNDS( GRANDPARENT, "D2a", "type2", "==", 0 ) )


EQUATION( "DcAlux" )
/*
Demand (in money terms) allocated to luxury consumption-good subsector
*/
RESULT( V( "DcA" ) - V( "DcAbas") )


EQUATION( "DcD" )
/*
Demand (in money terms) desired from consumption-good sector
*/
RESULT( SUMS( GRANDPARENT, "D2d" ) )


EQUATION( "DcDbas" )
/*
Demand (in money terms) desired from basic consumption-good subsector
*/
RESULT( SUM_CNDS( GRANDPARENT, "D2d", "type2", "==", 0 ) )


EQUATION( "DcDlux" )
/*
Demand (in money terms) desired from luxury consumption-good subsector
*/
RESULT( V( "DcD" ) - V( "DcDbas") )


EQUATION( "DcE" )
/*
Demand (in money terms) expected by consumer-good sector
*/
RESULT( SUMS( GRANDPARENT, "D2e" ) )


EQUATION( "DcEbas" )
/*
Demand (in money terms) expected by basic consumer-good subsector
*/
RESULT( SUM_CNDS( GRANDPARENT, "D2e", "type2", "==", 0 ) )


EQUATION( "DcElux" )
/*
Demand (in money terms) expected by luxury consumer-good subsector
*/
RESULT( V( "DcE" ) - V( "DcEbas") )


EQUATION( "DebC" )
/*
Total debt of consumption-good sector
*/
RESULT( SUMS( GRANDPARENT, "Deb2" ) )


EQUATION( "DebCbas" )
/*
Total debt of basic consumption-good subsector
*/
RESULT( SUM_CNDS( GRANDPARENT, "Deb2", "type2", "==", 0 ) )


EQUATION( "DebClux" )
/*
Total debt of luxury consumption-good subsector
*/
RESULT( V( "DebC" ) - V( "DebCbas") )


EQUATION( "EXc" )
/*
Entry-exit index for consumption-good sector
*/

v[1] = v[2] = 0;
CYCLES( GRANDPARENT, cur, "Consumption" )
{
	v[1] += WHTAVES( cur, "entry2", "F2" );
	v[2] += WHTAVES( cur, "exit2", "F2" );
}

RESULT( v[1] + v[2] > 0 ? 1 - abs( v[1] - v[2] ) / ( v[1] + v[2] ) : 0 )


EQUATION( "FcF" )
/*
Number of firms in consumer-good sector
*/
RESULT( SUMS( GRANDPARENT, "F2" ) )


EQUATION( "FcFbas" )
/*
Number of firms in basic consumer-good subsector
*/
RESULT( SUM_CNDS( GRANDPARENT, "F2", "type2", "==", 0 ) )


EQUATION( "FcFlux" )
/*
Number of firms in luxury consumer-good subsector
*/
RESULT( V( "FcF" ) - V( "FcFbas" ) )


EQUATION( "HHc" )
/*
Herfindahl-Hirschman index for consumption-good sector
*/
i = COUNTS( GRANDPARENT, "Consumption" )
RESULT( i > 1 ? max( 0, ( WHTAVES( GRANDPARENT, "f2e", "f2e" ) - 1.0 / i ) /
						( 1 - 1.0 / i ) ) : 1 )


EQUATION( "HPc" )
/*
Hymer-Pashigian index for consumption-good sector
*/

v[0] = 0;										// index accumulator
CYCLES( GRANDPARENT, cur, "Consumption" )
	v[0] += fabs( VLS( cur, "f2e", 1 ) - VS( cur, "f2e" ) );// sum share changes

RESULT( v[0] )


EQUATION( "KcBas" )
/*
Aggregated capital stock (in machine-number terms) in
basic consumption-good subsector
*/
RESULT( SUM_CNDS( GRANDPARENT, "K2", "type2", "==", 0 ) )


EQUATION( "KcLux" )
/*
Aggregated capital stock (in machine-number terms) in
basic consumption-good subsector
*/
RESULT( VS( GRANDPARENT, "K" ) - V( "KcBas" ) )


EQUATION( "Lc" )
/*
Work force (labor) size employed by consumption-good sector
*/
RESULT( SUMS( GRANDPARENT, "L2" ) )


EQUATION( "LcBas" )
/*
Work force (labor) size employed by basic consumption-good subsector
*/
RESULT( SUM_CNDS( GRANDPARENT, "L2", "type2", "==", 0 ) )


EQUATION( "LcLux" )
/*
Work force (labor) size employed by basic consumption-good subsector
*/
RESULT( V( "Lc" ) - V( "LcBas" ) )


EQUATION( "LdC" )
/*
Total labor demand in consumption-good sector
*/
RESULT( SUMS( GRANDPARENT, "L2d" ) )


EQUATION( "MCcAvg" )
/*
Weighted-average expected market conditions index for entry in consumer-good industries
*/
RESULT( WHTAVES( GRANDPARENT, "MC2", "f2e" ) )


EQUATION( "NcNomBas" )
/*
Total inventories (in money terms) of basic consumption-good subsector
*/
RESULT( WHTAVE_CNDS( GRANDPARENT, "N2", "p2", "type2", "==", 0 ) )


EQUATION( "NcNomLux" )
/*
Total inventories (in money terms) of luxury consumption-good subsector
*/
RESULT( VS( MACSTAL2, "Nnom" ) - V( "NcNomBas" ) )


EQUATION( "NWc" )
/*
Total net wealth (free cash) of consumption-good sector
*/
RESULT( SUMS( GRANDPARENT, "NW2" ) )


EQUATION( "PiC" )
/*
Total profits of consumer-good sector
*/
RESULT( SUMS( GRANDPARENT, "Pi2" ) )


EQUATION( "PiCrateAvg" )
/*
Average (weighted by wallet share) profit rate (over capital)
of consumption-good sector
*/
RESULT( WHTAVES( GRANDPARENT, "Pi2rateAvg", "f2e" ) / SUMS( GRANDPARENT, "f2e" ) )


EQUATION( "PiCrateAvgBas" )
/*
Average (weighted by wallet share) profit rate (over capital)
of basic consumption-good subsector
*/
v[1] = SUM_CNDS( GRANDPARENT, "f2e", "type2", "==", 0 );
RESULT( v[1] > 0 ?
		WHTAVE_CNDS( GRANDPARENT, "Pi2rateAvg", "f2e", "type2", "==", 0 ) /
		v[1] : 0 )


EQUATION( "PiCrateAvgLux" )
/*
Average (weighted by wallet share) profit rate (over capital)
of luxury consumption-good subsector
*/
v[1] = SUM_CNDS( GRANDPARENT, "f2e", "type2", ">", 0 );
RESULT( v[1] > 0 ?
		WHTAVE_CNDS( GRANDPARENT, "Pi2rateAvg", "f2e", "type2", ">", 0 ) /
		v[1] : 0 )


EQUATION( "Qc" )
/*
Total planned output (in unit terms) before
labor/financial constraints of consumption-good sector
*/
RESULT( SUMS( GRANDPARENT, "Q2" ) )


EQUATION( "QcBas" )
/*
Total planned output (in unit terms) before
labor/financial constraints of basic consumption-good subsector
*/
RESULT( SUM_CNDS( GRANDPARENT, "Q2", "type2", "==", 0 ) )


EQUATION( "QcLux" )
/*
Total planned output (in unit terms) before
labor/financial constraints of luxury consumption-good subsector
*/
RESULT( V( "Qc" ) - V( "QcBas" ) )


EQUATION( "QcD" )
/*
Total desired output (in unit terms) of consumption-good sector
*/
RESULT( SUMS( GRANDPARENT, "Q2d" ) )


EQUATION( "QcDbas" )
/*
Total desired output (in unit terms) of basic consumption-good subsector
*/
RESULT( SUM_CNDS( GRANDPARENT, "Q2d", "type2", "==", 0 ) )


EQUATION( "QcDlux" )
/*
Total desired output (in unit terms) of luxury consumption-good subsector
*/
RESULT( V( "QcD" ) - V( "QcDbas" ) )


EQUATION( "QcDnom" )
/*
Total desired nominal output (in money terms) of consumption-goods sector
*/

v[0] = 0;
CYCLES( GRANDPARENT, cur, "Consumption" )
	v[0] += WHTAVES( cur, "_Q2d", "_p2" );

RESULT( v[0] )


EQUATION( "QcDnomBas" )
/*
Total desired nominal output (in money terms) of
basic consumption-goods subsector
*/

v[0] = 0;
CYCLES( GRANDPARENT, cur, "Consumption" )
	if ( VS( cur, "type2" ) == 0 )
		v[0] += WHTAVES( cur, "_Q2d", "_p2" );

RESULT( v[0] )


EQUATION( "QcDnomLux")
/*
Total desired nominal output (in money terms) of
luxury consumption-goods subsector
*/
RESULT( V( "QcDnom" ) - V( "QcDnomBas" ) )


EQUATION( "QcE" )
/*
Total effective output (in unit terms) of industries in consumption-good sector
*/
RESULT( SUMS( GRANDPARENT, "Q2e" ) )


EQUATION( "QcEbas" )
/*
Total effective output (in unit terms) of basic consumption-good subsector
*/
RESULT( SUM_CNDS( GRANDPARENT, "Q2e", "type2", "==", 0 ) )


EQUATION( "QcElux" )
/*
Total effective output (in unit terms) of luxury consumption-good subsector
*/
RESULT( V( "QcE" ) - V( "QcEbas" ) )


EQUATION( "QcEnom" )
/*
Total effective nominal output (in money terms) of consumption-goods sector
*/

v[0] = 0;
CYCLES( GRANDPARENT, cur, "Consumption" )
	v[0] += WHTAVES( cur, "_Q2e", "_p2" );

RESULT( v[0] )


EQUATION( "QcEnomBas" )
/*
Total effective nominal output (in money terms) of
basic consumption-goods subsector
*/

v[0] = 0;
CYCLES( GRANDPARENT, cur, "Consumption" )
	if ( VS( cur, "type2" ) == 0 )
		v[0] += WHTAVES( cur, "_Q2e", "_p2" );

RESULT( v[0] )


EQUATION( "QcEnomLux")
/*
Total effective nominal output (in money terms) of
luxury consumption-goods subsector
*/
RESULT( V( "QcEnom" ) - V( "QcEnomBas" ) )


EQUATION( "QcP" )
/*
Potential output (in unit terms) with current machines/workers in
consumption-good sector
*/
RESULT( SUMS( GRANDPARENT, "Q2p" ) )


EQUATION( "QcPbas" )
/*
Potential output (in unit terms) with current machines/workers in
basic consumption-good subsector
*/
RESULT( SUM_CNDS( GRANDPARENT, "Q2p", "type2", "==", 0 ) )


EQUATION( "QcPlux" )
/*
Potential output (in unit terms) with current machines/workers in
luxury consumption-good subsector
*/
RESULT( V( "QcP" ) - V( "QcPbas" ) )


EQUATION( "QcU" )
/*
Average capacity utilization of consumption-good sector
*/
v[1] = V( "QcP" );
RESULT( v[1] > 0 ? WHTAVES( GRANDPARENT, "Q2u", "Q2p" ) / v[1] : 0 )


EQUATION( "QcUbas" )
/*
Capacity utilization of basic consumption-good subsector
*/
v[1] = V( "QcPbas" );
RESULT( v[1] > 0 ? WHTAVE_CNDS( GRANDPARENT, "Q2u", "Q2p", "type2", "==", 0 ) /
				   v[1] : 0 )


EQUATION( "QcUlux" )
/*
Capacity utilization of luxury consumption-good subsector
*/
v[1] = V( "QcPlux" );
RESULT( v[1] > 0 ? WHTAVE_CNDS( GRANDPARENT, "Q2u", "Q2p", "type2", ">", 0 ) /
				   v[1] : 0 )


EQUATION( "RSc" )
/*
Machine (planned) scrapping rate in consumption-good sector
*/
v[1] = SUMLS( GRANDPARENT, "K2", 1 );
RESULT( T > 1 && v[1] > 0 ? SUMS( GRANDPARENT, "RS2" ) / v[1] : 0 )


EQUATION( "RScBas" )
/*
Machine (planned) scrapping rate in basic consumption-good sector
*/
v[1] = SUM_CNDLS( GRANDPARENT, "K2", "type2", "==", 0, 1 );
RESULT( v[1] > 1 ? SUM_CNDS( GRANDPARENT, "RS2", "type2", "==", 0 ) / v[1] : 0 )


EQUATION( "RScLux" )
/*
Machine (planned) scrapping rate in basic consumption-good sector
*/
v[1] = SUM_CNDLS( GRANDPARENT, "K2", "type2", ">", 0, 1 );
RESULT( v[1] > 1 ? SUM_CNDS( GRANDPARENT, "RS2", "type2", ">", 0 ) / v[1] : 0 )


EQUATION( "TaxC" )
/*
Total taxes paid in consumption-good sector
*/
RESULT( SUMS( GRANDPARENT, "Tax2" ) )


EQUATION( "VAcW" )
/*
Value added per worker in consumption-good sector
*/
RESULT( SUMS( GRANDPARENT, "VA2w" ) )


EQUATION( "VAcWbas" )
/*
Value added per worker in for basic consumption-good subsector
*/
RESULT( SUM_CNDS( GRANDPARENT, "VA2w", "type2", "==", 0 ) )


EQUATION( "VAcWlux" )
/*
Value added per worker in luxury consumption-good subsector
*/
RESULT( V( "VAcW" ) - V( "VAcWbas" ) )


EQUATION( "Wc" )
/*
Total wages paid in consumption-good sector
*/
RESULT( SUMS( GRANDPARENT, "W2" ) )


EQUATION( "ageCavg" )
/*
Average age of firms in consumption-good sector
*/
RESULT( AVES( GRANDPARENT, "age2avg" ) )


EQUATION( "cCeAvg")
/*
Effective weighted-average unit cost in consumption-good sector
*/
RESULT( WHTAVES( GRANDPARENT, "c2eAvg", "f2e" ) )


EQUATION( "cCeAvgBas")
/*
Effective weighted-average unit cost in basic consumption-good subsector
*/
v[1] = WHTAVE_CNDS( GRANDPARENT, "c2eAvg", "f2e", "type2", "==", 0 );
v[2] = SUM_CNDS( GRANDPARENT, "f2e", "type2", "==", 0 );
RESULT( v[1] > 0 && v[2] > 0 ? v[1] / v[2] : 0 )


EQUATION( "cCeAvgLux")
/*
Effective weighted-average unit cost in basic consumption-good subsector
*/
v[1] = WHTAVE_CNDS( GRANDPARENT, "c2eAvg", "f2e", "type2", ">", 0 );
v[2] = SUM_CNDS( GRANDPARENT, "f2e", "type2", ">", 0 );
RESULT( v[1] > 0 && v[2] > 0 ? v[1] / v[2] : 0 )


EQUATION( "credCc" )
/*
Total credit constraint of firms in consumer-good sector
*/
RESULT( SUMS( GRANDPARENT, "cred2c" ) )


EQUATION( "credCcBas" )
/*
Total credit constraint of firms in basic consumer-good subsector
*/
RESULT( SUM_CNDS( GRANDPARENT, "cred2c", "type2", "==", 0 ) )


EQUATION( "credCcLux" )
/*
Total credit constraint of firms in luxury consumer-good subsector
*/
RESULT( V( "credCc" ) - V( "credCcBas" ) )


EQUATION( "entryC" )
/*
Rate of entering firms in consumption-good sector
*/
RESULT( WHTAVES( GRANDPARENT, "entry2", "F2" ) / SUMS( GRANDPARENT, "F2" ) )


EQUATION( "exitC" )
/*
Rate of exiting firms in consumption-good sector
*/
RESULT( WHTAVES( GRANDPARENT, "exit2", "F2" ) / SUMS( GRANDPARENT, "F2" ) )


EQUATION( "fCposChg" )
/*
Weighted-average joint market share hold by all firms of post-change type
in consumption-good sector
*/
RESULT( WHTAVES( GRANDPARENT, "f2posChg", "F2" ) / SUMS( GRANDPARENT, "F2" ) )


EQUATION( "iCnetC" )
/*
Net interest cost over total costs in consumption-good sector
*/
RESULT( 1 / ( 1 + VS( LABSTAL2, "Wc" ) / V( "iCnet" ) ) )


EQUATION( "iCnet" )
/*
Net interest expenses (income) by consumption-good sector
*/
RESULT( SUMS( GRANDPARENT, "i2net" ) )


EQUATION( "iCnetBas" )
/*
Net interest expenses (income) by basic consumption-good subsector
*/
RESULT( SUM_CNDS( GRANDPARENT, "i2net", "type2", "==", 0 ) )


EQUATION( "iCnetLux" )
/*
Net interest expenses (income) by luxury consumption-good subsector
*/
RESULT( V( "iCnet" ) - V( "iCnetBas" ) )


EQUATION( "kCavg" )
/*
Weighted-average expected complexity of consumption-good sector
*/
RESULT( WHTAVES( GRANDPARENT, "k2", "f2" ) / SUMS( GRANDPARENT, "f2" ) )


EQUATION( "muCavg" )
/*
Weighted-average mark-up of firms in consumption-good sector
*/
RESULT( WHTAVES( GRANDPARENT, "mu2avg", "f2e" ) / SUMS( GRANDPARENT, "f2e" ) )


EQUATION( "muCavgBas" )
/*
Weighted-average mark-up of firms in basic consumption-good subsector
*/
v[1] = WHTAVE_CNDS( GRANDPARENT, "mu2avg", "f2e", "type2", "==", 0 );
v[2] = SUM_CNDS( GRANDPARENT, "f2e", "type2", "==", 0 );
RESULT( v[1] > 0 && v[2] > 0 ? v[1] / v[2] : 0 )


EQUATION( "muCavgLux" )
/*
Weighted-average mark-up of firms in basic consumption-good subsector
*/
v[1] = WHTAVE_CNDS( GRANDPARENT, "mu2avg", "f2e", "type2", ">", 0 );
v[2] = SUM_CNDS( GRANDPARENT, "f2e", "type2", ">", 0 );
RESULT( v[1] > 0 && v[2] > 0 ? v[1] / v[2] : 0 )


EQUATION( "nBrochCavg" )
/*
Average number of machine brochures available to firms in consumer-good sector
*/

v[0] = i = 0;
CYCLES( GRANDPARENT, cur, "Consumption" )
	CYCLES( cur, cur1, "Firm2" )
	{
		v[0] += COUNTS( cur1, "Broch");
		++i;
	}

RESULT( v[0] / i )


EQUATION( "nCavg" )
/*
Weighted-average product newness of consumption-good sector
*/
RESULT( T - WHTAVES( GRANDPARENT, "t2ent", "f2e" ) / SUMS( GRANDPARENT, "f2e" ) )


EQUATION( "nCavgBas" )
/*
Weighted-average newness (age) of industries in basic consumption-good subsector
*/
v[1] = WHTAVE_CNDS( GRANDPARENT, "t2ent", "f2e", "type2", "==", 0 );
v[2] = SUM_CNDS( GRANDPARENT, "f2e", "type2", "==", 0 );
RESULT( v[1] > 0 && v[2] > 0 ? T - v[1] / v[2] : 0 )


EQUATION( "nCavgLux" )
/*
Weighted-average newness (age) of industries in luxury consumption-good subsector
*/
v[1] = WHTAVE_CNDS( GRANDPARENT, "t2ent", "f2e", "type2", ">", 0 );
v[2] = SUM_CNDS( GRANDPARENT, "f2e", "type2", ">", 0 );
RESULT( v[1] > 0 && v[2] > 0 ? T - v[1] / v[2] : 0 )


EQUATION( "noWrkC" )
/*
Share of operating firms with no worker in consumption-good sector
*/
RESULT( WHTAVES( GRANDPARENT, "noWrk2", "F2" ) / SUMS( GRANDPARENT, "F2" ) )


EQUATION( "noWrkCbas" )
/*
Share of operating firms with no worker in basic consumption-good subsector
*/
v[1] = SUM_CNDS( GRANDPARENT, "F2", "type2", "==", 0 );
RESULT( v[1] > 0 ? WHTAVE_CNDS( GRANDPARENT, "noWrk2", "F2", "type2", "==", 0 ) /
		v[1] : 0 )


EQUATION( "noWrkClux" )
/*
Share of operating firms with no worker in luxury consumption-good subsector
*/
v[1] = SUM_CNDS( GRANDPARENT, "F2", "type2", ">", 0 );
RESULT( v[1] > 0 ? WHTAVE_CNDS( GRANDPARENT, "noWrk2", "F2", "type2", ">", 0 ) /
		v[1] : 0 )


EQUATION( "pCavg" )
/*
Weighted-average price of consumption-good sector
*/
RESULT( WHTAVES( GRANDPARENT, "p2", "f2e" ) / SUMS( GRANDPARENT, "f2e" ) )


EQUATION( "pCavgBas" )
/*
Weighted-average price of basic consumption-good subsector
*/
v[1] = WHTAVE_CNDS( GRANDPARENT, "p2", "f2e", "type2", "==", 0 );
v[2] = SUM_CNDS( GRANDPARENT, "f2e", "type2", "==", 0 );
RESULT( v[1] > 0 && v[2] > 0 ? v[1] / v[2] : 0 )


EQUATION( "pCavgLux" )
/*
Weighted-average price of luxury consumption-good subsector
*/
v[1] = WHTAVE_CNDS( GRANDPARENT, "p2", "f2e", "type2", ">", 0 );
v[2] = SUM_CNDS( GRANDPARENT, "f2e", "type2", ">", 0 );
RESULT( v[1] > 0 && v[2] > 0 ? v[1] / v[2] : 0 )


EQUATION( "qCavg" )
/*
Weighted-average expected product quality of consumption-good sector
*/
v[1] = SUMS( GRANDPARENT, "Q2e" );
RESULT( v[1] > 0 ? WHTAVES( GRANDPARENT, "q2", "Q2e" ) / v[1] : CURRENT )


EQUATION( "qCpreChg" )
/*
Weighted-average productivity of pre-change firms in consumption-good sector
Also updates 'qCposChg'
*/

v[0] = v[1] = v[2] = v[3] = 0;
CYCLES( GRANDPARENT, cur, "Consumption" )
	CYCLES( cur, cur1, "Firm2" )
	{
		v[4] = VS( cur1, "_q2" );
		v[5] = VS( cur1, "_Q2e" );

		if ( ! VS( cur1, "_post2chg" ) )		// pre-change?
		{
			v[0] += v[4] * v[5];
			v[1] += v[5];
		}
		else
		{
			v[2] += v[4] * v[5];
			v[3] += v[5];
		}
	}

WRITE( "qCposChg", v[3] > 0 ? v[2] / v[3] : 0 );

RESULT( v[1] > 0 ? v[0] / v[1] : 0 )


EQUATION( "wCavg" )
/*
Weighted-average wage paid in consumption-good sector
*/
v[1] = SUMS( GRANDPARENT, "L2" );
RESULT( v[1] > 0 ? WHTAVES( GRANDPARENT, "w2avg", "L2" ) / v[1] : 0 )


EQUATION( "wCavgBas" )
/*
Weighted-average wage paid in basic consumption-good subsector
*/
v[1] = WHTAVE_CNDS( GRANDPARENT, "w2avg", "L2", "type2", "==", 0 );
v[2] = SUM_CNDS( GRANDPARENT, "L2", "type2", "==", 0 );
RESULT( v[1] > 0 && v[2] > 0 ? v[1] / v[2] : 0 )


EQUATION( "wCavgLux" )
/*
Weighted-average wage paid in luxury consumption-good subsector
*/
v[1] = WHTAVE_CNDS( GRANDPARENT, "w2avg", "L2", "type2", ">", 0 );
v[2] = SUM_CNDS( GRANDPARENT, "L2", "type2", ">", 0 );
RESULT( v[1] > 0 && v[2] > 0 ? v[1] / v[2] : 0 )


/*============================= LABOR STATS ==================================*/

EQUATION( "BonPayers" )
/*
Share of bonus paying firms in consumption-good sector
*/

i = j = 0;										// firm counter
CYCLES( GRANDPARENT, cur, "Consumption" )
	CYCLES( cur, cur1, "Firm2" )
	{
		if ( VS( cur1, "_Bon2" ) > 0 )
			++i;
		++j;
	}

RESULT( ( double ) i / j )


EQUATION( "In" )
/*
Income received by workers (including bonus and unemployment benefits)
*/
RESULT( SUMS( LABSUPL2, "_In" ) * VS( LABSUPL2, "Lscale" ) )


EQUATION( "InAvg" )
/*
Average income received by workers (including bonus and unemployment benefits)
*/
RESULT( AVES( LABSUPL2, "_In" ) )


EQUATION( "Lent" )
/*
Entry rate of labor (hires over total labor)
*/
RESULT( ( SUMS( GRANDPARENT, "hires1" ) + SUMS( GRANDPARENT, "hires2" ) ) /
		VS( LABSUPL2, "Ls" ) )


EQUATION( "Lexit" )
/*
Exit rate of labor (fires+quits+retires over labor employed)
*/
v[1] = SUMS( GRANDPARENT, "fires1" ) + SUMS( GRANDPARENT, "quits1" ) +
	   SUMS( GRANDPARENT, "retires1" ) + SUMS( GRANDPARENT, "fires2" ) +
	   SUMS( GRANDPARENT, "quits2" ) + SUMS( GRANDPARENT, "retires2" );
RESULT( v[1] / VS( LABSUPL2, "Ls" ) )


EQUATION( "Llarg" )
/*
Number of workers of largest firm
*/

v[0] = 0;

CYCLES( GRANDPARENT, cur, "Capital" )
	v[0] = max( v[0], MAXS( cur, "_L1" ) );

CYCLES( GRANDPARENT, cur, "Consumption" )
	v[0] = max( v[0], MAXS( cur, "_L2" ) );

RESULT( v[0] )


EQUATION( "TuAvg" )
/*
Average number of periods of unemployment for unemployed workers
*/
v[0] = AVE_CNDS( LABSUPL2, "_Tu", "_employed", "==", 0 );
RESULT( v[0] > 0 ? v[0] : 0 )


EQUATION( "V" )
/*
Effective vacancy rate (unfilled positions over total labor supply)
*/
RESULT( T > 1 ? min( ( SUMS( GRANDPARENT, "JO1" ) + SUMS( GRANDPARENT, "JO2" ) ) /
					   VS( LABSUPL2, "Ls" ), 1 ) : 0 )


EQUATION( "dw" )
/*
Nominal average wage growth rate
*/
v[1] = VLS( LABSUPL2, "wAvg", 1 );
v[2] = VS( LABSUPL2, "wAvg" );
RESULT( v[1] > 0 && v[2] > 0 ? log( v[2] ) - log( v[1] ) : 0 )


EQUATION( "noWrk" )
/*
Share of operating firms with no worker hired
*/
RESULT( ( WHTAVES( GRANDPARENT, "noWrk1", "L1" ) +
		  WHTAVES( GRANDPARENT, "noWrk2", "L2" ) ) / VS( LABSUPL2, "L" ) )


EQUATION( "Lpart" )
/*
Participation rate of the work force (employed + looking for jobs)
*/

i = 0;
CYCLES( LABSUPL2, cur, "Worker" )				// consider all workers
	// but account only employed or searching workers
	if ( VS( cur, "_employed" ) || VS( cur, "_appl" ) > 0 )
		++i;

RESULT( i * VS( LABSUPL2, "Lscale" ) / VS( LABSUPL2, "Ls" ) )


EQUATION( "wAvgLarg" )
/*
Average wage paid by largest firm (in number of workers)
*/

v[0] = v[1] = 0;								// max accumulator

CYCLES( GRANDPARENT, cur, "Capital" )
	CYCLES( cur, cur1, "Firm1" )				// scan all firms
	{
		v[2] = VS( cur1, "_W1" );
		v[3] =  VS( cur1, "_L1" );

		if ( v[3] > v[1] )						// largest firm yet?
		{
			v[1] = v[3];
			v[0] = v[2] / v[3];					// average wage of largest
		}
	}

CYCLES( GRANDPARENT, cur, "Consumption" )
	CYCLES( cur, cur1, "Firm2" )				// scan all firms
	{
		v[2] = VS( cur1, "_W2" );
		v[3] =  VS( cur1, "_L2" );

		if ( v[3] > v[1] )						// largest firm yet?
		{
			v[1] = v[3];
			v[0] = v[2] / v[3];					// average wage of largest
		}
	}

RESULT( v[0] )


EQUATION( "InAvgReal" )
/*
Average real income received by workers (including bonus and unemployment benefit)
Also updates 'InMax', 'InMin', 'Gini', 'wrAvg', 'wrMax', 'wrMin', 'wsAvg',
'wsMax', 'wsMin', 'InLogSD', 'wrLogSD', 'wsLogSD'
*/

double Lscale = VS( LABSUPL2, "Lscale" );		// workers per object

j = COUNTS( LABSUPL2, "Worker" ) * Lscale;		// count workers
j += SUMS( GRANDPARENT, "F1" );					// count firms (after entry-exit)
j += SUMS( GRANDPARENT, "F2" );

double *rank = new double[ j ];					// allocate space for indiv. rank

i = k = 0;
v[1] = v[2] = v[3] = v[4] = 0;
v[11] = v[12] = v[13] = v[14] = 0;
v[21] = v[22] = v[23] = v[24] = 0;
v[5] = v[15] = v[25] = DBL_MAX;					// minimum registers
CYCLES( LABSUPL2, cur, "Worker" )				// consider all workers
{
	v[0] = VS( cur, "_In" );					// current income
	v[1] += v[0];								// sum of income
	v[2] += log( v[0] + 1 );					// sum of log income
	v[3] += pow( log( v[0] + 1 ), 2 );			// sum of squared log income
	v[4] = max( v[0], v[4] );					// max income
	v[5] = min( v[0], v[5] );					// min income

	for ( int n = 0; n < Lscale; ++n )
		rank[ k++ ] = v[0];						// prepare income rank

	v[0] = VS( cur, "_wS" );					// current satisfacing wage
	v[11] += v[0];								// sum of satisfacing wages
	v[12] += log( v[0] + 1 );					// sum of log satisfacing wages
	v[13] += pow( log( v[0] + 1 ), 2 );			// sum of sq. log satisf. wages
	v[14] = max( v[0], v[14] );					// max satisfacing wage
	v[15] = min( v[0], v[15] );					// min satisfacing wage

	v[0] = VS( cur, "_wR" );					// current requested wage
	v[21] += v[0];								// sum of requested wages
	v[22] += log( v[0] + 1 );					// sum of log requested wages
	v[23] += pow( log( v[0] + 1 ), 2 );			// sum of sq. log req.  wages
	v[24] = max( v[0], v[24] );					// max requested wage
	v[25] = min( v[0], v[25] );					// min requested wage

	++i;
}

CYCLES( GRANDPARENT, cur, "Capital" )
	CYCLES( cur, cur1, "Firm1" )				// consider sector 1 firm owners
		rank[ k++ ] = max( VS( cur1, "_Div1" ) + VS( cur1, "_NW1" ) -
						   VLS( cur1, "_NW1", 1 ), 0 );
												// Positive net wealth increase

CYCLES( GRANDPARENT, cur, "Consumption" )
	CYCLES( cur, cur1, "Firm2" )				// consider sector 2 firm owners
		rank[ k++ ] = max( VS( cur1, "_Div2" ) + VS( cur1, "_NW2" ) -
						   VLS( cur1, "_NW2", 1 ), 0 );
												// Positive net wealth increase
// averages
v[6] = v[1] / i;								// average of wages
v[16] = v[11] / i;								// average of satisfacing wages
v[26] = v[21] / i;								// average of requested wages

// SDs
v[7] = sqrt( max( ( v[3] / i ) - pow( v[2] / i, 2 ), 0 ) );// SD of log wages
v[17] = sqrt( max( ( v[13] / i ) - pow( v[12] / i, 2 ), 0 ) );// SD log sat. wage
v[27] = sqrt( max( ( v[23] / i ) - pow( v[22] / i, 2 ), 0 ) );// SD log req. wage

sort( rank, rank + j, std::greater< double > ( ) );	// sort in descending order

for ( v[8] = v[9] = k = 0; k < j; ++k )
{
	v[8] += ( k + 1 ) * rank[ k ];				// sum rank x income
	v[9] += rank[ k ];							// sum income
}

delete [] rank;

// apply the Jasso-Deaton formula
v[10] = ( double ) ( j + 1 ) / ( j - 1 ) - 2 * v[8] / ( ( j - 1 ) * v[9] );

// write results into dummy vars
WRITE( "InMax", v[4] );							// max income
WRITE( "InMin", v[5] );							// min income
WRITE( "InLogSD", v[7] );						// log income SD
WRITE( "wsAvg", v[16] );						// average satisficing wage
WRITE( "wsMax", v[14] );						// max satisficing wage
WRITE( "wsMin", v[15] );						// min satisficing wage
WRITE( "wsLogSD", v[17] );						// log satisficing wage SD
WRITE( "wrAvg", v[26] );						// average requested wage
WRITE( "wrMax", v[24] );						// max requested wage
WRITE( "wrMin", v[25] );						// min requested wage
WRITE( "wrLogSD", v[27] );						// log requested wage SD
WRITES( MACSTAL2, "Gini", v[10] );				// overall Gini index

RESULT( v[6] / VS( MACSTAL2, "GDPdefl" ) )


EQUATION( "wAvgReal" )
/*
Average real wage received by employed workers
(excluding bonus and unemployment benefits)
Also updates 'wMax', 'wMin', 'wGini'
*/

j = COUNTS( LABSUPL2, "Worker" );				// count max scaled workers
double *rank = new double[ j ];					// allocate space for rank

i = 0;
v[1] = v[2] = 0;
v[3] = DBL_MAX;									// minimum register
CYCLES( LABSUPL2, cur, "Worker" )				// consider all workers
	if ( VS( cur, "_employed" ) )				// account only employed
	{
		v[0] = VS( cur, "_w" );					// current wage
		v[1] += v[0];							// sum of wages
		v[2] = max( v[0], v[2] );				// max wage
		v[3] = min( v[0], v[3] );				// min wage

		rank[ i++ ] = v[0];						// insert wage in rank array
	}

sort( rank, rank + i, std::greater< double >( ) );// sort in descending order

for ( v[4] = v[5] = 0, k = 0; k < i; ++k )
{
	v[4] += ( k + 1 ) * rank[ k ];				// sum rank x wage
	v[5] += rank[ k ];							// sum wage
}

delete [] rank;

// apply the Jasso-Deaton formula
v[6] = ( double ) ( i + 1 ) / ( i - 1 ) - 2 * v[4] / ( ( i - 1 ) * v[5] );

WRITE( "wMax", v[2] );
WRITE( "wMin", v[3] );
WRITE( "wGini", i > 1 && v[5] > 0 ? v[6] : 0 );

RESULT( i > 0 ? v[1] / i / VS( MACSTAL2, "GDPdefl" ) : CURRENT )


EQUATION( "wCrealPreChg" )
/*
Weighted average real wage paid by consumption-goods pre-change firms
Also updates 'wCrealPosChg'
*/

i = j = v[0] = v[1] = 0;						// worker counter/wage average
CYCLES( GRANDPARENT, cur, "Consumption" )
	CYCLES( cur, cur1, "Firm2" )				// scan all firms
		if ( ! VS( cur1, "_post2chg" ) )		// pre-change?
		{
			i += h = VS( cur1, "_L2" );
			v[0] +=  VS( cur1, "_w2avg" ) * h;
		}
		else
		{
			j += h = VS( cur1, "_L2" );
			v[1] +=  VS( cur1, "_w2avg" ) * h;
		}

WRITE( "wCrealPosChg", j > 0 ? v[1] / j / VS( MACSTAL2, "GDPdefl" ) : 0 );

RESULT( i > 0 ? v[0] / i / VS( MACSTAL2, "GDPdefl" ) : 0 )


/*========================== INDUSTRY-LEVEL STATS ============================*/

EQUATION( "EI2d" )
/*
esired expansion investment (in machine-number terms) in industry
*/
RESULT( SUM( "_EI2d" ) )


EQUATION( "RS2" )
/*
Number of machines to scrap in consumption-good industry
*/
RESULT( SUM( "_RS2" ) )


EQUATION( "SI2d" )
/*
esired substitution investment (in machine-number terms) in industry
*/
RESULT( SUM( "_SI2d" ) )


EQUATION( "VA2w")
/*
Value added per worker in consumption-good industry
*/
RESULT( SUM( "_VA2w" ) )


EQUATION( "i1net" )
/*
Net interest expenses (income) by firms in capital-good industry
*/
RESULT( SUM( "_i1net" ) )


EQUATION( "i2net" )
/*
Net interest expenses (income) by firms in consumption-good industry
*/
RESULT( SUM( "_i2net" ) )


EQUATION( "w2realAvg" )
/*
Weighted average real wage paid by firms in consumption-good industry
*/
RESULT( V( "w2avg" ) / VS( MACSTAL1, "GDPdefl" ) )


EQUATION( "w2realSD" )
/*
Standard deviation of real wage paid by firms in consumption-good industry
*/
v[1] = SD( "_w2avg" );
RESULT( is_finite( v[1] ) ? v[1] / VS( MACSTAL1, "GDPdefl" ) : 0 )


/*============================ AGENT-LEVEL STATS =============================*/

EQUATION( "_A2e" )
/*
Effective (single-stage) labor productivity of firm in consumption-good sector
Machine-level productivity is notional (single production stage) and
DO NOT directly represent firm final labor productivity
*/
i = V( "_L2" );
RESULT( i > 0 ? V( "_Q2e" ) / i : CURRENT )


EQUATION( "_RS2" )
/*
Number of machines to scrap of firm in consumption-good sector
*/

v[0] = 0;
CYCLE( cur, "Vint" )
	v[0] += abs( VS( cur, "_RSvint" ) );

RESULT( v[0] )


EQUATION( "_VA2w")
/*
Value added per worker for a firm in consumption-good industry
*/
VS( PARENT, "D2" );								// ensure $ demand is allocated
v[1] = V( "_L2" );
RESULT( v[1] > 0 ? V( "_p2" ) * V( "_Q2e" ) / v[1]  : 0 )


EQUATION( "_fG" )
/*
Market share of technological generation (paradigm) of machines
*/
v[1] = VS( MACSTAL2, "Kinst" );
RESULT( v[1] > 0 ? V( "_nG" ) / v[1] : 0 )


EQUATION( "_sT2avg" )
/*
Weighted average workers tenure skills of a firm in consumption-good sector
*/

i = VS( GRANDPARENT, "flagWorkerLBU" );			// worker-level learning mode
if ( i == 0 || i == 1 )							// no learning by tenure mode?
	END_EQUATION( INISKILL );

v[0] = v[1] = 0;
CYCLE( cur, "Wrk2" )
{
	cur1 = SHOOKS( cur );
	v[1] += v[2] = VS( cur1, "_Q" );			// worker current production
	v[0] += VS( cur1, "_sT" ) * v[2] ;			// add worker potential
}

RESULT( v[1] > 0 ? v[0] / v[1] : 0 )			// handle the no production case


EQUATION( "_sV2avg" )
/*
Weighted average workers vintage skills of a firm in consumption-good sector
*/

i = VS( GRANDPARENT, "flagWorkerLBU" );			// worker-level learning mode
if ( i == 0 || i == 2 )							// no learning by vintage mode?
	END_EQUATION( INISKILL );

v[0] = v[1] = 0;
CYCLE( cur, "Wrk2" )
{
	cur1 = SHOOKS( cur );
	v[1] += v[2] = VS( cur1, "_Q" );			// worker current production
	v[0] += VS( cur1, "_sV" ) * v[2] ;			// add worker potential
}

if ( v[0] == 0 || v[1] == 0 )					// no worker hired or no prod.?
{
	j = VS( HOOK( TOPVINT ), "_IDvint" );		// current top vintage
	v[0] = V_EXTS( GRANDPARENT, countryE, vintProd[ j ].sVp );
}												// assume public skills
else
	v[0] /= v[1];								// weighthed average

RESULT( v[0] )


EQUATION( "_InReal" )
/*
Real income of worker
*/
RESULT( V( "_In" ) / VS( MACSTAL2, "GDPdefl" ) )


EQUATION( "_wReal" )
/*
Real wage received by worker
*/
RESULT( V( "_w" ) / VS( MACSTAL2, "GDPdefl" ) )


EQUATION( "_w2realAvg" )
/*
Average real wage paid by firm in consumption-good industry
*/
RESULT( V( "_w2avg" ) / VS( MACSTAL2, "GDPdefl" ) )


/*============================= DUMMY EQUATIONS ==============================*/

EQUATION_DUMMY( "AcPosChg", "Ac" )
/*
Weighted-average (single-stage) labor productivity of post-change firms in
consumption-goods sector
Machine-level productivity is notional (single production stage) and
DO NOT directly represent firm final labor productivity
Updated in 'Ac'
*/

EQUATION_DUMMY( "AcPreChg", "Ac" )
/*
Weighted-average (single-stage) labor productivity of pre-change firms in
consumption-goods sector
Machine-level productivity is notional (single production stage) and
DO NOT directly represent firm final labor productivity
Updated in 'Ac'
*/

EQUATION_DUMMY( "AsdCposChg", "AsdC" )
/*
Standard deviation of (single-stage) log labor productivity of post-change
firms in consumption-goods sector
Updated in 'AsdC'
*/

EQUATION_DUMMY( "AsdCpreChg", "AsdC" )
/*
Standard deviation of (single-stage) log labor productivity of pre-change
firms in consumption-goods sector
Updated in 'AsdC'
*/

EQUATION_DUMMY( "Gini", "" )
/*
Gini index including workers all income and firm owners net cash flows
Updated in 'InAvgReal'
*/

EQUATION_DUMMY( "InLogSD", "InAvgReal" )
/*
Income dispersion measured by the standard deviation of income log
Updated in 'InAvgReal'
*/

EQUATION_DUMMY( "InMax", "InAvgReal" )
/*
Highest income
Updated in 'InAvgReal'
*/

EQUATION_DUMMY( "InMin", "InAvgReal" )
/*
Lowest income
Updated in 'InAvgReal'
*/

EQUATION_DUMMY( "qCposChg", "" )
/*
Weighted-average productivity of post-change firms in consumption-good sector
Updated in 'qCpreChg'
*/

EQUATION_DUMMY( "wCrealPosChg", "" )
/*
Weighted average real wage paid by post-change consumption-goods firms
Updated in 'wCrealPreChg'
*/

EQUATION_DUMMY( "wGini", "wAvgReal" )
/*
Wage dispersion measured by the Gini index (Jasso-Deaton formula)
Updated in 'wAvgReal'
*/

EQUATION_DUMMY( "wMax", "wAvgReal" )
/*
Highest wage of employed workers
Updated in 'wAvgReal'
*/

EQUATION_DUMMY( "wMin", "wAvgReal" )
/*
Lowest wage of employed workers
Updated in 'wAvgReal'
*/

EQUATION_DUMMY( "wrAvg", "InAvgReal" )
/*
Average wage requested by workers
Updated in 'InAvgReal'
*/

EQUATION_DUMMY( "wrLogSD", "InAvgReal" )
/*
Log wage requested standard deviation
Updated in 'InAvgReal'
*/

EQUATION_DUMMY( "wrMax", "InAvgReal" )
/*
Highest wage requested by workers
Updated in 'InAvgReal'
*/

EQUATION_DUMMY( "wrMin", "InAvgReal" )
/*
Lowest wage requested by workers
Updated in 'InAvgReal'
*/

EQUATION_DUMMY( "wsAvg", "InAvgReal" )
/*
Average satisficing wage of workers
Updated in 'InAvgReal'
*/

EQUATION_DUMMY( "wsLogSD", "InAvgReal" )
/*
Log satisficing wage standard deviation
Updated in 'InAvgReal'
*/

EQUATION_DUMMY( "wsMax", "InAvgReal" )
/*
Highest satisficing wage of workers
Updated in 'InAvgReal'
*/

EQUATION_DUMMY( "wsMin", "InAvgReal" )
/*
Lowest satisficing wage of workers
Updated in 'InAvgReal'
*/

EQUATION_DUMMY( "_nG", "Kinst" )
/*
Number of installed machines of technological generation (paradigm)
*/
