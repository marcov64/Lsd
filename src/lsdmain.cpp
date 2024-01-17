/*************************************************************

	LSD 9.0 - January 2024
	written by Marco Valente, Universita' dell'Aquila
	and by Marcelo Pereira, University of Campinas

	Copyright Marco Valente and Marcelo Pereira
	LSD is distributed under the GNU General Public License

	See Readme.txt for copyright information of
	third parties' code used in LSD

 *************************************************************/

/*************************************************************
 LSDMAIN.CPP
 Contains the LSD GUI global variables.
*************************************************************/

#include "LSD.h"


/*********************************
 GLOBAL VARIABLES
 *********************************/

// global variables setting program defaults
bool ignore_eq_file = false;// flag to ignore equation file in configuration file
char err_file[ ] = "LSD.err";// error log file name
int doover = false;			// overwrite results folder (bool)
int overwConf = true;		// overwrite configuration on run flag (bool)
int saveConf = false;		// save configuration on results saving (bool)
int strWindowOn = true;		// control the presentation of the model structure window (bool)

// regular program global variables
bool brCovered = false;		// browser cover currently covered
bool check_save;			// control saving message inside disabled objects
bool eq_dum = false;		// current equation is dummy
bool log_ok = false;		// control for log window available
bool meta_par_in[ META_PAR_NUM ];// flag meta parameter for simulation settings found
bool redrawRoot;			// control for redrawing root window (.)
bool redrawStruc;			// control for redrawing model structure window
bool redrawReq = false;		// flag for asynchronous window redraw request
bool tk_ok = false;			// control for tk ready to operate
char path_sens[ MAX_PATH_LENGTH ] = "";	// path of last used sensitivity directory
char tcl_dir[ MAX_PATH_LENGTH ] = "";	// Tcl/Tk directory
const char *res_g = NULL;	// structure window result variable
int choice_g;				// Tcl menu control variable (structure window)
int elem_count;				// recursive element counter for show elements menu
int macro;					// equations style (macros or C++) (bool)
int platform = 0;			// OS platform (1=Linux, 2=Mac, 3=Windows)
int stop;					// activity interruption flag (Tcl boolean)
object *currObj = NULL;		// pointer to current object in browser
Tcl_Interp *interp = NULL;	// global Tcl interpreter in LSD

// constant arrays
const char *lmm_defaults[ LMM_OPTIONS_NUM ] = LMM_OPTIONS_DEFAULT;
const char *lmm_options[ LMM_OPTIONS_NUM ] = LMM_OPTIONS_NAME;
const char *model_defaults[ MODEL_INFO_NUM ] = MODEL_INFO_DEFAULT;
const char *model_info[ MODEL_INFO_NUM ] = MODEL_INFO_NAME;
const char *wnd_names[ LSD_WIN_NUM ] = LSD_WIN_NAME;
