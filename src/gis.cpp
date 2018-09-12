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
  bool object::init_gis_singleObj(object* gisObj, double _x, double _y, int xn, int yn, int _wrap)
  {
    if ( gisObj->ptr_map() != NULL ){
          sprintf( msg, "failure in init_gis_singleObj() for object '%s'", label );
		      error_hard( msg, "the object was already part of a GIS",
					"check your code to prevent this situation" );
      return false; //there is already a gis structure
    }
    gisMap* map = init_map(xn, yn, _wrap); //create gis
    return gisObj->register_at_map(map, _x, _y); //register the object at the gis
  }

  //  init_gis_regularGrid
  //  Initialise a regular Grid GIS and link the objects of the same type to it.
  //  The gis objects need to be contained in the calling object
  bool object::init_gis_regularGrid(char const lab[], int xn, int yn, int _wrap){
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
    add_n_objects2( lab , nodes2create( this, lab, numNodes ) );	// creates the missing node objects,
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
    return new gisMap(xn, yn, _wrap); //delete is taken care of later.
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
      return false; //error
    }
    delete position;
    position = NULL;
    return true;
  }

  bool object::register_at_map(gisMap* map, double _x, double _y)
  {
    if (ptr_map() != NULL || map == NULL) {
      sprintf( msg, "failure in register_at_map() for object '%s'", label );
		      error_hard( msg, (map == NULL? "no map to connect to provided":"object was already registered"),
					"check your code to prevent this situation" );
      return false; //re-registering not allowed. derigster at gis first."
    }
    position = new  gisPosition (map, _x, _y);
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
    if (_x < 0 || _x >= position->map->xn || _y < 0 || _y >= position->map->yn) {
      return false; //error!
    }
    position->map->elements.at(int(_x)).at(int(_y)).push_back( this );
    position->map->nelements++;
    return true;
  }

  bool object::unregister_position(bool move) {
    //https://stackoverflow.com/a/31329841/3895476
    auto& list = position->map->elements.at(int(position->x)).at(int(position->y));
    auto begin = list.begin();
    auto end   = list.end();
    for (auto it_item =  list.begin();  it_item != list.end(); /*nothing*/){
      if (*it_item == this ){
        list.erase(it_item);
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

  bool object::change_position(double _x, double _y){
    if (unregister_position(true) == false) {
      return false; //cannot change position, error in init position
    }
    return register_position( _x,  _y);
  }


    //  pseudo_distance
    //  Calculate the pseudo (squared) distance between an object p and another object b.
  double object::pseudo_distance(object *b){
    if (b->position == NULL){
      //invalid result - both elements need to be gis elements
      return NaN;
    } else if (b->position->map != position->map){
      //both elements need to be part of the same gis
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

    position->in_radius.clear();//reset vector
    double pseudo_radius = radius*radius;
    //fill vector - naive approach until Kyaw's algorithm is ready
    for (int x=0; x<position->map->xn;x++){
      for (int y=0; y<position->map->yn;y++){
        for (object* candidate : position->map->elements.at(x).at(y)){
          //in naive approach no sorting!
            if ( strcmp(candidate->label,lab) == 0){
              if (pseudo_distance(candidate) <= pseudo_radius) {
                position->in_radius.push_back(candidate);
              }
            }
        }
      }
    }
    return position->in_radius.begin();
  }


//   std::vector<object*> gisPosition::at_position(char const label[], int x, int y){
//     std::vector<object*> elements;
//     gisMap *map = map_from_obj(gisObj);
//     if (map == NULL){
//       return elements;
//     }
//     //fill vector
//     return elements;
//   }


