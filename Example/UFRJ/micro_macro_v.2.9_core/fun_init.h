
EQUATION("Initial_Sector_Demand")
/*
The equation calibrates the inital demand for the model with 3 sectors with government and external sector. 
If one or both of the exogenous blocks are not included, all respective parameters will be zero, rewritten by the main switch parameter. Therefore, the calculation of initial demand remains as in this equation.
The demand of the consumption goods sector depends on exports and income classes consumption, which therefor depends on wages and profits of all firms and government wages
The demand of the capital goods sector depends on exports and initial depreciation of all firms.
The demand of the intermediate goods sector depends on exports and the technical relationship and effective production of all firms, including those on the intermediate goods sector.

Essential Parameters: demand calibration depends on the fOllowing parameters only (and some hypotesis):
1-class_profit_share
2-class_wage_share
3-class_propensity_to_consume
4-class_propensity_to_import
5-class_direct_tax
6-avg_price_initial
7-markup_initial
8-input_technical_relationship
9-avg_productivity_initial
10-imitation_revenue_proportion
11-innovation_revenue_proportion
12-indirect_tax
13-profit_distribution_rate
14-exports_coefficient
15-number_object_firms

Calibrarion Hypotesis:
1-No past growth, therefore expected demand is the current demand and there is no investment for productive capacity expansion
2-All firms begin equally, with the same parameters and distribution (implies that all average and maximum values are the same and equal to the firm value)
3-All prices equals 1 initially
4-All capital good's productivity and therefor firm's average productivity equals 1 initially, and sector productivity equals 1.
5-Sales equlas Production that equals demand. There is no Delivery Delay initially. 
6-Initial balanced government budget. Total Expenses equal total taxes. Government Wages equals Total Expenses.
7-No past R&D expenses.
*/

/*****BEGIN AGGREGATING CLASSES*****/

USE_ZERO_INSTANCE;

v[5]=v[6]=v[7]=v[8]=v[9]=v[10]=0;
CYCLE(cur, "CLASSES")
{
	v[0]=VS(cur, "class_profit_share");
	v[1]=VS(cur, "class_wage_share");
	v[2]=VS(cur, "class_propensity_to_consume");
	v[3]=VS(cur, "class_propensity_to_import");
	v[4]=VS(cur, "class_direct_tax");
	
	v[5]=v[5]+v[1]*v[2]*(1-v[4]);				//effective aggregate propensity to consume on wages
	v[6]=v[6]+v[0]*v[2]*(1-v[4]);				//effective aggregate propensity to consume on profits
	v[7]=v[7]+v[0]*v[4];						//effective direct tax over profits
	v[8]=v[8]+v[1]*v[4];						//effective direct tax over wages
	v[9]=v[9]+v[1]*v[3]*(1-v[4]);				//effective aggregate propensity to import on wages
	v[10]=v[10]+v[0]*v[3]*(1-v[4]);				//effective aggregate propensity to import on profits
}

v[18]=0;
CYCLE(cur, "SECTORS")
{
	v[14]=V("investment_period");
	v[15]=V("number_object_firms");
	v[18]=v[18]+v[15]/v[14];				//number of capitals that will depreciate in each period (demand for capital goods)
}


/*****BEGIN DEFINING SECTORAL PARAMETERS*****/

cur1 = SEARCH_CND("id_consumption_goods_sector", 1);
cur2 = SEARCH_CND("id_capital_goods_sector", 1);
cur3 = SEARCH_CND("id_intermediate_goods_sector", 1);

