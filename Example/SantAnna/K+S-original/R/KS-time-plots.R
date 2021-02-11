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

  plot_lists( c( "GDP", "I", "D2" ), Adata, mdata, Mdata, Sdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, log0 = TRUE, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Logs",
              tit = "GDP, investment and consumption",
              subtit = paste( "MC runs =", nSize ),
              leg2 = c( "GDP", "Investment", "Consumption" ) )


  # ------ GDP Graphs ------

  # add the GDP @ 100% utilization series to dataset
  for( k in 1 : nExp ) {
    Agdp100 <- vector( "numeric", length = nTsteps )
    for( i in 1 : nTsteps ) {
      if( is.finite( Adata[[ k ]]$Q2u[ i ] ) && Adata[[ k ]]$Q2u[ i ] != 0 )
        Agdp100[ i ] <- Adata[[ k ]]$GDP[ i ] / Adata[[ k ]]$Q2u[ i ]
      else
        Agdp100[ i ] <- NA
    }

    Adata[[ k ]][ "GDP100" ] <- Agdp100
    mdata[[ k ]][ "GDP100" ] <- mdata[[ k ]]$GDP / min( Adata[[ k ]]$Q2u, na.rm = TRUE )
    Mdata[[ k ]][ "GDP100" ] <- Mdata[[ k ]]$GDP / min( Adata[[ k ]]$Q2u, na.rm = TRUE )
    Sdata[[ k ]][ "GDP100" ] <- sqrt( Sdata[[ k ]]$GDP ^ 2 + Sdata[[ k ]]$Q2u ^ 2 )
  }

  plot_lists( c( "GDP", "GDP100" ), Adata, mdata, Mdata, Sdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, log0 = TRUE, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Log real GDP",
              tit = "GDP", subtit = paste( "MC runs =", nSize ),
              leg2 = c( "Effective GDP", "GDP @ 100% utilization" ) )


  # ------ Tax & government expenditures in GDP terms ------

  # add the series to dataset
  for( k in 1 : nExp ) {
    Adata[[ k ]][ "TaxGDP" ] <- Adata[[ k ]]$Tax / Adata[[ k ]]$GDPnom
    mdata[[ k ]][ "TaxGDP" ] <- mdata[[ k ]]$Tax / Adata[[ k ]]$GDPnom
    Mdata[[ k ]][ "TaxGDP" ] <- Mdata[[ k ]]$Tax / Adata[[ k ]]$GDPnom
    Sdata[[ k ]][ "TaxGDP" ] <- Sdata[[ k ]]$Tax / Adata[[ k ]]$GDPnom
    Adata[[ k ]][ "GGDP" ] <- Adata[[ k ]]$G / Adata[[ k ]]$GDPnom
    mdata[[ k ]][ "GGDP" ] <- mdata[[ k ]]$G / Adata[[ k ]]$GDPnom
    Mdata[[ k ]][ "GGDP" ] <- Mdata[[ k ]]$G / Adata[[ k ]]$GDPnom
    Sdata[[ k ]][ "GGDP" ] <- Sdata[[ k ]]$G / Adata[[ k ]]$GDPnom
  }

  plot_lists( c( "TaxGDP", "GGDP" ), Adata, mdata, Mdata, Sdata,
              leg = legends, mask = TmaskPlot, nMC = nSize, CI = CI,
              col = colors, lty = lTypes, xlab = "Time",
              ylab = "Government tax income and expenditure over GDP",
              tit = "Government income and expenditure",
              subtit = paste( "MC runs =", nSize ),
              leg2 = c( "Tax", "Gov. expenditure" ) )


  # ------ Government deficit in GDP terms------

  plot_lists( "DefGDP", Adata, mdata, Mdata, Sdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Government deficit over GDP",
              tit = "Government deficit", subtit = paste( "MC runs =", nSize ) )


  # ------ Government debt in GDP terms ------

  plot_lists( "DebGDP", Adata, mdata, Mdata, Sdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Government debt over GDP",
              tit = "Government debt", subtit = paste( "MC runs =", nSize ) )


  # ------ Total loans in GDP terms ------

  # add the series to dataset
  for( k in 1 : nExp ) {
    Adata[[ k ]][ "LoansGDP" ] <- Adata[[ k ]]$Loans / Adata[[ k ]]$GDPnom
    mdata[[ k ]][ "LoansGDP" ] <- mdata[[ k ]]$Loans / Adata[[ k ]]$GDPnom
    Mdata[[ k ]][ "LoansGDP" ] <- Mdata[[ k ]]$Loans / Adata[[ k ]]$GDPnom
    Sdata[[ k ]][ "LoansGDP" ] <- Sdata[[ k ]]$Loans / Adata[[ k ]]$GDPnom
  }

  plot_lists( "LoansGDP", Adata, mdata, Mdata, Sdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,
              lty = lTypes, xlab = "Time",
              ylab = "Total firm debt stock over GDP",
              tit = "Firm loans",
              subtit = paste( "MC runs =", nSize ) )


  # ------ Unemployment and vacancy rates ------

  plot_lists( c( "U", "V" ), Adata, mdata, Mdata, Sdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Unemployment and vacancy rates",
              tit = "Unemployment and vacancy",
              subtit = paste( "MC runs =", nSize ),
              leg2 = c( "Unemployment", "Vacancy" ) )


  # ------ Real wages ------

  plot_lists( "wReal", Adata, mdata, Mdata, Sdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, log0 = TRUE, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Log real wage",
              tit = "Real wage",
              subtit = paste( "MC runs =", nSize ) )


  # ------ Real wages share in GDP terms ------

  # add the series to dataset
  for( k in 1 : nExp ) {
    Adata[[ k ]][ "WGDP" ] <- ( Adata[[ k ]]$W1 + Adata[[ k ]]$W2 ) /
      Adata[[ k ]]$GDPnom
    mdata[[ k ]][ "WGDP" ] <- ( mdata[[ k ]]$W1 + mdata[[ k ]]$W2 ) /
      Adata[[ k ]]$GDPnom
    Mdata[[ k ]][ "WGDP" ] <- ( Mdata[[ k ]]$W1 + Mdata[[ k ]]$W2 ) /
      Adata[[ k ]]$GDPnom
    Sdata[[ k ]][ "WGDP" ] <- sqrt( Sdata[[ k ]]$W1 ^ 2 + Sdata[[ k ]]$W2 ^ 2 ) /
      Adata[[ k ]]$GDPnom
  }

  plot_lists( "WGDP", Adata, mdata, Mdata, Sdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Total real wages on GDP",
              tit = "Wage share", subtit = paste( "MC runs =", nSize ) )


  # ------ Innovation and imitation in sector 1 ------

  plot_lists( c( "inn", "imi" ), Adata, mdata, Mdata, Sdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,
              lty = lTypes, xlab = "Time",
              ylab = "Share of innovating and imitating firms",
              tit = "Innovation and imitation",
              subtit = paste( "Capital-good sector only / MC runs =", nSize ),
              leg2 = c( "Innovation", "Imitation" ) )


  # ------ Productivity ------

  plot_lists( c( "A", "A1", "A2" ), Adata, mdata, Mdata, Sdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, log = TRUE, col = colors,
              lty = lTypes, xlab = "Time",
              ylab = "Relative average log labor productivity",
              tit = "Productivity",
              subtit = paste( "MC runs =", nSize ),
              leg2 = c( "Overall", "Capital-good sector", "Consumption-good sector" ) )


  # ------ Concentration ------

  plot_lists( c( "HH1", "HH2" ), Adata, mdata, Mdata, Sdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,
              lty = lTypes, xlab = "Time",
              ylab = "Standardized Herfindahl-Hirschman index",
              tit = "Market concentration",
              subtit = paste( "MC runs =", nSize ),
              leg2 = c( "Capital-good sector", "Consumption-good sector" ) )


  # ------ Markup ------

  plot_lists( "mu2avg", Adata, mdata, Mdata, Sdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Weighted average mark-up rate",
              tit = "Mark-up",
              subtit = paste( "Consumption-good sector only / MC runs =", nSize ) )

}
