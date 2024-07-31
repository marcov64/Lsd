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
 terminal executables. The graphical GUI code is stored in
 LATTICE.CPP.
 *************************************************************/

#include "lib/libLSD.h"				// LSD library classes


/*************************************************************
 _INIT_LATTICE_ (*)
 Create a new run time lattice having:
 - pix= maximum pixel (600 should fit in typical screens,
        0=default size)
 - nrow= number of rows
 - ncol= number of columns
 - lrow= label of variable or parameter indicating the row value
 - lcol= label of variable or parameter indicating the column value
 - lvar= label of variable or parameter from which to read the
         color of the cell
 - p= pointer of the object containing the initial color of the
      cell (if flag==-1)
 - init_color= indicate the type of initialization.
   If init_color < 0, the (positive) RGB equivalent to init_color
   is used. Otherwise, the lattice is homogeneously initialized
   to the palette color specified by init_color.
 *************************************************************/
double lsd::equation::_init_lattice_( double pixW, double pixH, double nrow, double ncol, const char lrow[ ], const char lcol[ ], const char lvar[ ], object *p, int init_color )
{
	int i, j;

	// ignore invalid values
	if ( ( int ) nrow < 1 || ( int ) ncol < 1 || ( int ) nrow > INT_MAX || ( int ) ncol > INT_MAX )
	{
		_sim_->plog( "\nError: invalid lattice initialization values, ignoring.\n");
		return -1;
	}

	init_color = std::min( init_color, 1099 );	// limit to valid palette

	// reset the LSD lattice, if any
	_close_lattice_( );
	_sim_->latt->rows = std::max( 0, ( int ) floor( nrow ) );
	_sim_->latt->columns = std::max( 0, ( int ) floor( ncol ) );
	_sim_->latt->errors = 0;

	// create the color data matrix
	_sim_->latt->array = new int *[ _sim_->latt->rows ];
	for ( i = 0; i < _sim_->latt->rows; ++i )
		_sim_->latt->array[ i ] = new int [ _sim_->latt->columns ];

	for ( i = 0; i < _sim_->latt->rows; ++i )
		for ( j = 0; j < _sim_->latt->columns; ++j )
			_sim_->latt->array[ i ][ j ] = init_color;

	if ( _sim_->liblnk != NULL )
		_sim_->liblnk->init_lattice_helper( pixW, pixH, nrow, ncol, init_color );

	return 0;
}

// call for macro
double lsd::equation::_init_lattice_( int init_color, double nrow, double ncol, double pixW, double pixH )
{
	return _init_lattice_( pixW, pixH, nrow, ncol, "y", "x", "", NULL, init_color );
}


/*************************************************************
 _CLOSE_LATTICE_ (*)
 *************************************************************/
void lsd::equation::_close_lattice_( void )
{
	cmd( "destroytop .lat" );

	if ( _sim_->latt->array != NULL && _sim_->latt->rows > 0 )
	{
		for ( int i = 0; i < _sim_->latt->rows; ++i )
			delete [ ] _sim_->latt->array[ i ];

		delete [ ] _sim_->latt->array;
	}

	_sim_->latt->array = NULL;
	_sim_->latt->rows = _sim_->latt->columns = 0;
}


/*************************************************************
 _UPDATE_LATTICE_ (*)
 update the cell line.col to the color val (1 to 21
 as set in default.tcl palette)
 negative values of val prompt for the use of the
 (positive) RGB equivalent
 *************************************************************/
double lsd::equation::_update_lattice_( double line, double col, double val )
{
	int line_int, col_int, val_int;

	line_int = line - 1;
	col_int = col - 1;
	val_int = std::max( 0, ( int ) floor( val ) );

	// ignore invalid values
	if ( line_int < 0 || col_int < 0 || line_int >= _sim_->latt->rows ||
		 col_int >= _sim_->latt->columns || ( int ) fabs( val ) > INT_MAX )
	{
		if ( _sim_->latt->errors == ERR_LIM )
			_sim_->plog( "\nWarning: too many lattice parameter errors, messages suppressed.\n");
		else
			if ( _sim_->latt->errors < ERR_LIM )
				_sim_->plog( "\nError: invalid lattice update values, ignoring." );

		++_sim_->latt->errors;

		return -1;
	}

	// save lattice color data

	if ( _sim_->latt->array != NULL && _sim_->latt->rows > 0 && _sim_->latt->columns > 0 )
	{
		if ( val >= 0 && _sim_->latt->array[ line_int ][ col_int ] == val_int )
			return 0;
		else
			_sim_->latt->array[ line_int ][ col_int ] = val_int;
	}

	if ( _sim_->liblnk != NULL )
		return _sim_->liblnk->update_lattice_helper( line, col, val, line_int, col_int, val_int );
	else
		return 0;
}


/*************************************************************
 _READ_LATTICE_ (*)
 read the cell line.col color val (1 to 21 as set in
 default.tcl palette)
 negative values of val mean the use of the (positive)
 RGB equivalent
 *************************************************************/
double lsd::equation::_read_lattice_( double line, double col )
{
	// ignore invalid values
	if ( ( int ) line <= 0 || ( int ) col <= 0 || ( int ) line > _sim_->latt->rows || ( int ) col > _sim_->latt->columns )
	{
		if ( _sim_->latt->errors == ERR_LIM )
			_sim_->plog( "\nWarning: too many lattice parameter errors, messages suppressed.\n");
		else
			if ( _sim_->latt->errors < ERR_LIM )
				_sim_->plog( "\nError: invalid lattice update values, ignoring." );

		++_sim_->latt->errors;

		return -1;
	}

	if ( _sim_->latt->array != NULL && _sim_->latt->rows > 0 && _sim_->latt->columns > 0 )
		return _sim_->latt->array[ ( int ) line - 1 ][ ( int ) col - 1 ];
	else
		return 0;
}


/*************************************************************
 _SAVE_LATTICE_ (*)
 Save the existing lattice (if any) to the specified
 file name.
 *************************************************************/
double lsd::equation::_save_lattice_( const char *fname )
{
	if ( _sim_->liblnk != NULL )
		return _sim_->liblnk->save_lattice_helper( fname );
	else
		return 0;
}
