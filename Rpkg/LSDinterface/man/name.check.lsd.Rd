\name{name.check.lsd}
\alias{name.check.lsd}
\title{
Check a set of LSD variables names against a LSD results file
}
\description{
This function checks if all variable names in a set are valid for a LSD results file (.res). If no name is provided, the function returns all the valid unique variable names in the file.
}
\usage{
name.check.lsd( file, col.names = NULL, check.names = TRUE )
}
\arguments{
  \item{file}{
the name of the LSD results file which the data are to be read from. If it does not contain an absolute path, the file name is relative to the current working directory, \code{\link[base]{getwd}()}. This can be a compressed file (see file) and must include the appropriated extension (usually \code{.res} or \code{.res.gz}).
}
  \item{col.names}{
a vector of optional names for the variables. The default is to read all (unique) variables.
}
  \item{check.names}{
logical. If TRUE then the names of the variables are checked to ensure that they are syntactically valid variable names. If necessary they are adjusted (by make.names) so that they are, and also to ensure that there are no duplicates.
}
}
\value{
Returns a character vector containing the valid variable names contained in the results file.
}
\author{Marcelo C. Pereira}
\seealso{
\code{\link{list.files.lsd}()}
\code{\link{info.names.lsd}()},
}
\examples{
# get the list of file names of example LSD results
files <- list.files.lsd( system.file( "extdata", package = "LSDinterface" ) )

# check all variable names
name.check.lsd( files[ 1 ] )

# check just two names
name.check.lsd( files[ 2 ], col.names = c( "GDP", "_growth1" ) )
}
\keyword{ attribute }
\keyword{ misc }
