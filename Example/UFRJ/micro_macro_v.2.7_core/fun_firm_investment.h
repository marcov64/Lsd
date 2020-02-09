
EQUATION("Investment_Period_Firm")
/*
This variable writes for each firm, if it is investment period or not, assuming 1 or 0.
*/
	v[0]=V("investment_period");							//sector investment period parameter
	v[1]=fmod((t+v[0]),v[0]);								//devides the current time step by the investment period and takes the rest
	v[2]=V("id_firm_number");								//firm number
	v[3]=fmod((v[2]+v[0]),v[0]);							//divides the firm number by the investment period and takes the rest
	if (v[3]==v[1])											//if the firm number rest matches the time step rest
		v[4]=1;												//mark as investment period for the firm	
	else													//if the rests do not match
		v[4]=0;												//do not mark as investment period for the firm
RESULT(v[4])


EQUATION("Expected_Sales_Long_Term")
/*
The sectors update their productive capacity from year to year, time it takes for new capacity to be produced and to be available for use. The variation in the desired productive capacity is then defined according to the expected orders for the following year. Expected orders are calculated from the average of actual orders for the last six periods and the growth projection for six periods thereafter. This projection is determined on the basis of the difference between the averages of orders for the last six periods and the six periods prior to them, on which a doubling annual extrapolation factor applies.
*/
	v[0]=V("investment_period");
	v[3]=0;													//initializes the sum
	for (v[1]=0; v[1]<=(v[0]-1); v[1]=v[1]+1)				//from 0 to investment period-1 lags
		{
		v[2]=VL("Effective_Orders", v[1]);					//computates firm's effective orders of the current lag
		v[3]=v[3]+v[2];										//sum up firm's lagged effective oders
		}
	v[4]=v[3]/v[0];											//average firm's effective orders of the last investment period
	
	v[5]=0;													//initializes the sum
	for (v[6]=v[0]; v[6]<=(2*v[0]-1); v[6]=v[6]+1)			//from investment period lag to twice investment period-1 lags
		{
		v[7]=VL("Effective_Orders", v[6]);					//computates firm's effective orders of the current lag
		v[5]=v[5]+v[7];										//sum up firm's lagged effective oders
		}
	v[8]=v[5]/v[0];											//average firm's effective orders of the investment period prior to the last
		
	v[9]=V("gama");                                 		//expectations parameter
	if (v[8]!=0)                                    		//if the average effective orders of the six periods prior to the last six is not zero
		{
		v[10]=v[4]*(1+2*v[9]*((v[4]-v[8])/v[8]));     		//expected orders for the next six periods will be the average effective orders of the last six periods multiplied by the growth rate between the averages and the double of the expectation parameter
		v[11]=max(0,v[10]);                           		//expected orders for the next six periods can never be negative
		}
	else                                            		//if the average effective orders of the six periods prior to the last six is zero
		v[11]=0;                                      		//expected orders for the next six periods will be zero
RESULT(v[11])


EQUATION("Desired_Expansion_Investment_Expenses")
/*
Nominal value of desired new capital goods. 
*/

	v[9]=V("Investment_Period_Firm");						//investment period for the firm
	if(v[9]==1)
		{
   		v[0]=V("Expected_Sales_Long_Term");																					 	
   		v[1]=V("Productive_Capacity"); 																								
   		v[2]=V("desired_inventories_proportion");																				
   		v[3]=V("desired_degree_capacity_utilization");
		v[4]=(v[0]/v[3]) - v[1];							//desired productive capacity will be the amount needed based on expected sales minus existing 
   		v[5]=max(0,v[4]);									//cannot be negative
   		v[6]=V("Price_Capital_Goods");
		v[7]=V("capital_output_ratio");
   		v[8]=v[5]*v[6]*v[7];								//desired expansion expenses is the nominal value of the capital goods to meet desired productive capacity
   		}
   	else
   		v[8]=0;
RESULT(v[8])


EQUATION("Effective_Expansion_Investment_Expenses")
/*
Nominal value of new capital goods, restricted to the amount of funds available.
*/
	v[0]=V("Desired_Expansion_Investment_Expenses");
	v[1]=V("Funds");
	v[3]=V("Available_Debt");
	v[4]=V("Available_Financial_Assets");
	if(v[1]<=0)												//no available funds
		v[2]=0;
	else
		{
		if(v[1]>=v[0]) 										//available funds and enought for the investment
			v[2]=v[0];
		else 												//available funds but not enought for the desired investment
			v[2]=v[1];
		}
RESULT(v[2])


