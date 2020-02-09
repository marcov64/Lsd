#******************************************************************
#
# ------------- Plot, fit and support functions -----------------
#
#******************************************************************

# ==== User parameters ====

useSubbotools <- TRUE          # use Subbotools (T) or normalp package (F)
subboMaxSample <- 5000         # maximum sample size in Subbotin fits (speed control)
subboMinSample <- 20           # minimum sample size in Subbotin fits (significance control)
subboBlimit <- 5               # maximum limit for b to be considered valid (0=no limit)
maxSample <- 10000             # maximum sample size in plots (pdf control)
topMargin <- 0.2               # top plot margin scaling factor
botMargin <- 0.1               # bottom plot margin scaling factor
def.digits <- 4                # default number of digits after comma for printing


# ==== Required libraries (order is relevant!) ====

require( normalp, warn.conflicts = FALSE, quietly = TRUE )
require( rmutil, warn.conflicts = FALSE, quietly = TRUE )
require( nortest, warn.conflicts = FALSE, quietly = TRUE )
require( gplots, warn.conflicts = FALSE, quietly = TRUE )
require( plotrix, warn.conflicts = FALSE, quietly = TRUE )
require( parallel, warn.conflicts = FALSE, quietly = TRUE )
suppressPackageStartupMessages( require( matrixStats, warn.conflicts = FALSE, quietly = TRUE ) )
suppressPackageStartupMessages( require( tseries, warn.conflicts = FALSE, quietly = TRUE ) )
suppressPackageStartupMessages( require( np, warn.conflicts = FALSE, quietly = TRUE ) )
suppressPackageStartupMessages( require( extrafont, warn.conflicts = FALSE, quietly = TRUE ) )
invisible( capture.output( require( mFilter, warn.conflicts = FALSE ) ) )

# subbotools location (leave blank in linux/Mac)
if( tolower( .Platform$OS.type ) == "windows" ){
  subbotoolsFolder <- "subbotools-1.3.0-cygwin64\\"
} else{
  subbotoolsFolder <- ""
}

# remove warnings for support functions
# !diagnostics suppress = paramp


# ==== Basic functions ====

# test if all elements in a matrix/dataframe row are NA
all.NA <- function( x ) {
  apply( x, 1, function( x ) all( is.na( x ) ) )
}

# expand is.nan to handle data frames
is.nan.data.frame <- function( x ) {
  do.call( cbind, lapply( x, is.nan ) )
}

# Redefine log with NA instead of -Inf
logNA <- function(x){
  try( y <- log(x), silent = TRUE )
  y[!is.finite(y)] <- NA
  return(y)
}

# Redefine log with zero floor
log0 <- function(x){
  try( y <- log(x), silent = TRUE )
  y[y < 0] <- 0
  y[!is.finite(y)] <- 0
  return(y)
}

# Redefine t-test with critCorr to use 0 in case of NaN
t.test0 <- function( x, mu = 0, conf.level = 0.95 ){
  x[ is.nan( x ) ] <- 0
  return( t.test( x, mu = mu, alternative = "greater",
                  conf.level = conf.level )$p.value )
}

# Standard error for a sample
se <- function( x, na.rm = TRUE ){
  if( na.rm )
    n <- length( x[ ! is.na( x ) ] )
  else
    n <- length( x )

  return( sd( x, na.rm = na.rm ) / sqrt( n ) )
}

# Rounding and formatting numbers for tables
fmt <- function( x, digits = def.digits, signif = NULL, scipen = NA ) {
  if( is.numeric( x ) ) {
    return( format( round( x, digits = digits ),
                    nsmall = digits, digits = signif, scientific = scipen ) )
  }
  if( is.data.frame( x ) ) {
    for( i in 1 : ncol( x ) )
      if( is.numeric( x[ , i ] ) )
        x[ , i ] <- round( x[ , i ], digits = digits )
      return( format( x, nsmall = digits, digits = signif, scientific = scipen ) )
  }
  stop( "Cannot format non-numeric data" )
}

# Define plot window y limits
findYlim <- function( yMin, yMax, zero = FALSE ) {
  ylim <- c( yMin - botMargin * ( yMax - yMin ),
             yMax + topMargin * ( yMax - yMin ) )
  if( zero ) {
    if( ylim[ 1 ] <= 0 )
      ylim[ 1 ] <- yMin * ( 1 - botMargin )
    if( ylim[ 2 ] <= ylim[ 1 ] )
      ylim[ 2 ] <- yMax * ( 1 + topMargin )
  }

  return( ylim )
}

# Determine the number of cores to use
nCores <- function( cores = 0, nStats = 0 ) {

  # find the maximum useful number of cores ( <= num. cores )
  if( cores == 0 )
    cores <- detectCores( )
  nc <- min( cores, detectCores( ) )

  if( nStats > 0 ) {
    i <- 1
    while( ceiling( nStats / i ) > cores )
      i <- i + 1
    nc <- ceiling( nStats / i )
  }

  if( cores == 1 )
    nc <- 1

  return( nc )
}


# ==== Statistical functions ====

#
# ====== function [] = fit_subbotin ======
# Output:
#  List with symmetric Subbotin distribution estimated parameters
#
# Input:
#   x: series
#

exec_subbofit <- function( x, type  = "symmetric" ) {

  if( type == "asymmetric" )
    command <- "subboafit"
  else
    command <- "subbofit"

  cat( "", as.character( Sys.time( ) ), "->", type, "subbofit, n =", length( x ), "... " )

  outStr <- system2( paste0( subbotoolsFolder, command ), args = "-O 3",
                     input = as.character( x ), stdout = TRUE, stderr = FALSE )
  try( subboFit <- scan( textConnection ( outStr ), quiet = TRUE ), silent = TRUE )

  if( type == "asymmetric" )
    se <- paste( subboFit[ 6 ], subboFit[7] )
  else
    se <- subboFit[ 4 ]
  cat( "done, b_se =", se, "\n" )

  return( subboFit )
}

fit_subbotin <- function( x ){
  # default return in case of error
  subboFit <- c( as.numeric( NA ), as.numeric( NA ), as.numeric( NA ) )

  # prepare valid data for Subbotools (no NA's & limited sample size)
  x <- x[ !is.na( x ) ]
  if( length( x ) > subboMaxSample )
    x <- sample( x, subboMaxSample )
  if( length( x ) < subboMinSample ){
    warning( "Too few observations to fit Subbotin: returning NA")
    return( list( b = subboFit[1], a = subboFit[2], m = subboFit[3] ) )
  }

  if( useSubbotools && length( x ) >= 50 )
    subboFit <- exec_subbofit( x )
  else{       # Alternative calculation using the normalp package
    sf <- paramp( x )
    sf$p <- estimatep( x, mu = sf$mean, p = sf$p, method = "inverse" )
    # use Subbotools when p < 1, as normalp doesn't work in this condition
    if( sf$p <= 1.01 )
      subboFit <- exec_subbofit( x )
    else
      subboFit <- c( sf$p, sf$sp, sf$mp )
  }

  # check for degenerated distribution
  if( subboBlimit != 0 && ! is.na( subboFit[1] ) && subboFit[1] > subboBlimit ){
    subboFit <- c( as.numeric( NA ), as.numeric( NA ), as.numeric( NA ) )
    warning( "Degenerated Subbotin distribution: returning NA")
  }

  return( list( b = subboFit[1], a = subboFit[2], m = subboFit[3] ) )
}


