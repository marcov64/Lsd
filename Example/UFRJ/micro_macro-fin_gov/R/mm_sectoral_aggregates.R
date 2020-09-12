#******************************************************************
#
# ------------------ MMM Sectoral analysis ----------------------
#
#******************************************************************

#******************************************************************
#
# ------------ Read Monte Carlo experiment files ----------------
#
#******************************************************************

folder   <- "./baseline2"                 # data files folder
baseName <- "Sim_1"                    # data files base name (same as .lsd file)
nExp <- 1                             # number of experiments
iniDrop <- 0                          # initial time steps to drop from analysis (0=none)
nKeep <- -1                           # number of time steps to keep (-1=all)
cores <- 0                            # maximum number of cores to allocate (0=all)
savDat <- TRUE                        # save processed data files and re-use if available?

expVal <- c( "Baseline" )   

aggrVars <- c(  "Real_GDP",     # Real GDP
                "GDP_G",        # Real GDP Growth
                "P_C",          # Consumption Sector Average Price
                "P_K",          # Capital Sector Average Price
                "P_I",          # Intermediate Sector Average Price
                "W_C",          # Consumption Sector Average Wage
                "W_K",          # Capital Sector Average Wage
                "W_I",          # Intermediate Sector Average Wage
                "MK_C",         # Consumption Sector Average Markup
                "MK_K",         # Capital Sector Average Markup
                "MK_I",         # Intermediate Sector Average Markup
                "PROD_C",       # Consumption Sector Average Productivity
                "PROD_K",       # Capital Sector Average Productivity
                "PROD_I",       # Intermediate Sector Average Productivity
                "DEBT_C",       # Consumption Sector Average Debt Rate
                "DEBT_K",       # Capital Sector Average Debt Rate
                "DEBT_I",       # Intermediate Sector Average Debt Rate
                "U_C",          # Consumption Sector Unemployment Rate
                "U_K",          # Capital Sector Unemployment Rate
                "U_I",          # Intermediate Sector Unemployment Rate
                "HHI_C",        # Consumption Sector Inverse HHI
                "HHI_K",        # Capital Sector Inverse HHI
                "HHI_I"        # Intermediate Sector Inverse HHI
            ) 

# Variables to test for stationarity and ergodicity
statErgo.vars <- c( "DEBT_C",       # Consumption Sector Average Debt Rate
                    "DEBT_K",       # Capital Sector Average Debt Rate
                    "DEBT_I",       # Intermediate Sector Average Debt Rate
                    "U_C",          # Consumption Sector Unemployment Rate
                    "U_K",          # Capital Sector Unemployment Rate
                    "U_I",          # Intermediate Sector Unemployment Rate
                    "HHI_C",        # Consumption Sector Inverse HHI
                    "HHI_K",        # Capital Sector Inverse HHI
                    "HHI_I"         # Intermediate Sector Inverse HHI
                  )

# Temporary data file suffix
datFilSfx <- "_sector.Rdata"

# ==== Log start mark ====

