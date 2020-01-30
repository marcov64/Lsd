#******************************************************************
#
# --------------- K+S sector 2 pool analysis --------------------
#
#******************************************************************

#******************************************************************
#
# ------------ Read Monte Carlo experiment files ----------------
#
#******************************************************************

folder   <- "data"                    # data files folder
baseName <- "Sim"                     # data files base name (same as .lsd file)
nExp <- 2                             # number of experiments
iniDrop <- 0                          # initial time steps to drop from analysis (0=none)
nKeep <- -1                           # number of time steps to keep (-1=all)

expVal <- c( "Fordist", "Competitive" )   # case parameter values

# Firm-level variables to use
firmVar <- c( "_A2", "_Q2e", "_w2realAvg", "_s2avg", "_postChg" )
addFirmVar <- c( "normO2", "lnormO2", "lnormO2match", "lnormO2matchLag", "normA2",
                 "normRW2", "normSK2", "normO2grow", "lnormO2grow", "normA2grow",
                 "normRW2grow", "normSK2grow", "normApreChg2", "normApostChg2",
                 "normRWpreChg2", "normRWpostChg2", "growth2" )
sector <- "( Consumption-goods sector )"


# ==== Process LSD result files ====

# Package with LSD interface functions
library( LSDinterface, verbose = FALSE, quietly = TRUE )
library( abind, verbose = FALSE, quietly = TRUE )

# remove warnings for saved data
# !diagnostics suppress = pool, nSize, nTsteps, nFirms

# ---- Read data files ----

