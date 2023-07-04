#*******************************************************************************
#
# -------------- LSD tools for impulse-response function analysis -------------
#
#   Written by Marcelo C. Pereira, University of Campinas
#              Marco Amendola, University of L'Aquila
#
#   Copyright Marcelo C. Pereira & Marco Amendola
#   Distributed under the GNU General Public License
#
#*******************************************************************************

# ====== Support function to check for valid 3d array(s) ======

check_3D_data <- function( data1, data2 = NULL ) {

  if( is.null( data2 ) )
    data2 <- data1

  if( is.null( data1 ) || ! is.array( data1 ) || length( dim( data1 ) ) != 3 ||
      is.null( data2 ) || ! is.array( data2 ) || length( dim( data2 ) ) != 3 )
    stop( "Invalid data set(s) format (not 3D arrays)" )

  if( dim( data1 )[ 1 ] < 2 || dim( data1 )[ 2 ] < 1 || dim( data1 )[ 3 ] < 2 ||
      dim( data2 )[ 1 ] < 2 || dim( data2 )[ 2 ] < 1 || dim( data2 )[ 3 ] < 2 )
    stop( "Data sets must be at least 2x1x2 (time steps/variables/MC samples)" )

  if( dim( data1 )[ 1 ] != dim( data2 )[ 1 ] ||
      dim( data1 )[ 3 ] != dim( data2 )[ 3 ] )
    stop( "Data sets must have the same number of time steps and MC samples" )
}


# ====== Support function to build data array ======

build_state_data <- function( data, irf, add.vars, state.vars,
                              eval.state = NULL, metr.irf = NULL ) {

  check_3D_data( data )

  if( ! inherits( irf, "irf.lsd" ) )
    stop( "Invalid irf object (not from irf.lsd())" )

  if( irf$data.crc != digest::digest( data, algo = "crc32" ) )
    stop( "Data is different from data used to generate irf" )

  if( dim( data )[ 3 ] != ( length( irf$t.shock ) + length( irf$outliers ) ) )
    stop( "Data incompatible with IRF (different number of MC samples)" )

  if( ! is.null( add.vars ) && ! is.function( add.vars ) )
    stop( "Invalid function to add variables (add.vars)" )

  if( ! is.null( state.vars ) && ( ! is.character( state.vars ) ||
                                   length( state.vars ) == 0 ||
                                   any( state.vars == "" ) ) )
    stop( "Invalid state variable(s) (state.vars)" )

  if( ! is.null( eval.state ) && ! is.function( eval.state ) )
    stop( "Invalid state-evaluation function (eval.state)" )

  if( ! is.null( metr.irf ) && ! is.function( metr.irf ) )
    stop( "Invalid IRF metric function (metr.irf)" )

  if( ! is.null( state.vars ) && length( state.vars ) > 1 &&
      is.null( eval.state ) && is.null( metr.irf ) )
    stop( "Multiple state variables (state.vars) but no evaluation function (eval.state)")

  if( is.null( dimnames( data )[[ 2 ]] ) )
    dimnames( data )[[ 2 ]] <- 1 : dim( data )[ 2 ]

  # remove already detected outliers
  if( ! is.null( irf$outliers ) && length( irf$outliers ) > 0 )
    data <- data[ , , - irf$outliers[ irf$outliers <= dim( data )[ 3 ] ],
                  drop = FALSE ]

  nMC <- dim( data )[ 3 ]

  # add new state variables
  if( ! is.null( add.vars ) ) {
    MC <- NULL
    try( MC <- add.vars( data.frame( data[ , , 1 ] ) ) )

    if( is.null( MC ) || ! all( is.finite( MC ) ) ||
        nrow( MC ) != dim( data )[ 1 ] || ncol( MC ) <= dim( data )[ 2 ] )
      stop( "Invalid result from function to add variables (add.vars)" )

    newData <- array( as.matrix( MC ), dim = c( nrow( MC ), ncol( MC ), 1 ),
                      dimnames = list( rownames( MC ), colnames( MC ), NULL ) )
    if( nMC > 1 )
      for( i in 2 : nMC ) {
        MC <- NULL
        try( MC <- add.vars( data.frame( data[ , , i ] ) ) )

        if( is.null( MC ) || ! all( is.finite( MC ) ) ||
            nrow( MC ) != dim( data )[ 1 ] || ncol( MC ) <= dim( data )[ 2 ] )
          stop( "Invalid result from function to add variables (add.vars)" )

        newData <- abind::abind( newData, as.matrix( MC ),
                                 use.first.dimnames = TRUE )
      }

    dimnames( newData )[[ 3 ]] <- dimnames( data )[[ 3 ]]
  } else
    newData <- data

  # default to select all variables
  if( ! is.null( state.vars ) ) {
    for( var in state.vars )
      if( ! var %in% dimnames( newData )[[ 2 ]] )
        stop( "Invalid state variable(s) (state.vars)" )

    newData <- newData[ , state.vars, , drop = FALSE ]
  }
  else
    state.vars <- dimnames( newData )[[ 2 ]]

  # extract state variable data at impulse time
  stateData <- matrix( nrow = 0, ncol = length( state.vars ),
                       dimnames = list( NULL, state.vars ) )
  for( i in 1 : nMC )
    stateData <- rbind( stateData, newData[ irf$t.shock[ i ], state.vars, i ] )

  if( ! is.null( metr.irf ) ) {

    # create dependent metric variable used in regressions
    metric <- NULL
    try( metric <- metr.irf( irf$cir ) )

    if( is.null( metric ) || ! all( is.finite( metric ) ) ||
        length( metric ) != nMC )
      stop( "Invalid result from IRF metric function (metr.irf)" )

    # remove incomplete cases
    nas <- vector( )
    for( i in 1 : nMC )
      if( ! all( is.finite( stateData[ i, ] ) ) || ! is.finite( metric[ i ] ) )
        nas <- append( nas, i )

    if( length( nas ) > 0 ) {
      stateData <- stateData[ - nas, ]
      metric <- metric[ - nas ]
    }
  } else
    metric <- rowSums( stateData, na.rm = TRUE )

  stateData <- cbind( metric, stateData )
  colnames( stateData ) <- c( "metric", state.vars )

  return( stateData )
}


