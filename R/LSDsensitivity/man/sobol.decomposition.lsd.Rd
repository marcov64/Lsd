\name{sobol.decomposition.lsd}
\alias{sobol.decomposition.lsd}
\title{
Sobol variance decomposition sensitivity analysis
}
\description{
This function performs the global sensitivity analysis of a previously fitted meta-model using the Sobol variance decomposition method (Saltelli et al., 2008).
}
\usage{
sobol.decomposition.lsd(data, model, krig.sa = FALSE, sa.samp = 1000)
}
\arguments{
  \item{data}{
an object created by a previous call to \code{\link{read.doe.lsd}} which contains all the required experimental data for the analysis.
}
  \item{model}{
an object created by a previous call to \code{\link{kriging.model.lsd}} or \code{\link{polynomial.model.lsd}} which contains the meta-model estimated hyper-parameters.
}
  \item{krig.sa}{
logical: use alternative Kriging-specific algorithm if TRUE (see \code{\link[sensitivity]{sobolGP}}). Default is FALSE. Applicable only to Kriging meta-models.
}
  \item{sa.samp}{
integer: number of samples to use in sensitivity analysis. The default is 1000.
}
}
\details{
This function performs the global sensitivity analysis on a meta-model, previously estimated with  \code{\link{kriging.model.lsd}} or \code{\link{polynomial.model.lsd}}, using the Sobol variance decomposition method (Saltelli et al., 2008).

This function is a wrapper to the functions \code{\link[sensitivity]{fast99}} and \code{\link[sensitivity]{sobolGP}} in \code{\link[sensitivity]{sensitivity-package}}.
}
\value{
The function returns an object/list of class \code{kriging-sa} or \code{polynomial-sa}, according to the input meta-model, containing several items:

\item{metamodel}{an object/list of class \code{\link[sensitivity]{fast99}} containing the estimated total and conditional variances for all the meta-model factors.}
\item{sa}{a print-ready table with the Sobol indexes for each factor.}
\item{topEffect}{a vector containing the indexes to the three most influential factors, automatically calculated (if \code{factorX = 0}) or according to the order pre selected by the user.}
}
\references{
Saltelli A, Ratto M, Andres T, Campolongo F, Cariboni J, Gatelli D, Saisana M, Tarantola S (2008) \emph{Global sensitivity analysis: the primer}. Wiley, New York
}
\author{
\packageAuthor{LSDsensitivity}
}
\note{
See the note in \link[LSDsensitivity]{LSDsensitivity-package} for step-by-step instructions on how to perform the complete sensitivity analysis process using LSD and R.
}
\seealso{
\code{\link{read.doe.lsd}},
\code{\link{kriging.model.lsd}},
\code{\link{polynomial.model.lsd}}

\code{\link[sensitivity]{fast99}},
\code{\link[sensitivity]{sobolGP}} in \code{\link[sensitivity]{sensitivity-package}}
}
\examples{
# Examples require the data files produced by LSD, please check the package
# notes and LSD documentation on how to generate you simulation results files.
# Please note that the full set of sensitivity analysis files must be available,
# as detailed in the help page for the read.doe.lsd function.

# Steps to use this function:
# 1. define the variables you want to use in the analysis
# 2. load data from a LSD simulation saved results using read.doe.lsd
# 3. fit a Kriging (or polynomial) meta-model using kriging.model.lsd
# 4. perform the sensitivity analysis applying sobol.decomposition.lsd

\donttest{
lsdVars <- c( "var1", "var2", "var3" )         # the definition of existing variables

dataSet <- read.doe.lsd( ".",                  # data files relative folder
                         "Sim1",               # data files base name (same as .lsd file)
                         "var1",               # variable name to perform the sensitivity analysis
                         does = 2,             # number of experiments (data + external validation)
                         saveVars = lsdVars )  # LSD variables to keep in dataset

model <- kriging.model.lsd( dataSet )          # estimate best Kriging meta-model

SA <- sobol.decomposition.lsd( dataSet,        # LSD experimental data set
                               model )         # estimated meta-model

print( SA$topEffect )                          # indexes to the top 3 factors
print( SA$sa )                                 # Sobol indexes table
barplot( t( sSA$sa ) )                         # plot Sobol indexes chart
}
}
\keyword{ methods }
\keyword{ models }
\keyword{ design }
