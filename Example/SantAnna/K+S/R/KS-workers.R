#******************************************************************
#
# ---------------- K+S Worker-level analysis --------------------
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

# Worker-level variables to use
workerVar <- c( "_wReal", "_s", "_Tu" )
addWorkerVar <- c( "normRW", "normSK", "normTU", "normRWgrow", "normSKgrow", "normTUgrow" )


# ==== Process LSD result files ====

# Package with LSD interface functions
library( LSDinterface, verbose = FALSE, quietly = TRUE )
library( abind, verbose = FALSE, quietly = TRUE )

# remove warnings for saved data
# !diagnostics suppress = mc, pool, nSize, nTsteps

# ---- Read data files ----

nWorkerVarNew <- length( addWorkerVar ) # number of new variables to add
newWorkerVar <- append( name.nice.lsd( workerVar ), addWorkerVar ) # new label set
nVar <- length( newWorkerVar )          # number of variables (worker-level)

# Function to read one experiment data (to be parallelized)
readExp <- function( exper ) {
  if( nExp > 1 ) {
    myFiles <- list.files( path = folder, pattern = paste0( baseName, exper, "_[0-9]+.res" ),
                           full.names = TRUE )
  } else {
    myFiles <- list.files( path = folder, pattern = paste0( baseName, "_[0-9]+.res" ),
                           full.names = TRUE )
  }

  if( length( myFiles ) < 1 )
    stop( "Data files not found. Check 'folder', 'baseName' and 'nExp' parameters." )

  cat( "Data files: ", myFiles, "\n" )

  # Read data from text files and format it as 4D array with labels
  mc <- read.4d.lsd( myFiles, col.names = workerVar, skip = iniDrop,
                     nrows = nKeep, nnodes = 1 )

  # Get dimensions information
  nTsteps <- dim( mc )[ 1 ]              # number of time steps
  nWorkers <- dim( mc )[ 3 ]             # number of workers (instances)
  nSize  <- dim( mc )[ 4 ]               # Monte Carlo sample size

  # Add new variables names (not in LSD files)
  nWorkerVarNew <- length( addWorkerVar )# number of new variables to add
  newWorkerVar <- append( name.nice.lsd( workerVar ), addWorkerVar ) # new label set
  nVar <- length( newWorkerVar )         # number of variables (worker-level)

  # ------ Add new variables to data set ------

  # Increase array size to allow for new variables
  mc <- abind( mc, array( as.numeric(NA),
                          dim = c( nTsteps, nWorkerVarNew,
                                   nWorkers, nSize ) ),
               along = 2, use.first.dimnames = TRUE )
  dimnames( mc )[[ 2 ]] <- newWorkerVar

  # Compute values for new variables, preventing infinite values
  for( m in 1 : nSize )                   # for all MC samples (files)
    for( i in 1 : nTsteps ){              # all time steps
      wReal <- as.vector( mc[ i, "wReal", , m ] )
      wReal <- wReal[ is.finite( wReal ) ]
      wReal <- wReal[ wReal > 0 ]
      mean.wReal <- mean( wReal, na.rm = TRUE )

      s <- as.vector( mc[ i, "s", , m ] )
      s <- s[ is.finite( s ) ]
      s <- s[ s > 0 ]
      mean.s <- mean( s, na.rm = TRUE )

      Tu <- as.vector( mc[ i, "Tu", , m ] )
      Tu <- Tu[ is.finite( Tu ) ]
      Tu <- Tu[ Tu >= 0 ]
      mean.Tu <- mean( Tu, na.rm = TRUE )

      if( ! is.finite( mean.wReal ) ) mean.wReal <- 0
      if( ! is.finite( mean.s ) ) mean.s <- 0
      if( ! is.finite( mean.Tu ) ) mean.Tu <- 0

      for( j in 1 : nWorkers ){           # and all workers (instances)

        # Change zero values to NA to avoid atifacts in statistics
        if( mc[ i, "wReal", j, m ] <= 0 )
          mc[ i, "wReal", j, m ] <- NA
        if( mc[ i, "s", j, m ] <= 0 )
          mc[ i, "s", j, m ] <- NA
        if( mc[ i, "Tu", j, m ] < 0 )
          mc[ i, "Tu", j, m ] <- NA

        # Normalization of variables using the period average size
        if( mean.wReal != 0 )
          mc[ i, "normRW", j, m ] <- mc[ i, "wReal", j, m ] / mean.wReal
        if( mean.s != 0 )
          mc[ i, "normSK", j, m ] <- mc[ i, "s", j, m ] / mean.s
        if( mean.Tu != 0 )
          mc[ i, "normTU", j, m ] <- mc[ i, "Tu", j, m ] / mean.Tu

        # Growth rates are calculated only from 2nd period
        if( i > 1 ){
          if( ! is.na( mc[ i - 1, "normRW", j, m ] ) && ! is.na( mc[ i, "normRW", j, m ] ) )
            mc[ i, "normRWgrow", j, m ] <- log( mc[ i, "normRW", j, m ] ) -
                                                log( mc[ i - 1, "normRW", j, m ] )
          if( ! is.na( mc[ i - 1, "normSK", j, m ] ) && ! is.na( mc[ i, "normSK", j, m ] ) )
            mc[ i, "normSKgrow", j, m ] <- log( mc[ i, "normSK", j, m ] ) -
                                                log( mc[ i - 1, "normSK", j, m ] )
          if( ! is.na( mc[ i - 1, "normTU", j, m ] ) && ! is.na( mc[ i, "normTU", j, m ] ) )
            mc[ i, "normTUgrow", j, m ] <- log( mc[ i, "normTU", j, m ] ) -
                                                log( mc[ i - 1, "normTU", j, m ] )

          if( is.infinite( mc[ i, "normRWgrow", j, m ] ) )
            mc[ i, "normRWgrow", j, m ] <- NA
          if( is.infinite( mc[ i, "normSKgrow", j, m ] ) )
            mc[ i, "normSKgrow", j, m ] <- NA
          if( is.infinite( mc[ i, "normTUgrow", j, m ] ) )
            mc[ i, "normTUgrow", j, m ] <- NA
        }
      }
    }

  # ---- Reorganize and save data ----

  # Create "flatter" 3D arrray, appending workers from different MC runs in sequence
  pool <- array( as.numeric( NA ), dim = c( nTsteps, nVar, nWorkers * nSize ),
                 dimnames = list( c( ( iniDrop + 1 ) : ( iniDrop + nTsteps ) ),
                                  newWorkerVar, c( 1 : ( nWorkers * nSize ) ) ) )
  l <- rep( 1, nVar )                   # absolute variable instance (worker) counter

  for( m in 1 : nSize )                 # MC sample
    for( j in 1 : nVar )                # for all variables
      for( i in 1 : nWorkers ){           # for all workers
        pool[ , j, l[ j ]] <- mc[ , j, i, m ]
        l[ j ] <- l[ j ] + 1
      }

  # Save temporary results to disk to save memory
  tmpFile <- paste0( folder, "/", baseName, exper, "_worker.Rdata" )
  save( mc, pool, nTsteps, nVar, nSize, nWorkers, file = tmpFile )

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

CI     <- 0.95   # desired confidence interval
nBins  <- 15     # number of bins to use in histograms
outLim <- 0.001  # outlier percentile (0=don't remove outliers)
warmUp <- 300    # number of "warm-up" runs
nTstat <- -1     # last period to consider for statistics (-1=all)

repName <- ""    # report files base name (if "" same baseName)
sDigits <- 4     # significant digits in tables
plotRows <- 1    # number of plots per row in a page
plotCols <- 1  	 # number of plots per column in a page
plotW <- 10      # plot window width
plotH <- 7       # plot window height

# Colors assigned to each experiment's lines in graphics
colors <- c( "black", "blue", "red", "orange", "green", "brown" )

# Line types assigned to each experiment
lTypes <- c( "solid", "solid", "solid", "solid", "solid", "solid" )

# Point types assigned to each experiment
pTypes <- c( 4, 4, 4, 4, 4, 4 )


# ====== External support functions & definitions ======

if( ! exists( "plot_norm", mode = "function" ) )     # already loaded?
  source( "KS-support-functions.R" )
# remove warnings for support functions
# !diagnostics suppress = logNA, log0, t.test0, se, bkfilter, adf.test, colSds
# !diagnostics suppress = plot_lognorm, plot_norm, plot_laplace, textplot
# !diagnostics suppress = fit_subbotin, comp_stats, comp_MC_stats, nCores


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


#
# ====== Do pooled analysis ======
#

# Open PDF plot file for output
pdf( paste0( folder, "/", repName, "_worker_pool.pdf" ),
     width = plotW, height = plotH )
par( mfrow = c ( plotRows, plotCols ) )             # define plots per page

wData <- sData <- tData <- nwData <- nsData <- ntData <-
  nwgData <- nsgData <- ntgData <- list( )

# run over all experiments individually
for( k in 1 : nExp ){

  # load pooled data from temporary files (first already loaded)
  if( k > 1 )
    load( tmpFiles[[ k ]] )

  rm( mc )                   # erase unused variables

  #
  # ------ Create data & statistics vectors ------
  #

  wData[[ k ]] <- as.vector( logNA( pool[ TmaskStat, "wReal", ] ) )
  sData[[ k ]] <- as.vector( logNA( pool[ TmaskStat, "s", ] ) )
  tData[[ k ]] <- as.vector( pool[ TmaskStat, "Tu", ] )
  nwData[[ k ]] <- as.vector( logNA( pool[ TmaskStat, "normRW", ] ) )
  nsData[[ k ]] <- as.vector( logNA( pool[ TmaskStat, "normSK", ] ) )
  ntData[[ k ]] <- as.vector( logNA( pool[ TmaskStat, "normTU", ] ) )
  nwgData[[ k ]] <- as.vector( pool[ TmaskStat, "normRWgrow", ] )
  nsgData[[ k ]] <- as.vector( pool[ TmaskStat, "normSKgrow", ] )
  ntgData[[ k ]] <- as.vector( pool[ TmaskStat, "normTUgrow", ] )

  # remove NAs
  nwData[[ k ]] <- nwData[[ k ]][ ! is.na( nwData[[ k ]] ) ]
  nsData[[ k ]] <- nsData[[ k ]][ ! is.na( nsData[[ k ]] ) ]
  ntData[[ k ]] <- ntData[[ k ]][ ! is.na( ntData[[ k ]] ) ]
  nwgData[[ k ]] <- nwgData[[ k ]][ ! is.na( nwgData[[ k ]] ) ]
  nsgData[[ k ]] <- nsgData[[ k ]][ ! is.na( nsgData[[ k ]] ) ]
  ntgData[[ k ]] <- ntgData[[ k ]][ ! is.na( ntgData[[ k ]] ) ]

  # compute each variable statistics in parallel
  stats <- lapply( list( nwData[[ k ]], nsData[[ k ]], ntData[[ k ]],
                         nwgData[[ k ]], nsgData[[ k ]], ntgData[[ k ]] ),
                   comp_stats )

  nw <- stats[[ 1 ]]
  ns <- stats[[ 2 ]]
  nt <- stats[[ 3 ]]
  nwg <- stats[[ 4 ]]
  nsg <- stats[[ 5 ]]
  ntg <- stats[[ 6 ]]

  #
  # ------ Build statistics table ------
  #

  key.stats <- matrix( c( nw$avg, ns$avg, nt$avg, nwg$avg, nsg$avg, ntg$avg,

                          nw$sd / sqrt( nSize ), ns$sd / sqrt( nSize ), nt$sd / sqrt( nSize ),
                          nwg$sd / sqrt( nSize ), nsg$sd / sqrt( nSize ), ntg$sd / sqrt( nSize ),

                          nw$sd, ns$sd, nt$sd, nwg$sd, nsg$sd, ntg$sd,

                          nw$subbo$b, ns$subbo$b, nt$subbo$b,
                          nwg$subbo$b, nsg$subbo$b, ntg$subbo$b,

                          nw$subbo$a, ns$subbo$a, nt$subbo$a,
                          nwg$subbo$a, nsg$subbo$a, ntg$subbo$a,

                          nw$subbo$m, ns$subbo$m, nt$subbo$m,
                          nwg$subbo$m, nsg$subbo$m, ntg$subbo$m,

                          nw$jb$statistic, ns$jb$statistic, nt$jb$statistic,
                          nwg$jb$statistic, nsg$jb$statistic, ntg$jb$statistic,

                          nw$jb$p.value, ns$jb$p.value, nt$jb$p.value,
                          nwg$jb$p.value, nsg$jb$p.value, ntg$jb$p.value,

                          nw$ll$statistic, ns$ll$statistic, nt$ll$statistic,
                          nwg$ll$statistic, nsg$ll$statistic, ntg$ll$statistic,

                          nw$ll$p.value, ns$ll$p.value, nt$ll$p.value,
                          nwg$ll$p.value, nsg$ll$p.value, ntg$ll$p.value,

                          nw$ad$statistic, ns$ad$statistic, nt$ad$statistic,
                          nwg$ad$statistic, nsg$ad$statistic, ntg$ad$statistic,

                          nw$ad$p.value, ns$ad$p.value, nt$ad$p.value,
                          nwg$ad$p.value, nsg$ad$p.value, ntg$ad$p.value,

                          nw$ac$t1, ns$ac$t1, nt$ac$t1,
                          nwg$ac$t1, nsg$ac$t1, ntg$ac$t1,

                          nw$ac$t2, ns$ac$t2, nt$ac$t2,
                          nwg$ac$t2, nsg$ac$t2, ntg$ac$t2 ),

                       ncol = 6, byrow = TRUE )
  colnames( key.stats ) <- c( "N.R.Wage(log)", "N.Skills(log)", "N.T.Unemp.(log)", "N.R.Wage Gr.",
                              "N.Skills Gr.", "N.T.Unemp.Gr." )
  rownames( key.stats ) <- c( "average", " (s.e.)", " (s.d.)", "Subbotin b", " a", " m", "Jarque-Bera X2",
                              " (p-val.)", "Lilliefors D", " (p-val.)", "Anderson-Darling t", " (p-val.)",
                              "autocorr. t-1", "autocorr. t-2" )

  textplot( formatC( key.stats, digits = sDigits, format = "g" ), cmar = 1.0 )
  title <- paste( "Pooled worker-level statistics (", legends[ k ], ")" )
  subTitle <- paste( eval( bquote( paste0( "( Sample size = ", .( nWorkers * nSize ),
                                           " workers / Period = ", .( warmUp + 1 ), "-",
                                           .( nTstat ), " )" ) ) ),
                     sep ="\n" )
  title( main = title, sub = subTitle )
}

#
# ====== Plot distributions ( overplots )  ======
#

# ------ Wages, skills & unemployment times distributions ( binned density x variable in log level )  ------

plot_norm( nwData, "log(normalized real wage)", "Binned density",
           "Pooled wages distribution ( all experiments )",
           "", outLim, nBins, legends, colors, lTypes, pTypes )

plot_norm( nsData, "log(normalized skills level)", "Binned density",
           "Pooled skills distribution ( all experiments )",
           "", outLim, nBins, legends, colors, lTypes, pTypes )

plot_norm( ntData, "log(normalized time unemployed)", "Binned density",
           "Pooled unemployment time distribution ( all experiments )",
           "", outLim, nBins, legends, colors, lTypes, pTypes )

plot_norm( tData, "Time unemployed", "Binned density",
           "Pooled unemployment time distribution ( all experiments )",
           "", outLim, nBins, legends, colors, lTypes, pTypes )

# ------ Wages, skills & unemployment times distributions distributions ( log variable x rank )  ------

plot_lognorm( lapply( nwData, exp ), "Normalized real wage", "Rank",
              "Pooled wage distribution ( all experiments )",
              "", outLim, nBins, legends, colors, lTypes )

plot_lognorm( lapply( nsData, exp ), "Normalized skills level", "Rank",
              "Pooled skills distribution ( all experiments )",
              "", outLim, nBins, legends, colors, lTypes )

plot_lognorm( lapply( nsData, exp ), "Normalized time unemployed", "Rank",
              "Pooled unemployment time distribution ( all experiments )",
              "", outLim, nBins, legends, colors, lTypes )

# ------ Growth rate distributions ( binned density x log growth rate )  ------

plot_laplace( nwgData, "Normalized real wage growth rate", "Binned density",
              "Pooled wage growth distribution ( all experiments )",
              "", outLim, nBins, legends, colors, lTypes, pTypes )

plot_laplace( nsgData, "Normalized skills level growth rate", "Binned density",
              "Pooled skills growth distribution ( all experiments )",
              "", outLim, nBins, legends, colors, lTypes, pTypes )

plot_laplace( ntgData, "Normalized time unemployed growth rate", "Binned density",
              "Pooled unemployment time growth distribution ( all experiments )",
              "", outLim, nBins, legends, colors, lTypes, pTypes )

# free memory
rm( pool, wData, sData, tData, nwData, nsData, ntData, nwgData, nsgData, ntgData )
invisible( gc( verbose = FALSE ) )

# Close plot file
dev.off()

#
# ===== Do sensitivity analysis =====
#

# Open PDF plot file for output
pdf(paste0( folder, "/", repName, "_worker_MC.pdf" ),
    width = plotW, height = plotH )
par( mfrow = c ( plotRows, plotCols ) )             # define plots per page

for( k in 1 : nExp ){             # do for each experiment

  # load MC data from temporary files
  load( tmpFiles[[ k ]] )
  file.remove( tmpFiles[[ k ]] )  # delete temporary file
  rm( pool )          # erase unused variables

  #
  # ------ Create MC statistics table ------
  #

  wMC <- sMC <- tMC <- nwMC <- nsMC <- ntMC <- nwgMC <- nsgMC <- ntgMC <- list( )

  for( l in 1 : nSize ){          # for each MC run

    # ------ Update MC statistics lists ------

    wData <- logNA( mc[ TmaskStat, "wReal", , l ] )
    sData <- logNA( mc[ TmaskStat, "s", , l ] )
    tData <- logNA( mc[ TmaskStat, "Tu", , l ] )
    nwgData <- mc[ TmaskStat, "normRWgrow", , l ]
    nsgData <- mc[ TmaskStat, "normSKgrow", , l ]
    ntgData <- mc[ TmaskStat, "normTUgrow", , l ]

    # compute each variable statistics
    stats <- lapply( list( wData, sData, tData, nwgData, nsgData,
                           ntgData ), comp_stats )

    wMC[[l]] <- stats[[ 1 ]]
    sMC[[l]] <- stats[[ 2 ]]
    tMC[[l]] <- stats[[ 3 ]]
    nwgMC[[l]] <- stats[[ 4 ]]
    nsgMC[[l]] <- stats[[ 5 ]]
    ntgMC[[l]] <- stats[[ 6 ]]
  }

  # ------ Compute MC statistics vectors ------

  # compute each variable statistics
  stats <- lapply( list( wMC, sMC, tMC, nwgMC, nsgMC, ntgMC ),
                   comp_MC_stats )

  w <- stats[[ 1 ]]
  s <- stats[[ 2 ]]
  t <- stats[[ 3 ]]
  nwg <- stats[[ 4 ]]
  nsg <- stats[[ 5 ]]
  ntg <- stats[[ 6 ]]

  # ------ Build statistics table ------

  key.stats <- matrix( c( w$avg$avg, s$avg$avg, t$avg$avg,
                          nwg$avg$avg, nsg$avg$avg, ntg$avg$avg,

                          w$se$avg, s$se$avg, t$se$avg,
                          nwg$se$avg, nsg$se$avg, ntg$se$avg,

                          w$sd$avg, s$sd$avg, t$sd$avg,
                          nwg$sd$avg, nsg$sd$avg, ntg$sd$avg,

                          w$avg$subbo$b, s$avg$subbo$b, t$avg$subbo$b,
                          nwg$avg$subbo$b, nsg$avg$subbo$b, ntg$avg$subbo$b,

                          w$se$subbo$b, s$se$subbo$b, t$se$subbo$b,
                          nwg$se$subbo$b, nsg$se$subbo$b, ntg$se$subbo$b,

                          w$sd$subbo$b, s$sd$subbo$b, t$sd$subbo$b,
                          nwg$sd$subbo$b, nsg$sd$subbo$b, ntg$sd$subbo$b,

                          w$avg$subbo$a, s$avg$subbo$a, t$avg$subbo$a,
                          nwg$avg$subbo$a, nsg$avg$subbo$a, ntg$avg$subbo$a,

                          w$se$subbo$a, s$se$subbo$a, t$se$subbo$a,
                          nwg$se$subbo$a, nsg$se$subbo$a, ntg$se$subbo$a,

                          w$sd$subbo$a, s$sd$subbo$a, t$sd$subbo$a,
                          nwg$sd$subbo$a, nsg$sd$subbo$a, ntg$sd$subbo$a,

                          w$avg$subbo$m, s$avg$subbo$m, t$avg$subbo$m,
                          nwg$avg$subbo$m, nsg$avg$subbo$m, ntg$avg$subbo$m,

                          w$se$subbo$m, s$se$subbo$m, t$se$subbo$m,
                          nwg$se$subbo$m, nsg$se$subbo$m, ntg$se$subbo$m,

                          w$sd$subbo$m, s$sd$subbo$m, t$sd$subbo$m,
                          nwg$sd$subbo$m, nsg$sd$subbo$m, ntg$sd$subbo$m,

                          w$avg$ac$t1, s$avg$ac$t1, t$avg$ac$t1,
                          nwg$avg$ac$t1, nsg$avg$ac$t1, ntg$avg$ac$t1,

                          w$se$ac$t1, s$se$ac$t1, t$se$ac$t1,
                          nwg$se$ac$t1, nsg$se$ac$t1, ntg$se$ac$t1,

                          w$sd$ac$t1, s$sd$ac$t1, t$sd$ac$t1,
                          nwg$sd$ac$t1, nsg$sd$ac$t1, ntg$sd$ac$t1,

                          w$avg$ac$t2, s$avg$ac$t2, t$avg$ac$t2,
                          nwg$avg$ac$t2, nsg$avg$ac$t2, ntg$avg$ac$t2,

                          w$se$ac$t2, s$se$ac$t2, t$se$ac$t2,
                          nwg$se$ac$t2, nsg$se$ac$t2, ntg$se$ac$t2,

                          w$sd$ac$t2, s$sd$ac$t2, t$sd$ac$t2,
                          nwg$sd$ac$t2, nsg$sd$ac$t2, ntg$sd$ac$t2 ),

                       ncol = 6, byrow = TRUE )
  colnames( key.stats ) <- c( "N.R.Wage(log)", "N.Skills(log)", "N.T.Unemp.(log)",
                              "N.R.Wage Gr.", "N.Skills Gr.", "N.T.Unemp.Gr." )
  rownames( key.stats ) <- c( "average", " (s.e.)", " (s.d.)", "Subbotin b", " (s.e.)", " (s.d.)",
                              "Subbotin a", " (s.e.)",  " (s.d.)", "Subbotin m", " (s.e.)", " (s.d.)",
                              "autocorr. t-1", " (s.e.)",  " (s.d.)", "autocorr. t-2", " (s.e.)",  " (s.d.)" )

  textplot( formatC( key.stats, digits = sDigits, format = "g" ), cmar = 1.0 )
  title <- paste( "Monte Carlo worker-level statistics (", legends[ k ], ")" )
  subTitle <- paste( eval( bquote( paste0( "( Sample size = ", .( nWorkers ), " workers / MC runs = ", nSize,
                                        " / Period = ", .( warmUp + 1 ), "-", .( nTstat ), " )" ) ) ),
                     sep ="\n" )
  title( main = title, sub = subTitle )
}

# Close PDF plot file
dev.off( )
