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
FILELIB.CPP
Contains the basic functions used to access files in DLL or
no-window executables. The remaining file-oriented functions
are stored in FILE.CPP.

The main methods of object contained in this file are:

- void object::load_struct( FILE *f )
Initialize a model by creating one as defined in the data file.
The model, after this stage, has only one instance for each
object type and variables and parameters are not configured.

- int object::load_insts( const char *file_name )
It loads from the file named as specified the instance data
for the objects. It is made in specular way in respect of
save_insts.
*************************************************************/

#include "lib/libLSD.h"				// LSD library classes


/*****************************************************************************
LOAD_CONFIGURATION
	Load current defined configuration from file (xml, gzip xml or legacy text)
	If quick is != 0, just the structure and the parameters are retrieved
	Returns: 0: load ok, 1,2,3,4,...: load failure
******************************************************************************/
int simulation::load_configuration( bool reload, string *warnings, int quick )
{
	char *buf = NULL, buf1[ MAX_FILE_SIZE ], msg[ MAX_LINE_SIZE ], name[ MAX_PATH_LENGTH ], full_name[ 2 * MAX_PATH_LENGTH ];
	int i, j, load = 0;
	set < int > warning;
	n_mapT node_map;
	object *cur;
	variable *cv, *cv1;
	description *cd;
	FILE *g, *f = NULL;
	gzFile fz;
	xml_doc xf;

	unload_configuration( false );				// unload current

	if ( strlen( conf_name ) == 0 )
		return 1;

	if ( ! reload || strlen( conf_file ) == 0 )
	{
		delete [ ] conf_file;
		conf_file = new char[ strlen( conf_path ) + strlen( conf_name ) + 6 ];
		sprintf( conf_file, "%s%s%s.lsd", conf_path, strlen( conf_path ) > 0 ? "/" : "", conf_name );
	}

	// try to open maybe compressed xml configuration
	fz = gzopen( conf_file, "rb" );
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
			return 21;							// invalid xml type/format

		// get model structure
		xml_node cfgNode = lsdNode.child( "configuration" );// load config.
		xml_node rootNode = cfgNode.child( "structure" ).child( "object" );

		if ( rootNode.empty( ) )
			return 22;							// missing root

		if ( ! reload || quick != 0 )
			empty_description( );				// remove existing descriptions

		// load non-instanced model structure
		load = root->load_xml_struct( rootNode, ( reload && quick == 2 ) || quick == 1 );
		if( load != 0 )
			goto endLoad;

		// load model structure instances
		load = root->load_xml_insts( rootNode, node_map, warning );
		if( load != 0 )
			goto endLoad;

		conf_ok = true;							// minimum configuration is ok
		root->set_blueprint( blueprint );		// set blueprint to initial condition

		if ( reload && quick == 2 )				// just quick reload?
			goto endLoad;

		xml_node setNode = cfgNode.child( "settings" );
		xml_node simNode = setNode.child( "simulation" );

		if ( setNode.empty( ) || simNode.empty( ) )	// missing settings
		{
			load = 23;
			goto endLoad;
		}

		// get simulation settings
		xml_attr hint;							// speed-up pointer
		last_t = simNode.attribute( "steps", hint ).as_uint( MAX_STEPS );
		last_run = simNode.attribute( "runs", hint ).as_uint( 1 );
		seed = simNode.attribute( "seed", hint ).as_uint( 1 );
		deb_t = simNode.attribute( "debug_start", hint ).as_uint( );
		no_ptr_chk = ! simNode.attribute( "ptr_check", hint ).as_bool( true );
		parallel_disable = ! simNode.attribute( "parallel", hint ).as_bool( true );
		stack_info = setNode.child( "profiling" ).attribute( "level", hint ).as_uint( );
		prof_min_msecs = setNode.child( "profiling" ).attribute( "time", hint ).as_uint( );
		prof_obs_only = setNode.child( "profiling" ).attribute( "observed", hint ).as_bool( );
		prof_aggr_time = setNode.child( "profiling" ).attribute( "aggregate", hint ).as_bool( );

		// get report file name
		snprintf( rep_file, MAX_PATH_LENGTH, "report_%s.html", conf_name );
		strcpyn( rep_file, setNode.child( "report_file" ).text( ).as_string( rep_file ), MAX_PATH_LENGTH );

		// get equation file name and content
		xml_node eqfNode = cfgNode.child( "equation_file" );
		if ( eqfNode.empty( ) )
		{
			load = 24;
			goto endLoad;
		}

		// use the current equation name only if the file exists
		snprintf( full_name, 2 * MAX_PATH_LENGTH, "%s/%s", model_path,
				  eqfNode.child( "filename" ).text( ).as_string( "NONE" ) );
		if ( ( f = fopen( full_name, "r" ) ) != NULL )
			strcpyn( conf_eq_file, eqfNode.child( "filename" ).text( ).get( ), MAX_PATH_LENGTH );

		if ( quick != 1 )						// load equation file?
			// decode xml ]]> escape sequences
			strdecdata( conf_eq_txt, eqfNode.child( "content" ).text( ).get( ), MAX_FILE_SIZE );
		else
			strcpy( conf_eq_txt, "" );

		goto endLoad;
	}

	// try to read legacy configuration
	f = fopen( conf_file, "rb" );
	if ( f == NULL )
		return 1;

	if ( ! root->load_struct( f ) )
	{
		load = 2;
		goto endLoad;
	}

	strcpy( msg, "" );
	fscanf( f, "%999s", msg );					// should be DATA
	if ( ! ( ! strcmp( msg, "DATA" ) && root->load_insts( conf_file, f ) ) )
	{
		load = 3;
		goto endLoad;
	}

	if ( reload && quick == 2 )					// just quick reload?
		goto endLoad;

	last_run = 1;
	fscanf( f, "%999s", msg );					// should be SIM_NUM
	if ( ! ( ! strcmp( msg, "SIM_NUM" ) && fscanf( f, "%d", & last_run ) && last_run > 0 ) )
	{
		load = 4;
		goto endLoad;
	}

	seed = 1;
	fscanf( f, "%999s", msg );					// should be SEED
	if ( ! ( ! strcmp( msg, "SEED" ) && fscanf( f, "%d", & seed ) && seed > 0 ) )
	{
		load = 5;
		goto endLoad;
	}

	last_t = MAX_STEPS;
	deb_t = stack_info = prof_min_msecs = 0;
	prof_obs_only = prof_aggr_time = no_ptr_chk = parallel_disable = 0;
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

	i = sscanf( msg, "%d %d %d %d %d %d %d %d", & last_t, & deb_t, & stack_info, & prof_min_msecs, & prof_obs_only, & prof_aggr_time, & no_ptr_chk, & parallel_disable );

	if ( i < 1 || last_t <= 0 || deb_t < 0 || stack_info < 0 || prof_min_msecs < 0 || prof_obs_only < 0 || prof_obs_only > 1 || prof_aggr_time < 0 || prof_aggr_time > 1 || no_ptr_chk < 0 || no_ptr_chk > 1 || parallel_disable < 0 || parallel_disable > 1 )
	{
		load = 6;
		goto endLoad;
	}

	conf_ok = true;								// basic configuration loaded

	fscanf( f, "%999s", msg );					// should be EQUATION
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
	snprintf( full_name, 2 * MAX_PATH_LENGTH, "%s/%s", model_path, name + 1 );
	g = fopen( full_name, "r" );
	if ( g != NULL )
	{
		fclose( g );
		strcpyn( conf_eq_file, name + 1, MAX_PATH_LENGTH );
	}

	snprintf( rep_file, MAX_PATH_LENGTH, "report_%s.html", conf_name );
	fscanf( f, "%999s", msg );					// should be MODELREPORT
	if ( ! ( ! strcmp( msg, "MODELREPORT" ) && fscanf( f, "%999s", rep_file ) ) )
	{
		load = 8;
		goto endLoad;
	}

	empty_description( );						// remove existing descriptions
	strcpy( conf_eq_txt, "" );					// and equation file

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
			cv = root->search_var( NULL, msg );
			if ( cv != NULL )
			{
				cd->observe = true;

				for ( cur = cv->up; cur != NULL; cur = cur->hyper_next( cv->up->label ) )
				{
					cv1 = cur->search_var( NULL, cv->label );
					if ( cv1 != NULL )
						cv1->observe = true;
				}
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
		cv = root->search_var( NULL, msg );
		if ( cd != NULL && cv != NULL )
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

	for ( j = 0; fgets( msg, MAX_LINE_SIZE, f ) != NULL && strncmp( msg, "END_EQ_FILE", 11 ) && strlen( conf_eq_txt ) < MAX_FILE_SIZE - MAX_LINE_SIZE && j < MAX_FILE_TRY; ++j )
		strcatn( conf_eq_txt, msg, MAX_FILE_SIZE );

endLoad:

	// remove extra clear space at the beginning/end and standardize line ends
	strcln( buf1, conf_eq_txt, MAX_FILE_SIZE );
	strcpyn( conf_eq_txt, buf1, MAX_FILE_SIZE );

	if ( f != NULL )
		fclose( f );

	if ( warnings != NULL )
	{
		warnings->clear( );
		for ( auto i : warning )
			*warnings += " " + to_string( i );
	}

	t = 0;

	return load;
}


