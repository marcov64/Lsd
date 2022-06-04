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
OBJECT.CPP
It contains the core code for LSD, together with VARIAB.CPP.

A model is nothing but a link of objects, whose behavior is defined here.
Only the methods for saving a loading a model are placed in another file, FILE.CPP.

An object is composed by some fields and a set of methods. The fields are
used to identify the object type and to insert it in a model.

- char *label;
Name of the object. The name is used indicate one specific type of object in the
model. Two objects are always identical in their definition. Inheritance is not
used in LSD, yet.

- variable *v;
the first element of a linked chain of variable. They are the computational content
of the model

- bool to_compute;
flag set by default to 1. If it is zero, the system will not compute the equations
of this object as a default, but only if they are requested by other equations.
Used to speed up the simulation.

- object *b;
pointer to the object's linked-list of bridges. The bridges connect the object with
its sons. There is one bridge for each son object (even if it has many instances).
The bridge points to the head of a linked list of the son instances.

- object *up;
pointer to the parent object. Root is the only object having no parent (the
value of up is then NULL).

- object *next;
pointer to the next object in the linked chain of the descendant of the parent
of this object.

- network *node;
pointer to the data structure containing the network links from the object
(see nets.cpp for the details)

The drawing below sketches one object. All the object of the same chain
same parent, that is up. They can only provide a way to continue along the
linked chain (via next). The only way to "go back" is by starting again: go
"up", pick the bridge to the desired son object and pick the head of the
corresponding linked list and then follow all the chain again.


   object *up
		   /\
	   ||
	   ||___________
	  |				|
	  |char *label	|------> object *next
	  |variable *v	|
	  |_____________|
	   ||
	   ||-----> bridge *b -----> object *b->head ------> *b->head->next ----> ...
	   ||
	   ||-----> bridge *b->next --> object *b->next->head --> *b->next->head->next --> ...
	   ..
	   ..

This definition of object allows to define a model as a multiple dimensional
tree, where it is possible to browse the model with very limited code.

METHODS
The methods for object implemented here all refer always to the "this" object.
That is, if you consider the following as functions, then they have always as
parameter the address of one object, refer to as "this", whose fields are
addressed as if they were public variables.

Methods as listed in two groups: the ones that can be used as functions in LSD
and the ones used for management of the model. This distinction is only
because of the functionalities, since all the methods are actually public, and
could be used anyway. It is just that you wouldn't like to, say, save a model
in the middle of an equation.

METHODS FOR EQUATIONS (marked with an *)

- double cal( char *l, int lag );
Interface to another type of cal(see below), that uses also the address of this.
Provides the value of one variable whose label is lab. The value corresponds
to the gloable time t-lag, where t is the global time value when the cal is
made.
If there is only one variale l in the model, that one is found, wherever is
placed in the model.
If, instead, there are many variables l, the variable returned depends
on the position of this in the model, in respect of the position of the
objects owning l.
If l is in the same object "this", then this is returned. Otherwise, is returned
the first l found following the strategy used in search_var (see below for
a detailed description of search_var).
In general, this means to return the intuitively correct variable. The
case for errors is when the variable l is in objects not directly related
with "this" in the hierarchical structure of the model.

- variable *search_var(object *caller,char *label);
It explores the model starting from this and
gradually extending till considering the whole model.
It searches for an object having a variable whose label is l and returns the
first found.
The research strategy used by this method is simple:
1) search among the variables of this. If not found
2) search among the variables of the descending objects. If not found
3) search among the variables of parent object.
Each object encountered during a search perform the same search strategy.
The strategy ensures that the whole model is searched, hence always returns
a value, provided that variable l exists. The problem is
to be sure that, in case of multiple instances of variable l, the correct one
is returned. This depends on the right choice of "this", that is, where the
search is starting from.
The field caller is used to avoid deadlocks when
from descendants the search goes up again, or from the parent down.

- object *search_var_cond( char *lab, double value, int lag );
Uses search_var, but returns the instance of the object that has the searched
variable with the desired value equal to value.

- double overall_max( char *lab, int lag );
Searches for the object having the variable lab. From that object, it considers
the whole group of object of the same type as the one found, and searches the
maximum value of the variables lab with lag lag there contained

- double sum( const char *lab1, int lag, bool cond, const char *lab2, const char *lop, double value );
Searches for the object having the variable lab1. From that object, it considers
the whole group of object of the same type as the one found, and returns
the sum of all the variables lab with lag lag in that group. If cond is true,
only objects satisfying the logical condition 'V( "lab2" ) lop value' will be
considered form summing. lab2 should be in the same object as lab1 or be the same
as lab1.

- double whg_av( char *lab1, char *lab2, int lag, bool cond, const char *lab2, const char *lop, double value );
Same as sum, but it adds up the product between variables lab and lab2 for each
object. lab and lab2 must be in the same object.

- void lsdqsort( char *obj, char *var, char *dir, int lag );
Sorts the Objects whose label is obj according to the values of their
variable var. The direction of sorting can be UP or DOWN. The method
is just an interface for sort_asc and sort_desc below.

IMPORTANT:
The initial Object must be the first element of the set of Objects to be sorted,
and hence is must contain a Variable or Parameter labeled Var_label.
The field from must be either the Object whose "next" is this, or, in case this
is the first element of descendants from some Objects and hence it is a son,
it must be the address of the parent of this.

- void delete_obj( void ) ;
eliminate the object, keeping in order the chain list.

- void stat( char *lab, double *v, int lag, bool cond, const char *lab2, const char *lop, double value );
Reports some statistics on the values of variable named lab contained
in one group of object descending from the this. The results are stored in the
vector v, with the following order:
v[ 0 ]=number of instances;
v[ 1 ]=average
v[ 2 ]=variance
v[ 3 ]=max
v[ 4 ]=min

- void write( char *lab, double value, int time, int lag )
Assign the value value to the variable lab, resulting as if this was the
value at gloabal time time. It does not make a search looking for lab. Lab
must be a variable of this.
The function allows to override the default system to update variables in LSD
during a simulation time step.
In general, through update, a variable is computed either because the system
requests its value via the "update" method, or because, before "update", another
equation needs its updated value.
An equation can instead call "write"

For sake of completeness, here are other two functions, not members of object,
that are extensively used in equations, besides in the following code.

- object *skip_next_obj( object *t, int *count );
Counts how many types of objects equal to t are in this
group. count returns such value, and the whole function returns the next object
after the last of the series of t.

- object *go_brother( object *c );
returns: c->next, if it is of the same type of c (brother).
Returns NULL otherwise. It is safe to use even when c or c->next are NULL.

METHODS NOT USED IN THE EQUATIONS

- double cal( object *caller, char *l, int lag, int *done );
It is the basic function used in the equations for LSD variables.
It is called by the former type of method cal(l,lag ), because that
is simpler to be used in the equation code. It activates the method
search_var( caller, label )
that returns a variable whose name is label and then calls the method
cal() for that variable (see variable::cal), that returns the desired value.

- void init( object *_up, char *_label, bool _to_compute );
Initialization for an object. Assigns _up to up and creates the label

- void update( bool recurse ) ;
The recursive function computing the equations for a new time step in the
simulation. It first requests the values for its own variables. Then calls update
for all the descendants if recurse is true.

- object *hyper_next( char *lab );
Returns the next object whose name is lab. That is, it makes a search only down
and up, but does never consider objects before the one from which the search
starts from. It is used to chase objects of lab type even when they are scattered
in different groups.

- void add_empty_var( char *label );
Add a variable to the object

- void add_obj( char *label, int num );
Add a new object type in the model as descendant of current one
 and initialize its name. It makes num copies
of it. This is NOT to be used to make more instances of existing objects.

- void move_obj( char *lab );
Move the current object as descendant to a new parent

- object *search( char *lab );
Explores one branch of the model to find for an object whose label is lab.
It searches only down and next. Only for Root, the search is extensive on the
whole model.

- void chg_lab( char *lab );
Changes the name of an object type, that is for all the object of this type in
the model

- void chg_var_lab( char *old, char *n );
Only to this object, changes the label of the variable whose label is old,
and it si changed in n

- variable *add_empty_var( char *str );
Add a variable before knowing its contents, setting to a default initialization
values all the fields in the variable. It operates only on object this

- void add_var_from_example( variable *example );
Add a variable copying all the fields by the variable example.
It operates only on object this

- void empty( void ) ;
Deletes all the contents of the object, freeing its memory. Used in delete_obj
just before suicide with
delete this;

METHODS FOR NETWORK OPERATION

see nets.cpp

METHODS FOR FILE OPERATION

see file.cpp
*************************************************************/

#include "decl.h"

char *qsort_lab;
char *qsort_lab_secondary;
int qsort_lag;
object *globalcur;


/****************************************************
BRIDGE
Constructor, copy constructor and destructor
****************************************************/
bridge::bridge( const char *lab )
{
	copy = false;
	counter_updated = false;
	next = NULL;
	mn = NULL;
	head = NULL;
	search_var = NULL;
	o_map.clear( );

	blabel = new char[ strlen( lab ) + 1 ];
	strcpy( blabel, lab );
}

bridge::bridge( const bridge &b )
{
	copy = true;
	counter_updated = b.counter_updated;
	next = b.next;
	blabel = b.blabel;
	mn = b.mn;
	head = b.head;
	search_var = b.search_var;
	o_map = b.o_map;
}

bridge::~bridge( void )
{
	object *cur, *cnext;

	if ( copy )
		return;					// don't empty copy bridges

	if ( mn != NULL )			// turbo search node exists?
	{
		mn->empty( );
		delete mn;
	}

	for ( cur = head; cur != NULL; cur = cnext )
	{
		cnext = cur->next;
		cur->collect_cemetery( );
		cur->empty( );
		delete cur;
	}

	delete [ ] search_var;

	delete [ ] blabel;
}


/****************************************************
INIT
Set the basics for a newly created object
****************************************************/
void object::init( object *_up, const char *lab, bool _to_compute )
{
	up = _up;
	v = NULL;
	v_map.clear( );
	next = NULL;
	to_compute = _to_compute;
	label = new char[ strlen( lab ) + 1 ];
	strcpy( label, lab );
	b = NULL;
	b_map.clear( );
	hook = NULL;
	hooks.clear( );
	node = NULL;				// not part of a network yet
	cext = NULL;				// no C++ object extension yet
	acounter = 0;				// "fail safe" when creating labels
	lstCntUpd = 0;				// counter never updated
	del_flag = NULL;			// address of flag to signal deletion
	deleting = false;			// not being deleted
}


/****************************************************
RECREATE_MAPS
Recreate both fast look-up maps
****************************************************/
void object::recreate_maps( void )
{
	bridge *cb;
	variable *cv;

	v_map.clear( );
	b_map.clear( );

	for ( cv = v; cv != NULL; cv = cv->next )
		v_map.insert( v_pairT( cv->label, cv ) );

	for ( cb = b; cb != NULL; cb = cb->next )
		b_map.insert( b_pairT( cb->blabel, cb ) );
}


/****************************************************
UPDATE (*)
Compute the value of all the Variables in the Object, saving the values
and updating the runtime plot.

For optimization purposes the system tries to ignores descending objects
marked to be not computed. The implementation is quite baroque, but it
should be the fastest.
****************************************************/
void object::update( bool recurse, bool user )
{
	static bool deleted;
	bridge *cb, *cb1;
	object *cur, *cnext;
	variable *cv;

	deleted = false;
	del_flag = & deleted;			// register feedback channel

	for ( cv = v; ! deleted && cv != NULL && quit != 2; cv = cv->next )
	{
		if ( cv->under_computation )// don't update if under computation!
			continue;

		if ( cv->param == 0 && cv->last_update < t )
		{
#ifndef _NP_
			if ( parallel_ready && cv->parallel && ! cv->dummy )
				parallel_update( cv, this );
			else
#endif
				cv->cal( NULL, 0 );
		}

		if ( ! deleted	)
		{
			if ( cv->save || cv->savei )
				cv->data[ t - cv->start ] = cv->val[ 0 ];
#ifndef _NW_
			if ( ! user && cv->plot == 1 )
				plot_rt( cv );
#endif
		}
	}

	if ( recurse )
		for ( cb = b; ! deleted && cb != NULL && quit != 2; cb = cb1 )
		{
			cb1 = cb->next;
			if ( cb->head != NULL && cb->head->to_compute )
				for ( cur = cb->head; ! deleted && cur != NULL; cur = cnext )
				{
					cnext = cur->next;
					cur->update( true, user );
				}
		}

	if ( ! deleted )				// do only if not already deleted
		del_flag = NULL;			// unregister feedback channel
}


