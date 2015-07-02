\name{read.init.lsd}
\alias{read.init.lsd}
\title{
Read initial conditions from a LSD results file
}
\description{
This function reads the initial condition values from a LSD results file (.res).
}
\usage{
read.init.lsd(file, stringsAsFactors = default.stringsAsFactors())
}
\arguments{
  \item{file}{
the name of the LSD results file which the data are to be read from. If it does not contain an absolute path, the file name is relative to the current working directory, getwd(). Tilde-expansion is performed where supported. This can be a compressed file (see file) and must include the appropriated extension (usually .res or .res.gz).
}
  \item{stringsAsFactors}{
logical: should character vectors be converted to factors?
}
}
\value{
Returns a data frame containing the initial conditions (row 1) of all variables contained in the selected results file.
}
\author{Marcelo C. Pereira}
\note{
The returned data frame contains all variables in the results file, even the ones that don't have an initial condition (indicated as NA).
Only variables automatically initialized automatically by LSD in t = 1 are included here.
}
\seealso{
\code{\link{read.info.lsd}}, 
\code{\link{read.names.lsd}}
}
\examples{
#read.init.lsd("Sim1_1.res")
#read.init.lsd("~/LSD/Work/Simulation_A/Sim1_2.res.gz", stringsAsFactors = FALSE)
}
% Add one or more standard keywords, see file 'KEYWORDS' in the
% R documentation directory.
\keyword{ ~kwd1 }
\keyword{ ~kwd2 }% __ONLY ONE__ keyword per line
