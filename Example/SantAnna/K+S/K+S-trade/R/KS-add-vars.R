#******************************************************************
#
# ---------- K+S additional variables for analysis --------------
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

# functions to add new variables to Monte Carlo data (to be parallelized)

# remove RStudio warnings for support functions
# !diagnostics suppress = log0, mean.clean, vector.clean,
# !diagnostics suppress = Xdata, Sdata, cdata, Cdata, mdata, Mdata


# ====== KS-aggregates.R ======

addVar_aggr <- function( mc, nElemMC, nTsteps, nVar ) {

  UzeroTrsh <- 0.01   # full employment threshold
  crisTrsh  <- -0.03  # GDP growth crisis threshold
  smoothing <- 1e5    # HP filter smoothing factor (lambda)

  for( i in 1 : nElemMC ) {            # all element instances

    mc[ , "A2sd", i ]   <- mc[ , "A2sdPreChg", i ] *
                           ( 1 - mc[ , "f2posChg", i ] ) +
                           mc[ , "A2sdPosChg", i ] * mc[ , "f2posChg", i ]
    mc[ , "BonW2", i ]  <- mc[ , "Bon2", i ] / mc[ , "W2", i ]
    mc[ , "W", i ]      <- mc[ , "W1", i ] + mc[ , "W2", i ]
    mc[ , "Xnet", i ]   <- mc[ , "X", i ] - mc[ , "M", i ]
    mc[ , "XcNet", i ]  <- mc[ , "Xc", i ] - mc[ , "Mc", i ]
    mc[ , "XkNet", i ]  <- mc[ , "Xk", i ] - mc[ , "Mk", i ]
    mc[ , "Y100", i ]   <- mc[ , "Y", i ] / mc[ , "Q2u", i ]
    mc[ , "nEntT1", i ] <- hpfilter( mc[ , "entry1exit", i ],
                                     smoothing )$trend[ , 1 ]
    mc[ , "nEntT2", i ] <- hpfilter( mc[ , "entry2exit", i ],
                                     smoothing )$trend[ , 1 ]

    for( t in 1 : nTsteps ) {           # all periods

      if( mc[ t , "U", i ] < UzeroTrsh )
        mc[ t , "Uzero", i ] <- 1
      else
        mc[ t , "Uzero", i ] <- 0

      if( t > 1 ) {
        mc[ t , "dYreal", i ] <- log0( mc[ t , "Yreal", i ] ) -
                                 log0( mc[ t - 1 , "Yreal", i ] )

        if( mc[ t , "dYreal", i ] < crisTrsh )
          mc[ t , "tCrisis", i ] <- 1
        else
          mc[ t , "tCrisis", i ] <- 0
      }
    }
  }

  mc[ ! is.finite( mc ) ] <- NA

  return( mc )
}


# function to add new variables comparing different countries' gaps
addGapVar <- function( var, mCnt, nExp ) {

    if ( mCnt && nExp > 1 && exists( "cdata" ) && exists( "Cdata" ) &&
         exists( "mdata" ) && exists( "Mdata" ) ) {

    for( i in 1 : length( var ) ) {

      for( k in 1 : nExp ) {    # for all countries
        v1 <- Xdata[[ k ]][ , var[[ i ]][ 1 ] ]
        if( k < nExp )
          v2 <- Xdata[[ k + 1 ]][ , var[[ i ]][ 1 ] ]
        else
          v2 <- Xdata[[ k - 1 ]][ , var[[ i ]][ 1 ] ]

        if( var[[ i ]][ 3 ] )
          val <- log0( v1 ) - log0( v2 )
        else
          val <- v1 - v2

        val[ ! is.finite( val ) ] <- NA
        sd <- rep( 0, length( Sdata[[ k ]][ , var[[ i ]][ 1 ] ] ) )

        Xdata[[ k ]][ var[[ i ]][ 2 ] ] <<- val
        cdata[[ k ]][ var[[ i ]][ 2 ] ] <<- val
        Cdata[[ k ]][ var[[ i ]][ 2 ] ] <<- val
        mdata[[ k ]][ var[[ i ]][ 2 ] ] <<- val
        Mdata[[ k ]][ var[[ i ]][ 2 ] ] <<- val
        Sdata[[ k ]][ var[[ i ]][ 2 ] ] <<- sd
      }
    }
  }
}


# ====== KS-aggregates-labor ======

