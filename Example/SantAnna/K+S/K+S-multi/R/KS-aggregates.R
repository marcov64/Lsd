#******************************************************************
#
# ----------------- K+S Aggregates analysis ---------------------
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
dComp     <- FALSE                  # compare experiments to data (baseName0.csv)
iniDrop   <- 0                      # initial time steps to drop (0=none)
nKeep     <- -1                     # number of time steps to keep (-1=all)
coresExp  <- 0                      # max cores for experiments (0=all)
coresMC   <- 0                      # max cores for Monte Carlo (0=all)
savDat    <- FALSE                  # save data files and re-use if available?
mcStat    <- "median"               # Monte Carlo statistic ("mean", "median")
mcDist    <- ""                     # distance metric to find MC typical run
CI        <- 0.95                   # confidence level
bootR     <- 999                    # bootstrap replicates (bootCI != NULL)
bootCI    <- NULL                   # bootstrap confidence interval method (SLOW)
                                    # (NULL (no bootstrap), "basic", or "bca")

# caption and file names
expVal    <- c( "Only unionized firms", "Mixed firms" )   # experiment captions
cntVal    <- c( "Country 1", "Country 2" )                # country captions
dcVal     <- "Empirical data"                             # comparison data
firmTypes <- c( "Pre-change firms", "Post-change firms" ) # firm-type captions
caption   <- "K+S aggregate analysis"                     # log caption
datFilSfx <- "aggr"                                       # data file suffix

# aggregated variables to import and add
logVar    <- c( "A", "AcBas", "AcLux", "AcPreChg", "AcPosChg", "BadDeb", "BonC",
                "C", "CdBas", "CdLux", "Creal", "Dc", "DcBas", "DcLux", "Deb",
                "Deb1", "DebC", "DebF", "Def", "EI", "Gbail", "Gcons", "Gtrain",
                "Gtrf", "GDPnom", "GDPreal", "I", "Inom", "In", "Ireal", "KcBas",
                "KcLux", "Loans", "NW", "NW1", "NWc", "S", "SavLux", "TC", "Tax",
                "VAcW", "W", "Wc", "dNnom", "kCavgBas", "kCavgLux", "wAvgReal",
                "wCrealPosChg", "wCrealPreChg" )
nlogVar   <- c( "AsdCpreChg", "AsdCposChg", "Bda", "Bfail", "CPI", "DebGDP",
                "DefGDP", "EXc", "EXk", "F1", "Fc", "FcBas", "FcLux", "FcFbas",
                "FcFlux", "Gini", "HH1", "HHc", "HP1", "HPc", "LcBas", "LcLux",
                "Lent", "Lexit", "Lpart", "Ls", "PPI", "QcU", "TeAvg", "U", "Ue",
                "V", "dA", "dCPI", "dGDPreal", "dw", "entry1","entryC", "exit1",
                "exitC", "fCposChg", "g1front", "g1max", "imi1", "inn1i", "inn1r",
                "muCavg", "muCavgBas", "muCavgLux", "qCavg", "qCposChg",
                "qCpreChg", "r", "sTavg", "sVavg", "nCavg", "nCavgBas",
                "nCavgLux", "wGini", "wLogSD" )
origVar   <- c( logVar, nlogVar )
addVar    <- c( )


# ==== Libraries, support functions and global options ====

source( "KS-support-functions.R" )
source( "KS-time-plots.R" )
source( "KS-box-plots.R" )
source( "KS-crisis-recover.R" )

options( warn = 0 )         # -1=no warning/0:warnings at end/2:warnings stop

# remove RStudio warnings for support functions
# !diagnostics suppress = showStartMark, showEndMark, setCmdLinePars, loadData
# !diagnostics suppress = catchError, startTime, clearTemp, setLabels, adrop
# !diagnostics suppress = log0, logNA, abind, plot_lin, plot_bpf, plot_histo
# !diagnostics suppress = textplot, corr_table, corr_struct, autocorr_table
# !diagnostics suppress = spectrum_table, growth_stats, time_plots, box_plots
# !diagnostics suppress = crisis_recover, ergod.test.lsd, repFile, readCSV
# !diagnostics suppress = saveCSV, zoo, t.test0, rollmedian, clearTemp
# !diagnostics suppress = setLabels, outDir, mc, mcP, mcX, pool, P, X, S, C, c
# !diagnostics suppress = M, m, n, nTsteps, nVar, nTsteps, nSize, legends,
# !diagnostics suppress = plotLegends, expLeg, listLeg, cntLeg, allLeg


# ==== Process LSD result files ====

showStartMark( caption )    # log start mark

setCmdLinePars( )           # read command line parameters (if any)

