#******************************************************************
#
# ----------------- K+S sector 2 MC analysis --------------------
#
#******************************************************************

#******************************************************************
#
# ------------ Read Monte Carlo experiment files ----------------
#
#******************************************************************

folder   <- "data"                    # data files folder
baseName <- "Sim"                     # data files base name (same as .lsd file)
nExp <- 2                             # number of experiments
iniDrop <- 0                          # initial time steps to drop from analysis (0=none)
nKeep <- -1                           # number of time steps to keep (-1=all)

expVal <- c( "Fordist", "Competitive" )   # case parameter values

firmTypes = c( "Pre-change firms", "Post-change firms" )

# Firm-level variables to use
firmVar <- c( "_A2", "_Q2e", "_w2realAvg", "_s2avg", "_postChg", "_L2", "_f2" )
addFirmVar <- c( "growth2", "normO2", "normA2", "normRW2", "normSK2", "normO2grow",
                 "normA2grow", "normRW2grow", "normSK2grow", "f2l", "A2d", "lA2",
                 "lrwAvg2" )
sector <- "( Consumption-goods sector )"


# ==== Process LSD result files ====

# Package with LSD interface functions
library( LSDinterface, verbose = FALSE, quietly = TRUE )
library( abind, verbose = FALSE, quietly = TRUE )

# remove warnings for saved data
# !diagnostics suppress = mc, nSize, nTsteps, nFirms

# ---- Read data files ----

