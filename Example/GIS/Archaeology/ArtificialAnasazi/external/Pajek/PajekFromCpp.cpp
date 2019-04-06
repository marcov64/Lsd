/*

  This is a completely rewritten version. Thx to Kyaw Oo for help.

  For more information on Pajek, etc., see PajekFromCpp_macro.h
  The info will move here later.

  Copyright: Frederik Schaff, Ruhr-University Bochum

*/

#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <deque>
#include <map>
#include <vector>
#include <algorithm>

#ifndef CREATEDIR_H //only include once
  #include "CreateDir.h" //to create dirs
#endif

#ifndef Y0isLow
  #define Y0isLow true //switch this off if you want to start top down instead
#endif

/* Some helpers */

inline double rel_pos(double rank, double ranks){
  return (rank-1)/(ranks-1);
}


/* CPP to Pajek below */

inline std::string uniqueID2string (const int ID,const int n_decimals){
    return std::string(n_decimals - std::to_string(ID).length(), ' ') + std::to_string(ID);
}

struct SourceTargetUnique{

  int unique_ID_source;
  int unique_ID_target;
  SourceTargetUnique( int unique_ID_source, int unique_ID_target) : unique_ID_source(unique_ID_source),unique_ID_target(unique_ID_target) {}

};

struct ID_kind {
	int ID;
	std::string kind;
  std::string label;

	ID_kind(int ID, std::string kind, std::string label ) : ID(ID), kind(kind), label(label)  {}
  ID_kind(int ID, std::string kind) : ID(ID), kind(kind), label("")  {}    //why move for string - any benefit?

    //sort by kind, next by id
  friend bool operator<(const ID_kind &a,const ID_kind &b){
		if (a.kind < b.kind ) {
      return true;
    } else if (b.kind < a.kind) {
      return false;
    } else {
      return (a.ID < b.ID );
    }
	}
	friend bool operator==(const ID_kind &a,const ID_kind &b){
		return !(a<b) && !(b<a);
	}

  std::string get_label(int n_zero = 5) const
  {
    if (label=="") {
      std::string t_label = std::to_string(ID);
      int add_zeros = n_zero - std::to_string(ID).length();
      if (add_zeros>=0) {
        t_label = kind + "_" + std::string(add_zeros, '0') + t_label;
      } else if (add_zeros<0)  { //else: exception..
        std::cout << "\nPAJEKFROMCPP Error! Exception at ID_kind.get_label() - add_zeros < 0";
      }
      return t_label;
    } else {
      return label;
    }
  }

};

  //to bind the source vertice (ID_kind) with the target vertice (ID_kind) and the arc/edge relation (relation).
struct unique_Relation {
  ID_kind source;
  ID_kind target;
  std::string relation;
  bool isEdge; //each relation is either edge (unidirected) or arc (directed)
  unique_Relation(ID_kind _source, ID_kind _target, std::string relation, bool isEdge) : source(_source), target(_target), relation(relation), isEdge(isEdge) {
     //in case of consistency checks it is good to have edges from low to high id_kind.
    if (isEdge==true && _target<_source){
      std::swap(source,target);
//       std::cout << "\nSwapped\n"; //checked and ok
    }
  }

    //this representation is clear and easy to understand, but might be less efficient.
  friend bool operator<(const unique_Relation &ur_a, const unique_Relation &ur_b){
        //compare isEdge
      if (ur_a.isEdge==false && ur_b.isEdge==true) {
          return true;
      } else if (ur_b.isEdge==false && ur_a.isEdge==true) {
        return false;

        //compare relation
      } else if (ur_a.relation < ur_b.relation) {
        return true;
      } else if (ur_b.relation < ur_a.relation) {
        return false;

        //compare source
      } else if (ur_a.source < ur_b.source) {
        return true;
      } else if (ur_b.source < ur_a.source) {
        return false;

        //compare target - final comparison!
       } else {
        return ur_a.target < ur_b.target;
       }
  }
  friend bool operator==(const unique_Relation &ur_a, const unique_Relation &ur_b){
    return !(ur_a<ur_b) && !(ur_b<ur_a);
  }

};

//holds all the attributes of a vertice
struct vertice_Attributes{
  //data
	double value;
	double co_X;
	double co_Y;
  std::string shape;
	double x_fact;
	double y_fact;
	std::string color;
  vertice_Attributes(double value, double co_X, double co_Y, std::string shape, double x_fact, double y_fact, std::string color) : value(value), co_X(co_X), co_Y(co_Y), shape(shape), x_fact(x_fact), y_fact(y_fact), color(color) {}

