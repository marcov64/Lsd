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
                        DCdata, mcStat, nExp, nSize, nTsteps, t0, tScale,
                        TmaskPlot, CI, Ptag, Xtag, legends, colors, lTypes,
                        smoothing ) {

  if( all( Xtag == mcStat ) )
    XtagAll <- mcStat
  else
    XtagAll <- paste( Xtag, collapse = "|" )

  subtitbase <- paste0( "period = ", min( TmaskPlot ), "-", max( TmaskPlot ),
                        " / MC runs = ", nSize, " / MC" )
  subtit <- paste( subtitbase, XtagAll )

  # ------ GDP, consumption and investment cases comparison charts ------

  plot_lists( c( "GDPreal", "Ireal", "Creal" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, t0 = t0, tScale = tScale, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, log0 = TRUE, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Log real values",
              tit = "GDP, investment and consumption", subtit = subtit,
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
              statMC = mcStat, DCdata = DCdata, t0 = t0, tScale = tScale,
              leg = legends, mask = TmaskPlot, nMC = nSize, CI = CI, log0 = TRUE,
              col = colors, lty = lTypes, xlab = "Time", ylab = "Log real GDP",
              tit = "GDP", subtit = subtit,
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
              DCdata = DCdata, t0 = t0, tScale = tScale, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, col = colors, lty = lTypes,
              xlab = "Time",
              ylab = "Government tax income and expenditure over GDP",
              tit = "Government income and expenditure", subtit = subtit,
              leg2 = c( "Tax", "Gov. expenditure", "Bank bail-out" ) )


  # ------ Industrial policy expenditures in GDP terms ------

  # add the series to dataset
  for( k in 1 : nExp ) {
    Xdata[[ k ]][ "TaxCredGDP" ] <- Xdata[[ k ]]$TaxCred / Xdata[[ k ]]$GDPnom
    mdata[[ k ]][ "TaxCredGDP" ] <- mdata[[ k ]]$TaxCred / Xdata[[ k ]]$GDPnom
    Mdata[[ k ]][ "TaxCredGDP" ] <- Mdata[[ k ]]$TaxCred / Xdata[[ k ]]$GDPnom
    cdata[[ k ]][ "TaxCredGDP" ] <- cdata[[ k ]]$TaxCred / Xdata[[ k ]]$GDPnom
    Cdata[[ k ]][ "TaxCredGDP" ] <- Cdata[[ k ]]$TaxCred / Xdata[[ k ]]$GDPnom
    Xdata[[ k ]][ "GrdGDP" ] <- Xdata[[ k ]]$Grd / Xdata[[ k ]]$GDPnom
    mdata[[ k ]][ "GrdGDP" ] <- mdata[[ k ]]$Grd / Xdata[[ k ]]$GDPnom
    Mdata[[ k ]][ "GrdGDP" ] <- Mdata[[ k ]]$Grd / Xdata[[ k ]]$GDPnom
    cdata[[ k ]][ "GrdGDP" ] <- cdata[[ k ]]$Grd / Xdata[[ k ]]$GDPnom
    Cdata[[ k ]][ "GrdGDP" ] <- Cdata[[ k ]]$Grd / Xdata[[ k ]]$GDPnom
    Xdata[[ k ]][ "GsiGDP" ] <- Xdata[[ k ]]$Gsi / Xdata[[ k ]]$GDPnom
    mdata[[ k ]][ "GsiGDP" ] <- mdata[[ k ]]$Gsi / Xdata[[ k ]]$GDPnom
    Mdata[[ k ]][ "GsiGDP" ] <- Mdata[[ k ]]$Gsi / Xdata[[ k ]]$GDPnom
    cdata[[ k ]][ "GsiGDP" ] <- cdata[[ k ]]$Gsi / Xdata[[ k ]]$GDPnom
    Cdata[[ k ]][ "GsiGDP" ] <- Cdata[[ k ]]$Gsi / Xdata[[ k ]]$GDPnom
  }

  plot_lists( c( "TaxCredGDP", "GrdGDP", "GsiGDP" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, t0 = t0, tScale = tScale, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, col = colors, lty = lTypes,
              xlab = "Time",
              ylab = "Government industrial policy expenditure over GDP",
              tit = "Industrial policies", subtit = subtit,
              leg2 = c( "R&D tax credit", "R&D subsidy", "Machine-replacement subsidy" ) )


  # ------ Sectoral industrial policy comparison in GDP terms ------

  # add the series to dataset
  for( k in 1 : nExp ) {
    Xdata[[ k ]][ "Tax1credGDP" ] <- Xdata[[ k ]]$Tax1cred / Xdata[[ k ]]$GDPnom
    mdata[[ k ]][ "Tax1credGDP" ] <- mdata[[ k ]]$Tax1cred / Xdata[[ k ]]$GDPnom
    Mdata[[ k ]][ "Tax1credGDP" ] <- Mdata[[ k ]]$Tax1cred / Xdata[[ k ]]$GDPnom
    cdata[[ k ]][ "Tax1credGDP" ] <- cdata[[ k ]]$Tax1cred / Xdata[[ k ]]$GDPnom
    Cdata[[ k ]][ "Tax1credGDP" ] <- Cdata[[ k ]]$Tax1cred / Xdata[[ k ]]$GDPnom
    Xdata[[ k ]][ "TaxEcredGDP" ] <- Xdata[[ k ]]$TaxEcred / Xdata[[ k ]]$GDPnom
    mdata[[ k ]][ "TaxEcredGDP" ] <- mdata[[ k ]]$TaxEcred / Xdata[[ k ]]$GDPnom
    Mdata[[ k ]][ "TaxEcredGDP" ] <- Mdata[[ k ]]$TaxEcred / Xdata[[ k ]]$GDPnom
    cdata[[ k ]][ "TaxEcredGDP" ] <- cdata[[ k ]]$TaxEcred / Xdata[[ k ]]$GDPnom
    Cdata[[ k ]][ "TaxEcredGDP" ] <- Cdata[[ k ]]$TaxEcred / Xdata[[ k ]]$GDPnom
    Xdata[[ k ]][ "Grd1GDP" ] <- Xdata[[ k ]]$Grd1 / Xdata[[ k ]]$GDPnom
    mdata[[ k ]][ "Grd1GDP" ] <- mdata[[ k ]]$Grd1 / Xdata[[ k ]]$GDPnom
    Mdata[[ k ]][ "Grd1GDP" ] <- Mdata[[ k ]]$Grd1 / Xdata[[ k ]]$GDPnom
    cdata[[ k ]][ "Grd1GDP" ] <- cdata[[ k ]]$Grd1 / Xdata[[ k ]]$GDPnom
    Cdata[[ k ]][ "Grd1GDP" ] <- Cdata[[ k ]]$Grd1 / Xdata[[ k ]]$GDPnom
    Xdata[[ k ]][ "GrdEGDP" ] <- Xdata[[ k ]]$GrdE / Xdata[[ k ]]$GDPnom
    mdata[[ k ]][ "GrdEGDP" ] <- mdata[[ k ]]$GrdE / Xdata[[ k ]]$GDPnom
    Mdata[[ k ]][ "GrdEGDP" ] <- Mdata[[ k ]]$GrdE / Xdata[[ k ]]$GDPnom
    cdata[[ k ]][ "GrdEGDP" ] <- cdata[[ k ]]$GrdE / Xdata[[ k ]]$GDPnom
    Cdata[[ k ]][ "GrdEGDP" ] <- Cdata[[ k ]]$GrdE / Xdata[[ k ]]$GDPnom
  }

  plot_lists( c( "Tax1credGDP", "TaxEcredGDP", "Grd1GDP", "GrdEGDP" ),
              Xdata, mdata, Mdata, cdata, Cdata, DCdata = DCdata, t0 = t0,
              tScale = tScale, leg = legends, mask = TmaskPlot, nMC = nSize,
              CI = CI, col = colors, lty = lTypes, xlab = "Time",
              ylab = "Government industrial policy expenditure over GDP",
              tit = "Sectoral industrial policy comparison", subtit = subtit,
              leg2 = c( "R&D tax credit capital sector", "R&D tax credit energy sector",
                        "R&D subsidy capital sector", "R&D subsidy energy sector" ) )


  # ------ Government deficit in GDP terms------

  plot_lists( c( "DefGDP", "DefPgdp" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, t0 = t0, tScale = tScale, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,
              lty = lTypes, xlab = "Time",
              ylab = "Government deficit over GDP",
              tit = "Government deficit", subtit = subtit,
              leg2 = c( "Total", "Primary" ) )


  # ------ Government debt in GDP terms ------

  plot_lists( "DebGDP", Pdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, t0 = t0, tScale = tScale, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,
              lty = lTypes, xlab = "Time",
              ylab = "Government debt over GDP",
              tit = "Government debt",
              subtit = paste( subtitbase, Ptag[ "DebGDP" ] ) )


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
              DCdata = DCdata, t0 = t0, tScale = tScale, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,
              lty = lTypes, xlab = "Time",
              ylab = "Total bank credit available and firm debt stock over GDP",
              tit = "Bank credit supply and firm loans", subtit = subtit,
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

  plot_lists( c( "CDGDP", "CSGDP" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, t0 = t0, tScale = tScale, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,
              lty = lTypes, xlab = "Time",
              ylab = "Effective total firm credit demand and bank credit supply over GDP",
              tit = "Credit demand and supply flow on GDP", subtit = subtit,
              leg2 = c( "Demand", "Supply" ) )


  # ------ Unemployment and vacancy rates ------

  plot_lists( c( "U", "V" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, t0 = t0, tScale = tScale, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,
              lty = lTypes, xlab = "Time",
              ylab = "Unemployment and vacancy rates",
              tit = "Unemployment and vacancy", subtit = subtit,
              leg2 = c( "Unemployment", "Vacancy" ) )


  # ------ Real wages ------

  plot_lists( "wReal", Pdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, t0 = t0, tScale = tScale, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, log0 = TRUE, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Log real wage",
              tit = "Real wage", subtit = paste( subtitbase, Ptag[ "wReal" ] ) )


  # ------ Real wages share in GDP terms ------

  # add the series to dataset
  for( k in 1 : nExp ) {
    Xdata[[ k ]][ "WGDP" ] <- ( Xdata[[ k ]]$We + Xdata[[ k ]]$W1 + Xdata[[ k ]]$W2 ) /
      Xdata[[ k ]]$GDPnom
    mdata[[ k ]][ "WGDP" ] <- ( mdata[[ k ]]$We + mdata[[ k ]]$W1 + mdata[[ k ]]$W2 ) /
      Xdata[[ k ]]$GDPnom
    Mdata[[ k ]][ "WGDP" ] <- ( Mdata[[ k ]]$We + Mdata[[ k ]]$W1 + Mdata[[ k ]]$W2 ) /
      Xdata[[ k ]]$GDPnom
    Sdata[[ k ]][ "WGDP" ] <- sqrt( Sdata[[ k ]]$We ^ 2 + Sdata[[ k ]]$W1 ^ 2 +
                                      Sdata[[ k ]]$W2 ^ 2 ) / Xdata[[ k ]]$GDPnom
  }

  plot_lists( "WGDP", Xdata, mdata, Mdata, sdMC = Sdata, DCdata = DCdata,
              t0 = t0, tScale = tScale, leg = legends, statMC = mcStat,
              mask = TmaskPlot, nMC = nSize,CI = CI, col = colors, lty = lTypes,
              xlab = "Time", ylab = "Total real wages on GDP",
              tit = "Wage share", subtit = subtit )


  # ------  Firms affected by industrial policy ------

  # add the series to dataset
  for( k in 1 : nExp ) {
    Xdata[[ k ]][ "fRD1sub" ] <- Xdata[[ k ]]$nRD1sub / Xdata[[ k ]]$F1
    mdata[[ k ]][ "fRD1sub" ] <- mdata[[ k ]]$nRD1sub / Xdata[[ k ]]$F1
    Mdata[[ k ]][ "fRD1sub" ] <- Mdata[[ k ]]$nRD1sub / Xdata[[ k ]]$F1
    cdata[[ k ]][ "fRD1sub" ] <- cdata[[ k ]]$nRD1sub / Xdata[[ k ]]$F1
    Cdata[[ k ]][ "fRD1sub" ] <- Cdata[[ k ]]$nRD1sub / Xdata[[ k ]]$F1
    Xdata[[ k ]][ "fRDeSub" ] <- Xdata[[ k ]]$nRDeSub / Xdata[[ k ]]$Fe
    mdata[[ k ]][ "fRDeSub" ] <- mdata[[ k ]]$nRDeSub / Xdata[[ k ]]$Fe
    Mdata[[ k ]][ "fRDeSub" ] <- Mdata[[ k ]]$nRDeSub / Xdata[[ k ]]$Fe
    cdata[[ k ]][ "fRDeSub" ] <- cdata[[ k ]]$nRDeSub / Xdata[[ k ]]$Fe
    Cdata[[ k ]][ "fRDeSub" ] <- Cdata[[ k ]]$nRDeSub / Xdata[[ k ]]$Fe
    Xdata[[ k ]][ "fSI2sub" ] <- Xdata[[ k ]]$nSI2sub / Xdata[[ k ]]$F2
    mdata[[ k ]][ "fSI2sub" ] <- mdata[[ k ]]$nSI2sub / Xdata[[ k ]]$F2
    Mdata[[ k ]][ "fSI2sub" ] <- Mdata[[ k ]]$nSI2sub / Xdata[[ k ]]$F2
    cdata[[ k ]][ "fSI2sub" ] <- cdata[[ k ]]$nSI2sub / Xdata[[ k ]]$F2
    Cdata[[ k ]][ "fSI2sub" ] <- Cdata[[ k ]]$nSI2sub / Xdata[[ k ]]$F2
    Xdata[[ k ]][ "fStd1ban" ] <- Xdata[[ k ]]$nStd1ban / Xdata[[ k ]]$F1
    mdata[[ k ]][ "fStd1ban" ] <- mdata[[ k ]]$nStd1ban / Xdata[[ k ]]$F1
    Mdata[[ k ]][ "fStd1ban" ] <- Mdata[[ k ]]$nStd1ban / Xdata[[ k ]]$F1
    cdata[[ k ]][ "fStd1ban" ] <- cdata[[ k ]]$nStd1ban / Xdata[[ k ]]$F1
    Cdata[[ k ]][ "fStd1ban" ] <- Cdata[[ k ]]$nStd1ban / Xdata[[ k ]]$F1
  }

  plot_lists( c( "fRD1sub", "fRDeSub", "fSI2sub", "fStd1ban" ),
              Xdata, mdata, Mdata, cdata, Cdata, DCdata = DCdata, t0 = t0,
              tScale = tScale, leg = legends, mask = TmaskPlot, nMC = nSize,
              CI = CI, col = colors, lty = lTypes, xlab = "Time",
              ylab = "Number of affected firms over total firms in sector",
              tit = "Firms affected by industrial policy", subtit = subtit,
              leg2 = c( "R&D subsidy in capital sector",
                        "R&D subsidy in energy sector",
                        "replacement subsidy in consumption sector",
                        "minimum standard in capital sector" ) )


  # ------  R&D expenses over GDP ------

  # add the series to dataset
  for( k in 1 : nExp ) {
    Xdata[[ k ]][ "RD1GDP" ] <- Xdata[[ k ]]$RD1 / Xdata[[ k ]]$GDPnom
    mdata[[ k ]][ "RD1GDP" ] <- mdata[[ k ]]$RD1 / Xdata[[ k ]]$GDPnom
    Mdata[[ k ]][ "RD1GDP" ] <- Mdata[[ k ]]$RD1 / Xdata[[ k ]]$GDPnom
    cdata[[ k ]][ "RD1GDP" ] <- cdata[[ k ]]$RD1 / Xdata[[ k ]]$GDPnom
    Cdata[[ k ]][ "RD1GDP" ] <- Cdata[[ k ]]$RD1 / Xdata[[ k ]]$GDPnom
    Xdata[[ k ]][ "RDeGDP" ] <- Xdata[[ k ]]$RDe / Xdata[[ k ]]$GDPnom
    mdata[[ k ]][ "RDeGDP" ] <- mdata[[ k ]]$RDe / Xdata[[ k ]]$GDPnom
    Mdata[[ k ]][ "RDeGDP" ] <- Mdata[[ k ]]$RDe / Xdata[[ k ]]$GDPnom
    cdata[[ k ]][ "RDeGDP" ] <- cdata[[ k ]]$RDe / Xdata[[ k ]]$GDPnom
    Cdata[[ k ]][ "RDeGDP" ] <- Cdata[[ k ]]$RDe / Xdata[[ k ]]$GDPnom
  }

  plot_lists( c( "RD1GDP", "RDeGDP" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, t0 = t0, tScale = tScale, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,
              lty = lTypes, xlab = "Time",
              ylab = "Research and development expenditure over GDP",
              tit = "R&D expediture", subtit = subtit,
              leg2 = c( "Capital-good sector", "Energy sector" ) )


  # ------ Innovation in energy sector ------

  plot_lists( c( "innDE", "innGE" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, t0 = t0, tScale = tScale, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Share of innovating firms",
              tit = "Energy innovation", subtit = subtit,
              leg2 = c( "Dirty energy", "Green energy" ) )


  # ------ Innovation and imitation in sector 1 ------

  plot_lists( c( "inn", "imi" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, t0 = t0, tScale = tScale, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,
              lty = lTypes, xlab = "Time",
              ylab = "Share of innovating and imitating firms",
              tit = "Machine innovation and imitation",
              subtit = paste( "Capital-good sector only /", subtit ),
              leg2 = c( "Innovation", "Imitation" ) )


  # ------ Power plant scrapping ------

  plot_lists( c( "RSde", "RSge" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, t0 = t0, tScale = tScale, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,
              lty = lTypes, xlab = "Time",
              ylab = "Number of power plants scrapped",
              tit = "Power plant scrapping", subtit = subtit,
              leg2 = c( "Dirty-energy plants", "Green-energy plants" ) )


  # ------ Levelized cost of energy in new plants ------

  plot_lists( c( "LCOEde", "LCOEge" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, t0 = t0, tScale = tScale, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, log = TRUE, col = colors,
              lty = lTypes, xlab = "Time",
              ylab = "Weighted-average levelized cost of energy of new power plants",
              tit = "New power plant LCOE", subtit = subtit,
              leg2 = c( "Dirty-energy plants", "Green-energy plants" ) )


  # ------ Productivity in energy sector ------

  plot_lists( "Ade", Pdata, mdata, Mdata, cdata, Cdata, DCdata = DCdata, t0 = t0,
              tScale = tScale, leg = legends, mask = TmaskPlot, nMC = nSize,
              CI = CI, na0 = TRUE, col = colors,lty = lTypes, xlab = "Time",
              ylab = "Thermal efficiency of energy generation",
              tit = "Thermal efficiency",
              subtit = paste( subtitbase, Ptag[ "Ade" ] ) )


  # ------ Productivities in sectors 1 and 2 ------

  plot_lists( c( "A", "Aee", "Aef" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, t0 = t0, tScale = tScale, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, log = TRUE, col = colors,
              lty = lTypes, xlab = "Time",
              ylab = "Relative average log productivity, efficiency and friendliness",
              tit = "Productivity and efficiency", subtit = subtit,
              leg2 = c( "Labor productivity", "Energy efficiency",
                        "Environmental friendliness" ) )


  # ------ Concentration ------

  plot_lists( c( "HHe", "HH1", "HH2" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, t0 = t0, tScale = tScale, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,lty = lTypes,
              xlab = "Time",
              ylab = "Standardized Herfindahl-Hirschman index",
              tit = "Market concentration", subtit = subtit,
              leg2 = c( "Energy sector", "Capital-good sector",
                        "Consumption-good sector" ) )


  # ------ Markup in energy sector ------

  plot_lists( "muEavg", Pdata, mdata, Mdata, cdata, Cdata, DCdata = DCdata,
              t0 = t0, tScale = tScale, leg = legends, mask = TmaskPlot,
              nMC = nSize, CI = CI, log = TRUE, col = colors, lty = lTypes,
              xlab = "Time",
              ylab = "Weighted average mark-up amount",
              tit = "Mark-up of energy",
              subtit = paste( subtitbase, Ptag[ "muEavg" ] ) )


  # ------ Markup in sector 2 ------

  plot_lists( "mu2avg", Pdata, mdata, Mdata, cdata, Cdata, DCdata = DCdata,
              t0 = t0, tScale = tScale, leg = legends, mask = TmaskPlot,
              nMC = nSize, CI = CI, col = colors, lty = lTypes, xlab = "Time",
              ylab = "Weighted average mark-up rate",
              tit = "Mark-up of consumer goods",
              subtit = paste( "Consumption-good sector only /", subtitbase,
                              Ptag[ "mu2avg" ] ) )


  # ------ Firms net entry trend in the market ------

  # add the series to dataset
  for( k in 1 : nExp ) {
    # calculate net entry rate trends per MC sample
    nEntTe <- nEntT1 <- nEntT2 <- matrix( nrow = 0, ncol = nTsteps )
    for( j in 1 : nSize ) {   # for each MC case
      nEntTe <- rbind( nEntTe, hpfilter( mcData[[ k ]][ , "entryEexit", j ],
                                         smoothing ) $ trend[ , 1 ] )
      nEntT1 <- rbind( nEntT1, hpfilter( mcData[[ k ]][ , "entry1exit", j ],
                                         smoothing ) $ trend[ , 1 ] )
      nEntT2 <- rbind( nEntT2, hpfilter( mcData[[ k ]][ , "entry2exit", j ],
                                         smoothing ) $ trend[ , 1 ] )
    }

    Xdata[[ k ]][ "nEntTe" ] <- colMeans( nEntTe, na.rm = TRUE )
    mdata[[ k ]][ "nEntTe" ] <- colMins( nEntTe, na.rm = TRUE )
    Mdata[[ k ]][ "nEntTe" ] <- colMaxs( nEntTe, na.rm = TRUE )
    Sdata[[ k ]][ "nEntTe" ] <- colSds( nEntTe, na.rm = TRUE )
    Xdata[[ k ]][ "nEntT1" ] <- colMeans( nEntT1, na.rm = TRUE )
    mdata[[ k ]][ "nEntT1" ] <- colMins( nEntT1, na.rm = TRUE )
    Mdata[[ k ]][ "nEntT1" ] <- colMaxs( nEntT1, na.rm = TRUE )
    Sdata[[ k ]][ "nEntT1" ] <- colSds( nEntT1, na.rm = TRUE )
    Xdata[[ k ]][ "nEntT2" ] <- colMeans( nEntT2, na.rm = TRUE )
    mdata[[ k ]][ "nEntT2" ] <- colMins( nEntT2, na.rm = TRUE )
    Mdata[[ k ]][ "nEntT2" ] <- colMaxs( nEntT2, na.rm = TRUE )
    Sdata[[ k ]][ "nEntT2" ] <- colSds( nEntT2, na.rm = TRUE )
  }

  plot_lists( c( "nEntTe", "nEntT1", "nEntT2" ), Xdata, mdata, Mdata, sdMC = Sdata,
              DCdata = DCdata, t0 = t0, tScale = tScale, leg = legends,
              statMC = mcStat, mask = TmaskPlot, nMC = nSize, CI = CI,
              col = colors, lty = lTypes, xlab = "Time",
              ylab = "Number of net entrant firms (HP-filtered)",
              tit = "Net entry of firms trend", subtit = subtit,
              leg2 = c( "Energy sector", "Capital-good sector",
                        "Consumption-good sector" ) )


  # ------ Firm exit from market ------

  plot_lists( c( "exitE", "exit1", "exit2" ), Xdata, mdata, Mdata, sdMC = Sdata,
              DCdata = DCdata, t0 = t0, tScale = tScale, leg = legends,
              statMC = mcStat, mask = TmaskPlot, nMC = nSize, CI = CI,
              col = colors, lty = lTypes, xlab = "Time",
              ylab = "Number of exiting firms over total",
              tit = "Firm exit", subtit = subtit,
              leg2 = c( "Energy sector", "Capital-good sector",
                        "Consumption-good sector" ) )


  # ------ Firm bankruptcy ------

  plot_lists( c( "exitEfail", "exit1fail", "exit2fail" ), Xdata, mdata, Mdata,
              sdMC = Sdata, DCdata = DCdata, t0 = t0, tScale = tScale,
              leg = legends, statMC = mcStat, mask = TmaskPlot, nMC = nSize,
              CI = CI, col = colors, lty = lTypes, xlab = "Time",
              ylab = "Number of bankrupt exiting firms over total",
              tit = "Firm bankruptcy", subtit = subtit,
              leg2 = c( "Energy sector", "Capital-good sector",
                        "Consumption-good sector" ) )


  # ------ Firm and machine vintage age ------

  plot_lists( c( "ageEavg", "age1avg", "age2avg", "ageVint2avg" ),
              Xdata, mdata, Mdata, cdata, Cdata, DCdata = DCdata, t0 = t0,
              tScale = tScale, log0 = TRUE, leg = legends, mask = TmaskPlot,
              nMC = nSize, CI = CI, col = colors,lty = lTypes, xlab = "Time",
              ylab = "Log age in periods of firms and machine vintages",
              tit = "Firm and machine vintage age", subtit = subtit,
              leg2 = c( "Energy firm", "Capital-good firm",
                        "Consumption-good firm",
                        "Consumption-good machine" ) )


  # ------ Energy price ------

  plot_lists( "pE", Pdata, mdata, Mdata, cdata, Cdata, DCdata = DCdata, t0 = t0,
              tScale = tScale, leg = legends, mask = TmaskPlot, nMC = nSize,
              CI = CI, col = colors, lty = lTypes, xlab = "Time",
              ylab = "Price of electrical energy",
              tit = "Energy price",
              subtit = paste( subtitbase, Ptag[ "pE" ] ) )


  # ------ Energy demand ------

  plot_lists( "EnGDP", Pdata, mdata, Mdata, cdata, Cdata, DCdata = DCdata,
              t0 = t0, tScale = tScale, leg = legends, mask = TmaskPlot,
              nMC = nSize, CI = CI, col = colors, lty = lTypes, xlab = "Time",
              ylab = "Energy demand over GDP",
              tit = "Energy demand",
              subtit = paste( subtitbase, Ptag[ "EnGDP" ] ) )


  # ------ Emissions ------

  # add the series to dataset
  for( k in 1 : nExp ) {
    Xdata[[ k ]][ "Em12" ] <- Xdata[[ k ]]$Em1 + Xdata[[ k ]]$Em2
    mdata[[ k ]][ "Em12" ] <- mdata[[ k ]]$Em1 + mdata[[ k ]]$Em2
    Mdata[[ k ]][ "Em12" ] <- Mdata[[ k ]]$Em1 + Mdata[[ k ]]$Em2
    Sdata[[ k ]][ "Em12" ] <- sqrt( Sdata[[ k ]]$Em1^2 + Sdata[[ k ]]$Em2^2 )
  }

  plot_lists( c( "Em", "EmE", "Em12" ), Xdata, mdata, Mdata, sdMC = Sdata,
              DCdata = DCdata, t0 = t0, tScale = tScale, leg = legends,
              statMC = mcStat, mask = TmaskPlot, nMC = nSize, CI = CI,
              col = colors, lty = lTypes, xlab = "Time",
              ylab = "CO2 emissions in kton",
              tit = "CO2 emissions", subtit = subtit,
              leg2 = c( "Total", "Energy generation", "Industry" ) )


  # ------ CO2 in atmosphere ------

  plot_lists( "CO2a", Pdata, mdata, Mdata, cdata, Cdata, DCdata = DCdata,
              t0 = t0, tScale = tScale, leg = legends, mask = TmaskPlot,
              nMC = nSize, CI = CI, col = colors, lty = lTypes, xlab = "Time",
              ylab = "CO2 atmospheric concentration in PPM",
              tit = "CO2 in atmosphere",
              subtit = paste( subtitbase, Ptag[ "CO2a" ] ) )


  # ------ Temperature anomaly ------

  plot_lists( "Tm", Pdata, mdata, Mdata, cdata, Cdata, DCdata = DCdata, t0 = t0,
              tScale = tScale, leg = legends, mask = TmaskPlot, nMC = nSize,
              CI = CI, col = colors, lty = lTypes, xlab = "Time",
              ylab = "Temperature change in C from preindustrial reference",
              tit = "Temperature anomaly",
              subtit = paste( "2000-2100 reference period /", subtitbase,
                              Ptag[ "Tm" ] ) )


  # ------ Climate shocks ------

  plot_lists( "shockAavg", Pdata, mdata, Mdata, cdata, Cdata, DCdata = DCdata,
              t0 = t0, tScale = tScale, leg = legends, mask = TmaskPlot,
              nMC = nSize, CI = CI, col = colors, lty = lTypes, xlab = "Time",
              ylab = "Expected climate shock size",
              tit = "Climate shocks (disaster generating function)",
              subtit = paste( "2000-2100 reference period /", subtitbase,
                              Ptag[ "shockAavg" ] ) )


  # ------ Share of green energy ------

  plot_lists( c( "fGE", "fKge" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, t0 = t0, tScale = tScale, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, col = colors,
              lty = lTypes, xlab = "Time",
              ylab = "Share of green plants on total generation and installed capacity",
              tit = "Green energy share", subtit = subtit,
              leg2 = c( "Generation share", "Capacity share" ) )

}
