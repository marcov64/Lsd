/*****MACRO VARIABLES******/


EQUATION("Loans_Distribution_Firms");
/*
Distributed effective loans to firms if there is credit rationing
This variable is very important
It is located in the macro level
There are some underlying hypothesis:
1-Income classes are never credit rationed and receive loans first.
2-A bank has a total amount of loans it can provide. After discounting the amount for the income classes, it distribute proportionally to each sector
3-Within each sector, it provides in a order of debt rate. High indebtedness firms migh not receive loans.
*/

v[0]=SUM("Firm_Demand_Loans");						//total demand of firm loans
v[12]=SUM("Class_Demand_Loans");

CYCLE(cur, "BANKS")
{
	v[1]=VS(cur, "Bank_Effective_Loans");
	v[2]=VS(cur, "bank_id");
	v[13]=VS(cur, "Bank_Market_Share");
	v[14]=v[13]*v[12];
	v[10]=0;
	CYCLES(root, cur1, "SECTORS")
	{
		v[3]=SUMS(cur1, "Firm_Demand_Loans");			//sector demand of loans
		if(v[0]!=0)
			v[4]=v[3]/v[0];								//sector share of demand
		else
			v[4]=0;
		
		v[5]=max(0,(v[1]-v[14])*v[4]);
		v[11]=V("switch_creditworthness");
				
			v[9]=0;
			if(v[11]==1)
				SORTS(root, "FIRMS", "Firm_Avg_Debt_Rate", "UP");
			if(v[11]==2)
				SORTS(root, "FIRMS", "firm_date_birth", "UP");
			if(v[11]==3)
				SORTS(root, "FIRMS", "firm_date_birth", "DOWN");
			CYCLES(cur1, cur2, "FIRMS")
			{
				v[6]=VS(cur2, "firm_bank");
					if (v[6]==v[2])
					{
						v[7]=VS(cur2, "Firm_Demand_Loans");
						if (v[5]>=v[7])
							v[8]=v[7];
						else
							v[8]=max(0, v[5]);
						v[5]=v[5]-v[8];
						WRITES(cur2, "firm_effective_loans", v[8]);
						v[9]=v[9]+1;
					}
					else
					{
						v[5]=v[5];
						v[9]=v[9];
					}		
			}

	v[10]=v[10]+v[9];
	}
WRITES(cur, "Bank_Number_Clients", v[10]);
}	
RESULT(0)

	
EQUATION("Country_Domestic_Intermediate_Demand")
/*
Calculates the domestic demand for inputs.
Must be called by the sectors.
*/
	v[2]=0;                                                      		//initializes the value for thr CYCLE
	CYCLE(cur, "SECTORS")                                        		//CYCLE trought all sectors
		v[2]=v[2]+SUMS(cur, "Firm_Input_Demand_Next_Period");           //sums up the demand for imputs of all setors
	v[0]=VS(input, "Sector_Avg_Price");
	v[1]=V("Government_Effective_Inputs");
	v[5]=v[0]!=0? v[1]/v[0] : 0;
	v[6]=v[2]+v[5];
RESULT(v[6])


EQUATION("Country_Domestic_Consumption_Demand")
/*
Calculates the domestic demand for consumption goods.
Must be called by the sector.
*/
	v[0]=SUM("Class_Real_Domestic_Consumption_Demand");
	v[1]=VS(consumption, "Sector_Avg_Price");
	v[2]=V("Government_Effective_Consumption");
	v[3]= v[1]!=0? v[2]/v[1] : 0;
	v[4]=v[0]+v[3];
RESULT(v[4])


EQUATION("Country_Domestic_Capital_Goods_Demand")
/*
The demand for capital goods is calculated by summing up the demand for capital goods from all sectors with government spending on investment.
Must be called by the sectors.
*/
	v[1]=0;                                                 			//initializes the CYCLE
	CYCLE(cur, "SECTORS")                                   			//CYCLE trought the sectors
	{	
		v[10]=SUMLS(cur, "Firm_Demand_Capital_Goods_Expansion",1);
		v[11]=SUMLS(cur, "Firm_Demand_Capital_Goods_Replacement",1);
		v[1]=v[1]+v[10]+v[11];                                       	//sums up all firm's capital goods demand
	}
	v[4]=VS(capital, "Sector_Avg_Price");
	v[5]=V("Government_Effective_Investment");
	v[6]= v[4]!=0? v[5]/v[4] : 0;
	v[7]=v[1]+v[6];
