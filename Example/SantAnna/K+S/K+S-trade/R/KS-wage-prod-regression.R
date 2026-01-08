#******************************************************************
#
# -------- Wage to productivity Epanechnikov regression ---------
#
#   Written by Marcelo C. Pereira, University of Campinas
#
#   Copyright Marcelo C. Pereira
#   Distributed under the GNU General Public License
#
#   Script used by KS-sector-2.R
#   This script should not be executed directly.
#
#******************************************************************

# remove RStudio warnings for support functions
# !diagnostics suppress = plot_epanechnikov, remove_outliers_table
# !diagnostics suppress = mc, se, textplot, saveCSV, repFile

wage_prod_regr <- function( files, nExp, nSize, warmUp, nTstat, sDigits,
                            legends, expLeg, sector, csBeg, csEnd, csJump,
                            ekOrd, ekPlt, outLim, CI, folder, outDir, repName,
                            datFilSfx, raster, res, plotH, plotW, plotRows,
                            plotCols ) {

  wAregs.cols <- c( "Intercept", "Beta", "Std. error", "p-value",
                    "R2 (parametric)", "Bandwidth", "R2 (non-parametric)",
                    "Firms" )

  wAregs <- lFits <- nlFits <- list( )
  wAregs.mc <- matrix( nrow = 2 * length( wAregs.cols ), ncol = nExp )
  plot.exp <- rep( 1, nExp )
  nElem <- rep( 0, nExp )
  max.deltaR2 <- 0

  # select time steps to perform analysis
  steps <- seq( max( csBeg, warmUp ), min( csEnd, nTstat ), csJump )

  for( k in 1 : nExp ){             # do for each experiment

    cat( "", expLeg, k, "of", nExp, "\n" )

    # Perform MC regression analysis
    lFits[[ k ]] <- nlFits[[ k ]] <- list( )
    wAregs[[ k ]] <- matrix( data = NA, nrow = nSize, ncol = length( wAregs.cols ) )
    colnames( wAregs[[ k ]] ) <- wAregs.cols

    for( l in 1 : nSize ) {                           # for all MC samples (files)

      cat( "  Monte Carlo case", l, "of", nSize, "\n" )

      # load MC data from temporary files
      load( files[[ k ]]$mc[ l ] )

      lrwAvg2 <- as.vector( mc[ steps, "lrwAvg2", ] )
      lA2 <- as.vector( mc[ steps, "lA2", ] )

      rm( mc )

      data <- as.data.frame( cbind( lrwAvg2, lA2 ) )
      data <- data[ complete.cases( data ), ]         # remove incomplete data
      data <- subset( data, lA2 > 0 & lrwAvg2 > 0 )   # remove error data
      data <- remove_outliers_table( data, outLim )   # remove outliers

      if( nrow( data ) < 5 )
        next

      # linear model
      lFits[[ k ]][[ l ]] <- lm( lrwAvg2 ~ lA2, data )
      fit <- summary( lFits[[ k ]][[ l ]] )
      wAregs[[ k ]][ l, "Intercept" ] <- fit$coefficients[ 1, 1 ]
      wAregs[[ k ]][ l, "Beta" ] <- fit$coefficients[ 2, 1 ]
      wAregs[[ k ]][ l, "Std. error" ] <- fit$coefficients[ 2, 2 ]
      wAregs[[ k ]][ l, "p-value" ] <- fit$coefficients[ 2, 4 ]
      wAregs[[ k ]][ l, "R2 (parametric)" ] <- fit$r.squared

      # nonlinear model
      nlFits[[ k ]][[ l ]] <- npreg( lrwAvg2 ~ lA2, data, ckerorder = ekOrd,
                                     ckertype = "epanechnikov" )
      wAregs[[ k ]][ l, "Bandwidth" ] <- signif( nlFits[[ k ]][[ l ]]$bw, 2 )
      wAregs[[ k ]][ l, "R2 (non-parametric)" ] <- nlFits[[ k ]][[ l ]]$R2

      # number of firms considered
      nElem[ k ] <- nElem[ k ] + nrow( data )
      wAregs[[ k ]][ l, "Firms" ] <- nrow( data )

      # save less linear case for plotting
      deltaR2 <- abs( wAregs[[ k ]][ l, "R2 (non-parametric)" ] -
                        wAregs[[ k ]][ l, "R2 (parametric)" ] )
      if( is.finite( deltaR2 ) && deltaR2 > max.deltaR2 ) {
        max.deltaR2 <- deltaR2
        plot.exp[ k ] <- l
      }
    }

    # compute MC statistics
    wAregs.rows <- vector( )
    for( i in 1 : length( wAregs.cols ) ) {
      wAregs.mc[ 2 * i - 1, k ] <- mean( wAregs[[ k ]][ , i ], na.rm = TRUE )
      wAregs.mc[ 2 * i, k ] <- se( wAregs[[ k ]][ , i ], na.rm = TRUE )
      wAregs.rows <- append( wAregs.rows, c( wAregs.cols[ i ], "(s.e.)" ) )
    }
    colnames( wAregs.mc ) <- legends
    rownames( wAregs.mc ) <- wAregs.rows

    # plot selected case
    plot_epanechnikov( lFits[[ k ]][[ plot.exp[ k ] ]], ekOrd = ekOrd, CI = CI,
                       xlab = "Log productivity", ylab = "Log real average wage",
                       tit = paste( "Regression of wage on productivity (",
                                    legends[ k ], ")" ),
                       subtit = paste0( "( Sample = ",
                                        wAregs[[ k ]][ plot.exp[ k ], "Firms" ],
                                        " firms / MC run = ", plot.exp[ k ],
                                        " / Bdwdth = ",
                                        wAregs[[ k ]][ plot.exp[ k ], "Bandwidth" ],
                                        " / CI = ", CI, " / C-S = ",
                                        min( steps ), "-", max( steps ), " by ",
                                        csJump, " / ", sector, " )" ) )
  }

  # remove NaNs
  wAregs.mc[ is.nan( wAregs.mc ) ] <- NA

  # plot MC table
  textplot( formatC( wAregs.mc, digits = sDigits, format = "g" ), cmar = 1.0 )
  title <- paste( "Monte Carlo regressions summary" )
  subTitle <- paste0( "( Sample size = ", sum( nElem ), " firms / MC runs = ",
                      nSize, " / Cross-section = ", min( steps ), "-",
                      max( steps ), " by ", csJump, " / ", sector, " )" )
  title( main = title, sub = subTitle )

  saveCSV( wAregs.mc, baseName = repName, baseFolder = folder,
           subFolder = outDir, suffix = datFilSfx, type = "wAregs" )

  # ------ Save Epanechnikov regression report ------

  if( ekPlt ) {

    cat( "\nGenerating regression report...\n" )

    repFile( repName, folder = folder, suffix = paste0( datFilSfx, "_np_regr" ),
             width = plotW, height = plotH, rows = plotRows, cols = plotCols,
             raster = raster, res = res )

    # plot all regression fits
    for( k in 1 : nExp )                                  # for each experiment
      for( j in 1 : nSize )                               # for each MC case
        plot_epanechnikov( lFits[[ k ]][[ j ]], ekOrd = ekOrd, CI = CI,
                           xlab = "Log productivity",
                           ylab = "Log real average wage",
                           tit = paste( "Regression of wage on productivity (",
                                        legends[ k ], ")" ),
                           subtit = paste0( "( Sample = ",
                                            wAregs[[ k ]][ j, "Firms" ],
                                            " firms / MC run = ", j,
                                            " / Bdwdth = ",
                                            wAregs[[ k ]][ j, "Bandwidth" ],
                                            " / CI = ", CI, " / C-S = ",
                                            min( steps ), "-", max( steps ),
                                            " by ", csJump, " / ", sector, " )" ) )
  }
}
