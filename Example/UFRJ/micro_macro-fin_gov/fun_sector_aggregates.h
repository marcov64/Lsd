/*****SECTOR ANALYSIS*****/

EQUATION("Sector_Number_Firms")
RESULT(COUNT("FIRMS"))

EQUATION("Sector_Profit_Rate")
/*
Sector Variable for Analysis
*/
	v[0]=0;                           		//initializes the CYCLE    
	CYCLE(cur, "FIRMS")               		//CYCLE trought all firms of the sector
	{
		v[1]=VS(cur, "Firm_Net_Profits");  
		v[2]=VLS(cur, "Firm_Capital",1);		
		v[5]=VS(cur, "Firm_Deposits_Return");
		if(v[2]!=0)
			v[0]=v[0]+(v[1]-v[5])/v[2];          
		else
			v[0]=v[0];
	}
	v[3]=COUNT("FIRMS");
	v[4]= v[3]!=0? v[0]/v[3] : 0;
RESULT(v[4])


EQUATION("Sector_Normalized_HHI")
/*
Sector Variable for Analysis
*/
	v[0]=0;                           		//initializes the CYCLE    
	CYCLE(cur, "FIRMS")               		//CYCLE trought all firms of the sector
	{
		v[1]=VS(cur, "Firm_Market_Share");  //firm's market share
		v[0]=v[0]+v[1]*v[1];            	//sums up firm's market share squared
	}
	v[2]=COUNT("FIRMS");
	if (v[2]!=1)
		v[3]=(v[0]-(1/v[2]))/(1- (1/v[2]));
	else	
		v[3]=1;
RESULT(v[3])


EQUATION("Sector_Participation")
/*
Sector participation over total gross value of production
*/
	v[0]=V("Sector_Sales");
	v[1]=V("Sector_Avg_Price");
	v[2]=V("Country_Total_Nominal_Production");
	v[3]= v[2]!=0? (v[0]*v[1])/v[2] : 0;
RESULT(v[3])


EQUATION("Sector_Turbulence")
/*
Sector Variable for Analysis
*/
	v[0]=0;                                           //initializes the CYCLE 
	CYCLE(cur, "FIRMS")                               //CYCLE trough all firms 
	{
	v[1]=VS(cur,"firm_date_birth");                   //firm's date of birth
 	if(v[1]==(double)t)                               //if the time period is the same of the firm's date of birth
 		v[4]=0;                                       //use zero
 	else                                              //if the time period is no the same of the firm's date of birth
 		{
 		v[2]=VS(cur,"Firm_Effective_Market_Share");   //firm's effective market share in current period
 		v[3]=VLS(cur,"Firm_Effective_Market_Share",1);//firm's effective market share in the last period
 		v[4]=abs(v[2]-v[3]);                          //returns the absolute value of the difference
 		}
 	v[0]=v[0]+v[4];                                   //sums up all absolute values for all firms
	}
RESULT(v[0])


EQUATION("Sector_Hedge_Share")
	v[0]=SUM("firm_hedge");
	v[1]=COUNT("FIRMS");
	v[2]= v[1]!=0? v[0]/v[1] : 0;
RESULT(v[2])


EQUATION("Sector_Speculative_Share")
	v[0]=SUM("firm_speculative");
	v[1]=COUNT("FIRMS");
	v[2]= v[1]!=0? v[0]/v[1] : 0;
RESULT(v[2])


EQUATION("Sector_Ponzi_Share")
	v[0]=SUM("firm_ponzi");
	v[1]=COUNT("FIRMS");
	v[2]= v[1]!=0? v[0]/v[1] : 0;
RESULT(v[2])


EQUATION("Sector_Hedge_Normalized_Share")
	v[0]=WHTAVE("firm_hedge", "Firm_Market_Share");		
	v[1]=COUNT("FIRMS");
	v[2]= v[1]!=0? v[0]/v[1] : 0;
RESULT(v[2])


EQUATION("Sector_Speculative_Normalized_Share")
	v[0]=WHTAVE("firm_speculative", "Firm_Market_Share");	
	v[1]=COUNT("FIRMS");
	v[2]= v[1]!=0? v[0]/v[1] : 0;
RESULT(v[2])


