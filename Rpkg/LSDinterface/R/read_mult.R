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

# ==== Read LSD variables from multiple runs into a 3D array ====

read.3d.lsd <- function( files, col.names = NULL, nrows = -1, skip = 0,
                         check.names = TRUE, instance = 1, nnodes = 1,
                         posit = NULL,
                         posit.match = c( "fixed", "glob", "regex" ) ) {

  # ---- Function to read data files (can be parallelized) ----

  readFile <- function( file ) {
    return( read.single.lsd( file, nrows = nrows, skip = skip,
                             col.names = col.names, check.names = check.names,
                             instance = instance, posit = posit,
                             posit.match = posit.match ) )
  }

  # ---- Read files in parallel ----

  posit.match <- match.arg( posit.match )
  n <- length( files )

  dataSet <- run.parallel( nnodes, n, files, readFile,
                           c( "nrows", "skip", "col.names", "check.names",
                              "instance", "posit", "posit.match" ) )

  # ---- Stack multiple 2D files as a 3D array ----

  for ( i in 1 : n ) {

    if( i == 1 ) {                          # don't bind if first file
      dataArray <- dataSet[[ i ]]
      nrows <- nrow( dataSet[[ i ]] )       # define base dimensions
      ncols <- ncol( dataSet[[ i ]] )
    } else {
      # check consistency
      if( nrow( dataSet[[ i ]] ) != nrows || ncol( dataSet[[ i ]] ) != ncols )
        stop( paste0( "File '", files[ i ], "' has incompatible dimensions!") )

      # 3D binding
      dataArray <- abind::abind( dataArray, dataSet[[ i ]], along = 3,
                                 use.first.dimnames = TRUE )
    }
  }

  # use results file names (no path/extensions) for dimension
  dimnames( dataArray )[[ 3 ]] <- matrix( unlist( strsplit( basename( files ),
                                                            ".", fixed = TRUE ) ),
                                          ncol = length( files ) )[ 1, ]

  return( dataArray )
}


# ==== Read LSD variables from multiple runs into a list ====

read.list.lsd <- function( files, col.names = NULL, nrows = -1, skip = 0,
                           check.names = TRUE, instance = 0, pool = FALSE,
                           nnodes = 1, posit = NULL,
                           posit.match = c( "fixed", "glob", "regex" ) ) {

  # ---- Function to read data files (can be parallelized) ----

  readFile <- function( file ) {
    return( read.raw.lsd( file, nrows = nrows, skip = skip,
                          col.names = col.names, check.names = check.names,
                          clean.names = TRUE, instance = instance,
                          posit = posit, posit.match = posit.match ) )
  }

  # ---- Read files in parallel ----

  posit.match <- match.arg( posit.match )
  n <- length( files )

  dataSet <- run.parallel( nnodes, n, files, readFile,
                           c( "nrows", "skip", "col.names", "check.names",
                              "instance", "posit", "posit.match" ) )

  # use results file names (no path/extensions) for naming list elements
  names( dataSet ) <- matrix( unlist( strsplit( basename( files ),
                                                ".", fixed = TRUE ) ),
                              ncol = length( files ) )[ 1, ]

  # ---- select aggregation mode

  if( ! pool ) {

    return( dataSet )

  } else {

    colnames( dataSet[[ 1 ]] ) <- paste0( colnames( dataSet[[ 1 ]] ), ".",
                                          names( dataSet )[ 1 ] )
    fileData <- dataSet[[ 1 ]]

    if( n > 1 )
      for( i in 2 : n ) {
        colnames( dataSet[[ i ]] ) <- paste0( colnames( dataSet[[ i ]] ), ".",
                                              names( dataSet )[ i ] )
        fileData <- cbind( fileData, dataSet[[ i ]] )
      }

    return( fileData )
  }
}


# ==== Read LSD variables from multiple runs into a 4D array ====

