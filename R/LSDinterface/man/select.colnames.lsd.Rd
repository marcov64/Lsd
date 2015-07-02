\name{select.colnames.lsd}
\alias{select.colnames.lsd}
\title{
Select a subset of a LSD results data frame (by column/variable names)
}
\description{
This function select a subset of a LSD results data frame (as produced by read.raw.lsd) by the column (variable) names, considering only the name part of the column labels.
}
\usage{
select.colnames.lsd(dataSet, col.names, instance = 0, stringsAsFactors = default.stringsAsFactors())
}
%- maybe also 'usage' for other objects documented here.
\arguments{
  \item{dataSet}{
data frame produced by the invocation of read.raw.lsd, read.lsd, read.multi.lsd or read.list.lsd (a single data frame a time) functions.
}
  \item{col.names}{
a vector of optional names for the variables. The default is to read all variables.
}
  \item{instance}{
integer: the instance of the variable to be read, for variables that exist in more than one object. This number is based on the position (column) of the variable in the results file. The default (0) is to read all instances.
}
  \item{stringsAsFactors}{
logical: should character vectors be converted to factors?
}
}
\value{
Returns a single data frame containing the selected variables' time series contained in the original data set.
}
\author{Marcelo C. Pereira}
\seealso{
\code{\link{select.colattrs.lsd}}
}
\examples{
## reads all instances of all variables
#bigTable <- read.raw.lsd("Sim1_1.res")

## read all instances of a set of variables named 'a' and 'b'
#abTable <- select.colnames.lsd(bigTable, c("a", "b"))

## reads first instance of variable 'a'
#aFirst <- select.colnames.lsd(bigTable, "a", instance = 1)
}
% Add one or more standard keywords, see file 'KEYWORDS' in the
% R documentation directory.
\keyword{ ~kwd1 }
\keyword{ ~kwd2 }% __ONLY ONE__ keyword per line
