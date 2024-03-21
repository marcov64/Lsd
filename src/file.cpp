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
FILE.CPP
Contains the non-basic methods and functions used to access
files. The remaining file-oriented functions are stored in
FILELIB.CPP.

The main methods of object contained in this file are:

- void object::save_struct( FILE *f, char *tab )
Saves the structure of the object (that is, the label,
variables and parameters and descendants, not number of
objects). Calls the save_struct for all the descendant types.

- void object::save_insts( FILE *f )
Save the numerical values for object instances (one number
for each group of object of this type) and the initial values
for variables. It save also option information, that is
whether to save, plot or debug the variables.
It calls the save_insts for all the descendant types.
*************************************************************/

#include "LSD.h"


/****************************************************
OPEN_CONFIGURATION
	Open a clean configuration,
	either the current or not
****************************************************/
bool open_configuration( object *&r, bool reload )
{
	bool loaded;
	const char *lab1, *lab2;
	int i;
	std::string warnings;

	if ( ! reload || strlen( sim.conf_name ) == 0 )
	{									// ask user the file to use, if not reloading
		cmd( "set fn [ tk_getOpenFile -parent . -title \"Open Configuration File\"	-defaultextension \".lsd\" -initialdir \"$path\" -filetypes { { {LSD model file} {.lsd} } } ]" );
		cmd( "if { [ string length $fn ] > 0 && ! [ fn_spaces \"$fn\" . ] } { \
				set path [ file dirname $fn ]; \
				set fn [ string map -nocase [ list [ file extension $fn ] \"\" ] [ file tail $fn ] ]; \
				set res 0 \
			} else { \
				set res 2 \
			}" );

		if ( get_int( "res" ) == 0 )
		{
			lab1 = get_str( "path" );
			lab2 = get_str( "fn" );
			if ( lab1 == NULL || lab2 == NULL || strlen( lab2 ) == 0 )
				return false;

			delete [ ] sim.conf_name;
			sim.conf_name = new char[ strlen( lab2 ) + 1 ];
			strcpy( sim.conf_name, lab2 );

			delete [ ] sim.conf_path;
			sim.conf_path = new char[ strlen( lab1 ) + 1 ];
			strcpy( sim.conf_path, lab1 );

			if ( strlen( sim.conf_path ) > 0 )
				cmd( "cd $path" );

			cmd( "set listfocus 1; set itemfocus 0" );// point for first var in listbox
			cmd( "set lastObj \"\"" );			// disable last object for reload
		}
		else
			if ( sim.conf_ok )
				reload = true;					// try to reload if use cancel load
			else
				return false;
	}

	if ( reload )
		r->save_pos( );							// save current position when reloading

	switch ( i = load_configuration_gui( reload, &warnings, 0 ) )// try to load the configuration
	{
		case 0:
			loaded = true;
			break;

		case 1:									// file/path not found
			if ( strlen( sim.conf_path ) > 0 )
				cmd( "ttk::messageBox -parent . -type ok -title Error -icon error -message \"File not found\" -detail \"File for model '%s' not found in directory '%s'.\"", strlen( sim.conf_name ) > 0 ? sim.conf_name : NO_CONF_NAME, sim.conf_path );
			else
				cmd( "ttk::messageBox -parent . -type ok -title Error -icon error -message \"File not found\" -detail \"File for model '%s' not found in current directory\"", strlen( sim.conf_name ) > 0 ? sim.conf_name : NO_CONF_NAME	 );
			loaded = false;
			break;

		case 2:									// problem from STRUCT section
		case 3:									// problem from DATA section
		case 21:								// invalid XML format
		case 22:								// missing XML root node
		case 31:								// internal XML error
		case 32:								// missing XML object name
		case 33:								// missing XML element type
		case 34:								// missing XML element name
		case 35:								// invalid XML element type
		case 41:								// internal XML error
		case 42 ... 43:							// XML inconsistent # of groups
		case 44 ... 45:							// XML inconsistent # of node ids/names
		case 46 ... 47:							// XML inconsistent # of link/weight groups
		case 48:								// XML invalid links
		case 49:								// XML inconsistent # of links
		case 50:								// XML inconsistent # of weights
		case 51:								// XML inconsistent # of link/weight groups
		case 52:								// missing XML element data
		case 53 ... 56:							// XML inconsistent variable values

			cmd( "ttk::messageBox -parent . -type ok -title Error -icon error -message \"Invalid or damaged file (%d :%.24s)\" -detail \"Incomplete configuration loaded!\n\nPlease check if a proper LSD configuration file was selected or complete the missing model components and settings.\"", i, warnings.c_str( ) );
			loaded = false;
			break;

		case 4:									// problem from SIM_NUM section
		case 5:									// problem from SEED
		case 6:									// problem from MAX_STEP section
		case 7:									// problem from EQUATION section
		case 8:									// problem from MODELREPORT section
		case 9:									// problem from DESCRIPTION section
		case 23:								// missing XML settings node
		case 24:								// missing XML equation node
			cmd( "ttk::messageBox -parent . -type ok -title Error -icon error -message \"Partially damaged file (%d :%.24s)\" -detail \"Element descriptions were lost but the configuration can still be used.\n\nPlease check if the desired LSD configuration file was selected or re-enter the description information if needed.\n\nIf this is a sensitivity analysis configuration file, this message is expected, and configuration file is ok.\"", i, warnings.c_str( ) );
			sim.root->reset_description( );
			loaded = true;
			break;

		case 10 ... 11:							// problem from DOCUOBSERVE section
		case 12 ... 13:							// problem from DOCUINITIAL section
			cmd( "ttk::messageBox -parent . -type ok -title Error -icon error -message \"Partially damaged file (%d :%.24s)\" -detail \"Observation flags and equation file were lost but the configuration can still be used.\n\nPlease check if the desired LSD configuration file was selected or re-configure the lost parts if needed.\"", i, warnings.c_str( ) );
			loaded = true;
			break;

		default:
			loaded = false;
	}

	if ( i == 0 && warnings.size( ) > 0 )
			cmd( "ttk::messageBox -parent . -type ok -title Warning -icon warning -message \"Partially damaged file (%d :%.24s)\" -detail \"Part of the configuration data was missing or invalid and was replaced by default values.\n\nPlease check if the desired LSD configuration file was selected or re-configure the affected parts as needed.\"", i, warnings.c_str( ) );

	if ( r != NULL && loaded && reload )
		currObj = r = sim.root->restore_pos( );	// restore pointed object and variable
	else
		currObj = r = sim.root;					// new structure

	if ( loaded && ! ignore_eq_file && strncmp( sim.conf_eq_txt, eq_txt, std::min( strlen( sim.conf_eq_txt ), strlen( eq_txt ) ) ) )
		plog( "\nWarning: the configuration file has been previously run with different equations\nfrom those used to create the LSD model program.\nChanges may affect the simulation results. You can offload the original\nequations in a new equation file and compare differences using TkDiff in LMM\n(menu File)." );

	redrawRoot = redrawStruc = true;			// force browser/structure redraw

	return loaded;
}


