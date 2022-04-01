#*******************************************************************************
#
# ------------------- LSD tools for sensitivity analysis ---------------------
#
#   Written by Marcelo C. Pereira, University of Campinas
#
#   Copyright Marcelo C. Pereira
#   Distributed under the GNU General Public License
#
#*******************************************************************************

# ==== Create DoE response file ====

write.response <- function( folder, baseName, outVar = "", iniExp = 1, nExp = 1,
                            iniDrop = 0, nKeep = -1, pool = TRUE, instance = 1,
                            posit = NULL, posit.match = c( "fixed", "glob", "regex" ),
                            na.rm = FALSE, conf = 0.95, saveVars = c(  ),
                            addVars = c(  ), eval.vars = NULL, eval.run = NULL,
                            rm.temp = TRUE, nnodes = 1, quietly = TRUE ) {

  # evaluate new variables (not in LSD files) names
  nVarNew <- length( addVars )           # number of new variables to add
  newNameVar <- LSDinterface::name.var.lsd( append( saveVars, addVars ) )
  nVar <- length( newNameVar )          # total number of variables

  if( nVar == 0 && nVarNew == 0 )
    stop( "No variable to be kept in the data set, at least one required" )

  if( length( saveVars ) == 0 )
    saveVars <- NULL

  if( length( outVar ) == 0 )           # no output var?
    outVar <- newNameVar[ 1 ]           # use first var
  else {
    outVar <- LSDinterface::name.var.lsd( outVar )
    if( ! outVar %in% newNameVar )
      stop( paste0( "Invalid output variable selected (", outVar, ")" ) )
  }

  # check if files are in a subfolder
  myFiles <- LSDinterface::list.files.lsd( path = folder,
                                           conf.name = paste0( baseName, "_",
                                                               iniExp ) )
  if( length( myFiles ) == 0 || ! file.exists( myFiles[ 1 ] ) )
    stop( "No data files (baseName_XX_YY.res[.gz]) found on informed path" )

  folder <- dirname( myFiles[ 1 ] )

  # first check if extraction interrupted and continue with partial files
  tempFile <- paste0( folder, "/", baseName, "_", iniExp,
                      "_", iniExp + nExp - 1, ".RData" )
  if( ! rm.temp && file.exists( tempFile ) )
    tempDate <- file.mtime( tempFile )
  else
    tempDate <- 0

  # test if data files exit and are newer
  dataDate <- 0
  for( k in 1 : nExp ) {
    myFiles <- LSDinterface::list.files.lsd( path = folder,
                                             conf.name = paste0( baseName, "_",
                                                                 iniExp + k - 1 ) )
    if( tempDate == 0 && length( myFiles ) < 2 )
      stop( "Not enough data files (baseName_XX_YY.res[.gz]) found" )

    # get newest date
    for( file in myFiles ) {
      date <- file.mtime( file )
      if( date > dataDate )
        dataDate <- date
    }
  }

  # Continue with temporary file if appropriate
  noTemp <- TRUE
  if( tempDate > dataDate ) {
    temp <- new.env( )
    load( tempFile, envir = temp )

    if( all( temp$newNameVar == newNameVar ) ) {
      if( ! quietly )
        cat( "Previously processed data found, not reading data files...\n\n" )

      noTemp <- FALSE
      load( tempFile )
    }

    rm( temp )
  }

  # if no temporary data, start from zero
  if( noTemp ) {

    if( dataDate == 0 )
      stop( "No data files (baseName_XX_YY.res[.gz]) found" )

    for( k in 1 : nExp ) {

      if( ! quietly )
        cat( "\nSample #", iniExp + k - 1, "\n" )

      # ---- Read data files ----

      # Get file names
      myFiles <- LSDinterface::list.files.lsd( path = folder,
                                               conf.name = paste0( baseName, "_",
                                                                   iniExp + k - 1 ) )
      if( ! quietly )
        cat( "\nData files: ", myFiles, "\n\n" )

      # Determine the DoE sample size (repetitions on the same DoE point)
      nSize  <- length( myFiles )

      if( k == 1 )
        poolData <- array( list( ), dim = c( nExp, nVar, nSize ),
                           dimnames = list( NULL, newNameVar, NULL ) )

      # Get data set details from first file
      dimData <- LSDinterface::info.dimensions.lsd( myFiles[ 1 ] )
      nTsteps <- dimData$tSteps
      origNvar <- dimData$nVars

      if( ! quietly ) {
        cat( "Number of MC runs: ", nSize, "\n" )
        cat( "Number of periods: ", nTsteps, "\n" )
      }

      nTsteps <- nTsteps - iniDrop
      if( nKeep != -1 )
        nTsteps <- min( nKeep, nTsteps )

      if( ! quietly ) {
        cat( "Number of used periods: ", nTsteps, "\n" )
        cat( "Number of variables: ", origNvar, "\n\n" )
        cat( "Reading data from files...\n" )
      }

      nSampl <- 0

      if( pool ) {

        nInsts <- 1                                 # single instanced data

        for( j in 1 : nSize ) {

          # ------ Read data one file at a time to restrict memory usage ------

          dataSet <- LSDinterface::read.single.lsd( myFiles[ j ],
                                                    col.names = saveVars,
                                                    nrows = nKeep,
                                                    skip = iniDrop,
                                                    instance = instance,
                                                    posit = posit,
                                                    posit.match = posit.match )

          origVars <- colnames( dataSet )           # original variables

          if( length( origVars ) == 0 )
            stop( "variable, instance or position not found (saveVars/instance/posit)" )

          # ------ Add new variables to data set ------

          if( nVarNew != 0 ) {
            dataSet <- abind::abind( dataSet, array( as.numeric( NA ),
                                                     dim = c( nTsteps, nVarNew ) ),
                                     along = 2, use.first.dimnames = TRUE )
            colnames( dataSet ) <-
              LSDinterface::name.var.lsd( append( origVars, addVars ) )
          }

          # Call function to fill new variables with data or reevaluate old ones
          if( ! is.null( eval.vars ) )
            dataSet <- eval.vars( dataSet, newNameVar )

          # ---- Reorganize and save data ----

          for( i in 1 : nVar ) {
            if( ! newNameVar[ i ] %in% colnames( dataSet ) )
              stop( paste0( "Invalid variable to be saved (", newNameVar[ i ]
                            ,")" ) )

            x <- dataSet[ , newNameVar[ i ] ]
            if( na.rm )
              poolData[[ k, i, j ]] <- x[ ! is.na( x ) ]
            else
              poolData[[ k, i, j ]] <- x

            nSampl <- nSampl + length( poolData[[ k, i, j ]] )
          }
        }

      } else {

        # ------ Read data at once, may require a lot of memory ------

        dataSet <- LSDinterface::read.4d.lsd( myFiles, col.names = saveVars,
                                              nrows = nKeep, skip = iniDrop,
                                              nnodes = nnodes, posit = posit,
                                              posit.match = posit.match )

        origVars <- dimnames( dataSet )[[ 2 ]]# original variables
        nInsts <- dim( dataSet )[ 3 ]         # total number of instances

        # ------ Add new variables to data set ------

        if( nVarNew != 0 )
          dataSet <- abind::abind( dataSet, array( as.numeric( NA ),
                                                   dim = c( nTsteps, nVarNew,
                                                            nInsts, nSize ) ),
                                   along = 2, use.first.dimnames = TRUE )

        dimnames( dataSet )[[ 2 ]] <-
          LSDinterface::name.var.lsd( append( origVars, addVars ) )

        # Call function to fill new variables with data or reevaluate old ones
        if( ! is.null( eval.vars ) )
          dataSet <- eval.vars( dataSet, newNameVar )

        # ---- Reorganize and save data ----

        for( j in 1 : nSize )
          for( i in 1 : nVar ){
            if( ! newNameVar[ i ] %in% dimnames( dataSet )[[ 2 ]] )
              stop( paste0( "Invalid variable to be saved (", newNameVar[ i ]
                            ,")" ) )

            x <- as.vector( dataSet[ , newNameVar[ i ], , j ] )
            if( na.rm )
              poolData[[ k, i, j ]] <- x[ ! is.na( x ) ]
            else
              poolData[[ k, i, j ]] <- x

            nSampl <- nSampl + length( poolData[[ k, i, j ]] )
          }
      }

      if( ! quietly ) {
        cat( "\nNumber of variables: ", nVar, "\n" )
        cat( "Number of samples: ", nSampl, "\n\n" )
      }

      # Clean temp variables
      rm( dataSet, x )
    }

    # Save data list to disk
    if( ! rm.temp )
      try( save( nExp, nVar, nSize, nInsts, newNameVar, poolData,
                 compress = TRUE, file = tempFile ), silent = TRUE )
  }

  # ---- Process data in each experiment (nSize points) ----

  varIdx <- match( outVar, newNameVar )

  # Process the DoE experiments
  respAvg <- respVar <- vector( "numeric" )
  tobs <- tdiscards <- 0

  for( k in 1 : nExp ) {

    # Call function to evaluate each experiment
    if( ! is.null( eval.run ) ) {

      resp <- eval.run( poolData, run = k, varIdx = varIdx, conf = conf  )

    } else {                            # default processing - just calculate
      # the average of all runs and the MC SD
      mAc <- vAc <- obs <- 0
      for( j in 1 : nSize ) {
        data <- poolData[[ k, varIdx, j ]]
        m <- mean( data, na.rm = TRUE )
        mAc <- mAc + m
        vAc <- vAc + m ^ 2
        obs <- obs + length( data[ ! is.na( data ) ] )
      }

      # avoid negative rounding errors
      if( is.finite( vAc ) && is.finite( mAc ) ) {
        mAc <- mAc / nSize
        if( vAc / nSize < mAc ^ 2 )
          sAc <- 0
        else
          sAc <- sqrt( vAc / nSize - mAc ^ 2 )
      } else {
        mAc <- vAc <- sAc <- NA
      }

      resp <- list( mAc, sAc, obs / nSize, 0 )
      rm( data )
    }

    respAvg[ k ] <- resp[[ 1 ]]
    respVar[ k ] <- resp[[ 2 ]]
    tobs <- tobs + resp[[ 3 ]]
    tdiscards <- tdiscards + resp[[ 4 ]]
  }

  # Write table to the disk as CSV file for Excel
  tresp <- as.data.frame( cbind( respAvg, respVar ) )
  colnames( tresp ) <- c( "Mean", "Variance" )

  if( ! rm.temp ) {
    respFile <- paste0( folder, "/", baseName, "_", iniExp, "_",
                        iniExp + nExp - 1, "_", outVar, ".csv" )

    tryCatch( suppressWarnings( utils::write.csv( tresp,
                                                  respFile,
                                                  row.names = FALSE ) ),
              error = function( e )
                stop( "Cannot write DoE response file to disk (read-only?)" ) )

    if( ! quietly )
      cat( "DoE response file saved:", respFile, "\n" )
  }

  if( ! quietly ) {
    cat( "Doe points =", k, "\n" )
    cat( "Total observations =", tobs, "\n" )
    cat( "Discarded observations =", tdiscards, "\n\n" )
  }

  rm( poolData, resp )
  if( rm.temp && file.exists( tempFile ) )
    unlink( tempFile )

  return( tresp )
}
