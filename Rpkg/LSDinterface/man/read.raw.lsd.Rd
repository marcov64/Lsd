\name{read.raw.lsd}
\alias{read.raw.lsd}
\title{
Read LSD results file and clean variables names
}
\description{
This function reads all the data series in a LSD results file (.res).
}
\usage{
read.raw.lsd(file, nrows = -1, skip = 0)
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
}
\value{
Returns a single matrix containing all variables' time series contained in the results file.
}
\author{Marcelo C. Pereira}
\note{
The returned matrix may be potentially very wide.
For extracting data in a more selective way, see \code{\link{read.single.lsd}} and \code{\link{read.multi.lsd}}. To use multiple results files simultaneously, see \code{\link{read.list.lsd}} and \code{\link{read.3d.lsd}}.
Variable names are never "cleaned", even for single instanced variables.
}
\seealso{
\code{\link{read.single.lsd}},
\code{\link{read.multi.lsd}},
\code{\link{read.list.lsd}},
\code{\link{read.3d.lsd}},
\code{\link{read.4d.lsd}}
}
\examples{
# Get the examples directory
path <- system.file( "extdata", package = "LSDinterface" )

# reads all instances of all variables,
# pasting the directory where the example files are (not required if in working dir)
bigTable <- read.raw.lsd( paste0( path, "/", "Sim1_1.res" ) )

# See matrix in the data viewer (require package 'utils')
\donttest{View( bigTable )}

# reads all instances of all variables, skipping the initial 20 time steps
# and keeping up to 50 time steps (from t = 21 up to t = 70)
all21_70 <- read.raw.lsd( paste0( path, "/", "Sim1_2.res.gz" ),
                          skip = 20, nrows = 50 )
\donttest{View( all21_70 )}
}
\keyword{ interface }
\keyword{ file }
\keyword{ datasets }
\keyword{ database }
