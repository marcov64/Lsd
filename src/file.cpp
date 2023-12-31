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
Contains the methods and functions used to save and load a model. The
data for a model are composed by a structure, initial values and simulation
setting.

The main methods of object contained in this file are:

- void object::save_struct( FILE *f, char *tab )
Saves the structure of the object (that is, the label,
variables and parameters and descendants, not number of objects).
This method is called first time by the browser in INTERF.CPP by the root of the
model.
Calls the save_struct for all the descendant type.

- void object::save_insts( FILE *f )
Save the numerical values for object instances (one digit
for each group of object of this type) and the initial values for variables.
It save also option information, that is whether to save, plot or debug the
variables.
It calls the save_insts for all the descendant type.
It is called in the browser, INTERF.CPP, immediately after save_struct, by the
root of the model.

- void object::load_struct( FILE *f )
Initialize a model by creating	one as defined
in the data file. The model, after this stage, has only one instance for each
object type and variables and parameters are simply labels.

- int object::load_insts( const char *file_name )
It loads from the file named as specified the instance data
for the objects. It is made in specular way in respect of save_insts.
*************************************************************/

#include "decl.h"


#ifndef _NW_

/****************************************************
OPEN_CONFIGURATION
	Open a clean configuration,
	either the current or not
****************************************************/
bool open_configuration( object *&r, bool reload )
{
	int i;
	const char *lab1, *lab2;

	if ( ! reload || strlen( simul_name ) == 0 )
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

			delete [ ] simul_name;
			simul_name = new char[ strlen( lab2 ) + 1 ];
			strcpy( simul_name, lab2 );

			delete [ ] path;
			path = new char[ strlen( lab1 ) + 1 ];
			strcpy( path, lab1 );

			if ( strlen( path ) > 0 )
				cmd( "cd $path" );

			cmd( "set listfocus 1; set itemfocus 0" );// point for first var in listbox
			cmd( "set lastObj \"\"" );			// disable last object for reload
		}
		else
			if ( struct_loaded )
				reload = true;					// try to reload if use cancel load
			else
				return false;
	}

	if ( r != NULL && reload )
		save_pos( r );							// save current position when reloading

	redrawRoot = redrawStruc = true;			// force browser/structure redraw
	iniShowOnce = false;						// show warning on # of columns in .ini

	switch ( i = load_configuration( reload ) )	// try to load the configuration
	{
		case 1:									// file/path not found
			if ( strlen( path ) > 0 )
				cmd( "ttk::messageBox -parent . -type ok -title Error -icon error -message \"File not found\" -detail \"File for model '%s' not found in directory '%s'.\"", strlen( simul_name ) > 0 ? simul_name : NO_CONF_NAME, path );
			else
				cmd( "ttk::messageBox -parent . -type ok -title Error -icon error -message \"File not found\" -detail \"File for model '%s' not found in current directory\"", strlen( simul_name ) > 0 ? simul_name : NO_CONF_NAME	 );
			return false;

		case 2:
		case 3:
			cmd( "ttk::messageBox -parent . -type ok -title Error -icon error -message \"Invalid or damaged file (%d)\" -detail \"Please check if a proper file was selected.\"", i );
			return false;

		case 4:
		case 5:
		case 6:
		case 7:
		case 8:									// problem from MODELREPORT section
		case 9:									// problem from DESCRIPTION section
			cmd( "ttk::messageBox -parent . -type ok -title Error -icon error -message \"Partially damaged file (%d)\" -detail \"Element descriptions were lost but the configuration can still be used.\n\nPlease check if the desired file was selected or re-enter the description information if needed.\n\nIf this is a sensitivity analysis configuration file, this message is expected, and configuration file is ok.\"", i );
			reset_description( root );
			break;

		case 10:								// problem from DOCUOBSERVE section
		case 11:
		case 12:								// problem from DOCUINITIAL section
		case 13:
			cmd( "ttk::messageBox -parent . -type ok -title Error -icon error -message \"Partially damaged file (%d)\" -detail \"Observation flags and equation file were lost but the configuration can still be used.\n\nPlease check if the desired file was selected or re-configure the lost parts if needed.\"", i );
	}

	if ( r != NULL && reload )
		currObj = r = restore_pos( root );		// restore pointed object and variable
	else
		currObj = r = root;						// new structure

	return true;
}

#endif

