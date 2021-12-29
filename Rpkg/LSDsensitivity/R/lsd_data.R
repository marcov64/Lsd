#*******************************************************************************
#
# ------------------- LSD tools for sensitivity analysis ---------------------
#
#   Written by Marcelo Pereira, University of Campinas
#
#   Copyright Marcelo Pereira
#   Distributed under the GNU General Public License
#
#*******************************************************************************

# ==== Read and pre-process DoE and response files ====

read.doe.lsd <- function( folder, baseName, outVar, does = 1, doeFile = NULL,
                          respFile = NULL, validFile = NULL, valRespFile = NULL,
                          confFile = NULL, limFile = NULL, iniDrop = 0,
                          nKeep = -1, saveVars = c(  ), addVars = c(  ),
                          eval.vars = NULL, eval.run = NULL, pool = TRUE,
                          na.rm = FALSE, rm.temp = TRUE, rm.outl = FALSE,
                          lim.outl = 10, nnodes = 1, quietly = TRUE ) {

  # ---- Process LSD result files ----

  # get available DoE and response file names
  does.found <- files.doe( folder, baseName )
  files <- does.found$files
  folder <- does.found$path
  if( is.null( doeFile ) ) {
    if( length( files ) > does )
      warning( "Too many DoE (.csv) files, using first one(s) only",
               call. = FALSE )
    if( length( files ) < 1 )
      stop( "No valid DoE file" )

    doeFile <- paste0( folder, "/", files[ 1 ], ".csv" )
  }

  if( is.null( respFile ) ) {
    if( length( files ) < 1 )
      stop( "No valid response file" )
    respFile <- paste0( folder, "/", files[ 1 ], "_", outVar, ".csv" )
  }

  if( does > 1 ) {
    if( is.null( validFile ) ) {
      if( length( files ) < 2 )
        stop( "No valid DoE validation file" )
      validFile <- paste0( folder, "/", files[ 2 ], ".csv" )
    }
    if( is.null( valRespFile ) ) {
      if( length( files ) < 2 )
        stop( "No valid DoE validation response file" )
      valRespFile <- paste0( folder, "/", files[ 2 ], "_", outVar, ".csv" )
    }
  }

  # read response files, if they don't exist, try to create them
  if( rm.temp || ! file.exists( respFile ) ) {
    resp <- write.response( folder, baseName, outVar = outVar,
                            iniDrop = iniDrop, nKeep = nKeep, rm.temp = rm.temp,
                            iniExp = size.doe( doeFile )[ 1 ], na.rm = na.rm,
                            nExp = size.doe( doeFile )[ 2 ], addVars = addVars,
                            pool = pool, eval.vars = eval.vars,
                            eval.run = eval.run, saveVars = saveVars,
                            nnodes = nnodes, quietly = quietly )
  } else {
    resp <- utils::read.csv( respFile )
    if( ! quietly )
      cat( paste0( "Using existing response file (", respFile, ")...\n\n" ) )
  }

  if( does > 1 && ( rm.temp || ! file.exists( valRespFile ) ) ) {
    valResp <- write.response( folder, baseName, outVar = outVar,
                               iniDrop = iniDrop, nKeep = nKeep,
                               rm.temp = rm.temp, na.rm = na.rm,
                               iniExp = size.doe( validFile )[ 1 ],
                               nExp = size.doe( validFile )[ 2 ],
                               addVars = addVars, eval.vars = eval.vars,
                               pool = pool, eval.run = eval.run,
                               saveVars = saveVars, nnodes = nnodes,
                               quietly = quietly )
  } else {
    if( does > 1 ) {
      valResp <- utils::read.csv( valRespFile )
      if( ! quietly )
        cat( paste0( "Using existing validation response file (",
                     valRespFile, ")...\n\n" ) )
    } else
      valResp <- NULL
  }

  # read design of experiments and external validation experiments definitions
  doe <- utils::read.csv( doeFile )
  if( does > 1 ) {
    valid <- utils::read.csv( validFile )
  } else
    valid <- NULL

  # read LSD default parameter configuration from base .lsd file
  if( is.null( confFile ) ) {
    config <- read.config( folder = folder, baseName = baseName )
  } else
    config <- read.config( fileName = confFile )

  # read LSD parameter limits file and join with default configuration
  if( is.null( limFile ) ) {
    limits <- read.sens( folder = folder, baseName = baseName )
  } else
    limits <- read.sens( fileName = limFile )

  limits$Def <- NA                          # add new column to param table
  for( i in 1 : nrow( limits ) ) {
    j <- which( config$Par == limits$Par[ i ], arr.ind = TRUE )
    if( length( j ) == 1 )                  # param/var is in limits df?
      limits$Def[ i ] <- config$Val[ j ]
  }

  # ---- Preprocess data ----

  # Create min/max/def lists for parameters in the same order as the DoE
  facLim <- list( )
  facLimLo <- facLimUp <- facDef <- vector( "numeric" )
  for( i in 1 : length( colnames( doe ) ) ) {
    j <- which( limits$Par == colnames( doe )[ i ], arr.ind = TRUE )
    if( j == 0 )
      stop( "Parameter not found in LSD sensitivity file" )
    facLim[[ i ]] <- list( min = limits$Min[ j ], max = limits$Max[ j ] )
    facLimLo[ i ] <- limits$Min[ j ]
    facLimUp[ i ] <- limits$Max[ j ]
    facDef[ i ] <- limits$Def[ j ]
  }
  names( facLim ) <- names( facLimLo ) <- names( facLimUp ) <- names( facDef ) <-
    colnames( doe )

  # Remove outliers, if appropriate
  if( rm.outl ) {
    clean <- remove.outliers( doe, resp, limit = lim.outl )
    doe <- clean[[ 1 ]]
    resp <- clean[[ 2 ]]

    if( does > 1 ) {
      clean <- remove.outliers( valid, valResp, limit = lim.outl )
      valid <- clean[[ 1 ]]
      valResp <- clean[[ 2 ]]
    }
  }

  doe <- list( doe = doe, resp = resp, valid = valid, valResp = valResp,
               facLim = facLim, facLimLo = facLimLo, facLimUp = facLimUp,
               facDef = facDef, saVarName = outVar )
  class( doe ) <- "lsd-doe"

  return( doe )
}


