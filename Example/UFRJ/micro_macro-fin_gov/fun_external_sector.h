/*****EXTERNAL SECTOR*****/


EQUATION("External_Real_Income")
/*
Real value of external income.
*/
	v[0]=CURRENT;
	v[3]=V("external_income_growth");						//fixed external income growth
	v[4]=V("external_income_sd");							//fixed external income sd
	v[5]=LAG_GROWTH(country, "Country_Real_GDP", 1, 1);
	v[7]=V("external_income_adjustmnent");                  //exogenous parameter that amplifies external growth based on domestic growth
	v[8]=norm((v[3]+v[5]*v[7]), v[4]);						//random draw from a normal distribution with average equals to past growth and standard deviation equals to past growth in absolute value	
	v[9]=V("external_shock_begin");          				//defines when the shock happens
	v[10]=V("external_shock_duration");       				//defines how long the shock lasts
	v[11]=V("external_shock_size");           				//defines the size, in percentage, of the shock
	if(t>=v[9]&&t<v[9]+v[10])
		v[12]=v[8]*(1+v[11]);
	else
		v[12]=v[8];

	v[13]=(1+v[12])*v[0];
RESULT(max(0,v[13]))


EQUATION("Country_Nominal_Exports")
/*
The total exports of the economy in nominal value are defined by the sum of the exports of each sector multiplied by the price charged in the period.
*/
	v[0]=0;
	CYCLES(country, cur, "SECTORS")
	{
		v[1]=VS(cur, "Sector_Real_Exports");
		v[2]=VS(cur, "Sector_Avg_Price");
		v[3]=VS(cur, "Sector_Demand_Met");
		v[4]=v[1]*v[2]*v[3];
		v[0]=v[0]+v[4];
	}
RESULT(v[0])


EQUATION("Country_Nominal_Imports")
/*
Total imports in nominal value are obtained from the sum of imports of all sectors multiplied by the respective international prices, and converted to national currency by the exchange rate.
*/
	v[0]=WHTAVES(country, "Sector_Extra_Imports", "Sector_External_Price");
	v[1]=SUMS(country, "Class_Effective_Real_Imported_Consumption");
	v[2]=VS(external,"Country_Exchange_Rate");
	v[4]=VS(consumption, "Sector_External_Price");
	v[3]=(v[0]+v[1]*v[4])*v[2];
	v[5]=0;
	CYCLES(country, cur, "SECTORS")
		v[5]=v[5]+SUMS(cur, "Firm_Input_Imports");
	v[6]=VS(input, "Sector_External_Price");
	v[7]=(v[0]+v[1]*v[4]+v[5]*v[6])*v[2];
RESULT(v[7])


EQUATION("Country_Trade_Balance")
RESULT(V("Country_Nominal_Exports")-V("Country_Nominal_Imports"))

EQUATION("Country_Capital_Flows")
/*
Country net capital flows are a function of the quarterly nominal interest rate differentials and is a multiple of the domestic GDP.
*/
	v[0]=VL("Country_GDP",1);
	v[1]=V("Central_Bank_Basic_Interest_Rate");
	v[2]=V("external_interest_rate");
	v[3]=pow(1+v[2],1/V("annual_frequency"))-1;
	v[4]=V("external_capital_flow_adjustment");
	v[5]=V("Country_Exchange_Rate");
	v[6]=(v[1]-v[3])*v[0]*v[4]*v[5];
RESULT(v[6])

EQUATION("Country_International_Reserves")
	
	v[1]=V("Country_Trade_Balance");
	v[2]=V("Country_Capital_Flows");
	v[3]=VL("Country_International_Reserves",1);
	v[4]=VL("Country_External_Debt",1);
	if(v[3]>=0)
		v[5]=v[3]+v[1]+v[2];
	if(v[4]>=0)
		v[5]=v[4]-v[1]-v[2];
	if(v[5]>=0)
	{
		v[6]=v[5];
		v[7]=0;
	}
	else
	{
		v[6]=0;
		v[7]=-v[5];
	}
	WRITE("Country_External_Debt", v[7]);
RESULT(v[6])

EQUATION_DUMMY("Country_External_Debt", "Country_International_Reserves")


EQUATION("Country_International_Reserves_GDP_Ratio")
	v[0]=V("annual_frequency");
	v[1]=V("Country_International_Reserves");
	v[2]=V("Country_GDP");
	v[3]= v[2]!=0? v[1]/v[2] : 0;
RESULT(v[3])


EQUATION("Country_External_Debt_GDP_Ratio")
	v[0]=V("annual_frequency");
	v[1]=V("Country_External_Debt");
	v[2]=V("Country_GDP");
	v[3]= v[2]!=0? v[1]/v[2] : 0;
RESULT(v[3])


EQUATION("Country_Exchange_Rate")
/*
Nominal exchange rate.
*/
	v[0]=CURRENT;
	v[1]=VL("Country_Trade_Balance",1)+VL("Country_Capital_Flows",1);
	v[2]=V("exchange_rate_adjustment");
	if(v[1]<0)
		v[3]=v[0]+v[2];
	else if(v[1]>0)
		v[3]=v[0]-v[2];
	else
		v[3]=v[0];
	//v[3]=v[0]+v[2]*v[1];
	v[4]=V("exchange_rate_min");
	v[5]=V("exchange_rate_max");
	v[6]=max(min(v[3],v[5]),v[4]);	
	v[7]=V("begin_flexible_exchange_rate");
	if(t>v[7]&&v[7]!=-1)
		v[8]=v[6];
	else
		v[8]=v[0];
RESULT(v[8])















