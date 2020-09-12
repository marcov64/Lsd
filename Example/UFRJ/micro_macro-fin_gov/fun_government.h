/*****PUBLIC SECTOR*****/

EQUATION("Government_Desired_Wages")
/*
Priority expenses.
If there are no maximum expenses, it is adjusted by average productivity growth and inflation.
*/

v[0]=V("government_period");
v[1]= fmod((double) t,v[0]);                                   //divides the time period by government adjustment period (adjust annualy)
v[2]=VL("Government_Desired_Wages",1);
if(v[1]==0)                                                    //if the rest of the division is zero (adjust government wages)
{
	v[3]=VL("Avg_Productivity",1);                             //avg productivity lagged 1
	v[4]=VL("Avg_Productivity", v[0]);                         //avg productivity lagged government period (4)
	if(v[4]!=0)                                                //if productivity is not zero
		v[5]=(v[3]-v[4])/v[4];                                 //calculate productivity growth
	else                                                       //if productivity is zero
		v[5]=0;												   //use 1
	v[6]=VL("Consumer_Price_Index",1);                         //consumer price index lagged 1
	v[7]=VL("Consumer_Price_Index", v[0]);                     //consumer price index lagged government period (4)
	if(v[7]!=0)                                                //if consumer price index is not zero
		v[8]=(v[6]-v[7])/v[7];                                 //calculate inflation
	else                                                       //if consumer price index is zero
		v[8]=0;												   //use 1
	v[9]=V("government_productivity_passtrought");             //productivity passtrough parameter
	v[10]=v[2]*(1+v[9]*v[5]+v[8]);                             //desired adjusted government wages with no restriction
}
else                                                           //if it is not adjustment period
	v[10]=v[2];                                                //use last period's
RESULT(v[10])


EQUATION("Government_Desired_Unemployment_Benefits")
/*
Counter-cyclical Expenses
Benefit is a share of average wage. 
The amount depends on current unemployment.
*/
v[0]=V("government_period");
v[1]= fmod((double) t,v[0]);                                   //divides the time period by government adjustment period (adjust annualy)
if(v[1]==0)                                                    //if the rest of the division is zero (adjust unemployment benefits)
{
	v[2]=WHTAVES(root, "Sector_Avg_Wage", "Sector_Employment");
	v[3]=SUM("Sector_Employment");
	if(v[3]!=0)
		v[4]=v[2]/v[3];
	else
		v[4]=0;
	
	v[5]=V("government_benefit");
	v[6]=SUMLS(root, "Sector_Potential_Employment",1);
	v[7]=SUMLS(root, "Sector_Employment",1);
	v[8]=v[4]*v[5]*(v[6]-v[7]);
}
else
	v[8]=CURRENT;
RESULT(max(0,v[8]))


EQUATION("Government_Desired_Investment")
/*
Desired Investment Expenses
Adjusted by a desired real growth rate and inflation
*/
v[0]=V("government_period");
v[1]= fmod((double) t,v[0]);                                   //divides the time period by government adjustment period (adjust annualy)
v[2]=VL("Government_Desired_Investment",1);
if(v[1]==0)                                                    //if the rest of the division is zero (adjust unemployment benefits)
{
	v[3]=V("government_demand_growth");
	v[5]=VL("Consumer_Price_Index",1);                         //consumer price index lagged 1
	v[6]=VL("Consumer_Price_Index", v[0]);                     //consumer price index lagged government period (4)
	if(v[6]!=0)                                                //if consumer price index is not zero
		v[7]=(v[5]-v[6])/v[6];                                 //calculate inflation
	else                                                       //if consumer price index is zero
		v[7]=0;												   //use 1
	v[8]=v[2]*(1+v[3]+v[7]);
}
else
	v[8]=v[2];
RESULT(v[8])


