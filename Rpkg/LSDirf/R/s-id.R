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

# ====== State identification function ======

state.ident.lsd <- function( data, irf, state.vars = NULL, metr.irf = NULL,
                             add.vars = NULL, state.cont = FALSE,
                             ntree = 500, maxdepth = 1, nodesize  = 5,
                             mtry = max( floor( ifelse( ! is.null( state.vars ),
                                                        length( state.vars ),
                                                        dim( data )[ 2 ] ) / 3 ),
                                         1 ),
                             quantile = 10, alpha = 0.05, seed = 1, ... ) {

  # check data, remove outliers, add new variables, and select state variables
  stateData <- build_state_data( data, irf, add.vars, state.vars,
                                 metr.irf = metr.irf )
  state.vars <- colnames( stateData )
  nVar <- length( state.vars )
  nMC <- nrow( stateData )

  if( is.null( ntree ) || ! is.finite( ntree ) || round( ntree ) < 1 )
    stop( "Invalid number of trees (ntree)" )

  if( is.null( maxdepth ) || ! is.finite( maxdepth ) || round( maxdepth ) < 1 )
    stop( "Invalid maximum tree depth (maxdepth)" )

  if( is.null( alpha ) || ! is.finite( alpha ) || alpha <= 0 || alpha > 0.5 )
    stop( "Invalid significance level (alpha)" )

  if( is.null( nodesize  ) || ! is.finite( nodesize  ) || round( nodesize )  < 1 )
    stop( "Invalid node minimum size (nodesize )" )

  if( is.null( mtry ) || ! is.finite( mtry ) || round( mtry ) < 1 )
    stop( "Invalid number of variable samples per node (mtry)" )

  if( is.null( quantile ) || ! is.finite( quantile ) || round( quantile ) < 2 )
    stop( "Invalid number of quantiles (quantile)" )

  if( ! is.null( seed ) && ! is.finite( seed ) )
    stop( "Invalid random seed (seed)" )

  if( is.null( state.cont ) || ! is.logical( state.cont ) )
    state.cont = FALSE

  ntree    <- round( ntree )
  maxdepth <- round( maxdepth )
  nodesize <- round( nodesize )
  mtry     <- round( mtry )
  quantile <- round( quantile )

  # create regression formula to use in trees
  formula <- paste( state.vars[ 1 ], "~", state.vars[ 2 ] )
  if( nVar > 2 )
    for( i in 3 : nVar )
      formula <- paste( formula, "+", state.vars[ i ] )

  formula <- stats::as.formula( formula )

  # data frame to store splitting paths (=states)
  nam <- c( )
  if( maxdepth > 1 )
    for( i in 1 : maxdepth )
      nam <- c( nam, paste0( "Var", i ), paste0( "Rel", i ), paste0( "Split", i ) )
  else
    nam <- c( "Var", "Rel", "Split" )

  states <- data.frame( matrix( NA, ncol = 3 * maxdepth + 1, nrow = 0 ) )
  colnames( states ) <- append( nam, "MetrD" )

  if( irf$stat == "median" )
    allMetr <- stats::median( stateData[ , 1 ], na.rm = TRUE )
  else
    allMetr <- mean( stateData[ , 1 ], na.rm = TRUE )

  #
  # -------- Algorithm to find continuous states --------
  #

  state <- 0            # found state counter
  set.seed( seed )      # reset PRNG seed to ensure reproducibility

  for( tree in 1 : ntree ) {

    # bootstrap data sample to generate new tree in forest
    smplData <- data.frame( stateData[ sample( nrow( stateData ),
                                               replace = TRUE ), ] )

    smplTree <- termNodes <- NULL

    # fit a tree to the sample and find terminal nodes
    # do not exclude low significance trees at this stage
    try( invisible( utils::capture.output(
      smplTree <- partykit::ctree( formula, smplData, maxdepth = maxdepth,
                                   mtry = mtry, alpha = 0.98 ) ) ),
      silent = TRUE )

    try( invisible( utils::capture.output(
      termNodes <- partykit::nodeids( smplTree, terminal = TRUE ) ) ),
      silent = TRUE )

    if( is.null( smplTree ) || is.null( termNodes ) )
      next

    for( nodeId in termNodes ) {

      # metrics for data (MC) observations in node
      nodeObs  <- rownames( smplTree[ nodeId ]$data )  # node data observ. #s
      nodeMetr <- smplData[ nodeObs, 1 ]     # node metric at each observation

      # metrics for other (out-of-node) observations
      otherMetr <- smplData[ ! rownames( smplData ) %in% nodeObs, 1 ]

      # U test to compare if node metric difference is significant
      t.pval <- NA

      if( irf$stat == "median" ) {
        try( t.pval <- suppressWarnings(
          stats::wilcox.test( nodeMetr, otherMetr, exact = FALSE,
                              digits.rank = 7 )$p.value ),
          silent = TRUE )
      } else {
        try( t.pval <- suppressWarnings(
          stats::t.test( nodeMetr, otherMetr )$p.value ),
          silent = TRUE )
      }

      if( is.na( t.pval ) )
        next

      # Decide whether to consider the splitting path to reach (economic state
      # of) node is interesting or not.
      #
      # Criteria:
      #   1) is this difference statistically significant?
      #   2) is the node large enough (observations) to be considered?
      #   3) is the node size smaller than the rest of the tree?
      #

      if( t.pval < alpha &&
          length( nodeMetr ) > nodesize  &&
          length( nodeMetr ) < length( otherMetr ) ) {

        # find the split path to reach node
        path <- NA
        try( path <- suppressWarnings(
          .list.rules.party( smplTree, nodeId ) ),
          silent = TRUE )

        if( is.na( path ) )
          next

        splitPos <- splitVar <- splitRel <- splitVal <- vector( )

        for( i in 2 : nVar ) {

          # find which variables are used in split and in which positions
          pos <- unlist( gregexpr( state.vars[ i ], path, fixed = TRUE ) )
          pos <- pos[ pos > 0 ]

          if( length( pos ) == 0 )
            next

          regex <- paste0( state.vars[ i ],
                           "[[:space:]]*([><]=?)[[:space:]]*([-.eE[:digit:]]+).*" )

          for( p in pos ) {
            rel <- sub( regex, "\\1", substr( path, p, nchar( path ) ) )
            val <- sub( regex, "\\2", substr( path, p, nchar( path ) ) )
            val <- suppressWarnings( as.numeric( val ) )

            if( ! rel %in% c( ">", ">=", "<", "<=" ) || ! is.finite( val ) )
              next

            splitPos <- append( splitPos, p )
            splitVar <- append( splitVar, state.vars[ i ] )
            splitRel <- append( splitRel, rel )
            splitVal <- append( splitVal, val )
          }
        }

        if( length( splitVar ) == 0 )
          next

        state <- state + 1            # new state found

        # organize all in the order variables appeared in split
        ord <- order( splitPos )
        splitVar <- splitVar[ ord ]
        splitRel <- splitRel[ ord ]
        splitVal <- splitVal[ ord ]

        # format the splitting path and store in states data frame
        for ( i in 1 : length( splitVar ) ) {
          states[ state, i * 3 - 2 ] <- splitVar[ i ]
          states[ state, i * 3 - 1 ] <- splitRel[ i ]
          states[ state, i * 3 ]     <- splitVal[ i ]
        }

        if( irf$stat == "median" )
          states[ state, 3 * maxdepth + 1 ] <- stats::median( nodeMetr,
                                                              na.rm = TRUE ) /
          allMetr - 1
        else
          states[ state, 3 * maxdepth + 1 ] <- mean( nodeMetr, na.rm = TRUE ) /
          allMetr - 1
      }
    }
  }

  if( state == 0 )
    stop( "No state/end node found, please check your data" )

  #
  # -------- Algorithm to find discrete states --------
  #

  # find quantile limits for state variables data
  probs <- seq( 0, 1, 1 / quantile )
  dataQuant <- matrix( nrow = quantile + 1, ncol = nVar - 1 )
  for( i in 2 : nVar )
    dataQuant[ , i - 1 ] <- stats::quantile( stateData[ , i ], probs = probs,
                                             type = 8 )
  colnames( dataQuant ) <- state.vars[ 2 : nVar ]

  stateDisc <- data.frame( matrix( nrow = nrow( states ), ncol = 2 ) )
  colnames( stateDisc ) <- c( "state", "metric" )
  varQuant <- list( )
  maxDep <- 0

  # discretize state thresholds at quantile levels and build state identifiers
  for( i in 1 : nrow( states ) ) {

    stateDisc$state[ i ] <- ""

    for( j in 1 : maxdepth ) {
      var <- states[ i, j * 3 - 2 ]
      rel <- substr( states[ i, j * 3 - 1 ], 1, 1 )
      val <- states[ i, j * 3 ]

      if( is.na( var ) || is.na( rel ) || is.na( val ) )
        next

      quant <- paste0( "q", findInterval( val, dataQuant[ , var ],
                                          all.inside = TRUE ) )

      # save state-defining tag
      stateDisc$state[ i ] <- paste0( stateDisc$state[ i ],
                                      ifelse( j > 1, " & ", "" ),
                                      var, " ", rel, " ", quant )
      stateDisc$metric[ i ] <- states[ i, 3 * maxdepth + 1 ]

      # save value to the variable quantile list
      if( ! exists( var, varQuant ) )
        varQuant[[ var ]] <- list( )

      if( ! exists( rel, varQuant[[ var ]] ) )
        varQuant[[ var ]][[ rel ]] <- list( )

      if( ! exists( quant, varQuant[[ var ]][[ rel ]] ) )
        varQuant[[ var ]][[ rel ]][[ quant ]] <- c( )

      varQuant[[ var ]][[ rel ]][[ quant ]] <-
        append( varQuant[[ var ]][[ rel ]][[ quant ]], val )

      if( j > maxDep )
        maxDep <- j
    }
  }

  # sort variable split in each discrete state if tree depth is more than one
  if( maxdepth > 1 )
    for( i in 1 : nrow( stateDisc ) )
      stateDisc$state[ i ] <- paste0( sort( strsplit( stateDisc$state[ i ],
                                                      " & ", fixed = TRUE )[[ 1 ]] ),
                                      collapse = " & " )

  # relative frequency of each discretized state tag
  stateFreq <- table( stateDisc$state )

  # estimate the mean/median relative metric of discrete states
  stateMetr <- stateAbsMetr <- vector( mode = "numeric", length = length( stateFreq ) )
  for( i in 1 : length( stateFreq ) ) {
    metrs <- stateDisc$metric[ which( stateDisc$state == names( stateFreq )[ i ] ) ]
    if( irf$stat == "median" ) {
      stateMetr[ i ] <- stats::median( metrs, na.rm = TRUE )
      stateAbsMetr[ i ] <- stats::median( abs( metrs ), na.rm = TRUE )
    } else {
      stateMetr[ i ] <- mean( metrs, na.rm = TRUE )
      stateAbsMetr[ i ] <- mean( abs( metrs ), na.rm = TRUE )
    }
  }

  stateFreq <- data.frame( stateFreq / sum( stateFreq ) )
  stateFreq <- cbind( stateFreq, stateMetr, stateAbsMetr )
  colnames( stateFreq ) <- c( "State", "Prob", "MetrD", "MetrAD" )
  stateFreq <- stateFreq[ order( stateFreq$Prob, decreasing = TRUE ), ]
  rownames( stateFreq ) <- 1 : nrow( stateFreq )

  if( irf$stat == "median" ) {
    statLab <- "qMed"
    distLab <- "qMAD"
  } else {
    statLab <- "qAvg"
    distLab <- "qSD"
  }

  # add information about quartiles
  qLabels <- vector( )
  if( maxDep > 1 )
    for( i in 1 : maxDep )
      qLabels <- append( qLabels, c( paste0( statLab, i ), paste0( distLab, i ),
                                     paste0( "qMin", i ), paste0( "qMax", i ) ) )
  else
    qLabels <- c( statLab, distLab, "qMin", "qMax" )

  stateFreq[ , qLabels ] <- NA
  regex <- "(.+) ([><]=?) (.+)"

  for( i in 1 : nrow( stateFreq ) ) {
    nodes <- unlist( strsplit( as.character( stateFreq$State )[ i ], " & ",
                               fixed = TRUE ) )

    for( j in 1 : length( nodes ) ) {
      var   <- sub( regex, "\\1", nodes[ j ] )
      rel   <- sub( regex, "\\2", nodes[ j ] )
      quant <- sub( regex, "\\3", nodes[ j ] )
      qData <- varQuant[[ var ]][[ rel ]][[ quant ]]

      if( irf$stat == "median" ) {
        stateFreq[ i, j * 4 + 1 ] <- stats::median( qData )
        stateFreq[ i, j * 4 + 2 ] <- stats::mad( qData )
      } else {
        stateFreq[ i, j * 4 + 1 ] <- mean( qData )
        stateFreq[ i, j * 4 + 2 ] <- stats::sd( qData )
      }

      stateFreq[ i, j * 4 + 3 ] <- min( qData )
      stateFreq[ i, j * 4 + 4 ] <- max( qData )
    }
  }

  sid <- list( state.freq = stateFreq, state.vars = state.vars[ 2 : nVar ],
               t.horiz = irf$t.horiz, var.irf = irf$var.irf,
               var.ref = irf$var.ref, stat = irf$stat, alpha = alpha,
               nsample = nMC, outliers = irf$outliers, ntree = ntree,
               maxdepth = maxdepth, nodesize = nodesize, mtry = mtry,
               quantile = quantile, state.cont = NULL, state.cont.num = state,
               call = match.call( ) )

  if( state.cont ) {
    # order states according to absolute metric deviation
    states <- states[ order( abs( states$MetrD ), decreasing = TRUE ), ]
    rownames( states ) <- 1 : nrow( states )
    sid[[ "state.cont" ]] <- states
  }

  class( sid ) <- "state.ident.lsd"

  return( sid )
}


