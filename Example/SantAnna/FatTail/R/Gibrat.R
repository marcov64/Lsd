########### Gibrat Law test ############

# ==== User defined parameters ====

folder   <- "MarkII/Beta"       # subfolder of working dir containing data
baseName <- "MarkII-Beta"       # data files base name
tSet <- NULL                    # time period for tests (vector of periods or NULL for all)
signif <- 0.05                  # tests significance
poolData <- FALSE               # pool all data files (T) or treat separately (F)
plotRows <- 1					          # number of plots per row in a page
plotCols <- 1					          # number of plots per column in a page
plotW <- 10                     # plot window width
plotH <- 7                      # plot window height

caseNames <- c( )               # enter custom cases names here


# ====== External support functions & definitions ======

source( "StatFuncs.R" )


# ==== MAIN SCRIPT (data processing starts here) ====

# ---- Read data files ----

readFiles <- list.files( path = folder, pattern = paste0( baseName, "_[0-9]+.res"),
                         full.names = TRUE )

dataSeries <- read.list.lsd( readFiles, "_s", instance = 0, pool = poolData )

numCases <- length( dataSeries )

# ---- Verify an/or create labels for each case (for plots) ----

numNames <- length( caseNames )

if( numNames < numCases )
  for( i in ( numNames + 1 ) : numCases )
    caseNames[i] <- paste( "Run", i )

# ---- Rename firms & mount data in panel format ----

panels <- list( )

for( i in 1 : numCases ){
  firms <- ncol( dataSeries[[ i ]] )
  periods <- nrow( dataSeries[[ i ]] )
  colnames( dataSeries[[ i ]] ) <- c( 1 : firms )
  panels[[ i ]] <- data.frame( )

  # convert size to normalized size
  for( t in 1 : periods ) {
    sAvg <- mean( dataSeries[[ i ]][ t, ], na.rm = TRUE )
    dataSeries[[ i ]][ t, ] <- dataSeries[[ i ]][ t, ] - sAvg
  }

  # create panel
  for( f in 1 : firms ) {
    z.1 <- z.2 <- NA
    for( t in 1 : periods ) {
      z <- dataSeries[[ i ]][ t, f ]
      if( ! is.na( z ) ) {
        panels[[ i ]] <- rbind( panels[[ i ]], c( f, t, z, z.1, z.2  ) )
      }
      z.2 <- z.1
      z.1 <- z
    }
  }
  colnames( panels[[ i ]] ) <- c( "Firm", "t", "z", "z.1", "z.2" )
}

# ---- Enter error handling mode so PDF can be closed in case of error/interruption ----

