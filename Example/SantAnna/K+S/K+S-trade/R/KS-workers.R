#******************************************************************
#
# ------------------- K+S worker analysis -----------------------
#
#   Written by Marcelo C. Pereira, University of Campinas
#
#   Copyright Marcelo C. Pereira
#   Distributed under the GNU General Public License
#
#   The default configuration assumes that the supplied LSD
#   simulation configurations (basename Sim):
#     R/data/Sim1.lsd
#     R/data/Sim2.lsd
#   are executed before this script is used.
#
#   To execute the simulations, (1) open LSD Model Manager (LMM),
#   (2) in LSD Model Browser, open the model which contains this
#   script (double click), (3) in LMM, compile and run the model
#   (menu Model>Compile and Run), (4) in LSD Model Browser, load
#   the desired configuration (menu File>Load), (5) execute the
#   configuration (menu Run>Run or Run> Parallel Run), accepting
#   the defaults (parallel run is optional but typically saves
#   significant execution time).
#
#   IMPORTANT: this script assumes the R working directory is set
#   to the R subfolder where this script file is stored. This can
#   be done automatically in RStudio if a project is created at
#   this subfolder, or using the command setwd(). The "folder"
#   variable below must always point to a (relative) subfolder
#   of the R working directory.
#
#******************************************************************

#******************************************************************
#
# ------------ Read Monte Carlo experiment files ----------------
#
#******************************************************************

# ====== User parameters ======

# data import parameters
folder    <- "data"                 # data files folder
baseName  <- "Sim"                  # data files base name (same as .lsd file)
nExp      <- 2                      # number of experiments/countries
mCnt      <- FALSE                  # experiments from multiple countries?
nCnt      <- 1                      # country to use (mCnt = FALSE) (0=all)
iniDrop   <- 0                      # initial time steps to drop (0=none)
nKeep     <- -1                     # number of time steps to keep (-1=all)
coresExp  <- 0                      # max cores for experiments (0=all)
coresMC   <- 0                      # max cores for Monte Carlo (0=all)
savDat    <- FALSE                  # save data files and re-use if available?
mcStat    <- "mean"                 # Monte Carlo statistic ("mean", "median")
CI        <- 0.95                   # confidence level
bootR     <- 999                    # bootstrap replicates (bootCI != NULL)
bootCI    <- NULL                   # bootstrap confidence interval method (SLOW)
                                    # (NULL (no bootstrap), "basic", or "bca")

# caption and file names
expVal    <- c( "No shock", "Random shock" )              # experiment captions
cntVal    <- c( "Country 1", "Country 2" )                # country captions
caption   <- "K+S worker analysis"                        # caption for logs
datFilSfx <- "worker"                                     # data file suffix

# worker-level variables to import and add
origVar   <- c( "_Tu", "_s", "_wReal" )
addVar    <- c( "normRW", "normRWgrow", "normSK", "normSKgrow", "normTU",
                "normTUgrow" )


# ==== Libraries, support functions and global options ====

source( "KS-support-functions.R" )

options( warn = 0 )         # -1=no warning/0:warnings at end/2:warnings stop

# remove RStudio warnings for support functions
# !diagnostics suppress = showStartMark, showEndMark, setCmdLinePars, loadData
# !diagnostics suppress = catchError, startTime, mean.clean, vector.clean
# !diagnostics suppress = logNA, textplot, plot_lognorm, plot_norm, saveCSV
# !diagnostics suppress = plot_laplace, plot_quant, comp_stats, comp_MC_stats
# !diagnostics suppress = startCores, stopCores, autoLapply, clearTemp, repFile
# !diagnostics suppress = mc, pool, nElem, nElemMC, nTsteps, nSize, legends
# !diagnostics suppress = setLabels, outDir, expLeg, cntLeg, allLeg


# ==== Define function to create new variables ====

