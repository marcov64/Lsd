/*************************************************************

	LSD 7.1 - December 2018
	written by Marco Valente, Universita' dell'Aquila
	and by Marcelo Pereira, University of Campinas

	This file: Frederik Schaff, Ruhr-University Bochum

	This file: Frederik Schaff, Ruhr-University Bochum
	LSD is distributed under the GNU General Public License

 *************************************************************/

/****************************************************************************************
    ABMAT.CPP
	Some tools for automated summary analysis of simulation runs.

    Similar to the blueprint objects there exist abmat objects.
    Each object corresponds to a variable with the same name in the real model.
    There are 4 basic kinds of abmat objects / statistics:

      Micro : This implies a variable number of objects with this variable.
        Distributions statistics are saved for each timestep.
      Macro : This implies a unique (single) object with this variable.
        Distribution statistics are saved at the end of the timestep.
        Additionally, time-series statistics are saved.
      Comp  : A set of two macro variables. The subject and its correspondance.
        Associative statistics are taken (Distances, Correlation, Association)
        and also distance metrics. Each comparative variable also is a macro v.
      Cond  : A set of two micro variables. The subject and a factorial
        indicator (that basically subsets the micro variables). Distribution
        statistics are saved for each timestep for ich subset.

    Macro and Comp variables need to survive the complete simulation.
    Micro and Cond variables may "die"

    Technically the following tree structure is given:

    abmat --|-- *micro --|--- micro_var_1
            |            |--- micro_var_*
            |
            |-- *macro --|--- macro_var_1
            |            |--- macro_var_*
            |
            |-- *comp  (virtual) : if a *micro variable has a targeted hook, it
            |                      is also a comparative variable.
            |
            |-- *cond  --|--- subject --|--- cond_sub_1 (hooked to cond_assoc_1)
            |            |              |--- cond_sub_*
            |            |
            |            |--- associate --|-- cond_ass_1 (hooked to cond_sub_1)
            |            |                |-- cond_ass_*

    Basically shadow objects are created for each variable. This comes at the
        cost of some overhead, but allows to handle the statistical variables
        as regular LSD variables w.r.t. the saving of the data and also
        reflection in LSD internal analysis tool.


    The overhead of adding the macro variables is justified by the fact that
    the user can in principle (and may have reason to) modify past values of
    any LSD variable. Thus we completely separate the statistical items from
    the model itself.

    For the Comparative variables, we add each item that is compared to
    something else only once and to the macro items. The information about
    comparisons is saved in form of hooks. If variable a is compared to b and c,
    than we add hooks from a to b and c. In this sense a comparative variable is
    just a virtual variable and we do not really represent it by an object.

****************************************************************************************/
#include "decl.h"
#ifdef CPP11

#define NADBL std::numeric_limits<double>::max() // A double not a number
#define FLOAT_PREC_INTVL                                                       \
  5 // the number of "steps" allowed between items to be "equal"


/********************************************
    ABMAT_STATS
    Produce advanced distribution statistics

    We allow only variable names that are short enough, in total max 31 chars
    Elements are:
    [1..6] variable name
    -- in case of a conditional subsetting
    [3] conditional indicator "_c_"
    [1..6] variable 2 name
    [1] "=" (condition)
    [3] 3-char number (the value of the conditional variable)
    -- in case of comparring two variables
    [3] comparison indicator "_v_"
    [1..6] variable 2 name
    --
    [4] stat type (cross-section timeseries)
    [3] "_In" the interval number. The interval info is saved separately.
    [4] stat type (cross time)

    Thus there are 18 chars gone for the specifiers, leaving 13 for the variable names.
    Thus, we need to shorten the variables to 6 chars to allow the full potential.
    For simplicity, any variable longer than 6 chars is shortened to 3 chars and
    ammended with a unique number from 0 to 999, so we allow for maximum 1000
    variables.

    The information of this mapping is later storred in an elements table txt file
    of the format: shortname longname.

    e.g. "var1" + {"_c_" + "var2" + "=" "nnn" +} {"_v_" + "var2" +} "_min"
        + "_In" + "_min"
    to do so, we build a map of the real names for var1 and var2
    and translate them to shortened names (storred in a text file)


    For conditional variables there is an additional field "shr" for percentage
    share of subset in the total set.

    Note: Some usual LSD things, like the search, do not work here as there
    may be more than one object with the same name in one brotherhood chain.

********************************************/

//global variables

std::set <std::pair<int, int> > s_abmat_intervals; //info on abmat intervals
std::map <const char*, variable*> m_abmat_variables; //fast access to all variables for total files.
std::map <const char*, std::string> m_abmat_varnames; //map variable names to shortened ones.
std::map <const char*, std::set<int> > m_abmat_conditions; //save conditioning factors
int i_abmat_varnames; //simple counter for up to 3 digits
int abmat_series_saved;