/****************************************************
GO_BROTHER
****************************************************/
object *go_brother( object *c )
{
	if ( c == NULL || c->next == NULL )
		return NULL;

	return c->next;
}


/****************************************************
SKIP_NEXT_OBJ
****************************************************/
object *skip_next_obj( object *tr, int *count )
{
	object *cur;

	for ( cur = tr, *count = 0; cur != NULL; cur = cur->next, *count += 1 );

	return skip_next_obj( tr );
}

object *skip_next_obj( object *tr )
{
	bridge *cb;

	if ( tr == NULL || tr->up == NULL )
		return NULL;

	cb = tr->up->search_bridge( tr->label );

	if ( cb == NULL || cb->next == NULL )
		return NULL;
	else
		return cb->next->head;
}


/****************************************************
HYPER_NEXT
Return the next Object in the model with the label
lab. The Object is searched in the whole model,
including different branches
****************************************************/
object *object::hyper_next( const char *lab )
{
	object *cur, *cur1;

	for ( cur1 = NULL, cur = next; cur != NULL; cur = skip_next_obj( cur ) )
	{
		cur1 = cur->search( lab );
		if ( cur1 != NULL )
			return cur1;
	}

	if ( up != NULL )
		cur = up->hyper_next( lab );

	return cur;
}

// search object with same name as the current object
object *object::hyper_next( void )
{
	return hyper_next( label );
}


/****************************************************
HYPER_COUNT
Return the total number of Object instances in the
model with the label lab. The Object is searched
in the whole model, including different branches
****************************************************/
int hyper_count( const char *lab )
{
	int n;
	object *cur;

	for ( n = 0, cur = root->search( lab ); cur != NULL; ++n, cur = cur->hyper_next( ) );

	return n;
}


/****************************************************
HYPER_COUNT_VAR
Return the total number of Object instances in the
model which contain the variable named lab. The Object
is searched in the whole model, including different branches
****************************************************/
int hyper_count_var( const char *lab )
{
	int n;
	object *cur;
	variable *cv;

	cv = root->search_var( root, lab, true );	// find variable to use

	if ( cv == NULL || cv->up == NULL )
		return 0;

	for ( n = 0, cur = cv->up; cur != NULL; ++n, cur = cur->hyper_next( ) );

	return n;
}


/****************************************************
SEARCH_BRIDGE
Search the bridge which contains the Object lab in this.
Uses the fast bridge look-up map.
***************************************************/
bridge *object::search_bridge( const char *lab, bool no_error )
{
	b_mapT::iterator bit;

	// find the bridge which contains the object
	if ( ( bit = b_map.find( lab ) ) != b_map.end( ) )
		return bit->second;

	if ( ! no_error )
		error_hard( "internal problem in LSD",
					"if error persists, please contact developers",
					true,
					"invalid data structure (bridge not found)" );
	return NULL;
}


/****************************************************
SEARCH (*)
Search the first Object lab in the branch of the model below this.
Uses the fast bridge look-up map.
***************************************************/
object *object::search( const char *lab, bool no_search )
{
	bridge *cb;
	object *cur;
	b_mapT::iterator bit;

	// the current object?
	if ( ! strcmp( label, lab ) )
		return this;

	// Search among the descendants of current object
	if ( ( bit = b_map.find( lab ) ) != b_map.end( ) )
		return bit->second->head;

	// stop if search is disabled
	if ( no_search )
		return NULL;

	// Search among descendants' descendants
	for ( cb = b; cb != NULL; cb = cb->next )
	{
		if ( cb->head != NULL )
			cur = cb->head->search( lab );
		else
			cur = NULL;

		if ( cur != NULL )
			return cur;
	}

	return NULL;
}


/************************************************
SEARCH_ERR
*************************************************/
object *object::search_err( const char *lab, bool no_search, const char *errmsg )
{
	object *cur;

	cur = search( lab, no_search );
	if ( cur == NULL )
	{	// check if it is not a zero-instance object
		cur = blueprint->search( lab );				// current object in blueprint
		if ( cur == NULL || ( no_search && strcmp( cur->up->label, this->label ) ) )
			error_hard( "object not found",
						"create object in model structure",
						false,
						"object '%s' is missing for %s%s", lab, errmsg, no_search && cur != NULL ? " (NO_SEARCH enabled!)" : "");

		if ( no_zero_instance )
			error_hard( "object has no instance",
						"check your equation code to ensure at least one instance\nof any object is kept or use command USE_ZERO_INSTANCE",
						true,
						"all instances of '%s' were deleted", lab );

		cur = NULL;									// do not return blueprint objects
	}

	return cur;
}


/****************************
EMPTY
turbosearch component
*****************************/
void mnode::empty( void )
{
	int i;

	if ( son != NULL )
	{
		for ( i = 0; i < 10; ++i )
			son[ i ].empty( );
		delete [ ] son;
	}
}


/****************************
EMPTYTURBO
remove all turbo search nodes
*****************************/
void object::emptyturbo( void )
{
	bridge *cb;
	object *cur;

	for ( cb = this->b; cb != NULL; cb = cb->next )
	{
		if ( cb->mn != NULL )
		{
			cb->mn->empty( );
			delete cb->mn;
			cb->mn = NULL;
		}
		for ( cur = cb->head; cur != NULL; cur = cur->next )
			cur->emptyturbo( );
	}
}


/****************************
CREATE
turbosearch component
*****************************/
void mnode::create( double level )
{
	int i;

	deflev = ( long int ) level;

	if ( level > 0 )
	{
		pntr = NULL;
		son = new mnode[ 10 ];

		for ( i = 0; i < 10 && globalcur != NULL; ++i )
			son[ i ].create( level - 1 );

		return;
	}

	son = NULL;
	pntr = globalcur;

	if ( globalcur->next != NULL )
		globalcur = globalcur->next;
}


/****************************
FETCH
turbosearch component
*****************************/
object *mnode::fetch( double *n, double level )
{
	object *cur;
	double a, b;

	if ( level <= 0 )
		level = deflev;

	--level;
	if ( level == 0 )
		cur = son[ ( int )( *n ) ].pntr;
	else
	{
		a = pow( 10, level );
		b = floor( *n / a );
		*n = *n - b * a ;
		cur = son[ ( int ) b ].fetch( n, level );
	}

	return cur;
}


/****************************
INITTURBO (*)
Generate the data structure required to use the turbosearch.
- lab must be the label of the descending object whose set is to be organized
- num is the total number of objects (if not provided or zero, it's calculated).
*****************************/
double object::initturbo( const char *lab, double tot = 0 )
{
	bridge *cb;
	object *cur;
	double lev;

	cb = search_bridge( lab, true );
	if ( cb == NULL )
	{
		error_hard( "object not found",
					"create object in model structure",
					false,
					"object '%s' is missing for turbo search", lab );
		return 0;
	}

	if ( cb->head == NULL )
	{
		error_hard( "object has no instance",
					"check your equation code to prevent this situation",
					true,
					"failure when initializing object '%s' for turbo search", lab );
		return 0;
	}

	if ( tot <= 0 )				// if size not informed, compute it
		for ( tot = 0, cur = this->search( lab ); cur != NULL; ++tot, cur = go_brother( cur ) );

#ifndef _NP_
	// prevent concurrent initialization by more than one thread
	lock_guard < mutex > lock( parallel_comp );
#endif

	if ( cb->mn != NULL )		// remove existing mnode
	{
		cb->mn->empty( );
		delete cb->mn;
	}

	globalcur = cb->head;
	lev = ( tot > 1 ) ? floor( log10( tot - 1 ) ) + 1 : 1;
	cb->mn = new mnode;
	cb->mn->create( lev );

	return tot;
}


/****************************
TURBOSEARCH (*)
Search the object lab placed in num position.
This search exploits the structure created with 'initturbo'
If tot is 0, previous set value is used
*****************************/
object *object::turbosearch( const char *lab, double tot, double num )
{
	bridge *cb;
	double val, lev;

	if ( num < 1 )
	{
		error_hard( "invalid search operation",
					"check your equation code to prevent this situation",
					true,
					"position '%.0lf' is invalid for turbo searching object '%s'", num, lab );
		return NULL;
	}

	cb = search_bridge( lab, true );
	if ( cb == NULL )
	{
		error_hard( "object not found",
					"check your equation code to prevent this situation",
					true,
					"failure when turbo searching object '%s'", lab );
		return NULL;
	}

	if ( cb->mn == NULL )
	{
		error_hard( "invalid search operation",
					"check your equation code to prevent this situation",
					true,
					"object '%s' is not initialized for turbo search", lab );
		return NULL;
	}

	val = num - 1;
	if ( tot > 1 )					// if size is informed
		lev = floor( log10( tot - 1 ) ) + 1;
	else
		lev = 0;					// if not, use default

	return( cb->mn->fetch( &val, lev ) );
}


/*******************************************
SEARCH_INST (*)
Searches the model for an object instance
pointed by 'obj' searching first among the
calling object instances and then into its
descendants, returning the instance number
or 0 if not found
********************************************/
void object::search_inst( object *obj, long *pos, long *checked )
{
	bool found;
	long i;
	bridge *cb;
	object *cur;

	// search among brothers
	for ( found = false, i = 1, cur = this; cur != NULL && *pos == 0; cur = cur->hyper_next( ), ++i )
	{
		if ( cur == obj )					// done if found
		{
			*pos = i;
			return;
		}

		if ( *checked >= 0 )				// don't stop during simulation
		{
			*checked += 1;
			if ( *checked > MAX_OBJ_CHK )	// stop if too many objects
			{
				*pos = -1;
				return;
			}
		}

		// search among descendants only if object yet not found (speed-up)
		if ( ! found )
		{
			if ( no_ptr_chk || strcmp( cur->label, obj->label ) )
			{
				for ( cb = cur->b; cb != NULL && *pos == 0; cb = cb->next )
					if ( cb->head != NULL )
						cb->head->search_inst( obj, pos, checked );
			}
			else
				found = true;
		}
	}
}

double object::search_inst( object *obj, bool fun )
{
	long pos, checked;
	object *cur;

	if ( obj == NULL )					// default is self
		obj = this;

	// if pointer check available quickly check for non-existing objects
	if ( obj != this && ! no_ptr_chk )
	{
		if ( obj_list.find( obj ) == obj_list.end( ) )
			return 0;

		cur = obj;
	}
	else
		cur = this;

	if ( cur->up != NULL )				// not root?
		// get first instance of found/current object brotherhood
		cur = cur->up->search_bridge( cur->label )->head;

	pos = 0;
	checked = fun ? -1 : 0;
	if ( cur != NULL )
		cur->search_inst( obj, &pos, &checked );// check for instance recursively

	return pos;
}


