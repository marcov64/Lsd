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


/*****************************************************************************
DESCRIPTION (CONSTRUCTOR)
******************************************************************************/
const char *descr_types[ 5 ] = { "Variable", "Parameter", "Function", "",
								 "Object" };

description::description( const char *_label, int _type, const char *_text,
						   const char *_init, bool _initial, bool _observe )
{
	char *str;
	int i, j;

	label = new char [ strlen( _label ) + 1 ];
	strcln( label, _label, strlen( _label ) + 1 );

	if ( _type < 0 || _type > 4 )
		_type = 3;

	type = new char [ strlen( descr_types[ _type ] ) + 1 ];
	strcpy( type, descr_types[ _type ] );

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
	else
		init = NULL;

	if ( _type != 4 )
	{
		initial = _initial;
		observe = _observe;
	}
	else
		initial = observe = false;

	next = NULL;
}


/*****************************************************************************
~DESCRIPTION (DESTRUCTOR)
******************************************************************************/
description::~description( void )
{
	delete [ ] label;
	delete [ ] type;
	delete [ ] text;
	delete [ ] init;
}


/***************************************************
ADD_DESCRIPTION
***************************************************/
description *simulation::add_description( const char *lab, int type, const char *text, const char *init, bool initial, bool observe )
{
	description *cd;

	if ( search_description( lab, false ) != NULL )	// already exists?
		return change_description( lab, NULL, type, text, init, initial, observe );

	if ( descr == NULL )
		cd = descr = new description( lab, type, text, init, initial, observe );
	else
	{
		for ( cd = descr; cd->next != NULL; cd = cd->next );
		cd->next = new description( lab, type, text, init, initial, observe );
		cd = cd->next;
	}

	return cd;
}


/***************************************************
CHANGE_DESCRIPTION
***************************************************/
description *simulation::change_description( const char *lab_old, const char *lab, int type, const char *text, const char *init, int initial, int observe )
{
	bool obj = false;
	char *str, ltype[ MAX_ELEM_LENGTH ];
	int i, j;
	description *cd, *cd1;

	for ( cd = descr; cd != NULL; cd = cd->next )
	{
		if ( ! strcmp( cd->label, lab_old ) )
		{

			if ( lab == NULL && type < 0 && text == NULL && init == NULL && initial == -1 && observe == -1 )
			{
				delete [ ] cd->label;
				delete [ ] cd->type;
				delete [ ] cd->text;
				delete [ ] cd->init;

				if ( cd == descr )
					descr = cd->next;
				else
				{
					for ( cd1 = descr; cd1->next != cd; cd1 = cd1->next );
					cd1->next = cd->next;
				}

				delete cd;

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
	}

	return NULL;
}


/*****************************************************************************
EMPTY_DESCRIPTION
******************************************************************************/
void simulation::empty_description( void )
{
	description *cd, *cd1;

	for ( cd = descr; cd != NULL; cd = cd1 )
	{
		cd1 = cd->next;
		delete cd;
	}

	descr = NULL;
}


/***************************************************
RESET_DESCRIPTION
regenerate recur. the descriptions of the model as it is
***************************************************/
void object::reset_description( void )
{
	bridge *cb;
	variable *cv;

	sim->search_description( label );

	for ( cv = v; cv != NULL; cv = cv->next )
		sim->search_description( cv->label, true );

	for ( cb = b; cb != NULL; cb = cb->next )
		if ( cb->head != NULL )
			cb->head->reset_description( );
}


/***************************************************
SEARCH_DESCRIPTION
***************************************************/
description *simulation::search_description( const char *lab, bool add_missing )
{
	description *cd;
	variable *cv;

	for ( cd = descr; cd != NULL; cd = cd->next )
		if ( ! strcmp( cd->label, lab ) )
			return cd;

	if ( ! add_missing )
		return NULL;

	if ( root->search( lab ) != NULL )
		return add_description( lab );

	cv = root->search_var( NULL, lab );
	if ( cv != NULL )
		return add_description( lab, cv->param );

	return NULL;
}


/***************************************************
HAS_DESCR_TEXT
***************************************************/
bool description::has_descr_text( void )
{
	if ( text != NULL && strlen( text ) > 0 && strstr( text, LEGACY_NO_DESCR ) == NULL && ( strlen( NO_DESCR ) == 0 || strstr( text, NO_DESCR ) == NULL ) )
		return true;
	else
		return false;
}
