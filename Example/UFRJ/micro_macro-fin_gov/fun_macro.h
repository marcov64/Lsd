/*****MACRO VARIABLES******/


EQUATION("Loans_Distribution_Firms");
/*
Distributed effective loans to firms if there is credit rationing
*/

v[0]=SUM("Firm_Demand_Loans");						//total demand of firm loans
v[12]=SUM("Class_Demand_Loans");

CYCLE(cur, "BANKS")
{
	v[1]=VS(cur, "Bank_Effective_Loans");
	v[2]=VS(cur, "id_bank");
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
		
		v[9]=0;
		SORTS(root, "FIRMS", "Firm_Avg_Debt_Rate", "UP");
		CYCLES(cur1, cur2, "FIRMS")
		{
			v[6]=VS(cur2, "id_firm_bank");
				if (v[6]==v[2])
				{
					v[7]=VS(cur2, "Firm_Demand_Loans");
					if (v[5]>=v[7])
						{
						v[8]=v[7];
						WRITES(cur2, "firm_credit_rationing", 0);
						}
					else
						{
						v[8]=max(0, v[5]);
						WRITES(cur2, "firm_credit_rationing", (v[7]-v[8]));
						}
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
WRITES(cur, "bank_number_clients", v[10]);
}	
RESULT(0)

/*
v[0]=V("Financial_Sector_Effective_Loans"); 		//maximum loans of the financial sector
v[1]=SUM("Class_Demand_Loans");						//class loans, not constrained
v[2]=SUM("Firm_Demand_Loans");						//total demand of firm loans
CYCLE(cur, "SECTORS")								//cycle trough sector
{
	v[3]=SUMS(cur, "Firm_Demand_Loans");			//sector demand of loans
	if(v[2]!=0)
		v[4]=v[3]/v[2];								//sector share of demand
	else
		v[4]=0;
	
	v[5]=v[4]*(v[0]-v[1]);							//share of effective loans to the sector, already discounting class loans
	SORTS(cur, "FIRMS", "Firm_Avg_Debt_Rate", "UP");
	CYCLES(cur, cur1, "FIRMS")
	{
		v[6]=VS(cur1, "Firm_Demand_Loans");			//firm demand loans
		if(v[5]>=v[6])								//if effective available loans to the sector is greater than firm demand for loans
			v[7]=v[6];								//firm effective loans equals firm demand
		else										//if effective loans available to the sector is lower than firm's demand
			v[7]=max(0, v[5]);						//firm receive what is available, can not be negative
		v[5]=v[5]-v[7];								//reduce the total amount of effective loans available to the sector
		WRITES(cur1, "firm_effective_loans",v[7]);	//writes firm effective loans
	}
}
RESULT(0)
*/

	
EQUATION("Total_Domestic_Intermediate_Demand")
/*
Calculates the domestic demand for inputs.
Must be called by the sectors.
*/
		v[2]=0;                                                      		//initializes the value for thr CYCLE
		CYCLE(cur, "SECTORS")                                        		//CYCLE trought all sectors
		{
			v[3]=0;                                                    		//initializes the value for the CYCLE
			CYCLES(cur, cur1, "FIRMS")                                 		//CYCLE trought all firms inside the sectors
			{    
				v[4]=VS(cur1, "Firm_Input_Demand_Next_Period");             //gives the demand for imputs of each firm
				v[3]=v[3]+v[4];                                          	//sums up the demand for imputs of all firms
			} 
		v[2]=v[2]+v[3];                                              		//sums up the demand for imputs of all setors
		}
	cur = SEARCH_CND("id_intermediate_goods_sector", 1);
	v[0]=VLS(cur, "Sector_Avg_Price", 1);
	v[1]=V("Government_Effective_Inputs");
	if(v[0]!=1)
		v[5]=v[1]/v[0];
	else
		v[5]=0;
	v[6]=v[2]+v[5];
RESULT(v[6])


EQUATION("Total_Domestic_Consumption_Demand")
/*
Calculates the domestic demand for consumption goods.
Must be called by the sector.
*/
	v[0]=0;                                                 		//initializes the CYCLE
	CYCLE(cur, "CLASSES")                                   		//CYCLE trought all income classes
	{
		v[1]=VS(cur, "Class_Real_Consumption_Demand");      	    //class consumption
		v[0]=v[0]+v[1];                                       		//sums up all classes consumption
	}
	cur = SEARCH_CND("id_consumption_goods_sector", 1);
	v[4]=VLS(cur, "Sector_Avg_Price", 1);
	v[5]=V("Government_Effective_Consumption");
	if(v[4]!=1)
		v[6]=v[5]/v[4];
	else
		v[6]=0;
	v[7]=v[0]+v[6];
RESULT(v[7])


EQUATION("Total_Domestic_Capital_Goods_Demand")
/*
The demand for capital goods is calculated by summing up the demand for capital goods from all sectors with government spending on investment.
Must be called by the sectors.
*/
	v[1]=0;                                                 	//initializes the CYCLE
	CYCLE(cur, "SECTORS")                                   	//CYCLE trought the sectors
	{	
		v[2]=0;                                               	//initializes the second CYCLE
		CYCLES(cur, cur1, "FIRMS")                            	//CYCLE trought the firms
		{
			v[3]=VS(cur1, "Firm_Demand_Capital_Goods");         //gives the demand for capital goods of each firm
			v[2]=v[2]+v[3];                                     //sums up all capital goods demand
		}
		v[1]=v[1]+v[2];                                       	//sums up all firm's capital goods demand
	}
	cur = SEARCH_CND("id_capital_goods_sector", 1);
	v[4]=VLS(cur, "Sector_Avg_Price", 1);
	v[5]=V("Government_Effective_Investment");
	if(v[4]!=1)
		v[6]=v[5]/v[4];
	else
		v[6]=0;
	v[7]=v[1]+v[6];
RESULT(v[7])


EQUATION("Price_Capital_Goods")
/*
Price of capital goods for the firms is the average price of the capital goods sector
*/
	cur = SEARCH_CND("id_capital_goods_sector", 1);
	v[0]=VLS(cur, "Sector_Avg_Price", 1);
RESULT(v[0])


EQUATION("Total_Distributed_Profits")
/*
Total amount of distributed profits by the firms. Will be used to determine the income of the income classes.
*/
	v[0]=0;                                            		//initializes the CYCLE
	CYCLE(cur, "SECTORS")                              		//CYCLE trought all sectors
	{
		v[1]=0;                                         	//initializes the second CYCLE
		CYCLES(cur, cur1, "FIRMS")                       	//CYCLE trought all firms
		{
			VS(cur1, "Firm_Retained_Profits");           	//make sure Retained_Profits was calculated before
			v[2]=VS(cur1, "Firm_Distributed_Profits");      //value of distributed profits of each firm
			v[1]=v[1]+v[2];                                	//sums up the value of all firms in the sector
		}
		v[0]=v[0]+v[1];                                  	//sums up the value of distributed profits of all sectors
	}
	v[3]=V("Financial_Sector_Distributed_Profits");
	v[4]=v[0]+v[3];
RESULT(v[4])


EQUATION("Total_Profits")
/*
Total Surplus of the Economy. Is the sum of all firms net profits. Will be used to calculate GDP
*/
	v[0]=0;                                                    		//initializes the CYCLE
	CYCLE(cur, "SECTORS")                                      		//CYCLE trought all sectors
	{ 
		v[1]=0;                                                  	//initializes the second CYCLE
		CYCLES(cur, cur1, "FIRMS")                               	//CYCLE trought all firms of the sector
		{
			v[2]=VS(cur1, "Firm_Net_Profits");                      //firm's surplus
			v[1]=v[1]+v[2];                                        	//sums up all firms' surplus of the sector
		}
		v[0]=v[0]+v[1];                                          	//sums up the surplus of all sectors
	}
	v[3]=V("Financial_Sector_Profits");
	v[4]=v[0]+v[3];
RESULT(v[4])


EQUATION("Total_Wages")
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
			v[4]=VLS(cur1, "Firm_Avg_Productivity", 1);            	//firm's productivity in the last period
			v[5]=VS(cur1, "Firm_RND_Expenses");                     //firm's rnd expeses, returned as salary to researchers
			v[1]=v[1]+v[3]*(v[2]/v[4])+v[5];                       	//sums up all firms' wage, determined by a unitary wage (sectorial wage divided by firm's productivity) multiplied by firm's effective production plus RND expenses
		}
		v[0]=v[0]+v[1];                                          	//sums up all wages of all sectors
	}
	v[6]=V("Government_Effective_Wages");                           //wages paid by the government
	v[8]=v[0]+v[6];                                            //sums up productive sectors wages with government wages
