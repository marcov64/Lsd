\name{read.raw.lsd}
\alias{read.raw.lsd}
\title{
Read LSD results file and clean variables names
}
\description{
This function reads all the data series in a LSD results file (.res).
}
\usage{
read.raw.lsd( file, nrows = -1, skip = 0, col.names = NULL,
              check.names = TRUE, clean.names = FALSE, instance = 0,
              posit = NULL, posit.match = c( "fixed", "glob", "regex" ) )
}
\arguments{
  \item{file}{
the name of the LSD results file which the data are to be read from. If it does not contain an absolute path, the file name is relative to the current working directory, \code{\link[base]{getwd}()}. This can be a compressed file (see file) and must include the appropriated extension (usually \code{.res} or \code{.res.gz}).
}
  \item{nrows}{
integer: the maximum number of time steps (rows) to read in. Negative and other invalid values are ignored. The default is to read all rows.
}
  \item{skip}{
integer: the number of time steps (rows) of the results file to skip before beginning to read data. The default is to read from the first time step (t = 1).
}
  \item{col.names}{
a vector of optional names for the variables. The default is to read all variables. The names must to be in LSD/C++ format, without dots (".") in the name. Any dot (and trailing characters) will be automatically removed.
}
  \item{check.names}{
logical. If \code{TRUE} the names of the variables are checked to ensure that they are syntactically valid variable names. If necessary they are adjusted to ensure that there are no duplicates.
}
  \item{clean.names}{
logical. If \code{TRUE} the names of the variables in the columns are "cleaned" to remove extra information from the header in the LSD results file. This option is incompatible (and will be ignored) when multiple instances of a single variable are selected. If \code{FALSE}, the default, preserve extra information in the names.
}
  \item{instance}{
integer: the instance of the variable to be read, for variables that exist in more than one object. This number is based on the relative position (column) of the variable in the results file. The default (0) is to read all instances.
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

When \code{posit} is supplied together with \code{col.names} or \code{instance}, the selection process is done in two steps. Firstly, the (absolute) instances defined by \code{posit} are selected. Secondly, the column names and (relative) instance positions set by \code{col.names} and \code{instance} are selected out of the first selection set.

See \code{\link{select.colnames.lsd}} and \code{\link{select.colattrs.lsd}} for examples on how to apply advanced selection options.
}
\value{
Returns a single matrix containing all variables' time series contained in the results file.
}
\author{Marcelo C. Pereira}
\note{
The returned matrix may be potentially very wide.
See \code{\link{read.single.lsd}} for more polished column names. To use multiple results files simultaneously, see \code{\link{read.list.lsd}} and \code{\link{read.3d.lsd}}.
}
\seealso{
\code{\link{list.files.lsd}()}
\code{\link{read.single.lsd}()},
\code{\link{read.multi.lsd}()},
\code{\link{read.list.lsd}()},
\code{\link{read.3d.lsd}()},
\code{\link{read.4d.lsd}()},
\code{\link{select.colattrs.lsd}()},
\code{\link{select.colnames.lsd}()}
}
\examples{
# get the list of file names of example LSD results
files <- list.files.lsd( system.file( "extdata", package = "LSDinterface" ) )

# read all instances of all variables of first file,
bigTable <- read.raw.lsd( files[ 1 ] )
print( bigTable[ 1 : 5, 1 : 7 ] )

# read all instances of all variables, skipping the initial 20 time steps
# and keeping up to 30 time steps (from t = 21 up to t = 30)
all21_30 <- read.raw.lsd( files[ 2 ], skip = 20, nrows = 30 )
print( all21_30[ , 1 : 7 ] )

# read the third instances of a set of variables named '_A1p' and '_growth1'
abTable <- read.raw.lsd( files[ 1 ], col.names = make.names( c( "_A1p", "_growth1" ) ),
                         instance = 3 )
print( abTable[ 10 : 20, ] )

# read instances of variable '_A1p' for the second and fourth objects under
# any top-level object (use globbing)
a24 <- read.raw.lsd( files[ 1 ], col.names = make.names( "_A1p" ),
                     posit = c( "*_2", "*_4" ), posit.match = "glob" )
print( a24[ 1 : 10, ] )
}
\keyword{ interface }
\keyword{ file }
\keyword{ datasets }
\keyword{ database }
