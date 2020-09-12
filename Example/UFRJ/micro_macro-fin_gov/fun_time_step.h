
EQUATION("Time_Step")
/*
Root Variable
Determines the order in which the variables will be calculated in each time step. It is the logical sequence of the model in each time step.
*/

/*****INTEREST RATES*****/
V("Basic_Interest_Rate");  
V("Interest_Rate_Deposits");
V("Interest_Rate_Loans_Short_Term");
V("Bank_Competitiveness");
V("Avg_Competitiveness_Financial_Sector");
V("Bank_Market_Share");
V("Bank_Desired_Long_Term_Spread");
V("Bank_Desired_Interest_Rate_Long_Term");
V("Bank_Interest_Rate_Long_Term");  
V("Avg_Interest_Rate_Long_Term");
V("Interest_Rate_Loans_Long_Term");
V("Firm_Avg_Debt_Rate");							   		
V("Firm_Interest_Rate_Short_Term");
V("Firm_Interest_Rate_Long_Term");
V("Class_Avg_Debt_Rate"); 
V("Class_Interest_Rate");   
V("Bank_Max_Total_Loans"); 

/*****EXPORTS (AUTONOMOUS) AND PRICE SETTING*****/
V("Sector_Bargain_Power");                                  
V("Firm_Wage");                                      	   	
V("External_Income");                                  		
V("Sector_External_Price");                                 
V("Exchange_Rate");                                    		
V("Firm_Input_Cost");                                       
V("Firm_Variable_Cost");                                    
V("Firm_Desired_Market_Share");                             
V("Firm_Desired_Markup");                                  
V("Firm_Desired_Price");                                    
V("Firm_Price");                                            
V("Sector_Avg_Price");                                     
V("Sector_Exports");                                   		

/*****CAPITAL GOODS DEMAND*****/
V("Firm_Demand_Capital_Goods");                             
V("Domestic_Capital_Demand");                          		
V("Effective_Orders_Capital_Firm");                    		
V("Firm_Effective_Orders_Capital_Goods");              		

/*****PLANNED PRODUCTION*****/
V("Firm_Expected_Sales");                                   
V("Firm_Planned_Production");                              

/*****EFFECTIVE PRODUCTION AND INPUT DEMAND*****/
V("Firm_Available_Inputs_Ratio");                       	
V("Capital_Good_Productivity");                        		
V("Firm_Competitiveness");                                  
V("Firm_Market_Share");                                     

V("Firm_Required_Inputs");                                  
V("Firm_Input_Demand_Next_Period");                         
V("Domestic_Intermediate_Demand");                     		
V("Intermediate_Production");                          		
V("Firm_Intermediate_Production");                     		
V("Firm_Effective_Production");                             
V("Firm_Capacity_Utilization");								

/*****CONSUMPTION DEMAND AND IMPORTS*****/
V("Class_Avg_Real_Income");
V("Class_Avg_Nominal_Income");
V("Class_Real_Autonomous_Consumption");
V("Class_Real_Desired_Consumption");
V("Class_Real_Desired_Imports");
V("Class_Desired_Expenses");
V("Class_Interest_Payment");
V("Class_Debt_Payment");
V("Class_Financial_Obligations");
V("Class_Liquidity_Preference");
V("Class_Retained_Deposits");
V("Class_Internal_Funds");
V("Class_Desired_Debt_Rate");
V("Class_Max_Loans");
V("Class_Demand_Loans");
V("Class_Effective_Loans");
V("Class_Funds");
V("Class_Maximum_Expenses");
V("Class_Real_Consumption_Demand");
V("Class_Real_Imports_Demand");
V("Domestic_Consumption_Demand"); 

/*****EFFECTIVE ORDERS, SALES AND PROFITS*****/
V("Sector_Effective_Orders");                          		
V("Effective_Orders_Consumption_Firm"); 
V("Firm_Effective_Orders");                                 
V("Firm_Sales");                                           
V("Firm_Delivery_Delay");                                  
V("Sector_Sales");                                     		
V("Firm_Stock_Inventories");                               
V("Firm_Effective_Market_Share");                           
V("Firm_Revenue");                                          
V("Firm_Indirect_Tax");                                     
V("Firm_Net_Revenue");									   	
V("Firm_RND_Expenses");									   	
V("Price_Capital_Goods");                              		           		
V("Firm_Interest_Payment");
V("Firm_Extra_Debt_Payment");
V("Firm_Debt_Payment");
V("Firm_Financial_Obligations");
V("Firm_Deposits_Return");
V("Firm_Net_Profits");                                      
V("Firm_Retained_Profits");                                 
V("Firm_Distributed_Profits");                             
V("Firm_Profit_Rate");	

/*****INCOME GENERATION*****/                           		                                   		                                 		
V("Sector_Demand_Met");                                		
V("Sector_Extra_Imports");                                     
V("Sector_Demand_Met_By_Imports");                            		

/*****R&D*****/
V("Firm_Innovation_Productivity");                          
V("Firm_Imitation_Productivity");                           
V("Firm_Frontier_Productivity");                            
V("Firm_Quality");										   	