RESULT(v[8])


EQUATION("Total_Investment_Expenses")
/*
Aggeregate Investment Expenses is calculated summing up the demand of capital goods of all firms and multiplying by the average price of the capital goods sector
*/
	cur = SEARCH_CND("id_capital_goods_sector", 1);
	v[4]=VS(cur, "Sector_Avg_Price");
	v[0]=0;
	CYCLE(cur, "SECTORS")
	{
		v[1]=0;
		CYCLES(cur, cur1, "FIRMS")
		{
			v[2]=VS(cur1, "Firm_Demand_Capital_Goods");
			v[1]=v[1]+v[2];
		}
	v[0]=v[0]+v[1];
	}
RESULT(v[0])


EQUATION("Profit_Share")
/*
Share of profits over the sum of profits and wages
*/
	v[0]=V("Total_Wages");
	v[1]=V("Total_Profits");
	v[2]=v[0]+v[1];
	if(v[2]!=0)
		v[3]=v[1]/v[2];
	else 
		v[3]=0;
RESULT(v[3])


EQUATION("Wage_Share")
/*
Share of profits over the sum of profits and wages
*/
	v[0]=V("Total_Wages");
	v[1]=V("Total_Profits");
	v[2]=v[0]+v[1];
	if(v[2]!=0)
		v[3]=v[0]/v[2];
	else 
		v[3]=0;
