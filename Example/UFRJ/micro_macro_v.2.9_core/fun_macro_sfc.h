/*****STOCK-FLOW CONSISTENCY*****/


EQUATION("SFC_Total_Variation_Financial_Assets_Classes")
	v[0]=SUM("Class_Financial_Assets");
	v[1]=SUML("Class_Financial_Assets", 1);
	v[2]=v[0]-v[1];
RESULT(v[2])

EQUATION("SFC_Total_Variation_Debt_Classes")
	v[0]=SUM("Class_Debt");
	v[1]=SUML("Class_Debt", 1);
	v[2]=v[0]-v[1];
RESULT(v[2])

EQUATION("SFC_Total_Financial_Assets_Return_Classes")
	v[0]=SUM("Class_Financial_Asset_Return");
RESULT(v[0])

EQUATION("SFC_Total_Interest_Payments_Classes")
	v[0]=SUML("Class_Debt",1);
	v[1]=V("Basic_Interest_Rate");
	v[2]=v[0]*v[1];
RESULT(v[2])

EQUATION("SFC_Total_Variation_Financial_Assets_Firms")
	v[0]=0;
	v[1]=V("Basic_Interest_Rate");
	CYCLE(cur, "SECTORS")
	{
		v[2]=0;
		CYCLES(cur, cur1, "FIRMS")
		{
			v[3]=VLS(cur1,"Firm_Stock_Financial_Assets",1);
			v[2]=v[2]+v[3]*v[1];
		}
	v[0]=v[0]+v[2];
	}
RESULT(v[0])

EQUATION("SFC_Total_Interest_Payments_Firms")
	v[0]=0;
	CYCLE(cur, "SECTORS")
	{
		v[1]=0;
		CYCLES(cur, cur1, "FIRMS")
		{
			v[2]=VS(cur1,"Firm_Interest_Rate");
			v[3]=VLS(cur1,"Firm_Stock_Debt",1);
			v[1]=v[1]+v[2]*v[3];
		}
	v[0]=v[0]+v[1];
	}
RESULT(v[0])

EQUATION("SFC_Total_Variation_Debt_Firms")
	v[0]=0;
	CYCLE(cur, "SECTORS")
	{
		v[1]=0;
		CYCLES(cur, cur1, "FIRMS")
		{
			v[2]=VS(cur1,"Firm_Debt_Flow");
			v[1]=v[1]+v[2];
		}
	v[0]=v[0]+v[1];
	}
RESULT(v[0])

EQUATION("SFC_Total_Financial_Assets_Return_Firms")
	v[0]=0;
	CYCLE(cur, "SECTORS")
	{
		v[1]=0;
		CYCLES(cur, cur1, "FIRMS")
		{
   	v[2]=VS(cur1,"Firm_Financial_Assets_Flow");
   	v[1]=v[1]+v[2];
		}
	v[0]=v[0]+v[1];
	}
RESULT(v[0])

EQUATION("SFC_Net_Rentability_Financial_Assets")
	v[0]=0;
	CYCLE(cur, "SECTORS")
	{
		v[1]=0;
		CYCLES(cur, cur1, "FIRMS")
		{
			v[2]=VS(cur1, "Firm_Financial_Assets_Return");
			v[1]=v[1]+v[2];
		}
	v[0]=v[0]+v[1];
	}
RESULT(v[0])

EQUATION("SFC_Financial_Sector_Wealth")
	v[0]=V("SFC_Total_Financial_Assets_Return_Classes");
	v[1]=V("SFC_Total_Financial_Assets_Return_Firms");
	v[2]=V("SFC_Total_Interest_Payments_Firms");
	v[3]=V("SFC_Total_Interest_Payments_Classes");
	v[4]=V("Government_Interest_Payment");
	v[5]=v[2]+v[3]+v[4]-v[0]-v[1];
RESULT(v[5])

EQUATION("SFC_Financial_Sector_Wealth_2")
	v[0]=V("SFC_Total_Variation_Debt_Classes");
	v[1]=V("SFC_Total_Variation_Debt_Firms");
	v[2]=V("Government_Deficit");
	v[3]=V("SFC_Total_Variation_Financial_Assets_Classes");
	v[4]=V("SFC_Total_Variation_Financial_Assets_Firms");
	v[5]=v[0]+v[1]+v[2]-v[3]-v[4];
RESULT(v[5])

EQUATION("SFC_Total_Savings")
v[0]=SUM("Class_Savings");
RESULT(v[0])

EQUATION("SFC_Total_Funds")
v[0]=0;
CYCLE(cur, "SECTORS")
{
	v[1]=0;
	CYCLES(cur, cur1, "FIRMS")
	{
		v[2]=VS(cur1, "Firm_Retained_Profits");
		v[1]=v[1]+v[2];
	}
v[0]=v[0]+v[1];
}
RESULT(v[0])




