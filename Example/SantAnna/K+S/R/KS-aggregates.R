#******************************************************************
#
# ----------------- K+S Aggregates analysis ---------------------
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

firmTypes = c( "Pre-change firms", "Post-change firms" )

# Aggregated variables to use
logVars <- c( "GDP", "EI", "dN", "C", "I", "Deb1", "Deb2",
              "NWb", "NW1", "NW2", "S1", "S2", "A", "A2preChg",
              "A2posChg", "wAvgReal", "w2realPreChg", "w2realPosChg",
              "G", "Gbail", "Gtrain", "Tax", "Deb", "Def", "BadDeb",
              "TC", "Loans", "W1", "W2", "B2" )
aggrVars <- append( logVars, c( "dGDP", "dCPI", "Q2u", "dA", "F1", "F2",
                                "entry1", "entry2", "exit1", "exit2", "f2posChg",
                                "HH1", "HH2", "imi", "inn", "mu2avg", "r",
                                "U", "Ue", "V", "Lent", "Lexit", "part", "TeAvg",
                                "dw", "wGini", "wLogSD", "sTavg", "sVavg",
                                "A2sdPreChg", "A2sdPosChg", "CPI", "q2avg",
                                "q2preChg", "q2posChg", "Bda", "Bfail" ) )

# Variables to test for stationarity and ergodicity
statErgo.vars <- c( "dGDP", "dA", "dw", "wLogSD", "V", "U",
                    "wGini", "mu2avg", "HH1", "HH2" )


# ==== Process LSD result files ====

# Package with LSD interface functions
library( LSDinterface, verbose = FALSE, quietly = TRUE )

# remove warnings for saved data
# !diagnostics suppress = A, S, M, m

# ---- Read data files ----

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

  # Read data from text files and format it as a 3D array with labels
  mc <- read.3d.lsd( myFiles, aggrVars, skip = iniDrop, nrows = nKeep, nnodes = 1 )

  # Get dimensions information
  nTsteps <- dim( mc )[ 1 ]              # number of time steps
  nVar <- dim( mc )[ 2 ]                 # number of variables
  nSize  <- dim( mc )[ 3 ]               # Monte Carlo sample size

  # Compute Monte Carlo averages and std. deviation and store in 2D arrrays
  stats <- info.stats.lsd( mc )

  # Insert a t column
  t <- as.integer( rownames( stats$avg ) )
  A <- as.data.frame( cbind( t, stats$avg ) )
  S <- as.data.frame( cbind( t, stats$sd ) )
  M <- as.data.frame( cbind( t, stats$max ) )
  m <- as.data.frame( cbind( t, stats$min ) )

  # Save temporary results to disk to save memory
  tmpFile <- paste0( folder, "/", baseName, exper, "_aggr.Rdata" )
  save( mc, A, S, M, m, nTsteps, nVar, nSize, file = tmpFile )

  return( tmpFile )
}

# load each experiment serially
tmpFiles <- lapply( 1 : nExp, readExp )

# ---- Organize data read from files ----

# fill the lists to hold data
mcData <- list()  # 3D Monte Carlo data
Adata <- list()  # average data
Sdata <- list()  # standard deviation data
Mdata <- list()  # maximum data
mdata <- list()  # minimum data
nTsteps.1 <- nSize.1 <- 0

for( k in 1 : nExp ) {                      # realocate data in separate lists

  load( tmpFiles[[ k ]] )                   # pick data from disk
  file.remove( tmpFiles[[ k ]] )            # and delete temporary file

  if( k > 1 && ( nTsteps != nTsteps.1 || nSize != nSize.1 ) )
    stop( "Inconsistent data files.\nSame number of time steps and of MC runs is required." )

  mcData[[ k ]] <- mc
  rm( mc )
  Adata[[ k ]] <- A
  Sdata[[ k ]] <- S
  Mdata[[ k ]] <- M
  mdata[[ k ]] <- m
  nTsteps.1 <- nTsteps
  nSize.1 <- nSize
}

# free memory
rm( tmpFiles, A, S, M, m, nTsteps.1, nSize.1 )
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
nTplot    <- -1     # last period to consider for plots (-1=all)
warmUpStat<- 300    # warm-up runs to evaluate all statistics
nTstat    <- -1     # last period to consider for statistics (-1=all)
lowP      <- 6      # bandpass filter minimum period
highP     <- 32     # bandpass filter maximum period
bpfK      <- 12     # bandpass filter order
lags      <- 4      # lags to analyze
bPlotCoef <- 1.5    # boxplot whiskers extension from the box (0=extremes)
bPlotNotc <- FALSE  # use boxplot notches
smoothing <- 1e5    # HP filter smoothing factor (lambda)

repName   <- ""     # report files base name (if "" same baseName)
transMk   <- 100    # regime transition mark after warm-up (-1:none)
sDigits   <- 4      # significant digits in tables
plotRows  <- 1      # number of plots per row in a page
plotCols  <- 1  	  # number of plots per column in a page
plotW     <- 10     # plot window width
plotH     <- 7      # plot window height

# Colors assigned to each experiment's lines in graphics
colors <- c( "black", "blue", "red", "orange", "green", "brown" )

# Line types assigned to each experiment
lTypes <- c( "solid", "solid", "solid", "solid", "solid", "solid" )

# Point types assigned to each experiment
pTypes <- c( 4, 4, 4, 4, 4, 4 )


# ====== External support functions & definitions ======