EQUATION("Government_Desired_Consumption")
/*
Desired Consumption Expenses
Adjusted by a desired real growth rate and inflation
*/
v[0]=V("government_period");
v[1]= fmod((double) t,v[0]);                                   //divides the time period by government adjustment period (adjust annualy)
v[2]=VL("Government_Desired_Consumption",1);
if(v[1]==0)                                                    //if the rest of the division is zero (adjust unemployment benefits)
{
	v[3]=V("government_demand_growth");
	v[5]=VL("Consumer_Price_Index",1);                         //consumer price index lagged 1
	v[6]=VL("Consumer_Price_Index", v[0]);                     //consumer price index lagged government period (4)
	if(v[6]!=0)                                                //if consumer price index is not zero
		v[7]=(v[5]-v[6])/v[6];                                 //calculate inflation
	else                                                       //if consumer price index is zero
		v[7]=0;												   //use 1
	v[8]=v[2]*(1+v[3]+v[7]);
}
else
	v[8]=v[2];
RESULT(v[8])


EQUATION("Government_Desired_Inputs")
/*
Desired Intermediate Expenses
Adjusted by a desired real growth rate and inflation
*/
v[0]=V("government_period");
v[1]= fmod((double) t,v[0]);                                   //divides the time period by government adjustment period (adjust annualy)
v[2]=VL("Government_Desired_Inputs",1);
if(v[1]==0)                                                    //if the rest of the division is zero (adjust unemployment benefits)
{
	v[3]=V("government_demand_growth");
	v[5]=VL("Consumer_Price_Index",1);                         //consumer price index lagged 1
	v[6]=VL("Consumer_Price_Index", v[0]);                     //consumer price index lagged government period (4)
	if(v[6]!=0)                                                //if consumer price index is not zero
		v[7]=(v[5]-v[6])/v[6];                                 //calculate inflation
	else                                                       //if consumer price index is zero
		v[7]=0;												   //use 1
	v[8]=v[2]*(1+v[3]+v[7]);
}
else
	v[8]=v[2];
RESULT(v[8])


EQUATION("Government_Surplus_Rate_Target")
/*
Adjusts government surplus target based on debt to gdp evolution
*/
v[0]=V("government_period");
v[1]= fmod((double) t,v[0]);                                   //divides the time period by government adjustment period (adjust annualy)
v[2]=VL("Government_Surplus_Rate_Target",1);                   //last period's target
if(v[1]==0)                                                    //if the rest of the division is zero (adjust unemployment benefits)
{
	v[3]=VL("Government_Debt_GDP_Ratio",1);                    //current debt to gdp ratio
	v[4]=V("government_max_debt");                             //maximum debt to gdp accepted, parameter
	v[5]=V("government_min_debt");                             //minimum debt to gdp accepted, parameter
	v[6]=V("government_surplus_target_adjustment");			   //adjustment parameter
	if(v[3]>v[4])                                              //if debt to gdp is higher than accepted
		v[7]=v[2]+v[6];										   //increase surplus target
	else if (v[3]<v[5])                                        //if debt to gdp is lower than accepted
		v[7]=v[2]-v[6];										   //deacrease surplus target
	else                                                       //if current debt to gdp is between accepted band
		v[7]=v[2];                                             //do not change surplus taget
}
else                                                           //if it is not adjustment period
	v[7]=v[2];                                                 //do not change surplus taget
RESULT(v[7])


EQUATION("Government_Max_Expenses")
/*
Maximum Government expenses imposed by the fiscal rule.
Fiscal rules can be two types: primary surplus target or expenses ceiling (or both).
Depend on the policy parameter.
*/

