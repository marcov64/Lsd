#******************************************************************
#
# ------------- Plot, fit and support functions -----------------
#
#   Written by Marcelo C. Pereira, University of Campinas
#
#   Copyright Marcelo C. Pereira
#   Distributed under the GNU General Public License
#
#   Script used by other scripts.
#   This script should not be executed directly.
#
#******************************************************************

# ==== User parameters ====

useSubbotools   <- TRUE       # use Subbotools (T) or normalp package (F)
subboMaxSample  <- 5000       # maximum sample size in Subbotin fits (speed control)
subboMinSample  <- 20         # minimum sample size in Subbotin fits (signif. control)
subboBlimit     <- 5          # maximum limit for b to be considered valid (0=no limit)
maxSample       <- 10000      # maximum sample size in plots (pdf control)
topMargin       <- 0.2        # top plot margin scaling factor
botMargin       <- 0.1        # bottom plot margin scaling factor
def.digits      <- 4          # default number of digits after comma for printing


# ==== Required libraries (order is relevant!) ====

suppressPackageStartupMessages( require( LSDinterface, warn.conflicts = FALSE ) )
suppressPackageStartupMessages( require( LSDsensitivity, warn.conflicts = FALSE ) )
suppressPackageStartupMessages( require( abind, warn.conflicts = FALSE ) )
suppressPackageStartupMessages( require( normalp, warn.conflicts = FALSE ) )
suppressPackageStartupMessages( require( rmutil, warn.conflicts = FALSE ) )
suppressPackageStartupMessages( require( nortest, warn.conflicts = FALSE ) )
suppressPackageStartupMessages( require( gplots, warn.conflicts = FALSE ) )
suppressPackageStartupMessages( require( plotrix, warn.conflicts = FALSE ) )
suppressPackageStartupMessages( require( parallel, warn.conflicts = FALSE ) )
suppressPackageStartupMessages( require( textplot, warn.conflicts = FALSE ) )
suppressPackageStartupMessages( require( corrplot, warn.conflicts = FALSE ) )
suppressPackageStartupMessages( require( matrixStats, warn.conflicts = FALSE ) )
suppressPackageStartupMessages( require( tseries, warn.conflicts = FALSE ) )
suppressPackageStartupMessages( require( np, warn.conflicts = FALSE ) )
suppressPackageStartupMessages( require( extrafont, warn.conflicts = FALSE ) )
suppressPackageStartupMessages( require( mFilter, warn.conflicts = FALSE ) )

# check minimum required versions
if( packageVersion( "LSDinterface" ) < "1.2.1" )
  stop( "Please update LSDinterface package to current version" )
if( packageVersion( "LSDsensitivity" ) < "1.2.1" )
  stop( "Please update LSDsensitivity package to current version" )

# remove warnings for support functions
# !diagnostics suppress = paramp


# ==== Basic functions ====

#
# ====== function [] = all.NA ======
#
# Test if all elements in a matrix/dataframe row are NA
#
# Output:
#   TRUE if all elements are NA
#
# Input:
#   x : vector/matrix/data frame to test
#

all.NA <- function( x ) {
  apply( x, 1, function( x ) all( is.na( x ) ) )
}


#
# ====== function [] = is.nan.data.frame ======
#
# Expand is.nan to handle data frames
#
# Output:
#   TRUE/FALSE vector/matrix/data frame
#
# Input:
#   x : vector/matrix/data frame to test
#

is.nan.data.frame <- function( x ) {
  do.call( cbind, lapply( x, is.nan ) )
}


#
# ====== function [] = is.nan.data.frame ======
#
# Expand is.finite to handle data frames
#
# Output:
#   TRUE/FALSE vector/matrix/data frame
#
# Input:
#   x : vector/matrix/data frame to test
#

is.finite.data.frame <- function( x ) {
  do.call( cbind, lapply( x, is.finite ) )
}


#
# ====== function [] = logNA ======
#
# Redefine log with NA instead of -Inf
#
# Output:
#   log vector/matrix/data frame
#
# Input:
#   x : vector/matrix/data frame to take log
#

logNA <- function( x ) {
  x[ x <= 0 ] <- NA
  return( log( x ) )
}


#
# ====== function [] = log0 ======
#
# Redefine log with zero floor
#
# Output:
#   log vector/matrix/data frame
#
# Input:
#   x : vector/matrix/data frame to take log
#

log0 <- function( x ) {
  y <- logNA( x )
  y[ is.na( y ) ] <- 0
  return( y )
}


#
# ====== function [] = logX ======
#
# Flexible log function
#
# Output:
#   (log) vector/matrix/data frame
#
# Input:
#   x : vector/matrix/data frame to take log if type > 0
#   type : 0=no log, 1=regular log, 2=log0, 3=logNA
#