/************************************************
SEARCH_VAR
Explore the model starting from this and
gradually extending till considering the whole model.
It searches for an object having a variable whose label is l and returns the
first found.
The research strategy used by this method is simple:
1) search among the variables of this. If not found:
2) search among the variables of the descending objects. If not found:
3) search among the variables of parent object. If not found return NULL

Each object encountered during a search perform the same search strategy.
The strategy ensures that the whole model is searched, hence always returns
a value, provided that variable l exists. The problem is
to be sure that, in case of multiple instances of variable l, the correct one
is returned. This depends on the right choice of "this", that is, where the
search is starting from.
The field caller is used to avoid deadlocks when
from descendants the search goes up again, or from the parent down.
Uses the fast variable look-up map of the searched variables.
*************************************************/
variable *object::search_var( object *caller, const char *lab, bool no_error,
							  bool no_search, bool search_sons )
{
	bridge *cb;
	variable *cv;
	v_mapT::iterator vit;

	// Search among the variables of current object
	if ( ( vit = v_map.find( lab ) ) != v_map.end( ) )
		return vit->second;

	// stop if search is disabled except if direct sons must still be searched
	if ( no_search && ! search_sons )
		return NULL;

	// Search among descendants
	for ( cb = b, cv = NULL; cb != NULL; cb = cb->next )
	{
		// search down only if one instance exists and the label is different from caller
		if ( cb->head != NULL && ( caller == NULL || strcmp( cb->head->label, caller->label ) ) )
		{
			cv = cb->head->search_var( this, lab, no_error, no_search, false );
			if ( cv != NULL )
				return cv;
		}
		else
			cv = NULL;
	}

	// stop if search is disabled
	if ( no_search )
		return NULL;

	// Search up in the tree
	if ( up != caller )
	{
		if ( up == NULL )
		{
			if ( ! no_error )
				error_hard( "variable or parameter not found",
							"create variable or parameter in model structure",
							false,
							"element '%s' is missing", lab );
			return NULL;
		}

		cv = up->search_var( this, lab, no_error, false, false );
	}

	return cv;
}


/************************************************
SEARCH_VAR_ERR
*************************************************/
variable *object::search_var_err( object *caller, const char *lab, bool no_search,
								  bool search_sons, const char *errmsg )
{
	object *cur;
	variable *cv;

	cv = search_var( caller, lab, true, no_search, search_sons );
	if ( cv == NULL && label != NULL )
	{	// check if it is not a zero-instance object
		cur = blueprint->search( label );				// current object in blueprint
		if ( cur == NULL || cur->search_var( NULL, lab, true, no_search, search_sons ) == NULL )
			error_hard( "variable or parameter not found",
						"create variable or parameter in model structure",
						false,
						"element '%s' is missing for %s", lab, errmsg );

		if ( no_zero_instance )
			error_hard( "last object instance deleted",
						"check your equation code to ensure at least one instance\nof any object is kept or use command USE_ZERO_INSTANCE",
						true,
						"all instances of the object containing '%s' were deleted", lab );
	}

	return cv;
}


/****************************************************
SEARCH_VAR_COND (*)
Search for the Variable or Parameter lab with value value and return it, if found.
Normally searches all branches of the object containing the variable, except
if the NO_SEARCH command is issued before the macro, when it only search the
current branch of the model.
Return NULL if not found.
****************************************************/
object *object::search_var_cond( const char *lab, double value, int lag )
{
	double res;
	object *cur, *cnext;
	variable *cv;

	cv = search_var_err( this, lab, no_search, true, "conditional searching" );
	if ( cv == NULL )
		return NULL;

	for ( cur = cv->up; cur != NULL; cur = cnext )
	{
		cnext = no_search ? cur->next : cur->hyper_next( );	// allow object suicide

		res = cur->cal( lab, lag );
		if ( res == value )
			return cur;
	}

	return NULL;
}


/****************************
INITTURBO_COND (*)
Generate the data structure required to use the turbosearch with condition.
*****************************/
double object::initturbo_cond( const char *lab )
{
	bridge *cb;
	object *cur, *cnext;
	variable *cv;
	b_mapT::iterator bit;

	cv = search_var_err( this, lab, no_search, true, "turbo conditional searching" );
	if ( cv == NULL )
		return 0;

	if ( cv->up->up == NULL )				// variable at root level?
	{
		error_hard( "invalid variable or parameter for turbo search",
					"check your model structure to prevent this situation",
					false,
					"element '%s' is at root level (always single-instanced)", lab );
		return 0;
	}

	// find the bridge which contains the object containing the variable
	if ( ( bit = cv->up->up->b_map.find( cv->up->label ) ) == cv->up->up->b_map.end( ) )
	{
		error_hard( "internal problem in LSD",
					"if error persists, please contact developers",
					true,
					"invalid data structure (bridge not found)" );
		return 0;
	}

#ifndef _NP_
	// prevent concurrent initialization by more than one thread
	lock_guard < mutex > lock( parallel_comp );
#endif

	cb = bit->second;
	cb->o_map.clear( );						// remove any existing mapping

	// fill the map with the object values
	for ( cur = cb->head; cur != NULL; cur = cnext )
	{
		cnext = cur->next;					// allow object suicide
		cb->o_map.insert( o_pairT ( cur->cal( lab, 0 ), cur ) );
	}

	// register the name of variable for which the map is set
	cb->search_var = new char [ strlen( lab ) + 1 ];
	strcpy( cb->search_var, lab );

	return cb->o_map.size( );
}


/****************************
TURBOSEARCH_COND (*)
Search the object instance containing a variable label with given value.
Return the containing object instance, if found, or NULL if not found.
This search exploits the structure created with 'initturbo_cond'.
*****************************/
object *object::turbosearch_cond( const char *lab, double value )
{
	bridge *cb;
	variable *cv;
	b_mapT::iterator bit;
	o_mapT::iterator oit;

	cv = search_var_err( this, lab, no_search, true, "turbo conditional searching" );
	if ( cv == NULL )
		return NULL;

	if ( cv->up->up == NULL )				// variable at root level?
	{
		error_hard( "invalid variable or parameter for turbo search",
					"check your model structure to prevent this situation",
					false,
					"element '%s' is at root level (always single-instanced)", lab );
		return NULL;
	}

	// find the bridge which contains the object containing the variable
	if ( ( bit = cv->up->up->b_map.find( cv->up->label ) ) == cv->up->up->b_map.end( ) )
	{
		error_hard( "internal problem in LSD",
					"if error persists, please contact developers",
					true,
					"invalid data structure (bridge not found)" );
		return NULL;
	}

	cb = bit->second;

	if ( cb->o_map.size( ) == 0 || cb->search_var == NULL || strcmp( cb->search_var, lab ) )
	{
		error_hard( "invalid search operation",
					"check your equation code to prevent this situation",
					true,
					"element '%s' is not initialized for turbo conditional search", lab );
		return NULL;
	}

	// find the object containing the variable
	if ( ( oit = cb->o_map.find( value ) ) != cb->o_map.end( ) )
		return oit->second;
	else
		return NULL;
}


/****************************************************
ADD_EMPTY_VAR
Add a new (empty) Variable, used in the creation of the model structure
****************************************************/
variable *object::add_empty_var( const char *lab )
{
	variable *cv;

	if ( search_var( this, lab, true, true, false ) != NULL )
	{
		error_hard( "variable or parameter not added",
					"choose an unique name for the element",
					true,
					"element '%s' already exists in object '%s'", lab, label );
		return NULL;
	}

#ifndef _NW_
	if ( ! valid_label( lab ) )
	{
		plog( "\nWarning: invalid variable name '%s', please rename", lab );
		cmd( "ttk::messageBox -parent . -title Warning -icon warning -type ok -message \"Invalid characters in variable name\" -detail \"Variable '%s' has an invalid name. Please rename it to prevent problems.\n\nNames must begin with a letter (English alphabet) or underscore ('_') and may contain letters, numbers or '_' but no spaces or other characters.\"", lab );
	}
#endif

	if ( v == NULL )
		cv = v = new variable;
	else
	{
		for ( cv = v; cv->next != NULL; cv = cv->next );
		cv->next = new variable;
		cv = cv->next;
	}

	cv->init( this, lab, -1, NULL, 0 );
	v_map.insert( v_pairT ( lab, cv ) );

	return cv;
}


/****************************************************
ADD_VAR_FROM_EXAMPLE
Add a Variable identical to the example.
****************************************************/
void object::add_var_from_example( variable *example )
{
	variable *cv;

	if ( search_var( this, example->label, true, true, false ) != NULL )
	{
		error_hard( "variable or parameter not added",
					"choose an unique name for the element",
					true,
					"element '%s' already exists in object '%s'", example->label, label );
		return;
	}

	if ( v == NULL )
		cv = v = new variable;
	else
	{
		for ( cv = v; cv->next != NULL; cv = cv->next );
		cv->next = new variable;
		cv = cv->next;
	}

	cv->init( this, example->label, example->num_lag, example->val, example->save );
	cv->savei = example->savei;
	cv->last_update = example->last_update;
	cv->delay = example->delay;
	cv->delay_range = example->delay_range;
	cv->period = example->period;
	cv->period_range = example->period_range;
	cv->plot = ( ! running ) ? example->plot : false;
	cv->parallel = example->parallel;
	cv->observe = example->observe;
	cv->param = example->param;
	cv->deb_cond = example->deb_cond;
	cv->debug = example->debug;
	cv->deb_cnd_val = example->deb_cnd_val;
	cv->data_loaded = example->data_loaded;

	v_map.insert( v_pairT ( example->label, cv ) );
}


/****************************************************
ADD_OBJ
Add num sons with label lab to ANY object like this one, wherever is on the
tree
****************************************************/
void object::add_obj( const char *lab, int num, int propagate )
{
	int i;
	bridge *cb;
	object *cur, *cur1;

#ifndef _NW_
	if ( ! valid_label( lab ) )
	{
		plog( "\nWarning: invalid object name '%s', please rename", lab );
		cmd( "ttk::messageBox -parent . -title Warning -icon warning -type ok -message \"Invalid characters in object name\" -detail \"Object '%s' has an invalid name. Please rename it to prevent problems.\n\nNames must begin with a letter (English alphabet) or underscore ('_') and may contain letters, numbers or '_' but no spaces or other characters.\"", lab );
	}
#endif

	for ( cur = this; cur != NULL; propagate == 1 ? cur = cur->hyper_next( label ) : cur = NULL )
	{
		// create bridge
		if ( cur->b == NULL )
			cb = cur->b = new bridge( lab );
		else
		{
			for ( cb = cur->b; cb->next != NULL; cb = cb->next );
			cb->next = new bridge( lab );
			cb = cb->next;
		}

		// create object instances
		for ( i = 0; i < num; ++i )
		{
			if ( i == 0 )
				cur1 = cb->head = new object;
			else
				cur1 = cur1->next = new object;

			cur1->init( cur, lab );
		}

		cur->b_map.insert( b_pairT ( lab, cb ) );
	}
}


/****************************************************
MOVE_OBJ
Move object in the model structure. The lab object
is placed below the provided dest object
****************************************************/
void move_obj( const char *lab, const char *dest )
{
	bridge *cb, *cb1, *mb = NULL, *nb;
	object *cur, *cur1, *d, *no, *o, *s;
	variable *cv;

	o = root->search( lab );		// pick first model instances
	d = root->search( dest );

	if ( o == NULL || d == NULL || o->search( dest ) != NULL )
	{
		error_hard( "missing/invalid source or destination object",
					"choose valid object names\nand non-nested destination",
					true,
					"cannot move object '%s'", lab );
		return;
	}

	// move bridges from source parent instances to destination parent instances
	s = o->up;
	while ( s != NULL || d != NULL )
	{
		if ( s != NULL )
		{
			// find bridge to object being copied in source parent
			for ( cb1 = NULL, cb = s->b; cb != NULL && strcmp( cb->blabel, lab ) != 0; cb1 = cb, cb = cb->next );

			// remove from the source parent's bridge linked list
			if ( cb1 == NULL )	// head of list?
				s->b = cb->next;
			else
				cb1->next = cb->next;

			mb = cb;
			mb->next = NULL;	// moved object bridge enters at the end of the new parent list

			s->b_map.erase( lab );	// update speedup maps
			s = s->hyper_next( );	// next source parent
		}
		else	// handle the case last object instance has to be cloned to fill
		{		// additional instances of destination parent
			// clone last object bridge to insert it on unmatched destination parents
			nb = new bridge( lab );

			// clone object instances and add them to the cloned bridge
			for ( cur = mb->head, cur1 = NULL; cur != NULL; cur = cur->next )
			{
				no = new object;

				// update linked list of object instances in bridge
				if ( cur1 == NULL )
					nb->head = no;
				else
					cur1->next = no;

				cur1 = no;

				// clone object instance, variables and descending objects
				cur1->init( d, lab, cur->to_compute );

				for ( cv = cur->v; cv != NULL; cv = cv->next )
					cur1->add_var_from_example( cv );

				copy_descendant( cur, cur1 );
			}

			mb = nb;	// bridge clone to move
		}

		if ( d != NULL )
		{
			// find tail of bridge list in destination parent
			for ( cb1 = NULL, cb = d->b; cb != NULL; cb1 = cb, cb = cb->next );

			// add moved object to the end of destination's bridge list
			if ( cb1 == NULL )	// head of list?
				d->b = mb;
			else
				cb1->next = mb;

			// adjust instances of moved object to the new parent
			for ( cur = mb->head; cur != NULL; cur = cur->next )
				cur->up = d;

			d->b_map.insert( b_pairT ( lab, mb ) );	// update speedup maps
			d = d->hyper_next( );					// next destination parent
		}
		else	// handle the case last object instances in source parent must be
				// deleted because there are no more instances in destination
			delete mb;		// delete unmatched object instances and the bridge
	}
}


