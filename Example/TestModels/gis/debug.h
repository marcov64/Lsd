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

This file contains the declarations and new macros for the population model.
It also includes the other core-code files (all not fun_*)
****************************************************/


/********************************************************/
/* Some LSD Macros for debugging                        */
/* Add BEFORE #include externals/debug.h

  . to switch .. off :
    .. testing off:       #define SWITCH_TEST_OFF
    .. verbose mode off:  #define SWITCH_VERBOSE_OFF
    .. tracking off:      #define SWITCH_TRACK_OFF

  . to define .. :
   ..  the maximum steps tracking is active: #define TRACK_SEQUENCE_MAX_T n
      //n>0 is the number of steps tracking is active

/********************************************************/
#ifndef MODULE_DEBUG //GUARD
  #define MODULE_DEBUG
#endif

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
  #include "validate.cpp"
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