/*****************************************************************************
LOAD_CONFIGURATION_GUI (DLL WRAPPER)
	Load configuration
	If full is false, just the model data is unloaded
	Returns: pointer to root object
******************************************************************************/
int load_configuration_gui( bool reload, std::string *warnings, int quick )
{
	int res;

	reset_configuration_gui( );

	if( ( res = sim.load_configuration( reload, warnings, quick ) ) == 0 )
	{
		cmd( "set lastConf [ string map -nocase { \"%s/\" \"\" } [ file normalize \"%s\" ] ]", model_path, sim.conf_file );
		sim.root->load_elem_lists( );
	}

	return res;
}


/*****************************************************************************
RESET_CONFIGURATION_GUI
	Reset the GUI part of a loaded configuration
******************************************************************************/
void reset_configuration_gui( void )
{
	currObj = NULL;								// no current object pointer
	unsaved_change( false );					// signal no unsaved change
	unsavedData = false;						// no unsaved simulation results
	unsavedSense = false;						// no sensitivity data to save
	findexSens = 0;								// reset sensitivity serial number
	NOLH_clear( );								// deallocate DoE
	sim.empty_sensitivity( );					// discard sensitivity analysis data

	cmd( "destroytop .lat" );					// remove lattice window
	cmd( "unset -nocomplain modObj modElem modVar modPar modFun" );// no elements

	if ( ! sim.running )
		cmd( "destroytop .plt" );				// remove run-time plot window
}


