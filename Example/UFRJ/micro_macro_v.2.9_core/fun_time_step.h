
EQUATION("Time_Step")
/*
Root Variable
Determines the order in which the variables will be calculated in each time step. It is the logical sequence of the model in each time step.
*/

/*****GOVERNMENT EXPENSES (AUTONOMOUS)*****/
V("Basic_Interest_Rate");                                   //government variable
V("Government_Max_Expenses");                          		//government variable
V("Government_Wages");                                 		//government variable

/*****EXPORTS (AUTONOMOUS) AND PRICE SETTING*****/
V("Sector_Bargain_Power");                                  //sector variable
V("Firm_Wage");                                      	   	//sector variable
V("External_Income");                                  		//external sector variable
V("Sector_External_Price");                                 //sector variable
V("Exchange_Rate");                                    		//external sector variable
V("Firm_Input_Cost");                                       //input variable
V("Firm_Variable_Cost");                                    //firm variable
V("Firm_Desired_Market_Share");                             //firm variable
V("Firm_Desired_Markup");                                   //firm variable
V("Firm_Desired_Price");                                    //firm variable
V("Firm_Price");                                            //firm variable
V("Sector_Avg_Price");                                      //sector variable
V("Sector_Exports");                                   		//sector variable 

/*****CAPITAL GOODS DEMAND*****/
V("Firm_Demand_Capital_Goods");                             //firm variable
V("Domestic_Capital_Demand");                          		//macro variable
V("Effective_Orders_Capital_Firm");                    		//sector variable
V("Firm_Effective_Orders_Capital_Goods");              		//firm variable

/*****PLANNED PRODUCTION*****/
V("Firm_Expected_Sales");                                   //firm variable
V("Firm_Planned_Production");                               //firm variable

/*****EFFECTIVE PRODUCTION AND INPUT DEMAND*****/
V("Firm_Available_Inputs_Ratio");                       	//firm variable
V("Capital_Good_Productivity");                        		//capital variable
V("Firm_Competitiveness");                                  //firm variable
V("Firm_Market_Share");                                     //firm variable

V("Firm_Required_Inputs");                                  //firm variable 
V("Firm_Input_Demand_Next_Period");                         //firm variable
V("Domestic_Intermediate_Demand");                     		//macro variable
V("Intermediate_Production");                          		//macro variable
V("Firm_Intermediate_Production");                     		//firm variable
V("Firm_Effective_Production");                             //firm variable

/*****CONSUMPTION DEMAND*****/
V("Domestic_Consumption_Demand");                      		//macro variable
V("Sector_Effective_Orders");                          		//sector variable
V("Effective_Orders_Consumption_Firm");                		//sector variable

/*****EFFECTIVE ORDERS, SALES AND PROFITS*****/
V("Firm_Effective_Orders");                                 //firm variable 
V("Firm_Sales");                                            //firm variable
V("Firm_Delivery_Delay");                                   //firm variable
V("Sector_Sales");                                     		//sector variable
V("Firm_Stock_Inventories");                                //firm variable
V("Firm_Effective_Market_Share");                           //firm variable
V("Firm_Revenue");                                          //firm variable
V("Firm_Indirect_Tax");                                     //firm variable
V("Firm_Net_Revenue");									   	//firm variable
V("Firm_RND_Expenses");									   	//firm variable
V("Price_Capital_Goods");                              		//macro variable
V("Firm_Interest_Rate");                               		//firm variable
V("Firm_Net_Profits");                                      //firm variable
V("Firm_Retained_Profits");                                 //firm variable
V("Firm_Distributed_Profits");                              //firm variable

/*****INCOME GENERATION*****/
V("Total_Distributed_Profits");                              		//macro variable
V("Total_Profits");                                    		//macro variable
V("Total_Wages");                                      		//macro variable
V("Sector_Demand_Met");                                		//sector variable
V("Sector_Extra_Imports");                                  //sector variable   
V("Sector_Demand_Met_By_Imports");                          //sector variable
V("Class_Nominal_Income");                             		//class variable
V("Class_Real_Income");                                		//class variable