void abmat_init()
{
    s_abmat_intervals.clear();
    m_abmat_variables.clear();
    m_abmat_varnames.clear();
    m_abmat_conditions.clear();
    i_abmat_varnames = 0;
    abmat_series_saved = 0;
}

const char* lfirst = "first";
const char* lsecond = "second";

const char* lmicro = "*micro";
const char* lmacro = "*macro";
const char* lcond = "*cond";
const char* lcomp = lmacro;

const char* astat_n = "n";
const char* astat_min = "min";
const char* astat_p05 = "p05";
const char* astat_p25 = "p25";
const char* astat_p50 = "p50";
const char* astat_p75 = "p75";
const char* astat_p95 = "p95";
const char* astat_max = "max";
const char* astat_avg = "avg";
const char* astat_sd = "sd";
const char* astat_mae = "mae";
const char* astat_Lcv = "Lcv";
const char* astat_Lsk = "Lsk";
const char* astat_Lku = "Lku";

const char* astat_gam = "gam";
const char* astat_ta = "ta";
const char* astat_tb = "tb";
const char* astat_tc = "tc";
const char* astat_xcr = "xcr";

const char* astat_L1 = "L1";
const char* astat_L2 = "L2";

/********************************************
    ABMAT_ADD_INTERVAL
    Add an interval for the analysis for the totals file.
    Intervals can start with 0 (initial value included) or higher.
    Intervals can also be open (negative end), implying that the
    end-point is determined dynamically.
    Intervals can also be fully dynamic, providing a negative start
    value and end value. In this case, the interval is set appropriately
    at the end of the simulation run.

    For each interval, the actual start and end values are saved
    in the totals file as I_n_start and I_n_end, where "n" is
    the interval identifier. This is handled elsewhere, however.

    It may make sense to dynamically insert intervals during
    the simulation and this is possible with the macro.
********************************************/
void abmat_add_interval( int start, int end )
{
    s_abmat_intervals.emplace(start, end); //add intervall.
}


/********************************************
    GET_ABMAT_VARNAMES_MAP
    Produces a formated string with the mapping of
    the variable names
********************************************/

std::string get_abmat_varnames_map()
{
    std::string varname_mapping;
    for (auto& elem : m_abmat_varnames) {
        varname_mapping += elem.first;
        varname_mapping += "\t";
        varname_mapping += elem.second;
        varname_mapping += "\n";
    }
    return varname_mapping;
}


/********************************************
    ABMAT_VARNAME_CONVERT
    Converts a varname as mentioned above.
    Applies to base variable names, not linked vars.
********************************************/

const char* abmat_varname_convert(const char* lab)
{
    //std::string s_lab = std::string(lab);
    //check if exists, else create
    if (m_abmat_varnames.count(lab) == 0 ) {
        std::string s_short = std::string(lab);
        if (s_short.length() > 6) {
            s_short.resize(3); //drop last chars
            s_short.append( std::to_string(i_abmat_varnames) );
            if (++i_abmat_varnames > 999) {
                sprintf( msg, "error in '%s'.", __func__);
                error_hard( msg, "too many variables to be shortened",
                            "If you need more than 1000, please contact the developer.",
                            true );
                return "";
            }
        }
        m_abmat_varnames[lab] = s_short;
    }
    try {
        return m_abmat_varnames.at(lab).c_str();
    }
    CatchAll("Uups")
}

/********************************************
    GET_ABMAT_VARNAME
    produce the abmat varname as a function of
    - stattype
    - var1lab
    - stat
    - var2lab
    -
********************************************/
std::string get_abmat_varname(Tabmat stattype, const char* condlab, const int condVal)
{
    return get_abmat_varname(stattype, condlab, "", "", condVal);
}

std::string get_abmat_varname(Tabmat stattype, const char* var1lab, const char* statname, const char* var2lab, const int condVal)
{
    std::string varname( abmat_varname_convert(var1lab) );
    //Add 2nd var
    switch (stattype) {
        case a_micro: //nothing to add
        case a_macro: //nothing to add
            break;
        case a_cond:
            varname.append("_c_");
            varname.append( abmat_varname_convert(var2lab) );
        case a_fact: //factorial share info.
            varname.append("=");
            if (condVal < 0 || condVal > 999) {
                sprintf( msg, "error in '%s'.", __func__);
                error_hard( msg, "conditional value is wrong",
                            "Control that it is in 0..999!",
                            true );
                return "";
            }
            varname.append( std::to_string( condVal ) );
            break;
        case a_comp:
            varname.append("_v_");
            varname.append( abmat_varname_convert(var2lab) );
            break;
        default: //irrelevant
            sprintf( msg, "error in '%s'.", __func__);
            error_hard( msg, "defaulting should not happen",
                        "contact the developer.",
                        true );
    }
    //Add stat info
    switch (stattype) {
        case a_micro:
        case a_cond:
        case a_comp: {
                varname.append("_");
                varname.append(statname);
            }
            break;
        case a_macro:
            //nothing to do.
            break;
        case a_fact:
            varname.append("_shr"); //share in 0,1
    }
    return varname; //.c_str();
}



