\name{name.var.lsd}
\alias{name.var.lsd}
\title{
Get original LSD variable name
}
\description{
This function generates the original LSD variable name, as it was defined in LSD and before R adjusts the name, from a R column name (with or without position or timing information appended).
}
\usage{
name.var.lsd( r.name )
}
\arguments{
  \item{r.name}{
a character vector, or an object which can be coerced to a character vector by as.character, from the column names produced by reading a LSD results file.
}
}
\details{
The conversion may be incorrect if the original LSD variable is named in the format "X_...". No checking is done to make sure the variable really exists.
}
\value{
A character vector of with the same attributes as x (after possible coercion).
}
\author{Marcelo C. Pereira}
\seealso{
\code{\link{name.clean.lsd}()},
\code{\link{info.names.lsd}()}
}
\examples{
name.var.lsd( "label" )

name.var.lsd( c( "label", "X_underlinelabel" ) )
}
\keyword{ attribute }
\keyword{ misc }
