#******************************************************************
#
# ----------------- K+S aggregates box-plots --------------------
#
#******************************************************************

# remove warnings for support functions
# !diagnostics suppress = log0, colSds, na.remove, rec.stats, textplot


box_plots <- function( mcData, nExp, nSize, TmaxStat, TmaskStat, warmUpStat,
                       nTstat, legends, legendList, sDigits, bPlotCoef,
                       bPlotNotc, folder, repName ) {

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
    names[[ stat ]] <- "GDP growth"
    units[[ stat ]] <- "Average GDP growth rate"
    # Calculates periodic GDP growth rates for each MC series
    for(j in 1 : nSize)
      for(i in TmaskStat)
        if(i == 1) {
          temp[i - warmUpStat, j ] <- 0
        }else
          temp[i - warmUpStat, j ] <- ( log0( mcData[[ k ]][i, "GDP", j ] ) -
                                       log0( mcData[[ k ]][i - 1, "GDP", j ] ) )
    # Remove +/-infinite values and replace by +/-1
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)] )
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out

    stat <- stat + 1
    names[[ stat ]] <- "Volatility of GDP growth"
    units[[ stat ]] <- "Standard deviation of GDP growth rate"
    # Calculates periodic GDP growth rates for each MC series
    for(j in 1 : nSize)
      for(i in TmaskStat)
        if(i == 1) {
          temp[i - warmUpStat, j ] <- 0
        } else
          temp[i - warmUpStat, j ] <- ( log0( mcData[[ k ]][i, "GDP", j ] ) -
                                       log0( mcData[[ k ]][i - 1, "GDP", j ] ) )
    # Remove +/-infinite values and replace by +/-1
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)] )
    x <- colSds( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out

    stat <- stat + 1
    names[[ stat ]] <- "Likelihood of GDP crises"
    units[[ stat ]] <- "Likelihood (probability) of GDP crises"
    # Mark crises periods (= 1) when GDP growth is less than -3%
    for(j in 1 : nSize){
      for(i in TmaskStat){
        if(i == 1){
          temp[i - warmUpStat, j ] <- 0
        }
        else{
          if( log0( mcData[[ k ]][i, "GDP", j ] ) -
             log0( mcData[[ k ]][i - 1, "GDP", j ] ) < -0.03){
            temp[i - warmUpStat, j ] <- 1
          }
          else{
            temp[i - warmUpStat, j ] <- 0
          }
        }
      }
    }
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out

    stat <- stat + 1
    names[[ stat ]] <- "Inflation"
    units[[ stat ]] <- "Consumer prices index average growth rate"
    temp <- mcData[[ k ]][ TmaskStat, "dCPI", ]
    temp[ ! is.finite( temp ) ] <- NA
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out

    stat <- stat + 1
    names[[ stat ]] <- "Tax"
    units[[ stat ]] <- "Government tax income over GDP"
    temp <- mcData[[ k ]][ TmaskStat, "Tax", ] / mcData[[ k ]][ TmaskStat, "GDP", ]
    temp[ ! is.finite( temp ) ] <- NA
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out

    stat <- stat + 1
    names[[ stat ]] <- "Government total expenditure"
    units[[ stat ]] <- "Total government expenditure over GDP"
    temp <- mcData[[ k ]][ TmaskStat, "G", ] / mcData[[ k ]][ TmaskStat, "GDP", ]
    temp[ ! is.finite( temp ) ] <- NA
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out

    stat <- stat + 1
    names[[ stat ]] <- "Government deficit"
    units[[ stat ]] <- "Government deficit over GDP"
    temp <- mcData[[ k ]][ TmaskStat, "Def", ] / mcData[[ k ]][ TmaskStat, "GDP", ]
    temp[ ! is.finite( temp ) ] <- NA
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out

    stat <- stat + 1
    names[[ stat ]] <- "Government debt"
    units[[ stat ]] <- "Government debt over GDP"
    temp <- mcData[[ k ]][ TmaskStat, "Deb", ] / mcData[[ k ]][ TmaskStat, "GDP", ]
    temp[ ! is.finite( temp ) ] <- NA
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out

    stat <- stat + 1
    names[[ stat ]] <- "Credit supply"
    units[[ stat ]] <- "Total banks credit supply over GDP"
    temp <- mcData[[ k ]][ TmaskStat, "TC", ] / mcData[[ k ]][ TmaskStat, "GDP", ]
    temp[ ! is.finite( temp ) ] <- NA
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out

    stat <- stat + 1
    names[[ stat ]] <- "Loans"
    units[[ stat ]] <- "Total banks loans over GDP"
    temp <- mcData[[ k ]][ TmaskStat, "Loans", ] / mcData[[ k ]][ TmaskStat, "GDP", ]
    temp[ ! is.finite( temp ) ] <- NA
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out

    stat <- stat + 1
    names[[ stat ]] <- "Capacity utilization"
    units[[ stat ]] <- "Average capacity utilization rate in sector 2"
    temp <- mcData[[ k ]][ TmaskStat, "Q2u", ]
    temp[ ! is.finite( temp ) ] <- NA
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out

    stat <- stat + 1
    names[[ stat ]] <- "Full employment frequency"
    units[[ stat ]] <- "Frequency (probability) of full employment"
    # Format full employment MC series (1 = full employment, 0 = otherwise)
    for(j in 1 : nSize)
      for(i in TmaskStat)
        if(mcData[[ k ]][i, "U", j ] == 0) {
          temp[i - warmUpStat, j ] <- 1
        } else
          temp[i - warmUpStat, j ] <- 0
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out

    stat <- stat + 1
    names[[ stat ]] <- "Unemployment"
    units[[ stat ]] <- "Average unemployment rate"
    temp <- mcData[[ k ]][ TmaskStat, "U", ]
    temp[ ! is.finite( temp ) ] <- NA
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out

    stat <- stat + 1
    names[[ stat ]] <- "Vacancy"
    units[[ stat ]] <- "Average net vacancy rate"
    temp <- mcData[[ k ]][ TmaskStat, "V", ]
    temp[ ! is.finite( temp ) ] <- NA
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out

    stat <- stat + 1
    names[[ stat ]] <- "Workers tenure"
    units[[ stat ]] <- "Average employment time in same firm (periods)"
    temp <- mcData[[ k ]][ TmaskStat, "TeAvg", ]
    temp[ ! is.finite( temp ) ] <- NA
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out

    stat <- stat + 1
    names[[ stat ]] <- "Workers tenure skills"
    units[[ stat ]] <- "Average workers skills level"
    temp <- mcData[[ k ]][ TmaskStat, "sTavg", ]
    temp[ ! is.finite( temp ) ] <- NA
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out

    stat <- stat + 1
    names[[ stat ]] <- "Workers vintage skills"
    units[[ stat ]] <- "Average workers skills level"
    temp <- mcData[[ k ]][ TmaskStat, "sVavg", ]
    temp[ ! is.finite( temp ) ] <- NA
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out

    stat <- stat + 1
    names[[ stat ]] <- "Wages spread"
    units[[ stat ]] <- "Standard deviation of log wages"
    temp <- mcData[[ k ]][ TmaskStat, "wLogSD", ]
    temp[ ! is.finite( temp ) ] <- NA
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out

    stat <- stat + 1
    names[[ stat ]] <- "Bonus to wage ratio"
    units[[ stat ]] <- "Average bonuses over wages in sector 2"
    temp <- mcData[[ k ]][ TmaskStat, "B2", ] / mcData[[ k ]][ TmaskStat, "W2", ]
    temp[ ! is.finite( temp ) ] <- NA
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out

    stat <- stat + 1
    names[[ stat ]] <- "Income concentration"
    units[[ stat ]] <- "Workers' income Gini index"
    temp <- mcData[[ k ]][ TmaskStat, "wGini", ]
    temp[ ! is.finite( temp ) ] <- NA
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out

    stat <- stat + 1
    names[[ stat ]] <- "Productivity growth"
    units[[ stat ]] <- "Average productivity growth rate"
    # Calculates periodic productivity growth rates for each MC series
    for(j in 1 : nSize)
      for(i in TmaskStat)
        if(i == 1) {
          temp[i - warmUpStat, j ] <- 0
        } else
          temp[i - warmUpStat, j ] <- ( log0( mcData[[ k ]][i, "A", j ] ) -
                                       log0( mcData[[ k ]][i - 1, "A", j ] ) )
    # Remove +/-infinite values and replace by +/-1
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)] )
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out

    stat <- stat + 1
    names[[ stat ]] <- "Productivity spread"
    units[[ stat ]] <- "Standard deviation of log productivity in sector 2"
    x <- mcData[[ k ]][ TmaskStat, "f2posChg", ]
    temp <- ( 1 - x ) * mcData[[ k ]][ TmaskStat, "A2sdPreChg", ] +
      x * mcData[[ k ]][ TmaskStat, "A2sdPosChg", ]
    temp[ ! is.finite( temp ) ] <- NA
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out

    stat <- stat + 1
    names[[ stat ]] <- "Quality"
    units[[ stat ]] <- "Weighted average product quality in sector 2"
    temp <- mcData[[ k ]][ TmaskStat, "q2avg", ]
    temp[ ! is.finite( temp ) ] <- NA
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out

    stat <- stat + 1
    names[[ stat ]] <- "Innovation"
    units[[ stat ]] <- "Innovating firms share in sector 1"
    temp <- mcData[[ k ]][ TmaskStat, "inn", ]
    temp[ ! is.finite( temp ) ] <- NA
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out

    stat <- stat + 1
    names[[ stat ]] <- "Imitation"
    units[[ stat ]] <- "Imitating firms share in sector 1"
    temp <- mcData[[ k ]][ TmaskStat, "imi", ]
    temp[ ! is.finite( temp ) ] <- NA
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out

    stat <- stat + 1
    names[[ stat ]] <- "Market concentration"
    units[[ stat ]] <- "Stadardized Herfindahl-Hirschman index in sector 2"
    temp <- mcData[[ k ]][ TmaskStat, "HH2", ]
    temp[ ! is.finite( temp ) ] <- NA
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out

    stat <- stat + 1
    names[[ stat ]] <- "Mark-ups"
    units[[ stat ]] <- "Weighted average mark-up in sector 2"
    temp <- mcData[[ k ]][ TmaskStat, "mu2avg", ]
    temp[ ! is.finite( temp ) ] <- NA
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out

    stat <- stat + 1
    names[[ stat ]] <- "Net entry of firms"
    units[[ stat ]] <- "Net entries in both markets"
    temp <- mcData[[ k ]][ TmaskStat, "entry1", ] - mcData[[ k ]][ TmaskStat, "exit1", ] +
      mcData[[ k ]][ TmaskStat, "entry2", ] - mcData[[ k ]][ TmaskStat, "exit2", ]
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
  statsTb <- statsTb[ - ( stat : 99 ), , , drop = FALSE ]
  statsBp <- statsBp[ - ( stat : 99 ), , , drop = FALSE ]
  n <- n[ - ( stat : 99 ), , drop = FALSE ]
  conf <- conf[ - ( stat : 99 ), , , drop = FALSE ]
  out <- out[ - ( stat : 99 ), , drop = FALSE ]
  rm( temp, x )


  # ---- Build experiments statistics table and performance comparison chart ----

  table.stats <- statsTb[ , , 1 ]
  table.names <- c( "Avg[1]", "SD[1]", "Min[1]", "Max[1]" )
  perf.comp <- statsTb[ , 1, 1 ]
  perf.names <- c( "Baseline[1]" )

  # Print whisker plots for each statistics

  for( stat in 1 : numStats ) {

    # find max/mins for all experiments
    lowLim <- Inf
    upLim <- -Inf
    for( k in 1 : nExp ) {
      if( conf[ stat, 1, k ] < lowLim )
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

    if( nExp > 1 )
      listBp <- list( stats = statsBp[ stat, , ], n = n[ stat, ], conf = conf[ stat, , ],
                      out = outVal, group = outGrp, names = legends )
    else
      listBp <- list( stats = matrix( statsBp[ stat, , ] ),
                      n = matrix( n[ stat, ] ), conf = matrix( conf[ stat, , ] ),
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

    for( k in 2 : nExp ){

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
  }
}
