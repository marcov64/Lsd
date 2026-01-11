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
 CHECK.H
 This file contains all the checking support functions required
 by macros to allow inlining the code for max performance.
 Also contains the macro error handlers.
 *************************************************************/

#ifndef LSDLIB
	#include "lib/libLSD.h"				// LSD library classes
#endif

namespace lsd
{
/*************************************************************
 _CHK_PTR_ (*)
 User pointer check
 *************************************************************/
	inline bool equation::_chk_ptr_( object *ptr )
	{
		bool obj_exists;

		if ( ptr == NULL )
			return true;

		if ( _sim_->no_ptr_chk )
			return false;

		if ( _sim_->parallel_mode ) 	// use lock (slow) only if really needed
		{
			// prevent concurrent update by more than one thread
			l_guardT lock( _sim_->obj_list_lck );
			obj_exists = _sim_->obj_list.find( ptr ) != _sim_->obj_list.end( );
		}
		else
			obj_exists = _sim_->obj_list.find( ptr ) != _sim_->obj_list.end( );

		if ( obj_exists )
			return false;

		return true;
	}


/*************************************************************
 _CHK_OBJ_ (*)
 User pointer check for valid or
 NULL pointer
 *************************************************************/
	inline bool equation::_chk_obj_( object *ptr )
	{
		bool obj_exists;

		if ( _sim_->no_ptr_chk || ptr == NULL )
			return false;

		if ( _sim_->parallel_mode ) 	// use lock (slow) only if really needed
		{
			// prevent concurrent update by more than one thread
			l_guardT lock( _sim_->obj_list_lck );
			obj_exists = _sim_->obj_list.find( ptr ) != _sim_->obj_list.end( );
		}
		else
			obj_exists = _sim_->obj_list.find( ptr ) != _sim_->obj_list.end( );

		if ( obj_exists )
			return false;

		return true;
	}


/*************************************************************
 _CHK_HOOK_ (*)
 Hook vector bound check
 *************************************************************/
	inline bool equation::_chk_hook_( object *ptr, unsigned num )
	{
		if ( ptr == NULL )
			return true;

		if ( _sim_->no_ptr_chk )
			return false;

		if ( num < ptr->hooks.size( ) )
			return false;

		return true;
	}


/*************************************************************
 _CHK_EQ_ (*)
 Get equation function pointer
 for label
 *************************************************************/
	inline eq_funcT equation::_chk_eq_( const char *lab )
	{
		auto eq_it = _eq_map_.find( lab );

		if ( eq_it != _eq_map_.end( ) )
			return eq_it->second;

		_sim_->error_hard( "equation not found",
						   "check your configuration (variable name) or\ncode (equation name) to prevent this situation\nPossible problems:\n- There is no equation for this variable\n- The equation name is different from the variable name (case matters!)",
						   false,
						   "equation not found for variable '%s'",
						   lab );
		return NULL;
	}


/*************************************************************
 CHK_RES (*)
 Check for invalid equation result
 *************************************************************/
	inline double variable::chk_res( double res )
	{
		if ( std::isfinite( res ) )
		{
			if ( attr->integer )
				res = round( res );

			if ( ! std::isnan( attr->max_val ) && res > attr->max_val )
				res = attr->max_val;
			else
				if ( ! std::isnan( attr->min_val ) && res < attr->min_val )
					res = attr->min_val;
		}
		else
			if ( attr->cont->sim->quit == 0 && ( ( ! attr->cont->sim->use_nan && std::isnan( res ) ) || std::isinf( res ) ) )
				attr->cont->sim->error_hard( "invalid equation result",
											 "check your equation code to prevent invalid math operations\nPossible problems:\n- Illegal math operation (division by zero, log of negative number etc.)\n- Use of too-large/small value in calculation\n- Use of non-initialized temporary variable in calculation",
													   true,
											 "equation for '%s' produces the invalid value '%lf' at time step %d",
											  attr->label, res, attr->cont->sim->t );

		return res;
	}


/*************************************************************
 CHK_DUMMY (*)
 Check if dummy must have master
 variable updated
 *************************************************************/
	inline double variable::chk_dummy( const char *lab )
	{
		variable *cv;

		if ( strlen( lab ) > 0 )
		{
			cv = up->search_var( up, lab, false, false, false );

			if ( cv != NULL )
			{
				attr->dummy = true;

				if ( ! cv->up->under_comput_var( lab ) )
					cv->up->cal( up, lab, 0, true );
			}
			else
				attr->cont->sim->error_hard( "updater variable not found",
													   "check updater variable name or create it in model structure",
													   false,
													   "variable '%s' is missing", lab );
		}

		return val[ 0 ];
	}


/*************************************************************
 _CYCLE_OBJ_ (*)
 Support function used in CYCLEx macros
 *************************************************************/
	inline object *equation::_cycle_obj_( object *parent, const char *label, const char *command )
	{
		object *cur = parent->search_err( label, _sim_->no_search, _sim_->no_search_up, "cycling" );

		if ( cur == NULL )   // invalid cyclable object, even if in blueprint
		{
			object *cur1 = _sim_->root->search( label );

			if ( _sim_->no_search && cur1 != NULL && cur1->up != NULL && parent->attr != cur1->up->attr )
				_sim_->error_hard( "object is not a descending object",
								   "move object in model structure, or specify a parent object",
								   false,
								   "object '%s' not directly under '%s' for cycling\n(NO_SEARCH enabled!)",
								   label, parent->attr->label );
		}

		return cur;
	}


/*************************************************************
 _BAD_PTR_*_ (*)
 Bad pointer error message
 Escape function for invalid
 pointers in macros
 *************************************************************/
	inline double equation::_bad_ptr_dbl_( object *ptr, const char *file, int line )
	{
		if ( ptr == NULL )
			_sim_->error_hard( "invalid pointer operation",
							   "check your equation code to ensure pointer points\nto a valid object before the operation",
							   true,
							   "NULL pointer used in file '%s', line %d",
							   file, line );
		else
			_sim_->error_hard( "invalid pointer operation",
							   "check your equation code to ensure pointer points\nto a valid object before the operation",
							   true,
							   "pointer to non-existing object used\nin file '%s', line %d",
							   file, line );
		return 0.;
	}

