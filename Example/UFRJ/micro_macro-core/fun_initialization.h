EQUATION("Initialization")
/*
***************
READ CAREFULLY
***************
This variable initializes the model.
It calls all control parameters defined by the user in the control object.
Then, it perform initial calculations to define other parameters and lagged values based only on the control parameters.
It also writes the dependent parameters and lagged values. User do not need to do it by hand.
Finally, it also creates objects based on control parameters. 1 object of each kind must be defined by the user first.
It is set in a specific simple configuration: 3 sectors and 3 income classes. Next step is to generalize this variable for N sectors.
Additionally, there is a strong hypothesis here: all sector are equals, except for how their demand is calculated.
*/

v[0]=V("investment_period");               				
v[1]=V("markup_period");
v[2]=V("class_period");									
v[3]=V("annual_period");
v[4]=V("government_period");
v[5]=V("depreciation_period");

v[6]=V("number_object_firms");

v[7]=V("initial_productivity");
v[8]=V("initial_markup");
v[9]=V("initial_wage");
v[10]=V("initial_quality");

v[11]=V("input_tech_coefficient");
v[12]=V("capital_output_ratio");

v[13]=V("rnd_revenue_proportion");
v[14]=V("profits_distribution_rate");
v[15]=V("indirect_tax_rate");

v[16]=V("exports_elasticity_income");
v[17]=V("desired_inventories_proportion");	
v[18]=V("desired_degree_capacity_utilization");


v[20]=v[21]=v[22]=v[23]=v[24]=v[25]=0;
CYCLE(cur, "CLASSES")
{
	v[26]=VS(cur, "class_profit_share");
	v[27]=VS(cur, "class_wage_share");
	v[28]=VS(cur, "class_propensity_to_consume");
	v[29]=VS(cur, "class_propensity_to_import");
	v[30]=VS(cur, "class_direct_tax");
	
	//v[20]=v[20]+v[27]*v[28]*(1-v[30]);															//effective aggregate propensity to consume on wages
	//v[21]=v[21]+v[26]*v[28]*(1-v[30]);															//effective aggregate propensity to consume on profits
	
	v[22]=v[22]+v[26]*v[30];																	//effective direct tax over profits
	v[23]=v[23]+v[27]*v[30];																	//effective direct tax over wages
	v[24]=v[24]+v[27]*v[29]*(1-v[30]);															//effective aggregate propensity to import on wages
	v[25]=v[25]+v[26]*v[29]*(1-v[30]);															//effective aggregate propensity to import on profits
}

v[20]=V("propensity_to_consume_wages");
v[21]=V("propensity_to_consume_profits");

//Begin Auxiliary Calculations
v[19]=(v[8]*v[9]/v[7])/(1-v[8]*v[11]);															//nominal price
v[31]=(v[9]/v[7]) + v[13]*(1-v[15])*v[19];														//sector effective wage margin over production including RND
v[32]=v[19]*(1-v[15])*(1-v[13])-((v[9]/v[7])+v[11]*v[19]);										//sector effective profit margin over production
v[33]=(v[15]*v[19] + v[22]*v[14]*v[32] + v[23]*v[31])/(1-v[23]);								//sector effective tax-public wage over production
v[34]=(v[31]+v[33])*v[24]+(v[32]*v[14])*v[25];													//sector effective imports propensity
v[35]=((v[20]+v[24]/3)*(v[31]+v[33]) + (v[21]+v[25]/3)*(v[32]*v[14]))/v[19];					//sector effective consumption propensity including exports
v[36]=v[11]+((v[24]/3)*(v[31]+v[33]) + (v[25]/3)*(v[32]*v[14]))/v[19];							//sector input tech coefficient including exports
v[37]=((v[24]/3)*(v[31]+v[33]) + (v[25]/3)*(v[32]*v[14]))/v[19];								//sector effective capital exports propensity

v[38]=v[36]/(1-v[36]);																			//sector effecive direct and indirect input tech coefficient
v[39]=v[35]+v[35]*v[38];																		//sector effective direct and indirect propensity to consume
v[40]=v[37]+v[37]*v[38];																		//sector effective direct and indirect capital exports propensity

//Begin Aggregate Calculations
v[41]=v[6]/(1-v[39]-(v[39]*v[40])/(1-v[40]));					//consumption sector initial demand
v[42]=(v[41]*v[40])/(1-v[40]);													//capital sector initial demand
v[43]=v[41]*v[38]+v[42]*v[38];																	//intermediate sector initial demand