  std::pair <std::string,std::string> as_string() const
  {
    return  std::make_pair<std::string,std::string>(  \
            " " + std::to_string(co_X)  + " " + std::to_string(co_Y) \
         +  " " + std::to_string(value), \
         + " " + shape \
         + " x_fact " + std::to_string(x_fact) + " y_fact " + std::to_string(y_fact) \
         + " ic " + color);
  }

};


struct arc_Attributes{

	double value;
	double width;
	std::string color;

  arc_Attributes(double value, double width, std::string color) : value(value),width(width),color(color) {}

  std::string as_string(bool forPajekToSVGAnim) {
    return  " "   + std::to_string(value) \
         +  " w " + (forPajekToSVGAnim?std::to_string(int(width)) : std::to_string(width) ) \
         +  " c " + color;
  }
};


  //stores summary information
struct Vertice{
  int unique_ID;  //unique in setting. Differs between "Time-line" and "Time-snap" !
	ID_kind id_kind;
  vertice_Attributes attributes; //the TL has also attributes (static init layout)

  std::deque<int> time_active; //only for timeline currently

  Vertice(int unique_ID, ID_kind id_kind, vertice_Attributes attributes) : unique_ID(unique_ID), id_kind(id_kind), attributes(attributes) {}
  void append(int time) { time_active.push_back(time); }

  bool operator<(const ID_kind &id_kind_comp) const
	{
		return this->id_kind < id_kind_comp;
	}
  bool operator==(const ID_kind &id_kind_comp) const
	{
		return this->id_kind == id_kind_comp;
	}

  //print in pajek format
  std::string time_active_as_string() const
  {
    if (time_active.size()>0) {
      std::string temp = "[";
      int start = time_active.front();
      int end = time_active.front();
      temp += std::to_string(start);
      for (const auto &ta : time_active) {
        if (ta > end +1){
          if (end > start) {
            temp += "-" + std::to_string(end);
          }
          temp += ",";
          start = ta;
          temp += std::to_string(start);
          end = ta;
        } else {
          end = ta;
        }
      }
      if (start != end){
        temp += "-" + std::to_string(end);
      }
      temp += "]"; //end
      return temp;
    } else {
      return "";
    }
  }

  std::pair<std::string,std::string> as_string (int n_unique_decimals, int n_decimals, int n_total) const
  {
    std::string temp_label = "\"" + id_kind.get_label(n_decimals) + "\"";
    int add_space = n_total - temp_label.length();
    if (add_space > 0) {
      temp_label = temp_label + std::string(add_space, ' ');
    } else if (add_space<0)  { //else: exception..
      std::cout << "\nPAJEKFROMCPP Error! Exception at Vertice.as_string() - add_space <0";
    }
      std::pair <std::string,std::string> paired_attributes = attributes.as_string();
    return std::make_pair <std::string,std::string>(" " + uniqueID2string(unique_ID, n_unique_decimals) + " " + temp_label \
        +  " " + paired_attributes.first, paired_attributes.second + " " + time_active_as_string());
  }

};


  //stores summary information
struct Arc {

  unique_Relation unique_relation; //the unique mapping identifier
  SourceTargetUnique sourceTargetUnique;
  arc_Attributes attributes;

  std::deque<int> time_active;

  Arc(unique_Relation unique_relation, SourceTargetUnique sourceTargetUnique, arc_Attributes attributes)
        : unique_relation(unique_relation), sourceTargetUnique(sourceTargetUnique), attributes(attributes) {}

  friend bool operator<(const Arc &a, const Arc &b){
    return a.unique_relation < b.unique_relation;
  }
  friend bool operator==(const Arc &a, const Arc &b){
    return a.unique_relation == b.unique_relation;
  }

  void append(int time) { time_active.push_back(time); }

  //print in pajek format
  //print in pajek format
  std::string time_active_as_string() {
    if (time_active.size()>0) {
      std::string temp = "[";
      int start = time_active.front();
      int end = time_active.front();
      temp += std::to_string(start);
      for (const auto &ta : time_active) {
        if (ta > end +1){
          if (end > start) {
            temp += "-" + std::to_string(end);
          }
          temp += ",";
          start = ta;
          temp += std::to_string(start);
          end = ta;
        } else {
          end = ta;
        }
      }
      if (start != end){
        temp += "-" + std::to_string(end);
      }
      temp += "]"; //end
      return temp;
    } else {
      return "";
    }
  }