/********************************************
    ABMAT_STATS
    Produce advanced distribution statistics
********************************************/
//dumified version for statnames and/or NAN
m_statsT abmat_stats( void )
{
    std::vector<double> dummy;
    return abmat_stats(dummy);
}

m_statsT abmat_stats(std::vector<double>& Data )
{
    m_statsT stats;

    stats[astat_n]=0; //number of items

    // O-Stats
    stats[astat_min];
    stats[astat_p05]; //lower
    stats[astat_p25]; //lower
    stats[astat_p50]; //interpolate
    stats[astat_p75]; //higher
    stats[astat_p95]; //higher
    stats[astat_max];

    // C-Stats
    stats[astat_avg];
    stats[astat_sd];
    stats[astat_mae];

    // L-Moments
    stats[astat_Lcv];
    stats[astat_Lsk];
    stats[astat_Lku];

    const int len_data = Data.size();
    const double rlen_data = static_cast<double>( len_data );

    if (len_data >= 1) {

        //O-Stats

        std::sort(Data.begin(), Data.end());
        stats[astat_min] = Data[0];
        stats[astat_max] = Data[len_data - 1];

        int index = static_cast<int>( (len_data * 1 / 20) ) - 1;
        if (index < 0)
            index = 0;
        stats[astat_p05] = Data[index];

        index = static_cast<int>( (len_data / 4) ) - 1;
        if (index < 0)
            index = 0;
        stats[astat_p25] = Data[index];

        index = static_cast<int>( std::ceil( rlen_data * 3.0 / 4.0 ) ) - 1;
        if (index > len_data - 1)
            index = len_data - 1;
        stats[astat_p75] = Data[index];

        index = static_cast<int>( std::ceil( rlen_data * 19.0 / 20.0 ) ) - 1;
        if (index > len_data - 1)
            index = len_data - 1;
        stats[astat_p95] = Data[index];

        if (len_data % 2 == 0) {
            index = len_data / 2 - 1;
            stats[astat_p50] = (Data[index] + Data[index + 1]) / 2.0;
        }
        else {
            index = (len_data - 1) / 2;
            stats[astat_p50] = Data[ index ];
        }

        //L-Moments and mean
        //Wang, Q. J. (1996): Direct Sample Estimators of L Moments.
        //In Water Resour. Res. 32 (12), pp. 3617–3619. DOI: 10.1029/96WR02675.
        //Fortran Routine and article unclear about casting. Here all real.
        //intermediates for the L-Moments
        double L1, L2, L3, L4, CL1, CL2, CL3, CR1, CR2, CR3;
        L1 = L2 = L3 = L4 = CL1 = CL2 = CL3 = CR1 = CR2 = CR3 = 0.0;

        //L1 == mean
        for (int i = 1; i <= len_data; i++) {
            double ri = static_cast<double>( i );
            CL1 = ri - 1;
            CL2 = CL1 * (ri - 1.0 - 1.0) / 2.0;
            CL3 = CL2 * (ri - 1.0 - 2.0) / 3.0;
            CR1 = (rlen_data - ri); // KLO:buggy! FS:??
            CR2 = CR1 * (rlen_data - ri - 1.0) / 2.0;
            CR3 = CR2 * (rlen_data - ri - 2.0) / 3.0;
            L1 += Data[i - 1];
            L2 += (CL1 - CR1) * Data[i - 1];
            L3 += (CL2 - 2.0 * CL1 * CR1 + CR2) * Data[i - 1];
            L4 += (CL3 - 3.0 * CL2 * CR1 + 3.0 * CL1 * CR2 - CR3) * Data[i - 1];
        }
        const double C1 = rlen_data; //just for readability
        double C2 = C1 * (rlen_data - 1.0) / 2.0;
        double C3 = C2 * (rlen_data - 2.0) / 3.0;
        double C4 = C3 * (rlen_data - 3.0) / 4.0;
        L1 = L1 / C1;
        stats[astat_avg] = L1;
        L2 = L2 / C2 / 2.0;
        L3 = L3 / C3 / 3.0;
        L4 = L4 / C4 / 4.0;

        stats[astat_Lcv] = (L1 == 0.0) ? 0.0 : L2 / L1; // L-cv
        stats[astat_Lsk] = (L2 == 0.0) ? 0.0 : L3 / L2;     // L-Skewness
        stats[astat_Lku] = (L2 == 0.0) ? 0.0 : L4 / L2;     // L-Kurtosis

        double MAE = 0.0;
        double SD = 0.0;
        for (int i = 0; i < len_data; i++) {
            MAE += std::abs(Data[i] - L1);
            SD += std::pow((Data[i] - L1), 2);
        }
        stats[astat_mae] = MAE / rlen_data;
        SD /= rlen_data;
        stats[astat_sd] = SD > 0 ? sqrt(SD) : 0.0;
    }
    else {
        for(auto& elem : stats) {
            elem.second = NAN;
        }
    }
    stats[astat_n] = rlen_data;
    return stats;
}

