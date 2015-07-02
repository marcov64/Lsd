\name{clean.name.lsd}
\alias{clean.name.lsd}
\title{Get clean (R) variable name}
\description{This function produces a more appropriate variable name from R initial column name conversion.}
\usage{
clean.name.lsd(r.name)
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
A character vector of with the same attributes as x (after possible coercion) and the format "NAME.POSITION.INI_END".
}
\author{Marcelo C. Pereira}
\seealso{
\code{\link{var.name.lsd}}, 
\code{\link{read.names.lsd}}
}
\examples{
clean.name.lsd("Var1.1_1..1.100.")
clean.name.lsd(c("Var1.1_1..1.100.", "Var2.1_2_3..50.70."))
}
% Add one or more standard keywords, see file 'KEYWORDS' in the
% R documentation directory.
\keyword{ ~kwd1 }
\keyword{ ~kwd2 }% __ONLY ONE__ keyword per line
