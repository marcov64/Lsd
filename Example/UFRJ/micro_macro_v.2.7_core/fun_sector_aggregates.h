/*****SECTOR ANALYSIS*****/


EQUATION("Inverse_Herf_Index")
/*
Sector Variable for Analysis
*/
	v[0]=0;                           	//initializes the CYCLE    
	CYCLE(cur, "FIRMS")               	//CYCLE trought all firms of the sector
	{
		v[1]=VS(cur, "Market_Share");   //firm's market share
		v[0]=v[0]+v[1]*v[1];            //sums up firm's market share squared
	}
	if(v[0]!=0)                       	//if the sum of the market share squared is not zero
		v[2]=1/v[0];                    //the index is the inverse of the sum of the market share squared
	else                              	//if the sum of the market share squared is zero
		v[2]=0;                         //the index is zero
RESULT(v[2])


EQUATION("Number_Firms")
/*
Sector Variable for Analysis
*/
	v[0]=COUNT("FIRMS");
RESULT(v[0])


EQUATION("Sector_Participation")
/*
Sector participation over total gross value of production
*/
	v[0]=V("Sales_Sector");
	v[1]=V("Avg_Price");
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
 		v[2]=VS(cur,"Effective_Market_Share");        //firm's effective market share in current period
 		v[3]=VLS(cur,"Effective_Market_Share",1);     //firm's effective market share in the last period
 		v[4]=abs(v[2]-v[3]);                          //returns the absolute value of the difference
 		}
 	v[0]=v[0]+v[4];                                   //sums up all absolute values for all firms
	}
RESULT(v[0])


/*****SECTOR AGGREGATES*****/


EQUATION("Sales_Sector")                          
/*
Sum up the sales of all firms in the sector
*/
	v[0]=SUM("Sales");                                     
RESULT(v[0])


EQUATION("Demand_Met_Sector")
/*
Percentage of demand fulfilled by each sector
*/
	v[0]=V("Effective_Orders_Sector");                     	//total effective orders of the sector
	v[1]=V("Sales_Sector");                                	//total sales of the sector
  	if(v[0]>0)                                           	//if effective orders is positive
    	v[2]=v[1]/v[0];                                    	//the percentage of the demand met by the sector will be the total sales over effective orders
  	else                                                 	//if effective orders is not positive
    	v[2]=1;                                            	//the percentage of the demand met by the sector is 100%
RESULT(v[2])


EQUATION("Inventories_Sector")
/*
Sum of the Inventories 
*/
	v[0]=SUM("Inventories");                                
RESULT(v[0])


EQUATION("Effective_Production_Sector")
/*
Sum of the Effective Production 
*/
	v[0]=SUM("Effective_Production");                        
RESULT(v[0])


EQUATION("Productive_Capacity_Sector")
/*
Sum of the Productive Capacity 
*/
	v[0]=SUM("Productive_Capacity");                     
RESULT(v[0])


EQUATION("Sum_Market_Share")
/*
Sum of the Market shares: this variable works to verify the Fischer Equation
*/
	v[0]=SUM("Market_Share");
RESULT(v[0])


EQUATION("Employment_Sector")
/*
Sum up firm's employment, given by firm's effective production over firm's avg productivity
*/
	v[0]=0;                                        		//initializes the CYCLE
	CYCLE(cur, "FIRMS")                            		//CYCLE trought the firms
	{
		v[1]=VS(cur, "Effective_Production");        	//firm's effective production
		v[2]=VLS(cur, "Firm_Avg_Productivity", 1);   	//firm's productivity in the last period
		v[0]=v[0]+(v[1]/v[2]);                       	//sums up the ratio between effective production and productivity
	}
RESULT(v[0])


