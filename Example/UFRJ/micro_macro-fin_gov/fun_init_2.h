
EQUATION("Initialization_2")

consumption=SEARCH_CND("id_consumption_goods_sector",1);
capital=SEARCH_CND("id_capital_goods_sector",1);
input=SEARCH_CND("id_intermediate_goods_sector",1);
government=SEARCH("GOVERNMENT");
financial=SEARCH("FINANCIAL");
external=SEARCH("EXTERNAL_SECTOR");
country=SEARCH("COUNTRY");
centralbank=SEARCH("CENTRAL_BANK");
aclass=SEARCH_CND("class_id",1);
bclass=SEARCH_CND("class_id",2);
cclass=SEARCH_CND("class_id",3);

//COUNTRY PARAMETERS
v[0]=VS(country, "annual_frequency");
v[1]=VS(country, "country_initial_depreciation_share_GDP");
v[2]=VS(country, "country_initial_government_share_GDP");
v[3]=VS(country, "country_initial_exports_share_GDP");
//CONSUMPTION PARAMETERS
v[10]=VS(consumption, "sector_initial_depreciation_scale");
v[11]=VS(consumption, "sector_investment_frequency");
v[12]=VS(consumption, "sector_number_object_firms");
v[13]=VS(consumption, "sector_initial_price");
v[14]=VS(consumption, "sector_input_tech_coefficient");
v[15]=VS(consumption, "sector_initial_propensity_import_inputs");
v[16]=VS(consumption, "sector_initial_exports_share");
v[17]=VS(consumption, "sector_initial_external_price");
//CAPITAL PARAMETERS
v[20]=VS(capital, "sector_initial_depreciation_scale");
v[21]=VS(capital, "sector_investment_frequency");
v[22]=VS(capital, "sector_number_object_firms");
v[23]=VS(capital, "sector_initial_price");
v[24]=VS(capital, "sector_input_tech_coefficient");
v[25]=VS(capital, "sector_initial_propensity_import_inputs");
v[26]=VS(capital, "sector_initial_exports_share");
v[27]=VS(capital, "sector_initial_external_price");
//INPUT PARAMETERS
v[30]=VS(input, "sector_initial_depreciation_scale");
v[31]=VS(input, "sector_investment_frequency");
v[32]=VS(input, "sector_number_object_firms");
v[33]=VS(input, "sector_initial_price");
v[34]=VS(input, "sector_input_tech_coefficient");
v[35]=VS(input, "sector_initial_propensity_import_inputs");
v[36]=VS(input, "sector_initial_exports_share");
v[37]=VS(input, "sector_initial_external_price");
//EXTERNAL SECTOR PARAMETERS
v[40]=VS(external, "external_interest_rate");
v[41]=VS(external, "initial_external_income_scale");
v[42]=VS(external, "external_capital_flow_adjustment");
v[43]=VS(external, "initial_reserves_ratio");
v[44]=VS(external, "initial_exchange_rate");
//FINANCIAL PARAMETERS
v[50]=VS(financial, "cb_annual_real_interest_rate");
v[51]=VS(financial, "fs_initial_leverage");
v[52]=VS(financial, "fs_spread_deposits");
v[53]=VS(financial, "fs_spread_short_term");
v[54]=VS(financial, "fs_spread_long_term");
v[55]=VS(financial, "fs_risk_premium_short_term");
v[56]=VS(financial, "fs_risk_premium_long_term");
v[57]=VS(financial, "fs_number_object_banks");
v[58]=VS(financial, "cb_minimum_capital_ratio");
v[59]=VS(financial, "fs_sensitivity_debt_rate");
//GOVERNMENT PARAMETERS
v[60]=VS(government, "government_initial_debt_gdp_ratio");
v[61]=VS(government, "government_initial_share_consumption");
v[62]=VS(government, "government_initial_share_capital");
v[63]=VS(government, "government_initial_share_input");
//CENTRAL BANK PARAMETERS
v[70]=VS(centralbank, "cb_target_annual_inflation");
	
	if(V("switch_monetary_policy")==2)			//smithin rule
		v[71]=v[70];	
	else if(V("switch_monetary_policy")==3)		//pasinetti rule
		v[71]=v[70];
	else if(V("switch_monetary_policy")==4)		//kansas city rule.
		v[71]=0;
	else					//taylor rule or fixed monetary policy
		v[71]=v[50]+v[70];	

	v[100]=(((v[20]*v[22]/v[21])+(v[30]*v[32]/v[31])+(v[10]*v[12]/v[11]))*v[23])/v[1];				//nominal GDP
	LOG("\nNominal GDP is %f.",v[100]);
		
	//GOVERNMENT INTERMEDIATE CALCULATION
	v[101]=v[100]*v[60];								//government debt
	v[102]=pow((1+v[71]),(1/v[0]))-1;					//quarterly basic interest rate
	v[103]=v[102]*v[101];								//government interest payment
	v[104]=v[2]*v[100];									//government expenses
	v[105]=v[103]+v[104];								//government total taxes
	v[106]=v[104]*v[61];								//government nominal consumption
	v[107]=v[104]*v[62];								//government nominal investment
	v[108]=v[104]*v[63];								//government nominal inputs
	v[109]=v[106]/v[13];								//government real consumption
	v[110]=v[107]/v[23];								//government real investment
	v[111]=v[108]/v[33];								//government real inputs
	v[112]=v[104]-v[106]-v[107]-v[108];					//government wages
	v[113]=v[103]/v[100];								//government surplus rate target
	
	//WRITTING GOVERNMENT LAGGED VALUES
	WRITELLS(government, "Government_Desired_Wages", v[112], 0, 1);
	WRITELLS(government, "Government_Desired_Unemployment_Benefits", 0, 0, 1);
	WRITELLS(government, "Government_Desired_Consumption", v[106], 0, 1);
	WRITELLS(government, "Government_Desired_Investment", v[107], 0, 1);
	WRITELLS(government, "Government_Desired_Inputs", v[108], 0, 1);
	WRITELLS(government, "Government_Surplus_Rate_Target", v[113], 0, 1);
	WRITELLS(government, "Government_Debt", v[101], 0, 1);
	WRITELLS(government, "Government_Total_Taxes", v[105], 0, 1);
	WRITELLS(government, "Government_Max_Expenses_Ceiling", v[104], 0, 1);//olhar depois
	WRITELLS(government, "Government_Max_Expenses_Surplus", v[104], 0, 1);//olhar depois
	WRITELLS(government, "Government_Max_Expenses", v[104], 0, 1);//olhar depois
	for(i=1;i<=v[0]+1;i++) 
		WRITELLS(government, "Government_Debt_GDP_Ratio", v[60], 0, i);
	for(i=1;i<=v[0];i++) 
		WRITELLS(government, "Government_Effective_Expenses", v[104], 0, i);

	//EXTERNAL INTERMEDIATE CALCULATION
	v[120]=v[41]*v[100];						        //external nominal income
	v[121]=v[100]*v[42]*(v[71]-v[40]);					//capital flows
	v[122]=v[43]*v[100];								//international reserves
	v[123]=v[100]*v[3];									//country nominal exports
	v[124]=v[123]+v[121];								//country nominal imports
	v[125]=v[123]*v[16];								//country nominal consuption exports
	v[126]=v[123]*v[26];								//country nominal capital exports
	v[127]=v[123]*v[36];								//country nominal input exports
	v[128]=v[125]/v[13];								//country real consumption exports
	v[129]=v[126]/v[23];								//country real capital exports
	v[130]=v[127]/v[33];								//country real input exports

	//SECTORAL DEMAND CALCULATION
	v[140]=v[100]*(1-v[1]-v[2]-v[3])-v[103];														//nominal domestic consumption
	v[141]=(v[140]/v[13])+v[128]+v[109];															//real consumption demand
	v[142]=(v[20]*v[22]/v[21])+(v[30]*v[32]/v[31])+(v[10]*v[12]/v[11])+v[129]+v[110];				//real capital demand
	v[143]=(v[141]*v[14]*(1-v[15])+v[142]*v[24]*(1-v[25])+v[130]+v[111])/(1-v[34]*(1-v[35]));		//real input demand
	WRITES(consumption, "sector_initial_demand", v[141]);
	WRITES(capital, "sector_initial_demand", v[142]);
	WRITES(input, "sector_initial_demand", v[143]);

	v[270]=WHTAVE("sector_initial_price", "sector_initial_demand")/SUM("sector_initial_demand");	//average price
	
	//WRITTING EXTERNAL SECTOR LAGGED VALUES
	WRITELLS(external, "External_Real_Income", v[120]/v[270], 0, 1);
	WRITELLS(external, "External_Real_Income", v[120]/v[270], 0, 2);
	WRITELLS(external, "Country_Exchange_Rate", v[44], 0, 1);
	WRITELLS(external, "Country_Nominal_Exports", v[123], 0, 1);
	WRITELLS(external, "Country_Nominal_Imports", v[124], 0, 1);
	WRITELLS(external, "Country_International_Reserves", v[122], 0, 1);
	WRITELLS(external, "Country_Trade_Balance", v[123]-v[124], 0, 1);
	WRITELLS(external, "Country_Capital_Flows", v[121], 0, 1);
	WRITELLS(external, "Country_International_Reserves_GDP_Ratio", v[43], 0, 1);
	WRITELLS(external, "Country_External_Debt", 0, 0, 1);
	