addVar_labor <- function( mc, nElemMC, nTsteps, nVar ) {

  for( i in 1 : nElemMC ) {            # all element instances

  }

  mc[ ! is.finite( mc ) ] <- NA

  return( mc )
}


# ====== KS-sector-1.R ======

addVar_firm1 <- function( mc, nElemMC, nTsteps, nVar ) {

  for( i in 1 : nTsteps ) {            # all time steps

    Q1e         <- vector.clean( mc[ i, "Q1e", ], min = 1 )
    mean.Q1e    <- mean.clean( Q1e )
    mean.lsize1 <- mean.clean( log( Q1e ) )
    mean.Btau   <- mean.clean( vector.clean( mc[ i, "Btau", ] ) )

    for( j in 1 : nElemMC ) {             # and all element instances

      # take care of entrants' first period and other data problems to avoid atifacts
      if( is.na( mc[ i, "Q1e", j ] ) || mc[ i, "Q1e", j ] < 1 ||
          is.na( mc[ i, "Btau", j ] ) || mc[ i, "Btau", j ] < 1 ) {
        mc[ i, , j ] <- NA
        next
      }

      # normalization of key variables using the period average size
      if( mean.Q1e != 0 )
        mc[ i, "normO1", j ] <- mc[ i,"Q1e", j ] / mean.Q1e
      if( mean.lsize1 != 0 )
        mc[ i, "lnormO1", j ] <- log( mc[ i, "Q1e", j ] ) - mean.lsize1
      if( mean.Btau != 0 )
        mc[ i, "normA1", j ] <- mc[ i, "Btau", j ] / mean.Btau

      # growth rates are calculated only from 2nd period and for non-entrant firms
      if( i > 1 ) {
        if( ! is.na( mc[ i - 1, "Q1e", j ] ) ){

          # Size, normalized sales and productivity growth
          mc[ i, "growth1", j ] <- log( mc[ i, "Q1e", j ] ) -
            log( mc[ i - 1, "Q1e", j ] )
          mc[ i, "normO1grow", j ] <- log( mc[ i, "normO1", j ] ) -
            log( mc[ i - 1, "normO1", j ] )
          mc[ i, "normA1grow", j ] <- log( mc[ i, "normA1", j ] ) -
            log( mc[ i - 1, "normA1", j ] )

          if( is.infinite( mc[ i, "normO1grow", j ] ) )
            mc[ i, "normO1grow", j ] <- NA
          if( is.infinite( mc[ i, "normA1grow", j ] ) )
            mc[ i, "normA1grow", j ] <- NA
        }

        if( is.finite( mc[ i, "lnormO1", j ] ) &&
            is.finite( mc[ i - 1, "lnormO1", j ] ) ) {

          mc[ i, "lnormO1match", j ] <- mc[ i, "lnormO1", j ]
          mc[ i, "lnormO1matchLag", j ] <- mc[ i - 1, "lnormO1", j ]
          mc[ i, "lnormO1grow", j ] <- mc[ i, "lnormO1", j ] -
            mc[ i - 1, "lnormO1", j ]

          if( ! is.finite( mc[ i, "lnormO1grow", j ] ) ) {
            mc[ i, "lnormO1grow", j ] <- NA
            mc[ i, "lnormO1match", j ] <- NA
            mc[ i, "lnormO1matchLag", j ] <- NA
          }
        }
      }
    }
  }

  return( mc )
}


# ====== KS-sector-2.R ======

