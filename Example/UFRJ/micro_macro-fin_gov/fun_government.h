/*****GOVERNMENT DECISION VARIABLES*****/

EQUATION("Government_Desired_Wages")
/*
Priority expenses.
If there are no maximum expenses, it is adjusted by average productivity growth and inflation.
*/                                     
	v[0]= LAG_GROWTH(consumption, "Sector_Avg_Price", 1, 1);		   		
	v[1]=V("government_real_growth");
	v[2]=CURRENT*(1+v[0]+v[1]);
RESULT(max(0,v[2]))


EQUATION("Government_Desired_Unemployment_Benefits")
/*
Counter-cyclical Expenses
Benefit is a share of average wage. 
The amount depends on current unemployment.
*/
	v[0]=V("government_benefit_rate");
	v[1]=0;
	CYCLES(country, cur, "SECTORS")
	{
		v[2]=VS(cur, "sector_desired_degree_capacity_utilization");
		v[3]=VLS(cur, "Sector_Idle_Capacity", 1);
		v[4]=v[3]-(1-v[2]);
		v[5]=VLS(cur, "Sector_Productive_Capacity", 1);
		v[6]=v[4]*v[5];
		v[7]=VLS(cur, "Sector_Avg_Productivity", 1);
		v[8]=VLS(cur, "Sector_Avg_Wage", 1);
		v[9]=v[6]*(v[0]*v[8]/v[7]);
		v[1]=v[1]+v[9];
	}
	
	v[1]=0;
	CYCLES(country, cur, "SECTORS")
	{
		v[2]=VLS(cur, "Sector_Employment",1);
		v[3]=VLS(cur, "Sector_Employment",2);
		v[4]=-(v[2]-v[3]);
		v[5]=max(0, v[4]);
		v[8]=VLS(cur, "Sector_Avg_Wage", 1);
		v[9]=v[5]*(v[0]*v[8]);
		v[1]=v[1]+v[9];
	}	
RESULT(max(0,v[1]))


EQUATION("Government_Desired_Investment")
/*
Desired Investment Expenses
Adjusted by a desired real growth rate and avg capital price growth
*/
	v[0]=V("government_real_growth");		
	v[1]=LAG_GROWTH(capital, "Sector_Avg_Price", 1, 1);
	v[2]=CURRENT*(1+v[0]+v[1]);
RESULT(max(0,v[2]))


EQUATION("Government_Desired_Consumption")
/*
Desired Consumption Expenses
Adjusted by a desired real growth rate and avg consumption price growth
*/
	v[0]=V("government_real_growth");   
	v[1]= LAG_GROWTH(consumption, "Sector_Avg_Price", 1, 1);
	v[2]=CURRENT*(1+v[0]+v[1]);
RESULT(max(0,v[2]))


EQUATION("Government_Desired_Inputs")
/*
Desired Intermediate Expenses
Adjusted by a desired real growth rate and avg input price growth
*/
	v[0]=V("government_real_growth");      
	v[1]=LAG_GROWTH(input, "Sector_Avg_Price", 1, 1);
	v[2]=CURRENT*(1+v[0]+v[1]);
RESULT(max(0,v[2]))


/*****FISCAL RULES VARIABLES*****/


