\name{read.raw.lsd}
\alias{read.raw.lsd}
\title{
Read LSD results file and clean variables names
}
\description{
This function reads all the data series in a LSD results file (.res).
}
\usage{
read.raw.lsd(file, nrows = -1, skip = 0, stringsAsFactors = default.stringsAsFactors())
}
\arguments{
  \item{file}{
the name of the LSD results file which the data are to be read from. If it does not contain an absolute path, the file name is relative to the current working directory, getwd(). This can be a compressed file (see file) and must include the appropriated extension (usually .res or .res.gz).
}
  \item{nrows}{
integer: the maximum number of time steps (rows) to read in. Negative and other invalid values are ignored. The default is to read all rows.
}
  \item{skip}{
integer: the number of time steps (rows) of the results file to skip before beginning to read data. The default is to read from the first time step (t = 1).
}
  \item{stringsAsFactors}{
logical: should character vectors be converted to factors?
}
}
\value{
Returns a single data frame containing all variables' time series contained in the results file.
}
\author{Marcelo C. Pereira}
\note{
The returned data frame may be potentially very wide.
For extracting data in a more selective way, see \code{\link{read.lsd}} and \code{\link{read.multi.lsd}}. To use multiple results files simultaneously, see \code{\link{read.list.lsd}} and \code{\link{read.3d.lsd}}
}
\seealso{
\code{\link{read.lsd}}, 
\code{\link{read.multi.lsd}}, 
\code{\link{read.list.lsd}}, 
\code{\link{read.3d.lsd}}
}
\examples{
## reads all instances of all variables
#bigTable <- read.raw.lsd("Sim1_1.res")

## reads all instances of all variables, skipping the initial 100 time steps
## and keeping up to 200 time steps (from t = 101 up to t = 300)
#allDF100_300 <- read.raw.lsd("Sim1_1.res", skip = 100, nrows = 200)
}
% Add one or more standard keywords, see file 'KEYWORDS' in the
% R documentation directory.
\keyword{ ~kwd1 }
\keyword{ ~kwd2 }% __ONLY ONE__ keyword per line
