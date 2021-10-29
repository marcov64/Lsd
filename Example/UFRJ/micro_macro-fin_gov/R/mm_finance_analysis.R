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

folder    <- "./Res_Final_Baseline"                  # data files folder
baseName  <- "Sim_"                     # data files base name (same as .lsd file)
nExp      <- 5                          # number of experiments (sensitivity/different cases)
iniDrop   <- 0                          # initial time steps to drop from analysis (0=none)
nKeep     <- -1                         # number of time steps to keep (-1=all)
cores     <- 0                          # maximum number of cores to allocate (0=all)
savDat    <- F                          # save processed data files and re-use if available?

expVal <- c("Baseline", "Credit Rationing", "HighFragility", "LowBasileia", "EndogenousChanges")

# Aggregated variables to use
logVars <- c( "Real_GDP",               # Real GDP
              "C_r",                    # Real Consumption
              "I_r",                    # Real Investment
              "K_r",                    # Real Stock of Capital
              "P",                      # Price Index
              "PROFITS",                # Real Profits
              "WAGE",                   # Real Wages
              "PROD",                   # Average Labor Productivity
              "MK",                     # Average Markup
              "EMP",                     # Employment in Hours of Labor
              "DEBT_C",
              "DEBT_K",
              "DEBT_I",
              "DEBT_1",
              "DEBT_2",
              "DEBT_3",
              "DEBT_FS",
              "DEBT_FS_ST",
              "DEBT_FS_LT",
              "DEP_C",
              "DEP_K",
              "DEP_I",
              "DEP_1",
              "DEP_2",
              "DEP_3",
              "DEP_FS"
              )

aggrVars <- append( logVars, 
                    c(  "GDP_G",        # Real GDP Growth
                        "P_G",          # Price Growth, Inflation
                        "CON_G",        # Real Consumption Growth
                        "INV_G",        # Real Investment Growth
                        "K_G",          # Real Stock of Capital Growth
                        "PROFITS_G",    # Real Profits Growth
                        "WAGE_G",       # Real Wages Growth
                        "PROD_G",       # Labor Productivity Growth
                        "MK_G",         # Markup Growth
                        "U",            # Unemployment Rate
                        "Profit_Share", # Profit Share 
                        "Wage_Share",   # Wage Share
                        "Avg_Profit_Rate",
                        "Financial_Sector_Demand_Met",
                        "Financial_Sector_Profits",
                        "Avg_Interest_Rate_Long_Term",
                        "DEBT_C_G",
                        "DEBT_K_G",
                        "DEBT_I_G",
                        "DEBT_1_G",
                        "DEBT_2_G",
                        "DEBT_3_G",
                        "DEBT_FS_G",
                        "DEBT_FS_ST_G",
                        "DEBT_FS_LT_G",
                        "DEP_C_G",
                        "DEP_K_G",
                        "DEP_I_G",
                        "DEP_1_G",
                        "DEP_2_G",
                        "DEP_3_G",
                        "DEP_FS_G",
                        "DEBT_RT_C",
                        "DEBT_RT_K",
                        "DEBT_RT_I",
                        "DEBT_RT_1",
                        "DEBT_RT_2",
                        "DEBT_RT_3",
                        "DEBT_RT_FI",
                        "DEBT_RT_CL",
                        "FS_HHI",
                        "FS_LEV",
                        "FS_STR"
                        ) )

