# KS-IRF-PAPER.R: SCRIPT FOR IMPULSE RESPONSE FUNCTION ANALYSIS USED IN PAPER
#
# This script assumes that the LSD simulation configurations are:
#   R/data/irf-noshock.lsd     (non-shocked runs)
#   R/data/irf-shock.lsd       (shocked runs)
# and that the R working directory is set to the R subfolder in
# the model folder (where this script file is), and data produced
# by LSD is in the R/data-irf subfolder (default location LSD use).

# parameters used for analysis
folder     <- "data-irf"            # LSD data subfolder (relative)
noshockCfg <- "irf-noshock"         # non-shocked configuration
shockCfg   <- "irf-shock"           # non-shocked configuration
mcStat     <- "mean"                # MC statistic (mean/median)
shockVar   <- "rShock"              # shock variable

irfHor     <- 16                    # time horizon to compute IRF
irfRel     <- TRUE                  # F=absolute deviation, T=relative
limOutl    <- 10                    # thresh. multiple for outliers (0=off)
bootAlpha  <- 0.10                  # bootstrap conf. interval significance
bootR      <- 999                   # bootstrap conf. interval replicates
bootCI     <- "basic"               # bootstrap CI method (basic/bca)
treeN      <- 5000                  # number of trees in random forest
treeDep    <- 1                     # maximum depth of random trees
nodeMin    <- 45                    # final node min observations
varTry     <- 3                     # variables to try/sample per node
alpha      <- 0.05                  # significance for node differences
quantile   <- 10                    # number of discrete state quantiles

# LSD original variables to read from files
readVars   <- c( "GDPreal", "Bda",  "Q2u", "r", "rDeb", "rShock", "dGDP",
                 "dCPI", "rDeb", "HH1", "HH2", "HHb", "exit2fail", "Bfail",
                 "dA", "Loans", "GDPnom", "Pi1", "Pi2", "PiB", "NWb",
                 "NW1", "NW2")

# potential state-defining variables
stateVars  <- c( "Bda", "NWbGDP", "Q2u", "r", "dGDP", "dCPI","LoansGDP",
                 "NWf", "HH1", "HH2", "HHb", "exit2fail", "Bfail", "dA")

# check/load required libraries, install missing (internet required)
reqLibs <- c( "LSDirf", "LSDinterface", "quantreg", "gplots" )
for( lib in reqLibs ) {
  if( ! require( lib, character.only = TRUE, quietly = TRUE ) )
    install.packages( lib, verbose = FALSE )
  library( lib, character.only = TRUE, quietly = TRUE )
}

# function to add new variables based on existing (LSD) ones
addVars <- function( data ) {
  data$PiFgdp   <- ( data$Pi1 + data$Pi2 ) / data$GDPnom
  data$PiBgdp   <- data$PiB / data$GDPnom
  data$NWbGDP   <- data$NWb / data$GDPnom
  data$LoansGDP <- data$Loans / data$GDPnom
  data$NWf      <- ( data$NW1 + data$NW2 ) / data$GDPnom
  return( data )
}

