#*******************************************************************************
#
# ------------------- LSD tools for sensitivity analysis ---------------------
#
#   Written by Marcelo C. Pereira, University of Campinas
#
#   Copyright Marcelo C. Pereira
#   Distributed under the GNU General Public License
#
#*******************************************************************************

# ==== Compute meta-model response surface ====

response.surface.lsd <- function( data, model, sa, gridSz = 25, defPos = 2,
                                  factor1 = 0, factor2 = 0, factor3 = 0 ) {

  if( ! inherits( data, "doe.lsd" ) )
    stop( "Invalid data (not from read.doe.lsd())" )

  if( ! inherits( model, "kriging.model.lsd" ) &&
      ! inherits( model, "polynomial.model.lsd" ) )
    stop( "Invalid model (not from polynomial or kriging.model.lsd())" )

  if( ! inherits( sa, "polynomial.sensitivity.lsd" ) &&
      ! inherits( sa, "kriging.sensitivity.lsd" ) )
    stop( "Invalid sensitivity analysis (not from sobol.decomposition.lsd())" )

  if( is.null( gridSz ) || ! is.finite( gridSz ) || round( gridSz ) < 1 )
    stop( "Invalid size of grid division (gridSz)" )

  if( is.null( defPos ) || ! is.finite( defPos ) || round( defPos ) < 1 ||
      round( defPos ) > 3 )
    stop( "Invalid default 3D plot position (defPos)" )

  if( is.null( factor1 ) || ! is.finite( factor1 ) || round( factor1 ) < 0 )
    stop( "Invalid index for first factor (factor1)" )

  if( is.null( factor2 ) || ! is.finite( factor2 ) || round( factor2 ) < 0 )
    stop( "Invalid index for second factor (factor2)" )

  if( is.null( factor3 ) || ! is.finite( factor3 ) || round( factor3 ) < 0 )
    stop( "Invalid index for third factor (factor3)" )

  gridSz <- round( gridSz )
  defPos <- round( defPos )
  factor1 <- round( factor1 )
  factor2 <- round( factor2 )
  factor3 <- round( factor3 )

  # check if use fixed or top factors
  if( factor1 != 0 )
    sa$topEffect[ 1 ] <- factor1
  if( factor2 != 0 )
    sa$topEffect[ 2 ] <- factor2
  if( factor3 != 0 )
    sa$topEffect[ 3 ] <- factor3

  # create drawing grid  with default ("center") values plus top factor full ranges
  grid <- grid.3D( data, sa, gridSz = gridSz )

  # get the values for the factors for 3 different 3D plots
  # each plot uses a different value (min, default, max) for the third most important factor
  factors <- factor.lists( data, sa, grid )

  # get the predicted responses for each plot
  response <- predicted.responses( data, model, sa, grid, factors )
  response[[ "grid" ]] <- grid
  response[[ "factors" ]] <- factors

  # calculate average 95% confidence interval maximum deviation over calibration point surface
  response[[ "avgCIdev" ]] <- mean( mean( response$calib[[ defPos ]]$mean -
                                            response$calib[[ defPos ]]$lower ),
                                    mean( response$calib[[ defPos ]]$upper -
                                            response$calib[[ defPos ]]$mean ) )
  class( response ) <- "response"

  return( response )
}


# ==== Calculate predicted responses ====

# predict model results given data

model.pred.lsd <- function( data.point, model ) {

  if( ! is.data.frame( data.point ) )
    stop( "Invalid data frame (data.point)" )

  x <- is.na( data.point[ 1, ] )
  if( length( x[ x == TRUE ] ) > 0 )
    return( NULL )

  if( inherits( model, "kriging.model.lsd" ) ) {
    out <- DiceKriging::predict( model$selected, data.point, type = "UK" )
    out <- list( mean = out$mean, lower = out$lower95, upper = out$upper95 )
  } else {
    if( inherits( model, "polynomial.model.lsd" ) ) {
      out <- stats::predict( model$selected, data.point, type = "response",
                             interval = "confidence"  )
      out <- list( mean = out[ , "fit" ], lower = out[ , "lwr" ],
                   upper = out[ , "upr" ] )
    } else
      stop( "Invalid model (not from polynomial or kriging.model.lsd())" )
  }

  return( out )
}


# calculate the predicted responses for factors lists for plotting

predicted.responses <- function( data, model, sa, grid, factors ) {

  calib <- predFac <- list( )

  # get predicted values for default configuration
  predDef <- model.pred.lsd( as.data.frame( structure( factors$default,
                                                       .Names = colnames( data$doe ) ) ),
                             model )

  # get predicted values for individual factors
  for( i in 1 : 3 )
    predFac[[ i ]] <- model.pred.lsd( as.data.frame( structure( factors$top[[ i ]],
                                                                .Names = colnames( data$doe ) ) ),
                                      model )

  # get predicted values for all 3D configurations
  for( i in 1 : length( grid[[ 4 ]] ) ) {
    # calculate the 3D response surface for each point in the grid
    factors$center[[ sa$topEffect[ 3 ] ]] <- grid[[ 4 ]][ i ]
    calib[[ i ]] <- model.pred.lsd( expand.grid( structure( factors$center,
                                                            .Names = colnames( data$doe ) ) ),
                                    model )
  }

  return( list( calib = calib, factor = predFac, default = predDef ) )
}
