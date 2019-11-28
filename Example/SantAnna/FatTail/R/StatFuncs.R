#******************************************************************
#
# ------------- Plot, fit and support functions -----------------
#
#******************************************************************

# ==== User parameters ====

subboMaxSample <- 5000         # maximum sample size in Subbotin fits (speed control)
subboMinSample <- 50           # minimum sample size in Subbotin fits (significance control)
subboBlimit <- 0               # maximum limit for b to be considered valid (0=no limit)
useSubbotools <- TRUE          # use Subbotools (T) or normalp package (F)
useASubbotin <- TRUE           # use symmetric (=F) or asymmetric Subbotin (=T)
subbotoolsFolder <- "subbotools-1.3.0\\"  # subbotools location (leave blank in linux)
useALaplace <- TRUE            # use symmetric (=F) or asymmetric Laplace (=T)
nBins  <- 20                   # number of bins to use in histograms
nSample <- 50                  # sample size for goodness of fit tests
outLim <- 100                  # outlier limit for normalized productivity (absolute)
zeroLim <- 1e-12               # minimum significant value
def.digits <- 4                # default number of digits after comma for printing


# ==== Required libraries (order is relevant!) ====

require( gplots, warn.conflicts = FALSE, quietly = TRUE )
require( LSDinterface, warn.conflicts = FALSE, quietly = TRUE )
require( LSDsensitivity, warn.conflicts = FALSE, quietly = TRUE )
require( LaplacesDemon, warn.conflicts = FALSE, quietly = TRUE )
require( normalp, warn.conflicts = FALSE, quietly = TRUE )
require( robustbase, warn.conflicts = FALSE, quietly = TRUE )
require( minpack.lm, warn.conflicts = FALSE, quietly = TRUE )

if( .Platform$OS.type == "unix" )
  subbotoolsFolder <- ""


# ==== Evaluation function to compute and add the Subbotin b to data ====

# eval.b <- function( dataSet, exp, var, conf ) {
#
#   obs <- discards <- 0
#   bSubbo <- vector( "numeric" )
#
#   # Compute Subbotin fits for each run
#
#   for( i in 1 : dim( dataSet )[ 3 ] ) {
#     fitSubbo <- fit_subbotin( dataSet[[ exp, var, i ]] )
#     # remove non significant results at the selected confidence%
#     if( fitSubbo[ 1 ] / fitSubbo[ 4 ] <
#         qt( ( 1 - conf ) / 2, length( data[[ exp, var, i ]] ), lower.tail = FALSE ) ) {
#       discards <- discards + 1
#       bSubbo[ i ] <- NA
#     } else {
#       obs <- obs + 1
#       bSubbo[ i ] <- fitSubbo[ 1 ]
#     }
#   }
#
#   return( list( mean( bSubbo, na.rm = TRUE ), sd( bSubbo, na.rm = TRUE ), obs, discards ) )
# }

eval.b <- function( dataSet, allVars ) {

  # clean data and fit Subbotin distribution
  data <- as.vector( dataSet )
  data <- data[ is.numeric( data ) ]
  fitSubbo <- fit_subbotin( data )

  # remove non significant results at the selected confidence%
  if( fitSubbo[ 1 ] / fitSubbo[ 4 ] <
      qt( ( 1 - conf ) / 2, length( data ), lower.tail = FALSE ) )
    bSubbo <- NA
  else
    bSubbo <- fitSubbo[ 1 ]

  # add to dataset (all periods)
  dataSet[ , "b" ] <- bSubbo

  return( dataSet )
}


# ==== Auxiliary functions ====

# ---- Rounding and formatting numbers for tables ----
fmt <- function( x, digits = def.digits )
  format( round( x, digits = digits ), nsmall = digits )

# ---- Rounding numbers of mixed type data frames ----
round_df <- function( df, digits ) {
  nums <- vapply( df, is.numeric, FUN.VALUE = logical( 1 ) )
  if( any( nums ) )   # any number to round?
    df[ , nums ] <- round( df[ , nums ], digits = digits )
  return( df )
}

# ---- Standard error for a sample ----
se <- function( x ){
  return( sd( x, na.rm = TRUE ) / sqrt( length( x[ ! is.na( x ) ] ) ) )
}

