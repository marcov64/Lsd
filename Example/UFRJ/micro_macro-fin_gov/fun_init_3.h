//#define EIGENLIB			// uncomment to use Eigen linear algebra library
//#define NO_POINTER_INIT	// uncomment to disable pointer checking


EQUATION("Initialization_3")

consumption=SEARCH_CND("id_consumption_goods_sector",1);
capital=SEARCH_CND("id_capital_goods_sector",1);
input=SEARCH_CND("id_intermediate_goods_sector",1);
government=SEARCH("GOVERNMENT");
financial=SEARCH("FINANCIAL");
external=SEARCH("EXTERNAL_SECTOR");
country=SEARCH("COUNTRY");
centralbank=SEARCH("CENTRAL_BANK");

//v[0]=V("depreciation_scale");
v[0]=1;

v[1]=VS(consumption, "sector_number_object_firms");
v[2]=VS(capital, "sector_number_object_firms");
v[3]=VS(input, "sector_number_object_firms");

v[4]=VS(consumption, "sector_investment_frequency");
v[5]=VS(capital, "sector_investment_frequency");
v[6]=VS(input, "sector_investment_frequency");

//v[7]=VS(country, "investment_share_GDP");
//v[8]=VS(country, "country_initial_exports_share_GDP");
//v[9]=VS(country, "country_initial_government_share_GDP");
v[7]=0.1;
v[8]=0.1;
v[9]=0.2;

v[10]=VS(consumption, "sector_initial_price");
v[11]=VS(capital, "sector_initial_price");
v[12]=VS(input, "sector_initial_price");

v[13]=VS(consumption, "sector_exports_share");
v[14]=VS(capital, "sector_exports_share");
v[15]=VS(input, "sector_exports_share");

v[16]=VS(government, "government_initial_consumption_share");
v[17]=VS(government, "government_initial_capital_share");
v[18]=VS(government, "government_initial_input_share");

v[19]=v[0]*((v[1]/v[4])+(v[2]/v[5])+(v[3]/v[6]));//domestic real capital demand
	LOG("\nDomestic Capital Demand is %f.",v[19]);
v[20]=(v[11]*v[19])/v[7];//nominal quarterly GDP
	LOG("\nNominal Quarterly GDP is %f.",v[20]);

v[21]=VS(government, "initial_government_debt_share_GDP");
v[22]=VS(country, "annual_frequency");
v[23]=v[20]*v[21]*v[22];//government debt
	LOG("\nGovernment Debt is %f.",v[23]);
v[24]=VS(financial, "cb_annual_real_interest_rate")+VS(centralbank, "cb_target_annual_inflation");
v[25]=v[24]*v[23];//government interest payment
	LOG("\nGovernment Interest Payment is %f.",v[25]);
v[26]=v[9]*v[20];//government total expenses
	LOG("\nGovernment Total Expenses is %f.",v[26]);
v[27]=v[25]+v[26];//government total taxes
	LOG("\nGovernment Total Taxes is %f.",v[27]);
v[28]=v[16]*v[26];//government consumption expenses
	LOG("\nGovernment Nominal Consumption is %f.",v[28]);
v[29]=v[17]*v[26];//government capital expenses
	LOG("\nGovernment Nominal Investment is %f.",v[29]);
v[30]=v[18]*v[26];//government input expenses
	LOG("\nGovernment Nominal Inputs is %f.",v[30]);
v[31]=v[28]/v[10];//government consumption real demand
	LOG("\nGovernment Real Consumption is %f.",v[31]);
v[32]=v[29]/v[11];//government capital real demand
	LOG("\nGovernment Real Capital is %f.",v[32]);
v[33]=v[30]/v[12];//government input real demand
	LOG("\nGovernment Real Inputs is %f.",v[33]);
v[34]=v[26]*(1-v[16]-v[17]-v[18]);//government wages
	LOG("\nGovernment Wages is %f.",v[34]);
		
WRITELLS(government,"Government_Total_Taxes", v[27], 0, 1);														//write initial total taxes, initial total taxes is calculated in the demand calibration based only on parameters
WRITELLS(government,"Government_Max_Expenses", v[26], 0, 1);        									//initial max government expenses equals total taxes calculated in the calibration
WRITELLS(government,"Government_Desired_Wages", v[34], 0, 0);		            		//initial government expenses is only wages, which thereafter will grow depending on inflation and average productivity		            				    
WRITELLS(government,"Government_Desired_Consumption", v[28], 0, 0);		            		//initial government expenses is only wages, which thereafter will grow depending on inflation and average productivity	
WRITELLS(government,"Government_Desired_Investment", v[29], 0, 0);		            			//initial government expenses is only wages, which thereafter will grow depending on inflation and average productivity	
WRITELLS(government,"Government_Desired_Inputs", v[30], 0, 0);		            			    //initial government expenses is only wages, which thereafter will grow depending on inflation and average productivity	
WRITELLS(government,"Government_Surplus_Rate_Target", v[169], 0, 1);
for (i=1 ; i<=V("annual_frequency")+1 ; i++)		              													//for (government_period) lags	
{
	WRITELLS(government,"Government_Debt", V("initial_debt_gdp")*V("annual_frequency")*v[150], 0, i);                  									//no debt initially																	//base interest rate parameter
	WRITELLS(government,"government_initial_debt_gdp_ratio", V("initial_debt_gdp"), 0, i);
	WRITELLS(government,"Government_Effective_Expenses", v[25]/v[20], 0, i);
}
	
v[35]=v[20]*v[8];//total exports
	LOG("\nTotal Exports is %f.",v[35]);
//v[36]=VS(external,"initial_trade_balance_share_GDP");
v[36]=0;
v[37]=v[35]-v[36]*v[20];//total imports
	LOG("\nTotal Imports is %f.",v[35]);
