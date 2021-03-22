/*************************************************************

	LSD 8.0 - March 2021
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
Initialize a model by creating  one as defined
in the data file. The model, after this stage, has only one instance for each
object type and variables and parameters are simply labels.

- int object::load_param( char *file_name, int repl )
It loads from the file named as specified the data
for the object. It is made in specular way in respect of save_param.
Called from browser in INTERF.CPP immediately after load_struct.
*************************************************************/

#include "decl.h"


/****************************************************
OBJECT::SAVE_STRUCT
****************************************************/
void object::save_struct( FILE *f, char const *tab )
{
	char tab1[ 30 ];
	bridge *cb;
	object *o;
	variable *var;

	if ( up == NULL )
		fprintf( f, "\t\n" );

	strcpy( tab1, tab );
	fprintf( f, "%sLabel %s\n%s{\n", tab1, label, tab1 );
	strcat( tab1, "\t" );
	
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
	description *cur_descr;
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
			cur_descr = search_description( cv->label );
			cur_descr->initial = 'n';
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
bool object::load_param( char *file_name, int repl, FILE *f )
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
		fscanf( f, "%99s", str ); 		// skip the 'Object: '
		fscanf( f, " %99s", str ); 		// skip the 'label'  
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
		fscanf( f, "%99s ", str ); 		// skip the 'Element: '
		fscanf( f, "%99s ", str ); 		// skip the 'label'
		
		if ( f == NULL )
			return false;

		if ( fscanf( f, "%d %c %c %c %c", &( cv->num_lag ), &ch1, &ch, &( cv->debug ), &ch2   ) != 5 )
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
	int len, i = 0;
	char ch[ MAX_ELEM_LENGTH ];
	bridge *cb;
	variable *cv;

	fscanf( f, "%99s", ch );
	while ( strcmp( ch, "Label" ) && ++i < MAX_FILE_TRY )
		fscanf( f,"%99s", ch );

	if ( i >= MAX_FILE_TRY )
		return false;

	fscanf( f, "%99s", ch );
	len = strlen( ch );
	if ( label == NULL )
	{
		label = new char[ len + 1 ];
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
bool load_description( char *msg, FILE *f )
{
	int j;
	char type[ 20 ], label[ MAX_ELEM_LENGTH ], str[ 10 * MAX_LINE_SIZE ], str1[ 10 * MAX_LINE_SIZE ];
	description *app;

	label[ MAX_ELEM_LENGTH - 1 ] = '\0';
	if ( strncmp( msg, "Object", 6 ) == 0 )
	{
		strcpy(type, "Object");
		strncpy(label, msg+7, MAX_ELEM_LENGTH-1);
	} 
	else
		if (strncmp( msg, "Variable", 8) == 0 )
		{
			strcpy( type, "Variable" );
			strncpy( label, msg + 9, MAX_ELEM_LENGTH - 1 );
		} 
		else
			if ( strncmp( msg, "Parameter", 9 ) == 0 )
			{
				strcpy( type, "Parameter" );
				strncpy( label, msg + 10, MAX_ELEM_LENGTH - 1 );
			} 
			else
				if ( strncmp( msg, "Function", 6 ) == 0 )
				{
					strcpy( type, "Function" );
					strncpy( label, msg + 9, MAX_ELEM_LENGTH - 1 );
				} 
				else
					return false;
	 
	if ( descr == NULL )
		app = descr = new description;
	else  
	{
		for ( app = descr; app->next != NULL; app = app->next );
		app->next = new description;
		app = app->next;
	} 
	
	app->next = NULL;
	app->text = app->init = NULL;
	app->label = new char[ strlen( label ) + 1 ];
	strcpy( app->label, label );
	app->type = new char[ strlen( type ) + 1 ];
	strcpy( app->type, type );

	strcpy( str1, "" );
	fgets( str, MAX_LINE_SIZE, f );		// skip first newline character
	for ( j = 0 ; fgets( str, MAX_LINE_SIZE, f ) != NULL && strncmp( str, "END_DESCRIPTION", 15 ) && strncmp( str, "_INIT_", 6 ) && strlen( str1 ) < 9 *MAX_LINE_SIZE && j < MAX_FILE_TRY ; ++j )
		strcat( str1, str );

	if ( strncmp( str, "END_DESCRIPTION", 15 ) && strncmp( str, "_INIT_", 6 ) )
		return false;

	clean_newlines( str1 );

	app->text = new char[ strlen( str1 ) + 1 ];
	strcpy( app->text, str1 );

	if ( ! strncmp( str, "_INIT_", 6 ) )
	{
		strcpy( str1, "" );
		for ( j = 0 ; fgets( str, MAX_LINE_SIZE, f ) != NULL && strncmp( str, "END_DESCRIPTION", 15 ) && strlen( str1 ) < 9 * MAX_LINE_SIZE && j < MAX_FILE_TRY ; ++j )
			strcat( str1, str );

		if ( strncmp( str, "END_DESCRIPTION", 15 ) )
			return false;

		clean_newlines( str1 );
		app->init = new char[ strlen( str1 ) + 1 ];
		strcpy( app->init, str1 );
	}
	else
	{
		app->init = new char[ 1 ];
		strcpy( app->init, "" );
	}
	app->initial = 'n';
	app->observe = 'n';  

	return true;
} 


/*****************************************************************************
EMPTY_DESCR
******************************************************************************/
void empty_description( void )
{
	description *cur, *cur1;
	
	for ( cur1 = descr; cur1 != NULL; cur1 = cur )
	{
		cur = cur1->next;
		delete [ ] cur1->label;
		delete [ ] cur1->type;
		delete [ ] cur1->text;
		delete [ ] cur1->init;
		delete cur1;
	}
	descr = NULL;
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
	if ( cd == NULL )
	{
		add_description( r->label, "Object", "(no description available)" );
		plog( "\nWarning: description for '%s' not found. New one created.", "", r->label );
		cd = search_description( r->label );
	} 

	if ( cd->init == NULL )     
		fprintf( f, "%s_%s\n%s\nEND_DESCRIPTION\n\n", cd->type, cd->label, cd->text );
	else
		fprintf( f, "%s_%s\n%s\n_INIT_\n%s\nEND_DESCRIPTION\n\n", cd->type, cd->label, cd->text, cd->init );

	for ( cv = r->v; cv != NULL; cv = cv->next )
	{
		cd = search_description( cv->label );
		if ( cd == NULL )
		{
			if ( cv->param == 0 )
				add_description( cv->label, "Variable", "(no description available)" );
			if ( cv->param == 1 )
				add_description( cv->label, "Parameter", "(no description available)" );  
			if ( cv->param == 2 )
				add_description( cv->label, "Function", "(no description available)" );
			
			add_description( cv->label, "Object", "(no description available)" );
			plog( "\nWarning: description for '%s' not found. New one created.", "", cv->label );
			cd = search_description( cv->label );
		} 

		if ( cd->init == NULL )     
			fprintf( f, "%s_%s\n%s\nEND_DESCRIPTION\n\n", cd->type, cd->label, cd->text );
		else
			fprintf( f, "%s_%s\n%s\n_INIT_\n%s\nEND_DESCRIPTION\n\n", cd->type, cd->label, cd->text, cd->init );
	   
	}

	for ( cb = r->b; cb != NULL; cb = cb->next )
		if ( cb->head != NULL )
			save_description( cb->head, f );
}


/*****************************************************************************
RESET_BLUEPRINT
	reset the current blueprint
******************************************************************************/
void reset_blueprint( object *r )
{
	empty_blueprint( );
	blueprint = new object;
	blueprint->init( NULL, "Root" );
	set_blueprint( blueprint, r );
}


/*****************************************************************************
EMPTY_BLUEPRINT
	remove the current blueprint
******************************************************************************/
void empty_blueprint( void )
{
	if ( blueprint == NULL )
		return;

	blueprint->empty( );
	blueprint->delete_obj( );
	blueprint = NULL;
}


/*****************************************************************************
SET_BLUEPRINT
	copy the naked structure of the model into another object, called blueprint, 
	to be used for adding objects without example
******************************************************************************/
void set_blueprint( object *container, object *r )
{
	bridge *cb, *cb1;
	object *cur, *cur1;
	variable *cv;
	
	if ( r == NULL )
		return;

	for ( cv = r->v; cv != NULL; cv = cv->next )
		container->add_var_from_example( cv );
	
	delete [ ] container->label;
	
	container->label = new char[ strlen( r->label ) + 1 ];
	strcpy( container->label, r->label );

	for ( cb = r->b; cb != NULL; cb = cb->next )
	{
		if ( cb->head == NULL )
			continue;
		
		cur1 = cb->head;
		container->add_obj( cur1->label, 1, 0 );
		
		for ( cb1 = container->b; strcmp( cb1->blabel, cb->blabel ); cb1 = cb1->next );
		
		cur = cb1->head;
		set_blueprint( cur, cur1 );
	}
}


/*****************************************************************************
LOAD_CONFIGURATION
	Load current defined configuration
	If quick is true, just the structure and the parameters are retrieved
	Returns: 0: load ok, 1,2,3,4,...: load failure
******************************************************************************/
int load_configuration( bool reload, bool quick )
{
	int i, j = 0, load = 0;
	char msg[ MAX_LINE_SIZE ], name[ MAX_PATH_LENGTH ], full_name[ 2 * MAX_PATH_LENGTH ];
	object *cur;
	variable *cur_var, *cur_var1;
	description *cur_descr;
	FILE *f, *g;
	
	unload_configuration( false );				// unload current
	
	if ( ! reload )
	{
		delete [ ] struct_file;
		if ( strlen( path ) > 0 )
		{
			struct_file = new char[ strlen( path ) + strlen( simul_name ) + 6 ];
			sprintf( struct_file, "%s/%s.lsd", path, simul_name );
		}
		else
		{
			struct_file = new char[ strlen( simul_name ) + 6 ];
			sprintf( struct_file, "%s.lsd", simul_name );
		}
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
	
	if ( reload && quick )						// just quick reload?
		goto endLoad;
		
	sim_num = 1;
	fscanf( f, "%999s", msg );					// should be SIM_NUM 
	if ( ! ( ! strcmp( msg, "SIM_NUM" ) && fscanf( f, "%d", &sim_num ) ) )
	{
		load = 4;
		goto endLoad;
	}
	
	seed = 1;
	fscanf( f, "%999s", msg );					// should be SEED
	if ( ! ( ! strcmp( msg, "SEED" ) && fscanf( f, "%d", &seed ) ) )
	{
		load = 5;
		goto endLoad;
	}
	
	max_step = 100;
	fscanf( f, "%999s", msg );					// should be MAX_STEP
	if ( ! ( ! strcmp( msg, "MAX_STEP" ) && fscanf( f, "%d", &max_step ) ) )
	{
		load = 6;
		goto endLoad;
	}

	fscanf( f, "%999s", msg );					// should be EQUATION
	if ( strcmp( msg, "EQUATION" ) )
	{
		load = 7;
		goto endLoad;
	}
	strcpy( name, "NONE" );
	fgets( name, MAX_PATH_LENGTH - 1, f );
    if ( name[ strlen( name ) - 1 ] == '\n' )
		name[ strlen( name ) - 1 ] = '\0';
    if ( name[ strlen( name ) - 1 ] == '\r' )
		name[ strlen( name ) - 1 ] = '\0';

	// use the current equation name only if the file exists
	snprintf( full_name, 2 * MAX_PATH_LENGTH - 1, "%s/%s", exec_path, name + 1 );
	g = fopen( full_name, "r" );
	if ( g != NULL )
	{
		fclose( g );
		strncpy( equation_name, name + 1, MAX_PATH_LENGTH - 1 );
	}
	
	fscanf( f, "%999s", msg );					// should be MODELREPORT
	if ( ! ( ! strcmp( msg, "MODELREPORT" ) && fscanf( f, "%499s", name_rep ) ) )
	{
		load = 8;
		goto endLoad;
	}

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
		cur_descr = search_description( msg );
		if ( cur_descr != NULL )
		{
			cur_descr->observe = 'y';
			cur_var = root->search_var( NULL, msg );
			if ( cur_var != NULL )
				for ( cur = cur_var->up; cur != NULL; cur = cur->hyper_next( cur_var->up->label ) )
				{
					cur_var1 = cur->search_var( NULL, cur_var->label );
					if ( cur_var1 != NULL )
						cur_var1->observe = true;
				}
		}
		fscanf( f, "%999s", msg );
	}
	
	if ( j >= MAX_FILE_TRY )
	{
		load = 11;
		goto endLoad;
	} 
	
	fscanf( f, "%999s", msg );  				// should be the DOCUINITIAL
	if ( strcmp( msg, "DOCUINITIAL" ) )
	{
		load = 12;
		goto endLoad;
	}  
	
	fscanf( f, "%999s", msg );  
	for ( j = 0; strcmp( msg, "END_DOCUINITIAL" ) && j < MAX_FILE_TRY; ++j )
	{
		cur_descr = search_description( msg );
		if ( cur_descr != NULL )
			cur_descr->initial = 'y';
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

	strcpy( lsd_eq_file, "" );
	for ( j = 0; fgets( msg, MAX_LINE_SIZE - 1, f ) != NULL && strncmp( msg, "END_EQ_FILE", 11 ) && strlen( lsd_eq_file ) < MAX_FILE_SIZE - MAX_LINE_SIZE && j < MAX_FILE_TRY; ++j )
		strcat( lsd_eq_file, msg );
	
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
	
	return load;
}


/*****************************************************************************
UNLOAD_CONFIGURATION
	Unload the current configuration
	If reload is true, just the model data is unloaded
	Returns: pointer to root object
******************************************************************************/
void unload_configuration ( bool full )
{
	empty_blueprint( );						// remove current model structure
	empty_description( );
	root->delete_obj( );
	root = new object;
	root->init( NULL, "Root" );
	add_description( "Root", "Object", "(no description available)" );      
	reset_blueprint( NULL );

	empty_cemetery( );							// garbage collection
	empty_sensitivity( rsense ); 				// discard sensitivity analysis data
	
	unsavedData = false;						// no unsaved simulation results
	unsavedSense = false;						// no sensitivity data to save
	rsense = NULL;								// no sense data 
	
	actual_steps = 0;							// reset steps counter
	findexSens = 0;								// reset sensitivity serial number
	nodesSerial = 0;							// reset network node serial number
	
#ifndef NW
	currObj = NULL;								// no current object pointer
	unsaved_change( false );					// signal no unsaved change
	cmd( "destroytop .lat" );					// remove lattice window
	cmd( "unset -nocomplain modObj modElem modVar modPar modFun" );	// no elements in model structure
	
	if ( ! running )
	{
		cmd( "destroytop .plt" );				// remove run-time plot window
		cmd( "if { [ file exists temp.html ] } { file delete temp.html }" );	// delete temporary files
	}
#endif

	if ( full )									// full unload? (no new config?)
	{
		delete [ ] path;						// reset current path
		path = new char[ strlen( exec_path ) + 1 ];
		strcpy( path, exec_path );
		
		delete [ ] simul_name;					// reset simulation name to default
		simul_name = new char[ strlen( DEF_CONF_FILE ) + 1 ];
		strcpy( simul_name, DEF_CONF_FILE );
		
		delete [ ] struct_file;					// reset structure
		struct_file = new char[ strlen( simul_name ) + 5 ];
		sprintf( struct_file, "%s.lsd", simul_name );
		struct_loaded = false;

		delete sens_file;						// reset sensitivity file name
		sens_file = NULL;
		
		strcpy( lsd_eq_file, "" );				// reset other file names
		sprintf( name_rep, "report_%s.html", simul_name );

#ifndef NW
		cmd( "set path \"%s\"", path );
		cmd( "set res \"%s\"", simul_name );
		if ( strlen( path ) > 0 )
			cmd( "cd \"$path\"" );
		
		cmd( "set listfocus 1; set itemfocus 0" ); 	// point for first var in listbox
		strcpy( lastObj, "" );					// disable last object for reload
		redrawRoot = redrawStruc = true;		// force browser/structure redraw
#endif
	}
}


/*********************************
SAVE_SINGLE
*********************************/
void save_single( variable *v )
{
	int i;
	FILE *f;

#ifndef NP
	// prevent concurrent use by more than one thread
	lock_guard < mutex > lock( v->parallel_comp );
#endif	

	set_lab_tit( v );
	sprintf( msg, "%s_%s-%d_%d_seed-%d.res", v->label, v->lab_tit, v->start, v->end, seed - 1 );
	f = fopen( msg, "wt" );  		// use text mode for Windows better compatibility

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
	Returns: true: save ok, false: save failure
******************************************************************************/
bool save_configuration( int findex )
{
	bool save_ok = false;
	int delta, indexDig;
	char ch[ MAX_PATH_LENGTH ], *save_file, *bak_file = NULL;
	description *cur_descr;
	FILE *f; 
	
	delta = ( findex > 0 ) ? sim_num * ( findex - 1 ) : 0;
	indexDig = ( findex > 0 ) ? ( int ) floor( log10( findex ) + 2 ) : 0;
	
	if ( strlen( path ) > 0 )
	{
		save_file = new char[ strlen( path ) + strlen( simul_name ) + 6 + indexDig ];
		sprintf( save_file, "%s/%s", path, simul_name );
	}
	else
	{
		save_file = new char[ strlen( simul_name ) + 6 + indexDig ];
		sprintf( save_file, "%s", simul_name );
	}
	
	if ( findex > 0 )
	{
		sprintf( ch, "_%d.lsd", findex );
		strcat( save_file, ch );
	}
	else
	{
		// create backup file when not indexed saving
		bak_file = new char[ strlen( save_file ) + 5 ];
		sprintf( bak_file, "%s.bak", save_file );
		
		strcat( save_file, ".lsd" );
	
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
	
	fprintf( f, "\nSIM_NUM %d\nSEED %d\nMAX_STEP %d\nEQUATION %s\nMODELREPORT %s\n", sim_num, seed + delta, max_step, equation_name, name_rep );
	
	fprintf( f, "\nDESCRIPTION\n\n" );
	save_description( root, f );
	
	fprintf( f, "\nDOCUOBSERVE\n" );
	for ( cur_descr = descr; cur_descr != NULL; cur_descr = cur_descr->next )
		if ( cur_descr->observe == 'y' )   
			fprintf( f, "%s\n", cur_descr->label );
	fprintf( f, "\nEND_DOCUOBSERVE\n\n" );
	
	fprintf( f, "\nDOCUINITIAL\n" );
	for ( cur_descr = descr; cur_descr != NULL; cur_descr = cur_descr->next )
		if ( cur_descr->initial == 'y' )     
			fprintf( f, "%s\n", cur_descr->label );
	fprintf( f, "\nEND_DOCUINITIAL\n\n" );
	
	save_eqfile( f );
	
	if ( ! ferror( f ) )
		save_ok = true;
	
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

		cs->label = new char[ strlen( lab ) + 1 ];  // save element name
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
			fscanf( f, ": " );	 			// remove separator
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
#ifndef NW
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
