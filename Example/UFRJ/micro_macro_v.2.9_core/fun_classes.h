/*****INCOME CLASSES*****/ 


EQUATION("Class_Nominal_Income")
/*
Class nominal income shall be calculated by summing the ratio of the total surplus to the proportion of the net salary that is allocated to the class plus the payment of government interest on the domestic public debt.
*/
	v[0]=V("Total_Distributed_Profits");                         				//total distributed profits
	v[1]=V("Total_Wages");                                 				//total wages
	v[2]=V("class_profit_share");                          				//profit share of each class
	v[3]=V("class_wage_share");                            				//wage share of each class
	v[4]=V("class_direct_tax");                            				//income tax percentage for each class
	v[9]=(v[0]*v[2]+v[1]*v[3])*(1-v[4]);                  				//class' net nominal income will be the class profits plus the class wages, minus the income tax
RESULT(v[9])


EQUATION("Class_Real_Income")
/*
Class real income is the nominal income divided by the class price index
*/
	v[0]=V("Class_Nominal_Income");
	v[1]=V("Consumer_Price_Index");
	v[2]=v[0]/v[1];
RESULT(v[2])


EQUATION("Class_Autonomous_Consumption")
/*
Class autonomous consumption depends on the average quality growth of the consumption goods sector
*/
	v[0]=VL("Class_Autonomous_Consumption",1);                          //class autonomous consumption in the last period
	v[1]=V("class_period");												//defines the class adjustment period 
	v[2]= fmod((double) t,v[1]);										//divides time period by the class period and takes the rest
	if (v[2]==0)														//if it is class adjustment period 	
		{
		cur = SEARCH_CNDS(root, "id_consumption_goods_sector", 1);		//search consumption good sector
		v[3]=VLS(cur, "Sector_Avg_Quality",1);     						//sector average quality in the last period                        
		v[4]=VLS(cur, "Sector_Avg_Quality",(v[1]+1)); 					//sector average quality in the last adjustment period                          
		if(v[4]!=0)														//if initial average quality is not zero                                                                              
			v[5]=(v[3]-v[4])/v[4];          							//computate quality growth                                                  			
		else     														//if average quality is zero                                                                               
			v[5]=0;														//quality growth is zero
		
		v[6]=V("class_autonomous_consumption_adjustment");				//autonomous consumption adjustment parameter
		if(v[5]>0) 														//if quality growth was positive
			v[7]=v[0]+v[6]; 											//increase autonomous consumption by the adjustment parameter
		if(v[5]==0)														//if quality grwoth was zero
			v[7]=v[0];													//use last period autonomous consumption
		if(v[5]<0)														//if quality growth was negative
			v[7]=v[0]+v[6];												//decrease autonomous consumption by the adjustment parameter
		}
else																	//if it is not class adjustment period
	v[7]=v[0]; 															//use lase period autonomous consumption                                                                       		
RESULT(v[7])


EQUATION("Class_Real_Domestic_Consumption")
/*
Class real domestic conumption is based on average past real income and on the class' propensity to consume, plus autonomous consumption
*/
	v[0]=V("class_period");												//define the class adjustment period
	v[1]=0;																//initializes the sum
		for (v[2]=1; v[2]<=v[0]; v[2]=v[2]+1)							//for the number os lags equal the adjustment parameter
			{
			v[3]=VL("Class_Real_Income", v[2]);           				//lagged class' income
			v[1]=v[1]+v[3];												//sum lagged income until the adjustment period
			}
  	v[4]=v[1]/v[0];                  									//class average income in the last v[0] periods
  	v[5]=V("class_propensity_to_consume");          					//class propensity to consume on income
  	v[6]=V("Class_Autonomous_Consumption");         					//class autonomous consumption
  	v[7]=v[4]*v[5]+v[6];                            					//class real induced consumption
RESULT(v[7])