v[38]=v[35]*v[13];//nominal consumption exports
	LOG("\nNominal Consumption Exports is %f.",v[38]);
v[39]=v[35]*v[14];//nominal capital exports
	LOG("\nNominal Capital Exports is %f.",v[39]);
v[40]=v[35]*v[15];//nominal input exports
	LOG("\nNominal Input Exports is %f.",v[40]);

v[41]=v[38]/v[10];//real consumption exports
	LOG("\nReal Consumption Exports is %f.",v[41]);
v[42]=v[39]/v[11];//real capital exports
	LOG("\nReal Capital Exports is %f.",v[42]);
v[43]=v[40]/v[12];//real input exports
	LOG("\nReal Input Exports is %f.",v[43]);
	
v[44]=v[42]+v[32]+v[19];//total capital demand
	LOG("\nReal Total Capital Demand is %f.",v[44]);
	WRITES(capital, "sector_initial_demand", v[44]);
v[45]=VS(capital, "sector_desired_degree_capacity_utilization");
v[46]=v[44]/v[2];//capital sector firm demand
	LOG("\nCapital Sector Firm Demand is %f.",v[46]);
v[47]=v[46]/v[45];//capital sector firm productive capacity
	LOG("\nCapital Sector Firm Capacity is %f.",v[47]);
v[48]=VS(capital, "sector_capital_output_ratio");
v[49]=ROUND(v[48]*v[47], "UP");
	LOG("\nCapital Sector Firm Number Capitals is %f.",v[49]);
v[50]=v[46]/v[49];//effective capacity utilization
	LOG("\nCapital Sector Effective Capaicty Utilization is %f.",v[50]);

v[51]=v[20]*(1-v[7]-v[8]-v[9])-v[25];//domestic consumption
	LOG("\nNominal Domestic Consumtpion is %f.",v[51]);
v[52]=v[51]+v[37];//total classes expenses
	LOG("\nTotal Classes Expenses is %f.",v[52]);
	
v[53]=v[51]/v[10];//domestic real consumption
	LOG("\nReal Domestic Consumtpion is %f.",v[53]);
v[54]=v[53]+v[31]+v[41];//consumption sector real demand
	LOG("\nReal Total Consumption Demand is %f.",v[54]);
	WRITES(consumption, "sector_initial_demand", v[54]);
v[55]=VS(consumption, "sector_desired_degree_capacity_utilization");
v[56]=v[54]/v[1];//consumption sector firm demand
	LOG("\nConsumption Sector Firm Demand is %f.",v[56]);
v[57]=v[56]/v[55];//consumption sector firm productive capacity
	LOG("\nConsumpion Sector Firm Capacity is %f.",v[57]);
v[58]=VS(consumption, "sector_capital_output_ratio");
v[59]=ROUND(v[58]*v[57], "UP");
	LOG("\nConsumption Sector Firm Number Capitals is %f.",v[59]);
v[60]=v[56]/v[59];//effective capacity utilization
	LOG("\nConsumption Sector Effective Capaicty Utilization is %f.",v[60]);
	
v[61]=VS(consumption,"sector_input_tech_coefficient");
v[62]=VS(capital,"sector_input_tech_coefficient");
v[63]=VS(input,"sector_input_tech_coefficient");	

v[64]=(v[54]*v[61] + v[44]*v[62] + v[33] + v[43])/(1-v[63]);
	LOG("\nReal Total Input Demand is %f.",v[64]);
	WRITES(input, "sector_initial_demand", v[64]);
v[65]=VS(input, "sector_desired_degree_capacity_utilization");
v[66]=v[64]/v[3];//input sector firm demand
	LOG("\nInput Sector Firm Demand is %f.",v[66]);
v[67]=v[66]/v[65];//input sector firm productive capacity
	LOG("\nInput Sector Firm Capacity is %f.",v[67]);
v[68]=VS(input, "sector_capital_output_ratio");
v[69]=ROUND(v[68]*v[67], "UP");
	LOG("\nInput Sector Firm Number Capitals is %f.",v[69]);
v[70]=v[66]/v[69];//effective capacity utilization
	LOG("\nInput Sector Effective Capaicty Utilization is %f.",v[70]);
	
v[71]=VS(financial, "fs_spread_long_term");
v[72]=VS(financial, "fs_risk_premium_long_term");
v[73]=VS(financial, "fs_spread_deposits");