v[210]=v[211]=v[212]=v[213]=v[214]=v[215]=v[216]=v[217]=v[218]=v[219]=v[226]=0;
CYCLE(cur, "SECTORS")
{
	v[150]=VS(cur, "sector_initial_demand");
	v[151]=VS(cur, "sector_investment_frequency");
	v[152]=VS(cur, "sector_number_object_firms");
	v[153]=VS(cur, "sector_initial_price");
	v[154]=VS(cur, "sector_input_tech_coefficient");
	v[155]=VS(cur, "sector_initial_propensity_import_inputs");
	v[156]=VS(cur, "sector_desired_degree_capacity_utilization");
	v[157]=VS(cur, "sector_initial_external_price");
	v[158]=VS(cur, "sector_capital_output_ratio");
	v[159]=VS(cur, "sector_initial_productivity");
	v[160]=VS(cur, "sector_indirect_tax_rate");
	v[161]=VS(cur, "sector_rnd_revenue_proportion");
	v[162]=VS(cur, "sector_initial_debt_rate");
	v[163]=VS(cur, "sector_initial_liquidity_preference");
	v[164]=VS(cur, "sector_initial_profit_rate");
	v[165]=VS(cur, "sector_capital_duration");
	v[166]=VS(cur, "sector_initial_depreciation_scale");
	v[167]=VS(cur, "sector_initial_quality");
	v[168]=VS(cur, "sector_desired_inventories_proportion");
	v[169]=VS(cur, "sector_price_frequency");
	
	v[170]=v[150]*v[153];											//sector revenue
	v[171]=v[150]*v[153]*v[160];									//sector taxation
	v[172]=v[150]*v[153]*(1-v[160])*v[161];							//sector rnd expenses
	v[173]=v[150]*v[154]*(1-v[155])*v[33];							//sector domestic input expenses
	v[174]=v[150]*v[154]*v[155]*v[37]*v[44];						//sector imported input expenses
	v[175]=v[173]+v[174];											//sector total input expenses
	v[176]=v[150]*v[164];											//sector gross profits
	
	v[177]=v[150]/v[156];											//sector desired capacity
	v[178]=v[177]/v[152];											//firm desired capacity
	v[179]=ROUND(v[178]*v[158], "UP");								//firm number capitals
	v[180]=v[179]*v[152];											//sector number capitals
	v[181]=v[180]*v[23];											//sector nominal capital
	v[182]=v[163]*v[181];											//sector stock deposits
	v[183]=v[181]*(1+v[163])*v[162];								//sector stock loans
	v[184]=v[102]+v[54]+v[56]*v[162];								//sector avg interest rate long term
	v[185]=v[184]*v[183];											//sector interest payment
	v[186]=v[182]*max(0,v[102]-v[52]);								//sector interest receivment
	v[187]=v[170]-v[171]-v[172]-v[175]-v[185]+v[186]-v[176];		//sector wage payment
	v[188]=v[187]*v[159]/v[150];									//sector wage rate
	v[189]=(v[188]/v[159])+(v[175]/v[150]);							//sector unit variable cost
	v[190]=v[153]/v[189];											//sector markup
	v[191]=v[150]/v[159];											//sector employment
	v[192]=v[183]/v[165];											//sector amortization expenses
	v[193]=v[23]*(v[166]*v[152]/v[151]);							//sector investment expenses
	v[194]=v[193]-v[192];											//sector retained profits
	v[195]=v[176]-v[194];											//sector distributed profits
	v[196]=v[195]/v[176];											//sector profit distribution rate
	v[197]=v[180]*v[158];											//sector productive capacity
	v[198]=v[150]/v[197];											//sector capacity utilization
	v[199]=(VS(cur,"sector_initial_exports_share")*v[123]/v[153])/(pow((v[157]*v[44]/v[153]),VS(cur,"sector_exports_elasticity_income"))*pow(v[120],VS(cur,"sector_exports_elasticity_income")));

	//WRITTING SECTOR LAGGED VALUES
	WRITES(cur, "sector_exports_coefficient", v[199]);
	WRITES(cur, "sector_desired_degree_capacity_utilization", v[198]);
	WRITES(cur, "sector_profits_distribution_rate", v[196]);
	WRITELLS(cur, "Sector_External_Price", v[157], 0, 1);
	WRITELLS(cur, "Sector_Productive_Capacity", v[197], 0, 1);
	WRITELLS(cur, "Sector_Capacity_Utilization", v[198], 0, 1);
	WRITELLS(cur, "Sector_Idle_Capacity", 1-v[198], 0, 1);
	WRITELLS(cur, "Sector_Productive_Capacity_Available", 0, 0, 1);
	WRITELLS(cur, "Sector_Number_Firms", v[152], 0, 1);
	WRITELLS(cur, "Sector_Avg_Productivity", v[159], 0, 1);
	WRITELLS(cur, "Sector_Max_Productivity", v[159], 0, 1);
	WRITELLS(cur, "Sector_Avg_Wage", v[188], 0, 1);
	WRITELLS(cur, "Sector_Max_Quality", v[167], 0, 1);
	WRITELLS(cur, "Sector_Propensity_Import_Inputs", v[155], 0, 1);
	WRITELLS(cur, "Sector_Exports_Share", (VS(cur,"sector_initial_exports_share")*v[123]/v[153])/v[150], 0, 1);
	for(i=1;i<=v[0]+1;i++) 
		WRITELLS(cur, "Sector_Avg_Price", v[153], 0, i);
	for(i=1;i<=v[0]+1;i++) 
		WRITELLS(cur, "Sector_Avg_Quality", v[167], 0, i);
	for(i=1;i<=v[0]+1;i++) 
		WRITELLS(cur, "Sector_Employment", v[191], 0, i);
	for(i=1;i<=v[151];i++) 
		WRITELLS(cur, "Sector_Demand_Met", 0, 0, i);
	for(i=1;i<=v[151];i++) 
		WRITELLS(cur, "Sector_Demand_Met_By_Imports", 1, 0, i);
	for(i=1;i<=v[151];i++) 
		WRITELLS(cur, "Sector_Effective_Orders", v[150], 0, i);
	
	LOG("\nSector %f.0",SEARCH_INST(cur));LOG(" Desired Capacity Uilization is %f.",v[198]);
	LOG("\nSector %f.0",SEARCH_INST(cur));LOG(" Profit Distribution Rate is %f.",v[196]);
	LOG("\nSector %f.0",SEARCH_INST(cur));LOG(" Wage Rate is %f.",v[188]);
	
	//WRITTING FIRM LAGGED VALUES
	cur1=SEARCHS(cur, "FIRMS");																	
	WRITES(cur1, "firm_date_birth", 0);   
	WRITELLS(cur1, "Firm_Effective_Market_Share", 1/v[152], 0, 1);
	WRITELLS(cur1, "Firm_Avg_Productivity", v[159], 0, 1);
	WRITELLS(cur1, "Firm_Price", v[153], 0, 1);
	WRITELLS(cur1, "Firm_Quality", v[167], 0, 1);
	WRITELLS(cur1, "Firm_Competitiveness", 1, 0, 1);
	WRITELLS(cur1, "Firm_Stock_Inventories", v[150]*v[168]/v[152], 0, 1);
	WRITELLS(cur1, "Firm_Delivery_Delay", 0, 0, 1);
	WRITELLS(cur1, "Firm_Stock_Deposits", v[182]/v[152], 0, 1);
	WRITELLS(cur1, "Firm_Wage", v[188], 0, 1);
	WRITELLS(cur1, "Firm_Desired_Markup", v[190], 0, 1);
	WRITELLS(cur1, "Firm_Avg_Debt_Rate", v[162], 0, 1);
	WRITELLS(cur1, "Firm_Max_Debt_Rate", 2*v[162], 0, 1);
	WRITELLS(cur1, "Firm_Stock_Inputs", v[150]*v[154]/v[152], 0, 1);
	WRITELLS(cur1, "Firm_Liquidity_Preference", v[163], 0, 1);
	WRITELLS(cur1, "Firm_Capital", v[181]/v[152], 0, 1);
	WRITELLS(cur1, "Firm_Stock_Loans", v[183]/v[152], 0, 1);
	for(i=1;i<=v[151];i++) 
		WRITELLS(cur1, "Firm_Demand_Capital_Goods_Expansion", 0, 0, i);
	for(i=1;i<=v[151];i++) 
		WRITELLS(cur1, "Firm_Demand_Capital_Goods_Replacement", 0, 0, i);
	for(i=1;i<=v[151];i++) 
		WRITELLS(cur1, "Firm_Frontier_Productivity", v[159], 0, i);
	for(i=1;i<=v[151];i++) 
		WRITELLS(cur1, "Firm_Productive_Capacity", v[197]/v[152], 0, i);
	for(i=1;i<=v[151];i++) 
		WRITELLS(cur1, "Firm_Interest_Payment", v[185]/v[152], 0, i);
	for(i=1;i<=v[151];i++) 
		WRITELLS(cur1, "Firm_Debt_Rate", v[162], 0, i);
	for(i=1;i<=v[151];i++) 
		WRITELLS(cur1, "Firm_Net_Profits", v[176]/v[152], 0, i);
	for(i=1;i<=v[151]-1;i++) 
		WRITELLS(cur1, "Firm_Effective_Orders_Capital_Goods", v[150]/v[152], 0, i);
	for(i=1;i<=2*v[151]-1;i++) 
		WRITELLS(cur1, "Firm_Effective_Orders", v[150]/v[152], 0, i);
	for(i=1;i<=v[169];i++) 
		WRITELLS(cur1, "Firm_Market_Share", 1/v[152], 0, i);
	for(i=1;i<=v[0]+1;i++) 
		WRITELLS(cur1, "Firm_Avg_Productivity", v[159], 0, i);
	
	//WRITTING CAPITAL LAGGED VALUES
	cur2=SEARCHS(cur1, "CAPITALS");														
	WRITELLS(cur2, "Capital_Good_Acumulated_Production", 0, 0, 1);      				
	WRITES(cur2, "capital_good_productive_capacity", 1/v[158]);     					
	WRITES(cur2, "capital_good_productivity_initial", v[159]);       		  			
	WRITES(cur2, "capital_good_to_replace", 0);
	WRITES(cur2, "capital_good_date_birth", 0);
	WRITES(cur2, "id_capital_good_number", 1);    
	
	//CREATING FIRM OBJECTS
	for(i=1; i<=(v[152]-1); i++)															
	cur4=ADDOBJ_EXLS(cur,"FIRMS", cur1, 0);
	CYCLES(cur, cur1, "FIRMS")                                                 				
		{
			v[200]=SEARCH_INSTS(cur, cur1);
			WRITES(cur1, "firm_id", v[200]);                         	
			v[201]=v[200]/(v[152]/v[57]);
			//WRITES(cur1, "firm_bank", ROUND(v[201], "UP"));
			WRITES(cur1, "firm_bank", uniform_int(1,v[57]));
			
			//WRITTING FIRM_LOANS LAGGED VALUES
			cur2=SEARCHS(cur1, "FIRM_LOANS");
			WRITES(cur2, "id_firm_loan_long_term", 1);     					
			WRITES(cur2, "id_firm_loan_short_term", 0);   
			WRITES(cur2, "firm_loan_total_amount", v[183]/v[152]);			
			WRITES(cur2, "firm_loan_interest_rate", v[184]);
			WRITES(cur2, "firm_loan_fixed_amortization", v[192]/v[152]);
			WRITES(cur2, "firm_loan_fixed_object", 0);
			
			//CREATING CAPITAL OBJECTS
			cur2=SEARCHS(cur1, "CAPITALS");   
			for(i=1; i<=(v[179]-1); i++)                        								
			{                                 			
			cur3=ADDOBJ_EXLS(cur1,"CAPITALS", cur2, 0);			                       		
			WRITES(cur3, "id_capital_good_number", (i+1));										
			}
			
			CYCLES(cur1, cur2, "CAPITALS")                                            			
				{
				v[202]=fmod(v[200]+v[151], v[151]);
				v[203]=VS(cur2, "id_capital_good_number");
				v[204]=v[202]+(v[203]-1)*v[151];			
				WRITES(cur2, "capital_good_depreciation_period", v[204]);
				}
		}

v[210]+=v[171];														//total indirect taxation
v[211]+=v[172];														//total RND expenses
v[212]+=v[174];														//total imported imput expenses
v[213]+=v[182];														//total firms stock deposits
v[214]+=v[183];														//total stock loans
v[215]+=v[185];														//total interest payment
v[216]+=v[186];														//total interest receivment
v[217]+=v[187];														//total wage payment
v[218]+=v[195];														//total distributed profits
v[219]+=v[181];														//total nominal capital
v[226]+=(v[193]-v[194]);											//total demand loans
}

	v[220]=v[214]/(v[213]+v[219]);									//average debt rate
	v[221]=v[214]/v[57];											//bank stock of debt
	v[222]=v[221]*(v[58]+v[59]*v[220]);								//bank initial accumulated profits
	v[223]=v[215]-v[216]+v[103];									//financial sector profits
	v[224]=v[214]/v[51];											//total stock deposits
	v[225]=v[224]-v[213];											//total class stock deposits

	//WRITTING FINANCIAL LAGGED VALUES
	WRITELLS(centralbank, "Central_Bank_Basic_Interest_Rate", v[102], 0, 1);
	WRITELLS(financial, "Financial_Sector_Avg_Competitiveness", 1, 0, 1);
	WRITELLS(financial, "Financial_Sector_Avg_Interest_Rate_Short_Term", v[102]+v[53], 0, 1);
	WRITELLS(financial, "Financial_Sector_Avg_Interest_Rate_Long_Term", v[102]+v[54], 0, 1);
	WRITELLS(financial, "Financial_Sector_Total_Stock_Loans_Growth", 0, 0, 1);
	WRITELLS(financial, "Financial_Sector_Total_Stock_Loans", v[214], 0, 1);
	
	//CREATING BANK OBJECTS
	cur1=SEARCHS(financial, "BANKS");
	for(i=1; i<=(v[57]-1); i++)																
	cur2=ADDOBJ_EXLS(financial,"BANKS", cur1, 0);

	//WRITTING BANK LAGGED VALUES
	CYCLES(financial, cur1, "BANKS")                                                 				
		{												
		WRITES(cur1, "bank_id", SEARCH_INSTS(root, cur1)); 
		WRITELLS(cur1, "Bank_Market_Share", 1/v[57], 0, 1);
		WRITELLS(cur1, "Bank_Default_Share", 0, 0, 1);
		WRITELLS(cur1, "Bank_Accumulated_Defaulted_Loans", 0, 0, 1);
		WRITELLS(cur1, "Bank_Total_Stock_Loans", v[214]/v[57], 0, 1);
		WRITELLS(cur1, "Bank_Competitiveness", 1, 0, 1);
		WRITELLS(cur1, "Bank_Demand_Met", 1, 0, 1);
		WRITELLS(cur1, "Bank_Demand_Loans", v[226]/v[57], 0, 1);
		WRITELLS(cur1, "Bank_Desired_Short_Term_Spread", v[53], 0, 1);
		WRITELLS(cur1, "Bank_Desired_Long_Term_Spread", v[54], 0, 1);
		WRITELLS(cur1, "Bank_Interest_Rate_Short_Term", v[102]+v[53], 0, 1);
		WRITELLS(cur1, "Bank_Interest_Rate_Long_Term", v[102]+v[54], 0, 1);
		WRITELLS(cur1, "Bank_Accumulated_Profits", v[222], 0, 1);
		}
		
	//AGGREGATE INTERMEDIATE VARIABLES
	v[230]=v[211]+v[217]+v[112];									//total wages
	v[231]=v[218]+v[223];											//total distributed profits
	v[232]=v[230]+v[231];											//total households gross income	
	v[233]=v[105]-v[210];											//total income taxation
	v[235]=v[124]-v[212];											//total imported consumption expenses
	
	if(V("switch_class_tax_structure")==0)							    	//taxation structure = no tax
		v[280]=0;
	if(V("switch_class_tax_structure")==1)									//taxation structure = only wages
		v[280]=WHTAVE("class_direct_tax", "class_wage_share")*v[230];
	if(V("switch_class_tax_structure")==2)									//taxation structure = only profits
		v[280]=WHTAVE("class_direct_tax", "class_profit_share")*v[231];
	if(V("switch_class_tax_structure")==3)									//taxation structure = profits and wages 
		v[280]=WHTAVE("class_direct_tax", "class_profit_share")*v[231]
		      +WHTAVE("class_direct_tax", "class_wage_share")*v[230];
	if(V("switch_class_tax_structure")==4)									//taxation structure = profits, wages and interest
		v[280]=WHTAVE("class_direct_tax", "class_profit_share")*v[231]
		      +WHTAVE("class_direct_tax", "class_wage_share")*v[230]
			  +WHTAVE("class_direct_tax", "class_profit_share")*max(0,v[102]-v[52])*v[225];
	LOG("\nPseudo Taxation %f.0",v[280]);
	LOG("\nTaxation %f.0",v[233]);
	v[281]=v[233]/v[280];
		
	//WRITTING CLASS LAGGED VALUES  
	v[251]=v[252]=0;
	CYCLE(cur, "CLASSES")
	{
		v[240]=VS(cur, "class_propensity_to_spend");
		v[241]=VS(cur, "class_profit_share");
		v[242]=VS(cur, "class_wage_share");
		v[254]=VS(cur, "class_initial_max_debt_rate");
		v[255]=VS(cur, "class_initial_liquidity_preference");
	
		v[243]=v[230]*v[242]+v[231]*v[241];
		
		if(V("switch_class_tax_structure")==0)							    	//taxation structure = no tax
		{
			v[234]=0;
			v[244]=0;															//average direct tax rate							   				   				//class total tax
		}
		if(V("switch_class_tax_structure")==1)									//taxation structure = only wages
		{
			v[234]=VS(cur,"class_direct_tax")*v[281];
			v[244]=v[234]*(v[230]*v[242]);										//average direct tax rate
		}
		if(V("switch_class_tax_structure")==2)									//taxation structure = only profits
		{
			v[234]=VS(cur,"class_direct_tax")*v[281];
			v[244]=v[234]*(v[231]*v[241]);										//average direct tax rate
		}
		if(V("switch_class_tax_structure")==3)									//taxation structure = profits and wages 
		{
			v[234]=VS(cur,"class_direct_tax")*v[281];
			v[244]=v[234]*(v[230]*v[242]+v[231]*v[241]);						//average direct tax rate
		}
		if(V("switch_class_tax_structure")==4)									//taxation structure = profits, wages and interest
		{
			v[234]=VS(cur,"class_direct_tax")*v[281];
			v[244]=v[234]*(v[230]*v[242]+v[231]*v[241]+max(0,v[102]-v[52])*v[225]*v[241]);												//average direct tax rate
		}
		LOG("\nClass %f.0",SEARCH_INST(cur));LOG(" Tax Rate is %f.",v[234]);

		v[245]=v[243]-v[244];										//class disposable income
		v[246]=v[245]*v[240];										//class induced expenses
		v[247]=v[235]*v[241];										//class nominal imports
		v[248]=v[247]/v[246];										//class import share
		v[249]=v[246]-v[247];										//class induced domestic consumption
		v[250]=v[245]-v[246];										//class induced savings
		v[251]+=v[249];												//total induced domestic consumption
		v[252]+=v[250];												//total induced savings
		
		WRITES(cur, "class_direct_tax", v[234]);//same tax rate 
		WRITES(cur, "class_initial_propensity_import", v[248]);
		WRITELLS(cur, "Class_Stock_Deposits", v[225]*v[241], 0, 1);
		WRITELLS(cur, "Class_Liquidity_Preference", v[255], 0, 1);//olhar depois
		WRITELLS(cur, "Class_Max_Debt_Rate", v[254], 0, 1);//olhar depois
		WRITELLS(cur, "Class_Stock_Loans", 0, 0, 1);
		WRITELLS(cur, "Class_Avg_Nominal_Income", v[245], 0, 1);
		for(i=1;i<=v[0]+1;i++)
			WRITELLS(cur, "Class_Nominal_Disposable_Income", v[245], 0, 1);
		for(i=1;i<=v[0]+1;i++)
			WRITELLS(cur, "Class_Real_Disposable_Income", v[245]/v[13], 0, 1);
		for(i=1;i<=v[0]+1;i++)
			WRITELLS(cur, "Class_Debt_Rate", 0, 0, 1);
	}
	
	v[253]=v[140]-v[251];//total autonomous consumption
	CYCLE(cur, "CLASSES")
		WRITELLS(cur, "Class_Real_Autonomous_Consumption", v[253]*VS(cur, "class_profit_share")/v[13], 0, 1);

