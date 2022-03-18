#******************************************************************
#
# ---------------- K+S energy sector analysis -------------------
#
#   Written by Marcelo C. Pereira, University of Campinas
#
#   Copyright Marcelo C. Pereira
#   Distributed under the GNU General Public License
#
#   The default configuration assumes that the supplied LSD
#   simulation configurations:
#     R/data/Sim1.lsd
#     R/data/Sim2.lsd
#   are executed before this script is used.
#
#   To execute the simulations, (1) open LSD Model Manager (LMM),
#   (2) in LSD Model Browser, open the model which contains this
#   script (double click), (3) in LMM, compile and run the model
#   (menu Model>Compile and Run), (4) in LSD Model Browser, load
#   the desired configuration (menu File>Load), (5) execute the
#   configuration (menu Run>Run or Run> Parallel Run), accepting
#   the defaults (parallel run is optional but typically saves
#   significant execution time).
#
#   IMPORTANT: this script assumes the R working directory is set
#   to the R subfolder where this script file is stored. This can
#   be done automatically in RStudio if a project is created at
#   this subfolder, or using the command setwd(). The "folder"
#   variable below must always point to a (relative) subfolder
#   of the R working directory.
#
#******************************************************************

#******************************************************************
#
# ------------ Read Monte Carlo experiment files ----------------
#
#******************************************************************

folder    <- "data"                 # data files folder
baseName  <- "Sim"                  # data files base name (same as .lsd file)
nExp      <- 2                      # number of experiments
iniDrop   <- 0                      # initial time steps to drop (0=none)
nKeep     <- -1                     # number of time steps to keep (-1=all)

expVal <- c( "Baseline", "Monopolistic power" )   # case parameter values

# Firm-level variables to use
firmVar <- c( "_Ade", "_Qde", "_Qge" )
addFirmVar <- c( "Qe", "growthE", "growthDE", "growthGE", "normOe", "normOde",
                 "normOge", "lnormOe", "lnormOeMatch", "lnormOeMatchLag",
                 "normAde", "normOeGrow", "lnormOeGrow", "normAdeGrow" )
sector <- "( Energy sector )"


# ==== Process LSD result files ====

# load support packages and functions
source( "KS-support-functions.R" )

# remove warnings for saved data
# !diagnostics suppress = mc, pool, nSize, nTsteps

# ---- Read data files ----

