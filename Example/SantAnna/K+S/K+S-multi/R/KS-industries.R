#******************************************************************
#
# --------- K+S consumption-goods industries analysis -----------
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
caption   <- "K+S industry analysis"                      # caption for logs
sector    <- "Consumer-goods sector"                      # caption for plots
datFilSfx <- "ind"                                        # data file suffix

# country, capital and consumer industry-level variables to  to import and add
origVarC  <- c( "g1max" )
origVar1  <- c( "B1", "D1", "L1d" )
origVar2  <- c( "A2", "A2p", "D2", "D2d", "Deb2", "F2", "HH2", "L2", "L2d",
               "K2d", "NW2", "S2", "VA2w", "f2e", "k2", "t2ent", "type2",
               "w2realAvg", "w2realSD", "f2posChg" )
origVar   <- c( origVarC, origVar1, origVar2 )

addVar    <- c( "age2", "normA2", "normA2p", "normD2", "normD2d", "normDeb2",
              "normK2d", "normNW2", "normRW2", "normA2pGrow", "normD2grow",
              "normD2dGrow", "normRW2grow", "normRW2sd" )


# ==== Libraries, support functions and global options ====

source( "KS-support-functions.R" )

options( warn = -1 )         # -1=no warning/0:warnings at end/2:warnings stop

# remove RStudio warnings for support functions
# !diagnostics suppress = showStartMark, showEndMark, setCmdLinePars, loadData
# !diagnostics suppress = catchError, startTime, mean.clean, vector.clean
# !diagnostics suppress = logNA, bkfilter, colSds, textplot, setLabels, rem_dec
# !diagnostics suppress = fit_subbotin, plot_lognorm, plot_norm, plot_laplace
# !diagnostics suppress = plot_lin, plot_loglin, comp_stats, compertz_model
# !diagnostics suppress = startCores, stopCores, autoLapply, clearTemp, repFile
# !diagnostics suppress = light_color, alignSeries, saveCSV, mc, pool, nTsteps
# !diagnostics suppress = nElem, nElemMC, nSize, outDir
# !diagnostics suppress = legends, expLeg, cntLeg, allLeg


# ==== Define function to create new variables ====

