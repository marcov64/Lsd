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
Contains the functions to work with graphical lattices on the
GUI. The basic functions are stored in LATTICELIB.CPP.
*************************************************************/

#include "LSD.h"


/***************************************************
INIT_LATTICE_HELPER (DLL WRAPPER)
Initialize the GUI part of the lattice.
***************************************************/
void init_lattice_helper( double pixW, double pixH, double nrow, double ncol, int init_color )
{
	char init_color_string[ 32 ];	// the final string to be used to define tk color to use
	int hsize, vsize, hsizeMax, vsizeMax;

	hsize = get_int( "hsizeLat" );			// 400
	vsize = get_int( "vsizeLat" );			// 400
	hsizeMax = get_int( "hsizeLatMax" );	// 1024
	vsizeMax = get_int( "vsizeLatMax" );	// 1024

	pixW = floor( pixW ) > 0 ? floor( pixW ) : hsize;
	pixH = floor( pixH ) > 0 ? floor( pixH ) : vsize;
	pixW = min( pixW, hsizeMax );
	pixH = min( pixH, vsizeMax );

	sim.latt->height = pixH / sim.latt->rows;
	sim.latt->width = pixW / sim.latt->columns;

	if ( init_color < 0 && ( - init_color ) <= 0xffffff )		// RGB mode selected?
		snprintf( init_color_string, 32, "#%06x", - init_color );	// yes: just use the positive RGB value
	else
	{
		snprintf( init_color_string, 32, "$c%d", init_color );		// no: use the positive RGB value
		// create (background color) pallete entry if invalid palette in init_color
		cmd( "if { ! [ info exist c%d ] } { set c%d $colorsTheme(bg) }", init_color, init_color  );
	}

	// create the window with the lattice, roughly 600 pixels as maximum dimension
	cmd( "newtop .lat \"%s%s - LSD Lattice (%.0lf x %.0lf)\" { destroytop .lat } \"\"", unsaved_change() ? "*" : " ", strlen( sim.conf_name ) > 0 ? sim.conf_name : NO_CONF_NAME, nrow, ncol );

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
		}", strlen( sim.conf_name ) > 0 ? sim.conf_name : "plot", sim.conf_path );

	cmd( "set rows %d", sim.latt->rows );
	cmd( "set columns %d", sim.latt->columns );
	cmd( "set dimH %.6g", sim.latt->height );
	cmd( "set dimW %.6g", sim.latt->width );

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
}


/***************************************************
UPDATE_LATTICE_HELPER (DLL WRAPPER)
Update the GUI part of the lattice.
***************************************************/
double update_lattice_helper( double line, double col, double val, int line_int, int col_int, int val_int )
{
	char val_string[ 32 ];		// the final string to be used to define tk color to use

	// avoid operation if canvas was closed
	if ( ! exists_window( ".lat.c" ) )
		return -1;

	if ( val < 0 && ( - ( int )  val ) <= 0xffffff )	// RGB mode selected?
		snprintf( val_string, 32, "#%06x", - ( int ) val );	// yes: just use the positive RGB value
	else
	{
		snprintf( val_string, 32, "$c%d", val_int );	// no: use the predefined Tk color
		// create (background color) pallete entry if invalid palette in val
		cmd( "if { ! [ info exist c%d ] } { set c%d $colorsTheme(bg) }", val_int, val_int  );
	}

	cmd( ".lat.c itemconfigure c%d_%d -fill %s", line_int + 1, col_int + 1, val_string );

	return 0;
}


/***************************************************
SAVE_LATTICE_HELPER (DLL WRAPPER)
Save the existing GUI lattice (if any).
***************************************************/
double save_lattice_helper( const char *fname )
{
	// avoid operation if no canvas or no file name
	if ( ! exists_window( ".lat.c" ) || fname == NULL || strlen( fname ) == 0 )
		return -1;

	cmd( "set latname \"%s\"", fname );
	cmd( "append latname .eps" );
	cmd( ".lat.c postscript -colormode color -file $latname" );

	return 0;
}
