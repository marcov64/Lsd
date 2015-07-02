########### Functions to load LSD result files in R ############

require( abind, warn.conflicts = FALSE, quietly = TRUE )


# ==== Get the original LSD variable name from a R column name ====

var.name.lsd <- function( r.name ){
  lsd.name <- gsub( "\\..+$", "", r.name )
  lsd.name <- gsub( "^X_", "_", lsd.name )
  return( lsd.name )
}


# ==== Get a clean (R) variable name from R initial column name conversion ====

clean.name.lsd <- function( r.name ){

  # adjust the time span format and remove trailing points
  clean.name <- gsub( "([0-9]+)\\.([0-9]+)\\.$", "\\1_\\2", r.name )
  clean.name <- gsub( "\\.\\.", "\\.", clean.name ) # replace double points by a single one

  return( clean.name )
}


# ==== Read variable names in results file (no duplicates) ====

read.names.lsd <- function( file, stringsAsFactors = default.stringsAsFactors()){

  # read from disk
  dataSet <- read.delim( file, na.strings = "NA", nrows = 1,
                         stringsAsFactors = stringsAsFactors )

  if( nrow( dataSet ) == 0 )            # invalid file?
    stop( paste0( "File '", file, "' is invalid!") )

  if( all( is.na( dataSet[ , ncol( dataSet ) ]) ) )  # remove empty last column
    dataSet <- dataSet[ , - ncol( dataSet )]

  # extract labels and remove duplicates
  lsd.name <- unique( var.name.lsd( colnames( dataSet ) ) )

  return( lsd.name )
}


# ==== Read initial conditions in results file ====

read.init.lsd <- function( file, stringsAsFactors = default.stringsAsFactors()){

  # read from disk
  dataSet <- read.delim( file, na.strings = "NA", nrows = 1,
                         stringsAsFactors = stringsAsFactors )

  if( nrow( dataSet ) == 0 )            # invalid file?
    stop( paste0( "File '", file, "' is invalid!") )

  # remove empty last column
  if( all( is.na( dataSet[ , ncol( dataSet ) ]) ) )
    dataSet <- dataSet[ , - ncol( dataSet )]

  # reformat column labels
  colnames( dataSet ) <- clean.name.lsd( colnames( dataSet ) )

  return( dataSet )
}


# ==== Read  info from a results file ====

read.info.lsd <- function( file, stringsAsFactors = default.stringsAsFactors()){

  # read from disk
  dataSet <- read.delim( file, na.strings = "NA", nrows = 1,
                         stringsAsFactors = stringsAsFactors )

  if( nrow( dataSet ) == 0 )            # invalid file?
    stop( paste0( "File '", file, "' is invalid!") )

  # remove empty last column
  if( all( is.na( dataSet[ , ncol( dataSet ) ]) ) )
    dataSet <- dataSet[ , - ncol( dataSet )]

  # reformat column labels
  colnames( dataSet ) <- clean.name.lsd( colnames( dataSet ) )

  # get the most "deep" object position
  maxPosit <- 1
  for( i in 1 : length( colnames( dataSet ) ) ){
    # break position into components
    parseName <- unlist( strsplit( colnames( dataSet )[ i ],"\\." ) )
    parsePosit <- unlist( strsplit( parseName[ 2 ], "_" ) )
    maxPosit <- max( maxPosit, length( parsePosit ) )
  }

  # get variable names
  fullNames <- colnames( dataSet )
  lsdNames <- var.name.lsd( fullNames )

  # organize dataset with variable names in rows
  info <- data.frame( stringsAsFactors = stringsAsFactors )
  for( i in 1 : length( colnames( dataSet ) ) ){
    # break position and time into components
    parseName <- unlist( strsplit( fullNames[ i ],"\\." ) )
    parsePosit <- unlist( strsplit( parseName[ 2 ], "_" ) )
    parseTime <- unlist( strsplit( parseName[ 3 ], "_" ) )
    # form new row
    newLine <- data.frame( Full_name = fullNames[ i ],
                           R_name = parseName[ 1 ],
                           LSD_name = lsdNames[ i ],
                           Init_value = dataSet[ 1, i ],
                           Init_time = strtoi( parseTime [ 1 ] ),
                           End_time = strtoi( parseTime [ 2 ] ),
                           stringsAsFactors = stringsAsFactors )
    # add positions > 1
    iniCol <- ncol( newLine )
    for( j in 1 : maxPosit ){
      if( j > length( parsePosit ) )
        posit <- NA
      else
        posit <- strtoi( parsePosit[ j ] )

      newLine <- cbind( newLine, posit )
      colnames( newLine )[ iniCol + j ] <- paste0( "Posit_", j )
    }

    info <- rbind( info, newLine )
  }

  return( info )
}


