########### Normalized productivity distribution (cross section data) ############

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

chartTitle <- "Normalized Productivity distribution (cross section data)"
xAxisLabel <- "log(Productivity)"
xAxisLabelNlog <- "Productivity"
yAxisLabel <- "Binned density (log scale)"
yAxisLabelNlog <- "Binned density"

caseNames <- c( )               # enter custom cases names here


# ====== External support functions & definitions ======

source( "StatFuncs.R" )


# ==== MAIN SCRIPT (data processing starts here) ====

# ---- Read data files ----

readFiles <- list.files( path = folder, pattern = paste0( baseName, "_[0-9]+.res" ),
                         full.names = TRUE )

dataSeries <- read.list.lsd( readFiles, "_aNorm", instance = 0, pool = poolData )

numCases <- length( dataSeries )

# ---- Verify an/or create labels for each case (for plots) ----

numNames <- length( caseNames )

if( numNames < numCases )
  for( i in ( numNames + 1 ) : numCases )
    caseNames[i] <- paste( "Run", i )

# ---- Select cross section data ----

for( i in 1 : numCases )
{
  dataSeries[[i]] <- dataSeries[[i]][ crossT, ]
  dataSeries[[i]] <- sort( dataSeries[[i]][!is.na( dataSeries[[i]] )] )  # ascending order, strip NAs
  dataSeries[[i]] <- dataSeries[[i]][ abs( dataSeries[[i]] ) < outLim ]  # remove outliers
}

# ---- Enter error handling mode so PDF can be closed in case of error/interruption ----

tryCatch({

  # ---- Open PDF plot file for output ----

  pdf( paste0( folder, "/", baseName, "_NormProdDistCS.pdf" ),
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

  statsNorm <- statsLognorm <- statsALap <- statsASubbo <- list( )

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