#******************************************************************
#
# ---------- K+S capital-good technologies analysis -------------
#
#   Written by Marcelo C. Pereira, University of Campinas
#
#   Copyright Marcelo C. Pereira
#   Distributed under the GNU General Public License
#
#   The default configuration assumes that the supplied LSD
#   simulation configurations (basename Sim):
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

# ====== User parameters ======

# data import parameters
folder    <- "data"                 # data files folder
baseName  <- "Sim"                  # data files base name (same as .lsd file)
nExp      <- 2                      # number of experiments/countries
mCnt      <- FALSE                  # experiments from multiple countries?
nCnt      <- 0                      # country to use (mCnt = FALSE) (0=all)
iniDrop   <- 0                      # initial time steps to drop (0=none)
nKeep     <- -1                     # number of time steps to keep (-1=all)
coresExp  <- 0                      # max cores for experiments (0=all)
coresMC   <- 0                      # max cores for Monte Carlo (0=all)
savDat    <- FALSE                  # save data files and re-use if available?
mcStat    <- "median"               # Monte Carlo statistic ("mean", "median")
CI        <- 0.95                   # confidence level
bootR     <- 999                    # bootstrap replicates (bootCI != NULL)
bootCI    <- NULL                   # bootstrap confidence interval method (SLOW)
                                    # (NULL (no bootstrap), "basic", or "bca")

# caption and file names
expVal    <- c( "Only unionized firms", "Mixed firms" )   # experiment captions
cntVal    <- c( "Country 1", "Country 2" )                # country captions
caption   <- "K+S technology analysis"                    # caption for logs
datFilSfx <- "tech"                                       # data file suffix

# aggregated variables to import and add
origVar   <- c( "g1front", "g1max", "_fG" )
addVar    <- c( )


# ==== Libraries, support functions and global options ====

source( "KS-support-functions.R" )

options( warn = 0 )         # -1=no warning/0:warnings at end/2:warnings stop

# remove RStudio warnings for support functions
# !diagnostics suppress = showStartMark, showEndMark, setCmdLinePars, loadData
# !diagnostics suppress = catchError, startTime, mean.clean, vector.clean, adrop
# !diagnostics suppress = plot_lists, mc, pool, P, M, S, m, nTsteps, nSize
# !diagnostics suppress = legends, expLeg, cntLeg, repFile, setLabels, clearTemp


# ==== Process LSD result files ====

showStartMark( caption )  # log start mark

setCmdLinePars( )         # read command line parameters (if any)

# read LSD files or load existing temporary files according to the case
files <- loadData( savDat = savDat, nExp = nExp, mCnt = mCnt, nCnt = nCnt,
                   folder = folder, baseName = baseName, iniDrop = iniDrop,
                   nKeep = nKeep, origVar = origVar, addVar = addVar,
                   datFilSfx = datFilSfx, coresExp = coresExp,
                   coresMC = coresMC, mcStat = mcStat, CI = CI,
                   bootR = bootR, bootCI = bootCI )


#******************************************************************
#
# --------------------- Plot statistics -------------------------
#
#******************************************************************

# ====== User parameters ======

nBins     <- 15     # number of bins to use in histograms
outLim    <- 0.001  # outlier percentile (0=don't remove outliers)
warmUp    <- 0      # number of "warm-up" runs
nTstat    <- -1     # last period to consider for statistics (-1=all)
frontPlt  <- TRUE   # plot all technological frontier plots in a separate file?

cores     <- 1      # maximum number of cores to allocate (0=all)
parStats  <- 1      # number of statistics to be computed in parallel
repName   <- ""     # report files base name (if "" same baseName)
transMk   <- -1     # regime transition mark after warm-up (-1:none)
sDigits   <- 4      # significant digits in tables
plotRows  <- 1      # number of plots per row in a page
plotCols  <- 1  	  # number of plots per column in a page
plotW     <- 10     # plot window width
plotH     <- 7      # plot window height
raster    <- FALSE  # raster or vector plots
res       <- 600    # resolution of raster mode (in dpi)

# colors assigned to each experiment's lines in graphics
colors <- c( "black", "blue", "red", "orange", "green", "brown", "cyan",
             "firebrick", "gray", "magenta", "orchid", "pink", "purple",
             "salmon", "sienna", "tomato", "turquoise", "violet", "yellow" )
#colors <- c( "black", "black", "black", "black", "black", "black" )

