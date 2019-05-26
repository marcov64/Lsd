#******************************************************************
#
# ------------- Island Model Aggregates analysis ----------------
#
#******************************************************************

#******************************************************************
#
# ------------ Read Monte Carlo experiment files ----------------
#
#******************************************************************

folder   <- "experiments"             # data files folder
baseName <- "exp"                     # data files base name (same as .lsd file)
nExp <- 2                             # number of experiments
iniDrop <- 0                          # initial time steps to drop from analysis (0=none)
nKeep <- -1                           # number of time steps to keep (-1=all)

expVal <- c( "Low oportunity", "High oportunity" )   # case parameter values

# Aggregated variables to use
logVars <- c( "Q" )
aggrVars <- append( logVars, c( "g", "l", "m", "J" ) )

# Variables to test for stationarity and ergodicity
statErgo.vars <- c( "Q", "g", "l", "m", "J" )


# ==== Process LSD result files ====

# Package with LSD interface functions
library( LSDinterface, verbose = FALSE, quietly = TRUE )
options( warn = -1 )

# ---- Read data files ----

# Get first experiment file names
if( nExp > 1 ) {
  myFiles <- list.files( path = folder, pattern = paste0( baseName, "1", "_[0-9]+.res" ),
                         full.names = TRUE )
} else {
  myFiles <- list.files( path = folder, pattern = paste0( baseName, "_[0-9]+.res" ),
                         full.names = TRUE )
}

# Determine the Monte Carlo sample size
nSize  <- length( myFiles )
# Get data set details from first file
dimData <- info.dimensions.lsd( myFiles[ 1 ] )
nTsteps <- dimData$tSteps
nVar <- dimData$nVars

cat( "\nData details:\n" )
cat( " Number of MC cases =", nSize, "\n" )
cat( " Number of periods =", nTsteps, "\n" )
nTsteps <- nTsteps - iniDrop
if( nKeep > 0 ){
  nTsteps <- min( nKeep, nTsteps )
} else {
  nKeep <- nTsteps
}
cat( " Number of used periods =", nTsteps, "\n" )
cat( " Number of variables =", nVar, "\n\n" )

# Function to read one experiment data (to be parallelized)
readExp <- function( exper ) {
  if( nExp > 1 ) {
    myFiles <- list.files( path = folder, pattern = paste0( baseName, "", exper, "_[0-9]+.res" ),
                           full.names = TRUE )
  } else {
    myFiles <- list.files( path = folder, pattern = paste0( baseName, "_[0-9]+.res" ),
                           full.names = TRUE )
  }
  cat( "\nData files: ", myFiles, "\n\n" )

  # Read data from text files and format it as a 3D array with labels
  mc <- read.3d.lsd( myFiles, aggrVars, skip = iniDrop, nrows = nKeep )

  # Compute Monte Carlo averages and std. deviation and store in 2D arrrays
  stats <- info.stats.lsd( mc )

  # Insert a t column
  t <- as.integer( rownames( stats$avg ) )
  A <- as.data.frame( cbind( t, stats$avg ) )
  S <- as.data.frame( cbind( t, stats$sd ) )
  M <- as.data.frame( cbind( t, stats$max ) )
  m <- as.data.frame( cbind( t, stats$min ) )

  # Save temporary results to disk to save memory
  tmpFile <- paste0( folder, "/", baseName, "", exper, "_aggr.Rdata" )
  save( mc, A, S, M, m, file = tmpFile )

  return( tmpFile )
}

# load each experiment serially (can be easily parallelized)
tmpFiles <- lapply( 1 : nExp, readExp )

# ---- Organize data read from files ----

# fill the lists to hold data
mcData <- list()  # 3D Monte Carlo data
Adata <- list()  # average data
Sdata <- list()  # standard deviation data
Mdata <- list()  # maximum data
mdata <- list()  # minimum data

for( k in 1 : nExp ) {                      # realocate data in separate lists

  load( tmpFiles[[ k ]] )                   # pick data from disk
  file.remove( tmpFiles[[ k ]] )            # and delete temporary file, if needed

  mcData[[ k ]] <- mc
  rm( mc )
  Adata[[ k ]] <- A
  Sdata[[ k ]] <- S
  Mdata[[ k ]] <- M
  mdata[[ k ]] <- m
}

# free memory
rm( tmpFiles, A, S, M, m )
invisible( gc( verbose = FALSE ) )


#******************************************************************
#
# --------------------- Plot statistics -------------------------
#
#******************************************************************

# ===================== User parameters =========================

bCase     <- 1      # experiment to be used as base case
CI        <- 0.95   # desired confidence interval
warmUpPlot<- 100    # number of "warm-up" runs for plots
warmUpStat<- 300    # warm-up runs to evaluate all statistics
lowP      <- 6      # bandpass filter minimum period
highP     <- 32     # bandpass filter maximum period
bpfK      <- 12     # bandpass filter order
lags      <- 4      # lags to analyze
bPlotCoef <- 1.5    # boxplot whiskers extension from the box (0=extremes)
bPlotNotc <- FALSE  # use boxplot notches
smoothing <- 1e5    # HP filter smoothing factor (lambda)
crisisTh  <- -0.03  # crisis growth threshold
crisisLen <- 3      # crisis minimum duration (periods)
crisisPre <- 4      # pre-crisis period to base trend start (>=1)
crisisRun <- 0      # the crisis case to be plotted (0=auto)
crisesPlt <- TRUE   # plot all the crisis plots in a separate pdf file?

repName   <- ""     # report files base name (if "" same baseName)
transMk   <- -1     # regime transition mark after warm-up (-1:none)
sDigits   <- 4      # significant digits in tables
plotRows  <- 1      # number of plots per row in a page
plotCols  <- 1  	  # number of plots per column in a page
plotW     <- 12     # plot window width
plotH     <- 8      # plot window height
raster    <- FALSE  # raster or vector plots
res       <- 600    # resolution of raster mode (in dpi)

