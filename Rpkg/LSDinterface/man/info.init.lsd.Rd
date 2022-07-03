\name{info.init.lsd}
\alias{info.init.lsd}
\title{
Read initial conditions from a LSD results file
}
\description{
This function reads the initial condition values from a LSD results file (.res).
}
\usage{
info.init.lsd( file )
}
\arguments{
  \item{file}{
the name of the LSD results file which the data are to be read from. If it does not contain an absolute path, the file name is relative to the current working directory, \code{\link[base]{getwd}()}. Tilde-expansion is performed where supported. This can be a compressed file (see file) and must include the appropriated extension (usually \code{.res} or \code{.res.gz}).
}
}
\value{
Returns a 1 line matrix containing the initial conditions (row 1) of all variables contained in the selected results file.
}
\author{Marcelo C. Pereira}
\note{
The returned matrix contains all variables in the results file, even the ones that don't have an initial condition (indicated as NA).
Only variables automatically initialized automatically by LSD in t = 1 are included here.
}
\seealso{
\code{\link{list.files.lsd}()}
\code{\link{info.details.lsd}()},
\code{\link{info.names.lsd}()}
\code{\link{info.dimensions.lsd}()}
}
\examples{
# get the list of file names of example LSD results
files <- list.files.lsd( system.file( "extdata", package = "LSDinterface" ) )

# get initialization data from first and second files
init1 <- info.init.lsd( files[ 1 ] )
init1[ , 4 : 8 ]

init2 <- info.init.lsd( files[ 2 ] )
init2[ , 4 : 8 ]
}
\keyword{ attribute }
\keyword{ file }
