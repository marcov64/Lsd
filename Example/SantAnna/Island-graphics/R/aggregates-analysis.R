#******************************************************************
#
# -------------- Island Model: aggregates analysis ---------------
#
#   Written by Marcelo C. Pereira, University of Campinas
#
#   Copyright Marcelo C. Pereira
#   Distributed under the GNU General Public License
#
#   The default configuration assumes that the supplied LSD
#   simulation configurations (basename exp):
#     R/experiments/exp1.lsd
#     R/experiments/exp2.lsd
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

# ==== data load parameters ====

folder   <- "experiments"             # data files folder
baseName <- "exp"                     # data files base name ( same as .lsd file)
nExp <- 2                             # number of experiments
iniDrop <- 0                          # initial time steps to drop (0=none)
nKeep <- -1                           # number of time steps to keep (-1=all)
expVal <- c( "Low oportunity", "High oportunity" )# experiment names
logVars <- c( "Q" )                   # log variables to use
aggrVars <- append( logVars, c( "g", "l", "e", "i", "m", "J" ) )# variables to use


# ==== process LSD result files ====

library( LSDinterface, verbose = FALSE, quietly = TRUE )

# Get first experiment file names
if( nExp > 1 ) {
  myFiles <- list.files.lsd( folder, paste0( baseName, 1 ) )
} else
  myFiles <- list.files.lsd( folder, baseName )

# Get data set details
nSize  <- length( myFiles )   # Monte Carlo sample size
dimData <- info.dimensions.lsd( myFiles[ 1 ] )
nTsteps <- dimData$tSteps
nVar <- dimData$nVars
nTsteps <- nTsteps - iniDrop
if( nKeep > 0 ) nTsteps <- min( nKeep, nTsteps )

# Function to read one experiment data ( to run in parallel)
readExp <- function( exper ) {
  if( nExp > 1 )
    myFiles <- list.files.lsd( folder, paste0( baseName, exper ) )
  else
    myFiles <- list.files.lsd( folder, baseName )

  # read data from text files and format it as a 3D array with labels
  mc <- read.3d.lsd( myFiles, aggrVars, skip = iniDrop, nrows = nKeep )
  stats <- info.stats.lsd( mc ) # compute Monte Carlo averages, SD, max, min
  t <- as.integer( rownames( stats$avg ) )      # time reference
  A <- as.data.frame( cbind( t, stats$avg ) )   # averages
  S <- as.data.frame( cbind( t, stats$sd ) )    # standard deviations
  M <- as.data.frame( cbind( t, stats$max ) )   # maximums
  m <- as.data.frame( cbind( t, stats$min ) )   # minimums

  # Save temporary results to disk to save memory
  tmpFile <- paste0( folder, "/", baseName, "", exper, "_aggr.Rdata" )
  save( mc, A, S, M, m, file = tmpFile )
  return( tmpFile )
}

# load each experiment serially (can be easily made parallel)
tmpFiles <- lapply( 1 : nExp, readExp )

# organize data read from files
mcData <- Adata <- Sdata <- Mdata <- mdata <- list( )
for( k in 1 : nExp ) {                      # reallocate data in separate lists
  load( tmpFiles[[ k ]] )                   # pick data from disk
  file.remove( tmpFiles[[ k ]] )            # delete temporary file
  Adata[[ k ]] <- A
  Sdata[[ k ]] <- S
  Mdata[[ k ]] <- M
  mdata[[ k ]] <- m
  mcData[[ k ]] <- mc
  rm( mc )
}

# ==== analysis parameters ====

CI        <- 0.95   # desired confidence interval
warmUpPlot<- 0      # number of "warm-up" runs for plots
warmUpStat<- 200    # warm-up runs to evaluate all statistics
lowP      <- 6      # bandpass filter minimum period
highP     <- 32     # bandpass filter maximum period
bpfK      <- 12     # bandpass filter order
lags      <- 4      # lags to analyze
bPlotCoef <- 1.5    # boxplot whiskers extension from the box (0=extremes)
bPlotNotc <- FALSE  # use boxplot notches
sDigits   <- 4      # significant digits in tables

# variables to test for stationarity and ergodicity
statErgo.vars <- c( "Q", "g", "l", "e", "i", "m", "J" )

# colors assigned to each experiment's lines in graphics
colors <- c( "black", "blue", "green", "red", "orange", "brown" )

# cine types assigned to each experiment
lTypes <- c( "solid", "solid", "solid", "solid", "solid", "solid" )

# point types assigned to each experiment
pTypes <- c( 4, 4, 4, 4, 4, 4 )

# ====== External support functions & definitions ======

source( "support-functions.R" )

