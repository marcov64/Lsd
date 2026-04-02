#******************************************************************
#
# ----------------- K+S labor market analysis -------------------
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
caption   <- "K+S labor analysis"                         # caption for logs
datFilSfx <- "labor"                                      # data file suffix

# aggregated variables to import and add
logVar    <- c( "A", "Yreal", "wAvg", "wAvgReal" )
nlogVar   <- c( "Ls", "Lent", "Lexit", "TuAvg", "U", "V", "dY" )
origVar   <- c( logVar, nlogVar )
addVar    <- c( )


# ==== Libraries, support functions and global options ====

source( "KS-support-functions.R" )

options( warn = 0 )         # -1=no warning/0:warnings at end/2:warnings stop

# remove RStudio warnings for support functions
# !diagnostics suppress = showStartMark, showEndMark, setCmdLinePars, loadData
# !diagnostics suppress = catchError, startTime, mean.clean, vector.clean, adrop
# !diagnostics suppress = log0, textplot, saveCSV, plot_xy, hpfilter, abs_max
# !diagnostics suppress = twoord.plot, clearTemp, repFile, setLabels, outDir
# !diagnostics suppress = mc, pool, nTsteps, nSize, legends, expLeg, listLeg
# !diagnostics suppress = cntLeg, allLeg, setLabels


# ==== Process LSD result files ====

showStartMark( caption )  # log start mark

setCmdLinePars( )         # read command line parameters (if any)

# read LSD files or load existing temporary files according to the case
files <- loadData( savDat = savDat, nExp = nExp, mCnt = mCnt, nCnt = nCnt,
                   folder = folder, baseName = baseName, iniDrop = iniDrop,
                   nKeep = nKeep, origVar = origVar, addVar = addVar,
                   datFilSfx = datFilSfx, coresExp = coresExp,
                   coresMC = coresMC, mcStat = mcStat, CI = CI,
                   bootR = bootR, bootCI = bootCI )


#******************************************************************
#
# --------------------- Plot statistics -------------------------
#
#******************************************************************

# ===================== User parameters =========================

bCase     <- 1      # experiment to be used as base case
warmUpPlot<- 100    # number of "warm-up" runs for plots
warmUpStat<- 300    # warm-up runs to evaluate all statistics
nTstat    <- -1     # last period to consider for statistics (-1=all)
smoothing <- 1e5    # HP filter smoothing factor (lambda)
numSeries <- 10     # number of Shimer statistics to create
numParam  <- 5      # number of MC parameters to be estimated
limOutl   <- 0.05   # quantile extreme limits for graphs (0=none)
crisisTh  <- -0.03  # crisis growth threshold
crisisLen <- 3      # crisis minimum duration (periods)

cores     <- 1      # maximum number of cores to allocate (0=all)
parStats  <- 1      # number of statistics to be computed in parallel
repName   <- ""     # report files base name (if "" same baseName)
transMk   <- -1     # regime transition mark after warm-up (-1:none)
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


# ==== Support stuff ====

# generate labels & build labels list legend
setLabels( nExp, mCnt, nCnt )

# load data from first experiment
load( files[[ 1 ]]$pool )

# number of periods to show in graphics and use in statistics
if( nTstat < 1 || nTstat > nTsteps || nTstat <= warmUpStat )
  nTstat <- nTsteps
TmaxStat <- nTstat - warmUpStat
TmaskStat <- ( warmUpStat + 1 ) : nTstat

# critical correlation limit for significance (under heroic assumptions!)
critCorr <- qnorm( 1 - ( 1 - CI ) / 2 ) / sqrt( nTstat )


# ==== Main code ====

