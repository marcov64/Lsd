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

if( exists( "folder" ) )				        # check for previous run (interactive mode)
  rm( list = ls( all.names = TRUE ) )   # clear the R environment


# ====== User parameters ======

# Database files / time steps to use
folder   <- "sensitivity"                      # data files folder
baseName <- "sensitivity"                    # data files base name (same as .lsd file)
varName <- "sdP"					          # analysis variable name
iniDrop <- 200                        # initial time steps to drop from analysis (0=none)
nKeep <- -1                           # number of time steps to keep (-1=all)
cores <- 0                            # maximum number of cores to allocate (0=all)
savDat <- TRUE                        # save processed data files and re-use if available?

# Flags for removing outliers from DoE & validation samples (use with care!)
remOutl <- FALSE                      # remove outliers from DoE & validation samples?
limOutl <- 10                         # limit deviation (number of standard deviations)

# General flags
conf <- 0.95                          # confidence level to use
p <- 4                                # number of levels of the design
jump <- 2                             # number of levels jumps when calculating EE

# Parameters for variables creation and results evaluation
smoothing <- 1e5                      # HP filter smoothing factor (lambda)
crisisTh  <- -0.03                    # crisis growth threshold
crisisLen <- 3                        # crisis minimum duration (periods)
crisisPre <- 4                        # pre-crisis period to base trend start (>=1)

# Report output configuration
raster <- FALSE  					            # raster or vector plots
res <- 600       					            # resolution of raster mode (in dpi)
plotRows <- 1   					            # number of plots per row in a page
plotCols <- 1  	 					            # number of plots per column in a page
plotW <- 10      					            # plot window width
plotH <- 7       					            # plot window height

# 3D surfaces visualization control
surf3d <- FALSE                       # ask the user to adjust a 3D perspective of response surface
theta3d <- 130                        # horizontal view angle
phi3d <- 30                           # vertical view angle


# ==== Random number generator

set.seed( 1, kind = "default" )       # set the random number generator
options( warn = -1 )


# ==== Log start mark ====

cat( "\nEE DoE analysis\n===============\n" )
cat( "\n", as.character( Sys.time( ) ), "-> Start processing...\n\n" )
startTime <- proc.time( )       # register current time


# ==== Read command line parameters (if any) ====

args <- commandArgs( trailingOnly = TRUE )
cat( "Command line arguments: ", args, "\n" )

if( length ( args ) > 0 ){  # first parameter has to be the folder
  folder <- args [1]
}
if( length ( args ) > 1 ){  # second parameter has to be the base name
  baseName <- args [2]
}
if( length ( args ) > 2 ){  # third parameter has to be the variable to analyze name
  varName <- args [3]
}
if( length ( args ) > 3 ){  # fourth parameter has to be outlier removal flag
  remOutl <- as.logical( args [4] )
}
if( length ( args ) > 4 ){  # fifth parameter has to be the number of levels of DoE
  p <- as.numeric( args [5] )
}
if( length ( args ) > 5 ){  # sixth parameter has to be the jump size in # of levels
  jump <- as.numeric( args [6] )
}
if( length ( args ) > 6 ){  # seventh parameter has to be the raster output flag (TRUE)
  raster <- as.logical( args [7] )
}
if( length ( args ) > 7 ){  # eigth parameter has to be the initial time period ( 0 is all )
  iniDrop <- as.integer( args [8] )
}
if( length ( args ) > 8 ){  # ninth parameter has to be the end periods to remove ( -1 is all )
  nKeep <- as.integer( args [9] )
}

cat( " Folder =", folder, "\n" )
cat( " Base name =", baseName, "\n" )
cat( " Variable name =", varName, "\n" )
cat( " Remove outliers =", remOutl, "\n" )
cat( " Number of levels in DoE =", p, "\n" )
cat( " Jump size in DoE =", jump, "\n" )
cat( " Raster =", raster, "\n" )
cat( " Initial time steps to drop =", iniDrop, "\n" )
cat( " Time steps to keep =", nKeep, "\n\n" )


# ====== External support functions & definitions ======

require( LSDsensitivity, warn.conflicts = FALSE, quietly = TRUE )
library( LSDsensitivity )
if( ! exists( "plot_norm", mode = "function" ) )             # already loaded?
  source( "support-functions.R" )

# remove warnings for support functions
# !diagnostics suppress = fit_subbotin, textplot, remove_outliers, log0, fmt
# !diagnostics suppress = plot3d.morris, rgl.snapshot, rgl.close, hpfilter


# ==== Aggregated variables to use ====

# Aggregated variables to use
logVars <- c( "Real_GDP", "C_r", "I_r"  )
aggrVars <- append( logVars, c( "GDP_G", "P_G", "U" ))
newVars <- c( "sdGDP", "sdP", "sdU" )

# ---- Make sure selected analysis variable is available ----

if( ! ( varName %in% aggrVars ) && ! ( varName %in% newVars ) )
  stop( "Invalid variable selected" )


# ====== Function to evaluate the selected variables to 2D dataset (pool = TRUE) ======