# ==== Get LSD Design of Experiments (DoE) files names without the .csv extension ====

files.doe <- function( folder, baseName ) {

  doeFiles <- LSDinterface::list.files.lsd( path = folder, conf.name = baseName,
                                            sensitivity = TRUE, type = "csv",
                                            compressed = FALSE )

  if( length( doeFiles ) < 1 )
    stop( "Valid DoE .csv file(s) required" )

  folder <- dirname( doeFiles[ 1 ] )

  for( i in 1 : length( doeFiles ) )
    doeFiles[ i ] <- sub( ".csv$", "", basename( doeFiles[ i ] ), ignore.case = TRUE )

  return( list( path = folder, files = doeFiles ) )
}


# ==== Read LSD Design of Experiments (DoE) size ====

size.doe <- function( doeFile ) {

  # Get basename and remove extension if present
  baseName <- sub( ".csv$", "", basename( doeFile ), ignore.case = TRUE )

  # First file must be the a DoE (baseName_xx_yy)
  split <- strsplit( baseName, "_" )[[ 1 ]]

  # Check invalid format
  if( length( split ) < 3 ||
      is.na( as.integer( split[ length( split ) ] ) ) ||
      is.na( as.integer( split[ length( split ) - 1 ] ) ) )
    stop( "Invalid DoE .csv file naming/numbering (must be baseName_XX_YY)" )

  doe <- c( as.integer( split[ length( split ) - 1 ] ),
            as.integer( split[ length( split ) ] ) -
              as.integer( split[ length( split ) - 1 ] ) + 1 )
  names( doe ) <- c( "ini", "n" )
  if( doe[ "n" ] < 1 )
    stop( "Invalid DoE .csv file numbering (must have at least 1 sampling point)" )

  return( doe )
}

# ==== Create DoE response file ====