/*****************************************************************************
UNLOAD_CONFIGURATION
	Unload the current configuration
	If full is false, just the model data is unloaded
	Returns: pointer to root object
******************************************************************************/
void simulation::unload_configuration( bool full )
{
	empty_blueprint( );							// remove current model structure
	root->delete_obj( );
	root = new object;
	root->init( NULL, this, "Root" );
	add_description( "Root" );
	reset_blueprint( NULL );
	empty_cemetery( );							// garbage collection

	save_ok = true;								// valid structure to save
	sens = NULL;								// no sensitivity data

	eff_t = 0;									// reset steps counter
	nodesSerial = 0;							// reset network node serial number

	if ( full )									// full unload? (no new config?)
	{
		empty_description( );					// remove element descriptions

		delete [ ] conf_path;					// reset current path
		conf_path = new char[ strlen( model_path ) + 1 ];
		strcpy( conf_path, model_path );

		delete [ ] conf_name;					// reset simulation name to default
		conf_name = new char[ strlen( "" ) + 1 ];
		strcpy( conf_name, "" );

		delete [ ] conf_file;					// reset structure
		conf_file = new char[ strlen( "" ) + 1 ];

		strcpy( conf_file, "" );
		strcpy( rep_file, "" );
		strcpy( conf_eq_txt, "" );

		conf_ok = false;
	}
}


