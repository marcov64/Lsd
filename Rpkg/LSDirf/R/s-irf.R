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

# ====== State-dependent impulse-response function ======

state.irf.lsd <- function( data, irf, states = NULL, state.num = 1,
                           state.vars = NULL, eval.state = NULL,
                           metr.irf = NULL, add.vars = NULL,
                           irf.type = c( "cumulative", "incremental", "none" ),
                           ci.R = 999, ci.type = c( "basic", "perc", "bca" ),
                           alpha = 0.05, seed = 1, ... ) {

  # check data, remove outliers, add new variables, and select state variables
  stateData <- build_state_data( data, irf, add.vars, state.vars,
                                 eval.state = eval.state, metr.irf = metr.irf )
  if( ! is.null( metr.irf ) ) {
    cirMetric <- stateData[ , "metric" ]
    stateData <- stateData[ , - which( colnames( stateData ) == "metric" ),
                            drop = FALSE ]
  } else
    cirMetric <- NULL

  state.vars <- colnames( stateData )
  nMC <- nrow( stateData )

  if( ! is.null( states ) && ! inherits( states, "state.ident.lsd" ) )
    stop( "Invalid states object (not from state.ident.lsd())" )

  if( ! is.null( states ) && ( is.null( state.num ) || ! is.finite( state.num ) ||
                               round( state.num ) < 1 || round( state.num ) > nrow( states$state.freq ) ) )
    stop( "Invalid state selection (state.num)" )

  if( is.null( alpha ) || ! is.finite( alpha ) ||
      alpha <= 0 || alpha > 0.5 )
    stop( "Invalid significance level (alpha)" )

  if( is.null( ci.R ) || ! is.finite( ci.R ) || round( ci.R ) < 1 )
    stop( "Invalid bootstrap repetitions (ci.R)" )

  if( ! is.null( seed ) && ! is.finite( seed ) )
    stop( "Invalid random seed (seed)" )

  state.num <- round( state.num )
  ci.R      <- round( ci.R )
  ci.type   <- match.arg( ci.type )
  irf.type      <- match.arg( irf.type )

  # default evaluation function: 2 states, lower or higher than mean/average
  if( is.null( states ) && is.null( eval.state ) ) {
    if( irf$stat == "median" )
      eval.state <- function( x )
        ! x[ , 1 ] < stats::median( x[ , 1 ], na.rm = TRUE )
    else
      eval.state <- function( x )
        ! x[ , 1 ] < mean( x[ , 1 ], na.rm = TRUE )
  }

  # evaluate state of each MC to split ir's, transform to 1,..,N
  if( is.null( states ) ) {
    state <- stateMC <- NULL

    try( stateMC <- as.integer( eval.state( stateData ) ) )

    if( is.null( stateMC ) || length( stateMC ) != nMC ||
        ! all( is.finite( stateMC ) ) )
      stop( "Invalid result from state-evaluation function (eval.state)" )
  } else {
    state <- as.character( states$state.freq$State )[ state.num ]
    stateMC <- eval.state.default( stateData, states, state.num )
  }

  uniqStates <- sort( unique( stateMC ) )

  if( length( uniqStates ) == 1 )
    stop( "Only one state produced by state-evaluation function (eval.state)" )

  stateMap <- list( )
  for( i in 1 : length( uniqStates ) )
    stateMap[[ as.character( uniqStates[ i ] ) ]] <- i

  for( i in 1 : length( stateMC ) )
    stateMC[ i ] <- stateMap[[ as.character( stateMC[ i ] ) ]]

  # define function to calculate boostrap state-dependent IRF confidence intervals
  if( irf$stat == "median" )
    funIRF <- function( x, idx ) stats::median( x[ idx ], na.rm = TRUE )
  else
    funIRF <- function( x, idx ) mean( x[ idx ], na.rm = TRUE )

  irfState <- irfStateCIlo <- irfStateCIhi <-
    cirfState <- cirfStateCIlo <- cirfStateCIhi <-
    irfStateYlim <- cirfStateYlim <- scirMetric <- list( )

  set.seed( seed )      # reset PRNG seed to ensure reproducibility

  # split state-specific ir's, compute state-dependent IRF's and conf. intervals
  for( i in 1 : length( uniqStates ) ) {
    sir <- scir <- matrix( nrow = 0, ncol = ncol( irf$ir ) )
    scirMetric[[ i ]] <- vector( "numeric" )

    for( j in 1 : length( stateMC ) )
      if( stateMC[ j ] == i ) {
        sir  <- rbind( sir, irf$ir[ j, ] )
        scir <- rbind( scir, irf$cir[ j, ] )

        if( ! is.null( cirMetric ) )
          scirMetric[[ i ]] <- append( scirMetric[[ i ]], cirMetric[ j ] )
      }

    if( irf$stat == "median" ) {
      irfState[[ i ]]  <- apply( sir, 2, stats::median, na.rm = TRUE )
      cirfState[[ i ]] <- apply( scir, 2, stats::median, na.rm = TRUE )
    } else {
      irfState[[ i ]]  <- apply( sir, 2, mean, na.rm = TRUE )
      cirfState[[ i ]] <- apply( scir, 2, mean, na.rm = TRUE )
    }


    irfStateCIlo[[ i ]] <- irfStateCIhi[[ i ]] <-
      cirfStateCIlo[[ i ]] <- cirfStateCIhi[[ i ]] <- rep( NA, ncol( sir ) )

    # compute bootstrap confidence intervals and IRF limits
    for( j in 1 : ncol( sir ) ) {
      sirfCI <- scirfCI <- NULL

      try( invisible(
        utils::capture.output(
          sirfCI <- boot::boot.ci( boot::boot( sir[ , j ], statistic = funIRF,
                                               R = ci.R ),
                                   conf = 1 - alpha, type = ci.type )
        )
      ), silent = TRUE )

      try( invisible(
        utils::capture.output(
          scirfCI <- boot::boot.ci( boot::boot( scir[ , j ], statistic = funIRF,
                                                R = ci.R ),
                                    conf = 1 - alpha, type = ci.type )
        )
      ), silent = TRUE )

      if( ! is.null( sirfCI ) ) {
        irfStateCIlo[[ i ]][ j ] <- sirfCI$basic[ 4 ]
        irfStateCIhi[[ i ]][ j ] <- sirfCI$basic[ 5 ]
      }

      if( ! is.null( scirfCI ) ) {
        cirfStateCIlo[[ i ]][ j ] <- scirfCI$basic[ 4 ]
        cirfStateCIhi[[ i ]][ j ] <- scirfCI$basic[ 5 ]
      }
    }

    # find IRF vertical limits
    irfStateYlim[[ i ]]  <- c( min( irfStateCIlo[[ i ]], na.rm = TRUE ),
                               max( irfStateCIhi[[ i ]], na.rm = TRUE ) )
    cirfStateYlim[[ i ]] <- c( min( cirfStateCIlo[[ i ]], na.rm = TRUE ),
                               max( cirfStateCIhi[[ i ]], na.rm = TRUE ) )
  }

  names( irfState ) <- names( irfStateCIlo ) <- names( irfStateCIhi ) <-
    names( cirfState ) <- names( cirfStateCIlo ) <- names( cirfStateCIhi ) <-
    names( irfStateYlim ) <- names( cirfStateYlim ) <-
    paste0( "s", c( 1 : length( uniqStates ) ) )

  irfTest <- cirfTest <- NA
  cirfTestThoriz <- NULL

  # test significance of the difference among states
  if( length( uniqStates ) == 2 ) {

    if( irf$stat == "median" ) {
      # wilcoxon U-test to test difference between 2 states
      try( irfTest <- suppressWarnings(
        stats::wilcox.test( irfState[[ 1 ]], irfState[[ 2 ]], paired = TRUE,
                            exact = FALSE, na.action = "na.omit",
                            digits.rank = 7 ) ),
        silent = TRUE )

      try( cirfTest <- suppressWarnings(
        stats::wilcox.test( cirfState[[ 1 ]], cirfState[[ 2 ]], paired = TRUE,
                            exact = FALSE, na.action = "na.omit",
                            digits.rank = 7 ) ),
        silent = TRUE )

      if( ! is.null( cirMetric ) )
        try( cirfTestThoriz <- suppressWarnings(
          stats::wilcox.test( scirMetric[[ 1 ]], scirMetric[[ 2 ]],
                              exact = FALSE, na.action = "na.omit",
                              digits.rank = 7 ) ),
          silent = TRUE )
    } else {
      # Welch t-test to test difference between 2 states
      try( irfTest <- suppressWarnings(
        stats::t.test( irfState[[ 1 ]], irfState[[ 2 ]], paired = TRUE,
                       na.action = "na.omit", conf.level = 1 - alpha ) ),
        silent = TRUE )

      try( cirfTest <- suppressWarnings(
        stats::t.test( cirfState[[ 1 ]], cirfState[[ 2 ]], paired = TRUE,
                       na.action = "na.omit", conf.level = 1 - alpha ) ),
        silent = TRUE )

      if( ! is.null( cirMetric ) )
        try( cirfTestThoriz <- suppressWarnings(
          stats::t.test( scirMetric[[ 1 ]], scirMetric[[ 2 ]],
                         na.action = "na.omit", conf.level = 1 - alpha ) ),
          silent = TRUE )
    }

  } else {

    if( irf$stat == "median" ) {
      # Kruskal-Wallis H-test to test difference between several states
      try( irfTest <- suppressWarnings(
        stats::kruskal.test( irfState, na.action = "na.omit" ) ),
        silent = TRUE )

      try( cirfTest <- suppressWarnings(
        stats::kruskal.test( cirfState, na.action = "na.omit" ) ),
        silent = TRUE )

      if( ! is.null( cirMetric ) )
        try( cirfTestThoriz <- suppressWarnings(
          stats::kruskal.test( scirMetric, na.action = "na.omit" ) ),
          silent = TRUE )
    } else {
      # rearrange data
      irfData <- data.frame( resp = rep( letters[ 1 : length( uniqStates ) ],
                                         each = length( irfState[[ 1 ]] ) ),
                             state = unlist( irfState ) )
      cirfData <- data.frame( resp = rep( letters[ 1 : length( uniqStates ) ],
                                          each = length( cirfState[[ 1 ]] ) ),
                              state = unlist( cirfState ) )

      # one-way ANOVA F-test to test difference between several states
      try( irfTest <- suppressWarnings(
        stats::anova( stats::lm( state ~ resp, irfData, na.action = "na.omit" ) ) ),
        silent = TRUE )

      try( cirfTest <- suppressWarnings(
        stats::anova( stats::lm( state ~ resp, cirfData, na.action = "na.omit" ) ) ),
        silent = TRUE )

      if( ! is.null( cirMetric ) ) {
        metrData <- data.frame( resp = rep( letters[ 1 : length( uniqStates ) ],
                                            each = length( scirMetric[[ 1 ]] ) ),
                                state = unlist( scirMetric ) )
        try( cirfTestThoriz <- suppressWarnings(
          stats::anova( stats::lm( state ~ resp, metrData, na.action = "na.omit" ) ) ),
          silent = TRUE )
      }
    }
  }

  # pack object
  sirf <- list( irf.state = irfState, cirf.state = cirfState,
                irf.state.ci.lo = irfStateCIlo, irf.state.ci.hi = irfStateCIhi,
                cirf.state.ci.lo = cirfStateCIlo, cirf.state.ci.hi = cirfStateCIhi,
                irf.state.ylim = irfStateYlim, cirf.state.ylim = cirfStateYlim,
                irf.test = irfTest, cirf.test = cirfTest,
                cirf.test.t.horiz = cirfTestThoriz, state = state,
                state.vars = state.vars, t.horiz = irf$t.horiz,
                var.irf = irf$var.irf, var.ref = irf$var.ref, stat = irf$stat,
                alpha = alpha, nsample = nMC, outliers = irf$outliers,
                call = match.call( ) )
  class( sirf ) <- "state.irf.lsd"

  if( irf.type != "none" )
    plot.state.irf.lsd( sirf, state = state, irf.type = irf.type, ... )

  return( sirf )
}