v[124]=v[125]=v[126]=v[127]=v[128]=v[129]=v[135]=v[139]=v[140]=v[142]=v[144]=0;
CYCLE(cur, "SECTORS")
{
	v[100]=VS(cur, "sector_initial_demand");
	v[101]=VS(cur, "sector_initial_debt_ratio");
	v[102]=VS(cur, "sector_initial_liquidity_preference");
	v[103]=VS(cur, "sector_capital_output_ratio");
	v[104]=VS(cur, "sector_desired_degree_capacity_utilization");
	v[105]=VS(cur, "sector_initial_price");
	v[106]=VS(cur, "sector_initial_productivity");
	v[107]=VS(cur, "sector_indirect_tax_rate");
	v[108]=VS(cur, "sector_rnd_revenue_proportion");
	v[109]=VS(cur, "sector_input_tech_coefficient");
	v[110]=VS(cur, "sector_profit_rate");
	v[111]=VS(cur, "sector_number_object_firms");
	
	v[112]=ROUND((((v[100]/v[111])/v[104])*v[103]),"UP")*v[111]*v[11];
		LOG("\nSector %.0f ", SEARCH_INST(cur)); LOG("Capital is %f.",v[112]);
	v[113]=v[112]*v[102];
		LOG("\nSector %.0f ", SEARCH_INST(cur)); LOG("Deposits is %f.",v[113]);
	v[114]=v[112]*(1+v[102])*v[101];
		LOG("\nSector %.0f ", SEARCH_INST(cur)); LOG("Loans is %f.",v[114]);
	v[115]=(v[24]-v[73])*v[113];
		LOG("\nSector %.0f ", SEARCH_INST(cur)); LOG("Interest on Deposits is %f.",v[115]);
	v[116]=(v[24]+v[71]+v[72]*v[101])*v[114];
		LOG("\nSector %.0f ", SEARCH_INST(cur)); LOG("Interest Payment is %f.",v[116]);
	v[117]=v[100]*v[105];
		LOG("\nSector %.0f ", SEARCH_INST(cur)); LOG("Revenue is %f.",v[117]);
	v[118]=v[100]*v[105]*v[107];
		LOG("\nSector %.0f ", SEARCH_INST(cur)); LOG("Taxation is %f.",v[118]);
	v[119]=v[100]*v[105]*(1-v[107])*v[108];
		LOG("\nSector %.0f ", SEARCH_INST(cur)); LOG("RND Expenses is %f.",v[119]);
	v[120]=v[100]*v[109]*v[12];
		LOG("\nSector %.0f ", SEARCH_INST(cur)); LOG("Input Expenses is %f.",v[120]);
	v[121]=v[100]*v[110];
		LOG("\nSector %.0f ", SEARCH_INST(cur)); LOG("Gross Profits is %f.",v[121]);
	v[122]=v[117]-v[118]-v[119]-v[120]-v[116]+v[115]-v[121];
		LOG("\nSector %.0f ", SEARCH_INST(cur)); LOG("Wage Payment is %f.",v[122]);
	v[123]=(v[122]/v[100])*v[106];
		LOG("\nSector %.0f ", SEARCH_INST(cur)); LOG("Nominal Wage Rate is %f.",v[123]);
		
	v[124]=v[124]+v[115];//total interest receivment
	v[125]=v[125]+v[116];//total interest payment
	v[126]=v[126]+v[119]+v[122];//total wage payment including RND
	v[127]=v[127]+v[120];//total input expenses
	v[128]=v[128]+v[121];//total profits
	v[129]=v[129]+v[118];//total indirect taxation
	
	v[130]=VS(cur, "sector_investment_frequency");
	v[131]=VS(cur, "sector_amortization_period");
	v[132]=v[0]*v[111]/v[130];
		LOG("\nSector %.0f ", SEARCH_INST(cur)); LOG("Capital Demand is %f.",v[132]);
	v[133]=v[132]*v[11];
		LOG("\nSector %.0f ", SEARCH_INST(cur)); LOG("Investment Expenses is %f.",v[133]);
	v[134]=v[114]/v[131];
		LOG("\nSector %.0f ", SEARCH_INST(cur)); LOG("Amortization Expenses is %f.",v[134]);
	v[135]=v[135]+v[133];
	v[136]=v[133]-v[134];
		LOG("\nSector %.0f ", SEARCH_INST(cur)); LOG("Retained Profits is %f.",v[136]);
	v[137]=v[121]-v[136];
		LOG("\nSector %.0f ", SEARCH_INST(cur)); LOG("Distributed Profits is %f.",v[137]);
	v[138]=v[137]/v[121];
		LOG("\nSector %.0f ", SEARCH_INST(cur)); LOG("Profit Distribution Rate is %f.",v[138]);
	v[139]=v[139]+v[137];
	v[140]=v[140]+v[136];
	v[141]=v[100]/v[106];
		LOG("\nSector %.0f ", SEARCH_INST(cur)); LOG("Employment is %f.",v[141]);
	v[142]=v[142]+v[141];
	v[143]=v[141]/v[111];
		LOG("\nSector %.0f ", SEARCH_INST(cur)); LOG("Firm Employment is %f.",v[143]);
	v[144]=v[144]+v[123]*v[141];
	
	for (i=1 ; i<=VS(cur, "sector_investment_frequency") ; i++)																	//for (investment_period) lags 
		{
		WRITELLS(cur, "Sector_Demand_Met", 1, 0, i); 										    //it is assumed that all demand is met initially. Equals 1 by definition
		WRITELLS(cur, "Sector_Demand_Met_By_Imports", 1, 0, i);                      			//it is assumed thatt all imports are met initially. Equals 1 by definition
		WRITELLS(cur, "Sector_Effective_Orders", v[100], 0, i);               					//Effective_Orders_Sectors equals demand_initial
		}		
	for (i=1 ; i<=(V("annual_frequency")+1) ; i++)                        		 								//for (class_period+1) lags
		{
		WRITELLS(cur, "Sector_Avg_Quality", 1, 0, i);               							//Effective_Orders_Sectors equals demand_initial
		WRITELLS(cur, "Sector_Employment", v[141], 0, i);               					//Effective_Orders_Sectors equals demand_initial
		WRITELLS(cur, "Sector_Avg_Price", v[105], 0, i);                                   		//Avg_Price equals avg_price initial
		}
		WRITELLS(cur, "Sector_Productive_Capacity_Available", 0, 0, 1);                  		//it is assumed that there is no entry or exit initially. Equals 0 by definition
		WRITELLS(cur, "Sector_Avg_Competitiveness", 1, 0, 1);                     				//if all firms are the same, equals 1 by definition
		WRITELLS(cur, "Sector_External_Price", v[105], 0, 1);                               	//Foreign_Price equals foreign_price initial
		WRITELLS(cur, "Sector_Avg_Productivity", v[106], 0,  1);               	 				//If all firms are the same, Avg Productivity will be the initial productivivity for all firms
		WRITELLS(cur, "Sector_Max_Productivity", v[106], 0,  1);                      			//If all capital goods have the same productivity, Max_Productivity equals productivity_initial 
		WRITELLS(cur, "Sector_Max_Quality", 1, 0, 1);
		WRITELLS(cur, "Sector_Inventories", (v[100]*VS(cur,"sector_desired_inventories_proportion"), 0, 1);                  			//Firms operate with desired level of inventories, thus, Current stock of inventories is the desired level times effective production
		WRITELLS(cur, "Sector_Productive_Capacity", v[100]/v[104], 0, 1);								//All firms start operating at desired degree of utilization, thus, productive capacity is endogenous calculated based on effective production and desired degree
		WRITELLS(cur, "Sector_Real_Exports", vs(cur, "sector_exports_share")*v[35]/v[105], 0, 1);										//Total exports are divided equally among sectors.
		WRITES(cur, "sector_exports_coefficient", vs(cur, "sector_exports_share")*v[35]/pow(v[20],VS(cur, "sector_exports_elasticity_income")));										//write the exports coefficient, assuming external price and foreign price starts as 1, so the exchange rate
	
		cur1=SEARCHS(cur, "FIRMS");																	//search the first and only instance of firms below each sector
		//Begin Writting Firm Parameters
		WRITES(cur1, "firm_date_birth", 0);                                   					//zero by definition
		
		//Begin Writting Independent Firm Variables
	for (i=1 ; i<=VS(cur, "sector_investment_frequency") ; i++)                                									//for (investment_period) lags
	  	{
	  	WRITELLS(cur1, "Firm_Demand_Productive_Capacity_Replacement", 0, 0, i);					//no replacement initially
	  	WRITELLS(cur1, "Firm_Debt_Rate", 0, 0, i);												//no debt initially
	  	WRITELLS(cur1, "Firm_Demand_Capital_Goods", 0, 0, i);
	  	WRITELLS(cur1, "Firm_Frontier_Productivity", v[106], 0, i);                 			//frontier productivity will be the initial frontier productivity
	  	WRITELLS(cur1, "Firm_Max_Productivity", v[106], 0, i);        							//max capital goods productivity will be the initial frontier productivity that will be the same for all capital goods
	  	WRITELLS(cur1, "Firm_Avg_Productivity", v[106], 0, i);									//firm's avg productivity will be the initial frontier productivity since all capital goods have the same productivity
	for(i=1; i<=(VS(cur, "sector_investment_frequency")+1); i++)										 							//for (investment period+1) lags (7)
		{
		WRITELLS(cur1, "Firm_Productive_Capacity_Depreciation", 0, 0, i);  						//write 0 
		WRITELLS(cur1, "Firm_Demand_Productive_Capacity_Expansion", 0, 0, i);     				//write 0 
		WRITELLS(cur1, "Firm_Demand_Capital_Goods_Expansion", 0, 0, i);     					//write 0
		WRITELLS(cur1, "Firm_Demand_Capital_Goods_Replacement", 0, 0, i);     					//write 0
		WRITELLS(cur1, "Firm_Effective_Orders_Capital_Goods", 0, 0, i);  						//write 0 
		}
	for (i=1 ; i<=(2*VS(cur, "sector_investment_frequency")) ; i++)																//for (2*investment period+1) lags
	  	WRITELLS(cur1, "Firm_Effective_Orders", (v[100]/v[111]), 0, i);                    		//firm's effective orders will be sector's effective orders (given by demand_initial) divided by the number of firms
	for (i=1 ; i<=(VS(cur, "sector_price_frequency")+1) ; i++)																//for (markup_period-1) lags
		{
		WRITELLS(cur1, "Firm_Market_Share", (1/v[111]), 0, i);             						//firm's market share will be the inverse of the number of firms in the sector (initial market share)
	  	WRITELLS(cur1, "Firm_Potential_Markup", v[206], 0, i);                      			//potential markup will be the initial markup
		}
		WRITELLS(cur1, "Firm_Effective_Market_Share", (1/v[212]), 0, 1);                    	//firm's effective market share will be the initial market share       
	  	WRITELLS(cur1, "Firm_Desired_Market_Share", (1/v[212]), 0, 1);                  		//firm's desired market share will be twice the initial market share  
	  	WRITELLS(cur1, "Firm_Avg_Market_Share", (1/v[212]), 0, 1);                     			//firm's avg market share will be the initial market share
	  	WRITELLS(cur1, "Firm_Price", v[204], 0, 1);                                      		//firm's price will be the initial price of the sector, the same for all firms
	  	WRITELLS(cur1, "Firm_Avg_Potential_Markup", v[206], 0, 1);								//avg potential markup will be the initial markup
	  	WRITELLS(cur1, "Firm_Desired_Markup", v[206], 0, 1); 									//desired markup will be the initial markup
	  	WRITELLS(cur1, "Firm_Sales", (v[200]/v[212]), 0, 1);									//firm's sales will be equal to effective orders, no delivery delay
	  	WRITELLS(cur1, "Firm_Revenue", (v[204]*v[200]/v[212]), 0, 1);                           //firm's revenue will be the firm's sales times firm price
	  	WRITELLS(cur1, "Firm_Stock_Inventories", ((v[200]/v[212])*v[217]), 0, 1);               //firm's inventories will be the sales times the desired inventories proportion (parameter)
	  	WRITELLS(cur1, "Firm_Stock_Inputs", v[223]/v[212], 0, 1);                    			//firm's stock of imputs will be the sales times the input tech relationship
	  	WRITELLS(cur1, "Firm_Productive_Capacity", v[222]/v[212], 0, 1);						//firm productive capacity will be the sales divided by the desired degree of capacity utlization (parameter)
	  	WRITELLS(cur1, "Firm_Capital", (v[219]*v[102]), 0, 1);									//firm nominal capital equals number of capital if capital goods price equals 1
	  	WRITELLS(cur1, "Firm_Wage", v[209], 0, 1); 												//firm's nominal wage equals sector nominal wage initial
		WRITELLS(cur1, "Firm_Variable_Cost", ((v[209]/v[205])+v[210]*v[100]), 0, 1);			//firm variable cost equals unitary wage plus unitary cost of inputs. The last equals the tech coefficient if input price equals 1
		WRITELLS(cur1, "Firm_Competitiveness", 1, 0, 1);                           				//if all firms are the same
	  	WRITELLS(cur1, "Firm_Delivery_Delay", 1, 0, 1);                           		  		//it is assumed that all demand is met initially, so equals 1 by definition
		WRITELLS(cur1, "Firm_Stock_Deposits", 0, 0, 1);											//no financial assets initially
	  	WRITELLS(cur1, "Firm_Stock_Loans", v[159]*(v[219]*v[102]), 0, 1);                       //no debt initially
	  	WRITELLS(cur1, "Firm_Avg_Debt_Rate", v[159], 0, 1);                       				//no debt initially
	  	WRITELLS(cur1, "Firm_Max_Debt_Rate", v[152], 0, 1);                       			//no debt initially
	  	WRITELLS(cur1, "Firm_Liquidity_Preference", v[153], 0, 1);                       		//no debt initially
	
	
	
	
	
	
	
	
	
}
	LOG("\nTotal Interest Receivment is %f.",v[124]);
	LOG("\nTotal Interest Payment is %f.",v[125]);
	LOG("\nTotal Wages is %f.",v[126]+v[34]);
	LOG("\nTotal Input Expenses is %f.",v[127]+v[30]+v[40]);
	LOG("\nTotal Productive Profits is %f.",v[128]);
	LOG("\nTotal Indirect Taxation is %f.",v[129]);
	LOG("\nFinancial Sector Profits is %f.",v[125]-v[124]+v[25]);
	LOG("\nNominal GDP Income is %f.",v[126]+v[34]+v[128]+v[125]-v[124]+v[25]);
	LOG("\nTotal Investment Expenses is %f.",v[135]+v[29]+v[39]);
	LOG("\nTotal Distributed Profits is %f.",v[139]);
	LOG("\nTotal Retained Profits is %f.",v[140]);
	LOG("\nTotal Sectors Employment is %f.",v[142]);
	LOG("\nAvg Wage Rate is %f.",v[144]/v[142]);
	LOG("\nGovernment Employment is %f.",v[34]/(v[144]/v[142]));
	LOG("\nTotal Labor Force is %f.",v[142]+v[34]/(v[144]/v[142]));
	
//v[160]=VS(country, "unemployment_rate");
v[160]-0;
v[161]=(v[142]+v[34]/(v[144]/v[142]))/(1-v[160]);
	LOG("\nTotal Population is %f.", ROUND(v[161], "UP"));
v[162]=VS(financial, "fs_number_object_banks");
v[163]=SUM("sector_number_object_firms");
v[164]=v[162]+v[163]+ROUND(v[142]+v[34]/(v[144]/v[142]),"UP");

v[150]=v[139]+v[125]-v[124]+v[25]+v[126]+v[34];
	LOG("\nHouseholds Gross Income is %f.",v[150]);
v[151]=v[27]-v[129];
	LOG("\nTotal Income Tax is %f.",v[151]);
v[152]=v[150]-v[151];
	LOG("\nHouseholds Disposable Income is %f.",v[152]);
v[153]=v[52]/v[152];
	LOG("\nHouseholds Propensity to Spend is %f.",v[153]);
	LOG("\nHouseholds Domestic Consumption is %f.",v[51]);
	LOG("\nHouseholds Inported Consumption is %f.",v[37]);
	LOG("\nHouseholds Total Consumption is %f.",v[51]+v[37]);
v[154]=v[151]/v[150];
	LOG("\nHouseholds Tax Rate is %f.",v[154]);
v[155]=v[37]/(v[51]+v[37]);
	LOG("\nHouseholds Imports Rate is %f.",v[155]);
	LOG("\nTotal Consumpion Expenses is %f.",v[51]+v[28]+v[38]);
	
cur1=SEARCH_CND("class_id", 1);
cur2=SEARCH_CND("class_id", 2);
cur3=SEARCH_CND("class_id", 3);
	
CYCLE(cur, "CLASSES")
{
	if(VS(cur,"class_id")==1)
	{
		v[170]=v[125]-v[124]+v[25];
		v[171]=v[162];
	}
	if(VS(cur,"class_id")==2)
	{
		v[170]=v[139];
		v[171]=v[163];
	}
	if(VS(cur,"class_id")==3)
	{
		v[170]=v[126]+v[34];
		v[171]=ROUND(v[142]+v[34]/(v[144]/v[142]),"UP");
	}
	v[172]=v[171]/v[164];
	v[173]=v[170]/v[150];
		LOG("\nClass %.0f ",VS(cur,"class_id")); LOG("Income is %f.",v[170]);
		LOG("\nClass %.0f ",VS(cur,"class_id")); LOG("Population is %f.",v[171]);
		LOG("\nClass %.0f ",VS(cur,"class_id")); LOG("Income Share is %f.",v[173]);
		LOG("\nClass %.0f ",VS(cur,"class_id")); LOG("Population Share is %f.",v[172]);
		WRITES(cur, "class_population_share", v[172]);
		WRITES(cur, "class_income_share", v[173]);
	v[174]=v[154]*v[170];
		LOG("\nClass %.0f ",VS(cur,"class_id")); LOG("Taxation is %f.",v[174]);
	v[175]=(1-v[154])*v[170];
		LOG("\nClass %.0f ",VS(cur,"class_id")); LOG("Disposable Income is %f.",v[175]);
		WRITES(cur, "class_disposable_income", v[175]);
}		
	v[176]=VS(cur3, "class_disposable_income");
	v[177]=0;
		
	v[178]=(1-v[155])*v[153]*VS(cur2, "class_disposable_income");
	v[179]=(v[155])*v[153]*VS(cur2, "class_disposable_income");
		
	v[180]=v[51]-v[176]-v[178];
	v[181]=v[37]-v[177]-v[179];

CYCLE(cur, "CLASSES")
{
	if(VS(cur,"class_id")==3)
	{
		v[182]=v[176];
		v[183]=0;
	}
	if(VS(cur,"class_id")==2)
	{
		v[182]=v[178];
		v[183]=v[179];
	}
	if(VS(cur,"class_id")==1)
	{
		v[182]=v[180];
		v[183]=v[181];
	}
		LOG("\nClass %.0f ",VS(cur,"class_id")); LOG("Domestic Consumption is %f.",v[182]);
		LOG("\nClass %.0f ",VS(cur,"class_id")); LOG("Imported Consumption is %f.",v[183]);
	v[184]=v[183]/(v[182]+v[183]);
		LOG("\nClass %.0f ",VS(cur,"class_id")); LOG("Imported Share is %f.",v[184]);
	v[185]=(v[182]+v[183])/VS(cur, "class_disposable_income");
		LOG("\nClass %f ",VS(cur,"class_id")); LOG("Propensity to Spend is %f.",v[185]);


	for (i=1 ; i<=V("annual_frequency") ; i++)                          										//for (class_period) lags
			{
			WRITELLS(cur, "Class_Nominal_Disposable_Income", VS(cur, "class_disposable_income"), 0, i);            					//writes Class_Nominal_Income
			WRITELLS(cur, "Class_Real_Disposable_Income", VS(cur, "class_disposable_income")/v[10], 0, i);							//writes Class_Real_Income
			}
			WRITELLS(cur, "Class_Avg_Nominal_Income", VS(cur, "class_disposable_income"), 0, 1);
			WRITELLS(cur, "Class_Avg_Real_Income", VS(cur, "class_disposable_income")/v[10], 0, 1);
			WRITELLS(cur, "Class_Real_Autonomous_Consumption", 0, 0, 1);         			//write class' autonomous consumption
			WRITELLS(cur, "Class_Liquidity_Preference", 0, 0, 1);
			WRITELLS(cur, "Class_Max_Debt_Rate", 0, 0, 1);
			WRITELLS(cur, "Class_Debt_Rate", 0, 0, 1);                              			//0, no debt initially
			WRITELLS(cur, "Class_Stock_Deposits", 0, 0, 1);



}	

v[191]=VS(cur1, "class_income_share");
v[192]=VS(cur2, "class_income_share");
v[193]=VS(cur3, "class_income_share");
v[194]=VS(cur1, "class_population_share");
v[195]=VS(cur2, "class_population_share");
v[196]=VS(cur3, "class_population_share");
v[197]=1-v[191]*v[194]-v[192]*v[195]-v[193]*v[196]-2*v[193]*v[195]-2*v[193]*v[194]-2*v[192]*v[194];
LOG("Gini Index is %f.",v[197]);

v[198]=WHTAVE("sector_initial_price","sector_initial_demand")/SUM("sector_initial_demand");

		WRITELLS(country,"Country_Nominal_Exports", v[35], 0, 1);
		WRITELLS(country,"Country_Nominal_Imports", v[35], 0, 1);
		WRITELLS(country,"Country_Annual_Growth", 0, 0, 1);													//zero by definition, no growth initally
		WRITELLS(country,"Country_Annual_Real_Growth", 0, 0, 1);                 							//zero by definition, no growth initally
		WRITELLS(country,"Country_Annual_Inflation", 0, 0, 1);	
	for (i=1 ; i<=(V("annual_frequency")+1) ; i++)                  												//for (annual period +1) lags
		{
		WRITELLS(country,"Country_Price_Index", v[198], 0, i);									 			//writes Price_Index, all initial price index is 1
		WRITELLS(country,"Country_Consumer_Price_Index", v[10], 0, i);          							//writes Consumper_Price_Index, all initial price index is 1
		WRITELLS(country,"Country_Capital_Goods_Price", v[11], 0, i);
		WRITELLS(country,"Country_Avg_Productivity", AVE("sector_initial_productivity"), 0, i);
		}
	for (i=1 ; i<=(2*V("annual_frequency")) ; i++)                  												//for (2*annual_period) lags
		{
		WRITELLS(country,"Country_GDP", v[20], 0, i);                     	 								//GDP
		WRITELLS(country,"Country_Real_GDP", (v[20]/v[198]), 0, i);                  						//Real GDP will be equal to nominal GDP because price index always begins as 1
		}

WRITES(external, "Country_Trade_Balance", 0);
WRITES(external, "Country_Capital_Flows", 0);
WRITELLS(external, "External_Income",  v[20], 0, 1);
WRITELLS(external, "External_Income",  v[20], 0, 2);
WRITELLS(external, "Country_International_Reserves",  v[20]*V("annual_frequency"), 0, 1);	

CYCLE(cur, "SECTORS")
{
	v[0]=VS(cur,"sector_investment_frequency"); 
	v[1]=VS(cur,"sector_price_frequency");	
	v[5]=VS(cur,"sector_capital_duration");
	v[201]=VS(cur, "id_consumption_goods_sector");
	v[202]=VS(cur, "id_capital_goods_sector");
	v[203]=VS(cur, "id_intermediate_goods_sector");
	
	v[205]=VS(cur,"sector_initial_productivity");
	v[206]=VS(cur,"sector_initial_markup");
	v[207]=VS(cur,"sector_initial_quality");
	v[209]=VS(cur,"sector_initial_wage");
	v[210]=VS(cur,"sector_input_tech_coefficient");
	v[211]=VS(cur,"sector_capital_output_ratio");
	v[212]=VS(cur,"sector_number_object_firms");
	v[213]=VS(cur,"sector_rnd_revenue_proportion");
	v[214]=VS(cur,"sector_profits_distribution_rate");
	v[215]=VS(cur,"sector_indirect_tax_rate");
	v[216]=VS(cur,"sector_exports_elasticity_income");
	v[217]=VS(cur,"sector_desired_inventories_proportion");	
	v[218]=VS(cur,"sector_desired_degree_capacity_utilization");
	
	if(v[201]==1)																				//if it is a consumption good sector
	{
		v[200]=v[141];																			//intial demand, production and sales
		v[204]=v[101];																			//initial price
		v[222]=v[200]*(1+v[217])/v[218];														//sector productive capacity
		v[223]=v[210]*v[200];
	}
	if(v[202]==1)																				//if it is a capital good sector
	{
		v[200]=v[142];																			//intial demand, production and sales
		v[204]=v[102];																			//initial price
		v[222]=v[157]*((v[41]*v[56]+v[42]*v[57]+v[43]*v[58])/v[0]);								//sector productive capacity, different for the capital good sector
		v[223]=v[210]*v[222];
	}
	if(v[203]==1)																				//if it is a intermediate goods sector
	{
		v[200]=v[143];																			//initial demand production and sales
		v[204]=v[100];																			//initial price
		v[222]=v[200]*(1+v[217])/v[218];														//sector productive capacity
		v[223]=v[210]*v[200];
	}

	
	v[219]=v[222]*v[211]/v[212];																//number of capital goods of each firm
	v[220]=v[204]*((v[149]*v[212]/v[74])/(pow(v[150], v[216])));								//calculate sector exports coefficient
	v[221]=(1+v[217])*v[211]/v[218];
	//v[222]=SEARCH_INSTS(root, cur);	
	
	cur1=SEARCHS(cur, "FIRMS");																	//search the first and only instance of firms below each sector
		//Begin Writting Firm Parameters
		WRITES(cur1, "firm_date_birth", 0);                                   					//zero by definition
		
		//Begin Writting Independent Firm Variables
	for (i=1 ; i<=v[0] ; i++)                                									//for (investment_period) lags
	  	{
	  	WRITELLS(cur1, "Firm_Demand_Productive_Capacity_Replacement", 0, 0, i);					//no replacement initially
	  	WRITELLS(cur1, "Firm_Debt_Rate", 0, 0, i);												//no debt initially
	  	WRITELLS(cur1, "Firm_Demand_Capital_Goods", 0, 0, i);
	  	WRITELLS(cur1, "Firm_Frontier_Productivity", v[205], 0, i);                 			//frontier productivity will be the initial frontier productivity
	  	WRITELLS(cur1, "Firm_Max_Productivity", v[205], 0, i);        							//max capital goods productivity will be the initial frontier productivity that will be the same for all capital goods
	  	WRITELLS(cur1, "Firm_Avg_Productivity", v[205], 0, i);									//firm's avg productivity will be the initial frontier productivity since all capital goods have the same productivity
		WRITELLS(cur1, "Firm_Capacity_Utilization", v[218], 0, i);}
	for(i=1; i<=(v[0]+1); i++)										 							//for (investment period+1) lags (7)
		{
		WRITELLS(cur1, "Firm_Productive_Capacity_Depreciation", 0, 0, i);  						//write 0 
		WRITELLS(cur1, "Firm_Demand_Productive_Capacity_Expansion", 0, 0, i);     				//write 0 
		WRITELLS(cur1, "Firm_Demand_Capital_Goods_Expansion", 0, 0, i);     					//write 0
		WRITELLS(cur1, "Firm_Demand_Capital_Goods_Replacement", 0, 0, i);     					//write 0
		WRITELLS(cur1, "Firm_Effective_Orders_Capital_Goods", 0, 0, i);  						//write 0 
		}
	for (i=1 ; i<=(2*v[0]) ; i++)																//for (2*investment period+1) lags
	  	WRITELLS(cur1, "Firm_Effective_Orders", (v[200]/v[212]), 0, i);                    		//firm's effective orders will be sector's effective orders (given by demand_initial) divided by the number of firms
	for (i=1 ; i<=(v[1]+1) ; i++)																//for (markup_period-1) lags
		{
		WRITELLS(cur1, "Firm_Market_Share", (1/v[212]), 0, i);             						//firm's market share will be the inverse of the number of firms in the sector (initial market share)
	  	WRITELLS(cur1, "Firm_Potential_Markup", v[206], 0, i);                      			//potential markup will be the initial markup
		}
		WRITELLS(cur1, "Firm_Effective_Market_Share", (1/v[212]), 0, 1);                    	//firm's effective market share will be the initial market share       
	  	WRITELLS(cur1, "Firm_Desired_Market_Share", (1/v[212]), 0, 1);                  		//firm's desired market share will be twice the initial market share  
	  	WRITELLS(cur1, "Firm_Avg_Market_Share", (1/v[212]), 0, 1);                     			//firm's avg market share will be the initial market share
	  	WRITELLS(cur1, "Firm_Price", v[204], 0, 1);                                      		//firm's price will be the initial price of the sector, the same for all firms
	  	WRITELLS(cur1, "Firm_Avg_Potential_Markup", v[206], 0, 1);								//avg potential markup will be the initial markup
	  	WRITELLS(cur1, "Firm_Desired_Markup", v[206], 0, 1); 									//desired markup will be the initial markup
	  	WRITELLS(cur1, "Firm_Sales", (v[200]/v[212]), 0, 1);									//firm's sales will be equal to effective orders, no delivery delay
	  	WRITELLS(cur1, "Firm_Revenue", (v[204]*v[200]/v[212]), 0, 1);                           //firm's revenue will be the firm's sales times firm price
	  	WRITELLS(cur1, "Firm_Stock_Inventories", ((v[200]/v[212])*v[217]), 0, 1);               //firm's inventories will be the sales times the desired inventories proportion (parameter)
	  	WRITELLS(cur1, "Firm_Stock_Inputs", v[223]/v[212], 0, 1);                    			//firm's stock of imputs will be the sales times the input tech relationship
	  	WRITELLS(cur1, "Firm_Productive_Capacity", v[222]/v[212], 0, 1);						//firm productive capacity will be the sales divided by the desired degree of capacity utlization (parameter)
	  	WRITELLS(cur1, "Firm_Capital", (v[219]*v[102]), 0, 1);									//firm nominal capital equals number of capital if capital goods price equals 1
	  	WRITELLS(cur1, "Firm_Wage", v[209], 0, 1); 												//firm's nominal wage equals sector nominal wage initial
		WRITELLS(cur1, "Firm_Variable_Cost", ((v[209]/v[205])+v[210]*v[100]), 0, 1);			//firm variable cost equals unitary wage plus unitary cost of inputs. The last equals the tech coefficient if input price equals 1
		WRITELLS(cur1, "Firm_Competitiveness", 1, 0, 1);                           				//if all firms are the same
	  	WRITELLS(cur1, "Firm_Delivery_Delay", 1, 0, 1);                           		  		//it is assumed that all demand is met initially, so equals 1 by definition
		WRITELLS(cur1, "Firm_Stock_Deposits", 0, 0, 1);											//no financial assets initially
	  	WRITELLS(cur1, "Firm_Stock_Loans", v[159]*(v[219]*v[102]), 0, 1);                       //no debt initially
	  	WRITELLS(cur1, "Firm_Avg_Debt_Rate", v[159], 0, 1);                       				//no debt initially
	  	WRITELLS(cur1, "Firm_Max_Debt_Rate", v[152], 0, 1);                       			//no debt initially
	  	WRITELLS(cur1, "Firm_Liquidity_Preference", v[153], 0, 1);                       		//no debt initially
		
	  		//Begin writting Capital Goods Variables and parameters
	  		cur2=SEARCHS(cur1, "CAPITALS");														//search the first and only instance of capital below firms
	  		WRITELLS(cur2, "Capital_Good_Acumulated_Production", 0, 0, 1);      				//zero by definition
			WRITES(cur2, "capital_good_productive_capacity", (1/v[211]));     					//inverse of capital output ratio  
			WRITES(cur2, "capital_good_productivity_initial", v[205]);       		  			//defined in the control parameters
			WRITES(cur2, "capital_good_to_replace", 0);
			WRITES(cur2, "id_capital_good_number", 1);                       					//1 for the first and only initial firm (the others will be created below)
	 			
	 	//Begin Creating Firms and writting some parameters
	 	for(i=1; i<=(v[212]-1); i++)															//for the number of firms of each sector (defined by the parameter)
	 	cur4=ADDOBJ_EXLS(cur,"FIRMS", cur1, 0);                             					//create new firm using the first and only firm as example
	 			
	 	CYCLES(cur, cur1, "FIRMS")                                                 				//CYCLE trough all firms
			{
			v[230]=SEARCH_INSTS(cur, cur1);														//search current firm position in the total economy
			WRITES(cur1, "firm_id", v[230]);                         					//write the firm number as the current position (only initial difference between firms)
			//WRITES(cur1, "firm_bank",(uniform_int(1, v[40])));								//firm's bank identifier
			v[225]=v[230]/(v[212]/v[40]);
			v[226]=round(v[225]);
			if(v[226]<v[225])
				v[227]=v[226]+1;
			else
				v[227]=v[226];
			WRITES(cur1, "firm_bank", v[227]);
			v[231]=fmod((double) (v[230]+v[0]), v[0]);                                 			//divide the firm's number plus investment period by the investment period and takes the rest (possible results if investment period =6 are 0, 5, 4, 3, 2, 1)
			
			//Begin creating capital goods and writting "capital_good_date_birth"		
			for(i=1; i<=(v[219]-1); i++)                        								//for the number of capital goods of each firm
			{
			cur2=SEARCHS(cur1, "CAPITALS");                                         			//search the first and only capital good of each firm
			cur3=ADDOBJ_EXLS(cur1,"CAPITALS", cur2, 0);			                       			//create new capital goods using the first one as example
			WRITES(cur3, "id_capital_good_number", (i+1));										//write the capital good number
			}
			CYCLES(cur1, cur5, "CAPITALS")                                            			//CYCLE trough all capital goods
				{
				v[232]=VS(cur5, "id_capital_good_number");
				v[233]=(-v[5]+v[231]+1)+(v[232]-1)*v[0];                                  		//calculates the capital good date of birth based on the firm number and the number of the capital good
				v[224]=uniform_int(30, 60);
				WRITES(cur5, "capital_good_date_birth", 0);										//write the capital good date of birth
				WRITES(cur5, "capital_good_depreciation_period", v[224]);
				}
			}					
}
	

PARAMETER
RESULT(0)

