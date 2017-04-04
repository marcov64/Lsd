/***************************************************
****************************************************
LSD 7.0 - August 2015
written by Marco Valente
Universita' dell'Aquila

Copyright Marco Valente
Lsd is distributed according to the GNU Public License

Comments and bug reports to marco.valente@univaq.it
****************************************************
****************************************************/


/****************************************************
VARIAB.CPP
The (C++) object variable is devoted to contain numerical values
of the model. Only double precision floating point numbers are considered
in Lsd.
Variables are mainly storages for information. Actually, all the work is
done by lsd objects.
The most important task of variables is to return their value when requested.
It is done by comparing the global time of the simulation with the time
the variable was most recently updated. If the value requested (considering the
lag) is already available, that is returned. Otherwise, the variable shifts
its lagged values, and calls its equation to compute the new value.
All fields and functions in variables are public, so that users may override
the default mechanism.
The fields composing a variables are

- char *label;
name of the variable. It needs to be unique in the model

- object *up;
address of the object containing the variable

- variable *next;
pointer to the next variable contained in the object. Variables in an object
are organized as a linked chain, and can be reached only  via their fields
next

- double *val;
vector of numerical values. val[0] is the most recent value computed by the
equation, that is, computed at time last_update. val[1] is the value computed
at time last_update - 1; val[2] at time last_update - 2 and so on.

- int num_lag;
number of lagged values stored for the variable

- int save;
flag identifying whether the variable has to be saved or not in the result file

- int plot;
Flag used to indicate variables that are plotted in the run time graph.

- char debug;
flag used to indicate the variables to debug. If this flag is equal 'd', when the
simulation is run in debug mode it stops immediately after the computation of
its value.

- int deb_cond;
Like the flag debug, but it stops the simulation if the attached coondition is
satisfied. It does not require that the simulation is run in debug mode.
Its different values represent the different conditions for stopping: <, > or ==

- double deb_cnd_val;
numerical value used for the conditional stop

- int under_computation;
control flag used to avoid infinite recursion of an equation calling itself.
Used to issue a message of error
- int last_update;
contain the global time when it was lastly computed the equation for the variable

- int param;
Flag set to 1, in case the variable is considered a parameter. In case it is,
when requested the value it is always returned its field val[0].

- char data_loaded;
flag indicatibe whether the variable has been initiliazed with numerical values
set as default by the system or if they were actually chosen by the user.
The flag is 0 in case of newly created objects and 1 in case the variable's
values has been at least shown once in the initial values editor window.
This flag is also saved in the data file, so thatthis information is not lost.
The flag prevents to run a simulation if the data where not confirmed by users.

- char computable;
Flag not currently used.



The methods of the (C++) object variable are:

- int init(object *_up, char *_label, int _num_lag, double *val, int _save);
perform the initialization.

- double cal(object *caller, int lag);
it is its main function. Return the numerical value

       val[last_update+lag-t]

if the condition

       t-lag<=last_update

is satisfied. That means that either the variable has already been updated,
and therefore the requested value is available, or that, though the variable
has not been still updated in the time step, the value requested is a lagged one
and therefore can be retrived from the vector of the past values.

Only in case the lag requested is zero and the variable has not been computed
at the present time step, the method shifts its lagged values and calls the method
fun
that perform the equation computation.

- void empty(void);
It is used to free all the memory assigned to the variable. Used by
object::delete_obj to cancel an object.

In the file FUNXXX.CPP there is the code for the last method of variable.
This is the only file that is model specific, since it contains the equations
for the variables. The names are normally of the form FUNXXX.CPP, where XXX
is a code for the different models.

- double fun(object *caller);
It is a method common to all the variables and stores the code for the equations.
Each equation needs to be defined as a block like:

if(!strcmp(label, "LabelOfTheVariable"))
{
... here any code
res=avalue;
goto end;
}

The method is common to all the variables, so that the blocks ensure that
only their piece of code is actually executed. Any code legal in C++ is allowed,
including the methods and function provided with Lsd.
The value assigned to "res" is assigned as val[0] to the variable.
The final line goto end; ensures that the equation has been computed.
See file FUNXXX.CPP for more information on this

Functions used here from other files are:

- void plog(char *m);
LSDMAIN.CPP print  message string m in the Log screen.

- int deb(object *r, object *c, char *lab, double *res);
DEBUG.CPP
activate the debugger.

****************************************************/

