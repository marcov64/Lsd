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

# ==== Read and pre-process DoE and response files ====

read.doe.lsd <- function( folder, baseName, outVar = "", does = 1, doeFile = NULL,
                          respFile = NULL, validFile = NULL, valRespFile = NULL,
                          confFile = NULL, limFile = NULL, iniDrop = 0,
                          nKeep = -1, saveVars = c(  ), addVars = c(  ),
                          eval.vars = NULL, eval.run = NULL, pool = TRUE,
                          na.rm = FALSE, rm.temp = TRUE, rm.outl = FALSE,
                          lim.outl = 10, nnodes = 1, quietly = TRUE,
                          instance = 1, posit = NULL,
                          posit.match = c( "fixed", "glob", "regex" ) ) {

  if( is.null( folder ) || ! is.character( folder ) )
    stop( "Invalid base path to LSD files (folder)" )

  if( is.null( baseName ) || ! is.character( baseName ) || baseName == "" )
    stop( "Invalid LSD data file base name (baseName)" )

  if( is.null( does ) || ! is.finite( does ) || round( does ) < 1 ||
      round( does ) > 2 )
    stop( "Invalid number of experiments/DoE's (does)" )

  if( ! is.null( doeFile ) && ! is.character( doeFile ) )
    stop( "Invalid DoE specification file (doeFile)" )

  if( ! is.null( respFile ) && ! is.character( respFile ) )
    stop( "Invalid DoE response file (respFile)" )

  if( ! is.null( validFile ) && ! is.character( validFile ) )
    stop( "Invalid external validation DoE specification file (validFile)" )

  if( ! is.null( valRespFile ) && ! is.character( valRespFile ) )
    stop( "Invalid external validation DoE response file (valRespFile)" )

  if( ! is.null( confFile ) && ! is.character( confFile ) )
    stop( "Invalid LSD baseline configuration file (confFile)" )

  if( ! is.null( limFile ) && ! is.character( limFile ) )
    stop( "Invalid factor limit ranges file (limFile)" )

  if( is.null( instance ) || ! is.finite( instance ) || round( instance ) < 1 )
    stop( "Invalid variable instance (instance)" )

  does      <- round( does )
  instance  <- round( instance )

  # ---- Process LSD result files ----

  if( folder == "" )
    folder <- "."

  baseName <- sub( "\\.lsd$", "", baseName, ignore.case = TRUE )

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

  if( ! file.exists( doeFile ) )
    stop( "DoE specification file does not exist (", doeFile, ")" )

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

    if( ! file.exists( validFile ) )
      stop( "Validation specification file does not exist (", validFile, ")" )

    if( is.null( valRespFile ) ) {
      if( length( files ) < 2 )
        stop( "No valid DoE validation response file" )
      valRespFile <- paste0( folder, "/", files[ 2 ], "_", outVar, ".csv" )
    }
  }

  if( outVar != "" && ! outVar %in% saveVars && ! outVar %in% addVars )
    saveVars <- outVar

  # read response files, if they don't exist, try to create them
  if( rm.temp || ! file.exists( respFile ) ) {
    resp <- write.response( folder, baseName, outVar = outVar,
                            iniDrop = iniDrop, nKeep = nKeep, rm.temp = rm.temp,
                            iniExp = size.doe( doeFile )[ 1 ], posit = posit,
                            posit.match = match.arg( posit.match ), na.rm = na.rm,
                            nExp = size.doe( doeFile )[ 2 ], addVars = addVars,
                            pool = pool, instance = instance, eval.vars = eval.vars,
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
                               nExp = size.doe( validFile )[ 2 ], posit = posit,
                               posit.match = match.arg( posit.match ),
                               addVars = addVars, eval.vars = eval.vars,
                               pool = pool, instance = instance,
                               eval.run = eval.run, saveVars = saveVars,
                               nnodes = nnodes, quietly = quietly )
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
  doe <- read.doe( doeFile, instance )
  if( does > 1 ) {
    valid <- read.doe( validFile, instance )
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

    if( length( j ) == 0 )
      stop( "Corrupt LSD sensitivity file (parameter not found)" )

    if( length( j ) != 1 )
      stop( "Corrupt LSD sensitivity file (duplicated parameter)" )

    effInst <- instance %% limits$Inst[ j ]
    if( effInst == 0 )
      effInst <- limits$Inst[ j ]

    m <- which( colnames( limits ) == paste0( "Min.", effInst ) )
    M <- which( colnames( limits ) == paste0( "Max.", effInst ) )

    facLim[[ i ]] <- list( min = limits[ j, m ], max = limits[ j, M ] )
    facLimLo[ i ] <- limits[ j, m ]
    facLimUp[ i ] <- limits[ j, M ]
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

  doeList <- list( doe = doe, resp = resp, valid = valid, valResp = valResp,
                   facLim = facLim, facLimLo = facLimLo, facLimUp = facLimUp,
                   facDef = facDef, saVarName = outVar )
  class( doeList ) <- "doe.lsd"

  return( doeList )
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
    doeFiles[ i ] <- sub( "\\.csv$", "", basename( doeFiles[ i ] ),
                          ignore.case = TRUE )

  return( list( path = folder, files = doeFiles ) )
}


# ==== Read LSD Design of Experiments (DoE) size ====

size.doe <- function( doeFile ) {

  # Get basename and remove extension if present
  baseName <- sub( "\\.csv$", "", basename( doeFile ), ignore.case = TRUE )

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


# ==== Read LSD DoE specification file for selected instance ====

read.doe <- function( fileName, instance ) {

  if( ! file.exists( fileName ) )
    stop( "DoE file does not exist (", fileName, ")" )

  doe <- utils::read.csv( fileName )

  split <- strsplit( colnames( doe ), ".", fixed = TRUE )

  names <- cols <- c( )
  for( i in 1 : length( split ) ) {
    if( length( split[[ i ]] ) == 1 || split[[ i ]][ 2 ] == 1 ) {
      names <- append( names, split[[ i ]][ 1 ] )
      if( instance == 1 )
        cols <- append( cols, i )
    } else {
      if( as.numeric( split[[ i ]][ 2 ] ) == instance )
        cols <- append( cols, i )
    }
  }

  if( length( cols ) != length( names ) )
    stop( "Selected instance is not available in DoE specification file (instance)" )

  doe <- doe[ cols ]
  colnames( doe ) <- names

  return( doe )
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

  limits <- utils::read.table( file, stringsAsFactors = FALSE, fill = TRUE )
  limits <- limits[ -2 : -3 ]
  if( ! is.numeric( limits[ 1, 2 ] ) )  # handle newer LSD versions bringing extra col
    limits <- limits[ -2 ]

  if( ( ncol( limits ) - 1 ) %% 2 > 0 ) {
    warning( "Unused sensitivity values for parameter(s), discarding last one(s)",
             call. = FALSE )
    limits <- limits[ - ncol( limits ) ]
  }

  tit <- inst <- c( )

  for( i in 1 : ( ( ncol( limits ) - 1 ) / 2 ) )
    tit <- append( tit, c( paste0( "Min.", i ), paste0( "Max.", i ) ) )

  for( i in 1 : nrow( limits ) ) {

    for( j in seq( 2, ncol( limits ), 2 ) ) {
      if( is.na( limits[ i, j ] ) || is.na( limits[ i, j + 1 ] ) ) {
        j <- j - 2
        break
      }

      if( limits[ i, j ] > limits[ i, j + 1 ] ) {
        temp <- limits[ i, j ]
        limits[ i, j ] <- limits[ i, j + 1 ]
        limits[ i, j + 1 ] <- temp
      }
    }

    inst[ i ] <- j / 2
  }

  limits <- cbind( limits, inst )
  colnames( limits ) <- c( "Par", tit, "Inst" )

  return( limits )
}
