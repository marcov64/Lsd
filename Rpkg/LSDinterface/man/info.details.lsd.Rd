\name{info.details.lsd}
\alias{info.details.lsd}
\title{
Get detailed information from a LSD results file
}
\description{
This function reads, analyze and organize the information from a LSD results file (.res).
}
\usage{
info.details.lsd( file )
}
\arguments{
  \item{file}{
the name of the LSD results file which the data are to be read from. If it does not contain an absolute path, the file name is relative to the current working directory, \code{getwd()}. Tilde-expansion is performed where supported. This can be a compressed file (see file) and must include the appropriated extension (usually \code{.res} or \code{.res.gz}).
}
}
\value{
Returns a data frame containing detailed description (columns) of all variables (rows) contained in the selected results file.
}
\author{Marcelo C. Pereira}
\seealso{
\code{\link{info.init.lsd}},
\code{\link{info.names.lsd}}
\code{\link{info.dimensions.lsd}}
}
\examples{
# get the example directory and file names
path <- system.file( "extdata", package = "LSDinterface" )
files <- list.files( path = path, full.names = TRUE,
                     pattern = paste0( "Sim1_[0-9]+.res.gz" ) )

# get details about all variables in first file
info.details.lsd( files[ 1 ] )
}
\keyword{ attribute }
\keyword{ file }


