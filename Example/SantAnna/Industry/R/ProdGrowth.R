#******************************************************************
#
# ------ Industry Model: pool. prod. growth distr. analysis ------
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
nKeep <- -1                     # number of time steps to keep (-1=all)
plotRows <- 1					          # number of plots per row in a page
plotCols <- 1					          # number of plots per column in a page
plotW <- 10                     # plot window width
plotH <- 7                      # plot window height

chartTitle <- "Pooled productivity growth rate distribution"
xAxisLabel <- "log(Productivity growth)"
xAxisLabelNlog <- "Productivity growth"
yAxisLabel <- "Binned density (log scale)"
yAxisLabelNlog <- "Binned density"

caseNames <- c( )               # enter custom cases names here


# ====== External support functions & definitions ======

source( "StatFuncs.R" )


# ==== MAIN SCRIPT (data processing starts here) ====

# ---- Read data files ----

readFiles <- list.files( path = folder, pattern = paste0( baseName, "_[0-9]+.res"),
                         full.names = TRUE )

growth_mkt <- read.list.lsd( readFiles, "_aGrowth", skip = iniDrop,
                             nrows= nKeep, instance = 0, pool = poolData )

numCases <- length( growth_mkt )

# ---- Verify an/or create labels for each case (for plots) ----

numNames <- length( caseNames )

if( numNames < numCases )
  for( i in ( numNames + 1 ) : numCases )
    caseNames[i] <- paste( "Run", i )

# ---- Order data, remove outliers and generate some statistics ----

dataSeries <- list( )

for( i in 1: numCases ){
  obs <- nrow( growth_mkt[[i]] )               # number of observations in i

  dataSeries[[i]] <- sort( as.vector( growth_mkt[[i]][ !is.na( growth_mkt[[i]] ) ] ) )  # ascending order
  iniLen <- length( dataSeries[[i]] )
  dataSeries[[i]] <- dataSeries[[i]][ abs( dataSeries[[i]] ) < outLim ]  # remove outliers (inserted as artifacts by LSD)
  finLen <- length( dataSeries[[i]] )
}

# ---- Enter error handling mode so PDF can be closed in case of error/interruption ----

tryCatch({

  # ---- Open PDF plot file for output ----

  pdf( paste0( folder, "/", baseName, "_ProdGrowth.pdf" ),
       width = plotW, height = plotH )
  options( scipen = 5 )                 # max 5 digits
  par( mfrow = c ( plotRows, plotCols ) )             # define plots per page

  # ---- Fit data to all used distributions ----

  normFit <- lognormFit <- lapFit <- aLapFit <- subboFit <- aSubboFit <- list( )

  for( i in 1 : numCases ){
    normFit[[i]] <- fit_normal( dataSeries[[i]] )
    lognormFit[[i]] <- fit_lognormal( dataSeries[[i]] )
    lapFit[[i]] <- fit_laplace( log( dataSeries[[i]][ dataSeries[[i]] > 0 ] ) )
    aLapFit[[i]] <- fit_alaplace( log( dataSeries[[i]][ dataSeries[[i]] > 0 ] ) )
    subboFit[[i]] <- fit_subbotin( log( dataSeries[[i]][ dataSeries[[i]] > 0 ] ) )
    aSubboFit[[i]] <- fit_asubbotin( log( dataSeries[[i]][ dataSeries[[i]] > 0 ] ) )
  }

  # ---- Plot growth rates against all fits ----

  statsNorm <- statsLognorm <- statsALap <- statsSubbo <- statsASubbo <- list( )

  for( i in 1 : numCases ){
    statsNorm[[i]] <- plot_normal( dataSeries[[i]], normFit[[i]], xAxisLabelNlog, yAxisLabelNlog, chartTitle, caseNames[i] )
    statsLognorm[[i]] <- plot_lognormal( dataSeries[[i]], lognormFit[[i]], xAxisLabel, yAxisLabel, chartTitle, caseNames[i] )
    statsALap[[i]] <- plot_alaplace( log( dataSeries[[i]][ dataSeries[[i]] > 0 ] ), aLapFit[[i]], xAxisLabel, yAxisLabel, chartTitle, caseNames[i] )
    statsASubbo[[i]] <- plot_asubbotin( log( dataSeries[[i]][ dataSeries[[i]] > 0 ] ), aSubboFit[[i]], xAxisLabel, yAxisLabel, chartTitle, caseNames[i] )
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
