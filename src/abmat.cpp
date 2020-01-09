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
      Fact  : Sometimes one might also only be interested in the way a factorial
              distribution changes. The factorial can thus also be invoked
              itself. It is possible to cheat by providing a negative value,
              this will endogenise the behaviour with unchecked consequences.
              Note: In general positive (incl. 0) values from 0 to 999 are ok.


      Note: For Factorials (also if from Cond) the factors need to be provided
        from the beginning. The reason is that otherwise a consistent attribution
        over time is not possible.

      You can set a switch variable abmat_dynamic_factors_allowed=true
        to allow automatic handling of factors, but this
        is currently not handled in the totals file. The problem is that each
        a changing number of variables is associated with this behaviour, which
        leads to an exception (the file format is static).

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
            |                      is also a comparative variable. The comp is
            |                      addad as macro variable automatically.
            |
            |-- *cond  --|--- cond_sub_1 (hooked to cond_ass_1)
            |            |--- cond_sub_*
            |            |
            |-- *fact  --|--- cond_ass_1 (hooked to cond_sub_1)
            |            |-- cond_ass_*

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
    [4] stat type (cross-section point in time)
    [3] "_In" the interval number. The interval info is saved separately.
    [4] stat type (cross time / comparison)

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
bool abmat_dynamic_factors_allowed;
bool abmat_short_names;

void abmat_init( )
{

  s_abmat_intervals.clear();
  m_abmat_variables.clear();
  m_abmat_varnames.clear();
  m_abmat_conditions.clear();
  i_abmat_varnames = 0;
  abmat_series_saved = 0;
  abmat_dynamic_factors_allowed = false;
  abmat_short_names = false;
  abmat_create_tree();
}

void abmat_use_long_names()
{
  abmat_short_names = true;
  plog("\n(ABMAT) : Not shortening variable names.");
  return;
}

void abmat_allow_dynamic_factors()
{
  if (abmat_dynamic_factors_allowed) {
    return;    //done
  }
  
  if (sim_num == 1) {
    abmat_dynamic_factors_allowed = true;
    plog("\n(ABMAT) : Factors are recognised dynamically.");
  }
  else {
    abmat_dynamic_factors_allowed = false;
    plog("\n(ABMAT) : ERROR: Cannot allow dynamic factor recognition");
  }
}

const int a_time_init = 1; //Time of initialisation / gathering of parameter values

//Constants for labels and text output

const char* lmicro = "micro";
const char* lmacro = "macro";
const char* lcond = "cond"; //conditional on factor
const char* lfact = "fact"; //factor
const char* lpstat = "P_Static";
const char* lpmic = "P_Micro";
const char* lpmac = "P_Macro";
const char* lfmic = "F_Micro";
const char* lfmac = "F_Macro";
const char* lpLSD = "P_LSD";

const char* apar_stat = "PStat";
const char* apar_mic = "PMicro";
const char* apar_mac = "PMacro";
const char* afin_mic = "FMicro";
const char* afin_mac = "FMacro";
const char* apar_LSD = "PLSD";

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

const char* abmat_varname_convert( const char* lab)
{
  //std::string s_lab = std::string(lab);
  //check if exists, else create
  if (m_abmat_varnames.count(lab) == 0 ) {
    std::string s_short = std::string(lab);

    if (false == abmat_short_names ) {
    
      if (s_short.length() > MAX_ABMAT_BASEVAR_LENGTH) {
        s_short.resize(MAX_ABMAT_BASEVAR_LENGTH - 3); //drop last chars
        s_short.append( std::to_string(i_abmat_varnames) );
        
        if (++i_abmat_varnames > 999) {
          error_hard( __DEV_ERR_INFO__, "too many variables to be shortened",
                      "If you need more than 1000, please contact the developer.",
                      true );
          return "";
        }
      }
    }
    
    m_abmat_varnames[lab] = s_short;
  }
  
  try {
    return m_abmat_varnames.at(lab).c_str();
  }
  
  CatchAll("Uups")
  
  return ""; //no compiler warnings.
}

/********************************************
    GET_ABMAT_VARNAME
    produce the abmat varname as a function of
    - stattype
    - var1lab
    - stat
    - var2lab
    These are the names for the time-series!
    Except for comparatives (there is no time-series).
********************************************/
std::string get_abmat_varname_fact_total( const char* condlab)
{
  return get_abmat_varname(a_fact, condlab, "", "", 0, true);
}


std::string get_abmat_varname_fact( const char* condlab, const int condVal)
{
  return get_abmat_varname(a_fact, condlab, "", "", condVal, false);
}

std::string get_abmat_varname_comp(const char* var1lab, const char* var2lab)
{
  return get_abmat_varname(a_comp, var1lab, "", var2lab, 0, false);
}