#
# ====== function [] = remove_extremes ======
# Output:
#  Series containing values outside defined quantiles
#
# Input:
#   x: series to remove outliers
#	  quant: limit quantile (0-1)
#	  na.rm: remove missing
#

remove_extremes <- function( x, quant = 0.25, na.rm = TRUE, ... ) {
  if( length( x ) == 0 )
    return( c( ) )
  qnt <- quantile( x, probs=c( quant, 1 - quant ), na.rm = na.rm, ... )
  y <- x
  y[ x < qnt[1] ] <- NA
  y[ x > qnt[2] ] <- NA
  if( na.rm )
    y <- y[ ! is.na( y ) ]
  return( y )
}


#
# ====== function [] = outliers ======
# Output:
#  outlier positions in vector x
#
# Input:
#   x: series to remove outliers
#	  quant: limit quantile (0-1)
#

outliers <- function( x, quant = 0.25, na.rm = TRUE, ... ) {
  if( length( x ) == 0 )
    return( c( ) )
  qnt <- quantile( x, probs=c( quant, 1 - quant ), na.rm = na.rm, ... )
  H <- 1.5 * ( IQR( x, na.rm = na.rm ) / 0.5 ) * 2 * quant
  y <- rep( FALSE, length( x ) )
  y[ x < ( qnt[ 1 ] - H ) ] <- TRUE
  y[ x > ( qnt[ 2 ] + H ) ] <- TRUE
  z <- vector( )
  for( i in 1 : length( y ) )
    if( y[ i ] )
      z <- append( z, i )
  return( z )
}


#
# ====== function [] = nOutliers ======
# Output:
#  non-outlier positions in vector x
#
# Input:
#   x: series to remove outliers
#	  quant: limit quantile (0-1)
#

nOutliers <- function( x, quant = 0.25, na.rm = TRUE, ... ) {
  if( length( x ) == 0 )
    return( c( ) )
  qnt <- quantile( x, probs=c( quant, 1 - quant ), na.rm = na.rm, ... )
  H <- 1.5 * ( IQR( x, na.rm = na.rm ) / 0.5 ) * 2 * quant
  y <- rep( TRUE, length( x ) )
  y[ x < ( qnt[ 1 ] - H ) ] <- FALSE
  y[ x > ( qnt[ 2 ] + H ) ] <- FALSE
  z <- vector( )
  for( i in 1 : length( y ) )
    if( y[ i ] )
      z <- append( z, i )
  return( z )
}


#
# ====== function [] = remove_outliers ======
# Output:
#  Series containing non-outlier values
#
# Input:
#   x: series to remove outliers
#	  quant: limit quantile (0-1)
#	  na.rm: remove missing
#

remove_outliers <- function( x, quant = 0.25, na.rm = TRUE, ... ) {
  if( length( x ) == 0 )
    return( c( ) )
  qnt <- quantile( x, probs=c( quant, 1 - quant ), na.rm = na.rm, ... )
  H <- 1.5 * ( IQR( x, na.rm = na.rm ) / 0.5 ) * 2 * quant
  y <- x
  y[ x < ( qnt[1] - H ) ] <- NA
  y[ x > ( qnt[2] + H ) ] <- NA
  if( na.rm )
    y <- y[ ! is.na( y ) ]
  return( y )
}


#
# ====== function [] = remove_outliers_table ======
# Output:
#  Table of series containing non-outlier values
#  All lines containing NAs are removed
#
# Input:
#   x: table of series (columns) to remove outliers
#	  quant: limit quantile (0-1)
#

remove_outliers_table <- function( x, quant = 0.25, ... ) {

  if( nrow( x ) == 0 )
    return( c( ) )

  iniLen <- nrow( x )        # initial number of observations
  outList <- vector( "numeric" )

  # remove NAs first
  for( i in 1 : ncol( x ) )
    x <- x[ ! is.na( x[ , i ] ), , drop = FALSE ]

  # remove NAs and identify outliers for each series
  for( i in 1 : ncol( x ) )
    outList <- append( outList, outliers( x[ , i ], quant, na.rm = TRUE, ... ) )

  # remove duplicates and sort list
  outList <- sort( unique( outList ) )

  # check abnormal number of outliers
  outPerc <- length( outList ) / iniLen
  if( outPerc > 2 * quant && outPerc > 0.05 )
    warning( "Too many outliers:", outPerc * 100, "%" )

  # remove outliers' lines from table
  return( x[ - outList, , drop = FALSE ] )
}


#
# ====== function [] = abs_max ======
# Output:
#  Series to find absolute maximum (excluding extremes)
#
# Input:
#   x: series to analyze
#	  quant: limit quantile (0-1)
#	  na.rm: remove missing
#

abs_max <- function( x, quant = 0.25 ) {
  y <- remove_extremes( x, quant = quant, na.rm = TRUE )
  return( max( max( y ), - min( y ) ) )
}


#
# ====== function [] = lm_outl( ======
# Output:
#  linear regression result, not considering extremes
#
# Input:
#   x, y: series to regress
#	  quant: limit quantile (0-1) to consider
#   rm.outl: remove outliers regression
#

lm_outl <- function( y, x, quant = 0.05, rm.outl = TRUE ){

  if( rm.outl ){
    x <- remove_extremes( x, quant = quant, na.rm = FALSE )
    y <- remove_extremes( y, quant = quant, na.rm = FALSE )
  }

  return( lm( y ~ x ) )
}


#
# ====== function [] = comp_stats ======
# Output:
#  several statistics obtained from x (in this order):
#   mean, std. deviation, symmetric subbotin fit,
#   jarque-bera normality test, lilliefors n. test,
#   anderson-darling n. test
#
# Input:
#   x: vector or matrix of data
#

