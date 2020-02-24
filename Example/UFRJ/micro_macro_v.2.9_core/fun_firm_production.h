
EQUATION("Firm_Expected_Sales")
/*
Firm's expected sales are calculated from an average of effective sales from the two previous periods, applying a expected growth rate. This expected growth rate is obtained from comparing the average of the two previous periods with the average of the two before that, adjusted by an expectation parameter.
*/
	v[1]=VL("Firm_Effective_Orders", 1);                    //firm's effective orders lagged 1
	v[2]=VL("Firm_Effective_Orders", 2);                    //firm's effective orders lagged 2
	v[3]=V("expectations");                                 //expectations parameter

	if(v[2]!=0)                                           	//if firm's effective orders lagged 2 is not zero
		{
		v[4]=v[1]*(1+v[3]*((v[1]-v[2])/v[2]));              //expected sales will be the effective orders in the last period multiplied by the growth rate between the two periods adjusted by the expectations parameter
		v[5]=max(0,v[4]);                                   //expected sales can never be negative
		}
	else                                                  	//if firm's effective orders lagged 2 is zero 
		v[5]=v[1];                                          //expected sales will be equal to effective orders of the last period
RESULT(v[5])


EQUATION("Firm_Planned_Production")
/*
It's calculated from expected sales for the period, considering the proportion of sales that is desired to be kept as inventories and discounting the acumulated stock of inventories in the last period. 
For the capital goods sector, program production is defined by effective orders.
Programed Production is subjected to a existing capactity restriction, but it is possible to increase production by incrising extra hours of labor, in any sector.
*/
	v[0]=V("id_capital_goods_sector");                    	//identifies the capital goods sector      
	v[1]=V("Firm_Expected_Sales");                          //calls the firm's expected sales
	v[2]=VL("Firm_Productive_Capacity", 1);                 //calls the firm's productive capacity of the last period
	v[5]=V("desired_inventories_proportion");             	//calls the firm's desired inventories ratio as a proportion of sales
	v[6]=VL("Firm_Stock_Inventories",1);                    //calls the firm's stock of inventories in the last period

	if(v[0]==0)                                           	//if it is not capital goods sector
		v[7]=v[1]*(1+v[5])-v[6];                            //planned production will be expected sales plus the desired proportion of investories minus the existing stock of inventories
	else                                                  	//if it is a capital goods sector
		v[7]=V("Firm_Effective_Orders_Capital_Goods");      //planned production will be the firm's effective orders (received in the beginning of the period)

	v[8]=max(0, min(v[2],v[7]));                          	//planned production can never be more then the maximum productive capacity and can never be negative
RESULT(v[8])



EQUATION("Firm_Effective_Production")
/*
The actual production of each sector will be determined by the constraint imposed by the availability of inputs to the realization of the programmed production (or production of inputs desired, in the case of intermediate sectors). Such constraint is defined by the lower ratio between available inputs and the inputs required for production.
*/
	v[0]=V("Firm_Planned_Production");                                                              //firm's planned production
	v[1]=V("id_intermediate_goods_sector");                                                      	//identifies intermediate goods sectors
	v[2]=V("Firm_Intermediate_Production");                                                      	//intermediate goods production for the firm of intermediate goods sectors
	v[3]=V("Firm_Available_Inputs_Ratio");
	if (v[1]==0)                                                                                 	//if it is not intermediate goods sector
		v[4]=v[3]*v[0];                                                                            	//effective planned production, constrained by the ratio of available inputs
	else                                                                                         	//if it is intermediate goods sector
		v[4]=v[3]*v[2];                                                                            	//effective planned production, constrained by the ratio of available inputs
	
	SORT("CAPITALS", "Capital_Good_Productivity", "DOWN");                                        	//rule for the use of capital goods, sorts firm's capital goods by productivity in a decreasing order
	v[5]=0;                                                                                      	//initializes the CYCLE
	CYCLE(cur, "CAPITALS")                                                                        	//CYCLE trought the capital goods of the firm
	{
		v[6]=VS(cur, "capital_good_productive_capacity");                                          	//capital productivity capacity
		v[8]=max(0,(min(v[6], v[4])));                                                             	//maximum capacity of each capital goods, constrained by effective planned production, and it can not be negative
		WRITES(cur, "Capital_Good_Production", v[8]);                                              	//the capacity of each capital goods is in fact its production
		v[4]=v[4]-v[8];                                                                            	//it subracts the production of the first capital good from the effective planned production before going to the next capital good
		v[5]=v[5]+v[8];                                                                            	//sums up the production of each capital good to determine firm's effective production
	}
RESULT(v[5])
