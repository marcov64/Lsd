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

      Micro : This implies a variable number of objects with this variable. Distributions statistics are saved for each timestep.
      Macro : This implies a unique (single) object with this variable. Distribution statistics are saved at the end of the timestep. Additionally, time-series statistics are saved.
      Corr  : A set of two macro variables. The subject and its correspondance. Associative statistics are taken (Distances, Correlation, Association)
      Cond  : A set of two micro variables. The subject and a factorial indicator (that basically subsets the micro variables). Distribution statistics are saved for each timestep for ich subset.

    Macro and Corr variables need to survive the complete simulation.
    Micro and Cond variables may "die"

    Technically the following tree structure is given:

    abmat --|-- *micro --|--- micro_var_1
            |            |--- micro_var_*
            |
            |-- *macro --|--- macro_var_1
            |            |--- macro_var_*
            |
            |-- *corr  --|--- subject --|--- corr_sub_1 (hooked to corr_assoc_1)
            |            |              |--- corr_sub_*
            |            |
            |            |--- associate --|-- corr_ass_1 (hooked to corr_sub_1)
            |            |                |-- corr_ass_*
            |
            |-- *cond  --|--- subject --|--- cond_sub_1 (hooked to cond_assoc_1)
            |            |              |--- cond_sub_*
            |            |
            |            |--- associate --|-- cond_ass_1 (hooked to cond_sub_1)
            |            |                |-- cond_ass_*

    Basically shadow objects are created for each variable. This comes at the cost of some overhead, but
    allows to handle the statistical variables as regular LSD variables w.r.t. the saving of the data
    and also reflection in LSD internal analysis tool.




****************************************************************************************/
#include "decl.h"
#ifdef CPP11

/********************************************
    ABMAT_STATS
    Produce advanced distribution statistics

    We allow only variable names that are short enough, in total max 31 chars
    Elements are:
    [1..6] variable name
    [4] stat type (cross)
    [4] stat type (time)
    [3] conditional indicator
    [1..6] variable 2 name
    [1] "=" (condition)
    [3] 3-char number (the value of the conditional variable)
    [3] "_In" the interval number. The interval info is saved separately.

    Thus there are 18 chars gone for the specifiers, leaving 13 for the variable names.
    Thus, we need to shorten the variables to 6 chars to allow the full potential.
    For simplicity, any variable longer than 6 chars is shortened to 3 chars and
    ammended with a unique number from 0 to 999, so we allow for maximum 1000
    variables.

    The information of this mapping is later storred in an elements table txt file
    of the format: shortname longname.

    e.g. "var1" + "_min" + "_min" + "_c_" + "var2" + "=" "nnn" + "_In"
    to do so, we build a map of the real names for var1 and var2
    and translate them

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
                            "If you need so many, please contact the developer.",
                            true );
                return "";
            }
        }
        m_abmat_varnames[lab] = s_short.c_str();
    }
    return m_abmat_varnames.at(lab);
}

/********************************************
    ABMAT_STATS
    Produce advanced distribution statistics
********************************************/