std::string get_abmat_varname(Tabmat stattype, const char* var1lab, const char* statname, const char* var2lab, const int condVal, bool flag_fact_n)
{
  std::string varname;
  
  if (stattype == a_pLSD) {//do not convert LSD names
    varname = var1lab;
  }
  else {
    varname = abmat_varname_convert(var1lab);
  }
  
  //Add 2nd var
  switch (stattype) {
    case a_pLSD:
      varname.insert(0, "_");
      varname.insert(0, apar_LSD);
      return varname;
      
    case a_pstat:
      varname.insert(0, "_");
      varname.insert(0, apar_stat);
      return varname;
      
    case a_pmac:
      varname.insert(0, "_");
      varname.insert(0, apar_mac);
      return varname;
      
    case a_pmic:
      varname.insert(0, "_");
      varname.insert(0, apar_mic);
      break;

    case a_fmac:
      varname.insert(0, "_");
      varname.insert(0, afin_mac);
      return varname;
      
    case a_fmic:
      varname.insert(0, "_");
      varname.insert(0, afin_mic);
      break;
      
    case a_macro:
      return varname;//nothing to add ever
      
      
    case a_comp:
      varname.append("_v_");
      varname.append( abmat_varname_convert(var2lab) );
      return varname;
      
    case a_micro:
      break;//nothing to add
      
    case a_cond:
      varname.append("_c_");
      varname.append( abmat_varname_convert(var2lab) );
      
    case a_fact: //factorial share info.
      if (!flag_fact_n) {
        varname.append("=");
        
        if (condVal < 0 || condVal > 999) {
          error_hard( __DEV_ERR_INFO__, "conditional value is wrong",
                      "Control that it is in 0..999!",
                      true );
          return "";
        }
        
        varname.append( std::to_string( condVal ) );
      }
      
      break;
      
    default: //irrelevant
      error_hard( __DEV_ERR_INFO__, "defaulting should not happen (1)",
                  "contact the developer.",
                  true );
  }
  
  //Add stat info
  switch (stattype) {
    case a_pmic:
    case a_micro:
    case a_cond: {
        varname.append("_");
        varname.append(statname);
      }
      
      return varname;
      
    case a_fact:
      if (flag_fact_n) {
        varname.append("_tot");    //total n
      }
      else {
        varname.append("_shr");    //share in 0,1
      }
      
      return varname;
      
    default:
      error_hard( __DEV_ERR_INFO__, "defaulting should not happen (2)",
                  "contact the developer.",
                  true );
      return "";
  }
}



/********************************************
    ABMAT_STATS
    Produce advanced distribution statistics
********************************************/
//dumified version for statnames and/or NAN
ms_statsT abmat_stats( void )
{
  std::vector<double> dummy;
  return abmat_stats(dummy);
}

ms_statsT abmat_stats(std::vector<double>& Data )
{
  ms_statsT stats;
  
  stats[astat_n] = 0; //number of items
  
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
    
    //floor for < 50%
    int index = static_cast<int>( (rlen_data / 20.0) ) - 1;
    
    if (index < 0) {
      index = 0;
    }
    
    stats[astat_p05] = Data[index];
    
    index = static_cast<int>( (rlen_data / 4.0) ) - 1;
    
    if (index < 0) {
      index = 0;
    }
    
    stats[astat_p25] = Data[index];
    
    //ceil for > 50%
    
    index = static_cast<int>( std::ceil( rlen_data * 3.0 / 4.0 ) ) - 1;
    stats[astat_p75] = Data[index];
    
    index = static_cast<int>( std::ceil( rlen_data * 19.0 / 20.0 ) ) - 1;
    stats[astat_p95] = Data[index];
    
    //interpolate median
    
    if (len_data % 2 == 0) {
      index = (len_data / 2) - 1;
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
    for (auto& elem : stats) {
      elem.second = NAN;
    }
  }
  
  stats[astat_n] = rlen_data;
  return stats;
}

/********************************************
    ABMAT_COMPARE
    Produce advanced comparative statistics
    for two variables, denoting as association
    NOTE: Only the partial case, i.e. looking at changes from t to t+1.
    and not completely comparing the ranks (other use of gamma and tau)
    a) Pearson's product moment correlation
    b) Goodman and Kruskal’s Gamma
    c) Kendall's Tau a and b

    NOTE: Kendall's Tau is probably not correct!
********************************************/
ms_statsT abmat_compare( void )
{
  std::vector<double> dummy;
  return abmat_compare(dummy, dummy);
}

