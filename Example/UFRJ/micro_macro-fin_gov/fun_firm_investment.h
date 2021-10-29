
EQUATION("Firm_Investment_Period")
/*
This variable writes for each firm, if it is investment period or not, assuming 1 or 0.
*/
	v[0]=V("sector_investment_frequency");						//sector investment period parameter
	v[1]=fmod((t+v[0]),v[0]);								//devides the current time step by the investment period and takes the rest
	v[2]=V("firm_id");								//firm number
	v[3]=fmod((v[2]+v[0]),v[0]);							//divides the firm number by the investment period and takes the rest
	if (v[3]==v[1])											//if the firm number rest matches the time step rest
		v[4]=1;												//mark as investment period for the firm	
	else													//if the rests do not match
		v[4]=0;												//do not mark as investment period for the firm
RESULT(v[4])


EQUATION("Firm_Productive_Capacity_Depreciation")
/*
Sum up the firm's productive capacity that was depreciated in each time step, not only investment period, and delete depreciated capital goods.
*/
	v[0]=0;																				//initializes the CYCLE on Capital. Will count the productiv capacity to depreciate
	v[1]=V("sector_capital_duration");
	CYCLE_SAFE(cur1, "CAPITALS")
	{
		v[2]=VS(cur1, "capital_good_date_birth");										//capital good date of birth	
		v[3]=VS(cur1, "capital_good_productive_capacity");								//capital good's prductive capacity
		v[4]=VS(cur1, "capital_good_to_depreciate");
		v[5]=VS(cur1, "capital_good_depreciation_period");
		if((double)t>=v[5]&&v[4]!=1)													//if the current time step is higher than the date of birth of the capital good plus the depreciation period, this capital good must be depreciated
			{
			v[0]=v[0]+v[3];																//sum up the capital good productive capacity to the total productive capacity to be depreciated
			WRITES(cur1, "capital_good_productive_capacity", 0);
			WRITES(cur1, "Capital_Good_Productivity", 0);
			WRITES(cur1, "capital_good_to_depreciate", 1);
			}																
	}                                                                          
RESULT(v[0])


EQUATION("Firm_Effective_Productive_Capacity_Variation")
/*
Depends on the demand for productive capacity in the last investment period. This will be added to the firm's productive capacity. Can be restricted by the demand not met of the domestic sector and international reserves
*/
	v[0]=V("Firm_Investment_Period");													//if it is investment period for the firm
	if(v[0]==1)
		{
		v[1]=V("sector_investment_frequency");												//investment period
		v[6]=v[7]=0;
		for (i=1; i<=v[1]; i++)															//for the current production period until the last investment period -1
			{
			v[4]=VLS(capital, "Sector_Demand_Met", i);										//computates the demand met by the sector in the current lag
			v[5]=VLS(capital, "Sector_Demand_Met_By_Imports", i);							//computates the demand met by imports in the current lag
			v[6]=v[6]+v[4];
			v[7]=v[7]+v[5];
			}
		v[8]=v[6]/v[1];																	//average demand met
		v[9]=v[7]/v[1];																	//average demand met by imports
		
		v[10]=VL("Firm_Demand_Capital_Goods_Expansion", v[1]);
		v[11]=VL("Firm_Demand_Capital_Goods_Replacement", v[1]);
			
		v[12]=v[10]*(v[8]+(1-v[8])*v[9]);
		v[13]=v[11]*(v[8]+(1-v[8])*v[9]);
		v[14]=v[12]+v[13];
		}
	else
		{
		v[12]=0;
		v[13]=0;
		v[14]=0;
		}
	WRITE("Firm_Effective_Capital_Goods_Expansion", v[12]);
	WRITE("Firm_Effective_Capital_Goods_Replacement", v[13]);
RESULT(v[14])

EQUATION_DUMMY("Firm_Effective_Capital_Goods_Expansion", "Firm_Effective_Productive_Capacity_Variation")
EQUATION_DUMMY("Firm_Effective_Capital_Goods_Replacement", "Firm_Effective_Productive_Capacity_Variation")


