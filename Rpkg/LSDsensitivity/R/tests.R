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

ergod.test.lsd <- function( data, vars = dimnames( data )[[ 2 ]],
                            start.period = 0, signif = 0.05, digits = 2,
                            ad.method = c( "asymptotic", "simulated", "exact" ) ) {

  if( ! is.array( data ) || length( dim( data ) ) != 3 )
    stop( "Invalid data set format (not 3D array)" )

  if( is.null( vars ) || ! is.character( vars ) || length( vars ) == 0 ||
      length( vars ) > dim( data )[ 2 ] )
    stop( "Invalid variable name vector (vars)" )

  if( is.null( start.period ) || ! is.finite( start.period ) ||
      round( start.period ) < 0 )
    stop( "Invalid starting period (start.period)" )

  if( is.null( signif ) || ! is.finite( signif ) || signif <= 0 || signif > 1 )
    stop( "Invalid significance level (signif)" )

  if( is.null( digits ) || ! is.finite( digits ) || round( digits ) < 0 )
    stop( "Invalid significant digits (digits)" )

  start.period <- round( start.period )
  digits       <- round( digits )
  ad.method    <- match.arg( ad.method )

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


# ==== Calculate unimodality & symmetry statistics table ====

symmet.test.lsd <- function( data, vars = dimnames( data )[[ 2 ]],
                             start.period = 0, signif = 0.05, digits = 2,
                             sym.boot = FALSE ) {

  if( ! is.array( data ) || length( dim( data ) ) != 3 )
    stop( "Invalid data set format (not 3D array)" )

  if( is.null( vars ) || ! is.character( vars ) || length( vars ) == 0 ||
      length( vars ) > dim( data )[ 2 ] )
    stop( "Invalid variable name vector (vars)" )

  if( is.null( start.period ) || ! is.finite( start.period ) ||
      round( start.period ) < 0 )
    stop( "Invalid starting period (start.period)" )

  if( is.null( signif ) || ! is.finite( signif ) || signif <= 0 || signif > 1 )
    stop( "Invalid significance level (signif)" )

  if( is.null( digits ) || ! is.finite( digits ) || round( digits ) < 0 )
    stop( "Invalid significant digits (digits)" )

  if( is.null( sym.boot ) || ! is.logical( sym.boot ) )
    stop( "Invalid boostrap critical values switch (sym.boot)" )

  start.period <- round( start.period )
  digits       <- round( digits )

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
