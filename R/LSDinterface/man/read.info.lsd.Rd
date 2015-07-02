\name{read.info.lsd}
\alias{read.info.lsd}
\title{
Read information from a LSD results file
}
\description{
This function reads, analyze and organize the information from a LSD results file (.res).
}
\usage{
read.info.lsd(file, stringsAsFactors = default.stringsAsFactors())
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
Returns a data frame containing detailed description (columns) of all variables (rows) contained in the selected results file.
}
\author{Marcelo C. Pereira}
\seealso{
\code{\link{read.init.lsd}}, 
\code{\link{read.names.lsd}}
}
\examples{
#read.info.lsd("Sim1_1.res")
#read.info.lsd("~/LSD/Work/Simulation_A/Sim1_2.res.gz", stringsAsFactors = FALSE)
}
% Add one or more standard keywords, see file 'KEYWORDS' in the
% R documentation directory.
\keyword{ ~kwd1 }
\keyword{ ~kwd2 }% __ONLY ONE__ keyword per line
