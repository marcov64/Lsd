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

# ==== Do sensitivity analysis of a fitted model ====

sobol.decomposition.lsd <- function( data, model = NULL, krig.sa = FALSE, sa.samp = 1000 ) {

  if( is.null( model ) )
    out <- data.sensitivity( data )
  else
    if( class( model ) == "kriging-model" )
      out <- kriging.sensitivity( data, model, krig.sa = krig.sa, sa.samp = sa.samp )
    else
      out <- polynomial.sensitivity( data, model, sa.samp = sa.samp )

    return( out )
}


# ==== Perform sensitivity analysis directly over data ====

data.sensitivity <- function( data, tries = 5 ) {

  # ---- Sensitivity analysis using a B-spline smoothing interpolation model ----

  metamodel <- try( sensitivity::sobolSmthSpl( as.matrix( data$resp$Mean ), data$doe ),
                    silent = TRUE )

  # try a few times, as it usually succeeds...
  while( class( metamodel ) == "try-error" && tries > 0 ) {
    metamodel <- try( sensitivity::sobolSmthSpl( as.matrix( data$resp$Mean ), data$doe ),
                      silent = TRUE )
    tries <- tries - 1
    if( class( metamodel ) != "try-error" )
      break
  }

  if( class( metamodel ) == "try-error" )
    return( NULL )

  mainEffect <- function( x ) x$S[ , 1 ]

  # algorithm provide only the main effects, so distribute the indirect effects evenly (approx.)
  totalEffect <- ( 1 - sum( mainEffect( metamodel ) ) )
  sa <- cbind( mainEffect( metamodel ),
               mainEffect( metamodel ) * totalEffect / sum( mainEffect( metamodel ) ) )

  rownames( sa ) <- colnames( data$doe )
  colnames( sa ) <- c( "Direct effects", "Interactions" )

  max.index <- function( x, pos = 1 )
    as.integer( sapply( sort( x, index.return = TRUE ), `[`, length( x ) - pos + 1 )[ 2 ] )

  topEffect <- c( max.index( mainEffect( metamodel ), 1 ), max.index( mainEffect( metamodel ), 2 ),
                  max.index( mainEffect( metamodel ), 3 ) )

  cat( "Top parameters influencing response surface:\n" )
  cat( " First:", colnames( data$doe )[ topEffect[ 1 ] ], "\n" )
  cat( " Second:", colnames( data$doe )[ topEffect[ 2 ] ], "\n" )
  cat( " Third:", colnames( data$doe )[ topEffect[ 3 ] ], "\n\n" )

  sa <- list( metamodel = metamodel, sa = sa, topEffect = topEffect )
  class( sa ) <- "b-spline-sa"

  return( sa )
}
