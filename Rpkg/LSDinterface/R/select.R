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

# ==== Select a subset of a data frame (by column names) ====

select.colnames.lsd <- function( dataSet, col.names = NULL, instance = 0,
                                 check.names = TRUE, posit = NULL,
                                 posit.match = c( "fixed", "glob", "regex" ) ) {

  if( is.null( col.names ) ) {
    col.names <- name.var.lsd( colnames( dataSet ) )
    check.names <- TRUE
  }

  if( length( col.names ) == 0 || ! is.character( col.names ) ||
      any( col.names == "" ) )
    stop( "Invalid vector of variable names (col.names)" )

  if( is.null( check.names ) || ! is.logical( check.names ) )
    stop( "Invalid variable name check switch (check.names)" )

  if( is.null( instance ) || ! is.finite( instance ) || round( instance ) < 0 )
    stop( "Invalid variable instance (instance)" )

  instance <- round( instance )
  posit.match <- match.arg( posit.match )

  # prefix leading underscores, remove trailing points, and duplicates
  if( check.names ) {
    col.names <- name.r.unique.lsd( col.names )
    all.names <- name.r( colnames( dataSet ) )
  } else {
    col.names <- unique( col.names )
    all.names <- colnames( dataSet )
  }

  # remove invalid column names
  col.names <- col.names[ which( col.names %in% all.names ) ]

  if( length( col.names ) == 0 )
    return( NULL )

  # find the matching columns to be preserved
  if( instance == 0 )

    col.match <- all.names %in% col.names

  else {

    n <- length( col.names )
    col.match <- vector( mode = "numeric", length = n )

    # select correct columns
    for( i in 1 : n ) {

      # extract one name at a time
      match <- grep( paste0( "^", col.names[ i ], "\\." ), colnames( dataSet ) )

      # bind one column (instance > 0) or all matching (instance = 0)
      if( length( match ) > 0 || length( match ) <= instance )
        col.match[ i ] <- match[ instance ]
      else
        col.match[ i ] <- NA
    }

    col.match <- col.match[ ! is.na( col.match ) ]

    if( length( col.match ) == 0 )
      return( NULL )
  }

  dataSet <- dataSet[ , col.match, drop = FALSE ]

  if( is.null( posit ) || length( posit ) == 0 || posit[ 1 ] == "" )
    return( dataSet )

  # format position if needed and extract positions in dataset
  posit <- fmt.posit( posit, posit.match )
  all.posit <- obj.posit( colnames( dataSet ) )
  all.posit <- all.posit[ ! is.na( all.posit ) ]

  if( length( all.posit ) == 0 )
    return( NULL )

  if( posit.match == "fixed" )
    col.match <- all.posit %in% posit
  else
    col.match <- grep( posit, all.posit )

  col.match <- col.match[ ! is.na( col.match ) ]

  if( length( col.match ) == 0 )
    return( NULL )

  return( dataSet[ , col.match, drop = FALSE ] )
}


# ==== Select a subset of a data frame (by variable attributes) ====

