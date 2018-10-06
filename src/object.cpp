/*************************************************************

	LSD 7.1 - May 2018
	written by Marco Valente, Universita' dell'Aquila
	and by Marcelo Pereira, University of Campinas

	Copyright Marco Valente
	LSD is distributed under the GNU General Public License
	
 *************************************************************/

/****************************************************
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
      |             |
	  |char *label  |------> object *next
      |variable *v  |
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

METHODS FOR EQUATIONS

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

- object *search_var_cond(char *lab, double value, int lag );
Uses search_var, but returns the instance of the object that has the searched
variable with the desired value equal to value.

- double overall_max(char *lab, int lag );
Searches for the object having the variable lab. From that object, it considers
the whole group of object of the same type as the one found, and searches the
maximum value of the variables lab with lag lag there contained

- double sum(char *lab, int lag );
Searches for the object having the variable lab. From that object, it considers
the whole group of object of the same type as the one found, and returns
the sum of all the variables lab with lag lag in that group.

- double whg_av(char *lab, char *lab2, int lag );
Same as sum, but it adds up the product between variables lab and lab2 for each
object. WARNING: if lab and lab2 are not in the same object, the results are
messy

- void sort(char *obj, char *var, char *dir);
Sorts the Objects whose label is obj according to the values of their
variable var. The direction of sorting can be UP or DOWN. The method
is just an interface for sort_asc and sort_desc below.

- void sort_asc( object *from, char *l_var);
Sorts (using bubblesort method) the objects in a group according to increasing
values of their variable l_var, restructuring the linked chain.
IMPORTANT:
The initial Object must be the first element of the set of Objects to be sorted,
and hence is must contain a Variable or Parameter labeled Var_label.
The field from must be either the Object whose "next" is this, or, in case this
is the first element of descendants from some Objects and hence it is a son,
it must be the address of the parent of this.
As example, consider a model having a number of Objects Capital defined as
the only type of descendants from Object Firm.
A Variable in Firm, can have an equation having the following line:
...
cur=search("Capital")
cur->sort_asc(p, "Productivity");
...
which will sort the objects Capital according to the ascending values of
Productivity
If instead the object Capital is defined after a group of object (say Clients)
then you need to following lines:

...
cur1=search("Capital"); // cur1 becomes the first Capital
for (cur2=cur1; cur2->next!=cur1; cur2=cur2->next); //finds the last Client before the first Capital
cur1->sort_asc(cur2, "Productivity"); //sorts the group starting from cur1 attached after cur2
...

- void sort_desc( object *from, char *l_var);
Same as sort_asc, but sorting in descending order

- void delete_obj( void ) ;
eliminate the object, keeping in order the chain list.

- object *add_an_object(char *lab);
Add an object of type lab to the descendant of this. It is placed in the last
position of the groups of the same object and initialized according to the initial
values contained in the file of the model, corresponding to the first object
of that type. The new object is returned, for further initialization.
The variables of the object are set to be already computed in the present
time step, and are all set to value 0.
WARNING: it creates, if necessary, the whole set of descendants for the new objects
but places one single element for each single type of object.

- object *add_an_object(char *lab, object *example);
It has the same effect as the method above, but it uses object given as
example, instead of reading the object initializations from the files. It is
much more efficient when the initialization files are huge, so that their
scanning becomes a very complex task.

- void stat(char *lab, double *v);
Reports some statistics on the values of variable named lab contained
in one group of object descending from the this. The results are stored in the
vector v, with the following order:
v[ 0 ]=number of instances;
v[ 1 ]=average
v[ 2 ]=variance
v[ 3 ]=max
v[ 4 ]=min

- void write(char *lab, double value, int time)
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

- object *skip_next_obj(object *t, int *count);
Counts how many types of objects equal to t are in this
group. count returns such value, and the whole function returns the next object
after the last of the series of t.

- object *go_brother(object *c);
returns: c->next, if it is of the same type of c (brother).
Returns NULL otherwise. It is safe to use even when c or c->next are NULL.

METHODS NOT USED IN THE EQUATIONS

- double cal(object *caller, char *l, int lag, int *done);
It is the basic function used in the equations for LSD variables.
It is called by the former type of method cal(l,lag ), because that
is simpler to be used in the equation code. It activates the method
search_var(caller, label)
that returns a variable whose name is label and then calls the method
cal() for that variable (see variable::cal), that returns the desired value.

- int init(object *_up, char *_label);
Initialization for an object. Assigns _up to up and creates the label

- void update( void ) ;
The recursive function computing the equations for a new time step in the
simulation. It first requests the values for its own variables. Then calls update
for all the descendants.

- object *hyper_next(char *lab);
Returns the next object whose name is lab. That is, it makes a search only down
and up, but does never consider objects before the one from which the search
starts from. It is used to chase objects of lab type even when they are scattered
in different groups.

- void add_var(char *label, int lag, double *val, int save);
Add a variable to all the object of this type, initializing its main fields,
that is label, number of lag, initial values and whether is has to be saved or
not

- void add_obj(char *label, int num );
Add a new object type in the model as descendant of current one
 and initialize its name. It makes num copies
of it. This is NOT to be used to make more instances of existing objects.

- void insert_parent_obj_one(char *lab);
Creates a new type of object that has the current one as descendant
and occupies the previous position of this in the chain of its (former) parent

- object *search(char *lab);
Explores one branch of the model to find for an object whose label is lab.
It searches only down and next. Only for Root, the search is extensive on the
whole model.

- void chg_lab(char *lab);
Changes the name of an object type, that is for all the object of this type in
the model

- void chg_var_lab(char *old, char *n);
Only to this object, changes the label of the variable whose label is old,
and it si changed in n

- variable *add_empty_var(char *str);
Add a variable before knowing its contents, setting to a default initialization
values all the fields in the variable. It operates only on object this

- void add_var_from_example(variable *example);
Add a variable copying all the fields by the variable example.
It operates only on object this

- void empty( void ) ;
Deletes all the contents of the object, freeing its memory. Used in delete_obj
just before suicide with
delete this;

METHODS FOR NETWORK OPERATION

see nets.cpp

****************************************************/

#include "decl.h"

bool zero_instances = false;	// flag to allow deleting last object instance
char *qsort_lab;
char *qsort_lab_secondary;
object *globalcur;


/****************************************************
INIT
Set the basics for a newly created object
****************************************************/
int object::init( object *_up, char const *_label )
{
	++total_obj;
	up = _up;
	v = NULL;
	next = NULL;
	to_compute = true;
	label = new char[ strlen( _label ) + 1 ];
	strcpy( label, _label );
	b = NULL;
	hook = NULL;
	node = NULL;				// not part of a network yet
	cext = NULL;				// no C++ object extension yet
	acounter = 0;				// "fail safe" when creating labels
	lstCntUpd = 0; 				// counter never updated
	del_flag = NULL;			// address of flag to signal deletion
	deleting = false;			// not being deleted
	
	return 0;
}


