#*******************************************************************************
#
# ------------------ Tools for interfacing with LSD results ------------------
#
#   Written by Marcelo C. Pereira, University of Campinas
#
#   Copyright Marcelo C. Pereira
#   Distributed under the GNU General Public License
#
#*******************************************************************************

# ==== Read LSD results file and clean variables names ====

read.raw.lsd <- function( file, nrows = -1, skip = 0, col.names = NULL,
                          check.names = TRUE, clean.names = FALSE,
                          instance = 0, posit = NULL,
                          posit.match = c( "fixed", "glob", "regex" ) ) {

  # read header line (labels) from disk
  header <- readLines( file, n = 1, warn = FALSE )
  header <- unlist( strsplit( header, "\t", fixed = TRUE ) )
  header <- make.names( header )
  nVar <- length( header )

  if( nVar == 0 )                       # invalid file?
    stop( paste0( "File '", file, "' is invalid!") )

  # try to calculate data size
  nrows <- max( as.integer( nrows ), -1 )
  skip <- max( as.integer( skip ), 0 )

  # nrows = 0 : get initial values only
  if ( nrows > 0 )
    nLines <- nrows - skip
  else
    if ( nrows < 0 )
      nLines <- num.lines( file ) - 2 - skip
    else {
      nLines <- 1
      skip <- -1
    }

  if( nLines <= 0 )
    return( NULL )

  # read data from disk
  dataSet <- matrix( unlist( scan( file, what = as.list( rep( 0.0, nVar ) ),
                                   nlines = nLines, quote = NULL, skip = skip + 2,
                                   na.strings = "NA", flush = TRUE, fill = TRUE,
                                   multi.line = FALSE, quiet = TRUE ),
                             recursive = FALSE, use.names = FALSE ),
                     ncol = nVar, nrow = nLines,
                     dimnames = list( c( ( 1 + skip ) : ( nLines + skip ) ),
                                      name.clean.lsd( header ) ) )

  if( nrow( dataSet ) == 0 || ncol( dataSet ) == 0 )      # invalid file?
    stop( paste0( "File '", file, "' contains no LSD data!") )

  # remove unwanted columns if needed
  if( ! is.null( col.names ) || instance != 0 ||
      ( ! is.null( posit ) && length( posit ) > 0 ) ) {

    # check column names to adjust to R imported column names
    if( check.names && ! is.null( col.names ) )
      col.names <- names.in.set( col.names, header )

    dataSet <- select.colnames.lsd( dataSet, col.names = col.names,
                                    instance = instance,
                                    check.names = check.names, posit = posit,
                                    posit.match = match.arg( posit.match ) )
  }

  if( clean.names && ! is.null( dataSet ) ) {

    cleaNames <- name.r.unique.lsd( colnames( dataSet ) )

    if( length( cleaNames ) == ncol( dataSet ) )
      colnames( dataSet ) <- cleaNames
  }

  return( dataSet )
}


# ==== Read LSD variables (one instance of each variable only) ====

read.single.lsd <- function( file, col.names = NULL, nrows = -1, skip = 0,
                             check.names = TRUE, instance = 1, posit = NULL,
                             posit.match = c( "fixed", "glob", "regex" ) ) {

  # only single instance
  if( ! is.numeric( instance ) || length( instance ) != 1 || instance <= 0 )
    instance <- 1
  else
    instance < as.integer( instance )

  dataSet <- read.raw.lsd( file, nrows = nrows, skip = skip,
                           col.names = col.names, check.names = check.names,
                           clean.names = TRUE, instance = instance,
                           posit = posit, posit.match = match.arg( posit.match ) )

  return( dataSet )
}


# ==== Read specified LSD variables (even if there are several instances) ====

read.multi.lsd <- function( file, col.names = NULL, nrows = -1, skip = 0,
                            check.names = TRUE, posit = NULL,
                            posit.match = c( "fixed", "glob", "regex" ),
                            posit.cols = FALSE ) {

  # ---- Read data from file and remove artifacts ----

  dataSet <- read.raw.lsd( file, nrows = nrows, skip = skip, instance = 0,
                           col.names = col.names, check.names = check.names,
                           posit = posit, posit.match = match.arg( posit.match ) )

  # ---- process field types ----

  fieldData <- list( )                  # list to store each variable
  fixedLabels <- name.r.unique.lsd( colnames( dataSet ) ) # unique variables

  for( i in 1 : length( fixedLabels ) ) {

    # ---- Select only required columns ----
browser()
    fieldData[[ i ]] <- select.colnames.lsd( dataSet, fixedLabels[ i ],
                                             instance = 0,
                                             check.names = check.names )
    names( fieldData )[ i ] <- fixedLabels[ i ]

    if( is.null( fieldData[[ i ]] ) )
      warning( paste0( "Variable '", col.names[i],
                       "' not found, skipping..."), call. = FALSE )
    else
      if( posit.cols )
        for( j in 1 : ncol( fieldData[[ i ]] ) )
          colnames( fieldData[[ i ]] )[ j ] <-
            unlist( strsplit( colnames( fieldData[[ i ]] )[ j ],
                              ".", fixed = TRUE ) )[ 2 ]
  }

  return( fieldData )                   # return a list of matrices
}


# ==== support function to read the number of lines of a text file ====

num.lines <- function( file ) {

  f <- gzfile( file, open = "rb" )

  nLines <- 0L
  while ( length( chunk <- readBin( f, "raw", 1048576 ) ) > 0 )
    nLines <- nLines + sum( chunk == as.raw( 10L ) )

  close( f )

  return( nLines )
}
