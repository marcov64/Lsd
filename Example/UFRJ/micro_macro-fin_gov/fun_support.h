//**********C++ SUPPORT FUNCTIONS*********//


/*
ROUND(value, "direction")
This MACRO returns the rouded value of a value specified by the user. 
The user can specify the direction wih the word UP and "DOWN", in quotes and capital letters, in which case the MACRO will round up or down, respectively.
If any other word or no word is specified, the MACRO will simply round the value.
*/
double ROUND( double x , string d = "none")
{
	double r = round(x);
	double y;
	if(d=="UP")
		y = r>x? r: r+1;
	else if(d=="DOWN")
		y = r<x? r: r-1;
	else
		y = r;
	return y;
}


/*
LAG_SUM(obj, "lab", lag1, lag2)
This MACRO returns the sum of lagged values of a specifed variable named "lab". 
The first lag defines how many lags to sum. The secong lag defines from which lag it will start summing. By default, the second lag is 1.
WARNING: make sure there are specified lagged values for the variable "lab".
EXAMPLE 1: LAG_SUM(p, "X", 4) will return VL("X",0) + VL("X",1) + VL("X",2) + VL("X",3).
EXAMPLE 2: LAG_SUM(p, "X", 3, 2) will return VL("X",2) + VL("X",3) + VL("X",4).
*/
double LAG_SUM( object *obj , const char *var , int lag = 0, int lag2 = 0)
{
	double x = 0;
	int i;
	for(i=lag2; i<=lag2+lag-1; i++)
		x = x + VLS( obj, var, i);
	return x;
}

/*
LAG_AVE(obj, "lab", lag1, lag2)
Same as LAG_SUM but this MACRO returns the average for the lag1 periods.
WARNING: make sure there are specified lagged values for the variable "lab".
EXAMPLE 1: LAG_AVE(p, "X", 4) will return (VL("X",0) + VL("X",1) + VL("X",2) + VL("X",3))/4.
EXAMPLE 2: LAG_AVE(p, "X", 3, 2) will return (VL("X",2) + VL("X",3) + VL("X",4))/3.
*/
double LAG_AVE( object *obj , const char *var , int lag = 0, int lag2 = 0)
{
	double x = 0;
	int i;
	for(i=lag2; i<=lag2+lag-1; i++)
		x = x + VLS( obj, var, i);
	return x/lag;
}

/*
LAG_GROWTH(obj, "lab", lag1, lag2)
This MACRO returns the growth rate of a variable named "lab" for the lag1 periods.
The first lag defines how many lags to sum. The secong lag defines from which lag it will start summing. By default, the second lag is 0.
WARNING: make sure there are specified lagged values for the variable "lab".
EXAMPLE 1: LAG_GROWTH(p, "X", 4) will return (V("X") - VL("X",4))/VL("X",4).
EXAMPLE 2: LAG_GROWTH(p, "X", 3, 2) will return (VL("X",2) - VL("X",5))/VL("X",5).
*/
double LAG_GROWTH( object *obj , const char *var , int lag = 0, int lag2 = 0)
{
	double x = VLS( obj, var, lag2);
	double y = VLS( obj, var, lag2+lag);
	double r = y!=0? (x-y)/y : 0;
	return r;
}

