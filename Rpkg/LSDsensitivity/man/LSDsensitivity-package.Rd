\name{LSDsensitivity-package}
\alias{LSDsensitivity}
\docType{package}
\title{
\packageTitle{LSDsensitivity}
}
\description{
\packageDescription{LSDsensitivity}
}
\details{
The LSDsensitivity R package provides tools to analyze simulated experiments from \strong{LSD}. LSD offers native tools to sample the configuration (factor) space of a simulation model using different design of experiments (DoE). The produced experimental design data can be transparently imported to R by using the function \code{\link{read.doe.lsd}()}.

The package offers two sensitivity analysis (SA) methods (\code{\link{elementary.effects.lsd}()} and \code{\link{sobol.decomposition.lsd}()}) pre-configured for application on LSD simulations: Morris Elementary Effects (EE) and Sobol Variance Decomposition (SVD).

EE (\code{\link{elementary.effects.lsd}()}) employs a simple one-factor-at-a-time (OAT) SA and is usually applied as an initial screening method while selecting relevant factors to a SVD global SA. EE requires an appropriate set of sample points (the DoE) which can be generated in LSD when "EE Sampling" is selected in the "Data" menu. Please make sure to take note of the DoE parameters used for the sampling, as they will be required for the configuration of the R analysis script.

Due to its high computational cost, \code{\link{sobol.decomposition.lsd}()} (SVD) is performed over a meta-model fitted from the experimental data produced by the LSD original model. The meta-model can be fitted using different sampling strategies offered by LSD, being "NOLH Sampling" (Near Orthogonal Latin Hypercube) usually the most efficient. Additionally to the set of samples used to fit the meta-model, it is recommended to also generate another set for the (external) validation of the meta-model ("MC Range Sampling" is the recommended option).

The package offers two meta-modeling (MM) methods for using with SVD: Kriging and polynomial. Kriging (\code{\link{kriging.model.lsd}()}) is offered under five different variance kernels (Matern 5/2, Matern3/2, Gaussian, exponential and power exponential) and two trend models (constant or first order polynomial) to choose, including auto-selection to the best fitting alternative. Polynomial meta-models of first or second order, with or without interactions, and auto-selection are also offered (\code{\link{polynomial.model.lsd}()}). Kriging is the recommended option in most cases.

Additionally, the package offers tools for the graphical representation of the meta-models response surfaces (2D and 3D) (\code{\link{response.surface.lsd}()}), to predict meta-model response in specific points in the factor space (\code{\link{model.pred.lsd}()}), to identify maximum and minimum responses from a set of factors (\code{\link{model.limits.lsd}()}), and to find optimal parameter settings using the meta-model (\code{\link{model.optim.lsd}()}).

For a complete list of exported functions, use \code{library(help = "LSDsensitivity")}.

\strong{LSD 7.0+} default installation provides \emph{example scripts} for the usage of the LSDsensitivity package. LSD can be downloaded at \url{https://github.com/marcov64/Lsd/}. They can also be retrieved from the package itself using the commands:

\strong{EE example}: \code{file.show(system.file("examples", "elementary-effects-SA.R", package = "LSDsensitivity"))}

\strong{Kriging SVD example}: \code{file.show(system.file("examples", "kriging-sobol-SA.R", package = "LSDsensitivity"))}

\strong{Polynomial SVD example}: \code{file.show(system.file("examples", "poly-sobol-SA.R", package = "LSDsensitivity"))}

\strong{Optimize MM example}: \code{file.show(system.file("examples", "optimize-MM.R", package = "LSDsensitivity"))}
}
\note{
Below are the minimum required steps to perform SA on a working LSD model using NOLH sampling, Kriging MM and SVD. The changes to perform an EE or to use a polynomial MM are also indicated, as options.

\enumerate{
\item Define the parameters/initial values to be explored in the SA, their max/min ranges and the result variables over which the SA is to be done

\item In \code{LMM} create a no-window (command prompt) version of your model by selecting menu \code{Model/Create 'No Window' Version}

\item In \code{LSD Browser} make sure that all parameters/initial values are set with the correct calibration/default values (menu \code{Data/Initial Values}), the required result variables are being saved (menu \code{Model/Change Element...}, click on \code{Save/OK} or simply \code{Save} in the right mouse button context menu) and the number of MC runs for each SA sample (point) is defined (menu \code{Run/Simulation Settings}, \code{Number of simulation runs} field, typically set to 10)

\item Save your setup in a baseline \code{.lsd} configuration file (menu \code{File/Save As...}), preferably in a new folder inside your current model configuration folder (you can create a new folder while in the \code{File/Save As...} dialog box, if you do not, LSD will create a new folder when saving SA configuration and results files, named as your baseline configuration)

\item (Re)load your baseline \code{.lsd} configuration if it is not already loaded (menu \code{File/Load...})

\item Choose the ranges (max/min) for each parameter/initial value in your SA exploration space by using the \code{Sensitivity Analysis} button in the menu \code{Model/Change Element...} window or the same option in the context menu (mouse right-button click on the parameter/variable name in the \code{Variables & Parameters} list box)

\item After choosing all ranges, save your exploration space definition as a \code{.sa} sensitivity analysis file using the same base name and folder as your \code{.lsd} baseline configuration (menu \code{File/Save Sensitivity...})

\item With both the created \code{.lsd} and \code{.sa} files loaded (use menu \code{File/Load...} and \code{File/Load Sensitivity...} if required), select \code{Data/Sensitivity Analysis/NOLH Sampling...} and accept the defaults (several new \code{.lsd} files will be created in your just created baseline configuration folder (or one created by LSD, if you did not), those are the sample points for the meta-model estimation)

\enumerate{
\item To perform Elementary Effects (EE) analysis instead of Sobol Variance Decomposition, in the step below select \code{Data/Sensitivity Analysis/EE Sampling...} instead (NOLH sampling cannot be used for EE)

\item If a polynomial meta-model (MM) is being estimated, sometimes it is preferred to use \code{Data/Sensitivity Analysis/MC Range Sampling...} despite not required
}

\item Immediately after the previous step, select menu \code{Data/Sensitivity Analysis/MC Range Sampling...} and accept the defaults (to create the external validation sample, more \code{.lsd} files will be created for the additional sampling points)

\enumerate{
\item EE analysis does not uses external validation, so skip this step for EE
}

\item Immediately after the previous step select menu \code{Run/Create/Run Parallel Batch}, accept using the just created configuration, adjust the number of cores only if going to run in another machine (8 in a modern PC, 20 in a basic server), and decide if you want to start the (time-consuming) processing right now or later (in the current or in another machine)

\item If running later in the same machine, you just have to execute the created script file (\code{.bat} or \code{.sh}) inside the new folder your baseline \code{.lsd} file was created (or the one LSD created if you did not)

\item If running in another machine, you have to copy the entire model folder and sub-folders to the new machine (the remaining LSD folders are not required), recompile LSD for the new platform if required and execute the script file (\code{.bat} or \code{.sh}) in the same folder as your baseline \code{.lsd} file

\item Open R (or RStudio) and check you have the following packages installed and download them if required (if you install LSDsensitivity from CRAN or another online repository, and not from a file, all other dependency packages should be automatically downloaded):

    \code{LSDsensitivity, LSDinterface, abind, tseries, car, minqa, nloptr, Rcpp, RcppEigen, lme4, SparseM, MatrixModels, pbkrtest, quantreg, DiceKriging, kSamples, SuppDists, randtoolbox, rngWELL, rgenoud, sensitivity, xts, TTR, quadprog, zoo, quantmod}

\item Open the \code{kriging-sobol-SA.R} example script (included in your LSD installation folder) in RStudio or your text editor

\enumerate{
\item For EE analysis, open \code{elementary-effects-SA.R} instead

\item For the use of a polynomial MM for the SVD analysis, open \code{poly-sobol-SA.R} instead
}

\item Adjust the vector \code{lsdVars} to contain all the LSD saved variables you want to use in your analysis (do not include saved but unused variables, for performance reasons), replacing the dummies \code{varX}

\item Adjust the vector \code{logVars} to contain all LSD variables (included in \code{lsdVars}) that require to have the log value used in the analysis (let the vector empty, i.e. \code{c( )}, if no log variable is required)

\item Include in the vector \code{newVars} any new variable (not included in \code{lsdVars}) that has to be added to the dataset (let the vector empty, i.e. \code{c( )}, if no new variable is required)

\item Adapt the \code{eval.vars()} function to compute any new variable included in \code{newVars} (use the commented example as a reference)

\item Adjust the arguments to the function \code{\link{read.doe.lsd}()} for the relative folder of LSD data files (default is same as R working directory), the data files base name (the file name chosen for the baseline configuration in step 4 without the \code{.lsd} suffix) and the name of the variable to be used as reference for the sensitivity analysis (you have to run the script multiple times if there is more than one)

\item Save the modified script, renaming if necessary, and run it in R (or click the \code{Source} button in RScript), redirecting output to a file first if required
}
}
\author{
\packageAuthor{LSDsensitivity}

Maintainer: \packageMaintainer{LSDsensitivity}
}
\references{
LSD documentation is available at \url{https://www.labsimdev.org/}

The latest LSD binaries and source code can be downloaded at \url{https://github.com/marcov64/Lsd/}.

Cioppa T, Lucas T (2007) \emph{Efficient nearly orthogonal and space-filling latin hypercubes}. Technometrics
49(1):45-55

Kleijnen JP (2009) \emph{Kriging metamodeling in simulation: a review}. Eur J Oper Res 192(3):707-716

Morris MD (1991) \emph{Factorial sampling plans for preliminary computational experiments}.Technometrics
33(1):161-174

Rasmussen C, Williams C (2006) \emph{Gaussian processes for machine learning}. MIT Press, Cambridge

Roustant O, Ginsbourger D, Deville Y (2012) \emph{Dicekriging, diceoptim: two R packages for the analysis of computer experiments by kriging-based metamodeling and optimization}. J Stat Softw 51(1):1-55

Saltelli A, Ratto M, Andres T, Campolongo F, Cariboni J, Gatelli D, Saisana M, Tarantola S (2008) \emph{Global sensitivity analysis: the primer}. Wiley, New York

Sekhon JS, Walter RM (1998). \emph{Genetic optimization using derivatives: theory and application to nonlinear models}. Political Analysis 7:187-210
}
\seealso{
\link[LSDinterface]{LSDinterface-package}
}
\keyword{ package }
\keyword{ interface }
\keyword{ file }
\keyword{ datasets }
\keyword{ datagen }
\keyword{ models }
\keyword{ design }