# ---- remove outliers from Doe/response tables ----
remove_outliers <- function( doe, resp, limit ) {

  origLen <- nrow( doe )
  if( origLen != nrow( resp ) )
    stop( "Design of Experiments and response files do not match!" )

  # check for abnormal DoE sample averages
  m <- mean( resp$Mean )
  d <- sd( resp$Mean ) * limit         # maximum deviation
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
    warning( "Too many DoE outliers (>10%), check 'limit' parameter" )

  return( list( doe, resp, removed ) )
}

# ---- MLE estimation of the parameters of a asymmetric Laplace distribution ----
# using the algorithm presented in Puig & Stephens 2007

delta <- function( mu, x ){
  sum( abs( x - mu ) ) / length( x )
}

psi <- function( mu, x ){
  radix <- delta( mu, x ) ^ 2 - ( mean( x ) - mu ) ^ 2
  if( abs( radix ) < zeroLim )      # discard negative rounding errors
    radix <- 0
  delta( mu, x ) + sqrt( radix )
}

# The parameter mu is estimated by minimizing the function psi over the data set
# Multiple minimums are theoretically possible, so the lowest ordered one is selected

mu.hat <- function( x ){
  x.min <- 1
  psi.min <- psi( x[1], x )
  for( i in 2 : length( x ) ){
    psi.i <- psi( x[i], x )
    if( psi.i < psi.min ){         # pick the first element leading to a minimum
      x.min <- i
      psi.min <- psi.i
    }
  }
  return( x[x.min] )
}

# The parameters alpha and beta estimates are directly derived from mu

alpha.hat <- function( mu, x ){
  1/2 * ( delta( mu, x ) - mean( x ) + mu + sqrt( delta( mu, x ) ^ 2 - ( mean( x ) - mu ) ^ 2 ) )
}

beta.hat <- function( mu, x ){
  1/2 * ( delta( mu, x ) + mean( x ) - mu + sqrt( delta( mu, x ) ^ 2 - ( mean( x ) - mu ) ^ 2 ) )
}

# Calculate goodness of fit statistics for asymmetric Laplace ( 1: W2, 2: A2, 3: sqrt( beta1 ) )
# Anderson-Darling and Cramér-von Mises tests in Puig & Stephens 2007, p. 52

alaplace_test <- function( x, mu, alpha, beta ){
  z <- sapply( x, aLaplaceCumDist, mu = mu, alpha = alpha, beta = beta )
  n <- length( z )
  testStats <- c( 0.0, 0.0, 0.0 )

  for( i in 1 : n ){
    testStats[1] <- testStats[1] + ( z[i] - ( 2 * i - 1 ) / ( 2 * n ) ) ^ 2
    testStats[2] <- testStats[2] + ( 2 * i - 1 ) * ( log( z[i] ) + log( 1 - z[n + 1 - i] ) )
  }

  testStats[1] <- testStats[1] + 1 / ( 12 * n )
  testStats[2] <- - n - ( 1 / n ) * testStats[2]
  testStats[3] <- 2 * ( beta ^ 3 - alpha ^ 3 ) / ( alpha ^ 2 + beta ^ 2 ) ^ ( 3 / 2 )

  return( testStats )
}

# Critical values for the Cramér-von Mises test (Laplace), n=50, significance= 1%,5%,10%,15%,20%
# from Kotz et al. 2001
criticalCvMn50 <- c( 0.212, 0.142, 0.113, 0.096, 0.085 )

# Critical values for the Anderson-Darling test (Laplace), n=50, significance= 1%,5%,10%,15%,20%
# from Kotz et al. 2001
criticalADn50 <- c( 1.393, 0.967, 0.783, 0.673, 0.607 )

# Calculate goodness of fit statistics for symmetric Laplace ( 1: W2, 2: A2 )
# Cramér-von Mises and Anderson-Darling tests in Kotz et al. 2001, p. 105-106,
# eq. 2.6.201 and 2.6.200 (corrected)