# ====== State-dependent impulse-response function print/plot ======

print.state.irf.lsd <- function( x, ... ) {

  if( ! inherits( x, "state.irf.lsd" ) )
    stop( "Invalid object (not from state.irf.lsd())" )

  if( ! exists( "irf.type" ) )
    irf.type <- "cumulative"
  else
    irf.type <- match.arg( irf.type, c( "cumulative", "incremental" ) )

  if( x$stat == "median" )
    test <- ifelse( length( x$irf.state ) == 2, "U-test", "H-test" )
  else
    test <- ifelse( length( x$irf.state ) == 2, "t-test", "F-test" )

  cirf.t.horiz.p.val <- NA

  if( x$stat == "mean" && length( x$irf.state ) > 2 ) {
    if( ! is.null( x$cirf.test.t.horiz ) )
      cirf.t.horiz.p.val <- x$cirf.test.t.horiz[ 1, "Pr(>F)" ]

    irf.p.val <- x$irf.test[ 1, "Pr(>F)" ]
    cirf.p.val <- x$cirf.test[ 1, "Pr(>F)" ]
  } else {
    if( ! is.null( x$cirf.test.t.horiz ) )
      cirf.t.horiz.p.val <- x$cirf.test.t.horiz$p.value

    irf.p.val <- x$irf.test$p.value
    cirf.p.val <- x$cirf.test$p.value
  }

  if( is.null( x$state ) ) {
    state <- paste( x$state.vars, collapse = ", " )
    stateTag <- "IRF state-dependent variable(s):"
  } else {
    state <- x$state
    stateTag <- "State-dependent variable(s) split:"
  }

  info <- data.frame( c( state,
                         x$var.irf,
                         x$var.ref,
                         length( x$irf.state[[ 1 ]] ),
                         signif( irf.p.val, digits = 4 ),
                         signif( cirf.p.val, digits = 4 ),
                         signif( cirf.t.horiz.p.val, digits = 4 ),
                         x$alpha,
                         x$nsample,
                         length( x$outliers ) ) )
  colnames( info ) <- NULL
  rownames( info ) <- c( stateTag,
                         "Impulse-response function variable:",
                         "Reference variable (impulse time):",
                         "Time-horizon response period:",
                         paste( "IRF", test, "p-value (H0: states =):"),
                         paste( "C-IRF", test, "p-value (H0: states =):"),
                         paste( "C-IRF t-horizon", test, "p-value:"),
                         "Conf. interval significance:",
                         "Number of employed MC samples:",
                         "Number of outlier MC samples:" )

  printPars <- list( ... )
  printPars[[ "irf.type" ]] <- NULL

  print( info, printPars )

  if( length( x$cirf.state ) == 2 ) {

    if( irf.type == "cumulative" ) {
      f <- "Cumulative impulse-response function"
      l <- "IRF"
      data <- x$cirf.state
      ciLo <- x$cirf.state.ci.lo
      ciHi <- x$cirf.state.ci.hi
    } else {
      f <- "Impulse-response function"
      l <- "C-IRF"
      data <- x$irf.state
      ciLo <- x$irf.state.ci.lo
      ciHi <- x$irf.state.ci.hi
    }

    cat( "\n" )
    cat( f, "according to the state-dependent variable(s):\n\n" )

    data <- data.frame( ciLo$s1, data$s1, ciHi$s1,
                        c( 0 : ( length( data$s1 ) - 1 ) ),
                        ciLo$s2, data$s2, ciHi$s2 )
    colnames( data ) <- c( paste0( "Lo", l, " ci-" ),
                           paste0( "Lo", l, " ", x$stat ),
                           paste0( "Lo", l, " ci+" ),
                           "t",
                           paste0( "Hi", l, " ci-" ),
                           paste0( "Hi", l, " ", x$stat ),
                           paste0( "Hi", l, " ci+" ) )
    print( data, row.names = FALSE, printPars )
  }
}