/****************************************************
LOAD_PREV_CONFIGURATION
Restore sensitivity configuration
****************************************************/
bool load_prev_configuration( void )
{
	char *saFile = NULL;
	int i, lstFidx = findexSens;
	std::string warnings;
	FILE *f;

	if ( sens_file != NULL )					// save SA file name if one is loaded
	{
		saFile = new char[ strlen( sens_file ) + 1 ];
		strcpy( saFile, sens_file );
	}

	if ( ( i = load_configuration_gui( true, &warnings, 0 ) ) != 0 )
	{
		cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Configuration file cannot be reloaded (%d :%.24s)\" -detail \"Previously loaded configuration could not be restored. Check if LSD still has access to the model directory.\n\nCurrent configuration will be reset now.\"", i, warnings.c_str( ) );

		unload_configuration_gui( true );		// full unload everything
		return false;
	}

	if ( saFile != NULL )						// restore SA configuration, if any
	{
		sim.empty_sensitivity( );
		NOLH_clear( );							// deallocate DoE
		f = fopen( saFile, "rt" );
		if ( f == NULL || load_sensitivity( f ) != 0 )
		{
			cmd( "ttk::messageBox -parent . -type ok -icon error -title Error -message \"Sensitivity analysis file cannot be reloaded\" -detail \"Previously loaded SA configuration could not be restored. Check if LSD still has access to the model directory.\n\nCurrent configuration will be reset now.\"" );
			return false;
		}

		if ( f != NULL )
			fclose( f );

		delete [ ] saFile;
	}

	findexSens = lstFidx;

	return true;
}


/*****************************************************************************
UNLOAD_CONFIGURATION_GUI (DLL WRAPPER)
	Unload the current configuration
	If full is false, just the model data is unloaded
******************************************************************************/
void unload_configuration_gui( bool full )
{
	sim.unload_configuration( full );
	reset_configuration_gui( );

	if ( full )									// full unload? (no new config?)
	{
		delete sens_file;						// reset sensitivity file name
		sens_file = NULL;

		cmd( "set path \"%s\"", model_path );
		if ( strlen( model_path ) > 0 )
			cmd( "cd \"$path\"" );

		cmd( "unset -nocomplain lastConf" );	// no last configuration to reload
		cmd( "set listfocus 1; set itemfocus 0" );// point for first var in listbox
		cmd( "set lastObj \"\"" );				// disable last object for reload
		redrawRoot = redrawStruc = true;		// force browser/structure redraw
	}
}


/****************************************************
LOAD_ELEM_LISTS
Load tcl lists of model objects and other elements
****************************************************/
void object::load_elem_lists( )
{
	bridge *cb;
	variable *cv;

	if ( up == NULL )							// reset lists if root
		cmd( "unset -nocomplain modObj modElem modVar modPar modFun" );
	else
		cmd( "lappend modObj %s", label );		// register object if not root

	// register elements in object
	for ( cv = v; cv != NULL; cv = cv->next )
	{
		switch( cv->param )
		{
			case 0:
				cmd( "lappend modVar %s", cv->label );
				break;
			case 1:
				cmd( "lappend modPar %s", cv->label );
				break;
			case 2:
				cmd( "lappend modFun %s", cv->label );
		}

		cmd( "lappend modElem %s", cv->label );
	}

	// register son objects
	for ( cb = b; cb != NULL; cb = cb->next )
		cb->head->load_elem_lists( );
}


/*****************************************************************************
SAVE_XML_CONFIGURATION_GUI (DLL WRAPPER)
	Save current defined configuration (adding tag index if appropriate) to
	gzip-compressed xml file
	If quick is true, just the structure and the parameters are saved
	Returns: true: save ok, false: save failure
******************************************************************************/
bool save_xml_configuration_gui( int findex, const char *dest_path, bool quick )
{
	bool saved;

	saved = sim.save_xml_configuration( findex, dest_path, quick, get_str( model_info[ 0 ] ), get_str( model_info[ 1 ] ), get_str( model_info[ 2 ] ), eq_file, eq_txt );

	if ( saved )
		cmd( "set lastConf [ string map -nocase { \"%s/\" \"\" } [ file normalize \"%s\" ] ]", model_path, sim.conf_file );

	return saved;
}


