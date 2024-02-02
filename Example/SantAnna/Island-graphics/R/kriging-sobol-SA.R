#*******************************************************************************
#
# --------- Island Model: NOLH Kriging Sobol sensitivity analysis -------------
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

# ====== estimate Kriging meta-models and select the best one ======

mModel <- kriging.model.lsd( dataSet )

# ====== Sobol sensitivity analysis on meta-model ======

sSA <- sobol.decomposition.lsd( dataSet, mModel, sa.samp = 5000 )

# ====== response surface 3D data ======

mResp <- response.surface.lsd( dataSet, mModel, sSA )

# ===== maximum and minimum response values for top factors =====

maxMin <- model.limits.lsd( dataSet, mModel, sSA )

# ====== sensitivity analysis report ======

outFile <- paste0( folder, "/", baseName, "_kriging_sobol_plots.pdf" )
pdf( outFile, width = 12, height = 8 )
cat( paste( "\nSaving results to:", outFile, "\n" ) )

textplot( signif( mModel$comparison, 4 ), rmar = 1 )
title( main = "Comparison of alternative kriging models",
       sub = "Q2: cross validation Q2 ( higher is better )\nRMSE/MAE/RMA: external validation RMSE/MAE/RMA ( lower is better )" )

textplot( mModel$estimation.std, show.colnames = FALSE, cmar = 2, rmar = 1 )
title( main = "Kriging meta-model estimation (standardized)",
       sub = paste0( "All variables rescaled to [ 0, 1 ] / Average 95% CI = +/- ",
                     round( mResp$avgCIdev, digits = 2 ),
                     "\nPredicted output at defaults: ", dataSet$saVarName,
                     " = ", round( mResp$default$mean, digits = 2 ),
                     " / 95% CI = [", round( mResp$default$lower, digits = 2 ),
                     ",", round( mResp$default$upper, digits = 2 ), "]" ) )

textplot( signif( sSA$sa, 4 ) )
title( main = "Sobol decomposition sensitivity analysis" )

barplot( t( sSA$sa ), col = c( "white", "gray" ), las = 2, ylim = c( 0, 1 ),
         legend.text = c( "main effects", "interactions" ) )
title( main = "Sobol decomposition sensitivity analysis", ylab = "Sobol Index" )

# ====== Plot 2D response surface ======

for( i in 1 : 3 ) {
  plot( mResp$grid[[ i ]], mResp$factor[[ i ]]$mean,
        ylim = c( min( mResp$calib[[ 2 ]]$lower ),
                  max( mResp$calib[[ 2 ]]$upper ) ),
        type = "l", xlab = colnames( dataSet$doe )[ sSA$topEffect[ i ] ],
        ylab = dataSet$saVarName,
        main = paste0( "Meta-model response for parameter '",
                       colnames( dataSet$doe )[ sSA$topEffect[ i ] ], "'" ),
        sub = "Dotted lines at 95% confidence interval" )
  lines( mResp$grid[[ i ]], mResp$factor[[ i ]]$lower, col = "black", lty = 2 )
  lines( mResp$grid[[ i ]], mResp$factor[[ i ]]$upper, col = "black", lty = 2 )
  points( as.numeric( dataSet$facDef[ sSA$topEffect[ i ] ] ), mResp$default$mean,
          col = "red", pch = 19, cex = 1.5 )
  points( maxMin[ sSA$topEffect[ i ], ( i * 2 ) - 1 ],
          maxMin[ "response", ( i * 2 ) - 1 ],
          col = "green", pch = 18, cex = 1.5 )
  points( maxMin[ sSA$topEffect[ i ], i * 2 ],
          maxMin[ "response", i * 2 ],
          col = "blue", pch = 18, cex = 1.5 )
}

# ====== 3D response surface (wireframe & shaded) ======

if( sSA$topEffect[ 1 ] > sSA$topEffect[ 2 ] ) {      # have to swap x and y axis
  swapXY <- TRUE
} else
  swapXY <- FALSE

