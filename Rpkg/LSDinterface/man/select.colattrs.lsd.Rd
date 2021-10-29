\name{select.colattrs.lsd}
\alias{select.colattrs.lsd}
\title{
Select a subset of a LSD results matrix (by variable attributes)
}
\description{
This function select a subset of a LSD results matrix (as produced by \code{\link{read.raw.lsd}}) by the variable attributes, considering the LSD object position and the time span.
}
\usage{
select.colattrs.lsd( dataSet, info, col.names = NULL, posit = NULL,
                     init.value = NA, init.time = NA, end.time = NA )
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
  \item{posit}{
a string, a vector of strings or an integer vector describing the LSD object position of the variable(s) to select. If a string or an integer vector, it should define the position of a SINGLE LSD object. If a vector of strings, each element of the vector should define a different LSD object, so the returning matrix will contain variables from more than one object.
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
}
\details{
Selection restriction parameters can be provided as needed; when not specified, each selection dimension include all available cases.
}
\value{
Returns a single matrix containing the selected variables' time series contained in the original data set.
}
\author{Marcelo C. Pereira}
\note{
If only variable names selection is needed, \code{\link{select.colnames.lsd}} is more efficient because information preprocessing (\code{\link{info.details.lsd}}) is not required.
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

# read some instances of a set of variables named '_A1p' and '_growth1'
abFirst2 <- select.colattrs.lsd( bigTable, info, c( "_A1p", "_growth1" ),
                                 posit = c( "1_2", "1_5" ) )
print( abFirst2[ 50 : 60, ] )

# read instances of variable '_A1p' that start at time step t = 1
a50 <- select.colattrs.lsd( bigTable, info, make.names( "_A1p" ), init.time = 1 )
print( a50[ 1 : 10, ] )
}
\keyword{ datasets }
\keyword{ database }