comp_stats <- function ( x ){
  x <- as.vector( x )
  x <- x[ ! is.na( x ) ]

  # define default (NA) values in case of error
  jbx <- list( statistic = as.numeric( NA ), parameter = as.numeric( NA ),
               p.value = as.numeric( NA ), method = "", data.name ="" )
  llx <- list( statistic = as.numeric( NA ), p.value = as.numeric( NA ),
               method = "", data.name ="" )
  adx <- list( statistic = as.numeric( NA ), p.value = as.numeric( NA ),
               method = "", data.name ="" )
  acfx <- c( as.numeric( NA ), as.numeric( NA ), as.numeric( NA ) )

  # run calculations
  mx <- mean( x )
  if( is.nan( mx ) )
    mx <- as.numeric( NA )
  sx <- sd( x )
  if( is.nan( sx ) )
    sx <- as.numeric( NA )
  subbox <- fit_subbotin( x )
  try( jbx <- jarque.bera.test( x ), silent = TRUE )
  try( llx <- lillie.test( x ), silent = TRUE )
  try( adx <- ad.test( x ), silent = TRUE )
  try( acfx <- acf( x, plot = FALSE )$acf, silent = TRUE )    # autocorrelation lags

  # check for infinite/NaN results
  if( ! is.finite( jbx$statistic ) ) jbx$statistic <- as.numeric( NA )
  if( ! is.finite( jbx$p.value ) ) jbx$p.value <- as.numeric( NA )
  if( ! is.finite( llx$statistic ) ) llx$statistic <- as.numeric( NA )
  if( ! is.finite( jbx$p.value ) ) jbx$p.value <- as.numeric( NA )
  if( ! is.finite( adx$statistic ) ) adx$statistic <- as.numeric( NA )
  if( ! is.finite( adx$p.value ) ) adx$p.value <- as.numeric( NA )
  if( ! is.finite( acfx[2] ) ) acfx[2] <- as.numeric( NA )
  if( ! is.finite( acfx[3] ) ) acfx[3] <- as.numeric( NA )

  return( list( avg = mx, sd = sx, subbo = subbox,
                jb = jbx, ll = llx,  ad = adx,
                ac = list ( t1 = acfx[2], t2 = acfx[3] ) ) )
}


#
# ====== function [] = comp_MC_stats ======
# Output:
#  statistics obtained from a set of Monte
#   Carlo runs:  mean, std. deviation, maximum,
#   minimum
#
# Input:
#   x: list of MC data
#

comp_MC_stats <- function ( x ){

  # build list with Monte Carlo average of runs for selected variables
  avg.avg <- mean( sapply( x, '[[', "avg" ), na.rm = TRUE )
  avg.sd <- mean( sapply( x, '[[', "sd" ), na.rm = TRUE )
  avg.subbo <- list( b = mean( simplify2array( sapply( x, '[[', "subbo" )[ "b", ] ), na.rm = TRUE ),
                     a = mean( simplify2array( sapply( x, '[[', "subbo" )[ "a", ] ), na.rm = TRUE ),
                     m = mean( simplify2array( sapply( x, '[[', "subbo" )[ "m", ] ), na.rm = TRUE ) )
  avg.jb <- list( statistic = mean( simplify2array( sapply( x, '[[', "jb" )[ "statistic", ] ), na.rm = TRUE ),
                  p.value = mean( simplify2array( sapply( x, '[[', "jb" )[ "p.value", ] ), na.rm = TRUE ) )
  avg.ll <- list( statistic = mean( simplify2array( sapply( x, '[[', "ll" )[ "statistic", ] ), na.rm = TRUE ),
                  p.value = mean( simplify2array( sapply( x, '[[', "ll" )[ "p.value", ] ), na.rm = TRUE ) )
  avg.ad <- list( statistic = mean( simplify2array( sapply( x, '[[', "ad" )[ "statistic", ] ), na.rm = TRUE ),
                  p.value = mean( simplify2array( sapply( x, '[[', "ad" )[ "p.value", ] ), na.rm = TRUE ) )
  avg.ac <- list( t1 = mean( simplify2array( sapply( x, '[[', "ac" )[ "t1", ] ), na.rm = TRUE ),
                  t2 = mean( simplify2array( sapply( x, '[[', "ac" )[ "t2", ] ), na.rm = TRUE ) )

  avg.x <- list( avg = avg.avg, sd = avg.sd, subbo = avg.subbo,
                 jb = avg.jb, ll = avg.ll, ad = avg.ad, ac = avg.ac  )

  # build list with Monte Carlo std. dev. of runs for selected variables
  sd.avg <- sd( sapply( x, '[[', "avg" ), na.rm = TRUE )
  sd.sd <- sd( sapply( x, '[[', "sd" ), na.rm = TRUE )
  sd.subbo <- list( b = sd( simplify2array( sapply( x, '[[', "subbo" )[ "b", ] ), na.rm = TRUE ),
                    a = sd( simplify2array( sapply( x, '[[', "subbo" )[ "a", ] ), na.rm = TRUE ),
                    m = sd( simplify2array( sapply( x, '[[', "subbo" )[ "m", ] ), na.rm = TRUE ) )
  sd.jb <- list( statistic = sd( simplify2array( sapply( x, '[[', "jb" )[ "statistic", ] ), na.rm = TRUE ),
                 p.value = sd( simplify2array( sapply( x, '[[', "jb" )[ "p.value", ] ), na.rm = TRUE ) )
  sd.ll <- list( statistic = sd( simplify2array( sapply( x, '[[', "ll" )[ "statistic", ] ), na.rm = TRUE ),
                 p.value = sd( simplify2array( sapply( x, '[[', "ll" )[ "p.value", ] ), na.rm = TRUE ) )
  sd.ad <- list( statistic = sd( simplify2array( sapply( x, '[[', "ad" )[ "statistic", ] ), na.rm = TRUE ),
                 p.value = sd( simplify2array( sapply( x, '[[', "ad" )[ "p.value", ] ), na.rm = TRUE ) )
  sd.ac <- list( t1 = sd( simplify2array( sapply( x, '[[', "ac" )[ "t1", ] ), na.rm = TRUE ),
                 t2 = sd( simplify2array( sapply( x, '[[', "ac" )[ "t2", ] ), na.rm = TRUE ) )

  sd.x <- list( avg = sd.avg, sd = sd.sd, subbo = sd.subbo,
                jb = sd.jb, ll = sd.ll, ad = sd.ad, ac = sd.ac  )

  # build list with Monte Carlo std. error of runs for selected variables
  se.avg <- se( sapply( x, '[[', "avg" ) )
  se.sd <- se( sapply( x, '[[', "sd" ) )
  se.subbo <- list( b = se( simplify2array( sapply( x, '[[', "subbo" )[ "b", ] ) ),
                    a = se( simplify2array( sapply( x, '[[', "subbo" )[ "a", ] ) ),
                    m = se( simplify2array( sapply( x, '[[', "subbo" )[ "m", ] ) ) )
  se.jb <- list( statistic = se( simplify2array( sapply( x, '[[', "jb" )[ "statistic", ] ) ),
                 p.value = se( simplify2array( sapply( x, '[[', "jb" )[ "p.value", ] ) ) )
  se.ll <- list( statistic = se( simplify2array( sapply( x, '[[', "ll" )[ "statistic", ] ) ),
                 p.value = se( simplify2array( sapply( x, '[[', "ll" )[ "p.value", ] ) ) )
  se.ad <- list( statistic = se( simplify2array( sapply( x, '[[', "ad" )[ "statistic", ] ) ),
                 p.value = se( simplify2array( sapply( x, '[[', "ad" )[ "p.value", ] ) ) )
  se.ac <- list( t1 = se( simplify2array( sapply( x, '[[', "ac" )[ "t1", ] ) ),
                 t2 = se( simplify2array( sapply( x, '[[', "ac" )[ "t2", ] ) ) )

  se.x <- list( avg = se.avg, sd = se.sd, subbo = se.subbo,
                jb = se.jb, ll = se.ll, ad = se.ad, ac = se.ac  )

  return( list( avg = avg.x, sd = sd.x, se =  se.x ) )
}


