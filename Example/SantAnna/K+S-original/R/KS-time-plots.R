#******************************************************************
#
# ---------------- K+S aggregates time plots --------------------
#
#******************************************************************

# remove warnings for support functions
# !diagnostics suppress = log0, plot_lists, hpfilter, colSds, colMins, colMaxs


time_plots <- function( mcData, Pdata, mdata, Mdata, Sdata, cdata, Cdata,
                        mcStat, nExp, nSize, nTsteps, TmaskPlot, CI, legends,
                        colors, lTypes, smoothing ) {

  # ------ GDP, consumption and investment cases comparison charts ------

  plot_lists( c( "GDP", "I", "D2" ), Pdata, mdata, Mdata, cdata, Cdata,
              leg = legends, mask = TmaskPlot, nMC = nSize, CI = CI, log0 = TRUE,
              col = colors, lty = lTypes, xlab = "Time", ylab = "Logs",
              tit = "GDP, investment and consumption",
              subtit = paste( "MC runs =", nSize, "/ MC", mcStat ),
              leg2 = c( "GDP", "Investment", "Consumption" ) )


  # ------ GDP Graphs ------

  # add the GDP @ 100% utilization series to dataset
  for( k in 1 : nExp ) {
    Pdata[[ k ]][ "GDP100" ] <- Pdata[[ k ]]$GDP / Pdata[[ k ]]$Q2u
    mdata[[ k ]][ "GDP100" ] <- mdata[[ k ]]$GDP / Pdata[[ k ]]$Q2u
    Mdata[[ k ]][ "GDP100" ] <- Mdata[[ k ]]$GDP / Pdata[[ k ]]$Q2u
    Sdata[[ k ]][ "GDP100" ] <- sqrt( Sdata[[ k ]]$GDP ^ 2 + Sdata[[ k ]]$Q2u ^ 2 )
  }

  plot_lists( c( "GDP", "GDP100" ), Pdata, mdata, Mdata, sdMC = Sdata,
              statMC = mcStat, leg = legends, mask = TmaskPlot, nMC = nSize,
              CI = CI, log0 = TRUE, col = colors, lty = lTypes,
              xlab = "Time", ylab = "Log real GDP",
              tit = "GDP", subtit = paste( "MC runs =", nSize, "/ MC", mcStat ),
              leg2 = c( "Effective GDP", "GDP @ 100% utilization" ) )


  # ------ Tax & government expenditures in GDP terms ------

  # add the series to dataset
  for( k in 1 : nExp ) {
    Pdata[[ k ]][ "TaxGDP" ] <- Pdata[[ k ]]$Tax / Pdata[[ k ]]$GDPnom
    mdata[[ k ]][ "TaxGDP" ] <- mdata[[ k ]]$Tax / Pdata[[ k ]]$GDPnom
    Mdata[[ k ]][ "TaxGDP" ] <- Mdata[[ k ]]$Tax / Pdata[[ k ]]$GDPnom
    cdata[[ k ]][ "TaxGDP" ] <- cdata[[ k ]]$Tax / Pdata[[ k ]]$GDPnom
    Cdata[[ k ]][ "TaxGDP" ] <- Cdata[[ k ]]$Tax / Pdata[[ k ]]$GDPnom
    Pdata[[ k ]][ "GGDP" ] <- Pdata[[ k ]]$G / Pdata[[ k ]]$GDPnom
    mdata[[ k ]][ "GGDP" ] <- mdata[[ k ]]$G / Pdata[[ k ]]$GDPnom
    Mdata[[ k ]][ "GGDP" ] <- Mdata[[ k ]]$G / Pdata[[ k ]]$GDPnom
    cdata[[ k ]][ "GGDP" ] <- cdata[[ k ]]$G / Pdata[[ k ]]$GDPnom
    Cdata[[ k ]][ "GGDP" ] <- Cdata[[ k ]]$G / Pdata[[ k ]]$GDPnom
  }

  plot_lists( c( "TaxGDP", "GGDP" ), Pdata, mdata, Mdata, cdata, Cdata,
              leg = legends, mask = TmaskPlot, nMC = nSize, CI = CI,
              col = colors, lty = lTypes, xlab = "Time",
              ylab = "Government tax income and expenditure over GDP",
              tit = "Government income and expenditure",
              subtit = paste( "MC runs =", nSize, "/ MC", mcStat ),
              leg2 = c( "Tax", "Gov. expenditure" ) )


  # ------ Government deficit in GDP terms------

  plot_lists( "DefGDP", Pdata, mdata, Mdata, cdata, Cdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Government deficit over GDP",
              tit = "Government deficit",
              subtit = paste( "MC runs =", nSize, "/ MC", mcStat ) )


  # ------ Government debt in GDP terms ------

  plot_lists( "DebGDP", Pdata, mdata, Mdata, cdata, Cdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Government debt over GDP",
              tit = "Government debt",
              subtit = paste( "MC runs =", nSize, "/ MC", mcStat ) )


  # ------ Total loans in GDP terms ------

  # add the series to dataset
  for( k in 1 : nExp ) {
    Pdata[[ k ]][ "LoansGDP" ] <- Pdata[[ k ]]$Loans / Pdata[[ k ]]$GDPnom
    mdata[[ k ]][ "LoansGDP" ] <- mdata[[ k ]]$Loans / Pdata[[ k ]]$GDPnom
    Mdata[[ k ]][ "LoansGDP" ] <- Mdata[[ k ]]$Loans / Pdata[[ k ]]$GDPnom
    cdata[[ k ]][ "LoansGDP" ] <- cdata[[ k ]]$Loans / Pdata[[ k ]]$GDPnom
    Cdata[[ k ]][ "LoansGDP" ] <- Cdata[[ k ]]$Loans / Pdata[[ k ]]$GDPnom
  }

  plot_lists( "LoansGDP", Pdata, mdata, Mdata, cdata, Cdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,
              lty = lTypes, xlab = "Time",
              ylab = "Total firm debt stock over GDP",
              tit = "Firm loans",
              subtit = paste( "MC runs =", nSize, "/ MC", mcStat ) )


  # ------ Unemployment and vacancy rates ------

  plot_lists( c( "U", "V" ), Pdata, mdata, Mdata, cdata, Cdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Unemployment and vacancy rates",
              tit = "Unemployment and vacancy",
              subtit = paste( "MC runs =", nSize, "/ MC", mcStat ),
              leg2 = c( "Unemployment", "Vacancy" ) )


  # ------ Real wages ------

  plot_lists( "wReal", Pdata, mdata, Mdata, cdata, Cdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, log0 = TRUE, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Log real wage",
              tit = "Real wage",
              subtit = paste( "MC runs =", nSize, "/ MC", mcStat ) )


  # ------ Real wages share in GDP terms ------

  # add the series to dataset
  for( k in 1 : nExp ) {
    Pdata[[ k ]][ "WGDP" ] <- ( Pdata[[ k ]]$W1 + Pdata[[ k ]]$W2 ) /
      Pdata[[ k ]]$GDPnom
    mdata[[ k ]][ "WGDP" ] <- ( mdata[[ k ]]$W1 + mdata[[ k ]]$W2 ) /
      Pdata[[ k ]]$GDPnom
    Mdata[[ k ]][ "WGDP" ] <- ( Mdata[[ k ]]$W1 + Mdata[[ k ]]$W2 ) /
      Pdata[[ k ]]$GDPnom
    Sdata[[ k ]][ "WGDP" ] <- sqrt( Sdata[[ k ]]$W1 ^ 2 + Sdata[[ k ]]$W2 ^ 2 ) /
      Pdata[[ k ]]$GDPnom
  }

  plot_lists( "WGDP", Pdata, mdata, Mdata, sdMC = Sdata, leg = legends,
              statMC = mcStat, mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Total real wages on GDP",
              tit = "Wage share",
              subtit = paste( "MC runs =", nSize, "/ MC", mcStat ) )


  # ------ Innovation and imitation in sector 1 ------

  plot_lists( c( "inn", "imi" ), Pdata, mdata, Mdata, cdata, Cdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,
              lty = lTypes, xlab = "Time",
              ylab = "Share of innovating and imitating firms",
              tit = "Innovation and imitation",
              subtit = paste( "Capital-good sector only / MC runs =", nSize,
                              "/ MC", mcStat ),
              leg2 = c( "Innovation", "Imitation" ) )


  # ------ Productivity ------

  plot_lists( c( "A", "A1", "A2" ), Pdata, mdata, Mdata, cdata, Cdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, log = TRUE, col = colors,
              lty = lTypes, xlab = "Time",
              ylab = "Relative average log labor productivity",
              tit = "Productivity",
              subtit = paste( "MC runs =", nSize, "/ MC", mcStat ),
              leg2 = c( "Overall", "Capital-good sector", "Consumption-good sector" ) )


  # ------ Concentration ------

  plot_lists( c( "HH1", "HH2" ), Pdata, mdata, Mdata, cdata, Cdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,
              lty = lTypes, xlab = "Time",
              ylab = "Standardized Herfindahl-Hirschman index",
              tit = "Market concentration",
              subtit = paste( "MC runs =", nSize, "/ MC", mcStat ),
              leg2 = c( "Capital-good sector", "Consumption-good sector" ) )


  # ------ Markup ------

  plot_lists( "mu2avg", Pdata, mdata, Mdata, cdata, Cdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Weighted average mark-up rate",
              tit = "Mark-up",
              subtit = paste( "Consumption-good sector only / MC runs =", nSize,
                              "/ MC", mcStat ) )

}