tryCatch({

  # ---- Open PDF plot file for output ----

  pdf( paste0( folder, "/", baseName, "_Gibrat.pdf" ),
       width = plotW, height = plotH )
  options( scipen = 3 )                 # max 3 digits
  par( mfrow = c ( plotRows, plotCols ) )             # define plots per page

  # ---- Test Gibrat law for each case ----

  test1 <- test2 <- test3 <- rej1M <- rej1m <- rej1e <- rej2M <- rej2m <- rej2e <-
    rej3M <- rej3m <- rej3e <- sig1 <- sig2 <- sig3 <- sig4 <- sig5 <- 0
  beta1V <- beta2V <- beta3V <- rho2V <- rho3V <- vector( )
  tCrit <- qnorm( 1 - signif / 2 )

  for( i in 1 : numCases ){

    try( rm( m1, m2 ), silent = TRUE )

    # fit linear model
    try( m1 <- lmrob( z ~ 0 + z.1, data = panels[[ i ]][ , 3 : 4 ], subset = tSet ),
         silent = TRUE )

    # fit nonlinear model using standard nonlinear regression algorithms
    try( m2 <- nls( z ~ ( beta + rho ) * z.1 + ( - beta * rho ) * z.2,
                    data = panels[[ i ]], start = list( beta = 1, rho = 0 ),
                    algorithm = "plinear", subset = tSet,
                    control = list( maxiter = 1000, tol = 1e-3,
                                    minFactor = 1e-16 ) ),
         silent = TRUE )
    # try other algorithms on failure
    if( ! exists( "m2" ) ) {
      # nonlinear fit using a modification of the Levenberg-Marquardt algorithm
      try( m2 <- nlsLM( z ~ ( beta + rho ) * z.1 + ( - beta * rho ) * z.2,
                        data = panels[[ i ]], start = list( beta = 1, rho = 0 ),
                        subset = tSet, control = list( maxiter = 1000, ptol = 1e-3 ) ),
           silent = TRUE )
      if( ! exists( "m2" ) ) {
        try( m2 <- nls( z ~ ( beta + rho ) * z.1 + ( - beta * rho ) * z.2,
                        data = panels[[ i ]], start = list( beta = 1, rho = 0 ),
                        subset = tSet, control = list( maxiter = 1000, tol = 1e-3,
                                                       minFactor = 1e-16 ) ),
             silent = TRUE )
        if( ! exists( "m2" ) ) {
          try( m2 <- nls( z ~ ( beta + rho ) * z.1 + ( - beta * rho ) * z.2,
                          data = panels[[ i ]], start = list( beta = 1, rho = 0 ),
                          algorithm = "port", lower = list( beta = 0, rho = 0 ),
                          upper = list( beta = 2, rho = 2 ), subset = tSet,
                          control = list( maxiter = 1000, tol = 1e-3,
                                          minFactor = 1e-16 ) ),
               silent = TRUE )
        }
      }
    }

    # Conventional autocorrelation fit (AR(1)) using feasible GLS
    res <- data.frame( list( u = resid( m1 )[ -1 ], u.1 = resid( m1 )[ - length( resid( m1 ) ) ] ) )
    try( mac <- lmrob( u ~ 0 + u.1, data = res ), silent = TRUE )
    if( exists( "mac" ) ) {
      rho.hat <- coef( mac )[[ 1 ]]
      try( m3 <- lmrob( I( z - rho.hat * z.1 ) ~ 0 + I( z.1 - rho.hat * z.2 ),
                        data = panels[[ i ]] ), silent = TRUE )
    }

    if( exists( "m1" ) ) {
      test1 <- test1 + 1
      beta <- coef( m1 )[[ 1 ]]
      se <- summary( m1 )$coefficients[ 1, 2 ]
      pval <- summary( m1 )$coefficients[ 1, 4 ]
      betaMin <- beta - tCrit * se
      betaMax <- beta + tCrit * se

      if( is.finite( pval ) && pval < signif ) {
        sig1 <- sig1 + 1
        beta1V[ sig1 ] <- beta
        if( betaMin > 1 || betaMax < 1 ) {
          rej1e <- rej1e + 1
        }
        if( betaMin > 1 ) {
          rej1m <- rej1m + 1
        } else {
          if( betaMax < 1 ) {
            rej1M <- rej1M + 1
          }
        }
      }
    }

    if( exists( "m2" ) ) {
      test2 <- test2 + 1
      beta <- summary( m2 )$parameters[ 1, 1 ]
      se <- summary( m2 )$parameters[ 1, 2 ]
      pval <- summary( m2 )$parameters[ 1, 4 ]
      betaMin <- beta - tCrit * se
      betaMax <- beta + tCrit * se
      rho <- summary( m2 )$parameters[ 2, 1 ]
      rhoSe <- summary( m2 )$parameters[ 2, 2 ]
      rhoPval <- summary( m2 )$parameters[ 2, 4 ]
      rhoMin <- rho - tCrit * rhoSe
      rhoMax <- rho + tCrit * rhoSe

      if( is.finite( pval ) && pval < signif ) {
        sig2 <- sig2 + 1
        beta2V[ sig2 ] <- beta
        if( betaMin > 1 || betaMax < 1 ) {
          rej2e <- rej2e + 1
        }
        if( betaMin > 1 ) {
          rej2m <- rej2m + 1
        } else {
          if( betaMax < 1 ) {
            rej2M <- rej2M + 1
          }
        }
      }

      if( is.finite( rhoPval ) && rhoPval < signif ) {
        sig3 <- sig3 + 1
        rho2V[ sig3 ] <- rho
      }
    }

    if( exists( "m3" ) ) {
      test3 <- test3 + 1
      beta <- coef( m3 )[[ 1 ]]
      se <- summary( m3 )$coefficients[ 1, 2 ]
      pval <- summary( m3 )$coefficients[ 1, 4 ]
      betaMin <- beta - tCrit * se
      betaMax <- beta + tCrit * se
      rho <- coef( mac )[[ 1 ]]
      rhoSe <- summary( mac )$coefficients[ 1, 2 ]
      rhoPval <- summary( mac )$coefficients[ 1, 4 ]
      rhoMin <- rho - tCrit * rhoSe
      rhoMax <- rho + tCrit * rhoSe

      if( is.finite( pval ) && pval < signif ) {
        sig4 <- sig4 + 1
        beta3V[ sig4 ] <- beta
        if( betaMin > 1 || betaMax < 1 ) {
          rej3e <- rej3e + 1
        }
        if( betaMin > 1 ) {
          rej3m <- rej3m + 1
        } else {
          if( betaMax < 1 ) {
            rej3M <- rej3M + 1
          }
        }
      }

      if( is.finite( rhoPval ) && rhoPval < signif ) {
        sig5 <- sig5 + 1
        rho3V[ sig5 ] <- rho
      }
    }
  }

  gibAnal <- matrix( c( sig1 / test1, test1, rej1e / sig1, sig1,
                        rej1M / sig1, sig1, rej1m / sig1, sig1,

                        sig3 / test2, test2, sig2 / test2, test2,
                        rej2e / sig2, sig2, rej2M / sig2, sig2,
                        rej2m / sig2, sig2,

                        sig5 / test3, test3, sig4 / test3, test3,
                        rej3e / sig4, sig4, rej3M / sig4, sig4,
                        rej3m / sig4, sig4 ),
                     ncol = 2, byrow = TRUE )

  rownames( gibAnal ) <- c( "Model 1 H0: beta = 0", "Model 1 H0: beta = 1",
                            "Model 1 H0: beta > 1", "Model 1 H0: beta < 1",
                            "Model 2 H0: rho = 0",
                            "Model 2 H0: beta = 0", "Model 2 H0: beta = 1",
                            "Model 2 H0: beta > 1", "Model 2 H0: beta < 1",
                            "Model 3 H0: rho = 0",
                            "Model 3 H0: beta = 0", "Model 3 H0: beta = 1",
                            "Model 3 H0: beta > 1", "Model 3 H0: beta < 1" )
  colnames( gibAnal ) <- c( "Rej. rate", "Runs" )

  textplot( fmt( gibAnal ), cex = 1.0 )
  title( main = "Gibrat Law tests", sub = paste( "( at", signif * 100,
                                                 "% significance / observations =",
                                                 numCases, ")" ) )

  write.csv( gibAnal, paste0( folder, "/", baseName, "_gibrat_tests.csv" ) )

  gibMC <- matrix( c( mean( beta1V ),
                      sd( beta1V, na.rm = TRUE ) / sqrt( length( beta1V[ ! is.na( beta1V ) ] ) ),
                      mean( beta2V ),
                      sd( beta2V, na.rm = TRUE ) / sqrt( length( beta2V[ ! is.na( beta2V ) ] ) ),
                      mean( rho2V ),
                      sd( rho2V, na.rm = TRUE ) / sqrt( length( rho2V[ ! is.na( rho2V ) ] ) ),
                      mean( beta3V ),
                      sd( beta3V, na.rm = TRUE ) / sqrt( length( beta3V[ ! is.na( beta3V ) ] ) ),
                      mean( rho3V ),
                      sd( rho3V, na.rm = TRUE ) / sqrt( length( rho3V[ ! is.na( rho3V ) ] ) ) ),
                   ncol = 2, byrow = TRUE )
  rownames( gibMC ) <- c( "Model 1 beta", "Model 2 beta", "Model 2 rho",
                          "Model 3 beta", "Model 3 rho" )
  colnames( gibMC ) <- c( "Mean", "S.E." )

  textplot( fmt( gibMC ), cex = 1.0 )
  title( main = "Gibrat Law coefficients", sub = paste( "( observations =",
                                                        numCases, ")" ) )

  write.csv( gibMC, paste0( folder, "/", baseName, "_gibrat_coeff.csv" ) )

  # ------------- Exception handling code (tryCatch) -------------

}, interrupt = function( ex ) {
  cat( "An interruption was detected.\n" )
  print( ex )
  textplot( "Report incomplete due to interruption." )
}, error = function( ex ) {
  cat( "An error was detected.\n" )
  print( ex )
  textplot( "Report incomplete due to processing error." )
}, finally = {
  # Close PDF plot file
  dev.off( )
})