#
# ====== function [] = plot_lognorm ======
# Output:
#  rank-size plot against Lognormal distribution
#
# Input:
#   x: list of vectors of data (not in log!)
#   xlab, ylab: label of x and y axes
#   tit, subtit: title/subtitle of the plot
#   outLim: limit for outliers (0=nolimit)
#	bins: number of bins to use in histogram
#	leg: experiments legends
#	col, lty, pty: colors, line and point types
#

plot_lognorm <- function( x, xlab = "", ylab = "", tit, subtit = "",
                          outLim = 0, bins = 15, leg = NULL,
                          col = NULL, lty = NULL ) {

  if( ! is.list( x ) )
    x <- list( x )

  nExp <- length( x )

  # fill default values
  if( is.null( leg ) )
    leg <- 1 : nExp
  if( is.null( col ) )
    col <- rep( "black", nExp )
  if( is.null( lty ) )
    lty <- rep( "solid", nExp )

  yMax <- xMax <- -Inf
  yMin <- xMin <- Inf
  xx <- rank <- fit <- list( )

  # compute statistics and find the right plot scale
  for( k in 1 : nExp ) {

    # remove NAs and limit number of points to make pdf's lighter
    xx[[ k ]] <- as.vector( x[[ k ]][ ! is.na( x[[ k ]] ) & x[[ k ]] > 0 ] )
    if( length( xx[[ k ]] ) > maxSample )
      xx[[ k ]] <- sample( xx[[ k ]], maxSample )

    # filter in ascending order
    xx[[ k ]] <- sort( xx[[ k ]] )
    iniLen <- length( xx[[ k ]] )        # initial number of observations
    if( outLim > 0 )                     # remove outliers
      xx[[ k ]] <- xx[[ k ]][ nOutliers( xx[[ k ]], outLim ) ]
    n <- length( xx[[ k ]] )             # final number of observations
    if( n == 0 ){
      warning( paste( "No remaining data to plot:", tit, ", series:", k ) )
      return( )
    }
    outPerc <- ( iniLen - n ) / iniLen
    if( outPerc > 2 * outLim )
      warning( "Too many outliers:", outPerc * 100, "%, series:", k )

    rank[[ k ]] <- n - c( 1 : n ) + 1    # rank support for distribution
    mxx <- mean( log( xx[[ k ]] ) )      # parameters of observed data
    sxx <- sd( log( xx[[ k ]] ) )

    # compute fitted theoretical distribution
    fit[[ k ]] <- n * ( 1 - plnorm( xx[[ k ]], meanlog = mxx, sdlog = sxx ) )

    # find y and x limits
    yMax = max( yMax, rank[[ k ]] )
    yMin = min( yMin, rank[[ k ]] )
    xMax = max( xMax, xx[[ k ]] )
    xMin = min( xMin, xx[[ k ]] )
  }

  # adjust margins for legends
  ylim <- findYlim( yMin, yMax, zero = TRUE )

  # now do the plots in the correct scale
  for( k in 1 : nExp ) {

    # plot histogram points
    if( k == 1 )
      plot( xx[[ k ]], rank[[ k ]], log = "xy", type = "p", pch = ".",
            col = col[ k ], main = tit, sub = subtit, xlab = xlab, ylab = ylab,
            xlim = c( xMin, xMax ), ylim = ylim, cex = 2 )
    else
      points( xx[[ k ]], rank[[ k ]], type = "p", pch = ".",
              col = col[ k ], cex = 2 )

    # plot fitted distribution
    lines( xx[[ k ]], fit[[ k ]], lwd = 1, col = col[ k ], lty = lty[ k ] )
  }

  if( nExp > 1 )
    legend( legend = c( leg, "Log-normal fits" ), x = "bottomleft",
            inset = 0.03, adj = 0.1, cex = 0.8,
            lty = c( rep( NA, nExp ), lty[ 1 ] ),
            lwd = c( rep( NA, nExp ), 1 ),
            pch = c( rep( 20, nExp ), NA ),
            col = c( col[ 1 : nExp ], "black" ) )
  else
    legend( legend = c( "Model", "Log-normal fit" ), x = "bottomleft",
            inset = 0.03, adj = 0.1, cex = 0.8,
            lty = c( NA, lty[ 1 ] ), lwd = c( NA, 1 ),
            pch = c( 20, NA ), col = "black" )
}

#
# ====== function [] = plot_norm ======
# Output:
#  binned densities plot against normal distribution
#
# Input:
#   x: list of vectors of data
#   xlab, ylab: label of x and y axes
#   tit, subtit: title/subtitle of the plot
#   outLim: limit for outliers (0=nolimit)
#	bins: number of bins to use in histogram
#	leg: experiments legends
#	col, lty, pty: colors, line and point types
#

plot_norm <- function( x, xlab = "", ylab = "", tit, subtit = "",
                       outLim = 0, bins = 15, leg = NULL,
                       col = NULL, lty = NULL, pty = NULL ) {

  if( ! is.list( x ) )
    x <- list( x )

  nExp <- length( x )

  # fill default values
  if( is.null( leg ) )
    leg <- 1 : nExp
  if( is.null( col ) )
    col <- rep( "black", nExp )
  if( is.null( lty ) )
    lty <- rep( "solid", nExp )
  if( is.null( pty ) )
    pty <- rep( 4, nExp )

  yMax <- xMax <- -Inf
  yMin <- xMin <- Inf
  xx <- hist <- fit <- list( )

  # compute statistics and find the right plot scale
  for( k in 1 : nExp ) {

    # remove NAs and limit number of points to make pdf's lighter
    xx[[ k ]] <- as.vector( x[[ k ]][ ! is.na( x[[ k ]] ) ] )
    if( length( xx[[ k ]] ) > maxSample )
      xx[[ k ]] <- sample( xx[[ k ]], maxSample )

    xx[[ k ]] <- sort( xx[[ k ]] )       # filter in ascending order
    iniLen <- length( xx[[ k ]] )        # initial number of observations
    if( outLim > 0 )                     # remove outliers
      xx[[ k ]] <- xx[[ k ]][ nOutliers( xx[[ k ]], outLim ) ]
    n <- length( xx[[ k ]] )             # final number of observations
    if( n == 0 ){
      warning( paste( "No remaining data to plot:", tit, ", series:", k ) )
      return( )
    }
    outPerc <- ( iniLen - n ) / iniLen
    if( outPerc > 2 * outLim )
      warning( "Too many outliers:", outPerc * 100, "%, series:", k )

    hist[[ k ]] <- hist( xx[[ k ]], breaks = bins, plot = FALSE )
    mxx <- mean( xx[[ k ]] )             # parameters of observed data
    sxx <- sd( xx[[ k ]] )

    # compute fitted theoretical distribution
    fit[[ k ]] <- dnorm( xx[[ k ]], mean = mxx, sd = sxx )

    # find y and x limits
    yMax = max( yMax, hist[[ k ]]$density[ hist[[ k ]]$density > 0 ] )
    yMin = min( yMin, hist[[ k ]]$density[ hist[[ k ]]$density > 0 ] )
    xMax = max( xMax, xx[[ k ]] )
    xMin = min( xMin, xx[[ k ]] )
  }

  # adjust margins for legends
  ylim <- findYlim( yMin, yMax, zero = TRUE )

  # now do the plots in the correct scale
  for( k in 1 : nExp ) {

    # plot histogram points
    if( k == 1 )
      plot( hist[[ k ]]$mids, hist[[ k ]]$density, log = "y", type = "p",
            pch = pty[ k ], col = col[ k ], main = tit, sub = subtit,
            xlab = xlab, ylab = ylab, xlim = c( xMin, xMax ), ylim = ylim )
    else
      points( hist[[ k ]]$mids, hist[[ k ]]$density, type = "p",
            pch = pty[ k ], col = col[ k ] )

    # plot fitted distribution
    lines( xx[[ k ]], fit[[ k ]], lwd = 1, col = col[ k ], lty = lty[ k ] )
  }

  if( nExp > 1 )
    legend( x = "topleft", inset = 0.03, adj = 0.1, cex = 0.8,
            legend = c( leg, "Normal fits" ), lty = c( rep( NA, nExp ), lty[ 1 ] ),
            lwd = c( rep( NA, nExp ), 1 ), pch = c( pty[ 1 : nExp ], NA ),
            col = c( col[ 1 : nExp ], "black" ) )
  else
    legend( x = "topleft", legend = c( "Model", "Normal fit" ), inset = 0.03,
            adj = 0.1, cex = 0.8, lty = c( NA, lty[ 1 ] ), lwd = c( NA, 1 ),
            pch = c( pty[ 1 ], NA ), col = "black" )
}

