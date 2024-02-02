#*******************************************************************************
#
# ----------------- Island Model: meta-model optimization --------------------
#
#   Written by Marcelo C. Pereira, University of Campinas
#
#   Copyright Marcelo C. Pereira
#   Distributed under the GNU General Public License
#
#   The default configuration assumes that the supplied LSD
#   main simulation configuration (basename sobol):
#     R/sa-sobol/sobol.lsd
#   is used with the also supplied sensitivity analysis (SA) set:
#     R/sa-sobol/sobol.sa
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
#   Several files are required to run:
#       folder/baseName_XX_YY.csv       : DoE specification from LSD
#       folder/baseName_YY+1_ZZ.csv     : external validation from LSD (optional)
#       folder/baseName_XX_YY_WWW.csv   : DoE response from R
#       folder/baseName_YY+1_ZZ_WWW.csv : ext. validation response from R (optional)
#       folder/baseName.lsd             : LSD configuration with default values
#       folder/baseName.sa              : factor sensitivity test ranges from LSD
#
#   Run this script ONLY after producing the required data in LSD
#
#*******************************************************************************

# ==== data load and analysis parameters ====

folder   <- "sa-sobol"                # data files folder
baseName <- "sobol"                   # data files base name (same as .lsd file)
varName  <- "gM"                      # variable to do sensitivity analysis on
iniDrop  <- 0                         # initial time steps to drop (0=none)
nKeep    <- -1                        # number of time steps to keep (-1=all)
lsdVars  <- c( "Q", "g", "l", "e", "i", "m", "J" )# all variables to use
logVars  <- c( )                      # log variables
newVars  <- c( "gM" )                 # variables to add to LSD dataset
grid3d   <- 25                        # density for 3D wireframe plots
theta3d  <- 320                       # horizontal view angle
phi3d    <- 20                        # vertical view angle

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

reqLibs <- c( "LSDsensitivity", "gplots", "rgl" )

for( lib in reqLibs ) {
  if( ! require( lib, character.only = TRUE, quietly = TRUE ) )
    install.packages( lib, verbose = FALSE )
  require( lib, character.only = TRUE, warn.conflicts = FALSE, quietly = TRUE )
}

# ==== process LSD result files (experimental data set) ====

dataSet <- read.doe.lsd( folder, baseName, varName, does = 2, iniDrop = iniDrop,
                         nKeep = nKeep, saveVars = lsdVars, addVars = newVars,
                         eval.vars = eval.vars )

# ====== estimate the meta-models and select the best one ======

mModel <- kriging.model.lsd( dataSet )

#mModel <- polynomial.model.lsd( dataSet )

# ====== Sobol sensitivity analysis on meta-model ======

sSA <- sobol.decomposition.lsd( dataSet, mModel, sa.samp = 5000 )

# ====== response surface 3D data ======

mResp <- response.surface.lsd( dataSet, mModel, sSA )

# ====== parameter sets that maximize and minimize the meta-model ======

maxPar <- model.optim.lsd( mModel,                           # estimated meta-model
                           lower.domain = dataSet$facLimLo,  # minimum values
                           upper.domain = dataSet$facLimUp,  # maximum values
                           starting.values = dataSet$facDef, # calibration values
                           minimize = FALSE )

minPar <- model.optim.lsd( mModel,
                           lower.domain = dataSet$facLimLo,
                           upper.domain = dataSet$facLimUp,
                           starting.values = dataSet$facDef )

# ====== meta-model predicted maximum and minimum values ======

maxPred <- model.pred.lsd( maxPar, mModel )

minPred <- model.pred.lsd( minPar, mModel )

# ====== meta-model comparison ======

outFile <- paste0( folder, "/", baseName, "_optim_plots.pdf" )
pdf( outFile, width = 12, height = 8 )
cat( paste( "\nSaving results to:", outFile, "\n" ) )

mClass <- class( mModel )
if ( mClass == "polynomial-model" ) {
  colNames <- TRUE
} else
  colNames <- FALSE

textplot( signif( mModel$comparison, 4 ), rmar = 1 )
title( main = paste( "Comparison of alternative models (", mClass, ")" ),
       sub = "Q2: cross validation Q2 ( higher is better )\nRMSE/MAE/RMA: external validation RMSE/MAE/RMA ( lower is better )" )

textplot( mModel$estimation.std, show.colnames = colNames, cmar = 2, rmar = 1 )
title( main = "Meta-model estimation (standardized)",
       sub = paste0( "All variables rescaled to [ 0, 1 ]" ) )

# ====== optimization results ======

