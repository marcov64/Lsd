

EQUATION("Firm_Productive_Capacity_Depreciation")
/*
Sum up the firm's productive capacity that was depreciated in each time step, not only investment period, and delete depreciated capital goods.
*/
	v[0]=0;																				//initializes the CYCLE on Capital. Will count the productiv capacity to depreciate
	v[1]=V("sector_depreciation_period");
	v[4]=COUNT("CAPITALS");
	CYCLE_SAFE(cur1, "CAPITALS")
	{
		v[2]=VS(cur1, "capital_good_date_birth");										//capital good date of birth	
		v[3]=VS(cur1, "capital_good_productive_capacity");								//capital good's prductive capacity
		v[5]=VS(cur1, "capital_good_depreciation_period");
		if((double)t>=v[5]&&v[4]>1)														//if the current time step is higher than the date of birth of the capital good plus the depreciation period, this capital good must be depreciated
			{
			v[0]=v[0]+v[3];																//sum up the capital good productive capacity to the total productive capacity to be depreciated
			DELETE(cur1);																//delete the capital good
			v[4]=v[4]-1;
			}
		else																			//if the current time step is not higher than the date of birth of the capital good plus the depreciation period, this capital good must remain
			{
			if((double)t>=v[5]&&v[4]<=1)
				{
				WRITES(cur1, "capital_good_productive_capacity", 0);
				WRITES(cur1, "Capital_Good_Productivity", 0);
				}
			else
				v[0]=v[0];																//do not sum the capital good productive capacity to the total to be depreiated
			}																			//do not sum the capital good productive capacity to the total to be depreiated
	}                                                                          
RESULT(v[0])


EQUATION("Firm_Effective_Productive_Capacity_Variation")
/*
Depends on the demand for productive capacity in the last investment period. This will be added to the firm's productive capacity. Can be restricted by the demand not met of the domestic sector and international reserves
*/
	v[0]=V("Firm_Investment_Period");													//if it is investment period for the firm
	if(v[0]==1)
		{
		v[1]=V("sector_investment_period");												//investment period
		cur=SEARCH_CNDS(root, "id_capital_goods_sector", 1);							//search the capital goods sector
		v[6]=v[7]=0;
		for (i=1; i<=v[1]; i++)															//for the current production period until the last investment period -1
			{
			v[4]=VLS(cur, "Sector_Demand_Met", i);										//computates the demand met by the sector in the current lag
			v[5]=VLS(cur, "Sector_Demand_Met_By_Imports", i);							//computates the demand met by imports in the current lag
			v[6]=v[6]+v[4];
			v[7]=v[7]+v[5];
			}
		v[8]=v[6]/v[1];																	//average demand met
		v[9]=v[7]/v[1];																	//average demand met by imports
		
		v[10]=VL("Firm_Demand_Capital_Goods_Expansion", v[1]);
		v[11]=VL("Firm_Demand_Capital_Goods_Replacement", v[1]);
			
		v[12]=v[10]*(v[6]+(1-v[6])*v[7]);
		v[13]=v[11]*(v[6]+(1-v[6])*v[7]);
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
	v[0]=V("sector_investment_period");											//investment period
	v[1]=VL("Firm_Frontier_Productivity",v[0]);									//available technology when the capital good was ordered
	v[2]=VL("Firm_Demand_Capital_Goods_Expansion",v[0]);	
	v[3]=VL("Firm_Demand_Capital_Goods_Replacement",v[0]);	
	v[4]=v[2]+v[3];	
	v[5]=V("sector_capital_output_ratio");										//amount of Capital Goods bought
	v[6]=COUNT("CAPITALS");
	v[7]=V("Firm_Investment_Period");											//if it is investment period for the firm
	v[8]=V("sector_depreciation_period");
	v[9]=VL("Price_Capital_Goods",v[0]);
	if(v[7]==1 && v[2]>0)
		{
		for(i=0; i<=v[2]; i++)													//for the amount of new capital goods bought
			{
			cur=ADDOBJ("CAPITALS");												//create new capital objects
			WRITES(cur, "capital_good_productivity_initial", v[1]);				//writes the new capital productivity as the frontier productivity when it was ordered
			WRITES(cur, "capital_good_productive_capacity", (1/v[5]));			//writes the productive capacity as the inverse of current capital output ratio of the sector
			WRITES(cur, "capital_good_date_birth", t);							//writes the new capital date of birth as the current time period
			WRITES(cur, "capital_good_to_replace", 0);							//writes the parameter that identifies the capital goods to be replaced as zero
			WRITES(cur, "capital_good_depreciation_period", (t+v[8]));
			WRITELS(cur, "Capital_Good_Acumulated_Production", 0, 1);			//writes the past acumulated production of the current new capital as zero
			v[6]=v[6]+1;
			}																																		//end the CYCLE for modernization
		}
		
	if(v[7]==1&&v[3]>0)
		{
		SORT("CAPITALS","Capital_Good_Productivity","UP");
		CYCLE(cur, "CAPITALS")																																					// CYCLE trought capital goods
   			{
     		v[10]=VS(cur, "capital_good_to_replace");
     		if(v[10]==1)
     			{
     			WRITES(cur, "capital_good_productivity_initial", v[1]);				//writes the new capital productivity as the frontier productivity when it was ordered
				WRITES(cur, "capital_good_date_birth", t);							//writes the new capital date of birth as the current time period
				WRITES(cur, "capital_good_to_replace", 0);							//writes the parameter that identifies the capital goods to be replaced as zero
				WRITES(cur, "capital_good_depreciation_period", (t+v[8]));
				WRITELS(cur, "Capital_Good_Acumulated_Production", 0, 1);			//writes the past acumulated production of the current new capital as zero
     			}																																							// do not sum replacement cost
  			}
  		}										
	if (v[6]!=0)																																					//if it is not zero
		v[15] = SUM("capital_good_productive_capacity");							//sum uo their productive capacity
	else																																							//if its zero
		v[15]=0;																																	//firm's productive capacity will be zero
RESULT(v[15])


EQUATION("Firm_Capital")
/*
Nominal value of firm's total capital
*/
	v[0]=V("Firm_Productive_Capacity");                       							//firm's productive capacity in the last period   
	v[1]=V("Price_Capital_Goods");                       								//price of capital goods
	v[2]=V("sector_capital_output_ratio");                      						//capital output ratio 
	v[3]=v[0]*v[1]*v[2];                                 								//nominal value of firm's total capital
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