/****************************************************
REPLICATE
****************************************************/
void object::replicate( int num, bool propagate )
{
	object *cur, *cur1;
	variable *cv;
	int i, usl;

	if ( propagate )
		cur = hyper_next( label );
	else
		cur = NULL;

	if ( cur != NULL )
		cur->replicate( num, true );

	skip_next_obj( this, &usl );
	for ( cur = this, i = 1; i < usl; cur = cur->next, ++i );

	for ( i = usl; i < num; ++i )
	{
		cur1 = cur->next;
		cur->next = new object;
		cur->next->init( up, label, to_compute );
		cur->next->next = cur1;
		cur->to_compute = to_compute;

		cur1 = cur->next;
		for ( cv = v; cv != NULL; cv = cv->next )
			cur1->add_var_from_example( cv );

		copy_descendant( this, cur1 );
	}
}


/****************************************************
COPY_DESCENDANT
****************************************************/
void copy_descendant( object *from, object *to )
{
	bridge *cb, *cb1;
	object *cur;
	variable *cv;

	if ( from->b == NULL )
	{
		to->b = NULL;
		return;
	}

	// create the first bridge
	to->b = new bridge( from->b->blabel );

	// add bridge to new object lookup map
	to->b_map.insert( b_pairT ( to->b->blabel, to->b ) );

	// create the first (head) object
	if ( from->b->head == NULL )
		cur = blueprint->search( from->b->blabel );
	else
		cur = from->b->head;

	to->b->head = new object;
	to->b->head->init( to, cur->label, cur->to_compute );

	// copy variables of head object
	for ( cv = cur->v; cv != NULL; cv = cv->next )
		to->b->head->add_var_from_example( cv );

	// copy head descendants
	copy_descendant( cur, to->b->head );

	// create following bridges
	for ( cb = to->b, cb1 = from->b->next; cb1 != NULL; cb1 = cb1->next )
	{
		cb->next = new bridge( cb1->blabel );
		cb = cb->next;
		to->b_map.insert( b_pairT ( cb1->blabel, cb ) );

		if ( cb1->head == NULL )
			cur = blueprint->search( cb1->blabel );
		else
			cur = cb1->head;

		cb->head = new object;
		cb->head->init( to, cur->label, cur->to_compute );

		for ( cv = cur->v; cv != NULL; cv = cv->next )
			cb->head->add_var_from_example( cv );

		copy_descendant( cur, cb->head );
	}
}


/****************************************************
ADD_N_OBJECTS2 (*)
As the type with the example, but the example is taken from the blueprint
In respect of the original version, it allows for the specification
of the time of last update if t_update is positive or zero. If
t_update is negative (<0) it takes the time of last update from
the example object (if >0) or current t (if =0)
****************************************************/
object *object::add_n_objects2( const char *lab, int n, int t_update )
{
	return add_n_objects2( lab, n, blueprint->search( lab ), t_update );
}


/****************************************************
ADD_N_OBJECTS2 (*)
Add N objects to the model making a copies of the example object ex
In respect of the original version, it allows for the specification
of the time of last update if t_update is positive or zero. If
t_update is negative (<0) it takes the time of last update from
the example object (if >0) or current t (if =0)
****************************************************/
object *object::add_n_objects2( const char *lab, int n, object *ex, int t_update )
{
	bool net;
	int i;
	bridge *cb, *cb1, *cb2;
	object *cur, *cur1, *last, *first = NULL;
	variable *cv;

	// check the labels and prepare the bridge to attach to
	for ( cb2 = b; cb2 != NULL && strcmp( cb2->blabel, lab ); cb2 = cb2->next );

	if ( cb2 == NULL )
	{
		error_hard( "object not found",
					"create son object in model structure",
					false,
					"object '%s' contains no son object '%s' for adding instance(s)", label, lab );
		return NULL;
	}

	if ( ex == NULL || strcmp( ex->label, lab ) )
	{
		error_hard( "invalid example object",
					"check your equation code to prevent this situation",
					true,
					"bad example pointer when adding object '%s'", lab );
		return NULL;
	}

#ifndef _NP_
	// prevent concurrent additions by more than one thread
	lock_guard < mutex > lock( parallel_comp );
#endif

	cb2->counter_updated = false;

	// check if the objects are nodes in a network (avoid using EX from blueprint)
	cur = search( lab );
	if ( cur != NULL && cur->node != NULL )
		net = true;
	else
		net = false;

	last = NULL;	// pointer of the object to link to, signaling also the special first case
	for ( i = 0; i < n; ++i )
	{
		// create a new copy of the object
		cur = new object;
		cur->init( this, lab );

		if ( net )						// if objects are nodes in a network
			cur->node = new netNode( );	// insert new nodes in network (as isolated nodes)

		// create its variables and initialize them
		for ( cv = ex->v; cv != NULL; cv = cv->next )
		  cur->add_var_from_example( cv );

		for ( cv = cur->v; cv != NULL; cv = cv->next )
		{
#ifndef _NP_
			// prevent concurrent use by more than one thread
			rec_lguardT lock( cv->parallel_comp );
#endif
			if ( running && cv->param != 1 )
			{
				if ( t_update < 0 && cv->last_update == 0 )
					cv->last_update = t;
				else
				{
					if ( t_update >= 0 && t_update < cv->last_update && t > 1 )
					{
						error_hard( "cannot add object",
									"check your equation code to prevent this situation",
									true,
									"invalid update case (%d) to set object '%s'\nvariable '%s' was updated later (%d)", t_update, lab, cv->label, cv->last_update );
						return NULL;
					}

					if ( t_update >= 0 )
						cv->last_update = t_update;
				}

				// choose next update step for special updating variables
				if ( cv->delay > 0 || cv->delay_range > 0 )
				{
					cv->next_update = cv->last_update + cv->delay;
					if ( cv->delay_range > 0 )
						cv->next_update += rnd_int( 0, cv->delay_range );
				}
			}

			if ( cv->save || cv->savei )
				alloc_save_var( cv );
		}

		// insert the descending objects in the newly created objects
		for ( cb1 = NULL, cb = ex->b; cb != NULL; cb = cb->next )
		{
			if ( cb1 == NULL )
				cb1 = cur->b = new bridge( cb->blabel );
			else
				cb1 = cb1->next = new bridge( cb->blabel );

			// add bridge to new object lookup map
			cur->b_map.insert( b_pairT ( cb->blabel, cb1 ) );

			for ( cur1 = cb->head; cur1 != NULL; cur1 = cur1->next )
				cur->add_n_objects2( cur1->label, 1, cur1, t_update );
		}

		// attach the new objects to the linked chain of the bridge
		if ( last == NULL )
		{	// this is the first object created
			first = cur;
			if ( cb2->head == NULL )
				cb2->head = cur;
			else
			{
				for ( cur1 = cb2->head; cur1->next != NULL; cur1 = cur1->next );
				cur1->next = cur;
			}
		}
		else
			last->next = cur;

		last = cur;

		// update object list for user pointer checking
		if ( ! no_ptr_chk )
		{
#ifndef _NP_
			// prevent concurrent update by more than one thread
			lock_guard < mutex > lock( lock_obj_list );
#endif
			obj_list.insert( cur );
		}
	}

	return first;
}


/****************************
DELETE_BRIDGE
Remove a bridge, used when an object is removed from the model.
*****************************/
void delete_bridge( object *d )
{
	bridge *cb, *cb1;

	if ( d->up->b == NULL )
		return;

	if ( d->up->b->head == d )
	{	// first bridge in the bridge chain
		cb = d->up->b;
		d->up->b = d->up->b->next;
		d->up->b_map.erase( cb->blabel );
		delete cb;
	}
	else
	{	// find position in bridge chain (not first)
		for ( cb = d->up->b, cb1 = NULL; cb != NULL; cb1 = cb, cb = cb->next )
			if ( cb->head == d && cb1 != NULL )
			{
				cb1->next = cb->next;			// previous bridge points to next
				d->up->b_map.erase( cb->blabel );
				delete cb;
				break;
			}
	}
}


/****************************************************
DELETE_OBJ (*)
Remove the object from the model
Before killing the Variables data to be saved are stored
in the "cemetery", a linked chain storing data to be analyzed.
****************************************************/
void object::delete_obj( variable *caller )
{
	object *cur = this;
	bridge *cb;

	if ( cur == NULL )
		return;					// ignore deleting null object

	{							// create context for lock
#ifndef _NP_
		// prevent concurrent deletion by more than one thread
		lock_guard < mutex > lock( parallel_comp );
#endif

		if ( deleting )			// ignore if deleting already going on
			return;

		if ( under_computation( ) )
		{
			if ( wait_delete != NULL && wait_delete != this )
			{
				error_hard( "deletion already pending",
							"check your equation code to prevent deleting objects recursively",
							true,
							"cannot schedule the deletion of object '%s'", label );
				return;
			}
			else
			{
				wait_delete = this;
				return;
			}
		}

		deleting = true;		// signal deletion to other threads

		if ( wait_delete == this )
			wait_delete = NULL;	// finally deleting pending object
	}

	// update object list for user pointer checking
	if ( ! no_ptr_chk )
	{
#ifndef _NP_
		// prevent concurrent update by more than one thread
		lock_guard < mutex > lock( lock_obj_list );
#endif
		obj_list.erase( this );
	}

	// collect required variables BEFORE removing instances (bridge)
	collect_cemetery( caller );

	// find the bridge
	if ( up != NULL )
		cb = up->search_bridge( label );
	else
		cb = NULL;

	if ( cb != NULL )
	{
		if ( cb->head == this )
		{
			if ( next != NULL )
				cb->head = next;
			else
			{
				if ( no_zero_instance )
				{
					error_hard( "last object instance deleted",
								"check your equation code to ensure at least one instance\nof any object is kept",
								true,
								"cannot delete all instances of '%s'", label );
					return;
				}

				cb->head = NULL;
				save_ok = false;		// model structure can no longer be saved
			}
		}
		else
		{
			for ( cur = cb->head; cur->next != this; cur = cur->next );
			cur->next = next;
		}

		cb->counter_updated = false;

		if ( cb->search_var != NULL )						// indexed objects?
			cb->o_map.erase( cal( cb->search_var, 0 ) );	// try to remove map entry
	}

	if ( del_flag != NULL )
		*del_flag = true;		// flag deletion to caller, if requested

	empty( );					// empty object but don't delete it

	delete this;				// delete (suicide) now
}


/****************************************************
EMPTY
Garbage collection for objects
Delete the entire son tree below
****************************************************/
void object::empty( void )
{
	bridge *cb, *cb1;
	variable *cv, *cv1;

	// remove variables if cemetery collection was not called before
	for ( cv = v; cv != NULL; cv = cv1 )
	{
		cv1 = cv->next;
		cv->empty( );
		delete cv;
	}

	v = NULL;
	v_map.clear( );

	for ( cb = b; cb != NULL; cb = cb1 )	// delete son bridges
	{
		cb1 = cb->next;
		delete cb;				// bridge destructor delete the rest
	}

	b = NULL;
	b_map.clear( );

	if ( node != NULL )			// network data to delete?
	{
		delete node;
		node = NULL;
	}

	delete [ ] label;
	label = NULL;
}