library( LSDsensitivity, verbose = FALSE, quietly = TRUE )
if( ! exists( "log0", mode = "function" ) ||
    ! exists( "time_plots", mode = "function" ) ||
    ! exists( "box_plots", mode = "function" ) ) {    # already loaded?
  source( "KS-support-functions.R" )
  source( "KS-time-plots.R" )
  source( "KS-box-plots.R" )
}

# remove warnings for support functions and saved data
# !diagnostics suppress = mc, nTsteps, nKeep, nVar, nSize
# !diagnostics suppress = log0, t.test0, bkfilter, hpfilter, adf.test
# !diagnostics suppress = textplot, colSds, plot_recovery, plot_bpf
# !diagnostics suppress = time_plots, box_plots


# ==== Support stuff ====

if( repName == "" )
  repName <- baseName

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

# Number of periods to show in graphics and use in statistics
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

# Calculates the critical correlation limit for significance (under heroic assumptions!)
critCorr <- qnorm( 1 - ( 1 - CI ) / 2 ) / sqrt( nTstat )


# ==== Main code ====

# Open PDF plot file for output
pdf( paste0( folder, "/", repName, "_aggr_plots.pdf" ),
     width = plotW, height = plotH )
par( mfrow = c ( plotRows, plotCols ) )             # define plots per page


#
# ====== MC PLOTS GENERATION ======
#

time_plots( mcData, Adata, mdata, Mdata, Sdata, nExp, nSize, nTsteps, TmaskPlot,
            CI, legends, colors, lTypes, transMk, smoothing, firmTypes )

#
# ====== STATISTICS GENERATION ======
#

# Create vectors and lists to hold the Monte Carlo results
gdp.growth <- cReal.growth <- iR.growth <- A.growth <- wAvgReal.growth <-
  sT.growth <- TeAvg.growth <- gdp.sd <- cReal.sd <- iR.sd <- A.sd <-
  wAvgReal.sd <- sT.sd <- TeAvg.sd <- vector( mode = "numeric", length = nSize )
gdp.adf <- cReal.adf <- iR.adf <- A.adf <- wAvgReal.adf <- sT.adf <- TeAvg.adf <-
  gdp.bpf.adf <- cReal.bpf.adf <- iR.bpf.adf <- A.bpf.adf <- wAvgReal.bpf.adf <-
  sT.bpf.adf <- TeAvg.bpf.adf <- list( )

gdp.gdp <- cReal.gdp <- iR.gdp <- A.gdp <- wAvgReal.gdp <- totDeb.gdp <-
  liqSale.gdp <- bankrup.gdp <- EI.gdp <- dN.gdp <- mu2avg.gdp <-
  U.gdp <- V.gdp <- Lent.gdp <- Lexit.gdp <- netEntr.gdp <-
  entry.gdp <- list( )
gdp.gdp.pval <- cReal.gdp.pval <- iR.gdp.pval <- A.gdp.pval <-
  wAvgReal.gdp.pval <- totDeb.gdp.pval <- liqSale.gdp.pval <- bankrup.gdp.pval <-
  EI.gdp.pval <- dN.gdp.pval <- mu2avg.gdp.pval <- U.gdp.pval <-
  V.gdp.pval <- Lent.gdp.pval <- Lexit.gdp.pval <- netEntr.gdp.pval <-
  entry.gdp.pval <- vector( mode = "numeric", length = nExp )

