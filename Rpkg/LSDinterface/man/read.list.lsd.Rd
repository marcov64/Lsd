\name{read.list.lsd}
\alias{read.list.lsd}
\title{
Read one or more instances of LSD variables (time series) from a set of LSD results file into a list
}
\description{
This function reads the data series associated to a specific or a set of instances of each selected variable from a set of LSD results file (.res) and saves them into separated matrices (one per file).
}
\usage{
read.list.lsd( files, col.names = NULL, nrows = -1, skip = 0,
               check.names = TRUE, instance = 0, pool = FALSE, nnodes = 1,
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
integer: the instance of the variable to be read, for variables that exist in more than one object. This number is based on the position (column) of the variable in the results file. The default (0) is to read all instances.
}
  \item{pool}{
logical. If \code{TRUE}, variables instances from all files are concatenated (by columns) into a single matrix. If \code{FALSE} (the default), each file is saved as a separated matrix in a list.
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

When \code{posit} is supplied together with \code{col.names} or \code{instance}, the selection process is done in two steps. Firstly, the column names and instance positions set by \code{col.names} and \code{instance} are selected. Secondly, the instances defined by \code{posit} are selected from the first selection set.

See \code{\link{select.colnames.lsd}} and \code{\link{select.colattrs.lsd}} for examples on how to apply advanced selection options.
}
\value{
Returns a named list of matrices with the selected variables' time series in the results files. If \code{pool = TRUE}, the return value is a single, consolidated matrix (column names are not unique).

The matrices dimension order is: time x variable.

Matrix column names are only "cleaned" if there are just single instanced variables selected. When multiple instanced variables are present, the column names include all the header information contained in the LSD results file. The name of the LSD variable associated to any column name can be retrieved with \code{\link{name.var.lsd}}.
}
\author{Marcelo C. Pereira}
\note{
When using the option \code{pool = TRUE}, columns from multiple files are consolidated with their original names plus the file name, to keep all column names unique. Use \code{\link{name.var.lsd}} to get just the LSD name of the variable corresponding to each column.

The returned matrices may be potentially very wide, in particular if variables are not well selected(see \code{col.names} above) or if there is a large number of instances.
}
\seealso{
\code{\link{list.files.lsd}()}
\code{\link{name.var.lsd}()}
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
# and pool data into a single matrix
abTable <- read.list.lsd( files, c( "_A1p", "_growth1" ), pool = TRUE )
print( abTable[ 10 : 20, 10 : 12 ] )

# read instance 4 of all variables, skipping the initial 20 time steps
# and keeping up to 30 time steps (from t = 21 up to t = 30)
inst4List21_30 <- read.list.lsd( files, skip = 20, nrows = 30, instance = 4 )
print( inst4List21_30[[ 1 ]] )
print( inst4List21_30[[ 2 ]] )

# read all variables in top-level objects, using up to 2 cores for processing
instTop <- read.list.lsd( files, posit = 1, nnodes = 2 )
print( instTop$Sim1_1[ 11 : 20, ] )   # use the file name to retrieve list item
print( instTop$Sim1_2[ 11 : 20, ] )
}
\keyword{ interface }
\keyword{ file }
\keyword{ datasets }
\keyword{ database }