RESULT(v[7])


EQUATION("Country_Capital_Goods_Price")
RESULT(VS(capital, "Sector_Avg_Price"))


EQUATION("Country_Price_Index")
/*
Average Price of all sector. GDP deflator
*/
	v[0]=WHTAVE("Sector_Sales","Sector_Avg_Price");
	v[1]=SUM("Sector_Sales");
	v[2]= v[1]!=0? v[0]/v[1] : CURRENT;
RESULT(v[2])


EQUATION("Country_Consumer_Price_Index")
/*
Average Price of the consumption goods sector
*/
	v[0]=VS(consumption, "Sector_Avg_Price");
	v[1]=VS(consumption, "Sector_External_Price");
	v[2]=VS(external, "Country_Exchange_Rate");
	v[3]=0;
	CYCLE(cur, "CLASSES")
	{
		v[5]=VS(cur, "Class_Imports_Share");
		v[6]=VLS(cur, "Class_Income_Share",1);
		v[3]=v[3]+v[5]*v[6];		
	}
	v[4]=v[0]*(1-v[3])+v[1]*v[2]*v[3];
RESULT(v[4])


EQUATION("Country_Annual_Inflation")
/*
Annual growth of the overall price index.
Uses support function
*/
RESULT(LAG_GROWTH(p, "Country_Price_Index", V("annual_frequency"), 1))


EQUATION("Country_Annual_CPI_Inflation")
/*
Annual growth of the consumer price index
Uses support function
*/
RESULT(LAG_GROWTH(p, "Country_Consumer_Price_Index", V("annual_frequency"), 1))


EQUATION("Country_Distributed_Profits")
/*
Total amount of distributed profits by the firms. Will be used to determine the income of the income classes.
*/
	v[0]=0;                                            		//initializes the CYCLE
	CYCLE(cur, "SECTORS")                              		//CYCLE trought all sectors
		v[0]=v[0]+SUMS(cur, "Firm_Distributed_Profits");    //sums up the value of distributed profits of all sectors
	v[3]=V("Financial_Sector_Distributed_Profits");
	v[4]=v[0]+v[3];
RESULT(v[4])


EQUATION("Country_Total_Profits")
/*
Total Surplus of the Economy. Is the sum of all firms net profits. Will be used to calculate GDP
*/
	v[0]=0;                                                    		//initializes the CYCLE
	CYCLE(cur, "SECTORS")                                      		//CYCLE trought all sectors
		v[0]=v[0]+SUMS(cur, "Firm_Net_Profits");                    //sums up the surplus of all sectors
	v[3]=V("Financial_Sector_Profits");
	v[4]=v[0]+v[3];
RESULT(v[4])


EQUATION("Country_Total_Wages")
/*
The total wage is calculated by the sum of the wages paid by the sectors with government wages. The wage per unit of production is a predetermined parameter, and the total salary is calculated by multiplying this unit wage by the actual production of each sector.
*/
	v[0]=0;                                                    		//initializes the CYCLE
	CYCLE(cur, "SECTORS")                                      		//CYCLE trought all sectors
	{
		v[1]=0;                                                  	//initializes the second CYCLE
		CYCLES(cur, cur1, "FIRMS")                               	//CYCLE trought all firms of the sector
		{
			v[2]=VS(cur1, "Firm_Wage");                             //firm's wage
			v[3]=VS(cur1, "Firm_Effective_Production");             //firm's effective production
			v[4]=VS(cur1, "Firm_Avg_Productivity");            		//firm's productivity in the last period
			v[5]=VS(cur1, "Firm_RND_Expenses");                     //firm's rnd expeses, returned as salary to researchers		
			if(v[4]!=0)
				v[1]=v[1]+v[3]*(v[2]/v[4])+v[5];               		//sums up all firms' wage, determined by a unitary wage (sectorial wage divided by firm's productivity) multiplied by firm's effective production plus RND expenses
			else
				v[1]=v[1];
		}
		v[0]=v[0]+v[1];                                          	//sums up all wages of all sectors
	}
	v[6]=V("Government_Effective_Wages");                           //wages paid by the government
	v[7]=v[0]+v[6];                                            		//sums up productive sectors wages with government wages
RESULT(v[7])


