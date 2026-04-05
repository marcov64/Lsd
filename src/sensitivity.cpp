/*************************************************************

	LSD 9.0 - January 2026
	written by Marco Valente, Universita' dell'Aquila
	and by Marcelo Pereira, University of Campinas

	Copyright Marco Valente and Marcelo Pereira
	LSD is distributed under the GNU General Public License

	See Readme.txt for copyright information of
	third parties' code used in LSD

 *************************************************************/

/*************************************************************
 SENSITIVITY.CPP
 Contains the methods and functions to work with sensitivity
 analysis. The remaining basic sensitivity-oriented methods
 and functions are stored in FILELIB.CPP.
 *************************************************************/

#include "LSD.h"
#include "nolhtables.h"

namespace gui
{
	int **NOLH_0 = NULL;				// pointer to the design loaded from file
}


/*************************************************************
 DATAENTRY_SENSITIVITY
 Get values for sensitivity analysis
 *************************************************************/
int lsd::sensitivity::dataentry( void )
{
	bool int_var = false;
	char *sss = NULL, *tok = NULL, type;
	const char *app;
	double temp, start, end;
	int i, j, res, nPar, samples;
	variable *cv = gui::sim.root->search_var( NULL, label, true );

	if ( cv != NULL && cv->attr->integer )
		int_var = true;

	cmd( "set integer %d", int_var ? 1 : integer );

	cmd( "set sens .sens" );
	cmd( "newtop .sens \"Sensitivity Analysis\" { set choice 2 }" );

	cmd( "ttk::frame .sens.lab" );
	cmd( "ttk::label .sens.lab.l1 -text \"Enter the desired values (at least 2) for:\"" );

	cmd( "ttk::label .sens.lab.l2 -style hl.TLabel -text \"%s\"", label );
	cmd( "pack .sens.lab.l1 .sens.lab.l2 -side left -padx $_2" );

	cmd( "ttk::label .sens.obs1 -text \"Paste of clipboard data is allowed, most separators are accepted\"" );
	cmd( "ttk::label .sens.obs2 -text \"Use a \'=BEGIN:END@SAMPLES%%TYPE\' clause\nto specify a number of samples within a range.\nSpaces are not allowed within clauses.\nTYPE values are \'L\' for linear and \'R\' for random samples.\" -justify center" );
	cmd( "pack .sens.lab .sens.obs1 .sens.obs2 -pady $_5" );

	cmd( "ttk::frame .sens.t" );
	cmd( "ttk::scrollbar .sens.t.v_scroll -command \".sens.t.t yview\"" );
	cmd( "ttk::text .sens.t.t -height 8 -width 50 -yscroll \".sens.t.v_scroll set\" -dark $darkTheme -style smallFixed.TText" );
	cmd( "pack .sens.t.t .sens.t.v_scroll -side left -fill y" );
	cmd( "mouse_wheel .sens.t.t" );
	cmd( "pack .sens.t" );

	cmd( "ttk::frame .sens.pad" );
	cmd( "pack .sens.pad -pady $_5" );

	cmd( "ttk::frame .sens.fb" );
	cmd( "ttk::checkbutton .sens.fb.int -variable integer -text \"Round to integer\"" );
	cmd( "ttk::button .sens.fb.paste -width $butWid -text Paste -command { tk_textPaste .sens.t.t }" );
	cmd( "ttk::button .sens.fb.del -width $butWid -text Delete -command { .sens.t.t delete 0.0 end }" );
	cmd( "ttk::button .sens.fb.rem -width $butWid -text Remove -command { set choice 3 }" );
	cmd( "pack .sens.fb.int .sens.fb.paste .sens.fb.del .sens.fb.rem -padx $butSpc -side left" );
	cmd( "pack .sens.fb -padx $butPad -anchor e" );

	cmd( "tooltip::tooltip .sens.fb.int \"Force rounding to integer values\"" );
	cmd( "tooltip::tooltip .sens.fb.paste \"Insert the content of clipboard\"" );
	cmd( "tooltip::tooltip .sens.fb.del \"Delete all current values\"" );
	cmd( "tooltip::tooltip .sens.fb.rem \"Remove variable from sensitivity analysis\"" );

	if ( int_var )
		cmd( ".sens.fb.int configure -state disabled" );

	cmd( "okhelpcancel .sens fb2 { set choice 1 } { LsdHelp menudata_sa.html#entry } { set choice 2 }" );
	cmd( "bind .sens.fb2.ok <KeyPress-Return> { set choice 1 }" );

	cmd( "showtop .sens topleftW" );
	cmd( "mousewarpto .sens.fb2.ok 0" );

	sss = new char [ MAX_ELEM_LENGTH * num_val + 1 ];// allocate space for string
	tok = new char [ MAX_ELEM_LENGTH ];
	strcpy( sss, "" );
	for ( i = 0; i < num_val; i++ )			// pass existing data as a string
	{
		snprintf( tok, MAX_ELEM_LENGTH, "%.15g ", val[ i ] );// add each value
		strcatn( sss, tok, MAX_ELEM_LENGTH * num_val + 1 );// to the string
	}

	cmd( "set sss \"%s\"", sss );			// pass string to Tk window
	cmd( ".sens.t.t insert 0.0 $sss" );		// insert string in entry window
	delete [ ] tok;
	delete [ ] sss;

	cmd( "focus .sens.t.t" );

	// reset random number generator to make random numbers reproducible
	gui::gui_prng.seed( gui::sim.seed );

	gui::choice = 0;

	do										// finish only after reading all values
	{
		while ( gui::choice == 0 )
			Tcl_DoOneEvent( 0 );

		if ( gui::choice == 2 )
		{
			res = num_val > 1 ? 1 : 2;
			goto end;
		}

		if ( gui::choice == 3 )
		{
			res = 2;
			goto end;
		}

		integer = gui::get_bool( "integer" );
		app = gui::eval_str( "[ .sens.t.t get 0.0 end ]" );
		sss = new char[ strlen( app ) + 1 ];
		strcpy( sss, app );

		char *tss, *ss = new char[ strlen( sss ) + 1 ];
		tss = ss;						// save original pointer to gc
		strcpy( ss, sss );				// make a draft copy

		i = 0;							// count number of values
		do
		{
			tok = strtok( ss, SENS_SEP );	// accepts several separators
			if ( tok == NULL )			// finished?
				break;

			ss = NULL;

			// is it a clause to be expanded?
			nPar = sscanf( tok, "=%lf:%lf@%u%%%c", &start, &end, &samples, &type );
			if ( nPar == 4 )			// all values are required
				i += samples;			// samples to create
			else						// no, read as regular double float
				i += sscanf( tok, "%lf", &temp );	// count valid doubles only
		}
		while ( tok != NULL );

		if ( i < 2 )					// invalid number of elements?
			i = 2;						// minimum is 2

		if ( num_val != i )				// change in space alloc'd?
		{
			delete [ ] val;				// free old and reallocate enough space
			val = new double[ i ];
			num_val = i;				// update # of values
		}

		delete [ ] tss;

		for ( i = 0; i < num_val; )
		{
			tok = strtok( sss, SENS_SEP );	// accepts several separators
			if ( tok == NULL )				// finished too early?
			{
				cmd( "ttk::messageBox -parent .sens -title \"Sensitivity Analysis\" -icon error -type ok -message \"Invalid or less than required values\" -detail \"Decimal numbers must use the point ('.') as the decimal separator. Insert the correct number of values.\"" );
				gui::choice = 0;
				cmd( "focus .sens.t.t" );
				break;
			}

			sss = NULL;

			// is it a clause to be expanded?
			nPar = sscanf( tok, "=%lf:%lf@%u%%%c", &start, &end, &samples, &type );

			if ( nPar == 4 )				// all values are required
			{
				if ( toupper( type ) == 'L' && samples > 0 )// linear sampling
				{
					val[ i++ ] = integer ? std::round( std::fmin( start, end ) ) : std::fmin( start, end );
					for ( int j = 1; j < samples; ++j, ++i )
					{
						val[ i ] = val[ i - 1 ] + ( std::fmax( start, end ) - std::fmin( start, end ) ) / ( samples - 1 );
						val[ i ] = integer ? std::round( val[ i ] ) : val[ i ];
					}
				}

				if ( toupper( type ) == 'R' && samples > 0 )// random sampling
					for ( int j = 0; j < samples; ++j, ++i )
					{
						val[ i ] = std::fmin( start, end ) + gui::ran1( ) * ( std::fmax( start, end ) - std::fmin( start, end ) );
						val[ i ] = integer ? std::round( val[ i ] ) : val[ i ];
					}
			}
			else							// no, read as regular double float
			{
				j = i;
				i += sscanf( tok, "%lf", &( val[ i ] ) );// count valid doubles only
				val[ j ] = integer ? std::round( val[ j ] ) : val[ j ];
			}
		}
	}
	while ( tok == NULL || i < 2 );	// require enough values (if more, extra ones are discarded)

	res = 0;

	end:

	cmd( "destroytop .sens" );

	return res;
}