# ==== Select a subset of a data frame (by column names) ====

select.colnames.lsd <- function( dataSet, col.names, instance = 0,
                                 stringsAsFactors = default.stringsAsFactors() ){

  # matrix to store the columns, keep rownames
  fieldData <- matrix( nrow = nrow( dataSet ), ncol = 0 )
  rownames( fieldData ) <- rownames( dataSet )

  # select only required columns
  for( i in 1 : length( col.names ) ){

    # extract one name at a time
    subSet <- dataSet[ , grep( paste0("^", col.names[ i ] ), colnames( dataSet ) ) ]

    # bind one column (instance > 0) or all matching (instance = 0)
    if( is.vector( subSet ) || instance == 0 )
      fieldData <- cbind( fieldData, subSet )
    else
      fieldData <- cbind( fieldData, subSet[ , instance ] )

    # add column name if needed
    if( is.vector( subSet ) )
      colnames( fieldData )[ ncol( fieldData ) ] <- col.names[ i ]
    if( ! is.vector( subSet ) && instance != 0 )
      colnames( fieldData )[ ncol( fieldData ) ] <- colnames( subSet )[ instance ]
  }

  return( data.frame( fieldData, stringsAsFactors = stringsAsFactors ) )
}


# ==== Select a subset of a data frame (by variable attributes) ====

select.colattrs.lsd <- function( dataSet, info, col.names = NA, posit = NA,
                                 init.value = NA, init.time = NA, end.time = NA,
                                 stringsAsFactors = default.stringsAsFactors() ){

  # test if files are compatible (in principle)
  if( ncol( dataSet ) != nrow( info ) )
    stop( "Info table incompatible with provided dataSet" )

  # format valid names for matching
  col.names <- make.names( clean.name.lsd( col.names ) )

  # check if position is not formatted as text and convert if needed
  if( ! is.na( posit ) && ! is.character( posit ) ){
    positChr <- paste0( posit[ 1 ] )
    for( i in 2 : length( posit ) )
      positStr <- paste0( positChr, "_", posit[ i ] )
    posit <- positStr
  }

  # data frame to store the columns, keep rownames
  fieldData <- data.frame( row.names = rownames( dataSet ),
                           stringsAsFactors = stringsAsFactors )
  fieldCols <- 0

  # select only required columns
  for( i in 1 : nrow( info ) ){

    # if column names are specified, check if belongs to the set
    if( ! is.na( col.names ) && ! ( info$R_name[ i ] %in% col.names ) )
      next

    # check if value attributes match
    if( ! is.na( init.value ) && ! ( info$Init_value[ i ] %in% init.value ) )
      next
    if( ! is.na( init.time ) && ! ( info$Init_time[ i ] %in% init.time ) )
      next
    if( ! is.na( end.time ) && ! ( info$End_time[ i ] %in% end.time ) )
      next

    # build position string and check it
    if( ! is.na( posit ) ){
      j <- which( colnames( info ) == "Posit_1")
      positStr <- paste0( info[ i, j ] )
      j <- j + 1
      while( j <= ncol( info ) && ! is.na( info[ i, j ] ) ){
        positStr <- paste0( positStr, "_", info[ i, j ] )
        j <- j + 1
      }
      if( ! ( positStr %in% posit ) )
        next
    }

    # ok, so the column should be included
    fieldData <- cbind( fieldData, dataSet[ , i ] )
    fieldCols <- fieldCols + 1

    # apply column labels (first column requires different handling)
    if( ncol( fieldData ) == 1 )
      colnames( fieldData ) <- info$Full_name[ i ]
    else
      colnames( fieldData )[ fieldCols ] <- as.character( info$Full_name[ i ] )
  }

  return( fieldData )
}