/***************************************************
COLLECT_CEMETERY
Processes variables from an object required to go to cemetery
Also destroy variables not requiring saving
***************************************************/
void object::collect_cemetery( variable *caller )
{
	variable *cv, *cv1;

	for ( cv = v; cv != NULL; cv = cv1 )	// scan all variables
	{
		cv1 = cv->next;						// pointer to next variable

		// need to save?
		if ( ( cv->save == true || cv->savei == true ) && running && actual_steps > 0 )
		{
			if ( cv->savei )
				save_single( cv );			// update file

			set_lab_tit( cv );				// update last lab_tit

			cv->end = t;					// define last period,
			cv->data[ t - cv->start ] = cv->val[ 0 ];	// and last value

			// use C stdlib to be able to deallocate memory for deleted objects
			cv->data = ( double * ) realloc( cv->data, ( t - cv->start + 1 ) * sizeof( double ) );

			add_cemetery( cv );				// transfer to cemetery
		}
		else
		{
			cv->empty( caller == NULL || cv == caller );// disable lock if emptying caller
			delete cv;
		}
	}

	v = NULL;
	v_map.clear( );
}


/***************************************************
ADD_CEMETERY
Store the variable in a list of variables in objects deleted
but to be used for analysis.
***************************************************/
void add_cemetery( variable *v )
{
	if ( cemetery == NULL )
		cemetery = last_cemetery = v;
	else
	{
		last_cemetery->next = v;
		last_cemetery = v;
	}

	last_cemetery->next = NULL;
}


/***************************************************
EMPTY_CEMETERY
***************************************************/
void empty_cemetery( void )
{
	variable *cv, *cv1;

	for ( cv = cemetery; cv !=NULL ; )
	{
		cv1 = cv->next;
		cv->empty( );
		delete cv;
		cv = cv1;
	}

	cemetery = last_cemetery = NULL;
}


/****************************************************
TO_DELETE (*)
Check if the object is scheduled for deletion
Objects are only deleted when all variables
under computation in it finish computation
****************************************************/
double object::to_delete( void )
{
	return wait_delete == this;
}


/****************************************************
DELETE_VAR
Remove the variable from the object
****************************************************/
void object::delete_var( const char *lab )
{
	variable *cv, *cv1;

	if ( ! strcmp( v->label, lab ) )
	{	// first variable in the chain
		v_map.erase( lab );
		cv = v->next;
		v->empty( );
		delete v;
		v = cv;
	}
	else		// not first variable, search
		for ( cv = v; cv->next != NULL; cv = cv->next)
			if ( ! strcmp( cv->next->label, lab ) )
			{
				v_map.erase( lab );
				cv1 = cv->next->next;
				cv->next->empty( );
				delete cv->next;
				cv->next = cv1;
				break;
			}
}


/****************************************************
CHG_LAB
Change the label of the Object, for all the instances
****************************************************/
void object::chg_lab( const char *lab )
{
	object *cur;
	bridge *cb;

	// change all groups of this objects
	cur = up->hyper_next( up->label );
	if ( cur != NULL )
	{
		cb = cur->search_bridge( label );

		if ( cb->head != NULL )
			cb->head->chg_lab( lab );
	}

	cb = up->search_bridge( label );

	up->b_map.erase( cb->blabel );
	delete [ ] cb->blabel;
	cb->blabel = new char[ strlen( lab ) + 1 ];
	strcpy( cb->blabel, lab );
	up->b_map.insert( b_pairT ( lab, cb ) );

	for ( cur = this; cur != NULL; cur = cur->next )
	{
		delete [ ] cur->label;
		cur->label = new char[ strlen( lab ) + 1 ];
		strcpy( cur->label, lab );
	}
}


/****************************************************
CHG_VAR_LAB
Change the label of the Variable from old to new
****************************************************/
void object::chg_var_lab( const char *old, const char *newname )
{
	variable *cv;

	for ( cv = v; cv != NULL; cv = cv->next)
		if ( ! strcmp( cv->label, old ) )
		{
			v_map.erase( old );
			delete [ ] cv->label;
			cv->label = new char[ strlen( newname ) + 1 ];
			strcpy( cv->label, newname );
			v_map.insert( v_pairT ( newname, cv ) );
			break;
		}
}


/****************************************************
UNDER_COMPUTATION
Check if any variable in or below the object is
still under computation.
****************************************************/
bool object::under_computation( void )
{
	bridge *cb;
	object *cur;
	variable *cv;

	// check variables in descendants
	for ( cb = b; cb != NULL; cb = cb->next )
		for ( cur = cb->head; cur != NULL; cur = cur->next )
			if ( cur->under_computation( ) )
				return true;

	// check variables directly contained in object
	for ( cv = v; cv != NULL; cv = cv->next )
		if ( cv->under_computation && ! cv->dummy )
			return true;

	return false;
}


/****************************************************
UNDER_COMPUT_VAR
Check if a variable in object is under computation
****************************************************/
bool object::under_comput_var( const char *lab )
{
	variable *cv;

	cv = search_var_err( this, lab, false, false, "retrieving" );

	if ( cv != NULL && cv->under_computation )
		return true;

	return false;
}


/****************************************************
CAL (*)
Return the value of Variable or Parameter with label lab with lag lag.
The method search for the Variable starting from this Object and then calls
the function variable->cal(caller, lag )
***************************************************/
double object::cal( object *caller, const char *lab, int lag, bool force_search )
{
	variable *cv;

	if ( quit == 2 )
		return NAN;

	cv = search_var_err( this, lab, force_search ? false : no_search, false, "retrieving" );
	if ( cv == NULL )
		return NAN;

#ifndef _NP_
	if ( lag == 0 && parallel_ready && cv->parallel && cv->last_update < t && ! cv->dummy )
		parallel_update( cv, this, caller );
#endif
	return cv->cal( caller, lag );
}

double object::cal( object *caller, const char *lab, int lag )
{
	variable *cv;

	if ( quit == 2 )
		return NAN;

	cv = search_var_err( this, lab, no_search, false, "retrieving" );
	if ( cv == NULL )
		return NAN;

#ifndef _NP_
	if ( lag == 0 && parallel_ready && cv->parallel && cv->last_update < t && ! cv->dummy )
		parallel_update( cv, this, caller );
#endif
	return cv->cal( caller, lag );
}

double object::cal( const char *lab, int lag )
{
	return cal( this, lab, lag );
}


/****************************************************
LAST_CAL (*)
Return the last time the variable was calculated
****************************************************/
double object::last_cal( const char *lab )
{
	variable *cv;

	cv = search_var_err( this, lab, no_search, false, "last updating" );
	if ( cv == NULL )
		return NAN;

	return cv->last_update;
}


/****************************************************
RECAL (*)
Mark variable as not calculated in the current time,
forcing recalculation if already calculated
****************************************************/
double object::recal( const char *lab )
{
	int i;
	double app;
	variable *cv;

	cv = search_var_err( this, lab, no_search, false, "recalculating" );
	if ( cv == NULL )
		return NAN;

	// don't do anything if not yet computed in t
	if ( cv->last_update < t )
		return( cv->val[ 0 ] );

	app = cv->val[ 0 ];

	for ( i = 0; i < cv->num_lag; ++i )		// scale up the past values
		cv->val[ i ] = cv->val[ i + 1 ];

	if ( ( cv->save || cv->savei ) && i + 1 <= t - cv->start )
		cv->val[ i ] = cv->data[ t - i - 1 - cv->start ];
	else
		cv->val[ i ] = NAN;

	cv->last_update = t - 1;
	cv->next_update = t;

	return app;
}


/****************************************************
SUM (*)
Compute the sum of Variables or Parameters lab1 with lag lag.
If cond is true check if expression 'V("lab2") lop value'
is true before adding each instance of the object.
The sum is computed over the elements in a single branch of the model.
****************************************************/
double object::sum( const char *lab1, int lag, bool cond, const char *lab2, const char *lop, double value )
{
	int n, lopc;
	double tot;
	object *cur, *cnext;
	variable *cv;

	cv = search_var_err( this, lab1, no_search, true, "summing" );
	if ( cv == NULL )
		return 0;

	if ( cond )
	{
		lopc = logic_op_code( lop, "summing" );
		if ( lopc < 0 || search_var_err( this, lab2, no_search, true, "summing" ) == NULL )
			return 0;
	}
	else
		lopc = -1;

	cur = cv->up;
	if ( cur->up != NULL )
		cur = ( cur->up )->search( cur->label );

	for ( tot = n = 0; cur != NULL; cur = cnext )
	{
		cnext = go_brother( cur );				// allow object suicide

		if ( ! cond || check_cond( cur->cal( this, lab2, lag ), lopc, value ) )
		{
			tot += cur->cal( this, lab1, lag );
			++n;
		}
	}

	return tot;
}


/****************************************************
OVERALL_MAX (*)
Compute the maximum of lab1, considering only the
objects in a single branch of the model.
If cond is true check if expression 'V("lab2") lop value'
is true before considering each instance of the object.
****************************************************/
double object::overall_max( const char *lab1, int lag, bool cond, const char *lab2, const char *lop, double value )
{
	int n, lopc;
	double tot, temp;
	object *cur, *cnext;
	variable *cv;

	cv = search_var_err( this, lab1, no_search, true, "maximizing" );
	if ( cv == NULL )
		return NAN;

	if ( cond )
	{
		lopc = logic_op_code( lop, "maximizing" );
		if ( lopc < 0 || search_var_err( this, lab2, no_search, true, "maximizing" ) == NULL )
			return NAN;
	}
	else
		lopc = -1;

	cur = cv->up;
	if ( cur->up != NULL )
		cur = ( cur->up )->search( cur->label );

	for ( tot = -DBL_MAX, n = 0; cur != NULL; cur = cnext )
	{
		cnext = go_brother( cur );				// allow object suicide

		if ( ! cond || check_cond( cur->cal( this, lab2, lag ), lopc, value ) )
		{
			if ( tot < ( temp = cur->cal( this, lab1, lag ) ) )
				tot = temp;
			++n;
		}
	}

	if ( n > 0 )
		return tot;
	else
		return NAN;
}


/****************************************************
OVERALL_MIN (*)
Compute the minimum of lab1, considering only the
objects in a single branch of the model.
If cond is true check if expression 'V("lab2") lop value'
is true before considering each instance of the object.
****************************************************/
double object::overall_min( const char *lab1, int lag, bool cond, const char *lab2, const char *lop, double value )
{
	int n, lopc;
	double tot, temp;
	object *cur, *cnext;
	variable *cv;

	cv = search_var_err( this, lab1, no_search, true, "minimizing" );
	if ( cv == NULL )
		return NAN;

	if ( cond )
	{
		lopc = logic_op_code( lop, "minimizing" );
		if ( lopc < 0 || search_var_err( this, lab2, no_search, true, "minimizing" ) == NULL )
			return NAN;
	}
	else
		lopc = -1;

	cur = cv->up;
	if ( cur->up != NULL )
		cur = ( cur->up )->search( cur->label );

	for ( tot = DBL_MAX, n = 0; cur != NULL; cur = cnext )
	{
		cnext = go_brother( cur );				// allow object suicide

		if ( ! cond || check_cond( cur->cal( this, lab2, lag ), lopc, value ) )
		{
			if ( tot > ( temp = cur->cal( this, lab1, lag ) ) )
				tot = temp;
			++n;
		}
	}

	if ( n > 0 )
		return tot;
	else
		return NAN;
}


