\name{read.single.lsd}
\alias{read.single.lsd}
\title{
Read LSD variables (time series) from a LSD results file (a single instance of each variable only)
}
\description{
This function reads the data series associated to one instance of each selected variable from a LSD results file (.res). Just a single instance (time series of a single LSD object) is read at each call.
}
\usage{
read.single.lsd( file, col.names = NULL, nrows = -1, skip = 0,
                 check.names = TRUE, instance = 1, posit = NULL,
                 posit.match = c( "fixed", "glob", "regex" ) )
}
\arguments{
  \item{file}{
the name of the LSD results file which the data are to be read from. If it does not contain an absolute path, the file name is relative to the current working directory, \code{\link[base]{getwd}()}. This can be a compressed file (see file) and must include the appropriated extension (usually \code{.res} or \code{.res.gz}).
}
  \item{col.names}{
a vector of optional names for the variables. The default is to read all variables. The names must to be in LSD/C++ format, without dots (".") in the name. Any dot (and trailing characters) will be automatically removed.
}
  \item{nrows}{
integer: the maximum number of time steps (rows) to read in. Negative and other invalid values are ignored. The default is to read all rows.
}
  \item{skip}{
integer: the number of time steps (rows) of the results file to skip before beginning to read data. The default is to read from the first time step (t = 1).
}
  \item{check.names}{
logical. If \code{TRUE} the names of the variables are checked to ensure that they are syntactically valid variable names. If necessary they are adjusted to ensure that there are no duplicates.
}
  \item{instance}{
integer: the instance of the variable to be read, for variables that exist in more than one object. This number is based on the relative position (column) of the variable in the results file. The default (0) is to read all instances.
}
  \item{posit}{
a string, a vector of strings or an integer vector describing the LSD object position of the variable(s) to select. If an integer vector, it should define the position of a SINGLE LSD object. If a string or vector of strings, each element should define one or more different LSD objects, so the returning matrix will contain variables from more than one object. By setting \code{posit.match}, globbing (wildcard), and regular expressions can be used to select multiple objects at once.
}
  \item{posit.match}{
a string defining how the \code{posit} argument, if provided, should be matched against the LSD object positions. If equal to \code{"fixed"}, the default, only exact matching is done. \code{"glob"} allows using simple wildcard characters ('*' and '?') in \code{posit} for matching. If \code{posit.match="regex"} interpret \code{posit} as POSIX 1003.2 extended regular expression(s). See \code{\link[=regex]{regular expressions}} for details of the different types of regular expressions. Options can be abbreviated.
}
}
\details{
Selection restriction arguments can be provided as needed; when not specified, all available cases are considered, but just one (relative) \code{instance} is considered.

When \code{posit} is supplied, the selection process is done in two steps. Firstly, the (absolute) instances defined by \code{posit} are selected. Secondly, the column names and the(relative) instance position set by \code{col.names} and \code{instance} are selected out of the first selection set.

See \code{\link{select.colnames.lsd}} and \code{\link{select.colattrs.lsd}} for examples on how to apply advanced selection options.
}
\value{
Returns a matrix containing the selected variables' time series contained in the results file.
}
\author{Marcelo C. Pereira}
\note{
This function is useful to extract time series for variables that are single instanced, like summary statistics.
For multi-instanced variables, see \code{\link{read.multi.lsd}}. For extracting data from multiple similar files (like sensitivity analysis results), see \code{\link{read.list.lsd}} (multi-instanced variables) and \code{\link{read.3d.lsd}} (single-instanced variables).
}

\seealso{
\code{\link{list.files.lsd}()}
\code{\link{read.multi.lsd}()},
\code{\link{read.list.lsd}()},
\code{\link{read.3d.lsd}()},
\code{\link{read.4d.lsd}()},
\code{\link{read.raw.lsd}()}
}
\examples{
# get the list of file names of example LSD results
files <- list.files.lsd( system.file( "extdata", package = "LSDinterface" ) )

# load first .res file into a simple matrix (first instances only)
macroVar <- read.single.lsd( files[ 1 ] )
print( macroVar[ 10 : 20, 5 : 9 ] )

# read second instance of a set of variables named '_A1p' and '_growth1'
ag2Table <- read.single.lsd( files[ 2 ], col.names = c( "_A1p", "_growth1" ),
                             instance = 2 )
print( ag2Table[ 10 : 15, ] )

# read first instance of all variables, skipping the initial 20 time steps
# and keeping up to 30 time steps (from t = 21 up to t = 30)
var21_30 <- read.single.lsd( files[ 3 ], skip = 20, nrows = 30 )
print( var21_30[ , 1 : 7 ] )

# read third instance of all variables at the second object level
var2_3_5 <- read.single.lsd( files[ 1 ], instance = 3, posit = "*_*",
                            posit.match = "glob" )
print( var2_3_5[ 20 : 25, ] )
}
\keyword{ interface }
\keyword{ file }
\keyword{ datasets }
\keyword{ database }
