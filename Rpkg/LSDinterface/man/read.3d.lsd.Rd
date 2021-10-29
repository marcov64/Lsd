\name{read.3d.lsd}
\alias{read.3d.lsd}
\title{
Read one instance of LSD variables (time series) from multiple LSD results files into a 3D array
}
\description{
This function reads the data series associated to a specific instance of each selected variable from a set of LSD results files (.res) and saves them into a 3-dimensional array (time step x variable x file).
}
\usage{
read.3d.lsd( files, col.names = NULL, nrows = -1, skip = 0,
             check.names = TRUE, instance = 1, nnodes = 1 )
}
\arguments{
  \item{files}{
a character vector containing the names of the LSD results files which the data are to be read from. If they do not contain an absolute path, the file names are relative to the current working directory, \code{\link[base]{getwd}()}. These can be compressed files and must include the appropriated extension (usually \code{.res} or \code{.res.gz}).
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
  \item{instance}{
integer: the instance of the variable to be read, for variables that exist in more than one object. This number is based on the position (column) of the variable in the results file. The default (1) is to read first instances.
}
  \item{nnodes}{
integer: the maximum number of parallel computing nodes (parallel threads) in the current computer to be used for reading the files. The default, \code{nnodes = 1}, means single thread processing (no parallel threads). If equal to zero, creates up to one node per CPU core. Only \code{PSOCK} clusters are used, to ensure compatibility with any platform. Please note that each node requires its own memory space, so memory usage increases linearly with the number of nodes.
}
}
\value{
Returns a 3D array containing data series from the selected variables.
}
\author{Marcelo C. Pereira}
\note{
If the selected files don't have the same columns available (names and instances), an error is produced.
}
\seealso{
\code{\link{list.files.lsd}()}
\code{\link{read.4d.lsd}()},
\code{\link{read.single.lsd}()},
\code{\link{read.multi.lsd}()},
\code{\link{read.list.lsd}()},
\code{\link{read.raw.lsd}()}
}
\examples{
# get the list of file names of example LSD results
files <- list.files.lsd( system.file( "extdata", package = "LSDinterface" ) )

# read first instance of all variables from files (one level each),
# pasting the directory where the example files are (not required if in working dir)
inst1Array <- read.3d.lsd( files )
print( inst1Array[ 5 : 10, 1 : 7, 1 ] )
print( inst1Array[ 5 : 10, 1 : 7, 2 ] )
print( inst1Array[ 5 : 10, 1 : 7, 3 ] )

# read first instance of a set of variables named '_A1p' and '_growth1'
ab1Array <- read.3d.lsd( files, c( "_A1p", "_growth1" ) )
print( ab1Array[ 20 : 25, , 1 ] )
print( ab1Array[ 20 : 25, , 2 ] )
print( ab1Array[ 20 : 25, , 3 ] )

# read instance 2 of all variables, skipping the initial 20 time steps
# and keeping up to 30 time steps (from t = 21 up to t = 30)
inst2Array21_30 <- read.3d.lsd( files, skip = 20, nrows = 30, instance = 2 )
print( inst2Array21_30[ , 1 : 7, 1 ] )
print( inst2Array21_30[ , 1 : 7, 2 ] )
}
\keyword{ interface }
\keyword{ file }
\keyword{ datasets }
\keyword{ database }
