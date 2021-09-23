\name{model.optim.lsd}
\alias{model.optim.lsd}
\title{
Find optimal meta-model factor settings
}
\description{
This function finds the optimal factor (parameter) settings using the estimated meta-model.
}
\usage{
model.optim.lsd( model, data, lower.domain, upper.domain, starting.values = NULL,
                 minimize = TRUE, pop.size = 1000, max.generations = 30,
                 wait.generations = 10, precision = 1e-05, nnodes = 1 )
}
\arguments{
  \item{model}{
an object created by a previous call to \code{\link{kriging.model.lsd}} or \code{\link{polynomial.model.lsd}} which contains the meta-model estimated hyper-parameters.
}
  \item{data}{
an optional object created by a previous call to \code{\link{read.doe.lsd}} which sets the default values for \code{lower.domain}, \code{upper.domain} and \code{starting.values}.
}
  \item{lower.domain}{
a vector or a single-line data frame which contains the minimum values to be considered for all the meta-model factors/variables. If \code{data} is provided, the default values are the lower limit ranges from the \code{.sa} file set in LSD.
}
  \item{upper.domain}{
a vector or a single-line data frame which contains the maximum values to be considered for all the meta-model factors/variables. If \code{data} is provided, the default values are the upper limit ranges from the \code{.sa} file set in LSD.
}
  \item{starting.values}{
a vector or a single-line data frame which contains the starting values to be used by \code{\link[rgenoud]{genoud}} for all the meta-model factors/variables. If \code{data} is provided, the default values are the calibration settings from the baseline configuration \code{.lsd} file set in LSD.
}
  \item{minimize}{
logical: set to FALSE to perform maximization. The default is TRUE (minimization).
}
  \item{pop.size}{
integer: the number of parallel search paths \code{\link[rgenoud]{genoud}} uses to solve the optimization problem. The default is 1000.
}
  \item{max.generations}{
integer: the maximum number of generations that \code{\link[rgenoud]{genoud}} will run when attempting to optimize a function. The default is 30.
}
  \item{wait.generations}{
integer: if there is no improvement in the objective function after this number of generations, \code{\link[rgenoud]{genoud}} will accept the optimum. The default is 10.
}
  \item{precision}{
numeric: the tolerance level used by \code{\link[rgenoud]{genoud}}. Numbers within \code{precision} are considered to be equal. The default is 1e-5.
}
  \item{nnodes}{
integer: the maximum number of parallel computing nodes (parallel threads) in the current computer to be used for reading the files. The default, \code{nnodes = 1}, means single thread processing (no parallel threads). If equal to zero, creates up to one node per CPU (physical) core. Only \code{Fork} clusters can be used, because \code{PSOCK} clusters are not working now, so this option is not available in Windows. Please note that each node requires its own memory space, so memory usage increases linearly with the number of nodes.
}
}
\details{
This function searches for maximum and minimum response surface values by the application of a genetic algorithm (Sekhon & Walter, 1998).

The function can be used to perform any form of optimization by means the user defines the proper objective function to be maximized (or minimized). Any form of objective function can be easily defined as a new variable to the DoE data set when it is created by \code{\link{read.doe.lsd}}.

This function is a wrapper to the function \code{\link[rgenoud]{genoud}} in \code{rgenoud} package.
}
\value{
The function returns a single-line data frame which contains values (in the rows) for all the meta-model factors/variables (in the columns) or NULL if optimization fails.
}
\references{
Sekhon JS, Walter RM (1998). \emph{Genetic optimization using derivatives: theory and application to nonlinear models}. Political Analysis 7:187-210
}
\author{
\packageAuthor{LSDsensitivity}
}
\seealso{
\code{\link{read.doe.lsd}()},
\code{\link{kriging.model.lsd}()},
\code{\link{polynomial.model.lsd}()}

\code{\link[rgenoud]{genoud}}
}
\examples{
# get the example directory name
path <- system.file( "extdata/sobol", package = "LSDsensitivity" )

# Steps to use this function:
# 1. define the variables you want to use in the analysis
# 2. load data from a LSD simulation saved results using read.doe.lsd
# 3. fit a Kriging (or polynomial) meta-model using kriging.model.lsd
# 4. find the factor configuration that produce the minimum (or maximum)
#    value for the analysis variable defined in step 2

lsdVars <- c( "var1", "var2", "var3" )         # the definition of existing variables

dataSet <- read.doe.lsd( path,                 # data files folder
                         "Sim3",               # data files base name (same as .lsd file)
                         "var3",               # variable name to perform the sensitivity analysis
                         does = 2,             # number of experiments (data + external validation)
                         saveVars = lsdVars )  # LSD variables to keep in dataset

model <- kriging.model.lsd( dataSet )          # estimate best Kriging meta-model
\donttest{
config <- model.optim.lsd( model,              # find meta-model configuration for minimum response
                           dataSet )           # use the full range of factors and starting from
                                               # calibration
print( config )
}
}
\keyword{ models }
\keyword{ design }
