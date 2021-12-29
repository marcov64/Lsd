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

# ==== Perform Elementary Effects sensitivity analysis ====

elementary.effects.lsd <- function( data, p = 4, jump = 2 ) {

  # ---- Sensitivity analysis for the EE model ----

  if( nrow( data$doe ) < ( ncol( data$doe ) + 1 ) ||
      nrow( data$doe ) %% ( ncol( data$doe ) + 1 ) != 0 )
    stop( "Invalid DoE size for Elementary Effects analysis" )

  # Create object of class "morris" to use standard sensitivity package
  sa <- list( model = NULL, factors = colnames( data$doe ), samples = nrow( data$doe ),
              r = nrow( data$doe ) / ( ncol( data$doe ) + 1 ),
              design = list( type = "oat", levels = p, grid.jump = jump ),
              binf = data$facLimLo, bsup = data$facLimUp, scale = TRUE,
              call = match.call( ) )
  class( sa ) <- "morris"

  # Scale all factors to the range [0,1]
  Binf <- matrix( sa$binf, nrow = nrow( data$doe ), ncol = length( sa$binf ), byrow = TRUE )
  Bsup <- matrix( sa$bsup, nrow = nrow( data$doe ), ncol = length( sa$bsup ), byrow = TRUE )
  sa$X <- ( data$doe - Binf ) / ( Bsup - Binf )
  sa$y <- data$resp$Mean
  rm( Binf, Bsup )

  # Call elementary effects analysis from sensitivity package
  sa$ee <- ee.oat( sa$X, sa$y )

  # change the class to lsd print/plot equivalents
  class( sa ) <- "morris.lsd"

  # add the standard error to the statistics
  sa$table <- as.data.frame( print( sa ) )
  sa$table$se <- sa$table$sigma / sqrt( sa$r )
  sa$table$p.value <- stats::pt( sa$table$mu.star / sa$table$se, df = sa$r - 1, lower.tail = FALSE )

  return( sa )
}