RESULT(v[3])


EQUATION("Avg_Markup")
/*
Agregated average markup, wheighted by the sales of each sector
*/
	v[0]=WHTAVE("Sector_Avg_Markup", "Sector_Sales");
	v[1]=SUM("Sector_Sales");
	if(v[1]!=0)
		v[2]=v[0]/v[1];
	else
		v[2]=0;
RESULT(v[2])


EQUATION("Avg_Debt_Rate_Firms")
/*
Aggregated average debt rate, wheighted by the sales of each sector
*/
	v[0]=WHTAVE("Sector_Avg_Debt_Rate", "Sector_Sales");
	v[1]=SUM("Sector_Sales");
	if(v[1]!=0)
		v[2]=v[0]/v[1];
	else
		v[2]=0;
RESULT(v[2])	


EQUATION("Avg_HHI")
/*
Aggregated average markup, wheighted by the number of firms
*/
	v[0]=WHTAVE("Sector_Normalized_HHI", "Sector_Number_Firms");
	v[1]=SUM("Sector_Number_Firms");
	if(v[1]!=0)
		v[2]=v[0]/v[1];
	else
		v[2]=0;
RESULT(v[2])	


EQUATION("Avg_Hedge_Share")
v[0]=0;
v[1]=0;
CYCLE(cur, "SECTORS")
{
	v[2]=0;
	v[3]=0;
	CYCLES(cur, cur1, "FIRMS")
	{
		v[4]=VS(cur1, "firm_hedge");
		v[3]=v[3]+v[4];
		v[2]=v[2]+1;
	}
	v[0]=v[0]+v[3];//total number of hedge firms
	v[1]=v[1]+v[2];//total number of firms
}
if(v[1]!=0)
	v[5]=v[0]/v[1];
else
	v[5]=0;
RESULT(v[5])	


EQUATION("Avg_Speculative_Share")
v[0]=0;
v[1]=0;
CYCLE(cur, "SECTORS")
{
	v[2]=0;
	v[3]=0;
	CYCLES(cur, cur1, "FIRMS")
	{
		v[4]=VS(cur1, "firm_speculative");
		v[3]=v[3]+v[4];
		v[2]=v[2]+1;
	}
	v[0]=v[0]+v[3];//total number of speculative firms
	v[1]=v[1]+v[2];//total number of firms
}
if(v[1]!=0)
	v[5]=v[0]/v[1];
else
	v[5]=0;
RESULT(v[5])


EQUATION("Avg_Ponzi_Share")
v[0]=0;
v[1]=0;
CYCLE(cur, "SECTORS")
{
	v[2]=0;
	v[3]=0;
	CYCLES(cur, cur1, "FIRMS")
	{
		v[4]=VS(cur1, "firm_ponzi");
		v[3]=v[3]+v[4];
		v[2]=v[2]+1;
	}
	v[0]=v[0]+v[3];//total number of ponzi firms
	v[1]=v[1]+v[2];//total number of firms
}
if(v[1]!=0)
	v[5]=v[0]/v[1];