/****************************************************
OBJECT::LOAD_XML_STRUCT
	Load the object structure tree under this object
	from an xml object node
	If quick is true, just the structure and the
	parameters are retrieved, no descriptions
****************************************************/
int object::load_xml_struct( xml_node &n, bool quick )
{
	bool obs, integer;
	const char *str, *desc, *init;
	int i, type, lags;
	vector < double > val;
	vector < string > data;
	bridge *cb;
	variable *cv;

	if ( strcmp( n.attribute( "name" ).value( ), label ) != 0 )
		return 31;

	to_compute = n.attribute( "compute" ).as_bool( true );

	// scan contained child objects and elements
	for ( xml_node cn : n.children( ) )
	{
		if ( ! strcmp( cn.name( ), "object" ) )			// add object?
		{
			str = cn.attribute( "name" ).value( );
			if ( strlen( str ) == 0 || ! valid_label( str ) )
				return 32;

			add_obj( str );
			cb = search_bridge( str );

			i = cb->head->load_xml_struct( cn, quick );
			if ( i != 0 )
				return i;

			if ( ! quick && ! cn.child( "description" ).empty( ) )
			{
				desc = strdecdata( NULL, cn.child( "description" ).child( "text" ).text( ).get( ) );
				sim->add_description( str, 4, desc );
				delete [ ] desc;
			}
		}
		else
			if ( ! strcmp( cn.name( ), "element" ) )	// add element?
			{
				str = cn.attribute( "type" ).value( );
				if ( strlen( str ) == 0 )
					return 33;

				for ( type = 0; type < ELEM_TYPE_NUM; ++type )
					if ( ! strcmp( str, elem_type_names[ type ] ) )
						break;

				str = cn.attribute( "name" ).value( );
				if ( strlen( str ) == 0 || ! valid_label( str ) )
					return 34;

				cv = add_empty_var( str );
				cv->param = type;

				if ( ! quick )
				{
					if ( ! cn.child( "description" ).empty( ) )
					{
						desc = strdecdata( NULL, cn.child( "description" ).child( "text" ).text( ).get( ) );
						init = strdecdata( NULL, cn.child( "description" ).child( "initialization" ).text( ).get( ) );
						obs = cn.child( "documentation" ).attribute( "observe" ).as_bool( );

						sim->add_description( str, type, desc, init,
										 cn.child( "documentation" ).attribute( "initialization" ).as_bool( ),
										 obs );
						cv->observe = obs;

						delete [ ] desc;
						delete [ ] init;
					}

					if ( ! cn.child( "sensitivity" ).empty( ) )
					{
						xml_node cns = cn.child( "sensitivity" );
						integer = cn.attribute( "integer" ).as_bool( );
						lags = cn.attribute( "lags" ).as_uint( );

						if ( type == 1 )
						{
							val = strtodsplit( cns.child( "values" ).text( ).get( ), ',' );

							if ( val.size( ) > 1 )
								new sensitivity( str, sim, type, 0, integer, val.size( ), &val );
						}
						else
							if ( type == 0 )
							{
								for ( xml_node sn : cns.children( ) )
								{
									data = strtostrsplit( sn.name( ), '-' );

									if ( data.size( ) < 2 || data[ 0 ] != "values" )
										continue;

									if ( ( i = strtol( data[ 1 ].c_str( ), NULL, 10, -1 ) ) < 1 || i > lags )
										continue;

									val = strtodsplit( sn.text( ).get( ), ',' );

									if ( val.size( ) > 1 )
										new sensitivity( str, sim, type, i - 1, integer, val.size( ), &val );
								}
							}
					}
				}
			}
	}

	return 0;
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
			add_obj( ch );

			// find the bridge which contains the object
			cb = search_bridge( ch );

			if ( cb->head == NULL || ! cb->head->load_struct( f ) )
				return false;
		}

		if ( ! strcmp( ch, "Var:" ) )
		{
			fscanf( f, "%*[ ]%99s", ch );
			cv = add_empty_var( ch );
			cv->param = 0;
		}

		if ( ! strcmp( ch, "Param:" ) )
		{
			fscanf( f, "%*[ ]%99s", ch );
			cv = add_empty_var( ch );
			cv->param = 1;
		}

		if ( ! strcmp( ch, "Func:" ) )
		{
			fscanf( f, "%*[ ]%99s", ch );
			cv = add_empty_var( ch );
			cv->param = 2;
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
int object::load_xml_insts( xml_node &n, n_mapT &node_map, set < int > &warning )
{
	int i;
	double d;
	long k, l, m, nd;
	string data;
	vector < double > wht, val1, lnkwht1;
	vector < long > num, nser, nid, lnkto1;
	vector < string > val, nnam, lnkto, lnkwht;
	bridge *cb;
	object *cur;
	variable *cv, *cv1;

	if ( strcmp( n.attribute( "name" ).value( ), label ) != 0 )
		return 41;

	// split the number of instances string into an integer vector
	num = strtolsplit( n.child( "counts" ).text( ).get( ), ',' );

	// set # of instances for each object group
	for ( nd = l = 0, cur = this; cur != NULL;
		  nd += num[ l ], ++l, cur = cur->hyper_next( label ) )
	{
		if ( l >= ( long ) num.size( ) || num[ l ] <= 0 )
		{
			warning.insert( 42 );				// inconsistent # of groups
			m = 1;
		}
		else
			m = num[ l ];

		cur->to_compute = to_compute;
		cur->replicate( m );

		for ( ; go_brother( cur ) != NULL; cur = cur->next );// go next group
	}

	if ( l < ( long ) num.size( ) || nd != reduce( num.begin( ), num.end( ) ) )
		warning.insert( 43 );					// inconsistent # of groups

	// load network attributes and links
	if ( up != NULL && ! n.child( "nodes" ).empty( ) )
	{
		xml_node nn = n.child( "nodes" );
		nser = strtolsplit( nn.child( "serials" ).text( ).get( ), ',', -1 );
		nid = strtolsplit( nn.child( "ids" ).text( ).get( ), ',', -1 );
		if ( ( long ) nser.size( ) != nd || ( long ) nid.size( ) != nd )
			warning.insert( 44 );				// inconsistent # of node serials/ids

		if ( ! nn.child( "names" ).empty( ) )
		{
			nnam = strtostrsplit( nn.child( "names" ).text( ).get( ), ',', true );
			if ( ( long ) nnam.size( ) != nd )
				warning.insert( 45 );			// inconsistent # of node names
		}

		for ( l = 0, cur = this; cur != NULL; ++l, cur = cur->hyper_next( label ) )
		{
			if ( l >= ( long ) nser.size( ) || nser[ l ] < 1 )
			{
				warning.insert( 44 );			// inconsistent # of node serials
				break;
			}
			else
				m = nser[ l ];

			if ( l >= ( long ) nid.size( ) || nid[ l ] < 1 )
			{
				warning.insert( 44 );			// inconsistent # of node ids
				k = m;
			}
			else
				k = nid[ l ];

			data = "";
			if ( nnam.size( ) > 0 )
			{
				if ( l >= ( long ) nnam.size( ) )
					warning.insert( 45 );			// inconsistent # of node names
				else
					if ( nnam[ l ] != to_string( k ) )// ignore name = ID
						data = nnam[ l ];
			}

			cur->add_node_net( k, data.c_str( ), true );// add node
			node_map.insert( n_pairT( m, cur ) );
		}

		if ( ! nn.child( "linksto" ).empty( ) )
		{
			lnkto = strtostrsplit( nn.child( "linksto" ).text( ).get( ), ';' );

			if ( ! nn.child( "linksweigth" ).empty( ) )
				lnkwht = strtostrsplit( nn.child( "linksweigth" ).text( ).get( ), ';' );

			// add links to node objects
			for ( l = k = 0, cur = this; cur != NULL; ++l, cur = cur->hyper_next( label ) )
				if ( cur->node != NULL )		// node on instance?
				{
					if ( l >= ( long ) lnkto.size( ) )
					{
						warning.insert( 46 );	// inconsistent # of link groups
						break;
					}

					lnkto1 = strtolsplit( lnkto[ l ].c_str( ), ',', -1 );

					if ( lnkwht.size( ) > 0 )
					{
						if ( l >= ( long ) lnkwht.size( ) )
						{
							warning.insert( 47 );// inconsistent # of link groups
							lnkwht1.clear( );
						}
						else
							lnkwht1 = strtodsplit( lnkwht[ l ].c_str( ), ',' );
					}

					for ( m = 0; m < ( long ) lnkto1.size( ); ++m )
					{
						if ( lnkto1[ m ] < 1 || node_map.find( lnkto1[ m ] ) ==
												node_map.end( ) )
						{
							warning.insert( 48 );// invalid links
							break;
						}

						d = 0;
						if ( lnkwht.size( ) > 0 )
						{
							if ( m >= ( long ) lnkwht1.size( ) )
								warning.insert( 49 );// inconsistent # of weights
							else
								d = lnkwht1[ m ];
						}

						cur->add_link_net( node_map[ lnkto1[ m ] ], d );
					}

					if ( m < ( long ) lnkwht1.size( ) )
						warning.insert( 50 );	// inconsistent # of weights
				}

			if ( l < ( long ) lnkto.size( ) || ( lnkwht.size( ) > 0 &&
												 l < ( long ) lnkwht.size( ) ) )
				warning.insert( 51 );			// inconsistent # of link groups
		}
	}

	// load elements (parameters, variables and functions)
	for ( cv = v; cv != NULL; cv = cv->next )
	{
		xml_node cn = n.find_child_by_attribute( "element", "name", cv->label );
		if ( cn.empty( ) )
			warning.insert( 52 );				// missing element data

		cv->num_lag = ( cv->param == 1 ) ? 0 : cn.attribute( "lags" ).as_uint( );
		cv->save = cn.attribute( "save" ).as_bool( );
		cv->savei = cn.attribute( "save_file" ).as_bool( );
		cv->plot = cn.attribute( "plot" ).as_bool( );
		cv->integer = cn.attribute( "integer" ).as_bool( );
		cv->parallel = cn.attribute( "parallel" ).as_bool( );
		cv->max_val = cn.attribute( "maximum" ).as_double( NAN );
		cv->min_val = cn.attribute( "minimum" ).as_double( NAN );
		cv->deb_mode = cn.attribute( "debug" ).as_string( "n" )[ 0 ];
		cv->initialized = cn.attribute( "initialized" ).as_bool( true );

		if ( cv->param == 0 )
		{
			cv->delay = cn.attribute( "delay" ).as_uint( );
			cv->delay_range = cn.attribute( "delay_range" ).as_uint( );
			cv->period = cn.attribute( "period" ).as_uint( 1 );
			cv->period_range = cn.attribute( "period_range" ).as_uint( );
		}

		if ( cv->param == 1 || cv->num_lag > 0 )
		{	// split the values of instances string into a string vector
			val = strtostrsplit( cn.child( "values" ).text( ).get( ), ';' );
			if ( ( long ) val.size( ) != nd )
				warning.insert( 53 );			// inconsistent # of value groups
		}

		// set values of instances for each variable instance
		for ( l = 0, cur = this; cur != NULL; cur = cur->hyper_next( label ), ++l )
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
				if ( l >= ( long ) val.size( ) || strlen( val[ l ].c_str( ) ) == 0 )
				{
					warning.insert( 54 );		// inconsistent value groups
					val1.clear( );
				}
				else
					val1 = strtodsplit( val[ l ].c_str( ), ',' );

				for ( i = 0; i < ( cv1->param == 1 ? 1 : cv1->num_lag ); ++i )
				{
					if ( i >= ( long ) val1.size( ) || ! isfinite( val1[ i ] ) )
					{
						warning.insert( 55 );	// inconsistent values
						d = 0;
					}
					else
						d = val1[ i ];

					cv1->val[ i ] = d;
				}
			}

			if ( cv1->param != 1 )				// remove trash from last position
				cv1->val[ cv1->num_lag ] = 0;
		}

		if ( l < nd )
			warning.insert( 56 );				// inconsistent # of value groups
	}

	for ( cb = b; cb != NULL; cb = cb->next )
	{
		xml_node cn = n.find_child_by_attribute( "object", "name", cb->blabel );
		i = cb->head->load_xml_insts( cn, node_map, warning );
		if ( i != 0 )
			return i;
	}

	return 0;
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

		if ( cv->param == 1 )
			cv->num_lag = 0;

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
		set_blueprint( sim->blueprint );

	return true;
}