/********************************************
    ABMAT_COMPARE
    Produce advanced comparative statistics
    between two variables
********************************************/


m_statsT abmat_compare(std::vector<double>& Data1, std::vector<double>& Data2)
{
    // checks
    double gamma;               // discard ties
    double tau_a, tau_b, tau_c; // correct for ties
    bool constVector;
    double concordant = 0.0, discordant = 0.0, tie = 0.0, tie_a = 0.0,
           tie_b = 0.0, tie_ab = 0.0;
    if (Data1.size() != Data2.size()) {
        sprintf(msg, "error in '%s'. ", __func__);
        error_hard(msg, "Data sizes are different", "Please contact the developer.",
                   true);
    }

    m_statsT compare;

    compare[astat_n]; // length of the timeseries

    // association, i.e. direction without magnitude
    compare[astat_gam]; // gamme correlation
    compare[astat_ta];
    compare[astat_tb];
    compare[astat_tc];

    // standard product moment correlation
    compare[astat_xcr];

    // Differences L-Norms
    compare[astat_L1]; // difference in means
    compare[astat_L2]; // difference as RMSE

    const int len_data = Data1.size();
    const double rlen_data = static_cast<double>(len_data);

    if (!is_const_dbl(Data1) || !is_const_dbl(Data2))
        constVector = false;
    else {
        constVector = true;
    }

    // Norms possible
    if (len_data >= 1) {
        double AE;
        double MAE = 0.0, RMSE = 0.0;
        for (auto it1 = Data1.begin(), it2 = Data2.begin(); it1 < Data1.end();
                ++it1, ++it2) {
            AE = *it1 - *it2;
            if (AE < 0) {
                AE = -AE;
            }
            MAE += AE;
            RMSE += AE * AE;
            if (len_data > 2 && constVector == false) {
                if (isWithinPrecisionInterval(*it1, *(std::next(it1)),
                                              FLOAT_PREC_INTVL) ||
                        isWithinPrecisionInterval(*it2, *(std::next(it2)),
                                                  FLOAT_PREC_INTVL)) {
                    continue; // skip this one
                }
                else if ((*it1 > *(std::next(it1)) && *it2 > *(std::next(it2))) ||
                         (*it1 < * (std::next(it1)) && *it2 < * (std::next(it2)))) {
                    concordant++;
                }

                else if ((*it1 > *(std::next(it1)) && *it2 < * (std::next(it2))) ||
                         (*it1 < * (std::next(it1)) && *it2 > *(std::next(it2)))) {
                    discordant++;
                }
                else {
                    // A tie: within a or b only, or between a and b (can also be within
                    // a and/or b)
                    bool t_a = *it1 == *(std::next(it1));
                    bool t_b = *it2 == *(std::next(it2));
                    if (t_a) {
                        tie_a++;
                    }
                    if (t_b) {
                        tie_b++;
                    }
                }
            }
        }

        MAE /= (len_data);
        RMSE /= (len_data);

        if (RMSE > 0) {
            RMSE = sqrt(RMSE);
        }
        if (len_data > 2 && constVector == false) {
            sprintf(msg, "\nconcordant discordant %g %g", concordant, discordant);
            plog(msg);
            if (concordant + discordant != 0) {
                double S = (concordant - discordant);
                gamma = S / (concordant + discordant);
                tau_a = S / ((Data1.size()) * (Data1.size() - 1));
                tau_b = S / sqrt(double(concordant + discordant + tie_a) *
                                 double(concordant + discordant + tie_b));
                // tau_c
            }
            else {
                gamma = 0.0;
                tau_a = 0.0;
                tau_b = 0.0;
            }
            compare[astat_gam] = gamma; // gamme correlation
            compare[astat_ta] = tau_a;
            compare[astat_tb] = tau_b;
            compare[astat_tc] = NAN;
            // compare["tauC"]=0.0;
        }
        else {
            compare[astat_gam] = NAN; // gamme correlation
            compare[astat_ta] = 0.0;
            compare[astat_tb] = 0.0;
            compare[astat_tc] = NAN;
        }
    }
    else {
        for (auto& elem : compare) {
            elem.second = NAN;
        }
    }

    compare[astat_n] = rlen_data; // only one never NAN
    return compare;
}

