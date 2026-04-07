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
 OBJECT.CPP
 It contains the core code for LSD, together with VARIAB.CPP.

 A model is nothing but a link of objects, whose behavior is
 defined here. Only the methods for saving a loading a model
 are placed in another file, FILE.CPP.

 An object is composed by some fields and a set of methods.
 The fields are used to identify the object type and to insert
 it in a model.

 - char *label;
 Name of the object. The name is used indicate one specific
 type of object in the model. Two objects are always identical
 in their definition. Inheritance is not used in LSD, yet.

 - variable *v;
 the first element of a linked chain of variable. They are
 the computational content of the model

 - bool to_compute;
 flag set by default to 1. If it is zero, the system will not
 compute the equations of this object as a default, but only
 if they are requested by other equations. Used to speed up
 the simulation.

 - object *b;
 pointer to the object's linked-list of bridges. The bridges
 connect the object with its sons. There is one bridge for
 each son object (even if it has many instances). The bridge
 points to the head of a linked list of the son instances.

 - object *up;
 pointer to the parent object. Root is the only object
 having no parent (the value of up is then NULL).

 - object *next;
 pointer to the next object in the linked chain of the
 descendant of the parent of this object.

 - network *node;
 pointer to the data structure containing the network links
 from the object (see nets.cpp for the details)

 The drawing below sketches one object. All the object of
 the same chain same parent, that is up. They can only
 provide a way to continue along the linked chain (via next).
 The only way to "go back" is by starting again: go "up",
 pick the bridge to the desired son object and pick the head
 of the corresponding linked list and then follow all the
 chain again.

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

 This definition of object allows to define a model as a
 multiple dimensional tree, where it is possible to browse
 the model with very limited code.

 METHODS
 The methods for object implemented here all refer always to
 the "this" object. That is, if you consider the following as
 functions, then they have always as parameter the address of
 one object, refer to as "this", whose fields are addressed as
 if they were public variables.

 Methods as listed in two groups: the ones that can be used as
 functions in LSD and the ones used for management of the model.
 This distinction is only because of the functionalities, since
 all the methods are actually public, and could be used anyway.
 It is just that you wouldn't like to, say, save a model in the
 middle of an equation.

 METHODS FOR EQUATIONS (marked with an *)

 - double cal( char *l, int lag );
 Interface to another type of cal(see below), that uses also
 the address of this. Provides the value of one variable whose
 label is lab. The value corresponds to the time t-lag, where
 t is the global time value when the cal is
 made.
 If there is only one variale l in the model, that one is
 found, wherever is placed in the model. If, instead, there
 are many variables l, the variable returned depends on the
 position of this in the model, in respect of the position of
 the objects owning l. If l is in the same object "this", then
 this is returned. Otherwise, is returned the first l found
 following the strategy used in search_var (see below for a
 detailed description of search_var). In general, this means
 to return the intuitively correct variable. The case for
 errors is when the variable l is in objects not directly
 related with "this" in the hierarchical structure of the model.

 - variable *search_var(object *caller,char *label);
 It explores the model starting from this and gradually
 extending till considering the whole model.
 It searches for an object having a variable whose label is l
 and returns the first found.
 The research strategy used by this method is simple:
 1) search among the variables of this. If not found
 2) search among the variables of the descending objects.
    If not found
 3) search among the variables of parent object.
 Each object encountered during a search perform the same
 search strategy. The strategy ensures that the whole model
 is searched, hence always returns a value, provided that
 variable l exists. The problem is to be sure that, in case
 of multiple instances of variable l, the correct one is
 returned. This depends on the right choice of "this", that
 is, where the search is starting from.
 The field caller is used to avoid deadlocks when from
 descendants the search goes up again, or from the parent down.

 - object *search_var_cond( char *lab, double value, int lag );
 Uses search_var, but returns the instance of the object that
 has the searched variable with the desired value equal to value.

 - double overall_max( char *lab, int lag );
 Searches for the object having the variable lab. From that
 object, it considers the whole group of object of the same type
 as the one found, and searches the maximum value of the
 variables lab with lag lag there contained

 - double sum( const char *lab1, int lag, bool cond, const char *lab2, const char *lop, double value );
 Searches for the object having the variable lab1. From that
 object, it considers the whole group of object of the same type
 as the one found, and returns the sum of all the variables lab
 with lag lag in that group. If cond is true, only objects
 satisfying the logical condition 'V( "lab2" ) lop value' will
 be considered form summing. lab2 should be in the same object as
 lab1 or be the same as lab1.

 - double whg_av( char *lab1, char *lab2, int lag, bool cond, const char *lab2, const char *lop, double value );
 Same as sum, but it adds up the product between variables lab
 and lab2 for each object. lab and lab2 must be in the same object.

 - void lsdqsort( char *obj, char *var, char *dir, int lag );
 Sorts the Objects whose label is obj according to the values of
 their variable var. The direction of sorting can be UP or DOWN.
 The method is just an interface for sort_asc and sort_desc below.

 IMPORTANT:
 The initial Object must be the first element of the set of
 Objects to be sorted, and hence is must contain a Variable or
 Parameter labeled Var_label. The field from must be either the
 Object whose "next" is this, or, in case this is the first element
 of descendants from some Objects and hence it is a son, it must be
 the address of the parent of this.

 - void delete_obj( const variable *caller ) ;
 Eliminate the object, keeping in order the chain list.

 - void stat( char *lab, double *v, int lag, bool cond, const char *lab2, const char *lop, double value );
 Reports some statistics on the values of variable named lab
 contained in one group of object descending from the this. The
 results are stored in the vector v, with the following order:
 v[0]=number of instances;
 v[1]=average
 v[2]=variance
 v[3]=max
 v[4]=min

 - void write( char *lab, double value, int time, int lag )
 Assign the value value to the variable lab, resulting as if this
 was the value at gloabal time time. It does not make a search
 looking for lab. Lab must be a variable of this.
 The function allows to override the default system to update
 variables in LSD during a simulation time step.
 In general, through update, a variable is computed either
 because the system requests its value via the "update" method,
 or because, before "update", another equation needs its updated
 value. An equation can instead call "write"

 For sake of completeness, here are other two functions, not
 members of object, that are extensively used in equations, besides
 in the following code

 - object *next_count( object *t, int *count );
 Counts how many types of objects equal to t are in this group.
 count returns such value, and the whole function returns the
 next object after the last of the series of t.

 METHODS NOT USED IN THE EQUATIONS

 - double cal( object *caller, char *l, int lag, int *done );
 It is the basic function used in the equations for LSD variables.
 It is called by the former type of method cal(l,lag ), because
 that is simpler to be used in the equation code. It activates
 the method search_var( caller, label ) that returns a variable
 whose name is label and then calls the method cal() for that
 variable (see variable::cal), that returns the desired value.

 - void init( object *_up, simulation *_sim, char *_label,
			  bool _to_compute );
 Initialization for an object. Assigns _up to up, _sim to sim,
 and creates the label

 - void update( bool recurse ) ;
 The recursive function computing the equations for a new time
 step in the simulation. It first requests the values for its
 own variables. Then calls update for all the descendants if
 recurse is true.

 - object *hyper_next( char *lab );
 Returns the next object whose name is lab. That is, it makes
 a search only down and up, but does never consider objects
 before the one from which the search starts from. It is used to
 chase objects of lab type even when they are scattered in
 different groups.

 - object *add_obj( char *label, int num, bool propagate,
					int prng_type, bool to_compute, bool blueprint );
 Add a new object type in the model as descendant of current one
 and initialize its name. It makes num copiesof it, and can
 propagate to other instances of the same parent object.

 - void move( char *dest );
 Move the current object as descendant to a new parent

 - object *search( char *lab );
 Explores one branch of the model to find for an object whose
 label is lab. It searches only down and next. Only for root,
 the search is extensive on the whole model.

 - void chg_lab( char *lab );
 Changes the name of an object type, that is for all the
 object of this type in the model

 - void chg_var_lab( char *old, char *n );
 Only to this object, changes the label of the variable whose
 label is old, and it si changed in n

 - variable *add_var( char *str );
 Add a variable before knowing its contents, setting to a
 default initialization values all the fields in the variable.
 It operates only on object this

 - variable *add_var( variable *example );
 Add a variable instance copying all the fields by the variable
 example. It operates only on object this

 - void empty( void ) ;
 Deletes all the contents of the object, freeing its memory.
 Used in delete_obj just before suicide with delete this;

 METHODS FOR NETWORK OPERATION

 see nets.cpp

 METHODS FOR FILE OPERATION

 see file.cpp
 *************************************************************/

#include "lib/libLSD.h"				// LSD library classes


/*************************************************************
 OBJATTR constructor
 *************************************************************/
lsd::objattr::objattr( objattributes *_cont, objattr *_par_attr, const char *_label )
{
	cont = _cont;
	par_attr = _par_attr;
	label_size = strlen( _label );

	label = new char [ label_size + 1 ];
	strcpy( label, _label );
}


/*************************************************************
 OBJATTR copy constructor
 *************************************************************/
lsd::objattr::objattr( const objattr & a )
{
	cont = a.cont;
	par_attr = a.par_attr;
	label_size = a.label_size;

	label = new char [ a.label_size + 1 ];
	strcpy( label, a.label );
}


/*************************************************************
 OBJATTR destructor
 *************************************************************/
lsd::objattr::~objattr( void )
{
	if ( cont != NULL )
		cont->attr_map.erase( label );

	delete [ ] label;
}


/*************************************************************
 OBJATTRIBUTES constructor
 *************************************************************/
lsd::objattributes::objattributes( simulation *_sim )
{
	sim = _sim;
}


/*************************************************************
 OBJATTRIBUTES destructor
 *************************************************************/
lsd::objattributes::~objattributes( void )
{
	empty_objattributes( sim );
}


/*************************************************************
 EMPTY_OBJATTRIBUTES
 *************************************************************/
void lsd::empty_objattributes( simulation *sim )
{
	if ( sim == NULL && sims.size( ) > 0 )
		sim = sims[ 0 ];

	if ( sim == NULL )
		return;

	sim->oa.attr.clear( );
	sim->oa.attr_map.clear( );
}


/*************************************************************
 SEARCH
 *************************************************************/
lsd::objattr *lsd::objattributes::search( const char *lab )
{
	if ( lab != NULL && strlen( lab ) > 0 )
	{
		auto d = attr_map.find( lab );
		if ( d != attr_map.end( ) )
			return & ( *( d->second ) );
	}

	return NULL;
}


/*************************************************************
 ADD
 *************************************************************/
lsd::objattr *lsd::objattributes::add( objattr *par_attr, const char *lab )
{
	auto d = attr_map.find( lab );

	// prevent concurrent use by more than one thread
	rec_lguardT lock( oattr_lck );

	if ( d != attr_map.end( ) )
	{
		d->second->par_attr = par_attr;
		return & ( *( d->second ) );
	}

	attr.emplace_back( this, par_attr, lab );
	attr_map.emplace( lab, --attr.end( ) );

	return & attr.back( );
}


/*************************************************************
 RENAME
 *************************************************************/
lsd::objattr *lsd::objattributes::rename( const char *old_lab, const char *new_lab )
{
	auto d = attr_map.find( old_lab );
	if ( d == attr_map.end( ) || strlen( new_lab ) == 0 )// doesn't exist or empty?
		return NULL;

	// prevent concurrent use by more than one thread
	rec_lguardT lock( oattr_lck );

	auto attr = d->second;
	attr->label_size = strlen( new_lab );

	delete [ ] attr->label;
	attr->label = new char [ attr->label_size + 1 ];
	strcpy( attr->label, new_lab );

	attr_map.erase( d );
	attr_map.emplace( new_lab, attr );

	return & ( *attr );
}


/*************************************************************
 OBJECT constructor
 *************************************************************/
lsd::object::object( object *_up, const char *_label, int _prng_type, bool _to_compute, bool blueprint, simulation *sim )
{
	objattr *par_attr;

	up = _up;
	prng_type = _prng_type;
	to_compute = _to_compute;

	if ( up != NULL )
	{
		sim = up->attr->cont->sim;
		par_attr = up->attr;
	}
	else						// handle case o root, no parent
		par_attr = NULL;

	if ( ( attr = sim->oa.search( _label ) ) == NULL )
		attr = sim->oa.add( par_attr, _label );
	else
	{
		attr->par_attr = par_attr;
		attr->cont = & sim->oa;
	}

	if ( ! blueprint )
		alloc_prng( );			// allocate random generator
}


/*************************************************************
 OBJECT destructor
 *************************************************************/
lsd::object::~object( void )
{
	bridge *cb, *cb1;
	variable *cv, *cv1;

	free_prng( );				// deallocate random generator

	// remove variables if cemetery collection was not called before
	for ( cv = v; cv != NULL; cv = cv1 )
	{
		cv1 = cv->next;
		cv->delete_var( );
	}

	for ( cb = b; cb != NULL; cb = cb1 )	// delete son bridges
	{
		cb1 = cb->next;
		delete cb;				// bridge destructor delete the rest
	}

	delete node;
}