/*************************************************************
 NUM_SENSITIVITY_POINTS
 Calculate the sensitivity space size
 *************************************************************/
long lsd::simulation::num_sensitivity_points( void )
{
	long nv;
	sensitivity *cs;

	for ( nv = 1, cs = sens; cs != NULL; cs = cs->next )
		nv *= cs->num_val;					// update the number of variables

	return nv;
}


/*************************************************************
 NUM_SENSITIVITY_VARIABLES
 Calculate the number of variables to test
 *************************************************************/
int lsd::simulation::num_sensitivity_variables( void )
{
	int nv;
	sensitivity *cs;

	for ( nv = 0, cs = sens; cs != NULL; cs = cs->next)
		if ( cs->num_val > 1 )				// count variables with 2 or more values
			nv++;

	return nv;
}


/*************************************************************
 SENSITIVITY_PARALLEL
 This function fills the initial values according to the sensitivity analysis
 system performed by parallel simulations: 1 single run over many independent
 configurations descending in parallel from root.

 Users can set one or more elements to be part of the sensitivity analysis. For
 each element the user has to provide the number of values to be explored and
 their values. When all elements involved in the sensitivity analysis are
 configured, the user must launch the command Sensitivity from menu Data in the
 main LSD Browser. This command generates as many copies as the product of all
 values for all elements in the s.a. It then kicks off the initialization of all
 elements involved so that each combination of parameters is assigned to one
 branch of the model.

 The user is supposed then to save the resulting configuration.

 Options concerning initialization for sensitivity analysis are not saved into
 the model configuration files, and are therefore lost when closing the LSD model
 program if not saved in a .sa file.
 *************************************************************/
lsd::object *lsd::object::sensitivity_parallel( sensitivity *s )
{
	int i;
	sensitivity *cs;
	object *cur;
	variable *cv;

	if ( s->next != NULL )
	{
		for ( cur = this, i = 0; i < s->num_val; ++i )
		{
			s->cur_val = i;
			cur = cur->sensitivity_parallel( s->next );
		}

		return cur;
	}

	for ( cur = this, i = 0; i < s->num_val; ++i )
	{
		s->cur_val = i;
		for ( cs = gui::sim.sens; cs != NULL; cs = cs->next )
		{
			cv = cur->search_var( cur, cs->label );
			if ( cs->param == 0 )				// handle lags > 0
				cv->val[ cs->lag ] = cs->val[ cs->cur_val ];
			else
				cv->val[ 0 ] = cs->val[ cs->cur_val ];
		}

		cur = cur->hyper_next( );
	}

	return cur;
}


/*************************************************************
 SENSITIVITY_SEQUENTIAL
 This function fills the initial values according to the sensitivity analysis
 system performed by sequential simulations: each run executes one configuration
 labeled with sequential labels.

 Contrary to parallel sensitivity settings, this function initialize all elements
 in the configuration with the specified label.

 Users can set one or more elements to be part of the sensitivity analysis. For
 each element the user has to provide the number of values to be explored and
 their values. When all elements involved in the sensitivity analysis are
 configured, the user must launch the command Sensitivity from menu Data in the
 main LSD Browser.

 Options concerning initialization for sensitivity analysis are saved into model
 configuration files, to be executed with a terminal version of the LSD model.
 One configuration file is created for each possible combination of the
 sensitivity analysis values (parameters and initial conditions). Optionally, it
 is possible to define the parameter "probSampl" with the (uniform) probability
 of a given point in the sensitivity analysis space is saved as configuration
 file. In practice, this allows for the Monte Carlo sampling of the parameter
 space, which is often necessary when the s.a. space is too big to be analyzed
 in its entirety.
 *************************************************************/
