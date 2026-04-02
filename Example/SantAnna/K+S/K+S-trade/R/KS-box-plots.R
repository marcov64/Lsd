#******************************************************************
#
# ----------------- K+S aggregates box-plots --------------------
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
# !diagnostics suppress = log0, colSds, na.remove, rec.stats, textplot, saveCSV
# !diagnostics suppress = colors, radarZoom, plot_bxp_vio, plot_radar

box_plots <- function( mcData, rec.stats, mcStat, nExp, nSize, mCnt, TmaxStat,
                       TmaskStat, warmUpStat, nTstat, radarStat, radarZoom,
                       legends, listLeg, cntLeg, allLeg, colors, lTypes,
                       sDigits, bPlotCoef, bPlotNotc, folder, outDir,
                       repName, datFilSfx ) {

  radarList <- list( )
  radarList[[ 1 ]] <- list( series = c( 1, 7, 40, 55, 56, 57 ),
                            title = "Country effects" )

  # ======= COMPARISON OF EXPERIMENTS =======

  maxStats <- 99
  statsTb <- statsBp <- array( dim = c( maxStats, 5, nExp ) )
  n <- array( dim = c( maxStats, nExp ) )
  conf <- array( dim = c( maxStats, 2, nExp ) )
  data <- out <- array( list( ), dim = c( maxStats, nExp ) )
  dataMC <- array( dim = c( maxStats, nSize, nExp ) )
  temp <- matrix( nrow = TmaxStat, ncol = nSize )
  names <- units <- radars <- list( )

  # create radar plot data structure
  for( r in 1 : length( radarList ) )
    radars[[ r ]] <- matrix( nrow = nExp, ncol = length( radarList[[ r ]]$series ),
                             dimnames = list( legends,
                                              rep( "", length( radarList[[ r ]]$series ) ) ) )

  # function to add data to the comparison data structures
  addStat <- function( stat, exper, x, tit, ylab ) {

    if( stat > maxStats ) {
      warning( "Insufficient space for stats, discarding" )
      return( )
    }

    dataMC[ stat, , exper ] <<- x
    x <- na.remove( x )
    data[[ stat, exper ]] <<- x
    names[[ stat ]] <<- tit
    units[[ stat ]] <<- ylab
    statsTb[ stat, , exper ] <<- c( mean( x ), median( x ), sd( x ), min( x ), max( x ) )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsBp[ stat, , exper ] <<- bPlotStats$stats
    n[ stat, exper ] <<- bPlotStats$n
    conf[ stat, , exper ] <<- bPlotStats$conf
    out[[ stat, exper ]] <<- bPlotStats$out

    for( r in 1 : length( radarList ) ) {
      pos <- match( stat, radarList[[ r ]]$series )
      if( ! is.na( pos ) ) {
        if( exper == 1 )
          colnames( radars[[ r ]] )[ pos ] <<- tit

        radars[[ r ]][ exper, pos ] <<- statsTb[ stat, radarStat, exper ]
      }
    }

    return( stat + 1 )
  }

  # ---- Collect the data for each experiment ----

  for( k in 1 : nExp ) {
    stat <- 1

    #1
    stat <- addStat( stat, k, colMeans( mcData[[ k ]][ TmaskStat, "dY", ],
                                        na.rm = TRUE ),
                     tit = "GDP growth",
                     ylab = "Average real GDP growth rate" )
    #2
    # calculates periodic GDP growth rates for each MC series
    for( j in 1 : nSize )
      for( i in TmaskStat )
        if( i == 1 ) {
          temp[ i - warmUpStat, j ] <- 0
        } else {
          temp[ i - warmUpStat, j ] <- ( log0( mcData[[ k ]][ i, "Yreal", j ] ) -
                                           log0( mcData[[ k ]][ i - 1, "Yreal", j ] ) )
        }

    # remove +/-infinite values and replace by +/-1
    temp[ is.infinite( temp ) ] <- sign( temp[ is.infinite( temp ) ] )
    stat <- addStat( stat, k, colSds( temp, na.rm = TRUE ),
                     tit = "Volatility of GDP growth",
                     ylab = "Standard deviation of real GDP growth rate" )
    #3
    # mark crises periods (= 1) when GDP growth is less than -3%
    for( j in 1 : nSize ) {
      for( i in TmaskStat ) {
        if( i == 1 ){
          temp[ i - warmUpStat, j ] <- 0
        } else {
          if( log0( mcData[[ k ]][ i, "Yreal", j ] ) -
              log0( mcData[[ k ]][ i - 1, "Yreal", j ] ) < -0.03 ){
            temp[ i - warmUpStat, j ] <- 1
          } else {
            temp[ i - warmUpStat, j ] <- 0
          }
        }
      }
    }

    stat <- addStat( stat, k, colMeans( temp, na.rm = TRUE ),
                     tit = "Likelihood of GDP crises",
                     ylab = "Probability of GDP reductions over 3%" )
    #4
    stat <- addStat( stat, k, rec.stats[ 2, , k ],
                     tit = "Recovery from GDP crises",
                     ylab = "Average GDP crises recovery period" )
    #5
    stat <- addStat( stat, k, rec.stats[ 5, , k ],
                     tit = "Losses from GDP crises",
                     ylab = "Average GDP losses during crises recovery" )
    #6
    stat <- addStat( stat, k, colMeans( mcData[[ k ]][ TmaskStat, "Q2u", ],
                                        na.rm = TRUE ),
                     tit = "Capacity utilization",
                     ylab = "Average capacity utilization rate in consumption-good sector" )
    #7
    temp <- mcData[[ k ]][ TmaskStat, "dCPI", ]
    temp[ ! is.finite( temp ) ] <- NA
    stat <- addStat( stat, k, colMeans( temp, na.rm = TRUE ),
                     tit = "Inflation",
                     ylab = "Consumer prices index average growth rate" )
    #8
    temp <- ( mcData[[ k ]][ TmaskStat, "Xk", ] +
              mcData[[ k ]][ TmaskStat, "Xc", ] -
              mcData[[ k ]][ TmaskStat, "Mk", ] -
              mcData[[ k ]][ TmaskStat, "Mc", ] ) / mcData[[ k ]][ TmaskStat, "Y", ]
    temp[ ! is.finite( temp ) ] <- NA
    stat <- addStat( stat, k, colMeans( temp, na.rm = TRUE ),
                     tit = "Balance of trade",
                     ylab = "Exports minus imports over GDP" )
    #9
    temp <- mcData[[ k ]][ TmaskStat, "Tax", ] / mcData[[ k ]][ TmaskStat, "Y", ]
    temp[ ! is.finite( temp ) ] <- NA
    stat <- addStat( stat, k, colMeans( temp, na.rm = TRUE ),
                     tit = "Government income",
                     ylab = "Government tax income over GDP" )
    #10
    temp <- mcData[[ k ]][ TmaskStat, "G", ] / mcData[[ k ]][ TmaskStat, "Y", ]
    temp[ ! is.finite( temp ) ] <- NA
    stat <- addStat( stat, k, colMeans( temp, na.rm = TRUE ),
                     tit = "Government expenditure",
                     ylab = "Total government expenditure over GDP" )
    #11
    temp <- mcData[[ k ]][ TmaskStat, "Gc", ] / mcData[[ k ]][ TmaskStat, "Y", ]
    temp[ ! is.finite( temp ) ] <- NA
    stat <- addStat( stat, k, colMeans( temp, na.rm = TRUE ),
                     tit = "Government consumption",
                     ylab = "Total government consumption over GDP" )
    #12
    temp <- mcData[[ k ]][ TmaskStat, "Gtrain", ] / mcData[[ k ]][ TmaskStat, "Y", ]
    temp[ ! is.finite( temp ) ] <- NA
    stat <- addStat( stat, k, colMeans( temp, na.rm = TRUE ),
                     tit = "Government training expenditure",
                     ylab = "Government costs to provide training over GDP" )
    #13
    temp <- mcData[[ k ]][ TmaskStat, "Gbail", ] / mcData[[k]][ TmaskStat, "Y", ]
    temp[ ! is.finite( temp ) ] <- NA
    stat <- addStat( stat, k, colMeans( temp, na.rm = TRUE ),
                     tit = "Government bank bail-out expenditure",
                     ylab = "Government costs to bail-out banks over GDP" )
    #14
    stat <- addStat( stat, k, colMeans( mcData[[ k ]][ TmaskStat, "DefY", ],
                                        na.rm = TRUE ),
                     tit = "Government deficit",
                     ylab = "Government deficit over GDP" )
    #15
    stat <- addStat( stat, k, colMeans( mcData[[ k ]][ TmaskStat, "DefPy", ],
                                        na.rm = TRUE ),
                     tit = "Government primary deficit",
                     ylab = "Government primary deficit over GDP" )
    #16
    stat <- addStat( stat, k, colMeans( mcData[[ k ]][ TmaskStat, "DebY", ],
                                        na.rm = TRUE ),
                     tit = "Government debt",
                     ylab = "Government debt over GDP" )
    #17
    temp <- mcData[[ k ]][ TmaskStat, "TC", ] / mcData[[ k ]][ TmaskStat, "Y", ]
    temp[ ! is.finite( temp ) ] <- NA
    stat <- addStat( stat, k, colMeans( temp, na.rm = TRUE ),
                     tit = "Bank credit supply",
                     ylab = "Total bank credit available over GDP" )
    #18
    temp <- mcData[[ k ]][ TmaskStat, "Loans", ] / mcData[[ k ]][ TmaskStat, "Y", ]
    temp[ ! is.finite( temp ) ] <- NA
    stat <- addStat( stat, k, colMeans( temp, na.rm = TRUE ),
                     tit = "Firm loans",
                     ylab = "Firm debt stock over GDP" )
    #19
    temp <- mcData[[ k ]][ TmaskStat, "BadDeb", ] / mcData[[ k ]][ TmaskStat, "Y", ]
    temp[ ! is.finite( temp ) ] <- NA
    stat <- addStat( stat, k, colMeans( temp, na.rm = TRUE ),
                     tit = "Bad debt",
                     ylab = "Total bank bad debt over GDP" )
    #20
    temp <- mcData[[ k ]][ TmaskStat, "Sav", ] / mcData[[ k ]][ TmaskStat, "Y", ]
    temp[ ! is.finite( temp ) ] <- NA
    stat <- addStat( stat, k, colMeans( temp, na.rm = TRUE ),
                     tit = "Savings",
                     ylab = "Worker savings over GDP" )
    #21
    stat <- addStat( stat, k, colMeans( mcData[[ k ]][ TmaskStat, "Bda", ],
                                        na.rm = TRUE ),
                     tit = "Financial fragility",
                     ylab = "Accumulated banks bad debt over assets" )
    #22
    stat <- addStat( stat, k, colMeans( mcData[[k]][ TmaskStat, "Bfail", ],
                                        na.rm = TRUE ),
                     tit = "Bank failures",
                     ylab = "Average bank failures per period" )
    #23
    stat <- addStat( stat, k, colMeans( mcData[[ k ]][ TmaskStat, "U", ],
                                        na.rm = TRUE ),
                     tit = "Unemployment",
                     ylab = "Overal unemployment rate" )
    #24
    stat <- addStat( stat, k, colMeans( mcData[[ k ]][ TmaskStat, "Ue", ],
                                        na.rm = TRUE ),
                     tit = "Unemployment ex-discouraged",
                     ylab = "Average unemployment rate excluding discouraged workers" )
    #25
    stat <- addStat( stat, k, colMeans( mcData[[ k ]][ TmaskStat, "U1", ],
                                        na.rm = TRUE ),
                     tit = "Unemployment (primary education)",
                     ylab = "Unemployment rate for workers with primary education" )
    #26
    stat <- addStat( stat, k, colMeans( mcData[[ k ]][ TmaskStat, "U2", ],
                                        na.rm = TRUE ),
                     tit = "Unemployment (secondary education)",
                     ylab = "Unemployment rate for workers with secondary education" )
    #27
    stat <- addStat( stat, k, colMeans( mcData[[ k ]][ TmaskStat, "U3", ],
                                        na.rm = TRUE ),
                     tit = "Unemployment (tertiary education)",
                     ylab = "Unemployment rate for workers with tertiary education" )
    #28
    temp <- ( mcData[[ k ]][ TmaskStat, "W1", ] +
              mcData[[ k ]][ TmaskStat, "W2", ] ) / mcData[[ k ]][ TmaskStat, "Y", ]
    temp[ ! is.finite( temp ) ] <- NA
    stat <- addStat( stat, k, colMeans( temp, na.rm = TRUE ),
                     tit = "Wage share",
                     ylab = "Total wages over GDP" )
    #29
    temp <- mcData[[ k ]][ TmaskStat, "In", ] / mcData[[ k ]][ TmaskStat, "Y", ]
    temp[ ! is.finite( temp ) ] <- NA
    stat <- addStat( stat, k, colMeans( temp, na.rm = TRUE ),
                     tit = "Income share",
                     ylab = "Total income over GDP" )
    #30
    # Format full employment MC series (1 = full employment, 0 = otherwise)
    for( j in 1 : nSize )
      for( i in TmaskStat )
        if( mcData[[ k ]][ i, "U", j ] == 0 )
          temp[ i - warmUpStat, j ] <- 1
        else
          temp[ i - warmUpStat, j ] <- 0
    stat <- addStat( stat, k, colMeans( temp, na.rm = TRUE ),
                     tit = "Full employment frequency",
                     ylab = "Probability of zero unemployment rate" )
    #31
    stat <- addStat( stat, k, colMeans( mcData[[ k ]][ TmaskStat, "V", ],
                                        na.rm = TRUE ),
                     tit = "Vacancy",
                     ylab = "Overall Vacancy rate" )
    #32
    stat <- addStat( stat, k, colMeans( mcData[[ k ]][ TmaskStat, "Lent", ],
                                        na.rm = TRUE ),
                     tit = "Entry rate of labor",
                     ylab = "Hires over total labor force" )
    #33
    stat <- addStat( stat, k, colMeans( mcData[[ k ]][ TmaskStat, "wAvgReal", ],
                                        na.rm = TRUE ),
                     tit = "Real wage",
                     ylab = "Average log overall real wage" )
    #34
    stat <- addStat( stat, k, colMeans( mcData[[ k ]][ TmaskStat, "wAvg1real", ],
                                        na.rm = TRUE ),
                     tit = "Real wage (primary education)",
                     ylab = "Average log real wage for workers with primary education" )
    #35
    stat <- addStat( stat, k, colMeans( mcData[[ k ]][ TmaskStat, "wAvg2real", ],
                                        na.rm = TRUE ),
                     tit = "Real wage (secondary education)",
                     ylab = "Average log real wage for workers with secondary education" )
    #36
    stat <- addStat( stat, k, colMeans( mcData[[ k ]][ TmaskStat, "wAvg3real", ],
                                        na.rm = TRUE ),
                     tit = "Real wage (tertiary education)",
                     ylab = "Average log real wage for workers with tertiary education" )
    #37
    stat <- addStat( stat, k, colMeans( mcData[[ k ]][ TmaskStat, "wLogSD", ],
                                        na.rm = TRUE ),
                     tit = "Wage spread",
                     ylab = "Standard deviation of log wage" )
    #38
    temp <- mcData[[ k ]][ TmaskStat, "Bon2", ] / mcData[[ k ]][ TmaskStat, "W2", ]
    temp[ ! is.finite( temp ) ] <- NA
    stat <- addStat( stat, k, colMeans( temp, na.rm = TRUE ),
                     tit = "Bonus to wage ratio",
                     ylab = "Average bonuses over wages in consumption-good sector" )
    #39
    stat <- addStat( stat, k, colMeans( mcData[[ k ]][ TmaskStat, "wGini", ],
                                        na.rm = TRUE ),
                     tit = "Gini index (wages)",
                     ylab = "Gini index on workers' income" )
    #40
    stat <- addStat( stat, k, colMeans( mcData[[ k ]][ TmaskStat, "Gini", ],
                                        na.rm = TRUE ),
                     tit = "Gini index (all income)",
                     ylab = "Gini index on overall income" )
    #41
    stat <- addStat( stat, k, colMeans( mcData[[ k ]][ TmaskStat, "TeAvg", ],
                                        na.rm = TRUE ),
                     tit = "Worker tenure",
                     ylab = "Average employment time" )
    #42
    stat <- addStat( stat, k, colMeans( mcData[[ k ]][ TmaskStat, "sTavg", ],
                                        na.rm = TRUE ),
                     tit = "Workers tenure skills",
                     ylab = "Average worker tenure skills level" )
    #43
    stat <- addStat( stat, k, colMeans( mcData[[ k ]][ TmaskStat, "sVavg", ],
                                        na.rm = TRUE ),
                     tit = "Workers vintage skills",
                     ylab = "Average worker vintage skills level" )
    #44
    stat <- addStat( stat, k, colMeans( mcData[[ k ]][ TmaskStat, "inn", ],
                                        na.rm = TRUE ),
                     tit = "Innovation",
                     ylab = "Share of innovating firms in capital-good sector" )
    #45
    stat <- addStat( stat, k, colMeans( mcData[[ k ]][ TmaskStat, "imi", ],
                                        na.rm = TRUE ),
                     tit = "Imitation",
                     ylab = "Share of imitating firms in capital-good sector" )
    #46
    stat <- addStat( stat, k, colMeans( mcData[[ k ]][ TmaskStat, "dA", ],
                                        na.rm = TRUE ),
                     tit = "Productivity growth",
                     ylab = "Labor productivity growth rate" )
    #47
    temp <- ( 1 - mcData[[ k ]][ TmaskStat, "f2posChg", ] ) *
      mcData[[ k ]][ TmaskStat, "A2sdPreChg", ] +
            mcData[[ k ]][ TmaskStat, "f2posChg", ] *
      mcData[[ k ]][ TmaskStat, "A2sdPosChg", ]
    temp[ ! is.finite( temp ) ] <- NA
    stat <- addStat( stat, k, colMeans( temp, na.rm = TRUE ),
                     tit = "Productivity spread",
                     ylab = "Standard deviation of log productivity in consumption-good sector" )
    #48
    stat <- addStat( stat, k, colMeans( mcData[[ k ]][ TmaskStat, "q2avg", ],
                                        na.rm = TRUE ),
                     tit = "Quality",
                     ylab = "Weighted average product quality in consumption-good sector" )
    #49
    stat <- addStat( stat, k, colMeans( mcData[[ k ]][ TmaskStat, "entry1exit", ] +
                                          mcData[[ k ]][ TmaskStat, "entry2exit", ],
                                        na.rm = TRUE ),
                     tit = "Net entry of firms",
                     ylab = "Number of net entrant firms in all sectors" )
    #50
    stat <- addStat( stat, k, colMeans( mcData[[ k ]][ TmaskStat, "HP1", ],
                                        na.rm = TRUE ),
                     tit = "Market-share turbulence in capital-good sector",
                     ylab = "Hymer-Pashigian index in capital-good sector" )
    #51
    stat <- addStat( stat, k, colMeans( mcData[[ k ]][ TmaskStat, "HP2", ],
                                        na.rm = TRUE ),
                     tit = "Market-share turbulence in consumption-good sector",
                     ylab = "Hymer-Pashigian index in consumption-good sector" )
    #52
    stat <- addStat( stat, k, colMeans( mcData[[ k ]][ TmaskStat, "HH1", ],
                                        na.rm = TRUE ),
                     tit = "Market concentration in capital-good sector",
                     ylab = "Standardized Herfindahl-Hirschman index in capital-good sector" )
    #53
    stat <- addStat( stat, k, colMeans( mcData[[ k ]][ TmaskStat, "HH2", ],
                                        na.rm = TRUE ),
                     tit = "Market concentration in consumption-good sector",
                     ylab = "Standardized Herfindahl-Hirschman index in consumption-good sector" )
    #54
    stat <- addStat( stat, k, colMeans( mcData[[ k ]][ TmaskStat, "mu2avg", ],
                                        na.rm = TRUE ),
                     tit = "Mark-up in consumption-good sector",
                     ylab = "Weighted average mark-up rate in consumption-good sector" )
    #55
    stat <- addStat( stat, k, colMeans( mcData[[ k ]][ TmaskStat, "XnetY", ],
                                        na.rm = TRUE ),
                     tit = "Trade balance",
                     ylab = "Exports minus imports over GDP" )
    #56
    temp <- ( mcData[[ k ]][ TmaskStat, "Xk", ] - mcData[[ k ]][ TmaskStat, "Mk", ] ) /
            mcData[[ k ]][ TmaskStat, "S1", ]
    temp[ ! is.finite( temp ) ] <- NA
    stat <- addStat( stat, k, colMeans( temp, na.rm = TRUE ),
                     tit = "Capital-goods trade",
                     ylab = "Capital-goods exports minus imports over domestic sales" )
    #57
    temp <- ( mcData[[ k ]][ TmaskStat, "Xc", ] - mcData[[ k ]][ TmaskStat, "Mc", ] ) /
            mcData[[ k ]][ TmaskStat, "S2", ]
    temp[ ! is.finite( temp ) ] <- NA
    stat <- addStat( stat, k, colMeans( temp, na.rm = TRUE ),
                     tit = "Consumption-goods trade",
                     ylab = "Consumption-goods exports minus imports over domestic sales" )


    # multi-country report gap measures
    if( mCnt && nExp > 1 ) {
      #58
      # GDP gap among countries
      for( j in 1 : nSize )
        for( i in TmaskStat )
          if( i == 1 ) {
            temp[ i - warmUpStat, j ] <- 0
          } else {
            if( k < nExp )
              temp[ i - warmUpStat, j ] <- ( ( mcData[[ k ]][ i, "Yreal", j ] -
                                                 mcData[[ k + 1 ]][ i, "Yreal", j ] ) /
                                               mcData[[ k ]][ i, "Yreal", j ] )
            else
              temp[ i - warmUpStat, j ] <- ( ( mcData[[ k - 1 ]][ i, "Yreal", j ] -
                                                 mcData[[ k ]][ i, "Yreal", j ] ) /
                                               mcData[[ k ]][ i, "Yreal", j ] )
          }

      temp[ ! is.finite( temp ) ] <- NA
      stat <- addStat( stat, k, colSds( temp, na.rm = TRUE ),
                       tit = "GDP gap",
                       ylab = "GDP gap between countries over GDP" )
      #59
      # unemployment gap among countries
      for( j in 1 : nSize )
        for( i in TmaskStat )
          if( i == 1 ) {
            temp[ i - warmUpStat, j ] <- 0
          } else {
            if( k < nExp )
              temp[ i - warmUpStat, j ] <- ( ( mcData[[ k ]][ i, "U", j ] -
                                                 mcData[[ k + 1 ]][ i, "U", j ] ) /
                                               mcData[[ k ]][ i, "U", j ] )
            else
              temp[ i - warmUpStat, j ] <- ( ( mcData[[ k - 1 ]][ i, "U", j ] -
                                                 mcData[[ k ]][ i, "U", j ] ) /
                                               mcData[[ k ]][ i, "U", j ] )
          }

      temp[ ! is.finite( temp ) ] <- NA
      stat <- addStat( stat, k, colSds( temp, na.rm = TRUE ),
                       tit = "Unemployment gap",
                       ylab = "Unemployment rate gap over unemployment rate" )
      #60
      # vacancy gap among countries
      for( j in 1 : nSize )
        for( i in TmaskStat )
          if( i == 1 ) {
            temp[ i - warmUpStat, j ] <- 0
          } else {
            if( k < nExp )
              temp[ i - warmUpStat, j ] <- ( ( mcData[[ k ]][ i, "V", j ] -
                                                 mcData[[ k + 1 ]][ i, "V", j ] ) /
                                               mcData[[ k ]][ i, "V", j ] )
            else
              temp[ i - warmUpStat, j ] <- ( ( mcData[[ k - 1 ]][ i, "V", j ] -
                                                 mcData[[ k ]][ i, "V", j ] ) /
                                               mcData[[ k ]][ i, "V", j ] )
          }

      temp[ ! is.finite( temp ) ] <- NA
      stat <- addStat( stat, k, colSds( temp, na.rm = TRUE ),
                       tit = "Vacancy gap",
                       ylab = "Vacancy rate gap over vacancy rate" )
      #61
      # real wage gap among countries
      for( j in 1 : nSize )
        for( i in TmaskStat )
          if( i == 1 ) {
            temp[ i - warmUpStat, j ] <- 0
          } else {
            if( k < nExp )
              temp[ i - warmUpStat, j ] <- ( ( mcData[[ k ]][ i, "wAvgReal", j ] -
                                                 mcData[[ k + 1 ]][ i, "wAvgReal", j ] ) /
                                               mcData[[ k ]][ i, "wAvgReal", j ] )
            else
              temp[ i - warmUpStat, j ] <- ( ( mcData[[ k - 1 ]][ i, "wAvgReal", j ] -
                                                 mcData[[ k ]][ i, "wAvgReal", j ] ) /
                                               mcData[[ k ]][ i, "wAvgReal", j ] )
          }

      temp[ ! is.finite( temp ) ] <- NA
      stat <- addStat( stat, k, colSds( temp, na.rm = TRUE ),
                       tit = "Wage gap",
                       ylab = "Log average real wage gap over real wage" )
      #62
      # productivity gap among countries
      for( j in 1 : nSize )
        for( i in TmaskStat )
          if( i == 1 ) {
            temp[ i - warmUpStat, j ] <- 0
          } else {
            if( k < nExp )
              temp[ i - warmUpStat, j ] <- ( ( mcData[[ k ]][ i, "A1", j ] -
                                                 mcData[[ k + 1 ]][ i, "A1", j ] ) /
                                               mcData[[ k ]][ i, "A1", j ] )
            else
              temp[ i - warmUpStat, j ] <- ( ( mcData[[ k ]][ i, "A1", j ] -
                                                 mcData[[ k - 1 ]][ i, "A1", j ] ) /
                                               mcData[[ k ]][ i, "A1", j ] )
          }

      temp[ ! is.finite( temp ) ] <- NA
      stat <- addStat( stat, k, colSds( temp, na.rm = TRUE ),
                       tit = "Productivity gap",
                       ylab = "Capital-good log labor productivity gap over productivity" )
    }
  }

  # remove unused stats space
  numStats <- stat - 1
  dataMC <- dataMC[ - ( stat : maxStats ), , , drop = FALSE ]
  statsTb <- statsTb[ - ( stat : maxStats ), , , drop = FALSE ]
  statsBp <- statsBp[ - ( stat : maxStats ), , , drop = FALSE ]
  n <- n[ - ( stat : maxStats ), , drop = FALSE ]
  conf <- conf[ - ( stat : maxStats ), , , drop = FALSE ]
  out <- out[ - ( stat : maxStats ), , drop = FALSE ]
  rm( temp )


  # ---- Build experiments statistics table and performance comparison chart ----

  perf.comp <- statsTb[ , 1, 1 ]
  perf.names <- c( "Baseline[1]" )

  # Print whisker plots for each statistics

  for( stat in 1 : numStats ) {

    # find max/mins for all experiments
    lowLim <- Inf
    upLim <- -Inf
    for( k in 1 : nExp ) {
      if( conf[ stat, 1, k ] < lowLim )
        lowLim <- conf[ stat, 1, k ]
      if( conf[ stat, 2, k ] > upLim )
        upLim <- conf[ stat, 2, k ]
    }
    upLim <- upLim + ( upLim - lowLim )
    lowLim <- lowLim - ( upLim - lowLim )

    # build the outliers vectors
    outVal <- outGrp <- vector( "numeric" )
    for( k in 1 : nExp ) {
      if( length( out[[ stat, k ]] ) == 0 )
        next
      outliers <- vector( "numeric" )
      for( i in 1 : length( out[[ stat, k ]] ) ) {
        if( out[[ stat, k ]][ i ] < upLim &&
            out[[ stat, k ]][ i ] > lowLim )
          outliers <- append( outliers, out[[ stat, k ]][ i ] )
      }
      if( length( outliers ) > 0 ) {
        outVal <- append( outVal, outliers )
        outGrp <- append( outGrp, rep( k, length( outliers ) ) )
      }
    }

    if( nExp > 1 )
      listBp <- list( stats = statsBp[ stat, , ], n = n[ stat, ], conf = conf[ stat, , ],
                      out = outVal, group = outGrp, names = legends )
    else
      listBp <- list( stats = matrix( statsBp[ stat, , ] ),
                      n = matrix( n[ stat, ] ), conf = matrix( conf[ stat, , ] ),
                      out = outVal, group = outGrp, names = legends )

    title <- names[[ stat ]]
    subTitle <- paste0(
      "( bar: median / box: 2nd-3rd quartile / whiskers: max-min / points: outliers / period = ",
      warmUpStat + 1, "-", nTstat, " / MC runs = ", nSize, " ", cntLeg, " )" )
    plot_bxp_vio( dataMC[ stat, , ], leg = legends, unit = units[[ stat ]],
                  notch = bPlotNotc, tit = title, subtit = subTitle )
  }

  if( mcStat == "mean" ) {
    scol <- 1
    slab <- "Avg"
    tlab <- "t-test"
  } else {
    scol <- 2
    slab <- "Med"
    tlab <- "U-test"
  }

  table.stats <- statsTb[ , c( scol, 3, 4, 5 ), 1 ]
  table.names <- c( paste0( slab, "[1]" ), "SD[1]", "Min[1]", "Max[1]" )
  perf.comp <- statsTb[ , 1, 1 ]
  perf.names <- c( "Baseline[1]" )

  if( nExp > 1 ){

    # Create 2D stats table and performance comparison table

    for( k in 2 : nExp ){

      # Stats table
      table.stats <- cbind( table.stats, statsTb[ , c( scol, 3, 4, 5 ), k ] )
      table.names <- cbind( table.names, c( paste0( slab, "[", k, "]" ),
                                            paste0( "SD[", k, "]" ),
                                            paste0( "Min[", k, "]" ),
                                            paste0( "Max[", k, "]" ) ) )

      # Performance comparison table
      if( mcStat == "mean" ) {
        perf.comp <- cbind( perf.comp, statsTb[ , 1, k ] / statsTb[ , 1, 1 ] )

        # t-test
        t <- ( statsTb[ , 1, k ] - statsTb[ , 1, 1 ] ) /
          sqrt( ( statsTb[ , 2, k ] ^ 2 + statsTb[ , 2, 1 ] ^ 2 ) / nSize )
        df <- floor( ( ( statsTb[ , 2, k ] ^ 2 + statsTb[ , 2, 1 ] ^ 2 ) / nSize ) ^ 2 /
                       ( ( 1 / ( nSize - 1 ) ) * ( ( statsTb[ , 2, k ] ^ 2 / nSize ) ^ 2 +
                                                     ( statsTb[ , 2, 1 ] ^ 2 / nSize ) ^ 2 ) ) )
        pval <- 2 * pt( - abs ( t ), df )

      } else {
        perf.comp <- cbind( perf.comp, statsTb[ , 2, k ] / statsTb[ , 2, 1 ] )

        # U-test (Mann-Whitney-Wilcoxon)
        pval <- rep( NA, numStats )
        for( stat in 1 : numStats ) {
          pval[ stat ] <- suppressWarnings( wilcox.test( data[[ stat, k ]],
                                                         data[[ stat, 1 ]],
                                                         digits.rank = 7 )$p.value )
        }
      }

      perf.comp <- cbind( perf.comp, pval )
      perf.names <- cbind( perf.names, t( c( paste0( "Ratio[", k, "]" ),
                                             paste0( "p-val[", k, "]" ) ) ) )
    }
  }

  # Print experiments table
  colnames( table.stats ) <- table.names
  rownames( table.stats ) <- names

  textplot( formatC( table.stats, digits = sDigits, format = "g" ), cmar = 1 )
  title <- paste( "Monte Carlo descriptive statistics", allLeg )
  subTitle <- paste0( "( numbers in brackets: experiment number / period = ",
                      warmUpStat + 1, "-", nTstat, " / MC runs = ", nSize, " ",
                      cntLeg, " )" )
  title( main = title, sub = subTitle )
  mtext( listLeg, side = 1, line = -2, outer = TRUE )

  saveCSV( table.stats, baseName = repName, baseFolder = folder,
           subFolder = outDir, suffix = datFilSfx, type = "exp_stat" )

  if( nExp > 1 ) {

    # Experiments performance comparison table

    colnames( perf.comp ) <- perf.names
    rownames( perf.comp ) <- names

    textplot( formatC( perf.comp, digits = sDigits, format = "g" ), cmar = 1 )
    title <- paste( "Performance comparison", allLeg )
    subTitle <- paste0(
      "( experiment number in brackets / ", tlab,
      " H0: no difference with baseline / period =", warmUpStat + 1, "-",
      nTstat, " / MC runs = ", nSize, " ", cntLeg, " )" )
    title( main = title, sub = subTitle )
    mtext( listLeg, side = 1, line = -2, outer = TRUE )

    saveCSV( perf.comp, baseName = repName, baseFolder = folder,
             subFolder = outDir, suffix = datFilSfx, type = "exp_comp" )
  }

  # ---- Radar plots ----

  statNames <- c( "mean", "median", "std. dev.", "minimum", "maximum" )
  subTitle <- paste0( "( ", statNames[ radarStat ], " values / period = ",
                      warmUpStat + 1, "-", nTstat, " / MC runs = ", nSize,
                      " ", cntLeg, " )" )

  for( r in 1 : length( radarList ) )
    if( length( radarList[[ r ]]$series ) >= 3 )
      plot_radar( radars[[ r ]], zoom = radarZoom, dig = 2, col = colors,
                  lty = lTypes, tit = radarList[[ r ]]$title, subtit = subTitle )
}
