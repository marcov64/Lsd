
EQUATION("Time_Step")
/*
Root Variable
Determines the order in which the variables will be calculated in each time step. It is the logical sequence of the model in each time step.
*/

/*****GOVERNMENT EXPENSES (AUTONOMOUS)*****/
V("Interest_Rate");                                    //government variable
V("Max_Government_Expenses");                          //government variable
V("Government_Wages");                                 //government variable

/*****EXPORTS (AUTONOMOUS) AND PRICE SETTING*****/
V("Bargain_Power");                                    //sector variable
V("Wage");                                      	   //sector variable
V("External_Income");                                  //external sector variable
V("Foreign_Price");                                    //sector variable
V("Exchange_Rate");                                    //external sector variable
V("Input_Cost");                                       //input variable
V("Variable_Cost");                                    //firm variable
V("Desired_Market_Share");                             //firm variable
V("Desired_Markup");                                   //firm variable
V("Desired_Price");                                    //firm variable
V("Price");                                            //firm variable
V("Avg_Price");                                        //sector variable
V("Exports_Sector");                                   //sector variable 

/*****CAPITAL GOODS DEMAND*****/
V("Demand_Capital_Goods");                             //firm variable
V("Domestic_Capital_Demand");                          //macro variable
V("Effective_Orders_Capital_Firm");                    //sector variable
V("Effective_Orders_Capital_Goods_Firm");              //firm variable

/*****PLANNED PRODUCTION*****/
V("Expected_Sales");                                   //firm variable
V("Planned_Production");                               //firm variable

/*****EFFECTIVE PRODUCTION AND INPUT DEMAND*****/
V("Available_Inputs_Ratio");                       	   //firm variable
V("Capital_Good_Productivity");                        //capital variable
V("Competitiveness");                                  //firm variable
V("Market_Share");                                     //firm variable

V("Required_Inputs");                                  //firm variable 
V("Input_Demand_Next_Period");                         //firm variable
V("Domestic_Intermediate_Demand");                     //macro variable
V("Intermediate_Production");                          //macro variable
V("Intermediate_Production_Firm");                     //firm variable
V("Effective_Production");                             //firm variable

/*****CONSUMPTION DEMAND*****/
V("Domestic_Consumption_Demand");                      //macro variable
V("Effective_Orders_Sector");                          //sector variable
V("Effective_Orders_Consumption_Firm");                //sector variable

/*****EFFECTIVE ORDERS, SALES AND PROFITS*****/
V("Effective_Orders");                                 //firm variable 
V("Sales");                                            //firm variable
V("Delivery_Delay");                                   //firm variable
V("Sales_Sector");                                     //sector variable
V("Inventories");                                      //firm variable
V("Effective_Market_Share");                           //firm variable
V("Revenue");                                          //firm variable
V("Indirect_Tax");                                     //firm variable
V("Net_Revenue");									   //firm variable
V("RND_Expenses");									   //firm variable
V("Price_Capital_Goods");                              //macro variable
V("Interest_Rate_Firm");                               //firm variable
V("Net_Profits");                                      //firm variable
V("Retained_Profits");                                 //firm variable
V("Profits_Distribution");                             //firm variable

/*****INCOME GENERATION*****/
V("Distributed_Profits");                              //macro variable
V("Total_Profits");                                    //macro variable
V("Total_Wages");                                      //macro variable
V("Demand_Met_Sector");                                //sector variable
V("Extra_Imports");                                    //sector variable   
V("Demand_Met_By_Imports");                            //sector variable
V("Class_Nominal_Income");                             //class variable
V("Class_Real_Income");                                //class variable

/*****R&D*****/
V("Innovation_Productivity");                          //firm variable
V("Imitation_Productivity");                           //firm variable
V("Frontier_Productivity");                            //firm variable
V("Quality");										   //firm variable

/*****CAPITAL AND INVESTMENT DECISIONS*****/
V("Investment_Period_Firm");						   //firm variable
V("Depreciation_Productive_Capacity");                 //firm variable
V("Effective_Productive_Capacity_Variation");          //firm variable
V("Productive_Capacity");                              //firm variable
V("Max_Capital_Goods_Productivity");                   //firm variable
V("Expected_Sales_Long_Term");                         //firm variable
V("Desired_Expansion_Investment_Expenses");            //firm variable

/*****FINANCIAL CONSTRAINTS*****/
V("Available_Financial_Assets");                       //firm variable
V("Available_Debt");                                   //firm variable
V("Funds");                                            //firm variable 

/*****EFFECTIVE INVESTMENT*****/
V("Effective_Expansion_Investment_Expenses");          //firm variable 
V("Demand_Productive_Capacity_Expansion");             //firm variable  
V("Available_Funds_Replacement");                      //firm variable 
V("Demand_Productive_Capacity_Replacement");           //firm variable 
V("Replacement_Expenses");							   //firm variable
V("Available_Funds_After_Replacement");				   //firm variable

/*****DEBT*****/
V("Flow_Debt");                                        //firm variable
V("Flow_Financial_Assets");                            //firm variable
V("Stock_Debt");                                       //firm variable  
V("Capital");                                          //firm variable  
V("Debt_Rate");                                        //firm variable
V("Firm_Avg_Debt_Rate");							   //firm variable

/*****ENTRY & EXIT*****/
V("Capital_Good_Production");                           //capital variable
V("Firm_Avg_Productivity");                             //firm variable
V("Avg_Productivity_Sector");                           //sector variable
V("Avg_Quality_Sector");                         		//sector variable
V("Max_Quality_Sector");                        		//sector variable
V("Entry_Condition");                                   //sector variable
V("Productive_Capacity_Exit");							//sector variable
V("Productive_Capacity_Entry");							//sector variable
V("Productive_Capacity_Available");                     //sector variable

/*****MACRO RESULTS*****/
V("Aggregate_Inventories");                             //macro variable
V("Gross_Value_Production");                            //macro variable
V("Aggregate_Rate_Capacity_Utilization");               //macro variable
V("Consumption");                                    	//macro variable                 
V("Investment");                                     	//macro variable
V("Intermediate");										//macro variable
V("Sector_Participation");								//sector variable
V("Indirect_Taxes");                                    //government variable
V("Profit_Share");                                      //macro variable
V("Wage_Share");                                        //macro variable
V("GDP");                                            	//macro variable
V("GDP_Demand");                                   	    //macro variable
V("Annual_Growth");                                     //macro variable
V("Price_Index");                                       //macro variable
V("Consumer_Price_Index");                              //macro variable
V("Real_GDP");                                          //macro variable
V("Annual_Real_Growth");                                //macro variable
V("Likelihood_Crisis");                                 //macro variable
V("Employment_Sector");                                 //sector variable
V("Potential_Employment_Sector");					    //sector variable
V("Unemployment_Sector");								//sector variable
V("Unemployment");                                      //macro variable

/*****ANALYSIS*****/
V("P");                                                
V("U");                                               
V("G_n");                                              
V("Cri");                                              
V("C");                                                
V("I");                                                
V("C_r");                                              
V("I_r");                                              
V("G_r");
V("M_r");
V("PDEBT");
V("DEBT");
V("PROD");                                       
V("PROFITS");
V("WAGE");

V("GDP_G");
V("CON_G");
V("INV_G");
V("GOV_G");
V("PROFITS_G");
V("WAGE_G");
V("PDEBT_G");
V("DEBT_G");
V("PROD_G");

RESULT(t)




