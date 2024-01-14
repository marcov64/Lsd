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

#include "decl.h"


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
	string warnings;

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

	switch ( i = load_configuration( reload, &warnings ) )// try to load the configuration
	{
		case 0:
			loaded = true;
			break;

		case 1:									// file/path not found
			if ( strlen( path ) > 0 )
				cmd( "ttk::messageBox -parent . -type ok -title Error -icon error -message \"File not found\" -detail \"File for model '%s' not found in directory '%s'.\"", strlen( simul_name ) > 0 ? simul_name : NO_CONF_NAME, path );
			else
				cmd( "ttk::messageBox -parent . -type ok -title Error -icon error -message \"File not found\" -detail \"File for model '%s' not found in current directory\"", strlen( simul_name ) > 0 ? simul_name : NO_CONF_NAME	 );
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
			reset_description( root );
			loaded = true;
			break;

		case 10 ... 11:							// problem from DOCUOBSERVE section
		case 12 ... 13:							// problem from DOCUINITIAL section
			cmd( "ttk::messageBox -parent . -type ok -title Error -icon error -message \"Partially damaged file (%d :%.24s)\" -detail \"Observation flags and equation file were lost but the configuration can still be used.\n\nPlease check if the desired LSD configuration file was selected or re-configure the lost parts if needed.\"", i, warnings.c_str( ) );
			loaded = true;
	}

	if ( i == 0 && warnings.size( ) > 0 )
			cmd( "ttk::messageBox -parent . -type ok -title Warning -icon warning -message \"Partially damaged file (%d :%.24s)\" -detail \"Part of the configuration data was missing or invalid and was replaced by default values.\n\nPlease check if the desired LSD configuration file was selected or re-configure the affected parts as needed.\"", i, warnings.c_str( ) );

	if ( loaded && r != NULL && reload )
		currObj = r = restore_pos( root );		// restore pointed object and variable
	else
		currObj = r = root;						// new structure

	return loaded;
}


