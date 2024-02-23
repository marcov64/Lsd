#   This script assumes that the supplied LSD simulation configurations are:
#     R/data/irf1.lsd            (non-shocked data)
#     R/data/irf2.lsd            (shocked data)
#   and that the R working directory is set to the R subfolder where this script file is stored.

#******************************************************************
# ------------ Read Monte Carlo experiment files ----------------
#******************************************************************

folder    <- "data"                 # data files folder
baseName  <- "irf"                  # data files base name (no-shock/shock:1/2)
iniDrop   <- 100                    # initial time steps to drop (0=none)
nKeep     <- -1                     # number of time steps to keep (-1=all)
mcStat    <- "mean"                 # Monte Carlo statistic ("mean", "median")

irfVar    <- "GDPreal"              # variable to compute impulse-response fun.
refVar    <- "GDPreal"              # reference var. to compute IRF as share (%)
shockVar  <- "rShock"               # shock variable name

# LSD original variables to read from files
readVars  <- c( "GDPreal", "Bda",  "Q2u", "r", "dGDP", "dCPI", "rDeb", 
                "HH1", "HH2", "HHb", "exit2fail", "Bfail", "dA", "Loans", "GDPnom", "Pi1",
                "Pi2", "PiB", "NWb", "NW1", "NW2")

# potential state-defining variables to consider, including added variables
stateVars <- c( "Bda", "NWbGDP", "Q2u", "r", "dGDP", "dCPI", "Loans_GDP", "NWF",
                "HH1", "HH2", "HHb", "exit2fail", "Bfail", "dA")

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

irfHor    <- 16                     # time horizon to compute IRF
irfRel    <- T                      # F=absolute deviation, T=relative deviation

limOutl   <- 10                     # limit threshold multiple for outliers (0=off)

bootAlpha <- 0.10                   # bootstrap confidence interval significance
bootR     <- 999                    # bootstrap confidence interval replicates
bootCI    <- "basic"                # bootstrap confidence interval method # ("basic" or "bca")

treeN     <- 5000                   # number of trees in random forest
treeDep   <- 1                      # maximum depth of random trees
nodeMin   <- 45                     # final node min number of observations
varTry    <- 3                      # number of variables to try/sample per node
alpha     <- 0.05                   # significance for node differences
quantile  <- 10                     # number of discrete state quantiles

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
  data$PiBgdp<- data$PiB / data$GDPnom
  data$NWbGDP <- data$NWb / data$GDPnom
  data$Loans_GDP <- data$Loans/ data$GDPnom
  data$NWF<- (data$NW1 + data$NW2) / data$GDPnom
  
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

if( irfRel ) irType <- "Relative" else irType <- "Absolute"


#******************************************************************
# Figure 2
#******************************************************************

#computing linear IRF of GDP
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

xlab <- "Relative time after shock"
col <- "red"
par(mfrow=c(1,2))

#plotting Robust linear impulse responses of GDP 
plot( linearIRF, irf.type = "incr.irf", scale = 2, center = TRUE, col = col,
      lwd = 2, lty.ci = 2,  xlab = xlab, ylab = "Impulse response",
      xaxt = 'n' , cex.lab=0.7, cex.axis=0.7)

axis(side=1,at=c(0,4,8,12,16), cex.axis=0.7)

#plotting Robust linear cumulative impulse responses of GDP 
plot( linearIRF, irf.type = "cum.irf", scale = 1, center = TRUE, col = col,
      lwd = 2, lty.ci = 2,  xlab = xlab, ylab = "Cumulative impulse response",
      xaxt = 'n', cex.lab=0.7, cex.axis=0.7  )

axis(side=1,at=c(0,4,8,12,16), cex.axis=0.7)

title("GDP", line= -2, outer = TRUE,  cex.main = 1)


#******************************************************************
# Figure 4
#******************************************************************

#Computing state-dependent impulse response functions, adopting median value of stateVar to split the sample 

#1) GDP Growth rate splitting

stateVar  <- "dGDP"                    # state variable to be used

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

par(mfrow=c(1,2))
col <- c( "green", "blue" )

plot( stateIRF, state = 0, irf.type = "cum.irf", scale = 1, center = TRUE,
      col = col, lwd = 2, col.ci = col, xlab = xlab, 
      ylab = "State-dependent Cumulative impulse response",
      xaxt = 'n', cex.lab=0.7, cex.axis=0.7,
      leg = c( "Low growth state", "High growth state") )

axis(side=1,at=c(0,4,8,12,16), cex.axis=0.7)

#2) Financial fragility rate splitting

stateVar  <- "Bda"                    # state variable to be used

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

plot( stateIRF, state = 0, irf.type = "cum.irf", scale = 1, center = TRUE,
      col = col, lwd = 2, col.ci = col, xlab = xlab, 
      ylab = "State-dependent Cumulative impulse response",
      xaxt = 'n', cex.lab=0.7, cex.axis=0.7,
      leg = c( "Low financial fragility state", "High financial fragility state") )