/*****************************************************************************
LOAD_CONFIGURATION
	Load current defined configuration from file (xml, gzip xml or legacy text)
	If quick is != 0, just the structure and the parameters are retrieved
	Returns: 0: load ok, 1,2,3,4,...: load failure
******************************************************************************/
int load_configuration( bool reload, int quick )
{
	char *buf = NULL, buf1[ MAX_FILE_SIZE ], msg[ MAX_LINE_SIZE ], name[ MAX_PATH_LENGTH ], full_name[ 2 * MAX_PATH_LENGTH ];
	int i, j, load = 0;
	object *cur;
	variable *cv, *cv1;
	description *cd;
	FILE *g, *f = NULL;
	gzFile fz;
	xml_doc xf;

	unload_configuration( false );				// unload current

	if ( strlen( simul_name ) == 0 )
		return 1;

	if ( ! reload || strlen( struct_file ) == 0 )
	{
		delete [ ] struct_file;
		struct_file = new char[ strlen( path ) + strlen( simul_name ) + 6 ];
		sprintf( struct_file, "%s%s%s.lsd", path, strlen( path ) > 0 ? "/" : "", simul_name );
	}

	// try to open maybe compressed xml configuration
	fz = gzopen( struct_file, "rb" );
	if ( fz == Z_NULL )
		return 1;

	// compute xml file size
	for ( i = 0, j = 1; j > 0; i += j )
		j = gzread( fz, ( void * ) buf1, MAX_FILE_SIZE );

	if ( i == 0 )
	{
		gzclose( fz );
		return 1;
	}

	gzrewind( fz );
	buf = static_cast < char * >( pugi::get_memory_allocation_function( )( i ) );
	j = gzread( fz, ( void * ) buf, i );
	gzclose( fz );

	if ( j < i )
		return 1;

	// set default values
	max_step = 100;
	sim_num = seed = 1;
	when_debug = stack_info = prof_min_msecs = 0;
	prof_obs_only = prof_aggr_time = no_ptr_chk = parallel_disable = false;
	snprintf( name_rep, MAX_PATH_LENGTH, "report_%s.html", simul_name );

	// try to read xml configuration
	auto res = xf.load_buffer_inplace_own( buf, i, pugi::parse_default |
										   pugi::parse_doctype |
										   pugi::parse_trim_pcdata );
	if ( res.status == pugi::status_ok )
	{
		xml_node typeNode = xf.first_child( );	// document type node
		xml_node lsdNode = xf.document_element( );	// LSD top element

		if ( strstr( typeNode.value( ), "LSD " ) != typeNode.value( ) ||
			 strcmp( lsdNode.name( ), "LSD" ) != 0 )
			return 1;							// invalid xml type/format

		// get model structure
		xml_node cfgNode = lsdNode.child( "configuration" );// load config.
		xml_node rootNode = cfgNode.child( "structure" ).child( "object" );

		if ( rootNode.empty( ) )
			return 1;							// missing root

		if ( ! reload || quick != 0 )
			empty_description( );				// remove existing descriptions

		// load non-instanced model structure
		struct_loaded = root->load_xml_struct( rootNode,
										( reload && quick == 2 ) || quick == 1 );
		if( ! struct_loaded )
		{
			load = 2;
			goto endLoad;
		}

		// load model structure instances
		if( ! root->load_xml_insts( rootNode ) )
		{
			load = 3;
			goto endLoad;
		}

		if ( reload && quick == 2 )				// just quick reload?
			goto endLoad;

		xml_node setNode = cfgNode.child( "settings" );
		xml_node simNode = setNode.child( "simulation" );

		if ( setNode.empty( ) || simNode.empty( ) )	// missing settings
		{
			load = 4;
			goto endLoad;
		}

		// get simulation settings
		xml_attr hint;							// speed-up pointer
		max_step = simNode.attribute( "steps", hint ).as_uint( max_step );
		sim_num = simNode.attribute( "runs", hint ).as_uint( sim_num );
		seed = simNode.attribute( "seed", hint ).as_uint( seed );
		when_debug = simNode.attribute( "debug_start", hint ).as_uint( when_debug );
		no_ptr_chk = ! simNode.attribute( "ptr_check", hint ).as_bool( ! no_ptr_chk );
		parallel_disable = ! simNode.attribute( "parallel", hint ).as_bool( ! parallel_disable );
		stack_info = setNode.child( "profiling" ).attribute( "level", hint ).as_uint( stack_info );
		prof_min_msecs = setNode.child( "profiling" ).attribute( "time", hint ).as_uint( prof_min_msecs );
		prof_obs_only = setNode.child( "profiling" ).attribute( "observed", hint ).as_bool( prof_obs_only );
		prof_aggr_time = setNode.child( "profiling" ).attribute( "aggregate", hint ).as_bool( prof_aggr_time );

		// get report file name
		strcpyn( name_rep, setNode.child( "report_file" ).text( ).as_string( name_rep ), MAX_PATH_LENGTH );

		// get equation file name and content
		xml_node eqfNode = cfgNode.child( "equation_file" );
		if ( eqfNode.empty( ) )
		{
			load = 7;
			goto endLoad;
		}

		// use the current equation name only if the file exists
		snprintf( full_name, 2 * MAX_PATH_LENGTH, "%s/%s", exec_path,
				  eqfNode.child( "filename" ).text( ).as_string( "NONE" ) );
		if ( ( f = fopen( full_name, "r" ) ) != NULL )
			strcpyn( equation_name, eqfNode.child( "filename" ).text( ).get( ), MAX_PATH_LENGTH );

		if ( quick != 1 )						// load equation file?
			// decode xml ]]> escape sequences
			strdecdata( lsd_eq_file, eqfNode.child( "content" ).text( ).get( ), MAX_FILE_SIZE );
		else
			strcpy( lsd_eq_file, "" );

		goto endLoad;
	}

	// try to read legacy configuration
	f = fopen( struct_file, "rb" );
	if ( f == NULL )
		return 1;

	struct_loaded = root->load_struct( f );
	if ( ! struct_loaded )
	{
		load = 2;
		goto endLoad;
	}

	strcpy( msg, "" );
	fscanf( f, "%999s", msg );					// should be DATA
	if ( ! ( ! strcmp( msg, "DATA" ) && root->load_insts( struct_file, f ) ) )
	{
		load = 3;
		goto endLoad;
	}

	if ( reload && quick == 2 )					// just quick reload?
		goto endLoad;

	fscanf( f, "%999s", msg );					// should be SIM_NUM
	if ( ! ( ! strcmp( msg, "SIM_NUM" ) && fscanf( f, "%d", & sim_num ) && sim_num > 0 ) )
	{
		load = 4;
		goto endLoad;
	}

	fscanf( f, "%999s", msg );					// should be SEED
	if ( ! ( ! strcmp( msg, "SEED" ) && fscanf( f, "%d", & seed ) && seed > 0 ) )
	{
		load = 5;
		goto endLoad;
	}

	fscanf( f, "%999s", msg );					// should be MAX_STEP
	if ( strcmp( msg, "MAX_STEP" ) )
	{
		load = 6;
		goto endLoad;
	}

	if ( fgets( msg, MAX_LINE_SIZE, f ) == NULL )// should be 1 to 8 values
	{
		load = 6;
		goto endLoad;
	}

	i = sscanf( msg, "%d %d %d %d %d %d %d %d", & max_step, & when_debug, & stack_info, & prof_min_msecs, & prof_obs_only, & prof_aggr_time, & no_ptr_chk, & parallel_disable );

	if ( i < 1 || max_step <= 0 || when_debug < 0 || stack_info < 0 || prof_min_msecs < 0 || prof_obs_only < 0 || prof_obs_only > 1 || prof_aggr_time < 0 || prof_aggr_time > 1 || no_ptr_chk < 0 || no_ptr_chk > 1 || parallel_disable < 0 || parallel_disable > 1 )
	{
		load = 6;
		goto endLoad;
	}

	fscanf( f, "%999s", msg );					  // should be EQUATION
	if ( strcmp( msg, "EQUATION" ) )
	{
		load = 7;
		goto endLoad;
	}

	strcpy( name, "NONE" );
	fgets( name, MAX_PATH_LENGTH, f );
	if ( name[ strlen( name ) - 1 ] == '\n' )
		name[ strlen( name ) - 1 ] = '\0';

	if ( name[ strlen( name ) - 1 ] == '\r' )
		name[ strlen( name ) - 1 ] = '\0';

	// use the current equation name only if the file exists
	snprintf( full_name, 2 * MAX_PATH_LENGTH, "%s/%s", exec_path, name + 1 );
	g = fopen( full_name, "r" );
	if ( g != NULL )
	{
		fclose( g );
		strcpyn( equation_name, name + 1, MAX_PATH_LENGTH );
	}

	fscanf( f, "%999s", msg );					// should be MODELREPORT
	if ( ! ( ! strcmp( msg, "MODELREPORT" ) && fscanf( f, "%999s", name_rep ) ) )
	{
		load = 8;
		goto endLoad;
	}

	empty_description( );						// remove existing descriptions
	strcpy( lsd_eq_file, "" );					// and equation file

	if ( quick == 1 )							// no descriptions
		goto endLoad;

	fscanf( f, "%999s", msg );					// should be DESCRIPTION
	if ( strcmp( msg, "DESCRIPTION" ) )
	{
		load = 9;
		goto endLoad;
	}

	i = fscanf( f, "%999s", msg );				// should be the first description
	for ( j = 0; strcmp( msg, "DOCUOBSERVE" ) && i == 1 && j < MAX_FILE_TRY; ++j )
	{
		i = load_description( msg, f );
		if ( ! fscanf( f, "%999s", msg ) )
			i = 0;
	}

	if ( i == 0 || j >= MAX_FILE_TRY )
	{
		load = 10;
		goto endLoad;
	}

	fscanf( f, "%999s", msg );
	for ( j = 0; strcmp( msg, "END_DOCUOBSERVE" ) && j < MAX_FILE_TRY; ++j )
	{
		cd = search_description( msg );
		if ( cd != NULL )
		{
			cd->observe = true;
			cv = root->search_var( NULL, msg );
			if ( cv != NULL )
				for ( cur = cv->up; cur != NULL; cur = cur->hyper_next( cv->up->label ) )
				{
					cv1 = cur->search_var( NULL, cv->label );
					if ( cv1 != NULL )
						cv1->observe = true;
				}
		}
		fscanf( f, "%999s", msg );
	}

	if ( j >= MAX_FILE_TRY )
	{
		load = 11;
		goto endLoad;
	}

	fscanf( f, "%999s", msg );					// should be the DOCUINITIAL
	if ( strcmp( msg, "DOCUINITIAL" ) )
	{
		load = 12;
		goto endLoad;
	}

	fscanf( f, "%999s", msg );
	for ( j = 0; strcmp( msg, "END_DOCUINITIAL" ) && j < MAX_FILE_TRY; ++j )
	{
		cd = search_description( msg );
		if ( cd != NULL )
			cd->initial = true;
		fscanf( f, "%999s", msg );
	}

	if ( j >= MAX_FILE_TRY )
	{
		load = 13;
		goto endLoad;
	}

	fscanf( f, "%999s\n", msg );				// here is the equation file
	if ( strcmp( msg, "EQ_FILE" ) )
	{
		load = 0;								// optional
		goto endLoad;
	}

	for ( j = 0; fgets( msg, MAX_LINE_SIZE, f ) != NULL && strncmp( msg, "END_EQ_FILE", 11 ) && strlen( lsd_eq_file ) < MAX_FILE_SIZE - MAX_LINE_SIZE && j < MAX_FILE_TRY; ++j )
		strcatn( lsd_eq_file, msg, MAX_FILE_SIZE );

endLoad:

	// remove extra clear space at the beginning/end and standardize line ends
	strcln( buf1, lsd_eq_file, MAX_FILE_SIZE );
	strcpyn( lsd_eq_file, buf1, MAX_FILE_SIZE );

	if ( quick == 0 && ! ignore_eq_file && strncmp( lsd_eq_file, eq_file, min( strlen( lsd_eq_file ), strlen( eq_file ) ) ) )
		plog( "\nWarning: the configuration file has been previously run with different equations\nfrom those used to create the LSD model program.\nChanges may affect the simulation results. You can offload the original\nequations in a new equation file and compare differences using TkDiff in LMM\n(menu File)." );

	if ( f != NULL )
		fclose( f );

	t = 0;

#ifndef _NW_
	if ( load == 0 )
		cmd( "set lastConf [ string map -nocase { \"%s/\" \"\" } [ file normalize \"%s\" ] ]", exec_path, struct_file );
#endif

	return load;
}


