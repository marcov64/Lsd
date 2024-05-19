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
 ASSIMLIB.CPP
 Data assimilation code used in DLL or no-window executables.
 The remaining DA code is stored in SET_ALL.CPP.
 *************************************************************/

#include "lib/libLSD.h"				// LSD library classes


/*************************************************************
 ASSIMILATION CONSTRUCTOR
 Add or update data assimilation settings for a model element
 *************************************************************/
lsd::assimilation::assimilation( const char *lab, simulation *_sim, const char *_csv, const char *_data_col_name, const char *_t_col_name, int _data_col_num, int _t_col_num )
{
	assimilation *ca;

	sim = _sim;

	if ( lab != NULL )
	{
		label = new char [ strlen( lab ) + 1 ];
		strcpy( label, lab );
	}

	if ( _csv != NULL && strlen( _csv ) > 0 )
	{
		csv = new char [ strlen( _csv ) + 1 ];
		strcpy( csv, _csv );

		if ( _data_col_name != NULL && strlen( _data_col_name ) > 0 )
		{
			data_col_name = new char [ strlen( _data_col_name ) + 1 ];
			strcpy( data_col_name, _data_col_name );
		}
		else
			data_col_num = _data_col_num;

		if ( _t_col_name != NULL && strlen( _t_col_name ) > 0 )
		{
			t_col_name = new char [ strlen( _t_col_name ) + 1 ];
			strcpy( t_col_name, _t_col_name );
		}
		else
			t_col_num = _t_col_num;
	}

	if ( sim->assim == NULL )
		sim->assim = this;
	else
	{
		for ( ca = sim->assim; ca->next != NULL; ca = ca->next );
		ca->next = this;
	}
}


/*************************************************************
 ASSIMILATION DESTRUCTOR
 Remove data assimilation settings for a model element
 *************************************************************/
lsd::assimilation::~assimilation( void )
{
	assimilation *ca, *pa;

	delete [ ] csv;
	delete [ ] data_col_name;
	delete [ ] label;
	delete [ ] t_col_name;
	delete [ ] val;

	if ( sim->assim != NULL )
	{
		for ( ca = sim->assim, pa = NULL; ca != this && ca != NULL; pa = ca, ca = ca->next );

		if ( ca == sim->assim )
			sim->assim = next;
		else
			if ( ca == this && pa != NULL )
				pa->next = next;
	}
}


/*************************************************************
 EMPTY_ASSIMILATION
 Deallocate data assimilation settings memory
 *************************************************************/
void lsd::simulation::empty_assimilation( assimilation *ca )
{
	if ( ca == NULL )
	{
		if ( assim == NULL )
			return;

		ca = assim;
		assim = NULL;
	}

	if ( ca->next != NULL )
		empty_assimilation( ca->next );

	delete ca;				// suicide
}


/*************************************************************
 SEARCH_ASSIMILATION
 Find element in data assimilation linked list
 *************************************************************/
lsd::assimilation *lsd::simulation::search_assimilation( const char *lab )
{
	assimilation *ca;

	for ( ca = assim; ca != NULL; ca = ca->next )
		if ( ! strcmp( ca->label, lab ) )
			 break;

	return ca;
}