read.4d.lsd <- function( files, col.names = NULL, nrows = -1, skip = 0,
                         check.names = TRUE, pool = FALSE, nnodes = 1,
                         posit = NULL,
                         posit.match = c( "fixed", "glob", "regex" ) ) {

  # ---- Function to read data files (can be parallelized) ----

  readFile <- function( file ) {
    return( read.raw.lsd( file, nrows = nrows, skip = skip,
                          col.names = col.names, check.names = check.names,
                          clean.names = FALSE, instance = 0,
                          posit = posit, posit.match = posit.match ) )
  }

  # ---- Read files in parallel ----

  posit.match <- match.arg( posit.match )
  n <- length( files )
  fileNames <- matrix( unlist( strsplit( basename( files ),
                                         ".", fixed = TRUE ) ), ncol = n )[ 1, ]

  dataSet <- run.parallel( nnodes, n, files, readFile,
                           c( "nrows", "skip", "col.names", "check.names",
                              "posit", "posit.match" ) )

  # ---- Select only required data ----

  fixedLabels <- unique( unlist( lapply( dataSet,
                                         function( x )
                                           name.r.unique.lsd( colnames( x ) ) ) ) )
  m <- length( fixedLabels )
  nInst <- matrix( 0, nrow = m, ncol = n )  # number of instances per var./file
  fileData <- list( )                       # list to hold file data
  nTsteps <- 0                              # records the maximum timespan yet

  for( i in 1 : n ) {

    nTsteps <- max( nTsteps, nrow( dataSet[[ i ]] ) )  # updates max time span
    fieldData <- list( )                    # list to store each variable in file

    for( j in 1 : m ) {                     # do for all possible var columns

      subSet <- dataSet[[ i ]][ , grepl( paste0( "^", fixedLabels[ j ], "\\..*" ),
                                  colnames( dataSet[[ i ]] ) ), drop = FALSE ]

      nInst[ j, i ] <- ncol( subSet )	      # save number of instances

      if( ncol( subSet ) == 0 )
        warning( paste0( "Variable '", name.var.lsd( fixedLabels[ j ] ),
                         "' not found in '", files[ i ], "', skipping..." ),
                 call. = FALSE )

      if( pool )
        colnames( subSet ) <- paste0( colnames( subSet ), ".", fileNames[ i ] )

      instData <- list( )                   # list to store each instance
      for( k in 1 : ncol( subSet ) )        # do for all instances
        instData[[ k ]] <- subSet[ , k ]

      fieldData[[ j ]] <- instData
    }

    fileData[[ i ]] <- fieldData
  }

  rm( dataSet, subSet, fieldData, instData )

  # ---- allocate 4D array and migrate list data to it  ----

  # If in pool mode, dimension array accordingly
  if( ! pool ) {

    dimFiles <- n
    namFiles <- fileNames
    dimInst <- max( nInst )
    namInst <- c( 1 : dimInst )

    # Alocate array and apply the labels
    dataArray <- array( as.numeric( NA ), dim = c( nTsteps, length( fixedLabels ),
                                                   dimInst, dimFiles ),
                        dimnames = list( c( ( skip + 1 ) : ( skip + nTsteps ) ),
                                         fixedLabels, namInst, namFiles ) )
  } else {

    dimInst <- max( rowSums( nInst ) )      # maximum number of instances req'd
    namInst <- c( 1 : dimInst )
    l <- rep( 1, m )

    dataArray <- array( as.numeric( NA ), dim = c( nTsteps, length( fixedLabels ),
                                                   dimInst ),
                        dimnames = list( c( ( skip + 1 ) : ( skip + nTsteps ) ),
                                         fixedLabels, namInst ) )
  }

  # Copy only existing t-series (vectors), let the rest as NA
  for( i in 1 : n )                                     # do for all files
    for( j in 1 : length( fileData[[ i ]] ) )           # all found variables
      for( k in 1 : length( fileData[[ i ]][[ j ]] ) )  # and all instances
        if( ! pool )
          dataArray[ , j, k, i ] <- fileData[[ i ]][[ j ]][[ k ]]
        else {
          dataArray[ , j, l[ j ] ] <- fileData[[ i ]][[ j ]][[ k ]]
          l[ j ] <- l[ j ] + 1
        }

  return( dataArray )
}


# ==== Read LSD results files in parallel, if possible ====

run.parallel <- function( nnodes, n, files, readFile, env ) {

  if( nnodes != 1 ) {

    if( nnodes == 0 )
      nnodes <- parallel::detectCores( )

    # find the maximum useful number of cores ( <= nnodes )
    i <- 1
    while( ceiling( n / i ) > nnodes )
      i <- i + 1
    nnodes <- ceiling( n / i )

    # initiate cluster for parallel loading
    cl <- parallel::makeCluster( min( nnodes, n ), outfile = "" )

    # configure cluster: export required variables & packages
    parallel::clusterExport( cl, env, envir = environment( readFile ) )
    invisible( parallel::clusterEvalQ( cl, library( LSDinterface ) ) )

    # read files in parallel
    dataSet <- parallel::parLapply( cl, files, readFile )

    # stop the cluster
    parallel::stopCluster( cl )

    return( dataSet )

  } else {  # read files serially

    return( lapply( files, readFile ) )
  }
}
