#******************************************************************
#
# ----------------- Micro-macro Aggregates analysis ---------------
#
#******************************************************************

#******************************************************************
#
# ------------ Read Monte Carlo experiment files ----------------
#
#******************************************************************

folder    <- "./finance"                  # data files folder
baseName  <- "debt_payment_"                     # data files base name (same as .lsd file)
nExp      <- 3                          # number of experiments (sensitivity/different cases)
iniDrop   <- 0                          # initial time steps to drop from analysis (0=none)
nKeep     <- -1                         # number of time steps to keep (-1=all)
cores     <- 0                          # maximum number of cores to allocate (0=all)
savDat    <- F                          # save processed data files and re-use if available?

expVal <- c( "Baseline", "0.01", "0.05" )                           # case parameter values

# Aggregated variables to use
logVars <- c( "Real_GDP",               # Real GDP
              "C_r",                    # Real Consumption
              "I_r",                    # Real Investment
              "G_r",                    # Real Government Expenses
              "M_r",                    # Real Imports
              "X_r",                    # Real Exports
              "NX_r",                   # Real Net Exports
              "K_r",                    # Real Stock of Capital
              "INVE_r",                 # Real Stock of Inventories
              "P",                      # Price Index
              "PROFITS",                # Real Profits
              "WAGE",                   # Real Wages
              "DEBT",                   # Private Stock of Debt
              "PDEBT",                  # Public Stock of Debt
              "PROD",                   # Average Labor Productivity
              "MK",                     # Average Markup
              "EMP"                     # Employment in Hours of Labor
              )

aggrVars <- append( logVars, 
                    c(  "GDP_G",        # Real GDP Growth
                        "P_G",          # Price Growth, Inflation
                        "CON_G",        # Real Consumption Growth
                        "INV_G",        # Real Investment Growth
                        "GOV_G",        # Real Government Expenses Growth
                        "M_G",          # Real Imports Growth
                        "X_G",          # Real Exports Growth
                        "NX_G",         # Real Net Exports Growth
                        "K_G",          # Real Stock of Capital Growth
                        "INVE_G",       # Real Stock of Inventories Growth
                        "PROFITS_G",    # Real Profits Growth
                        "WAGE_G",       # Real Wages Growth
                        "DEBT_G",       # Private Stock of Debt Growth
                        "PDEBT_G",      # Public Sotck of Debt Growth
                        "PROD_G",       # Labor Productivity Growth
                        "MK_G",         # Markup Growth
                        "U",            # Unemployment Rate
                        "Profit_Share", # Profit Share 
                        "Wage_Share"    # Wage Share
                        ) )

# Variables to test for stationarity and ergodicity
    statErgo.vars <- c( "GDP_G",        # Real GDP Growth
                        "P_G",          # Price Growth, Inflation
                        "CON_G",        # Real Consumption Growth
                        "INV_G",        # Real Investment Growth
                        "GOV_G",        # Real Government Expenses Growth
                        "M_G",          # Real Imports Growth
                        "X_G",          # Real Exports Growth
                        "NX_G",         # Real Net Exports Growth
                        "K_G",          # Real Stock of Capital Growth
                        "INVE_G",       # Real Stock of Inventories Growth
                        "PROFITS_G",    # Real Profits Growth
                        "WAGE_G",       # Real Wages Growth
                        "DEBT_G",       # Private Stock of Debt Growth
                        "PDEBT_G",      # Public Sotck of Debt Growth
                        "PROD_G",       # Labor Productivity Growth
                        "MK_G",         # Markup Growth
                        "U",            # Unemployment Rate
                        "Profit_Share", # Profit Share 
                        "Wage_Share"    # Wage Share
                    )

# Temporary data file suffix
datFilSfx <- "_aggr.Rdata"

# ==== Log start mark ====

cat( "\nMicro-macro aggregates analysis\n=======================\n" )
cat( "\n", as.character( Sys.time( ) ), "-> Start processing...\n\n" )
startTime <- proc.time( )       # register current time
options( warn = -1 )

# ==== Read command line parameters (if any) ====
args <- commandArgs( trailingOnly = TRUE )
cat( "Command line arguments: ", args, "\n" )

if( length ( args ) > 0 ){  # first parameter has to be the folder
  folder <- args [1]
}
if( length ( args ) > 1 ){  # second parameter has to be the base name
  baseName <- args [2]
}
if( length ( args ) > 2 ){  # third parameter has to be the number of experiments
  nExp <- as.integer( args [3] )
}
if( length ( args ) > 3 ){  # fourth parameter has to be the initial time period ( 0 is all )
  iniDrop <- as.integer( args [4] )
}
if( length ( args ) > 4 ){  # fifth parameter has to be the end periods to remove ( -1 is all )
  nKeep <- as.integer( args [5] )
}
if( length ( args ) > 5 ){  # sixth parameter has to be the number of cores ( 0 is all )
  cores <- as.integer( args [6] )
}
if( length ( args ) > 6 ){  # seventh parameter has to be the intermediate data saving flag
  savDat <- as.logical( args [7] )
}

cat( " Folder =", folder, "\n" )
cat( " Base name =", baseName, "\n" )
cat( " Number of experiments =", nExp, "\n" )
cat( " Initial time steps to drop =", iniDrop, "\n" )
cat( " Time steps to keep =", nKeep, "\n" )
cat( " Maximum cores to use =", cores, "\n" )
cat( " Re-use data files =", savDat, "\n\n" )

# ==== Process LSD result files ====
# Package with LSD interface functions
library( LSDinterface, verbose = FALSE, quietly = TRUE )
library( parallel, verbose = FALSE, quietly = TRUE )