EQUATION("Firm_Productive_Capacity")
/*
In this variable, the firm receive the new capital goods ordered in the last investment period, including the ones for modernization and the ones for expansion. Then, it counts the total productive capacity of old and new capital goods.
*/
	USE_ZERO_INSTANCE
	v[0]=V("sector_investment_frequency");										//investment period
	v[1]=VL("Firm_Frontier_Productivity",v[0]);									//available technology when the capital good was ordered
	v[2]=V("Firm_Effective_Capital_Goods_Expansion");	
	v[3]=V("Firm_Effective_Capital_Goods_Replacement");	
	v[5]=V("sector_capital_output_ratio");										//amount of Capital Goods bought
	v[8]=uniform_int(30, 50);
	
	if(v[2]>0)
		{
		for(i=0; i<=v[2]; i++)													//for the amount of new capital goods bought
			{
			cur=ADDOBJ("CAPITALS");												//create new capital objects
			WRITES(cur, "capital_good_productivity_initial", v[1]);				//writes the new capital productivity as the frontier productivity when it was ordered
			WRITES(cur, "capital_good_productive_capacity", (1/v[5]));			//writes the productive capacity as the inverse of current capital output ratio of the sector
			WRITES(cur, "capital_good_date_birth", t);							//writes the new capital date of birth as the current time period
			WRITES(cur, "capital_good_to_replace", 0);							//writes the parameter that identifies the capital goods to be replaced as zero
			WRITES(cur, "capital_good_to_depreciate", 0);
			WRITES(cur, "capital_good_depreciation_period", (t+v[8]));
			WRITELS(cur, "Capital_Good_Acumulated_Production", 0, 1);			//writes the past acumulated production of the current new capital as zero
			v[6]=v[6]+1;
			}																																		//end the CYCLE for modernization
		}
		
	if(v[3]>0)
		{
		SORT("CAPITALS","Capital_Good_Productivity","UP");
		CYCLE(cur, "CAPITALS")																																					// CYCLE trought capital goods
   			{
     		v[10]=VS(cur, "capital_good_to_replace");
     		if(v[10]==1&&v[3]>0)
     			{
     			WRITES(cur, "capital_good_productivity_initial", v[1]);				//writes the new capital productivity as the frontier productivity when it was ordered
				WRITES(cur, "capital_good_date_birth", t);							//writes the new capital date of birth as the current time period
				WRITES(cur, "capital_good_to_replace", 0);							//writes the parameter that identifies the capital goods to be replaced as zero
				WRITES(cur, "capital_good_to_depreciate", 0);
				WRITES(cur, "capital_good_depreciation_period", (t+v[8]));
				WRITELS(cur, "Capital_Good_Acumulated_Production", 0, 1);			//writes the past acumulated production of the current new capital as zero
     			v[3]=v[3]-1;
				}																																							// do not sum replacement cost
  			}
  		}
	
	v[19]=0;
	CYCLE_SAFE(cur, "CAPITALS")
	{
		v[20]=VS(cur, "capital_good_to_depreciate");
		if(v[20]==1)
			{
			DELETE(cur);
			v[19]=v[19]+1;
			}
	}
	v[22]=v[19]/v[5];
	WRITE("Firm_Depreciated_Productive_Capacity", v[22]);
	v[6]=COUNT("CAPITALS");
	v[15] = v[6]!=0? SUM("capital_good_productive_capacity") : 0;						//sum uo their productive capacity																															//firm's productive capacity will be zero
RESULT(v[15])


EQUATION_DUMMY("Firm_Depreciated_Productive_Capacity", "Firm_Productive_Capacity")


