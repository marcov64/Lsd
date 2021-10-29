# library(rgl)

# Morris's screening method (Morris 1992, Campolongo 2007)
#
# Gilles Pujol 2006-2008
# Modified by Frank Weber (2016): support model functions
# returning a matrix or a 3-dimensional array.
#
# Adapted by Marcelo C. Pereira (2017) to reuse just
# the ee.oat function and print/plot methods
# as CRAN does not accept linking unexported code
#
# Sub-files:
# * morris_oat.R


ind.rep <- function(i, p) {
# indices of the points of the ith trajectory in the DoE
  (1 : (p + 1)) + (i - 1) * (p + 1)
}


print.morris.lsd <- function(x, ...) {
  cat("\nCall:\n", deparse(x$call), "\n", sep = "")
  if (!is.null(x$y) && inherits(x$y, "numeric")) {
    cat("\nModel runs:", length(x$y), "\n")
    mu <- apply(x$ee, 2, mean)
    mu.star <- apply(x$ee, 2, function(x) mean(abs(x)))
    sigma <- apply(x$ee, 2, stats::sd)

    out <- data.frame(mu, mu.star, sigma)
    rownames(out) <- colnames(x$ee)
    print(out)
  } else if (!is.null(x$y) && inherits(x$y, "matrix")) {
    cat("\nModel runs:", nrow(x$y), "\n")
    mu <- apply(x$ee, 3, function(M){
      apply(M, 2, mean)
    })
    mu.star <- apply(abs(x$ee), 3, function(M){
      apply(M, 2, mean)
    })
    sigma <- apply(x$ee, 3, function(M){
      apply(M, 2, stats::sd)
    })
    out <- list("mu" = mu, "mu.star" = mu.star, "sigma" = sigma)
    print.listof(out)
  } else if (!is.null(x$y) && inherits(x$y, "array")) {
    cat("\nModel runs:", dim(x$y)[1], "\n")
    mu <- lapply(1:dim(x$ee)[4], function(i){
      apply(x$ee[, , , i, drop = FALSE], 3, function(M){
        apply(M, 2, mean)
      })
    })
    mu.star <- lapply(1:dim(x$ee)[4], function(i){
      apply(abs(x$ee)[, , , i, drop = FALSE], 3, function(M){
        apply(M, 2, mean)
      })
    })
    sigma <- lapply(1:dim(x$ee)[4], function(i){
      apply(x$ee[, , , i, drop = FALSE], 3, function(M){
        apply(M, 2, stats::sd)
      })
    })
    names(mu) <- names(mu.star) <- names(sigma) <- dimnames(x$ee)[[4]]
    cat("----------------\nmu:\n\n")
    print.listof(mu)
    cat("----------------\nmu.star:\n\n")
    print.listof(mu.star)
    cat("----------------\nsigma:\n\n")
    print.listof(sigma)
  } else {
    cat("\n(empty)\n")
  }
}


plot.morris.lsd <- function(x, identify = FALSE, atpen = FALSE,
                        y_col = NULL, y_dim3 = NULL, ...) {
  if (!is.null(x$ee)) {
#    if(is(x$y,"numeric")){
    if(inherits(x$y, "numeric")){
      mu.star <- apply(x$ee, 2, function(x) mean(abs(x)))
      sigma <- apply(x$ee, 2, stats::sd)
#    } else if(is(x$y,"matrix")){
    } else if(inherits(x$y, "matrix")){
      if(is.null(y_col)) y_col <- 1
      if(!is.null(y_dim3)){
        warning("Argument \"y_dim3\" is ignored since the model output is ",
                "a matrix")
      }
      mu.star <- apply(x$ee[, , y_col, drop = FALSE], 2,
                       function(x) mean(abs(x)))
      sigma <- apply(x$ee[, , y_col, drop = FALSE], 2, stats::sd)
      #    } else if(is(x$y,"array")){
    } else if(inherits(x$y, "array")){
      if(is.null(y_col)) y_col <- 1
      if(is.null(y_dim3)) y_dim3 <- 1
      mu.star <- apply(x$ee[, , y_col, y_dim3, drop = FALSE], 2,
                       function(x) mean(abs(x)))
      sigma <- apply(x$ee[, , y_col, y_dim3, drop = FALSE], 2, stats::sd)
    }

    graphics::plot(mu.star, sigma, pch = 20, xlab = expression(mu^"*"),
         ylab = expression(sigma), ...)

    if (identify) {
      graphics::identify(mu.star, sigma, labels = colnames(x$ee), atpen = atpen)
    } else {
      graphics::text(mu.star, sigma, labels = colnames(x$ee), pos = 4)
    }
  }
}