/*************************************************************
 BRIDGE constructor
 *************************************************************/
lsd::bridge::bridge( objattr *_attr )
{
	attr = _attr;
}


/*************************************************************
 BRIDGE move constructor
 *************************************************************/
lsd::bridge::bridge( bridge && b )
{
	b.search_var = NULL;
	b.head = NULL;
}


/*************************************************************
 BRIDGE destructor
 *************************************************************/
lsd::bridge::~bridge( void )
{
	object *cnext;

	for ( auto cur = head; cur != NULL; cur = cnext )
	{
		cnext = cur->next;
		cur->collect_cemetery( );
		delete cur;
	}

	delete [ ] search_var;
}


/*************************************************************
 RECREATE_MAPS
 Recreate both fast look-up maps
 *************************************************************/
void lsd::object::recreate_maps( void )
{
	bridge *cb;
	variable *cv;

	v_map.clear( );
	b_map.clear( );

	for ( cv = v; cv != NULL; cv = cv->next )
		v_map.insert( v_pairT( cv->attr, cv ) );

	for ( cb = b; cb != NULL; cb = cb->next )
		b_map.insert( b_pairT ( cb->attr, cb ) );
}


/*************************************************************
 UPDATE (*)
 Compute the value of all the Variables in the Object, saving
 the values and updating the runtime plot.
 For optimization purposes the system tries to ignores
 descending objects marked to be not computed. The
 implementation is quite baroque, but it should be the fastest.
 *************************************************************/
