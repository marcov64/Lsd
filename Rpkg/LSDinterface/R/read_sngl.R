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
  header <- scan( file, what = character( ), sep = "\t", quote = NULL,
                  nlines = 1, quiet = TRUE )
  header <- header[ 1 : ( length( header ) - 1 ) ]  # remove last tab
  header <- make.names( header )

  if( length( header ) == 0 )            # invalid file?
    stop( paste0( "File '", file, "' is invalid!") )

  # try to calculate data size
  if ( nrows > 0 ) {
    n <- length( header ) * ( nrows - skip )
  } else {
    if ( nrows < 0 ) {
      n <- -1
    } else {                            # nrows = 0 : get initial values
      n <- length( header )
      skip <- -1
    }
  }

  # read data from disk
  dataSet <- matrix( scan( file, what = numeric( ), n = n, quote = NULL,
                           skip = skip + 2, na.strings = "NA", quiet = TRUE ),
                     ncol = length( header ), byrow = TRUE )

  if( nrow( dataSet ) == 0 )            # invalid file?
    stop( paste0( "File '", file, "' is invalid!") )

  # adjust row labels
  rownames( dataSet ) <- c( ( 1 + skip ) : ( nrow( dataSet ) + skip ) )

  # reformat column labels
  colnames( dataSet ) <- name.clean.lsd( header )

  # remove unwanted objects' columns if needed
  if( ! is.null( posit ) && length( posit ) > 0 ) {

    # format position if needed
    posit <- fmt.posit( posit, match.arg( posit.match ) )

    # matrix to store the columns, keep rownames
    fieldData <- matrix( nrow = nrow( dataSet ), ncol = 0,
                         dimnames = list( rownames( dataSet ) ) )
    fieldCols <- 0

    # select only required columns
    for( i in 1 : ncol( dataSet ) ) {

      # build position string and check it
      pos <- unlist( strsplit( header[ i ], ".", fixed = TRUE ) )[ 2 ]

      if( match.arg( posit.match ) == "fixed" ) {
        if( ! ( pos %in% posit ) )
          next

      } else {
        found <- FALSE
        for( j in 1 : length( posit ) )
          if( grepl( posit[ j ], pos ) )
            found <- TRUE

        if( ! found )
          next
      }

      # ok, so the column should be included
      fieldData <- cbind( fieldData, dataSet[ , i ] )
      fieldCols <- fieldCols + 1

      # apply column labels (first column requires different handling)
      if( ncol( fieldData ) == 1 )
        colnames( fieldData ) <- name.clean.lsd( header[ i ] )
      else
        colnames( fieldData )[ fieldCols ] <- name.clean.lsd( header[ i ] )
    }

    if( ncol( fieldData ) == 0 )
      stop( paste0( "File '", file, "' has no variable with the specified position(s)") )

    dataSet <- fieldData
  }

  # remove unwanted columns if needed
  if( ! is.null( col.names ) || instance != 0 ) {

    # check column names to adjust to R imported column names
    if( check.names && ! is.null( col.names ) )
      col.names <- names.in.set( col.names, header )

    dataSet <- select.colnames.lsd( dataSet, col.names = col.names,
                                    instance = instance,
                                    check.names = check.names )
  }

  if( clean.names ) {
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
