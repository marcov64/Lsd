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


/***************************************************

Example for an equation file. Users should include in this header a
brief description of the model.

Include the equations in the space indicated below, after the line:

Place here your equations


****************************************************
****************************************************/

#include "fun_head.h"

double variable::fun(object *caller)
{
//These are the local variables used by default
double v[40], res;
object *p, *c, *cur1, *cur2, *cur3, *cur4, *cur5;

//Declare here any other local variable to be used in your equations
//You may need an integer to be used as a counter
int i, j;
//and an object (a pointer to)
register object *cur;


if(quit==2)
 return -1;

p=up;
c=caller;
FILE *f;


//Uncommenting the following lines the file "log.log" will
//contain the name of the variable just computed.
//To be used in case of unexpected crashes. It slows down sensibly the simulation
/**
f=fopen("log.log","a");
 fprintf(f,"t=%d %s\n",t,label);
 fclose(f);
**/

//Place here your equations. They must be blocks of the following type

if(!strcmp(label, "VarX"))
{
/*
comment the equation
*/
res=v[0]; //final result for Variable VarX at the generic time step t
goto end;
}


/*********************

Do not place equations beyond this point.

*********************/

sprintf(msg, "\nEquation for %s not found", label);
plog(msg);
quit=2;
return -1;


end :


if( ((!use_nan && isnan(res)) || isinf(res)==1) && quit!=1)
 { 
  sprintf(msg, "At time %d the equation for '%s' produces the non-valid value '%lf'. Check the equation code and the temporary values v\\[...\\] to find the faulty line.",t, label, res );
  error(msg);

  debug_flag=1;
  debug='d';
 } 

if(debug_flag==1)
 {
 for(i=0; i<40; i++)
  i_values[i]=v[i];
 }

return(res);
}

/*
This function is executed once at the end of a simulation run. It may be used
to perform some cleanup, in case the model allocated memory during the simulation.
*/
void close_sim(void)
{

}