# read LSD files or load existing temporary files according to the case
files <- loadData( savDat = savDat, nExp = nExp, mCnt = mCnt, nCnt = nCnt,
                   folder = folder, baseName = baseName, iniDrop = iniDrop,
                   nKeep = nKeep, origVar = origVar, addVar = addVar,
                   datFilSfx = datFilSfx, coresExp = coresExp,
                   coresMC = coresMC, mcStat = mcStat, mcDist = mcDist,
                   CI = CI, bootR = bootR, bootCI = bootCI )


#******************************************************************
#
# --------------------- Plot statistics -------------------------
#
#******************************************************************

# ===================== User parameters =========================

bCase     <- 1      # experiment to be used as base case
nBins     <- 15     # number of bins to use in histograms
warmUpPlot<- 100    # number of "warm-up" runs for plots
nTplot    <- -1     # last period to consider for plots (-1=all)
warmUpStat<- 200    # warm-up runs to evaluate all statistics
nTstat    <- 400    # last period to consider for statistics (-1=all)
lowP      <- 6      # bandpass filter minimum period
highP     <- 32     # bandpass filter maximum period
bpfK      <- 12     # bandpass filter order
lags      <- 4      # lags to analyze (short term) (must be even)
lagsLT    <- 20     # lags to analyze (long term)
bPlotCoef <- 1.5    # boxplot whiskers extension from the box (0=extremes)
bPlotNotc <- FALSE  # use boxplot notches
smoothing <- 1e5    # HP filter smoothing factor (lambda)
radarStat <- 2      # radar plot statistic: 1=mean, 2=median, 3=sd, 4=min, 5=max
radarZoom <- 1.1    # zoom factor to use in radar plots
crisisTh  <- -0.03  # crisis growth threshold
crisisLen <- 3      # crisis minimum duration (periods)
crisisPre <- 4      # pre-crisis period to base trend start (>=1)
crisisRun <- 0      # the crisis case to be plotted (0=auto)

repName   <- ""     # report files base name (if "" same baseName)
transMk   <- -1     # regime transition mark after warm-up (-1:none)
sDigits   <- 4      # significant digits in tables
plotRows  <- 1      # number of plots per row in a page
plotCols  <- 1  	  # number of plots per column in a page
plotW     <- 10     # plot window width
plotH     <- 7      # plot window height
raster    <- FALSE  # raster (TRUE) or vector (FALSE) plots
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


# ==== Support stuff ====

# organize data read from temporary files
mcData <- mcPtag <- mcXtag <- Pdata <- Xdata <- Sdata <- Cdata <- cdata <-
  Mdata <- mdata <- list()
nTsteps.1 <- nVar.1 <- nSize.1 <- 0

for( k in 1 : nExp ) {                      # realocate data in separate lists

  load( files[[ k ]]$pool )                 # pooled data

  if( k > 1 && ( nTsteps != nTsteps.1 || nVar != nVar.1 || nSize != nSize.1 ) )
    stop( "Inconsistent data files.\nSame number of time steps, variables and of MC runs is required." )

  Pdata[[ k ]] <- P
  Sdata[[ k ]] <- S
  Cdata[[ k ]] <- C
  cdata[[ k ]] <- c
  Mdata[[ k ]] <- M
  mdata[[ k ]] <- m
  nTsteps.1 <- nTsteps
  nVar.1 <- nVar
  nSize.1 <- nSize

  if( ! is.null( mcDist ) && mcDist != "" ) {   # use typical runs?
    mcPtag[[ k ]] <- mcP
    mcXtag[[ k ]] <- mcX
    Xdata[[ k ]] <- X
  } else
    Xdata[[ k ]] <- P

  mcData[[ k ]] <- array( dim = c( nTsteps, nVar, nSize ),
                          dimnames = list( dimnames( pool )[[ 1 ]],
                                           dimnames( pool )[[ 2 ]],
                                           1 : nSize ) )

  for( j in 1 : nSize ) {                    # for each MC case

    # load MC data from temporary files and remove instance dimension (always 1)
    load( files[[ k ]]$mc[ j ] )
    mc <- adrop( mc[ , , 1, drop = FALSE ], drop = 3 )
    mcData[[ k ]][ , , j ] <- mc
  }
}

rm( pool, mc, mcP, mcX, P, X, S, C, c, M, m, n )

# read data comparison file
if( ! mCnt && dComp ) {
  DCdata <- readCSV( folder, baseName, "0", iniDrop, nTsteps )
} else {
  dComp <- FALSE
  DCdata <- NULL
}

# generate labels & build labels list legend
setLabels( nExp, mCnt, nCnt, dComp )