v[25]=V("initial_productivity");
v[26]=V("initial_productivity");
v[27]=V("initial_productivity");
v[28]=V("indirect_tax_rate");
v[29]=V("indirect_tax_rate");
v[30]=V("indirect_tax_rate");
v[31]=V("input_tech_coefficient");
v[32]=V("input_tech_coefficient");
v[33]=V("input_tech_coefficient");
v[34]=V("profits_distribution_rate");
v[35]=V("profits_distribution_rate");
v[36]=V("profits_distribution_rate");
v[37]=V("initial_markup");
v[38]=V("initial_markup");
v[39]=V("initial_markup");
v[40]=V("initial_avg_price");
v[41]=V("initial_avg_price");
v[42]=V("initial_avg_price");
v[43]=V("number_object_firms");
v[44]=V("number_object_firms");
v[45]=V("number_object_firms");
v[46]=V("rnd_revenue_proportion");
v[47]=V("rnd_revenue_proportion");
v[48]=V("rnd_revenue_proportion");

/*****BEGIN CALCULATING INITIAL DEMAND*****/	

	v[19]=V("number_object_firms")*3;
	v[49]=v[43]/v[19];																				//sector 1 share of firms
	v[50]=v[44]/v[19];																				//sector 2 share of firms
	v[51]=v[45]/v[19];																				//sector 3 share of firms

	v[52]=((v[40]/v[37])-v[31])/v[25] + v[46]*(1-v[28])*v[40];										//sector 1 effective wage margin over production
	v[53]=((v[41]/v[38])-v[32])/v[26] + v[47]*(1-v[29])*v[41];										//sector 2 effective wage margin over production
	v[54]=((v[42]/v[39])-v[33])/v[27] + v[48]*(1-v[30])*v[42];										//sector 3 effective wage margin over production

	v[55]=v[40]*(1-v[28])*(1-v[46])-((((v[40]/v[37])-v[31])/v[25])+v[31]*v[42]);					//sector 1 effective profit margin over production
	v[56]=v[41]*(1-v[29])*(1-v[47])-((((v[41]/v[38])-v[32])/v[26])+v[32]*v[42]);					//sector 2 effective profit margin over production
	v[57]=v[42]*(1-v[30])*(1-v[48])-((((v[42]/v[39])-v[33])/v[27])+v[33]*v[42]);					//sector 3 effective profit margin over production

	v[58]=(v[28]*v[40] + v[7]*v[34]*v[55] + v[8]*v[52])/(1-v[8]);									//sector 1 effective tax-public wage over production
	v[59]=(v[29]*v[41] + v[7]*v[35]*v[56] + v[8]*v[53])/(1-v[8]);									//sector 1 effective tax-public wage over production
	v[60]=(v[30]*v[42] + v[7]*v[36]*v[57] + v[8]*v[54])/(1-v[8]);									//sector 1 effective tax-public wage over production	
	
	v[61]=(v[52]+v[58])*v[9]+(v[55]*v[34])*v[10];													//sector 1 effective imports propensity
	v[62]=(v[53]+v[59])*v[9]+(v[56]*v[35])*v[10];													//sector 2 effective imports propensity
	v[63]=(v[54]+v[60])*v[9]+(v[57]*v[36])*v[10];													//sector 3 effective imports propensity

	v[67]=(v[5]+v[9]*v[49])*(v[52]+v[58]) + (v[6]+v[10]*v[49])*(v[55]*v[34]);						//sector 1 effective consumption propensity including exports
	v[68]=(v[5]+v[9]*v[49])*(v[53]+v[59]) + (v[6]+v[10]*v[49])*(v[56]*v[35]);						//sector 2 effective consumption propensity including exports
	v[69]=(v[5]+v[9]*v[49])*(v[54]+v[60]) + (v[6]+v[10]*v[49])*(v[57]*v[36]);						//sector 3 effective consumption propensity including exports

	v[70]=v[31]+(v[9]*v[51])*(v[52]+v[58]) + (v[10]*v[51])*(v[55]*v[34]);							//sector 1 input tech coefficient including exports
	v[71]=v[32]+(v[9]*v[51])*(v[53]+v[59]) + (v[10]*v[51])*(v[56]*v[35]);							//sector 2 input tech coefficient including exports
	v[72]=v[33]+(v[9]*v[51])*(v[54]+v[60]) + (v[10]*v[51])*(v[57]*v[36]);							//sector 3 input tech coefficient including exports

	v[73]=(v[9]*v[50])*(v[52]+v[58]) + (v[10]*v[50])*(v[55]*v[34]);									//sector 1 effective capital exports propensity
	v[74]=(v[9]*v[50])*(v[53]+v[59]) + (v[10]*v[50])*(v[56]*v[35]);									//sector 2 effective capital exports propensity
	v[75]=(v[9]*v[50])*(v[54]+v[60]) + (v[10]*v[50])*(v[57]*v[36]);									//sector 3 effective capital exports propensity

	v[76]=v[70]/(1-v[72]);																			//sector 1 effecive direct and indirect input tech coefficient
	v[77]=v[71]/(1-v[72]);																			//sector 2 effecive direct and indirect input tech coefficient

	v[78]=v[67]+v[69]*v[76];																		//sector 1 effective direct and indirect propensity to consume
	v[79]=v[68]+v[69]*v[77];																		//sector 2 effective direct and indirect propensity to consume

	v[80]=v[73]+v[75]*v[76];																		//sector 1 effective direct and indirect capital exports propensity
	v[81]=v[74]+v[75]*v[77];																		//sector 2 effective direct and indirect capital exports propensity