EQUATION("Firm_Capital")
/*
Nominal value of firm's total capital
*/
	v[0]=V("Firm_Productive_Capacity");                       					//firm's productive capacity in the last period   
	v[1]=V("Country_Capital_Goods_Price");                       				//price of capital goods
	v[2]=V("sector_capital_output_ratio");                      				//capital output ratio 
	v[3]=v[0]*v[1]*v[2];                                 						//nominal value of firm's total capital
RESULT(v[3])


EQUATION("Firm_Max_Productivity")
/*
Maximum productivity among firm's capital goods
*/
	v[0]=COUNT("CAPITALS");
	v[1]= v[0]!=0? MAX("Capital_Good_Productivity") : 0;
RESULT(v[1])


EQUATION("Firm_Expected_Sales_Long_Term")
/*
The sectors update their productive capacity from year to year, time it takes for new capacity to be produced and to be available for use. The variation in the desired productive capacity is then defined according to the expected orders for the following year. Expected orders are calculated from the average of actual orders for the last six periods and the growth projection for six periods thereafter. This projection is determined on the basis of the difference between the averages of orders for the last six periods and the six periods prior to them, on which a doubling annual extrapolation factor applies.
*/
	v[0]=V("sector_investment_frequency");
	v[1]=LAG_AVE(p,"Firm_Effective_Orders", v[0]);
	v[2]=LAG_AVE(p,"Firm_Effective_Orders", v[0], v[0]);	
	v[3]=V("sector_expectations");                          //firm expectations
	if (v[2]!=0)                                    		//if the average effective orders of the six periods prior to the last six is not zero
		v[4]=v[1]*(1+2*v[3]*((v[1]-v[2])/v[2]));     		//expected orders for the next six periods will be the average effective orders of the last six periods multiplied by the growth rate between the averages and the double of the expectation parameter
	else                                            		//if the average effective orders of the six periods prior to the last six is zero
		v[4]=0;                                      		//expected orders for the next six periods will be zero
RESULT(max(0,v[4]))


EQUATION("Firm_Desired_Expansion_Investment_Expenses")
/*
Nominal value of desired new capital goods. 
*/

	v[9]=V("Firm_Investment_Period");								//investment period for the firm
	if(v[9]==1)														//if it is investment period for the firm
		{
   		v[0]=V("Firm_Expected_Sales_Long_Term");					//firm expected sales																				 	
   		v[1]=V("Firm_Productive_Capacity"); 						//current productive capacity
		
		v[10]=0;													//initializes the cycle
		v[11]=V("sector_capital_duration");							//depreciation period
		v[12]=V("sector_investment_frequency");						//investment period
		
		CYCLE(cur, "CAPITALS")										//cycle trough all capital goods
		{
			v[15]=VS(cur, "capital_good_depreciation_period");
			v[13]=VS(cur, "capital_good_date_birth");				//current capital good date of birth
			v[14]=VS(cur, "capital_good_productive_capacity");		//current capital good productive capacity
			if((double)t!=v[13] && ((double)t+v[12]) > v[15])		//if current capital good will depreciate in the next investment period (before new capital goods arrive)
				v[10]=v[10]+v[14];									//sum up productive capacity to depreciate in the next investment period
			else													//if current capital good wilnot depreciate in the next investment period
				v[10]=v[10];										//do not sum up productive capacity
		}
		
   		v[2]=V("sector_desired_inventories_proportion");																										
   		v[3]=V("sector_desired_degree_capacity_utilization");
		v[4]=((v[0])/v[3]) - (v[1] - v[10]);						//desired productive capacity will be the amount needed based on expected sales minus existing plus what will depreciate in the next investment period
   		v[5]=max(0,v[4]);
		v[6]=V("Country_Capital_Goods_Price");						//price of capital goods
		v[7]=V("sector_capital_output_ratio");						//capital output ratio
   		v[8]=v[5]*v[6]*v[7];										//desired expansion expenses is the nominal value of the capital goods to meet desired productive capacity
		}
   	else
   		v[8]=0;
RESULT(v[8])