/*****************************************************************************
UNLOAD_CONFIGURATION
	Unload the current configuration
	If full is false, just the model data is unloaded
	Returns: pointer to root object
******************************************************************************/
void unload_configuration ( bool full )
{
	empty_blueprint( );							// remove current model structure
	root->delete_obj( );
	root = new object;
	root->init( NULL, "Root" );
	add_description( "Root" );
	reset_blueprint( NULL );

	empty_cemetery( );							// garbage collection
	empty_sensitivity( rsense );				// discard sensitivity analysis data

	save_ok = true;								// valid structure to save
	unsavedData = false;						// no unsaved simulation results
	unsavedSense = false;						// no sensitivity data to save
	rsense = NULL;								// no sense data

	actual_steps = 0;							// reset steps counter
	findexSens = 0;								// reset sensitivity serial number
	nodesSerial = 0;							// reset network node serial number

#ifndef _NW_
	currObj = NULL;								// no current object pointer
	unsaved_change( false );					// signal no unsaved change
	cmd( "destroytop .lat" );					// remove lattice window
	cmd( "unset -nocomplain modObj modElem modVar modPar modFun" );	// no elements in model structure

	if ( ! running )
		cmd( "destroytop .plt" );				// remove run-time plot window
#endif

	if ( full )									// full unload? (no new config?)
	{
		empty_description( );					// remove element descriptions

		delete [ ] path;						// reset current path
		path = new char[ strlen( exec_path ) + 1 ];
		strcpy( path, exec_path );

		delete [ ] simul_name;					// reset simulation name to default
		simul_name = new char[ strlen( "" ) + 1 ];
		strcpy( simul_name, "" );

		delete [ ] struct_file;					// reset structure
		struct_file = new char[ strlen( "" ) + 1 ];

		strcpy( struct_file, "" );
		strcpy( name_rep, "" );
		strcpy( lsd_eq_file, "" );

		struct_loaded = false;

		delete sens_file;						// reset sensitivity file name
		sens_file = NULL;

#ifndef _NW_
		cmd( "set path \"%s\"", path );
		if ( strlen( path ) > 0 )
			cmd( "cd \"$path\"" );

		cmd( "unset -nocomplain lastConf" );	// no last configuration to reload
		cmd( "set listfocus 1; set itemfocus 0" );// point for first var in listbox
		cmd( "set lastObj \"\"" );				// disable last object for reload
		redrawRoot = redrawStruc = true;		// force browser/structure redraw
#endif
	}
}


/****************************************************
OBJECT::LOAD_XML_STRUCT
	Load the object structure tree under this object
	from an xml object node
	If quick is true, just the structure and the
	parameters are retrieved, no descriptions
****************************************************/
const char *type_names[ ] = { "variable", "parameter", "function" };
const int type_num = 3;

bool object::load_xml_struct( xml_node &n, bool quick )
{
	bool obs;
	const char *str, *desc, *init;
	int i;
	bridge *cb;
	variable *cv;

	if ( strcmp( n.attribute( "name" ).value( ), label ) != 0 )
		return false;

	to_compute = n.attribute( "compute" ).as_bool( true );

	// scan contained child objects and elements
	for ( xml_node cn : n.children( ) )
	{
		if ( ! strcmp( cn.name( ), "object" ) )			// add object?
		{
			str = cn.attribute( "name" ).value( );
			if ( strlen( str ) == 0 )
				return false;

			cmd( "lappend modObj %s", str );

			add_obj( str, 1, 0 );
			cb = search_bridge( str );

			if ( cb->head == NULL || ! cb->head->load_xml_struct( cn, quick ) )
				return false;

			if ( ! quick )
			{
				desc = strdecdata( NULL, cn.child( "description" ).child( "text" ).text( ).get( ) );
				add_description( str, 4, desc );
				delete [ ] desc;
			}
		}
		else
			if ( ! strcmp( cn.name( ), "element" ) )	// add element?
			{
				str = cn.attribute( "type" ).value( );
				if ( strlen( str ) == 0 )
					return false;

				for ( i = 0; i < type_num; ++i )
					if ( ! strcmp( str, type_names[ i ] ) )
						break;

				str = cn.attribute( "name" ).value( );
				if ( strlen( str ) == 0 )
					return false;

				switch( i )
				{
					case 0:
						cmd( "lappend modVar %s", str );
						break;
					case 1:
						cmd( "lappend modPar %s", str );
						break;
					case 2:
						cmd( "lappend modFun %s", str );
						break;
					default:
						return false;
				}

				cmd( "lappend modElem %s", str );

				cv = add_empty_var( str );
				cv->param = i;

				if ( ! quick )
				{
					desc = strdecdata( NULL, cn.child( "description" ).child( "text" ).text( ).get( ) );
					init = strdecdata( NULL, cn.child( "description" ).child( "initialization" ).text( ).get( ) );
					obs = cn.child( "documentation" ).attribute( "observe" ).as_bool( );

					add_description( str, i, desc, init, cn.child( "documentation" ).attribute( "initialization" ).as_bool( ), obs );
					cv->observe = obs;

					delete [ ] desc;
					delete [ ] init;
				}
			}
	}

	return true;
}


/****************************************************
OBJECT::LOAD_STRUCT (LEGACY)
	Load the object structure tree under this object
	from a LEGACY text file
****************************************************/
bool object::load_struct( FILE *f )
{
	int i = 0;
	char ch[ MAX_ELEM_LENGTH ];
	bridge *cb;
	variable *cv;

	fscanf( f, "%99s", ch );
	while ( strcmp( ch, "Label" ) && ++i < MAX_FILE_TRY )
		fscanf( f,"%99s", ch );

	if ( i >= MAX_FILE_TRY )
		return false;

	fscanf( f, "%99s", ch );
	if ( label == NULL )
	{
		label = new char[ strlen( ch ) + 1 ];
		strcpy( label, ch );
	}

	i = 0;
	fscanf( f, "%*[{\r\t\n]%99s", ch );
	while ( strcmp( ch, "}" ) && ++i < MAX_FILE_TRY )
	{
		if ( ! strcmp( ch, "Son:" ) )
		{
			fscanf( f, "%*[ ]%99s", ch );
			add_obj( ch, 1, 0 );
			cmd( "lappend modObj %s", ch );

			// find the bridge which contains the object
			cb = search_bridge( ch );

			if ( cb->head == NULL || ! cb->head->load_struct( f ) )
				return false;
		}

		if ( ! strcmp( ch, "Var:" ) )
		{
			fscanf( f, "%*[ ]%99s", ch );
			add_empty_var( ch );
			cmd( "lappend modElem %s", ch );
			cmd( "lappend modVar %s", ch );
		}

		if ( ! strcmp( ch, "Param:" ) )
		{
			fscanf( f, "%*[ ]%99s", ch );
			cv = add_empty_var( ch );
			cv->param = 1;
			cmd( "lappend modElem %s", ch );
			cmd( "lappend modPar %s", ch );
		}

		if ( ! strcmp( ch, "Func:" ) )
		{
			fscanf( f, "%*[ ]%99s", ch );
			cv = add_empty_var( ch );
			cv->param = 2;
			cmd( "lappend modElem %s", ch );
			cmd( "lappend modFun %s", ch );
		}

		fscanf( f, "%*[{\r\t\n]%99s", ch );
	}

	if ( i >= MAX_FILE_TRY )
		return false;

	return true;
}


/****************************************************
OBJECT::LOAD_XML_INSTS
	Load the object instances of tree under this
	object from an xml object node
****************************************************/
bool object::load_xml_insts( xml_node &n )
{
	int i, j;
	string tmp;
	bridge *cb;
	object *cur;
	variable *cv, *cv1;

	if ( strcmp( n.attribute( "name" ).value( ), label ) != 0 )
		return false;

	// split the number of instances string into a integer vector
	string i1( n.child( "counts" ).text( ).get( ) );
	stringstream s1( i1 );
	vector < int > cnt;
	while ( getline( s1, tmp, ',' ) )
		cnt.push_back( stoi( tmp ) );

	// set # of instances for each object group
	for ( i = 0, cur = this; cur != NULL; cur = cur->hyper_next( label ), ++i )
	{
		if ( i >= ( int ) cnt.size( ) )		// inconsistent # of groups
			return false;

		cur->to_compute = to_compute;
		cur->replicate( cnt[ i ] );

		for ( ; go_brother( cur ) != NULL; cur = cur->next );	// go next group
	}

	if ( i < ( int ) cnt.size( ) )			// inconsistent # of groups
		return false;

	for ( cv = v; cv != NULL; cv = cv->next )
	{
		xml_node cn = n.find_child_by_attribute( "element", "name", cv->label );
		if ( cn.empty( ) )
			return false;

		// split the values of instances string into a double vector
		string i2( cn.child( "values" ).text( ).get( ) );
		stringstream s2( i2 );
		vector < double > val;
		while ( getline( s2, tmp, ',' ) )
			val.push_back( stod( tmp ) );

		if ( cv->param != 1 )
			cv->num_lag = cn.attribute( "lags" ).as_uint( );

		cv->save = cn.attribute( "save" ).as_bool( );
		cv->savei = cn.attribute( "save_file" ).as_bool( );
		cv->plot = cn.attribute( "plot" ).as_bool( );
		cv->parallel = cn.attribute( "parallel" ).as_bool( );
		cv->deb_mode = cn.attribute( "debug" ).as_string( "n" )[ 0 ];
		cv->initialized = cn.attribute( "initialized" ).as_bool( true );

		if ( cv->param == 0 )
		{
			cv->delay = cn.attribute( "delay" ).as_uint( );
			cv->delay_range = cn.attribute( "delay_range" ).as_uint( );
			cv->period = cn.attribute( "period" ).as_uint( 1 );
			cv->period_range = cn.attribute( "period_range" ).as_uint( );
		}

		// set values of instances for each variable group
		for ( i = 0, cur = this; cur != NULL; cur = cur->hyper_next( label ), ++i )
		{
			cv1 = cur->search_var( NULL, cv->label );
			cv1->param = cv->param;
			cv1->num_lag = cv->num_lag;
			cv1->save = cv->save;
			cv1->savei = cv->savei;
			cv1->plot = cv->plot;
			cv1->parallel = cv->parallel;
			cv1->deb_mode = cv->deb_mode;
			cv1->initialized = cv->initialized;
			cv1->delay = cv->delay;
			cv1->delay_range = cv->delay_range;
			cv1->period = cv->period;
			cv1->period_range = cv->period_range;
			cv1->observe = cv->observe;

			// set parameters and initial conditions
			cv1->val = new double[ cv1->num_lag + 1 ];

			if ( cv1->param == 1 || cv1->num_lag > 0 )
			{
				if ( i >= ( int ) val.size( ) )	// inconsistent # of groups
					return false;

				for ( j = 0; j < ( cv1->param == 1 ? 1 : cv1->num_lag ); ++j )
					cv1->val[ j ] = val[ i ];
			}

			if ( cv1->param != 1 )				// remove trash from last position
				cv1->val[ cv1->num_lag ] = 0;
		}

		if ( i < ( int ) val.size( ) )			// inconsistent # of groups
			return false;
	}

	for ( cb = b; cb != NULL; cb = cb->next )
	{
		xml_node cn = n.find_child_by_attribute( "object", "name", cb->blabel );
		if ( cb->head == NULL || ! cb->head->load_xml_insts( cn ) )
			return false;
	}

	if ( up == NULL )	// this is the root, and therefore the end of the loading
		set_blueprint( blueprint, this );

	return true;
}


