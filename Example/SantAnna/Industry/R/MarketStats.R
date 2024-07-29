#******************************************************************
#
# -------- Industry Model: market level data generation ----------
#
#   Written by Marcelo C. Pereira, University of Campinas
#
#   Copyright Marcelo C. Pereira
#   Distributed under the GNU General Public License
#
#   IMPORTANT: this script assumes the R working directory is set
#   to the R subfolder where this script file is stored. This can
#   be done automatically in RStudio if a project is created at
#   this subfolder, or using the command setwd(). The "folder"
#   variable below must always point to a (relative) subfolder
#   of the R working directory.
#
#******************************************************************

# ==== User defined parameters ====

folder   <- "MarkII/Beta"             # subfolder of working dir containing data
baseName <- "MarkII-Beta"             # data files base name
iniDrop <- 0                          # initial time steps to drop from analysis (0=none)
nKeep <- -1                           # number of time steps to keep (-1=all)


# ==== Fields to process ====

fieldNames <- c( "E", "HHI", "aGrowthAvg", "ageAvg", "dS", "growthAvg" )


# ====== External support functions & definitions ======

source( "StatFuncs.R" )


# ==== MAIN SCRIPT (data processing starts here) ====

# ---- Read data files ----

readFiles <- list.files( path = folder, pattern = paste0( baseName, "_[0-9]+.res"),
                         full.names = TRUE )

# Determine the Monte Carlo sample size
nSize  <- length( readFiles )

# Read data from text files and format it as a 3D array with labels
dataSet <- read.3d.lsd( readFiles, fieldNames, skip = iniDrop, nrows= nKeep )

# Compute Monte Carlo averages and std. deviation and store in 2D arrrays
stats <- info.stats.lsd( dataSet )

# Insert a t column
t <- as.integer( rownames( stats$avg ) )
Adata <- as.data.frame( cbind( t, stats$avg ) )
Sdata <- as.data.frame( cbind( t, stats$sd ) )
Mdata <- as.data.frame( cbind( t, stats$max ) )
mdata <- as.data.frame( cbind( t, stats$min ) )

# Write to the disk as CSV files for Excel
write.csv( Adata, paste0( folder, "/", baseName, "_avg.csv" ),
           row.names = FALSE )
write.csv( Sdata, paste0( folder, "/", baseName, "_sd.csv" ),
           row.names = FALSE )
write.csv( Mdata, paste0( folder, "/", baseName, "_max.csv" ),
           row.names = FALSE )
write.csv( mdata, paste0( folder, "/", baseName, "_min.csv" ),
           row.names = FALSE )

# ==== Aggregated variables unimodality and symmetry tests ====

statUni <- symmet.test.lsd( dataSet, fieldNames, digits = 3 )

# write to disk
write.csv( statUni, paste0(folder, "/", baseName, "_unimod_tests.csv" ) )

# ==== Aggregated variables stationarity and ergodicity tests ====

statErgo <- ergod.test.lsd( dataSet, fieldNames, digits = 3 )

# write to disk
write.csv( statErgo, paste0(folder, "/", baseName, "_ergod_tests.csv" ) )