ms_statsT abmat_compare(std::vector<double> const& Data1, std::vector<double> const& Data2)
{

  ms_statsT compare;
  
  compare[astat_n] = NAN; // length of the timeseries
  
  // association, i.e. direction without magnitude
  compare[astat_gam] = NAN; // gamma correlation
  compare[astat_ta] = NAN; //tau_a -- not correcting for ties
  compare[astat_tb] = NAN; //tau_b -- correcting for ties
  
  // standard product moment correlation
  compare[astat_xcr] = NAN;
  
  // Differences L-Norms
  compare[astat_L1] = NAN; // difference in means
  compare[astat_L2] = NAN; // difference as RMSE
  
  if (&Data1 == &Data2)
  { return compare; } //Dummy
  
  if (Data1.size() != Data2.size()) {
    sprintf(msg, "error in '%s'. ", __func__);
    error_hard(msg, "Data sizes are different", "Please contact the developer.",
               true);
    return compare;
  }
  
  if (Data1.size() == 0 ) {
    sprintf(msg, "error in '%s'. ", __func__);
    error_hard(msg, "no Data!", "Please check your code.",
               true);
    return compare;
  }
  
  const int len_data = Data1.size();
  const double rlen_data = static_cast<double>(len_data);
  
  compare[astat_n] = rlen_data; // length of the timeseries
  
  //if any vector is const, there cannot be any correlation and we take a short route.
  bool constVector = (is_const_dbl(Data1) || is_const_dbl(Data2) );
  
  double AE;
  
  if (constVector) {
    AE = Data1[0] - Data2[0];
    
    if (AE < 0)
    { AE = -AE; }
    
    compare[astat_L1] = AE;
    compare[astat_L2] = AE;
    
    return compare;
  }
  
  //else we commence.
  double MAE = 0.0, RMSE = 0.0, sx = 0.0, sy = 0.0;
  double tie_a = 0.0, tie_b = 0.0, tie_any = 0.0, tie_both = 0.0;
  double concordant = 0.0, discordant = 0.0;
  double gamma, tau_a, tau_b;
  
  for (auto it1 = Data1.begin(), it2 = Data2.begin(); it1 != Data1.end(); ++it1, ++it2) {
  
    AE = *it1 - *it2;
    sx += *it1;
    sy += *it2;
    
    if (AE < 0) {
      AE = -AE;
    }
    
    MAE += AE;
    RMSE += AE * AE;
    
    //stop one before end!
    if ( std::next(it1) != Data1.end() ) {
    
      //Check if either series one or two are tied.
      bool t_a = isWithinPrecisionInterval(*it1, *(std::next(it1)),
                                           FLOAT_PREC_INTVL);
      bool t_b = isWithinPrecisionInterval(*it2, *(std::next(it2)),
                                           FLOAT_PREC_INTVL);
                                           
      if (t_a)  {
        tie_a++;
      }
      
      
      if (t_b)  {
        tie_b++;
      }
      
      if (t_a && t_b) {
        tie_both++;
      }
      
      if (t_a || t_b) {
        tie_any++;
        continue; //neither
      }
      
      
      if (    (*it1 > *(std::next(it1)) && *it2 > *(std::next(it2) ) )
              ||  (*it1 < * (std::next(it1)) && *it2 < * (std::next(it2) ) )
         ) {
        concordant++;
      }
      else {
        discordant++;
      }
    }
  }
  
  if (concordant + discordant != 0) {
    double S = (concordant - discordant);
    gamma = S / ( concordant + discordant );
    tau_a = S / ( concordant + discordant + tie_any );
    tau_b = S / sqrt( double(concordant + discordant + tie_a - tie_both) *
                      double(concordant + discordant + tie_b - tie_both) );
    // tau_c
  }
  else {
    gamma = 0.0;
    tau_a = 0.0;
    tau_b = 0.0;
    // tau_c
  }
  
  compare[astat_gam] = gamma; // gamme correlation
  compare[astat_ta] = tau_a;
  compare[astat_tb] = tau_b;
  
  MAE /= (rlen_data);
  RMSE /= (rlen_data);
  
  if (RMSE > 0) {
    RMSE = sqrt(RMSE);
  }
  
  compare[astat_L1] = MAE;
  compare[astat_L2] = RMSE;
  
  double xbar = sx / rlen_data;
  double ybar = sy / rlen_data;
  double sxx = 0.0, syy = 0.0, sxy = 0.0;
  
  
  for (auto it1 = Data1.begin(), it2 = Data2.begin(); it1 != Data1.end();
       ++it1, ++it2) {
    sx = *it1 - xbar;
    sy = *it2 - ybar;
    sxx += sx * sx;
    syy += sy * sy;
    sxy += sx * sy;
  }
  
  double xcorr;
  
  if (sxy != 0.0) {
    double den = sxx * syy;
    
    if (den > 0.0) {    //should be unnecessary, for the series are not constant.
      xcorr = sxy / sqrt(den); //correlated (pos or neg)
    }
    else {
      xcorr = NAN; //ndef
    }
  }
  else {
    xcorr = 0.0;  //Uncorrelated perfect
  }
  
  compare[astat_xcr] = xcorr;
  
  return compare;
}

