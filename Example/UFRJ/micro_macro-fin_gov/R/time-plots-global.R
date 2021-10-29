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
  
  # ------ Shares of GDP------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    
    exps[[ k ]] <- list( Adata[[ k ]]$CGDP[ TmaskPlot ], 
                         Adata[[ k ]]$IGDP[ TmaskPlot ])
    
    # minimum and maximum MC runs
    
    min[[ k ]] <- list( mdata[[ k ]]$CGDP[ TmaskPlot ],
                        mdata[[ k ]]$IGDP[ TmaskPlot ] )
    
    max[[ k ]] <- list( Mdata[[ k ]]$CGDP[ TmaskPlot ],
                        Mdata[[ k ]]$IGDP[ TmaskPlot ])
    
    # MC confidence interval
    
    lo[[ k ]] <- list( Adata[[ k ]]$CGDP[ TmaskPlot ] -
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$CGDP[ TmaskPlot ] / sqrt( nSize ),
                       Adata[[ k ]]$IGDP[ TmaskPlot ] -
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$IGDP[ TmaskPlot ] / sqrt( nSize ))
    
    hi[[ k ]] <- list( Adata[[ k ]]$CGDP[ TmaskPlot ] +
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$CGDP[ TmaskPlot ] / sqrt( nSize ),
                       Adata[[ k ]]$IGDP[ TmaskPlot ] +
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$IGDP[ TmaskPlot ] / sqrt( nSize ))
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Shares",
              tit = "Consumption and Investment",
              subtit = paste( "MC runs =", nSize ),
              leg2 = c( "Consumption", "Investment" ) )

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
  
  
  # ------ Capital Labor Ratio  ------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( log0( Adata[[ k ]]$KL[ TmaskPlot ] ))
    # minimum and maximum MC runs
    min[[ k ]] <- list( log0( mdata[[ k ]]$KL[ TmaskPlot ] ))
    max[[ k ]] <- list( log0( Mdata[[ k ]]$KL[ TmaskPlot ] ))
    # MC confidence interval
    lo[[ k ]] <- list( log0( Adata[[ k ]]$KL[ TmaskPlot ] -
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$KL[ TmaskPlot ] / sqrt( nSize ) ))
    hi[[ k ]] <- list( log0( Adata[[ k ]]$KL[ TmaskPlot ] +
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$KL[ TmaskPlot ] / sqrt( nSize ) ))
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Log",
              tit = "Capital-Labor Ratio",
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
  
  # ------ Profit Rate  ------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( Adata[[ k ]]$PR[ TmaskPlot ] )
    # minimum and maximum MC runs
    min[[ k ]] <- list( mdata[[ k ]]$PR[ TmaskPlot ] )
    max[[ k ]] <- list( Mdata[[ k ]]$PR[ TmaskPlot ] )
    # MC confidence interval
    lo[[ k ]] <- list( Adata[[ k ]]$PR[ TmaskPlot ] -
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$PR[ TmaskPlot ] / sqrt( nSize ) )
    hi[[ k ]] <- list( Adata[[ k ]]$PR[ TmaskPlot ] +
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$PR[ TmaskPlot ] / sqrt( nSize ) )
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Rate",
              tit = "Profit Rate",
              subtit = paste( "MC runs =", nSize ) )
  
  # ------ Capacity Utilization  ------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( Adata[[ k ]]$PCU[ TmaskPlot ] )
    # minimum and maximum MC runs
    min[[ k ]] <- list( mdata[[ k ]]$PCU[ TmaskPlot ] )
    max[[ k ]] <- list( Mdata[[ k ]]$PCU[ TmaskPlot ] )
    # MC confidence interval
    lo[[ k ]] <- list( Adata[[ k ]]$PCU[ TmaskPlot ] -
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$PCU[ TmaskPlot ] / sqrt( nSize ) )
    hi[[ k ]] <- list( Adata[[ k ]]$PCU[ TmaskPlot ] +
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$PCU[ TmaskPlot ] / sqrt( nSize ) )
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Rate",
              tit = "Capacity Utilization",
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
    exps[[ k ]] <- list( Adata[[ k ]]$KGDP[ TmaskPlot ] )
    # minimum and maximum MC runs
    min[[ k ]] <- list( mdata[[ k ]]$KGDP[ TmaskPlot ] )
    max[[ k ]] <- list( Mdata[[ k ]]$KGDP[ TmaskPlot ] )
    # MC confidence interval
    lo[[ k ]] <- list( Adata[[ k ]]$KGDP[ TmaskPlot ] -
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$KGDP[ TmaskPlot ] / sqrt( nSize ) )
    hi[[ k ]] <- list( Adata[[ k ]]$KGDP[ TmaskPlot ] +
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$KGDP[ TmaskPlot ] / sqrt( nSize ) )
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Rate",
              tit = "Capital Stock to GDP",
              subtit = paste( "MC runs =", nSize ) )
  
  
  # ------ Capital Stock ------
  
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
  
  # ------ Leverage ------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( Adata[[ k ]]$FS_LEV[ TmaskPlot ] )
    # minimum and maximum MC runs
    min[[ k ]] <- list( mdata[[ k ]]$FS_LEV[ TmaskPlot ]  )
    max[[ k ]] <- list( Mdata[[ k ]]$FS_LEV[ TmaskPlot ] )
    # MC confidence interval
    lo[[ k ]] <- list( Adata[[ k ]]$FS_LEV[ TmaskPlot ] -
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$FS_LEV[ TmaskPlot ] / sqrt( nSize  ) )
    hi[[ k ]] <- list( Adata[[ k ]]$FS_LEV[ TmaskPlot ] +
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$FS_LEV[ TmaskPlot ] / sqrt( nSize  ) )
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Value",
              tit = "Financial Sector Leverage",
              subtit = paste( "MC runs =", nSize ) )
  
  # ------ Financial Sector HHI ------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( Adata[[ k ]]$FS_HHI[ TmaskPlot ] )
    # minimum and maximum MC runs
    min[[ k ]] <- list( mdata[[ k ]]$FS_HHI[ TmaskPlot ]  )
    max[[ k ]] <- list( Mdata[[ k ]]$FS_HHI[ TmaskPlot ] )
    # MC confidence interval
    lo[[ k ]] <- list( Adata[[ k ]]$FS_HHI[ TmaskPlot ] -
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$FS_HHI[ TmaskPlot ] / sqrt( nSize  ) )
    hi[[ k ]] <- list( Adata[[ k ]]$FS_HHI[ TmaskPlot ] +
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$FS_HHI[ TmaskPlot ] / sqrt( nSize  ) )
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Value",
              tit = "Financial Sector Normalized HHI",
              subtit = paste( "MC runs =", nSize ) )
  
  # ------ Financial Sector Short-Term Rate ------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( Adata[[ k ]]$FS_STR[ TmaskPlot ] )
    # minimum and maximum MC runs
    min[[ k ]] <- list( mdata[[ k ]]$FS_STR[ TmaskPlot ]  )
    max[[ k ]] <- list( Mdata[[ k ]]$FS_STR[ TmaskPlot ] )
    # MC confidence interval
    lo[[ k ]] <- list( Adata[[ k ]]$FS_STR[ TmaskPlot ] -
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$FS_STR[ TmaskPlot ] / sqrt( nSize  ) )
    hi[[ k ]] <- list( Adata[[ k ]]$FS_STR[ TmaskPlot ] +
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$FS_STR[ TmaskPlot ] / sqrt( nSize  ) )
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Rate",
              tit = "Financial Sector Short Term Rate",
              subtit = paste( "MC runs =", nSize ) )
  
  # ------ Financial Sector Demand Met ------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( Adata[[ k ]]$Financial_Sector_Demand_Met[ TmaskPlot ] )
    # minimum and maximum MC runs
    min[[ k ]] <- list( mdata[[ k ]]$Financial_Sector_Demand_Met[ TmaskPlot ]  )
    max[[ k ]] <- list( Mdata[[ k ]]$Financial_Sector_Demand_Met[ TmaskPlot ] )
    # MC confidence interval
    lo[[ k ]] <- list( Adata[[ k ]]$Financial_Sector_Demand_Met[ TmaskPlot ] -
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$Financial_Sector_Demand_Met[ TmaskPlot ] / sqrt( nSize  ) )
    hi[[ k ]] <- list( Adata[[ k ]]$Financial_Sector_Demand_Met[ TmaskPlot ] +
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$Financial_Sector_Demand_Met[ TmaskPlot ] / sqrt( nSize  ) )
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Rate",
              tit = "Financial Sector Demand Met",
              subtit = paste( "MC runs =", nSize ) )
  
  # ------ Financial Sector Profits ------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( log0( Adata[[ k ]]$FS_PR[ TmaskPlot ] ))
    # minimum and maximum MC runs
    min[[ k ]] <- list( log0( mdata[[ k ]]$FS_PR[ TmaskPlot ] ))
    max[[ k ]] <- list( log0( Mdata[[ k ]]$FS_PR[ TmaskPlot ] ))
    # MC confidence interval
    lo[[ k ]] <- list( log0( Adata[[ k ]]$FS_PR[ TmaskPlot ] -
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$FS_PR[ TmaskPlot ] / sqrt( nSize ) ))
    hi[[ k ]] <- list( log0( Adata[[ k ]]$FS_PR[ TmaskPlot ] +
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$FS_PR[ TmaskPlot ] / sqrt( nSize ) ))
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Log",
              tit = "Financial Sector Profits",
              subtit = paste( "MC runs =", nSize ) )
  
  # ------ Financial Sector Defaulted Loans ------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( Adata[[ k ]]$FS_DEF[ TmaskPlot ] )
    # minimum and maximum MC runs
    min[[ k ]] <- list( mdata[[ k ]]$FS_DEF[ TmaskPlot ]  )
    max[[ k ]] <- list( Mdata[[ k ]]$FS_DEF[ TmaskPlot ] )
    # MC confidence interval
    lo[[ k ]] <- list( Adata[[ k ]]$FS_DEF[ TmaskPlot ] -
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$FS_DEF[ TmaskPlot ] / sqrt( nSize  ) )
    hi[[ k ]] <- list( Adata[[ k ]]$FS_DEF[ TmaskPlot ] +
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$FS_DEF[ TmaskPlot ] / sqrt( nSize  ) )
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Value",
              tit = "Financial Sector Defaulted Loans",
              subtit = paste( "MC runs =", nSize ) )
  
  # ------ Financial Sector Default Rate ------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( Adata[[ k ]]$FS_DR[ TmaskPlot ] )
    # minimum and maximum MC runs
    min[[ k ]] <- list( mdata[[ k ]]$FS_DR[ TmaskPlot ]  )
    max[[ k ]] <- list( Mdata[[ k ]]$FS_DR[ TmaskPlot ] )
    # MC confidence interval
    lo[[ k ]] <- list( Adata[[ k ]]$FS_DR[ TmaskPlot ] -
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$FS_DR[ TmaskPlot ] / sqrt( nSize  ) )
    hi[[ k ]] <- list( Adata[[ k ]]$FS_DR[ TmaskPlot ] +
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$FS_DR[ TmaskPlot ] / sqrt( nSize  ) )
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Rate",
              tit = "Financial Sector Default Rate",
              subtit = paste( "MC runs =", nSize ) )
  
  # ------ Fiancial Postures------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    
    exps[[ k ]] <- list( Adata[[ k ]]$PONZI[ TmaskPlot ], 
                         Adata[[ k ]]$SPEC[ TmaskPlot ],
                         Adata[[ k ]]$HEDGE[ TmaskPlot ])
    
    # minimum and maximum MC runs
    
    min[[ k ]] <- list( mdata[[ k ]]$PONZI[ TmaskPlot ],
                        mdata[[ k ]]$SPEC[ TmaskPlot ],
                        mdata[[ k ]]$HEDGE[ TmaskPlot ] )
    
    max[[ k ]] <- list( Mdata[[ k ]]$PONZI[ TmaskPlot ],
                        Mdata[[ k ]]$SPEC[ TmaskPlot ],
                        Mdata[[ k ]]$HEDGE[ TmaskPlot ])
    
    # MC confidence interval
    
    lo[[ k ]] <- list( Adata[[ k ]]$PONZI[ TmaskPlot ] -
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$PONZI[ TmaskPlot ] / sqrt( nSize ),
                       Adata[[ k ]]$SPEC[ TmaskPlot ] -
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$SPEC[ TmaskPlot ] / sqrt( nSize ),
                       Adata[[ k ]]$HEDGE[ TmaskPlot ] -
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$HEDGE[ TmaskPlot ] / sqrt( nSize ))
    
    hi[[ k ]] <- list( Adata[[ k ]]$PONZI[ TmaskPlot ] +
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$PONZI[ TmaskPlot ] / sqrt( nSize ),
                       Adata[[ k ]]$SPEC[ TmaskPlot ] +
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$SPEC[ TmaskPlot ] / sqrt( nSize ),
                       Adata[[ k ]]$HEDGE[ TmaskPlot ] +
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$HEDGE[ TmaskPlot ] / sqrt( nSize ))
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Shares",
              tit = "Financial Positions",
              subtit = paste( "MC runs =", nSize ),
              leg2 = c( "Ponzi", "Speculative", "Hedge" ) )
  
  # ------ Basic Interest Rate ------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( Adata[[ k ]]$IR[ TmaskPlot ] )
    # minimum and maximum MC runs
    min[[ k ]] <- list( mdata[[ k ]]$IR[ TmaskPlot ]  )
    max[[ k ]] <- list( Mdata[[ k ]]$IR[ TmaskPlot ] )
    # MC confidence interval
    lo[[ k ]] <- list( Adata[[ k ]]$IR[ TmaskPlot ] -
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$IR[ TmaskPlot ] / sqrt( nSize  ) )
    hi[[ k ]] <- list( Adata[[ k ]]$IR[ TmaskPlot ] +
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$IR[ TmaskPlot ] / sqrt( nSize  ) )
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Rate",
              tit = "Basic Interest Rate",
              subtit = paste( "MC runs =", nSize ) )
  
  # ------ Deposits Interest Rate ------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( Adata[[ k ]]$IR_DEP[ TmaskPlot ] )
    # minimum and maximum MC runs
    min[[ k ]] <- list( mdata[[ k ]]$IR_DEP[ TmaskPlot ]  )
    max[[ k ]] <- list( Mdata[[ k ]]$IR_DEP[ TmaskPlot ] )
    # MC confidence interval
    lo[[ k ]] <- list( Adata[[ k ]]$IR_DEP[ TmaskPlot ] -
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$IR_DEP[ TmaskPlot ] / sqrt( nSize  ) )
    hi[[ k ]] <- list( Adata[[ k ]]$IR_DEP[ TmaskPlot ] +
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$IR_DEP / sqrt( nSize  ) )
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Rate",
              tit = "Deposits Interest Rate",
              subtit = paste( "MC runs =", nSize ) )
  
  # ------ Short Term Interest Rate ------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( Adata[[ k ]]$IR_ST[ TmaskPlot ] )
    # minimum and maximum MC runs
    min[[ k ]] <- list( mdata[[ k ]]$IR_ST[ TmaskPlot ]  )
    max[[ k ]] <- list( Mdata[[ k ]]$IR_ST[ TmaskPlot ] )
    # MC confidence interval
    lo[[ k ]] <- list( Adata[[ k ]]$IR_ST[ TmaskPlot ] -
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$IR_ST[ TmaskPlot ] / sqrt( nSize  ) )
    hi[[ k ]] <- list( Adata[[ k ]]$IR_ST[ TmaskPlot ] +
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$IR_ST / sqrt( nSize  ) )
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Rate",
              tit = "Short Term Interest Rate",
              subtit = paste( "MC runs =", nSize ) )
  
  # ------ Long Term Interest Rate ------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( Adata[[ k ]]$IR_LT[ TmaskPlot ] )
    # minimum and maximum MC runs
    min[[ k ]] <- list( mdata[[ k ]]$IR_LT[ TmaskPlot ]  )
    max[[ k ]] <- list( Mdata[[ k ]]$IR_LT[ TmaskPlot ] )
    # MC confidence interval
    lo[[ k ]] <- list( Adata[[ k ]]$IR_LT[ TmaskPlot ] -
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$IR_LT[ TmaskPlot ] / sqrt( nSize  ) )
    hi[[ k ]] <- list( Adata[[ k ]]$IR_LT[ TmaskPlot ] +
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$IR_LT / sqrt( nSize  ) )
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Rate",
              tit = "Long Term Interest Rate",
              subtit = paste( "MC runs =", nSize ) )
  
  # ------ Bankrupt Rate ------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( Adata[[ k ]]$BKR_RT[ TmaskPlot ] )
    # minimum and maximum MC runs
    min[[ k ]] <- list( mdata[[ k ]]$BKR_RT[ TmaskPlot ]  )
    max[[ k ]] <- list( Mdata[[ k ]]$BKR_RT[ TmaskPlot ] )
    # MC confidence interval
    lo[[ k ]] <- list( Adata[[ k ]]$BKR_RT[ TmaskPlot ] -
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$BKR_RT[ TmaskPlot ] / sqrt( nSize  ) )
    hi[[ k ]] <- list( Adata[[ k ]]$BKR_RT[ TmaskPlot ] +
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$BKR_RT / sqrt( nSize  ) )
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Rate",
              tit = "Bankrupt Rate",
              subtit = paste( "MC runs =", nSize ) )
  
  # ------ Banking Crisis ------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( log0( Adata[[ k ]]$Financial_Sector_Rescue[ TmaskPlot ] ))
    # minimum and maximum MC runs
    min[[ k ]] <- list( log0( mdata[[ k ]]$Financial_Sector_Rescue[ TmaskPlot ] ))
    max[[ k ]] <- list( log0( Mdata[[ k ]]$Financial_Sector_Rescue[ TmaskPlot ] ))
    # MC confidence interval
    lo[[ k ]] <- list( log0( Adata[[ k ]]$Financial_Sector_Rescue[ TmaskPlot ] -
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$Financial_Sector_Rescue[ TmaskPlot ] / sqrt( nSize ) ))
    hi[[ k ]] <- list( log0( Adata[[ k ]]$Financial_Sector_Rescue[ TmaskPlot ] +
                               qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$Financial_Sector_Rescue[ TmaskPlot ] / sqrt( nSize ) ))
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Log",
              tit = "Banking Crisis",
              subtit = paste( "MC runs =", nSize ) )
  

}