addVar_firm2 <- function( mc, nElemMC, nTsteps, nVar ) {

  for( i in 1 : nTsteps ) {                # all time steps

    Q2e <- vector.clean( mc[ i, "Q2e", ], min = 1 )
    mean.Q2e <- mean.clean( Q2e )
    mean.lsize2 <- mean.clean( log( Q2e ) )
    mean.A2 <- mean.clean( vector.clean( mc[ i, "A2", ] ) )
    mean.s2avg <- mean.clean( vector.clean( mc[ i, "s2avg", ] ) )
    mean.w2realAvg <- mean.clean( vector.clean( mc[ i, "w2realAvg", ] ) )

    tot2 <- vector.clean( mc[ i, "L2", ] )
    sum.tot2 <- sum( tot2, na.rm = TRUE )
    if( ! is.finite( sum.tot2 ) ) sum.tot2 <- 0

    for( j in 1 : nElemMC ) {             # and all element instances

      # take care of entrants' first period and other data problems to avoid artifacts
      if( is.na( mc[ i, "Q2e", j ] ) || mc[ i, "Q2e", j ] < 1 ||
          is.na( mc[ i, "A2", j ] ) || mc[ i, "A2", j ] <= 0 ||
          is.na( mc[ i, "w2realAvg", j ] ) || mc[ i, "w2realAvg", j ] <= 0 ||
          is.na( mc[ i, "s2avg", j ] ) || mc[ i, "s2avg", j ] <= 0 ||
          is.na( mc[ i, "postChg", j ] ) || is.na( mc[ i, "L2", j ] ) ) {
        mc[ i, , j ] <- NA
        next
      }

      # normalization of key variables using the period average size
      if( mean.Q2e != 0 )
        mc[ i, "normO2", j ] <- mc[ i, "Q2e", j ] / mean.Q2e
      if( mean.lsize2 != 0 )
        mc[ i, "lnormO2", j ] <- log( mc[ i, "Q2e", j ] ) - mean.lsize2
      if( mean.A2 != 0 ) {
        mc[ i, "normA2", j ] <- mc[ i, "A2", j ] / mean.A2
        # handle separate firms groups
        if( ! is.na( mc[ i, "postChg", j ] ) ) {
          if( mc[ i, "postChg", j ] == 0 )
            mc[ i, "normApreChg2", j ] = mc[ i, "normA2", j ]
          else
            mc[ i, "normApostChg2", j ] = mc[ i, "normA2", j ]
        }
      }
      if( mean.w2realAvg != 0 ) {
        mc[ i, "normRW2", j ] <- mc[ i, "w2realAvg", j ] / mean.w2realAvg
        # handle separate firms groups
        if( ! is.na( mc[ i, "postChg", j ] ) ) {
          if( mc[ i, "postChg", j ] == 0 )
            mc[ i, "normRWpreChg2", j ] = mc[ i, "normRW2", j ]
          else
            mc[ i, "normRWpostChg2", j ] = mc[ i, "normRW2", j ]
        }
      }
      if( mean.s2avg != 0 )
        mc[ i, "normSK2", j ] <- mc[ i, "s2avg", j ] / mean.s2avg

      # FHK decomposition variables
      if( mc[ i, "Q2e", j ] > 0 && mc[ i, "L2", j ] > 0 ) {
        mc[ i, "A2d", j ] <- mc[ i, "Q2e", j ] / mc[ i, "L2", j ]
        mc[ i, "lA2", j ] <- log( mc[ i, "Q2e", j ] / mc[ i, "L2", j ] )

        if( sum.tot2 > 0 )
          mc[ i, "f2l", j ] <- mc[ i, "L2", j ] / sum.tot2
      }

      # wage x productivity regression extra variable
      if( mc[ i, "w2realAvg", j ] > 0 )
        mc[ i, "lrwAvg2", j ] <- log( mc[ i, "w2realAvg", j ] )

      # growth rates and deltas calculated only from 2nd period for non-entrant
      if( i > 1 ) {
        if( is.finite( mc[ i - 1, "Q2e", j ] ) ) {

          # Size, normalized sales and productivity growth
          mc[ i, "growth2", j ] <- log( mc[ i, "Q2e", j ] ) -
            log( mc[ i - 1, "Q2e", j ] )
          mc[ i, "normO2grow", j ] <- log( mc[ i, "normO2", j ] ) -
            log( mc[ i - 1, "normO2", j ] )
          mc[ i, "normA2grow", j ] <- log( mc[ i, "normA2", j ] ) -
            log( mc[ i - 1, "normA2", j ] )
          mc[ i, "normRW2grow", j ] <- log( mc[ i, "normRW2", j ] ) -
            log( mc[ i - 1, "normRW2", j ] )
          mc[ i, "normSK2grow", j ] <- log( mc[ i, "normSK2", j ] ) -
            log( mc[ i - 1, "normSK2", j ] )

          if( ! is.finite( mc[ i, "normO2grow", j ] ) )
            mc[ i, "normO2grow", j ] <- NA
          if( ! is.finite( mc[ i, "normA2grow", j ] ) )
            mc[ i, "normA2grow", j ] <- NA
          if( ! is.finite( mc[ i, "normRW2grow", j ] ) )
            mc[ i, "normRW2grow", j ] <- NA
          if( ! is.finite( mc[ i, "normSK2grow", j ] ) )
            mc[ i, "normSK2grow", j ] <- NA
        }

        if( is.finite( mc[ i, "lnormO2", j ] ) &&
            is.finite( mc[ i - 1, "lnormO2", j ] ) ) {

          mc[ i, "lnormO2match", j ] <- mc[ i, "lnormO2", j ]
          mc[ i, "lnormO2matchLag", j ] <- mc[ i - 1, "lnormO2", j ]
          mc[ i, "lnormO2grow", j ] <- mc[ i, "lnormO2", j ] -
            mc[ i - 1, "lnormO2", j ]

          if( ! is.finite( mc[ i, "lnormO2grow", j ] ) ) {
            mc[ i, "lnormO2grow", j ] <- NA
            mc[ i, "lnormO2match", j ] <- NA
            mc[ i, "lnormO2matchLag", j ] <- NA
          }
        }
      }
    }
  }

  return( mc )
}


