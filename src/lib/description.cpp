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
 DESCRIPTION.CPP
 Contains the functions to operate on model's object and
 element textual descriptions.
 *************************************************************/

#include "lib/libLSD.h"				// LSD library classes


/*************************************************************
 DESCR constructor
 *************************************************************/
lsd::descr::descr( description *_container, const char *_label, int _type, const char *_text, const char *_init, bool _initial, bool _observe )
{
	char *str;
	int i, j;

	container = _container;

	label = new char [ strlen( _label ) + 1 ];
	strcln( label, _label, strlen( _label ) + 1 );

	if ( _type < 0 || _type > 4 )
		_type = 3;

	type = new char [ strlen( desc_type_names[ _type ] ) + 1 ];
	strcpy( type, desc_type_names[ _type ] );

	if ( ! strwsp( _text ) && strstr( _text, LEGACY_NO_DESCR ) == NULL &&
		 ( strlen( NO_DESCR ) == 0 || strstr( _text, NO_DESCR ) == NULL ) )
	{
		for ( i = 0; i < DESC_KEY_NUM; ++i )
		{
			str = ( char * ) strstr( _text, desc_key_words[ i ] );
			if ( str != NULL )
				for( j = 0; j < ( int ) strlen( desc_key_words[ i ] ); ++j, ++str )
					*str = tolower( *str );
		}

		text = new char [ strlen( _text ) + 1 ];
		strcln( text, _text, strlen( _text ) + 1 );
	}
	else
	{
		text = new char[ strlen( NO_DESCR ) + 1 ];
		strcln( text, NO_DESCR, strlen( NO_DESCR ) + 1 );
	}

	if ( ! strwsp( _init ) )
	{
		str = ( char * ) strstr( _init, desc_key_words[ 1 ] );
		if ( str != NULL )
			for( j = 0; j < ( int ) strlen( desc_key_words[ 1 ] ); ++j, ++str )
				*str = tolower( *str );

		init = new char [ strlen( _init ) + 1 ];
		strcln( init, _init, strlen( _init ) + 1 );
	}

	if ( _type != 4 )
	{
		initial = _initial;
		observe = _observe;
	}
}


/*************************************************************
 ~DESCR destructor
 *************************************************************/
lsd::descr::~descr( void )
{
	if ( container != NULL )
	{
		auto d = container->elem_map.find( label );
		if ( d != container->elem_map.end( ) )
			container->elem_map.erase( d );
	}

	delete [ ] label;
	delete [ ] type;
	delete [ ] text;
	delete [ ] init;
}


/*************************************************************
 HAS_DESCR_TEXT
 *************************************************************/
bool lsd::descr::has_descr_text( void )
{
	if ( text != NULL && strlen( text ) > 0 && strstr( text, LEGACY_NO_DESCR ) == NULL && ( strlen( NO_DESCR ) == 0 || strstr( text, NO_DESCR ) == NULL ) )
		return true;
	else
		return false;
}


/*************************************************************
 DESCRIPTION constructor
 *************************************************************/
lsd::description::description( void )
{
	add_descr( "Root" );
}


/*************************************************************
 DESCRIPTION destructor
 *************************************************************/
lsd::description::~description( void )
{
	empty_description( this );
}


/*************************************************************
 EMPTY_DESCRIPTION
 *************************************************************/
void lsd::empty_description( description *d )
{
	if ( d == NULL )
		d = desc;

	if ( d == NULL )
		return;

	d->elem.clear( );
	d->elem_map.clear( );
}


/*************************************************************
 RESET_DESCR
	regenerate recur. the descriptions of the model as it is
 *************************************************************/
void lsd::description::reset_descr( object *r )
{
	search_descr( r->label, true );

	for ( auto cv = r->v; cv != NULL; cv = cv->next )
		search_descr( cv->label, true );

	for ( auto cb = r->b; cb != NULL; cb = cb->next )
		if ( cb->head != NULL )
			reset_descr( cb->head );
}


/*************************************************************
 SEARCH_DESCR
 *************************************************************/