for( k in c( TRUE, FALSE ) ) {            # minimize and maximize

  if( k ) {
    label <- "minimizing"
    par <- minPar
    pred <- minPred
  } else {
    label <- "maximizing"
    par <- maxPar
    pred <- maxPred
  }

  if( is.na( par[ 1 ] ) ) {
    textplot( paste( par[1], "\n\nThe", label, "has failed!" ) )
    next
  }

  # optimized parameter table
  print <- signif( t( par ), 4 )
  nRows <- length( print )
  maxRows <- 18
  if( nRows > maxRows ) {                  # too many rows for a single column?
    nMove <- floor( nRows / 2 )
    col2.label <- names( print[ ( nRows - nMove + 1 ) : nRows, 1 ] )
    col2.value <- print[ ( nRows - nMove + 1 ) : nRows ]
    print <- print[ - ( ( nRows - nMove + 1 ) : nRows ), ]
    if( nMove < length( print ) ) {
      col2.label <- append( col2.label, "" )
      col2.value <- append( col2.value, "" )
    }
    print <- cbind( print, col2.label, col2.value )
  }

  textplot( print, show.colnames = FALSE, cex = 1 )
  title( main = paste( "Settings for", label, "meta-model (", mClass, ")" ),
         sub = paste( "Predicted output at optimal setting:",
                      dataSet$saVarName, "=", round( pred$mean, digits = 2 ) ) )

  # 3D response surface using optimal parameter values
  center2 <- list( )
  for( i in 1 : length( colnames( dataSet$doe ) ) ) {
    if( i == sSA$topEffect[ 1 ] )
      center2[[ i ]] <- mResp$grid[[ 1 ]]
    else
      if( i == sSA$topEffect[ 2 ] )
        center2[[ i ]] <- mResp$grid[[ 2 ]]
      else
        center2[[ i ]] <- as.numeric( par[ i ] )
  }

  pred2 <- model.pred.lsd( expand.grid( structure( center2,
                                                   .Names = colnames( dataSet$doe ) ) ),
                           mModel )$mean
  zMat <- matrix( pred2, grid3d, grid3d, byrow = FALSE )
  zlim <- range( zMat, na.rm = TRUE )
  if( zlim[ 1 ] == zlim[ 2 ] )          # handle flat surfaces
    if( zlim[ 1 ] != 0 ) {
      zlim <- zlim + c( - abs( zlim[ 1 ] ), abs( zlim[ 2 ] ) )
    } else
      zlim <- c( -1, 1 )

  vt2 <- persp( mResp$grid[[ 1 ]], mResp$grid[[ 2 ]], zMat, col = "gray90",
                xlab = colnames( dataSet$doe )[ sSA$topEffect[ 1 ] ],
                zlim = zlim,
                ylab = colnames( dataSet$doe )[ sSA$topEffect[ 2 ] ],
                zlab = dataSet$saVarName,
                theta = theta3d, phi = phi3d, ticktype = "detailed"  )
  points( trans3d( as.numeric( par[ sSA$topEffect[ 1 ] ] ),
                   as.numeric( par[ sSA$topEffect[ 2 ] ] ),
                   pred$mean, vt2 ), col = "red", pch = 19, cex = 1.0 )
  title( main = paste( "Response surface for", label, "settings" ),
         sub = paste0( "95% confidence interval: ", dataSet$saVarName, " = [",
                       round( pred$lower, 2 ), ",",
                       round( pred$upper, 2 ), "] at optimum (point)" ) )

  contour( mResp$grid[[ 1 ]], mResp$grid[[ 2 ]], zMat, nlevels = 12 )
  points( par[ sSA$topEffect[ 1 ] ], par[ sSA$topEffect[ 2 ] ],
          col = "red", pch = 19, cex = 1.5 )
  title( main = paste( "Response surface for", label, "settings" ),
         sub = paste0( "95% confidence interval: ", dataSet$saVarName, " = [",
                       round( pred$lower, 2 ), ",",
                       round( pred$upper, 2 ), "] at optimum (red dot)" ),
         xlab = colnames( dataSet$doe )[ sSA$topEffect[ 1 ] ],
         ylab = colnames( dataSet$doe )[ sSA$topEffect[ 2 ] ] )

  # 3D shaded response surface
  persp3d( mResp$grid[[ 1 ]], mResp$grid[[ 2 ]], zMat, col = "lightgray",
           xlab = colnames( dataSet$doe )[ sSA$topEffect[ 1 ] ],
           ylab = colnames( dataSet$doe )[ sSA$topEffect[ 2 ] ],
           zlab = dataSet$saVarName )
}

dev.off( )
