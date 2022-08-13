\name{irf.lsd}

\alias{irf.lsd}

\title{
Impulse-response function analysis
}

\description{
This function performs a (linear) impulse-response function (IRF) analysis on the data produced by a Monte Carlo experiment, typically from (but not restricted to) a LSD simulation model.
}

\usage{
irf.lsd( data, data.shock, t.horiz, var.irf, var.shock, var.ref = NULL,
         irf.type = c( "incr.irf", "cum.irf", "peak.mult", "cum.mult", "none" ),
         stat = c( "mean", "median" ), ci.R = 999,
         ci.type = c( "basic", "perc", "bca" ),
         lim.outl = 0, alpha = 0.05, seed = 1, ... )
}

\arguments{
  \item{data}{numeric: a 3-dimensional array containing data from Monte Carlo (MC) simulation samples where the impulse (shock/treatment) was not applied/occurred. The array must have dimensions ordered as time steps x variables x MC samples. This format is automatically produced by \link[LSDinterface]{read.3d.lsd} but using it is not required. The second array dimension (variables) must be named with the names of the variables used in the analysis. The absolute minimum array dimensions are 2x1x2.
  }

  \item{data.shock}{numeric: a 3-dimensional array similar to \code{data} but containing data from MC samples where the impulse (shock/treatment) was applied/occurred. The array must have as dimensions: time steps x variables x MC samples. This can produced by \link[LSDinterface]{read.3d.lsd}, but this is not required. The first two dimensions must be similar between \code{data} and \code{data.shock}, containing the same time steps and variable names
  }

  \item{t.horiz}{integer: a positive value indicating the post-impulse time span to consider for analysis.
  }

  \item{var.irf}{string: the variable name on which perform the analysis. It must be one of the column names in \code{data}.
  }

  \item{var.shock}{string: the name of the variable containing the impulse/shock. It must be one of the column names in \code{data.shock}.
  }

  \item{var.ref}{string: the name of a reference variable to scale down (divide) the shock variable. The default is to do no scaling. If provided, it must be one of the column names in \code{data}. The special value \code{"\%"} represents the same variable as the shock one, but using the corresponding non-shock values, so effectively standardizing the shock values.
  }

  \item{irf.type}{string: one of five options (\code{"incr.irf"}, \code{"cum.irf"}, \code{"peak.mult"}, \code{"cum.mult"}, or \code{"none"}) defining the type of function plot to be produced: incremental impulse-response, cumulative impulse-response, peak multiplier, cumulative multiplier, or no plot. It also selects the type of function to be used when printing reports (response or multiplier functions). The default is \code{"incr.irf"}. This option only affect the plot, not the produced data, as all function values are always computed and saved in the output object.
  }

  \item{stat}{string: one of \code{"mean"} or \code{"median"}, representing the Monte Carlo statistic used to compare samples. It also selects the corresponding deviation measure, between the standard deviation (SD) or the median absolute deviation (MAD). The default is to use the mean and SD.
  }

  \item{ci.R}{integer: number of bootstrap replicates when computing the bootstrap confidence interval.
  }

  \item{ci.type}{string: the type of bootstrap confidence interval to compute, must be one of \code{"basic"} (the default), \code{"perc"} (percentile interval), or \code{"bca"} (BCa - adjusted percentile interval).
  }

  \item{lim.outl}{numeric: a positive outlier threshold limit multiple, applied over the distance between the first and the fourth quartiles of the Monte Carlo data. Outlier samples below or above the computed thresholds are removed from the analysis. The default (\code{0}), is to keep all samples (no outlier removal).
  }

  \item{alpha}{numeric: a value between 0 and 0.5, defining the desired statistical significance level to be adopted in the analysis. The default is 0.05 (5\%).
  }

  \item{seed}{integer: a value defining the initial state of the pseudo-random number generator.
  }

  \item{...}{additional parameters to configure printing and plotting.
  }
}

\details{
As a dynamic system, a simulation model may have its outputs analyzed when a brief input signal (an impulse or "shock") is applied to one of its inputs.

The function operates over \code{data} from multiple realizations of a Monte Carlo experiment.
}

