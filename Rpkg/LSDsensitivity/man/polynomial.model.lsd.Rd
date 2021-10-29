\name{polynomial.model.lsd}
\alias{polynomial.model.lsd}
\title{
Fit a polynomial meta-model to a LSD model sample data
}
\description{
This function fits a Polynomial meta-model of first or second order, with or without interactions, to the sampled data from a LSD simulation model. Polynomial meta-models are usually inadequate to fit nonlinear simulation models, please use the estimated meta-model carefully.
}
\usage{
polynomial.model.lsd( data, ext.wgth = 0.5, ols.sig = 0.2,
                      orderModel = 0, interactModel = 0, digits = 4 )
}
\arguments{
  \item{data}{
an object created by a previous call to \code{\link{read.doe.lsd}} which contains all the required experimental data for the analysis.
}
  \item{ext.wgth}{
numeric in [0, 1]: the weight given to the fitting metrics calculated over the out-of-sample (external) validation sample in regard to the in-sample metrics. The default value is 0.5.
}
  \item{ols.sig}{
numeric in [0, 1]: the minimum significance considered in the OLS regression.
}
  \item{orderModel}{
a number corresponding to the polynomial model order: 0 = automatic selection (according to fitting metrics, the default); 1 = first order; 2 = second order.
}
  \item{interactModel}{
a number indicating the presence of interaction terms in the model: 0 = automatic selection (according to fitting metrics, the default); 1 = no , 2 = yes.
}
  \item{digits}{
integer: the number of significant digits to show in results. The default is 4.
}
}
\details{
This function fits a polynomial meta-model to the experimental data set previously loaded with  \code{\link{read.doe.lsd}} using the ordinary least-squares (OLS) method.

This function is a wrapper to the function \code{\link[stats]{lm}} in \code{\link[stats]{stats-package}}.
}
\value{
The function returns an object/list of class \code{polynomial-model} containing several items:

\item{selected}{an object containing the selected estimated meta-model.}
\item{comparison}{a print-ready table with all fitting statistics for all fitted meta-model specifications.}
\item{R2}{the adjusted R2 in-sample fitting statistic for the selected meta-model.}
\item{rmse}{the RMSE out-of-sample fitting statistic for the selected meta-model.}
\item{mae}{the MAE out-of-sample fitting statistic for the selected meta-model.}
\item{rma}{the RMA out-of-sample fitting statistic for the selected meta-model.}
\item{extN}{number of out-of-sample observations.}
\item{estimation}{a print-ready table with the coefficients (hyper-parameters) of the selected estimated meta-model.}
\item{estimation.std}{a print-ready table with the standardized coefficients (hyper-parameters) of the selected estimated meta-model.}
\item{coefficients}{a vector with the coefficients (hyper-parameters) of the selected estimated meta-model.}
\item{coefficients.std}{a vector with the standardized coefficients (hyper-parameters) of the selected estimated meta-model.}
\item{order}{order of the selected polynomial model.}
\item{polyNames}{name of the selected polynomial model.}
\item{interact}{number of the selected interaction mode.}
\item{interactNames}{name of the selected interaction mode.}
}
\author{
\packageAuthor{LSDsensitivity}
}
\note{
See the note in \link[LSDsensitivity]{LSDsensitivity-package} for step-by-step instructions on how to perform the complete sensitivity analysis process using LSD and R.
}
\seealso{
\code{\link{read.doe.lsd}()}
}
\examples{
# get the example directory name
path <- system.file( "extdata/sobol", package = "LSDsensitivity" )

# Steps to use this function:
# 1. define the variables you want to use in the analysis
# 2. load data from a LSD simulation saved results using read.doe.lsd,
#    preferrably using two sets of sampled data (DoEs), one for model
#    estimation and the other for out-of-sample (external) validation
# 3. fit the polynomial meta-model using polynomial.model.lsd

lsdVars <- c( "var1", "var2", "var3" )         # the definition of existing variables

dataSet <- read.doe.lsd( path,                 # data files folder
                         "Sim3",               # data files base name (same as .lsd file)
                         "var3",               # variable name to perform the sensitivity analysis
                         does = 2,             # number of experiments (data + external validation)
                         saveVars = lsdVars )  # LSD variables to keep in dataset

model <- polynomial.model.lsd( dataSet )       # estimate best polynomial meta-model
                                               # using defaults (auto model selection)

print( model$comparison )                      # model comparison table
print( model$estimation.std )                  # model estimation (standardized) table
}
\keyword{ methods }
\keyword{ models }
\keyword{ design }
