#******************************************************************
#
# ------------- K+S capital-good firm analysis ------------------
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
caption   <- "K+S capital-good firm analysis"             # caption for logs
sector    <- "Capital-goods sector"                       # caption for plots
datFilSfx <- "firm1"                                      # data file suffix

# firm-level variables to import and add
origVar   <- c( "_B1", "_Q1e" )
addVar    <- c( "growth1", "normO1", "lnormO1", "lnormO1grow", "lnormO1match",
                "lnormO1matchLag", "normA1", "normA1grow", "normO1grow" )


# ==== Libraries, support functions and global options ====

source( "KS-support-functions.R" )

options( warn = 0 )           # -1=no warning/0:warnings at end/2:warnings stop

# remove RStudio warnings for support functions
# !diagnostics suppress = showStartMark, showEndMark, setCmdLinePars, loadData
# !diagnostics suppress = catchError, startTime, mean.clean, vector.clean
# !diagnostics suppress = logNA, textplot, plot_lognorm, plot_norm, setLabels
# !diagnostics suppress = plot_laplace, plot_lin, size_bins, comp_stats, saveCSV
# !diagnostics suppress = comp_MC_stats, startCores, stopCores, autoLapply
# !diagnostics suppress = clearTemp, repFile, legends, expLeg, allLeg
# !diagnostics suppress = mc, pool, nElem, nElemMC, nTsteps, nSize, outDir


# ==== Define function to create new variables ====