plot.state.irf.lsd <- function( x, ... ) {

  if( ! inherits( x, "state.irf.lsd" ) )
    stop( "Invalid object (not from state.irf.lsd())" )

  if( ! exists( "state" ) )
    state <- 0
  else
    if( is.null( state ) || ! is.finite( state ) || round( state ) < 0 ||
        round( state ) > ( length( x$thr.state.val ) + 1 ) )
      stop( "Invalid state selected" )

  if( ! exists( "irf.type" ) )
    irf.type <- "cumulative"
  else
    irf.type <- match.arg( irf.type, c( "cumulative", "incremental" ) )

  state <- round( state )

  if( irf.type == "cumulative" ) {
    data <- x$cirf.state
    ciLo <- x$cirf.state.ci.lo
    ciHi <- x$cirf.state.ci.hi
    ylim <- x$cirf.state.ylim
  } else {
    data <- x$irf.state
    ciLo <- x$irf.state.ci.lo
    ciHi <- x$irf.state.ci.hi
    ylim <- x$irf.state.ylim
  }

  if( state != 0 ) {
    data <- data[[ state ]]
    ciLo <- ciLo[[ state ]]
    ciHi <- ciHi[[ state ]]
    ylim <- ylim[[ state ]]
  }

  plotPars <- list( ... )
  plotPars[[ "state" ]] <- NULL
  plotPars[[ "irf.type" ]] <- NULL

  do.call( plot_irf, c( list( data, ciLo, ciHi, ylim ), plotPars ) )
}
