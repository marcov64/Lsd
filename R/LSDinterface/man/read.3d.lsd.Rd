\name{read.3d.lsd}
\alias{read.3d.lsd}
\title{
Read one instance of LSD variables (time series) from a LSD results file into a 3D array
}
\description{
This function reads the data series associated to a specific instance of each selected variable from a LSD results file (.res) and saves them into a 3-dimensional array (time step x variable x file).
}
\usage{
read.3d.lsd(files, col.names = NULL, nrows = -1, skip = 0, check.names = TRUE, instance = 1, stringsAsFactors = default.stringsAsFactors())
}
\arguments{
  \item{files}{
a character vector containing the names of the LSD results files which the data are to be read from. If they do not contain an absolute path, the file names are relative to the current working directory, getwd(). These can be compressed files and must include the appropriated extension (usually .res or .res.gz).
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
  \item{instance}{
integer: the instance of the variable to be read, for variables that exist in more than one object. This number is based on the position (column) of the variable in the results file. The default (1) is to read first instances.
}
  \item{stringsAsFactors}{
logical: should character vectors be converted to factors?
}
}
\value{
Returns a 3D array containing data series from the selected variables.
}
\author{Marcelo C. Pereira}
\note{
If the selected files don't have the same columns available (names and instances), an error is produced.
}
\seealso{
\code{\link{read.lsd}}, 
\code{\link{read.multi.lsd}}, 
\code{\link{read.list.lsd}}, 
\code{\link{read.raw.lsd}}
}
\examples{
## reads first instance of all variables from three files (one data frame each)
#inst1Array <- read.3d.lsd(c("Sim1_1.res", "Sim1_2.res", "Sim1_3.res"))

## read first instance of a set of variables named 'a' and 'b'
#ab1Array <- read.3d.lsd(c("~/LSD/Work/Simulation_A/Sim1_1.res.gz", "~/LSD/Work/Simulation_A/Sim1_2.res.gz"), c("a", "b"))

## reads instance 2 of all variables, skipping the initial 100 time steps
## and keeping up to 200 time steps (from t = 101 up to t = 300)
#inst2Array100_300 <- read.3d.lsd(c("Sim1_1.res", "Sim1_2.res"), skip = 100, nrows = 200, instance = 2)
}
% Add one or more standard keywords, see file 'KEYWORDS' in the
% R documentation directory.
\keyword{ ~kwd1 }
\keyword{ ~kwd2 }% __ONLY ONE__ keyword per line