/*******************************************
DEB_LOG
Creates/saves the log file and
enable/disable logging the variables
computation order and enable/disable the
debugger
********************************************/
void deb_log( bool on, int time )
{
	char fname[ MAX_PATH_LENGTH ];

	// check if should turn off
	if ( ! on || sim.parallel_mode || sim.fast_mode != 0 )
	{
		// disable debugging
		if ( time > sim.t && sim.deb_t >= time )
			sim.deb_t = 0;
		else
			if ( ( time == 0 && sim.deb_t == sim.t ) || time == sim.t )
				sim.deb_set = false;

		// act now?
		if ( time == 0 || sim.t > time )
		{
			// close file if open
			if ( sim.log_file_ptr != NULL )
			{
				fclose( sim.log_file_ptr );
				sim.log_file_ptr = NULL;
			}
		}
		else
			sim.log_stop = time;
	}

	// check if should turn on
	if ( on && ! sim.parallel_mode && sim.fast_mode == 0 )
	{
		// enable debugging
		if ( time > sim.t )
			sim.deb_t = time;
		else
			if ( time == 0 || time == sim.t )
			{
				sim.deb_t = sim.t;
				sim.deb_set = true;
				cmd( "focustop .deb" );
			}

		// ignore if log already open
		if ( sim.log_file_ptr == NULL )
		{
			snprintf( fname, MAX_PATH_LENGTH, "%s/%s", model_path, LOG_FILE );
			sim.log_file_ptr = fopen( fname, "a" );
			sim.log_start = time;
			sim.log_stop = sim.last_t;
		}
	}

	if ( on && ( sim.parallel_mode || sim.fast_mode > 0 ) )
		plog( "\nWarning: %s is active, debug command ignored", sim.parallel_mode ? "parallel processing" : "fast mode" );
}


/****************************************************
NEED_RES_DIR
Evaluate if a separated results directory must be
created according to a set of criteria
****************************************************/
#define RES_AVOID_PATTERN "*.cpp *.h *.txt *.R *.o *.exe *.html"
bool need_res_dir( const char *dest_path, const char *sim_name, char *buf, int buf_sz )
{
	bool newDir = false;

	cmd( "if { [ string length \"%s\" ] > 0 } { \
			set f [ file normalize \"%s/%s\" ]; \
		} else { \
			set f [ file normalize \"%s\" ]; \
		}", dest_path, dest_path, sim_name, sim_name );

	cmd( "set s \".*[ file tail $f ]_\\[0-9\\]+\\.lsd$\"" );
	cmd( "set f \"$f.lsd\"" );
	cmd( "set d [ file dirname $f ]" );

	// check if path is valid
	cmd( "if { [ file exists $d ] && [ file isdirectory $d ] } { set res 1 } { set res 0 }" );
	if ( get_bool( "res" ) )
	{
		// check if in the main model directory
		cmd( "if { $d eq [ file normalize \"%s\" ] } { set res 1 } { set res 0 }", model_path );
		if ( get_bool( "res" ) )
			newDir = true;

		// check if we are in a code directory
		cmd( "set l [ glob -nocomplain -directory $d %s ]", RES_AVOID_PATTERN );
		cmd( "if { [ llength $l ] > 0 } { set res 1 } { set res 0 }" );
		if ( get_bool( "res" ) )
			newDir = true;

		// check if the only LSD configuration is the current one or sensitivity version
		cmd( "set l [ glob -nocomplain -directory $d *.lsd ]" );
		cmd( "set l [ lsearch -exact -all -inline -not $l $f ]" );
		cmd( "set l [ lsearch -regexp -all -inline -not $l $s ]" );

		cmd( "if { [ llength $l ] > 0 } { set res 1 } { set res 0 }" );
		if ( get_bool( "res" ) )
			newDir = true;

		if ( newDir )
			cmd( "set d \"$d/[ file tail \"%s\" ]\"", sim_name );
	}
	else
		cmd( "set d \"\"" );

	get_str( "d", buf, buf_sz );

	return newDir;
}


