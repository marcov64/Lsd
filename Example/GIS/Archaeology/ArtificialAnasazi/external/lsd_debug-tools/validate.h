/*************************************************************
    validate.h
    Copyright: Frederik Schaff
    Version: 1.11 (dec 2018)

    Licence: MIT

    usage: just include in your model file ("fun_XXX.cpp") header
    before MODELBEGIN:

    #include "validate.h"


    Commenting is not good yet...

    This file contains methods to allow printing the caller graph.
    It is an additional toolset to the LSD Stack Printing possible in
    online mode. In specific, it allows to print the sequence of actions
    and some information of who called whom.
    It makes use of the opportunity of LSD-GIS to provide unique IDs
    to LSD objects, faciliating the validation / debuging.      

    TEQUATION
     For a fast testing/logging, exchange all EQUATION with TEQUATION.


    New set of Macros (all to be used within equations code):

    - Macros to retrace the order in which actions are performed.

     TRACK_SEQUENCE : If used, it prints information on the current variable
      and object. It is less powerfull than the LSD print stack option as it
      does not measure execution times. But it has other usefuls things. By
      using it when a function is called, not when the execution is finished,
      it is simpler to understand. Inaddition, it makes use of the unique IDs
      of unique objects, printing theIDs information. Thus it becomes more easy
      to see what exactly happens,especially together with user defined printing
      of information.TRACK_SEQUENCE will only trigger in the first
      TRACK_SEQUENCE_MAX_T times.
      
     SWITCH_TRACK_SEQUENCE_ALL : If defined before MODELLBEGIN information for all 
      variables will be printed whenever they are updated, as long as t <= TRACK_SINGLE_TEQUATION_MAX_T     
     .
     TRACK_SEQUENCE_FIRST_OR_LAST : It will only trigger for the first and last
      element of a brotherhood of objects.

     TRACK_SEQUENCE_FIRST_OR_LAST_ALWAYS : triggers also after
      TRACK_SEQUENCE_MAX_T time steps.


    - Macros for profiling. Whenever the argument check_true evaluates to true,
      information is printed.
    SET_LOCAL_CLOCK_X(check_true) : Set a new local clock in the current scope.

    ADD_LOCAL_CLOCK_INFO(test)   : Add info to be printed with the clock
      information. The strings will be conactated.

    ADD_LOCAL_CLOCK_TRACKSEQUENCE : Add special info: Adds the TRACK_SEQUENCE
      info to the clock.

    REPORT_LOCAL_CLOCK_X(check_true) : Report the results of the Local Clock
      (time taken, Info text)

    RESET_LOCAL_CLOCK_X(check_true) : Reset the local clock. If check_true
      evaluates true, information about the reset is printed

    - Macros to switch on/off the execution of test-code sections or messages

    TEST_MODE(check_true){ *user code section* }  : If check_true evaluates to
      true, the code instructions in the paranthesis are processed.

    TEST_ELSE : basically just a "} else {" that allows to define clearly
      different testing conditions.

    VERBOSE_MODE(check_true){ *user code section* }  : If check_true evaluates
      to true, the code instructions in the paranthesis are processed.
      Practically the same as the TEST_MODE, but with the aim to add info
      messages and not validation checks.


    Switching Options:
    The following defines can be used before MODELLBEGIN

    #define USE_ASSERTS
*  **  This Option allows to trigger asserts if a specific time

    #define TRACK_SEQUENCE_MAX_T _steps_
*  **  This Option allows to set a maximum number of _steps_ for the
    TRACK_SEQUENCE. The default is no maximum.

    #define TRACK_SINGLE_TEQUATION_MAX_T _number_
*  **  This Option allows to define a maximum number of objects in a
    brother-hood chain for which individual information is printed in
    TEQUATION mode. The default is 50

    #define NO_WINDOW_TRACKING
*  **  This Option activates the validation macros in NO_WINDOW mode, where they
      are otherwise disabled.

    #define DISABLE_LOCAL_CLOCKS
*  **  Disables the local clocks set, see above

    #define SWITCH_TEST_OFF
*  **  Switch off the conditional tests, see above

    #define SWITCH_VERBOSE_OFF
*  **  Switch off the verbose mode, see above


*************************************************************/

#include <time.h>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <assert.h>

#ifndef TRACK_SEQUENCE_MAX_T
#define TRACK_SEQUENCE_MAX_T max_step
#endif

