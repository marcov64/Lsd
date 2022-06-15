\name{state.irf.lsd}

\alias{state.irf.lsd}

\title{
State-dependent impulse-response function analysis
}

\description{
This function performs a state-dependent impulse-response function (IRF) analysis on the data produced by a Monte Carlo experiment, typically from (but not restricted to) a LSD simulation model.
}

\usage{
state.irf.lsd( data, irf, states = NULL, state.num = 1,
               state.vars = NULL, eval.state = NULL,
               metr.irf = NULL, add.vars = NULL,
               irf.type = c( "cumulative", "incremental", "none" ),
               ci.R = 999, ci.type = c( "basic", "perc", "bca" ),
               alpha = 0.05, seed = 1, ... )
}

\arguments{
  \item{data}{numeric: a 3-dimensional array containing data from Monte Carlo (MC) simulation samples where the impulse (shock/treatment) was not applied/occurred. The array must have dimensions ordered as time steps x variables x MC samples. This format is automatically produced by \link[LSDinterface]{read.3d.lsd} but using it is not required. The second array dimension (variables) must be named with the names of the variables used in the analysis. The absolute minimum array dimensions are 2x1x2.
  }

  \item{irf}{object: an object produced by a previous run of \code{\link{irf.lsd}} over the same dataset (as defined by \code{data}).
  }

  \item{states}{object: an optional object produced by a previous run of \code{\link{state.ident.lsd}} containing a set of identified state candidates. Only one state candidate, as defined by \code{state.num} is used in each run.
  }

  \item{state.num}{integer: the index (1,2,...) of the state candidate in \code{states} to use. The default is to use the first state, which is usually the most likely one.
  }

  \item{state.vars}{character: a vector of variable names to use as state variables. If more than one name is provided, a proper \code{eval.state} state evaluation function supporting multi-variable states must be also provided.
  }

  \item{eval.state}{function: a function able to define the corresponding state of each run of a Monte Carlo experiment. The function must take a matrix as argument, organized as runs on rows and the state variable(s) on columns. It must return an integer vector of length equal to the number of runs, defining the state of each run. States are defined by a sequence of integer values, e.g., 0, 1 or 1, 2 ,3, The minimum number of states is two and there is no maximum. If no function is supplied (\code{NULL}), the default, an internal 2-state evaluation function is used, using the mean or the median, according to \code{stat}, to split the MC set into two states.
  }

  \item{metr.irf}{function: a function that assigns a metric to compare each run of a Monte Carlo experiment, to be used on regressions. The function must take a cumulative impulse-response matrix, organized as runs on rows and response times (0, 1, ...,\code{t.horiz}) on columns. It must return a numeric vector of length equal to the number of runs, defining the metric associated with each run. Higher metric values correspond to increased impulse effect. If no function is supplied (\code{NULL}), the default, the sum of state variable value(s) at impulse time is used as metric.
  }

  \item{add.vars}{function: a optional function to add new variables to the MC dataset, before the analysis is performed. The function must take a single Monte Carlo run data frame, organized as time on rows and (original) variables on columns. It must return this data frame with new column(s) added, one per each new variable.
  }

  \item{irf.type}{string: one of three options (\code{"incremental"}, \code{"cumulative"}, or \code{"none"}) defining the type of impulse-response function plot to be produced. The default is \code{"incremental"} response. This option only affect the plot, not the produced data.
  }

  \item{ci.R}{integer: number of bootstrap replicates when computing the bootstrap confidence interval.
  }

  \item{ci.type}{string: the type of bootstrap confidence interval to compute, must be one of \code{"basic"} (the default), \code{"perc"} (percentile interval), or \code{"bca"} (BCa - adjusted percentile interval).
  }

  \item{alpha}{numeric: a value between 0 and 0.5, defining the desired statistical significance level to be adopted in the analysis. The default is 0.05 (5\%).
  }

  \item{seed}{integer: a value defining the initial state of the pseudo-random number generator.
  }

  \item{...}{additional parameters to configure printing and plotting.
  }
}

