/***************************************************
****************************************************
LSD 2.0 - April 2000
written by Marco Valente
Aalborg University

Example for an equation file. Users should include in this header a
brief description of the model.

Include the equations in the space indicated below, after the line:

Place here your equations


****************************************************
****************************************************/

#include "../src/fun_head.h"

object *database;
double *values;

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


if(!strcmp(label,"set_param"))
{
/*
Update the parameter's values.
Sequentially, each parameter is added its step. If this cause to reach the limit
of the parameter, then it is set to its initial value, and the next parameter is
increased.

*/

v[0]=p->cal("a",0);
v[1]=p->cal("limit_a",0);

v[2]=p->cal("step_a",0);
v[4]=p->increment("a",v[2]);

//continue if the par continue to 
// stay larger/smaller than the limit 
if( (v[0]>v[1] && v[4]>=v[1]) || (v[0]<v[1] && v[4]<=v[1]) ) 
 { //continue varying a
  res=1;
  goto end;
 }

//'a' must be re-initialized

p->write("a",p->cal("init_a",0), 0);


v[10]=p->cal("b",0);
v[11]=p->cal("limit_b",0);

v[12]=p->cal("step_b",0);
v[14]=p->increment("b",v[12]);

//continue if the par continue to 
// stay larger/smaller than the limit 
if((v[10]>v[11] && v[14]>=v[11]) || (v[10]<v[11] && v[14]<=v[11]) ) 
 { //continue varying b
  res=2;
  goto end;
 }

p->write("b",p->cal("init_b",0), 0);



v[20]=p->cal("k",0);
v[21]=p->cal("limit_k",0);
v[22]=p->cal("step_k",0);
v[24]=p->increment("k",v[22]);

//continue if the par continue to 
// stay larger/smaller than the limit 
if((v[20]>v[21] && v[24]>=v[21]) || (v[20]<v[21] && v[24]<=v[21]) ) 
 { //continue varying k
  res=3;
  goto end;
 }

res=4;
goto end;
}


if(!strcmp(label,"cum_error"))
{
/*
Compute the fitness error with the current parameter settings.

Here the function must be placed, in this case it is:
f(t)=k*exp(t*a)/[b+exp(t*a)]

Note that it is possible to apply different error methods
*/
p->cal("set_param",0);
v[10]=p->cal("a",0);
v[11]=p->cal("k",0);
v[12]=p->cal("b",0);
cur=database->son;
v[13]=database->cal("num_years",0);
for(v[5]=0,v[0]=1; v[0]<=v[13]; v[0]++)
 {
 v[4]=exp(v[10]*v[0]);
 v[2]=v[11]*(v[4]/(v[12]+v[4])); //here is the function. Change here for a different fitting function
 v[1]=values[(int)v[0]-1]; //this is the equivalent true value for year v[0]. See init
// v[5]+=(v[1]-v[2])*(v[1]-v[2]);  //absolute error
 v[5]+=(v[1]-v[2])*(v[1]-v[2])/v[1]; //relative error

 } 
res=v[5]; 
goto end;
}


if(!strcmp(label,"store"))
{
/*
If the cumulated error is the minimum found so far, then record the relevant data:
- parameters' values
- estimations

Of course, the value for the minimum error is replaced if necessary.
*/


v[0]=p->cal("min_error",0);
v[1]=p->cal("cum_error",0);
if(v[0]>v[1] || t==1)
 {
  v[2]=p->cal("a",0);
  v[3]=p->cal("k",0);
  v[4]=p->cal("b",0);
  p->write("best_a",v[2], 0);
  p->write("best_k",v[3], 0);
  p->write("best_b",v[4], 0);
  p->write("min_error",v[1], 0);
  
   cur=database->son;
   v[14]=database->cal("num_years",0);
  for(v[5]=1; v[5]<=v[14]; v[5]++)
   {
   v[8]=exp(v[2]*v[5]);
   v[9]=v[3]*(v[8]/(v[4]+v[8]));
   
   cur->write("best_value",v[9],0);
   cur->write("err_best-true",v[9]-values[(int)v[5]-1],0);
   cur=cur->next;
   } 
  res=1;
 }
else 
  res=0;
goto end;
}

if(!strcmp(label,"init"))
{
/*
This equation is computed only at the very beginning of the simulation and never again.
Since the simulation speed and the initialization consistency is crucial, then this equation
perform the necessary actions.

- set the value for num_years equal to the actual number of Objects "Years". 
- Store the true values in a global vector to speed up their retrieval
- allocate the number of attempts for each parameter. The same number of attempts is 
assigned to each parameter, computing the steps by equally dividing the research area from init_x
up to limit_x.

*/
database=p->search("Database");
for(v[0]=0,cur=database->search("Years"); cur!=NULL; cur=go_brother(cur),v[0]++ );
database->write("num_years",v[0], 0);
values=new double[(int)v[0]]; //assign memory to 'values', deleted in close_sim()
for(v[0]=0,cur=database->search("Years"); cur!=NULL; cur=go_brother(cur),v[0]++ )
 {
  v[1]=cur->cal("true_value",0);
  values[(int)v[0]]=v[1];
 }

v[0]=(double)max_step; //hack. This is the Lsd private variable to store the number of steps for the simulation.

cur=p->search("Obj");

v[3]=cur->cal("init_a",0);
v[4]=cur->cal("limit_a",0);
v[5]=cur->cal("init_b",0);
v[6]=cur->cal("limit_b",0);
v[7]=cur->cal("init_k",0);
v[8]=cur->cal("limit_k",0);

v[9]=(double)((v[3]!=v[4])+(v[5]!=v[6])+(v[7]!=v[8])); //# of par's to vary
v[10]=1/v[9];
v[1]=pow(v[0],v[10]); //the number of steps for each parameter is the cube root of max_step


cur->write("a",v[3], 0);
v[11]=(v[4]-v[3])/v[1];
cur->write("step_a",v[11], 0);

cur->write("b",v[5], 0);
v[12]=(v[6]-v[5])/v[1];
cur->write("step_b",v[12], 0);

cur->write("k",v[7], 0);
v[13]=(v[8]-v[7])/v[1];
cur->write("step_k",v[13], 0);

res=1; //whatever
param=1; //never compute again this equation
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
if( (isnan(res)==1 || isinf(res)==1) && quit!=1)
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
delete values;
}












