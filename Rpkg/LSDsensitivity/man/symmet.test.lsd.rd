\name{symmet.test.lsd}
\alias{symmet.test.lsd}
\title{
Unimodality and symmetry tests
}
\description{
Perform a set of unimodality and symmetry tests useful for simulation model data from a Monte Carlo experiment distributions. The included tests are: Hartigans dip test for unimodality (Hdip), and the Cabilio and Masaro (CM), the Mira (M), and the Miao, Gel and Gastwirth tests for symmetry.
}
\usage{
symmet.test.lsd( data, vars = names( data[ 1, , 1 ] ), start.period = 0,
                 signif = 0.05, digits = 2, sym.boot = FALSE )
}
\arguments{
  \item{data}{
a three-dimensional array, as the ones produced by \code{\link[LSDinterface]{read.3d.lsd}}, organized as (time steps x variables x Monte Carlo instances).
}
  \item{vars}{
a vector of the variable names (as strings) contained in \code{data} for which the tests will be performed. The default is to test all variables.
}
  \item{start.period}{
integer: the first time step in \code{data} to be considered for the tests. The default value is 0 (all time steps considered).
}
  \item{signif}{
numeric in [0, 1]: statistical significance to evaluate the tests rejection of the null-hypothesis. The default value is 0.05 (5\%).
}
  \item{digits}{
integer: the number of significant digits to show in results. The default is 2.
}
  \item{sym.boot}{
logical: set to \code{TRUE} to use bootstrap to obtain critical values. The default (\code{FALSE}) is to use asymptotic distribution of the statistics.
}
}
\details{
This function is a wrapper to the functions \code{\link[diptest]{dip.test}} in \code{diptest} package, and \code{\link[lawstat]{symmetry.test}} in \code{lawstat} package.
}
\value{
The function returns a data frame presenting both the average test statistics and the frequency of test null-hypothesis rejections for all the variables selected in \code{vars}.
Null hypothesis (H0) for Hdip test is an unimodal distribution for the Monte Carlo distribution. Null hypothesis (H0) for CM, M and MGG tests is a symmetric distribution for the Monte Carlo distribution.
}
\author{
\packageAuthor{LSDsensitivity}
}
\seealso{
\code{\link{ergod.test.lsd}()},

\code{\link[LSDinterface]{list.files.lsd}()}, \code{\link[LSDinterface]{read.3d.lsd}()} in \code{\link[LSDinterface]{LSDinterface-package}},

\code{\link[diptest]{dip.test}()}, \code{\link[lawstat]{symmetry.test}()}
}
\examples{
# get the list of file names of example LSD results
library( LSDinterface )
files <- list.files.lsd( system.file( "extdata", package = "LSDsensitivity" ),
                         "Sim1.lsd", recursive = TRUE )

# Steps to use this function:
# 1. load data from a LSD simulation saved results using a read.xxx.lsd
#    function from LSDinterface package (read.3d.lsd, for instance)
# 2. use symmet.test.lsd to apply the tests on the relevant variables,
#    replacing "var1", "var2" etc. with your data

# read data from Monte Carlo runs
dataSet <- read.3d.lsd( files )

# apply tests
tests <- symmet.test.lsd( dataSet,              # the data set to use
                          c( "var2", "var3" ),  # the variables to test
                          signif = 0.01,        # use 1\% significance
                          digits = 4,           # show results using 4 digits
                          sym.boot = FALSE )    # use bootstrap for precision
print( tests )
}
\keyword{ models }
\keyword{ htest }
