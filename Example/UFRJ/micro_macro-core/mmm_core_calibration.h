EQUATION("Calibration")

v[0]=V("i_per");               				
v[1]=V("mk_per");
v[2]=V("c_per");									
v[3]=V("a_per");
v[4]=V("gov_per");
v[5]=V("depre_per");
v[6]=V("ir");
v[7]=V("aut_con");

//Class Aggregation
v[20]=v[21]=v[22]=v[23]=v[24]=v[25]=0;
CYCLE(cur, "CLASSES")
{
	v[26]=VS(cur, "class_profit_share");
	v[27]=VS(cur, "class_wage_share");
	v[28]=VS(cur, "class_propensity_to_consume");
	v[29]=VS(cur, "class_propensity_to_import");
	v[30]=VS(cur, "class_direct_tax");
	
	v[20]=v[20]+v[27]*v[28]*(1-v[30]);				//effective aggregate propensity to consume on wages
	v[21]=v[21]+v[26]*v[28]*(1-v[30]);				//effective aggregate propensity to consume on profits
	
	v[22]=v[22]+v[26]*v[30];						//effective direct tax over profits
	v[23]=v[23]+v[27]*v[30];						//effective direct tax over wages
	v[24]=v[24]+v[27]*v[29]*(1-v[30]);				//effective aggregate propensity to import on wages
	v[25]=v[25]+v[26]*v[29]*(1-v[30]);				//effective aggregate propensity to import on profits
}

v[41]=V("C_eta");									//C sector initial number of firms
v[42]=V("K_eta");									//K sector initial number of firms
v[43]=V("I_eta");									//I sector initial number of firms

v[44]=V("C_phi");									//C sector intial productivity
v[45]=V("K_phi");									//K sector intial productivity
v[46]=V("I_phi");									//I sector intial productivity

v[47]=V("C_mk");									//C sector initial markup
v[48]=V("K_mk");									//K sector initial markup
v[49]=V("I_mk");									//I sector initial markup

v[50]=V("C_w");										//C sector initial nominal wage
v[51]=V("K_w");										//K sector initial nominal wage
v[52]=V("I_w");										//I sector initial nominal wage

v[53]=V("C_alpha");									//C sector input tech coefficient
v[54]=V("K_alpha");									//K sector input tech coefficient
v[55]=V("I_alpha");									//I sector input tech coefficient

v[56]=V("C_beta");									//C sector capital output ratio
v[57]=V("K_beta");									//K sector capital output ratio
v[58]=V("I_beta");									//I sector capital output ratio

v[59]=V("C_lambda");								//C sector rnd revenue proportion
v[60]=V("K_lambda");								//K sector rnd revenue proportion
v[61]=V("I_lambda");								//I sector rnd revenue proportion

v[62]=V("C_d");										//C sector profits distribution rate
v[63]=V("K_d");										//K sector profits distribution rate
v[64]=V("I_d");										//I sector profits distribution rate

v[65]=V("C_tau");									//C sector indirect tax rate
v[66]=V("K_tau");									//K sector indirect tax rate
v[67]=V("I_tau");									//I sector indirect tax rate

v[68]=V("C_q");										//C sector initial quality
v[69]=V("K_q");										//K sector initial quality
v[70]=V("I_q");										//I sector initial quality

v[71]=V("C_i_per");									//C sector investment period
v[72]=V("K_i_per");									//K sector investment period
v[73]=V("I_i_per");									//I sector investment period

v[75]=V("C_exp_sh");								//C sector share of exports
v[76]=V("K_exp_sh");								//K sector share of exports
v[77]=V("I_exp_sh");								//i sector share of exports

v[78]=V("C_psi");									//C sector desired degree of capacity utilization
v[79]=V("K_psi");									//K sector desired degree of capacity utilization
v[80]=V("I_psi");									//I sector desired degree of capacity utilization