/****************************************************
CHECK_RES_DIR
Check if the results directory exists and
contains files to be deleted
****************************************************/
#define RES_CLEAR_PATTERN "*.res *.tot *.csv *.gz *.log *.bat *.pdf *.eps *.svg *.Rdata *.bak"
bool check_res_dir( const char *dest_path, const char *sim_name )
{
	bool done;

	cmd( "set d \"%s\"", dest_path );

	cmd( "if { [ file exists $d ] && [ file isdirectory $d ] && [ file normalize $d ] ne [ file normalize \"%s\" ] && [ llength [ glob -nocomplain -directory $d %s ] ] > 0 } { set res 1 } { set res 0 }", model_path, RES_CLEAR_PATTERN );
	done = get_bool( "res" );

	if ( sim_name != NULL )
	{
		cmd( "if { [ file exists $d ] && [ file isdirectory $d ] } { \
				set l [ glob -nocomplain -directory $d *.lsd ]; \
				set n [ llength $l ]; \
				if { $n > 1 } { \
					set res 1 \
				} elseif { $n == 1 && [ file normalize [ lindex $l 0 ] ] ne [ file normalize \"$d/%s.lsd\" ] } { \
					set res 1 \
				} else { \
					set res 0 \
				} \
			}", clean_file( sim_name ) );

		done |= get_bool( "res" );
	}

	return done;
}


/****************************************************
CREATE_RES_DIR
Create the results directory, if not exists yet
****************************************************/
bool create_res_dir( const char *dest_path )
{
	cmd( "set d \"%s\"", dest_path );

	cmd( "if { [ file exists $d ] && [ file isdirectory $d ] } { set res 1 } { set res 0 }" );
	if ( ! get_bool( "res" ) )
	{
		cmd( "if { [ file exists $d ] } { catch { file delete -force $d } }" );
		cmd( "catch { file mkdir $d }" );
		cmd( "if { ! [ file exists $d ] || ! [ file isdirectory $d ] } { set res 1 } { set res 0 }" );
		if ( get_bool( "res" ) )
			return false;
	}

	return true;
}


/****************************************************
CLEAN_RES_DIR
Clear LSD produced files in the results directory,
if existent,
****************************************************/
void clean_res_dir( const char *dest_path, const char *sim_name )
{
	cmd( "set d \"%s\"", dest_path );

	cmd( "if { [ file exists $d ] && [ file isdirectory $d ] } { \
			set l [ glob -nocomplain -directory $d %s ]; \
			if { [ llength $l ] > 0 } { \
				catch { file delete -force {*}$l } \
			} \
		}", RES_CLEAR_PATTERN );

	if ( sim_name != NULL )
		cmd( "if { [ file exists $d ] && [ file isdirectory $d ] } { \
				set l [ glob -nocomplain -directory $d *.lsd ]; \
				foreach f $l { \
					if { [ file normalize $f ] ne [ file normalize \"$d/%s.lsd\" ] } { \
						catch { file delete -force $f } \
					} \
				} \
			}", clean_file( sim_name ) );
}


