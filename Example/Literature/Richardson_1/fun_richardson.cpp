//#define NO_POINTER_INIT	// uncomment to disable pointer checking

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
	ADDOBJ("Firm");				// add missing objects

i = 1;
CYCLE(cur, "Firm")				// set initial conditions (t=0)
{
	WRITELS(cur, "Convenience", v[1], 0);
	WRITELS(cur, "Quality", v[2], 0);
	WRITELS(cur, "Prob", 0, 0);
	WRITELS(cur, "sigma", 0, 0);
	WRITELS(cur, "ms", 1 / v[0], 0);
}

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
v[4] = VL("sigma", 1);

v[5] = norm(v[2], v[4]);				// innovation potential

v[6] = v[0] * v[2] + (1 - v[0]) * v[3];	// current probability
v[7] = v[0] * v[5] + (1 - v[0]) * v[1];	// innovation probability

if (v[6] < v[7])	// does innovation increase probability?
	v[8] = v[5];	// yes - adopt innovation
else
 	v[8] = v[2];	// no - keep current product quality
 
RESULT(v[8])


EQUATION("sigma")
/*
Accumulated knowledge
*/

v[0] = VL("Quality", 1);
v[1] = V("Quality");

if (v[1] == v[0])			// if there was no innovation
{
	v[2] = VL("sigma", 1);
	v[3] = V("delta");
	
	v[4] = v[2] + v[3];		// increase knowledge
}
else
 	v[4] = 0; 				// if not, reset knowledge
 	
RESULT(v[4])


EQUATION("Convenience")
/*
Product convenience (inverse of price)
*/

v[0] = VL("Quality", 1);
v[1] = V("Quality");

if (v[1] == v[0])			// if there was no innovation
{
	v[2] = V("tau");
	v[3] = V("Cmax");
	v[4] = VL("Convenience", 1);
	v[5] = VL("ms", 1);
	
	v[6] = v[4] * (1 - v[2] * v[5]) + v[3] * v[2] * v[5];
}
else
	v[6] = V("Cmin"); 

RESULT(v[6])


EQUATION("ms")
/*
Market share of a firm
*/

v[0] = V("Prob");
v[1] = V("TotProb");

if (v[1] != 0)				// handle all prob's zero
	v[2] = v[0] / v[1];
else
	v[2] = 0;

RESULT(v[2])


EQUATION("TotProb")
/*
Sum of probabilities of all firms
*/

RESULT(SUM("Prob"))


EQUATION("HHI")
/*
Herfindhal-Hirschman Index
Sum of the squared firm market shares 
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


void close_sim( void )
{
}