/****************************************************
SEARCH_DATA_STR (LEGACY)
****************************************************/
FILE *search_data_str( const char *name, const char *init, const char *str )
{
	FILE *f;
	char got[ MAX_LINE_SIZE ];

	f = fopen( name, "r" );
	if ( f == NULL )
		return NULL;

	fscanf( f, "%999s", got );
	for ( int i = 0; strcmp( got, init ) && i < MAX_FILE_TRY; ++i )
		if ( fscanf( f, "%999s", got ) == EOF )
			return NULL;

	if ( strcmp( got, init ) )
		return NULL;

	for ( int i = 0; strcmp( got, str ) && i < MAX_FILE_TRY; ++i )
		if ( fscanf( f, "%999s", got ) == EOF )
			return NULL;

	if ( ! strcmp( got, str ) )
		return f;
	else
		return NULL;
}


/****************************************************
LOAD_DESCRIPTION (LEGACY)
	Load the descriptions of elements of tree under
	this object from a LEGACY text file
****************************************************/
bool simulation::load_description( const char *d, FILE *f )
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
	for ( j = 0 ; fgets( str, MAX_LINE_SIZE, f ) != NULL && strncmp( str, desc_key_words[ 1 ], strlen( desc_key_words[ 1 ] ) ) && strncmp( str, desc_key_words[ 0 ], strlen( desc_key_words[ 0 ] ) ) && strlen( text ) <= 9 * MAX_LINE_SIZE && j < MAX_FILE_TRY ; ++j )
		strcatn( text, str, 10 * MAX_LINE_SIZE + 1 );

	if ( strncmp( str, desc_key_words[ 1 ], strlen( desc_key_words[ 1 ] ) ) && strncmp( str, desc_key_words[ 0 ], strlen( desc_key_words[ 0 ] ) ) )
		return false;

	if ( ! strncmp( str, desc_key_words[ 0 ], strlen( desc_key_words[ 0 ] ) ) )
	{
		for ( j = 0 ; fgets( str, MAX_LINE_SIZE, f ) != NULL && strncmp( str, desc_key_words[ 1 ], strlen( desc_key_words[ 1 ] ) ) && strlen( init ) <= 9 * MAX_LINE_SIZE && j < MAX_FILE_TRY ; ++j )
			strcatn( init, str, 10 * MAX_LINE_SIZE + 1 );

		if ( strncmp( str, desc_key_words[ 1 ], strlen( desc_key_words[ 1 ] ) ) )
			return false;
	}

	add_description( label, type, text, init );

	return true;
}


