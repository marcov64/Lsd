#******************************************************************
#
# ----------- K+S Impulse-response function analysis ------------
#
#   Written by Marcelo C. Pereira, University of Campinas
#              Marco Amendola, University of Napoli
#
#   Copyright Marcelo C. Pereira & Marco Amendola
#   Distributed under the GNU General Public License
#
#   The default configuration assumes that the supplied LSD
#   simulation configurations (basename irf):
#     R/data/irf1.lsd
#     R/data/irf2.lsd
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
baseName  <- "irf"                  # data files base name (no-shock/shock:1/2)
iniDrop   <- 100                    # initial time steps to drop (0=none)
nKeep     <- -1                     # number of time steps to keep (-1=all)
mcStat    <- "median"               # Monte Carlo statistic ("mean", "median")

irfVar    <- "GDPreal"              # variable to compute impulse-response fun.
refVar    <- "GDPreal"              # reference var. to compute IRF as share (%)
shockVar  <- "rShock"               # shock variable name
stateVar  <- "Bda"                  # state variable to be used

# LSD original variables to read from files
readVars  <- c( "Bda", "Q2u", "dGDP", "r", "dCPI", "mu2avg", "DefPgdp", "N",
                "Q2", "PiB", "GDPnom", "Pi1", "Pi2", "NWb", "NW1", "NW2" )

# potential state-defining variables to consider, including added variables
stateVars <- c( "Bda", "NWbGDP", "PiFgdp", "Q2u", "mu2avg", "r" )


# ==== Process LSD result files ====

# load support packages and functions
source( "KS-support-functions.R" )

# ---- Read data files ----

mc <- read.3d.lsd( list.files.lsd( folder, paste0( baseName, "1" ) ),
                   c( readVars, irfVar, refVar ),
                   skip = iniDrop, nrows = nKeep )
mcShock <- read.3d.lsd( list.files.lsd( folder, paste0( baseName, "2" ) ),
                        c( shockVar, irfVar ),
                        skip = iniDrop, nrows = nKeep )


#******************************************************************
#
# --------------------- Plot statistics -------------------------
#
#******************************************************************

# ===================== User parameters =========================

irfHor    <- 20                     # time horizon to compute IRF
irfRel    <- TRUE                   # F=absolute deviation, T=relative deviation

limOutl   <- 3                      # limit threshold multiple for outliers (0=off)

bootAlpha <- 0.05                   # bootstrap confidence interval significance
bootR     <- 999                    # bootstrap confidence interval replicates
bootCI    <- "basic"                # bootstrap confidence interval method
                                    # ("basic" or "bca")

treeN     <- 1000                   # number of trees in random forest
treeDep   <- 2                      # maximum depth of random trees
nodeMin   <- 30                     # final node min number of observations
varTry    <- 2                      # number of variables to try/sample per node
alpha     <- 0.05                   # significance for node differences
quantile  <- 10                     # number of discrete state quantiles

repName   <- ""                     # report files base name (if "" same baseName)
sDigits   <- 4                      # significant digits in tables
plotRows  <- 1                      # number of plots per row in a page
plotCols  <- 1  	                  # number of plots per column in a page
plotW     <- 10                     # plot window width
plotH     <- 7                      # plot window height


# ====== Functions to process dataset ======

# function to define IRF states according to the value of state variable(s)
evalState <- function( data ) {

  # vector of probabilities (quantiles) to split data
  n <- 2
  probs <- seq( 0, 1, 1 / n )
  dataQuant <- quantile( data[ , stateVar ], probs, na.rm = TRUE, type = 8 )

  return( findInterval( data[ , stateVar ], dataQuant, all.inside = TRUE ) )
}

# function to add new state variables based on existing ones (readVars)
addVars <- function( data ) {

  data$PiFgdp <- ( data$Pi1 + data$Pi2 ) / data$GDPnom
  data$NWbGDP <- data$NWb / data$GDPnom

  return( data )
}

# function to compute IRF metric (higher values mean better performance)
irfMetric <- function( data ) {

  # cumulative irf time weights in reverse order (0 if not set)
  irfWght <- c( 1, 1, 1, 1 )

  metric <- rep( 0, nrow( data ) )
  irfWght <- irfWght[ 1 : min( ncol( data ), length( irfWght ) ) ]

  for( i in 1 : length( irfWght ) )
    metric <- metric - irfWght[ i ] * data[ , irfHor - i + 1 ]

  metric <- metric / sum( irfWght )

  return( metric )
}


