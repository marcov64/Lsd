\name{kriging.model.lsd}
\alias{kriging.model.lsd}
\title{
Fit a Kriging meta-model to a LSD model sample data
}
\description{
This function fits a Kriging meta-model (also known as a Gaussian process), using five alternative variance kernels and two trend model options, to the sampled data from a LSD simulation model.
}
\usage{
kriging.model.lsd( data, ext.wgth = 0.5, trendModel = 0, covModel = 0, digits = 4 )
}
\arguments{
  \item{data}{
an object created by a previous call to \code{\link{read.doe.lsd}} which contains all the required experimental data for the analysis.
}
  \item{ext.wgth}{
numeric in [0, 1]: the weight given to the fitting metrics calculated over the out-of-sample (external) validation sample in regard to the in-sample metrics. The default value is 0.5.
}
  \item{trendModel}{
a number corresponding to the trend model: 0 = automatic selection (according to fitting metrics, the default); 1 = constant; 2 = first order polynomial.
}
  \item{covModel}{
a number corresponding to the covariance model (or kernel): 0 = automatic selection (according to fitting metrics, the default); 1 = Matern 5/2; 2 = Matern 3/2; 3 = Gaussian; 4 = exponential; 5 = power exponential.
}
  \item{digits}{
integer: the number of significant digits to show in results. The default is 4.
}
}
\details{
This function fits a universal Kriging meta-model to the experimental data set previously loaded with  \code{\link{read.doe.lsd}} using the Gaussian process method (Rasmussen & Williams, 2006).

This function is a wrapper to the function \code{\link[DiceKriging]{km}} in \code{\link[DiceKriging]{DiceKriging-package}}.
}
\value{
The function returns an object/list of class \code{kriging-model} containing several items:

\item{selected}{an object containing the selected estimated meta-model (standardized).}
\item{comparison}{a print-ready table with all fitting statistics for all fitted meta-model specifications.}
\item{Q2}{the Q2 in-sample fitting statistic for the selected meta-model.}
\item{rmse}{the RMSE out-of-sample fitting statistic for the selected meta-model.}
\item{mae}{the MAE out-of-sample fitting statistic for the selected meta-model.}
\item{rma}{the RMA out-of-sample fitting statistic for the selected meta-model.}
\item{extN}{number of out-of-sample observations.}
\item{estimation}{a print-ready table with the coefficients (hyper-parameters) of the selected estimated meta-model.}
\item{estimation.std}{a print-ready table with the standardized coefficients (hyper-parameters) of the selected estimated meta-model.}
\item{coefficients}{a vector with the coefficients (hyper-parameters) of the selected estimated meta-model.}
\item{coefficients.std}{a vector with the standardized coefficients (hyper-parameters) of the selected estimated meta-model.}
\item{trend}{number of the selected trend model.}
\item{trendNames}{name of the selected trend model.}
\item{cov}{number of the selected covariance model (kernel).}
\item{covNames}{name of the selected covariance model (kernel).}
}
\references{
Kleijnen JP (2009) \emph{Kriging metamodeling in simulation: a review}. Eur J Oper Res 192(3):707-716

Rasmussen C, Williams C (2006) \emph{Gaussian processes for machine learning}. MIT Press, Cambridge

Roustant O, Ginsbourger D, Deville Y (2012) \emph{Dicekriging, diceoptim: two R packages for the analysis of computer experiments by kriging-based metamodeling and optimization}. J Stat Softw 51(1):1-55
}
\author{
\packageAuthor{LSDsensitivity}
}
\note{
See the note in \link[LSDsensitivity]{LSDsensitivity-package} for step-by-step instructions on how to perform the complete sensitivity analysis process using LSD and R.
}
\seealso{
\code{\link{read.doe.lsd}()}

\code{\link[DiceKriging]{km}} in \code{\link[DiceKriging]{DiceKriging-package}}
}
\examples{
# get the example directory name
path <- system.file( "extdata/sobol", package = "LSDsensitivity" )

# Steps to use this function:
# 1. define the variables you want to use in the analysis
# 2. load data from a LSD simulation saved results using read.doe.lsd,
#    preferrably using two sets of sampled data (DoEs), one for model
#    estimation and the other for out-of-sample (external) validation
# 3. fit a Kriging (or polynomial) meta-model using kriging.model.lsd

lsdVars <- c( "var1", "var2", "var3" )         # the definition of existing variables

dataSet <- read.doe.lsd( path,                 # data files folder
                         "Sim3",               # data files base name (same as .lsd file)
                         "var3",               # variable name to perform the sensitivity analysis
                         does = 2,             # number of experiments (data + external validation)
                         saveVars = lsdVars )  # LSD variables to keep in dataset

model <- kriging.model.lsd( dataSet )          # estimate best Kriging meta-model

print( model$comparison )                      # model comparison table
print( model$estimation.std )                  # model estimation (standardized) table
}
\keyword{ methods }
\keyword{ models }
\keyword{ design }
