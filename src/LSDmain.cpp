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

namespace gui
{
/*************************************************************
 GLOBAL VARIABLES
 *************************************************************/

	// global variables setting program defaults
	bool ignore_eq_file = false;	// ignore equation file in configuration file
	char err_file[ ] = "LSD.err";	// error log file name
	int doover = false;				// overwrite results folder (bool)
	int overwConf = true;			// overwrite configuration on run flag (bool)
	int saveConf = false;			// save configuration on results saving (bool)
	int strWindowOn = true;			// presentation of model structure window (bool)

	// regular program global variables
	bool brCovered = false;			// browser cover currently covered
	bool check_save;				// control saving message inside disabled objects
	bool eq_dum = false;			// current equation is dummy
	bool log_ok = false;			// control for log window available
	bool meta_par_in[ META_PAR_NUM ];// meta parameter for simulation settings found
	bool pause_run;					// pause running simulation
	bool redrawRoot;				// control for redrawing root window (.)
	bool redrawStruc;				// control for redrawing model structure window
	bool redrawReq = false;			// flag for asynchronous window redraw request
	bool scrollB = true;			// scroll box state in current runtime plot
	bool tk_ok = false;				// control for tk ready to operate
	bool unsavedChange = false;		// control unsaved changes in configuration
	bool unsavedData = false;		// flag unsaved simulation configurations
	bool unsavedSense = false;		// control for unsaved sensitivity data
	char *eq_txt = NULL;			// equation file content
	char *sens_file = NULL;			// current sensitivity analysis file
	char eq_file[ MAX_PATH_LENGTH ] = "";// equation file name
	char path_sens[ MAX_PATH_LENGTH ] = "";	// path of last used sensitivity directory
	char tcl_dir[ MAX_PATH_LENGTH ] = "";// Tcl/Tk directory
	const char *res_g = NULL;		// structure window result variable
	int choice;						// Tcl menu control variable (main window)
	int choice_g;					// Tcl menu control variable (structure window)
	int cur_plt;					// current graph plot number
	int done_in;					// Tcl menu control variable (log window)
	int elem_count;					// recursive element counter for show elements menu
	int findexSens = 0;				// sequential sensitivity index to filenames
	int macro;						// equations style (macros or C++) (bool)
	int platform = 0;				// OS platform (1=Linux, 2=Mac, 3=Windows)
	int stop;						// activity interruption flag (Tcl boolean)
	int watch;						// allow for graph generation interruption (bool)
	lsd::object *currObj = NULL;	// pointer to current object in browser
	lsd::object *lastObj = NULL;	// pointer to last selected object in structure
	lsd::simulation sim;			// the single GUI simulation object
	Tcl_Interp *interp = NULL;		// global Tcl interpreter in LSD

	// constant arrays
	const char *lmm_defaults[ LMM_OPTIONS_NUM ] = LMM_OPTIONS_DEFAULT;
	const char *lmm_options[ LMM_OPTIONS_NUM ] = LMM_OPTIONS_NAME;
	const char *lsd_nw_src[ LSD_NW_NUM ] = LSD_NW_SRC;
	const char *model_defaults[ MODEL_INFO_NUM ] = MODEL_INFO_DEFAULT;
	const char *model_info[ MODEL_INFO_NUM ] = MODEL_INFO_NAME;
	const char *tk_wnd_names[ TK_WIN_NUM ] = TK_WIN_NAME;
	const char *wnd_names[ LSD_WIN_NUM ] = LSD_WIN_NAME;
}
