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
LATTICE.CPP
Contains the methods and functions to work with graphical
lattices.
*************************************************************/

#include "decl.h"

int **lattice = NULL;					// lattice data colors array
int rows = 0;							// lattice size
int columns = 0;
int error_count;						// error counters
double dimW = 0;						// lattice screen size
double dimH = 0;


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
double init_lattice( double pixW, double pixH, double nrow, double ncol, const char lrow[ ], const char lcol[ ], const char lvar[ ], object *p, int init_color )
{
	char init_color_string[ 32 ];	// the final string to be used to define tk color to use
	int i, j, hsize, vsize, hsizeMax, vsizeMax;

	// ignore invalid values
	if ( ( int ) nrow < 1 || ( int ) ncol < 1 || ( int ) nrow > INT_MAX || ( int ) ncol > INT_MAX )
	{
		plog( "\nError: invalid lattice initialization values, ignoring.\n");
		return -1;
	}

	init_color = min( init_color, 1099 );	// limit to valid palette

	// reset the LSD lattice, if any
	close_lattice( );
	rows = ( int ) max( 0, floor( nrow ) );
	columns = ( int ) max( 0, floor( ncol ) );
	error_count = 0;

	// create the color data matrix
	lattice = new int *[ rows ];
	for ( i = 0; i < rows; ++i )
		lattice[ i ] = new int [ columns ];

	for ( i = 0; i < rows; ++i )
		for ( j = 0; j < columns; ++j )
			lattice[ i ][ j ] = init_color;

#ifndef _NW_

	hsize = get_int( "hsizeLat" );			// 400
	vsize = get_int( "vsizeLat" );			// 400
	hsizeMax = get_int( "hsizeLatMax" );	// 1024
	vsizeMax = get_int( "vsizeLatMax" );	// 1024

	pixW = floor( pixW ) > 0 ? floor( pixW ) : hsize;
	pixH = floor( pixH ) > 0 ? floor( pixH ) : vsize;
	pixW = min( pixW, hsizeMax );
	pixH = min( pixH, vsizeMax );

	dimH = pixH / rows;
	dimW = pixW / columns;

	if ( init_color < 0 && ( - init_color ) <= 0xffffff )		// RGB mode selected?
		snprintf( init_color_string, 32, "#%06x", - init_color );	// yes: just use the positive RGB value
	else
	{
		snprintf( init_color_string, 32, "$c%d", init_color );		// no: use the positive RGB value
		// create (background color) pallete entry if invalid palette in init_color
		cmd( "if { ! [ info exist c%d ] } { set c%d $colorsTheme(bg) }", init_color, init_color  );
	}

	// create the window with the lattice, roughly 600 pixels as maximum dimension
	cmd( "newtop .lat \"%s%s - LSD Lattice (%.0lf x %.0lf)\" { destroytop .lat } \"\"", unsaved_change() ? "*" : " ", strlen( simul_name ) > 0 ? simul_name : NO_CONF_NAME, nrow, ncol );

	cmd( "ttk::canvas .lat.c -height %d -width %d -entry 0 -dark $darkTheme", ( unsigned int ) pixH, ( unsigned int ) pixW );

	if ( init_color != 1001 )
		cmd( ".lat.c configure -background %s", init_color_string );

	cmd( "pack .lat.c" );

	cmd( "save .lat b { \
			if { ! [ info exists pltSavFmt ] } { \
				set pltSavFmt svg \
			}; \
			if { [ string equal $pltSavFmt eps ] } { \
				set c \"Encapsulated Postscript\" \
			} else { \
				set c \"Scalable Vector Graphics\" \
			}; \
			set a [ tk_getSaveFile -parent .lat -title \"Save Lattice to File\" -defaultextension .$pltSavFmt -initialfile %s.$pltSavFmt -initialdir \"%s\" -filetypes { { {Scalable Vector Graphics} {.svg} } { {Encapsulated Postscript} {.eps} } { {All files} {*} } } -typevariable c ]; \
			if { [ string length $a ] != 0 } { \
				set a [ file nativename $a ]; \
				set b [ string trimleft [ file extension $a ] . ]; \
				if { $b in [ list svg eps ] } { \
					set pltSavFmt $b \
				}; \
				if [ string equal $pltSavFmt eps ] { \
					.lat.c postscript -colormode color -file \"$a\" \
				} else { \
					canvas2svg .lat.c \"$a\" \
				}; \
				plog \"\nPlot saved: $a\n\" \
			} \
		}", strlen( simul_name ) > 0 ? simul_name : "plot", path );

	cmd( "set rows %d", rows );
	cmd( "set columns %d", columns );
	cmd( "set dimH %.6g", dimH );
	cmd( "set dimW %.6g", dimW );

	cmd( "for { set i 1 } { $i <= $rows } { incr i } { \
			for { set j 1 } { $j <= $columns } { incr j } { \
				set x1 [ expr { ( $j - 1 ) * $dimW } ]; \
				set y1 [ expr { ( $i - 1 ) * $dimH } ]; \
				set x2 [ expr { $j * $dimW } ]; \
				set y2 [ expr { $i * $dimH } ]; \
				.lat.c create rectangle $x1 $y1 $x2 $y2 -outline \"\" -tags c${i}_${j} \
			} \
		}" );

	cmd( "showtop .lat centerS no no no" );

	cmd( "tooltip::tooltip .lat.b.ok \"Save plot to file\"" );

	cmd( "bind .lat <Button-2> { .lat.b.ok invoke }" );
	cmd( "bind .lat <Button-3> { event generate .lat <Button-2> -x %%x -y %%y }" );
	cmd( "bind .lat <F1> { LsdHelp lattice.html }" );
	set_shortcuts_run( ".lat" );

#endif

	return 0;
}

