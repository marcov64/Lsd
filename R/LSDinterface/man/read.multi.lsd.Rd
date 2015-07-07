\name{read.multi.lsd}
\alias{read.multi.lsd}
\title{
Read all instances of LSD variables (time series) from a LSD results file
}
\description{
This function reads the data series associated to all instances of each selected variable from a LSD results file (.res).
}
\usage{
read.multi.lsd(file, col.names = NULL, nrows = -1, skip = 0, check.names = TRUE)
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
logical. If TRUE then the names of the variables are checked to ensure that they are syntactically valid variable names. If necessary they are adjusted (by make.names) so that they are, and also to ensure that there are no duplicates.
}
}
\value{
Returns a list of matrices, each containing one of the selected variables' time series from the results file.
}
\author{Marcelo C. Pereira}
\note{
For extracting data from multiple similar files (like sensitivity analysis results), see \code{\link{read.list.lsd}}.
}
\seealso{
\code{\link{read.single.lsd}},
\code{\link{read.list.lsd}},
\code{\link{read.3d.lsd}},
\code{\link{read.4d.lsd}},
\code{\link{read.raw.lsd}}
}
\examples{
# Save current work directory and change to the examples directory
currWD <- getwd()
setwd(system.file("extdata", package = "LSDinterface"))

# Load a sample .res file into a simple matrix (all instances)
macroList <- read.multi.lsd("Sim1_1.res")

# See matrix in the data viewer (require package 'utils')
length(macroList)
#View(macroList[[1]])
#View(macroList[[8]])

# reads first instance of 2 variables, skipping the initial 20 time steps
# and keeping up to 50 time steps (from t = 21 up to t = 70)
varsList21_70 <- read.multi.lsd("Sim1_1.res", c("_A1p", "_growth1"), skip = 20, nrows = 50)
#View(varsList21_70[[1]])
#View(varsList21_70[[2]])

# Restore working directory
setwd(currWD)
}
% Add one or more standard keywords, see file 'KEYWORDS' in the
% R documentation directory.
\keyword{ ~kwd1 }
\keyword{ ~kwd2 }% __ONLY ONE__ keyword per line