v[44]=v[33]*(v[41]+v[42]+v[43]);							 									//total taxes
v[45]=v[15]*v[19]*(v[41]+v[42]+v[43]);															//indirect taxes
v[46]=v[31]*(v[41]+v[42]+v[43]) + v[44];			 											//total wages including public
v[47]=v[32]*(v[41]+v[42]+v[43]);							 									//total profits
v[48]=v[14]*v[47];																				//distributed profits
v[49]=v[34]*(v[41]+v[42]+v[43]);																//total imports
v[50]=v[46]+v[47]+v[45];																		//GDP

//Begin Writting Macro Variables
CYCLE(cur, "MACRO")
{
		WRITES(cur, "initial_avg_price", v[19]);
		WRITELLS(cur,"Likelihood_Crisis", 0, 0, 1);                  							//zero by definition
		WRITELLS(cur,"Annual_Growth", 0, 0, 1);													//zero by definition, no growth initally
		WRITELLS(cur,"Annual_Real_Growth", 0, 0, 1);                 							//zero by definition, no growth initally
	for (v[101]=1 ; v[101]<=(v[3]+1) ; v[101]=v[101]+1)                  						//for (annual period +1) lags
		{
		WRITELLS(cur,"Price_Index", v[19], 0, v[101]);									 		//writes Price_Index, all initial price index is 1
		WRITELLS(cur,"Consumer_Price_Index", v[19], 0, v[101]);          						//writes Consumper_Price_Index, all initial price index is 1
		}
	for (v[102]=1 ; v[102]<=(2*v[3]) ; v[102]=v[102]+1)                  						//for (2*annual_period) lags
		{
		WRITELLS(cur,"GDP", v[50], 0, v[102]);                     	 							//GDP
		WRITELLS(cur,"Real_GDP", (v[50]/v[19]), 0, v[102]);                  					//Real GDP will be equal to nominal GDP because price index always begins as 1
		}
}

//Begin Writing Classes Variables
v[75]=COUNT("CLASSES");
CYCLE(cur, "CLASSES")
{
	v[51]=VS(cur, "class_profit_share");                						   		  		//class parameter
	v[52]=VS(cur, "class_wage_share"); 															//class parameter
	v[53]=VS(cur, "class_direct_tax");															//class parameter
	v[54]=(v[51]*v[48]+v[52]*(v[46]))*(1-v[53]);             									//class nominal net income																		//total imports
	v[55]=v[6]/v[75];																			//class initial autonomous consumption
		for (v[104]=1 ; v[104]<=v[2] ; v[104]=v[104]+1)                          				//for (class_period) lags
			{
			WRITELLS(cur, "Class_Nominal_Income", v[54], 0, v[104]);            				//writes Class_Nominal_Income
			WRITELLS(cur, "Class_Real_Income", (v[54]/v[19]), 0, v[104]);						//writes Class_Real_Income
			}
			WRITELLS(cur, "Class_Autonomous_Consumption", v[55], 0, 1);         				//write class' autonomous consumption
			WRITELLS(cur, "Class_Debt", 0, 0, 1);                              					//0, no debt initially
			WRITELLS(cur, "Class_Debt_Payment", 0, 0, 1);										//0, no debt initially
			WRITELLS(cur, "Class_Financial_Assets", 0, 0, 1);									//0, no financial assets initially
			WRITELLS(cur, "Class_Financial_Assets", 0, 0, 2);									//0, no financial assets initially

		if (v[51]==1)
			WRITES(cur, "class_propensity_to_consume", v[21]);
		if (v[52]==1)
			WRITES(cur, "class_propensity_to_consume", v[20]);
}

//Begin Writing External Variables
cur2 = SEARCH("EXTERNAL_SECTOR");																//search the external sector
WRITELLS(cur2, "External_Income", v[50], 0, 1);													//writes initial external income equal to domestic GDP

//Begin Writing Government Variables
cur = SEARCH("GOVERNMENT");																		//initial total taxes is calculated in the demand calibration based only on parameters
WRITELLS(cur,"Total_Taxes", v[44], 0, 1);														//write initial total taxes
WRITELLS(cur,"Government_Max_Expenses", v[44], 0, 1);        									//initial max government expenses equals total taxes calculated in the calibration
WRITELLS(cur,"Government_Wages", v[44], 0, 1);		            								//initial government expenses is only wages, which thereafter will grow depending on inflation and average productivity		
for (v[105]=1 ; v[105]<=v[4] ; v[105]=v[105]+1)		              								//for (government_period) lags	
	WRITELLS(cur,"Government_Debt", 0, 0, v[105]);                  							//no debt initially	