EQUATION("Country_Total_Investment_Expenses")
/*
Aggeregate Investment Expenses is calculated summing up the demand of capital goods of all firms and multiplying by the average price of the capital goods sector
*/
	v[0]=0;
	CYCLE(cur, "SECTORS")
		v[0]=v[0]+SUMS(cur, "Firm_Effective_Investment_Expenses");
RESULT(v[0])


EQUATION("Country_Profit_Share")
/*
Share of profits over the sum of profits and wages
*/
	v[0]=V("Country_Total_Wages");
	v[1]=V("Country_Total_Profits");
	v[2]=v[0]+v[1];
	v[3]= v[2]!=0? v[1]/v[2] : 0;
RESULT(v[3])


EQUATION("Country_Wage_Share")
/*
Share of profits over the sum of profits and wages
*/
	v[0]=V("Country_Total_Wages");
	v[1]=V("Country_Total_Profits");
	v[2]=v[0]+v[1];
	v[3]= v[2]!=0? v[0]/v[2] : 0;
RESULT(v[3])


EQUATION("Country_Avg_Markup")
/*
Agregated average markup, wheighted by the sales of each sector
*/
	v[0]=WHTAVE("Sector_Avg_Markup", "Sector_Sales");
	v[1]=SUM("Sector_Sales");
	v[2]= v[1]!=0? v[0]/v[1]: 0;
RESULT(v[2])


EQUATION("Country_Debt_Rate_Firms")
/*
Aggregated average debt rate, wheighted by the sales of each sector
*/
	v[0]=WHTAVE("Sector_Avg_Debt_Rate", "Sector_Sales");
	v[1]=SUM("Sector_Sales");
	v[2]= v[1]!=0? v[0]/v[1]: 0;
RESULT(v[2])	


EQUATION("Country_Avg_HHI")
/*
Aggregated average markup, wheighted by the number of firms
*/
	v[0]=WHTAVE("Sector_Normalized_HHI", "Sector_Number_Firms");
	v[1]=SUM("Sector_Number_Firms");
	v[2]= v[1]!=0? v[0]/v[1]: 0;
RESULT(v[2])	


EQUATION("Country_Hedge_Share")
	v[0]=0;
	CYCLE(cur, "SECTORS")
		v[0]=v[0]+SUMS(cur, "firm_hedge");
	v[2]=COUNT_ALL("FIRMS");
	v[3]= v[2]!=0? v[0]/v[2] : 0;
RESULT(v[3])	


EQUATION("Country_Speculative_Share")
	v[0]=0;
	CYCLE(cur, "SECTORS")
		v[0]=v[0]+SUMS(cur, "firm_speculative");
	v[2]=COUNT_ALL("FIRMS");
	v[3]= v[2]!=0? v[0]/v[2] : 0;
RESULT(v[3])


EQUATION("Country_Ponzi_Share")
	v[0]=0;
	CYCLE(cur, "SECTORS")
		v[0]=v[0]+SUMS(cur, "firm_ponzi");
	v[2]=COUNT_ALL("FIRMS");
	v[3]= v[2]!=0? v[0]/v[2] : 0;
RESULT(v[3])


EQUATION("Country_GDP")
/*
Nominal quarterly GDP is calculated summing up profits, wages and indirect taxes
*/
	v[0]=V("Country_Total_Profits");                       
	v[1]=V("Country_Total_Wages");
	v[2]=V("Government_Indirect_Taxes");
	v[3]=v[0]+v[1]+v[2];
	v[4]=V("Country_GDP_Demand");
RESULT(v[3])


EQUATION("Country_Annual_GDP")
RESULT(LAG_SUM(p, "Country_GDP", V("annual_frequency")))


EQUATION("Country_Annual_Real_GDP")
RESULT(LAG_SUM(p, "Country_Real_GDP", V("annual_frequency")))


EQUATION("Country_Real_GDP")
/*
Real quarterly GDP is the nominal GDP over the price index.
*/
	v[0]=V("Country_GDP");              //nominal GDP
	v[1]=V("Country_Price_Index");      //current price index
	v[2]= v[1]!=0? v[0]/v[1] : 0;       //real GDP is the nominal GDP devided by the price index
RESULT(v[2])


EQUATION("Country_Annual_Growth")
/*
Annual Nominal GDP growth rate.
*/
	v[1]=LAG_SUM(p, "Country_GDP", V("annual_frequency"));
	v[2]=LAG_SUM(p, "Country_GDP", V("annual_frequency"), V("annual_frequency") );
	v[3]= v[2]!=0? (v[1]-v[2])/v[2] : 0;