/****************************************************
OBJECT::LOAD_INSTS (LEGACY)
	Load the object instances of tree under this
	object from a LEGACY text file
****************************************************/
bool object::load_insts( const char *file_name, FILE *f )
{
	char str[ MAX_ELEM_LENGTH ], ch1, ch2, ch3, ch4;
	int num, i;
	double app;
	fpos_t pos;
	bridge *cb;
	object *cur;
	variable *cv, *cv1;

	if ( f == NULL )
		f = search_data_str( file_name, "DATA", label );
	else
	{
		fscanf( f, "%99s", str );		// skip the 'Object: '
		fscanf( f, " %99s", str );		// skip the 'label'
	}

	if ( f == NULL )
		return false;

	if ( fscanf( f, " %c", &ch1 ) != 1 )
		return false;

	if ( ch1 == 'C' )
		to_compute = true;
	else
		to_compute = false;

	for ( cur = this; cur != NULL; cur = cur->hyper_next( cur->label ) )
	{
		if ( fscanf( f, "\t%d", &num ) != 1 )
			return false;

		cur->to_compute = to_compute;
		cur->replicate( num );

		for ( ; go_brother( cur ) != NULL; cur = cur->next );
	}

	for ( cv = v; cv != NULL; cv = cv->next )
	{
		fscanf( f, "%99s ", str );		// skip the 'Element: '
		fscanf( f, "%99s ", str );		// skip the 'label'

		if ( f == NULL )
			return false;

		if ( fscanf( f, "%d %c %c %c %c", &( cv->num_lag ), &ch1, &ch2, &ch3, &ch4 ) != 5 )
			return false;

		cv->save = ( tolower( ch1 ) == 's' ) ? true : false;
		cv->savei = ( ch1 == 'S' || ch1 == 'N' ) ? true : false;
		cv->initialized = ( ch2 == '+' ) ? true : false;
		cv->deb_mode = ch3;
		cv->plot = ( tolower( ch4 ) == 'p' ) ? true : false;
		cv->parallel = ( ch4 == 'P' || ch4 == 'N' ) ? true : false;

		for ( cur = this; cur != NULL; cur = cur->hyper_next( label ) )
		{
			cv1 = cur->search_var( NULL, cv->label );
			cv1->val = new double[ cv->num_lag + 1 ];
			cv1->param = cv->param;
			cv1->num_lag = cv->num_lag;
			cv1->save = cv->save;
			cv1->savei = cv->savei;
			cv1->plot = cv->plot;
			cv1->initialized = cv->initialized;
			cv1->deb_mode = cv->deb_mode;
			cv1->parallel = cv->parallel;

			if ( cv1->param == 1 )
			{
				if ( fscanf( f, "%lf", &app ) != 1 )
					return false;
				else
					cv1->val[ 0 ] = app;
			}
			else
			{
				for ( i = 0; i < cv->num_lag; ++i )
					if ( fscanf( f, "\t%lf", &app ) != 1 )
						return false;
					else
						// place values shifted one position, since they are "time 0" values
						cv1->val[ i ] = app;

				cv1->val[ cv->num_lag ] = 0;
			}
		}

		// check for non-default updating scheme
		if ( cv->param == 0 )
		{
			fgetpos( f, & pos );
			num = fscanf( f, "\t<upd: %d %d %d %d>", & cv->delay, & cv->delay_range, & cv->period, & cv->period_range );

			if ( num > 0 && num < 4 )
				return false;

			if ( num > 0 )
				for ( cur = this; cur != NULL; cur = cur->hyper_next( label ) )
				{
					cv1 = cur->search_var( NULL, cv->label );
					cv1->delay = cv->delay;
					cv1->delay_range = cv->delay_range;
					cv1->period = cv->period;
					cv1->period_range = cv->period_range;
				}
			else
				fsetpos( f, & pos );
		}
	}

	for ( cb = b; cb != NULL; cb = cb->next )
	{
		if ( cb->head == NULL || ! cb->head->load_insts( file_name, f ) )
			return false;
		num = 0;
	}

	if ( up == NULL )	// this is the root, and therefore the end of the loading
		set_blueprint( blueprint, this );

	return true;
}


/****************************************************
LOAD_DESCRIPTION (LEGACY)
	Load the descriptions of elements of tree under
	this object from a LEGACY text file
****************************************************/
bool load_description( const char *d, FILE *f )
{
	int j, type, ctype;
	char label[ MAX_ELEM_LENGTH ], text[ 10 * MAX_LINE_SIZE + 1 ], init[ 10 * MAX_LINE_SIZE + 1 ], str[ 10 * MAX_LINE_SIZE + 1 ];
	variable *cv;

	strcpy( text, "" );
	strcpy( init, "" );
	strcpy( str, "" );

	if ( strncmp( d, "Object", 6 ) == 0 )
	{
		type = 4;
		strcpyn( label, d + 7, MAX_ELEM_LENGTH );
	}
	else
		if ( strncmp( d, "Variable", 8 ) == 0 )
		{
			type = 0;
			strcpyn( label, d + 9, MAX_ELEM_LENGTH );
		}
		else
			if ( strncmp( d, "Parameter", 9 ) == 0 )
			{
				type = 1;
				strcpyn( label, d + 10, MAX_ELEM_LENGTH );
			}
			else
				if ( strncmp( d, "Function", 6 ) == 0 )
				{
					type = 2;
					strcpyn( label, d + 9, MAX_ELEM_LENGTH );
				}
				else
					return false;

	// check correct type and ignore orphan entries
	if ( root->search( label ) != NULL )
		ctype = 4;
	else
		if ( ( cv = root->search_var( NULL, label ) ) != NULL )
			ctype = cv->param;
		else
			ctype = -1;

	if ( ctype < 0 )
		return true;			// ignore orphan (old LSD bug)
	else
		type = ctype;			// silently fix wrong type (old LSD bug)

	fgets( str, MAX_LINE_SIZE, f );		// skip first newline character
	for ( j = 0 ; fgets( str, MAX_LINE_SIZE, f ) != NULL && strncmp( str, END_DESCR, strlen( END_DESCR ) ) && strncmp( str, BEG_INIT, strlen( BEG_INIT ) ) && strlen( text ) <= 9 * MAX_LINE_SIZE && j < MAX_FILE_TRY ; ++j )
		strcatn( text, str, 10 * MAX_LINE_SIZE + 1 );

	if ( strncmp( str, END_DESCR, strlen( END_DESCR ) ) && strncmp( str, BEG_INIT, strlen( BEG_INIT ) ) )
		return false;

	if ( ! strncmp( str, BEG_INIT, strlen( BEG_INIT ) ) )
	{
		for ( j = 0 ; fgets( str, MAX_LINE_SIZE, f ) != NULL && strncmp( str, END_DESCR, strlen( END_DESCR ) ) && strlen( init ) <= 9 * MAX_LINE_SIZE && j < MAX_FILE_TRY ; ++j )
			strcatn( init, str, 10 * MAX_LINE_SIZE + 1 );

		if ( strncmp( str, END_DESCR, strlen( END_DESCR ) ) )
			return false;
	}

	add_description( label, type, text, init );

	return true;
}


