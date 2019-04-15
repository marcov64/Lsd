/*************************************************************
  validate.cpp
  Copyright: Frederik Schaff
  Version: 1.1 (dec 2018)

  Licence: MIT

  usage: just include in your model file ("fun_XXX.cpp") header
  before MODELBEGIN:

  #include "validate.h"

*************************************************************/

/*
* This file contains methods to allow printing the caller graph.
* It is an additional toolset to the LSD Stack Printing possible in
* online mode. In specific, it allows to print the sequence of actions
* and some information of who called whom.
* It makes use of the opportunity of LSD-GIS to provide unique IDs
* to LSD objects, faciliating the validation / debuging.
*
* In non-CPP11 mode the pointer value will be printed as id instead.
*
* There are a few global variables that can be changed to control the printing
* of the stack. See the description of the respective macros in the manual. To
* enable the printing, you need to add the following define
* to the fun_*.cpp file holding the model:
*  #define TRACK_SEQUENCE
*
*
* To change the number of steps tracked.
*
*/

#include <string>


#ifndef TRACK_SEQUENCE_MAX_T
  #define TRACK_SEQUENCE_MAX_T max_step
#endif

#define USE_OLD_ID_LABEL_PATTERN false //a switch to allow the name pattern ID_Label instead of default Label_ID
#define MAX_ID 99999
#define ID_CHAR_BUFF 6 //decimals in MAX_ID +1