# function to add new variables to Monte Carlo sample (to be parallelized)
addVarFn <- function( mc, nElemMC, nTsteps, nVar ) {

  origVar2 <- c( "A2", "A2p", "D2", "D2d", "Deb2", "F2", "HH2", "L2", "L2d",
                 "K2d", "NW2", "S2", "VA2w", "f2e", "k2", "t2ent", "type2",
                 "w2realAvg", "w2realSD", "f2posChg" )

  for( i in 1 : nTsteps ) {                # all time steps

    age2 <- i - vector.clean( mc[ i, "t2ent", ] )
    mean.A2 <- mean.clean( vector.clean( mc[ i, "A2", ] ) )
    mean.A2p <- mean.clean( vector.clean( mc[ i, "A2p", ] ) )
    mean.D2 <- mean.clean( vector.clean( mc[ i, "D2", ] ) )
    mean.D2d <- mean.clean( vector.clean( mc[ i, "D2d", ] ) )
    mean.Deb2 <- mean.clean( vector.clean( mc[ i, "Deb2", ] ) )
    mean.K2d <- mean.clean( vector.clean( mc[ i, "K2d", ] ) )
    mean.NW2 <- mean.clean( vector.clean( mc[ i, "NW2", ] ) )
    mean.w2realAvg <- mean.clean( vector.clean( mc[ i, "w2realAvg", ] ) )
    mean.w2realSD <- mean.clean( vector.clean( mc[ i, "w2realSD", ] ) )

    for( j in 1 : nElemMC ) {             # and all industry instances

      # take care of entrants' first period and other problems to avoid artifacts
      if( any( ! is.finite( mc[ i, origVar2, j ] ) ) ) {
        mc[ i, origVar2, j ] <- NA        # don't mess with other variables
        next
      }

      mc[ i, "age2", j ] <- age2[ j ]

      # normalization of key variables using the period average size
      if( mean.A2 != 0 )
        mc[ i, "normA2", j ] <- mc[ i, "A2", j ] / mean.A2
      if( mean.A2p != 0 )
        mc[ i, "normA2p", j ] <- mc[ i, "A2p", j ] / mean.A2p
      if( mean.D2 != 0 )
        mc[ i, "normD2", j ] <- mc[ i, "D2", j ] / mean.D2
      if( mean.D2d != 0 )
        mc[ i, "normD2d", j ] <- mc[ i, "D2d", j ] / mean.D2d
      if( mean.Deb2 != 0 )
        mc[ i, "normDeb2", j ] <- mc[ i, "Deb2", j ] / mean.Deb2
      if( mean.K2d != 0 )
        mc[ i, "normK2d", j ] <- mc[ i, "K2d", j ] / mean.K2d
      if( mean.NW2 != 0 )
        mc[ i, "normNW2", j ] <- mc[ i, "NW2", j ] / mean.NW2
      if( mean.w2realAvg != 0 )
        mc[ i, "normRW2", j ] <- mc[ i, "w2realAvg", j ] / mean.w2realAvg
      if( mean.w2realSD != 0 )
        mc[ i, "normRW2sd", j ] <- mc[ i, "w2realSD", j ] / mean.w2realSD

      # growth rates and deltas calculated from 2nd period and for non-entrants
      if( i > 1 ) {
        if( is.finite( mc[ i - 1, "A2p", j ] ) ) {

          # Size, normalized sales and productivity growth
          mc[ i, "normA2pGrow", j ] <- log( mc[ i, "normA2p", j ] ) -
            log( mc[ i - 1, "A2p", j ] )
          mc[ i, "normD2grow", j ] <- log( mc[ i, "normD2", j ] ) -
            log( mc[ i - 1, "normD2", j ] )
          mc[ i, "normD2dGrow", j ] <- log( mc[ i, "normD2d", j ] ) -
            log( mc[ i - 1, "normD2d", j ] )
          mc[ i, "normRW2grow", j ] <- log( mc[ i, "normRW2", j ] ) -
            log( mc[ i - 1, "normRW2", j ] )

          if( ! is.finite( mc[ i, "normA2pGrow", j ] ) )
            mc[ i, "normA2pGrow", j ] <- NA
          if( ! is.finite( mc[ i, "normD2grow", j ] ) )
            mc[ i, "normD2grow", j ] <- NA
          if( ! is.finite( mc[ i, "normD2dGrow", j ] ) )
            mc[ i, "normD2dGrow", j ] <- NA
          if( ! is.finite( mc[ i, "normRW2grow", j ] ) )
            mc[ i, "normRW2grow", j ] <- NA
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
outLim    <- 0.10   # outlier percentile (0=don't remove outliers)
warmUp    <- 300    # number of "warm-up" time steps
nTstat    <- 400    # last period to consider for statistics (-1=all)
relTmax   <- 150    # maximum relative time to consider in plots
limOutl   <- 0.10   # quantile extreme limits (0=none)
lowP      <- 6      # bandpass filter minimum period
highP     <- 32     # bandpass filter maximum period
bpfK      <- 12     # bandpass filter order
lags      <- 4      # lags to analyze

cores     <- 8      # maximum number of cores to allocate (0=all)
parStats  <- 15     # number of statistics to be computed in parallel
repName   <- ""     # report files base name (if "" same baseName)
sDigits   <- 4      # significant digits in tables
plotRows  <- 1      # number of plots per row in a page
plotCols  <- 1  	  # number of plots per column in a page
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
statsMC <- function( nMC, exper, nSize, filesMC,
                     lags, lowP, highP, bpfK ) {

  cat( "  Monte Carlo case", nMC, "of", nSize, "\n" )

  # load MC data from temporary files
  load( filesMC[ nMC ] )

  # ------ Update MC statistics lists ------

  ccA2mc <- ccB1mc <- ccD1mc <- ccD2mc <- ccL1mc <- ccL2mc <- ccg1mc <-
    matrix( nrow = 0, ncol = 2 * lags + 1 )
  indTypeMC <- indStartMC <- vector( )
  DcurveMC <- LcurveMC <- KLbasMC <- KLluxMC <- fBasMC <- fLuxMC <- list( )

  # ------ MC single-instance statistics ------

  B1 <- logNA( mc[ TmaskStat, "B1", 1 ] )
  D1 <- logNA( mc[ TmaskStat, "D1", 1 ] )
  L1d <- mc[ TmaskStat, "L1d", 1 ]
  g1 <- mc[ TmaskStat, "g1max", 1 ]


  B1 <- B1[ ! is.na( B1 ) ]
  D1 <- D1[ ! is.na( D1 ) ]
  L1d <- L1d[ ! is.na( L1d ) ]
  g1 <- g1[ ! is.na( g1 ) ]

  # ------ collect correlation structures ------

  ref <- 3                          # series to be used as reference
  set1 <- alignSeries( list( B1, D1, L1d ), ref )  # series to analyze

  for( i in 1 : length( set1 ) )
    if( length( set1[[ i ]] ) >  2 * bpfK ) {
      set1[[ i ]] <- bkfilter( set1[[ i ]], pl = lowP, pu = highP,
                               nfix = bpfK )$cycle[ , 1 ]
    } else {
      set1[[ i ]] <- NA
    }

  if( length( set1[[ 1 ]] ) > 1 )
    ccB1mc <- rbind( ccB1mc, ccf( set1[[ ref ]], set1[[ 1 ]], lag.max = lags,
                                  plot = FALSE, na.action = na.pass )$acf )
  if( length( set1[[ 2 ]] ) > 1 )
    ccD1mc <- rbind( ccD1mc, ccf( set1[[ ref ]], set1[[ 2 ]], lag.max = lags,
                                  plot = FALSE, na.action = na.pass )$acf )
  if( length( set1[[ 3 ]] ) > 1 )
    ccL1mc <- rbind( ccL1mc, ccf( set1[[ ref ]], set1[[ 3 ]], lag.max = lags,
                                  plot = FALSE, na.action = na.pass )$acf )

  # ------ MC multi-instance statistics ------

  for( j in 1 : nElemMC ) {

    if( all( is.na( mc[ TmaskStat, "type2", j ] ) ) )
      next

    A2 <- mc[ TmaskStat, "normA2p", j ]
    D2 <- mc[ TmaskStat, "normD2", j ]
    D2d <- mc[ TmaskStat, "normD2d", j ]
    L2 <- mc[ TmaskStat, "L2", j ]
    L2d <- mc[ TmaskStat, "L2d", j ]
    K2d <- mc[ TmaskStat, "K2d", j ]
    f2 <- mc[ TmaskStat, "f2e", j ]
    type2 <- max( mc[ TmaskStat, "type2", j ], na.rm = TRUE )

    A2 <- A2[ ! is.na( A2 ) ]
    D2 <- D2[ ! is.na( D2 ) ]
    D2d <- D2d[ ! is.na( D2d ) ]
    L2 <- L2[ ! is.na( L2 ) ]
    L2d <- L2d[ ! is.na( L2d ) ]
    K2d <- K2d[ ! is.na( K2d ) ]
    f2 <- f2[ ! is.na( f2 ) ]

    # ------ collect demand growth shapes ------

    # remove pre-production period
    while( length( D2 ) > 1 && ( is.na( D2[ 1 ] ) || D2[ 1 ] == 0 ) )
      D2 <- D2[ 2 : length( D2 ) ]

    while( length( L2 ) > 1 && ( is.na( L2[ 1 ] ) || L2[ 1 ] == 0 ) )
      L2 <- L2[ 2 : length( L2 ) ]

    # ignore no-data industries
    if( ! is.na( D2[ 1 ] ) && ! is.na( L2[ 1 ] ) ) {
      indTypeMC <- append( indTypeMC, type2 )
      indStartMC <- append( indStartMC, as.numeric( names( L2[ 1 ] ) ) )
      DcurveMC <- append( DcurveMC, list( D2 ) )
      LcurveMC <- append( LcurveMC, list( L2 ) )
    }

    # ------ collect subsector-level variables ------

    if( length( L2d ) > 0 && length( L2d ) == length( K2d ) ) {
      Lpos <- replace( L2d, L2d == 0, NA )
      KLratio <- logNA( K2d / Lpos )
      KLratio <- KLratio[ ! is.na( KLratio ) ]

      if( length( KLratio ) > 0 ) {
        if( type2 == 0 )
          KLbasMC <- append( KLbasMC, list( KLratio ) )
        else
          KLluxMC <- append( KLluxMC, list( KLratio ) )
      }
    }

    if( length( f2 ) > 0 ) {
      if( type2 == 0 )
        fBasMC <- append( fBasMC, list( f2 ) )
      else
        fLuxMC <- append( fLuxMC, list( f2 ) )
    }

    # ------ collect correlation structures ------

    ref <- 3                          # series to be used as reference
    set2 <- alignSeries( list( A2, D2d, L2d, g1 ), ref )  # series to analyze

    for( i in 1 : length( set2 ) )
      if( length( set2[[ i ]] ) >  2 * bpfK ) {
        set2[[ i ]] <- bkfilter( set2[[ i ]], pl = lowP, pu = highP,
                                 nfix = bpfK )$cycle[ , 1 ]
      } else {
        set2[[ i ]] <- NA
      }

    if( length( set2[[ 1 ]] ) > 1 )
      ccA2mc <- rbind( ccA2mc, ccf( set2[[ ref ]], set2[[ 1 ]], lag.max = lags,
                                    plot = FALSE, na.action = na.pass )$acf )
    if( length( set2[[ 2 ]] ) > 1 )
      ccD2mc <- rbind( ccD2mc, ccf( set2[[ ref ]], set2[[ 2 ]], lag.max = lags,
                                    plot = FALSE, na.action = na.pass )$acf )
    if( length( set2[[ 3 ]] ) > 1 )
      ccL2mc <- rbind( ccL2mc, ccf( set2[[ ref ]], set2[[ 3 ]], lag.max = lags,
                                    plot = FALSE, na.action = na.pass )$acf )
    if( length( set2[[ 4 ]] ) > 1 )
      ccg1mc <- rbind( ccg1mc, ccf( set2[[ ref ]], set2[[ 4 ]], lag.max = lags,
                                    plot = FALSE, na.action = na.pass )$acf )
  }

  return( list( indTypeMC = indTypeMC, indStartMC = indStartMC,
                DcurveMC = DcurveMC, LcurveMC = LcurveMC, KLbasMC = KLbasMC,
                KLluxMC = KLluxMC, fBasMC = fBasMC, fLuxMC = fLuxMC,
                ccB1mc = ccB1mc, ccD1mc = ccD1mc, ccL1mc = ccL1mc,
                ccA2mc = ccA2mc, ccD2mc = ccD2mc, ccL2mc = ccL2mc,
                ccg1mc = ccg1mc, nElemMC = nElemMC ) )
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

  Fdata <- Ldata <- ageData <- fData <- nAdata <- nApData <- nDdata <-
    nDdData <- nDebData <- nNWdata <- nwData <- nwSDdata<- nApgData <-
    nDgData <- nDdgData <- nwgData <- HH2data <- f2data <- f2Udata <-
    f2UrwSDdata <- list()

  # run over all experiments individually
  for( k in 1 : nExp ) {

    cat( "", expLeg, k, "of", nExp, "\n" )

    # load pooled data from temporary files (first already loaded)
    if( k > 1 )
      load( files[[ k ]]$pool )

    #
    # ------ Create data & statistics vectors ------
    #

    Fdata[[ k ]]    <- as.vector( pool[ TmaskStat, "F2", ] )
    HH2data[[ k ]]  <- as.vector( pool[ TmaskStat, "HH2", ] )
    Ldata[[ k ]]    <- as.vector( logNA( pool[ TmaskStat, "L2", ] ) )
    fData[[ k ]]    <- as.vector( logNA( pool[ TmaskStat, "f2e", ] ) )
    f2data[[ k ]]   <- as.vector( pool[ TmaskStat, "f2e", ] )
    f2Udata[[ k ]]  <- as.vector( 1 - pool[ TmaskStat, "f2posChg", ] )
    ageData[[ k ]]  <- as.vector( pool[ TmaskStat, "age2", ] )
    nAdata[[ k ]]   <- as.vector( logNA( pool[ TmaskStat, "normA2", ] ) )
    nApData[[ k ]]  <- as.vector( logNA( pool[ TmaskStat, "normA2p", ] ) )
    nDdata[[ k ]]   <- as.vector( logNA( pool[ TmaskStat, "normD2", ] ) )
    nDdData[[ k ]]  <- as.vector( logNA( pool[ TmaskStat, "normD2d", ] ) )
    nDebData[[ k ]] <- as.vector( logNA( pool[ TmaskStat, "normDeb2", ] ) )
    nNWdata[[ k ]]  <- as.vector( logNA( pool[ TmaskStat, "normNW2", ] ) )
    nwData[[ k ]]   <- as.vector( logNA( pool[ TmaskStat,"normRW2", ] ) )
    nwSDdata[[ k ]] <- as.vector( logNA( 1 + pool[ TmaskStat,"normRW2sd", ] ) )
    nApgData[[ k ]] <- as.vector( pool[ TmaskStat, "normA2pGrow", ] )
    nDgData[[ k ]]  <- as.vector( pool[ TmaskStat, "normD2grow", ] )
    nDdgData[[ k ]] <- as.vector( pool[ TmaskStat, "normD2dGrow", ] )
    nwgData[[ k ]]  <- as.vector( pool[ TmaskStat, "normRW2grow", ] )

    f2Udata[[ k ]][ f2Udata[[ k ]] < 0 ] <- 0
    f2UrwSDdata[[ k ]] <- cbind( f2Udata[[ k ]], nwSDdata[[ k ]] )

    rm( pool, P, S, C, c, M, m, n )

    # remove NAs
    Fdata[[ k ]]    <- Fdata[[ k ]][ ! is.na( Fdata[[ k ]] ) ]
    HH2data[[ k ]]  <- HH2data[[ k ]][ ! is.na( HH2data[[ k ]] ) ]
    Ldata[[ k ]]    <- Ldata[[ k ]][ ! is.na( Ldata[[ k ]] ) ]
    ageData[[ k ]]  <- ageData[[ k ]][ ! is.na( ageData[[ k ]] ) ]
    fData[[ k ]]    <- fData[[ k ]][ ! is.na( fData[[ k ]] ) ]
    f2data[[ k ]]   <- f2data[[ k ]][ ! is.na( f2data[[ k ]] ) ]
    f2Udata[[ k ]]  <- f2Udata[[ k ]][ ! is.na( f2Udata[[ k ]] ) ]
    nAdata[[ k ]]   <- nAdata[[ k ]][ ! is.na( nAdata[[ k ]] ) ]
    nApData[[ k ]]  <- nApData[[ k ]][ ! is.na( nApData[[ k ]] ) ]
    nDdata[[ k ]]   <- nDdata[[ k ]][ ! is.na( nDdata[[ k ]] ) ]
    nDdData[[ k ]]  <- nDdData[[ k ]][ ! is.na( nDdData[[ k ]] ) ]
    nDebData[[ k ]] <- nDebData[[ k ]][ ! is.na( nDebData[[ k ]] ) ]
    nNWdata[[ k ]]  <- nNWdata[[ k ]][ ! is.na( nNWdata[[ k ]] ) ]
    nwData[[ k ]]   <- nwData[[ k ]][ ! is.na( nwData[[ k ]] ) ]
    nwSDdata[[ k ]] <- nwSDdata[[ k ]][ ! is.na( nwSDdata[[ k ]] ) ]
    nApgData[[ k ]] <- nApgData[[ k ]][ ! is.na( nApgData[[ k ]] ) ]
    nDgData[[ k ]]  <- nDgData[[ k ]][ ! is.na( nDgData[[ k ]] ) ]
    nDdgData[[ k ]] <- nDdgData[[ k ]][ ! is.na( nDdgData[[ k ]] ) ]
    nwgData[[ k ]]  <- nwgData[[ k ]][ ! is.na( nwgData[[ k ]] ) ]

    f2UrwSDdata[[ k ]] <- f2UrwSDdata[[ k ]][ complete.cases( f2UrwSDdata[[ k ]] ), ]

    # compute each variable statistics in parallel
    stats <- autoLapply( cl, list( Fdata[[ k ]], Ldata[[ k ]], ageData[[ k ]],
                                   fData[[ k ]], nAdata[[ k ]], nApData[[ k ]],
                                   nDdata[[ k ]], nDdData[[ k ]], nDebData[[ k ]],
                                   nNWdata[[ k ]], nwData[[ k ]], nApgData[[ k ]],
                                   nDgData[[ k ]], nDdgData[[ k ]], nwgData[[ k ]] ),
                         comp_stats )

    N2   <- stats[[ 1 ]]
    L2   <- stats[[ 2 ]]
    age2 <- stats[[ 3 ]]
    f2   <- stats[[ 4 ]]
    A2   <- stats[[ 5 ]]
    A2p  <- stats[[ 6 ]]
    D2   <- stats[[ 7 ]]
    D2d  <- stats[[ 8 ]]
    Deb2 <- stats[[ 9 ]]
    NW2  <- stats[[ 10 ]]
    w2   <- stats[[ 11 ]]
    A2pg <- stats[[ 12 ]]
    D2g  <- stats[[ 13 ]]
    D2dg <- stats[[ 14 ]]
    w2g  <- stats[[ 15 ]]

    #
    # ------ Build statistics tables ------
    #

    key.stats.1 <- matrix( c( A2$avg, A2p$avg, D2$avg,
                              D2d$avg, A2pg$avg, D2g$avg,
                              D2dg$avg,

                              A2$sd / sqrt( nSize ), A2p$sd / sqrt( nSize ),
                              D2$sd / sqrt( nSize ), D2d$sd / sqrt( nSize ),
                              A2pg$sd / sqrt( nSize ), D2g$sd / sqrt( nSize ),
                              D2dg$sd / sqrt( nSize ),

                              A2$sd, A2p$sd, D2$sd,
                              D2d$sd, A2pg$sd, D2g$sd,
                              D2dg$sd,

                              A2$subbo$b, A2p$subbo$b, D2$subbo$b,
                              D2d$subbo$b, A2pg$subbo$b, D2g$subbo$b,
                              D2dg$subbo$b,

                              A2$subbo$a, A2p$subbo$a, D2$subbo$a,
                              D2d$subbo$a, A2pg$subbo$a, D2g$subbo$a,
                              D2dg$subbo$a,

                              A2$subbo$m, A2p$subbo$m, D2$subbo$m,
                              D2d$subbo$m, A2pg$subbo$m, D2g$subbo$m,
                              D2dg$subbo$m,

                              A2$jb$statistic, A2p$jb$statistic, D2$jb$statistic,
                              D2d$jb$statistic, A2pg$jb$statistic, D2g$jb$statistic,
                              D2dg$jb$statistic,

                              A2$jb$p.value, A2p$jb$p.value, D2$jb$p.value,
                              D2d$jb$p.value, A2pg$jb$p.value, D2g$jb$p.value,
                              D2dg$jb$p.value,

                              A2$ll$statistic, A2p$ll$statistic, D2$ll$statistic,
                              D2d$ll$statistic, A2pg$ll$statistic, D2g$ll$statistic,
                              D2dg$ll$statistic,

                              A2$ll$p.value, A2p$ll$p.value, D2$ll$p.value,
                              D2d$ll$p.value, A2pg$ll$p.value, D2g$ll$p.value,
                              D2dg$ll$p.value,

                              A2$ad$statistic, A2p$ad$statistic, D2$ad$statistic,
                              D2d$ad$statistic, A2pg$ad$statistic, D2g$ad$statistic,
                              D2dg$ad$statistic,

                              A2$ad$p.value, A2p$ad$p.value, D2$ad$p.value,
                              D2d$ad$p.value, A2pg$ad$p.value, D2g$ad$p.value,
                              D2dg$ad$p.value,

                              A2$ac$t1, A2p$ac$t1, D2$ac$t1,
                              D2d$ac$t1, A2pg$ac$t1, D2g$ac$t1,
                              D2dg$ac$t1,

                              A2$ac$t2, A2p$ac$t2, D2$ac$t2,
                              D2d$ac$t2, A2pg$ac$t2, D2g$ac$t2,
                              D2dg$ac$t2 ),

                           ncol = 7, byrow = TRUE )
    colnames( key.stats.1 ) <- c( "Prod.", "P.Prod.", "Demand", "D.Demand",
                                  "P.Prod.Gr.", "Demand Gr.", "D.Dem.Gr." )

    key.stats.2 <- matrix( c( N2$avg, f2$avg, Deb2$avg,
                              NW2$avg, L2$avg, w2$avg,
                              w2g$avg,

                              N2$sd / sqrt( nSize ), f2$sd / sqrt( nSize ),
                              Deb2$sd / sqrt( nSize ), NW2$sd / sqrt( nSize ),
                              L2$sd / sqrt( nSize ), w2$sd / sqrt( nSize ),
                              w2g$sd / sqrt( nSize ),

                              N2$sd, f2$sd, Deb2$sd,
                              NW2$sd, L2$sd, w2$sd,
                              w2g$sd,

                              N2$subbo$b, f2$subbo$b, Deb2$subbo$b,
                              NW2$subbo$b, L2$subbo$b, w2$subbo$b,
                              w2g$subbo$b,

                              N2$subbo$a, f2$subbo$a, Deb2$subbo$a,
                              NW2$subbo$a, L2$subbo$a, w2$subbo$a,
                              w2g$subbo$a,

                              N2$subbo$m, f2$subbo$m, Deb2$subbo$m,
                              NW2$subbo$m, L2$subbo$m, w2$subbo$m,
                              w2g$subbo$m,

                              N2$jb$statistic, f2$jb$statistic, Deb2$jb$statistic,
                              NW2$jb$statistic, L2$jb$statistic, w2$jb$statistic,
                              w2g$jb$statistic,

                              N2$jb$p.value, f2$jb$p.value, Deb2$jb$p.value,
                              NW2$jb$p.value, L2$jb$p.value, w2$jb$p.value,
                              w2g$jb$p.value,

                              N2$ll$statistic, f2$ll$statistic, Deb2$ll$statistic,
                              NW2$ll$statistic, L2$ll$statistic, w2$ll$statistic,
                              w2g$ll$statistic,

                              N2$ll$p.value, f2$ll$p.value, Deb2$ll$p.value,
                              NW2$ll$p.value, L2$ll$p.value, w2$ll$p.value,
                              w2g$ll$p.value,

                              N2$ad$statistic, f2$ad$statistic, Deb2$ad$statistic,
                              NW2$ad$statistic, L2$ad$statistic, w2$ad$statistic,
                              w2g$ad$statistic,

                              N2$ad$p.value, f2$ad$p.value, Deb2$ad$p.value,
                              NW2$ad$p.value, L2$ad$p.value, w2$ad$p.value,
                              w2g$ad$p.value,

                              N2$ac$t1, f2$ac$t1, Deb2$ac$t1,
                              NW2$ac$t1, L2$ac$t1, w2$ac$t1,
                              w2g$ac$t1,

                              N2$ac$t2, f2$ac$t2, Deb2$ac$t2,
                              NW2$ac$t2, L2$ac$t2, w2$ac$t2,
                              w2g$ac$t2 ),
                           ncol = 7, byrow = TRUE )
    colnames( key.stats.2 ) <- c( "N.Firms", "W.Share*", "Debt**", "N.Wealth**",
                                  "N.Workers*", "R.Wage**", "R.Wage Gr.**" )

    rownames( key.stats.1 ) <- rownames( key.stats.2 ) <-
      c( "average", " (s.e.)", " (s.d.)", "Subbotin b", " a", " m", "Jar.-Bera X2",
         " (p-val.)", "Lilliefors D", " (p-val.)", "And.-Darling A", " (p-val.)",
         "autocorr. t-1", "autocorr. t-2" )

    title.1 <- paste( "Pooled industry-level statistics (1) (", legends[ k ], ")" )
    title.2 <- paste( "Pooled industry-level statistics (2) (", legends[ k ], ")" )
    subTitle <- paste( paste0( "( Log-normalized values / Sample size = ", nElem,
                               " industries / Period = ", warmUp + 1, "-",
                               nTstat, " )" ),
                       paste( "(", sector, ")" ), sep ="\n" )

    textplot( formatC( key.stats.1, digits = sDigits, format = "g" ), cmar = 1.0 )
    title( main = title.1, sub = subTitle )
    textplot( formatC( key.stats.2, digits = sDigits, format = "g" ), cmar = 1.0 )
    title( main = title.2, sub = subTitle )

    saveCSV( key.stats.1, baseName = repName, num = k, baseFolder = folder,
             subFolder = outDir, suffix = datFilSfx, type = "pool1" )
    saveCSV( key.stats.2, baseName = repName, num = k, baseFolder = folder,
             subFolder = outDir, suffix = datFilSfx, type = "pool2" )

    #
    # ====== experiment-specific plots ======
    #

    # ------ Wallet share x HHI concentration spread ------

    plot( f2data[[ k ]], HH2data[[ k ]], pch = ".",
          xlab = "Pooled log wallet share",
          ylab = "Pooled Herfindahl-Hirschman concentration index" )
    abline( lm( HH2data[[ k ]] ~ f2data[[ k ]] ), col = light_color( "black" ) )
    title( main = paste( "Industry wallet share vs. concentration (", legends[ k ],
                         ")" ),
           sub = paste0( "( Consumer-good sector only / sample size = ", nElem,
                         " industries / Period = ", warmUp + 1, "-", nTstat,
                         " ", cntLeg,  " )" ) )
    legend( legend = c( "Data", "OLS fit" ),
            inset = 0.03, cex = 0.8, pch = c( ".", NA ), lty = c( NA, 1 ),
            col = c( "black", light_color( "black" ) ), x = "topright" )

    # ------ Unionized firm share distribution ------

    bins <- 100
    hData <- hist( f2Udata[[ k ]], breaks = bins, freq = FALSE, plot = FALSE )
    plot( hData$mids, hData$density, type = "h", log = "y", xlim = c( 0, 1 ),
          xlab = "Pooled industry market share of unionized firms",
          ylab = "Probability log density" )
    title( main = paste( "Unionized firm share distribution (", legends[ k ],
                         ")" ),
           sub = paste0( "( Consumer-good sector only / sample size = ", nElem,
                         " industries / Period = ", warmUp + 1, "-", nTstat,
                         " ", cntLeg, " / Bins = ", bins, " )" ) )

    # ------ Real wage distribution ------

    hData <- hist( nwData[[ k ]], breaks = 100, freq = FALSE, plot = FALSE )
    plot( hData$mids, hData$density, type = "h", log = "y", xlim = c( -1.5, 1.5 ),
          xlab = "Pooled industry log-normalized average real wage",
          ylab = "Probability log density" )
    title( main = paste( "Wage distribution (", legends[ k ],
                         ")" ),
           sub = paste0( "( Consumer-good sector only / sample size = ", nElem,
                         " industries / Period = ", warmUp + 1, "-", nTstat,
                         " ", cntLeg, " / Bins = ", bins, " )" ) )

    # ------ Unionization x real wage spread ------

    plot( f2Udata[[ k ]], nwData[[ k ]], pch = ".",
          xlab = "Pooled industry market share of unionized firms",
          ylab = "Pooled industry log-normalized average real wage" )
    lReg <- lm( nwData[[ k ]] ~ f2Udata[[ k ]] )
    if( is.finite( lReg$coefficients[ 2 ] ) ) {
      abline( lReg, col = light_color( "black" ) )
    }
    title( main = paste( "Unionization vs. wages (", legends[ k ],
                         ")" ),
           sub = paste0( "( Consumer-good sector only / sample size = ", nElem,
                         " industries / Period = ", warmUp + 1, "-", nTstat,
                         " ", cntLeg,  " )" ) )
    legend( legend = c( "Data", paste0( "OLS fit (β = ",
                                        round( lReg$coefficients[ 2 ], 2 ),
                                        ")" ) ),
            inset = 0.03, cex = 0.8, pch = c( ".", NA ), lty = c( NA, 1 ),
            col = c( "black", light_color( "black" ) ), x = "topleft" )

    # ------ Unionization x real wage standard-deviation spread ------

    plot( f2UrwSDdata[[ k ]], pch = ".",
          xlab = "Pooled industry market share of unionized firms",
          ylab = "Pooled industry log-normalized real wage standard deviation" )
    lReg <- lm( f2UrwSDdata[[ k ]][ , 1 ] ~ f2UrwSDdata[[ k ]][ , 2 ] )
    if( is.finite( lReg$coefficients[ 2 ] ) ) {
      abline( lReg, col = light_color( "black" ) )
    }
    title( main = paste( "Unionization vs. wage dispersion (", legends[ k ],
                         ")" ),
           sub = paste0( "( Consumer-good sector only / sample size = ", nElem,
                         " industries / Period = ", warmUp + 1, "-", nTstat,
                         " ", cntLeg,  " )" ) )
    legend( legend = c( "Data", paste0( "OLS fit (β = ",
                                        round( lReg$coefficients[ 2 ], 2 ),
                                        ")" ) ),
            inset = 0.03, cex = 0.8, pch = c( ".", NA ), lty = c( NA, 1 ),
            col = c( "black", light_color( "black" ) ), x = "topleft" )
  }

  #
  # ====== Plot distributions (overplots)  ======
  #

  subTitle <- paste0( "( Sample size = ", nElem, " industries / Period = ",
                      warmUp + 1, "-", nTstat, " ", cntLeg,  " )" )

  # ------ Age distribution (binned density x log-level variable)  ------

  plot_loglin( ageData, xlab = "Pooled number of periods from entry",
               ylab = "Binned density",
               tit = paste( "Industry age distribution", allLeg ),
               subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
               col = colors, lty = lTypes, pty = pTypes )


  # ------ Size, etc. distributions (binned density x log-level variable)  ------

  plot_norm( nAdata, xlab = "Pooled log-normalized effective productivity",
             ylab = "Binned density",
             tit = paste( "Industry effective productivity distribution", allLeg ),
             subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
             col = colors, lty = lTypes, pty = pTypes )

  plot_norm( nApData, xlab = "Pooled log-normalized potential productivity",
             ylab = "Binned density",
             tit = paste( "Industry potential productivity distribution", allLeg ),
             subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
             col = colors, lty = lTypes, pty = pTypes )

  plot_norm( nDdata, xlab = "Pooled log-normalized effective demand",
             ylab = "Binned density",
             tit = paste( "Industry effective demand distribution", allLeg ),
             subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
             col = colors, lty = lTypes, pty = pTypes )

  plot_norm( nDdData, xlab = "Pooled log-normalized desired demand",
             ylab = "Binned density",
             tit = paste( "Industry desired demand distribution", allLeg ),
             subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
             col = colors, lty = lTypes, pty = pTypes )

  plot_norm( Fdata, xlab = "Pooled number of firms",
             ylab = "Binned density",
             tit = paste( "Industry number of firms distribution", allLeg ),
             subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
             col = colors, lty = lTypes, pty = pTypes )

  plot_norm( fData, xlab = "Pooled wallet share",
             ylab = "Binned density",
             tit = paste( "Industry wallet share distribution", allLeg ),
             subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
             col = colors, lty = lTypes, pty = pTypes )

  plot_norm( f2Udata, xlab = "Pooled market share of unionized firms",
             ylab = "Binned density",
             tit = paste( "Industry unionization distribution", allLeg ),
             subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
             col = colors, lty = lTypes, pty = pTypes )

  plot_norm( nDebData, xlab = "Pooled log-normalized bank debt",
             ylab = "Binned density",
             tit = paste( "Industry debt distribution", allLeg ),
             subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
             col = colors, lty = lTypes, pty = pTypes )

  plot_norm( nNWdata, xlab = "Pooled log-normalized net wealth",
             ylab = "Binned density",
             tit = paste( "Industry net wealth (cash) distribution", allLeg ),
             subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
             col = colors, lty = lTypes, pty = pTypes )

  plot_norm( Ldata, xlab = "Pooled log-number of workers",
             ylab = "Binned density",
             tit = paste( "Industry number of workers distribution", allLeg ),
             subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
             col = colors, lty = lTypes, pty = pTypes )

  plot_norm( nwData, xlab = "Pooled log-normalized average real wage",
             ylab = "Binned density",
             tit = paste( "Industry real wages distribution", allLeg ),
             subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
             col = colors, lty = lTypes, pty = pTypes )

  # ------ Log-normal-fitted distributions ( log level x rank )  ------

  plot_lognorm( nDdata, xlab = "Pooled log-normalized effective demand",
                ylab = "Rank",
                tit = paste( "Industry effective demand distribution", allLeg ),
                subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
                col = colors, lty = lTypes )

  plot_lognorm( nDdData, xlab = "Pooled log-normalized desired demand",
                ylab = "Rank",
                tit = paste( "Industry desired demand distribution", allLeg ),
                subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
                col = colors, lty = lTypes )

  plot_lognorm( Fdata, xlab = "Pooled number of firms in industry",
                ylab =  "Rank",
                tit = paste( "Industry number of firms distribution", allLeg ),
                subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
                col = colors, lty = lTypes )

  plot_lognorm( nDebData, xlab = "Pooled log-normalized bank debt",
                ylab = "Rank",
                tit = paste( "Industry debt distribution", allLeg ),
                subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
                col = colors, lty = lTypes )

  plot_lognorm( Ldata, xlab = "Pooled log-number of workers",
                ylab = "Rank",
                tit = paste( "Industry number of workers distribution", allLeg ),
                subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
                col = colors, lty = lTypes )

  plot_lognorm( nNWdata, xlab = "Pooled log-normalized net wealth",
                ylab = "Rank",
                tit = paste( "Industry net wealth (cash) distribution", allLeg ),
                subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
                col = colors, lty = lTypes )

  # ------ Laplace growth distributions ( binned dens. x log gr. rate )  ------

  plot_laplace( nApgData, xlab = "Pooled normalized potential productivity growth rate",
                ylab = "Binned density",
                tit = paste( "Industry potential productivity growth rate distribution",
                             allLeg ),
                subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
                col = colors, lty = lTypes, pty = pTypes )

  plot_laplace( nDgData, xlab = "Pooled normalized effective demand growth rate",
                ylab = "Binned density",
                tit = paste( "Industry effective demand growth rate distribution",
                             allLeg ),
                subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
                col = colors, lty = lTypes, pty = pTypes )

  plot_laplace( nDdgData, xlab = "Pooled normalized desired demand growth rate",
                ylab = "Binned density",
                tit = paste( "Industry desired demand growth rate distribution",
                             allLeg ),
                subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
                col = colors, lty = lTypes, pty = pTypes )

  plot_laplace( nwgData, xlab = "Pooled normalized average real wage growth rate",
                ylab = "Binned density",
                tit = paste( "Industry average wage growth distribution", allLeg ),
                subtit = subTitle, outLim = outLim, bins = nBins, leg = legends,
                col = colors, lty = lTypes, pty = pTypes )

  rm( Fdata, Ldata, ageData, fData, nAdata, nApData, nDdata, nDdData, nDebData,
      nNWdata, nwData, nApgData, nDgData, nDdgData, nwgData, HH2data, f2data )

  #
  # ===== Monte Carlo analysis =====
  #

  cat( "\nGenerating MC reports...\n" )

  nElem <- rep( 0, nExp )

  for( k in 1 : nExp ) {             # do for each experiment

    cat( "", expLeg, k, "of", nExp, "\n" )

    # ------ compute single MC statistics ------

    # compute each MC case in parallel
    stats <- autoLapply( cl, 1 : nSize, statsMC, k, nSize, files[[ k ]]$mc,
                         lags, lowP, highP, bpfK )

    # reorganize data
    ccA2 <- ccB1 <- ccD1 <- ccD2 <- ccL1 <- ccL2 <- ccg1 <-
      matrix( nrow = 0, ncol = 2 * lags + 1 )
    KLbas <- KLlux <- fBas <- fLux <- Dcurve <- Lcurve <- list( )
    indType <- indStart <- vector( )

    for( l in 1 : nSize ) {          # for each MC run
      nElem[ k ]  <- nElem[ k ] + stats[[ l ]]$nElemMC
      indType     <- c( indType, stats[[ l ]]$indTypeMC )
      indStart    <- c( indStart, stats[[ l ]]$indStartMC )
      Dcurve      <- append( Dcurve, stats[[ l ]]$DcurveMC )
      Lcurve      <- append( Lcurve, stats[[ l ]]$LcurveMC )
      KLbas       <- append( KLbas, stats[[ l ]]$KLbasMC )
      KLlux       <- append( KLlux, stats[[ l ]]$KLluxMC )
      fBas        <- append( fBas, stats[[ l ]]$fBasMC )
      fLux        <- append( fLux, stats[[ l ]]$fLuxMC )
      ccA2        <- rbind( ccA2, stats[[ l ]]$ccA2mc )
      ccB1        <- rbind( ccB1, stats[[ l ]]$ccB1mc )
      ccD1        <- rbind( ccD1, stats[[ l ]]$ccD1mc )
      ccD2        <- rbind( ccD2, stats[[ l ]]$ccD2mc )
      ccL1        <- rbind( ccL1, stats[[ l ]]$ccL1mc )
      ccL2        <- rbind( ccL2, stats[[ l ]]$ccL2mc )
      ccg1        <- rbind( ccg1, stats[[ l ]]$ccg1mc )
    }

    # ------ Plot relative-time series ------

    maxLenBas <- maxLenLux <- 0
    for( i in 1 : length( KLbas ) )
      if( length( KLbas[[ i ]] ) > maxLenBas )
        maxLenBas <- length( KLbas[[ i ]] )
    for( i in 1 : length( KLlux ) )
      if( length( KLlux[[ i ]] ) > maxLenLux )
        maxLenLux <- length( KLlux[[ i ]] )

    KLbasMC2 <- matrix( nrow = length( KLbas ), ncol = maxLenBas )
    KLluxMC2 <- matrix( nrow = length( KLlux ), ncol = maxLenLux )
    for( i in 1 : length( KLbas ) ) {
      KLratio <- KLbas[[ i ]]
      length( KLratio ) <- maxLenBas
      KLbasMC2[ i, ] <- KLratio
    }
    for( i in 1 : length( KLlux ) ) {
      KLratio <- KLlux[[ i ]]
      length( KLratio ) <- maxLenLux
      KLluxMC2[ i, ] <- KLratio
    }

    KLbasAvg <- colMeans( KLbasMC2, na.rm = TRUE )
    KLluxAvg <- colMeans( KLluxMC2, na.rm = TRUE )

    maxLenBas <- maxLenLux <- 0
    for( i in 1 : length( fBas ) )
      if( length( fBas[[ i ]] ) > maxLenBas )
        maxLenBas <- length( fBas[[ i ]] )
    for( i in 1 : length( fLux ) )
      if( length( fLux[[ i ]] ) > maxLenLux )
        maxLenLux <- length( fLux[[ i ]] )

    fBasMC2 <- matrix( nrow = length( fBas ), ncol = maxLenBas )
    fLuxMC2 <- matrix( nrow = length( fLux ), ncol = maxLenLux )
    for( i in 1 : length( fBas ) ) {
      f2 <- fBas[[ i ]]
      length( f2 ) <- maxLenBas
      fBasMC2[ i, ] <- f2
    }
    for( i in 1 : length( fLux ) ) {
      f2 <- fLux[[ i ]]
      length( f2 ) <- maxLenLux
      fLuxMC2[ i, ] <- f2
    }

    fBasMC2[ is.na( fBasMC2 ) ] <- 0
    fLuxMC2[ is.na( fLuxMC2 ) ] <- 0
    fBasAvg <- colMeans( fBasMC2 )
    fLuxAvg <- colMeans( fLuxMC2 )

    plot( KLbasAvg[ 1 : relTmax ], ylab = "Average log(capital / labor) ratio",
          xlab = "Relative time" )
    points( KLluxAvg[ 1 : relTmax ], pch = "x", col = "blue" )
    lines( predict( loess( KLbasAvg[ 1 : relTmax ] ~ c( 1 : relTmax ) ) ),
           col = light_color( "black" ) )
    lines( predict( loess( KLluxAvg[ 1 : relTmax ] ~ c( 1 : relTmax ) ) ),
           col = light_color( "blue" ) )
    title <- paste( "Average industry capital-to-labor ratio (", legends[ k ], ")" )
    subTitle <- paste( eval( bquote( paste0( "( relative time cut-off at ", relTmax,
                                             " / sample size = ", nElem[ k ],
                                             " / MC runs = ", nSize,
                                             " / period = ", warmUp + 1, "-",
                                             nTstat, " )" ) ) ) )
    title( main = title, sub = subTitle )
    legend( legend = c( "Basic industries", "Luxury industries",
                        "Local regression fit" ), inset = 0.03, cex = 0.8,
            pch = c( "o", "x", NA ), lty = c( NA, NA, 1 ),
            col = c( "black", "blue", light_color( "black" ) ), x = "bottomright" )

    plot( fBasAvg[ 1 : relTmax ], ylab = "Average wallet share",
          xlab = "Relative time" )
    points( fLuxAvg[ 1 : relTmax ], pch = "x", col = "blue" )
    lines( predict( loess( fBasAvg[ 1 : relTmax ] ~ c( 1 : relTmax ) ) ),
           col = light_color( "black" ) )
    lines( predict( loess( fLuxAvg[ 1 : relTmax ] ~ c( 1 : relTmax ) ) ),
           col = light_color( "blue" ) )
    title <- paste( "Industry average wallet share (", legends[ k ], ")" )
    subTitle <- paste0( "( relative time cut-off at ", relTmax,
                        " / sample size = ", nElem[ k ], " / MC runs = ", nSize,
                        " / period = ", warmUp + 1, "-", nTstat, " ", cntLeg, " )" )
    title( main = title, sub = subTitle )
    legend( legend = c( "Basic industries", "Luxury industries",
                        "Local regression fit" ), inset = 0.03, cex = 0.8,
            pch = c( "o", "x", NA ), lty = c( NA, NA, 1 ),
            col = c( "black", "blue", light_color( "black" ) ), x = "topright" )

    # ------ Build statistics table ------

    cc.avg <- rbind( colMeans( ccL1, na.rm = TRUE ),
                     colSds( ccL1, na.rm = TRUE ) / sqrt( length( ccL1 ) ),
                     colMeans( ccL2, na.rm = TRUE ),
                     colSds( ccL2, na.rm = TRUE ) / sqrt( length( ccL1 ) ),
                     colMeans( ccD1, na.rm = TRUE ),
                     colSds( ccD1, na.rm = TRUE ) / sqrt( length( ccL1 ) ),
                     colMeans( ccD2, na.rm = TRUE ),
                     colSds( ccD2, na.rm = TRUE ) / sqrt( length( ccL1 ) ),
                     colMeans( ccB1, na.rm = TRUE ),
                     colSds( ccB1, na.rm = TRUE ) / sqrt( length( ccL1 ) ),
                     colMeans( ccA2, na.rm = TRUE ),
                     colSds( ccA2, na.rm = TRUE ) / sqrt( length( ccL1 ) ),
                     colMeans( ccg1, na.rm = TRUE ),
                     colSds( ccg1, na.rm = TRUE ) / sqrt( length( ccL1 ) ) )

    colnames( cc.avg ) <- c( -lags : -1, 0, 1 : lags )
    rownames( cc.avg ) <- c( "Labor demand (s.1)", " (s.e.)", "Labor demand (s.2)",
                             " (s.e.)", "Machine demand (s.1)", " (s.e.)",
                             "Goods demand (s.2)", " (s.e.)", "Productivity (s.1)",
                             " (s.e.)", "Productivity (s.2)", " (s.e.)",
                             "Machine technology (s.2)", " (s.e.)" )

    textplot( formatC( cc.avg, digits = sDigits, format = "g" ), cmar = 1.0 )
    title <- paste( "Correlation structure to labor demand (", legends[ k ], ")" )
    subTitle <- paste0( "( MC standard errors / sample size = ", nElem[ k ],
                        " industries / MC runs = ", nSize, " / period = ",
                        warmUp + 1, "-", nTstat, " ", cntLeg, " )" )
    title( main = title, sub = subTitle )

    saveCSV( cc.avg, baseName = repName, num = k, baseFolder = folder,
             subFolder = outDir, suffix = datFilSfx, type = "cor_str" )

    #
    # ------ Industry growth analysis ------
    #

    steps <- nTstat - warmUp
    Dbas <- Dlux <- Lbas <- Llux <- matrix( nrow = steps, ncol = 0 )
    rownames( Dbas ) <- rownames( Dlux ) <- rownames( Lbas ) <-
      rownames( Llux ) <- 1 : steps

    # collect data from industries
    for( i in 1 : length( indType ) ) {  # scan all industries

      # ignore industries out of analysis time scope
      if( indStart[ i ] <= warmUp || indStart[ i ] > nTstat )
        next

      # save basic and luxury industries separately
      if( indType[ i ] == 0 ) {
        Dbas <- cbind( Dbas, c( Dcurve[[ i ]],
                                rep( NA, steps - length( Dcurve[[ i ]] ) ) ) )
        Lbas <- cbind( Lbas, c( Lcurve[[ i ]],
                                rep( NA, steps - length( Lcurve[[ i ]] ) ) ) )
      } else {
        Dlux <- cbind( Dlux, c( Dcurve[[ i ]],
                                rep( NA, steps - length( Dcurve[[ i ]] ) ) ) )
        Llux <- cbind( Llux, c( Lcurve[[ i ]],
                                rep( NA, steps - length( Lcurve[[ i ]] ) ) ) )
      }
    }

    # consolidate data from selected industries
    Dbas.avg <- rowSums( Dbas, na.rm = TRUE ) / ncol( Dbas )
    Dlux.avg <- rowSums( Dlux, na.rm = TRUE ) / ncol( Dlux )
    Lbas.avg <- rowSums( Lbas, na.rm = TRUE ) / ncol( Lbas )
    Llux.avg <- rowSums( Llux, na.rm = TRUE ) / ncol( Llux )


    Dbas.inc <- rem_dec( Dbas.avg )
    Dlux.inc <- rem_dec( Dlux.avg )
    Lbas.inc <- rem_dec( Lbas.avg )
    Llux.inc <- rem_dec( Llux.avg )

    Dbas.fit <- compertz_model( Dbas.inc )
    Dlux.fit <- compertz_model( Dlux.inc )
    Lbas.fit <- compertz_model( Lbas.inc )
    Llux.fit <- compertz_model( Llux.inc )

    # create fitting results table
    comp.fit <- matrix( c( Dbas.fit$beta, Dlux.fit$beta,
                           Lbas.fit$beta, Llux.fit$beta,

                           Dbas.fit$betaSE, Dlux.fit$betaSE,
                           Lbas.fit$betaSE, Llux.fit$betaSE,

                           Dbas.fit$betaPr, Dlux.fit$betaPr,
                           Lbas.fit$betaPr, Llux.fit$betaPr,

                           Dbas.fit$gamma, Dlux.fit$gamma,
                           Lbas.fit$gamma, Llux.fit$gamma,

                           Dbas.fit$gammaSE, Dlux.fit$gammaSE,
                           Lbas.fit$gammaSE, Llux.fit$gammaSE,

                           Dbas.fit$gammaPr, Dlux.fit$gammaPr,
                           Lbas.fit$gammaPr, Llux.fit$gammaPr,

                           Dbas.fit$corr, Dlux.fit$corr,
                           Lbas.fit$corr, Llux.fit$corr,

                           Dbas.fit$R2, Dlux.fit$R2,
                           Lbas.fit$R2, Llux.fit$R2,

                           length( Dbas.avg ), length( Dlux.avg ),
                           length( Lbas.avg ), length( Llux.avg ),

                           ncol( Dbas ), ncol( Dlux ),
                           ncol( Lbas ), ncol( Llux ) ),

                        ncol = 4, byrow = TRUE )
    colnames( comp.fit ) <- c( "Demand Bas.", "Demand Lux.", "Labor Bas.",
                               "Labor Lux." )

    rownames( comp.fit ) <- c( "beta", " (s.e.)", " (p-val.)", "gamma",
                               " (s.e.)", " (p-val.)", "Pred.Corr.", "R2",
                               "Periods", "Industries" )

    title <- paste( "Compertz growth model fitting (", legends[ k ], ")" )
    subTitle <- paste( paste0( "( Model: log(dlx) ~ -gamma * t + log(beta * exp(gamma) - beta) / Period = ",
                               warmUp + 1, "-", nTstat, " )" ),
                       paste( "(", sector, ")" ), sep ="\n" )

    textplot( formatC( comp.fit, digits = sDigits, format = "g" ), cmar = 1.0 )
    title( main = title, sub = subTitle )

    saveCSV( comp.fit, baseName = repName, num = k, baseFolder = folder,
             subFolder = outDir, suffix = datFilSfx, type = "growth" )

    #
    # ====== Case-specific plots ======
    #

    plot( Dbas.avg[ 1 : relTmax ], xlab = "Relative time",
          ylab = "Pooled average log-normalized effective demand" )
    lines( predict( loess( Dbas.avg[ 1 : relTmax ] ~ c( 1 : relTmax ) ) ),
           col = light_color( "black" ) )
    title <- paste( "Average basic industry demand growth curve (",
                    legends[ k ], ")" )
    subTitle <- paste( "( Relative time cut-off at", relTmax, "/ samples =",
                       ncol( Dbas ), "/ basic consumer-good industries only",
                       cntLeg, ")" )
    title( main = title, sub = subTitle )
    legend( legend = c( "Pooled average data", "Local regression fit" ),
            inset = 0.03, cex = 0.8, pch = c( "o", NA ), lty = c( NA, 1 ),
            col = c( "black", light_color( "black" ) ), x = "topright" )

    plot( Dlux.avg[ 1 : relTmax ], xlab = "Relative time",
          ylab = "Pooled average log-normalized effective demand" )
    lines( predict( loess( Dlux.avg[ 1 : relTmax ] ~ c( 1 : relTmax ) ) ),
           col = light_color( "black" ) )
    title <- paste( "Average luxury industry demand growth curve (",
                    legends[ k ], ")" )
    subTitle <- paste( "( Local regression fit / samples =", ncol( Dlux ),
                       "/ luxury consumer-good industries only", cntLeg, ")" )
    title( main = title, sub = subTitle )
    legend( legend = c( "Pooled average data", "Local regression fit" ),
            inset = 0.03, cex = 0.8, pch = c( "o", NA ), lty = c( NA, 1 ),
            col = c( "black", light_color( "black" ) ), x = "topright" )

    plot( Lbas.avg[ 1 : relTmax ], xlab = "Relative time",
          ylab = "Pooled average number of workers" )
    lines( predict( loess( Lbas.avg[ 1 : relTmax ] ~ c( 1 : relTmax ) ) ),
           col = light_color( "black" ) )
    title <- paste( "Average basic industry labor growth curve (",
                    legends[ k ], ")" )
    subTitle <- paste( "( Local regression fit / samples =", ncol( Dbas ),
                       "/ basic consumer-good industries only", cntLeg, ")" )
    title( main = title, sub = subTitle )
    legend( legend = c( "Pooled average data", "Local regression fit" ),
            inset = 0.03, cex = 0.8, pch = c( "o", NA ), lty = c( NA, 1 ),
            col = c( "black", light_color( "black" ) ), x = "topright" )

    plot( Llux.avg[ 1 : relTmax ], xlab = "Relative time",
          ylab = "Pooled average number of workers" )
    lines( predict( loess( Llux.avg[ 1 : relTmax ] ~ c( 1 : relTmax ) ) ),
           col = light_color( "black" ) )
    title <- paste( "Average luxury industry labor growth curve (",
                    legends[ k ], ")" )
    subTitle <- paste( "( Local regression fit / samples =", ncol( Dlux ),
                       "/ luxury consumer-good industries only", cntLeg, ")" )
    title( main = title, sub = subTitle )
    legend( legend = c( "Pooled average data", "Local regression fit" ),
            inset = 0.03, cex = 0.8, pch = c( "o", NA ), lty = c( NA, 1 ),
            col = c( "black", light_color( "black" ) ), x = "topright" )
  }


  stopCores( cl )

  clearTemp( savDat = savDat, files = files, nExp = nExp,
             folder = folder, baseName = baseName, datFilSfx = datFilSfx )


  # ------ End of report ------

}, interrupt = catchError, error = catchError, finally = showEndMark( ) )
