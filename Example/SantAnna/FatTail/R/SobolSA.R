#******************************************************************
#
# -------- Industry Model: Sobol Kriging DoE analysis ------------
#
#   Written by Marcelo C. Pereira, University of Campinas
#
#   Copyright Marcelo C. Pereira
#   Distributed under the GNU General Public License
#
#	Several files are required to run:
#		folder/baseName_XX_YY.csv		    : DoE specification from LSD
#		folder/baseName_YY+1_ZZ.csv		  : external validation from LSD (optional)
#		folder/baseName_XX_YY_WWW.csv	  : DoE response from R
#		folder/baseName_YY+1_ZZ_WWW.csv	: ext. validation response from R (optional)
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
folder   <- "MarkII/Beta/Sobol"       # data files folder
baseName <- "MarkII-Beta"             # data files base name (same as .lsd file)
varName <- "b"						            # analysis variable name
iniDrop <- 0                          # initial time steps to drop from analysis (0=none)
nKeep <- -1                           # number of time steps to keep (-1=all)

lsdVars <- c( )                       # no existing variable to be saved
newVars <- c( "b" )                   # add the Subbotin b parameter

# Flags for removing outliers from DoE & validation samples (use with care!)
remOutl <- FALSE                      # remove outliers from DoE & validation samples?
limOutl <- 10                         # limit deviation (number of standard deviations)
does <- 2                             # number of DoEs (1=only cross validation / 2=external valid.)
saSamples <- 5000                     # number of samples in sensitivity analysis
conf <- 0.95                          # confidence level to use

# Report output configuration
plotRows <- 1					                # number of plots per row in a page
plotCols <- 1					                # number of plots per column in a page
plotW <- 10                           # plot window width
plotH <- 7                            # plot window height

# 3D surfaces visualization control
surf3d <- FALSE                       # ask the user to adjust a 3D perspective of response surface
theta3d <- 310                        # horizontal view angle
phi3d <- 30                           # vertical view angle

# Force selection of specific trend model and correlation function (0=auto)
trendModel <- 0                       # trend model (1=constant / 2=1st order polynomial)
covModel <- 0                         # 1=Matern5/2 / 2=Matern3/2 / 3=Gaussian / 4=expon. /5=power exp.
extWgth <- 0.5                        # weight of external error measures on model selection (0-1)

# Force usage of specific factors in response surface graphs (0=auto)
factor1 <- 0                          # first factor (# according to DoE factor order)
factor2 <- 0                          # second factor
factor3 <- 0                          # third factor


# ====== External support functions & definitions ======

source( "StatFuncs.R" )


# ==== Process LSD set of result files (an experimental data set) ====

dataSet <- read.doe.lsd( folder,                 # data files relative folder
                         baseName,               # data files base name (same as .lsd file)
                         varName,				         # variable name to perform the sensitivity analysis
                         does = does,            # number of experiments (data + external validation)
                         iniDrop = iniDrop,      # initial time steps to drop from analysis (0=none)
                         nKeep = nKeep,          # number of time steps to keep (-1=all)
                         saveVars = lsdVars,     # LSD variables to keep in dataset
                         addVars = newVars,      # new variables to add to the LSD dataset
                         eval.vars = eval.b,     # function to evaluate/adjust/expand the dataset
                         rm.temp = TRUE,         # remove temporary speedup files?
                         rm.outl = remOutl,      # remove outliers from dataset
                         lim.outl = limOutl )    # limit non-outlier deviation (number of std. devs.)

# ====== Estimate Kriging meta-models and select the best one ======

mModel <- kriging.model.lsd( dataSet,            # LSD experimental data set
                             ext.wgth = extWgth, # weight of external errors on model selection (0-1)
                             trendModel = trendModel, # trend model
                             covModel = covModel,# covariance kernel
                             digits = 3 )        # precision digits


# ====== Sobol sensitivity analysis on meta-model ======

sSA <- sobol.decomposition.lsd( dataSet,         # LSD experimental data set
                                mModel,          # estimated meta-model
                                krig.sa = FALSE, # use alternative kriging-specific SA algorithm
                                sa.samp = saSamples ) # number of samples in SA


# ====== Generate response surface 3D data for plotting ======

mResp <- response.surface.lsd( dataSet,          # LSD experimental data set
                               mModel,           # estimated meta-model
                               sSA,              # Sobol sensitivity analysis results
                               gridSz = 25,      # density for 3D wireframe (grid) plots
                               defPos = 2,       # default settings position in plot set (1, 2 or 3)
                               factor1 = factor1,# first factor according to DoE factor order)
                               factor2 = factor2,# second factor according to DoE factor order)
                               factor3 = factor3 )# third factor according to DoE factor order)


# ===== Find maximum and minimum response values for top factors =====

maxMin <- model.limits.lsd( dataSet,             # LSD experimental data set
                            mModel,              # estimated meta-model
                            sSA )                # use auto factor order