/*********************************
SAVE_SINGLE
	Save the value of a single
	element to file during run
*********************************/
void variable::save_single( void )
{
	char fn[ MAX_PATH_LENGTH ];
	int i;
	FILE *f;

#ifndef _NP_
	// prevent concurrent use by more than one thread
	rec_lguardT lock( var_comp_lck );
#endif

	set_lab_tit( );
	snprintf( fn, MAX_PATH_LENGTH, "%s_%s-%d_%d_seed-%d.res",
			  label, lab_tit, start, end, sim->seed - 1 );
	f = fopen( fn, "wt" );			// use text mode for Windows better compatibility

	fprintf( f, "%s %s (%d %d)\t\n", label, lab_tit, start, end );

	for ( i = 0; i <= sim->t - 1; ++i )
		if ( i >= start && i <= end && ! isnan( data[ i - start ] ) )	// save NaN as n/a
			fprintf( f,"%lf\t\n", data[ i - start ] );
		else
			fprintf( f,"%s\t\n", nonavail );

	fclose( f );
}


/*****************************************************************************
SENSITIVITY CONSTRUCTOR
Add or update sensitivity settings for a model element
******************************************************************************/
sensitivity::sensitivity( const char *lab, simulation *_sim, int _param, int _lag,
						  bool _integer, int _num_val, vector < double > *_val )
{
	int i;
	sensitivity *cs;

	sim = _sim;
	param = _param;
	lag = _lag;
	integer = _integer;

	if ( lab != NULL )
	{
		label = new char [ strlen( lab ) + 1 ];
		strcpy( label, lab );
	}

	if ( _num_val > 0 && _val != NULL )
	{
		num_val = _num_val;
		val = new double [ _val->size( ) ];
		for ( i = 0; i < num_val; ++i )
			val[ i ] = integer ? round( ( *_val )[ i ] ) : ( *_val )[ i ];
	}

	if ( sim->sens == NULL )
		sim->sens = this;
	else
	{
		for ( cs = sim->sens; cs->next != NULL; cs = cs->next );
		cs->next = this;
	}
}


