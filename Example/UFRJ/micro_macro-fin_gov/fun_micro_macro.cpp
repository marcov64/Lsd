//#define EIGENLIB			// uncomment to use Eigen linear algebra library
//#define NO_POINTER_INIT	// uncomment to disable pointer checking

#include "fun_head_fast.h"
#define FAST_FULL
#include "fun_support.h"

//Create global pointers
object *country;
object *external;
object *government;
object *financial;
object *consumption;
object *capital;
object *input;
object *centralbank;
object *aclass;
object *bclass;
object *cclass;

MODELBEGIN

/*****EQUATION FILES*****/

#include "fun_time_step.h"          				// Time Step Variable
#include "fun_init_2.h"               				// Initialization Variables
#include "fun_macro.h"              				// Macro Object Variables 
	#include "fun_classes.h"            			// Income Classes Object Variables
	#include "fun_government.h"         			// Government Object Variables
	#include "fun_external_sector.h"    			// External Sector Object Variables
	#include "fun_financial.h"    					// Financial Sector Object Variables
	#include "fun_banks.h"    						// Bank Object Variables
		#include "fun_entry_exit.h"       			// Entry and Exit Variables (inside Sector Object)
		#include "fun_sector_demand.h"				// Sector Variables for Demand Calculations
		#include "fun_sector_aggregates.h"			// Sector Agreggates and Averages Variables
			#include "fun_firm_rnd.h"				// Firm's R&D Variables			
			#include "fun_firm_production.h"		// Firm's Production Variables
			#include "fun_firm_investment.h"		// Firm's Investment Variables
			#include "fun_firm_sales.h"				// Firm's Sales Variables
			#include "fun_firm_price.h"				// Firm's Price Variables
			#include "fun_firm_profit.h"			// Firm's Profit Variables
			#include "fun_firm_finance.h"			// Firm's Finance Variables
			#include "fun_firm_inputs.h" 			// Firm's Input Variable      
				#include "fun_capital_goods.h"  	// Capital Goods Object Variables
#include "fun_analysis.h"           				// Variables for Analysis



MODELEND

// do not add Equations in this area

void close_sim( void )
{
// close simulation special commands go here
}
