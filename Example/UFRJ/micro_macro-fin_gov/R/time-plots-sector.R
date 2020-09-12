#******************************************************************
#
# ---------------- Aggregates time plots --------------------
#
#******************************************************************

# remove warnings for support functions
# !diagnostics suppress = log0, plot_lists, hpfilter, colSds, colMins, colMaxs


time_plots_sector <- function( mcData, Adata, mdata, Mdata, Sdata, nExp, nSize, nTsteps,
                        TmaskPlot, CI, legends, colors, lTypes, transMk, smoothing,
                        firmTypes ) {

  # ------ Sector Avg Price ------

  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( log0( Adata[[ k ]]$P_C[ TmaskPlot ] ),
                       log0( Adata[[ k ]]$P_K[ TmaskPlot ] ),
                       log0( Adata[[ k ]]$P_I[ TmaskPlot ] ) )
    # minimum and maximum MC runs
    min[[ k ]] <- list( log0( mdata[[ k ]]$P_C[ TmaskPlot ] ),
                      log0( mdata[[ k ]]$P_K[ TmaskPlot ] ),
                      log0( mdata[[ k ]]$P_I[ TmaskPlot ] ) )
    max[[ k ]] <- list( log0( Mdata[[ k ]]$P_C[ TmaskPlot ] ),
                      log0( Mdata[[ k ]]$P_K[ TmaskPlot ] ),
                      log0( Mdata[[ k ]]$P_I[ TmaskPlot ] ) )
    # MC confidence interval
    lo[[ k ]] <- list( log0( Adata[[ k ]]$P_C[ TmaskPlot ] -
                             qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$P_C[ TmaskPlot ] / sqrt( nSize ) ),
                     log0( Adata[[ k ]]$P_K[ TmaskPlot ] -
                             qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$P_K[ TmaskPlot ] / sqrt( nSize ) ),
                     log0( Adata[[ k ]]$P_I[ TmaskPlot ] -
                             qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$P_I[ TmaskPlot ]  / sqrt( nSize ) ) )
    hi[[ k ]] <- list( log0( Adata[[ k ]]$P_C[ TmaskPlot ] +
                             qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$P_C[ TmaskPlot ] / sqrt( nSize ) ),
                     log0( Adata[[ k ]]$P_K[ TmaskPlot ] +
                             qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$P_K[ TmaskPlot ] / sqrt( nSize ) ),
                     log0( Adata[[ k ]]$P_I[ TmaskPlot ] +
                             qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$P_I[ TmaskPlot ]  / sqrt( nSize ) ) )
  }

  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Logs",
              tit = "Sector Average Price",
              subtit = paste( "MC runs =", nSize ),
              leg2 = c( "Consumption Sector", "Capital Sector", "Intermediate Sector" ) )
  
  # ------ Consumption Sector Price------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( log0(Adata[[ k ]]$P_C[ TmaskPlot ]) )
    # minimum and maximum MC runs
    min[[ k ]] <- list( log0(mdata[[ k ]]$P_C[ TmaskPlot ]))
    max[[ k ]] <- list( log0(Mdata[[ k ]]$P_C[ TmaskPlot ]))
    # MC confidence interval
    lo[[ k ]] <- list( log0(Adata[[ k ]]$P_C[ TmaskPlot ] -
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$P_C[ TmaskPlot ] / sqrt( nSize )))
    hi[[ k ]] <- list( log0(Adata[[ k ]]$P_C[ TmaskPlot ] +
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$P_C[ TmaskPlot ] / sqrt( nSize )))
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Log",
              tit = "Consumption Good Sector Average Price",
              subtit = paste( "MC runs =", nSize ) )
  
  # ------ Capital Sector Price------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( log0(Adata[[ k ]]$P_K[ TmaskPlot ]) )
    # minimum and maximum MC runs
    min[[ k ]] <- list( log0(mdata[[ k ]]$P_K[ TmaskPlot ]))
    max[[ k ]] <- list( log0(Mdata[[ k ]]$P_K[ TmaskPlot ]))
    # MC confidence interval
    lo[[ k ]] <- list( log0(Adata[[ k ]]$P_K[ TmaskPlot ] -
                              qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$P_K[ TmaskPlot ] / sqrt( nSize )))
    hi[[ k ]] <- list( log0(Adata[[ k ]]$P_K[ TmaskPlot ] +
                              qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$P_K[ TmaskPlot ] / sqrt( nSize )))
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Log",
              tit = "Capital Good Sector Average Price",
              subtit = paste( "MC runs =", nSize ) )
  
  
  # ------ Intermediate Sector Price------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( log0(Adata[[ k ]]$P_I[ TmaskPlot ]) )
    # minimum and maximum MC runs
    min[[ k ]] <- list( log0(mdata[[ k ]]$P_I[ TmaskPlot ]))
    max[[ k ]] <- list( log0(Mdata[[ k ]]$P_I[ TmaskPlot ]))
    # MC confidence interval
    lo[[ k ]] <- list( log0(Adata[[ k ]]$P_I[ TmaskPlot ] -
                              qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$P_I[ TmaskPlot ] / sqrt( nSize )))
    hi[[ k ]] <- list( log0(Adata[[ k ]]$P_I[ TmaskPlot ] +
                              qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$P_I[ TmaskPlot ] / sqrt( nSize )))
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Log",
              tit = "Intermediate Good Sector Average Price",
              subtit = paste( "MC runs =", nSize ) )


  # ------ Sector Avg Wage ------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( log0( Adata[[ k ]]$W_C[ TmaskPlot ] ),
                         log0( Adata[[ k ]]$W_K[ TmaskPlot ] ),
                         log0( Adata[[ k ]]$W_I[ TmaskPlot ] ) )
    # minimum and maximum MC runs
    min[[ k ]] <- list( log0( mdata[[ k ]]$W_C[ TmaskPlot ] ),
                        log0( mdata[[ k ]]$W_K[ TmaskPlot ] ),
                        log0( mdata[[ k ]]$W_I[ TmaskPlot ] ) )
    max[[ k ]] <- list( log0( Mdata[[ k ]]$W_C[ TmaskPlot ] ),
                        log0( Mdata[[ k ]]$W_K[ TmaskPlot ] ),
                        log0( Mdata[[ k ]]$W_I[ TmaskPlot ] ) )
    # MC confidence interval
    lo[[ k ]] <- list( log0( Adata[[ k ]]$W_C[ TmaskPlot ] -
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$W_C[ TmaskPlot ] / sqrt( nSize ) ),
                       log0( Adata[[ k ]]$W_K[ TmaskPlot ] -
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$W_K[ TmaskPlot ] / sqrt( nSize ) ),
                       log0( Adata[[ k ]]$W_I[ TmaskPlot ] -
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$W_I[ TmaskPlot ]  / sqrt( nSize ) ) )
    hi[[ k ]] <- list( log0( Adata[[ k ]]$W_C[ TmaskPlot ] +
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$W_C[ TmaskPlot ] / sqrt( nSize ) ),
                       log0( Adata[[ k ]]$W_K[ TmaskPlot ] +
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$W_K[ TmaskPlot ] / sqrt( nSize ) ),
                       log0( Adata[[ k ]]$W_I[ TmaskPlot ] +
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$W_I[ TmaskPlot ]  / sqrt( nSize ) ) )
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Logs",
              tit = "Sector Average Wage",
              subtit = paste( "MC runs =", nSize ),
              leg2 = c( "Consumption Sector", "Capital Sector", "Intermediate Sector" ) )
  
  # ------ Consumption Sector Wage------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( log0(Adata[[ k ]]$W_C[ TmaskPlot ]) )
    # minimum and maximum MC runs
    min[[ k ]] <- list( log0(mdata[[ k ]]$W_C[ TmaskPlot ]))
    max[[ k ]] <- list( log0(Mdata[[ k ]]$W_C[ TmaskPlot ]))
    # MC confidence interval
    lo[[ k ]] <- list( log0(Adata[[ k ]]$W_C[ TmaskPlot ] -
                              qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$W_C[ TmaskPlot ] / sqrt( nSize )))
    hi[[ k ]] <- list( log0(Adata[[ k ]]$W_C[ TmaskPlot ] +
                              qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$W_C[ TmaskPlot ] / sqrt( nSize )))
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Log",
              tit = "Consumption Good Sector Average Wage",
              subtit = paste( "MC runs =", nSize ) )
  
  # ------ Capital Sector Wage------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( log0(Adata[[ k ]]$W_K[ TmaskPlot ]) )
    # minimum and maximum MC runs
    min[[ k ]] <- list( log0(mdata[[ k ]]$W_K[ TmaskPlot ]))
    max[[ k ]] <- list( log0(Mdata[[ k ]]$W_K[ TmaskPlot ]))
    # MC confidence interval
    lo[[ k ]] <- list( log0(Adata[[ k ]]$W_K[ TmaskPlot ] -
                              qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$W_K[ TmaskPlot ] / sqrt( nSize )))
    hi[[ k ]] <- list( log0(Adata[[ k ]]$W_K[ TmaskPlot ] +
                              qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$W_K[ TmaskPlot ] / sqrt( nSize )))
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Log",
              tit = "Capital Good Sector Average Wage",
              subtit = paste( "MC runs =", nSize ) )
  
  
  # ------ Intermediate Sector Wage------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( log0(Adata[[ k ]]$W_I[ TmaskPlot ]) )
    # minimum and maximum MC runs
    min[[ k ]] <- list( log0(mdata[[ k ]]$W_I[ TmaskPlot ]))
    max[[ k ]] <- list( log0(Mdata[[ k ]]$W_I[ TmaskPlot ]))
    # MC confidence interval
    lo[[ k ]] <- list( log0(Adata[[ k ]]$W_I[ TmaskPlot ] -
                              qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$W_I[ TmaskPlot ] / sqrt( nSize )))
    hi[[ k ]] <- list( log0(Adata[[ k ]]$W_I[ TmaskPlot ] +
                              qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$W_I[ TmaskPlot ] / sqrt( nSize )))
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Log",
              tit = "Intermediate Good Sector Average Wage",
              subtit = paste( "MC runs =", nSize ) )
  
  
  # ------ Sector Avg Markup ------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( log0( Adata[[ k ]]$MK_C[ TmaskPlot ] ),
                         log0( Adata[[ k ]]$MK_K[ TmaskPlot ] ),
                         log0( Adata[[ k ]]$MK_I[ TmaskPlot ] ) )
    # minimum and maximum MC runs
    min[[ k ]] <- list( log0( mdata[[ k ]]$MK_C[ TmaskPlot ] ),
                        log0( mdata[[ k ]]$MK_K[ TmaskPlot ] ),
                        log0( mdata[[ k ]]$MK_I[ TmaskPlot ] ) )
    max[[ k ]] <- list( log0( Mdata[[ k ]]$MK_C[ TmaskPlot ] ),
                        log0( Mdata[[ k ]]$MK_K[ TmaskPlot ] ),
                        log0( Mdata[[ k ]]$MK_I[ TmaskPlot ] ) )
    # MC confidence interval
    lo[[ k ]] <- list( log0( Adata[[ k ]]$MK_C[ TmaskPlot ] -
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$MK_C[ TmaskPlot ] / sqrt( nSize ) ),
                       log0( Adata[[ k ]]$MK_K[ TmaskPlot ] -
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$MK_K[ TmaskPlot ] / sqrt( nSize ) ),
                       log0( Adata[[ k ]]$MK_I[ TmaskPlot ] -
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$MK_I[ TmaskPlot ]  / sqrt( nSize ) ) )
    hi[[ k ]] <- list( log0( Adata[[ k ]]$MK_C[ TmaskPlot ] +
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$MK_C[ TmaskPlot ] / sqrt( nSize ) ),
                       log0( Adata[[ k ]]$MK_K[ TmaskPlot ] +
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$MK_K[ TmaskPlot ] / sqrt( nSize ) ),
                       log0( Adata[[ k ]]$MK_I[ TmaskPlot ] +
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$MK_I[ TmaskPlot ]  / sqrt( nSize ) ) )
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Logs",
              tit = "Sector Average Markup",
              subtit = paste( "MC runs =", nSize ),
              leg2 = c( "Consumption Sector", "Capital Sector", "Intermediate Sector" ) )
  
  # ------ Consumption Sector Markup------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( log0(Adata[[ k ]]$MK_C[ TmaskPlot ]) )
    # minimum and maximum MC runs
    min[[ k ]] <- list( log0(mdata[[ k ]]$MK_C[ TmaskPlot ]))
    max[[ k ]] <- list( log0(Mdata[[ k ]]$MK_C[ TmaskPlot ]))
    # MC confidence interval
    lo[[ k ]] <- list( log0(Adata[[ k ]]$MK_C[ TmaskPlot ] -
                              qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$MK_C[ TmaskPlot ] / sqrt( nSize )))
    hi[[ k ]] <- list( log0(Adata[[ k ]]$MK_C[ TmaskPlot ] +
                              qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$MK_C[ TmaskPlot ] / sqrt( nSize )))
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Log",
              tit = "Consumption Good Sector Average Markup",
              subtit = paste( "MC runs =", nSize ) )
  
  # ------ Capital Sector Markup------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( log0(Adata[[ k ]]$MK_K[ TmaskPlot ]) )
    # minimum and maximum MC runs
    min[[ k ]] <- list( log0(mdata[[ k ]]$MK_K[ TmaskPlot ]))
    max[[ k ]] <- list( log0(Mdata[[ k ]]$MK_K[ TmaskPlot ]))
    # MC confidence interval
    lo[[ k ]] <- list( log0(Adata[[ k ]]$MK_K[ TmaskPlot ] -
                              qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$MK_K[ TmaskPlot ] / sqrt( nSize )))
    hi[[ k ]] <- list( log0(Adata[[ k ]]$MK_K[ TmaskPlot ] +
                              qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$MK_K[ TmaskPlot ] / sqrt( nSize )))
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Log",
              tit = "Capital Good Sector Average Markup",
              subtit = paste( "MC runs =", nSize ) )
  
  
  # ------ Intermediate Sector Markup------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( log0(Adata[[ k ]]$MK_I[ TmaskPlot ]) )
    # minimum and maximum MC runs
    min[[ k ]] <- list( log0(mdata[[ k ]]$MK_I[ TmaskPlot ]))
    max[[ k ]] <- list( log0(Mdata[[ k ]]$MK_I[ TmaskPlot ]))
    # MC confidence interval
    lo[[ k ]] <- list( log0(Adata[[ k ]]$MK_I[ TmaskPlot ] -
                              qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$MK_I[ TmaskPlot ] / sqrt( nSize )))
    hi[[ k ]] <- list( log0(Adata[[ k ]]$MK_I[ TmaskPlot ] +
                              qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$MK_I[ TmaskPlot ] / sqrt( nSize )))
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Log",
              tit = "Intermediate Good Sector Average Markup",
              subtit = paste( "MC runs =", nSize ) )
  
  
  
  # ------ Sector Avg Productivity ------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( log0( Adata[[ k ]]$PROD_C[ TmaskPlot ] ),
                         log0( Adata[[ k ]]$PROD_K[ TmaskPlot ] ),
                         log0( Adata[[ k ]]$PROD_I[ TmaskPlot ] ) )
    # minimum and maximum MC runs
    min[[ k ]] <- list( log0( mdata[[ k ]]$PROD_C[ TmaskPlot ] ),
                        log0( mdata[[ k ]]$PROD_K[ TmaskPlot ] ),
                        log0( mdata[[ k ]]$PROD_I[ TmaskPlot ] ) )
    max[[ k ]] <- list( log0( Mdata[[ k ]]$PROD_C[ TmaskPlot ] ),
                        log0( Mdata[[ k ]]$PROD_K[ TmaskPlot ] ),
                        log0( Mdata[[ k ]]$PROD_I[ TmaskPlot ] ) )
    # MC confidence interval
    lo[[ k ]] <- list( log0( Adata[[ k ]]$PROD_C[ TmaskPlot ] -
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$PROD_C[ TmaskPlot ] / sqrt( nSize ) ),
                       log0( Adata[[ k ]]$PROD_K[ TmaskPlot ] -
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$PROD_K[ TmaskPlot ] / sqrt( nSize ) ),
                       log0( Adata[[ k ]]$PROD_I[ TmaskPlot ] -
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$PROD_I[ TmaskPlot ]  / sqrt( nSize ) ) )
    hi[[ k ]] <- list( log0( Adata[[ k ]]$PROD_C[ TmaskPlot ] +
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$PROD_C[ TmaskPlot ] / sqrt( nSize ) ),
                       log0( Adata[[ k ]]$PROD_K[ TmaskPlot ] +
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$PROD_K[ TmaskPlot ] / sqrt( nSize ) ),
                       log0( Adata[[ k ]]$PROD_I[ TmaskPlot ] +
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$PROD_I[ TmaskPlot ]  / sqrt( nSize ) ) )
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Logs",
              tit = "Sector Average Productivity",
              subtit = paste( "MC runs =", nSize ),
              leg2 = c( "Consumption Sector", "Capital Sector", "Intermediate Sector" ) )
  
  # ------ Consumption Sector Productivity------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( log0(Adata[[ k ]]$PROD_C[ TmaskPlot ]) )
    # minimum and maximum MC runs
    min[[ k ]] <- list( log0(mdata[[ k ]]$PROD_C[ TmaskPlot ]))
    max[[ k ]] <- list( log0(Mdata[[ k ]]$PROD_C[ TmaskPlot ]))
    # MC confidence interval
    lo[[ k ]] <- list( log0(Adata[[ k ]]$PROD_C[ TmaskPlot ] -
                              qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$PROD_C[ TmaskPlot ] / sqrt( nSize )))
    hi[[ k ]] <- list( log0(Adata[[ k ]]$PROD_C[ TmaskPlot ] +
                              qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$PROD_C[ TmaskPlot ] / sqrt( nSize )))
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Log",
              tit = "Consumption Good Sector Average Productivity",
              subtit = paste( "MC runs =", nSize ) )
  
  # ------ Capital Sector Productivity------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( log0(Adata[[ k ]]$PROD_K[ TmaskPlot ]) )
    # minimum and maximum MC runs
    min[[ k ]] <- list( log0(mdata[[ k ]]$PROD_K[ TmaskPlot ]))
    max[[ k ]] <- list( log0(Mdata[[ k ]]$PROD_K[ TmaskPlot ]))
    # MC confidence interval
    lo[[ k ]] <- list( log0(Adata[[ k ]]$PROD_K[ TmaskPlot ] -
                              qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$PROD_K[ TmaskPlot ] / sqrt( nSize )))
    hi[[ k ]] <- list( log0(Adata[[ k ]]$PROD_K[ TmaskPlot ] +
                              qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$PROD_K[ TmaskPlot ] / sqrt( nSize )))
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Log",
              tit = "Capital Good Sector Average Productivity",
              subtit = paste( "MC runs =", nSize ) )
  
  
  # ------ Intermediate Sector Productivity------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( log0(Adata[[ k ]]$PROD_I[ TmaskPlot ]) )
    # minimum and maximum MC runs
    min[[ k ]] <- list( log0(mdata[[ k ]]$PROD_I[ TmaskPlot ]))
    max[[ k ]] <- list( log0(Mdata[[ k ]]$PROD_I[ TmaskPlot ]))
    # MC confidence interval
    lo[[ k ]] <- list( log0(Adata[[ k ]]$PROD_I[ TmaskPlot ] -
                              qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$PROD_I[ TmaskPlot ] / sqrt( nSize )))
    hi[[ k ]] <- list( log0(Adata[[ k ]]$PROD_I[ TmaskPlot ] +
                              qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$PROD_I[ TmaskPlot ] / sqrt( nSize )))
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Log",
              tit = "Intermediate Good Sector Average Productivity",
              subtit = paste( "MC runs =", nSize ) )
  
  
  
  # ------ Sector Unemployment ------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( Adata[[ k ]]$U_C[ TmaskPlot ] ,
                         Adata[[ k ]]$U_K[ TmaskPlot ] ,
                         Adata[[ k ]]$U_I[ TmaskPlot ] )
    # minimum and maximum MC runs
    min[[ k ]] <- list( mdata[[ k ]]$U_C[ TmaskPlot ],
                        mdata[[ k ]]$U_K[ TmaskPlot ],
                        mdata[[ k ]]$U_I[ TmaskPlot ] )
    max[[ k ]] <- list( Mdata[[ k ]]$U_C[ TmaskPlot ],
                        Mdata[[ k ]]$U_K[ TmaskPlot ],
                        Mdata[[ k ]]$U_I[ TmaskPlot ] )
    # MC confidence interval
    lo[[ k ]] <- list( Adata[[ k ]]$U_C[ TmaskPlot ] -
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$U_C[ TmaskPlot ] / sqrt( nSize ) ,
                       Adata[[ k ]]$U_K[ TmaskPlot ] -
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$U_K[ TmaskPlot ] / sqrt( nSize ) ,
                       Adata[[ k ]]$U_I[ TmaskPlot ] -
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$U_I[ TmaskPlot ]  / sqrt( nSize ) )
    hi[[ k ]] <- list( Adata[[ k ]]$U_C[ TmaskPlot ] +
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$U_C[ TmaskPlot ] / sqrt( nSize ) ,
                       Adata[[ k ]]$U_K[ TmaskPlot ] +
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$U_K[ TmaskPlot ] / sqrt( nSize ) ,
                       Adata[[ k ]]$U_I[ TmaskPlot ] +
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$U_I[ TmaskPlot ]  / sqrt( nSize ) )
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Rate",
              tit = "Sector Unemployment",
              subtit = paste( "MC runs =", nSize ),
              leg2 = c( "Consumption Sector", "Capital Sector", "Intermediate Sector" ) )
  
  # ------ Consumption Sector Unemployment------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( Adata[[ k ]]$U_C[ TmaskPlot ]) 
    # minimum and maximum MC runs
    min[[ k ]] <- list( mdata[[ k ]]$U_C[ TmaskPlot ])
    max[[ k ]] <- list( Mdata[[ k ]]$U_C[ TmaskPlot ])
    # MC confidence interval
    lo[[ k ]] <- list( Adata[[ k ]]$U_C[ TmaskPlot ] -
                              qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$U_C[ TmaskPlot ] / sqrt( nSize ))
    hi[[ k ]] <- list( Adata[[ k ]]$U_C[ TmaskPlot ] +
                              qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$U_C[ TmaskPlot ] / sqrt( nSize ))
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Rate",
              tit = "Consumption Good Sector Unemployment",
              subtit = paste( "MC runs =", nSize ) )
  
  # ------ Capital Sector Unemployment------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( Adata[[ k ]]$U_K[ TmaskPlot ]) 
    # minimum and maximum MC runs
    min[[ k ]] <- list( mdata[[ k ]]$U_K[ TmaskPlot ])
    max[[ k ]] <- list( Mdata[[ k ]]$U_K[ TmaskPlot ])
    # MC confidence interval
    lo[[ k ]] <- list( Adata[[ k ]]$U_K[ TmaskPlot ] -
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$U_K[ TmaskPlot ] / sqrt( nSize ))
    hi[[ k ]] <- list( Adata[[ k ]]$U_K[ TmaskPlot ] +
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$U_K[ TmaskPlot ] / sqrt( nSize ))
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Rate",
              tit = "Capital Good Sector Unemployment",
              subtit = paste( "MC runs =", nSize ) )
  
  
  # ------ Intermediate Sector Unemployment------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( Adata[[ k ]]$U_I[ TmaskPlot ]) 
    # minimum and maximum MC runs
    min[[ k ]] <- list( mdata[[ k ]]$U_I[ TmaskPlot ])
    max[[ k ]] <- list( Mdata[[ k ]]$U_I[ TmaskPlot ])
    # MC confidence interval
    lo[[ k ]] <- list( Adata[[ k ]]$U_I[ TmaskPlot ] -
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$U_I[ TmaskPlot ] / sqrt( nSize ))
    hi[[ k ]] <- list( Adata[[ k ]]$U_I[ TmaskPlot ] +
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$U_I[ TmaskPlot ] / sqrt( nSize ))
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Rate",
              tit = "Intermediate Good Sector Unemployment",
              subtit = paste( "MC runs =", nSize ) )
  
  # ------ Sector Avg Debt Rate ------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( Adata[[ k ]]$DEBT_C[ TmaskPlot ] ,
                         Adata[[ k ]]$DEBT_K[ TmaskPlot ] ,
                         Adata[[ k ]]$DEBT_I[ TmaskPlot ] )
    # minimum and maximum MC runs
    min[[ k ]] <- list( mdata[[ k ]]$DEBT_C[ TmaskPlot ],
                        mdata[[ k ]]$DEBT_K[ TmaskPlot ],
                        mdata[[ k ]]$DEBT_I[ TmaskPlot ] )
    max[[ k ]] <- list( Mdata[[ k ]]$DEBT_C[ TmaskPlot ],
                        Mdata[[ k ]]$DEBT_K[ TmaskPlot ],
                        Mdata[[ k ]]$DEBT_I[ TmaskPlot ] )
    # MC confidence interval
    lo[[ k ]] <- list( Adata[[ k ]]$DEBT_C[ TmaskPlot ] -
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$DEBT_C[ TmaskPlot ] / sqrt( nSize ) ,
                       Adata[[ k ]]$DEBT_K[ TmaskPlot ] -
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$DEBT_K[ TmaskPlot ] / sqrt( nSize ) ,
                       Adata[[ k ]]$DEBT_I[ TmaskPlot ] -
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$DEBT_I[ TmaskPlot ]  / sqrt( nSize ) )
    hi[[ k ]] <- list( Adata[[ k ]]$DEBT_C[ TmaskPlot ] +
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$DEBT_C[ TmaskPlot ] / sqrt( nSize ) ,
                       Adata[[ k ]]$DEBT_K[ TmaskPlot ] +
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$DEBT_K[ TmaskPlot ] / sqrt( nSize ) ,
                       Adata[[ k ]]$DEBT_I[ TmaskPlot ] +
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$DEBT_I[ TmaskPlot ]  / sqrt( nSize ) )
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Rate",
              tit = "Sector Avg Debt Rate",
              subtit = paste( "MC runs =", nSize ),
              leg2 = c( "Consumption Sector", "Capital Sector", "Intermediate Sector" ) )
  
  # ------ Consumption Sector Avg Debt Rate------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( Adata[[ k ]]$DEBT_C[ TmaskPlot ]) 
    # minimum and maximum MC runs
    min[[ k ]] <- list( mdata[[ k ]]$DEBT_C[ TmaskPlot ])
    max[[ k ]] <- list( Mdata[[ k ]]$DEBT_C[ TmaskPlot ])
    # MC confidence interval
    lo[[ k ]] <- list( Adata[[ k ]]$DEBT_C[ TmaskPlot ] -
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$DEBT_C[ TmaskPlot ] / sqrt( nSize ))
    hi[[ k ]] <- list( Adata[[ k ]]$DEBT_C[ TmaskPlot ] +
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$DEBT_C[ TmaskPlot ] / sqrt( nSize ))
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Rate",
              tit = "Consumption Good Sector Avg Debt Rate",
              subtit = paste( "MC runs =", nSize ) )
  
  # ------ Capital Sector Avg Debt Rate------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( Adata[[ k ]]$DEBT_K[ TmaskPlot ]) 
    # minimum and maximum MC runs
    min[[ k ]] <- list( mdata[[ k ]]$DEBT_K[ TmaskPlot ])
    max[[ k ]] <- list( Mdata[[ k ]]$DEBT_K[ TmaskPlot ])
    # MC confidence interval
    lo[[ k ]] <- list( Adata[[ k ]]$DEBT_K[ TmaskPlot ] -
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$DEBT_K[ TmaskPlot ] / sqrt( nSize ))
    hi[[ k ]] <- list( Adata[[ k ]]$DEBT_K[ TmaskPlot ] +
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$DEBT_K[ TmaskPlot ] / sqrt( nSize ))
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Rate",
              tit = "Capital Good Sector Avg Debt Rate",
              subtit = paste( "MC runs =", nSize ) )
  
  
  # ------ Intermediate Sector Avg Debt Rate------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( Adata[[ k ]]$DEBT_I[ TmaskPlot ]) 
    # minimum and maximum MC runs
    min[[ k ]] <- list( mdata[[ k ]]$DEBT_I[ TmaskPlot ])
    max[[ k ]] <- list( Mdata[[ k ]]$DEBT_I[ TmaskPlot ])
    # MC confidence interval
    lo[[ k ]] <- list( Adata[[ k ]]$DEBT_I[ TmaskPlot ] -
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$DEBT_I[ TmaskPlot ] / sqrt( nSize ))
    hi[[ k ]] <- list( Adata[[ k ]]$DEBT_I[ TmaskPlot ] +
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$DEBT_I[ TmaskPlot ] / sqrt( nSize ))
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Rate",
              tit = "Intermediate Good Sector Avg Debt Rate",
              subtit = paste( "MC runs =", nSize ) )
  
  
  # ------ Sector Inverse HHI ------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( Adata[[ k ]]$HHI_C[ TmaskPlot ] ,
                         Adata[[ k ]]$HHI_K[ TmaskPlot ] ,
                         Adata[[ k ]]$HHI_I[ TmaskPlot ] )
    # minimum and maximum MC runs
    min[[ k ]] <- list( mdata[[ k ]]$HHI_C[ TmaskPlot ],
                        mdata[[ k ]]$HHI_K[ TmaskPlot ],
                        mdata[[ k ]]$HHI_I[ TmaskPlot ] )
    max[[ k ]] <- list( Mdata[[ k ]]$HHI_C[ TmaskPlot ],
                        Mdata[[ k ]]$HHI_K[ TmaskPlot ],
                        Mdata[[ k ]]$HHI_I[ TmaskPlot ] )
    # MC confidence interval
    lo[[ k ]] <- list( Adata[[ k ]]$HHI_C[ TmaskPlot ] -
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$HHI_C[ TmaskPlot ] / sqrt( nSize ) ,
                       Adata[[ k ]]$HHI_K[ TmaskPlot ] -
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$HHI_K[ TmaskPlot ] / sqrt( nSize ) ,
                       Adata[[ k ]]$HHI_I[ TmaskPlot ] -
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$HHI_I[ TmaskPlot ]  / sqrt( nSize ) )
    hi[[ k ]] <- list( Adata[[ k ]]$HHI_C[ TmaskPlot ] +
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$HHI_C[ TmaskPlot ] / sqrt( nSize ) ,
                       Adata[[ k ]]$HHI_K[ TmaskPlot ] +
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$HHI_K[ TmaskPlot ] / sqrt( nSize ) ,
                       Adata[[ k ]]$HHI_I[ TmaskPlot ] +
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$HHI_I[ TmaskPlot ]  / sqrt( nSize ) )
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Index",
              tit = "Sector Inverse HHI",
              subtit = paste( "MC runs =", nSize ),
              leg2 = c( "Consumption Sector", "Capital Sector", "Intermediate Sector" ) )
  
  # ------ Consumption Sector Inverse HHI------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( Adata[[ k ]]$HHI_C[ TmaskPlot ]) 
    # minimum and maximum MC runs
    min[[ k ]] <- list( mdata[[ k ]]$HHI_C[ TmaskPlot ])
    max[[ k ]] <- list( Mdata[[ k ]]$HHI_C[ TmaskPlot ])
    # MC confidence interval
    lo[[ k ]] <- list( Adata[[ k ]]$HHI_C[ TmaskPlot ] -
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$HHI_C[ TmaskPlot ] / sqrt( nSize ))
    hi[[ k ]] <- list( Adata[[ k ]]$HHI_C[ TmaskPlot ] +
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$HHI_C[ TmaskPlot ] / sqrt( nSize ))
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Index",
              tit = "Consumption Good Sector Inverse HHI",
              subtit = paste( "MC runs =", nSize ) )
  
  # ------ Capital Sector Inverse HHI------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( Adata[[ k ]]$HHI_K[ TmaskPlot ]) 
    # minimum and maximum MC runs
    min[[ k ]] <- list( mdata[[ k ]]$HHI_K[ TmaskPlot ])
    max[[ k ]] <- list( Mdata[[ k ]]$HHI_K[ TmaskPlot ])
    # MC confidence interval
    lo[[ k ]] <- list( Adata[[ k ]]$HHI_K[ TmaskPlot ] -
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$HHI_K[ TmaskPlot ] / sqrt( nSize ))
    hi[[ k ]] <- list( Adata[[ k ]]$HHI_K[ TmaskPlot ] +
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$HHI_K[ TmaskPlot ] / sqrt( nSize ))
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Index",
              tit = "Capital Good Sector Inverse HHI",
              subtit = paste( "MC runs =", nSize ) )
  
  
  # ------ Intermediate Sector Inverse HHI------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( Adata[[ k ]]$HHI_I[ TmaskPlot ]) 
    # minimum and maximum MC runs
    min[[ k ]] <- list( mdata[[ k ]]$HHI_I[ TmaskPlot ])
    max[[ k ]] <- list( Mdata[[ k ]]$HHI_I[ TmaskPlot ])
    # MC confidence interval
    lo[[ k ]] <- list( Adata[[ k ]]$HHI_I[ TmaskPlot ] -
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$HHI_I[ TmaskPlot ] / sqrt( nSize ))
    hi[[ k ]] <- list( Adata[[ k ]]$HHI_I[ TmaskPlot ] +
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$HHI_I[ TmaskPlot ] / sqrt( nSize ))
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Index",
              tit = "Intermediate Good Sector Inverse HHI",
              subtit = paste( "MC runs =", nSize ) )
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  

}