else
	v[5]=0;
RESULT(v[5])


EQUATION("GDP")
/*
Nominal quarterly GDP is calculated summing up profits, wages and indirect taxes
*/
	v[0]=V("Total_Profits");                       
	v[1]=V("Total_Wages");
	v[2]=V("Total_Indirect_Taxes");
	v[3]=v[0]+v[1]+v[2];
RESULT(v[3])


EQUATION("Real_GDP")
/*
Real quarterly GDP is the nominal GDP over the price index.
*/
	v[0]=V("GDP");                 	 	//nominal GDP
	v[1]=V("Price_Index");           	//current price index
	if(v[1]!=0)                      	//if the price index is not zero
		v[2]=v[0]/v[1];                	//real GDP is the nominal GDP devided by the price index
	else                             	//if the price index is zero
		v[2]=v[0];                     	//real GDP equals nominal GDP
RESULT(v[2])


EQUATION("Annual_Growth")
/*
Annual Nominal GDP growth rate.
*/

	v[0]=V("annual_period");
	v[1]=0;
	for (v[2]=0; v[2]<=(v[0]-1); v[2]=v[2]+1)
		{
		v[3]=VL("GDP", v[2]);
		v[1]=v[1]+v[3];
		}
	v[4]=0;
	for (v[5]=v[0]; v[5]<=(2*v[0]-1); v[5]=v[5]+1)
		{
		v[6]=VL("GDP", v[5]);
		v[4]=v[4]+v[6];
		}
	if (v[4]!=0)
		v[7]=(v[1]-v[4])/v[4];
	else
		v[7]=1;
RESULT(v[7])


EQUATION("Annual_Real_Growth")
/*
Annual Real GDP Growth rate.
*/

	v[0]=V("annual_period");
	v[1]=0;
	for (v[2]=0; v[2]<=(v[0]-1); v[2]=v[2]+1)
		{
		v[3]=VL("Real_GDP", v[2]);
		v[1]=v[1]+v[3];
		}
	v[4]=0;
	for (v[5]=v[0]; v[5]<=(2*v[0]-1); v[5]=v[5]+1)
		{
		v[6]=VL("Real_GDP", v[5]);
		v[4]=v[4]+v[6];
		}
	if (v[4]!=0)
		v[7]=(v[1]-v[4])/v[4];
	else
	v[7]=1;
RESULT(v[7])


EQUATION("Likelihood_Crisis")
/*
Counts the number of crisis ocurrances. 
*/
	v[7]=V("annual_period");
	v[0]= fmod((double) t,v[7]);        		//divides the time period by four
	if(v[0]==0)                        		 	//if the rest of the above division is zero (begenning of the year)
		{
		v[1]=V("Annual_Real_Growth");     		//real growth rate
		v[2]=V("crisis_threshold");       		//parameter that defines crisis
		if(v[1]<v[2])                     		//if the real growth rate is lower the the crisis threshold
			v[3]=1;                         	//counts a crisis
		else                              		//if the real growth rate is not lower the the crisis threshold
			v[3]=0;                         	//do not count a crisis
		}
	else                                		//if the rest of the division is not zero
		v[3]=0;                           		//do not count a crisis   
	v[4]=VL("Likelihood_Crisis",1);     		//crisis counter in the last period
	v[5]=v[4]+v[3];                     		//acumulates the crisis counters
	v[6]=(v[5]/t/v[7]);                      		//gives the probability, total crisis counter divided by the number of time periods
RESULT(v[3])


EQUATION("Total_Consumption")
/*
Quarterly aggregate nominal consumption, given by the nominal value of the consumption good sector sales
*/
	v[0]=0;                                              	//initializes the CYCLE  
	CYCLE(cur, "SECTORS")                                	//CYCLE trough all sectors 
	{
		v[1]=VS(cur, "Sector_Sales");                      	//sector sales
		v[2]=VS(cur, "Sector_Avg_Price");                   //sector average price
		v[3]=VS(cur, "id_consumption_goods_sector");       	//identifies consumption goods sectors
		if (v[3]==1)                                       	//if it is a consumption good sector
			v[0]=v[0]+v[1]*v[2];                            //sums up nominal value of sector production
		else                                               	//if it is not a consumption good sector
			v[0]=v[0];	                                    //does not sum up
	}
RESULT(v[0])