# Variables to test for stationarity and ergodicity
    statErgo.vars <- c( "DEBT_FS_G",
                        "DEBT_FS_ST_G",
                        "DEBT_FS_LT_G",
                        "DEP_FS_G",
                        "DEBT_RT_C",
                        "DEBT_RT_K",
                        "DEBT_RT_I",
                        "DEBT_RT_1",
                        "DEBT_RT_2",
                        "DEBT_RT_3",
                        "DEBT_RT_FI",
                        "DEBT_RT_CL",
                        "FS_HHI",
                        "FS_LEV",
                        "FS_STR"
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
  write.csv( A, paste0( folder, "/", baseName, exper, "_finance_avg.csv" ),
             row.names = FALSE, quote = FALSE )
  write.csv( S, paste0( folder, "/", baseName, exper, "_finance_sd.csv" ),
             row.names = FALSE, quote = FALSE )
  write.csv( M, paste0( folder, "/", baseName, exper, "_finance_max.csv" ),
             row.names = FALSE, quote = FALSE )
  write.csv( m, paste0( folder, "/", baseName, exper, "_finance_min.csv" ),
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
warmUpPlot<- 100    # number of "warm-up" runs for plots
nTplot    <- 500     # last period to consider for plots (-1=all)
warmUpStat<- 100    # warm-up runs to evaluate all statistics
nTstat    <- 500     # last period to consider for statistics (-1=all)
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
raster    <- TRUE  # raster or vector plots
res       <- 600    # resolution of raster mode (in dpi)

# Colors assigned to each experiment's lines in graphics
colors <- c( "black", "blue", "red", "orange", "green", "brown", "yellow", "purple", "gray" )
#colors <- rep( "black", 6 )

# Line types assigned to each experiment
lTypes <- c( "solid", "solid", "solid", "solid", "solid", "solid", "solid", "solid", "solid")
#lTypes <- c( "solid", "dashed", "dotted", "dotdash", "longdash", "twodash" )

# Point types assigned to each experiment
pTypes <- c( 4, 4, 4, 4, 4, 4, 4, 4, 4 )
#pTypes <- c( 4, 0, 1, 2, 3, 5 )

# ====== External support functions & definitions ======

library("LSDsensitivity")
source( "support-functions.R" )
source( "time-plots-finance.R" )
source( "box-plots-finance.R" )

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
  outDir <- format( Sys.time(), "Finance_%Y-%m-%d" )
  if( ! dir.exists( paste0( folder, "/", outDir ) ) )
    dir.create( paste0( folder, "/", outDir ) )
  
  cat( paste( "\nSaving results and data to:", paste0( folder, "/", outDir ), "\n" ) )
  
  # Select type of output
  if( raster ){
    # Open PNG (bitmap) files for output
    png( paste0( folder, "/", outDir, "/", repName, "_finance_plots_%d.png" ),
         width = plotW, height = plotH, units = "in", res = res )
    TRUE
  } else {
    # Open PDF plot file for output
    pdf( paste0( folder, "/", outDir, "/", repName, "_finance_plots.pdf" ),
         width = plotW, height = plotH )
    par( mfrow = c ( plotRows, plotCols ) )             # define plots per page
    png( paste0( folder, "/", outDir, "/", repName, "_finance_plots_%d.png" ),
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
  gdp_gr <- 
  infla <- 
  cr_gr <- 
  ir_gr <- 
  profits_gr <- 
  wage_gr <- 
  k_gr <- 
  pr_sh <- 
  wg_sh <- 
  prod_gr <- 
  u_rt  <- 
  mk_gr <- 
  pr_rt <-
  debtc_gr <-
  debtk_gr <- 
  debti_gr <- 
  debt1_gr <- 
  debt2_gr <- 
  debt3_gr <- 
  debtfs_gr <- 
  debtfsst_gr <- 
  depfslt_gr <- 
  depc_gr <- 
  depk_gr <- 
  depi_gr <- 
  dep1_gr <- 
  dep2_gr <- 
  dep3_gr <- 
  depfs_gr <- 
  debtc_rt <- 
  debtk_rt <-
  debti_rt <-
  debt1_rt <-
  debt2_rt <-
  debt3_rt <-
  debt_fi_rt <-
  debt_cl_rt <-
  fs_hhi <- 
  fs_lev <-
  fs_str <-
  vector( mode = "numeric", length = nSize )
  
  gdp_r_sd <-
  con_r_sd <-
  inv_r_sd <-
  p_sd <-
  profits_sd <-
  wage_sd <-
  prod_sd <-
  k_sd <-
  u_sd <-
  pr_sh_sd <-
  wg_sh_sd <-
  mk_sd <-
  debtc_sd <-
  debtk_sd <-
  debti_sd <-
  debt1_sd <-
  debt2_sd <-
  debt3_sd <-
  debtfs_sd <-
  debtfs_st_sd <-
  debtfs_lt_sd <-
  depc_sd <-
  depk_sd <-
  depi_sd <-
  dep1_sd <-
  dep2_sd <-
  dep3_sd <-
  depfs_sd <-
  drtc_sd <-
  drtk_sd <-
  drti_sd <-
  drt1_sd <-
  drt2_sd <-
  drt3_sd <-
  drt_fi_sd <-
  drt_cl_sd <-
  fshhi_sd <-
  fslev_sd <-
  fsstr_sd <-
  vector( mode = 'numeric', length = nSize )
  
  gdp_gdp <-
  debtfs_gdp <-
  debtfs_st_gdp <-
  debtfs_lt_gdp <-
  depfs_gdp <-
  drt_fi_gdp <-
  drt_cl_gdp <-
  fshhi_gdp <-
  fslev_gdp <-
  fsstr_gdp <-
  list( )
  
  gdp_gdp_pval <-
  debtfs_gdp_pval <-
  debtfs_st_gdp_pval <-
  debtfs_lt_gdp_pval <-
  depfs_gdp_pval <-
  drt_fi_gdp_pval <-
  drt_cl_gdp_pval <-
  fshhi_gdp_pval <-
  fslev_gdp_pval <-
  fsstr_gdp_pval <-
  vector( mode = "numeric", length = nExp )
 
  adf_gdp_r <-
  adf_con_r <-
  adf_inv_r <-
  adf_p <-
  adf_profits <-
  adf_wage <-
  adf_prod <-
  adf_k <-
  adf_u <-
  adf_pr_sh <-
  adf_wg_sh <-
  adf_mk <-
  adf_debtc <-
  adf_debtk <-
  adf_debti <-
  adf_debt1 <-
  adf_debt2 <-
  adf_debt3 <-
  adf_debtfs <-
  adf_debtfs_st <-
  adf_debtfs_lt <-
  adf_depc <-
  adf_depk <-
  adf_depi <-
  adf_dep1 <-
  adf_dep2 <-
  adf_dep3 <-
  adf_depfs <-
  adf_drtc <-
  adf_drtk <- 
  adf_drti <-
  adf_drt1 <-
  adf_drt2 <- 
  adf_drt3 <-
  adf_drt_fi <-
  adf_drt_cl <- 
  adf_fshhi <-
  adf_fslev <- 
  adf_fsstr <-
  list( )

  for(k in 1 : nExp){ # Experiment k
    
    #
    # ---- Bandpass filtered GDP, consumption and investment cycles graphic ----
    #
    
    plot_bpf( list( log0( Adata[[ k ]]$Real_GDP ), log0( Adata[[ k ]]$C_r ), 
                    log0( Adata[[ k ]]$I_r ) ),
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
      profits_gr[ j ]   <- mcData[[ k ]][ nTstat, "PROFITS_G", j ]
      wage_gr[ j ]      <- mcData[[ k ]][ nTstat, "WAGE_G", j ]
      k_gr [ j ]        <- mcData[[ k ]][ nTstat, "K_G", j ]
      pr_sh [ j ]       <- mcData[[ k ]][ nTstat, "Profit_Share", j ]
      wg_sh [ j ]       <- mcData[[ k ]][ nTstat, "Wage_Share", j ]
      prod_gr [ j ]     <- mcData[[ k ]][ nTstat, "PROD_G", j ]
      u_rt [ j ]        <- mcData[[ k ]][ nTstat, "U", j ]
      mk_gr [ j ]       <- mcData[[ k ]][ nTstat, "MK_G", j ]
      pr_rt [ j ]      <- mcData[[ k ]][ nTstat, "Avg_Profit_Rate", j ]
      debtc_gr [ j ]      <- mcData[[ k ]][ nTstat, "DEBT_C_G", j ]
      debtk_gr [ j ]      <- mcData[[ k ]][ nTstat, "DEBT_K_G", j ]
      debti_gr [ j ]      <- mcData[[ k ]][ nTstat, "DEBT_I_G", j ]
      debt1_gr [ j ]      <- mcData[[ k ]][ nTstat, "DEBT_1_G", j ]
      debt2_gr [ j ]      <- mcData[[ k ]][ nTstat, "DEBT_2_G", j ]
      debt3_gr [ j ]      <- mcData[[ k ]][ nTstat, "DEBT_3_G", j ]
      debtfs_gr [ j ]      <- mcData[[ k ]][ nTstat, "DEBT_FS_G", j ]
      debtfsst_gr [ j ]      <- mcData[[ k ]][ nTstat, "DEBT_FS_ST_G", j ]
      depfslt_gr [ j ]      <- mcData[[ k ]][ nTstat, "DEBT_FS_LT_G", j ]
      depc_gr [ j ]      <- mcData[[ k ]][ nTstat, "DEP_C_G", j ]
      depk_gr [ j ]      <- mcData[[ k ]][ nTstat, "DEP_K_G", j ]
      depi_gr [ j ]      <- mcData[[ k ]][ nTstat, "DEP_I_G", j ]
      dep1_gr [ j ]      <- mcData[[ k ]][ nTstat, "DEP_1_G", j ]
      dep2_gr [ j ]      <- mcData[[ k ]][ nTstat, "DEP_2_G", j ]
      dep3_gr [ j ]      <- mcData[[ k ]][ nTstat, "DEP_3_G", j ]
      depfs_gr [ j ]      <- mcData[[ k ]][ nTstat, "DEP_FS_G", j ]
      debtc_rt [ j ]      <- mcData[[ k ]][ nTstat, "DEBT_RT_C", j ]
      debtk_rt [ j ]      <- mcData[[ k ]][ nTstat, "DEBT_RT_K", j ]
      debti_rt [ j ]      <- mcData[[ k ]][ nTstat, "DEBT_RT_I", j ]
      debt1_rt [ j ]      <- mcData[[ k ]][ nTstat, "DEBT_RT_1", j ]
      debt2_rt [ j ]      <- mcData[[ k ]][ nTstat, "DEBT_RT_2", j ]
      debt3_rt [ j ]      <- mcData[[ k ]][ nTstat, "DEBT_RT_3", j ]
      debt_fi_rt [ j ]      <- mcData[[ k ]][ nTstat, "DEBT_RT_FI", j ]
      debt_cl_rt [ j ]      <- mcData[[ k ]][ nTstat, "DEBT_RT_CL", j ]
      fs_hhi [ j ]      <- mcData[[ k ]][ nTstat, "FS_HHI", j ]
      fs_lev [ j ]      <- mcData[[ k ]][ nTstat, "FS_LEV", j ]
      fs_str [ j ]      <- mcData[[ k ]][ nTstat, "FS_STR", j ]
      

      # Apply Baxter-King filter to the series
      gdp_bpf     <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "Real_GDP", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      con_bpf     <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "C_r", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      inv_bpf     <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "I_r", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      p_bpf       <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "P", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      profits_bpf <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "PROFITS", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      wage_bpf    <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "WAGE", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      prod_bpf    <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "PROD", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      u_bpf       <- bkfilter( mcData[[ k ]][ TmaskStat, "U", j ] , pl = lowP, pu = highP, nfix = bpfK )
      k_bpf       <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "K_r", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      pr_sh_bpf   <- bkfilter( mcData[[ k ]][ TmaskStat, "Profit_Share", j ] , pl = lowP, pu = highP, nfix = bpfK )
      wg_sh_bpf   <- bkfilter( mcData[[ k ]][ TmaskStat, "Wage_Share", j ] , pl = lowP, pu = highP, nfix = bpfK )
      mk_bpf      <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "MK", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      debtc_bpf     <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "DEBT_C", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      debtk_bpf     <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "DEBT_K", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      debti_bpf     <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "DEBT_I", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      debt1_bpf     <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "DEBT_1", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      debt2_bpf     <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "DEBT_2", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      debt3_bpf     <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "DEBT_3", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      debtfs_bpf     <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "DEBT_FS", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      debtfs_st_bpf     <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "DEBT_FS_ST", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      debtfs_lt_bpf     <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "DEBT_FS_LT", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      depc_bpf     <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "DEP_C", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      depk_bpf     <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "DEP_K", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      depi_bpf     <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "DEP_I", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      dep1_bpf     <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "DEP_1", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      dep2_bpf     <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "DEP_2", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      dep3_bpf     <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "DEP_3", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      depfs_bpf     <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "DEP_FS", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      drtc_bpf       <- bkfilter( mcData[[ k ]][ TmaskStat, "DEBT_RT_C", j ] , pl = lowP, pu = highP, nfix = bpfK )
      drtk_bpf       <- bkfilter( mcData[[ k ]][ TmaskStat, "DEBT_RT_K", j ] , pl = lowP, pu = highP, nfix = bpfK )
      drti_bpf       <- bkfilter( mcData[[ k ]][ TmaskStat, "DEBT_RT_I", j ] , pl = lowP, pu = highP, nfix = bpfK )
      drt1_bpf       <- bkfilter( mcData[[ k ]][ TmaskStat, "DEBT_RT_1", j ] , pl = lowP, pu = highP, nfix = bpfK )
      drt2_bpf       <- bkfilter( mcData[[ k ]][ TmaskStat, "DEBT_RT_2", j ] , pl = lowP, pu = highP, nfix = bpfK )
      drt3_bpf       <- bkfilter( mcData[[ k ]][ TmaskStat, "DEBT_RT_3", j ] , pl = lowP, pu = highP, nfix = bpfK )
      drt_fi_bpf       <- bkfilter( mcData[[ k ]][ TmaskStat, "DEBT_RT_FI", j ] , pl = lowP, pu = highP, nfix = bpfK )
      drt_cl_bpf       <- bkfilter( mcData[[ k ]][ TmaskStat, "DEBT_RT_CL", j ] , pl = lowP, pu = highP, nfix = bpfK )
      fshhi_bpf       <- bkfilter( mcData[[ k ]][ TmaskStat, "FS_HHI", j ] , pl = lowP, pu = highP, nfix = bpfK )
      fslev_bpf       <- bkfilter( mcData[[ k ]][ TmaskStat, "FS_LEV", j ] , pl = lowP, pu = highP, nfix = bpfK )
      fsstr_bpf       <- bkfilter( mcData[[ k ]][ TmaskStat, "FS_STR", j ] , pl = lowP, pu = highP, nfix = bpfK )
      
      # Augmented Dickey-Fuller tests for unit roots
      adf_gdp_r[[ j ]]    <- adf.test( log0( mcData[[ k ]][ TmaskStat, "Real_GDP", j ] ) )
      adf_con_r[[ j ]]    <- adf.test( log0( mcData[[ k ]][ TmaskStat, "C_r", j ] ) )
      adf_inv_r[[ j ]]    <- adf.test( log0( mcData[[ k ]][ TmaskStat, "I_r", j ] ) )
      adf_p[[ j ]]        <- adf.test( log0( mcData[[ k ]][ TmaskStat, "P", j ] ) )
      adf_profits[[ j ]]  <- adf.test( log0( mcData[[ k ]][ TmaskStat, "PROFITS", j ] ) )
      adf_wage[[ j ]]     <- adf.test( log0( mcData[[ k ]][ TmaskStat, "WAGE", j ] ) )
      adf_prod[[ j ]]     <- adf.test( log0( mcData[[ k ]][ TmaskStat, "PROD", j ] ) )
      adf_k[[ j ]]        <- adf.test( log0( mcData[[ k ]][ TmaskStat, "K_r", j ] ) )
      adf_u[[ j ]]        <- adf.test( mcData[[ k ]][ TmaskStat, "U", j ] ) 
      adf_pr_sh[[ j ]]    <- adf.test( mcData[[ k ]][ TmaskStat, "Profit_Share", j ]  )
      adf_wg_sh[[ j ]]    <- adf.test( mcData[[ k ]][ TmaskStat, "Wage_Share", j ]  )
      adf_mk[[ j ]]       <- adf.test( log0( mcData[[ k ]][ TmaskStat, "MK", j ] ) )
      adf_debtc[[ j ]]       <- adf.test( log0( mcData[[ k ]][ TmaskStat, "DEBT_C", j ] ) )
      adf_debtk[[ j ]]       <- adf.test( log0( mcData[[ k ]][ TmaskStat, "DEBT_K", j ] ) )
      adf_debti[[ j ]]       <- adf.test( log0( mcData[[ k ]][ TmaskStat, "DEBT_I", j ] ) )
      adf_debt1[[ j ]]       <- adf.test( log0( mcData[[ k ]][ TmaskStat, "DEBT_1", j ] ) )
      adf_debt2[[ j ]]       <- adf.test( log0( mcData[[ k ]][ TmaskStat, "DEBT_2", j ] ) )
      adf_debt3[[ j ]]       <- adf.test( log0( mcData[[ k ]][ TmaskStat, "DEBT_3", j ] ) )
      adf_debtfs[[ j ]]       <- adf.test( log0( mcData[[ k ]][ TmaskStat, "DEBT_FS", j ] ) )
      adf_debtfs_st[[ j ]]       <- adf.test( log0( mcData[[ k ]][ TmaskStat, "DEBT_FS_ST", j ] ) )
      adf_debtfs_lt[[ j ]]       <- adf.test( log0( mcData[[ k ]][ TmaskStat, "DEBT_FS_LT", j ] ) )
      adf_depc[[ j ]]       <- adf.test( log0( mcData[[ k ]][ TmaskStat, "DEP_C", j ] ) )
      adf_depk[[ j ]]       <- adf.test( log0( mcData[[ k ]][ TmaskStat, "DEP_K", j ] ) )
      adf_depi[[ j ]]       <- adf.test( log0( mcData[[ k ]][ TmaskStat, "DEP_I", j ] ) )
      adf_dep1[[ j ]]       <- adf.test( log0( mcData[[ k ]][ TmaskStat, "DEP_1", j ] ) )
      adf_dep2[[ j ]]       <- adf.test( log0( mcData[[ k ]][ TmaskStat, "DEP_2", j ] ) )
      adf_dep3[[ j ]]       <- adf.test( log0( mcData[[ k ]][ TmaskStat, "DEP_3", j ] ) )
      adf_depfs[[ j ]]       <- adf.test( log0( mcData[[ k ]][ TmaskStat, "DEP_FS", j ] ) )
      adf_drtc[[ j ]]        <- adf.test( mcData[[ k ]][ TmaskStat, "DEBT_RT_C", j ] ) 
      adf_drtk[[ j ]]        <- adf.test( mcData[[ k ]][ TmaskStat, "DEBT_RT_K", j ] ) 
      adf_drti[[ j ]]        <- adf.test( mcData[[ k ]][ TmaskStat, "DEBT_RT_I", j ] ) 
      adf_drt1[[ j ]]        <- adf.test( mcData[[ k ]][ TmaskStat, "DEBT_RT_1", j ] ) 
      adf_drt2[[ j ]]        <- adf.test( mcData[[ k ]][ TmaskStat, "DEBT_RT_2", j ] ) 
      adf_drt3[[ j ]]        <- adf.test( mcData[[ k ]][ TmaskStat, "DEBT_RT_3", j ] ) 
      adf_drt_fi[[ j ]]        <- adf.test( mcData[[ k ]][ TmaskStat, "DEBT_RT_FI", j ] ) 
      adf_drt_cl[[ j ]]        <- adf.test( mcData[[ k ]][ TmaskStat, "DEBT_RT_CL", j ] ) 
      adf_fshhi[[ j ]]        <- adf.test( mcData[[ k ]][ TmaskStat, "FS_HHI", j ] ) 
      adf_fslev[[ j ]]        <- adf.test( mcData[[ k ]][ TmaskStat, "FS_LEV", j ] ) 
      adf_fsstr[[ j ]]        <- adf.test( mcData[[ k ]][ TmaskStat, "FS_STR", j ] ) 
      
      # Standard deviations of filtered series
      gdp_r_sd[ j ]       <- sd( gdp_bpf$cycle[ TmaskBpf, 1 ] )
      con_r_sd[ j ]       <- sd( con_bpf$cycle[ TmaskBpf, 1 ] )
      inv_r_sd[ j ]       <- sd( inv_bpf$cycle[ TmaskBpf, 1 ] )
      p_sd[ j ]           <- sd( p_bpf$cycle[ TmaskBpf, 1 ] )
      profits_sd[ j ]     <- sd( profits_bpf$cycle[ TmaskBpf, 1 ] )
      wage_sd[ j ]        <- sd( wage_bpf$cycle[ TmaskBpf, 1 ] )
      prod_sd[ j ]        <- sd( prod_bpf$cycle[ TmaskBpf, 1 ] )
      k_sd[ j ]           <- sd( k_bpf$cycle[ TmaskBpf, 1 ] )
      u_sd[ j ]           <- sd( u_bpf$cycle[ TmaskBpf, 1 ] )
      pr_sh_sd[ j ]       <- sd( pr_sh_bpf$cycle[ TmaskBpf, 1 ] )
      wg_sh_sd[ j ]       <- sd( wg_sh_bpf$cycle[ TmaskBpf, 1 ] )
      mk_sd[ j ]          <- sd( mk_bpf$cycle[ TmaskBpf, 1 ] )
      debtc_sd[ j ]           <- sd( debtc_bpf$cycle[ TmaskBpf, 1 ] )
      debtk_sd[ j ]           <- sd( debtk_bpf$cycle[ TmaskBpf, 1 ] )
      debti_sd[ j ]           <- sd( debti_bpf$cycle[ TmaskBpf, 1 ] )
      debt1_sd[ j ]           <- sd( debt1_bpf$cycle[ TmaskBpf, 1 ] )
      debt2_sd[ j ]           <- sd( debt2_bpf$cycle[ TmaskBpf, 1 ] )
      debt3_sd[ j ]           <- sd( debt3_bpf$cycle[ TmaskBpf, 1 ] )
      debtfs_sd[ j ]           <- sd( debtfs_bpf$cycle[ TmaskBpf, 1 ] )
      debtfs_st_sd[ j ]           <- sd( debtfs_st_bpf$cycle[ TmaskBpf, 1 ] )
      debtfs_lt_sd[ j ]           <- sd( debtfs_lt_bpf$cycle[ TmaskBpf, 1 ] )
      depc_sd[ j ]           <- sd( depc_bpf$cycle[ TmaskBpf, 1 ] )
      depk_sd[ j ]           <- sd( depk_bpf$cycle[ TmaskBpf, 1 ] )
      depi_sd[ j ]           <- sd( depi_bpf$cycle[ TmaskBpf, 1 ] )
      dep1_sd[ j ]           <- sd( dep1_bpf$cycle[ TmaskBpf, 1 ] )
      dep2_sd[ j ]           <- sd( dep2_bpf$cycle[ TmaskBpf, 1 ] )
      dep3_sd[ j ]           <- sd( dep3_bpf$cycle[ TmaskBpf, 1 ] )
      depfs_sd[ j ]           <- sd( depfs_bpf$cycle[ TmaskBpf, 1 ] )
      drtc_sd[ j ]           <- sd( drtc_bpf$cycle[ TmaskBpf, 1 ] )
      drtk_sd[ j ]           <- sd( drtk_bpf$cycle[ TmaskBpf, 1 ] )
      drti_sd[ j ]           <- sd( drti_bpf$cycle[ TmaskBpf, 1 ] )
      drt1_sd[ j ]           <- sd( drt1_bpf$cycle[ TmaskBpf, 1 ] )
      drt2_sd[ j ]           <- sd( drt2_bpf$cycle[ TmaskBpf, 1 ] )
      drt3_sd[ j ]           <- sd( drt3_bpf$cycle[ TmaskBpf, 1 ] )
      drt_fi_sd[ j ]           <- sd( drt_fi_bpf$cycle[ TmaskBpf, 1 ] )
      drt_cl_sd[ j ]           <- sd( drt_cl_bpf$cycle[ TmaskBpf, 1 ] )
      fshhi_sd[ j ]           <- sd( fshhi_bpf$cycle[ TmaskBpf, 1 ] )
      fslev_sd[ j ]           <- sd( fslev_bpf$cycle[ TmaskBpf, 1 ] )
      fsstr_sd[ j ]           <- sd( fsstr_bpf$cycle[ TmaskBpf, 1 ] )
    
      
      # Build the correlation structures
      gdp_gdp[[ j ]]  <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ],
                              gdp_bpf$cycle[ TmaskBpf, 1 ],
                              lag.max = lags, plot = FALSE, na.action = na.pass )
      debtfs_gdp[[ j ]]  <- ccf( debtfs_bpf$cycle[ TmaskBpf, 1 ],
                              gdp_bpf$cycle[ TmaskBpf, 1 ],
                              lag.max = lags, plot = FALSE, na.action = na.pass )
      debtfs_st_gdp[[ j ]]  <- ccf( debtfs_st_bpf$cycle[ TmaskBpf, 1 ],
                              gdp_bpf$cycle[ TmaskBpf, 1 ],
                              lag.max = lags, plot = FALSE, na.action = na.pass )
      debtfs_lt_gdp[[ j ]]  <- ccf( debtfs_lt_bpf$cycle[ TmaskBpf, 1 ],
                              gdp_bpf$cycle[ TmaskBpf, 1 ],
                              lag.max = lags, plot = FALSE, na.action = na.pass )
      depfs_gdp[[ j ]]  <- ccf( depfs_bpf$cycle[ TmaskBpf, 1 ],
                              gdp_bpf$cycle[ TmaskBpf, 1 ],
                              lag.max = lags, plot = FALSE, na.action = na.pass )
      drt_fi_gdp[[ j ]]  <- ccf( drt_fi_bpf$cycle[ TmaskBpf, 1 ],
                              gdp_bpf$cycle[ TmaskBpf, 1 ],
                              lag.max = lags, plot = FALSE, na.action = na.pass )
      drt_cl_gdp[[ j ]]  <- ccf( drt_cl_bpf$cycle[ TmaskBpf, 1 ],
                              gdp_bpf$cycle[ TmaskBpf, 1 ],
                              lag.max = lags, plot = FALSE, na.action = na.pass )
      fshhi_gdp[[ j ]]  <- ccf( fshhi_bpf$cycle[ TmaskBpf, 1 ],
                              gdp_bpf$cycle[ TmaskBpf, 1 ],
                              lag.max = lags, plot = FALSE, na.action = na.pass )
      fslev_gdp[[ j ]]  <- ccf( fslev_bpf$cycle[ TmaskBpf, 1 ],
                              gdp_bpf$cycle[ TmaskBpf, 1 ],
                              lag.max = lags, plot = FALSE, na.action = na.pass )
      fsstr_gdp[[ j ]]  <- ccf( fsstr_bpf$cycle[ TmaskBpf, 1 ],
                              gdp_bpf$cycle[ TmaskBpf, 1 ],
                              lag.max = lags, plot = FALSE, na.action = na.pass )
      
    }
    
    # Applies t test to the mean lag results to test their significance (H0: lag < critCorr)
    for(i in 1 : (2 * lags + 1) ){ #do for all lags
      if(i != lags + 1)  # don't try to compute autocorrelation at lag 0
      gdp_gdp_pval[ i ] <- t.test0( abs( unname( sapply( gdp_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      debtfs_gdp_pval[ i ]  <- t.test0( abs( unname( sapply( debtfs_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      debtfs_st_gdp_pval[ i ]  <- t.test0( abs( unname( sapply(  debtfs_st_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      debtfs_lt_gdp_pval[ i ] <- t.test0( abs( unname( sapply( debtfs_lt_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      depfs_gdp_pval[ i ] <- t.test0( abs( unname( sapply( depfs_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      drt_fi_gdp_pval[ i ]   <- t.test0( abs( unname( sapply( drt_fi_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      drt_cl_gdp_pval[ i ]  <- t.test0( abs( unname( sapply( drt_cl_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      fshhi_gdp_pval[ i ]   <- t.test0( abs( unname( sapply( fshhi_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      fslev_gdp_pval[ i ]<- t.test0( abs( unname( sapply( fslev_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      fsstr_gdp_pval[ i ]<- t.test0( abs( unname( sapply( fsstr_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      
    }
    
    #
    # ---- Summary statistics table (averages, standard errors and p-values) ----
    #
    
    key.stats.1 <- matrix(
      c(
        ## avg. growth rate
        mean( debtc_gr ), 
        mean( debtk_gr ), 
        mean( debti_gr ), 
        mean( debt1_gr ), 
        mean( debt2_gr ),
        mean( debt3_gr ), 
        mean( debtfs_gr ), 
        
        ## (s.e.)
        sd( debtc_gr ) / sqrt( nSize ), 
        sd( debtk_gr ) / sqrt( nSize ),
        sd( debti_gr ) / sqrt( nSize ), 
        sd( debt1_gr ) / sqrt( nSize ),
        sd( debt2_gr ) / sqrt( nSize ),
        sd( debt3_gr ) / sqrt( nSize ),
        sd( debtfs_gr ) / sqrt( nSize ),
      
        ## ADF test (logs)
        mean( unname( sapply( adf_debtc, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_debtk, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_debti, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_debt1, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_debt2, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_debt3, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_debtfs, `[[`, "statistic" ) ) ),
        
        ## ADF test (logs) s.e.
        sd( unname( sapply( adf_debtc, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_debtk, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_debti, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_debt1, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_debt2, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_debt3, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_debtfs, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        
        ## ADF test (logs) p.value
        mean( unname( sapply( adf_debtc, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_debtk, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_debti, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_debt1, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_debt2, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_debt3, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_debtfs, `[[`, "p.value" ) ) ),
        
        ## ADF test (logs) p.value s.e.
        sd( unname( sapply( adf_debtc, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_debtk, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_debti, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_debt1, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_debt2, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_debt3, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_debtfs, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        
        ## S.d. of bpf series
        mean( debtc_sd ), 
        mean( debtk_sd ), 
        mean( debti_sd ),
        mean( debt1_sd ), 
        mean( debt2_sd ), 
        mean( debt3_sd ), 
        mean( debtfs_sd ), 
        
        ## S.e. of bpf series s.d.
        se( debtc_sd ) / sqrt( nSize ), 
        se( debtk_sd ) / sqrt( nSize ), 
        se( debti_sd ) / sqrt( nSize ), 
        se( debt1_sd ) / sqrt( nSize ), 
        se( debt2_sd ) / sqrt( nSize ), 
        se( debt3_sd ) / sqrt( nSize ), 
        se( debtfs_sd ) / sqrt( nSize ), 
        
        ## relative s.d. (to GDP)
        mean( debtc_sd ) / mean( gdp_r_sd ), 
        mean( debtk_sd ) / mean( gdp_r_sd ), 
        mean( debti_sd ) / mean( gdp_r_sd ),
        mean( debt1_sd ) / mean( gdp_r_sd ), 
        mean( debt2_sd ) / mean( gdp_r_sd ),
        mean( debt3_sd ) / mean( gdp_r_sd ),
        mean( debtfs_sd ) / mean( gdp_r_sd )
        
      ),
      ncol = 7, byrow = T)
    
    colnames( key.stats.1 ) <- c( "C. Sector Debt", 
                                  "K. Sector Debt", 
                                  "I. Sector Debt", 
                                  "Class A Debt.",
                                  "Class B Debt",
                                  "Class c Debt",
                                  "Financial Sector Total Debt"
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
        mean( depc_gr ), 
        mean( depk_gr ), 
        mean( depi_gr ),
        mean( dep1_gr ), 
        mean( dep2_gr ), 
        mean( dep3_gr ), 
        mean( depfs_gr ), 
        
        ## (s.e.)
        sd( depc_gr ) / sqrt( nSize ), 
        sd( depk_gr ) / sqrt( nSize ), 
        sd( depi_gr ) / sqrt( nSize ),
        sd( dep1_gr ) / sqrt( nSize ),
        sd( dep2_gr ) / sqrt( nSize ),
        sd( dep3_gr ) / sqrt( nSize ), 
        sd( depfs_gr ) / sqrt( nSize ),
        
        ## ADF test (logs)
        mean( unname( sapply( adf_depc, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_depk, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_depi, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_dep1, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_dep2, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_dep3, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_depfs, `[[`, "statistic" ) ) ),
        
        ## ADF test (logs) s.e.
        sd( unname( sapply( adf_depc, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_depk, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_depi, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_dep1, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_dep2, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_dep3, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_depfs, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        
        ## ADF test (logs) p.value
        mean( unname( sapply( adf_depc, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_depk, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_depi, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_dep1, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_dep2, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_dep3, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_depfs, `[[`, "p.value" ) ) ),
        
        ## ADF test (logs) p.value s.e.
        sd( unname( sapply( adf_depc, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_depk, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_depi, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_dep1, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_dep2, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_dep3, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_depfs, `[[`, "p.value" ) ) ) / sqrt( nSize ),
      
        ## S.d. of bpf series
        mean( depc_sd ),
        mean( depk_sd ), 
        mean( depi_sd ),
        mean( dep1_sd ),
        mean( dep2_sd ),
        mean( dep3_sd ),
        mean( depfs_sd ), 
        
        ## S.e. of bpf series s.d.
        se( depc_sd ) / sqrt( nSize ),
        se( depk_sd ) / sqrt( nSize ), 
        se( depi_sd ) / sqrt( nSize ),
        se( dep1_sd ) / sqrt( nSize ),
        se( dep2_sd ) / sqrt( nSize ),
        se( dep3_sd ) / sqrt( nSize ), 
        se( depfs_sd ) / sqrt( nSize ), 
        
        ## relative s.d. (to GDP)
        mean( depc_sd ) / mean( gdp_r_sd ), 
        mean( depk_sd ) / mean( gdp_r_sd ),
        mean( depi_sd ) / mean( gdp_r_sd ), 
        mean( dep1_sd ) / mean( gdp_r_sd ),
        mean( dep2_sd ) / mean( gdp_r_sd ),
        mean( dep3_sd ) / mean(gdp_r_sd ),
        mean( depfs_sd ) / mean( gdp_r_sd )
        
      ),
      ncol = 7, byrow = T)
    
    colnames( key.stats.2 ) <- c( "C. Sector Deposits", 
                                  "K. Sector Deposits", 
                                  "I. Sector Deposits",
                                  "Class A Deposits",
                                  "Class B Deposits",
                                  "Class C Deposits", 
                                  "Financial Sector Total Deposits"
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
        mean( debtc_rt ),
        mean( debtk_rt ),
        mean( debti_rt ), 
        mean( debt1_rt ),
        mean( debt2_rt ), 
        mean( debt3_rt ),
        mean( debt_fi_rt ),
        mean( debt_cl_rt ),
      
        ## (s.e.)
        sd( debtc_rt ) / sqrt( nSize ),
        sd( debtk_rt ) / sqrt( nSize ),
        sd( debti_rt ) / sqrt( nSize ),
        sd( debt1_rt ) / sqrt( nSize ),
        sd( debt2_rt ) / sqrt( nSize ),
        sd( debt3_rt ) / sqrt( nSize ),
        sd( debt_fi_rt ) / sqrt( nSize ),
        sd( debt_cl_rt ) / sqrt( nSize ),
        
        ## ADF test (logs)
        mean( unname( sapply( adf_drtc, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_drtk, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_drti, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_drt1, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_drt2, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_drt3, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_drt_fi, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_drt_cl, `[[`, "statistic" ) ) ),
        
        ## ADF test (logs) s.e.
        sd( unname( sapply( adf_drtc, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_drtk, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_drti, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_drt1, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_drt2, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_drt3, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_drt_fi, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_drt_cl, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        
        ## ADF test (logs) p.value
        mean( unname( sapply( adf_drtc, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_drtk, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_drti, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_drt1, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_drt2, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_drt3, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_drt_fi, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_drt_cl, `[[`, "p.value" ) ) ),
        
        ## ADF test (logs) p.value s.e.
        sd( unname( sapply( adf_drtc, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_drtk, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_drti, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_drt1, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_drt2, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_drt3, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_drt_fi, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_drt_cl, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        
        ## S.d. of bpf series
        mean( drtc_sd ),
        mean( drtk_sd ),
        mean( drti_sd ),
        mean( drt1_sd ),
        mean( drt2_sd ),
        mean( drt3_sd ),
        mean( drt_fi_sd ),
        mean( drt_cl_sd ),
        
        ## S.e. of bpf series s.d.
        se( drtc_sd ) / sqrt( nSize ),
        se( drtk_sd ) / sqrt( nSize ),
        se( drti_sd ) / sqrt( nSize ),
        se( drt1_sd ) / sqrt( nSize ),
        se( drt2_sd ) / sqrt( nSize ),
        se( drt3_sd ) / sqrt( nSize ),
        se( drt_fi_sd ) / sqrt( nSize ),
        se( drt_cl_sd ) / sqrt( nSize ),
        
        ## relative s.d. (to GDP)
        mean( drtc_sd ) / mean(gdp_r_sd ),
        mean( drtk_sd ) / mean(gdp_r_sd ),
        mean( drti_sd ) / mean(gdp_r_sd ),
        mean( drt1_sd ) / mean(gdp_r_sd ),
        mean( drt2_sd ) / mean(gdp_r_sd ),
        mean( drt3_sd ) / mean(gdp_r_sd ),
        mean( drt_fi_sd ) / mean(gdp_r_sd ),
        mean( drt_cl_sd ) / mean(gdp_r_sd )
      ),
      ncol = 8, byrow = T)
    
    colnames( key.stats.3 ) <- c( "C. Sector Debt Rate",
                                  "K. Sector Debt Rate",
                                  "I. Sector Debt Rate",
                                  "Class A Debt Rate",
                                  "Class B Debt Rate",
                                  "Class C Debt Rate",
                                  "Avg. Firms Debt Rate",
                                  "Avg. Class Debt Rate"
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

    corr.struct.1 <- matrix(c(colMeans(t( unname( sapply(debtfs_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(debtfs_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              debtfs_gdp_pval,
                              
                              colMeans(t( unname( sapply(debtfs_st_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(debtfs_st_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              debtfs_st_gdp_pval,
                              
                              colMeans(t( unname( sapply(debtfs_lt_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(debtfs_lt_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              debtfs_lt_gdp_pval,
                              
                              colMeans(t( unname( sapply(depfs_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(depfs_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              depfs_gdp_pval,
                              
                              colMeans(t( unname( sapply(drt_fi_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(drt_fi_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              drt_fi_gdp_pval,
                              
                              colMeans(t( unname( sapply(drt_cl_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(drt_cl_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              drt_cl_gdp_pval,
                              
                              colMeans(t( unname( sapply(fslev_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(fslev_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              fslev_gdp_pval
    ),
    
    ncol = 2 * lags + 1, byrow = T)
    colnames( corr.struct.1 ) <- gdp_gdp[[1]]$lag
    rownames( corr.struct.1 ) <- c( "Total Debt", " (s.e.)", " (p-val.)",
                                    "Short Term Loans", " (s.e.)", " (p-val.)",
                                    "Long Term Loans", " (s.e.)", " (p-val.)",
                                    "Total Deposits", " (s.e.)", " (p-val.)",
                                    "Firms Debt Rate", " (s.e.)", " (p-val.)",
                                    "Class Debt Rate", " (s.e.)", " (p-val.)",
                                    "Financial Sector Leverage", " (s.e.)", " (p-val.)"
    )
    
    
    textplot( formatC( corr.struct.1, digits = sDigits, format = "g" ), cmar = 1 )
    title( main = title, sub = subTitle )
    
    # Write tables to the disk as CSV files for Excel
    write.csv( cbind( key.stats.1, key.stats.2, key.stats.3) , quote = FALSE,
               paste0( folder, "/", outDir, "/", repName, k, "_key_stats.csv" ) )
    write.csv( rbind( corr.struct.1 ), quote = FALSE,
               paste0( folder, "/", outDir, "/", repName, k, "_corr_struct.csv" ) )
    
    #
    # ---- Aggregated variables stationarity and ergodicity tests ----
    #
    
    # select data to plot
    
    
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