EQUATION("Sector_Ponzi_Normalized_Share")
	v[0]=WHTAVE("firm_ponzi", "Firm_Market_Share");	
	v[1]=COUNT("FIRMS");
	v[2]= v[1]!=0? v[0]/v[1] : 0;
RESULT(v[2])


EQUATION("Sector_Short_Term_Rate")
/*
Sector Variable for Analysis
Share of  credit grating that is short term
*/
v[0]=SUM("Firm_Effective_Loans");
v[1]=0;
CYCLE(cur, "FIRMS")
{
	v[2]=VS(cur, "Firm_Desired_Investment_Expenses");
	v[3]=VS(cur, "Firm_Effective_Loans");
	if(v[2]==0)
		v[1]=v[1]+v[3];
	else
		v[1]=v[1];
}
	v[4]= v[0]!=0? v[1]/v[0] : v[0];
RESULT(v[4])



/*****SECTOR AGGREGATES*****/


EQUATION("Sector_Sales")                                                               
RESULT(SUM("Firm_Sales"))

EQUATION("Sector_Inventories")                              
RESULT(SUM("Firm_Stock_Inventories"))

EQUATION("Sector_Effective_Production")                       
RESULT(SUM("Firm_Effective_Production"))

EQUATION("Sector_Productive_Capacity")                     
RESULT(SUM("Firm_Productive_Capacity"))

EQUATION("Sector_Taxation")
RESULT(SUM("Firm_Indirect_Tax"))

EQUATION("Sector_Profits")
RESULT(SUM("Firm_Net_Profits"))

EQUATION("Sector_Stock_Loans")
RESULT(SUM("Firm_Stock_Loans"))

EQUATION("Sector_Stock_Deposits")
RESULT(SUM("Firm_Stock_Deposits"))

EQUATION("Sector_Effective_Loans")
RESULT(SUM("Firm_Effective_Loans"))

EQUATION("Sector_Demand_Met")
/*
Percentage of demand fulfilled by each sector
*/
	v[0]=V("Sector_Effective_Orders");                     	//total effective orders of the sector
	v[1]=V("Sector_Sales");                                	//total sales of the sector
    v[2]= v[0]>0? v[1]/v[0] : 1;                        	//the percentage of the demand met by the sector will be the total sales over effective orders
RESULT(v[2])


EQUATION("Sector_Employment")
/*
Sum up firm's employment, given by firm's effective production over firm's avg productivity
*/
	v[0]=0;                                        		//initializes the CYCLE
	CYCLE(cur, "FIRMS")                            		//CYCLE trought the firms
	{
		v[1]=VS(cur, "Firm_Effective_Production");      //firm's effective production
		v[2]=VS(cur, "Firm_Avg_Productivity");   		//firm's productivity in the last period
		if(v[2]!=0)
			v[0]=v[0]+v[1]/v[2];                       	//sums up the ratio between effective production and productivity
		else
			v[0]=v[0];
	}
RESULT(v[0])


EQUATION("Sector_Bargain_Power")
	v[0]=CURRENT;                                                          	 			 //firm wage in the last period
	v[1]=V("annual_frequency");
	v[2]= fmod((double) t-1,v[1]);                                                      //divide the time period by the annual period parameter
	v[3]=V("sector_passthrough_productivity");
	v[4]=V("sector_passthrough_inflation");
	if(v[2]==0)                                                                      	 //if the rest of the above division is zero (beggining of the year, adjust wages)
		{
		v[5]=LAG_GROWTH(p, "Sector_Employment", v[1], 1);
		v[6]=V("sector_bargain_power_adjustment");
		if(v[5]>0)
			{
			v[7]=v[3]*(1+v[5]*v[6]);
			v[8]=v[4]*(1+v[5]*v[6]);
			}
		else if(v[5]<0)
			{
			v[7]=v[3]*(1+v[5]*v[6]);
			v[8]=v[4]*(1+v[5]*v[6]);				
			}
		else
			{
			v[7]=v[3];		
			v[8]=v[4];
			}
		}
	else                                                                             	 //if the rest of the division is not zero, do not adjust wages
		{
		v[7]=v[3];		
		v[8]=v[4];                                                                      //current wages will be the last period's
		}
	v[9]=min(1,max(v[7],0));
	v[10]=min(1,max(v[8],0));
	WRITE("sector_passthrough_productivity",v[9]);
	WRITE("sector_passthrough_inflation",v[10]);
