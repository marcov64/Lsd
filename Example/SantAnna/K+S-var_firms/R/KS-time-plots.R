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
    Adata[[ k ]][ "GbailGDP" ] <- Adata[[ k ]]$Gbail / Adata[[ k ]]$GDPnom
    mdata[[ k ]][ "GbailGDP" ] <- mdata[[ k ]]$Gbail / Adata[[ k ]]$GDPnom
    Mdata[[ k ]][ "GbailGDP" ] <- Mdata[[ k ]]$Gbail / Adata[[ k ]]$GDPnom
    Sdata[[ k ]][ "GbailGDP" ] <- Sdata[[ k ]]$Gbail / Adata[[ k ]]$GDPnom
  }

  plot_lists( c( "TaxGDP", "GGDP", "GbailGDP" ), Adata, mdata, Mdata, Sdata,
              leg = legends, mask = TmaskPlot, nMC = nSize, CI = CI,
              col = colors, lty = lTypes, xlab = "Time",
              ylab = "Government tax income and expenditure over GDP",
              tit = "Government income and expenditure",
              subtit = paste( "MC runs =", nSize ),
              leg2 = c( "Tax", "Gov. expenditure", "Bank bail-out" ) )


  # ------ Government deficit in GDP terms------

  plot_lists( c( "DefGDP", "DefPgdp" ), Adata, mdata, Mdata, Sdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Government deficit over GDP",
              tit = "Government deficit", subtit = paste( "MC runs =", nSize ),
              leg2 = c( "Total", "Primary" ) )


  # ------ Government debt in GDP terms ------

  plot_lists( "DebGDP", Adata, mdata, Mdata, Sdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Government debt over GDP",
              tit = "Government debt", subtit = paste( "MC runs =", nSize ) )


  # ------ Total credit supply and loans in GDP terms ------

  # add the series to dataset
  for( k in 1 : nExp ) {
    Adata[[ k ]][ "TCGDP" ] <- Adata[[ k ]]$TC / Adata[[ k ]]$GDPnom
    mdata[[ k ]][ "TCGDP" ] <- mdata[[ k ]]$TC / Adata[[ k ]]$GDPnom
    Mdata[[ k ]][ "TCGDP" ] <- Mdata[[ k ]]$TC / Adata[[ k ]]$GDPnom
    Sdata[[ k ]][ "TCGDP" ] <- Sdata[[ k ]]$TC / Adata[[ k ]]$GDPnom
    Adata[[ k ]][ "LoansGDP" ] <- Adata[[ k ]]$Loans / Adata[[ k ]]$GDPnom
    mdata[[ k ]][ "LoansGDP" ] <- mdata[[ k ]]$Loans / Adata[[ k ]]$GDPnom
    Mdata[[ k ]][ "LoansGDP" ] <- Mdata[[ k ]]$Loans / Adata[[ k ]]$GDPnom
    Sdata[[ k ]][ "LoansGDP" ] <- Sdata[[ k ]]$Loans / Adata[[ k ]]$GDPnom
  }

  plot_lists( c( "TCGDP", "LoansGDP" ), Adata, mdata, Mdata, Sdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,
              lty = lTypes, xlab = "Time",
              ylab = "Total bank credit available and firm debt stock over GDP",
              tit = "Bank credit supply and firm loans",
              subtit = paste( "MC runs =", nSize ),
              leg2 = c( "Credit available", "Loans" ) )


  # ------ Credit demand and supply in GDP terms ------

  # add the series to dataset
  for( k in 1 : nExp ) {
    Adata[[ k ]][ "CDGDP" ] <- Adata[[ k ]]$CD / Adata[[ k ]]$GDPnom
    mdata[[ k ]][ "CDGDP" ] <- mdata[[ k ]]$CD / Adata[[ k ]]$GDPnom
    Mdata[[ k ]][ "CDGDP" ] <- Mdata[[ k ]]$CD / Adata[[ k ]]$GDPnom
    Sdata[[ k ]][ "CDGDP" ] <- Sdata[[ k ]]$CD / Adata[[ k ]]$GDPnom
    Adata[[ k ]][ "CSGDP" ] <- Adata[[ k ]]$CS / Adata[[ k ]]$GDPnom
    mdata[[ k ]][ "CSGDP" ] <- mdata[[ k ]]$CS / Adata[[ k ]]$GDPnom
    Mdata[[ k ]][ "CSGDP" ] <- Mdata[[ k ]]$CS / Adata[[ k ]]$GDPnom
    Sdata[[ k ]][ "CSGDP" ] <- Sdata[[ k ]]$CS / Adata[[ k ]]$GDPnom
  }

  plot_lists( c( "CDGDP", "CSGDP" ), Adata, mdata, Mdata, Sdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,
              lty = lTypes, xlab = "Time",
              ylab = "Effective total firm credit demand and bank credit supply over GDP",
              tit = "Credit demand and supply flow on GDP",
              subtit = paste( "MC runs =", nSize ),
              leg2 = c( "Demand", "Supply" ) )


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


  # ------ Firms net entry trend in the market ------

  # add the series to dataset
  for( k in 1 : nExp ) {
    # calculate net entry rate trends per MC sample
    nEntT1 <- nEntT2 <- matrix( nrow = 0, ncol = nTsteps )
    for( j in 1 : nSize ) {   # for each MC case
      nEntT1 <- rbind( nEntT1, hpfilter( mcData[[ k ]][ , "entry1exit", j ],
                                            smoothing ) $ trend[ , 1 ] )
      nEntT2 <- rbind( nEntT2, hpfilter( mcData[[ k ]][ , "entry2exit", j ],
                                            smoothing ) $ trend[ , 1 ] )
  }

    Adata[[ k ]][ "nEntT1" ] <- colMeans( nEntT1, na.rm = TRUE )
    mdata[[ k ]][ "nEntT1" ] <- colMins( nEntT1, na.rm = TRUE )
    Mdata[[ k ]][ "nEntT1" ] <- colMaxs( nEntT1, na.rm = TRUE )
    Sdata[[ k ]][ "nEntT1" ] <- colSds( nEntT1, na.rm = TRUE )
    Adata[[ k ]][ "nEntT2" ] <- colMeans( nEntT2, na.rm = TRUE )
    mdata[[ k ]][ "nEntT2" ] <- colMins( nEntT2, na.rm = TRUE )
    Mdata[[ k ]][ "nEntT2" ] <- colMaxs( nEntT2, na.rm = TRUE )
    Sdata[[ k ]][ "nEntT2" ] <- colSds( nEntT2, na.rm = TRUE )
  }

  plot_lists( c( "nEntT1", "nEntT2" ), Adata, mdata, Mdata, Sdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,
              lty = lTypes, xlab = "Time",
              ylab = "Number of net entrant firms (HP-filtered)",
              tit = "Net entry of firms trend",
              subtit = paste( "MC runs =", nSize ),
              leg2 = c( "Consumption-good sector", "Capital-good sector" ) )

}