laplace_test <- function( x, m, s ){
  z <- sapply( x, plaplace, location = m, scale = s )
  n <- length( z )
  testStats <- c( 0.0, 0.0 )

  for( j in 1 : n ){
    testStats[1] <- testStats[1] + ( z[j] - ( 2 * j - 1 ) / ( 2 * n ) ) ^ 2
    testStats[2] <- testStats[2] + ( 2 * j - 1 ) * ( log( z[j] ) + log( 1 - z[n - j + 1 ] ) )
  }

  testStats[1] <- 1 / ( 12 * n ) + testStats[1]
  testStats[2] <- - n - ( 1 / n ) * testStats[2]

  return( testStats )
}

# Approximate p-values for the fitness tests

p_value <- function( test, stat, n ){
  if( ! is.finite( stat ) || n != 50 )
    return( "" )

  if( test == "AD"){
    if( stat <= criticalADn50[5] )
      return( "20%" )
    if( stat <= criticalADn50[4] )
      return( "15%" )
    if( stat <= criticalADn50[3] )
      return( "10" )
    if( stat <= criticalADn50[2] )
      return( "5%" )
    if( stat <= criticalADn50[5] )
      return( "1%" )

    return( "< 1%" )
  }

  if( test == "CvM"){
    if( stat <= criticalCvMn50[5] )
      return( "20%" )
    if( stat <= criticalCvMn50[4] )
      return( "15%" )
    if( stat <= criticalCvMn50[3] )
      return( "10" )
    if( stat <= criticalCvMn50[2] )
      return( "5%" )
    if( stat <= criticalCvMn50[5] )
      return( "1%" )

    return( "< 1%" )
  }
}


# ======== Distribution functions (outside packages) ========

# ---- The asymmetric Laplace distribution function ----

aLaplaceDist <- function( x, mu, alpha, beta ){
  normC <- 1 / ( alpha + beta )
  ifelse( x < mu,
          normC * exp( ( x - mu ) / alpha ),
          normC * exp( ( mu - x ) / beta ) )
}

# ---- The asymmetric Laplace cummulative distribution function ----

aLaplaceCumDist <- function( x, mu, alpha, beta ){
  ifelse( x < mu,
          alpha * exp( ( x - mu ) / alpha ) / ( alpha + beta ),
          1 - beta * exp( ( mu - x ) / beta ) / ( alpha + beta ) )
}

# ---- The symmetric Subbotin distribution function ----

subbotinDist <- function( x, m, a, b ){
  normC <- 1 / ( 2 * a * b ^ ( 1 / b ) * gamma( 1 + 1 / b ) )
  normC * exp( - abs( ( x - m ) / a ) ^ b / b )
}

# ---- The asymmetric Subbotin distribution function ----

aSubbotinDist <- function( x, m, a1, a2, b1, b2 ){
  normC <- a1 * b1 ^ ( 1 / ( b1 - 1 ) ) * gamma( 1 + 1 / b1 )
          + a2 * b2 ^ ( 1 / ( b2 - 1 ) ) * gamma( 1 + 1 / b2 )
  ifelse( x < m,
          ( 1 / normC ) * exp( - abs( ( x - m ) / a1 ) ^ b1 / b1 ),
          ( 1 / normC ) * exp( - abs( ( x - m ) / a2 ) ^ b2 / b2 ) )
}


# ============ Functions for fitting data to theoretical distributions ============

# ---- Normal distribution ----

fit_normal <- function( x ){
  avgx <- mean ( x )
  sdx <- sd( x )

  return( c( sdx, avgx ) )
}

# ---- Lognormal distribution ----

fit_lognormal <- function( x ){
  locx <- mean ( log( x[ x > 0 ] ) )
  scalex <- sd( log( x[ x > 0 ] ) )

  return( c( scalex, locx ) )
}

# ---- Laplace distribution ----

fit_laplace <- function( x ){
  mx <- median( x )
  sx <- sum( abs( x - mx ) ) / length( x )  # robust estimate of standard deviation

  return( c( sx, mx ) )
}

# ---- Asymmetric Laplace distribution ----

fit_alaplace <- function( x ){
  mux <- mu.hat( x )
  alphax <- alpha.hat( mux, x )
  betax <- beta.hat( mux, x )

  return( c( betax, alphax, mux ) )
}

# ---- Subbotin distribution ----