  std::string as_string (int n_unique_decimals, bool forPajekToSVGAnim){
    return  " " + uniqueID2string(sourceTargetUnique.unique_ID_source, n_unique_decimals) \
        +   " " + uniqueID2string(sourceTargetUnique.unique_ID_target, n_unique_decimals) \
        +  " " + attributes.as_string(forPajekToSVGAnim) + " " + time_active_as_string();
  }

};

// struct TimeOverview{
//
// };

struct TimeOverview{
  std::deque<Vertice>  Vertices_TO;
  std::deque<Arc>      Arcs_TO;
  std::map<ID_kind,Vertice*>       Vertices_TO_map;
  std::map<unique_Relation,Arc*>   Arcs_TO_map;
};

struct TimeSnap{
	int time;

  //deque because pointers are never invalidated
	std::deque<Vertice> Vertices;
	std::deque<Arc> Arcs;

  TimeSnap(int time) : time(time) {}

    //because we need the unique ID, the q is if reducing memory reallocation (this approach) is better than reducing search time (maps approach)
  int get_unique_ID (ID_kind id_kind) const
  {
    for (const auto &ver : Vertices){
      if (ver == id_kind) {
        return ver.unique_ID;
      }
    }
    std::cout << "\nPAJEKFROMCPP Error! Error in get_unique_ID: The vertice does not exist." << std::endl;
    return -1; //error!
  }

  #ifdef PAJEK_CONSISTENCY_CHECK_ON
    //note: Consistency checks are slow (O(n)) - but turning them off, the speed is higher than with using maps, I supose.

    bool is_exist_Relation(unique_Relation in_unique_relation) const
    {
  		for (const auto &it:Arcs)
      {
  		  if((it.unique_relation.relation==in_unique_relation.relation)&& it.unique_relation.source == in_unique_relation.source && it.unique_relation.target ==in_unique_relation.target ){
  			   return true;  //not checking for isEdge
        }
      }
      return false;
    }


    bool is_exist_Vertice(ID_kind id_kind) const
    {
      return (std::find( Vertices.begin(), Vertices.end(), id_kind) !=  Vertices.end() );
    }
  #endif

  bool add_arc(unique_Relation unique_relation,arc_Attributes attributes)
  {

    #ifdef PAJEK_CONSISTENCY_CHECK_ON
    		if (is_exist_Vertice(unique_relation.source)==false || is_exist_Vertice(unique_relation.target)==false ){
          std::cout<<"\nPAJEK_CONSISTENCY_CHECK: relation not added. source or target not existent."<<std::endl;
          return false;
        //-4. consistency check: does the exact relation exist?
        } else if(is_exist_Relation(unique_relation)==true){
          std::cout<<"\nPAJEK_CONSISTENCY_CHECK: existing relation "<<std::endl;
          return false;
    		}
    #endif

      Arcs.emplace_back(unique_relation,SourceTargetUnique(get_unique_ID(unique_relation.source),get_unique_ID(unique_relation.target)),attributes);
      return true;
  }


  bool add_vertice(ID_kind id_kind, vertice_Attributes attributes)
  {

    #ifdef PAJEK_CONSISTENCY_CHECK_ON  //to check for consistency, optional
  		if (is_exist_Vertice(id_kind)==true)
  		{
      			std::cout<<"\nPAJEK_CONSISTENCY_CHECK: vertice exists already" <<std::endl;
            return false;
      }
    #endif

    Vertices.emplace_back(Vertices.size()+1,id_kind,attributes);
    return true;
  }

