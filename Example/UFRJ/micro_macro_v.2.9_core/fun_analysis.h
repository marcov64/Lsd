/*****ANALYSIS*****/

EQUATION("P")
/*
Price Index
*/
cur = SEARCHS(root, "MACRO");
v[0]=VS(cur, "Price_Index");
RESULT(v[0])


EQUATION("P_G")
/*
Inflation
*/
cur = SEARCHS(root, "MACRO");
v[0]=VS(cur, "Annual_Inflation");
RESULT(v[0])


EQUATION("U")
/*
Unemployment
*/
cur = SEARCHS(root, "MACRO");
v[0]=VS(cur, "Unemployment");
RESULT(v[0])


EQUATION("EMP")
/*
Employment
*/
cur = SEARCHS(root, "SECTORS");
v[0]=SUMS(cur, "Sector_Employment");
RESULT(v[0])


EQUATION("GDP_G")
/*
GDP real growth rate
*/
cur = SEARCHS(root, "MACRO");
v[0]=VS(cur, "Annual_Real_Growth");
RESULT(v[0])


EQUATION("G_n")
/*
GDP nominal growth rate
*/
cur = SEARCHS(root, "MACRO");
v[0]=VS(cur, "Annual_Growth");
RESULT(v[0])


EQUATION("Cri")
/*
Crisis counters
*/
cur = SEARCHS(root, "MACRO");
v[0]=VS(cur, "Likelihood_Crisis");
RESULT(v[0])


EQUATION("C")
/*
Quarterly Nominal Consumption
*/
cur = SEARCHS(root, "MACRO");
v[0]=VS(cur, "Total_Consumption");
RESULT(v[0])


EQUATION("C_r")
/*
Quarterly Real Consumption
*/
cur = SEARCHS(root, "MACRO");
v[0]=VS(cur, "Total_Consumption");
v[1]=VS(cur, "Price_Index");
v[2]=v[0]/v[1];
RESULT(v[2])


EQUATION("CON_G")
/*
Quarterly Real Consumption Growth rate
*/
v[0]=V("C_r");
v[1]=VL("C_r", 1);
if (v[1]!=0)
v[2]=(v[0]-v[1])/v[1];
else
v[2]=0;
RESULT(v[2])


EQUATION("I")
/*
Quarterly Nominal Investment
*/
cur = SEARCHS(root, "MACRO");
v[0]=VS(cur, "Total_Investment");
RESULT(v[0])


EQUATION("I_r")
/*
Quarterly Real Investment
*/
cur = SEARCHS(root, "MACRO");
v[0]=VS(cur, "Total_Investment");
v[1]=VS(cur, "Price_Index");
v[2]=v[0]/v[1];
RESULT(v[2])


EQUATION("INV_G")
/*
Quarterly Real Investment Growth rate
*/
v[0]=V("I_r");
v[1]=VL("I_r", 1);
if (v[1]!=0)
v[2]=(v[0]-v[1])/v[1];
else
v[2]=0;
RESULT(v[2])


EQUATION("PROD")
/*
Average Productivity
*/
cur = SEARCHS(root, "MACRO");
v[0]=VS(cur, "Avg_Productivity");
RESULT(v[0])


EQUATION("PROD_G")
/*
Average Productivity Growth
*/
v[0]=V("PROD");
v[1]=VL("PROD", 1);
if (v[1]!=0)
v[2]=(v[0]-v[1])/v[1];
else
v[2]=0;
RESULT(v[2])


EQUATION("MK")
/*
Average Markup
*/
cur = SEARCHS(root, "MACRO");
v[0]=VS(cur, "Avg_Markup");
RESULT(v[0])


EQUATION("MK_G")
/*
Average Markup Growth
*/
v[0]=V("MK");
v[1]=VL("MK", 1);
if (v[1]!=0)
v[2]=(v[0]-v[1])/v[1];
else
v[2]=0;
RESULT(v[2])


EQUATION("INVE_r")
/*
Real Aggregate Inventories
*/
cur = SEARCHS(root, "MACRO");
v[0]=VS(cur, "Total_Inventories");
v[1]=VS(cur, "Price_Index");
v[2]=v[0]/v[1];
RESULT(v[2])


EQUATION("INVE_G")
/*
Real Aggregate Inventories Growth
*/
v[0]=V("INVE_r");
v[1]=VL("INVE_r", 1);
if (v[1]!=0)
v[2]=(v[0]-v[1])/v[1];
else
v[2]=0;
RESULT(v[2])


EQUATION("K_r")
/*
Real Stock of Capital
*/
cur = SEARCHS(root, "MACRO");
v[0]=VS(cur, "Total_Capital_Stock");
v[1]=VS(cur, "Price_Index");
v[2]=v[0]/v[1];
RESULT(v[2])


