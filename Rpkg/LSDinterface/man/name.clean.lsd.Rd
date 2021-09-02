\name{name.clean.lsd}
\alias{name.clean.lsd}
\title{Get clean (R) variable name}
\description{This function produces a more appropriate variable name from R initial column name conversion.}
\usage{
name.clean.lsd( r.name )
}
\arguments{
  \item{r.name}{
a character vector, or an object which can be coerced to a character vector by as.character, from the column names produced by reading a LSD results file.
}
}
\details{
The function removes the extra/ending '.' characters introduced by R and introduces a '_' between time span values.
}
\value{
A character vector of with the same attributes as x (after possible coercion) and the format \code{NAME.POSITION.INI_END}.
}
\author{Marcelo C. Pereira}
\seealso{
\code{\link{name.var.lsd}()},
\code{\link{name.nice.lsd}()},
\code{\link{info.names.lsd}()}
}
\examples{
name.clean.lsd( "Var1.1_1..1.100." )

name.clean.lsd( c( "Var1.1_1..1.100.", "Var2.1_2_3..50.70." ) )
}
\keyword{ attribute }
\keyword{ misc }