logX <- function( x, type ) {
  if( type == 1 )
    logX <- log( x )
  else
    if( type == 2 )
      logX <- log0( x )
    else
      if( type == 3 )
        logX <- logNA( x )
      else
        logX <- x
}


#
# ====== function [] = t.test0 ======
#
# Redefine t-test with to use 0 in case of NaN
#
# Output:
#   t-test p-value
#
# Input:
#   x : vector of data to test
#   mu: average to test against
#   conf.level: confidence level
#

t.test0 <- function( x, mu = 0, conf.level = 0.95 ) {
  x[ is.nan( x ) ] <- 0

  sdx <- sd( x, na.rm = TRUE )
  if( is.na( sdx ) || sdx < 1e-12 )
    return( 0 )

  return( t.test( x, mu = mu, alternative = "greater",
                  conf.level = conf.level, na.action = "na.omit" )$p.value )
}


#
# ====== function [] = se ======
#
# Standard error for a sample
#
# Output:
#   standard error
#
# Input:
#   x : vector of data to use
#   na.rm: remove NAs if TRUE
#

se <- function( x, na.rm = TRUE ) {
  if( na.rm )
    n <- length( x[ ! is.na( x ) ] )
  else
    n <- length( x )

  return( sd( x, na.rm = na.rm ) / sqrt( n ) )
}


#
# ====== function [] = fmt ======
#
# Rounding and formatting numbers for tables
#
# Output:
#   formatted table
#
# Input:
#   x : vector/matrix/data frame to format
#   digits: number of decimal digits
#   signif: number of significant digits
#   scipen: R scientific notation parameter
#

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


#
# ====== function [] = light_color ======
#
# Provide a lighter version of color
#
# Output:
#   corresponding lighter color
#
# Input:
#   color: color to use
#   factor: 0= black / 1=white
#   name = optional name for saving the color
#

light_color <- function( color, factor = 0.7, name = NULL ) {
  if ( factor > 1 || factor < 0 )
    return( color )
  c <- col2rgb(color)
  c <- c + ( 255 - c ) * factor
  light_color <- rgb( t( c ), maxColorValue = 255, names = name )
  invisible( light_color )
}


#
# ====== function [] = transp_color ======
#
# Provide a transparent version of color
#
# Output:
#   corresponding transparent color
#
# Input:
#   color: color to use
#   factor: level of transparency
#   name = optional name for saving the color
#

transp_color <- function( color, factor = 0.5, name = NULL ) {
  rgb <- col2rgb( color )
  transp_color <- rgb( rgb[ 1 ], rgb[ 2 ], rgb[ 3 ], maxColorValue = 255,
                       alpha = ( 1  - factor ) * 255, names = name )
  invisible( transp_color )
}


#
# ====== function [] = findYlim ======
#
# Define plot window y limits
#
# Output:
#   two-value vector with minimum and maximum values for y axis
#
# Input:
#   yMin: minimum value in data
#   yMax: maximum value in data
#   zero: limit bottom margin to zero if TRUE
#
# Environment:
#   botMargin: bottom margin of plot area
#   topMargin: top margin of plot area
#

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


#
# ====== function [] = nCores ======
#
# Determine the number of cores to use
# Bounded to the number of cores available
#
# Output:
#   valid number of cores to use
#
# Input:
#   cores: desired number of cores to use
#   nStats: maximum number of statistics to compute in parallel
#

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


#
# ====== function [] = notIn ======
#
# Test if all strings in a vector are contained in another vector
#
# Output:
#   missing values or an empty string if none
#
# Input:
#   a, b: two vectors of strings
#

