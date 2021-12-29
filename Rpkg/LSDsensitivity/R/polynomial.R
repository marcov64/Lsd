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