RESULT(v[3])


EQUATION("Country_Annual_Real_Growth")
/*
Annual Real GDP Growth rate.
*/
	v[1]=LAG_SUM(p, "Country_Real_GDP", V("annual_frequency"));
	v[2]=LAG_SUM(p, "Country_Real_GDP", V("annual_frequency"), V("annual_frequency") );
	v[3]= v[2]!=0? (v[1]-v[2])/v[2] : 0;
RESULT(v[3])


EQUATION("Country_Likelihood_Crisis")
/*
Counts the number of crisis ocurrances. 
*/
	v[7]=V("annual_frequency");
	v[0]= fmod((double) t,v[7]);        		//divides the time period by four
	if(v[0]==0)                        		 	//if the rest of the above division is zero (begenning of the year)
		{
		v[1]=V("Country_Annual_Real_Growth");   //real growth rate
		if(v[1]<0)                     			//if the real growth rate is lower the the crisis threshold
			v[3]=1;                         	//counts a crisis
		else                              		//if the real growth rate is not lower the the crisis threshold
			v[3]=0;                         	//do not count a crisis
		}
	else                                		//if the rest of the division is not zero
		v[3]=0;                           		//do not count a crisis   
	v[4]=CURRENT;     							//crisis counter in the last period
	v[5]=v[4]+v[3];                     		//acumulates the crisis counters
	v[6]=(v[5]/t/v[7]);                      	//gives the probability, total crisis counter divided by the number of time periods
RESULT(v[3])


EQUATION("Country_Nominal_Consumption_Production")
RESULT(VS(consumption, "Sector_Sales")*VS(consumption, "Sector_Avg_Price"))

EQUATION("Country_Nominal_Capital_Production")
RESULT(VS(capital, "Sector_Sales")*VS(capital, "Sector_Avg_Price"))

EQUATION("Country_Nominal_Input_Production")
RESULT(VS(input, "Sector_Sales")*VS(input, "Sector_Avg_Price"))

EQUATION("Country_Total_Nominal_Production")
RESULT(WHTAVE("Sector_Avg_Price","Sector_Sales"))

EQUATION("Country_Capacity_Utilization")
/*
Sum up sector's effective production over productive capacity, weighted by sector's nominal value of production over total gross value of production
*/
	v[0]=WHTAVE("Sector_Capacity_Utilization", "Sector_Effective_Production");
	v[1]=SUM("Sector_Effective_Production");
	v[2]= v[1]!=0? v[0]/v[1]: 0;
RESULT(v[2])

EQUATION("Country_Idle_Capacity")
RESULT(1-V("Country_Capacity_Utilization"))

EQUATION("Country_Inventories")
RESULT(WHTAVE("Sector_Avg_Price","Sector_Inventories"))


EQUATION("Country_Inventories_Variation")
/*
Sum up the value of changes in iventories of all firms
*/
	v[0]=0;
	CYCLE(cur, "SECTORS")
		v[0]=v[0]+SUMS(cur, "Firm_Inventories_Variation");
RESULT(v[0])


EQUATION("Country_Avg_Productivity")
/*
Average Productivity of the economy weighted by the employment of each sector 
*/
	v[0]=WHTAVE("Sector_Avg_Productivity", "Sector_Employment");
	v[1]=SUM("Sector_Employment");
	v[2]= v[1]!=0? v[0]/v[1] : 0;
RESULT(v[2])


EQUATION("Country_GDP_Demand")
/*
GDP calculated by the demand perspective
*/
	v[0]=V("Country_Total_Classes_Expenses");
	v[1]=V("Country_Total_Investment_Expenses");
	v[2]=V("Government_Effective_Expenses");
	v[3]=V("Country_Nominal_Exports");
	v[4]=V("Country_Nominal_Imports");
	v[5]=v[0]+v[1]+v[2]+v[3]-v[4];
RESULT(v[5])


EQUATION("Country_Real_GDP_Demand")
/*
Real quarterly GDP is the nominal GDP over the price index.
*/
	v[0]=V("Country_GDP_Demand");       //nominal GDP
	v[1]=V("Country_Price_Index");      //current price index
	v[2]= v[1]!=0? v[0]/v[1] : 0;   	//real GDP is the nominal GDP devided by the price index
RESULT(v[2])


