

EQUATION("Firm_Productive_Capacity_Depreciation")
/*
Sum up the firm's productive capacity that was depreciated in each time step, not only investment period, and delete depreciated capital goods.
*/
	v[0]=0;																				//initializes the CYCLE on Capital. Will count the productiv capacity to depreciate
	v[1]=V("depreciation_period");
	CYCLE_SAFE(cur1, "CAPITALS")
	{
		v[2]=VS(cur1, "capital_good_date_birth");										//capital good date of birth	
		v[3]=VS(cur1, "capital_good_productive_capacity");								//capital good's prductive capacity
		if((double)t>=v[2]+v[1])														//if the current time step is higher than the date of birth of the capital good plus the depreciation period, this capital good must be depreciated
			{
			v[0]=v[0]+v[3];																//sum up the capital good productive capacity to the total productive capacity to be depreciated
			DELETE(cur1);																//delete the capital good
			}
		else																			//if the current time step is not higher than the date of birth of the capital good plus the depreciation period, this capital good must remain
			v[0]=v[0];																	//do not sum the capital good productive capacity to the total to be depreiated
	}                                                                    
RESULT(v[0])


EQUATION("Firm_Effective_Productive_Capacity_Variation")
/*
Depends on the demand for productive capacity in the last investment period. This will be added to the firm's productive capacity. Can be restricted by the demand not met of the domestic sector and international reserves
*/
	v[0]=V("Firm_Investment_Period");													//if it is investment period for the firm
	if(v[0]==1)
		{
		v[1]=V("investment_period");													//investment period
		cur=SEARCH_CNDS(root, "id_capital_goods_sector", 1);							//search the capital goods sector
		v[2]=0;
		for (v[3]=0; v[3]<=(v[1]-1); v[3]=v[3]+1)										//for the current production period until the last investment period -1
			{
			v[4]=VLS(cur, "Sector_Demand_Met", v[3]);									//computates the demand met by the sector in the current lag
			v[5]=VLS(cur, "Sector_Demand_Met_By_Imports", v[3]);						//computates the demand met by imports in the current lag
			v[6]=VL("Firm_Demand_Capital_Goods", v[3]);
			v[7]=v[6]*(v[4]+(1-v[4])*v[5]);
			v[2]=v[2]+v[7];
			}	
		}
	else
		v[2]=0;
RESULT(v[2])


EQUATION("Firm_Productive_Capacity")
/*
In this variable, the firm receive the new capital goods ordered in the last investment period, including the ones for modernization and the ones for expansion. Then, it counts the total productive capacity of old and new capital goods.
*/
	v[0]=V("investment_period");												//investment period
	v[1]=VL("Firm_Frontier_Productivity",v[0]);									//available technology when the capital good was ordered
	v[3]=V("Firm_Effective_Productive_Capacity_Variation");						//effective productive capacity variation
	v[4]=V("capital_output_ratio");												//capital output ratio
	v[5]=v[3]*v[4];																//amount of Capital Goods bought
	v[7]=V("Firm_Investment_Period");											//if it is investment period for the firm
	if(v[7]==1 && v[3]>0)
		{
		CYCLE_SAFE(cur, "CAPITALS")												//CYCLE trought Capital for modernization															
			{
			v[11]=VS(cur, "capital_good_to_replace");							//paramter that identifies if the current capital good must be replaced or not
			if (v[11]==1)																																		//if the current capital good must be replaced (this in fact does not create new objects, it simply rewrites the current capital object with the values of a new one
			DELETE(cur);
			}																																								//end the CYCLE for modernization
		for(v[13]=0; v[13]<=v[5]; v[13]=v[13]+1)								//for the amount of new capital goods bought
			{
			cur=ADDOBJ("CAPITALS");												//create new capital objects
			WRITES(cur, "capital_good_productivity_initial", v[1]);				//writes the new capital productivity as the frontier productivity when it was ordered
			WRITES(cur, "capital_good_productive_capacity", (1/v[4]));			//writes the productive capacity as the inverse of current capital output ratio of the sector
			WRITES(cur, "capital_good_date_birth", (double)t);					//writes the new capital date of birth as the current time period
			WRITES(cur, "capital_good_to_replace", 0);							//writes the parameter that identifies the capital goods to be replaced as zero
			WRITELS(cur, "Capital_Good_Acumulated_Production", 0, 1);			//writes the past acumulated production of the current new capital as zero
			}
		}
	i=COUNT("CAPITALS");														//count the number of capital goods
	if (i!=0)																																					//if it is not zero
		v[15] = SUM("capital_good_productive_capacity");						//sum uo their productive capacity
	else																																							//if its zero
		v[15]=0;																																				//firm's productive capacity will be zero
RESULT(v[15])


EQUATION("Firm_Capital")
/*
Nominal value of firm's total capital
*/
	v[0]=V("Firm_Productive_Capacity");                       					//firm's productive capacity in the last period   
	v[1]=V("Price_Capital_Goods");                       						//price of capital goods
	v[2]=V("capital_output_ratio");                      						//capital output ratio 
	v[3]=v[0]*v[1]*v[2];                                 						//nominal value of firm's total capital
RESULT(v[3])


EQUATION("Firm_Avg_Productivity")
/*
Firm's productivity will be an average of each capital good productivity weighted by their repective production	
*/
	v[0]=V("Firm_Effective_Production");                                		//firm's effective production
	v[1]=VL("Firm_Avg_Productivity", 1);                           				//firm's average productivity in the last period
	v[2]=0;                                                        				//initializes the CYCLE
	v[3]=0;                                                        				//initializes the CYCLE
	CYCLE(cur, "CAPITALS")                                          			//CYCLE trought firm's capital goods
	{
		v[4]=VS(cur, "Capital_Good_Productivity");                   			//capital good productivity
		v[5]=VS(cur, "Capital_Good_Production");                    			//capital good production
		v[2]=v[2]+v[4]*v[5];                                        			//sums up the product of each capital good productivity and production
		v[3]=v[3]+v[5];                                             			//sums up the production of each capital good
	}
	if (v[3]!=0)                                                   				//if the sum of the production is not zero
		v[6]=v[2]/v[3];                                             			//firm's average productivity will be the average of each capital good productivity weighted by its respective production
	else                                                           				//if the sum of the production is zero
		v[6]=v[1];                                                   			//firm's average production will be the last period's 
RESULT(v[6])


EQUATION("Firm_Max_Productivity");
/*
Maximum productivity among firm's capital goods
*/
	i=COUNT("CAPITALS");
	if( i != 0)
		v[0]=MAX("Capital_Good_Productivity");
	else
		v[0]=0;
RESULT(v[0])







