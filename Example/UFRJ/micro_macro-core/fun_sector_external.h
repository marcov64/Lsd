
/*****SECTOR VARIABLES RELATED TO THE EXTERNAL SECTOR*****/


EQUATION("Sector_External_Price")
/*
External price of the sector's goods. 
*/
	v[0]=VL("Sector_External_Price",1);
	v[1]=V("external_price_growth");
	if(v[1]!=0)	
		v[2]=(1+v[1])*v[0];
	else
   	{     
		v[3]=VL("Sector_Avg_Price",2);
		v[4]=VL("Sector_Avg_Price",1);
		v[5]=V("external_price_competitiveness");
		v[2]=v[0]*(1+v[5]*((v[4]-v[3])/v[3]));
   	}  
RESULT(v[2])


EQUATION("Sector_Exports")
/*
Exports are defined for each sector based on the application of an export coefficient on external income. The division by price observed in the past period allows to transform the value of exports into units of exported products.
*/
	v[0]=V("External_Income");
	v[1]=V("sector_exports_coefficient");
	v[2]=V("Sector_Avg_Price");
	v[3]=V("Sector_External_Price");
	v[4]=V("exports_elasticity_income");
	v[5]=V("exports_elasticity_price");
	v[6]=V("Exchange_Rate");
	v[7]=v[1]*pow((v[3]*v[6])/v[2],v[5])*pow(v[0],v[4]);
	v[8]=v[7]/v[2]; 
RESULT(v[8])


EQUATION("Sector_Extra_Imports")
/*
The extra import, if the sector can not meet its internal demand, is determined by the difference between the actual orders of the sector and its actual production plus the available stock of products. The value of these imports is obtained by multiplying the previous result by the external price of the inputs of the sector in question.
*/
	v[0]=V("Domestic_Consumption_Demand");
	v[1]=V("Domestic_Capital_Demand");
	v[2]=V("Domestic_Intermediate_Demand");
	v[3]=V("Sector_Demand_Met");
	v[4]=(v[0]+v[1]+v[2])*(1-v[3]);
	v[5]=VL("International_Reserves",1);
	if(v[4]>0)
		{
		if(v[5]>0)
			{
			v[6]=v[4];
			v[7]=1;
			}
		else
			{
			v[6]=0;
			v[7]=0;
			}
		WRITE("Sector_Demand_Met_By_Imports", v[7]);
		}
	else
		v[6]=0;
RESULT(v[6])


EQUATION("Sector_Demand_Met_By_Imports")
RESULT(CURRENT)