# function to add new variables to Monte Carlo sample (to be parallelized)
addVarFn <- function( mc, nElemMC, nTsteps, nVar ) {

  for( i in 1 : nTsteps ) {                # all time steps

    mean.wReal <- mean.clean( vector.clean( mc[ i, "wReal", ] ) )
    mean.s <- mean.clean( vector.clean( mc[ i, "s", ] ) )
    mean.Tu <- mean.clean( vector.clean( mc[ i, "Tu", ] ) )

    for( j in 1 : nElemMC ){             # and all element instances

      # change zero values to NA to avoid artifacts in statistics
      if( mc[ i, "wReal", j ] <= 0 )
        mc[ i, "wReal", j ] <- NA
      if( mc[ i, "s", j ] <= 0 )
        mc[ i, "s", j ] <- NA
      if( mc[ i, "Tu", j ] < 0 )
        mc[ i, "Tu", j ] <- NA

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


# ==== Process LSD result files ====

showStartMark( caption )  # log start mark

setCmdLinePars( )         # read command line parameters (if any)

# read LSD files or load existing temporary files according to the case
files <- loadData( savDat = savDat, nExp = nExp, mCnt = mCnt, nCnt = nCnt,
                   folder = folder, baseName = baseName, iniDrop = iniDrop,
                   nKeep = nKeep, origVar = origVar, addVar = addVar,
                   addVarFn = addVarFn, datFilSfx = datFilSfx, CI = CI,
                   coresExp = coresExp, coresMC = coresMC, mcStat = mcStat,
                   bootR = bootR, bootCI = bootCI )


#******************************************************************
#
# --------------------- Plot statistics -------------------------
#
#******************************************************************

# ====== User parameters ======

nBins     <- 15       # number of bins to use in histograms
outLim    <- 0.001    # outlier percentile (0=don't remove outliers)
warmUp    <- 300      # number of "warm-up" runs
nTstat    <- -1       # last period to consider for statistics (-1=all)

cores     <- 5        # maximum number of cores to allocate (0=all)
parStats  <- 10       # number of statistics to be computed in parallel
repName   <- ""       # report files base name (if "" same baseName)
sDigits   <- 4        # significant digits in tables
plotRows  <- 1        # number of plots per row in a page
plotCols  <- 1  	    # number of plots per column in a page
plotW     <- 10       # plot window width
plotH     <- 7        # plot window height
raster    <- FALSE    # raster or vector plots
res       <- 600      # resolution of raster mode (in dpi)

# colors assigned to each experiment's lines in graphics
colors <- c( "black", "blue", "red", "orange", "green", "brown", "cyan",
             "firebrick", "gray", "magenta", "orchid", "pink", "purple",
             "salmon", "sienna", "tomato", "turquoise", "violet", "yellow" )
#colors <- c( "black", "black", "black", "black", "black", "black" )

# line types assigned to each experiment
lTypes <- c( "solid", "solid", "solid", "solid", "solid", "solid" )
#lTypes <- c( "solid", "dashed", "dotted", "dotdash", "longdash", "twodash" )

# point types assigned to each experiment
pTypes <- c( 4, 4, 4, 4, 4, 4 )
#pTypes <- c( 4, 0, 1, 2, 3, 5 )


# ====== Support stuff ======

# generate labels & build labels list legend
setLabels( nExp, mCnt, nCnt )

# load data from first experiment
load( files[[ 1 ]]$pool )

# number of periods to show in graphics and use in statistics
if( nTstat < 1 || nTstat > nTsteps || nTstat <= warmUp )
  nTstat <- nTsteps
TmaskStat <- ( warmUp + 1 ) : nTstat


# ====== Monte Carlo stats function ======

# function to treat one Monte Carlo sample (to be parallelized)
statsMC <- function( nMC, exper, nSize, filesMC ) {

  cat( "  Monte Carlo case", nMC, "of", nSize, "\n" )

  # load MC data from temporary files
  load( filesMC[ nMC ] )

  # ------ Update MC statistics lists ------

  wData <- logNA( mc[ TmaskStat, "wReal", ] )
  sData <- logNA( mc[ TmaskStat, "s", ] )
  tData <- logNA( mc[ TmaskStat, "Tu", ] )
  nwgData <- mc[ TmaskStat, "normRWgrow", ]
  nsgData <- mc[ TmaskStat, "normSKgrow", ]
  ntgData <- mc[ TmaskStat, "normTUgrow", ]

  rm( mc )

  # initiate 2nd level cluster for parallel processing of statistics
  cl2 <- startCores( cores, parStats )

  # compute each variable statistics in parallel
  stat <- autoLapply( cl2, list( wData, sData, tData, nwgData, nsgData, ntgData ),
                      comp_stats )

  stopCores( cl2 )          # stop 2nd level cluster

  return( list( wMC = stat[[ 1 ]], sMC = stat[[ 2 ]], tMC = stat[[ 3 ]],
                nwgMC = stat[[ 4 ]], nsgMC = stat[[ 5 ]], ntgMC = stat[[ 6 ]],
                nElemMC = nElemMC ) )
}


# ====== Main code ======

tryCatch( {   # enter error handling mode so PDF can be closed in case of error

  # create the report file(s) in a daily output directory
  repFile( repName, folder = folder, suffix = datFilSfx, width = plotW,
           height = plotH, rows = plotRows, cols = plotCols,
           raster = raster, res = res )

  # initiate cluster for parallel loading and preset fixed objects
  cl <- startCores( cores, max( parStats, nSize ), "cores", "parStats",
                    "TmaskStat", "statsMC" )

  #
  # ====== Pooled analysis ======
  #

  cat( "\nGenerating pooled reports...\n" )

  wData <- sData <- tData <- nwData <- nsData <- ntData <-
  nwgData <- nsgData <- ntgData <- list( )

  # run over all experiments individually
  for( k in 1 : nExp ) {

    cat( "", expLeg, k, "of", nExp, "\n" )

    # load pooled data from temporary files (first already loaded)
    if( k > 1 )
      load( files[[ k ]]$pool )

    #
    # ------ Create data & statistics vectors ------
    #

    wData[[ k ]]    <- as.vector( logNA( pool[ TmaskStat, "wReal", ] ) )
    sData[[ k ]]    <- as.vector( logNA( pool[ TmaskStat, "s", ] ) )
    tData[[ k ]]    <- as.vector( pool[ TmaskStat, "Tu", ] )
    nwData[[ k ]]   <- as.vector( logNA( pool[ TmaskStat, "normRW", ] ) )
    nsData[[ k ]]   <- as.vector( logNA( pool[ TmaskStat, "normSK", ] ) )
    ntData[[ k ]]   <- as.vector( logNA( pool[ TmaskStat, "normTU", ] ) )
    nwgData[[ k ]]  <- as.vector( pool[ TmaskStat, "normRWgrow", ] )
    nsgData[[ k ]]  <- as.vector( pool[ TmaskStat, "normSKgrow", ] )
    ntgData[[ k ]]  <- as.vector( pool[ TmaskStat, "normTUgrow", ] )

    rm( pool, P, S, C, c, M, m, n )

    # remove NAs
    nwData[[ k ]]   <- nwData[[ k ]][ ! is.na( nwData[[ k ]] ) ]
    nsData[[ k ]]   <- nsData[[ k ]][ ! is.na( nsData[[ k ]] ) ]
    ntData[[ k ]]   <- ntData[[ k ]][ ! is.na( ntData[[ k ]] ) ]
    nwgData[[ k ]]  <- nwgData[[ k ]][ ! is.na( nwgData[[ k ]] ) ]
    nsgData[[ k ]]  <- nsgData[[ k ]][ ! is.na( nsgData[[ k ]] ) ]
    ntgData[[ k ]]  <- ntgData[[ k ]][ ! is.na( ntgData[[ k ]] ) ]

    # compute each variable statistics in parallel
    stats <- autoLapply( cl, list( nwData[[ k ]], nsData[[ k ]], ntData[[ k ]],
                                   nwgData[[ k ]], nsgData[[ k ]], ntgData[[ k ]] ),
                         comp_stats )

    nw  <- stats[[ 1 ]]
    ns  <- stats[[ 2 ]]
    nt  <- stats[[ 3 ]]
    nwg <- stats[[ 4 ]]
    nsg <- stats[[ 5 ]]
    ntg <- stats[[ 6 ]]

    #
    # ------ Build statistics table ------
    #

    ssz <- sqrt( nSize )
    key.stats <- matrix( c( nw$avg, ns$avg, nt$avg, nwg$avg, nsg$avg, ntg$avg,

                            nw$sd / ssz, ns$sd / ssz, nt$sd / ssz,
                            nwg$sd / ssz, nsg$sd / ssz, ntg$sd / ssz,

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
    colnames( key.stats ) <- c( "R.Wages", "Skills", "T.Un.", "R.Wages Gr.",
                                "Skills Gr.", "T.Un.Gr." )
    rownames( key.stats ) <- c( "average", " (s.e.)", " (s.d.)", "Subbotin b",
                                " a", " m", "Jar.-Bera X2", " (p-val.)",
                                "Lilliefors D", " (p-val.)", "And.-Darling t",
                                " (p-val.)", "autocorr. t-1", "autocorr. t-2" )

    textplot( formatC( key.stats, digits = sDigits, format = "g" ), cmar = 1.0 )
    title <- paste( "Pooled worker-level statistics (", legends[ k ], ")" )
    subTitle <- paste0( "( Log-normalized values / Sample size = ", nElem,
                        " workers / Period = ", warmUp + 1, "-", nTstat, " ",
                        cntLeg, " )" )
    title( main = title, sub = subTitle )

    saveCSV( key.stats, baseName = repName, num = k, baseFolder = folder,
             subFolder = outDir, suffix = datFilSfx, type = "pool" )
  }


  #
  # ====== Plot distributions (overplots)  ======
  #

  # ------ Wage, etc. distributions (binned density x log-level variable)  ------

  plot_norm( nwData, xlab = "Pooled log-normalized real wage",
             ylab = "Binned density",
             tit = paste( "Wages distribution", allLeg ),
             subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
             col = colors, lty = lTypes, pty = pTypes )

  plot_norm( nsData, xlab = "Pooled log-normalized skills level",
             ylab = "Binned density",
             tit = paste( "Skills distribution", allLeg ),
             subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
             col = colors, lty = lTypes, pty = pTypes )

  plot_norm( ntData, xlab = "Pooled log-normalized time unemployed",
             ylab = "Binned density",
             tit = paste( "Unemployment time distribution", allLeg ),
             subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
             col = colors, lty = lTypes, pty = pTypes )

  plot_norm( tData, xlab = "Time unemployed",
             ylab = "Binned density",
             tit = paste( "Unemployment time distribution", allLeg ),
             subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
             col = colors, lty = lTypes, pty = pTypes )

  # ------ Wages, etc. distributions  (log variable x rank) ------

  plot_lognorm( lapply( nwData, exp ), xlab = "Pooled normalized real wage",
                ylab = "Rank",
                tit = paste( "Wage distribution", allLeg ),
                subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
                col = colors, lty = lTypes )

  plot_lognorm( lapply( nsData, exp ), xlab = "Pooled normalized skills level",
                ylab = "Rank",
                tit = paste( "Skills distribution", allLeg ),
                subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
                col = colors, lty = lTypes )

  plot_lognorm( lapply( ntData, exp ), xlab = "Pooled normalized time unemployed",
                ylab = "Rank",
                tit = paste( "Unemployment time distribution", allLeg ),
                subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
                col = colors, lty = lTypes )

  # ------ Growth rate distributions (binned density x log growth rate)  ------

  plot_laplace( nwgData, xlab = "Pooled normalized real wage growth rate",
                ylab = "Binned density",
                tit = paste( "Wage growth distribution", allLeg ),
                subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
                col = colors, lty = lTypes, pty = pTypes )

  plot_laplace( nsgData, xlab = "Pooled normalized skills level growth rate",
                ylab = "Binned density",
                tit = paste( "Skills growth distribution", allLeg ),
                subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
                col = colors, lty = lTypes, pty = pTypes )

  plot_laplace( ntgData, xlab = "Pooled normalized time unemployed growth rate",
                ylab = "Binned density",
                tit = paste( "Unemployment time growth distribution", allLeg ),
                subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
                col = colors, lty = lTypes, pty = pTypes )

  rm( wData, sData, tData, nwData, nsData, ntData, nwgData, nsgData, ntgData )


  #
  # ===== Monte Carlo analysis =====
  #

  cat( "\nGenerating MC reports...\n" )

  nElem <- rep( 0, nExp )

  for( k in 1 : nExp ) {             # do for each experiment

    cat( "", expLeg, k, "of", nExp, "\n" )

    # ------ compute single MC statistics ------

    # compute each MC case in parallel
    stats <- autoLapply( cl, 1 : nSize, statsMC, k, nSize, files[[ k ]]$mc )

    # reorganize data
    wMC <- sMC <- tMC <- nwMC <- nsMC <- ntMC <- nwgMC <- nsgMC <- ntgMC <- list( )

    for( l in 1 : nSize ) {         # for each MC run
      nElem[ k ] <- nElem[ k ] + stats[[ l ]]$nElemMC
      wMC[[l]]   <- stats[[ l ]]$wMC
      sMC[[l]]   <- stats[[ l ]]$sMC
      tMC[[l]]   <- stats[[ l ]]$tMC
      nwgMC[[l]] <- stats[[ l ]]$nwgMC
      nsgMC[[l]] <- stats[[ l ]]$nsgMC
      ntgMC[[l]] <- stats[[ l ]]$ntgMC
    }

    # ------ Compute MC statistics vectors ------

    # compute each variable statistics in parallel
    stats <- autoLapply( cl, list( wMC, sMC, tMC, nwgMC, nsgMC, ntgMC ),
                         comp_MC_stats )

    w   <- stats[[ 1 ]]
    s   <- stats[[ 2 ]]
    t   <- stats[[ 3 ]]
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
    colnames( key.stats ) <- c( "R.Wages", "Skills", "T.Un.", "R.Wages Gr.",
                                "Skills Gr.", "T.Un.Gr." )
    rownames( key.stats ) <- c( "average", " (s.e.)", " (s.d.)", "Subbotin b",
                                " (s.e.)", " (s.d.)", "Subbotin a", " (s.e.)",
                                " (s.d.)", "Subbotin m", " (s.e.)", " (s.d.)",
                                "autocorr. t-1", " (s.e.)",  " (s.d.)",
                                "autocorr. t-2", " (s.e.)",  " (s.d.)" )

    textplot( formatC( key.stats, digits = sDigits, format = "g" ), cmar = 1.0 )
    title <- paste( "Monte Carlo worker-level statistics (", legends[ k ], ")" )
    subTitle <- paste0( "( Normalized log values / Sample size = ", sum( nElem ),
                        " workers / MC runs = ", nSize, " / Period = ",
                        warmUp + 1, "-", nTstat, " ", cntLeg, " )" )
    title( main = title, sub = subTitle )

    saveCSV( key.stats, baseName = repName, num = k, baseFolder = folder,
             subFolder = outDir, suffix = datFilSfx, type = "MC" )
  }


  stopCores( cl )

  clearTemp( savDat = savDat, files = files, nExp = nExp,
             folder = folder, baseName = baseName, datFilSfx = datFilSfx )


  # ------ End of report ------

}, interrupt = catchError, error = catchError, finally = showEndMark( ) )