# Function to read one experiment data (to be parallelized)
readExp <- function( exper ) {
  if( nExp > 1 )
    myFiles <- list.files.lsd( folder, paste0( baseName, exper ) )
  else
    myFiles <- list.files.lsd( folder, baseName )

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
  newFirmVar <- name.nice.lsd( c( dimnames( mc )[[ 2 ]], addFirmVar ) )
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
      mc[ i, "Qe", , m ] <- mc[ i, "Qde", , m ] + mc[ i, "Qge", , m ]
      Qe <- as.vector( mc[ i, "Qe", , m ] )
      Qe <- Qe[ is.finite( Qe ) ]
      Qe <- Qe[ Qe >= 1 ]
      mean.Qe <- mean( Qe, na.rm = TRUE )
      mean.lsizeE <- mean( log( Qe ), na.rm = TRUE )

      Qde <- as.vector( mc[ i, "Qde", , m ] )
      Qde <- Qde[ is.finite( Qde ) ]
      Qde <- Qde[ Qde >= 1 ]
      mean.Qde <- mean( Qde, na.rm = TRUE )
      mean.lsizeDE <- mean( log( Qde ), na.rm = TRUE )

      Qge <- as.vector( mc[ i, "Qge", , m ] )
      Qge <- Qge[ is.finite( Qge ) ]
      Qge <- Qge[ Qge >= 1 ]
      mean.Qge <- mean( Qge, na.rm = TRUE )
      mean.lsizeGE <- mean( log( Qge ), na.rm = TRUE )

      Ade <- as.vector( mc[ i, "Ade", , m ] )
      Ade <- Ade[ is.finite( Ade ) ]
      Ade <- Ade[ Ade >= 1 ]
      mean.Ade <- mean( Ade, na.rm = TRUE )

      if( ! is.finite( mean.Qe ) ) mean.Qe <- 0
      if( ! is.finite( mean.Qde ) ) mean.Qde <- 0
      if( ! is.finite( mean.Qge ) ) mean.Qge <- 0
      if( ! is.finite( mean.lsizeE ) ) mean.lsizeE <- 0
      if( ! is.finite( mean.lsizeDE ) ) mean.lsizeDE <- 0
      if( ! is.finite( mean.lsizeGE ) ) mean.lsizeGE <- 0
      if( ! is.finite( mean.Ade ) ) mean.Ade <- 0

      for( j in 1 : nFirms ){           # and all firms (instances)

        # Take care of entrants' first period and other data problems to avoid atifacts
        if( is.na( mc[ i, "Qe", j, m ] ) || mc[ i, "Qe", j, m ] < 1 ||
            is.na( mc[ i, "Ade", j, m ] ) || mc[ i, "Ade", j, m ] <= 0 ) {
          mc[ i, , j, m ] <- rep( NA, nVar )
          next
        }

        # Normalization of key variables using the period average size
        if( mean.Qe != 0 )
          mc[ i, "normOe", j, m ] <- mc[ i,"Qe", j, m ] / mean.Qe
        if( mean.Qde != 0 )
          mc[ i, "normOde", j, m ] <- mc[ i,"Qde", j, m ] / mean.Qde
        if( mean.Qge != 0 )
          mc[ i, "normOge", j, m ] <- mc[ i,"Qge", j, m ] / mean.Qge
        if( mean.lsizeE != 0 )
          mc[ i, "lnormOe", j, m ] <- log( mc[ i, "Qe", j, m ] ) - mean.lsizeE
        if( mean.Ade != 0 )
          mc[ i, "normAde", j, m ] <- mc[ i, "Ade", j, m ] / mean.Ade

        # Growth rates are calculated only from 2nd period and for non-entrant firms
        if( i > 1 ) {
          if( ! is.na( mc[ i - 1, "Qe", j, m ] ) ){

            # Size, normalized sales and productivity growth
            mc[ i, "growthE", j, m ] <- log( mc[ i, "Qe", j, m ] ) -
                                        log( mc[ i - 1, "Qe", j, m ] )
            mc[ i, "growthDE", j, m ] <- log( mc[ i, "Qde", j, m ] ) -
                                         log( mc[ i - 1, "Qde", j, m ] )
            mc[ i, "growthGE", j, m ] <- log( mc[ i, "Qge", j, m ] ) -
                                         log( mc[ i - 1, "Qge", j, m ] )
            mc[ i, "normOeGrow", j, m ] <- log( mc[ i, "normOe", j, m ] ) -
                                           log( mc[ i - 1, "normOe", j, m ] )
            mc[ i, "normAdeGrow", j, m ] <- log( mc[ i, "normAde", j, m ] ) -
                                                log( mc[ i - 1, "normAde", j, m ] )

            if( is.infinite( mc[ i, "growthDE", j, m ] ) )
              mc[ i, "growthDE", j, m ] <- NA
            if( is.infinite( mc[ i, "growthGE", j, m ] ) )
              mc[ i, "growthGE", j, m ] <- NA
            if( is.infinite( mc[ i, "normOeGrow", j, m ] ) )
              mc[ i, "normOeGrow", j, m ] <- NA
            if( is.infinite( mc[ i, "normAdeGrow", j, m ] ) )
              mc[ i, "normAdeGrow", j, m ] <- NA
          }

          if( is.finite( mc[ i, "lnormOe", j, m ] ) &&
              is.finite( mc[ i - 1, "lnormOe", j, m ] ) ) {

            mc[ i, "lnormOeMatch", j, m ] <- mc[ i, "lnormOe", j, m ]
            mc[ i, "lnormOeMatchLag", j, m ] <- mc[ i - 1, "lnormOe", j, m ]
            mc[ i, "lnormOeGrow", j, m ] <- mc[ i, "lnormOe", j, m ] -
                                                 mc[ i - 1, "lnormOe", j, m ]

            if( ! is.finite( mc[ i, "lnormOeGrow", j, m ] ) ) {
              mc[ i, "lnormOeGrow", j, m ] <- NA
              mc[ i, "lnormOeMatch", j, m ] <- NA
              mc[ i, "lnormOeMatchLag", j, m ] <- NA
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

CI        <- 0.95   # desired confidence interval
nBins     <- 15     # number of bins to use in histograms
outLim    <- 0.001  # outlier percentile (0=don't remove outliers)
warmUp    <- 200    # number of "warm-up" runs
nTstat    <- -1     # last period to consider for statistics (-1=all)

repName   <- ""     # report files base name (if "" same baseName)
sDigits   <- 4      # significant digits in tables
plotRows  <- 1      # number of plots per row in a page
plotCols  <- 1      # number of plots per column in a page
plotW     <- 10     # plot window width
plotH     <- 7      # plot window height

# Colors assigned to each experiment's lines in graphics
colors <- c( "black", "blue", "red", "orange", "green", "brown" )

# Line types assigned to each experiment
lTypes <- c( "solid", "solid", "solid", "solid", "solid", "solid" )

# Point types assigned to each experiment
pTypes <- c( 4, 4, 4, 4, 4, 4 )


# ====== External support functions & definitions ======

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
pdf( paste0( folder, "/", repName, "_firmE_pool.pdf" ),
     width = plotW, height = plotH )
par( mfrow = c ( plotRows, plotCols ) )             # define plots per page

szData <- oData <- oDEdata <- oGEdata <- Adata <- gData <- gDEdata <- gGEdata <- ngData <- ngAdata <-
  exData <- svData <- bins <- list( )

# run over all experiments individually
for( k in 1 : nExp ){

  # load pooled data from temporary files (first already loaded)
  if( k > 1 )
    load( tmpFiles[[ k ]] )

  rm( mc )                   # erase unused variables

  #
  # ------ Create data & statistics vectors ------
  #

  szData[[ k ]] <- as.vector( pool[ TmaskStat, "Qe", ] )
  oData[[ k ]] <- as.vector( logNA( pool[ TmaskStat, "normOe", ] ) )
  oDEdata[[ k ]] <- as.vector( logNA( pool[ TmaskStat, "normOde", ] ) )
  oGEdata[[ k ]] <- as.vector( logNA( pool[ TmaskStat, "normOge", ] ) )
  Adata[[ k ]] <- as.vector( logNA( pool[ TmaskStat, "normAde", ] ) )
  gData[[ k ]] <- as.vector( pool[ TmaskStat, "growthE", ] )
  gDEdata[[ k ]] <- as.vector( pool[ TmaskStat, "growthDE", ] )
  gGEdata[[ k ]] <- as.vector( pool[ TmaskStat, "growthDE", ] )
  ngData[[ k ]] <- as.vector( pool[ TmaskStat, "normOeGrow", ] )
  ngAdata[[ k ]] <- as.vector( pool[ TmaskStat, "normAdeGrow", ] )
  exData[[ k ]] <- as.vector( as.numeric( is.na( pool[ TmaskStat, "growthE", ] ) ) )

  # remove NAs
  szData[[ k ]] <- szData[[ k ]][ ! is.na( szData[[ k ]] ) ]
  oData[[ k ]] <- oData[[ k ]][ ! is.na( oData[[ k ]] ) ]
  oDEdata[[ k ]] <- oDEdata[[ k ]][ ! is.na( oDEdata[[ k ]] ) ]
  oGEdata[[ k ]] <- oGEdata[[ k ]][ ! is.na( oGEdata[[ k ]] ) ]
  Adata[[ k ]] <- Adata[[ k ]][ ! is.na( Adata[[ k ]] ) ]
  gData[[ k ]] <- gData[[ k ]][ ! is.na( gData[[ k ]] ) ]
  gDEdata[[ k ]] <- gDEdata[[ k ]][ ! is.na( gDEdata[[ k ]] ) ]
  gGEdata[[ k ]] <- gGEdata[[ k ]][ ! is.na( gGEdata[[ k ]] ) ]
  ngData[[ k ]] <- ngData[[ k ]][ ! is.na( ngData[[ k ]] ) ]
  ngAdata[[ k ]] <- ngAdata[[ k ]][ ! is.na( ngAdata[[ k ]] ) ]
  exData[[ k ]] <- exData[[ k ]][ ! is.na( exData[[ k ]] ) ]

  # compute each variable statistics
  stats <- lapply( list( oData[[ k ]], oDEdata[[ k ]], oGEdata[[ k ]],
                         Adata[[ k ]], gData[[ k ]], gDEdata[[ k ]],
                         gGEdata[[ k ]], ngData[[ k ]], ngAdata[[ k ]],
                         exData[[ k ]] ),
                   comp_stats )

  o <- stats[[ 1 ]]
  oDE <- stats[[ 2 ]]
  oGE <- stats[[ 3 ]]
  A <- stats[[ 4 ]]
  g <- stats[[ 5 ]]
  gDE <- stats[[ 6 ]]
  gGE <- stats[[ 7 ]]
  ng <- stats[[ 8 ]]
  ngA <- stats[[ 9 ]]
  ex <- stats[[ 10 ]]

  # prepare data for Gibrat and scaling variance plots
  bins[[ k ]] <- size_bins( as.vector( pool[ TmaskStat, "lnormOeMatch", ] ),
                            as.vector( pool[ TmaskStat, "lnormOeMatchLag", ] ),
                            as.vector( pool[ TmaskStat, "lnormOeGrow", ] ),
                            bins = 2 * nBins, outLim = outLim )

  #
  # ------ Build statistics table ------
  #

  key.stats <- matrix( c( o$avg, A$avg, g$avg, ng$avg, ngA$avg, ex$avg,

                          o$sd / sqrt( nSize ), A$sd / sqrt( nSize ),
                          oDE$sd / sqrt( nSize ), oGE$sd / sqrt( nSize ),
                          g$sd / sqrt( nSize ), gDE$sd / sqrt( nSize ),
                          gGE$sd / sqrt( nSize ), ng$sd / sqrt( nSize ),
                          ngA$sd / sqrt( nSize ), ex$sd / sqrt( nSize ),

                          o$sd, oDE$sd, oGE$sd, A$sd, g$sd, gDE$sd,
                          gGE$sd, ng$sd, ngA$sd, ex$sd,

                          o$subbo$b, oDE$subbo$b, oGE$subbo$b, A$subbo$b,
                          g$subbo$b, gDE$subbo$b, gGE$subbo$b, ng$subbo$b,
                          ngA$subbo$b, ex$subbo$b,

                          o$subbo$a, oDE$subbo$a, oGE$subbo$a, A$subbo$a,
                          g$subbo$a, gDE$subbo$a, gGE$subbo$a, ng$subbo$a,
                          ngA$subbo$a, ex$subbo$a,

                          o$subbo$m, oDE$subbo$m, oGE$subbo$m, A$subbo$m,
                          g$subbo$m, gDE$subbo$m, gGE$subbo$m, ng$subbo$m,
                          ngA$subbo$m, ex$subbo$m,

                          o$jb$statistic, oDE$jb$statistic, oGE$jb$statistic,
                          A$jb$statistic, g$jb$statistic, gDE$jb$statistic,
                          gGE$jb$statistic, ng$jb$statistic, ngA$jb$statistic,
                          ex$jb$statistic,

                          o$jb$p.value, oDE$jb$p.value, oGE$jb$p.value,
                          A$jb$p.value, g$jb$p.value, gDE$jb$p.value,
                          gGE$jb$p.value, ng$jb$p.value, ngA$jb$p.value,
                          ex$jb$p.value,

                          o$ll$statistic, oDE$ll$statistic, oGE$ll$statistic,
                          A$ll$statistic, g$ll$statistic, gDE$ll$statistic,
                          gGE$ll$statistic, ng$ll$statistic, ngA$ll$statistic,
                          ex$ll$statistic,

                          o$ll$p.value, oDE$ll$p.value, oGE$ll$p.value,
                          A$ll$p.value, g$ll$p.value, gDE$ll$p.value,
                          gGE$ll$p.value, ng$ll$p.value, ngA$ll$p.value,
                          ex$ll$p.value,

                          o$ad$statistic, oDE$ad$statistic, oGE$ad$statistic,
                          A$ad$statistic, g$ad$statistic, gDE$ad$statistic,
                          gGE$ad$statistic, ng$ad$statistic, ngA$ad$statistic,
                          ex$ad$statistic,

                          o$ad$p.value, oDE$ad$p.value, oGE$ad$p.value,
                          A$ad$p.value, g$ad$p.value, gDE$ad$p.value,
                          gGE$ad$p.value, ng$ad$p.value, ngA$ad$p.value,
                          ex$ad$p.value,

                          o$ac$t1, oDE$ac$t1, oGE$ac$t1, A$ac$t1, g$ac$t1,
                          gDE$ac$t1, gGE$ac$t1, ng$ac$t1, ngA$ac$t1, ex$ac$t1,

                          o$ac$t2, oDE$ac$t2, oGE$ac$t2, A$ac$t2, g$ac$t2,
                          gDE$ac$t2, gGE$ac$t2, ng$ac$t2, ngA$ac$t2, ex$ac$t2 ),

                       ncol = 10, byrow = TRUE )
  colnames( key.stats ) <- c( "N.Power(log)", "N.D.Power(log)", "N.G.Power(log)",
                              "N.T.Eff.(log)", "Power Gr.", "D.Power Gr.",
                              "G.Power Gr.", "N.Output Gr.", "N.Prod.Gr.",
                              "Exit Rate")
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

  if( ! is.na( bins[[ k ]] ) ) {

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
}

#
# ====== Plot distributions ( overplots )  ======
#

# ------ Size and efficiency  distributions ( binned density x log variable in level )  ------

plot_norm( oData, "log(normalized size)", "Binned density",
           "Pooled size (power) distribution ( all experiments )",
           sector, outLim, nBins, legends, colors, lTypes, pTypes )

plot_norm( Adata, "log(normalized thermal efficiency)", "Binned density",
           "Pooled efficiency distribution ( all experiments )",
           sector, outLim, nBins, legends, colors, lTypes, pTypes )

# ------ Size distributions ( log size x rank )  ------

plot_lognorm( szData, "Size", "Rank",
              "Pooled size (power) distribution ( all experiments )",
              sector, outLim, nBins, legends, colors, lTypes )

# ------ Growth rate distributions ( binned density x growth rate )  ------

plot_laplace( gData, "Size growth rate", "Binned density",
              "Pooled size (power) growth rate distribution ( all experiments )",
              sector, outLim, nBins, legends, colors, lTypes, pTypes )

plot_laplace( ngData, "Normalized size growth rate", "Binned density",
              "Pooled size (power) growth rate distribution ( all experiments )",
              sector, outLim, nBins, legends, colors, lTypes, pTypes )

plot_laplace( ngAdata, "Normalized thermal efficiency growth rate", "Binned density",
              "Pooled efficiency growth rate distribution ( all experiments )",
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
pdf( paste0( folder, "/", repName, "_firmE_MC.pdf" ),
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

  oMC <- oDEMC <- oGEMC <- AMC <- gMC <- gDEMC <- gGEMC <- ngMC <- ngAMC <-
    exMC <- list( )

  for( l in 1 : nSize ){          # for each MC run

    # ------ Update MC statistics lists ------

    oData <- logNA( mc[ TmaskStat, "Qe", , l ] )
    oDEdata <- logNA( mc[ TmaskStat, "Qde", , l ] )
    oGEdata <- logNA( mc[ TmaskStat, "Qge", , l ] )
    Adata <- logNA( mc[ TmaskStat, "Ade", , l ] )
    gData <- mc[ TmaskStat,"growthE", , l ]
    gDEdata <- mc[ TmaskStat,"growthDE", , l ]
    gGEdata <- mc[ TmaskStat,"growthGE", , l ]
    ngData <- mc[ TmaskStat,"normOeGrow", , l ]
    ngAdata <- mc[ TmaskStat,"normAdeGrow", , l ]
    exData <- as.numeric( is.na( mc[ TmaskStat, "growthE", , l ] ) )

    # compute each variable statistics
    stats <- lapply( list( oData, oDEdata, oGEdata, Adata, gData, gDEdata,
                           gGEdata, ngData, ngAdata, exData ),
                     comp_stats )

    oMC[[l]] <- stats[[ 1 ]]
    oDEMC[[l]] <- stats[[ 2 ]]
    oGEMC[[l]] <- stats[[ 3 ]]
    AMC[[l]] <- stats[[ 4 ]]
    gDEMC[[l]] <- stats[[ 5 ]]
    gGEMC[[l]] <- stats[[ 6 ]]
    gMC[[l]] <- stats[[ 7 ]]
    ngMC[[l]] <- stats[[ 8 ]]
    ngAMC[[l]] <- stats[[ 9 ]]
    exMC[[l]] <- stats[[ 10 ]]
  }

  # ------ Compute MC statistics vectors ------

  # compute each variable statistics
  stats <- lapply( list( oMC, oDEMC, oGEMC, AMC, gMC, gDEMC, gGEMC, ngMC,
                         ngAMC, exMC ), comp_MC_stats )

  o <- stats[[ 1 ]]
  oDE <- stats[[ 2 ]]
  oGE <- stats[[ 3 ]]
  A <- stats[[ 4 ]]
  g <- stats[[ 5 ]]
  gDE <- stats[[ 6 ]]
  gGE <- stats[[ 7 ]]
  ng <- stats[[ 8 ]]
  ngA <- stats[[ 9 ]]
  ex <- stats[[ 10 ]]

  # ------ Build statistics table ------

  key.stats <- matrix( c( o$avg$avg, oDE$avg$avg, oGE$avg$avg, A$avg$avg,
                          g$avg$avg, gDE$avg$avg, gGE$avg$avg, ng$avg$avg,
                          ngA$avg$avg, ex$avg$avg,

                          o$se$avg, oDE$se$avg, oGE$se$avg, A$se$avg,
                          g$se$avg, gDE$se$avg, gGE$se$avg, ng$se$avg,
                          ngA$se$avg, ex$se$avg,

                          o$sd$avg, oDE$sd$avg, oGE$sd$avg, A$sd$avg,
                          g$sd$avg, gDE$sd$avg, gGE$sd$avg, ng$sd$avg,
                          ngA$sd$avg, ex$sd$avg,

                          o$avg$subbo$b, oDE$avg$subbo$b, oGE$avg$subbo$b,
                          A$avg$subbo$b, g$avg$subbo$b, gDE$avg$subbo$b,
                          gGE$avg$subbo$b, ng$avg$subbo$b, ngA$avg$subbo$b,
                          ex$avg$subbo$b,

                          o$se$subbo$b, oDE$se$subbo$b, oGE$se$subbo$b,
                          A$se$subbo$b, g$se$subbo$b, gDE$se$subbo$b,
                          gGE$se$subbo$b, ng$se$subbo$b, ngA$se$subbo$b,
                          ex$se$subbo$b,

                          o$sd$subbo$b, oDE$sd$subbo$b, oGE$sd$subbo$b,
                          A$sd$subbo$b, g$sd$subbo$b, gDE$sd$subbo$b,
                          gGE$sd$subbo$b, ng$sd$subbo$b, ngA$sd$subbo$b,
                          ex$sd$subbo$b,

                          o$avg$subbo$a, oDE$avg$subbo$a, oGE$avg$subbo$a,
                          A$avg$subbo$a, g$avg$subbo$a, gDE$avg$subbo$a,
                          gGE$avg$subbo$a, ng$avg$subbo$a, ngA$avg$subbo$a,
                          ex$avg$subbo$a,

                          o$se$subbo$a, oDE$se$subbo$a, oGE$se$subbo$a,
                          A$se$subbo$a, g$se$subbo$a, gDE$se$subbo$a,
                          gGE$se$subbo$a, ng$se$subbo$a, ngA$se$subbo$a,
                          ex$se$subbo$a,

                          o$sd$subbo$a, oDE$sd$subbo$a, oGE$sd$subbo$a,
                          A$sd$subbo$a, g$sd$subbo$a, gDE$sd$subbo$a,
                          gGE$sd$subbo$a, ng$sd$subbo$a, ngA$sd$subbo$a,
                          ex$sd$subbo$a,

                          o$avg$subbo$m, oDE$avg$subbo$m, oGE$avg$subbo$m,
                          A$avg$subbo$m, g$avg$subbo$m, gDE$avg$subbo$m,
                          gGE$avg$subbo$m, ng$avg$subbo$m, ngA$avg$subbo$m,
                          ex$avg$subbo$m,

                          o$se$subbo$m, oDE$se$subbo$m, oGE$se$subbo$m,
                          A$se$subbo$m, g$se$subbo$m, gDE$se$subbo$m,
                          gGE$se$subbo$m, ng$se$subbo$m, ngA$se$subbo$m,
                          ex$se$subbo$m,

                          o$sd$subbo$m, oDE$sd$subbo$m, oGE$sd$subbo$m,
                          A$sd$subbo$m, g$sd$subbo$m, gDE$sd$subbo$m,
                          gGE$sd$subbo$m, ng$sd$subbo$m, ngA$sd$subbo$m,
                          ex$sd$subbo$m,

                          o$avg$ac$t1, oDE$avg$ac$t1, oGE$avg$ac$t1, A$avg$ac$t1,
                          g$avg$ac$t1, gDE$avg$ac$t1, gGE$avg$ac$t1,
                          ng$avg$ac$t1, ngA$avg$ac$t1, ex$avg$ac$t1,

                          o$se$ac$t1, oDE$se$ac$t1, oGE$se$ac$t1, A$se$ac$t1,
                          g$se$ac$t1, gDE$se$ac$t1, gGE$se$ac$t1,
                          ng$se$ac$t1, ngA$se$ac$t1, ex$se$ac$t1,

                          o$sd$ac$t1, oDE$sd$ac$t1, oGE$sd$ac$t1, A$sd$ac$t1,
                          g$sd$ac$t1, gDE$sd$ac$t1, gGE$sd$ac$t1,
                          ng$sd$ac$t1, ngA$sd$ac$t1, ex$sd$ac$t1,

                          o$avg$ac$t2, oDE$avg$ac$t2, oGE$avg$ac$t2, A$avg$ac$t2,
                          g$avg$ac$t2, gDE$avg$ac$t2, gGE$avg$ac$t2,
                          ng$avg$ac$t2, ngA$avg$ac$t2, ex$avg$ac$t2,

                          o$se$ac$t2, oDE$se$ac$t2, oGE$se$ac$t2, A$se$ac$t2,
                          g$se$ac$t2, gDE$se$ac$t2, gGE$se$ac$t2,
                          ng$se$ac$t2, ngA$se$ac$t2, ex$se$ac$t2,

                          o$sd$ac$t2, oDE$sd$ac$t2, oGE$sd$ac$t2, A$sd$ac$t2,
                          g$sd$ac$t2, gDE$sd$ac$t2, gGE$sd$ac$t2,
                          ng$sd$ac$t2, ngA$sd$ac$t2, ex$sd$ac$t2 ),

                       ncol = 10, byrow = TRUE )
  colnames( key.stats ) <- c( "N.Power(log)", "N.D.Power(log)", "N.G.Power(log)",
                              "N.T.Eff.(log)", "Power Gr.", "D.Power Gr.",
                              "G.Power Gr.", "N.Output Gr.", "N.Prod.Gr.",
                              "Exit Rate")
  rownames( key.stats ) <- c( "average", " (s.e.)", " (s.d.)", "Subbotin b",
                              " (s.e.)", " (s.d.)", "Subbotin a", " (s.e.)",
                              " (s.d.)", "Subbotin m", " (s.e.)", " (s.d.)",
                              "autocorr. t-1", " (s.e.)",  " (s.d.)",
                              "autocorr. t-2", " (s.e.)",  " (s.d.)" )

  textplot( formatC( key.stats, digits = sDigits, format = "g" ), cmar = 1.0 )
  title <- paste( "Monte Carlo firm-level statistics (", legends[ k ], ")" )
  subTitle <- paste( eval( bquote( paste0( "( Sample size = ", nFirms,
                                           " firms / MC runs = ", nSize,
                                           " / Period = ", warmUp + 1, "-",
                                           nTstat, " )" ) ) ),
                    sector, sep ="\n" )
  title( main = title, sub = subTitle )
}


# Close PDF plot file
dev.off( )
