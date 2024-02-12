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
	string warnings;

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

	if ( r != NULL && reload )
		save_pos( r );							// save current position when reloading

	redrawRoot = redrawStruc = true;			// force browser/structure redraw
	iniShowOnce = false;						// show warning on # of columns in .ini

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
			sim.reset_description( sim.root );
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

	if ( loaded && r != NULL && reload )
		currObj = r = restore_pos( sim.root );	// restore pointed object and variable
	else
		currObj = r = sim.root;					// new structure

	if ( loaded )
	{
		load_elem_lists( sim.root );

		if ( ! ignore_eq_file && strncmp( sim.conf_eq_txt, eq_txt, min( strlen( sim.conf_eq_txt ), strlen( eq_txt ) ) ) )
			plog( "\nWarning: the configuration file has been previously run with different equations\nfrom those used to create the LSD model program.\nChanges may affect the simulation results. You can offload the original\nequations in a new equation file and compare differences using TkDiff in LMM\n(menu File)." );
	}

	return loaded;
}


/****************************************************
LOAD_PREV_CONFIGURATION
Restore sensitivity configuration
****************************************************/
bool load_prev_configuration( void )
{
	char *saFile = NULL;
	int i, lstFidx = findexSens;
	string warnings;
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
	else
	{
		load_elem_lists( sim.root );
		cmd( "set lastConf [ string map -nocase { \"%s/\" \"\" } [ file normalize \"%s\" ] ]", model_path, sim.conf_file );
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
LOAD_CONFIGURATION_GUI (DLL WRAPPER)
	Load configuration
	If full is false, just the model data is unloaded
	Returns: pointer to root object
******************************************************************************/
int load_configuration_gui( bool reload, string *warnings, int quick )
{
	int res = sim.load_configuration( reload, warnings, quick );

	unsavedData = false;						// no unsaved simulation results
	unsavedSense = false;						// no sensitivity data to save

	return res;
}


/*****************************************************************************
UNLOAD_CONFIGURATION_GUI (DLL WRAPPER)
	Unload the current configuration
	If full is false, just the model data is unloaded
	Returns: pointer to root object
******************************************************************************/
void unload_configuration_gui( bool full )
{
	sim.unload_configuration( full );

	currObj = NULL;								// no current object pointer
	unsaved_change( false );					// signal no unsaved change
	unsavedData = false;						// no unsaved simulation results
	unsavedSense = false;						// no sensitivity data to save
	findexSens = 0;								// reset sensitivity serial number

	NOLH_clear( );								// deallocate DoE
	sim.empty_sensitivity( );					// discard sensitivity analysis data

	cmd( "destroytop .lat" );					// remove lattice window
	cmd( "unset -nocomplain modObj modElem modVar modPar modFun" );	// no elements in model structure

	if ( ! sim.running )
		cmd( "destroytop .plt" );				// remove run-time plot window

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
void load_elem_lists( object *r )
{
	bridge *cb;
	variable *cv;

	if ( r->up == NULL )						// reset lists if root
		cmd( "unset -nocomplain modObj modElem modVar modPar modFun" );
	else
		cmd( "lappend modObj %s", r->label );	// register object if not root

	// register elements in object
	for ( cv = r->v; cv != NULL; cv = cv->next )
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
	for ( cb = r->b; cb != NULL; cb = cb->next )
		load_elem_lists( cb-> head );
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
	bool saved;
	int delta, indexDig, save_len;
	char ch[ MAX_PATH_LENGTH ], *save_file, *bak_file;
	const char *save_path;
	long node_serial = 1;
	FILE *f;
	gzFile fz;
	ostringstream buf;
	xml_doc xf;

	delta = ( findex > 0 ) ? sim.last_run * ( findex - 1 ) : 0;
	indexDig = ( findex > 0 ) ? ( int ) floor( log10( findex ) + 2 ) : 0;

	if ( dest_path == NULL )
		save_path = sim.conf_path;
	else
		save_path = dest_path;

	if ( strlen( sim.conf_name ) == 0 )
	{
		delete [ ] sim.conf_name;
		sim.conf_name = new char[ strlen( DEF_CONF_FILE ) + 1 ];
		strcpy( sim.conf_name, DEF_CONF_FILE );
	}

	if ( strlen( sim.rep_file ) == 0 )
		snprintf( sim.rep_file, MAX_PATH_LENGTH, "report_%s.html", sim.conf_name );

	if ( strlen( sim.conf_path ) > 0 )
	{
		save_len = strlen( save_path ) + strlen( sim.conf_name ) + 6 + indexDig;
		save_file = new char[ save_len ];
		sprintf( save_file, "%s/%s", save_path, sim.conf_name );
	}
	else
	{
		save_len = strlen( sim.conf_name ) + 6 + indexDig;
		save_file = new char[ save_len ];
		sprintf( save_file, "%s", sim.conf_name );
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
	simNode.append_attribute( "steps" ) = sim.last_t;
	simNode.append_attribute( "runs" ) = sim.last_run;
	simNode.append_attribute( "seed" ) = sim.seed + delta;

	// optional settings (include only if non-default)
	if ( sim.deb_t > 0 )
		simNode.append_attribute( "debug_start" ) = sim.deb_t;

	if ( sim.no_ptr_chk )
		simNode.append_attribute( "ptr_check" ) = false;

	if ( sim.parallel_disable )
		simNode.append_attribute( "parallel" ) = false;

	// add profile settings, if any
	if ( sim.stack_info > 0 || sim.prof_min_msecs > 0 || sim.prof_obs_only || sim.prof_aggr_time )
	{
		xml_node profNode = setNode.append_child( "profiling" );

		if ( sim.stack_info > 0 )
			profNode.append_attribute( "level" ) = sim.stack_info;

		if ( sim.prof_min_msecs > 0 )
			profNode.append_attribute( "time" ) = sim.prof_min_msecs;

		if ( sim.prof_obs_only )
			profNode.append_attribute( "observed" ) = true;

		if ( sim.prof_aggr_time )
			profNode.append_attribute( "aggregate" ) = true;
	}

	// add report file name
	setNode.append_child( "report_file" ).text( ) = sim.rep_file;

	// add model structure
	xml_node strNode = cfgNode.append_child( "structure" );
	sim.root->save_xml_struct( strNode, node_serial, quick );

	// add equation file name and content
	xml_node eqfNode = cfgNode.append_child( "equation_file" );
	eqfNode.append_child( "filename" ).text( ) = eq_file;

	if ( ! quick )
	{
		if ( eq_txt != NULL && ( strlen( sim.conf_eq_txt ) == 0 || strcmp( sim.conf_eq_txt, eq_txt ) != 0 ) )
			strcpyn( sim.conf_eq_txt, eq_txt, MAX_FILE_SIZE );

		// encode xml ]]> escape sequences
		eqfNode.append_child( "content" ).append_child( pugi::node_cdata ).set_value( strencdata( sim.conf_eq_txt, sim.conf_eq_txt, MAX_FILE_SIZE ) );
	}

	xf.save( buf );

	if ( ( fz = gzopen( save_file, "wb9" ) ) != Z_NULL )
	{
		saved = gzputs( fz, buf.str( ).c_str( ) );
		saved = gzclose( fz ) == Z_OK ? saved : false;
	}
	else
		saved = false;

	if ( saved )
		cmd( "set lastConf [ string map -nocase { \"%s/\" \"\" } [ file normalize \"%s\" ] ]", sim.conf_path, sim.conf_file );

	delete [ ] save_file;

	return saved;
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
		cd = sim->search_description( label );

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

					if ( curl->to == NULL || curl->to->node == NULL )
						continue;				// ignore invalid link

					lnkto += to_string( curl->to->node->serNum );
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
			sim->blueprint->search( cb->blabel )->save_xml_struct( n, node_serial, quick );
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
		cd = sim->search_description( cv->label );

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
		for ( cs = sim->rsense; cs != NULL; cs = cs->next )
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
bool save_configuration( const char *dest_path, const char *rname, const char *ext )
{
	bool saved = false;
	char *save_file, *bak_file;
	description *cd;
	FILE *f;

	save_file = new char[ strlen( dest_path ) + strlen( rname ) + strlen( ext ) + 2 ];
	sprintf( save_file, "%s%s%s%s", dest_path, strlen( dest_path ) > 0 ? "/" : "", rname, ext );

	f = fopen( save_file, "r" );
	if ( f != NULL )
	{
		fclose( f );

		// create backup file
		bak_file = new char[ strlen( save_file ) - strlen( ext ) + 5 ];
		sprintf( bak_file, "%s%s%s.bak", dest_path, strlen( dest_path ) > 0 ? "/" : "", rname );

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
		sim.root->save_struct( f, "" );
		fprintf( f, "\nDATA\n" );
		sim.root->save_insts( f );

		fprintf( f, "\nSIM_NUM %d\nSEED %d\nMAX_STEP %d", sim.last_run, sim.seed, sim.last_t );

		if ( sim.deb_t > 0 || sim.stack_info > 0 || sim.prof_min_msecs > 0 || sim.prof_obs_only || sim.prof_aggr_time || sim.no_ptr_chk || sim.parallel_disable )
			fprintf( f, " %d %d %d %d %d %d %d", sim.deb_t, sim.stack_info, sim.prof_min_msecs, sim.prof_obs_only ? 1 : 0, sim.prof_aggr_time ? 1 : 0, sim.no_ptr_chk ? 1 : 0, sim.parallel_disable ? 1 : 0 );

		fprintf( f, "\nEQUATION %s\nMODELREPORT %s\n", eq_file, sim.rep_file );

		fprintf( f, "\nDESCRIPTION\n\n" );
		sim.root->save_description( f );

		fprintf( f, "\nDOCUOBSERVE\n" );
		for ( cd = sim.descr; cd != NULL; cd = cd->next )
			if ( cd->observe )
				fprintf( f, "%s\n", cd->label );
		fprintf( f, "\nEND_DOCUOBSERVE\n\n" );

		fprintf( f, "\nDOCUINITIAL\n" );
		for ( cd = sim.descr; cd != NULL; cd = cd->next )
			if ( cd->initial )
				fprintf( f, "%s\n", cd->label );
		fprintf( f, "\nEND_DOCUINITIAL\n\n" );

		save_eqfile( f );

		saved = ! ferror( f );
		fclose( f );
	}

	return saved;
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
			sim->blueprint->search( cb->blabel )->save_struct( f, tab1 );
		else
			cb->head->save_struct( f, tab1 );
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
void object::save_description( FILE *f )
{
	bridge *cb;
	variable *cv;
	description *cd;

	cd = sim->search_description( label );

	if ( strwsp( cd->init ) )
		fprintf( f, "%s_%s\n%s\n%s\n\n", cd->type, cd->label, cd->text, desc_key_words[ 1 ] );
	else
		fprintf( f, "%s_%s\n%s\n%s\n%s\n%s\n\n", cd->type, cd->label, cd->text, desc_key_words[ 0 ], cd->init, desc_key_words[ 1 ] );

	for ( cv = v; cv != NULL; cv = cv->next )
	{
		cd = sim->search_description( cv->label );

		if ( ( cv->param != 1 && cv->num_lag == 0 ) || strwsp( cd->init ) )
			fprintf( f, "%s_%s\n%s\n%s\n\n", cd->type, cd->label, cd->text, desc_key_words[ 1 ] );
		else
			fprintf( f, "%s_%s\n%s\n%s\n%s\n%s\n\n", cd->type, cd->label, cd->text, desc_key_words[ 1 ], cd->init, desc_key_words[ 1 ] );
	}

	for ( cb = b; cb != NULL; cb = cb->next )
		if ( cb->head != NULL )
			cb->head->save_description( f );
}


/*******************************************
DEB_LOG
Creates/saves the file "log.txt" and
enable/disable logging the variables
computation order and enable/disable the
debugger
********************************************/
void deb_log( bool on, int time )
{
	// check if should turn off
	if ( ! on || sim.parallel_mode || sim.fast_mode != 0 )
	{
		// disable debugging
		if ( time > sim.t && sim.deb_t >= time )
			sim.deb_t = 0;
		else
			if ( ( time == 0 && sim.deb_t == sim.t ) || time == sim.t )
				deb_set = false;

		// act now?
		if ( time == 0 || sim.t > time )
		{
			// close file if open
			if ( log_file_ptr != NULL )
			{
				fclose( log_file_ptr );
				log_file_ptr = NULL;
			}
		}
		else
			log_stop = time;
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
				deb_set = true;
				cmd( "focustop .deb" );
			}

		// ignore if log already open
		if ( log_file_ptr == NULL )
		{
			log_file_ptr = fopen( "log.txt", "a" );
			log_start = time;
			log_stop = sim.last_t;
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

		cv = sim.root->search_var( sim.root, lab );
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

		new sense( lab, & sim, param, lag, numv, & v, integer );
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
	sense *cs;

	for ( cs = sim.rsense; cs != NULL; cs = cs->next )
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


/***************************************************
SAVE_EQFILE
	Save to disk the current loaded equation file
	from configuration file
***************************************************/
void save_eqfile( FILE *f )
{
	if ( eq_txt != NULL && ( strlen( sim.conf_eq_txt ) == 0 || strcmp( sim.conf_eq_txt, eq_txt ) != 0 ) )
		strcpyn( sim.conf_eq_txt, eq_txt, MAX_FILE_SIZE );

	fprintf( f, "\nEQ_FILE\n" );
	fprintf( f, "%s", sim.conf_eq_txt );
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
			cd = sim.search_description( cv->label, false );
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
			co = sim.blueprint->search( cb->blabel );
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

	for ( cs = sim.rsense; cs != NULL; cs = cs->next )
	{
		// get current value (first object)
		cv = r->search_var( NULL, cs->label );

		// get element description
		cd = sim.search_description( cs->label, false );
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


/****************************************************
SHOW_LOGS
	Open tail/multitail to show log files dynamically
****************************************************/
void show_logs( const char *dest_path, vector < string > & logs, bool par_cntl )
{
	char exec[ MAX_PATH_LENGTH	];
	int i, j, n, sz;

	cmd( "switch [ ttk::messageBox -parent . -type yesno -default yes -icon info -title \"Background run monitor\" -message \"Open the background run monitor?\" -detail \"The selected simulation runs were started as parallel background job(s). Each job progress can be monitored in a separated window results by choosing 'Yes'\n\nLog files are being created in the folder:\n\n[ fn_break [ file nativename \"%s\" ] 40 ]\" ] { yes { set ans 1 } no { set ans 0 } }", dest_path );

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
