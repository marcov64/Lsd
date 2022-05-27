/*************************************************************

	LSD 8.0 - May 2022
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

- void object::save_param( FILE *f )
Save the numerical values for the object (one digit
for each group of object of this type) and the initial values for variables.
It save also option information, that is whether to save, plot or debug the
variables.
It calls the save_param for all the descendant type.
It is called in the browser, INTERF.CPP, immediately after save_struct, by the
root of the model.

- void object::load_struct( FILE *f )
Initialize a model by creating	one as defined
in the data file. The model, after this stage, has only one instance for each
object type and variables and parameters are simply labels.

- int object::load_param( const char *file_name, int repl )
It loads from the file named as specified the data
for the object. It is made in specular way in respect of save_param.
Called from browser in INTERF.CPP immediately after load_struct.
*************************************************************/

#include "decl.h"


/****************************************************
OBJECT::SAVE_STRUCT
****************************************************/
void object::save_struct( FILE *f, const char *tab )
{
	char tab1[ MAX_ELEM_LENGTH ];
	bridge *cb;
	object *o;
	variable *var;

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

	for ( var = v; var != NULL; var = var->next )
	{
		if ( var->param == 0 )
			fprintf( f, "%sVar: %s\n", tab1, var->label );
		if ( var->param == 1 )
			fprintf( f, "%sParam: %s\n", tab1, var->label );
		if ( var->param == 2)
			fprintf( f, "%sFunc: %s\n", tab1, var->label );
	}

	fprintf( f, "\n" );
	fprintf( f, "%s}\n\n", tab );
}


