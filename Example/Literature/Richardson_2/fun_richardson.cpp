#define NO_POINTER_INIT			// comment to enable pointer checking

#include "fun_head_fast.h"


MODELBEGIN


EQUATION("Init")
/*
Model initialization
*/

v[0] = V("N");
v[1] = V("C0");
v[2] = V("Q0");
v[3] = COUNT("Firm");

for (i = v[3] + 1; i <= v[0]; ++i)
	ADDOBJ("Firm");				// add missing firm objects

i = 1;
CYCLE(cur, "Firm")				// set initial conditions (t=0)
{
	WRITES(cur, "Id", i++);
	WRITELS(cur, "Convenience", v[1], 0);
	WRITELS(cur, "Quality", v[2], 0);
	WRITELS(cur, "Sales", 0, 0);
	WRITELS(cur, "Prob", 0, 0);
	WRITELS(cur, "Clients", 0, 0);
	WRITELS(cur, "sigma", 0, 0);
	WRITELS(cur, "ms", 1 / v[0], 0);
}

v[4] = V("M");
v[5] = COUNT("Consumer");

for (j = v[5] + 1; j <= v[4]; ++j)
	ADDOBJ("Consumer");			// add missing consumer objects
	
CYCLE(cur, "Consumer")			// asign current product used
	WRITELS(cur, "Product", uniform_int(1, v[0]), 0);

PARAMETER;

RESULT(1)


EQUATION("Prob")
/*
Probability of firm being chosen
*/

v[0] = V("alpha");
v[1] = V("Quality");
v[2] = V("Convenience");

v[3] = v[0] * v[1] + (1 - v[0]) * v[2];

RESULT(v[3])


EQUATION("Quality")
/*
New product quality, possibly obtained by innovation
*/

v[0] = V("alpha");
v[1] = V("Cmin");
v[2] = VL("Quality", 1);
v[3] = VL("Convenience", 1);
v[4] = V("sigma");

v[5] = norm(v[1], v[4]);				// innovation potential

v[6] = v[0] * v[2] + (1 - v[0]) * v[3];	// current probability
v[7] = v[0] * v[5] + (1 - v[0]) * v[1];	// innovation probability

if (v[6] < v[7])		// does innovation increase probability?
{						// yes - adopt innovation
	WRITE("Convenience", v[1]);
	WRITE("sigma", 0);
	v[8] = v[5];
}
else
	v[8] = v[2];		// no - keep current product quality

RESULT(v[8])


EQUATION("sigma")
/*
Accumulated knowledge
*/

v[0] = V("delta");
v[1] = VL("sigma", 1);

v[2] = v[1] + v[0];

RESULT(v[2])


EQUATION("Convenience")
/*
Product convenience (inverse of price)
*/

v[0] = V("tau");
v[1] = V("Cmax");
v[2] = VL("Convenience", 1);
v[3] = VL("ms", 1);

v[4] = v[2] * (1 - v[0] * v[3]) + v[1] * v[0] * v[3];

RESULT(v[4])


EQUATION("ms")
/*
Market share of a firm
*/

V("Account");					// make sure accounting is done

v[0] = V("M"); 					// number of consumers
v[1] = V("Clients");			// own customers

v[2] = v[1] / v[0];

RESULT(v[2])


EQUATION("TotSales")
/*
Total market sales
*/

RESULT(SUM("Sales"))


EQUATION("Account")
/*
Compute the number of customers
Also clear the period sales
*/

CYCLE(cur, "Firm")				// reset firm data
{
	WRITES(cur, "Clients", 0);
	WRITES(cur, "Sales", 0);
}

CYCLE(cur, "Consumer")			// count sales and clients
{
	v[0] = VS(cur, "Product");
	cur1 = SEARCH_CND("Id", v[0]);
	INCRS(cur1, "Clients", 1);
}

RESULT(1)


EQUATION_DUMMY("Clients", "Account")
/*
Number os consumers with products from firm in use
Updated in 'Accounting"
*/


EQUATION("Buy")
/*
Function in returning a random product
*/

v[0] = V("TotProb");

cur = RNDDRAW_TOT("Firm", "Prob", v[0]);// choose randomly one firm
v[1] = VS(cur, "Id");			// record the id. number of the firm
INCRS(cur, "Sales", 1);			// increment firm sales

RESULT(v[1])


EQUATION_DUMMY("Sales", "")
/*
Firm sales in period
Updated in 'Accounting' and 'Buy'
*/


EQUATION("Product")
/*
Identification number of product used in period, either buying it 
if broken or using the previously used one otherwise
*/

v[0] = VL("Product", 1);		// look at the product used
cur = SEARCH_CND("Id", v[0]);	// pointer to current firm
v[1] = VS(cur, "beta");			// probability of product failure

if (RND < v[1])					// Does product broke down?
	v[2] = V("Buy");			// yes - a new one must be bought
else
	v[2] = v[0];				// no - the product is still working
 
RESULT(v[2])


EQUATION("TotProb")
/*
Sum of probabilities of all firms
*/

RESULT(SUM("Prob"))


EQUATION("HHI")
/*
Herfindhal-Hirschman Index
*/

v[0] = 0;
CYCLE(cur, "Firm")
{
	v[1] = VS(cur, "ms");
	v[0] += v[1] * v[1];
}

RESULT(v[0])


EQUATION("HPI")
/*
Hymer & Pashigian Instability Index
Sum of the absolute changes in firm market shares
*/

v[0] = 0;
CYCLE(cur, "Firm")
{
	v[1] = VLS(cur, "ms", 1);
	v[2] = VS(cur, "ms");
	v[0] += abs(v[2] - v[1]);
}

RESULT(v[0])


MODELEND


void close_sim(void)
{
}