lsd::descr *lsd::description::search_descr( const char *lab, bool add_missing )
{
	auto d = elem_map.find( lab );
	if ( d != elem_map.end( ) )
		return & ( *( d->second ) );

	if ( ! add_missing || sims[ 0 ] == NULL )
		return NULL;

	if ( sims[ 0 ]->root->search( lab ) != NULL )
		return add_descr( lab );

	auto cv = sims[ 0 ]->root->search_var( NULL, lab );
	if ( cv != NULL )
		return add_descr( lab, cv->param );

	return NULL;
}


/*************************************************************
 ADD_DESCR
 *************************************************************/
lsd::descr *lsd::description::add_descr( const char *lab, int type, const char *text, const char *init, bool initial, bool observe )
{
	if ( search_descr( lab ) != NULL )	// already exists?
		return change_descr( lab, NULL, type, text, init, initial, observe );

	elem.emplace_back( this, lab, type, text, init, initial, observe );
	elem_map.emplace( lab, --elem.end( ) );

	return & elem.back( );
}


/*************************************************************
 CHANGE_DESCR
 *************************************************************/
lsd::descr *lsd::description::change_descr( const char *lab_old, const char *lab, int type, const char *text, const char *init, int initial, int observe )
{
	bool obj = false;
	char *str, ltype[ MAX_ELEM_LENGTH ];
	int i, j;

	auto cd = search_descr( lab_old );
	if ( cd == NULL )
		return NULL;

	if ( lab == NULL && type < 0 && text == NULL && init == NULL && initial == -1 && observe == -1 )
	{
		elem.erase( elem_map[ lab_old ] );
		return NULL;
	}

	if ( lab != NULL )
	{
		delete [ ] cd->label;
		cd->label = new char [ strlen( lab ) + 1 ];
		strcln( cd->label, lab, strlen( lab ) + 1 );
	}

	if ( type >= 0 )
	{
		delete [ ] cd->type;

		switch ( type )
		{
			case 0:
				strcpy( ltype, "Variable" );
				break;
			case 1:
				strcpy( ltype, "Parameter" );
				break;
			case 2:
				strcpy( ltype, "Function" );
				break;
			case 4:
			default:
				strcpy( ltype, "Object" );
				obj = true;
		}

		cd->type = new char [ strlen( ltype ) + 1 ];
		strcpy( cd->type, ltype );
	}

	if ( text != NULL )
	{
		delete [ ] cd->text;

		if ( ! strwsp( text ) && strstr( text, LEGACY_NO_DESCR ) == NULL && ( strlen( NO_DESCR ) == 0 || strstr( text, NO_DESCR ) == NULL ) )
		{
			for ( i = 0; i < DESC_KEY_NUM; ++i )
			{
				str = ( char * ) strstr( text, desc_key_words[ i ] );
				if ( str != NULL )
					for( j = 0; j < ( int ) strlen( desc_key_words[ i ] ); ++j, ++str )
						*str = tolower( *str );
			}

			cd->text = new char [ strlen( text ) + 1 ];
			strcln( cd->text, text, strlen( text ) + 1 );
		}
		else
		{
			cd->text = new char[ strlen( NO_DESCR ) + 1 ];
			strcln( cd->text, NO_DESCR, strlen( NO_DESCR ) + 1 );
		}
	}

	if ( init != NULL )
	{
		delete [ ] cd->init;

		if ( ! strwsp( init ) )
		{
			str = ( char * ) strstr( init, desc_key_words[ 1 ] );
			if ( str != NULL )
				for( j = 0; j < ( int ) strlen( desc_key_words[ 1 ] ); ++j, ++str )
					*str = tolower( *str );

			cd->init = new char [ strlen( init ) + 1 ];
			strcln( cd->init, init, strlen( init ) + 1 );
		}
		else
			cd->init = NULL;
	}

	if ( ! obj && initial != -1 )
		cd->initial = initial;

	if ( ! obj && observe != -1 )
		cd->observe = observe;

	return cd;
}