# ====== KS-workers.R ======

addVar_worker <- function( mc, nElemMC, nTsteps, nVar ) {

  for( i in 1 : nTsteps ) {                # all time steps

    mean.wReal <- mean.clean( vector.clean( mc[ i, "wReal", ] ) )
    mean.s <- mean.clean( vector.clean( mc[ i, "s", ] ) )
    mean.Tu <- mean.clean( vector.clean( mc[ i, "Tu", ] ) )

    for( j in 1 : nElemMC ){             # and all element instances

      # change zero values to NA to avoid artifacts in statistics
      if( is.na( mc[ i, "wReal", j ] ) || mc[ i, "wReal", j ] <= 0 ||
          is.na( mc[ i, "s", j ] ) || mc[ i, "s", j ] <= 0 ||
          is.na( mc[ i, "Tu", j ] ) || mc[ i, "Tu", j ] < 0 )
        mc[ i, , j ] <- NA

      # normalization of variables using the period average size
      if( mean.wReal != 0 )
        mc[ i, "normRW", j ] <- mc[ i, "wReal", j ] / mean.wReal
      if( mean.s != 0 )
        mc[ i, "normSK", j ] <- mc[ i, "s", j ] / mean.s
      if( mean.Tu != 0 )
        mc[ i, "normTU", j ] <- mc[ i, "Tu", j ] / mean.Tu

      # growth rates are calculated only from 2nd period
      if( i > 1 ){
        if( ! is.na( mc[ i - 1, "normRW", j ] ) && ! is.na( mc[ i, "normRW", j ] ) )
          mc[ i, "normRWgrow", j ] <- log( mc[ i, "normRW", j ] ) -
            log( mc[ i - 1, "normRW", j ] )
        if( ! is.na( mc[ i - 1, "normSK", j ] ) && ! is.na( mc[ i, "normSK", j ] ) )
          mc[ i, "normSKgrow", j ] <- log( mc[ i, "normSK", j ] ) -
            log( mc[ i - 1, "normSK", j ] )
        if( ! is.na( mc[ i - 1, "normTU", j ] ) && ! is.na( mc[ i, "normTU", j ] ) )
          mc[ i, "normTUgrow", j ] <- log( mc[ i, "normTU", j ] ) -
            log( mc[ i - 1, "normTU", j ] )

        if( is.infinite( mc[ i, "normRWgrow", j ] ) )
          mc[ i, "normRWgrow", j ] <- NA
        if( is.infinite( mc[ i, "normSKgrow", j ] ) )
          mc[ i, "normSKgrow", j ] <- NA
        if( is.infinite( mc[ i, "normTUgrow", j ] ) )
          mc[ i, "normTUgrow", j ] <- NA
      }
    }
  }

  return( mc )
}


# ====== KS-elementary-effects-SA.R ======

