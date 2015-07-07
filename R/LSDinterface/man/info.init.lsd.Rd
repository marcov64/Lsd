\name{info.init.lsd}
\alias{info.init.lsd}
\title{
Read initial conditions from a LSD results file
}
\description{
This function reads the initial condition values from a LSD results file (.res).
}
\usage{
info.init.lsd(file)
}
\arguments{
  \item{file}{
the name of the LSD results file which the data are to be read from. If it does not contain an absolute path, the file name is relative to the current working directory, getwd(). Tilde-expansion is performed where supported. This can be a compressed file (see file) and must include the appropriated extension (usually .res or .res.gz).
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
\code{\link{info.details.lsd}},
\code{\link{info.names.lsd}}
\code{\link{info.dimensions.lsd}}
}
\examples{
# Save current work directory and change to the examples directory
currWD <- getwd()
setwd(system.file("extdata", package = "LSDinterface"))

init1 <- info.init.lsd("Sim1_1.res")
#View(init1)

init2 <- info.init.lsd("Sim1_2.res.gz")
#View(init2)

# Restore working directory
setwd(currWD)
}
% Add one or more standard keywords, see file 'KEYWORDS' in the
% R documentation directory.
\keyword{ ~kwd1 }
\keyword{ ~kwd2 }% __ONLY ONE__ keyword per line
