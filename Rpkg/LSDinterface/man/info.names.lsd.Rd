\name{info.names.lsd}
\alias{info.names.lsd}
\title{
Read unique variable names from a LSD results file (no duplicates)
}
\description{
This function reads the variable names (columns) from a LSD results file (.res). The names returned are converted to the original LSD names whenever possible and duplicates are removed.
}
\usage{
info.names.lsd( file )
}
\arguments{
  \item{file}{
the name of the LSD results file which the data are to be read from. If it does not contain an absolute path, the file name is relative to the current working directory, \code{\link[base]{getwd}()}. Tilde-expansion is performed where supported. This can be a compressed file (see file) and must include the appropriated extension (usually \code{.res} or \code{.res.gz}).
}
}
\value{
Returns a character vector containing the names of all unique variables contained in the selected results file.
}
\author{Marcelo C. Pereira}
\note{
Not all names can be automatically reconverted to the original LSD names. The conversion may be incorrect if the original LSD variable is named in the format "X_...".
}
\seealso{
\code{\link{list.files.lsd}()}
\code{\link{info.details.lsd}()},
\code{\link{info.init.lsd}()}
\code{\link{info.dimensions.lsd}()}
}
\examples{
# get the list of file names of example LSD results
files <- list.files.lsd( system.file( "extdata", package = "LSDinterface" ) )

# get variable names from first file
info.names.lsd( files[ 1 ] )
}
\keyword{ attribute }
\keyword{ file }
