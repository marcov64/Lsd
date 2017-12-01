\name{info.dimensions.lsd}
\alias{info.dimensions.lsd}
\title{
Dimension information for a LSD results file
}
\description{
This function reads some dimension information from a LSD results file (.res): number of time steps, number of variables and the original column (variable) names.
}
\usage{
info.dimensions.lsd(file)
}
%- maybe also 'usage' for other objects documented here.
\arguments{
  \item{file}{
the name of the LSD results file which the data are to be read from. If it does not contain an absolute path, the file name is relative to the current working directory, getwd(). Tilde-expansion is performed where supported. This can be a compressed file (see file) and must include the appropriated extension (usually .res or .res.gz).
}
}
\details{
The returned number of time steps does not include the initial value (t = 0) for lagged variables (the second line of a .res format file).
}
\value{
Returns a list containing two integer values and a character vector describing the selected results file.
\item{tSteps}{Number of time steps in file}
\item{nVars}{Number of variables (including duplicated instances) in file}
\item{varNames}{Names of variables (including duplicated instances) in file, after R name conversion}
}
\author{Marcelo C. Pereira}
\seealso{
\code{\link{info.details.lsd}},
\code{\link{info.names.lsd}},
\code{\link{info.init.lsd}}
}
\examples{
# Save current work directory and change to the examples directory
currWD <- getwd( )
setwd( system.file( "extdata", package = "LSDinterface" ) )

info.dimensions.lsd( "Sim1_1.res" )
info.dimensions.lsd( "Sim1_2.res.gz" )

# Restore working directory
setwd( currWD )
}
\keyword{ attribute }
\keyword{ file }