# ====== Support default (two-) state-defining function ======

eval.state.default <- function( data, states, state.num ) {

  regex <- "(.+) ([><]=?) (.+)"

  nodes <- unlist( strsplit( as.character( states$state.freq$State )[ state.num ],
                             " & ", fixed = TRUE ) )

  splits <- data.frame( matrix( nrow = length( nodes ), ncol = 3 ) )
  colnames( splits ) <- c( "var", "gt", "val" )

  # create table of splits to test on each MC run
  for( i in 1 : length( nodes ) ) {
    splits$var[ i ] <- sub( regex, "\\1", nodes[ i ] )
    splits$gt[ i ] <- substr( sub( regex, "\\2", nodes[ i ] ), 1, 1 ) == ">"
    splits$val[ i ] <- states$state.freq[ state.num, i * 4 ]
  }

  # evaluate the state of each MC run (line)
  state <- vector( length = nrow( data ) )
  for( i in 1 : nrow( data ) ) {

    # first split in state
    state[ i ] <- data[ i, splits$var[ 1 ] ] > splits$val[ 1 ]
    if( ! splits$gt[ 1 ] )
      state[ i ] <- ! state[ i ]

    # test other splits, as necessary
    if( state[ i ] && length( nodes ) > 1 )
      for( j in 2 : length( nodes ) ) {
        this <- data[ i, splits$var[ j ] ] > splits$val[ j ]
        if( ! splits$gt[ j ] )
          this <- ! this
        state[ i ] <- state[ i ] && this
        if( ! state[ i ] )
          break
      }
  }

  return( as.integer( state ) + 1 )
}


# ====== Support function to effectively handle IRF plotting ======

plot_irf <- function( data, ciLo, ciHi, ylim, xlab = "", ylab = "", leg = NULL,
                      scale = 1, center = TRUE, col = "black", lty = 1, lwd = 1,
                      col.ci = "black", lty.ci = 0, lwd.ci = 1, alpha.f = 0.2,
                      ... ) {

  if( ! inherits( data, "list" ) )
    data <- list( data )

  if( ! inherits( ciLo, "list" ) )
    ciLo <- list( ciLo )

  if( ! inherits( ciHi, "list" ) )
    ciHi <- list( ciHi )

  if( ! inherits( ylim, "list" ) )
    ylim <- list( ylim )

  nPlots <- length( data )

  yMin <- Inf
  yMax <- - Inf

  # find plot final vertical limits
  for( i in 1 : nPlots ) {
    if( ylim[[ i ]][ 1 ] >= 0 )
      ylim[[ i ]][ 1 ] <- ylim[[ i ]][ 1 ] / scale
    else
      ylim[[ i ]][ 1 ] <-  ylim[[ i ]][ 1 ] * scale

    if( ylim[[ i ]][ 2 ] >= 0 )
      ylim[[ i ]][ 2 ] <- ylim[[ i ]][ 2 ] * scale
    else
      ylim[[ i ]][ 2 ] <- ylim[[ i ]][ 2 ] / scale

    yMin <- min( yMin, ylim[[ i ]][ 1 ], na.rm = TRUE )
    yMax <- max( yMax, ylim[[ i ]][ 2 ], na.rm = TRUE )
  }

  if( center )
    ylim <- c( - max( abs( yMin ), abs( yMax ) ),
               max( abs( yMin ), abs( yMax ) ) )
  else
    ylim <- findYlim( yMin, yMax )

  x <- 0 : ( length( data[[ 1 ]] ) - 1 )
  col <- rep_len( col, nPlots )
  lty <- rep_len( lty, nPlots )
  lwd <- rep_len( lwd, nPlots )
  colCI <- rep_len( col.ci, nPlots )
  ltyCI <- rep_len( lty.ci, nPlots )
  lwdCI <- rep_len( lwd.ci, nPlots )

  # plot axes, titles and labels
  graphics::plot( x, data[[ 1 ]], ylim = ylim, xlab = xlab, ylab = ylab,
                  col = 0, ... )

  # plot each IRF
  for( i in 1 : nPlots ) {
    graphics::polygon( c( x, rev( x ) ), c( ciLo[[ i ]], rev( ciHi[[ i ]] ) ),
                       border = NA,
                       col = grDevices::adjustcolor( colCI[ i ],
                                                     alpha.f = alpha.f ) )
    graphics::abline( 0, 0 )
    graphics::lines( x, ciLo[[ i ]], col = colCI[ i ], lty = ltyCI[ i ],
                     lwd = lwdCI[ i ] )
    graphics::lines( x, ciHi[[ i ]], col = colCI[ i ], lty = ltyCI[ i ],
                     lwd = lwdCI[ i ] )
    graphics::lines( x, data[[ i ]], col = col[ i ], lty = lty[ i ],
                     lwd = lwd[ i ] )
  }

  if( ! is.null( leg ) ) {
    if( ! center && yMax <= 0 )
      pos <- "bottomleft"
    else
      pos <- "topleft"

    graphics::legend( x = pos, legend = leg, inset = 0.03, cex = 0.8,
                      lty = lty, lwd = 2, col = col )
  }
}