#include "decl.h"

clock_t start_profile[100], end_profile[100];

/****************************************************
INIT
****************************************************/
int variable::init(object *_up, char const *_label, int _num_lag, double *v, int _save)
{
int i;

total_var++;

up=_up;
label=NULL;
i=strlen(_label)+1;
label=new char[i];
strcpy(label, _label);
	param = 0;
num_lag=_num_lag;
if ( num_lag >= 0 )
{
	val = new double[ num_lag + 1 ];
	for( i = 0; i < num_lag + 1; i++ )
		val[ i ] = v[ i ];
}
else
	val = NULL;
next=NULL;
last_update=0;
save=_save;
savei = false;
under_computation = false;
deb_cond=0;
deb_cnd_val=0;
data_loaded='-';
plot = false;
data=NULL;
lab_tit=NULL;

return 0;
}


/***************************************************
CAL
****************************************************/
double variable::cal( object *caller, int lag )
{
int i, eff_lag;
double app;

if ( param == 1 ) //it is a parameter, ignore lags
	return val[ 0 ];

// effective lag for variables (compatible with older versions)
eff_lag = ( last_update < t ) ? lag - 1 : lag;

// check lag error
if ( lag != 0 )
{	
	bool lag_ok = true;
	
	if ( param == 0 )		// it's a variable
	{
		if ( eff_lag < 0 )	// with negative lag
			lag_ok = false;

		if ( ! ( save || savei ) && eff_lag > num_lag )	// not saved with too large lag
			lag_ok = false;

		if ( ( save || savei ) && eff_lag > num_lag && lag > t - start )	// saved with too large lag
			lag_ok = false;
	}
	else					// function
		if ( lag < 0 || lag > num_lag )	// with invalid lag
			lag_ok = false;

	if ( ! lag_ok )
	{
		sprintf( msg, "in object '%s' variable or function '%s' requested \nwith lag=%d but declared with lag=%d\nThree possible fixes:\n- change the model configuration, declaring '%s' with at least lag=%d,\n- change the code of '%s' requesting the value of '%s' with lag=%d maximum, or\n- mark '%s' to be saved (variables only)", up->label, label, lag, num_lag, label, lag, caller == NULL ? "(no label)" : caller->label, label, num_lag, label );
		error_hard( msg, "Lag error", "Check your configuration or code to prevent this situation." );
	}
}

if ( param == 0 )
{	//variable
	if ( lag > 0 || last_update >= t )	// lagged value or already computed
	{
		if ( eff_lag <= num_lag )
			return val[ eff_lag ];
		else
			if ( save || savei )
				return data[ t - lag ];	// use saved data
			else
				return val[ 0 ];		// ignore lag request
	}
}
else
{	//function
	if ( lag > 0  )						// lagged value
		return val[ lag - 1 ]; 

	if ( caller == NULL )
		return val[ 0 ];   
 } 

//value to be computed
stack++;

if ( under_computation )
{
	sprintf( msg, "the equation for '%s' (in object '%s') requested \nits own value while computing its current value", label, caller == NULL ? "(no label)" : ( ( object * ) caller )->label );
	error_hard( msg, "Dead lock", "Check your code to prevent this situation." );
}

under_computation = true;

//Add the Variable to the stack
if(stacklog->next==NULL)
{
 stacklog->next=new lsdstack;
 stacklog->next->next=NULL;
} 

strcpy(stacklog->next->label, label);
stacklog->next->ns=stack;
stacklog->next->vs=this;

stacklog->next->prev=stacklog;
stacklog=stacklog->next;

#ifndef NO_WINDOW
if(stackinfo_flag>=stack)
  start_profile[stack-1]=clock();
#endif

//Compute the Variable's equation
user_exception = true;			// allow distinguishing among internal & user exceptions
try 							// do it while catching exceptions to avoid obscure aborts
{
	app = fun( caller );
}
catch ( std::exception& exc )
{
	plog( "\n\nAn exception was detected while computing the equation \nfor '%s' requested by object '%s'", "", label, caller == NULL ? "(no label)" : ( ( object * ) caller )->label );
	quit = 2;
	throw;
}
catch ( ... )
{
	if ( quit != 2 )			// error message not already presented?
	{
		plog( "\n\nAn unknown problem was detected while computing the equation \nfor '%s' requested by object '%s'", "", label, caller == NULL ? "(no label)" : ( ( object * ) caller )->label );
		quit = 2;
		throw;
	}
	else
	{
		app = NAN;				// mark result as invalid
		use_nan = true;			// and allow propagation
	}
}
user_exception = false;

for ( i = 0; i < num_lag; ++i ) 	//scale down the past values
	val[ num_lag - i ] = val[ num_lag - i - 1 ];
val[ 0 ] = app;

last_update++;

#ifndef NO_WINDOW
if ( stackinfo_flag >= stack )
 {
  end_profile[stack-1]=clock();
  set_lab_tit(this);
  if(caller==NULL)
    plog( "\n%s (%s) = %6g,\t t = %d, stack = %d, %4g secs, caller = SYSTEM", "", label, lab_tit, val[0], t, stack, ( end_profile[stack-1] - start_profile[stack-1] ) / (double) CLOCKS_PER_SEC );
  else
    plog( "\n%s (%s) = %6g,\t t = %d, stack = %d, %4g secs, caller = %s, trigger var. = %s", "", label, lab_tit, val[0], t, stack, ( end_profile[stack-1] - start_profile[stack-1] ) / (double) CLOCKS_PER_SEC, caller->label, stacklog->prev->label );
 }

if ( debug_flag && debug == 'd' && deb_cond == 0 )
 deb( (object *)up, caller, label, &val[0] );
else
 switch(deb_cond)
 {
  case 0: break;
  case 1: if(val[0]==deb_cnd_val)
			  deb( (object *)up, caller, label, &val[0] );
			 break;
  case 2: if(val[0]>deb_cnd_val)
			  deb( (object *)up,caller, label, &val[0] );
			 break;
  case 3: if(val[0]<deb_cnd_val)
			  deb( (object *)up,caller, label, &val[0] );
			 break;
  default:
			sprintf( msg, "conditional debug '%d' in variable '%s'", deb_cond, label );
			error_hard( msg, "Internal error", "If error persists, please contact developers." );
			return -1;
 }
#endif

//Remove the element of the stack
stack--;
stacklog = stacklog->prev;
if ( stacklog != NULL )
{
	delete stacklog->next; //removed. The stack is maintained to avoid creation/destruction of memory. REINSERTED
	stacklog->next = NULL; //REINSERTED
}

under_computation = false;

return val[ 0 ];
//by default the requested value is the last one, not yet computed
}


/****************************************************
EMPTY
****************************************************/
void variable::empty(void)
{

if ( ( data != NULL && save != true && savei != true ) || label == NULL )
 {
  sprintf(msg, "failure in emptying Variable %s", label);
  error_hard( msg, "Invalid pointer", "Check your code to prevent this situation." );
  return;
 }

total_var--;
delete[] label;
if(data!=NULL)
  delete[] data;
if(lab_tit!=NULL)
  delete[] lab_tit;
if( val != NULL )
	delete[] val;
}