\value{
It returns an object of class \code{irf.lsd}, which has \code{print}- and \code{plot}-specific methods for presenting the analysis results. This object contains several items:

  \item{irf}{numeric: vector of length \code{t.horiz + 1} containing the average or median (according to \code{stat}) incremental impulse response function data.
  }

  \item{cirf}{numeric: vector of length \code{t.horiz + 1} containing the average or median (according to \code{stat}) cumulative impulse response function data.
  }

  \item{pmf}{numeric: vector of length \code{t.horiz + 1} containing the average or median (according to \code{stat}) peak-multiplier function data. The peak impulse-multiplier function evaluates the analysis variable (\code{var.irf}) as a multiple of the impulse variable (\code{var.shock}) at the shock start time.
  }

  \item{cmf}{numeric: vector of length \code{t.horiz + 1} containing the average or median (according to \code{stat}) cumulative impulse-multiplier function data. The cumulative-multiplier function evaluates the accumulated analysis variable (\code{var.irf}), from impulse time, as a multiple of the accumulated impulse variable (\code{var.shock}) over the time horizon (\code{t.horiz}).
  }

  \item{irf.ci.lo}{numeric: vector of length \code{t.horiz + 1} containing the confidence interval lower limit of incremental impulse response function data.
  }

  \item{irf.ci.hi}{numeric: vector of length \code{t.horiz + 1} containing the confidence interval upper limit of incremental impulse response function data.
  }

  \item{cirf.ci.lo}{numeric: vector of length \code{t.horiz + 1} containing the confidence interval lower limit of cumulative impulse response function data.
  }

  \item{cirf.ci.hi}{numeric: vector of length \code{t.horiz + 1} containing the confidence interval upper limit of cumulative impulse response function data.
  }

  \item{pmf.ci.lo}{numeric: vector of length \code{t.horiz + 1} containing the confidence interval lower limit of peak multiplier function data.
  }

  \item{pmf.ci.hi}{numeric: vector of length \code{t.horiz + 1} containing the confidence interval upper limit of peak multiplier function data.
  }

  \item{cmf.ci.lo}{numeric: vector of length \code{t.horiz + 1} containing the confidence interval lower limit of cumulative multiplier function data.
  }

  \item{cmf.ci.hi}{numeric: vector of length \code{t.horiz + 1} containing the confidence interval upper limit of cumulative multiplier function data.
  }

  \item{irf.ylim}{numeric: vector of length two containing the absolute minimum and maximum values for the incremental impulse response function data.
  }

  \item{cirf.ylim}{numeric: vector of length two containing the absolute minimum and maximum values for the cumulative impulse response function data.
  }

  \item{pmf.ylim}{numeric: vector of length two containing the absolute minimum and maximum values for the peak multiplier function data.
  }

  \item{cmf.ylim}{numeric: vector of length two containing the absolute minimum and maximum values for the cumulative multiplier function data.
  }

  \item{ir}{numeric: data frame with \code{nsample} (see below) rows and \code{t.horiz + 1} columns. The data frame is the set of individual incremental impulse responses from each Monte Carlo (MC) sample.
  }

  \item{cir}{numeric: data frame with \code{nsample} (see below) rows and \code{t.horiz + 1} columns. The data frame is the set of individual cumulative impulse responses from each MC sample.
  }

  \item{pm}{numeric: data frame with \code{nsample} (see below) rows and \code{t.horiz + 1} columns. The data frame is the set of individual peak multipliers from each MC sample.
  }

  \item{cm}{numeric: data frame with \code{nsample} (see below) rows and \code{t.horiz + 1} columns. The data frame is the set of individual cumulative multipliers from each MC sample.
  }

  \item{t.shock}{numeric: vector of length equal to \code{nsample} (see below), containing the time the shock was detected in each MC sample.
  }

  \item{t.horiz}{integer: the time horizon used in the analysis (same as the \code{t.horiz} argument).
  }

  \item{var.irf}{character: the name of the variable used in the impulse-response analysis (same as the \code{var.irf} argument).
  }

  \item{var.shock}{character: the name of the shock variable used in the analysis (same as the \code{var.shock} argument).
  }

  \item{var.ref}{character: the name of the scale-reference variable used in the analysis (same as the \code{var.ref} argument).
  }

  \item{stat}{character: the Monte Carlo statistic used in the analysis (same as the \code{stat} argument).
  }

  \item{alpha}{numeric: the statistical significance level used in the analysis (same as the \code{alpha} argument).
  }

  \item{nsample}{integer: the effective number of of Monte Carlo (MC) samples effectively used for deriving the response function, after the removal of outliers if \code{lim.outl > 0}.
  }

  \item{outliers}{integer: vector containing the number of each MC sample considered an outlier, and so removed from the analysis, or an empty vector if no outlier was excluded. The MC numbers are the indexes to the third dimension of \code{data}.
  }

  \item{data.crc}{character: an hexadecimal sting containing the 32-bit Cyclic Redundancy Check (CRC32) for the \code{data} used in the analysis.
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
See the note in \link[LSDirf]{LSDirf-package} for an methodological overview and for instructions on how to perform the (linear) impulse-response function analysis.
}

\seealso{
\code{\link{state.irf.lsd}},
\code{\link[LSDinterface]{read.3d.lsd}},
\code{\link[LSDinterface]{read.4d.lsd}},
}

\examples{
# get the example directory name
path <- system.file( "extdata/lirf", package = "LSDirf" )

}

\keyword{methods}
\keyword{models}
\keyword{design}

