#*******************************************************************************
#
# ------------------- LSD tools for sensitivity analysis ---------------------
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


# ===== Wald-Wolfowitz test for ergodicity (Grazzini 2012) ======
# Adapted from Jakob Grazzini (2012), Analysis of the Emergent Properties:
# Stationarity and Ergodicity, Journal of Artificial Societies and Social
# Simulation 15 (2) 7
#
# Output:
#   The Wald-Wolfowitz test z standardized statistic and the p-value
#
# Input:
#   time.series: a list composed by many time series that have to be long
#     as the sub time series computed over the first long time series
#   window.size: length of the sub time series window

ww.test <- function( time.series, window.size ) {

  # time.series is formed by k + 1 processes. take the first series divide it in k
  # samples and compute the moments, then compare such a sample with the moments
  # obtained from the k processes

  if( window.size < 50 )
    warning( "The window size is small (< 50))", call. = FALSE )

  for( i in 1 : length( time.series ) ) {
    time.series[[ i ]] <- time.series[[ i ]][ ! is.na( time.series[[ i ]] ) ]
    if( length( time.series[[ i ]] ) < window.size )
      stop( "Series have to be at least as long as the windows size" )
  }

  obs <- length( time.series[[ 1 ]] )
  k <- floor( obs / window.size )
  if( k > length( time.series ) - 1 )
    stop( "Insufficient series for the selected window size" )
  if( k < length( time.series ) - 1 )
    warning( "Not all series being used (window may be too large)", call. = FALSE )

  # series have to be long to have many well estimated moments
  # divide the first series in k samples and compute the sample means

  r <- vector( )
  rr <- list( )
  j <- 0
  for( i in 1 : obs ) {
    r <- append( r, time.series[[ 1 ]][ i ] )
    if( i %% window.size == 0 ) {
      j <- j + 1
      rr[[ j ]] <- r
      r <- vector( )
    }
  }

  sample_means <- process_means <- vector( )
  for( i in 1 : j )
    sample_means <- append( sample_means, mean( rr[[ i ]] ) )
  for( i in 1 : k )
    process_means <- append( process_means, mean( time.series[[ i + 1 ]] ) )

  # we have two samples
  all_moments <- data.frame( row.names = c( "mean", "symbol" ) )
  for( i in sample_means )
    all_moments <- rbind( all_moments, c( i, 0 ) )
  for( i in process_means )
    all_moments <- rbind( all_moments, c( i, 1 ) )

  sortedMoments <- all_moments[ order( all_moments[ , 1 ] ), ]

  V <- sortedMoments[ , 2 ]

  # runs test over V
  state <- -1
  ones <- zeros <- runs <- 0
  for( i in V ) {
    if( i == 1 )
      ones <- ones + 1

    if( i == 1 && state != 1 ) {
      state <- 1
      runs <- runs + 1
    }

    if( i == 0 )
      zeros <- zeros + 1

    if( i == 0 && state != 0 ) {
      state <- 0
      runs <- runs + 1
    }
  }

  # if runs are to few then we reject
  mu <- ( ( 2 * ones * zeros ) / ( ones + zeros ) ) + 1
  s2 <- ( ( mu - 1 ) * ( mu - 2 ) ) / ( zeros + ones - 1 )
  sigma = sqrt( s2 )
  z <- ( runs - mu ) / sigma
  p_value <- stats::pnorm( z )

  # one tail test!! only of runs are few!! see wald wolfowitz
  # the previous test took as p value the density of the normal with abs(z) -1
  # this meant that p value was law if z was negative or positive and high in absolute value
  # with one tail (smallest) value we just have to compute the density to z, if it is small
  # then we reject!!

  return( list( statistic = z, p.value = p_value ) )
}


# ==== Calculate stationarity and ergodicity statistics table

ergod.test.lsd <- function( data, vars = names( data[ 1, , 1 ] ), start.period = 0,
                            signif = 0.05, digits = 2, ad.method = "asymptotic" ) {

  stats <- c( "avg ADF", "rej ADF", "avg PP", "rej PP", "avg KPSS", "rej KPSS",
              "avg BDS", "rej BDS", "avg KS", "rej KS", "AD", "WW" )
  statErgo <- data.frame( matrix( nrow = length( vars ), ncol = length( stats ),
                                  dimnames = list( vars, stats ) ) )
  nTsteps <- length( data[ , 1, 1 ] )
  nSize <- length( data[ 1, 1, ] )
  if( start.period >= nTsteps )
    stop( "Invalid start period" )

  # do tests individually for each variable
  for( j in vars ) {
    # extract each series as a matrix where time in is the rows
    # and MC instances in the columns
    series <- data[ start.period : nTsteps, j, ]
    series.list <- list( )
    nADF <- nPP <- nKPSS <- nBDS <- nKS <-
      sumADF <- sumPP <- sumKPSS <- sumBDS <- sumKS <-
      rejADF <- rejPP <- rejKPSS <- rejBDS <- rejKS <- 0
    # test all instances for stationarity
    for( n in 1 : nSize ) {
      series.list[[ n ]] <- series[ , n ]
      adf <- pp <- kpss <- bds <- NA
      try( suppressWarnings( adf <- tseries::adf.test( series[ , n ] )$p.value ),
           silent = TRUE )
      if( is.finite( adf ) ) {
        nADF <- nADF + 1
        sumADF <- sumADF + adf
        if( adf < signif ) rejADF <- rejADF + 1
      }
      try( suppressWarnings( pp <- stats::PP.test( series[ , n ] )$p.value ),
           silent = TRUE )
      if( is.finite( pp ) ) {
        nPP <- nPP + 1
        sumPP <- sumPP + pp
        if( pp < signif ) rejPP <- rejPP + 1
      }
      try( suppressWarnings( kpss <- tseries::kpss.test( series[ , n ] )$p.value ),
           silent = TRUE )
      if( is.finite( kpss ) ) {
        nKPSS <- nKPSS + 1
        sumKPSS <- sumKPSS + kpss
        if( kpss < signif ) rejKPSS <- rejKPSS + 1
      }
      try( suppressWarnings( bds <- mean( tseries::bds.test( series[ , n ] )$p.value,
                                          na.rm = TRUE ) ), silent = TRUE )
      if( is.finite( bds ) ) {
        nBDS <- nBDS + 1
        sumBDS <- sumBDS + bds
        if( bds < signif ) rejBDS <- rejBDS + 1
      }
      if( n < nSize )
        for( m in ( n + 1 ) : nSize ) {
          ks <- NA
          try( suppressWarnings( ks <- stats::ks.test( series[ , n ],
                                                      series[ , m ] )$p.value ),
               silent = TRUE )
          if( is.finite( ks ) ) {
            nKS <- nKS + 1
            sumKS <- sumKS + ks
            if( ks < signif ) rejKS <- rejKS + 1
          }
        }
    }
    # set all to NA for the case of errors
    avgADF <- rADF <- avgPP <- rPP <- avgKPSS <- rKPSS <- avgBDS <- rBDS <-
      avgKS <- rKS <- ad <- ww <- NA
    # do Anderson-Darling for same distribution function
    try( suppressWarnings( ad <- kSamples::ad.test( series.list,
                                                    method = ad.method )$ad[ 1, 3 ] ),
                          silent = TRUE )
    # do Wald-Wolfowitz for ergodicity (Grazzini 2012)
    try( suppressWarnings( ww <- ww.test( series.list,
                                          floor( length( series.list[[ 1 ]][ ! is.na( series.list[[ 1 ]] ) ] ) /
                                                   nSize + 1 ) )$p.value ),
         silent = TRUE )

    if( nADF > 0 ) {
      avgADF <- sumADF / nADF
      rADF <- rejADF / nADF
    }
    if( nPP > 0 ) {
      avgPP <- sumPP / nPP
      rPP <- rejPP / nPP
    }
    if( nKPSS > 0 ) {
      avgKPSS <- sumKPSS / nKPSS
      rKPSS <- rejKPSS / nKPSS
    }
    if( nBDS > 0 ) {
      avgBDS <- sumBDS / nBDS
      rBDS <- rejBDS / nBDS
    }
    if( nKS > 0 ) {
      avgKS <- sumKS / nKS
      rKS <- rejKS / nKS
    }
    statErgo[ j, ] <- c( avgADF, rADF, avgPP, rPP, avgKPSS, rKPSS,
                         avgBDS, rBDS, avgKS, rKS, ad, ww )
  }

  return( format( round.df( statErgo, digits = digits ), nsmall = digits ) )
}


# ==== Calculate unimodality & symmetry estatistics table ====