# Generate fancy labels & build labels list legend
legends <- vector( )
legendList <- "Experiments: "
for( k in 1 : nExp ) {
  if( is.na( expVal[ k ] ) || expVal[ k ] == "" )
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

# Calculates critical limits for significance (heroic assumptions!)
critCI <- qnorm( 1 - ( 1 - CI ) / 2 ) / sqrt( nSize )
critCorr <- qnorm( 1 - ( 1 - CI ) / 2 ) / sqrt( nTsteps )

# open PDF plot file for output
outFile <- paste0( folder, "/", baseName, "_aggr_plots.pdf" )
pdf( outFile, width = 12, height = 8 )
cat( paste( "\nSaving results to:", outFile, "\n" ) )

# ==== time plots ====

# ------ GDP ------

exps <- min <- max <- lo <- hi <- list( )
for( k in 1 : nExp ) {
  exps[[ k ]] <- list( log0( Adata[[ k ]]$Q[ TmaskPlot ] ) )
  min[[ k ]] <- list( log0( mdata[[ k ]]$Q[ TmaskPlot ] ) )
  max[[ k ]] <- list( log0( Mdata[[ k ]]$Q[ TmaskPlot ] ) )
  lo[[ k ]] <- list( log0( Adata[[ k ]]$Q[ TmaskPlot ] -
                         critCI * Sdata[[ k ]]$Q[ TmaskPlot ] ) )
  hi[[ k ]] <- list( log0( Adata[[ k ]]$Q[ TmaskPlot ] +
                         critCI * Sdata[[ k ]]$Q[ TmaskPlot ] ) )
}

plot_lists( exps, min, max, lo, hi, leg = legends, col = colors,
            lty = lTypes, xlab = "Time", ylab = "Log output",
            tit = "Total output (GDP)",
            subtit = paste( "MC runs =", nSize ) )

# ------ number of islands ------

exps <- min <- max <- lo <- hi <- list( )
for( k in 1 : nExp ) {
  exps[[ k ]] <- list( Adata[[ k ]]$l[ TmaskPlot ], Adata[[ k ]]$J[ TmaskPlot ] )
  min[[ k ]] <- list( mdata[[ k ]]$l[ TmaskPlot ], mdata[[ k ]]$J[ TmaskPlot ] )
  max[[ k ]] <- list( Mdata[[ k ]]$l[ TmaskPlot ], Mdata[[ k ]]$J[ TmaskPlot ] )
  lo[[ k ]] <- list( Adata[[ k ]]$l[ TmaskPlot ] -
                       critCI * Sdata[[ k ]]$l[ TmaskPlot ],
                     Adata[[ k ]]$J[ TmaskPlot ] -
                       critCI * Sdata[[ k ]]$J[ TmaskPlot ] )
  hi[[ k ]] <- list( Adata[[ k ]]$l[ TmaskPlot ] +
                       critCI * Sdata[[ k ]]$l[ TmaskPlot ],
                     Adata[[ k ]]$J[ TmaskPlot ] +
                       critCI * Sdata[[ k ]]$J[ TmaskPlot ] )
}

plot_lists( exps, min, max, lo, hi, leg = legends, col = colors,
            lty = lTypes, xlab = "Time", ylab = "Number of islands",
            tit = "Number of islands",
            subtit = paste( "MC runs =", nSize ),
            leg2 = c( "Known", "Total" ) )

# ------ number of explorers, imitators and miners ------

exps <- min <- max <- lo <- hi <- list( )
for( k in 1 : nExp ) {
  exps[[ k ]] <- list( Adata[[ k ]]$m[ TmaskPlot ], Adata[[ k ]]$i[ TmaskPlot ],
                       Adata[[ k ]]$e[ TmaskPlot ] )
  min[[ k ]] <- list( mdata[[ k ]]$m[ TmaskPlot ], mdata[[ k ]]$i[ TmaskPlot ],
                      mdata[[ k ]]$e[ TmaskPlot ] )
  max[[ k ]] <- list( Mdata[[ k ]]$m[ TmaskPlot ], Mdata[[ k ]]$i[ TmaskPlot ],
                      Mdata[[ k ]]$e[ TmaskPlot ] )
  lo[[ k ]] <- list( Adata[[ k ]]$m[ TmaskPlot ] -
                       critCI * Sdata[[ k ]]$m[ TmaskPlot ],
                     Adata[[ k ]]$i[ TmaskPlot ] -
                       critCI * Sdata[[ k ]]$i[ TmaskPlot ],
                     Adata[[ k ]]$e[ TmaskPlot ] -
                       critCI * Sdata[[ k ]]$e[ TmaskPlot ] )
  hi[[ k ]] <- list( Adata[[ k ]]$m[ TmaskPlot ] +
                       critCI * Sdata[[ k ]]$m[ TmaskPlot ],
                     Adata[[ k ]]$i[ TmaskPlot ] +
                       critCI * Sdata[[ k ]]$i[ TmaskPlot ],
                     Adata[[ k ]]$e[ TmaskPlot ] +
                       critCI * Sdata[[ k ]]$e[ TmaskPlot ] )
}

plot_lists( exps, min, max, lo, hi, leg = legends, col = colors,
            lty = lTypes, xlab = "Time", ylab = "Number of agents",
            tit = "Agent types",
            subtit = paste( "MC runs =", nSize ),
            leg2 = c( "Miners", "Imitators", "Explorers" ) )

# ====== statistics ======

Q.growth <- m.growth <- i.growth <- e.growth <- l.growth <- J.growth <-
  Q.sd <- m.sd <- i.sd <- e.sd <- l.sd <- J.sd <-
  vector( mode = "numeric", length = nSize )
Q.Q.pval <- m.Q.pval <- i.Q.pval <- e.Q.pval <- l.Q.pval <- J.Q.pval <-
  vector( mode = "numeric", length = nExp )
Q.adf <- m.adf <- i.adf <- e.adf <- l.adf <- J.adf <-
  Q.bpf.adf <- m.bpf.adf <- i.bpf.adf <- e.bpf.adf <- l.bpf.adf <- J.bpf.adf <-
  Q.gdp <- m.gdp <- i.gdp <- e.gdp <- l.gdp <- J.gdp <-
  Q.Q <- m.Q <- i.Q <- e.Q <- l.Q <- J.Q <- list( )

for( k in 1 : nExp ) { # handle each experiment separately

  # ---- bandpass filtered GDP, miners and islands ----

  plot_bpf( list( log0( Adata[[ k ]]$Q ), log0( Adata[[ k ]]$m ),
                  log0( Adata[[ k ]]$i ), log0( Adata[[ k ]]$e ) ),
            pl = lowP, pu = highP, nfix = bpfK, mask = TmaskPlot,
            col = colors, lty = lTypes, leg = c( "Q (output)", "m (miners)",
                                                 "i (imitators)",
                                                 "e (explorers)" ),
            xlab = "Time", ylab = "Filtered series",
            tit = paste( "GDP cycles (", legends[ k ], " )" ),
            subtit = paste( "( Baxter-King bandpass-filtered, low = 6Q / high = 32Q / order = 12 / MC runs =",
                            nSize, " )" ) )

  # ---- table statistics computation ----

  for( j in 1 : nSize ) {  # for every Monte Carlo run

    # Monte carlo average growth rates
    Q.growth[j ] <- ( log0( mcData[[ k ]][ nTsteps,"Q", j ] ) -
                      log0( mcData[[ k ]][ warmUpStat + 1,"Q", j ] ) ) /
      ( nTsteps - warmUpStat )
    m.growth[j ] <- ( mcData[[ k ]][ nTsteps,"m", j ] -
                        mcData[[ k ]][ warmUpStat + 1,"m", j ] ) /
      ( nTsteps - warmUpStat )
    i.growth[j ] <- ( mcData[[ k ]][ nTsteps,"i", j ] -
                        mcData[[ k ]][ warmUpStat + 1,"i", j ] ) /
      ( nTsteps - warmUpStat )
    e.growth[j ] <- ( mcData[[ k ]][ nTsteps,"e", j ] -
                        mcData[[ k ]][ warmUpStat + 1,"e", j ] ) /
      ( nTsteps - warmUpStat )
    l.growth[j ] <- ( mcData[[ k ]][ nTsteps,"l", j ] -
                      mcData[[ k ]][ warmUpStat + 1,"l", j ] ) /
      ( nTsteps - warmUpStat )
    J.growth[j ] <- ( mcData[[ k ]][ nTsteps,"J", j ] -
                      mcData[[ k ]][ warmUpStat + 1,"J", j ] ) /
      ( nTsteps - warmUpStat )

    # apply Baxter-King filter to the series
    Q.bpf <- bkfilter( log0( mcData[[ k ]][ TmaskStat,"Q", j ] ), pl = lowP,
                       pu = highP, nfix = bpfK )
    m.bpf <- bkfilter( mcData[[ k ]][ TmaskStat,"m", j ], pl = lowP,
                       pu = highP, nfix = bpfK )
    i.bpf <- bkfilter( mcData[[ k ]][ TmaskStat,"i", j ], pl = lowP,
                       pu = highP, nfix = bpfK )
    e.bpf <- bkfilter( mcData[[ k ]][ TmaskStat,"e", j ], pl = lowP,
                       pu = highP, nfix = bpfK )
    l.bpf <- bkfilter( mcData[[ k ]][ TmaskStat,"l", j ], pl = lowP,
                       pu = highP, nfix = bpfK )
    J.bpf <- bkfilter( mcData[[ k ]][ TmaskStat,"J", j ], pl = lowP,
                       pu = highP, nfix = bpfK )

    # Augmented Dickey-Fuller tests for unit roots
    Q.adf[[ j ]] <- Q.bpf.adf[[ j ]] <- m.adf[[ j ]] <- m.bpf.adf[[ j ]] <-
      l.adf[[ j ]] <-l.bpf.adf[[ j ]] <- J.adf[[ j ]] <- J.bpf.adf[[ j ]] <- NA
    try( Q.adf[[ j ]] <- adf.test( log0( mcData[[ k ]][ TmaskStat,"Q", j ] ) ),
         silent = TRUE )
    try( Q.bpf.adf[[ j ]] <- adf.test( Q.bpf$cycle[ TmaskBpf, 1 ] ),
         silent = TRUE )
    try( m.adf[[ j ]] <- adf.test( mcData[[ k ]][ TmaskStat,"m", j ] ),
         silent = TRUE )
    try( m.bpf.adf[[ j ]] <- adf.test( i.bpf$cycle[ TmaskBpf, 1 ] ),
         silent = TRUE )
    try( i.adf[[ j ]] <- adf.test( mcData[[ k ]][ TmaskStat,"i", j ] ),
         silent = TRUE )
    try( i.bpf.adf[[ j ]] <- adf.test( i.bpf$cycle[ TmaskBpf, 1 ] ),
         silent = TRUE )
    try( e.adf[[ j ]] <- adf.test( mcData[[ k ]][ TmaskStat,"e", j ] ),
         silent = TRUE )
    try( e.bpf.adf[[ j ]] <- adf.test( e.bpf$cycle[ TmaskBpf, 1 ] ),
         silent = TRUE )
    try( l.adf[[ j ]] <- adf.test( mcData[[ k ]][ TmaskStat,"l", j ] ),
         silent = TRUE )
    try( l.bpf.adf[[ j ]] <- adf.test( l.bpf$cycle[ TmaskBpf, 1 ] ),
         silent = TRUE )
    try( J.adf[[ j ]] <- adf.test( mcData[[ k ]][ TmaskStat,"J", j ] ),
         silent = TRUE )
    try( J.bpf.adf[[ j ]] <- adf.test( J.bpf$cycle[ TmaskBpf, 1 ] ),
         silent = TRUE )

    # standard deviations of filtered series
    Q.sd[j ] <- sd( Q.bpf$cycle[ TmaskBpf, 1 ] )
    m.sd[j ] <- sd( m.bpf$cycle[ TmaskBpf, 1 ] )
    i.sd[j ] <- sd( i.bpf$cycle[ TmaskBpf, 1 ] )
    e.sd[j ] <- sd( e.bpf$cycle[ TmaskBpf, 1 ] )
    l.sd[j ] <- sd( l.bpf$cycle[ TmaskBpf, 1 ] )
    J.sd[j ] <- sd( J.bpf$cycle[ TmaskBpf, 1 ] )

    # correlation structures
    Q.Q[[ j ]] <- ccf( Q.bpf$cycle[ TmaskBpf, 1 ], Q.bpf$cycle[ TmaskBpf, 1 ],
                       lag.max = lags, plot = FALSE, na.action = na.pass )
    m.Q[[ j ]] <- ccf( Q.bpf$cycle[ TmaskBpf, 1 ], m.bpf$cycle[ TmaskBpf, 1 ],
                       lag.max = lags, plot = FALSE, na.action = na.pass )
    i.Q[[ j ]] <- ccf( Q.bpf$cycle[ TmaskBpf, 1 ], i.bpf$cycle[ TmaskBpf, 1 ],
                       lag.max = lags, plot = FALSE, na.action = na.pass )
    e.Q[[ j ]] <- ccf( Q.bpf$cycle[ TmaskBpf, 1 ], e.bpf$cycle[ TmaskBpf, 1 ],
                       lag.max = lags, plot = FALSE, na.action = na.pass )
    l.Q[[ j ]] <- ccf( Q.bpf$cycle[ TmaskBpf, 1 ], l.bpf$cycle[ TmaskBpf, 1 ],
                       lag.max = lags, plot = FALSE, na.action = na.pass )
    J.Q[[ j ]] <- ccf( Q.bpf$cycle[ TmaskBpf, 1 ],J.bpf$cycle[ TmaskBpf, 1 ],
                       lag.max = lags, plot = FALSE, na.action = na.pass )
  }

  # t test to the mean lag results to test their significance (H0: lag < critCorr)
  for( i in 1 : (2 * lags + 1 ) ) { #do for all lags
    if( i != lags + 1 )  # don't try to compute autocorrelation at lag 0
      Q.Q.pval[ i ] <- t.test0(abs( unname( sapply( Q.Q, `[[`, "acf" ) )[ i,] ),
                               critCorr, CI )
    m.Q.pval[ i ] <- t.test0(abs( unname( sapply( m.Q, `[[`, "acf" ) )[ i,] ),
                             critCorr, CI )
    i.Q.pval[ i ] <- t.test0(abs( unname( sapply( i.Q, `[[`, "acf" ) )[ i,] ),
                             critCorr, CI )
    e.Q.pval[ i ] <- t.test0(abs( unname( sapply( e.Q, `[[`, "acf" ) )[ i,] ),
                             critCorr, CI )
    l.Q.pval[ i ] <- t.test0(abs( unname( sapply( l.Q, `[[`, "acf" ) )[ i,] ),
                             critCorr, CI )
    J.Q.pval[ i ] <- t.test0(abs( unname( sapply( J.Q, `[[`, "acf" ) )[ i,] ),
                             critCorr, CI )
  }

  # ---- summary statistics table ----

  key.stats <- matrix( c( mean( Q.growth, na.rm = T ),
                          mean( m.growth, na.rm = T ),
                          mean( i.growth, na.rm = T ),
                          mean( e.growth, na.rm = T ),
                          mean( l.growth, na.rm = T ),

                          sd( Q.growth, na.rm = T ) / sqrt( nSize ),
                          sd( m.growth, na.rm = T ) / sqrt( nSize ),
                          sd( i.growth, na.rm = T ) / sqrt( nSize ),
                          sd( e.growth, na.rm = T ) / sqrt( nSize ),
                          sd( l.growth, na.rm = T ) / sqrt( nSize ),

                          mean( unname( sapply( Q.adf, `[[`, "statistic" ) ),
                                na.rm = T ),
                          mean( unname( sapply( m.adf, `[[`, "statistic" ) ),
                                na.rm = T ),
                          mean( unname( sapply( i.adf, `[[`, "statistic" ) ),
                                na.rm = T ),
                          mean( unname( sapply( e.adf, `[[`, "statistic" ) ),
                                na.rm = T ),
                          mean( unname( sapply( l.adf, `[[`, "statistic" ) ),
                                na.rm = T ),

                          sd( unname( sapply( Q.adf, `[[`, "statistic" ) ),
                              na.rm = T ) / sqrt( nSize ),
                          sd( unname( sapply( m.adf, `[[`, "statistic" ) ),
                              na.rm = T ) / sqrt( nSize ),
                          sd( unname( sapply( i.adf, `[[`, "statistic" ) ),
                              na.rm = T ) / sqrt( nSize ),
                          sd( unname( sapply( e.adf, `[[`, "statistic" ) ),
                              na.rm = T ) / sqrt( nSize ),
                          sd( unname( sapply( l.adf, `[[`, "statistic" ) ),
                              na.rm = T ) / sqrt( nSize ),

                          mean( unname( sapply( Q.adf, `[[`, "p.value" ) ),
                                na.rm = T ),
                          mean( unname( sapply( m.adf, `[[`, "p.value" ) ),
                                na.rm = T ),
                          mean( unname( sapply( i.adf, `[[`, "p.value" ) ),
                                na.rm = T ),
                          mean( unname( sapply( e.adf, `[[`, "p.value" ) ),
                                na.rm = T ),
                          mean( unname( sapply( l.adf, `[[`, "p.value" ) ),
                                na.rm = T ),

                          sd( unname( sapply( Q.adf, `[[`, "p.value" ) ),
                              na.rm = T ) / sqrt( nSize ),
                          sd( unname( sapply( m.adf, `[[`, "p.value" ) ),
                              na.rm = T ) / sqrt( nSize ),
                          sd( unname( sapply( i.adf, `[[`, "p.value" ) ),
                              na.rm = T ) / sqrt( nSize ),
                          sd( unname( sapply( e.adf, `[[`, "p.value" ) ),
                              na.rm = T ) / sqrt( nSize ),
                          sd( unname( sapply( l.adf, `[[`, "p.value" ) ),
                              na.rm = T ) / sqrt( nSize ),

                          mean( unname( sapply( Q.bpf.adf, `[[`, "statistic" ) ),
                                na.rm = T ),
                          mean( unname( sapply( m.bpf.adf, `[[`, "statistic" ) ),
                                na.rm = T ),
                          mean( unname( sapply( i.bpf.adf, `[[`, "statistic" ) ),
                                na.rm = T ),
                          mean( unname( sapply( e.bpf.adf, `[[`, "statistic" ) ),
                                na.rm = T ),
                          mean( unname( sapply( l.bpf.adf, `[[`, "statistic" ) ),
                                na.rm = T ),

                          sd( unname( sapply( Q.bpf.adf, `[[`, "statistic" ) ),
                              na.rm = T ) / sqrt( nSize ),
                          sd( unname( sapply( m.bpf.adf, `[[`, "statistic" ) ),
                              na.rm = T ) / sqrt( nSize ),
                          sd( unname( sapply( i.bpf.adf, `[[`, "statistic" ) ),
                              na.rm = T ) / sqrt( nSize ),
                          sd( unname( sapply( e.bpf.adf, `[[`, "statistic" ) ),
                              na.rm = T ) / sqrt( nSize ),
                          sd( unname( sapply( l.bpf.adf, `[[`, "statistic" ) ),
                              na.rm = T ) / sqrt( nSize ),

                          mean( unname( sapply( Q.bpf.adf, `[[`, "p.value" ) ),
                                na.rm = T ),
                          mean( unname( sapply( m.bpf.adf, `[[`, "p.value" ) ),
                                na.rm = T ),
                          mean( unname( sapply( i.bpf.adf, `[[`, "p.value" ) ),
                                na.rm = T ),
                          mean( unname( sapply( e.bpf.adf, `[[`, "p.value" ) ),
                                na.rm = T ),
                          mean( unname( sapply( l.bpf.adf, `[[`, "p.value" ) ),
                                na.rm = T ),

                          sd( unname( sapply( Q.bpf.adf, `[[`, "p.value" ) ) ) /
                            sqrt( nSize ),
                          sd( unname( sapply( m.bpf.adf, `[[`, "p.value" ) ) ) /
                            sqrt( nSize ),
                          sd( unname( sapply( i.bpf.adf, `[[`, "p.value" ) ) ) /
                            sqrt( nSize ),
                          sd( unname( sapply( e.bpf.adf, `[[`, "p.value" ) ) ) /
                            sqrt( nSize ),
                          sd( unname( sapply( l.bpf.adf, `[[`, "p.value" ) ) ) /
                            sqrt( nSize ),

                          mean( Q.sd, na.rm = T ),
                          mean( m.sd, na.rm = T ),
                          mean( i.sd, na.rm = T ),
                          mean( e.sd, na.rm = T ),
                          mean( l.sd, na.rm = T ),

                          sd( Q.sd, na.rm = T ) / sqrt( nSize ),
                          sd( m.sd, na.rm = T ) / sqrt( nSize ),
                          sd( i.sd, na.rm = T ) / sqrt( nSize ),
                          sd( e.sd, na.rm = T ) / sqrt( nSize ),
                          sd( l.sd, na.rm = T ) / sqrt( nSize ),

                          1,
                          mean( m.sd, na.rm = T ) / mean( Q.sd, na.rm = T ),
                          mean( i.sd, na.rm = T ) / mean( Q.sd, na.rm = T ),
                          mean( e.sd, na.rm = T ) / mean( Q.sd, na.rm = T ),
                          mean( l.sd, na.rm = T ) / mean( Q.sd, na.rm = T ) ),

                       ncol = 5, byrow = T )
  colnames( key.stats ) <- c( "Q (output)", "m (miners)", "i (imitators)",
                              "e (explorers)", "l (known islands)" )
  rownames( key.stats ) <- c( "avg. growth rate", " (s.e.)",
                              "ADF test (logs)", " (s.e.)", " (p-val.)",
                              " (s.e.)","ADF test (bpf)", " (s.e.)",
                              " (p-val.)", " (s.e.)", " s.d. (bpf)", " (s.e.)",
                              " relative s.d. (Q)" )

  textplot( formatC( key.stats, digits = sDigits, format = "g" ), cmar = 2,
            cex = 1.0 )
  title <- paste( "Key statistics and unit roots tests for cycles (",
                  legends[ k ], " )" )
  subTitle <- paste( eval( bquote( paste0( "( bpf: Baxter-King bandpass-filtered series, low = ",
                                           .( lowP), "Q / high = ", .( highP ),
                                           "Q / order = ", .( bpfK ),
                                           " / MC runs = ", .( nSize ),
                                           " / period = ", .( warmUpStat ), " - ",
                                           .( nTsteps ), " )" ) ) ),
                     eval( bquote( paste0( "( ADF test H0: there are unit roots / non-stationary at ",
                                           .( ( 1 - CI ) * 100 ),"% level",
                                           " )" ) ) ), sep ="\n" )
  title( main = title, sub = subTitle )

  # ---- correlation structure table ----

  corr.struct <- matrix( c( colMeans( t( unname( sapply( Q.Q, `[[`, "acf" ) ) ),
                                      na.rm = T ),
                            colSds( t( unname( sapply( Q.Q, `[[`, "acf" ) ) ),
                                    na.rm = T ) / sqrt( nSize ),
                            Q.Q.pval,

                            colMeans( t( unname( sapply( m.Q, `[[`, "acf" ) ) ),
                                      na.rm = T ),
                            colSds( t( unname( sapply( m.Q, `[[`, "acf" ) ) ),
                                    na.rm = T ) / sqrt( nSize ),
                            m.Q.pval,

                            colMeans( t( unname( sapply( i.Q, `[[`, "acf" ) ) ),
                                      na.rm = T ),
                            colSds( t( unname( sapply( i.Q, `[[`, "acf" ) ) ),
                                    na.rm = T ) / sqrt( nSize ),
                            i.Q.pval,

                            colMeans( t( unname( sapply( e.Q, `[[`, "acf" ) ) ),
                                      na.rm = T ),
                            colSds( t( unname( sapply( e.Q, `[[`, "acf" ) ) ),
                                    na.rm = T ) / sqrt( nSize ),
                            e.Q.pval,

                            colMeans( t( unname( sapply( l.Q, `[[`, "acf" ) ) ),
                                      na.rm = T ),
                            colSds( t( unname( sapply( l.Q, `[[`, "acf" ) ) ),
                                    na.rm = T ) / sqrt( nSize ),
                            l.Q.pval,

                            colMeans( t( unname( sapply( J.Q, `[[`, "acf" ) ) ),
                                      na.rm = T ),
                            colSds( t( unname( sapply( J.Q, `[[`, "acf" ) ) ),
                                    na.rm = T ) / sqrt( nSize ),
                            J.Q.pval ),

                          ncol = 2 * lags + 1, byrow = T )
  colnames( corr.struct ) <- Q.Q[[ 1 ]]$lag
  rownames( corr.struct ) <- c( "Q (output)", " (s.e.)", " (p-val.)",
                                "m (miners)", " (s.e.)", " (p-val.)",
                                "i (imitators)", " (s.e.)", " (p-val.)",
                                "e (explorers)", " (s.e.)", " (p-val.)",
                                "l (known islands)", " (s.e.)", " (p-val.)",
                                "J (islands)", " (s.e.)", " (p-val.)" )

  title <- paste( "Correlation structure for Q (", legends[ k ], " )" )
  subTitle <- paste( eval( bquote( paste0( "( non-rate/ratio series are Baxter-King bandpass-filtered, low = ",
                                           .( lowP ), "Q / high = ", .( highP ),
                                           "Q / order = ", .( bpfK ),
                                           " / MC runs = ", .( nSize ),
                                           " / period = ", .( warmUpStat ),
                                           " - ", .( nTsteps ), " )" ) ) ),
                     eval( bquote ( paste0( "( test H0: lag coefficient is not significant at ",
                                            .( ( 1 - CI ) * 100 ),"% level",
                                            " )" ) ) ), sep ="\n" )

  textplot( formatC( corr.struct, digits = sDigits, format = "g" ), cmar = 1 )
  title( main = title, sub = subTitle )
}

# ---- stationarity and ergodicity tests ----

for( k in 1 : nExp ) {

  statErgo <- ergod.test.lsd( mcData[[ k ]][ TmaskStat, , ], signif = 1 - CI,
                              vars = statErgo.vars )

  textplot( statErgo, cmar = 2, rmar = 0.5, cex = 1.0 )
  title( main = paste( "Stationarity, i.i.d. and ergodicity tests (",
                       legends[ k ], " )" ),
         sub = paste( "( average p-values for testing H0 and rate of rejection of H0 / MC runs =",
                      nSize, "/ period =", warmUpStat, "-", nTsteps,
                      " )\n ( ADF/PP H0: non-stationary, KPSS H0: stationary, BDS H0: i.i.d., KS/AD/WW H0: ergodic )\n( significance =",
                      1 - CI, " )" ) )
}

# ======= experiment comparison =======

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
  for( j in 1 : nSize )
    for( i in TmaskStat )
      if( i == 1 ) {
        temp[ i - warmUpStat, j ] <- 0
      } else
        temp[ i - warmUpStat, j ] <- ( log0( mcData[[ k ]][ i, "Q", j ] ) -
                                         log0( mcData[[ k ]][ i - 1, "Q", j ] ) )
  temp[ is.infinite( temp ) ] <- sign( temp[ is.infinite( temp ) ] )
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
  for( j in 1 : nSize)
    for( i in TmaskStat)
      if( i == 1 ) {
        temp[ i - warmUpStat, j ] <- 0
      } else
        temp[ i - warmUpStat, j ] <- ( log0( mcData[[ k ]][ i,"Q", j ] ) -
                                     log0( mcData[[ k ]][ i - 1,"Q", j ] ) )
  temp[ is.infinite( temp ) ] <- sign( temp[ is.infinite( temp ) ] )
  x <- colSds( temp, na.rm = TRUE )
  bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
  statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
  statsBp[ stat, , k ] <- bPlotStats$stats
  n[ stat, k ] <- bPlotStats$n
  conf[ stat, , k ] <- bPlotStats$conf
  out[[ stat, k ]] <- bPlotStats$out

  stat <- stat + 1
  names[[ stat ]] <- "Likelihood of GDP crises"
  units[[ stat ]] <- "Likelihood ( probability) of GDP crises"
  for( j in 1 : nSize) {
    for( i in TmaskStat) {
      if( i == 1 ) {
        temp[ i - warmUpStat, j ] <- 0
      } else {
        if( log0( mcData[[ k ]][ i, "Q", j ] ) -
           log0( mcData[[ k ]][ i - 1, "Q", j ] ) < -0.03 ) {
          temp[ i - warmUpStat, j ] <- 1
        } else {
          temp[ i - warmUpStat, j ] <- 0
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
  names[[ stat ]] <- "Known islands"
  units[[ stat ]] <- "Number of known islands"
  temp <- mcData[[ k ]][ TmaskStat, "l", ]
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
  temp <- mcData[[ k ]][ TmaskStat, "J", ]
  temp[ ! is.finite( temp ) ] <- NA
  x <- colMeans( temp, na.rm = TRUE )
  bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
  statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
  statsBp[ stat, , k ] <- bPlotStats$stats
  n[ stat, k ] <- bPlotStats$n
  conf[ stat, , k ] <- bPlotStats$conf
  out[[ stat, k ]] <- bPlotStats$out

  stat <- stat + 1
  names[[ stat ]] <- "Miners"
  units[[ stat ]] <- "Number of miners"
  temp <- mcData[[ k ]][ TmaskStat, "m", ]
  temp[ ! is.finite( temp ) ] <- NA
  x <- colMeans( temp, na.rm = TRUE )
  bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
  statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
  statsBp[ stat, , k ] <- bPlotStats$stats
  n[ stat, k ] <- bPlotStats$n
  conf[ stat, , k ] <- bPlotStats$conf
  out[[ stat, k ]] <- bPlotStats$out

  stat <- stat + 1
  names[[ stat ]] <- "Imitators"
  units[[ stat ]] <- "Number of imitators"
  temp <- mcData[[ k ]][ TmaskStat, "i", ]
  temp[ ! is.finite( temp ) ] <- NA
  x <- colMeans( temp, na.rm = TRUE )
  bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
  statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
  statsBp[ stat, , k ] <- bPlotStats$stats
  n[ stat, k ] <- bPlotStats$n
  conf[ stat, , k ] <- bPlotStats$conf
  out[[ stat, k ]] <- bPlotStats$out

  stat <- stat + 1
  names[[ stat ]] <- "Explorers"
  units[[ stat ]] <- "Number of explorers"
  temp <- mcData[[ k ]][ TmaskStat, "e", ]
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

# ---- experiments statistics table and performance comparison chart ----

table.stats <- statsTb[ , , 1 ]
table.names <- c( "Avg[1]", "SD[1]", "Min[1]", "Max[1]" )
perf.comp <- statsTb[ , 1, 1 ]
perf.names <- c( "Baseline[1]" )

for( stat in 1 : numStats ) {

  lowLim <- Inf
  upLim <- -Inf
  for( k in 1 : nExp ) {
    if( conf[ stat, 1, k ] < lowLim )
      lowLim <- conf[ stat, 1, k ]
    if( conf[ stat, 2, k ] > upLim )
      upLim <- conf[ stat, 2, k ]
  }
  upLim <- upLim + ( upLim - lowLim )
  lowLim <- lowLim - ( upLim - lowLim )

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

  if ( nExp > 1 )
    listBp <- list( stats = statsBp[ stat, , ], n = n[ stat, ],
                    conf = conf[ stat, , ], out = outVal, group = outGrp,
                    names = legends )
  else
    listBp <- list( stats = matrix( statsBp[ stat, , ] ),
                    n = matrix( n[ stat, ] ), conf = matrix( conf[ stat, , ] ),
                    out = outVal, group = outGrp, names = legends )

  title <- names[[ stat ]]
  subTitle <- as.expression(bquote( paste( "( bar: median / box: 2nd-3rd quartile / whiskers: max-min / points: outliers / MC runs = ",
                                          .( nSize ), " / period = ",
                                          .( warmUpStat ), " - ", .( nTsteps ),
                                          " )" ) ) )
  tryCatch( bxp( listBp, range = bPlotCoef, notch = bPlotNotc, main = title,
                 sub = subTitle, ylab = units[[ stat ]] ),
            error = function( e ) {
              warning( "In boxplot (bxp ): problem while plotting: ", title,
                       "\n\n" )
              textplot( paste( "Plot for <", title, "> failed." ) )
            } )
}

if( nExp > 1 ) {
  for( k in 2 : nExp ) {

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

colnames( table.stats ) <- table.names
rownames( table.stats ) <- names

textplot( formatC( table.stats, digits = sDigits, format = "g" ), cmar = 1 )
title <- paste( "Monte Carlo descriptive statistics ( all experiments )" )
subTitle <- paste( "( numbers in brackets indicate the experiment number / MC runs =",
                   nSize, "/ period =", warmUpStat, "-", nTsteps, " )" )
title( main = title, sub = subTitle )
mtext( legendList, side = 1, line = -2, outer = TRUE )

if( nExp > 1 ) {
  colnames( perf.comp ) <- perf.names
  rownames( perf.comp ) <- names

  textplot( formatC( perf.comp, digits = sDigits, format = "g" ), cmar = 1 )
  title <- paste( "Performance comparison ( all experiments )" )
  subTitle <- paste( "( numbers in brackets indicate the experiment number / H0: no difference with baseline / MC runs =",
                     nSize, "/ period =", warmUpStat, "-", nTsteps, " )" )
  title( main = title, sub = subTitle )
  mtext( legendList, side = 1, line = -2, outer = TRUE )
}

dev.off( )    # Close PDF plot file