	inline char *equation::_bad_ptr_chr_( object *ptr, const char *file, int line )
	{
		_bad_ptr_dbl_( ptr, file, line );
		return NULL;
	}

	inline netlink *equation::_bad_ptr_lnk_( object *ptr, const char *file, int line )
	{
		_bad_ptr_dbl_( ptr, file, line );
		return NULL;
	}

	inline object *equation::_bad_ptr_obj_( object *ptr, const char *file, int line )
	{
		_bad_ptr_dbl_( ptr, file, line );
		return NULL;
	}

	inline void equation::_bad_ptr_void_( object *ptr, const char *file, int line )
	{
		_bad_ptr_dbl_( ptr, file, line );
		return;
	}


/*************************************************************
 _NUL_LINK_*_ (*)
 NULL link error message
 Escape function for invalid
 network link pointers in macros
 *************************************************************/
	inline double equation::_nul_lnk_dbl_( const char *file, int line )
	{
		_sim_->error_hard( "invalid network link",
						   "check your equation code to ensure pointer points\nto a valid link before the operation",
						   true,
						   "NULL network link pointer used\nin file '%s', line %d",
						   file, line );
		return 0.;
	}

	inline object *equation::_nul_lnk_obj_( const char *file, int line )
	{
		_nul_lnk_dbl_( file, line );
		return NULL;
	}

	inline void equation::_nul_lnk_void_( const char *file, int line )
	{
		_nul_lnk_dbl_( file, line );
		return;
	}


/*************************************************************
 _NO_HOOK_OBJ_ (*)
 Invalid hook error message
 Escape function for invalid
 hook pointers in macros
 *************************************************************/
	inline object *equation::_no_hook_obj_( object *ptr, unsigned num, const char *file, int line )
	{
		bool bad_index = false;
		char err_msg[ MAX_LINE_SIZE ];

		if ( ptr == NULL )
			snprintf( err_msg, MAX_LINE_SIZE, "NULL pointer used in file '%s', line %d", file, line );
		else
		{
			// prevent concurrent update by more than one thread
			l_guardT lock( _sim_->obj_list_lck );
			if ( _sim_->obj_list.find( ptr ) == _sim_->obj_list.end( ) )
				snprintf( err_msg, MAX_LINE_SIZE, "pointer to non-existing object used\nin file '%s', line %d", file, line );
			else
				bad_index = true;
		}

		if ( ! bad_index )
			_sim_->error_hard( "invalid pointer operation",
							   "check your equation code to ensure pointer points\nto a valid object before the operation",
							   true, err_msg );
		else
		{
			if ( ptr->hooks.size( ) > 0 )
				snprintf( err_msg, MAX_LINE_SIZE, "hook number %d over maximum set (%d)\nin file '%s', line %d", num, ( int ) ptr->hooks.size( ) - 1, file, line );
			else
				snprintf( err_msg, MAX_LINE_SIZE, "hook used but none is allocated\nin file '%s', line %d", file, line );

			_sim_->error_hard( "invalid hook index",
							   "check your equation code to ensure setting hook indexes\nto valid values (0 to n-1, n is the number of hooks)\nor use ADDHOOK to allocate the requested hook",
							   true, err_msg );
		}

		return NULL;
	}


/*************************************************************
 _NO_NODE_*_ (*)
 No network node error message
 Escape function for invalid
 network object in macros
 *************************************************************/
	inline double equation::_no_node_dbl_( const char *lab, const char *file, int line )
	{
		_sim_->error_hard( "invalid network object",
						   "check your equation code to add\nthe network structure before using this macro",
						   true,
						   "object '%s' has no network data structure\nin file '%s', line %d",
						   lab, file, line );
		return 0.;
	}

	inline char *equation::_no_node_chr_( const char *lab, const char *file, int line )
	{
		_no_node_dbl_( lab, file, line );
		return NULL;
	}
}