RESULT(0)


/*****SECTOR AVERAGES, SD AND MAX*****/


EQUATION("Sector_Avg_Interest_Rate_Short_Term")
/*
Sector average interest rate on short term loans weighted by stock of short term loans
*/
	v[0]=WHTAVE("Firm_Interest_Rate_Short_Term", "Firm_Stock_Loans_Short_Term");
	v[1]=SUM("Firm_Stock_Loans_Short_Term");
	v[2]= v[1]!=0? v[0]/v[1] : 0;
RESULT(v[2])


EQUATION("Sector_Avg_Interest_Rate_Long_Term")
/*
Sector average interest rate on long term loans weighted by stock of long term loans
*/
	v[0]=WHTAVE("Firm_Interest_Rate_Long_Term", "Firm_Stock_Loans_Long_Term");
	v[1]=SUM("Firm_Stock_Loans_Long_Term");
	v[2]= v[1]!=0? v[0]/v[1] : 0;	
RESULT(v[2])


EQUATION("Sector_Avg_Competitiveness")
/*
Average competitiveness, weighted by firm's market share
*/
	v[0]=0;                                         	//initializes the CYCLE
	CYCLE(cur, "FIRMS")                             	//CYCLE trought all firms in the sector
	{
		v[1]=VS(cur, "Firm_Competitiveness");           //firm's competitiveness
		v[2]=VLS(cur, "Firm_Market_Share", 1);          //firm's market share in the last period
		v[0]=v[0]+v[1]*v[2];                          	//sector average competitiveness will be a average of firms competitiveness weighted by their respective market shares
	}
RESULT(v[0])


EQUATION("Sector_Avg_Price")
RESULT(WHTAVE("Firm_Price", "Firm_Market_Share"))

EQUATION("Sector_Avg_Wage")
RESULT(WHTAVE("Firm_Wage", "Firm_Market_Share"))

EQUATION("Sector_Max_Productivity")        
RESULT(MAX("Firm_Frontier_Productivity"))

EQUATION("Sector_Avg_Markup")
RESULT(WHTAVE("Firm_Effective_Markup", "Firm_Market_Share"))

EQUATION("Sector_Avg_Productivity")
RESULT(WHTAVE("Firm_Avg_Productivity", "Firm_Market_Share"))

EQUATION("Sector_Avg_Debt_Rate")
RESULT(WHTAVE("Firm_Debt_Rate", "Firm_Market_Share"))

EQUATION("Sector_Avg_Max_Debt_Rate")
RESULT(WHTAVE("Firm_Max_Debt_Rate", "Firm_Market_Share"))

EQUATION("Sector_Avg_Liquidity_Rate")
RESULT(WHTAVE("Firm_Liquidity_Rate", "Firm_Market_Share"))

EQUATION("Sector_Max_Quality")
RESULT(MAX("Firm_Quality"))

EQUATION("Sector_Avg_Quality")
RESULT(WHTAVE("Firm_Quality", "Firm_Market_Share"))

EQUATION("Sector_Avg_Financial_Position")
RESULT(WHTAVE("Firm_Financial_Position", "Firm_Market_Share"))

EQUATION("Sector_Avg_Modernization_Rate")
RESULT(WHTAVE("Firm_Modernization_Rate", "Firm_Market_Share"))

EQUATION("Sector_Avg_Investment_Rate")
RESULT(WHTAVE("Firm_Investment_Rate", "Firm_Market_Share"))

EQUATION("Sector_Avg_Investment_Constraint_Rate")
RESULT(WHTAVE("Firm_Investment_Constraint_Rate", "Firm_Market_Share"))

EQUATION("Sector_SD_Investment_Rate")
RESULT(SD("Firm_Investment_Rate"))

EQUATION("Sector_Avg_Internal_Finance_Rate")
RESULT(WHTAVE("Firm_Internal_Finance_Rate", "Firm_Market_Share"))

EQUATION("Sector_Avg_External_Finance_Rate")
RESULT(WHTAVE("Firm_External_Finance_Rate", "Firm_Market_Share"))

EQUATION("Sector_Capacity_Utilization")
RESULT(WHTAVE("Firm_Capacity_Utilization", "Firm_Market_Share"))

EQUATION("Sector_Idle_Capacity")
RESULT(1-V("Sector_Capacity_Utilization"))

