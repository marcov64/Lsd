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

# ==== Perform Kriging sensitivity analysis ====

kriging.sensitivity <- function( data, model, krig.sa = FALSE, sa.samp = 1000 ) {

  # ---- Sensitivity analysis for the kriging model ----

  if( krig.sa ) {                     # use Kriging specific SA method
    # generate MC points for Sobol decomposition using low-discrepancy sequences
    X1 <- sobol.rnd.exp( sa.samp, colnames( data$doe ), lwr.bound = data$facLimLo,
                         upr.bound = data$facLimUp )
    X2 <- sobol.rnd.exp( sa.samp, colnames( data$doe ), lwr.bound = data$facLimLo,
                         upr.bound = data$facLimUp )

    metamodel <- sensitivity::sobolGP( model = model$selected, type = "UK",
                                       X1 = X1, X2 = X2,
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

  sa <- cbind( mainEffect( metamodel ), totEffect( metamodel ) -
                 mainEffect( metamodel ) )
  rownames( sa ) <- colnames( data$doe )
  colnames( sa ) <- c( "Direct effects", "Interactions" )

  max.index <- function( x, pos = 1 )
    as.integer( sapply( sort( x, index.return = TRUE ), `[`,
                        length( x ) - pos + 1 )[ 2 ] )

  topEffect <- c( max.index( totEffect( metamodel ), 1 ),
                  max.index( totEffect( metamodel ), 2 ),
                  max.index( totEffect( metamodel ), 3 ) )

  cat( "Top parameters influencing response surface:\n" )
  cat( " First:", colnames( data$doe )[ topEffect[ 1 ] ], "\n" )
  cat( " Second:", colnames( data$doe )[ topEffect[ 2 ] ], "\n" )
  cat( " Third:", colnames( data$doe )[ topEffect[ 3 ] ], "\n\n" )

  sa <- list( metamodel = metamodel, sa = sa, topEffect = topEffect )
  class( sa ) <- "kriging.sensitivity.lsd"

  return( sa )
}


# ==== Kriging model selection ====

kriging.model.lsd <- function( data, ext.wgth = 0.5, trendModel = 0,
                               covModel = 0, digits = 4 ) {

  if( ! inherits( data, "doe.lsd" ) )
    stop( "Invalid data (not from read.doe.lsd())" )

  if( is.null( ext.wgth ) || ! is.finite( ext.wgth ) || ext.wgth < 0 )
    stop( "Invalid weight of external validation sample (ext.wgth)" )

  if( is.null( trendModel ) || ! is.finite( trendModel ) ||
      round( trendModel ) < 0 || round( trendModel ) > 2 )
    stop( "Invalid trend model order (trendModel)" )

  if( is.null( covModel ) || ! is.finite( covModel ) ||
      round( covModel ) < 0 || round( covModel ) > 5 )
    stop( "Invalid covariance model type (covModel)" )

  if( is.null( digits ) || ! is.finite( digits ) || round( digits ) < 0 )
    stop( "Invalid significant digits (digits)" )

  trendModel  <- round( trendModel )
  covModel    <- round( covModel )
  digits      <- round( digits )

  # The following code was adapted and expanded from Salle & Yildizoglu 2014
  #
  # x = doe is in data.frame format, and contains the DoE (n rows, k columns)
  # z = valid is in data.frame format, and contains the values of the factors at the
  # additional points (n rows, k columns)
  # y = resp[ , 1 ] is in matrix format, and contains the values of the response
  # d at the n points of the DoE, over the nSize replications (column is
  # named totDist, n rows)
  # sigma = resp[ , 2 ] is a column of n rows, with contains the SD/MAD of the
  # response d over the nSize replications of each n experiments.
  # w = valResp[ , 1 ] is in matrix format, and contains the values of the
  # response at the additional points (over the nSize replications ).
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
      km <- fit.kriging( data$resp[ , 1 ], data$doe, resp.noise = data$resp[ , 2 ],
                         trials = maxTrials, trend.func = trendTypes[[ i ]],
                         cov.func = covTypes[ j ] )
      models[[ i, j ]] <- km$model
      Q2[ i, j ] <- km$Q2

      if( ! onlyCross ) {
        rmse[ i, j ] <- rmse.kriging( km$model, data$valResp[ , 1 ], data$valid )
        mae[ i, j ] <- mae.kriging( km$model, data$valResp[ , 1 ], data$valid )
        rma[ i, j ] <- rma.kriging( km$model, data$valResp[ , 1 ], data$valid )
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
    rownames( table ) <- c( apply( cbind( rep( "Q2", length( trendTypes ) ),
                                          trendNames,
                                          rep( "trend", length( trendTypes ) ) ),
                                   1, paste, collapse = " " ) )
    rmseStat <- maeStat <- rmaStat <- valExtN <- NA
  } else {
    table <- data.frame( rbind( Q2, rmse, mae, rma, deparse.level = 0 ),
                         row.names = NULL )
    rownames( table ) <- c( apply( cbind( rep( "Q2", length( trendTypes ) ),
                                          trendNames,
                                          rep( "trend", length( trendTypes ) ) ),
                                   1, paste, collapse = " " ),
                            apply( cbind( rep( "RMSE", length( trendTypes ) ),
                                          trendNames,
                                          rep( "trend", length( trendTypes ) ) ),
                                   1, paste, collapse = " " ),
                            apply( cbind( rep( "MAE", length( trendTypes ) ),
                                          trendNames,
                                          rep( "trend", length( trendTypes ) ) ),
                                   1, paste, collapse = " " ),
                            apply( cbind( rep( "RMA", length( trendTypes ) ),
                                          trendNames,
                                          rep( "trend", length( trendTypes ) ) ),
                                   1, paste, collapse = " " ) )
    rmseStat <- rmse[ trendModel, covModel ]
    maeStat <- mae[ trendModel, covModel ]
    rmaStat <- rma[ trendModel, covModel ]
    valExtN <- nrow( data$valResp )
  }
  colnames( table ) <- covNames

  fmt <- function( x )
    format( round( x, digits = digits ), nsmall = digits )

  coef.label <- c( "trend(intercept)", "trend(inclination)",
                   apply( cbind( rep( paste0( m@covariance@range.names, "(" ),
                                      m@covariance@d ),
                                 m@covariance@var.names, rep( ")",
                                                              m@covariance@d ) ),
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

  m.std <- fit.kriging( data$resp[ , 1 ], X.std, resp.noise = data$resp[ , 2 ],
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
                 mae = maeStat, rma = rmaStat, extN = valExtN,
                 estimation = estimation, estimation.std = estimation.std,
                 coefficients = coefficients,
                 coefficients.std = coefficients.std, trend = trendModel,
                 trendNames = trendNames, cov = covModel, covNames = covNames )
  class( model ) <- "kriging.model.lsd"

  return( model )
}


# ==== Fit Kriging meta-model to response surface ====

fit.kriging <- function( response, doe, resp.noise = NULL, trend.func = ~1,
                         cov.func = "matern5_2", trials = 1 ) {

  # Multiplicative noise scaling factor for variance/noise
  scaleFactor <- 0.5

  # Cross validation - don't use noise info because of Q2 doesn't support it
  fit <- DiceKriging::km( design = doe, response = response, formula = trend.func,
                          covtype = cov.func, control = list( trace = FALSE,
                                                              print.level = 0 ) )
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
                                      noise.var = ( resp.noise ^ 2 ) *
                                        ( scaleFactor ^ trial ),
                                      control = list( trace = FALSE,
                                                      print.level = 0 ) ),
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
