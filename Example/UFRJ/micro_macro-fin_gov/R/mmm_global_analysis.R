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

folder    <- "./Res_teste_depreciation"                  # data files folder
baseName  <- "Sim_"                     # data files base name (same as .lsd file)
nExp      <- 2                         # number of experiments (sensitivity/different cases)
iniDrop   <- 0                          # initial time steps to drop from analysis (0=none)
nKeep     <- -1                         # number of time steps to keep (-1=all)
cores     <- 0                          # maximum number of cores to allocate (0=all)
savDat    <- F                          # save processed data files and re-use if available?

expVal <- c("No", "Yes")                   # case parameter values

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
              "PROD",                   # Average Labor Productivity
              "MK",                     # Average Markup
              "EMP",                    # Employment in Hours of Labor
              "KL",                     # Capital Labor Ratio
              "DEBT_FS",
              "DEBT_FS_ST",
              "DEBT_FS_LT",
              "DEP_FS",
              "FS_PR"
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
                        "PROD_G",       # Productivity Growth
                        "PROFITS_G",    # Real Profits Growth
                        "WAGE_G",       # Real Wages Growth
                        "MK_G",         # Markup Growth
                        "EMP_G",        # Employment Growth
                        "U",            # Unemployment Rate
                        "Profit_Share", # Profit Share 
                        "Wage_Share",   # Wage Share
                        "PCU",          # Productive Capacity Utilization
                        "PR",           # Profit Rate
                        "CGDP",         # Consumption Share of GDP
                        "IGDP",         # Investment Share of GDP
                        "GGDP",         # Government Expenses Share of GDP
                        "NXGDP",        # Net Exports Share of GDP
                        "INVGDP",       # Inventories Share of GDP
                        "KGDP",          # Capital Stock Share of GDP (K/Y Ratio)
                        "Financial_Sector_Demand_Met",
                        "Financial_Sector_Rescue",
                        "DEBT_FS_G",
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
                        "FS_STR",
                        "FS_DR",
                        "FS_DEF",
                        "PONZI",
                        "SPEC",
                        "HEDGE",
                        "IR",
                        "IR_DEP",
                        "IR_ST",
                        "IR_LT",
                        "Cri",
                        "BKR",
                        "BKR_RT"
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
                        "MK_G",         # Markup Growth
                        "EMP_G",        # Employment Growth
                        "U",            # Unemployment Rate
                        "Profit_Share", # Profit Share 
                        "Wage_Share",   # Wage Share
                        "PCU",          # Productive Capacity Utilization
                        "PR",           # Profit Rate
                        "CGDP",         # Consumption Share of GDP
                        "IGDP",         # Investment Share of GDP
                        "GGDP",         # Government Expenses Share of GDP
                        "NXGDP",        # Net Exports Share of GDP
                        "INVGDP",       # Inventories Share of GDP
                        "KGDP",         # Capital Stock Share of GDP (K/Y Ratio)
                        "DEBT_FS_G",
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
warmUpPlot<- 100    # number of "warm-up" runs for plots
nTplot    <- 500     # last period to consider for plots (-1=all)
warmUpStat<- 300    # warm-up runs to evaluate all statistics
nTstat    <- 500     # last period to consider for statistics (-1=all)
bPlotCoef <- 1.5    # boxplot whiskers extension from the box (0=extremes)
bPlotNotc <- FALSE  # use boxplot notches
lowP      <- 6      # bandpass filter minimum period
highP     <- 32     # bandpass filter maximum period
bpfK      <- 12     # bandpass filter order
lags      <- 6      # lags to analyze
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
colors <- c( "black", "blue", "red", "orange", "green", "brown", "yellow", "purple", "gray", "blueviolet" )
#colors <- rep( "black", 6 )

# Line types assigned to each experiment
lTypes <- c( "solid", "solid", "solid", "solid", "solid", "solid", "solid", "solid", "solid", "solid")
#lTypes <- c( "solid", "dashed", "dotted", "dotdash", "longdash", "twodash" )

# Point types assigned to each experiment
pTypes <- c( 4, 4, 4, 4, 4, 4, 4, 4, 4, 4 )
#pTypes <- c( 4, 0, 1, 2, 3, 5 )

# ====== External support functions & definitions ======

