#******************************************************************
#
# ----------------- K+S aggregates box-plots --------------------
#
#******************************************************************

# remove warnings for support functions
# !diagnostics suppress = log0, colSds, na.remove, rec.stats, textplot


box_plots <- function( mcData, nExp, nSize, TmaxStat, TmaskStat, warmUpStat,
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
    names[[ stat ]] <- "C.Sector Debt Growth"
    units[[ stat ]] <- "Average growth rate"
    temp <- mcData[[ k ]][ TmaskStat, "DEBT_C_G", ]
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)] ) 
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "K.Sector Debt Growth"
    units[[ stat ]] <- "Average growth rate"
    temp <- mcData[[ k ]][ TmaskStat, "DEBT_K_G", ]
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)] ) 
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "I.Sector Debt Growth"
    units[[ stat ]] <- "Average growth rate"
    temp <- mcData[[ k ]][ TmaskStat, "DEBT_I_G", ]
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)] ) 
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "Class A Debt Growth"
    units[[ stat ]] <- "Average growth rate"
    temp <- mcData[[ k ]][ TmaskStat, "DEBT_1_G", ]
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)] ) 
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "Class B Debt Growth"
    units[[ stat ]] <- "Average growth rate"
    temp <- mcData[[ k ]][ TmaskStat, "DEBT_2_G", ]
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)] ) 
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "Class C Debt Growth"
    units[[ stat ]] <- "Average growth rate"
    temp <- mcData[[ k ]][ TmaskStat, "DEBT_3_G", ]
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)] ) 
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "C. Sector Deposits Growth"
    units[[ stat ]] <- "Average growth rate"
    temp <- mcData[[ k ]][ TmaskStat, "DEP_C_G", ]
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)] ) 
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "K. Sector Deposits Growth"
    units[[ stat ]] <- "Average growth rate"
    temp <- mcData[[ k ]][ TmaskStat, "DEP_K_G", ]
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)] ) 
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "I. Sector Deposits Growth"
    units[[ stat ]] <- "Average growth rate"
    temp <- mcData[[ k ]][ TmaskStat, "DEP_I_G", ]
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)] ) 
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "Class A Deposits Growth"
    units[[ stat ]] <- "Average growth rate"
    temp <- mcData[[ k ]][ TmaskStat, "DEP_1_G", ]
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)] ) 
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "Class B Deposits Growth"
    units[[ stat ]] <- "Average growth rate"
    temp <- mcData[[ k ]][ TmaskStat, "DEP_2_G", ]
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)] ) 
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "Class C Deposits Growth"
    units[[ stat ]] <- "Average growth rate"
    temp <- mcData[[ k ]][ TmaskStat, "DEP_3_G", ]
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)] ) 
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "Total Debt Growth"
    units[[ stat ]] <- "Average growth rate"
    temp <- mcData[[ k ]][ TmaskStat, "DEBT_FS_G", ]
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)] ) 
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "Volatility of Total Debt Growth"
    units[[ stat ]] <- "Standard deviation of growth rate"
    temp <- mcData[[ k ]][ TmaskStat, "DEBT_FS_G", ]
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)] )
    x <- colSds( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "Total Deposits Growth"
    units[[ stat ]] <- "Average growth rate"
    temp <- mcData[[ k ]][ TmaskStat, "DEP_FS_G", ]
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)] ) 
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "Volatility of Total Deposits Growth"
    units[[ stat ]] <- "Standard deviation of growth rate"
    temp <- mcData[[ k ]][ TmaskStat, "DEP_FS_G", ]
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)] )
    x <- colSds( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "C.Sector Debt Rate"
    units[[ stat ]] <- "Average rate"
    temp <- mcData[[ k ]][ TmaskStat, "DEBT_RT_C", ]
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)] ) 
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "K.Sector Debt Rate"
    units[[ stat ]] <- "Average rate"
    temp <- mcData[[ k ]][ TmaskStat, "DEBT_RT_K", ]
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)] ) 
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "I.Sector Debt Rate"
    units[[ stat ]] <- "Average rate"
    temp <- mcData[[ k ]][ TmaskStat, "DEBT_RT_I", ]
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)] ) 
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "Class A Debt Rate"
    units[[ stat ]] <- "Average rate"
    temp <- mcData[[ k ]][ TmaskStat, "DEBT_RT_1", ]
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)] ) 
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "Class B Debt Rate"
    units[[ stat ]] <- "Average rate"
    temp <- mcData[[ k ]][ TmaskStat, "DEBT_RT_2", ]
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)] ) 
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "Class C Debt Rate"
    units[[ stat ]] <- "Average rate"
    temp <- mcData[[ k ]][ TmaskStat, "DEBT_RT_3", ]
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)] ) 
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "Firms Avg Debt Rate"
    units[[ stat ]] <- "Average rate"
    temp <- mcData[[ k ]][ TmaskStat, "DEBT_RT_FI", ]
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)] ) 
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "Volatility of Firms Avg Debt Rate"
    units[[ stat ]] <- "Standard deviation of Average rate"
    temp <- mcData[[ k ]][ TmaskStat, "DEBT_RT_FI", ]
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)] )
    x <- colSds( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "Classes Avg Debt Rate"
    units[[ stat ]] <- "Average rate"
    temp <- mcData[[ k ]][ TmaskStat, "DEBT_RT_CL", ]
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)] ) 
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "Volatility of Classes Avg Debt Rate"
    units[[ stat ]] <- "Standard deviation of Average rate"
    temp <- mcData[[ k ]][ TmaskStat, "DEBT_RT_CL", ]
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)] )
    x <- colSds( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out

    stat <- stat + 1
    names[[ stat ]] <- "Financial Sector N.HHI"
    units[[ stat ]] <- "Average rate"
    temp <- mcData[[ k ]][ TmaskStat, "FS_HHI", ]
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)] ) 
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "Volatility of Financial Sector N.HHI"
    units[[ stat ]] <- "Standard deviation of Average rate"
    temp <- mcData[[ k ]][ TmaskStat, "FS_HHI", ]
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)] )
    x <- colSds( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "Financial Sector Leverage"
    units[[ stat ]] <- "Average rate"
    temp <- mcData[[ k ]][ TmaskStat, "FS_LEV", ]
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)] ) 
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "Volatility of Financial Sector Leverage"
    units[[ stat ]] <- "Standard deviation of Average rate"
    temp <- mcData[[ k ]][ TmaskStat, "FS_LEV", ]
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)] )
    x <- colSds( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "Financial Sector Short Term Rate"
    units[[ stat ]] <- "Average rate"
    temp <- mcData[[ k ]][ TmaskStat, "FS_STR", ]
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)] ) 
    x <- colMeans( temp, na.rm = TRUE )
    bPlotStats <- boxplot.stats( x, coef = bPlotCoef )
    statsTb[ stat, , k ] <- c( mean( x ), sd( x ), min( x ), max( x ) )
    statsBp[ stat, , k ] <- bPlotStats$stats
    n[ stat, k ] <- bPlotStats$n
    conf[ stat, , k ] <- bPlotStats$conf
    out[[ stat, k ]] <- bPlotStats$out
    
    stat <- stat + 1
    names[[ stat ]] <- "Volatility of Financial Sector Short Term Rate"
    units[[ stat ]] <- "Standard deviation of Average rate"
    temp <- mcData[[ k ]][ TmaskStat, "FS_STR", ]
    temp[is.infinite(temp)] <- sign(temp[is.infinite(temp)] )
    x <- colSds( temp, na.rm = TRUE )
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