# function to define IRF states according to the value of variables
evalState <- function( data ) {
  # vector of probabilities (quantiles) to split data
  n <- 2
  probs <- seq( 0, 1, 1 / n )
  dataQuant <- quantile( data[ , stateVar ], probs, na.rm = TRUE, type = 8 )
  return( findInterval( data[ , stateVar ], dataQuant, all.inside = TRUE ) )
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

# read data files (ignore first 100 steps to speed-up)
mc <- read.3d.lsd( list.files.lsd( folder, noshockCfg ), readVars,
                   skip = 100 )
mcShock <- read.3d.lsd( list.files.lsd( folder, shockCfg ), readVars,
                        skip = 100 )

# compute linear IRF of real GDP (figure 2)
# reference variable is the real (output terms) GDP here
linearIRF_GDP <- irf.lsd( data = mc, data.shock = mcShock, t.horiz = irfHor,
                          var.irf = "GDPreal", var.ref = "GDPreal",
                          var.shock = shockVar, irf.type = "none",
                          stat = mcStat, ci.R = bootR, ci.type = bootCI,
                          lim.outl = limOutl, alpha = bootAlpha )

plot( linearIRF_GDP, irf.type = "incr.irf", scale = 2, center = TRUE,
      col = "red", lwd = 2, lty.ci = 2, xlab = "Relative time after shock",
      ylab = "Impulse response", main = "GDP linear IRF",
      xaxt = 'n', cex.lab = 0.7, cex.axis = 0.7 )
axis( side = 1, at = c( 0, 4, 8, 12, 16 ), cex.axis = 0.7 )

plot( linearIRF_GDP, irf.type = "cum.irf", scale = 1, center = TRUE,
      col = "red", lwd = 2, lty.ci = 2, xlab = "Relative time after shock",
      ylab = "Cumulative impulse response", main = "GDP linearcumulative IRF",
      xaxt = 'n', cex.lab = 0.7, cex.axis = 0.7 )
axis( side = 1, at = c( 0, 4, 8, 12, 16 ), cex.axis = 0.7 )

# compute linear IRF of interest rate (figure 3a)
linearIRF_rDeb <- irf.lsd( data = mc, data.shock = mcShock, t.horiz = irfHor,
                           var.irf = "rDeb", var.shock = shockVar,
                           var.ref = NULL, irf.type = "none", stat = mcStat,
                           ci.R = bootR, ci.type = bootCI,
                           lim.outl = limOutl, alpha = bootAlpha )

plot( linearIRF_rDeb, irf.type = "incr.irf", scale = 2, center = TRUE,
      col = "red", lwd = 2, lty.ci = 2, xlab = "Relative time after shock",
      ylab = "Impulse response", main = "Interest rate linear IRF",
      xaxt = 'n', cex.lab = 0.7, cex.axis = 0.7 )
axis( side = 1, at = c( 0, 4, 8, 12, 16 ), cex.axis = 0.7 )

# compute linear IRF of failing rate of firms (figure 3b)
linearIRF_exit2 <- irf.lsd( data = mc, data.shock = mcShock, t.horiz = irfHor,
                            var.irf = "exit2fail", var.shock = shockVar,
                            var.ref = NULL, irf.type = "none", stat = mcStat,
                            ci.R = bootR, ci.type = bootCI,
                            lim.outl = limOutl, alpha = bootAlpha )

plot( linearIRF_exit2, irf.type = "incr.irf", scale = 2, center = TRUE,
      col = "red", lwd = 2, lty.ci = 2, xlab = "Relative time after shock",
      ylab = "Impulse response", main = "Firm bankrupticy rate linear IRF",
      xaxt = 'n', cex.lab = 0.7, cex.axis = 0.7 )
axis( side = 1, at = c( 0, 4, 8, 12, 16 ), cex.axis = 0.7 )

# compute linear IRF of profits of consumption-goods firms (figure 3c)
# reference variable is the nominal (money terms) GDP here
linearIRF_Pi2 <- irf.lsd( data = mc, data.shock = mcShock, t.horiz = irfHor,
                          var.irf = "Pi2", var.shock = shockVar,
                          var.ref = "GDPnom", irf.type = "none", stat = mcStat,
                          ci.R = bootR, ci.type = bootCI, lim.outl = limOutl,
                          alpha = bootAlpha )

plot( linearIRF_Pi2, irf.type = "incr.irf", scale = 2, center = TRUE,
      col = "red", lwd = 2, lty.ci = 2,  xlab = "Relative time after shock",
      ylab = "Impulse response", main = "Firm profits linear IRF",
      xaxt = 'n', cex.lab = 0.7, cex.axis = 0.7 )
axis( side = 1, at = c( 0, 4, 8, 12, 16 ), cex.axis = 0.7 )

# compute linear IRF of financial fragility (figure 3d)
linearIRF_Bda <- irf.lsd( data = mc, data.shock = mcShock, t.horiz = irfHor,
                          var.irf = "Bda", var.shock = shockVar,
                          var.ref = NULL, irf.type = "none", stat = mcStat,
                          ci.R = bootR, ci.type = bootCI, lim.outl = limOutl,
                          alpha = bootAlpha )

plot( linearIRF_Bda, irf.type = "incr.irf", scale = 2, center = TRUE,
      col = "red", lwd = 2, lty.ci = 2,  xlab = "Relative time after shock",
      ylab = "Impulse response", main = "Financial fragility linear IRF",
      xaxt = 'n', cex.lab = 0.7, cex.axis = 0.7 )
axis( side = 1, at = c( 0, 4, 8, 12, 16 ), cex.axis = 0.7 )

# compute state-dependent impulse response functions (figure 4)
# use median value of state variable to split the sample

# GDP growth rate splitting (figure 4a)
stateVar <- "dGDP"         # state variable to be used in evalState function
stateIRF <- state.irf.lsd( data = mc, irf = linearIRF_GDP,
                           state.vars = stateVar, eval.state = evalState,
                           metr.irf = irfMetric, add.vars = addVars,
                           irf.type = "none", ci.R = bootR, ci.type = bootCI,
                           alpha = bootAlpha )

plot( stateIRF, state = 0, irf.type = "cum.irf", scale = 1, center = TRUE,
      col = c( "green", "blue" ), col.ci = c( "green", "blue" ), lwd = 2,
      xlab = "Relative time after shock",
      ylab = "State-dependent Cumulative impulse response",
      leg = c( "Low growth state", "High growth state" ),
      main = "GDP growth rate cumulative state-dependent IRF",
      xaxt = 'n', cex.lab = 0.7, cex.axis = 0.7 )
axis( side = 1, at = c( 0, 4, 8, 12, 16 ), cex.axis = 0.7 )

# financial fragility splitting (figure 4b)
stateVar <- "Bda"          # state variable to be used in evalState function
stateIRF <- state.irf.lsd( data = mc, irf = linearIRF_GDP,
                           state.vars = stateVar, eval.state = evalState,
                           metr.irf = irfMetric, add.vars = addVars,
                           irf.type = "none", ci.R = bootR, ci.type = bootCI,
                           alpha = bootAlpha )

plot( stateIRF, state = 0, irf.type = "cum.irf", scale = 1, center = TRUE,
      col = c( "green", "blue" ), col.ci = c( "green", "blue" ), lwd = 2,
      xlab = "Relative time after shock",
      ylab = "State-dependent Cumulative impulse response",
      leg = c( "Low financial fragility state",
               "High financial fragility state"),
      main = "Financial fragility cumulative state-dependent IRF",
      xaxt = 'n', cex.lab = 0.7, cex.axis = 0.7 )
axis( side = 1, at = c( 0, 4, 8, 12, 16 ), cex.axis = 0.7 )

# Random-forest state identification (figure 6)
stateIdent <- state.ident.lsd( data = mc, irf = linearIRF_GDP,
                               state.vars = stateVars,  metr.irf = irfMetric,
                               add.vars = addVars, ntree = treeN,
                               maxdepth = treeDep, nodesize = nodeMin,
                               mtry = varTry, alpha = alpha,
                               quantile = quantile )

textplot( format( stateIdent$state.freq[ 1 : 10, ], digits = 3 ),
          main = "Top-10 identified states",
          cmar = 1, show.rownames = FALSE )

# state-dependent cumulative irf for the 3 most frequent states (figure 5)
stateIRF1 <- state.irf.lsd( data = mc, irf = linearIRF_GDP, states = stateIdent,
                            state.num = 1, metr.irf = irfMetric,
                            add.vars = addVars, irf.type = "none",
                            ci.R = bootR, ci.type = bootCI, alpha = bootAlpha )

plot( stateIRF1, state = 0, irf.type = "cum.irf", scale = 1, center = TRUE,
      col = c( "green", "blue" ), col.ci = c( "green", "blue" ), lwd = 2,
      xlab = "Relative time after shock",
      ylab = "State-dependent Cumulative impulse response",
      leg = c( paste( "Low interest rate", "state" ),
               paste( "High interest rate" , "state" ) ),
      main = "Cumulative state-dependent IRF for first more frequent state",
      xaxt = 'n', cex.lab = 0.7, cex.axis = 0.7 )
axis( side = 1, at = c( 0, 4, 8, 12, 16 ), cex.axis = 0.7 )

stateIRF2 <- state.irf.lsd( data = mc, irf = linearIRF_GDP, states = stateIdent,
                            state.num = 2, metr.irf = irfMetric,
                            add.vars = addVars, irf.type = "none",
                            ci.R = bootR, ci.type = bootCI, alpha = bootAlpha )

plot( stateIRF2, state = 0, irf.type = "cum.irf", scale = 1, center = TRUE,
      col = c( "blue", "green" ), col.ci = c( "blue", "green" ), lwd = 2,
      xlab = "Relative time after shock",
      ylab = "State-dependent Cumulative impulse response",
      leg = c( paste( "No banking crisis", "state" ),
               paste( "Banking crisis" , "state" ) ),
      main = "Cumulative state-dependent IRF for second more frequent state",
      xaxt = 'n', cex.lab = 0.7, cex.axis = 0.7 )
axis( side = 1, at = c( 0, 4, 8, 12, 16 ), cex.axis = 0.7 )

stateIRF3 <- state.irf.lsd( data = mc, irf = linearIRF_GDP, states = stateIdent,
                            state.num = 3, metr.irf = irfMetric,
                            add.vars = addVars, irf.type = "none",
                            ci.R = bootR, ci.type = bootCI, alpha = bootAlpha )

plot( stateIRF3, state = 0, irf.type = "cum.irf", scale = 1, center = TRUE,
      col = c( "green", "blue" ), col.ci = c( "green", "blue" ), lwd = 2,
      xlab = "Relative time after shock",
      ylab = "State-dependent Cumulative impulse response",
      leg = c( paste( "Low capacity ut. rate", "state" ),
               paste( "Normal/high capacity ut. rate" , "state" ) ),
      main = "Cumulative state-dependent IRF for third more frequent state",
      xaxt = 'n', cex.lab = 0.7, cex.axis = 0.7 )
axis( side = 1, at = c( 0, 4, 8, 12, 16 ), cex.axis = 0.7 )
