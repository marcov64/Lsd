\name{elementary.effects.lsd}
\alias{elementary.effects.lsd}
\title{
Elementary effects sensitivity analysis
}
\description{
This function performs the an elementary effects sensitivity analysis on the sample data produced by a LSD simulation model using the Morris (1991) one-at-a-time sampling method.
}
\usage{
elementary.effects.lsd( data, p = 4, jump = 2 )
}
\arguments{
  \item{data}{
an object created by a previous call to \code{\link{read.doe.lsd}} which contains all the required experimental data for the analysis.
}
  \item{p}{
integer: the number of levels of the DoE as set in LSD when the DoE was configured. The default is 4 (also the LSD default).
}
  \item{jump}{
integer: the size of the jump (increase/decrease) for each point change in the DoE as set in LSD when the DoE was configured. The default is 2 (also the LSD default).
}
}
\details{
The elementary effects analysis statistics are only meaningful if the DoE was created using the Morris design, as when LSD \code{EE Sampling...} option is used to produce the DoE.

This function is a wrapper to the function \code{\link[sensitivity]{morris}} in \code{\link[sensitivity]{sensitivity-package}}.
}
\value{
The function returns an object/list of class \code{morris.lsd} containing several items, among them:

\item{table}{the elementary effects sensitivity analysis results data.}

The returned object can also be directly printed or plotted using \code{plot()} or any similar function.  See the class \code{\link[sensitivity]{morris}} for full details, as this class is equivalent to it.
}
\references{
Morris MD (1991) \emph{Factorial sampling plans for preliminary computational experiments}.Technometrics 33(1):161-174
}
\author{
\packageAuthor{LSDsensitivity}
}
\note{
See the note in \link[LSDsensitivity]{LSDsensitivity-package} for step-by-step instructions on how to perform the complete sensitivity analysis process using LSD and R.
}
\seealso{
\code{\link{read.doe.lsd}},

\code{\link[sensitivity]{morris}} in \code{\link[sensitivity]{sensitivity-package}}
}
\examples{
# get the example directory name
path <- system.file( "extdata/ee", package = "LSDsensitivity" )

# Steps to use this function:
# 1. define the variables you want to use in the analysis
# 2. load data from a LSD simulation saved results using read.doe.lsd
# 3. perform the elementary effects analysis applying elementary.effects.lsd

lsdVars <- c( "var1", "var2", "var3" )         # the definition of existing variables

dataSet <- read.doe.lsd( path,                 # data files  folder
                         "Sim2",               # data files base name (same as .lsd file)
                         "var1",               # variable name to perform the sensitivity analysis
                         saveVars = lsdVars )  # LSD variables to keep in dataset

SA <- elementary.effects.lsd( dataSet,         # LSD experimental data set
                              p = 4,           # number of levels of the design (as set in LSD)
                              jump = 2 )       # number of jumps per level (as set in LSD)

print( SA )                                    # show analysis table
plot( SA )                                     # plot analysis chart
}
\keyword{ methods }
\keyword{ models }
\keyword{ design }