/****************************************************
UPDATE
Compute the value of all the Variables in the Object, saving the values 
and updating the runtime plot. 

For optimization purposes the system tries to ignores descending objects
marked to be not computed. The implementation is quite baroque, but it
should be the fastest.
****************************************************/
void object::update( void ) 
{
	bool deleted = false;
	object *cur, *cur1;
	bridge *cb, *cb1;
	variable *cv;
	
	del_flag = & deleted;			// register feedback channel

	for ( cv = v; ! deleted && cv != NULL && quit != 2; cv = cv->next )
	{ 
		if ( cv->last_update < t && cv->param == 0 )
		{
#ifdef PARALLEL_MODE
			if ( parallel_ready && cv->parallel )
				parallel_update( cv, this );
			else
#endif
				cv->cal( NULL, 0 );
		}
		
		if ( ! deleted  )
		{
			if ( cv->save || cv->savei )
				cv->data[ t ] = cv->val[ 0 ];
#ifndef NO_WINDOW    
			if ( cv->plot == 1 )
			plot_rt( cv );
#endif   
		}
	}

	for ( cb = b; ! deleted && cb != NULL && quit != 2; cb = cb1 )
	{
		cb1 = cb->next;
		if ( cb->head != NULL && cb->head->to_compute )
			for ( cur = cb->head; ! deleted && cur != NULL; cur = cur1 )
			{
				cur1 = cur->next;
				cur->update( );
			}
	}
	
	if ( ! deleted )				// do only if not already deleted
		del_flag = NULL;			// unregister feedback channel
} 


/****************************************************
HYPER_NEXT
Return the next Object in the model with the label lab. The Object is searched
in the whole model, including different branches
****************************************************/
object *object::hyper_next( char const *lab )
{
	object *cur, *cur1;

	for ( cur1 = NULL, cur = next; cur != NULL; cur = skip_next_obj( cur ) )
	{
		cur1 = cur->search( lab );
		if ( cur1 != NULL )
			break;
	}

	if ( cur1 != NULL )
		return cur1;

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
SEARCH
Search the first Object lab in the branch of the model below this
***************************************************/
object *object::search( char const *lab )
{
	object *cur;
	bridge *cb;

	if ( ! strcmp( label, lab ) )
		return this;

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


/********************************************
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
*************************************************/
variable *object::search_var( object *caller, char const *l, bool no_error )
{
	variable *cv;
	bridge *cb; 

	// Search among the variables 
	for ( cv = v; cv != NULL; cv = cv->next )
		if ( ! strcmp( l, cv->label ) )
			return cv;

	// Search among descendants
	for ( cb = b, cv = NULL; cb != NULL; cb = cb->next )
	{
		// search down only if one instance exists and the label is different from caller
		if ( cb->head != NULL && ( caller == NULL || strcmp( cb->head->label, caller->label ) ) )
		{
			cv = cb->head->search_var( this, l, no_error );
			if ( cv != NULL )
				return cv;
		}   
		else
			cv = NULL; 
	}

	// Search up in the tree
	if ( caller != up )
	{ 
		if ( up == NULL )
		{
			if ( ! no_error )
			{
				sprintf( msg, "'%s' is missing", l );
				error_hard( msg, "variable or parameter not found", 
							"check your code to prevent this situation" );
			}
			
			return NULL;
		}
		
		cv = up->search_var( this, l, no_error );
	}

	return cv;
}


/****************************************************
SEARCH_VAR_COND
Search for the Variable or Parameter lab with value value and return it, if found.
Return NULL if not found.
****************************************************/
object *object::search_var_cond( char const *lab, double value, int lag )
{
	bool var_exist = false;
	double res;
	object *cur, *cur1;
	variable *cv;

	for ( cur1 = this; cur1 != NULL; cur1 = cur1->up )
	{
		cv = cur1->search_var( this, lab, true );
		if ( cv == NULL )
		{
			if ( ! var_exist )
			{
				sprintf( msg, "'%s' is missing for conditional searching", lab );
				error_hard( msg, "variable or parameter not found", 
							"check your code to prevent this situation" );
			}
			
			return NULL;
		}
		else
			var_exist = true;				// at least one instance was found

		for ( cur = cv->up; cur != NULL; cur = cur->hyper_next(  ) )
		{
			res = cur->cal( lab, lag );
			if ( res == value )
				return( cur );
		}
	}

	return NULL;
}


/****************************************************
ADD_VAR
Add a Variable to all the Objects of this type in the model
****************************************************/
void object::add_var( char const *lab, int lag, double *val, int save )
{
	variable *a;
	object *cur;

	for ( cur = this; cur != NULL; cur = cur->hyper_next( label ) )
	{
		a = cur->v;
		if ( a != NULL )
		{
			for (  ; ; a = a->next )
				if ( a->next == NULL )
					break;
			a->next = new variable;
			a = a->next;
			a->init( cur, lab, lag, val, save );
		}
		else
		{
			cur->v = new variable;
			( cur->v )->init( cur, lab, lag, val, save );
		}
	}
}


/****************************************************
ADD_EMPTY_VAR
Add a new (empty) Variable, used in the creation of the model structure
****************************************************/
variable *object::add_empty_var( char const *str )
{
	variable *cv;
	variable *app;

	if ( v == NULL )
	{
		app = new variable;
		if ( app == NULL )
		{
			error_hard( "cannot allocate memory for adding variable", 
						"out of memory" );
			return NULL;
		}
		
		v = app;
		v->next = NULL;
		cv = v;
	}
	else
	{
		for ( cv = v; cv->next != NULL; cv = cv->next );
		cv->next = new variable;
		if ( cv->next == NULL )
		{
			error_hard( "cannot allocate memory for adding variable", 
						"out of memory" );
			return NULL;
		}
		cv = cv->next;
	}
	 
	cv->init( this, str, -1, NULL, 0 );

	return cv;
}


/****************************************************
ADD_VAR_FROM_EXAMPLE
Add a Variable identical to the example.
****************************************************/
void object::add_var_from_example( variable *example )
{
	variable *cv;

	if ( v == NULL )
	{
		v = new variable;
		if ( v == NULL )
		{
			error_hard( "cannot allocate memory for adding object", 
						"out of memory" );
			return;
		}

		v->next = NULL;
		cv = v;
	}
	else
	{
		for ( cv = v; cv->next != NULL; cv = cv->next );
	   
		cv->next = new variable;
		if ( cv->next == NULL )
		{
			error_hard( "cannot allocate memory for adding object", 
						"out of memory" );
			return;
		}

		cv = cv->next;
	}

	cv->init( this, example->label, example->num_lag, example->val, example->save );
	
#ifdef PARALLEL_MODE
	// prevent concurrent use by more than one thread
	lock_guard < mutex > lock( cv->parallel_comp );
#endif	

	cv->savei = example->savei;
	cv->last_update = example->last_update;
	cv->plot = ( ! running ) ? example->plot : false;
	cv->parallel = example->parallel;
	cv->observe = example->observe;
	cv->param = example->param;
	cv->deb_cond = example->deb_cond;
	cv->debug = example->debug;
	cv->deb_cnd_val = example->deb_cnd_val;
	cv->data_loaded = example->data_loaded;
}


/****************************************************
ADD_OBJ
Add num sons with label lab to ANY object like this one, wherever is on the
tree 
****************************************************/
void object::add_obj( char const *lab, int num, int propagate )
{
	int i;
	bridge *cb;
	object *a, *cur;

	for ( cur = this; cur != NULL; propagate == 1 ? cur = cur->hyper_next( label ) : cur = NULL )
	{

		if ( cur->b == NULL )
			cb = cur->b = new bridge;
		else
		{
			for ( cb = cur->b; cb->next != NULL; cb = cb->next );
			cb->next = new bridge;
			cb = cb->next;
		}
		cb->counter_updated = false; 
		cb->next = NULL;  
		cb->mn = NULL;

		cb->blabel = new char[ strlen( lab ) + 1 ];
		strcpy( cb->blabel, lab );
		
		a = cb->head = new object;
		a->init( cur, lab );
		for ( i = 1; i < num; ++i )
		{
			a = a->next = new object;
			a->init( cur, lab );
		} 
	}
}


/****************************************************
INSERT_PARENT_OBJ_ONE
Insert a new parent in the model structure. The this object is placed below
the new (otherwise empty) Object
****************************************************/
void object::insert_parent_obj_one( char const *lab )
{
	object *cur,*c2, *newo;
	bridge *cb, *cb1, *newb;

	c2 = up->hyper_next( up->label );
	if ( c2 != NULL )
	{//implement the change to all (old) parents in the model
		cur = c2->search( label );
		cur->insert_parent_obj_one( lab );
	}

	newo = new object;
	newo->init( up, lab );

	newb = new bridge;
	newb->mn = NULL;
	newb->blabel = new char[ strlen( lab ) + 1 ];
	strcpy( newb->blabel, lab );
	newb->head = newo;
	newb->counter_updated = false;

	for ( cb1 = NULL, cb = up->b; strcmp( cb->blabel, label ); cb1 = cb, cb = cb->next );

	if ( cb1 == NULL )
		up->b = newb;		// the replaced object is the first
	else
		cb1->next = newb;	// the replaced object is in between, cb1 is the preceding bridge

	newb->next = cb->next;	// new bridge continues chain of siblings
	cb->next = NULL; 		// replaced object becomes single son

	newo->b = cb;
	for ( cur = cb->head; cur != NULL; cur = cur->next )
		cur->up = newo;
} 


/****************************************************
ADD_N_OBJECTS2 
As the type with the example, but the example is taken from the blueprint
*************************************/
object *object::add_n_objects2( char const *lab, int n )
{
	object *cur;

