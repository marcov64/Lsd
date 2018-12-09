#*******************************************************************************
#
# --------------- Elementary Effects DoE analysis (Morris 1991) ---------------
#
#	Several files are required to run:
#		folder/baseName_XX_YY.csv		    : DoE specification from LSD
#		folder/baseName_XX_YY_WWW.res	  : DoE response from R
#		folder/baseName.lsd				      : LSD configuration with default values
#		folder/baseName.sa		          : factor sensitivity test ranges from LSD
#
#*******************************************************************************

library( LSDsensitivity )


# ==== LSD variables to use (all, the ones to be used in log and new) ====

lsdVars <- c( "HHI", "HPI" )


# ==== Process LSD set of result files (an experimental data set) ====

dataSet <- read.doe.lsd( "sa-morris",            # data files relative folder
                         "morris",               # data files base name (same as .lsd file)
                         "HPI",				           # variable name to perform the sensitivity analysis
                         iniDrop = 0,            # initial time steps to drop from analysis (0=none)
                         nKeep = -1,             # number of time steps to keep (-1=all)
                         saveVars = lsdVars,     # LSD variables to keep in dataset
                         rm.temp = FALSE,        # remove temporary speedup files?
                         rm.outl = FALSE,        # remove outliers from dataset
                         lim.outl = 10 )         # limit non-outlier deviation (number of std. devs.)


# ====== Do Elementary Effects analysis ======

eeSA <- elementary.effects.lsd( dataSet,         # LSD experimental data set
                                p = 4,           # number of levels of the design (as set in LSD)
                                jump = 2 )       # number of jumps per level (as set in LSD)



#*******************************************************************************
#
# ----------------------- Analysis results presentation -----------------------
#
#*******************************************************************************

library( gplots )           # package for plotting tables


# ====== Present sensitivity analysis for the EE model ======

textplot( signif( eeSA$table, 3 ), cex = 1.0 )
title( main = paste0( "Elementary effects distributions statistics ( ", dataSet$saVarName, " )" ),
       sub = paste0( "All variables rescaled to [ 0, 1 ] / H0: mu.star = 0\nLevels (p) = ", 4,
                     ", delta = ", signif( 2 / ( 4 - 1 ) , 3 ),
                     ", trajectories (r) = ", eeSA$r,
                     ", samples = ", eeSA$samples ) )

plot( eeSA )
title( main = paste0( "Elementary effects composition ( ", dataSet$saVarName, " )" ),
       sub = "mu.star: overall effects / sigma: non-linear/non-additive effects" )