EQUATION("Total_Investment")
/*
Quarterly aggregate nominal investment, given by the nominal value of capital goods sector sales
*/
	v[0]=0;                                              		//initializes the CYCLE  
	CYCLE(cur, "SECTORS")                                		//CYCLE trough all sectors 
	{
		v[1]=VS(cur, "Sector_Sales");                      		//sector sales
		v[2]=VS(cur, "Sector_Avg_Price");                       //sector average price
		v[3]=VS(cur, "id_capital_goods_sector");           		//identifies capital goods sectors
		if (v[3]==1)                                       		//if it is a capital good sector
			v[0]=v[0]+v[1]*v[2];                             	//sums up nominal value of sector production
		else                                               		//if it is not a capital good sector
			v[0]=v[0];	                                     	//does not sum up
	}
RESULT(v[0])


EQUATION("Total_Intermediate")
/*
Quarterly aggregate nominal intermediate consumption, given by the nominal value of intermediate goods sector sales
*/
	v[0]=0;                                              		//initializes the CYCLE  
	CYCLE(cur, "SECTORS")                                		//CYCLE trough all sectors 
	{
		v[1]=VS(cur, "Sector_Sales");                      		//sector sales
		v[2]=VS(cur, "Sector_Avg_Price");                       //sector average price
		v[3]=VS(cur, "id_intermediate_goods_sector");      		//identifies intermediate goods sectors
		if (v[3]==1)                                       		//if it is a intermediate good sector
			v[0]=v[0]+v[1]*v[2];                            	//sums up nominal value of sector production
		else                                               		//if it is not a intermediate good sector
			v[0]=v[0];	                                     	//does not sum up
	}
RESULT(v[0])


EQUATION("Gross_Value_Production")
/*
Nominal value of total sales of the economy
*/
	v[0]=0;                                                		//initializes the CYCLE
	CYCLE(cur, "SECTORS")                                  		//CYCLE trough all sectors
	{
		v[1]=VS(cur,"Sector_Avg_Price");                        //sector average price
		v[2]=VS(cur,"Sector_Sales");                         	//sector sales
		v[0]=v[0]+v[1]*v[2];                                 	//calculates and sums up nominal value of all sectors effective sales
	}
RESULT(v[0])


EQUATION("Avg_Rate_Capacity_Utilization")
/*
Sum up sector's effective production over productive capacity, weighted by sector's nominal value of production over total gross value of production
*/
	v[0]=0;                                                		//initializes the CYCLE
	CYCLE(cur, "SECTORS")                                  		//CYCLE trough all sectors                         
	{
		v[1]=VLS(cur,"Sector_Productive_Capacity",1);        	//sector productive capacity in the last period
		v[2]=VS(cur,"Sector_Effective_Production");          	//sector effective production
		v[3]=V("Gross_Value_Production");                    	//gross value of production
		v[4]=VS(cur,"Sector_Sales");                         	//sector sales
		v[5]=VS(cur,"Sector_Avg_Price");                        //sector average price
		if  (v[1]!=0&&v[3]!=0)
 			v[0]=v[0]+((v[4]*v[5]/v[3])*(v[2]/v[1]));           //calculates and sums up rate of capacity utilization of all sectors. The rate of capacity utilization is given by sales multiplyed by the price and divided by the gross value of production, multiplyed by the ratio between effective production and productive capacit
 		else
 			v[0]=0;
 	}
RESULT(v[0])


EQUATION("Total_Inventories")
/*
Sum up the nominal value of the stock of invesntories of each sector
*/
	v[0]=0;                                                		//initializes the CYCLE
	CYCLE(cur, "SECTORS")                                  		//CYCLE trough all sectors 
	{
		v[1]=VS(cur,"Sector_Avg_Price");                        //sector average price    
		v[2]=VS(cur,"Sector_Inventories");                   	//sector stock of inventories
		v[0]=v[0]+v[1]*v[2];                                 	//calculates and sums up nominal value of inventories of all sectors
	}
RESULT(v[0])


EQUATION("Total_Inventories_Variation")
/*
Sum up the value of changes in iventories of all firms
*/
	v[0]=0;
	CYCLE(cur, "SECTORS")
	{
		v[1]=0;
		CYCLES(cur, cur1, "FIRMS")
		{
			v[2]=VS(cur1, "Firm_Inventories_Variation");
			v[1]=v[1]+v[2];
		}
		v[0]=v[0]+v[1];
	}
