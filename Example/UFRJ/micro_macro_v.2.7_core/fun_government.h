/*****PUBLIC SECTOR*****/

EQUATION("Max_Government_Expenses")
/*
Maximum Government expenses imposed by the surplus target fiscal rule.
*/
	v[8]=V("government_period");
	v[7]= fmod((double) t,v[8]);                                    	//divides the time period by government adjustment period (adjust annualy)
	if(v[7]==0)                                                     	//if the rest of the division is zero (adjust maximum expenses)
		{
		v[0]=VL("GDP", 1);                                          	//GDP lagged 1
		v[1]=VL("GDP", v[8]);                                       	//GDP lagged government period (4)
		v[2]=V("gama_gov");                                        		//expectation parameter for macro variables
		if(v[1]!=0)                                                   	//if last semiannual GDP is not zero
			v[3]=1+v[2]*(v[0]-v[1])/v[1];                               //expected growth of gdp
		else                                                          	//if last semiannual GDP is zero
			v[3]=1;                                                     //use one for the expected growth
		v[4]=VL("Total_Taxes",1);                                     	//total taxes in the last period
		v[5]=V("government_surplus_rate_target");                     	//government surplus target rate
		v[6]=v[3]*(v[4]-(v[0]*v[5]));                                 	//maximum expenses will be total taxes multiplyed by expected growth minus the surplus target
		}
	else                                                            	//if the rest of the division is not zero (do not maximum expenses)
		v[6]=VL("Max_Government_Expenses",1);                         	//use last period's
RESULT(v[6])


EQUATION("Government_Wages")
/*
Government spends all with wages
*/
	v[0]=V("Max_Government_Expenses");                    
RESULT(v[0])


EQUATION("Income_Taxes")
/*
Stores the value of the function
*/
	v[0]=V("Income_Taxes_Function");
RESULT(v[0])


EQUATION("Indirect_Taxes")
/*
Stores the value of the function
*/
	v[0]=V("Indirect_Taxes_Function");
RESULT(v[0])


EQUATION("Total_Taxes")
/*
Sum of income and indirect taxes
*/
	v[0]=V("Income_Taxes");
	v[1]=V("Indirect_Taxes");
	v[2]=v[0]+v[1];
RESULT(v[2])


EQUATION("Primary_Surplus")
/*
Total Taxes minus Government Expenses
*/
	v[0]=V("Government_Wages");
	v[1]=V("Total_Taxes");
	v[2]=v[1]-v[0];
RESULT(v[2])


EQUATION("Government_Interest_Payment")
/*
Defined as the government stock of public debt in the last period multipyed by the interest rate
*/
	v[0]=VL("Public_Debt",1);
	v[1]=V("Interest_Rate");
	v[2]=v[0]*v[1];
RESULT(v[2])


EQUATION("Public_Deficit")
/*
Government expenses minus taxes plus interest payments
*/
	v[0]=V("Government_Wages");
	v[1]=V("Total_Taxes");
	v[2]=V("Government_Interest_Payment");
	v[3]=v[0]-v[1]+v[2];
RESULT(v[3])


EQUATION("Public_Debt")
/*
Defined as the stock of debt in the last period plus current government deficit
*/
	v[0]=VL("Public_Debt",1);
	v[1]=V("Public_Deficit");
	v[2]=max(0,v[0]+v[1]);
RESULT(v[2])


EQUATION("Public_Debt_GDP_Ratio")
/*
Public debt over annual GDP
*/
	v[0]=V("annual_period");
	v[1]=0;
	for (v[2]=0; v[2]<=v[0]; v[2]=v[2]+1)
		{
		v[3]=VL("GDP",v[2]);
		v[1]=v[1]+v[3];
		}
	v[4]=V("Public_Debt");
	v[5]=v[4]/v[1];
RESULT(v[5])


EQUATION("Interest_Rate")
/*
Nominal Interest rate is set by the central bank following a (possible) dual mandate Taylor Rule, considering the inflation and unemployment gaps.
*/
	v[0]=V("interest_rate");
	v[1]=V("inflation_target");
	v[2]=V("unemployment_target");
	v[3]=V("Annual_Inflation");
	v[4]=VL("Unemployment", 1);
	v[5]=V("inflation_sensitivity");
	v[6]=V("unemployment_sensitivity");
	v[7]=v[0]+v[5]*(v[3]-v[1])-v[6]*(v[4]-v[2]);
	v[8]=V("interest_rate_adjustment");
	v[9]=VL("Interest_Rate", 1);
	if (abs(v[7]-v[9])>v[8])
		{
		if (v[7]>v[9])
			v[10]=v[9]+v[8];
		else
			v[10]=v[9]-v[8];
		}
	else
		v[10]=v[7];
RESULT(max(0,v[10]))












