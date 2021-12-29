#*******************************************************************************
#
# ------------------- LSD tools for sensitivity analysis ---------------------
#
#   Written by Marcelo Pereira, University of Campinas
#
#   Copyright Marcelo Pereira
#   Distributed under the GNU General Public License
#
#*******************************************************************************

# ==== Compute meta-model response surface ====

response.surface.lsd <- function( data, model, sa, gridSz = 25, defPos = 2,
                                  factor1 = 0, factor2 = 0, factor3 = 0 ) {

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

  x <- is.na( data.point[ 1, ] )
  if( length( x[ x == TRUE ] ) > 0 )
    return( NULL )

  if( class( model ) == "kriging-model" ) {
    out <- DiceKriging::predict( model$selected, data.point, type = "UK" )
    out <- list( mean = out$mean, lower = out$lower95, upper = out$upper95 )
  } else {
    out <- stats::predict( model$selected, data.point, type = "response", interval = "confidence"  )
    out <- list( mean = out[ , "fit" ], lower = out[ , "lwr" ], upper = out[ , "upr" ] )
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