RESULT(v[0])


EQUATION("Price_Index")
/*
Paasche price index of all sector. GDP deflator
*/
	v[0]=v[1]=0;                                         	//initializes the CYCLE  
	CYCLE(cur, "SECTORS")                                	//CYCLE trough all sectors 
	{
		v[2]=VS(cur, "Sector_Sales");                      	//sector sales
		v[3]=VS(cur, "Sector_Avg_Price");                   //sector average price
		v[4]=VS(cur, "sector_initial_price");               //sector initial average price

		v[0]=v[0]+v[2]*v[3];                               	//current nominal prices of the sector
		v[1]=v[1]+v[2]*v[4];                               	//initial prices of the sector
	}
	if (v[1]!=0)                                         	//if initial prices is not zero
		v[5]=v[4]*(v[0]/v[1]);                              //price index will be current prices over initial prices, 
	else                                                 	//if initial prices is zero
		v[5]=VL("Price_Index", 1);                         	//use last period price index
RESULT(v[5])


EQUATION("Consumer_Price_Index")
/*
Paasche price index of consumption goods sector. Used for inflation target and income classes real income.
*/
	v[0]=v[1]=0;                                         		//initializes the CYCLE  
	CYCLE(cur, "SECTORS")                                		//CYCLE trough all sectors 
	{
		v[6]=VS(cur, "id_consumption_goods_sector");
		if (v[6]==1)
			{
			v[2]=VS(cur, "Sector_Sales");                      	//sector sales
			v[3]=VS(cur, "Sector_Avg_Price");                   //sector average price
			v[4]=VS(cur, "sector_initial_price");               //sector initial average price
			v[0]=v[0]+v[2]*v[3];                               	//current nominal prices of the sector
			v[1]=v[1]+v[2]*v[4];                               	//initial prices of the sector
			}
		else
			{
			v[0]=v[0];
			v[1]=v[1];	
			}	
	}	
	if (v[1]!=0)                                         		//if initial prices is not zero
		v[5]=v[4]*(v[0]/v[1]);                                  //price index will be current prices over initial prices, 
	else                                                 		//if initial prices is zero
		v[5]=VL("Consumer_Price_Index", 1);                		//use last period price index
RESULT(v[5])


EQUATION("Annual_Inflation")
/*
Annual growth of the consumer price index
*/
	v[0]=V("annual_period");
	v[1]=VL("Consumer_Price_Index",1);
	v[2]=VL("Consumer_Price_Index",(v[0]+1));
	v[3]=(v[1]/v[2])-1;
RESULT(v[3])


EQUATION("Avg_Productivity")
/*
Average Productivity of the economy weighted by the employment of each sector 
*/
	v[0]=WHTAVE("Sector_Avg_Productivity", "Sector_Employment");
	v[1]=SUM("Sector_Employment");
	if(v[1]!=0)
		v[2]=v[0]/v[1];
	else
		v[2]=0;
RESULT(v[2])


EQUATION("Unemployment")
/*
Unemployment rate, in percentage value
*/
	v[0]=SUM("Sector_Potential_Employment");
	v[1]=SUM("Sector_Employment");
	if(v[0]==0)
		v[2]=0;
	else
		v[2]=(v[0]-v[1])/v[0];
RESULT(max(0,v[2]))


EQUATION("Total_Exports")
/*
The total exports of the economy in nominal value are defined by the sum of the exports of each sector multiplied by the price charged in the period.
*/
	v[0]=0;
	CYCLE(cur, "SECTORS")
	{
		v[1]=VS(cur, "Sector_Exports");
		v[2]=VS(cur, "Sector_Avg_Price");
		v[3]=VS(cur, "Sector_Demand_Met");
		v[4]=v[1]*v[2]*v[3];
		v[0]=v[0]+v[4];
	}
RESULT(v[0])


EQUATION("Total_Imports")
/*
Total imports in nominal value are obtained from the sum of imports of all sectors multiplied by the respective international prices, and converted to national currency by the exchange rate.
*/
	v[0]=WHTAVE("Sector_Extra_Imports", "Sector_External_Price");
	v[1]=SUM("Class_Effective_Imports");
	v[2]=V("Exchange_Rate");
	v[3]=(v[0]+v[1])*v[2];
RESULT(v[3])


