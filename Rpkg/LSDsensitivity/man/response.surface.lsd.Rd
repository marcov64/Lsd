\name{response.surface.lsd}
\alias{response.surface.lsd}
\title{
Generate the meta-model 3D response surface data
}
\description{
This function produces a data object for the three-dimensional graphical representations of the meta-model response surfaces for a set of factors (parameters), including the confidence interval for the surfaces.
}
\usage{
response.surface.lsd( data, model, sa, gridSz = 25, defPos = 2,
                      factor1 = 0, factor2 = 0, factor3 = 0 )
}
\arguments{
  \item{data}{
an object created by a previous call to \code{\link{read.doe.lsd}} which contains all the required experimental data for the analysis.
}
  \item{model}{
an object created by a previous call to \code{\link{kriging.model.lsd}} or \code{\link{polynomial.model.lsd}} which contains the meta-model estimated hyper-parameters.
}
  \item{sa}{
an object created by a previous call to \code{\link{sobol.decomposition.lsd}} which contains the estimated total and conditional variances for all the meta-model factors.
}
  \item{gridSz}{
integer: the number of divisions in the 3D wire frame grid. The default is 25.
}
  \item{defPos}{
1, 2, 3: the position of the default/calibration configuration on the 3 plot sequence. The default is 2 (center position).
}
  \item{factor1}{
integer: the index of the first most-important factor: 0 = automatic selection (according to the Sobol index, the default); any other number = the selected factor index, according to DoE factor order.
}
  \item{factor2}{
integer: the index of the second most-important factor: 0 = automatic selection (according to the Sobol index, the default); any other number = the selected factor index, according to DoE factor order.
}
  \item{factor3}{
integer: the index of the third most-important factor: 0 = automatic selection (according to the Sobol index, the default); any other number = the selected factor index, according to DoE factor order.
}
}
\details{
This function produces data for three different wire frame 3D plots. In the 3 plots, the x-y plan is defined by the 2 most-important factors (calculated or set by the user in \code{\link{sobol.decomposition.lsd}}) and the z axis represents the response variable chosen. The three different plots shows the response surface for three values of the third most-important factor: the minimum, the default/calibration and the maximum. The order the three response surfaces are shown is defined by \code{defPos}.

The automatically set most-important factors can be overridden by any factors chosen by the user by the usage of the arguments \code{factor1}, \code{factor2} and \code{factor3}. This way, the response surfaces can be represented for a combination of any 3 factors (parameters) in the model.
}
\value{
The function returns an object/list of class \code{response} containing three similar objects, one for each 3D plot, each of them comprised of:

\item{calib}{the predicted meta-model response values on each point of the 3D grid.}
\item{factor}{the predicted values for each individual factor.}
\item{default}{the predicted values for the default/calibration configuration.}
}
\author{
\packageAuthor{LSDsensitivity}
}
\note{
See the note in \link[LSDsensitivity]{LSDsensitivity-package} for step-by-step instructions on how to perform the complete sensitivity analysis process using LSD and R.
}
\seealso{
\code{\link{read.doe.lsd}()},
\code{\link{kriging.model.lsd}()},
\code{\link{polynomial.model.lsd}()},
\code{\link{sobol.decomposition.lsd}()}
}
\examples{
# get the example directory name
path <- system.file( "extdata/sobol", package = "LSDsensitivity" )

# Steps to use this function:
# 1. define the variables you want to use in the analysis
# 2. load data from a LSD simulation saved results using read.doe.lsd
# 3. fit a Kriging (or polynomial) meta-model using kriging.model.lsd
# 4. identify the most influential factors applying sobol.decomposition.lsd
# 5. calculate the response surface for the selected factors using model.limits.lsd
# 6. plot the response surface

lsdVars <- c( "var1", "var2", "var3" )          # the definition of existing variables

dataSet <- read.doe.lsd( path,                  # data files folder
                         "Sim3",                # data files base name (same as .lsd file)
                         "var3",                # variable name to perform the sensitivity analysis
                         does = 2,              # number of experiments (data + external validation)
                         saveVars = lsdVars )   # LSD variables to keep in dataset

model <- kriging.model.lsd( dataSet )           # estimate best Kriging meta-model

SA <- sobol.decomposition.lsd( dataSet, model ) # find Sobol indexes

resp <- response.surface.lsd( dataSet,          # LSD experimental data set
                              model,            # estimated meta-model
                              SA )              # Sobol sensitivity analysis results

theta3d <- 310                                  # horizontal view angle
phi3d <- 30                                     # vertical view angle
grid3d <- 25

for( i in 1 : 3 ) {                             # do for each top factor
                                                # plot 3D grid charts
  zMat <- matrix( resp$calib[[ i ]]$mean, grid3d, grid3d, byrow = TRUE )
  zlim <- range( zMat, na.rm = TRUE )

  vt <- persp( resp$grid[[ 1 ]], resp$grid[[ 2 ]], zMat, col = "gray90",
               xlab = colnames( dataSet$doe )[ SA$topEffect[ 1 ] ], zlim = zlim,
               ylab = colnames( dataSet$doe )[ SA$topEffect[ 2 ] ], zlab = dataSet$saVarName,
               theta = theta3d, phi = phi3d, ticktype = "detailed" )
}
}
\keyword{ models }
\keyword{ design }