# ====== External support functions & definitions ======

library( LSDirf )

# remove warnings for support functions and saved data
# !diagnostics suppress = irf.lsd, state.irf.lsd


# ==== Support stuff ====

if( repName == "" ) repName <- baseName
if( irfRel ) irType <- "Relative" else irType <- "Absolute"


# ====== Analyze IRF data ======

linearIRF <- irf.lsd( data = mc,              # non-shocked MC data
                      data.shock = mcShock,   # shocked data
                      t.horiz = irfHor,       # post-shock analysis time horizon
                      var.irf = irfVar,       # variable to compute IRF
                      var.shock = shockVar,   # shock variable (impulse)
                      var.ref = refVar,       # reference variable to IR measure
                      irf.type = "none",      # no plot now
                      stat = mcStat,          # type of statistic to use
                      ci.R = bootR,           # CI bootstrap repetitions (odd)
                      ci.type = bootCI,       # CI algorithm type
                      lim.outl = limOutl,     # outlier limit/threshold
                      alpha = bootAlpha )     # confidence interval conf. level

stateIRF <- state.irf.lsd(
                      data = mc,              # non-shock MC data
                      irf = linearIRF,        # linear IRF produced by irf.lsd()
                      state.vars = stateVar,  # variable defining states
                      eval.state = evalState, # function to evaluate state(s)
                      metr.irf = irfMetric,   # function to compare C-IR's
                      add.vars = addVars,     # function to add new variables
                      irf.type = "none",      # no plot now
                      ci.R = bootR,           # CI bootstrap repetitions (odd)
                      ci.type = bootCI,       # CI algorithm type
                      alpha = bootAlpha )     # confidence interval conf. level


# ====== Random-forest state identification ======

stateIdent <- state.ident.lsd(
                      data = mc,              # non-shock MC data
                      irf = linearIRF,        # linear IRF produced by irf.lsd()
                      state.vars = stateVars, # MC state variables to consider
                      metr.irf = irfMetric,   # function to compare C-IR's
                      add.vars = addVars,     # function to add new variables
                      ntree = treeN,          # number of trees in random forest
                      maxdepth = treeDep,     # maximum depth of random trees
                      nodesize = nodeMin,     # final node min number of observations
                      mtry = varTry,          # number of variable samples per node
                      alpha = alpha,          # significance for node differences
                      quantile = quantile )   # number of discrete state quantiles

stateSens <- state.sa.lsd(
                      data = mc,              # non-shock MC data
                      irf = linearIRF,        # linear IRF produced by irf.lsd()
                      state.vars = stateVars, # MC state variables to consider
                      metr.irf = irfMetric,   # function to compare C-IR's
                      add.vars = addVars,     # function to add new variables
                      ntree = treeN,          # number of trees in random forest
                      nodesize = nodeMin,     # final node min number of observations
                      mtry = varTry,          # number of variable samples per node
                      alpha = alpha,          # significance for node differences
                      no.plot = TRUE )        # do not plot yet


# ====== Analyze state-dependent IRF of top 3 identified states ======

stateIRF1 <- state.irf.lsd(
                      data = mc,              # non-shock MC data
                      irf = linearIRF,        # linear IRF produced by irf.lsd()
                      states = stateIdent,    # object with identified states
                      state.num = 1,          # number of identified state to analyze
                      metr.irf = irfMetric,   # function to compare C-IR's
                      add.vars = addVars,     # function to add new variables
                      irf.type = "none",      # no plot now
                      ci.R = bootR,           # CI bootstrap repetitions (odd)
                      ci.type = bootCI,       # CI algorithm type
                      alpha = bootAlpha )     # confidence interval conf. level

stateIRF2 <- state.irf.lsd(
                      data = mc,              # non-shock MC data
                      irf = linearIRF,        # linear IRF produced by irf.lsd()
                      states = stateIdent,    # object with identified states
                      state.num = 2,          # number of identified state to analyze
                      metr.irf = irfMetric,   # function to compare C-IR's
                      add.vars = addVars,     # function to add new variables
                      irf.type = "none",      # no plot now
                      ci.R = bootR,           # CI bootstrap repetitions (odd)
                      ci.type = bootCI,       # CI algorithm type
                      alpha = bootAlpha )     # confidence interval conf. level