EQUATION("Demand_Productive_Capacity_Expansion")
/*
Effective productive capacity demanded, in real values 
*/
	v[0]=V("Effective_Expansion_Investment_Expenses");
	v[1]=V("Price_Capital_Goods");
	v[2]=V("capital_output_ratio");
	v[3]=(v[0]/v[1])/v[2];
RESULT(v[3])


EQUATION("Available_Funds_Replacement")
/*
Available funds for replacement investment will be the available funds minus the amount of effective expansion investment.
*/
	v[1]=V("Funds");
	v[2]=V("Effective_Expansion_Investment_Expenses");
	v[3]=max(0, (v[1]-v[2]));
RESULT(v[3])


EQUATION("Demand_Productive_Capacity_Replacement")
/*
New productive capacity in aquisition of new equipment to replace obsolete ones.
*/
	v[0]=V("Available_Funds_Replacement");
	v[1]=V("investment_period");
	v[3]=V("Investment_Period_Firm");
	v[4]=V("Frontier_Productivity");
	v[5]=V("learning_adjustment");
	v[6]=V("antecipation");
	v[7]=V("Max_Capital_Goods_Productivity");
	v[8]=max(v[4]*(1+v[6]*v[5]),v[7]);
	v[9]=V("depreciation");
	v[10]=V("Wage");
	v[11]=V("Price_Capital_Goods");
	v[12]=V("capital_output_ratio");
	v[13]=V("payback");
  				
	if(v[3]==1)														// if it is investment period for the firm
	{
		v[14]=0;													// initializes the CYCLE for expenses 
		v[15]=v[0];													// count available funds
		v[16]=0;													// initializes the CYCLE for productive capacity
		SORT("CAPITAL","Capital_Good_Productivity","UP");			// sort capital goods from the lowest to highest 
		CYCLE(cur, "CAPITAL")										// CYCLE trought capital goods
		{
			v[19]=VS(cur, "capital_good_date_birth");				// capital good date of birth
			if((double)t!=v[19] && (double)t < v[9]+v[19]-v[1])		// if the capitalgood was not created in the current period nor will depreciate in the next investment period
				{
				v[17]=VS(cur, "Capital_Good_Productivity");			// current capital good productivivty
				v[18]=VS(cur, "capital_good_productive_capacity");  // current capital good productive capacity  
				v[20]=v[18]*v[12]*v[11];							// current nominal cost of new capital goods to replace that amount of productive capacity
				v[15]=v[0]-v[14];									// subtract the replacement cost from the available funds to replacement. At the end this will be available funds after replacemenr
				v[23]=v[11]/(v[10]*((1/(v[17]))-(1/(v[8]))));		// calculates the payback
                 
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


EQUATION("Replacement_Expenses")
/*
Nominal value of desired new capital goods for modernization. 
*/
	v[1]=V("Demand_Productive_Capacity_Replacement");
	v[2]=V("Price_Capital_Goods");
	v[3]=V("capital_output_ratio");
	v[4]=v[1]*v[3]*v[2];
RESULT(v[4])


EQUATION("Available_Funds_After_Replacement")
/*
Available funds after replacement investment will be the available funds for replacement minus the amount of effective replacmenet investment.
*/
	v[1]=V("Available_Funds_Replacement");
	v[2]=V("Replacement_Expenses");
	v[3]=max(0, (v[1]-v[2]));
RESULT(v[3])


EQUATION("Demand_Capital_Goods")
/*
The demand for capital goods in each period will be determined by the sum of 1/6 of the variation of the productive capacity with modernization
*/
	v[0]=V("investment_period");
	v[4]=0;																					//initializes the sum
	for (v[1]=1; v[1]<=v[0]; v[1]=v[1]+1)													//from the last production period until the last investment period
		{
		v[2]=VL("Demand_Productive_Capacity_Replacement", v[1]);							//computates desired productive capacity to replace of the current lag
		v[3]=VL("Demand_Productive_Capacity_Expansion", v[1]);								//computates desired productive capacity to expand of the current lag
		v[4]=v[4]+v[2]+v[3];																//sum up firm's lagged  desired productive capacity to replace plus productive capacity to expand
		}
	v[5]=v[4]/v[0];																			//divides the total amount of demand by the invesmet period. This is because capital goods take a whole investment period to be produced. This distributed the demand for the capital goods firms equally during the investment period. It does not mean that each firm demands 1/6 of capital each period, it's just to distribute the production															
	v[6]=V("capital_output_ratio");															//capital outrput ratio
	v[7]=v[5]*v[6];																			//number of capital goods to be demanded (produced) in each production period inside the investment period
RESULT(v[7])