EQUATION("Class_Real_Imports")
/*
Class real imports is based on average past real income and on the class' propensity to import and restricted to the amount of existing international resevres
*/
	v[0]=V("class_period");												//define the class adjustment period
	v[1]=0;																//initializes the sum
		for (v[2]=1; v[2]<=v[0]; v[2]=v[2]+1)							//for the number os lags equal the adjustment parameter
			{
			v[3]=VL("Class_Real_Income", v[2]);           				//lagged class' income
			v[1]=v[1]+v[3];												//sum lagged income until the adjustment period
			}
  	v[4]=v[1]/v[0];                  									//class average income in the last v[0] periods
  	v[5]=V("class_propensity_to_import");           					//class propensity to import
  	v[6]=v[4]*v[5];
  
  	cur=SEARCHS(root, "EXTERNAL_SECTOR");								//search the external sector object
	v[7]=VLS(cur, "International_Reserves", 1);							//level of international reserves in the last period
	if (v[7]>0)															//if the amount of reserves is positive 
		v[8]=v[6];														//effective level of imports is equal to desired
	else																//if there are no international reserves																						
		v[8]=0;															//effective level of imports is zero, as well extra imports	
RESULT(v[8])


EQUATION("Class_Effective_Domestic_Consumption")
/*
Class effective real domestic consumption, depending on how much the domestic consumption goods sector was able to meet demand.
*/
	cur=SEARCH_CNDS(root, "id_consumption_goods_sector", 1);     		//identifies the consumption goods sector
	v[0]=VS(cur,"Sector_Demand_Met");                     			 	//percentage of the total demand met by the sector
	v[1]=V("Class_Real_Domestic_Consumption");							//desired level of domestic consumption      
	v[2]=v[0]*v[1];
RESULT(v[2])


EQUATION("Class_Effective_Imports")
/*
Class effective external domestic consumption, depending on desired level of imports plus the demand not met by the domestic production
*/
	cur=SEARCH_CNDS(root, "id_consumption_goods_sector", 1);     		//identifies the consumption goods sector
	v[0]=VS(cur,"Sector_Demand_Met");                     			 	//percentage of the total demand met by the sector
	v[1]=VS(cur,"Sector_Demand_Met_By_Imports");                 		//identifies if classes were capable of importing the amount not mey by the domestic production
	v[2]=(1-v[0])*v[1];													//percentage of domestic demand met by extra imports
	v[3]=V("Class_Real_Domestic_Consumption");							//desired level of domestic consumption 
	v[4]=V("Class_Real_Imports"); 									 	//desired level of external consumption
	v[5]=v[2]*v[3] + v[4];												//effective imports is the derired level plus additional imports if domestic sector cannot supply						
RESULT(v[5])


EQUATION("Class_Expenses")
/*
Class' nominal expenses depends on effective domestic consumption times avg price plus effective external consumption times foreign price
*/
	v[0]=V("Class_Effective_Domestic_Consumption");                     // class effective domesti consumption
	v[1]=V("Class_Effective_Imports");                                	// class effective external consumption
	v[2]=V("Exchange_Rate");                                            //exchange rate
	
	cur=SEARCH_CNDS(root, "id_consumption_goods_sector", 1);     		//identifies the consumption goods sector
	v[3]=VS(cur, "Sector_Avg_Price");                                   //sector average price
	v[4]=VS(cur, "Sector_External_Price");                              //sector external price
	v[5]=v[0]*v[3] + v[1]*v[2]*v[4];     								//total nominal expenses                  	
RESULT(v[5])


EQUATION("Class_Savings")
/*
Class current debt is the diference between current nominal income and current expenses
*/
	v[0]=V("Class_Nominal_Income");
	v[1]=V("Class_Expenses");
	v[2]=v[0]-v[1];
RESULT(v[2])


EQUATION("Class_Government_Interest_Payment")
/*
Distributes government interest payment for the classes given by the total amount of government interest payment mutiplied by the class share on intrest payment
*/
	v[0]=V("Government_Interest_Payment");
	v[1]=V("class_interest_payment_share");
	v[2]=v[0]*v[1];
RESULT(v[2])


