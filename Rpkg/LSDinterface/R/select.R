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
                                 check.names = TRUE ) {

  if( is.null( col.names ) || ! is.character( col.names ) ) {
    col.names <- name.var.lsd( colnames( dataSet ) )
    check.names <- TRUE
  }

  # prefix leading underscores, remove trailing points, and duplicates
  if( check.names )
    col.names <- name.r.unique.lsd( col.names )

  # matrix to store the columns, keep rownames
  fieldData <- matrix( nrow = nrow( dataSet ), ncol = 0 )
  rownames( fieldData ) <- rownames( dataSet )

  # select only required columns
  for( i in 1 : length( col.names ) ) {

    # extract one name at a time
    subSet <- dataSet[ , grep( paste0( "^", col.names[ i ], "\\." ),
                               colnames( dataSet ) ), drop = FALSE ]

    # bind one column (instance > 0) or all matching (instance = 0)
    if( ncol( subSet ) == 1 || instance == 0 )
      fieldData <- cbind( fieldData, subSet )
    else
      if( instance <= ncol( subSet ) ) {
        fieldData <- cbind( fieldData, subSet[ , instance ] )
        colnames( fieldData )[ ncol( fieldData ) ] <- colnames( subSet )[ instance ]
      } else {
        stop( paste( "Error: invalid instance (", instance, ")" ) )
      }
  }

  if( ncol( fieldData ) == 0 )
    return( NULL )
  else
    return( fieldData )
}


# ==== Select a subset of a data frame (by variable attributes) ====

select.colattrs.lsd <- function( dataSet, info, col.names = NULL, init.value = NA,
                                 init.time = NA, end.time = NA, posit = NULL,
                                 posit.match = c( "fixed", "glob", "regex" ) ) {

  # test if files are compatible (in principle)
  if( ! is.matrix( dataSet ) || ! is.data.frame( info ) ||
      ncol( dataSet ) != nrow( info ) )
    stop( "Info table invalid or incompatible with provided dataSet" )

  # format valid names for matching
  if( ! is.null( col.names ) )
    col.names <- make.names( name.clean.lsd( col.names ) )

  # format position if needed
  posit <- fmt.posit( posit, match.arg( posit.match ) )

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

  return( posit )
}