# Function to read one experiment data (to be parallelized)
readExp <- function( exper ) {
  if( nExp > 1 ) {
    myFiles <- list.files( path = folder, pattern = paste0( baseName, exper, "_[0-9]+.res"),
                          full.names = TRUE )
  } else {
    myFiles <- list.files( path = folder, pattern = paste0( baseName, "_[0-9]+.res"),
                          full.names = TRUE )
  }

  if( length( myFiles ) < 1 )
    stop( "Data files not found. Check 'folder', 'baseName' and 'nExp' parameters." )

  cat( "Data files: ", myFiles, "\n" )

  # Read data from text files and format it as 4D array with labels
  mc <- read.4d.lsd( myFiles, col.names = firmVar, skip = iniDrop,
                     nrows= nKeep, nnodes = 1 )

  # Get dimensions information
  nTsteps <- dim( mc )[ 1 ]              # number of time steps
  nFirms <- dim( mc )[ 3 ]               # number of firms (instances)
  nSize  <- dim( mc )[ 4 ]               # Monte Carlo sample size

  # Add new variables names (not in LSD files)
  nFirmVarNew <- length( addFirmVar )   # number of new variables to add
  newFirmVar <- append( name.nice.lsd( firmVar ), addFirmVar ) # new label set
  nVar <- length( newFirmVar )          # number of variables (firm-level)

  # ------ Add new variables to data set ------

  # Increase array size to allow for new variables
  mc <- abind( mc, array( as.numeric( NA ),
                          dim = c( nTsteps, nFirmVarNew,
                                   nFirms, nSize ) ),
               along = 2, use.first.dimnames = TRUE )
  dimnames( mc )[[ 2 ]] <- newFirmVar

  # Compute values for new variables, preventing infinite values
  for( m in 1 : nSize )                 # for all MC samples (files)
    for( i in 1 : nTsteps ){            # all time steps
      # clean size vector
      Q2e <- as.vector( mc[ i, "Q2e", , m ] )
      Q2e <- Q2e[ is.finite( Q2e ) ]
      Q2e <- Q2e[ Q2e >= 1 ]
      mean.Q2e <- mean( Q2e, na.rm = TRUE )
      mean.lsize2 <- mean( log( Q2e ), na.rm = TRUE )

      A2 <- as.vector( mc[ i, "A2", , m ] )
      A2 <- A2[ is.finite( A2 ) ]
      A2 <- A2[ A2 >= 1 ]
      mean.A2 <- mean( A2, na.rm = TRUE )

      w2realAvg <- as.vector( mc[ i, "w2realAvg", , m ] )
      w2realAvg <- w2realAvg[ is.finite( w2realAvg ) ]
      w2realAvg <- w2realAvg[ w2realAvg > 0 ]
      mean.w2realAvg <- mean( w2realAvg, na.rm = TRUE )

      s2avg <- as.vector( mc[ i, "s2avg", , m ] )
      s2avg <- s2avg[ is.finite( s2avg ) ]
      s2avg <- s2avg[ s2avg > 0 ]
      mean.s2avg <- mean( s2avg, na.rm = TRUE )

      if( ! is.finite( mean.Q2e ) ) mean.Q2e <- 0
      if( ! is.finite( mean.lsize2 ) ) mean.lsize2 <- 0
      if( ! is.finite( mean.A2 ) ) mean.A2 <- 0
      if( ! is.finite( mean.w2realAvg ) ) mean.w2realAvg <- 0
      if( ! is.finite( mean.s2avg ) ) mean.s2avg <- 0

      for( j in 1 : nFirms ){            # and all firms (instances)

        # Take care of entrants' first period and other data problems to avoid artifacts
        if( is.na( mc[ i, "Q2e", j, m ] ) || mc[ i, "Q2e", j, m ] < 1 ||
            is.na( mc[ i, "A2", j, m ] ) || mc[ i, "A2", j, m ] < 1 ||
            is.na( mc[ i, "w2realAvg", j, m ] ) || mc[ i, "w2realAvg", j, m ] <= 0 ||
            is.na( mc[ i, "s2avg", j, m ] ) || mc[ i, "s2avg", j, m ] <= 0 ||
            is.na( mc[ i, "postChg", j, m ] ) || is.na( mc[ i, "postChg", j, m ] ) ) {
          mc[ i, , j, m ] <- rep( NA, nVar )
          next
        }

        # Normalization of key variables using the period average size
        if( mean.Q2e != 0 )
          mc[ i, "normO2", j, m ] <- mc[ i, "Q2e", j, m ] / mean.Q2e
        if( mean.lsize2 != 0 )
          mc[ i, "lnormO2", j, m ] <- log( mc[ i, "Q2e", j, m ] ) - mean.lsize2
        if( mean.A2 != 0 ) {
          mc[ i, "normA2", j, m ] <- mc[ i, "A2", j, m ] / mean.A2
          # handle separate firms groups
          if( ! is.na( mc[ i, "postChg", j, m ] ) )
            if( mc[ i, "postChg", j, m ] == 0 )
              mc[ i, "normApreChg2", j, m ] = mc[ i, "normA2", j, m ]
            else
              mc[ i, "normApostChg2", j, m ] = mc[ i, "normA2", j, m ]
        }
        if( mean.w2realAvg != 0 ) {
          mc[ i, "normRW2", j, m ] <- mc[ i, "w2realAvg", j, m ] / mean.w2realAvg
          # handle separate firms groups
          if( ! is.na( mc[ i, "postChg", j, m ] ) )
            if( mc[ i, "postChg", j, m ] == 0 )
              mc[ i, "normRWpreChg2", j, m ] = mc[ i, "normRW2", j, m ]
            else
              mc[ i, "normRWpostChg2", j, m ] = mc[ i, "normRW2", j, m ]
         }
        if( mean.s2avg != 0 )
          mc[ i, "normSK2", j, m ] <- mc[ i, "s2avg", j, m ] / mean.s2avg

        # Growth rates and deltas are calculated only from 2nd period and for non-entrant firms
        if( i > 1 ) {
          if( is.finite( mc[ i - 1, "Q2e", j, m ] ) ) {

            # Size, normalized sales and productivity growth
            mc[ i, "growth2", j, m ] <- log( mc[ i, "Q2e", j, m ] ) -
                                                log( mc[ i - 1, "Q2e", j, m ] )
            mc[ i, "normO2grow", j, m ] <- log( mc[ i, "normO2", j, m ] ) -
                                                log( mc[ i - 1, "normO2", j, m ] )
            mc[ i, "normA2grow", j, m ] <- log( mc[ i, "normA2", j, m ] ) -
                                                log( mc[ i - 1, "normA2", j, m ] )
            mc[ i, "normRW2grow", j, m ] <- log( mc[ i, "normRW2", j, m ] ) -
                                                 log( mc[ i - 1, "normRW2", j, m ] )
            mc[ i, "normSK2grow", j, m ] <- log( mc[ i, "normSK2", j, m ] ) -
                                                 log( mc[ i - 1, "normSK2", j, m ] )

            if( ! is.finite( mc[ i, "normO2grow", j, m ] ) )
              mc[ i, "normO2grow", j, m ] <- NA
            if( ! is.finite( mc[ i, "normA2grow", j, m ] ) )
              mc[ i, "normA2grow", j, m ] <- NA
            if( ! is.finite( mc[ i, "normRW2grow", j, m ] ) )
              mc[ i, "normRW2grow", j, m ] <- NA
            if( ! is.finite( mc[ i, "normSK2grow", j, m ] ) )
              mc[ i, "normSK2grow", j, m ] <- NA
          }

          if( is.finite( mc[ i, "lnormO2", j, m ] ) &&
              is.finite( mc[ i - 1, "lnormO2", j, m ] ) ) {

            mc[ i, "lnormO2match", j, m ] <- mc[ i, "lnormO2", j, m ]
            mc[ i, "lnormO2matchLag", j, m ] <- mc[ i - 1, "lnormO2", j, m ]
            mc[ i, "lnormO2grow", j, m ] <- mc[ i, "lnormO2", j, m ] -
                                                 mc[ i - 1, "lnormO2", j, m ]

            if( ! is.finite( mc[ i, "lnormO2grow", j, m ] ) ) {
              mc[ i, "lnormO2grow", j, m ] <- NA
              mc[ i, "lnormO2match", j, m ] <- NA
              mc[ i, "lnormO2matchLag", j, m ] <- NA
            }
          }
        }
      }
    }

  # ---- Reorganize and save data ----

  # Create "flatter" 3D arrray, appending firms from different MC runs in sequence
  pool <- array( as.numeric( NA ), dim = c( nTsteps, nVar, nFirms * nSize ),
                 dimnames = list( c( ( iniDrop + 1 ) : ( iniDrop + nTsteps ) ),
                                  newFirmVar, c( 1 : ( nFirms * nSize ) ) ) )
  l <- rep( 1, nVar )                   # absolute variable instance (firm) counter

  for( m in 1 : nSize )                 # MC sample
    for( j in 1 : nVar )                # for all variables
      for( i in 1 : nFirms ){           # for all firms
        pool[ , j, l[ j ]] <- mc[ , j, i, m ]
        l[ j ] <- l[ j ] + 1
      }

  # Save temporary results to disk to save memory
  tmpFile <- paste0( folder, "/", baseName, exper, "_firm2_pool.Rdata" )
  save( pool, nTsteps, nVar, nSize, nFirms, file = tmpFile )

  return( tmpFile )
}

