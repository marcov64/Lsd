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
 FILELIB.CPP
 Contains the basic functions used to access files in DLL or
 terminal executables. The remaining file-oriented functions
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


/*************************************************************
 LOAD_CONFIGURATION
 Load current defined configuration from file (xml, gzip xml
 or legacy text)
 If quick is != 0, just the structure and the parameters are
 retrieved
 Returns: 0: load ok, 1,2,3,4,...: load failure
 *************************************************************/
int lsd::simulation::load_configuration( bool reload, strT *warnings, int quick )
{
	bool legacy = false;
	char *buf = NULL, buf1[ MAX_FILE_SIZE ], full_name[ 2 * MAX_PATH_LENGTH ];
	int i, j, sz, load = 0;
	n_mapT node_map;
	i_setT warning;
	FILE *f = NULL;
	gzFile fz;
	x_docT xf;

	unload_configuration( false );				// unload current

	if ( strlen( conf_name ) == 0 )
		return 1;

	if ( ! reload || conf_file == NULL || strlen( conf_file ) == 0 )
	{
		delete [ ] conf_file;
		sz = strlen( conf_path ) + strlen( conf_name ) + 6;
		conf_file = new char [ sz ];
		snprintf( conf_file, sz, "%s%s%s.lsd", conf_path, strlen( conf_path ) > 0 ? "/" : "", conf_name );
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
		x_nodeT lsdNode = xf.document_element( );// LSD top element
		x_nodeT cfgNode = lsdNode.child( "configuration" );// load config.

		if ( strcmp( lsdNode.name( ), "LSD" ) != 0 || strcmp( cfgNode.name( ), "configuration" ) != 0 )
			return 21;							// invalid xml type/format

		// get model structure
		x_nodeT rootNode = cfgNode.child( "structure" ).child( "object" );

		if ( rootNode.empty( ) )
			return 22;							// missing root

		if ( ! ( ( reload && quick == 2 ) || quick == 1 ) )
		{
			empty_sensitivity( );				// discard sensitivity analysis data
			empty_description( );				// remove existing descriptions
			empty_assimilation( );				// remove assimilation info
		}

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

		x_nodeT setNode = cfgNode.child( "settings" );
		x_nodeT simNode = setNode.child( "simulation" );

		if ( setNode.empty( ) || simNode.empty( ) )	// missing settings
		{
			load = 23;
			goto endLoad;
		}

		// get simulation settings
		x_attrT hint;							// speed-up pointer
		last_t = simNode.attribute( "steps", hint ).as_uint( MAX_STEPS );
		last_run = simNode.attribute( "runs", hint ).as_uint( 1 );
		seed = simNode.attribute( "seed", hint ).as_uint( 1 );
		deb_t = simNode.attribute( "debug_start", hint ).as_uint( );
		no_ptr_chk = ! simNode.attribute( "ptr_check", hint ).as_bool( true );
		parallel_disable = ! simNode.attribute( "parallel", hint ).as_bool( true );

		if ( da != NULL )
		{
			da->disable = setNode.child( "data_assimilation" ).attribute( "disable", hint ).as_bool( );
			da->algorithm = setNode.child( "data_assimilation" ).attribute( "algorithm", hint ).as_uint( );
			da->align_trim = setNode.child( "data_assimilation" ).attribute( "trim_instances", hint ).as_bool( );
			da->med_stats = setNode.child( "data_assimilation" ).attribute( "median_statistics", hint ).as_bool( );
			da->sav_obs = setNode.child( "data_assimilation" ).attribute( "save_observations", hint ).as_bool( );
			da->sav_fct = setNode.child( "data_assimilation" ).attribute( "save_forecasts", hint ).as_bool( );
			da->sav_dsp = setNode.child( "data_assimilation" ).attribute( "save_disp_matrix", hint ).as_bool( );
			da->sav_ci = setNode.child( "data_assimilation" ).attribute( "save_intervals", hint ).as_bool( );
			da->conf_lev = setNode.child( "data_assimilation" ).attribute( "confidence", hint ).as_double( 95 );

			da->use_dsp_file = setNode.child( "data_assimilation" ).attribute( "use_dispersion_file", hint ).as_bool( );
			if ( ( i = strlen( setNode.child( "data_assimilation" ).attribute( "dispersion_file", hint ).as_string( ) ) ) > 0 )
				da->dsp_file = setNode.child( "data_assimilation" ).attribute( "dispersion_file", hint ).as_string( );

			da->ens_infl = ! setNode.child( "data_assimilation" ).child( "ensemble_inflation" ).empty( );
			da->infl_fac = setNode.child( "data_assimilation" ).child( "ensemble_inflation" ).attribute( "inflation_factor" ).as_double( 1 );
			da->infl_time = setNode.child( "data_assimilation" ).child( "ensemble_inflation" ).attribute( "inflation_time" ).as_uint( 2 );

		}

		stack_info = setNode.child( "profiling" ).attribute( "level", hint ).as_uint( );
		prof_min_msecs = setNode.child( "profiling" ).attribute( "time", hint ).as_uint( );
		prof_obs_only = setNode.child( "profiling" ).attribute( "observed", hint ).as_bool( );
		prof_aggr_time = setNode.child( "profiling" ).attribute( "aggregate", hint ).as_bool( );

		// get report file name
		snprintf( rep_file, MAX_PATH_LENGTH, "report_%s.html", conf_name );
		strcpyn( rep_file, setNode.child( "report_file" ).text( ).as_string( rep_file ), MAX_PATH_LENGTH );

		// get equation file name and content
		x_nodeT eqfNode = cfgNode.child( "equation_file" );
		if ( eqfNode.empty( ) )
		{
			load = 24;
			goto endLoad;
		}

		// use the current equation name only if the file exists
		snprintf( full_name, 2 * MAX_PATH_LENGTH, "%s/%s", model_path, eqfNode.child( "filename" ).text( ).as_string( "NONE" ) );
		if ( ( f = fopen( full_name, "r" ) ) != NULL )
			strcpyn( conf_eq_file, eqfNode.child( "filename" ).text( ).get( ), MAX_PATH_LENGTH );

		if ( quick != 1 )						// load equation file?
			// decode xml ]]> escape sequences
			strdecdata( conf_eq_txt, eqfNode.child( "content" ).text( ).get( ), MAX_FILE_SIZE );
		else
			strcpy( conf_eq_txt, "" );
	}
	else
	{
		// try to read legacy configuration
		legacy = true;
		if ( ( load = load_txt_configuration( reload, quick ) ) == 1 )
			return load;
	}

endLoad:

	// remove extra clear space at the beginning/end and standardize line ends
	strcln( buf1, conf_eq_txt, MAX_FILE_SIZE );
	strcpyn( conf_eq_txt, buf1, MAX_FILE_SIZE );

	if ( f != NULL )
		fclose( f );

	if ( ! legacy && warnings != NULL )
	{
		warnings->clear( );

		if ( res.status != pugi::status_ok )
			*warnings = res.description( );

		for ( auto & i : warning )
			*warnings += " " + std::to_string( i );
	}

	return load;
}


/*************************************************************
 UNLOAD_CONFIGURATION
 Unload the current configuration
 If full is false, just the model data is unloaded
 *************************************************************/