bool is_const_dbl(std::vector<double>& Data1)
{
    for (auto it = Data1.begin(); it != Data1.end(); it++) {
        if (!isWithinPrecisionInterval(
                    *it, *Data1.end(),
                    FLOAT_PREC_INTVL)) { // always check against last to mitigate
            // problem of trend.
            return false;
        }
    }
    return true;
}

bool abmat_linked_vars_exists_not(object* oFirst, const char* lVar1, const char* lVar2)
{
    //search in all existing abmat variables of current category for the link
    if ( oFirst == NULL ) {
        sprintf( msg, "error in '%s'. ", __func__ );
        error_hard( msg, "no oFirst NULL",
                    "Please contact the developer.",
                    true );
    }
    if (oFirst->b == NULL)
        return true; //no desc. objects yet.

    object* cur = oFirst->b->head;
    while (cur != NULL) {
        if ( cur->hook == NULL ) {
            sprintf( msg, "error in '%s'. ", __func__ );
            error_hard( msg, "Hook is null",
                        "Please contact the developer.",
                        true );
        }
        if ( strcmp(cur->label, lVar1) == 0 && strcmp(cur->hook->label, lVar2) == 0 ) {
            return false; //link exists
        }
        cur = cur->next;
    }

    return true;
}



/********************************************
    create_abmat_object
    Add the abmat objects for the variable varlab of type type. if type is
    cond or comp, var2lab is the reference variable (in the same object).

    issue: Map for objects (no more unique labs!)

    for macro and comp
********************************************/
void add_abmat_object(std::string abmat_type, char const* varlab, char const* var2lab)
{
    //check if the abmat object associated to the variable
    //exists. Note: It may be included in multiple categories
    //so we check on category level.

    //first check if the variable varLab exists in the model.
    if (root->search_var(root, varlab) == NULL) {
        sprintf( msg, "error in '%s'. Variable %s is not in the model.", __func__, varlab );
        error_hard( msg, "Wrong variable name",
                    "Check your code to prevent this error.",
                    true );
    }

    object* parent = NULL;
    const char* typeLab = NULL;
    Tabmat type;
    if (abmat_type.find("micro") != std::string::npos ) {
        typeLab = lmicro;
        type = a_micro;
    }
    else if (abmat_type.find("macro") != std::string::npos ) {
        typeLab = lmacro;
        type = a_macro;
    }
    else if (abmat_type.find("cond") != std::string::npos ) {
        typeLab = lcond;
        type = a_cond;
    }
    else if (abmat_type.find("comp") != std::string::npos ) {
        typeLab = lcomp;
        type = a_comp;
    }
    else {
        sprintf( msg, "error in '%s'. Type %s is not valid.", __func__, abmat_type.c_str() );
        error_hard( msg, "wrong type",
                    "Check your code to prevent this error.",
                    true );
        return;
    }

    //get the parent for the type, create if it exists not yet
    parent = abmat->search_local(typeLab);
    object* oVar = NULL;
    object* oVar2 = NULL;

    if (parent == NULL) {
        abmat->add_obj(typeLab, 1, false );
        parent = abmat->search_local(typeLab);
    }

    //add the objects
    bool var1_added = false;
    bool var2_added = false;
    if (type == a_micro || type == a_macro || type == a_comp) {
        //Add the variable as an object to the category
        oVar = parent->search_local(varlab);
        if (oVar == NULL) {
            parent->add_obj(varlab, 1, false);
            oVar = parent->search_local(varlab);
            plog("\nAdded object of type ");
            plog(abmat_type.c_str());
            plog(" with name ");
            plog(oVar->label);
            var1_added = true;
        }

        //Comparative vars as pair of macro vars, linked via hooks (single direction)
        if (type == a_comp) {
            var2_added = true;
            oVar2 = parent->search_local(var2lab);
            if (oVar2 == NULL) {
                parent->add_obj(var2lab, 1, false);
                oVar2 = parent->search_local(var2lab);
                plog("\nAdded object of type ");
                plog(abmat_type.c_str());
                plog(" with name ");
                plog(oVar2->label);
            }
            //Add hook from var1 to var2, if not exists.
            bool hook_exists = false;
            for (auto& h : oVar->hooks) {
                if (h == oVar2) {
                    hook_exists = true;
                    break;
                }
            }
            if (false == hook_exists) {
                try {
                    oVar->hooks.push_back(oVar2); //check for integretiy with o_vecT
                    plog("\nAdded link for comparison of variable ");
                    plog(oVar->label);
                    plog(" with variable ");
                    plog(oVar2->label);
                }
                CatchAll("Check if the type o_vecT changed!");
            }
        }

    }
    else if (type == a_cond) {
        //In case we have type cond, we add the variables within the
        //respective subgroups first and second and hook the first one
        //to its conditional, using the hooks (there may be several
        //conditionals)


        //get variable aka "first" and conditional aka "second"
        object* oFirst = parent->search(lfirst);
        if (oFirst == NULL ) {
            parent->add_obj(lfirst, 1, false );
            oFirst = parent->search_local(lfirst);
        }
        object* oSecond = parent->search(lsecond);
        if (oSecond == NULL) {
            parent->add_obj(lsecond, 1, false );
            oSecond = parent->search_local(lsecond);
        }

        //check if the first variable exists or create it
        oVar = oFirst->search_local(varlab);
        if (oVar == NULL) {
            oFirst->add_obj(varlab, 1, false);
            oVar = oFirst->search_local(varlab);
        }

        oVar2 = oSecond->search_local(var2lab);
        if (oVar2 == NULL) {
            oSecond->add_obj(var2lab, 1, false);
            oVar2 = oSecond->search_local(var2lab);
            //add set with all past conditions.
            m_abmat_conditions[oVar2->label]; //create placeholder.
            //do not use var2lab, \0 missing!
        }

        bool hook_exists = false;
        for (auto& h : oVar->hooks) {
            if (h == oVar2) {
                hook_exists = true;
                break;
            }
        }
        //alt: oVar->hooks.count(oVar2)>0

        if (false == hook_exists) {
            try {
                oVar->hooks.push_back(oVar2);
                oVar2->hooks.push_back(oVar);
            }
            CatchAll("Check if the type o_vecT changed!");
            var1_added = var2_added = true; //new unique combination
        }
    }

    // Add the variables
    switch (type) {
        case a_micro:
            if (var1_added == true) {
                auto stats_template = abmat_stats( ); //retrieve map of stats
                //create a variable for each statistic
                for (auto& elem : stats_template) {
                    // plog("\n");
                    // plog(oVar->label);
                    std::string nvarLab = get_abmat_varname ( type, oVar->label, elem.first);
                    // plog(" : ");
                    // plog(nvarLab.c_str());
                    abmat_add_var(oVar, nvarLab.c_str());
                }
            }
            break;

        case a_comp:
            if (var2_added == true) { //same as a macro variable.
                std::string nvarLab2 = get_abmat_varname( a_macro, oVar2->label); //name is same as original, shortened
                abmat_add_var(oVar2, nvarLab2.c_str());
            }
        //and pass to macro case for first vase, too!
        case a_macro:
            if (var1_added == true) {
                //a macro object holds a variable with the same name, maybe shortened.
                std::string nvarLab = get_abmat_varname(type, oVar->label); //name is same as original, shortened
                abmat_add_var(oVar, nvarLab.c_str());
            }
            break;


        case a_cond: {
                //the top objects for the unique pair variable and conditioning
                //variable exist. The rest is dynamically checked/produced
                //in the update procedure
            }
            break;
    }

    //visualise the added variables.
    plog("\n---- Added new variables to abmat ---");
    if (var1_added)
        plog_object_tree_up(oVar);
    if (var2_added)
        plog_object_tree_up(oVar2);
    plog("\n--------------------------------------\n");
    //Add ,,,
}

