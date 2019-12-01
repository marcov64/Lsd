#******************************************************************
#
# ---------------- K+S aggregates time plots --------------------
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
    exps[[ k ]] <- list( log0( Adata[[ k ]]$GDP[ TmaskPlot ] ),
                       log0( Adata[[ k ]]$I[ TmaskPlot ] ),
                       log0( Adata[[ k ]]$C[ TmaskPlot ] ) )
    # minimum and maximum MC runs
    min[[ k ]] <- list( log0( mdata[[ k ]]$GDP[ TmaskPlot ] ),
                      log0( mdata[[ k ]]$I[ TmaskPlot ] ),
                      log0( mdata[[ k ]]$C[ TmaskPlot ] ) )
    max[[ k ]] <- list( log0( Mdata[[ k ]]$GDP[ TmaskPlot ] ),
                      log0( Mdata[[ k ]]$I[ TmaskPlot ] ),
                      log0( Mdata[[ k ]]$C[ TmaskPlot ] ) )
    # MC confidence interval
    lo[[ k ]] <- list( log0( Adata[[ k ]]$GDP[ TmaskPlot ] -
                             qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$GDP[ TmaskPlot ] / sqrt( nSize ) ),
                     log0( Adata[[ k ]]$I[ TmaskPlot ] -
                             qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$I[ TmaskPlot ] / sqrt( nSize ) ),
                     log0( Adata[[ k ]]$C[ TmaskPlot ] -
                             qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$C[ TmaskPlot ]  / sqrt( nSize ) ) )
    hi[[ k ]] <- list( log0( Adata[[ k ]]$GDP[ TmaskPlot ] +
                             qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$GDP[ TmaskPlot ] / sqrt( nSize ) ),
                     log0( Adata[[ k ]]$I[ TmaskPlot ] +
                             qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$I[ TmaskPlot ] / sqrt( nSize ) ),
                     log0( Adata[[ k ]]$C[ TmaskPlot ] +
                             qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$C[ TmaskPlot ]  / sqrt( nSize ) ) )
  }

  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Logs",
              tit = "GDP, investment and consumption",
              subtit = paste( "MC runs =", nSize ),
              leg2 = c( "GDP", "Investment", "Consumption" ) )


  # ------ GDP Graphs ------

  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ) {

    # create the GDP @ 100% utilization series
    Sgdp100 <- sqrt( Sdata[[ k ]]$GDP ^ 2 + Sdata[[ k ]]$Q2u ^ 2 ) # very crude approximation
    Agdp100 <- vector( "numeric", length = nTsteps )
    for( i in 1 : nTsteps ) {
      if( is.finite( Adata[[ k ]]$Q2u[ i ] ) && Adata[[ k ]]$Q2u[ i ] != 0 )
        Agdp100[ i ] <- Adata[[ k ]]$GDP[ i ] / Adata[[ k ]]$Q2u[ i ]
      else
        Agdp100[ i ] <- NA
    }
    # MC averages
    exps[[ k ]] <- list( log0( Adata[[ k ]]$GDP[ TmaskPlot ] ),
                       log0( Agdp100[ TmaskPlot ] ) )
    # minimum and maximum MC runs
    min[[ k ]] <- list( log0( mdata[[ k ]]$GDP[ TmaskPlot ] ),
                      log0( mdata[[ k ]]$GDP[ TmaskPlot ] ) )
    max[[ k ]] <- list( log0( Mdata[[ k ]]$GDP[ TmaskPlot ] ),
                      log0( Mdata[[ k ]]$GDP[ TmaskPlot ] /
                        min( Adata[[ k ]]$Q2u[ TmaskPlot ], na.rm = TRUE ) ) )
    # MC confidence interval
    lo[[ k ]] <- list( log0( Adata[[ k ]]$GDP[ TmaskPlot ] -
                             qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$GDP[ TmaskPlot ] / sqrt( nSize ) ),
                     log0( Agdp100[ TmaskPlot ] -
                             qnorm(1 - (1 - CI ) / 2) * Sgdp100[ TmaskPlot ] / sqrt( nSize ) ) )
    hi[[ k ]] <- list( log0( Adata[[ k ]]$GDP[ TmaskPlot ] +
                             qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$GDP[ TmaskPlot ] / sqrt( nSize ) ),
                     log0( Agdp100[ TmaskPlot ] +
                             qnorm(1 - (1 - CI ) / 2) * Sgdp100[ TmaskPlot ] / sqrt( nSize ) ) )
  }

  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Log GDP",
              tit = "GDP", subtit = paste( "MC runs =", nSize ),
              leg2 = c( "Effective GDP", "GDP @ 100% utilization" ) )


  # ------ Tax & government expenditures in GDP terms ------

  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( Adata[[ k ]]$Tax[ TmaskPlot ] / Adata[[ k ]]$GDP[ TmaskPlot ],
                       Adata[[ k ]]$G[ TmaskPlot ] / Adata[[ k ]]$GDP[ TmaskPlot ],
                       Adata[[ k ]]$Gtrain[ TmaskPlot ] / Adata[[ k ]]$GDP[ TmaskPlot ] )
    # minimum and maximum MC runs
    min[[ k ]] <- list( mdata[[ k ]]$Tax[ TmaskPlot ] / Adata[[ k ]]$GDP[ TmaskPlot ],
                      mdata[[ k ]]$G[ TmaskPlot ] / Adata[[ k ]]$GDP[ TmaskPlot ],
                      mdata[[ k ]]$Gtrain[ TmaskPlot ] / Adata[[ k ]]$GDP[ TmaskPlot ] )
    max[[ k ]] <- list( Mdata[[ k ]]$Tax[ TmaskPlot ] / Adata[[ k ]]$GDP[ TmaskPlot ],
                      Mdata[[ k ]]$G[ TmaskPlot ] / Adata[[ k ]]$GDP[ TmaskPlot ],
                      Mdata[[ k ]]$Gtrain[ TmaskPlot ] / Adata[[ k ]]$GDP[ TmaskPlot ] )
    # MC confidence interval
    lo[[ k ]] <- list( Adata[[ k ]]$Tax[ TmaskPlot ] / Adata[[ k ]]$GDP[ TmaskPlot ] -
                       qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$Tax[ TmaskPlot ] /
                       Adata[[ k ]]$GDP[ TmaskPlot ] / sqrt( nSize ),
                     Adata[[ k ]]$G[ TmaskPlot ] / Adata[[ k ]]$GDP[ TmaskPlot ] -
                       qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$G[ TmaskPlot ] /
                       Adata[[ k ]]$GDP[ TmaskPlot ] / sqrt( nSize ),
                     Adata[[ k ]]$Gtrain[ TmaskPlot ] / Adata[[ k ]]$GDP[ TmaskPlot ] -
                       qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$Gtrain[ TmaskPlot ] /
                       Adata[[ k ]]$GDP[ TmaskPlot ] / sqrt( nSize ) )
    hi[[ k ]] <- list( Adata[[ k ]]$Tax[ TmaskPlot ] / Adata[[ k ]]$GDP[ TmaskPlot ] +
                       qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$Tax[ TmaskPlot ] /
                       Adata[[ k ]]$GDP[ TmaskPlot ] / sqrt( nSize ),
                     Adata[[ k ]]$G[ TmaskPlot ] / Adata[[ k ]]$GDP[ TmaskPlot ] +
                       qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$G[ TmaskPlot ] /
                       Adata[[ k ]]$GDP[ TmaskPlot ] / sqrt( nSize ),
                     Adata[[ k ]]$Gtrain[ TmaskPlot ] / Adata[[ k ]]$GDP[ TmaskPlot ] +
                       qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$Gtrain[ TmaskPlot ] /
                       Adata[[ k ]]$GDP[ TmaskPlot ] / sqrt( nSize ) )
  }

  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Log government tax income and expenditure over GDP",
              tit = "Government income and expenditure on GDP", subtit = paste( "MC runs =", nSize ),
              leg2 = c( "Tax", "Total expenditure", "Training expenditure" ) )


  # ------ Government deficit in GDP terms------

  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( Adata[[ k ]]$Def[ TmaskPlot ] / Adata[[ k ]]$GDP[ TmaskPlot ] )
    # minimum and maximum MC runs
    min[[ k ]] <- list( mdata[[ k ]]$Def[ TmaskPlot ] / Adata[[ k ]]$GDP[ TmaskPlot ] )
    max[[ k ]] <- list( Mdata[[ k ]]$Def[ TmaskPlot ] / Adata[[ k ]]$GDP[ TmaskPlot ] )
    # MC confidence interval
    lo[[ k ]] <- list( Adata[[ k ]]$Def[ TmaskPlot ] / Adata[[ k ]]$GDP[ TmaskPlot ] -
                       qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$Def[ TmaskPlot ] /
                       Adata[[ k ]]$GDP[ TmaskPlot ] / sqrt( nSize ) )
    hi[[ k ]] <- list( Adata[[ k ]]$Def[ TmaskPlot ] / Adata[[ k ]]$GDP[ TmaskPlot ] +
                       qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$Def[ TmaskPlot ] /
                       Adata[[ k ]]$GDP[ TmaskPlot ] / sqrt( nSize ) )
  }

  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Government deficit over GDP",
              tit = "Government deficit on GDP", subtit = paste( "MC runs =", nSize ),
              leg2 = c( "Deficit" ) )


  # ------ Government debt in GDP terms ------

  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[k]] <- list( Adata[[k]]$Deb[ TmaskPlot ] / Adata[[k]]$GDP[ TmaskPlot ] )
    # minimum and maximum MC runs
    min[[k]] <- list( mdata[[k]]$Deb[ TmaskPlot ] / Adata[[k]]$GDP[ TmaskPlot ] )
    max[[k]] <- list( Mdata[[k]]$Deb[ TmaskPlot ] / Adata[[k]]$GDP[ TmaskPlot ] )
    # MC confidence interval
    lo[[k]] <- list( Adata[[k]]$Deb[ TmaskPlot ] / Adata[[k]]$GDP[ TmaskPlot ] -
                       qnorm(1 - (1 - CI) / 2) * Sdata[[k]]$Deb[ TmaskPlot ] /
                       Adata[[k]]$GDP[ TmaskPlot ] / sqrt( nSize ) )
    hi[[k]] <- list( Adata[[k]]$Deb[ TmaskPlot ] / Adata[[k]]$GDP[ TmaskPlot ] +
                       qnorm(1 - (1 - CI) / 2) * Sdata[[k]]$Deb[ TmaskPlot ] /
                       Adata[[k]]$GDP[ TmaskPlot ] / sqrt( nSize ) )
  }

  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Government debt over GDP",
              tit = "Government debt on GDP", subtit = paste( "MC runs =", nSize ),
              leg2 = c( "Gov. debt" ) )


  # ------ Total credit supply and loans in GDP terms ------

  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( Adata[[ k ]]$TC[ TmaskPlot ] / Adata[[ k ]]$GDP[ TmaskPlot ],
                         Adata[[ k ]]$Loans[ TmaskPlot ] / Adata[[ k ]]$GDP[ TmaskPlot ] )
    # minimum and maximum MC runs
    min[[ k ]] <- list( mdata[[ k ]]$TC[ TmaskPlot ] / Adata[[ k ]]$GDP[ TmaskPlot ],
                        mdata[[ k ]]$Loans[ TmaskPlot ] / Adata[[ k ]]$GDP[ TmaskPlot ] )
    max[[ k ]] <- list( Mdata[[ k ]]$TC[ TmaskPlot ] / Adata[[ k ]]$GDP[ TmaskPlot ],
                        Mdata[[ k ]]$Loans[ TmaskPlot ] / Adata[[ k ]]$GDP[ TmaskPlot ] )
    # MC confidence interval
    lo[[ k ]] <- list( Adata[[ k ]]$TC[ TmaskPlot ] / Adata[[ k ]]$GDP[ TmaskPlot ] -
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$TC[ TmaskPlot ] /
                         Adata[[ k ]]$GDP[ TmaskPlot ] / sqrt( nSize ),
                       Adata[[ k ]]$Loans[ TmaskPlot ] / Adata[[ k ]]$GDP[ TmaskPlot ] -
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$Loans[ TmaskPlot ] /
                         Adata[[ k ]]$GDP[ TmaskPlot ] / sqrt( nSize ) )
    hi[[ k ]] <- list( Adata[[ k ]]$TC[ TmaskPlot ] / Adata[[ k ]]$GDP[ TmaskPlot ] +
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$TC[ TmaskPlot ] /
                         Adata[[ k ]]$GDP[ TmaskPlot ] / sqrt( nSize ),
                       Adata[[ k ]]$Loans[ TmaskPlot ] / Adata[[ k ]]$GDP[ TmaskPlot ] +
                         qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$Loans[ TmaskPlot ] /
                         Adata[[ k ]]$GDP[ TmaskPlot ] / sqrt( nSize ) )
  }

  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Total banks credit supply and firms loans debt over GDP",
              tit = "Total credit supply and loans on GDP", subtit = paste( "MC runs =", nSize ),
              leg2 = c( "Total credit", "Loans" ) )


  # ------ Unemployment and vacancy rates ------

  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( Adata[[ k ]]$U[ TmaskPlot ],
                       Adata[[ k ]]$V[ TmaskPlot ] )
    # minimum and maximum MC runs
    min[[ k ]] <- list( mdata[[ k ]]$U[ TmaskPlot ],
                      mdata[[ k ]]$V[ TmaskPlot ] )
    max[[ k ]] <- list( Mdata[[ k ]]$U[ TmaskPlot ],
                      Mdata[[ k ]]$V[ TmaskPlot ] )
    # MC confidence interval
    lo[[ k ]] <- list( Adata[[ k ]]$U[ TmaskPlot ] -
                       qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$U[ TmaskPlot ] / sqrt( nSize ),
                     Adata[[ k ]]$V[ TmaskPlot ] -
                       qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$V[ TmaskPlot ]  / sqrt( nSize ) )
    hi[[ k ]] <- list( Adata[[ k ]]$U[ TmaskPlot ] +
                       qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$U[ TmaskPlot ] / sqrt( nSize ),
                     Adata[[ k ]]$V[ TmaskPlot ] +
                       qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$V[ TmaskPlot ]  / sqrt( nSize ) )
  }

  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Rate",
              tit = "Unemployment and vacancy rates",
              subtit = paste( "MC runs =", nSize ),
              leg2 = c( "Unemployment", "Vacancy" ) )


  # ------ Real wages ------

  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( log0( Adata[[ k ]]$wAvgReal[ TmaskPlot ] ) )
    # minimum and maximum MC runs
    min[[ k ]] <- list( log0( mdata[[ k ]]$wAvgReal[ TmaskPlot ] ) )
    max[[ k ]] <- list( log0( Mdata[[ k ]]$wAvgReal[ TmaskPlot ] ) )
    # MC confidence interval
    lo[[ k ]] <- list( log0( Adata[[ k ]]$wAvgReal[ TmaskPlot ] -
                           qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$wAvgReal[ TmaskPlot ] / sqrt( nSize ) ) )
    hi[[ k ]] <- list( log0( Adata[[ k ]]$wAvgReal[ TmaskPlot ] +
                             qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$wAvgReal[ TmaskPlot ] / sqrt( nSize ) ) )
  }

  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Log wages",
              tit = "Real wages average",
              subtit = paste( "MC runs =", nSize ) )


  # ------ Real wages share in GDP terms ------

  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    SWreal <- sqrt( Sdata[[ k ]]$W1 ^ 2 + Sdata[[ k ]]$W2 ^ 2 ) / Adata[[ k ]]$CPI # very crude approximation
    AWreal <- MWreal <- mWreal <- vector( "numeric", length = nTsteps )
    for( i in 1 : nTsteps ) {
      if( is.finite( Adata[[ k ]]$CPI[ i ] ) && Adata[[ k ]]$CPI[ i ] != 0 ) {
        AWreal[ i ] <- ( Adata[[ k ]]$W1[ i ] + Adata[[ k ]]$W2[ i ] ) / Adata[[ k ]]$CPI[ i ]
        MWreal[ i ] <- ( Mdata[[ k ]]$W1[ i ] + Mdata[[ k ]]$W2[ i ] ) / Adata[[ k ]]$CPI[ i ]
        mWreal[ i ] <- ( mdata[[ k ]]$W1[ i ] + mdata[[ k ]]$W2[ i ] ) / Adata[[ k ]]$CPI[ i ]
      } else {
        AWreal[ i ] <- NA
        MWreal[ i ] <- NA
        mWreal[ i ] <- NA
      }
    }
    # MC averages
    exps[[ k ]] <- list( AWreal[ TmaskPlot ] / Adata[[ k ]]$GDP[ TmaskPlot ] )
    # minimum and maximum MC runs
    min[[ k ]] <- list( mWreal[ TmaskPlot ] / Adata[[ k ]]$GDP[ TmaskPlot ] )
    max[[ k ]] <- list( MWreal[ TmaskPlot ] / Adata[[ k ]]$GDP[ TmaskPlot ] )
    # MC confidence interval
    lo[[ k ]] <- list( AWreal[ TmaskPlot ] / Adata[[ k ]]$GDP[ TmaskPlot ] -
                       qnorm(1 - (1 - CI ) / 2) * SWreal[ TmaskPlot ] /
                       Adata[[ k ]]$GDP[ TmaskPlot ] / sqrt( nSize ) )
    hi[[ k ]] <- list( AWreal[ TmaskPlot ] / Adata[[ k ]]$GDP[ TmaskPlot ] +
                       qnorm(1 - (1 - CI ) / 2) * SWreal[ TmaskPlot ] /
                       Adata[[ k ]]$GDP[ TmaskPlot ] / sqrt( nSize ) )
  }

  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Total real wages on GDP",
              tit = "Wage share", subtit = paste( "MC runs =", nSize ) )


  # ------ Gini index ------

  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( Adata[[ k ]]$wGini[ TmaskPlot ] )
    # minimum and maximum MC runs
    min[[ k ]] <- list( mdata[[ k ]]$wGini[ TmaskPlot ] )
    max[[ k ]] <- list( Mdata[[ k ]]$wGini[ TmaskPlot ] )
    # MC confidence interval
    lo[[ k ]] <- list( Adata[[ k ]]$wGini[ TmaskPlot ] -
                       qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$wGini[ TmaskPlot ]  / sqrt( nSize ) )
    hi[[ k ]] <- list( Adata[[ k ]]$wGini[ TmaskPlot ] +
                       qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$wGini[ TmaskPlot ]  / sqrt( nSize ) )
  }

  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Workers' income Gini index",
              tit = "Gini index",
              subtit = paste( "MC runs =", nSize ) )


  # ------ Average tenure and vintage skills ------

  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( Adata[[ k ]]$sTavg[ TmaskPlot ],
                       Adata[[ k ]]$sVavg[ TmaskPlot ] )
    # minimum and maximum MC runs
    min[[ k ]] <- list( mdata[[ k ]]$sTavg[ TmaskPlot ],
                      mdata[[ k ]]$sVavg[ TmaskPlot ] )
    max[[ k ]] <- list( Mdata[[ k ]]$sTavg[ TmaskPlot ],
                      Mdata[[ k ]]$sVavg[ TmaskPlot ] )
    # MC confidence interval
    lo[[ k ]] <- list( Adata[[ k ]]$sTavg[ TmaskPlot ] -
                       qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$sTavg[ TmaskPlot ] / sqrt( nSize ),
                     Adata[[ k ]]$sVavg[ TmaskPlot ] -
                       qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$sVavg[ TmaskPlot ]  / sqrt( nSize ) )
    hi[[ k ]] <- list( Adata[[ k ]]$sTavg[ TmaskPlot ] +
                       qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$sTavg[ TmaskPlot ] / sqrt( nSize ),
                     Adata[[ k ]]$sVavg[ TmaskPlot ] +
                       qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$sVavg[ TmaskPlot ]  / sqrt( nSize ) )
  }

  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Average skills level",
              tit = "Workers skills",
              subtit = paste( "MC runs =", nSize ),
              leg2 = c( "Tenure skills", "Vintage skills" ) )


  # ------ Innovation and imitation in sector 1 ------

  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( Adata[[ k ]]$inn[ TmaskPlot ],
                       Adata[[ k ]]$imi[ TmaskPlot ] )
    # minimum and maximum MC runs
    min[[ k ]] <- list( mdata[[ k ]]$inn[ TmaskPlot ],
                      mdata[[ k ]]$imi[ TmaskPlot ] )
    max[[ k ]] <- list( Mdata[[ k ]]$inn[ TmaskPlot ],
                      Mdata[[ k ]]$imi[ TmaskPlot ] )
    # MC confidence interval
    lo[[ k ]] <- list( Adata[[ k ]]$inn[ TmaskPlot ] -
                       qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$inn[ TmaskPlot ] / sqrt( nSize ),
                     Adata[[ k ]]$imi[ TmaskPlot ] -
                       qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$imi[ TmaskPlot ] / sqrt( nSize ) )
    hi[[ k ]] <- list( Adata[[ k ]]$inn[ TmaskPlot ] +
                       qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$inn[ TmaskPlot ] / sqrt( nSize ),
                     Adata[[ k ]]$imi[ TmaskPlot ] +
                       qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$imi[ TmaskPlot ] / sqrt( nSize ) )
  }

  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Share of innovating and imitating firms",
              tit = "Innovation and imitation",
              subtit = paste( "sector 1 only / MC runs =", nSize ),
              leg2 = c( "Innovation", "Imitation" ) )


  # ------ Productivity by firm type in sector 2 ------

  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( log0( Adata[[ k ]]$A2preChg[ TmaskPlot ] ),
                       log0( Adata[[ k ]]$A2posChg[ TmaskPlot ] ) )
    # minimum and maximum MC runs
    min[[ k ]] <- list( log0( mdata[[ k ]]$A2preChg[ TmaskPlot ] ),
                      log0( mdata[[ k ]]$A2posChg[ TmaskPlot ] ) )
    max[[ k ]] <- list( log0( Mdata[[ k ]]$A2preChg[ TmaskPlot ] ),
                      log0( Mdata[[ k ]]$A2posChg[ TmaskPlot ] ) )
    # MC confidence interval
    lo[[ k ]] <- list( log0( Adata[[ k ]]$A2preChg[ TmaskPlot ] -
                             qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$A2preChg[ TmaskPlot ] / sqrt( nSize ) ),
                     log0( Adata[[ k ]]$A2posChg[ TmaskPlot ] -
                             qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$A2posChg[ TmaskPlot ] / sqrt( nSize ) ) )
    hi[[ k ]] <- list( log0( Adata[[ k ]]$A2preChg[ TmaskPlot ] +
                             qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$A2preChg[ TmaskPlot ] / sqrt( nSize ) ),
                     log0( Adata[[ k ]]$A2posChg[ TmaskPlot ] +
                             qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$A2posChg[ TmaskPlot ] / sqrt( nSize ) ) )
  }

  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Log productivity",
              tit = "Productivity by firm type",
              subtit = paste( "sector 2 only / MC runs =", nSize ),
              leg2 = firmTypes )


  # ------ Market share of post-change firms ------

  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( Adata[[ k ]]$f2posChg[ TmaskPlot ] )
    # minimum and maximum MC runs
    min[[ k ]] <- list( mdata[[ k ]]$f2posChg[ TmaskPlot ] )
    max[[ k ]] <- list( Mdata[[ k ]]$f2posChg[ TmaskPlot ] )
    # MC confidence interval
    lo[[ k ]] <- list( Adata[[ k ]]$f2posChg[ TmaskPlot ] -
                       qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$f2posChg[ TmaskPlot ] / sqrt( nSize ) )
    hi[[ k ]] <- list( Adata[[ k ]]$f2posChg[ TmaskPlot ] +
                       qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$f2posChg[ TmaskPlot ] / sqrt( nSize ) )
  }

  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time",
              ylab = "Market share",
              tit = paste( "Market share of", firmTypes[2] ),
              subtit = paste( "Sector 2 only / MC runs =", nSize ) )


  # ------ Firms net entry trend in the market ------

  netEntTrd1 <- netEntTrd2 <- list( )
  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ) {

    # calculate net entry rate trends per MC sample
    netEntTrd1[[ k ]] <- netEntTrd2[[ k ]] <- matrix( nrow = 0, ncol = nTsteps )
    for( j in 1 : nSize ) {   # for each MC case
      netEntTrd1[[ k ]] <- rbind( netEntTrd1[[ k ]],
                           hpfilter( mcData[[ k ]][ , "entry1", j ] - mcData[[ k ]][ , "exit1", j ],
                                     smoothing ) $ trend[ , 1 ] )
      netEntTrd2[[ k ]] <- rbind( netEntTrd2[[ k ]],
                           hpfilter( mcData[[ k ]][ , "entry2", j ] - mcData[[ k ]][ , "exit2", j ],
                                     smoothing ) $ trend[ , 1 ] )
    }

    # calculate MC results
    AnetEntrTrd1 <- colMeans( netEntTrd1[[ k ]], na.rm = TRUE )
    AnetEntrTrd2 <- colMeans( netEntTrd2[[ k ]], na.rm = TRUE )
    SnetEntrTrd1 <- colSds( netEntTrd1[[ k ]], na.rm = TRUE )
    SnetEntrTrd2 <- colSds( netEntTrd2[[ k ]], na.rm = TRUE )
    mnetEntrTrd1 <- colMins( netEntTrd1[[ k ]], na.rm = TRUE )
    mnetEntrTrd2 <- colMins( netEntTrd2[[ k ]], na.rm = TRUE )
    MnetEntrTrd1 <- colMaxs( netEntTrd1[[ k ]], na.rm = TRUE )
    MnetEntrTrd2 <- colMaxs( netEntTrd2[[ k ]], na.rm = TRUE )

    # MC averages
    exps[[ k ]] <- list( AnetEntrTrd2[ TmaskPlot ],
                       AnetEntrTrd1[ TmaskPlot ] )
    # minimum and maximum MC runs
    min[[ k ]] <- list( mnetEntrTrd2[ TmaskPlot ],
                      mnetEntrTrd1[ TmaskPlot ] )
    max[[ k ]] <- list( MnetEntrTrd2[ TmaskPlot ],
                      MnetEntrTrd1[ TmaskPlot ] )
    # MC confidence interval
    lo[[ k ]] <- list( AnetEntrTrd2[ TmaskPlot ] -
                       qnorm(1 - (1 - CI ) / 2) * SnetEntrTrd2[ TmaskPlot ] / sqrt( nSize ),
                     AnetEntrTrd1[ TmaskPlot ] -
                       qnorm(1 - (1 - CI ) / 2) * SnetEntrTrd1[ TmaskPlot ]  / sqrt( nSize ) )
    hi[[ k ]] <- list( AnetEntrTrd2[ TmaskPlot ] +
                       qnorm(1 - (1 - CI ) / 2) * SnetEntrTrd2[ TmaskPlot ] / sqrt( nSize ),
                     AnetEntrTrd1[ TmaskPlot ] +
                       qnorm(1 - (1 - CI ) / 2) * SnetEntrTrd1[ TmaskPlot ]  / sqrt( nSize ) )
  }

  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Number of net entrant firms (HP-filtered)",
              tit = "Net entry of firms trend",
              subtit = paste( "MC runs =", nSize ),
              leg2 = c( "Consumption-good sector", "Capital-good sector" ) )


  # ------ Concentration ------

  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( Adata[[ k ]]$HH2[ TmaskPlot ] )
    # minimum and maximum MC runs
    min[[ k ]] <- list( mdata[[ k ]]$HH2[ TmaskPlot ] )
    max[[ k ]] <- list( Mdata[[ k ]]$HH2[ TmaskPlot ] )
    # MC confidence interval
    lo[[ k ]] <- list( Adata[[ k ]]$HH2[ TmaskPlot ] -
                       qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$HH2[ TmaskPlot ] / sqrt( nSize ) )
    hi[[ k ]] <- list( Adata[[ k ]]$HH2[ TmaskPlot ] +
                       qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$HH2[ TmaskPlot ] / sqrt( nSize ) )
  }

  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time",
              ylab = "Standardized Herfindahl-Hirschman index",
              tit = "Market concentration",
              subtit = paste( "Sector 2 only / MC runs =", nSize ) )


  # ------ Markup ------

  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( Adata[[ k ]]$mu2avg[ TmaskPlot ] )
    # minimum and maximum MC runs
    min[[ k ]] <- list( mdata[[ k ]]$mu2avg[ TmaskPlot ] )
    max[[ k ]] <- list( Mdata[[ k ]]$mu2avg[ TmaskPlot ] )
    # MC confidence interval
    lo[[ k ]] <- list( Adata[[ k ]]$mu2avg[ TmaskPlot ] -
                       qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$mu2avg[ TmaskPlot ] / sqrt( nSize ) )
    hi[[ k ]] <- list( Adata[[ k ]]$mu2avg[ TmaskPlot ] +
                       qnorm(1 - (1 - CI ) / 2) * Sdata[[ k ]]$mu2avg[ TmaskPlot ] / sqrt( nSize ) )
  }

  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Weighted average mark-up rate",
              tit = "Mark-up average",
              subtit = paste( "Sector 2 only / MC runs =", nSize ) )

}
