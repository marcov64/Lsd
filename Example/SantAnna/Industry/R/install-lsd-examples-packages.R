#******************************************************************
#
# ----------- Install Island Model required packages -------------
#
#******************************************************************

options( repos = c( CRAN = "https://cloud.r-project.org",
                    LSD = "https://ews.santannapisa.it/rpackages",
                    GGHALVES = "https://erocoar.r-universe.dev" ) )

install.packages( c( "LSDsensitivity",
                     "Rsubbotools",
                     "tseries",
                     "normalp",
                     "nortest",
                     "mFilter",
                     "np",
                     "matrixStats",
                     "gplots",
                     "rmutil",
                     "plotrix",
                     "extrafont",
                     "LaplacesDemon",
                     "robustbase",
                     "minpack.lm" ) )