# Colors assigned to each experiment's lines in graphics
colors <- c( "black", "blue", "green", "red", "orange", "brown" )
#colors <- c( "black", "black", "black", "black", "black", "black" )

# Line types assigned to each experiment
lTypes <- c( "solid", "solid", "solid", "solid", "solid", "solid" )
#lTypes <- c( "solid", "dashed", "dotted", "dotdash", "longdash", "twodash" )

# Point types assigned to each experiment
pTypes <- c( 4, 4, 4, 4, 4, 4 )
#pTypes <- c( 4, 0, 1, 2, 3, 5 )


# ====== External support functions & definitions ======

if( ! exists( "plot_norm", mode = "function" ) )     # already loaded?
  source( "support-functions.R" )
# remove warnings for support functions
# !diagnostics suppress = logNA, log0, t.test0, se, bkfilter, hpfilter, adf.test
# !diagnostics suppress = plot_lognorm, plot_norm, plot_laplace, textplot, colSds
# !diagnostics suppress = fit_subbotin, comp_stats, comp_MC_stats, plot_lists
# !diagnostics suppress = twoord.plot, abs_max, plot_xy, lm_outl
# !diagnostics suppress = plot_recovery, plot_bpf, colMins, colMaxs


# ==== Support stuff ====

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

# Number of periods to show in graphics
TmaxPlot <- nTsteps - warmUpPlot
TmaxStat <- nTsteps - warmUpStat
TmaskPlot <- ( warmUpPlot + 1 ) : nTsteps
TmaskStat <- ( warmUpStat + 1 ) : nTsteps
TmaskBpf <- ( bpfK + 1 ) : ( TmaxStat - bpfK )

# Calculates the critical correlation limit for significance (under heroic assumptions!)
critCorr <- qnorm(1-(1-CI)/2) / sqrt(nTsteps)


# ==== Main code ====

