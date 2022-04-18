#******************************************************************
#
# ---------------- K+S aggregates time plots --------------------
#
#   Written by Marcelo C. Pereira, University of Campinas
#
#   Copyright Marcelo C. Pereira
#   Distributed under the GNU General Public License
#
#   Script used by KS-aggregates.R
#   This script should not be executed directly.
#
#******************************************************************

# remove warnings for support functions
# !diagnostics suppress = log0, plot_lists, hpfilter, colSds, colMins, colMaxs

time_plots <- function( mcData, Pdata, Xdata, mdata, Mdata, Sdata, cdata, Cdata,
                        mcStat, nExp, nSize, nTsteps, TmaskPlot, CI, Ptag, Xtag,
                        legends, colors, lTypes, smoothing ) {
                        
  if( all( Xtag == mcStat ) )
    XtagAll <- mcStat
  else
    XtagAll <- paste( Xtag, collapse = "|" )

  # ------ GDP, consumption and investment cases comparison charts ------

  plot_lists( c( "GDPreal", "Ireal", "Creal" ), Xdata, mdata, Mdata, cdata, Cdata,
              leg = legends, mask = TmaskPlot, nMC = nSize, CI = CI, log0 = TRUE,
              col = colors, lty = lTypes,
              xlab = "Time", ylab = "Log real values",
              tit = "GDP, investment and consumption",
              subtit = paste( "MC runs =", nSize, "/ MC", XtagAll ),
              leg2 = c( "GDP", "Investment", "Consumption" ) )


  # ------ GDP Graphs ------

  # add the GDP @ 100% utilization series to dataset
  for( k in 1 : nExp ) {
    Xdata[[ k ]][ "GDP100" ] <- Xdata[[ k ]]$GDPreal / Xdata[[ k ]]$Q2u
    mdata[[ k ]][ "GDP100" ] <- mdata[[ k ]]$GDPreal / Xdata[[ k ]]$Q2u
    Mdata[[ k ]][ "GDP100" ] <- Mdata[[ k ]]$GDPreal / Xdata[[ k ]]$Q2u
    Sdata[[ k ]][ "GDP100" ] <- sqrt( Sdata[[ k ]]$GDPreal ^ 2 + Sdata[[ k ]]$Q2u ^ 2 )
  }

  plot_lists( c( "GDPreal", "GDP100" ), Xdata, mdata, Mdata, sdMC = Sdata,
              statMC = mcStat, leg = legends, mask = TmaskPlot, nMC = nSize,
              CI = CI, log0 = TRUE, col = colors, lty = lTypes,
              xlab = "Time", ylab = "Log real GDP",
              tit = "GDP", subtit = paste( "MC runs =", nSize, "/ MC", XtagAll ),
              leg2 = c( "Effective GDP", "GDP @ 100% utilization" ) )


  # ------ Tax & government expenditures in GDP terms ------

  # add the series to dataset
  for( k in 1 : nExp ) {
    Xdata[[ k ]][ "TaxGDP" ] <- Xdata[[ k ]]$Tax / Xdata[[ k ]]$GDPnom
    mdata[[ k ]][ "TaxGDP" ] <- mdata[[ k ]]$Tax / Xdata[[ k ]]$GDPnom
    Mdata[[ k ]][ "TaxGDP" ] <- Mdata[[ k ]]$Tax / Xdata[[ k ]]$GDPnom
    cdata[[ k ]][ "TaxGDP" ] <- cdata[[ k ]]$Tax / Xdata[[ k ]]$GDPnom
    Cdata[[ k ]][ "TaxGDP" ] <- Cdata[[ k ]]$Tax / Xdata[[ k ]]$GDPnom
    Xdata[[ k ]][ "GGDP" ] <- Xdata[[ k ]]$G / Xdata[[ k ]]$GDPnom
    mdata[[ k ]][ "GGDP" ] <- mdata[[ k ]]$G / Xdata[[ k ]]$GDPnom
    Mdata[[ k ]][ "GGDP" ] <- Mdata[[ k ]]$G / Xdata[[ k ]]$GDPnom
    cdata[[ k ]][ "GGDP" ] <- cdata[[ k ]]$G / Xdata[[ k ]]$GDPnom
    Cdata[[ k ]][ "GGDP" ] <- Cdata[[ k ]]$G / Xdata[[ k ]]$GDPnom
    Xdata[[ k ]][ "GbailGDP" ] <- Xdata[[ k ]]$Gbail / Xdata[[ k ]]$GDPnom
    mdata[[ k ]][ "GbailGDP" ] <- mdata[[ k ]]$Gbail / Xdata[[ k ]]$GDPnom
    Mdata[[ k ]][ "GbailGDP" ] <- Mdata[[ k ]]$Gbail / Xdata[[ k ]]$GDPnom
    cdata[[ k ]][ "GbailGDP" ] <- cdata[[ k ]]$Gbail / Xdata[[ k ]]$GDPnom
    Cdata[[ k ]][ "GbailGDP" ] <- Cdata[[ k ]]$Gbail / Xdata[[ k ]]$GDPnom
  }

  plot_lists( c( "TaxGDP", "GGDP", "GbailGDP" ), Xdata, mdata, Mdata, cdata, Cdata,
              leg = legends, mask = TmaskPlot, nMC = nSize, CI = CI,
              col = colors, lty = lTypes, xlab = "Time",
              ylab = "Government tax income and expenditure over GDP",
              tit = "Government income and expenditure",
              subtit = paste( "MC runs =", nSize, "/ MC", XtagAll ),
              leg2 = c( "Tax", "Gov. expenditure", "Bank bail-out" ) )


  # ------ Government deficit in GDP terms------

  plot_lists( c( "DefGDP", "DefPgdp" ), Xdata, mdata, Mdata, cdata, Cdata,
              leg = legends, mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Government deficit over GDP",
              tit = "Government deficit",
              subtit = paste( "MC runs =", nSize, "/ MC", XtagAll ),
              leg2 = c( "Total", "Primary" ) )


  # ------ Government debt in GDP terms ------

  plot_lists( "DebGDP", Pdata, mdata, Mdata, cdata, Cdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Government debt over GDP",
              tit = "Government debt",
              subtit = paste( "MC runs =", nSize, "/ MC", Ptag[ "DebGDP" ] ) )


  # ------ Total credit supply and loans in GDP terms ------

  # add the series to dataset
  for( k in 1 : nExp ) {
    Xdata[[ k ]][ "TCGDP" ] <- Xdata[[ k ]]$TC / Xdata[[ k ]]$GDPnom
    mdata[[ k ]][ "TCGDP" ] <- mdata[[ k ]]$TC / Xdata[[ k ]]$GDPnom
    Mdata[[ k ]][ "TCGDP" ] <- Mdata[[ k ]]$TC / Xdata[[ k ]]$GDPnom
    cdata[[ k ]][ "TCGDP" ] <- cdata[[ k ]]$TC / Xdata[[ k ]]$GDPnom
    Cdata[[ k ]][ "TCGDP" ] <- Cdata[[ k ]]$TC / Xdata[[ k ]]$GDPnom
    Xdata[[ k ]][ "LoansGDP" ] <- Xdata[[ k ]]$Loans / Xdata[[ k ]]$GDPnom
    mdata[[ k ]][ "LoansGDP" ] <- mdata[[ k ]]$Loans / Xdata[[ k ]]$GDPnom
    Mdata[[ k ]][ "LoansGDP" ] <- Mdata[[ k ]]$Loans / Xdata[[ k ]]$GDPnom
    cdata[[ k ]][ "LoansGDP" ] <- cdata[[ k ]]$Loans / Xdata[[ k ]]$GDPnom
    Cdata[[ k ]][ "LoansGDP" ] <- Cdata[[ k ]]$Loans / Xdata[[ k ]]$GDPnom
  }

  plot_lists( c( "TCGDP", "LoansGDP" ), Xdata, mdata, Mdata, cdata, Cdata,
              leg = legends, mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,
              lty = lTypes, xlab = "Time",
              ylab = "Total bank credit available and firm debt stock over GDP",
              tit = "Bank credit supply and firm loans",
              subtit = paste( "MC runs =", nSize, "/ MC", XtagAll ),
              leg2 = c( "Credit available", "Loans" ) )


  # ------ Credit demand and supply in GDP terms ------

  # add the series to dataset
  for( k in 1 : nExp ) {
    Xdata[[ k ]][ "CDGDP" ] <- Xdata[[ k ]]$CD / Xdata[[ k ]]$GDPnom
    mdata[[ k ]][ "CDGDP" ] <- mdata[[ k ]]$CD / Xdata[[ k ]]$GDPnom
    Mdata[[ k ]][ "CDGDP" ] <- Mdata[[ k ]]$CD / Xdata[[ k ]]$GDPnom
    cdata[[ k ]][ "CDGDP" ] <- cdata[[ k ]]$CD / Xdata[[ k ]]$GDPnom
    Cdata[[ k ]][ "CDGDP" ] <- Cdata[[ k ]]$CD / Xdata[[ k ]]$GDPnom
    Xdata[[ k ]][ "CSGDP" ] <- Xdata[[ k ]]$CS / Xdata[[ k ]]$GDPnom
    mdata[[ k ]][ "CSGDP" ] <- mdata[[ k ]]$CS / Xdata[[ k ]]$GDPnom
    Mdata[[ k ]][ "CSGDP" ] <- Mdata[[ k ]]$CS / Xdata[[ k ]]$GDPnom
    cdata[[ k ]][ "CSGDP" ] <- cdata[[ k ]]$CS / Xdata[[ k ]]$GDPnom
    Cdata[[ k ]][ "CSGDP" ] <- Cdata[[ k ]]$CS / Xdata[[ k ]]$GDPnom
  }

  plot_lists( c( "CDGDP", "CSGDP" ), Xdata, mdata, Mdata, cdata, Cdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,
              lty = lTypes, xlab = "Time",
              ylab = "Effective total firm credit demand and bank credit supply over GDP",
              tit = "Credit demand and supply flow on GDP",
              subtit = paste( "MC runs =", nSize, "/ MC", XtagAll ),
              leg2 = c( "Demand", "Supply" ) )


  # ------ Unemployment and vacancy rates ------

  plot_lists( c( "U", "V" ), Xdata, mdata, Mdata, cdata, Cdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Unemployment and vacancy rates",
              tit = "Unemployment and vacancy",
              subtit = paste( "MC runs =", nSize, "/ MC", XtagAll ),
              leg2 = c( "Unemployment", "Vacancy" ) )


  # ------ Real wages ------

  plot_lists( "wReal", Pdata, mdata, Mdata, cdata, Cdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, log0 = TRUE, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Log real wage",
              tit = "Real wage",
              subtit = paste( "MC runs =", nSize, "/ MC", Ptag[ "wReal" ] ) )


  # ------ Real wages share in GDP terms ------

  # add the series to dataset
  for( k in 1 : nExp ) {
    Xdata[[ k ]][ "WGDP" ] <- ( Xdata[[ k ]]$W1 + Xdata[[ k ]]$W2 ) /
      Xdata[[ k ]]$GDPnom
    mdata[[ k ]][ "WGDP" ] <- ( mdata[[ k ]]$W1 + mdata[[ k ]]$W2 ) /
      Xdata[[ k ]]$GDPnom
    Mdata[[ k ]][ "WGDP" ] <- ( Mdata[[ k ]]$W1 + Mdata[[ k ]]$W2 ) /
      Xdata[[ k ]]$GDPnom
    Sdata[[ k ]][ "WGDP" ] <- sqrt( Sdata[[ k ]]$W1 ^ 2 + Sdata[[ k ]]$W2 ^ 2 ) /
      Xdata[[ k ]]$GDPnom
  }

  plot_lists( "WGDP", Xdata, mdata, Mdata, sdMC = Sdata, leg = legends,
              statMC = mcStat, mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Total real wages on GDP",
              tit = "Wage share",
              subtit = paste( "MC runs =", nSize, "/ MC", XtagAll ) )


  # ------ Innovation and imitation in sector 1 ------

  plot_lists( c( "inn", "imi" ), Xdata, mdata, Mdata, cdata, Cdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,
              lty = lTypes, xlab = "Time",
              ylab = "Share of innovating and imitating firms",
              tit = "Innovation and imitation",
              subtit = paste( "Capital-good sector only / MC runs =", nSize,
                              "/ MC", XtagAll ),
              leg2 = c( "Innovation", "Imitation" ) )


  # ------ Productivities ------

  plot_lists( c( "A", "Aee", "Aef" ), Xdata, mdata, Mdata, cdata, Cdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, log = TRUE, col = colors,
              lty = lTypes, xlab = "Time",
              ylab = "Relative average log productivity, efficiency and friendliness",
              tit = "Productivity and efficiency",
              subtit = paste( "MC runs =", nSize, "/ MC", XtagAll ),
              leg2 = c( "Labor productivity", "Energy efficiency",
                        "Environmental friendliness" ) )


  # ------ Concentration ------

  plot_lists( c( "HH1", "HH2" ), Xdata, mdata, Mdata, cdata, Cdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,
              lty = lTypes, xlab = "Time",
              ylab = "Standardized Herfindahl-Hirschman index",
              tit = "Market concentration",
              subtit = paste( "MC runs =", nSize, "/ MC", XtagAll ),
              leg2 = c( "Capital-good sector", "Consumption-good sector" ) )


  # ------ Markup ------

  plot_lists( "mu2avg", Pdata, mdata, Mdata, cdata, Cdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Weighted average mark-up rate",
              tit = "Mark-up",
              subtit = paste( "Consumption-good sector only / MC runs =", nSize,
                              "/ MC", Ptag[ "mu2avg" ] ) )


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

    Xdata[[ k ]][ "nEntT1" ] <- colMeans( nEntT1, na.rm = TRUE )
    mdata[[ k ]][ "nEntT1" ] <- colMins( nEntT1, na.rm = TRUE )
    Mdata[[ k ]][ "nEntT1" ] <- colMaxs( nEntT1, na.rm = TRUE )
    Sdata[[ k ]][ "nEntT1" ] <- colSds( nEntT1, na.rm = TRUE )
    Xdata[[ k ]][ "nEntT2" ] <- colMeans( nEntT2, na.rm = TRUE )
    mdata[[ k ]][ "nEntT2" ] <- colMins( nEntT2, na.rm = TRUE )
    Mdata[[ k ]][ "nEntT2" ] <- colMaxs( nEntT2, na.rm = TRUE )
    Sdata[[ k ]][ "nEntT2" ] <- colSds( nEntT2, na.rm = TRUE )
  }

  plot_lists( c( "nEntT1", "nEntT2" ), Xdata, mdata, Mdata, sdMC = Sdata,
  			  leg = legends, statMC = mcStat, mask = TmaskPlot, nMC = nSize,
  			  CI = CI, col = colors, lty = lTypes, xlab = "Time",
              ylab = "Number of net entrant firms (HP-filtered)",
              tit = "Net entry of firms trend",
              subtit = paste( "MC runs =", nSize, "/ MC", XtagAll ),
              leg2 = c( "Consumption-good sector", "Capital-good sector" ) )


  # ------ Energy price ------

  plot_lists( "pE", Pdata, mdata, Mdata, cdata, Cdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Price of electrical energy",
              tit = "Energy price",
              subtit = paste( "MC runs =", nSize, "/ MC", Ptag[ "pE" ] ) )


  # ------ Energy demand ------

  plot_lists( "EnGDP", Pdata, mdata, Mdata, cdata, Cdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Energy demand over GDP",
              tit = "Energy demand",
              subtit = paste( "MC runs =", nSize, "/ MC", Ptag[ "EnGDP" ] ) )


  # ------ Emissions ------

  # add the series to dataset
  for( k in 1 : nExp ) {
    Xdata[[ k ]][ "Em12" ] <- Xdata[[ k ]]$Em1 + Xdata[[ k ]]$Em2
    mdata[[ k ]][ "Em12" ] <- mdata[[ k ]]$Em1 + mdata[[ k ]]$Em2
    Mdata[[ k ]][ "Em12" ] <- Mdata[[ k ]]$Em1 + Mdata[[ k ]]$Em2
    Sdata[[ k ]][ "Em12" ] <- sqrt( Sdata[[ k ]]$Em1^2 + Sdata[[ k ]]$Em2^2 )
  }

  plot_lists( c( "Em", "EmE", "Em12" ), Xdata, mdata, Mdata, sdMC = Sdata, leg = legends,
              statMC = mcStat, mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,
              lty = lTypes, xlab = "Time", ylab = "CO2 emissions in kton",
              tit = "CO2 emissions",
              subtit = paste( "MC runs =", nSize, "/ MC", XtagAll ),
              leg2 = c( "Total", "Energy generation", "Industry" ) )


  # ------ CO2 in atmosphere ------

  plot_lists( "CO2a", Pdata, mdata, Mdata, cdata, Cdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,
              lty = lTypes, xlab = "Time",
              ylab = "CO2 atmospheric concentration in PPM",
              tit = "CO2 in atmosphere",
              subtit = paste( "MC runs =", nSize, "/ MC", Ptag[ "CO2a" ] ) )


  # ------ Temperature anomaly ------

  plot_lists( "Tm", Pdata, mdata, Mdata, cdata, Cdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,
              lty = lTypes, xlab = "Time",
              ylab = "Temperature change in C from preindustrial reference",
              tit = "Temperature anomaly",
              subtit = paste( "2000-2100 reference period / MC runs =", nSize,
              				  "/ MC", Ptag[ "Tm" ] ) )


  # ------ Climate shocks ------

  plot_lists( "shockAavg", Pdata, mdata, Mdata, cdata, Cdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,
              lty = lTypes, xlab = "Time",
              ylab = "Expected climate shock size",
              tit = "Climate shocks (disaster generating function)",
              subtit = paste( "2000-2100 reference period / MC runs =", nSize,
              				  "/ MC", Ptag[ "shockAavg" ] ) )


  # ------ Share of green energy ------

  plot_lists( c( "fGE", "fKge" ), Xdata, mdata, Mdata, cdata, Cdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,
              lty = lTypes, xlab = "Time",
              ylab = "Share of green plants on total generation and installed capacity",
              tit = "Green energy share",
              subtit = paste( "MC runs =", nSize, "/ MC", XtagAll ),
              leg2 = c( "Generation share", "Capacity share" ) )

}