v[101]=(v[43]*(1-v[81])+v[18]*v[79])/((1-v[81])*(1-v[78])-v[79]*v[80]);								//sector 1 initial demand
v[102]=(v[18]+v[101]*v[80])/(1-v[81]);																//sector 2 initial demand
v[103]=v[101]*v[76]+v[102]*v[77];																	//sector 3 initial demand

WRITES(cur1, "sector_initial_demand", v[101]);
WRITES(cur2, "sector_initial_demand", v[102]);
WRITES(cur3, "sector_initial_demand", v[103]);

/*****BEGIN CALCULATING AGGREGATE VARIABLES*****/

v[104]=v[58]*v[101]+v[59]*v[102]+v[60]*v[103];							 							//total taxes
v[105]=v[28]*v[40]*v[101]+v[29]*v[41]*v[102]+v[30]*v[42]*v[103];									//indirect taxes
v[106]=v[52]*v[101]+v[53]*v[102]+v[54]*v[103] + v[104];			 									//total wages including public
v[107]=v[55]*v[101]+v[56]*v[102]+v[57]*v[103];							 							//total profits
v[108]=v[34]*v[55]*v[101]+v[35]*v[56]*v[102]+v[36]*v[57]*v[103];									//distributed profits
v[109]=v[106]+v[107]+v[105];																		//GDP

/*****BEGIN WRITTING EXPORTS*****/

v[110]=v[61]*v[101]+v[62]*v[102]+v[63]*v[103];							 							//total imports
v[111]=v[49]*v[110];																				//sector 1 exports
v[112]=v[50]*v[110];																				//sector 1 exports
v[113]=v[51]*v[110];																				//sector 1 exports

WRITELLS(cur1, "Sector_Exports", v[111], 0, 1);														//write sector 1 initial exports
WRITELLS(cur2, "Sector_Exports", v[112], 0, 1);														//write sector 2 initial exports
WRITELLS(cur3, "Sector_Exports", v[113], 0, 1);														//write sector 3 initial exports

cur4 = SEARCH("EXTERNAL_SECTOR");																	//search the external sector
v[114]=VLS(cur4, "External_Income", 1);																//external income initial

CYCLE(cur, "SECTORS")																				//CYCLE trough sectors to write exports coefficient
{
	v[115]=VLS(cur, "Sector_Exports", 1);															//sector exports
	v[116]=V("exports_elasticity_income");													//sector income elasticicty of exports
	v[117]=(v[115]/(pow(v[114], v[116])));															//calculate sector exports coefficient
		WRITES(cur, "sector_exports_coefficient", v[117]);													//write the exports coefficient, assuming external price and foreign price starts as 1, so the exchange rate
}

