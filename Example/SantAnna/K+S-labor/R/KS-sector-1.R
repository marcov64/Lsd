#******************************************************************
#
# ------------------ K+S sector 1 analysis ----------------------
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
firmVar <- c( "_Btau", "_Q1e" )
addFirmVar <- c( "growth1", "normO1", "lnormO1", "lnormO1match", "lnormO1matchLag",
                 "normA1", "normO1grow", "lnormO1grow", "normA1grow" )
sector <- "( Capital-goods sector )"


# ==== Process LSD result files ====

# Package with LSD interface functions
library( LSDinterface, verbose = FALSE, quietly = TRUE )
library( abind, verbose = FALSE, quietly = TRUE )

# remove warnings for saved data
# !diagnostics suppress = mc, pool, nSize, nTsteps

# ---- Read data files ----

# Function to read one experiment data (to be parallelized)
readExp <- function( exper ) {
  if( nExp > 1 ) {
    myFiles <- list.files( path = folder, pattern = paste0( baseName, exper, "_[0-9]+.res" ),
                           full.names = TRUE )
  } else {
    myFiles <- list.files( path = folder, pattern = paste0( baseName, "_[0-9]+.res" ),
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
      Q1e <- as.vector( mc[ i, "Q1e", , m ] )
      Q1e <- Q1e[ is.finite( Q1e ) ]
      Q1e <- Q1e[ Q1e >= 1 ]
      mean.Q1e <- mean( Q1e, na.rm = TRUE )
      mean.lsize1 <- mean( log( Q1e ), na.rm = TRUE )

      Btau <- as.vector( mc[ i, "Btau", , m ] )
      Btau <- Btau[ is.finite( Btau ) ]
      Btau <- Btau[ Btau >= 1 ]
      mean.Btau <- mean( Btau, na.rm = TRUE )

      if( ! is.finite( mean.Q1e ) ) mean.Q1e <- 0
      if( ! is.finite( mean.lsize1 ) ) mean.lsize1 <- 0
      if( ! is.finite( mean.Btau ) ) mean.Btau <- 0

      for( j in 1 : nFirms ){           # and all firms (instances)

        # Take care of entrants' first period and other data problems to avoid atifacts
        if( is.na( mc[ i, "Q1e", j, m ] ) || mc[ i, "Q1e", j, m ] < 1 ||
            is.na( mc[ i, "Btau", j, m ] ) || mc[ i, "Btau", j, m ] < 1 ) {
          mc[ i, , j, m ] <- rep( NA, nVar )
          next
        }

        # Normalization of key variables using the period average size
        if( mean.Q1e != 0 )
          mc[ i, "normO1", j, m ] <- mc[ i,"Q1e", j, m ] / mean.Q1e
        if( mean.lsize1 != 0 )
          mc[ i, "lnormO1", j, m ] <- log( mc[ i, "Q1e", j, m ] ) - mean.lsize1
        if( mean.Btau != 0 )
          mc[ i, "normA1", j, m ] <- mc[ i, "Btau", j, m ] / mean.Btau

        # Growth rates are calculated only from 2nd period and for non-entrant firms
        if( i > 1 ) {
          if( ! is.na( mc[ i - 1, "Q1e", j, m ] ) ){

            # Size, normalized sales and productivity growth
            mc[ i, "growth1", j, m ] <- log( mc[ i, "Q1e", j, m ] ) -
                                        log( mc[ i - 1, "Q1e", j, m ] )
            mc[ i, "normO1grow", j, m ] <- log( mc[ i, "normO1", j, m ] ) -
                                           log( mc[ i - 1, "normO1", j, m ] )
            mc[ i, "normA1grow", j, m ] <- log( mc[ i, "normA1", j, m ] ) -
                                                log( mc[ i - 1, "normA1", j, m ] )

            if( is.infinite( mc[ i, "normO1grow", j, m ] ) )
              mc[ i, "normO1grow", j, m ] <- NA
            if( is.infinite( mc[ i, "normA1grow", j, m ] ) )
              mc[ i, "normA1grow", j, m ] <- NA
          }

          if( is.finite( mc[ i, "lnormO1", j, m ] ) &&
              is.finite( mc[ i - 1, "lnormO1", j, m ] ) ) {

            mc[ i, "lnormO1match", j, m ] <- mc[ i, "lnormO1", j, m ]
            mc[ i, "lnormO1matchLag", j, m ] <- mc[ i - 1, "lnormO1", j, m ]
            mc[ i, "lnormO1grow", j, m ] <- mc[ i, "lnormO1", j, m ] -
                                                 mc[ i - 1, "lnormO1", j, m ]

            if( ! is.finite( mc[ i, "lnormO1grow", j, m ] ) ) {
              mc[ i, "lnormO1grow", j, m ] <- NA
              mc[ i, "lnormO1match", j, m ] <- NA
              mc[ i, "lnormO1matchLag", j, m ] <- NA
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
  tmpFile <- paste0( folder, "/", baseName, exper, "_firm1.Rdata" )
  save( mc, pool, nTsteps, nVar, nSize, nFirms, file = tmpFile )

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

CI     <- 0.95   # desired confidence interval
nBins  <- 15     # number of bins to use in histograms
outLim <- 0.001  # outlier percentile (0=don't remove outliers)
warmUp <- 300    # number of "warm-up" runs
nTstat <- -1     # last period to consider for statistics (-1=all)

repName <- ""    # report files base name (if "" same baseName)
sDigits <- 4     # significant digits in tables
plotRows <- 1    # number of plots per row in a page
plotCols <- 1  	 # number of plots per column in a page
plotW <- 10      # plot window width
plotH <- 7       # plot window height

# Colors assigned to each experiment's lines in graphics
colors <- c( "black", "blue", "red", "orange", "green", "brown" )

# Line types assigned to each experiment
lTypes <- c( "solid", "solid", "solid", "solid", "solid", "solid" )

# Point types assigned to each experiment
pTypes <- c( 4, 4, 4, 4, 4, 4 )


# ====== External support functions & definitions ======

if( ! exists( "plot_norm", mode = "function" ) )     # already loaded?
  source( "KS-support-functions.R" )
# remove warnings for support functions
# !diagnostics suppress = logNA, log0, t.test0, se, bkfilter, adf.test, colSds
# !diagnostics suppress = plot_lognorm, plot_norm, plot_laplace, plot_lin, textplot
# !diagnostics suppress = fit_subbotin, comp_stats, comp_MC_stats, size_bins, nCores


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
pdf( paste0( folder, "/", repName, "_firm1_pool.pdf" ),
     width = plotW, height = plotH )
par( mfrow = c ( plotRows, plotCols ) )             # define plots per page

szData <- oData <- Adata <- gData <- ngData <- ngAdata <- exData <- svData <-
  bins <- list( )

# run over all experiments individually
for( k in 1 : nExp ){

  # load pooled data from temporary files (first already loaded)
  if( k > 1 )
    load( tmpFiles[[ k ]] )

  rm( mc )                   # erase unused variables

  #
  # ------ Create data & statistics vectors ------
  #

  szData[[ k ]] <- as.vector( pool[ TmaskStat, "Q1e", ] )
  oData[[ k ]] <- as.vector( logNA( pool[ TmaskStat, "normO1", ] ) )
  Adata[[ k ]] <- as.vector( logNA( pool[ TmaskStat, "normA1", ] ) )
  gData[[ k ]] <- as.vector( pool[ TmaskStat, "growth1", ] )
  ngData[[ k ]] <- as.vector( pool[ TmaskStat, "normO1grow", ] )
  ngAdata[[ k ]] <- as.vector( pool[ TmaskStat, "normA1grow", ] )
  exData[[ k ]] <- as.vector( as.numeric( is.na( pool[ TmaskStat, "growth1", ] ) ) )

  # remove NAs
  szData[[ k ]] <- szData[[ k ]][ ! is.na( szData[[ k ]] ) ]
  oData[[ k ]] <- oData[[ k ]][ ! is.na( oData[[ k ]] ) ]
  Adata[[ k ]] <- Adata[[ k ]][ ! is.na( Adata[[ k ]] ) ]
  gData[[ k ]] <- gData[[ k ]][ ! is.na( gData[[ k ]] ) ]
  ngData[[ k ]] <- ngData[[ k ]][ ! is.na( ngData[[ k ]] ) ]
  ngAdata[[ k ]] <- ngAdata[[ k ]][ ! is.na( ngAdata[[ k ]] ) ]
  exData[[ k ]] <- exData[[ k ]][ ! is.na( exData[[ k ]] ) ]

  # compute each variable statistics
  stats <- lapply( list( oData[[ k ]], Adata[[ k ]], gData[[ k ]],
                         ngData[[ k ]], ngAdata[[ k ]], exData[[ k ]] ),
                   comp_stats )

  o <- stats[[ 1 ]]
  A <- stats[[ 2 ]]
  g <- stats[[ 3 ]]
  ng <- stats[[ 4 ]]
  ngA <- stats[[ 5 ]]
  ex <- stats[[ 6 ]]

  # prepare data for Gibrat and scaling variance plots
  bins[[ k ]] <- size_bins( as.vector( pool[ TmaskStat, "lnormO1match", ] ),
                            as.vector( pool[ TmaskStat, "lnormO1matchLag", ] ),
                            as.vector( pool[ TmaskStat, "lnormO1grow", ] ),
                            bins = 2 * nBins, outLim = outLim )

  #
  # ------ Build statistics table ------
  #

  key.stats <- matrix( c( o$avg, A$avg, g$avg, ng$avg, ngA$avg, ex$avg,

                          o$sd / sqrt( nSize ), A$sd / sqrt( nSize ),
                          g$sd / sqrt( nSize ), ng$sd / sqrt( nSize ),
                          ngA$sd / sqrt( nSize ), ex$sd / sqrt( nSize ),

                          o$sd, A$sd, g$sd, ng$sd, ngA$sd, ex$sd,

                          o$subbo$b, A$subbo$b, g$subbo$b, ng$subbo$b,
                          ngA$subbo$b, ex$subbo$b,

                          o$subbo$a, A$subbo$a, g$subbo$a, ng$subbo$a,
                          ngA$subbo$a, ex$subbo$a,

                          o$subbo$m, A$subbo$m, g$subbo$m, ng$subbo$m,
                          ngA$subbo$m, ex$subbo$m,

                          o$jb$statistic, A$jb$statistic, g$jb$statistic,
                          ng$jb$statistic, ngA$jb$statistic, ex$jb$statistic,

                          o$jb$p.value, A$jb$p.value, g$jb$p.value,
                          ng$jb$p.value, ngA$jb$p.value, ex$jb$p.value,

                          o$ll$statistic, A$ll$statistic, g$ll$statistic,
                          ng$ll$statistic, ngA$ll$statistic, ex$ll$statistic,

                          o$ll$p.value, A$ll$p.value, g$ll$p.value,
                          ng$ll$p.value, ngA$ll$p.value, ex$ll$p.value,

                          o$ad$statistic, A$ad$statistic, g$ad$statistic,
                          ng$ad$statistic, ngA$ad$statistic, ex$ad$statistic,

                          o$ad$p.value, A$ad$p.value, g$ad$p.value,
                          ng$ad$p.value, ngA$ad$p.value, ex$ad$p.value,

                          o$ac$t1, A$ac$t1, g$ac$t1, ng$ac$t1,
                          ngA$ac$t1, ex$ac$t1,

                          o$ac$t2, A$ac$t2, g$ac$t2, ng$ac$t2,
                          ngA$ac$t2, ex$ac$t2 ),

                       ncol = 6, byrow = TRUE )
  colnames( key.stats ) <- c( "N.Output(log)", "N.Prod.(log)", "Output Gr.",
                              "N.Output Gr.", "N.Prod.Gr.", "Exit Rate")
  rownames( key.stats ) <- c( "average", " (s.e.)", " (s.d.)", "Subbotin b",
                              " a", " m", "Jarque-Bera X2",
                              " (p-val.)", "Lilliefors D", " (p-val.)",
                              "Anderson-Darling A", " (p-val.)",
                              "autocorr. t-1", "autocorr. t-2" )

  textplot( formatC( key.stats, digits = sDigits, format = "g" ), cmar = 1.0 )
  title <- paste( "Pooled firm-level statistics (", legends[ k ], ")" )
  subTitle <- paste( eval( bquote( paste0( "( Sample size = ", .( nFirms * nSize ),
                                           " firms / Period = ", .( warmUp + 1 ),
                                           "-", .( nTstat ), " )" ) ) ),
                    sector, sep ="\n" )
  title( main = title, sub = subTitle )

  # ------ Gibrat law test plot ------

  plot_lin( bins[[ k ]]$sLagAvg, bins[[ k ]]$s1avg,
            "log(normalized size of firms in t-1)",
            "log(normalized size of firms in t)",
            paste( "Gibrat law (", legends[ k ], ")" ), sector )

  # ------ Scaling variance plot ------

  plot_lin( bins[[ k ]]$s2avg, bins[[ k ]]$gSD,
            "log(normalized size of firms)",
            "log(standard deviation of growth rate)",
            paste( "Scaling of variance of firm size (",
                   legends[ k ], ")" ), sector )
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

# ------ Size distributions ( log size x rank )  ------

plot_lognorm( szData, "Size", "Rank",
              "Pooled size (output) distribution ( all experiments )",
              sector, outLim, nBins, legends, colors, lTypes )

# ------ Growth rate distributions ( binned density x growth rate )  ------

plot_laplace( gData, "Size growth rate", "Binned density",
              "Pooled size (output) growth rate distribution ( all experiments )",
              sector, outLim, nBins, legends, colors, lTypes, pTypes )

plot_laplace( ngData, "Normalized size growth rate", "Binned density",
              "Pooled size (output) growth rate distribution ( all experiments )",
              sector, outLim, nBins, legends, colors, lTypes, pTypes )

plot_laplace( ngAdata, "Normalized productivity growth rate", "Binned density",
              "Pooled productivity growth rate distribution ( all experiments )",
              sector, outLim, nBins, legends, colors, lTypes, pTypes )

# free memory
rm( bins, pool, szData, oData, Adata, gData, ngData, ngAdata, exData, svData )
invisible( gc( verbose = FALSE ) )

# Close plot file
dev.off()

#
# ===== Do sensitivity analysis =====
#

# Open PDF plot file for output
pdf( paste0( folder, "/", repName, "_firm1_MC.pdf" ),
     width = plotW, height = plotH )
par( mfrow = c ( plotRows, plotCols ) )             # define plots per page

for( k in 1 : nExp ){             # do for each experiment

  # load MC data from temporary files
  load( tmpFiles[[ k ]] )
  file.remove( tmpFiles[[ k ]] )  # delete temporary file
  rm( pool )                      # erase unused variables

  #
  # ------ Create MC statistics table ------
  #

  oMC <- AMC <- gMC <- ngMC <- ngAMC <- exMC <- list( )

  for( l in 1 : nSize ){          # for each MC run

    # ------ Update MC statistics lists ------

    oData <- logNA( mc[ TmaskStat, "Q1e", , l ] )
    Adata <- logNA( mc[ TmaskStat, "Btau", , l ] )
    gData <- mc[ TmaskStat,"growth1", , l ]
    ngData <- mc[ TmaskStat,"normO1grow", , l ]
    ngAdata <- mc[ TmaskStat,"normA1grow", , l ]
    exData <- as.numeric( is.na( mc[ TmaskStat, "growth1", , l ] ) )

    # compute each variable statistics
    stats <- lapply( list( oData, Adata, gData, ngData, ngAdata, exData ),
                     comp_stats )

    oMC[[l]] <- stats[[ 1 ]]
    AMC[[l]] <- stats[[ 2 ]]
    gMC[[l]] <- stats[[ 3 ]]
    ngMC[[l]] <- stats[[ 4 ]]
    ngAMC[[l]] <- stats[[ 5 ]]
    exMC[[l]] <- stats[[ 6 ]]
  }

  # ------ Compute MC statistics vectors ------

  # compute each variable statistics
  stats <- lapply( list( oMC, AMC, gMC, ngMC, ngAMC, exMC ),
                   comp_MC_stats )

  o <- stats[[ 1 ]]
  A <- stats[[ 2 ]]
  g <- stats[[ 3 ]]
  ng <- stats[[ 4 ]]
  ngA <- stats[[ 5 ]]
  ex <- stats[[ 6 ]]

  # ------ Build statistics table ------

  key.stats <- matrix( c( o$avg$avg, A$avg$avg, g$avg$avg, ng$avg$avg,
                          ngA$avg$avg, ex$avg$avg,

                          o$se$avg, A$se$avg, g$se$avg, ng$se$avg,
                          ngA$se$avg, ex$se$avg,

                          o$sd$avg, A$sd$avg, g$sd$avg, ng$sd$avg,
                          ngA$sd$avg, ex$sd$avg,

                          o$avg$subbo$b, A$avg$subbo$b, g$avg$subbo$b,
                          ng$avg$subbo$b, ngA$avg$subbo$b, ex$avg$subbo$b,

                          o$se$subbo$b, A$se$subbo$b, g$se$subbo$b,
                          ng$se$subbo$b, ngA$se$subbo$b, ex$se$subbo$b,

                          o$sd$subbo$b, A$sd$subbo$b, g$sd$subbo$b,
                          ng$sd$subbo$b, ngA$sd$subbo$b, ex$sd$subbo$b,

                          o$avg$subbo$a, A$avg$subbo$a, g$avg$subbo$a,
                          ng$avg$subbo$a, ngA$avg$subbo$a, ex$avg$subbo$a,

                          o$se$subbo$a, A$se$subbo$a, g$se$subbo$a,
                          ng$se$subbo$a, ngA$se$subbo$a, ex$se$subbo$a,

                          o$sd$subbo$a, A$sd$subbo$a, g$sd$subbo$a,
                          ng$sd$subbo$a, ngA$sd$subbo$a, ex$sd$subbo$a,

                          o$avg$subbo$m, A$avg$subbo$m, g$avg$subbo$m,
                          ng$avg$subbo$m, ngA$avg$subbo$m, ex$avg$subbo$m,

                          o$se$subbo$m, A$se$subbo$m, g$se$subbo$m,
                          ng$se$subbo$m, ngA$se$subbo$m, ex$se$subbo$m,

                          o$sd$subbo$m, A$sd$subbo$m, g$sd$subbo$m,
                          ng$sd$subbo$m, ngA$sd$subbo$m, ex$sd$subbo$m,

                          o$avg$ac$t1, A$avg$ac$t1, g$avg$ac$t1,
                          ng$avg$ac$t1, ngA$avg$ac$t1, ex$avg$ac$t1,

                          o$se$ac$t1, A$se$ac$t1, g$se$ac$t1,
                          ng$se$ac$t1, ngA$se$ac$t1, ex$se$ac$t1,

                          o$sd$ac$t1, A$sd$ac$t1, g$sd$ac$t1,
                          ng$sd$ac$t1, ngA$sd$ac$t1, ex$sd$ac$t1,

                          o$avg$ac$t2, A$avg$ac$t2, g$avg$ac$t2,
                          ng$avg$ac$t2, ngA$avg$ac$t2, ex$avg$ac$t2,

                          o$se$ac$t2, A$se$ac$t2, g$se$ac$t2,
                          ng$se$ac$t2, ngA$se$ac$t2, ex$se$ac$t2,

                          o$sd$ac$t2, A$sd$ac$t2, g$sd$ac$t2,
                          ng$sd$ac$t2, ngA$sd$ac$t2, ex$sd$ac$t2 ),

                       ncol = 6, byrow = TRUE )
  colnames( key.stats ) <- c( "Output(log)", "Prod.(log)", "Output Gr.",
                              "N.Output Gr.", "N.Prod.Gr.", "Exit Rate")
  rownames( key.stats ) <- c( "average", " (s.e.)", " (s.d.)", "Subbotin b",
                              " (s.e.)", " (s.d.)", "Subbotin a", " (s.e.)",
                              " (s.d.)", "Subbotin m", " (s.e.)", " (s.d.)",
                              "autocorr. t-1", " (s.e.)",  " (s.d.)",
                              "autocorr. t-2", " (s.e.)",  " (s.d.)" )

  textplot( formatC( key.stats, digits = sDigits, format = "g" ), cmar = 1.0 )
  title <- paste( "Monte Carlo firm-level statistics (", legends[ k ], ")" )
  subTitle <- paste( eval( bquote( paste0( "( Sample size = ", .( nFirms ), " firms / MC runs = ", nSize,
                                        " / Period = ", .( warmUp + 1 ), "-",
                                        .( nTstat ), " )" ) ) ),
                    sector, sep ="\n" )
  title( main = title, sub = subTitle )
}


# Close PDF plot file
dev.off( )
