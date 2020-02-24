#******************************************************************
#
# ---------------- Aggregates time plots --------------------
#
#******************************************************************

# remove warnings for support functions
# !diagnostics suppress = log0, plot_lists, hpfilter, colSds, colMins, colMaxs


time_plots <- function( mcData, Adata, mdata, Mdata, Sdata, nExp, nSize, nTsteps,
                        TmaskPlot, CI, legends, colors, lTypes, transMk, smoothing,
                        firmTypes ) {

  # ------ GDP, consumption and investment cases comparison charts ------

  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( log0( Adata[[ k ]]$Real_GDP[ TmaskPlot ] ),
                       log0( Adata[[ k ]]$I_r[ TmaskPlot ] ),
                       log0( Adata[[ k ]]$C_r[ TmaskPlot ] ) )
    # minimum and maximum MC runs
    min[[ k ]] <- list( log0( mdata[[ k ]]$Real_GDP[ TmaskPlot ] ),
                      log0( mdata[[ k ]]$I_r[ TmaskPlot ] ),
                      log0( mdata[[ k ]]$C_r[ TmaskPlot ] ) )
    max[[ k ]] <- list( log0( Mdata[[ k ]]$Real_GDP[ TmaskPlot ] ),
                      log0( Mdata[[ k ]]$I_r[ TmaskPlot ] ),
                      log0( Mdata[[ k ]]$C_r[ TmaskPlot ] ) )
    # MC confidence interval
    lo[[ k ]] <- list( log0( Adata[[ k ]]$Real_GDP[ TmaskPlot ] -
                             qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$Real_GDP[ TmaskPlot ] / sqrt( nSize ) ),
                     log0( Adata[[ k ]]$I_r[ TmaskPlot ] -
                             qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$I_r[ TmaskPlot ] / sqrt( nSize ) ),
                     log0( Adata[[ k ]]$C_r[ TmaskPlot ] -
                             qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$C_r[ TmaskPlot ]  / sqrt( nSize ) ) )
    hi[[ k ]] <- list( log0( Adata[[ k ]]$Real_GDP[ TmaskPlot ] +
                             qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$Real_GDP[ TmaskPlot ] / sqrt( nSize ) ),
                     log0( Adata[[ k ]]$I_r[ TmaskPlot ] +
                             qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$I_r[ TmaskPlot ] / sqrt( nSize ) ),
                     log0( Adata[[ k ]]$C_r[ TmaskPlot ] +
                             qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$C_r[ TmaskPlot ]  / sqrt( nSize ) ) )
  }

  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Logs",
              tit = "GDP, Investment and Consumption",
              subtit = paste( "MC runs =", nSize ),
              leg2 = c( "GDP", "Investment", "Consumption" ) )
  
  # ------ GDP, Exports and Imports cases comparison charts ------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( log0( Adata[[ k ]]$Real_GDP[ TmaskPlot ] ),
                         log0( Adata[[ k ]]$M_r[ TmaskPlot ] ),
                         log0( Adata[[ k ]]$X_r[ TmaskPlot ] ) )
    # minimum and maximum MC runs
    min[[ k ]] <- list( log0( mdata[[ k ]]$Real_GDP[ TmaskPlot ] ),
                        log0( mdata[[ k ]]$M_r[ TmaskPlot ] ),
                        log0( mdata[[ k ]]$X_r[ TmaskPlot ] ) )
    max[[ k ]] <- list( log0( Mdata[[ k ]]$Real_GDP[ TmaskPlot ] ),
                        log0( Mdata[[ k ]]$M_r[ TmaskPlot ] ),
                        log0( Mdata[[ k ]]$X_r[ TmaskPlot ] ) )
    # MC confidence interval
    lo[[ k ]] <- list( log0( Adata[[ k ]]$Real_GDP[ TmaskPlot ] -
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$Real_GDP[ TmaskPlot ] / sqrt( nSize ) ),
                       log0( Adata[[ k ]]$M_r[ TmaskPlot ] -
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$M_r[ TmaskPlot ] / sqrt( nSize ) ),
                       log0( Adata[[ k ]]$X_r[ TmaskPlot ] -
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$X_r[ TmaskPlot ]  / sqrt( nSize ) ) )
    hi[[ k ]] <- list( log0( Adata[[ k ]]$Real_GDP[ TmaskPlot ] +
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$Real_GDP[ TmaskPlot ] / sqrt( nSize ) ),
                       log0( Adata[[ k ]]$M_r[ TmaskPlot ] +
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$M_r[ TmaskPlot ] / sqrt( nSize ) ),
                       log0( Adata[[ k ]]$X_r[ TmaskPlot ] +
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$X_r[ TmaskPlot ]  / sqrt( nSize ) ) )
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Logs",
              tit = "GDP, Imports and Exports",
              subtit = paste( "MC runs =", nSize ),
              leg2 = c( "GDP", "Imports", "Exports" ) )
  
  # ------ GDP, Government and Net Exports cases comparison charts ------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( log0( Adata[[ k ]]$Real_GDP[ TmaskPlot ] ),
                         log0( Adata[[ k ]]$G_r[ TmaskPlot ] ),
                         log0( Adata[[ k ]]$NX_r[ TmaskPlot ] ) )
    # minimum and maximum MC runs
    min[[ k ]] <- list( log0( mdata[[ k ]]$Real_GDP[ TmaskPlot ] ),
                        log0( mdata[[ k ]]$G_r[ TmaskPlot ] ),
                        log0( mdata[[ k ]]$NX_r[ TmaskPlot ] ) )
    max[[ k ]] <- list( log0( Mdata[[ k ]]$Real_GDP[ TmaskPlot ] ),
                        log0( Mdata[[ k ]]$G_r[ TmaskPlot ] ),
                        log0( Mdata[[ k ]]$NX_r[ TmaskPlot ] ) )
    # MC confidence interval
    lo[[ k ]] <- list( log0( Adata[[ k ]]$Real_GDP[ TmaskPlot ] -
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$Real_GDP[ TmaskPlot ] / sqrt( nSize ) ),
                       log0( Adata[[ k ]]$G_r[ TmaskPlot ] -
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$G_r[ TmaskPlot ] / sqrt( nSize ) ),
                       log0( Adata[[ k ]]$NX_r[ TmaskPlot ] -
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$NX_r[ TmaskPlot ]  / sqrt( nSize ) ) )
    hi[[ k ]] <- list( log0( Adata[[ k ]]$Real_GDP[ TmaskPlot ] +
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$Real_GDP[ TmaskPlot ] / sqrt( nSize ) ),
                       log0( Adata[[ k ]]$G_r[ TmaskPlot ] +
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$G_r[ TmaskPlot ] / sqrt( nSize ) ),
                       log0( Adata[[ k ]]$NX_r[ TmaskPlot ] +
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$NX_r[ TmaskPlot ]  / sqrt( nSize ) ) )
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Logs",
              tit = "GDP, Government Expenses and Net Exports",
              subtit = paste( "MC runs =", nSize ),
              leg2 = c( "GDP", "Government", "Net.Exports" ) )

  # ------ Unemployment ------

  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( Adata[[ k ]]$U[ TmaskPlot ] )
    # minimum and maximum MC runs
    min[[ k ]] <- list( mdata[[ k ]]$U[ TmaskPlot ])
    max[[ k ]] <- list( Mdata[[ k ]]$U[ TmaskPlot ])
    # MC confidence interval
    lo[[ k ]] <- list( Adata[[ k ]]$U[ TmaskPlot ] -
                       qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$U[ TmaskPlot ] / sqrt( nSize ))
    hi[[ k ]] <- list( Adata[[ k ]]$U[ TmaskPlot ] +
                       qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$U[ TmaskPlot ] / sqrt( nSize ))
  }

  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Rate",
              tit = "Unemployment",
              subtit = paste( "MC runs =", nSize ),
              leg2 = c( "Unemployment") )
  
  # ------ Inflation ------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( Adata[[ k ]]$P_G[ TmaskPlot ] )
    # minimum and maximum MC runs
    min[[ k ]] <- list( mdata[[ k ]]$P_G[ TmaskPlot ])
    max[[ k ]] <- list( Mdata[[ k ]]$P_G[ TmaskPlot ])
    # MC confidence interval
    lo[[ k ]] <- list( Adata[[ k ]]$P_G[ TmaskPlot ] -
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$P_G[ TmaskPlot ] / sqrt( nSize ))
    hi[[ k ]] <- list( Adata[[ k ]]$P_G[ TmaskPlot ] +
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$P_G[ TmaskPlot ] / sqrt( nSize ))
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Rate",
              tit = "Inflation",
              subtit = paste( "MC runs =", nSize ) )


  # ------ Wages and Profits ------

  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    
    exps[[ k ]] <- list( log0( Adata[[ k ]]$WAGE[ TmaskPlot ] ), 
                         log0( Adata[[ k ]]$PROFITS[ TmaskPlot ] ))
    
    # minimum and maximum MC runs
    
    min[[ k ]] <- list( log0( mdata[[ k ]]$WAGE[ TmaskPlot ] ),
                        log0( mdata[[ k ]]$PROFITS[ TmaskPlot ] ) )
    
    max[[ k ]] <- list( log0( Mdata[[ k ]]$WAGE[ TmaskPlot ] ),
                        log0( Mdata[[ k ]]$PROFITS[ TmaskPlot ] ))
    
    # MC confidence interval
    
    lo[[ k ]] <- list( log0( Adata[[ k ]]$WAGE[ TmaskPlot ] -
                           qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$WAGE[ TmaskPlot ] / sqrt( nSize ) ),
                       log0( Adata[[ k ]]$PROFITS[ TmaskPlot ] -
                           qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$PROFITS[ TmaskPlot ] / sqrt( nSize ) ))
    
    hi[[ k ]] <- list( log0( Adata[[ k ]]$WAGE[ TmaskPlot ] +
                             qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$WAGE[ TmaskPlot ] / sqrt( nSize ) ),
                       log0( Adata[[ k ]]$PROFITS[ TmaskPlot ] +
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$PROFITS[ TmaskPlot ] / sqrt( nSize ) ))
  }

  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Logs",
              tit = "Wages and Profits",
              subtit = paste( "MC runs =", nSize ),
              leg2 = c( "Wages", "Profits" ) )
  
  
  # ------ Wages and Profits Shares------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    
    exps[[ k ]] <- list( Adata[[ k ]]$Wage_Share[ TmaskPlot ], 
                         Adata[[ k ]]$Profit_Share[ TmaskPlot ])
    
    # minimum and maximum MC runs
    
    min[[ k ]] <- list( mdata[[ k ]]$Wage_Share[ TmaskPlot ],
                        mdata[[ k ]]$Profit_Share[ TmaskPlot ] )
    
    max[[ k ]] <- list( Mdata[[ k ]]$Wage_Share[ TmaskPlot ],
                        Mdata[[ k ]]$Profit_Share[ TmaskPlot ])
    
    # MC confidence interval
    
    lo[[ k ]] <- list( Adata[[ k ]]$Wage_Share[ TmaskPlot ] -
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$Wage_Share[ TmaskPlot ] / sqrt( nSize ),
                       Adata[[ k ]]$Profit_Share[ TmaskPlot ] -
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$Profit_Share[ TmaskPlot ] / sqrt( nSize ))
    
    hi[[ k ]] <- list( Adata[[ k ]]$Wage_Share[ TmaskPlot ] +
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$Wage_Share[ TmaskPlot ] / sqrt( nSize ),
                       Adata[[ k ]]$Profit_Share[ TmaskPlot ] +
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$Profit_Share[ TmaskPlot ] / sqrt( nSize ))
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Shares",
              tit = "Wages and Profits",
              subtit = paste( "MC runs =", nSize ),
              leg2 = c( "Wages", "Profits" ) )

  
  # ------ Productivity  ------

  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( log0( Adata[[ k ]]$PROD[ TmaskPlot ] ))
    # minimum and maximum MC runs
    min[[ k ]] <- list( log0( mdata[[ k ]]$PROD[ TmaskPlot ] ))
    max[[ k ]] <- list( log0( Mdata[[ k ]]$PROD[ TmaskPlot ] ))
    # MC confidence interval
    lo[[ k ]] <- list( log0( Adata[[ k ]]$PROD[ TmaskPlot ] -
                             qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$PROD[ TmaskPlot ] / sqrt( nSize ) ))
    hi[[ k ]] <- list( log0( Adata[[ k ]]$PROD[ TmaskPlot ] +
                             qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$PROD[ TmaskPlot ] / sqrt( nSize ) ))
  }

  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Log",
              tit = "Average Productivity",
              subtit = paste( "MC runs =", nSize ) )
  
  
  # ------ Markup  ------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( Adata[[ k ]]$MK[ TmaskPlot ] )
    # minimum and maximum MC runs
    min[[ k ]] <- list( mdata[[ k ]]$MK[ TmaskPlot ] )
    max[[ k ]] <- list( Mdata[[ k ]]$MK[ TmaskPlot ] )
    # MC confidence interval
    lo[[ k ]] <- list( Adata[[ k ]]$MK[ TmaskPlot ] -
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$MK[ TmaskPlot ] / sqrt( nSize ) )
    hi[[ k ]] <- list( Adata[[ k ]]$MK[ TmaskPlot ] +
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$MK[ TmaskPlot ] / sqrt( nSize ) )
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Value",
              tit = "Average Mark-up",
              subtit = paste( "MC runs =", nSize ) )
  
  # ------ Inventories  ------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( log0( Adata[[ k ]]$INVE_r[ TmaskPlot ] ))
    # minimum and maximum MC runs
    min[[ k ]] <- list( log0( mdata[[ k ]]$INVE_r[ TmaskPlot ] ))
    max[[ k ]] <- list( log0( Mdata[[ k ]]$INVE_r[ TmaskPlot ] ))
    # MC confidence interval
    lo[[ k ]] <- list( log0( Adata[[ k ]]$INVE_r[ TmaskPlot ] -
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$INVE_r[ TmaskPlot ] / sqrt( nSize ) ))
    hi[[ k ]] <- list( log0( Adata[[ k ]]$INVE_r[ TmaskPlot ] +
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$INVE_r[ TmaskPlot ] / sqrt( nSize ) ))
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Log",
              tit = "Aggregate Inventories",
              subtit = paste( "MC runs =", nSize ) )
  
  
  # ------ Capital Stock  ------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( log0( Adata[[ k ]]$K_r[ TmaskPlot ] ))
    # minimum and maximum MC runs
    min[[ k ]] <- list( log0( mdata[[ k ]]$K_r[ TmaskPlot ] ))
    max[[ k ]] <- list( log0( Mdata[[ k ]]$K_r[ TmaskPlot ] ))
    # MC confidence interval
    lo[[ k ]] <- list( log0( Adata[[ k ]]$K_r[ TmaskPlot ] -
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$K_r[ TmaskPlot ] / sqrt( nSize ) ))
    hi[[ k ]] <- list( log0( Adata[[ k ]]$K_r[ TmaskPlot ] +
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$K_r[ TmaskPlot ] / sqrt( nSize ) ))
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Log",
              tit = "Aggregate Capital Stock",
              subtit = paste( "MC runs =", nSize ) )
  

}