for( k in 1 : nExp ){ # Experiment k

  #
  # ---- Bandpass filtered GDP, consumption and investment cycles graphic ----
  #

  plot_bpf( list( log0( Adata[[ k ]]$GDP ), log0( Adata[[ k ]]$C ),
                  log0( Adata[[ k ]]$I ), log0( Adata[[ k ]]$A ) ),
            pl = lowP, pu = highP, nfix = bpfK, mask = TmaskPlot,
            mrk = transMk, col = colors, lty = lTypes,
            leg = c("GDP", "Consumption", "Investment", "Productivity" ),
            xlab = "Time", ylab = "Filtered series",
            tit = paste( "GDP cycles (", legends[ k ], ")" ),
            subtit = paste( "( Baxter-King bandpass-filtered, low = 6Q / high = 32Q / order = 12 / MC runs =",
                            nSize, ")" ) )

  #
  # ---- Bandpass filtered vacancy, unemployment and productivity ----
  #

  plot_bpf( list( Adata[[ k ]]$U, Adata[[ k ]]$V ),
            pl = lowP, pu = highP, nfix = bpfK, mask = TmaskPlot,
            mrk = transMk, col = colors, lty = lTypes,
            leg = c( "Productivity", "Unemployment", "Vacancy" ),
            xlab = "Time", ylab = "Filtered series",
            tit = paste( "Shimer puzzle (", legends[ k ], ")" ),
            subtit = paste( "( Baxter-King bandpass-filtered, low = 6Q / high = 32Q / order = 12 / MC runs =",
                            nSize, ")" ) )

  #
  # ---- Bandpass filtered GDP and net entry ----
  #

  plot_bpf( list( log0( Adata[[ k ]]$GDP ), Adata[[ k ]]$entry2 - Adata[[ k ]]$exit2 ),
            pl = lowP, pu = highP, nfix = bpfK, mask = TmaskPlot,
            mrk = transMk, resc = c( 0.5, NA ), col = colors, lty = lTypes,
            leg = c( "GDP", "Net entry (sector 2)" ),
            xlab = "Time", ylab = "Filtered series (rescaled)",
            tit = paste( "Net entry dynamics and business cycle (", legends[ k ], ")" ),
            subtit = paste( "( Baxter-King bandpass-filtered, low = 6Q / high = 32Q / order = 12 / MC runs =",
                            nSize, ")" ) )

  #
  # ==== Statistics computation for tables ====
  #

  for( j in 1 : nSize ){  # Execute for every Monte Carlo run

    # Monte carlo average growth rates
    gdp.growth[ j ] <- ( log0( mcData[[ k ]][ nTstat, "GDP", j ] ) -
                        log0( mcData[[ k ]][ warmUpStat + 1, "GDP", j ] ) ) / TmaxStat
    cReal.growth[ j ] <- ( log0( mcData[[ k ]][ nTstat, "C", j ] ) -
                          log0( mcData[[ k ]][ warmUpStat + 1, "C", j ] ) ) / TmaxStat
    iR.growth[ j ] <- ( log0( mcData[[ k ]][ nTstat, "I", j ] ) -
                       log0( mcData[[ k ]][ warmUpStat + 1, "I", j ] ) ) / TmaxStat
    A.growth[ j ] <- ( log0( mcData[[ k ]][ nTstat, "A", j ] ) -
                       log0( mcData[[ k ]][ warmUpStat + 1, "A", j ] ) ) / TmaxStat
    wAvgReal.growth[ j ] <- ( log0( mcData[[ k ]][ nTstat, "wAvgReal", j ] ) -
                       log0( mcData[[ k ]][ warmUpStat + 1, "wAvgReal", j ] ) ) / TmaxStat
    sT.growth[ j ] <- ( log0( mcData[[ k ]][ nTstat, "sTavg", j ] ) -
                       log0( mcData[[ k ]][ warmUpStat + 1, "sTavg", j ] ) ) / TmaxStat
    TeAvg.growth[ j ] <- ( log0( mcData[[ k ]][ nTstat, "TeAvg", j ] ) -
                       log0( mcData[[ k ]][ warmUpStat + 1, "TeAvg", j ] ) ) / TmaxStat

    # Apply Baxter-King filter to the series
    gdp.bpf <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "GDP", j ] ), pl = lowP, pu = highP, nfix = bpfK )
    cReal.bpf <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "C", j ] ), pl = lowP, pu = highP, nfix = bpfK )
    iR.bpf <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "I", j ] ), pl = lowP, pu = highP, nfix = bpfK )
    A.bpf <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "A", j ] ), pl = lowP, pu = highP, nfix = bpfK )
    wAvgReal.bpf <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "wAvgReal", j ] ), pl = lowP, pu = highP, nfix = bpfK )
    sT.bpf <- bkfilter( mcData[[ k ]][ TmaskStat, "sTavg", j ], pl = lowP, pu = highP, nfix = bpfK )
    TeAvg.bpf <- bkfilter( mcData[[ k ]][ TmaskStat, "TeAvg", j ], pl = lowP, pu = highP, nfix = bpfK )
    totDeb.bpf <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "Deb1", j ] + mcData[[ k ]][ TmaskStat, "Deb2", j ] ),
                           pl = lowP, pu = highP, nfix = bpfK )
    liqSale.bpf <- bkfilter((mcData[[ k ]][ TmaskStat, "NW1", j ] + mcData[[ k ]][ TmaskStat, "NW2", j ] ) /
                              (mcData[[ k ]][ TmaskStat, "S1", j ] + mcData[[ k ]][ TmaskStat, "S2", j ] ),
                            pl = lowP, pu = highP, nfix = bpfK )
    bankrup.bpf <- bkfilter( mcData[[ k ]][ TmaskStat, "exit2", j ] / mcData[[ k ]][ TmaskStat, "F2", j ],
                            pl = lowP, pu = highP, nfix = bpfK )
    EI.bpf <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "EI", j ] ), pl = lowP, pu = highP, nfix = bpfK )
    dN.bpf <- bkfilter( mcData[[ k ]][ TmaskStat, "dN", j ], pl = lowP, pu = highP, nfix = bpfK )
    mu2avg.bpf <- bkfilter( mcData[[ k ]][ TmaskStat, "mu2avg", j ], pl = lowP, pu = highP, nfix = bpfK )
    U.bpf <- bkfilter( mcData[[ k ]][ TmaskStat, "U", j ], pl = lowP, pu = highP, nfix = bpfK )
    V.bpf <- bkfilter( mcData[[ k ]][ TmaskStat, "V", j ], pl = lowP, pu = highP, nfix = bpfK )
    Lent.bpf <- bkfilter( mcData[[ k ]][ TmaskStat, "Lent", j ], pl = lowP, pu = highP, nfix = bpfK )
    Lexit.bpf <- bkfilter( mcData[[ k ]][ TmaskStat, "Lexit", j ], pl = lowP, pu = highP, nfix = bpfK )
    netEntr.bpf <- bkfilter( mcData[[ k ]][ TmaskStat, "entry1", j ] - mcData[[ k ]][ TmaskStat, "exit1", j ] +
                               mcData[[ k ]][ TmaskStat, "entry2", j ] - mcData[[ k ]][ TmaskStat, "exit2", j ],
                              pl = lowP, pu = highP, nfix = bpfK )
    entry.bpf <- bkfilter( mcData[[ k ]][ TmaskStat, "entry1", j ] + mcData[[ k ]][ TmaskStat, "entry2", j ],
                              pl = lowP, pu = highP, nfix = bpfK )

    # Augmented Dickey-Fuller tests for unit roots
    gdp.adf[[ j ]] <- adf.test( log0( mcData[[ k ]][ TmaskStat, "GDP", j ] ) )
    gdp.bpf.adf[[ j ]] <- adf.test(gdp.bpf$cycle[ TmaskBpf, 1 ] )
    cReal.adf[[ j ]] <- adf.test( log0( mcData[[ k ]][ TmaskStat, "C", j ] ) )
    cReal.bpf.adf[[ j ]] <- adf.test(cReal.bpf$cycle[ TmaskBpf, 1 ] )
    iR.adf[[ j ]] <- adf.test( log0( mcData[[ k ]][ TmaskStat, "I", j ] ) )
    iR.bpf.adf[[ j ]] <- adf.test( iR.bpf$cycle[ TmaskBpf, 1 ] )
    A.adf[[ j ]] <- adf.test( log0( mcData[[ k ]][ TmaskStat, "A", j ] ) )
    A.bpf.adf[[ j ]] <- adf.test( A.bpf$cycle[ TmaskBpf, 1 ] )
    wAvgReal.adf[[ j ]] <- adf.test( log0( mcData[[ k ]][ TmaskStat, "wAvgReal", j ] ) )
    wAvgReal.bpf.adf[[ j ]] <- adf.test( wAvgReal.bpf$cycle[ TmaskBpf, 1 ] )
    sT.adf[[ j ]] <- adf.test( mcData[[ k ]][ TmaskStat, "sTavg", j ] )
    sT.bpf.adf[[ j ]] <- adf.test( sT.bpf$cycle[ TmaskBpf, 1 ] )
    TeAvg.adf[[ j ]] <- adf.test( mcData[[ k ]][ TmaskStat, "TeAvg", j ] )
    TeAvg.bpf.adf[[ j ]] <- adf.test( TeAvg.bpf$cycle[ TmaskBpf, 1 ] )

    # Standard deviations of filtered series
    gdp.sd[ j ] <- sd( gdp.bpf$cycle[ TmaskBpf, 1 ] )
    cReal.sd[ j ] <- sd( cReal.bpf$cycle[ TmaskBpf, 1 ] )
    iR.sd[ j ] <- sd( iR.bpf$cycle[ TmaskBpf, 1 ] )
    A.sd[ j ] <- sd( A.bpf$cycle[ TmaskBpf, 1 ] )
    wAvgReal.sd[ j ] <- sd( wAvgReal.bpf$cycle[ TmaskBpf, 1 ] )
    sT.sd[ j ] <- sd( sT.bpf$cycle[ TmaskBpf, 1 ] )
    TeAvg.sd[ j ] <- sd( TeAvg.bpf$cycle[ TmaskBpf, 1 ] )

    # Build the correlation structures
    gdp.gdp[[ j ]] <- ccf( gdp.bpf$cycle[ TmaskBpf, 1 ],
                         gdp.bpf$cycle[ TmaskBpf, 1 ],
                         lag.max = lags, plot = FALSE, na.action = na.pass )
    cReal.gdp[[ j ]] <- ccf( gdp.bpf$cycle[ TmaskBpf, 1 ],
                           cReal.bpf$cycle[ TmaskBpf, 1 ],
                           lag.max = lags, plot = FALSE, na.action = na.pass )
    iR.gdp[[ j ]] <- ccf( gdp.bpf$cycle[ TmaskBpf, 1 ],
                        iR.bpf$cycle[ TmaskBpf, 1 ],
                        lag.max = lags, plot = FALSE, na.action = na.pass )
    A.gdp[[ j ]] <- ccf( gdp.bpf$cycle[ TmaskBpf, 1 ],
                        A.bpf$cycle[ TmaskBpf, 1 ],
                        lag.max = lags, plot = FALSE, na.action = na.pass )
    wAvgReal.gdp[[ j ]] <- ccf( gdp.bpf$cycle[ TmaskBpf, 1 ],
                        wAvgReal.bpf$cycle[ TmaskBpf, 1 ],
                        lag.max = lags, plot = FALSE, na.action = na.pass )
    totDeb.gdp[[ j ]] <- ccf( gdp.bpf$cycle[ TmaskBpf, 1 ],
                            totDeb.bpf$cycle[ TmaskBpf, 1 ],
                            lag.max = lags, plot = FALSE, na.action = na.pass )
    liqSale.gdp[[ j ]] <- ccf( gdp.bpf$cycle[ TmaskBpf, 1 ],
                             liqSale.bpf$cycle[ TmaskBpf, 1 ],
                             lag.max = lags, plot = FALSE, na.action = na.pass )
    bankrup.gdp[[ j ]] <- ccf( gdp.bpf$cycle[ TmaskBpf, 1 ],
                             bankrup.bpf$cycle[ TmaskBpf, 1 ],
                             lag.max = lags, plot = FALSE, na.action = na.pass )
    EI.gdp[[ j ]] <- ccf( gdp.bpf$cycle[ TmaskBpf, 1 ],
                           EI.bpf$cycle[ TmaskBpf, 1 ],
                           lag.max = lags, plot = FALSE, na.action = na.pass )
    dN.gdp[[ j ]] <- ccf( gdp.bpf$cycle[ TmaskBpf, 1 ],
                           dN.bpf$cycle[ TmaskBpf, 1 ],
                           lag.max = lags, plot = FALSE, na.action = na.pass )
    mu2avg.gdp[[ j ]] <- ccf( gdp.bpf$cycle[ TmaskBpf, 1 ],
                           mu2avg.bpf$cycle[ TmaskBpf, 1 ],
                           lag.max = lags, plot = FALSE, na.action = na.pass )
    U.gdp[[ j ]] <- ccf( gdp.bpf$cycle[ TmaskBpf, 1 ],
                       U.bpf$cycle[ TmaskBpf, 1 ],
                       lag.max = lags, plot = FALSE, na.action = na.pass )
    V.gdp[[ j ]] <- ccf( gdp.bpf$cycle[ TmaskBpf, 1 ],
                       V.bpf$cycle[ TmaskBpf, 1 ],
                       lag.max = lags, plot = FALSE, na.action = na.pass )
    Lent.gdp[[ j ]] <- ccf( gdp.bpf$cycle[ TmaskBpf, 1 ],
                            Lent.bpf$cycle[ TmaskBpf, 1 ],
                            lag.max = lags, plot = FALSE, na.action = na.pass )
    Lexit.gdp[[ j ]] <- ccf( gdp.bpf$cycle[ TmaskBpf, 1 ],
                           Lexit.bpf$cycle[ TmaskBpf, 1 ],
                           lag.max = lags, plot = FALSE, na.action = na.pass )
    netEntr.gdp[[ j ]] <- ccf( gdp.bpf$cycle[ TmaskBpf, 1 ],
                               netEntr.bpf$cycle[ TmaskBpf, 1 ],
                               lag.max = lags, plot = FALSE, na.action = na.pass )
    entry.gdp[[ j ]] <- ccf( gdp.bpf$cycle[ TmaskBpf, 1 ],
                                entry.bpf$cycle[ TmaskBpf, 1 ],
                                lag.max = lags, plot = FALSE, na.action = na.pass )
  }

  # Applies t test to the mean lag results to test their significance (H0: lag < critCorr)
  for(i in 1 : (2 * lags + 1) ){ #do for all lags
    if(i != lags + 1)  # don't try to compute autocorrelation at lag 0
      gdp.gdp.pval[ i ] <- t.test0( abs( unname( sapply( gdp.gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
    cReal.gdp.pval[ i ] <- t.test0( abs( unname( sapply( cReal.gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
    iR.gdp.pval[ i ] <- t.test0( abs( unname( sapply( iR.gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
    A.gdp.pval[ i ] <- t.test0( abs( unname( sapply( A.gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
    wAvgReal.gdp.pval[ i ]<- t.test0( abs( unname( sapply( wAvgReal.gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
    totDeb.gdp.pval[ i ] <- t.test0( abs( unname( sapply( totDeb.gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
    liqSale.gdp.pval[ i ] <- t.test0( abs( unname( sapply( liqSale.gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
    bankrup.gdp.pval[ i ] <- t.test0( abs( unname( sapply( bankrup.gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
    EI.gdp.pval[ i ] <- t.test0( abs( unname( sapply( EI.gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
    dN.gdp.pval[ i ] <- t.test0( abs( unname( sapply( dN.gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
    mu2avg.gdp.pval[ i ] <- t.test0( abs( unname( sapply( mu2avg.gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
    U.gdp.pval[ i ] <- t.test0( abs( unname( sapply( U.gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
    V.gdp.pval[ i ] <- t.test0( abs( unname( sapply( V.gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
    Lent.gdp.pval[ i ] <- t.test0( abs( unname( sapply( Lent.gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
    Lexit.gdp.pval[ i ] <- t.test0( abs( unname( sapply( Lexit.gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
    netEntr.gdp.pval[ i ] <- t.test0( abs( unname( sapply( netEntr.gdp, `[[`, "acf" ) )[ i, ] ),
                                     critCorr, CI )
    entry.gdp.pval[ i ] <- t.test0( abs( unname( sapply( entry.gdp, `[[`, "acf" ) )[ i, ] ),
                                     critCorr, CI )
  }

  #
  # ---- Summary statistics table (averages, standard errors and p-values) ----
  #

  key.stats <- matrix(c(mean( gdp.growth ), mean( cReal.growth ), mean( iR.growth ),
                        mean( A.growth ), mean( wAvgReal.growth ), mean( sT.growth ),
                        mean( TeAvg.growth ),

                        sd( gdp.growth ) / sqrt( nSize ), sd( cReal.growth ) / sqrt( nSize ),
                        sd( iR.growth ) / sqrt( nSize ), sd( A.growth ) / sqrt( nSize ),
                        sd( wAvgReal.growth )/ sqrt( nSize ), sd( sT.growth )/ sqrt( nSize ),
                        sd( TeAvg.growth )/ sqrt( nSize ),

                        mean( unname( sapply( gdp.adf, `[[`, "statistic" ) ) ),
                        mean( unname( sapply( cReal.adf, `[[`, "statistic" ) ) ),
                        mean( unname( sapply( iR.adf, `[[`, "statistic" ) ) ),
                        mean( unname( sapply( A.adf, `[[`, "statistic" ) ) ),
                        mean( unname( sapply( wAvgReal.adf, `[[`, "statistic" ) ) ),
                        mean( unname( sapply( sT.adf, `[[`, "statistic" ) ) ),
                        mean( unname( sapply( TeAvg.adf, `[[`, "statistic" ) ) ),

                        sd( unname( sapply( gdp.adf, `[[`, "statistic" ) ) ) / sqrt( nSize ),
                        sd( unname( sapply( cReal.adf, `[[`, "statistic" ) ) ) / sqrt( nSize ),
                        sd( unname( sapply( iR.adf, `[[`, "statistic" ) ) ) / sqrt( nSize ),
                        sd( unname( sapply( A.adf, `[[`, "statistic" ) ) ) / sqrt( nSize ),
                        sd( unname( sapply( wAvgReal.adf, `[[`, "statistic" ) ) ) / sqrt( nSize ),
                        sd( unname( sapply( sT.adf, `[[`, "statistic" ) ) ) / sqrt( nSize ),
                        sd( unname( sapply( TeAvg.adf, `[[`, "statistic" ) ) ) / sqrt( nSize ),

                        mean( unname( sapply( gdp.adf, `[[`, "p.value" ) ) ),
                        mean( unname( sapply( cReal.adf, `[[`, "p.value" ) ) ),
                        mean( unname( sapply( iR.adf, `[[`, "p.value" ) ) ),
                        mean( unname( sapply( A.adf, `[[`, "p.value" ) ) ),
                        mean( unname( sapply( wAvgReal.adf, `[[`, "p.value" ) ) ),
                        mean( unname( sapply( sT.adf, `[[`, "p.value" ) ) ),
                        mean( unname( sapply( TeAvg.adf, `[[`, "p.value" ) ) ),

                        sd( unname( sapply( gdp.adf, `[[`, "p.value" ) ) ) / sqrt( nSize ),
                        sd( unname( sapply( cReal.adf, `[[`, "p.value" ) ) ) / sqrt( nSize ),
                        sd( unname( sapply( iR.adf, `[[`, "p.value" ) ) ) / sqrt( nSize ),
                        sd( unname( sapply( A.adf, `[[`, "p.value" ) ) ) / sqrt( nSize ),
                        sd( unname( sapply( wAvgReal.adf, `[[`, "p.value" ) ) ) / sqrt( nSize ),
                        sd( unname( sapply( sT.adf, `[[`, "p.value" ) ) ) / sqrt( nSize ),
                        sd( unname( sapply( TeAvg.adf, `[[`, "p.value" ) ) ) / sqrt( nSize ),

                        mean( unname( sapply( gdp.bpf.adf, `[[`, "statistic" ) ) ),
                        mean( unname( sapply( cReal.bpf.adf, `[[`, "statistic" ) ) ),
                        mean( unname( sapply( iR.bpf.adf, `[[`, "statistic" ) ) ),
                        mean( unname( sapply( A.bpf.adf, `[[`, "statistic" ) ) ),
                        mean( unname( sapply( wAvgReal.bpf.adf, `[[`, "statistic" ) ) ),
                        mean( unname( sapply( sT.bpf.adf, `[[`, "statistic" ) ) ),
                        mean( unname( sapply( TeAvg.bpf.adf, `[[`, "statistic" ) ) ),

                        sd( unname( sapply( gdp.bpf.adf, `[[`, "statistic" ) ) ) / sqrt( nSize ),
                        sd( unname( sapply( cReal.bpf.adf, `[[`, "statistic" ) ) ) / sqrt( nSize ),
                        sd( unname( sapply( iR.bpf.adf, `[[`, "statistic" ) ) ) / sqrt( nSize ),
                        sd( unname( sapply( A.bpf.adf, `[[`, "statistic" ) ) ) / sqrt( nSize ),
                        sd( unname( sapply( wAvgReal.bpf.adf, `[[`, "statistic" ) ) ) / sqrt( nSize ),
                        sd( unname( sapply( sT.bpf.adf, `[[`, "statistic" ) ) ) / sqrt( nSize ),
                        sd( unname( sapply( TeAvg.bpf.adf, `[[`, "statistic" ) ) ) / sqrt( nSize ),

                        mean( unname( sapply( gdp.bpf.adf, `[[`, "p.value" ) ) ),
                        mean( unname( sapply( cReal.bpf.adf, `[[`, "p.value" ) ) ),
                        mean( unname( sapply( iR.bpf.adf, `[[`, "p.value" ) ) ),
                        mean( unname( sapply( A.bpf.adf, `[[`, "p.value" ) ) ),
                        mean( unname( sapply( wAvgReal.bpf.adf, `[[`, "p.value" ) ) ),
                        mean( unname( sapply( sT.bpf.adf, `[[`, "p.value" ) ) ),
                        mean( unname( sapply( TeAvg.bpf.adf, `[[`, "p.value" ) ) ),

                        sd( unname( sapply( gdp.bpf.adf, `[[`, "p.value" ) ) ) / sqrt( nSize ),
                        sd( unname( sapply( cReal.bpf.adf, `[[`, "p.value" ) ) ) / sqrt( nSize ),
                        sd( unname( sapply( iR.bpf.adf, `[[`, "p.value" ) ) ) / sqrt( nSize ),
                        sd( unname( sapply( A.bpf.adf, `[[`, "p.value" ) ) ) / sqrt( nSize ),
                        sd( unname( sapply( wAvgReal.bpf.adf, `[[`, "p.value" ) ) ) / sqrt( nSize ),
                        sd( unname( sapply( sT.bpf.adf, `[[`, "p.value" ) ) ) / sqrt( nSize ),
                        sd( unname( sapply( TeAvg.bpf.adf, `[[`, "p.value" ) ) ) / sqrt( nSize ),

                        mean(gdp.sd ), mean(cReal.sd ), mean(iR.sd ), mean(A.sd ),
                        mean(wAvgReal.sd ), mean(sT.sd ), mean(TeAvg.sd ),

                        sd( gdp.sd ) / sqrt( nSize ), sd( cReal.sd ) / sqrt( nSize ),
                        sd( iR.sd ) / sqrt( nSize ), sd( A.sd ) / sqrt( nSize ),
                        sd( wAvgReal.sd ) / sqrt( nSize ), sd( sT.sd ) / sqrt( nSize ),
                        sd( TeAvg.sd ) / sqrt( nSize ),

                        1, mean( cReal.sd ) / mean( gdp.sd ), mean( iR.sd ) / mean( gdp.sd ),
                        mean( A.sd ) / mean( gdp.sd ), mean( wAvgReal.sd ) / mean( gdp.sd ),
                        mean( sT.sd ) / mean( gdp.sd ), mean( TeAvg.sd ) / mean(gdp.sd ) ),

                      ncol = 7, byrow = T)
  colnames( key.stats ) <- c( "GDP (output)", "Consumption", "Investment", "Product.",
                              "Real wage", "Ten. skills", "Tenure" )
  rownames( key.stats ) <- c( "avg. growth rate", " (s.e.)",
                              "ADF test (logs)", " (s.e.)", " (p-val.)", " (s.e.)",
                              "ADF test (bpf)", " (s.e.)", " (p-val.)", " (s.e.)",
                              " s.d. (bpf)", " (s.e.)",
                              " relative s.d. (GDP)" )

  textplot( formatC( key.stats, digits = sDigits, format = "g" ), cmar = 2 )
  title <- paste( "Key statistics and unit roots tests for cycles (", legends[ k ], ")" )
  subTitle <- paste( eval( bquote(paste0( "( bpf: Baxter-King bandpass-filtered series, low = ", .( lowP ),
                                          "Q / high = ", .( highP ), "Q / order = ", .( bpfK ),
                                          " / MC runs = ", .( nSize ), " / period = ",
                                          .( warmUpStat + 1 ), " - ", .( nTstat ), " )" ) ) ),
                     eval( bquote( paste0( "( ADF test H0: there are unit roots / non-stationary at ",
                                           .( (1 - CI ) * 100), "% level", " )" ) ) ), sep ="\n" )
  title( main = title, sub = subTitle )

  #
  # ---- Correlation structure tables (lags, standard errors and p-values) ----
  #

  corr.struct.1 <- matrix(c(colMeans(t( unname( sapply(gdp.gdp, `[[`, "acf" ) ) ), na.rm = T),
                            colSds(t( unname( sapply(gdp.gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                            gdp.gdp.pval,

                            colMeans(t( unname( sapply(cReal.gdp, `[[`, "acf" ) ) ), na.rm = T),
                            colSds(t( unname( sapply(cReal.gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                            cReal.gdp.pval,

                            colMeans(t( unname( sapply(iR.gdp, `[[`, "acf" ) ) ), na.rm = T),
                            colSds(t( unname( sapply(iR.gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                            iR.gdp.pval,

                            colMeans(t( unname( sapply(EI.gdp, `[[`, "acf" ) ) ), na.rm = T),
                            colSds(t( unname( sapply(EI.gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                            EI.gdp.pval,

                            colMeans(t( unname( sapply(dN.gdp, `[[`, "acf" ) ) ), na.rm = T),
                            colSds(t( unname( sapply(dN.gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                            dN.gdp.pval,

                            colMeans(t( unname( sapply(U.gdp, `[[`, "acf" ) ) ), na.rm = T),
                            colSds(t( unname( sapply(U.gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                            U.gdp.pval,

                            colMeans(t( unname( sapply(A.gdp, `[[`, "acf" ) ) ), na.rm = T),
                            colSds(t( unname( sapply(A.gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                            A.gdp.pval,

                            colMeans(t( unname( sapply(mu2avg.gdp, `[[`, "acf" ) ) ), na.rm = T),
                            colSds(t( unname( sapply(mu2avg.gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                            mu2avg.gdp.pval,

                            colMeans(t( unname( sapply(totDeb.gdp, `[[`, "acf" ) ) ), na.rm = T),
                            colSds(t( unname( sapply(totDeb.gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                            totDeb.gdp.pval,

                            colMeans(t( unname( sapply(liqSale.gdp, `[[`, "acf" ) ) ), na.rm = T),
                            colSds(t( unname( sapply(liqSale.gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                            liqSale.gdp.pval,

                            colMeans(t( unname( sapply(bankrup.gdp, `[[`, "acf" ) ) ), na.rm = T),
                            colSds(t( unname( sapply(bankrup.gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                            bankrup.gdp.pval ),

                          ncol = 2 * lags + 1, byrow = T)
  colnames( corr.struct.1 ) <- gdp.gdp[[1]]$lag
  rownames( corr.struct.1 ) <- c( "GDP (output)", " (s.e.)", " (p-val.)",
                                  "Consumption", " (s.e.)", " (p-val.)",
                                  "Investment", " (s.e.)", " (p-val.)",
                                  "Net investment", " (s.e.)", " (p-val.)",
                                  "Change in inventories", " (s.e.)", " (p-val.)",
                                  "Unemployment rate", " (s.e.)", " (p-val.)",
                                  "Productivity", " (s.e.)", " (p-val.)",
                                  "Mark-up (sector 2)", " (s.e.)", " (p-val.)",
                                  "Total firm debt", " (s.e.)", " (p-val.)",
                                  "Liquidity-to-sales ratio", " (s.e.)", " (p-val.)",
                                  "Bankruptcy rate", " (s.e.)", " (p-val.)" )

  corr.struct.2 <- matrix(c(colMeans(t( unname( sapply(gdp.gdp, `[[`, "acf" ) ) ), na.rm = T),
                          colSds(t( unname( sapply(gdp.gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                          gdp.gdp.pval,

                          colMeans(t( unname( sapply(cReal.gdp, `[[`, "acf" ) ) ), na.rm = T),
                          colSds(t( unname( sapply(cReal.gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                          cReal.gdp.pval,

                          colMeans(t( unname( sapply(iR.gdp, `[[`, "acf" ) ) ), na.rm = T),
                          colSds(t( unname( sapply(iR.gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                          iR.gdp.pval,

                          colMeans(t( unname( sapply(A.gdp, `[[`, "acf" ) ) ), na.rm = T),
                          colSds(t( unname( sapply(A.gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                          A.gdp.pval,

                          colMeans( t( unname( sapply( netEntr.gdp, `[[`, "acf" ) ) ), na.rm = T ),
                          colSds( t( unname( sapply( netEntr.gdp, `[[`, "acf" ) ) ), na.rm = T ) /
                            sqrt( nSize ),
                          netEntr.gdp.pval,

                          colMeans( t( unname( sapply( entry.gdp, `[[`, "acf" ) ) ), na.rm = T ),
                          colSds( t( unname( sapply( entry.gdp, `[[`, "acf" ) ) ), na.rm = T ) /
                            sqrt( nSize ),
                          entry.gdp.pval,

                          colMeans(t( unname( sapply(wAvgReal.gdp, `[[`, "acf" ) ) ), na.rm = T),
                          colSds(t( unname( sapply(wAvgReal.gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                          wAvgReal.gdp.pval,

                          colMeans(t( unname( sapply(U.gdp, `[[`, "acf" ) ) ), na.rm = T),
                          colSds(t( unname( sapply(U.gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                          U.gdp.pval,

                          colMeans(t( unname( sapply(V.gdp, `[[`, "acf" ) ) ), na.rm = T),
                          colSds(t( unname( sapply(V.gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                          V.gdp.pval,

                          colMeans(t( unname( sapply(Lent.gdp, `[[`, "acf" ) ) ), na.rm = T),
                          colSds(t( unname( sapply(Lent.gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                          Lent.gdp.pval,

                          colMeans(t( unname( sapply(Lexit.gdp, `[[`, "acf" ) ) ), na.rm = T),
                          colSds(t( unname( sapply(Lexit.gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                          Lexit.gdp.pval),

                          ncol = 2 * lags + 1, byrow = T)
  colnames( corr.struct.2 ) <- gdp.gdp[[1]]$lag
  rownames( corr.struct.2 ) <- c( "GDP (output)", " (s.e.)", " (p-val.)",
                                "Consumption", " (s.e.)", " (p-val.)",
                                "Investment", " (s.e.)", " (p-val.)",
                                "Productivity", " (s.e.)", " (p-val.)",
                                "Net entry", " (s.e.)", " (p-val.)",
                                "Entry", " (s.e.)", " (p-val.)",
                                "Wage", " (s.e.)", " (p-val.)",
                                "Unemployment rate", " (s.e.)", " (p-val.)",
                                "Vacancy rate", " (s.e.)", " (p-val.)",
                                "Hiring rate", " (s.e.)", " (p-val.)",
                                "Firing rate", " (s.e.)", " (p-val.)" )

  title <- paste( "Correlation structure for GDP (", legends[ k ], ")" )
  subTitle <- paste( eval( bquote( paste0( "( non-rate/ratio series are Baxter-King bandpass-filtered, low = ",
                                           .( lowP ), "Q / high = ", .( highP ), "Q / order = ", .( bpfK ),
                                           " / MC runs = ", .( nSize ), " / period = ",
                                           .( warmUpStat + 1 ), " - ", .( nTstat ), " )" ) ) ),
                     eval( bquote ( paste0( "( test H0: lag coefficient is not significant at ",
                                            .( ( 1 - CI ) * 100), "% level", " )" ) ) ), sep ="\n" )

  textplot( formatC( corr.struct.1, digits = sDigits, format = "g" ), cmar = 1 )
  title( main = title, sub = subTitle )
  textplot( formatC( corr.struct.2, digits = sDigits, format = "g" ), cmar = 1 )
  title( main = title, sub = subTitle )
}

#
# ======= STATIONARITY AND ERGODICITY TESTS =======
#

# select data to plot
for( k in 1 : nExp ){

  statErgo <- ergod.test.lsd( mcData[[ k ]][ TmaskStat, , ], signif = 1 - CI,
                              vars = statErgo.vars )

  # plot table
  textplot( statErgo, cmar = 2, rmar = 0.5 )
  title( main = paste( "Stationarity, i.i.d. and ergodicity tests (", legends[ k ], ")" ),
         sub = paste( "( average p-values for testing H0 and rate of rejection of H0 / MC runs =", nSize, "/ period =", warmUpStat + 1, "-", nTstat, ")\n ( ADF/PP H0: non-stationary, KPSS H0: stationary, BDS H0: i.i.d., KS/AD/WW H0: ergodic )\n( significance =",
                      1 - CI, ")" ) )
}

#
# ======= COMPARISON OF EXPERIMENTS =======
#

box_plots( mcData, nExp, nSize, TmaxStat, TmaskStat, warmUpStat,
           nTstat, legends, legendList, sDigits, bPlotCoef,
           bPlotNotc, folder, repName )


# Close plot file
dev.off( )