void lsd::object::update( bool recurse, bool user )
{
	static bool deleted;
	bridge *cb, *cb1;
	object *cur, *cnext;
	simulation *sim;
	variable *cv;

	deleted = false;
	sim = attr->cont->sim;
	del_flag = & deleted;			// register feedback channel

	for ( cv = v; ! deleted && cv != NULL && sim->quit != 2; cv = cv->next )
	{
		if ( cv->under_computation )// don't update if under computation!
			continue;

		if ( cv->param == 0 && cv->last_update < sim->t )
		{
			if ( sim->parallel_ready && cv->attr->parallel && ! cv->attr->dummy )
				sim->parallel_update( cv, this );
			else
				cv->cal( NULL, 0 );
		}

		if ( ! deleted	)
		{
			if ( cv->attr->save || cv->attr->savei )
				cv->data[ sim->t - cv->start ] = cv->val[ 0 ];
#ifndef _TERM_
			if ( ! user && cv->plot && sim->liblnk != NULL && sim->liblnk->plot_runtime != NULL )
			{
				if ( cv->param == 1 || cv->attr->num_lag == 0 )
					sim->liblnk->plot_runtime( NULL, sim->t, cv->val[ 0 ], NAN );
				else
					sim->liblnk->plot_runtime( NULL, sim->t, cv->val[ 0 ], cv->val[ 1 ] );
			}
#endif
		}
	}

	if ( recurse )
		for ( cb = b; ! deleted && cb != NULL && sim->quit != 2; cb = cb1 )
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


/*************************************************************
 NEXT_OBJ
 Return the next (different) object under the same
 parent. Search doesn't move to different branches.
 *************************************************************/
lsd::object *lsd::object::next_obj( object *obj )
{
	bridge *cb;

	if ( obj == NULL || obj->up == NULL )
		return NULL;

	cb = obj->up->search_bridge( obj->attr );

	if ( cb == NULL || cb->next == NULL )
		return NULL;
	else
		return cb->next->head;
}


/*************************************************************
 NEXT_COUNT
 Counts the number of instances of the given object
 obj before the the next (different) object under
 the same parent, returning it. Search doesn't move
 to different branches.
 *************************************************************/
lsd::object *lsd::object::next_count( object *obj, int *count )
{
	object *cur;

	for ( cur = obj, *count = 0; cur != NULL; cur = cur->next, ++( *count ) );

	return next_obj( obj );
}


/*************************************************************
 HYPER_NEXT
 Return the next Object in the model with the attribute at
 or label lab. The Object is searched in the whole model,
 including different branches
 *************************************************************/
lsd::object *lsd::object::hyper_next( objattr *at )
{
	object *cur, *cur1;

	for ( cur1 = NULL, cur = next; cur != NULL; cur = next_obj( cur ) )
	{
		cur1 = cur->search( at );
		if ( cur1 != NULL )
			return cur1;
	}

	if ( up != NULL )
		cur = up->hyper_next( at );

	return cur;
}

lsd::object *lsd::object::hyper_next( const char *lab )
{
	objattr *at = attr->cont->sim->oa.search( lab );
	if ( at != NULL )
		return hyper_next( at );

	return NULL;
}

// search object with same name as the current object
lsd::object *lsd::object::hyper_next( void )
{
	return hyper_next( attr );
}


/*************************************************************
 HYPER_COUNT
 Return the total number of Object instances in the
 model with the label lab. The Object is searched
 in the whole model, including different branches
 *************************************************************/
int lsd::simulation::hyper_count( const char *lab )
{
	int n;
	object *cur;

	for ( n = 0, cur = root->search( lab ); cur != NULL; ++n, cur = cur->hyper_next( ) );

	return n;
}


/*************************************************************
 HYPER_COUNT_VAR
 Return the total number of Object instances in the
 model which contain the variable named lab.
 The Object is searched in the whole model, including
 different branches
 *************************************************************/
int lsd::simulation::hyper_count_var( const char *lab )
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


/*************************************************************
 SEARCH_BRIDGE
 Search the bridge which contains the Object lab
 in this.
 Uses the fast bridge look-up map.
 *************************************************************/
lsd::bridge *lsd::object::search_bridge( objattr *at, bool no_error )
{
	auto bit = b_map.find( at );
	if ( bit != b_map.end( ) )
		return bit->second;

	return NULL;
}

lsd::bridge *lsd::object::search_bridge( const char *lab, bool no_error )
{
	objattr *at = attr->cont->sim->oa.search( lab );
	if ( at != NULL )
		return search_bridge( at, no_error );

	return NULL;
}


/*************************************************************
 SEARCH (*)
 Search the first Object lab in the branch of the
 model below this.
 Uses the fast bridge look-up map.
 *************************************************************/
lsd::object *lsd::object::search( objattr *at, bool no_search, bool no_search_up )
{
	bridge *cb;
	object *cur;

	// the current object?
	if ( attr == at )
		return this;

	// Search among the descendants of current object
	auto bit = b_map.find( at );
	if ( bit != b_map.end( ) )
		return bit->second->head;

	// stop if search is disabled
	if ( no_search )
		return NULL;

	// Search among descendants' descendants
	for ( cb = b; cb != NULL; cb = cb->next )
	{
		if ( cb->head != NULL )
			cur = cb->head->search( at, false, true );
		else
			cur = NULL;

		if ( cur != NULL )
			return cur;
	}

	// search in the entire tree if enabled
	if ( ! no_search_up && up != NULL )
	{
		cur = up->search( at, false, false );
		return cur;
	}

	return NULL;
}

lsd::object *lsd::object::search( const char *lab, bool no_search, bool no_search_up )
{
	objattr *at = attr->cont->sim->oa.search( lab );
	if ( at != NULL )
		return search( at, no_search, no_search_up );

	return NULL;
}


/*************************************************************
 SEARCH_ERR
 *************************************************************/
lsd::object *lsd::object::search_err( const char *lab, bool no_search, bool no_search_up, const char *errmsg )
{
	object *cur, *cur1;

	cur = search( lab, no_search, no_search_up );
	if ( cur == NULL )
	{	// check if it is a zero-instance object
		simulation *sim = attr->cont->sim;
		cur = sim->blueprint->search( lab );

		if ( ! sim->no_zero_instance && cur != NULL )// zero instance allowed?
			return NULL;						// deleted instance but NULL is ok

		// check if object exists somewhere
		cur1 = sim->root->search( lab );

		if ( cur1 == NULL )		// doesn't exist in current tree
			if ( cur == NULL )	// never existed
				sim->error_hard( "object not found",
								 "create object in model structure",
								 false,
								 "object '%s' is missing for %s", lab, errmsg );
			else 				// exists only in blueprint but no zero instance
				sim->error_hard( "last object instance deleted",
								 "check your equation code to ensure at least one instance\nof any object is kept or use command USE_ZERO_INSTANCE",
								 true,
								 "all instances of '%s' were deleted", lab );
		else		// exits in current tree but not (directly) below
			sim->error_hard( "object is not a descending object",
							 "move object in model structure, or specify a parent object",
							 false,
							 "object '%s' not%s under '%s' for %s%s",
							 lab, no_search ? " directly" : "", attr->label,
							 errmsg, no_search ? "\n(NO_SEARCH enabled!)" : "" );
	}

	return cur;
}


/*************************************************************
 INITTURBO (*)
 Generate the map required to use the turbosearch.
 lab must be the label of the descending object
 whose set is to be organized
 num is not used (legacy code compatibility)
 *************************************************************/
double lsd::object::initturbo( const char *lab, double tot )
{
	return initturbo( lab );
}

double lsd::object::initturbo( const char *lab )
{
	long l;
	bridge *cb;
	object *cur;

	cb = search_bridge( lab, true );
	if ( cb == NULL )
	{
		attr->cont->sim->error_hard( "object not found",
									 "create object in model structure",
									 false,
									 "object '%s' is missing for turbo search",
									 lab );
		return 0;
	}

	if ( cb->head == NULL )
	{
		attr->cont->sim->error_hard( "object has no instance",
									 "check your equation code to prevent this situation",
									 true,
									 "failure when initializing object '%s' for turbo search",
									 lab );
		return 0;
	}

	// prevent concurrent initialization by more than one thread
	l_guardT lock( obj_comp_lck );

	cb->t_map.clear( );

	// fill the map with the object positions
	for ( l = 1, cur = search( cb->attr ); cur != NULL; ++l, cur = BROTHER( cur ) )
		cb->t_map.insert( n_pairT( l, cur ) );

	return ( double ) cb->t_map.size( );
}


/*************************************************************
 TURBOSET (*)
 Check if the turbosearch for object lab was set with
 initturbo and is still valid. Object instance creation and
 destruction destroy the turbosearch map.
 returns 0 if there is no map or the number of objects in map.
 *************************************************************/
double lsd::object::turboset( const char *lab )
{
	bridge *cb;

	cb = search_bridge( lab, true );
	if ( cb == NULL )
	{
		attr->cont->sim->error_hard( "object not found",
									 "check your equation code to prevent this situation",
									 true,
									 "cannot find turbo search object '%s'",
									 lab );
		return 0;
	}

	return ( double ) cb->t_map.size( );
}


/*************************************************************
 TURBOSEARCH (*)
 Search the object lab placed in num position.
 This search requires the map previously created with
 'initturbo'. tot is ignored (legacy code compatibility)
 *************************************************************/
lsd::object *lsd::object::turbosearch( const char *lab, double tot, double num )
{
	return turbosearch( lab, num );
}

lsd::object *lsd::object::turbosearch( const char *lab, double num )
{
	bridge *cb;

	cb = search_bridge( lab, true );
	if ( cb == NULL )
	{
		attr->cont->sim->error_hard( "object not found",
									 "check your equation code to prevent this situation",
									 true,
									 "failure when turbo searching object '%s'",
									 lab );
		return NULL;
	}

	if ( cb->t_map.size( ) == 0 )
	{
		attr->cont->sim->error_hard( "invalid search operation",
									 "check your equation code to prevent this situation",
									 true,
									 "object '%s' is not initialized for turbo search",
									 lab );
		return NULL;
	}

	// find the object in position
	auto nit = cb->t_map.find( ( long ) floor ( num ) );
	if ( nit != cb->t_map.end( ) )
		return nit->second;
	else
		return NULL;
}


/*************************************************************
 SEARCH_INST (*)
 Searches the model for an object instance pointed by 'obj'
 searching first among the calling object instances and then
 into its descendants, returning the instance number or 0 if
 not found
 *************************************************************/
void lsd::object::search_inst( object *obj, long *pos, long *checked )
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
			if ( attr->cont->sim->no_ptr_chk || strcmp( cur->attr->label, obj->attr->label ) )
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

double lsd::object::search_inst( object *obj, bool fun )
{
	long pos, checked;
	object *cur;
	simulation *sim = attr->cont->sim;

	if ( obj == NULL )					// default is self
		obj = this;

	// if pointer check available quickly check for non-existing objects
	if ( obj != this && ! sim->no_ptr_chk )
	{
		if ( sim->obj_list.find( obj ) == sim->obj_list.end( ) )
			return 0;

		cur = obj;
	}
	else
		cur = this;

	if ( cur->up != NULL )				// not root?
		// get first instance of found/current object brotherhood
		cur = cur->up->search_bridge( cur->attr )->head;

	pos = 0;
	checked = fun ? -1 : 0;
	if ( cur != NULL )
		cur->search_inst( obj, &pos, &checked );// check for instance recursively

	return pos;
}


/*************************************************************
 SEARCH_VAR
 Explore the model starting from this and
 gradually extending till considering the whole model.
 It searches for an object having a variable whose attribute
 is at or label is lab and returns the first found.
 The research strategy used by this method is simple:
 1) search among the variables of this. If not found:
 2) search among the variables of the descending objects.
    If not found:
 3) search among the variables of parent object. If not found
    return NULL

 Each object encountered during a search perform the same
 search strategy. The strategy ensures that the whole model
 is searched, hence always returns a value, provided that
 variable l exists. The problem is to be sure that, in case
 of multiple instances of variable l, the correct one is
 returned. This depends on the right choice of "this", that
 is, where the search is starting from.
 The field caller is used to avoid deadlocks when from
 descendants the search goes up again, or from the parent down.
 Uses the fast variable look-up map of the searched variables.
 *************************************************************/
lsd::variable *lsd::object::search_var( object *caller, varattr *at, bool no_error, bool no_search, bool no_search_up, bool search_sons )
{
	bridge *cb;
	variable *cv;

	// Search among the variables of current object
	auto vit = v_map.find( at );
	if ( vit != v_map.end( ) )
		return vit->second;

	// stop if search is disabled except if direct sons must still be searched
	if ( no_search && ! search_sons )
		return NULL;

	// Search among descendants
	for ( cb = b, cv = NULL; cb != NULL; cb = cb->next )
	{
		// search down only if one instance exists and the label is different from caller
		if ( cb->head != NULL && ( caller == NULL || cb->head->attr != caller->attr ) )
		{
			cv = cb->head->search_var( this, at, no_error, no_search, true, false );
			if ( cv != NULL )
				return cv;
		}
		else
			cv = NULL;
	}

	// stop if search is disabled
	if ( no_search || no_search_up )
		return NULL;

	// search up in the tree
	if ( up != caller )
	{
		if ( up == NULL )
		{
			if ( ! no_error )
				attr->cont->sim->error_hard( "variable or parameter not found",
											 "create variable or parameter in model structure",
											 false,
											 "element '%s' is missing",
											 at->label );
			return NULL;
		}

		cv = up->search_var( this, at, no_error, false, false, false );
	}

	return cv;
}

lsd::variable *lsd::object::search_var( object *caller, const char *lab, bool no_error, bool no_search, bool no_search_up, bool search_sons )
{
	varattr *at = attr->cont->sim->va.search( lab );
	if ( at != NULL )
		return search_var( caller, at, no_error, no_search, no_search_up, search_sons );

	return NULL;
}


/*************************************************************
 SEARCH_VAR_ERR
 *************************************************************/
lsd::variable *lsd::object::search_var_err( object *caller, const char *lab, bool no_search, bool no_search_up, bool search_sons, const char *errmsg )
{
	object *cur;
	variable *cv, *cv1;

	cv = search_var( caller, lab, true, no_search, no_search_up, search_sons );
	if ( cv == NULL )
	{	// check if it is a zero-instance object
		simulation *sim = attr->cont->sim;
		cur = sim->blueprint->search( attr );
		if ( cur != NULL )
			cv = cur->search_var( NULL, lab, true, no_search, no_search_up, search_sons );

		if ( ! sim->no_zero_instance && cv != NULL )// zero instance allowed?
			return NULL;						// deleted instance but NULL is ok

		// check if variable exists somewhere
		cv1 = sim->root->search_var( NULL, lab, true );

		if ( cv1 == NULL )		// doesn't exist in current tree
			if ( cv == NULL )	// never existed
				sim->error_hard( "variable or parameter not found",
								 "create variable or parameter in model structure",
								 false,
								 "element '%s' is missing for %s",
								 lab, errmsg );
			else 				// exists only in blueprint
				sim->error_hard( "last object instance deleted",
								 "check your equation code to ensure at least one instance\nof any object is kept or use command USE_ZERO_INSTANCE",
								 true,
								 "all instances of the object containing '%s' were deleted",
								 lab );
		else		// exits in current tree but not (directly) below
			sim->error_hard( "variable or parameter not in a descending object",
							 "move object in model structure, or specify a parent object",
							 false,
							 "'%s' in '%s' not%s under '%s' for %s%s",
							 lab, cv1->up != NULL ? cv1->up->attr->label : "?",
							 no_search ? " directly" : "", attr->label, errmsg,
							 no_search ? "\n(NO_SEARCH enabled!)" : "" );
	}

	return cv;
}


/*************************************************************
 SEARCH_VAR_COND (*)
 Search for the Variable or Parameter lab with
 value value and return it, if found.
 Normally searches all branches of the object
 containing the variable, except if the NO_SEARCH
 command is issued before the macro, when it only
 search the current branch of the model.
 Return NULL if not found.
 *************************************************************/
lsd::object *lsd::object::search_var_cond( const char *lab, double value, int lag )
{
	double res;
	object *cur, *cnext;
	simulation *sim = attr->cont->sim;
	variable *cv;

	cv = search_var_err( this, lab, sim->no_search, sim->no_search_up, true, "conditional searching" );
	if ( cv == NULL )
		return NULL;

	for ( cur = cv->up; cur != NULL; cur = cnext )
	{
		cnext = sim->no_search ? cur->next : cur->hyper_next( );	// allow object suicide

		res = cur->cal( lab, lag );
		if ( res == value )
			return cur;
	}

	return NULL;
}


/*************************************************************
 INITTURBO_COND (*)
 Generate the data structure required
 to use the turbosearch with condition.
 *************************************************************/
double lsd::object::initturbo_cond( const char *lab )
{
	bridge *cb;
	object *cur, *cnext;
	variable *cv;

	cv = search_var_err( this, lab, attr->cont->sim->no_search, attr->cont->sim->no_search_up, true, "turbo conditional searching" );
	if ( cv == NULL )
		return 0;

	if ( cv->up->up == NULL )				// variable at root level?
	{
		attr->cont->sim->error_hard( "invalid variable or parameter for turbo search",
									 "check your model structure to prevent this situation",
									 false,
									 "element '%s' is at root level (always single-instanced)",
									 lab );
		return 0;
	}

	// find the bridge which contains the object containing the variable
	auto bit = cv->up->up->b_map.find( cv->up->attr );
	if ( bit == cv->up->up->b_map.end( ) )
	{
		attr->cont->sim->error_hard( "internal problem in LSD",
									 "if error persists, please contact developers",
									 true,
									 "invalid data structure (bridge not found)" );
		return 0;
	}

	// prevent concurrent initialization by more than one thread
	l_guardT lock( obj_comp_lck );

	cb = bit->second;
	cb->o_map.clear( );						// remove any existing mapping
	delete [ ] cb->search_var;

	// fill the map with the object values
	for ( cur = cb->head; cur != NULL; cur = cnext )
	{
		cnext = cur->next;					// allow object suicide
		cb->o_map.insert( o_pairT ( cur->cal( lab, 0 ), cur ) );
	}

	// register the name of variable for which the map is set
	cb->search_var = new char [ strlen( lab ) + 1 ];
	strcpy( cb->search_var, lab );

	return ( double ) cb->o_map.size( );
}


/*************************************************************
 TURBOSET_COND (*)
 Check if the turbosearch for object lab with a condition
 was set with initturbo_cond on on variable lab and is still
 valid. Object instance creation and destruction destroy the
 turbosearch map.
 returns 0 if there is no map or the number of nodes in map.
 *************************************************************/
double lsd::object::turboset_cond( const char *lab )
{
	variable *cv;

	cv = search_var_err( this, lab, attr->cont->sim->no_search, attr->cont->sim->no_search_up, true, "turbo conditional searching" );
	if ( cv == NULL )
		return 0;

	if ( cv->up->up == NULL )				// variable at root level?
	{
		attr->cont->sim->error_hard( "invalid variable or parameter for turbo search",
									 "check your model structure to prevent this situation",
									 false,
									 "element '%s' is at root level (always single-instanced)",
									 lab );
		return 0;
	}

	// find the bridge which contains the object containing the variable
	auto bit = cv->up->up->b_map.find( cv->up->attr );
	if ( bit == cv->up->up->b_map.end( ) )
	{
		attr->cont->sim->error_hard( "internal problem in LSD",
									 "if error persists, please contact developers",
									 true,
									 "invalid data structure (bridge not found)" );
		return 0;
	}

	return ( double ) bit->second->o_map.size( );
}


/*************************************************************
 TURBOSEARCH_COND (*)
 Search the object instance containing a variable
 label with given value.
 Return the containing object instance, if found,
 or NULL if not found.
 This search exploits the structure created with
 'initturbo_cond'.
 *************************************************************/
lsd::object *lsd::object::turbosearch_cond( const char *lab, double value )
{
	bridge *cb;
	variable *cv;

	cv = search_var_err( this, lab, attr->cont->sim->no_search, attr->cont->sim->no_search_up, true, "turbo conditional searching" );
	if ( cv == NULL )
		return NULL;

	if ( cv->up->up == NULL )				// variable at root level?
	{
		attr->cont->sim->error_hard( "invalid variable or parameter for turbo search",
									 "check your model structure to prevent this situation",
									 false,
									 "element '%s' is at root level (always single-instanced)",
									 lab );
		return NULL;
	}

	// find the bridge which contains the object containing the variable
	auto bit = cv->up->up->b_map.find( cv->up->attr );
	if ( bit == cv->up->up->b_map.end( ) )
	{
		attr->cont->sim->error_hard( "internal problem in LSD",
									 "if error persists, please contact developers",
									 true,
									 "invalid data structure (bridge not found)" );
		return NULL;
	}

	cb = bit->second;

	if ( cb->o_map.size( ) == 0 || cb->search_var == NULL || strcmp( cb->search_var, lab ) )
	{
		attr->cont->sim->error_hard( "invalid search operation",
									 "check your equation code to prevent this situation",
									 true,
									 "element '%s' is not initialized for turbo conditional search",
									 lab );
		return NULL;
	}

	// find the object containing the variable
	auto oit = cb->o_map.find( value );
	if ( oit != cb->o_map.end( ) )
		return oit->second;
	else
		return NULL;
}


/*************************************************************
 ADD_VAR
 Add a new (empty) element, used in the creation
 of the model structure
 *************************************************************/
lsd::variable *lsd::object::add_var( const char *lab, int par, int lags, bool plot, char deb )
{
	variable *cv;

	if ( search_var( this, lab, true, true ) != NULL || search( lab ) != NULL )
	{
		attr->cont->sim->plog( "\nWarning: duplicated element name '%s', please rename", lab );
		cmd( "ttk::messageBox -parent . -title Error -icon error -type ok -message \"Duplicated element name\" -detail \"Element '%s' name is already used in the model. Please rename it.\n\nThe names of objects, variables, parameters, and functions must be unique.\"", lab );
		return NULL;
	}

#ifndef _TERM_
	if ( ! valid_label( lab ) )
	{
		attr->cont->sim->plog( "\nWarning: invalid element name '%s', please rename", lab );
		cmd( "ttk::messageBox -parent . -title Error -icon error -type ok -message \"Invalid characters in element name\" -detail \"Element '%s' has an invalid name. Please rename it.\n\nNames must begin with a letter (English alphabet) or underscore ('_') and may contain letters, numbers or '_' but no spaces or other characters.\"", lab );
		return NULL;
	}
#endif

	if ( par < 0 || par > 2 )
	{
		attr->cont->sim->error_hard( "internal problem in LSD",
									 "if error persists, please contact developers",
									 true,
									 "invalid element type %d",
									 par );
		return NULL;
	}

	if ( v == NULL )
		cv = v = new variable ( this, lab, par, lags, plot, deb );
	else
	{
		for ( cv = v; cv->next != NULL; cv = cv->next );
		cv->next = new variable ( this, lab, par, lags, plot, deb );
		cv = cv->next;
	}

	v_map.insert( v_pairT ( cv->attr, cv ) );

	return cv;
}


/*************************************************************
 CHECK_LABEL
 Control that the label lab does not already exist
 in the model. Also prevents invalid characters in
 the names.
 *************************************************************/
int lsd::object::check_label( const char *lab )
{
	object *cur;

	if ( ! valid_label( lab ) )
		return 2;				// invalid characters (incl. spaces)

	if ( ! strcmp( lab, attr->label ) )
		return 1;

	for ( auto cv = v; cv != NULL; cv = cv->next )
		if ( ! strcmp( lab, cv->attr->label ) )
			return 1;

	for ( auto cb = b; cb != NULL; cb = cb->next )
	{
		if ( cb->head == NULL )
			cur = attr->cont->sim->blueprint->search( cb->attr );
		else
			cur = cb->head;

		if ( cur->check_label( lab ) )
			return 1;
	}

	return 0;
}


/*************************************************************
 ADD_VAR
 Add a new element instance identical to the example
 *************************************************************/
lsd::variable *lsd::object::add_var( variable *example )
{
	variable *cv;

	if ( search_var( this, example->attr, true, true ) != NULL )
	{
		attr->cont->sim->error_hard( "variable or parameter not added",
									 "choose an unique name for the element",
									 true,
									 "element '%s' already exists in object '%s'",
									 example->attr->label, attr->label );
		return NULL;
	}

	if ( v == NULL )
		cv = v = new variable ( *example );
	else
	{
		for ( cv = v; cv->next != NULL; cv = cv->next );
		cv->next = new variable ( *example );
		cv = cv->next;
	}

	cv->up = this;
	v_map.insert( v_pairT ( cv->attr, cv ) );

	return cv;
}


/*************************************************************
 ADD_OBJ
 Add num new sons with label lab, to ANY object like
 this one if propagate = true, wherever is on the
 tree
 *************************************************************/
lsd::object *lsd::object::add_obj( const char *lab, int num, bool propagate, int prng_type, bool to_compute, bool blueprint )
{
	int i;
	bridge *cb;
	object *cur, *cur1, *cur2 = NULL;

	if ( search( lab ) != NULL )
	{
		attr->cont->sim->error_hard( "object not added",
									 "choose an unique name for the object",
									 true,
									 "an object named '%s' already exists in the model",
									 lab );
		return NULL;
	}

	if ( search_var( NULL, lab, true ) != NULL )
	{
		attr->cont->sim->error_hard( "object not added",
									 "choose an unique name for the object",
									 true,
									 "an element named '%s' already exists in the model",
									 lab );
		return NULL;
	}

#ifndef _TERM_
	if ( ! valid_label( lab ) )
	{
		attr->cont->sim->plog( "\nWarning: invalid object name '%s', please rename", lab );
		cmd( "ttk::messageBox -parent . -title Warning -icon warning -type ok -message \"Invalid characters in object name\" -detail \"Object '%s' has an invalid name. Please rename it to prevent problems.\n\nNames must begin with a letter (English alphabet) or underscore ('_') and may contain letters, numbers or '_' but no spaces or other characters.\"", lab );
	}
#endif

	if ( num < 1 )
		return NULL;

	for ( cur = this; cur != NULL; propagate ? cur = cur->hyper_next( ) : cur = NULL )
	{
		// create bridge
		if ( cur->b == NULL )
			cb = cur->b = new bridge ( NULL );
		else
		{
			for ( cb = cur->b; cb->next != NULL; cb = cb->next );
			cb->next = new bridge ( NULL );
			cb = cb->next;
		}

		// create non-configured object instances
		for ( i = 0; i < num; ++i )
			if ( i == 0 )
				cur1 = cur2 = cb->head = new object ( cur, lab, prng_type, to_compute, blueprint );
			else
				cur1 = cur1->next = new object ( cur, lab, prng_type, to_compute, blueprint );

		cb->attr = cur1->attr;
		cur->b_map.insert( b_pairT ( cb->attr, cb ) );
	}

	return cur2;
}


/*************************************************************
 MOVE
 Move object in the model structure. The object
 is placed below the provided dest object
 *************************************************************/
void lsd::object::move( const char *dest )
{
	bridge *cb, *cb1, *mb = NULL, *nb;
	object *cur, *cur1, *d, *o, *s;
	variable *cv;

	o = attr->cont->sim->root->search( attr );	// pick first model instances
	d = attr->cont->sim->root->search( dest );

	if ( o == NULL || d == NULL || o->search( dest ) != NULL )
	{
		attr->cont->sim->error_hard( "missing/invalid source or destination object",
									 "choose valid object names\nand non-nested destination",
									 true,
									 "cannot move object '%s'",
									 attr->label );
		return;
	}

	// move bridges from source parent instances to destination parent instances
	s = o->up;
	while ( s != NULL || d != NULL )
	{
		if ( s != NULL )
		{
			// find bridge to object being copied in source parent
			for ( cb1 = NULL, cb = s->b; cb != NULL && cb->attr != attr; cb1 = cb, cb = cb->next );

			// remove from the source parent's bridge linked list
			if ( cb1 == NULL )	// head of list?
				s->b = cb->next;
			else
				cb1->next = cb->next;

			mb = cb;
			mb->next = NULL;	// moved object bridge enters at the end of the new parent list

			s->b_map.erase( attr );			// update speedup maps
			s = s->hyper_next( );			// next source parent
		}
		else	// handle the case last object instance has to be cloned to fill
		{		// additional instances of destination parent
			// clone last object bridge to insert it on unmatched destination parents
			nb = new bridge ( attr );

			// clone object instances and add them to the cloned bridge
			for ( cur = mb->head, cur1 = NULL; cur != NULL; cur = cur->next )
			{
				// update linked list of object instances in bridge
				if ( cur1 == NULL )
					cur1 = nb->head = new object ( d, attr->label, cur->prng_type, cur->to_compute );
				else
					cur1 = cur1->next = new object ( d, attr->label, cur->prng_type, cur->to_compute );

				for ( cv = cur->v; cv != NULL; cv = cv->next )
					cur1->add_var( cv );

				cur->copy_descendant( cur1 );
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

			d->b_map.insert( b_pairT ( attr, mb ) );	// update speedup maps
			d = d->hyper_next( );						// next destination parent
		}
		else	// handle the case last object instances in source parent must be
				// deleted because there are no more instances in destination
			delete mb;		// delete unmatched object instances and the bridge
	}
}


/*************************************************************
 REPLICATE
 *************************************************************/
void lsd::object::replicate( int num, bool propagate )
{
	object *cur, *cur1;
	variable *cv;
	int i, usl;

	if ( propagate )
		cur = hyper_next( attr->label );
	else
		cur = NULL;

	if ( cur != NULL )
		cur->replicate( num, true );

	next_count( this, & usl );
	for ( cur = this, i = 1; i < usl; cur = cur->next, ++i );

	for ( i = usl; i < num; ++i )
	{
		cur1 = cur->next;
		cur->next = new object ( up, attr->label, prng_type, to_compute );
		cur->next->next = cur1;

		cur1 = cur->next;
		for ( cv = v; cv != NULL; cv = cv->next )
			cur1->add_var( cv );

		copy_descendant( cur1 );
	}
}


/*************************************************************
 COPY_DESCENDANT
 *************************************************************/
void lsd::object::copy_descendant( object *to )
{
	bridge *cb, *cb1;
	object *cur;
	simulation *sim = attr->cont->sim;
	variable *cv;

	if ( b == NULL )
	{
		to->b = NULL;
		return;
	}

	// create the first bridge
	to->b = new bridge ( b->attr );

	// add bridge to new object lookup map
	to->b_map.insert( b_pairT ( to->b->attr, to->b ) );

	// create the first (head) object
	if ( b->head == NULL )
		cur = sim->blueprint->search( b->attr );
	else
		cur = b->head;

	to->b->head = new object ( to, cur->attr->label, cur->prng_type, cur->to_compute );

	// copy variables of head object
	for ( cv = cur->v; cv != NULL; cv = cv->next )
		to->b->head->add_var( cv );

	// copy head descendants
	cur->copy_descendant( to->b->head );

	// create following bridges
	for ( cb = to->b, cb1 = b->next; cb1 != NULL; cb1 = cb1->next )
	{
		cb->next = new bridge ( cb1->attr );
		cb = cb->next;
		to->b_map.insert( b_pairT ( cb1->attr, cb ) );

		if ( cb1->head == NULL )
			cur = sim->blueprint->search( cb1->attr );
		else
			cur = cb1->head;

		cb->head = new object ( to, cur->attr->label, cur->prng_type, cur->to_compute );

		for ( cv = cur->v; cv != NULL; cv = cv->next )
			cb->head->add_var( cv );

		cur->copy_descendant( cb->head );
	}
}


/*************************************************************
 ADD_N_OBJECTS2 (*)
 As the type with the example, but the example is
 taken from the blueprint
 In respect of the original version, it allows for
 the specification of the time of last update if
 t_update is positive or zero. If t_update is
 negative (<0) it takes the time of last update from
 the example object (if >0) or current t (if =0)
 *************************************************************/
lsd::object *lsd::object::add_n_objects2( const char *lab, int n, int t_update )
{
	return add_n_objects2( lab, n, attr->cont->sim->blueprint->search( lab ), t_update );
}

lsd::object *lsd::object::add_n_objects2( const char *lab, int n, object *ex, int t_update )
{
	bool net;
	int i;
	bridge *cb, *cb1, *cb2;
	object *cur, *cur1, *last, *first = NULL;
	simulation *sim = attr->cont->sim;
	variable *cv;

	// check the labels and prepare the bridge to attach to
	for ( cb2 = b; cb2 != NULL && strcmp( cb2->attr->label, lab ); cb2 = cb2->next );

	if ( cb2 == NULL )
	{
		sim->error_hard( "object not found",
						 "create son object in model structure",
						 false,
						 "object '%s' contains no son object '%s' for adding instance(s)",
						 attr->label, lab );
		return NULL;
	}

	if ( ex == NULL || strcmp( ex->attr->label, lab ) )
	{
		sim->error_hard( "invalid example object",
						 "check your equation code to prevent this situation",
						 true,
						 "bad example pointer when adding object '%s'",
						 lab );
		return NULL;
	}

	// prevent concurrent additions by more than one thread
	l_guardT lock( obj_comp_lck );

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
		// create a new copy of the object, with proper attributes
		cur = new object ( this, lab, ex->prng_type, ex->to_compute );

		// set new PRNG new seed
		if ( cur->prng_type > 0 )
			cur->set_rnd_seed( attr->cont->sim->seeder( ) );

		if ( net )						// if objects are nodes in a network
			cur->node = new netnode( this );// insert new nodes in network (as isolated nodes)

		// create its variables and initialize them
		for ( cv = ex->v; cv != NULL; cv = cv->next )
		  cur->add_var( cv );

		for ( cv = cur->v; cv != NULL; cv = cv->next )
		{
			// prevent concurrent use by more than one thread
			rec_lguardT lock( cv->var_comp_lck );

			if ( sim->running && cv->param != 1 )
			{
				if ( t_update < 0 && cv->last_update == 0 )
					cv->last_update = sim->t;
				else
				{
					if ( t_update >= 0 && t_update < cv->last_update && sim->t > 1 )
					{
						sim->error_hard( "cannot add object",
										 "check your equation code to prevent this situation",
										 true,
										 "invalid update time step (%d) to set object '%s'\nvariable '%s' was updated later (%d)",
										 t_update, lab, cv->attr->label, cv->last_update );
						return NULL;
					}

					if ( t_update >= 0 )
						cv->last_update = t_update;
				}

				// choose next update step for special updating variables
				if ( cv->attr->delay > 0 || cv->attr->delay_range > 0 )
				{
					cv->next_update = cv->last_update + cv->attr->delay;
					if ( cv->attr->delay_range > 0 )
						cv->next_update += ( int ) rnd_uniform_int( 0, cv->attr->delay_range );
				}
			}

			if ( cv->attr->save || cv->attr->savei )
				cv->alloc_save_var( );
		}

		// insert the descending objects in the newly created objects
		for ( cb1 = NULL, cb = ex->b; cb != NULL; cb = cb->next )
		{
			if ( cb1 == NULL )
				cb1 = cur->b = new bridge ( cb->attr );
			else
				cb1 = cb1->next = new bridge ( cb->attr );

			// add bridge to new object lookup map
			cur->b_map.insert( b_pairT ( cb->attr, cb1 ) );

			for ( cur1 = cb->head; cur1 != NULL; cur1 = cur1->next )
				cur->add_n_objects2( cur1->attr->label, 1, cur1, t_update );
		}

		// destroy invalidated turbosearch trees
		cb2->t_map.clear( );
		cb2->o_map.clear( );
		delete [ ] cb2->search_var;
		cb2->search_var = NULL;

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
		if ( ! sim->no_ptr_chk )
		{
			// prevent concurrent update by more than one thread
			l_guardT lock( sim->obj_list_lck );

			sim->obj_list.insert( cur );
		}
	}

	return first;
}


/*************************************************************
 DELETE_OBJ (*)
 Remove the object from the model
 Before killing the Variables data to be saved are stored
 in the "cemetery", a linked chain storing data to be analyzed.
 *************************************************************/
void lsd::object::delete_obj( const variable *caller )
{
	bridge *cb;
	object *cur = this;
	simulation *sim = attr->cont->sim;

	if ( cur == NULL )
		return;					// ignore deleting null object

	{							// create context for lock
		// prevent concurrent deletion by more than one thread
		l_guardT lock( obj_comp_lck );

		if ( deleting )			// ignore if deleting already going on
			return;

		if ( under_computation( ) )
		{
			if ( sim->wait_delete != NULL && sim->wait_delete != this )
			{
				sim->error_hard( "deletion already pending",
								 "check your equation code to prevent deleting objects recursively",
								 true,
								 "cannot schedule the deletion of object '%s'",
								 attr->label );
				return;
			}
			else
			{
				sim->wait_delete = this;
				return;
			}
		}

		deleting = true;		// signal deletion to other threads

		if ( sim->wait_delete == this )
			sim->wait_delete = NULL;// finally deleting pending object
	}

	// update object list for user pointer checking
	if ( ! sim->no_ptr_chk )
	{
		// prevent concurrent update by more than one thread
		l_guardT lock( sim->obj_list_lck );

		sim->obj_list.erase( this );
	}

	// collect required variables BEFORE removing instances (bridge)
	collect_cemetery( caller );

	// find the bridge
	if ( up != NULL )
		cb = up->search_bridge( attr );
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
				if ( sim->no_zero_instance )
				{
					sim->error_hard( "last object instance deleted",
									 "check your equation code to ensure at least one instance\nof any object is kept",
									 true,
									 "cannot delete all instances of '%s'",
									 attr->label );
					return;
				}

				cb->head = NULL;
				sim->save_ok = false;// model structure can no longer be saved
			}
		}
		else
		{
			for ( cur = cb->head; cur->next != this; cur = cur->next );
			cur->next = next;
		}

		cb->counter_updated = false;

		// destroy invalidated turbosearch trees
		cb->t_map.clear( );
		cb->o_map.clear( );
		delete [ ] cb->search_var;
		cb->search_var = NULL;
	}

	if ( del_flag != NULL )
		*del_flag = true;		// flag deletion to caller, if requested

	next = NULL;				// try to prevent improper CYCLE to continue

	delete this;				// delete (suicide) now
}


