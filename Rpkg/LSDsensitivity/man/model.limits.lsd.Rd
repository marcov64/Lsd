\name{model.limits.lsd}
\alias{model.limits.lsd}
\title{
Find maximum and minimum meta-model responses
}
\description{
This function identifies the maximum and minimum meta-model response values when exploring a subset of three meta-model factors (parameters): one at a time and jointly changing the first and the second factors. All the remaining factors are kept at default/calibration values.
}
\usage{
model.limits.lsd( data, model, sa = NULL, factor1 = 1, factor2 = 2, factor3 = 3,
                  pop.size = 1000, max.generations = 30, wait.generations = 10,
                  precision = 1e-05, nnodes = 1 )
}
\arguments{
  \item{data}{
an object created by a previous call to \code{\link{read.doe.lsd}} which contains all the required experimental data for the analysis.
}
  \item{model}{
an object created by a previous call to \code{\link{kriging.model.lsd}} or \code{\link{polynomial.model.lsd}} which contains the meta-model estimated hyper-parameters.
}
  \item{sa}{
an optional object created by a previous call to \code{\link{sobol.decomposition.lsd}} which contains the meta-model factors importance used to select the top 3 most influential ones for the analysis.
}
  \item{factor1}{
integer: the index (according to the Sobol index table) to the first factor to be evaluated individually and jointly with the second factor. The default is the first (index order) factor. Not used if a \code{sa} object is supplied.
}
  \item{factor2}{
integer: the index (according to the Sobol index table) to the second factor to be evaluated individually and jointly with the first factor. The default is the second (index order) factor. Not used if a \code{sa} object is supplied.
}
  \item{factor3}{
integer: the index (according to the Sobol index table) to the third factor to be evaluated only individually. The default is the third (index order) factor. Not used if a \code{sa} object is supplied.
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

This function is a wrapper to the function \code{\link[rgenoud]{genoud}} in \code{rgenoud} package.
}
\value{
The function returns a data frame containing the found limit values.
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
# 4. identify the most influential factors applying sobol.decomposition.lsd
# 5. find the maximum and minimum response values for the 3 top-influential
#    factors/parameters using model.limits.lsd
# 6. plot the response surface indicating the limit points found

lsdVars <- c( "var1", "var2", "var3" )          # the definition of existing variables

dataSet <- read.doe.lsd( path,                  # data files folder
                         "Sim3",                # data files base name (same as .lsd file)
                         "var3",                # variable name to perform the sensitivity analysis
                         does = 2,              # number of experiments (data + external validation)
                         saveVars = lsdVars )   # LSD variables to keep in dataset

model <- kriging.model.lsd( dataSet )           # estimate best Kriging meta-model

SA <- sobol.decomposition.lsd( dataSet, model ) # find Sobol indexes
\donttest{
limits <- model.limits.lsd( dataSet,            # LSD experimental data set
                            model,              # estimated meta-model
                            SA )                # use top factors found before

print( limits )                                 # print a table with the limits

resp <- response.surface.lsd( dataSet, model, SA )# prepare surfaces data

# plot the 3D surface (top 2 factors)
theta3d <- 310                                  # horizontal view angle
phi3d <- 30                                     # vertical view angle
grid3d <- 25

zMat <- matrix( resp$calib[[ 2 ]]$mean, grid3d, grid3d, byrow = TRUE )
zlim <- range( zMat, na.rm = TRUE )

vt <- persp( resp$grid[[ 1 ]], resp$grid[[ 2 ]], zMat, col = "gray90",
             xlab = colnames( dataSet$doe )[ SA$topEffect[ 1 ] ], zlim = zlim,
             ylab = colnames( dataSet$doe )[ SA$topEffect[ 2 ] ], zlab = dataSet$saVarName,
             theta = theta3d, phi = phi3d, ticktype = "detailed" )

# plot the max, min and default points as colored markers
points( trans3d( as.numeric( dataSet$facDef[ SA$topEffect[ 1 ] ] ),
                 as.numeric( dataSet$facDef[ SA$topEffect[ 2 ] ] ),
                 resp$default$mean, vt ), col = "red", pch = 19, cex = 1.0 )
points( trans3d( limits[ SA$topEffect[ 1 ], 7 ],
                 limits[ SA$topEffect[ 2 ], 7 ],
                 limits[ "response", 7 ], vt ), col = "green", pch = 18, cex = 1.0 )
points( trans3d( limits[ SA$topEffect[ 1 ], 8 ],
                 limits[ SA$topEffect[ 2 ], 8 ],
                 limits[ "response", 8 ], vt ), col = "blue", pch = 18, cex = 1.0 )
}
}
\keyword{ models }
\keyword{ design }
