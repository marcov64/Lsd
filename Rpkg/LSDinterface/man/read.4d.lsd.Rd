\name{read.4d.lsd}
\alias{read.4d.lsd}
\title{
Read multiple instances of LSD variables (time series) from a set of LSD results file into a 4D array
}
\description{
This function reads the data series associated to a set of instances of each selected variable from a set of LSD results files (.res) and saves them into a 4-dimensional array (time x variable x instance x file).
}
\usage{
read.4d.lsd( files, col.names = NULL, nrows = -1, skip = 0,
             check.names = TRUE, pool = FALSE, nnodes = 1,
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
  \item{pool}{
logical. If \code{TRUE}, variables instances from all files are concatenated (by columns) as a single 3-dimensional array. If \code{FALSE} (the default), each file is saved as a separated dimension (fourth) in the array.
}
  \item{nnodes}{
integer: the maximum number of parallel computing nodes (parallel threads) in the current computer to be used for reading the files. The default, \code{nnodes = 1}, means single thread processing (no parallel threads). If equal to zero, creates up to one node per CPU core. Only \code{PSOCK} clusters are used, to ensure compatibility with any platform. Please note that each node requires its own memory space, so memory usage increases linearly with the number of nodes.
}
  \item{posit}{
a string, a vector of strings or an integer vector describing the LSD object position of the variable(s) to select. If an integer vector, it should define the position of a SINGLE LSD object. If a string or vector of strings, each element should define one or more different LSD objects, so the returning matrix will contain variables from more than one object. By setting \code{posit.match}, globbing (wildcard), and regular expressions can be used to select multiple objects at once; in this case, all matching objects are returned.
}
  \item{posit.match}{
a string defining how the \code{posit} argument, if provided, should be matched against the LSD object positions. If equal to \code{"fixed"}, the default, only exact matching is done. \code{"glob"} allows using simple wildcard characters ('*' and '?') in \code{posit} for matching. If \code{posit.match="regex"} interpret \code{posit} as POSIX 1003.2 extended regular expression(s). See \code{\link[=regex]{regular expressions}} for details of the different types of regular expressions. Options can be abbreviated.
}
}
\details{
Selection restriction arguments can be provided as needed; when not specified, all available cases are selected.

When \code{posit} is supplied together with \code{col.names}, the selection process is done in two steps. Firstly, the (absolute) instances defined by \code{posit} are selected. Secondly, the column names set by \code{col.names} are selected out of the first selection set.

See \code{\link{select.colnames.lsd}} and \code{\link{select.colattrs.lsd}} for examples on how to apply advanced selection options.
}
\value{
Returns a 4D array containing data series for each instance from the selected variables.

The array dimension order is: time x variable x instance x file.

When \code{pool = TRUE}, the produced array is 3-dimensional. Pooling require that selected columns contains EXACTLY the same variables (number of instances may be different).
}
\author{Marcelo C. Pereira}
\note{
If the selected files don't have the same columns available (names), after column selection, an error is produced.

When using the option \code{pool = TRUE}, columns from multiple files are consolidated with their original names plus the file name, to keep all column names unique. Use \code{\link{name.var.lsd}} to get just the LSD name of the variable corresponding to each column.
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
print( allArray[ 11 : 20, "X_A1p", , "Sim1_2" ] ) # all instances of _A1p in Sim1_2 (10 times)
print( allArray[ 50, 9, , ] ) # all instances of all files of 9th variable for t=50

# the same, but pooling all files into a single (3D!) array
allArrayPool <- read.4d.lsd( files, pool = TRUE )
print( allArrayPool[ 1 : 10, 8 : 9, 3 ] ) # 3rd instances of last 2 vars (10 times)
print( allArrayPool[ 11 : 20, "X_A1p", 4 : 9 ] ) # 6 instances of _A1p variable (10 times)
print( allArrayPool[ 50, 9, 4 : 9 ] ) # 6 instances of all files of 9th variable for t=50

# read instances of a set of variables named '_A1p' and '_growth1'
abArray <- read.4d.lsd( files, c( "_A1p", "_growth1" ) )
print( abArray[ 1 : 10, , 1, 2 ] ) # 1st instances of 2nd file (all vars and 10 times)
print( abArray[ 11 : 20, 2, , "Sim1_3" ] ) # all instances of 2nd variable in Sim1_3 (10 times)
print( abArray[ 50, "X_A1p", , ] ) # all instances of all files of _A1p variable for t=50

# read all variables/variables, skipping the initial 20 time steps
# and keeping up to 30 time steps (from t = 21 up to t = 30)
allArray21_30 <- read.4d.lsd( files, skip = 20, nrows = 30 )
print( allArray21_30[ , "X_growth1", , 2 ] ) # all instances of _growth1 variable in 2nd file
print( allArray21_30[ 10, 8, , ] ) # all instances of all files of 8th variable for t=30

# read all variables in second-level objects, using up to 2 cores for processing
abArray2 <- read.4d.lsd( files, posit = "*_*", posit.match = "glob", nnodes = 2 )
print( abArray2[ 11 : 20, , 5, "Sim1_1" ] ) # 5th instances in Sim1_1 file
}
\keyword{ interface }
\keyword{ file }
\keyword{ datasets }
\keyword{ database }
