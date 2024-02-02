#*******************************************************************************
#
# ---- Island Model: elementary effects (Morris 1991) sensitivity analysis ----
#
#   Written by Marcelo C. Pereira, University of Campinas
#
#   Copyright Marcelo C. Pereira
#   Distributed under the GNU General Public License
#
#   The default configuration assumes that the supplied LSD
#   main simulation configuration (basename morris):
#     R/sa-morris/morris.lsd
#   is used with the also supplied sensitivity analysis (SA) set:
#     R/sa-morris/morris.sa
#   to generate a set of configurations which must be executed
#   before this script is used
#
#   To generate the configuration set, (1) open LSD Model Manager
#   (LMM), (2) in LSD Model Browser, open the model which contains
#   this script (double click), (3) in LMM, compile and run the
#   model (menu Model>Compile and Run), (4) in LSD Model Browser,
#   load the desired configuration (menu File>Load), (5) load the
#   desired SA set (menu File>Load Sensitivity), (6) generate the
#   configuration set (menu Data>Sensitivity Analysis>NOLH Sampling),
#   accepting all the default options, (7) create an out-of-sample
#   set of samples when asked by LSD, accepting the proposed number
#   of samples.
#
#   To execute the simulation set, (8) create a parallel processing
#   batch (menu Run>Parallel Batch), accepting to use the configura-
#   tion set just created and the default options, (9) execute the
#   created batch, accepting the option offered, (10) confirm opening
#   the background run monitor, (11) wait until all simulation runs
#   in the background run monitor finish.
#
#	Several files are required to run:
#       folder/baseName_XX_YY.csv     : DoE specification from LSD
#       folder/baseName_XX_YY_WWW.csv : DoE response from R
#       folder/baseName.lsd           : LSD configuration with default values
#       folder/baseName.sa            : factor sensitivity test ranges from LSD
#
#	Run this script ONLY after producing the required data in LSD
#
#*******************************************************************************

# ==== data load and analysis parameters ====

folder   <- "sa-morris"               # data files folder
baseName <- "morris"                  # data files base name (same as .lsd file)
varName  <- "gM"                      # variable to do sensitivity analysis on
iniDrop  <- 0                         # initial time steps to drop (0=none)
nKeep    <- -1                        # number of time steps to keep (-1=all)
lsdVars  <- c( "Q", "g", "l", "e", "i", "m", "J" )# all variables to use
logVars  <- c( )                      # log variables
newVars  <- c( "gM" )                 # variables to add to LSD dataset

# ====== function to extend the data set ======

eval.vars <- function( dataSet, allVars ) {
  tsteps <- nrow( dataSet )           # timesteps in simulated data set
  if ( dataSet[ tsteps, "Q" ] > 0 )
    dataSet[ , "gM" ] <- log( dataSet[ tsteps, "Q" ] ) / tsteps
  else
    dataSet[ , "gM" ] <- 0
  return( dataSet )
}

# ==== required libraries ====

reqLibs <- c( "LSDsensitivity", "gplots" )

for( lib in reqLibs ) {
  if( ! require( lib, character.only = TRUE, quietly = TRUE ) )
    install.packages( lib, verbose = FALSE )
  require( lib, character.only = TRUE, warn.conflicts = FALSE, quietly = TRUE )
}

# ==== process LSD result files (experimental data set) ====

dataSet <- read.doe.lsd( folder, baseName, varName, iniDrop = iniDrop,
                         nKeep = nKeep, saveVars = lsdVars, addVars = newVars,
                         eval.vars = eval.vars )

# ====== Elementary Effects analysis ======

eeSA <- elementary.effects.lsd( dataSet )

# ====== analysis report ======

# open PDF plot file for output
outFile <- paste0( folder, "/", baseName, "_ee_plots.pdf" )
pdf( outFile, width = 12, height = 8 )
cat( paste( "\nSaving results to:", outFile, "\n" ) )

textplot( signif( eeSA$table, 3 ), cex = 1.0 )
title( main = paste0( "Elementary effects distributions statistics ( ",
                      dataSet$saVarName, " )" ),
       sub = paste0( "All variables rescaled to [ 0, 1 ] / H0: mu.star = 0\nLevels (p) = ",
                     4, ", delta = ", signif( 2 / ( 4 - 1 ) , 3 ),
                     ", trajectories (r) = ", eeSA$r, ", samples = ",
                     eeSA$samples ) )

plot( eeSA )
title( main = paste0( "Elementary effects composition ( ",
                      dataSet$saVarName, " )" ),
       sub = "mu.star: overall effects / sigma: non-linear/non-additive effects" )

dev.off( )