/****************************************************
AV (*)
Compute the average of lab1.
If cond is true check if expression 'V("lab2") lop value'
is true before considering each instance of the object.
****************************************************/
double object::av( const char *lab1, int lag, bool cond, const char *lab2, const char *lop, double value )
{
	int n, lopc;
	double tot;
	object *cur, *cnext;
	variable *cv;

	cv = search_var_err( this, lab1, no_search, true, "averaging" );
	if ( cv == NULL )
		return NAN;

	if ( cond )
	{
		lopc = logic_op_code( lop, "averaging" );
		if ( lopc < 0 || search_var_err( this, lab2, no_search, true, "averaging" ) == NULL )
			return NAN;
	}
	else
		lopc = -1;

	cur = cv->up;
	if ( cur->up != NULL )
		cur = ( cur->up )->search( cur->label );

	for ( tot = n = 0; cur != NULL; cur = cnext )
	{
		cnext = go_brother( cur );				// allow object suicide

		if ( ! cond || check_cond( cur->cal( this, lab2, lag ), lopc, value ) )
		{
			tot += cur->cal( this, lab1, lag );
			++n;
		}
	}

	if ( n > 0 )
		return tot / n;
	else
		return NAN;
}


/****************************************************
WHG_AV (*)
Compute the weighted average (or product sum) of lab1 and lab2.
If cond is true check if expression 'V("lab3") lop value'
is true before considering each instance of the object.
****************************************************/
double object::whg_av( const char *lab1, const char *lab2, int lag, bool cond, const char *lab3, const char *lop, double value )
{
	int n, lopc;
	double tot;
	object *cur, *cnext;
	variable *cv;

	cv = search_var_err( this, lab1, no_search, true, "weighted averaging" );
	if ( cv == NULL )
		return 0;

	cv = search_var_err( this, lab2, no_search, true, "weighted averaging" );
	if ( cv == NULL )
		return 0;

	if ( cond )
	{
		lopc = logic_op_code( lop, "weighted averaging" );
		if ( lopc < 0 || search_var_err( this, lab3, no_search, true, "weighted averaging" ) == NULL )
			return 0;
	}
	else
		lopc = -1;

	cur = cv->up;
	if ( cur->up != NULL )
		cur = ( cur->up )->search( cur->label );

	for ( tot = n = 0; cur != NULL; cur = cnext )
	{
		cnext = go_brother( cur );				// allow object suicide

		if ( ! cond || check_cond( cur->cal( this, lab3, lag ), lopc, value ) )
		{
			tot += cur->cal( this, lab1, lag ) * cur->cal( this, lab2, lag );
			++n;
		}
	}

	return tot;
}


/****************************************************
MED (*)
Compute the median of lab1.
If cond is true check if expression 'V("lab2") lop value'
is true before considering each instance of the object.
****************************************************/
double object::med( const char *lab1, int lag, bool cond, const char *lab2, const char *lop, double value )
{
	return perc( lab1, 0.5, lag, cond, lab2, lop, value );
}


/****************************************************
PERC (*)
Compute the percentile p of lab1.
If cond is true check if expression 'V("lab2") lop value'
is true before considering each instance of the object.
****************************************************/
double object::perc( const char *lab1, double p, int lag, bool cond, const char *lab2, const char *lop, double value )
{
	int n, lopc, floor_x;
	double x, vx, vx1, tmp;
	object *cur, *cnext;
	variable *cv;
	vector < double > vals;

	if ( p < 0 || p > 1 )
	{
		error_hard( "invalid value (0 <= p <= 1 required)",
					"check your equation code to prevent this situation",
					true,
					"percentile '%g' is invalid", p );

		return NAN;
	}

	cv = search_var_err( this, lab1, no_search, true, "calculating percentile" );
	if ( cv == NULL )
		return NAN;

	if ( cond )
	{
		lopc = logic_op_code( lop, "calculating percentile" );
		if ( lopc < 0 || search_var_err( this, lab2, no_search, true, "calculating percentile" ) == NULL )
			return NAN;
	}
	else
		lopc = -1;

	cur = cv->up;
	if ( cur->up != NULL )
		cur = ( cur->up )->search( cur->label );

	// copy selected data series to vector
	for ( n = 0; cur != NULL; cur = cnext )
	{
		cnext = go_brother( cur );				// allow object suicide

		if ( ! cond || check_cond( cur->cal( this, lab2, lag ), lopc, value ) )
		{
			vals.push_back( cur->cal( this, lab1, lag ) );
			++n;
		}
	}

	if ( n > 0 )
	{
		sort( vals.begin( ), vals.end( ) );

		// compute using the C=1 variant a la NumPy
		x = p * ( n - 1 ) + 1;
		floor_x = floor( x );
		vx = vals[ floor_x - 1 ];
		vx1 = floor_x < n ? vals[ floor_x ] : vx;

		return vx + modf( x, &tmp ) * ( vx1 - vx );
	}
	else
		return NAN;
}


/****************************************************
SD (*)
Compute the (population) standard deviation of lab1.
If cond is true check if expression 'V("lab2") lop value'
is true before considering each instance of the object.
****************************************************/
double object::sd( const char *lab1, int lag, bool cond, const char *lab2, const char *lop, double value )
{
	int n, lopc;
	double x, tot, tot2;
	object *cur, *cnext;
	variable *cv;

	cv = search_var_err( this, lab1, no_search, true, "calculating s.d." );
	if ( cv == NULL )
		return NAN;

	if ( cond )
	{
		lopc = logic_op_code( lop, "calculating s.d." );
		if ( lopc < 0 || search_var_err( this, lab2, no_search, true, "calculating s.d." ) == NULL )
			return NAN;
	}
	else
		lopc = -1;

	cur = cv->up;
	if ( cur->up != NULL )
		cur = ( cur->up )->search( cur->label );

	for ( tot = tot2 = n = 0; cur != NULL; cur = cnext )
	{
		cnext = go_brother( cur );				// allow object suicide

		if ( ! cond || check_cond( cur->cal( this, lab2, lag ), lopc, value ) )
		{
			tot += x = cur->cal( this, lab1, lag );
			tot2 += x * x;
			++n;
		}
	}

	if ( n > 0 )
		return sqrt( tot2 / n - pow( tot / n, 2 ) );
	else
		return NAN;
}


/****************************************************
COUNT (*)
Count the number of object lab1 instances below this.
If cond is true check if expression 'V("lab2") lop value'
is true before considering each instance of the object.
****************************************************/
double object::count( const char *lab1, int lag, bool cond, const char *lab2, const char *lop, double value )
{
	int n, lopc;
	object *cur, *cnext;

	cur = search_err( lab1, no_search, "counting" );

	if ( cur == NULL )
		return 0;

	if ( cond )
	{
		lopc = logic_op_code( lop, "counting" );
		if ( lopc < 0 || search_var_err( this, lab2, no_search, true, "counting" ) == NULL )
			return NAN;
	}
	else
		lopc = -1;

	for ( n = 0; cur != NULL; cur = cnext )
	{
		cnext = go_brother( cur );				// allow object suicide

		if ( ! cond || check_cond( cur->cal( this, lab2, lag ), lopc, value ) )
			++n;
	}

	return n;
}


/****************************************************
COUNT_ALL (*)
Count the number of all object lab1 instances below
and besides the current object type (include siblings).
If cond is true check if expression 'V("lab2") lop value'
is true before considering each instance of the object.
****************************************************/
double object::count_all( const char *lab1, int lag, bool cond, const char *lab2, const char *lop, double value )
{
	int n, lopc;
	object *cur, *cnext;

	if ( up->b->head != NULL )
		cur = up->b->head->search_err( lab1, no_search, "counting" );// pick always first instance
	else
		cur = search_err( lab1, no_search, "counting" );	// count from here (bad)

	if ( cur == NULL )
		return 0;

	if ( cond )
	{
		lopc = logic_op_code( lop, "counting" );
		if ( lopc < 0 || search_var_err( this, lab2, no_search, true, "counting" ) == NULL )
			return NAN;
	}
	else
		lopc = -1;

	for ( n = 0; cur != NULL; cur = cnext )
	{
		cnext = cur->hyper_next( lab1 );				// allow object suicide

		if ( ! cond || check_cond( cur->cal( this, lab2, lag ), lopc, value ) )
			++n;
	}

	return n;
}


/****************************************************
STAT (*)
Compute some basic statistics of a group of Variables or
Paramters with label lab1 and storing the results in a vector of double.
If cond is true check if expression 'V("lab2") lop value'
is true before considering each instance of the object.
Return the number of element instances counted (same as r[ 0 ]).

r[ 0 ]=num;
r[ 1 ]=average
r[ 2 ]=variance
r[ 3 ]=max
r[ 4 ]=min
r[ 5 ]=median
r[ 6 ]=standard deviation

****************************************************/
double object::stat( const char *lab1, double *r, int lag, bool cond, const char *lab2, const char *lop, double value )
{
	int n, lopc;
	double val, r_temp[ 7 ];
	object *cur, *cnext;
	variable *cv;
	vector < double > vals;

	if ( r == NULL )
		r = r_temp;

	cv = search_var_err( this, lab1, no_search, true, "calculating statistics" );
	if ( cv == NULL || cv->up == NULL )
	{
		r[ 0 ] = 0;
		r[ 1 ] = r[ 2 ] = r[ 3 ] = r[ 4 ] = r[ 5 ] = r[ 6 ] = NAN;
		return 0;
	}

	if ( cond )
	{
		lopc = logic_op_code( lop, "calculating statistics" );
		if ( lopc < 0 || search_var_err( this, lab2, no_search, true, "calculating statistics" ) == NULL )
			return NAN;
	}
	else
		lopc = -1;

	cur = cv->up;
	r[ 1 ] =  r[ 2 ] = 0;
	r[ 3 ] = DBL_MIN;
	r[ 4 ] = DBL_MAX;

	for ( n = 0; cur != NULL; cur = cnext )
	{
		cnext = go_brother( cur );				// allow object suicide

		if ( ! cond || check_cond( cur->cal( this, lab2, lag ), lopc, value ) )
		{
			val = cur->cal( lab1, lag );
			r[ 1 ] += val;
			r[ 2 ] += val * val;

			if ( val > r[ 3 ] )
				r[ 3 ] = val;

			if ( val < r[ 4 ] )
				r[ 4 ] = val;

			vals.push_back( val );
			++n;
		}
	}

	r[ 0 ] = n;

	if ( n > 0 )
	{
		r[ 1 ] /= n;
		r[ 2 ] = r[ 2 ] / n - r[ 1 ] * r[ 1 ];
		r[ 6 ] = r[ 2 ] >= 0 ? sqrt( r[ 2 ] ) : NAN;

		sort( vals.begin( ), vals.end( ) );

		if ( n % 2 == 0 )
			r[ 5 ] = ( vals[ n / 2 - 1 ] + vals[ n / 2 ] ) / 2;
		else
			r[ 5 ] = vals[ n / 2 ];
	}
	else
		r[ 1 ] = r[ 2 ] = r[ 3 ] = r[ 4 ] = r[ 5 ] = r[ 6 ] = NAN;

	return r[ 0 ];
}


/****************************************************
LSDQSORT (*)
Use the qsort function in the standard library to sort
a group of Object with label obj according to the values of var
if var is NULL, try sorting using the network node id
****************************************************/
int sort_function_up( const void *a, const void *b )
{
	if ( qsort_lab != NULL )		// variable defined?
	{
		if ( ( *( object ** ) a )->cal( qsort_lab, qsort_lag ) < ( *( object ** ) b )->cal( qsort_lab, qsort_lag ) )
			return -1;
		else
			return 1;
	}

	// handles the case of node id comparison
	if ( ( *( object ** ) a )->node->id < ( *( object ** ) b )->node->id )
		return -1;
	else
		return 1;
}

int sort_function_down( const void *a, const void *b )
{
	if ( qsort_lab != NULL )		// variable defined?
	{
		if ( ( *( object ** ) a )->cal( qsort_lab, qsort_lag ) > ( *( object ** ) b )->cal( qsort_lab, qsort_lag ) )
			return -1;
		else
			return 1;
	}

	// handles the case of node id comparison
	if ( ( *( object ** ) a )->node->id > ( *( object ** ) b )->node->id )
		return -1;
	else
		return 1;
}

object *object::lsdqsort( const char *obj, const char *var, const char *direction, int lag )
{
	char dir[ 6 ];
	int num, i;
	bridge *cb;
	object *cur, **mylist;
	variable *cv;
	bool useNodeId = ( var == NULL ) ? true : false;		// sort on node id and not on variable

