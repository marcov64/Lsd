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
                        mcStat, nExp, nSize, nTsteps, mCnt, TmaskPlot, CI, Ptag,
                        Xtag, legends, cntLeg, colors, lTypes, smoothing,
                        transMk, firmTypes ) {

  if( all( Xtag == mcStat ) )
    XtagAll <- mcStat
  else
    XtagAll <- paste( Xtag, collapse = "|" )

  # ------ Real GDP, consumption and investment ------

  plot_lists( c( "GDPreal", "Ireal", "Creal" ), Xdata, mdata, Mdata, cdata, Cdata,
              leg = legends, mask = TmaskPlot, nMC = nSize, CI = CI, log0 = TRUE,
              col = colors, mrk = transMk, lty = lTypes,
              xlab = "Time", ylab = "Log real (initial prices) aggregate",
              tit = "Real GDP, investment and consumption",
              subtit = paste( "MC runs =", nSize, "/ MC", XtagAll, cntLeg ),
              leg2 = c( "GDP", "Investment", "Consumption" ) )


  # ------ Nominal GDP, consumption and investment ------

  plot_lists( c( "GDPnom", "Inom", "C" ), Xdata, mdata, Mdata, cdata, Cdata,
              leg = legends, mask = TmaskPlot, nMC = nSize, CI = CI, log0 = TRUE,
              col = colors, mrk = transMk, lty = lTypes,
              xlab = "Time", ylab = "Log nominal (current prices) aggregate",
              tit = "Nominal GDP, investment and consumption",
              subtit = paste( "MC runs =", nSize, "/ MC", XtagAll, cntLeg ),
              leg2 = c( "GDP", "Investment", "Consumption" ) )


  # ------ Capacity utilization ------

  for( k in 1 : nExp ) {
    Xdata[[ k ]][ "GDP100" ] <- Xdata[[ k ]]$GDPreal / Xdata[[ k ]]$QcU
    mdata[[ k ]][ "GDP100" ] <- mdata[[ k ]]$GDPreal / Xdata[[ k ]]$QcU
    Mdata[[ k ]][ "GDP100" ] <- Mdata[[ k ]]$GDPreal / Xdata[[ k ]]$QcU
    Sdata[[ k ]][ "GDP100" ] <- sqrt( Sdata[[ k ]]$GDPreal ^ 2 + Sdata[[ k ]]$QcU ^ 2 )
  }

  plot_lists( c( "GDPreal", "GDP100" ), Xdata, mdata, Mdata, sdMC = Sdata,
              statMC = mcStat, leg = legends, mask = TmaskPlot, nMC = nSize,
              CI = CI, log0 = TRUE, col = colors, mrk = transMk, lty = lTypes,
              mCnt = mCnt, xlab = "Time", ylab = "Log real (initial prices) GDP",
              tit = "Capacity utilization",
              subtit = paste( "MC runs =", nSize, "/ MC", XtagAll, cntLeg ),
              leg2 = c( "Effective GDP", "GDP @ 100% utilization" ) )


  # ------ Tax & government expenditures in GDP terms ------

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
    Cdata[[ k ]][ "GedGDP" ] <- Cdata[[ k ]]$Ged / Xdata[[ k ]]$GDPnom
    Xdata[[ k ]][ "GedGDP" ] <- Xdata[[ k ]]$Ged / Xdata[[ k ]]$GDPnom
    mdata[[ k ]][ "GedGDP" ] <- mdata[[ k ]]$Ged / Xdata[[ k ]]$GDPnom
    Mdata[[ k ]][ "GedGDP" ] <- Mdata[[ k ]]$Ged / Xdata[[ k ]]$GDPnom
    cdata[[ k ]][ "GedGDP" ] <- cdata[[ k ]]$Ged / Xdata[[ k ]]$GDPnom
    Cdata[[ k ]][ "GedGDP" ] <- Cdata[[ k ]]$Ged / Xdata[[ k ]]$GDPnom
    Xdata[[ k ]][ "GtrainGDP" ] <- Xdata[[ k ]]$Gtrain / Xdata[[ k ]]$GDPnom
    mdata[[ k ]][ "GtrainGDP" ] <- mdata[[ k ]]$Gtrain / Xdata[[ k ]]$GDPnom
    Mdata[[ k ]][ "GtrainGDP" ] <- Mdata[[ k ]]$Gtrain / Xdata[[ k ]]$GDPnom
    cdata[[ k ]][ "GtrainGDP" ] <- cdata[[ k ]]$Gtrain / Xdata[[ k ]]$GDPnom
    Cdata[[ k ]][ "GtrainGDP" ] <- Cdata[[ k ]]$Gtrain / Xdata[[ k ]]$GDPnom
  }

  plot_lists( c( "TaxGDP", "GGDP", "GbailGDP", "GedGDP", "GtrainGDP" ), Xdata, mdata,
              Mdata, cdata, Cdata, leg = legends, mask = TmaskPlot, nMC = nSize,
              CI = CI, mrk = transMk, col = colors, lty = lTypes, mCnt = mCnt,
              xlab = "Time", ylab = "Government tax income and expenditure over GDP",
              tit = "Government income and expenditure",
              subtit = paste( "MC runs =", nSize, "/ MC", XtagAll, cntLeg ),
              leg2 = c( "Tax", "Gov. expenditure", "Bank bail-out",
                        "Education exp.", "Training exp." ) )


  # ------ Government deficit in GDP terms------

  plot_lists( "DefGDP", Pdata, mdata, Mdata, cdata, Cdata,
              leg = legends, mask = TmaskPlot, nMC = nSize, CI = CI,
              mrk = transMk, col = colors, lty = lTypes, mCnt = mCnt,
              xlab = "Time", ylab = "Government deficit over GDP",
              tit = "Government deficit",
              subtit = paste( "MC runs =", nSize, "/ MC", Ptag[ "DefGDP" ],
                              cntLeg ),
              trend = "loess" )


  # ------ Government debt in GDP terms ------

  plot_lists( "DebGDP", Pdata, mdata, Mdata, cdata, Cdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Government debt over GDP",
              tit = "Government debt",
              subtit = paste( "MC runs =", nSize, "/ MC", Ptag[ "DebGDP" ],
                              cntLeg ),
              trend = "linear" )


  # ------ Total credit supply and loans in GDP terms ------

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
              leg = legends, mask = TmaskPlot, nMC = nSize, CI = CI,
              mrk = transMk, col = colors, lty = lTypes, mCnt = mCnt, xlab = "Time",
              ylab = "Total bank credit available and firm debt stock over GDP",
              tit = "Bank credit supply and firm loans",
              subtit = paste( "MC runs =", nSize, "/ MC", XtagAll, cntLeg ),
              leg2 = c( "Credit available", "Loans" ) )


  # ------ Loans and bad debt in GDP terms ------

  for( k in 1 : nExp ) {
    Xdata[[ k ]][ "BadDebGDP" ] <- Xdata[[ k ]]$BadDeb / Xdata[[ k ]]$GDPnom
    mdata[[ k ]][ "BadDebGDP" ] <- mdata[[ k ]]$BadDeb / Xdata[[ k ]]$GDPnom
    Mdata[[ k ]][ "BadDebGDP" ] <- Mdata[[ k ]]$BadDeb / Xdata[[ k ]]$GDPnom
    cdata[[ k ]][ "BadDebGDP" ] <- cdata[[ k ]]$BadDeb / Xdata[[ k ]]$GDPnom
    Cdata[[ k ]][ "BadDebGDP" ] <- Cdata[[ k ]]$BadDeb / Xdata[[ k ]]$GDPnom
  }

  plot_lists( c( "LoansGDP", "BadDebGDP" ), Xdata, mdata, Mdata, cdata, Cdata,
              leg = legends, mask = TmaskPlot, nMC = nSize, CI = CI,
              mrk = transMk, col = colors, lty = lTypes, mCnt = mCnt, xlab = "Time",
              ylab = "Firm debt stock and bad-debt on period over GDP",
              tit = "Firm loans and bank bad debt",
              subtit = paste( "MC runs =", nSize, "/ MC", XtagAll, cntLeg ),
              leg2 = c( "Loans", "Bad debt" ) )


  # ------ Bank fragility ------

  plot_lists( "Bda", Pdata, mdata, Mdata, cdata, Cdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, mrk = transMk, col = colors,
              lty = lTypes, mCnt = mCnt, xlab = "Time",
              ylab = "Accumulated banks bad debt over assets",
              tit = "Bank fragility",
              subtit = paste( "MC runs =", nSize, "/ MC", Ptag[ "Bda" ], cntLeg ),
              trend = "loess" )


  # ------ Capacity utilization and labor market participation rates ------

  plot_lists( c( "QcU", "Lpart" ), Xdata, mdata, Mdata, cdata, Cdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, mrk = transMk, col = colors,
              lty = lTypes, mCnt = mCnt, xlab = "Time",
              ylab = "Rates of capital utilization and labor participation",
              tit = "Capacity utilization and labor participation",
              subtit = paste( "MC runs =", nSize, "/ MC", XtagAll, cntLeg ),
              leg2 = c( "Capacity utilization", "Labor participation" ),
              trend = "loess" )


  # ------ Unemployment and vacancy rates ------

  plot_lists( c( "U", "V" ), Xdata, mdata, Mdata, cdata, Cdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, mrk = transMk, col = colors,
              lty = lTypes, mCnt = mCnt, xlab = "Time",
              ylab = "Overall unemployment and vacancy rates",
              tit = "Unemployment and vacancy",
              subtit = paste( "MC runs =", nSize, "/ MC", XtagAll, cntLeg ),
              leg2 = c( "Unemployment", "Vacancy" ) )


  # ------ Real wages ------

  plot_lists( "wAvgReal", Pdata, mdata, Mdata, cdata, Cdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, log0 = TRUE, mrk = transMk,
              col = colors, lty = lTypes, mCnt = mCnt, xlab = "Time",
              ylab = "Log average real wage", tit = "Real wage",
              subtit = paste( "MC runs =", nSize, "/ MC", Ptag[ "wAvgReal" ],
                              cntLeg ) )


  # ------ Real wages by firm type ------

  plot_lists( c( "wCrealPreChg", "wCrealPosChg" ), Xdata, mdata, Mdata,
              cdata, Cdata, leg = legends, mask = TmaskPlot, nMC = nSize,
              CI = CI, log0 = TRUE, mrk = transMk, col = colors, lty = lTypes,
              mCnt = mCnt, xlab = "Time", ylab = "Log average real wage",
              tit = "Wages by firm type",
              subtit = paste( "Consumption-good sector only / MC runs =",
                              nSize, "/ MC", XtagAll, cntLeg ),
              leg2 = firmTypes )


  # ------ Wage and bonus share in GDP terms ------

  for( k in 1 : nExp ) {
    Xdata[[ k ]][ "WGDP" ] <- Xdata[[ k ]]$W / Xdata[[ k ]]$GDPnom
    mdata[[ k ]][ "WGDP" ] <- mdata[[ k ]]$W / Xdata[[ k ]]$GDPnom
    Mdata[[ k ]][ "WGDP" ] <- Mdata[[ k ]]$W / Xdata[[ k ]]$GDPnom
    cdata[[ k ]][ "WGDP" ] <- cdata[[ k ]]$W / Xdata[[ k ]]$GDPnom
    Cdata[[ k ]][ "WGDP" ] <- Cdata[[ k ]]$W / Xdata[[ k ]]$GDPnom
    Xdata[[ k ]][ "BonCgdp" ] <- Xdata[[ k ]]$BonC / Xdata[[ k ]]$GDPnom
    mdata[[ k ]][ "BonCgdp" ] <- mdata[[ k ]]$BonC / Xdata[[ k ]]$GDPnom
    Mdata[[ k ]][ "BonCgdp" ] <- Mdata[[ k ]]$BonC / Xdata[[ k ]]$GDPnom
    cdata[[ k ]][ "BonCgdp" ] <- cdata[[ k ]]$BonC / Xdata[[ k ]]$GDPnom
    Cdata[[ k ]][ "BonCgdp" ] <- Cdata[[ k ]]$BonC / Xdata[[ k ]]$GDPnom
  }

  plot_lists( c( "WGDP", "BonCgdp" ), Xdata, mdata, Mdata, cdata, Cdata,
              leg = legends, statMC = mcStat, mask = TmaskPlot, nMC = nSize,
              CI = CI, mrk = transMk, col = colors, lty = lTypes, mCnt = mCnt,
              xlab = "Time", ylab = "Total wages and bonus on GDP",
              tit = "Wage share",
              subtit = paste( "MC runs =", nSize, "/ MC", XtagAll, cntLeg ),
              leg2 = c( "Wages", "Bonus" ), trend = "loess" )


  # ------ Wages spread ------

  plot_lists( "wLogSD", Pdata, mdata, Mdata, cdata, Cdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, mrk = transMk,
              col = colors, lty = lTypes, mCnt = mCnt, xlab = "Time",
              ylab = "Standard deviation of log real wage", tit = "Wage spread",
              subtit = paste( "MC runs =", nSize, "/ MC", Ptag[ "wLogSD" ],
                              cntLeg ),
              trend = "loess" )


  # ------ Gini index ------

  plot_lists( c( "Gini", "wGini" ), Xdata, mdata, Mdata, cdata, Cdata,
              leg = legends, mask = TmaskPlot, nMC = nSize, CI = CI,
              mrk = transMk, col = colors, lty = lTypes, mCnt = mCnt,
              xlab = "Time", ylab = "Gini index", tit = "Gini index",
              subtit = paste( "MC runs =", nSize, "/ MC", XtagAll, cntLeg ),
              leg2 = c( "Total income", "Wage income only" ),
              trend = "loess" )


  # ------ Basic-goods demand share of income (Engel's law) ------

  for( k in 1 : nExp ) {
    Xdata[[ k ]][ "DcIn" ] <- Xdata[[ k ]]$DcBas / Xdata[[ k ]]$In
    mdata[[ k ]][ "DcIn" ] <- mdata[[ k ]]$DcBas / Xdata[[ k ]]$In
    Mdata[[ k ]][ "DcIn" ] <- Mdata[[ k ]]$DcBas / Xdata[[ k ]]$In
    cdata[[ k ]][ "DcIn" ] <- cdata[[ k ]]$DcBas / Xdata[[ k ]]$In
    Cdata[[ k ]][ "DcIn" ] <- Cdata[[ k ]]$DcBas / Xdata[[ k ]]$In
  }

  plot_lists( "DcIn", Xdata, mdata, Mdata, cdata, Cdata, leg = legends,
              statMC = mcStat, mask = TmaskPlot, nMC = nSize, CI = CI,
              mrk = transMk, col = colors, lty = lTypes, mCnt = mCnt,
              xlab = "Time",
              ylab = "Share of basic goods consumption on current income",
              tit = "Engel's law",
              subtit = paste( "MC runs =", nSize, "/ MC", XtagAll, cntLeg ),
              trend = "linear" )


  # ------ Consumption and savings for luxury goods ------

  plot_lists( c( "SavLux", "DcLux" ), Xdata, mdata, Mdata, cdata, Cdata,
              leg = legends, mask = TmaskPlot, nMC = nSize, CI = CI,
              log0 = TRUE, mrk = transMk, col = colors, lty = lTypes,
              mCnt = mCnt, xlab = "Time",
              ylab = "Log savings and consumption of luxury goods",
              tit = "Savings and consumption of luxury goods",
              subtit = paste( "MC runs =", nSize, "/ MC", XtagAll, cntLeg ),
              leg2 = c( "Savings", "Consumption" ) )


  # ------ Average tenure and vintage skills ------

  plot_lists( c( "sTavg", "sVavg" ), Xdata, mdata, Mdata, cdata, Cdata,
              leg = legends, mask = TmaskPlot, nMC = nSize, CI = CI,
              mrk = transMk, col = colors, lty = lTypes, mCnt = mCnt,
              xlab = "Time", ylab = "Average worker skills level",
              tit = "Workers' skills",
              subtit = paste( "MC runs =", nSize, "/ MC", XtagAll, cntLeg ),
              leg2 = c( "Tenure skills", "Vintage skills" ) )


  # ------ Technological frontier ------

  plot_lists( c( "g1front", "g1max" ), Xdata, mdata, Mdata, cdata, Cdata,
              leg = legends, mask = TmaskPlot, nMC = nSize, CI = CI,
              mrk = transMk, col = colors, lty = lTypes, mCnt = mCnt,
              xlab = "Time",
              ylab = "Machine-generation sequential identification",
              tit = "Technological frontier",
              subtit = paste( "MC runs =", nSize, "/ MC", XtagAll, cntLeg ),
              leg2 = c( "Technological frontier",
                        "Most advanced in production" ) )


  # ------ Innovation and imitation ------

  plot_lists( c( "inn1i", "inn1r", "imi1" ), Xdata, mdata, Mdata, cdata, Cdata,
              leg = legends, mask = TmaskPlot, nMC = nSize, CI = CI,
              mrk = transMk, col = colors, lty = lTypes, mCnt = mCnt,
              xlab = "Time", ylab = "Share of innovating and imitating firms",
              tit = "Innovation and imitation",
              subtit = paste( "Capital-good sector only / MC runs =", nSize,
                              "/ MC", XtagAll, cntLeg ),
              leg2 = c( "Incremental innovation", "Radical innovation",
                        "Imitation" ), trend = "loess" )


  # ------ Productivity by sector ------

  plot_lists( c( "A", "AcBas", "AcLux" ), Xdata, mdata, Mdata, cdata, Cdata,
              leg = legends, mask = TmaskPlot, nMC = nSize, CI = CI, log = TRUE,
              mrk = transMk, col = colors, lty = lTypes, mCnt = mCnt,
              xlab = "Time", ylab = "Relative average log labor productivity",
              tit = "Productivity by sector",
              subtit = paste( "MC runs =", nSize, "/ MC", XtagAll, cntLeg ),
              leg2 = c( "Overall", "Basic goods", "Luxury goods" ) )


  # ------ Productivity by firm type ------

  plot_lists( c( "AcPreChg", "AcPosChg" ), Xdata, mdata, Mdata, cdata, Cdata,
              leg = legends, mask = TmaskPlot, nMC = nSize, CI = CI, log0 = TRUE,
              mrk = transMk, col = colors, lty = lTypes, mCnt = mCnt, xlab = "Time",
              ylab = "Relative average log labor productivity",
              tit = "Productivity by firm type",
              subtit = paste( "MC runs =", nSize, "/ MC", XtagAll, cntLeg ),
              leg2 = firmTypes )


  # ------ Productivity spread by firm type ------

  plot_lists( c( "AsdCpreChg", "AsdCposChg" ), Xdata, mdata, Mdata, cdata, Cdata,
              leg = legends, mask = TmaskPlot, nMC = nSize, CI = CI,
              mrk = transMk, col = colors, lty = lTypes, mCnt = mCnt, xlab = "Time",
              ylab = "Standard deviation of firm log productivity",
              tit = "Productivity spread",
              subtit = paste( "MC runs =", nSize, "/ MC", XtagAll, cntLeg ),
              leg2 = firmTypes )


  # ------ Product quality by firm type ------

  plot_lists( c( "qCpreChg", "qCposChg" ), Xdata, mdata, Mdata, cdata, Cdata,
              leg = legends, mask = TmaskPlot, nMC = nSize, CI = CI,
              mrk = transMk, col = colors, lty = lTypes, mCnt = mCnt,
              xlab = "Time",
              ylab = "Weighted average consumer-goods reference quality",
              tit = "Product quality",
              subtit = paste( "MC runs =", nSize, "/ MC", XtagAll, cntLeg ),
              leg2 = firmTypes, trend = "loess" )


  # ------ Product complexity ------

  plot_lists( c( "kCavgBas", "kCavgLux" ), Xdata, mdata, Mdata, cdata, Cdata,
              leg = legends, mask = TmaskPlot, nMC = nSize, CI = CI, log0 = TRUE,
              mrk = transMk, col = colors, lty = lTypes, mCnt = mCnt, xlab = "Time",
              ylab = "Log Weighted-average consumer-good complexity",
              tit = "Product complexity",
              subtit = paste( "MC runs =", nSize, "/ MC", XtagAll, cntLeg ),
              leg2 = c( "Basic goods", "Luxury goods" ) )


  # ------ Product newness ------

  plot_lists( c( "nCavgBas", "nCavgLux" ), Xdata, mdata, Mdata, cdata, Cdata,
              leg = legends, mask = TmaskPlot, nMC = nSize, CI = CI,
              mrk = transMk, col = colors, lty = lTypes, mCnt = mCnt,
              xlab = "Time",
              ylab = "Weighted average consumer-goods time in market",
              tit = "Product newness",
              subtit = paste( "MC runs =", nSize, "/ MC", XtagAll, cntLeg ),
              leg2 = c( "Basic goods", "Luxury goods" ) )


  # ------ Market share of post-change firms ------

  plot_lists( "fCposChg", Pdata, mdata, Mdata, cdata, Cdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, mrk = transMk,
              col = colors, lty = lTypes, mCnt = mCnt, xlab = "Time",
              ylab = paste( "Market share of", firmTypes[2] ),
              tit = paste( "Market share of", firmTypes[2] ),
              subtit = paste( "Consumption-good sector only / MC runs =",
                              nSize, "/ MC", Ptag[ "fCposChg" ], cntLeg ) )


  # ------ Industries in the consumption-goods market ------

  plot_lists( c( "FcBas", "FcLux" ), Xdata, mdata, Mdata, cdata, Cdata,
              leg = legends, mask = TmaskPlot, nMC = nSize, CI = CI,
              mrk = transMk, col = colors, lty = lTypes, mCnt = mCnt,
              xlab = "Time", ylab = "Number of industries",
              tit = "Industries operating in the consumption-goods market",
              subtit = paste( "MC runs =", nSize, "/ MC", XtagAll, cntLeg ),
              leg2 = c( "Basic goods", "Luxury goods" ) )


  # ------ Firms in the market ------

  plot_lists( c( "FcFbas", "FcFlux", "F1" ), Xdata, mdata, Mdata, cdata, Cdata,
              leg = legends, mask = TmaskPlot, nMC = nSize, CI = CI,
              mrk = transMk, col = colors, lty = lTypes, mCnt = mCnt,
              xlab = "Time", ylab = "Number of firms",
              tit = "Firms operating in the market",
              subtit = paste( "MC runs =", nSize, "/ MC", XtagAll, cntLeg ),
              leg2 = c( "Basic goods", "Luxury goods", "Capital goods" ) )


  # ------ Firm entry and exit in the consumer market ------

  plot_lists( c( "entryC", "exitC" ), Xdata, mdata, Mdata, cdata, Cdata,
              leg = legends, mask = TmaskPlot, nMC = nSize, CI = CI,
              mrk = transMk, col = colors, lty = lTypes, mCnt = mCnt,
              xlab = "Time",
              ylab = "Share of total firms",
              tit = "Firm entry and exit in the consumption-goods market",
              subtit = paste( "Consumption-good sector only / MC runs =",
                              nSize, "/ MC", XtagAll, cntLeg ),
              leg2 = c( "Entry", "Exit" ), trend = "loess" )


  # ------ Firms net entry trend in the market ------

  for( k in 1 : nExp ) {
    # calculate net entry rate trends per MC sample
    nEntT1 <- nEntTc <- matrix( nrow = 0, ncol = nTsteps )
    for( j in 1 : nSize ) {   # for each MC case
      nEntT1 <- rbind( nEntT1, hpfilter( ( mcData[[ k ]][ , "entry1", j ] -
                                             mcData[[ k ]][ , "exit1", j ] ) *
                                           mcData[[ k ]][ , "F1", j ],
                                         smoothing ) $ trend[ , 1 ] )
      nEntTc <- rbind( nEntTc, hpfilter( ( mcData[[ k ]][ , "entryC", j ] -
                                             mcData[[ k ]][ , "exitC", j ] ) *
                                           mcData[[ k ]][ , "Fc", j ],
                                         smoothing ) $ trend[ , 1 ] )
    }

    Xdata[[ k ]][ "nEntT1" ] <- colMeans( nEntT1, na.rm = TRUE )
    mdata[[ k ]][ "nEntT1" ] <- colMins( nEntT1, na.rm = TRUE )
    Mdata[[ k ]][ "nEntT1" ] <- colMaxs( nEntT1, na.rm = TRUE )
    Sdata[[ k ]][ "nEntT1" ] <- colSds( nEntT1, na.rm = TRUE )
    Xdata[[ k ]][ "nEntTc" ] <- colMeans( nEntTc, na.rm = TRUE )
    mdata[[ k ]][ "nEntTc" ] <- colMins( nEntTc, na.rm = TRUE )
    Mdata[[ k ]][ "nEntTc" ] <- colMaxs( nEntTc, na.rm = TRUE )
    Sdata[[ k ]][ "nEntTc" ] <- colSds( nEntTc, na.rm = TRUE )
  }

  plot_lists( c( "nEntT1", "nEntTc" ), Xdata, mdata, Mdata, sdMC = Sdata,
              leg = legends, statMC = mcStat, mask = TmaskPlot, nMC = nSize,
              CI = CI, mrk = transMk, col = colors, lty = lTypes, mCnt = mCnt,
              xlab = "Time", ylab = "Number of net entrant firms (HP-filtered)",
              tit = "Net entry of firms trend",
              subtit = paste( "MC runs =", nSize, "/ MC", XtagAll, cntLeg ),
              leg2 = c( "Capital-good sector", "Consumption-good sector" ) )


  # ------ Market-share turbulence ------

  plot_lists( c( "HP1", "HPc" ), Xdata, mdata, Mdata, cdata, Cdata,
              leg = legends, mask = TmaskPlot, nMC = nSize, CI = CI,
              mrk = transMk, col = colors, lty = lTypes, mCnt = mCnt,
              xlab = "Time", ylab = "Hymer-Pashigian index",
              tit = "Market-share turbulence",
              subtit = paste( "MC runs =", nSize, "/ MC", XtagAll, cntLeg ),
              leg2 = c( "Capital-good sector", "Consumption-good sector" ),
              trend = "loess" )


  # ------ Entry-exit turbulence ------

  plot_lists( c( "EXk", "EXc" ), Xdata, mdata, Mdata, cdata, Cdata,
              leg = legends, mask = TmaskPlot, nMC = nSize, CI = CI,
              mrk = transMk, col = colors, lty = lTypes, mCnt = mCnt,
              xlab = "Time",
              ylab = "index = 1 - | entries - exits | / ( entries + exits )",
              tit = "Entry-exit turbulence index",
              subtit = paste( "MC runs =", nSize, "/ MC", XtagAll, cntLeg ),
              leg2 = c( "Capital-good sector", "Consumption-good sector" ),
              trend = "linear" )


  # ------ Concentration ------

  plot_lists( c( "HH1", "HHc" ), Xdata, mdata, Mdata, cdata, Cdata,
              leg = legends, mask = TmaskPlot, nMC = nSize, CI = CI,
              mrk = transMk, col = colors, lty = lTypes, mCnt = mCnt,
              xlab = "Time", ylab = "Standardized Herfindahl-Hirschman index",
              tit = "Market concentration",
              subtit = paste( "MC runs =", nSize, "/ MC", XtagAll, cntLeg ),
              leg2 = c( "Capital-good sector", "Consumption-good sector" ),
              trend = "loess" )


  # ------ Markup by firm type ------

  plot_lists( c( "muCavgBas", "muCavgLux" ), Xdata, mdata, Mdata, cdata, Cdata,
              leg = legends, mask = TmaskPlot, nMC = nSize, CI = CI, mrk = transMk,
              col = colors, lty = lTypes, mCnt = mCnt, xlab = "Time",
              ylab = "Weighted average mark-up rate",
              tit = "Mark-up",
              subtit = paste( "Consumption-good sector only / MC runs =", nSize,
                              "/ MC", XtagAll, cntLeg ),
              leg2 = c( "Basic goods", "Luxury goods" ) )
}