bool is_const_dbl(std::vector<double> const& Data1)
{
  for (auto it = Data1.begin(); it != Data1.end(); it++) {
    if (!isWithinPrecisionInterval(
          *it, Data1.back(),
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
  
  if (oFirst->b == NULL) {
    return true;    //no desc. objects yet.
  }
  
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

    Base Types: Micro, Macro, Factorials
    Dependent Types: Conditional (Micro + Factorial) & Comparative (Macro vs Macro)

********************************************/
//entry point functions

void abmat_add_micro(std::string varlab)
{
  abmat_add_object_intern(a_micro, varlab.c_str(), "", std::set<int>());
}

void abmat_add_macro(std::string varlab)
{
  abmat_add_object_intern(a_macro, varlab.c_str(), "", std::set<int>());
}

void abmat_add_fact(std::string varlab, std::string factList)
{
  abmat_add_object_intern(a_fact, varlab.c_str(), "", stringOfIntsToSet(factList));
}

void abmat_add_comp(std::string varlab, std::string var2lab)
{
  abmat_add_object_intern(a_comp, varlab.c_str(), var2lab.c_str(), std::set<int>());
}

void abmat_add_cond(std::string varlab, std::string var2lab)
{
  abmat_add_object_intern(a_cond, varlab.c_str(), var2lab.c_str(), std::set<int>());
}

void abmat_add_par_static(std::string varlab)
{
  abmat_add_object_intern(a_pstat, varlab.c_str(), "", std::set<int>());
}

void abmat_add_par_micro(std::string varlab)
{
  abmat_add_object_intern(a_pmic, varlab.c_str(), "", std::set<int>());
}

void abmat_add_par_macro(std::string varlab)
{
  abmat_add_object_intern(a_pmac, varlab.c_str(), "", std::set<int>());
}

void abmat_add_final_micro(std::string varlab)
{
  abmat_add_object_intern(a_fmic, varlab.c_str(), "", std::set<int>());
}

void abmat_add_final_macro(std::string varlab)
{
  abmat_add_object_intern(a_fmac, varlab.c_str(), "", std::set<int>());
}


/****************************************
    ABMAT_CREATE_TREE
    Create the basic ABMAT structure with variables and parameters
****************************************/
void abmat_create_tree()
{
  const char* typeLab = NULL;
  
  for (auto item : AllAbmatTypes) {
    switch (item) {
      case a_micro:
        typeLab = lmicro;
        break;
        
      case a_macro:
        typeLab = lmacro;
        break;
        
      case a_cond:
        typeLab = lcond;
        break;
        
      case a_comp:
        typeLab = lmacro; //treat as two linked macro variables
        break;
        
      case a_fact:
        typeLab = lfact;
        break;
        
      case a_pstat:
        typeLab = lpstat;
        break;
        
      case a_pmic:
        typeLab = lpmic;
        break;
        
      case a_pmac:
        typeLab = lpmac;
        break;

      case a_fmic:
        typeLab = lfmic;
        break;
        
      case a_fmac:
        typeLab = lfmac;
        break;
        
      case a_pLSD:
        typeLab = lpLSD;
        break;
        
      default: //checked already
        error_hard( __DEV_ERR_INFO__, "defaulting should not happen",
                    "contact the developer.",
                    true );
        return;
    }
    
    abmat->add_obj(typeLab, 1, false );
  }
}


/********************************************
    ABMAT_UPDATE_SIM_PARS
    Add information of the simulation runs in LSD that are important.
    Currently this is ONLY the seed
********************************************/
void abmat_update_sim_pars()
{
  if (t != 1) {
    return;
  }
  
  plog("\nCALLING ------ ");
  //Add simulation parameters.
  abmat_add_sim_par("seed", seed - 1); //seed is already incemented when called.
  abmat_add_sim_par("cur_sim", cur_sim); //run number in curent batch
}

void abmat_add_sim_par(std::string slab, double value)
{
  object* parent = abmat->search_local(lpLSD);
  plog("\nAdding ");
  plog(slab.c_str());
  //Add seed
  slab = get_abmat_varname(a_pLSD, slab.c_str());
  abmat_add_var( parent, slab.c_str() );
  abmat_write(parent, slab.c_str(), value);
  
}

void abmat_add_object_intern(Tabmat type, char const* varlab, char const* var2lab, const std::set<int> factors)
{
  //check if the abmat object associated to the variable
  //exists. Note: It may be included in multiple categories
  //so we check on category level.
  
  //check that the object added is not a function.
  variable* target1 = root->search_var_global(root, varlab);
  
  if (target1 == NULL) {
    //first check if the variable varLab exists in the model.
    sprintf( msg, "error in '%s'. Variable %s is not in the model.", __func__, varlab );
    error_hard( msg, "Wrong variable name",
                "Check your code to prevent this error.",
                true );
    return;
  }
  
  if ( target1->param != 1 && target1->param != 0 ) { //1: parameter, 0: variable
    sprintf( msg, "error in '%s'. Variable %s is a function", __func__, varlab );
    error_hard( msg, "You may not select functions as ABMAT variables",
                "Check your code to prevent this error.",
                true );
    return;
  }
  
  
  if (!abmat_dynamic_factors_allowed && type == a_cond ) {
    //Check that factorial is defined!
    object* oFact = NULL;
    oFact = abmat->search_local(lfact);
    
    if (oFact != NULL) {
      oFact = oFact->search_local(var2lab);
    }
    
    if (oFact == NULL) {
      sprintf(msg, "You need to define the variable %s as factortial variable first", var2lab);
      error_hard( __DEV_ERR_INFO__, msg, "Check your code.");
      return;
    }
  }
  
  if ( !abmat_dynamic_factors_allowed && type == a_fact) {
    if ( factors.size() == 0 ) {
      error_hard( __DEV_ERR_INFO__, "A factorial variable needs factors!", "Check your code.", true);
      return;
    }
  }
  
  object* parent = NULL;
  object* fparent = NULL;
  const char* typeLab = NULL;
  
  switch (type) {
    case a_micro:
      typeLab = lmicro;
      break;
      
    case a_macro:
      typeLab = lmacro;
      break;
      
    case a_cond:
      typeLab = lcond;
      break;
      
    case a_comp:
      typeLab = lmacro; //treat as two linked macro variables
      break;
      
    case a_fact:
      typeLab = lfact;
      break;
      
    case a_pstat:
      typeLab = lpstat;
      break;
      
    case a_pmic:
      typeLab = lpmic;
      break;
      
    case a_pmac:
      typeLab = lpmac;
      break;
    
    case a_fmic:
        typeLab = lfmic;
        break;
        
    case a_fmac:
      typeLab = lfmac;
      break;
      
    default: //checked already
      error_hard( __DEV_ERR_INFO__, "defaulting should not happen",
                  "contact the developer.",
                  true );
      return;
  }
  
  
  
  //get the parent for the type, create if it exists not yet
  parent = abmat->search_local(typeLab);
  object* oVar = NULL;
  object* oVar2 = NULL;
  bool var1_added = false;
  bool var2_added = false;
  
  //Add the variable as an object to the category
  oVar = parent->search_local(varlab);
  
  if (oVar == NULL) {
    //a factor needs factors.
    if (!abmat_dynamic_factors_allowed && type == a_fact && factors.size() == 0 ) {
      error_hard( __DEV_ERR_INFO__, "A factor needs factors.",
                  "Check your code to prevent this error.",
                  true );
    }
    
    parent->add_obj(varlab, 1, false);
    oVar = parent->search_local(varlab);
    plog("\n(ABMAT) : Added object of type ");
    plog(typeLab);
    plog(" with name ");
    plog(oVar->label);
    var1_added = true;
  }
  
  if (type == a_fact) {
    m_abmat_conditions[oVar->label]; //create placeholder.
  }
  
  //Comparative vars as pair of macro vars, linked via hooks (single direction)
  if (type == a_comp || type == a_cond) {
    var2_added = true;
    
    //add 2nd as macro via recursive call.
    if (type == a_comp) {
      oVar2 = parent->up->search_local(lmacro)->search_local(var2lab);
      
      if (oVar2 == NULL) {
        abmat_add_object_intern(a_macro, var2lab);
        oVar2 = parent->up->search_local(lmacro)->search_local(var2lab);
      }
    }
    else if (type == a_cond) {
    
      oVar2 = parent->up->search_local(lfact)->search_local(var2lab);
      
      if ( oVar2 == NULL) {
        if ( !abmat_dynamic_factors_allowed && factors.size() == 0 ) {
          error_hard( __DEV_ERR_INFO__, "A factorial variable needs factors!", "Check your code.", true);
          return;
        }
        else {
          abmat_add_object_intern(a_fact, var2lab);
          oVar2 = parent->up->search_local(lfact)->search_local(var2lab);
        }
      }
      
    }
    
    
    if (oVar2 == NULL) {
      error_hard(__DEV_ERR_INFO__, "The variable should exist.", "Contact the developer", true);
      return;
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
        plog("\n(ABMAT) : Added link for comparison/factorial of variable ");
        plog(oVar->label);
        plog(" with variable ");
        plog(oVar2->label);
      }
      
      CatchAll("Check if the type o_vecT changed!");
    }
  }
  
  // Add the single variables
  switch (type) {
  
    case a_fact:
      if (var1_added) {
      
        //each factorial is treated as a macro variable, holding the share of items as information, and the total number.
        std::string var1lab = get_abmat_varname_fact_total( oVar->label );
        variable* fracVar = oVar->search_var_local(var1lab.c_str());
        
        if (fracVar != NULL) {
          error_hard(__DEV_ERR_INFO__, "The factorial exists already.", "Check your code...", true);
          return;
        }
        
        abmat_add_var(oVar, var1lab.c_str()); //total
        
        for (auto factor : factors) {
          var1lab = get_abmat_varname_fact( oVar->label, factor );
          abmat_add_var(oVar, var1lab.c_str()); //share of each factor
          m_abmat_conditions[oVar->label].insert(factor); //add to map
        }
        
      }
      
      break;
      
    case a_cond:
      if (var2_added) {
        //the top objects for the unique pair variable and conditioning
        //variable exist. The rest is dynamically checked/produced
        //in the update procedure
        try {
        
          for (auto factor : m_abmat_conditions.at(oVar2->label) ) {
          
            auto stats_template = abmat_stats( ); //retrieve map of stats
            
            for (auto& elem : stats_template) {   //create a variable for each statistic
              std::string nvarLab = get_abmat_varname ( type, oVar->label, elem.first.c_str(), oVar2->label, factor );
              abmat_add_var(oVar, nvarLab.c_str());
            }
          }
        }
        
        CatchAll("Check the creation of factorials");
      }
      
      type = a_micro;
      
    //each cond is also treated as non-cond micro. pmic is considered the same except for saving data.
    case a_pmic:
    case a_fmic:
    case a_micro:
      if (var1_added) {
        auto stats_template = abmat_stats( ); //retrieve map of stats
        
        //create a variable for each statistic
        for (auto& elem : stats_template) {
          // plog("\n(ABMAT) : ");
          // plog(oVar->label);
          std::string nvarLab = get_abmat_varname ( type, oVar->label, elem.first.c_str());
          // plog(" : ");
          // plog(nvarLab.c_str());
          abmat_add_var(oVar, nvarLab.c_str());
        }
      }
      
      break;
      
    case a_comp:
      type = a_macro;//brother done already in the additional recursive call.
      
    case a_pstat:
    case a_pmac:
    case a_fmac:
    case a_macro:
      if (var1_added) {
        //a macro object holds a variable with the same name, maybe shortened.
        std::string nvarLab = get_abmat_varname(type, oVar->label); //name is same as original, shortened
        abmat_add_var(oVar, nvarLab.c_str());
      }
      
      break;
      
    default:
      error_hard( __DEV_ERR_INFO__, "defaulting should not happen",
                  "contact the developer.",
                  true );
      return;
      
  }
  
  // //visualise the added variables.
  // plog("\n(ABMAT) : ---- Added new variables to abmat ---");
  // if (var1_added)
  // plog_object_tree_up(oVar);
  // if (var2_added)
  // plog_object_tree_up(oVar2);
  // plog("\n(ABMAT) : --------------------------------------\n");
  // //Add ,,,
}

/********************************************************
    ABMAT_ADD_VAR
    Add an abmat variable to an abmat object
********************************************************/
variable* abmat_add_var(object* parent, char const* lab)
{
  plog("\nAdding variable ");
  plog(parent->label);
  plog("->");
  plog(lab);
  variable* var = parent->search_var_local(lab);
  
  if (var != NULL) {
    plog("\n(ABMAT) : Variable ");
    plog(lab);
    plog(" already added. ");
    return NULL;
  }
  
  var = parent->add_var_basic(lab, 0, NULL, true, true); //no lags, no values, save
  var->param = 1; //1 is parameter. Other fields are not used.
  var->abmat = true;
  var->start = 1; //start at time 1, do not consider initial values.
  var->end = 1; //temporay.
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
  
  if ( cv->num_lag > 0  || cv->param == 1 ) {
    cv->data[ 0 ] = cv->val[ 0 ];
  }
  
}

/*******************************************************
    PLOG_STATS
    Helper to plog ms_statsT object content.
******************************************************/
void plog_stats(ms_statsT const& stats, const char* title)
{
  plog("\n(ABMAT) : ");
  
  if (strlen(title) > 0) {
    plog("ABMAT Stats for ");
    plog(title);
    plog("\n(ABMAT) : -------------------------------\n");
  }
  
  int i = 0;
  
  for (auto const& item : stats) {
    plog(item.first.c_str());
    plog("\t");
    plog(std::to_string(item.second).c_str());
    plog("\n(ABMAT) : ");
    
    if (i++ > 100) {
      plog("\n(ABMAT) :  ... more than 100.");
      break;
    }
  }
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
  for (object* parent = startO->up; parent != NULL; parent = parent->up) {
    tree.insert(0, "'\n|");
    tree.insert(0, parent->label);
    tree.insert(0, "\n'");
  }
  
  //add self and variables
  tree += "\n";
  tree += startO->label;
  tree += " :\t";
  int count = 0;
  
  for (variable* cv = startO->v; cv != NULL; cv = cv->next) {
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
    
    if (++count % 4 == 0) {
      tree += "\n\t";
    }
  }
  
  plog(tree.c_str());
}

/****************************************************
    ABMAT_WRITE (*)
    Simplified, faster version of the object::write command.
    No NAN checking. Only for ABMAT.
 ***************************************************/

void abmat_write(object* oVar, char const* lab, double value)
{
  variable* cv = oVar->search_var_local( lab);
  
  if ( cv == NULL ) {
    sprintf(msg, "the ABMAT variable %s cannot be found in object %s->%s->%s", lab, oVar->up->up->label, oVar->up->label, oVar->label);
    error_hard( __DEV_ERR_INFO__, msg,
                "contact the developers", true );
    return;
  }
  
  abmat_write( cv, lab, value );
}

void abmat_write(variable* cv, char const* lab, double value)
{
  if (cv->last_update >= t) {
    error_hard( __DEV_ERR_INFO__, "Called at least twice at the same time!", "Contact the developer", true);
    return;
  }
  
  if (!(cv->abmat)) {
    error_hard( __DEV_ERR_INFO__, "Using special abmat procedure on non-abmat variable.", "Contact the developer", true);
    return;
  }
  
  cv->val[ 0 ] = value; //current value
  cv->last_update = t;
  cv->data[ t ] = value; //Basically manipulate the track record.
}

/**************************************************
    ABMAT_UPDATE
    Update the content of all abmat (time series) variables.
    Cycles through the abmat tree and gathers all the data, writing it to
    the parameters.
**************************************************/

void abmat_update()
{
  for_each_abmat_base_variable( abmat_update_variable );
}

/*******************************************************************
    FOR_EACH_ABMAT_BASE_VARIABLE
    Cycles through the abmat objects and applies the function or functor f
    on all the abmat variables.
*******************************************************************/
template <typename FuncType>
void for_each_abmat_base_variable(FuncType& f )
{
  //cycle through ABMAT objects and update the information.
  for (bridge* pb = abmat->b; pb != NULL; pb = pb->next) {
    object* parent = pb->head;
    
    Tabmat type = abmat_toVar_type(parent);
    
    if ( type == a_pLSD ) {
      f(parent, type);
      continue;
    }
    
    
    // if (type == a_fact)
    // continue; //is treated with conditionals
    
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
        f(oVar, type);
      }
    }
  }
}

/*****************************************************************
    ABMAT_TOTAL_STATS
    a functor to collect all the data from the abmat variables and
    calculate the total stats scalars for the (new) totals file.
*****************************************************************/
ms_statsT const& abmat_total_stats::operator()() const
{
  return total_stats;
}

void abmat_total_stats::operator()(object* oVar, Tabmat type)
{

  for (variable* cv = oVar->v ; cv != NULL; cv = cv->next) {
    // plog("\n(ABMAT) : Adding ");plog(cv->label);
    // plog("\n(ABMAT) : size before :");plog(std::to_string(total_stats.size()).c_str());
    abmat_scalars(cv, type, total_stats);
    // plog("\n(ABMAT) : size after :");plog(std::to_string(total_stats.size()).c_str());
  }
}

void abmat_total_stats::emplace(std::string const& text, double const& value)
{
  total_stats.emplace(text, value);
}

/**************************************************
    abmat_varname_tot
    Add the Interval info for the totals file
**************************************************/


std::string abmat_varname_tot(std::string base, int interval, std::string stat)
{
  base = abmat_varname_tot(base, interval);
  base += "_";
  base += stat;
  return base;
}

std::string abmat_varname_tot(std::string base, int interval)
{
  base += "_I";
  base += std::to_string(interval);
  return base;
}

/**************************************************
    ABMAT_SCALARS
    Produce the final stats for the total file for the given variable.
**************************************************/

ms_statsT abmat_scalars(variable* vVar)
{
  plog("\n(ABMAT) : Copy Version 1");
  Tabmat type = abmat_vVar_type(vVar);
  return abmat_scalars(vVar, type);
}

ms_statsT abmat_scalars(variable* vVar, Tabmat type)
{
  plog("\n(ABMAT) : Copy Version 2");
  ms_statsT scalars;
  abmat_scalars(vVar, type, scalars);
  return scalars;
}


void abmat_scalars(variable* vVar, Tabmat type, ms_statsT& scalars)
{
  // plog("\n(ABMAT) : Reference Version");
  
  //imutable parameters, only first value relevant
  if (type == a_pLSD || type == a_pstat) {
    scalars.emplace(vVar->label, vVar->data[1]);
    plog("\nAdded variable ");
    plog(vVar->label);
    plog(" with value ");
    plog(std::to_string(vVar->data[1]).c_str());
    return;
  }
  
  bool defInterval = false;
  
  if (s_abmat_intervals.size() == 0) {
    s_abmat_intervals.emplace(1, t); //add interval
    defInterval = true;
  }
  
  // plog("\n(ABMAT) : Called abmat_scalars for variable ");plog(vVar->label);
  // plog("with ");plog(std::to_string(s_abmat_intervals.size()).c_str());plog(" intervals");
  
  
  
  
  int i = -1;
  
  for (auto interval : s_abmat_intervals) {
    ++i; //increase interval counter
    
    //for each abmat time-series variable, create this information
    auto data = vVar->copy_data( interval.first, interval.second );
    
    
    //additional type specific stats that do not change the data
    switch (type) {
    
      case a_macro:
      
        //comparative stuff
        for (auto oVar : vVar->up->hooks) {
          plog("\n(ABMAT) : checking comparative variables");
          
          if (oVar->v == NULL) {
            error_hard(__DEV_ERR_INFO__, "There is no variable in the abmat comparative object.", "Contact the developers");
            return;
          }
          
          variable* cVar = oVar->v;//careful! As long as we keep that there is but a single macro (copy) object this is save.
          
          if (cVar->next != NULL) {
            error_hard(__DEV_ERR_INFO__, "There are multiple abmat comparative objects?!", "Contact the developers");
            return;
          }
          
          auto vname = get_abmat_varname_comp(vVar->label, cVar->label);
          auto data2 = cVar->copy_data( interval.first, interval.second);
          auto cstats = abmat_compare(data, data2);
          
          for (auto& stat : cstats) {
            auto vname2 = abmat_varname_tot(vname, i, stat.first);
            scalars[vname2] = stat.second;
          }
        }
        
        // plog_stats(scalars, "Comp var 2");
        break;
        
      case a_micro:
      case a_cond:
      case a_fact:
        break;
        
      case a_pmac:
      case a_pmic: 
      {
        auto vname2 = abmat_varname_tot(vVar->label, i);          
        scalars.emplace(vname2, data.front());
        plog("\nAdded variable ");
        plog(vname2.c_str());
        plog(" with value ");
        plog(std::to_string(data.front()).c_str());
        //auto vname = abmat_varname_tot(vVar->label, i, stat.first);
        
      }
      break;

      case a_fmac:
      case a_fmic: 
      {
        auto vname2 = abmat_varname_tot(vVar->label, i);          
        scalars.emplace(vname2, data.back());
        plog("\nAdded variable ");
        plog(vname2.c_str());
        plog(" with value ");
        plog(std::to_string(data.back()).c_str());
        //auto vname = abmat_varname_tot(vVar->label, i, stat.first);
        
      }
        
        return;
        
        
      case a_pLSD:
      default: //checked already
        error_hard( __DEV_ERR_INFO__, "defaulting should not happen",
                    "contact the developer.",
                    true );
        return;
        
        
    }
    
    //standard stats that change the data
    auto stats = abmat_stats( data );
    
    // plog_stats(stats, "Comp var 1");
    for (auto& stat : stats) {
      if (stat.second == NAN) {
        plog("\n(ABMAT) : debug");
      }
      
      auto vname = abmat_varname_tot(vVar->label, i, stat.first);
      scalars.emplace(vname, stat.second); // [vname] = stat.second;
    }
  }
  
  // plog_stats(scalars, "Joined ");// vVar->up->label
  
  if (defInterval) {
    s_abmat_intervals.clear();    //empty again.
  }
}


/**************************************************
    ABMAT_VVAR_TYPE
    ABMAT_OVAR_TYPE
    ABMAT_TOVAR_TYPE
    Return the type of the abmat category of the lsd variable,
        abmat variable (lsd object) or abmat category object (lsd object)
**************************************************/

Tabmat abmat_vVar_type(variable* vVar)
{
  if (vVar == NULL) {
    error_hard( __DEV_ERR_INFO__, "Null Variable passed.", "Contact the developer", true );
  }
  else if (vVar->up == NULL) {
    error_hard( __DEV_ERR_INFO__, "Variable parent is Null.", "Contact the developer", true );
  }
  
  return abmat_oVar_type(vVar->up);
}

Tabmat abmat_oVar_type(object* oVar)
{
  if (oVar == NULL) {
    error_hard( __DEV_ERR_INFO__, "Null Object passed.", "Contact the developer", true );
  }
  else if (oVar->up == NULL) {
    error_hard( __DEV_ERR_INFO__, "Object parent is Null.", "Contact the developer", true );
  }
  
  return abmat_toVar_type(oVar->up);
}

Tabmat abmat_toVar_type(object* toVar)
{
  if (toVar->up != abmat) {
    error_hard( __DEV_ERR_INFO__, "object is not an abmat-type object", "Contact the developer", true );
  }
  
  Tabmat type; //only micro, macro or conditional here
  
  if ( strcmp(toVar->label, lmicro) == 0 ) {
    type = a_micro;
  }
  else if ( strcmp(toVar->label, lmacro) == 0 ) {
    type = a_macro;
  }
  else if ( strcmp(toVar->label, lcond) == 0 ) {
    type = a_cond;
  }
  else if ( strcmp(toVar->label, lfact) == 0 ) {
    type = a_fact;
  }
  else if ( strcmp(toVar->label, lpstat) == 0 ) {
    type = a_pstat;
  }
  else if ( strcmp(toVar->label, lpmic) == 0 ) {
    type = a_pmic;
  }
  else if ( strcmp(toVar->label, lpmac) == 0 ) {
    type = a_pmac;
  }
  else if ( strcmp(toVar->label, lfmic) == 0 ) {
    type = a_fmic;
  }
  else if ( strcmp(toVar->label, lfmac) == 0 ) {
    type = a_fmac;
  }
  else if ( strcmp(toVar->label, lpLSD) == 0 ) {
    type = a_pLSD;
  }
  else {
    sprintf(msg, "wrong abmat type-object of type %s", toVar->label);
    error_hard( __DEV_ERR_INFO__, msg, "Contact the developer.", true );
  }
  
  return type;
}

/**************************************************
    ABMAT_UPDATE_VARIABLE
    Update a given abmat variable (storred as lsd object with child variables)
**************************************************/
void abmat_update_variable(object* oVar, Tabmat type)
{
  if (oVar == NULL) {
    error_hard( __DEV_ERR_INFO__, "Null Object passed.", "Contact the developer");
  }
  
  //write data
  switch (type) {
  
    case a_pstat: {
        if (t != a_time_init) {
          return;    //imutable, only track first time
        }
      }
      
    case a_pLSD: {
        return;    //imutable, considered directly elsewhere
      }
      
    case a_pmic:  //track, although only 1. item at each interval is relevant
    case a_fmic:
    case a_micro: {
        std::vector<double> data = root->gatherData_all(oVar->label); //calls also with NULL
        auto stats_template = abmat_stats( data ); //retrieve map of stats
        
        //save data
        
        //visualise the added variables.
        // plog("\n(ABMAT) : ---- Prep to write data ---");
        // plog_object_tree_up(oVar);
        // plog("\n(ABMAT) : ---");
        
        for (auto& elem : stats_template) {
          std::string nvarLab = get_abmat_varname ( type, oVar->label, elem.first.c_str());
          abmat_write( oVar, nvarLab.c_str(), elem.second );
        }
      }
      break;
      
      
    case a_pmac:    //track, although only 1. item at each interval is relevant
    case a_fmac:
    case a_macro: {
        //simply copy the current data.
        double val = root->cal(NULL, oVar->label, 0);
        std::string varlab = get_abmat_varname(type, oVar->label);
        abmat_write(oVar, varlab.c_str(), val);
      }
      break;
      
    case a_fact: {
    
        double total = 0.0; //elements
        std::map < int, std::vector<double> > fact_vData_map;
        auto cycle_var = next_var(root, oVar->label, true);
        
        for (variable* curv = cycle_var(); curv != NULL; curv = cycle_var()) {
          variable* curv2 = curv->up->search_var_local(oVar->label);
          
          if (curv2 == NULL) {
            sprintf( msg, "variable '%s' is missing in object '%s' in function '%s'", oVar->label, curv->up->label, __func__ );
            error_hard( msg, "variable or parameter not found",
                        "check model structure for conditional abmat" );
            return;
          }
          
          total++;
          int factor = static_cast<int> ( curv2->cal( NULL, 0) ); //same as in object::update()
          double val = curv->cal( NULL, 0);
          fact_vData_map[factor].push_back(val);
        }
        
        // save info on number of elements.
        std::string varlab = get_abmat_varname_fact_total(oVar->label);
        abmat_write(oVar, varlab.c_str(), total);
        
        
        //write the share info for each factor
        for (auto& subset : fact_vData_map) {
          //save information of relative size to conditional variable
          double fraction = static_cast<double>(subset.second.size() ) / total;
          
          
          //create the variable name
          std::string varlab = get_abmat_varname_fact( oVar->label, subset.first );
          variable* fracVar = oVar->search_var_local(varlab.c_str());
          
          //create variables the first time a factor appears
          if (fracVar == NULL) {
            bool dynamic_mode = sim_num == 1 ? abmat_dynamic_factors_allowed : false;
            
            if (dynamic_mode) {
              plog("\n(ABMAT) :  Encountered new factor for factorial. Creating new variable: ");
              plog(varlab.c_str());
              //conditioning variable for fraction tracking
              fracVar = abmat_add_var(oVar, varlab.c_str());
              
              try {
                m_abmat_conditions.at(oVar->label).insert(subset.first);
              }
              
              CatchAll("Uuups22");
            }
            else {
              sprintf(msg, "The number of factors for factorial '%s' changed. This is not allowed.", oVar->label);
              error_hard(__DEV_ERR_INFO__, msg, "Make sure to define the factors ex-ante", true);
              return;
            }
          }
          
          //save info
          abmat_write(oVar, varlab.c_str(), fraction);
        }
        
      }
      break;
      
    //Cond is different,because here is a first and second variable, too.
    
    case a_cond:
    
      //Each conditional variable may be linked to several conditions (i.e. factors).
      for (object* oVar2 : oVar->hooks) {
      
        if (oVar2->v != NULL && oVar2->v->last_update < t) {
          abmat_update_variable(oVar2, a_fact);    //update, if not yet done.
        }
        
        //create set and copy information of previous sets.
        //it is a map of factors and data.
        std::map < int, std::vector<double> > cond_vData_map;
        
        for (auto& element : m_abmat_conditions.at(oVar2->label) ) {
          cond_vData_map[element]; //create empty set for integer factors
        }
        
        
        
        //gather all data and save in sets per condition/factor
        double total = 0.0;
        auto cycle_var = next_var(root, oVar->label, true);
        
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
          
          try {
            cond_vData_map.at(condVal).push_back(val);
          }
          
          CatchAll("Uups");
        }
        
        //save data
        for (auto& subset : cond_vData_map) {
          auto stats = abmat_stats(subset.second);
          
          for (auto& stat : stats) {
            std::string nvarLab = get_abmat_varname ( type, oVar->label, stat.first.c_str(), oVar2->label, subset.first );
            //create variables the first time a factor appears
            //dynamic mode is checked in a_frac
            variable* cv = oVar->search_var_local( nvarLab.c_str() );
            
            if (NULL == cv) {
              cv = abmat_add_var(oVar, nvarLab.c_str());
            }
            
            abmat_write(oVar, nvarLab.c_str(), stat.second);
          }
        }
      }
      
      break;
      
      
    case a_comp:
      error_hard(__DEV_ERR_INFO__, "a_comp / a_fact case should not exist.",  "Contact the developer");
      break;
  }
  
  //update time
  for (variable* cv = oVar->v; cv != NULL; cv = cv->next) {
    cv->end = t;
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
    error_hard( __DEV_ERR_INFO__, "ABMAT is already connected to something",
                "please contact developers",
                true );
    return;
  }
  
  abmat->up = root;
  bridge* cb;
  
  // create bridge
  if ( root->b == NULL ) {
    cb = root->b = new bridge(abmat->label);
  }
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

abmat_total_stats abmat_total()
{

  //if no intervals are specified, consider whole time
  bool defInterval = false;
  
  if (s_abmat_intervals.size() == 0) {
    s_abmat_intervals.emplace(1, actual_steps); //add interval
    defInterval = true;
  }
  
  abmat_total_stats total_stats;
  
  //Add interval infos
  int i = 0;
  
  for (auto const& intvl : s_abmat_intervals) {
    std::string start = "Interval_I";
    start += std::to_string(i++);
    std::string end = start;
    start += "_start";
    end +=   "_end";
    total_stats.emplace(start, intvl.first);
    total_stats.emplace(end, intvl.second);
  }
  
  //Cycle through the variables
  for_each_abmat_base_variable( total_stats ); //create and add all the infos
  
  if ( defInterval ) {
    s_abmat_intervals.clear();    //reset
  }
  
  return total_stats;
}

#endif
