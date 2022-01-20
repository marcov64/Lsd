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

# ==== Find LSD results files inside a sub-directory tree ====

list.files.lsd <- function( path = ".", conf.name = "",
                            type = c( "res", "tot", "csv" ),
                            compressed = NULL, recursive = FALSE,
                            join = FALSE, full.names = FALSE,
                            sensitivity = FALSE ) {

  conf.name <- basename( trimws( conf.name ) )

  path <- trimws( path )
  if( file.exists( path ) && ! dir.exists( path ) ) {
    if( conf.name == "" )
      conf.name <- basename( path )
    path <- dirname( path )
  }

  if( ! dir.exists( path ) )
    stop( paste( "Invalid base directory path (", path, ")" ) )

  conf.name <- sub( ".lsd$", "", conf.name, ignore.case = TRUE )

  if( recursive )
    dirs <- list.dirs( path = path, recursive = TRUE, full.names = TRUE )
  else {
    dirs <- c( )
    if( conf.name != "" && dir.exists( paste0( path, "/", conf.name ) ) )
      dirs <- append( dirs, paste0( path, "/", conf.name ) )

    conf.base <- sub( "_[0-9]+$", "", conf.name )
    if( conf.base != conf.name && dir.exists( paste0( path, "/", conf.base ) ) )
      dirs <- append( dirs, paste0( path, "/", conf.base ) )

    dirs <- append( dirs, path )
  }

  if( length( dirs ) == 0 )
    stop( paste( "Invalid base directory path (", normalizePath( path ), ")" ) )

  if( conf.name == "" )
    pattern <- ".+"
  else
    pattern <- conf.name

  if( sensitivity )
    pattern <- paste0( pattern, "_[0-9]+_[0-9]+[.]", match.arg( type ) )
  else
    pattern <- paste0( pattern, "_[0-9]+[.]", match.arg( type ) )

  if( is.null( compressed ) )
    pattern <- paste0( pattern, "([.]gz)?$" )
  else
    if( compressed )
      pattern <- paste0( pattern, "[.]gz$" )
  else
    pattern <- paste0( pattern, "$" )

  res_dirs <- list( )
  n <- 0
  for( dir in dirs ) {
    files <- list.files( path = dir, pattern = pattern,
                         ignore.case = TRUE, full.names = TRUE )
    if( length( files ) > 0 ) {
      n <- n + 1
      if( full.names )
        files <- normalizePath( files, winslash = "/" )

      res_dirs[[ n ]] <- list( dir = dir, files = files )
    }
  }

  if( n == 0 ) {
    warning( paste( "No files found under target path (", normalizePath( path ),
                    ") and name pattern (", pattern, ")" ), call. = FALSE )
    return( NULL )
  }

  if( ! join && n > 1 ) {
    dirs_list <- ""
    for( dir in res_dirs )
      if( dirs_list != "" )
        dirs_list <- paste0( dirs_list, ", ", normalizePath( dir$dir ) )
    else
      dirs_list <- normalizePath( dir$dir )

    warning( paste( "Multiple paths contain files (", dirs_list,
                    "), using first one only" ), call. = FALSE )
  }

  if( join ) {
    files <- vector( mode = "character" )
    for( dir in res_dirs )
      files <- append( files, dir$files )

    return( files )
  } else
    return( res_dirs[[ 1 ]]$files )
}
