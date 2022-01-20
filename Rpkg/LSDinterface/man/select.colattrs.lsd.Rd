\name{select.colattrs.lsd}
\alias{select.colattrs.lsd}
\title{
Select a subset of a LSD results matrix (by variable attributes)
}
\description{
This function select a subset of a LSD results matrix (as produced by \code{\link{read.raw.lsd}}) by the variable attributes, considering the LSD object position and the time span.
}
\usage{
select.colattrs.lsd( dataSet, info, col.names = NULL, init.value = NA,
                     init.time = NA, end.time = NA, posit = NULL,
                     posit.match = c( "fixed", "glob", "regex" ) )
}
\arguments{
  \item{dataSet}{
matrix produced by the invocation of \code{\link{read.raw.lsd}}, \code{\link{read.single.lsd}}, \code{\link{read.multi.lsd}} or \code{\link{read.list.lsd}} (a single matrix a time) functions.
}
  \item{info}{
data frame produced by \code{\link{info.details.lsd}} for the same results file from where \code{dataSet} was extracted.
}
  \item{col.names}{
a vector of optional names for the variables to select from. The default is to select from all variables.
}
  \item{init.value}{
initial value attributed to the variable(s) to select.
}
  \item{init.time}{
initial time attributed to the variable(s) to select.
}
  \item{end.time}{
end time attributed to the variable(s) to select.
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

See also the \code{read.XXXX.lsd} functions which may select just specific \code{posit} object instances when loading LSD results. If only a single set of instances is required, this would be more efficient than using this function.
}
\value{
Returns a single matrix containing the selected variables' time series contained in the original data set.
}
\author{Marcelo C. Pereira}
\note{
If only variable names selection is needed, \code{\link{select.colnames.lsd}} is more efficient because information pre-processing (\code{\link{info.details.lsd}}) is not required.
}
\seealso{
\code{\link{list.files.lsd}()}
\code{\link{info.details.lsd}()},
\code{\link{select.colnames.lsd}()}
}
\examples{
# get the list of file names of example LSD results
files <- list.files.lsd( system.file( "extdata", package = "LSDinterface" ) )

# read all instances of all variables of first file
bigTable <- read.raw.lsd( files[ 1 ] )

# build the info table
info <- info.details.lsd( files[ 1 ] )

# extract specific instances of a set of variables named '_A1p' and '_growth1'
abFirst2 <- select.colattrs.lsd( bigTable, info, c( "_A1p", "_growth1" ),
                                 posit = c( "1_2", "1_5" ) )
print( abFirst2[ 50 : 60, ] )

# extract instances of variable '_A1p' that start at time step t = 1
# for the second and fourth objects under any top-level object (use globbing)
a24 <- select.colattrs.lsd( bigTable, info, make.names( "_A1p" ), init.time = 1,
                            posit = c( "*_2", "*_4" ), posit.match = "glob" )
print( a24[ 1 : 10, ] )

# extract all second-level object instances of all variables
aSec <- select.colattrs.lsd( bigTable, info, posit = "*_*", posit.match = "glob" )
print( aSec[ 1 : 10, ] )

# extract just top-level object instances variables
aTop <- select.colattrs.lsd( bigTable, info, posit = "^[0-9]+$",
                             posit.match = "regex" )
print( aTop[ 1 : 10, ] )
}
\keyword{ datasets }
\keyword{ database }