# ====== Support function to define plot window y limits ======

findYlim <- function( yMin, yMax, zero = FALSE,
                      botMargin = 0.1, topMargin= 0.2 ) {
  ylim <- c( yMin - botMargin * ( yMax - yMin ),
             yMax + topMargin * ( yMax - yMin ) )
  if( zero ) {
    if( ylim[ 1 ] <= 0 )
      ylim[ 1 ] <- yMin * ( 1 - botMargin )
    if( ylim[ 2 ] <= ylim[ 1 ] )
      ylim[ 2 ] <- yMax * ( 1 + topMargin )
  }

  return( ylim )
}


# ====== Unexported function adapted from partykit package ======

# Version: 	1.2-15
# Published: 	2021-08-23
# Author: 	Torsten Hothorn, Heidi Seibold, Achim Zeileis
# Maintainer: 	Torsten Hothorn <Torsten.Hothorn@R-project.org>
# License: 	GPL-2 | GPL-3

.list.rules.party <- function(x, i = NULL, ...) {
  if (is.null(i)) i <- partykit::nodeids(x, terminal = TRUE)
  if (length(i) > 1) {
    ret <- sapply(i, .list.rules.party, x = x)
    names(ret) <- if (is.character(i)) i else names(x)[i]
    return(ret)
  }
  if (is.character(i) && !is.null(names(x)))
    i <- which(names(x) %in% i)
  stopifnot(length(i) == 1 & is.numeric(i))
  stopifnot(i <= length(x) & i >= 1)
  i <- as.integer(i)
  dat <- partykit::data_party(x, i)
  if (!is.null(x$fitted)) {
    findx <- which("(fitted)" == names(dat))[1]
    fit <- dat[,findx:ncol(dat), drop = FALSE]
    dat <- dat[,-(findx:ncol(dat)), drop = FALSE]
    if (ncol(dat) == 0)
      dat <- x$data
  } else {
    fit <- NULL
    dat <- x$data
  }

  rule <- c()

  recFun <- function(node) {
    if (partykit::id_node(node) == i) return(NULL)
    kid <- sapply(partykit::kids_node(node), partykit::id_node)
    whichkid <- max(which(kid <= i))
    split <- partykit::split_node(node)
    ivar <- partykit::varid_split(split)
    svar <- names(dat)[ivar]
    index <- partykit::index_split(split)
    if (is.factor(dat[, svar])) {
      if (is.null(index))
        index <- ((1:nlevels(dat[, svar])) > partykit::breaks_split(split)) + 1
      slevels <- levels(dat[, svar])[index == whichkid]
      srule <- paste(svar, " %in% c(\"",
                     paste(slevels, collapse = "\", \"", sep = ""), "\")",
                     sep = "")
    } else {
      if (is.null(index)) index <- 1:length(kid)
      breaks <- cbind(c(-Inf, partykit::breaks_split(split)),
                      c(partykit::breaks_split(split), Inf))
      sbreak <- breaks[index == whichkid,]
      right <- partykit::right_split(split)
      srule <- c()
      if (is.finite(sbreak[1]))
        srule <- c(srule,
                   paste(svar, ifelse(right, ">", ">="), sbreak[1]))
      if (is.finite(sbreak[2]))
        srule <- c(srule,
                   paste(svar, ifelse(right, "<=", "<"), sbreak[2]))
      srule <- paste(srule, collapse = " & ")
    }
    rule <<- c(rule, srule)
    return(recFun(node[[whichkid]]))
  }
  node <- recFun(partykit::node_party(x))
  paste(rule, collapse = " & ")
}