# Function to read one experiment data (to be parallelized)
readExp <- function( exper ) {
  if( nExp > 1 ) {
    myFiles <- list.files( path = folder, pattern = paste0( baseName, exper, "_[0-9]+.res"),
                          full.names = TRUE )
  } else {
    myFiles <- list.files( path = folder, pattern = paste0( baseName, "_[0-9]+.res"),
                          full.names = TRUE )
  }

  if( length( myFiles ) < 1 )
    stop( "Data files not found. Check 'folder', 'baseName' and 'nExp' parameters." )

  cat( "Data files: ", myFiles, "\n" )

  # Read data from text files and format it as 4D array with labels
  mc <- read.4d.lsd( myFiles, col.names = firmVar, skip = iniDrop,
                     nrows = nKeep, nnodes = 1 )

  # Get dimensions information
  nTsteps <- dim( mc )[ 1 ]              # number of time steps
  nFirms <- dim( mc )[ 3 ]               # number of firms (instances)
  nSize  <- dim( mc )[ 4 ]               # Monte Carlo sample size

  # Add new variables names (not in LSD files)
  nFirmVarNew <- length( addFirmVar )   # number of new variables to add
  newFirmVar <- append( name.nice.lsd( firmVar ), addFirmVar ) # new label set
  nVar <- length( newFirmVar )          # number of variables (firm-level)

  # ------ Add new variables to data set ------

  # Increase array size to allow for new variables
  mc <- abind( mc, array( as.numeric( NA ),
                          dim = c( nTsteps, nFirmVarNew,
                                   nFirms, nSize ) ),
               along = 2, use.first.dimnames = TRUE )
  dimnames( mc )[[ 2 ]] <- newFirmVar

  # Compute values for new variables, preventing infinite values
  for( m in 1 : nSize )                 # for all MC samples (files)
    for( i in 1 : nTsteps ){            # all time steps
      # clean size vector
      Q2e <- as.vector( mc[ i, "Q2e", , m ] )
      Q2e <- Q2e[ is.finite( Q2e ) ]
      Q2e <- Q2e[ Q2e >= 1 ]
      mean.Q2e <- mean( Q2e, na.rm = TRUE )

      A2 <- as.vector( mc[ i, "A2", , m ] )
      A2 <- A2[ is.finite( A2 ) ]
      A2 <- A2[ A2 >= 1 ]
      mean.A2 <- mean( A2, na.rm = TRUE )

      w2realAvg <- as.vector( mc[ i, "w2realAvg", , m ] )
      w2realAvg <- w2realAvg[ is.finite( w2realAvg ) ]
      w2realAvg <- w2realAvg[ w2realAvg > 0 ]
      mean.w2realAvg <- mean( w2realAvg, na.rm = TRUE )

      s2avg <- as.vector( mc[ i, "s2avg", , m ] )
      s2avg <- s2avg[ is.finite( s2avg ) ]
      s2avg <- s2avg[ s2avg > 0 ]
      mean.s2avg <- mean( s2avg, na.rm = TRUE )

      tot2 <- as.vector( mc[ i, "L2", , m ] )
      tot2 <- tot2[ is.finite( tot2 ) ]
      tot2 <- tot2[ tot2 > 0 ]
      sum.tot2 <- sum( tot2, na.rm = TRUE )

      if( ! is.finite( mean.Q2e ) ) mean.Q2e <- 0
      if( ! is.finite( mean.A2 ) ) mean.A2 <- 0
      if( ! is.finite( mean.w2realAvg ) ) mean.w2realAvg <- 0
      if( ! is.finite( mean.s2avg ) ) mean.s2avg <- 0
      if( ! is.finite( sum.tot2 ) ) sum.tot2 <- 0

      for( j in 1 : nFirms ){            # and all firms (instances)

        # Take care of entrants' first period and other data problems to avoid artifacts
        if( is.na( mc[ i, "Q2e", j, m ] ) || mc[ i, "Q2e", j, m ] < 1 ||
            is.na( mc[ i, "A2", j, m ] ) || mc[ i, "A2", j, m ] < 1 ||
            is.na( mc[ i, "w2realAvg", j, m ] ) || mc[ i, "w2realAvg", j, m ] <= 0 ||
            is.na( mc[ i, "s2avg", j, m ] ) || mc[ i, "s2avg", j, m ] <= 0 ||
            is.na( mc[ i, "postChg", j, m ] ) || is.na( mc[ i, "Q2e", j, m ] ) ||
            is.na( mc[ i, "L2", j, m ] ) || is.na( mc[ i, "postChg", j, m ] ) ) {
          mc[ i, , j, m ] <- rep( NA, nVar )
          next
        }

        # Normalization of key variables using the period average size
        if( mean.Q2e != 0 )
          mc[ i, "normO2", j, m ] <- mc[ i, "Q2e", j, m ] / mean.Q2e
        if( mean.A2 != 0 )
          mc[ i, "normA2", j, m ] <- mc[ i, "A2", j, m ] / mean.A2
        if( mean.w2realAvg != 0 )
          mc[ i, "normRW2", j, m ] <- mc[ i, "w2realAvg", j, m ] / mean.w2realAvg
        if( mean.s2avg != 0 )
          mc[ i, "normSK2", j, m ] <- mc[ i, "s2avg", j, m ] / mean.s2avg

        # FHK decomposition variables
        if( mc[ i, "Q2e", j, m ] > 0 && mc[ i, "L2", j, m ] > 0 ) {
          mc[ i, "A2d", j, m ] <- mc[ i, "Q2e", j, m ] / mc[ i, "L2", j, m ]
          mc[ i, "lA2", j, m ] <- log( mc[ i, "Q2e", j, m ] / mc[ i, "L2", j, m ] )

          if( sum.tot2 > 0 )
            mc[ i, "f2l", j, m ] <- mc[ i, "L2", j, m ] / sum.tot2
        }

        # wage x productivity regression extra variable
        if( mc[ i, "w2realAvg", j, m ] > 0 )
          mc[ i, "lrwAvg2", j, m ] <- log( mc[ i, "w2realAvg", j, m ] )

        # Growth rates and deltas are calculated only from 2nd period and for non-entrant firms
        if( i > 1 ) {
          if( is.finite( mc[ i - 1, "Q2e", j, m ] ) ) {

            # Size, normalized sales and productivity growth
            mc[ i, "growth2", j, m ] <- log( mc[ i, "Q2e", j, m ] ) -
                                                log( mc[ i - 1, "Q2e", j, m ] )
            mc[ i, "normO2grow", j, m ] <- log( mc[ i, "normO2", j, m ] ) -
                                                log( mc[ i - 1, "normO2", j, m ] )
            mc[ i, "normA2grow", j, m ] <- log( mc[ i, "normA2", j, m ] ) -
                                                log( mc[ i - 1, "normA2", j, m ] )
            mc[ i, "normRW2grow", j, m ] <- log( mc[ i, "normRW2", j, m ] ) -
                                                 log( mc[ i - 1, "normRW2", j, m ] )
            mc[ i, "normSK2grow", j, m ] <- log( mc[ i, "normSK2", j, m ] ) -
                                                 log( mc[ i - 1, "normSK2", j, m ] )

            if( ! is.finite( mc[ i, "normO2grow", j, m ] ) )
              mc[ i, "normO2grow", j, m ] <- NA
            if( ! is.finite( mc[ i, "normA2grow", j, m ] ) )
              mc[ i, "normA2grow", j, m ] <- NA
            if( ! is.finite( mc[ i, "normRW2grow", j, m ] ) )
              mc[ i, "normRW2grow", j, m ] <- NA
            if( ! is.finite( mc[ i, "normSK2grow", j, m ] ) )
              mc[ i, "normSK2grow", j, m ] <- NA
          }
        }
      }
    }

  # Save temporary results to disk to save memory
  tmpFile <- paste0( folder, "/", baseName, exper, "_firm2_MC.Rdata" )
  save( mc, nTsteps, nVar, nSize, nFirms, file = tmpFile )

  return( tmpFile )
}