/*****BEGIN WRITTING GOVERNMENT VARIABLES*****/

cur = SEARCH("GOVERNMENT");																				
WRITES(cur, "initial_taxes", v[104]);

	
PARAMETER
RESULT(0)


EQUATION("Initialization")
/*
Initializes the initial values for endogenous variables and parameters
Located at Root
Must be the first variable to be calculated, before Initial_Demand_Calibration
Depends on some Hypotesis:
1-No past growth, therefore expected demand is the current demand and there is no investment for productive capacity adjustment
2-All firms begin equally, with the same parameters and distribution (implies that all average and maximum values are the same and equal to the firm value)
3-All prices equals 1 initially
4-All capital good's productivity and therefor firm's average productivity equals 1 initially
5-Production equals demand. There is no Delivery Delay initially. 
Firm's productive capacity and number of capitals are endogenous, depending on production and desired degree of capacity utilization
*/

v[50]=v[51]=v[52]=v[53]=0;
CYCLE(cur, "SECTORS")
{
	//Sector Control Parameters
	v[0]=V("investment_period");
	v[1]=V("number_object_firms");     
	v[2]=V("depreciation_period");                                                   
	v[3]=V("initial_avg_price");                   
	v[4]=V("initial_external_price");                
	v[6]=V("initial_productivity");
	v[7]=V("initial_markup");
	v[8]=V("capital_output_ratio");
	v[9]=V("desired_inventories_proportion");	
	v[10]=V("desired_degree_capacity_utilization");
	v[11]=1/v[1]; 																				//market_share_initial
	v[13]=VS(cur, "sector_initial_demand");														//production_sector_initial (production=demand)
	v[12]=v[13]/v[1]; 																			//production_firm initial
	v[14]=((v[12]*(1+v[9]))/v[10])*v[8];														//number of capital goods of each firm
	v[17]=V("input_tech_coefficient"); 	
	v[16]=(v[3]/v[7])-v[17];                        											//sector initial wage
	
	v[18]=V("rnd_revenue_proportion");
	v[19]=V("indirect_tax_rate");
	v[23]=V("profits_distribution_rate");

	//Begin Wirtting Sector Independent Variables
		WRITELLS(cur, "Sector_Demand_Met", 1, 0, 1);                              				//it is assumed that all demand is met initially. Equals 1 by definition
		WRITELLS(cur, "Sector_Demand_Met_By_Imports", 1, 0, 1);                      			//it is assumed thatt all imports are met initially. Equals 1 by definition
		WRITELLS(cur, "Sector_Productive_Capacity_Available", 0, 0, 1);                  		//it is assumed that there is no entry or exit initially. Equals 0 by definition
		WRITELLS(cur, "Sector_Avg_Competitiveness", 1, 0, 1);                     				//if all firms are the same, equals 1 by definition
	//End Writting Sector Independent Variables
	
	//Begin Writting Sector Dependent Variables
		WRITELLS(cur, "Sector_Avg_Price", v[3], 0, 1);                                   		//Avg_Price equals avg_price initial
		WRITELLS(cur, "Sector_Avg_Price", v[3], 0, 2);											//Avg_Price equals avg_price initial
		WRITELLS(cur, "Sector_External_Price", v[4], 0, 1);                               		//Foreign_Price equals foreign_price initial
		WRITELLS(cur, "Sector_Avg_Productivity", v[6], 0,  1);               	 				//If all firms are the same, Avg Productivity will be the initial productivivity for all firms
		WRITELLS(cur, "Sector_Max_Productivity", v[6], 0,  1);                      			//If all capital goods have the same productivity, Max_Productivity equals productivity_initial 
	for (v[101]=1 ; v[101]<=8 ; v[101]=v[101]+1)                        		 				//for 8 lags
		WRITELLS(cur, "Sector_Effective_Orders", v[13], 0, v[101]);               				//Effective_Orders_Sectors equals demand_initial
		WRITELLS(cur, "Sector_Inventories", (v[13]*v[9]), 0, 1);                  				//Firms operate with desired level of inventories, thus, Current stock of inventories is the desired level times effective production
		WRITELLS(cur, "Sector_Productive_Capacity", ((v[13]*(1+v[9]))/v[10]), 0, 1);			//All firms start operating at desired degree of utilization, thus, productive capacity is endogenous calculated based on effective production and desired degree
	//End Writting Sector Dependent Variables
	
	cur1=SEARCHS(cur, "FIRMS");																	//search the first and only instance of firms below each sector
	  //Begin Writting Firm Parameters
		WRITES(cur1, "capital_goods_effective_orders_firm_temporary", 0);						//all temporary parameters starts at zero
		WRITES(cur1, "capital_goods_production_temporary", 0);									//all temporary parameters starts at zero
		WRITES(cur1, "consumption_effective_orders_firm_temporary", 0);	   	  					//all temporary parameters starts at zero
		WRITES(cur1, "intermediate_effective_orders_firm_temporary", 0);						//all temporary parameters starts at zero
		WRITES(cur1, "intermediate_production_firm_temporary", 0);								//all temporary parameters starts at zero
		WRITES(cur1, "firm_date_birth", 0);                                   					//zero by definition
		//End Writting Firm Parameters
	
		//Begin Writting Independent Firm Variables
		WRITELLS(cur1, "Firm_Competitiveness", 1, 0, 1);                           				//if all firms are the same
	  	WRITELLS(cur1, "Firm_Delivery_Delay", 1, 0, 1);                           		  		//it is assumed that all demand is met initially, so equals 1 by definition
	  for (v[102]=1 ; v[102]<=6 ; v[102]=v[102]+1)                                				//for 6 lags
	  	{
	  	WRITELLS(cur1, "Firm_Demand_Productive_Capacity_Replacement", 0, 0, v[102]);			//no replacement initially
	  	WRITELLS(cur1, "Firm_Debt_Rate", 0, 0, v[102]);											//no debt initially
	  	}
	  	WRITELLS(cur1, "Firm_Stock_Financial_Assets", 0, 0, 1);									//no financial assets initially
	  	WRITELLS(cur1, "Firm_Stock_Debt", 0, 0, 1);                                    			//no debt initially
	  	WRITELLS(cur1, "Firm_Avg_Debt_Rate", 0, 0, 1);                       					//no debt initially
	  for(v[122]=1; v[122]<=(v[0]+1); v[122]=v[122]+1)										 	//for (investment period+1) lags (7)
	 		{
	 		WRITELLS(cur1, "Firm_Productive_Capacity_Depreciation", 0, 0, v[122]);  			//write 0 
	 		WRITELLS(cur1, "Firm_Demand_Productive_Capacity_Expansion", 0, 0, v[122]);     		//write 0 
	 		}
	  //End Writting Independent Firm Variables
	  
	  //Begin Writting Dependent Firm Variables
	  for (v[107]=1 ; v[107]<=7 ; v[107]=v[107]+1)												//for 7 lags
	  	WRITELLS(cur1, "Firm_Market_Share", v[11], 0, v[107]);             						//firm's market share will be the inverse of the number of firms in the sector (initial market share)
	  	WRITELLS(cur1, "Firm_Effective_Market_Share", v[11], 0, 1);                    			//firm's effective market share will be the initial market share       
	  	WRITELLS(cur1, "Firm_Desired_Market_Share", (v[11]), 0, 1);                  			//firm's desired market share will be twice the initial market share  
	  	WRITELLS(cur1, "Firm_Avg_Market_Share", v[11], 0, 1);                     				//firm's avg market share will be the initial market share
	  	WRITELLS(cur1, "Firm_Price", v[3], 0, 1);                                      			//firm's price will be the initial price of the sector, the same for all firms
	  for (v[108]=1 ; v[108]<=6 ; v[108]=v[108]+1)                                				//for 6 lags
	  	{
	  	WRITELLS(cur1, "Firm_Frontier_Productivity", v[6], 0, v[108]);                 			//frontier productivity will be the initial frontier productivity
	  	WRITELLS(cur1, "Firm_Max_Productivity", v[6], 0, v[108]);        						//max capital goods productivity will be the initial frontier productivity that will be the same for all capital goods
	  	WRITELLS(cur1, "Firm_Avg_Productivity", v[6], 0, v[108]);								//firm's avg productivity will be the initial frontier productivity since all capital goods have the same productivity
	  	}
	  	WRITELLS(cur1, "Firm_Avg_Potential_Markup", v[7], 0, 1);								//avg potential markup will be the initial markup
	  	WRITELLS(cur1, "Firm_Desired_Markup", v[7], 0, 1); 										//desired markup will be the initial markup
	  for (v[111]=1 ; v[111]<=7 ; v[111]=v[111]+1)                                				//for 7 lags
	  	WRITELLS(cur1, "Firm_Potential_Markup", v[7], 0, v[111]);                      			//potential markup will be the initial markup
	  for (v[112]=1 ; v[112]<=12 ; v[112]=v[112]+1)												//for 12 lags
	  	WRITELLS(cur1, "Firm_Effective_Orders", v[12], 0, v[112]);                     			//firm's effective orders will be sector's effective orders (given by demand_initial) divided by the number of firms
	  	WRITELLS(cur1, "Firm_Sales", v[12], 0, 1);												//firm's sales will be equal to effective orders, no delivery delay
	  	WRITELLS(cur1, "Firm_Revenue", (v[12]*v[3]), 0, 1);                            			//firm's revenue will be the firm's sales times firm price
	  	WRITELLS(cur1, "Firm_Stock_Inventories", (v[12]*v[9]), 0, 1);                        	//firm's inventories will be the sales times the desired inventories proportion (parameter)
	  	WRITELLS(cur1, "Firm_Stock_Inputs", (v[17]*v[12]), 0, 1);                      			//firm's stock of imputs will be the sales times the input tech relationship
	  	WRITELLS(cur1, "Firm_Productive_Capacity", ((v[12]*(1+v[9]))/v[10]), 0, 1);				//firm productive capacity will be the sales divided by the desired degree of capacity utlization (parameter)
	  	WRITELLS(cur1, "Firm_Capital", v[14], 0, 1);											//firm nominal capital equals number of capital if capital goods price equals 1
	  	WRITELLS(cur1, "Firm_Wage", v[16], 0, 1); 												//firm's nominal wage equals sector nominal wage initial
			WRITELLS(cur1, "Firm_Variable_Cost", (v[16]+v[17]), 0, 1);							//firm variable cost equals unitary wage plus unitary cost of inputs. The last equals the tech coefficient if input price equals 1
				  		
	  		//Begin writting Capital Goods Variables and parameters
	  		cur2=SEARCHS(cur1, "CAPITALS");														//search the first and only instance of capital below firms
	  		WRITELLS(cur2, "Capital_Good_Acumulated_Production", 0, 0, 1);      				//zero by definition
				WRITES(cur2, "capital_good_productive_capacity", (1/v[8]));     				//inverse of capital output ratio  
				WRITES(cur2, "capital_good_productivity_initial", v[6]);       		  			//defined in the control parameters
				WRITES(cur2, "capital_good_to_replace", 0);
				WRITES(cur2, "id_capital_good_number", 1);                       				//1 for the first and only initial firm (the others will be created below)
	 			
	 			//Begin Creating Firms and writting some parameters
	 			for(v[121]=1; v[121]<=(v[1]-1); v[121]=v[121]+1)								//for the number of firms of each sector (defined by the parameter)
	 				cur4=ADDOBJ_EXLS(cur,"FIRMS", cur1, 0);                             		//create new firm using the first and only firm as example
	 			
	 			CYCLES(cur, cur1, "FIRMS")                                                 		//CYCLE trough all firms
	 				{
	 				v[26]=SEARCH_INSTS(root, cur1);												//search current firm position in the total economy
	 				WRITES(cur1, "id_firm_number", v[26]);                         				//write the firm number as the current position (only initial difference between firms)
	 				v[27]=fmod((double) (v[26]+v[0]), v[0]);                                 	//divide the firm's number plus investment period by the investment period and takes the rest (possible results if investment period =6 are 0, 5, 4, 3, 2, 1)
	 				
	 				//Begin creating capital goods and writting "capital_good_date_birth"		
	 				for(v[120]=1; v[120]<=v[14]; v[120]=v[120]+1)                        		//for the number of capital goods of each firm
					{
					cur2=SEARCHS(cur1, "CAPITALS");                                         	//search the first and only capital good of each firm
					cur3=ADDOBJ_EXLS(cur1,"CAPITALS", cur2, 0);			                       	//create new capital goods using the first one as example
					WRITES(cur3, "id_capital_good_number", (v[120]+1));							//write the capital good number
					}
					CYCLES(cur1, cur5, "CAPITALS")                                            	//CYCLE trough all capital goods
	 					{
	 					v[28]=VS(cur5, "id_capital_good_number");
						v[25]=(-v[2]+v[27]+1)+(v[28]-1)*v[0];                                  	//calculates the capital good date of birth based on the firm number and the number of the capital good
						WRITES(cur5, "capital_good_date_birth", v[25]);							//write the capital good date of birth
						}
					}
					
	//Begin GDP Calculation inside sector
	v[20]=v[13]*v[3]*v[19];                           											//sector indirect taxes
	v[21]=v[16]*(v[13]/v[6]+v[18]*(1-v[19])*v[3]);      										//sector wages including R&D wages
	v[22]=(v[3]*(1-v[19])*(1-v[18])-((v[16]/v[6])+v[17]))*v[13];								//sector surplus
	v[50]=v[50]+v[20];																			//total indirect taxes
	v[51]=v[51]+v[21];																			//total wages
	v[52]=v[52]+v[22];																			//total surplus
	v[53]=v[53]+(v[23]*v[22]);																	//total distributed profits
}