/*****************************************************************************
LOAD_SENSITIVITY
	Load defined sensitivity analysis configuration
	Returns: 0: load ok, 1,2,3,4,...: load failure
******************************************************************************/
int load_sensitivity( FILE *f )
{
	bool integer;
	d_vecT val;
	int i, lag, param, num_val;
	char cc, lab[ MAX_ELEM_LENGTH ];
	variable *cv;
	sensitivity *cs;

	// read data from file (1 line per element, '#' indicate comment)
	while ( ! feof( f ) )
	{	// read element by element, skipping comments
		fscanf( f, "%99s", lab );			// read string
		while ( lab[ 0 ] == '#' )			// start of a comment
		{
			do								// jump to next line
				cc = fgetc( f );
			while ( ! feof( f ) && cc != '\n' );
			fscanf( f, "%99s", lab );		// try again
		}

		if ( feof( f ) )					// ended too early?
			break;

		cv = sim.root->search_var( sim.root, lab );
		if ( cv == NULL || ( cv->param != 1 && cv->num_lag == 0 ) )
			goto error1;					// and not parameter or lagged variable

		// get lags and # of values to test
		if ( fscanf( f, "%d %d ", & lag, & num_val ) < 2 )
			goto error2;

		// get variable type (newer versions)
		if ( fscanf( f, "%c ", &cc ) < 1 )
			goto error3;

		if ( cc == 'i' || cc == 'd' || cc == 'f' )
		{
			integer = ( cc == 'i' ) ? true : false;
			fscanf( f, ": " );				// remove separator
		}
		else
			if ( cc == ':' )
				integer = false;
			else
				goto error4;

		if ( lag == 0 )						// adjust type and lag #
			param = 1;
		else
		{
			param = 0;
			lag = abs( lag ) - 1;
		}

		for ( val.resize( num_val ), i = 0; i < num_val; ++i )
			if ( ! fscanf( f, "%lf", & val[ i ] ) )
				goto error5;

		if ( ( cs = search_sensitivity( lab, lag ) ) != NULL )
			delete cs;

		new sensitivity( lab, & sim, param, lag, integer, num_val, & val );
	}

	return 0;

	// error handling
	error1:
		if ( cv != NULL )
			cmd( "ttk::messageBox -parent . -title Error -icon error -type ok -message \"Invalid lag selected\" -detail \"Variable '%s' has no lags set.\"", lab );
		i = 1;
		goto error;

	error2:
		cmd( "ttk::messageBox -parent . -title Error -icon error -type ok -message \"Invalid range\" -detail \"Element '%s' has less than two values to test.\"", lab );
		i = 2;
		goto error;

	error3:
		cmd( "ttk::messageBox -parent . -title Error -icon error -type ok -message \"Invalid element type\" -detail \"Element '%s' has an invalid value set.\"", lab );
		i = 3;
		goto error;

	error4:
		cmd( "ttk::messageBox -parent . -title Error -icon error -type ok -message \"Missing separator\" -detail \"Element '%s' has no separator character (':').\"", lab );
		i = 4;
		goto error;

	error5:
		cmd( "ttk::messageBox -parent . -title Error -icon error -type ok -message \"Invalid range value\" -detail \"Element '%s' has non-numeric range values.\"", lab );
		i = 5;
		goto error;

	error:

	sim.empty_sensitivity( );					// discard read data

	return i;
}


/*****************************************************************************
SAVE_SENSITIVITY
	Save current sensitivity configuration to file
	Returns: true: save ok, false: save failure
******************************************************************************/
bool save_sensitivity( FILE *f )
{
	int i;
	sensitivity *cs;

	for ( cs = sim.sens; cs != NULL; cs = cs->next )
	{
		if ( cs->param == 1 )
			fprintf( f, "%s 0 %d %c:", cs->label, cs->num_val, cs->integer ? 'i' : 'f' );
		else
			fprintf( f, "%s -%d %d %c:", cs->label, cs->lag + 1, cs->num_val, cs->integer ? 'i' : 'f' );

		for ( i = 0; cs->val != NULL && i < cs->num_val; ++i )
			fprintf( f," %g", cs->val[ i ] );

		fprintf( f,"\n" );
	}

	return ! ferror( f );
}


/***************************************************
LOAD_EQFILE
	Load from disk the current equation file
***************************************************/
char *load_eqfile( void )
{
	char s[ MAX_FILE_SIZE ], *buf1, *buf2, *eq;
	int i;
	long sz;
	FILE *f;

	read_eqfile_name( s, MAX_PATH_LENGTH );
	if ( ( f = fopen( s, "r" ) ) == NULL )
	{
		cmd( "ttk::messageBox -parent . -title Warning -icon warning -type ok -message \"Equation file not found\" -detail \"File '%s' missing, cannot upload the equation file.\nYou may have to restore your equation file using the copy in the configuration file (menu File > Restore Equation File).\"", s );
		return NULL;
	}

	// obtain file size
	for ( sz = 0, i = 1; i > 0; sz += i )
		i = fread( ( void * ) s, 1, MAX_FILE_SIZE, f );

	rewind( f );

	buf1 = new char[ sz + 1 ];
	strcpy( buf1, "" );
	fread( ( void * ) buf1, 1, sz, f );
	fclose( f );
	buf1[ sz ] = '\0';

	// remove extra clear space at the beginning/end and standardize line ends
	buf2 = new char[ sz + 1 ];
	sz = strcln( buf2, buf1, sz + 1 );

	eq = new char[ sz + 1 ];
	strcpyn( eq, buf2, sz + 1 );

	delete [ ] buf1;
	delete [ ] buf2;

	return eq;
}