EQUATION("Government_Surplus_Rate_Target")
/*
Adjusts government surplus target based on debt to gdp evolution
*/
	v[0]=V("government_max_surplus_target");                     
	v[1]=V("government_min_surplus_target");
	v[2]=CURRENT;                   						   //last period's target
	v[3]=VL("Government_Debt_GDP_Ratio",1);                    //current debt to gdp ratio
	v[8]=VL("Government_Debt_GDP_Ratio",2);
	v[4]=V("government_max_debt_ratio");                       //maximum debt to gdp accepted, parameter
	v[5]=V("government_min_debt_ratio");                       //minimum debt to gdp accepted, parameter
	v[6]=V("government_surplus_target_adjustment");			   //adjustment parameter
	v[9]=V("begin_flexible_surplus_target");
	v[10]=V("annual_frequency");
	v[11]= fmod((double) t-1,v[10]);
	if(t>=v[9]&&v[9]!=-1&&v[11]==0)
	{
	if(v[3]>v[4])                           		   //if debt to gdp is higher than accepted 
		v[7]=v[2]+v[6];							       //increase surplus target
	else if (v[3]<v[5])                     		   //if debt to gdp is lower than accepted 
		v[7]=v[2]-v[6];								   //deacrease surplus target
	else											   //if debt to gdp is between acceptable band
		{
		if(v[3]>v[8])
			v[7]=v[2]+v[6];							   //increase surplus target
		else
			v[7]=v[2];		
		}		
	}
	else                                               //if flexible surplus target rule is not active
		v[7]=v[2];                                     //do not change surplus taget  
		
	v[8]=max(min(v[0],v[7]),v[1]);
RESULT(v[8])


EQUATION("Government_Max_Expenses_Surplus")
/*
Government Max Expenses determined by Primary Surplus Target Fiscal rule
*/
	v[0]=VL("Country_GDP",1);
	v[1]=VL("Country_GDP",2);
	v[2]=V("government_expectations");
	v[3]= v[1]!=0? v[2]*(v[0]-v[1])/v[1] : 0;
	v[4]=VL("Government_Total_Taxes",1);
	v[5]=V("Government_Surplus_Rate_Target");
	v[6]=(1+v[3])*(v[4]-v[5]*v[0]);
RESULT(v[6])


EQUATION("Government_Max_Expenses_Ceiling")
/*
Government Max Expenses determined by Expenses Ceiling Target Fiscal rule
*/
	v[1]=LAG_GROWTH(consumption, "Sector_Avg_Price", 1, 1);
	v[2]=VL("Government_Effective_Expenses",1);
	v[3]=v[2]*(1+v[1]);
RESULT(v[3])


EQUATION("Government_Max_Expenses")
/*
Maximum Government expenses imposed by the fiscal rule.
Fiscal rules can be two types: primary surplus target or expenses ceiling (or both).
Depend on the policy parameter.
*/

v[1]=V("begin_surplus_target_rule");                           //define when surplus target rule begins
v[2]=V("begin_expenses_ceiling_rule");                         //define when expenses ceiling begins
																	
v[3]=V("Government_Max_Expenses_Surplus");
v[4]=V("Government_Max_Expenses_Ceiling");

	if ((t>=v[1]&&v[1]!=-1)&&(t>=v[2]&&v[2]!=-1))
		v[5]=min(v[3],v[4]);												//surplus rule and ceiling rule
	else if ((t>=v[1]&&v[1]!=-1)&&(t<v[2]||v[2]==-1))
		v[5]=v[3];															//only surplus rule
	else if ((t<v[1]||v[1]==-1)&&(t>=v[2]&&v[2]!=-1))
		v[5]=v[4];															//only ceiling rule
	else
		v[5]=-1;															//no rule															
RESULT(v[5])


EQUATION("Government_Effective_Expenses")
/*
Priority expenses.
If there are no maximum expenses, it is adjusted by average productivity growth and inflation.

switch_government_composition: Determines the composition and priority of government expenses, and initial distribution
0--> Government Wages(100%) [Only Public Workers]
1--> Government Wages(100%) + Unemployment Benefits(0%) [Public Workers and Unemployment Benefits]
2--> Government Wages(1-(c+k+i)%) + Unemployment Benefits(0%) + Government Consumption (c%), Investment(k%) and Intermediate Demand(i%) [Public Workers, Unemployment Benefits, and Demand to Sectors]

government_initial_consumption_share
government_initial_capital_share
government_initial_input_share
government_initial_benefits_share (zero by hypothesis)
those parameters can be used to exclude one or more types of goods, for instance.

*/

v[0]=V("Government_Max_Expenses");
v[1]=V("Government_Desired_Wages");
v[2]=V("Government_Desired_Unemployment_Benefits");
v[3]=V("Government_Desired_Consumption");
v[4]=V("Government_Desired_Investment");
v[5]=V("Government_Desired_Inputs");

