########### Functions to load LSD result files in R ############

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


# ==== Get the original LSD variable name from a R column name ====

name.var.lsd <- function( r.name ) {
  lsd.name <- gsub( "\\..+$", "", r.name )
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
  if( length( col.names ) == 0 )
    fixedLabels = unique( make.names( info.names.lsd( file ) ) )
  else{
    fixedLabels = col.names

    if( check.names )                 # verify validity
    {
      fixedLabels <- unique( make.names( fixedLabels ) )
      allLabels <- unique( make.names( info.names.lsd( file ) ) )

      for( i in 1 : length( fixedLabels ) )
        if( ! fixedLabels[ i ] %in% allLabels )
          stop( paste( "Error: invalid column name selected (",
                       name.var.lsd( fixedLabels[ i ] ), ")" ) )
    }
  }

  return( fixedLabels )
}

# ==== Read effective dimensions of results file (rows x columns) ====

info.dimensions.lsd <- function( file ) {

  # read from disk
  dataSet <- read.raw.lsd( file )

  # caclulate statistics
  tSteps <- nrow( dataSet )
  nVars <- ncol( dataSet )
  varNames <- name.clean.lsd( colnames( dataSet ) )

  return( list( tSteps = tSteps, nVars = nVars, varNames = varNames ) )
}


# ==== Read variable names in results file (no duplicates) ====

info.names.lsd <- function( file ) {

  # read header line (labels) from disk
  header <- scan( file, what = character( ), sep = "\t", quote = NULL,
                  nlines = 1, quiet = TRUE )
  header <- header[ 1 : ( length( header ) - 1 ) ]  # remove last tab

  if( length( header ) == 0 )            # invalid file?
    stop( paste0( "File '", file, "' is invalid!") )

  # extract labels and remove duplicates
  lsd.name <- unique( name.var.lsd( make.names( header ) ) )

  return( lsd.name )
}


# ==== Read initial conditions in results file ====

info.init.lsd <- function( file ) {

  # read from disk
  dataSet <- read.raw.lsd( file, nrows = 0 )

  return( dataSet )
}


# ==== Read  info from a results file ====