/***************************************************
READ_EQFILE_NAME
	Get the file name of the current equation file
***************************************************/
void read_eqfile_name( char *s, int sz )
{
	char lab[ MAX_PATH_LENGTH ];
	FILE *f;

	snprintf( lab, MAX_PATH_LENGTH, "%s/%s", model_path, MODEL_OPTIONS );
	f = fopen( lab, "r" );

	if ( f == NULL )
	{
		cmd( "ttk::messageBox -parent . -title Error -icon error -type ok -message \"File not found\" -detail \"File '$MODEL_OPTIONS' missing, cannot upload the equation file.\nYou may have to recreate your model configuration.\"" );
		return;
	}

	fscanf( f, "%999s", lab );
	for ( int i = 0; strncmp( lab, "FUN=", 4 ) && fscanf( f, "%999s", lab ) != EOF && i < MAX_FILE_TRY; ++i );
	fclose( f );
	if ( strncmp( lab, "FUN=", 4 ) != 0 )
	{
		cmd( "ttk::messageBox -parent . -type ok -title -title Error -icon error -message \"File corrupted\" -detail \"File '$MODEL_OPTIONS' has invalid contents, cannot upload the equation file.\nYou may have to recreate your model configuration.\"" );
		return;
	}

	strcpyn( s, lab + 4, sz );
	strcatn( s, ".cpp", sz );

	return;
}


/****************************************************
GET_SAVED
	Get the set of elements which values are saved
	during simulation run
****************************************************/
void object::get_saved( FILE *out, const char *sep, bool all_var )
{
	int i, sl;
	char *lab;
	bridge *cb;
	description *cd;
	object *cur;
	variable *cv;

	for ( cv = v; cv != NULL; cv = cv->next )
		if ( cv->save || all_var )
		{
			// get element description
			cd = sim->search_description( cv->label, false );
			if ( cd != NULL && cd->text != NULL && ( sl = strlen( cd->text ) ) > 0 )
			{
				// select just the first description line
				lab = new char[ sl + 1 ];
				strcpy( lab, cd->text );
				for ( i = 0; i < sl; ++i )
					if ( lab[ i ] == '\n' || lab[ i ] == '\r' )
					{
						lab[ i ] = '\0';
						break;
					}
			}
			else
				lab = NULL;

			fprintf( out, "%s%s%s%s%s%s%s\n", cv->label, sep, cv->param ? "parameter" : "variable", sep, label, sep, lab != NULL ? lab : "" );
		}

	for ( cb = b; cb != NULL; cb = cb->next )
	{
		if ( cb->head == NULL )
			cur = sim->blueprint->search( cb->blabel );
		else
			cur = cb->head;

		cur->get_saved( out, sep, all_var );
	}
}


/****************************************************
GET_SA_LIMITS
	Get the max-min limits used for sensitivity
	analysis of variables
****************************************************/
const char *meta_par_name[ META_PAR_NUM ] = META_PAR_NAME;

void object::get_sa_limits( FILE *out, const char *sep )
{
	int i, sl;
	char *lab, type[ 10 ];
	variable *cv;
	description *cd;
	sensitivity *cs;

	for ( i = 0; i < META_PAR_NUM; ++i )
		meta_par_in[ i ] = false;

	for ( cs = sim->sens; cs != NULL; cs = cs->next )
	{
		// get current value (first object)
		cv = search_var( NULL, cs->label );

		// get element description
		cd = sim->search_description( cs->label, false );
		if ( cd != NULL && cd->text != NULL && ( sl = strlen( cd->text ) ) > 0 )
		{
			// select just the first description line
			lab = new char[ sl + 1 ];
			strcpy( lab, cd->text );
			for ( i = 0; i < sl; ++i )
				if ( lab[ i ] == '\n' || lab[ i ] == '\r' )
				{
					lab[ i ] = '\0';
					break;
				}
		}
		else
			lab = NULL;

		// find max and min values
		double min = HUGE_VAL, max = - HUGE_VAL;
		for ( i = 0; cs->val != NULL &&  i < cs->num_val; ++i )
			if ( cs->val[ i ] < min )
				min = cs->val[ i ];
			else
				if ( cs->val[ i ] > max )
					max = cs->val[ i ];

		// check meta-parameters
		if ( cs->param == 1 )
		{
			strcpy( type, "parameter" );

			for ( i = 0; i < META_PAR_NUM; ++i )
				if ( ! strcmp( cs->label, meta_par_name[ i ] ) )
				{
					strcpy( type, "setting" );
					meta_par_in[ i ] = true;
					break;
				}
		}
		else
			strcpy( type, "variable" );

		fprintf( out, "%s%s%s%s%d%s%s%s%g%s%g%s%g%s\"%s\"\n", cs->label, sep, type, sep, cs->param == 1 ? 0 : cs->lag + 1, sep, cs->integer ? "integer" : "real", sep, cv != NULL ? cv->val[ cs->lag ] : NAN, sep, min, sep, max, sep, lab != NULL ? lab : "" );

		delete [ ] lab;
	}
}


