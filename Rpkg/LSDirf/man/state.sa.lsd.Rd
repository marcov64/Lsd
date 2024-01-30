\name{state.sa.lsd}

\alias{state.sa.lsd}

\title{
Sensitivity analysis of IRF to state variables
}

\description{
This function performs a sensitivity analysis of the impulse-response function (IRF) to selected state variables of data from a Monte Carlo experiment, typically from (but not restricted to) a LSD simulation model.
}

\usage{
state.sa.lsd( data, irf, state.vars = NULL, metr.irf = NULL,
              add.vars = NULL, ntree = 500, nodesize = 5,
              mtry = max( floor( ifelse( ! is.null( state.vars ),
                                         length( state.vars ),
                                         dim( data )[ 2 ] ) / 3 ),
                          1 ),
              no.plot = FALSE, alpha = 0.05, seed = 1, ... )
}

\arguments{
  \item{data}{numeric: a 3-dimensional array containing data from Monte Carlo (MC) simulation samples where the impulse (shock/treatment) was not applied/occurred. The array must have dimensions ordered as time steps x variables x MC samples. This format is automatically produced by \link[LSDinterface]{read.3d.lsd} but using it is not required. The second array dimension (variables) must be named with the names of the variables used in the analysis. The absolute minimum array dimensions are 2x1x2.
  }

  \item{irf}{object: an object produced by a previous run of \code{\link{irf.lsd}} over the same dataset (as defined by \code{data}).
  }

  \item{state.vars}{character: a vector of variable names to consider as state variables.
  }

  \item{metr.irf}{function: a function that assigns a metric to compare each run of a Monte Carlo experiment, to be used on regressions. The function must take a cumulative impulse-response matrix, organized as runs on rows and response times (0, 1, ...,\code{t.horiz}) on columns. It must return a numeric vector of length equal to the number of runs, defining the metric associated with each run. Higher metric values correspond to increased impulse effect. If no function is supplied (\code{NULL}), the default, the sum of state variable value(s) at impulse time is used as metric.
  }

  \item{add.vars}{function: an optional function to add new variables to the MC dataset, before the analysis is performed. The function must take a single Monte Carlo run data frame, organized as time on rows and (original) variables on columns. It must return this data frame with new column(s) added, one per each new variable.
  }

  \item{ntree}{integer: number of trees to grow. This number should not be set to too small values, to ensure that every possible state gets predicted at least a few times.
  }

  \item{nodesize}{integer: minimum number of associated data observations to a node be considered in the analysis.
  }

  \item{mtry}{integer: number of state variables randomly sampled as candidates at each node for the random forest algorithm. The default is to use one third of the number of considered state variables.
  }

  \item{no.plot}{logical: if \code{TRUE}, the default, a bar plot is presented with the results. If set to \code{FALSE}, the bar plot is not shown.
  }

  \item{alpha}{numeric: a value between 0 and 0.5, defining the desired statistical significance level to be adopted in the analysis. The default is 0.05 (5\%).
  }

  \item{seed}{integer: a value defining the initial state of the pseudo-random number generator.
  }

  \item{...}{additional parameters to configure printing and plotting.
  }
}

\details{
As a dynamic system, a simulation model may have its outputs analyzed when a brief input signal (an impulse or "shock") is applied to one of its inputs. In particular, the effect of the shock may be correlated to some system-specific state, in which it may be amplified or attenuated, associated to specific model variables. This function evaluates how sensitive such states are to each of the specified variables.

The function operates over \code{data} from multiple realizations of a Monte Carlo experiment, and a previous (linear) impulse-response function analysis (\code{irf}) performed by \code{\link{irf.lsd}}.
}