v[56]=VS(cur, "interest_rate");																	//base interest rate parameter
WRITELLS(cur,"Basic_Interest_Rate", v[56], 0, 1);

//Begin Writing Sector Variables
CYCLE(cur, "SECTORS")
{
	v[57]=VS(cur, "id_consumption_goods_sector");
	v[58]=VS(cur, "id_capital_goods_sector");
	v[59]=VS(cur, "id_intermediate_goods_sector");
	if(v[57]==1)																				//if it is a consumption good sector
		v[60]=v[41];																			//intial demand, production and sales
	if(v[58]==1)																				//if it is a capital good sector
		v[60]=v[42];																			//intial demand, production and sales
	if(v[59]==1)																				//if it is a intermediate goods sector
		v[60]=v[43];																			//initial demand production and sales
	
	v[61]=v[60]/v[6]; 																			//initial production of each firm
	v[62]=((v[61]*(1+v[17]))/v[18])*v[12];														//number of capital goods of each firm
	v[63]=((v[19]/v[8])-v[11]*v[19])/v[7];                        								//sector initial wage
	
	for (v[106]=1 ; v[106]<=v[0] ; v[106]=v[106]+1)												//for (investment_period) lags 
		{
		WRITELLS(cur, "Sector_Demand_Met", 1, 0, v[106]); 										                            				//it is assumed that all demand is met initially. Equals 1 by definition
		WRITELLS(cur, "Sector_Demand_Met_By_Imports", 1, 0, v[106]);                      		//it is assumed thatt all imports are met initially. Equals 1 by definition
		WRITELLS(cur, "Sector_Effective_Orders", v[60], 0, v[107]);               				//Effective_Orders_Sectors equals demand_initial
		}		
	for (v[108]=1 ; v[108]<=(v[2]+1) ; v[108]=v[108]+1)                        		 			//for (class_period+1) lags
		WRITELLS(cur, "Sector_Avg_Quality", v[10], 0, v[108]);               					//Effective_Orders_Sectors equals demand_initial
		WRITELLS(cur, "Sector_Productive_Capacity_Available", 0, 0, 1);                  		//it is assumed that there is no entry or exit initially. Equals 0 by definition
		WRITELLS(cur, "Sector_Avg_Competitiveness", 1, 0, 1);                     				//if all firms are the same, equals 1 by definition
		WRITELLS(cur, "Sector_Avg_Price", v[19], 0, 1);                                   		//Avg_Price equals avg_price initial
		WRITELLS(cur, "Sector_Avg_Price", v[19], 0, 2);											//Avg_Price equals avg_price initial
		WRITELLS(cur, "Sector_External_Price", v[19], 0, 1);                               		//Foreign_Price equals foreign_price initial
		WRITELLS(cur, "Sector_Avg_Productivity", v[7], 0,  1);               	 				//If all firms are the same, Avg Productivity will be the initial productivivity for all firms
		WRITELLS(cur, "Sector_Max_Productivity", v[7], 0,  1);                      			//If all capital goods have the same productivity, Max_Productivity equals productivity_initial 
		WRITELLS(cur, "Sector_Max_Quality", v[10], 0,  1);
		WRITELLS(cur, "Sector_Inventories", (v[60]*v[17]), 0, 1);                  				//Firms operate with desired level of inventories, thus, Current stock of inventories is the desired level times effective production
		WRITELLS(cur, "Sector_Productive_Capacity", ((v[60]*(1+v[17]))/v[18]), 0, 1);			//All firms start operating at desired degree of utilization, thus, productive capacity is endogenous calculated based on effective production and desired degree
		WRITELLS(cur, "Sector_Exports", (v[49]/3), 0, 1);										//Total exports are divided equally among sectors.
		v[75]=((v[49]/3)/(pow(v[50], v[16])));													//calculate sector exports coefficient
		WRITES(cur, "sector_exports_coefficient", v[75]);										//write the exports coefficient, assuming external price and foreign price starts as 1, so the exchange rate
	
	cur1=SEARCHS(cur, "FIRMS");																	//search the first and only instance of firms below each sector
		//Begin Writting Firm Parameters
		WRITES(cur1, "capital_goods_effective_orders_firm_temporary", 0);						//all temporary parameters starts at zero
		WRITES(cur1, "capital_goods_production_temporary", 0);									//all temporary parameters starts at zero
		WRITES(cur1, "consumption_effective_orders_firm_temporary", 0);	   	  					//all temporary parameters starts at zero
		WRITES(cur1, "intermediate_effective_orders_firm_temporary", 0);						//all temporary parameters starts at zero
		WRITES(cur1, "intermediate_production_firm_temporary", 0);								//all temporary parameters starts at zero
		WRITES(cur1, "firm_date_birth", 0);                                   					//zero by definition
	
		//Begin Writting Independent Firm Variables
	for (v[109]=1 ; v[109]<=v[0] ; v[109]=v[109]+1)                                				//for (investment_period) lags
	  	{
	  	WRITELLS(cur1, "Firm_Demand_Productive_Capacity_Replacement", 0, 0, v[109]);			//no replacement initially
	  	WRITELLS(cur1, "Firm_Debt_Rate", 0, 0, v[109]);											//no debt initially
	  	WRITELLS(cur1, "Firm_Demand_Capital_Goods", 0, 0, v[109]);
	  	WRITELLS(cur1, "Firm_Frontier_Productivity", v[7], 0, v[109]);                 			//frontier productivity will be the initial frontier productivity
	  	WRITELLS(cur1, "Firm_Max_Productivity", v[7], 0, v[109]);        						//max capital goods productivity will be the initial frontier productivity that will be the same for all capital goods
	  	WRITELLS(cur1, "Firm_Avg_Productivity", v[7], 0, v[109]);								//firm's avg productivity will be the initial frontier productivity since all capital goods have the same productivity
		}
	for(v[110]=1; v[110]<=(v[0]+1); v[110]=v[110]+1)										 	//for (investment period+1) lags (7)
		{
		WRITELLS(cur1, "Firm_Productive_Capacity_Depreciation", 0, 0, v[110]);  				//write 0 
		WRITELLS(cur1, "Firm_Demand_Productive_Capacity_Expansion", 0, 0, v[110]);     			//write 0 
		}
	for (v[111]=1 ; v[111]<=(2*v[0]) ; v[111]=v[111]+1)											//for (2*investment period+1) lags
	  	WRITELLS(cur1, "Firm_Effective_Orders", v[61], 0, v[111]);                     			//firm's effective orders will be sector's effective orders (given by demand_initial) divided by the number of firms
	for (v[112]=1 ; v[112]<=(v[1]-1) ; v[112]=v[112]+1)											//for (markup_period-1) lags
		{
		WRITELLS(cur1, "Firm_Market_Share", (1/v[6]), 0, v[112]);             					//firm's market share will be the inverse of the number of firms in the sector (initial market share)
	  	WRITELLS(cur1, "Firm_Potential_Markup", v[8], 0, v[112]);                      			//potential markup will be the initial markup
		}
		WRITELLS(cur1, "Firm_Effective_Market_Share", (1/v[6]), 0, 1);                    		//firm's effective market share will be the initial market share       
	  	WRITELLS(cur1, "Firm_Desired_Market_Share", (1/v[6]), 0, 1);                  			//firm's desired market share will be twice the initial market share  
	  	WRITELLS(cur1, "Firm_Avg_Market_Share", (1/v[6]), 0, 1);                     			//firm's avg market share will be the initial market share
	  	WRITELLS(cur1, "Firm_Price", v[19], 0, 1);                                      			//firm's price will be the initial price of the sector, the same for all firms
	  	WRITELLS(cur1, "Firm_Avg_Potential_Markup", v[8], 0, 1);								//avg potential markup will be the initial markup
	  	WRITELLS(cur1, "Firm_Desired_Markup", v[8], 0, 1); 										//desired markup will be the initial markup
	  	WRITELLS(cur1, "Firm_Sales", v[61], 0, 1);												//firm's sales will be equal to effective orders, no delivery delay
	  	WRITELLS(cur1, "Firm_Revenue", (v[61]*v[19]), 0, 1);                            			//firm's revenue will be the firm's sales times firm price
	  	WRITELLS(cur1, "Firm_Stock_Inventories", (v[61]*v[17]), 0, 1);                        	//firm's inventories will be the sales times the desired inventories proportion (parameter)
	  	WRITELLS(cur1, "Firm_Stock_Inputs", (v[11]*v[61]), 0, 1);                      			//firm's stock of imputs will be the sales times the input tech relationship
	  	WRITELLS(cur1, "Firm_Productive_Capacity", ((v[61]*(1+v[17]))/v[18]), 0, 1);			//firm productive capacity will be the sales divided by the desired degree of capacity utlization (parameter)
	  	WRITELLS(cur1, "Firm_Capital", (v[62]*v[19]), 0, 1);										//firm nominal capital equals number of capital if capital goods price equals 1
	  	WRITELLS(cur1, "Firm_Wage", v[9], 0, 1); 												//firm's nominal wage equals sector nominal wage initial
		WRITELLS(cur1, "Firm_Variable_Cost", ((v[9]/v[7])+v[11]*v[19]), 0, 1);							//firm variable cost equals unitary wage plus unitary cost of inputs. The last equals the tech coefficient if input price equals 1
		WRITELLS(cur1, "Firm_Competitiveness", 1, 0, 1);                           				//if all firms are the same
	  	WRITELLS(cur1, "Firm_Delivery_Delay", 1, 0, 1);                           		  		//it is assumed that all demand is met initially, so equals 1 by definition
		WRITELLS(cur1, "Firm_Stock_Financial_Assets", 0, 0, 1);									//no financial assets initially
	  	WRITELLS(cur1, "Firm_Stock_Debt", 0, 0, 1);                                    			//no debt initially
	  	WRITELLS(cur1, "Firm_Avg_Debt_Rate", 0, 0, 1);                       					//no debt initially
		
	  		//Begin writting Capital Goods Variables and parameters
	  		cur2=SEARCHS(cur1, "CAPITALS");														//search the first and only instance of capital below firms
			WRITELLS(cur2, "Capital_Good_Acumulated_Production", 0, 0, 1);      				//zero by definition
			WRITES(cur2, "capital_good_productive_capacity", (1/v[12]));     					//inverse of capital output ratio  
			WRITES(cur2, "capital_good_productivity_initial", v[7]);       		  				//defined in the control parameters
			WRITES(cur2, "capital_good_to_replace", 0);
			WRITES(cur2, "id_capital_good_number", 1);                       					//1 for the first and only initial firm (the others will be created below)
	 			
	 	//Begin Creating Firms and writting some parameters
	 	for(v[113]=1; v[113]<=(v[6]-1); v[113]=v[113]+1)										//for the number of firms of each sector (defined by the parameter)
	 	cur4=ADDOBJ_EXLS(cur,"FIRMS", cur1, 0);                             					//create new firm using the first and only firm as example
	 			
	 	CYCLES(cur, cur1, "FIRMS")                                                 				//CYCLE trough all firms
			{
			v[70]=SEARCH_INSTS(cur, cur1);														//search current firm position in the total economy
			WRITES(cur1, "id_firm_number", v[70]);                         						//write the firm number as the current position (only initial difference between firms)
			v[71]=fmod((double) (v[70]+v[0]), v[0]);                                 			//divide the firm's number plus investment period by the investment period and takes the rest (possible results if investment period =6 are 0, 5, 4, 3, 2, 1)
			
			//Begin creating capital goods and writting "capital_good_date_birth"		
			for(v[114]=1; v[114]<=v[62]-1; v[114]=v[114]+1)                        				//for the number of capital goods of each firm
			{
			cur2=SEARCHS(cur1, "CAPITALS");                                         			//search the first and only capital good of each firm
			cur3=ADDOBJ_EXLS(cur1,"CAPITALS", cur2, 0);			                       			//create new capital goods using the first one as example
			WRITES(cur3, "id_capital_good_number", (v[114]+1));									//write the capital good number
			}
			CYCLES(cur1, cur5, "CAPITALS")                                            			//CYCLE trough all capital goods
				{
				v[72]=VS(cur5, "id_capital_good_number");
				v[73]=(-v[5]+v[71]+1)+(v[72]-1)*v[0];                                  			//calculates the capital good date of birth based on the firm number and the number of the capital good
				WRITES(cur5, "capital_good_date_birth", v[73]);									//write the capital good date of birth
				WRITES(cur5, "capital_good_depreciation_period", (v[73]+v[5]));
				}
			}					
}			


PARAMETER
RESULT(0)
