/*************************************************************

	LSD 7.1 - May 2018
	written by Marco Valente, Universita' dell'Aquila
	and by Marcelo Pereira, University of Campinas

	Copyright Marco Valente
	LSD is distributed under the GNU General Public License
	
 *************************************************************/

/****************************************************************************************
GIS.CPP
	GIS tools: Advancing LSD to represent a GIS structure, where objects can be
    located in 2d space.

	
	v1: initial compilation by Frederik Schaff
	to do: v2: full integration with LSD
	
	All functions work on specially defined LSD object's data structures (named here as 
    "gisPosition" and "gisMap"), with the following organization:

	
    object --+-- gisPosition  --+- x (double) : position in x direction
                                +- y (double) : position in y direction
                                +- z (double) : position in z direction (if any)
                                +- map (ptr)  : pointer to gisMap

  to do
	
****************************************************************************************/

//to do: correct linking. Currently included in example file.

extern char msg[300];

//NEW gis creation stuff below

  //  init_gis_singleObj
  //  Initialise the gis and add the single object to it
  bool object::init_gis_singleObj(double _x, double _y, int xn, int yn, int _wrap)
  {
    if ( ptr_map() != NULL ){
          sprintf( msg, "failure in init_gis_singleObj() for object '%s'", label );
		      error_hard( msg, "the object was already part of a GIS",
					"check your code to prevent this situation" );
      return false; //there is already a gis structure
    }
    gisMap* map = init_map(xn, yn, _wrap); //create gis
    return register_at_map(map, _x, _y); //register the object at the gis
  }

  //  init_gis_regularGrid
  //  Initialise a regular Grid GIS and link the objects of the same type to it.
  //  The gis objects need to be contained in the calling object
  bool object::init_gis_regularGrid(char const lab[], int xn, int yn, int _wrap, int _lag){
    if ( ptr_map() != NULL ){
        sprintf( msg, "failure in init_gis_regularGrid() for object '%s'", label );
		    error_hard( msg, "the object was already part of a GIS",
					"check your code to prevent this situation" );
      return false; //there is already a gis structure
    }
    gisMap* map = init_map(xn, yn, _wrap); //create gis
    object *firstNode;
    object *cur;
    if (strcmp(label,lab)==0){
      firstNode = up->search( lab );
    } else {
      firstNode = search( lab );
    }
    if ( firstNode == NULL ){
      sprintf( msg, "failure in init_gis_regularGrid() for object '%s'", label );
		  error_hard( msg, "No object present below callee object",
					"check your code to prevent this situation" );
      return false; //error
    }
    int numNodes = xn*yn;
    add_n_objects2( lab , nodes2create( this, lab, numNodes ), _lag );	// creates the missing node objects,
																	// cloning the first one
    int _x = 0;
    int _y = 0;
  	for ( cur = firstNode; cur != NULL; cur = go_brother( cur ) ){
  		  if (cur->register_at_map(map, _x, _y) == false){							// scan all nodes aplying ID numbers
          return false; //error!
        }
      _y++;
      if (_y == yn){
        _y = 0;
        _x++;
      }
    }
    if (_x != xn || _y != 0){
      sprintf( msg, "failure in init_gis_regularGrid() for object '%s'", label );
		      error_hard( msg, "check the implementation",
					"check your code to prevent this situation" );
      return false; //error!
    } else {
      return true;
    }

  }

  //  map_from_obj
  //  Check if the object is a gis object and return pointer to map.
  gisMap* object::ptr_map(){
    if (position == NULL){
      return NULL;
    } else {
      return position->map;
    }
  }

  //  init_map
  //  create a new map and return the pointer
  //  Caution: Only use in combo with creating a gis object
  gisMap* object::init_map(int xn, int yn, int _wrap)
  {
    gisMap* map = new gisMap(xn, yn, _wrap);
    if ( map == NULL )
		{
			error_hard( "cannot allocate memory for init_map()",
						"out of memory" );
			return NULL;
		}
    return map;
  }

  //  delete_map
  //  delete the gis data structure, resetting all the objects.
  bool object::delete_map()
  {
    gisMap* map = ptr_map();
    if (map == NULL ){
      sprintf( msg, "failure in delete_map() for object '%s'", label );
		      error_hard( msg, "the object was not part of a gis",
					"check your code to prevent this situation" );
      return false; //not part of a gis
    }

    //temporary vector with all items, if any
    std::deque<object*> temp_list;
    for (auto itemx : map->elements)
    {
      for (auto itemxy : itemx) {
        for (object* obj_ptr : itemxy) {
          temp_list.push_back(obj_ptr);
        }
      }
    }
    for (object* item: temp_list){
      if (item->unregister_from_gis() == false){ //the last unregister deletes the map
        return false;
      }
    }
    return true;
  }

  bool object::unregister_from_gis()
  {
    if (  unregister_position( false ) == false){
        sprintf( msg, "failure in unregister_from_gis() for object '%s'", label );
		      error_hard( msg, "not connected to a gis",
					"check your code to prevent this situation" );
      return false; //error
    }
    delete position;
    position = NULL;
    return true;
  }

  bool object::register_at_map(object *shareObj)
  {
    if (shareObj -> ptr_map() == NULL ) {
      sprintf( msg, "failure in register_at_map() for object '%s' at position of object %s", label, shareObj->label );
		      error_hard( msg, "the destination object is not registered in any space" ,
					"likely, the shareObj provided is not registered in any space. Check your code to prevent this situation" );
      return false; //re-registering not allowed. derigster at gis first."
    }

    return register_at_map(shareObj -> position -> map , shareObj -> position -> x, shareObj -> position -> y );
  }

  bool object::register_at_map(gisMap* map, double _x, double _y)
  {
    if (map == NULL) {
      sprintf( msg, "failure in register_at_map() for object '%s'", label );
		      error_hard( msg, "no map to connect to provided",
					"check your code to prevent this situation" );
      return false; //re-registering not allowed. derigster at gis first."
    }
    if ( ptr_map() != NULL ){  //already registered?!
      if ( ptr_map() != map ) {
          sprintf( msg, "failure in register_at_map() for object '%s'", label );
  		      error_hard( msg, "already registered in a space different from the one provided",
  					"check your code to prevent this situation. If you want to change the space, deregister elemet from old one first." );
        return false; //re-registering not allowed. derigster at gis first."
      } else {
        sprintf( msg,"\nInfo (t=%i): In register_at_map() the item %s is already part of the space.",t,label);
        plog(msg);
        sprintf( msg,"It will be moved from pos (%g,%g) to pos (%g,%g) instead.",position->x,position->y,_x,_y);
        plog(msg);
        return change_position(_x,_y);
      }
    }
    position = new  gisPosition (map, _x, _y);
    if ( position == NULL )
		{
			error_hard( "cannot allocate memory for register_at_map()",
						"out of memory" );
			return false;
		}
    register_position(_x,_y);
    return true;
  }