tryCatch( {   # enter error handling mode so PDF can be closed in case of error

  # create the report file(s) in a daily output directory
  repFile( repName, folder = folder, suffix = datFilSfx, width = plotW,
           height = plotH, rows = plotRows, cols = plotCols,
           raster = raster, res = res )

  #
  # ===== Monte Carlo analysis =====
  #

  cat( "\nGenerating MC reports...\n" )

  shimer.series <- array( dim = c( numSeries, TmaxStat, nExp ) )
  temp.series <- array( dim = c( numSeries, TmaxStat ) )
  series.labels <- list( "u", "v", "v/u", "f", "s", "p", "wReal", "w", "du", "g" )
  shimer.param <- array( dim = c( numParam, 2, nSize, nExp ) )
  param.labels <- list( "Beveridge curve", "Matching function", "Wage curve",
                        "Phillips curve", "Okun curve" )
  shimer.param.mc <- array( dim = c( numParam, 4, nExp ) )
  param.mc.labels <- list( "b avg", "b se", "R2 avg", "R2 se" )
  recovery.param <- array( dim = c( 3, nSize, nExp ) )
  recovery.param.mc <- array( dim = c( 6, nExp ) )

  for( k in 1 : nExp ) {                                # do for each experiment

    cat( "", expLeg, k, "of", nExp, "\n" )

    # ------ compute statistics and find a nice case to present ------

    R2.total <- -1

    for( j in 1 : nSize ) {                             # for each MC case

      cat( "  Monte Carlo case", j, "of", nSize, "\n" )

      # load MC data from temporary files and remove instance dimension (always 1)
      load( files[[ k ]]$mc[ j ] )
      mc <- adrop( mc[ , , 1, drop = FALSE ], drop = 3 )

      # ------ compute single MC statistics ------

      laborForce <- mc[ TmaskStat, "Ls" ]

      # Find the gaps (in level) between unemployment, vacancy, entry and exit
      # This is used to force the average unemployment to be the same as
      # the average vacancy and so on, so the standard deviations of logs can be
      # compared (I guess this was the same trick Shimer used to have SDs compared)
      # For the following analysis, this have a scale effect only
      uvGap <- mean( mc[ TmaskStat, "V" ] -
                     mc[ TmaskStat, "U" ] )
      fvGap <- mean( mc[ TmaskStat, "V" ] -
                       mc[ TmaskStat, "Lent" ] )

      # Calculates detrended unemployment for selected MC series
      series <- log0( ( mc[ TmaskStat, "U" ] + uvGap ) * laborForce )
      temp.series[ 1, ] <- series - hpfilter( series, smoothing )$trend

      # Calculates detrended job vacancies for selected MC series
      series <- log0( mc[ TmaskStat, "V" ] * laborForce )
      temp.series[ 2, ] <- series - hpfilter( series, smoothing )$trend

      # Calculates detrended job vacancies/unemployment ratio for selected MC series
      temp.series[ 3, ] <- temp.series[ 2, ] - temp.series[ 1, ]

      # Calculates detrended job-finding rate for selected MC series
      series <- log0( ( mc[ TmaskStat, "Lent" ] + fvGap ) * laborForce )
      temp.series[ 4, ] <- series - hpfilter( series, smoothing )$trend

      # Calculates detrended real log wage for selected MC series
      series <- log0( mc[ TmaskStat, "wAvgReal" ] )
      temp.series[ 7, ] <- series - hpfilter( series, smoothing )$trend

      # Calculates detrended nominal log wage change for selected MC series
      series <- log0( mc[ TmaskStat, "wAvg" ] )
      trend <- hpfilter( series, smoothing )$trend
      temp.series[ 8, 1 ] <- 0
      for( i in 2 : TmaxStat ) {
        temp.series[ 8, i ] <- ( series[ i ] - trend[ i ] ) -
          ( series[ i - 1 ] - trend[ i - 1 ] )
      }

      # Calculates detrended unemployment change for selected MC series
      temp.series[ 9, 1 ] <- 0
      for( i in 2 : TmaxStat )
        temp.series[ 9, i ] <- temp.series[ 1, i ] - temp.series[ 1, i - 1 ]

      # Calculates detrended GDP log growth for selected MC series
      series <- log0( mc[ TmaskStat, "Yreal" ] )
      trend <- hpfilter( series, smoothing )$trend
      temp.series[ 10, 1 ] <- 0
      for( i in 2 : TmaxStat ) {
        temp.series[ 10, i ] <- ( series[ i ] - trend[ i ] ) -
          ( series[ i - 1 ] - trend[ i - 1 ] )
      }

      # ------ Update MC statistics ------

      # Beveridge curve
      linReg <- lm( temp.series[ 2, ] ~ temp.series[ 1, ] )
      R2.bever <- summary( linReg ) $ adj.r.squared
      shimer.param[ 1, , j, k ] <- c( linReg $ coefficients[2], R2.bever )

      # Matching function
      linReg <- lm( temp.series[ 4, ] ~ temp.series[ 3, ] )
      R2.match <-  summary( linReg ) $ adj.r.squared
      shimer.param[ 2, , j, k ] <- c( linReg $ coefficients[2], R2.match )

      # Wage curve
      linReg <- lm( temp.series[ 7, ] ~ temp.series[ 1, ] )
      R2.wagec <-  summary( linReg ) $ adj.r.squared
      shimer.param[ 3, , j, k ] <- c( linReg $ coefficients[2], R2.wagec )

      # Phillips curve
      linReg <- lm( temp.series[ 8, ] ~ temp.series[ 1, ] )
      R2.phill <-  summary( linReg ) $ adj.r.squared
      shimer.param[ 4, , j, k ] <- c( linReg $ coefficients[2], R2.phill )

      # Okun curve
      linReg <- lm( temp.series[ 10, ] ~ temp.series[ 9, ] )
      R2.okun <-  summary( linReg ) $ adj.r.squared
      shimer.param[ 5, , j, k ] <- c( linReg $ coefficients[2], R2.okun )

      # Search for the best case to present
      if( ! is.nan( R2.bever ) && ! is.nan( R2.match ) && ! is.nan( R2.wagec ) &&
          ! is.nan( R2.phill ) && ! is.nan( R2.okun ) )
        if( ( R2.bever + R2.match + R2.wagec + R2.phill + R2.okun ) > R2.total ){
          mcCase <- j
          shimer.series[ , , k ] <- temp.series
          R2.total <- R2.bever + R2.match + R2.wagec + R2.phill + R2.okun
        }

      # Calculate average recovery times for unemployment time
      recovery <- FALSE
      recTime <- vector( mode = "numeric" )
      preCrisisTuAvg <- mc[ warmUpStat, "TuAvg" ]
      lastCrisis <- warmUpStat
      for( i in ( warmUpStat + 1 ) : nTstat ) {
        if( recovery && mc[ i, "TuAvg" ] <= preCrisisTuAvg ) {
          recovery <- FALSE
          if( i - lastCrisis > crisisLen ) # record only if lasting more x periods
            recTime <- append( recTime, i - lastCrisis )
        }
        if( ! recovery && mc[ i, "dY" ] < crisisTh ) {
          recovery <- TRUE
          lastCrisis <- i
          preCrisisTuAvg <- mc[ i - 1, "TuAvg" ]
        }
      }

      # prepare to consolidate MC results
      if( length( recTime ) > 0 )
        recovery.param[ , j, k ] <- c( mean( recTime ), sd( recTime ),
                                       length( recTime ) )
      else
        recovery.param[ , j, k ] <- c( NA, NA, NA )
    }

    cat( " Experiment", k, ": MC run #", mcCase, "selected\n" )

    # reload MC data for selected MC run
    load( files[[ k ]]$mc[ mcCase ] )
    mc <- adrop( mc[ , , 1, drop = FALSE ], drop = 3 )

    # Calculates detrended separation rate for selected MC series
    svGap <- mean( mc[ TmaskStat, "V" ] -
                   mc[ TmaskStat, "Lexit" ] )
    series <- log0( ( mc[ TmaskStat, "Lexit" ] + svGap ) * laborForce )
    shimer.trend.5 <- hpfilter( series, smoothing )$trend
    shimer.series[ 5, , k ] <- series - shimer.trend.5

    # Calculates detrended per worker log production for selected MC series
    series <- log0( mc[ TmaskStat, "A" ] )
    shimer.trend.6 <- hpfilter( series, smoothing )$trend
    shimer.series[ 6, , k ] <- series - shimer.trend.6


    #
    # ==== Plot graphs ====
    #

    if( limOutl > 0 ) {
      subtit <- paste0( "Outliers removed, included percentiles = ",
                        limOutl * 100, "-", ( 1 - limOutl ) * 100,
                        " / period = ", warmUpStat + 1, "-", nTstat,
                        " / MC case = ", mcCase, " ", cntLeg )
    } else {
      subtit <- paste0( "period = ", warmUpStat + 1, "-", nTstat,
                        " / MC case = ", mcCase, " ", cntLeg )
    }

    # ---- Beveridge curve ----

    plot_xy( shimer.series[ 1, , k ], shimer.series[ 2, , k ], quant = limOutl,
             tit = paste( "Beveridge curve (", legends[k], ")" ),
             subtit = subtit,
             xlab = "Unemployment rate", ylab = "Vacancy rate" )

    # ---- Matching function ----

    plot_xy( shimer.series[ 3, , k ], shimer.series[ 4, , k ], quant = limOutl,
             tit = paste( "Matching function (", legends[k], ")" ), subtit = subtit,
             xlab = "Vacancies/unemployment ratio", ylab = "Job-finding rate" )

    # ---- Wage curve ----

    plot_xy( shimer.series[ 1, , k ], shimer.series[ 7, , k ], quant = limOutl,
             tit = paste( "Wage curve (", legends[k], ")" ), subtit = subtit,
             xlab = "Unemployment rate", ylab = "Log real wage" )

    # ---- Phillips curve ----

    plot_xy( shimer.series[ 1, , k ], shimer.series[ 8, , k ], quant = limOutl,
             tit = paste( "Phillips curve (", legends[k], ")" ), subtit = subtit,
             xlab = "Unemployment rate", ylab = "Log nominal wage change" )

    # ---- Okun curve ----

    plot_xy( shimer.series[ 9, , k ], shimer.series[ 10, , k ], quant = limOutl,
             tit = paste( "Okun curve (", legends[k], ")" ), subtit = subtit,
             xlab = "Unemployment change", ylab = "GDP growth" )

    # ---- Periodic separation ----

    subtit <- paste0( "( HP-filtered trend, smoothing parameter = ", smoothing,
                      " / period = ", warmUpStat + 1, "-", nTstat,
                      " / MC case = ", mcCase, " ", cntLeg, " )" )

    plot( TmaskStat, shimer.series[ 5, , k ] + shimer.trend.5, type = "l",
          main = paste( "Periodic separation (", legends[k], ")" ),
          sub = subtit, xlab = "Time", ylab = "Separation probability",
          col = colors[ 1 ], lty = lTypes[ 1 ] )

    lines( TmaskStat, shimer.trend.5, type = "l", lwd = 2,
           col = colors[ 1 ], lty = lTypes[ 1 ] )

    if( transMk > 0 )
      lines( x = c( transMk, transMk ),
             y = c( min( shimer.series[ 5, , k ] + shimer.trend.5 ),
                    max( shimer.series[ 5, , k ] + shimer.trend.5 ) ),
             lty = 3, col = "black" )

    legend( x = "topleft", inset = 0.03, cex = 0.8,
            legend = c( "Series", "Trend" ),
            lwd = c( 1, 2 ), col = c( colors[ 1 ], colors[ 1 ] ) )

    # ---- Labor productivity ----

    plot( TmaskStat, shimer.series[ 6, , k ] + shimer.trend.6, type = "l",
          main = paste( "Labor productivity (", legends[k], ")" ),
          sub = subtit, xlab = "Time", ylab = "Average log labor productivity",
          col = colors[ 1 ], lty = lTypes[ 1 ] )

    lines( TmaskStat, shimer.trend.6, type = "l", lwd = 2,
           col = colors[ 1 ], lty = lTypes[ 1 ] )

    if( transMk > 0 )
      lines( x = c( transMk, transMk ),
             y = c( min( shimer.series[ 6, , k ] + shimer.trend.6 ),
                    max( shimer.series[ 6, , k ] + shimer.trend.6 ) ),
             lty = 3, col = "black" )

    legend( x = "topleft", inset = 0.03, cex = 0.8,
            legend = c( "Series", "Trend" ),
            lwd = c( 1, 2 ), col = c( colors[ 1 ], colors[ 1 ] ) )

    # ---- Vacancy-unemployment & productivity ----

    mL <- abs_max( shimer.series[ 3, , k ], quant = 0 )
    mR <- abs_max( shimer.series[ 6, , k ], quant = 0 )

    twoord.plot( TmaskStat, shimer.series[ 3, , k ],
                 TmaskStat, shimer.series[ 6, , k ],
                 main = paste( "Vacancy-unemployment & productivity (",
                               legends[k], ")" ),
                 sub = paste0( "( period = ", warmUpStat + 1, "-", nTstat,
                               " / MC case = ", mcCase, " ", cntLeg, " )" ),
                 xlab = "Time", ylab = "Vacancy-unemployment rate",
                 rylab = "Average log labor productivity",
                 lcol = colors[ 1 ], rcol = colors[ 2 ],
                 lylim = c( -mL, mL ), rylim = c( -mR, mR ), type = "l" )

    if( transMk > 0 )
      lines( x = c( transMk, transMk ), y = c( -mL, mL ), lty = 3, col = "black" )

    legend( x = "topleft", inset = 0.03, cex = 0.8,
           legend = c( "Vacancy-unemployment", "Productivity" ),
           lwd = 2, col = colors )

    #
    # ======= Experiments comparison table =======
    #

    shimer.results <- matrix( c(
      sd( shimer.series[ 1, , k ] ), sd( shimer.series[ 2, , k ] ),
      sd( shimer.series[ 3, , k ] ), sd( shimer.series[ 4, , k ] ),
      sd( shimer.series[ 5, , k ] ), sd( shimer.series[ 6, , k ] ),

      acf( shimer.series[ 1, , k ], plot = FALSE )$acf[ 2 ],
      acf( shimer.series[ 2, , k ], plot = FALSE )$acf[ 2 ],
      acf( shimer.series[ 3, , k ], plot = FALSE )$acf[ 2 ],
      acf( shimer.series[ 4, , k ], plot = FALSE )$acf[ 2 ],
      acf( shimer.series[ 5, , k ], plot = FALSE )$acf[ 2 ],
      acf( shimer.series[ 6, , k ], plot = FALSE )$acf[ 2 ],

      1,
      cor( shimer.series[ 1, , k ], shimer.series[ 2, , k ] ),
      cor(  shimer.series[ 1, , k ], shimer.series[ 3, , k ] ),
      cor( shimer.series[ 1, , k ], shimer.series[ 4, , k ] ),
      cor(  shimer.series[ 1, , k ], shimer.series[ 5, , k ] ),
      cor( shimer.series[ 1, , k ], shimer.series[ 6, , k ] ),

      NA, 1,
      cor(  shimer.series[ 2, , k ], shimer.series[ 3, , k ] ),
      cor(  shimer.series[ 2, , k ], shimer.series[ 4, , k ] ),
      cor( shimer.series[ 2, , k ], shimer.series[ 5, , k ] ),
      cor(  shimer.series[ 2, , k ], shimer.series[ 6, , k ] ),

      NA, NA, 1,
      cor(  shimer.series[ 3, , k ], shimer.series[ 4, , k ] ),
      cor( shimer.series[ 3, , k ], shimer.series[ 5, , k ] ),
      cor( shimer.series[ 3, , k ], shimer.series[ 6, , k ] ),

      NA, NA, NA, 1,
      cor(  shimer.series[ 4, , k ], shimer.series[ 5, , k ] ),
      cor(  shimer.series[ 4, , k ], shimer.series[ 6, , k ] ),

      NA, NA, NA, NA, 1,
      cor(  shimer.series[ 5, , k ], shimer.series[ 6, , k ] ),

      NA, NA, NA, NA, NA, 1
    ), ncol = 6, byrow = TRUE )

    # print experiments table
    colnames( shimer.results ) <- unlist( series.labels )[ 1 : 6 ]
    rownames( shimer.results ) <- cbind( c( "Standard deviation",
                                            "Quarterly autocorrelation",
                                            unlist( series.labels )[ 1 : 6 ] ) )
    plot.results <- formatC( shimer.results, digits = sDigits, format = "g" )
    plot.results[ grep( "NA", plot.results ) ] <- ""
    textplot( plot.results, cmar = 1, cex = 1.0 )
    title <- paste( "Summary statistics (", legends[ k ], ")" )
    subTitle <- paste0(
      "( corr. matrix: unempl., vacancy, vac./unempl., job-finding, separation, product. / period = ",
      warmUpStat + 1, "-", nTstat, " / MC case = ", mcCase, " ", cntLeg," )" )
    title( main = title, sub = subTitle )

    saveCSV( plot.results, baseName = repName, num = k, baseFolder = folder,
             subFolder = outDir, suffix = datFilSfx, type = "shimer" )

    # ---- MC results for curves parameters ----

    for( i in 1 : numParam ) {
      shimer.param.mc[ i, 1, k ] <- mean( shimer.param[ i, 1, , k ] )
      shimer.param.mc[ i, 2, k ] <- sd( shimer.param[ i, 1, , k ] ) / sqrt( nSize )
      shimer.param.mc[ i, 3, k ] <- mean( shimer.param[ i, 2, , k ] )
      shimer.param.mc[ i, 4, k ] <- sd( shimer.param[ i, 2, , k ] ) / sqrt( nSize )
    }

    # ---- Average recovery times for unemployment time ----

    recovery.param.mc[ 1, k ] <- mean( recovery.param[ 1, , k ], na.rm = TRUE )
    recovery.param.mc[ 2, k ] <- sd( recovery.param[ 1, , k ], na.rm = TRUE ) /
      sqrt( nSize )
    recovery.param.mc[ 3, k ] <- mean( recovery.param[ 2, , k ], na.rm = TRUE )
    recovery.param.mc[ 4, k ] <- sd( recovery.param[ 2, , k ], na.rm = TRUE ) /
      sqrt( nSize )
    recovery.param.mc[ 5, k ] <- mean( recovery.param[ 3, , k ], na.rm = TRUE )
    recovery.param.mc[ 6, k ] <- sd( recovery.param[ 3, , k ], na.rm = TRUE ) /
      sqrt( nSize )
  }

  rm( temp.series )

  # ---- Per case curves statistics table ----

  curves.param <- shimer.param.mc[ , , 1 ]
  curves.labels <- c( paste0( param.mc.labels[[ 1 ]], "[1]" ),
                      paste0( param.mc.labels[[ 2 ]], "[1]" ),
                      paste0( param.mc.labels[[ 3 ]], "[1]" ),
                      paste0( param.mc.labels[[ 4 ]], "[1]" ) )

  if( nExp > 1 ) {    # create table
    for( k in 2 : nExp ) {
      curves.param <- cbind( curves.param, shimer.param.mc[ , , k ] )
      curves.labels <- cbind( curves.labels, c( paste0( param.mc.labels[[ 1 ]],
                                                        "[", k, "]" ),
                                                paste0( param.mc.labels[[ 2 ]],
                                                        "[", k, "]" ),
                                                paste0( param.mc.labels[[ 3 ]],
                                                        "[", k, "]" ),
                                                paste0( param.mc.labels[[ 4 ]],
                                                        "[", k, "]" ) ) )
    }
  }

  # print curves table
  colnames( curves.param ) <- curves.labels
  rownames( curves.param ) <- param.labels

  textplot( formatC( curves.param, digits = sDigits, format = "g" ), cmar = 1 )
  title <- paste( "Curves fitting", allLeg )
  subTitle <- paste0( "( numbers in brackets: experiment number / period = ",
                      warmUpStat + 1, "-", nTstat, " / MC runs = ", nSize, " ",
                      cntLeg, " )" )
  title( main = title, sub = subTitle )
  mtext( listLeg, side = 1, line = -3, outer = TRUE )

  saveCSV( curves.param, baseName = repName, baseFolder = folder,
           subFolder = outDir, suffix = datFilSfx, type = "curve_fit" )


  #
  # ======= Employment time recovery times table =======
  #

  recovery <- matrix( c( recovery.param.mc[ 5, 1 ], recovery.param.mc[ 6, 1 ],
                         recovery.param.mc[ 1, 1 ], recovery.param.mc[ 2, 1 ],
                         recovery.param.mc[ 3, 1 ], recovery.param.mc[ 4, 1 ] ) )
  recovery.labels <- legends[ 1 ]

  if( nExp > 1 ) {    # Create table
    for( k in 2 : nExp ) {
      recovery <- cbind( recovery, c( recovery.param.mc[ 5, k ],
                                      recovery.param.mc[ 6, k ],
                                      recovery.param.mc[ 1, k ],
                                      recovery.param.mc[ 2, k ],
                                      recovery.param.mc[ 3, k ],
                                      recovery.param.mc[ 4, k ] ) )
      recovery.labels <- append( recovery.labels, legends[ k ] )
    }
  }

  colnames( recovery ) <- recovery.labels
  rownames( recovery ) <- c( "Avg. number of crisis", "(s.e.)",
                             "Avg. recovery periods", "(s.e.)",
                             "Std. dev. rec. periods", "(s.e.)" )

  textplot( formatC( recovery, digits = sDigits, format = "g" ), cmar = 1 )
  title <- paste( "Unemployment time recovery after crisis", allLeg )
  subTitle <- paste0( "( MC standard error in parentheses / period = ",
                      warmUpStat + 1, "-", nTstat, " / MC runs = ", nSize, " ",
                      cntLeg, " )" )
  title( main = title, sub = subTitle )

  saveCSV( recovery, baseName = repName, baseFolder = folder,
           subFolder = outDir, suffix = datFilSfx, type = "Tu_recovery" )


  clearTemp( savDat = savDat, files = files, nExp = nExp,
             folder = folder, baseName = baseName, datFilSfx = datFilSfx )


  # ------ End of report ------

}, interrupt = catchError, error = catchError, finally = showEndMark( ) )
