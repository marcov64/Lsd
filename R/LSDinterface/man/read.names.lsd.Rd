\name{read.names.lsd}
\alias{read.names.lsd}
\title{
Read unique variable names from a LSD results file (no duplicates)
}
\description{
This function reads the variable names (columns) from a LSD results file (.res). The names returned are converted to the original LSD names whenever possible and duplicates are removed.
}
\usage{
read.names.lsd(file, stringsAsFactors = default.stringsAsFactors())
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
Returns a character vector containing the names of all unique variables contained in the selected results file.
}
\author{Marcelo C. Pereira}
\note{
Not all names can be automatically reconverted to the original LSD names. The conversion may be incorrect if the original LSD variable is named in the format "X_...".
}
\seealso{
\code{\link{read.info.lsd}}, 
\code{\link{read.init.lsd}}
}
\examples{
#read.names.lsd("Sim1_1.res")
#read.names.lsd("~/LSD/Work/Simulation_A/Sim1_2.res.gz", stringsAsFactors = FALSE)
}
% Add one or more standard keywords, see file 'KEYWORDS' in the
% R documentation directory.
\keyword{ ~kwd1 }
\keyword{ ~kwd2 }% __ONLY ONE__ keyword per line