addVar_sa <- function( data, vars ) {

  ma.length <- 51
  ma.sides <- 2

  if( length( notIn( vars, colnames( data ) ) ) > 0 )
    cat( "Missing variables in data:", notIn( vars, colnames( data ) ), "\n" )

  # ---- general variables ----
  tsteps <- nrow( data )

  # ---- recompute values for existing and create new variables ----

  for( var in vars ) {

    if( var %in% logVar ) {   # consider the log values of fast growing variables

      data[ , var ] <- log0( data[ , var ] )

    } else if( var == "Y_sd" ) {   # GDP std. dev.

      data[ , var ] <- sd( data[ , "dY" ], na.rm = TRUE )

    } else if( var == "cris_llh" ) {   # crises likelihood

      data[ , var ] <- sum( data[ , "dY" ] < -0.03, na.rm = TRUE ) /
        sum( ! is.na( data[ , "dY" ] ) )

    } else if( var == "freq_FE" ) {   # frequency of crises

      data[ , var ] <- sum( data[ , "U" ] == 0, na.rm = TRUE ) /
        sum( ! is.na( data[ , "U" ] ) )

    } else if( var == "vu" && useStats ) {   # V/U (log)

      data[ , var ] <- log( data[ , "V" ] * data[ , "Ls" ] + 1 ) -
        log( data[ , "U" ] * data[ , "Ls" ] + 1 )

    } else if( var == "rec_num" ) {   # crisis recovery number, duration & losses

      # calculate long-term ex post growth rate trend
      growthTrend <- hpfilter( data[ , "dY" ], smoothing )$trend[ , 1 ]

      # Calculate recovery statistics for GDP to the pre-crisis trend
      recovery <- FALSE
      recTime <- recCost <- vector( mode = "numeric" )
      preCrisisY <- gTrend <- timeCrisis <- costCrisis <- NA
      for( i in 2 : tsteps ) {
        if( recovery ) {                                        # in a recovery?
          preCrisisY <- preCrisisY + gTrend                     # update pre crisis trend
          if( data[ i, "Yreal" ] >= preCrisisY ) {              # recovery is over?
            recovery <- FALSE
            if( i - timeCrisis > crisisLen ) { # record crisis longer than x periods
              recTime <- append( recTime, i - timeCrisis )
              recCost <- append( recCost, costCrisis )
            }
          } else {                                              # recovery not over
            gap <- preCrisisY - data[ i, "Yreal" ]              # update statistics
            costCrisis <- costCrisis + gap
          }
        }
        if( ! recovery && data[ i, "dY" ] < crisisTh && i > crisisPre ) {
          recovery <- TRUE
          timeCrisis <- i
          costCrisis <- 0
          preCrisisY <- mean( data[ ( i - crisisPre ) : ( i - 1 ), "Yreal" ] )
          gTrend <- mean( growthTrend[ ( i - crisisPre ) : ( i - 1 ) ], na.rm = TRUE )
        }
      }

      # consolidate results (same values for all time steps)
      if( length( recTime ) > 0 ) {
        data[ , "rec_num" ] <- length( recTime )
        data[ , "rec_dur" ] <- mean( recTime )
        data[ , "rec_loss" ] <- mean( recCost )
      } else {
        data[ , "rec_num" ] <- 0
        data[ , "rec_dur" ] <- 0
        data[ , "rec_loss" ] <- 0
      }
    } else if( var == "XkNetY" ) {
      vals <- data[ , "Xk" ] - data[ , "Mk" ] / data[ , "Y" ]
      data[ , var ] <- ifelse( is.finite( vals ), vals, 0 )
    } else if( var == "XcNetY" ) {
      vals <- data[ , "Xc" ] - data[ , "Mc" ] / data[ , "Y" ]
      data[ , var ] <- ifelse( is.finite( vals ), vals, 0 )
    } else if( var == "tXk" ) {
      XkNetSign <- sign( mavg( data[ , "XkNetY" ], len = ma.length, sides = ma.sides ) )
      data[ , var ] <- as.numeric( row.names( data )[ 1 ] ) + length( XkNetSign )
      for( t in 2 : length( XkNetSign ) ) {
        if( ! is.na( XkNetSign[ t - 1 ] ) && ! is.na( XkNetSign[ t ] ) &&
            XkNetSign[ t ] != XkNetSign[ t - 1 ] ) {
          data[ , var ] <- as.numeric( row.names( data )[ 1 ] ) - 1 + t
          break
        }
      }
    } else if( var == "tXc" ) {
      XcNetSign <- sign( mavg( data[ , "XcNetY" ], len = ma.length, sides = ma.sides ) )
      data[ , var ] <- as.numeric( row.names( data )[ 1 ] ) + length( XcNetSign ) + 1
      for( t in 2 : length( XcNetSign ) ) {
        if( ! is.na( XcNetSign[ t - 1 ] ) && ! is.na( XcNetSign[ t ] ) &&
            XcNetSign[ t ] != XcNetSign[ t - 1 ] ) {
          data[ , var ] <- as.numeric( row.names( data )[ 1 ] ) - 1 + t
          break
        }
      }
    }
  }

  return( data )
}


# ====== KS-kriging-sobol-SA.R ======



# ====== KS-polynomial-sobol-SA.R ======