\details{
As a dynamic system, a simulation model may have its outputs analyzed when a brief input signal (an impulse or "shock") is applied to one of its inputs. In particular, the effect of the shock may be correlated to some system-specific state, in which it may be amplified or attenuated. This function allows for the investigation of such differentiated effects, given an objective criterion to split the system status (i.e., the model outputs) in two or more states.

The function operates over \code{data} from multiple realizations of a Monte Carlo experiment, and a previous (linear) impulse-response function analysis (\code{irf}) performed by \code{\link{irf.lsd}}.
}

\value{
It returns an object of class \code{state.irf.lsd}, which has \code{print}- and \code{plot}-specific methods for presenting the analysis results. This object contains several items:

  \item{irf.state}{list: each list element is a vector of length \code{t.horiz + 1} containing the average or median (according to \code{stat}) incremental impulse response function data for each identified state.
  }

  \item{cirf.state}{list: each list element is a vector of length \code{t.horiz + 1} containing the average or median (according to \code{stat}) cumulative impulse response function data for each identified state.
  }

  \item{irf.state.ci.lo}{list: each list element is a vector of length \code{t.horiz + 1} containing the confidence interval lower limit of incremental impulse response function data for each identified state.
  }

  \item{irf.state.ci.hi}{list: each list element is a vector of length \code{t.horiz + 1} containing the confidence interval upper limit of incremental impulse response function data for each identified state.
  }

  \item{cirf.state.ci.lo}{list: each list element is a vector of length \code{t.horiz + 1} containing the confidence interval lower limit of cumulative impulse response function data for each identified state.
  }

  \item{cirf.state.ci.hi}{list: each list element is a vector of length \code{t.horiz + 1} containing the confidence interval upper limit of cumulative impulse response function data for each identified state.
  }

  \item{irf.state.ylim}{list: each list element is a vector of length two containing the absolute minimum and maximum values for the incremental impulse response function data for each identified state.
  }

  \item{cirf.state.ylim}{list: each list element is a vector of length two containing the absolute minimum and maximum values for the cumulative impulse response function data for each identified state.
  }

  \item{irf.test}{object: the result of the test comparing the statistical significance of the incremental impulse-response function difference among different states. Two-state setups are evaluated with t or U tests, according to \code{stat} in \code{\link{irf.lsd}}, or H or F tests otherwise.
  }

  \item{cirf.test}{object: the result of the test comparing the statistical significance of the cumulative impulse-response function difference among different states, considering the entire period of analysis (1, ..., \code{t.horiz}). Two-state setups are evaluated with t or U tests, according to \code{stat} in \code{\link{irf.lsd}}, or H or F tests otherwise.
  }

  \item{cirf.test.t.horiz}{object: the result of the test comparing the statistical significance of the cumulative impulse-response function difference among different states just at the end of the analysis time horizon (\code{t.horiz}). Two-state setups are evaluated with t or U tests, according to \code{stat} in \code{\link{irf.lsd}}, or H or F tests otherwise.
  }

  \item{state}{character: a textual description of the tested state.
  }

  \item{state.vars}{character: a vector of variable names effectively available as state variables.
  }

  \item{t.horiz}{integer: the time horizon used in the analysis (same as the \code{t.horiz} argument in \code{\link{irf.lsd}}).
  }

  \item{var.irf}{character: the name of the variable used in the impulse-response analysis (same as the \code{var.irf} argument in \code{\link{irf.lsd}}).
  }

  \item{var.ref}{character: the name of the scale-reference variable used in the analysis (same as the \code{var.ref} argument in \code{\link{irf.lsd}}).
  }

  \item{stat}{character: the Monte Carlo statistic used in the analysis (same as the \code{stat} argument in \code{\link{irf.lsd}}).
  }

  \item{alpha}{numeric: the statistical significance level used in the analysis (same as the \code{alpha} argument in \code{\link{irf.lsd}}).
  }

  \item{nsample}{integer: the effective number of of Monte Carlo (MC) samples effectively used for deriving the response function, after the removal of outliers if \code{lim.outl > 0} in \code{\link{irf.lsd}}.
  }

  \item{outliers}{integer: vector containing the number of each MC sample considered an outlier, and so removed from the analysis in \code{\link{irf.lsd}}, or an empty vector if no outlier was excluded. The MC numbers are the indexes to the third dimension of \code{data}.
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
# get the example directory name
path <- system.file( "extdata/sirf", package = "LSDirf" )

}

\keyword{methods}
\keyword{models}
\keyword{design}