# ==== Read data files ----

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
  
  cat( "\nData files: ", myFiles, "\n" )
  
  # Read data from text files and format it as a 3D array with labels
  mc <- read.3d.lsd( myFiles, aggrVars, skip = iniDrop, nrows = nKeep, nnodes = lsdCores )
  
  # Get dimensions information
  nTsteps <- dim( mc )[ 1 ]              # number of time steps
  nVar <- dim( mc )[ 2 ]                 # number of variables
  nSize  <- dim( mc )[ 3 ]               # Monte Carlo sample size
  
  cat( "\nData details:\n" )
  cat( " Number of MC cases =", nSize, "\n" )
  cat( " Number of variables =", nVar, "\n" )
  cat( " Number of used periods =", nTsteps, "\n\n" )
  
  # Compute Monte Carlo averages and std. deviation and store in 2D arrrays
  stats <- info.stats.lsd( mc )
  
  # Insert a t column
  t <- as.integer( rownames( stats$avg ) )
  A <- as.data.frame( cbind( t, stats$avg ) )
  S <- as.data.frame( cbind( t, stats$sd ) )
  M <- as.data.frame( cbind( t, stats$max ) )
  m <- as.data.frame( cbind( t, stats$min ) )
  
  # Write to the disk as (European) CSV files for Excel
  write.csv( A, paste0( folder, "/", baseName, exper, "_aggr_avg.csv" ),
             row.names = FALSE, quote = FALSE )
  write.csv( S, paste0( folder, "/", baseName, exper, "_aggr_sd.csv" ),
             row.names = FALSE, quote = FALSE )
  write.csv( M, paste0( folder, "/", baseName, exper, "_aggr_max.csv" ),
             row.names = FALSE, quote = FALSE )
  write.csv( m, paste0( folder, "/", baseName, exper, "_aggr_min.csv" ),
             row.names = FALSE, quote = FALSE )
  
  # Save temporary results to disk to save memory
  tmpFile <- paste0( folder, "/", baseName, exper, datFilSfx )
  save( mc, A, S, M, m, nTsteps, nVar, nSize, file = tmpFile )
  
  return( tmpFile )
}

# only reprocess results file if requested/needed
if( savDat ) {
  
  tmpFiles <- list( )
  noDat <- FALSE
  
  # check all .dat files exist and are newer than .res files
  for( i in 1 : nExp ) {
    tmpFiles[[ i ]] <- paste0( folder, "/", baseName, i, datFilSfx )
    if( ! file.exists( tmpFiles[[ i ]] ) )
      noDat <- TRUE
    else {
      myFiles <- list.files( path = folder, pattern = paste0( baseName, i, "_[0-9]+.res" ),
                             full.names = TRUE )
      # if any .res file is newer, redo everything
      if( length( myFiles ) > 0 &&
          max( file.mtime( myFiles ) ) > file.mtime( tmpFiles[[ i ]] ) ) {
        if( ! noDat )
          cat( "New data files detected, removing previously saved data...\n\n" )
        unlink( tmpFiles[[ i ]] )
        noDat <- TRUE
      }
    }
  }
  
  if( ! noDat )
    cat( "Re-using previously saved data...\n" )
}

if( ! savDat || noDat ) {
  
  cat( "Reading data from files...\n" )
  
  # configure clusters for 2 level parallel loading
  if( cores == 0 )
    cores <- detectCores( )
  cores <- min( cores, detectCores( ) )
  lsdCores <- 1
  if( cores != 1 ) {
    # fully allocate cores (round up to ensure 100% utilization)
    if( cores > nExp )
      lsdCores <- ceiling( cores / nExp )
    
    # initiate cluster for parallel loading
    cl <- makeCluster( min( nExp, cores ) )
    
    # configure cluster: export required variables & packages
    clusterExport( cl, c( "nExp", "folder", "baseName", "aggrVars", "iniDrop",
                          "nKeep", "datFilSfx", "lsdCores" ) )
    invisible( clusterEvalQ( cl, library( LSDinterface ) ) )
    
    # load each experiment in parallel
    tmpFiles <- parLapplyLB( cl, 1 : nExp, readExp )
    
    stopCluster( cl )
    
  } else {
    
    # load each experiment serially
    tmpFiles <- lapply( 1 : nExp, readExp )
  }
}

# ==== Organize data read from files ----

# fill the lists to hold data
mcData <- list() # 3D Monte Carlo data
Adata <- list()  # average data
Sdata <- list()  # standard deviation data
Mdata <- list()  # maximum data
mdata <- list()  # minimum data
nTsteps.1 <- nSize.1 <- 0