axis(side=1,at=c(0,4,8,12,16), cex.axis=0.7)

title("GDP", line= -2, outer = TRUE,  cex.main = 1)

#******************************************************************
# Figure 5-6
#******************************************************************

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

#Figure 6: plot 10 more frequent states

par(mfrow=c(1,1))

rows <- 10

textplot( format( stateIdent$state.freq[ 1 : rows, ], digits = 3 ),
          cmar = 1, show.rownames = FALSE )

#Figure 5: state-dependent cumulative irf for the 3 most frequent states

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


col <- c( "green", "blue" )
col1<- c("blue", "green")

par(mfrow=c(1,3))


plot( stateIRF1, state = 0, irf.type = "cum.irf", scale = 1, center = TRUE,
      col = col, lwd = 2, col.ci = col, xlab = xlab, 
      ylab = "State-dependent Cumulative impulse response",
      xaxt = 'n', cex.lab=0.7, cex.axis=0.7,
      leg = c( paste( "Low interest rate", "state" ),
               paste( "High interest rate" , "state") ))

axis(side=1,at=c(0,4,8,12,16), cex.axis=0.7)

plot( stateIRF3, state = 0, irf.type = "cum.irf", scale = 1, center = TRUE,
      col = col, lwd = 2, col.ci = col, xlab = xlab, 
      ylab = "State-dependent Cumulative impulse response",
      xaxt = 'n', cex.lab=0.7, cex.axis=0.7,
      leg = c( paste( "Low capacity ut. rate", "state" ),
               paste( "Normal/high capacity ut. rate" , "state") ))

axis(side=1,at=c(0,4,8,12,16), cex.axis=0.7)


plot( stateIRF2, state = 0, irf.type = "cum.irf", scale = 1, center = TRUE,
      col = col1, lwd = 2, col.ci = col1, xlab = xlab, 
      ylab = "State-dependent Cumulative impulse response",
      xaxt = 'n', cex.lab=0.7, cex.axis=0.7,
      leg = c( paste( "No banking crisis", "state" ),
               paste( "Banking crisis" , "state") ))

axis(side=1,at=c(0,4,8,12,16), cex.axis=0.7)

title("GDP", line= -2, outer = TRUE,  cex.main = 1)


#******************************************************************
# Figure 3
#******************************************************************

#computing linear IRF of INTEREST RATE

#Interest rate
irfVar <- "rDeb"

mc <- read.3d.lsd( list.files.lsd( folder, paste0( baseName, "1" ) ),
                   c( readVars, irfVar, refVar ),
                   skip = iniDrop, nrows = nKeep )
mcShock <- read.3d.lsd( list.files.lsd( folder, paste0( baseName, "2" ) ),
                        c( shockVar, irfVar ),
                        skip = iniDrop, nrows = nKeep )

linearIRF_INTRATE <- irf.lsd( data = mc,      # non-shocked MC data
                      data.shock = mcShock,   # shocked data
                      t.horiz = irfHor,       # post-shock analysis time horizon
                      var.irf = irfVar,       # variable to compute IRF
                      var.shock = shockVar,   # shock variable (impulse)
                      var.ref = NULL,         # reference variable to IR measure
                      irf.type = "none",      # no plot now
                      stat = mcStat,          # type of statistic to use
                      ci.R = bootR,           # CI bootstrap repetitions (odd)
                      ci.type = bootCI,       # CI algorithm type
                      lim.outl = limOutl,     # outlier limit/threshold
                      alpha = bootAlpha )     # confidence interval conf. level

xlab <- "Relative time after shock"
col <- "red"
par(mfrow=c(1,1))

#plotting Robust linear impulse responses of GDP 
plot( linearIRF_INTRATE, irf.type = "incr.irf", scale = 2, center = TRUE, col = col,
      lwd = 2, lty.ci = 2,  xlab = xlab, ylab = "Impulse response",
      xaxt = 'n' , cex.lab=0.7, cex.axis=0.7)

axis(side=1,at=c(0,4,8,12,16), cex.axis=0.7)

title("Interest rate on loans", line= -2, outer = TRUE,  cex.main = 1)

#Failing rate of firms

irfVar <- "exit2fail"

mc <- read.3d.lsd( list.files.lsd( folder, paste0( baseName, "1" ) ),
                   c( readVars, irfVar, refVar ),
                   skip = iniDrop, nrows = nKeep )
mcShock <- read.3d.lsd( list.files.lsd( folder, paste0( baseName, "2" ) ),
                        c( shockVar, irfVar ),
                        skip = iniDrop, nrows = nKeep )