void gui::sensitivity_sequential( int *findex, lsd::sensitivity *s, double probSampl, const char *dest_path )
{
	int i;
	lsd::object *cur;
	lsd::sensitivity *cs;
	lsd::variable *cv;

	if ( s->next != NULL )
	{
		for ( i = 0; i < s->num_val && ! stop; ++i )
		{
			s->cur_val = i;
			sensitivity_sequential( findex, s->next, probSampl, dest_path );
		}

		return;
	}

	for ( i = 0; i < s->num_val && ! stop; ++i )
	{
		for ( s->cur_val = i, cs = sim.sens; cs != NULL; cs = cs->next )
		{
			cv = sim.root->search_var( sim.root, cs->label );

			for ( cur = cv->up; cur != NULL; cur = cur->hyper_next( ) )
			{
				cv = cur->search_var( cur, cs->label );
				if ( cs->param == 1 )				// handle lags > 0
					cv->val[ 0 ] = cs->val[ cs->cur_val ];
				else
					cv->val[ cs->lag ] = cs->val[ cs->cur_val ];
			}
		}

		if ( probSampl == 1.0 || ran1( ) <= probSampl )	// if required draw if point will be sampled
		{
			// generate a configuration file for the experiment (no descriptions)
			if ( ! sim.save_xml_configuration( dest_path, NULL, NULL, *findex, true, true, get_str( model_options[ 0 ] ), get_str( model_options[ 1 ] ), get_str( model_options[ 2 ] ), eq_file ) )
			{
				plog( "Aborted\n" );
				cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Configuration files cannot be saved\" -detail \"Check if the drive or the current directory is set READ-ONLY, select a drive/directory with write permission and try again.\"" );
				return;
			}

			if ( ( *findex + 1 ) % 10 == 0 )
				cmd( "prgboxupdate .psa %d", *findex );

			*findex = *findex + 1;
		}
	}
}


/*************************************************************
 NOLH_TABLE
 Calculate a Near Orthogonal Latin Hypercube (NOLH) design for sampling.
 Include tables to up to 29 variables ( sanchez 2009, Cioppa and Lucas 2007).
 Returns the number of samples (n) required for the calculated design and a
 pointer to the matrix n x k, where k is the number of factors ( variables).

 It is possible to load one additional design table from disk ( file NOLH.csv in
 the same folder as the configuration file .lsd). The table should be formed
 by positive integers only, in the n (rows) x k ( columns), separated by commas,
 one row per text line and no empty lines. The table can be loaded manually
 (NOLH_load function) or automatically as needed during sampling (NOLH_sampler).
 This function gets the index to the default NOLH design table or -1 otherwise.
 *************************************************************/
int gui::NOLH_table( int k )
{
	for ( unsigned int i = 0; i < ( ( sizeof NOLH ) / sizeof NOLH[ 0 ] ); ++i )
		if ( k >= NOLH[ i ].kMin && k <= NOLH[ i ].kMax )
			return i;

	return -1;		// number of factors not supported by the preloaded tables
}


/*************************************************************
 NOLH_VALID_TABLES
 Determine the valid NOLH tables for the number of factors
 *************************************************************/
char *gui::NOLH_valid_tables( int k, char *out, int sz )
{
	int min_tab = NOLH_table( k );
	char buff[ MAX_ELEM_LENGTH ];

	if ( min_tab <= 0 )
		snprintf( out, sz, "External only" );
	else
	{
		strcpy( out, "" );
		for ( int i = min_tab; ( unsigned ) i < ( ( sizeof NOLH ) / sizeof NOLH[ 0 ] ); ++i )
		{
			snprintf( buff, MAX_ELEM_LENGTH, " \"%d\u00D7%d\u00D7%d\"", NOLH[ i ].kMax, NOLH[ i ].n1, NOLH[ i ].n2 );
			lsd::strcatn( out, buff, sz );
		}
	}

	return out;
}


/*************************************************************
 NOLH_CLEAR
 Function to remove table 0
 *************************************************************/
void gui::NOLH_clear( void )
{
	if ( NOLH_0 == NULL )			// table is not allocated?
		return;
	delete [ ] NOLH_0[ 0 ];
	delete [ ] NOLH_0;
	NOLH_0 = NULL;
	NOLH[ 0 ].kMin = NOLH[ 0 ].kMax = NOLH[ 0 ].n1 = NOLH[ 0 ].n2 = NOLH[ 0 ].loLevel = NOLH[ 0 ].hiLevel = 0;
	NOLH[ 0 ].table = NULL;
}


/*************************************************************
 NOLH_LOAD
 Function to load a .csv file named NOLH.csv as table 0 (first to be used)
 If option 'force' is used, will be used for any number of factors
 *************************************************************/
bool gui::NOLH_load( const char baseName[ ], bool force )
{
	int i, j, sz, n = 1, loLevel = INT_MAX, hiLevel = 1, kFile = 0;
	char *fileName, *lBuffer, *str, *num;
	bool ok = false;
	FILE *NOLHfile;

	if ( NOLH_0 != NULL )			// table already loaded?
		NOLH_clear( );

	if ( strlen( sim.conf_path ) > 0 )
	{
		sz = strlen( sim.conf_path ) + strlen( baseName ) + 2;
		fileName = new char [ sz ];
		snprintf( fileName, sz, "%s/%s", sim.conf_path, baseName );
	}
	else
	{
		sz = strlen( baseName ) + 1;
		fileName = new char [ sz ];
		snprintf( fileName, sz, "%s", baseName );
	}
	NOLHfile = fopen( fileName, "r" );
	if ( NOLHfile == NULL )
	{
		sim.error_hard( "problem accessing the design of experiment file",
						"check if the requested file exists",
						false,
						"cannot open NOHL design file '%s'", fileName );
		return false;
	}

	lBuffer = str = new char [ MAX_FILE_SIZE ];

	// get first text line
	fgets( str, MAX_FILE_SIZE, NOLHfile );
	do								// count factors
	{
		num = strtok( str, ",;" );	// get next value
		str = NULL;					// continue strtok from last position
		kFile++;					// factor counter
	}
	while ( num != NULL );
	kFile--;						// adjust for the last non read

	do								// count file lines
	{
		fgets( lBuffer, MAX_FILE_SIZE, NOLHfile );
		n++;
	}
	while ( ! feof( NOLHfile ) );

	// get contiguous space for the 2D table
	NOLH_0 = new int * [ n ];
	NOLH_0[ 0 ] = new int [ n * kFile ];
	for ( i = 1; i < n; i++ )
		NOLH_0[ i ] = NOLH_0[ i - 1 ] + kFile;

	rewind( NOLHfile );				// restart from the beginning
	for ( i = 0; i < n ; i++ )		// read file content
	{
		// get next text line
		fgets( lBuffer, MAX_FILE_SIZE, NOLHfile );
		str = lBuffer;
		for ( j = 0; j < kFile ; j++ )	// get factor values
		{
			num = strtok( str, "," );	// get next value
			str = NULL;					// continue strtok from last position
			NOLH_0[ i ][ j ] = atoi( num );

			if ( num == NULL || NOLH_0[ i ][ j ] == 0 )
			{
				delete [ ] NOLH_0[ 0 ];
				delete [ ] NOLH_0;
				NOLH_0 = NULL;
				sim.error_hard( "invalid design of experiment file",
								"check the file contents",
								false,
								"invalid format in NOHL file '%s', line=%d", fileName, i + 1 );
				goto end;
			}

			if ( NOLH_0[ i ][ j ] < loLevel )
				loLevel = NOLH_0[ i ][ j ];
			if ( NOLH_0[ i ][ j ] > hiLevel )
				hiLevel = NOLH_0[ i ][ j ];
		}
	}

	// set new table characteristics
	if ( force )
		NOLH[ 0 ].kMin = 1;
	else
		NOLH[ 0 ].kMin = NOLH[ sizeof NOLH / sizeof NOLH[ 0 ] - 1 ].kMax + 1;

	NOLH[ 0 ].kMax = kFile;
	NOLH[ 0 ].n1 = NOLH[ 0 ].n2 = n;
	NOLH[ 0 ].loLevel = loLevel;
	NOLH[ 0 ].hiLevel = hiLevel;
	NOLH[ 0 ].table = NOLH_0[ 0 ];

	plog( "\nNOLH file loaded: %s\nk = %d, n = %d, low level = %d, high level = %d", fileName, kFile, n, loLevel, hiLevel );

	ok = true;
end:
	delete [ ] fileName;
	delete [ ] lBuffer;
	return ok;
}