  std::string as_string(std::string network_name, int n_decimals_unique, int n_decimals, int n_label, bool forPajekToSVGAnim)
  {
    std::string output;
    size_t to_reserve = Vertices.size()*120 + Arcs.size()*60 + 50;
    output.reserve(to_reserve); //a rough estimate
    output = "*Network "  +  network_name + " in time point " + std::to_string(time) +  "\n"; //header
		output += "*Vertices " + std::to_string(Vertices.size()) + "\n";
//     for (auto ver : Vertices) {
// //       std::cout << " " << uniqueID2string(ver.unique_ID,n_decimals) << " " << ver.id_kind.get_label() << " " << ver.id_kind.ID << " " << ver.id_kind.kind << "[";
//       output += ver.as_string(n_decimals_unique,n_decimals,n_label) + "\n";
//     }
//     std::pair<std::string,std::string> vert_string;
    std::string vert_string_second_former="";
    for (const auto &ver : Vertices){           //do not use the map, we need the unique_ID order!
//       vert_string = ver.as_string(n_decimals_unique,n_decimals,n_label);
      std::pair<std::string,std::string> vert_string(ver.as_string(n_decimals_unique,n_decimals,n_label));
      output += vert_string.first;
      if (vert_string.second != vert_string_second_former ){
        vert_string_second_former = vert_string.second;
        output += vert_string.second;
      }
       output += "\n";
    }

    //edge/arcs
    //procedure: we create a vector with known size and sort it to get the right structure
    std::vector<Arc*> sorted_arcs;
    sorted_arcs.reserve(Arcs.size());
    for (auto& item : Arcs){
//       auto* ptr = &item; //for some reason I cannot
//       sorted_arcs.push_back(ptr);
      sorted_arcs.emplace_back(&item);    //important: works only with emplace, not push_back!
    }
//     std::cout<<"Test 1:\n ";
//     for (auto it_Arc : sorted_arcs ){
//       std::cout << it_Arc->as_string(n_decimals_unique) << "\n";
//     }
    std::sort(sorted_arcs.begin(),sorted_arcs.end(),[](auto const &A1, auto const &A2)
    {return *A1<*A2;} );

//     std::cout<<"Test 2 - now sorted:\n ";
//     for (auto it_Arc : sorted_arcs ){
//       std::cout << it_Arc->as_string(n_decimals_unique) << "\n";
//     }

    //we print the info
    std::string relation_name = "";
    std::string relation_kind = "*Arcs";
    bool curent_isEdge = false;
    int count_rel = 0; //ID of the relation

    for (const auto &it_Arc : sorted_arcs ){
      if (it_Arc->unique_relation.relation != relation_name ) {
        if (it_Arc->unique_relation.isEdge != curent_isEdge) {
          curent_isEdge = !curent_isEdge;
          //count_rel = 0; //reset counter, now arcs follow
          relation_kind="*Edges"; //change name
        }
        count_rel++;
        relation_name = it_Arc->unique_relation.relation;
        output += relation_kind + " :" + std::to_string(count_rel) + " \"" + relation_name + "\"" + "\n";
      }
      output += it_Arc->as_string(n_decimals_unique,forPajekToSVGAnim) + "\n";
    }
    output+= "\n";
//     output.shrink_to_fit();
//     std::cout << "String size with "<< Vertices.size() << " vertices and " << Arcs.size() << " relations is: " << output.capacity() << "vs estimate: " << to_reserve << std::endl;
    return output;
  }
};


class Pajek{

  //file will be saved in parent_folder / set_name / set_name _ set_id .paj
  bool paj_error_hard = false;
  std::string parent_folder   = "Networks"; //information of the parentfolder, relative to the program dir
  std::string set_name        = "MyNet";
  int set_id                  = 0; //the id is integer number - e.g. the seed.
  bool initialised            = false;

  // the network will be given a name networ_name
  std::string network_name;
  bool forPajekToSVGAnim = false;   //width integerised, no partition tables, Edges as Arcs (one way, marked)
  bool staticNetMode     = false; //a switch. One may also use pajek to create a single file (.net)

	int cur_time=-1;
  int largest_ID=0; //largest ID in all ID_kinds - to specify #decimals
  int n_label=32; //size of the labels - will be updated later

  //to normalise the sizes
  const double min_coord_offset = 0.000001; //Pajek needs the minimum to be > 0.
  double largest_x_y=min_coord_offset;
  bool is_norm_coords = false;
  bool is_norm_id_label_length = false;

  std::deque<TimeSnap>     timeSnaps; //dynamic data

      //static / overview data, front of paj file. To do: make a struct
  TimeOverview timeOverview;


#ifdef PAJEK_CONSISTENCY_CHECK_ON  //to check for consistency, optional
	std::deque<std::pair<std::string,bool>> Relations;//for consistency checks
                        //relation, isEdge
#endif


  void update_max_x_y (const double coord){
    if (coord>largest_x_y){
      largest_x_y=coord;
    }
  }

  //a function to norm a coordinate. Carful: only use once
  void norm_coord(double &coord){
    coord /= largest_x_y;
    if (coord < min_coord_offset){
      coord = min_coord_offset;
    }
  }


  void norm_coords(){
    if (is_norm_coords == false){
      if (staticNetMode == false) {
        for (auto &ver_TO : timeOverview.Vertices_TO) {
          norm_coord(ver_TO.attributes.co_X);
          norm_coord(ver_TO.attributes.co_Y);
          #if Y0isLow
          ver_TO.attributes.co_Y=1-ver_TO.attributes.co_Y;
          #endif
        }
      }
      for (auto &ts : timeSnaps){
        for (auto &ver : ts.Vertices) {
          norm_coord(ver.attributes.co_X);
          norm_coord(ver.attributes.co_Y);
          #if Y0isLow
            ver.attributes.co_Y = 1-ver.attributes.co_Y;
          #endif
        }
      }
    }
    is_norm_coords = true;
  }