# ==== Read LSD results file and clean variables names ====

read.raw.lsd <- function( file, nrows = -1, skip = 0,
                          stringsAsFactors = default.stringsAsFactors() ){
  # read from disk
  dataSet <- read.delim( file, na.strings = "NA",
                         stringsAsFactors = stringsAsFactors )

  if( nrow( dataSet ) == 0 )            # invalid file?
    stop( paste0( "File '", file, "' is invalid!") )

  # remove unwanted rows and columns (artifacts)
  dataSet <- dataSet[ -1, ]             # remove first data row (initial conditions)
  if( all( is.na( dataSet[ , ncol( dataSet ) ]) ) )  # remove empty last column
    dataSet <- dataSet[ , - ncol( dataSet )]

  # remove rows to discard (user)
  if( skip > 0 )                        # there are rows to skip?
    dataSet <- dataSet[ - c( 1 : skip ), ]
  if( nrows >= 0 && nrow( dataSet ) >= nrows) # there are rows to remove?
    dataSet <- dataSet[ c( 0 : nrows ), ]

  # adjust rown labels
  if( nrow( dataSet ) > 0 )             #any row to label?
    rownames( dataSet ) <- c( ( 1 + skip ) : ( nrow( dataSet ) + skip ) )

  # reformat column labels
  colnames( dataSet ) <- clean.name.lsd( colnames( dataSet ) )

  return( dataSet )
}


# ==== Read LSD variables (one instance of each variable only) ====

read.lsd <- function( file, col.names = NULL, nrows = -1, skip = 0, check.names = TRUE,
                      instance = 1, stringsAsFactors = default.stringsAsFactors()){

  # ---- check column names to adjust to R imported column names ----

  if( length( col.names ) == 0 )
    fixedLabels = read.names.lsd( file, stringsAsFactors = stringsAsFactors )
  else
    fixedLabels = col.names

  if( check.names )
    fixedLabels <- unique( make.names( fixedLabels ) )

  if( instance <= 0 )                 # only single instance
    instance <- 1

  # ---- Read data from file and remove artifacts ----

  dataSet <- read.raw.lsd( file, nrows = nrows, skip = skip,
                           stringsAsFactors = stringsAsFactors )

  # ---- remove unwanted or duplicated (multiple instances) columns ----

  fieldData <- data.frame( select.colnames.lsd( dataSet, fixedLabels, instance ),
                           stringsAsFactors = stringsAsFactors )

  return( fieldData )
}


# ==== Read specified LSD variables (even if there are several instances) ====

read.multi.lsd <- function( file, col.names = NULL, nrows = -1, skip = 0,
							check.names = TRUE,
							stringsAsFactors = default.stringsAsFactors()){

  # ---- check column names to adjust to R imported column names ----

  if( length( col.names ) == 0 )
    fixedLabels = read.names.lsd( file, stringsAsFactors = stringsAsFactors )
  else
    fixedLabels = col.names

  if( check.names )
    fixedLabels <- unique( make.names( fixedLabels ) )

  # ---- Read data from file and remove artifacts ----

  dataSet <- read.raw.lsd( file, nrows = nrows, skip = skip,
                           stringsAsFactors = stringsAsFactors )

  # ---- process field types ----

  fieldData <- list()                   # list to store each variable

  for( i in 1 : length( fixedLabels ) ){

    # ---- Select only required columns ----

    fieldData[[ i ]] <- data.frame( select.colnames.lsd( dataSet, fixedLabels[ i ],
                                                         instance = 0 ),
                                    stringsAsFactors = stringsAsFactors )

    if( ncol( fieldData[[ i ]] ) == 0 )
      warning( paste0( "Variable '", col.names[i],"' not found, skipping...") )
  }

  if( length( fieldData ) == 0 )        # nothing to return?
    return( NULL )
  if( i > 1 )                           # there are multiple variables?
    return( fieldData )                 # return a list of data frames
  else
    return( fieldData[[1]] )            # or a single data frame
}


