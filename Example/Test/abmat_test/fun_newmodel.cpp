//#define EIGENLIB			// uncomment to use Eigen linear algebra library
//#define NO_POINTER_INIT	// uncomment to disable pointer checking

//#include "fun_head_fast.h"

// do not add Equations in this area
#include "fun_head.h"
MODELBEGIN


EQUATION("a")
RESULT(V("id_A")+T)

EQUATION("b")
RESULT(V("id_B") +T)

EQUATION("c")
RESULT(V("id_C") +T)

EQUATION("d1")
RESULT(V("id_D1")  +T)

EQUATION("d2")
RESULT(V("id_D2")  +T)
//////////////////////////////////////////////////////////////////////
EQUATION("allStatsALL_P")
PLOG("\n+++++++++++++++++\"allStatsALL_P at TIME= %g \"+++++++++++++++++",T);
PLOG("\n++++++++++statsALL A++++++++++\n");
PLOG("++++++++++%s++++++++++\n",p->label);

      X_STAT_ALL("a");
PLOG("+++++++++++++++++++++++++++++++++++");
PLOG("\n++++++++++statsALL B++++++++++\n");
PLOG("++++++++++%s++++++++++\n",p->label);

        X_STAT_ALL("b");
        
PLOG("\n++++++++++statsALL C++++++++++\n");
PLOG("++++++++++%s++++++++++\n",p->label);


      X_STAT_ALL("c");
PLOG("\n+++++++++++++++++statsALL C with cond+++++++++++++++++\n");
X_STAT_ALL_CND("c","id_C",">",200);
PLOG("len_data: %g\n",v[0]);
PLOG("min     : %g\n",v[1]);
PLOG("max     : %g\n",v[2]);
PLOG("lq      : %g\n",v[3]);
PLOG("uq      : %g\n",v[4]);
PLOG("L_cv     : %g\n",v[9]);
PLOG("L2     : %g\n",v[10]);
PLOG("L_sk      :%g \n",v[11]);
PLOG("L_ku      :%g \n",v[12]);

PLOG("++++++++++++++++++++++++++++++++++");
PLOG("\n++++++++++statsALL D1++++++++++\n");
PLOG("++++++++++%s++++++++++\n",p->label);

        X_STAT_ALL("d1");
PLOG("++++++++++++++++++++++++++++++++++");
PLOG("\n++++++++++statsALL D2++++++++++\n");
PLOG("++++++++++%s++++++++++\n",p->label);

        X_STAT_ALL("d2");
PLOG("len_data: %g\n",v[0]);
PLOG("min     : %g\n",v[1]);
PLOG("max     : %g\n",v[2]);
PLOG("lq      : %g\n",v[3]);
PLOG("uq      : %g\n",v[4]);
PLOG("L_cv     : %g\n",v[9]);
PLOG("L2     : %g\n",v[10]);
PLOG("L_sk      :%g \n",v[11]);
PLOG("L_ku      :%g \n",v[12]);

PLOG("++++++++++++++++++++++++++++++++++");
RESULT(0 )
/////////////////////////////////////////////////////////////////////
EQUATION("allStatsALL_A")
PLOG("\n+++++++++++++++++\"allStatsALL_A with ID %g TIME=%g\"+++++++++++++++++",V("id_A"),T);
PLOG("\n++++++++++Tseries of A++++++++++\n");
T_STAT("a");
PLOG("len_data: %g\n",v[0]);
PLOG("min     : %g\n",v[1]);
PLOG("max     : %g\n",v[2]);
PLOG("lq      : %g\n",v[3]);
PLOG("mean      : %g\n",v[6]);

PLOG("\n++++++++++Tseries_INVL of A with lag 1++++++++++\n");
if (T>=2)
	T_STAT_INTVL("a",1);
PLOG("len_data: %g\n",v[0]);
PLOG("min     : %g\n",v[1]);
PLOG("max     : %g\n",v[2]);
PLOG("lq      : %g\n",v[3]);
PLOG("mean      : %g\n",v[6]);

if(T==4)
	T_STAT("b");

PLOG("+++++++++++++++++++++++++++++++++++");
PLOG("\n++++++++++statsALL B++++++++++\n");
PLOG("++++++++++%s++++++++++\n",p->label);

        X_STAT_ALL("b");
PLOG("\n++++++++++statsALL C++++++++++\n");
PLOG("++++++++++%s++++++++++\n",p->label);

      X_STAT_ALL("c");
PLOG("\n++++++++++statsALL D1++++++++++\n");
PLOG("++++++++++%s++++++++++\n",p->label);

        X_STAT_ALL("d1");

PLOG("\n++++++++++statsALL D2++++++++++\n");
PLOG("++++++++++%s++++++++++\n",p->label);

        X_STAT_ALL("d2");

