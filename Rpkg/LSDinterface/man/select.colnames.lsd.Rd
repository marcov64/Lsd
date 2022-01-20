\name{select.colnames.lsd}
\alias{select.colnames.lsd}
\title{
Select a subset of a LSD results matrix (by column/variable names)
}
\description{
This function select a subset of a LSD results matrix (as produced by \code{\link{read.raw.lsd}}) by the column (variable) names, considering only the name part of the column labels.
}
\usage{
select.colnames.lsd( dataSet, col.names = NULL, instance = 0,
                     check.names = TRUE )
}
%- maybe also 'usage' for other objects documented here.
\arguments{
  \item{dataSet}{
matrix produced by the invocation of \code{\link{read.raw.lsd}}, \code{\link{read.single.lsd}}, \code{\link{read.multi.lsd}} or \code{\link{read.list.lsd}} (a single matrix a time) functions.
}
  \item{col.names}{
a vector of optional names for the variables. The default is to read all variables. The names must to be in LSD/C++ format, without dots (".") in the name. Any dot (and trailing characters) will be automatically removed.
}
  \item{instance}{
integer: the instance of the variable to be read, for variables that exist in more than one object. This number is based on the relative position (column) of the variable in the results file. The default (0) is to read all instances.
}
  \item{check.names}{
logical. If \code{TRUE} the names of the variables are checked to ensure that they are syntactically valid variable names. If necessary they are adjusted to ensure that there are no duplicates.
}
}
\details{
Selection restriction arguments can be provided as needed; when not specified, all available cases are selected.

See also the \code{read.XXXX.lsd} functions which may select just specific \code{col.names} columns or \code{instance} instances when loading LSD results. If only a single set of columns/instance is required, this may be more efficient than using this function.
}
\value{
Returns a single matrix containing the selected variables' time series contained in the original data set.
}
\author{Marcelo C. Pereira}
\note{
The variable/column names must be valid R column names (e.g., names do not start with a underscore). Use \code{\link{make.names}} if required.
}
\seealso{
\code{\link{list.files.lsd}()}
\code{\link{select.colattrs.lsd}()},
\code{\link{make.names}()}
}
\examples{
# get the list of file names of example LSD results
files <- list.files.lsd( system.file( "extdata", package = "LSDinterface" ) )

# read all instances of all variables in first file
bigTable <- read.raw.lsd( files[ 1 ] )
print( bigTable[ 1 : 10, 1 : 7 ] )

# extract all instances of a set of variables named '_A1p' and '_growth1'
abTable <- select.colnames.lsd( bigTable, make.names( c( "_A1p", "_growth1" ) ) )
print( abTable[ 10 : 20, ] )
}
\keyword{ datasets }
\keyword{ database }
