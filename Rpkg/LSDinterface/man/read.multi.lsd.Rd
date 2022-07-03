\name{read.multi.lsd}
\alias{read.multi.lsd}
\title{
Read all instances of LSD variables (time series) from a LSD results file
}
\description{
This function reads the data series associated to all instances of each selected variable from a LSD results file (.res).
}
\usage{
read.multi.lsd( file, col.names = NULL, nrows = -1, skip = 0,
                check.names = TRUE, posit = NULL,
                posit.match = c( "fixed", "glob", "regex" ),
                posit.cols = FALSE )
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
logical. If \code{TRUE} the names of the variables are checked to ensure that they are syntactically valid variable names. If necessary they are adjusted (by make.names) so that they are, and also to ensure that there are no duplicates.
}
  \item{posit}{
a string, a vector of strings or an integer vector describing the LSD object position of the variable(s) to select. If an integer vector, it should define the position of a SINGLE LSD object. If a string or vector of strings, each element should define one or more different LSD objects, so the returning matrix will contain variables from more than one object. By setting \code{posit.match}, globbing (wildcard), and regular expressions can be used to select multiple objects at once; in this case, all matching objects are returned.
}
  \item{posit.match}{
a string defining how the \code{posit} argument, if provided, should be matched against the LSD object positions. If equal to \code{"fixed"}, the default, only exact matching is done. \code{"glob"} allows using simple wildcard characters ('*' and '?') in \code{posit} for matching. If \code{posit.match="regex"} interpret \code{posit} as POSIX 1003.2 extended regular expression(s). See \code{\link[=regex]{regular expressions}} for details of the different types of regular expressions. Options can be abbreviated.
}
  \item{posit.cols}{
logical. If \code{TRUE} just the position information is used as the names of the columns in each variable list. If \code{FALSE}, the default, the column names include all the header information contained in the LSD results file (name, position and time span).
}
}
\details{
Selection restriction arguments can be provided as needed; when not specified, all available cases are selected.

When \code{posit} is supplied together with \code{col.names}, the selection process is done in two steps. Firstly, the column names set by \code{col.names} are selected. Secondly, the instances defined by \code{posit} are selected from the first selection set.

See \code{\link{select.colnames.lsd}} and \code{\link{select.colattrs.lsd}} for examples on how to apply advanced selection options.
}
\value{
Returns a named list of matrices, each containing one of the selected variables' time series from the results file.

Variable names are converted to valid R ones when defining list names. Matrix column names are not "cleaned", even for single instanced variables. The column names include all the header information contained in the LSD results file.
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
length( macroList )           # number of lists holding variables
names( macroList )            # name of each list
print( macroList[[ 1 ]][ 1 : 5, , drop = FALSE ] )
print( macroList$X_A1p[ 10 : 20, ] )

# read first instance of 2 variables, skipping the initial 20 time steps
# and keeping up to 30 time steps (from t = 21 up to t = 30), positions in cols
varsList21_30 <- read.multi.lsd( files[ 2 ], c( "_A1p", "_growth1" ),
                                 skip = 20, nrows = 30, posit.cols = TRUE )
print( varsList21_30[[ 1 ]] )
print( varsList21_30$X_growth1 )
}
\keyword{ interface }
\keyword{ file }
\keyword{ datasets }
\keyword{ database }