//New GIS handling stuff below

  bool object::register_position(double _x, double _y){
    if (ptr_map() == NULL){
      sprintf( msg, "failure in register_position() for object '%s'", label );
		      error_hard( msg, "the object is not yet connected to a map",
					"check your code to prevent this situation" );
      return false;
    }
    if (_x < 0.0 || _x >= double(position->map->xn) || _y < 0.0 || _y >= double(position->map->yn) ) {
      sprintf( msg, "failure in register_position() for object '%s' position (%g,%g)", label,_x,_y );
		      error_hard( msg, "the position is outside the map",
					"check your code to prevent this situation" );
      return false; //error!
    }
    position->x=_x;
    position->y=_y;
    position->map->elements.at(int(_x)).at(int(_y)).push_back( this );
    position->map->nelements++;
    return true;
  }

  bool object::unregister_position(bool move) {
    if (ptr_map() == NULL){
      sprintf( msg, "failure in unregister_position() for object '%s'", label );
		      error_hard( msg, "the object is not yet connected to a map",
					"check your code to prevent this situation" );
      return false;
    }
    //https://stackoverflow.com/a/31329841/3895476
    auto begin = position->map->elements.at(int(position->x)).at(int(position->y)).begin();
    auto end   = position->map->elements.at(int(position->x)).at(int(position->y)).end();
    for (auto it_item =  begin;  it_item != end; it_item++){
      if (*it_item == this ){
        position->map->elements.at(int(position->x)).at(int(position->y)).erase(it_item);
        position->map->nelements--;
        if (move == false && position->map->nelements == 0){ //important: If unregistering before move, do not delete gis map
          //Last element removed
          delete position->map;
          position->map = NULL;
        }
        return true;
      }
    }
      sprintf( msg, "failure in unregister_position() for object '%s'", label );
		      error_hard( msg, "the object is not registered in the map connected to",
					"check your code to prevent this situation" );
    return false;
  }

  bool object::change_position(object* shareObj)
  {
    if (shareObj -> ptr_map() == NULL ) {
      sprintf( msg, "failure in change_position() for object '%s' at position of object %s", label, shareObj->label );
		      error_hard( msg, "the destination object is not registered in any space" ,
					"likely, the shareObj provided is not registered in any space. Check your code to prevent this situation" );
      return false; //re-registering not allowed. derigster at gis first."
    }
    if (ptr_map() == NULL){
      sprintf( msg, "failure in change_position() for object '%s'", label );
		      error_hard( msg, "the source object is not registered in any space" ,
					"check your code to prevent this situation" );
      return false; //re-registering not allowed. derigster at gis first."
    }
    if (shareObj -> ptr_map() != ptr_map() ) {
      sprintf( msg, "failure in change_position() for object '%s' at position of object %s", label, shareObj->label );
		      error_hard( msg, "the destination object is not registered at the same space as the target object" ,
					"check your code to prevent this situation. If you want to use positions from one space in another, use explicit approach via x,y coordinates." );
      return false; //re-registering not allowed. derigster at gis first."
    }
    return change_position(shareObj->position->x, shareObj->position->y);
  }

  bool object::change_position(double _x, double _y)
  {
    if (check_positions(_x, _y) == false){
      return false; //Out of range
    }
    if (unregister_position(true) == false) {
      return false; //cannot change position, error in init position
    }
    return register_position( _x,  _y);
  }

    //  pseudo_distance
    //  Calculate the pseudo (squared) distance between an object p and another object b.
  double object::pseudo_distance(object *b){
    if (ptr_map() == NULL){
        sprintf( msg, "failure in pseudo_distance() for object '%s'", label );
  		      error_hard( msg, "the object is not yet connected to a map",
  					"check your code to prevent this situation" );
        return NaN;
    }
    if (b->ptr_map() == NULL){
        sprintf( msg, "failure in pseudo_distance() for second object '%s'", b->label );
  		      error_hard( msg, "the object is not yet connected to a map",
  					"check your code to prevent this situation" );
        return NaN;
    }
    if (b->ptr_map() != ptr_map()){
      //both elements need to be part of the same gis
      sprintf( msg, "failure in pseudo_distance() for objects '%s' and '%s'", label, b->label );
  		      error_hard( msg, "the objects are not on the same map",
  					"check your code to prevent this situation" );
        return NaN;
    }

    return pseudo_distance( b->position->x , b->position->y );
  }


    //  pseudo_distance
    //  Calculate the pseudo (squared) distance between an object p and another object b.
  double object::pseudo_distance(double x_2, double y_2){
    if (ptr_map() == NULL){
        sprintf( msg, "failure in pseudo_distance() for object '%s'", label );
  		      error_hard( msg, "the object is not yet connected to a map",
  					"check your code to prevent this situation" );
        return NaN;
    }

    double x_1 = position->x;
    double y_1 = position->y;

    double xn = position->map->xn;
    double yn = position->map->yn;

    double a_sq = x_1-x_2;
    double b_sq = y_1-y_2;

    if (position->map->wrap.noWrap == false) {
      if (position->map->wrap.right && x_1>x_2){
        double alt_a = xn - x_1 + x_2;
        if (alt_a < a_sq){
          a_sq=alt_a;
        }
      } else if (position->map->wrap.left) {
        double alt_a = xn - x_2 + x_1;
        if (alt_a < a_sq){
          a_sq=alt_a;
        }
      }

      if (position->map->wrap.top && y_1 > y_2){
        double alt_b = yn - y_1 + y_2;
        if (alt_b < b_sq){
          b_sq=alt_b;
        }
      } else if (position->map->wrap.bottom){
        double alt_b = yn - y_2 + y_1;
        if (alt_b < b_sq){
          b_sq=alt_b;
        }
      }
    }

    a_sq *= a_sq;
    b_sq *= b_sq;
    return a_sq + b_sq;
  }

  bool object::boundingBox(int &left_io, int &right_io, int &top_io, int &bottom_io, double radius){
    if (ptr_map()==NULL){
        sprintf( msg, "failure in boundingBox() for object '%s'", label );
  	      error_hard( msg, "the object is not registered in any map",
  				"check your code to prevent this situation" );
      return false;
    }
    return boundingBox(position->x, position->y, left_io, right_io, top_io, bottom_io, radius);
  }

    //Calculate the bounding box points.
    //Takes care of wrapping
  bool object::boundingBox(double x, double y, int &left_io, int &right_io, int &top_io, int &bottom_io, double radius){
    if (ptr_map()==NULL){
        sprintf( msg, "failure in boundingBox() for object '%s'", label );
  	      error_hard( msg, "the object is not registered in any map",
  				"check your code to prevent this situation" );
      return false;
    }
    //define the bounding box
    left_io   = floor( x - radius );
    right_io  = ceil(  x + radius );
    top_io    = ceil(  y + radius );
    bottom_io = floor( y - radius );

    //adjust box for wrapping
    if (position->map->wrap.left == false)
      left_io = max(0,left_io);
    if (position->map->wrap.right == false)
      right_io = min(position->map->xn,right_io);
    if (position->map->wrap.top == false)
      top_io = min(position->map->yn,top_io);
    if (position->map->wrap.bottom == false)
      bottom_io = max(0,bottom_io);

    return true;
    //we could make sure that we do not traverse the same point several times.
  }


  //a function that receives all objects inside the bounding box of the object
  //at x,y with radius and performs whatever the provided funtion do_stuff tells
  bool object::traverse_boundingBox(double radius, std::function<bool(object* use_obj)> do_stuff )
  {
    //define the bounding box
    int x_left, x_right, y_top, y_bottom;
    if (boundingBox(x_left, x_right, y_top, y_bottom,radius) == false){
      return false; //Error msg in boundingBox
    }

      //fill vector - naive approach (complete)
    for (int x=x_left; x<x_right;x++){
      for (int y=y_bottom; y<y_top;y++){
        access_GridPosElements(x,y,do_stuff); //do nothing if rvalue is false/wrong
//         double x_test = x;
//         double y_test = y;
//         if (check_positions(x_test,y_test) == false ){
//           continue; //invalid position
//         }
//         for (object* candidate : position->map->elements.at(int(x_test)).at(int(y_test)) ) {
//           do_stuff(candidate); //do not use rvalue (true/false)
//         }
      }
    }
    return true; //went to end without any break;
  }

  //Access all elements registered at the position x,y and use the function on them
  bool object::access_GridPosElements (int x, int y, std::function<bool(object* use_obj)> do_stuff)
  {
      //control for wrapping and adjust if necessary
      double x_test = x;
      double y_test = y;
      if (check_positions(x_test,y_test) == false ){
        return false; //invalid position
      }

      //Cycle through all the potential ellements
      for (object* candidate : position->map->elements.at(int(x_test)).at(int(y_test)) ) {
        do_stuff(candidate); //do not use rvalue (true/false)
      }
      return true;
  }


  //a function that receives all objects inside the belt of the bounding box
  //(i.e., only the right/left columns and top/bottom rows of the grid)
  //at x,y with radius and performs whatever the provided funtion do_stuff tells
  bool object::traverse_boundingBoxBelt(double radius, std::function<bool(object* use_obj)> do_stuff )
  {
    //define the bounding box
    int x_left, x_right, y_top, y_bottom;
    if (boundingBox(x_left, x_right, y_top, y_bottom,radius) == false){
      return false; //Error msg in boundingBox
    }

    //left column
    for (int x=x_left, y=y_top; y>=y_bottom; y--){
      access_GridPosElements(x,y,do_stuff); //do nothing if rvalue is false/wrong
    }

    //right column
    for (int x=x_right, y=y_top; y>=y_bottom; y--){
      access_GridPosElements(x,y,do_stuff); //do nothing if rvalue is false/wrong
    }

    //top row w/o left/right element
    for (int y=y_top, x=x_left+1; x<x_right; x++){
      access_GridPosElements(x,y,do_stuff); //do nothing if rvalue is false/wrong
    }

    //bottom row w/o left/right element
    for (int y=y_bottom, x=x_left+1; x<x_right; x++){
      access_GridPosElements(x,y,do_stuff); //do nothing if rvalue is false/wrong
    }
    return true; //went to end without any break;
  }

  // distance
  // Calculate the distance between to objects in the same gis.
  double object::distance(object* b){
    return sqrt( pseudo_distance(b) );
  }

  double object::distance(double x, double y){
    return sqrt( pseudo_distance(x, y) );
  }


  variable* object::search_var_local(char const l[])
  {
    for ( variable* cv = v; cv != NULL; cv = cv->next ){
      if ( ! strcmp( l, cv->label ) ){
        return cv;
      }
    }
      sprintf( msg, "'%s' is missing for conditional searching in search_var_local()", l );
				error_hard( msg, "variable or parameter not found",
							"check your code to prevent this situation" );
    return NULL;
  }

    //in addition to add_if_dist_lab checks if VAR CONDITION condVAL is true
    //VAR is a variable contained in the object lab.
    //if operator() is called with negative pseudo_radius, it will ignore the distance check
  struct add_if_dist_lab_cond
  {
    object* this_obj; //object for which the search is conducted
    double pseudo_radius;
    char lab[ MAX_ELEM_LENGTH ];
    object* fake_caller; //can be a fake-caller if not NULL.
    int lag;
    char varLab[ MAX_ELEM_LENGTH ];
    char condition; // 1 : == ; 2 : > ; 3 : < ; 4: !=; that's it
    double condVal;

//     //unconditional
//     add_if_dist_lab_cond(object* this_obj, double pseudo_radius, char const _lab[])
//       : this_obj(this_obj), pseudo_radius(pseudo_radius)
//       {
//         strcpy(lab, _lab);
//         caller = NULL;
//         strcpy(varLab,"");
//       };

    //conditional
    add_if_dist_lab_cond(object* this_obj, double pseudo_radius, char const _lab[],object* fake_caller, int lag, char const _varLab[], char const _condition[], double condVal)
      : this_obj(this_obj), pseudo_radius(pseudo_radius), fake_caller(fake_caller), lag(lag), condition(_condition[0]), condVal(condVal)
      {
        strcpy(lab, _lab);
        strcpy(varLab,_varLab);
      };

                                        //defaults for the unconditional call
    bool operator()(object* candidate) const
      {
        if (candidate == this_obj)
          return false; //do not collect self

        if ( strcmp(candidate->label,lab) == 0){
          double ps_dst = this_obj->pseudo_distance(candidate);
          if (pseudo_radius<0 || ps_dst <= pseudo_radius) {
            bool isCandidate = true;

            //if conditional, additional check
            if (condition == '>' || condition == '<' || condition == '=' || condition == '!' ){
              variable* condVar = candidate->search_var_local(varLab);
              if (condVar == NULL){
                sprintf( msg, "'%s' is missing for conditional searching in add_if_dist_lab_cond()", varLab );
    				      error_hard( msg, "variable or parameter not found",
    							"check your code to prevent this situation" );
                return false;
              }

              double val;
              if (fake_caller == NULL)
                val = condVar->cal(candidate,lag);
              else
                val = condVar->cal(fake_caller,lag);

              switch (condition)
              {
                case '=': isCandidate = ( val == condVal ? true : false );
                          break;
                case '>': isCandidate = ( val > condVal ? true : false );
                          break;
                case '<': isCandidate = ( val < condVal ? true : false );
                          break;
                case '!': isCandidate = ( val != condVal ? true : false );
                          break;
                default : isCandidate = false;
              }
            }//end conditional
            if ( isCandidate == true ){
  			     this_obj->position->objDis_inRadius.push_back(make_pair(ps_dst,candidate));
              return true;
            }
          }
        }
        return false;
      } //operator()
  };

  inline void object::sort_objDisSet(bool pointer_sort){
    if (pointer_sort)
      std::sort( position->objDis_inRadius.begin(),  position->objDis_inRadius.end() ); //sort - for unique its important that also second is used.
    else
      std::sort( position->objDis_inRadius.begin(),  position->objDis_inRadius.end(), [](auto const &A, auto const &B ){return A.first < B.first; } ); //sort only by distance
  }


  void object::make_objDisSet_unique(bool sorted)
  {
    if (sorted == false)
      sort_objDisSet(true);

    for (auto it = position->objDis_inRadius.begin(); it!= position->objDis_inRadius.end(); /*nothing*/)
    {
      if (it == position->objDis_inRadius.begin()){
        ++it;
        continue; //skip 1. entry
      }

      if (std::prev(it)->second == it->second){
        it = position->objDis_inRadius.erase(it);
      } else {
         ++it;
      }
    }
  };

    //go through intervals with same distance and randomise the order in each
  void object::randomise_objDisSetIntvls(bool sorted){
    if (sorted == false)
      sort_objDisSet(false); //sort only by distance

    auto it_start = position->objDis_inRadius.begin();
    auto it_last = position->objDis_inRadius.end()-1;
    for (auto it = position->objDis_inRadius.begin(); it != position->objDis_inRadius.end(); it++){

        //entering new interval or at end of last interval?
      if (it->second != it_start->second || it == it_last)
      {
          //interval with more than one object?
        if (std::distance(it_start,it)>1){
           std::shuffle(it_start,it,PRNG());
        }
        it_start = it;
      }
    }
  }

  // within_radius
  // produce iterable list of objects with label inside of radius around origin.
  // the list is stored with the asking object. This allows parallelisation AND easy iterating with a macro.
  // give back first element in list
  void object::it_in_radius(char const lab[], double radius, bool random, object* caller, int lag, char const varLab[], char const condition[], double condVal){

    position->objDis_inRadius.clear();//reset vector
    double pseudo_radius = radius*radius;


    //depending on the call of this function, the conditions are initialised meaningfully or not.
    add_if_dist_lab_cond functor_add(this,pseudo_radius,lab,caller,lag,varLab,condition,condVal);  //define conditions for adding


    traverse_boundingBox(radius, functor_add ); //add all elements inside bounding box to the list, if they are within radius


    //sort by distance
    sort_objDisSet(true); //pointer_sort = true

    //make items unique
    make_objDisSet_unique(true); //sorted = true

    //randomize in intervals of same distance
    randomise_objDisSetIntvls(true); //sorted = true

	  position->it_obj = position->objDis_inRadius.begin();
  }

  //check if the current state points to an existing object or all objects have been traversed.
  bool object::next_neighbour_exists()
  {
    return position->it_obj != position->objDis_inRadius.end();
  }

  //Initialise the nearest neighbour search and return nearest neighbours
  object* object::next_neighbour()
  {
    object* next_ngbo = NULL;
    if (position->it_obj != position->objDis_inRadius.end() ){
      next_ngbo =  position->it_obj->second;
      position->it_obj++; //advance
    }
    return next_ngbo;
  }

  object* object::first_neighbour(char const lab[], double radius, bool random, object* caller, int lag, char const varLab[], char const condition[], double condVal)
  {
    if (ptr_map()==NULL){
        sprintf( msg, "failure in next_neighbour() for object '%s'", label );
		      error_hard( msg, "the object is not registered in any map",
					"check your code to prevent this situation" );
      return NULL;
    }
    it_in_radius(lab, radius, random, caller, lag, varLab, condition, condVal);
    return next_neighbour();
  }

  //Return the radius that is necessary to include all the potential items
  //NOT OPTIMISED
  double object::complete_radius()
  {
    return max(  max(position->x,position->map->xn - position->x) , max(position->y , position->map->yn - position->y) );
  }
  
    //find object with label lab closest to caller, if any inside radius fits
    //efficient implementation with increasing search radius
  object* object::closest_in_distance(char const lab[], double radius, bool random, object* caller, int lag, char const varLab[], char const condition[], double condVal)
  {
    double cur_radius = ceil(min(radius,1.0));
    double max_radius = complete_radius(); //we do not need to go beyond this radius
    position->objDis_inRadius.clear();//reset vector

    //depending on the call of this function, the conditions are initialised meaningfully or not.
    add_if_dist_lab_cond functor_add(this,-1,lab,caller,lag,varLab,condition,condVal);  //define conditions for adding



    //In a first initial step, we identify the items in the boundary box.
    traverse_boundingBox(cur_radius, functor_add ); //add all elements inside bounding box to the list, if they are within radius
    std::sort( position->objDis_inRadius.begin(),position->objDis_inRadius.end() ); //sort list

    for (/*is init*/; (cur_radius < radius && cur_radius < max_radius); cur_radius++ )
    {
//       PLOG("\nRadius now: %g",cur_radius);
      //always: check if the set is yet empty, in which case the radius is increased and new items are added before we continue
      if (position->objDis_inRadius.empty() == false){

        //check if there is a closed interval OR the radius is at least 1 level beyond the element
        if (position->objDis_inRadius.front().first < position->objDis_inRadius.back().first
           || ceil(position->objDis_inRadius.back().first) < cur_radius ){
//           PLOG("\nFound a solution set.");
          break; //we found a solution set
        }
      }

      traverse_boundingBoxBelt(cur_radius, functor_add );//add new options
      //sort the elements - THIS CAN BE OPTIMISED
      std::sort( position->objDis_inRadius.begin(),position->objDis_inRadius.end() );
    }


    if (position->objDis_inRadius.empty() == true){
      return NULL; //no option found;
      PLOG("\nNo Options.");
    } else {
      if (random == false)
        return position->objDis_inRadius.front().second;

      int n = -1;  //select randomly amongst set of candidates.
      for (auto const& item : position->objDis_inRadius){
        if (item.first == position->objDis_inRadius.front().first){
          n++;
        }
      }
//       PLOG("\nTotal %i options, selecting randomly",n);
      if (n>0){
        n = uniform_int(0,n);
      }
      return position->objDis_inRadius.at( n ).second;
    }

  }

    //find object at position xy. Check that it is the only one. Use exact position.
  object* object::search_at_position(char const lab[], double x, double y, bool single) {
    if (ptr_map()==NULL){
        sprintf( msg, "failure in search_at_position() for object '%s'", label );
		      error_hard( msg, "the object is not registered in any map",
					"check your code to prevent this situation" );
      return NULL;
    }
    if (check_positions(x,y) == false ){
        sprintf( msg, "failure in search_at_position() searching at position (%g,%g) for '%s'", x,y, lab );
		      error_hard( msg, "the position is not on the map",
					"check your code to prevent this situation. Could be wrapping issues." );
      return NULL; //position incorrect
    }
    std::vector<object*> singleCandidates;
    for (object* candidate : position->map->elements.at(int(x)).at(int(y)) ) {
      //return first element with label
      if (x == candidate->position->x && y == candidate->position->y) {
        if (strcmp(lab,candidate->label) == 0 ){
          if (single == true && singleCandidates.empty() == false){
            sprintf( msg, "failure in search_at_position() searching at position (%g,%g) for '%s'", x,y, lab );
  		        error_hard( msg, "there are several (at least two) items of this type present at the map.",
  					   "check your code to prevent this situation." );
            return NULL;
          }
          singleCandidates.push_back(candidate);
        }
      }
    }
    if (single == true) {
      if (singleCandidates.empty() == false)
        return singleCandidates.front();
      else
        return NULL; //no candidate at position
    } else {
      int select_random = uniform_int(0,singleCandidates.size()-1);
      return singleCandidates.at(select_random);
    }
  }

  object* object::search_at_position(char const lab[], bool single, bool grid)
  {
    if (ptr_map()==NULL){
        sprintf( msg, "failure in search_at_position() for object '%s'", label );
		      error_hard( msg, "the object is not registered in any map",
					"check your code to prevent this situation" );
      return NULL;
    }
    if (grid == false)
      return search_at_position(lab, position->x, position->y, single);
    else
      return search_at_position(lab, trunc(position->x), trunc(position->y), single);
  }

  double object::random_pos(const char xy)
  {
    if (ptr_map()==NULL){
        sprintf( msg, "failure in random_pos() for object '%s'", label );
		      error_hard( msg, "the object is not registered in any map",
					"check your code to prevent this situation" );
      return -1;
    }
    switch (xy) {

      case 'x':
      case 'X': return uniform(0,position->map->xn);
      case 'y':
      case 'Y': return uniform(0,position->map->yn);
      default :
        sprintf( msg, "failure in random_pos() for object '%s' parameter '%c'", label, xy );
		      error_hard( msg, "the parameter is not correct.",
					"check your code to prevent this situation. Options are 'x' or 'y'" );
        return -1;
    }
  }

  double object::get_pos(char xyz)
  {
    if (ptr_map()==NULL){
        sprintf( msg, "failure in pos() for object '%s'", label );
		      error_hard( msg, "the object is not registered in any map",
					"check your code to prevent this situation" );
      return -1;
    }
    switch (xyz) {
      case 'x' :
      case 'X' : return position->x;
      case 'y' :
      case 'Y' : return position->y;
      case 'Z' :
      case 'z' : return position->z;
      default  : ;
    }
    sprintf( msg, "failure in pos() for object '%s'", label );
		      error_hard( msg, "only position x, y or z possible",
					"check your code to prevent this situation" );
      return -1;
  }

  bool object::move(char const direction[])
  {
    int dir = 0; //stay put

    switch (direction[0]) {

      case 'n':   if (direction[1]=='w'){
                    dir = 8; //north-west
                  } else if (direction[1]=='e'){
                    dir = 2; //north-west
                  } else {
                    dir = 1; //north
                  }
                  break;
      case 'w':   dir = 3;
                  break;
      case 's':   if (direction[1]=='w'){
                    dir = 6; //south-west
                  } else if (direction[1]=='e'){
                    dir = 4; //south-west
                  } else {
                    dir = 5; //south
                  }
                  break;
      case 'e':   dir = 7;
                  break;
      default :   dir = 0; //stay put.
    }

    return move(dir);
  }

  bool object::move(int direction)
  {
    if (ptr_map()==NULL){
        sprintf( msg, "failure in move() for object '%s'", label );
		      error_hard( msg, "the object is not registered in any map",
					"check your code to prevent this situation" );
      return false;
    }
    double x_out = position->x;
    double y_out = position->y;
    switch (direction) {
      case 0: return true;
      case 1: y_out++; break;
      case 2: y_out++; x_out++; break;
      case 3: x_out++; break;
      case 4: y_out--; x_out++; break;
      case 5: y_out--; break;
      case 6: y_out--; x_out--; break;
      case 7: x_out--; break;
      case 8: y_out++; x_out--; break;
    }
    return change_position(x_out, y_out);
  }

    //  check_positions
    //  Function to check if the x any y are in the bounds of the map.
    //  If wrapping is possible and allowed, the positions are transformed
    //  accordingly
  bool object::check_positions(double& _xOut, double& _yOut)
  {
    if (ptr_map()==NULL){
        sprintf( msg, "failure in check_positions() for object '%s'", label );
		      error_hard( msg, "the object is not registered in any map",
					"check your code to prevent this situation" );
      return false;
    }
    if (   (_xOut >= 0.0 && _xOut < double(position->map->xn) )
        && (_yOut >= 0.0 && _yOut < double(position->map->yn) ) ) {
      return true; //all fine, nothing to change.
    }

    double _x = _xOut;
    double _y = _yOut;
    if (_x <0) {
      if (position->map->wrap.left == true){
        while (_x <0) {_x = double(position->map->xn) + _x;} //_x is neg
      } else {
        return false;
      }
    }
    if (_y <0) {
      if (position->map->wrap.bottom == true){
        while (_y <0) {_y = double(position->map->yn) + _y;} //_y is neg
      } else {
        return false;
      }
    }
    if (_x >= position->map->xn){
      if (position->map->wrap.right == true){
        while (_x >= position->map->xn) { _x -= double(position->map->xn); }
      } else {
        return false;
      }
    }
    if (_y >= position->map->yn){
      if (position->map->wrap.top == true){
        while (_y >= position->map->yn) { _y -= double(position->map->yn); }
      } else {
        return false;
      }
    }
    if (check_positions(_x, _y) == false){ //recursively for new corner cases!
      return false;
    }
    _yOut = _y; //set values
    _xOut = _x;
    return true;
  }


