
/******MACRO FUNCTIONS******/


EQUATION("Intermediate_Demand_Function")
/*
Calculates the domestic demand for inputs.
Must be called by the sectors.
*/
	v[0]=VS(c, "id_intermediate_goods_sector");                    			//identifies the intermediate goods sector                   
	if (v[0]==1)                                                   			//if it is a intermediate good sector
		{
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
		}
	else                                                          			//if it is not intermediate goods sector
		v[2]=0;                                                        		//the demand for imputs is zero
RESULT(v[2])


EQUATION("Domestic_Consumption_Demand_Function")
/*
Calculates the domestic demand for consumption goods.
Must be called by the sector.
*/
	v[0]=0;                                                 		//initializes the CYCLE
	CYCLE(cur, "CLASSES")                                   		//CYCLE trought all income classes
	{
		v[1]=VS(cur, "Class_Real_Domestic_Consumption");      	    //class consumption
		v[0]=v[0]+v[1];                                       		//sums up all classes consumption
	}
RESULT(v[0])


EQUATION("Capital_Goods_Demand_Function")
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
RESULT(v[1])


EQUATION("Income_Taxes_Function")
/*
Calculates income taxes of the classes and must be called by the government variable
Must be called by Government.
*/
	v[0]=V("Total_Wages");                                 		//total wages          
	v[1]=V("Total_Distributed_Profits");                         		//distributed profits for the classes
	v[2]=0;                                               		//initializes the CYCLE
	CYCLE(cur, "CLASSES")                                 		//CYCLE trought all classes
	{
		v[3]=VS(cur,"class_direct_tax");                     	//income tax percentage of each class
		v[4]=VS(cur,"class_profit_share");                   	//class profit share
		v[5]=VS(cur,"class_wage_share");                     	//class wage share
		v[6]=(v[0]*v[5]+v[1]*v[4])*v[3];                     	//income tax of each class is the tax percentage multiplyed by class total income
		v[2]=v[2]+v[6];                                     	//sums up income taxes of all classes
	}
RESULT(v[2])


EQUATION("Indirect_Taxes_Function")
/*
Calculates indirect taxes of all firms. Must be called by the government variable
Must be called by Government.
*/
	v[0]=0;                                						//initializes the CYCLE
	CYCLE(cur, "SECTORS")                  						//CYCLE trought all sectors
	{              
		v[1]=0;                              					//initializes the second CYCLE
		CYCLES(cur, cur1, "FIRMS")           					//CYCLE trought all firms in the sector
		{
			v[2]=VS(cur1, "Firm_Indirect_Tax");     			//firm's indirect tax
			v[1]=v[1]+v[2];                    					//sums up all firm's indirect tax of the sector
		}
	v[0]=v[0]+v[1];                        						//sums up all sectors indirect tax
	}
RESULT(v[0])