v[81]=V("C_sigma");									//C sector desired inventories proportion
v[82]=V("K_sigma");									//K sector desired inventories proportion
v[83]=V("I_sigma");									//I sector desired inventories proportion

v[84]=V("C_delta");									//C sector depreciation period
v[85]=V("K_delta");									//K sector depreciation period
v[86]=V("I_delta");									//I sector depreciation period

v[87]=V("C_mk_per");								//C sector markup period
v[88]=V("K_mk_per");								//K sector markup period
v[89]=V("I_mk_per");								//I sector markup period

v[100]=(v[49]*v[52]/v[46])/(1-v[49]*v[55]);														//intermediate sector price
v[101]=v[47]*((v[50]/v[44])+(v[100]*v[53]));													//consumption sector price
v[102]=v[48]*((v[51]/v[45])+(v[100]*v[54]));													//capital sector price

v[103]=(v[50]/v[44]) + v[59]*(1-v[65])*v[101];													//C sector effective wage margin over production including RND
v[104]=(v[51]/v[45]) + v[60]*(1-v[66])*v[102];													//K sector effective wage margin over production including RND
v[105]=(v[52]/v[46]) + v[61]*(1-v[67])*v[100];													//I sector effective wage margin over production including RND

v[106]=v[101]*(1-v[65])*(1-v[59])-((v[50]/v[44]) + v[53]*v[100]);								//C sector effective profit margin over production
v[107]=v[102]*(1-v[66])*(1-v[60])-((v[51]/v[45]) + v[54]*v[100]);								//K sector effective profit margin over production
v[108]=v[100]*(1-v[67])*(1-v[61])-((v[52]/v[46]) + v[55]*v[100]);								//I sector effective profit margin over production

v[109]=(v[101]*v[65] +v[22]*v[62]*v[106] + v[23]*v[103])/(1-v[23]);								//C sector effective tax-public wage over production
v[110]=(v[102]*v[66] +v[22]*v[63]*v[107] + v[23]*v[104])/(1-v[23]);								//K sector effective tax-public wage over production
v[111]=(v[100]*v[67] +v[22]*v[64]*v[108] + v[23]*v[105])/(1-v[23]);								//I sector effective tax-public wage over production

v[112]=(v[103]+v[109])*v[24] + (v[106]*v[62])*v[25];											//C sector effective imports propensity
v[113]=(v[104]+v[110])*v[24] + (v[107]*v[63])*v[25];											//K sector effective imports propensity
v[114]=(v[105]+v[111])*v[24] + (v[108]*v[64])*v[25];											//I sector effective imports propensity

v[115]=((v[20]+(v[24]*v[75]))*(v[103]+v[109]) + (v[21]+(v[25]*v[75]))*v[106]*v[62])/v[101];		//C sector effective consumption propensity including exports
v[116]=((v[20]+(v[24]*v[75]))*(v[104]+v[110]) + (v[21]+(v[25]*v[75]))*v[107]*v[63])/v[101];		//K sector effective consumption propensity including exports
v[117]=((v[20]+(v[24]*v[75]))*(v[105]+v[111]) + (v[21]+(v[25]*v[75]))*v[108]*v[64])/v[101];		//I sector effective consumption propensity including exports

v[118]=v[53]+((v[24]*v[77]*(v[103]+v[109])) + (v[25]*v[77]*v[106]*v[62]))/v[100];				//C sector input tech coefficient including exports
v[119]=v[54]+((v[24]*v[77]*(v[104]+v[110])) + (v[25]*v[77]*v[107]*v[63]))/v[100];				//K sector input tech coefficient including exports
v[120]=v[55]+((v[24]*v[77]*(v[105]+v[111])) + (v[25]*v[77]*v[108]*v[64]))/v[100];				//I sector input tech coefficient including exports