/*************************************************************
 MAT_*
 Matrix operations support functions for morris_oat() and enhancements
 *************************************************************/

// Random choice between two numbers
#define RND_CHOICE( o1, o2 ) ( ran1( ) < 0.5 ? o1 : o2 )


/*************************************************************
 MAT_NEW
 allocate dynamic space for matrix
 *************************************************************/
double **gui::mat_new( int m, int n )
{
	double **c = new double * [ m ];
	for ( int i = 0; i < m ; ++i )		//rows
		c[ i ] = new double [ n ];
	return c;
}


/*************************************************************
 MAT_DEL
 deallocate dynamic space for matrix
 *************************************************************/
void gui::mat_del( double **a, int m )
{
	for ( int i = 0; i < m ; ++i )		//rows
		delete [ ] a[ i ];
	delete [ ] a;
}


/*************************************************************
 MAT_MULT_MAT
 multiply two matrices ( c<-a*b)
 *************************************************************/
double **gui::mat_mult_mat( double **a, int m, int n, double **b, int o, int p, double **c )
{
	if ( n != o )
		return NULL;
	for ( int i = 0; i < m ; ++i )		//row of first matrix
		for ( int j = 0; j < p; ++j )	//column of second matrix
		{
			c[ i ][ j ] = 0;
			for ( int k = 0; k < n; ++k )
				c[ i ][ j ] += a[ i ][ k ] * b[ k ][ j ];
		}
	return c;
}


/*************************************************************
 MAT_ADD_MAT
 add two same size matrices ( c<-a+b)
 *************************************************************/
double **gui::mat_add_mat( double **a, int m, int n, double **b, double **c )
{
	for ( int i = 0; i < m ; ++i )		//rows
		for ( int j = 0; j < n; ++j )	//columns
			c[ i ][ j ] = a[ i ][ j ] + b[ i ][ j ];
	return c;
}


/*************************************************************
 MAT_MULT_SCAL
 multiply all positions in matrix by a scalar
 *************************************************************/
double **gui::mat_mult_scal( double **a, int m, int n, double b, double **c )
{
	for ( int i = 0; i < m ; ++i )		//rows
		for ( int j = 0; j < n; ++j )	//columns
			c[ i ][ j ] = a[ i ][ j ] * b;
	return c;
}


/*************************************************************
 MAT_ADD_SCAL
 add a scalar to all positions in matrix
 *************************************************************/
double **gui::mat_add_scal( double **a, int m, int n, double b, double **c )
{
	for ( int i = 0; i < m ; ++i )		//rows
		for ( int j = 0; j < n; ++j )	//columns
			c[ i ][ j ] = a[ i ][ j ] + b;
	return c;
}


/*************************************************************
 MAT_COPY_SCAL
 copy a scalar to all positions in matrix
 *************************************************************/
double **gui::mat_copy_scal( double **a, int m, int n, double b )
{
	for ( int i = 0; i < m ; ++i )		//rows
		for ( int j = 0; j < n; ++j )	//columns
			a[ i ][ j ] = b;
	return a;
}


/*************************************************************
 MAT_COPY_MAT
 copy same size matrices
 *************************************************************/
double **gui::mat_copy_mat( double **a, int m, int n, double **b )
{
	for ( int i = 0; i < m ; ++i )		//rows
		for ( int j = 0; j < n; ++j )	//columns
			a[ i ][ j ] = b[ i ][ j ];
	return a;
}


/*************************************************************
 MAT_INS_MAT
 insert lines (replacing) in matrix (a<-b)
 *************************************************************/
double **gui::mat_ins_mat( double **a, int m, int n, double **b, int o, int p, int lpos )
{
	if ( lpos + o > m || p > n )
		return NULL;
	for ( int i = 0; i < m ; ++i )		//rows
		for ( int j = 0; j < n; ++j )	//columns
			if ( i >= lpos && i < lpos + o && j < p )
				a[ i ][ j ] = b[ i - lpos ][ j ];
	return a;
}


/*************************************************************
 MAT_EXT_MAT
 extract lines (replacing) in matrix (a<-b)
 *************************************************************/
double **gui::mat_ext_mat( double **a, int m, int n, double **b, int o, int p, int lpos )
{
	if ( lpos + m > o || n < p )
		return NULL;
	for ( int i = 0; i < m ; ++i )		//rows
		for ( int j = 0; j < n; ++j )	//columns
				a[ i ][ j ] = b[ i + lpos ][ j ];
	return a;
}


/*************************************************************
 MAT_SUM_DISTS
 Sum the Euclidean distances of points in two matrices of same size
 Calculates the distance between all points pairs and adds them
 The matrices a and b must have the same size
 *************************************************************/