EQUATION("Firm_Desired_Replacement_Investment_Expenses")
/*
Nominal value of derired new capital goods for modernization replacement
*/
 v[1]=V("sector_investment_frequency");
 v[3]=V("Firm_Investment_Period");
 v[4]=V("Firm_Frontier_Productivity");
 v[5]=V("sector_learning_adjustment");
 v[6]=V("sector_antecipation");
 v[7]=V("Firm_Max_Productivity");
 v[8]=max(v[4]*(1+v[6]*v[5]),v[7]);
 
 v[9]=V("sector_capital_duration");
 v[10]=V("Firm_Wage");
 v[11]=V("Country_Capital_Goods_Price");
 v[12]=V("sector_capital_output_ratio");
 v[13]=V("sector_payback_period");
 
 v[30]=V("switch_interest_investment");
	if(v[30]==0)//no interest
	v[31]=0;
	if(v[30]==1)//use basic interest
	v[31]=V("Central_Bank_Basic_Interest_Rate");
	if(v[30]==2)//use firm interest
	v[31]=V("Firm_Interest_Rate_Long_Term");
  				
  if(v[3]==1)																																											// if it is investment period for the firm
  {
  v[16]=0;																																												// initializes the CYCLE for productive capacity
  SORT("CAPITALS","Capital_Good_Productivity","UP");																								// sort capital goods from the lowest to highest 
   CYCLE(cur, "CAPITALS")																																					// CYCLE trought capital goods
   {
     v[19]=VS(cur, "capital_good_depreciation_period");																										// capital good date of birth
     if((double)t!=v[19] && (double)t < v[19]-v[1])																					// if the capitalgood was not created in the current period nor will depreciate in the next investment period
     {
     v[17]=VS(cur, "Capital_Good_Productivity");																									// current capital good productivivty
     v[18]=VS(cur, "capital_good_productive_capacity");   																				// current capital good productive capacity  
     v[23]=(v[11]*(1+v[31]))/(v[10]*((1/(v[17]))-(1/(v[8]))));																								// calculates the payback           
         if(v[8]>v[17] && v[23]<=v[13])																														// if the cost of replacement is lower than current available funds and the paybakc calculus  is lower than the payback parameter
        	v[16]=v[16]+v[18];         																															// sum up the productive capacity to replace
         else																																											// else
         	v[16]=v[16]; 																																						// do not sum replacement cost  
      }
      else																																												// else
      	v[16]=v[16]; 																																							// do not sum replacement cost
   }
  }
  else																																														// if it is not investmnet period for the firm
  v[16]=0;																																												// productive capacity to replace is zero
  
  v[17]=v[16]*v[11]*v[12];																																				// nominal desired expenses
RESULT(v[17])


EQUATION("Firm_Desired_Investment_Expenses")
/*
Nominal value of total desired investment expenses, including replacement and expansion.
*/
	v[0]=V("Firm_Desired_Expansion_Investment_Expenses");
	v[1]=V("Firm_Desired_Replacement_Investment_Expenses");
	v[2]=v[0]+v[1];
RESULT(v[2])


EQUATION("Firm_Effective_Expansion_Investment_Expenses")
/*
Nominal value of new capital goods, restricted to the amount of funds available.
*/
	v[0]=V("Firm_Desired_Expansion_Investment_Expenses");
	v[1]=V("Firm_Total_Funds");
	v[2]=max(0,min(v[0],v[1]));
RESULT(v[2])


EQUATION("Firm_Demand_Productive_Capacity_Expansion")
/*
Effective productive capacity demanded, in real values 
*/
	v[0]=V("Firm_Effective_Expansion_Investment_Expenses");
	v[1]=V("Country_Capital_Goods_Price");
	v[2]=V("sector_capital_output_ratio");
	v[3]=(v[0]/v[1])/v[2];
RESULT(v[3])


EQUATION("Firm_Available_Funds_Replacement")
/*
Available funds for replacement investment will be the available funds minus the amount of effective expansion investment.
*/
	v[1]=V("Firm_Total_Funds");
	v[2]=V("Firm_Effective_Expansion_Investment_Expenses");
	v[3]=max(0, (v[1]-v[2]));