#
# ====== function [] = plot_laplace ======
# Output:
#  binned densities plot against Laplace distribution
#
# Input:
#   x: list of vectors of data
#   xlab, ylab: label of x and y axes
#   tit, subtit: title/subtitle of the plot
#   outLim: limit for outliers (0=nolimit)
#	bins: number of bins to use in histogram
#	leg: experiments legends
#	col, lty, pty: colors, line and point types
#

plot_laplace <- function( x, xlab = "", ylab = "", tit, subtit = "",
                          outLim = 0, bins = 15, leg = NULL,
                          col = NULL, lty = NULL, pty = NULL ) {

  if( ! is.list( x ) )
    x <- list( x )

  nExp <- length( x )

  # fill default values
  if( is.null( leg ) )
    leg <- 1 : nExp
  if( is.null( col ) )
    col <- rep( "black", nExp )
  if( is.null( lty ) )
    lty <- rep( "solid", nExp )
  if( is.null( pty ) )
    pty <- rep( 4, nExp )

  yMax <- xMax <- -Inf
  yMin <- xMin <- Inf
  xx <- hist <- fit <- list( )

  # compute statistics and find the right plot scale
  for( k in 1 : nExp ) {

    # remove NAs and limit number of points to make pdf's lighter
    xx[[ k ]] <- as.vector( x[[ k ]][ ! is.na( x[[ k ]] ) ] )
    if( length( xx[[ k ]] ) > maxSample )
      xx[[ k ]] <- sample( xx[[ k ]], maxSample )

    xx[[ k ]] <- sort( xx[[ k ]] )       # filter in ascending order
    iniLen <- length( xx[[ k ]] )        # initial number of observations
    if( outLim > 0 )                     # remove outliers
      xx[[ k ]] <- xx[[ k ]][ nOutliers( xx[[ k ]], outLim ) ]
    n <- length( xx[[ k ]] )             # final number of observations
    if( n == 0 ){
      warning( paste( "No remaining data to plot:", tit, ", series:", k ) )
      return( )
    }
    outPerc <- ( iniLen - n ) / iniLen
    if( outPerc > 2 * outLim )
      warning( "Too many outliers:", outPerc * 100, "%, series:", k )

    hist[[ k ]] <- hist( xx[[ k ]], breaks = bins, plot = FALSE )
    mxx <- median( xx[[ k ]] )             # parameters of observed data
    sxx <- sum( abs( xx[[ k ]] - mxx ) ) / n

    # compute fitted theoretical distribution
    if( sxx != 0 )
      fit[[ k ]] <- dlaplace( xx[[ k ]], m = mxx, s = sxx )
    else
      fit[[ k ]] <- c( NA )

    # find y and x limits
    yMax = max( yMax, hist[[ k ]]$density[ hist[[ k ]]$density > 0 ] )
    yMin = min( yMin, hist[[ k ]]$density[ hist[[ k ]]$density > 0 ] )
    xMax = max( xMax, xx[[ k ]] )
    xMin = min( xMin, xx[[ k ]] )
  }

  # adjust margins for legends
  ylim <- findYlim( yMin, yMax, zero = TRUE )

  # now do the plots in the correct scale
  for( k in 1 : nExp ) {

    # plot histogram points
    if( k == 1 )
      plot( hist[[ k ]]$mids, hist[[ k ]]$density, log = "y", type = "p",
            pch = pty[ k ], col = col[ k ], main = tit, sub = subtit,
            xlab = xlab, ylab = ylab, xlim = c( xMin, xMax ), ylim = ylim )
    else
      points( hist[[ k ]]$mids, hist[[ k ]]$density, type = "p",
              pch = pty[ k ], col = col[ k ] )

  # plot fitted distribution, if available
	if( ! is.na( fit[[ k ]][ 1 ] ) )
    lines( xx[[ k ]], fit[[ k ]], lwd = 1, col = col[ k ], lty = lty[ k ] )
  }

  if( nExp > 1 )
    legend( x = "topleft", inset = 0.03, adj = 0.1, cex = 0.8,
            legend = c( leg, "Laplace fits" ), lty = c( rep( NA, nExp ), lty[ 1 ] ),
            lwd = c( rep( NA, nExp ), 1 ), pch = c( pty[ 1 : nExp ], NA ),
            col = c( col[ 1 : nExp ], "black" ) )
  else
    legend( x = "topleft", legend = c( "Model", "Laplace fit" ), inset = 0.03,
            adj = 0.1, cex = 0.8, lty = c( NA, lty[ 1 ] ), lwd = c( NA, 1 ),
            pch = c( pty[ 1 ], NA ), col = "black" )
}


#
# ====== function [] = plot_lists ======
# Output:
#  Time series plot of multiple experiments
#
# Input:
#   exps: list of lists of experiments containing 1 or more series each to plot
#	  min, max, CIlo, CIhi: lists of lists of experiments min, max and confidence limits (1:1 with exps)
#   mrk: plot vertical dotted lin in timestep (only if >0)
#	  leg: vector of experiment names
#	  col, lty: vectors of experiments colors and line types
#   xlab, ylab: label of x and y axes
#   tit, subtit: title/subtitle of the plot
#	  leg2: legends fot type of plots
#

