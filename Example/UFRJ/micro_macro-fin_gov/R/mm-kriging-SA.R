#*******************************************************************************
#
# ---------------------------- Kriging DoE analysis ---------------------------
#
#	Several files are required to run:
#		folder/basename_XX_YY.csv		    : DoE specification from LSD
#		folder/basename_YY+1_ZZ.csv		  : external validation from LSD (optional)
#		folder/basename_XX_YY_WWW.csv 	: DoE response from R
#		folder/basename_YY+1_ZZ_WWW.csv	: ext. validation response from R (optional)
#		folder/basename.lsd			       	: LSD configuration with default values
#		folder/basename.sa		          : factor sensitivity test ranges from LSD
#
#*******************************************************************************

if( exists( "folder" ) )				        # check for previous run (interactive mode)
  rm( list = ls( all.names = TRUE ) )   # clear the R environment


# ====== User parameters ======

# Database files
folder   <- "Res_Kriging"                    # data files folder
baseName <- "Sim_1"                    # data files base name (same as .lsd file)
varName <- "PONZI"                        # analysis variable name
iniDrop <- 100                        # initial time steps to drop from analysis (0=none)
nKeep <- -1                           # number of time steps to keep (-1=all)
cores <- 0                            # maximum number of cores to allocate (0=all)
savDat <- TRUE                        # save processed data files and re-use if available?
onlyCross <- FALSE                   # use only cross validation to select model
optimPar <- FALSE                     # find optimized parameters (default: max/min)

# Force selection of specific trend model and correlation function (0=auto)
trendModel <- 0                       # trend model (1=constant / 2=1st order polynomial)
covModel <- 0                         # 1=Matern5/2 / 2=Matern3/2 / 3=Gaussian / 4=expon. /5=power exp.

# Other model specific settings
krigingSA <- FALSE                    # use kriging specific SA algorithm

# Flags for removing outliers from DoE & validation samples (use with care!)
remOutl <- FALSE                      # remove outliers from DoE & validation samples?
limOutl <- 10                         # limit deviation (number of standard deviations)

# General flags
extWgth <- 0.5                        # weight of external error measures on model selection (0-1)
conf <- 0.95                          # confidence level to use
saSamples <- 1000                     # number of samples in sensitivity analysis

# Parameters for variables creation and results evaluation
smoothing <- 1e5                      # HP filter smoothing factor (lambda)
crisisTh  <- -0.03                    # crisis growth threshold
crisisLen <- 3                        # crisis minimum duration (periods)
crisisPre <- 4                        # pre-crisis period to base trend start (>=1)

# Force usage of specific factors in response surface graphs (0=auto)
factor1 <- 0                          # first factor (# according to DoE factor order)
factor2 <- 0                          # second factor
factor3 <- 0                          # third factor

# Report output configuration
raster <- TRUE  					            # raster or vector plots
res <- 600       					            # resolution of raster mode (in dpi)
plotRows <- 1   					            # number of plots per row in a page
plotCols <- 1  	 					            # number of plots per column in a page
plotW <- 10      					            # plot window width
plotH <- 7       					            # plot window height

# 3D surfaces visualization control
grid3d <- 25                          # density for 3D wireframe plots
surf3d <- FALSE                       # ask the user to adjust a 3D perspective of response surface
theta3d <- 330 #40                         # horizontal view angle
phi3d <- 30                           # vertical view angle


# ==== Random number generator & debug

set.seed( 1, kind = "default" )       # set the random number generator
options( warn = -1 )


# ==== Log start mark ====

cat( "\nKriging DoE analysis\n====================\n" )
cat( "\n", as.character( Sys.time( ) ), "-> Start processing...\n\n" )
startTime <- proc.time( )       # register current time


# ==== Read command line parameters (if any) ====

args <- commandArgs( trailingOnly = TRUE )
cat( "Command line arguments: ", args, "\n" )