/*****************************************************************************
SAVE_CONFIGURATION
	Save current defined configuration (adding tag index if appropriate) to
	gzip-compressed xml file
	If quick is true, just the structure and the parameters are saved
	Returns: true: save ok, false: save failure
******************************************************************************/
bool save_configuration( int findex, const char *dest_path, bool quick )
{
	bool save_ok = false;
	int delta, indexDig, save_len;
	char ch[ MAX_PATH_LENGTH ], *save_file, *bak_file = NULL;
	const char *save_path;
	description *cd;
	FILE *f;
	gzFile fz;
	ostringstream buf;
	xml_doc xf;

	delta = ( findex > 0 ) ? sim_num * ( findex - 1 ) : 0;
	indexDig = ( findex > 0 ) ? ( int ) floor( log10( findex ) + 2 ) : 0;

	if ( dest_path == NULL )
		save_path = path;
	else
		save_path = dest_path;

	if ( strlen( simul_name ) == 0 )
	{
		delete [ ] simul_name;
		simul_name = new char[ strlen( DEF_CONF_FILE ) + 1 ];
		strcpy( simul_name, DEF_CONF_FILE );
	}

	if ( strlen( name_rep ) == 0 )
		snprintf( name_rep, MAX_PATH_LENGTH, "report_%s.html", simul_name );

	if ( strlen( path ) > 0 )
	{
		save_len = strlen( save_path ) + strlen( simul_name ) + 6 + indexDig;
		save_file = new char[ save_len ];
		sprintf( save_file, "%s/%s", save_path, simul_name );
	}
	else
	{
		save_len = strlen( simul_name ) + 6 + indexDig;
		save_file = new char[ save_len ];
		sprintf( save_file, "%s", simul_name );
	}

	if ( findex > 0 )
	{
		snprintf( ch, MAX_PATH_LENGTH, "_%d.lsd", findex );
		strcatn( save_file, ch, save_len );
	}
	else
	{
		// create backup file when not indexed saving
		bak_file = new char[ strlen( save_file ) + 5 ];
		sprintf( bak_file, "%s.bak", save_file );

		strcatn( save_file, ".lsd", save_len );

		f = fopen( save_file, "r" );
		if ( f != NULL )
		{
			fclose( f );

			f = fopen( bak_file, "r" );
			if ( f != NULL )
			{
				fclose( f );

				if( ! remove( bak_file ) )
					rename( save_file, bak_file );
			}
			else
				rename( save_file, bak_file );
		}
	}

	// legacy file save (TO REMOVE)
	if ( false )
	{
		string save_file_leg( save_file );
		save_file_leg.erase( save_file_leg.find_last_of( "." ) );
		save_file_leg += "_leg.lsd";

		f = fopen( save_file_leg.c_str( ), "wb" );
		if ( f != NULL )
		{
			root->save_struct( f, "" );
			fprintf( f, "\nDATA\n" );
			root->save_insts( f );

			fprintf( f, "\nSIM_NUM %d\nSEED %d\nMAX_STEP %d", sim_num, seed + delta, max_step );

			if ( when_debug > 0 || stack_info > 0 || prof_min_msecs > 0 || prof_obs_only || prof_aggr_time || no_ptr_chk || parallel_disable )
				fprintf( f, " %d %d %d %d %d %d %d", when_debug, stack_info, prof_min_msecs, prof_obs_only ? 1 : 0, prof_aggr_time ? 1 : 0, no_ptr_chk ? 1 : 0, parallel_disable ? 1 : 0 );

			fprintf( f, "\nEQUATION %s\nMODELREPORT %s\n", equation_name, name_rep );

			if ( ! quick )
			{
				fprintf( f, "\nDESCRIPTION\n\n" );
				save_description( root, f );

				fprintf( f, "\nDOCUOBSERVE\n" );
				for ( cd = descr; cd != NULL; cd = cd->next )
					if ( cd->observe )
						fprintf( f, "%s\n", cd->label );
				fprintf( f, "\nEND_DOCUOBSERVE\n\n" );

				fprintf( f, "\nDOCUINITIAL\n" );
				for ( cd = descr; cd != NULL; cd = cd->next )
					if ( cd->initial )
						fprintf( f, "%s\n", cd->label );
				fprintf( f, "\nEND_DOCUINITIAL\n\n" );

				save_eqfile( f );
			}

			if ( ! ferror( f ) )
			{
				save_ok = true;

#ifndef _NW_
				cmd( "set lastConf [ string map -nocase { \"%s/\" \"\" } [ file normalize \"%s\" ] ]", exec_path, struct_file );
#endif
			}
		}

		fclose( f );
	}

	// add XML declaration, type and root node
	xml_node declNode = xf.append_child( pugi::node_declaration );
	declNode.append_attribute( "version" ) = "1.0";
	declNode.append_attribute( "encoding" ) = "ANSI";
	declNode.append_attribute( "standalone" ) = "yes";
	xf.append_child( pugi::node_doctype ).set_value( "LSD [\n \
	<!ELEMENT LSD (configuration)>\n \
	<!ELEMENT configuration (settings, structure, equation_file)>\n \
	<!ELEMENT settings (simulation, profiling?, #PCDATA)>\n \
	<!ELEMENT structure (object)>\n \
	<!ELEMENT equation_file (#PCDATA, #CDATA?)>\n \
	<!ELEMENT object (#PCDATA, object*, element*, description?)>\n \
	<!ELEMENT element (#PCDATA?, description?, documentation?)>\n \
	<!ELEMENT description (#PCDATA+)>\n \
	<!ELEMENT documentation EMPTY> ]" );
	xml_node lsdNode = xf.append_child( "LSD" );
	xml_node cfgNode = lsdNode.append_child( "configuration" );
	cfgNode.append_attribute( "version" ) = "1.0";

	// add simulation settings
	xml_node setNode = cfgNode.append_child( "settings" );
	xml_node simNode = setNode.append_child( "simulation" );
	simNode.append_attribute( "steps" ) = max_step;
	simNode.append_attribute( "runs" ) = sim_num;
	simNode.append_attribute( "seed" ) = seed + delta;

	// optional settings (include only if non-default)
	if ( when_debug > 0 )
		simNode.append_attribute( "debug_start" ) = when_debug;

	if ( no_ptr_chk )
		simNode.append_attribute( "ptr_check" ) = false;

	if ( parallel_disable )
		simNode.append_attribute( "parallel" ) = false;

	// add profile settings, if any
	if ( stack_info > 0 || prof_min_msecs > 0 || prof_obs_only || prof_aggr_time )
	{
		xml_node profNode = setNode.append_child( "profiling" );

		if ( stack_info > 0 )
			profNode.append_attribute( "level" ) = stack_info;

		if ( prof_min_msecs > 0 )
			profNode.append_attribute( "time" ) = prof_min_msecs;

		if ( prof_obs_only )
			profNode.append_attribute( "observed" ) = true;

		if ( prof_aggr_time )
			profNode.append_attribute( "aggregate" ) = true;
	}

	// add report file name
	setNode.append_child( "report_file" ).text( ) = name_rep;

	// add model structure
	xml_node strNode = cfgNode.append_child( "structure" );
	root->save_xml_struct( strNode, quick );

	// add equation file name and content
	xml_node eqfNode = cfgNode.append_child( "equation_file" );
	eqfNode.append_child( "filename" ).text( ) = equation_name;

	if ( ! quick )
	{
		if ( eq_file != NULL && ( strlen( lsd_eq_file ) == 0 || strcmp( lsd_eq_file, eq_file ) != 0 ) )
			strcpyn( lsd_eq_file, eq_file, MAX_FILE_SIZE );

		// encode xml ]]> escape sequences
		eqfNode.append_child( "content" ).append_child( pugi::node_cdata ).set_value( strencdata( lsd_eq_file, lsd_eq_file, MAX_FILE_SIZE ) );
	}

	xf.save( buf );

	if ( ( fz = gzopen( save_file, "wb9" ) ) != Z_NULL )
	{
		save_ok = gzputs( fz, buf.str( ).c_str( ) );
		save_ok = gzclose( fz ) == Z_OK ? save_ok : false;
	}
	else
		save_ok = false;

	delete [ ] save_file;
	delete [ ] bak_file;

	return save_ok;
}


