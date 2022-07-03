#******************************************************************
#
# ------------ Industry Model: size growth analysis --------------
#
#   Written by Marcelo C. Pereira, University of Campinas
#
#   Copyright Marcelo C. Pereira
#   Distributed under the GNU General Public License
#
#   IMPORTANT: this script assumes the R working directory is set
#   to the R subfolder where this script file is stored. This can
#   be done automatically in RStudio if a project is created at
#   this subfolder, or using the command setwd(). The "folder"
#   variable below must always point to a (relative) subfolder
#   of the R working directory.
#
#******************************************************************

# ==== User defined parameters ====

folder   <- "MarkII/Beta"       # subfolder of working dir containing data
baseName <- "MarkII-Beta"       # data files base name
poolData <- FALSE               # pool all data files (T) or treat separately (F)
plotQQ <- FALSE                 # plot quartile to quartile fit graphs or not
iniDrop <- 0                    # initial time steps to drop from analysis (0=none)
plotRows <- 1					          # number of plots per row in a page
plotCols <- 1					          # number of plots per column in a page
plotW <- 10                     # plot window width
plotH <- 7                      # plot window height

chartTitle <- "Pooled market share growth rate distribution"
xAxisLabel <- "Size growth rate"
yAxisLabel <- "Binned density (log scale)"

caseNames <- c( )               # enter custom cases names here


# ====== External support functions & definitions ======

source( "StatFuncs.R" )


# ==== MAIN SCRIPT (data processing starts here) ====

# ---- Read data files ----

readFiles <- list.files( path = folder, pattern = paste0( baseName, "_[0-9]+.res" ),
                         full.names = TRUE )

growth_mkt <- read.list.lsd( readFiles, "_growth", skip = iniDrop,
                             instance = 0, pool = poolData )

numCases <- length( growth_mkt )

# ---- Verify an/or create labels for each case (for plots) ----

numNames <- length( caseNames )

if( numNames < numCases )
  for( i in ( numNames + 1 ) : numCases )
    caseNames[i] <- paste( "Run", i )

# ---- Order data, remove outliers and generate some statistics ----

dataSeries <- list( )

for( i in 1: numCases ){
  obs <- nrow( growth_mkt[[i]] )                # number of observations in i

  dataSeries[[i]] <- sort( as.vector( growth_mkt[[i]][!is.na( growth_mkt[[i]] )] ) )  # ascending order
  iniLen <- length( dataSeries[[i]] )
  dataSeries[[i]] <- dataSeries[[i]][abs( dataSeries[[i]] ) < outLim]  # remove outliers (inserted as artifacts by LSD)
  finLen <- length( dataSeries[[i]] )
}

# ---- Enter error handling mode so PDF can be closed in case of error/interruption ----

tryCatch({

  # ---- Open PDF plot file for output ----

  pdf( paste0( folder, "/", baseName, "_SizeGrowth.pdf" ),
       width = plotW, height = plotH )
  options( scipen = 5 )                 # max 5 digits
  par( mfrow = c ( plotRows, plotCols ) )             # define plots per page

  # ---- Fit data to all used distributions ----

  normFit <- lapFit <- aLapFit <- subboFit <- aSubboFit <- list( )
  fileData <- paste0( folder, "/", "data_sg_", baseName, ".csv" )
  fileFit <- paste0( folder, "/", "subbofit_sg_", baseName, ".csv" )
  notFirst <- FALSE

  for( i in 1 : numCases ){
    normFit[[i]] <- fit_normal( dataSeries[[i]] )
    lapFit[[i]] <- fit_laplace( dataSeries[[i]] )
    aLapFit[[i]] <- fit_alaplace( dataSeries[[i]] )
    subboFit[[i]] <- fit_subbotin( dataSeries[[i]] )
    aSubboFit[[i]] <- fit_asubbotin( dataSeries[[i]] )

    write.table( t( matrix( dataSeries[[i]] ) ), fileData, append = notFirst,
                 sep = ",", row.names = FALSE, col.names = FALSE )
    if( ! notFirst )
      write.table( t( matrix( c( "b", "a", "m" ) ) ), fileFit,
                   sep = ",", row.names = FALSE, col.names = FALSE )
    notFirst <- TRUE
    write.table( t( matrix( subboFit[[i]][c(1:3)] ) ), fileFit, sep = ",",
                 append = TRUE, row.names = FALSE, col.names = FALSE )
  }

  # ---- Plot growth rates against all fits ----

  statsNorm <- statsLap <- statsALap <- statsSubbo <- statsASubbo <- list( )

  for( i in 1 : numCases ){
    statsNorm[[i]] <- plot_lognormal( exp( dataSeries[[i]] ), normFit[[i]], xAxisLabel, yAxisLabel, chartTitle, caseNames[i] )
    statsLap[[i]] <- plot_laplace( dataSeries[[i]], lapFit[[i]], xAxisLabel, yAxisLabel, chartTitle, caseNames[i] )
    if( ! useASubbotin )
      statsSubbo[[i]] <- plot_subbotin( dataSeries[[i]], subboFit[[i]], xAxisLabel, yAxisLabel, chartTitle, caseNames[i] )
    else
      statsASubbo[[i]] <- plot_asubbotin( dataSeries[[i]], aSubboFit[[i]], xAxisLabel, yAxisLabel, chartTitle, caseNames[i] )

    plot_all( dataSeries[[i]], normFit[[i]], lapFit[[i]], subboFit[[i]], xAxisLabel, yAxisLabel, chartTitle, caseNames[i] )
  }

  # ------------- Exception handling code (tryCatch) -------------

}, interrupt = function( ex ) {
  cat( "An interruption was detected.\n" )
  print( ex )
  textplot( "Report incomplete due to interruption." )
}, error = function( ex ) {
  cat( "An error was detected.\n" )
  print( ex )
  textplot( "Report incomplete due to processing error." )
}, finally = {
  # Close PDF plot file
  dev.off( )
})