  void norm_id_label_length(){
    auto it_ver = timeOverview.Vertices_TO.begin();
    auto ver_end = timeOverview.Vertices_TO.end();
    if (staticNetMode == true ) {
      it_ver = timeSnaps.at(0).Vertices.begin();
      ver_end = timeSnaps.at(0).Vertices.end();
    }
    for (;it_ver < ver_end; it_ver++){
      largest_ID = std::max(it_ver->id_kind.ID,largest_ID); //update largest ID
      if (it_ver->id_kind.label>""){
        n_label = std::max(n_label, int(it_ver->id_kind.label.length()));
      } else {
        n_label = std::max(n_label, int(it_ver->id_kind.kind.length()+std::to_string(largest_ID).length()+1));
      }
    }
    is_norm_id_label_length = true;
  }

  int n_decimals_uniqueIDs() {
    if (staticNetMode == false ) {
      return std::to_string(timeOverview.Vertices_TO.size()).length();
    } else {
      return std::to_string(timeSnaps.at(0).Vertices.size()).length();
    }
  }

  int n_decimals_IDs() {
    return std::to_string(largest_ID).length();
  }





public:
    Pajek(){ initialised = false; } //default constructor - only provide empty object
    Pajek(std::string parent_folder, std::string set_name, int set_id, std::string _network_name, bool forPajekToSVGAnim=false, bool staticNetMode=false)
    : parent_folder(parent_folder), set_name(set_name), set_id(set_id), network_name(_network_name), forPajekToSVGAnim(forPajekToSVGAnim), staticNetMode(staticNetMode)
    {
      network_name += " (" + std::to_string(set_id) + ")";
      initialised = true;
    }

    void clear() //clear everything for a new initialisation
    {
    paj_error_hard      = false;
    parent_folder   = "Networks"; //information of the parentfolder, relative to the program dir
    set_name        = "MyNet";
    set_id          = 0; //the id is integer number - e.g. the seed.
    initialised     = false;

    network_name="";
    forPajekToSVGAnim = false;   //width integerised, no partition tables, Edges as Arcs (one way, marked)
    staticNetMode     = false; //a switch. One may also use pajek to create a single file (.net)

  	cur_time=-1;
    largest_ID=0; //largest ID in all ID_kinds - to specify #decimals
    n_label=32; //size of the labels - will be updated later

    largest_x_y=min_coord_offset;
    is_norm_coords = false;
    is_norm_id_label_length = false;

      timeSnaps.clear();
      timeOverview = TimeOverview();

      #ifdef PAJEK_CONSISTENCY_CHECK_ON  //to check for consistency, optional
        Relations.clear();
      #endif
    }

    void init(std::string _parent_folder, std::string _set_name, int _set_id, std::string _network_name, bool _forPajekToSVGAnim=false, bool _staticNetMode=false)
    {
      if ( paj_error_hard )
        return;
      if ( initialised == true ){
        clear();
      }
      parent_folder = _parent_folder;
      set_name=_set_name;
      set_id=_set_id;
      network_name=_network_name;
      forPajekToSVGAnim=_forPajekToSVGAnim;
      staticNetMode = _staticNetMode;
      network_name += " (" + std::to_string(set_id) + ")";
      initialised = true;
    }

  void update_set_id(int id) { set_id = id;}

  std::string partition_as_string() const
  {
    std::set<std::string>  Kinds; //for the partition table
    for (const auto &ver : timeOverview.Vertices_TO){
      Kinds.emplace(ver.id_kind.kind);
    }

    std::string output="";
    size_t to_reserve = timeOverview.Vertices_TO.size()*3 + Kinds.size()*50 + 50;
    output.reserve(to_reserve); //a rough estimate


    for (const auto &part_kind : Kinds){
      output += "*Partition \"" + part_kind + "\"\n";
      output += "*Vertices " + std::to_string(timeOverview.Vertices_TO.size()) + "\n";
      for (const auto &ver : timeOverview.Vertices_TO){
        output += std::to_string(ver.id_kind.kind == part_kind?1:0) + "\n";
      }
      output+= "\n";
    }
//     output.shrink_to_fit();
//     std::cout << "String size with "<< timeOverview.Vertices_TO.size() << " vertices and " << Kinds.size() << " kinds is: " << output.capacity() << "vs estimate: " << to_reserve << std::endl;

    return output;
  }