for( k in 1 : nExp ) {                      # realocate data in separate lists
  
  load( tmpFiles[[ k ]] )                   # pick data from disk
  if( ! savDat )
    file.remove( tmpFiles[[ k ]] )          # and delete temporary file, if needed
  
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
# ===================== User parameters =========================

bCase     <- 1      # experiment to be used as base case
CI        <- 0.95   # desired confidence interval
warmUpPlot<- 200    # number of "warm-up" runs for plots
nTplot    <- -1     # last period to consider for plots (-1=all)
warmUpStat<- 200    # warm-up runs to evaluate all statistics
nTstat    <- -1     # last period to consider for statistics (-1=all)
bPlotCoef <- 1.5    # boxplot whiskers extension from the box (0=extremes)
bPlotNotc <- FALSE  # use boxplot notches
lowP      <- 6      # bandpass filter minimum period
highP     <- 32     # bandpass filter maximum period
bpfK      <- 12     # bandpass filter order
lags      <- 4      # lags to analyze
smoothing <- 1600   # HP filter smoothing factor (lambda)
crisisTh  <- 0.00   # crisis growth threshold
crisisLen <- 3      # crisis minimum duration (periods)
crisisPre <- 4      # pre-crisis period to base trend start (>=1)
crisisRun <- 0      # the crisis case to be plotted (0=auto)
crisesPlt <- TRUE   # plot all the crisis plots in a separate pdf file?

repName   <- "MMM"  # report files base name (if "" same baseName)
sDigits   <- 4      # significant digits in tables
transMk   <- -1     # regime transition mark after warm-up (-1:none)
plotRows  <- 1      # number of plots per row in a page
plotCols  <- 1  	  # number of plots per column in a page
plotW     <- 10     # plot window width
plotH     <- 7      # plot window height
raster    <- FALSE  # raster or vector plots
res       <- 600    # resolution of raster mode (in dpi)

# Colors assigned to each experiment's lines in graphics
colors <- c( "black", "blue", "red", "orange", "green", "brown", "yellow", "purple" )
#colors <- rep( "black", 6 )

# Line types assigned to each experiment
lTypes <- c( "solid", "solid", "solid", "solid", "solid", "solid", "solid", "solid")
#lTypes <- c( "solid", "dashed", "dotted", "dotdash", "longdash", "twodash" )

# Point types assigned to each experiment
pTypes <- c( 4, 4, 4, 4, 4, 4, 4, 4 )
#pTypes <- c( 4, 0, 1, 2, 3, 5 )

# ====== External support functions & definitions ======

library("LSDsensitivity")
source( "support-functions.R" )
source( "time-plots.R" )
source( "box-plots.R" )

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
    TRUE
  } else {
    # Open PDF plot file for output
    pdf( paste0( folder, "/", outDir, "/", repName, "_aggr_plots.pdf" ),
         width = plotW, height = plotH )
    par( mfrow = c ( plotRows, plotCols ) )             # define plots per page
    png( paste0( folder, "/", outDir, "/", repName, "_aggr_plots_%d.png" ),
         width = plotW, height = plotH, units = "in", res = res )
  }
  
  #
  # ====== MC PLOTS GENERATION ======
  #
  
  cat( "\nProcessing experiments and generating reports...\n")
  
  time_plots( mcData, Adata, mdata, Mdata, Sdata, nExp, nSize, nTsteps, TmaskPlot,
              CI, legends, colors, lTypes, transMk, smoothing )
  
  
  box_plots( mcData, nExp, nSize, TmaxStat, TmaskStat, warmUpStat,
                         nTstat, legends, legendList, sDigits, bPlotCoef,
                         bPlotNotc, folder, outDir, repName )
  
  #
  # ====== STATISTICS GENERATION ======
  #
  
  # Create vectors and lists to hold the Monte Carlo results
  gdp_gr <- infla <- prod_gr <- cr_gr <- ir_gr <- gr_gr <-
    mr_gr <- profits_gr <- wage_gr <- debt_gr <- pdebt_gr <- 
    mk_gr <- u_gr <- x_gr <- nx_gr <- pr_sh <- wg_sh <- k_gr <- inve_gr <- emp_gr <-
    vector( mode = "numeric", length = nSize )
  
  gdp_r_sd <- con_r_sd <- inv_r_sd <- gov_r_sd <- imp_r_sd <- 
    p_sd <- profits_sd <- wage_sd <- debt_sd <- pdebt_sd <- prod_sd <- 
    u_sd <- emp_sd <- pr_sh_sd <- wg_sh_sd <- mk_sd <- 
    inve_sd <- k_sd <- x_r_sd <- nx_r_sd <-
    vector( mode = 'numeric', length = nSize )
  
  gdp_gdp <- cr_gdp <- ir_gdp <- gov_gdp <- imp_gdp <- x_gdp <- nx_gdp <- 
    p_gdp <- profits_gdp <- wage_gdp <- pr_sh_gdp <- wg_sh_gdp <- debt_gdp <- pdebt_gdp <- 
    prod_gdp <- mk_gdp <- inve_gdp <- k_gdp <- u_gdp <- emp_gdp <-  list( )
  
  gdp_gdp_pval <- cr_gdp_pval <- ir_gdp_pval <- gov_gdp_pval <- imp_gdp_pval <- x_gdp_pval <- nx_gdp_pval <- 
    p_gdp_pval <- profits_gdp_pval <- wage_gdp_pval <- pr_sh_gdp_pval <- wg_sh_gdp_pval <- debt_gdp_pval <- pdebt_gdp_pval <- 
    prod_gdp_pval <- mk_gdp_pval <- inve_gdp_pval <- k_gdp_pval <- u_gdp_pval <- emp_gdp_pval <- 
    vector( mode = "numeric", length = nExp )
  
  adf_gdp_r <- adf_con_r <- adf_inv_r <- adf_gov_r <- adf_imp_r <- adf_p <-
    adf_profits <- adf_wage <- adf_debt <- adf_pdebt <- adf_prod <- adf_x <-
    adf_nx <- adf_k <- adf_inve <- adf_pr_sh <- adf_wg_sh <- adf_mk <- adf_emp <- adf_u <- list( )
  
  for(k in 1 : nExp){ # Experiment k
    
    #
    # ---- Bandpass filtered GDP, consumption and investment cycles graphic ----
    #
    
    plot_bpf( list( log0( Adata[[ k ]]$Real_GDP ), log0( Adata[[ k ]]$C_r ), 
                    log0( Adata[[ k ]]$I_r ), log0(  Adata[[ k ]]$G_r ) ),
              pl = lowP, pu = highP, nfix = bpfK, mask = TmaskPlot,
              mrk = transMk, # uncomment to add vertical line in selected point (e.g. mark of regime change)
              col = colors, lty = lTypes,
              leg = c( "GDP", "Consumption", "Investment", "Gov. Expenditure" ),
              xlab = "Time", ylab = "Filtered series",
              tit = paste( "GDP cycles (", legends[ k ], ")" ),
              subtit = paste( "( Baxter-King bandpass-filtered, low = 6Q / high = 32Q / order = 12 / MC runs =",
                              nSize, ")" ) )
    
    #
    # ==== Statistics computation for tables ====
    #
    
    for( j in 1 : nSize ){  # Execute for every Monte Carlo run
      
      # Monte carlo average growth rates
      gdp_gr[ j ]       <- mcData[[ k ]][ nTstat, "GDP_G", j ]
      infla [ j ]       <- mcData[[ k ]][ nTstat, "P_G", j ]
      cr_gr[ j ]        <- mcData[[ k ]][ nTstat, "CON_G", j ]
      ir_gr[ j ]        <- mcData[[ k ]][ nTstat, "INV_G", j ]
      gr_gr[ j ]        <- mcData[[ k ]][ nTstat, "GOV_G", j ]
      mr_gr[ j ]        <- mcData[[ k ]][ nTstat, "M_G", j ]
      profits_gr[ j ]   <- mcData[[ k ]][ nTstat, "PROFITS_G", j ]
      wage_gr[ j ]      <- mcData[[ k ]][ nTstat, "WAGE_G", j ]
      debt_gr[ j ]      <- mcData[[ k ]][ nTstat, "DEBT_G", j ]
      pdebt_gr[ j ]     <- mcData[[ k ]][ nTstat, "PDEBT_G", j ]
      x_gr [ j ]        <- mcData[[ k ]][ nTstat, "X_G", j ]
      nx_gr [ j ]       <- mcData[[ k ]][ nTstat, "NX_G", j ]
      inve_gr [ j ]     <- mcData[[ k ]][ nTstat, "INVE_G", j ]
      k_gr [ j ]        <- mcData[[ k ]][ nTstat, "K_G", j ]
      pr_sh [ j ]       <- mcData[[ k ]][ nTstat, "Profit_Share", j ]
      wg_sh [ j ]       <- mcData[[ k ]][ nTstat, "Wage_Share", j ]
      prod_gr [ j ]     <- mcData[[ k ]][ nTstat, "PROD_G", j ]
      u_gr [ j ]        <- mcData[[ k ]][ nTstat, "U", j ]
      mk_gr [ j ]       <- mcData[[ k ]][ nTstat, "MK_G", j ]
      emp_gr [ j ]      <- mcData[[ k ]][ nTstat, "EMP", j ]
      
      # Apply Baxter-King filter to the series
      gdp_bpf     <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "Real_GDP", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      con_bpf     <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "C_r", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      inv_bpf     <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "I_r", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      gov_bpf     <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "G_r", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      imp_bpf     <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "M_r", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      p_bpf       <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "P", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      profits_bpf <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "PROFITS", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      wage_bpf    <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "WAGE", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      debt_bpf    <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "DEBT", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      pdebt_bpf   <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "PDEBT", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      prod_bpf    <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "PROD", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      u_bpf       <- bkfilter( mcData[[ k ]][ TmaskStat, "U", j ] , pl = lowP, pu = highP, nfix = bpfK )
      x_bpf       <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "X_r", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      nx_bpf      <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "NX_r", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      k_bpf       <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "K_r", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      inve_bpf    <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "INVE_r", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      emp_bpf     <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "EMP", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      pr_sh_bpf   <- bkfilter( mcData[[ k ]][ TmaskStat, "Profit_Share", j ] , pl = lowP, pu = highP, nfix = bpfK )
      wg_sh_bpf   <- bkfilter( mcData[[ k ]][ TmaskStat, "Wage_Share", j ] , pl = lowP, pu = highP, nfix = bpfK )
      mk_bpf      <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "MK", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      
      # Augmented Dickey-Fuller tests for unit roots
      adf_gdp_r[[ j ]]    <- adf.test( log0( mcData[[ k ]][ TmaskStat, "Real_GDP", j ] ) )
      adf_con_r[[ j ]]    <- adf.test( log0( mcData[[ k ]][ TmaskStat, "C_r", j ] ) )
      adf_inv_r[[ j ]]    <- adf.test( log0( mcData[[ k ]][ TmaskStat, "I_r", j ] ) )
      adf_gov_r[[ j ]]    <- adf.test( log0( mcData[[ k ]][ TmaskStat, "G_r", j ] ) )
      adf_imp_r[[ j ]]    <- adf.test( log0( mcData[[ k ]][ TmaskStat, "M_r", j ] ) )
      adf_p[[ j ]]        <- adf.test( log0( mcData[[ k ]][ TmaskStat, "P", j ] ) )
      adf_profits[[ j ]]  <- adf.test( log0( mcData[[ k ]][ TmaskStat, "PROFITS", j ] ) )
      adf_wage[[ j ]]     <- adf.test( log0( mcData[[ k ]][ TmaskStat, "WAGE", j ] ) )
      adf_debt[[ j ]]     <- adf.test( log0( mcData[[ k ]][ TmaskStat, "DEBT", j ] ) )
      adf_pdebt[[ j ]]    <- adf.test( log0( mcData[[ k ]][ TmaskStat, "PDEBT", j ] ) )
      adf_prod[[ j ]]     <- adf.test( log0( mcData[[ k ]][ TmaskStat, "PROD", j ] ) )
      adf_x[[ j ]]        <- adf.test( log0( mcData[[ k ]][ TmaskStat, "X_r", j ] ) )
      adf_nx[[ j ]]       <- adf.test( log0( mcData[[ k ]][ TmaskStat, "NX_r", j ] ) )
      adf_k[[ j ]]        <- adf.test( log0( mcData[[ k ]][ TmaskStat, "K_r", j ] ) )
      adf_inve[[ j ]]     <- adf.test( log0( mcData[[ k ]][ TmaskStat, "INVE_r", j ] ) )
      adf_u[[ j ]]        <- adf.test( mcData[[ k ]][ TmaskStat, "U", j ] ) 
      adf_emp[[ j ]]      <- adf.test( log0( mcData[[ k ]][ TmaskStat, "EMP", j ] ) )
      adf_pr_sh[[ j ]]    <- adf.test( mcData[[ k ]][ TmaskStat, "Profit_Share", j ]  )
      adf_wg_sh[[ j ]]    <- adf.test( mcData[[ k ]][ TmaskStat, "Wage_Share", j ]  )
      adf_mk[[ j ]]       <- adf.test( log0( mcData[[ k ]][ TmaskStat, "MK", j ] ) )
      
      # Standard deviations of filtered series
      gdp_r_sd[ j ]       <- sd( gdp_bpf$cycle[ TmaskBpf, 1 ] )
      con_r_sd[ j ]       <- sd( con_bpf$cycle[ TmaskBpf, 1 ] )
      inv_r_sd[ j ]       <- sd( inv_bpf$cycle[ TmaskBpf, 1 ] )
      gov_r_sd[ j ]       <- sd( gov_bpf$cycle[ TmaskBpf, 1 ] )
      imp_r_sd[ j ]       <- sd( imp_bpf$cycle[ TmaskBpf, 1 ] )
      p_sd[ j ]           <- sd( p_bpf$cycle[ TmaskBpf, 1 ] )
      profits_sd[ j ]     <- sd( profits_bpf$cycle[ TmaskBpf, 1 ] )
      wage_sd[ j ]        <- sd( wage_bpf$cycle[ TmaskBpf, 1 ] )
      debt_sd[ j ]        <- sd( debt_bpf$cycle[ TmaskBpf, 1 ] )
      pdebt_sd[ j ]       <- sd( pdebt_bpf$cycle[ TmaskBpf, 1 ] )
      prod_sd[ j ]        <- sd( prod_bpf$cycle[ TmaskBpf, 1 ] )
      x_r_sd[ j ]         <- sd( x_bpf$cycle[ TmaskBpf, 1 ] )
      nx_r_sd[ j ]        <- sd( nx_bpf$cycle[ TmaskBpf, 1 ] )
      k_sd[ j ]           <- sd( k_bpf$cycle[ TmaskBpf, 1 ] )
      inve_sd[ j ]        <- sd( inve_bpf$cycle[ TmaskBpf, 1 ] )
      emp_sd[ j ]         <- sd( emp_bpf$cycle[ TmaskBpf, 1 ] )
      u_sd[ j ]           <- sd( u_bpf$cycle[ TmaskBpf, 1 ] )
      pr_sh_sd[ j ]       <- sd( pr_sh_bpf$cycle[ TmaskBpf, 1 ] )
      wg_sh_sd[ j ]       <- sd( wg_sh_bpf$cycle[ TmaskBpf, 1 ] )
      mk_sd[ j ]          <- sd( mk_bpf$cycle[ TmaskBpf, 1 ] )
      
      # Build the correlation structures
      gdp_gdp[[ j ]]  <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ],
                              gdp_bpf$cycle[ TmaskBpf, 1 ],
                              lag.max = lags, plot = FALSE, na.action = na.pass )
      cr_gdp[[ j ]] <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ],
                            con_bpf$cycle[ TmaskBpf, 1 ],
                            lag.max = lags, plot = FALSE, na.action = na.pass )
      ir_gdp[[ j ]] <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ],
                            inv_bpf$cycle[ TmaskBpf, 1 ],
                            lag.max = lags, plot = FALSE, na.action = na.pass )
      gov_gdp[[ j ]] <- ccf(gdp_bpf$cycle[ TmaskBpf, 1 ],
                            gov_bpf$cycle[ TmaskBpf, 1 ],
                            lag.max = lags, plot = FALSE, na.action = na.pass )
      imp_gdp[[ j ]] <- ccf(gdp_bpf$cycle[ TmaskBpf, 1 ],
                            imp_bpf$cycle[ TmaskBpf, 1 ],
                            lag.max = lags, plot = FALSE, na.action = na.pass )
      x_gdp[[ j ]] <- ccf(gdp_bpf$cycle[ TmaskBpf, 1 ],
                            x_bpf$cycle[ TmaskBpf, 1 ],
                            lag.max = lags, plot = FALSE, na.action = na.pass )
      nx_gdp[[ j ]] <- ccf(gdp_bpf$cycle[ TmaskBpf, 1 ],
                          nx_bpf$cycle[ TmaskBpf, 1 ],
                          lag.max = lags, plot = FALSE, na.action = na.pass )
      p_gdp[[ j ]] <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ],
                           p_bpf$cycle[ TmaskBpf, 1 ],
                           lag.max = lags, plot = FALSE, na.action = na.pass )
      profits_gdp[[ j ]] <- ccf(gdp_bpf$cycle[ TmaskBpf, 1 ],
                                profits_bpf$cycle[ TmaskBpf, 1 ],
                                lag.max = lags, plot = FALSE, na.action = na.pass )
      wage_gdp[[ j ]]    <- ccf(gdp_bpf$cycle[ TmaskBpf, 1 ],
                                wage_bpf$cycle[ TmaskBpf, 1 ],
                                lag.max = lags, plot = FALSE, na.action = na.pass )
      pr_sh_gdp[[ j ]] <- ccf(gdp_bpf$cycle[ TmaskBpf, 1 ],
                                pr_sh_bpf$cycle[ TmaskBpf, 1 ],
                                lag.max = lags, plot = FALSE, na.action = na.pass )
      wg_sh_gdp[[ j ]] <- ccf(gdp_bpf$cycle[ TmaskBpf, 1 ],
                                wg_sh_bpf$cycle[ TmaskBpf, 1 ],
                                lag.max = lags, plot = FALSE, na.action = na.pass )
      debt_gdp[[ j ]] <- ccf(gdp_bpf$cycle[ TmaskBpf, 1 ],
                              debt_bpf$cycle[ TmaskBpf, 1 ],
                              lag.max = lags, plot = FALSE, na.action = na.pass )
      pdebt_gdp[[ j ]] <- ccf(gdp_bpf$cycle[ TmaskBpf, 1 ],
                              pdebt_bpf$cycle[ TmaskBpf, 1 ],
                              lag.max = lags, plot = FALSE, na.action = na.pass )
      prod_gdp[[ j ]] <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ],
                              prod_bpf$cycle[ TmaskBpf, 1 ],
                              lag.max = lags, plot = FALSE, na.action = na.pass )
      u_gdp[[ j ]] <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ],
                           u_bpf$cycle[ TmaskBpf, 1 ],
                           lag.max = lags, plot = FALSE, na.action = na.pass )
      emp_gdp[[ j ]] <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ],
                           emp_bpf$cycle[ TmaskBpf, 1 ],
                           lag.max = lags, plot = FALSE, na.action = na.pass )
      inve_gdp[[ j ]] <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ],
                             inve_bpf$cycle[ TmaskBpf, 1 ],
                             lag.max = lags, plot = FALSE, na.action = na.pass )
      k_gdp[[ j ]] <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ],
                             k_bpf$cycle[ TmaskBpf, 1 ],
                             lag.max = lags, plot = FALSE, na.action = na.pass )
      mk_gdp[[ j ]] <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ],
                           mk_bpf$cycle[ TmaskBpf, 1 ],
                           lag.max = lags, plot = FALSE, na.action = na.pass )
      
    }
    
    # Applies t test to the mean lag results to test their significance (H0: lag < critCorr)
    for(i in 1 : (2 * lags + 1) ){ #do for all lags
      if(i != lags + 1)  # don't try to compute autocorrelation at lag 0
      gdp_gdp_pval[ i ] <- t.test0( abs( unname( sapply( gdp_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      cr_gdp_pval[ i ]  <- t.test0( abs( unname( sapply( cr_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      ir_gdp_pval[ i ]  <- t.test0( abs( unname( sapply( ir_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      gov_gdp_pval[ i ] <- t.test0( abs( unname( sapply( gov_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      imp_gdp_pval[ i ] <- t.test0( abs( unname( sapply( imp_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      x_gdp_pval[ i ]   <- t.test0( abs( unname( sapply( x_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      nx_gdp_pval[ i ]  <- t.test0( abs( unname( sapply( nx_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      p_gdp_pval[ i ]   <- t.test0( abs( unname( sapply( p_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      profits_gdp_pval[ i ]<- t.test0( abs( unname( sapply( profits_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      wage_gdp_pval[ i ]<- t.test0( abs( unname( sapply( wage_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      pr_sh_gdp_pval[ i ]<- t.test0( abs( unname( sapply( pr_sh_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      wg_sh_gdp_pval[ i ]<- t.test0( abs( unname( sapply( wg_sh_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      debt_gdp_pval[ i ]<- t.test0( abs( unname( sapply( debt_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      pdebt_gdp_pval[ i ]<- t.test0( abs( unname( sapply( pdebt_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      prod_gdp_pval[ i ]<- t.test0( abs( unname( sapply( prod_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      u_gdp_pval[ i ]   <- t.test0( abs( unname( sapply( u_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      emp_gdp_pval[ i ]   <- t.test0( abs( unname( sapply( emp_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      inve_gdp_pval[ i ] <- t.test0( abs( unname( sapply( inve_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      k_gdp_pval[ i ] <- t.test0( abs( unname( sapply( k_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      mk_gdp_pval[ i ] <- t.test0( abs( unname( sapply( mk_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
    }
    
    #
    # ---- Summary statistics table (averages, standard errors and p-values) ----
    #
    
    key.stats.1 <- matrix(
      c(
        ## avg. growth rate
        mean( gdp_gr ), 
        mean( cr_gr ), 
        mean( ir_gr ), 
        mean( gr_gr ), 
        mean( mr_gr ),
        mean( x_gr ), 
        mean( nx_gr ), 
        
        ## (s.e.)
        sd( gdp_gr ) / sqrt( nSize ), 
        sd( cr_gr ) / sqrt( nSize ),
        sd( ir_gr ) / sqrt( nSize ), 
        sd( gr_gr ) / sqrt( nSize ),
        sd( mr_gr ) / sqrt( nSize ),
        sd( x_gr ) / sqrt( nSize ),
        sd( nx_gr ) / sqrt( nSize ),
      
        ## ADF test (logs)
        mean( unname( sapply( adf_gdp_r, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_con_r, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_inv_r, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_gov_r, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_imp_r, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_x, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_nx, `[[`, "statistic" ) ) ),
        
        ## ADF test (logs) s.e.
        sd( unname( sapply( adf_gdp_r, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_con_r, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_inv_r, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_gov_r, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_imp_r, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_x, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_nx, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        
        ## ADF test (logs) p.value
        mean( unname( sapply( adf_gdp_r, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_con_r, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_inv_r, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_gov_r, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_imp_r, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_x, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_nx, `[[`, "p.value" ) ) ),
        
        ## ADF test (logs) p.value s.e.
        sd( unname( sapply( adf_gdp_r, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_con_r, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_inv_r, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_gov_r, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_imp_r, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_x, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_nx, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        
        ## S.d. of bpf series
        mean( gdp_r_sd ), 
        mean( con_r_sd ), 
        mean( inv_r_sd ),
        mean( gov_r_sd ), 
        mean( imp_r_sd ), 
        mean( x_r_sd ), 
        mean( nx_r_sd ), 
        
        ## S.e. of bpf series s.d.
        se( gdp_r_sd ) / sqrt( nSize ), 
        se( con_r_sd ) / sqrt( nSize ), 
        se( inv_r_sd ) / sqrt( nSize ), 
        se( gov_r_sd ) / sqrt( nSize ), 
        se( imp_r_sd ) / sqrt( nSize ), 
        se( x_r_sd ) / sqrt( nSize ), 
        se( nx_r_sd ) / sqrt( nSize ), 
        
        ## relative s.d. (to GDP)
        1, 
        mean( con_r_sd ) / mean( gdp_r_sd ), 
        mean( inv_r_sd ) / mean( gdp_r_sd ),
        mean( gov_r_sd ) / mean( gdp_r_sd ), 
        mean( imp_r_sd ) / mean( gdp_r_sd ),
        mean( x_r_sd ) / mean( gdp_r_sd ),
        mean( nx_r_sd ) / mean( gdp_r_sd )
        
      ),
      ncol = 7, byrow = T)
    
    colnames( key.stats.1 ) <- c( "GDP (output)", 
                                  "Consumption", 
                                  "Investment", 
                                  "Gov. Expend.",
                                  "Imports",
                                  "Exports",
                                  "Net Exports"
                                  )
    rownames( key.stats.1 ) <- c( "avg. growth rate", 
                                  " (s.e.)",
                                  "ADF test (logs)",
                                  " (s.e.)", 
                                  " (p-val.)", 
                                  " (s.e.)",
                                  " s.d. (bpf)", 
                                  " (s.e.)",
                                  " relative s.d. (GDP)" )
    
    textplot( formatC( key.stats.1, digits = sDigits, format = "g" ), cmar = 2 )
    title <- paste( "Key statistics and unit roots tests for cycles (", legends[ k ], ")" )
    subTitle <- paste( eval( bquote(paste0( "( bpf: Baxter-King bandpass-filtered series, low = ", .( lowP ),
                                            "Q / high = ", .( highP ), "Q / order = ", .( bpfK ),
                                            " / MC runs = ", .( nSize ), " / period = ",
                                            .( warmUpStat + 1 ), " - ", .( nTstat ), " )" ) ) ),
                       eval( bquote( paste0( "( ADF test H0: there are unit roots / non-stationary at ",
                                             .( (1 - CI ) * 100), "% level", " )" ) ) ), sep ="\n" )
    title( main = title, sub = subTitle )
    
    
    key.stats.2 <- matrix(
      c(
        ## avg. growth rate
        mean( infla ), 
        mean( profits_gr ), 
        mean( wage_gr ),
        mean( pr_sh ), 
        mean( wg_sh ), 
        mean( debt_gr ), 
        mean( pdebt_gr ), 
        
        ## (s.e.)
        sd( infla ) / sqrt( nSize ), 
        sd( profits_gr ) / sqrt( nSize ), 
        sd( wage_gr ) / sqrt( nSize ),
        sd( pr_sh ) / sqrt( nSize ),
        sd( wg_sh ) / sqrt( nSize ),
        sd( debt_gr ) / sqrt( nSize ), 
        sd( pdebt_gr ) / sqrt( nSize ),
        
        ## ADF test (logs)
        mean( unname( sapply( adf_p, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_profits, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_wage, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_pr_sh, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_wg_sh, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_debt, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_pdebt, `[[`, "statistic" ) ) ),
        
        ## ADF test (logs) s.e.
        sd( unname( sapply( adf_p, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_profits, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_wage, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_pr_sh, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_wg_sh, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_debt, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_pdebt, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        
        ## ADF test (logs) p.value
        mean( unname( sapply( adf_p, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_profits, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_wage, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_pr_sh, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_wg_sh, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_debt, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_pdebt, `[[`, "p.value" ) ) ),
        
        ## ADF test (logs) p.value s.e.
        sd( unname( sapply( adf_p, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_profits, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_wage, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_pr_sh, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_wg_sh, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_debt, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_pdebt, `[[`, "p.value" ) ) ) / sqrt( nSize ),
      
        ## S.d. of bpf series
        mean( p_sd ),
        mean( profits_sd ), 
        mean( wage_sd ),
        mean( pr_sh_sd ),
        mean( wg_sh_sd ),
        mean( debt_sd ),
        mean( pdebt_sd ), 
        
        ## S.e. of bpf series s.d.
        se( p_sd ) / sqrt( nSize ),
        se( profits_sd ) / sqrt( nSize ), 
        se( wage_sd ) / sqrt( nSize ),
        se( pr_sh_sd ) / sqrt( nSize ),
        se( wg_sh_sd ) / sqrt( nSize ),
        se( debt_sd ) / sqrt( nSize ), 
        se( pdebt_sd ) / sqrt( nSize ), 
        
        ## relative s.d. (to GDP)
        mean( p_sd ) / mean( gdp_r_sd ), 
        mean( profits_sd ) / mean( gdp_r_sd ),
        mean( wage_sd ) / mean( gdp_r_sd ), 
        mean( pr_sh_sd ) / mean( gdp_r_sd ),
        mean( wg_sh_sd ) / mean( gdp_r_sd ),
        mean( debt_sd ) / mean(gdp_r_sd ),
        mean( pdebt_sd ) / mean( gdp_r_sd )
        
      ),
      ncol = 7, byrow = T)
    
    colnames( key.stats.2 ) <- c( "Price level", 
                                  "Profit", 
                                  "Wages",
                                  "Profit Share",
                                  "Wage Share",
                                  "Debt", 
                                  "Gov. Debt"
                                 )
    rownames( key.stats.2 ) <- c( "avg. growth rate", 
                                  " (s.e.)",
                                  "ADF test (logs)",
                                  " (s.e.)", 
                                  " (p-val.)", 
                                  " (s.e.)",
                                  " s.d. (bpf)", 
                                  " (s.e.)",
                                  " relative s.d. (GDP)" )
    
    textplot( formatC( key.stats.2, digits = sDigits, format = "g" ), cmar = 2 )
    title <- paste( "Key statistics and unit roots tests for cycles (", legends[ k ], ")" )
    subTitle <- paste( eval( bquote(paste0( "( bpf: Baxter-King bandpass-filtered series, low = ", .( lowP ),
                                            "Q / high = ", .( highP ), "Q / order = ", .( bpfK ),
                                            " / MC runs = ", .( nSize ), " / period = ",
                                            .( warmUpStat + 1 ), " - ", .( nTstat ), " )" ) ) ),
                       eval( bquote( paste0( "( ADF test H0: there are unit roots / non-stationary at ",
                                             .( (1 - CI ) * 100), "% level", " )" ) ) ), sep ="\n" )
    title( main = title, sub = subTitle )
    
    
    key.stats.3 <- matrix(
      c(
        ## avg. growth rate
        mean( prod_gr ),
        mean( mk_gr ),
        mean( inve_gr ), 
        mean( k_gr ),
        mean( u_gr ), 
        mean( emp_gr ),
      
        ## (s.e.)
        sd( prod_gr ) / sqrt( nSize ),
        sd( mk_gr ) / sqrt( nSize ),
        sd( inve_gr ) / sqrt( nSize ),
        sd( k_gr ) / sqrt( nSize ),
        sd( u_gr ) / sqrt( nSize ),
        sd( emp_gr ) / sqrt( nSize ),
        
        ## ADF test (logs)
        mean( unname( sapply( adf_prod, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_mk, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_inve, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_k, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_u, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_emp, `[[`, "statistic" ) ) ),
        
        ## ADF test (logs) s.e.
        sd( unname( sapply( adf_prod, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_mk, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_inve, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_k, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_u, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_emp, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        
        ## ADF test (logs) p.value
        mean( unname( sapply( adf_prod, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_mk, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_inve, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_k, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_u, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_emp, `[[`, "p.value" ) ) ),
        
        ## ADF test (logs) p.value s.e.
        sd( unname( sapply( adf_prod, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_mk, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_inve, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_k, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_u, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_emp, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        
        ## S.d. of bpf series
        mean( prod_sd ),
        mean( mk_sd ),
        mean( inve_sd ),
        mean( k_sd ),
        mean( u_sd ),
        mean( emp_sd ),
        
        ## S.e. of bpf series s.d.
        se( prod_sd ) / sqrt( nSize ),
        se( mk_sd ) / sqrt( nSize ),
        se( inve_sd ) / sqrt( nSize ),
        se( k_sd ) / sqrt( nSize ),
        se( u_sd ) / sqrt( nSize ),
        se( emp_sd ) / sqrt( nSize ),
        
        ## relative s.d. (to GDP)
        mean( prod_sd ) / mean(gdp_r_sd ),
        mean( mk_sd ) / mean(gdp_r_sd ),
        mean( inve_sd ) / mean(gdp_r_sd ),
        mean( k_sd ) / mean(gdp_r_sd ),
        mean( u_sd ) / mean(gdp_r_sd ),
        mean( emp_sd ) / mean(gdp_r_sd )
      ),
      ncol = 6, byrow = T)
    
    colnames( key.stats.3 ) <- c( "Productivity",
                                  "Mark-up",
                                  "Inventories",
                                  "Capital Stock",
                                  "Unemployment",
                                  "Employment"
    )
    rownames( key.stats.3 ) <- c( "avg. growth rate", 
                                  " (s.e.)",
                                  "ADF test (logs)",
                                  " (s.e.)", 
                                  " (p-val.)", 
                                  " (s.e.)",
                                  " s.d. (bpf)", 
                                  " (s.e.)",
                                  " relative s.d. (GDP)" )
    
    textplot( formatC( key.stats.3, digits = sDigits, format = "g" ), cmar = 2 )
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
    corr.struct.1 <- matrix(c(colMeans(t( unname( sapply(gdp_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(gdp_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              gdp_gdp_pval,
                              
                              colMeans(t( unname( sapply(cr_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(cr_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              cr_gdp_pval,
                              
                              colMeans(t( unname( sapply(ir_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(ir_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              ir_gdp_pval,
                              
                              colMeans(t( unname( sapply(gov_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(gov_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              gov_gdp_pval,
                              
                              colMeans(t( unname( sapply(imp_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(imp_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              imp_gdp_pval,
                              
                              colMeans(t( unname( sapply(x_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(x_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              x_gdp_pval,
                              
                              colMeans(t( unname( sapply(nx_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(nx_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              nx_gdp_pval
    ),
    
    ncol = 2 * lags + 1, byrow = T)
    colnames( corr.struct.1 ) <- gdp_gdp[[1]]$lag
    rownames( corr.struct.1 ) <- c( "GDP (output)", " (s.e.)", " (p-val.)",
                                    "Consumption", " (s.e.)", " (p-val.)",
                                    "Investment", " (s.e.)", " (p-val.)",
                                    "Gov. Expenditure", " (s.e.)", " (p-val.)",
                                    "Imports", " (s.e.)", " (p-val.)",
                                    "Exports", " (s.e.)", " (p-val.)",
                                    "Net Exports", " (s.e.)", " (p-val.)"
    )
    
    corr.struct.2 <- matrix(c(colMeans(t( unname( sapply(p_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(p_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              p_gdp_pval,
                              
                              colMeans(t( unname( sapply(profits_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(profits_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              profits_gdp_pval,
                              
                              colMeans(t( unname( sapply(wage_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(wage_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              wage_gdp_pval,
                              
                              colMeans(t( unname( sapply(pr_sh_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(pr_sh_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              pr_sh_gdp_pval,
                              
                              colMeans(t( unname( sapply(wg_sh_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(wg_sh_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              wg_sh_gdp_pval,
                              
                              colMeans(t( unname( sapply(debt_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(debt_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              debt_gdp_pval,
                              
                              colMeans(t( unname( sapply(pdebt_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(pdebt_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              pdebt_gdp_pval
                            
    ),
    
    ncol = 2 * lags + 1, byrow = T)
    colnames( corr.struct.2 ) <- gdp_gdp[[1]]$lag
    rownames( corr.struct.2 ) <- c( "Price", " (s.e.)", " (p-val.)",
                                    "Profits", " (s.e.)", " (p-val.)",
                                    "Wages", " (s.e.)", " (p-val.)",
                                    "Profit Share", " (s.e.)", " (p-val.)",
                                    "Wage Share", " (s.e.)", " (p-val.)",
                                    "Private Debt", " (s.e.)", " (p-val.)",
                                    "Public Debt", " (s.e.)", " (p-val.)"
                                  )
    
    title <- paste( "Correlation structure for GDP (", legends[ k ], ")" )
    subTitle <- paste( eval( bquote( paste0( "( non-rate/ratio series are Baxter-King bandpass-filtered, low = ",
                                             .( lowP ), "Q / high = ", .( highP ), "Q / order = ", .( bpfK ),
                                             " / MC runs = ", .( nSize ), " / period = ",
                                             .( warmUpStat + 1 ), " - ", .( nTstat ), " )" ) ) ),
                       eval( bquote ( paste0( "( test H0: lag coefficient is not significant at ",
                                              .( ( 1 - CI ) * 100), "% level", " )" ) ) ), sep ="\n" )
    
    
    corr.struct.3 <- matrix(c(colMeans(t( unname( sapply(prod_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(prod_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              prod_gdp_pval,
                              
                              colMeans(t( unname( sapply(mk_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(mk_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              mk_gdp_pval,
                              
                              colMeans(t( unname( sapply(inve_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(inve_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              inve_gdp_pval,
                              
                              colMeans(t( unname( sapply(k_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(k_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              k_gdp_pval,
                              
                              colMeans(t( unname( sapply(u_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(u_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              u_gdp_pval,
                              
                              colMeans(t( unname( sapply(emp_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(emp_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              emp_gdp_pval
    ),
    
    ncol = 2 * lags + 1, byrow = T)
    colnames( corr.struct.3 ) <- gdp_gdp[[1]]$lag
    rownames( corr.struct.3 ) <- c( "Productivity", " (s.e.)", " (p-val.)",
                                    "Mark-up", " (s.e.)", " (p-val.)",
                                    "Inventories", " (s.e.)", " (p-val.)",
                                    "Capital Stock", " (s.e.)", " (p-val.)",
                                    "Unemployment", " (s.e.)", " (p-val.)",
                                    "Employment", " (s.e.)", " (p-val.)"
    )
    
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
    textplot( formatC( corr.struct.3, digits = sDigits, format = "g" ), cmar = 1 )
    title( main = title, sub = subTitle )
    
    # Write tables to the disk as CSV files for Excel
    write.csv( cbind( key.stats.1, key.stats.2, key.stats.3) , quote = FALSE,
               paste0( folder, "/", outDir, "/", repName, k, "_key_stats.csv" ) )
    write.csv( rbind( corr.struct.1, corr.struct.2, corr.struct.3 ), quote = FALSE,
               paste0( folder, "/", outDir, "/", repName, k, "_corr_struct.csv" ) )
    
    #
    # ---- Aggregated variables stationarity and ergodicity tests ----
    #
    
    # select data to plot
    for( k in 1 : nExp ){
      
      statErgo <- ergod.test.lsd( mcData[[ k ]][ TmaskStat, , ], signif = 1 - CI,
                                  vars = statErgo.vars )
      mcData[[k]]
      # plot table
      textplot( statErgo, cmar = 2, rmar = 0.5 )
      title( main = paste( "Stationarity, i.i.d. and ergodicity tests (", legends[ k ], ")" ),
             sub = paste( "( average p-values for testing H0 and rate of rejection of H0 / MC runs =", nSize, "/ period =", warmUpStat + 1, "-", nTstat, ")\n ( ADF/PP H0: non-stationary, KPSS H0: stationary, BDS H0: i.i.d., KS/AD/WW H0: ergodic )\n( significance =",
                          1 - CI, ")" ) )
      # write to disk
      write.csv( statErgo, quote = FALSE,
                 paste0( folder, "/", outDir, "/", repName, k, "_ergod_tests.csv" ) )
    }
    
  }
  cat( "\nDone...\n" )
  
}, interrupt = function( ex ) {
  cat( "An interrupt was detected.\n" )
  print( ex )
  textplot( "Report incomplete due to interrupt." )
}, error = function( ex ) {
  cat( "An error was detected.\n" )
  print( ex )
  textplot( "Report incomplete due to processing error." )
}, finally = {
  options( warn = 0 )
  cat( "\n", as.character( Sys.time( ) ), "-> Releasing resources...\n\n" )
  totalTime <- proc.time( ) - startTime
  print( totalTime )
  # Close PDF plot file
  dev.off( )
} )