v[271]=WHTAVE("sector_initial_productivity", "sector_initial_demand")/SUM("sector_initial_demand");
v[272]=WHTAVE("sector_desired_degree_capacity_utilization", "sector_initial_demand")/SUM("sector_initial_demand");
	
//WRITTING COUNTRY LAGGED VALUES  
WRITELLS(country, "Country_Debt_Rate_Firms", v[220], 0, 1);
WRITELLS(country, "Country_Idle_Capacity", 1-v[272], 0, 1);
WRITELLS(country, "Country_Avg_Productivity", v[271], 0, 1);
WRITELLS(country, "Country_Avg_Productivity", v[271], 0, 2);
WRITELLS(country, "Country_Annual_CPI_Inflation", v[70], 0, 1);
for(i=1;i<=v[0]+1;i++)
	WRITELLS(country, "Country_Price_Index", v[270], 0, i);
for(i=1;i<=v[0]+1;i++)
	WRITELLS(country, "Country_Consumer_Price_Index", v[13], 0, i);
for(i=1;i<=2*v[0]+1;i++)
	WRITELLS(country, "Country_GDP", v[100], 0, i);
for(i=1;i<=2*v[0]+1;i++)
	WRITELLS(country, "Country_Real_GDP", v[100]/v[270], 0, i);
for(i=1;i<=v[0];i++)
	WRITELLS(country, "Country_Capital_Goods_Price", v[23], 0, i);
	
PARAMETER
RESULT(0)