/*************************************************************
 COLLECT_CEMETERY
 Processes variables from an object required to go to cemetery
 Also destroy variables not requiring saving
 *************************************************************/
void lsd::object::collect_cemetery( const variable *caller )
{
	simulation *sim = attr->cont->sim;
	variable *cv, *cv1;

	for ( cv = v; cv != NULL; cv = cv1 )	// scan all variables
	{
		cv1 = cv->next;						// pointer to next variable

		// need to save?
		if ( ( cv->attr->save == true || cv->attr->savei == true ) && sim->running && sim->eff_t > 0 && sim->eff_t <= cv->end && sim->quit != 2 )
		{
			cv->set_lab_tit( );				// update last lab_tit
			cv->data[ sim->eff_t - cv->start ] = cv->val[ 0 ];// define last value

			if ( cv->attr->savei )
				cv->save_single( );			// update file

			if ( cv->end > sim->eff_t )		// remove unused store positions
			{
				cv->end = sim->eff_t;

				// use C stdlib to be able to deallocate memory for deleted objects
				cv->data = ( double * ) realloc( cv->data, ( cv->end - cv->start + 1 ) * sizeof( double ) );
			}

			cv->add_cemetery( );			// transfer to cemetery
		}
		else
			cv->delete_var( caller == NULL || cv == caller );// disable lock if emptying caller
	}

	v = NULL;
	v_map.clear( );
}


