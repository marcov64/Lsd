/*************************************************************

	LSD 9.0 - January 2026
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
	int str_wnd = true;				// presentation of model structure window (bool)

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
	bool tcl_exit = false;			// control for tcl being destroyed
	bool tcl_ok = false;			// control for tcl ready to operate
	bool tk_ok = false;				// control for tk ready to operate
	bool unsavedChange = false;		// control unsaved changes in configuration
	bool unsavedData = false;		// flag unsaved simulation configurations
	char *eq_txt = NULL;			// equation file content
	char *model_make = NULL;		// model makefile options
	char *sens_file = NULL;			// current sensitivity analysis file
	char *system_make = NULL;		// system makefile options
	char cfg_path[ MAX_PATH_LENGTH ] = "";// path of LSD configuration file
	char eq_file[ MAX_PATH_LENGTH ] = "";// equation file name
	char sens_path[ MAX_PATH_LENGTH ] = "";// path of last used sensitivity directory
	const char *res_g = NULL;		// structure window result variable
	int choice;						// Tcl menu control variable (main window)
	int choice_g;					// Tcl menu control variable (structure window)
	int cur_plt_var;				// current graph plot number
	int done_in;					// Tcl menu control variable (log window)
	int elem_count;					// recursive element counter for show elements menu
	int findexSens = 0;				// sequential sensitivity index to filenames
	int platform = 0;				// OS platform (1=Linux, 2=Mac, 3=Windows)
	int stop;						// activity interruption flag (Tcl boolean)
	int watch;						// allow for graph generation interruption (bool)
	lsd::assimilation da;			// data assimilation object
	lsd::description desc;			// element description object
	lsd::object *curr_obj = NULL;	// pointer to current object in browser
	lsd::object *last_obj = NULL;	// pointer to last selected object in structure
	lsd::simulation sim;			// the master GUI simulation object
	std::mt19937 gui_prng;			// internal pseudo-random number generator
	Tcl_Interp *interp = NULL;		// global Tcl interpreter in LSD

	// constant arrays
	const char *group_defaults[ GROUP_OPTIONS_NUM ] = GROUP_OPTIONS_DEFAULT;
	const char *group_options[ GROUP_OPTIONS_NUM ] = GROUP_OPTIONS_NAME;
	const char *lmm_defaults[ LMM_OPTIONS_NUM ] = LMM_OPTIONS_DEFAULT;
	const char *lmm_options[ LMM_OPTIONS_NUM ] = LMM_OPTIONS_NAME;
	const char *lsd_term_src[ LSD_TERM_NUM ] = LSD_TERM_SRC;
	const char *model_defaults[ MODEL_OPTIONS_NUM ] = MODEL_OPTIONS_DEFAULT;
	const char *model_options[ MODEL_OPTIONS_NUM ] = MODEL_OPTIONS_NAME;
	const char *tk_wnd_names[ TK_WIN_NUM ] = TK_WIN_NAME;
	const char *wnd_names[ LSD_WIN_NUM ] = LSD_WIN_NAME;
	const char group_types[ GROUP_OPTIONS_NUM ] = GROUP_OPTIONS_TYPE;
	const char lmm_types[ LMM_OPTIONS_NUM ] = LMM_OPTIONS_TYPE;
	const char model_types[ MODEL_OPTIONS_NUM ] = MODEL_OPTIONS_TYPE;
}
