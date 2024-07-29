#******************************************************************
#
# --------- Industry Model: scaling of variance analysis ---------
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
useLogSize <- FALSE             # use size in logs (T) or not (F)
iniDrop <- 100                  # initial time steps to drop from analysis (0=none)
nKeep <- 3                      # number of time steps to keep (-1=all)
plotRows <- 1					          # number of plots per row in a page
plotCols <- 1					          # number of plots per column in a page
plotW <- 10                     # plot window width
plotH <- 7                      # plot window height

chartTitle <- "Scaling of variance regarding firm size"
xAxisLabel <- "Size of firms"
yAxisLabel <- "Standard deviation of growth rate"

caseNames <- c( )               # enter custom cases names here


# ====== External support functions & definitions ======

source( "StatFuncs.R" )
outLim <- 1       # outlier limit for size (absolute)


# ==== MAIN SCRIPT (data processing starts here) ====

# ---- Read data files ----

readFiles <- list.files( path = folder, pattern = paste0( baseName, "_[0-9]+.res"),
                         full.names = TRUE )

share <- read.list.lsd( readFiles, "_s", skip = iniDrop, nrows = nKeep,
                        instance = 0, pool = poolData )

numCases <- length( share )

for( i in 1 : numCases ) {
  if( is.null( share[[i]] ) ) {
    stop( "Inconsistent data, check if variable '_s' is saved on data" )
  }
}

# ---- Verify an/or create labels for each case ----

numNames <- length( caseNames )

if( numNames < numCases )
  for( i in ( numNames + 1 ) : numCases )
    caseNames[i] <- paste( "Run", i )

if( useLogSize ){
  xAxisLabel <- paste( xAxisLabel, "(log)" )
  yAxisLabel <- paste( yAxisLabel, "(log)" )
}

# ---- Preprocess data and create growth matrix ----

growth <- list( )

for( i in 1 : numCases )
{
  obs <- nrow( share[[i]] )                     # number of observations in i
  firms <- ncol( share[[i]] )                   # number of firms in i

  # compute normalized log size, if required
  if( useLogSize ) {
    for( j in 1 : obs ){                        # compute normalized size line-wise
      sizes <- share[[i]][ j, ]
      avgSlog <- mean( sizes[ ! is.na( sizes ) ] )
      for( k in 1 : firms )                     # and firm-wise
        if( ! is.na( share[[i]][ j, k ] ) )
          share[[i]][ j, k ] <- log( share[[i]][ j, k ] ) - avgSlog
    }

    rm( sizes, avgSlog )
  }

  # create growth matrix
  growth[[i]] <- matrix( NA, obs, firms )       # create empty firm growth matrix
  for( j in 2 : obs )                           # compute growth line-wise
    for( k in 1 : firms )                       # and them firm-wise
      if( ! is.na(share[[i]][ j - 1, k ] ) &&
          ! is.na(share[[i]][ j , k ] ) ){ # if possible
        if( useLogSize )
          growth[[i]][ j, k ] <- share[[i]][ j , k ] - share[[i]][ j - 1, k ]
        else
          growth[[i]][ j, k ] <- share[[i]][ j , k ] / share[[i]][ j - 1, k ] - 1
      }
}

# ---- Order data, remove outliers and generate some statistics ----

sortData <- list( )

for( i in 1: numCases ){
  obs <- nrow( growth[[i]] )                # number of observations in i

  # transforms share & growth matrices into vectors and the back int a n x 2 matrix
  shareVect <- as.vector( share[[i]] )
  growthVect <- as.vector( growth[[i]] )
  sortData[[i]] <- cbind( shareVect, growthVect )

  # remove rows without growth, outliers and sort by size
  iniLen <- nrow( sortData[[i]] )
  sortData[[i]] <- sortData[[i]][ !is.na( sortData[[i]][ , 2 ] ), , drop = FALSE ]
  sortData[[i]] <- sortData[[i]][ abs( sortData[[i]][ , 1 ] ) <= outLim, , drop = FALSE ]
  sortData[[i]] <- sortData[[i]][ order( sortData[[i]][ , 1 ] ), , drop = FALSE ]
  finLen <- nrow( sortData[[i]] )
}

rm( shareVect, growthVect )
rm( share, growth )

# ---- Compute all required statistics ----

binsSet <- binsSAvg <- binsGSD <- list( )

for( i in 1 : numCases ){

  binsSet[[i]] <- list( )                       # a list of bins inside a list of cases
  binsSAvg[[i]] <- binsGSD[[i]] <- vector( "numeric" )

  # organize data set into bins
  bins <- hist( sortData[[i]][ , 1 ], breaks = nBins, plot = F ) # define bins limits

  j <- 1                                            # start reading first obs

  for( k in 1 : ( length( bins$breaks ) - 1 ) ){        # for all bins

    binsSet[[i]][[k]] <- matrix( nrow = 0, ncol = 2 ) # create empty matrix for bins

    # scan till the end of dataset or for all members of bin
    while( j <= nrow( sortData[[i]] ) &&
           sortData[[i]][ j, 1 ] < bins$breaks[ k + 1 ] ){
      # add observation to the bin and move to next obs
      binsSet[[i]][[k]] <- rbind( binsSet[[i]][[k]], sortData[[i]][ j, ],
                                  deparse.level = 0, make.row.names = FALSE )
      j <- j + 1
    }

    # calculate average of size and SD of growth for each bin
    binsSAvg[[i]] <- append( binsSAvg[[i]], mean( binsSet[[i]][[k]][ , 1 ] ) )
    if( useLogSize )
      binsGSD[[i]] <- append( binsGSD[[i]], log( sd( binsSet[[i]][[k]][ , 2 ] ) ) )
    else
      binsGSD[[i]] <- append( binsGSD[[i]], sd( binsSet[[i]][[k]][ , 2 ] ) )
  }
}

# ---- Enter error handling mode so PDF can be closed in case of error/interruption ----

tryCatch({

  # ---- Open PDF plot file for output ----

  pdf( paste0( folder, "/", baseName, "_ScalingVar.pdf" ), width = plotW, height = plotH )
  options( scipen = 5 )                 # max 5 digits
  par( mfrow = c ( plotRows, plotCols ) )             # define plots per page

  # ---- Plot sd of growth rates against sizes ----

  for( i in 1 : numCases ){

    # fit a line to linear model SD = alpha + beta * size
    gSD.lm <- lm( V2 ~ V1, as.data.frame( cbind( binsSAvg[[i]], binsGSD[[i]] ) ) )

    plot( binsSAvg[[i]], binsGSD[[i]],type = "p", pch = 1, main = chartTitle,
          sub = caseNames[i], xlab = xAxisLabel, ylab = yAxisLabel )
    abline( gSD.lm, col = "gray" )

    legend( x = "topright", legend = c( "Data", "Linear fit" ),
            inset = 0.03, cex = 0.8, lty = c( 1, 1 ), lwd = 2.5,
            col = c( "black", "gray"), adj = 0.1)

    legend( x = "bottomleft", legend = c( sprintf( "beta = %.4f", coef(summary(gSD.lm))[2,1] ),
                                        sprintf( "std. err. = %.4f", coef(summary(gSD.lm))[2,2] ),
                                        sprintf( "p-val. = %.4f", coef(summary(gSD.lm))[2,4] ) ,
                                        sprintf( "adj. R2 = %.4f", summary(gSD.lm)$adj.r.squared ) ),
    inset = 0.03, cex = 0.8, adj = 0.1)

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