/*************************************************************
 ADD_CEMETERY
 Store the variable in a list of variables in
 objects deleted but to be used for analysis.
 *************************************************************/
void lsd::variable::add_cemetery( void )
{
	if ( attr->cont->sim->cemetery == NULL )
		attr->cont->sim->cemetery = attr->cont->sim->last_cemetery = this;
	else
	{
		attr->cont->sim->last_cemetery->next = this;
		attr->cont->sim->last_cemetery = this;
	}

	attr->cont->sim->last_cemetery->next = NULL;
	up = NULL;								// remove parent
}


/*************************************************************
 EMPTY_CEMETERY
 *************************************************************/
void lsd::simulation::empty_cemetery( void )
{
	variable *cv, *cv1;

	for ( cv = cemetery; cv !=NULL; cv = cv1 )
	{
		cv1 = cv->next;
		cv->delete_var( );
	}

	cemetery = last_cemetery = NULL;
}


/*************************************************************
 TO_DELETE (*)
 Check if the object is scheduled for deletion
 Objects are only deleted when all variables
 under computation in it finish computation
 *************************************************************/
double lsd::object::to_delete( void )
{
	return attr->cont->sim->wait_delete == this;
}


/*************************************************************
 DELETE_BRIDGE
 Remove a bridge, used when an object is removed from the model
 *************************************************************/
void lsd::object::delete_bridge( void )
{
	bridge *cb = NULL, *cb1;

	if ( up->b == NULL )
		return;

	if ( up->b->head == this )
	{	// first bridge in the bridge chain
		cb = up->b;
		up->b = up->b->next;
	}
	else	// find position in bridge chain (not first)
		for ( cb = up->b, cb1 = NULL; cb != NULL; cb1 = cb, cb = cb->next )
			if ( cb->head == this && cb1 != NULL )
			{
				cb1->next = cb->next;			// previous bridge points to next
				break;
			}

	if ( cb != NULL )
	{
		up->b_map.erase( cb->attr );
		delete cb;
	}
}


/*************************************************************
 DELETE_VAR
 Remove the variable from the object
 *************************************************************/
void lsd::object::delete_var( const char *lab )
{
	variable *cv, *cv1;

	if ( strcmp( v->attr->label, lab ) == 0 )
	{	// first variable in the chain
		cv = v->next;
		v_map.erase( v->attr );
		v->delete_var( );
		v = cv;
	}
	else		// not first variable, search
		for ( cv = v; cv->next != NULL; cv = cv->next)
			if ( ! strcmp( cv->next->attr->label, lab ) )
			{
				cv1 = cv->next->next;
				v_map.erase( cv->next->attr );
				cv->next->delete_var( );
				cv->next = cv1;
				break;
			}
}


/*************************************************************
 CHG_LAB
 Change the label of the Object, for all the
 instances
 *************************************************************/
void lsd::object::chg_lab( const char *lab )
{
	attr->cont->sim->oa.rename( attr->label, lab );
}


/*************************************************************
 CHG_VAR_LAB
 Change the label of the Variable from old to new
 *************************************************************/
void lsd::object::chg_var_lab( const char *old, const char *newname )
{
	for ( auto cv = v; cv != NULL; cv = cv->next )
		if ( strcmp( cv->attr->label, old ) == 0 )
		{
			attr->cont->sim->va.rename( old, newname );
			break;
		}
}


/*************************************************************
 UNDER_COMPUTATION
 Check if any variable in or below the object is
 still under computation.
 *************************************************************/
bool lsd::object::under_computation( void )
{
	// check variables in descendants
	for ( auto cb = b; cb != NULL; cb = cb->next )
		for ( auto cur = cb->head; cur != NULL; cur = cur->next )
			if ( cur->under_computation( ) )
				return true;

	// check variables directly contained in object
	for ( auto cv = v; cv != NULL; cv = cv->next )
		if ( cv->under_computation && ! cv->attr->dummy )
			return true;

	return false;
}


/*************************************************************
 UNDER_COMPUT_VAR
 Check if a variable in object is under computation
 *************************************************************/
bool lsd::object::under_comput_var( const char *lab )
{
	variable *cv;

	cv = search_var_err( this, lab, false, false, false, "retrieving" );

	if ( cv != NULL && cv->under_computation )
		return true;

	return false;
}


/*************************************************************
 CAL (*)
 Return the value of Variable or Parameter with
 label lab with lag lag.
 The method search for the Variable starting from
 this Object and then calls the function
 variable->cal(caller, lag )
 *************************************************************/
double lsd::object::cal( object *caller, const char *lab, int lag, bool force_search )
{
	simulation *sim = attr->cont->sim;
	variable *cv;

	if ( sim->quit == 2 )
		return NAN;

	cv = search_var_err( this, lab, force_search ? false : sim->no_search, false, false, "retrieving" );
	if ( cv == NULL )
		return NAN;

	if ( lag == 0 && sim->parallel_ready && cv->attr->parallel && cv->last_update < sim->t && ! cv->attr->dummy )
		sim->parallel_update( cv, this, caller );

	return cv->cal( caller, lag );
}

double lsd::object::cal( object *caller, const char *lab, int lag )
{
	simulation *sim = attr->cont->sim;
	variable *cv;

	if ( sim->quit == 2 )
		return NAN;

	cv = search_var_err( this, lab, sim->no_search, false, false, "retrieving" );
	if ( cv == NULL )
		return NAN;

	if ( lag == 0 && sim->parallel_ready && cv->attr->parallel && cv->last_update < sim->t && ! cv->attr->dummy )
		sim->parallel_update( cv, this, caller );

	return cv->cal( caller, lag );
}

double lsd::object::cal( const char *lab, int lag )
{
	return cal( this, lab, lag );
}


/*************************************************************
 LAST_CAL (*)
 Return the last time the variable was calculated
 *************************************************************/
double lsd::object::last_cal( const char *lab )
{
	variable *cv;

	cv = search_var_err( this, lab, attr->cont->sim->no_search, false, false, "last updating" );
	if ( cv == NULL )
		return NAN;

	return cv->last_update;
}


/*************************************************************
 RECAL (*)
 Mark variable as not calculated in the current time,
 forcing recalculation if already calculated
 *************************************************************/
double lsd::object::recal( const char *lab )
{
	int i;
	double app;
	simulation *sim = attr->cont->sim;
	variable *cv;

	cv = search_var_err( this, lab, sim->no_search, false, false, "recalculating" );
	if ( cv == NULL )
		return NAN;

	// don't do anything if parameter or function or not yet computed in t
	if ( cv->param != 0 || cv->last_update < sim->t )
		return( cv->val[ 0 ] );

	app = cv->val[ 0 ];

	for ( i = 0; i < cv->attr->num_lag; ++i )		// scale up the past values
		cv->val[ i ] = cv->val[ i + 1 ];

	if ( ( cv->attr->save || cv->attr->savei ) && i + 1 <= sim->t - cv->start )
		cv->val[ i ] = cv->data[ sim->t - i - 1 - cv->start ];
	else
		cv->val[ i ] = NAN;

	cv->last_update = sim->t - 1;
	cv->next_update = sim->t;

	return app;
}


/*************************************************************
 SUM (*)
 Compute the sum of Variables or Parameters lab1
 with lag lag.
 If cond is true check if expression 'V("lab2") lop value'
 is true before adding each instance of the object.
 The sum is computed over the elements in a single
 branch of the model.
 *************************************************************/
double lsd::object::sum( const char *lab1, int lag, bool cond, const char *lab2, const char *lop, double value )
{
	int n, lopc;
	double tot;
	object *cur, *cnext;
	simulation *sim = attr->cont->sim;
	variable *cv;

	cv = search_var_err( this, lab1, sim->no_search, sim->no_search_up, true, "summing" );
	if ( cv == NULL )
		return 0;

	if ( cond )
	{
		lopc = logic_op_code( lop, "summing" );
		if ( lopc < 0 || search_var_err( this, lab2, sim->no_search, sim->no_search_up, true, "summing" ) == NULL )
			return 0;
	}
	else
		lopc = -1;

	cur = cv->up;
	if ( cur->up != NULL )
		cur = cur->up->search( cur->attr );

	for ( tot = n = 0; cur != NULL; cur = cnext )
	{
		cnext = BROTHER( cur );				// allow object suicide

		if ( ( ! cond || check_cond( cur->cal( this, lab2, lag ), lopc, value ) ) && ! cur->deleting )
		{
			tot += cur->cal( this, lab1, lag );
			++n;
		}
	}

	return tot;
}


/*************************************************************
 OVERALL_MAX (*)
 Compute the maximum of lab1, considering only the
 objects in a single branch of the model.
 If cond is true check if expression 'V("lab2") lop value'
 is true before considering each instance of the object.
 *************************************************************/
