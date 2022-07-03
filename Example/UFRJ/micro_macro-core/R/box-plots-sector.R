#******************************************************************
#
# ----------------- MMM aggregates box-plots --------------------
#
#******************************************************************

# remove warnings for support functions
# !diagnostics suppress = log0, colSds, na.remove, rec.stats, textplot


box_plots_sector <- function( mcData, nExp, nSize, TmaxStat, TmaskStat, warmUpStat,
                       nTstat, legends, legendList, sDigits, bPlotCoef,
                       bPlotNotc, folder, outDir, repName ) {

  # ======= COMPARISON OF EXPERIMENTS =======

  numStats <- 99
  statsTb <- array( dim = c( numStats, 4, nExp ) )
  statsBp <- array( dim = c( numStats, 5, nExp ) )
  n <- array( dim = c( numStats, nExp ) )
  conf <- array( dim = c( numStats, 2, nExp ) )
  out <- array( list( ), dim = c( numStats, nExp ) )
  names <- units <- list( )

  # ---- Collect the data for each experiment ----

  for( k in 1 : nExp ) {
    stat <- 0
    temp <- matrix( nrow = TmaxStat, ncol = nSize )

    stat <- stat + 1
    names[[ stat ]] <- "Consumption Sector Price Growth"
    units[[ stat ]] <- "Consumption Sector Avg. Price growth rate"
    for(j in 1 : nSize)
      for(i in TmaskStat)
        if(i == 1) {
          temp[i - warmUpStat, j ] <- 0
        }else
          temp[i - warmUpStat, j ] <- ( log0( mcData[[ k ]][i, "P_C", j ] ) - log0( mcData[[ k ]][i - 1, "P_C", j ] ) ) 
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)] ) 
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "Capital Sector Price Growth"
    units[[ stat ]] <- "Capital Sector Avg. Price growth rate"
    for(j in 1 : nSize)
      for(i in TmaskStat)
        if(i == 1) {
          temp[i - warmUpStat, j ] <- 0
        }else
          temp[i - warmUpStat, j ] <- ( log0( mcData[[ k ]][i, "P_K", j ] ) - log0( mcData[[ k ]][i - 1, "P_K", j ] ) )
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)] ) 
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "Intermediate Sector Price Growth"
    units[[ stat ]] <- "Intermediate Sector Avg. Price growth rate"
    for(j in 1 : nSize)
      for(i in TmaskStat)
        if(i == 1) {
          temp[i - warmUpStat, j ] <- 0
        }else
          temp[i - warmUpStat, j ] <- ( log0( mcData[[ k ]][i, "P_I", j ] ) - log0( mcData[[ k ]][i - 1, "P_I", j ] ) ) 
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)] ) 
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "Consumption Sector Wage Growth"
    units[[ stat ]] <- "Consumption Sector Avg. Wage growth rate"
    for(j in 1 : nSize)
      for(i in TmaskStat)
        if(i == 1) {
          temp[i - warmUpStat, j ] <- 0
        }else
          temp[i - warmUpStat, j ] <- ( log0( mcData[[ k ]][i, "W_C", j ] ) - log0( mcData[[ k ]][i - 1, "W_C", j ] ) ) 
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)] ) 
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "Capital Sector Wage Growth"
    units[[ stat ]] <- "Capital Sector Avg. Wage growth rate"
    for(j in 1 : nSize)
      for(i in TmaskStat)
        if(i == 1) {
          temp[i - warmUpStat, j ] <- 0
        }else
          temp[i - warmUpStat, j ] <- ( log0( mcData[[ k ]][i, "W_K", j ] ) - log0( mcData[[ k ]][i - 1, "W_K", j ] ) )
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)] ) 
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "Intermediate Sector Wage Growth"
    units[[ stat ]] <- "Intermediate Sector Avg. Wage growth rate"
    for(j in 1 : nSize)
      for(i in TmaskStat)
        if(i == 1) {
          temp[i - warmUpStat, j ] <- 0
        }else
          temp[i - warmUpStat, j ] <- ( log0( mcData[[ k ]][i, "W_I", j ] ) - log0( mcData[[ k ]][i - 1, "W_I", j ] ) )
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)] ) 
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    
    stat <- stat + 1
    names[[ stat ]] <- "Consumption Sector Markup Growth"
    units[[ stat ]] <- "Consumption Sector Avg. Markup growth rate"
    for(j in 1 : nSize)
      for(i in TmaskStat)
        if(i == 1) {
          temp[i - warmUpStat, j ] <- 0
        }else
          temp[i - warmUpStat, j ] <- ( log0( mcData[[ k ]][i, "MK_C", j ] ) - log0( mcData[[ k ]][i - 1, "MK_C", j ] ) ) 
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)] ) 
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "Capital Sector Markup Growth"
    units[[ stat ]] <- "Capital Sector Avg. Markup growth rate"
    for(j in 1 : nSize)
      for(i in TmaskStat)
        if(i == 1) {
          temp[i - warmUpStat, j ] <- 0
        }else
          temp[i - warmUpStat, j ] <- ( log0( mcData[[ k ]][i, "MK_K", j ] ) - log0( mcData[[ k ]][i - 1, "MK_K", j ] ) )
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)] ) 
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "Intermediate Sector Markup Growth"
    units[[ stat ]] <- "Intermediate Sector Avg. Markup growth rate"
    for(j in 1 : nSize)
      for(i in TmaskStat)
        if(i == 1) {
          temp[i - warmUpStat, j ] <- 0
        }else
          temp[i - warmUpStat, j ] <- ( log0( mcData[[ k ]][i, "MK_I", j ] ) - log0( mcData[[ k ]][i - 1, "MK_I", j ] ) )
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)] ) 
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    
    stat <- stat + 1
    names[[ stat ]] <- "Consumption Sector Productivity Growth"
    units[[ stat ]] <- "Consumption Sector Avg. Productivity growth rate"
    for(j in 1 : nSize)
      for(i in TmaskStat)
        if(i == 1) {
          temp[i - warmUpStat, j ] <- 0
        }else
          temp[i - warmUpStat, j ] <- ( log0( mcData[[ k ]][i, "PROD_C", j ] ) - log0( mcData[[ k ]][i - 1, "PROD_C", j ] ) ) 
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)] ) 
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "Capital Sector Productivity Growth"
    units[[ stat ]] <- "Capital Sector Avg. Productivity growth rate"
    for(j in 1 : nSize)
      for(i in TmaskStat)
        if(i == 1) {
          temp[i - warmUpStat, j ] <- 0
        }else
          temp[i - warmUpStat, j ] <- ( log0( mcData[[ k ]][i, "PROD_K", j ] ) - log0( mcData[[ k ]][i - 1, "PROD_K", j ] ) )
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)] ) 
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "Intermediate Sector Productivity Growth"
    units[[ stat ]] <- "Intermediate Sector Avg. Productivity growth rate"
    for(j in 1 : nSize)
      for(i in TmaskStat)
        if(i == 1) {
          temp[i - warmUpStat, j ] <- 0
        }else
          temp[i - warmUpStat, j ] <- ( log0( mcData[[ k ]][i, "PROD_I", j ] ) - log0( mcData[[ k ]][i - 1, "PROD_I", j ] ) )
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)] ) 
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "Consumption Sector Unemployment"
    units[[ stat ]] <- "Consumption Sector Unemployment"
    temp <- mcData[[ k ]][ TmaskStat, "U_C", ]
    temp[ ! is.finite( temp ) ] <- NA
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "Capital Sector Unemployment"
    units[[ stat ]] <- "Capital Sector Unemployment"
    temp <- mcData[[ k ]][ TmaskStat, "U_K", ]
    temp[ ! is.finite( temp ) ] <- NA
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "Intermediate Sector Unemployment"
    units[[ stat ]] <- "Intermediate Sector Unemployment"
    temp <- mcData[[ k ]][ TmaskStat, "U_I", ]
    temp[ ! is.finite( temp ) ] <- NA
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "Consumption Sector Debt Rate"
    units[[ stat ]] <- "Consumption Sector Avg. Debt Rate"
    temp <- mcData[[ k ]][ TmaskStat, "DEBT_C", ]
    temp[ ! is.finite( temp ) ] <- NA
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "Capital Sector Debt Rate"
    units[[ stat ]] <- "Capital Sector Avg. Debt Rate"
    temp <- mcData[[ k ]][ TmaskStat, "DEBT_K", ]
    temp[ ! is.finite( temp ) ] <- NA
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "Intermediate Debt Rate"
    units[[ stat ]] <- "Intermediate Avg. Debt Rate"
    temp <- mcData[[ k ]][ TmaskStat, "DEBT_I", ]
    temp[ ! is.finite( temp ) ] <- NA
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "Consumption Sector Inverse HHI"
    units[[ stat ]] <- "Consumption Sector Inverse HHI"
    temp <- mcData[[ k ]][ TmaskStat, "HHI_C", ]
    temp[ ! is.finite( temp ) ] <- NA
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "Capital Sector Inverse HHI"
    units[[ stat ]] <- "Capital Sector Inverse HHI"
    temp <- mcData[[ k ]][ TmaskStat, "HHI_K", ]
    temp[ ! is.finite( temp ) ] <- NA
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "Intermediate Inverse HHI"
    units[[ stat ]] <- "Intermediate  Inverse HHI"
    temp <- mcData[[ k ]][ TmaskStat, "HHI_I", ]
    temp[ ! is.finite( temp ) ] <- NA
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out

   

  }

  # remove unused stats space
  numStats <- stat
  stat <- stat + 1
  statsTb <- statsTb[ - ( stat : 99 ), , ]
  statsBp <- statsBp[ - ( stat : 99 ), , ]
  n <- n[ - ( stat : 99 ), ]
  conf <- conf[ - ( stat : 99 ), , ]
  out <- out[ - ( stat : 99 ), ]
  rm( temp, x )


  # ---- Build experiments statistics table and performance comparison chart ----

  table.stats <- statsTb[ , , 1 ]
  table.names <- c( "Avg[1]", "SD[1]", "Min[1]", "Max[1]" )
  perf.comp <- statsTb[ , 1, 1 ]
  perf.names <- c( "Baseline[1]" )

  # Print whisker plots for each statistics

  for( stat in 1 : numStats ) {

    # find max/mins for all experiments
    LowLim <- Inf
    upLim <- -Inf
    for( k in 1 : nExp ) {
      if( conf[ stat, 1, k ] < LowLim )
        lowLim <- conf[ stat, 1, k ]
      if( conf[ stat, 2, k ] > upLim )
        upLim <- conf[ stat, 2, k ]
    }
    upLim <- upLim + ( upLim - lowLim )
    lowLim <- lowLim - ( upLim - lowLim )

    # build the outliers vectors
    outVal <- outGrp <- vector( "numeric" )
    for( k in 1 : nExp ) {
      if( length( out[[ stat, k ]] ) == 0 )
        next
      outliers <- vector( "numeric" )
      for( i in 1 : length( out[[ stat, k ]] ) ) {
        if( out[[ stat, k ]][ i ] < upLim &&
            out[[ stat, k ]][ i ] > lowLim )
          outliers <- append( outliers, out[[ stat, k ]][ i ] )
      }
      if( length( outliers ) > 0 ) {
        outVal <- append( outVal, outliers )
        outGrp <- append( outGrp, rep( k, length( outliers ) ) )
      }
    }

    listBp <- list( stats = statsBp[ stat, , ], n = n[ stat, ], conf = conf[ stat, , ],
                    out = outVal, group = outGrp, names = legends )
    title <- names[[ stat ]]
    subTitle <- as.expression(bquote(paste( "( bar: median / box: 2nd-3rd quartile / whiskers: max-min / points: outliers / MC runs = ",
                                            .( nSize ), " / period = ", .( warmUpStat + 1 ), " - ",
                                            .( nTstat ), " )" ) ) )
    tryCatch( bxp( listBp, range = bPlotCoef, notch = bPlotNotc, main = title,
                   sub = subTitle, ylab = units[[ stat ]] ),
              error = function( e ) {
                warning( "In boxplot (bxp): problem while plotting: ", title, "\n\n" )
                textplot( paste( "Plot for <", title, "> failed." ) )
              } )
  }

  if( nExp > 1 ){

    # Create 2D stats table and performance comparison table

    for(k in 2 : nExp){

      # Stats table
      table.stats <- cbind( table.stats, statsTb[ , , k ] )
      table.names <- cbind( table.names, c( paste0( "Avg[", k, "]" ),
                                            paste0( "SD[", k, "]" ),
                                            paste0( "Min[", k, "]" ),
                                            paste0( "Max[", k, "]" ) ) )

      # Performance comparison table
      perf.comp <- cbind( perf.comp, statsTb[ , 1, k ] / statsTb[ , 1, 1 ] )
      t <- ( statsTb[ , 1, k ] - statsTb[ , 1, 1 ] ) /
        sqrt( ( statsTb[ , 2, k ] ^ 2 + statsTb[ , 2, 1 ] ^ 2 ) / nSize )
      df <- floor( ( ( statsTb[ , 2, k ] ^ 2 + statsTb[ , 2, 1 ] ^ 2 ) / nSize ) ^ 2 /
                     ( ( 1 / ( nSize - 1 ) ) * ( ( statsTb[ , 2, k ] ^ 2 / nSize ) ^ 2 +
                                                   ( statsTb[ , 2, 1 ] ^ 2 / nSize ) ^ 2 ) ) )
      pval <- 2 * pt( - abs ( t ), df )
      perf.comp <- cbind( perf.comp, pval )
      perf.names <- cbind( perf.names, t( c( paste0( "Ratio[", k, "]" ),
                                             paste0( "p-val[", k, "]" ) ) ) )
    }
  }

  # Print experiments table
  colnames( table.stats ) <- table.names
  rownames( table.stats ) <- names

  textplot( formatC( table.stats, digits = sDigits, format = "g" ), cmar = 1 )
  title <- paste( "Monte Carlo descriptive statistics ( all experiments )" )
  subTitle <- paste( "( numbers in brackets indicate the experiment number / MC runs =",
                     nSize, "/ period =", warmUpStat + 1, "-", nTstat, ")" )
  title( main = title, sub = subTitle )
  mtext( legendList, side = 1, line = -2, outer = TRUE )

  # Write table to the disk as CSV files for Excel
  write.csv( table.stats, quote = FALSE,
             paste0( folder, "/", outDir, "/", repName, "_exps_stats.csv" ) )

  if( nExp > 1 ) {

    # Experiments performance comparison table

    colnames( perf.comp ) <- perf.names
    rownames( perf.comp ) <- names

    textplot( formatC( perf.comp, digits = sDigits, format = "g" ), cmar = 1 )
    title <- paste( "Performance comparison ( all experiments )" )
    subTitle <- paste( "( numbers in brackets indicate the experiment number / H0: no difference with baseline / MC runs =",
                       nSize, "/ period =", warmUpStat + 1, "-", nTstat, ")" )
    title( main = title, sub = subTitle )
    mtext( legendList, side = 1, line = -2, outer = TRUE )

    # Write table to the disk as CSV files for Excel
    write.csv( perf.comp, quote = FALSE,
               paste0( folder, "/", outDir, "/", repName, "_perf_comp.csv" ) )
  }
}
