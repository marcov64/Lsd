/*************************************************************
                                                    May 2018
  LSD Debug module - backend for LSD (least 7.0)
  written by Frederik Schaff, Ruhr-University Bochum

  for infos on LSD see ...

	Copyright Frederik Schaff
  This code is distributed under the GNU General Public License

  The complete package has the following files:
  [0] readme.md          ; readme file with instructions and information on this
                           LSD module.
  [1] debug.h            ; contains the macros and includes other module files
  [2] validate.cpp       ; simple utilities to enhance the validation /
                           debugging of LSD models

 *************************************************************/

/***************************************************
debug.h

Simple utilities to enhance the validation / debugging of LSD models.
Note: First or Last flag is only on parent-level. If the objects are
further down in the tree, this will not be flagged.
****************************************************/

#include <string>


#ifndef TRACK_SEQUENCE_MAX_T
  #define TRACK_SEQUENCE_MAX_T max_step
#endif

#define USE_OLD_ID_LABEL_PATTERN false //a switch to allow the name pattern ID_Label instead of default Label_ID
#define MAX_ID 99999
#define ID_CHAR_BUFF 6 //decimals in MAX_ID +1

namespace LSD_VALIDATE {

//   static bool dummy_has_id;
  //return label of object
  std::string label_id_of_o(object* obj_=NULL, bool treat_as_id=true ){
    std::string ID_LABEL;
    if (obj_==NULL){
      return "Null";
    } else if (obj_ == root) {
      return "root";
    } else {
      try {
        ID_LABEL = std::string(obj_->label);
      }
      catch (...){
        return "Not Root Not Null Not Regular Object";
      }
    }

    if (treat_as_id) { //some may be treated as without id.
      //check if 'LABEL'_ID exists, if yes retrieve it
      //also allow "ID" only
      variable* curr;

      #if !(USE_OLD_ID_LABEL_PATTERN)
      std::string ID_VAR = ID_LABEL + "_ID";
      #else
      std::string ID_VAR = "ID_" + ID_LABEL;
      #endif

      for(curr=obj_->v; curr!=NULL;curr=curr->next)
    		if( (strcmp(ID_VAR.c_str(),curr->label)==0) || (strcmp("ID",curr->label)==0 ) )
    			break;

      if (curr!=NULL) {
      int id=curr->val[ 0 ]; //simply pick the value

        if (id>MAX_ID){
          ID_LABEL += " >MAX_ID";
        } else if (id < -MAX_ID) {
          ID_LABEL += " <-MAX_ID";
        } else {
         char id_buff[ID_CHAR_BUFF];
         snprintf(id_buff,sizeof(char)*ID_CHAR_BUFF,"%3i",id);
          ID_LABEL += " " + std::string(id_buff);
        }
      } else {
        ID_LABEL += " noID";
      }
    }

//     if (*has_id != *dummy_has_id){
//       /* Check if a variable for the info if it is a named object was provided*/
//       if(curr!=NULL)
//         has_id=true;
//       else
//         has_id=false;
//     }

    return ID_LABEL;
  }

  //return label of variable under computation
  std::string label_of_var_of_o(variable* var=NULL){
    if (var==NULL){
      return "noVarErr?";
    } else {
      try{
        return std::string(var->label);
      }
      catch (...){
        return "noLabelForVar?";
      }
    }
  }

  /* To do: Gather same groups of calls (loops) */

  // object* former_callee_obj=NULL;
  // object* former_caller_obj=NULL;
  // int init_former_callee=-1;

  std::vector< std::pair < std::string, int > > non_id_object_vars_called;
  std::string current_callee_type;

  std::string track_source(object* p, object* c=NULL, variable* var=NULL, bool has_id=true){
  //   if (init_former_callee < 0 && (former_caller_obj=!c || former_callee_obj->next != p){
  //     init_former_callee=0;
  //     former_caller_obj=c;
  //     former_callee_obj=p;//(re)initialise
  //   } else {
  //     init_former_callee++;
  //   }

    //those objects without id are only reported if the are first and/or last.
    std::string first_last_add = "";
    int first_or_last = 0;
    if (!has_id){
      //first, check if p is the last of its kind.
      if (p->next == NULL){
        first_or_last++;
        first_last_add = " :: LAST obj of its kind";
      }

      //next, find first object of that kind and check if it is p.
//       object* first = NULL;
      if (p!= root){
        bridge* cb = p->up->b;
        while (cb != NULL && strcmp(cb->head->label,p->label)!=0 ){ //if there is a cb->head and (only then) if this is not of the same type as p
          cb = cb->next;
        }
        if (cb == NULL){
          return "\nError! check 106 in validate.cpp";
        }
        if (cb->head == p){   //p is the first of its kind
          first_or_last++;
          first_last_add = " :: FIRST obj of its kind";
        }
      }
      if (first_or_last == 2){
        //unique element in obj.
        first_last_add = " :: SINGLE element in parent";
      }
    }

    if (has_id || first_or_last>0){
      char buffer[300];
      snprintf(buffer,sizeof(char)*300,"\n%-5i : %-40s -> %-32s called by %-32s %s",t,label_id_of_o(p).c_str(),label_of_var_of_o(var).c_str(),label_id_of_o(c,has_id).c_str(),first_last_add.c_str() );
      return std::string(buffer);
    } else {
      return "";
    }
  }

  int time_call = -1;
  std::string track_sequence(int time, object* p, object* c=NULL, variable* var=NULL, bool has_id=true){
    //of those with has_id false only the first and last element are printed
    std::string track_info = "";
    char buffer[300];
    if (time != time_call){
      time_call = time;
      snprintf(buffer,sizeof(char)*300,"\n%-80s","- -- - ");
      track_info += string(buffer);
      snprintf(buffer,sizeof(char)*300,"\n     Time is now: %i",time);
      track_info += string(buffer);
      snprintf(buffer,sizeof(char)*300,"\n%-5s: %-40s -> %-32s called by %s","'time'","'Object'","'Variable'","'Calling Object'" );
      track_info += string(buffer);
    }
    snprintf(buffer,sizeof(char)*300,"%s",track_source(p,c,var,has_id).c_str());
    track_info += string(buffer);
    return track_info;
  }

  // returns: -2 -error, -1: is root, 0: not head not last, 1: head, 2: last
  int p_is_first_or_last_in_line(object *p){
    if (p == root){
      return -1; //root
    } else if (p->next == NULL) {
      return 2; //last
    } else {
      bridge* cb = p->up->b;
      while (cb != NULL && strcmp(cb->head->label,p->label)!=0 ){ //if there is a cb->head and (only then) if this is not of the same type as p
        cb = cb->next;
      }
      if (cb == NULL){
        return -2; //does not exist
      }
      if (cb->head == p){
        return 1; //is head;
      } else {
        return 0; //is not head
      }
    }
  }

}

#undef ID_CHAR_BUFF
#undef MAX_ID