RESULT(0 )
/////////////////////////////////////////////////////////////////////////////////////////////////
EQUATION("allStatsALL_B")
PLOG("\n+++++++++++++++++\"allStatsALL_B with ID %g \"+++++++++++++++++",V("id_B"));
PLOG("\n++++++++++statsALL A++++++++++\n");
PLOG("++++++++++%s++++++++++\n",p->label);
PLOG(" ERROR cannot call stats of variable of the above level and skipping for \"a\" \n");

PLOG("\n++++++++++statsALL B++++++++++\n");
PLOG("++++++++++%s++++++++++\n",p->label);
PLOG(" ERROR cannot call stats of variable of the same level and skipping for \"a\" \n");

PLOG("\n++++++++++statsALL C++++++++++\n");
PLOG("++++++++++%s++++++++++\n",p->label);
X_STAT_ALL("c");

PLOG("\n++++++++++statsALL D1++++++++++\n");
PLOG("++++++++++%s++++++++++\n",p->label);
X_STAT_ALL("d1");


PLOG("\n++++++++++statsALL D2++++++++++\n");
PLOG("++++++++++%s++++++++++\n",p->label);
X_STAT_ALL("d2");

RESULT(0 )
/////////////////////////////////////////////////////////////////////
EQUATION("allStatsALL_C")
PLOG("\n+++++++++++++++++\"allStatsALL_C with ID %g \"+++++++++++++++++",V("id_C"));
PLOG("\n++++++++++statsALL A++++++++++\n");
PLOG("++++++++++%s++++++++++\n",p->label);
PLOG(" ERROR cannot call stats of variable of the above level and skipping for \"a\" \n");

PLOG("\n++++++++++statsALL B++++++++++\n");
PLOG("++++++++++%s++++++++++\n",p->label);
PLOG(" ERROR cannot call stats of variable of the above level and skipping for \"b\" \n");

PLOG("\n++++++++++statsALL C++++++++++\n");
PLOG("++++++++++%s++++++++++\n",p->label);
PLOG(" ERROR cannot call from the same level and skipping for \"c\" \n");

PLOG("\n++++++++++statsALL D1++++++++++\n");
PLOG("++++++++++%s++++++++++\n",p->label);
X_STAT_ALL("d1");

PLOG("\n++++++++++statsALL D2++++++++++\n");
PLOG("++++++++++%s++++++++++\n",p->label);
X_STAT_ALL("d2");

RESULT(0 )
/////////////////////////////////////////////////
EQUATION("allStatsALL_D1")
PLOG("\n+++++++++++++++++\"allStatsALL_D1 with ID %g \"+++++++++++++",V("id_D1"));
PLOG("\n++++++++++statsALL A++++++++++\n");
PLOG("++++++++++%s++++++++++\n",p->label);
PLOG(" ERROR cannot call stats of variable of the above level and skipping for \"a\" \n");

PLOG("\n++++++++++statsALL B++++++++++\n");
PLOG("++++++++++%s++++++++++\n",p->label);
PLOG(" ERROR cannot call stats of variable of the above level and skipping for \"b\" \n");

PLOG("\n++++++++++statsALL C++++++++++\n");
PLOG("++++++++++%s++++++++++\n",p->label);
PLOG(" ERROR cannot call stats of variable of the above level and skipping for \"c\" \n");

PLOG("\n++++++++++statsALL D1++++++++++\n");
PLOG("++++++++++%s++++++++++\n",p->label);
PLOG(" ERROR cannot call from the same level and skipping for \"d1\" \n");

PLOG("\n++++++++++statsALL D2++++++++++\n");
PLOG("++++++++++%s++++++++++\n",p->label);
PLOG(" ERROR cannot call from the same level and skipping for \"d2\" \n");

RESULT(0 )


/////////////////////////////////////////////////
EQUATION("allStatsALL_D2")
PLOG("\n+++++++++++++++++\"allStatsALL_D2 with ID %g \"+++++++++++++",V("id_D2"));
PLOG("\n++++++++++statsALL A++++++++++\n");
PLOG("++++++++++%s++++++++++\n",p->label);
PLOG(" ERROR cannot call stats of variable of the above level and skipping for \"a\" \n");

PLOG("\n++++++++++statsALL B++++++++++\n");
PLOG("++++++++++%s++++++++++\n",p->label);
PLOG(" ERROR cannot call stats of variable of the above level and skipping for \"b\" \n");

PLOG("\n++++++++++statsALL C++++++++++\n");
PLOG("++++++++++%s++++++++++\n",p->label);
PLOG(" ERROR cannot call stats of variable of the above level and skipping for \"c\" \n");

PLOG("\n++++++++++statsALL D1++++++++++\n");
PLOG("++++++++++%s++++++++++\n",p->label);
PLOG(" ERROR cannot call from the same level and skipping for \"d1\" \n");

PLOG("\n++++++++++statsALL D2++++++++++\n");
PLOG("++++++++++%s++++++++++\n",p->label);
PLOG(" ERROR cannot call from the same level and skipping for \"d2\" \n");

RESULT(0 )










MODELEND

// do not add Equations in this area

void close_sim( void )
{
	// close simulation special commands go here
}