cat( "\nMicro-macro sectoral analysis\n=======================\n" )
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
  write.csv( A, paste0( folder, "/", baseName, exper, "_sector_avg.csv" ),
             row.names = FALSE, quote = FALSE )
  write.csv( S, paste0( folder, "/", baseName, exper, "_sector_sd.csv" ),
             row.names = FALSE, quote = FALSE )
  write.csv( M, paste0( folder, "/", baseName, exper, "_sector_max.csv" ),
             row.names = FALSE, quote = FALSE )
  write.csv( m, paste0( folder, "/", baseName, exper, "_sector_min.csv" ),
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

repName   <- "MMM_sector"  # report files base name (if "" same baseName)
sDigits   <- 4      # significant digits in tables
transMk   <- -1     # regime transition mark after warm-up (-1:none)
plotRows  <- 1      # number of plots per row in a page
plotCols  <- 1  	  # number of plots per column in a page
plotW     <- 10     # plot window width
plotH     <- 7      # plot window height
raster    <- TRUE  # raster or vector plots
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
source( "time-plots-sector.R" )
source( "box-plots-sector.R" )

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
    png( paste0( folder, "/", outDir, "/", repName, "_sector_plots_%d.png" ),
         width = plotW, height = plotH, units = "in", res = res )
    TRUE
  } else {
    # Open PDF plot file for output
    pdf( paste0( folder, "/", outDir, "/", repName, "_sector_plots.pdf" ),
         width = plotW, height = plotH )
    par( mfrow = c ( plotRows, plotCols ) )             # define plots per page
    
    png( paste0( folder, "/", outDir, "/", repName, "_sector_plots_%d.png" ),
         width = plotW, height = plotH, units = "in", res = res )
  }
  
  #
  # ====== MC PLOTS GENERATION ======
  #
  
  cat( "\nProcessing experiments and generating reports...\n")
  
  time_plots_sector( mcData, Adata, mdata, Mdata, Sdata, nExp, nSize, nTsteps, TmaskPlot,
              CI, legends, colors, lTypes, transMk, smoothing )
  
  
  box_plots_sector( mcData, nExp, nSize, TmaxStat, TmaskStat, warmUpStat,
             nTstat, legends, legendList, sDigits, bPlotCoef,
             bPlotNotc, folder, outDir, repName )
  
  #
  # ====== STATISTICS GENERATION ======
  #
  
  # Create vectors and lists to hold the Monte Carlo results
  gdp_gr <- p_c_gr <- w_c_gr <- mk_c_gr <- prod_c_gr <- u_c <- debt_c <- hhi_c <- 
            p_k_gr <- w_k_gr <- mk_k_gr <- prod_k_gr <- u_k <- debt_k <- hhi_k <-
            p_i_gr <- w_i_gr <- mk_i_gr <- prod_i_gr <- u_i <- debt_i <- hhi_i <-
            vector( mode = "numeric", length = nSize )
  
  gdp_r_sd <- p_c_sd <- w_c_sd <- mk_c_sd <- prod_c_sd <- u_c_sd <- debt_c_sd <- hhi_c_sd <- 
              p_k_sd <- w_k_sd <- mk_k_sd <- prod_k_sd <- u_k_sd <- debt_k_sd <- hhi_k_sd <-
              p_i_sd <- w_i_sd <- mk_i_sd <- prod_i_sd <- u_i_sd <- debt_i_sd <- hhi_i_sd <-
              vector( mode = 'numeric', length = nSize )
  
  adf_gdp_r <- adf_p_c_gr <- adf_w_c_gr <- adf_mk_c_gr <- adf_prod_c_gr <- adf_u_c <- adf_debt_c <- adf_hhi_c <- 
               adf_p_k_gr <- adf_w_k_gr <- adf_mk_k_gr <- adf_prod_k_gr <- adf_u_k <- adf_debt_k <- adf_hhi_k <-
               adf_p_i_gr <- adf_w_i_gr <- adf_mk_i_gr <- adf_prod_i_gr <- adf_u_i <- adf_debt_i <- adf_hhi_i <-
                list( )
  
  gdp_gdp <- p_c_gdp <- p_k_gdp <- p_i_gdp <- w_c_gdp <- w_k_gdp <- w_i_gdp <- 
    mk_c_gdp <- mk_k_gdp <- mk_i_gdp <- prod_c_gdp <- prod_k_gdp <- prod_i_gdp <- u_c_gdp <- 
    u_k_gdp <- u_i_gdp <- debt_c_gdp <- debt_k_gdp <- debt_i_gdp <- hhi_c_gdp <- hhi_k_gdp <- hhi_i_gdp  <-  list( )
  
  gdp_gdp_pval <- p_c_gdp_pval <- p_k_gdp_pval <- p_i_gdp_pval <- w_c_gdp_pval <- w_k_gdp_pval <- w_i_gdp_pval <- 
    mk_c_gdp_pval <- mk_k_gdp_pval <- mk_i_gdp_pval <- prod_c_gdp_pval <- prod_k_gdp_pval <- prod_i_gdp_pval <- u_c_gdp_pval <- 
    u_k_gdp_pval <- u_i_gdp_pval <- debt_c_gdp_pval <- debt_k_gdp_pval <- debt_i_gdp_pval <- hhi_c_gdp_pval <- hhi_k_gdp_pval <- hhi_i_gdp_pval <-
    vector( mode = "numeric", length = nExp )
  
  for(k in 1 : nExp){ # Experiment k
    
    #
    # ==== Statistics computation for tables ====
    #
    
    for( j in 1 : nSize ){  # Execute for every Monte Carlo run
      
      # Monte carlo average growth rates
      
      gdp_gr[ j ] <- mcData[[ k ]][ nTstat, "GDP_G", j ]
      
      p_c_gr[ j ] <- ( log0( mcData[[ k ]][ nTstat, "P_C", j ] ) - log0( mcData[[ k ]][ warmUpStat + 1, "P_C", j ] ) ) / TmaxStat    
      p_k_gr[ j ] <- ( log0( mcData[[ k ]][ nTstat, "P_K", j ] ) - log0( mcData[[ k ]][ warmUpStat + 1, "P_K", j ] ) ) / TmaxStat
      p_i_gr[ j ] <- ( log0( mcData[[ k ]][ nTstat, "P_I", j ] ) - log0( mcData[[ k ]][ warmUpStat + 1, "P_I", j ] ) ) / TmaxStat
      
      w_c_gr[ j ] <- ( log0( mcData[[ k ]][ nTstat, "W_C", j ] ) - log0( mcData[[ k ]][ warmUpStat + 1, "W_C", j ] ) ) / TmaxStat    
      w_k_gr[ j ] <- ( log0( mcData[[ k ]][ nTstat, "W_K", j ] ) - log0( mcData[[ k ]][ warmUpStat + 1, "W_K", j ] ) ) / TmaxStat
      w_i_gr[ j ] <- ( log0( mcData[[ k ]][ nTstat, "W_I", j ] ) - log0( mcData[[ k ]][ warmUpStat + 1, "W_I", j ] ) ) / TmaxStat
      
      mk_c_gr[ j ] <- ( log0( mcData[[ k ]][ nTstat, "MK_C", j ] ) - log0( mcData[[ k ]][ warmUpStat + 1, "MK_C", j ] ) ) / TmaxStat    
      mk_k_gr[ j ] <- ( log0( mcData[[ k ]][ nTstat, "MK_K", j ] ) - log0( mcData[[ k ]][ warmUpStat + 1, "MK_K", j ] ) ) / TmaxStat
      mk_i_gr[ j ] <- ( log0( mcData[[ k ]][ nTstat, "MK_I", j ] ) - log0( mcData[[ k ]][ warmUpStat + 1, "MK_I", j ] ) ) / TmaxStat
      
      prod_c_gr[ j ] <- ( log0( mcData[[ k ]][ nTstat, "PROD_C", j ] ) - log0( mcData[[ k ]][ warmUpStat + 1, "PROD_C", j ] ) ) / TmaxStat    
      prod_k_gr[ j ] <- ( log0( mcData[[ k ]][ nTstat, "PROD_K", j ] ) - log0( mcData[[ k ]][ warmUpStat + 1, "PROD_K", j ] ) ) / TmaxStat
      prod_i_gr[ j ] <- ( log0( mcData[[ k ]][ nTstat, "PROD_I", j ] ) - log0( mcData[[ k ]][ warmUpStat + 1, "PROD_I", j ] ) ) / TmaxStat
      
      u_c[ j ] <- mcData[[ k ]][ nTstat, "U_C", j ]    
      u_k[ j ] <- mcData[[ k ]][ nTstat, "U_K", j ] 
      u_i[ j ] <- mcData[[ k ]][ nTstat, "U_I", j ] 
      
      debt_c[ j ] <- mcData[[ k ]][ nTstat, "DEBT_C", j ]   
      debt_k[ j ] <- mcData[[ k ]][ nTstat, "DEBT_K", j ]  
      debt_i[ j ] <- mcData[[ k ]][ nTstat, "DEBT_I", j ] 
      
      hhi_c[ j ] <- mcData[[ k ]][ nTstat, "HHI_C", j ]    
      hhi_k[ j ] <- mcData[[ k ]][ nTstat, "HHI_K", j ]  
      hhi_i[ j ] <- mcData[[ k ]][ nTstat, "HHI_I", j ] 
      
      
      # Apply Baxter-King filter to the series
      
      gdp_bpf     <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "Real_GDP", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      
      p_c_bpf     <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "P_C", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      p_k_bpf     <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "P_K", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      p_i_bpf     <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "P_I", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      
      w_c_bpf     <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "W_C", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      w_k_bpf     <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "W_K", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      w_i_bpf     <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "W_I", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      
      mk_c_bpf    <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "MK_C", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      mk_k_bpf    <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "MK_K", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      mk_i_bpf    <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "MK_I", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      
      prod_c_bpf  <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "PROD_C", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      prod_k_bpf  <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "PROD_K", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      prod_i_bpf  <- bkfilter( log0( mcData[[ k ]][ TmaskStat, "PROD_I", j ] ), pl = lowP, pu = highP, nfix = bpfK )
      
      debt_c_bpf  <- bkfilter( mcData[[ k ]][ TmaskStat, "DEBT_C", j ] , pl = lowP, pu = highP, nfix = bpfK )
      debt_k_bpf  <- bkfilter( mcData[[ k ]][ TmaskStat, "DEBT_K", j ] , pl = lowP, pu = highP, nfix = bpfK )
      debt_i_bpf  <- bkfilter( mcData[[ k ]][ TmaskStat, "DEBT_I", j ] , pl = lowP, pu = highP, nfix = bpfK )
      
      u_c_bpf     <- bkfilter( mcData[[ k ]][ TmaskStat, "U_C", j ] , pl = lowP, pu = highP, nfix = bpfK )
      u_k_bpf     <- bkfilter( mcData[[ k ]][ TmaskStat, "U_K", j ] , pl = lowP, pu = highP, nfix = bpfK )
      u_i_bpf     <- bkfilter( mcData[[ k ]][ TmaskStat, "U_I", j ] , pl = lowP, pu = highP, nfix = bpfK )
      
      hhi_c_bpf   <- bkfilter( mcData[[ k ]][ TmaskStat, "HHI_C", j ] , pl = lowP, pu = highP, nfix = bpfK )
      hhi_k_bpf   <- bkfilter( mcData[[ k ]][ TmaskStat, "HHI_K", j ] , pl = lowP, pu = highP, nfix = bpfK )
      hhi_i_bpf   <- bkfilter( mcData[[ k ]][ TmaskStat, "HHI_I", j ] , pl = lowP, pu = highP, nfix = bpfK )
      
      # Augmented Dickey-Fuller tests for unit roots
      
      adf_gdp_r[[ j ]]    <- adf.test( log0( mcData[[ k ]][ TmaskStat, "Real_GDP", j ] ) )
      
      adf_p_c_gr[[ j ]]    <- adf.test( ( log0( mcData[[ k ]][ nTstat, "P_C", j ] ) - log0( mcData[[ k ]][ warmUpStat + 1, "P_C", j ] ) ) / TmaxStat  )
      adf_p_k_gr[[ j ]]    <- adf.test( ( log0( mcData[[ k ]][ nTstat, "P_K", j ] ) - log0( mcData[[ k ]][ warmUpStat + 1, "P_K", j ] ) ) / TmaxStat  )
      adf_p_i_gr[[ j ]]    <- adf.test( ( log0( mcData[[ k ]][ nTstat, "P_I", j ] ) - log0( mcData[[ k ]][ warmUpStat + 1, "P_I", j ] ) ) / TmaxStat  )
      
      adf_w_c_gr[[ j ]]    <- adf.test( ( log0( mcData[[ k ]][ nTstat, "W_C", j ] ) - log0( mcData[[ k ]][ warmUpStat + 1, "W_C", j ] ) ) / TmaxStat  )
      adf_w_k_gr[[ j ]]    <- adf.test( ( log0( mcData[[ k ]][ nTstat, "W_K", j ] ) - log0( mcData[[ k ]][ warmUpStat + 1, "W_K", j ] ) ) / TmaxStat  )
      adf_w_i_gr[[ j ]]    <- adf.test( ( log0( mcData[[ k ]][ nTstat, "W_I", j ] ) - log0( mcData[[ k ]][ warmUpStat + 1, "W_I", j ] ) ) / TmaxStat  )
      
      adf_mk_c_gr[[ j ]]    <- adf.test( ( log0( mcData[[ k ]][ nTstat, "MK_C", j ] ) - log0( mcData[[ k ]][ warmUpStat + 1, "MK_C", j ] ) ) / TmaxStat  )
      adf_mk_k_gr[[ j ]]    <- adf.test( ( log0( mcData[[ k ]][ nTstat, "MK_K", j ] ) - log0( mcData[[ k ]][ warmUpStat + 1, "MK_K", j ] ) ) / TmaxStat  )
      adf_mk_i_gr[[ j ]]    <- adf.test( ( log0( mcData[[ k ]][ nTstat, "MK_I", j ] ) - log0( mcData[[ k ]][ warmUpStat + 1, "MK_I", j ] ) ) / TmaxStat  )
      
      adf_prod_c_gr[[ j ]]  <- adf.test( ( log0( mcData[[ k ]][ nTstat, "PROD_C", j ] ) - log0( mcData[[ k ]][ warmUpStat + 1, "PROD_C", j ] ) ) / TmaxStat  )
      adf_prod_k_gr[[ j ]]  <- adf.test( ( log0( mcData[[ k ]][ nTstat, "PROD_K", j ] ) - log0( mcData[[ k ]][ warmUpStat + 1, "PROD_K", j ] ) ) / TmaxStat  )
      adf_prod_i_gr[[ j ]]  <- adf.test( ( log0( mcData[[ k ]][ nTstat, "PROD_I", j ] ) - log0( mcData[[ k ]][ warmUpStat + 1, "PROD_I", j ] ) ) / TmaxStat  )
      
      adf_u_c[[ j ]]        <- adf.test( mcData[[ k ]][ TmaskStat, "U_C", j ] ) 
      adf_u_k[[ j ]]        <- adf.test( mcData[[ k ]][ TmaskStat, "U_K", j ] )
      adf_u_i[[ j ]]        <- adf.test( mcData[[ k ]][ TmaskStat, "U_I", j ] )
      
      adf_debt_c[[ j ]]     <- adf.test( mcData[[ k ]][ TmaskStat, "DEBT_C", j ] ) 
      adf_debt_k[[ j ]]     <- adf.test( mcData[[ k ]][ TmaskStat, "DEBT_K", j ] )
      adf_debt_i[[ j ]]     <- adf.test( mcData[[ k ]][ TmaskStat, "DEBT_I", j ] )
      
      adf_hhi_c[[ j ]]      <- adf.test( mcData[[ k ]][ TmaskStat, "HHI_C", j ] ) 
      adf_hhi_k[[ j ]]      <- adf.test( mcData[[ k ]][ TmaskStat, "HHI_K", j ] )
      adf_hhi_i[[ j ]]      <- adf.test( mcData[[ k ]][ TmaskStat, "HHI_I", j ] )
      
      
      # Standard deviations of filtered series
      
      gdp_r_sd[ j ]     <- sd( gdp_bpf$cycle[ TmaskBpf, 1 ] )
      
      p_c_sd[ j ]       <- sd( p_c_bpf$cycle[ TmaskBpf, 1 ] )
      p_k_sd[ j ]       <- sd( p_k_bpf$cycle[ TmaskBpf, 1 ] )
      p_i_sd[ j ]       <- sd( p_i_bpf$cycle[ TmaskBpf, 1 ] )
      
      w_c_sd[ j ]       <- sd( w_c_bpf$cycle[ TmaskBpf, 1 ] )
      w_k_sd[ j ]       <- sd( w_k_bpf$cycle[ TmaskBpf, 1 ] )
      w_i_sd[ j ]       <- sd( w_i_bpf$cycle[ TmaskBpf, 1 ] )
      
      mk_c_sd[ j ]      <- sd( mk_c_bpf$cycle[ TmaskBpf, 1 ] )
      mk_k_sd[ j ]      <- sd( mk_k_bpf$cycle[ TmaskBpf, 1 ] )
      mk_i_sd[ j ]      <- sd( mk_i_bpf$cycle[ TmaskBpf, 1 ] )
      
      prod_c_sd[ j ]    <- sd( prod_c_bpf$cycle[ TmaskBpf, 1 ] )
      prod_k_sd[ j ]    <- sd( prod_k_bpf$cycle[ TmaskBpf, 1 ] )
      prod_i_sd[ j ]    <- sd( prod_i_bpf$cycle[ TmaskBpf, 1 ] )
      
      u_c_sd[ j ]       <- sd( u_c_bpf$cycle[ TmaskBpf, 1 ] )
      u_k_sd[ j ]       <- sd( u_k_bpf$cycle[ TmaskBpf, 1 ] )
      u_i_sd[ j ]       <- sd( u_i_bpf$cycle[ TmaskBpf, 1 ] )
      
      debt_c_sd[ j ]    <- sd( debt_c_bpf$cycle[ TmaskBpf, 1 ] )
      debt_k_sd[ j ]    <- sd( debt_k_bpf$cycle[ TmaskBpf, 1 ] )
      debt_i_sd[ j ]    <- sd( debt_i_bpf$cycle[ TmaskBpf, 1 ] )
      
      hhi_c_sd[ j ]     <- sd( hhi_c_bpf$cycle[ TmaskBpf, 1 ] )
      hhi_k_sd[ j ]     <- sd( hhi_k_bpf$cycle[ TmaskBpf, 1 ] )
      hhi_i_sd[ j ]     <- sd( hhi_i_bpf$cycle[ TmaskBpf, 1 ] )
      
      # Build the correlation structures
      
      gdp_gdp[[ j ]]  <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ],
                              gdp_bpf$cycle[ TmaskBpf, 1 ],
                              lag.max = lags, plot = FALSE, na.action = na.pass )
      p_c_gdp[[ j ]] <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ],
                            p_c_bpf$cycle[ TmaskBpf, 1 ],
                            lag.max = lags, plot = FALSE, na.action = na.pass )
      p_k_gdp[[ j ]] <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ],
                            p_k_bpf$cycle[ TmaskBpf, 1 ],
                            lag.max = lags, plot = FALSE, na.action = na.pass )
      p_i_gdp[[ j ]] <- ccf(gdp_bpf$cycle[ TmaskBpf, 1 ],
                            p_i_bpf$cycle[ TmaskBpf, 1 ],
                            lag.max = lags, plot = FALSE, na.action = na.pass )
      w_c_gdp[[ j ]] <- ccf(gdp_bpf$cycle[ TmaskBpf, 1 ],
                            w_c_bpf$cycle[ TmaskBpf, 1 ],
                            lag.max = lags, plot = FALSE, na.action = na.pass )
      w_k_gdp[[ j ]] <- ccf(gdp_bpf$cycle[ TmaskBpf, 1 ],
                          w_k_bpf$cycle[ TmaskBpf, 1 ],
                          lag.max = lags, plot = FALSE, na.action = na.pass )
      w_i_gdp[[ j ]] <- ccf(gdp_bpf$cycle[ TmaskBpf, 1 ],
                           w_i_bpf$cycle[ TmaskBpf, 1 ],
                           lag.max = lags, plot = FALSE, na.action = na.pass )
      mk_c_gdp[[ j ]] <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ],
                           mk_c_bpf$cycle[ TmaskBpf, 1 ],
                           lag.max = lags, plot = FALSE, na.action = na.pass )
      mk_k_gdp[[ j ]] <- ccf(gdp_bpf$cycle[ TmaskBpf, 1 ],
                                mk_k_bpf$cycle[ TmaskBpf, 1 ],
                                lag.max = lags, plot = FALSE, na.action = na.pass )
      mk_i_gdp[[ j ]]    <- ccf(gdp_bpf$cycle[ TmaskBpf, 1 ],
                                mk_i_bpf$cycle[ TmaskBpf, 1 ],
                                lag.max = lags, plot = FALSE, na.action = na.pass )
      prod_c_gdp[[ j ]] <- ccf(gdp_bpf$cycle[ TmaskBpf, 1 ],
                              prod_c_bpf$cycle[ TmaskBpf, 1 ],
                              lag.max = lags, plot = FALSE, na.action = na.pass )
      prod_k_gdp[[ j ]] <- ccf(gdp_bpf$cycle[ TmaskBpf, 1 ],
                              prod_k_bpf$cycle[ TmaskBpf, 1 ],
                              lag.max = lags, plot = FALSE, na.action = na.pass )
      prod_i_gdp[[ j ]] <- ccf(gdp_bpf$cycle[ TmaskBpf, 1 ],
                             prod_i_bpf$cycle[ TmaskBpf, 1 ],
                             lag.max = lags, plot = FALSE, na.action = na.pass )
      u_c_gdp[[ j ]] <- ccf(gdp_bpf$cycle[ TmaskBpf, 1 ],
                              u_c_bpf$cycle[ TmaskBpf, 1 ],
                              lag.max = lags, plot = FALSE, na.action = na.pass )
      u_k_gdp[[ j ]] <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ],
                              u_k_bpf$cycle[ TmaskBpf, 1 ],
                              lag.max = lags, plot = FALSE, na.action = na.pass )
      u_i_gdp[[ j ]] <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ],
                           u_i_bpf$cycle[ TmaskBpf, 1 ],
                           lag.max = lags, plot = FALSE, na.action = na.pass )
      debt_c_gdp[[ j ]] <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ],
                             debt_c_bpf$cycle[ TmaskBpf, 1 ],
                             lag.max = lags, plot = FALSE, na.action = na.pass )
      debt_k_gdp[[ j ]] <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ],
                              debt_k_bpf$cycle[ TmaskBpf, 1 ],
                              lag.max = lags, plot = FALSE, na.action = na.pass )
      debt_i_gdp[[ j ]] <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ],
                           debt_i_bpf$cycle[ TmaskBpf, 1 ],
                           lag.max = lags, plot = FALSE, na.action = na.pass )
      hhi_c_gdp[[ j ]] <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ],
                            hhi_c_bpf$cycle[ TmaskBpf, 1 ],
                            lag.max = lags, plot = FALSE, na.action = na.pass )
      hhi_k_gdp[[ j ]] <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ],
                               hhi_k_bpf$cycle[ TmaskBpf, 1 ],
                               lag.max = lags, plot = FALSE, na.action = na.pass )
      hhi_i_gdp[[ j ]] <- ccf( gdp_bpf$cycle[ TmaskBpf, 1 ],
                               hhi_i_bpf$cycle[ TmaskBpf, 1 ],
                               lag.max = lags, plot = FALSE, na.action = na.pass )
      
    }
    
    # Applies t test to the mean lag results to test their significance (H0: lag < critCorr)
    for(i in 1 : (2 * lags + 1) ){ #do for all lags
      if(i != lags + 1)  # don't try to compute autocorrelation at lag 0
      gdp_gdp_pval[ i ]   <- t.test0( abs( unname( sapply( gdp_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      p_c_gdp_pval[ i ]   <- t.test0( abs( unname( sapply( p_c_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      p_k_gdp_pval[ i ]   <- t.test0( abs( unname( sapply( p_k_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      p_i_gdp_pval[ i ]   <- t.test0( abs( unname( sapply( p_i_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      w_c_gdp_pval[ i ]   <- t.test0( abs( unname( sapply( w_c_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      w_k_gdp_pval[ i ]   <- t.test0( abs( unname( sapply( w_k_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      w_i_gdp_pval[ i ]   <- t.test0( abs( unname( sapply( w_i_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      mk_c_gdp_pval[ i ]  <- t.test0( abs( unname( sapply( mk_c_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      mk_k_gdp_pval[ i ]  <- t.test0( abs( unname( sapply( mk_k_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      mk_i_gdp_pval[ i ]  <- t.test0( abs( unname( sapply( mk_i_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      prod_c_gdp_pval[ i ]<- t.test0( abs( unname( sapply( prod_c_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      prod_k_gdp_pval[ i ]<- t.test0( abs( unname( sapply( prod_k_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      prod_i_gdp_pval[ i ]<- t.test0( abs( unname( sapply( prod_i_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      u_c_gdp_pval[ i ]   <- t.test0( abs( unname( sapply( u_c_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      u_k_gdp_pval[ i ]   <- t.test0( abs( unname( sapply( u_k_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      u_i_gdp_pval[ i ]   <- t.test0( abs( unname( sapply( u_i_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      debt_c_gdp_pval[ i ]<- t.test0( abs( unname( sapply( debt_c_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      debt_k_gdp_pval[ i ]<- t.test0( abs( unname( sapply( debt_k_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      debt_i_gdp_pval[ i ]<- t.test0( abs( unname( sapply( debt_i_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      hhi_c_gdp_pval[ i ] <- t.test0( abs( unname( sapply( hhi_c_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      hhi_k_gdp_pval[ i ] <- t.test0( abs( unname( sapply( hhi_k_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
      hhi_i_gdp_pval[ i ] <- t.test0( abs( unname( sapply( hhi_i_gdp, `[[`, "acf" ) )[ i, ] ), critCorr, CI )
    }
    
    #
    # ---- Summary statistics table (averages, standard errors and p-values) ----
    #
    
    key.stats.1 <- matrix(
      c(
        ## avg. growth rate
        mean( p_c_gr ), 
        mean( w_c_gr ), 
        mean( mk_c_gr ), 
        mean( prod_c_gr ), 
        mean( u_c ),
        mean( debt_c ), 
        mean( hhi_c ), 
        
        ## (s.e.)
        sd( p_c_gr ) / sqrt( nSize ), 
        sd( w_c_gr ) / sqrt( nSize ),
        sd( mk_c_gr ) / sqrt( nSize ), 
        sd( prod_c_gr ) / sqrt( nSize ),
        sd( u_c ) / sqrt( nSize ),
        sd( debt_c ) / sqrt( nSize ),
        sd( hhi_c  ) / sqrt( nSize ),
        
        ## ADF test (logs)
        mean( unname( sapply( adf_p_c_gr, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_w_c_gr, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_mk_c_gr, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_prod_c_gr, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_u_c, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_debt_c, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_hhi_c, `[[`, "statistic" ) ) ),
        
        ## ADF test (logs) s.e.
        sd( unname( sapply( adf_p_c_gr, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_w_c_gr, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_mk_c_gr, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_prod_c_gr, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_u_c, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_debt_c, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_hhi_c, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        
        ## ADF test (logs) p.value
        mean( unname( sapply( adf_p_c_gr, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_w_c_gr, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_mk_c_gr, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_prod_c_gr, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_u_c, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_debt_c, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_hhi_c, `[[`, "p.value" ) ) ),
        
        ## ADF test (logs) p.value s.e.
        sd( unname( sapply( adf_p_c_gr, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_w_c_gr, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_mk_c_gr, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_prod_c_gr, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_u_c, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_debt_c, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_hhi_c, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        
        ## S.d. of bpf series
        mean( p_c_sd ), 
        mean( w_c_sd ), 
        mean( mk_c_sd ),
        mean( prod_c_sd ), 
        mean( u_c_sd ), 
        mean( debt_c_sd ), 
        mean( hhi_c_sd ), 
        
        ## S.e. of bpf series s.d.
        se( p_c_sd ) / sqrt( nSize ), 
        se( w_c_sd ) / sqrt( nSize ), 
        se( mk_c_sd ) / sqrt( nSize ), 
        se( prod_c_sd ) / sqrt( nSize ), 
        se( u_c_sd ) / sqrt( nSize ), 
        se( debt_c_sd ) / sqrt( nSize ), 
        se( hhi_c_sd ) / sqrt( nSize ),
        
        ## relative s.d. (to GDP)
        mean( p_c_sd ) / mean( gdp_r_sd ), 
        mean( w_c_sd ) / mean( gdp_r_sd ),
        mean( mk_c_sd ) / mean( gdp_r_sd ), 
        mean( prod_c_sd ) / mean( gdp_r_sd ),
        mean( u_c_sd ) / mean( gdp_r_sd ),
        mean( debt_c_sd ) / mean(gdp_r_sd ),
        mean( hhi_c_sd ) / mean( gdp_r_sd )
        
      ),
      ncol = 7, byrow = T)
    
    colnames( key.stats.1 ) <- c( "Avg. Price", 
                                  "Avg. Wage", 
                                  "Avg. Mark-up", 
                                  "Avg. Productivity",
                                  "Unemployment",
                                  "Avg. Debt Rate",
                                  "Inverse HHI"
    )
    rownames( key.stats.1 ) <- c( "avg. rate", 
                                  " (s.e.)",
                                  "ADF test (logs)",
                                  " (s.e.)", 
                                  " (p-val.)", 
                                  " (s.e.)",
                                  " s.d. (bpf)", 
                                  " (s.e.)",
                                  " relative s.d. (GDP)" )
    
    textplot( formatC( key.stats.1, digits = sDigits, format = "g" ), cmar = 2 )
    title <- paste( "Key statistics and unit roots tests - Consumption Sector (", legends[ k ], ")" )
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
        mean( p_k_gr ), 
        mean( w_k_gr ), 
        mean( mk_k_gr ), 
        mean( prod_k_gr ), 
        mean( u_k ),
        mean( debt_k ), 
        mean( hhi_k ), 
        
        ## (s.e.)
        sd( p_k_gr ) / sqrt( nSize ), 
        sd( w_k_gr ) / sqrt( nSize ),
        sd( mk_k_gr ) / sqrt( nSize ), 
        sd( prod_k_gr ) / sqrt( nSize ),
        sd( u_k ) / sqrt( nSize ),
        sd( debt_k ) / sqrt( nSize ),
        sd( hhi_k  ) / sqrt( nSize ),
        
        ## ADF test (logs)
        mean( unname( sapply( adf_p_k_gr, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_w_k_gr, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_mk_k_gr, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_prod_k_gr, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_u_k, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_debt_k, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_hhi_k, `[[`, "statistic" ) ) ),
        
        ## ADF test (logs) s.e.
        sd( unname( sapply( adf_p_k_gr, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_w_k_gr, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_mk_k_gr, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_prod_k_gr, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_u_k, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_debt_k, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_hhi_k, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        
        ## ADF test (logs) p.value
        mean( unname( sapply( adf_p_k_gr, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_w_k_gr, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_mk_k_gr, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_prod_k_gr, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_u_k, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_debt_k, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_hhi_k, `[[`, "p.value" ) ) ),
        
        ## ADF test (logs) p.value s.e.
        sd( unname( sapply( adf_p_k_gr, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_w_k_gr, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_mk_k_gr, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_prod_k_gr, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_u_k, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_debt_k, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_hhi_k, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        
        ## S.d. of bpf series
        mean( p_k_sd ), 
        mean( w_k_sd ), 
        mean( mk_k_sd ),
        mean( prod_k_sd ), 
        mean( u_k_sd ), 
        mean( debt_k_sd ), 
        mean( hhi_k_sd ), 
        
        ## S.e. of bpf series s.d.
        se( p_k_sd ) / sqrt( nSize ), 
        se( w_k_sd ) / sqrt( nSize ), 
        se( mk_k_sd ) / sqrt( nSize ), 
        se( prod_k_sd ) / sqrt( nSize ), 
        se( u_k_sd ) / sqrt( nSize ), 
        se( debt_k_sd ) / sqrt( nSize ), 
        se( hhi_k_sd ) / sqrt( nSize ),
        
        ## relative s.d. (to GDP)
        mean( p_k_sd ) / mean( gdp_r_sd ), 
        mean( w_k_sd ) / mean( gdp_r_sd ),
        mean( mk_k_sd ) / mean( gdp_r_sd ), 
        mean( prod_k_sd ) / mean( gdp_r_sd ),
        mean( u_k_sd ) / mean( gdp_r_sd ),
        mean( debt_k_sd ) / mean(gdp_r_sd ),
        mean( hhi_k_sd ) / mean( gdp_r_sd )
        
      ),
      ncol = 7, byrow = T)
    
    colnames( key.stats.2 ) <- c( "Avg. Price", 
                                  "Avg. Wage", 
                                  "Avg. Mark-up", 
                                  "Avg. Productivity",
                                  "Unemployment",
                                  "Avg. Debt Rate",
                                  "Inverse HHI"
    )
    rownames( key.stats.2 ) <- c( "avg. rate", 
                                  " (s.e.)",
                                  "ADF test (logs)",
                                  " (s.e.)", 
                                  " (p-val.)", 
                                  " (s.e.)",
                                  " s.d. (bpf)", 
                                  " (s.e.)",
                                  " relative s.d. (GDP)" )
    
    textplot( formatC( key.stats.2, digits = sDigits, format = "g" ), cmar = 2 )
    title <- paste( "Key statistics and unit roots tests - Capital Sector (", legends[ k ], ")" )
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
        mean( p_i_gr ), 
        mean( w_i_gr ), 
        mean( mk_i_gr ), 
        mean( prod_i_gr ), 
        mean( u_i ),
        mean( debt_i ), 
        mean( hhi_i ), 
        
        ## (s.e.)
        sd( p_i_gr ) / sqrt( nSize ), 
        sd( w_i_gr ) / sqrt( nSize ),
        sd( mk_i_gr ) / sqrt( nSize ), 
        sd( prod_i_gr ) / sqrt( nSize ),
        sd( u_i ) / sqrt( nSize ),
        sd( debt_i ) / sqrt( nSize ),
        sd( hhi_i  ) / sqrt( nSize ),
        
        ## ADF test (logs)
        mean( unname( sapply( adf_p_i_gr, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_w_i_gr, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_mk_i_gr, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_prod_i_gr, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_u_i, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_debt_i, `[[`, "statistic" ) ) ),
        mean( unname( sapply( adf_hhi_i, `[[`, "statistic" ) ) ),
        
        ## ADF test (logs) s.e.
        sd( unname( sapply( adf_p_i_gr, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_w_i_gr, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_mk_i_gr, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_prod_i_gr, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_u_i, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_debt_i, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_hhi_i, `[[`, "statistic" ) ) ) / sqrt( nSize ),
        
        ## ADF test (logs) p.value
        mean( unname( sapply( adf_p_i_gr, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_w_i_gr, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_mk_i_gr, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_prod_i_gr, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_u_i, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_debt_i, `[[`, "p.value" ) ) ),
        mean( unname( sapply( adf_hhi_i, `[[`, "p.value" ) ) ),
        
        ## ADF test (logs) p.value s.e.
        sd( unname( sapply( adf_p_i_gr, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_w_i_gr, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_mk_i_gr, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_prod_i_gr, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_u_i, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_debt_i, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        sd( unname( sapply( adf_hhi_i, `[[`, "p.value" ) ) ) / sqrt( nSize ),
        
        ## S.d. of bpf series
        mean( p_i_sd ), 
        mean( w_i_sd ), 
        mean( mk_i_sd ),
        mean( prod_i_sd ), 
        mean( u_i_sd ), 
        mean( debt_i_sd ), 
        mean( hhi_i_sd ), 
        
        ## S.e. of bpf series s.d.
        se( p_i_sd ) / sqrt( nSize ), 
        se( w_i_sd ) / sqrt( nSize ), 
        se( mk_i_sd ) / sqrt( nSize ), 
        se( prod_i_sd ) / sqrt( nSize ), 
        se( u_i_sd ) / sqrt( nSize ), 
        se( debt_i_sd ) / sqrt( nSize ), 
        se( hhi_i_sd ) / sqrt( nSize ),
        
        ## relative s.d. (to GDP)
        mean( p_i_sd ) / mean( gdp_r_sd ), 
        mean( w_i_sd ) / mean( gdp_r_sd ),
        mean( mk_i_sd ) / mean( gdp_r_sd ), 
        mean( prod_i_sd ) / mean( gdp_r_sd ),
        mean( u_i_sd ) / mean( gdp_r_sd ),
        mean( debt_i_sd ) / mean(gdp_r_sd ),
        mean( hhi_i_sd ) / mean( gdp_r_sd )
        
      ),
      ncol = 7, byrow = T)
    
    colnames( key.stats.3 ) <- c( "Avg. Price", 
                                  "Avg. Wage", 
                                  "Avg. Mark-up", 
                                  "Avg. Productivity",
                                  "Unemployment",
                                  "Avg. Debt Rate",
                                  "Inverse HHI"
    )
    rownames( key.stats.3 ) <- c( "avg. rate", 
                                  " (s.e.)",
                                  "ADF test (logs)",
                                  " (s.e.)", 
                                  " (p-val.)", 
                                  " (s.e.)",
                                  " s.d. (bpf)", 
                                  " (s.e.)",
                                  " relative s.d. (GDP)" )
    
    textplot( formatC( key.stats.3, digits = sDigits, format = "g" ), cmar = 2 )
    title <- paste( "Key statistics and unit roots tests - Intermediate Sector (", legends[ k ], ")" )
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
    corr.struct.1 <- matrix(c(colMeans(t( unname( sapply(p_c_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(p_c_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              p_c_gdp_pval,
                              
                              colMeans(t( unname( sapply(w_c_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(w_c_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              w_c_gdp_pval,
                              
                              colMeans(t( unname( sapply(mk_c_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(mk_c_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              mk_c_gdp_pval,
                              
                              colMeans(t( unname( sapply(prod_c_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(prod_c_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              prod_c_gdp_pval,
                              
                              colMeans(t( unname( sapply(u_c_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(u_c_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              u_c_gdp_pval,
                              
                              colMeans(t( unname( sapply(debt_c_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(debt_c_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              debt_c_gdp_pval,
                              
                              colMeans(t( unname( sapply(hhi_c_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(hhi_c_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              hhi_c_gdp_pval
    ),
    
    ncol = 2 * lags + 1, byrow = T)
    colnames( corr.struct.1 ) <- gdp_gdp[[1]]$lag
    rownames( corr.struct.1 ) <- c( "Avg. Price", " (s.e.)", " (p-val.)",
                                    "Avg. Wage", " (s.e.)", " (p-val.)",
                                    "Avg. Markup", " (s.e.)", " (p-val.)",
                                    "Avg. Productivity", " (s.e.)", " (p-val.)",
                                    "Unemployment", " (s.e.)", " (p-val.)",
                                    "Avg. Debt Rate", " (s.e.)", " (p-val.)",
                                    "Inverse HHI", " (s.e.)", " (p-val.)"
    )
    
    corr.struct.2 <- matrix(c(colMeans(t( unname( sapply(p_k_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(p_k_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              p_k_gdp_pval,
                              
                              colMeans(t( unname( sapply(w_k_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(w_k_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              w_k_gdp_pval,
                              
                              colMeans(t( unname( sapply(mk_k_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(mk_k_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              mk_k_gdp_pval,
                              
                              colMeans(t( unname( sapply(prod_k_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(prod_k_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              prod_k_gdp_pval,
                              
                              colMeans(t( unname( sapply(u_k_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(u_k_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              u_k_gdp_pval,
                              
                              colMeans(t( unname( sapply(debt_k_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(debt_k_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              debt_k_gdp_pval,
                              
                              colMeans(t( unname( sapply(hhi_k_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(hhi_k_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              jji_k_gdp_pval
                              
    ),
    
    ncol = 2 * lags + 1, byrow = T)
    colnames( corr.struct.2 ) <- gdp_gdp[[1]]$lag
    rownames( corr.struct.2 ) <- c( "Avg. Price", " (s.e.)", " (p-val.)",
                                    "Avg. Wage", " (s.e.)", " (p-val.)",
                                    "Avg. Markup", " (s.e.)", " (p-val.)",
                                    "Avg. Productivity", " (s.e.)", " (p-val.)",
                                    "Unemployment", " (s.e.)", " (p-val.)",
                                    "Avg. Debt Rate", " (s.e.)", " (p-val.)",
                                    "Inverse HHI", " (s.e.)", " (p-val.)"
    )
    
    title <- paste( "Correlation structure for GDP (", legends[ k ], ")" )
    subTitle <- paste( eval( bquote( paste0( "( non-rate/ratio series are Baxter-King bandpass-filtered, low = ",
                                             .( lowP ), "Q / high = ", .( highP ), "Q / order = ", .( bpfK ),
                                             " / MC runs = ", .( nSize ), " / period = ",
                                             .( warmUpStat + 1 ), " - ", .( nTstat ), " )" ) ) ),
                       eval( bquote ( paste0( "( test H0: lag coefficient is not significant at ",
                                              .( ( 1 - CI ) * 100), "% level", " )" ) ) ), sep ="\n" )
    
    
    corr.struct.3 <- matrix(c(colMeans(t( unname( sapply(p_i_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(p_i_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              p_i_gdp_pval,
                              
                              colMeans(t( unname( sapply(w_i_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(w_i_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              w_i_gdp_pval,
                              
                              colMeans(t( unname( sapply(mk_i_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(mk_i_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              mk_i_gdp_pval,
                              
                              colMeans(t( unname( sapply(prod_i_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(prod_i_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              prod_i_gdp_pval,
                              
                              colMeans(t( unname( sapply(u_i_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(u_i_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              u_i_gdp_pval,
                              
                              colMeans(t( unname( sapply(debt_i_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(debt_i_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              debt_i_gdp_pval,
                              
                              colMeans(t( unname( sapply(hhi_i_gdp, `[[`, "acf" ) ) ), na.rm = T),
                              colSds(t( unname( sapply(hhi_i_gdp, `[[`, "acf" ) ) ), na.rm = T) / sqrt( nSize ),
                              jji_i_gdp_pval
    ),
    
    ncol = 2 * lags + 1, byrow = T)
    colnames( corr.struct.3 ) <- gdp_gdp[[1]]$lag
    rownames( corr.struct.3 ) <- c( "Avg. Price", " (s.e.)", " (p-val.)",
                                    "Avg. Wage", " (s.e.)", " (p-val.)",
                                    "Avg. Markup", " (s.e.)", " (p-val.)",
                                    "Avg. Productivity", " (s.e.)", " (p-val.)",
                                    "Unemployment", " (s.e.)", " (p-val.)",
                                    "Avg. Debt Rate", " (s.e.)", " (p-val.)",
                                    "Inverse HHI", " (s.e.)", " (p-val.)"
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
      
    }
    # write to disk
    write.csv( statErgo, quote = FALSE,
               paste0( folder, "/", outDir, "/", repName, k, "sector_ergod_tests.csv" ) )
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



  
  