v[121]=((v[24]*v[76]*(v[103]+v[109])) + (v[25]*v[76]*v[106]*v[62]))/v[102];						//C sector effective capital exports propensity
v[122]=((v[24]*v[76]*(v[104]+v[110])) + (v[25]*v[76]*v[107]*v[63]))/v[102];						//K sector effective capital exports propensity
v[123]=((v[24]*v[76]*(v[105]+v[111])) + (v[25]*v[76]*v[108]*v[64]))/v[102];						//I sector effective capital exports propensity

v[124]=v[118]/(1-v[120]);																		//sector direct and indirect input tech
v[125]=v[119]/(1-v[120]);																		//sector direct and indirect input tech

v[126]=v[115]+v[117]*v[124];																	//sector effective direct and indirect propensity to consume
v[127]=v[116]+v[117]*v[125];																	//sector effective direct and indirect propensity to consume

v[128]=v[121]+v[123]*v[124];																	//sector effective direct and indirect capital propensity
v[129]=v[122]+v[123]*v[125];																	//sector effective direct and indirect capital propensity

//Begin Aggregate Calculations
v[141]=v[7]/(1-v[126]-v[127]*v[128]/(1-v[129]));												//consumption sector initial demand
v[142]=(v[141]*v[128])/(1-v[129]);																//capital sector initial demand
v[143]=v[141]*v[124]+v[142]*v[125];																//intermediate sector initial demand

v[144]=v[109]*v[141] + v[110]*v[142] + v[111]*v[143];							 				//total taxes
v[145]=v[65]*v[101]*v[141] + v[66]*v[102]*v[142] + v[67]*v[100]*v[143];							//indirect taxes
v[146]=v[103]*v[141] + v[104]*v[142] + v[105]*v[143] + v[144];									//total wages including public
v[147]=v[106]*v[141] + v[107]*v[142] + v[108]*v[143];							 				//total profits
v[148]=v[62]*v[106]*v[141] + v[63]*v[107]*v[142] + v[64]*v[108]*v[143];							//distributed profits
v[149]=v[112]*v[141] + v[113]*v[142] + v[114]*v[143];											//total imports
v[150]=v[146]+v[147]+v[145];																	//GDP
v[151]=(v[141]*v[101]+v[142]*v[102]+v[143]*v[100])/(v[141]+v[142]+v[143]);						//initial price index

//WRITTING MACRO VARIABLES
cur = SEARCH("MACRO");
WRITES(cur, "initial_avg_price", v[151]);
WRITELLS(cur,"Likelihood_Crisis", 0, 0, 1);                  				//zero by definition
WRITELLS(cur,"Annual_Growth", 0, 0, 1);										//zero by definition, no growth initally
WRITELLS(cur,"Annual_Real_Growth", 0, 0, 1);                 				//zero by definition, no growth initally
for (i=1 ; i<=(V("a_per")+1) ; i++)                  						//for (annual period +1) lags
	{
	WRITELLS(cur,"Price_Index", v[151], 0, i);								//writes Price_Index
	WRITELLS(cur,"Consumer_Price_Index", v[101], 0,i);          			//writes Consumper_Price_Index
	}
for (i=1 ; i<=2*V("a_per") ; i++)                  							//for (2*annual_period) lags
	{	
	WRITELLS(cur,"GDP", v[150], 0, i);                     	 				//writes GDP
	WRITELLS(cur,"Real_GDP", (v[150]/v[151]), 0, i);                		//writes GDP Real 
	}

//WRITTING EXTERNAL SECTOR VARIABLES
cur = SEARCH("EXTERNAL_SECTOR");											//search the external sector object
WRITELLS(cur, "External_Income", v[150], 0, 1);								//writes initial external income equal to domestic GDP

