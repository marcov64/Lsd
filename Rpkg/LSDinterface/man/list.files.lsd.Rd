\name{list.files.lsd}
\alias{list.files.lsd}
\title{
List results files from a set of LSD runs
}
\description{
This function produce a character vector of the names of results files produced after the execution of LSD simulation runs. The list can be used with all function in this package requiring the argument \code{files}.
}
\usage{
list.files.lsd( path = ".", conf.name = "",
                type = c( "res", "tot", "csv" ),
                compressed = NULL, recursive = FALSE,
                join = FALSE, full.names = FALSE,
                sensitivity = FALSE )
}
\arguments{
  \item{path}{
a character vector of full or relative path name to the base directory from where to search the files; the default corresponds to the working directory, \code{\link[base]{getwd}()}. Tilde expansion is performed. Alternatively, the full path and name of the corresponding LSD configuration file (including the \code{.lsd} extension) can be provided.
}
  \item{conf.name}{
the LSD configuration file name (optionally including the \code{.lsd} extension) used to generate the desired results files; the default is to return all results files, irrespective of the configuration file used. Alternatively, a \link[base:regex]{regular expression} can be supplied. This argument takes precedence of any configuration file name provided together with the \code{path} argument.
}
  \item{type}{
the type (format/extension) of LSD results files to use among the options \code{c( "res", "tot", "csv" )}, used to define the extension of the files to be considered. \code{"res"} is the default. This option can be abbreviated.
}
  \item{compressed}{
a logical value indicating if (\code{TRUE}) to look only for compressed files with \code{.gz} extension, or uncompressed ones otherwise (\code{FALSE}). The default (\code{NULL}) is to list files irrespective if compressed or not.
}
  \item{recursive}{
a logical value indicating if the listing should recurse into sub-directories of \code{path}. The default (\code{FALSE}) is to scan just the sub-directory with the same name as \code{conf.name} (without the \code{.lsd} extension or numeric tags), if present (regular expression in \code{conf.name} is not considered), and \code{path}. If \code{TRUE}, the entire sub-directory tree, starting at \code{path}, is scanned for files.
}
  \item{join}{
a logical value indicating if results files from multiple sub-directories should be joined together in the return list. The default (\code{FALSE}) is to list files from just a single sub-directory, the first one found during the search starting from \code{path}.
}
  \item{full.names}{
a logical value specifying if (\code{TRUE}) the file names should be expanded to absolute path names. The default (\code{FALSE}) is to use relative (to \code{path}) file names.
}
  \item{sensitivity}{
a logical value specifying if (\code{TRUE}) the target results files are part of a sensitivity analysis design of experiment (DoE), which are double numbered in a particular format (\code{conf.name_XXX_YYY.res[.gz]}). The default (\code{FALSE}) is to assume files are just single numbered, which is usually inappropriate for DoE results files. See \link[LSDsensitivity:LSDsensitivity-package]{LSDsensitivity package documentation} for details.
}
}
\details{
The order by which sub-directories are explored may be relevant. By default, the function scans for results files in a sub-directory named as \code{conf.name}, if present, in the given initial directory \code{path}. Next, if \code{conf.name} has a numeric suffix in the format \code{name_XXX}, where XXX is any number of algarisms, it searches the sub-directory \code{name}, if present. Finally, it scans the initial \code{path} itself. If results files are present in more than one sub-directory, function returns only the files found in first one (except if \code{join = TRUE}), and issues a warning message. If \code{recursive = TRUE}, file search starts from \code{path} and proceeds until it encompasses the entire sub-directory tree. In this case, if multiple sub-directories contain the desired files, only the initial \code{path} takes precedence, and the rest of the tree is recurred in alphabetical order.

Please note that joining files from different sub-directories (\code{join = TRUE}) may combine results with incompatible data which cannot be processed together by the \code{read.xxx.lsd()} family of functions.
}
\value{
A character vector containing the names of the found results files in the specified (sub) directories (empty if there were no files). If a path does not exist or is not a directory or is unreadable it is skipped.
}
\note{
File naming conventions are platform dependent. The pattern matching works with the case of file names as returned by the OS.

\code{path} must specify paths which can be represented in the current codepage, and files/directories below path whose names cannot be represented in that codepage will most likely not be found.
}
\author{Marcelo C. Pereira}
\seealso{
\code{\link{read.3d.lsd}()},
\code{\link{read.4d.lsd}()},
\code{\link{read.raw.lsd}()},
\code{\link{read.single.lsd}()},
\code{\link{read.multi.lsd}()},
\code{\link{read.list.lsd}()},
\link[LSDsensitivity:LSDsensitivity-package]{LSDsensitivity package},
}
\examples{
# get the names of all files the example directory
list.files.lsd( system.file( "extdata", package = "LSDinterface" ) )

# expand search to the entire example directory tree
# for results from a configuration file named "Sim1.lsd"
# and join files found in all sub-directories conatining data
list.files.lsd( system.file( "extdata", package = "LSDinterface" ),
                "Sim1.lsd", recursive = TRUE, join = TRUE )
}
\keyword{ file }
\keyword{ interface }
\keyword{ misc }