plot_lists <- function( exps, min = NA, max = NA, CIlo = NA, CIhi = NA,
                        mrk = -1, xlab = "", ylab = "", tit = "", subtit = "",
                        leg = NULL, leg2 = NULL, col = NULL, lty = NULL ){

  # All experiments in the same plot
  nExp <- length( exps )
  nPlots <- length( exps[[ 1 ]] )

  # fill default values
  if( is.null( leg ) )
    leg <- 1 : nExp
  if( is.null( leg2 ) )
    leg2 <- rep( "", nExp )
  if( is.null( col ) )
    col <- rep( "black", nExp )
  if( is.null( lty ) )
    lty <- rep( "solid", nExp )

  # find y and x limits
  yMax <- xMax <- -Inf
  yMin <- xMin <- Inf
  xM <- xm <- yM <- ym <- array( dim = c( nExp, nPlots ) )

  for( k in 1 : nExp )
    for( j in 1 : length( exps[[k]] ) ){
      # find first and last valid times
      xM[k,j] <- xm[k,j] <- 1
      for( i in 1 : length( exps[[k]][[j]] ) ){
        if( is.finite( exps[[k]][[j]][i] ) ){
          xM[k,j] <- i
        } else {
          if( xM[k,j] == 1 ){
            xm[k,j] <- i + 1
          }
        }
      }

      yM[k,j] <- max( exps[[k]][[j]], na.rm = TRUE )
      ym[k,j] <- min( exps[[k]][[j]], na.rm = TRUE )
      yMax = max( yMax, yM[k,j] )
      yMin = min( yMin, ym[k,j] )
      xMax = max( xMax, xM[k,j] )
      xMin = min( xMin, xm[k,j] )
    }

  # adjust margins for legends
  ylim <- findYlim( yMin, yMax )

  # Plot base frame

  title <- paste( tit, " ( all experiments )" )
  if( mrk > 0 )
    sub <- paste( "vertical dotted line: regime change /", subtit )
  else
    sub <- subtit

  plot( x = c( xMin : xMax ), y = exps[[1]][[1]][ xMin : xMax ], type = "l",
        main = title, sub = paste( "(", sub, ")" ), xlab = xlab, ylab = ylab,
        col = col[1], lty = lty[1], ylim = ylim )

  if( nExp > 1 )
    for( k in 2 : nExp )
      lines( x = c( xm[k,1] : xM[k,1] ), y = exps[[k]][[1]][ xm[k,1] : xM[k,1] ],
             col = col[k], lty = lty[k] )

  if( nPlots > 1 )
    for( k in 1 : nExp )
      for( j in 2 : nPlots )
        if( lty[ 1 ] == lty[ length( lty ) ] )
          lines( x = c( xm[k,j] : xM[k,j] ), y = exps[[k]][[j]][ xm[k,j] : xM[k,j] ],
                 col = col[k], lty = j )
        else
          lines( x = c( xm[k,j] : xM[k,j] ), y = exps[[k]][[j]][ xm[k,j] : xM[k,j] ],
                 col = col[k], lty = lty[k], lwd = j )

  # plot regime transition mark
  if( mrk > 0 )
    lines( x = c( mrk, mrk ), y = ylim, lty = 3, col = "black" )

  legend( x = "topleft", legend = leg, inset = 0.03, cex = 0.8,
          lty = lty, lwd = 2, col = col )
  if( nPlots > 1 )
    if( lty[ 1 ] == lty[ length( lty ) ] )
      legend( x = "topright", legend = leg2, inset = 0.03,
              cex = 0.8, lty = 1 : 6, lwd = 2 )
    else
      legend( x = "topright", legend = leg2, inset = 0.03,
              cex = 0.8, lty = 1, lwd = 1 : nPlots )

  # Each experiment averages with confidence and max/min intervals

  for( k in 1 : nExp ){

    # find y limits
    xMax <- max( xM[k, ] )
    xMin <- min( xm[k, ] )
    yMax <- max( yM[k, ] )
    yMin <- min( ym[k, ] )

    # adjust margins for legends
    ylim <- findYlim( yMin, yMax )

    title <- paste( tit, "(", leg[k], ")" )
    subTitle <- as.expression( bquote( paste( "( ", .(CI * 100),
                                              "% confidence band in gray, min/max values in light gray / ",
                                              .(sub), " )" ) ) )
    plot( x = c( xMin : xMax ), y = exps[[k]][[1]][ xMin : xMax ], type = "l",
          main = title, sub = subTitle, xlab = xlab, ylab = ylab, ylim = ylim )

    # Plot max/min area first for all series in experiment
    if( length( min ) == nExp && length( max ) == nExp )
      for( j in 1 : length( exps[[k]] ) )
        if( length( min[[k]][[j]] ) == length( exps[[k]][[j]] ) &&
            length( max[[k]][[j]] ) == length( exps[[k]][[j]] ) )
          polygon( c( xm[k,j] : xM[k,j], xM[k,j] : xm[k,j] ),
                   c( pmin( max[[k]][[j]][ xm[k,j] : xM[k,j] ], ylim[ 2 ], na.rm = TRUE ),
                      rev( pmax( min[[k]][[j]][ xm[k,j] : xM[k,j] ], ylim[ 1 ], na.rm = TRUE ) ) ),
                   col = "gray90", border = NA )

    # Then plot confidence interval area for all series
    if( length( CIlo ) == nExp && length( CIhi ) == nExp )
      for( j in 1 : length( exps[[k]] ) )
        if( length( CIhi[[k]][[j]] ) == length( exps[[k]][[j]] ) &&
            length( CIlo[[k]][[j]] ) == length( exps[[k]][[j]] ) )
          polygon( c( xm[k,j] : xM[k,j], xM[k,j] : xm[k,j] ),
                   c( pmin( CIhi[[k]][[j]][ xm[k,j] : xM[k,j] ], ylim[ 2 ], na.rm = TRUE ),
                      rev( pmax( CIlo[[k]][[j]][ xm[k,j] : xM[k,j] ], ylim[ 1 ], na.rm = TRUE ) ) ),
                   col = "gray70", border = NA )

    # And finally plot the series lines, on top of all
    for( j in 1 : length( exps[[k]] ) )
		lines( x = c( xm[k,j] : xM[k,j] ), y = exps[[k]][[j]][ xm[k,j] : xM[k,j] ], lty = j )

    # plot regime transition mark
    if( mrk > 0 )
      lines( x = c( mrk, mrk ), y = ylim, lty = 3, col = "black" )

    if( nPlots > 1 )
      legend( x = "topright", inset = 0.03, cex = 0.8, legend = leg2,
             lty = c( 1 : 5 ), lwd = 2, col = "black" )
  }
}


#
# ====== function [] = plot_bpf ======
# Output:
#   BK band-pass filtered time series plot of multiple experiments
#
# Input:
#   x: list of experiments containing 1 time series each
#   resc: vextor indicating rescaling factors for experiments (NA=no rescale)
#   pl, pu, nfix: BK band-pass filter parameters
#   mask: vector with the time mask to plot
#   mrk: plot vertical dotted lin in timestep (only if >0)
#	  leg: vector of experiment names
#	  col, lty: vectors of experiments colors and line types
#   xlab, ylab: label of x and y axes
#   tit, subtit: title/subtitle of the plot
#