namespace LSD_VALIDATE {

//   static bool dummy_has_id;
  //label_id_of_o
  //return the label of the object with its unique ID, if wanted.
  std::string label_id_of_o(object* obj_=NULL, bool treat_as_id=true )
  {
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

    if (treat_as_id == true)
    {
      if ( obj_->is_unique() ){
        ID_LABEL += " (" + std::to_string( int( obj_->unique_id() ) ) + ")";
      } else {
        char buffer[64];
        sprintf(buffer,"%p",obj_);
        ID_LABEL += " (" + std::string(buffer) + ")" ;
      }
      return ID_LABEL;
    }
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

/*** Following: a set of macros as API */
#include <ctime>
#include <string>

//in no window mode, stop all information printing
#ifndef NO_WINDOW_TRACKING
  #ifdef NO_WINDOW
    #ifndef DISABLE_LOCAL_CLOCKS
      #define DISABLE_LOCAL_CLOCKS
    #endif
    #ifdef USE_GDB_DEBUG_GLOBAL
      #undef USE_GDB_DEBUG_GLOBAL
    #endif
    #ifndef SWITCH_TRACK_SEQUENCE_OFF
      #define SWITCH_TRACK_SEQUENCE_OFF
    #endif
    #ifndef SWITCH_VERBOSE_OFF
      #define SWITCH_VERBOSE_OFF
    #endif
    #ifndef SWITCH_TEST_OFF
      #define SWITCH_TEST_OFF
    #endif
  #endif
#endif


#ifdef USE_GDB_DEBUG_GLOBAL
  bool GDB_DEBUG_GLOBAL = false; //use in gdb: watch GDB_DEBUG_GLOBAL
  //to do: measure time instead
  #define SET_LOOP_CHECK int GDB_DEBUG_LOOP_CHECK = 0;
  #define RESET_LOOP_CHECK GDB_DEBUG_LOOP_CHECK = 0;
  #define CHECK_LOOP_CHECK     if (GDB_DEBUG_LOOP_CHECK++ > 100000) { GDB_DEBUG_GLOBAL=!GDB_DEBUG_GLOBAL; PLOG("\nERROR: Very slow process!"); }
#else
  #define SET_LOOP_CHECK
  #define RESET_LOOP_CHECK
  #define CHECK_LOOP_CHECK
#endif

// To get time info, see   http://stackoverflow.com/a/2962914/3895476
//The reporter also provides a string (for printing) to print data if the
//condition is fulfilled. Only used in DirectPrint setting.

#ifdef DISABLE_LOCAL_CLOCKS

  #define SET_LOCAL_CLOCK_X(DirectPrint)     void(DirectPrint);
  #define RESET_LOCAL_CLOCK_X(DirectPrint)   void(DirectPrint);
  #define REPORT_LOCAL_CLOCK_X(mintime)         void(mintime);
  #define ADD_LOCAL_CLOCK_INFO(text)            void(text);
  #define ADD_LOCAL_CLOCK_TRACKSEQUENCE      void();

#else  // DISABLE_LOCAL_CLOCKS not defined

  #define SET_LOCAL_CLOCK_X(DirectPrint) \
    bool REPORT_LOCAL_CLOCK_DirectPrint = DirectPrint;\
    std::string REPORT_LOCAL_CLOCK_report; \
    struct timespec local_start, local_finish;   \
    clock_gettime(CLOCK_MONOTONIC, &local_start); \
    int local_clock_id = int(local_start.tv_sec);\
    if (REPORT_LOCAL_CLOCK_DirectPrint) { \
      PLOG("\n\tCLOCK Setting clock with ID %i",local_clock_id); \
    } else { \
      REPORT_LOCAL_CLOCK_report+="\n\tCLOCK Setting clock with ID " + std::to_string(local_clock_id); \
    }

  #define RESET_LOCAL_CLOCK_X(DirectPrint) \
    REPORT_LOCAL_CLOCK_DirectPrint = DirectPrint;\
    REPORT_LOCAL_CLOCK_report.clear();\
    local_clock_id++;\
    clock_gettime(CLOCK_MONOTONIC, &local_start);\
    local_clock_id = int(local_start.tv_sec);\
    if(REPORT_LOCAL_CLOCK_DirectPrint) {\
      PLOG("\n\tCLOCK Re-setting clock with new ID %i",local_clock_id);\
    } else { \
      REPORT_LOCAL_CLOCK_report+="\n\tCLOCK Re-setting clock with new ID " + std::to_string(local_clock_id); \
    }

  #define REPORT_LOCAL_CLOCK_X(mintime)  \
    {                             \
      clock_gettime(CLOCK_MONOTONIC, &local_finish);\
      double elapsed = (local_finish.tv_sec - local_start.tv_sec);\
             elapsed += (local_finish.tv_nsec - local_start.tv_nsec) / 1000000000.0;\
      if (double(mintime)<elapsed){\
        if (!REPORT_LOCAL_CLOCK_DirectPrint){ \
          PLOG("%s",REPORT_LOCAL_CLOCK_report.c_str()); \
        } \
          PLOG("\n\tCLOCK Local clock with ID %i: Seconds elapsed: %g",local_clock_id,elapsed);\
      }\
    }


  #define ADD_LOCAL_CLOCK_INFO(text) \
    if (REPORT_LOCAL_CLOCK_DirectPrint){ \
      PLOG(text); \
    } else {    \
      REPORT_LOCAL_CLOCK_report += string(text);  \
    }

  #define ADD_LOCAL_CLOCK_TRACKSEQUENCE \
      REPORT_LOCAL_CLOCK_report += TRACK_SEQUENCE_INFO;

#endif  //defined DISABLE_LOCAL_CLOCKS end


/*also there is
       Some wrapers
*/
#define SET_LOCAL_CLOCK                 SET_LOCAL_CLOCK_X(true)
#define SET_LOCAL_CLOCK_RF              SET_LOCAL_CLOCK_X(false)




#define RESET_LOCAL_CLOCK               RESET_LOCAL_CLOCK_X(true)
#define RESET_LOCAL_CLOCK_RF            RESET_LOCAL_CLOCK_X(false)



#define REPORT_LOCAL_CLOCK              REPORT_LOCAL_CLOCK_X(0.0)
#define REPORT_LOCAL_CLOCK_CND(time)    REPORT_LOCAL_CLOCK_X(time)


/* To clearly mark tests and also allow to not run them */

#ifndef SWITCH_TEST_OFF
  #define TEST_MODE(X) if (X)           //Testing on
  #define TEST_ELSE } else {
#else
  #define TEST_MODE(X) if (false && X)  //Testing off
  #define TEST_ELSE
#endif

/* A verbose mode */

#ifndef SWITCH_VERBOSE_OFF
  #define VERBOSE_MODE(X) if (X)          //Verbose on
#else
  #define VERBOSE_MODE(X) if (false && X) //Verbose off
#endif

/* A macro to save the stats withour updating */
#ifndef ABMAT_USE_ANALYSIS
  #define ABORT_STATS
#else
  #define ABORT_STATS V_CHEAT("ABMAT_UPDATE",NULL);
#endif

/* A more severe ABORT, that also stops the computation of the
  current EQUATION but not the subsequent simulations*/
#define ABORT2 \
        ABORT_STATS \
        quit = 1;       \
        END_EQUATION(0.0) //prematurely end the current equation

/* Tracking of equations etc., special tracking of objects with "_ID" or "ID".*/

#ifndef SWITCH_TRACK_SEQUENCE_OFF

//   #undef TRACK_SEQUENCE
  #define TRACK_SEQUENCE \
  if ( t <= TRACK_SEQUENCE_MAX_T)  { PLOG(LSD_VALIDATE::track_sequence(t,p,c,var).c_str()); };
  //   #undef TRACK_SEQUENCE_FIRST_OR_LAST
  #define TRACK_SEQUENCE_FIRST_OR_LAST \
    if ( t <= TRACK_SEQUENCE_MAX_T )  { PLOG(LSD_VALIDATE::track_sequence(t,p,c,var,false).c_str()); };
  #define TRACK_SEQUENCE_FIRST_OR_LAST_ALWAYS \
    PLOG(LSD_VALIDATE::track_sequence(t,p,c,var,false).c_str());
  #define TRACK_SEQUENCE_ALWAYS { PLOG(LSD_VALIDATE::track_sequence(t,p,c,var).c_str()); };
  #define TRACK_SEQUENCE_INFO  LSD_VALIDATE::track_sequence(t,p,c,var).c_str()
#else
  #define TRACK_SEQUENCE  void();
  #define TRACK_SEQUENCE_FIRST_OR_LAST void();
  #define TRACK_SEQUENCE_FIRST_OR_LAST_ALWAYS void();
  #define TRACK_SEQUENCE_ALWAYS void();
  #define TRACK_SEQUENCE_INFO void();
#endif

#define TEQUATION( X ) \
  EQUATION( X ) \
  TRACK_SEQUENCE