# ==== Read LSD variables from multiple runs into a 3D array ====

read.3d.lsd <- function( files, col.names = NULL, nrows = -1, skip = 0,
                         check.names = TRUE, instance = 1,
                         stringsAsFactors = default.stringsAsFactors()){

  # ---- check column names to adjust to R imported column names ----

  if( length( col.names ) == 0 )
    fixedLabels = read.names.lsd( files[1], stringsAsFactors = stringsAsFactors )
  else
    fixedLabels = col.names

  if( check.names )
    fixedLabels <- unique( make.names( fixedLabels ) )

  if( instance <= 0 )                 # only single instance
    instance <- 1

  # ---- Run across all files ----

  for (i in 1:length( files )) {

    # ---- Read data from file and remove artifacts ----

    dataSet <- read.raw.lsd( files[ i ], nrows = nrows, skip = skip,
                             stringsAsFactors = stringsAsFactors )

    # ---- Select only required columns ----

    fieldData <- select.colnames.lsd( dataSet, fixedLabels, instance )

    # ---- Stack multiple 2D files as a 3D array ----

    if( i == 1 ){                     # don't bind if first file
      dataArray <- fieldData
      nrows <- nrow( fieldData )      # define base dimensions
      ncols <- ncol( fieldData )
    } else{
      # check consistency
      if( nrow( fieldData ) != nrows || ncol( fieldData ) != ncols )
        stop( paste0( "File '", files[ i ], "' has incompatible dimensions!") )

      # 3D binding
      dataArray <- abind( dataArray, fieldData, along = 3, use.first.dimnames = TRUE )
    }
  }

  return( dataArray )
}


# ==== Read LSD variables from multiple runs into a list ====

read.list.lsd <- function( files, col.names = NULL, nrows = -1, skip = 0,
                           check.names = TRUE, instance = 0, pool = FALSE,
                           stringsAsFactors = default.stringsAsFactors()){

  # ---- check column names to adjust to R imported column names ----

  if( length( col.names ) == 0 )
    fixedLabels = read.names.lsd( files[1], stringsAsFactors = stringsAsFactors )
  else
    fixedLabels = col.names

  if( check.names )
    fixedLabels <- unique( make.names( fixedLabels ) )

  # ---- Run across all files ----

  fieldData <- list( )                # create list to hold data

  for(i in 1:length( files )) {

    # ---- Read data from file and remove artifacts ----

    dataSet <- read.raw.lsd( files[ i ], nrows = nrows, skip = skip,
                             stringsAsFactors = stringsAsFactors )

    # ---- Select only required columns ----

    subSet <- select.colnames.lsd( dataSet, fixedLabels, instance )

    # ---- select aggregation mode

    if( ! pool )
      fieldData[[ i ]] <- subSet
    else
      if( i == 1 )
        fieldData[[ 1 ]] <- subSet
      else{
        # bind one column (instance > 0) or all matching (instance = 0)
        if( is.vector( subSet ) || instance == 0 )
          fieldData[[ 1 ]] <- cbind( fieldData[[ 1 ]], subSet )
        else
          fieldData[[ 1 ]] <- cbind( fieldData[[ 1 ]], subSet[ , instance ] )

        # add column name if needed
        if( is.vector( subSet ) && length( fixedLabels ) >= i )
          colnames( fieldData[[ 1 ]] )[ ncol( fieldData[[ 1 ]] ) ] <- fixedLabels[ i ]
        if( ! is.vector( subSet ) && instance != 0 )
          colnames( fieldData[[ 1 ]] )[ ncol( fieldData[[ 1 ]] ) ] <- colnames( subSet )[ instance ]
      }
  }

  return( fieldData )
}