# load each experiment serially
tmpFiles <- lapply( 1 : nExp, readExp )

invisible( gc( verbose = FALSE ) )


#******************************************************************
#
# --------------------- Plot statistics -------------------------
#
#******************************************************************

# ====== User parameters ======

CI     <- 0.95      # desired confidence interval
nBins  <- 15        # number of bins to use in histograms
outLim <- 0.10      # outlier percentile (0=don't remove outliers)
warmUp <- 300       # number of "warm-up" time steps
nTstat <- -1        # last period to consider for statistics (-1=all)
limOutl<- 0.10      # quantile extreme limits (0=none)

repName <- ""       # report files base name (if "" same baseName)
sDigits <- 4        # significant digits in tables
plotRows <- 1       # number of plots per row in a page
plotCols <- 1  	    # number of plots per column in a page
plotW <- 10         # plot window width
plotH <- 7          # plot window height

# Colors assigned to each experiment's lines in graphics
colors <- c( "black", "blue", "red", "orange", "green", "brown" )

# Line types assigned to each experiment
lTypes <- c( "solid", "solid", "solid", "solid", "solid", "solid" )

# Point types assigned to each experiment
pTypes <- c( 4, 4, 4, 4, 4, 4 )


# ====== External support functions & definitions ======

if( ! exists( "plot_norm", mode = "function" ) )      # already loaded?
  source( "KS-support-functions.R" )

# remove warnings for support functions
# !diagnostics suppress = logNA, log0, t.test0, se, bkfilter, adf.test, colSds
# !diagnostics suppress = plot_lognorm, plot_norm, plot_laplace, plot_lin
# !diagnostics suppress = fit_subbotin, comp_stats, comp_MC_stats, size_bins
# !diagnostics suppress = remove_outliers_table, all.NA, textplot, nCores

# ====== Support stuff ======

if( repName == "" )
  repName <- baseName

# Generate fancy labels & build labels list legend
legends <- vector( )
legendList <- "Experiments: "
for( k in 1 : nExp ) {
  if( is.na( expVal[ k ]) || expVal[ k ] == "" )
    legends[ k ] <- paste( "Case", k )
  else
    legends[ k ] <- expVal[ k ]
  if( k != 1 )
    legendList <- paste0( legendList, ",  " )
  legendList <- paste0( legendList, "[", k, "] ", legends[ k ] )
}