EQUATION("Class_Financial_Assets")
/*
Financial Assets of each income class.
*/
	v[0]=VL("Class_Financial_Assets",1);                                         //class financial assets in the last period
	v[1]=V("Class_Savings");                                                     //class current debt
	v[2]=VL("Class_Debt_Payment",1);                                             //class debt payment in the last period
	v[3]=V("Class_Financial_Asset_Return");                                      //return on the class financial assets
	v[4]=VL("Class_Debt",1);                                                     //class stock of debt in the last period
	v[5]=V("Basic_Interest_Rate");                                               //interest rate
	v[6]=max(0,v[4]*v[5]);                                                       //class interest payment, interest rate multiplied by the stock of debt, can never be negative
	v[7]=V("Class_Government_Interest_Payment");                                 //government interest payment for each class
	v[8]=max((v[3]+v[7])-v[6],0);                                                //net financial income in the period, given by total interest received, discounted income taxes over it, minus class interest payment, and it can never be negative
	if(v[1]>0)                                                                   //if class current debt is positive (income higher then expenses)
		v[9]=v[1];                                                               //the diference becomes the class new financial assets
	else                                                                         //if the current debt is negative (expenses higher then income)
		v[9]=0;                                                                  //class new financial assets will be zero
	v[10]=v[0]+v[9]+v[8]-v[2];                                                   //the stock of class financial assets will be the stock in the last period plus new financial assets plus net financial income, minus the debt payment
	v[11]=max(v[10], 0);                                                         //stock of financial assets can never be negative
RESULT(v[11])


EQUATION("Class_Financial_Asset_Return")
/*
Return on the financial assets of each income class, given by the stock of financial assets in the last period multiplied by the return rate
*/
	v[0]=VL("Class_Financial_Assets",1);
	v[1]=V("Basic_Interest_Rate");
	v[2]=v[0]*v[1];
RESULT(v[2])


EQUATION("Class_Debt")
/*
Stock of debt of each income class
*/
	v[0]=VL("Class_Debt",1);                                                   //stock of debt in the last period
	v[1]=V("Class_Savings");                                                   //class current debt
	v[2]=VL("Class_Debt_Payment",1);                                           //debt payment in the last period
	v[3]=V("Class_Financial_Asset_Return");                                    //class financial assets return                                                                    
	v[5]=V("Basic_Interest_Rate");                                             //interest rate
	v[6]=max(0,v[0]*v[5]);                                                     //class interest payment, can never be negative
	v[7]=-min(v[3]*(-v[6]),0);                                                 //net financial income, given by the assets return discounted the income tax, minus interest payments, and can never be negative
	if(v[1]>0)                                                                 //if class income is higher then expenses
		v[8]=0;                                                                //class current debt is zero
	else                                                                       //if class expenses is higher then income
		v[8]=-v[1];                                                            //current debt will be de diference                                                
	v[9]=v[0]+v[8]-v[2]+v[7];                                                  //class stock of debt will be the stock of debt in the last period plus new debts minus debt payment of the last period minus net financial income
	v[10]=max(v[9],0);                                                         //class stock of debt can never be negative
RESULT(v[10])


EQUATION("Class_Debt_Ratio")
/*
Is the ratio between class stock of debt and class financial assets
*/
	v[0]=V("Class_Debt");
	v[1]=V("Class_Financial_Assets");
	if(v[1]!=0)
		v[2]=v[0]/v[1];
	else
		v[2]=0;
RESULT(v[2])


EQUATION("Class_Debt_Payment")
/*
Defines a priority between debt payment with financial assets, selling properties and possibilities of autonomous consumption.
*/
	v[0]=V("Class_Debt_Ratio");                                              	//current class debt ratio                                
	v[1]=V("class_max_debt_ratio");                                             //maximun debt ratio of the class
	v[2]=V("Class_Financial_Assets");                                           //stock of financial assets
	v[3]=V("Class_Debt");                                                       //stock of debt
	v[4]=v[3]-v[1]*v[2];                                                        //current debt minus the minimum debt, based on the minimum debt ratio and the current stock of financial assets                                                                    
	v[5]=max(min(v[2],v[4]),0);                                                 //debt payment will be the diference but can not be higher then the financial assets, and can never be negative
RESULT(v[5])