\value{
It returns an object of class \code{state.sa.lsd}, which has \code{print}- and \code{plot}-specific methods for presenting the analysis results. This object contains several items:

  \item{importance}{data frame: contains the state variable importance measure (mean decrease in accuracy) produced by the random forest regression, one row for each state variable. First column presents the importance measure, second column brings the measure standard error, and third, the p-value of t test comparing the measure to zero.
  }

  \item{state.vars}{character: a vector of variable names effectively available as state variables.
  }

  \item{t.horiz}{integer: the time horizon used in the analysis (same as the \code{t.horiz} argument in \code{\link{irf.lsd}}).
  }

  \item{var.irf}{character: the name of the variable used in the impulse-response analysis (same as the \code{var.irf} argument in \code{\link{irf.lsd}}).
  }

  \item{var.ref}{var.ref}{character: the name of the scale-reference variable used in the analysis (same as the \code{var.ref} argument in \code{\link{irf.lsd}}).
  }

  \item{stat}{character: the Monte Carlo statistic used in the analysis (same as the \code{stat} argument in \code{\link{irf.lsd}}).
  }

  \item{alpha}{numeric: the statistical significance level used in the analysis (same as the \code{alpha} argument).
  }

  \item{nsample}{integer: the effective number of of Monte Carlo (MC) samples effectively used for deriving the response function, after the removal of outliers if \code{lim.outl > 0} in \code{\link{irf.lsd}}.
  }

  \item{outliers}{integer: vector containing the number of each MC sample considered an outlier, and so removed from the analysis in \code{\link{irf.lsd}}, or an empty vector if no outlier was excluded. The MC numbers are the indexes to the third dimension of \code{data}.
  }

  \item{ntree}{integer: number of trees grown (same as \code{ntree} argument).
  }

  \item{nodesize}{integer: minimum number of data observations in a node considered  (same as \code{nodesize} argument).
  }

  \item{mtry}{integer: number of state variables sampled per node (same as \code{mtry} argument).
  }

  \item{rsq}{numeric: the “pseudo R-squared” (1 - MSE / Var(y)) of the random forest regression.
  }

  \item{call}{character: the command line used to call the function.
  }
}

%\references{
%% ~put references to the literature/web site here ~
%}

\author{
\packageAuthor{LSDirf}
}

\note{
See the note in \link[LSDirf]{LSDirf-package} for an methodological overview and for instructions on how to perform the state-dependent impulse-response function analysis.
}

\seealso{
\code{\link{irf.lsd}},
\code{\link[LSDinterface]{read.3d.lsd}},
\code{\link[LSDinterface]{read.4d.lsd}},
}

\examples{
# Example data generation: Y is an AR(1) process that may receive a shock at
# t=50, S is the shock (0/1), a combination of 3 AR(1) processes (X1-X3)
# X4 is another AR(1) process, uncorrelated with S, X4sq is just X4^2
# All AR(1) processes have the same phi=0.98 coefficient, and are Monte
# Carlo sampled 500 times
set.seed( 1 )   # make results reproducible
# LSD-like arrays to store simulated time series (t x var x MC)
dataNoShock <- dataShock <-array ( 0, dim = c( 60, 7, 500 ) )
colnames( dataNoShock ) <- colnames( dataShock ) <-
  c( "Y", "S", "X1", "X2", "X3", "X4", "X4sq" )
# Monte Carlo sampling
for( n in 1 : 500 ) {
  # simulation time
  for( t in 2 : 60 ) {
    # AR process on X vars
    for( v in c( "X1", "X2", "X3", "X4" ) ) {
      dataNoShock[ t, v, n ] = dataShock[ t, v, n ] =
        0.98 * dataShock[ t - 1, v, n ] + rnorm( 1, 0, 0.1 )
    }
    # apply shock once
    if( t == 50 ) {
      dataShock[ t, "S", n ] <- 1
      shockEff <- 0.4 + 0.7 * isTRUE( dataShock[ t, "X1", n ] > 0.1 ) -
        0.4 * isTRUE( dataShock[ t, "X2", n ] > 0.1 ) +
        0.2 * isTRUE( dataShock[ t, "X3", n ] > 0.05 ) + rnorm( 1, 0, 0.2 )
    } else
      shockEff <- 0
    # AR process on Y var
    rs <- rnorm( 1, 0, 0.1 )
    dataNoShock[ t, "Y", n ] = 0.98 * dataNoShock[ t - 1, "Y", n ] + rs
    dataShock[ t, "Y", n ] = 0.98 * dataShock[ t - 1, "Y", n ] + shockEff + rs
  }
}
# another uncorrelated var
dataNoShock[ , "X4sq", ] <- dataShock[ , "X4sq", ] <- dataShock[ , "X4", ] ^ 2

# linear IRF analysis
linearIRF <- irf.lsd( data = dataNoShock,       # non-shocked MC data
                      data.shock = dataShock,   # shocked data
                      t.horiz = 10,             # post-shock analysis t horizon
                      var.irf = "Y",            # variable to compute IRF
                      var.shock = "S",          # shock variable (impulse)
                      irf.type = "none" )       # no plot of linear IRF

# state-variable sensitivity
stateSens <- state.sa.lsd( data = dataNoShock,  # non-shocked MC data
                           irf = linearIRF,     # linear IRF produced by irf.lsd
                           state.vars = c( "X1", "X2", "X3", "X4", "X4sq" ),
                                                # state variables to consider
                           mtry = 3 )           # number of samples per node

print( stateSens )                              # show sensitivity data
}

\keyword{methods}
\keyword{models}
\keyword{design}

