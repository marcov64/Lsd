#******************************************************************
#
# ------ Dosi and Nelson (2010) productivity decomposition ------
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
# !diagnostics suppress = mc, all.NA, se, textplot, saveCSV

DN_decomp <- function( nExp, nSize, nElem, csBeg, csEnd, fShare, files,
                       sDigits, legends, expLeg, sector, folder, outDir,
                       repName, datFilSfx, firmTypes ) {

  errThr <- 1e-10             # rounding error threshold

  prdLab <- c( "A2.1", "A2", "A2.pre.1", "A2.pre", "A2.pos.1", "A2.pos",
               "A2.ent", "A2.ext.1", "A2.inc.1", "A2.inc", "A2.inc.pre.1",
               "A2.inc.pre", "A2.inc.pos.1", "A2.inc.pos", "delta.A2",
               "delta.A2.pre", "delta.A2.pos", "delta.A2.inc",
               "delta.A2.inc.pre", "delta.A2.inc.pos", "B.pre", "B.pos",
               "W.pre", "W.pos", "C.pre", "C.pos", "N.pre", "N.pos", "X.pre",
               "X.pos", "sum.dec", "dec.err", "dec.err.pre", "dec.err.pos" )

  if( fShare == "f2e" )
    shType = "output"
  else
    shType = "labor"

  prdDec <- prdDecTot <- list( )
  bars.tot <- bars.pre <- bars.pos <- matrix( nrow = 4, ncol = 0 )

  for( k in 1 : nExp ){             # do for each experiment

    cat( "", expLeg, k, "of", nExp, "\n" )

    prdDec[[ k ]] <- list( )
    prdDecTot[[ k ]] <- data.frame( matrix( nrow = nSize,
                                            ncol = length( prdLab ) ) )
    rownames( prdDecTot[[ k ]] ) <- 1 : nSize
    colnames( prdDecTot[[ k ]] ) <- prdLab

    nElem[ k ] <- 0

    # Calculate the decomposition period-by-period for all MC runs
    for( l in 1 : nSize ) {                        # for all MC samples (files)

      cat( "  Monte Carlo case", l, "of", nSize, "\n" )

      # load MC data from temporary files
      load( files[[ k ]]$mc[ l ] )

      # create the decomposition matrix for the experiment
      nPer <- csEnd - csBeg + 1
      prdDec[[ k ]][[ l ]] <- data.frame( matrix( nrow = nPer,
                                                  ncol = length( prdLab ) ) )
      rownames( prdDec[[ k ]][[ l ]] ) <- ( csBeg + 1 ) : ( csEnd + 1 )
      colnames( prdDec[[ k ]][[ l ]] ) <- prdLab

      for( i in ( csBeg + 1 ) : ( csEnd + 1 ) ) {  # all time steps

        # concatenate data from t-k and t
        cols <- c( fShare, "A2d", "post2chg" )
        fd <- t( mc[ i, cols, ] )
        fd.1 <- t( mc[ max( i - 1, 1 ), cols, ] )
        cols <- c( "f2", "A2", "post2chg" )
        colnames( fd ) <- cols
        colnames( fd.1 ) <- cols
        fd <- data.frame( cbind( fd, fd.1 ) )

        # remove lines without data
        fd <- fd[ ! all.NA( fd ), , drop = FALSE ]

        # remove lines with incomplete data (incumbents with zero production)
        toRemove <- vector( "numeric" )
        for( j in 1 : nrow ( fd ) ) {
          # firms without data in both periods
          if( ( is.na( fd$f2[ j ] ) && is.na( fd$A2[ j ] ) &&
                is.na( fd$f2.1[ j ] ) && is.na( fd$A2.1[ j ] ) ) ||
              ( is.na( fd$A2[ j ] ) && is.na( fd$A2.1[ j ] ) ) )
            toRemove <- append( toRemove, j )
          # non-exiting firm with workers but no production
        }
        if( length( toRemove ) > 0 )
          fd <- fd[ - toRemove, , drop = FALSE ]

        # classify firms considering the one entering/exiting during the window
        for( j in 1 : nrow( fd ) ) {
          if( ( is.na( fd$post2chg[ j ] ) && is.na( fd$post2chg.1[ j ] ) ) ||
              ( ( ! is.na( fd$post2chg[ j ] ) && ! is.na( fd$post2chg.1[ j ] ) )
                && fd$post2chg[ j ] != fd$post2chg.1[ j ] ) ||
              ( is.na( fd$f2[ j ] ) && is.na( fd$f2.1[ j ] ) ) )
            stop( "Firm data matrix is inconsistent" )

          if( ( is.na( fd$post2chg[ j ] ) ) )
            fd$post2chg[ j ] <- fd$post2chg.1[ j ]

          if( ! is.na( fd$f2[ j ] ) && ! is.na( fd$f2.1[ j ] ) )
            fd$type[ j ] <- 2              # incumbent firm
          if( is.na( fd$f2[ j ] ) )
            fd$type[ j ] <- 3              # exiting firm
          if( is.na( fd$f2.1[ j ] ) )
            fd$type[ j ] <- 1              # entrant firm

          # remove market share of non-producing firms
          if( is.na( fd$A2[ j ] ) )
            fd$f2[ j ] <- NA
          if( is.na( fd$A2.1[ j ] ) )
            fd$f2.1[ j ] <- NA
        }
        fd <- fd[ , ! ( names( fd ) %in% c( "post2chg.1" ) ) ]

        # number of firms considered
        nElem[ k ] <- nElem[ k ] + nrow( fd )

        # recalculate market shares to add up to 1
        sum.f2 <- sum( fd$f2, na.rm = TRUE )
        fd$f2 <- fd$f2 / sum.f2
        sum.f2.1 <- sum( fd$f2.1, na.rm = TRUE )
        fd$f2.1 <- fd$f2.1 / sum.f2.1

        # add delta data
        fd$delta.f2 <- fd$f2 - fd$f2.1
        fd$delta.A2 <- fd$A2 - fd$A2.1

        j <- i - csBeg

        # weighted average productivity and growth

        # all firms
        prdDec[[ k ]][[ l ]]$A2.1[ j ] <- A2.1 <-
          sum( fd$f2.1 * fd$A2.1, na.rm = TRUE )     # average in t-1
        prdDec[[ k ]][[ l ]]$A2[ j ] <- A2 <-
          sum( fd$f2 * fd$A2, na.rm = TRUE )         # average in t
        prdDec[[ k ]][[ l ]]$delta.A2[ j ] <- ( A2 - A2.1 ) / A2.1  # % growth

        # pre-change firms
        prdDec[[ k ]][[ l ]]$A2.pre.1[ j ] <- A2.pre.1 <-
          sum( fd$f2.1[ fd$post2chg == 0 ] * fd$A2.1[ fd$post2chg == 0 ],
               na.rm = TRUE )
        prdDec[[ k ]][[ l ]]$A2.pre[ j ] <- A2.pre <-
          sum( fd$f2[ fd$post2chg == 0 ] * fd$A2[ fd$post2chg == 0 ], na.rm = TRUE )
        prdDec[[ k ]][[ l ]]$delta.A2.pre[ j ] <- ( A2.pre - A2.pre.1 ) / A2.1

        # post-change firms
        prdDec[[ k ]][[ l ]]$A2.pos.1[ j ] <- A2.pos.1 <-
          sum( fd$f2.1[ fd$post2chg == 1 ] * fd$A2.1[ fd$post2chg == 1 ], na.rm = TRUE )
        prdDec[[ k ]][[ l ]]$A2.pos[ j ] <- A2.pos <-
          sum( fd$f2[ fd$post2chg == 1 ] * fd$A2[ fd$post2chg == 1 ], na.rm = TRUE )
        prdDec[[ k ]][[ l ]]$delta.A2.pos[ j ] <- ( A2.pos - A2.pos.1 ) / A2.1

        # entrant firms
        prdDec[[ k ]][[ l ]]$A2.ent[ j ] <-
          sum( fd$f2[ fd$type == 1 ] * fd$A2[ fd$type == 1 ], na.rm = TRUE )  # t only

        # exiting firms
        prdDec[[ k ]][[ l ]]$A2.ext.1[ j ] <-
          sum( fd$f2.1[ fd$type == 3 ] * fd$A2.1[ fd$type == 3 ], na.rm = TRUE )  # t-1 only

        # incumbent (continuing) firms
        prdDec[[ k ]][[ l ]]$A2.inc.1[ j ] <- A2.inc.1 <-
          sum( fd$f2.1[ fd$type == 2 ] * fd$A2.1[ fd$type == 2 ], na.rm = TRUE )
        prdDec[[ k ]][[ l ]]$A2.inc[ j ] <- A2.inc <-
          sum( fd$f2[ fd$type == 2 ] * fd$A2[ fd$type == 2 ], na.rm = TRUE )
        prdDec[[ k ]][[ l ]]$delta.A2.inc[ j ] <- ( A2.inc - A2.inc.1 ) / A2.1

        # incumbent (continuing) pre-change firms
        prdDec[[ k ]][[ l ]]$A2.inc.pre.1[ j ] <- A2.inc.pre.1 <-
          sum( fd$f2.1[ fd$post2chg == 0 & fd$type == 2 ] *
                 fd$A2.1[ fd$post2chg == 0 & fd$type == 2 ], na.rm = TRUE )
        prdDec[[ k ]][[ l ]]$A2.inc.pre[ j ] <- A2.inc.pre <-
          sum( fd$f2[ fd$post2chg == 0 & fd$type == 2 ] *
                 fd$A2[ fd$post2chg == 0 & fd$type == 2 ], na.rm = TRUE )
        prdDec[[ k ]][[ l ]]$delta.A2.inc.pre[ j ] <- ( A2.inc.pre - A2.inc.pre.1 ) / A2.1

        # incumbent (continuing) post-change firms
        prdDec[[ k ]][[ l ]]$A2.inc.pos.1[ j ] <- A2.inc.pos.1 <-
          sum( fd$f2.1[ fd$post2chg == 1 & fd$type == 2 ] *
                 fd$A2.1[ fd$post2chg == 1 & fd$type == 2 ], na.rm = TRUE )
        prdDec[[ k ]][[ l ]]$A2.inc.pos[ j ] <- A2.inc.pos <-
          sum( fd$f2[ fd$post2chg == 1 & fd$type == 2 ] *
                 fd$A2[ fd$post2chg == 1 & fd$type == 2 ], na.rm = TRUE )
        prdDec[[ k ]][[ l ]]$delta.A2.inc.pos[ j ] <- ( A2.inc.pos - A2.inc.pos.1 ) / A2.1


        # do the decomposition of productivity % growth

        # incumbents within components
        prdDec[[ k ]][[ l ]]$W.pre[ j ] <-          # pre-change firms
          sum( fd$f2.1[ fd$type == 2 & fd$post2chg == 0 ] *
                 fd$delta.A2[ fd$type == 2 & fd$post2chg == 0 ],
               na.rm = TRUE ) / A2.1
        prdDec[[ k ]][[ l ]]$W.pos[ j ] <-          # post-change firms
          sum( fd$f2.1[ fd$type == 2 & fd$post2chg == 1 ] *
                 fd$delta.A2[ fd$type == 2 & fd$post2chg == 1 ],
               na.rm = TRUE ) / A2.1

        # incumbents between components
        prdDec[[ k ]][[ l ]]$B.pre[ j ] <-
          sum( fd$A2.1[ fd$type == 2 & fd$post2chg == 0 ] *
                 fd$delta.f2[ fd$type == 2 & fd$post2chg == 0 ],
               na.rm = TRUE ) / A2.1
        prdDec[[ k ]][[ l ]]$B.pos[ j ] <-
          sum( fd$A2.1[ fd$type == 2 & fd$post2chg == 1 ] *
                 fd$delta.f2[ fd$type == 2 & fd$post2chg == 1 ],
               na.rm = TRUE ) / A2.1

        # incumbents interaction terms
        prdDec[[ k ]][[ l ]]$C.pre[ j ] <-
          sum( fd$delta.f2[ fd$type == 2 & fd$post2chg == 0 ] *
                 fd$delta.A2[ fd$type == 2 & fd$post2chg == 0 ],
               na.rm = TRUE ) / A2.1
        prdDec[[ k ]][[ l ]]$C.pos[ j ] <-
          sum( fd$delta.f2[ fd$type == 2 & fd$post2chg == 1 ] *
                 fd$delta.A2[ fd$type == 2 & fd$post2chg == 1 ],
               na.rm = TRUE ) / A2.1

        # entrants components
        prdDec[[ k ]][[ l ]]$N.pre[ j ] <-
          sum( fd$f2[ fd$type == 1 & fd$post2chg == 0 ] *
                 fd$A2[ fd$type == 1 & fd$post2chg == 0 ],
               na.rm = TRUE ) / A2.1
        prdDec[[ k ]][[ l ]]$N.pos[ j ] <-
          sum( fd$f2[ fd$type == 1 & fd$post2chg == 1 ] *
                 fd$A2[ fd$type == 1 & fd$post2chg == 1 ],
               na.rm = TRUE ) / A2.1

        # exiting firms components
        prdDec[[ k ]][[ l ]]$X.pre[ j ] <-
          sum( fd$f2.1[ fd$type == 3 & fd$post2chg == 0 ] *
                 fd$A2.1[ fd$type == 3 & fd$post2chg == 0 ],
               na.rm = TRUE ) / A2.1
        prdDec[[ k ]][[ l ]]$X.pos[ j ] <-
          sum( fd$f2.1[ fd$type == 3 & fd$post2chg == 1 ] *
                 fd$A2.1[ fd$type == 3 & fd$post2chg == 1 ],
               na.rm = TRUE ) / A2.1

        # calculate decomposition errors
        prdDec[[ k ]][[ l ]]$sum.dec[ j ] <- prdDec[[ k ]][[ l ]]$B.pre[ j ] +
                                             prdDec[[ k ]][[ l ]]$B.pos[ j ] +
                                             prdDec[[ k ]][[ l ]]$W.pre[ j ] +
                                             prdDec[[ k ]][[ l ]]$W.pos[ j ] +
                                             prdDec[[ k ]][[ l ]]$C.pre[ j ] +
                                             prdDec[[ k ]][[ l ]]$C.pos[ j ] +
                                             prdDec[[ k ]][[ l ]]$N.pre[ j ] +
                                             prdDec[[ k ]][[ l ]]$N.pos[ j ] -
                                             prdDec[[ k ]][[ l ]]$X.pre[ j ] -
                                             prdDec[[ k ]][[ l ]]$X.pos[ j ]
        prdDec[[ k ]][[ l ]]$dec.err[ j ] <- prdDec[[ k ]][[ l ]]$delta.A2[ j ] -
                                             prdDec[[ k ]][[ l ]]$sum.dec[ j ]
        prdDec[[ k ]][[ l ]]$dec.err.pre[ j ] <- prdDec[[ k ]][[ l ]]$delta.A2.pre[ j ] -
                                                 prdDec[[ k ]][[ l ]]$B.pre[ j ] -
                                                 prdDec[[ k ]][[ l ]]$W.pre[ j ] -
                                                 prdDec[[ k ]][[ l ]]$C.pre[ j ] -
                                                 prdDec[[ k ]][[ l ]]$N.pre[ j ] +
                                                 prdDec[[ k ]][[ l ]]$X.pre[ j ]
        prdDec[[ k ]][[ l ]]$dec.err.pos[ j ] <- prdDec[[ k ]][[ l ]]$delta.A2.pos[ j ] -
                                                 prdDec[[ k ]][[ l ]]$B.pos[ j ] -
                                                 prdDec[[ k ]][[ l ]]$W.pos[ j ] -
                                                 prdDec[[ k ]][[ l ]]$C.pos[ j ] -
                                                 prdDec[[ k ]][[ l ]]$N.pos[ j ] +
                                                 prdDec[[ k ]][[ l ]]$X.pos[ j ]

        # discard rounding errors
        if( prdDec[[ k ]][[ l ]]$dec.err[ j ] < errThr )
          prdDec[[ k ]][[ l ]]$dec.err[ j ] <- 0
        if( prdDec[[ k ]][[ l ]]$dec.err.pre[ j ] < errThr )
          prdDec[[ k ]][[ l ]]$dec.err.pre[ j ] <- 0
        if( prdDec[[ k ]][[ l ]]$dec.err.pos[ j ] < errThr )
          prdDec[[ k ]][[ l ]]$dec.err.pos[ j ] <- 0
      }

      prdDecTot[[ k ]][ l, ] <- colMeans( prdDec[[ k ]][[ l ]], na.rm = TRUE )
    }

    rm( mc )

    # Productivity decomposition table

    prdDec.stats <- matrix( c( mean( prdDecTot[[ k ]]$delta.A2, na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$W.pre + prdDecTot[[ k ]]$W.pos,
                                     na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$B.pre + prdDecTot[[ k ]]$B.pos,
                                     na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$C.pre + prdDecTot[[ k ]]$C.pos,
                                     na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$N.pre + prdDecTot[[ k ]]$N.pos -
                                       prdDecTot[[ k ]]$X.pre - prdDecTot[[ k ]]$X.pos,
                                     na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$dec.err, na.rm = TRUE ),

                               se( prdDecTot[[ k ]]$delta.A2 ),
                               se( prdDecTot[[ k ]]$W.pre + prdDecTot[[ k ]]$W.pos ),
                               se( prdDecTot[[ k ]]$B.pre + prdDecTot[[ k ]]$B.pos ),
                               se( prdDecTot[[ k ]]$C.pre + prdDecTot[[ k ]]$C.pos ),
                               se( prdDecTot[[ k ]]$N.pre + prdDecTot[[ k ]]$N.pos -
                                     prdDecTot[[ k ]]$X.pre - prdDecTot[[ k ]]$X.pos ),
                               se( prdDecTot[[ k ]]$dec.err ),

                               mean( prdDecTot[[ k ]]$delta.A2.pre, na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$W.pre, na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$B.pre, na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$C.pre, na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$N.pre - prdDecTot[[ k ]]$X.pre,
                                     na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$dec.err.pre, na.rm = TRUE ),

                               se( prdDecTot[[ k ]]$delta.A2.pre ),
                               se( prdDecTot[[ k ]]$W.pre ),
                               se( prdDecTot[[ k ]]$B.pre ),
                               se( prdDecTot[[ k ]]$C.pre ),
                               se( prdDecTot[[ k ]]$N.pre - prdDecTot[[ k ]]$X.pre ),
                               se( prdDecTot[[ k ]]$dec.err.pre ),

                               mean( prdDecTot[[ k ]]$delta.A2.pos, na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$W.pos, na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$B.pos, na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$C.pos, na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$N.pos - prdDecTot[[ k ]]$X.pos,
                                     na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$dec.err.pos, na.rm = TRUE ),

                               se( prdDecTot[[ k ]]$delta.A2.pos ),
                               se( prdDecTot[[ k ]]$W.pos ),
                               se( prdDecTot[[ k ]]$B.pos ),
                               se( prdDecTot[[ k ]]$C.pos ),
                               se( prdDecTot[[ k ]]$N.pos - prdDecTot[[ k ]]$X.pos ),
                               se( prdDecTot[[ k ]]$dec.err.pos ) ),
                            ncol = 6, byrow = TRUE )
    colnames( prdDec.stats ) <- c( "Total", "Within", "Between",
                                   "Cross", "Net entry", "Error" )
    rownames( prdDec.stats ) <- c( "Overall", "(s.e.)", firmTypes[1],
                                   "(s.e.)", firmTypes[2], "(s.e.)" )

    # remove NaNs
    prdDec.stats[ is.nan( prdDec.stats ) ] <- 0

    # absolute productivity growth log values table
    textplot( formatC( prdDec.stats, digits = sDigits, format = "g" ), cmar = 1.0 )
    title <- paste( "DN decomposition of labor productivity growth (", legends[ k ], ")" )
    subTitle <- paste( paste0( "( Share = ", shType, " / sample size = ",
                               nElem[ k ], " firms / MC runs = ", nSize,
                               " / Period = ", csBeg + 1, "-", csEnd + 1, " )" ),
                       paste( "(", sector, ")" ), sep ="\n" )
    title( main = title, sub = subTitle )

    saveCSV( prdDec.stats, baseName = repName, num = k, baseFolder = folder,
             subFolder = outDir, suffix = datFilSfx, type = "DN_dec" )

    # add data to global chart (as % of total productivity growth)
    bars.tot <- cbind( bars.tot, c( prdDec.stats[ 1, 2 : 5 ] ) )
    bars.pre <- cbind( bars.pre, c( prdDec.stats[ 3, 2 : 5 ] ) )
    bars.pos <- cbind( bars.pos, c( prdDec.stats[ 5, 2 : 5 ] ) )
  }

  # name the bar groups
  colnames( bars.tot ) <- legends
  colnames( bars.pre ) <- legends
  colnames( bars.pos ) <- legends


  # Plot the bar graphs

  barplot( bars.tot, beside = TRUE, legend.text = TRUE, axisnames = TRUE, xpd = FALSE,
           ylab = "Labor productivity growth rate", ylim = round( range( bars.tot ) * 1.5, 2 ) )
  title <- paste( "DN decomposition of productivity growth ( all firms )" )
  subTitle <- paste( paste0( "( Share = ", shType, " / MC runs = ", nSize,
                             " / Period = ", csBeg + 1, "-", csEnd + 1, " )" ),
                     paste( "(", sector, ")" ), sep ="\n" )
  title( main = title, sub = subTitle )

  barplot( bars.pre, beside = TRUE, legend.text = TRUE, axisnames = TRUE, xpd = FALSE,
           ylab = "Labor productivity growth rate", ylim = round( range( bars.pre ) * 1.5, 2 ) )
  title <- paste( "DN decomposition of productivity growth ( pre-change firms )" )
  subTitle <- paste( paste0( "( Share = ", shType, " / MC runs = ", nSize,
                             " / Period = ", csBeg + 1, "-", csEnd + 1, " )" ),
                     paste( "(", sector, ")" ), sep ="\n" )
  title( main = title, sub = subTitle )

  barplot( bars.pos, beside = TRUE, legend.text = TRUE, axisnames = TRUE, xpd = FALSE,
           ylab = "Labor productivity growth rate", ylim = round( range( bars.pos ) * 1.5, 2 ) )
  title <- paste( "DN decomposition of productivity growth (", firmTypes[2], ")" )
  subTitle <- paste( paste0( "( Share = ", shType, " / MC runs = ", nSize,
                             " / Period = ", csBeg + 1, "-", csEnd + 1, " )" ),
                     paste( "(", sector, ")" ), sep ="\n" )
  title( main = title, sub = subTitle )

}