exec_subbofit <- function( x, type  = "symmetric" ) {
  if( ! exists( "folder" ) )  # folder where to run Subbotools defined?
    folder <- "."
  if( type == "asymmetric" )
    command <- "subboafit"
  else
    command <- "subbofit"

  cat( "Running external SubboFit on a MC run ... " )

  outStr <- system2( paste0( subbotoolsFolder, command ), args = "-O 3",
                     input = as.character( x ), stdout = TRUE, stderr = FALSE )
  try( subboFit <- scan( textConnection ( outStr ), quiet = TRUE ), silent = TRUE )

  if( type == "asymmetric" )
    se <- paste( subboFit[ 6 ], subboFit[7] )
  else
    se <- subboFit[ 4 ]
  cat( "done\n" )

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
    return( subboFit )
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

  return( subboFit )
}

# ---- Asymmetric Subbotin distribution ----

fit_asubbotin <- function( x ){
  # default return in case of error
  subboaFit <- c( as.numeric( NA ), as.numeric( NA ), as.numeric( NA ), as.numeric( NA ) )

  # prepare valid data for Subbotools (no NA's & limited sample size)
  x <- x[ !is.na( x ) ]
  if( length( x ) > subboMaxSample )
    x <- sample( x, subboMaxSample )
  if( length( x ) < subboMinSample ){
    warning( "Too few observations to fit Subbotin: returning NA")
    return( subboaFit )
  }

  subboaFit <- exec_subbofit( x, type = "asymmetric" )

  # check for degenerated distribution
  if( subboBlimit != 0 && ! is.na( subboaFit[1] ) && subboaFit[1] > subboBlimit ){
    subboaFit <- c( as.numeric( NA ), as.numeric( NA ), as.numeric( NA ), as.numeric( NA ) )
    warning( "Degenerated Subbotin distribution: returning NA")
  }

  return( subboaFit )
}


# ==== Functions to plot the empirical and the fitted theoretical distributions ====

# ---- Symmetric Laplace distribution ----

plot_laplace <- function( x, lapFit, xlab, ylab, tit, subtit ){
  bins <- hist( x, breaks = nBins, plot = F )
  plot( bins$mids, bins$density, log = "y", type = "p", pch = 4,
        main = tit, sub = subtit, xlab = xlab, ylab = ylab,
        ylim = c( min( bins$density[bins$density!=0] ), 2 * max( bins$density ) ) )

  fit <- dalaplace( x, location = lapFit[2], scale = lapFit[1], kappa = 1 )

  lines( x, fit, col = "gray")

  legend( x = "topleft", legend = c( "Model", "Laplace" ),
          inset = 0.03, cex = 0.8, lty = c( 1, 1 ), lwd = 2.5,
          col = c( "black", "gray"), adj = 0.1)

  stats <- laplace_test( sample ( x, nSample ), lapFit[2], lapFit[1] )
  legend( x = "topright", legend = c( sprintf( "m = %.4f", lapFit[2] ),
                                      sprintf( "s = %.4f", lapFit[1] ),
                                      sprintf( "W2 = %.4f", stats[1] ),
                                      sprintf( "p-val. = %s", p_value( "CvM", stats[1], nSample ) ),
                                      sprintf( "A2 = %.4f", stats[2] ),
                                      sprintf( "p-val. = %s", p_value( "AD", stats[2], nSample ) ) ),
          inset = 0.03, cex = 0.8, adj = 0.1)

  if( plotQQ ){
    qqplot( qalaplace( ppoints( length( x ) ), location = lapFit[2], scale = lapFit[1], kappa = 1 ),
            x, main = "Laplace distribution Q-Q plot", sub = subtit,
            xlab = "Theoretical quantiles", ylab = "Model quantiles")
    abline( 0, 1 )
  }
  return( stats )
}

# ---- Asymmetric Laplace distribution ----

