#*******************************************************************************
#
# -------------- LSD tools for impulse-response function analysis -------------
#
#   Written by Marcelo C. Pereira, University of Campinas
#              Marco Amendola, University of L'Aquila
#
#   Copyright Marcelo C. Pereira & Marco Amendola
#   Distributed under the GNU General Public License
#
#*******************************************************************************

# ==== Linear impulse-response function ====

irf.lsd <- function( data, data.shock, t.horiz, var.irf, var.shock, var.ref = NULL,
                     irf.type = c( "incremental", "cumulative", "none" ),
                     stat = c( "mean", "median" ), ci.R = 999,
                     ci.type = c( "basic", "perc", "bca" ),
                     lim.outl = 0, alpha = 0.05, seed = 1, ... ) {

  check_3D_data( data, data.shock )

  if( is.null( dimnames( data )[[ 2 ]] ) )
    dimnames( data )[[ 2 ]] <- 1 : dim( data )[ 2 ]

  if( is.null( dimnames( data.shock )[ 2 ] ) )
    dimnames( data.shock )[[ 2 ]] <- 1 : dim( data.shock )[ 2 ]

  if( is.null( var.irf ) || length( var.irf ) > 1 ||
      ! var.irf %in% dimnames( data )[[ 2 ]] )
    stop( "Invalid IRF variable (var.irf)" )

  if( is.null( var.shock ) || length( var.shock ) > 1 ||
      ! var.shock %in% dimnames( data.shock )[[ 2 ]] )
    stop( "Invalid shock variable (var.shock)" )

  if( is.null( t.horiz ) || ! is.finite( t.horiz ) || round( t.horiz ) < 1 )
    stop( "Invalid time horizon (t.horiz)" )

  if( is.null( alpha ) || ! is.finite( alpha ) ||
      alpha <= 0 || alpha > 0.5 )
    stop( "Invalid significance level (alpha)" )

  if( is.null( ci.R ) || ! is.finite( ci.R ) || round( ci.R ) < 1 )
    stop( "Invalid bootstrap repetitions (ci.R)" )

  if( is.null( var.ref ) )
    var.ref = ""

  if( var.ref == "%" )
    var.ref = var.irf

  if( ! is.character( var.ref ) || length( var.ref ) > 1 ||
      ( var.ref != "" && ! var.ref %in% dimnames( data )[[ 2 ]] ) )
    stop( "Invalid IRF reference variable (var.ref)" )

  if( is.null( lim.outl ) || ! is.finite( lim.outl ) || lim.outl < 0 )
    stop( "Invalid outlier threshold limit (lim.outl)" )

  if( ! is.null( seed ) && ! is.finite( seed ) )
    stop( "Invalid random seed (seed)" )

  t.horiz <- round( t.horiz )

  if( dim( data )[ 1 ] < ( t.horiz + 1 ) ) {
    warning( "Insufficient data for time horizon (t.horiz), adjusted" )
    t.horiz <- dim( data )[ 1 ] - 1
  }

  ci.R <- round( ci.R )
  irf.type <- match.arg( irf.type )
  stat <- match.arg( stat )
  ci.type <- match.arg( ci.type )

  tShock <- outliers <- vector( "numeric" )
  ir <- cir <- data.frame( )

  # compute run-specific IRF and run-specific C-IRF
  for( i in 1 : dim( data )[ 3 ] ) {    # for each MC run

    # search for first shock period
    for( t in 2 : dim( data.shock )[ 1 ] )
      if( data.shock[ t, var.shock, i ] != data.shock[ t - 1, var.shock, i ] )
        break

    # ignore if shock was not detected
    if ( data.shock[ t, var.shock, i ] != data.shock[ t - 1, var.shock, i ] ) {

      # collect IR absolute data
      irData <- data.shock[ t : ( t + t.horiz ), var.irf, i ] -
        data[ t : ( t + t.horiz ), var.irf, i ]

      # accumulate ir
      cirData <- irData[ 1 ]
      for ( w in 2 : length( irData ) )
        cirData[ w ] <- cirData[ w - 1 ] + irData[ w ]

      # handle relative ir's
      if( var.ref != "" ) {
        irData <- irData / data[ t, var.ref, i ]
        cirData <- cirData / data[ t, var.ref, i ]

        irData[ ! is.finite( irData ) ] <- NA     # handle divisions by zero
        cirData[ ! is.finite( cirData ) ] <- NA
      }
    } else {
      t <- NA
      irData <- cirData <- rep( NA, t.horiz + 1 )
    }

    # add to dataset
    tShock <- append( tShock, t )
    ir <- rbind( ir, irData )
    cir <- rbind( cir, cirData )
  }

  names( tShock ) <- rownames( ir ) <- rownames( cir ) <- dimnames( data )[[ 3 ]]
  colnames( ir ) <- colnames( cir ) <- 0 : ( ncol( ir ) - 1 )

  if( lim.outl > 0 ) {             # remove outliers?

    # find upper/lower quartiles for ir's
    hiQuart <- unlist( lapply( ir, stats::quantile, prob = 0.75, type = 8,
                               na.rm = TRUE ) )
    loQuart <- unlist( lapply( ir, stats::quantile, prob = 0.25, type = 8,
                               na.rm = TRUE ) )

    # define limits for outliers based on a multiple over upper/lower quartiles
    maxOutl <- hiQuart + lim.outl * ( hiQuart - loQuart )
    minOutl <- loQuart - lim.outl * ( hiQuart - loQuart )

    # remove cases with strong impact or on the cumulated irf for metric
    for( i in 1: nrow( ir ) )
      if( any( ir[ i, ] > maxOutl ) || any( ir[ i, ] < minOutl ) )
        outliers <- append( outliers, i )

    if( length( outliers ) > 0 ) {
      names( outliers ) <- rownames( ir )[ outliers ]
      ir  <- ir[ - outliers, ]
      cir <- cir[ - outliers, ]
      tShock  <- tShock[ - outliers ]
    }
  }

  # compute IRF, culumative IRF and define function to calculate boostrap IRF
  if( stat == "median" ) {
    irf <- apply( ir, 2, stats::median, na.rm = TRUE )
    cirf <- apply( cir, 2, stats::median, na.rm = TRUE )
    funIRF <- function( data, idx ) stats::median( data[ idx ], na.rm = TRUE )
  } else {
    irf <- apply( ir, 2, mean, na.rm = TRUE )
    cirf <- apply( cir, 2, mean, na.rm = TRUE )
    funIRF <- function( data, idx ) mean( data[ idx ], na.rm = TRUE )
  }

  irfCIlo <- irfCIhi <- cirfCIlo <- cirfCIhi <- rep( NA, ncol( ir ) )

  set.seed( seed )      # reset PRNG seed to ensure reproducibility

  # compute bootstrap confidence intervals and IRF limits
  for( i in 1 : ncol( ir ) ) {
    irfCI <- cirfCI <- NULL

    try( invisible(
      utils::capture.output(
        irfCI <- boot::boot.ci( boot::boot( ir[ , i ], statistic = funIRF,
                                            R = ci.R ),
                                conf = 1 - alpha, type = ci.type )
      )
    ), silent = TRUE )

    try( invisible(
      utils::capture.output(
        cirfCI <- boot::boot.ci( boot::boot( cir[ , i ], statistic = funIRF,
                                             R = ci.R ),
                                 conf = 1 - alpha, type = ci.type )
      )
    ), silent = TRUE )

    if( ! is.null( irfCI ) ) {
      irfCIlo[ i ] <- irfCI$basic[ 4 ]
      irfCIhi[ i ] <- irfCI$basic[ 5 ]
    }

    if( ! is.null( cirfCI ) ) {
      cirfCIlo[ i ] <- cirfCI$basic[ 4 ]
      cirfCIhi[ i ] <- cirfCI$basic[ 5 ]
    }
  }

  # find IRF vertical limits
  irfYlim  <- c( min( irfCIlo, na.rm = TRUE ), max( irfCIhi, na.rm = TRUE ) )
  cirfYlim <- c( min( cirfCIlo, na.rm = TRUE ), max( cirfCIhi, na.rm = TRUE ) )

  # data signature
  dataCRC <- digest::digest( data, algo = "crc32" )

  # pack object
  irf <- list( irf = irf, cirf = cirf, irf.ci.lo = irfCIlo, irf.ci.hi = irfCIhi,
               cirf.ci.lo = cirfCIlo, cirf.ci.hi = cirfCIhi, irf.ylim = irfYlim,
               cirf.ylim = cirfYlim, ir = ir, cir = cir, t.shock = tShock,
               t.horiz = t.horiz, var.irf = var.irf, var.shock = var.shock,
               var.ref = var.ref, stat = stat, alpha = alpha, nsample = nrow( ir ),
               outliers = outliers, data.crc = dataCRC, call = match.call( ) )
  class( irf ) <- "irf.lsd"

  if( irf.type != "none" )
    plot.irf.lsd( irf, irf.type = irf.type, ... )

  return( irf )
}