/*****CAPITAL AND INVESTMENT DECISIONS*****/
V("Firm_Investment_Period");						   		
V("Firm_Productive_Capacity_Depreciation");                 
V("Firm_Effective_Productive_Capacity_Variation");          
V("Firm_Productive_Capacity");   
V("Firm_Capital");                          
V("Firm_Max_Productivity");                   				
V("Firm_Expected_Sales_Long_Term");                         
V("Firm_Desired_Expansion_Investment_Expenses");            
V("Firm_Desired_Replacement_Investment_Expenses");			
V("Firm_Desired_Investment_Expenses");

/*****FINANCIAL CONSTRAINTS*****/
V("Firm_Liquidity_Preference");								
V("Firm_Retained_Deposits");								
V("Firm_Internal_Funds");									
V("Firm_Desired_Debt_Rate");								
V("Firm_Available_Loans");                                  
V("Firm_Demand_Loans");
V("Bank_Demand_Loans");
V("Bank_Effective_Loans");
V("Bank_Demand_Met");
V("Loans_Distribution_Firms");
V("Firm_Effective_Loans");
V("Firm_Credit_Rationing");
V("Firm_Total_Funds");                                       

/*****EFFECTIVE INVESTMENT*****/
V("Firm_Effective_Expansion_Investment_Expenses");          
V("Firm_Demand_Productive_Capacity_Expansion");              
V("Firm_Available_Funds_Replacement");                       
V("Firm_Demand_Productive_Capacity_Replacement");           
V("Firm_Replacement_Expenses");							   	
V("Firm_Effective_Investment_Expenses");
V("Firm_Available_Funds_After_Replacement");		
V("Firm_Modernization_Rate");		

/*****DEBT*****/
V("Firm_Stock_Loans_Short_Term");
V("Firm_Stock_Loans_Long_Term");
V("Firm_Stock_Loans");                                       
V("Firm_Stock_Deposits");									                                          
V("Firm_Debt_Rate");  
V("Firm_Financial_Position");                                      

/*****ENTRY & EXIT*****/
V("Capital_Good_Production");                           	
V("Firm_Avg_Productivity");                             	
V("Sector_Avg_Productivity");                           	
V("Sector_Avg_Quality");                         			
V("Sector_Max_Quality"); 
V("Exit"); 
V("Exit_Deposits_Distributed"); 
V("Exit_Defaulted_Loans");                     			
V("Sector_Entry_Condition");                                
V("Sector_Productive_Capacity_Exit");						
V("Sector_Productive_Capacity_Entry");						
V("Sector_Productive_Capacity_Available"); 

/*****CLASS INCOME*****/
V("Financial_Sector_Profits");
V("Financial_Sector_Distributed_Profits");  
V("Total_Distributed_Profits"); 
V("Total_Profits"); 
V("Total_Wages");  
V("Class_Nominal_Income"); 
V("Class_Taxation");                            		
V("Class_Real_Income"); 

/*****FINANCIAL SECTOR CONSOLIDATION*****/
V("Sector_Stock_Loans");
V("Sector_Stock_Deposits");
V("Class_Available_Deposits");
V("Class_Stock_Loans");
V("Class_Stock_Deposits");
V("Bank_Stock_Loans_Short_Term");
V("Bank_Stock_Loans_Long_Term");
V("Bank_Total_Stock_Loans");
V("Bank_Stock_Deposits");
V("Bank_Default_Share");
V("Financial_Sector_Accumulated_Profits");
V("Financial_Sector_Stock_Loans_Short_Term");
V("Financial_Sector_Stock_Loans_Long_Term");
V("Financial_Sector_Total_Stock_Loans");
V("Financial_Sector_Short_Term_Rate");
V("Financial_Sector_Stock_Deposits");
V("Financial_Sector_Normalized_HHI");

/*****MACRO RESULTS*****/
V("Total_Inventories");                             	
V("Gross_Value_Production");                            	
V("Avg_Rate_Capacity_Utilization");               	
V("Total_Consumption");                                    		               
V("Total_Investment");                                     		
V("Total_Intermediate");											
V("Sector_Participation");									
V("Total_Indirect_Taxes");                                  
V("Profit_Share");                                      	
V("Wage_Share"); 
V("Avg_Profit_Rate");                                       	
V("GDP");                                            		
V("GDP_Demand");                                   	    	
V("Annual_Growth");                                     	
V("Price_Index");                                       	
V("Consumer_Price_Index");                              	
V("Real_GDP");                                          	
V("Annual_Real_Growth");                                	
V("Likelihood_Crisis");                                 	
V("Sector_Employment");                                 	
V("Sector_Potential_Employment");					    	
V("Sector_Unemployment");									
V("Unemployment");                                      	

/*****ANALYSIS*****/
V("Class_Income_Share");
V("Class_Wealth_Share");
V("Gini_Income_Class");
V("Gini_Wealth_Class");
V("Gini_Income_Population");
V("Gini_Wealth_Population");
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
V("PROD");                                       
V("PROFITS");
V("GDP_G");
V("CON_G");
V("INV_G");
V("GOV_G");
V("PROFITS_G");
V("WAGE_G");
V("PDEBT_G");
V("PROD_G");

RESULT(t)




