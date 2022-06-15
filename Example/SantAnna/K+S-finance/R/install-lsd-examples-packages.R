#******************************************************************
#
# ------------- Install K+S Model required packages --------------
#
#******************************************************************

install.packages( c( "LSDsensitivity",
                     "LSDirf",
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
                     "extrafont" ) )

#
# ATTENTION: LSD R packages can be also installed from disk,
# whenever not available or outdated in CRAN server.
#
# Download the latest versions from:
# https://github.com/SantAnnaKS/LSD, inside folder Rpkg
#
