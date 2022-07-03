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

# ====== Sensitivity analysis for state variables ======

state.sa.lsd <- function( data, irf, state.vars = NULL, metr.irf = NULL,
                          add.vars = NULL, ntree = 500, nodesize = 5,
                          mtry = max( floor( ifelse( ! is.null( state.vars ),
                                                     length( state.vars ),
                                                     dim( data )[ 2 ] ) / 3 ),
                                      1 ),
                          no.plot = FALSE, alpha = 0.05, seed = 1, ... ) {

  # check data, remove outliers, add new variables, and select state variables
  stateData <- build_state_data( data, irf, add.vars, state.vars,
                                 metr.irf = metr.irf )
  state.vars <- colnames( stateData )
  nVar <- length( state.vars )

  if( is.null( ntree ) || ! is.finite( ntree ) || round( ntree ) < 1 )
    stop( "Invalid number of trees (ntree)" )

  if( is.null( nodesize  ) || ! is.finite( nodesize  ) || round( nodesize )  < 1 )
    stop( "Invalid node minimum size (nodesize )" )

  if( is.null( mtry ) || ! is.finite( mtry ) || round( mtry ) < 1 )
    stop( "Invalid number of variable samples per node (mtry)" )

  if( is.null( alpha ) || ! is.finite( alpha ) || alpha <= 0 || alpha > 0.5 )
    stop( "Invalid significance level (alpha)" )

  if( ! is.null( seed ) && ! is.finite( seed ) )
    stop( "Invalid random seed (seed)" )

  if( is.null( no.plot ) || ! is.logical( no.plot ) )
    no.plot <- FALSE

  ntree    <- round( ntree )
  nodesize <- round( nodesize )
  mtry     <- round( mtry )

  # create regression formula to use in trees
  formula <- paste( state.vars[ 1 ], "~", state.vars[ 2 ] )
  if( nVar > 2 )
    for( i in 3 : nVar )
      formula <- paste( formula, "+", state.vars[ i ] )

  formula <- stats::as.formula( formula )

  set.seed( seed )      # reset PRNG seed to ensure reproducibility

  # estimation of random forest regression model
  forest <- randomForest::randomForest( formula, stateData, ntree = ntree,
                                        nodesize = nodesize, mtry = mtry,
                                        importance = TRUE )

  importance <- data.frame( randomForest::importance( forest, type = 1,
                                                      scale = FALSE ),
                            forest$importanceSD,
                            round( stats::pt( forest$importance[ , 1 ] /
                                                forest$importanceSD, ntree - 1,
                                              lower.tail = FALSE ), 4 ) )
  colnames( importance ) <- c( "MDA", "se", "p.value" )

  rfsa <- list( importance = importance,
                state.vars = state.vars[ 2 : nVar ], t.horiz = length( irf$irf ),
                var.irf = irf$var.irf, var.ref = irf$var.ref, stat = irf$stat,
                alpha = alpha, nsample = nrow( irf$ir ), outliers = irf$outliers,
                ntree = ntree, nodesize = nodesize, mtry = mtry,
                rsq = abs( mean( forest$rsq ) ), call = match.call( ) )
  class( rfsa ) <- "state.sa.lsd"

  if( ! no.plot )
    plot.state.sa.lsd( rfsa, alpha = alpha, ... )

  return( rfsa )
}


# ====== State-variable sensitivity analysis print/plot ======

print.state.sa.lsd <- function( x, ... ) {

  if( ! inherits( x, "state.sa.lsd" ) )
    stop( "Invalid object (not from state.sa.lsd())" )

  cat( "\nTested IRF state variables:",
       paste( x$state.vars, collapse = ", " ), "\n" )

  info <- data.frame( c( x$var.irf,
                         x$var.ref,
                         x$t.horiz,
                         x$ntree,
                         x$nodesize,
                         x$mtry,
                         signif( x$rsq, digits = 2 ),
                         x$nsample,
                         length( x$outliers ) ) )
  colnames( info ) <- NULL
  rownames( info ) <- c( "Impulse-response function variable:",
                         "Reference variable (impulse time):",
                         "Time-horizon response period:",
                         "Number of grown random trees:",
                         "Tree node minimum size:",
                         "Number of variables sampled per split:",
                         "Mean pseudo R-squared:",
                         "Number of employed MC samples:",
                         "Number of outlier MC samples:" )
  print( info, ... )

  if( ! is.null( x$importance ) && length( x$importance ) > 0 ) {

    cat( "\nImportance of state-defining variables:\n\n" )

    print( x$importance[ order( x$importance$MDA, decreasing = TRUE ), ],
           ... )
  }
}


plot.state.sa.lsd <- function( x, ... ) {

  if( ! inherits( x, "state.sa.lsd" ) )
    stop( "Invalid state irf object (not from state.sa.lsd())" )

  data <- x$importance[ order( x$importance$MDA, decreasing = TRUE ), ]
  ciAmpl <- stats::qt( 1 - x$alpha / 2, df = x$ntree - 1 ) * data$se

  gplots::barplot2( data$MDA, names.arg = rownames( data ), plot.ci = TRUE,
                    ci.l = data$MDA - ciAmpl, ci.u = data$MDA + ciAmpl,
                    ylab = "Mean Decrease in Accuracy", ... )
}
