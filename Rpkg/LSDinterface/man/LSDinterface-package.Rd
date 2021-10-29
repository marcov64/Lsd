\name{LSDinterface-package}
\alias{LSDinterface}
\docType{package}
\title{
\packageTitle{LSDinterface}
}
\description{
\packageDescription{LSDinterface}
}
\details{
There are specific \code{read.xxx.lsd()} functions for different types of LSD data structures.

\code{\link{read.raw.lsd}()} simply import LSD saved data in tabular (data frame) format (variables in columns and time steps in rows). \code{\link{read.single.lsd}()} is appropriate to simple LSD data structures where each saved variable is single-instanced (inside an object with a single copy). \code{\link{read.multi.lsd}()} reads all instances of all variables from the LSD results file, renaming multi-instanced variables. \code{\link{read.list.lsd}()} is similar to \code{\link{read.multi.lsd}()} but saves  multiple-instanced variables as R lists, preventing renaming.

\code{\link{read.3d.lsd}()} and \code{\link{read.4d.lsd}()} are specialized versions for extracting data from multiple LSD results files simultaneously. The files must have the same structure (selected variables and number of time steps). They are frequently used to acquire data from Monte Carlo experiments or sensitivity analysis. \code{\link{read.3d.lsd}()} operates like \code{\link{read.single.lsd}()} but add each additional results file into a separate dimension of the produced 3-dimensional array (variable x time step x file). \code{\link{read.4d.lsd}()} adds the ability to read each instance of a multi-instanced variable to the fourth dimension of the generated 4D array (variable x instance x time step x file).

\code{\link{list.files.lsd}()} is a helper function to simplify the collection of results files to be used by the other functions in this package. It can be directly used to supply the \code{files} argument in the \code{read.xxx.lsd()} family of functions.

\code{\link{select.colattrs.lsd}()} and \code{\link{select.colnames.lsd}()} provide methods to extract/summarize information from previously imported LSD data structures.

\code{info.xxx.lsd}() functions provide information about LSD data structures. \code{name.xxx.lsd}() functions offer tools for dealing with LSD variable names in R.

For a complete list of exported functions, use \code{library( help = "LSDinterface" )}.
}
\author{
\packageAuthor{LSDinterface}

Maintainer: \packageMaintainer{LSDinterface}
}
\references{
LSD documentation is available at \url{https://www.labsimdev.org/}.

The latest LSD binaries and source code can be downloaded at \url{https://github.com/marcov64/Lsd/}.
}
\keyword{ package }
\keyword{ interface }
\keyword{ file }
\keyword{ datasets }
\keyword{ database }