if( length ( args ) > 0 ){  # first parameter has to be the folder
  folder <- args [1]
}
if( length ( args ) > 1 ){  # second parameter has to be the base name
  baseName <- args [2]
}
if( length ( args ) > 2 ){  # third parameter has to be the variable to analyze name
  varName <- args [3]
}
if( length ( args ) > 3 ){  # fourth parameter has to be outlier removal flag
  remOutl <- as.logical( args [4] )
}
if( length ( args ) > 4 ){  # fifth parameter has to be use cross validation only
  onlyCross <- as.logical( args [5] )
}
if( length ( args ) > 5 ){  # sixth parameter has to be the raster output flag (TRUE)
  raster <- as.logical( args [6] )
}
if( length ( args ) > 6 ){  # seventh parameter has to be the initial time period ( 0 is all )
  iniDrop <- as.integer( args [7] )
}
if( length ( args ) > 7 ){  # eigth parameter has to be the end periods to remove ( -1 is all )
  nKeep <- as.integer( args [8] )
}

cat( " Folder =", folder, "\n" )
cat( " Base name =", baseName, "\n" )
cat( " Variable name =", varName, "\n" )
cat( " Remove outliers =", remOutl, "\n" )
cat( " Cross validation only =", onlyCross, "\n" )
cat( " Raster =", raster, "\n" )
cat( " Initial time steps to drop =", iniDrop, "\n" )
cat( " Time steps to keep =", nKeep, "\n\n" )


# ====== External support functions & definitions ======

require( LSDsensitivity, warn.conflicts = FALSE, quietly = TRUE )

if( ! exists( "plot_norm", mode = "function" ) )             # already loaded?
  source( "support-functions.R" )

# remove warnings for support functions
# !diagnostics suppress = fit_subbotin, textplot, fmt, remove_outliers, log0, logNA
# !diagnostics suppress = persp3d, rgl.snapshot, rgl.close, hpfilter


# ==== Aggregated variables to use ====

# Aggregated variables to use
logVars <- c( "Real_GDP", 
              "I_r", 
              "C_r", 
              "FS_PR")
aggrVars <- append( logVars, c( "GDP_G", 
                                "FS_LEV", 
                                "DEBT_RT_CL", 
                                "DEBT_RT_FI", 
                                "P_G", 
                                "INV_G", 
                                "CON_G", 
                                "DEBT_FS_G", 
                                "U",
                                "FS_STR",
                                "FS_DR",
                                "PONZI",
                                "PR"))
newVars <- c( "sdGDP",
              "sdP",
              "sdU" )

# ---- Make sure selected analysis variable is available ----

if( ! ( varName %in% aggrVars ) && ! ( varName %in% newVars ) )
  stop( "Invalid variable selected" )


# ====== Function to evaluate the selected variables to 2D dataset (pool = TRUE) ======

eval.vars <- function( data, vars ) {
  
  # ---- General variables ----
  tsteps <- nrow( data )
  #nvars <- ncol( data )
  temp <- vector( "numeric" )
  
  # ---- Reompute values for existing and create new variables ----
  
  for( var in vars ) {
    
    if( var %in% logVars ) {   # Consider the log values of fast growing variables
      
      data[ , var ] <- log0( data[ , var ] )
      
    } else if( var == "sdGDP" ) {   # add the GDP std. dev. variable to data set
      
      data[ , var ] <- sd( data[ , "Real_GDP" ], na.rm = TRUE )
      
    } else if( var == "sdP" ) {   # add the Inflation std. dev. variable to data set
      
      data[ , var ] <- sd( data[ , "P_G" ], na.rm = TRUE )
      
    } else if( var == "sdU" ) {   # add the Unemployment dev. variable to data set
      
      data[ , var ] <- sd( data[ , "U" ], na.rm = TRUE )
    }
  }
  return( data )
}


# ==== Process LSD result files ====

if( onlyCross ) doeNum <- 1 else doeNum <- 2

data <- read.doe.lsd( folder, baseName, varName, does = doeNum,
                      iniDrop = iniDrop, nKeep = nKeep, saveVars = aggrVars,
                      addVars = newVars, eval.vars = eval.vars,
                      rm.temp = ! savDat, nnodes = cores, rm.outl = remOutl,
                      lim.outl = limOutl )


# ====== Select the best Kriging model ======

cat( "Estimating Kriging models...\n")

