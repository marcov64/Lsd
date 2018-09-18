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

	
	v1: initial compilation by Frederik Schaff and Kyaw Oo
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

    double x_1 = position->x;
    double x_2 = b->position->x;
    double y_1 = position->y;
    double y_2 = b->position->y;

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

  // distance
  // Calculate the distance between to objects in the same gis.
  double object::distance(object* b){
    return sqrt( pseudo_distance(b) );
  }

  // within_radius
  // produce iterable list of objects with label inside of radius around origin.
  // the list is stored with the asking object. This allows parallelisation AND easy iterating with a macro.
  // give back first element in list
  std::deque<object*>::iterator object::it_in_radius(char const lab[], double radius, bool random){

    //Add check

    position->in_radius.clear();//reset vector
    double pseudo_radius = radius*radius;

    //bounding boxes
    int x_left  = floor( position->x - radius );
    int x_right = ceil(  position->x + radius );
    int y_top   = ceil(  position->y + radius );
    int y_bottom= floor( position->y - radius );

    //to do (?) recursive adjustment if to far outside

    //adjust box for wrapping
    if (position->map->wrap.left == false)
      x_left = max(0,x_left);
    if (position->map->wrap.right == false)
      x_right = min(position->map->xn,x_right);
    if (position->map->wrap.top == false)
      y_top = min(position->map->yn,y_top);
    if (position->map->wrap.bottom == false)
      y_bottom = max(0,y_bottom);


    //fill vector - naive approach until Kyaw's algorithm is ready
    for (int x=x_left; x<x_right;x++){
      for (int y=y_bottom; y<y_top;y++){
        double x_test = x;
        double y_test = y;
        if (check_positions(x_test,y_test) == false ){
          continue; //invalid position
        }
        for (object* candidate : position->map->elements.at(int(x_test)).at(int(y_test)) ) {
          //in naive approach no sorting!
            if (candidate == this){
              continue; //skip self
            }
            if ( strcmp(candidate->label,lab) == 0){
              if (pseudo_distance(candidate) <= pseudo_radius) {
                position->in_radius.push_back(candidate);
              }
            }
        }
      }
    }
    //sort by distance
    //make items unique
    return position->in_radius.begin();
  }

    //find object with label lab closest to caller, if any inside radius fits
  object* object::closest_in_distance(char const lab[], double radius, bool random)
  {
    it_in_radius(lab, radius, random);
    if (position->in_radius.size()==0 ) {
      return NULL; //no candidate
    }
    return position->in_radius.front();
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
        return singleCandidates.front(); //no candidate at position
      else
        return NULL;
    } else {
      int select_random = uniform_int(0,singleCandidates.size()-1);
      return singleCandidates.at(select_random);
    }
  }

  object* object::search_at_position(char const lab[], bool single)
  {
    if (ptr_map()==NULL){
        sprintf( msg, "failure in search_at_position() for object '%s'", label );
		      error_hard( msg, "the object is not registered in any map",
					"check your code to prevent this situation" );
      return NULL;
    }
    return search_at_position(lab, position->x, position->y, single);
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