double gui::mat_sum_dists( double **a, int m, int n, double **b )
{
	double sum = 0;
	for ( int i = 0; i < m ; ++i )			//rows in a
		for ( int k = 0; k < m; ++k )		//rows in b
		{
			double dist2 = 0;
			for ( int j = 0; j < n ; ++j )	//columns
				dist2 += pow( a[ i ][ j ] - b[ k ][ j ], 2 );
			sum += sqrt( dist2 );
		}
	return sum;
}


/*************************************************************
 MORRIS_OAT
 Calculate a DoE for Elementary Effects (Morris 1991) analysis,
 according to Saltelli et al 2008. Code adapted from SAlib by
 Jon Herman.

 Delta is fixed at p/[2(p-1)]

 k: number of factors
 r: number of trajectories
 p: number of grid levels
 jump: delta measured in grid levels
 X: preallocated memory area to save the trajectories
 *************************************************************/
double **gui::morris_oat( int k, int r, int p, int jump, double **X )
{
	int i, j, l;
	double delta = ( double ) jump / ( p - 1 );	// grid step delta

	// reset random number generator
	gui_prng.seed( sim.seed );

	// allocate all temporary matrices
	double **B = mat_new( k + 1, k ),
		**DM = mat_new( k, k ),
		**P = mat_new( k, k ),
		**X_base = mat_new( k + 1, k ),
		**delta_diag = mat_new( k, k ),
		**temp_1 = mat_new( k + 1, k ),
		**temp_2 = mat_new( k + 1, k );

	// orientation matrix B: lower triangular (1) + upper triangular (-1)
	for ( i = 0; i < k + 1; ++i )
		for ( j = 0; j < k; ++j )
			B[ i ][ j ] = ( i > j ) ? 1 : -1;

	// Create r trajectories. Each trajectory contains k+1 parameter sets.
	// ( starts at a base point, and then changes one parameter at a time )

	cmd( "progressbox .psa \"Creating DoE\" \"Analyzing EE trajectories\" \"Trajectory\" %d", r );

	for ( l = 0; l < r; ++l )
	{
		// directions matrix DM - diagonal matrix of either +1 or -1
		for ( i = 0; i < k; ++i )
			for ( j = 0; j < k; ++j )
				DM[ i ][ j ] = ( i == j )? RND_CHOICE( -1, 1 ) : 0;

		// permutation matrix P
		int *perm = new int [ k ];
		for ( i = 0; i < k; ++i )
			perm [ i ] = i;

		shuffle( & perm[ 0 ], & perm[ k ], gui_prng );

		P = mat_copy_scal( P, k, k, 0 );
		for ( i = 0; i < k; ++i )
			P[ i ][ perm[ i ] ] = 1;

		delete [ ] perm;

		// starting point for this trajectory
		for ( j = 0; j < k; ++j )
		{
			double start = sim.rnd_int( 0, p - delta * ( p - 1 ) - 1 ) / ( p - 1 );
			for ( i = 0; i < k + 1; ++i )
				X_base[ i ][ j ] = start;
		}

		// Indices to be assigned to X, corresponding to this trajectory
		int index_list = l * ( k + 1 );
		for ( i = 0; i < k; ++i )
			for ( j = 0; j < k; ++j )
				delta_diag[ i ][ j ] = ( i == j ) ? delta : 0;

		temp_1 = mat_mult_mat( B, k + 1, k, P, k, k, temp_1 );
		temp_2 = mat_mult_mat( temp_1, k + 1, k, DM, k, k, temp_2 );
		temp_1 = mat_add_scal( temp_2, k + 1, k, 1, temp_1 );
		temp_2 = mat_mult_mat( temp_1, k + 1, k, delta_diag, k, k, temp_2 );
		temp_1 = mat_mult_scal( temp_2, k + 1, k, 0.5, temp_1 );
		temp_2 = mat_add_mat( temp_1, k + 1, k, X_base, temp_2 );
		X = mat_ins_mat( X, r * ( k + 1 ), k, temp_2, k + 1, k, index_list );

		cmd( "prgboxupdate .psa %d", l + 1 );
	}

	cmd( "destroytop .psa" );

	// deallocate all temporary matrices
	mat_del( B, k + 1 );
	mat_del( DM, k );
	mat_del( P, k );
	mat_del( X_base, k + 1 );
	mat_del( delta_diag, k );
	mat_del( temp_1, k + 1 );
	mat_del( temp_2, k + 1 );

	return X;
}


/*************************************************************
 COMPUTE_DISTANCE_MATRIX
 Optimize a DoE for Elementary Effects (Morris 1991) analysis,
 according to Campolongo et al 2007 and Ruano 2012. Code adapted
 from SAlib by Jon Herman.

 sample: pool of trajectories produced by morris_oat()
 M: number of trajectories in pool
 r: number of final trajectories (<= M)
 DM: preallocated memory area to save the trajectories
 *************************************************************/
double **gui::compute_distance_matrix( double **sample, int M, int k, double **DM )
{
	double **input_1 = mat_new( k + 1, k ),
		   **input_2 = mat_new( k + 1, k );

	DM = mat_copy_scal( DM, M, M, 0 );

	cmd( "progressbox .psa \"Creating DoE\" \"Compute EE distance matrix\" \"Trajectory\" %d", M );

	for ( int i = 0 ; i < M; ++i )
	{
		input_1 = mat_ext_mat( input_1, k + 1, k,
							   sample, M * ( k + 1 ), k,
							   i * ( k + 1 ) );
		for ( int j = i + 1; j < M; ++j )
		{
			input_2 = mat_ext_mat( input_2, k + 1, k,
								   sample, M * ( k + 1 ), k,
								   j * ( k + 1 ) );
			DM[ i ][ j ] = DM[ j ][ i ] =
				mat_sum_dists( input_1, k + 1, k, input_2 );
		}

		cmd( "prgboxupdate .psa %d", i + 1 );
	}

	cmd( "destroytop .psa" );

	mat_del( input_1, k + 1 );
	mat_del( input_2, k + 1 );

	return DM;
}


/*************************************************************
 COMBINATIONS
 Calculate the combinations of indices, r-to-r
 *************************************************************/
i2_vecT gui::combinations( i_listT indices, int r )
{
	i_vecT comb;
	i2_vecT combs;

	// copy list to vector
	i_vecT ind( indices.begin( ), indices.end( ) );
	int n = ind.size( );
	if ( r > n )
		return combs;
	// create selection array with r selectors
	b_vecT v( n );
	fill( v.begin( ), v.end( ) - n + r, true );
	// create all permutations of the selectors
	do
	{
		// set member if it is selected in the current permutation of v
		for ( int i = 0; i < n; ++i )
			if ( v[ i ] )
				comb.push_back( ind[ i ] );
		combs.push_back( comb );
		comb.clear( );
	}
	while ( prev_permutation( v.begin( ), v.end( ) ) );

	return combs;
}


