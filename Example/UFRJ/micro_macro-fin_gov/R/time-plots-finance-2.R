#******************************************************************
#
# ---------------- Finance time plots --------------------
#
#******************************************************************

# remove warnings for support functions
# !diagnostics suppress = log0, plot_lists, hpfilter, colSds, colMins, colMaxs


time_plots <- function( mcData, Adata, mdata, Mdata, Sdata, nExp, nSize, nTsteps,
                        TmaskPlot, CI, legends, colors, lTypes, transMk, smoothing,
                        firmTypes ) {
  
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
  
}