# load each experiment serially
tmpFiles <- lapply( 1 : nExp, readExp )

invisible( gc( verbose = FALSE ) )


#******************************************************************
#
# --------------------- Plot statistics -------------------------
#
#******************************************************************

# ====== User parameters ======

CI     <- 0.95      # desired confidence interval
outLim <- 0.10      # outlier percentile (0=don't remove outliers)
limOutl<- 0.10      # quantile extreme limits (0=none)
warmUp <- 300       # number of "warm-up" time steps
nTstat <- -1        # last period to consider for statistics (-1=all)
csBeg  <- 300       # beginning step for cross-section regressions
csEnd  <- 307       # last step for cross-section regressions

repName <- ""       # report files base name (if "" same baseName)
sDigits <- 4        # significant digits in tables
plotRows <- 1       # number of plots per row in a page
plotCols <- 1  	    # number of plots per column in a page
plotW <- 10         # plot window width
plotH <- 7          # plot window height

# Colors assigned to each experiment's lines in graphics
colors <- c( "black", "blue", "red", "orange", "green", "brown" )

# Line types assigned to each experiment
lTypes <- c( "solid", "solid", "solid", "solid", "solid", "solid" )

# Point types assigned to each experiment
pTypes <- c( 4, 4, 4, 4, 4, 4 )


# ====== External support functions & definitions ======

if( ! exists( "plot_norm", mode = "function" ) ) {     # already loaded?
  source( "KS-support-functions.R" )
}

# remove warnings for support functions
# !diagnostics suppress = logNA, log0, t.test0, se, bkfilter, adf.test, colSds
# !diagnostics suppress = plot_lognorm, plot_norm, plot_laplace, plot_lin
# !diagnostics suppress = fit_subbotin, comp_stats, comp_MC_stats, size_bins
# !diagnostics suppress = remove_outliers_table, all.NA, textplot, npreg
# !diagnostics suppress = FHK_decomp, DN_decomp, plot_epanechnikov, nCores

# ====== Support stuff ======

if( repName == "" )
  repName <- baseName

# Generate fancy labels & build labels list legend
legends <- vector( )
legendList <- "Experiments: "
for( k in 1 : nExp ) {
  if( is.na( expVal[ k ]) || expVal[ k ] == "" )
    legends[ k ] <- paste( "Case", k )
  else
    legends[ k ] <- expVal[ k ]
  if( k != 1 )
    legendList <- paste0( legendList, ",  " )
  legendList <- paste0( legendList, "[", k, "] ", legends[ k ] )
}

# load data from first experiment
load( tmpFiles[[ 1 ]] )

# Number of periods to show in graphics and use in statistics
if( nTstat < 1 || nTstat > nTsteps || nTstat <= warmUp )
  nTstat <- nTsteps
TmaskStat <- ( warmUp + 1 ) : nTstat
if( csBeg < 1 || csBeg >= nTsteps || csBeg < warmUp )
  csBeg <- warmUp
if( csEnd < 1 || csEnd >= nTsteps || csEnd <= warmUp || csEnd <= csBeg )
  csEnd <- nTsteps - 1


# ====== Main code ======

# Open PDF plot file for output
pdf( paste0( folder, "/", repName, "_firm2_MC.pdf" ),
     width = plotW, height = plotH )
par( mfrow = c ( plotRows, plotCols ) )   # define plots per page

