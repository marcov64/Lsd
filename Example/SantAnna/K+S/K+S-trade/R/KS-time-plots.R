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
# !diagnostics suppress = log0, plot_lists, hpfilter, colSds, colMins, colMaxs, mavg

time_plots <- function( mcData, Pdata, Xdata, mdata, Mdata, Sdata, cdata, Cdata,
                        DCdata, mcStat, nExp, nSize, nTsteps, mCnt, TmaskPlot, CI,
                        Ptag, Xtag, legends, cntLeg, colors, lTypes, smoothing,
                        transMk, firmTypes ) {

  if( all( Xtag == mcStat ) )
    XtagAll <- mcStat
  else
    XtagAll <- paste( Xtag, collapse = "|" )

  subtitbase <- paste0( "period = ", min( TmaskPlot ), "-", max( TmaskPlot ),
                        " / MC runs = ", nSize, " / MC" )
  subtit <- paste( subtitbase, XtagAll, cntLeg )

  # ------ Real GDP, consumption and investment ------

  plot_lists( c( "Yreal", "Creal", "Ireal" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, leg = legends, mask = TmaskPlot, nMC = nSize,
              CI = CI, log0 = TRUE, col = colors, mrk = transMk, lty = lTypes,
              xlab = "Time", ylab = "Log real (initial prices) aggregate",
              tit = "Real GDP, consumption, and investment", subtit = subtit,
              leg2 = c( "GDP", "Consumption", "Investment" ) )


  # ------ Nominal GDP, consumption, investment, net exports ------

  plot_lists( c( "Y", "C", "Inom", "X" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, leg = legends, mask = TmaskPlot, nMC = nSize,
              CI = CI, log0 = TRUE, col = colors, mrk = transMk, lty = lTypes,
              xlab = "Time", ylab = "Log nominal (current prices) aggregate",
              tit = "Nominal GDP, consumption, investment, and net exports",
              subtit = subtit,
              leg2 = c( "GDP", "Consumption", "Investment", "Net exports" ) )


  # ------ GDP gap ------

  if ( mCnt && nExp > 1 ) {

    var <- "Yreal"
    varNew <- "Ygap"

    for( k in 1 : ( nExp - 1 ) ) {
      Xdata[[ k ]][ varNew ] <- mdata[[ k ]][ varNew ] <- Mdata[[ k ]][ varNew ] <-
        log0( Xdata[[ k ]][ , var ] ) - log0( Xdata[[ k + 1 ]][ , var ] )
      Sdata[[ k ]][ varNew ] <- rep( 0, length( Xdata[[ k ]][ , var ] ) )
    }

    Xdata[[ nExp ]][ varNew ] <- mdata[[ nExp ]][ varNew ] <- Mdata[[ nExp ]][ varNew ] <-
      log0( Xdata[[ nExp ]][ , var ] ) - log0( Xdata[[ nExp - 1 ]][ , var ] )
    Sdata[[ nExp ]][ varNew ] <- rep( 0, length( Xdata[[ k ]][ , var ] ) )

    plot_lists( varNew, Xdata, mdata, Mdata, sdMC = Sdata, statMC = mcStat,
                DCdata = DCdata, leg = legends, mask = TmaskPlot, nMC = nSize,
                CI = CI, col = colors, mrk = transMk, lty = lTypes,
                mCnt = mCnt, xlab = "Time", ylab = "Log gross domestic product gap",
                tit = "GDP gap", subtit = subtit )
  }


  # ------ Capacity utilization ------

  for( k in 1 : nExp ) {
    Xdata[[ k ]][ "Y100" ] <- Xdata[[ k ]]$Yreal / Xdata[[ k ]]$Q2u
    mdata[[ k ]][ "Y100" ] <- mdata[[ k ]]$Yreal / Xdata[[ k ]]$Q2u
    Mdata[[ k ]][ "Y100" ] <- Mdata[[ k ]]$Yreal / Xdata[[ k ]]$Q2u
    Sdata[[ k ]][ "Y100" ] <- sqrt( Sdata[[ k ]]$Yreal ^ 2 + Sdata[[ k ]]$Q2u ^ 2 )
  }

  plot_lists( c( "Yreal", "Y100" ), Xdata, mdata, Mdata, sdMC = Sdata, statMC = mcStat,
              DCdata = DCdata, leg = legends, mask = TmaskPlot, nMC = nSize,
              CI = CI, log0 = TRUE, col = colors, mrk = transMk, lty = lTypes,
              mCnt = mCnt, xlab = "Time", ylab = "Log real (initial prices) GDP",
              tit = "Capacity utilization", subtit = subtit,
              leg2 = c( "Effective GDP", "GDP @ 100% utilization" ) )


  # ------ International trade ------

  plot_lists( c( "Xk", "Mk" ), Xdata, mdata, Mdata, sdMC = Sdata, statMC = mcStat,
              DCdata = DCdata, leg = legends, mask = TmaskPlot, nMC = nSize,
              CI = CI, log0 = TRUE, col = colors, mrk = transMk, lty = lTypes,
              mCnt = mCnt, xlab = "Time", ylab = "Log monetary values",
              tit = "International machine trade", subtit = subtit,
              leg2 = c( "Exports", "Imports" ) )

  plot_lists( c( "Xc", "Mc" ), Xdata, mdata, Mdata, sdMC = Sdata, statMC = mcStat,
              DCdata = DCdata, leg = legends, mask = TmaskPlot, nMC = nSize,
              CI = CI, log0 = TRUE, col = colors, mrk = transMk, lty = lTypes,
              mCnt = mCnt, xlab = "Time", ylab = "Log monetary values",
              tit = "International consumption-good trade", subtit = subtit,
              leg2 = c( "Exports", "Imports" ) )

  plot_lists( "e", Xdata, mdata, Mdata, sdMC = Sdata, statMC = mcStat,
              DCdata = DCdata, leg = legends, mask = TmaskPlot, nMC = nSize,
              CI = CI, col = colors, mrk = transMk, lty = lTypes, mCnt = mCnt,
              xlab = "Time", ylab = "Nominal exchange rate",
              tit = "Exchange rate", subtit = subtit )


  # ------ Trade balance in GDP terms ------

  for( k in 1 : nExp ) {
    Xdata[[ k ]][ "XnetY" ] <- ( Xdata[[ k ]]$X - Xdata[[ k ]]$M ) / Xdata[[ k ]]$Y
    mdata[[ k ]][ "XnetY" ] <- ( mdata[[ k ]]$X - mdata[[ k ]]$M ) / Xdata[[ k ]]$Y
    Mdata[[ k ]][ "XnetY" ] <- ( Mdata[[ k ]]$X - Mdata[[ k ]]$M ) / Xdata[[ k ]]$Y
    cdata[[ k ]][ "XnetY" ] <- ( cdata[[ k ]]$X + cdata[[ k ]]$M ) / Xdata[[ k ]]$Y
    Cdata[[ k ]][ "XnetY" ] <- ( Cdata[[ k ]]$X + Cdata[[ k ]]$M ) / Xdata[[ k ]]$Y
  }

  plot_lists( "XnetY", Xdata, mdata, Mdata, sdMC = Sdata, statMC = mcStat,
              DCdata = DCdata, leg = legends, mask = TmaskPlot, nMC = nSize,
              CI = CI, col = colors, mrk = transMk, lty = lTypes, mCnt = mCnt,
              xlab = "Time", ylab = "Exports minus imports over GDP",
              tit = "Balance of trade", subtit = subtit )


  # ------ Long-term trade balance per sector in GDP terms ------

  ma.length <- 51
  ma.sides <- 2
  init <- TmaskPlot[ 1 ] - 1
  tXk <- tXc <- list( )

  for( k in 1 : nExp ) {
    Xdata[[ k ]][ "XkNetY" ] <- ( Xdata[[ k ]]$Xk - Xdata[[ k ]]$Mk ) / Xdata[[ k ]]$Y
    mdata[[ k ]][ "XkNetY" ] <- ( mdata[[ k ]]$Xk - mdata[[ k ]]$Mk ) / Xdata[[ k ]]$Y
    Mdata[[ k ]][ "XkNetY" ] <- ( Mdata[[ k ]]$Xk - Mdata[[ k ]]$Mk ) / Xdata[[ k ]]$Y
    cdata[[ k ]][ "XkNetY" ] <- ( cdata[[ k ]]$Xk + cdata[[ k ]]$Mk ) / Xdata[[ k ]]$Y
    Cdata[[ k ]][ "XkNetY" ] <- ( Cdata[[ k ]]$Xk + Cdata[[ k ]]$Mk ) / Xdata[[ k ]]$Y
    Xdata[[ k ]][ "XcNetY" ] <- ( Xdata[[ k ]]$Xc - Xdata[[ k ]]$Mc ) / Xdata[[ k ]]$Y
    mdata[[ k ]][ "XcNetY" ] <- ( mdata[[ k ]]$Xc - mdata[[ k ]]$Mc ) / Xdata[[ k ]]$Y
    Mdata[[ k ]][ "XcNetY" ] <- ( Mdata[[ k ]]$Xc - Mdata[[ k ]]$Mc ) / Xdata[[ k ]]$Y
    cdata[[ k ]][ "XcNetY" ] <- ( cdata[[ k ]]$Xc + cdata[[ k ]]$Mc ) / Xdata[[ k ]]$Y
    Cdata[[ k ]][ "XcNetY" ] <- ( Cdata[[ k ]]$Xc + Cdata[[ k ]]$Mc ) / Xdata[[ k ]]$Y

    XkNetSign <- sign( mavg( Xdata[[ k ]]$XkNetY, len = ma.length, sides = ma.sides ) )
    for( t in 2 : length( XkNetSign ) ) {
      if( ! is.na( XkNetSign[ t - 1 ] ) && ! is.na( XkNetSign[ t ] ) &&
          XkNetSign[ t ] != XkNetSign[ t - 1 ] ) {
        tXk[[ k ]] <- t - init
        break
      }
    }

    XcNetSign <- sign( mavg( Xdata[[ k ]]$XcNetY, len = ma.length, sides = ma.sides ) )
    for( t in 2 : length( XcNetSign ) ) {
      if( ! is.na( XcNetSign[ t - 1 ] ) && ! is.na( XcNetSign[ t ] ) &&
          XcNetSign[ t ] != XcNetSign[ t - 1 ] ) {
        tXc[[ k ]] <- t - init
        break
      }
    }
  }

  tXk = round( mean( unlist( tXk ) ) )
  tXc = round( mean( unlist( tXc ) ) )

  plot_lists( c( "XkNetY", "XcNetY" ), Xdata, mdata, Mdata, sdMC = Sdata, statMC = mcStat,
              DCdata = DCdata, leg = legends, mask = TmaskPlot, nMC = nSize,
              CI = CI, col = colors, mrk = transMk, lty = lTypes, mCnt = mCnt,
              xlab = "Time", ylab = "Exports minus imports over GDP",
              tit = "Sectoral balance of trade", subtit = subtit,
              leg2 = c( paste0( "Net capital exports (tx=", tXk, ")" ),
                        paste0( "Net consumption exports (tx=", tXc, ")" ) ) )


  # ------ Tax & government expenditures in GDP terms ------

  for( k in 1 : nExp ) {
    Xdata[[ k ]][ "TaxY" ] <- Xdata[[ k ]]$Tax / Xdata[[ k ]]$Y
    mdata[[ k ]][ "TaxY" ] <- mdata[[ k ]]$Tax / Xdata[[ k ]]$Y
    Mdata[[ k ]][ "TaxY" ] <- Mdata[[ k ]]$Tax / Xdata[[ k ]]$Y
    cdata[[ k ]][ "TaxY" ] <- cdata[[ k ]]$Tax / Xdata[[ k ]]$Y
    Cdata[[ k ]][ "TaxY" ] <- Cdata[[ k ]]$Tax / Xdata[[ k ]]$Y
    Xdata[[ k ]][ "GY" ] <- Xdata[[ k ]]$G / Xdata[[ k ]]$Y
    mdata[[ k ]][ "GY" ] <- mdata[[ k ]]$G / Xdata[[ k ]]$Y
    Mdata[[ k ]][ "GY" ] <- Mdata[[ k ]]$G / Xdata[[ k ]]$Y
    cdata[[ k ]][ "GY" ] <- cdata[[ k ]]$G / Xdata[[ k ]]$Y
    Cdata[[ k ]][ "GY" ] <- Cdata[[ k ]]$G / Xdata[[ k ]]$Y
    Xdata[[ k ]][ "GbailY" ] <- Xdata[[ k ]]$Gbail / Xdata[[ k ]]$Y
    mdata[[ k ]][ "GbailY" ] <- mdata[[ k ]]$Gbail / Xdata[[ k ]]$Y
    Mdata[[ k ]][ "GbailY" ] <- Mdata[[ k ]]$Gbail / Xdata[[ k ]]$Y
    cdata[[ k ]][ "GbailY" ] <- cdata[[ k ]]$Gbail / Xdata[[ k ]]$Y
    Cdata[[ k ]][ "GedY" ] <- Cdata[[ k ]]$Ged / Xdata[[ k ]]$Y
    Xdata[[ k ]][ "GedY" ] <- Xdata[[ k ]]$Ged / Xdata[[ k ]]$Y
    mdata[[ k ]][ "GedY" ] <- mdata[[ k ]]$Ged / Xdata[[ k ]]$Y
    Mdata[[ k ]][ "GedY" ] <- Mdata[[ k ]]$Ged / Xdata[[ k ]]$Y
    cdata[[ k ]][ "GedY" ] <- cdata[[ k ]]$Ged / Xdata[[ k ]]$Y
    Cdata[[ k ]][ "GedY" ] <- Cdata[[ k ]]$Ged / Xdata[[ k ]]$Y
    Xdata[[ k ]][ "GtrainY" ] <- Xdata[[ k ]]$Gtrain / Xdata[[ k ]]$Y
    mdata[[ k ]][ "GtrainY" ] <- mdata[[ k ]]$Gtrain / Xdata[[ k ]]$Y
    Mdata[[ k ]][ "GtrainY" ] <- Mdata[[ k ]]$Gtrain / Xdata[[ k ]]$Y
    cdata[[ k ]][ "GtrainY" ] <- cdata[[ k ]]$Gtrain / Xdata[[ k ]]$Y
    Cdata[[ k ]][ "GtrainY" ] <- Cdata[[ k ]]$Gtrain / Xdata[[ k ]]$Y
  }

  plot_lists( c( "TaxY", "GY", "GbailY", "GedY", "GtrainY" ), Xdata, mdata,
              Mdata, cdata, Cdata, DCdata = DCdata, leg = legends,
              mask = TmaskPlot, nMC = nSize, CI = CI, mrk = transMk,
              col = colors, lty = lTypes, mCnt = mCnt, xlab = "Time",
              ylab = "Government tax income and expenditure over GDP",
              tit = "Government income and expenditure", subtit = subtit,
              leg2 = c( "Tax", "Gov. expenditure", "Bank bail-out",
                        "Education exp.", "Training exp." ) )


  # ------ Government deficit in GDP terms------

  plot_lists( c( "DefY", "DefPy" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, leg = legends, mask = TmaskPlot, nMC = nSize,
              CI = CI, mrk = transMk, col = colors, lty = lTypes, mCnt = mCnt,
              xlab = "Time", ylab = "Government deficit over GDP",
              tit = "Government deficit", subtit = subtit,
              leg2 = c( "Total", "Primary" ), trend = "loess" )


  # ------ Government debt in GDP terms ------

  plot_lists( "DebY", Pdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, leg = legends, mask = TmaskPlot, nMC = nSize,
              CI = CI, mrk = transMk, col = colors, lty = lTypes,
              xlab = "Time", ylab = "Government debt over GDP",
              tit = "Government debt",
              subtit = paste( subtitbase, Ptag[ "DebY" ], cntLeg ),
              trend = "linear" )


  # ------ Total credit supply and loans in GDP terms ------

  for( k in 1 : nExp ) {
    Xdata[[ k ]][ "TCY" ] <- Xdata[[ k ]]$TC / Xdata[[ k ]]$Y
    mdata[[ k ]][ "TCY" ] <- mdata[[ k ]]$TC / Xdata[[ k ]]$Y
    Mdata[[ k ]][ "TCY" ] <- Mdata[[ k ]]$TC / Xdata[[ k ]]$Y
    cdata[[ k ]][ "TCY" ] <- cdata[[ k ]]$TC / Xdata[[ k ]]$Y
    Cdata[[ k ]][ "TCY" ] <- Cdata[[ k ]]$TC / Xdata[[ k ]]$Y
    Xdata[[ k ]][ "LoansY" ] <- Xdata[[ k ]]$Loans / Xdata[[ k ]]$Y
    mdata[[ k ]][ "LoansY" ] <- mdata[[ k ]]$Loans / Xdata[[ k ]]$Y
    Mdata[[ k ]][ "LoansY" ] <- Mdata[[ k ]]$Loans / Xdata[[ k ]]$Y
    cdata[[ k ]][ "LoansY" ] <- cdata[[ k ]]$Loans / Xdata[[ k ]]$Y
    Cdata[[ k ]][ "LoansY" ] <- Cdata[[ k ]]$Loans / Xdata[[ k ]]$Y
  }

  plot_lists( c( "TCY", "LoansY" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, leg = legends, mask = TmaskPlot, nMC = nSize,
              CI = CI, mrk = transMk, col = colors, lty = lTypes, mCnt = mCnt,
              xlab = "Time",
              ylab = "Total bank credit available and firm debt stock over GDP",
              tit = "Bank credit supply and firm loans", subtit = subtit,
              leg2 = c( "Credit available", "Loans" ) )


  # ------ Credit demand and supply in GDP terms ------

  for( k in 1 : nExp ) {
    Xdata[[ k ]][ "CDY" ] <- Xdata[[ k ]]$CD / Xdata[[ k ]]$Y
    mdata[[ k ]][ "CDY" ] <- mdata[[ k ]]$CD / Xdata[[ k ]]$Y
    Mdata[[ k ]][ "CDY" ] <- Mdata[[ k ]]$CD / Xdata[[ k ]]$Y
    cdata[[ k ]][ "CDY" ] <- cdata[[ k ]]$CD / Xdata[[ k ]]$Y
    Cdata[[ k ]][ "CDY" ] <- Cdata[[ k ]]$CD / Xdata[[ k ]]$Y
    Xdata[[ k ]][ "CSY" ] <- Xdata[[ k ]]$CS / Xdata[[ k ]]$Y
    mdata[[ k ]][ "CSY" ] <- mdata[[ k ]]$CS / Xdata[[ k ]]$Y
    Mdata[[ k ]][ "CSY" ] <- Mdata[[ k ]]$CS / Xdata[[ k ]]$Y
    cdata[[ k ]][ "CSY" ] <- cdata[[ k ]]$CS / Xdata[[ k ]]$Y
    Cdata[[ k ]][ "CSY" ] <- Cdata[[ k ]]$CS / Xdata[[ k ]]$Y
  }

  plot_lists( c( "CDY", "CSY" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, leg = legends, mask = TmaskPlot, nMC = nSize,
              CI = CI, mrk = transMk, col = colors, lty = lTypes, mCnt = mCnt,
              xlab = "Time",
              ylab = "Effective total firm credit demand and bank credit supply over GDP",
              tit = "Credit demand and supply flow over GDP", subtit = subtit,
              leg2 = c( "Demand", "Supply" ) )


  # ------ Loans and bad debt in GDP terms ------

  for( k in 1 : nExp ) {
    Xdata[[ k ]][ "BadDebY" ] <- Xdata[[ k ]]$BadDeb / Xdata[[ k ]]$Y
    mdata[[ k ]][ "BadDebY" ] <- mdata[[ k ]]$BadDeb / Xdata[[ k ]]$Y
    Mdata[[ k ]][ "BadDebY" ] <- Mdata[[ k ]]$BadDeb / Xdata[[ k ]]$Y
    cdata[[ k ]][ "BadDebY" ] <- cdata[[ k ]]$BadDeb / Xdata[[ k ]]$Y
    Cdata[[ k ]][ "BadDebY" ] <- Cdata[[ k ]]$BadDeb / Xdata[[ k ]]$Y
  }

  plot_lists( c( "LoansY", "BadDebY" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, leg = legends, mask = TmaskPlot, nMC = nSize,
              CI = CI, mrk = transMk, col = colors, lty = lTypes, mCnt = mCnt,
              xlab = "Time",
              ylab = "Firm debt stock and bad-debt on period over GDP",
              tit = "Firm loans and bank bad debt", subtit = subtit,
              leg2 = c( "Loans", "Bad debt" ) )


  # ------ Savings in GDP terms ------

  for( k in 1 : nExp ) {
    Xdata[[ k ]][ "SavY" ] <- Xdata[[ k ]]$Sav / Xdata[[ k ]]$Y
    mdata[[ k ]][ "SavY" ] <- mdata[[ k ]]$Sav / Xdata[[ k ]]$Y
    Mdata[[ k ]][ "SavY" ] <- Mdata[[ k ]]$Sav / Xdata[[ k ]]$Y
    cdata[[ k ]][ "SavY" ] <- cdata[[ k ]]$Sav / Xdata[[ k ]]$Y
    Cdata[[ k ]][ "SavY" ] <- Cdata[[ k ]]$Sav / Xdata[[ k ]]$Y
    Xdata[[ k ]][ "SavAccY" ] <- Xdata[[ k ]]$SavAcc / Xdata[[ k ]]$Y
    mdata[[ k ]][ "SavAccY" ] <- mdata[[ k ]]$SavAcc / Xdata[[ k ]]$Y
    Mdata[[ k ]][ "SavAccY" ] <- Mdata[[ k ]]$SavAcc / Xdata[[ k ]]$Y
    cdata[[ k ]][ "SavAccY" ] <- cdata[[ k ]]$SavAcc / Xdata[[ k ]]$Y
    Cdata[[ k ]][ "SavAccY" ] <- Cdata[[ k ]]$SavAcc / Xdata[[ k ]]$Y
  }

  plot_lists( c( "SavY", "SavAccY" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, leg = legends, mask = TmaskPlot, nMC = nSize,
              CI = CI, mrk = transMk, col = colors, lty = lTypes, mCnt = mCnt,
              xlab = "Time", ylab = "Savings over GDP",
              tit = "Worker savings", subtit = subtit,
              leg2 = c( "Current savings", "Accumulated savings" ) )


  # ------ Bank fragility ------

  plot_lists( "Bda", Pdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, leg = legends, mask = TmaskPlot, nMC = nSize,
              CI = CI, mrk = transMk, col = colors, lty = lTypes, mCnt = mCnt,
              xlab = "Time", ylab = "Accumulated banks bad debt over assets",
              tit = "Bank fragility",
              subtit = paste( subtitbase, Ptag[ "Bda" ], cntLeg ),
              trend = "loess" )


  # ------ Capacity utilization and labor market participation rates ------

  for( k in 1 : nExp ) {
    Xdata[[ k ]][ "Lpart" ] <- Xdata[[ k ]]$L / Xdata[[ k ]]$Ls
    mdata[[ k ]][ "Lpart" ] <- mdata[[ k ]]$L / Xdata[[ k ]]$Ls
    Mdata[[ k ]][ "Lpart" ] <- Mdata[[ k ]]$L / Xdata[[ k ]]$Ls
    cdata[[ k ]][ "Lpart" ] <- cdata[[ k ]]$L / Xdata[[ k ]]$Ls
    Cdata[[ k ]][ "Lpart" ] <- Cdata[[ k ]]$L / Xdata[[ k ]]$Ls
  }

  plot_lists( c( "Q2u", "Lpart" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, leg = legends, mask = TmaskPlot, nMC = nSize,
              CI = CI, mrk = transMk, col = colors, lty = lTypes, mCnt = mCnt,
              xlab = "Time",
              ylab = "Rates of capital utilization and labor participation",
              tit = "Capacity utilization and labor participation", subtit = subtit,
              leg2 = c( "Capacity utilization", "Labor participation" ),
              trend = "loess" )


  # ------ Unemployment and vacancy rates ------

  plot_lists( c( "U", "V" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, leg = legends, mask = TmaskPlot, nMC = nSize,
              CI = CI, mrk = transMk, col = colors, lty = lTypes, mCnt = mCnt,
              xlab = "Time", ylab = "Overall unemployment and vacancy rates",
              tit = "Unemployment and vacancy", subtit = subtit,
              leg2 = c( "Unemployment", "Vacancy" ) )


  # ------ Unemployment rates by category ------

  plot_lists( c( "U1", "U2", "U3" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, leg = legends, mask = TmaskPlot, nMC = nSize,
              CI = CI, mrk = transMk, col = colors, lty = lTypes, mCnt = mCnt,
              xlab = "Time", ylab = "Unemployment rates",
              tit = "Unemployment by education level", subtit = subtit,
              leg2 = c( "Primary education", "Secondary education",
                        "Tertiary education" ) )


  # ------ Unemployment gap by category ------

  if ( mCnt && nExp > 1 ) {

    var <- "U1"
    varNew <- "U1gap"

    for( k in 1 : ( nExp - 1 ) ) {
      Xdata[[ k ]][ varNew ] <- mdata[[ k ]][ varNew ] <- Mdata[[ k ]][ varNew ] <-
        Xdata[[ k ]][ , var ] - Xdata[[ k + 1 ]][ , var ]
      Sdata[[ k ]][ varNew ] <- rep( 0, length( Xdata[[ k ]][ , var ] ) )
    }

    Xdata[[ nExp ]][ varNew ] <- mdata[[ nExp ]][ varNew ] <- Mdata[[ nExp ]][ varNew ] <-
      Xdata[[ nExp ]][ , var ] - Xdata[[ nExp - 1 ]][ , var ]
    Sdata[[ nExp ]][ varNew ] <- rep( 0, length( Xdata[[ k ]][ , var ] ) )

    var <- "U2"
    varNew <- "U2gap"

    for( k in 1 : ( nExp - 1 ) ) {
      Xdata[[ k ]][ varNew ] <- mdata[[ k ]][ varNew ] <- Mdata[[ k ]][ varNew ] <-
        Xdata[[ k ]][ , var ] - Xdata[[ k + 1 ]][ , var ]
      Sdata[[ k ]][ varNew ] <- rep( 0, length( Xdata[[ k ]][ , var ] ) )
    }

    Xdata[[ nExp ]][ varNew ] <- mdata[[ nExp ]][ varNew ] <- Mdata[[ nExp ]][ varNew ] <-
      Xdata[[ nExp ]][ , var ] - Xdata[[ nExp - 1 ]][ , var ]
    Sdata[[ nExp ]][ varNew ] <- rep( 0, length( Xdata[[ k ]][ , var ] ) )

    var <- "U3"
    varNew <- "U3gap"

    for( k in 1 : ( nExp - 1 ) ) {
      Xdata[[ k ]][ varNew ] <- mdata[[ k ]][ varNew ] <- Mdata[[ k ]][ varNew ] <-
        Xdata[[ k ]][ , var ] - Xdata[[ k + 1 ]][ , var ]
      Sdata[[ k ]][ varNew ] <- rep( 0, length( Xdata[[ k ]][ , var ] ) )
    }

    Xdata[[ nExp ]][ varNew ] <- mdata[[ nExp ]][ varNew ] <- Mdata[[ nExp ]][ varNew ] <-
      Xdata[[ nExp ]][ , var ] - Xdata[[ nExp - 1 ]][ , var ]
    Sdata[[ nExp ]][ varNew ] <- rep( 0, length( Xdata[[ k ]][ , var ] ) )

    plot_lists( c( "U1gap", "U2gap", "U3gap" ), Xdata, mdata, Mdata, sdMC = Sdata,
                statMC = mcStat, DCdata = DCdata, leg = legends, mask = TmaskPlot,
                nMC = nSize, CI = CI, col = colors, mrk = transMk, lty = lTypes,
                mCnt = mCnt, xlab = "Time", ylab = "Unemployment ratio gap",
                tit = "Unemployment gap", subtit = subtit,
                leg2 = c( "Primary education", "Secondary education",
                          "Tertiary education" ) )
  }


  # ------ Vacancy rates by category ------

  plot_lists( c( "V1", "V2", "V3" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, leg = legends, mask = TmaskPlot, nMC = nSize,
              CI = CI, mrk = transMk, col = colors, lty = lTypes, mCnt = mCnt,
              xlab = "Time", ylab = "Vacancy rates",
              tit = "Vacancy by education level", subtit = subtit,
              leg2 = c( "Primary education", "Secondary education",
                        "Tertiary education" ) )


  # ------ Vacancy gap by category ------

  if ( mCnt && nExp > 1 ) {

    var <- "V1"
    varNew <- "V1gap"

    for( k in 1 : ( nExp - 1 ) ) {
      Xdata[[ k ]][ varNew ] <- mdata[[ k ]][ varNew ] <- Mdata[[ k ]][ varNew ] <-
        Xdata[[ k ]][ , var ] - Xdata[[ k + 1 ]][ , var ]
      Sdata[[ k ]][ varNew ] <- rep( 0, length( Xdata[[ k ]][ , var ] ) )
    }

    Xdata[[ nExp ]][ varNew ] <- mdata[[ nExp ]][ varNew ] <- Mdata[[ nExp ]][ varNew ] <-
      Xdata[[ nExp ]][ , var ] - Xdata[[ nExp - 1 ]][ , var ]
    Sdata[[ nExp ]][ varNew ] <- rep( 0, length( Xdata[[ k ]][ , var ] ) )

    var <- "V2"
    varNew <- "V2gap"

    for( k in 1 : ( nExp - 1 ) ) {
      Xdata[[ k ]][ varNew ] <- mdata[[ k ]][ varNew ] <- Mdata[[ k ]][ varNew ] <-
        Xdata[[ k ]][ , var ] - Xdata[[ k + 1 ]][ , var ]
      Sdata[[ k ]][ varNew ] <- rep( 0, length( Xdata[[ k ]][ , var ] ) )
    }

    Xdata[[ nExp ]][ varNew ] <- mdata[[ nExp ]][ varNew ] <- Mdata[[ nExp ]][ varNew ] <-
      Xdata[[ nExp ]][ , var ] - Xdata[[ nExp - 1 ]][ , var ]
    Sdata[[ nExp ]][ varNew ] <- rep( 0, length( Xdata[[ k ]][ , var ] ) )

    var <- "V3"
    varNew <- "V3gap"

    for( k in 1 : ( nExp - 1 ) ) {
      Xdata[[ k ]][ varNew ] <- mdata[[ k ]][ varNew ] <- Mdata[[ k ]][ varNew ] <-
        Xdata[[ k ]][ , var ] - Xdata[[ k + 1 ]][ , var ]
      Sdata[[ k ]][ varNew ] <- rep( 0, length( Xdata[[ k ]][ , var ] ) )
    }

    Xdata[[ nExp ]][ varNew ] <- mdata[[ nExp ]][ varNew ] <- Mdata[[ nExp ]][ varNew ] <-
      Xdata[[ nExp ]][ , var ] - Xdata[[ nExp - 1 ]][ , var ]
    Sdata[[ nExp ]][ varNew ] <- rep( 0, length( Xdata[[ k ]][ , var ] ) )

    plot_lists( c( "V1gap", "V2gap", "V3gap" ), Xdata, mdata, Mdata, sdMC = Sdata,
                statMC = mcStat, DCdata = DCdata, leg = legends, mask = TmaskPlot,
                nMC = nSize, CI = CI, col = colors, mrk = transMk, lty = lTypes,
                mCnt = mCnt, xlab = "Time", ylab = "Vacancy ratio gap",
                tit = "Vacancy gap", subtit = subtit,
                leg2 = c( "Primary education", "Secondary education",
                          "Tertiary education" ) )
  }


  # ------ Real wages ------

  plot_lists( "wAvgReal", Pdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, leg = legends, mask = TmaskPlot, nMC = nSize,
              CI = CI, log0 = TRUE, mrk = transMk, col = colors, lty = lTypes,
              mCnt = mCnt, xlab = "Time",
              ylab = "Log average real wage", tit = "Real wage",
              subtit = paste( subtitbase, Ptag[ "wAvgReal" ], cntLeg ) )

  # ------ Real wages by category ------

  plot_lists( c( "wAvg1real", "wAvg2real", "wAvg3real" ), Xdata, mdata, Mdata,
              cdata, Cdata, DCdata = DCdata, leg = legends, mask = TmaskPlot,
              nMC = nSize, CI = CI, log0 = TRUE, mrk = transMk, col = colors,
              lty = lTypes, mCnt = mCnt, xlab = "Time",
              ylab = "Log average real wage",
              tit = "Wages by education level", subtit = subtit,
              leg2 = c( "Primary education", "Secondary education",
                        "Tertiary education" ) )


  # ------ Wage gap by category ------

  if ( mCnt && nExp > 1 ) {

    var <- "wAvg1real"
    varNew <- "w1gap"

    for( k in 1 : ( nExp - 1 ) ) {
      Xdata[[ k ]][ varNew ] <- mdata[[ k ]][ varNew ] <- Mdata[[ k ]][ varNew ] <-
        log0( Xdata[[ k ]][ , var ] ) - log0( Xdata[[ k + 1 ]][ , var ] )
      Sdata[[ k ]][ varNew ] <- rep( 0, length( Xdata[[ k ]][ , var ] ) )
    }

    Xdata[[ nExp ]][ varNew ] <- mdata[[ nExp ]][ varNew ] <- Mdata[[ nExp ]][ varNew ] <-
      log0( Xdata[[ nExp ]][ , var ] ) - log0(  Xdata[[ nExp - 1 ]][ , var ] )
    Sdata[[ nExp ]][ varNew ] <- rep( 0, length( Xdata[[ k ]][ , var ] ) )

    var <- "wAvg2real"
    varNew <- "w2gap"

    for( k in 1 : ( nExp - 1 ) ) {
      Xdata[[ k ]][ varNew ] <- mdata[[ k ]][ varNew ] <- Mdata[[ k ]][ varNew ] <-
        log0( Xdata[[ k ]][ , var ] ) - log0( Xdata[[ k + 1 ]][ , var ] )
      Sdata[[ k ]][ varNew ] <- rep( 0, length( Xdata[[ k ]][ , var ] ) )
    }

    Xdata[[ nExp ]][ varNew ] <- mdata[[ nExp ]][ varNew ] <- Mdata[[ nExp ]][ varNew ] <-
      log0( Xdata[[ nExp ]][ , var ] ) - log0(  Xdata[[ nExp - 1 ]][ , var ] )
    Sdata[[ nExp ]][ varNew ] <- rep( 0, length( Xdata[[ k ]][ , var ] ) )

    var <- "wAvg3real"
    varNew <- "w3gap"

    for( k in 1 : ( nExp - 1 ) ) {
      Xdata[[ k ]][ varNew ] <- mdata[[ k ]][ varNew ] <- Mdata[[ k ]][ varNew ] <-
        log0( Xdata[[ k ]][ , var ] ) - log0( Xdata[[ k + 1 ]][ , var ] )
      Sdata[[ k ]][ varNew ] <- rep( 0, length( Xdata[[ k ]][ , var ] ) )
    }

    Xdata[[ nExp ]][ varNew ] <- mdata[[ nExp ]][ varNew ] <- Mdata[[ nExp ]][ varNew ] <-
      log0( Xdata[[ nExp ]][ , var ] ) - log0(  Xdata[[ nExp - 1 ]][ , var ] )
    Sdata[[ nExp ]][ varNew ] <- rep( 0, length( Xdata[[ k ]][ , var ] ) )

    plot_lists( c( "w1gap", "w2gap", "w3gap" ), Xdata, mdata, Mdata, sdMC = Sdata,
            statMC = mcStat, DCdata = DCdata, leg = legends, mask = TmaskPlot,
            nMC = nSize, CI = CI, col = colors, mrk = transMk, lty = lTypes,
            mCnt = mCnt, xlab = "Time", ylab = "Log average real wage gap",
            tit = "Wage gap", subtit = subtit,
            leg2 = c( "Primary education", "Secondary education",
                      "Tertiary education" ) )
  }


  # ------ Real wages by firm type ------

  plot_lists( c( "w2realPreChg", "w2realPosChg" ), Xdata, mdata, Mdata,
              cdata, Cdata, DCdata = DCdata, leg = legends, mask = TmaskPlot,
              nMC = nSize, CI = CI, log0 = TRUE, mrk = transMk, col = colors,
              lty = lTypes, mCnt = mCnt, xlab = "Time",
              ylab = "Log average real wage",
              tit = "Wages by firm type",
              subtit = paste( "Consumption-good sector only /", subtit ),
              leg2 = firmTypes )


  # ------ Wage, bonus and total income share in GDP terms ------

  for( k in 1 : nExp ) {
    Xdata[[ k ]][ "WY" ] <- ( Xdata[[ k ]]$W1 + Xdata[[ k ]]$W2 ) /
      Xdata[[ k ]]$Y
    mdata[[ k ]][ "WY" ] <- ( mdata[[ k ]]$W1 + mdata[[ k ]]$W2 ) /
      Xdata[[ k ]]$Y
    Mdata[[ k ]][ "WY" ] <- ( Mdata[[ k ]]$W1 + Mdata[[ k ]]$W2 ) /
      Xdata[[ k ]]$Y
    Sdata[[ k ]][ "WY" ] <- sqrt( Sdata[[ k ]]$W1 ^ 2 + Sdata[[ k ]]$W2 ^ 2 ) /
      Xdata[[ k ]]$Y
    Xdata[[ k ]][ "Bon2Y" ] <- Xdata[[ k ]]$Bon2 / Xdata[[ k ]]$Y
    mdata[[ k ]][ "Bon2Y" ] <- mdata[[ k ]]$Bon2 / Xdata[[ k ]]$Y
    Mdata[[ k ]][ "Bon2Y" ] <- Mdata[[ k ]]$Bon2 / Xdata[[ k ]]$Y
    Sdata[[ k ]][ "Bon2Y" ] <- Sdata[[ k ]]$Bon2 / Xdata[[ k ]]$Y
    Xdata[[ k ]][ "InY" ] <- Xdata[[ k ]]$In / Xdata[[ k ]]$Y
    mdata[[ k ]][ "InY" ] <- mdata[[ k ]]$In / Xdata[[ k ]]$Y
    Mdata[[ k ]][ "InY" ] <- Mdata[[ k ]]$In / Xdata[[ k ]]$Y
    Sdata[[ k ]][ "InY" ] <- Sdata[[ k ]]$In / Xdata[[ k ]]$Y
  }

  plot_lists( c( "WY", "Bon2Y", "InY" ), Xdata, mdata, Mdata, sdMC = Sdata,
              DCdata = DCdata, leg = legends, statMC = mcStat, mask = TmaskPlot,
              nMC = nSize, CI = CI, mrk = transMk, col = colors, lty = lTypes,
              mCnt = mCnt, xlab = "Time",
              ylab = "Total wages, bonus, and income over GDP",
              tit = "Wage share", subtit = subtit,
              leg2 = c( "Wages", "Bonus", "Income" ), trend = "loess" )


  # ------ Wages spread ------

  plot_lists( "wLogSD", Pdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, leg = legends, mask = TmaskPlot, nMC = nSize,
              CI = CI, mrk = transMk, col = colors, lty = lTypes, mCnt = mCnt,
              xlab = "Time", ylab = "Standard deviation of log real wage",
              tit = "Wage spread",
              subtit = paste( subtitbase, Ptag[ "wLogSD" ], cntLeg ),
              trend = "loess" )


  # ------ Gini index ------

  plot_lists( c( "Gini", "wGini" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, leg = legends, mask = TmaskPlot, nMC = nSize,
              CI = CI, mrk = transMk, col = colors, lty = lTypes, mCnt = mCnt,
              xlab = "Time", ylab = "Gini index", tit = "Gini index",
              subtit = subtit, leg2 = c( "Total income", "Wage income only" ),
              trend = "loess" )


  # ------ Average tenure and vintage skills ------

  plot_lists( c( "sTavg", "sVavg" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, leg = legends, mask = TmaskPlot, nMC = nSize,
              CI = CI, mrk = transMk, col = colors, lty = lTypes, mCnt = mCnt,
              xlab = "Time", ylab = "Average worker skills level",
              tit = "Workers' skills", subtit = subtit,
              leg2 = c( "Tenure skills", "Vintage skills" ) )


  # ------ Innovation and imitation ------

  plot_lists( c( "inn", "imi" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, leg = legends, mask = TmaskPlot, nMC = nSize,
              CI = CI, mrk = transMk, col = colors, lty = lTypes, mCnt = mCnt,
              xlab = "Time", ylab = "Share of innovating and imitating firms",
              tit = "Innovation and imitation",
              subtit = paste( "Capital-good sector only /", subtit ),
              leg2 = c( "Innovation", "Imitation" ), trend = "loess" )


  # ------ Productivity by sector ------

  plot_lists( c( "A", "A1", "A2" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, leg = legends, mask = TmaskPlot, nMC = nSize,
              CI = CI, log = TRUE, mrk = transMk, col = colors, lty = lTypes,
              mCnt = mCnt, xlab = "Time", ylab = "Average log labor productivity",
              tit = "Productivity by sector", subtit = subtit,
              leg2 = c( "Overall", "Capital-good sector", "Consumption-good sector" ) )


  # ------ Productivity gap ------

  if ( mCnt && nExp > 1 ) {

    var <- "A1"
    varNew <- "A1gap"

    for( k in 1 : ( nExp - 1 ) ) {
      Xdata[[ k ]][ varNew ] <- mdata[[ k ]][ varNew ] <- Mdata[[ k ]][ varNew ] <-
        log0( Xdata[[ k ]][ , var ] ) - log0( Xdata[[ k + 1 ]][ , var ] )
      Sdata[[ k ]][ varNew ] <- rep( 0, length( Xdata[[ k ]][ , var ] ) )
    }

    Xdata[[ nExp ]][ varNew ] <- mdata[[ nExp ]][ varNew ] <- Mdata[[ nExp ]][ varNew ] <-
      log0( Xdata[[ nExp ]][ , var ] ) - log0( Xdata[[ nExp - 1 ]][ , var ] )
    Sdata[[ nExp ]][ varNew ] <- rep( 0, length( Xdata[[ k ]][ , var ] ) )

    plot_lists( varNew, Xdata, mdata, Mdata, sdMC = Sdata, statMC = mcStat,
                DCdata = DCdata, leg = legends, mask = TmaskPlot, nMC = nSize,
                CI = CI, col = colors, mrk = transMk, lty = lTypes,
                mCnt = mCnt, xlab = "Time",
                ylab = "Capital-good log labor productivity gap",
                tit = "Productivity gap", subtit = subtit )
  }


  # ------ Productivity by firm type ------

  plot_lists( c( "A2preChg", "A2posChg" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, leg = legends, mask = TmaskPlot, nMC = nSize,
              CI = CI, log0 = TRUE, mrk = transMk, col = colors, lty = lTypes,
              mCnt = mCnt, xlab = "Time",
              ylab = "Average log labor productivity",
              tit = "Productivity by firm type", subtit = subtit,
              leg2 = firmTypes )


  # ------ Productivity spread by firm type ------

  plot_lists( c( "A2sdPreChg", "A2sdPosChg" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, leg = legends, mask = TmaskPlot, nMC = nSize,
              CI = CI, mrk = transMk, col = colors, lty = lTypes, mCnt = mCnt,
              xlab = "Time", ylab = "Standard deviation of firm log productivity",
              tit = "Productivity spread", subtit = subtit,
              leg2 = firmTypes )


  # ------ Product quality by firm type ------

  plot_lists( c( "q2preChg", "q2posChg" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, leg = legends, mask = TmaskPlot, nMC = nSize,
              CI = CI, mrk = transMk, col = colors, lty = lTypes, mCnt = mCnt,
              xlab = "Time",
              ylab = "Weighted average consumer-goods reference quality",
              tit = "Product quality", subtit = subtit,
              leg2 = firmTypes, trend = "loess" )


  # ------ Share of public firms in capital -------

  plot_lists( "f1g", Pdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, leg = legends, mask = TmaskPlot, nMC = nSize,
              CI = CI, mrk = transMk, col = colors, lty = lTypes, mCnt = mCnt,
              xlab = "Time",
              ylab = "Number of government-owned capital-good firms over total",
              tit = "Share of public capital-good firms",
              subtit = paste( subtitbase, Ptag[ "f2posChg" ], cntLeg ) )


  # ------ Share of public firms in consumption -------

  plot_lists( "f2g", Pdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, leg = legends, mask = TmaskPlot, nMC = nSize,
              CI = CI, mrk = transMk, col = colors, lty = lTypes, mCnt = mCnt,
              xlab = "Time",
              ylab = "Number of government-owned consumption-good firms over total",
              tit = "Share of public consumption-good firms",
              subtit = paste( subtitbase, Ptag[ "f2posChg" ], cntLeg ) )


  # ------ Market share of post-change firms ------

  plot_lists( "f2posChg", Pdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, leg = legends, mask = TmaskPlot, nMC = nSize,
              CI = CI, mrk = transMk, col = colors, lty = lTypes, mCnt = mCnt,
              xlab = "Time", ylab = paste( "Market share of", firmTypes[2] ),
              tit = paste( "Market share of", firmTypes[2] ),
              subtit = paste( "Consumption-good sector only /", subtitbase,
                              Ptag[ "f2posChg" ], cntLeg ) )


  # ------ Firms in the market ------

  plot_lists( c( "F2", "F1" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, leg = legends, mask = TmaskPlot, nMC = nSize,
              CI = CI, mrk = transMk, col = colors, lty = lTypes, mCnt = mCnt,
              xlab = "Time", ylab = "Number of firms",
              tit = "Firms operating in the market", subtit = subtit,
              leg2 = c( "Consumption goods", "Capital goods" ) )


  # ------ Firm entry and exit in the consumer market ------

  plot_lists( c( "entry2", "exit2" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, leg = legends, mask = TmaskPlot, nMC = nSize,
              CI = CI, mrk = transMk, col = colors, lty = lTypes, mCnt = mCnt,
              xlab = "Time", ylab = "Share of total firms",
              tit = "Firm entry and exit in the consumption-goods market",
              subtit = paste( "Consumption-good sector only /", subtit ),
              leg2 = c( "Entry", "Exit" ), trend = "loess" )


  # ------ Firms net entry trend in the market ------

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
              DCdata = DCdata, leg = legends, statMC = mcStat, mask = TmaskPlot,
              nMC = nSize, CI = CI, mrk = transMk, col = colors, lty = lTypes,
              mCnt = mCnt, xlab = "Time",
              ylab = "Number of net entrant firms (HP-filtered)",
              tit = "Net entry of firms trend", subtit = subtit,
              leg2 = c( "Capital-good sector", "Consumption-good sector" ) )


  # ------ Market-share turbulence ------

  plot_lists( c( "HP1", "HP2" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, leg = legends, mask = TmaskPlot, nMC = nSize,
              CI = CI, mrk = transMk, col = colors, lty = lTypes, mCnt = mCnt,
              xlab = "Time", ylab = "Hymer-Pashigian index",
              tit = "Market-share turbulence", subtit = subtit,
              leg2 = c( "Capital-good sector", "Consumption-good sector" ),
              trend = "loess" )


  # ------ Concentration ------

  plot_lists( c( "HH1", "HH2" ), Xdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, leg = legends, mask = TmaskPlot, nMC = nSize,
              CI = CI, mrk = transMk, col = colors, lty = lTypes, mCnt = mCnt,
              xlab = "Time", ylab = "Standardized Herfindahl-Hirschman index",
              tit = "Market concentration", subtit = subtit,
              leg2 = c( "Capital-good sector", "Consumption-good sector" ),
              trend = "loess" )


  # ------ Markup ------

  plot_lists( "mu2avg", Pdata, mdata, Mdata, cdata, Cdata,
              DCdata = DCdata, leg = legends, mask = TmaskPlot, nMC = nSize,
              CI = CI, mrk = transMk, col = colors, lty = lTypes, mCnt = mCnt,
              xlab = "Time", ylab = "Weighted average mark-up rate",
              tit = "Mark-up",
              subtit = paste( "Consumption-good sector only /", subtitbase,
                              Ptag[ "mu2avg" ], cntLeg ) )
}