eval.vars <- function( data, vars ) {

  # ---- General variables ----
  tsteps <- nrow( data )
  #nvars <- ncol( data )
  temp <- vector( "numeric" )

  # ---- Reompute values for existing and create new variables ----

  for( var in vars ) {

    if( var %in% logVars ) {   # Consider the log values of fast growing variables

      data[ , var ] <- log0( data[ , var ] )

    } else if( var == "sdGDP" ) {   # add the GDP std. dev. variable to data set

      data[ , var ] <- sd( data[ , "Real_GDP" ], na.rm = TRUE )

    } else if( var == "sdP" ) {   # add the GDP std. dev. variable to data set
      
      data[ , var ] <- sd( data[ , "P_G" ], na.rm = TRUE )
      
    } else if( var == "sdU" ) {   # add the GDP std. dev. variable to data set
      
      data[ , var ] <- sd( data[ , "U" ], na.rm = TRUE )
    }
  }
  return( data )
}


# ==== Process LSD result files ====

data <- read.doe.lsd( folder, baseName, varName, iniDrop = iniDrop,
                      nKeep = nKeep, saveVars = aggrVars, addVars = newVars,
                      eval.vars = eval.vars, rm.temp = ! savDat,
                      nnodes = cores, rm.outl = remOutl, lim.outl = limOutl )


# ====== Do Elementary Effects analysis ======

cat( "Estimating Elementary Effects...\n\n")

SA <- elementary.effects.lsd( data, p = p, jump = jump )


# ====== Sobol sensitivity analysis on experimental data ======

cat( "\nCalculating Sobol indexes...\n\n")

sSA <- sobol.decomposition.lsd( data )


# ====== Report generation start ======

tryCatch({    # enter error handling mode so PDF can be closed in case of error/interruption

  # create a daily output directory
  outDir <- format( Sys.time(), "%Y-%m-%d" )
  if( ! dir.exists( paste0( folder, "/", outDir ) ) )
    dir.create( paste0( folder, "/", outDir ) )

  cat( paste( "Saving results and data to:", paste0( folder, "/", outDir ), "\n" ) )

  # Select type of output
  if( raster ) {
    # Open PNG (bitmap) files for output
    png( paste0( folder, "/", outDir, "/", baseName, "_EE_", varName, "_%d.png" ),
         width = plotW, height = plotH, units = "in", res = res )

  } else {
    # Open PDF plot file for output
    pdf( paste0( folder, "/", outDir, "/", baseName, "_EE_", varName, ".pdf" ),
         width = plotW, height = plotH )
    par( mfrow = c( plotRows, plotCols ) )             # define plots per page
  }
  options( scipen = 5 )                               # max 5 digits

  cat( "\nGenerating reports...\n")

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
    if( raster ) {
      rgl.snapshot( paste0( folder, "/", outDir, "/", baseName, "_ee_%d.png" ) )
    } else {
      rgl.snapshot( paste0( folder, "/", outDir, "/", baseName, "_ee_3d.png" ) )
    }
    rgl.close( )
  }

  # ------ Save EE SA results to disk ------

  write.csv( SA$table, paste0( folder, "/", outDir, "/", baseName, "_EE_SA_", varName, ".csv" ) )

  if( ! is.null( sSA ) ) {

  # ------ Sobol sensitivity analysis table ------

  textplot( signif( sSA$sa, 3 ) )

  title( main = paste0( "Sobol decomposition indexes ( ", varName, " )" ),
         sub = paste0( "Variables rescaled to [0,1] / H0: mu.star = 0 / ",
                       "interaction effects are linear approximations\n",
                       "Levels (p) = ", p, ", delta = ", fmt( jump / ( p - 1 ) , 3 ),
                       ", trajectories (r) = ", SA$r, ", samples = ", SA$samples,
                       ", time = [", iniDrop + 1, ",", nKeep, "]" ) )

  # ------ Sobol sensitivity analysis chart ------

  barplot( t( sSA$sa ), col = c( "white", "gray" ), las = 2, ylim = c( 0, 1 ),
           legend.text = c( "main effects", "interactions" ) )

  title( main = paste0( "Sobol decomposition indexes ( ", varName, " )" ),
         sub = paste0( "Variables rescaled to [0,1] / H0: mu.star = 0 / ",
                       "interaction effects are linear approximations\n",
                       "Levels (p) = ", p, ", delta = ", fmt( jump / ( p - 1 ) , 3 ),
                       ", trajectories (r) = ", SA$r, ", samples = ", SA$samples,
                       ", time = [", iniDrop + 1, ",", nKeep, "]" ),
         ylab = "Sobol Index")

  # ------ Save Sobol SA results to disk ------

  write.csv( sSA$sa, paste0( folder, "/", outDir, "/", baseName, "_EE_Sobol_", varName, ".csv" ) )

  } else
    textplot( "The B-spline modeling failed.\nSobol SA results are unavailable." )


  cat( "\nDone...\n" )

#******************************************************************
#
# ------------- Exception handling code (tryCatch) -------------
#
#******************************************************************

}, interrupt = function( ex ) {
  cat( "An interrupt was detected.\n" )
  print(ex)
  textplot( "Report incomplete due to interrupt.")
}, error = function( ex ) {
  cat( "An error was detected.\n" )
  print( ex )
  textplot( "Report incomplete due to processing error." )
}, finally = {
  options( warn = 0 )
  cat( "\n", as.character( Sys.time( ) ), "-> Releasing resources...\n\n" )
  totalTime <- proc.time( ) - startTime
  print( totalTime )
  # Close PDF plot file
  dev.off( )
} )
