#******************************************************************
#
# ------- Foster et al. (2001) productivity decomposition -------
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
# !diagnostics suppress = mc, all.NA, se, count, textplot, saveCSV

FHK_decomp <- function( nExp, nSize, nTstat, nElem, warmUp, prdWnd, fShare,
                        files, sDigits, legends, expLeg, sector, folder, outDir,
                        repName, datFilSfx, firmTypes ) {

  errThr <- 1e-10             # rounding error threshold

  prdLab <- c( "lA2.1", "lA2", "lA2.pre.1", "lA2.pre", "lA2.pos.1", "lA2.pos",
               "lA2.ent", "lA2.ext.1", "lA2.inc.1", "lA2.inc", "lA2.inc.pre.1",
               "lA2.inc.pre", "lA2.inc.pos.1", "lA2.inc.pos", "delta.lA2",
               "delta.lA2.pre", "delta.lA2.pos", "delta.lA2.inc",
               "delta.lA2.inc.pre", "delta.lA2.inc.pos", "B.pre", "B.pos",
               "W.pre", "W.pos", "C.pre", "C.pos", "N.pre", "N.pos", "X.pre",
               "X.pos", "sum.dec", "dec.err", "dec.err.pre", "dec.err.pos",
               "nF", "nF.inc", "nF.pre", "nF.pos", "nF.inc.pre", "nF.inc.pos" )

  if( fShare == "f2e" )
    shType = "output"
  else
    shType = "labor"

  prdDec <- prdDecTot <- list( )
  bars.tot <- bars.pre <- bars.pos <- matrix( nrow = 5, ncol = 0 )

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

      cat( "  Monte Carlo case", l, "of", nSize, "\n")

      # load MC data from temporary files
      load( files[[ k ]]$mc[ l ] )

      # create the decomposition matrix for the experiment
      nPer <- nTstat - warmUp - prdWnd + 1
      prdDec[[ k ]][[ l ]] <- data.frame( matrix( nrow = nPer,
                                                  ncol = length( prdLab ) ) )
      rownames( prdDec[[ k ]][[ l ]] ) <- ( warmUp + prdWnd ) : nTstat
      colnames( prdDec[[ k ]][[ l ]] ) <- prdLab

      for( i in ( 1 + prdWnd ) : ( nTstat - warmUp + 1 ) ) {  # all time steps

        # concatenate data from t-k and t
        cols <- c( fShare, "lA2", "post2chg" )
        fd <- t( mc[ warmUp + i - 1, cols, ] )
        fd.1 <- t( mc[ max( warmUp + i - 1 - prdWnd, 1 ), cols, ] )
        cols[ 1 ] <- "f2"
        colnames( fd ) <- cols
        colnames( fd.1 ) <- cols
        fd <- data.frame( cbind( fd, fd.1 ) )

        # remove lines without data
        fd <- fd[ ! all.NA( fd ), , drop = FALSE ]

        # remove lines with incomplete data (incumbents with zero production)
        toRemove <- vector( "numeric" )
        for( j in 1 : nrow ( fd ) ) {
          # firms without data in both periods
          if( ( is.na( fd$f2[ j ] ) && is.na( fd$lA2[ j ] ) &&
                is.na( fd$f2.1[ j ] ) && is.na( fd$lA2.1[ j ] ) ) ||
              ( is.na( fd$lA2[ j ] ) && is.na( fd$lA2.1[ j ] ) ) )
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
          if( is.na( fd$lA2[ j ] ) )
            fd$f2[ j ] <- NA
          if( is.na( fd$lA2.1[ j ] ) )
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
        fd$delta.lA2 <- fd$lA2 - fd$lA2.1

        j <- i - prdWnd

        # number of firms (divided by number of time steps to add-up to the average)
        prdDec[[ k ]][[ l ]]$nF[ j ] <- count( ! is.na( fd$lA2 ) ) / nPer
        prdDec[[ k ]][[ l ]]$nF.pre[ j ] <-
          count( ! is.na( fd$lA2[ fd$post2chg == 0 ] ) ) / nPer
        prdDec[[ k ]][[ l ]]$nF.pos[ j ] <-
          count( ! is.na( fd$lA2[ fd$post2chg == 1 ] ) ) / nPer
        prdDec[[ k ]][[ l ]]$nF.inc[ j ] <-
          count( ! is.na( fd$lA2[ fd$type == 2 ] ) ) / nPer
        prdDec[[ k ]][[ l ]]$nF.inc.pre[ j ] <-
          count( ! is.na( fd$lA2[ fd$post2chg == 0 & fd$type == 2 ] ) ) / nPer
        prdDec[[ k ]][[ l ]]$nF.inc.pos[ j ] <-
          count( ! is.na( fd$lA2[ fd$post2chg == 1 & fd$type == 2 ] ) ) / nPer

        # weighted average productivity and growth
        prdDec[[ k ]][[ l ]]$lA2.1[ j ] <-
          sum( fd$f2.1 * fd$lA2.1, na.rm = TRUE )     # in window start (t-k)
        prdDec[[ k ]][[ l ]]$lA2[ j ] <-
          sum( fd$f2 * fd$lA2, na.rm = TRUE )         # in window end (t)
        prdDec[[ k ]][[ l ]]$delta.lA2[ j ] <-
          prdDec[[ k ]][[ l ]]$lA2[ j ] - prdDec[[ k ]][[ l ]]$lA2.1[ j ]

        prdDec[[ k ]][[ l ]]$lA2.pre.1[ j ] <-
          sum( fd$f2.1[ fd$post2chg == 0 ] * fd$lA2.1[ fd$post2chg == 0 ],
               na.rm = TRUE )
        prdDec[[ k ]][[ l ]]$lA2.pre[ j ] <-
          sum( fd$f2[ fd$post2chg == 0 ] * fd$lA2[ fd$post2chg == 0 ], na.rm = TRUE )
        prdDec[[ k ]][[ l ]]$delta.lA2.pre[ j ] <-
          prdDec[[ k ]][[ l ]]$lA2.pre[ j ] - prdDec[[ k ]][[ l ]]$lA2.pre.1[ j ]

        prdDec[[ k ]][[ l ]]$lA2.pos.1[ j ] <-
          sum( fd$f2.1[ fd$post2chg == 1 ] * fd$lA2.1[ fd$post2chg == 1 ], na.rm = TRUE )
        prdDec[[ k ]][[ l ]]$lA2.pos[ j ] <-
          sum( fd$f2[ fd$post2chg == 1 ] * fd$lA2[ fd$post2chg == 1 ], na.rm = TRUE )
        prdDec[[ k ]][[ l ]]$delta.lA2.pos[ j ] <-
          prdDec[[ k ]][[ l ]]$lA2.pos[ j ] - prdDec[[ k ]][[ l ]]$lA2.pos.1[ j ]

        prdDec[[ k ]][[ l ]]$lA2.ent[ j ] <-
          sum( fd$f2[ fd$type == 1 ] * fd$lA2[ fd$type == 1 ], na.rm = TRUE )

        prdDec[[ k ]][[ l ]]$lA2.ext.1[ j ] <-
          sum( fd$f2.1[ fd$type == 3 ] * fd$lA2.1[ fd$type == 3 ], na.rm = TRUE )

        prdDec[[ k ]][[ l ]]$lA2.inc.1[ j ] <-
          sum( fd$f2.1[ fd$type == 2 ] * fd$lA2.1[ fd$type == 2 ], na.rm = TRUE )
        prdDec[[ k ]][[ l ]]$lA2.inc[ j ] <-
          sum( fd$f2[ fd$type == 2 ] * fd$lA2[ fd$type == 2 ], na.rm = TRUE )
        prdDec[[ k ]][[ l ]]$delta.lA2.inc[ j ] <-
          prdDec[[ k ]][[ l ]]$lA2.inc[ j ] - prdDec[[ k ]][[ l ]]$lA2.inc.1[ j ]

        prdDec[[ k ]][[ l ]]$lA2.inc.pre.1[ j ] <-
          sum( fd$f2.1[ fd$post2chg == 0 & fd$type == 2 ] *
                 fd$lA2.1[ fd$post2chg == 0 & fd$type == 2 ], na.rm = TRUE )
        prdDec[[ k ]][[ l ]]$lA2.inc.pre[ j ] <-
          sum( fd$f2[ fd$post2chg == 0 & fd$type == 2 ] *
                 fd$lA2[ fd$post2chg == 0 & fd$type == 2 ], na.rm = TRUE )
        prdDec[[ k ]][[ l ]]$delta.lA2.inc.pre[ j ] <-
          prdDec[[ k ]][[ l ]]$lA2.inc.pre[ j ] - prdDec[[ k ]][[ l ]]$lA2.inc.pre.1[ j ]

        prdDec[[ k ]][[ l ]]$lA2.inc.pos.1[ j ] <-
          sum( fd$f2.1[ fd$post2chg == 1 & fd$type == 2 ] *
                 fd$lA2.1[ fd$post2chg == 1 & fd$type == 2 ], na.rm = TRUE )
        prdDec[[ k ]][[ l ]]$lA2.inc.pos[ j ] <-
          sum( fd$f2[ fd$post2chg == 1 & fd$type == 2 ] *
                 fd$lA2[ fd$post2chg == 1 & fd$type == 2 ], na.rm = TRUE )
        prdDec[[ k ]][[ l ]]$delta.lA2.inc.pos[ j ] <-
          prdDec[[ k ]][[ l ]]$lA2.inc.pos[ j ] - prdDec[[ k ]][[ l ]]$lA2.inc.pos.1[ j ]

        # do the FHK decomposition of delta productivity
        prdDec[[ k ]][[ l ]]$B.pre[ j ] <-
          sum( fd$delta.f2[ fd$type == 2 & fd$post2chg == 0 ] *
                 ( fd$lA2.1[ fd$type == 2 & fd$post2chg == 0 ] -
                     prdDec[[ k ]][[ l ]]$lA2.pre.1[ j ] ),
               na.rm = TRUE )
        prdDec[[ k ]][[ l ]]$B.pos[ j ] <-
          sum( fd$delta.f2[ fd$type == 2 & fd$post2chg == 1 ] *
                 ( fd$lA2.1[ fd$type == 2 & fd$post2chg == 1 ] -
                     prdDec[[ k ]][[ l ]]$lA2.pos.1[ j ] ),
               na.rm = TRUE )
        prdDec[[ k ]][[ l ]]$W.pre[ j ] <-
          sum( fd$f2.1[ fd$type == 2 & fd$post2chg == 0 ] *
                 fd$delta.lA2[ fd$type == 2 & fd$post2chg == 0 ] ,
               na.rm = TRUE )
        prdDec[[ k ]][[ l ]]$W.pos[ j ] <-
          sum( fd$f2.1[ fd$type == 2 & fd$post2chg == 1 ] *
                 fd$delta.lA2[ fd$type == 2 & fd$post2chg == 1 ] ,
               na.rm = TRUE )
        prdDec[[ k ]][[ l ]]$C.pre[ j ] <-
          sum( fd$delta.f2[ fd$type == 2 & fd$post2chg == 0 ] *
                 fd$delta.lA2[ fd$type == 2 & fd$post2chg == 0 ] ,
               na.rm = TRUE )
        prdDec[[ k ]][[ l ]]$C.pos[ j ] <-
          sum( fd$delta.f2[ fd$type == 2 & fd$post2chg == 1 ] *
                 fd$delta.lA2[ fd$type == 2 & fd$post2chg == 1 ] ,
               na.rm = TRUE )
        prdDec[[ k ]][[ l ]]$N.pre[ j ] <-
          sum( fd$f2[ fd$type == 1 & fd$post2chg == 0 ] *
                 ( fd$lA2[ fd$type == 1 & fd$post2chg == 0 ] -
                     prdDec[[ k ]][[ l ]]$lA2.pre.1[ j ] ),
               na.rm = TRUE )
        prdDec[[ k ]][[ l ]]$N.pos[ j ] <-
          sum( fd$f2[ fd$type == 1 & fd$post2chg == 1 ] *
                 ( fd$lA2[ fd$type == 1 & fd$post2chg == 1 ] -
                     prdDec[[ k ]][[ l ]]$lA2.pos.1[ j ] ),
               na.rm = TRUE )
        prdDec[[ k ]][[ l ]]$X.pre[ j ] <-
          sum( fd$f2.1[ fd$type == 3 & fd$post2chg == 0 ] *
                 ( fd$lA2.1[ fd$type == 3 & fd$post2chg == 0 ] -
                     prdDec[[ k ]][[ l ]]$lA2.pre.1[ j ] ),
               na.rm = TRUE )
        prdDec[[ k ]][[ l ]]$X.pos[ j ] <-
          sum( fd$f2.1[ fd$type == 3 & fd$post2chg == 1 ] *
                 ( fd$lA2.1[ fd$type == 3 & fd$post2chg == 1 ] -
                     prdDec[[ k ]][[ l ]]$lA2.pos.1[ j ] ),
               na.rm = TRUE )

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
        prdDec[[ k ]][[ l ]]$dec.err[ j ] <- prdDec[[ k ]][[ l ]]$delta.lA2[ j ] -
                                             prdDec[[ k ]][[ l ]]$sum.dec[ j ]
        prdDec[[ k ]][[ l ]]$dec.err.pre[ j ] <- prdDec[[ k ]][[ l ]]$delta.lA2.pre[ j ] -
                                                 prdDec[[ k ]][[ l ]]$B.pre[ j ] -
                                                 prdDec[[ k ]][[ l ]]$W.pre[ j ] -
                                                 prdDec[[ k ]][[ l ]]$C.pre[ j ] -
                                                 prdDec[[ k ]][[ l ]]$N.pre[ j ] +
                                                 prdDec[[ k ]][[ l ]]$X.pre[ j ]
        prdDec[[ k ]][[ l ]]$dec.err.pos[ j ] <- prdDec[[ k ]][[ l ]]$delta.lA2.pos[ j ] -
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

      prdDecTot[[ k ]][ l, ] <- colSums( prdDec[[ k ]][[ l ]], na.rm = TRUE )
    }

    rm( mc )

    # Productivity growth table

    prdGrw.stats <- matrix( c( mean( prdDecTot[[ k ]]$nF, na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$lA2, na.rm = TRUE ) /
                                 mean( prdDecTot[[ k ]]$nF, na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$delta.lA2, na.rm = TRUE ) /
                                 mean( prdDecTot[[ k ]]$nF, na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$delta.lA2, na.rm = TRUE ),

                               mean( prdDecTot[[ k ]]$nF.pre, na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$lA2.pre, na.rm = TRUE ) /
                                 mean( prdDecTot[[ k ]]$nF.pre, na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$delta.lA2.pre, na.rm = TRUE ) /
                                 mean( prdDecTot[[ k ]]$nF.pre, na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$delta.lA2.pre, na.rm = TRUE ),

                               mean( prdDecTot[[ k ]]$nF.pos, na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$lA2.pos, na.rm = TRUE ) /
                                 mean( prdDecTot[[ k ]]$nF.pos, na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$delta.lA2.pos, na.rm = TRUE ) /
                                 mean( prdDecTot[[ k ]]$nF.pos, na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$delta.lA2.pos, na.rm = TRUE ),

                               mean( prdDecTot[[ k ]]$nF.inc, na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$lA2.inc, na.rm = TRUE ) /
                                 mean( prdDecTot[[ k ]]$nF.inc, na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$delta.lA2.inc, na.rm = TRUE ) /
                                 mean( prdDecTot[[ k ]]$nF.inc, na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$delta.lA2.inc, na.rm = TRUE ),

                               mean( prdDecTot[[ k ]]$nF.inc.pre, na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$lA2.inc.pre, na.rm = TRUE ) /
                                 mean( prdDecTot[[ k ]]$nF.inc.pre, na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$delta.lA2.inc.pre, na.rm = TRUE ) /
                                 mean( prdDecTot[[ k ]]$nF.inc.pre, na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$delta.lA2.inc.pre, na.rm = TRUE ),

                               mean( prdDecTot[[ k ]]$nF.inc.pos, na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$lA2.inc.pos, na.rm = TRUE ) /
                                 mean( prdDecTot[[ k ]]$nF.inc.pos, na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$delta.lA2.inc.pos, na.rm = TRUE ) /
                                 mean( prdDecTot[[ k ]]$nF.inc.pos, na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$delta.lA2.inc.pos, na.rm = TRUE ) ),
                            ncol = 4, byrow = TRUE )
    colnames( prdGrw.stats ) <- c( "Firm #", "Avg. prod.", "Avg. growth", "Total growth" )
    rownames( prdGrw.stats ) <- c( "Overall", firmTypes[1], firmTypes[2],
                                   "Incumbents", paste( firmTypes[1], "incumbents" ),
                                   paste( firmTypes[2], "incumbents" ) )

    # remove NaNs
    prdGrw.stats[ is.nan( prdGrw.stats ) ] <- 0

    textplot( formatC( prdGrw.stats, digits = sDigits, format = "g" ), cmar = 1.0 )
    title <- paste( "Productivity dynamics per firm type (", legends[ k ], ")" )
    subTitle <- paste( paste0( "( Sample size = ", nElem[ k ],
                               " firms / Window = ", prdWnd,
                               " periods / MC runs = ", nSize, " / Period = ",
                               warmUp + 1, "-", nTstat, " )" ),
                       paste( "(", sector, ")" ), sep ="\n" )
    title( main = title, sub = subTitle )

    saveCSV( prdGrw.stats, baseName = repName, num = k, baseFolder = folder,
             subFolder = outDir, suffix = datFilSfx, type = "prd_grw" )

    # Productivity decomposition table

    prdDec.stats <- matrix( c( mean( prdDecTot[[ k ]]$delta.lA2, na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$W.pre + prdDecTot[[ k ]]$W.pos,
                                     na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$B.pre + prdDecTot[[ k ]]$B.pos,
                                     na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$C.pre + prdDecTot[[ k ]]$C.pos,
                                     na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$N.pre + prdDecTot[[ k ]]$N.pos,
                                     na.rm = TRUE ),
                               mean( - prdDecTot[[ k ]]$X.pre - prdDecTot[[ k ]]$X.pos,
                                     na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$dec.err, na.rm = TRUE ),

                               se( prdDecTot[[ k ]]$delta.lA2 ),
                               se( prdDecTot[[ k ]]$W.pre + prdDecTot[[ k ]]$W.pos ),
                               se( prdDecTot[[ k ]]$B.pre + prdDecTot[[ k ]]$B.pos ),
                               se( prdDecTot[[ k ]]$C.pre + prdDecTot[[ k ]]$C.pos ),
                               se( prdDecTot[[ k ]]$N.pre + prdDecTot[[ k ]]$N.pos ),
                               se( prdDecTot[[ k ]]$X.pre + prdDecTot[[ k ]]$X.pos ),
                               se( prdDecTot[[ k ]]$dec.err ),

                               mean( prdDecTot[[ k ]]$delta.lA2.pre, na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$W.pre, na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$B.pre, na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$C.pre, na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$N.pre, na.rm = TRUE ),
                               mean( - prdDecTot[[ k ]]$X.pre, na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$dec.err.pre, na.rm = TRUE ),

                               se( prdDecTot[[ k ]]$delta.lA2.pre ),
                               se( prdDecTot[[ k ]]$W.pre ),
                               se( prdDecTot[[ k ]]$B.pre ),
                               se( prdDecTot[[ k ]]$C.pre ),
                               se( prdDecTot[[ k ]]$N.pre ),
                               se( prdDecTot[[ k ]]$X.pre ),
                               se( prdDecTot[[ k ]]$dec.err.pre ),

                               mean( prdDecTot[[ k ]]$delta.lA2.pos, na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$W.pos, na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$B.pos, na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$C.pos, na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$N.pos, na.rm = TRUE ),
                               mean( - prdDecTot[[ k ]]$X.pos, na.rm = TRUE ),
                               mean( prdDecTot[[ k ]]$dec.err.pos, na.rm = TRUE ),

                               se( prdDecTot[[ k ]]$delta.lA2.pos ),
                               se( prdDecTot[[ k ]]$W.pos ),
                               se( prdDecTot[[ k ]]$B.pos ),
                               se( prdDecTot[[ k ]]$C.pos ),
                               se( prdDecTot[[ k ]]$N.pos ),
                               se( prdDecTot[[ k ]]$X.pos ),
                               se( prdDecTot[[ k ]]$dec.err.pos ) ),
                            ncol = 7, byrow = TRUE )
    colnames( prdDec.stats ) <- c( "Total", "Within", "Between",
                                   "Cross", "Entry", "Exit", "Error" )
    rownames( prdDec.stats ) <- c( "Overall", "(s.e.)", firmTypes[1],
                                   "(s.e.)", firmTypes[2], "(s.e.)" )

    # remove NaNs
    prdDec.stats[ is.nan( prdDec.stats ) ] <- 0

    # absolute productivity growth log values table
    textplot( formatC( prdDec.stats, digits = sDigits, format = "g" ), cmar = 1.0 )
    title <- paste( "FHK decomposition of labor productivity growth (", legends[ k ], ")" )
    subTitle <- paste( paste0( "( Share = ", shType, " / sample size = ",
                               nElem[ k ], " firms / Window = ", prdWnd,
                               " periods / MC runs = ", nSize, " / Period = ",
                               warmUp + 1, "-", nTstat, " )" ),
                       paste( "(", sector, ")" ), sep ="\n" )
    title( main = title, sub = subTitle )

    saveCSV( prdDec.stats, baseName = repName, num = k, baseFolder = folder,
             subFolder = outDir, suffix = datFilSfx, type = "FHK_dec" )

    # add data to global chart (as % of total productivity growth)
    bars.tot <- cbind( bars.tot, c( prdDec.stats[ 1, 2 : 6 ] ) )
    bars.pre <- cbind( bars.pre, c( prdDec.stats[ 3, 2 : 6 ] ) )
    bars.pos <- cbind( bars.pos, c( prdDec.stats[ 5, 2 : 6 ] ) )
  }

  # name the bar groups
  colnames( bars.tot ) <- legends
  colnames( bars.pre ) <- legends
  colnames( bars.pos ) <- legends


  # Plot the bar graphs

  barplot( bars.tot, beside = TRUE, legend.text = TRUE, axisnames = TRUE, xpd = FALSE,
           ylab = "Labor productivity growth", ylim = round( range( bars.tot ) * 1.5, 2 ) )
  title <- paste( "FHK decomposition of productivity growth ( all firms )" )
  subTitle <- paste( paste0( "( Share = ", shType, " / MC runs = ", nSize,
                             " / Window = ", prdWnd, " periods / Period = ",
                             warmUp + 1, "-", nTstat, " )" ),
                     paste( "(", sector, ")" ), sep ="\n" )
  title( main = title, sub = subTitle )

  barplot( bars.pre, beside = TRUE, legend.text = TRUE, axisnames = TRUE, xpd = FALSE,
           ylab = "Labor productivity growth", ylim = round( range( bars.pre ) * 1.5, 2 ) )
  title <- paste( "FHK decomposition of productivity growth ( pre-change firms )" )
  subTitle <- paste( paste0( "( Share = ", shType, " / MC runs = ", nSize,
                             " / Window = ", prdWnd, " periods / Period = ",
                             warmUp + 1, "-", nTstat, " )" ),
                     paste( "(", sector, ")" ), sep ="\n" )
  title( main = title, sub = subTitle )

  barplot( bars.pos, beside = TRUE, legend.text = TRUE, axisnames = TRUE, xpd = FALSE,
           ylab = "Labor productivity growth", ylim = round( range( bars.pos ) * 1.5, 2 ) )
  title <- paste( "FHK decomposition of productivity growth (", firmTypes[2], ")" )
  subTitle <- paste( paste0( "( Share = ", shType, " / MC runs = ", nSize,
                             " / Window = ", prdWnd, " periods / Period = ",
                             warmUp + 1, "-", nTstat, " )" ),
                     paste( "(", sector, ")" ), sep ="\n" )
  title( main = title, sub = subTitle )

}