# load data from first experiment
load( tmpFiles[[ 1 ]] )

# Number of periods to show in graphics and use in statistics
if( nTstat < 1 || nTstat > nTsteps || nTstat <= warmUp )
  nTstat <- nTsteps
TmaskStat <- ( warmUp + 1 ) : nTstat


#
# ====== Do pooled analysis ======
#

# Open PDF plot file for output
pdf( paste0( folder, "/", repName, "_firm2_pool.pdf" ),
     width = plotW, height = plotH )
par( mfrow = c ( plotRows, plotCols ) )             # define plots per page

szData <- oData <- Adata <- AprData <- ApoData <- wData <-
  wPrData <- wPoData <- sData <- gData <- ngData <- ngAdata <- nwgData <-
  nsgData <- exData <- svData <- bins <- list( )

# run over all experiments individually
for( k in 1 : nExp ){

  # load pooled data from temporary files (first already loaded)
  if( k > 1 )
    load( tmpFiles[[ k ]] )

  file.remove( tmpFiles[[ k ]] )      # delete temporary file

  #
  # ------ Create data & statistics vectors ------
  #

  szData[[ k ]] <- as.vector( pool[ TmaskStat, "Q2e", ] )
  oData[[ k ]] <- as.vector( logNA( pool[ TmaskStat, "normO2", ] ) )
  Adata[[ k ]] <- as.vector( logNA( pool[ TmaskStat, "normA2", ] ) )
  AprData[[ k ]] <- as.vector( logNA( pool[ TmaskStat, "normApreChg2", ] ) )
  ApoData[[ k ]] <- as.vector( logNA( pool[ TmaskStat, "normApostChg2", ] ) )
  wData[[ k ]] <- as.vector( logNA( pool[ TmaskStat, "normRW2", ] ) )
  wPrData[[ k ]] <- as.vector( logNA( pool[ TmaskStat, "normRWpreChg2", ] ) )
  wPoData[[ k ]] <- as.vector( logNA( pool[ TmaskStat, "normRWpostChg2", ] ) )
  sData[[ k ]] <- as.vector( logNA( pool[ TmaskStat,"normSK2", ] ) )
  gData[[ k ]] <- as.vector( pool[ TmaskStat, "growth2", ] )
  ngData[[ k ]] <- as.vector( pool[ TmaskStat, "normO2grow", ] )
  ngAdata[[ k ]] <- as.vector( pool[ TmaskStat, "normA2grow", ] )
  nwgData[[ k ]] <- as.vector( pool[ TmaskStat, "normRW2grow", ] )
  nsgData[[ k ]] <- as.vector( pool[ TmaskStat, "normSK2grow", ] )
  exData[[ k ]] <- as.vector( as.numeric( is.na( pool[ TmaskStat, "growth2", ] ) ) )

  # remove NAs
  szData[[ k ]] <- szData[[ k ]][ ! is.na( szData[[ k ]] ) ]
  oData[[ k ]] <- oData[[ k ]][ ! is.na( oData[[ k ]] ) ]
  Adata[[ k ]] <- Adata[[ k ]][ ! is.na( Adata[[ k ]] ) ]
  AprData[[ k ]] <- AprData[[ k ]][ ! is.na( AprData[[ k ]] ) ]
  ApoData[[ k ]] <- ApoData[[ k ]][ ! is.na( ApoData[[ k ]] ) ]
  wData[[ k ]] <- wData[[ k ]][ ! is.na( wData[[ k ]] ) ]
  wPrData[[ k ]] <- wPrData[[ k ]][ ! is.na( wPrData[[ k ]] ) ]
  wPoData[[ k ]] <- wPoData[[ k ]][ ! is.na( wPoData[[ k ]] ) ]
  sData[[ k ]] <- sData[[ k ]][ ! is.na( sData[[ k ]] ) ]
  gData[[ k ]] <- gData[[ k ]][ ! is.na( gData[[ k ]] ) ]
  ngData[[ k ]] <- ngData[[ k ]][ ! is.na( ngData[[ k ]] ) ]
  ngAdata[[ k ]] <- ngAdata[[ k ]][ ! is.na( ngAdata[[ k ]] ) ]
  nwgData[[ k ]] <- nwgData[[ k ]][ ! is.na( nwgData[[ k ]] ) ]
  nsgData[[ k ]] <- nsgData[[ k ]][ ! is.na( nsgData[[ k ]] ) ]
  exData[[ k ]] <- exData[[ k ]][ ! is.na( exData[[ k ]] ) ]

  # compute each variable statistics
  stats <- lapply( list( oData[[ k ]], Adata[[ k ]], wData[[ k ]],
                         sData[[ k ]], gData[[ k ]], ngData[[ k ]],
                         ngAdata[[ k ]], nwgData[[ k ]], nsgData[[ k ]],
                         exData[[ k ]] ), comp_stats )

  o <- stats[[ 1 ]]
  A <- stats[[ 2 ]]
  w <- stats[[ 3 ]]
  s <- stats[[ 4 ]]
  g <- stats[[ 5 ]]
  ng <- stats[[ 6 ]]
  ngA <- stats[[ 7 ]]
  nwg <- stats[[ 8 ]]
  nsg <- stats[[ 9 ]]
  ex <- stats[[ 10 ]]

  # prepare data for Gibrat and scaling variance plots
  bins[[ k ]] <- size_bins( as.vector( pool[ TmaskStat, "lnormO2match", ] ),
                            as.vector( pool[ TmaskStat, "lnormO2matchLag", ] ),
                            as.vector( pool[ TmaskStat, "lnormO2grow", ] ),
                            bins = 2 * nBins, outLim = outLim )

  #
  # ------ Build statistics table ------
  #

  key.stats <- matrix( c( o$avg, A$avg, w$avg, s$avg, g$avg,
                          ng$avg, ngA$avg, nwg$avg, nsg$avg, ex$avg,

                          o$sd / sqrt( nSize ), A$sd / sqrt( nSize ),
                          w$sd / sqrt( nSize ), s$sd / sqrt( nSize ),
                          g$sd / sqrt( nSize ), ng$sd / sqrt( nSize ),
                          ngA$sd / sqrt( nSize ), nwg$sd / sqrt( nSize ),
                          nsg$sd / sqrt( nSize ), ex$sd / sqrt( nSize ),

                          o$sd, A$sd, w$sd, s$sd, g$sd, ng$sd,
                          ngA$sd, nwg$sd, nsg$sd, ex$sd,

                          o$subbo$b, A$subbo$b, w$subbo$b, s$subbo$b,
                          g$subbo$b, ng$subbo$b, ngA$subbo$b, nwg$subbo$b,
                          nsg$subbo$b, ex$subbo$b,

                          o$subbo$a, A$subbo$a, w$subbo$a, s$subbo$a,
                          g$subbo$a, ng$subbo$a, ngA$subbo$a, nwg$subbo$a,
                          nsg$subbo$a, ex$subbo$a,

                          o$subbo$m, A$subbo$m, w$subbo$m, s$subbo$m,
                          g$subbo$m, ng$subbo$m, ngA$subbo$m, nwg$subbo$m,
                          nsg$subbo$m, ex$subbo$m,

                          o$jb$statistic, A$jb$statistic, w$jb$statistic,
                          s$jb$statistic, g$jb$statistic, ng$jb$statistic,
                          ngA$jb$statistic, nwg$jb$statistic, nsg$jb$statistic,
                          ex$jb$statistic,

                          o$jb$p.value, A$jb$p.value, w$jb$p.value,
                          s$jb$p.value, g$jb$p.value, ng$jb$p.value,
                          ngA$jb$p.value, nwg$jb$p.value, nsg$jb$p.value,
                          ex$jb$p.value,

                          o$ll$statistic, A$ll$statistic, w$ll$statistic,
                          s$ll$statistic, g$ll$statistic, ng$ll$statistic,
                          ngA$ll$statistic, nwg$ll$statistic, nsg$ll$statistic,
                          ex$ll$statistic,

                          o$ll$p.value, A$ll$p.value, w$ll$p.value,
                          s$ll$p.value, g$ll$p.value, ng$ll$p.value,
                          ngA$ll$p.value, nwg$ll$p.value, nsg$ll$p.value,
                          ex$ll$p.value,

                          o$ad$statistic, A$ad$statistic, w$ad$statistic,
                          s$ad$statistic, g$ad$statistic, ng$ad$statistic,
                          ngA$ad$statistic, nwg$ad$statistic, nsg$ad$statistic,
                          ex$ad$statistic,

                          o$ad$p.value, A$ad$p.value, w$ad$p.value,
                          s$ad$p.value, g$ad$p.value, ng$ad$p.value,
                          ngA$ad$p.value, nwg$ad$p.value, nsg$ad$p.value,
                          ex$ad$p.value,

                          o$ac$t1, A$ac$t1, w$ac$t1, s$ac$t1, g$ac$t1,
                          ng$ac$t1, ngA$ac$t1, nwg$ac$t1, nsg$ac$t1, ex$ac$t1,

                          o$ac$t2, A$ac$t2, w$ac$t2, s$ac$t2, g$ac$t2,
                          ng$ac$t2, ngA$ac$t2, nwg$ac$t2, nsg$ac$t2, ex$ac$t2 ),

                       ncol = 10, byrow = TRUE )
  colnames( key.stats ) <- c( "N.Output(log)", "N.Prod.(log)", "N.R.Wage(log)",
                              "N.Skills(log)", "Output Gr.", "N.Output Gr.",
                              "N.Prod.Gr.", "N.R.Wage Gr.", "N.Skills Gr.", "Exit Rate" )
  rownames( key.stats ) <- c( "average", " (s.e.)", " (s.d.)", "Subbotin b", " a", " m", "Jarque-Bera X2",
                              " (p-val.)", "Lilliefors D", " (p-val.)", "Anderson-Darling A", " (p-val.)",
                              "autocorr. t-1", "autocorr. t-2" )

  textplot( formatC( key.stats, digits = sDigits, format = "g" ), cmar = 1.0 )
  title <- paste( "Pooled firm-level statistics (", legends[ k ], ")" )
  subTitle <- paste( eval( bquote( paste0( "( Sample size = ", sum( nFirms ),
                                           " firms / Period = ", warmUp + 1,
                                           "-", nTstat, " )" ) ) ),
                    sector, sep ="\n" )
  title( main = title, sub = subTitle )

  # ------ Gibrat law test plot ------

  plot_lin( bins[[ k ]]$sLagAvg, bins[[ k ]]$s1avg,
            "log(normalized size of firms in t-1)",
            "log(normalized size of firms in t)",
            tit = paste( "Gibrat law (", legends[ k ], ")" ),
            subtit = sector, invleg = TRUE )

  # ------ Scaling variance plot ------

  plot_lin( bins[[ k ]]$s2avg, bins[[ k ]]$gSD,
            "log(normalized size of firms)",
            "log(standard deviation of growth rate)",
            tit = paste( "Scaling of variance of firm size (", legends[ k ], ")" ),
            subtit = sector )
}

