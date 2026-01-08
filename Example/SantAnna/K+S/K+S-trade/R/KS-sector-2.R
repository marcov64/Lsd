#******************************************************************
#
# ------------ K+S consumer-goods firm analysis -----------------
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
firmTypes <- c( "Pre-change firms", "Post-change firms" ) # firm-type captions
caption   <- "K+S consumption-good firm analysis"         # caption for logs
sector    <- "Consumption-goods sector"                   # caption for plots
datFilSfx <- "firm2"                                      # data file suffix

# firm-level variables to import and add
origVar   <- c( "_A2", "_L2", "_Q2e", "_f2", "_postChg", "_s2avg", "_w2realAvg" )
addVar    <- c( "A2d", "f2l", "growth2", "lA2", "lnormO2", "lnormO2grow",
                "lnormO2match", "lnormO2matchLag", "lrwAvg2", "normA2",
                "normA2grow", "normApreChg2", "normApostChg2", "normO2",
                "normO2grow", "normRW2", "normRW2grow", "normSK2", "normSK2grow",
                "normRWpreChg2", "normRWpostChg2" )


# ==== Libraries, support functions and global options ====

source( "KS-support-functions.R" )
source( "KS-FHK-decomposition.R" )
source( "KS-DN-decomposition.R" )
source( "KS-wage-prod-regression.R" )

options( warn = 0 )           # -1=no warning/0:warnings at end/2:warnings stop

# remove RStudio warnings for support functions
# !diagnostics suppress = showStartMark, showEndMark, setCmdLinePars, loadData
# !diagnostics suppress = catchError, startTime, mean.clean, vector.clean
# !diagnostics suppress = logNA, textplot, plot_lognorm, plot_norm, setLabels
# !diagnostics suppress = plot_laplace, plot_lin, comp_stats, saveCSV, legends
# !diagnostics suppress = comp_MC_stats, size_bins, expLeg, allLeg
# !diagnostics suppress = FHK_decomp, DN_decomp, wage_prod_regr, npreg, se
# !diagnostics suppress = startCores, stopCores, autoLapply, clearTemp, repFile
# !diagnostics suppress = mc, pool, nElem, nElemMC, nTsteps, nSize, outDir


# ==== Define function to create new variables ====