for( k in 1 : nExp ){             # do for each experiment

  # load pooled data from temporary files (first already loaded)
  if( k > 1 )
    load( tmpFiles[[ k ]] )

  file.remove( tmpFiles[[ k ]] )      # delete temporary file

  #
  # ------ Create MC statistics table ------
  #

  oMC <- AMC <- wMC <- sMC <- gMC <- ngMC <- ngAMC <-
    nwgMC <- nsgMC <- exMC <- list( )

  for( l in 1 : nSize ){          # for each MC run

    # ------ Update MC statistics lists ------

    oData <- logNA( mc[ TmaskStat, "Q2e", , l ] )
    Adata <- logNA( mc[ TmaskStat, "A2", , l ] )
    wData <- logNA( mc[ TmaskStat, "w2realAvg", , l ] )
    sData <- logNA( mc[ TmaskStat, "s2avg", , l ] )
    gData <- mc[ TmaskStat, "growth2", , l ]
    ngData <- mc[ TmaskStat, "normO2grow", , l ]
    ngAdata <- mc[ TmaskStat, "normA2grow", , l ]
    nwgData <- mc[ TmaskStat, "normRW2grow", , l ]
    nsgData <- mc[ TmaskStat, "normSK2grow", , l ]
    exData <- as.numeric( is.na( mc[ TmaskStat, "growth2", , l ] ) )

    # compute each variable statistics
    stats <- lapply( list( oData, Adata, wData, sData, gData, ngData,
                           ngAdata, nwgData, nsgData, exData ), comp_stats )

    oMC[[l]] <- stats[[ 1 ]]
    AMC[[l]] <- stats[[ 2 ]]
    wMC[[l]] <- stats[[ 3 ]]
    sMC[[l]] <- stats[[ 4 ]]
    gMC[[l]] <- stats[[ 5 ]]
    ngMC[[l]] <- stats[[ 6 ]]
    ngAMC[[l]] <- stats[[ 7 ]]
    nwgMC[[l]] <- stats[[ 8 ]]
    nsgMC[[l]] <- stats[[ 9 ]]
    exMC[[l]] <- stats[[ 10 ]]
  }

  # ------ Compute MC statistics vectors ------

  # compute each variable statistics
  stats <- lapply( list( oMC, AMC, wMC, sMC, gMC, ngMC, ngAMC, nsgMC,
                         nwgMC, exMC ), comp_MC_stats )

  o <- stats[[ 1 ]]
  A <- stats[[ 2 ]]
  w <- stats[[ 3 ]]
  s <- stats[[ 4 ]]
  g <- stats[[ 5 ]]
  ng <- stats[[ 6 ]]
  ngA <- stats[[ 7 ]]
  nsg <- stats[[ 8 ]]
  nwg <- stats[[ 9 ]]
  ex <- stats[[ 10 ]]

  # ------ Build statistics table ------

  key.stats <- matrix( c( o$avg$avg, A$avg$avg, w$avg$avg, s$avg$avg,
                          g$avg$avg, ng$avg$avg,
                          ngA$avg$avg, nwg$avg$avg, nsg$avg$avg, ex$avg$avg,

                          o$se$avg, A$se$avg, w$se$avg, s$se$avg,
                          g$se$avg, ng$se$avg, ngA$se$avg,
                          nwg$se$avg, nsg$se$avg, ex$se$avg,

                          o$sd$avg, A$sd$avg, w$sd$avg, s$sd$avg,
                          g$sd$avg, ng$sd$avg, ngA$sd$avg,
                          nwg$sd$avg, nsg$sd$avg, ex$sd$avg,

                          o$avg$subbo$b, A$avg$subbo$b,
                          w$avg$subbo$b, s$avg$subbo$b, g$avg$subbo$b,
                          ng$avg$subbo$b,
                          ngA$avg$subbo$b, nwg$avg$subbo$b, nsg$avg$subbo$b,
                          ex$avg$subbo$b,

                          o$se$subbo$b, A$se$subbo$b, w$se$subbo$b,
                          s$se$subbo$b, g$se$subbo$b, ng$se$subbo$b,
                          ngA$se$subbo$b, nwg$se$subbo$b,
                          nsg$se$subbo$b, ex$se$subbo$b,

                          o$sd$subbo$b, A$sd$subbo$b, w$sd$subbo$b,
                          s$sd$subbo$b, g$sd$subbo$b, ng$sd$subbo$b,
                          ngA$sd$subbo$b, nwg$sd$subbo$b,
                          nsg$sd$subbo$b, ex$sd$subbo$b,

                          o$avg$subbo$a, A$avg$subbo$a,
                          w$avg$subbo$a, s$avg$subbo$a, g$avg$subbo$a,
                          ng$avg$subbo$a,
                          ngA$avg$subbo$a, nwg$avg$subbo$a, nsg$avg$subbo$a,
                          ex$avg$subbo$a,

                          o$se$subbo$a, A$se$subbo$a, w$se$subbo$a,
                          s$se$subbo$a, g$se$subbo$a, ng$se$subbo$a,
                          ngA$se$subbo$a, nwg$se$subbo$a,
                          nsg$se$subbo$a, ex$se$subbo$a,

                          o$sd$subbo$a, A$sd$subbo$a, w$sd$subbo$a,
                          s$sd$subbo$a, g$sd$subbo$a, ng$sd$subbo$a,
                          ngA$sd$subbo$a, nwg$sd$subbo$a,
                          nsg$sd$subbo$a, ex$sd$subbo$a,

                          o$avg$subbo$m, A$avg$subbo$m,
                          w$avg$subbo$m, s$avg$subbo$m, g$avg$subbo$m,
                          ng$avg$subbo$m,
                          ngA$avg$subbo$m, nwg$avg$subbo$m, nsg$avg$subbo$m,
                          ex$avg$subbo$m,

                          o$se$subbo$m, A$se$subbo$m, w$se$subbo$m,
                          s$se$subbo$m, g$se$subbo$m, ng$se$subbo$m,
                          ngA$se$subbo$m, nwg$se$subbo$m,
                          nsg$se$subbo$m, ex$se$subbo$m,

                          o$sd$subbo$m, A$sd$subbo$m, w$sd$subbo$m,
                          s$sd$subbo$m, g$sd$subbo$m, ng$sd$subbo$m,
                          ngA$sd$subbo$m, nwg$sd$subbo$m,
                          nsg$sd$subbo$m, ex$sd$subbo$m,

                          o$avg$ac$t1, A$avg$ac$t1, w$avg$ac$t1,
                          s$avg$ac$t1, g$avg$ac$t1, ng$avg$ac$t1,
                          ngA$avg$ac$t1, nwg$avg$ac$t1, nsg$avg$ac$t1,
                          ex$avg$ac$t1,

                          o$se$ac$t1, A$se$ac$t1, w$se$ac$t1,
                          s$se$ac$t1, g$se$ac$t1, ng$se$ac$t1,
                          ngA$se$ac$t1, nwg$se$ac$t1, nsg$se$ac$t1,
                          ex$se$ac$t1,

                          o$sd$ac$t1, A$sd$ac$t1, w$sd$ac$t1,
                          s$sd$ac$t1, g$sd$ac$t1, ng$sd$ac$t1,
                          ngA$sd$ac$t1, nwg$sd$ac$t1, nsg$sd$ac$t1,
                          ex$sd$ac$t1,

                          o$avg$ac$t2, A$avg$ac$t2, w$avg$ac$t2,
                          s$avg$ac$t2, g$avg$ac$t2, ng$avg$ac$t2,
                          ngA$avg$ac$t2, nwg$avg$ac$t2, nsg$avg$ac$t2,
                          ex$avg$ac$t2,

                          o$se$ac$t2, A$se$ac$t2, w$se$ac$t2,
                          s$se$ac$t2, g$se$ac$t2, ng$se$ac$t2,
                          ngA$se$ac$t2, nwg$se$ac$t2, nsg$se$ac$t2,
                          ex$se$ac$t2,

                          o$sd$ac$t2, A$sd$ac$t2, g$sd$ac$t2,
                          w$sd$ac$t2, s$sd$ac$t2, ng$sd$ac$t2,
                          ngA$sd$ac$t2, nwg$sd$ac$t2, nsg$sd$ac$t2,
                          ex$sd$ac$t2 ),

                       ncol = 10, byrow = TRUE )
  colnames( key.stats ) <- c( "Output(log)", "Prod.(log)", "R.Wage(log)",
                              "Skills(log)", "Output Gr.", "N.Output Gr.",
                              "N.Prod.Gr.", "N.R.Wage Gr.", "N.Skills Gr.", "Exit Rate" )
  rownames( key.stats ) <- c( "average", " (s.e.)", " (s.d.)", "Subbotin b",
                              " (s.e.)", " (s.d.)", "Subbotin a", " (s.e.)",
                              " (s.d.)", "Subbotin m", " (s.e.)", " (s.d.)",
                              "autocorr. t-1", " (s.e.)",  " (s.d.)",
                              "autocorr. t-2", " (s.e.)",  " (s.d.)" )

  textplot( formatC( key.stats, digits = sDigits, format = "g" ), cmar = 1.0 )
  title <- paste( "Monte Carlo firm-level statistics (", legends[ k ], ")" )
  subTitle <- paste( eval( bquote( paste0( "( Sample size = ", nFirms[ k ],
                                           " firms / MC runs = ", nSize,
                                        " / Period = ", warmUp + 1, "-",
                                        nTstat, " )" ) ) ),
                    sector, sep ="\n" )
  title( main = title, sub = subTitle )
}


# Close PDF plot file
dev.off( )