model <- kriging.model.lsd( data, ext.wgth = extWgth, trendModel = trendModel,
                            covModel = covModel, digits = 3 )


# ====== Sensitivity analysis for best model ======

cat( "Calculating sensitivity analysis...\n\n" )

SA <- sobol.decomposition.lsd( data, model, krig.sa = krigingSA,
                               sa.samp = saSamples )


# ====== Generate response surface 3D data for plotting ======

cat( "Modeling response surface...\n\n" )

defPos <- 2             # default settings position in plot set (1, 2 or 3)

response <- response.surface.lsd( data, model, SA, gridSz = grid3d, defPos = defPos,
                                  factor1 = factor1, factor2 = factor2, factor3 = factor3 )


# ===== Maximum and minimum responses for top factors =====

maxMin <- model.limits.lsd( data, model, SA )


# ====== Report generation start ======

tryCatch({    # enter error handling mode so PDF can be closed in case of error/interruption

  # create a daily output directory
  outDir <- format( Sys.time(), "%Y-%m-%d" )
  if( ! dir.exists( paste0( folder, "/", outDir ) ) )
    dir.create( paste0( folder, "/", outDir ) )

  cat( paste( "Saving results and data to:", paste0( folder, "/", outDir ), "\n" ) )

  # Select type of output
  if( raster ){
    # Open PNG (bitmap) files for output
    png( paste0( folder, "/", outDir, "/", baseName, "_kriging_", varName, "_%d.png" ),
         width = plotW, height = plotH, units = "in", res = res )

  } else {
    # Open PDF plot file for output
    pdf( paste0( folder, "/", outDir, "/", baseName, "_kriging_", varName, ".pdf" ),
         width = plotW, height = plotH )
    par( mfrow = c ( plotRows, plotCols ) )           # define plots per page
  }
  options( scipen = 5 )                               # max 5 digits


  cat( "\nGenerating reports...\n\n")

  # ------ Model comparison table ------

  textplot( fmt( model$comparison ), rmar = 1, cmar = 1.5 )
  title( main = "Comparison of alternative kriging models",
         sub = "Q2: cross validation Q2 ( higher is better )\nRMSE/MAE/RMA: external validation RMSE/MAE/RMA ( lower is better )" )

  # ------ Model estimation table ------

  textplot( model$estimation.std, show.colnames = FALSE, cmar = 2, rmar = 1 )
  title( main = "Kriging meta-model estimation (standardized)",
         sub = paste0( "Variables rescaled to [0,1] / Average 95% CI = +/- ",
                       round( response$avgCIdev, digits = 2 ), "\nPredicted output at defaults: ",
                       varName, " = ", round( response$default$mean, digits = 2 ), ", 95% CI = [",
                       round( response$default$lower, digits = 2 ), ",",
                       round( response$default$upper, digits = 2 ),
                       "], time = [", iniDrop + 1, ",", nKeep, "]" ) )

  # ------ Save model estimation to disk ------

  write.csv( model$comparison, paste0( folder, "/", outDir, "/", baseName,
                                       "_kriging_models_", varName, ".csv" ) )
  write.csv( model$estimation.std, paste0( folder, "/", outDir, "/", baseName,
                                           "_kriging_estimation_", varName, ".csv" ) )

  # ------ Sobol sensitivity analysis table ------

  textplot( fmt( SA$sa, 3 ) )

  title( main = paste0( "Sobol decomposition indexes ( ", varName, " )" ),
         sub = paste0( " Samples = ", length( SA$metamodel$s ),
                       " / time = [", iniDrop + 1, ",", nKeep, "]" ) )

  # ------ Sobol sensitivity analysis chart ------

  barplot( t( SA$sa ), col = c( "white", "gray" ), las = 2, ylim = c( 0, 1 ),
           legend.text = c( "main effects", "interactions" ) )

  title( main = paste0( "Sobol decomposition indexes ( ", varName, " )" ),
         sub = paste0( " Samples = ", length( SA$metamodel$s ),
                       " / time = [", iniDrop + 1, ",", nKeep, "]" ),
         ylab = "Sobol Index" )

  # ------ Save Sobol SA results to disk ------

  write.csv( SA$sa, paste0( folder, "/", outDir, "/", baseName,
                            "_kriging_Sobol_", varName, ".csv" ) )

  # ------ 2D response surface ------

  # plot 2D charts for top factors (around defaut parameter configuration)
  for( i in 1 : 3 ) {
    plot( response$grid[[ i ]], response$factor[[ i ]]$mean,
          ylim = c( min( response$calib[[ defPos ]]$lower ),
                    max( response$calib[[ defPos ]]$upper ) ),
          type = "l", xlab = colnames( data$doe )[ SA$topEffect[ i ] ], ylab = varName,
          main = paste0( "Meta-model response for parameter '",
                         colnames( data$doe )[ SA$topEffect[ i ] ], "'" ),
          sub = "Dotted lines at 95% confidence interval" )
    lines( response$grid[[ i ]], response$factor[[ i ]]$lower, col = "black", lty = 2 )
    lines( response$grid[[ i ]], response$factor[[ i ]]$upper, col = "black", lty = 2 )
    points( as.numeric( data$facDef[ SA$topEffect[ i ] ] ), response$default$mean,
            col = "red", pch = 19, cex = 2 )
    points( maxMin[ SA$topEffect[ i ], ( i * 2 ) - 1 ],
            maxMin[ "response", ( i * 2 ) - 1 ],
            col = "orange", pch = 18, cex = 2 )
    points( maxMin[ SA$topEffect[ i ], i * 2 ],
            maxMin[ "response", i * 2 ],
            col = "blue", pch = 18, cex = 2 )
  }

  # ------ 3D response surface (wireframe) ------

  if( SA$topEffect[ 1 ] > SA$topEffect[ 2 ] ) {      # check if we have to swap x and y axis
    swapXY <- TRUE
  } else
    swapXY <- FALSE

  for( i in 1 : length( response$grid[[ 4 ]] ) ) {
    # plot 3D grid charts
    zMat <- matrix( response$calib[[ i ]]$mean, grid3d, grid3d, byrow = swapXY )

    #handle flat surfaces
    zlim <- range( zMat, na.rm = TRUE )
    if( zlim[ 1 ] == zlim[ 2 ] )
      if( zlim[ 1 ] != 0 ) {
        zlim <- zlim + c( - abs( zlim[ 1 ] ), abs( zlim[ 2 ] ) )
      } else
        zlim <- c( -1, 1 )

    vt <- persp( response$grid[[ 1 ]], response$grid[[ 2 ]], zMat, col = "gray90",
                 xlab = colnames( data$doe )[ SA$topEffect[ 1 ] ], zlim = zlim,
                 ylab = colnames( data$doe )[ SA$topEffect[ 2 ] ], zlab = varName,
                 theta = theta3d, phi = phi3d, ticktype = "detailed" )

    if( length( response$grid[[ 4 ]] ) == 1 ||
        ( length( response$grid[[ 4 ]] ) == 3 && i == 2 ) ) {
      points( trans3d( as.numeric( data$facDef[ SA$topEffect[ 1 ] ] ),
                       as.numeric( data$facDef[ SA$topEffect[ 2 ] ] ),
                       response$default$mean, vt ), col = "red", pch = 19, cex = 2 )
      points( trans3d( maxMin[ SA$topEffect[ 1 ], 7 ],
                       maxMin[ SA$topEffect[ 2 ], 7 ],
                       maxMin[ "response", 7 ], vt ), col = "orange", pch = 18, cex = 2 )
      points( trans3d( maxMin[ SA$topEffect[ 1 ], 8 ],
                       maxMin[ SA$topEffect[ 2 ], 8 ],
                       maxMin[ "response", 8 ], vt ), col = "blue", pch = 18, cex = 2 )
      subTitle <- paste0( "95% confidence interval: ", varName, " = [",
                          round( response$default$lower, 2 ),
                          ",", round( response$default$upper, 2 ),
                          "] at defaults (red dot)" )

      # ------ 3D response surface (shaded) ------

      if( surf3d ) {                                    # 3D GL model
        library( rgl, verbose = FALSE, quietly = TRUE )
        persp3d( response$grid[[ 1 ]], response$grid[[ 2 ]],
                 matrix( response$calib[[ i ]]$mean, grid3d, grid3d ), col = "lightgray",
                 xlab = colnames( data$doe )[ SA$topEffect[ 1 ] ],
                 ylab = colnames( data$doe )[ SA$topEffect[ 2 ] ], zlab = varName )
        readline( "Go to RGL window, resize it, adjust plot perspective and press enter when done" )
        if( raster )
          rgl.snapshot( paste0( folder, "/", outDir, "/", baseName, "_kriging_", varName,"_%d.png" ) )
        else
          rgl.snapshot( paste0( folder, "/", outDir, "/", baseName, "_kriging_3d_", varName, ".png" ) )
        rgl.close( )
      }
    }
    else
      subTitle <- paste0( "All other parameters are at default settings" )

    title( main = paste( paste( "Meta-model response surface (",
                                colnames( data$doe )[ SA$topEffect[ 3 ] ],
                                "=", response$grid[[ 4 ]][ i ], ")" ) ),
           sub = subTitle )
  }

  # ------ 3D response surface (isolevels) ------

  for( i in 1 : length( response$grid[[ 4 ]] ) ) {
    zMat <- matrix( response$calib[[ i ]]$mean, grid3d, grid3d, byrow = swapXY )
    contour( response$grid[[ 1 ]], response$grid[[ 2 ]], zMat, nlevels = 12 )
    if( length( response$grid[[ 4 ]] ) == 1 ||
        ( length( response$grid[[ 4 ]] ) == 3 && i == 2 ) ) {
      points( data$facDef[ SA$topEffect[ 1 ] ], data$facDef[ SA$topEffect[ 2 ] ],
              col = "red", pch = 19, cex = 2 )
      points( maxMin[ SA$topEffect[ 1 ], 7 ],
              maxMin[ SA$topEffect[ 2 ], 7 ],
              col = "orange", pch = 18, cex = 2 )
      points( maxMin[ SA$topEffect[ 1 ], 8 ],
              maxMin[ SA$topEffect[ 2 ], 8 ],
              col = "blue", pch = 18, cex = 2 )
      subTitle <- paste0( "95% confidence interval: ", varName, " = [",
                          round( response$default$lower, 2 ),
                          ",", round( response$default$upper, 2 ),
                          "] at defaults (red dot)" )
    } else
      subTitle <- paste0( "All other parameters are at default settings" )

    title( main = paste( "Meta-model response surface (",
                         colnames( data$doe )[ SA$topEffect[ 3 ] ], "=",
                         response$grid[[ 4 ]][ i ], ")" ),
           sub = subTitle, xlab = colnames( data$doe )[ SA$topEffect[ 1 ] ],
           ylab = colnames( data$doe )[ SA$topEffect[ 2 ] ] )
  }


  # ====== Optimize the kriging meta-model parameters ======

  if( optimPar ) {

    for( k in c( TRUE, FALSE ) ) {           # minimize and maximize

      if( k ) label <- "minimizing" else label <- "maximizing"

      opt <- model.optim.lsd( model, data, minimize = k )

      if( is.na( opt[ 1 ] ) ) {
        textplot( paste( opt[1], "\n\nThe", label, "has failed!" ) )
        next
      }

      predOpt <- model.pred.lsd( opt, model )

      print <- fmt( t( opt ) )
      nRows <- length( print )
      maxRows <- 18
      if( nRows > maxRows ) {                  # too many rows for a single column?
        nMove <- floor( nRows / 2 )
        col2.label <- names( print[ ( nRows - nMove + 1 ) : nRows, 1 ] )
        col2.value <- print[ ( nRows - nMove + 1 ) : nRows ]
        print <- print[ - ( ( nRows - nMove + 1 ) : nRows ), ]
        if( nMove < length( print ) ) {
          col2.label <- append( col2.label, "" )
          col2.value <- append( col2.value, "" )
        }
        print <- cbind( print, col2.label, col2.value )
      }

      textplot( print, show.colnames = FALSE )
      title( main = paste( "Settings for", label, "kriging meta-model" ),
             sub = paste( "Predicted output at optimal setting:",
                          varName, "=", round( predOpt$mean, digits = 2 ) ) )

      cat( "\nParameter", label, ":\n" )
      cat( " Result:", varName, "=", predOpt$mean, "\n" )
      for( i in 1 : length( opt[ 1, ] ) )
        cat( " Parameter:", colnames( data$doe )[ i ], "=", opt[ 1, i ], "\n" )

      # ------ 3D response surface using optimal parameter values ------

      center2 <- list( )
      for( i in 1 : length( colnames( data$doe ) ) ) {
        if( i == SA$topEffect[ 1 ] )
          center2[[ i ]] <- response$grid[[ 1 ]]
        else
          if( i == SA$topEffect[ 2 ] )
            center2[[ i ]] <- response$grid[[ 2 ]]
          else
            center2[[ i ]] <- as.numeric( opt[ i ] )
      }

      pred2 <- model.pred.lsd( expand.grid( structure( center2, .Names = colnames( data$doe ) ) ),
                               model )$mean

      zMat <- matrix( pred2, grid3d, grid3d, byrow = swapXY )
      #handle flat surfaces
      zlim <- range( zMat, na.rm = TRUE )
      if( zlim[ 1 ] == zlim[ 2 ] )
        if( zlim[ 1 ] != 0 ) {
          zlim <- zlim + c( - abs( zlim[ 1 ] ), abs( zlim[ 2 ] ) )
        } else
          zlim <- c( -1, 1 )

      vt2 <- persp( response$grid[[ 1 ]], response$grid[[ 2 ]], zMat, col = "gray90",
                    xlab = colnames( data$doe )[ SA$topEffect[ 1 ] ], zlim = zlim,
                    ylab = colnames( data$doe )[ SA$topEffect[ 2 ] ], zlab = varName,
                    theta = theta3d, phi = phi3d, ticktype = "detailed"  )
      points( trans3d( as.numeric( opt[ SA$topEffect[ 1 ] ] ),
                       as.numeric( opt[ SA$topEffect[ 2 ] ] ),
                       predOpt$mean, vt2 ), col = "red", pch = 19, cex = 2 )
      title( main = paste( "Response surface for", label, "settings" ),
             sub = paste0( "95% confidence interval: ", varName, " = [",
                           round( predOpt$lower, 2 ), ",",
                           round( predOpt$upper, 2 ), "] at optimum (point)" ) )

      contour( response$grid[[ 1 ]], response$grid[[ 2 ]], zMat, nlevels = 12 )
      points( opt[ SA$topEffect[ 1 ] ], opt[ SA$topEffect[ 2 ] ],
              col = "red", pch = 19, cex = 2 )
      title( main = paste( "Response surface for", label, "settings" ),
             sub = paste0( "95% confidence interval: ", varName, " = [",
                           round( predOpt$lower, 2 ), ",",
                           round( predOpt$upper, 2 ), "] at optimum (red dot)" ),
             xlab = colnames( data$doe )[ SA$topEffect[ 1 ] ],
             ylab = colnames( data$doe )[ SA$topEffect[ 2 ] ] )
    }

    # ------ Save optimization results to disk ------

    write.csv( maxMin, paste0( folder, "/", outDir, "/", baseName,
                               "_kriging_max-min_", varName, ".csv" ) )
  }


  cat( "\nDone...\n" )

#******************************************************************
#
# ------------- Exception handling code (tryCatch) -------------
#
#******************************************************************

}, interrupt = function( ex ) {
  cat( "An interrupt was detected.\n" )
  print( ex )
  textplot( "Report incomplete due to interrupt." )
}, error = function( ex ) {
  cat("An error was detected.\n")
  print( ex )
  textplot("Report incomplete due to processing error.")
}, finally = {
  options( warn = 0 )
  cat( "\n", as.character( Sys.time( ) ), "-> Releasing resources...\n\n" )
  totalTime <- proc.time( ) - startTime
  print( totalTime )
  # Close PDF plot file
  dev.off( )
} )

