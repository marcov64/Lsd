\name{select.colattrs.lsd}
\alias{select.colattrs.lsd}
\title{
Select a subset of a LSD results data frame (by variable attributes)
}
\description{
This function select a subset of a LSD results data frame (as produced by \code{\link{read.raw.lsd}}) by the variable attributes, considering the LSD object position and the time span.
}
\usage{
select.colattrs.lsd(dataSet, info, col.names = NA, posit = NA, init.value = NA, init.time = NA, end.time = NA, stringsAsFactors = default.stringsAsFactors())
}
\arguments{
  \item{dataSet}{
data frame produced by the invocation of \code{\link{read.raw.lsd}}, \code{\link{read.lsd}}, \code{\link{read.multi.lsd}} or \code{\link{read.list.lsd}} (a single data frame a time) functions.
}
  \item{info}{
data frame produced by \code{\link{read.info.lsd}} for the same results file from where 'dataSet' was extracted.
}
  \item{col.names}{
a vector of optional names for the variables to select from. The default is to select from all variables.
}
  \item{posit}{
string or integer vector describing the LSD object position of the variable(s) to select.
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
  \item{stringsAsFactors}{
logical: should character vectors be converted to factors?
}
}
\details{
Selection restriction parameters can be provided as needed; when not specified, each selection dimension include all available cases.
}
\value{
Returns a single data frame containing the selected variables' time series contained in the original data set.
}
\author{Marcelo C. Pereira}
\note{
If only variable names selection is needed, \code{\link{select.colnames.lsd}} is more efficient because information pre-processing (\code{\link{read.info.lsd}}) is not required.
}
\seealso{
\code{\link{select.colnames.lsd}}
}
\examples{
## reads all instances of all variables
#bigTable <- read.raw.lsd("Sim1_1.res")

## read some instances of a set of variables named 'a' and 'b'
#abFirst2 <- select.colattrs.lsd(bigTable, c("a", "b"), posit = c("1_1", "1_2"))

## reads instances of variable 'a' that start at time step t = 50
#a50 <- select.colattrs.lsd(bigTable, "a", init.time = 50)
}
% Add one or more standard keywords, see file 'KEYWORDS' in the
% R documentation directory.
\keyword{ ~kwd1 }
\keyword{ ~kwd2 }% __ONLY ONE__ keyword per line
