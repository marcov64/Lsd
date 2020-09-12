/*****STOCK-FLOW CONSISTENCY*****/


EQUATION("Total_Demand_Loans_Classes")
	v[0]=SUM("Class_Demand_Loans");
RESULT(v[0])

EQUATION("Total_Deposits_Return_Classes")
	v[0]=SUM("Class_Deposits_Return");
RESULT(v[0])

EQUATION("Total_Interest_Payment_Classes")
	v[0]=SUM("Class_Interest_Payment");
RESULT(v[0])

EQUATION("Total_Debt_Payment_Classes")
	v[0]=SUM("Class_Debt_Payment");
RESULT(v[0])

EQUATION("Total_Stock_Loans_Classes")
	v[0]=SUM("Class_Stock_Loans");
RESULT(v[0])

EQUATION("Total_Stock_Deposits_Classes")
	v[0]=SUM("Class_Stock_Deposits");
RESULT(v[0])



