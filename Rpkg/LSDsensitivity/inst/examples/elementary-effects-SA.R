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

lsdVars <- c( "var1", "var2", "var3" )
logVars <- c( "var1", "var3" )
newVars <- c( "var4" )


# ====== Optional function to evaluate the selected variables ======

eval.vars <- function( dataSet, allVars ) {
  #tsteps <- nrow( dataSet )        # number of timesteps in simulated data set
  #nvars <- ncol( dataSet )         # number of variables in data set (including new ones)

  # ---- Recompute values for existing variables ----

  for( var in allVars ) {
    if( var %in% logVars ) {    # take the log values of selected variables
      try( dataSet[ , var ] <- log( dataSet[ , var ] ), silent = TRUE )  # <= 0 as NaN
    }
  }

  # ---- Calculate values of new variables (added to LSD dataset) ----

  #dataSet[ , "var4" ] <- dataSet[ , "var1" ] + dataSet[ , "var2" ]   # example of new var

  return( dataSet )
}


# ==== Process LSD set of result files (an experimental data set) ====

dataSet <- read.doe.lsd( ".",                    # data files relative folder
                         "Sim1",                 # data files base name (same as .lsd file)
                         "var1",				         # variable name to perform the sensitivity analysis
                         iniDrop = 0,            # initial time steps to drop from analysis (0=none)
                         nKeep = -1,             # number of time steps to keep (-1=all)
                         saveVars = lsdVars,     # LSD variables to keep in dataset
                         addVars = newVars,      # new variables to add to the LSD dataset
                         eval.vars = eval.vars,  # function to evaluate/adjust/expand the dataset
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
