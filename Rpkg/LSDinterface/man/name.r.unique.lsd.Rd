\name{name.r.unique.lsd}
\alias{name.r.unique.lsd}
\title{Get valid unique R variable name}
\description{This function produces a valid and unique variable name from names produced from multi-instanced LSD variables (as in \code{read.raw.lsd}).}
\usage{
name.r.unique.lsd( r.name )
}
\arguments{
  \item{r.name}{
a string, a vector of strings, or an object which can be coerced to a character vector by \code{as.character}, from the column names produced by reading a LSD results file.
}
}
\details{
The function removes the trailing '.' characters, and the text between, introduced during the conversion from LSD results files, add an 'X' prefix to names started by an '_'. After this initial transformation, all repeated variable names (originated from multi-instanced variables) are removed.

The produced names are R valid variable names, similar to the original LSD/C++ variable names, but with an 'X' prepended to variables starting with an '_' (which are invalid in R).
}
\value{
A string or a string vector of converted string(s) including only non-repeated ones.
}
\author{Marcelo C. Pereira}
\seealso{
\code{\link{name.var.lsd}()},
\code{\link{name.clean.lsd}()},
\code{\link{name.nice.lsd}()},
\code{\link{info.names.lsd}()}
}
\examples{
name.r.unique.lsd( "Var1.1_1.1_100" )

name.r.unique.lsd( c( "Var1.1_1.1_100", "_Var2.1_1.1_100", "_Var2.1_2.50_70" ) )
}
\keyword{ attribute }
\keyword{ misc }
