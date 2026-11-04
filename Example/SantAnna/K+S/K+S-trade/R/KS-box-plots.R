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
# !diagnostics suppress = colSds, saveCSV, gap_country, addStat, create_comp_data
# !diagnostics suppress = plot_boxplots, plot_stat_table, plot_perf_comp
# !diagnostics suppress = plot_radars

box_plots <- function( mcData, rec.stats, nExp, mCnt, nCnt, TmaxStat, TmaskStat,
                       warmUpStat, allLeg, folder, outDir, repName, datFilSfx ) {

  radarList <- list( )
  radarList[[ 1 ]] <- list( series = c( 10, 57, 58, 1, 42, 7 ),
                            title = "Country effects" )

  # function to add data to the comparison data structures
  statFn <- function( e ) {
    stat <- 1

    #1
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "dY", ],
                                        na.rm = TRUE ),
                     tit = "GDP growth",
                     ylab = "Average real GDP growth rate" )
    #2
    stat <- addStat( stat, e, colSds( mcData[[ e ]][ TmaskStat, "dYreal", ],
                                      na.rm = TRUE ),
                     tit = "Volatility of GDP growth",
                     ylab = "Standard deviation of real GDP growth rate" )
    #3
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "tCrisis", ],
                                        na.rm = TRUE ),
                     tit = "Likelihood of GDP crises",
                     ylab = "Probability of GDP reductions over 3%" )
    #4
    stat <- addStat( stat, e, rec.stats[ 2, , e ],
                     tit = "Recovery from GDP crises",
                     ylab = "Average GDP crises recovery period" )
    #5
    stat <- addStat( stat, e, rec.stats[ 5, , e ] /
                              colMeans( mcData[[ e ]][ TmaskStat, "Y", ] ),
                     tit = "Losses from GDP crises",
                     ylab = "Average GDP losses during crises recovery over GDP" )
    #6
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "Q2u", ],
                                        na.rm = TRUE ),
                     tit = "Capacity utilization",
                     ylab = "Average capacity utilization rate in consumption-good sector" )
    #7
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "dCPI", ],
                                        na.rm = TRUE ),
                     tit = "Inflation",
                     ylab = "Consumer prices index average growth rate" )
    #8
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "C_Y", ],
                                        na.rm = TRUE ),
                     tit = "Consumption",
                     ylab = "Workers' consumption expenditure over GDP" )
    #9
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "Inom_Y", ],
                                        na.rm = TRUE ),
                     tit = "Investment",
                     ylab = "Firms' investment expenditure over GDP" )
    #10
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "XnetY", ],
                                        na.rm = TRUE ),
                     tit = "Balance of trade",
                     ylab = "Exports minus imports over GDP" )
    #11
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "Tax_Y", ],
                                        na.rm = TRUE ),
                     tit = "Government income",
                     ylab = "Government tax income over GDP" )
    #12
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "G_Y", ],
                                        na.rm = TRUE ),
                     tit = "Government expenditure",
                     ylab = "Total government expenditure over GDP" )
    #13
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "Gc_Y", ],
                                        na.rm = TRUE ),
                     tit = "Government consumption",
                     ylab = "Total government consumption over GDP" )
    #14
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "Gtrain_Y", ],
                                        na.rm = TRUE ),
                     tit = "Government training expenditure",
                     ylab = "Government costs to provide training over GDP" )
    #15
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "Gbail_Y", ],
                                        na.rm = TRUE ),
                     tit = "Government bank bail-out expenditure",
                     ylab = "Government costs to bail-out banks over GDP" )
    #16
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "DefY", ],
                                        na.rm = TRUE ),
                     tit = "Government deficit",
                     ylab = "Government deficit over GDP" )
    #17
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "DefPy", ],
                                        na.rm = TRUE ),
                     tit = "Government primary deficit",
                     ylab = "Government primary deficit over GDP" )
    #18
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "DebY", ],
                                        na.rm = TRUE ),
                     tit = "Government debt",
                     ylab = "Government debt over GDP" )
    #19
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "TC_Y", ],
                                        na.rm = TRUE ),
                     tit = "Bank credit supply",
                     ylab = "Total bank credit available over GDP" )
    #20
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "Loans_Y", ],
                                        na.rm = TRUE ),
                     tit = "Firm loans",
                     ylab = "Firm debt stock over GDP" )
    #21
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "BadDeb_Y", ],
                                        na.rm = TRUE ),
                     tit = "Bad debt",
                     ylab = "Total bank bad debt over GDP" )
    #22
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "Sav_Y", ],
                                        na.rm = TRUE ),
                     tit = "Savings",
                     ylab = "Worker savings over GDP" )
    #23
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "Bda", ],
                                        na.rm = TRUE ),
                     tit = "Financial fragility",
                     ylab = "Accumulated banks bad debt over assets" )
    #24
    stat <- addStat( stat, e, colMeans( mcData[[e]][ TmaskStat, "Bfail", ],
                                        na.rm = TRUE ),
                     tit = "Bank failures",
                     ylab = "Average bank failures per period" )
    #25
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "U", ],
                                        na.rm = TRUE ),
                     tit = "Unemployment",
                     ylab = "Overal unemployment rate" )
    #26
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "Ue", ],
                                        na.rm = TRUE ),
                     tit = "Unemployment ex-discouraged",
                     ylab = "Average unemployment rate excluding discouraged workers" )
    #27
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "U1", ],
                                        na.rm = TRUE ),
                     tit = "Unemployment (primary education)",
                     ylab = "Unemployment rate for workers with primary education" )
    #28
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "U2", ],
                                        na.rm = TRUE ),
                     tit = "Unemployment (secondary education)",
                     ylab = "Unemployment rate for workers with secondary education" )
    #29
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "U3", ],
                                        na.rm = TRUE ),
                     tit = "Unemployment (tertiary education)",
                     ylab = "Unemployment rate for workers with tertiary education" )
    #30
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "W_Y", ],
                                        na.rm = TRUE ),
                     tit = "Wage share",
                     ylab = "Total wages over GDP" )
    #31
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "In_Y", ],
                                        na.rm = TRUE ),
                     tit = "Income share",
                     ylab = "Total income over GDP" )
    #32
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "Uzero", ],
                                        na.rm = TRUE ),
                     tit = "Full employment frequency",
                     ylab = "Probability of < 1% unemployment rate" )
    #33
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "V", ],
                                        na.rm = TRUE ),
                     tit = "Vacancy",
                     ylab = "Overall Vacancy rate" )
    #34
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "Lent", ],
                                        na.rm = TRUE ),
                     tit = "Entry rate of labor",
                     ylab = "Hires over total labor force" )
    #35
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "wAvgReal", ],
                                        na.rm = TRUE ),
                     tit = "Real wage",
                     ylab = "Average log overall real wage" )
    #36
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "wAvg1real", ],
                                        na.rm = TRUE ),
                     tit = "Real wage (primary education)",
                     ylab = "Average log real wage for workers with primary education" )
    #37
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "wAvg2real", ],
                                        na.rm = TRUE ),
                     tit = "Real wage (secondary education)",
                     ylab = "Average log real wage for workers with secondary education" )
    #38
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "wAvg3real", ],
                                        na.rm = TRUE ),
                     tit = "Real wage (tertiary education)",
                     ylab = "Average log real wage for workers with tertiary education" )
    #39
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "wLogSD", ],
                                        na.rm = TRUE ),
                     tit = "Wage spread",
                     ylab = "Standard deviation of log wage" )
    #40
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "BonW2", ],
                                        na.rm = TRUE ),
                     tit = "Bonus to wage ratio",
                     ylab = "Average bonuses over wages in consumption-good sector" )
    #41
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "wGini", ],
                                        na.rm = TRUE ),
                     tit = "Gini index (wages)",
                     ylab = "Gini index on workers' income" )
    #42
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "Gini", ],
                                        na.rm = TRUE ),
                     tit = "Gini index (all income)",
                     ylab = "Gini index on overall income" )
    #43
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "TeAvg", ],
                                        na.rm = TRUE ),
                     tit = "Worker tenure",
                     ylab = "Average employment time" )
    #44
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "sTavg", ],
                                        na.rm = TRUE ),
                     tit = "Workers tenure skills",
                     ylab = "Average worker tenure skills level" )
    #45
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "sVavg", ],
                                        na.rm = TRUE ),
                     tit = "Workers vintage skills",
                     ylab = "Average worker vintage skills level" )
    #46
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "inn", ],
                                        na.rm = TRUE ),
                     tit = "Innovation",
                     ylab = "Share of innovating firms in capital-good sector" )
    #47
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "imi", ],
                                        na.rm = TRUE ),
                     tit = "Imitation",
                     ylab = "Share of imitating firms in capital-good sector" )
    #48
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "dA", ],
                                        na.rm = TRUE ),
                     tit = "Productivity growth",
                     ylab = "Labor productivity growth rate" )
    #49
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "A2sd", ],
                                        na.rm = TRUE ),
                     tit = "Productivity spread",
                     ylab = "Standard deviation of log productivity in consumption-good sector" )
    #50
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "q2avg", ],
                                        na.rm = TRUE ),
                     tit = "Quality",
                     ylab = "Weighted average product quality in consumption-good sector" )
    #51
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "entry1exit", ] +
                                          mcData[[ e ]][ TmaskStat, "entry2exit", ],
                                        na.rm = TRUE ),
                     tit = "Net entry of firms",
                     ylab = "Number of net entrant firms in all sectors" )
    #52
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "HP1", ],
                                        na.rm = TRUE ),
                     tit = "Market-share turbulence in capital-good sector",
                     ylab = "Hymer-Pashigian index in capital-good sector" )
    #53
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "HP2", ],
                                        na.rm = TRUE ),
                     tit = "Market-share turbulence in consumption-good sector",
                     ylab = "Hymer-Pashigian index in consumption-good sector" )
    #54
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "HH1", ],
                                        na.rm = TRUE ),
                     tit = "Market concentration in capital-good sector",
                     ylab = "Standardized Herfindahl-Hirschman index in capital-good sector" )
    #55
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "HH2", ],
                                        na.rm = TRUE ),
                     tit = "Market concentration in consumption-good sector",
                     ylab = "Standardized Herfindahl-Hirschman index in consumption-good sector" )
    #56
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "mu2avg", ],
                                        na.rm = TRUE ),
                     tit = "Mark-up in consumption-good sector",
                     ylab = "Weighted average mark-up rate in consumption-good sector" )
    #57
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "XkNet_Y", ],
                                        na.rm = TRUE ),
                     tit = "Capital-good trade",
                     ylab = "Capital-good exports minus imports over GDP" )
    #58
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "XcNet_Y", ],
                                        na.rm = TRUE ),
                     tit = "Consumption-good trade",
                     ylab = "Consumption-good exports minus imports over GDP" )

    #59
    stat <- addStat( stat, e, colMeans( mcData[[ e ]][ TmaskStat, "e", ],
                                        na.rm = TRUE ),
                     tit = "Exchange rate",
                     ylab = "Nominal exchange rate relative to int'l currency" )


    # multi-country report gap measures
    if( mCnt && nExp > 1 ) {
      #60
      stat <- addStat( stat, e, colSds( gap_country( mcData, "Yreal", e ),
                                        na.rm = TRUE ),
                       tit = "GDP gap",
                       ylab = "GDP gap between countries over GDP" )
      #61
      stat <- addStat( stat, e, colSds( gap_country( mcData, "U", e ),
                                        na.rm = TRUE ),
                       tit = "Unemployment gap",
                       ylab = "Unemployment rate gap over unemployment rate" )
      #62
      stat <- addStat( stat, e, colSds( gap_country( mcData, "V", e ),
                                        na.rm = TRUE ),
                       tit = "Vacancy gap",
                       ylab = "Vacancy rate gap over vacancy rate" )
      #63
      stat <- addStat( stat, e, colSds( gap_country( mcData, "wAvgReal", e ),
                                        na.rm = TRUE ),
                       tit = "Wage gap",
                       ylab = "Log average real wage gap over real wage" )
      #64
      stat <- addStat( stat, e, colSds( gap_country( mcData, "A1", e ),
                                        na.rm = TRUE ),
                       tit = "Productivity gap",
                       ylab = "Capital-good log labor productivity gap over productivity" )

    }

    return( stat - 1 )
  }

  # create data structures for analysis
  data <- create_comp_data( radarList, statFn )

  # print whisker plots for each statistics
  plot_boxplots( data )

  # print radar plots
  plot_radars( data, radarList )

  # print experiments table
  tab <- plot_stat_table( data, paste( "Monte Carlo descriptive statistics",
                                       allLeg ) )

  saveCSV( tab, baseName = repName, baseFolder = folder, subFolder = outDir, 
           nCnt = nCnt, suffix = datFilSfx, type = "exp_stat" )

  # print performance comparison table
  if( nExp > 1 ) {
    tab <- plot_perf_comp( data, paste( "Performance comparison", allLeg ) )
  
    saveCSV( tab, baseName = repName, baseFolder = folder, subFolder = outDir,
             nCnt = nCnt, suffix = datFilSfx, type = "exp_comp" )
  }
}