	if ( ! useNodeId )
	{
		cv = search_var_err( this, var, no_search, true, "sorting" );
		if ( cv == NULL )
			return NULL;

		cur = cv->up;
		if ( cur == NULL || strcmp( obj, cur->label ) )
		{
			error_hard( "variable or parameter not found",
						"create variable or parameter in model structure",
						false,
						"element '%s' is missing (object '%s') for sorting", var, obj );
			return NULL;
		}

		if ( cur->up == NULL )
		{
			error_hard( "object not found",
						"create object in model structure",
						false,
						"object '%s' is missing for sorting", obj );
			return NULL;
		}

		cb = cur->up->search_bridge( obj, true );
	}
	else									// pick network object to sort
	{
		cur = search( obj );
		if ( cur != NULL )
			if ( cur->node != NULL )		// valid network node?
				cb = cur->up->search_bridge( obj, true );
			else
			{
				error_hard( "invalid network object",
							"check your equation code to add\nthe network structure before using this macro",
							true,
							"object '%s' has no network data structure", obj );
				return NULL;
			}
		else
			cb = NULL;
	}

	if ( cb == NULL )
	{
		error_hard( "object not found",
					"create object in model structure",
					false,
					"object '%s' is missing for sorting", obj );
		return NULL;
	}

	if ( cb->head == NULL )
	{
		error_hard( "object has no instance",
					"check your equation code to prevent this situation",
					true,
					"all instances of object '%s' were deleted", obj );
		return NULL;
	}

#ifndef _NP_
	// prevent concurrent sorting by more than one thread
	lock_guard < mutex > lock( parallel_comp );
#endif

	cb->counter_updated = false;
	cur = cb->head;

	skip_next_obj( cur, &num );
	mylist = new object *[ num ];
	for ( i = 0; i < num; ++i )
	{
		mylist[ i ] = cur;
		cur = cur->next;
	}

	strcpyn( dir, direction, 6 );
	strupr( dir );

	qsort_lag = lag;
	qsort_lab = ( char * ) var;

	if ( ! strcmp( dir, "UP" ) )
		qsort( ( void * ) mylist, num, sizeof( mylist[ 0 ] ), sort_function_up );
	else
		if ( ! strcmp( dir, "DOWN" ) )
			qsort( ( void * ) mylist, num, sizeof( mylist[ 0 ] ), sort_function_down );
		else
		{
			error_hard( "invalid sort option ('UP' or 'DOWN' required)",
						"check your equation code to prevent this situation",
						true,
						"direction '%s' is invalid for sorting", direction );
			delete [ ] mylist;
			return NULL;
		}

	cb->head = mylist[ 0 ];

	for ( i = 1; i < num; ++i )
		( mylist[ i - 1 ] )->next = mylist[ i ];

	mylist[ i - 1 ]->next = NULL;

	delete [ ] mylist;

	return cb->head;
}


/****************************************************
LSDQSORT
Two stage sorting. Objects with identical values of var1 are sorted according to their value of var2
****************************************************/
int sort_function_up_two( const void *a, const void *b )
{
	double x, y;

	x = ( *( object ** ) a )->cal( qsort_lab, qsort_lag );
	y = ( *( object ** ) b )->cal( qsort_lab, qsort_lag );

	if ( x < y )
		return -1;
	else
		if ( x > y )
			return 1;
		else
			if ( ( * ( object ** ) a )->cal( qsort_lab_secondary, qsort_lag ) < ( *( object ** ) b )->cal( qsort_lab_secondary, qsort_lag ) )
				return -1;
			else
				return 1;
}

int sort_function_down_two( const void *a, const void *b )
{
	double x, y;

	x = ( *( object ** ) a )->cal( qsort_lab, qsort_lag );
	y = ( *( object ** ) b )->cal( qsort_lab, qsort_lag );

	if ( x > y )
		return -1;
	else
		if ( x < y )
			return 1;
		else
			if ( ( *( object ** ) a )->cal( qsort_lab_secondary, qsort_lag ) > ( *( object ** ) b )->cal( qsort_lab_secondary, qsort_lag ) )
				return -1;
			else
				return 1;
}

object *object::lsdqsort( const char *obj, const char *var1, const char *var2, const char *direction, int lag )
{
	char dir[ 6 ];
	int num, i;
	bridge *cb;
	object *cur, **mylist;
	variable *cv;

	cb = search_bridge( obj, true );			// try to find the bridge

	if ( cb == NULL )
	{
		error_hard( "object not found",
					"create object in model structure",
					false,
					"object '%s' is missing for sorting", obj );
		return NULL;
	}

	if ( cb->head == NULL )
	{
		error_hard( "object has no instance",
					"check your equation code to ensure at least one instance\nof any object is kept",
					true,
					"all instances of object '%s' were deleted", obj );
		return NULL;
	}

	cv = search_var_err( this, var1, no_search, true, "sorting" );
	if ( cv == NULL )
		return NULL;

	cur = cv->up;
	if ( cur == NULL || strcmp( obj, cur->label ) )
	{
		error_hard( "variable or parameter not found",
					"create variable or parameter in model structure",
					false,
					"element '%s' is missing (object '%s') for sorting", var1, obj );
		return NULL;
	}

	if ( cur->up == NULL )
	{
		error_hard( "object not found",
					"create object in model structure",
					false,
					"object '%s' is missing for sorting", obj );
		return NULL;
	}

#ifndef _NP_
	// prevent concurrent sorting by more than one thread
	lock_guard < mutex > lock( parallel_comp );
#endif

	cb->counter_updated = false;
	cur = cb->head;

	skip_next_obj( cur, &num );
	mylist = new object *[ num ];
	for ( i = 0; i < num; ++i )
	{
		mylist[ i ] = cur;
		cur = cur->next;
	}

	strcpyn( dir, direction, 6 );
	strupr( dir );

	qsort_lag = lag;
	qsort_lab = ( char * ) var1;
	qsort_lab_secondary = ( char * ) var2;

	if ( ! strcmp( dir, "UP" ) )
		qsort( ( void * ) mylist, num, sizeof( mylist[ 0 ] ), sort_function_up_two );
	else
		if ( ! strcmp( dir, "DOWN" ) )
			qsort( ( void * ) mylist, num, sizeof( mylist[ 0 ] ), sort_function_down_two );
		else
		{
			error_hard( "invalid sort option ('UP' or 'DOWN' required)",
						"check your equation code to prevent this situation",
						true,
						"direction '%s' is invalid for sorting", direction );
			delete [ ] mylist;
			return NULL;
		}

	cb->head = mylist[ 0 ];

	for ( i = 1; i < num; ++i )
		( mylist[ i - 1 ] )->next = mylist[ i ];

	mylist[ i - 1 ]->next = NULL;

	delete [ ] mylist;

	return cb->head;
}


/*********************
DRAW_RND (*)
Draw randomly an object with label lo with probabilities proportional
to the values of their Variables or Parameters lv
*********************/
object *object::draw_rnd( const char *lo, const char *lv, int lag )
{
	double a, b;
	object *cur, *cur1, *cnext;
	variable *cv;

	cv = search_var_err( this, lv, no_search, true, "random drawing" );
	if ( cv == NULL )
		return NULL;

	cur1 = cur = cv->up;

	for ( a = 0; cur != NULL; cur = cnext )
	{
		cnext = cur->next;						// allow object suicide
		a += cur->cal( lv, lag );
	}

	if ( is_nan( a ) || is_inf( a ) )
	{
		error_hard( "invalid random draw option",
					"check your equation code to prevent this situation",
					true,
					"element '%s' has invalid value '%g' for random drawing", lv, a );
		return NULL;
	}

	if ( a == 0 )
	{
		error_hard( "invalid random draw option",
					"check your equation code to prevent this situation",
					true,
					"element '%s' has only zero values for random drawing", lv );
		return NULL;
	}

	do
	{
		b = ran1( ) * a;
	}
	while ( b == a );	// avoid ran1 == 1

	a = cur1->cal( lv, lag );
	for ( cur = cur1, cur1 = cur1->next; a <= b && cur1 != NULL; cur1 = cnext )
	{
		cnext = cur1->next;						// allow object suicide
		a += cur1->cal( lv, lag );
		cur = cur1;
	}

	return cur;
}


/*********************
DRAW_RND (*)
Draw randomly an object with label lab with identical probabilities
*********************/
object *object::draw_rnd( const char *lab )
{
	double a, b;
	object *cur, *cur1;

	cur1 = cur = search_err( lab, no_search, "random drawing" );

	if ( cur == NULL )
		return NULL;

	for ( a = 0 ; cur != NULL; cur = cur->next )
		++a;

	if ( a == 0 )
	{
		error_hard( "object not found",
					"create object in model structure",
					false,
					"object '%s' is missing for random drawing", lab );
		return NULL;
	}

	do
	{
		b = ran1( ) * a;
	}
	while ( b == a );	// avoid ran1 == 1

	for ( a = 1, cur = cur1, cur1 = cur1->next; a <= b && cur1 != NULL; cur1 = cur1->next )
	{
		++a;
		cur = cur1;
	}

	return cur;
}


/*************
DRAW_RND (*)
Same as draw_rnd but faster, assuming the sum of the probabilities to be tot
***************/
object *object::draw_rnd( const char *lo, const char *lv, int lag, double tot )
{
	double a, b;
	object *cur, *cur1, *cnext;
	variable *cv;

	if ( tot <= 0 )
	{
		error_hard( "invalid random draw option",
					"check your equation code to prevent this situation",
					true,
					"element '%s' has invalid value '%g' for random drawing", lv, tot );
		return NULL;
	}

	cv = search_var_err( this, lv, no_search, true, "random drawing" );
	if ( cv == NULL )
		return NULL;

	cur1 = cur = cv->up;

	b = ran1( ) * tot;
	cnext = cur1->next;
	a = cur1->cal( lv, lag );
	for ( cur1 = cnext; a <= b && cur1 != NULL; cur1 = cnext )
	{
		cnext = cur1->next;				// allow object suicide
		a += cur1->cal( lv, lag );
		cur = cur1;
	}

	if ( a > tot )
	{
		error_hard( "invalid random draw option",
					"check your equation code to prevent this situation",
					true,
					"element '%s' has invalid value '%g' for random drawing", lv, tot );
		return NULL;
	}

	return cur;
}


/****************************************************
 WRITE (*)
 Write the value in the Variable or Parameter lab, making it appearing as if
 it was computed at time lag and the variable updated at time time.
 ***************************************************/