plot_bpf <- function( x, pl, pu, nfix, mask, resc = NA, mrk = -1,
                      xlab = "", ylab = "", tit = "", subtit = "",
                      leg = NULL, col = NULL, lty = NULL ) {

  if( ! is.list( x ) )
    x <- list( x )

  nExp <- length( x )

  # fill default values
  if( is.null( leg ) )
    leg <- 1 : nExp
  if( is.null( col ) )
    col <- rep( "black", nExp )
  if( is.null( lty ) )
    lty <- rep( "solid", nExp )

  # produce bpf-filtered series & find max plot values
  x.plot <- list()
  maxX <- scaleY <- vector( "numeric" )
  for( k in 1 : nExp ) {
    bpf <- bkfilter( x[[ k ]], pl = pl, pu = pu, nfix = nfix )
    x.plot[[ k ]] <- bpf$cycle[ mask ][ ! is.nan( bpf$cycle[ mask ] ) ]
    maxX[ k ] <- max( abs( x.plot[[ k ]] ), na.rm = TRUE )
  }
  maxY <- max( maxX, na.rm = TRUE )
  for( k in 1 : nExp )
    if( is.na( resc[ k ] ) || resc[ k ] <= 0 )
      scaleY[ k ] <- 1                  # no rescaling
  else
    scaleY[ k ] <- resc[ k ] * maxY / maxX[ k ]

  # adjust margins for legends
  ylim <- findYlim( - maxY, maxY )

  # plot frame & first experiment
  plot( x.plot[[ 1 ]] * scaleY[ 1 ], ylim = ylim, type = "l",
        col = col[ 1 ], lty = lty[ 1 ], main = tit, sub = subtit,
        xlab = xlab, ylab = ylab )

  # plot the rest
  if( nExp > 1 )
    for( k in 2 : nExp )
      lines( x.plot[[ k ]] * scaleY[ k ], col = col[ k ], lty = lty[ k ] )

  # plot regime transition mark
  if( mrk > 0 )
    lines( x = c( mrk, mrk ), y = ylim, lty = 3, col = "black" )

  legend( x = "bottomleft", inset = 0.0, cex = 0.4,
          legend = leg, lty = lty, lwd = 2, col = col )
}


#
# ====== function [] = plot_xy ======
# Output:
#   X-Y plot of 2 series, trend line and extremes control
#
# Input:
#   x, y: series to plot
#   na.rm: remove NAs from plot
#	  outLim: outliers quantile limit (0=none)
#   xlab, ylab: label of x and y axes
#   tit, subtit: title/subtitle of the plot
#

plot_xy <- function( x, y, quant = 0.25, xlab = "",
                     ylab = "", tit = "", subtit = "" ){

  # build data analysis matrix, samples in rows
  xy <- cbind( x, y )

  x <- remove_extremes( x, quant = quant, na.rm = FALSE )
  y <- remove_extremes( y, quant = quant, na.rm = FALSE )

  mX <- abs_max( x, quant = quant )
  mY <- abs_max( y, quant = quant )

  # adjust margins for legends
  ylim <- findYlim( - mY, mY )

  plot( x, y, xlim = c( -mX, mX ), ylim = ylim,
        main = tit, sub = paste( "(", subtit, ")" ),
        xlab = xlab, ylab = ylab )

  regLine <- lm( y ~ x )
  if( ! ( TRUE %in% is.na( regLine$coefficients ) ) )
    abline( regLine, col = "gray70" )

  legend( x = "topright", inset = 0.03, cex = 0.8, adj = 0.1,
          legend = c( sprintf( "Adj. R-squared = %.2f",
                               summary( regLine ) $ adj.r.squared ),
                      sprintf( "Coeff. b = %.4f",
                               regLine $ coefficients[2] ) ) )
}


#
# ====== function [] = plot_recovery ======
# Output:
#   Time series plot of marked crises
#
# Input:
#   x: time series to plot (not in logs)
#	  growth: time series growth rate (log first difference)
#	  strt: vector of crises starts
#	  dur: vector of crises durations
#   per: pre-crisis period to base trend start (>=1)
#   mask: vector with the time mask to plot
#   warm: warm-up time (not to plot) (0=none)
#   mrk: plot vertical dotted lin in timestep (only if >0)
#   xlab, ylab: label of x and y axes
#   tit, subtit: title/subtitle of the plot
#

plot_recovery <- function( x, growth, strt, dur, per, mask, warm = 0, mrk = -1,
                           xlab = "", ylab = "", tit = "", subtit = "" ) {

  y <- log( x[ mask ] )    # log GDP series

  yMin <- min( y, na.rm = TRUE )
  yMax <- max( y, na.rm = TRUE )

  # adjust margins for legends
  ylim <- findYlim( yMin, yMax )

  # plot base chart
  plot( x = mask - warm, y = y, ylim = ylim, type = "l",
        main = tit, sub = subtit, xlab = xlab, ylab = ylab )

  if( is.na( strt ) || is.na( dur ) )
    return( )

  growthTrend <- hpfilter( growth, smoothing ) $ trend[ , 1 ]

  # mark crisis and plot trend lines
  preCrisisTrend <- c( rep( NA , length( x ) ) )
  for( i in 1 : length( strt ) ) {
    start <- strt[ i ]
    end <- start + dur[ i ]
    polygon( x = c( start, start, end, end ) - warm,
             y = c( rev( ylim ), ylim ),
             col = "gray90", border = NA )

    preCrisisTrend[ start - 1 ] <- log( mean( x[ ( start - per ) : ( start - 1 ) ] ) )
    gTrend <- mean( growthTrend[ ( start - per ) : ( start - 1 ) ], na.rm = TRUE )
    for( t in start : end )
      preCrisisTrend[ t ] <- preCrisisTrend[ t - 1 ] + gTrend

    lines( x = c( start : end ) - warm, y = preCrisisTrend[ start : end ],
           type = "l", lty = "dotted" )
  }

  # replot GDP curve
  lines( x = mask - warm, y = y )

  # plot regime transition mark
  if( mrk > 0 )
    lines( x = c( mrk, mrk ), y = ylim, lty = 3, col = "black" )
}


#
# ====== function [] = plot_lin ======
# Output:
#  x-y scatter plot against linear model
#
# Input:
#   x, y: vectors of data
#   xlab, ylab: label of x and y axes
#   tit, subtit: title/subtitle of the plot
#   invleg: invert legends positions
#

plot_lin <- function( x, y, xlab = "", ylab = "", tit, subtit = "",
                      invleg = FALSE ) {

  plot( x, y, type = "p", pch = 1,
        main = tit, sub = subtit, xlab = xlab, ylab = ylab )

  # fit a line to linear model growth SD = alpha + beta * size
  y.lm <- lm( y ~ x, as.data.frame( cbind( x, y ) ) )
  abline( y.lm, col = "gray" )

  if( invleg ) {
    l1 = "bottomright"
    l2 = "topleft"
  } else {
    l1 = "topright"
    l2 = "bottomleft"
  }

  legend( x = l1, legend = c( "Data", "Linear fit" ),
          inset = 0.03, cex = 0.8, lty = c( 1, 1 ), lwd = 2.5,
          col = c( "black", "gray"), adj = 0.1 )

  legend( x = l2, legend = c( sprintf( "beta = %.4f", coef( summary( y.lm ) )[ 2, 1 ] ),
                              sprintf( "std. err. = %.4f", coef( summary( y.lm ) )[ 2, 2 ] ),
                              sprintf( "p-val. = %.4f", coef( summary( y.lm ) )[ 2, 4 ] ) ,
                              sprintf( "adj. R2 = %.4f", summary( y.lm )$adj.r.squared ) ),
          inset = 0.03, cex = 0.8, adj = 0.1 )
}


