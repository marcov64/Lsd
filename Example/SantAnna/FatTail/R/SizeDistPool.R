########### Size distribution (pooled data) ############

# ==== User defined parameters ====

folder   <- "MarkII/Beta"       # subfolder of working dir containing data
baseName <- "MarkII-Beta"       # data files base name
poolData <- TRUE               # pool all data files (T) or treat separately (F)
plotQQ <- FALSE                 # plot quartile to quartile fit graphs or not
iniDrop <- 0                    # initial time steps to drop from analysis (0=none)
nKeep <- -1                     # number of time steps to keep (-1=all)
plotRows <- 1					          # number of plots per row in a page
plotCols <- 1					          # number of plots per column in a page
plotW <- 10                     # plot window width
plotH <- 7                      # plot window height

chartTitle <- "Size (market share) distribution (pooled data)"
xAxisLabel <- "log(Size)"
xAxisLabelNlog <- "Size"
yAxisLabel <- "log(Rank)"
yAxisLabelNlog <- "Rank"

caseNames <- c( )               # enter custom cases names here


# ====== External support functions & definitions ======

source( "StatFuncs.R" )


# ==== MAIN SCRIPT (data processing starts here) ====

# ---- Read data files ----

readFiles <- list.files( path = folder, pattern = paste0( baseName, "_[0-9]+.res" ),
                         full.names = TRUE )

size_mkt <- read.list.lsd( readFiles, "_s", skip = iniDrop, nrows= nKeep,
                           instance = 0, pool = poolData )

numCases <- length( size_mkt )

# ---- Verify an/or create labels for each case (for plots) ----

numNames <- length( caseNames )

if( numNames < numCases )
  for( i in ( numNames + 1 ) : numCases )
    caseNames[i] <- paste( "Run", i )

# ---- Order data, remove outliers and generate some statistics ----

dataSeries <- list( )

for( i in 1: numCases ){
  obs <- nrow( size_mkt[[i]] )               # number of observations in i

  dataSeries[[i]] <- sort( as.vector( size_mkt[[i]][!is.na( size_mkt[[i]] )] ) )  # ascending order
  iniLen <- length( dataSeries[[i]] )
  dataSeries[[i]] <- dataSeries[[i]][abs( dataSeries[[i]] ) < outLim]  # remove outliers (inserted as artifacts by LSD)
  finLen <- length( dataSeries[[i]] )
}

# ---- Enter error handling mode so PDF can be closed in case of error/interruption ----

tryCatch({

  # ---- Open PDF plot file for output ----

  pdf( paste0( folder, "/", baseName, "_SizeDistPool.pdf" ),
       width = 8, height = 12 )
  options( scipen = 5 )                 # max 5 digits
  par( mfrow = c ( plotRows, plotCols ) )             # define plots per page

  # ---- Fit data to lognormal distribution ----

  lognormFit <- list( )
  fileData <- paste0( folder, "/", "data_rs_", baseName, ".csv" )
  fileFit <- paste0( folder, "/", "lnormfit_rs_", baseName, ".csv" )
  notFirst <- FALSE

  for( i in 1 : numCases ){
    lognormFit[[i]] <- fit_lognormal( dataSeries[[i]] )

    write.table( t( matrix( dataSeries[[i]] ) ), fileData, sep = ",",
                 append = notFirst, row.names = FALSE, col.names = FALSE )
    if( ! notFirst )
      write.table( t( matrix( c( "sd", "avg" ) ) ), fileFit,
                   sep = ",", row.names = FALSE, col.names = FALSE )
    notFirst <- TRUE
    write.table( t( matrix( lognormFit[[i]][c(1:2)] ) ), fileFit, sep = ",",
                 append = TRUE, row.names = FALSE, col.names = FALSE )
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