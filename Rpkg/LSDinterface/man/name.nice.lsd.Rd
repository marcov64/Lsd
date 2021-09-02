\name{name.nice.lsd}
\alias{name.nice.lsd}
\title{Get a nice (R) variable name}
\description{This function produces a nicer variable name from R initial column name conversion, in particular removing leading underscores.}
\usage{
name.nice.lsd( r.name )
}
\arguments{
  \item{r.name}{
a character vector, or an object which can be coerced to a character vector by as.character, from the column names produced by reading a LSD results file.
}
}
\details{
The function removes the extra/ending '.' characters introduced by R and introduces a '_' between time span values and deletes leading underscores ('_'), converted to 'X_' by R.
}
\value{
A character vector of with the same attributes as x (after possible coercion) and the format \code{NAME[.POSITION.INI_END]}.
}
\author{Marcelo C. Pereira}
\seealso{
\code{\link{name.var.lsd}()},
\code{\link{name.clean.lsd}()},
\code{\link{info.names.lsd}()}
}
\examples{
name.nice.lsd( "X_Var1.1_1..1.100." )

name.nice.lsd( c( "_Var1.1_1..1.100.", "X_Var2.1_2_3..50.70." ) )

name.nice.lsd( c( "_Var1", "X_Var2" ) )
}
\keyword{ attribute }
\keyword{ misc }