//WRITTING GOVERNMENT VARIABLES
cur = SEARCH("GOVERNMENT");										
WRITELLS(cur,"Total_Taxes", v[144], 0, 1);									//write initial total taxes
WRITELLS(cur,"Government_Max_Expenses", v[144], 0, 1);        				//initial max government expenses equals total taxes calculated in the calibration
WRITELLS(cur,"Government_Wages", v[144], 0, 1);		            			//initial government expenses is only wages, which thereafter will grow depending on inflation and average productivity		
for (i=1 ; i<=V("gov_per");  i++)		              						//for (government_period) lags	
	WRITELLS(cur,"Government_Debt", 0, 0, i);                  				//no debt initially	
WRITELLS(cur,"Basic_Interest_Rate", V("ir"), 0, 1);							//writes interest rate

//WRITTING CLASS VARIABLES
v[152]=COUNT("CLASSES");
CYCLE(cur, "CLASSES")
{
	v[153]=VS(cur, "class_profit_share");                					//class parameter
	v[154]=VS(cur, "class_wage_share"); 									//class parameter
	v[155]=VS(cur, "class_direct_tax");										//class parameter
	v[156]=(v[153]*v[148]+v[154]*v[146])*(1-v[155]);             			//class nominal net income																		//total imports
	v[157]=v[7]/v[152];														//class initial autonomous consumption
		for (i=1 ; i<=V("c_per") ; i++)                          			//for (class_period) lags
			{
			WRITELLS(cur, "Class_Nominal_Income", v[156], 0,i);            	//writes Class_Nominal_Income
			WRITELLS(cur, "Class_Real_Income", (v[156]/v[101]), 0,i);		//writes Class_Real_Income
			}
			WRITELLS(cur, "Class_Autonomous_Consumption", v[157], 0, 1);    //write class' autonomous consumption
			WRITELLS(cur, "Class_Debt", 0, 0, 1);                           //0, no debt initially
			WRITELLS(cur, "Class_Debt_Payment", 0, 0, 1);					//0, no debt initially
			WRITELLS(cur, "Class_Financial_Assets", 0, 0, 1);				//0, no financial assets initially
			WRITELLS(cur, "Class_Financial_Assets", 0, 0, 2);				//0, no financial assets initially
}

//WRITTING SECTOR VARIABLES
cur1=SEARCH_CND("id_consumption_goods_sector",1);
cur2=SEARCH_CND("id_capital_goods_sector",1);
cur3=SEARCH_CND("id_intermediate_goods_sector",1);