linearIRF_EXIT <- irf.lsd( data = mc,      # non-shocked MC data
                              data.shock = mcShock,   # shocked data
                              t.horiz = irfHor,       # post-shock analysis time horizon
                              var.irf = irfVar,       # variable to compute IRF
                              var.shock = shockVar,   # shock variable (impulse)
                              var.ref = NULL,         # reference variable to IR measure
                              irf.type = "none",      # no plot now
                              stat = mcStat,          # type of statistic to use
                              ci.R = bootR,           # CI bootstrap repetitions (odd)
                              ci.type = bootCI,       # CI algorithm type
                              lim.outl = limOutl,     # outlier limit/threshold
                              alpha = bootAlpha )     # confidence interval conf. level

xlab <- "Relative time after shock"
col <- "red"
par(mfrow=c(1,1))

#plotting Robust linear impulse responses of GDP 
plot( linearIRF_EXIT, irf.type = "incr.irf", scale = 2, center = TRUE, col = col,
      lwd = 2, lty.ci = 2,  xlab = xlab, ylab = "Impulse response",
      xaxt = 'n' , cex.lab=0.7, cex.axis=0.7)

axis(side=1,at=c(0,4,8,12,16), cex.axis=0.7)

title("Failing rate of firms", line= -2, outer = TRUE,  cex.main = 1)

#Profits of firms

irfVar <- "Pi2"
refVar    <- "GDPnom" 

mc <- read.3d.lsd( list.files.lsd( folder, paste0( baseName, "1" ) ),
                   c( readVars, irfVar, refVar ),
                   skip = iniDrop, nrows = nKeep )
mcShock <- read.3d.lsd( list.files.lsd( folder, paste0( baseName, "2" ) ),
                        c( shockVar, irfVar ),
                        skip = iniDrop, nrows = nKeep )

linearIRF_PROF <- irf.lsd( data = mc,      # non-shocked MC data
                           data.shock = mcShock,   # shocked data
                           t.horiz = irfHor,       # post-shock analysis time horizon
                           var.irf = irfVar,       # variable to compute IRF
                           var.shock = shockVar,   # shock variable (impulse)
                           var.ref = refVar,         # reference variable to IR measure
                           irf.type = "none",      # no plot now
                           stat = mcStat,          # type of statistic to use
                           ci.R = bootR,           # CI bootstrap repetitions (odd)
                           ci.type = bootCI,       # CI algorithm type
                           lim.outl = limOutl,     # outlier limit/threshold
                           alpha = bootAlpha )     # confidence interval conf. level

xlab <- "Relative time after shock"
col <- "red"
par(mfrow=c(1,1))

#plotting Robust linear impulse responses of GDP 
plot( linearIRF_PROF, irf.type = "incr.irf", scale = 2, center = TRUE, col = col,
      lwd = 2, lty.ci = 2,  xlab = xlab, ylab = "Impulse response",
      xaxt = 'n' , cex.lab=0.7, cex.axis=0.7)

axis(side=1,at=c(0,4,8,12,16), cex.axis=0.7)

title("Profits of firms", line= -2, outer = TRUE,  cex.main = 1)


#Financial fragility

irfVar <- "Bda"

mc <- read.3d.lsd( list.files.lsd( folder, paste0( baseName, "1" ) ),
                   c( readVars, irfVar, refVar ),
                   skip = iniDrop, nrows = nKeep )
mcShock <- read.3d.lsd( list.files.lsd( folder, paste0( baseName, "2" ) ),
                        c( shockVar, irfVar ),
                        skip = iniDrop, nrows = nKeep )

linearIRF_BDA <- irf.lsd( data = mc,      # non-shocked MC data
                           data.shock = mcShock,   # shocked data
                           t.horiz = irfHor,       # post-shock analysis time horizon
                           var.irf = irfVar,       # variable to compute IRF
                           var.shock = shockVar,   # shock variable (impulse)
                           var.ref = NULL,         # reference variable to IR measure
                           irf.type = "none",      # no plot now
                           stat = mcStat,          # type of statistic to use
                           ci.R = bootR,           # CI bootstrap repetitions (odd)
                           ci.type = bootCI,       # CI algorithm type
                           lim.outl = limOutl,     # outlier limit/threshold
                           alpha = bootAlpha )     # confidence interval conf. level

xlab <- "Relative time after shock"
col <- "red"
par(mfrow=c(1,1))

#plotting Robust linear impulse responses of GDP 
plot( linearIRF_BDA, irf.type = "incr.irf", scale = 2, center = TRUE, col = col,
      lwd = 2, lty.ci = 2,  xlab = xlab, ylab = "Impulse response",
      xaxt = 'n' , cex.lab=0.7, cex.axis=0.7)

axis(side=1,at=c(0,4,8,12,16), cex.axis=0.7)

title("Financial fragility", line= -2, outer = TRUE,  cex.main = 1)