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
CHECK.H
This file contains all the checking support functions required 
by macros to allow inlining the code for max performance.
Also contains the macro error handlers.
*************************************************************/

/****************************
CHK_PTR
User pointer check
*****************************/
inline bool chk_ptr( object *ptr )
{
	extern o_setT obj_list;			// list with all existing LSD objects
	
	if ( ptr == NULL )
		return true;
	
	if ( no_ptr_chk || obj_list.find( ptr ) != obj_list.end( ) )
		return false;
		
	return true;
}


/****************************
CHK_OBJ
User pointer check for 
valid or NULL pointer
*****************************/
inline bool chk_obj( object *ptr )
{
	extern o_setT obj_list;			// list with all existing LSD objects
	
	if ( ptr == NULL || no_ptr_chk || obj_list.find( ptr ) != obj_list.end( ) )
		return false;
	
	return true;
}


/****************************
CHK_HOOK
Hook vector bound check
*****************************/
inline bool chk_hook( object *ptr, unsigned num )
{
	if ( ptr == NULL )
		return true;
	
	if ( no_ptr_chk || num < ptr->hooks.size( ) )
		return false;
	
	return true;
}


/***************************************************
CYCLE_OBJ
Support function used in CYCLEx macros
***************************************************/
inline object *cycle_obj( object *parent, char const *label, char const *command )
{
	return parent->search_err( label, no_search, "cycling" );
}


/****************************************************
BROTHER
****************************************************/
inline object *brother( object *c )
{
	if ( c == NULL || c->next == NULL )
		return NULL;
	
	return c->next;
}


/****************************
BAD_POINTER_*
Bad pointer error message
Escape function for invalid 
pointers in macros
*****************************/
double bad_ptr_dbl( object *ptr, const char *file, int line )
{
	char msg[ MAX_LINE_SIZE ];
	
	if ( ptr == NULL )
		sprintf( msg, "NULL pointer used in file '%s', line %d", file, line );
	else
		sprintf( msg, "pointer to non-existing object used\nin file '%s', line %d", file, line );
	
	error_hard( msg, "invalid pointer operation", 
				"check your equation code to ensure pointer points\nto a valid object before the operation",
				true );

	return 0.;
}

char *bad_ptr_chr( object *ptr, const char *file, int line )
{
	bad_ptr_dbl( ptr, file, line );
	return NULL;
}

netLink *bad_ptr_lnk( object *ptr, const char *file, int line )
{
	bad_ptr_dbl( ptr, file, line );
	return NULL;
}

object *bad_ptr_obj( object *ptr, const char *file, int line )
{
	bad_ptr_dbl( ptr, file, line );
	return NULL;
}

void bad_ptr_void( object *ptr, const char *file, int line )
{
	bad_ptr_dbl( ptr, file, line );
	return;
}


/****************************
NUL_LINK_*
NULL link error message
Escape function for invalid 
network link pointers in macros
*****************************/
double nul_lnk_dbl( const char *file, int line )
{
	char msg[ MAX_LINE_SIZE ];
	
	sprintf( msg, "NULL network link pointer used\nin file '%s', line %d", file, line );
	
	error_hard( msg, "invalid network link", 
				"check your equation code to ensure pointer points\nto a valid link before the operation",
				true );

	return 0.;
}

object *nul_lnk_obj( const char *file, int line )
{
	nul_lnk_dbl( file, line );
	return NULL;
}

void nul_lnk_void( const char *file, int line )
{
	nul_lnk_dbl( file, line );
	return;
}


/****************************
NO_HOOK_OBJ
Invalid hook error message
Escape function for invalid 
hook pointers in macros
*****************************/
object *no_hook_obj( object *ptr, unsigned num, const char *file, int line )
{
	extern o_setT obj_list;			// list with all existing LSD objects
	
	bool bad_index = false;
	char msg[ MAX_LINE_SIZE ];
	
	if ( ptr == NULL )
		sprintf( msg, "NULL pointer used in file '%s', line %d", file, line );
	else
		if ( obj_list.find( ptr ) == obj_list.end( ) )
			sprintf( msg, "pointer to non-existing object used\nin file '%s', line %d", file, line );
		else
			bad_index = true;
	
	if ( ! bad_index )
		error_hard( msg, "invalid pointer operation", 
					"check your equation code to ensure pointer points\nto a valid object before the operation",
					true );
	else
	{
		if ( ptr->hooks.size( ) > 0 )
			sprintf( msg, "hook number %d over maximum set (%d)\nin file '%s', line %d", num, ( int ) ptr->hooks.size( ) - 1, file, line );
		else
			sprintf( msg, "hook used but none is allocated\nin file '%s', line %d", file, line );
		
		error_hard( msg, "invalid hook index", 
					"check your equation code to ensure setting hook indexes\nto valid values (0 to n-1, n is the number of hooks)\nor use ADDHOOK to allocate the requested hook",
					true );
	}
	
	return NULL;
}


/****************************
NO_NODE_*
No network node error message
Escape function for invalid 
network object in macros 
*****************************/
double no_node_dbl( const char *lab, const char *file, int line )
{
	char msg[ MAX_LINE_SIZE ];
	
	sprintf( msg, "object '%s' has no network data structure\nin file '%s', line %d", lab, file, line );
	error_hard( msg, "invalid network object", 
				"check your equation code to add\nthe network structure before using this macro",
				true );
				
	return 0.;
}

char *no_node_chr( const char *lab, const char *file, int line )
{
	no_node_dbl( lab, file, line );
	return NULL;
}