/********************************************************
    ABMAT_ADD_VAR
    Add an abmat variable to an abmat object
********************************************************/
variable* abmat_add_var(object* parent, char const* lab)
{
    variable* var = parent->add_var_basic(lab, 0, NULL, true, true); //no lags, no values, save
    var->param = 1; //1 is parameter. Other fields are not used.
    var->abmat = true;
    abmat_alloc_save_mem_var(var);
    abmat_series_saved++;
    m_abmat_variables[lab] = var;
    return var;
}

/********************************************************
    ABMAT_ALLOC_SAVE_MEM_VAR
    Allocate the memory to save a variable.
********************************************************/
void abmat_alloc_save_mem_var(variable* cv)
{
    if ( cv->data != NULL ) {
        sprintf( msg, "Error in %s! The variable %s in object %s already has a data field", __func__, cv->up->label, cv->label );
        error_hard( msg, "Fix code",
                    "please contact developers",
                    true );
        return;
    }

    cv->data = new double[ max_step + 1 ];

    if (cv->data == NULL) {
        sprintf( msg, "Error in %s! The variable %s in object %s cannot be saved.", __func__, cv->up->label, cv->label );
        error_hard( msg, "out of memory",
                    "if there is memory available and the error persists,\nplease contact developers",
                    true );
        return;
    }

    if ( cv->num_lag > 0  || cv->param == 1 )
        cv->data[ 0 ] = cv->val[ 0 ];

}