# ====== Linear impulse-response function print/plot ======

print.irf.lsd <- function( x, ... ) {

  if( ! inherits( x, "irf.lsd" ) )
    stop( "Invalid object (not from irf.lsd())" )

  if( min( x$t.shock ) == max( x$t.shock ) )
    t.shock <- x$t.shock[ 1 ]
  else
    t.shock <- paste0( "[", min( x$t.shock ), ", ", max( x$t.shock ), "]" )

  info <- data.frame( c( x$var.irf,
                         x$var.shock,
                         x$var.ref,
                         t.shock,
                         length( x$irf ),
                         x$alpha,
                         x$nsample,
                         length( x$outliers ) ) )
  colnames( info ) <- NULL
  rownames( info ) <- c( "Impulse-response function variable:",
                         "Impulse (shock) variable:",
                         "Reference variable (impulse time):",
                         "Impulse (relative) time (range):",
                         "Time-horizon response period:",
                         "Confidence interval significance:",
                         "Number of employed MC samples:",
                         "Number of outlier MC samples:" )
  print( info, ... )

  cat( "\nImpulse-response and cumulative impulse-response functions:\n\n" )

  data <- data.frame( x$irf.ci.lo, x$irf, x$irf.ci.hi,
                      c( 0 : ( length( x$irf ) - 1 ) ),
                      x$cirf.ci.lo, x$cirf, x$cirf.ci.hi )
  colnames( data ) <- c( "IRF ci-", paste( "IRF", x$stat ), "IRF ci+",
                         "t",
                         "C-IRF ci-", paste( "C-IRF", x$stat ), "C-IRF ci+" )
  print( data, row.names = FALSE, ... )
}


plot.irf.lsd <- function( x, ... ) {

  if( ! inherits( x, "irf.lsd" ) )
    stop( "Invalid object (not from irf.lsd())" )

  if( ! exists( "irf.type" ) )
    irf.type <- "cumulative"
  else
    irf.type <- match.arg( irf.type, c( "cumulative", "incremental" ) )

  if( irf.type == "cumulative" ) {
    data <- x$cirf
    ciLo <- x$cirf.ci.lo
    ciHi <- x$cirf.ci.hi
    ylim <- x$cirf.ylim
  } else {
    data <- x$irf
    ciLo <- x$irf.ci.lo
    ciHi <- x$irf.ci.hi
    ylim <- x$irf.ylim
  }

  plotPars <- list( ... )
  plotPars[[ "irf.type" ]] <- NULL

  do.call( plot_irf, c( list( data, ciLo, ciHi, ylim ), plotPars ) )
}