    //each time a vertice is added to a time snap, we also update the time line info
  void update_vertice_TL(int time,const ID_kind &id_kind, const vertice_Attributes &attributes)
  {
    //check if the vertice exists
    auto map_find =  timeOverview.Vertices_TO_map.find(id_kind);

      //if it does not yet exist, create it and add it to the map
    if (map_find == timeOverview.Vertices_TO_map.end()){
      timeOverview.Vertices_TO.emplace_back(timeOverview.Vertices_TO.size()+1,id_kind,attributes);
      timeOverview.Vertices_TO_map.emplace(id_kind,&timeOverview.Vertices_TO.back());
//       largest_ID = std::max(id_kind.ID,largest_ID); //update largest ID
//       if (id_kind.label>""){
//         n_label = std::max(n_label, int(id_kind.label.length()));
//       } else {
//         n_label = std::max(n_label, int(id_kind.kind.length()+std::to_string(largest_ID).length()+1));
//       }
//take out and make a function, like the norm_coords, of it. Necessary for the static version.
    }

    timeOverview.Vertices_TO_map.at(id_kind)->append(time);
  }

    //each time an arc is added to a time snap, we also update the time line info
  void update_arcs_TL(int time, const unique_Relation &unique_relation, const arc_Attributes &attributes)
  {
    //check if the vertice exists
    auto it_map_pos = timeOverview.Arcs_TO_map.find(unique_relation);

    if (it_map_pos == timeOverview.Arcs_TO_map.end()){
      //if it does not yet exist, create it and add it to the map
//       std::cout << "the item does not yet exist in timeOverview.Arcs_TO_map" << std::endl;
      SourceTargetUnique sourceTargetUnique(get_unique_TL_ID(unique_relation.source),get_unique_TL_ID(unique_relation.target) );

      if (sourceTargetUnique.unique_ID_source < 0 || sourceTargetUnique.unique_ID_target < 0){
//         std::cout << "error: The unique relation cannot be added to timeOverview.Arcs_TO because the Vertice objects of source and/or target are missing!" << std::endl;
        return;
      }

      timeOverview.Arcs_TO.emplace_back(unique_relation, sourceTargetUnique, attributes);
//       std::cout << "Added the new element to timeOverview.Arcs_TO. Check: source is " << timeOverview.Arcs_TO.back().unique_relation.source.ID << "/" << timeOverview.Arcs_TO.back().unique_relation.source.kind << " target is "  << timeOverview.Arcs_TO.back().unique_relation.target.ID << "/" << timeOverview.Arcs_TO.back().unique_relation.target.kind << "and relation is " << timeOverview.Arcs_TO.back().unique_relation.relation << " of kind " << (timeOverview.Arcs_TO.back().unique_relation.isEdge? " Edge ": " Arc ") << std::endl;

      auto test = timeOverview.Arcs_TO_map.emplace(unique_relation,&timeOverview.Arcs_TO.back());
      it_map_pos = test.first;
//       std::cout << "Added the new element to timeOverview.Arcs_TO_map. Check: " << (test.second?" Added! ":" already there! ") <<  std::endl;
//       std::cout << ".... Check: source is " << it_map_pos->second->unique_relation.source.ID << "/" << it_map_pos->second->unique_relation.source.kind << " target is "  << it_map_pos->second->unique_relation.target.ID << "/" << it_map_pos->second->unique_relation.target.kind << "and relation is " << it_map_pos->second->unique_relation.relation << " of kind " << (it_map_pos->second->unique_relation.isEdge? " Edge ": " Arc ") << std::endl;
    }

    //why does it not work?
    //timeOverview.Arcs_TO_map.at(unique_relation)->append(time); //for whatever reason, this does NOT work yet!
    it_map_pos->second->append(time); //this does... cost me half a day

//     std::cout<< "Is the key stored at the it pos the same as the initial key?: " << (it_map_pos->first == unique_relation ? "Yes" : "No") << std::endl;

//     std::cout << "added unique relation" << std::endl;
  }

