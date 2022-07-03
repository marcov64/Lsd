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
    exps[[ k ]] <- list( Adata[[ k ]]$Financial_Sector_Profits[ TmaskPlot ] )
    # minimum and maximum MC runs
    min[[ k ]] <- list( mdata[[ k ]]$Financial_Sector_Profits[ TmaskPlot ]  )
    max[[ k ]] <- list( Mdata[[ k ]]$Financial_Sector_Profits[ TmaskPlot ] )
    # MC confidence interval
    lo[[ k ]] <- list( Adata[[ k ]]$Financial_Sector_Profits[ TmaskPlot ] -
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$Financial_Sector_Profits[ TmaskPlot ] / sqrt( nSize  ) )
    hi[[ k ]] <- list( Adata[[ k ]]$Financial_Sector_Profits[ TmaskPlot ] +
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$Financial_Sector_Profits[ TmaskPlot ] / sqrt( nSize  ) )
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Value",
              tit = "Financial Sector Profits",
              subtit = paste( "MC runs =", nSize ) )
  
  # ------ Interest Rate ------
  
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( Adata[[ k ]]$Avg_Interest_Rate_Long_Term[ TmaskPlot ] )
    # minimum and maximum MC runs
    min[[ k ]] <- list( mdata[[ k ]]$Avg_Interest_Rate_Long_Term[ TmaskPlot ]  )
    max[[ k ]] <- list( Mdata[[ k ]]$Avg_Interest_Rate_Long_Term[ TmaskPlot ] )
    # MC confidence interval
    lo[[ k ]] <- list( Adata[[ k ]]$Avg_Interest_Rate_Long_Term[ TmaskPlot ] -
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$Avg_Interest_Rate_Long_Term[ TmaskPlot ] / sqrt( nSize  ) )
    hi[[ k ]] <- list( Adata[[ k ]]$Avg_Interest_Rate_Long_Term[ TmaskPlot ] +
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$Avg_Interest_Rate_Long_Term[ TmaskPlot ] / sqrt( nSize  ) )
  }
  
  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Rate",
              tit = "Interest Rate",
              subtit = paste( "MC runs =", nSize ) )
  
}