/*************************************************************
 SUM_DISTANCES
 Calculate combinatorial distance between a select group of trajectories,
 indicated by indices
 indices: list of candidate pairs of points = list < int >
 	DM: distance matrix = array (M,M)
 *************************************************************/
double gui::sum_distances( i_listT indices, double **DM )
{
	// get all combination pairs of indices
	i2_vecT combs = combinations( indices, 2 );

	// add distance of all points pairs
	double D = 0;
	for ( unsigned int j = 0; j < combs.size( ); ++j )
		D += DM[ combs[ j ][ 0 ] ][ combs[ j ][ 1 ] ];

	return D;
}


/*************************************************************
 TOP_IDX
 	Get the top-i size items index from a unidimensional array
 *************************************************************/
i_listT gui::top_idx( double *a, int n, int i )
{
	b_vecT used( n, false );
	i_listT top;

	for ( int k = 0; k < i; ++k )
	{
		int max_idx = -1;
		double max = -INFINITY;
		for ( int j = 0; j < n; ++j )
			if ( ! used[ j ] && a[ j ] > max )
			{
				max_idx = j;
				max = a[ j ];
			}
		used[ max_idx ] = true;
		top.push_back( max_idx );
	}

	return top;
}


/*************************************************************
 GET_MAX_SUM_IND
 Get the indice that belong to the maximum distance in an array of distances
 indices_list = list of points
 distance = array (M)
 *************************************************************/
i_listT gui::get_max_sum_ind( i_list_vecT indices_list, d_vecT row_maxima_i )
{
	int max_idx = -1;
	double max = -INFINITY;

	for ( unsigned int j = 0; j < indices_list.size( ); ++j )
		if ( row_maxima_i[ j ] > max )
		{
			max_idx = j;
			max = row_maxima_i[ j ];
		}

	return indices_list[ max_idx ];
}


/*************************************************************
 ADD_INDICES
 Adds extra indices for the combinatorial problem.
 For indices = (1,2 ) and M=5, the method returns [(1,2,3),(1,2,4),(1,2,5)]
 *************************************************************/
i_list_vecT gui::add_indices( i_listT m_max_ind, int M )
{
	i_list_vecT list_new_indices;
	i_listT copy = m_max_ind;

	for ( int i = 0; i < M; ++i )
		if ( find( m_max_ind.begin( ), m_max_ind.end( ), i ) == m_max_ind.end( ) )
		{
			copy.push_back( i );
			list_new_indices.push_back( copy );
			copy.pop_back( );
		}

	return list_new_indices;
}


/*************************************************************
 OPT_TRAJECTORIES
 An alternative by Ruano et al. (2012 ) for the brute force approach as
 originally proposed by Campolongo et al. (2007). The method should improve
 the speed with which an optimal set of trajectories is found tremendously
 for larger sample sizes.
 *************************************************************/
double **gui::opt_trajectories( int k, double **pool, int M, int r, double **X )
{
	if ( r >= M )					// nothing to do?
	{
		X = mat_copy_mat( X, r * ( k + 1 ), k, pool );
		return X;
	}

	i_listT indices, i_max_ind, m_max_ind, tot_max;
	i_list_vecT tot_indices_list, indices_list, m_ind;

	double **DM = mat_new( M, M );
	DM = compute_distance_matrix( pool, M, k, DM );

	d_vecT tot_max_array( r - 1, 0 );

	//#############Loop 'i'#############
	// i starts at 1
	for ( int i = 1; i < r; ++i )
	{
		indices_list.clear( );
		d_vecT row_maxima_i( M, 0 );

		for ( int row = 0; row < M; ++row )
		{
			indices = top_idx( DM[ row ], M, i );
			indices.push_back( row );
			row_maxima_i[ row ] = sum_distances( indices, DM );
			indices_list.push_back( indices );
		}

		// Find the indices belonging to the maximum distance
		i_max_ind = get_max_sum_ind( indices_list, row_maxima_i );

		// ######### Loop 'm' ( called loop 'k' in Ruano) ############
		m_max_ind = i_max_ind;
		// m starts at 1
		for ( int m = 1; m <= r - i - 1; ++m )
		{
			m_ind = add_indices( m_max_ind, M );
			d_vecT m_maxima( m_ind.size( ), 0 );

			for ( unsigned int n = 0; n < m_ind.size( ); ++n )
				m_maxima[ n ] = sum_distances( m_ind[ n ], DM );

			m_max_ind = get_max_sum_ind( m_ind, m_maxima );
		}
		tot_indices_list.push_back( m_max_ind );
		tot_max_array[ i - 1 ] = sum_distances( m_max_ind, DM );
	}

	tot_max = get_max_sum_ind( tot_indices_list, tot_max_array );
	tot_max.sort( );
	i_vecT max( tot_max.begin( ), tot_max.end( ) );

	// index the submatrix for each trajectory
	i_vecT index_list( M, 0 );
	for ( int i = 0; i < M; ++i )
		index_list[ i ] = i * ( k + 1 );

	// move the best trajectories to caller 2D array
	double **temp = mat_new( k + 1, k );
	for ( int i = 0; i < r; ++i )
	{
		temp = mat_ext_mat( temp, k + 1, k, pool, M * ( k + 1 ), k, index_list[ max[ i ] ] );
		X = mat_ins_mat( X, r * ( k + 1 ), k, temp, k + 1, k, index_list[ i ] );
	}

	mat_del( temp, k + 1 );
	mat_del( DM, M );

	return X;
}


/*************************************************************
 ~DESIGN
 Destructor function to the design object
 *************************************************************/
gui::design::~design( void )
{
	clear_design( );
}

/*************************************************************
 CLEAR_DESIGN
 free all the memory in a design object
 *************************************************************/
void gui::design::clear_design( void )
{
	int i, j;

	for ( i = 0; i < n; ++i )			// free memory through all experiments
	{
		for ( j = 0; j < k; ++j )
			delete [ ] doe[ i ][ j ];

		delete [ ] doe[ i ];
	}

	for ( i = 0; i < k; ++i )			// and all variables
	{
		delete [ ] hi[ i ];
		delete [ ] lo[ i ];
		delete [ ] lab[ i ];
	}

	delete [ ] par;
	delete [ ] lag;
	delete [ ] inst;
	delete [ ] intg;
	delete [ ] hi;
	delete [ ] lo;
	delete [ ] doe;
	delete [ ] lab;

	typ = tab = n = k = 0;
	par = lag = inst = NULL;
	intg = NULL;
	hi = lo = NULL;
	lab = NULL;
	doe = NULL;
}


