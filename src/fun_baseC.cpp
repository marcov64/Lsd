/***************************************************
Example for an equation file. Users should include 
in this header a brief description of the model.

Include the equations in the space indicated below, 
after the indicated line.
****************************************************/

//#define EIGENLIB				// uncomment to use Eigen linear algebra library

#include "fun_head.h"

double variable::fun( object *caller )
{
// Don't proceed if simulation is stopping
if ( quit == 2 )
	return -1;

// These are the local variables used by default
double v[ USER_D_VARS ], res = 0;
object *p = up, *c = caller;
object *cur, *cur1, *cur2, *cur3, *cur4, *cur5, *cur6, *cur7, *cur8, *cur9;
cur = cur1 = cur2 = cur3 = cur4 = cur5 = cur6 = cur7 = cur8 = cur9 = NULL;
netLink *curl, *curl1, *curl2, *curl3, *curl4, *curl5, *curl6, *curl7, *curl8, *curl9;
curl = curl1 = curl2 = curl3 = curl4 = curl5 = curl6 = curl7 = curl8 = curl9 = NULL;
FILE *f = NULL;

// Declare here any other local variable to be used in your equations
// You may need an integer to be used as a counter
int i, j, h, k;

// Place here your equations. They must be blocks of the following type:


if ( ! strcmp( label, "VarX" ) )
{
	/*
	comment the equation
	*/
	res = v[0]; // final result for Variable VarX at the generic time step t
	goto end;
}


// Do not place equations beyond this point

sprintf( msg, "equation not found for variable '%s'", label );
error_hard( msg, "equation not found", "check your configuration (variable name) or\ncode (equation name) to prevent this situation\nPossible problems:\n- There is no equation for this variable\n- The equation name is different from the variable name (case matters!)" );
return -1;

end :

if ( ( quit == 0 && ( ( ! use_nan && is_nan( res ) ) || is_inf( res ) ) ) )
{
	sprintf( msg, "equation for '%s' produces the invalid value '%lf' at time %d", label, res, t );
	error_hard( msg, "invalid equation result", "check your equation code to prevent invalid math operations\nPossible problems:\n- Illegal math operation (division by zero, log of negative number etc.)\n- Use of too-large/small value in calculation\n- Use of non-initialized temporary variable in calculation", true );
	debug_flag = true;
	debug = 'd';
}

// Uncommenting the following lines the file "log.txt" will contain the name of 
// the variable just computed. To be used in case of unexpected crashes. 
// It slows down sensibly the simulation
/**
f = fopen( "log.txt", "a" );
fprintf( f, "%s\t= %g\t(t=%d)\n", label, res, t );
fclose( f );
**/

#ifndef NO_WINDOW
if ( debug_flag )
{
		for ( int n = 0; n < USER_D_VARS; ++n )
			d_values[ n ] = v[ n ];
		
		i_values[ 0 ] = i;
		i_values[ 1 ] = j;
		i_values[ 2 ] = h;
		i_values[ 3 ] = k;
		o_values[ 0 ] = cur;
		o_values[ 1 ] = cur1;
		o_values[ 2 ] = cur2;
		o_values[ 3 ] = cur3;
		o_values[ 4 ] = cur4;
		o_values[ 5 ] = cur5;
		o_values[ 6 ] = cur6;
		o_values[ 7 ] = cur7;
		o_values[ 8 ] = cur8;
		o_values[ 9 ] = cur9;
		n_values[ 0 ] = curl;
		n_values[ 1 ] = curl1;
		n_values[ 2 ] = curl2;
		n_values[ 3 ] = curl3;
		n_values[ 4 ] = curl4;
		n_values[ 5 ] = curl5;
		n_values[ 6 ] = curl6;
		n_values[ 7 ] = curl7;
		n_values[ 8 ] = curl8;
		n_values[ 9 ] = curl9;
}
#endif

return res;
}


// This function is executed once at the end of a simulation run. It may be used
// to perform some cleanup, in case the model allocated memory during the simulation.
void close_sim( void )
{

}