/*****R&D*****/
V("Firm_Innovation_Productivity");                          //firm variable
V("Firm_Imitation_Productivity");                           //firm variable
V("Firm_Frontier_Productivity");                            //firm variable
V("Firm_Quality");										   	//firm variable

/*****CAPITAL AND INVESTMENT DECISIONS*****/
V("Firm_Investment_Period");						   		//firm variable
V("Firm_Productive_Capacity_Depreciation");                 //firm variable
V("Firm_Effective_Productive_Capacity_Variation");          //firm variable
V("Firm_Productive_Capacity");                              //firm variable
V("Firm_Max_Productivity");                   				//firm variable
V("Firm_Expected_Sales_Long_Term");                         //firm variable
V("Firm_Desired_Expansion_Investment_Expenses");            //firm variable

/*****FINANCIAL CONSTRAINTS*****/
V("Firm_Available_Financial_Assets");                       //firm variable
V("Firm_Available_Debt");                                   //firm variable
V("Firm_Total_Funds");                                      //firm variable 

/*****EFFECTIVE INVESTMENT*****/
V("Firm_Effective_Expansion_Investment_Expenses");          //firm variable 
V("Firm_Demand_Productive_Capacity_Expansion");             //firm variable  
V("Firm_Available_Funds_Replacement");                      //firm variable 
V("Firm_Demand_Productive_Capacity_Replacement");           //firm variable 
V("Firm_Replacement_Expenses");							   	//firm variable
V("Firm_Available_Funds_After_Replacement");				//firm variable

/*****DEBT*****/
V("Firm_Debt_Flow");                                        //firm variable
V("Firm_Financial_Assets_Flow");                            //firm variable
V("Firm_Stock_Debt");                                       //firm variable  
V("Firm_Capital");                                          //firm variable  
V("Firm_Debt_Rate");                                        //firm variable
V("Firm_Avg_Debt_Rate");							   		//firm variable

/*****ENTRY & EXIT*****/
V("Capital_Good_Production");                           	//capital variable
V("Firm_Avg_Productivity");                             	//firm variable
V("Sector_Avg_Productivity");                           	//sector variable
V("Sector_Avg_Quality");                         			//sector variable
V("Sector_Max_Quality");                        			//sector variable
V("Sector_Entry_Condition");                                //sector variable
V("Sector_Productive_Capacity_Exit");						//sector variable
V("Sector_Productive_Capacity_Entry");						//sector variable
V("Sector_Productive_Capacity_Available");                  //sector variable

/*****MACRO RESULTS*****/
V("Total_Inventories");                             	//macro variable
V("Gross_Value_Production");                            	//macro variable
V("Avg_Rate_Capacity_Utilization");               	//macro variable
V("Total_Consumption");                                    		//macro variable                 
V("Total_Investment");                                     		//macro variable
V("Total_Intermediate");											//macro variable
V("Sector_Participation");									//sector variable
V("Total_Indirect_Taxes");                                  //government variable
V("Profit_Share");                                      	//macro variable
V("Wage_Share");                                        	//macro variable
V("GDP");                                            		//macro variable
V("GDP_Demand");                                   	    	//macro variable
V("Annual_Growth");                                     	//macro variable
V("Price_Index");                                       	//macro variable
V("Consumer_Price_Index");                              	//macro variable
V("Real_GDP");                                          	//macro variable
V("Annual_Real_Growth");                                	//macro variable
V("Likelihood_Crisis");                                 	//macro variable
V("Sector_Employment");                                 	//sector variable
V("Sector_Potential_Employment");					    	//sector variable
V("Sector_Unemployment");									//sector variable
V("Unemployment");                                      	//macro variable

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
V("Firm_Wage");

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