#
# ====== Plot distributions ( overplots )  ======
#

# ------ Size and productivity  distributions ( binned density x log variable in level )  ------

plot_norm( oData, "log(normalized size)", "Binned density",
           "Pooled size (output) distribution ( all experiments )",
           sector, outLim, nBins, legends, colors, lTypes, pTypes )

plot_norm( Adata, "log(normalized productivity)", "Binned density",
           "Pooled productivity distribution ( all experiments )",
           sector, outLim, nBins, legends, colors, lTypes, pTypes )

plot_norm( wData, "log(normalized average real wage)", "Binned density",
           "Pooled average wages distribution ( all experiments )",
           sector, outLim, nBins, legends, colors, lTypes, pTypes )

plot_norm( sData, "log(normalized average skills level)", "Binned density",
           "Pooled average skills distribution ( all experiments )",
           sector, outLim, nBins, legends, colors, lTypes, pTypes )

# ------ Size distributions ( log size x rank )  ------

plot_lognorm( szData, "Size", "Rank",
              "Pooled size (output) distribution ( all experiments )",
              sector, outLim, nBins, legends, colors, lTypes )

# ------ Growth rate distributions ( binned density x log growth rate )  ------

plot_laplace( gData, "Size growth rate", "Binned density",
              "Pooled size (output) growth rate distribution ( all experiments )",
              sector, outLim, nBins, legends, colors, lTypes, pTypes )

plot_laplace( ngData, "Normalized size growth rate", "Binned density",
              "Pooled size (output) growth rate distribution ( all experiments )",
              sector, outLim, nBins, legends, colors, lTypes, pTypes )

plot_laplace( ngAdata, "Normalized productivity growth rate", "Binned density",
              "Pooled productivity growth rate distribution ( all experiments )",
              sector, outLim, nBins, legends, colors, lTypes, pTypes )

plot_laplace( nwgData, "Normalized average real wage growth rate", "Binned density",
              "Pooled average wage growth distribution ( all experiments )",
              sector, outLim, nBins, legends, colors, lTypes, pTypes )

plot_laplace( nsgData, "Normalized average skills level growth rate", "Binned density",
              "Pooled average skills growth distribution ( all experiments )",
              sector, outLim, nBins, legends, colors, lTypes, pTypes )


# Close PDF plot file
dev.off( )