/*************************************************************
 LOAD_DESIGN_DATA
 Load the design data from sensitivity object
 *************************************************************/
void gui::design::load_design_data( lsd::sensitivity *rsens, int n )
{
	int h, i, j, nVal;
	lsd::sensitivity *cs;

	// allocate memory for data
	par = new int [ k ];		// array of variable type (parameter/lagged variable)
	lag = new int [ k ];		// array of lags
	inst = new int [ k ];		// array of number of instances
	intg = new bool [ k ];		// array of format (integer/float)
	hi = new double * [ k ];	// array of high factor values (per instance)
	lo = new double * [ k ];	// array of low factor values (per instance)
	lab = new char * [ k ];		// array of variable labels
	doe = new double ** [ n ];	// allocate space for weighted design table

	// define low and high values from sensitivity data for each factor/variable
	for ( i = 0, cs = rsens; i < k && cs != NULL; ++i, cs = cs->next )
	{
		inst[ i ] = sim.hyper_count_var( cs->label );
		nVal = cs->num_val;			// number of data values
		nVal = nVal % 2 == 0 ? nVal : nVal - 1 ;// discard last unpaired value

		if ( inst[ i ] == 0 || nVal < 2 )// only multi-instance/value factor
			continue;

		par[ i ] = cs->param;		// set factor type
		lag[ i ] = cs->lag;			// set number of lags
		intg[ i ] = cs->integer;	// set factor format
		lab[ i ] = new char [ strlen( cs->label ) + 1 ];
		strcpy( lab[ i ], cs->label );// set factor name

		hi[ i ] = new double [ inst[ i ] ];
		lo[ i ] = new double [ inst[ i ] ];
		for ( h = j = 0; j < inst[ i ]; ++j )
		{
			if ( 2 * j + 1 < nVal )	// data available?
			{
				hi[ i ][ j ] = std::max( cs->val[ 2 * j ], cs->val[ 2 * j + 1 ] );
				lo[ i ][ j ] = std::min( cs->val[ 2 * j ], cs->val[ 2 * j + 1 ] );
			}
			else					// recycle previous data
			{
				hi[ i ][ j ] = hi[ i ][ h ];
				lo[ i ][ j ] = lo[ i ][ h++ ];
			}
		}
	}

	// define data structure to hold DoE
	for ( i = 0; i < n; ++i )		// for all experiments
	{
		doe[ i ] = new double * [ k ];// allocate 2nd level data
		for ( j = 0; j < k; ++j )	// for all factors
			doe[ i ][ j ] = new double[ inst[ j ] ];
	}
}


/*************************************************************
 DESIGN
 Constructor function to the design object
 type = 1: NOLH
 type = 2: random sampling
 type = 3: Elementary Effects sampling (Morris, 1991)
 samples = -1: use extended predefined sample size (n2)
 factors = 0: use automatic DoE size
 *************************************************************/