plot_alaplace <- function( x, aLapFit, xlab, ylab, tit, subtit ){
  bins <- hist( x, breaks = nBins, plot = F )
  plot( bins$mids, bins$density, log = "y", type = "p", pch = 4,
        main = tit, sub = subtit, xlab = xlab, ylab = ylab,
        ylim = c( min( bins$density[bins$density!=0] ), 2 * max( bins$density ) ) )

  fit <- aLaplaceDist( x, mu = aLapFit[3], alpha = aLapFit[2], beta = aLapFit[1] )

  lines( x, fit, col = "gray")

  legend( x = "topleft", legend = c( "Model", "As.Laplace" ),
          inset = 0.03, cex = 0.8, lty = c( 1, 1 ), lwd = 2.5,
          col = c( "black", "gray"), adj = 0.1)

  stats <- alaplace_test( sample ( x, nSample ), aLapFit[3], aLapFit[2], aLapFit[1] )
  legend( x = "topright", legend = c( sprintf( "mu = %.4f", aLapFit[3] ),
                                      sprintf( "alpha = %.4f", aLapFit[2] ),
                                      sprintf( "beta = %.4f", aLapFit[1] ),
                                      sprintf( "W2 = %.4f", stats[1] ),
                                      sprintf( "p-val. = %s", p_value( "CvM", stats[1], nSample ) ),
                                      sprintf( "A2 = %.4f", stats[2] ),
                                      sprintf( "p-val. = %s", p_value( "AD", stats[2], nSample ) ),
                                      sprintf( "sqrt(beta1) = %.4f", stats[3] ) ),
          inset = 0.03, cex = 0.8, adj = 0.1)
  return( stats )
}

# ---- Subbotin distribution ----

plot_subbotin <- function( x, subboFit, xlab, ylab, tit, subtit ){
  bins <- hist( x, breaks = nBins, plot = F )
  plot( bins$mids, bins$density, log = "y", type = "p", pch = 4,
        main = tit, sub = subtit, xlab = xlab, ylab = ylab,
        ylim = c( min( bins$density[bins$density!=0] ), 2 * max( bins$density ) ) )

  fit <- subbotinDist( x, m = subboFit[3], a = subboFit[2], b = subboFit[1] )

  lines( x, fit, col = "gray")


  legend( x = "topleft", legend = c( "Model", "Subbotin" ),
          inset = 0.03, cex = 0.8, lty = c( 1, 1 ), lwd = 2.5,
          col = c( "black", "gray"), adj = 0.1)

  stats <- ks.test( sample ( x, nSample ), subbotinDist, m = subboFit[3], a = subboFit[2], b = subboFit[1] )
  legend( x = "topright", legend = c( sprintf( "m = %.4f", subboFit[3] ),
                                      sprintf( "a = %.4f", subboFit[2] ),
                                      sprintf( "b = %.4f", subboFit[1] ),
                                      sprintf( "D = %.4f", stats$statistic ),
                                      sprintf( "p-val. = %.4f", stats$p.value ) ),
          inset = 0.03, cex = 0.8, adj = 0.1)

  if( plotQQ && subboFit[1] >= 1 ){      # qnormp does not work for p < 1
    qqplot( qnormp( ppoints( length( x ) ), mu = subboFit[3], sigmap = subboFit[2], p = subboFit[1] ),
            x, main = "Subbotin distribution Q-Q plot", sub = subtit,
            xlab = "Theoretical quantiles", ylab = "Model quantiles")
    abline( 0, 1 )
  }
  return( stats )
}

overplot_subbotin <- function( i, x, subboFit, xlab, ylab, tit, subtit ){
  bins <- hist( x, breaks = nBins, plot = F )

  if( i == 1 )              # first plot
    plot( bins$mids, bins$density, log = "y", type = "p", pch = i,
          main = tit, sub = subtit, xlab = xlab, ylab = ylab,
          ylim = c( min( bins$density[bins$density!=0] ), 2 * max( bins$density ) ) )
  else
    points( bins$mids, bins$density, type = "p", pch = i, col = colors[i] )

  fit <- subbotinDist( x, m = subboFit[3], a = subboFit[2], b = subboFit[1] )

  lines( x, fit, col = colors[i] )
}

# ---- Asymmetric Subbotin distribution ----