if(v[0]==-1)                                               //no fiscal rule
{
	v[8]=v[1];											   //government wages equal desired wages
	v[9]=v[2];    										   //government unemployment benefits equal 0
	v[10]=v[3];                                            //government consumption equal desired
	v[11]=v[5];                                            //government intermediate equal desired
	v[12]=v[4];                                            //government investment demand equals desired
}
else
{
		v[8]=min(v[0],v[1]);								   //government wages is desired limited by maximum expenses
		v[9]=min(v[2],(v[0]-v[8]));    						   //government unemployment benefits is desired limited by maximum expenses minus wages
		v[10]=min(v[3],(v[0]-v[8]-v[9]));       			   //government consumption is desired limited by maximum expenses minus wages and benefits
		v[11]=min(v[5],(v[0]-v[8]-v[9]-v[10]));        		   //government intermediate is desired limited by maximum expenses minus wages and benefits
		v[12]=min(v[4],(v[0]-v[8]-v[9]-v[10]-v[11]));          //government investment is desired limited by maximum expenses minus wages and benefits
		v[14]=max(0,(v[0]-(v[8]+v[9]+v[10]+v[11]+v[12])));
		if(V("switch_extra_gov_expenses")==1)
			v[15]=v[12]+v[14];
		else
			v[15]=v[12];	
}
WRITE("Government_Effective_Wages", max(0,v[8]));
WRITE("Government_Effective_Unemployment_Benefits",  max(0,v[9]));
WRITE("Government_Effective_Consumption",  max(0,v[10]));
WRITE("Government_Effective_Investment",  max(0,v[15]));
WRITE("Government_Effective_Inputs",  max(0,v[11]));
WRITE("Government_Desired_Expenses",  v[1]+v[2]+v[3]+v[4]+v[5]);
v[13]=max(0,(v[8]+v[9]+v[10]+v[11]+v[15]));
RESULT(v[13])

EQUATION_DUMMY("Government_Effective_Wages","Government_Effective_Expenses")
EQUATION_DUMMY("Government_Effective_Unemployment_Benefits","Government_Effective_Expenses")
EQUATION_DUMMY("Government_Effective_Consumption","Government_Effective_Expenses")
EQUATION_DUMMY("Government_Effective_Investment","Government_Effective_Expenses")
EQUATION_DUMMY("Government_Effective_Inputs","Government_Effective_Expenses")
EQUATION_DUMMY("Government_Desired_Expenses","Government_Effective_Expenses")

/*****GOVERNMENT RESULT VARIABLES*****/


EQUATION("Government_Income_Taxes")
RESULT(SUMS(country,"Class_Taxation"))

EQUATION("Government_Indirect_Taxes")
RESULT(SUMS(country,"Sector_Taxation"))

EQUATION("Government_Total_Taxes")
RESULT(V("Government_Income_Taxes")+V("Government_Indirect_Taxes"))

EQUATION("Government_Primary_Result")
RESULT(V("Government_Total_Taxes")-V("Government_Effective_Expenses"))

EQUATION("Government_Interest_Payment")
RESULT(V("Central_Bank_Basic_Interest_Rate")*max(0,VL("Government_Debt",1)))

EQUATION("Government_Nominal_Result")
RESULT(V("Government_Primary_Result")-V("Government_Interest_Payment"))

EQUATION("Government_Debt")
RESULT(CURRENT-V("Government_Nominal_Result")+VS(financial, "Financial_Sector_Rescue"))

EQUATION("Government_Debt_GDP_Ratio")
	v[1]=V("Government_Debt");
	v[2]=V("Country_GDP");
	v[3]= v[2]!=0? v[1]/v[2] : 0;
RESULT(v[3])

EQUATION("Government_Surplus_GDP_Ratio")
	v[1]=V("Government_Primary_Result");
	v[2]=V("Country_GDP");
	v[3]= v[2]!=0? v[1]/v[2] : 0;
RESULT(v[3])

	