symmet.test.lsd <- function( data, vars = names( data[ 1, , 1 ] ),
                             start.period = 0, signif = 0.05, digits = 2,
                             sym.boot = FALSE ) {

  stats <- c( "avg Hdip", "rej Hdip", "avg CM", "rej CM",
              "avg M", "rej M", "avg MGG", "rej MGG" )
  statSymmet <- data.frame( matrix( nrow = length( vars ), ncol = length( stats ),
                                    dimnames = list( vars, stats ) ) )
  nTsteps <- length( data[ , 1, 1 ] )
  nSize <- length( data[ 1, 1, ] )
  if( start.period >= nTsteps )
    stop( "Invalid start period" )

  # do unimodality and symmetry tests individually for each variable
  # counting the number of non-rejecting cases
  for( j in vars ) {
    # extract each series as a matrix where time in is the rows
    # and MC instances in the columns
    series <- data[ start.period : nTsteps, j, ]
    nHdip <- nCM <- nM <- nMGG <-
      sumHdip <- sumCM <- sumM <- sumMGG <-
      rejHdip <- rejCM <- rejM <- rejMGG <- 0
    # test all instances
    # Hdip H0: unimodal, HVllh H0: bimodal, CM/M/MGG H0: symmetric
    for( n in 1 : nSize ) {
      Hdip <- CM <- M <- MGG <- NA
      try( Hdip <- diptest::dip.test( series[ , n ] )$p.value, silent = TRUE )
      if( is.finite( Hdip ) ) {
        nHdip <- nHdip + 1
        sumHdip <- sumHdip + Hdip
        if( Hdip < signif ) rejHdip <- rejHdip + 1
      }
      try( CM <- lawstat::symmetry.test( series[ , n ], option = "CM",
                                         boot = sym.boot )$p.value, silent = TRUE )
      if( is.finite( CM ) ) {
        nCM <- nCM + 1
        sumCM <- sumCM + CM
        if( CM < signif ) rejCM <- rejCM + 1
      }
      try( M <- lawstat::symmetry.test( series[ , n ], option = "M",
                                        boot = sym.boot )$p.value, silent = TRUE )
      if( is.finite( M ) ) {
        nM <- nM + 1
        sumM <- sumM + M
        if( M < signif ) rejM <- rejM + 1
      }
      try( MGG <- lawstat::symmetry.test( series[ , n ], option = "MGG",
                                          boot = sym.boot )$p.value, silent = TRUE )
      if( is.finite( MGG ) ) {
        nMGG <- nMGG + 1
        sumMGG <- sumMGG + MGG
        if( MGG < signif ) rejMGG <- rejMGG + 1
      }
    }
    # set all to NA for the case of errors
    avgHdip <- rHdip <- avgCM <- rCM <- avgM <- rM <- avgMGG <- rMGG <- NA

    if( nHdip > 0 ) {
      avgHdip <- sumHdip / nHdip
      rHdip <- rejHdip / nHdip
    }
    if( nCM > 0 ) {
      avgCM <- sumCM / nCM
      rCM <- rejCM / nCM
    }
    if( nM > 0 ) {
      avgM <- sumM / nM
      rM <- rejM / nM
    }
    if( nMGG > 0 ) {
      avgMGG <- sumMGG / nMGG
      rMGG <- rejMGG / nMGG
    }

    statSymmet[ j, ] <- c( avgHdip, rHdip, avgCM, rCM, avgM, rM, avgMGG, rMGG )
  }

  return( format( round.df( statSymmet, digits = digits ), nsmall = digits ) )
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

# populates lists with the values for all factors to be ploted

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


# ==== Do sensitivity analysis of a fitted model ====

sobol.decomposition.lsd <- function( data, model = NULL, krig.sa = FALSE, sa.samp = 1000 ) {

  if( is.null( model ) )
    out <- data.sensitivity( data )
  else
    if( class( model ) == "kriging-model" )
      out <- kriging.sensitivity( data, model, krig.sa = krig.sa, sa.samp = sa.samp )
    else
      out <- polynomial.sensitivity( data, model, sa.samp = sa.samp )

  return( out )
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


# ==== Optimize meta-model (minimize or maximize) ====

model.optim.lsd <- function( model, data = NULL, lower.domain = NULL,
                             upper.domain = NULL, starting.values = NULL,
                             minimize = TRUE, pop.size = 1000, max.generations = 30,
                             wait.generations = 10, precision = 1e-5,
                             nnodes = 1 ) {

  if( ! is.null( data ) && class( data ) == "lsd-doe" ) {
	if( is.null( lower.domain ) )
	  lower.domain <- data$facLimLo
	if( is.null( upper.domain ) )
	  upper.domain <- data$facLimUp
	if( is.null( starting.values ) )
	  starting.values <- data$facDef
  }

  # check variables allowed to variate (lower != upper)
  varName <- varLo <- varUp <- varStart <- vector( )
  j <- 0
  for( i in 1 : length( lower.domain ) )
    if( lower.domain[ i ] < upper.domain[ i ] ) {
      j <- j + 1
      varName[ j ] <- names( lower.domain )[ i ]
      varLo[ j ] <- lower.domain[ i ]
      varUp[ j ] <- upper.domain[ i ]
      if( ! is.null( starting.values ) )
        varStart[ j ] <- starting.values[ i ]
    }
  names( varLo ) <- varName
  names( varUp ) <- varName
  if( ! is.null( starting.values ) )
    names( varStart ) <- varName
  else
    varStart <- NULL

  # function to be called by the optimizer
  f <- function( x ) {

    if( ! is.null( starting.values ) )
      X <- starting.values
    else
      X <- lower.domain

    for( ii in 1 : length( lower.domain ) )
      for( jj in 1 : length( x ) )
        if( names( lower.domain )[ ii ] == varName[ jj ] )
          X[ ii ] <- x[ jj ]

        X <- as.data.frame( t( X ) )
        return ( as.numeric( model.pred.lsd( X, model )$mean ) )
  }

  # enable multiprocessing (not working in Windows)
  if( nnodes == 0 )
    nnodes <- parallel::detectCores( ) / 2

  if( nnodes > 1 ) {
    if( .Platform$OS.type == "unix" ) {
      cluster <- parallel::makeForkCluster( min( nnodes,
                                                 parallel::detectCores( ) ) )
      parallel::setDefaultCluster( cluster )
    } else {
      #cluster <- parallel::makePSOCKcluster( min( nnodes,
      #                                            parallel::detectCores( ) ) )
      #parallel::setDefaultCluster( cluster )
      cluster <- FALSE
    }
  } else {
    cluster <- FALSE
  }

  xStar <- try( rgenoud::genoud( fn = f, nvars = j, max = ! minimize,
                                 starting.values = varStart,
                                 Domains = cbind( varLo, varUp ),
                                 boundary.enforcement = 2, pop.size = pop.size,
                                 max.generations = max.generations,
                                 wait.generations =  wait.generations,
                                 solution.tolerance = precision,
                                 gradient.check = FALSE,
                                 P1 = 39, P2 = 40, P3 = 40, P4 = 40,
                                 P5 = 40, P6 = 40, P7 = 40, P8 = 40,
                                 cluster = cluster,
                                 print.level = 0 ),
                silent = TRUE )

  # release multiprocessing cluster
  if( cluster != FALSE )
    try( parallel::stopCluster( cluster ), silent = TRUE )

  if( class( xStar ) == "try-error" ) return( t( rep( NA, length( lower.domain ) ) ) )

  if( ! is.null( starting.values ) )
    opt <- starting.values
  else
    opt <- lower.domain

  # rebuilt the full parameter vector with the optimized values
  for( i in 1 : length( lower.domain ) )
    for( j in 1 : length( xStar$par ) )
      if( names( lower.domain )[ i ] == varName[ j ] )
        opt[ i ] <- xStar$par[ j ]
  opt <- as.data.frame( t( opt ) )
  rownames( opt ) <- c( "Value" )

  return( opt )
}


# ==== Compute meta-model minimum and maximum points for up to 2 factors ====

model.limits.lsd <- function( data, model, sa = NULL,
                              factor1 = 1, factor2 = 2, factor3 = 3,
                              pop.size = 1000, max.generations = 30,
                              wait.generations = 10, precision = 1e-5,
                              nnodes = 1 ) {

  if( ! is.null( sa ) && ( class( sa ) == "kriging-sa" || class( sa ) == "polynomial-sa" ) ) {
    factor1 <- sa$topEffect[ 1 ]
    factor2 <- sa$topEffect[ 2 ]
    factor3 <- sa$topEffect[ 3 ]
  }

  vars <- names( data$facDef )

  # first calculate max & min for each factor
  min1 <- max1 <- list( )
  minResp <- maxResp <- labels <- vector( )
  j <- 1
  for( i in c( factor1, factor2, factor3 ) ) {
    lo <- up <- data$facDef
    lo[ i ] <- data$facLimLo[ i ]
    up[ i ] <- data$facLimUp[ i ]

    min1[[ j ]] <- model.optim.lsd( model, lower.domain = lo, upper.domain = up,
                                    starting.values = data$facDef,
                                    minimize = TRUE, pop.size = pop.size,
                                    max.generations = max.generations,
                                    wait.generations = wait.generations,
                                    nnodes = nnodes )
    if( ! is.na( min1[[ j ]][ 1 ] ) )
      minResp[ j ] <- model.pred.lsd( min1[[ j ]], model )$mean
    else
      minResp[ j ] <- NA
    labels[ ( 2 * j ) - 1 ] <- paste( vars[ i ], "min" )

    max1[[ j ]] <- model.optim.lsd( model, lower.domain = lo, upper.domain = up,
                                    starting.values = data$facDef,
                                    minimize = FALSE, pop.size = pop.size,
                                    max.generations = max.generations,
                                    wait.generations = wait.generations,
                                    nnodes = nnodes )
    if( ! is.na( max1[[ j ]][ 1 ] ) )
      maxResp[ j ] <- model.pred.lsd( max1[[ j ]], model )$mean
    else
      maxResp[ j ] <- NA
    labels[ 2 * j ] <- paste( vars[ i ], "max" )

    j <- j + 1
  }

  # then calculate max & min for the first two factors jointly
  lo <- up <- data$facDef
  for( i in c( factor1, factor2 ) ) {
    lo[ i ] <- data$facLimLo[ i ]
    up[ i ] <- data$facLimUp[ i ]
  }

  min2 <- model.optim.lsd( model, lower.domain = lo, upper.domain = up,
                           starting.values = data$facDef,
                           minimize = TRUE, pop.size = pop.size,
                           max.generations = max.generations,
                           wait.generations = wait.generations,
                           nnodes = nnodes )
  if( ! is.na( min2[ 1 ] ) )
    min2Resp <- model.pred.lsd( min2, model )$mean
  else
    min2Resp <- NA

  max2 <- model.optim.lsd( model, lower.domain = lo, upper.domain = up,
                           starting.values = data$facDef,
                           minimize = FALSE, pop.size = pop.size,
                           max.generations = max.generations,
                           wait.generations = wait.generations,
                           nnodes = nnodes )
  if( ! is.na( max2[ 1 ] ) )
    max2Resp <- model.pred.lsd( max2, model )$mean
  else
    max2Resp <- NA
  labels[ ( 2 * j ) - 1 ] <- paste0( vars[ factor1 ], "-", vars[ factor2 ], " min" )
  labels[ 2 * j ] <- paste0( vars[ factor1 ], "-", vars[ factor2 ], " max" )

  max.min <- as.data.frame( cbind( t( min1[[ 1 ]] ), t( max1[[ 1 ]] ),
                                   t( min1[[ 2 ]] ), t( max1[[ 2 ]] ),
                                   t( min1[[ 3 ]] ), t( max1[[ 3 ]] ),
                                   t( min2 ), t( max2 ) ) )
  max.min <- rbind( max.min, c( minResp[ 1 ], maxResp[ 1 ], minResp[ 2 ],
                                maxResp[ 2 ], minResp[ 3 ], maxResp[ 3 ],
                                min2Resp, max2Resp ) )
  dimnames( max.min ) <- list( append( vars, "response" ), labels )
  return( max.min )
}


# ==== Calculate polynomial regression goodness-of-fit ====

# Compute the root mean square error for a polynomial model m
rmse.poly <- function( m, y, x ) {
  if( length( y ) != nrow( x ) )
    stop( "Vectors must have the same size" )
  pred <- stats::predict( m, newdata = x, type = "response", se.fit = FALSE )
  return( sqrt( mean( ( y - pred ) ^ 2, na.rm = TRUE ) ) )
}

# Compute the mean absolute error for a polynomial model m
mae.poly <- function( m, y, x ) {
  if( length( y ) != nrow( x ) )
    stop( "Vectors must have the same size" )
  pred <- stats::predict( m, newdata = x, type = "response", se.fit = FALSE )
  return( mean( abs( y - pred ), na.rm = TRUE ) )
}

# Compute the relative maximal absolute criterion for a polynomial model m
rma.poly <- function( m, y, x ) {
  if( length( y ) != nrow( x ) )
    stop( "Vectors must have the same size" )
  pred <- stats::predict( m, newdata = x, type = "response", se.fit = FALSE )
  return( max( abs( y - pred ) / stats::sd( y, na.rm = TRUE ),
               na.rm = TRUE ) )
}


# ==== Calculate Kriging goodness-of-fit ====

# Compute the Q2 coefficient for a kriging model m without noise
Q2.kriging <- function ( m, type = "UK", trend.reestim = TRUE ) {
  error <- ( m@y - DiceKriging::leaveOneOut.km( m, type = type,
                                                trend.reestim = trend.reestim )$mean ) ^ 2
  deviation <- ( m@y - mean( m@y ) ) ^ 2
  return( min( max( 1 - sum( error ) / sum( deviation ), 0 ), 1 ) )
}

# Compute the root mean square error for a kriging model m
rmse.kriging <- function( m, y, x, type = "UK" ) {
  if( length( y ) != nrow( x ) )
    stop( "Vectors must have the same size" )
  pred <- DiceKriging::predict( m, newdata = x, type = type )$mean
  return( sqrt( mean( ( y - pred ) ^ 2, na.rm = TRUE ) ) )
}

# Compute the mean absolute error for a kriging model m
mae.kriging <- function( m, y, x, type = "UK" ) {
  if( length( y ) != nrow( x ) )
    stop( "Vectors must have the same size" )
  pred <- DiceKriging::predict( m, newdata = x, type = type )$mean
  return( mean( abs( y - pred ), na.rm = TRUE ) )
}

# Compute the relative maximal absolute criterion for a kriging model m
rma.kriging <- function( m, y, x, type = "UK" ) {
  if( length( y ) != nrow( x ) )
    stop( "Vectors must have the same size" )
  pred <- DiceKriging::predict( m, newdata = x, type = type )$mean
  return( max( abs( y - pred ) / stats::sd( y, na.rm = TRUE ), na.rm = TRUE ) )
}


# ==== Fit Polynomial meta-model to response surface ====

fit.poly <- function( response, doe, resp.noise = NULL,
                      order = 1, interaction = 0 ) {
  maxOrder <- 2
  if( order > maxOrder || interaction > 2 )
    stop( "Invalid polynomial specification" )

  y <- response
  formulas <- matrix( list( y ~ ., y ~ . * ., NA, NA ), ncol = 2, byrow = TRUE,
                      dimnames = list( c( "Order 1", "Order 2" ),
                                       c( "No interact.", "Interacts." ) ) )

  regr <- colnames( doe )                     # regressor list
  inter <- ""
  for( j in 1 : ( ncol( doe ) - 1 ) )                 # create interaction terms
    for( k in ( j + 1 ) : ncol( doe ) ) {
      inter <- paste( inter, paste0( "I(", regr[ j ], "*", regr[ k ] , ")" ) )
      if( j < ( ncol( doe ) - 1 ) )
        inter <- paste( inter, "+" )
    }

  form <- "y ~"
  for( i in 1 : maxOrder ) {                  # create higher order terms
    for( j in 1 : ncol( doe ) ) {
      if( i == 1 ) {
        form <- paste( form, regr[ j ] )
      } else
        form <- paste( form, paste0( "I(", regr[ j ], "^", i , ")" ) )
      if( j < ncol( doe ) )
        form <- paste( form, "+" )
    }
    formulas[[ i, 1 ]] <- stats::as.formula( form )
    formulas[[ i, 2 ]] <- stats::as.formula( paste( form, "+", inter ) )
    form <- paste( form, "+" )
  }

  if( is.null( resp.noise ) || min( abs( resp.noise ), na.rm = TRUE ) == 0 ) {
    weigths <- rep( 1, nrow( doe ) )
  } else {
    weigths <- 1 / resp.noise
  }
  # fit bot using the normal and standardized vars
  fit <- stats::lm( formula = formulas[[ order, interaction + 1 ]],
                    data = doe, weights = weigths, na.action = stats::na.exclude )
  fit.std <- stats::lm( formula = formulas[[ order, interaction + 1 ]],
                        data = as.data.frame( scale( doe ) ), weights = weigths,
                        na.action = stats::na.exclude )

  # calculate variance inflation factors
  fit.uwgth <- stats::lm( formula = formulas[[ order, interaction + 1 ]],
                          data = doe, na.action = stats::na.exclude )
  inflat <- try( suppressWarnings( car::vif( fit.uwgth ) ), silent = TRUE )

  fit <- list( model = fit, R2 = summary( fit )$adj.r.squared,
               vif = inflat, f = summary( fit )$fstatistic,
               coeff = data.frame( summary( fit )$coefficients ),
               std.coeff = data.frame( summary( fit.std )$coefficients ) )
  class( fit ) <- "polynomial-fit"

  return( fit )
}


# ==== Fit Kriging meta-model to response surface ====

fit.kriging <- function( response, doe, resp.noise = NULL, trend.func = ~1,
                         cov.func = "matern5_2", trials = 1 ) {

  # Multiplicative noise scaling factor for variance/noise
  scaleFactor <- 0.5

  # Cross validation - don't use noise info because of Q2 doesn't support it
  fit <- DiceKriging::km( design = doe, response = response, formula = trend.func,
                          covtype = cov.func, control = list( trace = FALSE, print.level = 0 ) )
  Q2 <- Q2.kriging( fit )

  # External validation - reestimate the model using noise information
  # Because of a bug in km under certain noise configurations, the code
  # below downscale the variance vector up to 5 times to try to fix the problem
  trial <- 0                  # max trial control (when optimizer crashes)
  ok <- FALSE
  while( ! is.null( resp.noise ) && ! ok && trial < trials ) {
    ok <- TRUE
    tryCatch( fit <- DiceKriging::km( design = doe, response = response,
                                      formula = trend.func, covtype = cov.func,
                                      noise.var = resp.noise * ( scaleFactor ^ trial ),
                                      control = list( trace = FALSE, print.level = 0 ) ),
              error = function( ex ) {
                warning( "Model search: Problem in function 'km', trying to scale down noise...",
                         call. = FALSE )
                trial <<- trial + 1
                ok <<- FALSE
              } )
  }
  if( trial == trials )
    stop( "Can't fit a model using function 'km', try removing outliers" )

  fit <- list( model = fit, Q2 = Q2 )
  class( fit ) <- "kriging-fit"

  return( fit )
}


# ==== Read LSD parameter limits file and check consistency and number order ====

read.sens <- function( folder = NULL, baseName = NULL, fileName = NULL ) {

  if( is.null( fileName ) && is.null( baseName ) )
    stop( "LSD sensitivity file name (or parts) missing" )

  if( is.null( fileName ) )
    file <- paste0( baseName, ".sa" )
  else
    file <- fileName

  if( ! is.null( folder ) && file.exists( folder ) )
    dir <- normalizePath( folder, winslash = "/", mustWork = TRUE )
  else
    dir <- getwd( )

  par <- dirname( dir )

  if( file.exists( paste0( dir, "/", file ) ) ) {
    file <- paste0( dir, "/", file )
  } else {
    if( file.exists( paste0( dir, "/", file, "n" ) ) ) {    # accept .san extension (CRAN bug)
      file <- paste0( dir, "/", file, "n" )
    } else {
      if( file.exists( paste0( par, "/", file ) ) ) {
        file <- paste0( par, "/", file )
      } else {
        if( file.exists( paste0( par, "/", file, "n" ) ) ) {
          file <- paste0( par, "/", file, "n" )
        } else {
          stop( "LSD sensitivity file does not exist" )
        }
      }
    }
  }

  limits <- utils::read.table( file, stringsAsFactors = FALSE )
  limits <- limits[ -2 : -3 ]
  if( ! is.numeric( limits[ 1, 2 ] ) )  # handle newer LSD versions that bring an extra column
    limits <- limits[ -2 ]
  if( length( limits[ 1, ] ) > 3 ) {
    warning( "Too many (>2) sensitivity values for a single parameter, using the first two only!",
             call. = FALSE )
    limits <- limits[ -4 : - length( limits[ 1, ] ) ]
  }

  dimnames( limits )[[ 2 ]] <- c( "Par", "Min", "Max" )
  for( i in 1 : length( limits[ , 1 ] ) )
    if( limits[ i, "Min" ] > limits[ i, "Max" ] ) {
      temp <- limits[ "Min", i ]
      limits[ "Min", i ] <- limits[ "Max", i ]
      limits[ "Max", i ] <- temp
    }

  return( limits )
}


# ==== Read LSD default parameter configuration from base .lsd file ====

read.config <- function( folder = NULL, baseName = NULL, fileName = NULL ) {

  if( is.null( fileName ) && is.null( baseName ) )
    stop( "LSD configuration file name missing" )

  if( is.null( fileName ) )
    file <- paste0( baseName, ".lsd" )
  else
    file <- fileName

  if( ! is.null( folder ) && file.exists( folder ) )
    dir <- normalizePath( folder, winslash = "/", mustWork = TRUE )
  else
    dir <- getwd( )

  par <- dirname( dir )

  if( file.exists( paste0( dir, "/", file ) ) ) {
    file <- paste0( dir, "/", file )
  } else {
    if( file.exists( paste0( par, "/", file ) ) ) {
      file <- paste0( par, "/", file )
    } else {
      stop( "LSD configuration file does not exist" )
    }
  }

  lsd <- readLines( file )
  config <- data.frame( stringsAsFactors = FALSE )
  i <- 1
  while( lsd[ i ] != "DATA" )
    i <- i + 1                # jump lines until DATA section

  while( lsd[ i ] != "DESCRIPTION" ) {
    tok <- unlist( strsplit( lsd[ i ], split = " " ) )
    if( length( tok ) > 0 && ( tok[ 1 ] == "Param:" || tok[ 1 ] == "Var:" ) ) {
      endtok <- unlist( strsplit( tok[ length( tok ) ], split = "\t" ) )
      if( length( endtok ) > 1 ) {    # there is a value
        param <- as.data.frame( list( tok[2], as.numeric( endtok[ 2 ] ) ),
                                stringsAsFactors = FALSE )
        colnames( param ) <- c( "Par", "Val" )
        config <- rbind( config, param )
      }
    }
    i <- i + 1                # jump lines until DESCRIPTION section
  }

  return( config )
}


# ==== Get LSD Design of Experiments (DoE) files names without the .csv extension ====

files.doe <- function( folder, baseName ) {

  doeFiles <- LSDinterface::list.files.lsd( path = folder, conf.name = baseName,
                                            sensitivity = TRUE, type = "csv",
                                            compressed = FALSE )

  if( length( doeFiles ) < 1 )
    stop( "Valid DoE .csv file(s) required" )

  folder <- dirname( doeFiles[ 1 ] )

  for( i in 1 : length( doeFiles ) )
    doeFiles[ i ] <- sub( ".csv$", "", basename( doeFiles[ i ] ), ignore.case = TRUE )

  return( list( path = folder, files = doeFiles ) )
}


# ==== Read LSD Design of Experiments (DoE) size ====

size.doe <- function( doeFile ) {

  # Get basename and remove extension if present
  baseName <- sub( ".csv$", "", basename( doeFile ), ignore.case = TRUE )

  # First file must be the a DoE (baseName_xx_yy)
  split <- strsplit( baseName, "_" )[[ 1 ]]

  # Check invalid format
  if( length( split ) < 3 ||
      is.na( as.integer( split[ length( split ) ] ) ) ||
      is.na( as.integer( split[ length( split ) - 1 ] ) ) )
    stop( "Invalid DoE .csv file naming/numbering (must be baseName_XX_YY)" )

  doe <- c( as.integer( split[ length( split ) - 1 ] ),
            as.integer( split[ length( split ) ] ) -
              as.integer( split[ length( split ) - 1 ] ) + 1 )
  names( doe ) <- c( "ini", "n" )
  if( doe[ "n" ] < 1 )
    stop( "Invalid DoE .csv file numbering (must have at least 1 sampling point)" )

  return( doe )
}

# ==== Create DoE response file ====

write.response <- function( folder, baseName, iniExp = 1, nExp = 1, outVar = "",
                            pool = TRUE, iniDrop = 0, nKeep = -1, na.rm = FALSE,
                            conf = 0.95, saveVars = c(  ), addVars = c(  ),
                            eval.vars = NULL, eval.run = NULL, rm.temp = TRUE,
                            nnodes = 1, quietly = TRUE ) {

  # evaluate new variables (not in LSD files) names
  nVarNew <- length( addVars )           # number of new variables to add
  newNameVar <- append( LSDinterface::name.nice.lsd( saveVars ), addVars ) # new label set
  nVar <- length( newNameVar )          # total number of variables

  if( nVar == 0 && nVarNew == 0 )
    stop( "No variable to be bept in the data set, at least one required" )

  # check if files are in a subfolder
  myFiles <- LSDinterface::list.files.lsd( path = folder,
                                           conf.name = paste0( baseName, "_", iniExp ) )
  if( length( myFiles ) == 0 || ! file.exists( myFiles[ 1 ] ) )
    stop( "No data files  (baseName_XX_YY.res[.gz]) found on informed path" )

  folder <- dirname( myFiles[ 1 ] )

  # first check if extraction was interrupted and continue with partial files if appropriate
  tempFile <- paste0( folder, "/", baseName, "_", iniExp,
                      "_", iniExp + nExp - 1, ".RData" )
  if( ! rm.temp && file.exists( tempFile ) )
    tempDate <- file.mtime( tempFile )
  else
    tempDate <- 0

  # test if data files exit and are newer
  dataDate <- 0
  for( k in 1 : nExp ) {
    myFiles <- LSDinterface::list.files.lsd( path = folder,
                                             conf.name = paste0( baseName, "_",
                                                                 iniExp + k - 1 ) )
    if( tempDate == 0 && length( myFiles ) < 2 )
      stop( "Not enough data files (baseName_XX_YY.res[.gz]) found" )

    # get newest date
    for( file in myFiles ) {
      date <- file.mtime( file )
      if( date > dataDate )
        dataDate <- date
    }
  }

  # Continue with temporary file if appropriate
  noTemp <- TRUE
  if( tempDate > dataDate ) {
    temp <- new.env( )
    load( tempFile, envir = temp )

    if( all( temp$newNameVar == newNameVar ) ) {
      if( ! quietly )
        cat( "Previously processed data found, not reading data files...\n\n" )

      noTemp <- FALSE
      load( tempFile )
    }

    rm( temp )
  }

  # if no temporary data, start from zero
  if( noTemp ) {

    if( dataDate == 0 )
      stop( "No data files (baseName_XX_YY.res[.gz]) found" )

    for( k in 1 : nExp ) {

      if( ! quietly )
        cat( "\nSample #", iniExp + k - 1, "\n" )

      # ---- Read data files ----

      # Get file names
      myFiles <- LSDinterface::list.files.lsd( path = folder,
                                               conf.name = paste0( baseName, "_",
                                                                   iniExp + k - 1 ) )
      if( ! quietly )
        cat( "\nData files: ", myFiles, "\n\n" )

      # Determine the DoE sample size (repetitions on the same DoE point)
      nSize  <- length( myFiles )

      # Get data set details from first file
      dimData <- LSDinterface::info.dimensions.lsd( myFiles[ 1 ] )
      nTsteps <- dimData$tSteps
      origNvar <- dimData$nVars

      if( ! quietly ) {
        cat( "Number of MC runs: ", nSize, "\n" )
        cat( "Number of periods: ", nTsteps, "\n" )
      }

      nTsteps <- nTsteps - iniDrop
      if( nKeep != -1 )
        nTsteps <- min( nKeep, nTsteps )

      if( ! quietly ) {
        cat( "Number of used periods: ", nTsteps, "\n" )
        cat( "Number of variable instances: ", origNvar, "\n\n" )
        cat( "Reading data from files...\n" )
      }

      if( pool ) {

        # Create simpler array, pooling all non-NA values from a single run
        if( k == 1 )                  # do just once
          poolData <- array( list( ), dim = c( nExp, nVar, nSize ),
                             dimnames = list( NULL, newNameVar, NULL ) )
        nSampl <- 0

        # Read data one file at a time to restrict memory usage
        for( j in 1 : nSize ) {

          dataSet <- LSDinterface::read.raw.lsd( myFiles[ j ], nrows = nKeep,
                                                 skip = iniDrop )

          if( nVarNew != 0 ) {
            # Increase array size to allow for new variables
            oldNameVar <- colnames( dataSet )
            dataSet <- abind::abind( dataSet, array( as.numeric(NA),
                                                     dim = c( nTsteps, nVarNew ) ),
                                     along = 2, use.first.dimnames = TRUE )
            colnames( dataSet ) <- append( LSDinterface::name.var.lsd( oldNameVar ),
                                           addVars )
          } else {
            colnames( dataSet ) <- LSDinterface::name.var.lsd( colnames( dataSet ) )
          }

          # Call function to fill new variables with data or reevaluate old ones
          if( ! is.null( eval.vars ) )
            dataSet <- eval.vars( dataSet, newNameVar )

          # Extract the requested variables (columns)
          for( i in 1 : nVar ) {
            if( ! make.names( newNameVar[ i ] ) %in% colnames( dataSet ) )
              stop( "Invalid variable to be saved (not in LSD data)" )

            x <- dataSet[ , make.names( newNameVar[ i ] ) ]
            if( na.rm ) {
              poolData[[ k, i, j ]] <- x[ ! is.na( x ) ]  # remove NAs
            } else
              poolData[[ k, i, j ]] <- x

            nSampl <- nSampl + length( poolData[[ k, i, j ]] )
          }
        }

        nInsts <- 1                     # single instanced after pooling

      } else {

        if( length( saveVars ) == 0 )
          saveVars <- NULL

        # Read data from text files and format it as 4D array with labels
        dataSet <- LSDinterface::read.4d.lsd( myFiles, col.names = saveVars,
                                              nrows = nKeep, skip = iniDrop,
                                              nnodes = nnodes )
        nInsts <- dim( dataSet )[ 3 ]         # total number of instances

        # ------ Add new variables to data set ------

        # Increase array size to allow for new variables
        dataSet <- abind::abind( dataSet, array( as.numeric(NA),
                                                 dim = c( nTsteps, nVarNew,
                                                          nInsts, nSize ) ),
                                 along = 2, use.first.dimnames = TRUE )
        dimnames( dataSet )[[ 2 ]] <- newNameVar

        # Call function to fill new variables with data or reevaluate old ones
        if( ! is.null( eval.vars ) )
          dataSet <- eval.vars( dataSet, newNameVar )

        # ---- Reorganize and save data ----

        # Create simpler array, pooling all non-NA values from a single run
        if( k == 1 )                    # do just once
          poolData <- array( list( ), dim= c( nExp, nVar, nSize ),
                             dimnames = list( NULL, newNameVar, NULL ) )
        nSampl<- 0

        for( j in 1 : nSize )
          for( i in 1 : nVar ){
            x <- as.vector( dataSet[ , newNameVar[ i ], , j ] )
            if( na.rm ) {
              poolData[[ k, i, j ]] <- x[ ! is.na( x ) ]  # remove NAs
            } else
              poolData[[ k, i, j ]] <- x

            nSampl <- nSampl + length( poolData[[ k, i, j ]] )
          }
      }

      if( ! quietly ) {
        cat( "\nNumber of variables selected: ", nVar, "\n" )
        cat( "Number of pooled samples: ", nSampl, "\n\n" )
      }

      # Clean temp variables
      rm( dataSet, x )
    }

    # Save data list to disk
    save( nExp, nVar, nSize, nInsts, newNameVar, poolData,
          compress = TRUE, file = tempFile )
  }

  # ---- Process data in each experiment (nSize points) ----

  if( length( outVar ) == 0 ) {         # no output var?
    outVar <- newNameVar[ 1 ]           # use first var
    varIdx <- 1
  } else
    if( outVar %in% newNameVar ) {
      varIdx <- match( outVar, newNameVar )
    } else
      stop( "Invalid output variable selected" )

  # Process the DoE experiments
  respAvg <- respVar <- vector( "numeric" )
  tobs <- tdiscards <- 0

  for( k in 1 : nExp ) {

    # Call function to evaluate each experiment
    if( ! is.null( eval.run ) ) {

      resp <- eval.run( poolData, run = k, varIdx = varIdx, conf = conf  )

    } else {                            # default processing - just calculate
                                        # the average of all runs and the MC SD
      mAc <- vAc <- obs <- 0
      for( i in 1 : nSize ) {
        data <- poolData[[ k, varIdx, i ]]
        m <- mean( data, na.rm = TRUE )
        mAc <- mAc + m
        vAc <- vAc + m ^ 2
        obs <- obs + length( data[ ! is.na( data ) ] )
      }

      # avoid negative rounding errors
      if( is.finite( vAc ) && is.finite( mAc ) ) {
        mAc <- mAc / nSize
        if( vAc / nSize < mAc ^ 2 )
          sAc <- 0
        else
          sAc <- sqrt( vAc / nSize - mAc ^ 2 )
      } else {
        mAc <- vAc <- sAc <- NA
      }

      resp <- list( mAc, sAc, obs / nSize, 0 )
      rm( data )
    }

    respAvg[ k ] <- resp[[ 1 ]]
    respVar[ k ] <- resp[[ 2 ]]
    tobs <- tobs + resp[[ 3 ]]
    tdiscards <- tdiscards + resp[[ 4 ]]
  }

  # Write table to the disk as CSV file for Excel
  tresp <- as.data.frame( cbind( respAvg, respVar ) )
  colnames( tresp ) <- c( "Mean", "Variance" )
  respFile <- paste0( folder, "/", baseName, "_", iniExp, "_",
                      iniExp + nExp - 1, "_", outVar, ".csv" )
  utils::write.csv( tresp, respFile, row.names = FALSE )

  if( ! quietly ) {
    cat( "DoE response file saved:", respFile, "\n" )
    cat( "Doe points =", k, "\n" )
    cat( "Total observations =", tobs, "\n" )
    cat( "Discarded observations =", tdiscards, "\n\n" )
  }

  rm( poolData, resp, tresp )
  if( rm.temp )
    unlink( tempFile )
}


# ==== Read and pre-process DoE and response files ====

read.doe.lsd <- function( folder, baseName, outVar, does = 1, doeFile = NULL,
                          respFile = NULL, validFile = NULL, valRespFile = NULL,
                          confFile = NULL, limFile = NULL, iniDrop = 0,
                          nKeep = -1, saveVars = c(  ), addVars = c(  ),
                          eval.vars = NULL, eval.run = NULL, pool = TRUE,
                          na.rm = FALSE, rm.temp = TRUE, rm.outl = FALSE,
                          lim.outl = 10, nnodes = 1, quietly = TRUE ) {

  # ---- Process LSD result files ----

  # Get available DoE and response file names
  does.found <- files.doe( folder, baseName )
  files <- does.found$files
  folder <- does.found$path
  if( is.null( doeFile ) ) {
    if( length( files ) > does )
      warning( "Too many DoE (.csv) files, using first one(s) only",
               call. = FALSE )
    if( length( files ) < 1 )
      stop( "No valid DoE file" )

    doeFile <- paste0( folder, "/", files[ 1 ], ".csv" )
  }

  if( is.null( respFile ) ) {
    if( length( files ) < 1 )
      stop( "No valid response file" )
    respFile <- paste0( folder, "/", files[ 1 ], "_", outVar, ".csv" )
  }

  if( does > 1 ) {
    if( is.null( validFile ) ) {
      if( length( files ) < 2 )
        stop( "No valid DoE validation file" )
      validFile <- paste0( folder, "/", files[ 2 ], ".csv" )
    }
    if( is.null( valRespFile ) ) {
      if( length( files ) < 2 )
        stop( "No valid DoE validation response file" )
      valRespFile <- paste0( folder, "/", files[ 2 ], "_", outVar, ".csv" )
    }
  }

  # If response files don't exist, try to create them
  if( rm.temp || ! file.exists( respFile ) ) {
    write.response( folder, baseName, outVar = outVar,
                    iniDrop = iniDrop, nKeep = nKeep, rm.temp = rm.temp,
                    iniExp = size.doe( doeFile )[ 1 ], na.rm = na.rm,
                    nExp = size.doe( doeFile )[ 2 ], addVars = addVars,
                    pool = pool, eval.vars = eval.vars, eval.run = eval.run,
                    saveVars = saveVars, nnodes = nnodes, quietly = quietly )
  } else
    if( ! quietly )
      cat( paste0( "Using existing response file (", respFile, ")...\n\n" ) )

  if( does > 1 && ( rm.temp || ! file.exists( valRespFile ) ) ) {
    write.response( folder, baseName, outVar = outVar,
                    iniDrop = iniDrop, nKeep = nKeep, rm.temp = rm.temp,
                    iniExp = size.doe( validFile )[ 1 ],
                    nExp = size.doe( validFile )[ 2 ], na.rm = na.rm,
                    addVars = addVars, eval.vars = eval.vars,
                    pool = pool, eval.run = eval.run, saveVars = saveVars,
                    nnodes = nnodes, quietly = quietly )
  } else
    if( ! quietly && does > 1 )
      cat( paste0( "Using existing validation response file (", valRespFile, ")...\n\n" ) )

  # Read design of experiments definition & response
  doe <- utils::read.csv( doeFile )
  resp <- utils::read.csv( respFile )

  # Read external validation experiments definition & response
  if( does > 1 ) {
    valid <- utils::read.csv( validFile )
    valResp <- utils::read.csv( valRespFile )
  } else
    valid <- valResp <- NULL

  # Read LSD default parameter configuration from base .lsd file
  if( is.null( confFile ) ) {
    config <- read.config( folder = folder, baseName = baseName )
  } else
    config <- read.config( fileName = confFile )

  # Read LSD parameter limits file and join with default configuration
  if( is.null( limFile ) ) {
    limits <- read.sens( folder = folder, baseName = baseName )
  } else
    limits <- read.sens( fileName = limFile )

  limits$Def <- NA                          # add new column to param table
  for( i in 1 : nrow( limits ) ) {
    j <- which( config$Par == limits$Par[ i ], arr.ind = TRUE )
    if( length( j ) == 1 )                  # param/var is in limits df?
      limits$Def[ i ] <- config$Val[ j ]
  }

  # ---- Preprocess data ----

  # Create min/max/def lists for parameters in the same order as the DoE
  facLim <- list( )
  facLimLo <- facLimUp <- facDef <- vector( "numeric" )
  for( i in 1 : length( colnames( doe ) ) ) {
    j <- which( limits$Par == colnames( doe )[ i ], arr.ind = TRUE )
    if( j == 0 )
      stop( "Parameter not found in LSD sensitivity file" )
    facLim[[ i ]] <- list( min = limits$Min[ j ], max = limits$Max[ j ] )
    facLimLo[ i ] <- limits$Min[ j ]
    facLimUp[ i ] <- limits$Max[ j ]
    facDef[ i ] <- limits$Def[ j ]
  }
  names( facLim ) <- names( facLimLo ) <- names( facLimUp ) <- names( facDef ) <-
    colnames( doe )

  # Remove outliers, if appropriate
  if( rm.outl ) {
    clean <- remove.outliers( doe, resp, limit = lim.outl )
    doe <- clean[[ 1 ]]
    resp <- clean[[ 2 ]]

    if( does > 1 ) {
      clean <- remove.outliers( valid, valResp, limit = lim.outl )
      valid <- clean[[ 1 ]]
      valResp <- clean[[ 2 ]]
    }
  }

  doe <- list( doe = doe, resp = resp, valid = valid, valResp = valResp,
               facLim = facLim, facLimLo = facLimLo, facLimUp = facLimUp,
               facDef = facDef, saVarName = outVar )
  class( doe ) <- "lsd-doe"

  return( doe )
}


# ==== Perform sensitivity analysis directly over data ====

data.sensitivity <- function( data, tries = 5 ) {

  # ---- Sensitivity analysis using a B-spline smoothing interpolation model ----

  metamodel <- try( sensitivity::sobolSmthSpl( as.matrix( data$resp$Mean ), data$doe ),
                    silent = TRUE )

  # try a few times, as it usually succeeds...
  while( class( metamodel ) == "try-error" && tries > 0 ) {
    metamodel <- try( sensitivity::sobolSmthSpl( as.matrix( data$resp$Mean ), data$doe ),
                      silent = TRUE )
    tries <- tries - 1
    if( class( metamodel ) != "try-error" )
      break
  }

  if( class( metamodel ) == "try-error" )
    return( NULL )

  mainEffect <- function( x ) x$S[ , 1 ]

  # algorithm provide only the main effects, so distribute the indirect effects evenly (approx.)
  totalEffect <- ( 1 - sum( mainEffect( metamodel ) ) )
  sa <- cbind( mainEffect( metamodel ),
               mainEffect( metamodel ) * totalEffect / sum( mainEffect( metamodel ) ) )

  rownames( sa ) <- colnames( data$doe )
  colnames( sa ) <- c( "Direct effects", "Interactions" )

  max.index <- function( x, pos = 1 )
    as.integer( sapply( sort( x, index.return = TRUE ), `[`, length( x ) - pos + 1 )[ 2 ] )

  topEffect <- c( max.index( mainEffect( metamodel ), 1 ), max.index( mainEffect( metamodel ), 2 ),
                  max.index( mainEffect( metamodel ), 3 ) )

  cat( "Top parameters influencing response surface:\n" )
  cat( " First:", colnames( data$doe )[ topEffect[ 1 ] ], "\n" )
  cat( " Second:", colnames( data$doe )[ topEffect[ 2 ] ], "\n" )
  cat( " Third:", colnames( data$doe )[ topEffect[ 3 ] ], "\n\n" )

  sa <- list( metamodel = metamodel, sa = sa, topEffect = topEffect )
  class( sa ) <- "b-spline-sa"

  return( sa )
}


# ==== Kriging model selection ====

kriging.model.lsd <- function( data, ext.wgth = 0.5, trendModel = 0,
                               covModel = 0, digits = 4 ) {

  # The following code was adapted and expanded from Salle & Yildizoglu 2014
  #
  # x = doe is in data.frame format, and contains the DoE (n rows, k columns)
  # z = valid is in data.frame format, and contains the values of the factors at the
  # additional points (n rows, k columns)
  # y = resp$Mean is in data.frame format, and contains the values of the response
  # d at the n points of the DoE, averaged over the nSize replications (column is
  # named totDist, n rows)
  # sigma = resp$Variance is a column of n rows, with contains the variance of the
  # response d over the nSize replications of each n experiments.
  # w = valResp$Mean is in data.frame format, and contains the values of the
  # response at the additional points (averaged over the nSize replications ).
  #

  # ------ Best model estimation & selection ------

  # Check if external validation is available or use cross validation only
  if( is.null( data$valid ) ) onlyCross <- TRUE else onlyCross <- FALSE

  # Estimating some kriging model alternatives and corresponding Q2 for:
  trendNames <- c( "constant", "1st order poly." )
  trendTypes <- c( ~ 1, ~ . )
  covNames <- c( "Matern 5/2", "Matern 3/2", "Gaussian", "exponent.", "power exp." )
  covTypes <- c( "matern5_2", "matern3_2", "gauss", "exp", "powexp" )

  # Trials to scale variance to prevent optimizer crashes
  maxTrials <- 5

  Q2 <- rmse <- mae <- rma <- rank <- matrix( nrow = length( trendTypes ),
                                              ncol = length( covTypes ),
                                              dimnames = list( trendNames, covNames ) )
  models <- matrix( list( ), nrow = length( trendTypes ), ncol = length( covTypes ),
                    dimnames = list( trendNames, covNames ) )

  for( i in 1 : length( trendTypes ) )
    for( j in 1 : length( covTypes ) ) {
      km <- fit.kriging( data$resp$Mean, data$doe, resp.noise = data$resp$Variance,
                         trials = maxTrials, trend.func = trendTypes[[ i ]],
                         cov.func = covTypes[ j ] )
      models[[ i, j ]] <- km$model
      Q2[ i, j ] <- km$Q2

      if( ! onlyCross ) {
        rmse[ i, j ] <- rmse.kriging( km$model, data$valResp$Mean, data$valid )
        mae[ i, j ] <- mae.kriging( km$model, data$valResp$Mean, data$valid )
        rma[ i, j ] <- rma.kriging( km$model, data$valResp$Mean, data$valid )
      }
    }

  # select model with maximum Q2
  Q2[ is.nan( Q2 ) ] <- NA
  bestCross <- as.vector( which( Q2 == max( Q2, na.rm = TRUE ),
                                 arr.ind = TRUE )[ 1, ] )

  cat( "Cross validation of alternative models:\n" )
  cat( " Best trend model:", trendNames[ bestCross[ 1 ] ], "\n" )
  cat( " Best covariation model:", covNames[ bestCross[ 2 ] ], "\n\n" )

  if( ! onlyCross ) {
    # remove failed statistics
    rmse[ is.nan( rmse ) ] <- NA
    mae[ is.nan( mae ) ] <- NA
    rma[ is.nan( rma ) ] <- NA

    # select model with minimum measures
    bestRmse <- as.vector( which( rmse == min( rmse, na.rm = TRUE ),
                                  arr.ind = TRUE )[ 1, ] )
    bestMae <- as.vector( which( mae == min( mae, na.rm = TRUE ),
                                 arr.ind = TRUE )[ 1, ] )
    bestRma <- as.vector( which( rma == min( rma, na.rm = TRUE ),
                                 arr.ind = TRUE )[ 1, ] )

    cat( "External validation of alternative models:\n" )
    cat( " Best trend model (RMSE):", trendNames[ bestRmse[ 1 ] ], "\n" )
    cat( " Best covariation model (RMSE):", covNames[ bestRmse[ 2 ] ], "\n" )
    cat( " Best trend model (MAE):", trendNames[ bestMae[ 1 ] ], "\n" )
    cat( " Best covariation model (MAE):", covNames[ bestMae[ 2 ] ], "\n" )
    cat( " Best trend model (RMA):", trendNames[ bestRma[ 1 ] ], "\n" )
    cat( " Best covariation model (RMA):", covNames[ bestRma[ 2 ] ], "\n\n" )
  }

  # select model with best rank (lower is beter)
  for( i in 1 : length( trendTypes ) )
    for( j in 1 : length( covTypes ) ) {
      rank[ i, j ] <- ( 1 - ext.wgth ) * ( 1 - Q2[ i, j ] )
      if( ! onlyCross )
        rank[ i, j ] <- rank[ i, j ] +
          ext.wgth * rmse[ i, j ] / ( 3 * max( rmse, na.rm = TRUE ) ) +
          ext.wgth * mae[ i, j ] / ( 3 * max( mae, na.rm = TRUE ) ) +
          ext.wgth * rma[ i, j ] / ( 3 * max( rma, na.rm = TRUE ) )
    }

  # Check if use fixed or best models
  bestOvrall <- as.vector( which( rank == min( rank, na.rm = TRUE ),
                                  arr.ind = TRUE )[ 1, ] )
  if( trendModel == 0 )
    trendModel = bestOvrall[ 1 ]
  if( covModel == 0 )
    covModel = bestOvrall[ 2 ]

  m <- models[[ trendModel, covModel ]]
  Q2stat <- Q2[ trendModel, covModel ]

  # ---- Create tables ----

  if( onlyCross ) {
    table <- data.frame( Q2, row.names = NULL )
    rownames( table ) <- c( apply( cbind( rep( "Q2", length( trendTypes ) ), trendNames,
                                          rep( "trend", length( trendTypes ) ) ),
                                   1, paste, collapse = " " ) )
    rmseStat <- maeStat <- rmaStat <- valExtN <- NA
  } else {
    table <- data.frame( rbind( Q2, rmse, mae, rma, deparse.level = 0 ), row.names = NULL )
    rownames( table ) <- c( apply( cbind( rep( "Q2", length( trendTypes ) ), trendNames,
                                          rep( "trend", length( trendTypes ) ) ),
                                   1, paste, collapse = " " ),
                            apply( cbind( rep( "RMSE", length( trendTypes ) ), trendNames,
                                          rep( "trend", length( trendTypes ) ) ),
                                   1, paste, collapse = " " ),
                            apply( cbind( rep( "MAE", length( trendTypes ) ), trendNames,
                                          rep( "trend", length( trendTypes ) ) ),
                                   1, paste, collapse = " " ),
                            apply( cbind( rep( "RMA", length( trendTypes ) ), trendNames,
                                          rep( "trend", length( trendTypes ) ) ),
                                   1, paste, collapse = " " ) )
    rmseStat <- rmse[ trendModel, covModel ]
    maeStat <- mae[ trendModel, covModel ]
    rmaStat <- rma[ trendModel, covModel ]
    valExtN <- length( data$valResp$Mean )
  }
  colnames( table ) <- covNames

  fmt <- function( x )
    format( round( x, digits = digits ), nsmall = digits )

  coef.label <- c( "trend(intercept)", "trend(inclination)",
                   apply( cbind( rep( paste0( m@covariance@range.names, "(" ), m@covariance@d ),
                                 m@covariance@var.names, rep( ")", m@covariance@d ) ),
                          1, paste0, collapse = "" ) )

  fit.label <- c( "Trend specification", "Correlation function", "Cross-sample Q2",
                  "External RMSE", "External MAE", "External RMA",
                  "DoE samples", "External samples" )

  coefficients <- c( m@trend.coef[ 1 : 2 ], m@covariance@range.val )
  names( coefficients ) <- coef.label
  coef <- fmt( coefficients )

  fit <- c( trendNames[ trendModel ], covNames[ covModel ],
            fmt( c( Q2stat, rmseStat, maeStat, rmaStat ) ),
            format( c( m@n, valExtN ), nsmall = 0 ) )

  nCoef <- length( coef )
  nFit <- length( fit )
  nMove <- 0
  maxRows <- 20
  if( nCoef > maxRows ) {                     # too many for a single column?
    # rows to move to 2nd column
    nMove <- min( nCoef - maxRows, maxRows - ( nFit + 1 ) )
    nCoef <- nCoef - nMove
    nFit <- nFit+ nMove + 1

    # move rows
    fit.label <- append( coef.label[ ( maxRows + 1 ) : ( maxRows + nMove ) ],
                         append( c( "" ), fit.label ) )
    fit <- append( coef[ ( maxRows + 1 ) : ( maxRows + nMove ) ],
                   append( c( "" ), fit ) )
    coef.label <- coef.label[ - ( ( maxRows + 1 ) : ( maxRows + nMove ) ) ]
    coef <- coef[ - ( ( maxRows + 1 ) : ( maxRows + nMove ) ) ]
  }
  if( nCoef > nFit ) {
    fit <- append( fit, rep( "", nCoef - nFit ) )
    fit.label <- append( fit.label, rep( "", nCoef - nFit ) )
  }
  if( nCoef < nFit ) {
    coef <- append( coef, rep( "", nFit - nCoef ) )
    coef.label <- append( coef.label, rep( "", nFit - nCoef ) )
  }

  estimation <- cbind( coef, fit.label, fit )
  colnames( estimation ) <- c( "Coefficient", "Other information", "" )

  # ---- Standardized model estimation ----

  # Scale all factors to the range [0,1]
  Binf <- matrix( data$facLimLo, nrow = nrow( data$doe ),
                  ncol = length( data$facLimLo ), byrow = TRUE )
  Bsup <- matrix( data$facLimUp, nrow = nrow( data$doe ),
                  ncol = length( data$facLimUp ), byrow = TRUE )
  X.std <- ( data$doe - Binf ) / ( Bsup - Binf )

  m.std <- fit.kriging( data$resp$Mean, X.std, resp.noise = data$resp$Variance,
                        trials = maxTrials, trend.func = trendTypes[[ trendModel ]],
                        cov.func = covTypes[ covModel ] )$model

  coefficients.std <- c( m.std@trend.coef[ 1 : 2 ], m.std@covariance@range.val )
  names( coefficients.std ) <- names( coefficients )
  coef.std <- fmt( coefficients.std )

  nCoef.std <- length( coef.std )
  if( nMove > 0 ) {
    fit[ 1 : nMove ] <- coef.std[ ( maxRows + 1 ) : ( maxRows + nMove ) ]
    coef.std <- coef.std[ - ( ( maxRows + 1 ) : ( maxRows + nMove ) ) ]

  } else
    if( nCoef.std < nFit )
      coef.std <- append( coef.std, rep( "", nFit - nCoef ) )

  estimation.std <- cbind( coef.std, fit.label, fit )
  colnames( estimation.std ) <- c( "Coefficient", "Other information", "" )

  model <- list( selected = m, Q2 = Q2stat, comparison = table, rmse = rmseStat,
                 mae = maeStat, rma = rmaStat, extN = valExtN, estimation = estimation,
                 estimation.std = estimation.std, coefficients = coefficients,
                 coefficients.std = coefficients.std, trend = trendModel,
                 trendNames = trendNames, cov = covModel, covNames = covNames )
  class( model ) <- "kriging-model"

  return( model )
}


# ==== Perform Kriging sensitivity analysis ====

kriging.sensitivity <- function( data, model, krig.sa = FALSE, sa.samp = 1000 ) {

  # ---- Sensitivity analysis for the kriging model ----

  if( krig.sa ) {                     # use Kriging specific SA method
    # generate MC points for Sobol decomposition using low-discrepancy sequences
    X1 <- sobol.rnd.exp( sa.samp, colnames( data$doe ), lwr.bound = data$facLimLo,
                         upr.bound = data$facLimUp )
    X2 <- sobol.rnd.exp( sa.samp, colnames( data$doe ), lwr.bound = data$facLimLo,
                         upr.bound = data$facLimUp )

    metamodel <- sensitivity::sobolGP( model = model$selected, type = "UK", X1 = X1, X2 = X2,
                                       MCmethod = "soboljansen", nboot = 100 )
    mainEffect <- function( x ) x$S$mean
    totEffect <- function( x ) x$T$mean
  } else {
    kriging.mean <- function( point, model ) DiceKriging::predict.km( model, point,
                                                                      type = "UK",
                                                                      se.compute = FALSE )$mean
    metamodel <- sensitivity::fast99( model = kriging.mean, m = model$selected,
                                      n = sa.samp, factors = colnames( data$doe ),
                                      q.arg = data$facLim )
    mainEffect <- function( x ) x$D1 / x$V
    totEffect <- function( x ) 1 - x$Dt / x$V
  }

  sa <- cbind( mainEffect( metamodel ), totEffect( metamodel ) - mainEffect( metamodel ) )
  rownames( sa ) <- colnames( data$doe )
  colnames( sa ) <- c( "Direct effects", "Interactions" )

  max.index <- function( x, pos = 1 )
    as.integer( sapply( sort( x, index.return = TRUE ), `[`, length( x ) - pos + 1 )[ 2 ] )

  topEffect <- c( max.index( totEffect( metamodel ), 1 ), max.index( totEffect( metamodel ), 2 ),
                  max.index( totEffect( metamodel ), 3 ) )

  cat( "Top parameters influencing response surface:\n" )
  cat( " First:", colnames( data$doe )[ topEffect[ 1 ] ], "\n" )
  cat( " Second:", colnames( data$doe )[ topEffect[ 2 ] ], "\n" )
  cat( " Third:", colnames( data$doe )[ topEffect[ 3 ] ], "\n\n" )

  sa <- list( metamodel = metamodel, sa = sa, topEffect = topEffect )
  class( sa ) <- "kriging-sa"

  return( sa )
}


# ==== Polynomial model selection ====

polynomial.model.lsd <- function( data, ext.wgth = 0.5, ols.sig = 0.2,
                                  orderModel = 0, interactModel = 0,
                                  digits = 4 ) {

  # Check if external validation is available or use cross validation only
  if( is.null( data$valid ) ) onlyCross <- TRUE else onlyCross <- FALSE

  # Estimating some polynomial model alternatives and corresponding R2 for:
  polyNames <- c( "order=1", "order=2" )
  interactNames <- c( "interact=N", "interact=Y" )
  modelNames <- matrix( c( "ord=1 int=N", "ord=1 int=Y",
                           "ord=2 int=N", "ord=2 int=Y" ),
                        ncol = 2, byrow = TRUE, dimnames = list( ) )

  R2 <- rmse <- mae <- rma <- rank <- matrix( nrow = nrow( modelNames ),
                                              ncol = ncol( modelNames ) )
  models <- f <- coeff <- std.coeff <- matrix( list( ), nrow = nrow( modelNames ),
                                               ncol = ncol( modelNames ) )

  for( i in 1 : nrow( modelNames ) )
    for( j in 1 : ncol( modelNames ) ) {
      # Estimate polynomial model y = f(x)
      lm <- fit.poly( data$resp$Mean, data$doe, resp.noise = data$resp$Variance,
                      order = i, interaction = j - 1 )
      models[[ i, j ]] <- lm$model
      R2[ i, j ] <- lm$R2
      f[[ i, j ]] <- lm$f
      coeff[[ i, j ]] <- lm$coeff
      std.coeff[[ i, j ]] <- lm$std.coeff

      if( ! onlyCross ) {
        rmse[ i, j ] <- rmse.poly( lm$model, data$valResp$Mean, data$valid )
        mae[ i, j ] <- mae.poly( lm$model, data$valResp$Mean, data$valid )
        rma[ i, j ] <- rma.poly( lm$model, data$valResp$Mean, data$valid )
      } else {
        rmse[ i, j ] <- rmse.poly( lm$model, data$resp$Mean, data$doe )
        mae[ i, j ] <- mae.poly( lm$model, data$resp$Mean, data$doe )
        rma[ i, j ] <- rma.poly( lm$model, data$resp$Mean, data$doe )
      }
    }

  # remove failed statistics
  R2[ is.nan( R2 ) ] <- NA
  rmse[ is.nan( rmse ) ] <- NA
  mae[ is.nan( mae ) ] <- NA
  rma[ is.nan( rma ) ] <- NA

  # select model with maximum R2
  bestCross <- as.vector( which( R2 == max( R2, na.rm = TRUE ),
                                 arr.ind = TRUE )[ 1, ] )

  cat( "Cross validation of alternative models:\n" )
  cat( " Best model (R2):", modelNames[ bestCross[ 1 ], bestCross[ 2 ] ], "\n\n" )

  if( ! onlyCross ) {
    # select model with minimum measures
    bestRmse <- as.vector( which( rmse == min( rmse, na.rm = TRUE ),
                                  arr.ind = TRUE )[ 1, ] )
    bestMae <- as.vector( which( mae == min( mae, na.rm = TRUE ),
                                 arr.ind = TRUE )[ 1, ] )
    bestRma <- as.vector( which( rma == min( rma, na.rm = TRUE ),
                                 arr.ind = TRUE )[ 1, ] )
    cat( "External validation of alternative models:\n" )

  } else {
    bestRmse <- as.vector( which( rmse == min( rmse, na.rm = TRUE ),
                                  arr.ind = TRUE )[ 1, ] )
    bestMae <- as.vector( which( mae == min( mae, na.rm = TRUE ),
                                 arr.ind = TRUE )[ 1, ] )
    bestRma <- as.vector( which( rma == min( rma, na.rm = TRUE ),
                                 arr.ind = TRUE )[ 1, ] )
  }
  cat( " Best model (RMSE):", modelNames[ bestRmse[ 1 ], bestRmse[ 2 ] ], "\n" )
  cat( " Best model (MAE):", modelNames[ bestMae[ 1 ], bestMae[ 2 ] ], "\n" )
  cat( " Best model (RMA):", modelNames[ bestRma[ 1 ], bestRma[ 2 ] ], "\n\n" )

  # select model with best rank (lower is beter)
  for( i in 1 : nrow( modelNames ) )
    for( j in 1 : ncol( modelNames ) ) {
      rank[ i, j ] <- ( 1 - ext.wgth ) * ( 1 - R2[ i, j ] )
      if( ! onlyCross )
        rank[ i, j ] <- rank[ i, j ] +
          ext.wgth * rmse[ i, j ] / ( 3 * max( rmse, na.rm = TRUE ) ) +
          ext.wgth * mae[ i, j ] / ( 3 * max( mae, na.rm = TRUE ) ) +
          ext.wgth * rma[ i, j ] / ( 3 * max( rma, na.rm = TRUE ) )
    }

  # Check if use fixed or best models
  bestOvrall <- as.vector( which( rank == min( rank, na.rm = TRUE ),
                                  arr.ind = TRUE )[ 1, ] )
  if( orderModel == 0 )
    orderModel = bestOvrall[ 1 ]
  if( interactModel == 0 )
    interactModel = bestOvrall[ 2 ]

  m <- models[[ orderModel, interactModel ]]
  R2stat <- R2[ orderModel, interactModel ]
  fStat <- f[[ orderModel, interactModel ]]
  fPval <- stats::pf( fStat[ 1 ], fStat[ 2 ], fStat[ 3 ], lower.tail = FALSE )
  coeff <- coeff[[ orderModel, interactModel ]]
  stdCoeff <- std.coeff[[ orderModel, interactModel ]]
  rmseStat <- rmse[ orderModel, interactModel ]
  maeStat <- mae[ orderModel, interactModel ]
  rmaStat <- rma[ orderModel, interactModel ]

  if( ! onlyCross ) {
    valExtN <- nrow( data$valResp )
    valType <- "External"
  } else {
    valExtN <- NA
    valType <- "Cross"
  }

  # ---- Create tables ----

  table <- data.frame( rbind( R2, rmse, mae, rma, deparse.level = 0 ) )
  colnames( table ) <- interactNames
  rownames( table ) <- c( apply( cbind( rep( "R2", nrow( modelNames ) ), polyNames ),
                                 1, paste, collapse = " " ),
                          apply( cbind( rep( "RMSE", nrow( modelNames ) ), polyNames ),
                                 1, paste, collapse = " " ),
                          apply( cbind( rep( "MAE", nrow( modelNames ) ), polyNames ),
                                 1, paste, collapse = " " ),
                          apply( cbind( rep( "RMA", nrow( modelNames ) ), polyNames ),
                                 1, paste, collapse = " " ) )

  # Prepare the coefficients table
  fmt <- function( x )
    format( x, digits = digits )

  maxRows <- 25                                   # maximum rows per column
  fixRows <- 8                                    # fixed rows (non-coefficient ones)

  estimation <- list( )
  coef <- list( coeff, stdCoeff )
  for( i in 1 : length( coef ) ) {                # tables for the 2 types

    colnames( coef[[ i ]] ) <- c( "SRC", "Std.Error", "t value", "p-value" )

    # auxiliary column for sorting
    coef[[ i ]]$intcpt <- rownames( coef[[ i ]] ) == "(Intercept)"
    # coef[[ i ]] <- coef[[ i ]][ order( - coef[[ i ]][ , 5 ],
    #                      coef[[ i ]][ , 1 ] ), ]                # sort by relevance
    coef[[ i ]] <- coef[[ i ]][ - c( 2, 3, 5 ) ]                  # remove std. errors and t values
    coef[[ i ]] <- coef[[ i ]][ coef[[ i ]][ , 2 ] <= ols.sig, ]  # discard non significant terms
    coef[[ i ]] <- fmt( coef[[ i ]] )                             # round up and convert to text
    if( nrow( coef[[ i ]] ) > 2 * maxRows - fixRows )             # too many variables?
      coef[[ i ]] <- coef[[ i ]][ 1 : ( 2 * maxRows - fixRows ), ]# remove extra ones

    if( nrow( coef[[ i ]] ) <= ( maxRows - fixRows ) ) {
      m.val1 <- c( coef[[ i ]][ , 1 ], modelNames[ orderModel, interactModel ],
                   fmt( fStat[ 1 ] ), fmt( R2stat ), fmt( rmseStat ), fmt( maeStat ),
                   fmt( rmaStat ), nrow( data$resp ), format( valExtN, digits = 1 ) )
      m.val2 <- c( coef[[ i ]][ , 2 ], "", fmt( fPval ), rep( "", length( m.val1 ) -
                                                         nrow( coef[[ i ]] ) - 2 ) )
      m.text <- c( rownames( coef[[ i ]] ), "Model specification", "f-statistic",
                   "Cross valid. adj. R2", paste( valType, "valid. RMSE" ),
                   paste( valType, "valid. MAE" ), paste( valType, "valid. RMA" ),
                   "DoE samples used", "External valid. samples" )
      estimation[[ i ]] <- cbind( m.val1, m.val2 )
      dimnames( estimation[[ i ]] ) <- list( m.text, c( colnames( coef[[ i ]] ) ) )

    } else {

      rowsCol1 <- min( nrow( coef[[ i ]] ), maxRows )
      m.val1 <- c( coef[[ i ]][ 1 : rowsCol1, 1 ] )
      m.val2 <- c( coef[[ i ]][ 1 : rowsCol1, 2 ] )
      m.text1 <- c( rownames( coef[[ i ]] )[ 1 : rowsCol1 ] )

      m.val3 <- m.val4 <- m.text2 <- vector( "character" )
      rowOvFlw <- 0
      if( nrow( coef[[ i ]] ) > maxRows ) {
        m.val3 <- coef[[ i ]][ ( maxRows + 1 ) : nrow( coef[[ i ]] ), 1 ]
        m.val4 <- coef[[ i ]][ ( maxRows + 1 ) : nrow( coef[[ i ]] ), 2 ]
        m.text2 <- rownames( coef[[ i ]] )[ ( maxRows + 1 ) : nrow( coef[[ i ]] ) ]
        rowOvFlw <- nrow( coef[[ i ]] ) - maxRows
      }

      m.val3 <- c( m.val3, rep( "", rowsCol1 - rowOvFlw - fixRows ),
                   modelNames[ orderModel, interactModel ], fmt( fStat[ 1 ] ),
                   fmt( R2stat ), fmt( rmseStat ), fmt( maeStat ), fmt( rmaStat ),
                   nrow( data$resp ), format( valExtN, digits = 1 ) )
      m.val4 <- c( m.val4, rep( "", rowsCol1 - rowOvFlw - fixRows + 1 ),
                   fmt( fPval ), rep( "", fixRows - 2 ) )
      m.text2 <- c( m.text2, rep( "", rowsCol1 - rowOvFlw - fixRows ),
                    "Model specification", "f-statistic", "Cross valid. adj. R2",
                    paste( valType, "valid. RMSE" ), paste( valType, "valid. MAE" ),
                    paste( valType, "valid. RMA" ), "DoE samples used",
                    "External valid. samples" )
      estimation[[ i ]] <- cbind( m.val1, m.val2, rep( "", min( nrow( coef[[ i ]] ), maxRows ) ),
                                  m.text2, m.val3, m.val4 )
      dimnames( estimation[[ i ]] ) <- list( m.text1,
                                             c( colnames( coef[[ i ]] ),
                                                rep( "", 2 ),
                                                colnames( coef[[ i ]] ) ) )
    }
  }

  model <- list( selected = m, R2 = R2stat, comparison = table, rmse = rmseStat,
                 mae = maeStat, rma = rmaStat, extN = valExtN,
                 estimation = estimation[[ 1 ]], estimation.std = estimation[[ 2 ]],
                 coefficients = coeff, coefficients.std = stdCoeff, order = orderModel,
                 polyNames = polyNames, interact = interactModel,
                 interactNames = interactNames )
  class( model ) <- "polynomial-model"

  return( model )
}


# ==== Perform polynomial sensitivity analysis ====

polynomial.sensitivity <- function( data, model, sa.samp = 1000 ) {

  # ------ Sensitivity analysis for best model ------

  metamodel <- sensitivity::fast99( model = model$selected, n = sa.samp,
                                    factors = colnames( data$doe ),
                                    q.arg = data$facLim )

  mainEffect <- function( x ) x$D1 / x$V
  totEffect <- function( x ) 1 - x$Dt / x$V

  sa <- cbind( mainEffect( metamodel ), totEffect( metamodel ) - mainEffect( metamodel ) )
  rownames( sa ) <- colnames( data$doe )
  colnames( sa ) <- c( "Direct effects", "Interactions" )

  max.index <- function( x, pos = 1 ) as.integer( sapply( sort( x, index.return = TRUE ),
                                                          `[`, length( x ) - pos + 1 )[ 2 ] )

  topEffect <- c( max.index( totEffect( metamodel ), 1 ),
                  max.index( totEffect( metamodel ), 2 ),
                  max.index( totEffect( metamodel ), 3 ) )

  cat( "Top parameters influencing response surface:\n" )
  cat( " First:", colnames( data$doe )[ topEffect[ 1 ] ], "\n" )
  cat( " Second:", colnames( data$doe )[ topEffect[ 2 ] ], "\n" )
  cat( " Third:", colnames( data$doe )[ topEffect[ 3 ] ], "\n\n" )

  sa <- list( metamodel = metamodel, sa = sa, topEffect = topEffect )
  class( sa ) <- "polynomial-sa"

  return( sa )
}


# ==== Perform Elementary Effects sensitivity analysis ====

elementary.effects.lsd <- function( data, p = 4, jump = 2 ) {

  # ---- Sensitivity analysis for the EE model ----

  if( nrow( data$doe ) < ( ncol( data$doe ) + 1 ) ||
      nrow( data$doe ) %% ( ncol( data$doe ) + 1 ) != 0 )
    stop( "Invalid DoE size for Elementary Effects analysis" )

  # Create object of class "morris" to use standard sensitivity package
  sa <- list( model = NULL, factors = colnames( data$doe ), samples = nrow( data$doe ),
              r = nrow( data$doe ) / ( ncol( data$doe ) + 1 ),
              design = list( type = "oat", levels = p, grid.jump = jump ),
              binf = data$facLimLo, bsup = data$facLimUp, scale = TRUE,
              call = match.call( ) )
  class( sa ) <- "morris"

  # Scale all factors to the range [0,1]
  Binf <- matrix( sa$binf, nrow = nrow( data$doe ), ncol = length( sa$binf ), byrow = TRUE )
  Bsup <- matrix( sa$bsup, nrow = nrow( data$doe ), ncol = length( sa$bsup ), byrow = TRUE )
  sa$X <- ( data$doe - Binf ) / ( Bsup - Binf )
  sa$y <- data$resp$Mean
  rm( Binf, Bsup )

  # Call elementary effects analysis from sensitivity package
  sa$ee <- ee.oat( sa$X, sa$y )

  # change the class to lsd print/plot equivalents
  class( sa ) <- "morris.lsd"

  # add the standard error to the statistics
  sa$table <- as.data.frame( print( sa ) )
  sa$table$se <- sa$table$sigma / sqrt( sa$r )
  sa$table$p.value <- stats::pt( sa$table$mu.star / sa$table$se, df = sa$r - 1, lower.tail = FALSE )

  return( sa )
}