# function to add new variables to Monte Carlo sample (to be parallelized)
addVarFn <- function( mc, nElemMC, nTsteps, nVar ) {

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

nBins     <- 15      # number of bins to use in histograms
outLim    <- 0.10    # outlier percentile (0=don't remove outliers)
limOutl   <- 0.10    # quantile extreme limits (0=none)
warmUp    <- 300     # number of "warm-up" time steps
nTstat    <- -1      # last period to consider for statistics (-1=all)
csBeg     <- 300     # beginning step for cross-section regressions and DN decomposition
csEnd     <- 307     # last step for cross-section regressions and DN decomposition
fShare    <- "f2l"   # share variable to use in decomposition ("f2l"=labor/"f2"=output)
prdWnd    <- 8       # FHK productivity decomposition rolling window period
csJump    <- 4       # jump between multi-step cross-section regressions
ekOrd     <- 4       # Epanechnikov kernel order on non-parametric regression
ekPlt     <- TRUE    # plot all kernel regression fits in separate report

cores     <- 5       # maximum number of cores to allocate (0=all)
parStats  <- 10      # number of statistics to be computed in parallel
repName   <- ""      # report files base name (if "" same baseName)
sDigits   <- 4       # significant digits in tables
plotRows  <- 1       # number of plots per row in a page
plotCols  <- 1  	   # number of plots per column in a page
plotW     <- 10      # plot window width
plotH     <- 7       # plot window height
raster    <- FALSE   # raster or vector plots
res       <- 600     # resolution of raster mode (in dpi)

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
if( csBeg < 1 || csBeg >= nTsteps || csBeg < warmUp )
  csBeg <- warmUp
if( csEnd < 1 || csEnd >= nTsteps || csEnd <= warmUp || csEnd <= csBeg )
  csEnd <- nTsteps - 1


# ====== Monte Carlo stats function ======

# function to treat one Monte Carlo sample (to be parallelized)
statsMC <- function( nMC, exper, nSize, filesMC ) {

  cat( "  Monte Carlo case", nMC, "of", nSize, "\n" )

  # load MC data from temporary files
  load( filesMC[ nMC ] )

  # ------ Update MC statistics lists ------

  oData   <- logNA( mc[ TmaskStat, "Q2e", ] )
  Adata   <- logNA( mc[ TmaskStat, "A2", ] )
  wData   <- logNA( mc[ TmaskStat, "w2realAvg", ] )
  sData   <- logNA( mc[ TmaskStat, "s2avg", ] )
  gData   <- mc[ TmaskStat, "growth2", ]
  ngData  <- mc[ TmaskStat, "normO2grow", ]
  ngAdata <- mc[ TmaskStat, "normA2grow", ]
  nwgData <- mc[ TmaskStat, "normRW2grow", ]
  nsgData <- mc[ TmaskStat, "normSK2grow", ]
  exData  <- as.numeric( is.na( mc[ TmaskStat, "growth2", ] ) )

  rm( mc )

  # initiate 2nd level cluster for parallel processing of statistics
  cl2 <- startCores( cores, parStats )

  # compute each variable statistics in parallel
  stat <- autoLapply( cl2, list( oData, Adata, wData, sData, gData, ngData,
                                 ngAdata, nwgData, nsgData, exData ),
                      comp_stats )

  stopCores( cl2 )          # stop 2nd level cluster

  return( list( oMC = stat[[ 1 ]], AMC = stat[[ 2 ]], wMC = stat[[ 3 ]],
                sMC = stat[[ 4 ]], gMC = stat[[ 5 ]], ngMC = stat[[ 6 ]],
                ngAMC = stat[[ 7 ]], nwgMC = stat[[ 8 ]], nsgMC = stat[[ 9 ]],
                exMC = stat[[ 10 ]], nElemMC = nElemMC ) )
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

  szData <- oData <- Adata <- AprData <- ApoData <- wData <-
    wPrData <- wPoData <- sData <- gData <- ngData <- ngAdata <- nwgData <-
    nsgData <- exData <- svData <- bins <- list( )

  # run over all experiments individually
  for( k in 1 : nExp ) {

    cat( "", expLeg, k, "of", nExp, "\n" )

    # load pooled data from temporary files (first already loaded)
    if( k > 1 )
      load( files[[ k ]]$pool )

    #
    # ------ Create data & statistics vectors ------
    #

    szData[[ k ]]   <- as.vector( pool[ TmaskStat, "Q2e", ] )
    oData[[ k ]]    <- as.vector( logNA( pool[ TmaskStat, "normO2", ] ) )
    Adata[[ k ]]    <- as.vector( logNA( pool[ TmaskStat, "normA2", ] ) )
    AprData[[ k ]]  <- as.vector( logNA( pool[ TmaskStat, "normApreChg2", ] ) )
    ApoData[[ k ]]  <- as.vector( logNA( pool[ TmaskStat, "normApostChg2", ] ) )
    wData[[ k ]]    <- as.vector( logNA( pool[ TmaskStat, "normRW2", ] ) )
    wPrData[[ k ]]  <- as.vector( logNA( pool[ TmaskStat, "normRWpreChg2", ] ) )
    wPoData[[ k ]]  <- as.vector( logNA( pool[ TmaskStat, "normRWpostChg2", ] ) )
    sData[[ k ]]    <- as.vector( logNA( pool[ TmaskStat,"normSK2", ] ) )
    gData[[ k ]]    <- as.vector( pool[ TmaskStat, "growth2", ] )
    ngData[[ k ]]   <- as.vector( pool[ TmaskStat, "normO2grow", ] )
    ngAdata[[ k ]]  <- as.vector( pool[ TmaskStat, "normA2grow", ] )
    nwgData[[ k ]]  <- as.vector( pool[ TmaskStat, "normRW2grow", ] )
    nsgData[[ k ]]  <- as.vector( pool[ TmaskStat, "normSK2grow", ] )
    exData[[ k ]]   <- as.vector( as.numeric( is.na( pool[ TmaskStat, "growth2", ] ) ) )

    # prepare data for Gibrat and scaling variance plots
    bins[[ k ]] <- size_bins( as.vector( pool[ TmaskStat, "lnormO2match", ] ),
                              as.vector( pool[ TmaskStat, "lnormO2matchLag", ] ),
                              as.vector( pool[ TmaskStat, "lnormO2grow", ] ),
                              bins = 2 * nBins, outLim = outLim )

    rm( pool, P, S, C, c, M, m, n )

    # remove NAs
    szData[[ k ]]   <- szData[[ k ]][ ! is.na( szData[[ k ]] ) ]
    oData[[ k ]]    <- oData[[ k ]][ ! is.na( oData[[ k ]] ) ]
    Adata[[ k ]]    <- Adata[[ k ]][ ! is.na( Adata[[ k ]] ) ]
    AprData[[ k ]]  <- AprData[[ k ]][ ! is.na( AprData[[ k ]] ) ]
    ApoData[[ k ]]  <- ApoData[[ k ]][ ! is.na( ApoData[[ k ]] ) ]
    wData[[ k ]]    <- wData[[ k ]][ ! is.na( wData[[ k ]] ) ]
    wPrData[[ k ]]  <- wPrData[[ k ]][ ! is.na( wPrData[[ k ]] ) ]
    wPoData[[ k ]]  <- wPoData[[ k ]][ ! is.na( wPoData[[ k ]] ) ]
    sData[[ k ]]    <- sData[[ k ]][ ! is.na( sData[[ k ]] ) ]
    gData[[ k ]]    <- gData[[ k ]][ ! is.na( gData[[ k ]] ) ]
    ngData[[ k ]]   <- ngData[[ k ]][ ! is.na( ngData[[ k ]] ) ]
    ngAdata[[ k ]]  <- ngAdata[[ k ]][ ! is.na( ngAdata[[ k ]] ) ]
    nwgData[[ k ]]  <- nwgData[[ k ]][ ! is.na( nwgData[[ k ]] ) ]
    nsgData[[ k ]]  <- nsgData[[ k ]][ ! is.na( nsgData[[ k ]] ) ]
    exData[[ k ]]   <- exData[[ k ]][ ! is.na( exData[[ k ]] ) ]

    # compute each variable statistics in parallel
    stats <- autoLapply( cl, list( oData[[ k ]], Adata[[ k ]], wData[[ k ]],
                                   sData[[ k ]], gData[[ k ]], ngData[[ k ]],
                                   ngAdata[[ k ]], nwgData[[ k ]], nsgData[[ k ]],
                                   exData[[ k ]] ), comp_stats )

    o   <- stats[[ 1 ]]
    A   <- stats[[ 2 ]]
    w   <- stats[[ 3 ]]
    s   <- stats[[ 4 ]]
    g   <- stats[[ 5 ]]
    ng  <- stats[[ 6 ]]
    ngA <- stats[[ 7 ]]
    nwg <- stats[[ 8 ]]
    nsg <- stats[[ 9 ]]
    ex  <- stats[[ 10 ]]

    #
    # ------ Build statistics table ------
    #

    key.stats <- matrix( c( o$avg, A$avg, w$avg, s$avg, g$avg,
                            ng$avg, ngA$avg, nwg$avg, nsg$avg, ex$avg,

                            o$sd / sqrt( nSize ), A$sd / sqrt( nSize ),
                            w$sd / sqrt( nSize ), s$sd / sqrt( nSize ),
                            g$sd / sqrt( nSize ), ng$sd / sqrt( nSize ),
                            ngA$sd / sqrt( nSize ), nwg$sd / sqrt( nSize ),
                            nsg$sd / sqrt( nSize ), ex$sd / sqrt( nSize ),

                            o$sd, A$sd, w$sd, s$sd, g$sd, ng$sd,
                            ngA$sd, nwg$sd, nsg$sd, ex$sd,

                            o$subbo$b, A$subbo$b, w$subbo$b, s$subbo$b,
                            g$subbo$b, ng$subbo$b, ngA$subbo$b, nwg$subbo$b,
                            nsg$subbo$b, ex$subbo$b,

                            o$subbo$a, A$subbo$a, w$subbo$a, s$subbo$a,
                            g$subbo$a, ng$subbo$a, ngA$subbo$a, nwg$subbo$a,
                            nsg$subbo$a, ex$subbo$a,

                            o$subbo$m, A$subbo$m, w$subbo$m, s$subbo$m,
                            g$subbo$m, ng$subbo$m, ngA$subbo$m, nwg$subbo$m,
                            nsg$subbo$m, ex$subbo$m,

                            o$jb$statistic, A$jb$statistic, w$jb$statistic,
                            s$jb$statistic, g$jb$statistic, ng$jb$statistic,
                            ngA$jb$statistic, nwg$jb$statistic, nsg$jb$statistic,
                            ex$jb$statistic,

                            o$jb$p.value, A$jb$p.value, w$jb$p.value,
                            s$jb$p.value, g$jb$p.value, ng$jb$p.value,
                            ngA$jb$p.value, nwg$jb$p.value, nsg$jb$p.value,
                            ex$jb$p.value,

                            o$ll$statistic, A$ll$statistic, w$ll$statistic,
                            s$ll$statistic, g$ll$statistic, ng$ll$statistic,
                            ngA$ll$statistic, nwg$ll$statistic, nsg$ll$statistic,
                            ex$ll$statistic,

                            o$ll$p.value, A$ll$p.value, w$ll$p.value,
                            s$ll$p.value, g$ll$p.value, ng$ll$p.value,
                            ngA$ll$p.value, nwg$ll$p.value, nsg$ll$p.value,
                            ex$ll$p.value,

                            o$ad$statistic, A$ad$statistic, w$ad$statistic,
                            s$ad$statistic, g$ad$statistic, ng$ad$statistic,
                            ngA$ad$statistic, nwg$ad$statistic, nsg$ad$statistic,
                            ex$ad$statistic,

                            o$ad$p.value, A$ad$p.value, w$ad$p.value,
                            s$ad$p.value, g$ad$p.value, ng$ad$p.value,
                            ngA$ad$p.value, nwg$ad$p.value, nsg$ad$p.value,
                            ex$ad$p.value,

                            o$ac$t1, A$ac$t1, w$ac$t1, s$ac$t1, g$ac$t1,
                            ng$ac$t1, ngA$ac$t1, nwg$ac$t1, nsg$ac$t1, ex$ac$t1,

                            o$ac$t2, A$ac$t2, w$ac$t2, s$ac$t2, g$ac$t2,
                            ng$ac$t2, ngA$ac$t2, nwg$ac$t2, nsg$ac$t2, ex$ac$t2 ),

                         ncol = 10, byrow = TRUE )
    colnames( key.stats ) <- c( "N.Output(log)", "N.Prod.(log)", "N.R.Wage(log)",
                                "N.Skills(log)", "Output Gr.", "N.Output Gr.",
                                "N.Prod.Gr.", "N.R.Wage Gr.", "N.Skills Gr.",
                                "Exit Rate" )
    rownames( key.stats ) <- c( "average", " (s.e.)", " (s.d.)", "Subbotin b",
                                " a", " m", "Jarque-Bera X2", " (p-val.)",
                                "Lilliefors D", " (p-val.)", "Anderson-Darling A",
                                " (p-val.)", "autocorr. t-1", "autocorr. t-2" )

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


    # ------ Scaling variance plot ------

    plot_lin( bins[[ k ]]$s2avg, bins[[ k ]]$gSD,
              xlab = "Log-normalized size of firms",
              ylab = "Log standard deviation of growth rate",
              tit = paste( "Scaling of variance of firm size (", legends[ k ], ")" ),
              subtit = subTitle )
  }


  #
  # ====== Plot distributions (overplots)  ======
  #

  # ------ Size distributions (binned density x log-level variable)  ------

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

  plot_norm( AprData, xlab = "Pooled log-normalized productivity",
             ylab = "Binned density",
             tit = paste( "Productivity distribution (pre-change firms)", allLeg ),
             subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
             col = colors, lty = lTypes, pty = pTypes )

  plot_norm( ApoData, xlab = "Pooled log-normalized productivity",
             ylab = "Binned density",
             tit = paste( "Productivity distribution (post-change firms)", allLeg ),
             subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
             col = colors, lty = lTypes, pty = pTypes )

  plot_norm( wData, xlab = "Pooled log-normalized average real wage",
             ylab = "Binned density",
             tit = paste( "Wage distribution", allLeg ),
             subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
             col = colors, lty = lTypes, pty = pTypes )

  plot_norm( wPrData, xlab = "Pooled log-normalized average real wage",
             ylab = "Binned density",
             tit = paste( "Wage distribution (pre-change firms)", allLeg ),
             subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
             col = colors, lty = lTypes, pty = pTypes )

  plot_norm( wPoData, xlab = "Pooled log-normalized average real wage",
             ylab = "Binned density",
             tit = paste( "Wage distribution (post-change firms)", allLeg ),
             subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
             col = colors, lty = lTypes, pty = pTypes )

  plot_norm( sData, xlab = "Pooled log-normalized average skills level",
             ylab = "Binned density",
             tit = paste( "Skills distribution", allLeg ),
             subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
             col = colors, lty = lTypes, pty = pTypes )

  # ------ Size distributions (log size x rank)  ------

  plot_lognorm( szData, xlab = "Pooled output",
                ylab = "Rank",
                tit = paste( "Size distribution", allLeg ),
                subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
                col = colors, lty = lTypes )

  # ------ Growth rate distributions (binned density x log growth rate)  ------

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

  plot_laplace( nwgData, xlab = "Pooled normalized average real wage growth rate",
                ylab = "Binned density",
                tit = paste( "Average wage growth distribution", allLeg ),
                subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
                col = colors, lty = lTypes, pty = pTypes )

  plot_laplace( nsgData, xlab = "Pooled normalized average skills level growth rate",
                ylab = "Binned density",
                tit = paste( "Average skills growth distribution", allLeg ),
                subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
                col = colors, lty = lTypes, pty = pTypes )

  rm( szData, oData, Adata, AprData, ApoData, wData, wPrData, wPoData, sData,
      gData, ngData, ngAdata, nwgData, nsgData, exData, svData, bins )


  #
  # ====== Monte Carlo analysis ======
  #

  cat( "\nGenerating MC reports...\n" )

  nElem <- rep( 0, nExp )

  for( k in 1 : nExp ) {             # do for each experiment

    cat( "", expLeg, k, "of", nExp, "\n" )

    # ------ compute single MC statistics ------

    # compute each MC case in parallel
    stats <- autoLapply( cl, 1 : nSize, statsMC, k, nSize, files[[ k ]]$mc )

    # reorganize data
    oMC <- AMC <- wMC <- sMC <- gMC <- ngMC <- ngAMC <-
      nwgMC <- nsgMC <- exMC <- list( )

    for( l in 1 : nSize ) {         # for each MC run
      nElem[ k ]  <- nElem[ k ] + stats[[ l ]]$nElemMC
      oMC[[ l ]]    <- stats[[ l ]]$oMC
      AMC[[ l ]]    <- stats[[ l ]]$AMC
      wMC[[ l ]]    <- stats[[ l ]]$wMC
      sMC[[ l ]]    <- stats[[ l ]]$sMC
      gMC[[ l ]]    <- stats[[ l ]]$gMC
      ngMC[[ l ]]   <- stats[[ l ]]$ngMC
      ngAMC[[ l ]]  <- stats[[ l ]]$ngAMC
      nwgMC[[ l ]]  <- stats[[ l ]]$nwgMC
      nsgMC[[ l ]]  <- stats[[ l ]]$nsgMC
      exMC[[ l ]]   <- stats[[ l ]]$exMC
    }

    # ------ compute MC statistics vectors ------

    # compute each variable statistics in parallel
    stats <- autoLapply( cl, list( oMC, AMC, wMC, sMC, gMC, ngMC, ngAMC, nsgMC,
                                   nwgMC, exMC ), comp_MC_stats )

    o   <- stats[[ 1 ]]
    A   <- stats[[ 2 ]]
    w   <- stats[[ 3 ]]
    s   <- stats[[ 4 ]]
    g   <- stats[[ 5 ]]
    ng  <- stats[[ 6 ]]
    ngA <- stats[[ 7 ]]
    nsg <- stats[[ 8 ]]
    nwg <- stats[[ 9 ]]
    ex  <- stats[[ 10 ]]

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
                                "N.Prod.Gr.", "N.R.Wage Gr.", "N.Skills Gr.",
                                "Exit Rate" )
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


  #
  # ====== experiment-specific plots ======
  #

  #
  # ------ Productivity decomposition ------
  #

  cat( "\nPerforming FHK productivity decomposition...\n" )

  FHK_decomp( nExp, nSize, nTstat, nElem, warmUp, prdWnd, fShare, files,
              sDigits, legends, expLeg, sector, folder, outDir, repName,
              datFilSfx, firmTypes )

  cat( "\nPerforming DN productivity decomposition...\n" )

  DN_decomp( nExp, nSize, nElem, csBeg, csEnd, fShare, files, sDigits, legends,
             expLeg, sector, folder, outDir, repName, datFilSfx, firmTypes )

  #
  # ------ Wage x productivity regressions ------
  #

  cat( "\nPerforming wage x productivity regressions...\n" )

  wage_prod_regr( files, nExp, nSize, warmUp, nTstat, sDigits, legends, expLeg,
                  sector, csBeg, csEnd, csJump, ekOrd, ekPlt, outLim, CI,
                  folder, outDir, repName, datFilSfx, raster, res, plotH, plotW,
                  plotRows, plotCols )


  stopCores( cl )

  clearTemp( savDat = savDat, files = files, nExp = nExp,
             folder = folder, baseName = baseName, datFilSfx = datFilSfx )


  # ------ End of report ------

}, interrupt = catchError, error = catchError, finally = showEndMark( ) )