/********************************************************
    PLOG_OBJECT_TREE_UP
    Helper for development issues.
    Visulises the object tree.
    Includes direct parents, all contained vars and all children.
********************************************************/
void plog_object_tree_up(object* startO, bool plotVars)
{
    std::string tree;

    //add parents
    for(object* parent = startO->up; parent != NULL; parent = parent->up) {
        tree.insert(0, "'\n|");
        tree.insert(0, parent->label);
        tree.insert(0, "\n'");
    }
    //add self and variables
    tree += "\n";
    tree += startO->label;
    tree += " :\t";
    int count = 0;
    for(variable* cv = startO->v; cv != NULL; cv = cv->next) {
        tree += " '";
        tree += cv->label;
        tree += "'(";
        if (plotVars) {
            tree += "t=";
            tree += std::to_string(t);
            tree += ",v=";
            tree += std::to_string(cv->data[t]);
            tree += ";";
        }
        tree += (cv->param == 1 ? "Par)," : cv->param == 0 ? "Var)," : "Fun),");
        if(++count % 4 == 0) {
            tree += "\n\t";
        }
    }
    plog(tree.c_str());
}

void update_abmat_vars()
{
    //cycle through ABMAT objects and update the information.
    for (bridge* pb = abmat->b; pb != NULL; pb = pb->next) {
        object* parent = pb->head;
        if (parent == NULL) {
            sprintf( msg, "error in '%s'.", __func__);
            error_hard( msg, "no parent object?",
                        "Contact the developer.",
                        true );
            return;
        }

        Tabmat type; //only micro, macro or conditional here
        if ( strcmp(parent->label, lmicro) == 0 ) {
            type = a_micro;
        }
        else if ( strcmp(parent->label, lmacro) == 0 ) {
            type = a_macro;
        }
        else if ( strcmp(parent->label, lcond) == 0 ) {
            type = a_cond;
        }
        else {
            sprintf( msg, "error in '%s' for object %s.", __func__, parent->label);
            error_hard( msg, "wrong abmat object.",
                        "Contact the developer.",
                        true );
            return;
        }

        //Cycle through all items of that type.
        for (bridge* bVar = parent->b; bVar != NULL; bVar = bVar->next ) {
            object* oVar = bVar->head;
            if (oVar == NULL) {
                sprintf( msg, "error in '%s', kind %s.", __func__, parent->label);
                error_hard( msg, "no head object for abmat kind?",
                            "Contact the developer.",
                            true );
                return;
            }
            for ( ; oVar != NULL; oVar = oVar->next) { //in most cases this is a once-loop. But for cond several objects with same label may exist.

                switch (type) {
                    case a_micro: {
                            std::vector<double> data = root->gatherData_all(oVar->label);
                            auto stats_template = abmat_stats( data ); //retrieve map of stats
                            //save data

                            //visualise the added variables.
                            // plog("\n---- Prep to write data ---");
                            // plog_object_tree_up(oVar);
                            // plog("\n---");

                            for (auto& elem : stats_template) {
                                std::string nvarLab = get_abmat_varname ( type, oVar->label, elem.first);
                                oVar->write( nvarLab.c_str(), elem.second, t );
                            }
                        }
                        break;

                    case a_macro: {
                            //simply copy the current data.
                            double val = root->cal(root, oVar->label, 0);
                            std::string varlab = get_abmat_varname(type, oVar->label);
                            oVar->write(varlab.c_str(), val, t);
                        }
                        break;

                    //Cond is different,because here is a first and second variable, too.

                    case a_cond:
                        //move down below first
                        if ( strcmp(oVar->label, lfirst) == 0 ) {

                            for (bridge* bVar1 = oVar->b; bVar1 != NULL; bVar1 = bVar1->next ) {
                                object* oVar1 = bVar1->head;
                                if (oVar1 == NULL) {
                                    sprintf( msg, "error in '%s', kind %s.", __func__, oVar->label);
                                    error_hard( msg, "no head object for abmat kind?",
                                                "Contact the developer.",
                                                true );
                                    return;
                                }
                                for ( ; oVar1 != NULL; oVar1 = oVar1->next) {

                                    //the top objects for the unique pair variable and conditioning
                                    //variable exist. The rest is dynamically checked/produced.
                                    for (object* oVar2 : oVar1->hooks) {

                                        //create set and copy information of previous sets.
                                        std::map < int, std::vector<double> > cond_vData_map;
                                        try {
                                            for (auto& element : m_abmat_conditions.at(oVar2->label) ) {
                                                cond_vData_map[element]; //create empty set
                                            }
                                        }
                                        CatchAll("Uups")


                                        //gather all data and save in sets per condition/factor
                                        double total = 0.0;
                                        auto cycle_var = next_var(root, oVar1->label, true);
                                        for (variable* curv = cycle_var(); curv != NULL; curv = cycle_var()) {
                                            variable* curv2 = curv->up->search_var_local(oVar2->label);
                                            if (curv2 == NULL) {
                                                sprintf( msg, "variable '%s' is missing in object '%s' in function '%s'", oVar2->label, curv->up->label, __func__ );
                                                error_hard( msg, "variable or parameter not found",
                                                            "check model structure for conditional abmat" );
                                                return;
                                            }
                                            total++;
                                            int condVal = static_cast<int> ( curv2->cal( NULL, 0) ); //same as in object::update()
                                            double val = curv->cal( NULL, 0);
                                            cond_vData_map[condVal].push_back(val);
                                        }

                                        //save data
                                        for (auto& subset : cond_vData_map) {
                                            //save information of relative size to conditional variable
                                            double fraction = static_cast<double>(subset.second.size() ) / total;
                                            //Save info on factors
                                            try {
                                                m_abmat_conditions.at(oVar2->label).insert(subset.first);
                                            }
                                            CatchAll("Uups")

                                            std::string var2lab = get_abmat_varname(a_fact, oVar2->label, subset.first);
                                            variable* fracVar = oVar2->search_var_local(var2lab.c_str());

                                            //create variables the first time a factor appears
                                            if (fracVar == NULL) {
                                                //conditioning variable for fraction tracking
                                                fracVar = abmat_add_var(oVar2, var2lab.c_str());
                                                //conditional variables
                                                auto stats_template = abmat_stats( ); //retrieve map of stats
                                                for (auto& elem : stats_template) {   //create a variable for each statistic
                                                    std::string nvarLab = get_abmat_varname ( type, oVar1->label, elem.first, oVar2->label, subset.first);
                                                    abmat_add_var(oVar1, nvarLab.c_str());
                                                }
                                            }

                                            //update variables
                                            oVar2->write(var2lab.c_str(), fraction, t);
                                            auto stats = abmat_stats(subset.second);
                                            for (auto& stat : stats) {
                                                std::string nvarLab = get_abmat_varname ( type, oVar1->label, stat.first, oVar2->label, subset.first);
                                                oVar1->write(nvarLab.c_str(), stat.second, t);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        break;

                    case a_fact:
                    case a_comp: {
                            plog("\nERROR a_comp / a_fact case should not exist.");
                        }
                        break;
                }
                // //visualise the added data.
                // plog("\n---- Added new data to abmat ---");
                // plog_object_tree_up(oVar, true);
                // plog("\n---");

            }

        }
    }
}


/********************************************
    CONNECT_ABMAT_TO_ROOT
    Adds the existing abmat tree as child to root.
    This allows integration in standard LSD results processing.
    Also, standard clean-up applies.
********************************************/

void connect_abmat_to_root()
{
    //check add_obj_basic for functionality
    //set up to root.
    if (abmat->up != NULL) {
        sprintf( msg, "Error in method '%s'", __func__ );
        error_hard( msg, "ABMAT is already connected to something",
                    "please contact developers",
                    true );
        return;
    }
    abmat->up = root;
    bridge* cb;

    // create bridge
    if ( root->b == NULL )
        cb = root->b = new bridge(abmat->label);
    else {
        for ( cb = root->b; cb->next != NULL; cb = cb->next );
        cb->next = new bridge(abmat->label);
        cb = cb->next;
    }

    if ( cb == NULL ) {
        sprintf( msg, "Error in method '%s'. Cannot link abmat to root.", __func__ );
        error_hard( msg, "out of memory",
                    "if there is memory available and the error persists,\nplease contact developers",
                    true );
        return;
    }

    cb->head = abmat; //link bridge against abmat
    cb->copy = true; //this is a "copy" bridge, meaning it will not empty the subtree when deleted.
    root->b_map.insert( b_pairT ( abmat->label, cb ) );

    //add bridge to root
    //link head to abmat

}

/********************************************
    DISCONNECT_ABMAT_TO_ROOT
    Reverse connection
********************************************/

void disconnect_abmat_from_root()
{
    //check add_obj_basic for functionality
    //unset up from root.
    if (abmat == NULL || abmat->up == NULL) {
        return;
    }
    delete_bridge(abmat);  //because bridge is copy, abmat stays.
    abmat->up = NULL;
}

/********************************************
    ABMAT_TOTAL
    Do the totals analysis
********************************************/

void abmat_total()
{
    //Cycle through the variables
    for (auto var : m_abmat_variables ) {

        //get data
        //auto ts_data =

        //gather time-series stats like runs

        //for comparative variables, gather comp stats


        //gather distributional stats


    }

}

#endif