EQUATION("Country_Total_Classes_Expenses")
RESULT(SUM("Class_Effective_Expenses"))


EQUATION("Country_Productive_Capacity_Depreciated")
/*
Sum up the value of depreciated productive capacity of all firms
*/
	v[0]=0;
	CYCLE(cur, "SECTORS")
		v[0]=v[0]+SUMS(cur, "Firm_Productive_Capacity_Depreciation");
RESULT(v[0])


EQUATION("Country_Productive_Capacity_Expansion")
/*
Sum up the value of productive capacity for expanstion of all firms
*/
	v[0]=0;
	CYCLE(cur, "SECTORS")
		v[0]=v[0]+SUMS(cur, "Firm_Demand_Productive_Capacity_Expansion");
RESULT(v[0])


EQUATION("Country_Productive_Capacity_Replacement")
/*
Sum up the value of productive capacity for replacement of all firms
*/
	v[0]=0;
	CYCLE(cur, "SECTORS")
		v[0]=v[0]+SUMS(cur, "Firm_Demand_Productive_Capacity_Replacement");
RESULT(v[0])


EQUATION("Country_Capital_Stock")
/*
Sum up the nominal value of firms stock of capital
*/
	v[0]=0;
	CYCLE(cur, "SECTORS")
		v[0]=v[0]+SUMS(cur, "Firm_Capital");
RESULT(v[0])


EQUATION("Country_Capital_Output_Ratio")
/*
Observed Ratio, Stock of Capital over GDP
*/
	v[0]=V("Country_GDP");
	v[1]=V("Country_Capital_Stock");
	v[2]= v[0]!=0? v[1]/v[0] : 0;
RESULT(v[2])


EQUATION("Country_Capital_Labor_Ratio")
/*
Observed Ratio, Stock of Capital over Total Employment
*/
	v[0]=SUM("Sector_Employment");
	v[1]=V("Country_Capital_Stock");
	v[2]= v[0]!=0? v[1]/v[0] : 0;
RESULT(v[2])


EQUATION("Country_Avg_Profit_Rate")
/*
Observed Ratio, Total Profits over Stock of Capital
*/
	v[0]=V("Country_Total_Profits")-V("Financial_Sector_Profits");
	v[1]=V("Country_Capital_Stock");
	v[2]= v[1]!=0? v[0]/v[1] : 0;
RESULT(v[2])


EQUATION("Country_Induced_Investment")
/*
Sum up the nominal value of effective expansion investment of all firms
*/
	v[0]=0;
	CYCLE(cur, "SECTORS")
		v[0]=v[0]+SUMS(cur, "Firm_Effective_Expansion_Investment_Expenses");
RESULT(v[0])


EQUATION("Country_Autonomous_Investment")
/*
Sum up the nominal value of effective replacement investment of all firms
*/
	v[0]=0;
	CYCLE(cur, "SECTORS")
		v[0]=v[0]+SUMS(cur, "Firm_Replacement_Expenses");
RESULT(v[0])


EQUATION("Country_Autonomous_Consumption")
/*
Sum up nominal value of autonomous consumption
*/
	v[0]=SUM("Class_Real_Autonomous_Consumption");
	v[1]=VS(consumption, "Sector_Avg_Price");
	v[2]=v[0]*v[1];
RESULT(v[2])


EQUATION("Country_Debt_Rate_Class")
/*
Aggregated average class debt rate, wheighted by the income of each class
*/
	v[0]=WHTAVE("Class_Debt_Rate", "Class_Nominal_Disposable_Income");
	v[1]=SUM("Class_Nominal_Disposable_Income");
	v[2]= v[1]!=0? v[0]/v[1] : 0;
RESULT(v[2])


EQUATION("Country_Gini_Index")
	v[1]=VS(cclass, "Class_Income_Share");
	v[2]=VS(bclass, "Class_Income_Share");
	v[3]=VS(aclass, "Class_Income_Share");
	v[4]=VS(cclass, "class_population_share");
	v[5]=VS(bclass, "class_population_share");
	v[6]=VS(aclass, "class_population_share");
	v[7]=1-v[4];
	v[8]=1-v[4]-v[5];
	v[9]=0;
	v[11]=v[1]*(v[4]+2*v[7]);
	v[12]=v[2]*(v[5]+2*v[8]);
	v[13]=v[3]*(v[6]+2*v[9]);
	v[0]=1-(v[11]+v[12]+v[13]);
RESULT(v[0])