/****************************************************
OBJECT::SAVE_PARAM
****************************************************/
void object::save_param( FILE *f )
{
	int i, count = 0;
	char ch, ch1, ch2;
	bridge *cb;
	description *cd;
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
		ch = '+';
		if ( cv->param == 1 || cv->num_lag > 0 )
			for ( cur = this; cur != NULL; cur = cur->hyper_next( label ) )
			{
				cv1 = cur->search_var( NULL, cv->label );
				if ( cv1->data_loaded == '-' )
				{
					ch = '-';
					break;
				}
			}
		else
		{	// avoid marking as to initialize for elements not worth it
			cd = search_description( cv->label );
			cd->initial = 'n';
		}

		ch1 = cv->save ? 's' : 'n';
		ch1 = cv->savei ? toupper( ch1 ) : ch1;
		ch2 = cv->plot ? 'p' : 'n';
		ch2 = cv->parallel ? toupper( ch2 ) : ch2;

		if ( cv->param == 0 )
			fprintf( f, "Var: %s %d %c %c %c %c", cv->label, cv->num_lag, ch1, ch, cv->debug, ch2 );
		if ( cv->param == 1 )
			fprintf( f, "Param: %s %d %c %c %c %c", cv->label, cv->num_lag, ch1, ch, cv->debug, ch2 );
		if ( cv->param == 2 )
			fprintf( f, "Func: %s %d %c %c %c %c", cv->label, cv->num_lag, ch1, ch, cv->debug, ch2 );

		for ( cur = this; cur != NULL; cur = cur->hyper_next( label ) )
		{
			cv1 = cur->search_var( NULL, cv->label );
			if ( cv1->param == 1 )
				if ( cv1->data_loaded == '+' )
					fprintf( f, "\t%.15g", cv1->val[ 0 ] );
				else
					fprintf( f, "\t%c", '0' );
			else
				for ( i = 0; i < cv->num_lag; ++i )
					if ( cv1->data_loaded == '+' )
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
			cb->head->save_param( f );
}


/****************************************************
OBJECT::LOAD_PARAM
****************************************************/
bool object::load_param( const char *file_name, int repl, FILE *f )
{
	char str[ MAX_ELEM_LENGTH ], ch, ch1, ch2;
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

	if ( fscanf( f, " %c", &ch ) != 1 )
		return false;

	if ( ch == 'C' )
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

		if ( fscanf( f, "%d %c %c %c %c", &( cv->num_lag ), &ch1, &ch, &( cv->debug ), &ch2	  ) != 5 )
			return false;

		cv->save = ( tolower( ch1 ) == 's' ) ? true : false;
		cv->savei = ( ch1 == 'S' || ch1 == 'N' ) ? true : false;
		cv->plot = ( tolower( ch2 ) == 'p' ) ? true : false;
		cv->parallel = ( ch2 == 'P' || ch2 == 'N' ) ? true : false;

		for ( cur = this; cur != NULL; repl == 1 ? cur = cur->hyper_next( label ) : cur = NULL )
		{
			cv1 = cur->search_var( NULL, cv->label );
			cv1->val = new double[ cv->num_lag + 1 ];
			cv1->num_lag = cv->num_lag;
			cv1->save = cv->save;
			cv1->savei = cv->savei;
			cv1->plot = cv->plot;
			cv1->parallel = cv->parallel;
			cv1->param = cv->param;
			cv1->debug = cv->debug;
			cv1->data_loaded = ch;

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
				for ( cur = this; cur != NULL; repl == 1 ? cur = cur->hyper_next( label ) : cur = NULL )
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
		if ( cb->head == NULL || ! cb->head->load_param( file_name, repl, f ) )
			return false;
		num = 0;
	}

	if ( up == NULL )	// this is the root, and therefore the end of the loading
		set_blueprint( blueprint, this );

	return true;
}


/****************************************************
OBJECT::LOAD_STRUCT
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


/*****************************************************************************
LOAD_DESCRIPTION
******************************************************************************/
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
SAVE_DESCRIPTION
******************************************************************************/
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
	Load current defined configuration
	If quick is != 0, just the structure and the parameters are retrieved
	Returns: 0: load ok, 1,2,3,4,...: load failure
******************************************************************************/
int load_configuration( bool reload, int quick )
{
	int i, j = 0, load = 0;
	char msg[ MAX_LINE_SIZE ], name[ MAX_PATH_LENGTH ], full_name[ 2 * MAX_PATH_LENGTH ];
	object *cur;
	variable *cv, *cv1;
	description *cd;
	FILE *f, *g;

	unload_configuration( false );				// unload current

	if ( strlen( simul_name ) == 0 )
		return 1;

	if ( ! reload || strlen( struct_file ) == 0 )
	{
		delete [ ] struct_file;
		struct_file = new char[ strlen( path ) + strlen( simul_name ) + 6 ];
		sprintf( struct_file, "%s%s%s.lsd", path, strlen( path ) > 0 ? "/" : "", simul_name );
	}

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
	if ( ! ( ! strcmp( msg, "DATA" ) && root->load_param( struct_file, 1, f ) ) )
	{
		load = 3;
		goto endLoad;
	}

	if ( reload && quick == 2 )					// just quick reload?
		goto endLoad;

	sim_num = 1;
	fscanf( f, "%999s", msg );					// should be SIM_NUM
	if ( ! ( ! strcmp( msg, "SIM_NUM" ) && fscanf( f, "%d", & sim_num ) && sim_num > 0 ) )
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

	max_step = 100;
	when_debug = stack_info = prof_min_msecs = 0;
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


	snprintf( name_rep, MAX_PATH_LENGTH, "report_%s.html", simul_name );
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
			cd->observe = 'y';
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
			cd->initial = 'y';
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

	// remove extra \n and \r (Windows) at the end
	if ( lsd_eq_file[ strlen( lsd_eq_file ) - 1 ] == '\n' )
		lsd_eq_file[ strlen( lsd_eq_file ) - 1 ] = '\0';
	if ( lsd_eq_file[ strlen( lsd_eq_file ) - 1 ] == '\r' )
		lsd_eq_file[ strlen( lsd_eq_file ) - 1 ] = '\0';

	if ( ! ignore_eq_file && strcmp( lsd_eq_file, eq_file ) )
	{
		plog( "\nWarning: the configuration file has been previously run with different equations\nfrom those used to create the LSD model program.\nChanges may affect the simulation results. You can offload the original\nequations in a new equation file and compare differences using TkDiff in LMM\n(menu File)." );
	}

endLoad:
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


/*********************************
SAVE_SINGLE
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


/*****************************************************************************
SAVE_CONFIGURATION
	Save current defined configuration (adding tag index if appropriate)
	If quick is true, just the structure and the parameters is saved
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
				if( remove( bak_file ) )
					goto error;
			}

			if ( rename( save_file, bak_file ) )
				goto error;
		}
	}

	f = fopen( save_file, "wb" );
	if ( f == NULL )
		goto error;

	root->save_struct( f, "" );
	fprintf( f, "\nDATA\n" );
	root->save_param( f );

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
			if ( cd->observe == 'y' )
				fprintf( f, "%s\n", cd->label );
		fprintf( f, "\nEND_DOCUOBSERVE\n\n" );

		fprintf( f, "\nDOCUINITIAL\n" );
		for ( cd = descr; cd != NULL; cd = cd->next )
			if ( cd->initial == 'y' )
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

	fclose( f );

	error:

	delete [ ] save_file;
	delete [ ] bak_file;

	return save_ok;
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


/****************************************************
GET_SAVED
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
SAVE_EQFILE
***************************************************/
void save_eqfile( FILE *f )
{
	if ( eq_file != NULL && ( strlen( lsd_eq_file ) == 0 || strcmp( lsd_eq_file, eq_file ) != 0 ) )
		strcpyn( lsd_eq_file, eq_file, MAX_FILE_SIZE );

	fprintf( f, "\nEQ_FILE\n" );
	fprintf( f, "%s", lsd_eq_file );
	fprintf( f, "\nEND_EQ_FILE\n" );
}


#ifndef _NW_

/***************************************************
READ_EQ_FILENAME
***************************************************/
void read_eq_filename( char *s, int sz )
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


/***************************************************
UPLOAD_EQFILE
***************************************************/
char *upload_eqfile( void )
{
	char s[ MAX_PATH_LENGTH ], line[ MAX_LINE_SIZE ], *eq;
	int sz;
	FILE *f;

	read_eq_filename( s, MAX_PATH_LENGTH );
	if ( ( f = fopen( s, "r" ) ) == NULL )
		return NULL;

	cmd( "set res [ file size %s ]", s );
	sz = get_int( "res" ) + 1;
	eq = new char[ sz ];
	strcpy( eq, "" );

	while ( fgets( line, MAX_LINE_SIZE, f ) != NULL )
		strcatn( eq, line, sz );

	fclose( f );

	return eq;
}


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

/***************************************************
RESULT
Methods for results file saving (class result)
***************************************************/

/***************************************************
DATA
Saves data to file in the specified period
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
TITLE
Saves header to file
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
CONSTRUCTOR
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
DESTRUCTOR
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
