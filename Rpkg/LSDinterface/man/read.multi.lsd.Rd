\name{read.multi.lsd}
\alias{read.multi.lsd}
\title{
Read all instances of LSD variables (time series) from a LSD results file
}
\description{
This function reads the data series associated to all instances of each selected variable from a LSD results file (.res).
}
\usage{
read.multi.lsd( file, col.names = NULL, nrows = -1, skip = 0, check.names = TRUE )
}
\arguments{
  \item{file}{
the name of the LSD results file which the data are to be read from. If it does not contain an absolute path, the file name is relative to the current working directory, \code{\link[base]{getwd}()}. This can be a compressed file (see file) and must include the appropriated extension (usually \code{.res} or \code{.res.gz}).
}
  \item{col.names}{
a vector of optional names for the variables. The default is to read all variables.
}
  \item{nrows}{
integer: the maximum number of time steps (rows) to read in. Negative and other invalid values are ignored. The default is to read all rows.
}
  \item{skip}{
integer: the number of time steps (rows) of the results file to skip before beginning to read data. The default is to read from the first time step (t = 1).
}
  \item{check.names}{
logical. If TRUE then the names of the variables are checked to ensure that they are syntactically valid variable names. If necessary they are adjusted (by make.names) so that they are, and also to ensure that there are no duplicates.
}
}
\value{
Returns a list of matrices, each containing one of the selected variables' time series from the results file.
}
\author{Marcelo C. Pereira}
\note{
For extracting data from multiple similar files (like sensitivity analysis results), see \code{\link{read.list.lsd}}.
}
\seealso{
\code{\link{list.files.lsd}()}
\code{\link{read.single.lsd}()},
\code{\link{read.list.lsd}()},
\code{\link{read.3d.lsd}()},
\code{\link{read.4d.lsd}()},
\code{\link{read.raw.lsd}()}
}
\examples{
# get the list of file names of example LSD results
files <- list.files.lsd( system.file( "extdata", package = "LSDinterface" ) )

# load first .res file into a simple matrix (all instances),
macroList <- read.multi.lsd( files[ 1 ] )
length( macroList )
print( macroList[[ 1 ]][ 1 : 5, ] )
print( macroList[[ 8 ]][ 1 : 5, ] )

# read first instance of 2 variables, skipping the initial 20 time steps
# and keeping up to 30 time steps (from t = 21 up to t = 30)
varsList21_30 <- read.multi.lsd( files[ 2 ], c( "_A1p", "_growth1" ),
                                 skip = 20, nrows = 30 )
print( varsList21_30[[ 1 ]][ , 1 : 3 ] )
print( varsList21_30[[ 2 ]][ , 1 : 3 ] )
}
\keyword{ interface }
\keyword{ file }
\keyword{ datasets }
\keyword{ database }