stateIRF3 <- state.irf.lsd(
                      data = mc,              # non-shock MC data
                      irf = linearIRF,        # linear IRF produced by irf.lsd()
                      states = stateIdent,    # object with identified states
                      state.num = 3,          # number of identified state to analyze
                      metr.irf = irfMetric,   # function to compare C-IR's
                      add.vars = addVars,     # function to add new variables
                      irf.type = "none",      # no plot now
                      ci.R = bootR,           # CI bootstrap repetitions (odd)
                      ci.type = bootCI,       # CI algorithm type
                      alpha = bootAlpha )     # confidence interval conf. level


# ==== Create PDF ====

pdf( paste0( folder, "/", repName, "_irf_", irfVar, "_", stateVar, "_",  mcStat,
             ".pdf" ), width = plotW, height = plotH )
par( mfrow = c ( plotRows, plotCols ) )


#
# ------ Plot linear IRF ------
#

xlab <- "Relative time after shock"
col <- "red"

plot( linearIRF, irf.type = "incr", scale = 2, center = TRUE, col = col,
      lwd = 2, lty.ci = 2,  xlab = xlab, ylab = "Impulse response",
      main = paste( "Linear impulse-response function for", irfVar ),
      sub = paste( "( MC sample =", linearIRF$nsample, "/ MC", mcStat,
                   "/ CI signif. =",  linearIRF$alpha, ")" ) )

plot( linearIRF, irf.type = "cum", scale = 1, center = FALSE, col = col,
      lwd = 2, lty.ci = 2,  xlab = xlab, ylab = "Cumulative impulse response",
      main = paste( "Linear cumulative impulse-response function for", irfVar ),
      sub = paste( "( MC sample =", linearIRF$nsample, "/ MC", mcStat,
                   "/ CI signif. =",  linearIRF$alpha, ")" ) )


#
# ------ Plot state-dependent IRFs ------
#

col <- c( "green", "blue" )

plot( stateIRF, state = 0, irf.type = "incr", scale = 1, center = TRUE,
      col = col, lwd = 2, col.ci = col, xlab = xlab,
      ylab = "Impulse response by state",
      main = paste( "State-dependent impulse-response functions for", irfVar ),
      sub = paste( "( MC sample =", stateIRF$nsample, "/ MC", mcStat,
                   "/ CI signif. =",  linearIRF$alpha,
                   "/ U-test p-val =", signif( stateIRF$irf.test$p.value, 2 ),
                   "/ H0: similar states )" ),
      leg = c( paste( "Low", stateVar, "state" ),
               paste( "High", stateVar, "state" ) ) )

plot( stateIRF, state = 0, irf.type = "cum", scale = 1, center = FALSE,
      col = col, lwd = 2, col.ci = col, xlab = xlab,
      ylab = "Cumulative impulse response by state",
      main = paste( "State-dependent cumulative impulse-response functions for",
                    irfVar ),
      sub = paste( "( MC sample =", stateIRF$nsample, "/ MC", mcStat,
                   "/ CI signif. =", linearIRF$alpha,
                   "/ U-test p-val =", signif( stateIRF$cirf.test$p.value, 2 ),
                   "/ H0: similar states )" ),
      leg = c( paste( "Low", stateVar, "state" ),
               paste( "High", stateVar, "state" ) ) )

plot( stateIRF1, state = 0, irf.type = "incr", scale = 1, center = TRUE,
      col = col, lwd = 2, col.ci = col, xlab = xlab,
      ylab = "Impulse response by state",
      main = paste( "State-dependent IRF for", stateIRF1$state ),
      sub = paste( "( MC sample =", stateIRF1$nsample, "/ MC", mcStat,
                   "/ CI signif. =",  linearIRF$alpha,
                   "/ U-test p-val =", signif( stateIRF1$irf.test$p.value, 2 ),
                   "/ H0: similar states )" ),
      leg = c( paste( "Low", stateVar, "state" ),
               paste( "High", stateVar, "state" ) ) )