	cur = blueprint->search( lab );
	cur = add_n_objects2( lab, n, cur, -1 );
	return cur;
}


/****************************************************
ADD_N_OBJECTS2 
Add N objects to the model making a copies of the example object ex
In respect of the original version, it leaves time of last update
as in the example object
****************************************************/
object *object::add_n_objects2( char const *lab, int n, object *ex )
{
	return add_n_objects2( lab, n, ex, -1 );
}


/****************************************************
ADD_N_OBJECTS2 
Add N objects to the model making a copies of the example object ex
In respect of the original version, it allows for the specification
of the time of last update if t_update is positive or zero. If
t_update is negative (<0) it takes the time of last update from
the example object (if >0) or current t (if =0)
****************************************************/
object *object::add_n_objects2( char const *lab, int n, int t_update )
{
	return add_n_objects2( lab, n, blueprint->search( lab ), t_update );
}


object *object::add_n_objects2( char const *lab, int n, object *ex, int t_update )
{
	bool net;
	int i;
	bridge *cb, *cb1, *cb2;
	object *cur, *last, *cur1, *first;
	variable *cv;

	// check the labels and prepare the bridge to attach to
	for ( cb2 = b; cb2 != NULL && strcmp( cb2->blabel, lab ); cb2 = cb2->next );
	if ( cb2 == NULL )
	{
		sprintf( msg, "'%s' contains no object '%s' for adding object(s)", label, lab );
		error_hard( msg, "object not found", 
					"check your code to prevent this situation" );
		return NULL;
	}
	
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

		if ( cur == NULL )
		{
			error_hard( "cannot allocate memory for adding object", 
						"out of memory" );
			return NULL;
		}
		cur->init( this, lab );

		if ( net )						// if objects are nodes in a network
		{
			cur->node = new netNode( );	// insert new nodes in network (as isolated nodes)
			if ( cur->node == NULL )
			{
				error_hard( "cannot allocate memory for adding object", 
							"out of memory" );
				return NULL;
			}
		}

		// create its variables and initialize them
		for ( cv = ex->v; cv != NULL; cv = cv->next )  
		  cur->add_var_from_example( cv );


		for ( cv = cur->v; cv != NULL; cv = cv->next )
		{
#ifdef PARALLEL_MODE
			// prevent concurrent use by more than one thread
			lock_guard < mutex > lock( cv->parallel_comp );
#endif		
			if ( running && cv->param == 0 )
			{
				if ( t_update < 0 && cv->last_update == 0 )
					cv->last_update = t;
				else
					if ( t_update >= 0 )
					{
						if ( t_update < cv->last_update && t > 1 )
						{
							if ( wr_warn_cnt <= ERR_LIM )
								plog( "\n\nWarning: while creating the object named '%s' in equation for '%s' \nthe time set for the last update (%d) is invalid in time t=%d. This may\nundermine the correct updating of variables in '%s', which will be\nrecalculated in the current period (%d)\n", "", lab, stacklog == NULL || stacklog->vs == NULL ? "(none)" : stacklog->vs->label, time, t, lab, t );
							if ( wr_warn_cnt == ERR_LIM )
								plog( "\nWarning: too many invalid update times, stop reporting...\n" );
							++wr_warn_cnt;
						}
						cv->last_update = t_update;
					}
			}
			if ( cv->save || cv->savei )
			{
				if ( running )
				   cv->data = new double[ max_step + 1 ];

				cv->start = t;
				cv->end = max_step;
			}
		}

		// insert the descending objects in the newly created objects
		cb1 = NULL;
		for ( cb = ex->b; cb != NULL; cb = cb->next )
		{
			if ( cb1 == NULL )
				cb1 = cur->b = new bridge;
			else
				cb1 = cb1->next = new bridge;
				
			cb1->mn = NULL;
			cb1->blabel = new char[ strlen( cb->blabel ) + 1 ];
			strcpy( cb1->blabel, cb->blabel );
			cb1->next = NULL;
			cb1->head = NULL;    
			cb1->counter_updated = false;
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
	}