notIn <- function( a, b ) {
  res <- c( )
  for( x in a )
    if( ! x %in% b )
      res <- append( res, x )

  return( res )
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
  # default return in case of error
  subboFit <- c( rep( as.numeric( NA ), 7 ) )

  if( type == "asymmetric" )
    command <- "subboafit"
  else
    command <- "subbofit"

  cat( "", as.character( Sys.time( ) ), "->", type, "subbofit, n =", length( x ), "... " )

  outStr <- system2( command, args = "-O 3", input = as.character( x ),
                     stdout = TRUE, stderr = FALSE )
  try( subboFit <- sapply( scan( textConnection ( outStr ), what = character( ), quiet = TRUE ),
                           as.numeric, silent = TRUE ),
       silent = TRUE )

  if( type == "asymmetric" )
    se <- paste( subboFit[ 6 ], subboFit[ 7 ] )
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
#   avg: list of variable names to plot
#	  Pdata, mdata, Mdata, Sdata, cdata, Cdata: lists of lists of experiments
#     statistic (mean or median), min, max, std. dev., conf. interval low and up
#     (all with the same dimensions)
#   stat: type of plot statistic (mean or median)
#   nMC: number of Monte Carlo runs
#   CI: confidence for confidence interval
#   log, log0: log or zero-bounded log to be applied on series
#   mrk: plot vertical dotted lin in timestep (only if >0)
#	  leg: vector of experiment names
#	  col, lty: vectors of experiments colors and line types
#   xlab, ylab: label of x and y axes
#   tit, subtit: title/subtitle of the plot
#	  leg2: legends fot type of plots
#

plot_lists <- function( vars, Pdata, mdata, Mdata, cdata = NULL, Cdata = NULL,
                        nMC, sdMC = NULL, statMC = "mean", mask = NULL,
                        CI = 0.95, log = FALSE, log0 = FALSE, na0 = FALSE,
                        mrk = -1, xlab = "", ylab = "", tit = "", subtit = "",
                        leg = NULL, leg2 = NULL, col = NULL, lty = NULL ) {

  nVar <- length( vars )
  nExp <- length( Pdata )

  # asymptotic distribution approximation factor when no CI is available
  if( statMC == "mean" )
    af <- qt( ( 1 - CI ) / 2, nMC - 1 ) / sqrt( nMC )
  else
    af <- sqrt( pi / ( 2 * nMC ) )  # asymptotic distribution factor

  # fill default values
  if( is.null( mask ) )
    mask <- 1 : length( Pdata[[ 1 ]][[ 1 ]] )
  if( is.null( leg ) )
    leg <- 1 : nExp
  if( is.null( leg2 ) )
    leg2 <- rep( "", nExp )
  if( is.null( col ) )
    col <- rep( "black", nExp )
  if( is.null( lty ) )
    lty <- rep( "solid", nExp )

  # prepare all time series
  plt <- min <- max <- CIlo <- CIhi <- list( )
  for( k in 1 : nExp ) {

    plt[[ k ]] <- min[[ k ]] <- max[[ k ]] <- CIlo[[ k ]] <- CIhi[[ k ]] <- list( )
    for( j in 1 : nVar ) {
      Pdata[[ k ]][ ! is.finite( Pdata[[ k ]] ) ] <- NA
      mdata[[ k ]][ ! is.finite( mdata[[ k ]] ) ] <- NA
      Mdata[[ k ]][ ! is.finite( Mdata[[ k ]] ) ] <- NA

      if( ! is.null( cdata ) )
        cdata[[ k ]][ ! is.finite( cdata[[ k ]] ) ] <- NA

      if( ! is.null( Cdata ) )
        Cdata[[ k ]][ ! is.finite( Cdata[[ k ]] ) ] <- NA

      if( ! is.null( sdMC ) )
        sdMC[[ k ]][ ! is.finite( sdMC[[ k ]] ) ] <- NA

      plt[[ k ]][[ j ]] <- Pdata[[ k ]][ mask, vars[ j ] ]
      min[[ k ]][[ j ]] <- mdata[[ k ]][ mask, vars[ j ] ]
      max[[ k ]][[ j ]] <- Mdata[[ k ]][ mask, vars[ j ] ]

      if( ! is.null( cdata ) )
        CIlo[[ k ]][[ j ]] <- cdata[[ k ]][ mask, vars[ j ] ]
      else {
        if( ! is.null( sdMC ) )
          CIlo[[ k ]][[ j ]] <- plt[[ k ]][[ j ]] - af * sdMC[[ k ]][ mask, vars[ j ] ]
        else
          CIlo[[ k ]][[ j ]] <- NA
      }

      if( ! is.null( Cdata ) )
        CIhi[[ k ]][[ j ]] <- Cdata[[ k ]][ mask, vars[ j ] ]
      else {
        if( ! is.null( sdMC ) )
          CIhi[[ k ]][[ j ]] <- plt[[ k ]][[ j ]] + af * sdMC[[ k ]][ mask, vars[ j ] ]
        else
          CIhi[[ k ]][[ j ]] <- NA
      }

      # apply logs if required
      if( log ) {
        plt[[ k ]][[ j ]] <- logNA( plt[[ k ]][[ j ]] )
        min[[ k ]][[ j ]] <- logNA( min[[ k ]][[ j ]] )
        max[[ k ]][[ j ]] <- logNA( max[[ k ]][[ j ]] )
        CIlo[[ k ]][[ j ]] <- logNA( CIlo[[ k ]][[ j ]] )
        CIhi[[ k ]][[ j ]] <- logNA( CIhi[[ k ]][[ j ]] )
      } else if( log0 ) {
        plt[[ k ]][[ j ]] <- log0( plt[[ k ]][[ j ]] )
        min[[ k ]][[ j ]] <- log0( min[[ k ]][[ j ]] )
        max[[ k ]][[ j ]] <- log0( max[[ k ]][[ j ]] )
        CIlo[[ k ]][[ j ]] <- log0( CIlo[[ k ]][[ j ]] )
        CIhi[[ k ]][[ j ]] <- log0( CIhi[[ k ]][[ j ]] )
      }

      # treat zeros as NAs
      if( na0 && plt[[ k ]][[ j ]] <= 0 ) {
        plt[[ k ]][[ j ]] <- min[[ k ]][[ j ]] <- max[[ k ]][[ j ]] <-
          CIlo[[ k ]][[ j ]] <- CIhi[[ k ]][[ j ]] <- NA
      }
    }
  }

  # find y and x limits
  yMax <- xMax <- -Inf
  yMin <- xMin <- Inf
  xM <- xm <- yM <- ym <- array( dim = c( nExp, nVar ) )

  for( k in 1 : nExp )
    for( j in 1 : length( plt[[k]] ) ){
      # find first and last valid times
      xM[k,j] <- xm[k,j] <- 1
      for( i in 1 : length( plt[[k]][[j]] ) ){
        if( is.finite( plt[[k]][[j]][i] ) ){
          xM[k,j] <- i
        } else {
          if( xM[k,j] == 1 ){
            xm[k,j] <- i + 1
          }
        }
      }

      yM[k,j] <- max( plt[[k]][[j]], na.rm = TRUE )
      ym[k,j] <- min( plt[[k]][[j]], na.rm = TRUE )
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

  plot( x = c( xMin : xMax ), y = plt[[1]][[1]][ xMin : xMax ], type = "l",
        main = title, sub = paste( "(", sub, ")" ), xlab = xlab, ylab = ylab,
        col = col[1], lty = lty[1], ylim = ylim )

  if( nExp > 1 )
    for( k in 2 : nExp )
      lines( x = c( xm[k,1] : xM[k,1] ), y = plt[[k]][[1]][ xm[k,1] : xM[k,1] ],
             col = col[k], lty = lty[k] )

  if( nVar > 1 )
    for( k in 1 : nExp )
      for( j in 2 : nVar )
        if( lty[ 1 ] == lty[ length( lty ) ] )
          lines( x = c( xm[k,j] : xM[k,j] ), y = plt[[k]][[j]][ xm[k,j] : xM[k,j] ],
                 col = col[k], lty = j )
        else
          lines( x = c( xm[k,j] : xM[k,j] ), y = plt[[k]][[j]][ xm[k,j] : xM[k,j] ],
                 col = col[k], lty = lty[k], lwd = j )

  # plot regime transition mark
  if( mrk > 0 )
    lines( x = c( mrk, mrk ), y = ylim, lty = 3, col = "black" )

  legend( x = "topleft", legend = leg, inset = 0.03, cex = 0.8,
          lty = lty, lwd = 2, col = col )
  if( nVar > 1 )
    if( lty[ 1 ] == lty[ length( lty ) ] )
      legend( x = "topright", legend = leg2, inset = 0.03,
              cex = 0.8, lty = 1 : 6, lwd = 2 )
    else
      legend( x = "topright", legend = leg2, inset = 0.03,
              cex = 0.8, lty = 1, lwd = 1 : nVar )

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
    subTitle <- paste0( "( gray: ", CI * 100,
                        "% confidence / light gray: min/max / ", sub, " )" )
    plot( x = c( xMin : xMax ), y = plt[[k]][[1]][ xMin : xMax ], type = "l",
          main = title, sub = subTitle, xlab = xlab, ylab = ylab, ylim = ylim )

    # Plot max/min area first for all series in experiment
    if( length( min ) == nExp && length( max ) == nExp )
      for( j in 1 : length( plt[[k]] ) )
        if( length( min[[k]][[j]] ) == length( plt[[k]][[j]] ) &&
            length( max[[k]][[j]] ) == length( plt[[k]][[j]] ) )
          polygon( c( xm[k,j] : xM[k,j], xM[k,j] : xm[k,j] ),
                   c( pmin( max[[k]][[j]][ xm[k,j] : xM[k,j] ], ylim[ 2 ], na.rm = TRUE ),
                      rev( pmax( min[[k]][[j]][ xm[k,j] : xM[k,j] ], ylim[ 1 ], na.rm = TRUE ) ) ),
                   col = "gray90", border = NA )

    # Then plot confidence interval area for all series
    if( length( CIlo ) == nExp && length( CIhi ) == nExp )
      for( j in 1 : length( plt[[k]] ) )
        if( length( CIhi[[k]][[j]] ) == length( plt[[k]][[j]] ) &&
            length( CIlo[[k]][[j]] ) == length( plt[[k]][[j]] ) )
          polygon( c( xm[k,j] : xM[k,j], xM[k,j] : xm[k,j] ),
                   c( pmin( CIhi[[k]][[j]][ xm[k,j] : xM[k,j] ], ylim[ 2 ], na.rm = TRUE ),
                      rev( pmax( CIlo[[k]][[j]][ xm[k,j] : xM[k,j] ], ylim[ 1 ], na.rm = TRUE ) ) ),
                   col = "gray70", border = NA )

    # And finally plot the series lines, on top of all
    for( j in 1 : length( plt[[k]] ) )
		lines( x = c( xm[k,j] : xM[k,j] ), y = plt[[k]][[j]][ xm[k,j] : xM[k,j] ], lty = j )

    # plot regime transition mark
    if( mrk > 0 )
      lines( x = c( mrk, mrk ), y = ylim, lty = 3, col = "black" )

    if( nVar > 1 )
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

  legend( x = "topleft", inset = 0.03, cex = 0.8,
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

  y <- log0( x[ mask ] )    # log series

  yMin <- min( y, na.rm = TRUE )
  yMax <- max( y, na.rm = TRUE )

  # adjust margins for legends
  ylim <- findYlim( yMin, yMax )

  # plot base chart
  plot( x = mask - warm, y = y, ylim = ylim, type = "l",
        main = tit, sub = subtit, xlab = xlab, ylab = ylab )

  if( is.null( strt ) || is.null( dur ) )
    return( )

  growthTrend <- hpfilter( growth, smoothing ) $ trend[ , 1 ]

  # mark crisis and plot trend lines
  preCrisisTrend <- c( rep( NA , length( x ) ) )
  for( i in 1 : length( strt ) ) {

    start <- strt[ i ]
    end <- start + dur[ i ]

    if( is.na( end ) )
      next

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

  # replot curve
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

  if( length( x ) == 0 || length( y ) == 0 ) {
    warning( "Zero x and/or y observations, cannot plot" )
    return( )
  }

  x[ ! is.finite( x ) ] <- NA
  y[ ! is.finite( y ) ] <- NA

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
# ====== function [] = plot_histo ======
# Output:
#   Plot stacked histogram/density at selected times
#
# Input:
#   times: vector of time cross-sections to use
#	  data: Monte Carlo experiment table (time x mc)
#   bins: number of bins in histogram
#   log: take log of variable (0=no log, 1=log, 2=log0, 3=logNA)
#   labVar: text label for variable
#   bw.adj: smoothing bandwidth adjustment
#   leg: vector of text legends to each cross-section
#   tit, subtit: title/subtitle of the plot
#

plot_histo <- function( times, data, bins = 10, log = 0, labVar = NULL,
                        bw.adj = 1, leg = NULL, tit = "", subtit = "" ) {

  nCS <- length( times )
  cs <- logX( data[ times, ], log )

  if( is.null( labVar ) )
    labVar <- var
  if( is.null( leg ) || length( leg ) != nCS )
    leg <- rep( "", nCS )

  # find histogram limits
  xMin <- min( cs, na.rm = TRUE )
  xMax <- max( cs, na.rm = TRUE )

  # define bin breaks
  breaks <- c( )
  incr <- ( xMax - xMin ) / bins
  for( i in 0 : bins )
    breaks <- c( breaks, xMin + i * incr )

  yMax <- 0
  for( i in 1 : nCS ) {
    d <- tryCatch( density( cs[ i, ], bw = "SJ", adjust = bw.adj, na.rm = TRUE )$y,
                   error = function( cond ) return( NA ) )
    h <- tryCatch( hist( cs[ i, ], breaks = breaks, plot = FALSE )$density,
                   error = function( cond ) return( NA ) )
    yMax <- max( yMax, d, h )
  }

  # change the output format but save existing conf to restore at end
  oldPar <- par( )
  par( mfrow = c( nCS, 1 ), oma = c( 2.5, 2, 3.5, 2 ), mar = c( 4, 4, 0, 2 ) )

  # plot all
  for( i in 1 : nCS ) {
    if( ! is.na( yMax ) ) {
      h <- hist( cs[ i, ], prob = TRUE, breaks = breaks, col = NULL,
                 main = "", xlab = paste0( labVar, " (", leg[ i ], ")" ),
                 xlim = c( xMin, xMax ), ylim = c( 0, 1.1 * yMax ) )
      d <- density( cs[ i, ], bw = "SJ", adjust = bw.adj, na.rm = TRUE )
      polygon( d, col = transp_color( "gray", 0.7 ) )
      xAvg <- mean( cs[ i, ], na.rm = TRUE )
      yAvg <- 1.1 * max( h$density, d$y )
      lines( c( xAvg, xAvg ), c( 0, yAvg ), type = "l", lty = "dotted" )
    } else {
      textplot( "Insufficient data to plot", cex = 1.0 )
    }
  }

  mtext( tit, outer = TRUE, cex = 1.2, font = 2, padj = -1 )
  mtext( subtit, side = 1, outer = TRUE, padj = 2 )

  par( mfrow = oldPar$mfrow, oma = oldPar$oma, mar = oldPar$mar )
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
  xx <- na.omit( cbind( s, sLag, g ) )

  # resample, reducing number of samples
  if( nrow( xx ) > 10 * maxSample )
    xx <- xx[ sample( nrow( xx ), 10 * maxSample ), ]

  # remove outliers
  if( outLim > 0 )
    xx <- remove_outliers_table( xx, quant = outLim )

  # sort by size in t/t-1 (test scaling variance/Gibrat)
  xx1 <- xx[ order( xx[ , 1 ] ), , drop = FALSE ]
  xx2 <- xx[ order( xx[ , 2 ] ), , drop = FALSE ]

  if ( nrow( xx ) < bins ) {
    warning( "Fewer observations than bins, returning NA." )
    return( NA )
  }

  # organize data set into bins by size in t/t-1
  bins1 <- hist( xx[ , 1 ], breaks = bins, plot = F ) # define bins limits
  bins2 <- hist( xx[ , 2 ], breaks = bins, plot = F )

  s1avg <- s2avg <- sLagAvg <- gAvg <- gSD <- vector( "numeric" )

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
    s1avg <- append( s1avg, mean( set[ , 1 ] ) )
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
    s2avg <- append( s2avg, mean( set[ , 1 ] ) )
    gAvg <- append( gAvg, mean( set[ , 3 ] ) )
    gSD <- append( gSD, logNA( sd( set[ , 3 ] ) ) )
  }

  return( list( s1avg = s1avg, s2avg = s2avg, sLagAvg = sLagAvg, gAvg = gAvg, gSD = gSD ) )
}


#
# ====== function [] = growth_stats ======
# Output:
#   Growth statistics table
#
# Input:
#   vars: vector of variable names to include in table
#	  data: Monte Carlo experiment dataset (time x vars x mc)
#   labVars: optional vector of text label for variables
#	  mask: vector of (continuous) range of time steps to use
#	  pl, pu: BK-filter lower/upper band-pass period parameter
#	  nfix: BK-filter order (selectivity)
#   CI: confidence level
#

growth_stats <- function( vars, data, labVars = NULL, mask = NULL,
                          pl = 6, pu = 32, nfix = 12, CI = 0.95 ) {
  nVar <- length( vars )
  nTsteps <- nrow( data )
  nSize <- dim( data )[ 3 ]

  if( is.null( labVars ) || length( labVars ) != nVar )
    labVars <- vars
  if( is.null( mask ) || max( mask ) - min( mask ) + 1 > nTsteps )
    mask <- c( 1 : nTsteps )

  meanPer <- nfix
  maskBpf <- ( nfix + 1 ) : ( max( mask ) - min( mask ) + 1 - nfix )

  # create the stats table
  tab <- matrix( nrow = 13, ncol = nVar )
  colnames( tab ) <- labVars
  rownames( tab ) <- c( "avg. growth rate", " (s.e.)",
                        "ADF test (logs)", " (s.e.)", " (p-val.)", " (s.e.)",
                        "ADF test (bpf)", " (s.e.)", " (p-val.)", " (s.e.)",
                        " s.d. (bpf)", " (s.e.)",
                        paste0( " relative s.d. (", vars[ 1 ], ")" ) )

  # fill the table
  for( i in 1 : nVar ) {
    gVar <- sVar <- rep( NA, nSize )
    dVar <- dfVar <- list( )
    for( j in 1 : nSize ) {

      # MC average growth rates
      start <- mean( log0( data[ mask[ 1 : meanPer ], vars[ i ], j ] ),
                     na.rm = TRUE )
      end <- mean( log0( data[ mask[ ( length( mask ) - meanPer + 1 ) :
                                       length( mask ) ], vars[ i ], j ] ),
                     na.rm = TRUE )
      gVar[ j ] <- ( end - start ) / ( max( mask ) - min( mask ) + 2 - meanPer )

      # Baxter-King filter
      fVar <- bkfilter( log0( data[ mask, vars[ i ], j ] ),
                        pl = pl, pu = pu, nfix = nfix )

      # Augmented Dickey-Fuller tests (unit roots) & standard deviations
      dVar[[ j ]] <- suppressWarnings( adf.test( log0( data[ mask, vars[ i ], j ] ) ) )
      dfVar[[ j ]] <- suppressWarnings( adf.test( fVar$cycle[ maskBpf, 1 ] ) )
      sVar[ j ] <- sd( fVar$cycle[ maskBpf, 1 ] )
    }


    if( i == 1 )
      sRef <- sVar

    tab[ , i ] <- c( mean( gVar ),
                    sd( gVar ) / sqrt( nSize ),
                    mean( unname( sapply( dVar, `[[`, "statistic" ) ) ),
                    sd( unname( sapply( dVar, `[[`, "statistic" ) ) ) / sqrt( nSize ),
                    mean( unname( sapply( dVar, `[[`, "p.value" ) ) ),
                    sd( unname( sapply( dVar, `[[`, "p.value" ) ) ) / sqrt( nSize ),
                    mean( unname( sapply( dfVar, `[[`, "statistic" ) ) ),
                    sd( unname( sapply( dfVar, `[[`, "statistic" ) ) ) / sqrt( nSize ),
                    mean( unname( sapply( dfVar, `[[`, "p.value" ) ) ),
                    sd( unname( sapply( dfVar, `[[`, "p.value" ) ) ) / sqrt( nSize ),
                    mean( sVar ),
                    sd( sVar ) / sqrt( nSize ),
                    mean( sVar ) / mean( sRef ) )
  }

  return( tab )
}


#
# ====== function [] = corr_table ======
# Output:
#   list with MC correlation, standard errors and p-value tables (vars x vars)
#
# Input:
#   vars: vector of variable names to include in table
#	  data: Monte Carlo experiment dataset (time x vars x mc)
#   logVars: vector of variables to take log (0=no log, 1=log, 2=log0)
#   labVars: optional vector of text label for variables
#	  mask: vector of (continuous) range of time steps to use
#	  pl, pu: BK-filter lower/upper band-pass period parameter
#	  nfix: BK-filter order (selectivity)
#   CI: confidence level
#   plot: plot a heatmap fo the table
#   tit, subtit: title/subtitle of the plot
#

corr_table <- function( vars, data, logVars = NULL, labVars = NULL,
                        mask = NULL, pl = 6, pu = 32, nfix = 12, CI = 0.95,
                        plot = FALSE, tit = "", subtit = "" ) {

  nVar <- length( vars )
  nTsteps <- nrow( data )
  nSize <- dim( data )[ 3 ]

  if( is.null( labVars ) || length( labVars ) != nVar )
    labVars <- vars
  if( is.null( logVars ) || length( logVars ) != nVar )
    logVars <- rep( 0, nVar )
  if( is.null( mask ) || max( mask ) - min( mask ) + 1 > nTsteps )
    mask <- c( 1 : nTsteps )

  maskBpf <- ( nfix + 1 ) : ( max( mask ) - min( mask ) + 1 - nfix )
  nTstat <- max( maskBpf ) - min( maskBpf ) + 1

  corr <- pval <- array( dim = c( nVar, nVar, nSize ),
                         dimnames = list( labVars, labVars,
                                          dimnames( data )[[ 3 ]] ) )

  # BK-filter each MC var and build the filtered series matrices
  for( j in 1 : nSize ) {
    mat <- matrix( nrow = nTstat, ncol = nVar )
    for( i in 1 : nVar ) {
      mat[ , i ] <- bkfilter( logX( data[ mask, vars[ i ], j ],
                                    logVars[ i ] ),
                              pl = pl, pu = pu, nfix = nfix )$cycle[ maskBpf, 1 ]

      for( h in 1 : i )
        pval[ i, h, j ] <- pval[ h, i, j ] <- tryCatch( suppressWarnings(
          cor.test( mat[ , i ], mat[ , h ], conf.level = CI )$p.value ), error = function( cond ) return( 1 ) )
    }

    corr[ , , j ] <- suppressWarnings( cor( mat ) )
  }

  mean <- apply( corr, 1 : 2, mean, na.rm = TRUE )
  se <- apply( corr, 1 : 2, se, na.rm = TRUE )
  p.value <- apply( pval, 1 : 2, mean, na.rm = TRUE )
  mean[ is.nan( mean ) ] <- 0
  se[ is.nan( se ) ] <- 0
  p.value[ is.nan( p.value ) ] <- 0

  if( plot ) {
    corrplot( round( mean, 2 ), p.mat = p.value, sig.level = 1 - CI,
              method = "color", type = "lower", title = tit, cl.pos = "n",
              diag = FALSE, mar = c( 2, 2, 2, 2 ), addCoef.col = "black",
              tl.col = "black", number.cex = 0.7, tl.srt = 45, insig = "blank" )

    title( sub = subtit )

    invisible( list( mean = mean, se = se, p.value = p.value ) )

  } else
    return( list( mean = mean, se = se, p.value = p.value ) )
}


#
# ====== function [] = corr_struct ======
# Output:
#   Correlation structure table
#
# Input:
#   ref: reference variable to use
#   vars: vector of variable names to include in table
#	  data: Monte Carlo experiment dataset (time x vars x mc)
#   logRef: take log of reference variable (0=no log, 1=log, 2=log0)
#   logVars: vector of variables to take log (0=no log, 1=log, 2=log0)
#   labRef: optional text label for reference variable
#   labVars: optional vector of text label for variables
#	  mask: vector of (continuous) range of time steps to use
#   lags: correlation lags to use
#	  pl, pu: BK-filter lower/upper band-pass period parameter
#	  nfix: BK-filter order (selectivity)
#   CI: confidence level
#

corr_struct <- function( ref, vars, data, logRef = 0, logVars = NULL,
                         labRef = NULL, labVars = NULL, mask = NULL,
                         lags = 4, pl = 6, pu = 32, nfix = 12, CI = 0.95 ) {
  nVar <- length( vars )
  nTsteps <- nrow( data )
  nSize <- dim( data )[ 3 ]
  nCols <- 2 * lags + 1

  if( is.null( labRef ) )
    labRef <- ref
  if( is.null( labVars ) || length( labVars ) != nVar )
    labVars <- vars
  if( is.null( logVars ) || length( logVars ) != nVar )
    logVars <- rep( 0, nVar )
  if( is.null( mask ) || max( mask ) - min( mask ) + 1 > nTsteps )
    mask <- c( 1 : nTsteps )

  # Calculates the critical correlation limit for significance (under heroic assumptions!)
  maskBpf <- ( nfix + 1 ) : ( max( mask ) - min( mask ) + 1 - nfix )
  nTstat <- max( maskBpf ) - min( maskBpf ) + 1
  critCorr <- qnorm( 1 - ( 1 - CI ) / 2 ) / sqrt( nTstat )

  # compute the correlation structure for each BK-filtered var and MC
  cRef <- fRef <- list( )
  for( j in 1 : nSize ) {
    fRef[[ j ]] <- bkfilter( logX( data[ mask, ref, j ], logRef ),
                             pl = pl, pu = pu, nfix = nfix )
    cRef[[ j ]] <- ccf( fRef[[ j ]]$cycle[ maskBpf, 1 ],
                        fRef[[ j ]]$cycle[ maskBpf, 1 ],
                        lag.max = lags, plot = FALSE, na.action = na.pass )
  }

  cVars <- list( )
  for( i in 1 : nVar ) {
    cVars[[ i ]] <- list( )
    for( j in 1 : nSize ) {
      fVar <- bkfilter( logX( data[ mask, vars[ i ], j ], logVars[ i ] ),
                        pl = pl, pu = pu, nfix = nfix )
      cVars[[ i ]][[ j ]] <- ccf( fRef[[ j ]]$cycle[ maskBpf, 1 ],
                                  fVar$cycle[ maskBpf, 1 ],
                                  lag.max = lags, plot = FALSE, na.action = na.pass )
    }
  }

  # apply t-test to the mean lag results to test significance (H0: lag < critCorr)
  pRef <- rep( NA, nCols )
  for( k in 1 : nCols )
    if( k != lags + 1 )    # no autocorrelation at lag 0
      pRef[ k ] <- t.test0( abs( unname( sapply( cRef, `[[`, "acf" ) )[ k, ] ),
                            critCorr, CI )

  pVars <- list( )
  for( i in 1 : nVar ) {
    pVars[[ i ]] <- rep( NA, nCols )
    for( k in 1 : nCols )
      pVars[[ i ]][ k ] <- t.test0( abs( unname( sapply( cVars[[ i ]],
                                                         `[[`, "acf" ) )[ k, ] ),
                                    critCorr, CI )
  }

  # mount the stats table
  tab <- matrix( nrow = 3 * ( nVar + 1 ), ncol = nCols )
  colnames( tab ) <- cRef[[ 1 ]]$lag

  tab[ 1, ] <- colMeans( t( unname( sapply( cRef, `[[`, "acf" ) ) ),
                            na.rm = TRUE )
  tab[ 2, ] <- colSds( t ( unname( sapply( cRef, `[[`, "acf" ) ) ),
                          na.rm = TRUE ) / sqrt( nSize )
  tab[ 3, ] <- pRef
  rowNames <- c( labRef, " (s.e.)", " (p-val.)" )

  for( i in 1 : nVar ) {
    tab[ i * 3 + 1, ] <- colMeans( t( unname( sapply( cVars[[ i ]], `[[`, "acf" ) ) ),
                           na.rm = TRUE )
    tab[ i * 3 + 2, ] <- colSds( t ( unname( sapply( cVars[[ i ]], `[[`, "acf" ) ) ),
                         na.rm = TRUE ) / sqrt( nSize )
    tab[ i * 3 + 3, ] <- pVars[[ i ]]
    rowNames <- c( rowNames, labVars[ i ], " (s.e.)", " (p-val.)" )
  }

  rownames( tab ) <- rowNames

  return( tab )
}
