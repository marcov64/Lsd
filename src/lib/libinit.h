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
 LIBINIT.H
 LSD preliminary definition of all C++ classes and types.
 *************************************************************/

#define LSDLIBINIT


// standard libraries
#include <atomic>
#include <condition_variable>
#include <csetjmp>
#include <list>
#include <map>
#include <mutex>
#include <random>
#include <set>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <zlib.h>

// platform-specific libraries
#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#undef DELETE
	#undef THIS
#else
	#include <errno.h>
	#include <signal.h>
	#include <sys/wait.h>
	#include <unistd.h>
	#include <wordexp.h>
#endif

#ifdef __APPLE__
	#include <IOKit/pwr_mgt/IOPMLib.h>
#endif

// third-party C++ libraries
#define PUGIXML_NO_XPATH				// XML library
#define PUGIXML_COMPACT
#include "clib/pugixml.hpp"
#include "clib/eigen.h"					// linear algebra library
#ifndef _TERM_
	#include <tk.h>
#endif


/*************************************************************
 GENERAL TYPE TEMPLATES
 *************************************************************/
#ifdef _WIN32
	typedef HANDLE handleT;
#else
	typedef pid_t handleT;
#endif

typedef pugi::xml_document x_docT;
typedef pugi::xml_node x_nodeT;
typedef pugi::xml_attribute x_attrT;
typedef std::atomic < bool > b_atomT;
typedef std::atomic < int > i_atomT;
typedef std::atomic < long > l_atomT;
typedef std::condition_variable cond_vT;
typedef std::list < int > i_listT;
typedef std::lock_guard < std::mutex > l_guardT;
typedef std::lock_guard < std::recursive_mutex > rec_lguardT;
typedef std::map < int, double > d_mapT;
typedef std::mutex mtxT;
typedef std::recursive_mutex rec_mtxT;
typedef std::set < int > i_setT;
typedef std::set < std::string > str_setT;
typedef std::string strT;
typedef std::thread thrT;
typedef std::thread::id thr_idT;
typedef std::vector < bool > b_vecT;
typedef std::vector < double > d_vecT;
typedef std::vector < int > i_vecT;
typedef std::vector < handleT > hand_vecT;
typedef std::vector < long > l_vecT;
typedef std::vector < strT > str_vecT;
typedef std::vector < std::thread > thr_vecT;
typedef std::vector < std::vector < int > > i2_vecT;
typedef std::vector < std::vector < strT > > str2_vecT;
typedef std::vector < std::list < int > > i_list_vecT;
typedef std::uint_least32_t u_long32T;
typedef std::unique_lock < std::mutex > uniq_lT;
typedef std::unique_lock < std::recursive_mutex > rec_uniqlT;
typedef std::unordered_map < strT, d_mapT > dm_mapT;
typedef std::unordered_map < strT, int > i_mapT;
typedef std::unordered_map < strT, strT > p_mapT;
typedef Eigen::MatrixXd e_matT;
typedef Eigen::VectorXd e_vecT;

namespace lsd
{
/*************************************************************
 LSD CLASSES
 *************************************************************/
	class assim;
	class assimilation;
	class assinstance;
	class asstatevars;
	class bridge;
	class descr;
	class description;
	class dlliblinkage;
	class equation;
	class lattice;
	class lsdstack;
	class netlink;
	class netnode;
	class objattr;
	class objattributes;
	class object;
	class profile;
	class result;
	class sensitivity;
	class simulation;
	class varattr;
	class varattributes;
	class variable;
	class worker;


/*************************************************************
 LSD TYPE TEMPLATES
 *************************************************************/
	typedef std::function < double( const variable *, object * ) > eq_funcT;
	typedef std::list < assim > ass_listT;
	typedef std::list < assim * > asp_listT;
	typedef std::list < descr > desc_listT;
	typedef std::list < objattr > oatt_listT;
	typedef std::list < varattr > vatt_listT;
	typedef std::vector < assim * > ass_vecT;
	typedef std::map < int, ass_vecT > ia_mapT;
	typedef std::map < strT, profile > prof_mapT;
	typedef std::map < thr_idT, worker * > wrk_mapT;
	typedef std::pair < double, object * > o_pairT;
	typedef std::pair < long, object * > n_pairT;
	typedef std::pair < objattr *, bridge * > b_pairT;
	typedef std::pair < varattr *, variable * > v_pairT;
	typedef std::vector < assinstance > ae_vecT;
	typedef std::vector < object * > o_vecT;
	typedef std::vector < simulation > sim_vecT;
	typedef std::vector < simulation * > simp_vecT;
	typedef std::vector < variable * > v_vecT;
	typedef std::unordered_map < double, object * > o_mapT;
	typedef std::unordered_map < long, object * > n_mapT;
	typedef std::unordered_map < objattr *, bridge * > b_mapT;
	typedef std::unordered_map < strT, ass_listT::iterator > ass_mapT;
	typedef std::unordered_map < strT, desc_listT::iterator > desc_mapT;
	typedef std::unordered_map < strT, oatt_listT::iterator > oatt_mapT;
	typedef std::unordered_map < strT, vatt_listT::iterator > vatt_mapT;
	typedef std::unordered_map < strT, eq_funcT > eq_mapT;
	typedef std::unordered_map < varattr *, variable * > v_mapT;
	typedef std::unordered_set < object * > o_setT;
	typedef ass_listT::iterator ass_list_itT;
	typedef ass_mapT::iterator ass_map_itT;
}