RESULT(v[3])


EQUATION("Firm_Demand_Productive_Capacity_Replacement")
/*
New productive capacity in aquisition of new equipment to replace obsolete ones.
*/
	v[0]=V("Firm_Available_Funds_Replacement");
	v[1]=V("sector_investment_frequency");
	v[3]=V("Firm_Investment_Period");
	v[4]=V("Firm_Frontier_Productivity");
	v[5]=V("sector_learning_adjustment");
	v[6]=V("sector_antecipation");
	v[7]=V("Firm_Max_Productivity");
	v[8]=max(v[4]*(1+v[6]*v[5]),v[7]);
	v[9]=V("sector_capital_duration");
	v[10]=V("Firm_Wage");
	v[11]=V("Country_Capital_Goods_Price");
	v[12]=V("sector_capital_output_ratio");
	v[13]=V("sector_payback_period");
	v[30]=V("switch_interest_investment");
	if(v[30]==0)//no interest
		v[31]=0;
	if(v[30]==1)//use basic interest
		v[31]=V("Central_Bank_Basic_Interest_Rate");
	if(v[30]==2)//use firm interest
		v[31]=V("Firm_Interest_Rate_Long_Term");
  				
	if(v[3]==1)														// if it is investment period for the firm
	{
		v[14]=0;													// initializes the CYCLE for expenses 
		v[15]=v[0];													// count available funds
		v[16]=0;													// initializes the CYCLE for productive capacity
		SORT("CAPITALS","Capital_Good_Productivity","UP");			// sort capital goods from the lowest to highest 
		CYCLE(cur, "CAPITALS")										// CYCLE trought capital goods
		{
			v[19]=VS(cur, "capital_good_depreciation_period");		// capital good date of birth
			if((double)t!=v[19] && (double)t < v[19]-v[1])			// if the capitalgood was not created in the current period nor will depreciate in the next investment period
				{
				v[17]=VS(cur, "Capital_Good_Productivity");			// current capital good productivivty
				v[18]=VS(cur, "capital_good_productive_capacity");  // current capital good productive capacity  
				v[20]=v[18]*v[12]*v[11];							// current nominal cost of new capital goods to replace that amount of productive capacity
				v[15]=v[0]-v[14];									// subtract the replacement cost from the available funds to replacement. At the end this will be available funds after replacemenr
				v[23]=(v[11]*(1+v[31]))/(v[10]*((1/(v[17]))-(1/(v[8]))));		// calculates the payback
                 
				if(v[8]>v[17] && v[20]<=v[15] && v[23]<=v[13])		// if the cost of replacement is lower than current available funds and the paybakc calculus  is lower than the payback parameter
					{
					WRITES(cur, "capital_good_to_replace",1);		// mark the current capital good to replace
					v[14]=v[14]+v[20];								// sum up the replacement cost
					v[16]=v[16]+v[18];         						// sum up the productive capacity to replace
					}
				else												// else
					{
					WRITES(cur, "capital_good_to_replace",0);		// do not mark the current capital good to replace
					v[16]=v[16]; 									// do not sum replacement cost
					v[14]=v[14];									// do not sum productive capacity to replace
					} 
				}
			else													// else
				{
				WRITES(cur, "capital_good_to_replace",0);			// do not mark the current capital good to replace
				v[16]=v[16]; 										// do not sum replacement cost
				v[14]=v[14];										// do not sum productive capacity to replace
				} 
		}
	}
	else															// if it is not investmnet period for the firm
		{
		v[14]=0;      												// nominal replacement expenses is zero																																								
		v[15]=v[0];													// available funds after replacement is the amount available before
		v[16]=0;													// productive capacity to replace is zero
		}	
RESULT(v[16])


