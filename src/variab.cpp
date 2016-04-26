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
flag indentifying whether the variable has to be saved or not in the result file

- int plot;
Flag used to indicate variables that are plotted in the run time graph.

- char debug;
flag used to indicate the variables to debug. If this flag is equal 1, when the
simulation is run in debug mode it stops immediately after the computation of
its value.

- int deb_cond;
Like the flag debug, but it stops the simulation if the attached coondition is
satisfied. It does not require that the simulation is run in debug mode.
Its different values represent the different conditions for stopping: <, > or ==

- double deb_cnd_val;
numberical value used for the conditional stop

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
#include "choose.h"

#include "decl.h"
#include <time.h>
#include <exception>

#ifndef NO_WINDOW
 #include <tk.h>
extern Tcl_Interp *inter;
void cmd(Tcl_Interp *inter, char *cc);
int deb(object *r, object *c, char const *lab, double *res);
#endif
 


extern int t;
extern int debug_flag;
extern int when_debug;
extern int stackinfo_flag;
extern int quit;
extern int stack;
extern char msg[];
extern lsdstack *stacklog;
extern int total_var;
extern bool fast;


void set_lab_tit(variable *var);
void plog(char const *msg);
void error_hard(void);
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
if(label==NULL)
 plog("\nError. No more memory");
strcpy(label, _label);
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
savei=0;
under_computation=0;
deb_cond=0;
deb_cnd_val=0;
data_loaded='-';
plot=0;
data=NULL;
lab_tit=NULL;
//su=NULL;

return 0;
}


/***************************************************
CAL

****************************************************/

double variable::cal(object *caller, int lag)
{
int i;
double app;
if(param==1 ) //it is a parameter 
 {
  return val[0];
 }
if(num_lag<lag ) //check lag error
 {sprintf(msg, "\nLag error (during execution of equation for '%s'):\nvariable or function '%s' requested with lag=%d but declared with lag=%d\n. There two possible fixes:\n- change the model configuration, declaring '%s' with at least %d lags, or\n- change the code of '%s' requesting the value of '%s' with maximum %d lags.\n\n", stacklog->vs->label, label, lag, num_lag, label, num_lag, stacklog->vs->label, label, num_lag);
  plog(msg);
  error_hard();
  quit=2;
  return -1;
 }


if(param==0)
 {//variable
  if(lag > 0  || last_update==t )//lagged value or already computed
   return(val[last_update+lag-t]);
 
 }
else
 {//function
 if(lag > 0  )//lagged value
   return(val[lag-1]); 
 if(caller==NULL)
   return(val[0]);   
 } 



//value to be computed
stack++;

if(under_computation==1)
 {sprintf(msg, "\nDead lock! An equation requested its own value while computing its current value.\n\nEquation for:\n%s\nrequested by object:\n%s\n\n",label, caller==NULL?"(No label)":((object *)caller)->label);
  plog(msg);
  error_hard();
  quit=2;
  return -1;
 }

under_computation=1;
/****************/
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


if(stackinfo_flag>=stack)
 {
  start_profile[stack-1]=clock();
 }
/****************/

//Compute the Variable's equation
if(!fast)				// not running in fast mode?
{
	try 				// do it while catching exceptions to avoid obscure aborts
	{
		app=fun(caller);
	}
	catch(std::exception& exc)
	{
		sprintf(msg, "\nException! An exception of type:\n '%s'\n was detected while computing the equation for:\n %s\nrequested by object:\n %sn\n", exc.what(), label, caller==NULL?"(No label)":((object *)caller)->label);
		plog(msg);
		error_hard();
		quit=2;
		return -1;
	}
	catch(...)
	{
		sprintf(msg, "\nException! An unknown problem was detected while computing the equation for:\n %s\nrequested by object:\n %s\n\nPLEASE CLOSE LSD BEFORE CONTINUING!!!\n", label, caller==NULL?"(No label)":((object *)caller)->label);
		plog(msg);
		error_hard();
		quit=2;
		return -1;
	}
}
else
	app=fun(caller);	// or simply do it unsupervised

for(i=0; i<num_lag; i++) //scale down the past values
 val[num_lag-i]=val[num_lag-i-1];
val[0]=app;
if(stackinfo_flag>=stack)
 {end_profile[stack-1]=clock();
  set_lab_tit(this);
  if(caller==NULL)
    sprintf(msg, "\n%s (%s) = %g \t t = %d, stack = %d, %g secs, caller = SYSTEM", label, lab_tit, val[0], t, stack,(double)(( end_profile[stack-1] - start_profile[stack-1]) /(double)CLOCKS_PER_SEC) );
  else
    sprintf(msg, "\n%s (%s) = %g \t t = %d, stack = %d, %g secs, caller = %s, triggering var. = %s", label, lab_tit, val[0], t, stack, (double)(( end_profile[stack-1] - start_profile[stack-1]) /(double)CLOCKS_PER_SEC), caller->label, stacklog->prev->label);

  plog(msg);
 }

last_update++;
#ifndef NO_WINDOW
if(debug_flag==1 && debug=='d')
	 deb( (object *)up, caller, label, &val[0] );
else
 switch(deb_cond)
 {
  case 0: break;
  case 1: if(val[0]==deb_cnd_val)
			  deb( (object *)up, caller, label, &val[0] );
			 break;
  case 2: if(val[0]<deb_cnd_val)
			  deb( (object *)up,caller, label, &val[0] );
			 break;
  case 3: if(val[0]>deb_cnd_val)
			  deb( (object *)up,caller, label, &val[0] );
			 break;
  default:printf("\nError 12: conditional debug %d in variable %s\n", deb_cond, label);
			 exit(1);
			 break;
 }
#endif

/*****************/
stack --;
//Remove the element of the stack
stacklog=stacklog->prev;
if ( stacklog != NULL )
{
delete stacklog->next; //removed. The stack is maintained to avoid creation/destruction of memory. REINSERTED
stacklog->next=NULL; //REIINSERTED
}
/****************/
under_computation=0;

return(val[0]);
//by default the requested value is the last one, not yet computed
}

/*******
archaic system to speedup simulations
void delete_su(speedup *su)
{
if(su->next!=NULL)
 delete_su(su->next);
delete su;

 
}
*******/
/****************************************************
EMPTY
****************************************************/

void variable::empty(void)
{


if((data!=NULL && save!=true && savei !=true) || this==NULL || label==NULL)
 {sprintf(msg, "Error in emptying Variable %s\n", label);
  plog(msg);
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
/*
if(su!=NULL)
 {delete_su(su);
  su=NULL;
 } 
*/ 
}


/*********************
REMOVED
void add_stack_log(char *var, int nstack)
{
stacklog->next=new lsdstack;
strcpy(stacklog->next->label, var);
stacklog->next->ns=nstack;
//stacklog->next->vs=this;

stacklog->next->next=NULL;
stacklog->next->prev=stacklog;
stacklog=stacklog->next;
}

void rem_stack_log(void)
{
stacklog=stacklog->prev;
delete stacklog->next;
stacklog->next=NULL;
}
********************/
