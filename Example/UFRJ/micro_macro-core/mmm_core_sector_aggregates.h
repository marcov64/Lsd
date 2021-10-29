/*****SECTOR ANALYSIS*****/


EQUATION("Sector_Inverse_HHI")
/*
Sector Variable for Analysis
*/
	v[0]=0;                           		//initializes the CYCLE    
	CYCLE(cur, "FIRMS")               		//CYCLE trought all firms of the sector
	{
		v[1]=VS(cur, "Firm_Market_Share");  //firm's market share
		v[0]=v[0]+v[1]*v[1];            	//sums up firm's market share squared
	}
	if(v[0]!=0)                       		//if the sum of the market share squared is not zero
		v[2]=1/v[0];                    	//the index is the inverse of the sum of the market share squared
	else                              		//if the sum of the market share squared is zero
		v[2]=0;                         	//the index is zero
RESULT(v[2])


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


EQUATION("Sector_Number_Firms")
/*
Sector Variable for Analysis
*/
	v[0]=COUNT("FIRMS");
RESULT(v[0])


EQUATION("Sector_Participation")
/*
Sector participation over total gross value of production
*/
	v[0]=V("Sector_Sales");
	v[1]=V("Sector_Avg_Price");
	v[2]=V("Gross_Value_Production");
	if(v[2]!=0)
		v[3]=(v[0]*v[1])/v[2];
	else
		v[3]=0;
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

EQUATION("Sector_Quality_Growth")
/*
Sector Avg Quality Annual Growth
*/
	v[0]=V("annual_period");
	v[1]=VL("Sector_Avg_Quality",1);
	v[2]=VL("Sector_Avg_Quality",(v[0]+1));
	v[3]=(v[1]/v[2])-1;                                                                              
RESULT(v[3])


/*****SECTOR AGGREGATES*****/


EQUATION("Sector_Sales")                                                            
RESULT(SUM("Firm_Sales"))

EQUATION("Sector_Inventories")                               
RESULT(SUM("Firm_Stock_Inventories"))

EQUATION("Sector_Effective_Production")                     
RESULT(SUM("Firm_Effective_Production"))

EQUATION("Sector_Productive_Capacity")                    
RESULT(SUM("Firm_Productive_Capacity"))

EQUATION("Sector_Sum_Market_Share")
RESULT(SUM("Firm_Market_Share"))

EQUATION("Sector_Taxation")
RESULT(SUM("Firm_Indirect_Tax"))


EQUATION("Sector_Demand_Met")
/*
Percentage of demand fulfilled by each sector
*/
	v[0]=V("Sector_Effective_Orders");                     	//total effective orders of the sector
	v[1]=V("Sector_Sales");                                	//total sales of the sector
  	if(v[0]>0)                                           	//if effective orders is positive
    	v[2]=v[1]/v[0];                                    	//the percentage of the demand met by the sector will be the total sales over effective orders
  	else                                                 	//if effective orders is not positive
    	v[2]=1;                                            	//the percentage of the demand met by the sector is 100%
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
			v[0]=v[0]+(v[1]/v[2]);                      //sums up the ratio between effective production and productivity
		else
			v[0]=v[0];
	}
RESULT(v[0])


EQUATION("Sector_Potential_Employment")
/*
Sum up firm's potential employment, gigvn by firm's productive capacity over firm's avg productivity
*/
	v[0]=0;                                        					//initializes the CYCLE
	CYCLE(cur, "FIRMS")                            					//CYCLE trought the firms
	{
		v[1]=0;
		CYCLES(cur, cur1, "CAPITALS")
		{
		v[2]=VS(cur1, "capital_good_productive_capacity");         	//firm's productive capacity
		v[3]=VS(cur1, "Capital_Good_Productivity"); 			   	//firm's productivity in the last period
		if(v[3]!=0)
			v[1]=v[1]+(v[2]/v[3]);                   				//sums up the ratio between effective production and productivity
		else
			v[1]=v[1];
		}
		v[0]=v[0]+v[1];
	}
RESULT(v[0])


EQUATION("Sector_Unemployment")
/*
Unemployment, calculated as the difference between effective employment and potential employment of the sector, in percentage value
*/
	v[0]=V("Sector_Employment");
	v[1]=V("Sector_Potential_Employment");
	if (v[1]!=0)
		v[2]=max(0,((v[1]-v[0])/v[1]));
	else
		v[2]=0;
RESULT(v[2])


	

/*****SECTOR AVERAGES AND MAX*****/


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

EQUATION("Sector_Max_Quality")
RESULT(MAX("Firm_Quality"))

EQUATION("Sector_Avg_Quality")
RESULT(WHTAVE("Firm_Quality", "Firm_Market_Share"))