EQUATION("Firm_Replacement_Expenses")
/*
Nominal value of desired new capital goods for modernization. 
*/
	v[1]=V("Firm_Demand_Productive_Capacity_Replacement");
	v[2]=V("Country_Capital_Goods_Price");
	v[3]=V("sector_capital_output_ratio");
	v[4]=v[1]*v[3]*v[2];
RESULT(v[4])


EQUATION("Firm_Effective_Investment_Expenses")
/*
Nominal value of total effective investment expenses, including replacement and expansion.
*/
	v[0]=V("Firm_Effective_Expansion_Investment_Expenses");
	v[1]=V("Firm_Replacement_Expenses");
	v[2]=v[0]+v[1];
RESULT(v[2])


EQUATION("Firm_Available_Funds_After_Replacement")
/*
Available funds after replacement investment will be the available funds for replacement minus the amount of effective replacmenet investment.
*/
	v[1]=V("Firm_Available_Funds_Replacement");
	v[2]=V("Firm_Replacement_Expenses");
	v[3]=max(0, (v[1]-v[2]));
RESULT(v[3])


EQUATION("Firm_Demand_Capital_Goods_Expansion")
/*
Number of capital goods demanded to expand productive capacity 
*/
	v[0]=V("Firm_Effective_Expansion_Investment_Expenses");
	v[1]=V("Country_Capital_Goods_Price");
	v[2]=v[0]/v[1];
RESULT(v[2])


EQUATION("Firm_Demand_Capital_Goods_Replacement")
/*
Number of capital goods demanded to modernie productive capacity 
*/
	v[1]=V("Firm_Demand_Productive_Capacity_Replacement");
	v[2]=V("sector_capital_output_ratio");
	v[3]=v[1]*v[2];
RESULT(v[3])


EQUATION("Firm_Demand_Capital_Goods")
/*
The demand for capital goods in each period will be determined by the sum of 1/6 of the variation of the productive capacity with modernization
*/
	v[0]=V("sector_investment_frequency");
	v[4]=0;																					//initializes the sum
	for (i=1; i<=v[0]; i++)																	//from the last production period until the last investment period
		{
		v[2]=VL("Firm_Demand_Capital_Goods_Expansion", i);									//computates desired productive capacity to replace of the current lag
		v[3]=VL("Firm_Demand_Capital_Goods_Replacement", i);								//computates desired productive capacity to expand of the current lag
		v[4]=v[4]+v[2]+v[3];																//sum up firm's lagged  desired productive capacity to replace plus productive capacity to expand
		}
	v[5]=v[4]/v[0];																			//divides the total amount of demand by the invesmet period. This is because capital goods take a whole investment period to be produced. This distributed the demand for the capital goods firms equally during the investment period. It does not mean that each firm demands 1/6 of capital each period, it's just to distribute the production															
RESULT(v[5])


EQUATION("Firm_Modernization_Rate")
/*
Percentage of productive capacity that is replaced for modernization at each time period.
*/
	v[0]=V("Firm_Demand_Productive_Capacity_Replacement");
	v[1]=V("Firm_Productive_Capacity");
	v[2]= v[1]!=0? v[0]/v[1] : 0;
RESULT(v[2])


EQUATION("Firm_Investment_Rate")
/*
Percentage of productive capacity that is replaced for modernization at each time period.
*/
	v[0]=V("Firm_Demand_Productive_Capacity_Replacement");
	v[1]=V("Firm_Demand_Productive_Capacity_Expansion");
	v[2]=V("Firm_Productive_Capacity");
	v[3]= v[2]!=0? (v[0]+v[1])/v[2] : 0;
RESULT(v[3])


EQUATION("Firm_Investment_Constraint_Rate")
/*
Percentage of productive capacity that is replaced for modernization at each time period.
*/
	v[0]=V("Firm_Effective_Investment_Expenses");
	v[1]=V("Firm_Desired_Investment_Expenses");
	v[2]= v[1]!=0? v[0]/v[1] : 0;
RESULT(v[2])












