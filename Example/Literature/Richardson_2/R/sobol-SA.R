#*******************************************************************************
#
# ----------------------------- Sobol DoE analysis ---------------------------
#
#	Several files are required to run:
#		folder/baseName_XX_YY.csv		    : DoE specification from LSD
#		folder/baseName_XX_YY_WWW.csv	  : DoE response from R
#		folder/baseName.lsd				      : LSD configuration with default values
#		folder/baseName.sa		          : factor sensitivity test ranges from LSD
#
#*******************************************************************************

library( LSDsensitivity )


# ==== LSD variables to use (all, the ones to be used in log and new) ====

lsdVars <- c( "HHI", "HPI" )


# ==== Process LSD set of result files (an experimental data set) ====

dataSet <- read.doe.lsd( "sa-sobol",             # data files relative folder
                         "sobol",                # data files base name (same as .lsd file)
                         "HHI",				           # variable name to perform the sensitivity analysis
                         does = 2,               # number of experiments (data + external validation)
                         iniDrop = 0,            # initial time steps to drop from analysis (0=none)
                         nKeep = -1,             # number of time steps to keep (-1=all)
                         saveVars = lsdVars,     # LSD variables to keep in dataset
                         rm.temp = FALSE,        # remove temporary speedup files?
                         rm.outl = FALSE,        # remove outliers from dataset
                         lim.outl = 10 )         # limit non-outlier deviation (number of std. devs.)


# ====== Sobol sensitivity analysis on experimental data ======

sSA <- sobol.decomposition.lsd( dataSet )        # LSD experimental data set



#*******************************************************************************
#
# ----------------------- Analysis results presentation -----------------------
#
#*******************************************************************************

library( gplots )           # package for plotting tables


# ====== Sobol sensitivity analysis results ======

# ------ Sobol sensitivity analysis table ------

textplot( signif( sSA$sa, 4 ) )
title( main = "Sobol decomposition sensitivity analysis" )

# ------ Sobol sensitivity analysis chart ------

barplot( t( sSA$sa ), col = c( "white", "gray" ), las = 2, ylim = c( 0, 1 ),
         legend.text = c( "main effects", "interactions" ) )
title( main = "Sobol decomposition sensitivity analysis", ylab = "Sobol Index" )