info.details.lsd <- function( file ) {

  # read from disk
  dataSet <- read.raw.lsd( file, nrows = 0 )

  # get the most "deep" object position
  maxPosit <- 1
  for( i in 1 : length( colnames( dataSet ) ) ) {
    # break position into components
    parseName <- unlist( strsplit( colnames( dataSet )[ i ],"\\." ) )
    parsePosit <- unlist( strsplit( parseName[ 2 ], "_" ) )
    maxPosit <- max( maxPosit, length( parsePosit ) )
  }

  # get variable names
  fullNames <- colnames( dataSet )
  lsdNames <- name.var.lsd( fullNames )

  # organize dataset with variable names in rows
  info <- data.frame( stringsAsFactors = FALSE )
  for( i in 1 : length( colnames( dataSet ) ) ) {
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
                           stringsAsFactors = FALSE )
    # add positions > 1
    iniCol <- ncol( newLine )
    for( j in 1 : maxPosit ) {
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


# ==== Compute statistics from multiple runs ====

info.stats.lsd <- function( array, rows = 1, cols = 2, median = FALSE,
                            ci = c( "none", "mean", "median", "auto" ),
                            ci.conf = 0.95, ci.boot = NULL, boot.R = 999,
                            na.rm = TRUE, inf.rm = TRUE ) {

  # Get dimension data
  dimArray <- dim( array )
  nDimArray <- length( dimArray )
  dimNames <- dimnames( array )
  if( nDimArray < 3 || nDimArray > 4 )
    stop( "Error: invalid array for statistics" )
  if( rows == cols || rows < 1 || rows > nDimArray ||
      cols < 1 || cols > nDimArray )
    stop( "Error: invalid dimension(s) for statistics" )

  if( rows > cols ) {                    # has to transpose at the end?
    dimH <- rows                        # make sure rows dim < cols dim
    rows <- cols
    cols <- dimH
    transp <- TRUE
  }
  else
    transp <- FALSE

  # Allocate 2D arrays
  avg <- sDev <- M <- m <-
    array( as.numeric( NA ), dim = c( dimArray[ rows ], dimArray[ cols ] ),
           dimnames = list( dimNames[[ rows ]], dimNames[[ cols ]] ) )

  if( median )
    med <- avg

  ci <- match.arg( ci )

  if( ci == "auto" ) {
    if( median )
      ci <- "median"
    else
      ci <- "mean"
  }

  if( ! is.null( ci.boot ) &&
      ! ci.boot %in% c( "basic", "perc", "bca" ) ) {
    ci.boot <- NULL
    warning( "Invalid bootstrap confidence interval type, ignoring",
             call. = FALSE )
  }

  if( ci != "none" )
    ci.lo <- ci.hi <- avg

  # prepare mask for dimension selection
  baseMask <- list( )
  for( k in 1 : nDimArray ) {
    if( rows == k || cols == k )        # dimensions to show
      baseMask[[ k ]] <- rep( FALSE, dimArray[ k ] )
    else
      baseMask[[ k ]] <- rep( TRUE, dimArray[ k ] )
  }

  # Compute averages, std. deviation etc. and store in 2D arrays
  for( j in 1 : dimArray[ cols ] ) {
    for( i in 1 : dimArray[ rows ] ) {

      # Get the appropriate vector (3D array) or matrix (4D) for analysis
      first <- TRUE
      mask <- baseMask
      for( k in 1 : nDimArray )       # adjust the mask for (i,j)
        if( rows == k || cols == k ) {
          if( first ) {
            mask[[ k ]][ i ] <- TRUE
            first <- FALSE
          }
          else
            mask[[ k ]][ j ] <- TRUE
        }
      if( nDimArray == 3 )            # handle 3D arrays
        elem <- array[ mask[[ 1 ]], mask[[ 2 ]], mask[[ 3 ]] ]
      else                            # handle 4D arrays
        elem <- array[ mask[[ 1 ]], mask[[ 2 ]], mask[[ 3 ]], mask[[ 4 ]] ]

      # calculate the statistics
      if( na.rm )
        elem <- elem[ ! is.na( elem ) ]

      if( inf.rm )
        elem <- elem[ is.finite( elem ) ]

      n = length( elem[ is.finite( elem ) ] )
      avg[ i, j ] <- mean( elem )
      sDev[ i, j ] <- stats::sd( elem )

      if( n > 0 ) {
        M[ i, j ] <- max( elem )
        m[ i, j ] <- min( elem )
      }
      else {                           # avoid Inf/-Inf when all is NA
        M[ i, j ] <- NA
        m[ i, j ] <- NA
      }

      if( median )
        med[ i, j ] <- stats::median( elem )

      if( ci != "none" ) {
        ci.lo[ i, j ] <- ci.hi[ i, j ] <- NA

        if( n > 0 ) {
          if( is.null( ci.boot ) ) {
            if( ci == "mean" ) {
              d = abs( stats::qt( ( 1 - ci.conf ) / 2, n - 1 ) ) * sDev[ i, j ] / sqrt( n )

              if( is.finite( avg[ i, j ] ) && ! is.null( d ) && is.finite( d ) ) {
                ci.lo[ i, j ] = avg[ i, j ] - d
                ci.hi[ i, j ] = avg[ i, j ] + d
              }
            } else {
              c <- NULL
              try( c <- suppressWarnings( stats::wilcox.test( elem, conf.int = TRUE,
                                                              conf.level = ci.conf,
                                                              digits.rank = 7 )$conf.int ),
                   silent = TRUE )
              if( ! is.null( c ) && is.finite( c[ 1 ] ) )
                ci.lo[ i, j ] <- c[ 1 ]

              if( ! is.null( c ) && is.finite( c[ 2 ] ) )
                ci.hi[ i, j ] <- c[ 2 ]
            }
          } else {
            if( ci == "mean" )
              f <- function( data, sel ) mean( data[ sel ] )
            else
              f <- function( data, sel ) stats::median( data[ sel ] )

            b <- c <- NULL
            try( invisible( utils::capture.output( b <- boot::boot( elem,
                                                                    statistic = f,
                                                                    R = boot.R ) ) ),
                 silent = TRUE )

            if( ! is.null( b ) ) {
              try( invisible( utils::capture.output( c <- boot::boot.ci( b,
                                                                         conf = ci.conf,
                                                                         type = ci.boot ) ) ),
                   silent = TRUE )
            }

            if( ci.boot == "perc" )
              ci.boot = "percent"       # adjust name difference in data

            if( ! is.null( c[[ ci.boot ]] ) && is.finite( c[[ ci.boot ]][ 4 ] ) )
              ci.lo[ i, j ] <- c[[ ci.boot ]][ 4 ]

            if( ! is.null( c[[ ci.boot ]] ) && is.finite( c[[ ci.boot ]][ 5 ] ) )
              ci.hi[ i, j ] <- c[[ ci.boot ]][ 5 ]
          }
        }
      }
    }
  }

  res <- list( avg = avg, sd = sDev, max = M, min = m )

  if( median )
    res[[ "med" ]] <- med

  if( ci != "none" ) {
    res[[ "ci.lo" ]] <- ci.lo
    res[[ "ci.hi" ]] <- ci.hi
  }

  if( transp )
    for( i in 1 : length( res ) )
      res[[ i ]] = t( res[[ i ]] )

  return( res )
}


# ==== Select a subset of a data frame (by column names) ====

select.colnames.lsd <- function( dataSet, col.names, instance = 0 ) {

  # matrix to store the columns, keep rownames
  fieldData <- matrix( nrow = nrow( dataSet ), ncol = 0 )
  rownames( fieldData ) <- rownames( dataSet )

  # select only required columns
  for( i in 1 : length( col.names ) ) {

    # extract one name at a time
    subSet <- dataSet[ , grep( paste0( "^", col.names[ i ], "\\." ),
                               colnames( dataSet ) ) ]

    # bind one column (instance > 0) or all matching (instance = 0)
    if( is.vector( subSet ) || instance == 0 )
      fieldData <- cbind( fieldData, subSet )
    else
      if( instance <= ncol( subSet ) )
        fieldData <- cbind( fieldData, subSet[ , instance ] )
      else
        stop( paste( "Error: invalid instance (", instance, ")" ) )

    # add column name if needed
    if( is.vector( subSet ) )
      colnames( fieldData )[ ncol( fieldData ) ] <- col.names[ i ]
    if( ! is.vector( subSet ) && instance != 0 )
      colnames( fieldData )[ ncol( fieldData ) ] <- colnames( subSet )[ instance ]
  }

  return( fieldData )
}


# ==== Select a subset of a data frame (by variable attributes) ====

select.colattrs.lsd <- function( dataSet, info, col.names = NULL, posit = NULL,
                                 init.value = NA, init.time = NA, end.time = NA ) {

  # test if files are compatible (in principle)
  if( ! is.matrix( dataSet ) || ! is.data.frame( info ) ||
      ncol( dataSet ) != nrow( info ) )
    stop( "Info table invalid or incompatible with provided dataSet" )

  # format valid names for matching
  if( ! is.null( col.names ) )
    col.names <- make.names( name.clean.lsd( col.names ) )

  # check if position is not formatted as text and convert if needed
  if( ! is.null( posit ) && ! is.character( posit ) ) {
    positChr <- paste0( posit[ 1 ] )
    for( i in 2 : length( posit ) )
      positChr <- paste0( positChr, "_", posit[ i ] )
    posit <- positChr
  }

  # matrix to store the columns, keep rownames
  fieldData <- matrix( nrow = length( rownames( dataSet ) ), ncol = 0,
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
      j <- which( colnames( info ) == "Posit_1")
      positStr <- paste0( info[ i, j ] )
      j <- j + 1
      while( j <= ncol( info ) && ! is.na( info[ i, j ] ) ) {
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

read.raw.lsd <- function( file, nrows = -1, skip = 0 ) {

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

  # adjust rown labels
  rownames( dataSet ) <- c( ( 1 + skip ) : ( nrow( dataSet ) + skip ) )

  # reformat column labels
  colnames( dataSet ) <- name.clean.lsd( header )

  return( dataSet )
}


# ==== Read LSD variables (one instance of each variable only) ====

read.single.lsd <- function( file, col.names = NULL, nrows = -1, skip = 0,
                             check.names = TRUE, instance = 1 ) {

  # ---- check column names to adjust to R imported column names ----

  fixedLabels <- name.check.lsd( file, col.names, check.names )

  if( instance <= 0 )                 # only single instance
    instance <- 1

  # ---- Read data from file and remove artifacts ----

  dataSet <- read.raw.lsd( file, nrows = nrows, skip = skip )

  # ---- remove unwanted or duplicated (multiple instances) columns ----

  return( select.colnames.lsd( dataSet, fixedLabels, instance ) )
}


# ==== Read specified LSD variables (even if there are several instances) ====

read.multi.lsd <- function( file, col.names = NULL, nrows = -1,
                            skip = 0, check.names = TRUE) {

  # ---- check column names to adjust to R imported column names ----

  fixedLabels <- name.check.lsd( file, col.names, check.names )

  # ---- Read data from file and remove artifacts ----

  dataSet <- read.raw.lsd( file, nrows = nrows, skip = skip )

  # ---- process field types ----

  fieldData <- list()                   # list to store each variable

  for( i in 1 : length( fixedLabels ) ) {

    # ---- Select only required columns ----

    fieldData[[ i ]] <- select.colnames.lsd( dataSet, fixedLabels[ i ],
                                             instance = 0 )

    if( ncol( fieldData[[ i ]] ) == 0 )
      warning( paste0( "Variable '", col.names[i],
                       "' not found, skipping..."), call. = FALSE )
  }

  return( fieldData )                   # return a list of matrices
}


# ==== Read LSD variables from multiple runs into a 3D array ====

read.3d.lsd <- function( files, col.names = NULL, nrows = -1, skip = 0,
                         check.names = TRUE, instance = 1, nnodes = 1 ) {

  # ---- check column names to adjust to R imported column names ----

  fixedLabels <- name.check.lsd( files[ 1 ], col.names, check.names )
  n <- length( files )

  if( instance <= 0 )                 # only single instance
    instance <- 1

  # ---- Function to read data files (can be parallelized) ----

  readFile <- function( file ) {

    # ---- Read data from file and remove artifacts ----

    dataSet <- read.raw.lsd( file, nrows = nrows, skip = skip )

    # ---- Select only required columns ----

    fileData <- select.colnames.lsd( dataSet, fixedLabels, instance )

    return( fileData )
  }

  # ---- Read files in parallel ----

  if( nnodes != 1 ) {

    if( nnodes == 0 )
      nnodes <- parallel::detectCores( )

    # find the maximum useful number of cores ( <= nnodes )
    i <- 1
    while( ceiling( n / i ) > nnodes )
      i <- i + 1
    nnodes <- ceiling( n / i )

    # initiate cluster for parallel loading
    cl <- parallel::makeCluster( min( nnodes, n ) )

    # configure cluster: export required variables & packages
    parallel::clusterExport( cl, c( "nrows", "skip", "fixedLabels", "instance" ),
                             envir = environment( readFile ) )
    invisible( parallel::clusterEvalQ( cl, library( LSDinterface ) ) )

    # read files in parallel
    fileData <- parallel::parLapply( cl, files, readFile )

    # stop the cluster
    parallel::stopCluster( cl )

  } else {

    # read files serially
    fileData <- lapply( files, readFile )
  }

  # ---- Stack multiple 2D files as a 3D array ----

  for ( i in 1 : n ) {

    if( i == 1 ) {                     # don't bind if first file
      dataArray <- fileData[[ i ]]
      nrows <- nrow( fileData[[ i ]] )      # define base dimensions
      ncols <- ncol( fileData[[ i ]] )
    } else {
      # check consistency
      if( nrow( fileData[[ i ]] ) != nrows || ncol( fileData[[ i ]] ) != ncols )
        stop( paste0( "File '", files[ i ], "' has incompatible dimensions!") )

      # 3D binding
      dataArray <- abind::abind( dataArray, fileData[[ i ]], along = 3,
                                 use.first.dimnames = TRUE )
    }
  }

  dimnames( dataArray )[[ 3 ]] <- files

  return( dataArray )
}


# ==== Read LSD variables from multiple runs into a list ====

read.list.lsd <- function( files, col.names = NULL, nrows = -1, skip = 0,
                           check.names = TRUE, instance = 0, pool = FALSE,
                           nnodes = 1 ) {

  # ---- check column names to adjust to R imported column names ----

  fixedLabels <- name.check.lsd( files[ 1 ], col.names, check.names )
  n <- length( files )

  # ---- Function to read data files (can be parallelized) ----

  readFile <- function( file ) {

    # ---- Read data from file and remove artifacts ----

    dataSet <- read.raw.lsd( file, nrows = nrows, skip = skip )

    # ---- Select only required columns ----

    subSet <- select.colnames.lsd( dataSet, fixedLabels, instance )

    return( subSet )
  }

  # ---- Read files in parallel ----

  if( nnodes != 1 ) {

    if( nnodes == 0 )
      nnodes <- parallel::detectCores( )

    # find the maximum useful number of cores ( <= nnodes )
    i <- 1
    while( ceiling( n / i ) > nnodes )
      i <- i + 1
    nnodes <- ceiling( n / i )

    # initiate cluster for parallel loading
    cl <- parallel::makeCluster( min( nnodes, n ) )

    # configure cluster: export required variables & packages
    parallel::clusterExport( cl, c( "nrows", "skip", "fixedLabels", "instance" ),
                             envir = environment( readFile ) )
    invisible( parallel::clusterEvalQ( cl, library( LSDinterface ) ) )

    # read files in parallel
    subSet <- parallel::parLapply( cl, files, readFile )

    # stop the cluster
    parallel::stopCluster( cl )

  } else {

    # read files serially
    subSet <- lapply( files, readFile )
  }

  # ---- select aggregation mode

  if( ! pool ) {

    return( subSet )

  } else {

    fileData <- list( )                # create list to hold data

    for( i in 1 : n ) {

      if( i == 1 )
        fileData[[ 1 ]] <- subSet[[ 1 ]]
      else {
        # bind one column (instance > 0) or all matching (instance = 0)
        if( is.vector( subSet[[ i ]] ) || instance == 0 )
          fileData[[ 1 ]] <- cbind( fileData[[ 1 ]], subSet[[ i ]] )
        else
          fileData[[ 1 ]] <- cbind( fileData[[ 1 ]], subSet[[ i ]][ , instance ] )

        # add column name if needed
        if( is.vector( subSet[[ i ]] ) && length( fixedLabels ) >= i )
          colnames( fileData[[ 1 ]] )[ ncol( fileData[[ 1 ]] ) ] <- fixedLabels[ i ]
        if( ! is.vector( subSet[[ i ]] ) && instance != 0 )
          colnames( fileData[[ 1 ]] )[ ncol( fileData[[ 1 ]] ) ] <- colnames( subSet[[ i ]] )[ instance ]
      }
    }

    return( fileData )
  }
}


# ==== Read LSD variables from multiple runs into a 4D array ====

read.4d.lsd <- function( files, col.names = NULL, nrows = -1, skip = 0,
                         check.names = TRUE, pool = FALSE, nnodes = 1 ) {

  # ---- check column names to adjust to R imported column names ----

  fixedLabels <- name.check.lsd( files[ 1 ], col.names, check.names )
  m <- length( fixedLabels )
  n <- length( files )

  # ---- Run across all files ----

  fileData <- list( )                       # list to hold file data
  nTsteps <- 0                              # records the maximum timespan yet
  # register the number of instances per variable and per file
  nInst <- matrix( 0, nrow = length( fixedLabels ), ncol = length( files ) )

  # ---- Function to read data files (can be parallelized) ----

  readFile <- function( file ) {

    # ---- Read data from file ----

    dataSet <- read.raw.lsd( file, nrows = nrows, skip = skip )

    return( dataSet )
  }

  # ---- Read files in parallel ----

  if( nnodes != 1 ) {

    if( nnodes == 0 )
      nnodes <- parallel::detectCores( )

    # find the maximum useful number of cores ( <= nnodes )
    i <- 1
    while( ceiling( n / i ) > nnodes )
      i <- i + 1
    nnodes <- ceiling( n / i )

    # initiate cluster for parallel loading
    cl <- parallel::makeCluster( min( nnodes, n ) )

    # configure cluster: export required variables & packages
    parallel::clusterExport( cl, c( "nrows", "skip" ),
                             envir = environment( readFile ) )
    invisible( parallel::clusterEvalQ( cl, library( LSDinterface ) ) )

    # read files in parallel
    dataSet <- parallel::parLapply( cl, files, readFile )

    # stop the cluster
    parallel::stopCluster( cl )

  } else {

    # read files serially
    dataSet <- lapply( files, readFile )
  }

  # ---- Select only required columns ----

  for( i in 1 : n ) {

    nTsteps <- max( nTsteps, nrow( dataSet[[ i ]] ) )  # updates max timespan

    fieldData <- list( )                  # list to store each variable

    for( j in 1 : m ) {                   # do for all selected columns

      # ---- get all instances of current column/variable ----

      subSet <- select.colnames.lsd( dataSet[[ i ]], fixedLabels[ j ], instance = 0 )

      nInst[ j, i ] <- ncol( subSet )	    # save number of instances

      if( ncol( subSet ) == 0 )
        warning( paste0( "Variable '", col.names[ j ],"' not found in '",
                         files[ i ], "', skipping..." ), call. = FALSE )

      instData <- list( )                 # list to store each instance

      for( k in 1 : ncol( subSet ) )      # do for all instances
        instData[[ k ]] <- subSet[ , k ]

      fieldData[[ j ]] <- instData
    }

    fileData[[ i ]] <- fieldData
  }

  rm( dataSet, subSet, fieldData )

  # ---- allocate 4D array and migrate list data to it  ----

  # If in pool mode, dimension array accordingly
  if( ! pool ) {
    dimFiles <- n
    namFiles <- files
    dimInst <- max( nInst )
    namInst <- c( 1 : dimInst )
  }
  else {
    dimFiles <- 1
    namFiles <- files[ 1 ]
    dimInst <- max( rowSums( nInst ) )      # maximum number of instances req'd
    namInst <- c( 1 : dimInst )
    l <- rep( 1, m )
  }

  # Alocate array and apply the labels
  dataArray <- array( as.numeric( NA ), dim = c( nTsteps, length( fixedLabels ),
                                                 dimInst, dimFiles ),
                      dimnames = list( c( ( skip + 1 ) : ( skip + nTsteps ) ),
                                       fixedLabels, namInst, namFiles ) )

  # Copy only existing t-series (vectors), let the rest as NA
  for( i in 1 : n )                                     # do for all files
    for( j in 1 : length( fileData[[ i ]] ) )           # all found variables
      for( k in 1 : length( fileData[[ i ]][[ j ]] ) )  # and all instances
        if( ! pool )
          dataArray[ , j, k, i ] <- fileData[[ i ]][[ j ]][[ k ]]
        else {
          dataArray[ , j, l[ j ], 1 ] <- fileData[[ i ]][[ j ]][[ k ]]
          l[ j ] <- l[ j ] + 1
        }

  return( dataArray )
}
