\name{info.stats.lsd}
\alias{info.stats.lsd}
\title{
Compute Monte Carlo statistics from a set of LSD runs
}
\description{
This function reads a 3 or 4-dimensional array produced by \code{\link{read.3d.lsd}} or \code{\link{read.4d.lsd}} and produces a list with 2D data frames containing the (Monte Carlo) mean, the standard deviation, the maximum, the minimum, and other optional statistics for each variable, at each time step.
}
\usage{
info.stats.lsd( array, rows = 1, cols = 2, median = FALSE,
                ci = c( "none", "mean", "median", "auto" ),
                ci.conf = 0.95, ci.boot = NULL, boot.R = 999,
                na.rm = TRUE, inf.rm = TRUE )
}
\arguments{
  \item{array}{
an 3D or 4D array as produced by \code{\link{read.3d.lsd}} and \code{\link{read.4d.lsd}}, where in the first dimension (rows) you have the time steps, in the second (columns), the variables and in the third/fourth dimension, the Monte Carlo experiments, and the instances in the third dimension (4D arrays only).
}
  \item{rows}{
an integer array dimension to be used as the rows for the statistics matrices, default is to use first array dimension.
}
  \item{cols}{
an integer array dimension to be used as the columns for the statistics matrices, default is to use second array dimension.
}
  \item{median}{
a logical value indicating if (\code{TRUE}) the median should also be computed.
}
  \item{ci}{
a character string specifying the type of confidence interval to compute, must be one of \code{"none"} (default) for no confidence interval computation, \code{"mean"}, to compute a confidence interval for the mean, \code{"median"}, for the median, or \code{"auto"}, to use the option set for the \code{median} argument (above). This option can be abbreviated.
}
  \item{ci.conf}{
confidence level of the confidence interval
}
  \item{ci.boot}{
a character string specifying the type of bootstrap confidence interval to compute, must be one of \code{"basic"}, \code{"perc"} (percentile interval), or \code{"bca"} (BCa - adjusted percentile interval). If set to \code{NULL} a regular asymptotic confidence interval is produced (no bootstrap), assuming normal distribution for the mean or using a non-parametric rank test for the median. Non-bootstrap percentiles are much faster to compute but generally less accurate.
}
  \item{boot.R}{
number of bootstrap replicates.
}
  \item{na.rm }{
a logical value indicating whether \code{NA} values should be stripped before the computation proceeds.
}
  \item{inf.rm}{
a logical value indicating whether non-finite values should be stripped before the computation proceeds.
}
}
\value{
Returns a list containing four to seven matrices, with the original size and naming of the selected 2 dimensions of the argument.
\item{avg}{a matrix with the mean of the MC experiments}
\item{sd}{a matrix with the standard deviation of the MC experiments}
\item{max}{a matrix with the maximum value of the MC experiments}
\item{min}{a matrix with the minimum value of the MC experiments}
\item{med}{a matrix with the median of the MC experiments (only present if argument \code{median} = \code{TRUE})}
\item{ci.hi}{a matrix with the maximum value of the MC experiments (only present if argument \code{ci} is not set to  \code{"none"})}
\item{ci.lo}{a matrix with the minimum value of the MC experiments (only present if argument \code{ci} is not set to \code{"none"})}
}
\author{Marcelo C. Pereira}
\seealso{
\code{\link{list.files.lsd}()}
\code{\link{read.3d.lsd}()},
\code{\link{read.4d.lsd}()},
\code{\link{info.dimensions.lsd}()}
}
\examples{
# get the list of file names of example LSD results
files <- list.files.lsd( system.file( "extdata", package = "LSDinterface" ) )

# read first instance of all variables from MC files (3D array)
inst1Array <- read.3d.lsd( files )

# create statistics data frames for the variables
inst1Stats <- info.stats.lsd( inst1Array )
print( inst1Stats$avg[ 10 : 20, ] )
print( inst1Stats$sd[ 10 : 20, ] )

# organize the stats, including medians, by variable (dim=2) and file (dim=3)
inst1Stats2 <- info.stats.lsd( inst1Array, rows = 2, cols = 3, median = TRUE )
print( inst1Stats2$med[ , 1 : 2 ] )

# the same but for all instance of all variables (from a 4D array)
# and a normal (non-boostrap) confidence intervals for the means
allArray <- read.4d.lsd( files )
allStats <- info.stats.lsd( allArray, ci = "auto" )
print( allStats$ci.lo[ 3, 1 : 7 ] )
print( allStats$avg[ 3, 1 : 7 ] )
print( allStats$ci.hi[ 3, 1 : 7 ] )

# organize the stats by file (dim=4) and variable (dim=2)
# plus boostrat confidence intervals for the median
\donttest{allStats2 <- info.stats.lsd( allArray, rows = 4, cols = 2, median = TRUE,
                             ci = "auto", ci.boot = "bca" )
print( allStats2$ci.lo[ , 1 : 3 ] )
print( allStats2$med[ , 1 : 3 ] )
print( allStats2$ci.hi[ , 1 : 3 ] )
}
}
\keyword{ statistics }
\keyword{ file }
