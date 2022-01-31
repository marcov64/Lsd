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
             check.names = TRUE, instance = 1, nnodes = 1,
             posit = NULL, posit.match = c( "fixed", "glob", "regex" ) )
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
logical. If \code{TRUE} the names of the variables are checked to ensure that they are syntactically valid variable names. If necessary they are adjusted (by make.names) so that they are, and also to ensure that there are no duplicates.
}
  \item{instance}{
integer: the instance of the variable to be read, for variables that exist in more than one object. This number is based on the position (column) of the variable in the results file. The default (1) is to read first instances.
}
  \item{nnodes}{
integer: the maximum number of parallel computing nodes (parallel threads) in the current computer to be used for reading the files. The default, \code{nnodes = 1}, means single thread processing (no parallel threads). If equal to zero, creates up to one node per CPU core. Only \code{PSOCK} clusters are used, to ensure compatibility with any platform. Please note that each node requires its own memory space, so memory usage increases linearly with the number of nodes.
}
  \item{posit}{
a string, a vector of strings or an integer vector describing the LSD object position of the variable(s) to select. If an integer vector, it should define the position of a SINGLE LSD object. If a string or vector of strings, each element should define one or more different LSD objects, so the returning matrix may contain variables from more than one object. By setting \code{posit.match}, globbing (wildcard), and regular expressions can be used to select multiple objects at once.
}
  \item{posit.match}{
a string defining how the \code{posit} argument, if provided, should be matched against the LSD object positions. If equal to \code{"fixed"}, the default, only exact matching is done. \code{"glob"} allows using simple wildcard characters ('*' and '?') in \code{posit} for matching. If \code{posit.match="regex"} interpret \code{posit} as POSIX 1003.2 extended regular expression(s). See \code{\link[=regex]{regular expressions}} for details of the different types of regular expressions. Options can be abbreviated.
}
}
\details{
Selection restriction arguments can be provided as needed; when not specified, all available cases are considered, but just one \code{instance} is considered.

When \code{posit} is supplied together with \code{col.names} or \code{instance}, the selection process is done in two steps. Firstly, the column names and the instance position set by \code{col.names} and \code{instance} are selected. Secondly, the instances defined by \code{posit} are selected from the first selection set.

See \code{\link{select.colnames.lsd}} and \code{\link{select.colattrs.lsd}} for examples on how to apply advanced selection options.
}
\value{
Returns a 3D array containing data series from the selected variables.

The array dimension order is: time x variable x file.
}
\author{Marcelo C. Pereira}
\note{
If the selected files don't have the same columns available (names and instances), after column selection, an error is produced.
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

# read first instance of a set of variables named _A1p and _growth1
ab1Array <- read.3d.lsd( files, c( "_A1p", "_growth1" ) )
print( ab1Array[ 20 : 25, , 1 ] )
print( ab1Array[ 20 : 25, , 2 ] )
print( ab1Array[ 20 : 25, , 3 ] )

# read instance 2 of all variables, skipping the initial 20 time steps
# and keeping up to 30 time steps (from t = 21 up to t = 30)
inst2Array21_30 <- read.3d.lsd( files, skip = 20, nrows = 30, instance = 2 )
print( inst2Array21_30[ , , "Sim1_1" ] )   # use the file name to retrieve
print( inst2Array21_30[ , , "Sim1_2" ] )

# read instance 5 of all variables in second-level objects, using up to 2 cores
inst5array2 <- read.3d.lsd( files, instance = 2, posit = "*_*",
                            posit.match = "glob", nnodes = 2 )
print( inst5array2[ 11 : 20, , 1 ] )
}
\keyword{ interface }
\keyword{ file }
\keyword{ datasets }
\keyword{ database }
