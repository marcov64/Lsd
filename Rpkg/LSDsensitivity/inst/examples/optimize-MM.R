#*******************************************************************************
#
# -------------------------- Meta-model optimization --------------------------
#
#	Run this script ONLY after fitting a meta-model for LSD experiment data
#  (by running "kriging-sobol-SA.R" or "poly-sobol-SA.R")
#
#*******************************************************************************

# ====== Find the parameter sets that maximize and minimize the meta-model ======

maxPar <- model.optim.lsd( mModel,                           # estimated kriging meta-model
                           lower.domain = dataSet$facLimLo,  # minimum values for all factors
                           upper.domain = dataSet$facLimUp,  # maximum values for all factors
                           starting.values = dataSet$facDef, # calibration values for all factors
                           minimize = FALSE )

minPar <- model.optim.lsd( mModel,                           # estimated kriging meta-model
                           lower.domain = dataSet$facLimLo,  # minimum values for all factors
                           upper.domain = dataSet$facLimUp,  # maximum values for all factors
                           starting.values = dataSet$facDef, # calibration values for all factors
                           minimize = FALSE )


# ====== Find the meta-model predicted maximum and minimum values ======

maxPred <- model.pred.lsd( maxPar,        # set of factors to predict meta model response
                           mModel )       # estimated kriging meta-model

minPred <- model.pred.lsd( minPar,        # set of factors to predict meta model response
                           mModel )       # estimated kriging meta-model



#*******************************************************************************
#
# ----------------------- Analysis results presentation -----------------------
#
#*******************************************************************************

library( gplots )           # package for plotting tables


# ====== Present meta-model selection details ======

# ------ Model comparison table ------

textplot( signif( mModel$comparison, 4 ), rmar = 1 )
title( main = "Comparison of alternative kriging models",
       sub = "Q2: cross validation Q2 ( higher is better )\nRMSE/MAE/RMA: external validation RMSE/MAE/RMA ( lower is better )" )

# ------ Model estimation table ------

textplot( mModel$estimation.std, show.colnames = FALSE, cmar = 2, rmar = 1 )
title( main = "Kriging meta-model estimation (standardized)",
       sub = paste0( "All variables rescaled to [ 0, 1 ]" ) )


# ====== Present optimization results ======

theta3d <- 40                             # horizontal view angle
phi3d <- 30                               # vertical view angle

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

  # ------ Optimized parameter table ------

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
  title( main = paste( "Settings for", label, "kriging meta-model" ),
         sub = paste( "Predicted output at optimal setting:",
                      dataSet$saVarName, "=", round( pred$mean, digits = 2 ) ) )

  # ------ 3D response surface using optimal parameter values ------

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

  pred2 <- model.pred.lsd( expand.grid( structure( center2, .Names = colnames( dataSet$doe ) ) ),
                        mModel )$mean

  zMat <- matrix( pred2, grid3d, grid3d, byrow = FALSE )

  # handle flat surfaces
  zlim <- range( zMat, na.rm = TRUE )
  if( zlim[ 1 ] == zlim[ 2 ] )
    if( zlim[ 1 ] != 0 ) {
      zlim <- zlim + c( - abs( zlim[ 1 ] ), abs( zlim[ 2 ] ) )
    } else
      zlim <- c( -1, 1 )

  vt2 <- persp( mResp$grid[[ 1 ]], mResp$grid[[ 2 ]], zMat, col = "gray90",
                xlab = colnames( dataSet$doe )[ sSA$topEffect[ 1 ] ], zlim = zlim,
                ylab = colnames( dataSet$doe )[ sSA$topEffect[ 2 ] ], zlab = dataSet$saVarName,
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
}
