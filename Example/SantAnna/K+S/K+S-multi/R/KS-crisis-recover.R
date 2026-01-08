#******************************************************************
#
# ---------------- K+S crisis-recovery report -------------------
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
# !diagnostics suppress = hpfilter, textplot, saveCSV, plot_recovery

crisis_recover <- function( mcData, nExp, nSize, TmaskStat, warmUpPlot,
                            warmUpStat, nTstat, crisisLen, crisisPre, crisisTh,
                            crisisRun, legends, expLeg, cntLeg, allLeg, sDigits,
                            smoothing, transMk, mCnt, folder, outDir, repName,
                            datFilSfx ) {

  rec.stats <- array( dim = c( 5, nSize, nExp ) )
  rec.stats.mc <- array( dim = c( 2 * dim( rec.stats )[ 1 ], nExp ) )
  rec.cases <- array( data = 0, dim = c( 2 , nExp ) )
  rec.starts <- rec.times <- array( list( ), dim = c( nSize, nExp ) )

  for( k in 1 : nExp ) {          # Experiment k

    cat( "", expLeg, k, "of", nExp, "\n" )

    for( j in 1 : nSize ) {       # for each MC case

      # calculate long-term ex post growth rate trend
      growthTrend <- hpfilter( mcData[[ k ]][ , "dGDPreal", j ],
                               smoothing )$trend[ , 1 ]

      # Calculate recovery statistics for GDP to the pre-crisis trend
      recovery <- FALSE
      recStart <- recTime <- recDepth <- recCost <- vector( mode = "numeric" )
      preCrisisGDP <- gTrend <- timeCrisis <- depthCrisis <- costCrisis <- NA

      for( i in TmaskStat ) {
        if( recovery ) {                         # in a recovery?
          preCrisisGDP <- preCrisisGDP + gTrend  # update pre crisis trend

          # recovery is over?
          if( log( mcData[[ k ]][ i, "GDPreal", j ] ) >= preCrisisGDP ) {
            recovery <- FALSE

            # record only crisis lasting longer than x periods
            if( i - timeCrisis > crisisLen ) {
              recStart <- append( recStart, timeCrisis )
              recTime <- append( recTime, i - timeCrisis )
              recDepth <- append( recDepth, depthCrisis )
              recCost <- append( recCost, costCrisis )
            }
          } else {    # recovery not over

            # update statistics
            gap <- preCrisisGDP - log( mcData[[ k ]][ i, "GDPreal", j ] )
            depthCrisis <- max( depthCrisis, gap )
            costCrisis <- costCrisis + gap
          }
        }

        if( ! recovery && mcData[[ k ]][ i, "dGDPreal", j ] < crisisTh &&
            i > crisisPre ) {
          recovery <- TRUE
          timeCrisis <- i
          depthCrisis <- costCrisis <- 0
          preCrisisGDP <- log( mean( mcData[[ k ]][ ( i - crisisPre ) : ( i - 1 ),
                                                    "GDPreal", j ] ) )
          gTrend <- mean( growthTrend[ ( i - crisisPre ) : ( i - 1 ) ],
                          na.rm = TRUE )
          depthCrisis <- costCrisis <- preCrisisGDP -
            log( mcData[[ k ]][ i, "GDPreal", j ] )
        }
      }

      # prepare to consolidate MC results
      if( length( recTime ) > 0 ) {

        rec.stats[ , j, k ] <- c( length( recTime ), mean( recTime ), sd( recTime ),
                                  mean( recDepth ), mean( recCost ) )
        rec.times[[ j, k ]] <- recTime
        rec.starts[[ j, k ]] <- recStart

        # record case with more average costs crises
        rankCr <- mean( recCost )
        if( rankCr > rec.cases[ 2, k ] ) {
          rec.cases[ 1, k ] <- j
          rec.cases[ 2, k ] <- rankCr
        }

      } else {
        rec.stats[ , j, k ] <- c( rep( NA, dim( rec.stats )[ 1 ] ) )
        rec.times[[ j, k ]] <- NA
        rec.starts[[ j, k ]] <- NA
      }
    }

    # calculate average recovery statistics for the experiment
    for( i in 1 : ( dim( rec.stats )[ 1 ] ) ) {
      rec.stats.mc[ 2 * i - 1, k ] <- mean( rec.stats[ i, , k ], na.rm = TRUE )
      rec.stats.mc[ 2 * i, k ] <- sd( rec.stats[ i, , k ], na.rm = TRUE ) /
        sqrt( nSize )
    }
  }

  recovery <- matrix( c( rec.stats.mc[ 1, 1 ], rec.stats.mc[ 2, 1 ],
                         rec.stats.mc[ 3, 1 ], rec.stats.mc[ 4, 1 ],
                         rec.stats.mc[ 5, 1 ], rec.stats.mc[ 6, 1 ],
                         rec.stats.mc[ 7, 1 ], rec.stats.mc[ 8, 1 ],
                         rec.stats.mc[ 9, 1 ], rec.stats.mc[ 10, 1 ] ) )
  recovery.labels <- legends[ 1 ]

  if( nExp > 1 ){
    # Create 2D table
    for( k in 2 : nExp ) {
      recovery <- cbind( recovery, c( rec.stats.mc[ 1, k ], rec.stats.mc[ 2, k ],
                                      rec.stats.mc[ 3, k ], rec.stats.mc[ 4, k ],
                                      rec.stats.mc[ 5, k ], rec.stats.mc[ 6, k ],
                                      rec.stats.mc[ 7, k ], rec.stats.mc[ 8, k ],
                                      rec.stats.mc[ 9, k ], rec.stats.mc[ 10, k ] ) )
      recovery.labels <- append( recovery.labels, legends[ k ] )
    }
  }

  colnames( recovery ) <- recovery.labels
  rownames( recovery ) <- c( "Avg. number of crisis", "(s.e.)",
                             "Avg. recovery periods", "(s.e.)",
                             "Std. dev. rec. periods", "(s.e.)",
                             "Avg. crisis peak", "(s.e.)",
                             "Avg. crisis loss", "(s.e.)" )

  textplot( formatC( recovery, digits = sDigits, format = "g" ), cmar = 1 )
  title <- paste( "GDP long-term trend recovery after crisis", allLeg )
  subTitle <- paste( "( MC standard error in parentheses / MC runs =",
                     nSize, "/ period =", warmUpStat + 1, "-", nTstat, cntLeg,
                     ")\n", "( crisis definition: GDP contraction larger than",
                     abs( crisisTh ) * 100, "%, for more than", crisisLen,
                     "periods )\n",
                     "( pre-crisis trend estimated using an HP filter with smoothing parameter =",
                     smoothing, "averaged for", crisisPre, "periods )" )
  title( main = title, sub = subTitle )

  saveCSV( recovery, baseName = repName, num = k, baseFolder = folder,
           subFolder = outDir, suffix = datFilSfx, type = "recovery" )

  #
  # ------ GDP recovery sample-run plots ------
  #

  for( k in 1 : nExp ) {                                # for each experiment
    if( crisisRun == 0 )
      j <- rec.cases[ 1, k ]                            # auto-selected MC run
    else
      j <- crisisRun                                    # or use the user's selected

    if( j == 0 )                                        # no crisis?
      j <- 1                                            # pick first

    plot_recovery( mcData[[ k ]][ , "GDPreal", j ],
                   mcData[[ k ]][ , "dGDPreal", j ],
                   mask = TmaskStat, warm = warmUpPlot, mrk = transMk,
                   strt = rec.starts[[ j, k ]], dur = rec.times[[ j, k ]],
                   per = crisisPre, xlab = "Time",
                   ylab = "Log real (initial prices) GDP",
                   tit = paste( "GDP long-term trend recovery after crisis example (",
                                legends[ k ], ")" ),
                   subtit = paste( "( dashed line: pre-crisis trend / gray boxes: trend recovey period / MC case =",
                                   j, "/ period =", warmUpStat + 1, "-",
                                   nTstat, cntLeg, ")" ) )
  }

  #
  # ------ crisis-recover report ------
  #

  # Plot all GDP recovery runs
  for( k in 1 : nExp )                                  # for each experiment
    for( j in 1 : nSize )                               # for each MC case
      plot_recovery( mcData[[ k ]][ , "GDPreal", j ],
                     mcData[[ k ]][ , "dGDPreal", j ],
                     mask = TmaskStat, warm = warmUpPlot, mrk = transMk,
                     strt = rec.starts[[ j, k ]], dur = rec.times[[ j, k ]],
                     per = crisisPre, xlab = "Time",
                     ylab = "Log real (initial prices) GDP",
                     tit = paste( "GDP long-term trend recovery after crisis sample (",
                                  legends[ k ], ")" ),
                     subtit = paste( "( dashed line: pre-crisis trend / gray boxes: trend recovey period / MC case =",
                                     j, "/ period =", warmUpStat + 1, "-",
                                     nTstat, cntLeg, ")" ) )

  return( rec.stats )
}
