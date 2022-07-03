//#define EIGENLIB			// uncomment to use Eigen linear algebra library
//#define NO_POINTER_INIT	// uncomment to disable pointer checking
#include "fun_head_fast.h"

// do not add Equations in this area

MODELBEGIN


/*****EQUATION FILES*****/

#include "mmm_core_time_step.h"          					// Time Step Variable
#include "mmm_core_calibration.h"               			// Calibration calculation
#include "mmm_core_macro.h"              					// Macro Object Variables and Functions
#include "mmm_core_macro_sfc.h"          					// Macro Object Variables for Stock-Flow Consistency
	#include "mmm_core_classes.h"            				// Income Classes Object Variables
	#include "mmm_core_government.h"         				// Government Object Variables
	#include "mmm_core_external_sector.h"    				// External Sector Object Variables
		#include "mmm_core_entry_exit.h"       				// Entry and Exit Variables (inside Sector Object)
		#include "mmm_core_sector_demand.h"					// Sector Variables for Demand Calculations
		#include "mmm_core_sector_aggregates.h"				// Sector Agreggates and Averages Variables
			#include "mmm_core_firm_rnd.h"					// Firm's R&D Variables			
			#include "mmm_core_firm_production.h"			// Firm's Production Variables
			#include "mmm_core_firm_investment.h"			// Firm's Investment Variables
			#include "mmm_core_firm_sales.h"				// Firm's Sales Variables
			#include "mmm_core_firm_price.h"				// Firm's Price Variables
			#include "mmm_core_firm_profit.h"				// Firm's Profit Variables
			#include "mmm_core_firm_capital.h"				// Firm's Capital Variables
			#include "mmm_core_firm_finance.h"				// Firm's Finance Variables
			#include "mmm_core_firm_inputs.h" 				// Firm's Input Variable      
				#include "mmm_core_capital_goods.h"  		// Capital Goods Object Variables
#include "mmm_core_analysis.h"           					// Variables for Analysis
#include "mmm_core_sector_analysis.h"						// Sector Variables for Analysis

MODELEND

// do not add Equations in this area

void close_sim( void )
{
// close simulation special commands go here
}