/****************************************************
OBJECT::SAVE_XML_STRUCT
	Save the object structure tree under this object
	to an xml object
	If quick is true, just the structure and the
	parameters are saved, no descriptions
****************************************************/
void object::save_xml_struct( xml_node &pn, bool quick )
{
	bool first, init;
	char *str, val[ 32 + 1 ];
	int i, count;
	string data;
	bridge *cb;
	description *cd;
	object *cur;
	variable *cv, *cv1;

	xml_node n = pn.append_child( "object" );
	n.append_attribute( "name" ) = label;

	if ( ! to_compute )
		n.append_attribute( "compute" ) = false;

	for ( data = "", first = true, cur = this; cur != NULL;
		  first = false, cur = cur->hyper_next( cur->label ) )
	{
		skip_next_obj( cur, &count );
		ostringstream str;
		str << ( first ? "" : "," ) << count;
		data.append( str.str( ) );
		for ( ; go_brother( cur ) != NULL; cur = cur->next );
	}

	n.append_child( "counts" ).text( ) = data.c_str( );

	if ( ! quick )
	{
		cd = search_description( label );

		if ( ! strwsp( cd->text ) )
		{
			xml_node nd = n.append_child( "description" );
			str = strencdata( NULL, cd->text );
			nd.append_child( "text" ).append_child( pugi::node_cdata ).set_value( str );
			delete [ ] str;
		}
	}

	for ( cb = b; cb != NULL; cb = cb->next )
		if ( cb->head == NULL )
			blueprint->search( cb->blabel )->save_xml_struct( n, quick );
		else
			cb->head->save_xml_struct( n, quick );

	for ( cv = v; cv != NULL; cv = cv->next )
	{
		xml_node cn = n.append_child( "element" );
		cn.append_attribute( "name" ) = cv->label;
		cn.append_attribute( "type" ) = type_names[ cv->param ];

		if ( cv->param != 1 )
			cn.append_attribute( "lags" ) = cv->num_lag;

		// search for uninitialized data
		if ( cv->param == 1 || cv->num_lag > 0 )
		{
			for ( init = true, cur = this; cur != NULL; cur = cur->hyper_next( label ) )
			{
				cv1 = cur->search_var( NULL, cv->label );
				if ( ! cv1->initialized )
				{
					init = false;
					break;
				}
			}

			if ( ! init )
				cn.append_attribute( "initialized" ) = false;
		}

		// save only non-default values
		if ( cv->save )
			cn.append_attribute( "save" ) = true;

		if ( cv->savei )
			cn.append_attribute( "save_file" ) = true;

		if ( cv->plot )
			cn.append_attribute( "plot" ) = true;

		if ( cv->parallel )
			cn.append_attribute( "parallel" ) = true;

		if ( cv->deb_mode != 'n' )
		{
			data = cv->deb_mode;
			cn.append_attribute( "debug" ) = data.c_str( );
		}

		if ( cv->delay > 0 )
			cn.append_attribute( "delay" ) = cv->delay;

		if ( cv->delay_range > 0 )
			cn.append_attribute( "delay_range" ) = cv->delay_range;

		if ( cv->period > 1 )
			cn.append_attribute( "period" ) = cv->period;

		if ( cv->period_range > 0 )
			cn.append_attribute( "period_range" ) = cv->period_range;

		// add initial values
		if ( cv->param == 1 || cv->num_lag > 0 )
		{
			for ( data = "", first = true, cur = this; cur != NULL;
				  first = false, cur = cur->hyper_next( label ) )
			{
				cv1 = cur->search_var( NULL, cv->label );

				for ( i = 0; i < ( cv1->param == 1 ? 1 : cv1->num_lag ); ++i )
				{
					snprintf( val, 32, "%s%.15g", first ? "" : ",",
							  cv1->initialized ? cv1->val[ i ] : 0. );
					data.append( val );
				}
			}

			cn.append_child( "values" ).text( ) = data.c_str( );
		}

		if ( quick )
			continue;

		// add description text
		cd = search_description( cv->label );

		if ( ! strwsp( cd->text ) || ! strwsp( cd->init ) )
		{
			xml_node cnd = cn.append_child( "description" );

			if ( ! strwsp( cd->text ) )
			{
				str = strencdata( NULL, cd->text );
				cnd.append_child( "text" ).append_child( pugi::node_cdata ).set_value( str );
				delete [ ] str;
			}

			if ( ! strwsp( cd->init ) )
			{
				str = strencdata( NULL, cd->init );
				cnd.append_child( "initialization" ).append_child( pugi::node_cdata ).set_value( str );
				delete [ ] str;
			}
		}

		// add documentation marks
		if ( cd->observe || cd->initial )
		{
			xml_node cnd = cn.append_child( "documentation" );

			if ( cd->observe )
				cnd.append_attribute( "observe" ) = true;

			if ( cd->initial )
				cnd.append_attribute( "initialization" ) = true;
		}
	}
}


/****************************************************
OBJECT::SAVE_STRUCT (LEGACY)
	Save the object structure tree under this object
	to a LEGACY text file
****************************************************/
void object::save_struct( FILE *f, const char *tab )
{
	char tab1[ MAX_ELEM_LENGTH ];
	bridge *cb;
	object *o;
	variable *cv;

	if ( up == NULL )
		fprintf( f, "\t\n" );

	strcpyn( tab1, tab, MAX_ELEM_LENGTH );
	fprintf( f, "%sLabel %s\n%s{\n", tab1, label, tab1 );
	strcatn( tab1, "\t", MAX_ELEM_LENGTH );

	for ( cb = b; cb != NULL; cb = cb->next )
	{
		fprintf( f, "%sSon: %s\n", tab1, cb->blabel );
		if ( cb->head == NULL )
			o = blueprint->search( cb->blabel );
		else
			o = cb->head;
		o->save_struct( f, tab1 );
	}

	for ( cv = v; cv != NULL; cv = cv->next )
	{
		if ( cv->param == 0 )
			fprintf( f, "%sVar: %s\n", tab1, cv->label );
		if ( cv->param == 1 )
			fprintf( f, "%sParam: %s\n", tab1, cv->label );
		if ( cv->param == 2)
			fprintf( f, "%sFunc: %s\n", tab1, cv->label );
	}

	fprintf( f, "\n" );
	fprintf( f, "%s}\n\n", tab );
}


/****************************************************
OBJECT::SAVE_INSTS (LEGACY)
	Save the object instances of tree under this
	object to a LEGACY text file
****************************************************/
void object::save_insts( FILE *f )
{
	int i, count;
	char ch1, ch2, ch3, ch4;
	bridge *cb;
	object *cur;
	variable *cv, *cv1;

	fprintf( f, "\nObject: %s", label );

	if ( to_compute )
		fprintf( f, " C" );
	else
		fprintf( f, " N" );

	for ( cur = this; cur != NULL; cur = cur->hyper_next( cur->label ) )
	{
		skip_next_obj( cur, &count );
		fprintf( f, "\t%d", count );
		for ( ; go_brother( cur ) != NULL; cur = cur->next );
	}
	fprintf( f, "\n" );

	for ( cv = v; cv != NULL; cv = cv->next )
	{
		// search for unloaded data
		ch2 = '+';
		if ( cv->param == 1 || cv->num_lag > 0 )
			for ( cur = this; cur != NULL; cur = cur->hyper_next( label ) )
			{
				cv1 = cur->search_var( NULL, cv->label );
				if ( ! cv1->initialized )
				{
					ch2 = '-';
					break;
				}
			}

		// debug mode: character coding for compatibility
		// ch1: n = no save
		//		s = save to memory
		//		S = save to disk
		// ch2: + = initialized
		//		- = not initialized
		// ch3: n = no debug or watch
		//		d = debug only
		//		w = watch only
		//		W = debug and watch
		//		r = watch write only
		//		R = debug and watch write
		// ch4: n = no runtime plot or parallel update
		//		N = parallel update only
		//		p = runtime plot only
		//		P = runtime plot and parallel update

		ch1 = cv->save ? 's' : 'n';
		ch1 = cv->savei ? toupper( ch1 ) : ch1;
		ch3 = cv->deb_mode;
		ch4 = cv->plot ? 'p' : 'n';
		ch4 = cv->parallel ? toupper( ch4 ) : ch4;

		if ( cv->param == 0 )
			fprintf( f, "Var: %s %d %c %c %c %c", cv->label, cv->num_lag, ch1, ch2, ch3, ch4 );
		if ( cv->param == 1 )
			fprintf( f, "Param: %s %d %c %c %c %c", cv->label, cv->num_lag, ch1, ch2, ch3, ch4 );
		if ( cv->param == 2 )
			fprintf( f, "Func: %s %d %c %c %c %c", cv->label, cv->num_lag, ch1, ch2, ch3, ch4 );

		for ( cur = this; cur != NULL; cur = cur->hyper_next( label ) )
		{
			cv1 = cur->search_var( NULL, cv->label );
			if ( cv1->param == 1 )
				if ( cv1->initialized )
					fprintf( f, "\t%.15g", cv1->val[ 0 ] );
				else
					fprintf( f, "\t%c", '0' );
			else
				for ( i = 0; i < cv->num_lag; ++i )
					if ( cv1->initialized )
						fprintf( f, "\t%.15g", cv1->val[ i ] );
					else
						fprintf( f, "\t%c", '0' );
		}

		// add optional special updating data
		if ( cv->param == 0 && ( cv->delay > 0 || cv->delay_range > 0 || cv->period > 1 || cv->period_range > 0 ) )
			fprintf( f, "\t<upd: %d %d %d %d>", cv->delay, cv->delay_range, cv->period, cv->period_range );

		fprintf( f, "\n" );
	}

	for ( cb = b; cb != NULL; cb = cb->next )
		if ( cb->head != NULL )
			cb->head->save_insts( f );
}