/*****************************************************************************
SAVE_XML_CONFIGURATION
	Save current defined configuration (adding tag index if appropriate) to
	gzip-compressed xml file
	If quick is true, just the structure and the parameters are saved
	Returns: true: save ok, false: save failure
******************************************************************************/
bool save_xml_configuration( int findex, const char *dest_path, bool quick )
{
	bool save_ok;
	int delta, indexDig, save_len;
	char ch[ MAX_PATH_LENGTH ], *save_file, *bak_file;
	const char *save_path;
	long node_serial = 1;
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

		delete [ ] bak_file;
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
	<!ELEMENT object (#PCDATA, description?, nodes?, object*, element*)>\n \
	<!ELEMENT description (#PCDATA+)>\n \
	<!ELEMENT nodes (#PCDATA, #PCDATA, #PCDATA?, #PCDATA?, #PCDATA?)>\n \
	<!ELEMENT element (#PCDATA?, description?, documentation?, sensitivity?)>\n \
	<!ELEMENT documentation EMPTY>\n \
	<!ELEMENT sensitivity (#PCDATA+)>\n]" );
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
	root->save_xml_struct( strNode, node_serial, quick );

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

#ifndef _NW_

	if ( save_ok )
		cmd( "set lastConf [ string map -nocase { \"%s/\" \"\" } [ file normalize \"%s\" ] ]", exec_path, struct_file );

#endif

	delete [ ] save_file;

	return save_ok;
}


/****************************************************
OBJECT::SAVE_XML_STRUCT
	Save the object structure tree under this object
	to an xml object
	If quick is true, just the structure and the
	parameters are saved, no descriptions
****************************************************/
void object::save_xml_struct( xml_node &pn, long &node_serial, bool quick )
{
	bool init, nodes, noWht;
	char *str;
	int i, count;
	long l, k;
	string data, text, nser, nid, nnam, lnkto, lnkwht;
	bridge *cb;
	description *cd;
	netLink *curl;
	object *cur;
	sense *cs;
	variable *cv, *cv1;

	xml_node n = pn.append_child( "object" );
	n.append_attribute( "name" ) = label;

	if ( ! to_compute )
		n.append_attribute( "compute" ) = false;

	for ( data = "", nodes = false, cur = this; cur != NULL;
		  cur = cur->hyper_next( cur->label ) )
	{
		if ( cur != this )
			data += ",";

		skip_next_obj( cur, &count );
		data += to_string( count );

		for ( ; go_brother( cur ) != NULL; cur = cur->next )
			if ( cur->node != NULL )	// check if object contains network nodes
				nodes = true;
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

	// save network attributes and links
	l = k = 0;
	noWht = true;
	if ( nodes )
	{	// first save nodes and attribute serials
		for ( cur = this; cur != NULL; ++l, cur = cur->hyper_next( cur->label ) )
		{
			if ( cur != this )
			{
				nser += ",";
				nid += ",";
				nnam += ",";
			}

			if ( cur->node != NULL )
			{
				cur->node->serNum = node_serial++;
				nser += to_string( cur->node->serNum );
				nid += to_string( cur->node->id );

				if ( cur->node->name != NULL )
					nnam += "\"" + ( data = cur->node->name ) + "\"";
				else
					nnam += "\"\"";
			}
		}

		// second save links using serials for destination
		for ( cur = this; cur != NULL; cur = cur->hyper_next( cur->label ) )
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

					if ( curl->ptrTo == NULL || curl->ptrTo->node == NULL )
						continue;				// ignore invalid link

					lnkto += to_string( curl->ptrTo->node->serNum );
					lnkwht += to_string( "%.15g", curl->weight );

					if ( curl->weight != 0 )
						noWht = false;
				}
		}

		xml_node nd = n.append_child( "nodes" );
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
			blueprint->search( cb->blabel )->save_xml_struct( n, node_serial, quick );
		else
			cb->head->save_xml_struct( n, node_serial, quick );

	// save elements (parameters, variables and functions)
	for ( cv = v; cv != NULL; cv = cv->next )
	{
		xml_node cn = n.append_child( "element" );
		cn.append_attribute( "name" ) = cv->label;
		cn.append_attribute( "type" ) = elem_type_names[ cv->param ];

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
			for ( data = "", cur = this; cur != NULL;
				  cur = cur->hyper_next( label ) )
			{
				if ( cur != this )
					data += ";";

				cv1 = cur->search_var( NULL, cv->label );
				for ( i = 0; i < ( cv1->param == 1 ? 1 : cv1->num_lag ); ++i )
				{
					if ( i != 0 )
						data += ",";

					data += to_string( "%.15g", cv1->initialized ? cv1->val[ i ] : 0 );
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

		// add sensitivity analysis data
		for ( cs = rsense; cs != NULL; cs = cs->next )
			if ( strcmp( cs->label, cv->label ) == 0 )
			{
				if ( cs->integer )
					cn.append_attribute( "integer" ) = true;

				xml_node cns;

				if ( cn.child( "sensitivity" ).empty( ) )
					cns = cn.append_child( "sensitivity" );
				else
					cns = cn.child( "sensitivity" );

				for ( data = "", i = 0; cs->v != NULL && i < cs->numv; ++i )
				{
					if ( i != 0 )
						data += ",";

					data += to_string( "%.15g", cs->v[ i ] );
				}

				if ( cv->param )
					text = "values";
				else
					text = "values-" + to_string( cs->lag + 1 );

				cns.append_child( text.c_str( ) ).text( ) = data.c_str( );
			}
	}
}


/*****************************************************************************
SAVE_CONFIGURATION (LEGACY)
	Save current defined configuration (adding tag index if appropriate) to
	gzip-compressed xml file
	If quick is true, just the structure and the parameters are saved
	Returns: true: save ok, false: save failure
******************************************************************************/
bool save_configuration( const char *path, const char *rname, const char *ext )
{
	bool save_ok = false;
	char *save_file, *bak_file;
	description *cd;
	FILE *f;

	save_file = new char[ strlen( path ) + strlen( rname ) + strlen( ext ) + 2 ];
	sprintf( save_file, "%s%s%s%s", path, strlen( path ) > 0 ? "/" : "", rname, ext );

	f = fopen( save_file, "r" );
	if ( f != NULL )
	{
		fclose( f );

		// create backup file
		bak_file = new char[ strlen( save_file ) - strlen( ext ) + 5 ];
		sprintf( bak_file, "%s%s%s.bak", path, strlen( path ) > 0 ? "/" : "", rname );

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
		root->save_struct( f, "" );
		fprintf( f, "\nDATA\n" );
		root->save_insts( f );

		fprintf( f, "\nSIM_NUM %d\nSEED %d\nMAX_STEP %d", sim_num, seed, max_step );

		if ( when_debug > 0 || stack_info > 0 || prof_min_msecs > 0 || prof_obs_only || prof_aggr_time || no_ptr_chk || parallel_disable )
			fprintf( f, " %d %d %d %d %d %d %d", when_debug, stack_info, prof_min_msecs, prof_obs_only ? 1 : 0, prof_aggr_time ? 1 : 0, no_ptr_chk ? 1 : 0, parallel_disable ? 1 : 0 );

		fprintf( f, "\nEQUATION %s\nMODELREPORT %s\n", equation_name, name_rep );

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

		save_ok = ! ferror( f );
		fclose( f );
	}

	return save_ok;
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
		fprintf( f, "%s_%s\n%s\n%s\n\n", cd->type, cd->label, cd->text, desc_key_words[ 1 ] );
	else
		fprintf( f, "%s_%s\n%s\n%s\n%s\n%s\n\n", cd->type, cd->label, cd->text, desc_key_words[ 0 ], cd->init, desc_key_words[ 1 ] );

	for ( cv = r->v; cv != NULL; cv = cv->next )
	{
		cd = search_description( cv->label );

		if ( ( cv->param != 1 && cv->num_lag == 0 ) || strwsp( cd->init ) )
			fprintf( f, "%s_%s\n%s\n%s\n\n", cd->type, cd->label, cd->text, desc_key_words[ 1 ] );
		else
			fprintf( f, "%s_%s\n%s\n%s\n%s\n%s\n\n", cd->type, cd->label, cd->text, desc_key_words[ 1 ], cd->init, desc_key_words[ 1 ] );
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
	bool integer;
	vector < double > v;
	int i, lag, param, numv;
	char cc, lab[ MAX_ELEM_LENGTH ];
	variable *cv;
	sense *cs;

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

		// get lags and # of values to test
		if ( fscanf( f, "%d %d ", &lag, &numv ) < 2 )
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

		for ( v.resize( numv ), i = 0; i < numv; ++i )
			if ( ! fscanf( f, "%lf", &v[ i ] ) )
				goto error5;

		if ( ( cs = search_sensitivity( lab, lag ) ) != NULL )
			delete cs;

		new sense( lab, param, lag, numv, &v, integer );
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

	empty_sensitivity( );					// discard read data

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
	sense *cs;

	for ( cs = rsense; cs != NULL; cs = cs->next )
	{
		if ( cs->param == 1 )
			fprintf( f, "%s 0 %d %c:", cs->label, cs->numv, cs->integer ? 'i' : 'f' );
		else
			fprintf( f, "%s -%d %d %c:", cs->label, cs->lag + 1, cs->numv, cs->integer ? 'i' : 'f' );

		for ( i = 0; cs->v != NULL && i < cs->numv; ++i )
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
		for ( i = 0; cs->v != NULL &&  i < cs->numv; ++i )
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