v[0]=V("begin_surplus_target_rule");                           //define when surplus target rule begins
v[1]=V("begin_expenses_ceiling_rule");                         //define when expenses ceiling begins
v[2]=V("government_period");
v[3]= fmod((double) t,v[2]);                                   //divides the time period by government adjustment period (adjust annualy)
if(v[3]==0)                                                    //if the rest of the division is zero (adjust maximum expenses)
{                                                              //adjust fiscal rules maximum expenses
		v[4]=VL("GDP", 1);                                          	//GDP lagged 1
		v[5]=VL("GDP", v[2]);                                       	//GDP lagged government period (4)
		v[6]=V("government_expectations");                              //government expectation parameter 
		if(v[5]!=0)                                                   	//if last semiannual GDP is not zero
			v[7]=1+v[6]*(v[4]-v[5])/v[5];                               //expected growth of gdp
		else                                                          	//if last semiannual GDP is zero
			v[7]=1;                                                     //use one for the expected growth
		v[8]=VL("Total_Taxes",1);                                     	//total taxes in the last period
		v[9]=V("Government_Surplus_Rate_Target");                     	//government surplus target rate
		v[10]=v[7]*(v[8]-(v[8]*v[9]));                                 	//maximum expenses will be total taxes multiplyed by expected growth minus the surplus target
	
		v[11]=VL("Government_Effective_Expenses", 1);                   //last period government expeneses
		v[12]=VL("Consumer_Price_Index",1);                             //consumer price index lagged 1
		v[13]=VL("Consumer_Price_Index", v[2]);                         //consumer price index lagged government period (4)
		if(v[13]!=0)                                                    //if consumer price index is not zero
			v[14]=1+((v[12]-v[13])/v[13]);                              //calculate inflation
		else                                                            //if consumer price index is zero
			v[14]=1;												    //use 1
		v[15]=v[11]*v[14];												//maximum expenses if ceiling rule is active
	
	if((t>=v[0]&&v[0]!=-1)&&(t>=v[1]&&v[1]!=-1))                        //if both rules are active
		v[16]=min(v[10],v[15]);                                         //effective maximum is the minimum between both rules
	else if ((t>=v[0]&&v[0]!=-1)&&(t<v[1]||v[1]==-1))                   //if only surplus target is active
		v[16]=v[10];
	else if ((t>=v[1]&&v[1]!=-1)&&(t<v[0]||v[0]==-1))                   //if only expenses ceiling is active
		v[16]=v[15];
	else													            //if none of the rules are active
		v[16]=-1; 											            //no maximum expenses
}
else	                                                                //if it is not adjustment period
	v[16]=CURRENT;											            //use last period's maximum
RESULT(v[16])


EQUATION("Government_Effective_Expenses")
/*
Priority expenses.
If there are no maximum expenses, it is adjusted by average productivity growth and inflation.

switch_government_composition: Determines the composition and priority of government expenses, and initial distribution
0--> Government Wages(100%) [Only Public Workers]
1--> Government Wages(100%) + Unemployment Benefits(0%) [Public Workers and Unemployment Benefits]
2--> Government Wages(70%) + Unemployment Benefits(0%) + Government Consumption, Investment and Intermediate Demand(10% each) [Public Workers, Unemployment Benefits, and Demand to Sectors]
*/

v[0]=V("Government_Max_Expenses");
v[1]=V("Government_Desired_Wages");
v[2]=V("Government_Desired_Unemployment_Benefits");
v[3]=V("Government_Desired_Consumption");
v[4]=V("Government_Desired_Investment");
v[5]=V("Government_Desired_Inputs");

v[7]=V("switch_government_composition");