// call for macro
double init_lattice( int init_color, double nrow, double ncol, double pixW, double pixH )
{
	return init_lattice( pixW, pixH, nrow, ncol, "y", "x", "", NULL, init_color );
}


/***************************************************
EMPTY_LATTICE
***************************************************/
void empty_lattice( void )
{
	if ( lattice != NULL && rows > 0 )
	{
		for ( int i = 0; i < rows; ++i )
			delete [ ] lattice[ i ];

		delete [ ] lattice;
	}

	lattice = NULL;
	rows = columns = 0;
}


/***************************************************
CLOSE_LATTICE
***************************************************/
void close_lattice( void )
{
	empty_lattice( );

#ifndef _NW_
	cmd( "destroytop .lat" );
#endif
}


/***************************************************
UPDATE_LATTICE
update the cell line.col to the color val (1 to 21 as set in default.tcl palette)
negative values of val prompt for the use of the (positive) RGB equivalent
***************************************************/
double update_lattice( double line, double col, double val )
{
	char val_string[ 32 ];		// the final string to be used to define tk color to use
	int line_int, col_int, val_int;

	line_int = line - 1;
	col_int = col - 1;
	val_int = max( 0, floor( val ) );

	// ignore invalid values
	if ( line_int < 0 || col_int < 0 || line_int >= rows ||
		 col_int >= columns || ( int ) fabs( val ) > INT_MAX )
	{
		if ( error_count == ERR_LIM )
			plog( "\nWarning: too many lattice parameter errors, messages suppressed.\n");
		else
			if ( error_count < ERR_LIM )
				plog( "\nError: invalid lattice update values, ignoring." );

		++error_count;

		return -1;
	}

	// save lattice color data

	if ( lattice != NULL && rows > 0 && columns > 0 )
	{
		if ( val >= 0 && lattice[ line_int ][ col_int ] == val_int )
			return 0;
		else
			lattice[ line_int ][ col_int ] = val_int;
	}
#ifndef _NW_

	// avoid operation if canvas was closed
	if ( ! exists_window( ".lat.c" ) )
		return -1;

	if ( val < 0 && ( - ( int )  val ) <= 0xffffff )	// RGB mode selected?
		snprintf( val_string, 32, "#%06x", - ( int ) val );	// yes: just use the positive RGB value
	else
	{
		snprintf( val_string, 32, "$c%d", val_int );			// no: use the predefined Tk color
		// create (background color) pallete entry if invalid palette in val
		cmd( "if { ! [ info exist c%d ] } { set c%d $colorsTheme(bg) }", val_int, val_int  );
	}

	cmd( ".lat.c itemconfigure c%d_%d -fill %s", line_int + 1, col_int + 1, val_string );

#endif

	return 0;
}


/***************************************************
READ_LATTICE
read the cell line.col color val (1 to 21 as set in default.tcl palette)
negative values of val mean the use of the (positive) RGB equivalent
***************************************************/
double read_lattice( double line, double col )
{
	// ignore invalid values
	if ( ( int ) line <= 0 || ( int ) col <= 0 || ( int ) line > rows || ( int ) col > columns )
	{
		if ( error_count == ERR_LIM )
			plog( "\nWarning: too many lattice parameter errors, messages suppressed.\n");
		else
			if ( error_count < ERR_LIM )
				plog( "\nError: invalid lattice update values, ignoring." );

		++error_count;

		return -1;
	}

	if ( lattice != NULL && rows > 0 && columns > 0 )
		return lattice[ ( int ) line - 1 ][ ( int ) col - 1 ];
	else
		return 0;
}


/***************************************************
SAVE_LATTICE
Save the existing lattice (if any) to the specified file name.
***************************************************/
double save_lattice( const char *fname )
{
#ifndef _NW_
	// avoid operation if no canvas or no file name
	if ( ! exists_window( ".lat.c" ) || fname == NULL || strlen( fname ) == 0 )
		return -1;

	cmd( "set latname \"%s\"", fname );
	cmd( "append latname .eps" );
	cmd( ".lat.c postscript -colormode color -file $latname" );
#endif
	return 0;
}