EQUATION("GDP_Demand")
/*
GDP calculated by the demand perspective
*/
	v[0]=V("Total_Classes_Expenses");
	v[1]=V("Government_Effective_Expenses");
	v[2]=V("Total_Exports");
	v[3]=V("Total_Imports");
	v[4]=V("Total_Inventories_Variation");
	v[6]=V("Total_Investment_Expenses");
	v[7]=v[0]+v[1]+v[2]-v[3]+v[4]+v[6];
RESULT(v[7])


EQUATION("Total_Classes_Expenses")
/*
Sum up the income classes expenses
*/
	v[0]=SUM("Class_Effective_Expenses");
RESULT(v[0])


EQUATION("Total_Productive_Capacity_Depreciated")
/*
Sum up the value of depreciated productive capacity of all firms
*/
	v[0]=0;
	CYCLE(cur, "SECTORS")
	{
		v[1]=0;
		CYCLES(cur, cur1, "FIRMS")
		{
			v[2]=VS(cur1, "Firm_Productive_Capacity_Depreciation");
			v[1]=v[1]+v[2];
		}
		v[0]=v[0]+v[1];
	}
RESULT(v[0])


EQUATION("Total_Productive_Capacity_Expansion")
/*
Sum up the value of productive capacity for expanstion of all firms
*/
	v[0]=0;
	CYCLE(cur, "SECTORS")
	{
		v[1]=0;
		CYCLES(cur, cur1, "FIRMS")
		{
			v[2]=VS(cur1, "Firm_Demand_Productive_Capacity_Expansion");
			v[1]=v[1]+v[2];
		}
		v[0]=v[0]+v[1];
	}
RESULT(v[0])


EQUATION("Total_Productive_Capacity_Replacement")
/*
Sum up the value of productive capacity for replacement of all firms
*/
	v[0]=0;
	CYCLE(cur, "SECTORS")
	{
		v[1]=0;
		CYCLES(cur, cur1, "FIRMS")
		{
			v[2]=VS(cur1, "Firm_Demand_Productive_Capacity_Replacement");
			v[1]=v[1]+v[2];
		}
		v[0]=v[0]+v[1];
	}
RESULT(v[0])


EQUATION("Total_Capital_Stock")
/*
Sum up the nominal value of firms stock of capital
*/
	v[0]=0;
	CYCLE(cur, "SECTORS")
	{
		v[1]=0;
		CYCLES(cur, cur1, "FIRMS")
		{
			v[2]=VS(cur1, "Firm_Capital");
			v[1]=v[1]+v[2];
		}
		v[0]=v[0]+v[1];
	}
RESULT(v[0])


EQUATION("Observed_Capital_Output_Ratio")
/*
Observed Ratio, Stock of Capital over GDP
*/
	v[0]=V("GDP");
	v[1]=V("Total_Capital_Stock");
	if (v[0]!=0)
		v[2]=v[1]/v[0];
	else 
		v[2]=0;
RESULT(v[2])


EQUATION("Observed_Capital_Labor_Ratio")
/*
Observed Ratio, Stock of Capital over Total Employment
*/
	v[0]=SUM("Sector_Employment");
	v[1]=V("Total_Capital_Stock");
	if (v[0]!=0)
		v[2]=v[1]/v[0];
	else 
		v[2]=0;
RESULT(v[2])


EQUATION("Avg_Profit_Rate")
/*
Observed Ratio, Total Profits over Stock of Capital
*/
	v[0]=V("Total_Profits");
	v[1]=V("Total_Capital_Stock");
	if (v[1]!=0)
		v[2]=v[0]/v[1];
	else 
		v[2]=0;
RESULT(v[2])


EQUATION("Total_Induced_Investment")
/*
Sum up the nominal value of effective expansion investment of all firms
*/
	v[0]=0;
	CYCLE(cur, "SECTORS")
	{
		v[1]=0;
		CYCLES(cur, cur1, "FIRMS")
		{
			v[2]=VS(cur1, "Firm_Effective_Expansion_Investment_Expenses");
			v[1]=v[1]+v[2];
		}
		v[0]=v[0]+v[1];
	}
RESULT(v[0])


EQUATION("Total_Autonomous_Investment")
/*
Sum up the nominal value of effective replacement investment of all firms
*/
	v[0]=0;
	CYCLE(cur, "SECTORS")
	{
		v[1]=0;
		CYCLES(cur, cur1, "FIRMS")
		{
			v[2]=VS(cur1, "Firm_Replacement_Expenses");
			v[1]=v[1]+v[2];
		}
		v[0]=v[0]+v[1];
	}
