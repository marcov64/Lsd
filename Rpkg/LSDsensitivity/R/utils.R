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

# ==== Create random experiments ====

# Create random experiment using Sobol low-discrepancy quasi-random sequences
sobolInit <- TRUE
sobol.rnd.exp <- function( n, factors, lwr.bound = 0, upr.bound = 1 ) {

  x <- randtoolbox::sobol( n, length( factors ), scrambling = 1, init = sobolInit )
  x <- as.data.frame( t( t( x ) * ( upr.bound - lwr.bound ) + lwr.bound ) )
  colnames( x ) <- factors
  sobolInit <<- FALSE

  return( x )
}


# ==== Round data frames containing not just numbers ====
# round all numeric variables
# x: data frame
# digits: number of digits to round

round.df <- function( x, digits = 4 ) {

  numeric_columns <- sapply( x, mode ) == 'numeric'
  x[ numeric_columns ] <-  round( x[ numeric_columns ], digits )

  return( x )
}


# ====== Remove outliers ======
# Output:
#  DoE/response tables with outliers removed
#
# Input:
#   doe, resp: DoE (X) and response (y) data frames
#	  limit: limit to consider outliers (number of standard deviations)
#

remove.outliers <- function( doe, resp, limit ) {

  origLen <- nrow( doe )
  if( origLen != nrow( resp ) )
    stop( "Design of Experiments and response files do not match" )

  # check for abnormal DoE sample averages
  m <- mean( resp$Mean )
  d <- stats::sd( resp$Mean ) * limit         # maximum deviation
  outl <- which( resp$Mean > m + d | resp$Mean < m - d, arr.ind = TRUE )
  if( length( outl ) > 0 ) {
    doe <- doe[ - outl, ]
    resp <- resp[ - outl, ]
  }

  # check for too much noise in DoE samples
  m <- sqrt( mean( resp$Variance ) )
  outl <- which( resp$Variance > m * limit, arr.ind = TRUE )
  if( length( outl ) > 0 ) {
    doe <- doe[ - outl, ]
    resp <- resp[ - outl, ]
  }

  removed <- origLen - nrow( doe )
  if( removed > 0.1 * origLen )
    warning( "Too many DoE outliers (>10%), check 'limit' parameter",
             call. = FALSE )

  return( list( doe, resp, removed ) )
}


# ==== Create 3D grid for plotting ====

# Create a grid using data limits for top effect variables and defaults for others

grid.3D <- function( data, sa, gridSz = 25 ) {

  grid <- list( )
  grid[[ 1 ]] <- seq( data$facLimLo[ sa$topEffect[ 1 ] ],
                      data$facLimUp[ sa$topEffect[ 1 ] ], length = gridSz )
  grid[[ 2 ]] <- seq( data$facLimLo[ sa$topEffect[ 2 ] ],
                      data$facLimUp[ sa$topEffect[ 2 ] ], length = gridSz )
  grid[[ 3 ]] <- seq( data$facLimLo[ sa$topEffect[ 3 ] ],
                      data$facLimUp[ sa$topEffect[ 3 ] ], length = gridSz )
  grid[[ 4 ]] <- as.numeric( c( data$facLimLo[ sa$topEffect[ 3 ] ],
                                data$facDef[ sa$topEffect[ 3 ] ],
                                data$facLimUp[ sa$topEffect[ 3 ] ] ) )

  return( grid )
}


# ==== Create factors values lists for plotting ====

# populates lists with the values for all factors to be plotted

factor.lists <- function( data, sa, grid ) {
  top <- list( )
  default <- center <- top[[ 1 ]] <- top[[ 2 ]] <- top[[ 3 ]] <- list( )
  for( i in 1 : length( colnames( data$doe ) ) ) {
    default[[ i ]] <- center[[ i ]] <- top[[ 1 ]][[ i ]] <- top[[ 2 ]][[ i ]] <-
      top [[ 3 ]][[ i ]] <- as.numeric( data$facDef[ i ] )

    if( i == sa$topEffect[ 1 ] )
      center[[ i ]] <- top[[ 1 ]][[ i ]] <- grid[[ 1 ]]
    if( i == sa$topEffect[ 2 ] )
      center[[ i ]] <- top[[ 2 ]][[ i ]] <- grid[[ 2 ]]
    if( i == sa$topEffect[ 3 ] )
      top[[ 3 ]][[ i ]] <- grid[[ 3 ]]
  }

  return( list( top = top, default = default, center = center ) )
}