# create tags for MC-specific plots
if( ! is.null( mcDist ) && mcDist != "" ) {   # use typical runs?
  Ptag <- c( )
  for( i in 1 : length( mcPtag[[ 1 ]] ) ) {
    Ptag[ i ] <- mcPtag[[ 1 ]][ i ]
    if( length( mcPtag ) > 1 )
      for( j in 2 : length( mcPtag ) )
        Ptag[ i ] <- paste( Ptag[ i ], mcPtag[[ j ]][ i ], sep = "|" )
  }
  names( Ptag ) <- names( mcPtag[[ 1 ]] )
  Xtag <- unlist( mcXtag )
  fileTag <- paste0( "_", ifelse( length( mcXtag ) > 1, "multi-mc", mcXtag[[ 1 ]] ) )
} else {
  Ptag <- rep( mcStat, nVar )
  names( Ptag ) <- dimnames( mcData[[ 1 ]] )[[ 2 ]]
  Xtag <- rep( mcStat, nExp )
  fileTag <- ""
}

# number of periods to show in graphics and use in statistics
if( nTplot < 1 || nTplot > nTsteps || nTplot <= warmUpPlot )
  nTplot <- nTsteps
if( nTstat < 1 || nTstat > nTsteps || nTstat <= warmUpStat )
  nTstat <- nTsteps
if( nTstat < ( warmUpStat + 2 * bpfK + 4 ) )
  nTstat <- warmUpStat + 2 * bpfK + 4         # minimum number of periods
TmaxStat <- nTstat - warmUpStat
TmaskPlot <- ( warmUpPlot + 1 ) : nTplot
TmaskStat <- ( warmUpStat + 1 ) : nTstat
TmaskBpf <- ( bpfK + 1 ) : ( TmaxStat - bpfK )


# ==== Main code ====

