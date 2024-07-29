#******************************************************************
#
# - Industry Model: Elementary Effects DoE analysis (Morris 1991) -
#
#   Written by Marcelo C. Pereira, University of Campinas
#
#   Copyright Marcelo C. Pereira
#   Distributed under the GNU General Public License
#
#	Several files are required to run:
#		folder/baseName_XX_YY.csv		    : DoE specification from LSD
#		folder/baseName_XX_YY_WWW.res	  : DoE response from R
#		folder/baseName.lsd				      : LSD configuration with default values
#		folder/baseName.sa		          : factor sensitivity test ranges from LSD
#
#   IMPORTANT: this script assumes the R working directory is set
#   to the R subfolder where this script file is stored. This can
#   be done automatically in RStudio if a project is created at
#   this subfolder, or using the command setwd(). The "folder"
#   variable below must always point to a (relative) subfolder
#   of the R working directory.
#
#******************************************************************

# ====== User parameters ======

# Database files
folder   <- "MarkII/Beta/EE"          # data files folder
baseName <- "MarkII-Beta"             # data files base name (same as .lsd file)
varName <- "b"						            # analysis variable name
iniDrop <- 0                          # initial time steps to drop from analysis (0=none)
nKeep <- -1                           # number of time steps to keep (-1=all)

lsdVars <- c( )                       # no existing variable to be saved
newVars <- c( "b" )                   # add the Subbotin b parameter

# Flags for removing outliers from DoE & validation samples (use with care!)
remOutl <- FALSE                      # remove outliers from DoE & validation samples?
limOutl <- 10                         # limit deviation (number of standard deviations)
p <- 4                                # number of levels of the design
jump <- 2                             # number of levels jumps when calculating EE
conf <- 0.95                          # confidence level to use

# Report output configuration
plotRows <- 1					                # number of plots per row in a page
plotCols <- 1					                # number of plots per column in a page
plotW <- 10                           # plot window width
plotH <- 7                            # plot window height

# 3D surfaces visualization control
surf3d <- FALSE                       # ask the user to adjust a 3D perspective of response surface
theta3d <- 130                        # horizontal view angle
phi3d <- 30                           # vertical view angle


# ====== External support functions & definitions ======

source( "StatFuncs.R" )


# ==== Process LSD the set of result files (the experimental data set) ====

dataSet <- read.doe.lsd( folder,                 # data files relative folder
                         baseName,               # data files base name (same as .lsd file)
                         varName,				         # variable name to perform the sensitivity analysis
                         iniDrop = iniDrop,      # initial time steps to drop from analysis (0=none)
                         nKeep = nKeep,          # number of time steps to keep (-1=all)
                         saveVars = lsdVars,     # LSD variables to keep in dataset
                         addVars = newVars,      # new variables to add to the LSD dataset
                         eval.vars = eval.b,     # function to evaluate/adjust/expand the dataset
                         rm.temp = TRUE,         # remove temporary speedup files?
                         rm.outl = remOutl,      # remove outliers from dataset
                         lim.outl = limOutl )    # limit non-outlier deviation (number of std. devs.)


# ====== Do Elementary Effects analysis ======

eeSA <- elementary.effects.lsd( dataSet,         # LSD experimental data set
                                p = p,           # number of levels of the design (as set in LSD)
                                jump = jump )    # number of jumps per level (as set in LSD)


# ====== Present sensitivity analysis for the EE model ======

# ---- Enter error handling mode so PDF can be closed in case of error/interruption ----

tryCatch({

  # ---- Open PDF plot file for output ----

  pdf( paste0( folder, "/", baseName, "_EE.pdf" ),
       width = plotW, height = plotH )
  options( scipen = 3 )                 # max 3 digits
  par( mfrow = c ( plotRows, plotCols ) )             # define plots per page

  # ---- Plot results ----

  textplot( signif( eeSA$table, 3 ), cex = 1.0 )
  title( main = paste0( "Elementary effects distributions statistics ( ", dataSet$saVarName, " )" ),
         sub = paste0( "All variables rescaled to [ 0, 1 ] / H0: mu.star = 0\nLevels (p) = ", 4,
                       ", delta = ", signif( jump / ( p - 1 ) , 3 ),
                       ", trajectories (r) = ", eeSA$r,
                       ", samples = ", eeSA$samples ) )

  plot( eeSA )
  title( main = paste0( "Elementary effects composition ( ", dataSet$saVarName, " )" ),
         sub = "mu.star: overall effects / sigma: non-linear/non-additive effects" )

  # ---- Save results to file ----

  write.csv( eeSA$table, paste0( folder, "/", baseName, "_EE_SA_", varName, ".csv" ) )

  # ------------- Exception handling code (tryCatch) -------------

}, interrupt = function( ex ) {
  cat( "An interruption was detected.\n" )
  print( ex )
  textplot( "Report incomplete due to interruption." )
}, error = function( ex ) {
  cat( "An error was detected.\n" )
  print( ex )
  textplot( "Report incomplete due to processing error." )
}, finally = {
  # Close PDF plot file
  dev.off( )
})