for( i in 1 : length( mResp$grid[[ 4 ]] ) ) {        # do for each top factor

  # plot 3D grid charts
  zMat <- matrix( mResp$calib[[ i ]]$mean, grid3d, grid3d, byrow = swapXY )

  # handle flat surfaces
  zlim <- range( zMat, na.rm = TRUE )
  if( zlim[ 1 ] == zlim[ 2 ] )
    if( zlim[ 1 ] != 0 ) {
      zlim <- zlim + c( - abs( zlim[ 1 ] ), abs( zlim[ 2 ] ) )
    } else
      zlim <- c( -1, 1 )

  vt <- persp( mResp$grid[[ 1 ]], mResp$grid[[ 2 ]], zMat, col = "gray90",
               xlab = colnames( dataSet$doe )[ sSA$topEffect[ 1 ] ],
               zlim = zlim,
               ylab = colnames( dataSet$doe )[ sSA$topEffect[ 2 ] ],
               zlab = dataSet$saVarName,
               theta = theta3d, phi = phi3d, ticktype = "detailed" )

  if( length( mResp$grid[[ 4 ]] ) == 1 ||
      ( length( mResp$grid[[ 4 ]] ) == 3 && i == 2 ) ) {
    points( trans3d( as.numeric( dataSet$facDef[ sSA$topEffect[ 1 ] ] ),
                     as.numeric( dataSet$facDef[ sSA$topEffect[ 2 ] ] ),
                     mResp$default$mean, vt ),
            col = "red", pch = 19, cex = 1.0 )
    points( trans3d( maxMin[ sSA$topEffect[ 1 ], 7 ],
                     maxMin[ sSA$topEffect[ 2 ], 7 ],
                     maxMin[ "response", 7 ], vt ),
            col = "green", pch = 18, cex = 1.0 )
    points( trans3d( maxMin[ sSA$topEffect[ 1 ], 8 ],
                     maxMin[ sSA$topEffect[ 2 ], 8 ],
                     maxMin[ "response", 8 ], vt ),
            col = "blue", pch = 18, cex = 1.0 )
    subTitle <- paste0( "95% confidence interval: ", dataSet$saVarName, " = [",
                        round( mResp$default$lower, 2 ),
                        ",", round( mResp$default$upper, 2 ),
                        "] at defaults (red dot)" )
  } else
    subTitle <- paste0( "All other parameters are at default settings" )

  title( main = paste( paste( "Meta-model response surface (",
                              colnames( dataSet$doe )[ sSA$topEffect[ 3 ] ],
                              "=", mResp$grid[[ 4 ]][ i ], ")" ) ),
         sub = subTitle )

  # 3D shaded response surface
  persp3d( mResp$grid[[ 1 ]], mResp$grid[[ 2 ]],
           matrix( mResp$calib[[ i ]]$mean, grid3d, grid3d ), col = "lightgray",
           xlab = colnames( dataSet$doe )[ sSA$topEffect[ 1 ] ],
           ylab = colnames( dataSet$doe )[ sSA$topEffect[ 2 ] ],
           zlab = dataSet$saVarName )
}

# ====== 3D response surface (isolevels) ======

for( i in 1 : length( mResp$grid[[ 4 ]] ) ) {
  zMat <- matrix( mResp$calib[[ i ]]$mean, grid3d, grid3d, byrow = swapXY )
  contour( mResp$grid[[ 1 ]], mResp$grid[[ 2 ]], zMat, nlevels = 12 )
  if( length( mResp$grid[[ 4 ]] ) == 1 ||
      ( length( mResp$grid[[ 4 ]] ) == 3 && i == 2 ) ) {
    points( dataSet$facDef[ sSA$topEffect[ 1 ] ],
            dataSet$facDef[ sSA$topEffect[ 2 ] ],
            col = "red", pch = 19, cex = 1.5 )
    points( maxMin[ sSA$topEffect[ 1 ], 7 ],
            maxMin[ sSA$topEffect[ 2 ], 7 ],
            col = "green", pch = 18, cex = 1.5 )
    points( maxMin[ sSA$topEffect[ 1 ], 8 ],
            maxMin[ sSA$topEffect[ 2 ], 8 ],
            col = "blue", pch = 18, cex = 1.5 )
    subTitle <- paste0( "95% confidence interval: ", dataSet$saVarName, " = [",
                        round( mResp$default$lower, 2 ),
                        ",", round( mResp$default$upper, 2 ),
                        "] at defaults (red dot)" )
  } else
    subTitle <- paste0( "All other parameters are at default settings" )

  title( main = paste( "Meta-model response surface (",
                       colnames( dataSet$doe )[ sSA$topEffect[ 3 ] ], "=",
                       mResp$grid[[ 4 ]][ i ], ")" ),
         sub = subTitle, xlab = colnames( dataSet$doe )[ sSA$topEffect[ 1 ] ],
         ylab = colnames( dataSet$doe )[ sSA$topEffect[ 2 ] ] )
}

dev.off( )