/*****************************************************************************
SENSITIVITY DESTRUCTOR
Add or update sensitivity settings for a model element
******************************************************************************/
sensitivity::~sensitivity( void )
{
	sensitivity *cs, *ps;

	delete [ ] label;
	delete [ ] val;

	if ( sim->sens != NULL )
	{
		for ( cs = sim->sens, ps = NULL; cs != this && cs != NULL; ps = cs, cs = cs->next );

		if ( cs == sim->sens )
			sim->sens = next;
		else
			if ( cs == this && ps != NULL )
				ps->next = next;
	}
}


/*****************************************************************************
EMPTY_SENSITIVITY
Deallocate sensitivity analysis memory
******************************************************************************/
void simulation::empty_sensitivity( sensitivity *cs )
{
	if ( cs == NULL )
	{
		if ( sens == NULL )
			return;

		cs = sens;
		sens = NULL;
	}

	if ( cs->next != NULL )
		empty_sensitivity( cs->next );

	delete cs;				// suicide
}


/***************************************************
RESULT::CONSTRUCTOR
	Open the appropriate file for saving the results
***************************************************/
result::result( const char *fname, const char *fmode, simulation *_sim,
				bool _dozip, bool _docsv )
{
	sim = _sim;
	docsv = _docsv;
	dozip = _dozip;

	if ( dozip )
		fz = gzopen( fname, fmode );
	else
		f = fopen( fname, fmode );
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
			cv->set_lab_tit( );
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
		for ( cv = sim->cemetery; cv != NULL; cv = cv->next )
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
			if ( cv->start <= i && cv->end >= i && ! isnan( cv->data[ i - cv->start ] ) )
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
		for ( cv = sim->cemetery; cv != NULL; cv = cv->next )
		{
			if ( cv->start <= i && cv->end >= i && ! isnan( cv->data[ i - cv->start ] ) )
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
