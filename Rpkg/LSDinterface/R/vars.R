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

# ==== Get the original LSD variable name from a R column name ====

name.var.lsd <- function( r.name ) {
  lsd.name <- gsub( "\\..*$", "", r.name )
  lsd.name <- gsub( "^X_", "_", lsd.name )
  return( lsd.name )
}


# ==== Get a clean (R) variable name from R initial column name conversion ====

name.clean.lsd <- function( r.name ) {

  # adjust the time span format and remove trailing points
  clean.name <- gsub( "([0-9]+)\\.([0-9]+)\\.$", "\\1_\\2", r.name )
  clean.name <- gsub( "\\.\\.", "\\.", clean.name ) # replace double points by a single one

  return( clean.name )
}


# ==== Get a nice variable name from R initial column name conversion ====

name.nice.lsd <- function( r.name ) {

  # adjust the time span format and remove trailing points
  nice.name <- name.clean.lsd( r.name )
  # remove the 'X_' from R converted LSD variables starting with '_'
  nice.name <- gsub( "^X_|^_", "", nice.name )

  return( nice.name )
}


# ==== Check for missing or invalid column (variable) names ====

name.check.lsd <- function( file, col.names = NULL, check.names = TRUE ) {

  # if no names, get from file
  if( is.null( col.names ) || ! is.character( col.names ) )
    col.names <- unique( make.names( info.names.lsd( file ) ) )
  else
    if( check.names )
      col.names <- names.in.set( col.names, info.names.lsd( file ) )

  return( name.var.lsd( col.names ) )
}


# ==== Get a valid/unique R variable name from the original LSD variable name ====

name.r.unique.lsd <- function( r.name ) {
  r.name <- sub( "\\..*$", "", r.name )
  r.name <- make.names( r.name )
  r.name <- unique( r.name )
  return( r.name )
}


# ==== Get a valid R variable name from the original LSD variable name ====

name.r <- function( r.name ) {
  r.name <- sub( "\\..*$", "", r.name )
  r.name <- make.names( r.name )
  return( r.name )
}


# ==== check is all given variable names are in the larger set ====

names.in.set <- function( sel.names, all.names ) {

  sel.names <- unique( make.names( sel.names ) )
  all.names <- unique( sub( "\\..*$", "", make.names( all.names ) ) )

  for( i in 1 : length( sel.names ) )
    if( ! sub( "\\..*$", "", sel.names[ i ] ) %in% all.names )
      stop( paste( "Error: invalid column name selected (", sel.names[ i ], ")" ) )

  return( sel.names )
}
