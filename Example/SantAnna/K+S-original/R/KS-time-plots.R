#******************************************************************
#
# ---------------- K+S aggregates time plots --------------------
#
#******************************************************************

# remove warnings for support functions
# !diagnostics suppress = log0, plot_lists, hpfilter, colSds, colMins, colMaxs


time_plots <- function( mcData, Adata, mdata, Mdata, Sdata, nExp, nSize, nTsteps,
                        TmaskPlot, CI, legends, colors, lTypes, smoothing ) {

  # ------ GDP, consumption and investment cases comparison charts ------

  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( log0( Adata[[ k ]]$GDP[ TmaskPlot ] ),
                         log0( Adata[[ k ]]$I[ TmaskPlot ] ),
                         log0( Adata[[ k ]]$D2[ TmaskPlot ] ) )
    # minimum and maximum MC runs
    min[[ k ]] <- list( log0( mdata[[ k ]]$GDP[ TmaskPlot ] ),
                        log0( mdata[[ k ]]$I[ TmaskPlot ] ),
                        log0( mdata[[ k ]]$D2[ TmaskPlot ] ) )
    max[[ k ]] <- list( log0( Mdata[[ k ]]$GDP[ TmaskPlot ] ),
                        log0( Mdata[[ k ]]$I[ TmaskPlot ] ),
                        log0( Mdata[[ k ]]$D2[ TmaskPlot ] ) )
    # MC confidence interval
    lo[[ k ]] <- list( log0( Adata[[ k ]]$GDP[ TmaskPlot ] -
                               qnorm( 1 - ( 1 - CI ) / 2 ) * Sdata[[ k ]]$GDP[ TmaskPlot ] / sqrt( nSize ) ),
                       log0( Adata[[ k ]]$I[ TmaskPlot ] -
                               qnorm( 1 - ( 1 - CI ) / 2 ) * Sdata[[ k ]]$I[ TmaskPlot ] / sqrt( nSize ) ),
                       log0( Adata[[ k ]]$D2[ TmaskPlot ] -
                               qnorm( 1 - ( 1 - CI ) / 2 ) * Sdata[[ k ]]$D2[ TmaskPlot ]  / sqrt( nSize ) ) )
    hi[[ k ]] <- list( log0( Adata[[ k ]]$GDP[ TmaskPlot ] +
                               qnorm( 1 - ( 1 - CI ) / 2 ) * Sdata[[ k ]]$GDP[ TmaskPlot ] / sqrt( nSize ) ),
                       log0( Adata[[ k ]]$I[ TmaskPlot ] +
                               qnorm( 1 - ( 1 - CI ) / 2 ) * Sdata[[ k ]]$I[ TmaskPlot ] / sqrt( nSize ) ),
                       log0( Adata[[ k ]]$D2[ TmaskPlot ] +
                               qnorm( 1 - ( 1 - CI ) / 2 ) * Sdata[[ k ]]$D2[ TmaskPlot ]  / sqrt( nSize ) ) )
  }

  plot_lists( exps, min, max, lo, hi, leg = legends, col = colors,
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
                               qnorm( 1 - ( 1 - CI ) / 2 ) * Sdata[[ k ]]$GDP[ TmaskPlot ] / sqrt( nSize ) ),
                       log0( Agdp100[ TmaskPlot ] -
                               qnorm( 1 - ( 1 - CI ) / 2 ) * Sgdp100[ TmaskPlot ] / sqrt( nSize ) ) )
    hi[[ k ]] <- list( log0( Adata[[ k ]]$GDP[ TmaskPlot ] +
                               qnorm( 1 - ( 1 - CI ) / 2 ) * Sdata[[ k ]]$GDP[ TmaskPlot ] / sqrt( nSize ) ),
                       log0( Agdp100[ TmaskPlot ] +
                               qnorm( 1 - ( 1 - CI ) / 2 ) * Sgdp100[ TmaskPlot ] / sqrt( nSize ) ) )
  }

  plot_lists( exps, min, max, lo, hi, leg = legends, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Log GDP",
              tit = "GDP", subtit = paste( "MC runs =", nSize ),
              leg2 = c( "Effective GDP", "GDP @ 100% utilization" ) )


  # ------ Tax & government expenditures in GDP terms ------

  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( Adata[[ k ]]$Tax[ TmaskPlot ] / Adata[[ k ]]$GDPnom[ TmaskPlot ],
                         Adata[[ k ]]$G[ TmaskPlot ] / Adata[[ k ]]$GDPnom[ TmaskPlot ] )
    # minimum and maximum MC runs
    min[[ k ]] <- list( mdata[[ k ]]$Tax[ TmaskPlot ] / Adata[[ k ]]$GDPnom[ TmaskPlot ],
                        mdata[[ k ]]$G[ TmaskPlot ] / Adata[[ k ]]$GDPnom[ TmaskPlot ] )
    max[[ k ]] <- list( Mdata[[ k ]]$Tax[ TmaskPlot ] / Adata[[ k ]]$GDPnom[ TmaskPlot ],
                        Mdata[[ k ]]$G[ TmaskPlot ] / Adata[[ k ]]$GDPnom[ TmaskPlot ] )
    # MC confidence interval
    lo[[ k ]] <- list( Adata[[ k ]]$Tax[ TmaskPlot ] / Adata[[ k ]]$GDPnom[ TmaskPlot ] -
                         qnorm( 1 - ( 1 - CI ) / 2 ) * Sdata[[ k ]]$Tax[ TmaskPlot ] /
                         Adata[[ k ]]$GDPnom[ TmaskPlot ] / sqrt( nSize ),
                       Adata[[ k ]]$G[ TmaskPlot ] / Adata[[ k ]]$GDPnom[ TmaskPlot ] -
                         qnorm( 1 - ( 1 - CI ) / 2 ) * Sdata[[ k ]]$G[ TmaskPlot ] /
                         Adata[[ k ]]$GDPnom[ TmaskPlot ] / sqrt( nSize ) )
    hi[[ k ]] <- list( Adata[[ k ]]$Tax[ TmaskPlot ] / Adata[[ k ]]$GDPnom[ TmaskPlot ] +
                         qnorm( 1 - ( 1 - CI ) / 2 ) * Sdata[[ k ]]$Tax[ TmaskPlot ] /
                         Adata[[ k ]]$GDPnom[ TmaskPlot ] / sqrt( nSize ),
                       Adata[[ k ]]$G[ TmaskPlot ] / Adata[[ k ]]$GDPnom[ TmaskPlot ] +
                         qnorm( 1 - ( 1 - CI ) / 2 ) * Sdata[[ k ]]$G[ TmaskPlot ] /
                         Adata[[ k ]]$GDPnom[ TmaskPlot ] / sqrt( nSize ) )
  }

  plot_lists( exps, min, max, lo, hi, leg = legends, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Log government tax income and expenditure over GDP",
              tit = "Government income and expenditure on GDP", subtit = paste( "MC runs =", nSize ),
              leg2 = c( "Tax", "Total expenditure" ) )


  # ------ Government deficit in GDP terms------

  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( Adata[[ k ]]$Def[ TmaskPlot ] / Adata[[ k ]]$GDPnom[ TmaskPlot ] )
    # minimum and maximum MC runs
    min[[ k ]] <- list( mdata[[ k ]]$Def[ TmaskPlot ] / Adata[[ k ]]$GDPnom[ TmaskPlot ] )
    max[[ k ]] <- list( Mdata[[ k ]]$Def[ TmaskPlot ] / Adata[[ k ]]$GDPnom[ TmaskPlot ] )
    # MC confidence interval
    lo[[ k ]] <- list( Adata[[ k ]]$Def[ TmaskPlot ] / Adata[[ k ]]$GDPnom[ TmaskPlot ] -
                         qnorm( 1 - ( 1 - CI ) / 2 ) * Sdata[[ k ]]$Def[ TmaskPlot ] /
                         Adata[[ k ]]$GDPnom[ TmaskPlot ] / sqrt( nSize ) )
    hi[[ k ]] <- list( Adata[[ k ]]$Def[ TmaskPlot ] / Adata[[ k ]]$GDPnom[ TmaskPlot ] +
                         qnorm( 1 - ( 1 - CI ) / 2 ) * Sdata[[ k ]]$Def[ TmaskPlot ] /
                         Adata[[ k ]]$GDPnom[ TmaskPlot ] / sqrt( nSize ) )
  }

  plot_lists( exps, min, max, lo, hi, leg = legends, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Government deficit over GDP",
              tit = "Government deficit on GDP", subtit = paste( "MC runs =", nSize ),
              leg2 = c( "Deficit" ) )


  # ------ Government debt in GDP terms ------

  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[k]] <- list( Adata[[k]]$Deb[ TmaskPlot ] / Adata[[k]]$GDPnom[ TmaskPlot ] )
    # minimum and maximum MC runs
    min[[k]] <- list( mdata[[k]]$Deb[ TmaskPlot ] / Adata[[k]]$GDPnom[ TmaskPlot ] )
    max[[k]] <- list( Mdata[[k]]$Deb[ TmaskPlot ] / Adata[[k]]$GDPnom[ TmaskPlot ] )
    # MC confidence interval
    lo[[k]] <- list( Adata[[k]]$Deb[ TmaskPlot ] / Adata[[k]]$GDPnom[ TmaskPlot ] -
                       qnorm(1 - (1 - CI) / 2) * Sdata[[k]]$Deb[ TmaskPlot ] /
                       Adata[[k]]$GDPnom[ TmaskPlot ] / sqrt( nSize ) )
    hi[[k]] <- list( Adata[[k]]$Deb[ TmaskPlot ] / Adata[[k]]$GDPnom[ TmaskPlot ] +
                       qnorm(1 - (1 - CI) / 2) * Sdata[[k]]$Deb[ TmaskPlot ] /
                       Adata[[k]]$GDPnom[ TmaskPlot ] / sqrt( nSize ) )
  }

  plot_lists( exps, min, max, lo, hi, leg = legends, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Government debt over GDP",
              tit = "Government debt on GDP", subtit = paste( "MC runs =", nSize ),
              leg2 = c( "Gov. debt" ) )


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
                         qnorm( 1 - ( 1 - CI ) / 2 ) * Sdata[[ k ]]$U[ TmaskPlot ] / sqrt( nSize ),
                       Adata[[ k ]]$V[ TmaskPlot ] -
                         qnorm( 1 - ( 1 - CI ) / 2 ) * Sdata[[ k ]]$V[ TmaskPlot ]  / sqrt( nSize ) )
    hi[[ k ]] <- list( Adata[[ k ]]$U[ TmaskPlot ] +
                         qnorm( 1 - ( 1 - CI ) / 2 ) * Sdata[[ k ]]$U[ TmaskPlot ] / sqrt( nSize ),
                       Adata[[ k ]]$V[ TmaskPlot ] +
                         qnorm( 1 - ( 1 - CI ) / 2 ) * Sdata[[ k ]]$V[ TmaskPlot ]  / sqrt( nSize ) )
  }

  plot_lists( exps, min, max, lo, hi, leg = legends, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Rate",
              tit = "Unemployment and vacancy rates",
              subtit = paste( "MC runs =", nSize ),
              leg2 = c( "Unemployment", "Vacancy" ) )


  # ------ Real wages ------

  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[ k ]] <- list( log0( Adata[[ k ]]$wReal[ TmaskPlot ] ) )
    # minimum and maximum MC runs
    min[[ k ]] <- list( log0( mdata[[ k ]]$wReal[ TmaskPlot ] ) )
    max[[ k ]] <- list( log0( Mdata[[ k ]]$wReal[ TmaskPlot ] ) )
    # MC confidence interval
    lo[[ k ]] <- list( log0( Adata[[ k ]]$wReal[ TmaskPlot ] -
                               qnorm( 1 - ( 1 - CI ) / 2 ) * Sdata[[ k ]]$wReal[ TmaskPlot ] / sqrt( nSize ) ) )
    hi[[ k ]] <- list( log0( Adata[[ k ]]$wReal[ TmaskPlot ] +
                               qnorm( 1 - ( 1 - CI ) / 2 ) * Sdata[[ k ]]$wReal[ TmaskPlot ] / sqrt( nSize ) ) )
  }

  plot_lists( exps, min, max, lo, hi, leg = legends, col = colors,
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
    exps[[ k ]] <- list( AWreal[ TmaskPlot ] / Adata[[ k ]]$GDPnom[ TmaskPlot ] )
    # minimum and maximum MC runs
    min[[ k ]] <- list( mWreal[ TmaskPlot ] / Adata[[ k ]]$GDPnom[ TmaskPlot ] )
    max[[ k ]] <- list( MWreal[ TmaskPlot ] / Adata[[ k ]]$GDPnom[ TmaskPlot ] )
    # MC confidence interval
    lo[[ k ]] <- list( AWreal[ TmaskPlot ] / Adata[[ k ]]$GDPnom[ TmaskPlot ] -
                         qnorm( 1 - ( 1 - CI ) / 2 ) * SWreal[ TmaskPlot ] /
                         Adata[[ k ]]$GDPnom[ TmaskPlot ] / sqrt( nSize ) )
    hi[[ k ]] <- list( AWreal[ TmaskPlot ] / Adata[[ k ]]$GDPnom[ TmaskPlot ] +
                         qnorm( 1 - ( 1 - CI ) / 2 ) * SWreal[ TmaskPlot ] /
                         Adata[[ k ]]$GDPnom[ TmaskPlot ] / sqrt( nSize ) )
  }

  plot_lists( exps, min, max, lo, hi, leg = legends, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Total real wages on GDP",
              tit = "Wage share", subtit = paste( "MC runs =", nSize ) )


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
                         qnorm( 1 - ( 1 - CI ) / 2 ) * Sdata[[ k ]]$inn[ TmaskPlot ] / sqrt( nSize ),
                       Adata[[ k ]]$imi[ TmaskPlot ] -
                         qnorm( 1 - ( 1 - CI ) / 2 ) * Sdata[[ k ]]$imi[ TmaskPlot ] / sqrt( nSize ) )
    hi[[ k ]] <- list( Adata[[ k ]]$inn[ TmaskPlot ] +
                         qnorm( 1 - ( 1 - CI ) / 2 ) * Sdata[[ k ]]$inn[ TmaskPlot ] / sqrt( nSize ),
                       Adata[[ k ]]$imi[ TmaskPlot ] +
                         qnorm( 1 - ( 1 - CI ) / 2 ) * Sdata[[ k ]]$imi[ TmaskPlot ] / sqrt( nSize ) )
  }

  plot_lists( exps, min, max, lo, hi, leg = legends, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Share of innovating and imitating firms",
              tit = "Innovation and imitation",
              subtit = paste( "sector 1 only / MC runs =", nSize ),
              leg2 = c( "Innovation", "Imitation" ) )


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
                         qnorm( 1 - ( 1 - CI ) / 2 ) * Sdata[[ k ]]$HH2[ TmaskPlot ] / sqrt( nSize ) )
    hi[[ k ]] <- list( Adata[[ k ]]$HH2[ TmaskPlot ] +
                         qnorm( 1 - ( 1 - CI ) / 2 ) * Sdata[[ k ]]$HH2[ TmaskPlot ] / sqrt( nSize ) )
  }

  plot_lists( exps, min, max, lo, hi, leg = legends, col = colors,
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
                         qnorm( 1 - ( 1 - CI ) / 2 ) * Sdata[[ k ]]$mu2avg[ TmaskPlot ] / sqrt( nSize ) )
    hi[[ k ]] <- list( Adata[[ k ]]$mu2avg[ TmaskPlot ] +
                         qnorm( 1 - ( 1 - CI ) / 2 ) * Sdata[[ k ]]$mu2avg[ TmaskPlot ] / sqrt( nSize ) )
  }

  plot_lists( exps, min, max, lo, hi, leg = legends, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Weighted average mark-up rate",
              tit = "Mark-up average",
              subtit = paste( "Sector 2 only / MC runs =", nSize ) )

}