/****************************************************
SAVE_DESCRIPTION (LEGACY)
	save the descriptions of elements of tree under
	this object to a LEGACY text file
****************************************************/
void save_description( object *r, FILE *f )
{
	bridge *cb;
	variable *cv;
	description *cd;

	cd = search_description( r->label );

	if ( strwsp( cd->init ) )
		fprintf( f, "%s_%s\n%s\n%s\n\n", cd->type, cd->label, cd->text, END_DESCR );
	else
		fprintf( f, "%s_%s\n%s\n%s\n%s\n%s\n\n", cd->type, cd->label, cd->text, BEG_INIT, cd->init, END_DESCR );

	for ( cv = r->v; cv != NULL; cv = cv->next )
	{
		cd = search_description( cv->label );

		if ( ( cv->param != 1 && cv->num_lag == 0 ) || strwsp( cd->init ) )
			fprintf( f, "%s_%s\n%s\n%s\n\n", cd->type, cd->label, cd->text, END_DESCR );
		else
			fprintf( f, "%s_%s\n%s\n%s\n%s\n%s\n\n", cd->type, cd->label, cd->text, BEG_INIT, cd->init, END_DESCR );
	}

	for ( cb = r->b; cb != NULL; cb = cb->next )
		if ( cb->head != NULL )
			save_description( cb->head, f );
}


/*****************************************************************************
LOAD_SENSITIVITY
	Load defined sensitivity analysis configuration
	Returns: 0: load ok, 1,2,3,4,...: load failure
******************************************************************************/
int load_sensitivity( FILE *f )
{
	int i;
	char cc, lab[ MAX_ELEM_LENGTH ];
	variable *cv;
	sense *cs = rsense;

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

		cv = root->search_var( root, lab );
		if ( cv == NULL || ( cv->param != 1 && cv->num_lag == 0 ) )
			goto error1;					// and not parameter or lagged variable

		// create memory allocation for new variable
		if ( rsense == NULL )				// allocate first element
			rsense = cs = new sense;
		else								// allocate next ones
		{
			cs->next = new sense;
			cs = cs->next;
		}
		cs->v = NULL;						// initialize struct pointers
		cs->next = NULL;

		cs->label = new char[ strlen( lab ) + 1 ];	// save element name
		strcpy( cs->label, lab );

		// get lags and # of values to test
		if ( fscanf( f, "%d %d ", &cs->lag, &cs->nvalues ) < 2 )
			goto error2;

		// get variable type (newer versions)
		if ( fscanf( f, "%c ", &cc ) < 1 )
			goto error3;

		if ( cc == 'i' || cc == 'd' || cc == 'f' )
		{
			cs->integer = ( cc == 'i' ) ? true : false;
			fscanf( f, ": " );				// remove separator
		}
		else
			if ( cc == ':' )
				cs->integer = false;
			else
				goto error4;

		if ( cs->lag == 0 )					// adjust type and lag #
			cs->param = 1;
		else
		{
			cs->param = 0;
			cs->lag = abs( cs->lag ) - 1;
		}

		cs->v = new double[ cs->nvalues ];	// get values
		for ( i = 0; i < cs->nvalues; ++i )
			if ( ! fscanf( f, "%lf", &cs->v[ i ] ) )
				goto error5;
			else
				if ( cs->integer )
					cs->v[ i ] = round( cs->v[ i ] );
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
	empty_sensitivity( rsense );		// discard read data
	rsense = NULL;

	return i;
}


/*****************************************************************************
EMPTY_SENSITIVITY
	Deallocate sensitivity analysis memory
******************************************************************************/
void empty_sensitivity( sense *cs )
{
	if ( cs == NULL )		// prevent invalid calls (last variable)
		return;

	if ( cs->next != NULL )	// recursively start from the end of the list
		empty_sensitivity( cs->next );
#ifndef _NW_
	else
		NOLH_clear( );		// deallocate DoE (last object only)
#endif
	if ( cs->v != NULL )	// deallocate requested memory, if applicable
		delete cs->v;
	if ( cs->label != NULL )
		delete cs->label;

	delete cs;				// suicide
}


/*****************************************************************************
SAVE_SENSITIVITY
	Save current sensitivity configuration
	Returns: true: save ok, false: save failure
******************************************************************************/
bool save_sensitivity( FILE *f )
{
	int i;
	sense *cs;

	for ( cs = rsense; cs != NULL; cs = cs->next )
	{
		if ( cs->param == 1 )
			fprintf( f, "%s 0 %d %c:", cs->label, cs->nvalues, cs->integer ? 'i' : 'f' );
		else
			fprintf( f, "%s -%d %d %c:", cs->label, cs->lag + 1, cs->nvalues, cs->integer ? 'i' : 'f' );
		for ( i = 0; cs->v != NULL && i < cs->nvalues; ++i )
			fprintf( f," %g", cs->v[ i ] );
		fprintf( f,"\n" );
	}

	return ! ferror( f );
}

#ifndef _NW_

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
		return NULL;

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

	snprintf( lab, MAX_PATH_LENGTH, "%s/%s", exec_path, MODEL_OPTIONS );
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

#endif

/***************************************************
SAVE_EQFILE
	Save to disk the current loaded equation file
	from configuration file
***************************************************/
void save_eqfile( FILE *f )
{
	if ( eq_file != NULL && ( strlen( lsd_eq_file ) == 0 || strcmp( lsd_eq_file, eq_file ) != 0 ) )
		strcpyn( lsd_eq_file, eq_file, MAX_FILE_SIZE );

	fprintf( f, "\nEQ_FILE\n" );
	fprintf( f, "%s", lsd_eq_file );
	fprintf( f, "\nEND_EQ_FILE\n" );
}

/*********************************
SAVE_SINGLE
	Save the value of a single
	element to file during run
*********************************/
void save_single( variable *v )
{
	char fn[ MAX_PATH_LENGTH ];
	int i;
	FILE *f;

#ifndef _NP_
	// prevent concurrent use by more than one thread
	rec_lguardT lock( v->parallel_comp );
#endif

	set_lab_tit( v );
	snprintf( fn, MAX_PATH_LENGTH, "%s_%s-%d_%d_seed-%d.res", v->label, v->lab_tit, v->start, v->end, seed - 1 );
	f = fopen( fn, "wt" );			// use text mode for Windows better compatibility

	fprintf( f, "%s %s (%d %d)\t\n", v->label, v->lab_tit, v->start, v->end );

	for ( i = 0; i <= t - 1; ++i )
		if ( i >= v->start && i <= v->end && ! is_nan( v->data[ i - v->start ] ) )	// save NaN as n/a
			fprintf( f,"%lf\t\n", v->data[ i - v->start ] );
		else
			fprintf( f,"%s\t\n", nonavail );

	fclose( f );
}


/***************************************************
RESULT::CONSTRUCTOR
	Open the appropriate file for saving the results
***************************************************/
result::result( const char *fname, const char *fmode, bool dozip, bool docsv )
{
	this->docsv = docsv;
	this->dozip = dozip;		// save local class flag
	if ( dozip )
		fz = gzopen( fname, fmode );
	else
		f = fopen( fname, fmode );
}


/***************************************************
RESULT::TITLE
	Creates header in results file
***************************************************/
void result::title( object *root, int flag )
{
	firstCol = true;

	title_recursive( root, flag );		// output header

	if ( dozip )						// and change line
		gzprintf( fz, "\n" );
	else
		fprintf( f, "\n" );
}