 	void add_vertice(int time,int ID,std::string kind,double value,
						double cor_X,double cor_Y,std::string shape, double x_fact,double y_fact, std::string color,std::string label="")
	{
    if ( paj_error_hard )
      return;
//     std::cout << "Called add_vertice: time " << time << ", ID " << ID << ", kind " << kind << ", value " << value << " ... label " << label << std::endl;
		if (time>cur_time ) //new time-snap
		{		
			cur_time=time;
			timeSnaps.emplace_back(time);
		}
    //consistency check 1
		else if (time<cur_time) //error
		{
			std::cout<<"\nPAJEKFROMCPP Error!: time error"<<std::endl;
        paj_error_hard = true;
			return;
		}
    update_max_x_y(cor_X>cor_Y?cor_X:cor_Y); // info for the norming
    ID_kind id_kind(ID,kind,label); //bind ID and kind to ID_kind unique identifier
    //add info

    if (forPajekToSVGAnim==true){
      if (shape == "man")
        shape = "box";
      else if (shape == "woman")
        shape = "ellipse";
    }

    vertice_Attributes attributes(value,cor_X,cor_Y,shape,x_fact,y_fact,color);

    if (timeSnaps.back().add_vertice(id_kind,attributes) == true ){
//       std::cout<<"vertice added"<<std::endl;
      if (staticNetMode == false) {
        update_vertice_TL(time,id_kind,attributes);
      }
// 		  std::cout<<"new vertice added to TL"<<std::endl;
    }
	}

void add_relation(int time, int source_ID, std::string source_kind,
                  int target_ID , std::string target_kind, bool isEdge, std::string relation,
                  double value, double width ,std::string color)
	{
    if ( paj_error_hard )
      return;
//     std::cout << "Called add_relation: time " << time << ", source ID " << source_ID << ", source kind " << source_kind
//     std::cout << ", target ID " << target_ID << ", target kind " << target_kind << (isEdge?", (Edge) ":",(Arc) ") << ", relation" << relation << ", value " << value << " ... " << std::endl;
    ID_kind source(source_ID,source_kind);
    ID_kind target(target_ID,target_kind);

    arc_Attributes attributes(value,width,color);

    unique_Relation unique_relation(source,target,relation,isEdge);
#ifdef PAJEK_CONSISTENCY_CHECK_ON
    //1. Consistency check: Check if the kind of relation confirms with previous calls
    {
  		bool rel_exists = false;
  		for (const auto &rel : Relations)
  		{
  			if (rel.first==relation){
  				rel_exists =true;
  				if (rel.second==isEdge){
//   					std::cout<<"\nPAJEK_CONSISTENCY_CHECK: relation is present and correct"<<std::endl;
  					break;
  				}
  				else {
  					std::cout<<"\nPAJEK_CONSISTENCY_CHECK: error: relation alreay present but different"<<std::endl;
  					return;
  				}
  			}
  		}

        //if relation does not yet exist, add it to table
  		if (rel_exists == false)
  		{
  			Relations.emplace_back(std::make_pair(relation,isEdge));
//   			std::cout<< "added to Relation " << relation << (isEdge?" (Edge)":" (Arc)") << " to std::deque"<<std::endl;
  		}
    }//end of first consistency check
#endif

    //add edge/arc

      //check if a new time-snap is necessary
		if (time>cur_time )
		{
			cur_time=time;
			timeSnaps.emplace_back(time);
		}

#ifdef PAJEK_CONSISTENCY_CHECK_ON
    //2. Consistency check: time as arrow?
		else if (time<cur_time) //error
		{
			std::cout<<"\nPAJEK_CONSISTENCY_CHECK: time error at add relation cur time is"<<cur_time<<std::endl;
			return;
		}
#endif

    //end add relation
    //timeSnaps.back().Arcs.emplace_back(unique_relation,SourceTargetUnique(-1,-1),attributes);   //-1,-1 -> not yet initialised
    bool addedArc = timeSnaps.back().add_arc(unique_relation,attributes);
    if (addedArc == true) {
//       std::cout << "added relation to back. Check: source is " << timeSnaps.back().Arcs.back().unique_relation.source.ID << "/" << timeSnaps.back().Arcs.back().unique_relation.source.kind << " target is "  << timeSnaps.back().Arcs.back().unique_relation.target.ID << "/" << timeSnaps.back().Arcs.back().unique_relation.target.kind << "and relation is " << timeSnaps.back().Arcs.back().unique_relation.relation << " of kind " << (timeSnaps.back().Arcs.back().unique_relation.isEdge? " Edge ": " Arc ") << std::endl;
      if (staticNetMode == false) {
      update_arcs_TL(time, unique_relation, attributes);
// 		  std::cout<<"relation added tot TL"<<std::endl;
      }
    }
	}

    //return the unique time line id of the vertice, if it exists. else return -1
  int get_unique_TL_ID (ID_kind id_kind)
  {
    auto map_find =  timeOverview.Vertices_TO_map.find(id_kind);

      //if it does not yet exist, create it and add it to the map
    if (map_find == timeOverview.Vertices_TO_map.end()){
      return -1; //does not exist
    } else {
      return map_find->second->unique_ID;
    }
  }


