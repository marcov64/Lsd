\name{info.distance.lsd}
\alias{info.distance.lsd}
\title{
Compute distance measure between LSD Monte Carlo time series and a set of references
}
\description{
This function reads a 3 or 4-dimensional array produced by \code{\link{read.3d.lsd}} or \code{\link{read.4d.lsd}} and computes several types of distance measures between the time series from a set of Monte Carlo runs and a set of reference time series (like the Monte Carlo average or median).
}
\usage{
info.distance.lsd( array, references, instance = 1, distance = "euclidean",
                   std.dist = FALSE, std.val = FALSE, rank = FALSE, weights = 1, ... )
}
\arguments{
  \item{array}{
a 3D or 4D array as produced by \code{\link{read.3d.lsd}} and \code{\link{read.4d.lsd}}, where in the first dimension (rows) you have the time steps, in the second (columns), the variables and in the third/fourth dimension, the Monte Carlo experiments, and the instances in the third dimension (4D arrays only). When 4D arrays are provided, only first instances are used in the computation.
}
  \item{references}{
a 2D matrix containing the reference time series, time in rows and variable values in named columns, from which the distance measures are to be computed. Columns must be named for the exact match to the names of the desired variables (contained in \code{array}). Only variables contained in both \code{array} and \code{references} are considered in the computation. According to the \code{distance} measure chosen, the number of time steps in \code{array} and \code{references} must be the same (as in the default Euclidean distance).
}
  \item{instance}{
integer: the instance of the variable to be read, for variables that exist in more than one object (4D \code{array} only). The default (1) is to read first instances.
}
  \item{distance}{
string: the distance measure to be used. The default is to compute the Euclidean distance (\code{"euclidean"}). For a comprehensive list of measure options, please refer to \code{\link{TSDistances}}. Measure names can be abbreviated.
}
  \item{std.dist}{
a logical value indicating, if \code{TRUE}, that the computed distances must be standardized with respect of the number of time steps involved. The default, \code{FALSE}, is not standardizing distances. This is relevant for properly comparing the metrics of series containing \code{NA}s.
}
  \item{std.val}{
a logical value indicating, if \code{TRUE}, that the series values must be standardized before computing the distances. The default, \code{FALSE}, is not standardizing values. This is relevant for properly comparing the metrics of series for different variables which are not distributed over the same range of values.
}
  \item{rank}{
a logical value indicating, if \code{TRUE}, that the Monte Carlo runs must be ranked in terms of closeness to the \code{references}. The default is not computing the run ranking, as this may be computationally expensive for some \code{distance} measures.
}
  \item{weights}{
a numerical vector containing the weights to be used for each variable in \code{reference} when \code{rank = TRUE}. The order of the weights must be the same as the one used for the variables (columns) in the \code{references} matrix. If the length of \code{weigths} is smaller the number of variables, the vector is recycled. The default is to use the same weight for all variables.
}
  \item{...}{
additional parameters required by the specific method (see \code{\link{TSDistances}}).
}
}
\details{
This function is a front-end to the extensive \link[=TSdist-package]{TSdist package} for interfacing it with LSD generated data. Please check the associated documentation for further information.

\link[=TSdist-package]{TSdist package} provides many different distance measure alternatives, including many that allow for different number of time steps among runs and references.

This function may also search the Monte Carlo run which has the overall smallest (standardized) distances from the given \code{references}. Irrespective of the options \code{std.dist} and \code{std.val}, the search uses always standardized values and distances for computation (this does not affect the distance measure matrix values).

One typical application of distance metrics is to select runs which are closer to the Monte Carlo average or median, that is, the runs which are more representative of the Monte Carlo Experiment. As there is no single criteria to define such "closeness", multiple distance measures may help to identify the set of most interesting runs.
}
\value{
Returns a list containing:
\item{dist}{a named matrix containing the distances for each Monte Carlo run (lines) and variables (columns) (contained both in \code{array} and \code{references})}
\item{rank}{(only if \code{rank = TRUE}) a named vector of weighted Monte Carlo run standardized distances, sorted in increasing distance order (closest run first)}
}
\note{
When comparing distance measures between different Monte Carlo runs and variables, it is important to standardize the distances and values to ensure consistency. For variables which may present \code{NA} values, setting \code{std.dist = TRUE} ensures distance comparability by dividing the absolute distance of each run-reference pair by the number of effective (non-\code{NA}) time steps. When comparing variables which are dimensionally heterogeneous, \code{std.val = TRUE} uses the relative measure (between 1 and the run value divided by the corresponding reference value) to compute the distances.

When setting \code{std.val = TRUE}, all points in which the \code{references}' values are equal to zero are effectively removed from calculations. This behavior is always applied when searching for the closest Monte Carlo run.
}
\author{Marcelo C. Pereira}
\seealso{
\code{\link{read.3d.lsd}()},
\code{\link{read.4d.lsd}()},
\code{\link{info.stats.lsd}()}
}
\examples{
# get the list of file names of example LSD results
files <- list.files.lsd( system.file( "extdata", package = "LSDinterface" ) )

# read first instance of all variables from MC files (3D array)
inst1Array <- read.3d.lsd( files )

# create statistics data frames for the variables
inst1Stats <- info.stats.lsd( inst1Array )

# compute the Euclidean distance to the mean for all variables and runs
info.distance.lsd( inst1Array, inst1Stats$avg )$dist

# the same exercise but for a 4D array and Manhattan distance to the median
# plus indicating the Monte Carlo run closest to the median
allArray <- read.4d.lsd( files )
allStats <- info.stats.lsd( allArray, median = TRUE )
allDist <- info.distance.lsd( allArray, allStats$med, distance = "manhattan",
                              rank = TRUE )
allDist$dist
allDist$rank
names( allDist$rank )[ 1 ]  # results file name of the closest run
}
\keyword{ statistics }
\keyword{ file }