plot_asubbotin <- function( x, aSubboFit, xlab, ylab, tit, subtit ){
  bins <- hist( x, breaks = nBins, plot = F )
  plot( bins$mids, bins$density, log = "y", type = "p", pch = 4,
        main = tit, sub = subtit, xlab = xlab, ylab = ylab,
        ylim = c( min( bins$density[bins$density!=0] ), 2 * max( bins$density ) ) )

  fit <- aSubbotinDist( x, m = aSubboFit[5], a1 = aSubboFit[3], a2 = aSubboFit[4],
                        b1 = aSubboFit[1], b2 = aSubboFit[2] )

  lines( x, fit, col = "gray")


  legend( x = "topleft", legend = c( "Model", "As.Subbotin" ),
          inset = 0.03, cex = 0.8, lty = c( 1, 1 ), lwd = 2.5,
          col = c( "black", "gray"), adj = 0.1)

  stats <- ks.test( sample ( x, nSample ), aSubbotinDist, m = aSubboFit[5], a1 = aSubboFit[3],
                    a2 = aSubboFit[4], b1 = aSubboFit[1], b2 = aSubboFit[2] )
  legend( x = "topright", legend = c( sprintf( "m = %.4f", aSubboFit[5] ),
                                      sprintf( "a1 = %.4f", aSubboFit[3] ),
                                      sprintf( "a2 = %.4f", aSubboFit[4] ),
                                      sprintf( "b1 = %.4f", aSubboFit[1] ),
                                      sprintf( "b2 = %.4f", aSubboFit[2] ),
                                      sprintf( "D = %.4f", stats$statistic ),
                                      sprintf( "p-val. = %.4f", stats$p.value ) ),
          inset = 0.03, cex = 0.8, adj = 0.1)

  return( stats )
}

# ---- lognormal distribution ----

plot_lognormal <- function( x, normFit, xlab, ylab, tit, subtit ){
  x <- log( x[ x>0 ] )
  bins <- hist( x, breaks = nBins, plot = F )
  plot( bins$mids, bins$density, log = "y", type = "p", pch = 4,
        main = tit, sub = subtit, xlab = xlab, ylab = ylab,
        ylim = c( min( bins$density[bins$density!=0] ), 2 * max( bins$density ) ) )

  fit <- dnorm( x, mean = normFit[2], sd = normFit[1] )

  lines( x, fit, col = "gray")

  legend( x = "topleft", legend = c( "Model", "Normal" ),
          inset = 0.03, cex = 0.8, lty = c( 1, 1 ), lwd = 2.5,
          col = c( "black", "gray"), adj = 0.1)

  stats <- shapiro.test( sample ( x, nSample ) )
  legend( x = "topright", legend = c( sprintf( "avg = %.4f", normFit[2] ),
                                      sprintf( "sd = %.4f", normFit[1] ),
                                      sprintf( "W = %.4f", stats$statistic ),
                                      sprintf( "p-val. = %.4f", stats$p.value ) ),
          inset = 0.03, cex = 0.8, adj = 0.1)

  if( plotQQ ){
    qqnorm( x, main = "(log) Normal distribution Q-Q plot", sub = subtit,
            xlab = "Theoretical quantiles", ylab = "Model quantiles")
    qqline( x )
  }
  return( stats )          # Shapiro-Wilks test for normality
}

overplot_lognormal <- function( i, x, lognormFit, xlab, ylab, tit, subtit ){
  x <- log( x[ x>0 ] )
  bins <- hist( x, breaks = nBins, plot = F )

  if( i == 1 ){              # first plot
    plot( bins$mids, bins$density, log = "y", type = "p", pch = i,
          main = tit, sub = subtit, xlab = xlab, ylab = ylab,
          ylim = c( min( bins$density[bins$density!=0] ), 2 * max( bins$density ) ) )

    legend( x = "topleft", legend = caseNames,
            inset = 0.03, cex = 0.8, lty = c( 1, 1 ), lwd = 2.5,
            col = colors[ c( 1 : length( caseNames ) ) ], adj = 0.1)
  }
  else
    points( bins$mids, bins$density, type = "p", pch = i, col = colors[i] )

  fit <- dnorm( x, mean = lognormFit[2], sd = lognormFit[1] )

  lines( x, fit, col = colors[i] )
}

# ---- Normal distribution (non log) ----

