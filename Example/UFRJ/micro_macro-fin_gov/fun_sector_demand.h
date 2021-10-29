/*****PRODUCTION AND DEMAND*****/


EQUATION("Sector_External_Price")
/*
External price of the sector's goods. Defined after firms set their price and sector average price is calculated.
It grows based on 3 parameters, in any combination.
-"sector_external_price_growth" sets a fixed exogenous growth rate
-"sector_external_price_competitiveness" sets how domestic sector influence external price. Is this parameter is 1, external price will grow exactly as the domestic price.
-"sector_external_price_sd" sets the random variation, regardless of the two growth parameters.
User can also define a external price shock:
-"sector_external_price_shock_begin" defines when the shock begin
-"sector_external_price_shock_duration" defines how many perio the shock will last
-"sector_external_price_shock_size" defines the intensity and the direction of the shock. For instance, if this parameter is 1, the external price will grow twice as much as the "normal" growth.
*/
	v[0]=CURRENT;											
	v[1]=V("sector_external_price_growth");									
	v[2]=V("sector_external_price_sd");										
	v[3]=V("sector_external_price_competitiveness");			
	v[4]=LAG_GROWTH(p, "Sector_Avg_Price", 1);
	v[5]=norm((v[1]+v[3]*v[4]), v[2]);			

	v[6]=V("sector_external_price_shock_begin");          				
	v[7]=V("sector_external_price_shock_duration");       				
	v[8]=V("sector_external_price_shock_size");           			
		if(t>=v[6]&&t<v[6]+v[7])
			v[9]=v[5]*(1+v[8]);
		else
			v[9]=v[5];
	v[10]=v[0]*(1+v[9]);	
RESULT(v[10])


EQUATION("Sector_Real_Exports")
/*
Exports are defined for each sector based on the application of an export coefficient on external income. 
This coefficient is endogenous calculated in the initialization. It represents the share of the external income that is allocated to the current sector as demand.
The amount of exports will also depend on sector specific elasticities on relative prices and external income.
The division by price observed in the past period allows to transform the value of exports into units of exported products.
Both sector average price and external price must be calculated before.
*/
	v[0]=V("External_Real_Income");
	v[1]=V("sector_exports_coefficient");
	v[2]=V("Sector_Avg_Price");
	v[3]=V("Sector_External_Price");
	v[4]=V("sector_exports_elasticity_income");
	v[5]=V("sector_exports_elasticity_price");
	v[6]=V("Country_Exchange_Rate");
	v[7]=v[1]*pow((v[3]*v[6])/v[2],v[5])*pow(v[0],v[4]);
	v[8]=v[7]/v[2];
RESULT(v[7])


EQUATION("Sector_Effective_Orders")
/*
Effective orders are determined from total demand for the products in the sector.
Depending on the type of sector, it will add total domestic demand for that type of good with real exports.
This must be changed in the case of more than 1 sector of each type.
This variable also writes an analysis variable that evaluates the relative weight of exports on sector's total demand.
*/
	if(V("id_intermediate_goods_sector")==1)
		v[0]=V("Country_Domestic_Intermediate_Demand");                                                       
	if(V("id_consumption_goods_sector")==1)
		v[0]=V("Country_Domestic_Consumption_Demand");                                                          
	if(V("id_capital_goods_sector")==1)
		v[0]=V("Country_Domestic_Capital_Goods_Demand");                                                    
	v[1]=V("Sector_Real_Exports");                                                              
	v[2]=v[0]+v[1];   
	v[4]=V("sector_initial_demand");
	v[5]=max(v[2],v[4]);
	v[3]= v[5]!=0? v[1]/v[5] : 0;
	WRITE("Sector_Exports_Share", v[3]);
RESULT(max(0,v[5]))

EQUATION_DUMMY("Sector_Exports_Share", "Sector_Effective_Orders")


EQUATION("Firm_Effective_Orders_Capital_Goods")
RESULT(VS(capital,"Firm_Market_Share")*VS(capital,"Sector_Effective_Orders"))


EQUATION("Sector_Extra_Imports")
/*
The extra import, if the sector can not meet its internal demand, is determined by the difference between the actual orders of the sector and its actual production plus the available stock of products. The value of these imports is obtained by multiplying the previous result by the external price of the inputs of the sector in question.
*/
	if(V("id_intermediate_goods_sector")==1)
		v[0]=V("Country_Domestic_Intermediate_Demand");                                                       
	if(V("id_consumption_goods_sector")==1)
		v[0]=V("Country_Domestic_Consumption_Demand");                                                          
	if(V("id_capital_goods_sector")==1)
		v[0]=V("Country_Domestic_Capital_Goods_Demand");
	v[3]=V("Sector_Demand_Met");
	v[4]=v[0]*(1-v[3]);
	v[1]=V("switch_extra_imports");
	if(v[4]>0&&v[1]==1)
		{
		v[6]=v[4];
		WRITE("Sector_Demand_Met_By_Imports", 1);
		}
	else
		{
		v[6]=0;
		WRITE("Sector_Demand_Met_By_Imports", 0);
		}
RESULT(v[6])

EQUATION_DUMMY("Sector_Demand_Met_By_Imports", "Sector_Extra_Imports")