	return first;
}


/****************************************************
DELETE_OBJ
Remove the object from the model
Before killing the Variables data to be saved are stored
in the "cemetery", a linked chain storing data to be analysed.
****************************************************/
void object::delete_obj( void ) 
{
	object *cur;
	bridge *cb;
	
	{							// create context for lock
#ifdef PARALLEL_MODE
		// prevent concurrent deletion by more than one thread
		lock_guard < mutex > lock( parallel_delete );
#endif	

		if ( deleting )			// ignore if deleting already going on
			return;					
		
		if ( under_computation( ) )
			if ( wait_delete != NULL && wait_delete != this )
			{
				sprintf( msg, "cannot schedule the deletion of object '%s'", label );
				error_hard( msg, "another deletion is already pending", 
							"check your code to prevent this situation" );
				return;
			}
			else
			{
				wait_delete = this;
				return;
			}
			
		deleting = true;		// signal deletion to other threads
		
		if ( wait_delete == this )
			wait_delete = NULL;	// finally deleting pending object
	}

	collect_cemetery( this );	// collect required variables BEFORE removing object from linked list

	// find the bridge
	if ( up != NULL )
		for ( cb = up->b; cb != NULL && strcmp( cb->blabel, label ); cb = cb->next );
	else
		cb = NULL;

	if ( cb != NULL )
	{
		if ( cb->head == this )
			if ( next != NULL )
				cb->head = next;
			else
				if ( zero_instances )
					cb->head = NULL;
				else
				{
					sprintf( msg, "cannot delete all instances of '%s'", label );
					error_hard( msg, "last object instance deleted", 
								"check your code to prevent this situation" );
					return;
				}
		else
		{
			for ( cur = cb->head; cur->next != this; cur = cur->next );
			cur->next = next;
		}
		
		cb->counter_updated = false;
	}	
	
	if ( del_flag != NULL )
		*del_flag = true;	// flag deletion to caller, if requested
		
	empty( );
	
	delete this;
}


/****************************************************
CHG_LAB
Change the label of the Object, for all the instances
****************************************************/
void object::chg_lab( char const *lab )
{
	object *cur;
	bridge *cb, *cb1;

	// change all groups of this objects
	cur = up->hyper_next( up->label );
	if ( cur != NULL )
	{
		for ( cb = cur->b; strcmp( cb->blabel, label ); cb = cb->next );
		if ( cb->head != NULL )
			cb->head->chg_lab( lab );
	}

	for ( cb = up->b; strcmp( cb->blabel, label ); cb = cb->next );
	
	delete [ ] cb->blabel;
	cb->blabel = new char[ strlen( lab ) + 1 ];
	strcpy( cb->blabel, lab );

	for ( cur = this; cur != NULL; cur = cur->next )
	{
		if ( cur->label == NULL )
		{
			cur->label = new char[ strlen( lab ) + 1 ];
			strcpy( cur->label, lab );
		}
		else
		{
			delete [ ] cur->label;
			cur->label = new char[ strlen( lab ) + 1 ];
			strcpy( cur->label, lab );
		}
	}
}


/****************************************************
CHG_VAR_LAB
Change the label of the Variable from old to n
****************************************************/
void object::chg_var_lab( char const *old, char const *n )
{
	variable *cur;

	for ( cur = v; cur != NULL; cur = cur->next)
	{
		if ( ! strcmp( cur->label, old ) )
		{ 
#ifdef PARALLEL_MODE
			// prevent concurrent use by more than one thread
			lock_guard < mutex > lock( cur->parallel_comp );
#endif	
			delete [ ] cur->label;
			cur->label = new char[ strlen( n ) + 1 ];
			strcpy( cur->label, n );
			break;
		}
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
		if ( cv->under_computation )
			return true;
	
	return false;
}
	
	
/****************************************************
EMPTY
Garbage collection for Objects.
****************************************************/
void object::empty( void ) 
{
	bridge *cb, *cb1;
	object *cur, *cur1;
	variable *cv, *cv1;

	if ( root == this )
		blueprint->empty( );
 
	for ( cv = v; cv != NULL; cv = cv1 )
		if ( running && ( cv->save || cv->savei ) )
			cv1 = cv->next; 	// variable should have been already saved to cemetery!!!
		else
		{
			cv1 = cv->next;
			cv->empty( );
			delete cv;
		}

	v = NULL;

	for ( cb = b; cb != NULL; cb = cb1 )
	{
		cb1 = cb->next;
		for ( cur = cb->head; cur != NULL; cur = cur1 )
		{ 
			cur1 = cur->next;
			cur->empty( );
			delete cur;
			--total_obj;
		}
		delete [ ] cb->blabel;
		
		if ( cb->mn != NULL )	// turbo search node exists?
		{
			cb->mn->empty( );
			delete cb->mn;
		}
		delete cb; 
	}

	if ( node != NULL )			// network data to delete?
	{
		delete node;
		node = NULL;
	}
	 
	delete [ ] label;
	label = NULL;
	b = NULL; 
}


/****************************************************
CAL
Return the value of Variable or Parameter with label l with lag lag.
The method search for the Variable starting from this Object and then calls
the function variable->cal(caller, lag )
***************************************************/
double object::cal( object *caller,  char const *l, int lag )
{
	variable *curr;

	if ( quit == 2 )
		return NAN;

	curr = search_var( this, l, true );
	if ( curr == NULL )
	{	// check if it is not a zero-instance object
		curr = blueprint->search_var( this, l, true );
		if ( curr == NULL )
		{
			sprintf( msg, "'%s' is missing for retrieving", l );
			error_hard( msg, "variable or parameter not found", 
						"check your code to prevent this situation" );
		}
		else
		{
			sprintf( msg, "all instances of '%s' (containing '%s') were deleted", curr->up->label, l );
			error_hard( msg, "no object instance", 
						"check your code to prevent this situation" );
		}
		
		return NAN;
	}

#ifdef PARALLEL_MODE
	if ( parallel_ready && curr->parallel && curr->last_update < t && lag == 0 )
		parallel_update( curr, this, caller );
#endif
	return curr->cal( caller, lag );
}


/****************************************************
CAL
Interface for object->cal(...), using the "this" object by default
****************************************************/
double object::cal( char const *l, int lag )
{
	return cal( this, l, lag );
}


/****************************************************
RECAL
Mark variable as not calculated in the current time,
forcing recalculation if already calculated
****************************************************/
void object::recal( char const *l )
{
	variable *curr;

	curr = search_var( this, l, true );
	if ( curr == NULL )
	{	// check if it is not a zero-instance object
		curr = blueprint->search_var( this, l, true );
		if ( curr == NULL )
		{
			sprintf( msg, "'%s' is missing for recalculating", l );
			error_hard( msg, "variable or parameter not found", 
						"check your code to prevent this situation" );
		}
		else
		{
			sprintf( msg, "all instances of '%s' (containing '%s') were deleted", curr->up->label, l );
			error_hard( msg, "no object instance", 
						"check your code to prevent this situation" );
		}
		
		return;
	}
	
	curr->last_update = t - 1;
}


/****************************************************
SUM
Compute the sum of Variables or Parameters lab with lag lag.
The sum is computed over the elements in a single branch of the model.
****************************************************/
double object::sum( char const *lab, int lag )
{
	double tot;
	object *cur;
	variable *cur_v;

	cur_v = search_var( this, lab, true );
	if ( cur_v == NULL )
	{	// check if it is not a zero-instance object
		if ( blueprint->search_var( this, lab, true ) == NULL )
		{
			sprintf( msg, "'%s' is missing for summing", lab );
			error_hard( msg, "variable or parameter not found", 
						"check your code to prevent this situation" );
		}
		
		return NAN;
	}

	cur = cur_v->up;
	if ( cur->up != NULL )
		cur = ( cur->up )->search( cur->label );

	for ( tot = 0; cur != NULL; cur = go_brother( cur ) )
		tot += cur->cal( this, lab, lag );

	return tot;
}


/****************************************************
OVERALL_MAX
Compute the maximum of lab, considering only the Objects in a single branch of the model.
****************************************************/
double object::overall_max( char const *lab, int lag )
{
	double tot, temp;
	object *cur;
	variable *cur_v;

	cur_v = search_var( this, lab, true );
	if ( cur_v == NULL )
	{	// check if it is not a zero-instance object
		if ( blueprint->search_var( this, lab, true ) == NULL )
		{
			sprintf( msg, "'%s' is missing for maximizing", lab );
			error_hard( msg, "variable or parameter not found", 
						"check your code to prevent this situation" );
		}
		
		return NAN;
	}

	cur = cur_v->up;
	if ( cur->up != NULL )
		cur = ( cur->up )->search( cur->label );
	
	for ( tot = -DBL_MAX; cur != NULL; cur = go_brother( cur ) )
		if ( tot < ( temp = cur->cal( this, lab, lag ) ) )
			tot = temp;

	return tot;
}


/****************************************************
OVERALL_MIN
Compute the minimum of lab, considering only the Objects in a single branch of the model.
****************************************************/
double object::overall_min( char const *lab, int lag )
{
	double tot, temp;
	object *cur;
	variable *cur_v;

	cur_v = search_var( this, lab, true );
	if ( cur_v == NULL )
	{	// check if it is not a zero-instance object
		if ( blueprint->search_var( this, lab, true ) == NULL )
		{
			sprintf( msg, "'%s' is missing for minimizing", lab );
			error_hard( msg, "variable or parameter not found", 
						"check your code to prevent this situation" );
		}
		
		return NAN;
	}

	cur = cur_v->up;
	if ( cur->up != NULL )
		cur = ( cur->up )->search( cur->label );
	
	for ( tot = DBL_MAX; cur != NULL; cur = go_brother( cur ) )
		if ( tot > ( temp = cur->cal( this, lab, lag ) ) )
			tot = temp;

	return tot;
}


/****************************************************
AV
Compute the average of lab
****************************************************/
double object::av( char const *lab, int lag )
{
	int n;
	double tot;
	object *cur;
	variable *cur_v;

	cur_v = search_var( this, lab, true );
	if ( cur_v == NULL )
	{	// check if it is not a zero-instance object
		if ( blueprint->search_var( this, lab, true ) == NULL )
		{
			sprintf( msg, "'%s' is missing for averaging", lab );
			error_hard( msg, "variable or parameter not found", 
						"check your code to prevent this situation" );
		}
		
		return NAN;
	}

	cur = cur_v->up;
	if ( cur->up != NULL )
		cur = ( cur->up )->search( cur->label );

	for ( n = 0, tot = 0; cur != NULL; cur = go_brother( cur ), ++n )
		tot += cur->cal( this, lab, lag );

	if ( n > 0 )
		return tot / n;
	else
		return NAN;
}


/****************************************************
WHG_AV
Compute the weighted average of lab (lab2 are the weights)
****************************************************/
double object::whg_av( char const *lab, char const *lab2, int lag )
{
	double tot, c1, c2;
	object *cur;
	variable *cur_v;

	cur_v = search_var( this, lab, true );
	if ( cur_v == NULL )
	{	// check if it is not a zero-instance object
		if ( blueprint->search_var( this, lab, true ) == NULL )
		{
			sprintf( msg, "'%s' is missing for weighted averaging", lab );
			error_hard( msg, "variable or parameter not found", 
						"check your code to prevent this situation" );
		}
		
		return NAN;
	}

	cur_v = search_var( this, lab2, true );
	if ( cur_v == NULL )
	{	// check if it is not a zero-instance object
		if ( blueprint->search_var( this, lab2, true ) == NULL )
		{
			sprintf( msg, "'%s' is missing for weighted averaging", lab2 );
			error_hard( msg, "variable or parameter not found", 
						"check your code to prevent this situation" );
		}
		
		return NAN;
	}

	cur = cur_v->up;
	if ( cur->up != NULL )
		cur = ( cur->up )->search( cur->label );

	for ( tot = 0; cur != NULL; cur = go_brother( cur ) )
	{
		c1 = cur->cal( this, lab, lag );
		c2 = cur->cal( this, lab2, lag );
		tot += c1 * c2;
	}

	return tot;
}


/****************************************************
SD
Compute the (population) standard deviation of lab
****************************************************/
double object::sd( char const *lab, int lag )
{
	int n;
	double x, tot, tot2;
	object *cur;
	variable *cur_v;

	cur_v = search_var( this, lab, true );
	if ( cur_v == NULL )
	{	// check if it is not a zero-instance object
		if ( blueprint->search_var( this, lab, true ) == NULL )
		{
			sprintf( msg, "'%s' is missing for calculating s.d.", lab );
			error_hard( msg, "variable or parameter not found", 
						"check your code to prevent this situation" );
		}
		
		return NAN;
	}
	
	cur = cur_v->up;
	if ( cur->up != NULL )
		cur = ( cur->up )->search( cur->label );

	for ( n = 0, tot = 0; cur != NULL; cur = go_brother( cur ), ++n )
	{
		tot += x = cur->cal( this, lab, lag );
		tot2 += x * x;
	}

	if ( n > 0 )
		return sqrt( tot2 / n - pow( tot / n, 2 ) );
	else
		return NAN;
}


/****************************************************
COUNT
Count the number of object lab instances below this 
****************************************************/
double object::count( char const *lab )
{
	int count, temp;
	object *cur, *cur1;
	
	cur = search( lab );
	
	if ( cur == NULL )
	{	// check if it is not a zero-instance object 
		if ( blueprint->search( lab ) == NULL )
		{
			sprintf( msg, "'%s' is missing for counting", lab );
			error_hard( msg, "object not found", 
						"check your code to prevent this situation" );
		}
		
		return 0;
	}
	
	for ( count = 0; cur != NULL; cur = go_brother( cur ), ++count );

	return count;
}


/****************************************************
COUNT_ALL
Count the number of all object lab instances below 
and besides the current object type (include siblings) 
****************************************************/
double object::count_all( char const *lab )
{
	int count, temp;
	object *cur, *cur1;
	
	if ( up->b->head != NULL )
		cur = up->b->head->search( lab );			// pick always first instance
	else
		cur = search( lab );						// count from here (bad)
	
	if ( cur == NULL )
	{	// check if it is not a zero-instance object 
		if ( blueprint->search( lab ) == NULL )
		{
			sprintf( msg, "'%s' is missing for counting", lab );
			error_hard( msg, "object not found", 
						"check your code to prevent this situation" );
		}
		
		return 0;
	}
	
	for ( count = 0; cur != NULL; cur = cur->hyper_next( lab ), ++count );

	return count;
}


/****************************************************
STAT
Compute some basic statistics of a group of Variables or Paramters with lab lab
and storing the results in a vector of double.
Return the number of element instances counted (same as r[ 0 ]).

r[ 0 ]=num;
r[ 1 ]=average
r[ 2 ]=variance
r[ 3 ]=max
r[ 4 ]=min

****************************************************/
double object::stat( char const *lab, double *r )
{
	double r_temp[ 5 ];
	object *cur;
	variable *cur_v;

	if ( r == NULL )
		r = r_temp;
	
	r[ 0 ] = r[ 1 ] = r[ 2 ] = r[ 3 ] = r[ 4 ] = 0;
	
	cur_v = search_var( this, lab, true );
	if ( cur_v == NULL )
	{	// check if it is not a zero-instance object
		if ( blueprint->search_var( this, lab, true ) == NULL )
		{
			sprintf( msg, "'%s' is missing for calculating statistics", lab );
			error_hard( msg, "variable or parameter not found", 
						"check your code to prevent this situation" );
		}
		
		return NAN;
	}

	cur = cur_v->up;
	if ( cur->up != NULL )
	{
		cur_v = cur->up->search_var( cur->up, lab, true );
		if ( cur_v == NULL )
		{	// check if it is not a zero-instance object
			if ( blueprint->search_var( cur->up, lab, true ) == NULL )
			{
				sprintf( msg, "'%s' is missing for calculating statistics", lab );
				error_hard( msg, "variable or parameter not found", 
							"check your code to prevent this situation" );
			}
			
			return NAN;
		}
	}
	else
		return NAN;
		
	cur = cur_v->up;
	if ( cur != NULL )
	{
		r[ 3 ] = r[ 4 ] = cur->cal( lab, 0 );
		for ( r[ 2 ] = 0, r[ 0 ] = 0, r[ 1 ] = 0; cur != NULL; cur = go_brother( cur ) )
		{
			++r[ 0 ];
			r[ 5 ] = cur->cal( lab, 0 );
			r[ 1 ] += r[ 5 ];
			r[ 2 ] += r[ 5 ] * r[ 5 ];
			if ( r[ 5 ] > r[ 3 ] )
				r[ 3 ] = r[ 5 ];
			if ( r[ 5 ] < r[ 4 ] )
				r[ 4 ] = r[ 5 ];
		}
		
		if ( r[ 0 ] > 0 )
		{
			r[ 1 ] /= r[ 0 ];
			r[ 2 ] = r[ 2 ] / r[ 0 ] - r[ 1 ] * r[ 1 ];
		}
		else
			r[ 1 ] = r[ 2 ] = 0;
	}
	
	return r[ 0 ];								// return the number of instances
}


/****************************************************
lsdqsort
Use the qsort function in the standard library to sort
a group of Object with label obj according to the values of var
if var is NULL, try sorting using the network node id
****************************************************/
void object::lsdqsort(char const *obj, char const *var, char const *direction)
{
	int num, i;
	object *cur, *nex, **mylist;
	bridge *cb;
	variable *cur_v;
	bool useNodeId = ( var == NULL ) ? true : false;		// sort on node id and not on variable

	if ( ! useNodeId )
	{
		cur_v = search_var( this, var, true );
		if ( cur_v == NULL )
		{	// check if it is not a zero-instance object
			if ( blueprint->search_var( this, var, true ) == NULL )
			{
				sprintf( msg, "'%s' is missing for sorting", var );
				error_hard( msg, "variable or parameter not found", 
							"check your code to prevent this situation" );
			}
			else
			{
				sprintf( msg, "all instances of '%s' (containing '%s') were deleted", cur_v->up->label, var );
				error_hard( msg, "no object instance", 
							"check your code to prevent this situation" );
			}
			
			return;
		}
		
		cur = cur_v->up;
		if ( strcmp( obj, cur->label ) )
		{
			sprintf( msg, "'%s' is missing (object '%s') for sorting", var, obj );
			error_hard( msg, "variable or parameter not found", 
						"check your code to prevent this situation" );
			return;
		}
	
		if ( cur->up == NULL )
		{
			sprintf( msg, "'%s' is missing for sorting", obj );
			error_hard( msg, "object not found", 
						"check your code to prevent this situation" );
			return;
		}
	
		cb = cur_v->up->up->b;
	}
	else									// pick network object to sort
	{
		cur = search( obj );
		if ( cur != NULL )
			if ( cur->node != NULL )		// valid network node?
				cb = cur->up->b;
			else
			{
				sprintf( msg, "'%s' has no network data structure", obj );
				error_hard( msg, "invalid network object", 
							"check your code to prevent this situation" );
				return;
			}
		else
			cb = NULL;
	}

	for ( ; cb != NULL && strcmp( cb->blabel, obj ); cb = cb->next );
	if ( cb == NULL || cb->head == NULL )
	{
		sprintf( msg, "'%s' is missing for sorting", label);
		error_hard( msg, "object not found", 
					"check your code to prevent this situation" );
		return;
	}
	
	cb->counter_updated = false;
	cur = cb->head;

	nex = skip_next_obj( cur, &num );
	mylist = new object *[ num ];
	for ( i = 0; i < num; ++i )
	{
		mylist[ i ] = cur;
		cur = cur->next;
	}
	
	qsort_lab = ( char * ) var;
	if ( ! strcmp( direction, "UP" ) )
		qsort( ( void * ) mylist, num, sizeof( mylist[ 0 ] ), sort_function_up );
	else
		if ( ! strcmp( direction, "DOWN" ) )
			qsort( ( void * ) mylist, num, sizeof( mylist[ 0 ] ), sort_function_down );
		else
		{
			sprintf( msg, "direction '%s' is invalid for sorting", direction );
			error_hard( msg, "invalid sort option", 
						"check your code to prevent this situation" );
			delete [ ] mylist;
			return;
		}
	
	cb->head = mylist[ 0 ];

	for ( i = 1; i < num; ++i )
		( mylist[ i - 1 ] )->next = mylist[ i ];

	mylist[ i - 1 ]->next = NULL;

	delete [ ] mylist;
}

/************************
Comparison function used in lsdqsort
*************************/
int sort_function_up( const void *a, const void *b )
{
	if ( qsort_lab != NULL )		// variable defined?
	{
		if ( (*( object ** ) a )->cal( qsort_lab, 0 ) < ( *( object ** ) b )->cal(qsort_lab, 0 ) )
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


/************************
Comparison function used in lsdqsort
*************************/
int sort_function_down( const void *a, const void *b )
{
	if ( qsort_lab != NULL )		// variable defined?
	{
		if ( ( *( object ** ) a )->cal( qsort_lab, 0) > ( *( object ** ) b )->cal( qsort_lab, 0 ) )
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


/****************************************************
LSDQSORT
Two stage sorting. Objects with identical values of var1 are sorted according to their value of var2
****************************************************/
void object::lsdqsort( char const *obj, char const *var1, char const *var2, char const *direction )
{
	int num, i;
	object *cur, *nex, **mylist;
	bridge *cb;
	variable *cur_v;

	for ( cb = b; cb != NULL; cb = cb->next )
		if ( ! strcmp( cb->blabel, obj ) )
			break;								// found a bridge
	   
	if ( cb == NULL || cb->head == NULL )
	{
		sprintf( msg, "'%s' is missing for sorting", obj );
		error_hard( msg, "object not found", 
					"check your code to prevent this situation" );
		return;
	}
	
	cur_v = search_var( this, var1, true );
	if ( cur_v == NULL )
	{	// check if it is not a zero-instance object
		if ( blueprint->search_var( this, var1, true ) == NULL )
		{
			sprintf( msg, "'%s' is missing for sorting", var1 );
			error_hard( msg, "variable or parameter not found", 
						"check your code to prevent this situation" );
		}
		else
		{
			sprintf( msg, "all instances of '%s' (containing '%s') were deleted", cur_v->up->label, var1 );
			error_hard( msg, "no object instance", 
						"check your code to prevent this situation" );
		}
		
		return;
	}
	 
	cur = cur_v->up;
	if ( strcmp( obj, cur->label ) )
	{
		sprintf( msg, "'%s' is missing (object '%s') for sorting", var1, obj );
		error_hard( msg, "variable or parameter not found", 
					"check your code to prevent this situation" );
		return;
	}

	if ( cur->up == NULL )
	{
		sprintf( msg, "'%s' is missing for sorting", obj );
		error_hard( msg, "object not found", 
					"check your code to prevent this situation" );
		return;
	}

	cb->counter_updated = false;
	cur = cb->head;

	nex = skip_next_obj( cur, &num );
	mylist = new object *[ num ];
	for ( i = 0; i < num; ++i )
	{
		mylist[ i ] = cur;
		cur = cur->next;
	}
	qsort_lab = ( char * ) var1;
	qsort_lab_secondary = ( char * ) var2;
    
	if ( ! strcmp( direction, "UP" ) )
		qsort( ( void * )mylist, num, sizeof( mylist[ 0 ] ), sort_function_up_two );
	else
		if ( ! strcmp( direction, "DOWN" ) )
			qsort( ( void * ) mylist, num, sizeof( mylist[ 0 ] ), sort_function_down_two );
		else
		{
			sprintf( msg, "direction '%s' is invalid for sorting", direction );
			error_hard( msg, "invalid sort option", 
						"check your code to prevent this situation" );
			delete [ ] mylist;
			return;
		}

	cb->head = mylist[ 0 ];

	for ( i = 1; i < num; ++i )
		( mylist[ i - 1 ] )->next = mylist[ i ];
	
	mylist[ i - 1 ]->next = NULL;

	delete [ ] mylist;
}


/************************
Comparison function used in lsdqsort with two stages
*************************/
int sort_function_up_two( const void *a, const void *b )
{
	double x, y;
	
	x = ( *( object ** ) a )->cal( qsort_lab, 0 );
	y = ( *( object ** ) b )->cal( qsort_lab, 0 );
	
	if ( x < y )
		return -1;
	else
		if ( x > y )
			return 1;
		else
			if ( (* ( object ** ) a )->cal( qsort_lab_secondary, 0 ) < ( *( object ** ) b )->cal( qsort_lab_secondary, 0) )
				return -1;
			else
				return 1;
}


/************************
Comparison function used in lsdqsort with two stages
*************************/
int sort_function_down_two( const void *a, const void *b )
{
	double x, y;
	
	x = ( *( object ** ) a )->cal( qsort_lab, 0 );
	y = ( *( object ** ) b )->cal( qsort_lab, 0 );

	if ( x > y )
		return -1;
	else
		if ( x < y )
			return 1;
		else
			if ( ( *( object ** ) a )->cal( qsort_lab_secondary, 0 ) > ( *( object ** ) b )->cal( qsort_lab_secondary, 0 ) )
				return -1;
			else
				return 1;
}


/*********************
Draw randomly an object with label lo with probabilities proportional to the values of their Variables or Parameters lv
*********************/
object *object::draw_rnd( char const *lo, char const *lv, int lag )
{
	double a, b;
	object *cur, *cur1;
	variable *cur_v;

	cur_v = search_var( this, lv, true );
	if ( cur_v == NULL )
	{	// check if it is not a zero-instance object
		if ( blueprint->search_var( this, lv, true ) == NULL )
		{
			sprintf( msg, "'%s' is missing for calculating statistics", lv );
			error_hard( msg, "variable or parameter not found", 
						"check your code to prevent this situation" );
		}
		
		return NULL;
	}

	cur1 = cur = cur_v->up;

	for ( a = 0; cur != NULL; cur = cur->next )
		a += cur->cal( lv, lag );
  
	if ( is_inf( a ) )
	{
		sprintf( msg, "'%s' has invalid values for random drawing", lv );  
		error_hard( msg, "invalid random draw option", 
					"check your code to prevent this situation" );
		return NULL;
	}

	if ( a == 0 )
	{
		sprintf( msg, "'%s' has only zero values for random drawing", lv );  
		error_hard( msg, "invalid random draw option", 
					"check your code to prevent this situation" );
		return NULL;
	}
	
	do
	{
		b = RND * a;
	}
	while ( b == a ); 	// avoid RND == 1
	
	a = cur1->cal( lv, lag );
	for ( cur = cur1, cur1 = cur1->next; a <= b && cur1 != NULL; cur1 = cur1->next )
	{
		a += cur1->cal( lv, lag );
		cur = cur1;
	}
	
	return cur;
}


/*********************
Draw randomly an object with label lo with identical probabilities 
*********************/
object *object::draw_rnd( char const *lo )
{
	object *cur, *cur1;
	double a, b;

	cur1 = cur = search( lo );

	if ( cur == NULL )
	{	// check if it is not a zero-instance object 
		if ( blueprint->search( lo ) == NULL )
		{
			sprintf( msg, "'%s' is missing for random drawing", lo );
			error_hard( msg, "object not found", 
						"check your code to prevent this situation" );
		}
		
		return NULL;
	}
	
	for ( a = 0 ; cur != NULL; cur = cur->next )
		++a;

	if ( a == 0 )
	{
		sprintf( msg, "'%s' is missing for random drawing", lo );
		error_hard( msg, "object not found", 
					"check your code to prevent this situation" );
		return NULL;
	}

	do
	{
		b = RND * a;
	}
	while ( b == a ); 	// avoid RND == 1
	
	for ( a = 1, cur = cur1, cur1 = cur1->next; a <= b && cur1 != NULL; cur1 = cur1->next )
	{
		++a;
		cur = cur1;
	}
	
	return cur;
}


/*************
Same as draw_rnd but faster, assuming the sum of the probabilities to be tot
***************/
object *object::draw_rnd( char const *lo, char const *lv, int lag, double tot )
{
	double a, b;
	object *cur, *cur1;
	variable *cur_v;

	if ( tot <= 0 )
	{
		sprintf( msg, "'%s'=%g is invalid for random drawing", lv, tot );  
		error_hard( msg, "invalid random draw option", 
					"check your code to prevent this situation" );
		return NULL;
	}  

	cur_v = search_var( this, lv, true );
	if ( cur_v == NULL )
	{	// check if it is not a zero-instance object
		if ( blueprint->search_var( this, lv, true ) == NULL )
		{
			sprintf( msg, "'%s' is missing for calculating statistics", lv );
			error_hard( msg, "variable or parameter not found", 
						"check your code to prevent this situation" );
		}
		
		return NULL;
	}

	cur1 = cur = cur_v->up;

	b = RND * tot;
	a = cur1->cal( lv, lag );
	for ( cur1 = cur1->next; a <= b && cur1 != NULL; cur1 = cur1->next )
	{
		a += cur1->cal( lv, lag );
		cur = cur1;
	}
	
	if ( a > tot )
	{
		sprintf( msg, "value %g for '%s' is invalid for random drawing", tot, lv );  
		error_hard( msg, "invalid random draw option", 
					"check your code to prevent this situation" );
		return NULL;
	}
	
	return cur;
}


/****************************************************
 WRITE
 Write the value in the Variable or Parameter lab, making it appearing as if
 it was computed at time time-lag and the variable updated at time time.
 ***************************************************/
void object::write( char const *lab, double value, int time, int lag )
{
    variable *cv;
	
    if ( ( ! use_nan && is_nan( value ) ) || is_inf( value ) )
    {
		sprintf( msg, "'%g' is invalid for writing to '%s'", value, lab );
		error_hard( msg, "invalid write operation", 
					"check your code to prevent this situation" );
        return;
    }
    
    for ( cv = v; cv != NULL; cv = cv->next)
    {
		if ( ! strcmp( cv->label, lab ) )
		{
			if ( cv->under_computation == 1 )
			{
				sprintf( msg, "'%s' is under computation and cannot be written", lab );
				error_hard( msg, "invalid write operation", 
							"check your code to prevent this situation" );
				return;
			}
			
#ifdef PARALLEL_MODE
			// prevent concurrent use by more than one thread
			lock_guard < mutex > lock( cv->parallel_comp );
#endif	
			if ( cv->param != 1 && time < t && t > 1 )
			{
				if ( wr_warn_cnt <= ERR_LIM )
					plog( "\n\nWarning: while writing variable '%s' in equation for '%s' \nthe time set for the last update (%d) is invalid in time t=%d. This would \nundermine the correct updating of variable '%s', which will be\nrecalculated in the current period (%d)\n", "", lab, stacklog == NULL || stacklog->vs == NULL ? "(none)" : stacklog->vs->label, time, t, lab, t );
				if ( wr_warn_cnt == ERR_LIM )
					plog( "\nWarning: too many invalid update times, stop reporting...\n" );
				++wr_warn_cnt;
				cv->val[ 0 ] = value;
				cv->last_update = t - 1;
			}
			else
			{	// allow for change of initial lagged values when starting simulation (t=1)
				if ( cv->param != 1 && time < 0 && t == 1 )
				{
					if ( - time > cv->num_lag )		// check for invalid lag
					{
						plog( "\n\nWarning: while writing variable '%s' in equation for '%s'\nthe selected lag (%d) or time (%d) is invalid, \n write command ignored\n", "", lab, stacklog == NULL || stacklog->vs == NULL ? "(none)" : stacklog->vs->label, lag, time );
					}
					else
					{
						cv->val[ - time - 1 ] = value;
						cv->last_update = 0;	// force new updating
						if ( time == -1 && ( cv->save || cv->savei ) )
							cv->data[ 0 ] = value;
					}
				}
				else
				{
					if ( lag < 0 || lag > cv->num_lag )
					{
						plog( "\n\nWarning: while writing variable '%s' in equation for '%s'\nthe selected lag (%d) is invalid, \n write command ignored\n", "", lab, stacklog == NULL || stacklog->vs == NULL ? "(none)" : stacklog->vs->label, lag );
					}
					else
					{
						cv->val[ lag ] = value;
						cv->last_update = time;
						int eff_time = time - lag;
						if ( eff_time >= 0 && eff_time <= max_step && ( cv->save || cv->savei ) )
							cv->data[ eff_time ] = value;
					}
				}
			}
			
			return;
		}
    }
	
    sprintf( msg, "'%s' is missing for writing", lab );
	error_hard( msg, "variable or parameter not found", 
				"check your code to prevent this situation" );
}

void object::write( char const *lab, double value, int time )
{
	this->write( lab, value, time, 0 );
}


/************************************************
INCREMENT
Increment the value of the variable lab with value.
If variable was not updated in the current period, first updates it.
Return the new value.
*************************************************/
double object::increment( char const *lab, double value )
{
    variable *cv;
	double new_value;
	
    if ( ( ! use_nan && is_nan( value ) ) || is_inf( value ) )
    {
		sprintf( msg, "'%g' is invalid for incrementing '%s'", value, lab );
		error_hard( msg, "invalid write operation", 
					"check your code to prevent this situation" );
        return NAN;
    }

	for ( cv = v; cv != NULL; cv = cv->next)
		if ( ! strcmp( cv->label, lab ) )
			break;

	if ( cv == NULL )
	{
		sprintf( msg, "'%s' is missing in object '%s' for incrementing", lab, label );
		error_hard( msg, "variable or parameter not found", 
					"check your code to prevent this situation" );
        return NAN;
	}
	
	new_value = cv->cal( this, 0 ) + value;
	this->write( lab, new_value, t );
	
	return new_value;
}


/************************************************
MULTIPLY
Multiply the value of the variable lv with value.
If variable was not updated in the current period, first updates it.
Return the new value.
*************************************************/
double object::multiply(char const *lab, double value)
{
    variable *cv;
	double new_value;
	
    if ( ( ! use_nan && is_nan( value ) ) || is_inf( value ) )
    {
		sprintf( msg, "'%g' is invalid for multiplying '%s'", value, lab );
		error_hard( msg, "invalid write operation", 
					"check your code to prevent this situation" );
        return NAN;
    }

	for ( cv = v; cv != NULL; cv = cv->next)
		if ( ! strcmp( cv->label, lab ) )
			break;

	if ( cv == NULL )
	{
		sprintf( msg, "'%s' is missing in object '%s' for multiplying", lab, label );
		error_hard( msg, "variable or parameter not found", 
					"check your code to prevent this situation" );
        return NAN;
	}
	
	new_value = cv->cal( this, 0 ) * value;
	this->write( lab, new_value, t );
	
	return new_value;
}


/****************************
LAT_DOWN
return the object "up" the cell of a lattice
*****************************/
object *object::lat_down( void ) 
{
	int i, j;
	object *cur;
	
	for ( i = 1, cur = up->search( label ); cur != this; cur = go_brother(cur), ++i );

	cur = go_brother( up );
	if ( cur == NULL )
		cur = up->up->search( up->label );
	 
	for ( j = 1, cur = cur->search( label ); j < i; cur = go_brother( cur ), ++j );

	return cur;
}


/****************************
LAT_UP
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
LAT_RIGHT
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
LAT_LEFT
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


/****************************
DELETE_BRIDGE
Remove a bridge, used when an object is removed from the model.
*****************************/
void delete_bridge( object *d )
{
	bridge *cb, *a;
	object *cur, *cur1; 

	if ( d->up->b == NULL )
		return;

	if ( d->up->b->head == d )
	{
		a = d->up->b;
		d->up->b = d->up->b->next;
		for ( cur = d ; cur != NULL; cur = cur1 )
		{
			cur1 = cur->next;
			cur->empty( ); 
			delete cur;
		} 
		
		delete [ ] a->blabel;
		if ( a->mn != NULL )		// turbo search node exists?
		{
			a->mn->empty( );
			delete a->mn;
		}
		delete a;
		return;
	} 

	for ( cb = d->up->b; cb != NULL; a = cb, cb = cb->next )
		if ( cb->head == d )
		{
			for ( cur = d ; cur != NULL; cur = cur1 )
			{
				cur1 = cur->next;
				cur->empty( ); 
				delete cur;
			} 

			a->next = cb->next;
	
			delete [ ] cb->blabel;
			if ( cb->mn != NULL )		// turbo search node exists?
			{
				cb->mn->empty( );
				delete cb->mn;
			}
			delete cb;
		}
}


/****************************
EMPTY
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
CREATE
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
CREATE
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
turbosearch
Search the object label placed in num position.
This search exploits the structure created with 'initturbo'
If tot is 0, previous set value is used
*****************************/
object *object::turbosearch( char const *label, double tot, double num )
{
	bridge *cb;
	double val, lev;

	for ( cb = b; cb != NULL; cb = cb->next )
		if ( ! strcmp( cb->blabel, label ) )
			break;
	if ( cb == NULL )
	{
		sprintf( msg, "failure when searching object '%s'", label ); 
		error_hard( msg, "object not found", 
					"check your code to prevent this situation" );
		return NULL;
	} 

	if ( cb->mn == NULL )
	{
		sprintf( msg, "'%s' is not initialized for turbo search", label ); 
		error_hard( msg, "invalid search operation", 
					"check your code to prevent this situation" );
		return NULL;
	} 
	 
	val = num - 1;
	if ( tot > 1 )					// if size is informed
		lev = floor( log10( tot - 1 ) ) + 1;
	else
		lev = 0;					// if not, use default
	return( cb->mn->fetch( &val, lev ) );
}


/****************************
INITTURBO
Generate the data structure required to use the turbo-search.
- label must be the label of the descending object whose set is to be organized 
- num is the total number of objects (if not provided or zero, it's calculated).
*****************************/
void object::initturbo( char const *label, double tot = 0 )
{
	bridge *cb;
	object *cur;
	double lev;

	for ( cb = b; cb != NULL; cb = cb->next )
		if ( ! strcmp( cb->blabel, label ) )
			break;
	if ( cb == NULL || cb->head == NULL )
	{
		sprintf( msg, "failure when initializing object '%s' for turbo search", label ); 
		error_hard( msg, "object has no instance", 
					"check your code to prevent this situation" );
		return;
	} 

	if ( tot <= 0 )				// if size not informed
		for ( tot = 0,cur = this->search( label ); cur != NULL; ++tot, cur = go_brother( cur ) );
								// compute it
	if ( cb->mn != NULL )		// remove existing mnode
	{
		cb->mn->empty( );
		delete cb->mn;
	}
	
	globalcur = cb->head;
	cb->mn = new mnode;
	lev = ( tot > 1 ) ? floor( log10( tot - 1 ) ) + 1 : 1;
	cb->mn->create( lev );
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