double object::write( const char *lab, double value, int time, int lag )
{
	int i, eff_lag, eff_time;
	variable *cv;

	if ( ( ! use_nan && is_nan( value ) ) || is_inf( value ) )
	{
		error_hard( "invalid write operation",
					"check your equation code to prevent this situation",
					true,
					"value '%g' is invalid for writing to element '%s'", value, lab );
		return NAN;
	}

	cv = search_var_err( this, lab, true, false, "writing" );
	if ( cv == NULL )
		return NAN;

	if ( cv->under_computation )
	{
		if ( ! cv->dummy )
		{
			error_hard( "invalid write operation",
						"check your equation code to prevent this situation",
						true,
						"variable '%s' is under computation and cannot be written", lab );
			return NAN;
		}

#ifndef _NP_
		if ( cv->parallel_comp.try_lock( ) )
			cv->parallel_comp.unlock( );
		else
		{
			error_hard( "deadlock during parallel computation",
						"check your equation code to prevent this situation",
						true,
						"variable '%s' is under dummy computation and cannot be written", lab );
			return NAN;
		}
#endif
	}

#ifndef _NP_
	// prevent concurrent use by more than one thread
	rec_lguardT lock( cv->parallel_comp );
#endif
	if ( cv->param != 1 && time <= 0 && t > 1 )
	{
		error_hard( "invalid write operation",
					"check your equation code to prevent this situation",
					true,
					"invalid update time (%d) for variable '%s'", time, lab );
		return NAN;
	}

	// allow for change of initial lagged values when starting simulation (t=1)
	if ( cv->param != 1 && time < 0 && t == 1 )
	{
		if ( - time > cv->num_lag )		// check for invalid lag
		{
			error_hard( "invalid write operation",
						"check your configuration (variable max lag) or\ncode (used lags in equation) to prevent this situation",
						false,
						"invalid initial lag (%d) for variable '%s'", time, lab );
			return NAN;
		}

		cv->val[ - time - 1 ] = value;
		cv->last_update = 0;	// force new updating

		if ( time == -1 && ( cv->save || cv->savei ) )
			cv->data[ 0 ] = value;

		// choose next update step for special updating variables
		if ( cv->delay > 0 || cv->delay_range > 0 )
		{
			cv->next_update = cv->delay;
			if ( cv->delay_range > 0 )
				cv->next_update += rnd_int( 0, cv->delay_range );
		}
	}
	else
	{
		if ( lag < 0 || ( cv->param != 1 && lag > cv->num_lag ) || ( cv->param == 1 && lag > 1 ) )
		{
			error_hard( "invalid write operation",
						"check your configuration (variable max lag) or\ncode (used lags in equation) to prevent this situation",
						false,
						"invalid lag (%d) for %s '%s'", lag, cv->param != 1 ? "variable" : "parameter", lab );
			return NAN;
		}

		if ( cv->param == 1 )
		{
			eff_lag = 0;
			eff_time = time;
		}
		else
		{
			// if not yet calculated this time step, adjust lagged values
			if ( time >= t && lag == 0 && cv->last_update < t )
				for ( i = 0; i < cv->num_lag; ++i )
					cv->val[ cv->num_lag - i ] = cv->val[ cv->num_lag - i - 1 ];

			if ( lag == 0 )
			{
				eff_lag = 0;
				eff_time = time;

				// choose next update step for special updating variables
				if ( cv->period > 1 || cv->period_range > 0 )
				{
					cv->next_update = t + cv->period;
					if ( cv->period_range > 0 )
						cv->next_update += rnd_int( 0, cv->period_range );
				}
			}
			else
			{
				// handle rewriting already computed values
				if ( time >= t || time >= cv->last_update )
				{
					eff_lag = lag - ( t - cv->last_update );	// first write in time t
					eff_time = time - lag;
				}
				else
				{
					eff_lag = lag - ( t - time );				// rewrite as t-h in time t
					eff_time = t - lag;
				}

				if ( eff_lag < 0 || eff_lag > cv->num_lag )
				{
					error_hard( "invalid write operation",
								"check your configuration (variable max lag) or\ncode (used lags in equation) to prevent this situation",
								true,
								"invalid update case (%d) and lag (%d) for variable '%s'", time, lag, lab );
					return NAN;
				}
			}
		}

		cv->val[ eff_lag ] = value;
		cv->last_update = time;

		if ( cv->save || cv->savei )
		{
			if ( eff_time >= cv->start && eff_time <= cv->end )
				cv->data[ eff_time - cv->start ] = value;
			else
				// handle special initial case
				if ( time == 0 && cv->start == 0 )
					cv->data[ 0 ] = value;
		}
	}

	return value;
}


/************************************************
INCREMENT (*)
Increment the value of the variable lab with value.
Mark variable as computed in t.
Return the new value.
*************************************************/
double object::increment( const char *lab, double value )
{
	variable *cv;
	double new_value;

	if ( ( ! use_nan && is_nan( value ) ) || is_inf( value ) )
	{
		error_hard( "invalid increment operation",
					"check your equation code to prevent this situation",
					true,
					"value '%g' is invalid for incrementing element '%s'", value, lab );
		return NAN;
	}

	cv = search_var_err( this, lab, true, false, "incrementing" );
	if ( cv == NULL )
		return NAN;

	if ( ! use_nan && is_nan( cv->val[ 0 ] ) )	// try to recover from RECALC
		cv->cal( this, 0 );

	if ( ( ! use_nan && is_nan( cv->val[ 0 ] ) ) || is_inf( cv->val[ 0 ] ) )
	{
		error_hard( "invalid increment operation",
					"check your equation code to prevent this situation",
					true,
					"current value '%g' of element '%s' is invalid for incrementing", cv->val[ 0 ], lab );
		return NAN;
	}

	new_value = cv->val[ 0 ] + value;
	this->write( lab, new_value, t );

	return new_value;
}


/************************************************
MULTIPLY (*)
Multiply the value of the variable lv with value.
Mark variable as computed in t.
Return the new value.
*************************************************/
double object::multiply( const char *lab, double value )
{
	variable *cv;
	double new_value;

	if ( ( ! use_nan && is_nan( value ) ) || is_inf( value ) )
	{
		error_hard( "invalid multiply operation",
					"check your equation code to prevent this situation",
					true,
					"value '%g' is invalid for multiplying element '%s'", value, lab );
		return NAN;
	}

	cv = search_var_err( this, lab, true, false, "multiplying" );
	if ( cv == NULL )
		return NAN;

	if ( ! use_nan && is_nan( cv->val[ 0 ] ) )	// try to recover from RECALC
		cv->cal( this, 0 );

	if ( ( ! use_nan && is_nan( cv->val[ 0 ] ) ) || is_inf( cv->val[ 0 ] ) )
	{
		error_hard( "invalid multiply operation",
					"check your equation code to prevent this situation",
					true,
					"current value '%g' of element '%s' is invalid for multiplying", cv->val[ 0 ], lab );
		return NAN;
	}

	new_value = cv->val[ 0 ] * value;
	this->write( lab, new_value, t );

	return new_value;
}


/****************************
LAT_DOWN (*)
return the object "up" the cell of a lattice
*****************************/
object *object::lat_down( void )
{
	int i, j;
	object *cur;

	for ( i = 1, cur = up->search( label ); cur != this; cur = go_brother( cur ), ++i );

	cur = go_brother( up );
	if ( cur == NULL )
		cur = up->up->search( up->label );

	for ( j = 1, cur = cur->search( label ); j < i; cur = go_brother( cur ), ++j );

	return cur;
}


/****************************
LAT_UP (*)
return the object "down" the cell of a lattice
*****************************/
object *object::lat_up( void )
{
	int i, k;
	object *cur, *cur1, *cur2;

	for ( i = 1, cur = up->search( label ); cur != this; cur = go_brother( cur ), ++i );

	cur = up->up->search( up->label );
	if ( cur == up )
		for ( cur1 = up; go_brother( cur1 ) != NULL; cur1 = go_brother( cur1 ) );
	else
		for ( cur1 = cur; go_brother( cur1 ) != up; cur1 = go_brother( cur1 ) );

	for ( cur2 = cur1->search( label ), k = 1; k < i; cur2 = go_brother( cur2 ), ++k );

	return cur2;
}


/****************************
LAT_RIGHT (*)
return the object "right" the cell of a lattice
*****************************/
object *object::lat_right( void )
{
	object *cur;

	cur = go_brother( this );
	if ( cur == NULL )
	 cur = up->search( label );

	return cur;
}


/****************************
LAT_LEFT (*)
return the object "left" the cell of a lattice
*****************************/
object *object::lat_left( void )
{
	object *cur;

	if ( up->search( label ) == this )
		for ( cur = this; go_brother( cur ) != NULL; cur = go_brother( cur ) );
	else
		for ( cur = up->search( label ); go_brother( cur ) != this; cur = go_brother( cur ) );

	return cur;
}


/****************************************************
BUILD_OBJ_LIST
Build the object list for user pointer checking
****************************************************/
double build_obj_list( bool set_list )
{
	if ( no_pointer_check )		// disabled in compilation?
	{
		no_ptr_chk = true;
		return 0;
	}

#ifndef _NP_
	// prevent concurrent update by more than one thread
	lock_guard < mutex > lock( lock_obj_list );
#endif

	obj_list.clear( );			// reset list

	if ( set_list )
	{
		collect_inst( root, obj_list );
		no_ptr_chk = false;
	}
	else
		no_ptr_chk = true;

	return obj_list.size( );
}


/****************************************************
COLLECT_INST
Collect all object under the selected object and
stores it in the provided C++ set container
****************************************************/
void collect_inst( object *r, o_setT &list )
{
	bridge *cb;
	object *cur;
	pair < o_setT::iterator, bool > res;

	// collect own address
	res = list.emplace( r );
	if ( ! res.second )
	{
		error_hard( "LSD internal error",
					"disable pointer checking by defining 'NO_POINTER_CHECK'",
					false,
					"object '%s' cannot be collected for pointer checking", r->label );
		return;
	}

	// search among descendants
	for ( cb = r->b; cb != NULL; cb = cb->next )
		for ( cur = cb->head; cur != NULL; cur = cur->next )
			collect_inst( cur, list );
}


/*******************************************
INTERACT (*)
Interrupt the simulation, as for the debugger, allowing the insertion of a value.
Note that the debugging window, in this model, accept the entry key stroke as a run.
********************************************/
double object::interact( const char *text, double v, double *tv, int i, int j,
						 int h, int k, object *cur, object *cur1, object *cur2,
						 object *cur3, object *cur4, object *cur5, object *cur6,
						 object *cur7, object *cur8, object *cur9, netLink *curl,
						 netLink *curl1, netLink *curl2, netLink *curl3,
						 netLink *curl4, netLink *curl5, netLink *curl6,
						 netLink *curl7, netLink *curl8, netLink *curl9 )
{
#ifndef _NW_
	int n;
	double app = v;

	if ( quit == 0 )
	{
		for ( n = 0; n < USER_D_VARS; ++n )
			d_values[ n ] = tv[ n ];

		i_values[ 0 ] = i;
		i_values[ 1 ] = j;
		i_values[ 2 ] = h;
		i_values[ 3 ] = k;
		o_values[ 0 ] = cur;
		o_values[ 1 ] = cur1;
		o_values[ 2 ] = cur2;
		o_values[ 3 ] = cur3;
		o_values[ 4 ] = cur4;
		o_values[ 5 ] = cur5;
		o_values[ 6 ] = cur6;
		o_values[ 7 ] = cur7;
		o_values[ 8 ] = cur8;
		o_values[ 9 ] = cur9;
		n_values[ 0 ] = curl;
		n_values[ 1 ] = curl1;
		n_values[ 2 ] = curl2;
		n_values[ 3 ] = curl3;
		n_values[ 4 ] = curl4;
		n_values[ 5 ] = curl5;
		n_values[ 6 ] = curl6;
		n_values[ 7 ] = curl7;
		n_values[ 8 ] = curl8;
		n_values[ 9 ] = curl9;

		deb( this, NULL, text, &app, true );	// signals INTERACT macro
	}

	return app;
#else
	return v;
#endif
}


/****************************************************
LOGIC_OP_CODE
Check for valid relational operator and return
operator code for CHECK_COND
****************************************************/
const unordered_map < string, int > logic_ops = { { "==", 0 }, { "=", 0 }, { "EQ", 0 }, { "!=", 1 }, { "=!", 1 }, { "NE", 1 }, { ">", 2 }, { "GT", 2 }, { ">=", 3 }, { "=>", 3 }, { "GE", 3 }, { "<", 4 }, { "LT", 4 }, { "<=", 5 }, { "=<", 5 }, { "LE", 5 } };

int logic_op_code( const char *lop, const char *errmsg )
{
	auto lopp = logic_ops.find( lop );

	if ( lopp != logic_ops.end( ) )
		return lopp->second;

	error_hard( "invalid logical relational operator",
				"use a valid operator (== != > >= < <=)",
				false,
				"cannot compare with '%s' for %s", lop, errmsg );
	return -1;
}


/****************************************************
CHECK_COND
Check if logical condition defined by the logical
operator code and the two values is true
****************************************************/
bool check_cond( double val1, int lopc, double val2 )
{
	switch ( lopc )
	{
		case 0:
			return val1 == val2;
		case 1:
			return val1 != val2;
		case 2:
			return val1 > val2;
		case 3:
			return val1 >= val2;
		case 4:
			return val1 < val2;
		case 5:
			return val1 <= val2;
		default:
			return false;
	}
}
