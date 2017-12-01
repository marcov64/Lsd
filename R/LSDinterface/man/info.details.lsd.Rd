\name{info.details.lsd}
\alias{info.details.lsd}
\title{
Get detailed information from a LSD results file
}
\description{
This function reads, analyze and organize the information from a LSD results file (.res).
}
\usage{
info.details.lsd(file)
}
\arguments{
  \item{file}{
the name of the LSD results file which the data are to be read from. If it does not contain an absolute path, the file name is relative to the current working directory, getwd(). Tilde-expansion is performed where supported. This can be a compressed file (see file) and must include the appropriated extension (usually .res or .res.gz).
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
# Save current work directory and change to the examples directory
currWD <- getwd( )
setwd( system.file( "extdata", package = "LSDinterface" ) )

info1 <- info.details.lsd( "Sim1_1.res" )
\donttest{View( info1 )}

info2 <- info.details.lsd( "Sim1_2.res.gz" )
\donttest{View( info2 )}

# Restore working directory
setwd( currWD )
}
\keyword{ attribute }
\keyword{ file }