void lsd::simulation::unload_configuration( bool full )
{
	empty_cemetery( );							// garbage collection
	empty_blueprint( );							// remove current model structure
	root->delete_obj( );
	empty_varattributes( this );
	empty_objattributes( this );

	root = new object ( NULL, ROOT_NAME, true, this );
	reset_blueprint( NULL );

	if ( desc != NULL )
		desc->add_descr( ROOT_NAME );			// ensure root has description

	save_ok = true;								// valid structure to save
	sens = NULL;								// no sensitivity data

	eff_t = 0;									// reset steps counter
	node_serial = 0;							// reset network node serial number

	if ( full )									// full unload? (no new config?)
	{
		empty_description( );					// remove element descriptions
		empty_assimilation( );					// remove assimilation info

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


/*************************************************************
 OBJECT::LOAD_XML_STRUCT
 Load the object structure tree under this object
 from an xml object node
 If quick is true, just the structure and the
 parameters are retrieved, no descriptions
 *************************************************************/
int lsd::object::load_xml_struct( x_nodeT &n, bool quick )
{
	bool integer, obs, plot;
	char debug;
	const char *str, *dsc, *init;
	int i, type, lags;
	d_vecT val;
	str_vecT data;
	bridge *cb;
	variable *cv;

	if ( strcmp( n.attribute( "name" ).value( ), attr->label ) != 0 )
		return 31;

	to_compute = n.attribute( "compute" ).as_bool( true );

	// scan contained child objects and elements
	for ( x_nodeT & cn : n.children( ) )
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

			if ( ! quick && attr->cont->sim == sims[ 0 ] && desc != NULL && ! cn.child( "description" ).empty( ) )
			{
				dsc = strdecdata( NULL, cn.child( "description" ).child( "text" ).text( ).get( ) );
				desc->add_descr( str, 4, dsc );
				delete [ ] dsc;
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

				lags = cn.attribute( "lags" ).as_uint( );
				plot = cn.attribute( "plot" ).as_bool( );
				debug = cn.attribute( "debug" ).as_string( "n" )[ 0 ];

				if ( ( cv = add_var( str, type, lags, plot, debug ) ) != NULL )
				{
					cv->attr->save = cn.attribute( "save" ).as_bool( );
					cv->attr->savei = cn.attribute( "save_file" ).as_bool( );
					cv->attr->integer = cn.attribute( "integer" ).as_bool( );
					cv->attr->parallel = cn.attribute( "parallel" ).as_bool( );
					cv->attr->max_val = cn.attribute( "maximum" ).as_double( NAN );
					cv->attr->min_val = cn.attribute( "minimum" ).as_double( NAN );
					cv->attr->initialized = cn.attribute( "initialized" ).as_bool( true );

					if ( type == 0 )
					{
						cv->attr->delay = cn.attribute( "delay" ).as_uint( );
						cv->attr->delay_range = cn.attribute( "delay_range" ).as_uint( );
						cv->attr->period = cn.attribute( "period" ).as_uint( 1 );
						cv->attr->period_range = cn.attribute( "period_range" ).as_uint( );
					}

					if ( ! quick )
					{
						if ( attr->cont->sim == sims[ 0 ] && desc != NULL && ! cn.child( "description" ).empty( ) )
						{
							dsc = strdecdata( NULL, cn.child( "description" ).child( "text" ).text( ).get( ) );
							init = strdecdata( NULL, cn.child( "description" ).child( "initialization" ).text( ).get( ) );
							obs = cn.child( "documentation" ).attribute( "observe" ).as_bool( );

							desc->add_descr( str, type, dsc, init, cn.child( "documentation" ).attribute( "initialization" ).as_bool( ), obs );
							cv->attr->observe = obs;

							delete [ ] dsc;
							delete [ ] init;
						}

						if ( ! cn.child( "sensitivity" ).empty( ) )
						{
							x_nodeT cns = cn.child( "sensitivity" );
							integer = cn.attribute( "integer" ).as_bool( );

							if ( type == 1 )
							{
								val = strtodsplit( cns.child( "values" ).text( ).get( ), ',' );

								if ( val.size( ) > 1 )
									new sensitivity( str, attr->cont->sim, type, 0, integer, val.size( ), &val );
							}
							else
								if ( type == 0 )
								{
									for ( x_nodeT & sn : cns.children( ) )
									{
										data = strtostrsplit( sn.name( ), '-' );

										if ( data.size( ) < 2 || data[ 0 ] != "values" )
											continue;

										if ( ( i = strtol( data[ 1 ].c_str( ), NULL, 10, -1 ) ) < 1 || i > lags )
											continue;

										val = strtodsplit( sn.text( ).get( ), ',' );

										if ( val.size( ) > 1 )
											new sensitivity( str, attr->cont->sim, type, i - 1, integer, val.size( ), &val );
									}
								}
						}
					}
				}

				if ( attr->cont->sim == sims[ 0 ] && da != NULL && ! cn.child( "assimilation" ).empty( ) )
				{
					x_nodeT cna = cn.child( "assimilation" );

					bool disable = cna.attribute( "disable" ).as_bool( );
					bool update = cna.attribute( "update" ).as_bool( true );
					bool data_obs = cna.attribute( "data_observations" ).as_bool( );

					strT data_file = cna.attribute( "data_file" ).value( );
					strT data_col_name = cna.attribute( "data_column_name" ).value( );
					strT t_col_name = cna.attribute( "time_column_name" ).value( );
					int data_col_num = cna.attribute( "data_column_number" ).as_uint( );
					int t_col_num = cna.attribute( "time_column_number" ).as_uint( );

					bool param = ! cna.child( "parameter" ).empty( );
					int par_dist = cna.child( "parameter" ).attribute( "distribution" ).as_uint( );
					double par_n_sd = cna.child( "parameter" ).attribute( "normal_sd" ).as_double( );
					double par_u_upp = cna.child( "parameter" ).attribute( "uniform_upper" ).as_double( );
					double par_u_low = cna.child( "parameter" ).attribute( "uniform_lower" ).as_double( );

					da->ass_elem.emplace_back( str, param, disable, update, data_obs, data_file, data_col_name, t_col_name, data_col_num, t_col_num, par_dist, par_n_sd, par_u_upp, par_u_low );
				}
			}
	}

	return 0;
}


/*************************************************************
 OBJECT::LOAD_XML_INSTS
 Load the object instances of tree under this
 object from an xml object node
 *************************************************************/
int lsd::object::load_xml_insts( x_nodeT &n, n_mapT &node_map, i_setT &warning )
{
	int i;
	double d;
	long k, l, m, nd;
	d_vecT wht, val1, lnkwht1;
	str_vecT val, nnam, lnkto, lnkwht;
	strT data;
	l_vecT num, nser, nid, lnkto1;
	bridge *cb;
	object *cur;
	variable *cv, *cv1;

	if ( strcmp( n.attribute( "name" ).value( ), attr->label ) != 0 )
		return 41;

	// split the number of instances string into an integer vector
	num = strtolsplit( n.child( "counts" ).text( ).get( ), ',' );

	// set # of instances for each object group
	for ( nd = l = 0, cur = this; cur != NULL; nd += num[ l ], ++l, cur = cur->hyper_next( ) )
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

		for ( ; BROTHER( cur ) != NULL; cur = cur->next );// go next group
	}

	if ( l < ( long ) num.size( ) || nd != reduce( num.begin( ), num.end( ) ) )
		warning.insert( 43 );					// inconsistent # of groups

	// load network attributes and links
	if ( up != NULL && ! n.child( "nodes" ).empty( ) )
	{
		x_nodeT nn = n.child( "nodes" );
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

		for ( l = 0, cur = this; cur != NULL; ++l, cur = cur->hyper_next( ) )
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
					if ( nnam[ l ] != std::to_string( k ) )// ignore name = ID
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
			for ( l = k = 0, cur = this; cur != NULL; ++l, cur = cur->hyper_next( ) )
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
		x_nodeT cn = n.find_child_by_attribute( "element", "name", cv->attr->label );
		if ( cn.empty( ) )
			warning.insert( 52 );				// missing element data

		if ( cv->param == 1 || cv->attr->num_lag > 0 )
		{	// split the values of instances string into a string vector
			val = strtostrsplit( cn.child( "values" ).text( ).get( ), ';' );
			if ( ( long ) val.size( ) != nd )
				warning.insert( 53 );			// inconsistent # of value groups
		}

		// set values of instances for each variable instance
		for ( l = 0, cur = this; cur != NULL; cur = cur->hyper_next( ), ++l )
		{
			cv1 = cur->search_var( NULL, cv->attr );

			// set parameters and initial conditions
			if ( cv1->param == 1 || cv1->attr->num_lag > 0 )
			{
				if ( l >= ( long ) val.size( ) || strlen( val[ l ].c_str( ) ) == 0 )
				{
					warning.insert( 54 );		// inconsistent value groups
					val1.clear( );
				}
				else
					val1 = strtodsplit( val[ l ].c_str( ), ',' );

				for ( i = 0; i < ( cv1->param == 1 ? 1 : cv1->attr->num_lag ); ++i )
				{
					if ( i >= ( long ) val1.size( ) || ! std::isfinite( val1[ i ] ) )
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
				cv1->val[ cv1->attr->num_lag ] = 0;
		}

		if ( l < nd )
			warning.insert( 56 );				// inconsistent # of value groups
	}

	for ( cb = b; cb != NULL; cb = cb->next )
	{
		x_nodeT cn = n.find_child_by_attribute( "object", "name", cb->attr->label );
		i = cb->head->load_xml_insts( cn, node_map, warning );
		if ( i != 0 )
			return i;
	}

	return 0;
}


/*************************************************************
 SAVE_XML_CONFIGURATION
 Save current defined configuration (adding tag index if
 appropriate) to gzip-compressed xml file
 If quick is true, just the structure and the parameters are
 saved
 Returns: true: save ok, false: save failure
 *************************************************************/
bool lsd::simulation::save_xml_configuration( const char *dest_path, const char *rname, const char *ext, int findex, bool zip, bool quick, const char mod_nam[ ], const char mod_ver[ ], const char mod_dat[ ], const char eq_file[ ], const char eq_txt[ ] )
{
	bool saved;
	int delta, indexDig, save_len, save_len_bkp;
	char ch[ MAX_PATH_LENGTH ], *save_file, *bak_file;
	const char *save_name, *save_ext, *save_path;
	long node_serial = 1;
	FILE *f;
	gzFile fz;
	std::ostringstream buf;
	x_docT xf;

	delta = ( findex > 0 ) ? last_run * ( findex - 1 ) : 0;
	indexDig = ( findex > 0 ) ? ( int ) floor( log10( findex ) + 2 ) : 0;

	if ( dest_path == NULL )
		save_path = conf_path;
	else
		save_path = dest_path;

	if ( strlen( conf_name ) == 0 )
	{
		delete [ ] conf_name;
		conf_name = new char[ strlen( DEF_CONF_FILE ) + 1 ];
		strcpy( conf_name, DEF_CONF_FILE );
	}

	if ( strlen( rep_file ) == 0 )
		snprintf( rep_file, MAX_PATH_LENGTH, "report_%s.html", conf_name );

	if ( rname == NULL || strlen( rname ) == 0 )
		save_name = conf_name;
	else
		save_name = rname;

	if ( ext == NULL || strlen( ext ) == 0 )
		save_ext = ".lsd";
	else
		save_ext = ext;

	save_len = strlen( save_path ) + strlen( save_name ) + strlen( save_ext ) + indexDig + 2;
	save_file = new char [ save_len ];
	snprintf( save_file, save_len, "%s%s%s", save_path, strlen( save_path ) > 0 ? "/" : "", save_name );

	if ( findex > 0 )
	{
		snprintf( ch, MAX_PATH_LENGTH, "_%d%s", findex, save_ext );
		strcatn( save_file, ch, save_len );
	}
	else
	{
		// create backup file when not indexed saving
		save_len_bkp = strlen( save_file ) + 5;
		bak_file = new char [ save_len_bkp ];
		snprintf( bak_file, save_len_bkp, "%s.bak", save_file );

		strcatn( save_file, save_ext, save_len );
		f = fopen( save_file, "r" );
		if ( f != NULL )
		{
			fclose( f );

			f = fopen( bak_file, "r" );
			if ( f != NULL )
			{
				fclose( f );

				if( remove( bak_file ) == 0 )
					rename( save_file, bak_file );
			}
			else
				rename( save_file, bak_file );
		}

		delete [ ] bak_file;
	}

	// add XML declaration, type and root node
	x_nodeT declNode = xf.append_child( pugi::node_declaration );
	declNode.append_attribute( "version" ) = "1.0";
	declNode.append_attribute( "encoding" ) = "ISO-8859-1";
	declNode.append_attribute( "standalone" ) = "yes";
	xf.append_child( pugi::node_doctype ).set_value( "LSD [\n \
	<!ELEMENT LSD (configuration)>\n \
	<!ELEMENT configuration (settings, structure, equation_file)>\n \
	<!ELEMENT settings (simulation, data_assimilation?, profiling?)>\n \
	<!ELEMENT simulation EMPTY>\n \
	<!ELEMENT data_assimilation (ensemble_inflation?)>\n \
	<!ELEMENT ensemble_inflation EMPTY>\n \
	<!ELEMENT profiling EMPTY>\n \
	<!ELEMENT structure (object)>\n \
	<!ELEMENT object (description?, nodes?, object*, element*)>\n \
	<!ELEMENT description (#PCDATA)>\n \
	<!ELEMENT nodes (#PCDATA)>\n \
	<!ELEMENT element (description?, documentation?, sensitivity?, assimilation?)>\n \
	<!ELEMENT documentation EMPTY>\n \
	<!ELEMENT sensitivity (#PCDATA)>\n \
	<!ELEMENT assimilation (parameter?)>\n \
	<!ELEMENT parameter EMPTY>\n \
	<!ELEMENT equation_file (#PCDATA)>\n]" );
	x_nodeT lsdNode = xf.append_child( "LSD" );
	x_nodeT cfgNode = lsdNode.append_child( "configuration" );
	cfgNode.append_attribute( "version" ) = "1.0";

	snprintf( ch, MAX_PATH_LENGTH, "LSD configuration file for model '%s', version %s, created in %s", mod_nam, mod_ver, mod_dat );
	cfgNode.append_attribute( "description" ) = ch;

	// add simulation settings
	x_nodeT setNode = cfgNode.append_child( "settings" );
	x_nodeT simNode = setNode.append_child( "simulation" );
	simNode.append_attribute( "steps" ) = last_t;
	simNode.append_attribute( "runs" ) = last_run;
	simNode.append_attribute( "seed" ) = seed + delta;

	// optional settings (include only if non-default)
	if ( deb_t > 0 )
		simNode.append_attribute( "debug_start" ) = deb_t;

	if ( no_ptr_chk )
		simNode.append_attribute( "ptr_check" ) = false;

	if ( parallel_disable )
		simNode.append_attribute( "parallel" ) = false;

	// add data assimilation global settings, if any
	if ( da != NULL && da->count( 0 ) > 0 )
	{
		x_nodeT assimNode = setNode.append_child( "data_assimilation" );

		if ( da->disable )
			assimNode.append_attribute( "disable" ) = true;

		if ( da->algorithm != 0 )
			assimNode.append_attribute( "algorithm" ) = da->algorithm;

		if ( da->align_trim )
			assimNode.append_attribute( "trim_instances" ) = true;

		if ( da->med_stats )
			assimNode.append_attribute( "median_statistics" ) = true;

		if ( da->sav_obs )
			assimNode.append_attribute( "save_observations" ) = true;

		if ( da->sav_fct )
			assimNode.append_attribute( "save_forecasts" ) = true;

		if ( da->sav_dsp )
			assimNode.append_attribute( "save_disp_matrix" ) = true;

		if ( da->sav_ci )
			assimNode.append_attribute( "save_intervals" ) = true;

		if ( da->conf_lev != 95 )
			assimNode.append_attribute( "confidence" ) = da->conf_lev;

		if ( da->use_dsp_file )
			assimNode.append_attribute( "use_dispersion_file" ) = true;

		if ( da->dsp_file.size( ) > 0 )
			assimNode.append_attribute( "dispersion_file" ) = da->dsp_file.c_str( );

		if ( da->ens_infl && da->infl_fac != 1 )
		{
			x_nodeT inflNode = assimNode.append_child( "ensemble_inflation" );

			inflNode.append_attribute( "inflation_factor" ) = da->infl_fac;

			if ( da->infl_time > 2 )
				inflNode.append_attribute( "inflation_time" ) = da->infl_time;
		}
	}

	// add profile settings, if any
	if ( stack_info > 0 || prof_min_msecs > 0 || prof_obs_only || prof_aggr_time )
	{
		x_nodeT profNode = setNode.append_child( "profiling" );

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
	setNode.append_child( "report_file" ).text( ) = rep_file;

	// add model structure
	x_nodeT strNode = cfgNode.append_child( "structure" );
	root->save_xml_struct( strNode, node_serial, quick );

	// add equation file name and content
	x_nodeT eqfNode = cfgNode.append_child( "equation_file" );
	eqfNode.append_child( "filename" ).text( ) = eq_file;

	if ( ! quick )
	{
		if ( eq_txt != NULL && ( strlen( conf_eq_txt ) == 0 || strcmp( conf_eq_txt, eq_txt ) != 0 ) )
			strcpyn( conf_eq_txt, eq_txt, MAX_FILE_SIZE );

		// encode xml ]]> escape sequences
		eqfNode.append_child( "content" ).append_child( pugi::node_cdata ).set_value( strencdata( conf_eq_txt, conf_eq_txt, MAX_FILE_SIZE ) );
	}

	xf.save( buf );

	saved = false;
	if ( zip )
	{
		if ( ( fz = gzopen( save_file, "wb9" ) ) != Z_NULL )
		{
			saved = gzputs( fz, buf.str( ).c_str( ) );
			saved = gzclose( fz ) == Z_OK ? saved : false;
		}
	}
	else
		if ( ( f = fopen( save_file, "wt" ) ) != NULL )
		{
			saved = fputs( buf.str( ).c_str( ), f ) != EOF ? true : false;
			saved = fclose( f ) != EOF ? saved : false;
		}

	delete [ ] save_file;

	return saved;
}


/*************************************************************
 OBJECT::SAVE_XML_STRUCT
 Save the object structure tree under this object
 to an xml object
 If quick is true, just the structure and the
 parameters are saved, no descriptions
 *************************************************************/
void lsd::object::save_xml_struct( x_nodeT &pn, long &node_serial, bool quick )
{
	bool init, nodes, noWht;
	char *str;
	int i, count;
	long l, k;
	strT data, text, nser, nid, nnam, lnkto, lnkwht;
	bridge *cb;
	netlink *curl;
	object *cur;
	variable *cv, *cv1;

	x_nodeT n = pn.append_child( "object" );
	n.append_attribute( "name" ) = attr->label;

	if ( ! to_compute )
		n.append_attribute( "compute" ) = false;

	for ( data = "", nodes = false, cur = this; cur != NULL; cur = cur->hyper_next( ) )
	{
		if ( cur != this )
			data += ",";

		next_count( cur, &count );
		data += std::to_string( count );

		for ( ; BROTHER( cur ) != NULL; cur = cur->next )
			if ( cur->node != NULL )	// check if object contains network nodes
				nodes = true;
	}

	n.append_child( "counts" ).text( ) = data.c_str( );

	if ( ! quick && desc != NULL )
	{
		auto cd = desc->search_descr( attr->label, true );
		if ( ! strwsp( cd->text ) )
		{
			x_nodeT nd = n.append_child( "description" );
			str = strencdata( NULL, cd->text );
			nd.append_child( "text" ).append_child( pugi::node_cdata ).set_value( str );
			delete [ ] str;
		}
	}

	// save network attributes and links
	l = k = 0;
	noWht = true;
	if ( nodes )
	{	// first save nodes and attribute serials
		for ( cur = this; cur != NULL; ++l, cur = cur->hyper_next( ) )
		{
			if ( cur != this )
			{
				nser += ",";
				nid += ",";
				nnam += ",";
			}

			if ( cur->node != NULL )
			{
				cur->node->serial = node_serial++;
				nser += std::to_string( cur->node->serial );
				nid += std::to_string( cur->node->id );

				if ( cur->node->name != NULL )
					nnam += "\"" + ( data = cur->node->name ) + "\"";
				else
					nnam += "\"\"";
			}
		}

		// second save links using serials for destination
		for ( cur = this; cur != NULL; cur = cur->hyper_next( ) )
		{
			if ( cur != this )
			{
				lnkto += ";";
				lnkwht += ";";
			}

			if ( cur->node != NULL )			// scan all links from node
				for ( curl = cur->node->first; curl != NULL; ++k, curl = curl->next )
				{
					if ( curl != cur->node->first )
					{
						lnkto += ",";
						lnkwht += ",";
					}

					if ( curl->to == NULL || curl->to->node == NULL )
						continue;				// ignore invalid link

					lnkto += std::to_string( curl->to->node->serial );
					lnkwht += to_string( "%.15g", curl->weight );

					if ( curl->weight != 0 )
						noWht = false;
				}
		}

		x_nodeT nd = n.append_child( "nodes" );
		nd.append_child( "serials" ).text( ) = nser.c_str( );
		nd.append_child( "ids" ).text( ) = nid.c_str( );

		if ( ( long ) nnam.size( ) > l * 3 - 1 )		// don't add if no name
			nd.append_child( "names" ).text( ) = nnam.c_str( );

		if ( ( long ) lnkto.size( ) > k - 1 )			// don't add if no link
		{
			nd.append_child( "linksto" ).text( ) = lnkto.c_str( );

			if ( ! noWht )
				nd.append_child( "linksweigth" ).text( ) = lnkwht.c_str( );
		}
	}

	// save son objects recursively
	for ( cb = b; cb != NULL; cb = cb->next )
		if ( cb->head == NULL )
			attr->cont->sim->blueprint->search( cb->attr )->save_xml_struct( n, node_serial, quick );
		else
			cb->head->save_xml_struct( n, node_serial, quick );

	// save elements (parameters, variables and functions)
	for ( cv = v; cv != NULL; cv = cv->next )
	{
		x_nodeT cn = n.append_child( "element" );
		cn.append_attribute( "name" ) = cv->attr->label;
		cn.append_attribute( "type" ) = elem_type_names[ cv->param ];

		if ( cv->param != 1 )
			cn.append_attribute( "lags" ) = cv->attr->num_lag;

		// search for uninitialized data
		if ( cv->param == 1 || cv->attr->num_lag > 0 )
		{
			for ( init = true, cur = this; cur != NULL; cur = cur->hyper_next( ) )
			{
				cv1 = cur->search_var( NULL, cv->attr );
				if ( ! cv1->attr->initialized )
				{
					init = false;
					break;
				}
			}

			if ( ! init )
				cn.append_attribute( "initialized" ) = false;
		}

		// save only non-default values
		if ( cv->attr->save )
			cn.append_attribute( "save" ) = true;

		if ( cv->attr->savei )
			cn.append_attribute( "save_file" ) = true;

		if ( cv->attr->integer )
			cn.append_attribute( "integer" ) = true;

		if ( cv->attr->parallel )
			cn.append_attribute( "parallel" ) = true;

		if ( ! std::isnan( cv->attr->max_val ) )
			cn.append_attribute( "maximum" ) = cv->attr->max_val;

		if ( ! std::isnan( cv->attr->min_val ) )
			cn.append_attribute( "minimum" ) = cv->attr->min_val;

		if ( cv->plot )
			cn.append_attribute( "plot" ) = true;

		if ( cv->deb_mode != 'n' )
		{
			data = cv->deb_mode;
			cn.append_attribute( "debug" ) = data.c_str( );
		}

		if ( cv->attr->delay > 0 )
			cn.append_attribute( "delay" ) = cv->attr->delay;

		if ( cv->attr->delay_range > 0 )
			cn.append_attribute( "delay_range" ) = cv->attr->delay_range;

		if ( cv->attr->period > 1 )
			cn.append_attribute( "period" ) = cv->attr->period;

		if ( cv->attr->period_range > 0 )
			cn.append_attribute( "period_range" ) = cv->attr->period_range;

		// add initial values
		if ( cv->param == 1 || cv->attr->num_lag > 0 )
		{
			for ( data = "", cur = this; cur != NULL; cur = cur->hyper_next( ) )
			{
				if ( cur != this )
					data += ";";

				cv1 = cur->search_var( NULL, cv->attr );
				for ( i = 0; i < ( cv1->param == 1 ? 1 : cv1->attr->num_lag ); ++i )
				{
					if ( i != 0 )
						data += ",";

					data += to_string( "%.15g", cv1->attr->initialized ? cv1->chk_val( cv1->val[ i ] ) : 0 );
				}
			}

			cn.append_child( "values" ).text( ) = data.c_str( );
		}

		if ( quick )
			continue;

		// add description text
		if ( desc != NULL )
		{
			auto cd = desc->search_descr( cv->attr->label, true );
			if ( ! strwsp( cd->text ) || ! strwsp( cd->init ) )
			{
				x_nodeT cnd = cn.append_child( "description" );

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
				x_nodeT cnd = cn.append_child( "documentation" );

				if ( cd->observe )
					cnd.append_attribute( "observe" ) = true;

				if ( cd->initial )
					cnd.append_attribute( "initialization" ) = true;
			}
		}

		// add sensitivity analysis data
		for ( auto cs = attr->cont->sim->sens; cs != NULL; cs = cs->next )
			if ( strcmp( cs->label, cv->attr->label ) == 0 )
			{
				if ( cs->integer )
					cn.append_attribute( "integer" ) = true;

				x_nodeT cns;

				if ( cn.child( "sensitivity" ).empty( ) )
					cns = cn.append_child( "sensitivity" );
				else
					cns = cn.child( "sensitivity" );

				for ( data = "", i = 0; cs->val != NULL && i < cs->num_val; ++i )
				{
					if ( i != 0 )
						data += ",";

					data += to_string( "%.15g", cs->val[ i ] );
				}

				if ( cv->param )
					text = "values";
				else
					text = "values-" + std::to_string( cs->lag + 1 );

				cns.append_child( text.c_str( ) ).text( ) = data.c_str( );
			}

		// add data assimilation settings
		if ( da != NULL )
		{
			auto ca = da->search( cv->attr->label );
			if ( ca != da->ass_elem.end( ) )
			{
				x_nodeT cna = cn.append_child( "assimilation" );

				if ( ca->disable )
					cna.append_attribute( "disable" ) = ca->disable;

				if ( ! ca->update )
					cna.append_attribute( "update" ) = ca->update;

				if ( ca->data_obs )
					cna.append_attribute( "data_observations" ) = ca->data_obs;

				if ( ca->data_file.size( ) > 0 )
					cna.append_attribute( "data_file" ) = ca->data_file;

				if ( ca->data_col_name.size( ) > 0 )
					cna.append_attribute( "data_column_name" ) = ca->data_col_name;
				else
					if ( ca->data_col_num > 0 )
						cna.append_attribute( "data_column_number" ) = ca->data_col_num;

				if ( ca->t_col_name.size( ) > 0 )
					cna.append_attribute( "time_column_name" ) = ca->t_col_name;
				else
					if ( ca->t_col_num > 0 )
						cna.append_attribute( "time_column_number" ) = ca->t_col_num;

				if ( ca->param )
				{
					x_nodeT cnap = cna.append_child( "parameter" );

					cnap.append_attribute( "distribution" ) = ca->par_dist;

					if ( ca->par_dist == 0 && ca->par_n_sd > 0 )
						cnap.append_attribute( "normal_sd" ) = ca->par_n_sd;

					if ( ca->par_dist == 1 && ( ca->par_u_upp > 0 || ca->par_u_low > 0 ) )
					{
						cnap.append_attribute( "uniform_upper" ) = ca->par_u_upp;
						cnap.append_attribute( "uniform_lower" ) = ca->par_u_low;
					}
				}
			}
		}
	}
}


/*************************************************************
 LOAD_TXT_CONFIGURATION (LEGACY)
 Load current defined configuration from file (legacy text
 only)
 If quick is != 0, just the structure and the parameters are
 retrieved
 Returns: 0: load ok, 1,2,3,4,...: load failure
 Must be used after/from load_configurations()
 *************************************************************/
int lsd::simulation::load_txt_configuration( bool reload, int quick )
{
	char msg[ MAX_LINE_SIZE ], name[ MAX_PATH_LENGTH ], full_name[ 2 * MAX_PATH_LENGTH ];
	int i, j, load = 0;
	object *cur;
	variable *cv, *cv1;
	FILE *g, *f;

	f = fopen( conf_file, "rb" );
	if ( f == NULL )
		return 1;

	if ( ! root->load_txt_struct( f ) )
	{
		load = 2;
		goto endLoad;
	}

	strcpy( msg, "" );
	fscanf( f, "%999s", msg );					// should be DATA
	if ( ! ( ! strcmp( msg, "DATA" ) && root->load_txt_insts( conf_file, f ) ) )
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
		if ( seed <= 0 )
			seed = 1;
		else
		{
			load = 5;
			goto endLoad;
		}
	}

	empty_assimilation( );
	if ( da != NULL )
	{
		da->dsp_file.clear( );
		da->disable = true;
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
		if ( desc != NULL )
			i = load_txt_descr( msg, f );

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
		auto cd = desc != NULL ? desc->search_descr( msg, true ) : NULL;
		if ( cd != NULL )
		{
			cv = root->search_var( NULL, msg );
			if ( cv != NULL )
			{
				cd->observe = true;

				for ( cur = cv->up; cur != NULL; cur = cur->hyper_next( ) )
				{
					cv1 = cur->search_var( NULL, cv->attr );
					if ( cv1 != NULL )
						cv1->attr->observe = true;
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
		auto cd = desc != NULL ? desc->search_descr( msg, true ) : NULL;
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

	fclose( f );

	return load;
}


/*************************************************************
 OBJECT::LOAD_TXT_STRUCT (LEGACY)
 Load the object structure tree under this object
 from a LEGACY text file
 *************************************************************/
bool lsd::object::load_txt_struct( FILE *f )
{
	int type, i = 0;
	char ch[ MAX_ELEM_LENGTH ];
	bridge *cb;

	fscanf( f, "%99s", ch );
	while ( strcmp( ch, "Label" ) && ++i < MAX_FILE_TRY )
		fscanf( f,"%99s", ch );

	if ( i >= MAX_FILE_TRY )
		return false;

	fscanf( f, "%99s", ch );
	if ( strcmp( attr->label, ch ) != 0 )
		return false;

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

			if ( cb->head == NULL || ! cb->head->load_txt_struct( f ) )
				return false;
		}
		else
		{
			if ( ! strcmp( ch, "Param:" ) )
				type = 1;
			else
				if ( ! strcmp( ch, "Func:" ) )
					type = 2;
				else
					type = 0;

			fscanf( f, "%*[ ]%99s", ch );
			add_var( ch, type );
		}

		fscanf( f, "%*[{\r\t\n]%99s", ch );
	}

	if ( i >= MAX_FILE_TRY )
		return false;

	return true;
}


/*************************************************************
 OBJECT::LOAD_TXT_INSTS (LEGACY)
 Load the object instances of tree under this
 object from a LEGACY text file
 *************************************************************/
bool lsd::object::load_txt_insts( const char *file_name, FILE *f )
{
	char str[ MAX_ELEM_LENGTH ], ch1, ch2, ch3, ch4;
	int num, i;
	double app;
	fpos_t pos;
	bridge *cb;
	object *cur;
	variable *cv, *cv1;

	if ( f == NULL )
		f = search_txt_data( file_name, "DATA", attr->label );
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

	for ( cur = this; cur != NULL; cur = cur->hyper_next( ) )
	{
		if ( fscanf( f, "\t%d", &num ) != 1 )
			return false;

		cur->to_compute = to_compute;
		cur->replicate( num );

		for ( ; BROTHER( cur ) != NULL; cur = cur->next );
	}

	for ( cv = v; cv != NULL; cv = cv->next )
	{
		fscanf( f, "%99s ", str );		// skip the 'Element: '
		fscanf( f, "%99s ", str );		// skip the 'label'

		if ( f == NULL )
			return false;

		if ( fscanf( f, "%d %c %c %c %c", &( cv->attr->num_lag ), &ch1, &ch2, &ch3, &ch4 ) != 5 )
			return false;

		if ( cv->param == 1 )
			cv->attr->num_lag = 0;

		cv->attr->save = ( tolower( ch1 ) == 's' ) ? true : false;
		cv->attr->savei = ( ch1 == 'S' || ch1 == 'N' ) ? true : false;
		cv->attr->initialized = ( ch2 == '+' ) ? true : false;
		cv->deb_mode = ch3;
		cv->plot = ( tolower( ch4 ) == 'p' ) ? true : false;
		cv->attr->parallel = ( ch4 == 'P' || ch4 == 'N' ) ? true : false;

		for ( cur = this; cur != NULL; cur = cur->hyper_next( ) )
		{
			cv1 = cur->search_var( NULL, cv->attr );
			cv1->plot = cv->plot;
			cv1->deb_mode = cv->deb_mode;

			delete [ ] cv1->val;
			cv1->val = new double [ cv->attr->num_lag + 1 ];

			if ( cv1->param == 1 )
			{
				if ( fscanf( f, "%lf", &app ) != 1 )
					return false;
				else
					cv1->val[ 0 ] = app;
			}
			else
			{
				for ( i = 0; i < cv->attr->num_lag; ++i )
					if ( fscanf( f, "\t%lf", &app ) != 1 )
						return false;
					else
						// place values shifted one position, since they are "time 0" values
						cv1->val[ i ] = app;

				cv1->val[ cv->attr->num_lag ] = 0;
			}
		}

		// check for non-default updating scheme
		if ( cv->param == 0 )
		{
			fgetpos( f, & pos );
			num = fscanf( f, "\t<upd: %d %d %d %d>", & cv->attr->delay, & cv->attr->delay_range, & cv->attr->period, & cv->attr->period_range );

			if ( num > 0 && num < 4 )
				return false;

			if ( num == 0 )
				fsetpos( f, & pos );
		}
	}

	for ( cb = b; cb != NULL; cb = cb->next )
	{
		if ( cb->head == NULL || ! cb->head->load_txt_insts( file_name, f ) )
			return false;
		num = 0;
	}

	if ( up == NULL )	// this is the root, and therefore the end of the loading
		set_blueprint( attr->cont->sim->blueprint );

	return true;
}


/*************************************************************
 LOAD_TXT_DESCR (LEGACY)
 Load the descriptions of elements of tree under
 this object from a LEGACY text file
 *************************************************************/
bool lsd::simulation::load_txt_descr( const char *d, FILE *f )
{
	int j, type, ctype;
	char label[ MAX_ELEM_LENGTH ], text[ 10 * MAX_LINE_SIZE + 1 ], init[ 10 * MAX_LINE_SIZE + 1 ], str[ 10 * MAX_LINE_SIZE + 1 ];
	variable *cv;

	if ( desc == NULL )
		return true;

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

	desc->add_descr( label, type, text, init );

	return true;
}


/*************************************************************
 SEARCH_TXT_DATA (LEGACY)
 *************************************************************/
FILE *lsd::object::search_txt_data( const char *name, const char *init, const char *str )
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


/*************************************************************
 SAVE_TXT_CONFIGURATION (LEGACY)
 Save current defined configuration (adding tag index if
 appropriate) to gzip-compressed xml file
 If quick is true, just the structure and the parameters are
 saved
 Returns: true: save ok, false: save failure
 *************************************************************/
bool lsd::simulation::save_txt_configuration( const char *dest_path, const char *rname, const char *ext, const char eq_file[ ], const char eq_txt[ ] )
{
	bool saved = false;
	char *save_file, *bak_file;
	int save_len;
	FILE *f;

	save_len = strlen( dest_path ) + strlen( rname ) + strlen( ext ) + 2;
	save_file = new char [ save_len ];
	snprintf( save_file, save_len, "%s%s%s%s", dest_path, strlen( dest_path ) > 0 ? "/" : "", rname, ext );

	f = fopen( save_file, "r" );
	if ( f != NULL )
	{
		fclose( f );

		// create backup file
		save_len = strlen( save_file ) - strlen( ext ) + 5;
		bak_file = new char [ save_len ];
		snprintf( bak_file, save_len, "%s%s%s.bak", dest_path, strlen( dest_path ) > 0 ? "/" : "", rname );

		f = fopen( bak_file, "r" );
		if ( f != NULL )
		{
			fclose( f );

			if( ! remove( bak_file ) )
				rename( save_file, bak_file );
		}
		else
			rename( save_file, bak_file );

		delete [ ] bak_file;
	}

	f = fopen( save_file, "wb" );
	delete [ ] save_file;

	if ( f != NULL )
	{
		root->save_txt_struct( f, "" );
		fprintf( f, "\nDATA\n" );
		root->save_txt_insts( f );

		fprintf( f, "\nSIM_NUM %d\nSEED %d\nMAX_STEP %d", last_run, seed, last_t );

		if ( deb_t > 0 || stack_info > 0 || prof_min_msecs > 0 || prof_obs_only || prof_aggr_time || no_ptr_chk || parallel_disable )
			fprintf( f, " %d %d %d %d %d %d %d", deb_t, stack_info, prof_min_msecs, prof_obs_only ? 1 : 0, prof_aggr_time ? 1 : 0, no_ptr_chk ? 1 : 0, parallel_disable ? 1 : 0 );

		fprintf( f, "\nEQUATION %s\nMODELREPORT %s\n", eq_file, rep_file );

		fprintf( f, "\nDESCRIPTION\n\n" );
		root->save_txt_descr( f );

		fprintf( f, "\nDOCUOBSERVE\n" );
		if ( desc != NULL )
			for ( auto & cd : desc->elem )
				if ( cd.observe )
					fprintf( f, "%s\n", cd.label );
		fprintf( f, "\nEND_DOCUOBSERVE\n\n" );

		fprintf( f, "\nDOCUINITIAL\n" );
		if ( desc != NULL )
			for ( auto & cd : desc->elem )
				if ( cd.initial )
					fprintf( f, "%s\n", cd.label );
		fprintf( f, "\nEND_DOCUINITIAL\n\n" );

		if ( eq_txt != NULL && ( strlen( conf_eq_txt ) == 0 || strcmp( conf_eq_txt, eq_txt ) != 0 ) )
			strcpyn( conf_eq_txt, eq_txt, MAX_FILE_SIZE );

		fprintf( f, "\nEQ_FILE\n" );
		fprintf( f, "%s", conf_eq_txt );
		fprintf( f, "\nEND_EQ_FILE\n" );

		saved = ! ferror( f );
		fclose( f );
	}

	return saved;
}


/*************************************************************
 OBJECT::SAVE_TXT_STRUCT (LEGACY)
 Save the object structure tree under this object
 to a LEGACY text file
 *************************************************************/
void lsd::object::save_txt_struct( FILE *f, const char *tab )
{
	char tab1[ MAX_ELEM_LENGTH ];
	bridge *cb;
	variable *cv;

	if ( up == NULL )
		fprintf( f, "\t\n" );

	strcpyn( tab1, tab, MAX_ELEM_LENGTH );
	fprintf( f, "%sLabel %s\n%s{\n", tab1, attr->label, tab1 );
	strcatn( tab1, "\t", MAX_ELEM_LENGTH );

	for ( cb = b; cb != NULL; cb = cb->next )
	{
		fprintf( f, "%sSon: %s\n", tab1, cb->attr->label );

		if ( cb->head == NULL )
			attr->cont->sim->blueprint->search( cb->attr )->save_txt_struct( f, tab1 );
		else
			cb->head->save_txt_struct( f, tab1 );
	}

	for ( cv = v; cv != NULL; cv = cv->next )
	{
		if ( cv->param == 0 )
			fprintf( f, "%sVar: %s\n", tab1, cv->attr->label );

		if ( cv->param == 1 )
			fprintf( f, "%sParam: %s\n", tab1, cv->attr->label );

		if ( cv->param == 2)
			fprintf( f, "%sFunc: %s\n", tab1, cv->attr->label );
	}

	fprintf( f, "\n" );
	fprintf( f, "%s}\n\n", tab );
}


/*************************************************************
 OBJECT::SAVE_TXT_INSTS (LEGACY)
 Save the object instances of tree under this
 object to a LEGACY text file
 *************************************************************/
void lsd::object::save_txt_insts( FILE *f )
{
	int i, count;
	char ch1, ch2, ch3, ch4;
	bridge *cb;
	object *cur;
	variable *cv, *cv1;

	fprintf( f, "\nObject: %s", attr->label );

	if ( to_compute )
		fprintf( f, " C" );
	else
		fprintf( f, " N" );

	for ( cur = this; cur != NULL; cur = cur->hyper_next( ) )
	{
		next_count( cur, &count );
		fprintf( f, "\t%d", count );
		for ( ; BROTHER( cur ) != NULL; cur = cur->next );
	}
	fprintf( f, "\n" );

	for ( cv = v; cv != NULL; cv = cv->next )
	{
		// search for unloaded data
		ch2 = '+';
		if ( cv->param == 1 || cv->attr->num_lag > 0 )
			for ( cur = this; cur != NULL; cur = cur->hyper_next( ) )
			{
				cv1 = cur->search_var( NULL, cv->attr );
				if ( ! cv1->attr->initialized )
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

		ch1 = cv->attr->save ? 's' : 'n';
		ch1 = cv->attr->savei ? toupper( ch1 ) : ch1;
		ch3 = cv->deb_mode;
		ch4 = cv->plot ? 'p' : 'n';
		ch4 = cv->attr->parallel ? toupper( ch4 ) : ch4;

		if ( cv->param == 0 )
			fprintf( f, "Var: %s %d %c %c %c %c", cv->attr->label, cv->attr->num_lag, ch1, ch2, ch3, ch4 );
		if ( cv->param == 1 )
			fprintf( f, "Param: %s %d %c %c %c %c", cv->attr->label, cv->attr->num_lag, ch1, ch2, ch3, ch4 );
		if ( cv->param == 2 )
			fprintf( f, "Func: %s %d %c %c %c %c", cv->attr->label, cv->attr->num_lag, ch1, ch2, ch3, ch4 );

		for ( cur = this; cur != NULL; cur = cur->hyper_next( ) )
		{
			cv1 = cur->search_var( NULL, cv->attr );
			if ( cv1->param == 1 )
				if ( cv1->attr->initialized )
					fprintf( f, "\t%.15g", cv1->chk_val( cv1->val[ 0 ] ) );
				else
					fprintf( f, "\t%c", '0' );
			else
				for ( i = 0; i < cv->attr->num_lag; ++i )
					if ( cv1->attr->initialized )
						fprintf( f, "\t%.15g", cv1->chk_val( cv1->val[ i ] ) );
					else
						fprintf( f, "\t%c", '0' );
		}

		// add optional special updating data
		if ( cv->param == 0 && ( cv->attr->delay > 0 || cv->attr->delay_range > 0 || cv->attr->period > 1 || cv->attr->period_range > 0 ) )
			fprintf( f, "\t<upd: %d %d %d %d>", cv->attr->delay, cv->attr->delay_range, cv->attr->period, cv->attr->period_range );

		fprintf( f, "\n" );
	}

	for ( cb = b; cb != NULL; cb = cb->next )
		if ( cb->head != NULL )
			cb->head->save_txt_insts( f );
}


/*************************************************************
 SAVE_TXT_DESCRIPTION (LEGACY)
 save the descriptions of elements of tree under
 this object to a LEGACY text file
 *************************************************************/
void lsd::object::save_txt_descr( FILE *f )
{
	auto cd = desc != NULL ? desc->search_descr( attr->label, true ) : NULL;
	if ( cd != NULL )
		if ( strwsp( cd->init ) )
			fprintf( f, "%s_%s\n%s\n%s\n\n", cd->type, cd->label, cd->text, desc_key_words[ 1 ] );
		else
			fprintf( f, "%s_%s\n%s\n%s\n%s\n%s\n\n", cd->type, cd->label, cd->text, desc_key_words[ 0 ], cd->init, desc_key_words[ 1 ] );
	else
		fprintf( f, "Object_%s\n%s\n\n", attr->label, desc_key_words[ 1 ] );


	for ( auto cv = v; cv != NULL; cv = cv->next )
	{
		auto cd = desc != NULL ? desc->search_descr( cv->attr->label, true ) : NULL;
		if ( cd != NULL )
			if ( ( cv->param != 1 && cv->attr->num_lag == 0 ) || strwsp( cd->init ) )
				fprintf( f, "%s_%s\n%s\n%s\n\n", cd->type, cd->label, cd->text, desc_key_words[ 1 ] );
			else
				fprintf( f, "%s_%s\n%s\n%s\n%s\n%s\n\n", cd->type, cd->label, cd->text, desc_key_words[ 0 ], cd->init, desc_key_words[ 1 ] );
		else
			fprintf( f, "%s_%s\n%s\n\n", cv->param == 1 ? "Parameter" : "Variable", cv->attr->label, desc_key_words[ 1 ] );
	}

	for ( auto cb = b; cb != NULL; cb = cb->next )
		if ( cb->head != NULL )
			cb->head->save_txt_descr( f );
}


/*************************************************************
 SAVE_SINGLE
 Save the value of a single
 element to file during run
 *************************************************************/
void lsd::variable::save_single( void )
{
	char fn[ MAX_PATH_LENGTH ];
	int i;
	FILE *f;

	// prevent concurrent use by more than one thread
	rec_lguardT lock( var_comp_lck );

	set_lab_tit( );
	snprintf( fn, MAX_PATH_LENGTH, "%s_%s-%d_%d_seed-%d.res",
			  attr->label, lab_tit, start, end, attr->cont->sim->seed - 1 );
	f = fopen( fn, "wt" );			// use text mode for Windows better compatibility

	fprintf( f, "%s %s (%d %d)\t\n", attr->label, lab_tit, start, end );

	for ( i = 0; i <= attr->cont->sim->t - 1; ++i )
		if ( i >= start && i <= end && ! std::isnan( data[ i - start ] ) )// save NaN as n/a
			fprintf( f,"%lf\t\n", data[ i - start ] );
		else
			fprintf( f,"%s\t\n", nonavail );

	fclose( f );
}


/*************************************************************
 SENSITIVITY CONSTRUCTOR
 Add or update sensitivity settings for a model element
 *************************************************************/
lsd::sensitivity::sensitivity( const char *lab, simulation *_sim, int _param, int _lag, bool _integer, int _num_val, d_vecT *_val )
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


/*************************************************************
 SENSITIVITY DESTRUCTOR
 Remove sensitivity settings for a model element
 *************************************************************/
lsd::sensitivity::~sensitivity( void )
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


/*************************************************************
 EMPTY_SENSITIVITY
 Deallocate sensitivity analysis memory
 *************************************************************/
void lsd::simulation::empty_sensitivity( sensitivity *cs )
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


/*************************************************************
 LOAD_TXT_SENSITIVITY
 Load defined sensitivity analysis configuration
 Returns: 0: load ok, 1,2,3,4,...: load failure
 *************************************************************/
int lsd::simulation::load_txt_sensitivity( FILE *f )
{
	bool integer;
	d_vecT val;
	int i, lag, param, num_val, err = 0;
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

		cv = root->search_var( root, lab );
		if ( cv == NULL || ( cv->param != 1 && cv->attr->num_lag == 0 ) )
		{
			err = 1;						// and not parameter or lagged variable
			break;
		}

		// get lags and # of values to test
		if ( fscanf( f, "%d %d ", & lag, & num_val ) < 2 )
		{
			err = 2;
			break;
		}

		// get variable type (newer versions)
		if ( fscanf( f, "%c ", &cc ) < 1 )
		{
			err = 3;
			break;
		}

		if ( cc == 'i' || cc == 'd' || cc == 'f' )
		{
			integer = ( cc == 'i' ) ? true : false;
			fscanf( f, ": " );				// remove separator
		}
		else
			if ( cc == ':' )
				integer = false;
			else
			{
				err = 4;
				break;
			}

		if ( lag == 0 )						// adjust type and lag #
			param = 1;
		else
		{
			param = 0;
			lag = abs( lag ) - 1;
		}

		for ( val.resize( num_val ), i = 0; i < num_val; ++i )
			if ( ! fscanf( f, "%lf", & val[ i ] ) )
			{
				err = 5;
				break;
			}

		if ( ( cs = search_sensitivity( lab, lag ) ) != NULL )
			delete cs;

		new sensitivity( lab, this, param, lag, integer, num_val, & val );
	}

	if ( err != 0 )
		empty_sensitivity( );				// discard read data

	return err;
}


/*************************************************************
 SEARCH_SENSITIVITY
 Find element in sensitivity data linked list
 *************************************************************/
lsd::sensitivity *lsd::simulation::search_sensitivity( const char *lab, int lag )
{
	sensitivity *cs;

	for ( cs = sens; cs != NULL; cs = cs->next )
		if ( ! strcmp( cs->label, lab ) &&
			 ( cs->param == 1 || cs->lag == lag ) )
			 break;

	return cs;
}


/*************************************************************
 RESULT::CONSTRUCTOR
 Open the appropriate file for saving the results
 *************************************************************/
lsd::result::result( const char *fname, const char *fmode, simulation *_sim, bool _dozip, bool _docsv )
{
	sim = _sim;
	docsv = _docsv;
	dozip = _dozip;

	if ( da != NULL && ! da->disable && da->ref_sim == sim )
		da_res = true;

	if ( dozip )
		fz = gzopen( fname, fmode );
	else
		f = fopen( fname, fmode );
}


/*************************************************************
 RESULT::DESTRUCTOR
 Close the results file
 *************************************************************/
lsd::result::~result( void )
{
	if ( dozip )
		gzclose( fz );
	else
		fclose( f );
}


/*************************************************************
 RESULT::TITLE
 Creates header in results file
 *************************************************************/
void lsd::result::title( object *root, int flag )
{
	if ( da != NULL )
		da->reset_insts( );				// used instances of all DA elements

	first_col = true;

	title_recursive( root, flag );		// output header

	if ( dozip )						// and change line
		gzprintf( fz, "\n" );
	else
		fprintf( f, "\n" );
}


/*************************************************************
 RESULT::TITLE_RECURSIVE
 Recursively add elements to header of results file
 *************************************************************/
void lsd::result::title_recursive( object *r, bool header )
{
	ass_map_itT ca;
	object *cur;

	for ( auto cv = r->v; cv != NULL; cv = cv->next )
		if ( cv->attr->save )
		{
			cv->set_lab_tit( );

			if ( da_res )
			{
				// check if there are still instances to be presented
				// because of DA data analysis, dynamic instances may have to enter
				// the DA process, but were still used in the model forecasts
				if ( da != NULL && ( ca = da->elem_map.find( cv->attr->label ) ) != da->elem_map.end( ) && ca->second->inst_idx + 1 < ( int ) ca->second->da_data.size( ) )
				{
					auto & ce = ca->second->da_data[ ++( ca->second->inst_idx ) ];
					if ( ! ce.saved )
					{
						for ( auto tag = TAG_ANL; tag <= TAG_OBS; ++tag )
							if ( ! ( tag == TAG_FCT && ! da->sav_fct ) && ! ( tag == TAG_OBS && ! da->sav_obs ) )
							{
								write_title( cv->attr->label, cv->lab_tit, cv->up, tag, header, ce.start, ce.end );

								if ( da->sav_ci )
								{
									write_title( to_string( "%s+", cv->attr->label ).c_str( ), cv->lab_tit, cv->up, tag, header, ce.start, ce.end );
									write_title( to_string( "%s-", cv->attr->label ).c_str( ), cv->lab_tit, cv->up, tag, header, ce.start, ce.end );
								}
							}

						ce.saved = true;
					}
				}
			}
			else
				write_title( cv->attr->label, cv->lab_tit, cv->up, TAG_NONE, header, cv->start, cv->end );
		}

	for ( auto cb = r->b; cb != NULL; cb = cb->next )
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
		for ( auto cv = sim->cemetery; cv != NULL; cv = cv->next )
		{
			if ( da_res )
			{
				if ( da != NULL && ( ca = da->elem_map.find( cv->attr->label ) ) != da->elem_map.end( ) && ca->second->inst_idx + 1 < ( int ) ca->second->da_data.size( ) )
				{
					auto & ce = ca->second->da_data[ ++( ca->second->inst_idx ) ];
					if ( ! ce.saved )
					{
						for ( auto i = TAG_ANL; i <= TAG_OBS; ++i )
							if ( ! ( i == TAG_FCT && ! da->sav_fct ) && ! ( i == TAG_OBS && ! da->sav_obs ) )
							{
								write_title( cv->attr->label, cv->lab_tit, cv->up, i );

								if ( da->sav_ci )
								{
									write_title( to_string( "%s_ci+", cv->attr->label ).c_str( ), cv->lab_tit, cv->up, i );
									write_title( to_string( "%s_ci-", cv->attr->label ).c_str( ), cv->lab_tit, cv->up, i );
								}
							}

						ce.saved = true;
					}
				}
			}
			else
				write_title( cv->attr->label, cv->lab_tit, cv->up, TAG_NONE );
		}
}


/*************************************************************
 RESULT::WRITE_TITLE
 Write a single element to header of results file
 *************************************************************/
void lsd::result::write_title( const char *lab, const char *lab_tit, object *par, int tag, bool header, int start, int end )
{
	bool just_name = false;
	varattr *va;

	if ( par == NULL && ( va = sim->va.search( lab ) ) != NULL )
		par = sim->root->search( va->par_attr->label );

	// prevent adding suffix to single objects
	if ( tag == 0 && ( ! strcmp( lab_tit, "1" ) || ! strcmp( lab_tit, "1_1" ) || ! strcmp( lab_tit, "1_1_1" ) || ! strcmp( lab_tit, "1_1_1_1" ) || ! strcmp( lab_tit, "1_1_1_1_1" ) || ! strcmp( lab_tit, "1_1_1_1_1_1" ) || ! strcmp( lab_tit, "1_1_1_1_1_1_1" ) || ! strcmp( lab_tit, "1_1_1_1_1_1_1_1" ) || ! strcmp( lab_tit, "1_1_1_1_1_1_1_1_1" ) || ! strcmp( lab_tit, "1_1_1_1_1_1_1_1_1_1" ) ) && ( par == NULL || par->hyper_next( ) == NULL ) )
		just_name = true;

	if ( header )
		if ( dozip )
			if ( docsv )
				gzprintf( fz, "%s%s%s%s%s", first_col ? "" : CSV_SEP, lab, just_name ? "" : "_", just_name ? "" : tag_pref[ tag ], just_name ? "" : lab_tit );
			else
				gzprintf( fz, "%s %s%s (%d %d)\t", lab, tag_pref[ tag ], lab_tit, start, end );
		else
			if ( docsv )
				fprintf( f, "%s%s%s%s%s", first_col ? "" : CSV_SEP, lab, just_name ? "" : "_", just_name ? "" : tag_pref[ tag ], just_name ? "" : lab_tit );
			else
				fprintf( f, "%s %s%s (%d %d)\t", lab, tag_pref[ tag ], lab_tit, start, end );
	else
		if ( dozip )
			if ( docsv )
				gzprintf( fz, "%s%s%s%s%s", first_col ? "" : CSV_SEP, lab, just_name ? "" : "_", just_name ? "" : tag_pref[ tag ], just_name ? "" : lab_tit );
			else
				gzprintf( fz, "%s %s%s (-1 -1)\t", lab, tag_pref[ tag ], lab_tit );
		else
			if ( docsv )
				fprintf( f, "%s%s%s%s%s", first_col ? "" : CSV_SEP, lab, just_name ? "" : "_", just_name ? "" : tag_pref[ tag ], just_name ? "" : lab_tit );
			else
				fprintf( f, "%s %s%s (-1 -1)\t", lab, tag_pref[ tag ], lab_tit );

	first_col = false;
}


/*************************************************************
 RESULT::DATA
 Adds data to results file in the specified period
 *************************************************************/
void lsd::result::data( object *root, int initstep, int endtstep )
{
	// don't include initialization (t=0) in .csv format
	initstep = ( docsv && initstep < 1 ) ? 1 : initstep;

	// adjust for 1 time step if needed
	endtstep = ( endtstep == 0 ) ? initstep : endtstep;

	for ( int t = initstep; t <= endtstep; t++ )
	{
		if ( da != NULL )
			da->reset_insts( );			// used instances of all DA elements

		first_col = true;

		data_recursive( root, t );		// output one data line

		if ( dozip )					// and change line
			gzprintf( fz, "\n" );
		else
			fprintf( f, "\n" );
	}
}


/*************************************************************
 RESULT::DATA_RECURSIVE
 Recursively add data to results file
 *************************************************************/
void lsd::result::data_recursive( object *r, int t )
{
	double *data;
	ass_map_itT ca;
	object *cur;

	for ( auto cv = r->v; cv != NULL; cv = cv->next )
		if ( cv->attr->save )
		{
			if ( da_res )
			{
				if ( da != NULL && ( ca = da->elem_map.find( cv->attr->label ) ) != da->elem_map.end( ) && ca->second->inst_idx + 1 < ( int ) ca->second->da_data.size( ) )
				{
					auto & ce = ca->second->da_data[ ++( ca->second->inst_idx ) ];
					if ( ! ce.saved )
					{
						for ( auto i = TAG_ANL; i <= TAG_OBS; ++i )
						{
							if ( ( i == TAG_FCT && ! da->sav_fct ) || ( i == TAG_OBS && ! da->sav_obs ) )
								continue;

							data = ( i == TAG_OBS ? ce.obs.data( ) : ( i == TAG_FCT ? ce.fct.data( ) : ce.anl.data( ) ) );
							write_data( data, t, ce.start, ce.end );

							if ( da->sav_ci )
							{
								data = ( i == TAG_OBS ? ce.obs_hi.data( ) : ( i == TAG_FCT ? ce.fct_hi.data( ) : ce.anl_hi.data( ) ) );
								write_data( data, t, ce.start, ce.end );
								data = ( i == TAG_OBS ? ce.obs_lo.data( ) : ( i == TAG_FCT ? ce.fct_lo.data( ) : ce.anl_lo.data( ) ) );
								write_data( data, t, ce.start, ce.end );
							}
						}

						ce.saved = true;
					}
				}
			}
			else
				write_data( cv->data, t, cv->start, cv->end );
		}

	for ( auto cb = r->b; cb != NULL; cb = cb->next )
	{
		if ( cb->head == NULL )
			continue;

		cur = cb->head;
		if ( cur->to_compute )
			for ( ; cur != NULL; cur = cur->next )
				data_recursive( cur, t );
	}

	if ( r->up == NULL )
		for ( auto cv = sim->cemetery; cv != NULL; cv = cv->next )
		{
			if ( da_res )
			{
				if ( da != NULL && ( ca = da->elem_map.find( cv->attr->label ) ) != da->elem_map.end( ) && ca->second->inst_idx + 1 < ( int ) ca->second->da_data.size( ) )
				{
					auto & ce = ca->second->da_data[ ++( ca->second->inst_idx ) ];
					if ( ! ce.saved )
					{
						for ( auto tag = TAG_ANL; tag <= TAG_OBS; ++tag )
						{
							if ( ( tag == TAG_FCT && ! da->sav_fct ) || ( tag == TAG_OBS && ! da->sav_obs ) )
								continue;

							data = ( tag == TAG_OBS ? ce.obs.data( ) : ( tag == TAG_FCT ? ce.fct.data( ) : ce.anl.data( ) ) );
							write_data( data, t, ce.start, ce.end );

							if ( da->sav_ci )
							{
								data = ( tag == TAG_OBS ? ce.obs_hi.data( ) : ( tag == TAG_FCT ? ce.fct_hi.data( ) : ce.anl_hi.data( ) ) );
								write_data( data, t, ce.start, ce.end );
								data = ( tag == TAG_OBS ? ce.obs_lo.data( ) : ( tag == TAG_FCT ? ce.fct_lo.data( ) : ce.anl_lo.data( ) ) );
								write_data( data, t, ce.start, ce.end );
							}
						}

						ce.saved = true;
					}
				}
			}
			else
				write_data( cv->data, t, cv->start, cv->end );
		}
}


/*************************************************************
 RESULT::WRITE_DATA
 Write a single element data to results file
 *************************************************************/
void lsd::result::write_data( double *data, int t, int start, int end )
{
	double val;

	if ( start <= t && t <= end )
		val = data[ t - start ];
	else
		val = NAN;

	if ( std::isfinite( val ) )
		if ( dozip )
			if ( docsv )
				gzprintf( fz, "%s%.*G", first_col ? "" : CSV_SEP, SIG_DIG, val );
			else
				gzprintf( fz, "%.*G\t", SIG_DIG, val );
		else
			if ( docsv )
				fprintf( f, "%s%.*G", first_col ? "" : CSV_SEP, SIG_DIG, val );
			else
				fprintf( f, "%.*G\t", SIG_DIG, val );
	else
		if ( dozip )		// save NaN as n/a
			if ( docsv )
				gzprintf( fz, "%s%s", first_col ? "" : CSV_SEP, nonavail );
			else
				gzprintf( fz, "%s\t", nonavail );
		else
			if ( docsv )
				fprintf( f, "%s%s", first_col ? "" : CSV_SEP, nonavail );
			else
				fprintf( f, "%s\t", nonavail );

	first_col = false;
}