CYCLE(cur, "SECTORS")
{
	if(VS(cur,"id_consumption_goods_sector")==1)
	{
	v[160]=v[141];																						//sector inital demand
	v[161]=V("C_eta");
	v[162]=v[101];																						//sector initial price
	v[163]=v[141]/V("C_eta");																			//firm production
	v[164]=v[163]*V("C_beta")/V("C_psi");																//number of capital goods for each firm
	v[165]=(v[149]*V("C_exp_sh"))/(pow(v[150], VS(cur,"sector_exports_elasticity_income")));
	v[166]=(v[149]*V("C_exp_sh"))/v[162];
	v[167]=1/V("C_eta");																				//sector market share
	v[168]=V("C_mk");
	v[169]=V("C_w");
	v[170]=(V("C_w")/V("C_phi"))+V("C_alpha")*v[100];
	WRITES(cur, "sector_exports_coefficient", v[165]);
	WRITES(cur, "sector_capital_output_ratio", V("C_beta"));
	WRITES(cur, "sector_depreciation_period", V("C_delta"));
	WRITES(cur, "sector_desired_degree_capacity_utilization", V("C_psi"));
	WRITES(cur, "sector_desired_inventories_proportion", V("C_sigma"));
	WRITES(cur, "sector_indirect_tax_rate", V("C_tau"));
	WRITES(cur, "sector_initial_productivity", V("C_phi"));
	WRITES(cur, "sector_initial_quality", V("C_q"));
	WRITES(cur, "sector_input_tech_coefficient", V("C_alpha"));
	WRITES(cur, "sector_investment_period", V("C_i_per"));
	WRITES(cur, "sector_markup_period", V("C_mk_per"));
	WRITES(cur, "sector_profits_distribution_rate", V("C_d"));
	WRITES(cur, "sector_rnd_revenue_proportion", V("C_lambda"));
	}
	
	if(VS(cur,"id_capital_goods_sector")==1)
	{
	v[160]=v[142];																					//sector inital demand
	v[161]=V("K_eta");
	v[162]=v[102];																					//sector initial price
	v[163]=v[142]/V("K_eta");																		//firm production
	v[164]=v[163]*V("K_beta")/V("K_psi");															//number of capital goods for each firm
	v[165]=(v[149]*V("K_exp_sh"))/(pow(v[150], VS(cur,"sector_exports_elasticity_income")));
	v[166]=(v[149]*V("K_exp_sh"))/v[162];
	v[167]=1/V("K_eta");																			//sector market share
	v[168]=V("K_mk");
	v[169]=V("K_w");
	v[170]=(V("K_w")/V("K_phi"))+V("K_alpha")*v[100];
	WRITES(cur, "sector_exports_coefficient", v[165]);
	WRITES(cur, "sector_capital_output_ratio", V("K_beta"));
	WRITES(cur, "sector_depreciation_period", V("K_delta"));
	WRITES(cur, "sector_desired_degree_capacity_utilization", V("K_psi"));
	WRITES(cur, "sector_desired_inventories_proportion", V("K_sigma"));
	WRITES(cur, "sector_indirect_tax_rate", V("K_tau"));
	WRITES(cur, "sector_initial_productivity", V("K_phi"));
	WRITES(cur, "sector_initial_quality", V("K_q"));
	WRITES(cur, "sector_input_tech_coefficient", V("K_alpha"));
	WRITES(cur, "sector_investment_period", V("K_i_per"));
	WRITES(cur, "sector_markup_period", V("K_mk_per"));
	WRITES(cur, "sector_profits_distribution_rate", V("K_d"));
	WRITES(cur, "sector_rnd_revenue_proportion", V("K_lambda"));
	}
	
	if(VS(cur,"id_intermediate_goods_sector")==1)
	{
	v[160]=v[143];																					//sector inital demand
	v[161]=V("I_eta");
	v[162]=v[100];																					//sector initial price
	v[163]=v[143]/V("I_eta");																		//firm production
	v[164]=v[163]*V("I_beta")/V("I_psi");															//number of capital goods for each firm
	v[165]=(v[149]*V("I_exp_sh"))/(pow(v[150], VS(cur,"sector_exports_elasticity_income")));		//sector exports coefficient
	v[166]=(v[149]*V("I_exp_sh"))/v[162];															//sector real exports
	v[167]=1/V("I_eta");																			//sector market share
	v[168]=V("I_mk");
	v[169]=V("I_w");
	v[170]=(V("I_w")/V("I_phi"))+V("I_alpha")*v[100];
	WRITES(cur, "sector_exports_coefficient", v[165]);
	WRITES(cur, "sector_capital_output_ratio", V("I_beta"));
	WRITES(cur, "sector_depreciation_period", V("I_delta"));
	WRITES(cur, "sector_desired_degree_capacity_utilization", V("I_psi"));
	WRITES(cur, "sector_desired_inventories_proportion", V("I_sigma"));
	WRITES(cur, "sector_indirect_tax_rate", V("I_tau"));
	WRITES(cur, "sector_initial_productivity", V("I_phi"));
	WRITES(cur, "sector_initial_quality", V("I_q"));
	WRITES(cur, "sector_input_tech_coefficient", V("I_alpha"));
	WRITES(cur, "sector_investment_period", V("I_i_per"));
	WRITES(cur, "sector_markup_period", V("I_mk_per"));
	WRITES(cur, "sector_profits_distribution_rate", V("I_d"));
	WRITES(cur, "sector_rnd_revenue_proportion", V("I_lambda"));
	}
		
	for (i=1 ; i<=VS(cur,"sector_investment_period") ; i++)											
		{
		WRITELLS(cur, "Sector_Demand_Met", 1, 0, i); 										    
		WRITELLS(cur, "Sector_Demand_Met_By_Imports", 1, 0, i);                      			
		WRITELLS(cur, "Sector_Effective_Orders", v[160], 0, i);               				
		}		
	for (i=1 ; i<=V("c_per")+1 ; i++)                        		 					
		WRITELLS(cur, "Sector_Avg_Quality", VS(cur, "sector_initial_quality"), 0, i);        		
	WRITELLS(cur, "Sector_Productive_Capacity_Available", 0, 0, 1);               
	WRITELLS(cur, "Sector_Avg_Competitiveness", 1, 0, 1);                     		
	WRITELLS(cur, "Sector_Avg_Price", v[162], 0, 1);                             
	WRITELLS(cur, "Sector_Avg_Price", v[162], 0, 2);										
	WRITELLS(cur, "Sector_External_Price", v[162], 0, 1);                               		
	WRITELLS(cur, "Sector_Avg_Productivity", VS(cur, "sector_initial_productivity"), 0,  1);               	 		
	WRITELLS(cur, "Sector_Max_Productivity", VS(cur, "sector_initial_productivity"), 0,  1);                      		
	WRITELLS(cur, "Sector_Max_Quality", VS(cur, "sector_initial_quality"), 0,  1);
	WRITELLS(cur, "Sector_Inventories", (v[160]*VS(cur, "sector_desired_inventories_proportion")), 0, 1);                  			
	WRITELLS(cur, "Sector_Productive_Capacity", (v[160]/VS(cur, "sector_desired_degree_capacity_utilization")), 0, 1);			
	WRITELLS(cur, "Sector_Exports", v[166], 0, 1);																			
	
	cur1=SEARCHS(cur, "FIRMS");										
	WRITES(cur1, "firm_date_birth", 0);                                   					
	
	//WRITTING FIRM VARIABLES
	for (i=1 ; i<=VS(cur,"sector_investment_period") ; i++) 
		{	
	  	WRITELLS(cur1, "Firm_Demand_Productive_Capacity_Replacement", 0, 0, i);							
	  	WRITELLS(cur1, "Firm_Debt_Rate", 0, 0, i);															
	  	WRITELLS(cur1, "Firm_Demand_Capital_Goods", 0, 0, i);
	  	WRITELLS(cur1, "Firm_Frontier_Productivity", VS(cur, "sector_initial_productivity"), 0, i);    			
	  	WRITELLS(cur1, "Firm_Max_Productivity", VS(cur, "sector_initial_productivity"), 0, i);      			
	  	WRITELLS(cur1, "Firm_Avg_Productivity", VS(cur, "sector_initial_productivity"), 0, i);					
		}
	for(i=1; i<=VS(cur,"sector_investment_period")+1; i++)										 				
		{
		WRITELLS(cur1, "Firm_Productive_Capacity_Depreciation", 0, 0, i);  									
		WRITELLS(cur1, "Firm_Demand_Productive_Capacity_Expansion", 0, 0, i);     							
		}
	for (i=1 ; i<=2*VS(cur,"sector_investment_period") ; i++)												
	  	WRITELLS(cur1, "Firm_Effective_Orders", v[163], 0, i);                     						
	for (i=1 ; i<=VS(cur,"sector_markup_period")-1 ; i++)														
		{
		WRITELLS(cur1, "Firm_Market_Share", v[167], 0, i);             										
	  	WRITELLS(cur1, "Firm_Potential_Markup", v[168], 0, i);                      						
		}
		WRITELLS(cur1, "Firm_Effective_Market_Share", v[167], 0, 1);                    					   
	WRITELLS(cur1, "Firm_Desired_Market_Share", v[167], 0, 1);                  						 
	WRITELLS(cur1, "Firm_Avg_Market_Share", v[167], 0, 1);                     						
	WRITELLS(cur1, "Firm_Price", v[162], 0, 1);                                      				
	WRITELLS(cur1, "Firm_Avg_Potential_Markup", v[168], 0, 1);										
	WRITELLS(cur1, "Firm_Desired_Markup", v[168], 0, 1); 												
	WRITELLS(cur1, "Firm_Sales", v[163], 0, 1);														
	WRITELLS(cur1, "Firm_Revenue", v[163]*v[162], 0, 1);                            					
	WRITELLS(cur1, "Firm_Stock_Inventories", v[163]*VS(cur, "sector_desired_inventories_proportion"), 0, 1);                       
	WRITELLS(cur1, "Firm_Stock_Inputs", VS(cur, "sector_input_tech_coefficient")*v[163], 0, 1);                      						
	WRITELLS(cur1, "Firm_Productive_Capacity", v[163]/VS(cur, "sector_desired_degree_capacity_utilization"), 0, 1);								
	WRITELLS(cur1, "Firm_Capital", (v[164]*v[102]), 0, 1);												
	WRITELLS(cur1, "Firm_Wage", v[169], 0, 1); 														
	WRITELLS(cur1, "Firm_Variable_Cost", v[170], 0, 1);								
	WRITELLS(cur1, "Firm_Competitiveness", 1, 0, 1);                           							
	WRITELLS(cur1, "Firm_Delivery_Delay", 1, 0, 1);                           		  					
	WRITELLS(cur1, "Firm_Stock_Financial_Assets", 0, 0, 1);											
	WRITELLS(cur1, "Firm_Stock_Debt", 0, 0, 1);                                    						
	WRITELLS(cur1, "Firm_Avg_Debt_Rate", 0, 0, 1);                       							
		
	//WRITTING CAPITAL VARIABLES
	cur2=SEARCHS(cur1, "CAPITALS");													
	WRITELLS(cur2, "Capital_Good_Acumulated_Production", 0, 0, 1);      				
	WRITES(cur2, "capital_good_productive_capacity", 1/VS(cur, "sector_capital_output_ratio"));     					  
	WRITES(cur2, "capital_good_productivity_initial", VS(cur, "sector_initial_productivity"));       		  				
	WRITES(cur2, "capital_good_to_replace", 0);
	WRITES(cur2, "id_capital_good_number", 1);                       					
	 			
	//CREATING FIRMS AND CAPITALS
	for(i=1; i<=(v[161]-1); i++)										
	cur4=ADDOBJ_EXLS(cur,"FIRMS", cur1, 0);                             					
	 			
	CYCLES(cur, cur1, "FIRMS")                                                 			
	{
	v[180]=SEARCH_INSTS(cur, cur1);														
	WRITES(cur1, "id_firm_number", v[180]);                         					
	v[181]=fmod((double) (v[180]+VS(cur,"sector_investment_period")), VS(cur,"sector_investment_period"));   
			
	//Begin creating capital goods and writting "capital_good_date_birth"		
	cur2=SEARCHS(cur1, "CAPITALS");   
	for(i=1; i<=v[164]-1; i++)                        				
	{                                     			
	cur3=ADDOBJ_EXLS(cur1,"CAPITALS", cur2, 0);			                       			
	WRITES(cur3, "id_capital_good_number", i+1);									
	}
	CYCLES(cur1, cur5, "CAPITALS")                                            			
	{
	v[182]=VS(cur5, "id_capital_good_number");
	v[183]=(-VS(cur,"sector_depreciation_period")+v[181]+1)+(v[182]-1)*VS(cur,"sector_investment_period");                                  			
	WRITES(cur5, "capital_good_date_birth", 0);											
//	WRITES(cur5, "capital_good_depreciation_period", (v[183]+VS(cur,"sector_depreciation_period")));
	WRITES(cur5, "capital_good_depreciation_period", 700);
	}
}					
}		

PARAMETER
RESULT(0)