/***************************************************
RESULT::TITLE_RECURSIVE
	Recursively add elements to header of results file
***************************************************/
void result::title_recursive( object *r, int header )
{
	bool single = false;
	bridge *cb;
	object *cur;
	variable *cv;

	for ( cv = r->v; cv != NULL; cv = cv->next )
	{
		if ( cv->save == 1 )
		{
			set_lab_tit( cv );
			if ( ( ! strcmp( cv->lab_tit, "1" ) || ! strcmp( cv->lab_tit, "1_1" ) || ! strcmp( cv->lab_tit, "1_1_1" ) || ! strcmp( cv->lab_tit, "1_1_1_1" ) ) && cv->up->hyper_next( ) == NULL )
				single = true;					// prevent adding suffix to single objects

			if ( header )
			{
				if ( dozip )
				{
					if ( docsv )
						gzprintf( fz, "%s%s%s%s", firstCol ? "" : CSV_SEP, cv->label, single ? "" : "_", single ? "" : cv->lab_tit );
					else
						gzprintf( fz, "%s %s (%d %d)\t", cv->label, cv->lab_tit, cv->start, cv->end );
				}
				else
				{
					if ( docsv )
						fprintf( f, "%s%s%s%s", firstCol ? "" : CSV_SEP, cv->label, single ? "" : "_", single ? "" : cv->lab_tit );
					else
						fprintf( f, "%s %s (%d %d)\t", cv->label, cv->lab_tit, cv->start, cv->end );
				}
			}
			else
			{
				if ( dozip )
				{
					if ( docsv )
						gzprintf( fz, "%s%s%s%s", firstCol ? "" : CSV_SEP, cv->label, single ? "" : "_", single ? "" : cv->lab_tit );
					else
						gzprintf( fz, "%s %s (-1 -1)\t", cv->label, cv->lab_tit );
				}
				else
				{
					if ( docsv )
						fprintf( f, "%s%s%s%s", firstCol ? "" : CSV_SEP, cv->label, single ? "" : "_", single ? "" : cv->lab_tit );
					else
						fprintf( f, "%s %s (-1 -1)\t", cv->label, cv->lab_tit );
				}
			}

			firstCol = false;
		}
	}

	for ( cb = r->b; cb != NULL; cb = cb->next )
	{
		if ( cb->head == NULL )
			continue;

		cur = cb->head;
		if ( cur->to_compute )
		{
			for ( ; cur != NULL; cur = cur->next )
			title_recursive( cur, header );
		}
	}

	if ( r->up == NULL )
	{
		for ( cv = cemetery; cv != NULL; cv = cv->next )
		{
			if ( dozip )
			{
				if ( docsv )
					gzprintf( fz, "%s%s%s%s", firstCol ? "" : CSV_SEP, cv->label, single ? "" : "_", single ? "" : cv->lab_tit );
				else
					gzprintf( fz, "%s %s (%d %d)\t", cv->label, cv->lab_tit, cv->start, cv->end );
			}
			else
			{
				if ( docsv )
					fprintf( f, "%s%s%s%s", firstCol ? "" : CSV_SEP, cv->label, single ? "" : "_", single ? "" : cv->lab_tit );
				else
					fprintf( f, "%s %s (%d %d)\t", cv->label, cv->lab_tit, cv->start, cv->end );
			}

			firstCol = false;
		}
	}
}


/***************************************************
RESULT::DATA
	Adds data to results file in the specified period
***************************************************/
void result::data( object *root, int initstep, int endtstep )
{
	// don't include initialization (t=0) in .csv format
	initstep = ( docsv && initstep < 1 ) ? 1 : initstep;
	// adjust for 1 time step if needed
	endtstep = ( endtstep == 0 ) ? initstep : endtstep;

	for ( int i = initstep; i <= endtstep; i++ )
	{
		firstCol = true;

		data_recursive( root, i );		// output one data line

		if ( dozip )					// and change line
			gzprintf( fz, "\n" );
		else
			fprintf( f, "\n" );
	}
}


/***************************************************
RESULT::DATA_RECURSIVE
	Recursively add data to results file
***************************************************/
void result::data_recursive( object *r, int i )
{
	bridge *cb;
	object *cur;
	variable *cv;

	for ( cv = r->v; cv != NULL; cv = cv->next )
	{
		if ( cv->save == 1 )
		{
			if ( cv->start <= i && cv->end >= i && ! is_nan( cv->data[ i - cv->start ] ) )
			{
				if ( dozip )
				{
					if ( docsv )
						gzprintf( fz, "%s%.*G", firstCol ? "" : CSV_SEP, SIG_DIG, cv->data[ i - cv->start ] );
					else
						gzprintf( fz, "%.*G\t", SIG_DIG, cv->data[ i - cv->start ] );
				}
				else
				{
					if ( docsv )
						fprintf( f, "%s%.*G", firstCol ? "" : CSV_SEP, SIG_DIG, cv->data[ i - cv->start ] );
					else
						fprintf( f, "%.*G\t", SIG_DIG, cv->data[ i - cv->start ] );
				}
			}
			else
			{
				if ( dozip )		// save NaN as n/a
				{
					if ( docsv )
						gzprintf( fz, "%s%s", firstCol ? "" : CSV_SEP, nonavail );
					else
						gzprintf( fz, "%s\t", nonavail );
				}
				else
				{
					if ( docsv )
						fprintf( f, "%s%s", firstCol ? "" : CSV_SEP, nonavail );
					else
						fprintf( f, "%s\t", nonavail );
				}
			}

			firstCol = false;
		}
	}

	for ( cb = r->b; cb != NULL; cb = cb->next )
	{
		if ( cb->head == NULL )
			continue;

		cur = cb->head;
		if ( cur->to_compute )
			for ( ; cur != NULL; cur = cur->next )
				data_recursive( cur, i );
	}

	if ( r->up == NULL )
	{
		for ( cv = cemetery; cv != NULL; cv = cv->next )
		{
			if ( cv->start <= i && cv->end >= i && ! is_nan( cv->data[ i - cv->start ] ) )
			{
				if ( dozip )
				{
					if ( docsv )
						gzprintf( fz, "%s%.*G", firstCol ? "" : CSV_SEP, SIG_DIG, cv->data[ i - cv->start ] );
					else
						gzprintf( fz, "%.*G\t", SIG_DIG, cv->data[ i - cv->start ] );
				}
				else
				{
					if ( docsv )
						fprintf( f, "%s%.*G", firstCol ? "" : CSV_SEP, SIG_DIG, cv->data[ i - cv->start ] );
					else
						fprintf( f, "%.*G\t", SIG_DIG, cv->data[ i - cv->start ] );
				}
			}
			else					// save NaN as n/a
			{
				if ( dozip )
				{
					if ( docsv )
						gzprintf( fz, "%s%s", firstCol ? "" : CSV_SEP, nonavail );
					else
						gzprintf( fz, "%s\t", nonavail );
				}
				else
				{
					if ( docsv )
						fprintf( f, "%s%s", firstCol ? "" : CSV_SEP, nonavail );
					else
						fprintf(f, "%s\t", nonavail );
				}
			}

			firstCol = false;
		}
	}
}


/***************************************************
RESULT::DESTRUCTOR
	Close the results file
***************************************************/
result::~result( void )
{
	if ( dozip )
		gzclose( fz );
	else
		fclose( f );
}


/****************************************************
GET_SAVED
	Get the set of elements which values are saved
	during simulation run
****************************************************/
void get_saved( object *n, FILE *out, const char *sep, bool all_var )
{
	int i, sl;
	char *lab;
	bridge *cb;
	description *cd;
	object *co;
	variable *cv;

	for ( cv = n->v; cv != NULL; cv = cv->next )
		if ( cv->save || all_var )
		{
			// get element description
			cd = search_description( cv->label, false );
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

			fprintf( out, "%s%s%s%s%s%s%s\n", cv->label, sep, cv->param ? "parameter" : "variable", sep, n->label, sep, lab != NULL ? lab : "" );
		}

	for ( cb = n->b; cb != NULL; cb = cb->next )
	{
		if ( cb->head == NULL )
			co = blueprint->search( cb->blabel );
		else
			co = cb->head;
		get_saved( co, out, sep, all_var );
	}
}


/****************************************************
GET_SA_LIMITS
	Get the max-min limits used for sensitivity
	analysis of variables
****************************************************/
const char *meta_par_name[ META_PAR_NUM ] = META_PAR_NAME;

void get_sa_limits( object *r, FILE *out, const char *sep )
{
	int i, sl;
	char *lab, type[ 10 ];
	variable *cv;
	description *cd;
	sense *cs;

	for ( i = 0; i < META_PAR_NUM; ++i )
		meta_par_in[ i ] = false;

	for ( cs = rsense; cs != NULL; cs = cs->next )
	{
		// get current value (first object)
		cv = r->search_var( NULL, cs->label );

		// get element description
		cd = search_description( cs->label, false );
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
		for ( i = 0; cs->v != NULL &&  i < cs->nvalues; ++i )
			if ( cs->v[ i ] < min )
				min = cs->v[ i ];
			else
				if ( cs->v[ i ] > max )
					max = cs->v[ i ];

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

#ifndef _NW_

/****************************************************
SHOW_LOGS
	Open tail/multitail to show log files dynamically
****************************************************/
void show_logs( const char *path, vector < string > & logs, bool par_cntl )
{
	char exec[ MAX_PATH_LENGTH	];
	int i, j, n, sz;

	cmd( "switch [ ttk::messageBox -parent . -type yesno -default yes -icon info -title \"Background run monitor\" -message \"Open the background run monitor?\" -detail \"The selected simulation runs were started as parallel background job(s). Each job progress can be monitored in a separated window results by choosing 'Yes'\n\nLog files are being created in the folder:\n\n[ fn_break [ file nativename \"%s\" ] 40 ]\" ] { yes { set ans 1 } no { set ans 0 } }", path );

	if ( ! get_int( "ans" ) || ( par_cntl && ! parallel_monitor ) )
		return;

	lock_guard < mutex > lock( lock_run_logs );

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

#endif
