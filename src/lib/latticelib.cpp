/*************************************************************

	LSD 9.0 - January 2024
	written by Marco Valente, Universita' dell'Aquila
	and by Marcelo Pereira, University of Campinas

	Copyright Marco Valente and Marcelo Pereira
	LSD is distributed under the GNU General Public License

	See Readme.txt for copyright information of
	third parties' code used in LSD

 *************************************************************/

/*************************************************************
LATTICELIB.CPP
Contains the functions to work with lattices in DLL and
no-window executables. The graphical GUI code is stored in
LATTICE.CPP.
*************************************************************/

#include "lib/libLSD.h"				// LSD library classes


/***************************************************
INIT_LATTICE
Create a new run time lattice having:
- pix= maximum pixel (600 should fit in typical screens, 0=default size)
- nrow= number of rows
- ncol= number of columns
- lrow= label of variable or parameter indicating the row value
- lcol= label of variable or parameter indicating the column value
- lvar= label of variable or parameter from which to read the color of the cell
- p= pointer of the object containing the initial color of the cell (if flag==-1)
- init_color= indicate the type of initialization.
  If init_color < 0, the (positive) RGB equivalent to init_color is used.
  Otherwise, the lattice is homogeneously initialized to the palette color specified by init_color.
***************************************************/
double simulation::init_lattice( double pixW, double pixH, double nrow, double ncol, const char lrow[ ], const char lcol[ ], const char lvar[ ], object *p, int init_color )
{
	int i, j;

	// ignore invalid values
	if ( ( int ) nrow < 1 || ( int ) ncol < 1 || ( int ) nrow > INT_MAX || ( int ) ncol > INT_MAX )
	{
		plog( "\nError: invalid lattice initialization values, ignoring.\n");
		return -1;
	}

	init_color = min( init_color, 1099 );	// limit to valid palette

	// reset the LSD lattice, if any
	close_lattice( );
	latt->rows = max( 0, ( int ) floor( nrow ) );
	latt->columns = max( 0, ( int ) floor( ncol ) );
	latt->errors = 0;

	// create the color data matrix
	latt->array = new int *[ latt->rows ];
	for ( i = 0; i < latt->rows; ++i )
		latt->array[ i ] = new int [ latt->columns ];

	for ( i = 0; i < latt->rows; ++i )
		for ( j = 0; j < latt->columns; ++j )
			latt->array[ i ][ j ] = init_color;

	if ( liblnk.init_lattice_helper != 0 )
		liblnk.init_lattice_helper( pixW, pixH, nrow, ncol, init_color );

	return 0;
}

// call for macro
double simulation::init_lattice( int init_color, double nrow, double ncol, double pixW, double pixH )
{
	return init_lattice( pixW, pixH, nrow, ncol, "y", "x", "", NULL, init_color );
}


/***************************************************
EMPTY_LATTICE
***************************************************/
void simulation::empty_lattice( void )
{
	if ( latt->array != NULL && latt->rows > 0 )
	{
		for ( int i = 0; i < latt->rows; ++i )
			delete [ ] latt->array[ i ];

		delete [ ] latt->array;
	}

	latt->array = NULL;
	latt->rows = latt->columns = 0;
}


/***************************************************
CLOSE_LATTICE
***************************************************/
void simulation::close_lattice( void )
{
	empty_lattice( );
	cmd_gui( "destroytop .lat" );
}


/***************************************************
UPDATE_LATTICE
update the cell line.col to the color val (1 to 21 as set in default.tcl palette)
negative values of val prompt for the use of the (positive) RGB equivalent
***************************************************/
double simulation::update_lattice( double line, double col, double val )
{
	int line_int, col_int, val_int;

	line_int = line - 1;
	col_int = col - 1;
	val_int = max( 0, ( int ) floor( val ) );

	// ignore invalid values
	if ( line_int < 0 || col_int < 0 || line_int >= latt->rows ||
		 col_int >= latt->columns || ( int ) fabs( val ) > INT_MAX )
	{
		if ( latt->errors == ERR_LIM )
			plog( "\nWarning: too many lattice parameter errors, messages suppressed.\n");
		else
			if ( latt->errors < ERR_LIM )
				plog( "\nError: invalid lattice update values, ignoring." );

		++latt->errors;

		return -1;
	}

	// save lattice color data

	if ( latt->array != NULL && latt->rows > 0 && latt->columns > 0 )
	{
		if ( val >= 0 && latt->array[ line_int ][ col_int ] == val_int )
			return 0;
		else
			latt->array[ line_int ][ col_int ] = val_int;
	}

	if ( liblnk.update_lattice_helper != 0 )
		return liblnk.update_lattice_helper( line, col, val, line_int, col_int, val_int );
	else
		return 0;
}


/***************************************************
READ_LATTICE
read the cell line.col color val (1 to 21 as set in default.tcl palette)
negative values of val mean the use of the (positive) RGB equivalent
***************************************************/
double simulation::read_lattice( double line, double col )
{
	// ignore invalid values
	if ( ( int ) line <= 0 || ( int ) col <= 0 || ( int ) line > latt->rows || ( int ) col > latt->columns )
	{
		if ( latt->errors == ERR_LIM )
			plog( "\nWarning: too many lattice parameter errors, messages suppressed.\n");
		else
			if ( latt->errors < ERR_LIM )
				plog( "\nError: invalid lattice update values, ignoring." );

		++latt->errors;

		return -1;
	}

	if ( latt->array != NULL && latt->rows > 0 && latt->columns > 0 )
		return latt->array[ ( int ) line - 1 ][ ( int ) col - 1 ];
	else
		return 0;
}


/***************************************************
SAVE_LATTICE
Save the existing lattice (if any) to the specified file name.
***************************************************/
double simulation::save_lattice( const char *fname )
{
	if ( liblnk.save_lattice_helper != 0 )
		return liblnk.save_lattice_helper( fname );
	else
		return 0;
}