EQUATION("Potential_Employment_Sector")
/*
Sum up firm's potential employment, gigvn by firm's productive capacity over firm's avg productivity
*/
	v[0]=0;                                        					//initializes the CYCLE
	CYCLE(cur, "FIRMS")                            					//CYCLE trought the firms
	{
		v[1]=0;
		CYCLES(cur, cur1, "CAPITAL")
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


EQUATION("Unemployment_Sector")
/*
Unemployment, calculated as the difference between effective employment and potential employment of the sector, in percentage value
*/
	v[0]=V("Employment_Sector");
	v[1]=V("Potential_Employment_Sector");
	if (v[1]!=0)
		v[2]=max(0,((v[1]-v[0])/v[1]));
	else
		v[2]=0;
RESULT(v[2])


EQUATION("Bargain_Power")
/*
Determines the productivity passthrough. If the employment in the sector is increasing, it increases the passtrhrough. Otherwise, it decreases.
*/
	v[0]=V("annual_period");								//annual period parameters
	v[1]=fmod((double)t,v[0]);							    //divides time step by the annual period and takes the rest
	if (v[1]==0)											//if it is the beggening of a new year
		{
		v[2]=VL("Employment_Sector",1);						//sector employment in the last period
		v[3]=VL("Employment_Sector",(v[0]+1));				//sector employment in the beggining of the last year
		v[4]=v[2]-v[3];										//check if sector employment has increased or decreased
		v[5]=VL("passthrough_productivity", 1);				//productivity passtrought in the last period
		v[6]=VL("passthrough_inflation", 1);				//inflation passtrought in the last period
		v[7]=V("bargain_power_adjustment_parameter");		//bargain power adjustment
		v[10]=V("minimum_passthrough");						//minimum accepted passtrought
			if (v[4]<0)										//if sector employemnt has decreased
				{
				v[8]=max((min(1,(v[5]-v[7]))),v[10]);		//reduce productivity passtrought
				v[9]=max((min(1,(v[6]-v[7]))),v[10]);		//reduce inflation passtrought
				WRITE("passthrough_productivity", v[8]);	//writes the new passtrought
				//WRITE("passthrough_inflation", v[9]);		//writes the new passtrought
				}
			else											//if sector employment has increased
				{
				v[8]=max((min(1,(v[5]+v[7]))),v[10]);		//reduce productivity passtrought
				v[9]=max((min(1,(v[6]+v[7]))),v[10]);		//reduce inflation passtrought
				WRITE("passthrough_productivity", v[8]);	//writes the new passtrought
				//WRITE("passthrough_inflation", v[9]);		//writes the new passtrought
				}
		}
	else
	{
	v[8]=0;
	v[9]=0;
	}
RESULT(0)
	

/*****SECTOR AVERAGES AND MAX*****/


EQUATION("Avg_Price")
/*
Average weighted by firm's market share
*/
	v[0]=WHTAVE("Price", "Market_Share");
RESULT(v[0])


EQUATION("Avg_Wage_Sector")
/*
Sector average nominal wage, weighted by firm's market share
*/
	v[0]=WHTAVE("Wage", "Market_Share");
RESULT(v[0])


EQUATION("Avg_Competitiveness_Sector")
/*
Average competitiveness, weighted by firm's market share
*/
	v[0]=0;                                         	//initializes the CYCLE
	CYCLE(cur, "FIRMS")                             	//CYCLE trought all firms in the sector
	{
		v[1]=VS(cur, "Competitiveness");              	//firm's competitiveness
		v[2]=VLS(cur, "Market_Share", 1);             	//firm's market share in the last period
		v[0]=v[0]+v[1]*v[2];                          	//sector average competitiveness will be a average of firms competitiveness weighted by their respective market shares
	}
RESULT(v[0])


EQUATION("Max_Productivity_Sector")
/*
Maximum productivity of the sector will be the higher between firm's frontier productivities
*/
	v[0]=MAX("Frontier_Productivity");          
RESULT(v[0])


EQUATION("Avg_Markup_Sector")
/*
Average markup of the sector, weighted by firm's effective market share
*/
	v[0]=WHTAVE("Effective_Markup", "Effective_Market_Share");
RESULT(v[0])


EQUATION("Avg_Productivity_Sector")
/*
Sector average productivity will be the average of firms productivity weighted by their market shares
*/
	v[0]=WHTAVE("Firm_Avg_Productivity", "Market_Share");
RESULT(v[0])


EQUATION("Avg_Debt_Rate_Sector")
/*
Sector average debt rate will be the average of firms productivity weighted by their market shares
*/
	v[0]=WHTAVE("Debt_Rate", "Market_Share");
RESULT(v[0])


EQUATION("Max_Quality_Sector")
/*
Maximum firm quality of the sector.
*/
	v[0]=MAX("Quality");
RESULT(v[0])


EQUATION("Avg_Quality_Sector")
/*
Average of the firm's quality weighted by their market share
*/
	v[0]=WHTAVE("Quality", "Market_Share");
RESULT(v[0])