# ---- Enter error handling mode so PDF can be closed in case of error/interruption ----

tryCatch({

  # ---- Open PDF plot file for output ----

  pdf( paste0( folder, "/", baseName, "_Sobol.pdf" ),
       width = plotW, height = plotH )
  options( scipen = 3 )                 # max 3 digits
  par( mfrow = c ( plotRows, plotCols ) )             # define plots per page

  # ====== Present meta-model selection details ======

  # ------ Model comparison table ------

  textplot( signif( mModel$comparison, 4 ), rmar = 1 )
  title( main = "Comparison of alternative kriging models",
         sub = "Q2: cross validation Q2 ( higher is better )\nRMSE/MAE/RMA: external validation RMSE/MAE/RMA ( lower is better )" )

  # ------ Model estimation table ------

  textplot( mModel$estimation.std, show.colnames = FALSE, cmar = 2, rmar = 1 )
  title( main = "Kriging meta-model estimation (standardized)",
         sub = paste0( "All variables rescaled to [ 0, 1 ] / Average 95% CI = +/- ",
                       round( mResp$avgCIdev, digits = 2 ), "\nPredicted output at defaults: ",
                       dataSet$saVarName, " = ", round( mResp$default$mean, digits = 2 ), " / 95% CI = [",
                       round( mResp$default$lower, digits = 2 ), ",",
                       round( mResp$default$upper, digits = 2 ), "]" ) )


  # ====== Sobol sensitivity analysis results ======

  # ------ Sobol sensitivity analysis table ------

  textplot( signif( sSA$sa, 4 ) )
  title( main = "Sobol decomposition sensitivity analysis" )

  # ------ Sobol sensitivity analysis chart ------

  barplot( t( sSA$sa ), col = c( "white", "gray" ), las = 2, ylim = c( 0, 1 ),
           legend.text = c( "main effects", "interactions" ) )
  title( main = "Sobol decomposition sensitivity analysis", ylab = "Sobol Index" )


  # ====== Plot 2D response surface ======

  # plot 2D charts for top factors (around defaut parameter configuration)
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

  grid3d <- 25                          # density for 3D wireframe plots
  theta3d <- 40                         # horizontal view angle
  phi3d <- 30                           # vertical view angle

  if( sSA$topEffect[ 1 ] > sSA$topEffect[ 2 ] ) {      # check if we have to swap x and y axis
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
                 xlab = colnames( dataSet$doe )[ sSA$topEffect[ 1 ] ], zlim = zlim,
                 ylab = colnames( dataSet$doe )[ sSA$topEffect[ 2 ] ], zlab = dataSet$saVarName,
                 theta = theta3d, phi = phi3d, ticktype = "detailed" )

    if( length( mResp$grid[[ 4 ]] ) == 1 ||
        ( length( mResp$grid[[ 4 ]] ) == 3 && i == 2 ) ) {
      points( trans3d( as.numeric( dataSet$facDef[ sSA$topEffect[ 1 ] ] ),
                       as.numeric( dataSet$facDef[ sSA$topEffect[ 2 ] ] ),
                       mResp$default$mean, vt ), col = "red", pch = 19, cex = 1.0 )
      points( trans3d( maxMin[ sSA$topEffect[ 1 ], 7 ],
                       maxMin[ sSA$topEffect[ 2 ], 7 ],
                       maxMin[ "response", 7 ], vt ), col = "green", pch = 18, cex = 1.0 )
      points( trans3d( maxMin[ sSA$topEffect[ 1 ], 8 ],
                       maxMin[ sSA$topEffect[ 2 ], 8 ],
                       maxMin[ "response", 8 ], vt ), col = "blue", pch = 18, cex = 1.0 )
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
  }


  # ====== 3D response surface (isolevels) ======

  for( i in 1 : length( mResp$grid[[ 4 ]] ) ) {
    zMat <- matrix( mResp$calib[[ i ]]$mean, grid3d, grid3d, byrow = swapXY )
    contour( mResp$grid[[ 1 ]], mResp$grid[[ 2 ]], zMat, nlevels = 12 )
    if( length( mResp$grid[[ 4 ]] ) == 1 ||
        ( length( mResp$grid[[ 4 ]] ) == 3 && i == 2 ) ) {
      points( dataSet$facDef[ sSA$topEffect[ 1 ] ], dataSet$facDef[ sSA$topEffect[ 2 ] ],
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

  # ---- Save results to file ----
  
  write.csv( mModel$comparison, paste0( folder, "/", baseName,
                                        "_Kriging_models_", varName, ".csv" ) )
  write.csv( mModel$estimation.std, paste0( folder, "/", baseName,
                                            "_Kriging_estim_", varName, ".csv" ) )
  write.csv( sSA$sa, paste0( folder, "/", baseName, "_Sobol_SA_", varName, ".csv" ) )

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