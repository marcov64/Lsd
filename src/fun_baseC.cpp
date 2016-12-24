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

double variable::fun( object *caller )
{
//Don't proceed if simulation is stopping
if ( quit == 2 )
	return -1;

//These are the local variables used by default
double v[ 1000 ], res = 0;
object *p = up, *c = caller;
object *cur, *cur1, *cur2, *cur3, *cur4, *cur5, *cur6, *cur7, *cur8, *cur9, *cur10;
cur = cur1 = cur2 = cur3 = cur4 = cur5 = cur6 = cur7 = cur8 = cur9 = cur10 = NULL;
netLink *curl, *curl1, *curl2, *curl3, *curl4, *curl5;
curl = curl1 = curl2 = curl3 = curl4 = curl5 = NULL;
FILE *f = NULL;

//Declare here any other local variable to be used in your equations
//You may need an integer to be used as a counter
int i, j, h, k;

//Uncommenting the following lines the file "log.log" will
//contain the name of the variable just computed.
//To be used in case of unexpected crashes. It slows down sensibly the simulation
/**
f = fopen( "log.log", "a" );
fprintf( f, "t=%d %s\n", t, label );
fclose( f );
**/

//Place here your equations. They must be blocks of the following type


if ( ! strcmp( label, "VarX" ) )
{
/*
comment the equation
*/
res = v[0]; //final result for Variable VarX at the generic time step t
goto end;
}


/*********************

Do not place equations beyond this point.

*********************/

sprintf( msg, "equation not found for variable '%s'\nPossible problems:\n- There is no equation for variable '%s';\n- The spelling in equation's code is different from the name in the configuration;\n- The equation's code was terminated incorrectly", label, label );
error_hard( msg, "Equation not found", "Check your configuration or code to prevent this situation." );
return -1;

end :

if ( ( quit == 0 && ( ( ! use_nan && is_nan( res ) ) || is_inf( res ) ) ) )
 {
  sprintf( msg, "at time %d the equation for '%s' produces the invalid value '%lf',\ncheck the equation code and the temporary values v\\[...\\] to find the faulty line.\nLsd debugger will open next.", t, label, res );
  error_hard( msg, "Invalid result", "Check your code to prevent this situation." );
  debug_flag = true;
  debug = 'd';
 }
if ( debug_flag )
 {
 for ( i = 0; i < 1000; i++ )
  i_values[i] = v[i];
 }
return res;
}

/*
This function is executed once at the end of a simulation run. It may be used
to perform some cleanup, in case the model allocated memory during the simulation.
*/
void close_sim( void )
{

}


