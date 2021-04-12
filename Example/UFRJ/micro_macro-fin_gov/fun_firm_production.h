
EQUATION("Firm_Expected_Demand")
/*
Firm's expected sales are calculated from an average of effective sales from the two previous periods, applying a expected growth rate. This expected growth rate is obtained from comparing the average of the two previous periods with the average of the two before that, adjusted by an expectation parameter.
*/
	v[1]=VL("Firm_Effective_Orders", 1);                    //firm's effective orders lagged 1
	v[2]=VL("Firm_Effective_Orders", 2);                    //firm's effective orders lagged 2
	v[3]=V("sector_expectations");                          //firm expectations
	if(v[2]!=0)                                           	//if firm's effective orders lagged 2 is not zero
		v[4]=v[1]*(1+v[3]*((v[1]-v[2])/v[2]));              //expected sales will be the effective orders in the last period multiplied by the growth rate between the two periods adjusted by the expectations parameter
	else                                                  	//if firm's effective orders lagged 2 is zero 
		v[4]=v[1];                                          //expected sales will be equal to effective orders of the last period
RESULT(max(0,v[4]))


EQUATION("Firm_Planned_Production")
/*
It's calculated from expected sales for the period, considering the proportion of sales that is desired to be kept as inventories and discounting the acumulated stock of inventories in the last period. 
For the capital goods sector, program production is defined by effective orders.
Programed Production is subjected to a existing capactity restriction, but it is possible to increase production by incrising extra hours of labor, in any sector.
*/
	v[0]=V("id_capital_goods_sector");                    	//identifies the capital goods sector      
	v[1]=V("Firm_Expected_Demand");                         //calls the firm's expected sales
	v[2]=VL("Firm_Productive_Capacity", 1);                 //calls the firm's productive capacity of the last period
	v[5]=V("sector_desired_inventories_proportion");        //calls the firm's desired inventories ratio as a proportion of sales
	v[6]=VL("Firm_Stock_Inventories",1);                    //calls the firm's stock of inventories in the last period
	if(v[0]==0)                                           	//if it is not capital goods sector
		v[8]=v[1]*(1+v[5])-v[6];                            //planned production will be expected sales plus the desired proportion of investories minus the existing stock of inventories
	else                                                  	//if it is a capital goods sector
		{
		v[10]=V("sector_investment_frequency");
		v[7]=0;
		for(i=0;i<=(v[10]-1);i++)
			{
			v[11]=VL("Firm_Effective_Orders_Capital_Goods",i);
			v[12]=v[11]/v[10];
			v[7]=v[7]+v[12];
			}
		v[8]=v[7]*(1+v[5]);
		}
	v[9]=max(0, v[8]);                          			//planned production can never be more then the maximum productive capacity and can never be negative
RESULT(v[9])


EQUATION("Firm_Effective_Production")
/*
The actual production of each sector will be determined by the constraint imposed by the availability of inputs to the realization of the programmed production (or production of inputs desired, in the case of intermediate sectors). Such constraint is defined by the lower ratio between available inputs and the inputs required for production.
*/
	v[0]=V("Firm_Planned_Production");                                                              //firm's planned production
	v[1]=V("Firm_Available_Inputs_Ratio");
	v[2]=v[1]*v[0];                                                                            		//effective planned production, constrained by the ratio of available inputs
	SORT("CAPITALS", "Capital_Good_Productivity", "DOWN");                                        	//rule for the use of capital goods, sorts firm's capital goods by productivity in a decreasing order
	v[3]=0;                                                                                      	//initializes the CYCLE
	CYCLE(cur, "CAPITALS")                                                                        	//CYCLE trought the capital goods of the firm
	{
		v[4]=VS(cur, "capital_good_productive_capacity");                                          	//capital productivity capacity
		v[5]=max(0,(min(v[4], v[2])));                                                             	//maximum capacity of each capital goods, constrained by effective planned production, and it can not be negative
		WRITES(cur, "Capital_Good_Production", v[5]);                                              	//the capacity of each capital goods is in fact its production
		v[2]=v[2]-v[5];                                                                            	//it subracts the production of the first capital good from the effective planned production before going to the next capital good
		v[3]=v[3]+v[5];                                                                            	//sums up the production of each capital good to determine firm's effective production
	}
RESULT(v[3])


EQUATION_DUMMY("Capital_Good_Production", "Firm_Effective_Production")


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
	v[6]= v[3]!=0? v[2]/v[3] : v[1];                                            //firm's average productivity will be the average of each capital good productivity weighted by its respective production
RESULT(v[6])


EQUATION("Firm_Capacity_Utilization")
/*
Firm effective production over firm productive capacity
*/
	v[0]=V("Firm_Effective_Production");
	v[1]=VL("Firm_Productive_Capacity",1);
	v[2]= v[1]!=0? v[0]/v[1] : 0;
RESULT(v[2])