gui::design::design( lsd::sensitivity *rsens, int typ, const char *fname, const char *dest_path, int findex, int samples, int factors, int jump, int trajs )
{
	int h, i, j, sz, kTab, doeRange, poolSz;
	double **pool, **traj;
	char *doefname, doeName[ MAX_ELEM_LENGTH ];
	FILE *f;

	// reset random number generator
	gui_prng.seed( sim.seed );

	if ( rsens == NULL )					// valid pointer?
		typ = 0;							// trigger invalid design
	else
		k = sim.num_sensitivity_variables( );// number of factors

	switch ( typ )
	{
		case 1:								// Near Orthogonal Latin Hypercube sampling
			if ( strcmp( fname, "" ) )		// if filename was specified
			{
				NOLH_load( fname, true );	// load file and force using it always
				kTab = k;
			}
			else
			{
				if ( factors != 0 && k > factors )	// invalid # of factors selected?
				{
					sim.error_hard( "invalid design of experiment parameters",
									"check the design",
									false,
									"number of NOLH variables selected is too small" );
					goto invalid;
				}

				// if user selected # of factors, use it to select internal table
				kTab = ( factors == 0 ) ? k : factors;
			}

			tab = NOLH_table( kTab );		// design table to use
			if ( tab == -1 )				// number of factors too large, try to load external table ( file )
			{
				if ( NOLH_load( ) )			// tentative table load from disk ok?
				{
					tab = NOLH_table( k );	// design table to use
					if ( tab == -1 )		// still too large?
					{
						sim.error_hard( "invalid design of experiment parameters",
										"check the design",
										false,
										"too many variables to test for NOLH.csv size" );
						goto invalid;		// abort
					}
				}
				else
				{
					sim.error_hard( "invalid design of experiment parameters",
									"check the design",
									false,
									"too many variables to test" );
					goto invalid;			// abort
				}
			}

			// get the number of samples required by the NOLH design, according to user choice (basic/extended)
			n = ( samples != -1 ) ? NOLH[ tab ].n1 : NOLH[ tab ].n2;

			plog( "\nNOLH table used: %d (%s), n = %d", tab, tab > 0 ? "built-in" : "from file", n );

			// load data from sensitivity objects
			load_design_data( rsens, n );

			// calculate the design of the experiment
			doeRange = NOLH[ tab ].hiLevel - NOLH[ tab ].loLevel;
			for ( i = 0; i < n; ++i )		// for all experiments
				for ( j = 0; j < k; ++j )	// for all factors
					for ( h = 0; h < inst[ j ]; ++h )	// for all instances
						doe[ i ][ j ][ h ] = lo[ j ][ h ] +
						( *( NOLH[ tab ].table + i * NOLH[ tab ].kMax + j ) - 1 ) *
						( hi[ j ][ h ] - lo[ j ][ h ] ) / doeRange;

			break;

		case 2:								// random sampling
			n = samples;					// number of samples required
			if ( n < 1 )					// at least one sample required
				goto invalid;

			// load data from sensitivity objects
			load_design_data( rsens, n );

			// calculate the design of the experiment
			for ( i = 0; i < n; ++i )		// for all experiments
				for ( j = 0; j < k; ++j )	// for all factors
					for ( h = 0; h < inst[ j ]; ++h )	// for all instances
						doe[ i ][ j ][ h ] = lo[ j ][ h ] +
											 ran1( ) * ( hi[ j ][ h ] - lo[ j ][ h ] );

			break;

		case 3:								// Elementary Effects sampling
			poolSz = samples * ( k + 1 );	// larger pool to extract samples
			n = trajs * ( k + 1 );			// number of effective samples
			if ( n < 1 || n > poolSz )		// at least one sample required
				goto invalid;

			// load data from sensitivity objects
			load_design_data( rsens, n );

			pool = new double * [ poolSz ];	// allocate space for weighted design table
			for ( i = 0; i < poolSz; ++i )	// for all pool trajectories
				pool[ i ] = new double [ k ];

			// calculate the Morris OAT pool of trajectories
			pool = morris_oat( k, samples, factors, jump, pool );

			// select the best trajectories from pool
			traj = new double * [ n ];
			for ( i = 0; i < n; ++i )		 // for all final trajectories
				traj[ i ] = new double [ k ];

			traj = opt_trajectories( k, pool, samples, trajs, traj );

			// scale the DoE to the sensitivity test ranges
			for ( i = 0; i < n; ++i )		// for all experiments
				for ( j = 0; j < k; ++j )	// for all factors
					for ( h = 0; h < inst[ j ]; ++h )	// for all instances
						doe[ i ][ j ][ h ] = lo[ j ][ h ] + traj[ i ][ j ] *
											 ( hi[ j ][ h ] - lo[ j ][ h ] );

			for ( i = 0; i < poolSz; ++i )	// free pool memory
				delete [ ] pool[ i ];

			for ( i = 0; i < n; ++i )		// free trajectory memory
				delete [ ] traj[ i ];

			delete [ ] pool;
			delete [ ] traj;

			break;

		case 0:
		default:							// invalid design!
		invalid:
			typ = tab = n = k = 0;
			par = lag = inst = NULL;
			hi = lo = NULL;
			intg = NULL;
			doe = NULL;
			lab = NULL;
			return;
	}

	// generate a configuration file for the experiment

	// file name for saving table
	snprintf( doeName, MAX_ELEM_LENGTH, "%u_%u", ( unsigned ) findex, ( unsigned ) ( findex + n - 1 ) );

	if ( strlen( dest_path ) > 0 )				// non-default folder?
	{
		sz = strlen( dest_path ) + strlen( sim.conf_name ) + strlen( doeName ) + 10;
		doefname = new char [ sz ];
		snprintf( doefname, sz, "%s/%s_%s.csv", dest_path, strlen( sim.conf_name ) > 0 ? sim.conf_name : "doe", doeName );
	}
	else
	{
		sz = strlen( sim.conf_name ) + strlen( doeName ) + 9;
		doefname = new char [ sz ];
		snprintf( doefname, sz, "%s_%s.csv", strlen( sim.conf_name ) > 0 ? sim.conf_name : "doe", doeName );
	}

	if ( ( f = fopen( doefname, "w" ) ) == NULL )
	{
		delete [ ] doefname;
		clear_design( );

		sim.error_hard( "cannot create DoE configuration file",
						"check if disk is not full or set READ-ONLY",
						false,
						"a disk error prevented creating the file" );
		return;
	}

	// write the doe table to disk
	for ( j = 0; j < k; ++j )		// write variable labels
		for ( h = 0; h < inst[ j ]; ++h )	// for all instances
			if( h == 0 )
				fprintf( f, "%s%c", lab[ j ],
						 j == k - 1 && h == inst[ j ] - 1 ? '\n' : ',' );
			else
				fprintf( f, "%s.%d%c", lab[ j ], h + 1,
						 j == k - 1 && h == inst[ j ] - 1 ? '\n' : ',' );

	for ( i = 0; i < n; ++i )		// for all experiments
		for ( j = 0; j < k; ++j )	// for all factors
			for ( h = 0; h < inst[ j ]; ++h )	// for all instances
			{
				// round to integer if necessary
				if ( intg[ j ] )
					doe[ i ][ j ][ h ] = std::round( doe[ i ][ j ][ h ] );

				fprintf( f, "%lf%c", doe[ i ][ j ][ h ],
						 j == k - 1 && h == inst[ j ] - 1 ? '\n' : ',' );
			}

	fclose( f );

	plog( "\nDoE configuration saved: %s", doefname );

	delete [ ] doefname;
}


/*************************************************************
 SENSITIVITY_DOE
 Generate the configuration files for the Design of Experiment (DoE)
 *************************************************************/
void gui::sensitivity_doe( int *findex, design *doe, const char *dest_path )
{
	int h, i, j, inif = *findex;
	lsd::object *cur;
	lsd::variable *cv;

	stop = false;
	cmd( "progressbox .psa \"Creating DoE\" \"Creating configuration files\" \"File\" %d { set stop true }", doe->n );

	for ( i = 0; i < doe->n && ! stop; ++i )	// run through all experiments
	{
		// set up the variables ( factors) with the experiment values
		for ( j = 0; j < doe->k; j++ )			// run through all factors
		{
			cv = sim.root->search_var( sim.root, doe->lab[ j ] );// find variable to set
			for ( h = 0, cur = cv->up; cur != NULL; ++h, cur = cur->hyper_next( ) )
			{									// run through all objects containing var
				cv = cur->search_var( cur, doe->lab[ j ] );
				if ( doe->par[ j ] == 1 )		// handle lags > 0
					cv->val[ 0 ] = doe->doe[ i ][ j ][ h ];
				else
					cv->val[ doe->lag[ j ] ] = doe->doe[ i ][ j ][ h ];
			}
		}

		// generate a configuration file for the experiment (no descriptions)
		if ( ! sim.save_xml_configuration( dest_path, NULL, NULL, *findex, true, true, get_str( model_options[ 0 ] ), get_str( model_options[ 1 ] ), get_str( model_options[ 2 ] ), eq_file ) )
		{
			plog( "Aborted\n" );
			cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Configuration files cannot be saved\" -detail \"Check if the drive or the current directory is set READ-ONLY, select a drive/directory with write permission and try again.\"" );
			return;
		}

		if ( ( i + 2 ) % 10 == 0 )
			cmd( "prgboxupdate .psa %d", i + 1 );

		++( *findex );
	}

	cmd( "destroytop .psa" );

	plog( "\nSensitivity analysis configurations produced: %d\n", findexSens - 1 );

	// if succeeded, explain user how to proceed
	if ( ! stop )
		sensitivity_created( dest_path, lsd::clean_file( strlen( sim.conf_name ) > 0 ? sim.conf_name : "doe" ), inif );
	else
		*findex = 0;							// don't consider for appending
}