  std::string overview_as_string(std::string network_name, int n_decimals_unique, int n_decimals, int n_label, bool forPajekToSVGAnim) const
  {
    std::string output;
    size_t to_reserve = timeOverview.Vertices_TO.size()*120 + timeOverview.Arcs_TO.size()*60 + 50;
    output.reserve(to_reserve); //a rough estimate
    output = "*Network " + network_name + "\n";
		output += "*Vertices " + std::to_string(timeOverview.Vertices_TO.size()) + "\n";
    std::pair<std::string,std::string> vert_string;
    std::string vert_string_second_former="";
    for (const auto &ver_TL: timeOverview.Vertices_TO){           //do not use the map, we need the unique_ID order!
      vert_string = ver_TL.as_string(n_decimals_unique,n_decimals,n_label);
      output += vert_string.first;
      if (vert_string.second != vert_string_second_former ){
        vert_string_second_former = vert_string.second;
        output += vert_string.second;
      }
       output += "\n";
    }

    //Next edges/arcs

    std::string relation_name = "";
    std::string relation_kind = "*Arcs";
    bool curent_isEdge = false;
    int count_rel = 0; //ID of the relation

    for (const auto &it_Arcs_TL : timeOverview.Arcs_TO_map ){
      if (it_Arcs_TL.first.relation != relation_name ) {
        if (it_Arcs_TL.first.isEdge != curent_isEdge) {
          curent_isEdge = !curent_isEdge;
          //count_rel = 0; //reset counter, now arcs follow
          relation_kind="*Edges"; //change name
        }
        count_rel++;
        relation_name = it_Arcs_TL.first.relation;
        output +=  relation_kind + " :" + std::to_string(count_rel) + " \"" + relation_name + "\"" + "\n";
      }
      output +=  it_Arcs_TL.second->as_string(n_decimals_unique, forPajekToSVGAnim) + "\n";
    }
    output+= "\n";
//     output.shrink_to_fit();
//     std::cout << "String size with "<< timeOverview.Vertices_TO.size() << " vertices and " << timeOverview.Arcs_TO.size() << " relations is: " << output.capacity() << "vs estimate: " << to_reserve << std::endl;

    return output;
  }

  void save_to_file()
  {
    if ( paj_error_hard )
      return;

      //create and open file
    if (makePath(parent_folder.c_str()) == false){
      std::cout << "\nPAJEKFROMCPP Error! Could not create parent folder: " << parent_folder;
        paj_error_hard = true;
    }
    std::string target_dir = parent_folder + "/" + set_name;
    if (makePath(target_dir.c_str()) == false ){
      std::cout << "\nPAJEKFROMCPP Error! Could not create target folder: " << target_dir;
        paj_error_hard = true;
    }
    std::string filetype = ".paj";
    if (staticNetMode == true) {
      filetype = "_t" + std::to_string(cur_time) +".net";
    }
    std::string filename = target_dir + "/" + set_name + "_" + std::to_string(set_id) + filetype;
    std::ofstream pajek_file;
    pajek_file.open(filename,std::ios_base::out | std::ios_base::trunc);
    if (pajek_file.is_open() == false){
      std::cout << "\nPAJEKFROMCPP Error! Could not open output file: " << filename;
        paj_error_hard = true;
    }

      //change some content - this can be more efficient if we make the structs members of Pajek

    norm_coords();
    norm_id_label_length();

      //write content to file

    if (staticNetMode == false) {
      pajek_file << overview_as_string(network_name,n_decimals_uniqueIDs(),n_decimals_IDs(), n_label, forPajekToSVGAnim);
    }
    for (auto it_snap = timeSnaps.begin(); it_snap != timeSnaps.end(); it_snap++) {
      pajek_file << it_snap->as_string(network_name,n_decimals_uniqueIDs(),n_decimals_IDs(),n_label, forPajekToSVGAnim);
    }
    if (forPajekToSVGAnim == false) {
      pajek_file << partition_as_string();
    }

      //close file
    pajek_file.close();
    if (pajek_file.is_open() == true){
      std::cout << "\nPAJEKFROMCPP Error! Could not close output file: " << filename;
        paj_error_hard = true;
    }
  }

  void printall()
  {
    if (staticNetMode == false) {
      std::cout << overview_as_string(network_name,n_decimals_uniqueIDs(),n_decimals_IDs(), n_label, forPajekToSVGAnim);
    }
    for (auto it_snap = timeSnaps.begin(); it_snap != timeSnaps.end(); it_snap++) {
      std::cout << it_snap->as_string(network_name,n_decimals_uniqueIDs(),n_decimals_IDs(),n_label, forPajekToSVGAnim);
    } //snaps end
    if (forPajekToSVGAnim == false){
      std::cout << partition_as_string();
    }
  }

};