plot_normal <- function( x, normFit, xlab, ylab, tit, subtit ){
  bins <- hist( x, breaks = nBins, plot = F )
  plot( bins$mids, bins$density, type = "p", pch = 4,
        main = tit, sub = subtit, xlab = xlab, ylab = ylab,
        ylim = c( min( bins$density[bins$density!=0] ), 1.5 * max( bins$density ) ) )

  fit <- dnorm( x, mean = normFit[2], sd = normFit[1] )
  lines( x, fit, col = "gray")

  legend( x = "topleft", legend = c( "Model", "Normal" ),
          inset = 0.03, cex = 0.8, lty = c( 1, 1 ), lwd = 2.5,
          col = c( "black", "gray"), adj = 0.1)

  stats <- shapiro.test( sample ( x, nSample ) )
  legend( x = "topright", legend = c( sprintf( "avg = %.4f", normFit[2] ),
                                      sprintf( "sd = %.4f", normFit[1] ),
                                      sprintf( "W = %.4f", stats$statistic ),
                                      sprintf( "p-val. = %.4f", stats$p.value ) ),
          inset = 0.03, cex = 0.8, adj = 0.1)

  if( plotQQ ){
    qqnorm( x, main = "Normal distribution Q-Q plot", sub = subtit,
            xlab = "Theoretical quantiles", ylab = "Model quantiles")
    qqline( x )
  }
  return( stats )          # Shapiro-Wilks test for normality
}

# ---- Jointly plot some fitted X theoretical distributions (in log) ----

plot_all <- function( x, normFit, lapFit, subboFit, xlab, ylab, tit, subtit ){
  bins <- hist( x, breaks = nBins, plot = F )
  plot( bins$mids, bins$density, log = "y", type = "p", pch = 4,
        main = tit, sub = subtit, xlab = xlab, ylab = ylab,
        ylim = c( min( bins$density[bins$density!=0] ), 2 * max( bins$density ) ) )

  fit1 <- dalaplace( x, location = lapFit[2], scale = lapFit[1], kappa = 1 )
  fit2 <- subbotinDist( x, m = subboFit[3], a = subboFit[2], b = subboFit[1] )
  fit3 <- dnorm( x, mean = normFit[2], sd = normFit[1] )

  lines( x, fit1, col = "green")
  lines( x, fit2, col = "blue")
  lines( x, fit3, col = "red")

  legend( x = "topleft", legend = c( "Model", "Laplace", "Subbotin", "Normal" ),
          inset = 0.03, cex = 0.8, lty = c( 1, 1 ), lwd = 2.5,
          col = c( "black", "green", "blue", "red"), adj = 0.1)
}

# ---- rank against lognormal distribution ----

plot_rankLognormal <- function( x, lognormFit, xlab, ylab, tit, subtit ){
  if( length( x ) > 100000 )
    x <- sample( x, 100000 )
  x <- sort( x[ x>0 ] )         # from the lowest to the highest
  logx <- log( x )
  n <- length( x )

  index <- c( 1 : n )           # create scale
  logIndex <- log( n - index )

  plot( logx, logIndex, main = tit, sub = subtit, xlab = xlab, ylab = ylab )

  fit <- log( n * ( 1 - plnorm( x, meanlog = lognormFit[2], sdlog = lognormFit[1]  ) ) )
  lines( logx, fit, col = "dark gray")

  legend( x = "topright", legend = c( "Model", "Lognormal" ),
          inset = 0.03, cex = 0.8, lty = c( 1, 1 ), lwd = 2.5,
          col = c( "black", "dark gray"), adj = 0.1)
}

overplot_rankLognormal <- function( i, x, lognormFit, xlab, ylab, tit, subtit ){
  if( length( x ) > 10000 )
    x <- sample( x, 10000 )
  x <- sort( x[ x > 0 ] )         # from the lowest to the highest
  logx <- log( x )
  n <- length( x )

  index <- c( 1 : n )           # create scale
  logIndex <- log( n - index )

  if( i == 1 ){              # first plot
    plot( logx, logIndex, main = tit, sub = subtit, xlab = xlab, ylab = ylab )

    legend( x = "topright", legend = caseNames,
            inset = 0.03, cex = 0.8, lty = c( 1, 1 ), lwd = 2.5,
            col = colors[ c( 1 : length( caseNames ) ) ], adj = 0.1)
  }
  else
    points( logx, logIndex, col = colors[i] )

  fit <- log( n * ( 1 - plnorm( x, meanlog = lognormFit[2], sdlog = lognormFit[1]  ) ) )

  lines( logx, fit, col = colors[i] )
}
