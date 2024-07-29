#******************************************************************
#
# --- Industry Model: cross-section size distribution analysis ---
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
crossT <- 100                   # period to do cross section analysis
plotRows <- 1					          # number of plots per row in a page
plotCols <- 1					          # number of plots per column in a page
plotW <- 10                     # plot window width
plotH <- 7                      # plot window height

chartTitle <- "Size (market share) distribution (cross-section data)"
xAxisLabel <- "log(Size)"
xAxisLabelNlog <- "Size"
yAxisLabel <- "log(Rank)"
yAxisLabelNlog <- "Rank"

caseNames <- c( )               # enter custom cases names here


# ====== External support functions & definitions ======

source( "StatFuncs.R" )


# ==== MAIN SCRIPT (data processing starts here) ====

# ---- Read data files ----

readFiles <- list.files( path = folder, pattern = paste0( baseName, "_[0-9]+.res"),
                         full.names = TRUE )

dataSeries <- read.list.lsd( readFiles, "_s", instance = 0, pool = poolData )

numCases <- length( dataSeries )

# ---- Verify an/or create labels for each case (for plots) ----

numNames <- length( caseNames )

if( numNames < numCases )
  for( i in ( numNames + 1 ) : numCases )
    caseNames[i] <- paste( "Run", i )

# ---- Select cross section data ----

for( i in 1: numCases ){
  dataSeries[[i]] <- dataSeries[[i]][ crossT, ]
  dataSeries[[i]] <- sort( dataSeries[[i]][ !is.na( dataSeries[[i]] ) ] ) # ascending order, strip NAs
  dataSeries[[i]] <- dataSeries[[i]][ abs( dataSeries[[i]] ) < outLim ]   # remove negative outliers
}

# ---- Enter error handling mode so PDF can be closed in case of error/interruption ----

tryCatch({

  # ---- Open PDF plot file for output ----

  pdf( paste0( folder, "/", baseName, "_SizeDistCS.pdf" ),
       width = plotW, height = plotH )
  options( scipen = 5 )                 # max 5 digits
  par( mfrow = c ( plotRows, plotCols ) )             # define plots per page

  # ---- Fit data to lognormal distributions ----

  lognormFit <- list( )

  for( i in 1 : numCases ){
    lognormFit[[i]] <- fit_lognormal( dataSeries[[i]] )
  }

  # ---- Plot size rank against lognormal fit ----

  for( i in 1 : numCases ){
    plot_rankLognormal( dataSeries[[i]], lognormFit[[i]], xAxisLabel,
                        yAxisLabel, chartTitle, caseNames[i] )
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
