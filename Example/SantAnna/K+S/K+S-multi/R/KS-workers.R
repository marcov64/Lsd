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
nCnt      <- 0                      # country to use (mCnt = FALSE) (0=all)
iniDrop   <- 0                      # initial time steps to drop (0=none)
nKeep     <- -1                     # number of time steps to keep (-1=all)
coresExp  <- 0                      # max cores for experiments (0=all)
coresMC   <- 0                      # max cores for Monte Carlo (0=all)
savDat    <- FALSE                  # save data files and re-use if available?
mcStat    <- "median"               # Monte Carlo statistic ("mean", "median")
CI        <- 0.95                   # confidence level
bootR     <- 999                    # bootstrap replicates (bootCI != NULL)
bootCI    <- NULL                   # bootstrap confidence interval method (SLOW)
                                    # (NULL (no bootstrap), "basic", or "bca")

# caption and file names
expVal    <- c( "Only unionized firms", "Mixed firms" )   # experiment captions
cntVal    <- c( "Country 1", "Country 2" )                # country captions
caption   <- "K+S worker analysis"                        # caption for logs
datFilSfx <- "worker"                                     # data file suffix

# worker-level variables to import and add
origVar   <- c( "_CdBas", "_CdLux", "_In", "_SavLux", "_Tu", "_s", "_wReal" )
addVar    <- c( "normIn", "normRW", "normSK", "normTU", "normInGrow",
                "normRWgrow", "normSKgrow", "normTUgrow", "normCdBas",
                "normCdLux", "normSavLux", "normCdBasGrow", "normSavLuxGrow",
                "CbasInSh", "CdBasInGrSh", "InGrSh", "SavLuxInGrSh",
                "luxCsh" )


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

    mean.In <- mean.clean( vector.clean( mc[ i, "In", ] ) )
    mean.wReal <- mean.clean( vector.clean( mc[ i, "wReal", ] ) )
    mean.s <- mean.clean( vector.clean( mc[ i, "s", ] ) )
    mean.Tu <- mean.clean( vector.clean( mc[ i, "Tu", ] ) )
    mean.CdBas <- mean.clean( vector.clean( mc[ i, "CdBas", ] ) )
    mean.CdLux <- mean.clean( vector.clean( mc[ i, "CdLux", ] ) )
    mean.SavLux <- mean.clean( vector.clean( mc[ i, "SavLux", ] ) )

    for( j in 1 : nElemMC ){             # and all element instances

      # change zero values to NA to avoid artifacts in statistics
      if( is.na( mc[ i, "In", j ] )     || mc[ i, "In", j ] <= 0 ||
          is.na( mc[ i, "wReal", j ] )  || mc[ i, "wReal", j ] <= 0 ||
          is.na( mc[ i, "s", j ] )      || mc[ i, "s", j ] <= 0 ||
          is.na( mc[ i, "Tu", j ] )     || mc[ i, "Tu", j ] < 0 ||
          is.na( mc[ i, "CdBas", j ] )  || mc[ i, "CdBas", j ] < 0 ||
          is.na( mc[ i, "CdLux", j ] )  || mc[ i, "CdLux", j ] < 0 ||
          is.na( mc[ i, "SavLux", j ] ) || mc[ i, "SavLux", j ] < 0 )
        mc[ i, , j ] <- NA

      # normalization of variables using the period average size
      if( mean.In != 0 )
        mc[ i, "normIn", j ] <- mc[ i, "In", j ] / mean.In
      if( mean.wReal != 0 )
        mc[ i, "normRW", j ] <- mc[ i, "wReal", j ] / mean.wReal
      if( mean.s != 0 )
        mc[ i, "normSK", j ] <- mc[ i, "s", j ] / mean.s
      if( mean.Tu != 0 )
        mc[ i, "normTU", j ] <- mc[ i, "Tu", j ] / mean.Tu
      if( mean.CdBas != 0 )
        mc[ i, "normCdBas", j ] <- mc[ i, "CdBas", j ] / mean.CdBas
      if( mean.CdLux != 0 )
        mc[ i, "normCdLux", j ] <- mc[ i, "CdLux", j ] / mean.CdLux
      if( mean.SavLux != 0 )
        mc[ i, "normSavLux", j ] <- mc[ i, "SavLux", j ] / mean.SavLux

      # growth rates are calculated only from 2nd period
      if( i > 1 ){
        if( ! is.na( mc[ i - 1, "normIn", j ] ) && ! is.na( mc[ i, "normIn", j ] ) )
          mc[ i, "normInGrow", j ] <- log( mc[ i, "normIn", j ] ) -
            log( mc[ i - 1, "normIn", j ] )
        if( ! is.na( mc[ i - 1, "normRW", j ] ) && ! is.na( mc[ i, "normRW", j ] ) )
          mc[ i, "normRWgrow", j ] <- log( mc[ i, "normRW", j ] ) -
            log( mc[ i - 1, "normRW", j ] )
        if( ! is.na( mc[ i - 1, "normSK", j ] ) && ! is.na( mc[ i, "normSK", j ] ) )
          mc[ i, "normSKgrow", j ] <- log( mc[ i, "normSK", j ] ) -
            log( mc[ i - 1, "normSK", j ] )
        if( ! is.na( mc[ i - 1, "normTU", j ] ) && ! is.na( mc[ i, "normTU", j ] ) )
          mc[ i, "normTUgrow", j ] <- log( mc[ i, "normTU", j ] ) -
            log( mc[ i - 1, "normTU", j ] )
        if( ! is.na( mc[ i - 1, "normCdBas", j ] ) && ! is.na( mc[ i, "normCdBas", j ] ) )
          mc[ i, "normCdBasGrow", j ] <- log( mc[ i, "normCdBas", j ] ) -
            log( mc[ i - 1, "normCdBas", j ] )
        if( ! is.na( mc[ i - 1, "normSavLux", j ] ) && ! is.na( mc[ i, "normSavLux", j ] ) )
          mc[ i, "normSavLuxGrow", j ] <- log( mc[ i, "normSavLux", j ] ) -
            log( mc[ i - 1, "normSavLux", j ] )

        if( is.infinite( mc[ i, "normInGrow", j ] ) )
          mc[ i, "normInGrow", j ] <- NA
        if( is.infinite( mc[ i, "normRWgrow", j ] ) )
          mc[ i, "normRWgrow", j ] <- NA
        if( is.infinite( mc[ i, "normSKgrow", j ] ) )
          mc[ i, "normSKgrow", j ] <- NA
        if( is.infinite( mc[ i, "normTUgrow", j ] ) )
          mc[ i, "normTUgrow", j ] <- NA
        if( is.infinite( mc[ i, "normCdBasGrow", j ] ) )
          mc[ i, "normCdBasGrow", j ] <- NA
        if( is.infinite( mc[ i, "normSavLuxGrow", j ] ) )
          mc[ i, "normSavLuxGrow", j ] <- NA
      }

      # ratio variables
      if( is.na( mc[ i, "CdBas", j ] ) ) {
        mc[ i, "CbasInSh", j ] <- NA
        mc[ i, "luxCsh", j ] <- NA
      } else {
        if( is.na( mc[ i, "In", j ] ) || mc[ i, "In", j ] <= 0 )
          mc[ i, "CbasInSh", j ] <- NA
        else
          mc[ i, "CbasInSh", j ] <- mc[ i, "CdBas", j ] /
            mc[ i, "In", j ]

        if( mc[ i, "CdLux", j ] <= 0 )
          mc[ i, "luxCsh", j ] <- 0
        else
          mc[ i, "luxCsh", j ] <- mc[ i, "CdLux", j ] /
            ( mc[ i, "CdBas", j ] + mc[ i, "CdLux", j ] )
      }

      if( is.na( mc[ i, "normInGrow", j ] ) ||
          is.na( mc[ i, "normCdBasGrow", j ] ) ||
          is.na( mc[ i, "normSavLuxGrow", j ] ) ||
          mc[ i, "normInGrow", j ] <= 0 ) {
        mc[ i, "InGrSh", j ] <- NA
        mc[ i, "CdBasInGrSh", j ] <- NA
        mc[ i, "SavLuxInGrSh", j ] <- NA
      } else {
        mc[ i, "InGrSh", j ] <- mc[ i, "normIn", j ]
        mc[ i, "CdBasInGrSh", j ] <- mc[ i, "normCdBasGrow", j ] /
          mc[ i, "normInGrow", j ]
        mc[ i, "SavLuxInGrSh", j ] <- mc[ i, "normSavLuxGrow", j ] /
          mc[ i, "normInGrow", j ]
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
warmUp    <- 100      # number of "warm-up" runs
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

  iData <- logNA( mc[ TmaskStat, "In", ] )
  wData <- logNA( mc[ TmaskStat, "wReal", ] )
  sData <- logNA( mc[ TmaskStat, "s", ] )
  tData <- logNA( mc[ TmaskStat, "Tu", ] )
  nigData <- mc[ TmaskStat, "normInGrow", ]
  nwgData <- mc[ TmaskStat, "normRWgrow", ]
  nsgData <- mc[ TmaskStat, "normSKgrow", ]
  ntgData <- mc[ TmaskStat, "normTUgrow", ]
  ncbData <- logNA( mc[ TmaskStat, "normCdBas", ] )
  nclData <- logNA( mc[ TmaskStat, "normCdLux", ] )

  rm( mc )

  # initiate 2nd level cluster for parallel processing of statistics
  cl2 <- startCores( cores, parStats )

  # compute each variable statistics in parallel
  stat <- autoLapply( cl2, list( iData, wData, sData, tData, nigData,
                                 nwgData, nsgData, ntgData, ncbData,
                                 nclData ),
                      comp_stats )

  stopCores( cl2 )          # stop 2nd level cluster

  return( list( iMC = stat[[ 1 ]], wMC = stat[[ 2 ]], sMC = stat[[ 3 ]],
                tMC = stat[[ 4 ]], nigMC = stat[[ 5 ]], nwgMC = stat[[ 6 ]],
                nsgMC = stat[[ 7 ]], ntgMC = stat[[ 8 ]], ncbMC = stat[[ 9 ]],
                nclMC = stat[[ 10 ]], nElemMC = nElemMC ) )
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

  iData <- wData <- sData <- tData <- niData <- nwData <- nsData <- ntData <-
    nigData <- nwgData <- nsgData <- ntgData <- ncbData <- nclData <- lsData <-
    CIsData <- IgsData <- CIgsData <- SIgsData <- list( )

  # run over all experiments individually
  for( k in 1 : nExp ) {

    cat( "", expLeg, k, "of", nExp, "\n" )

    # load pooled data from temporary files (first already loaded)
    if( k > 1 )
      load( files[[ k ]]$pool )

    #
    # ------ Create data & statistics vectors ------
    #

    iData[[ k ]]    <- as.vector( logNA( pool[ TmaskStat, "In", ] ) )
    wData[[ k ]]    <- as.vector( logNA( pool[ TmaskStat, "wReal", ] ) )
    sData[[ k ]]    <- as.vector( logNA( pool[ TmaskStat, "s", ] ) )
    tData[[ k ]]    <- as.vector( pool[ TmaskStat, "Tu", ] )
    niData[[ k ]]   <- as.vector( logNA( pool[ TmaskStat, "normIn", ] ) )
    nwData[[ k ]]   <- as.vector( logNA( pool[ TmaskStat, "normRW", ] ) )
    nsData[[ k ]]   <- as.vector( logNA( pool[ TmaskStat, "normSK", ] ) )
    ntData[[ k ]]   <- as.vector( logNA( pool[ TmaskStat, "normTU", ] ) )
    nigData[[ k ]]  <- as.vector( pool[ TmaskStat, "normInGrow", ] )
    nwgData[[ k ]]  <- as.vector( pool[ TmaskStat, "normRWgrow", ] )
    nsgData[[ k ]]  <- as.vector( pool[ TmaskStat, "normSKgrow", ] )
    ntgData[[ k ]]  <- as.vector( pool[ TmaskStat, "normTUgrow", ] )
    ncbData[[ k ]]  <- as.vector( logNA( pool[ TmaskStat, "normCdBas", ] ) )
    nclData[[ k ]]  <- as.vector( logNA( pool[ TmaskStat, "normCdLux", ] ) )
    lsData[[ k ]]   <- as.vector( pool[ TmaskStat, "luxCsh", ] )
    CIsData[[ k ]]  <- as.vector( pool[ TmaskStat, "CbasInSh", ] )
    IgsData[[ k ]]  <- as.vector( pool[ TmaskStat, "InGrSh", ] )
    CIgsData[[ k ]] <- as.vector( pool[ TmaskStat, "CdBasInGrSh", ] )
    SIgsData[[ k ]] <- as.vector( pool[ TmaskStat, "SavLuxInGrSh", ] )

    rm( pool, P, S, C, c, M, m, n )

    # remove NAs
    niData[[ k ]]   <- niData[[ k ]][ ! is.na( niData[[ k ]] ) ]
    nwData[[ k ]]   <- nwData[[ k ]][ ! is.na( nwData[[ k ]] ) ]
    nsData[[ k ]]   <- nsData[[ k ]][ ! is.na( nsData[[ k ]] ) ]
    ntData[[ k ]]   <- ntData[[ k ]][ ! is.na( ntData[[ k ]] ) ]
    nigData[[ k ]]  <- nigData[[ k ]][ ! is.na( nigData[[ k ]] ) ]
    nwgData[[ k ]]  <- nwgData[[ k ]][ ! is.na( nwgData[[ k ]] ) ]
    nsgData[[ k ]]  <- nsgData[[ k ]][ ! is.na( nsgData[[ k ]] ) ]
    ntgData[[ k ]]  <- ntgData[[ k ]][ ! is.na( ntgData[[ k ]] ) ]
    ncbData[[ k ]]  <- ncbData[[ k ]][ ! is.na( ncbData[[ k ]] ) ]
    nclData[[ k ]]  <- nclData[[ k ]][ ! is.na( nclData[[ k ]] ) ]
    lsData[[ k ]]   <- lsData[[ k ]][ ! is.na( lsData[[ k ]] ) ]
    CIsData[[ k ]]  <- CIsData[[ k ]][ ! is.na( CIsData[[ k ]] ) ]
    IgsData[[ k ]]  <- IgsData[[ k ]][ ! is.na( IgsData[[ k ]] ) ]
    CIgsData[[ k ]] <- CIgsData[[ k ]][ ! is.na( CIgsData[[ k ]] ) ]
    SIgsData[[ k ]] <- SIgsData[[ k ]][ ! is.na( SIgsData[[ k ]] ) ]

    # compute each variable statistics in parallel
    stats <- autoLapply( cl, list( niData[[ k ]], nwData[[ k ]], nsData[[ k ]],
                                   ntData[[ k ]], nigData[[ k ]], nwgData[[ k ]],
                                   nsgData[[ k ]], ntgData[[ k ]], ncbData[[ k ]],
                                   nclData[[ k ]] ),
                         comp_stats )

    ni  <- stats[[ 1 ]]
    nw  <- stats[[ 2 ]]
    ns  <- stats[[ 3 ]]
    nt  <- stats[[ 4 ]]
    nig <- stats[[ 5 ]]
    nwg <- stats[[ 6 ]]
    nsg <- stats[[ 7 ]]
    ntg <- stats[[ 8 ]]
    ncb <- stats[[ 9 ]]
    ncl <- stats[[ 10 ]]

    #
    # ------ Build statistics table ------
    #

    ssz <- sqrt( nSize )
    key.stats <- matrix( c( ni$avg, nw$avg, ns$avg, nt$avg,
                            nig$avg, nwg$avg, nsg$avg, ntg$avg,
                            ncb$avg, ncl$avg,

                            ni$sd / ssz, nw$sd / ssz, ns$sd / ssz, nt$sd / ssz,
                            nig$sd / ssz, nwg$sd / ssz, nsg$sd / ssz, ntg$sd / ssz,
                            ncb$sd / ssz, ncl$sd / ssz,

                            ni$sd, nw$sd, ns$sd, nt$sd,
                            nig$sd, nwg$sd, nsg$sd, ntg$sd,
                            ncb$sd, ncl$sd,

                            ni$subbo$b, nw$subbo$b, ns$subbo$b, nt$subbo$b,
                            nig$subbo$b, nwg$subbo$b, nsg$subbo$b, ntg$subbo$b,
                            ncb$subbo$b, ncl$subbo$b,

                            ni$subbo$a, nw$subbo$a, ns$subbo$a, nt$subbo$a,
                            nig$subbo$a, nwg$subbo$a, nsg$subbo$a, ntg$subbo$a,
                            ncb$subbo$a, ncl$subbo$a,

                            ni$subbo$m, nw$subbo$m, ns$subbo$m, nt$subbo$m,
                            nig$subbo$m, nwg$subbo$m, nsg$subbo$m, ntg$subbo$m,
                            ncb$subbo$m, ncl$subbo$m,

                            ni$jb$statistic, nw$jb$statistic, ns$jb$statistic,
                            nt$jb$statistic, nig$jb$statistic, nwg$jb$statistic,
                            nsg$jb$statistic, ntg$jb$statistic, ncb$jb$statistic,
                            ncl$jb$statistic,

                            ni$jb$p.value, nw$jb$p.value, ns$jb$p.value, nt$jb$p.value,
                            nig$jb$p.value, nwg$jb$p.value, nsg$jb$p.value,
                            ntg$jb$p.value, ncb$jb$p.value, ncl$jb$p.value,

                            ni$ll$statistic, nw$ll$statistic, ns$ll$statistic,
                            nt$ll$statistic, nig$ll$statistic, nwg$ll$statistic,
                            nsg$ll$statistic, ntg$ll$statistic, ncb$ll$statistic,
                            ncl$ll$statistic,

                            ni$ll$p.value, nw$ll$p.value, ns$ll$p.value, nt$ll$p.value,
                            nig$ll$p.value, nwg$ll$p.value, nsg$ll$p.value,
                            ntg$ll$p.value, ncb$ll$p.value, ncl$ll$p.value,

                            ni$ad$statistic, nw$ad$statistic, ns$ad$statistic,
                            nt$ad$statistic, nig$ad$statistic, nwg$ad$statistic,
                            nsg$ad$statistic, ntg$ad$statistic, ncb$ad$statistic,
                            ncl$ad$statistic,

                            ni$ad$p.value, nw$ad$p.value, ns$ad$p.value, nt$ad$p.value,
                            nig$ad$p.value, nwg$ad$p.value, nsg$ad$p.value,
                            ntg$ad$p.value, ncb$ad$p.value, ncl$ad$p.value,

                            ni$ac$t1, nw$ac$t1, ns$ac$t1, nt$ac$t1,
                            nig$ac$t1, nwg$ac$t1, nsg$ac$t1, ntg$ac$t1,
                            ncb$ac$t1, ncl$ac$t1,

                            ni$ac$t2, nw$ac$t2, ns$ac$t2, nt$ac$t2,
                            nig$ac$t2, nwg$ac$t2, nsg$ac$t2, ntg$ac$t2,
                            ncb$ac$t2, ncl$ac$t2 ),

                         ncol = 10, byrow = TRUE )
    colnames( key.stats ) <- c( "Income", "R.Wages", "Skills", "T.Un.",
                                "Income Gr.", "R.Wages Gr.", "Skills Gr.",
                                "T.Un.Gr.", "C.D.Bas.", "C.D.Lux." )
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

    #
    # ====== experiment-specific plots ======
    #

    # ------ luxury consumption share by income deciles  ------

    plot_quant( niData[[ k ]], lsData[[ k ]], bins = 10,
                xlab = "Income decile",
                ylab = "Desired luxury consumption share",
                tit = paste( "Desired luxury consumption by income (", legends[ k ], ")" ),
                subtit = subTitle )

    # ------ basic consumption share on income by income deciles  ------

    plot_quant( niData[[ k ]], CIsData[[ k ]], bins = 10,
                xlab = "Income decile",
                ylab = "Desired basic consumption on income",
                tit = paste( "Share of desired basic consumption by income (", legends[ k ], ")" ),
                subtit = subTitle )

    # ------ change in basic consumption share on change in income by income deciles  ------

    plot_quant( IgsData[[ k ]], CIgsData[[ k ]], bins = 10,
                xlab = "Income decile",
                ylab = "Change in desired basic consumption on change in income",
                tit = paste( "Basic consumption change on income change (", legends[ k ], ")" ),
                subtit = subTitle )

    # ------ change in luxury savings share on change in income by income deciles  ------

    plot_quant( IgsData[[ k ]], SIgsData[[ k ]], bins = 10,
                xlab = "Income decile",
                ylab = "Change in savings for luxury on change in income",
                tit = paste( "Luxury savings change on income change (", legends[ k ], ")" ),
                subtit = subTitle )
  }


  #
  # ====== Plot distributions (overplots)  ======
  #

  # ------ Wage, etc. distributions (binned density x log-level variable)  ------

  plot_norm( niData, xlab = "Pooled log-normalized income",
             ylab = "Binned density",
             tit = paste( "Income distribution", allLeg ),
             subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
             col = colors, lty = lTypes, pty = pTypes )

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

  # ------ Consumption distributions (binned density x log-level variable) ------

  plot_norm( ncbData, xlab = "Pooled log-normalized desired basic consumption",
             ylab = "Binned density",
             tit = paste( "Basic consumption distribution", allLeg ),
             subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
             col = colors, lty = lTypes, pty = pTypes )

  plot_norm( nclData, xlab = "Pooled log-normalized desired luxury consumption",
             ylab = "Binned density",
             tit = paste( "Luxury consumption distribution", allLeg ),
             subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
             col = colors, lty = lTypes, pty = pTypes )

  # ------ Wages, etc. distributions  (log variable x rank) ------

  plot_lognorm( lapply( niData, exp ), xlab = "Pooled normalized income",
                ylab = "Rank",
                tit = paste( "Income distribution", allLeg ),
                subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
                col = colors, lty = lTypes )

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

  plot_laplace( nigData, xlab = "Pooled normalized income growth rate",
                ylab = "Binned density",
                tit = paste( "Income growth distribution", allLeg ),
                subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
                col = colors, lty = lTypes, pty = pTypes )

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

  rm( iData, wData, sData, tData, niData, nwData, nsData, ntData, nigData,
      nwgData, nsgData, ntgData, ncbData, nclData, lsData, CIsData, IgsData,
      CIgsData, SIgsData )


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
    iMC <- wMC <- sMC <- tMC <- niMC <- nwMC <- nsMC <- ntMC <- nigMC <-
      nwgMC <- nsgMC <- ntgMC <- ncbMC <- nclMC <- list( )

    for( l in 1 : nSize ) {         # for each MC run
      nElem[ k ] <- nElem[ k ] + stats[[ l ]]$nElemMC
      iMC[[l]]   <- stats[[ l ]]$iMC
      wMC[[l]]   <- stats[[ l ]]$wMC
      sMC[[l]]   <- stats[[ l ]]$sMC
      tMC[[l]]   <- stats[[ l ]]$tMC
      nigMC[[l]] <- stats[[ l ]]$nigMC
      nwgMC[[l]] <- stats[[ l ]]$nwgMC
      nsgMC[[l]] <- stats[[ l ]]$nsgMC
      ntgMC[[l]] <- stats[[ l ]]$ntgMC
      ncbMC[[l]] <- stats[[ l ]]$ncbMC
      nclMC[[l]] <- stats[[ l ]]$nclMC
    }

    # ------ Compute MC statistics vectors ------

    # compute each variable statistics in parallel
    stats <- autoLapply( cl, list( iMC, wMC, sMC, tMC, nigMC, nwgMC, nsgMC,
                                   ntgMC, ncbMC, nclMC ), comp_MC_stats )

    i   <- stats[[ 1 ]]
    w   <- stats[[ 2 ]]
    s   <- stats[[ 3 ]]
    t   <- stats[[ 4 ]]
    nig <- stats[[ 5 ]]
    nwg <- stats[[ 6 ]]
    nsg <- stats[[ 7 ]]
    ntg <- stats[[ 8 ]]
    ncb <- stats[[ 9 ]]
    ncl <- stats[[ 10 ]]

    # ------ Build statistics table ------

    key.stats <- matrix( c( i$avg$avg, w$avg$avg, s$avg$avg, t$avg$avg,
                            nig$avg$avg, nwg$avg$avg, nsg$avg$avg, ntg$avg$avg,
                            ncb$avg$avg, ncl$avg$avg,

                            i$se$avg, w$se$avg, s$se$avg, t$se$avg,
                            nig$se$avg, nwg$se$avg, nsg$se$avg, ntg$se$avg,
                            ncb$se$avg, ncl$se$avg,

                            i$sd$avg, w$sd$avg, s$sd$avg, t$sd$avg,
                            nig$sd$avg, nwg$sd$avg, nsg$sd$avg, ntg$sd$avg,
                            ncb$sd$avg, ncl$sd$avg,

                            i$avg$subbo$b, w$avg$subbo$b, s$avg$subbo$b,
                            t$avg$subbo$b, nig$avg$subbo$b, nwg$avg$subbo$b,
                            nsg$avg$subbo$b, ntg$avg$subbo$b, ncb$avg$subbo$b,
                            ncl$avg$subbo$b,

                            i$se$subbo$b, w$se$subbo$b, s$se$subbo$b,
                            t$se$subbo$b, nig$se$subbo$b, nwg$se$subbo$b,
                            nsg$se$subbo$b, ntg$se$subbo$b, ncb$se$subbo$b,
                            ncl$se$subbo$b,

                            i$sd$subbo$b, w$sd$subbo$b, s$sd$subbo$b,
                            t$sd$subbo$b, nig$sd$subbo$b, nwg$sd$subbo$b,
                            nsg$sd$subbo$b, ntg$sd$subbo$b, ncb$sd$subbo$b,
                            ncl$sd$subbo$b,

                            i$avg$subbo$a, w$avg$subbo$a, s$avg$subbo$a,
                            t$avg$subbo$a, nig$avg$subbo$a, nwg$avg$subbo$a,
                            nsg$avg$subbo$a, ntg$avg$subbo$a, ncb$avg$subbo$a,
                            ncl$avg$subbo$a,

                            i$se$subbo$a, w$se$subbo$a, s$se$subbo$a,
                            t$se$subbo$a, nig$se$subbo$a, nwg$se$subbo$a,
                            nsg$se$subbo$a, ntg$se$subbo$a, ncb$se$subbo$a,
                            ncl$se$subbo$a,

                            i$sd$subbo$a, w$sd$subbo$a, s$sd$subbo$a,
                            t$sd$subbo$a, nig$sd$subbo$a, nwg$sd$subbo$a,
                            nsg$sd$subbo$a, ntg$sd$subbo$a, ncb$sd$subbo$a,
                            ncl$sd$subbo$a,

                            i$avg$subbo$m, w$avg$subbo$m, s$avg$subbo$m,
                            t$avg$subbo$m, nig$avg$subbo$m, nwg$avg$subbo$m,
                            nsg$avg$subbo$m, ntg$avg$subbo$m, ncb$avg$subbo$m,
                            ncl$avg$subbo$m,

                            i$se$subbo$m, w$se$subbo$m, s$se$subbo$m,
                            t$se$subbo$m, nig$se$subbo$m, nwg$se$subbo$m,
                            nsg$se$subbo$m, ntg$se$subbo$m, ncb$se$subbo$m,
                            ncl$se$subbo$m,

                            i$sd$subbo$m, w$sd$subbo$m, s$sd$subbo$m,
                            t$sd$subbo$m, nig$sd$subbo$m, nwg$sd$subbo$m,
                            nsg$sd$subbo$m, ntg$sd$subbo$m, ncb$sd$subbo$m,
                            ncl$sd$subbo$m,

                            i$avg$ac$t1, w$avg$ac$t1, s$avg$ac$t1, t$avg$ac$t1,
                            nig$avg$ac$t1, nwg$avg$ac$t1, nsg$avg$ac$t1,
                            ntg$avg$ac$t1, ncb$avg$ac$t1, ncl$avg$ac$t1,

                            i$se$ac$t1, w$se$ac$t1, s$se$ac$t1, t$se$ac$t1,
                            nig$se$ac$t1, nwg$se$ac$t1, nsg$se$ac$t1,
                            ntg$se$ac$t1, ncb$se$ac$t1, ncl$se$ac$t1,

                            i$sd$ac$t1, w$sd$ac$t1, s$sd$ac$t1, t$sd$ac$t1,
                            nig$sd$ac$t1, nwg$sd$ac$t1, nsg$sd$ac$t1,
                            ntg$sd$ac$t1, ncb$sd$ac$t1, ncl$sd$ac$t1,

                            i$avg$ac$t2, w$avg$ac$t2, s$avg$ac$t2, t$avg$ac$t2,
                            nig$avg$ac$t2, nwg$avg$ac$t2, nsg$avg$ac$t2,
                            ntg$avg$ac$t2, ncb$avg$ac$t2, ncl$avg$ac$t2,

                            i$se$ac$t2, w$se$ac$t2, s$se$ac$t2, t$se$ac$t2,
                            nig$se$ac$t2, nwg$se$ac$t2, nsg$se$ac$t2,
                            ntg$se$ac$t2, ncb$se$ac$t2, ncl$se$ac$t2,

                            i$sd$ac$t2, w$sd$ac$t2, s$sd$ac$t2, t$sd$ac$t2,
                            nig$sd$ac$t2, nwg$sd$ac$t2, nsg$sd$ac$t2,
                            ntg$sd$ac$t2, ncb$sd$ac$t2, ncl$sd$ac$t2 ),

                         ncol = 10, byrow = TRUE )
    colnames( key.stats ) <- c( "Income", "R.Wages", "Skills", "T.Un.",
                                "Income Gr.", "R.Wages Gr.", "Skills Gr.",
                                "T.Un.Gr.", "C.D.Bas.", "C.D.Lux." )
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