if(v[0]==-1)                                                   //no fiscal rule
{
	if(v[7]==0)												   //if only government wages
	{
		v[8]=v[1];											   //government wages equal desired wages
		v[9]=0;    											   //government unemployment benefits equal 0
		v[10]=0;                                               //government consumption equal 0
		v[11]=0;                                               //government investment equal 0
		v[12]=0;                                               //government intermediate demand equals 0
	}
	if(v[7]==1)                                                //if only wages and unemployment benefits
	{
		v[8]=v[1];											   //government wages equal desired wages
		v[9]=v[2];    										   //government unemployment benefits equal 0
		v[10]=0;                                               //government consumption equal 0
		v[11]=0;                                               //government investment equal 0
		v[12]=0;                                               //government intermediate demand equals 0
	}
	if(v[7]==2)                                                //if only wages, unemployment benefits and consumption
	{
		v[8]=v[1];											   //government wages equal desired wages
		v[9]=v[2];    										   //government unemployment benefits equal 0
		v[10]=v[3];                                            //government consumption equal desired
		v[11]=v[4];                                            //government investment equal desired
		v[12]=v[5];                                            //government intermediate demand equals desired
	}
}
else
{
	if(v[7]==0)												   //if only government wages
	{
		v[8]=min(v[0],v[1]);								   //government wages is desired limited by maximum expenses
		v[9]=0;    											   //government unemployment benefits equal 0
		v[10]=0;                                               //government consumption equal 0
		v[11]=0;                                               //government investment equal 0
		v[12]=0;                                               //government intermediate demand equals 0
	}
	if(v[7]==1)                                                //if only wages and unemployment benefits
	{
		v[8]=min(v[0],v[1]);								   //government wages is desired limited by maximum expenses
		v[9]=min(v[2],(v[0]-v[8]));    						   //government unemployment benefits is desired limited by maximum expenses minus wages
		v[10]=0;                                               //government consumption equal 0
		v[11]=0;                                               //government investment equal 0
		v[12]=0;                                               //government intermediate demand equals 0
	}
	if(v[7]==2)                                                //if only wages, unemployment benefits and consumption
	{
		v[8]=min(v[0],v[1]);								   //government wages is desired limited by maximum expenses
		v[9]=min(v[2],(v[0]-v[8]));    						   //government unemployment benefits is desired limited by maximum expenses minus wages
		v[10]=min(v[3],(v[0]-v[8]-v[9])/3);                    //government consumption is desired limited by maximum expenses minus wages and benefits
		v[11]=min(v[4],(v[0]-v[8]-v[9])/3);                    //government investment is desired limited by maximum expenses minus wages and benefits
		v[12]=min(v[5],(v[0]-v[8]-v[9])/3);                    //government intermediate is desired limited by maximum expenses minus wages and benefits
	}
	
}
WRITE("Government_Effective_Wages", v[8]);
WRITE("Government_Effective_Unemployment_Benefits", v[9]);
WRITE("Government_Effective_Consumption", v[10]);
WRITE("Government_Effective_Investment", v[11]);
WRITE("Government_Effective_Inputs", v[12]);
v[13]=v[8]+v[9]+v[10]+v[11]+v[12];
RESULT(v[13])

EQUATION_DUMMY("Government_Effective_Wages","Government_Effective_Expenses")
EQUATION_DUMMY("Government_Effective_Unemployment_Benefits","Government_Effective_Expenses")
EQUATION_DUMMY("Government_Effective_Consumption","Government_Effective_Expenses")
EQUATION_DUMMY("Government_Effective_Investment","Government_Effective_Expenses")
EQUATION_DUMMY("Government_Effective_Inputs","Government_Effective_Expenses")


EQUATION("Total_Income_Taxes")
/*
Stores the value of the function
*/
	v[0]=SUM("Class_Taxation");
RESULT(v[0])


EQUATION("Total_Indirect_Taxes")
/*
Stores the value of the function
*/
	v[0]=SUM("Sector_Indirect_Tax");
RESULT(v[0])


EQUATION("Total_Taxes")
/*
Sum of income and indirect taxes
*/
	v[0]=V("Total_Income_Taxes");
	v[1]=V("Total_Indirect_Taxes");
	v[2]=v[0]+v[1];
RESULT(v[2])


EQUATION("Government_Primary_Surplus")
/*
Total Taxes minus Government Expenses
*/
	v[0]=V("Government_Effective_Expenses");
	v[1]=V("Total_Taxes");
	v[2]=v[1]-v[0];
RESULT(v[2])


EQUATION("Government_Interest_Payment")
/*
Defined as the government stock of public debt in the last period multiplyed by the interest rate
*/
	v[0]=VL("Government_Debt",1);
	v[1]=V("Basic_Interest_Rate");
	v[2]=v[0]*v[1];
RESULT(v[2])


EQUATION("Government_Nominal_Deficit")
/*
Government expenses minus taxes plus interest payments
*/
	v[0]=V("Government_Effective_Expenses");
	v[1]=V("Total_Taxes");
	v[2]=V("Government_Interest_Payment");
	v[3]=v[0]-v[1]+v[2];
RESULT(v[3])


EQUATION("Government_Debt")
/*
Defined as the stock of debt in the last period plus current government deficit
*/
	v[0]=VL("Government_Debt",1);
	v[1]=V("Government_Nominal_Deficit");
	v[2]=max(0,v[0]+v[1]);
RESULT(v[2])


EQUATION("Government_Debt_GDP_Ratio")
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
	v[4]=V("Government_Debt");
	if (v[1]!=0)
		v[5]=v[4]/v[1];
	else	
		v[5]=0;
RESULT(v[5])















