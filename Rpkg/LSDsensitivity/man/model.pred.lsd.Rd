\name{model.pred.lsd}
\alias{model.pred.lsd}
\title{
Predict meta-model response at given point(s)
}
\description{
This function predicts the meta-model response at a specific point(s) in the factor (parameter) space and provides a confidence interval for the prediction(s) at 95\% confidence.
}
\usage{
model.pred.lsd( data.point, model )
}
\arguments{
  \item{data.point}{
a single or multi line data frame which contains values (in the rows) for all the meta-model factors/variables (in the columns).
}
  \item{model}{
an object created by a previous call to \code{\link{kriging.model.lsd}} or \code{\link{polynomial.model.lsd}} which contains the meta-model estimated hyper-parameters.
}
}
\details{
This function simply evaluate the meta-model value at the given point. All factor values must be specified. \code{data.point} can also be specified as an ordered vector or matrix, following the same order for the factors as defined in the meta-model specification.

This function is a wrapper to the functions \code{\link[DiceKriging]{predict.km}} in \code{\link[DiceKriging]{DiceKriging-package}} and \code{\link[stats]{predict.lm}} in \code{\link[stats]{stats-package}}.
}
\value{
The function returns a list containing the prediction(s) and the confidence bounds. If \code{data.point} is a data frame or matrix with more than one line, the list elements are vectors. The list element names are:

\item{mean}{the expected response value.}
\item{lower}{the lower confidence bound.}
\item{upper}{the upper confidence bound.}
}
\author{
\packageAuthor{LSDsensitivity}
}
\seealso{
\code{\link{kriging.model.lsd}()},
\code{\link{polynomial.model.lsd}()}

\code{\link[DiceKriging]{predict.km}} in \code{\link[DiceKriging]{DiceKriging-package}},
\code{\link[stats]{predict.lm}} in \code{\link[stats]{stats-package}}
}
\examples{
# get the example directory name
path <- system.file( "extdata/sobol", package = "LSDsensitivity" )

# Steps to use this function:
# 1. define the variables you want to use in the analysis
# 2. load data from a LSD simulation saved results using read.doe.lsd
# 3. fit a Kriging (or polynomial) meta-model using kriging.model.lsd
# 4. estimate the meta-model response at any set of points applying
#    model.pred.lsd

lsdVars <- c( "var1", "var2", "var3" )         # the definition of existing variables

dataSet <- read.doe.lsd( path,                 # data files folder
                         "Sim3",               # data files base name (same as .lsd file)
                         "var3",               # variable name to perform the sensitivity analysis
                         does = 2,             # number of experiments (data + external validation)
                         saveVars = lsdVars )  # LSD variables to keep in dataset

model <- kriging.model.lsd( dataSet )          # estimate best Kriging meta-model

# creates a set of four random points in parameter space
points <- data.frame( par1 = rnorm( 4 ), par2 = rnorm( 4 ), par3 = rnorm( 4 ) )

response <- model.pred.lsd( points, model )    # predict model response at the 3 points

print( points )
print( response )
}
\keyword{ models }
\keyword{ design }