plot( stateIRF1, state = 0, irf.type = "cum", scale = 1, center = FALSE,
      col = col, lwd = 2, col.ci = col, xlab = xlab,
      ylab = "Cumulative impulse response by state",
      main = paste( "State-dependent C-IRF for", stateIRF1$state ),
      sub = paste( "( MC sample =", stateIRF1$nsample, "/ MC", mcStat,
                   "/ CI signif. =", linearIRF$alpha,
                   "/ U-test p-val =", signif( stateIRF1$cirf.test$p.value, 2 ),
                   "/ H0: similar states )" ),
      leg = c( paste( "Low", stateVar, "state" ),
               paste( "High", stateVar, "state" ) ) )

plot( stateIRF2, state = 0, irf.type = "incr", scale = 1, center = TRUE,
      col = col, lwd = 2, col.ci = col, xlab = xlab,
      ylab = "Impulse response by state",
      main = paste( "State-dependent IRF for", stateIRF2$state ),
      sub = paste( "( MC sample =", stateIRF2$nsample, "/ MC", mcStat,
                   "/ CI signif. =",  linearIRF$alpha,
                   "/ U-test p-val =", signif( stateIRF2$irf.test$p.value, 2 ),
                   "/ H0: similar states )" ),
      leg = c( paste( "Low", stateVar, "state" ),
               paste( "High", stateVar, "state" ) ) )

plot( stateIRF2, state = 0, irf.type = "cum", scale = 1, center = FALSE,
      col = col, lwd = 2, col.ci = col, xlab = xlab,
      ylab = "Cumulative impulse response by state",
      main = paste( "State-dependent C-IRF for", stateIRF2$state ),
      sub = paste( "( MC sample =", stateIRF2$nsample, "/ MC", mcStat,
                   "/ CI signif. =", linearIRF$alpha,
                   "/ U-test p-val =", signif( stateIRF2$cirf.test$p.value, 2 ),
                   "/ H0: similar states )" ),
      leg = c( paste( "Low", stateVar, "state" ),
               paste( "High", stateVar, "state" ) ) )

plot( stateIRF3, state = 0, irf.type = "incr", scale = 1, center = TRUE,
      col = col, lwd = 2, col.ci = col, xlab = xlab,
      ylab = "Impulse response by state",
      main = paste( "State-dependent IRF for", stateIRF3$state ),
      sub = paste( "( MC sample =", stateIRF3$nsample, "/ MC", mcStat,
                   "/ CI signif. =",  linearIRF$alpha,
                   "/ U-test p-val =", signif( stateIRF3$irf.test$p.value, 2 ),
                   "/ H0: similar states )" ),
      leg = c( paste( "Low", stateVar, "state" ),
               paste( "High", stateVar, "state" ) ) )

plot( stateIRF3, state = 0, irf.type = "cum", scale = 1, center = FALSE,
      col = col, lwd = 2, col.ci = col, xlab = xlab,
      ylab = "Cumulative impulse response by state",
      main = paste( "State-dependent C-IRF for", stateIRF3$state ),
      sub = paste( "( MC sample =", stateIRF3$nsample, "/ MC", mcStat,
                   "/ CI signif. =", linearIRF$alpha,
                   "/ U-test p-val =", signif( stateIRF3$cirf.test$p.value, 2 ),
                   "/ H0: similar states )" ),
      leg = c( paste( "Low", stateVar, "state" ),
               paste( "High", stateVar, "state" ) ) )


#
# ------ Show detected important states ------
#

rows <- 10

textplot( format( stateIdent$state.freq[ 1 : rows, ], digits = sDigits ),
          cmar = 1, show.rownames = FALSE )
title( main = paste( "Top", rows, "discrete-state frequency (",
                     stateIdent$var.ref, "C-IRF )" ),
       sub = paste( "( MC sample =", stateIdent$nsample, "/ Tree depth =",
                    stateIdent$maxdepth, "/ Significance =",
                    stateIdent$alpha, "/ Quantiles =",
                    stateIdent$quantile, " )" ) )


#
# ------ Random forest sensitivity analysis ------
#

plot( stateSens, xlab = "State-defining variables",
      main = paste( "Sensitivity analysis of state-defining variables (",
                    irfVar, "IRF )" ),
      sub = paste( "( MC sample =", stateSens$nsample, "/ MC", mcStat,
                   "/ Pseudo R2 =", signif( stateSens$rsq, digits = 2 ),
                   "/ CI signif. =", stateSens$alpha, ")" ) )


# close plot file
dev.off( )
