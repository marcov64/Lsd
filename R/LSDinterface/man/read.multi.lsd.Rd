\name{read.multi.lsd}
\alias{read.multi.lsd}
\title{
Read all instances of LSD variables (time series) from a LSD results file 
}
\description{
This function reads the data series associated to all instances of each selected variable from a LSD results file (.res).
}
\usage{
read.multi.lsd(file, col.names = NULL, nrows = -1, skip = 0, check.names = TRUE, stringsAsFactors = default.stringsAsFactors())
}
\arguments{
  \item{file}{
the name of the LSD results file which the data are to be read from. If it does not contain an absolute path, the file name is relative to the current working directory, getwd(). This can be a compressed file (see file) and must include the appropriated extension (usually .res or .res.gz).
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
logical. If TRUE then the names of the variables in the data frame are checked to ensure that they are syntactically valid variable names. If necessary they are adjusted (by make.names) so that they are, and also to ensure that there are no duplicates.
}
  \item{stringsAsFactors}{
logical: should character vectors be converted to factors?
}
}
\value{
Returns a single data frame containing the selected variables' time series contained in the results file.
}
\author{Marcelo C. Pereira}
\note{
The returned data frame may be potentially very wide, in particular if variables are not well selected(see col.names above) or if there is a large number of instances.
For extracting data from multiple similar files (like sensitivity analysis results), see \code{\link{read.list.lsd}}.
}
\seealso{
\code{\link{read.lsd}}, 
\code{\link{read.list.lsd}}, 
\code{\link{read.3d.lsd}}, 
\code{\link{read.raw.lsd}}
}
\examples{
## reads all instances of all variables
#bigTable <- read.multi.lsd("Sim1_1.res")

## read all instances of a set of variables named 'a' and 'b'
#abTable <- read.multi.lsd("~/LSD/Work/Simulation_A/Sim1_2.res.gz", c("a", "b"))

## reads all instances of all variables, skipping the initial 100 time steps
## and keeping up to 200 time steps (from t = 101 up to t = 300)
#allDF100_300 <- read.multi.lsd("Sim1_1.res", skip = 100, nrows = 200)
}
% Add one or more standard keywords, see file 'KEYWORDS' in the
% R documentation directory.
\keyword{ ~kwd1 }
\keyword{ ~kwd2 }% __ONLY ONE__ keyword per line