tryCatch( {   # enter error handling mode so PDF can be closed in case of error

  #
  # ==== Crisis-recovery analysis ====
  #

  # close main MC report and open new one
  repFile( repName, folder = folder, width = plotW, height = plotH,
           suffix = paste0( datFilSfx, "_", mcStat, fileTag, "_crisis" ),
           rows = plotRows, cols = plotCols, raster = raster, res = res )

  cat( "\nGenerating crisis-recovery report...\n" )

  rec.stats <- crisis_recover( mcData, nExp, nSize, TmaskStat, warmUpPlot,
                               warmUpStat, nTstat, crisisLen, crisisPre,
                               crisisTh, crisisRun, legends, expLeg, cntLeg,
                               allLeg, sDigits, smoothing, transMk, mCnt,
                               folder, outDir, repName, datFilSfx )


  #
  # ====== Pooled analysis ======
  #

  # create the report file(s) in a daily output directory
  repFile( repName, folder = folder, width = plotW, height = plotH,
           suffix = paste0( datFilSfx, "_", mcStat, fileTag, "_pool" ),
           rows = plotRows, cols = plotCols, raster = raster, res = res )

  cat( "\nGenerating pooled reports...\n" )

  #
  # ------ time plots ------
  #

  time_plots( mcData, Pdata, Xdata, mdata, Mdata, Sdata, cdata, Cdata, DCdata,
              mcStat, nExp, nSize, nTsteps, mCnt, TmaskPlot, CI, Ptag, Xtag,
              plotLegends, cntLeg, colors, lTypes, smoothing, transMk, firmTypes )

  #
  # ------ comparison of experiments ------
  #

  box_plots( mcData, rec.stats, mcStat, nExp, nSize, mCnt, TmaxStat, TmaskStat,
             warmUpStat, nTstat, radarStat, radarZoom, legends, listLeg, cntLeg,
             allLeg, colors, lTypes, sDigits, bPlotCoef, bPlotNotc, folder,
             outDir, repName, datFilSfx )


  #
  # ====== Case-specific analysis ======
  #

  cat( "\nGenerating case-specific reports...\n" )

  # close previous report and open new one
  repFile( repName, folder = folder, width = plotW, height = plotH,
           suffix = paste0( datFilSfx, "_", mcStat, fileTag, "_cases"),
           rows = plotRows, cols = plotCols, raster = raster, res = res )

  for( k in 1 : nExp ) {

    cat( "", expLeg, k, "of", nExp, "\n" )

    #
    # ---- experiment's distribution plots ----
    #

    # cross-section times selection
    csT <- c( round( ( warmUpPlot + nTplot + 1 ) / 2 ), nTplot )
    subtit <- paste( "( mean at dotted line / cross sections at (",
                     paste( csT, collapse = ", " ), ") / MC runs =",
                     nSize, cntLeg, ")" )

    plot_histo( csT, mcData[[ k ]][ , "GDPreal", ], log = 3, bins = nBins,
                tit = paste( "GDP distribution (",
                             legends[ k ], ")" ),
                subtit = subtit,
                labVar = "Log real gross domestic product",
                leg = paste( csT ) )

    plot_histo( csT, mcData[[ k ]][ , "A", ], log = 1, bins = nBins,
                tit = paste( "Productivity distribution (",
                             legends[ k ], ")" ),
                subtit = subtit,
                labVar = "Relative log labor productivity",
                leg = paste( csT ) )

    plot_histo( csT, mcData[[ k ]][ , "wAvgReal", ], log = 1, bins = nBins,
                tit = paste( "Real wage distribution (",
                             legends[ k ], ")" ),
                subtit = subtit,
                labVar = "Log real wage",
                leg = paste( csT ) )

    plot_histo( csT, mcData[[ k ]][ , "wGini", ], bins = nBins,
                tit = paste( "Worker inequality (",
                             legends[ k ], ")" ),
                subtit = subtit,
                labVar = "Gini index for worker income",
                leg = paste( csT ) )

    plot_histo( csT, mcData[[ k ]][ , "DebGDP", ], bins = nBins,
                tit = paste( "Government debt distribution (",
                             legends[ k ], ")" ),
                subtit = subtit,
                labVar = "Government debt over GDP",
                leg = paste( csT ) )

    #
    # ---- bandpass-filtered series plots ----
    #

    bpfMsg <- paste0( "Baxter-King bandpass-filtered series, low =", lowP,
                      "Q / high = ", highP, "Q / order = ", bpfK )
    subtit <- paste0( "( ", bpfMsg, " / period = ", warmUpStat + 1, "-", nTstat,
                      " / MC runs = ", nSize, " / MC ", Xtag[ k ], cntLeg, ")" )

    plot_bpf( list( log0( Xdata[[ k ]]$GDPreal ), log0( Xdata[[ k ]]$Creal ),
                    log0( Xdata[[ k ]]$Ireal ), log0( Xdata[[ k ]]$A ) ),
              pl = lowP, pu = highP, nfix = bpfK, mask = TmaskPlot,
              mrk = transMk, col = colors, lty = lTypes,
              leg = c( "GDP", "Consumption", "Investment", "Productivity" ),
              xlab = "Time", ylab = "Filtered series",
              tit = paste( "GDP cycles (", legends[ k ], ")" ),
              subtit = subtit )

    plot_bpf( list( Xdata[[ k ]]$U, Xdata[[ k ]]$V ),
              pl = lowP, pu = highP, nfix = bpfK, mask = TmaskPlot,
              mrk = transMk, col = colors, lty = lTypes,
              leg = c( "Productivity", "Unemployment", "Vacancy" ),
              xlab = "Time", ylab = "Filtered series",
              tit = paste( "Shimer puzzle (", legends[ k ], ")" ),
              subtit = subtit )

    entry1exit <- ( Xdata[[ k ]]$entry1 - Xdata[[ k ]]$exit1 ) * Xdata[[ k ]]$F1
    entryCexit <- ( Xdata[[ k ]]$entryC - Xdata[[ k ]]$exitC ) * Xdata[[ k ]]$Fc

    plot_bpf( list( log0( Xdata[[ k ]]$GDPreal ), entry1exit, entryCexit ),
              pl = lowP, pu = highP, nfix = bpfK, mask = TmaskPlot,
              resc = c( 0.5, NA ), mrk = transMk, col = colors, lty = lTypes,
              leg = c( "GDP", "Net entry (capital)",
                       "Net entry (consumption)" ),
              xlab = "Time", ylab = "Filtered series (rescaled)",
              tit = paste( "Net entry and business cycle (", legends[ k ], ")" ),
              subtit = subtit )

    #
    # ---- GDP autocorrelation ----
    #

    acf( logNA( Xdata[[ k ]]$GDPreal[ TmaskStat ] ), lag.max = 150, ci = CI,
         xlab = "GDP lag periods", ylab = "Real GDP average autocorrelation",
         main = paste( "GDP autocorrelation (", legends[ k ], ")" ),
         sub = paste0( "( blue: ", CI * 100, "% confidence level / period = ",
                       warmUpStat + 1, "-", nTstat, " / MC runs = ", nSize,
                       " / MC ", Xtag[ k ], " ", cntLeg, " )" ) )

    gdp.acf.stats <- autocorr_table( mcData[[ k ]][ TmaskStat, "GDPreal", ],
                                     lagsLT, logVar = 3, CI = CI )

    textplot( formatC( gdp.acf.stats, digits = sDigits, format = "g" ), cmar = 2,
              show.rownames = FALSE )
    title <- paste( "GDP autocorrelation MC results (", legends[ k ], ")" )
    subTitle <- paste( paste0(
                    "( auto-correlation function estimation means / period = ",
                    warmUpStat + 1, "-", nTstat, " / MC runs = ", nSize,
                    " / MC ", mcStat, " ", cntLeg, " )" ),
                    paste0( "( test H0: lag is not significant at ",
                            ( 1 - CI ) * 100, "% level )" ), sep ="\n" )
    title( main = title, sub = subTitle )

    saveCSV( gdp.acf.stats, baseName = repName, num = k, baseFolder = folder,
             subFolder = outDir, suffix = datFilSfx, type = "gdp_autocorr" )

    #
    # ---- GDP spectral analysis ----
    #

    spectrum( logNA( Xdata[[ k ]]$GDPreal[ TmaskStat ] ), spans = 4,
              detrend = TRUE, ci = CI, xlab="Frequency (1/period)",
              ylab="Log real GDP average spectral density",
              main = paste( "GDP spectral analysis (", legends[ k ], ")" ) )

    gdp.spec.stats <- spectrum_table( mcData[[ k ]][ TmaskStat, "GDPreal", ],
                                      lagsLT, logVar = 3, CI = CI )

    textplot( formatC( gdp.spec.stats, digits = sDigits, format = "g" ), cmar = 2,
              show.rownames = FALSE )
    title <- paste( "GDP spectral analysis MC results (", legends[ k ], ")" )
    subTitle <- paste( paste0( "( spectral densities estimation means / period = ",
                               warmUpStat + 1, "-", nTstat, " / MC runs = ",
                               nSize, " / MC ", Xtag[ k ], " ", cntLeg, " )" ),
                       paste0( "( test H0: period is not significant at ",
                               ( 1 - CI ) * 100, "% level )" ), sep ="\n" )
    title( main = title, sub = subTitle )

    saveCSV( gdp.spec.stats, baseName = repName, num = k, baseFolder = folder,
             subFolder = outDir, suffix = datFilSfx, type = "gdp_spectrum" )

    #
    # ---- A = f( K / L ) ----
    #

    subtit <- paste0( "( period = ", warmUpStat + 1, "-", nTstat, " / MC runs = ",
                      nSize, " / MC ", Xtag[ k ], " ", cntLeg, " )" )

    plot_lin( Xdata[[ k ]]$KcBas[ TmaskStat ] / Xdata[[ k ]]$LcBas[ TmaskStat ],
              Xdata[[ k ]]$AcBas[ TmaskStat ],
              xlab = "Number of machines to workers ratio",
              ylab = "Average labor productivity",
              tit = paste( "Process innovation in basic industries (",
                           legends[ k ], ")" ),
              subtit = subtit, invleg = TRUE )

    plot_lin( Xdata[[ k ]]$KcLux[ TmaskStat ] / Xdata[[ k ]]$LcLux[ TmaskStat ],
              Xdata[[ k ]]$AcLux[ TmaskStat ],
              xlab = "Number of machines to workers ratio",
              ylab = "Average labor productivity",
              tit = paste( "Process innovation in Luxury industries (",
                           legends[ k ], ")" ),
              subtit = subtit, invleg = TRUE )

    #
    # ---- correlation table ----
    #

    corr_table( c( "GDPreal", "Creal", "Ireal", "CPI", "A", "U", "wAvgReal",
                   "muCavg", "r", "DebGDP", "TC", "Loans", "BadDeb", "exit1",
                   "exitC", "sTavg", "TeAvg" ),
                mcData[[ k ]], plot = TRUE,
                logVars = c( 1, 1, 1, 0, 1, 0, 1, 0, 0, 0, 2, 2, 2, 0, 0, 0, 0 ),
                mask = TmaskStat, pl = lowP, pu = highP, nfix = bpfK,
                tit = paste( "Pearson correlation coefficients (", legends[ k ], ")" ),
                subtit = paste0( "( insignificant values at ", ( 1 - CI ) * 100,
                                 "% in white / period = ", warmUpStat + 1, "-",
                                 nTstat, " / MC runs = ", nSize, " ", cntLeg, " )" ),
                labVars = c( "GDP", "Consumption", "Investment", "Cons. price",
                             "L. productivity", "Unemployment", "Wage", "Mark-up",
                             "Interest", "Gov. debt", "Credit supply", "Loans",
                             "Bad debt", "Exit (cap.)", "Exit (cons.)", "Skills",
                             "Tenure" ) )

    #
    # ---- correlation structure tables ----
    #

    # add additional composed variables to dataset
    newVar <- dim( mcData[[ k ]] )[[ 2 ]] + 1
    mcData[[ k ]] <- abind( mcData[[ k ]],
                            mcData[[ k ]][ , "Deb1", ] + mcData[[ k ]][ , "DebC", ],
                            ( mcData[[ k ]][ , "NW1", ] + mcData[[ k ]][ , "NWc", ] ) /
                              mcData[[ k ]][ , "S", ],
                            mcData[[ k ]][ , "exit1", ] * mcData[[ k ]][ , "F1", ] +
                              mcData[[ k ]][ , "exitC", ] * mcData[[ k ]][ , "Fc", ],
                            mcData[[ k ]][ , "entry1", ] * mcData[[ k ]][ , "F1", ] +
                              mcData[[ k ]][ , "entryC", ] * mcData[[ k ]][ , "Fc", ],
                            ( mcData[[ k ]][ , "entry1", ] -
                                mcData[[ k ]][ , "exit1", ] ) *
                              mcData[[ k ]][ , "F1", ] +
                              ( mcData[[ k ]][ , "entryC", ] -
                                  mcData[[ k ]][ , "exitC", ] ) *
                              mcData[[ k ]][ , "Fc", ],
                            along = 2 )
    dimnames( mcData[[ k ]] )[[ 2 ]][ seq( newVar, newVar - 1 + 5 ) ] <-
      c( "Deb1c", "NWS1c", "exit1c", "entry1c", "netEntr1c" )

    corr.struct.1 <- corr_struct( "GDPreal", c( "Creal", "Ireal", "EI", "dNnom",
                                                "U", "A", "muCavg", "Deb1c",
                                                "NWS1c", "entry1c", "exit1c" ),
                                  mcData[[ k ]], labRef = "GDP (output)",
                                  labVars = c( "Consumption", "Investment",
                                               "Net investment", "Change in inventories",
                                               "Unemployment rate", "Productivity",
                                               "Mark-up (cons.)", "Total firm debt",
                                               "Liquidity-to-sales ratio",
                                               "Ëntry", "Exit" ),
                                  logVars = c( 1, 1, 1, 2, 0, 1, 0, 2, 2, 0, 0 ),
                                  logRef = 2, mask = TmaskStat, lags = lags,
                                  pl = lowP, pu = highP, nfix = bpfK, CI = CI )

    textplot( formatC( corr.struct.1, digits = sDigits, format = "g" ), cmar = 1 )

    title <- paste( "Correlation structure for GDP (1) (", legends[ k ], ")" )
    testMsg <- paste0( "( test H0: lag coefficient is not significant at ",
                       ( 1 - CI ) * 100, "% level", " )" )
    subTitle <- paste( paste0( "( ", bpfMsg, " / period = ", warmUpStat + 1, "-",
                               nTstat, " / MC runs = ", nSize, " ", cntLeg, " )" ),
                       testMsg, sep = "\n" )
    title( main = title, sub = subTitle )

    corr.struct.2 <- corr_struct( "GDPreal", c( "Creal", "Ireal", "A",
                                                "netEntr1c", "wAvgReal", "U", "V",
                                                "sTavg", "TeAvg", "Lent", "Lexit" ),
                                  mcData[[ k ]], labRef = "GDP (output)",
                                  labVars = c( "Consumption", "Investment",
                                               "Productivity", "Net entry",
                                               "Wage", "Unemployment rate",
                                               "Vacancy rate", "Skills",
                                               "Tenure", "Hiring", "Firing" ),
                                  logVars = c( 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0 ),
                                  logRef = 2, mask = TmaskStat, lags = lags,
                                  pl = lowP, pu = highP, nfix = bpfK, CI = CI )

    textplot( formatC( corr.struct.2, digits = sDigits, format = "g" ), cmar = 1 )

    title <- paste( "Correlation structure for GDP (2) (", legends[ k ], ")" )
    title( main = title, sub = subTitle )

    saveCSV( rbind( corr.struct.1, corr.struct.2 ), baseName = repName, num = k,
             baseFolder = folder, subFolder = outDir, suffix = datFilSfx,
             type = "corr_struct" )

    #
    # ---- MC growth statistics and unit root tests ----
    #

    key.stats <- growth_stats( c( "GDPreal", "Creal", "Ireal", "A", "wAvgReal",
                                  "sTavg", "TeAvg" ),
                               mcData[[ k ]], mask = TmaskStat,
                               labVars = c( "GDP (output)", "Consumption",
                                            "Investment", "Product.", "Real wage",
                                            "Skills", "Tenure" ),
                               pl = lowP, pu = highP, nfix = bpfK, CI = CI )

    textplot( formatC( key.stats, digits = sDigits, format = "g" ), cmar = 2 )

    title <- paste( "Growth statistics and unit roots tests (",
                    legends[ k ], ")" )
    testMsg <- paste0( "( test H0: there are unit roots / non-stationary at ",
                       ( 1 - CI ) * 100, "% level", " )" )
    subTitle <- paste( paste0( "( ", bpfMsg, " / period = ", warmUpStat + 1, "-",
                               nTstat," / MC runs = ", nSize, " ", cntLeg, " )" ),
                       testMsg, sep = "\n" )
    title( main = title, sub = subTitle )

    saveCSV( key.stats, baseName = repName, num = k, baseFolder = folder,
             subFolder = outDir, suffix = datFilSfx, type = "key_stats" )

    #
    # ------ Stationarity & ergodicity tests ------
    #

    statErgo <- ergod.test.lsd( mcData[[ k ]][ TmaskStat, , ], signif = 1 - CI,
                                vars = c( "dGDPreal", "dA", "dw", "wGini", "wLogSD",
                                          "V", "U", "muCavg", "HH1", "HHc",
                                          "entry1", "entryC", "exit1", "exitC" ) )
    rownames( statErgo ) <- c( "GDP g.r.", "Prod.g.r.", "Wage g.r.", "Wage Gini",
                               "Wage spread", "Vacancy", "Unemployment",
                               "Mark-up", "Conc. (cap.)", "Conc. (cons.)",
                               "Entry (cap.)", "Entry (cons.)", "Exit (cap.)",
                               "Exit (cons.)" )

    textplot( statErgo, cmar = 1 )

    title <- paste( "Stationarity, i.i.d. and ergodicity tests (",
                    legends[ k ], ")" )
    testMsg <- paste(
      "( ADF/PP H0: non-stationary, KPSS H0: stationary, BDS H0: i.i.d., KS/AD/WW H0: ergodic )" ,
      paste0( "( significance = ", ( 1 - CI ) * 100, "% )" ), sep = "\n" )
    subTitle <- paste( paste0(
      "( average p-values for testing H0 and rate of rejection of H0 / period = ",
      warmUpStat + 1, "-", nTstat, " / MC runs = ", nSize, " ", cntLeg, " )" ),
      testMsg, sep = "\n" )
    title( main = title, sub = subTitle )

    saveCSV( statErgo, baseName = repName, num = k, baseFolder = folder,
             subFolder = outDir, suffix = datFilSfx, type = "ergod" )

    #
    # ---- Unemployment = f( new industry arrival ) ----
    #

    # stats::lag.zoo must be use to avoid incompatible dplyr::lag
    lag <- stats::lag

    U <- as.vector( rollmedian( Xdata[[ k ]]$Ue[ TmaskStat ], lags + 1,
                                fill = NA ) )

    Fc.lag <- lag( zoo( Xdata[[ k ]]$FcBas[ TmaskStat ] ), k = - lags - 1,
                   na.pad = TRUE )
    DeltaFc <- as.vector( diff( Fc.lag, na.pad = TRUE ) / Fc.lag )
    arrivals <- replace ( DeltaFc, which( DeltaFc <= 0 ), NA )

    subtit <- paste0( "( Consider only net positive entry cases / period = ",
                      warmUpStat + 1, "-", nTstat, " / MC runs = ", nSize,
                      " / MC ", Xtag[ k ], " ", cntLeg, " )" )

    plot_lin( arrivals, U, xlab = "Average rate of basic-good industries arriving",
              ylab = "Median unemployment",
              tit = paste( "Unemployment vs. arrival of new basic industry (",
                           legends[ k ], ")" ),
              subtit = subtit, legalign = TRUE )

    Fc.lag <- lag( zoo( Xdata[[ k ]]$FcLux[ TmaskStat ] ), k = - lags - 1, na.pad = TRUE )
    DeltaFc <- as.vector( diff( Fc.lag, na.pad = TRUE ) / Fc.lag )
    arrivals <- replace ( DeltaFc, which( DeltaFc <= 0 ), NA )

    plot_lin( arrivals, U, xlab = "Average rate of luxury-good industries arriving",
              ylab = "Median unemployment",
              tit = paste( "Unemployment vs. arrival of new luxury industry (",
                           legends[ k ], ")" ),
              subtit = subtit, legalign = TRUE )

    U.arrivals.bas.b <- U.arrivals.bas.R2 <- U.arrivals.lux.b <-
      U.arrivals.lux.R2 <- A.KL.bas.b <- A.KL.bas.R2 <- A.KL.lux.b <-
      A.KL.lux.R2 <- rep( NA_real_, nSize )

    for( j in 1 : nSize ) {

      # model estimations
      U <- as.vector( rollmedian( mcData[[ k ]][ TmaskStat, "Ue", j ],
                                  k = lags + 1, fill = NA ) )
      Fc.lag <- lag( zoo( mcData[[ k ]][ TmaskStat, "FcBas", j ] ),
                     k = - lags - 1, na.pad = TRUE )
      DeltaFc <- as.vector( diff( Fc.lag, na.pad = TRUE ) / Fc.lag )
      arrivals <- replace ( DeltaFc, which( DeltaFc <= 0 ), NA )

      tryCatch( {
        model <- lm( U ~ arrivals )
        U.arrivals.bas.b[ j ] <- coef( summary( model ) )[ 2, 1 ]
        U.arrivals.bas.R2[ j ] <- summary( model )$adj.r.squared
      }, error = function( cond ) { } )

      Fc.lag <- lag( zoo( mcData[[ k ]][ TmaskStat, "FcLux", j ] ),
                     k = - lags - 1, na.pad = TRUE )
      DeltaFc <- as.vector( diff( Fc.lag, na.pad = TRUE ) / Fc.lag )
      arrivals <- replace ( DeltaFc, which( DeltaFc <= 0 ), NA )

      tryCatch( {
        model <- lm( U ~ arrivals )
        U.arrivals.lux.b[ j ] <- coef( summary( model ) )[ 2, 1 ]
        U.arrivals.lux.R2[ j ] <- summary( model )$adj.r.squared
      }, error = function( cond ) { } )

      tryCatch( {
        model <- lm( mcData[[ k ]][ TmaskStat, "AcBas", j ] ~
                       mcData[[ k ]][ TmaskStat, "KcBas", j ] /
                       mcData[[ k ]][ TmaskStat, "LcBas", j ] )
        A.KL.bas.b[ j ] <- coef( summary( model ) )[ 2, 1 ]
        A.KL.bas.R2[ j ] <- summary( model )$adj.r.squared
      }, error = function( cond ) { } )

      tryCatch( {
        model <- lm( mcData[[ k ]][ TmaskStat, "AcLux", j ] ~
                       mcData[[ k ]][ TmaskStat, "KcLux", j ] /
                       mcData[[ k ]][ TmaskStat, "LcLux", j ] )
        A.KL.lux.b[ j ] <- coef( summary( model ) )[ 2, 1 ]
        A.KL.lux.R2[ j ] <- summary( model )$adj.r.squared
      }, error = function( cond ) { } )
    }

    critCorr <- qnorm( 1 - ( 1 - CI ) / 2 ) / sqrt( nTstat )

    U.arrivals.stats <- matrix( c( mean( U.arrivals.bas.b, na.rm = TRUE ),
                                   mean( U.arrivals.lux.b, na.rm = TRUE ),

                                   sd( U.arrivals.bas.b, na.rm = TRUE ) /
                                     sqrt( nSize ),
                                   sd( U.arrivals.lux.b, na.rm = TRUE ) /
                                     sqrt( nSize ),

                                   t.test0( U.arrivals.bas.b, critCorr, CI ),
                                   t.test0( U.arrivals.lux.b, critCorr, CI ),

                                   max( U.arrivals.bas.b, na.rm = TRUE ),
                                   max( U.arrivals.lux.b, na.rm = TRUE ),

                                   min( U.arrivals.bas.b, na.rm = TRUE ),
                                   min( U.arrivals.lux.b, na.rm = TRUE ),

                                   mean( U.arrivals.bas.R2, na.rm = TRUE ),
                                   mean( U.arrivals.lux.R2, na.rm = TRUE ),

                                   sd( U.arrivals.bas.R2, na.rm = TRUE ) /
                                     sqrt( nSize ),
                                   sd( U.arrivals.lux.R2, na.rm = TRUE ) /
                                     sqrt( nSize ),

                                   t.test0( U.arrivals.bas.R2, critCorr, CI ),
                                   t.test0( U.arrivals.lux.R2, critCorr, CI ),

                                   max( U.arrivals.bas.R2, na.rm = TRUE ),
                                   max( U.arrivals.lux.R2, na.rm = TRUE ),

                                   min( U.arrivals.bas.R2, na.rm = TRUE ),
                                   min( U.arrivals.lux.R2, na.rm = TRUE ) ),
                                ncol = 2, byrow = TRUE )

    colnames( U.arrivals.stats ) <- c( "Basic industries", "Luxury industries" )
    rownames( U.arrivals.stats ) <- c( "Mean slope", " (s.e.)", " (p-value)",
                                       "Max. slope", "Min. slope", "Mean std. R2",
                                       " (s.e.)", " (p-value)", "Max. R2", "Min. R2" )
    textplot( formatC( U.arrivals.stats, digits = sDigits, format = "g" ), cmar = 2 )
    title <- paste( "Unemployment vs. arrival of new industries MC results (",
                    legends[ k ], ")" )
    subTitle <- paste( paste0( "( linear regression / period = ", warmUpStat + 1,
                               "-", nTstat, " / MC runs = ", nSize, "/ MC ",
                               Xtag[ k ], " ", cntLeg, " )" ),
                       paste0( "( test H0: statistic is not significant at ",
                               ( 1 - CI ) * 100, "% level",
                               " )" ), sep ="\n" )
    title( main = title, sub = subTitle )

    saveCSV( U.arrivals.stats, baseName = repName, num = k, baseFolder = folder,
             subFolder = outDir, suffix = datFilSfx, type = "arrival" )
  }


  clearTemp( savDat = savDat, files = files, nExp = nExp,
             folder = folder, baseName = baseName, datFilSfx = datFilSfx )


  # ------ End of report ------

}, interrupt = catchError, error = catchError, finally = showEndMark( ) )
