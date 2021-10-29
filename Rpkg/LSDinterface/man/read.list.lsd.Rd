\name{read.list.lsd}
\alias{read.list.lsd}
\title{
Read one or all instances of LSD variables (time series) from a LSD results file into a list
}
\description{
This function reads the data series associated to a specific or all instances of each selected variable from a LSD results file (.res) and saves them into separated matrices (one per variable).
}
\usage{
read.list.lsd( files, col.names = NULL, nrows = -1, skip = 0,
               check.names = TRUE, instance = 0, pool = FALSE, nnodes = 1 )
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
integer: the instance of the variable to be read, for variables that exist in more than one object. This number is based on the position (column) of the variable in the results file. The default (0) is to read all instances.
}
  \item{pool}{
logical. If TRUE, variables instances from all files are concatenated (by columns) into a single matrix. If FALSE (the default), each file is saved as a separated matrix.
}
  \item{nnodes}{
integer: the maximum number of parallel computing nodes (parallel threads) in the current computer to be used for reading the files. The default, \code{nnodes = 1}, means single thread processing (no parallel threads). If equal to zero, creates up to one node per CPU core. Only \code{PSOCK} clusters are used, to ensure compatibility with any platform. Please note that each node requires its own memory space, so memory usage increases linearly with the number of nodes.
}
}
\value{
Returns a list containing matrices with the selected variables' time series in the results file. If \code{pool = TRUE}, the list contains a single, consolidated matrix (column names are not unique).
}
\author{Marcelo C. Pereira}
\note{
When using the option 'pool = TRUE', columns from multiple files are consolidated with their original names, so names will not be unique anymore. You may use \code{\link{unique}} to change column names so they become unique, if required.
The returned matrices may be potentially very wide, in particular if variables are not well selected(see col.names above) or if there is a large number of instances.
}
\seealso{
\code{\link{list.files.lsd}()}
\code{\link{read.single.lsd}()},
\code{\link{read.multi.lsd}()},
\code{\link{read.3d.lsd}()},
\code{\link{read.4d.lsd}()},
\code{\link{read.raw.lsd}()},
}
\examples{
# get the list of file names of example LSD results
files <- list.files.lsd( system.file( "extdata", package = "LSDinterface" ) )

# read all instances of all variables from three files (one matrix each),
tableList <- read.list.lsd( files )
print( tableList[[ 1 ]][ 1 : 5, 1 : 7 ] )
print( tableList[[ 2 ]][ 1 : 5, 1 : 7 ] )
print( tableList[[ 3 ]][ 1 : 5, 1 : 7 ] )

# read all instances of a set of variables named '_A1p' and '_growth1'
# and pool data
abTable <- read.list.lsd( files, c( "_A1p", "_growth1" ), pool = TRUE )
print( abTable[[ 1 ]][ 10 : 20, 5 : 9 ] )

# read instance 4 of all variables, skipping the initial 20 time steps
# and keeping up to 30 time steps (from t = 21 up to t = 30)
inst4List21_30 <- read.list.lsd( files, skip = 20, nrows = 30, instance = 4 )
print( inst4List21_30[[ 1 ]] )
print( inst4List21_30[[ 2 ]] )
}
\keyword{ interface }
\keyword{ file }
\keyword{ datasets }
\keyword{ database }