/***************************************************
COUNT_LINES
	Counts the number of lines in a text file
***************************************************/
int count_lines( const char *fname, bool dozip )
{
	char *res, buf[ FILE_BUF_SIZE ];
	int fend, n = 0;
	FILE *f = NULL;
	gzFile fz = NULL;

	if ( ! dozip )
		f = fopen( fname, "rt" );
	else
		fz = gzopen( fname, "rt" );

	if ( f == NULL && fz == Z_NULL )
		return 0;

	do
	{
		if ( ! dozip )
		{
			res = fgets( buf, FILE_BUF_SIZE, f );
			fend = feof( f );
		}
		else
		{
			res = gzgets( fz, buf, FILE_BUF_SIZE );
			fend = gzeof( fz );
		}

		if ( res == NULL )
			return n;

		if ( fend || strchr( buf, '\n' ) != NULL )
			++n;
	}
	while ( ! fend );

	return n;
}


/****************************************************
SHOW_LOGS
	Open tail/multitail to show log files dynamically
****************************************************/
void show_logs( const char *dest_path, s_vecT & logs, bool par_cntl )
{
	char exec[ MAX_PATH_LENGTH	];
	int i, j, n, sz;

	cmd( "switch [ ttk::messageBox -parent . -type yesno -default yes -icon info -title \"Background run monitor\" -message \"Open the background run monitor?\" -detail \"The selected simulation runs were started as parallel background job(s). Each job progress can be monitored in a separated window results by choosing 'Yes'\n\nLog files are being created in the folder:\n\n[ fn_break [ file nativename \"%s\" ] 40 ]\" ] { yes { set res 1 } no { set res 0 } }", dest_path );

	if ( ! get_int( "res" ) || ( par_cntl && ! sim.parallel_monitor ) )
		return;

	l_guardT lock( sim.run_logs_lck );

	n = logs.size( );
	if ( n == 0 )
		return;

	for ( i = j = 0; i < n; ++i )
		j += logs[ i ].length( );

	sz = i + j + 1;
	char logs_str[ sz ];
	strcpy( logs_str, "" );

	for ( i = 0; i < n; ++i )
	{
		strcatn( logs_str, logs[ i ].c_str( ), sz );

		if ( i < n - 1 )
			strcatn( logs_str, " ", sz );
	}

	if ( n == 1 )
		strcpy( exec, "tail -n 20 -F" );
	else
	{
		// number of terminal columns
		j = n > 4 ? ( n > 8 ? ( n > 12 ? 4 : 3 ) : 2 ) : 1;

		if ( j == 1 )
			snprintf( exec, MAX_PATH_LENGTH , "multitail%s", platform == _WIN_ ? "" : " --retry-all" );
		else
			snprintf( exec, MAX_PATH_LENGTH , "multitail%s -s %d", platform == _WIN_ ? "" : " --retry-all", j );
	}

	cmd( "if { [ open_terminal \"%s %s\" ] != 0 } { \
			ttk::messageBox -parent . -type ok -icon error -title Error -message \"%s failed to launch\" -detail \"Please check if %s is installed and set up properly.\n\nDetail:\n$termResult\" \
		}", exec, logs_str, exec, exec );
}