double lsd::object::overall_max( const char *lab1, int lag, bool cond, const char *lab2, const char *lop, double value )
{
	int n, lopc;
	double tot, temp;
	object *cur, *cnext;
	simulation *sim = attr->cont->sim;
	variable *cv;

	cv = search_var_err( this, lab1, sim->no_search, sim->no_search_up, true, "maximizing" );
	if ( cv == NULL )
		return NAN;

	if ( cond )
	{
		lopc = logic_op_code( lop, "maximizing" );
		if ( lopc < 0 || search_var_err( this, lab2, sim->no_search, sim->no_search_up, true, "maximizing" ) == NULL )
			return NAN;
	}
	else
		lopc = -1;

	cur = cv->up;
	if ( cur->up != NULL )
		cur = cur->up->search( cur->attr );

	for ( tot = -DBL_MAX, n = 0; cur != NULL; cur = cnext )
	{
		cnext = BROTHER( cur );				// allow object suicide

		if ( ( ! cond || check_cond( cur->cal( this, lab2, lag ), lopc, value ) ) && ! cur->deleting )
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


/*************************************************************
 OVERALL_MIN (*)
 Compute the minimum of lab1, considering only the
 objects in a single branch of the model.
 If cond is true check if expression 'V("lab2") lop value'
 is true before considering each instance of the object.
 *************************************************************/
double lsd::object::overall_min( const char *lab1, int lag, bool cond, const char *lab2, const char *lop, double value )
{
	int n, lopc;
	double tot, temp;
	object *cur, *cnext;
	simulation *sim = attr->cont->sim;
	variable *cv;

	cv = search_var_err( this, lab1, sim->no_search, sim->no_search_up, true, "minimizing" );
	if ( cv == NULL )
		return NAN;

	if ( cond )
	{
		lopc = logic_op_code( lop, "minimizing" );
		if ( lopc < 0 || search_var_err( this, lab2, sim->no_search, sim->no_search_up, true, "minimizing" ) == NULL )
			return NAN;
	}
	else
		lopc = -1;

	cur = cv->up;
	if ( cur->up != NULL )
		cur = cur->up->search( cur->attr );

	for ( tot = DBL_MAX, n = 0; cur != NULL; cur = cnext )
	{
		cnext = BROTHER( cur );				// allow object suicide

		if ( ( ! cond || check_cond( cur->cal( this, lab2, lag ), lopc, value ) ) && ! cur->deleting )
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


/*************************************************************
 MAVE (*)
 Return the moving average of Variable with label lab
 with period per and computed from lag to lag+per
 *************************************************************/
double lsd::object::mav( object *caller, const char *lab, double per, const double weight[ ], int lag )
{
	int i, maxlag;
	double sumv, sumw;
	simulation *sim = attr->cont->sim;
	variable *cv;

	if ( ( ! sim->use_nan && std::isnan( per ) ) || std::isinf( per ) || std::abs( per ) < 1 )
	{
		sim->error_hard( "invalid moving average period",
						 "check your equation code to prevent this situation",
						 true,
						 "period '%g' is invalid for moving average '%s'",
						 per, lab );
		return NAN;
	}

	cv = search_var_err( this, lab, sim->no_search, sim->no_search_up, true, "move-averaging" );
	if ( cv == NULL )
		return NAN;

	per = std::round( per );
	if ( per < 0 )
	{
		per = - per;
		maxlag = cv->attr->num_lag;
	}
	else
		maxlag = 0;

	for ( i = sumv = sumw = 0; i < per; ++i )
	{
		if ( i + lag - sim->t >= maxlag )
			break;

		if ( weight == NULL )
			sumv += cv->cal( caller, i + lag );
		else
		{
			sumv += cv->cal( caller, i + lag ) * weight[ i ];
			sumw += weight[ i ];
		}
	}

	return weight == NULL ? sumv / i : sumv / sumw;
}

double lsd::object::mav( object *caller, const char *lab, double per, int lag )
{
	return mav( caller, lab, per, NULL, lag );
}


/*************************************************************
 AVE (*)
 Compute the average of lab1.
 If cond is true check if expression 'V("lab2") lop value'
 is true before considering each instance of the object.
 *************************************************************/
double lsd::object::av( const char *lab1, int lag, bool cond, const char *lab2, const char *lop, double value )
{
	int n, lopc;
	double tot;
	object *cur, *cnext;
	simulation *sim = attr->cont->sim;
	variable *cv;

	cv = search_var_err( this, lab1, sim->no_search, sim->no_search_up, true, "averaging" );
	if ( cv == NULL )
		return NAN;

	if ( cond )
	{
		lopc = logic_op_code( lop, "averaging" );
		if ( lopc < 0 || search_var_err( this, lab2, sim->no_search, sim->no_search_up, true, "averaging" ) == NULL )
			return NAN;
	}
	else
		lopc = -1;

	cur = cv->up;
	if ( cur->up != NULL )
		cur = cur->up->search( cur->attr );

	for ( tot = n = 0; cur != NULL; cur = cnext )
	{
		cnext = BROTHER( cur );				// allow object suicide

		if ( ( ! cond || check_cond( cur->cal( this, lab2, lag ), lopc, value ) ) && ! cur->deleting )
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


/*************************************************************
 WHTAVE (*)
 Compute the weighted average (or product sum) of lab1 and lab2.
 If cond is true check if expression 'V("lab3") lop value'
 is true before considering each instance of the object.
 *************************************************************/
double lsd::object::whg_av( const char *lab1, const char *lab2, int lag, bool cond, const char *lab3, const char *lop, double value )
{
	int n, lopc;
	double tot;
	object *cur, *cnext;
	simulation *sim = attr->cont->sim;
	variable *cv;

	cv = search_var_err( this, lab1, sim->no_search, sim->no_search_up, true, "weighted averaging" );
	if ( cv == NULL )
		return 0;

	cv = search_var_err( this, lab2, sim->no_search, sim->no_search_up, true, "weighted averaging" );
	if ( cv == NULL )
		return 0;

	if ( cond )
	{
		lopc = logic_op_code( lop, "weighted averaging" );
		if ( lopc < 0 || search_var_err( this, lab3, sim->no_search, sim->no_search_up, true, "weighted averaging" ) == NULL )
			return 0;
	}
	else
		lopc = -1;

	cur = cv->up;
	if ( cur->up != NULL )
		cur = cur->up->search( cur->attr );

	for ( tot = n = 0; cur != NULL; cur = cnext )
	{
		cnext = BROTHER( cur );				// allow object suicide

		if ( ( ! cond || check_cond( cur->cal( this, lab3, lag ), lopc, value ) ) && ! cur->deleting )
		{
			tot += cur->cal( this, lab1, lag ) * cur->cal( this, lab2, lag );
			++n;
		}
	}

	return tot;
}


/*************************************************************
 MED (*)
 Compute the median of lab1.
 If cond is true check if expression 'V("lab2") lop value'
 is true before considering each instance of the object.
 *************************************************************/
double lsd::object::med( const char *lab1, int lag, bool cond, const char *lab2, const char *lop, double value )
{
	return perc( lab1, 0.5, lag, cond, lab2, lop, value );
}


/*************************************************************
 PERC (*)
 Compute the percentile p of lab1.
 If cond is true check if expression 'V("lab2") lop value'
 is true before considering each instance of the object.
 *************************************************************/
double lsd::object::perc( const char *lab1, double p, int lag, bool cond, const char *lab2, const char *lop, double value )
{
	int n, lopc, floor_x;
	double x, vx, vx1, tmp;
	object *cur, *cnext;
	simulation *sim = attr->cont->sim;
	variable *cv;
	d_vecT vals;

	if ( p < 0 || p > 1 )
	{
		sim->error_hard( "invalid value (0 <= p <= 1 required)",
						 "check your equation code to prevent this situation",
						 true,
						 "percentile '%g' is invalid", p );

		return NAN;
	}

	cv = search_var_err( this, lab1, sim->no_search, sim->no_search_up, true, "calculating percentile" );
	if ( cv == NULL )
		return NAN;

	if ( cond )
	{
		lopc = logic_op_code( lop, "calculating percentile" );
		if ( lopc < 0 || search_var_err( this, lab2, sim->no_search, sim->no_search_up, true, "calculating percentile" ) == NULL )
			return NAN;
	}
	else
		lopc = -1;

	cur = cv->up;
	if ( cur->up != NULL )
		cur = cur->up->search( cur->attr );

	// copy selected data series to vector
	for ( n = 0; cur != NULL; cur = cnext )
	{
		cnext = BROTHER( cur );				// allow object suicide

		if ( ( ! cond || check_cond( cur->cal( this, lab2, lag ), lopc, value ) ) && ! cur->deleting )
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


/*************************************************************
 SD (*)
 Compute the (population) standard deviation of lab1.
 If cond is true check if expression 'V("lab2") lop value'
 is true before considering each instance of the object.
 *************************************************************/
double lsd::object::sd( const char *lab1, int lag, bool cond, const char *lab2, const char *lop, double value )
{
	int n, lopc;
	double x, tot, tot2;
	object *cur, *cnext;
	simulation *sim = attr->cont->sim;
	variable *cv;

	cv = search_var_err( this, lab1, sim->no_search, sim->no_search_up, true, "calculating s.d." );
	if ( cv == NULL )
		return NAN;

	if ( cond )
	{
		lopc = logic_op_code( lop, "calculating s.d." );
		if ( lopc < 0 || search_var_err( this, lab2, sim->no_search, sim->no_search_up, true, "calculating s.d." ) == NULL )
			return NAN;
	}
	else
		lopc = -1;

	cur = cv->up;
	if ( cur->up != NULL )
		cur = cur->up->search( cur->attr );

	for ( tot = tot2 = n = 0; cur != NULL; cur = cnext )
	{
		cnext = BROTHER( cur );				// allow object suicide

		if ( ( ! cond || check_cond( cur->cal( this, lab2, lag ), lopc, value ) ) && ! cur->deleting )
		{
			tot += x = cur->cal( this, lab1, lag );
			tot2 += x * x;
			++n;
		}
	}

	if ( n > 0 )
		return sqrt( std::max( tot2 / n - pow( tot / n, 2 ), 0. ) );
	else
		return NAN;
}


/*************************************************************
 COUNT (*)
 Count the number of object lab1 instances below this.
 If cond is true check if expression 'V("lab2") lop value'
 is true before considering each instance of the object.
 *************************************************************/
double lsd::object::count( const char *lab1, int lag, bool cond, const char *lab2, const char *lop, double value )
{
	int n, lopc;
	object *cur, *cnext;
	simulation *sim = attr->cont->sim;

	cur = search_err( lab1, sim->no_search, sim->no_search_up, "counting" );

	if ( cur == NULL )
		return 0;

	if ( cond )
	{
		lopc = logic_op_code( lop, "counting" );
		if ( lopc < 0 || search_var_err( this, lab2, sim->no_search, sim->no_search_up, true, "counting" ) == NULL )
			return NAN;
	}
	else
		lopc = -1;

	for ( n = 0; cur != NULL; cur = cnext )
	{
		cnext = BROTHER( cur );				// allow object suicide

		if ( ( ! cond || check_cond( cur->cal( this, lab2, lag ), lopc, value ) ) && ! cur->deleting )
			++n;
	}

	return n;
}


/*************************************************************
 COUNT_ALL (*)
 Count the number of all object lab1 instances below
 and besides the current object type (include siblings).
 If cond is true check if expression 'V("lab2") lop value'
 is true before considering each instance of the object.
 *************************************************************/
double lsd::object::count_all( const char *lab1, int lag, bool cond, const char *lab2, const char *lop, double value )
{
	int n, lopc;
	object *cur, *cnext;
	simulation *sim = attr->cont->sim;

	if ( up->b->head != NULL )
		cur = up->b->head->search_err( lab1, sim->no_search, sim->no_search_up, "counting all" );// pick always first instance
	else
		cur = search_err( lab1, sim->no_search, sim->no_search_up, "counting all" );	// count from here (bad)

	if ( cur == NULL )
		return 0;

	if ( cond )
	{
		lopc = logic_op_code( lop, "counting" );
		if ( lopc < 0 || search_var_err( this, lab2, sim->no_search, sim->no_search_up, true, "counting all" ) == NULL )
			return NAN;
	}
	else
		lopc = -1;

	for ( n = 0; cur != NULL; cur = cnext )
	{
		cnext = cur->hyper_next( lab1 );				// allow object suicide

		if ( ( ! cond || check_cond( cur->cal( this, lab2, lag ), lopc, value ) ) && ! cur->deleting )
			++n;
	}

	return n;
}


/*************************************************************
 STAT (*)
 Compute some basic statistics of a group of Variables or
 Paramters with label lab1 and storing the results in a
 vector of double.
 If cond is true check if expression 'V("lab2") lop value'
 is true before considering each instance of the object.
 Return the number of element instances counted (same as
 r[ 0 ]).

 r[ 0 ]=num;
 r[ 1 ]=average
 r[ 2 ]=variance
 r[ 3 ]=max
 r[ 4 ]=min
 r[ 5 ]=median
 r[ 6 ]=standard deviation
 *************************************************************/
double lsd::object::stat( const char *lab1, double *r, int lag, bool cond, const char *lab2, const char *lop, double value )
{
	int n, lopc;
	double val, r_temp[ 7 ];
	object *cur, *cnext;
	simulation *sim = attr->cont->sim;
	variable *cv;
	d_vecT vals;

	if ( r == NULL )
		r = r_temp;

	cv = search_var_err( this, lab1, sim->no_search, sim->no_search_up, true, "calculating statistics" );
	if ( cv == NULL || cv->up == NULL )
	{
		r[ 0 ] = 0;
		r[ 1 ] = r[ 2 ] = r[ 3 ] = r[ 4 ] = r[ 5 ] = r[ 6 ] = NAN;
		return 0;
	}

	if ( cond )
	{
		lopc = logic_op_code( lop, "calculating statistics" );
		if ( lopc < 0 || search_var_err( this, lab2, sim->no_search, sim->no_search_up, true, "calculating statistics" ) == NULL )
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
		cnext = BROTHER( cur );				// allow object suicide

		if ( ( ! cond || check_cond( cur->cal( this, lab2, lag ), lopc, value ) ) && ! cur->deleting )
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


/*************************************************************
 SORT_*_*
 support comparison functions for object sorting
 *************************************************************/
bool lsd::object::sort_up_1( object *a, object *b, const char *var, int lag )
{
	if ( var != NULL )					// variable defined?
		return a->cal( var, lag ) < b->cal( var, lag );
	else
		return a->node->id < b->node->id;
}

bool lsd::object::sort_down_1( object *a, object *b, const char *var, int lag )
{
	if ( var != NULL )					// variable defined?
		return a->cal( var, lag ) > b->cal( var, lag );
	else
		return a->node->id > b->node->id;
}

bool lsd::object::sort_up_2( object *a, object *b, const char *var1, const char *var2, int lag )
{
	double x, y;

	x = a->cal( var1, lag );
	y = b->cal( var1, lag );

	if ( x < y )
		return true;
	else
		if ( x > y )
			return false;
		else
			return a->cal( var2, lag ) < b->cal( var2, lag );
}

bool lsd::object::sort_down_2( object *a, object *b, const char *var1, const char *var2, int lag )
{
	double x, y;

	x = a->cal( var1, lag );
	y = b->cal( var1, lag );

	if ( x > y )
		return true;
	else
		if ( x < y )
			return false;
		else
			return a->cal( var2, lag ) > b->cal( var2, lag );
}


/*************************************************************
 LSDQSORT (*)
 Use the qsort function in the standard library to sort
 a group of Object with label obj according to the values of var
 if var is NULL, try sorting using the network node id
 *************************************************************/
lsd::object *lsd::object::lsdqsort( const char *obj, const char *var, const char *direction, int lag )
{
	char dir[ 6 ];
	int num, i;
	bridge *cb;
	object *cur;
	simulation *sim = attr->cont->sim;
	variable *cv;
	bool useNodeId = ( var == NULL ) ? true : false;		// sort on node id and not on variable

	if ( ! useNodeId )
	{
		cv = search_var_err( this, var, sim->no_search, sim->no_search_up, true, "sorting" );
		if ( cv == NULL )
			return NULL;

		cur = cv->up;
		if ( cur == NULL || strcmp( obj, cur->attr->label ) )
		{
			sim->error_hard( "variable or parameter not found",
							 "create variable or parameter in model structure",
							 false,
							 "element '%s' is missing (object '%s') for sorting",
							 var, obj );
			return NULL;
		}

		if ( cur->up == NULL )
		{
			sim->error_hard( "object not found",
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
				cb = cur->up->search_bridge( cur->attr, true );
			else
			{
				sim->error_hard( "invalid network object",
								 "check your equation code to add\nthe network structure before using this macro",
								 true,
								 "object '%s' has no network data structure",
								 obj );
				return NULL;
			}
		else
			cb = NULL;
	}

	if ( cb == NULL )
	{
		sim->error_hard( "object not found",
						 "create object in model structure",
						 false,
						 "object '%s' is missing for sorting", obj );
		return NULL;
	}

	if ( cb->head == NULL )
	{
		sim->error_hard( "object has no instance",
						 "check your equation code to prevent this situation",
						 true,
						 "all instances of object '%s' were deleted", obj );
		return NULL;
	}

	// prevent concurrent sorting by more than one thread
	l_guardT lock( obj_comp_lck );
#
	cb->counter_updated = false;
	cur = cb->head;

	next_count( cur, & num );
	o_vecT new_order( num );
	for ( i = 0; i < num; ++i )
	{
		new_order[ i ] = cur;
		cur = cur->next;
	}

	strcpyn( dir, direction, 6 );
	strupr( dir );

	if ( ! strcmp( dir, "UP" ) )
		std::stable_sort( new_order.begin( ), new_order.end( ), [ & ] ( object *a, object *b ) { return sort_up_1( a, b, var, lag ); } );

	else
		if ( ! strcmp( dir, "DOWN" ) )
			std::stable_sort( new_order.begin( ), new_order.end( ), [ & ] ( object *a, object *b ) { return sort_down_1( a, b, var, lag ); } );
		else
		{
			sim->error_hard( "invalid sort option ('UP' or 'DOWN' required)",
							 "check your equation code to prevent this situation",
							 true,
							 "direction '%s' is invalid for sorting", direction );
			return NULL;
		}

	cb->head = new_order[ 0 ];

	for ( i = 1; i < num; ++i )
		( new_order[ i - 1 ] )->next = new_order[ i ];

	new_order[ i - 1 ]->next = NULL;

	return cb->head;
}


/*************************************************************
 LSDQSORT
 Two stage sorting. Objects with identical values of
 var1 are sorted according to their value of var2
 *************************************************************/
lsd::object *lsd::object::lsdqsort( const char *obj, const char *var1, const char *var2, const char *direction, int lag )
{
	char dir[ 6 ];
	int num, i;
	bridge *cb;
	object *cur;
	simulation *sim = attr->cont->sim;
	variable *cv;

	cb = search_bridge( obj, true );			// try to find the bridge

	if ( cb == NULL )
	{
		sim->error_hard( "object not found",
						 "create object in model structure",
						 false,
						 "object '%s' is missing for sorting", obj );
		return NULL;
	}

	if ( cb->head == NULL )
	{
		sim->error_hard( "object has no instance",
						 "check your equation code to ensure at least one instance\nof any object is kept",
						 true,
						 "all instances of object '%s' were deleted", obj );
		return NULL;
	}

	cv = search_var_err( this, var1, sim->no_search, sim->no_search_up, true, "sorting" );
	if ( cv == NULL )
		return NULL;

	cur = cv->up;
	if ( cur == NULL || cb->attr != cur->attr )
	{
		sim->error_hard( "variable or parameter not found",
						 "create variable or parameter in model structure",
						 false,
						 "element '%s' is missing (object '%s') for sorting",
						 var1, obj );
		return NULL;
	}

	if ( cur->up == NULL )
	{
		sim->error_hard( "object not found",
						 "create object in model structure",
						 false,
						 "object '%s' is missing for sorting",
						 obj );
		return NULL;
	}

	// prevent concurrent sorting by more than one thread
	l_guardT lock( obj_comp_lck );

	cb->counter_updated = false;
	cur = cb->head;

	next_count( cur, & num );
	o_vecT new_order( num );
	for ( i = 0; i < num; ++i )
	{
		new_order[ i ] = cur;
		cur = cur->next;
	}

	strcpyn( dir, direction, 6 );
	strupr( dir );

	if ( ! strcmp( dir, "UP" ) )
		std::stable_sort( new_order.begin( ), new_order.end( ), [ & ] ( object *a, object *b ) { return sort_up_2( a, b, var1, var2, lag ); } );
	else
		if ( ! strcmp( dir, "DOWN" ) )
			std::stable_sort( new_order.begin( ), new_order.end( ), [ & ] ( object *a, object *b ) { return sort_down_2( a, b, var1, var2, lag ); } );
		else
		{
			sim->error_hard( "invalid sort option ('UP' or 'DOWN' required)",
							 "check your equation code to prevent this situation",
							 true,
							 "direction '%s' is invalid for sorting", direction );
			return NULL;
		}

	cb->head = new_order[ 0 ];

	for ( i = 1; i < num; ++i )
		( new_order[ i - 1 ] )->next = new_order[ i ];

	new_order[ i - 1 ]->next = NULL;

	return cb->head;
}


/*************************************************************
 DRAW_RND (*)
 Draw randomly an object with label lo with
 probabilities proportional to the values of their
 Variables or Parameters lv
 *************************************************************/
lsd::object *lsd::object::draw_rnd( const char *lo, const char *lv, int lag )
{
	double a, b;
	object *cur, *cur1, *cnext;
	simulation *sim = attr->cont->sim;
	variable *cv;

	cv = search_var_err( this, lv, sim->no_search, sim->no_search_up, true, "random drawing" );
	if ( cv == NULL )
		return NULL;

	cur1 = cur = cv->up;

	for ( a = 0; cur != NULL; cur = cnext )
	{
		cnext = cur->next;						// allow object suicide
		a += cur->cal( lv, lag );
	}

	if ( std::isnan( a ) || std::isinf( a ) )
	{
		sim->error_hard( "invalid random draw option",
						 "check your equation code to prevent this situation",
						 true,
						 "element '%s' has invalid value '%g' for random drawing",
						 lv, a );
		return NULL;
	}

	if ( a == 0 )
	{
		sim->error_hard( "invalid random draw option",
						 "check your equation code to prevent this situation",
						 true,
						 "element '%s' has only zero values for random drawing",
						 lv );
		return NULL;
	}

	do
		b = rnd_01( ) * a;
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


/*************************************************************
 DRAW_RND (*)
 Draw randomly an object with label lab with
 identical probabilities
 *************************************************************/
lsd::object *lsd::object::draw_rnd( const char *lab )
{
	double a, b;
	object *cur, *cur1;
	simulation *sim = attr->cont->sim;

	cur1 = cur = search_err( lab, sim->no_search, sim->no_search_up, "random drawing" );

	if ( cur == NULL )
		return NULL;

	for ( a = 0 ; cur != NULL; cur = cur->next )
		++a;

	if ( a == 0 )
	{
		sim->error_hard( "object not found",
						 "create object in model structure",
						 false,
						 "object '%s' is missing for random drawing", lab );
		return NULL;
	}

	do
		b = rnd_01( ) * a;
	while ( b == a );	// avoid ran1 == 1

	for ( a = 1, cur = cur1, cur1 = cur1->next; a <= b && cur1 != NULL; cur1 = cur1->next )
	{
		++a;
		cur = cur1;
	}

	return cur;
}


/*************************************************************
 DRAW_RND (*)
 Same as draw_rnd but faster, assuming the sum of the
 probabilities to be tot
 *************************************************************/
lsd::object *lsd::object::draw_rnd( const char *lo, const char *lv, int lag, double tot )
{
	double a, b;
	object *cur, *cur1, *cnext;
	simulation *sim = attr->cont->sim;
	variable *cv;

	if ( tot <= 0 )
	{
		sim->error_hard( "invalid random draw option",
						 "check your equation code to prevent this situation",
						 true,
						 "element '%s' has invalid value '%g' for random drawing",
						 lv, tot );
		return NULL;
	}

	cv = search_var_err( this, lv, sim->no_search, sim->no_search_up, true, "random drawing" );
	if ( cv == NULL )
		return NULL;

	cur1 = cur = cv->up;

	b = rnd_01( ) * tot;
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
		sim->error_hard( "invalid random draw option",
						 "check your equation code to prevent this situation",
						 true,
						 "element '%s' has invalid value '%g' for random drawing",
						 lv, tot );
		return NULL;
	}

	return cur;
}


/*************************************************************
 WRITE (*)
 Write the value in the Variable or Parameter lab,
 making it appearing as if it was computed at time
 lag and the variable updated at time time.
 *************************************************************/
double lsd::object::write( const char *lab, double value, int time, int lag )
{
	int i, eff_lag, eff_time;
	simulation *sim = attr->cont->sim;
	variable *cv;

	if ( ( ! sim->use_nan && std::isnan( value ) ) || std::isinf( value ) )
	{
		sim->error_hard( "invalid write operation",
						 "check your equation code to prevent this situation",
						 true,
						 "value '%g' is invalid for writing to element '%s'",
						 value, lab );
		return NAN;
	}

	cv = search_var_err( this, lab, true, true, false, "writing" );
	if ( cv == NULL )
		return NAN;

	if ( cv->under_computation )
	{
		if ( ! cv->attr->dummy )
		{
			sim->error_hard( "invalid write operation",
							 "check your equation code to prevent this situation",
							 true,
							 "variable '%s' is under computation and cannot be written",
							 lab );
			return NAN;
		}

		if ( cv->var_comp_lck.try_lock( ) )
			cv->var_comp_lck.unlock( );
		else
		{
			sim->error_hard( "deadlock during parallel computation",
							 "check your equation code to prevent this situation",
							 true,
							 "variable '%s' is under dummy computation and cannot be written",
							 lab );
			return NAN;
		}
	}

	// prevent concurrent use by more than one thread
	rec_lguardT lock( cv->var_comp_lck );

	if ( cv->param != 1 && time <= 0 && sim->t > 1 )
	{
		sim->error_hard( "invalid write operation",
						 "check your equation code to prevent this situation",
						 true,
						 "invalid update time (%d) for variable '%s'", time, lab );
		return NAN;
	}

	// adjust value if necessary
	value = cv->chk_val( value );

	// allow for change of initial lagged values when starting simulation (t=1)
	if ( cv->param != 1 && time < 0 && sim->t == 1 )
	{
		if ( - time > cv->attr->num_lag )		// check for invalid lag
		{
			sim->error_hard( "invalid write operation",
							 "check your configuration (variable max lag) or\ncode (used lags in equation) to prevent this situation",
							 false,
							 "invalid initial lag (%d) for variable '%s'",
							 time, lab );
			return NAN;
		}

		cv->val[ - time - 1 ] = value;
		cv->last_update = 0;	// force new updating

		if ( time == -1 && ( cv->attr->save || cv->attr->savei ) )
			cv->data[ 0 ] = value;

		// choose next update step for special updating variables
		if ( cv->attr->delay > 0 || cv->attr->delay_range > 0 )
		{
			cv->next_update = cv->attr->delay;
			if ( cv->attr->delay_range > 0 )
				cv->next_update += ( int ) rnd_uniform_int( 0, cv->attr->delay_range );
		}
	}
	else
	{
		if ( lag < 0 || ( cv->param != 1 && lag > cv->attr->num_lag ) || ( cv->param == 1 && lag > 1 ) )
		{
			sim->error_hard( "invalid write operation",
							 "check your configuration (variable max lag) or\ncode (used lags in equation) to prevent this situation",
							 false,
							 "invalid lag (%d) for %s '%s'",
							 lag, cv->param != 1 ? "variable" : "parameter", lab );
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
			if ( time >= sim->t && lag == 0 && cv->last_update < sim->t )
				for ( i = 0; i < cv->attr->num_lag; ++i )
					cv->val[ cv->attr->num_lag - i ] = cv->val[ cv->attr->num_lag - i - 1 ];

			if ( lag == 0 )
			{
				eff_lag = 0;
				eff_time = time;

				// choose next update step for special updating variables
				if ( cv->attr->period > 1 || cv->attr->period_range > 0 )
				{
					cv->next_update = sim->t + cv->attr->period;
					if ( cv->attr->period_range > 0 )
						cv->next_update += ( int ) rnd_uniform_int( 0, cv->attr->period_range );
				}
			}
			else
			{
				// handle rewriting already computed values
				if ( time >= sim->t || time >= cv->last_update )
				{
					eff_lag = lag - ( sim->t - cv->last_update );	// first write in time t
					eff_time = time - lag;
				}
				else
				{
					eff_lag = lag - ( sim->t - time );				// rewrite as t-h in time t
					eff_time = sim->t - lag;
				}

				if ( eff_lag < 0 || eff_lag > cv->attr->num_lag )
				{
					sim->error_hard( "invalid write operation",
									 "check your configuration (variable max lag) or\ncode (used lags in equation) to prevent this situation",
									 true,
									 "invalid update time step (%d) and lag (%d) for variable '%s'",
									 time, lag, lab );
					return NAN;
				}
			}
		}

		cv->val[ eff_lag ] = value;
		cv->last_update = time;

		if ( cv->attr->save || cv->attr->savei )
		{
			if ( eff_time >= cv->start && eff_time <= cv->end )
				cv->data[ eff_time - cv->start ] = value;
			else
				// handle special initial case
				if ( time == 0 && cv->start == 0 )
					cv->data[ 0 ] = value;
		}
	}

	if ( sim->deb_set && sim->t == sim->deb_t && cv->deb_mode != 'n' && cv->deb_mode != 'd' )
	{
		sim->watch_trigger = true;
		sim->watch_write_mode = true;
		strncpy( sim->watch_elem, cv->attr->label, MAX_ELEM_LENGTH );
	}

	return value;
}


/*************************************************************
 INCREMENT (*)
 Increment the value of the variable lab with value.
 Mark variable as computed in t.
 Return the new value.
 *************************************************************/
double lsd::object::increment( const char *lab, double value )
{
	double new_value;
	simulation *sim = attr->cont->sim;
	variable *cv;

	if ( ( ! sim->use_nan && std::isnan( value ) ) || std::isinf( value ) )
	{
		sim->error_hard( "invalid increment operation",
						 "check your equation code to prevent this situation",
						 true,
						 "value '%g' is invalid for incrementing element '%s'",
						 value, lab );
		return NAN;
	}

	cv = search_var_err( this, lab, true, true, false, "incrementing" );
	if ( cv == NULL )
		return NAN;

	if ( ! sim->use_nan && std::isnan( cv->val[ 0 ] ) )	// try to recover from RECALC
		cv->cal( this, 0 );

	if ( ( ! sim->use_nan && std::isnan( cv->val[ 0 ] ) ) || std::isinf( cv->val[ 0 ] ) )
	{
		sim->error_hard( "invalid increment operation",
						 "check your equation code to prevent this situation",
						 true,
						 "current value '%g' of element '%s' is invalid for incrementing",
						 cv->val[ 0 ], lab );
		return NAN;
	}

	new_value = cv->chk_val( cv->val[ 0 ] + value );
	write( lab, new_value, sim->t );

	return new_value;
}


/*************************************************************
 MULTIPLY (*)
 Multiply the value of the variable lv with value.
 Mark variable as computed in t.
 Return the new value.
 *************************************************************/
double lsd::object::multiply( const char *lab, double value )
{
	double new_value;
	simulation *sim = attr->cont->sim;
	variable *cv;

	if ( ( ! sim->use_nan && std::isnan( value ) ) || std::isinf( value ) )
	{
		sim->error_hard( "invalid multiply operation",
						 "check your equation code to prevent this situation",
						 true,
						 "value '%g' is invalid for multiplying element '%s'",
						 value, lab );
		return NAN;
	}

	cv = search_var_err( this, lab, true, true, false, "multiplying" );
	if ( cv == NULL )
		return NAN;

	if ( ! sim->use_nan && std::isnan( cv->val[ 0 ] ) )	// try to recover from RECALC
		cv->cal( this, 0 );

	if ( ( ! sim->use_nan && std::isnan( cv->val[ 0 ] ) ) || std::isinf( cv->val[ 0 ] ) )
	{
		sim->error_hard( "invalid multiply operation",
						 "check your equation code to prevent this situation",
						 true,
						 "current value '%g' of element '%s' is invalid for multiplying",
						 cv->val[ 0 ], lab );
		return NAN;
	}

	new_value = cv->chk_val( cv->val[ 0 ] * value );
	write( lab, new_value, sim->t );

	return new_value;
}


/*************************************************************
 LAT_DOWN (*)
 return the object "up"
 the cell of a lattice
 *************************************************************/
lsd::object *lsd::object::lat_down( void )
{
	int i, j;
	object *cur;

	for ( i = 1, cur = up->search( attr ); cur != this; cur = BROTHER( cur ), ++i );

	cur = BROTHER( up );
	if ( cur == NULL )
		cur = up->up->search( up->attr );

	for ( j = 1, cur = cur->search( attr ); j < i; cur = BROTHER( cur ), ++j );

	return cur;
}


/*************************************************************
 LAT_UP (*)
 return the object "down"
 the cell of a lattice
 *************************************************************/
lsd::object *lsd::object::lat_up( void )
{
	int i, k;
	object *cur, *cur1, *cur2;

	for ( i = 1, cur = up->search( attr ); cur != this; cur = BROTHER( cur ), ++i );

	cur = up->up->search( up->attr );
	if ( cur == up )
		for ( cur1 = up; BROTHER( cur1 ) != NULL; cur1 = BROTHER( cur1 ) );
	else
		for ( cur1 = cur; BROTHER( cur1 ) != up; cur1 = BROTHER( cur1 ) );

	for ( cur2 = cur1->search( attr ), k = 1; k < i; cur2 = BROTHER( cur2 ), ++k );

	return cur2;
}


/*************************************************************
 LAT_RIGHT (*)
 return the object "right"
 the cell of a lattice
 *************************************************************/
lsd::object *lsd::object::lat_right( void )
{
	if ( next == NULL )
		return up->search( attr );
	else
		return next;
}


/*************************************************************
 LAT_LEFT (*)
 return the object "left"
 the cell of a lattice
 *************************************************************/
lsd::object *lsd::object::lat_left( void )
{
	object *cur;

	if ( up->search( attr ) == this )
		for ( cur = this; BROTHER( cur ) != NULL; cur = BROTHER( cur ) );
	else
		for ( cur = up->search( attr ); BROTHER( cur ) != this; cur = BROTHER( cur ) );

	return cur;
}


/*************************************************************
 BUILD_OBJ_LIST
 Build the object list for user pointer checking
 *************************************************************/
double lsd::simulation::build_obj_list( bool set_list )
{
	if ( no_pointer_check )		// disabled in compilation?
	{
		no_ptr_chk = true;
		return 0;
	}

	// prevent concurrent update by more than one thread
	l_guardT lock( obj_list_lck );

	obj_list.clear( );			// reset list

	if ( set_list )
	{
		root->collect_inst( obj_list );
		no_ptr_chk = false;
	}
	else
		no_ptr_chk = true;

	return obj_list.size( );
}


/*************************************************************
 COLLECT_INST
 Collect all object under the selected object and
 stores it in the provided C++ set container
 *************************************************************/
void lsd::object::collect_inst( o_setT &list )
{
	bridge *cb;
	object *cur;

	// collect own address
	auto res = list.emplace( this );
	if ( ! res.second )
	{
		attr->cont->sim->error_hard( "LSD internal error",
									 "disable pointer checking by defining 'NO_POINTER_CHECK'",
									 false,
									 "object '%s' cannot be collected for pointer checking",
									 attr->label );
		return;
	}

	// search among descendants
	for ( cb = b; cb != NULL; cb = cb->next )
		for ( cur = cb->head; cur != NULL; cur = cur->next )
			cur->collect_inst( list );
}


/*************************************************************
 INTERACT (*)
 Interrupt the simulation, as for the debugger,
 allowing the insertion of a value.
 Note that the debugging window, in this model,
 accept the entry key stroke as a run.
 *************************************************************/
double lsd::object::interact( const char *text, double v, double *tv, int i, int j, int h, int k, object *cur, object *cur1, object *cur2, object *cur3, object *cur4, object *cur5, object *cur6, object *cur7, object *cur8, object *cur9, netlink *curl, netlink *curl1, netlink *curl2, netlink *curl3, netlink *curl4, netlink *curl5, netlink *curl6, netlink *curl7, netlink *curl8, netlink *curl9, FILE *f )
{
#ifndef _TERM_
	int n;
	double app = v;
	simulation *sim = attr->cont->sim;

	if ( sim->quit == 0 )
	{
		for ( n = 0; n < USER_D_VARS; ++n )
			sim->_d_values_[ n ] = tv[ n ];

		sim->_i_values_[ 0 ] = i;
		sim->_i_values_[ 1 ] = j;
		sim->_i_values_[ 2 ] = h;
		sim->_i_values_[ 3 ] = k;
		sim->_o_values_[ 0 ] = cur;
		sim->_o_values_[ 1 ] = cur1;
		sim->_o_values_[ 2 ] = cur2;
		sim->_o_values_[ 3 ] = cur3;
		sim->_o_values_[ 4 ] = cur4;
		sim->_o_values_[ 5 ] = cur5;
		sim->_o_values_[ 6 ] = cur6;
		sim->_o_values_[ 7 ] = cur7;
		sim->_o_values_[ 8 ] = cur8;
		sim->_o_values_[ 9 ] = cur9;
		sim->_n_values_[ 0 ] = curl;
		sim->_n_values_[ 1 ] = curl1;
		sim->_n_values_[ 2 ] = curl2;
		sim->_n_values_[ 3 ] = curl3;
		sim->_n_values_[ 4 ] = curl4;
		sim->_n_values_[ 5 ] = curl5;
		sim->_n_values_[ 6 ] = curl6;
		sim->_n_values_[ 7 ] = curl7;
		sim->_n_values_[ 8 ] = curl8;
		sim->_n_values_[ 9 ] = curl9;
		sim->_f_values_[ 0 ] = f;

		if ( sim->liblnk != NULL && sim->liblnk->debugger != NULL )
			( this ->*sim->liblnk->debugger )( NULL, text, &app, true, "" );// signals INTERACT macro
	}

	return app;
#else
	return v;
#endif
}


/*************************************************************
 LOGIC_OP_CODE
 Check for valid relational operator and return
 operator code for CHECK_COND
 *************************************************************/
int lsd::object::logic_op_code( const char *lop, const char *errmsg )
{
	auto lopp = logic_ops_map.find( lop );

	if ( lopp != logic_ops_map.end( ) )
		return lopp->second;

	attr->cont->sim->error_hard( "invalid logical relational operator",
								 "use a valid operator (== != > >= < <=)",
								 false,
								 "cannot compare with '%s' for %s", lop, errmsg );
	return -1;
}


/*************************************************************
 CHECK_COND
 Check if logical condition defined by the logical
 operator code and the two values is true
 *************************************************************/
bool lsd::object::check_cond( double val1, int lopc, double val2 )
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
		case 6:
			return ! std::isnan( val1 );
		case 7:
			return std::isnan( val1 );
		case 8:
			return ! std::isinf( val1 );
		case 9:
			return std::isinf( val1 );
		case 10:
			return ! std::isfinite( val1 );
		case 11:
			return std::isfinite( val1 );
		default:
			return false;
	}
}