m_statsT abmat_stats(std::vector<double>& Data )
{
    m_statsT stats;

    stats["n"]; //number of items

    // O-Stats
    stats["min"];
    stats["p05"]; //lower
    stats["p25"]; //lower
    stats["p50"]; //interpolate
    stats["p75"]; //higher
    stats["p95"]; //higher
    stats["max"];

    // C-Stats
    stats["avg"];
    stats["sd"];
    stats["mae"];

    // L-Moments
    stats["Lcv"];
    stats["Lsk"];
    stats["Lku"];

    const int len_data = Data.size();
    const double rlen_data = (double) len_data;

    if (len_data >= 1) {

        //O-Stats

        std::sort(Data.begin(), Data.end());
        stats["min"] = Data[0];
        stats["max"] = Data[len_data - 1];
        int index = (int)(len_data / 4);
        stats["p25"] = Data[index];
        index = (int) std::ceil( rlen_data * 3.0 / 4.0 );
        stats["p75"] = Data[index];
        index = (int) (len_data * 1 / 20);
        stats["p05"] = Data[index];
        index = (int) std::ceil( rlen_data * 19.0 / 20.0 );
        stats["p95"] = Data[index];

        if (len_data % 2 == 0) {
            index = (int)len_data / 2 - 1;
            stats["p50"] = (Data[index] + Data[index + 1]) / 2;
        }
        else {
            stats["p50"] = Data[(int)(len_data - 1) / 2];
        }

        //L-Moments and mean
        //Wang, Q. J. (1996): Direct Sample Estimators of L Moments.
        //In Water Resour. Res. 32 (12), pp. 3617–3619. DOI: 10.1029/96WR02675.
        //Fortran Routine and article unclear about casting. Here all real.
        //intermediates for the L-Moments
        double L1, L2, L3, L4, CL1, CL2, CL3, CR1, CR2, CR3, rlen_data;
        L1 = L2 = L3 = L4 = CL1 = CL2 = CL3 = CR1 = CR2 = CR3 = 0.0;

        //L1 == mean
        for (int i = 1; i <= rlen_data; i++) {
            double ri = (double) i;
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
        const double& C1 = rlen_data; //just for readability
        double C2 = C1 * (rlen_data - 1.0) / 2.0;
        double C3 = C2 * (rlen_data - 2.0) / 3.0;
        double C4 = C3 * (rlen_data - 3.0) / 4.0;
        L1 = L1 / C1;
        stats["avg"] = L1;
        L2 = L2 / C2 / 2.0;
        L3 = L3 / C3 / 3.0;
        L4 = L4 / C4 / 4.0;

        stats["Lcv"] = (L1 == 0.0) ? 0.0 : L2 / L1; // L-cv
        stats["Lsk"] = (L2 == 0.0) ? 0.0 : L3 / L2;     // L-Skewness
        stats["Lku"] = (L2 == 0.0) ? 0.0 : L4 / L2;     // L-Kurtosis

        double MAE = 0.0;
        double SD = 0.0;
        for (int i = 0; i < len_data; i++) {
            MAE += std::abs(Data[i] - L1);
            SD += std::pow((Data[i] - L1), 2);
        }
        stats["mae"] /= rlen_data;
        SD /= rlen_data;
        stats["sd"] = SD > 0 ? sqrt(SD) : 0.0;
    }
    else {
        for(auto& elem : stats) {
            elem.second = NAN;
        }
    }
    stats["n"] = (double)len_data;
    return stats;
}

/********************************************
    create_abmat_object
    Add the abmat objects for the variable varlab of type type. if type is
    cond or corr, var2lab is the reference variable (in the same object).
********************************************/
void add_abmat_object(std::string abmat_type, char const* varlab, char const* var2lab)
{
    //check if the abmat object associated to the variable
    //exists. Note: It may be included in multiple categories
    //so we check on category level.

    //first check if the variable varLab exists in the model.
    if (root->search_var(root,varlab) == NULL){
        sprintf( msg, "error in '%s'. Variable %s is not in the model.", __func__,varlab );
        error_hard( msg, "Wrong variable name",
                    "Check your code to prevent this error.",
                    true );
    }

    object* parent = NULL;
    const size_t s_typeLab = 10;
    char typeLab[s_typeLab];
    enum Ttype {micro, macro, cond, corr};
    Ttype type;
    if (abmat_type.find("micro") != std::string::npos ) {
        snprintf(typeLab, sizeof(char)*s_typeLab, "*micro");
        type = micro;
    }
    else if (abmat_type.find("macro") != std::string::npos ) {
        snprintf(typeLab, sizeof(char)*s_typeLab, "*macro");
        type = macro;
    }
    else if (abmat_type.find("cond") != std::string::npos ) {
        snprintf(typeLab, sizeof(char)*s_typeLab, "*cond");
        type = cond;
    }
    else if (abmat_type.find("corr") != std::string::npos ) {
        snprintf(typeLab, sizeof(char)*s_typeLab, "*corr");
        type = corr;
    }
    else {
        sprintf( msg, "error in '%s'. Type %s is not valid.", __func__, abmat_type.c_str() );
        error_hard( msg, "wrong type",
                    "Check your code to prevent this error.",
                    true );
        return;
    }

    //get the parent for the type, create if it exists not yet
    parent = abmat->search(typeLab);
    if (parent == NULL) {
        abmat->add_obj(typeLab, 0, false );
        parent = abmat->search(typeLab);
    }

    //Add the variable as an object to the category
    parent->add_obj(varlab, 1, false);
    object* oVar = parent->search(varlab);

    //In case we have type cond or corr, add second oVar
    if (type == cond || type == corr) {
        parent->add_obj(var2lab, 1, false);
        object* oVar2 = parent->search(var2lab);
        oVar->hook = oVar2;
        oVar2->hook = oVar;
    }



    //Add the variables
    switch (type) {
        case micro:
            std::vector<double> dummy;
            auto stats_template = abmat_stats( dummy ); //retrieve map of stats
            //create a parameter for each statistic
            for (auto& elem : stats_template) {
                std::string varLab = abmat_varname_convert(oVar->label);
                varLab.append("_");
                varLab.append(std::to_string(elem.second));
                oVar->add_var(varLab.c_str(), -1, NULL, true); //no lags, no values --> only data
                variable* var = oVar->search_var(oVar, varLab.c_str(), true, true, oVar);
                if (var == NULL) {
                    sprintf( msg, "error in '%s'.", __func__ );
                    error_hard( msg, "abmat variable not found",
                                "Contact the developer.",
                                true );
                    return;
                }
                var->param = 0; //0 is parameter. Other fields are not used.
            }

            break;

    }

    //Add ,,,
}
#endif

