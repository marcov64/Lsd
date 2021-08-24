\name{info.dimensions.lsd}
\alias{info.dimensions.lsd}
\title{
Dimension information for a LSD results file
}
\description{
This function reads some dimension information from a LSD results file (.res): number of time steps, number of variables and the original column (variable) names.
}
\usage{
info.dimensions.lsd( file )
}
%- maybe also 'usage' for other objects documented here.
\arguments{
  \item{file}{
the name of the LSD results file which the data are to be read from. If it does not contain an absolute path, the file name is relative to the current working directory, \code{getwd()}. Tilde-expansion is performed where supported. This can be a compressed file (see file) and must include the appropriated extension (usually \code{.res} or \code{.res.gz}).
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
# get the example directory and file names
path <- system.file( "extdata", package = "LSDinterface" )
files <- list.files( path = path, full.names = TRUE,
                     pattern = paste0( "Sim1_[0-9]+.res.gz" ) )

# get dimensions from second file
info.dimensions.lsd( files[ 2 ] )
}
\keyword{ attribute }
\keyword{ file }
