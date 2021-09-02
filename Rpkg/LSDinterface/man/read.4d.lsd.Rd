\name{read.4d.lsd}
\alias{read.4d.lsd}
\title{
Read all instances of LSD variables (time series) from multiple LSD results file into a 4D array
}
\description{
This function reads the data series associated to all instances of each selected variable from a set of LSD results files (.res) and saves them into a 4-dimensional array (time step x variable x instance x file).
}
\usage{
read.4d.lsd( files, col.names = NULL, nrows = -1, skip = 0,
             check.names = TRUE, pool = FALSE, nnodes = 1 )
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
  \item{pool}{
logical. If TRUE, variables instances from all files are concatenated (by columns) as a single file. If FALSE (the default), each file is saved as a separated dimension (fourth) in the array.
}
  \item{nnodes}{
integer: the maximum number of parallel computing nodes (parallel threads) in the current computer to be used for reading the files. The default, \code{nnodes = 1}, means single thread processing (no parallel threads). If equal to zero, creates up to one node per CPU core. Only \code{PSOCK} clusters are used, to ensure compatibility with any platform. Please note that each node requires its own memory space, so memory usage increases linearly with the number of nodes.
}
}
\value{
Returns a 4D array containing data series for each instance from the selected variables.
}
\author{Marcelo C. Pereira}
\note{
If the selected files don't have the same columns available (names), an error is produced.
When 'pool = TRUE', the produced array is still 4-dimensional but the fourth dimension has just one value (= 1). Pooling require that all files contains EXACTLY the same variables (number of instances may be different).
}
\seealso{
\code{\link{list.files.lsd}()}
\code{\link{read.3d.lsd}()},
\code{\link{read.single.lsd}()},
\code{\link{read.multi.lsd}()},
\code{\link{read.list.lsd}()},
\code{\link{read.raw.lsd}()}
}
\examples{
# get the list of file names of example LSD results
files <- list.files.lsd( system.file( "extdata", package = "LSDinterface" ) )

# read all instances of all variables from files,
allArray <- read.4d.lsd( files )
print( allArray[ 1 : 10, 1 : 7, 1, 1 ] ) # 1st instance of 1st file (7 vars and 10 times)
print( allArray[ 1 : 10, 9, , 2 ] ) # all instances of 9th variable in 2nd file (10 times)
print( allArray[ 50, 8, , ] ) # all instances of all files of 8th variable for t=50

# the same, but pooling all files into a single one
allArrayPool <- read.4d.lsd( files, pool = TRUE )
print( allArrayPool[ 1 : 10, 1 : 7, 1, 1 ] ) # 1st instance of 1st file (7 vars and 10 times)
print( allArrayPool[ 1 : 10, 9, 4 : 9 , 1 ] ) # 6 instances of 9th variable in 2nd file (10 times)
print( allArrayPool[ 50, 8, 4 : 9, ] ) # 6 instances of all files of 8th variable for t=50

# read instances of a set of variables named '_A1p' and '_growth1'
abArray <- read.4d.lsd( files, c( "_A1p", "_growth1" ) )
print( abArray[ 1 : 10, , 1, 2 ] ) # 1st instances of 2nd file (all vars and 10 times)
print( abArray[ 1 : 10, 2, , 3 ] ) # all instances of 2nd variable in3rd file (10 times)
print( abArray[ 50, 1, , ] ) # all instances of all files of 1st variable for t=50

# read all variables/variables, skipping the initial 20 time steps
# and keeping up to 30 time steps (from t = 21 up to t = 30)
allArray21_30 <- read.4d.lsd( files, skip = 20, nrows = 30 )
print( allArray21_30[ , 9, , 2 ] ) # all instances of 9th variable in 2nd file
print( allArray21_30[ 10, 8, , ] ) # all instances of all files of 8th variable for t=30
}
\keyword{ interface }
\keyword{ file }
\keyword{ datasets }
\keyword{ database }