# function to add new variables to Monte Carlo sample (to be parallelized)
addVarFn <- function( mc, nElemMC, nTsteps, nVar ) {

  for( i in 1 : nTsteps ) {            # all time steps

    Q1e         <- vector.clean( mc[ i, "Q1e", ], min = 1 )
    mean.Q1e    <- mean.clean( Q1e )
    mean.lsize1 <- mean.clean( log( Q1e ) )
    mean.B1     <- mean.clean( vector.clean( mc[ i, "B1", ] ) )

    for( j in 1 : nElemMC ) {             # and all element instances

      # take care of entrants' first period and other data problems to avoid atifacts
      if( is.na( mc[ i, "Q1e", j ] ) || mc[ i, "Q1e", j ] < 1 ||
          is.na( mc[ i, "B1", j ] ) || mc[ i, "B1", j ] < 1 ) {
        mc[ i, , j ] <- NA
        next
      }

      # normalization of key variables using the period average size
      if( mean.Q1e != 0 )
        mc[ i, "normO1", j ] <- mc[ i,"Q1e", j ] / mean.Q1e
      if( mean.lsize1 != 0 )
        mc[ i, "lnormO1", j ] <- log( mc[ i, "Q1e", j ] ) - mean.lsize1
      if( mean.B1 != 0 )
        mc[ i, "normA1", j ] <- mc[ i, "B1", j ] / mean.B1

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

nBins     <- 15     # number of bins to use in histograms
outLim    <- 0.001  # outlier percentile (0=don't remove outliers)
warmUp    <- 100    # number of "warm-up" runs
nTstat    <- -1     # last period to consider for statistics (-1=all)

cores     <- 6      # maximum number of cores to allocate (0=all)
parStats  <- 6      # number of statistics to be computed in parallel
repName   <- ""     # report files base name (if "" same baseName)
sDigits   <- 4      # significant digits in tables
plotRows  <- 1      # number of plots per row in a page
plotCols  <- 1      # number of plots per column in a page
plotW     <- 10     # plot window width
plotH     <- 7      # plot window height
raster    <- FALSE  # raster or vector plots
res       <- 600    # resolution of raster mode (in dpi)

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

  oData   <- logNA( mc[ TmaskStat, "Q1e", ] )
  Adata   <- logNA( mc[ TmaskStat, "B1", ] )
  gData   <- mc[ TmaskStat,"growth1", ]
  ngData  <- mc[ TmaskStat,"normO1grow", ]
  ngAdata <- mc[ TmaskStat,"normA1grow", ]
  exData  <- as.numeric( is.na( mc[ TmaskStat, "growth1", ] ) )

  rm( mc )

  # initiate 2nd level cluster for parallel processing of statistics
  cl2 <- startCores( cores, parStats )

  # compute each variable statistics in parallel
  stat <- autoLapply( cl2, list( oData, Adata, gData, ngData, ngAdata, exData ),
                      comp_stats )

  stopCores( cl2 )          # stop 2nd level cluster

  return( list( oMC = stat[[ 1 ]], AMC = stat[[ 2 ]], gMC = stat[[ 3 ]],
                ngMC = stat[[ 4 ]], ngAMC = stat[[ 5 ]], exMC = stat[[ 6 ]],
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

  szData <- oData <- Adata <- gData <- ngData <- ngAdata <- exData <- svData <-
    bins <- list( )

  # run over all experiments individually
  for( k in 1 : nExp ) {

    cat( "", expLeg, k, "of", nExp, "\n" )

    # load pooled data from temporary files (first already loaded)
    if( k > 1 )
      load( files[[ k ]]$pool )

    #
    # ------ create data & statistics vectors ------
    #

    szData[[ k ]]   <- as.vector( pool[ TmaskStat, "Q1e", ] )
    oData[[ k ]]    <- as.vector( logNA( pool[ TmaskStat, "normO1", ] ) )
    Adata[[ k ]]    <- as.vector( logNA( pool[ TmaskStat, "normA1", ] ) )
    gData[[ k ]]    <- as.vector( pool[ TmaskStat, "growth1", ] )
    ngData[[ k ]]   <- as.vector( pool[ TmaskStat, "normO1grow", ] )
    ngAdata[[ k ]]  <- as.vector( pool[ TmaskStat, "normA1grow", ] )
    exData[[ k ]]   <- as.vector( as.numeric( is.na( pool[ TmaskStat, "growth1", ] ) ) )

    # prepare data for Gibrat and scaling variance plots
    bins[[ k ]] <- size_bins( as.vector( pool[ TmaskStat, "lnormO1match", ] ),
                              as.vector( pool[ TmaskStat, "lnormO1matchLag", ] ),
                              as.vector( pool[ TmaskStat, "lnormO1grow", ] ),
                              bins = 2 * nBins, outLim = outLim )

    rm( pool, P, S, C, c, M, m, n )

    # remove NAs
    szData[[ k ]]   <- szData[[ k ]][ ! is.na( szData[[ k ]] ) ]
    oData[[ k ]]    <- oData[[ k ]][ ! is.na( oData[[ k ]] ) ]
    Adata[[ k ]]    <- Adata[[ k ]][ ! is.na( Adata[[ k ]] ) ]
    gData[[ k ]]    <- gData[[ k ]][ ! is.na( gData[[ k ]] ) ]
    ngData[[ k ]]   <- ngData[[ k ]][ ! is.na( ngData[[ k ]] ) ]
    ngAdata[[ k ]]  <- ngAdata[[ k ]][ ! is.na( ngAdata[[ k ]] ) ]
    exData[[ k ]]   <- exData[[ k ]][ ! is.na( exData[[ k ]] ) ]

    # compute each variable statistics in parallel
    stats <- autoLapply( cl, list( oData[[ k ]], Adata[[ k ]], gData[[ k ]],
                                   ngData[[ k ]], ngAdata[[ k ]], exData[[ k ]] ),
                         comp_stats )

    o <- stats[[ 1 ]]
    A <- stats[[ 2 ]]
    g <- stats[[ 3 ]]
    ng <- stats[[ 4 ]]
    ngA <- stats[[ 5 ]]
    ex <- stats[[ 6 ]]

    #
    # ------ build statistics table ------
    #

    key.stats <- matrix( c( o$avg, A$avg, g$avg, ng$avg, ngA$avg, ex$avg,

                            o$sd / sqrt( nSize ), A$sd / sqrt( nSize ),
                            g$sd / sqrt( nSize ), ng$sd / sqrt( nSize ),
                            ngA$sd / sqrt( nSize ), ex$sd / sqrt( nSize ),

                            o$sd, A$sd, g$sd, ng$sd, ngA$sd, ex$sd,

                            o$subbo$b, A$subbo$b, g$subbo$b, ng$subbo$b,
                            ngA$subbo$b, ex$subbo$b,

                            o$subbo$a, A$subbo$a, g$subbo$a, ng$subbo$a,
                            ngA$subbo$a, ex$subbo$a,

                            o$subbo$m, A$subbo$m, g$subbo$m, ng$subbo$m,
                            ngA$subbo$m, ex$subbo$m,

                            o$jb$statistic, A$jb$statistic, g$jb$statistic,
                            ng$jb$statistic, ngA$jb$statistic, ex$jb$statistic,

                            o$jb$p.value, A$jb$p.value, g$jb$p.value,
                            ng$jb$p.value, ngA$jb$p.value, ex$jb$p.value,

                            o$ll$statistic, A$ll$statistic, g$ll$statistic,
                            ng$ll$statistic, ngA$ll$statistic, ex$ll$statistic,

                            o$ll$p.value, A$ll$p.value, g$ll$p.value,
                            ng$ll$p.value, ngA$ll$p.value, ex$ll$p.value,

                            o$ad$statistic, A$ad$statistic, g$ad$statistic,
                            ng$ad$statistic, ngA$ad$statistic, ex$ad$statistic,

                            o$ad$p.value, A$ad$p.value, g$ad$p.value,
                            ng$ad$p.value, ngA$ad$p.value, ex$ad$p.value,

                            o$ac$t1, A$ac$t1, g$ac$t1, ng$ac$t1,
                            ngA$ac$t1, ex$ac$t1,

                            o$ac$t2, A$ac$t2, g$ac$t2, ng$ac$t2,
                            ngA$ac$t2, ex$ac$t2 ),

                         ncol = 6, byrow = TRUE )
    colnames( key.stats ) <- c( "N.Output(log)", "N.Prod.(log)", "Output Gr.",
                                "N.Output Gr.", "N.Prod.Gr.", "Exit Rate")
    rownames( key.stats ) <- c( "average", " (s.e.)", " (s.d.)", "Subbotin b",
                                " a", " m", "Jarque-Bera X2",
                                " (p-val.)", "Lilliefors D", " (p-val.)",
                                "Anderson-Darling A", " (p-val.)",
                                "autocorr. t-1", "autocorr. t-2" )

    textplot( formatC( key.stats, digits = sDigits, format = "g" ), cmar = 1.0 )
    title <- paste( "Pooled firm-level statistics (", legends[ k ], ")" )
    subTitle <- paste( paste0( "( Sample size = ", nElem, " firms / Period = ",
                               warmUp + 1, "-", nTstat, " )" ),
                       paste( "(", sector, ")" ), sep ="\n" )
    title( main = title, sub = subTitle )

    saveCSV( key.stats, baseName = repName, num = k, baseFolder = folder,
             subFolder = outDir, suffix = datFilSfx, type = "pool" )

    #
    # ====== experiment-specific plots ======
    #

    subTitle <- paste0( "( Sample size = ", nElem, " firms / Period = ",
                        warmUp + 1, "-", nTstat, " / ", sector, " )" )

    # ------ Gibrat law test plot ------

    plot_lin( bins[[ k ]]$sLagAvg, bins[[ k ]]$s1avg,
              xlab = "Log-normalized size of firms in t-1",
              ylab = "Log-normalized size of firms in t",
              tit = paste( "Gibrat law (", legends[ k ], ")" ),
              subtit = subTitle, invleg = TRUE )

    # ------ scaling variance plot ------

    plot_lin( bins[[ k ]]$s2avg, bins[[ k ]]$gSD,
              xlab = "Log-normalized size of firms",
              ylab = "Log-standard deviation of growth rate",
              tit = paste( "Scaling of variance of firm size (",
                           legends[ k ], ")" ),
              subtit = subTitle )
  }


  #
  # ====== Plot distributions (overplots)  ======
  #

  # ------ Size, etc. distributions (binned density x log-level variable)  ------

  plot_norm( oData, xlab = "Pooled log-normalized output",
             ylab = "Binned density",
             tit = paste( "Size distribution", allLeg ),
             subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
             col = colors, lty = lTypes, pty = pTypes )

  plot_norm( Adata, xlab = "Pooled log-normalized productivity",
             ylab = "Binned density",
             tit = paste( "Productivity distribution", allLeg ),
             subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
             col = colors, lty = lTypes, pty = pTypes )

  # ------ size distributions (log size x rank)  ------

  plot_lognorm( szData, xlab = "Pooled output",
                ylab = "Rank",
                tit = paste( "Size distribution", allLeg ),
                subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
                col = colors, lty = lTypes )

  # ------ growth rate distributions (binned density x growth rate)  ------

  plot_laplace( gData, xlab = "Pooled output growth rate",
                ylab = "Binned density",
                tit = paste( "Size growth rate distribution", allLeg ),
                subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
                col = colors, lty = lTypes, pty = pTypes )

  plot_laplace( ngData, xlab = "Pooled normalized output growth rate",
                ylab = "Binned density",
                tit = paste( "Size growth rate distribution", allLeg ),
                subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
                col = colors, lty = lTypes, pty = pTypes )

  plot_laplace( ngAdata, xlab = "Pooled normalized productivity growth rate",
                ylab = "Binned density",
                tit = paste( "Productivity growth rate distribution", allLeg ),
                subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
                col = colors, lty = lTypes, pty = pTypes )

  rm( szData, oData, Adata, gData, ngData, ngAdata, exData, svData, bins )


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
    oMC <- AMC <- gMC <- ngMC <- ngAMC <- exMC <- list( )

    for( l in 1 : nSize ){          # for each MC run
      nElem[ k ]    <- nElem[ k ] + stats[[ l ]]$nElemMC
      oMC[[ l ]]    <- stats[[ l ]]$oMC
      AMC[[ l ]]    <- stats[[ l ]]$AMC
      gMC[[ l ]]    <- stats[[ l ]]$gMC
      ngMC[[ l ]]   <- stats[[ l ]]$ngMC
      ngAMC[[ l ]]  <- stats[[ l ]]$ngAMC
      exMC[[ l ]]   <- stats[[ l ]]$exMC
    }

    # ------ compute MC statistics vectors ------

    # compute each variable statistics in parallel
    stats <- autoLapply( cl, list( oMC, AMC, gMC, ngMC, ngAMC, exMC ),
                         comp_MC_stats )

    o   <- stats[[ 1 ]]
    A   <- stats[[ 2 ]]
    g   <- stats[[ 3 ]]
    ng  <- stats[[ 4 ]]
    ngA <- stats[[ 5 ]]
    ex  <- stats[[ 6 ]]

    # ------ build statistics table ------

    key.stats <- matrix( c( o$avg$avg, A$avg$avg, g$avg$avg, ng$avg$avg,
                            ngA$avg$avg, ex$avg$avg,

                            o$se$avg, A$se$avg, g$se$avg, ng$se$avg,
                            ngA$se$avg, ex$se$avg,

                            o$sd$avg, A$sd$avg, g$sd$avg, ng$sd$avg,
                            ngA$sd$avg, ex$sd$avg,

                            o$avg$subbo$b, A$avg$subbo$b, g$avg$subbo$b,
                            ng$avg$subbo$b, ngA$avg$subbo$b, ex$avg$subbo$b,

                            o$se$subbo$b, A$se$subbo$b, g$se$subbo$b,
                            ng$se$subbo$b, ngA$se$subbo$b, ex$se$subbo$b,

                            o$sd$subbo$b, A$sd$subbo$b, g$sd$subbo$b,
                            ng$sd$subbo$b, ngA$sd$subbo$b, ex$sd$subbo$b,

                            o$avg$subbo$a, A$avg$subbo$a, g$avg$subbo$a,
                            ng$avg$subbo$a, ngA$avg$subbo$a, ex$avg$subbo$a,

                            o$se$subbo$a, A$se$subbo$a, g$se$subbo$a,
                            ng$se$subbo$a, ngA$se$subbo$a, ex$se$subbo$a,

                            o$sd$subbo$a, A$sd$subbo$a, g$sd$subbo$a,
                            ng$sd$subbo$a, ngA$sd$subbo$a, ex$sd$subbo$a,

                            o$avg$subbo$m, A$avg$subbo$m, g$avg$subbo$m,
                            ng$avg$subbo$m, ngA$avg$subbo$m, ex$avg$subbo$m,

                            o$se$subbo$m, A$se$subbo$m, g$se$subbo$m,
                            ng$se$subbo$m, ngA$se$subbo$m, ex$se$subbo$m,

                            o$sd$subbo$m, A$sd$subbo$m, g$sd$subbo$m,
                            ng$sd$subbo$m, ngA$sd$subbo$m, ex$sd$subbo$m,

                            o$avg$ac$t1, A$avg$ac$t1, g$avg$ac$t1,
                            ng$avg$ac$t1, ngA$avg$ac$t1, ex$avg$ac$t1,

                            o$se$ac$t1, A$se$ac$t1, g$se$ac$t1,
                            ng$se$ac$t1, ngA$se$ac$t1, ex$se$ac$t1,

                            o$sd$ac$t1, A$sd$ac$t1, g$sd$ac$t1,
                            ng$sd$ac$t1, ngA$sd$ac$t1, ex$sd$ac$t1,

                            o$avg$ac$t2, A$avg$ac$t2, g$avg$ac$t2,
                            ng$avg$ac$t2, ngA$avg$ac$t2, ex$avg$ac$t2,

                            o$se$ac$t2, A$se$ac$t2, g$se$ac$t2,
                            ng$se$ac$t2, ngA$se$ac$t2, ex$se$ac$t2,

                            o$sd$ac$t2, A$sd$ac$t2, g$sd$ac$t2,
                            ng$sd$ac$t2, ngA$sd$ac$t2, ex$sd$ac$t2 ),

                         ncol = 6, byrow = TRUE )
    colnames( key.stats ) <- c( "Output(log)", "Prod.(log)", "Output Gr.",
                                "N.Output Gr.", "N.Prod.Gr.", "Exit Rate")
    rownames( key.stats ) <- c( "average", " (s.e.)", " (s.d.)", "Subbotin b",
                                " (s.e.)", " (s.d.)", "Subbotin a", " (s.e.)",
                                " (s.d.)", "Subbotin m", " (s.e.)", " (s.d.)",
                                "autocorr. t-1", " (s.e.)",  " (s.d.)",
                                "autocorr. t-2", " (s.e.)",  " (s.d.)" )

    textplot( formatC( key.stats, digits = sDigits, format = "g" ), cmar = 1.0 )
    title <- paste( "Monte Carlo firm-level statistics (", legends[ k ], ")" )
    subTitle <- paste( paste0( "( Sample size = ", nElem[ k ],
                               " firms / MC runs = ", nSize, " / Period = ",
                               warmUp + 1, "-", nTstat, " )" ),
                       paste( "(", sector, ")" ), sep ="\n" )
    title( main = title, sub = subTitle )

    saveCSV( key.stats, baseName = repName, num = k, baseFolder = folder,
             subFolder = outDir, suffix = datFilSfx, type = "MC" )
  }

  stopCores( cl )

  clearTemp( savDat = savDat, files = files, nExp = nExp,
             folder = folder, baseName = baseName, datFilSfx = datFilSfx )


  # ------ End of report ------

}, interrupt = catchError, error = catchError, finally = showEndMark( ) )