#
# ====== function [] = plot_epanechnikov ======
# Output:
#  Epanechnikov non-parametric regression plot against linear regression fit
#
# Input:
#   lFit: linear regression fit object (produced by lm)
#   xlab, ylab: label of x and y axes
#   tit, subtit: title/subtitle of the plot
#

plot_epanechnikov <- function( lFit, ekOrd = 4, CI = 0.95, xlab = "", ylab = "",
                               tit = "", subtit = "" ) {

  # redo the fitting because of a bug in np package
  # npreg & npplot must be executed in sequence
  nlFit <- npreg( as.formula( lFit$call[[ "formula" ]] ), data = lFit$model,
                  ckerorder = ekOrd, ckertype = "epanechnikov" )

  # get non-parametric results and bootstrap standard errors
  nlPlot <- plot( nlFit, plot.behavior = "data", plot.errors.method = "bootstrap",
                  plot.errors.quantiles = c( ( 1 - CI ) / 2, 1 - ( 1 - CI ) / 2 ) )
  xyVal <- data.frame( cbind( nlPlot$r1$eval[ , 1 ],
                              nlPlot$r1$mean,
                              nlPlot$r1$mean + nlPlot$r1$merr[ , 1 ],
                              nlPlot$r1$mean + nlPlot$r1$merr[ , 2 ]) )
  colnames( xyVal ) <- c( "x", "y", "ciLo", "ciHi" )
  xyVal <- subset( xyVal, x > 0.01 & y > 0.01 & ciLo > 0.01 & ciHi > 0.01 )

  # plot base chart
  plot( xyVal$x, xyVal$y, type = "l", col = "white",
        xlim = c( max( 0, min( xyVal$x ) ), max( xyVal$x ) ),
        ylim = c( max( 0, min( xyVal$ciLo ) ), max( xyVal$ciHi ) ),
        xlab = xlab, ylab = ylab )

  # plot confidence interval area
  polygon( c( xyVal$x, rev( xyVal$x ) ),
           c( xyVal$ciLo, rev( xyVal$ciHi ) ),
           col = "gray80", border = NA )

  # plot data
  # points( lFit$model, pch="." )

  # plot regression lines
  lines( xyVal[ , 1 : 2 ], type = "l", lwd = 2 )
  abline( lFit, lty = 2 )

  # plot regression info boxes and legends
  legend( x = "topleft", legend = c( paste( "Non-parametric",
                                            sprintf( "(R2 = %.2f)",
                                                     nlFit$R2 ) ),
                                     paste( "Parametric",
                                            sprintf( "(R2 = %.2f)",
                                                     summary( lFit )$r.squared ) ) ),
          inset = 0.03, cex = 0.8, lty = c( 1, 2 ), lwd = c( 2, 1 ), adj = 0.05 )

  legend( x = "bottomright", legend = c( "Parametric fit:",
                                         sprintf( "beta = %.4f",
                                                  coef( summary( lFit ) )[ 2, 1 ] ),
                                         sprintf( "(s.e.) = %.4f",
                                                  coef( summary( lFit ) )[ 2, 2 ] ),
                                         sprintf( "(p-value) = %.4f",
                                                  coef( summary( lFit ) )[ 2, 4 ] ) ),
          inset = 0.03, cex = 0.8, adj = 0.1 )

  title( main = tit, sub = subtit )
}


#
# ====== function [] = size_bins ======
# Output:
#   Statistic bins grouped by firm size s
#
# Input:
#   s: sizes of firms in t (in logs)
#   s: sizes of firms in t-1 (in logs)
#	  g: size growth rates from t-1 to t
#	  bins: number of bins to use
#	  outLim: absolute outliers limit for growth rate (0=none)
#

size_bins <- function( s, sLag, g, bins = 30, outLim = 0 ) {

  # build data analysis matrix, samples in rows
  xx <- cbind( s, sLag, g )

  # resample, reducing number of samples
  if( nrow( xx ) > 10 * maxSample )
    xx <- xx[ sample( nrow( xx ), 10 * maxSample ), ]

  # remove outliers
  if( outLim > 0 )
    xx <- remove_outliers_table( xx, quant = outLim )

  # sort by size in t/t-1 (test scaling variance/Gibrat)
  xx1 <- xx[ order( xx[ , 1 ] ), , drop = FALSE ]
  xx2 <- xx[ order( xx[ , 2 ] ), , drop = FALSE ]

  # organize data set into bins by size in t/t-1
  bins1 <- hist( xx[ , 1 ], breaks = bins, plot = F ) # define bins limits
  bins2 <- hist( xx[ , 2 ], breaks = bins, plot = F )

  sAvg1 <- sAvg2 <- sLagAvg <- gAvg <- gSD <- vector( "numeric" )

  # do it first for Gibrat stats (reference is s(t-1))
  j <- 1                                 # start reading first obs
  for( i in 1 : length( bins1$mids ) ) { # for all bins
    # create empty matrix for bins
    set <- matrix( nrow = 0, ncol = 3 )

    # scan till the end of dataset or for all members of bin
    while( j <= nrow( xx1 ) && xx1[ j, 1 ] < bins1$breaks[ i + 1 ] ) {
      # add observation to the bin and move to next obs
      set <- rbind( set, xx1[ j, ], deparse.level = 0, make.row.names = FALSE )
      j <- j + 1
    }

    # calculate average of size (t/t-1), growth rate and SD of growth for each bin
    sAvg1 <- append( sAvg1, mean( set[ , 1 ] ) )
    sLagAvg <- append( sLagAvg, mean( set[ , 2 ] ) )
  }

  # then redo it for scaling variance stats (reference is s(t))
  j <- 1                                 # start reading first obs
  for( i in 1 : length( bins2$mids ) ) { # for all bins
    # create empty matrix for bins
    set <- matrix( nrow = 0, ncol = 3 )

    # scan till the end of dataset or for all members of bin
    while( j <= nrow( xx2 ) && xx2[ j, 2 ] < bins2$breaks[ i + 1 ] ) {
      # add observation to the bin and move to next obs
      set <- rbind( set, xx2[ j, ], deparse.level = 0, make.row.names = FALSE )
      j <- j + 1
    }

    # calculate average of size (t/t-1), growth rate and SD of growth for each bin
    sAvg2 <- append( sAvg2, mean( set[ , 1 ] ) )
    gAvg <- append( gAvg, mean( set[ , 3 ] ) )
    gSD <- append( gSD, log( sd( set[ , 3 ] ) ) )
  }

  return( list( sAvg1 = sAvg1, sAvg2 = sAvg2, sLagAvg = sLagAvg, gAvg = gAvg, gSD = gSD ) )
}
