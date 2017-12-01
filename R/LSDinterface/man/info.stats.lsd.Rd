\name{info.stats.lsd}
\alias{info.stats.lsd}
\title{
Compute Monce Carlo statistics from a set of LSD runs
}
\description{
This function reads a 3 or 4-dimensional array produced by \code{\link{read.3d.lsd}} or \code{\link{read.4d.lsd}} and produces a list with 2D data frames containing the average, the standard deviation, the maximum and the mininum for each variable, at each time step.
}
\usage{
info.stats.lsd(array, rows = 1, cols = 2)
}
\arguments{
  \item{array }{
an 3D or 4D array as produced by \code{\link{read.3d.lsd}} and \code{\link{read.4d.lsd}}, where in the first dimension (rows) you have the time steps, in the second (columns), the variables and in the third/fourth dimension, the Monte Carlo experiments, and the instances in the third dimension (4D arrays only).
}
  \item{rows }{
integer: array dimension to be used as the rows for the statistics matrices, default is to use first array dimension.
}
  \item{cols }{
integer: array dimension to be used as the columns for the statistics matrices, default is to use second array dimension.
}
}
\value{
Returns a list containing four matrices, with the original size and naming of the selected 2 dimensions of the argument.
\item{avg }{a matrix with the average of the MC experiments}
\item{sd }{a matrix with the standard deviation of the MC experiments}
\item{max }{a matrix with the maximum value of the MC experiments}
\item{min }{a matrix with the minimum value of the MC experiments}
}
\author{Marcelo C. Pereira}
\seealso{
\code{\link{read.3d.lsd}},
\code{\link{read.4d.lsd}},
\code{\link{info.dimensions.lsd}}
}
\examples{
# Save current work directory and change to the examples directory
currWD <- getwd( )
setwd( system.file( "extdata", package = "LSDinterface" ) )

# reads first instance of all variables from three MC files (3D array)
inst1Array <- read.3d.lsd( c( "Sim1_1.res", "Sim1_2.res", "Sim1_3.res" ) )

# creates statistics data frames for the variables
inst1Stats <- info.stats.lsd( inst1Array )

# See matrix in the data viewer (require package 'utils')
\donttest{View( inst1Stats )}

# organize the stats by variable (dim=2) and file (dim=3)
inst1Stats2 <- info.stats.lsd( inst1Array, rows = 2, cols = 3 )
\donttest{View( inst1Stats2 )}

# the same but for all instance of all variables (from a 4D array)
allArray <- read.4d.lsd( c( "Sim1_1.res", "Sim1_2.res", "Sim1_3.res" ) )
allStats <- info.stats.lsd( allArray )
\donttest{View( allStats )}

# organize the stats by file (dim=4) and variable (dim=2)
allStats2 <- info.stats.lsd( allArray, rows = 4, cols = 2 )
\donttest{View( allStats2 )}

# Restore working directory
setwd( currWD )
}
\keyword{ attribute }
\keyword{ file }