RESULT(v[0])


EQUATION("Total_Autonomous_Consumption")
/*
Sum up nominal value of autonomous consumption
*/
	v[0]=SUM("Class_Real_Autonomous_Consumption");
	cur = SEARCH_CND("id_consumption_goods_sector", 1);
	v[1]=VS(cur, "Sector_Avg_Price");
	v[2]=v[0]*v[1];
RESULT(v[2])


EQUATION("Avg_Debt_Rate_Class")
/*
Aggregated average class debt rate, wheighted by the income of each class
*/
	v[0]=WHTAVE("Class_Debt_Rate", "Class_Nominal_Income");
	v[1]=SUM("Class_Nominal_Income");
	if(v[1]!=0)
		v[2]=v[0]/v[1];
	else
		v[2]=0;
RESULT(v[2])


EQUATION("Gini_Income_Class")
/*
Gini Index for income classes. Fixed rate.
*/
v[0]=COUNT("CLASSES");               //count number of income classes
v[1]=MIN("Class_Income_Share");      //lowest income share
v[2]=MED("Class_Income_Share");      //median income share
v[3]=MAX("Class_Income_Share");      //high income share

v[4]=1/v[0];                         //lowest class share of population
v[5]=1/v[0];                         //mid class share of population
v[6]=1/v[0];                         //high class share of population

v[7]=v[4]*v[1]+v[5]*v[2]+v[6]*v[3]+2*v[5]*v[1]+2*v[6]*v[2]+2*v[6]*v[1];
v[8]=0.5-(v[7]/2);
RESULT(v[8])


EQUATION("Gini_Wealth_Class")
/*
Gini Index for wealth. Fixed rate.
*/
v[0]=COUNT("CLASSES");               //count number of income classes
v[1]=MIN("Class_Wealth_Share");      //lowest income share
v[2]=MED("Class_Wealth_Share");      //median income share
v[3]=MAX("Class_Wealth_Share");      //high income share

v[4]=1/v[0];                         //lowest class share of population
v[5]=1/v[0];                         //mid class share of population
v[6]=1/v[0];                         //high class share of population

v[7]=v[4]*v[1]+v[5]*v[2]+v[6]*v[3]+2*v[5]*v[1]+2*v[6]*v[2]+2*v[6]*v[1];
v[8]=0.5-(v[7]/2);
RESULT(v[8])


EQUATION("Gini_Income_Population")
/*
Gini Index for income classes. Based on Population share
*/
v[1]=MIN("Class_Income_Share");      //lowest income share
v[2]=MED("Class_Income_Share");      //median income share
v[3]=MAX("Class_Income_Share");      //high income share

cur1=SEARCH_CND("Class_Income_Share",v[1]);
cur2=SEARCH_CND("Class_Income_Share",v[2]);
cur3=SEARCH_CND("Class_Income_Share",v[3]);

v[4]=VS(cur1,"class_population_share");                         //lowest class share of population
v[5]=VS(cur2,"class_population_share");                         //mid class share of population
v[6]=VS(cur3,"class_population_share");                         //high class share of population

v[7]=v[4]*v[1]+v[5]*v[2]+v[6]*v[3]+2*v[5]*v[1]+2*v[6]*v[2]+2*v[6]*v[1];
v[8]=0.5-(v[7]/2);
RESULT(v[8])


EQUATION("Gini_Wealth_Population")
/*
Gini Index for wealth. Fixed rate.
*/
v[1]=MIN("Class_Wealth_Share");      //lowest income share
v[2]=MED("Class_Wealth_Share");      //median income share
v[3]=MAX("Class_Wealth_Share");      //high income share

cur1=SEARCH_CND("Class_Wealth_Share",v[1]);
cur2=SEARCH_CND("Class_Wealth_Share",v[2]);
cur3=SEARCH_CND("Class_Wealth_Share",v[3]);

v[4]=VS(cur1,"class_population_share");                         //lowest class share of population
v[5]=VS(cur2,"class_population_share");                         //mid class share of population
v[6]=VS(cur3,"class_population_share");                         //high class share of population

v[7]=v[4]*v[1]+v[5]*v[2]+v[6]*v[3]+2*v[5]*v[1]+2*v[6]*v[2]+2*v[6]*v[1];
v[8]=0.5-(v[7]/2);
RESULT(v[8])


