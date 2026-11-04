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
# !diagnostics suppress = plot_lists, mavg

time_plots <- function( Pdata, Xdata, mdata, Mdata, Sdata, cdata, Cdata,
                        DCdata, mcStat, nExp, nSize, mCnt, TmaskPlot,
                        Ptag, Xtag, cntLeg, firmTypes ) {

  if( all( Xtag == mcStat ) )
    XtagAll <- mcStat
  else
    XtagAll <- paste( Xtag, collapse = "|" )

  subtitbase <- paste0( "period = ", min( TmaskPlot ), "-", max( TmaskPlot ),
                        " / MC runs = ", nSize, " / MC" )
  subtit <- paste( subtitbase, XtagAll, cntLeg )


  # ------ Real GDP, consumption and investment ------

  plot_lists( c( "Yreal", "Creal", "Ireal" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, log0 = TRUE, subtit = subtit, xlab = "Time",
              ylab = "Log real (initial prices) aggregate",
              tit = "Real GDP, consumption, and investment",
              leg2 = c( "GDP", "Consumption", "Investment" ) )


  # ------ Nominal GDP, consumption, investment, net exports ------

  plot_lists( c( "Y", "C", "Inom", "X" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, log0 = TRUE, subtit = subtit, xlab = "Time",
              ylab = "Log nominal (current prices) aggregate",
              tit = "Nominal GDP, consumption, investment, and net exports",
              leg2 = c( "GDP", "Consumption", "Investment", "Net exports" ) )


  # ------ GDP gap ------

  if ( mCnt && nExp > 1 )
    plot_lists( "Ygap", Xdata, mdata, Mdata, sdMC = Sdata,
                DCdata = DCdata, subtit = subtit, xlab = "Time",
                ylab = "Log gross domestic product gap",
                tit = "GDP gap" )


  # ------ Capacity utilization ------

  plot_lists( c( "Yreal", "Y100" ), Xdata, mdata, Mdata, sdMC = Sdata,
              DCdata = DCdata, log0 = TRUE, subtit = subtit,
              xlab = "Time", ylab = "Log real (initial prices) GDP",
              tit = "Capacity utilization",
              leg2 = c( "Effective GDP", "GDP @ 100% utilization" ) )


  # ------ International trade ------

  plot_lists( c( "Xk", "Mk" ), Xdata, mdata, Mdata, sdMC = Sdata,
              DCdata = DCdata, log0 = TRUE, subtit = subtit,
              xlab = "Time", ylab = "Log monetary values",
              tit = "International machine trade",
              leg2 = c( "Exports", "Imports" ) )

  plot_lists( c( "Xc", "Mc" ), Xdata, mdata, Mdata, sdMC = Sdata,
              DCdata = DCdata, log0 = TRUE, subtit = subtit,
              xlab = "Time", ylab = "Log monetary values",
              tit = "International consumption-good trade",
              leg2 = c( "Exports", "Imports" ) )

  plot_lists( "e", Xdata, mdata, Mdata, sdMC = Sdata,
              DCdata = DCdata, subtit = subtit, xlab = "Time",
              ylab = "Nominal exchange rate",
              tit = "Exchange rate" )


  # ------ Trade balance in GDP terms ------

  plot_lists( "XnetY", Xdata, mdata, Mdata, sdMC = Sdata,
              DCdata = DCdata, subtit = subtit, xlab = "Time",
              ylab = "Exports minus imports over GDP",
              tit = "Balance of trade" )


  # ------ Long-term trade balance per sector in GDP terms ------

  ma.length <- 51
  ma.sides <- 2
  init <- TmaskPlot[ 1 ] - 1
  tXk <- tXc <- list( )

  for( k in 1 : nExp ) {
    XkNetSign <- sign( mavg( Xdata[[ k ]]$XkNet_Y, len = ma.length, sides = ma.sides ) )
    for( t in 2 : length( XkNetSign ) ) {
      if( ! is.na( XkNetSign[ t - 1 ] ) && ! is.na( XkNetSign[ t ] ) &&
          XkNetSign[ t ] != XkNetSign[ t - 1 ] ) {
        tXk[[ k ]] <- t - init
        break
      }
    }

    XcNetSign <- sign( mavg( Xdata[[ k ]]$XcNet_Y, len = ma.length, sides = ma.sides ) )
    for( t in 2 : length( XcNetSign ) ) {
      if( ! is.na( XcNetSign[ t - 1 ] ) && ! is.na( XcNetSign[ t ] ) &&
          XcNetSign[ t ] != XcNetSign[ t - 1 ] ) {
        tXc[[ k ]] <- t - init
        break
      }
    }
  }

  suppressWarnings( tXk <- round( mean( unlist( tXk ), na.rm = TRUE ) ) )
  suppressWarnings( tXc <- round( mean( unlist( tXc ), na.rm = TRUE ) ) )

  plot_lists( c( "XkNet_Y", "XcNet_Y" ), Xdata, mdata, Mdata, sdMC = Sdata,
              DCdata = DCdata, subtit = subtit, xlab = "Time",
              ylab = "Exports minus imports over GDP",
              tit = "Sectoral balance of trade",
              leg2 = c( paste0( "Net capital exports (tx=", tXk, ")" ),
                        paste0( "Net consumption exports (tx=", tXc, ")" ) ) )


  # ------ Tax & government expenditures in GDP terms ------

  plot_lists( c( "Tax_Y", "G_Y", "Gbail_Y", "Ged_Y", "Gtrain_Y" ), Xdata, mdata,
              Mdata, cdata, Cdata, DCdata = DCdata, subtit = subtit,
              xlab = "Time",
              ylab = "Government tax income and expenditure over GDP",
              tit = "Government income and expenditure",
              leg2 = c( "Tax", "Gov. expenditure", "Bank bail-out",
                        "Education exp.", "Training exp." ) )


  # ------ Government deficit in GDP terms------

  plot_lists( c( "DefY", "DefPy" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, trend = "loess", subtit = subtit, xlab = "Time",
              ylab = "Government deficit over GDP",
              tit = "Government deficit",
              leg2 = c( "Total", "Primary" ) )


  # ------ Government debt in GDP terms ------

  plot_lists( "DebY", Pdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, trend = "linear", xlab = "Time",
              ylab = "Government debt over GDP",
              tit = "Government debt",
              subtit = paste( subtitbase, Ptag[ "DebY" ], cntLeg ) )


  # ------ Total credit supply and loans in GDP terms ------

  plot_lists( c( "TC_Y", "Loans_Y" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, subtit = subtit, xlab = "Time",
              ylab = "Total bank credit available and firm debt stock over GDP",
              tit = "Bank credit supply and firm loans",
              leg2 = c( "Credit available", "Loans" ) )


  # ------ Credit demand and supply in GDP terms ------

  plot_lists( c( "CD_Y", "CS_Y" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, subtit = subtit, xlab = "Time",
              ylab = "Effective total firm credit demand and bank credit supply over GDP",
              tit = "Credit demand and supply flow over GDP",
              leg2 = c( "Demand", "Supply" ) )


  # ------ Loans and bad debt in GDP terms ------

  plot_lists( c( "Loans_Y", "BadDeb_Y" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, subtit = subtit, xlab = "Time",
              ylab = "Firm debt stock and bad-debt on period over GDP",
              tit = "Firm loans and bank bad debt",
              leg2 = c( "Loans", "Bad debt" ) )


  # ------ Savings in GDP terms ------

  plot_lists( c( "Sav_Y", "SavAcc_Y" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, subtit = subtit, xlab = "Time",
              ylab = "Savings over GDP",
              tit = "Worker savings",
              leg2 = c( "Current savings", "Accumulated savings" ) )


  # ------ Bank fragility ------

  plot_lists( "Bda", Pdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, trend = "loess", xlab = "Time",
              ylab = "Accumulated banks bad debt over assets",
              tit = "Bank fragility",
              subtit = paste( subtitbase, Ptag[ "Bda" ], cntLeg ) )


  # ------ Capacity utilization and labor market participation rates ------

  plot_lists( c( "Q2u", "Lpart" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, trend = "loess", subtit = subtit, xlab = "Time",
              ylab = "Rates of capital utilization and labor participation",
              tit = "Capacity utilization and labor participation",
              leg2 = c( "Capacity utilization", "Labor participation" ) )


  # ------ Unemployment and vacancy rates ------

  plot_lists( c( "U", "V" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, subtit = subtit, xlab = "Time",
              ylab = "Overall unemployment and vacancy rates",
              tit = "Unemployment and vacancy",
              leg2 = c( "Unemployment", "Vacancy" ) )


  # ------ Unemployment rates by category ------

  plot_lists( c( "U1", "U2", "U3" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, subtit = subtit, xlab = "Time",
              ylab = "Unemployment rates",
              tit = "Unemployment by education level",
              leg2 = c( "Primary education", "Secondary education",
                        "Tertiary education" ) )


  # ------ Unemployment gap by category ------

  if ( mCnt && nExp > 1 )
    plot_lists( c( "U1gap", "U2gap", "U3gap" ), Xdata, mdata, Mdata, sdMC = Sdata,
                DCdata = DCdata, subtit = subtit, xlab = "Time",
                ylab = "Unemployment ratio gap",
                tit = "Unemployment gap",
                leg2 = c( "Primary education", "Secondary education",
                          "Tertiary education" ) )


  # ------ Vacancy rates by category ------

  plot_lists( c( "V1", "V2", "V3" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, subtit = subtit, xlab = "Time",
              ylab = "Vacancy rates",
              tit = "Vacancy by education level",
              leg2 = c( "Primary education", "Secondary education",
                        "Tertiary education" ) )


  # ------ Vacancy gap by category ------

  if ( mCnt && nExp > 1 )
    plot_lists( c( "V1gap", "V2gap", "V3gap" ), Xdata, mdata, Mdata, sdMC = Sdata,
                DCdata = DCdata, subtit = subtit, xlab = "Time",
                ylab = "Vacancy ratio gap",
                tit = "Vacancy gap",
                leg2 = c( "Primary education", "Secondary education",
                          "Tertiary education" ) )


  # ------ Real wages ------

  plot_lists( "wAvgReal", Pdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, log0 = TRUE, xlab = "Time",
              ylab = "Log average real wage", tit = "Real wage",
              subtit = paste( subtitbase, Ptag[ "wAvgReal" ], cntLeg ) )

  # ------ Real wages by category ------

  plot_lists( c( "wAvg1real", "wAvg2real", "wAvg3real" ), Xdata, mdata, Mdata,
              cdata, Cdata, DCdata = DCdata, log0 = TRUE, subtit = subtit,
              xlab = "Time", ylab = "Log average real wage",
              tit = "Wages by education level",
              leg2 = c( "Primary education", "Secondary education",
                        "Tertiary education" ) )


  # ------ Wage gap by category ------

  if ( mCnt && nExp > 1 )
    plot_lists( c( "w1gap", "w2gap", "w3gap" ), Xdata, mdata, Mdata, sdMC = Sdata,
            DCdata = DCdata, subtit = subtit, xlab = "Time",
            ylab = "Log average real wage gap",
            tit = "Wage gap",
            leg2 = c( "Primary education", "Secondary education",
                      "Tertiary education" ) )


  # ------ Real wages by firm type ------

  plot_lists( c( "w2realPreChg", "w2realPosChg" ), Xdata, mdata, Mdata,
              cdata, Cdata, DCdata = DCdata, log0 = TRUE, xlab = "Time",
              ylab = "Log average real wage",
              tit = "Wages by firm type",
              subtit = paste( "Consumption-good sector only /", subtit ),
              leg2 = firmTypes )


  # ------ Wage, bonus and total income share in GDP terms ------

  plot_lists( c( "W_Y", "Bon2_Y", "In_Y" ), Xdata, mdata, Mdata, sdMC = Sdata,
              DCdata = DCdata, trend = "loess", subtit = subtit, xlab = "Time",
              ylab = "Total wages, bonus, and income over GDP",
              tit = "Wage share",
              leg2 = c( "Wages", "Bonus", "Income" ) )


  # ------ Wages spread ------

  plot_lists( "wLogSD", Pdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, trend = "loess", xlab = "Time",
              ylab = "Standard deviation of log real wage",
              tit = "Wage spread",
              subtit = paste( subtitbase, Ptag[ "wLogSD" ], cntLeg ) )


  # ------ Gini index ------

  plot_lists( c( "Gini", "wGini" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, trend = "loess", subtit = subtit, xlab = "Time",
              ylab = "Gini index",
              tit = "Gini index",
              leg2 = c( "Total income", "Wage income only" ) )


  # ------ Average tenure and vintage skills ------

  plot_lists( c( "sTavg", "sVavg" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, subtit = subtit, xlab = "Time",
              ylab = "Average worker skills level",
              tit = "Workers' skills",
              leg2 = c( "Tenure skills", "Vintage skills" ) )


  # ------ Innovation and imitation ------

  plot_lists( c( "inn", "imi" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, trend = "loess", xlab = "Time",
              ylab = "Share of innovating and imitating firms",
              tit = "Innovation and imitation",
              subtit = paste( "Capital-good sector only /", subtit ),
              leg2 = c( "Innovation", "Imitation" ) )


  # ------ Productivity by sector ------

  plot_lists( c( "A", "A1", "A2" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, log = TRUE, subtit = subtit, xlab = "Time",
              ylab = "Average log labor productivity",
              tit = "Productivity by sector",
              leg2 = c( "Overall", "Capital-good sector", "Consumption-good sector" ) )


  # ------ Productivity gap ------

  if ( mCnt && nExp > 1 )
    plot_lists( "A1gap", Xdata, mdata, Mdata, sdMC = Sdata,
                DCdata = DCdata, subtit = subtit, xlab = "Time",
                ylab = "Capital-good log labor productivity gap",
                tit = "Productivity gap" )


  # ------ Productivity by firm type ------

  plot_lists( c( "A2preChg", "A2posChg" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, log0 = TRUE, subtit = subtit, xlab = "Time",
              ylab = "Average log labor productivity",
              tit = "Productivity by firm type",
              leg2 = firmTypes )


  # ------ Productivity spread by firm type ------

  plot_lists( c( "A2sdPreChg", "A2sdPosChg" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, subtit = subtit, xlab = "Time",
              ylab = "Standard deviation of firm log productivity",
              tit = "Productivity spread",
              leg2 = firmTypes )


  # ------ Product quality by firm type ------

  plot_lists( c( "q2preChg", "q2posChg" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, trend = "loess", subtit = subtit, xlab = "Time",
              ylab = "Weighted average consumer-goods reference quality",
              tit = "Product quality",
              leg2 = firmTypes )


  # ------ Share of public firms in capital -------

  plot_lists( "f1g", Pdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, xlab = "Time",
              ylab = "Number of government-owned capital-good firms over total",
              tit = "Share of public capital-good firms",
              subtit = paste( subtitbase, Ptag[ "f2posChg" ], cntLeg ) )


  # ------ Share of public firms in consumption -------

  plot_lists( "f2g", Pdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata,  xlab = "Time",
              ylab = "Number of government-owned consumption-good firms over total",
              tit = "Share of public consumption-good firms",
              subtit = paste( subtitbase, Ptag[ "f2posChg" ], cntLeg ) )


  # ------ Market share of post-change firms ------

  plot_lists( "f2posChg", Pdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, xlab = "Time",
              ylab = paste( "Market share of", firmTypes[2] ),
              tit = paste( "Market share of", firmTypes[2] ),
              subtit = paste( "Consumption-good sector only /", subtitbase,
                              Ptag[ "f2posChg" ], cntLeg ) )


  # ------ Firms in the market ------

  plot_lists( c( "F2", "F1" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, subtit = subtit, xlab = "Time",
              ylab = "Number of firms",
              tit = "Firms operating in the market",
              leg2 = c( "Consumption goods", "Capital goods" ) )


  # ------ Firm entry and exit in the consumer market ------

  plot_lists( c( "entry2", "exit2" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, trend = "loess", xlab = "Time",
              ylab = "Share of total firms",
              tit = "Firm entry and exit in the consumption-goods market",
              subtit = paste( "Consumption-good sector only /", subtit ),
              leg2 = c( "Entry", "Exit" ) )


  # ------ Firms net entry trend in the market ------

  plot_lists( c( "nEntT1", "nEntT2" ), Xdata, mdata, Mdata, sdMC = Sdata,
              DCdata = DCdata, subtit = subtit, xlab = "Time",
              ylab = "Number of net entrant firms (HP-filtered)",
              tit = "Net entry of firms trend",
              leg2 = c( "Capital-good sector", "Consumption-good sector" ) )


  # ------ Market-share turbulence ------

  plot_lists( c( "HP1", "HP2" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, trend = "loess", subtit = subtit, xlab = "Time",
              ylab = "Hymer-Pashigian index",
              tit = "Market-share turbulence",
              leg2 = c( "Capital-good sector", "Consumption-good sector" ) )


  # ------ Concentration ------

  plot_lists( c( "HH1", "HH2" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, trend = "loess", subtit = subtit, xlab = "Time",
              ylab = "Standardized Herfindahl-Hirschman index",
              tit = "Market concentration",
              leg2 = c( "Capital-good sector", "Consumption-good sector" ) )


  # ------ Markup ------

  plot_lists( "mu2avg", Pdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, xlab = "Time",
              ylab = "Weighted average mark-up rate",
              tit = "Mark-up",
              subtit = paste( "Consumption-good sector only /", subtitbase,
                              Ptag[ "mu2avg" ], cntLeg ) )
}
