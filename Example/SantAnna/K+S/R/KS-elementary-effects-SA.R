#*******************************************************************************
#
# --------------- Elementary Effects DoE analysis (Morris 1991) ---------------
#
#	Several files are required to run:
#		folder/basename_XX_YY.csv		  : DoE specification from LSD
#		folder/basename_XX_YY_WWW.res : DoE response from R
#		folder/basename.lsd			  	  : LSD configuration with default values
#		folder/basename.sa		        : factor sensitivity test ranges from LSD
#
#*******************************************************************************

# Database files / time steps to use
folder   <- "ee"                      # data files folder
baseName <- "Sim1-ee"                 # data files base name (same as .lsd file)
varName <- "U"					              # analysis variable name
iniDrop <- 300                        # initial time steps to drop from analysis (0=none)
nKeep <- -1                           # number of time steps to keep (-1=all)

# Flags for removing outliers from DoE & validation samples (use with care!)
remOutl <- FALSE                      # remove outliers from DoE & validation samples?
limOutl <- 10                         # limit deviation (number of standard deviations)

# General flags
conf <- 0.95                          # confidence level to use
p <- 4                                # number of levels of the design
jump <- 2                             # number of levels jumps when calculating EE

# Report output configuration
plotRows <- 1   					            # number of plots per row in a page
plotCols <- 1  	 					            # number of plots per column in a page
plotW <- 10      					            # plot window width
plotH <- 7       					            # plot window height

# 3D surfaces visualization control
surf3d <- FALSE                       # ask the user to adjust a 3D perspective of response surface
theta3d <- 130                        # horizontal view angle
phi3d <- 30                           # vertical view angle


# ==== Aggregated variables to consider ====

# Aggregated variables to use
logVars <- c( "GDP", "EI", "C", "I", "Deb1", "Deb2", "NWb", "NW1", "NW2", "S1",
              "S2", "A", "wAvg", "G", "Gbail", "Gtrain", "Tax", "Deb", "Def",
              "Loans", "W1", "W2", "B2" )
aggrVars <- append( logVars, c( "dGDP", "dCPI", "Q2u", "F1", "F2", "entry1",
                                "entry2", "exit1", "exit2", "f2posChg", "imi",
                                "inn", "r", "U", "Ue", "TeAvg", "sTavg",
                                "sVavg", "CPI", "q2avg" ) )
newVars <- c( "freq_FE", "cris_llh", "GDP_sd" )


# ====== External support functions & definitions ======

require( LSDsensitivity, warn.conflicts = FALSE, quietly = TRUE )

if( ! exists( "plot_norm", mode = "function" ) )             # already loaded?
  source( "KS-support-functions.R" )

# remove warnings for support functions
# !diagnostics suppress = fit_subbotin, textplot, remove_outliers, log0, fmt
# !diagnostics suppress = plot3d.morris, rgl.snapshot, rgl.close, hpfilter, notIn


# ---- Make sure selected analysis variable is available ----

if( ! ( varName %in% aggrVars ) && ! ( varName %in% newVars ) )
  stop( "Invalid variable selected" )


# ====== Function to evaluate the selected variables to 2D dataset (pool = TRUE) ======

eval.vars <- function( data, vars ) {

  if( length( notIn( vars, colnames( data ) ) ) > 0 )
    cat( "Missing variables in data:", notIn( vars, colnames( data ) ), "\n" )

  # ---- General variables ----
  tsteps <- nrow( data )
  temp <- vector( "numeric" )

  # ---- Reompute values for existing and create new variables ----

  for( var in vars ) {

    if( var %in% logVars ) {   # Consider the log values of fast growing variables

      data[ , var ] <- log0( data[ , var ] )

    } else if( var == "freq_FE" ) {   # add the frequency of crises variable to data set

      # Format full employment series (1 = full employment, 0 = otherwise)
      for( i in 1 : tsteps ) {
        if( data[ i, "U" ] == 0 ) temp[ i ] <- 1 else temp[ i ] <- 0
      }
      data[ , var ] <- mean( temp )

    } else if( var == "cris_llh" ) {   # add the crises likelihood variable to data set

      # Mark crises periods (= 1) when GDP growth is less than -3%
      temp[ 1 ] <- 0;
      for( i in 2 : tsteps ) {
        if( data[ i, "dGDP" ] < -0.03 ) temp[ i ] <- 1 else temp[ i ] <- 0
      }
      data[ , var ] <- mean( temp )

    } else if( var == "GDP_sd" ) {   # add the GDP std. dev. variable to data set

      data[ , var ] <- sd( data[ , "dGDP" ], na.rm = TRUE )
    }
  }

  return( data )
}


# ==== Process LSD result files ====

data <- read.doe.lsd( folder, baseName, varName, iniDrop = iniDrop,
                      nKeep = nKeep, saveVars = aggrVars, addVars = newVars,
                      eval.vars = eval.vars, rm.outl = remOutl, lim.outl = limOutl )


# ====== Do Elementary Effects analysis ======

SA <- elementary.effects.lsd( data, p = p, jump = jump )


# ====== Report generation start ======

# Open PDF plot file for output
pdf( paste0( folder, "/", baseName, "_EE_", varName, ".pdf" ),
     width = plotW, height = plotH )
par( mfrow = c( plotRows, plotCols ) )             # define plots per page
options( scipen = 5 )                              # max 5 digits

# ------ Present sensitivity analysis for the EE model ------

textplot( fmt( SA$table, 3 ) )
title( main = paste0( "Elementary effects distributions statistics ( ", varName, " )" ),
       sub = paste0( "Variables rescaled to [0,1] / H0: mu.star = 0\nLevels (p) = ", p,
                     ", delta = ", fmt( jump / ( p - 1 ) , 3 ),
                     ", trajectories (r) = ", SA$r,
                     ", samples = ", SA$samples,
                     ", time = [", iniDrop + 1, ",", nKeep, "]" ) )

# ------ EE SA plots ------

try( plot( SA ) )
title( main = paste0( "Elementary effects composition ( ", varName, " )" ),
       sub = "mu.star: overall effects / sigma: non-linear/non-additive effects" )

if( surf3d ) {                                    # 3D GL model
  require( rgl, warn.conflicts = FALSE, quietly = TRUE )
  plot3d.morris( SA )
  readline( "Go to RGL window, resize it, adjust plot perspective and press enter when done" )
  rgl.snapshot( paste0( folder, "/", baseName, "_ee_3d.png" ) )
  rgl.close( )
}


# Close PDF plot file
dev.off( )