write.response <- function( folder, baseName, iniExp = 1, nExp = 1, outVar = "",
                            pool = TRUE, iniDrop = 0, nKeep = -1, na.rm = FALSE,
                            conf = 0.95, saveVars = c(  ), addVars = c(  ),
                            eval.vars = NULL, eval.run = NULL, rm.temp = TRUE,
                            nnodes = 1, quietly = TRUE ) {

  # evaluate new variables (not in LSD files) names
  nVarNew <- length( addVars )           # number of new variables to add
  newNameVar <- append( LSDinterface::name.nice.lsd( saveVars ), addVars ) # new label set
  nVar <- length( newNameVar )          # total number of variables

  if( nVar == 0 && nVarNew == 0 )
    stop( "No variable to be bept in the data set, at least one required" )

  # check if files are in a subfolder
  myFiles <- LSDinterface::list.files.lsd( path = folder,
                                           conf.name = paste0( baseName, "_", iniExp ) )
  if( length( myFiles ) == 0 || ! file.exists( myFiles[ 1 ] ) )
    stop( "No data files  (baseName_XX_YY.res[.gz]) found on informed path" )

  folder <- dirname( myFiles[ 1 ] )

  # first check if extraction was interrupted and continue with partial files if appropriate
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
        cat( "Number of variable instances: ", origNvar, "\n\n" )
        cat( "Reading data from files...\n" )
      }

      if( pool ) {

        # Create simpler array, pooling all non-NA values from a single run
        if( k == 1 )                  # do just once
          poolData <- array( list( ), dim = c( nExp, nVar, nSize ),
                             dimnames = list( NULL, newNameVar, NULL ) )
        nSampl <- 0

        # Read data one file at a time to restrict memory usage
        for( j in 1 : nSize ) {

          dataSet <- LSDinterface::read.raw.lsd( myFiles[ j ], nrows = nKeep,
                                                 skip = iniDrop )

          if( nVarNew != 0 ) {
            # Increase array size to allow for new variables
            oldNameVar <- colnames( dataSet )
            dataSet <- abind::abind( dataSet, array( as.numeric(NA),
                                                     dim = c( nTsteps, nVarNew ) ),
                                     along = 2, use.first.dimnames = TRUE )
            colnames( dataSet ) <- append( LSDinterface::name.var.lsd( oldNameVar ),
                                           addVars )
          } else {
            colnames( dataSet ) <- LSDinterface::name.var.lsd( colnames( dataSet ) )
          }

          # Call function to fill new variables with data or reevaluate old ones
          if( ! is.null( eval.vars ) )
            dataSet <- eval.vars( dataSet, newNameVar )

          # Extract the requested variables (columns)
          for( i in 1 : nVar ) {
            if( ! make.names( newNameVar[ i ] ) %in% colnames( dataSet ) )
              stop( "Invalid variable to be saved (not in LSD data)" )

            x <- dataSet[ , make.names( newNameVar[ i ] ) ]
            if( na.rm ) {
              poolData[[ k, i, j ]] <- x[ ! is.na( x ) ]  # remove NAs
            } else
              poolData[[ k, i, j ]] <- x

            nSampl <- nSampl + length( poolData[[ k, i, j ]] )
          }
        }

        nInsts <- 1                     # single instanced after pooling

      } else {

        if( length( saveVars ) == 0 )
          saveVars <- NULL

        # Read data from text files and format it as 4D array with labels
        dataSet <- LSDinterface::read.4d.lsd( myFiles, col.names = saveVars,
                                              nrows = nKeep, skip = iniDrop,
                                              nnodes = nnodes )
        nInsts <- dim( dataSet )[ 3 ]         # total number of instances

        # ------ Add new variables to data set ------

        # Increase array size to allow for new variables
        dataSet <- abind::abind( dataSet, array( as.numeric(NA),
                                                 dim = c( nTsteps, nVarNew,
                                                          nInsts, nSize ) ),
                                 along = 2, use.first.dimnames = TRUE )
        dimnames( dataSet )[[ 2 ]] <- newNameVar

        # Call function to fill new variables with data or reevaluate old ones
        if( ! is.null( eval.vars ) )
          dataSet <- eval.vars( dataSet, newNameVar )

        # ---- Reorganize and save data ----

        # Create simpler array, pooling all non-NA values from a single run
        if( k == 1 )                    # do just once
          poolData <- array( list( ), dim= c( nExp, nVar, nSize ),
                             dimnames = list( NULL, newNameVar, NULL ) )
        nSampl<- 0

        for( j in 1 : nSize )
          for( i in 1 : nVar ){
            x <- as.vector( dataSet[ , newNameVar[ i ], , j ] )
            if( na.rm ) {
              poolData[[ k, i, j ]] <- x[ ! is.na( x ) ]  # remove NAs
            } else
              poolData[[ k, i, j ]] <- x

            nSampl <- nSampl + length( poolData[[ k, i, j ]] )
          }
      }

      if( ! quietly ) {
        cat( "\nNumber of variables selected: ", nVar, "\n" )
        cat( "Number of pooled samples: ", nSampl, "\n\n" )
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

  if( length( outVar ) == 0 ) {         # no output var?
    outVar <- newNameVar[ 1 ]           # use first var
    varIdx <- 1
  } else
    if( outVar %in% newNameVar ) {
      varIdx <- match( outVar, newNameVar )
    } else
      stop( "Invalid output variable selected" )

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
      for( i in 1 : nSize ) {
        data <- poolData[[ k, varIdx, i ]]
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


# ==== Read LSD default parameter configuration from base .lsd file ====

read.config <- function( folder = NULL, baseName = NULL, fileName = NULL ) {

  if( is.null( fileName ) && is.null( baseName ) )
    stop( "LSD configuration file name missing" )

  if( is.null( fileName ) )
    file <- paste0( baseName, ".lsd" )
  else
    file <- fileName

  if( ! is.null( folder ) && file.exists( folder ) )
    dir <- normalizePath( folder, winslash = "/", mustWork = TRUE )
  else
    dir <- getwd( )

  par <- dirname( dir )

  if( file.exists( paste0( dir, "/", file ) ) ) {
    file <- paste0( dir, "/", file )
  } else {
    if( file.exists( paste0( par, "/", file ) ) ) {
      file <- paste0( par, "/", file )
    } else {
      stop( "LSD configuration file does not exist" )
    }
  }

  lsd <- readLines( file )
  config <- data.frame( stringsAsFactors = FALSE )
  i <- 1
  while( lsd[ i ] != "DATA" )
    i <- i + 1                # jump lines until DATA section

  while( lsd[ i ] != "DESCRIPTION" ) {
    tok <- unlist( strsplit( lsd[ i ], split = " " ) )
    if( length( tok ) > 0 && ( tok[ 1 ] == "Param:" || tok[ 1 ] == "Var:" ) ) {
      endtok <- unlist( strsplit( tok[ length( tok ) ], split = "\t" ) )
      if( length( endtok ) > 1 ) {    # there is a value
        param <- as.data.frame( list( tok[2], as.numeric( endtok[ 2 ] ) ),
                                stringsAsFactors = FALSE )
        colnames( param ) <- c( "Par", "Val" )
        config <- rbind( config, param )
      }
    }
    i <- i + 1                # jump lines until DESCRIPTION section
  }

  return( config )
}


# ==== Read LSD parameter limits file and check consistency and number order ====

read.sens <- function( folder = NULL, baseName = NULL, fileName = NULL ) {

  if( is.null( fileName ) && is.null( baseName ) )
    stop( "LSD sensitivity file name (or parts) missing" )

  if( is.null( fileName ) )
    file <- paste0( baseName, ".sa" )
  else
    file <- fileName

  if( ! is.null( folder ) && file.exists( folder ) )
    dir <- normalizePath( folder, winslash = "/", mustWork = TRUE )
  else
    dir <- getwd( )

  par <- dirname( dir )

  if( file.exists( paste0( dir, "/", file ) ) ) {
    file <- paste0( dir, "/", file )
  } else {
    if( file.exists( paste0( dir, "/", file, "n" ) ) ) {    # accept .san extension (CRAN bug)
      file <- paste0( dir, "/", file, "n" )
    } else {
      if( file.exists( paste0( par, "/", file ) ) ) {
        file <- paste0( par, "/", file )
      } else {
        if( file.exists( paste0( par, "/", file, "n" ) ) ) {
          file <- paste0( par, "/", file, "n" )
        } else {
          stop( "LSD sensitivity file does not exist" )
        }
      }
    }
  }

  limits <- utils::read.table( file, stringsAsFactors = FALSE )
  limits <- limits[ -2 : -3 ]
  if( ! is.numeric( limits[ 1, 2 ] ) )  # handle newer LSD versions that bring an extra column
    limits <- limits[ -2 ]
  if( length( limits[ 1, ] ) > 3 ) {
    warning( "Too many (>2) sensitivity values for a single parameter, using the first two only!",
             call. = FALSE )
    limits <- limits[ -4 : - length( limits[ 1, ] ) ]
  }

  dimnames( limits )[[ 2 ]] <- c( "Par", "Min", "Max" )
  for( i in 1 : length( limits[ , 1 ] ) )
    if( limits[ i, "Min" ] > limits[ i, "Max" ] ) {
      temp <- limits[ i, "Min" ]
      limits[ i, "Min" ] <- limits[ i, "Max" ]
      limits[ i, "Max" ] <- temp
    }

  return( limits )
}