tryCatch({    # enter error handling mode so PDF can be closed in case of error/interruption

  # create a daily output directory
  outDir <- format( Sys.time(), "%Y-%m-%d" )
  if( ! dir.exists( paste0( folder, "/", outDir ) ) )
    dir.create( paste0( folder, "/", outDir ) )

  cat( paste( "\nSaving results and data to:", paste0( folder, "/", outDir ), "\n" ) )

  # Select type of output
  if( raster ){
    # Open PNG (bitmap) files for output
    png( paste0( folder, "/", outDir, "/", repName, "_aggr_plots_%d.png" ),
         width = plotW, height = plotH, units = "in", res = res )

  } else {
    # Open PDF plot file for output
    pdf( paste0( folder, "/", outDir, "/", repName, "_aggr_plots.pdf" ),
         width = plotW, height = plotH )
    par( mfrow = c ( plotRows, plotCols ) )             # define plots per page
  }

  #
  # ====== MC PLOTS GENERATION ======
  #

  cat( "\nProcessing experiments and generating reports...\n")

  #
  # ------ GDP ------
  #

  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[k]] <- list( log0( Adata[[k]]$Q[ TmaskPlot ] ) )
    # minimum and maximum MC runs
    min[[k]] <- list( log0( mdata[[k]]$Q[ TmaskPlot ] ) )
    max[[k]] <- list( log0( Mdata[[k]]$Q[ TmaskPlot ] ) )
    # MC confidence interval
    lo[[k]] <- list( log0( Adata[[k]]$Q[ TmaskPlot ] -
                           qnorm(1 - (1 - CI) / 2) * Sdata[[k]]$Q[ TmaskPlot ] / sqrt( nSize ) ) )
    hi[[k]] <- list( log0( Adata[[k]]$Q[ TmaskPlot ] +
                           qnorm(1 - (1 - CI) / 2) * Sdata[[k]]$Q[ TmaskPlot ] / sqrt( nSize ) ) )
  }

  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Log output",
              tit = "Total output (GDP)",
              subtit = paste( "MC runs =", nSize ) )

  #
  # ------ Number of islands ------
  #

  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[k]] <- list( Adata[[k]]$l[ TmaskPlot ] )
    # minimum and maximum MC runs
    min[[k]] <- list( mdata[[k]]$l[ TmaskPlot ] )
    max[[k]] <- list( Mdata[[k]]$l[ TmaskPlot ] )
    # MC confidence interval
    lo[[k]] <- list( Adata[[k]]$l[ TmaskPlot ] -
                       qnorm(1 - (1 - CI) / 2) * Sdata[[k]]$l[ TmaskPlot ]  / sqrt( nSize ) )
    hi[[k]] <- list( Adata[[k]]$l[ TmaskPlot ] +
                       qnorm(1 - (1 - CI) / 2) * Sdata[[k]]$l[ TmaskPlot ]  / sqrt( nSize ) )
  }

  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Number",
              tit = "Known islands",
              subtit = paste( "MC runs =", nSize ) )

  #
  # ------ Number of miners ------
  #

  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[k]] <- list( Adata[[k]]$m[ TmaskPlot ] )
    # minimum and maximum MC runs
    min[[k]] <- list( mdata[[k]]$m[ TmaskPlot ] )
    max[[k]] <- list( Mdata[[k]]$m[ TmaskPlot ] )
    # MC confidence interval
    lo[[k]] <- list( Adata[[k]]$m[ TmaskPlot ] -
                     qnorm(1 - (1 - CI) / 2) * Sdata[[k]]$m[ TmaskPlot ]  / sqrt( nSize ) )
    hi[[k]] <- list( Adata[[k]]$m[ TmaskPlot ] +
                     qnorm(1 - (1 - CI) / 2) * Sdata[[k]]$m[ TmaskPlot ]  / sqrt( nSize ) )
  }

  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Number of miners",
              tit = "Active miners",
              subtit = paste( "MC runs =", nSize ) )

  #
  # ------ Number of islands ------
  #

  exps <- min <- max <- lo <- hi <- list( )
  # select data to plot
  for( k in 1 : nExp ){
    # MC averages
    exps[[k]] <- list( Adata[[k]]$l[ TmaskPlot ],
                       Adata[[k]]$J[ TmaskPlot ] )
    # minimum and maximum MC runs
    min[[k]] <- list( mdata[[k]]$l[ TmaskPlot ],
                      mdata[[k]]$J[ TmaskPlot ] )
    max[[k]] <- list( Mdata[[k]]$l[ TmaskPlot ],
                      Mdata[[k]]$J[ TmaskPlot ] )
    # MC confidence interval
    lo[[k]] <- list( Adata[[k]]$l[ TmaskPlot ] -
                     qnorm(1 - (1 - CI) / 2) * Sdata[[k]]$l[ TmaskPlot ] / sqrt( nSize ),
                     Adata[[k]]$J[ TmaskPlot ] -
                     qnorm(1 - (1 - CI) / 2) * Sdata[[k]]$J[ TmaskPlot ]  / sqrt( nSize ) )
    hi[[k]] <- list( Adata[[k]]$l[ TmaskPlot ] +
                     qnorm(1 - (1 - CI) / 2) * Sdata[[k]]$l[ TmaskPlot ] / sqrt( nSize ),
                     Adata[[k]]$J[ TmaskPlot ] +
                     qnorm(1 - (1 - CI) / 2) * Sdata[[k]]$J[ TmaskPlot ]  / sqrt( nSize ) )
  }

  plot_lists( exps, min, max, lo, hi, leg = legends, mrk = transMk, col = colors,
              lty = lTypes, xlab = "Time", ylab = "Number of islands",
              tit = "Number of islands",
              subtit = paste( "MC runs =", nSize ),
              leg2 = c( "Known", "Total" ) )

  #
  # ====== STATISTICS GENERATION ======
  #

  # Create vectors and lists to hold the Monte Carlo results
  Q.growth <- m.growth <- l.growth <- J.growth <- vector( mode = "numeric", length = nSize )
  Q.sd <- m.sd <- l.sd <- J.sd <- vector( mode = "numeric", length = nSize )
  Q.adf <- m.adf <- l.adf <- J.adf <- list( )
  Q.bpf.adf <- m.bpf.adf <- l.bpf.adf <- J.bpf.adf <- list( )
  Q.gdp <- m.gdp <- l.gdp <- J.gdp <- list( )
  Q.Q <- m.Q <- l.Q <- J.Q <- list( )
  Q.Q.pval <- m.Q.pval <- l.Q.pval <- J.Q.pval <- vector( mode = "numeric", length = nExp )

  for(k in 1 : nExp){ # Experiment k

    #
    # ---- Bandpass filtered GDP, miners and islands ----
    #

    plot_bpf( list( log0( Adata[[k]]$Q ), log0( Adata[[k]]$m ),
                    log0( Adata[[k]]$l ), log0( Adata[[k]]$J ) ),
              pl = lowP, pu = highP, nfix = bpfK, mask = TmaskPlot,
              mrk = transMk, col = colors, lty = lTypes,
              leg = c("Q", "m", "l", "J" ),
              xlab = "Time", ylab = "Filtered series",
              tit = paste( "GDP cycles (", legends[ k ], ")" ),
              subtit = paste( "( Baxter-King bandpass-filtered, low = 6Q / high = 32Q / order = 12 / MC runs =",
                              nSize, ")" ) )

    #
    # ==== Statistics computation for tables ====
    #

    for(j in 1 : nSize){  # Execute for every Monte Carlo run

      # Monte carlo average growth rates
      Q.growth[j] <- (log0(mcData[[k]][nTsteps,"Q",j]) -
                        log0(mcData[[k]][warmUpStat + 1,"Q",j])) / ( nTsteps - warmUpStat )
      m.growth[j] <- (mcData[[k]][nTsteps,"m",j] -
                        mcData[[k]][warmUpStat + 1,"m",j]) / ( nTsteps - warmUpStat )
      l.growth[j] <- (mcData[[k]][nTsteps,"l",j] -
                        mcData[[k]][warmUpStat + 1,"l",j]) / ( nTsteps - warmUpStat )
      J.growth[j] <- (mcData[[k]][nTsteps,"J",j] -
                        mcData[[k]][warmUpStat + 1,"J",j]) / ( nTsteps - warmUpStat )

      # Apply Baxter-King filter to the series
      Q.bpf <- bkfilter(log0(mcData[[k]][ TmaskStat,"Q",j]), pl = lowP, pu = highP, nfix = bpfK)
      m.bpf <- bkfilter(mcData[[k]][ TmaskStat,"m",j], pl = lowP, pu = highP, nfix = bpfK)
      l.bpf <- bkfilter(mcData[[k]][ TmaskStat,"l",j], pl = lowP, pu = highP, nfix = bpfK)
      J.bpf <- bkfilter(mcData[[k]][ TmaskStat,"J",j], pl = lowP, pu = highP, nfix = bpfK)

      # Augmented Dickey-Fuller tests for unit roots
      Q.adf[[j]] <- Q.bpf.adf[[j]] <- m.adf[[j]] <- m.bpf.adf[[j]] <- l.adf[[j]] <-
        l.bpf.adf[[j]] <- J.adf[[j]] <- J.bpf.adf[[j]] <- NA
      try( Q.adf[[j]] <- adf.test(log0(mcData[[k]][ TmaskStat,"Q",j])), silent = TRUE )
      try( Q.bpf.adf[[j]] <- adf.test(Q.bpf$cycle[ TmaskBpf, 1 ]), silent = TRUE )
      try( m.adf[[j]] <- adf.test(mcData[[k]][ TmaskStat,"m",j]), silent = TRUE )
      try( m.bpf.adf[[j]] <- adf.test(m.bpf$cycle[ TmaskBpf, 1 ]), silent = TRUE )
      try( l.adf[[j]] <- adf.test(mcData[[k]][ TmaskStat,"l",j]), silent = TRUE )
      try( l.bpf.adf[[j]] <- adf.test(l.bpf$cycle[ TmaskBpf, 1 ]), silent = TRUE )
      try( J.adf[[j]] <- adf.test(mcData[[k]][ TmaskStat,"J",j]), silent = TRUE )
      try( J.bpf.adf[[j]] <- adf.test(J.bpf$cycle[ TmaskBpf, 1 ]), silent = TRUE )

      # Standard deviations of filtered series
      Q.sd[j] <- sd(Q.bpf$cycle[ TmaskBpf, 1 ])
      m.sd[j] <- sd(m.bpf$cycle[ TmaskBpf, 1 ])
      l.sd[j] <- sd(l.bpf$cycle[ TmaskBpf, 1 ])
      J.sd[j] <- sd(J.bpf$cycle[ TmaskBpf, 1 ])

      # Build the correlation structures
      Q.Q[[j]] <- ccf( Q.bpf$cycle[ TmaskBpf, 1 ],
                       Q.bpf$cycle[ TmaskBpf, 1 ],
                       lag.max = lags, plot = FALSE, na.action = na.pass )
      m.Q[[j]] <- ccf( Q.bpf$cycle[ TmaskBpf, 1 ],
                       m.bpf$cycle[ TmaskBpf, 1 ],
                       lag.max = lags, plot = FALSE, na.action = na.pass )
      l.Q[[j]] <- ccf( Q.bpf$cycle[ TmaskBpf, 1 ],
                       l.bpf$cycle[ TmaskBpf, 1 ],
                       lag.max = lags, plot = FALSE, na.action = na.pass )
      J.Q[[j]] <- ccf( Q.bpf$cycle[ TmaskBpf, 1 ],
                       J.bpf$cycle[ TmaskBpf, 1 ],
                       lag.max = lags, plot = FALSE, na.action = na.pass )
    }

    # Applies t test to the mean lag results to test their significance (H0: lag < critCorr)
    for(i in 1 : (2 * lags + 1)){ #do for all lags
      if(i != lags + 1)  # don't try to compute autocorrelation at lag 0
        Q.Q.pval[i] <- t.test0(abs(unname(sapply(Q.Q, `[[`, "acf"))[i,]), critCorr, CI)
      m.Q.pval[i] <- t.test0(abs(unname(sapply(m.Q, `[[`, "acf"))[i,]), critCorr, CI)
      l.Q.pval[i] <- t.test0(abs(unname(sapply(l.Q, `[[`, "acf"))[i,]), critCorr, CI)
      J.Q.pval[i] <- t.test0(abs(unname(sapply(J.Q, `[[`, "acf"))[i,]), critCorr, CI)
    }

    #
    # ---- Summary statistics table (averages, standard errors and p-values) ----
    #

    key.stats <- matrix(c(mean(Q.growth, na.rm = T), mean(m.growth, na.rm = T),
                          mean(l.growth, na.rm = T),

                          sd(Q.growth, na.rm = T) / sqrt(nSize), sd(m.growth, na.rm = T) / sqrt(nSize),
                          sd(l.growth, na.rm = T) / sqrt(nSize),

                          mean(unname(sapply(Q.adf, `[[`, "statistic")), na.rm = T),
                          mean(unname(sapply(m.adf, `[[`, "statistic")), na.rm = T),
                          mean(unname(sapply(l.adf, `[[`, "statistic")), na.rm = T),

                          sd(unname(sapply(Q.adf, `[[`, "statistic")), na.rm = T) / sqrt(nSize),
                          sd(unname(sapply(m.adf, `[[`, "statistic")), na.rm = T) / sqrt(nSize),
                          sd(unname(sapply(l.adf, `[[`, "statistic")), na.rm = T) / sqrt(nSize),

                          mean(unname(sapply(Q.adf, `[[`, "p.value")), na.rm = T),
                          mean(unname(sapply(m.adf, `[[`, "p.value")), na.rm = T),
                          mean(unname(sapply(l.adf, `[[`, "p.value")), na.rm = T),

                          sd(unname(sapply(Q.adf, `[[`, "p.value")), na.rm = T) / sqrt(nSize),
                          sd(unname(sapply(m.adf, `[[`, "p.value")), na.rm = T) / sqrt(nSize),
                          sd(unname(sapply(l.adf, `[[`, "p.value")), na.rm = T) / sqrt(nSize),

                          mean(unname(sapply(Q.bpf.adf, `[[`, "statistic")), na.rm = T),
                          mean(unname(sapply(m.bpf.adf, `[[`, "statistic")), na.rm = T),
                          mean(unname(sapply(l.bpf.adf, `[[`, "statistic")), na.rm = T),

                          sd(unname(sapply(Q.bpf.adf, `[[`, "statistic")), na.rm = T) / sqrt(nSize),
                          sd(unname(sapply(m.bpf.adf, `[[`, "statistic")), na.rm = T) / sqrt(nSize),
                          sd(unname(sapply(l.bpf.adf, `[[`, "statistic")), na.rm = T) / sqrt(nSize),

                          mean(unname(sapply(Q.bpf.adf, `[[`, "p.value")), na.rm = T),
                          mean(unname(sapply(m.bpf.adf, `[[`, "p.value")), na.rm = T),
                          mean(unname(sapply(l.bpf.adf, `[[`, "p.value")), na.rm = T),

                          sd(unname(sapply(Q.bpf.adf, `[[`, "p.value"))) / sqrt(nSize),
                          sd(unname(sapply(m.bpf.adf, `[[`, "p.value"))) / sqrt(nSize),
                          sd(unname(sapply(l.bpf.adf, `[[`, "p.value"))) / sqrt(nSize),

                          mean(Q.sd, na.rm = T), mean(m.sd, na.rm = T),
                          mean(l.sd, na.rm = T),

                          sd(Q.sd, na.rm = T) / sqrt(nSize), sd(m.sd, na.rm = T) / sqrt(nSize),
                          sd(l.sd, na.rm = T) / sqrt(nSize),

                          1, mean(m.sd, na.rm = T) / mean(Q.sd, na.rm = T),
                          mean(l.sd, na.rm = T) / mean(Q.sd, na.rm = T) ),

                        ncol = 3, byrow = T)
    colnames( key.stats ) <- c( "Q (output)", "m (miners)", "l (known islands)" )
    rownames( key.stats ) <- c( "avg. growth rate", " (s.e.)",
                                "ADF test (logs)", " (s.e.)", " (p-val.)", " (s.e.)",
                                "ADF test (bpf)", " (s.e.)", " (p-val.)", " (s.e.)",
                                " s.d. (bpf)", " (s.e.)",
                                " relative s.d. (Q)" )

    textplot( formatC( key.stats, digits = sDigits, format = "g" ), cmar = 2, cex = 1.0 )
    title <- paste( "Key statistics and unit roots tests for cycles (", legends[ k ], ")" )
    subTitle <- paste( eval( bquote(paste0( "( bpf: Baxter-King bandpass-filtered series, low = ", .(lowP),
                                            "Q / high = ", .(highP), "Q / order = ", .(bpfK),
                                            " / MC runs = ", .(nSize), " / period = ",
                                            .(warmUpStat), " - ", .(nTsteps), " )" ) ) ),
                       eval( bquote( paste0( "( ADF test H0: there are unit roots / non-stationary at ",
                                             .( (1 - CI) * 100),"% level", " )" ) ) ), sep ="\n" )
    title( main = title, sub = subTitle )

    #
    # ---- Correlation structure tables (lags, standard errors and p-values) ----
    #

    corr.struct.1 <- matrix(c(colMeans(t(unname(sapply(Q.Q, `[[`, "acf"))), na.rm = T),
                              colSds(t(unname(sapply(Q.Q, `[[`, "acf"))), na.rm = T) / sqrt(nSize),
                              Q.Q.pval,

                              colMeans(t(unname(sapply(m.Q, `[[`, "acf"))), na.rm = T),
                              colSds(t(unname(sapply(m.Q, `[[`, "acf"))), na.rm = T) / sqrt(nSize),
                              m.Q.pval,

                              colMeans(t(unname(sapply(l.Q, `[[`, "acf"))), na.rm = T),
                              colSds(t(unname(sapply(l.Q, `[[`, "acf"))), na.rm = T) / sqrt(nSize),
                              l.Q.pval,

                              colMeans(t(unname(sapply(J.Q, `[[`, "acf"))), na.rm = T),
                              colSds(t(unname(sapply(J.Q, `[[`, "acf"))), na.rm = T) / sqrt(nSize),
                              J.Q.pval ),

                            ncol = 2 * lags + 1, byrow = T)
    colnames( corr.struct.1 ) <- Q.Q[[1]]$lag
    rownames( corr.struct.1 ) <- c( "Q (output)", " (s.e.)", " (p-val.)",
                                    "m (miners)", " (s.e.)", " (p-val.)",
                                    "l (known islands)", " (s.e.)", " (p-val.)",
                                    "J (islands)", " (s.e.)", " (p-val.)" )

    title <- paste( "Correlation structure for Q (", legends[ k ], ")" )
    subTitle <- paste( eval( bquote( paste0( "( non-rate/ratio series are Baxter-King bandpass-filtered, low = ",
                                             .(lowP), "Q / high = ", .(highP), "Q / order = ", .(bpfK),
                                             " / MC runs = ", .(nSize), " / period = ",
                                             .(warmUpStat), " - ", .(nTsteps), " )" ) ) ),
                       eval( bquote ( paste0( "( test H0: lag coefficient is not significant at ",
                                              .( (1 - CI) * 100),"% level", " )" ) ) ), sep ="\n" )

    textplot( formatC( corr.struct.1, digits = sDigits, format = "g" ), cmar = 1 )
    title( main = title, sub = subTitle )

    # Write tables to the disk as CSV files for Excel
    write.csv( key.stats, quote = FALSE,
               paste0( folder, "/", outDir, "/", repName, "", k, "_key_stats.csv") )
    write.csv( corr.struct.1, quote = FALSE,
               paste0( folder, "/", outDir, "/", repName, "", k, "_corr_struct.csv") )
  }

  #
  # ---- Aggregated variables stationarity and ergodicity tests ----
  #

  # select data to plot
  for( k in 1 : nExp ){

    statErgo <- ergod.test.lsd( mcData[[ k ]][ TmaskStat, , ], signif = 1 - CI,
                                vars = statErgo.vars )

    # plot table
    textplot( statErgo, cmar = 2, rmar = 0.5, cex = 1.0 )
    title( main = paste( "Stationarity, i.i.d. and ergodicity tests (", legends[ k ], ")" ),
           sub = paste( "( average p-values for testing H0 and rate of rejection of H0 / MC runs =", nSize, "/ period =", warmUpStat, "-", nTsteps, ")\n ( ADF/PP H0: non-stationary, KPSS H0: stationary, BDS H0: i.i.d., KS/AD/WW H0: ergodic )\n( significance =",
                        1 - CI, ")" ) )

    # write to disk
    write.csv( statErgo, quote = FALSE,
               paste0( folder, "/", outDir, "/", repName, "", k, "_ergod_tests.csv" ) )
  }

  #
  # ---- GDP long-term recovery times table ----
  #

  rec.stats <- array( dim = c( 5, nSize, nExp ) )
  rec.stats.mc <- array( dim = c( 2 * dim( rec.stats )[ 1 ], nExp ) )
  rec.cases <- array( data = 0, dim = c( 2 , nExp ) )
  rec.starts <- rec.times <- array( list( ), dim = c( nSize, nExp ) )

  for( k in 1 : nExp ) {      # for each experiment
    for( j in 1 : nSize ) {   # for each MC case

      # calculate long-term ex post growth rate trend
      growthTrend <- hpfilter( mcData[[ k ]][ , "g", j ], smoothing )$trend[ , 1 ]

      # Calculate recovery statistics for GDP to the pre-crisis trend
      recovery <- FALSE
      recStart <- recTime <- recDepth <- recCost <- vector( mode = "numeric" )
      preCrisisQ <- gTrend <- timeCrisis <- depthCrisis <- costCrisis <- NA
      for( i in TmaskStat ) {
        if( recovery ) {                                              # in a recovery?
          preCrisisQ <- preCrisisQ + gTrend                       # update pre crisis trend
          if( log( mcData[[ k ]][ i, "Q", j ] ) >= preCrisisQ ) { # recovery is over?
            recovery <- FALSE
            if( i - timeCrisis > crisisLen ) { # record only crisis lasting longer than x periods
              recStart <- append( recStart, timeCrisis )
              recTime <- append( recTime, i - timeCrisis )
              recDepth <- append( recDepth, depthCrisis )
              recCost <- append( recCost, costCrisis )
            }
          } else {                                                    # recovery not over
            gap <- preCrisisQ - log( mcData[[ k ]][ i, "Q", j ] ) # update recovery statistics
            depthCrisis <- max( depthCrisis, gap )
            costCrisis <- costCrisis + gap
          }
        }
        if( ! recovery && mcData[[ k ]][ i, "g", j ] < crisisTh && i > crisisPre ) {
          recovery <- TRUE
          timeCrisis <- i
          depthCrisis <- costCrisis <- 0
          preCrisisQ <- log( mean( mcData[[ k ]][ ( i - crisisPre ) : ( i - 1 ), "Q", j ] ) )
          gTrend <- mean( growthTrend[ ( i - crisisPre ) : ( i - 1 ) ], na.rm = TRUE )
          depthCrisis <- costCrisis <- preCrisisQ - log( mcData[[ k ]][ i, "Q", j ] )
        }
      }

      # prepare to consolidate MC results
      if( length( recTime ) > 0 ) {
        rec.stats[ , j, k ] <- c( length( recTime ), mean( recTime ), sd( recTime ),
                                  mean( recDepth ), mean( recCost ) )
        rec.times[[ j, k ]] <- recTime
        rec.starts[[ j, k ]] <- recStart

        # record case with more average costs crises
        rankCr <- mean( recCost )
        if( rankCr > rec.cases[ 2, k ] ) {
          rec.cases[ 1, k ] <- j
          rec.cases[ 2, k ] <- rankCr
        }

      } else {
        rec.stats[ , j, k ] <- c( rep( NA, dim( rec.stats )[ 1 ] ) )
        rec.times[[ j, k ]] <- NA
        rec.starts[[ j, k ]] <- NA
      }
    }

    # Calculate average recovery statistics for the experiment
    for( i in 1 : ( dim( rec.stats )[ 1 ] ) ) {
      rec.stats.mc[ 2 * i - 1, k ] <- mean( rec.stats[ i, , k ], na.rm = TRUE )
      rec.stats.mc[ 2 * i, k ] <- sd( rec.stats[ i, , k ], na.rm = TRUE ) / sqrt( nSize )
    }
  }

  recovery <- matrix( c( rec.stats.mc[ 1, 1 ], rec.stats.mc[ 2, 1 ],
                         rec.stats.mc[ 3, 1 ], rec.stats.mc[ 4, 1 ],
                         rec.stats.mc[ 5, 1 ], rec.stats.mc[ 6, 1 ],
                         rec.stats.mc[ 7, 1 ], rec.stats.mc[ 8, 1 ],
                         rec.stats.mc[ 9, 1 ], rec.stats.mc[ 10, 1 ] ) )
  recovery.labels <- legends[ 1 ]

  if( nExp > 1 ){
    # Create 2D table
    for( k in 2 : nExp ) {
      recovery <- cbind( recovery, c( rec.stats.mc[ 1, k ], rec.stats.mc[ 2, k ],
                                      rec.stats.mc[ 3, k ], rec.stats.mc[ 4, k ],
                                      rec.stats.mc[ 5, k ], rec.stats.mc[ 6, k ],
                                      rec.stats.mc[ 7, k ], rec.stats.mc[ 8, k ],
                                      rec.stats.mc[ 9, k ], rec.stats.mc[ 10, k ]) )
      recovery.labels <- append( recovery.labels, legends[ k ] )
    }
  }

  colnames( recovery ) <- recovery.labels
  rownames( recovery ) <- c( "Avg. number of crisis", "(s.e.)", "Avg. recovery periods",
                             "(s.e.)", "Std. dev. rec. periods", "(s.e.)",
                             "Avg. crisis peak", "(s.e.)", "Avg. crisis loss", "(s.e.)" )

  textplot( formatC( recovery, digits = sDigits, format = "g" ), cmar = 1 )
  title <- paste( "GDP long-term trend recovery after crisis ( all experiments )" )
  subTitle <- paste( "( MC standard error in parentheses / MC runs =",
                     nSize, "/ period =", warmUpStat, "-", nTsteps, ")\n",
                     "( crisis definition: GDP contraction larger than", abs( crisisTh ) * 100,
                     "%, for more than", crisisLen, "periods )\n",
                     "( pre-crisis trend estimated using an HP filter with smoothing parameter =",
                     smoothing, "averaged for", crisisPre, "periods )" )
  title( main = title, sub = subTitle )

  # Write table to the disk as CSV files for Excel
  write.csv( recovery, quote = FALSE,
             paste0( folder, "/", outDir, "/", repName, "", k, "_GDP_recovery.csv" ) )

  #
  # ---- GDP recovery sample run plot ----
  #

  for( k in 1 : nExp ) {                                # for each experiment
    if( crisisRun == 0 )
      j <- rec.cases[ 1, k ]                            # auto-selected MC run
    else
      j <- crisisRun                                    # or use the user's selected

    if( j == 0 )                                        # no crisis?
      j <- 1                                            # pick first

    plot_recovery( mcData[[ k ]][ , "Q", j ], mcData[[ k ]][ , "g", j ],
                   mask = TmaskStat, warm = warmUpPlot, mrk = transMk,
                   strt = rec.starts[[ j, k ]], dur = rec.times[[ j, k ]],
                   per = crisisPre, xlab = "Time", ylab = "Log Q",
                   tit = paste( "GDP long-term trend recovery after crisis sample (", legends[ k ], ")" ),
                   subtit = paste( "( dashed line: pre-crisis trend / gray boxes: trend recovey period / MC case =",
                                   j, "/ period =", warmUpStat, "-", nTsteps, ")" ) )
  }

  #
  # ======= COMPARISON OF EXPERIMENTS =======
  #

  cat( "\nRunning experiments comparison...\n")

  numStats <- 99
  statsTb <- array( dim = c( numStats, 4, nExp ) )
  statsBp <- array( dim = c( numStats, 5, nExp ) )
  n <- array( dim = c( numStats, nExp ) )
  conf <- array( dim = c( numStats, 2, nExp ) )
  out <- array( list( ), dim = c( numStats, nExp ) )
  names <- units <- list( )

  # ---- Collect the data for each experiment ----

  for( k in 1 : nExp ) {
    stat <- 0
    temp <- matrix(nrow = TmaxStat, ncol = nSize)

    stat <- stat + 1
    names[[ stat ]] <- "GDP growth"
    units[[ stat ]] <- "Average GDP growth rate"
    # Calculates periodic GDP growth rates for each MC series
    for(j in 1 : nSize)
      for(i in TmaskStat)
        if(i == 1) {
          temp[i - warmUpStat,j] <- 0
        }else
          temp[i - warmUpStat,j] <- (log0(mcData[[k]][i,"Q",j]) -
                                       log0(mcData[[k]][i - 1,"Q",j]))
    # Remove +/-infinite values and replace by +/-1
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)])
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out

    stat <- stat + 1
    names[[ stat ]] <- "Volatility of GDP growth"
    units[[ stat ]] <- "Standard deviation of GDP growth rate"
    # Calculates periodic GDP growth rates for each MC series
    for(j in 1 : nSize)
      for(i in TmaskStat)
        if(i == 1) {
          temp[i - warmUpStat,j] <- 0
        } else
          temp[i - warmUpStat,j] <- (log0(mcData[[k]][i,"Q",j]) -
                                       log0(mcData[[k]][i - 1,"Q",j]))
    # Remove +/-infinite values and replace by +/-1
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)])
    x <- colSds( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out

    stat <- stat + 1
    names[[ stat ]] <- "Likelihood of GDP crises"
    units[[ stat ]] <- "Likelihood (probability) of GDP crises"
    # Mark crises periods (= 1) when GDP growth is less than -3%
    for(j in 1 : nSize){
      for(i in TmaskStat){
        if(i == 1){
          temp[i - warmUpStat,j] <- 0
        }
        else{
          if(log0(mcData[[k]][i,"Q",j]) -
             log0(mcData[[k]][i - 1,"Q",j]) < -0.03){
            temp[i - warmUpStat,j] <- 1
          }
          else{
            temp[i - warmUpStat,j] <- 0
          }
        }
      }
    }
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out

    stat <- stat + 1
    names[[ stat ]] <- "Recovery from GDP crises"
    units[[ stat ]] <- "Average GDP crises recovery period"
    x <- na.remove( rec.stats[ 2, , k ] )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out

    stat <- stat + 1
    names[[ stat ]] <- "Losses from GDP crises"
    units[[ stat ]] <- "Average GDP losses during crises recovery"
    x <- na.remove( rec.stats[ 5, , k ] )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out

    stat <- stat + 1
    names[[ stat ]] <- "Active miners"
    units[[ stat ]] <- "Number of miners"
    temp <- mcData[[k]][ TmaskStat, "m", ]
    temp[ ! is.finite( temp ) ] <- NA
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out

    stat <- stat + 1
    names[[ stat ]] <- "Known islands"
    units[[ stat ]] <- "Number of islands"
    temp <- mcData[[k]][ TmaskStat, "l", ]
    temp[ ! is.finite( temp ) ] <- NA
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out

    stat <- stat + 1
    names[[ stat ]] <- "Total islands"
    units[[ stat ]] <- "Number of islands"
    temp <- mcData[[k]][ TmaskStat, "J", ]
    temp[ ! is.finite( temp ) ] <- NA
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out

  }

  # remove unused stats space
  numStats <- stat
  stat <- stat + 1
  statsTb <- statsTb[ - ( stat : 99 ), , , drop = FALSE ]
  statsBp <- statsBp[ - ( stat : 99 ), , , drop = FALSE ]
  n <- n[ - ( stat : 99 ), , drop = FALSE ]
  conf <- conf[ - ( stat : 99 ), , , drop = FALSE ]
  out <- out[ - ( stat : 99 ), , drop = FALSE ]
  rm( temp, x )


  # ---- Build experiments statistics table and performance comparison chart ----

  table.stats <- statsTb[ , , 1 ]
  table.names <- c( "Avg[1]", "SD[1]", "Min[1]", "Max[1]" )
  perf.comp <- statsTb[ , 1, 1 ]
  perf.names <- c( "Baseline[1]" )

  # Print whisker plots for each statistics

  for( stat in 1 : numStats ) {

    # find max/mins for all experiments
    LowLim <- Inf
    upLim <- -Inf
    for( k in 1 : nExp ) {
      if( conf[ stat, 1, k ] < LowLim )
        lowLim <- conf[ stat, 1, k ]
      if( conf[ stat, 2, k ] > upLim )
        upLim <- conf[ stat, 2, k ]
    }
    upLim <- upLim + ( upLim - lowLim )
    lowLim <- lowLim - ( upLim - lowLim )

    # build the outliers vectors
    outVal <- outGrp <- vector( "numeric" )
    for( k in 1 : nExp ) {
      if( length( out[[ stat, k ]] ) == 0 )
        next
      outliers <- vector( "numeric" )
      for( i in 1 : length( out[[ stat, k ]] ) ) {
        if( out[[ stat, k ]][ i ] < upLim &&
            out[[ stat, k ]][ i ] > lowLim )
          outliers <- append( outliers, out[[ stat, k ]][ i ] )
      }
      if( length( outliers ) > 0 ) {
        outVal <- append( outVal, outliers )
        outGrp <- append( outGrp, rep( k, length( outliers ) ) )
      }
    }

    listBp <- list( stats = statsBp[ stat, , ], n = n[ stat, ], conf = conf[ stat, , ],
                    out = outVal, group = outGrp, names = legends )
    title <- names[[ stat ]]
    subTitle <- as.expression(bquote(paste( "( bar: median / box: 2nd-3rd quartile / whiskers: max-min / points: outliers / MC runs = ",
                                            .(nSize), " / period = ", .(warmUpStat), " - ", .(nTsteps), " )" ) ) )
    tryCatch( bxp( listBp, range = bPlotCoef, notch = bPlotNotc, main = title,
                   sub = subTitle, ylab = units[[ stat ]] ),
              error = function( e ) {
                warning( "In boxplot (bxp): problem while plotting: ", title, "\n\n" )
                textplot( paste( "Plot for <", title, "> failed." ) )
              } )
  }

  if( nExp > 1 ){

    # Create 2D stats table and performance comparison table

    for(k in 2 : nExp){

      # Stats table
      table.stats <- cbind( table.stats, statsTb[ , , k ] )
      table.names <- cbind( table.names, c( paste0( "Avg[", k, "]" ),
                                            paste0( "SD[", k, "]" ),
                                            paste0( "Min[", k, "]" ),
                                            paste0( "Max[", k, "]" ) ) )

      # Performance comparison table
      perf.comp <- cbind( perf.comp, statsTb[ , 1, k ] / statsTb[ , 1, 1 ] )
      t <- ( statsTb[ , 1, k ] - statsTb[ , 1, 1 ] ) /
        sqrt( ( statsTb[ , 2, k ] ^ 2 + statsTb[ , 2, 1 ] ^ 2 ) / nSize )
      df <- floor( ( ( statsTb[ , 2, k ] ^ 2 + statsTb[ , 2, 1 ] ^ 2 ) / nSize ) ^ 2 /
                     ( ( 1 / ( nSize - 1 ) ) * ( ( statsTb[ , 2, k ] ^ 2 / nSize ) ^ 2 +
                                                   ( statsTb[ , 2, 1 ] ^ 2 / nSize ) ^ 2 ) ) )
      pval <- 2 * pt( - abs ( t ), df )
      perf.comp <- cbind( perf.comp, pval )
      perf.names <- cbind( perf.names, t( c( paste0( "Ratio[", k, "]" ),
                                             paste0( "p-val[", k, "]" ) ) ) )
    }
  }

  # Print experiments table
  colnames( table.stats ) <- table.names
  rownames( table.stats ) <- names

  textplot( formatC( table.stats, digits = sDigits, format = "g" ), cmar = 1 )
  title <- paste( "Monte Carlo descriptive statistics ( all experiments )" )
  subTitle <- paste( "( numbers in brackets indicate the experiment number / MC runs =",
                     nSize, "/ period =", warmUpStat, "-", nTsteps, ")" )
  title( main = title, sub = subTitle )
  mtext( legendList, side = 1, line = -2, outer = TRUE )

  # Write table to the disk as CSV files for Excel
  write.csv( table.stats, quote = FALSE,
             paste0( folder, "/", outDir, "/", repName, "", k, "_exps_stats.csv") )

  if( nExp > 1 ) {

    # Experiments performance comparison table

    colnames( perf.comp ) <- perf.names
    rownames( perf.comp ) <- names

    textplot( formatC( perf.comp, digits = sDigits, format = "g" ), cmar = 1 )
    title <- paste( "Performance comparison ( all experiments )" )
    subTitle <- paste( "( numbers in brackets indicate the experiment number / H0: no difference with baseline / MC runs =",
                       nSize, "/ period =", warmUpStat, "-", nTsteps, ")" )
    title( main = title, sub = subTitle )
    mtext( legendList, side = 1, line = -2, outer = TRUE )

    # Write table to the disk as CSV files for Excel
    write.csv( perf.comp, quote = FALSE,
               paste0( folder, "/", outDir, "/", repName, "", k, "_perf_comp.csv") )
  }

  #
  # ---- Produce crisis recoveries report ----
  #

  if( crisesPlt ) {

    cat( "\nGenerating crises report...\n")

    # Close main plot file
    dev.off()

    # Select type of output
    if( raster ){
      # Open PNG (bitmap) files for output
      png( paste0( folder, "/", outDir, "/", repName, "_crisis_%d.png" ),
           width = plotW, height = plotH, units = "in", res = res )

    } else {
      # Open PDF plot file for output
      pdf(paste0( folder, "/", outDir, "/", repName, "_crisis.pdf" ),
          width = plotW, height = plotH )
      par( mfrow = c ( plotRows, plotCols ) )             # define plots per page
    }

    # Plot all GDP recovery runs

    for( k in 1 : nExp )                                  # for each experiment
      for( j in 1 : nSize )                               # for each MC case
        plot_recovery( mcData[[ k ]][ , "Q", j ], mcData[[ k ]][ , "g", j ],
                       mask = TmaskStat, warm = warmUpPlot, mrk = transMk,
                       strt = rec.starts[[ j, k ]], dur = rec.times[[ j, k ]],
                       per = crisisPre, xlab = "Time", ylab = "Log Q",
                       tit = paste( "GDP long-term trend recovery after crisis sample (", legends[ k ], ")" ),
                       subtit = paste( "( dashed line: pre-crisis trend / gray boxes: trend recovey period / MC case =",
                                       j, "/ period =", warmUpStat, "-", nTsteps, ")" ) )
  }

#******************************************************************
#
# ------------- Exception handling code (tryCatch) -------------
#
#******************************************************************

}, interrupt = function(ex) {
  cat("An interrupt was detected.\n")
  print(ex)
  textplot("Report incomplete due to interrupt.")
}, error = function(ex) {
  cat("An error was detected.\n")
  print(ex)
  textplot("Report incomplete due to processing error.")
}, finally = {
  options( warn = 0 )
  # Close PDF plot file
  dev.off()
})