#ifndef TRACK_SINGLE_TEQUATION_MAX_T
#define TRACK_SINGLE_TEQUATION_MAX_T 5
#endif

#define USE_OLD_ID_LABEL_PATTERN false //a switch to allow the name pattern ID_Label instead of default Label_ID

namespace LSD_VALIDATE {
    struct s_assert_time;

    struct s_assert_time {
        double time; //allowed time
        timespec t_start;
        timespec t_end;

        s_assert_time( double time ) : time(time)
        {
            clock_gettime(CLOCK_MONOTONIC, &t_start);
        };

        bool operator()( )
        {
            clock_gettime(CLOCK_MONOTONIC, &t_end);
            double elapsed = ( (double) (t_end.tv_sec - t_start.tv_sec) ) + ( (double) (t_end.tv_nsec - t_start.tv_nsec) ) / 10.0e9;
            if (elapsed > time){
                plog("\nError in s_assert_time()");
                return false;
            }
            else
                return true;
        };
    };

    //   static bool dummy_has_id;
    //label_id_of_o
    //return the label of the object with its unique ID, if wanted.
    std::string label_id_of_o(object* obj_ = NULL, bool treat_as_id = true )
    {
        //std::string ID_LABEL;
        stringstream sID_LABEL;
        sID_LABEL << "";
        if (obj_ == NULL) {
            return "Null";
        }
        else if (obj_ == root) {
            return "root";
        }
        else {
            try {
                sID_LABEL << obj_->label;
            }
            catch (...) {
                return "Not Root Not Null Not Regular Object";
            }
        }

        if (treat_as_id == true) {
            if ( obj_->is_unique() ) {
                sID_LABEL << " (" << obj_->unique_id() << ")";
            }
            else if (obj_->next != NULL) {

                sID_LABEL << obj_;
            }
        }
        return sID_LABEL.str();
    }

    //return label of variable under computation
    std::string label_of_var_of_o(variable* var = NULL)
    {
        if (var == NULL) {
            return "noVarErr?";
        }
        else {
            try {
                return std::string(var->label);
            }
            catch (...) {
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

    std::string track_source(object* p, object* c = NULL, variable* var = NULL, bool has_id = true)
    {

        //those objects without id are only reported if the are first and/or last.
        std::string first_last_add = "";
        int first_or_last = 0;
        if (!has_id) {
            //first, check if p is the last of its kind.
            if (p->next == NULL) {
                first_or_last++;
                first_last_add = " :: LAST obj of its kind";
            }

            //next, find first object of that kind and check if it is p.
            if (p->up->search(p->label) == p) {  //p is the first of its kind
                first_or_last++;
                first_last_add = " :: FIRST obj of its kind";
            }
            if (first_or_last == 2) {
                //unique element in obj.
                first_last_add = " :: SINGLE element in parent";
            }
        }

        if ( has_id || first_or_last > 0) {
            stringstream ss;
            ss << "";
            try {
                ss << "\n" << std::setw(5) << t;
                ss << " : " << std::setw(40) << label_id_of_o(p);
                ss << " -> " << std::setw(32) << label_of_var_of_o(var);
                ss << " called by " << std::setw(32) << label_id_of_o(c, has_id);
                ss << " " << first_last_add;
                return ss.str();
            }
            catch (...) {
                ss << "\n... error";
                return ss.str();
            }
        }
        else {
            return "";
        }
    }

    int time_call = -1;
    std::string track_sequence(int time, object* p, object* c = NULL, variable* var = NULL, bool has_id = true)
    {
        stringstream ss;
        ss << "";
        if (time != time_call) {
            time_call = time;
            ss << "\n" << setw(80) << " -- -- -- -- -- -- -- -- ";
            ss << "\n" << setw(5) << "" << "Time is now:" << time;
            ss << "\n" << setw(5) << "'t'" << ":" << setw(40) << "'Object'" << "->" << setw(32) << "'Variable'" << " called by " << "'Calling Object'";
        }
        ss << track_source(p, c, var, has_id);
        return ss.str();
    }

    // returns: -2 -error, -1: is root, 0: not head not last, 1: head, 2: last
    int p_is_first_or_last_in_line(object* p)
    {
        if (p == root) {
            return -1; //root
        }
        else if (p->next == NULL) {
            return 2; //last
        }
        else {
            bridge* cb = p->up->b;
            while (cb != NULL && strcmp(cb->head->label, p->label) != 0 ) { //if there is a cb->head and (only then) if this is not of the same type as p
                cb = cb->next;
            }
            if (cb == NULL) {
                return -2; //does not exist
            }
            if (cb->head == p) {
                return 1; //is head;
            }
            else {
                return 0; //is not head
            }
        }
    }

}

/*** Following: a set of macros as API */

//in no window mode, stop all information printing and testing
#ifndef NO_WINDOW_TRACKING
#ifdef NO_WINDOW
#ifndef DISABLE_LOCAL_CLOCKS
#define DISABLE_LOCAL_CLOCKS
#endif
#ifdef USE_ASSERTS
#undef USE_ASSERTS
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

/* Tools for assertions */
#ifdef USE_ASSERTS
#define SET_TIME_CHECK( TIME ) LSD_VALIDATE::s_assert_time assert_time( TIME );  plog("\nSet Time Check");
#define RESET_TIME_CHECK       assert_time = LSD_VALIDATE::s_assert_time(assert_time.time);
#define ASSERT_TIME_CHECK      assert( assert_time( ) );
#define ASSERT_EXPRESSION( EXPR )      assert( EXPR );
#else
#define SET_TIME_CHECK( TIME ) void( TIME );
#define RESET_TIME_CHECK
#define ASSERT_TIME_CHECK
#define ASSERT_EXPRESSION( EXPR ) void( EXPR );
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
      LOG("\n\tCLOCK Setting clock with ID %i",local_clock_id); \
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
      LOG("\n\tCLOCK Re-setting clock with new ID %i",local_clock_id);\
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
          LOG("%s",REPORT_LOCAL_CLOCK_report.c_str()); \
        } \
          LOG("\n\tCLOCK Local clock with ID %i: Seconds elapsed: %g",local_clock_id,elapsed);\
      }\
    }