# line types assigned to each experiment
lTypes <- c( "solid", "solid", "solid", "solid", "solid", "solid" )
#lTypes <- c( "solid", "dashed", "dotted", "dotdash", "longdash", "twodash" )

# point types assigned to each experiment
pTypes <- c( 4, 4, 4, 4, 4, 4 )
#pTypes <- c( 4, 0, 1, 2, 3, 5 )


# ====== Support stuff ======

# generate labels & build labels list legend
setLabels( nExp, mCnt, nCnt )

# load data from first experiment
load( files[[ 1 ]]$pool )

# number of periods to show in graphics and use in statistics
if( nTstat < 1 || nTstat > nTsteps || nTstat <= warmUp )
  nTstat <- nTsteps
TmaskStat <- ( warmUp + 1 ) : nTstat


# ====== Main code ======

tryCatch( {   # enter error handling mode so PDF can be closed in case of error

  # create the report file(s) in a daily output directory
  repFile( repName, folder = folder, suffix = datFilSfx, width = plotW,
           height = plotH, rows = plotRows, cols = plotCols,
           raster = raster, res = res )

  #
  # ===== Monte Carlo analysis =====
  #

  cat( "\nGenerating MC reports...\n" )

  for( k in 1 : nExp ) {                                # do for each experiment

    cat( "", expLeg, k, "of", nExp, "\n" )

    # ------ compute single MC statistics ------

    for( j in 1 : nSize ) {                             # for each MC case

      cat( "  Monte Carlo case", j, "of", nSize, "\n" )

      # load MC data from temporary files
      load( files[[ k ]]$mc[ j ] )

      # ------ Update MC statistics ------

      # remove zeros and empty columns
      fG <- mc[ TmaskStat, "fG", ]
      fG <- replace ( fG, which( fG <= 0 ), NA )
      fG <- fG[ , colSums( is.na( fG ) ) < nrow( fG ) ]

      # ------ Plot time series ------

      plot( fG[ , 1 ], type = "l", col = colors[ 1 ], lty = lTypes[ 1 ],
            ylim = c( 0, 1 ),
            xlab = "Time", ylab = "Machine-generation capital stock share",
            main = paste( "Diffusion of new technologies sample (", legends[ k ], ")" ),
            sub = paste( "MC case =", j, "/ period =",
                         TmaskStat[ 1 ], "-", TmaskStat[ length( TmaskStat ) ],
                         cntLeg ) )

      for( i in 2 : ncol( fG ) )
        lines( fG[ , i ], col = colors[ i %% length( colors ) ],
               lty = lTypes[ i %% length( lTypes ) ] )
    }
  }

  #
  # ======= TECHNOLOGICAL FRONTIER REPORT =======
  #

  if( frontPlt ) {

    cat( "\nGenerating technological frontier report...\n" )

    # close main report and open new one
    repFile( repName, folder = folder, suffix = "tech_front", width = plotW,
             height = plotH, rows = plotRows, cols = plotCols,
             raster = raster, res = res )

    data <- list( )

    # Plot all runs
    for( k in 1 : nExp ) {                                # for each experiment

      cat( "", expLeg, k, "of", nExp, "\n" )

      for( j in 1 : nSize ) {                             # for each MC case

        cat( "  Monte Carlo case", j, "of", nSize, "\n" )

        # load MC data from temporary files and drop multiple instances
        load( files[[ k ]]$mc[ j ] )
        data[[ 1 ]] <- adrop( mc[ , , 1, drop = FALSE ], drop = 3 )

        # ------ Update MC statistics ------

        plot_lists( c( "g1front", "g1max" ), data, data, data, statMC = mcStat,
                    leg = legends, mask = TmaskStat, nMC = nSize, CI = CI,
                    mrk = transMk, col = colors, lty = lTypes,mCnt = mCnt,
                    xlab = "Time",
                    ylab = "Machine-generation sequential identification",
                    tit = paste( "Technological frontier sample" ),
                    subtit = paste( "MC case =", j, "/ period =", warmUp + 1,
                                    "-", nTstat, cntLeg ),
                    leg2 = c( "Technological frontier",
                              "Most advanced in production" ) )
      }
    }
  }


  clearTemp( savDat = savDat, files = files, nExp = nExp,
             folder = folder, baseName = baseName, datFilSfx = datFilSfx )


  # ------ End of report ------

}, interrupt = catchError, error = catchError, finally = showEndMark( ) )
