#*******************************************************************************
#
# ------------------------- Kriging-Sobol DoE analysis ------------------------
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

# Database files
folder   <- "sobol"                   # data files folder
baseName <- "Sim1-sobol"              # data files base name (same as .lsd file)
varName <- "U"                        # analysis variable name
iniDrop <- 300                        # initial time steps to drop from analysis (0=none)
nKeep <- -1                           # number of time steps to keep (-1=all)
onlyCross <- FALSE                    # use only cross validation to select model

# Force selection of specific trend model and correlation function (0=auto)
trendModel <- 0                       # trend model (1=constant / 2=1st order polynomial)
covModel <- 0                         # 1=Matern5/2 / 2=Matern3/2 / 3=Gaussian / 4=expon. /5=power exp.
# Flags for removing outliers from DoE & validation samples (use with care!)
remOutl <- FALSE                      # remove outliers from DoE & validation samples?
limOutl <- 10                         # limit deviation (number of standard deviations)

# General flags
extWgth <- 0.5                        # weight of external error measures on model selection (0-1)
conf <- 0.95                          # confidence level to use
saSamples <- 1000                     # number of samples in sensitivity analysis

# Force usage of specific factors in response surface graphs (0=auto)
factor1 <- 0                          # first factor (# according to DoE factor order)
factor2 <- 0                          # second factor
factor3 <- 0                          # third factor

# Report output configuration
plotRows <- 1   					            # number of plots per row in a page
plotCols <- 1  	 					            # number of plots per column in a page
plotW <- 10      					            # plot window width
plotH <- 7       					            # plot window height

# 3D surfaces visualization control
grid3d <- 25                          # density for 3D wireframe plots
theta3d <- 40                         # horizontal view angle
phi3d <- 30                           # vertical view angle


# ==== Aggregated variables to consider ====

# Aggregated variables to use
logVars <- c( "GDP", "EI", "C", "I", "Deb1", "Deb2", "NWb", "NW1", "NW2", "S1",
              "S2", "A", "wAvg", "G", "Gbail", "Gtrain", "Tax", "Deb", "Def",
              "Loans", "W1", "W2", "B2" )
aggrVars <- append( logVars, c( "dGDP", "dCPI", "Q2u", "F1", "F2", "entry1",
                                "entry2", "exit1", "exit2", "f2posChg", "imi",
                                "inn", "r", "U", "Ue", "TeAvg", "sTavg",
                                "sVavg", "CPI", "q2avg" ) )
newVars <- c( "freq_FE", "cris_llh", "GDP_sd" )


# ====== External support functions & definitions ======

require( LSDsensitivity, warn.conflicts = FALSE, quietly = TRUE )

if( ! exists( "plot_norm", mode = "function" ) )             # already loaded?
  source( "KS-support-functions.R" )

# remove warnings for support functions
# !diagnostics suppress = fit_subbotin, textplot, fmt, remove_outliers, log0, logNA
# !diagnostics suppress = persp3d, rgl.snapshot, rgl.close, hpfilter, notIn

# ---- Make sure selected analysis variable is available ----

if( ! ( varName %in% aggrVars ) && ! ( varName %in% newVars ) )
  stop( "Invalid variable selected" )


# ====== Function to evaluate the selected variables to 2D dataset (pool = TRUE) ======

eval.vars <- function( data, vars ) {

  if( length( notIn( vars, colnames( data ) ) ) > 0 )
    cat( "Missing variables in data:", notIn( vars, colnames( data ) ), "\n" )

  # ---- General variables ----
  tsteps <- nrow( data )
  temp <- vector( "numeric" )

  # ---- Recompute values for existing and create new variables ----

  for( var in vars ) {

    if( var %in% logVars ) {   # Consider the log values of fast growing variables

      data[ , var ] <- log0( data[ , var ] )

    } else if( var == "freq_FE" ) {   # add the frequency of crises variable to data set

      # Format full employment series (1 = full employment, 0 = otherwise)
      for( i in 1 : tsteps ) {
        if( data[ i, "U" ] == 0 ) temp[ i ] <- 1 else temp[ i ] <- 0
      }
      data[ , var ] <- mean( temp )

    } else if( var == "cris_llh" ) {   # add the crises likelihood variable to data set

      # Mark crises periods (= 1) when GDP growth is less than -3%
      temp[ 1 ] <- 0;
      for( i in 2 : tsteps ) {
        if( data[ i, "dGDP" ] < -0.03 ) temp[ i ] <- 1 else temp[ i ] <- 0
      }
      data[ , var ] <- mean( temp )

    } else if( var == "GDP_sd" ) {   # add the GDP std. dev. variable to data set

      data[ , var ] <- sd( data[ , "dGDP" ], na.rm = TRUE )
    }
  }

  return( data )
}


# ==== Process LSD result files ====

if( onlyCross ) doeNum <- 1 else doeNum <- 2

data <- read.doe.lsd( folder, baseName, varName, does = doeNum,
                      iniDrop = iniDrop, nKeep = nKeep, saveVars = aggrVars,
                      addVars = newVars, eval.vars = eval.vars,
                      rm.outl = remOutl, lim.outl = limOutl )


# ====== Select the best Kriging model ======

model <- kriging.model.lsd( data, ext.wgth = extWgth, trendModel = trendModel,
                            covModel = covModel, digits = 3 )


# ====== Sensitivity analysis for best model ======

SA <- sobol.decomposition.lsd( data, model, sa.samp = saSamples )


# ====== Generate response surface 3D data for plotting ======

defPos <- 2             # default settings position in plot set (1, 2 or 3)

response <- response.surface.lsd( data, model, SA, gridSz = grid3d, defPos = defPos,
                                  factor1 = factor1, factor2 = factor2, factor3 = factor3 )


# ===== Maximum and minimum responses for top factors =====

maxMin <- model.limits.lsd( data, model, SA )


# ====== Report generation start ======

# Open PDF plot file for output
pdf( paste0( folder, "/", baseName, "_kriging_", varName, ".pdf" ),
     width = plotW, height = plotH )
par( mfrow = c ( plotRows, plotCols ) )           # define plots per page
options( scipen = 5 )                               # max 5 digits

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


# Close PDF plot file
dev.off( )