#define ADD_LOCAL_CLOCK_INFO(text) \
    if (REPORT_LOCAL_CLOCK_DirectPrint){ \
      LOG(text); \
    } else {    \
      REPORT_LOCAL_CLOCK_report += string(text);  \
    }

#define ADD_LOCAL_CLOCK_TRACKSEQUENCE \
      REPORT_LOCAL_CLOCK_report += LSD_VALIDATE::track_sequence(t,p,c,var);

#endif  //defined DISABLE_LOCAL_CLOCKS end


/*  also there is
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

/* Tracking of equations etc., special tracking of objects with "_ID" or "ID".*/

#ifndef SWITCH_TRACK_SEQUENCE_OFF

//   #undef TRACK_SEQUENCE
#define TRACK_SEQUENCE \
  if ( !fast && t <= TRACK_SEQUENCE_MAX_T)  { LOG(LSD_VALIDATE::track_sequence(t,p,c,var).c_str()); };
//   #undef TRACK_SEQUENCE_FIRST_OR_LAST
#define TRACK_SEQUENCE_FIRST_OR_LAST \
    if ( !fast && t <= TRACK_SEQUENCE_MAX_T )  { LOG(LSD_VALIDATE::track_sequence(t,p,c,var,false).c_str()); };
#define TRACK_SEQUENCE_FIRST_OR_LAST_ALWAYS \
    LOG(LSD_VALIDATE::track_sequence(t,p,c,var,false).c_str());
#define TRACK_SEQUENCE_ALWAYS { LOG(LSD_VALIDATE::track_sequence(t,p,c,var).c_str()); };
#define TRACK_SEQUENCE_INFO  LSD_VALIDATE::track_sequence(t,p,c,var).c_str()
#else
#define TRACK_SEQUENCE  void();
#define TRACK_SEQUENCE_FIRST_OR_LAST void();
#define TRACK_SEQUENCE_FIRST_OR_LAST_ALWAYS void();
#define TRACK_SEQUENCE_ALWAYS void();
#define TRACK_SEQUENCE_INFO void();
#endif

#ifdef SWITCH_TRACK_SEQUENCE_ALL
#define TEQUATION( X ) \
    EQUATION( X ) \
    if ( COUNT(p->label) > TRACK_SINGLE_TEQUATION_MAX_T) { \
      TRACK_SEQUENCE_FIRST_OR_LAST  \
    } else {           \
      TRACK_SEQUENCE \
    }
#else
#define TEQUATION EQUATION( X )
#endif