//Begin Writing Government Variables
cur = SEARCH("GOVERNMENT");
v[41]=VS(cur, "initial_taxes"); 																//initial total taxes is calculated in the demand calibration based only on parameters
WRITELLS(cur,"Total_Taxes", v[41], 0, 1);														//write initial total taxes
WRITELLS(cur,"Government_Max_Expenses", v[41], 0, 1);        									//initial max government expenses equals total taxes calculated in the calibration
WRITELLS(cur,"Government_Wages", v[41], 0, 1);		            								//initial government expenses is only wages, which thereafter will grow depending on inflation and average productivity		
for (v[114]=1 ; v[114]<=4 ; v[114]=v[114]+1)		              								//for 4 lags	
	WRITELLS(cur,"Government_Debt", 0, 0, v[114]);                  							//no debt initially	
v[46]=VS(cur, "interest_rate");																	//base interest rate parameter
WRITELLS(cur,"Basic_Interest_Rate", v[46], 0, 1);												//write initial interest rate


cur3 = SEARCH_CND("id_consumption_goods_sector", 1);
v[47]=V("number_object_firms");																	//initial total autonomous consumption, 1 good for each consumption goods firm

//Begin Writing Classes Variables
v[37]=v[39]=0;
CYCLE(cur, "CLASSES")
{
	v[29]=VS(cur, "class_propensity_to_consume");                								//class parameter
	v[30]=VS(cur, "class_profit_share");                						   		  		//class parameter
	v[31]=VS(cur, "class_wage_share"); 															//class parameter
	v[32]=VS(cur, "class_direct_tax");															//class parameter
	v[33]=(v[30]*v[53]+v[31]*(v[51]+v[41]))*(1-v[32]);             								//class nominal net income
	v[34]=(v[30]*v[53]+v[31]*(v[41]+v[51]))*(v[32]);             								//class tax payment
	v[35]=VS(cur, "class_propensity_to_import");												//class parameter
	v[36]=v[33]*v[35];																			//class imports
	v[37]=v[37]+v[36];																			//total imports
	v[38]=v[31]*v[47];																			//class initial autonomous consumption
	v[39]=v[39]+v[34];																			//total taxes payed by classes
		for (v[113]=1 ; v[113]<=4 ; v[113]=v[113]+1)                          					//for 4 lags
			{
			WRITELLS(cur, "Class_Nominal_Income", v[33], 0, v[113]);            				//writes Class_Nominal_Income
			WRITELLS(cur, "Class_Real_Income", v[33], 0, v[113]);								//writes Class_Real_Income
			}
			WRITELLS(cur, "Class_Autonomous_Consumption", v[38], 0, 1);         				//write class' autonomous consumption
			WRITELLS(cur, "Class_Debt", 0, 0, 1);                              					//0, no debt initially
			WRITELLS(cur, "Class_Debt_Payment", 0, 0, 1);										//0, no debt initially
			WRITELLS(cur, "Class_Financial_Assets", 0, 0, 1);									//0, no financial assets initially
			WRITELLS(cur, "Class_Financial_Assets", 0, 0, 2);									//0, no financial assets initially
}
//End CYCLE on CLASSES


