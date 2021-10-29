
EQUATION("Time_Step")
/*
Root Variable
Determines the order in which the variables will be calculated in each time step. 
It is the logical sequence of the model in each time step.
Comments indicate the level of each variable. 
*/
V("Calibration");

/*****GOVERNMENT EXPENSES (AUTONOMOUS)*****/
V("Basic_Interest_Rate");                                  
V("Government_Max_Expenses");                          		
V("Government_Wages");                                 		

/*****PRICE SETTING*****/
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

/*****CAPITAL GOODS DEMAND*****/
V("Firm_Demand_Capital_Goods");                             
V("Sector_Domestic_Capital_Demand");                          		                  		
V("Firm_Effective_Orders_Capital_Goods");              		

/*****PRODUCTION*****/
V("Firm_Expected_Sales");                                   
V("Firm_Planned_Production");                               
V("Firm_Available_Inputs_Ratio");                       	
V("Capital_Good_Productivity");                        		
V("Firm_Competitiveness");                                  
V("Firm_Market_Share");                                    
V("Firm_Required_Inputs");                                  
V("Firm_Input_Demand_Next_Period");                         
V("Sector_Domestic_Intermediate_Demand");                   
V("Firm_Effective_Production");                             
V("Capital_Good_Production");                           	
V("Capital_Good_Acumulated_Production");					
V("Capital_Good_Productivity");								
V("Firm_Capacity_Utilization");								
V("Firm_Avg_Productivity");                             	
V("Sector_Avg_Productivity");                           	
V("Sector_Employment");                                 	
V("Sector_Potential_Employment");					    	
V("Sector_Unemployment");									

/*****CONSUMPTION DEMAND*****/
V("Sector_Domestic_Consumption_Demand"); 
V("Sector_Exports");                     
V("Sector_Effective_Orders");                          		             		

/*****EFFECTIVE ORDERS, SALES AND PROFITS*****/
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
V("Firm_Interest_Rate");                               		
V("Firm_Net_Profits");                                     
V("Firm_Retained_Profits");                                
V("Firm_Distributed_Profits");                             

/*****INCOME GENERATION*****/
V("Total_Distributed_Profits");                            
V("Total_Profits");                                    		
V("Total_Wages");                                      		
V("Sector_Demand_Met");                                		
V("Sector_Extra_Imports");                                 
V("Sector_Demand_Met_By_Imports");                          
V("Class_Nominal_Income");                             		
V("Class_Real_Income");                                		

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
V("Firm_Max_Productivity");                   				
V("Firm_Expected_Sales_Long_Term");                        
V("Firm_Desired_Expansion_Investment_Expenses");           

/*****FINANCIAL CONSTRAINTS*****/
V("Firm_Available_Financial_Assets");                      
V("Firm_Available_Debt");                                   
V("Firm_Total_Funds");                                       

/*****EFFECTIVE INVESTMENT*****/
V("Firm_Effective_Expansion_Investment_Expenses");           
V("Firm_Demand_Productive_Capacity_Expansion");             
V("Firm_Available_Funds_Replacement");                      
V("Firm_Demand_Productive_Capacity_Replacement");            
V("Firm_Replacement_Expenses");							  
V("Firm_Available_Funds_After_Replacement");				

/*****DEBT*****/
V("Firm_Debt_Flow");                                        
V("Firm_Financial_Assets_Flow");                            
V("Firm_Stock_Debt");                                        
V("Firm_Capital");                                           
V("Firm_Debt_Rate");                                        
V("Firm_Avg_Debt_Rate");							   	

/*****ENTRY & EXIT*****/
V("Sector_Productive_Capacity");
V("Sector_Avg_Quality");                         			
V("Sector_Max_Quality");                        			
V("Exit"); 													
V("Exit_Deposits_Distributed"); 							
V("Exit_Defaulted_Loans");    								                 			
V("Sector_Entry_Condition"); 								                         
V("Sector_Productive_Capacity_Exit");									
V("Sector_Productive_Capacity_Entry");										
V("Sector_Productive_Capacity_Available"); 					

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
V("GDP");                                            		
V("GDP_Demand");                                   	    	
V("Annual_Growth");                                     	
V("Price_Index");                                       	
V("Consumer_Price_Index");                              	
V("Real_GDP");                                          	
V("Annual_Real_Growth");                                	
V("Likelihood_Crisis");                                 	
V("Unemployment");                                      	

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