select.colattrs.lsd <- function( dataSet, info, col.names = NULL, init.value = NA,
                                 init.time = NA, end.time = NA, posit = NULL,
                                 posit.match = c( "fixed", "glob", "regex" ) ) {

  # test if files are compatible (in principle)
  if( ( ! is.matrix( dataSet ) && ! is.data.frame( dataSet ) ) )
    stop( "Invalid dataSet (dataSet)" )

  if( ! inherits( info, "info.details.lsd" ) || ncol( dataSet ) != nrow( info ) )
    stop( "Info table invalid or incompatible with provided dataSet (info)" )

  # format valid names for matching
  if( ! is.null( col.names ) ) {
    if( length( col.names ) == 0 || ! is.character( col.names ) ||
        any( col.names == "" ) )
      stop( "Invalid vector of variable names (col.names)" )

    col.names <- make.names( name.clean.lsd( col.names ) )
  }

  if( ! is.na( init.value ) && ( is.null( init.value ) ||
                                 length( init.value ) == 0 ||
                                 any( ! is.finite( init.value ) ) ) )
    stop( "Invalid initial value(s) (init.value)" )

  if( ! is.na( init.time ) && ( is.null( init.time ) ||
                                length( init.time ) == 0 ||
                                any( ! is.finite( init.time ) ) ||
                                any( round( init.time ) < 1 ) ) )
    stop( "Invalid initial time(s) (init.time)" )

  if( ! is.na( end.time ) && ( is.null( end.time ) ||
                               length( end.time ) == 0 ||
                               any( ! is.finite( end.time ) ) ||
                               any( round( end.time ) < 1 ) ) )
    stop( "Invalid end time(s) (end.time)" )

  if( ! is.null( posit ) && ! ( is.character( posit ) || all( is.finite( posit ) ) ) )
    stop( "Invalid instance position specification (posit)" )

  init.time <- round( init.time )
  end.time <- round( end.time )
  posit.match <- match.arg( posit.match )

  # format position if needed
  posit <- fmt.posit( posit, posit.match )

  # matrix to store the columns, keep rownames
  fieldData <- matrix( nrow = nrow( dataSet ), ncol = 0,
                       dimnames = list( rownames( dataSet ) ) )
  fieldCols <- 0

  # select only required columns
  for( i in 1 : nrow( info ) ) {

    # if column names are specified, check if belongs to the set
    if( ! is.null( col.names ) && ! ( info$R_name[ i ] %in% col.names ) )
      next

    # check if value attributes match
    if( ! is.na( init.value ) && ! ( info$Init_value[ i ] %in% init.value ) )
      next
    if( ! is.na( init.time ) && ! ( info$Init_time[ i ] %in% init.time ) )
      next
    if( ! is.na( end.time ) && ! ( info$End_time[ i ] %in% end.time ) )
      next

    # build position string and check it
    if( length( posit ) > 0 ) {
      pos <- unlist( strsplit( info[ i, "Full_name" ], ".", fixed = TRUE ) )[ 2 ]

      if( posit.match == "fixed" ) {
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
    }

    # ok, so the column should be included
    fieldData <- cbind( fieldData, dataSet[ , i ] )
    fieldCols <- fieldCols + 1

    # apply column labels (first column requires different handling)
    if( ncol( fieldData ) == 1 )
      colnames( fieldData ) <- name.clean.lsd( info$Full_name[ i ] )
    else
      colnames( fieldData )[ fieldCols ] <- name.clean.lsd( info$Full_name[ i ] )
  }

  if( ncol( fieldData ) == 0 )
    return( NULL )
  else
    return( fieldData )
}


# ==== support function to validate/expand the position argument ====

fmt.posit <- function ( posit, posit.match = c( "fixed", "glob", "regex" ) ) {

  # check if position is not formatted as text and convert if needed
  if( ! is.null( posit ) && ! is.character( posit ) ) {
    pos <- as.integer( posit[ 1 ] )
    if( is.na( pos ) )
      stop( "Invalid object position (must be a string or integer vector)" )

    positChr <- paste0( pos )

    if( length( posit ) > 1 )
      for( i in 2 : length( posit ) )
        positChr <- paste0( positChr, "_", posit[ i ] )

    posit <- positChr
  }

  if( ! is.null( posit ) && match.arg( posit.match ) == "glob" ) {
    posit <- utils::glob2rx( posit, trim.tail = FALSE )
  }

  if( length( posit > 1 ) && match.arg( posit.match ) != "fixed" )
    posit <- paste( posit, collapse = "|" )

  return( posit )
}


# ==== support function to extract the object position from full name ====

obj.posit <- function( posit ) {

  if( is.null( posit ) || ! is.character( posit ) )
    return( NA )

  return( sapply( strsplit( posit, ".", fixed = TRUE ), function( x ) x[ 2 ] ) )
}