library("LSDsensitivity")
source( "support-functions.R" )
source( "time-plots-global.R" )
source( "box-plots-global.R" )

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
  
  if(nExp>1)
  {
  box_plots( mcData, nExp, nSize, TmaxStat, TmaskStat, warmUpStat,
                         nTstat, legends, legendList, sDigits, bPlotCoef,
                         bPlotNotc, folder, outDir, repName )
  }
  
  #
  # ====== STATISTICS GENERATION ======
  #
  
  # Create vectors and lists to hold the Monte Carlo results
  gdp_gr<-
  infla<-
  cr_gr<-
  ir_gr<-
  gr_gr<-
  mr_gr<-
  x_gr<-
  nx_gr<-
  inve_gr<-
  k_gr<-
  prod_gr<-
  profits_gr<-
  wage_gr<-
  mk_gr<-
  emp_gr<-
  kl_rt<-
  pr_sh<-
  wg_sh<-  
  pr_rt<-
  u_rt<-
  pcu_rt<-
  cgdp_rt<-
  igdp_rt<-
  ggdp_rt<-
  nxgdp_rt<-
  invgdp_rt<-
  kgdp_rt<-
  debtfs_gr<-
  debtfsst_gr<-
  debtfslt_gr<-
  depfslt_gr<-
  depfs_gr<-
  debtc_rt<-
  debtk_rt<-
  debti_rt<-
  debt1_rt<-
  debt2_rt<-
  debt3_rt<-
  debt_fi_rt<-
  debt_cl_rt<-
  fs_hhi<-
  fs_lev<-
  fs_str<-
  fs_dr<-
  ponzi<-
  spec<-
  hedge<-
  vector( mode = "numeric", length = nSize )
  
  gdp_r_sd<-
  con_r_sd<-
  inv_r_sd<-
  gov_r_sd<-
  imp_r_sd<-
  x_r_sd<-
  nx_r_sd<-
  p_sd<-
  k_sd<-
  inve_sd<-
  emp_sd<-
  profits_sd<-
  wage_sd <-
  pr_sh_sd<-
  wg_sh_sd<-
  prod_sd<-
  mk_sd<-
  kl_sd<-
  infla_sd<-
  u_sd<-
  pcu_sd<-
  pr_sd<-
  cgdp_sd<-
  igdp_sd<-
  ggdp_sd<-
  nxgdp_sd<-
  invgdp_sd<-
  kgdp_sd<-
  debtfs_sd<-
  debtfs_st_sd<-
  debtfs_lt_sd<-
  depfs_sd<-
  drtc_sd<-
  drtk_sd<-
  drti_sd<-
  drt1_sd<-
  drt2_sd<-
  drt3_sd<-
  drt_fi_sd<-
  drt_cl_sd<-
  fshhi_sd<-
  fslev_sd<-
  fsstr_sd<-
  fsdr_sd<-
  fspr_sd<-
  fsdef_sd<-
  ponzi_sd<-
  spec_sd<-
  hedge_sd<-
  vector( mode = 'numeric', length = nSize )
  
  gdp_gdp<-
  cr_gdp<-
  ir_gdp<-
  gov_gdp<-
  imp_gdp<-
  x_gdp<-
  nx_gdp<-
  p_gdp<-
  k_gdp<-
  inve_gdp<-
  emp_gdp<-
  profits_gdp<-
  wage_gdp<-
  prod_gdp<-
  mk_gdp<-
  kl_gdp<-
  infla_gdp<-
  u_gdp<-
  pr_sh_gdp<-
  wg_sh_gdp<-
  pcu_gdp<-
  pr_gdp<-
  cgdp_gdp<-
  igdp_gdp<-
  ggdp_gdp<-
  nxgdp_gdp<-
  invgdp_gdp<-
  kgdp_gdp<-
  debtfs_gdp<-
  debtfs_st_gdp<-
  debtfs_lt_gdp<-
  depfs_gdp<-
  drt_fi_gdp<-
  drt_cl_gdp<-
  fshhi_gdp<-
  fslev_gdp<-
  fsstr_gdp<-
  fspr_gdp<-
  fsdr_gdp<-
  fsdef_gdp<-
  ponzi_gdp<-
  spec_gdp<-
  hedge_gdp<-
  rescue_gdp<-
    drt_fi_debt<-
    drt_cl_debt<-
    fshhi_debt<-
    fslev_debt<-
    fsstr_debt<-
    fspr_debt<-
    fsdr_debt<-
    fsdef_debt<-
    ponzi_debt<-
    spec_debt<-
    hedge_debt<-
    rescue_debt<-
    drt_fi_inv<-
    drt_cl_inv<-
    fshhi_inv<-
    fslev_inv<-
    fsstr_inv<-
    fspr_inv<-
    fsdr_inv<-
    fsdef_inv<-
    ponzi_inv<-
    spec_inv<-
    hedge_inv<-
    rescue_inv<-
  list( )
  
  gdp_gdp_pval<- 
  cr_gdp_pval<- 
  ir_gdp_pval<- 
  gov_gdp_pval<- 
  imp_gdp_pval<- 
  x_gdp_pval<- 
  nx_gdp_pval<- 
  p_gdp_pval<- 
  k_gdp_pval<- 
  inve_gdp_pval<- 
  emp_gdp_pval<- 
  wage_gdp_pval<- 
  profits_gdp_pval<- 
  wage_gdp_pval<- 
  prod_gdp_pval<- 
  mk_gdp_pval<- 
  kl_gdp_pval<- 
  infla_gdp_pval<- 
  u_gdp_pval<- 
  pr_sh_gdp_pval<- 
  wg_sh_gdp_pval<- 
  pcu_gdp_pval<- 
  pr_gdp_pval<- 
  cgdp_gdp_pval<- 
  igdp_gdp_pval<- 
  ggdp_gdp_pval<- 
  nxgdp_gdp_pval<- 
  invgdp_gdp_pval<- 
  kgdp_gdp_pval<- 
  debtfs_gdp_pval<-
  debtfs_st_gdp_pval<-
  debtfs_lt_gdp_pval<-
  depfs_gdp_pval<-
  drt_fi_gdp_pval<-
  drt_cl_gdp_pval<-
  fshhi_gdp_pval<-
  fslev_gdp_pval<-
  fsstr_gdp_pval<-
  fspr_gdp_pval<-
  fsdr_gdp_pval<-
  ponzi_gdp_pval<-
  spec_gdp_pval<-
  hedge_gdp_pval<-
  vector( mode = "numeric", length = nExp )
  
  adf_gdp_r<-
  adf_con_r<-
  adf_inv_r<-
  adf_gov_r<-
  adf_imp_r<-
  adf_x<-
  adf_nx<-
  adf_p<-
  adf_k<-
  adf_inve<-
  adf_emp<-
  adf_profits<-
  adf_wage<-
  adf_prod<-
  adf_mk<-
  adf_kl<-
  adf_u<-
  adf_pr_sh<-
  adf_wg_sh<-
  adf_infla<-
  adf_pcu<-
  adf_pr<-
  adf_cgdp<-
  adf_igdp<-
  adf_ggdp<-
  adf_nxgdp<-
  adf_invgdp<-
  adf_kgdp<- 
  adf_drtc<-
  adf_drtk<- 
  adf_drti<-
  adf_drt1<-
  adf_drt2<-
  adf_drt3<-
  adf_drt_fi<-
  adf_drt_cl<-
  adf_fshhi<-
  adf_fslev<-
  adf_fsstr<-
  adf_fsdr<-
  adf_ponzi<-
  adf_spec<-
  adf_hedge<-
  adf_dst_g<-
  adf_dlt_g<-
  adf_d_g<-
  adf_dep_g<-
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
              leg = c( "GDP", "Consumption", "Investment" ),
              xlab = "Time", ylab = "Filtered series",
              tit = paste( "GDP cycles (", legends[ k ], ")" ),
              subtit = paste( "( Baxter-King bandpass-filtered, low = 6Q / high = 32Q / order = 12 / MC runs =",
                              nSize, ")" ) )
    
    #
    # ==== Statistics computation for tables ====
    #
    
    for( j in 1 : nSize ){  # Execute for every Monte Carlo run
      
      # Monte carlo average growth rates and shares
      gdp_gr[ j ]<- mcData[[ k ]][ nTstat, "GDP_G", j ]           
      infla [ j ]<- mcData[[ k ]][ nTstat, "P_G", j ]      
      cr_gr[ j ]<- mcData[[ k ]][ nTstat, "CON_G", j ]    
      ir_gr[ j ]<- mcData[[ k ]][ nTstat, "INV_G", j ]   
      gr_gr[ j ]<- mcData[[ k ]][ nTstat, "GOV_G", j ]   
      mr_gr[ j ]<- mcData[[ k ]][ nTstat, "M_G", j ]     
      x_gr [ j ]<- mcData[[ k ]][ nTstat, "X_G", j ]     
      nx_gr [ j ]<- mcData[[ k ]][ nTstat, "NX_G", j ]        
      inve_gr [ j ]<- mcData[[ k ]][ nTstat, "INVE_G", j ]     
      k_gr [ j ]<- mcData[[ k ]][ nTstat, "K_G", j ]        
      prod_gr [ j ]<- mcData[[ k ]][ nTstat, "PROD_G", j ]      
      profits_gr[ j ]<- mcData[[ k ]][ nTstat, "PROFITS_G", j ]     
      wage_gr[ j ]<- mcData[[ k ]][ nTstat, "WAGE_G", j ]    
      mk_gr [ j ]<- mcData[[ k ]][ nTstat, "MK_G", j ]       
      emp_gr [ j ]<- mcData[[ k ]][ nTstat, "EMP_G", j ]     
      kl_rt [ j ]<- mcData[[ k ]][ nTstat, "KL", j ]         
      pr_sh [ j ]<- mcData[[ k ]][ nTstat, "Profit_Share", j ]  
      wg_sh [ j ]<- mcData[[ k ]][ nTstat, "Wage_Share", j ]   
      pr_rt [ j ]<- mcData[[ k ]][ nTstat, "PR", j ]             
      u_rt [ j ]<- mcData[[ k ]][ nTstat, "U", j ]               
      pcu_rt [ j ]<- mcData[[ k ]][ nTstat, "PCU", j ]             
      cgdp_rt [ j ]<- mcData[[ k ]][ nTstat, "CGDP", j ]           
      igdp_rt [ j ]<- mcData[[ k ]][ nTstat, "IGDP", j ]           
      ggdp_rt [ j ]<- mcData[[ k ]][ nTstat, "GGDP", j ]           
      nxgdp_rt [ j ]<- mcData[[ k ]][ nTstat, "NXGDP", j ]        
      invgdp_rt [ j ]<- mcData[[ k ]][ nTstat, "INVGDP", j ]       
      kgdp_rt [ j ]<- mcData[[ k ]][ nTstat, "KGDP", j ]         
      debtfs_gr [ j ]<- mcData[[ k ]][ nTstat, "DEBT_FS_G", j ]
      debtfsst_gr [ j ]<- mcData[[ k ]][ nTstat, "DEBT_FS_ST_G", j ]
      debtfslt_gr [ j ]<- mcData[[ k ]][ nTstat, "DEBT_FS_LT_G", j ]
      depfs_gr [ j ]<- mcData[[ k ]][ nTstat, "DEP_FS_G", j ]
      debtc_rt [ j ]<- mcData[[ k ]][ nTstat, "DEBT_RT_C", j ]
      debtk_rt [ j ]<- mcData[[ k ]][ nTstat, "DEBT_RT_K", j ]
      debti_rt [ j ]<- mcData[[ k ]][ nTstat, "DEBT_RT_I", j ]
      debt1_rt [ j ]<- mcData[[ k ]][ nTstat, "DEBT_RT_1", j ]
      debt2_rt [ j ]<- mcData[[ k ]][ nTstat, "DEBT_RT_2", j ]
      debt3_rt [ j ]<- mcData[[ k ]][ nTstat, "DEBT_RT_3", j ]
      debt_fi_rt [ j ]<- mcData[[ k ]][ nTstat, "DEBT_RT_FI", j ]
      debt_cl_rt [ j ]<- mcData[[ k ]][ nTstat, "DEBT_RT_CL", j ]
      fs_hhi [ j ]<- mcData[[ k ]][ nTstat, "FS_HHI", j ]
      fs_lev [ j ]<- mcData[[ k ]][ nTstat, "FS_LEV", j ]
      fs_str [ j ]<- mcData[[ k ]][ nTstat, "FS_STR", j ]
      fs_dr [ j ]<- mcData[[ k ]][ nTstat, "FS_DR", j ]
      ponzi [ j ]<- mcData[[ k ]][ nTstat, "PONZI", j ]
      spec [ j ]<- mcData[[ k ]][ nTstat, "SPEC", j ]
      hedge [ j ]<- mcData[[ k ]][ nTstat, "HEDGE", j ]
      
      # Apply Baxter-King filter to the series
      gdp_bpf<- bkfilter( log0( mcData[[ k ]][ TmaskStat, "Real_GDP", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      con_bpf<- bkfilter( log0( mcData[[ k ]][ TmaskStat, "C_r", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      inv_bpf<- bkfilter( log0( mcData[[ k ]][ TmaskStat, "I_r", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      gov_bpf<- bkfilter( log0( mcData[[ k ]][ TmaskStat, "G_r", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      imp_bpf<- bkfilter( log0( mcData[[ k ]][ TmaskStat, "M_r", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      x_bpf<- bkfilter( log0( mcData[[ k ]][ TmaskStat, "X_r", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      nx_bpf<- bkfilter( log0( mcData[[ k ]][ TmaskStat, "NX_r", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      p_bpf<- bkfilter( log0( mcData[[ k ]][ TmaskStat, "P", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      k_bpf<- bkfilter( log0( mcData[[ k ]][ TmaskStat, "K_r", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      inve_bpf<- bkfilter( log0( mcData[[ k ]][ TmaskStat, "INVE_r", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      emp_bpf<- bkfilter( log0( mcData[[ k ]][ TmaskStat, "EMP", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      profits_bpf<- bkfilter( log0( mcData[[ k ]][ TmaskStat, "PROFITS", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      wage_bpf<- bkfilter( log0( mcData[[ k ]][ TmaskStat, "WAGE", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      prod_bpf<- bkfilter( log0( mcData[[ k ]][ TmaskStat, "PROD", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      mk_bpf<- bkfilter( log0( mcData[[ k ]][ TmaskStat, "MK", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      kl_bpf<- bkfilter( log0( mcData[[ k ]][ TmaskStat, "KL", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      infla_bpf<- bkfilter( mcData[[ k ]][ TmaskStat, "P_G", j ] , pl = lowP, pu = highP, nfix = bpfK )
      pr_sh_bpf<- bkfilter( mcData[[ k ]][ TmaskStat, "Profit_Share", j ] , pl = lowP, pu = highP, nfix = bpfK )
      wg_sh_bpf<- bkfilter( mcData[[ k ]][ TmaskStat, "Wage_Share", j ] , pl = lowP, pu = highP, nfix = bpfK )
      u_bpf<- bkfilter( mcData[[ k ]][ TmaskStat, "U", j ] , pl = lowP, pu = highP, nfix = bpfK )
      pcu_bpf<- bkfilter( mcData[[ k ]][ TmaskStat, "PCU", j ] , pl = lowP, pu = highP, nfix = bpfK )
      pr_bpf<- bkfilter( mcData[[ k ]][ TmaskStat, "PR", j ] , pl = lowP, pu = highP, nfix = bpfK )
      cgdp_bpf<- bkfilter( mcData[[ k ]][ TmaskStat, "CGDP", j ] , pl = lowP, pu = highP, nfix = bpfK )
      igdp_bpf<- bkfilter( mcData[[ k ]][ TmaskStat, "IGDP", j ] , pl = lowP, pu = highP, nfix = bpfK )
      ggdp_bpf<- bkfilter( mcData[[ k ]][ TmaskStat, "GGDP", j ] , pl = lowP, pu = highP, nfix = bpfK )
      nxgdp_bpf<- bkfilter( mcData[[ k ]][ TmaskStat, "NXGDP", j ] , pl = lowP, pu = highP, nfix = bpfK )
      invgdp_bpf<- bkfilter( mcData[[ k ]][ TmaskStat, "INVGDP", j ] , pl = lowP, pu = highP, nfix = bpfK )
      kgdp_bpf<- bkfilter( mcData[[ k ]][ TmaskStat, "KGDP", j ] , pl = lowP, pu = highP, nfix = bpfK )
      debtfs_bpf<- bkfilter( log0( mcData[[ k ]][ TmaskStat, "DEBT_FS", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      debtfs_st_bpf<- bkfilter( log0( mcData[[ k ]][ TmaskStat, "DEBT_FS_ST", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      debtfs_lt_bpf<- bkfilter( log0( mcData[[ k ]][ TmaskStat, "DEBT_FS_LT", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      depfs_bpf<- bkfilter( log0( mcData[[ k ]][ TmaskStat, "DEP_FS", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      drtc_bpf<- bkfilter( mcData[[ k ]][ TmaskStat, "DEBT_RT_C", j ] , pl = lowP, pu = highP, nfix = bpfK )
      drtk_bpf<- bkfilter( mcData[[ k ]][ TmaskStat, "DEBT_RT_K", j ] , pl = lowP, pu = highP, nfix = bpfK )
      drti_bpf<- bkfilter( mcData[[ k ]][ TmaskStat, "DEBT_RT_I", j ] , pl = lowP, pu = highP, nfix = bpfK )
      drt1_bpf<- bkfilter( mcData[[ k ]][ TmaskStat, "DEBT_RT_1", j ] , pl = lowP, pu = highP, nfix = bpfK )
      drt2_bpf<- bkfilter( mcData[[ k ]][ TmaskStat, "DEBT_RT_2", j ] , pl = lowP, pu = highP, nfix = bpfK )
      drt3_bpf<- bkfilter( mcData[[ k ]][ TmaskStat, "DEBT_RT_3", j ] , pl = lowP, pu = highP, nfix = bpfK )
      drt_fi_bpf<- bkfilter( mcData[[ k ]][ TmaskStat, "DEBT_RT_FI", j ] , pl = lowP, pu = highP, nfix = bpfK )
      drt_cl_bpf<- bkfilter( mcData[[ k ]][ TmaskStat, "DEBT_RT_CL", j ] , pl = lowP, pu = highP, nfix = bpfK )
      fshhi_bpf<- bkfilter( mcData[[ k ]][ TmaskStat, "FS_HHI", j ] , pl = lowP, pu = highP, nfix = bpfK )
      fslev_bpf<- bkfilter( mcData[[ k ]][ TmaskStat, "FS_LEV", j ] , pl = lowP, pu = highP, nfix = bpfK )
      fsstr_bpf<- bkfilter( mcData[[ k ]][ TmaskStat, "FS_STR", j ] , pl = lowP, pu = highP, nfix = bpfK )
      fsdr_bpf<- bkfilter( mcData[[ k ]][ TmaskStat, "FS_DR", j ] , pl = lowP, pu = highP, nfix = bpfK )
      fspr_bpf<- bkfilter( log0( mcData[[ k ]][ TmaskStat, "FS_PR", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      fsdef_bpf<- bkfilter( log0( mcData[[ k ]][ TmaskStat, "FS_DEF", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      ponzi_bpf<- bkfilter( mcData[[ k ]][ TmaskStat, "PONZI", j ] , pl = lowP, pu = highP, nfix = bpfK )
      spec_bpf<- bkfilter( mcData[[ k ]][ TmaskStat, "SPEC", j ] , pl = lowP, pu = highP, nfix = bpfK )
      hedge_bpf<- bkfilter( mcData[[ k ]][ TmaskStat, "HEDGE", j ] , pl = lowP, pu = highP, nfix = bpfK )
      rescue_bpf<- bkfilter( mcData[[ k ]][ TmaskStat, "Financial_Sector_Rescue", j ] , pl = lowP, pu = highP, nfix = bpfK )
      
      # Augmented Dickey-Fuller tests for unit roots
      adf_gdp_r[[ j ]]<- adf.test( log0( mcData[[ k ]][ TmaskStat, "Real_GDP", j ] ) )
      adf_con_r[[ j ]]<- adf.test( log0( mcData[[ k ]][ TmaskStat, "C_r", j ] ) )
      adf_inv_r[[ j ]]<- adf.test( log0( mcData[[ k ]][ TmaskStat, "I_r", j ] ) )
      adf_gov_r[[ j ]]<- adf.test( log0( mcData[[ k ]][ TmaskStat, "G_r", j ] ) )
      adf_imp_r[[ j ]]<- adf.test( log0( mcData[[ k ]][ TmaskStat, "M_r", j ] ) )
      adf_x[[ j ]]<- adf.test( log0( mcData[[ k ]][ TmaskStat, "X_r", j ] ) )
      adf_nx[[ j ]]<- adf.test( log0( mcData[[ k ]][ TmaskStat, "NX_r", j ] ) )
      adf_p[[ j ]]<- adf.test( log0( mcData[[ k ]][ TmaskStat, "P", j ] ) )
      adf_k[[ j ]]<- adf.test( log0( mcData[[ k ]][ TmaskStat, "K_r", j ] ) )
      adf_inve[[ j ]]<- adf.test( log0( mcData[[ k ]][ TmaskStat, "INVE_r", j ] ) )
      adf_emp[[ j ]]<- adf.test( log0( mcData[[ k ]][ TmaskStat, "EMP", j ] ) )
      adf_profits[[ j ]]<- adf.test( log0( mcData[[ k ]][ TmaskStat, "PROFITS", j ] ) )
      adf_wage[[ j ]]<- adf.test( log0( mcData[[ k ]][ TmaskStat, "WAGE", j ] ) )
      adf_prod[[ j ]]<- adf.test( log0( mcData[[ k ]][ TmaskStat, "PROD", j ] ) )
      adf_mk[[ j ]]<- adf.test( log0( mcData[[ k ]][ TmaskStat, "MK", j ] ) )
      adf_kl[[ j ]]<- adf.test( log0( mcData[[ k ]][ TmaskStat, "KL", j ] ) )
      adf_u[[ j ]]<- adf.test( mcData[[ k ]][ TmaskStat, "U", j ] ) 
      adf_pr_sh[[ j ]]<- adf.test( mcData[[ k ]][ TmaskStat, "Profit_Share", j ]  )
      adf_wg_sh[[ j ]]<- adf.test( mcData[[ k ]][ TmaskStat, "Wage_Share", j ]  )
      adf_infla[[ j ]]<- adf.test( mcData[[ k ]][ TmaskStat, "P_G", j ]  )
      adf_pcu[[ j ]]<- adf.test( mcData[[ k ]][ TmaskStat, "PCU", j ]  )
      adf_pr[[ j ]]<- adf.test( mcData[[ k ]][ TmaskStat, "PR", j ]  )
      adf_cgdp[[ j ]]<- adf.test( mcData[[ k ]][ TmaskStat, "CGDP", j ]  )
      adf_igdp[[ j ]]<- adf.test( mcData[[ k ]][ TmaskStat, "IGDP", j ]  )
      adf_ggdp[[ j ]]<- adf.test( mcData[[ k ]][ TmaskStat, "GGDP", j ]  )
      adf_nxgdp[[ j ]]<- adf.test( mcData[[ k ]][ TmaskStat, "NXGDP", j ]  )
      adf_invgdp[[ j ]]<- adf.test( mcData[[ k ]][ TmaskStat, "INVGDP", j ]  )
      adf_kgdp[[ j ]]<- adf.test( mcData[[ k ]][ TmaskStat, "KGDP", j ]  )
      adf_drtc[[ j ]]<- adf.test( mcData[[ k ]][ TmaskStat, "DEBT_RT_C", j ] ) 
      adf_drtk[[ j ]]<- adf.test( mcData[[ k ]][ TmaskStat, "DEBT_RT_K", j ] ) 
      adf_drti[[ j ]]<- adf.test( mcData[[ k ]][ TmaskStat, "DEBT_RT_I", j ] ) 
      adf_drt1[[ j ]]<- adf.test( mcData[[ k ]][ TmaskStat, "DEBT_RT_1", j ] ) 
      adf_drt2[[ j ]]<- adf.test( mcData[[ k ]][ TmaskStat, "DEBT_RT_2", j ] ) 
      adf_drt3[[ j ]]<- adf.test( mcData[[ k ]][ TmaskStat, "DEBT_RT_3", j ] ) 
      adf_drt_fi[[ j ]]<- adf.test( mcData[[ k ]][ TmaskStat, "DEBT_RT_FI", j ] ) 
      adf_drt_cl[[ j ]]<- adf.test( mcData[[ k ]][ TmaskStat, "DEBT_RT_CL", j ] ) 
      adf_fshhi[[ j ]]<- adf.test( mcData[[ k ]][ TmaskStat, "FS_HHI", j ] ) 
      adf_fslev[[ j ]]<- adf.test( mcData[[ k ]][ TmaskStat, "FS_LEV", j ] ) 
      adf_fsstr[[ j ]]<- adf.test( mcData[[ k ]][ TmaskStat, "FS_STR", j ] )
      adf_fsdr[[ j ]]<- adf.test( mcData[[ k ]][ TmaskStat, "FS_DR", j ] )
      adf_ponzi[[ j ]]<- adf.test( mcData[[ k ]][ TmaskStat, "PONZI", j ] )
      adf_spec[[ j ]]<- adf.test( mcData[[ k ]][ TmaskStat, "SPEC", j ] )
      adf_hedge[[ j ]]<- adf.test( mcData[[ k ]][ TmaskStat, "HEDGE", j ] )
      adf_dst_g[[ j ]]<- adf.test( mcData[[ k ]][ TmaskStat, "DEBT_FS_ST_G", j ] )
      adf_dlt_g[[ j ]]<- adf.test( mcData[[ k ]][ TmaskStat, "DEBT_FS_LT_G", j ] )
      adf_d_g[[ j ]]<- adf.test( mcData[[ k ]][ TmaskStat, "DEBT_FS_G", j ] )
      adf_dep_g[[ j ]]<- adf.test( mcData[[ k ]][ TmaskStat, "DEP_FS_G", j ] )
      
      # Standard deviations of filtered series
      gdp_r_sd[ j ]<- sd( gdp_bpf$cycle[ TmaskBpf, 1 ] )
      con_r_sd[ j ]<- sd( con_bpf$cycle[ TmaskBpf, 1 ] )
      inv_r_sd[ j ]<- sd( inv_bpf$cycle[ TmaskBpf, 1 ] )
      gov_r_sd[ j ]<- sd( gov_bpf$cycle[ TmaskBpf, 1 ] )
      imp_r_sd[ j ]<- sd( imp_bpf$cycle[ TmaskBpf, 1 ] )
      x_r_sd[ j ]<- sd( x_bpf$cycle[ TmaskBpf, 1 ] )
      nx_r_sd[ j ]<- sd( nx_bpf$cycle[ TmaskBpf, 1 ] )
      p_sd[ j ]<- sd( p_bpf$cycle[ TmaskBpf, 1 ] )
      k_sd[ j ]<- sd( k_bpf$cycle[ TmaskBpf, 1 ] )
      inve_sd[ j ]<- sd( inve_bpf$cycle[ TmaskBpf, 1 ] )
      emp_sd[ j ]<- sd( emp_bpf$cycle[ TmaskBpf, 1 ] )
      profits_sd[ j ]<- sd( profits_bpf$cycle[ TmaskBpf, 1 ] )
      wage_sd[ j ]<- sd( wage_bpf$cycle[ TmaskBpf, 1 ] )
      prod_sd[ j ]<- sd( prod_bpf$cycle[ TmaskBpf, 1 ] )
      mk_sd[ j ]<- sd( mk_bpf$cycle[ TmaskBpf, 1 ] )
      kl_sd[ j ]<- sd( kl_bpf$cycle[ TmaskBpf, 1 ] )
      infla_sd[ j ]<- sd( infla_bpf$cycle[ TmaskBpf, 1 ] )
      u_sd[ j ]<- sd( u_bpf$cycle[ TmaskBpf, 1 ] )
      pcu_sd[ j ]<- sd( pcu_bpf$cycle[ TmaskBpf, 1 ] )
      pr_sd[ j ]<- sd( pr_bpf$cycle[ TmaskBpf, 1 ] )
      cgdp_sd[ j ]<- sd( cgdp_bpf$cycle[ TmaskBpf, 1 ] )
      igdp_sd[ j ]<- sd( igdp_bpf$cycle[ TmaskBpf, 1 ] )
      ggdp_sd[ j ]<- sd( ggdp_bpf$cycle[ TmaskBpf, 1 ] )
      nxgdp_sd[ j ]<- sd( nxgdp_bpf$cycle[ TmaskBpf, 1 ] )
      invgdp_sd[ j ]<- sd( invgdp_bpf$cycle[ TmaskBpf, 1 ] )
      kgdp_sd[ j ]<- sd( kgdp_bpf$cycle[ TmaskBpf, 1 ] )
      debtfs_sd[ j ]<- sd( debtfs_bpf$cycle[ TmaskBpf, 1 ] )
      debtfs_st_sd[ j ]<- sd( debtfs_st_bpf$cycle[ TmaskBpf, 1 ] )
      debtfs_lt_sd[ j ]<- sd( debtfs_lt_bpf$cycle[ TmaskBpf, 1 ] )
      depfs_sd[ j ]<- sd( depfs_bpf$cycle[ TmaskBpf, 1 ] )
      drtc_sd[ j ]<- sd( drtc_bpf$cycle[ TmaskBpf, 1 ] )
      drtk_sd[ j ]<- sd( drtk_bpf$cycle[ TmaskBpf, 1 ] )
      drti_sd[ j ]<- sd( drti_bpf$cycle[ TmaskBpf, 1 ] )
      drt1_sd[ j ]<- sd( drt1_bpf$cycle[ TmaskBpf, 1 ] )
      drt2_sd[ j ]<- sd( drt2_bpf$cycle[ TmaskBpf, 1 ] )
      drt3_sd[ j ]<- sd( drt3_bpf$cycle[ TmaskBpf, 1 ] )
      drt_fi_sd[ j ]<- sd( drt_fi_bpf$cycle[ TmaskBpf, 1 ] )
      drt_cl_sd[ j ]<- sd( drt_cl_bpf$cycle[ TmaskBpf, 1 ] )
      fshhi_sd[ j ]<- sd( fshhi_bpf$cycle[ TmaskBpf, 1 ] )
      fslev_sd[ j ]<- sd( fslev_bpf$cycle[ TmaskBpf, 1 ] )
      fsstr_sd[ j ]<- sd( fsstr_bpf$cycle[ TmaskBpf, 1 ] )
      fsdr_sd[ j ]<- sd( fsdr_bpf$cycle[ TmaskBpf, 1 ] )
      fspr_sd[ j ]<- sd( fspr_bpf$cycle[ TmaskBpf, 1 ] )
      fsdef_sd[ j ]<- sd( fsdef_bpf$cycle[ TmaskBpf, 1 ] )
      ponzi_sd[ j ]<- sd( ponzi_bpf$cycle[ TmaskBpf, 1 ] )
      spec_sd[ j ]<- sd( spec_bpf$cycle[ TmaskBpf, 1 ] )
      hedge_sd[ j ]<- sd( hedge_bpf$cycle[ TmaskBpf, 1 ] )
      
      # Build the correlation structures
      gdp_gdp[[ j ]]  <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ],gdp_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      cr_gdp[[ j ]] <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ], con_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      ir_gdp[[ j ]] <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ], inv_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      gov_gdp[[ j ]] <- ccf(gdp_bpf$cycle[ TmaskBpf, 1 ], gov_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      imp_gdp[[ j ]] <- ccf(gdp_bpf$cycle[ TmaskBpf, 1 ], imp_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      x_gdp[[ j ]] <- ccf(gdp_bpf$cycle[ TmaskBpf, 1 ], x_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      nx_gdp[[ j ]] <- ccf(gdp_bpf$cycle[ TmaskBpf, 1 ], nx_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      p_gdp[[ j ]] <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ], p_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      k_gdp[[ j ]] <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ], k_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      inve_gdp[[ j ]] <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ], inve_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      emp_gdp[[ j ]] <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ], emp_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      profits_gdp[[ j ]] <- ccf(gdp_bpf$cycle[ TmaskBpf, 1 ], profits_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      wage_gdp[[ j ]] <- ccf(gdp_bpf$cycle[ TmaskBpf, 1 ],wage_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      prod_gdp[[ j ]] <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ], prod_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      mk_gdp[[ j ]] <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ], mk_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      kl_gdp[[ j ]] <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ], kl_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      infla_gdp[[ j ]] <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ], infla_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      u_gdp[[ j ]] <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ], u_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      pr_sh_gdp[[ j ]] <- ccf(gdp_bpf$cycle[ TmaskBpf, 1 ], pr_sh_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      wg_sh_gdp[[ j ]] <- ccf(gdp_bpf$cycle[ TmaskBpf, 1 ], wg_sh_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      pcu_gdp[[ j ]] <- ccf(gdp_bpf$cycle[ TmaskBpf, 1 ], pcu_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      pr_gdp[[ j ]] <- ccf(gdp_bpf$cycle[ TmaskBpf, 1 ], pr_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      cgdp_gdp[[ j ]] <- ccf(gdp_bpf$cycle[ TmaskBpf, 1 ], cgdp_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      igdp_gdp[[ j ]] <- ccf(gdp_bpf$cycle[ TmaskBpf, 1 ], igdp_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      ggdp_gdp[[ j ]] <- ccf(gdp_bpf$cycle[ TmaskBpf, 1 ], ggdp_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      nxgdp_gdp[[ j ]] <- ccf(gdp_bpf$cycle[ TmaskBpf, 1 ], nxgdp_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      invgdp_gdp[[ j ]] <- ccf(gdp_bpf$cycle[ TmaskBpf, 1 ], invgdp_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      kgdp_gdp[[ j ]] <- ccf(gdp_bpf$cycle[ TmaskBpf, 1 ], kgdp_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      debtfs_gdp[[ j ]]  <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ], debtfs_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      debtfs_st_gdp[[ j ]]  <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ], debtfs_st_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      debtfs_lt_gdp[[ j ]]  <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ], debtfs_lt_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      depfs_gdp[[ j ]]  <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ], depfs_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      drt_fi_gdp[[ j ]]  <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ], drt_fi_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      drt_cl_gdp[[ j ]]  <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ], drt_cl_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      fshhi_gdp[[ j ]]  <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ], fshhi_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      fslev_gdp[[ j ]]  <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ], fslev_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      fsstr_gdp[[ j ]]  <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ], fsstr_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      fspr_gdp[[ j ]]  <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ], fspr_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      fsdr_gdp[[ j ]]  <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ], fsdr_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      fsdef_gdp[[ j ]]  <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ], fsdef_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      ponzi_gdp[[ j ]]  <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ], ponzi_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      spec_gdp[[ j ]]  <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ], spec_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      hedge_gdp[[ j ]]  <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ], hedge_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      rescue_gdp[[ j ]]  <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ], rescue_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      
      drt_fi_debt[[ j ]]  <- ccf( debtfs_bpf$cycle[ TmaskBpf, 1 ], drt_fi_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      drt_cl_debt[[ j ]]  <- ccf( debtfs_bpf$cycle[ TmaskBpf, 1 ], drt_cl_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      fshhi_debt[[ j ]]  <- ccf( debtfs_bpf$cycle[ TmaskBpf, 1 ], fshhi_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      fslev_debt[[ j ]]  <- ccf( debtfs_bpf$cycle[ TmaskBpf, 1 ], fslev_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      fsstr_debt[[ j ]]  <- ccf( debtfs_bpf$cycle[ TmaskBpf, 1 ], fsstr_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      fspr_debt[[ j ]]  <- ccf( debtfs_bpf$cycle[ TmaskBpf, 1 ], fspr_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      fsdr_debt[[ j ]]  <- ccf( debtfs_bpf$cycle[ TmaskBpf, 1 ], fsdr_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      fsdef_debt[[ j ]]  <- ccf( debtfs_bpf$cycle[ TmaskBpf, 1 ], fsdef_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      ponzi_debt[[ j ]]  <- ccf( debtfs_bpf$cycle[ TmaskBpf, 1 ], ponzi_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      spec_debt[[ j ]]  <- ccf( debtfs_bpf$cycle[ TmaskBpf, 1 ], spec_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      hedge_debt[[ j ]]  <- ccf( debtfs_bpf$cycle[ TmaskBpf, 1 ], hedge_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      rescue_debt[[ j ]]  <- ccf( debtfs_bpf$cycle[ TmaskBpf, 1 ], rescue_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      
      drt_fi_inv[[ j ]]  <- ccf( inv_bpf$cycle[ TmaskBpf, 1 ], drt_fi_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      drt_cl_inv[[ j ]]  <- ccf( inv_bpf$cycle[ TmaskBpf, 1 ], drt_cl_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      fshhi_inv[[ j ]]  <- ccf( inv_bpf$cycle[ TmaskBpf, 1 ], fshhi_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      fslev_inv[[ j ]]  <- ccf( inv_bpf$cycle[ TmaskBpf, 1 ], fslev_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      fsstr_inv[[ j ]]  <- ccf( inv_bpf$cycle[ TmaskBpf, 1 ], fsstr_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      fspr_inv[[ j ]]  <- ccf( inv_bpf$cycle[ TmaskBpf, 1 ], fspr_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      fsdr_inv[[ j ]]  <- ccf( inv_bpf$cycle[ TmaskBpf, 1 ], fsdr_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      fsdef_inv[[ j ]]  <- ccf( inv_bpf$cycle[ TmaskBpf, 1 ], fsdef_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      ponzi_inv[[ j ]]  <- ccf( inv_bpf$cycle[ TmaskBpf, 1 ], ponzi_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
      spec_inv[[ j ]]  <- ccf( inv_bpf$cycle[ TmaskBpf, 1 ], spec_bpf$cycle[ TmaskBpf, 1 ],  lag.max = lags, plot = FALSE, na.action = na.pass )
      hedge_inv[[ j ]]  <- ccf( inv_bpf$cycle[ TmaskBpf, 1 ], hedge_bpf$cycle[ TmaskBpf, 1 ],lag.max = lags, plot = FALSE, na.action = na.pass )
      rescue_inv[[ j ]]  <- ccf( inv_bpf$cycle[ TmaskBpf, 1 ], rescue_bpf$cycle[ TmaskBpf, 1 ], lag.max = lags, plot = FALSE, na.action = na.pass )
    }
    
    # Applies t test to the mean lag results to test their significance (H0: lag < critCorr)
    for(i in 1 : (2 * lags + 1) ){ #do for all lags
      if(i != lags + 1)  # don't try to compute autocorrelation at lag 0
      gdp_gdp_pval[ i ]<- t.test0( abs( unname( sapply( gdp_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      cr_gdp_pval[ i ]<- t.test0( abs( unname( sapply( cr_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      ir_gdp_pval[ i ]<- t.test0( abs( unname( sapply( ir_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      gov_gdp_pval[ i ]<- t.test0( abs( unname( sapply( gov_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      imp_gdp_pval[ i ]<- t.test0( abs( unname( sapply( imp_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      x_gdp_pval[ i ]<- t.test0( abs( unname( sapply( x_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      nx_gdp_pval[ i ]<- t.test0( abs( unname( sapply( nx_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      p_gdp_pval[ i ]<- t.test0( abs( unname( sapply( p_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      k_gdp_pval[ i ]<- t.test0( abs( unname( sapply( k_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      inve_gdp_pval[ i ]<- t.test0( abs( unname( sapply( inve_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      emp_gdp_pval[ i ]<- t.test0( abs( unname( sapply( emp_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      wage_gdp_pval[ i ]<- t.test0( abs( unname( sapply( wage_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      profits_gdp_pval[ i ]<- t.test0( abs( unname( sapply( profits_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      wage_gdp_pval[ i ]<- t.test0( abs( unname( sapply( wage_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      prod_gdp_pval[ i ]<- t.test0( abs( unname( sapply( prod_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      mk_gdp_pval[ i ]<- t.test0( abs( unname( sapply( mk_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      kl_gdp_pval[ i ]<- t.test0( abs( unname( sapply( kl_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      infla_gdp_pval[ i ]<- t.test0( abs( unname( sapply( infla_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      u_gdp_pval[ i ]<- t.test0( abs( unname( sapply( u_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      pr_sh_gdp_pval[ i ]<- t.test0( abs( unname( sapply( pr_sh_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      wg_sh_gdp_pval[ i ]<- t.test0( abs( unname( sapply( wg_sh_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      pcu_gdp_pval[ i ]<- t.test0( abs( unname( sapply( pcu_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      pr_gdp_pval[ i ]<- t.test0( abs( unname( sapply( pr_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      cgdp_gdp_pval[ i ]<- t.test0( abs( unname( sapply( cgdp_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      igdp_gdp_pval[ i ]<- t.test0( abs( unname( sapply( igdp_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      ggdp_gdp_pval[ i ]<- t.test0( abs( unname( sapply( ggdp_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      nxgdp_gdp_pval[ i ]<- t.test0( abs( unname( sapply( nxgdp_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      invgdp_gdp_pval[ i ]<- t.test0( abs( unname( sapply( invgdp_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      kgdp_gdp_pval[ i ]<- t.test0( abs( unname( sapply( kgdp_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      debtfs_gdp_pval[ i ]<- t.test0( abs( unname( sapply( debtfs_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      debtfs_st_gdp_pval[ i ]<- t.test0( abs( unname( sapply(  debtfs_st_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      debtfs_lt_gdp_pval[ i ]<- t.test0( abs( unname( sapply( debtfs_lt_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      depfs_gdp_pval[ i ]<- t.test0( abs( unname( sapply( depfs_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      drt_fi_gdp_pval[ i ]<- t.test0( abs( unname( sapply( drt_fi_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      drt_cl_gdp_pval[ i ]<- t.test0( abs( unname( sapply( drt_cl_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      fshhi_gdp_pval[ i ]<- t.test0( abs( unname( sapply( fshhi_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      fslev_gdp_pval[ i ]<- t.test0( abs( unname( sapply( fslev_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      fsstr_gdp_pval[ i ]<- t.test0( abs( unname( sapply( fsstr_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      fspr_gdp_pval[ i ]<- t.test0( abs( unname( sapply( fspr_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      fsdr_gdp_pval[ i ]<- t.test0( abs( unname( sapply( fsdr_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      ponzi_gdp_pval[ i ]<- t.test0( abs( unname( sapply( ponzi_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      spec_gdp_pval[ i ]<- t.test0( abs( unname( sapply( spec_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      hedge_gdp_pval[ i ]<- t.test0( abs( unname( sapply( hedge_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
    }
    
    #
    # ---- Summary statistics table (averages, standard errors and p-values) ----
    #
    
    key.stats <- matrix(
      c(
        ## avg. growth rate
        mean( gdp_gr ), 
        mean( cr_gr ), 
        mean( ir_gr ), 
        mean( gr_gr ), 
        mean( mr_gr ),
        mean( x_gr ), 
        mean( nx_gr ),
        mean( infla ), 
        mean( profits_gr ), 
        mean( wage_gr ),
        mean( pr_sh ), 
        mean( wg_sh ), 
        mean( mk_gr ), 
        mean( pr_rt ),
        mean( prod_gr ),
        mean( pcu_rt ),
        mean( inve_gr ), 
        mean( k_gr ),
        mean( u_rt ), 
        mean( emp_gr ),
        mean( kl_rt ),
        mean( cgdp_rt ),
        mean( igdp_rt ),
        mean( ggdp_rt ), 
        mean( nxgdp_rt ),
        mean( invgdp_rt ), 
        mean( kgdp_rt ),
        mean( debtfsst_gr ), 
        mean( debtfslt_gr ), 
        mean( debtfs_gr ), 
        mean( depfs_gr ),
        mean( fs_hhi  ), 
        mean( fs_lev  ), 
        mean( fs_str  ),
        mean( fs_dr  ), 
        mean( ponzi  ), 
        mean( spec  ), 
        mean( hedge  ),
        mean( debtc_rt ),
        mean( debtk_rt ),
        mean( debti_rt ), 
        mean( debt1_rt ),
        mean( debt2_rt ), 
        mean( debt3_rt ),
        mean( debt_fi_rt ),
        mean( debt_cl_rt ),
        
        ## (Std. Dev.)
        sd( gdp_gr ),
        sd( cr_gr ),
        sd( ir_gr ),
        sd( gr_gr ),
        sd( mr_gr ),
        sd( x_gr ),
        sd( nx_gr ),
        sd( infla ),
        sd( profits_gr ),
        sd( wage_gr ),
        sd( pr_sh ),
        sd( wg_sh ),
        sd( mk_gr ), 
        sd( pr_rt ),
        sd( prod_gr ),
        sd( pcu_rt ),
        sd( inve_gr ),
        sd( k_gr ),
        sd( u_rt ),
        sd( emp_gr ),
        sd( kl_rt ),
        sd( cgdp_rt ),
        sd( igdp_rt ),
        sd( ggdp_rt ),
        sd( nxgdp_rt ),
        sd( invgdp_rt ),
        sd( kgdp_rt ),
        sd( debtfsst_gr ), 
        sd( debtfslt_gr ),
        sd( debtfs_gr ), 
        sd( depfs_gr ),
        sd( fs_hhi  ), 
        sd( fs_lev  ), 
        sd( fs_str  ),
        sd( fs_dr  ),
        sd( ponzi ),
        sd( spec ), 
        sd( hedge ),
        sd( debtc_rt ),
        sd( debtk_rt ),
        sd( debti_rt ),
        sd( debt1_rt ),
        sd( debt2_rt ),
        sd( debt3_rt ),
        sd( debt_fi_rt ),
        sd( debt_cl_rt ),
        
        ## ADF test (logs)
        mean( unname( sapply( adf_gdp_r, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_con_r, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_inv_r, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_gov_r, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_imp_r, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_x, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_nx, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_p, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_profits, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_wage, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_pr_sh, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_wg_sh, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_mk, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_pr, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_prod, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_pcu, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_inve, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_k, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_u, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_emp, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_kl, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_cgdp, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_igdp, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_ggdp, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_nxgdp, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_invgdp, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_kgdp, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_dst_g, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_dlt_g, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_d_g, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_dep_g, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_fshhi, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_fslev, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_fsstr, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_fsdr, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_ponzi, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_spec, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_hedge, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_drtc, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_drtk, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_drti, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_drt1, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_drt2, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_drt3, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_drt_fi, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_drt_cl, `[[`, "statistic" ) ) ),
        
        ## ADF test (logs) p.value
        mean( unname( sapply( adf_gdp_r, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_con_r, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_inv_r, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_gov_r, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_imp_r, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_x, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_nx, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_p, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_profits, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_wage, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_pr_sh, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_wg_sh, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_mk, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_pr, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_prod, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_pcu, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_inve, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_k, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_u, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_emp, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_kl, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_cgdp, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_igdp, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_ggdp, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_nxgdp, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_invgdp, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_kgdp, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_dst_g, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_dlt_g, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_d_g, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_dep_g, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_fshhi, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_fslev, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_fsstr, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_fsdr, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_ponzi, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_spec, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_hedge, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_drtc, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_drtk, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_drti, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_drt1, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_drt2, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_drt3, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_drt_fi, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_drt_cl, `[[`, "p.value" ) ) ),
        
        ## S.d. of bpf series
        mean( gdp_r_sd ), 
        mean( con_r_sd ), 
        mean( inv_r_sd ),
        mean( gov_r_sd ), 
        mean( imp_r_sd ), 
        mean( x_r_sd ), 
        mean( nx_r_sd ), 
        mean( p_sd ),
        mean( profits_sd ), 
        mean( wage_sd ),
        mean( pr_sh_sd ),
        mean( wg_sh_sd ),
        mean( mk_sd ),
        mean( pr_sd ), 
        mean( prod_sd ),
        mean( pcu_sd ),
        mean( inve_sd ),
        mean( k_sd ),
        mean( u_sd ),
        mean( emp_sd ),
        mean( kl_sd ),
        mean( cgdp_sd ),
        mean( igdp_sd ),
        mean( ggdp_sd ),
        mean( nxgdp_sd ),
        mean( invgdp_sd ),
        mean( kgdp_sd ),
        mean( debtfs_st_sd ), 
        mean( debtfs_lt_sd ), 
        mean( debtfs_sd ),
        mean( depfs_sd ),
        mean( fshhi_sd ),
        mean( fslev_sd ), 
        mean( fsstr_sd ),
        mean( fsdr_sd ),
        mean( ponzi_sd ),
        mean( spec_sd ),
        mean( hedge_sd ),
        mean( drtc_sd ),
        mean( drtk_sd ),
        mean( drti_sd ),
        mean( drt1_sd ),
        mean( drt2_sd ),
        mean( drt3_sd ),
        mean( drt_fi_sd ),
        mean( drt_cl_sd ),
        
        ## relative s.d. (to GDP)
        1, 
        mean( con_r_sd ) / mean( gdp_r_sd ), 
        mean( inv_r_sd ) / mean( gdp_r_sd ),
        mean( gov_r_sd ) / mean( gdp_r_sd ), 
        mean( imp_r_sd ) / mean( gdp_r_sd ),
        mean( x_r_sd ) / mean( gdp_r_sd ),
        mean( nx_r_sd ) / mean( gdp_r_sd ),
        mean( p_sd ) / mean( gdp_r_sd ), 
        mean( profits_sd ) / mean( gdp_r_sd ),
        mean( wage_sd ) / mean( gdp_r_sd ), 
        mean( pr_sh_sd ) / mean( gdp_r_sd ),
        mean( wg_sh_sd ) / mean( gdp_r_sd ),
        mean( mk_sd ) / mean(gdp_r_sd ),
        mean( pr_sd ) / mean( gdp_r_sd ),
        mean( prod_sd ) / mean(gdp_r_sd ),
        mean( pcu_sd ) / mean(gdp_r_sd ),
        mean( inve_sd ) / mean(gdp_r_sd ),
        mean( k_sd ) / mean(gdp_r_sd ),
        mean( u_sd ) / mean(gdp_r_sd ),
        mean( emp_sd ) / mean(gdp_r_sd ),
        mean( kl_sd ) / mean(gdp_r_sd ),
        mean( cgdp_sd ) / mean(gdp_r_sd ),
        mean( igdp_sd ) / mean(gdp_r_sd ),
        mean( ggdp_sd ) / mean(gdp_r_sd ),
        mean( nxgdp_sd ) / mean(gdp_r_sd ),
        mean( invgdp_sd ) / mean(gdp_r_sd ),
        mean( kgdp_sd ) / mean(gdp_r_sd ),
        mean( debtfs_st_sd ) / mean( gdp_r_sd ), 
        mean( debtfs_lt_sd ) / mean( gdp_r_sd ), 
        mean( debtfs_sd ) / mean( gdp_r_sd ),
        mean( depfs_sd ) / mean( gdp_r_sd ),
        mean( fshhi_sd ) / mean( gdp_r_sd ), 
        mean( fslev_sd ) / mean( gdp_r_sd ),
        mean( fsstr_sd ) / mean( gdp_r_sd ), 
        mean( fsdr_sd ) / mean( gdp_r_sd ),
        mean( ponzi_sd ) / mean( gdp_r_sd ),
        mean( spec_sd ) / mean(gdp_r_sd ),
        mean( hedge_sd ) / mean( gdp_r_sd ),
        mean( drtc_sd ) / mean(gdp_r_sd ),
        mean( drtk_sd ) / mean(gdp_r_sd ),
        mean( drti_sd ) / mean(gdp_r_sd ),
        mean( drt1_sd ) / mean(gdp_r_sd ),
        mean( drt2_sd ) / mean(gdp_r_sd ),
        mean( drt3_sd ) / mean(gdp_r_sd ),
        mean( drt_fi_sd ) / mean(gdp_r_sd ),
        mean( drt_cl_sd ) / mean(gdp_r_sd )
        
      ),
      ncol = 6, byrow = F)
    
    rownames( key.stats ) <- c( "GDP (output)", 
                                  "Consumption", 
                                  "Investment", 
                                  "Gov. Expend.",
                                  "Imports",
                                  "Exports",
                                  "Net Exports",
                                  "Inflation", 
                                  "Profit", 
                                  "Wages",
                                  "Profit Share",
                                  "Wage Share",
                                  "Markup", 
                                  "Profit Rate",
                                  "Productivity",
                                  "Capacity Utilization",
                                  "Inventories",
                                  "Capital Stock",
                                  "Unemployment",
                                  "Employment",
                                  "Capital-Labor Ratio",
                                  "Consumption (Share of GDP)",
                                  "Investment (Share of GDP)",
                                  "Government Expenses (Share of GDP)",
                                  "Net Exports (Share of GDP)",
                                  "Inventories (Share of GDP)",
                                  "Capital Stock (Share of GDP)",
                                  "Short Term Loans", 
                                  "Long Term Loans", 
                                  "Total Loans", 
                                  "Total Deposits",
                                  "Financial Sector I.HHI", 
                                  "Financial Sector Leverage", 
                                  "Financial Sector Short Term Rate",
                                  "Financial Sector Default Rate",
                                  "Share of Ponzi Firms",
                                  "Share of Speculative Firms", 
                                  "Share of Hedge Firms",
                                  "C. Sector Debt Rate",
                                  "K. Sector Debt Rate",
                                  "I. Sector Debt Rate",
                                  "Class A Debt Rate",
                                  "Class B Debt Rate",
                                  "Class C Debt Rate",
                                  "Avg. Firms Debt Rate",
                                  "Avg. Class Debt Rate"
                                  )
    
    colnames( key.stats ) <- c(   "Avg. (Growth) Rate",
                                  "Std. Dev.",
                                  "ADF test (logs)",
                                  "ADF p-val", 
                                  " s.d. (bpf)", 
                                  " s.d. relative to GDP" )
    
    textplot( formatC( key.stats, digits = sDigits, format = "g" ), cmar = 2 )
    title <- paste( "Key statistics and unit roots tests for cycles (", legends[ k ], ")" )
    subTitle <- paste( eval( bquote(paste0( "( bpf: Baxter-King bandpass-filtered series, low = ", .( lowP ),
                                            "Q / high = ", .( highP ), "Q / order = ", .( bpfK ),
                                            " / MC runs = ", .( nSize ), " / period = ",
                                            .( warmUpStat + 1 ), " - ", .( nTstat ), " )" ) ) ),
                       eval( bquote( paste0( "( ADF test H0: there are unit roots / non-stationary at ",
                                             .( (1 - CI ) * 100), "% level", " )" ) ) ), sep ="\n" )
    title( main = title, sub = subTitle )
    
    # Write tables to the disk as CSV files for Excel
    write.csv( key.stats , quote = FALSE,
               paste0( folder, "/", outDir, "/", repName, k, "_key_stats.csv" ) )
    
  
   
    
    #
    # ---- Correlation structure tables (lags, standard errors and p-values) ----
    #
    corr.struct <- matrix(c(  colMeans(t( unname( sapply(gdp_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colMeans(t( unname( sapply(cr_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colMeans(t( unname( sapply(ir_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colMeans(t( unname( sapply(gov_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colMeans(t( unname( sapply(imp_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colMeans(t( unname( sapply(x_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colMeans(t( unname( sapply(nx_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colMeans(t( unname( sapply(p_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colMeans(t( unname( sapply(profits_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colMeans(t( unname( sapply(wage_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colMeans(t( unname( sapply(pr_sh_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colMeans(t( unname( sapply(wg_sh_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colMeans(t( unname( sapply(mk_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colMeans(t( unname( sapply(pr_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colMeans(t( unname( sapply(prod_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colMeans(t( unname( sapply(pcu_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colMeans(t( unname( sapply(inve_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colMeans(t( unname( sapply(k_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colMeans(t( unname( sapply(u_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colMeans(t( unname( sapply(emp_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colMeans(t( unname( sapply(kl_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colMeans(t( unname( sapply(infla_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colMeans(t( unname( sapply(cgdp_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colMeans(t( unname( sapply(igdp_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colMeans(t( unname( sapply(ggdp_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colMeans(t( unname( sapply(nxgdp_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colMeans(t( unname( sapply(invgdp_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colMeans(t( unname( sapply(kgdp_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colMeans(t( unname( sapply(debtfs_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colMeans(t( unname( sapply(debtfs_st_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colMeans(t( unname( sapply(debtfs_lt_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colMeans(t( unname( sapply(depfs_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colMeans(t( unname( sapply(drt_fi_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colMeans(t( unname( sapply(drt_cl_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colMeans(t( unname( sapply(fslev_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colMeans(t( unname( sapply(fspr_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colMeans(t( unname( sapply(fsdr_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colMeans(t( unname( sapply(fsdef_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colMeans(t( unname( sapply(ponzi_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colMeans(t( unname( sapply(spec_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colMeans(t( unname( sapply(hedge_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colMeans(t( unname( sapply(rescue_gdp, `[[`, "acf" ) ) ), na.rm = T)
                            ),
    
    ncol = 2 * lags + 1, byrow = T)
    colnames( corr.struct ) <- gdp_gdp[[1]]$lag
    rownames( corr.struct ) <- c(   "GDP (output)",
                                    "Consumption", 
                                    "Investment",
                                    "Gov. Expenditure",
                                    "Imports",
                                    "Exports", 
                                    "Net Exports", 
                                    "Price",
                                    "Profits", 
                                    "Wages", 
                                    "Profit Share", 
                                    "Wage Share", 
                                    "Markup",
                                    "Profit Rate", 
                                    "Productivity", 
                                    "Capacity Utilization", 
                                    "Inventories", 
                                    "Capital Stock", 
                                    "Unemployment", 
                                    "Employment", 
                                    "Capital-Labor", 
                                    "Inflation",
                                    "Consumption (Share of GDP)", 
                                    "Investment (Share of GDP)", 
                                    "Gov. Expenditure (Share of GDP)",
                                    "Net Exports (Share of GDP)", 
                                    "Inventories (Share of GDP)", 
                                    "Capital Stock (Share of GDP)",
                                    "Total Debt",
                                    "Short Term Loans", 
                                    "Long Term Loans", 
                                    "Total Deposits", 
                                    "Firms Debt Rate", 
                                    "Class Debt Rate", 
                                    "Financial Sector Leverage",
                                    "Financial Sector Profits", 
                                    "Financial Sector Default Rate",
                                    "Defaulted Loans",
                                    "Share of Ponzi Firms", 
                                    "Share of Speculative Firms", 
                                    "Share of Hedge Firms",
                                    "Bank Rescue"
    )
    
    textplot( formatC( corr.struct, digits = sDigits, format = "g" ), cmar = 1 )
    title <- paste( "Correlation structure for GDP (", legends[ k ], ")" )
    subTitle <- paste( eval( bquote( paste0( "( non-rate/ratio series are Baxter-King bandpass-filtered, low = ",
                                             .( lowP ), "Q / high = ", .( highP ), "Q / order = ", .( bpfK ),
                                             " / MC runs = ", .( nSize ), " / period = ",
                                             .( warmUpStat + 1 ), " - ", .( nTstat ), " )" ) ) ),
                       eval( bquote ( paste0( "( test H0: lag coefficient is not significant at ",
                                              .( ( 1 - CI ) * 100), "% level", " )" ) ) ), sep ="\n" )
    title( main = title, sub = subTitle )
    write.csv( corr.struct, quote = FALSE,
               paste0( folder, "/", outDir, "/", repName, k, "_corr_struct.csv" ) )
    
    corr.struct.2 <- matrix(c( colMeans(t( unname( sapply(drt_fi_debt, `[[`, "acf" ) ) ), na.rm = T),
                               colMeans(t( unname( sapply(drt_cl_debt, `[[`, "acf" ) ) ), na.rm = T),
                               colMeans(t( unname( sapply(fslev_debt, `[[`, "acf" ) ) ), na.rm = T),
                               colMeans(t( unname( sapply(fspr_debt, `[[`, "acf" ) ) ), na.rm = T),
                               colMeans(t( unname( sapply(fsdr_debt, `[[`, "acf" ) ) ), na.rm = T),
                               colMeans(t( unname( sapply(fsdef_debt, `[[`, "acf" ) ) ), na.rm = T),
                               colMeans(t( unname( sapply(ponzi_debt, `[[`, "acf" ) ) ), na.rm = T),
                               colMeans(t( unname( sapply(spec_debt, `[[`, "acf" ) ) ), na.rm = T),
                               colMeans(t( unname( sapply(hedge_debt, `[[`, "acf" ) ) ), na.rm = T),
                               colMeans(t( unname( sapply(rescue_debt, `[[`, "acf" ) ) ), na.rm = T)
    ),
    
    ncol = 2 * lags + 1, byrow = T)
    colnames( corr.struct.2 ) <- gdp_gdp[[1]]$lag
    rownames( corr.struct.2 ) <- c( "Firms Debt Rate", 
                                    "Class Debt Rate", 
                                    "Financial Sector Leverage",
                                    "Financial Sector Profits", 
                                    "Financial Sector Default Rate",
                                    "Defaulted Loans",
                                    "Share of Ponzi Firms", 
                                    "Share of Speculative Firms", 
                                    "Share of Hedge Firms",
                                    "Bank Rescue"
    )
    
    textplot( formatC( corr.struct.2, digits = sDigits, format = "g" ), cmar = 1 )
    title <- paste( "Correlation structure for Stock of Credit (", legends[ k ], ")" )
    subTitle <- paste( eval( bquote( paste0( "( non-rate/ratio series are Baxter-King bandpass-filtered, low = ",
                                             .( lowP ), "Q / high = ", .( highP ), "Q / order = ", .( bpfK ),
                                             " / MC runs = ", .( nSize ), " / period = ",
                                             .( warmUpStat + 1 ), " - ", .( nTstat ), " )" ) ) ),
                       eval( bquote ( paste0( "( test H0: lag coefficient is not significant at ",
                                              .( ( 1 - CI ) * 100), "% level", " )" ) ) ), sep ="\n" )
    title( main = title, sub = subTitle )
    write.csv( corr.struct.2, quote = FALSE,
               paste0( folder, "/", outDir, "/", repName, k, "_corr_struct.2.csv" ) )
    
    
    corr.struct.3 <- matrix(c( colMeans(t( unname( sapply(drt_fi_inv, `[[`, "acf" ) ) ), na.rm = T),
                               colMeans(t( unname( sapply(drt_cl_inv, `[[`, "acf" ) ) ), na.rm = T),
                               colMeans(t( unname( sapply(fslev_inv, `[[`, "acf" ) ) ), na.rm = T),
                               colMeans(t( unname( sapply(fspr_inv, `[[`, "acf" ) ) ), na.rm = T),
                               colMeans(t( unname( sapply(fsdr_inv, `[[`, "acf" ) ) ), na.rm = T),
                               colMeans(t( unname( sapply(fsdef_inv, `[[`, "acf" ) ) ), na.rm = T),
                               colMeans(t( unname( sapply(ponzi_inv, `[[`, "acf" ) ) ), na.rm = T),
                               colMeans(t( unname( sapply(spec_inv, `[[`, "acf" ) ) ), na.rm = T),
                               colMeans(t( unname( sapply(hedge_inv, `[[`, "acf" ) ) ), na.rm = T),
                               colMeans(t( unname( sapply(rescue_inv, `[[`, "acf" ) ) ), na.rm = T)
    ),
    
    ncol = 2 * lags + 1, byrow = T)
    colnames( corr.struct.3 ) <- gdp_gdp[[1]]$lag
    rownames( corr.struct.3 ) <- c( "Firms Debt Rate", 
                                    "Class Debt Rate", 
                                    "Financial Sector Leverage",
                                    "Financial Sector Profits", 
                                    "Financial Sector Default Rate",
                                    "Defaulted_Loans",
                                    "Share of Ponzi Firms", 
                                    "Share of Speculative Firms", 
                                    "Share of Hedge Firms",
                                    "Bank Rescue"
    )
    
    textplot( formatC( corr.struct.3, digits = sDigits, format = "g" ), cmar = 1 )
    title <- paste( "Correlation structure for Inverstment (", legends[ k ], ")" )
    subTitle <- paste( eval( bquote( paste0( "( non-rate/ratio series are Baxter-King bandpass-filtered, low = ",
                                             .( lowP ), "Q / high = ", .( highP ), "Q / order = ", .( bpfK ),
                                             " / MC runs = ", .( nSize ), " / period = ",
                                             .( warmUpStat + 1 ), " - ", .( nTstat ), " )" ) ) ),
                       eval( bquote ( paste0( "( test H0: lag coefficient is not significant at ",
                                              .( ( 1 - CI ) * 100), "% level", " )" ) ) ), sep ="\n" )
    title( main = title, sub = subTitle )
    write.csv( corr.struct.3, quote = FALSE,
               paste0( folder, "/", outDir, "/", repName, k, "_corr_struct.3.csv" ) )

    #
    # ---- Aggregated variables stationarity and ergodicity tests ----
    #
    
    # select data to plot
    if( k == bCase ){
      
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


