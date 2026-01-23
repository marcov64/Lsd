#******************************************************************
#
# ------------- Install K+S Model required packages --------------
#
#******************************************************************

options( repos = c( CRAN = "https://cloud.r-project.org",
                    LSD = "https://ews.santannapisa.it/rpackages",
                    GGHALVES = "https://erocoar.r-universe.dev" ) )

install.packages( c( "LSDsensitivity",
                     "LSDirf",
                     "Rsubbotools",
                     "tseries",
                     "normalp",
                     "nortest",
                     "mFilter",
                     "np",
                     "matrixStats",
                     "gplots",
                     "corrplot",
                     "rmutil",
                     "plotrix",
                     "textplot",
                     "extrafont",
                     "robustbase",
                     "gghalves",
                     "ggthemes" ) )

#
# ATTENTION: LSD R packages can be also installed from disk,
# whenever not available or outdated in CRAN server.
#
# Download the latest versions from:
# https://github.com/SantAnnaKS/LSD, inside folder Rpkg
#