# ====== State-variable sensitivity analysis print ======

print.state.ident.lsd <- function( x, ... ) {

  if( ! inherits( x, "state.ident.lsd" ) )
    stop( "Invalid object (not from state.ident.lsd())" )

  cat( "\nTested IRF state variables:",
       paste( x$state.vars, collapse = ", " ), "\n" )

  info <- data.frame( c( x$var.irf,
                         x$var.ref,
                         x$t.horiz,
                         x$state.cont.num,
                         nrow( x$state.freq ),
                         x$ntree,
                         x$maxdepth,
                         x$nodesize,
                         x$mtry,
                         x$alpha,
                         x$quantile,
                         x$nsample,
                         length( x$outliers ) ) )
  colnames( info ) <- NULL
  rownames( info ) <- c( "Impulse-response function variable:",
                         "Reference variable (impulse time):",
                         "Time-horizon response period:",
                         "Number of continuous states found:",
                         "Number of discrete states found:",
                         "Number of grown random trees:",
                         "Tree maximum depth:",
                         "Tree node minimum size:",
                         "Number of variables sampled per split:",
                         "Tree estimation significance:",
                         "Number of discrete states (quantiles):",
                         "Number of employed MC samples:",
                         "Number of outlier MC samples:" )
  print( info, ... )

  cat( "\nTop-10 discrete states:\n\n" )

  print( x$state.freq[ 1 : min( length( x$state.freq ), 10 ) ],
         row.names = FALSE, ... )
}