v[99]=v[50]+v[51]+v[52]+v[41]; 																	//GDP

v[48]=V("number_object_firms")*3;																//total number of firms
cur2 = SEARCH("EXTERNAL_SECTOR");																//search the external sector
WRITELLS(cur2, "External_Income", v[99], 0, 1);													//initial external income is equal to nominal gdp
v[42]=VLS(cur2, "External_Income", 1);															//external income initial

CYCLE(cur, "SECTORS")																			//CYCLE trough sectors to write exports coefficient
{
	v[49]=V("number_object_firms");																//sector number of firms
	v[45]=v[49]/v[48];																			//sector share of firms
		WRITELLS(cur, "Sector_Exports", (v[37]*v[45]), 0, 1);									//write sector initial exports
	v[43]=V("exports_elasticity_income");														//sector income elasticicty of exports
	v[44]=((v[37]*v[45])/(pow(v[42], v[43])));													//calculate sector exports coefficient
		WRITES(cur, "sector_exports_coefficient", v[44]);										//write the exports coefficient, assuming external price and foreign price starts as 1, so the exchange rate
}

//Begin Writting Macro Variables
CYCLE(cur, "MACRO")
{
		WRITELLS(cur,"Likelihood_Crisis", 0, 0, 1);                  							//zero by definition
		WRITELLS(cur,"Annual_Real_Growth", 0, 0, 1);                 							//zero by definition, no growth initally
	for (v[116]=1 ; v[116]<=5 ; v[116]=v[116]+1)                  								//for 5 lags
		{
		WRITELLS(cur,"Price_Index", 1, 0, v[116]);									 			//writes Price_Index, all initial price index is 1
		WRITELLS(cur,"Consumer_Price_Index", 1, 0, v[116]);          							//writes Consumper_Price_Index, all initial price index is 1
		}
	for (v[117]=1 ; v[117]<=10 ; v[117]=v[117]+1)                  								//for 10 lags
		WRITELLS(cur,"GDP", v[99], 0, v[117]);                     	 							//GDP
	for (v[118]=1 ; v[118]<=8 ; v[118]=v[118]+1)                   								//for 8 lags
		WRITELLS(cur,"Real_GDP", v[99], 0, v[118]);                  							//Real GDP will be equal to nominal GDP because price index always begins as 1
}

PARAMETER
RESULT(0)