EQUATION("K_G")
/*
Real Stock of Capital Growth
*/
v[0]=V("K_r");
v[1]=VL("K_r", 1);
if (v[1]!=0)
v[2]=(v[0]-v[1])/v[1];
else
v[2]=0;
RESULT(v[2])


EQUATION("PROFITS")
/*
Real Surplus
*/
cur = SEARCHS(root, "MACRO");
v[0]=VS(cur, "Total_Profits");
v[1]=VS(cur, "Price_Index");
v[2]=v[0]/v[1];
RESULT(v[2])


EQUATION("WAGE")
/*
Real Wages
*/
cur = SEARCHS(root, "MACRO");
v[0]=VS(cur, "Total_Wages");
v[1]=VS(cur, "Price_Index");
v[2]=v[0]/v[1];
RESULT(v[2])


EQUATION("PROFITS_G")
/*
Real Surplus Growth rate
*/
v[0]=V("PROFITS");
v[1]=VL("PROFITS", 1);
if (v[1]!=0)
v[2]=(v[0]-v[1])/v[1];
else
v[2]=0;
RESULT(v[2])


EQUATION("WAGE_G")
/*
Real Wages growth rate
*/
v[0]=V("WAGE");
v[1]=VL("WAGE", 1);
if (v[1]!=0)
v[2]=(v[0]-v[1])/v[1];
else
v[2]=0;
RESULT(v[2])


EQUATION("G_r")
/*
Quarterly Real Government Expenses
*/
cur = SEARCHS(root, "GOVERNMENT");
v[0]=VS(cur, "Government_Wages");
cur1 = SEARCHS(root, "MACRO");
v[1]=VS(cur1, "Price_Index");
v[2]=v[0]/v[1];
RESULT(v[2])


EQUATION("GOV_G")
/*
Quarterly Real Government Expenses Growth rate
*/
v[0]=V("G_r");
v[1]=VL("G_r", 1);
if (v[1]!=0)
v[2]=(v[0]-v[1])/v[1];
else
v[2]=0;
RESULT(v[2])


EQUATION("PDEBT")
/*
Public Debt
*/
cur = SEARCHS(root, "GOVERNMENT");
v[0]=VS(cur, "Government_Debt");
RESULT(v[0])


EQUATION("PDEBT_G")
/*
Public Debt Growth rate
*/
v[0]=V("PDEBT");
v[1]=VL("PDEBT", 1);
if (v[1]!=0)
v[2]=(v[0]-v[1])/v[1];
else
v[2]=0;
RESULT(v[2])


EQUATION("M_r")
/*
Quarterly Real Imports
*/
cur = SEARCHS(root, "MACRO");
v[0]=VS(cur, "Total_Imports");
v[1]=VS(cur, "Price_Index");
v[2]=v[0]/v[1];
RESULT(v[2])


EQUATION("M_G")
/*
Quarterly Real Imports Growth rate
*/
v[0]=V("M_r");
v[1]=VL("M_r", 1);
if (v[1]!=0)
v[2]=(v[0]-v[1])/v[1];
else
v[2]=0;
RESULT(v[2])


EQUATION("X_r")
/*
Quarterly Real Exports
*/
cur = SEARCHS(root, "MACRO");
v[0]=VS(cur, "Total_Exports");
v[1]=VS(cur, "Price_Index");
v[2]=v[0]/v[1];
RESULT(v[2])


EQUATION("X_G")
/*
Quarterly Real Exports Growth rate
*/
v[0]=V("X_r");
v[1]=VL("X_r", 1);
if (v[1]!=0)
v[2]=(v[0]-v[1])/v[1];
else
v[2]=0;
RESULT(v[2])


EQUATION("NX_r")
/*
Quarterly Real Net Exports
*/
v[0]=V("X_r");
v[1]=V("M_r");
v[2]=v[0]-v[1];
RESULT(v[2])


EQUATION("NX_G")
/*
Quarterly Real Net Exports Growth rate
*/
v[0]=V("NX_r");
v[1]=VL("NX_r", 1);
if (v[1]!=0)
v[2]=(v[0]-v[1])/v[1];
else
v[2]=0;
RESULT(v[2])


EQUATION("DEBT")
/*
Firms' Stock of Debt
*/
v[0]=0;
CYCLES(root, cur, "SECTORS")
{
	v[1]=0;
	CYCLES(cur, cur1, "FIRMS")
	{
		v[2]=VS(cur1, "Firm_Stock_Debt");
		v[1]=v[1]+v[2];
	}
v[0]=v[0]+v[1];
}
RESULT(v[0])


EQUATION("DEBT_G")
/*
Firms' Debt Growth rate
*/
v[0]=V("DEBT");
v[1]=VL("DEBT", 1);
if (v[1]!=0)
v[2]=(v[0]-v[1])/v[1];
else
v[2]=0;
RESULT(v[2])
