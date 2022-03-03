#******************************************************************
#
# ----------- K+S Impulse-response function analysis ------------
#
# Copyright by Marco Amendola
#
#******************************************************************

#needed libraries
library( LSDinterface )
library( boot )
library( partykit )
library( randomForest )

# ----------------------------EXPERIMENT SETTINGS------------------------------------

folder      <- "data"               # data files folder
baseName    <- "irf"                # data files base name (no-shock/shock:1/2)
iniDrop     <- 0                    # initial time steps to drop (0=none)
nKeep       <- -1                   # number of time steps to keep (-1=all)

sho         <- "rShock"             # name of the shock variable
va          <- "GDPreal"            # name of the variable to compute irf
nam         <- "GDP (real)"         # name of the variable to put in the irf-plot

per         <- 21                   # Max horizon to compute irf
ci_boot     <- 0.9                  # CI for bootstrap
rep_boot    <- 1000                 # bootstrap repetition
flag_cum    <- 0                    # 0 no cumulative irf, 1 cumulative irf
flag_perc   <- 1                    # 0 absolute deviation, 1 irf in percentage term
flag_median <- 0                    # 0 to report mean irf, 1 to report median irf
k           <- 3                    # outliers parameter
tr          <- 0.02                 # trim parameter for trimmed mean

lim_up      <- 0.13                 # max y-scale of graph for irf
lim_low     <- -0.15                # min y-scale of graph for irf
lim_up_cum  <- 0.13                 # max y-scale of graph for C-Irf
lim_low_cum <- -2.0                 # min y-scale of graph for C-Irf

flag_cum_statedep <- 1              # 1:graph state-dependent x irf, 0:no graph

repName     <- ""                   # report files base name (if "" same baseName)
plotRows    <- 1                    # number of plots per row in a page
plotCols    <- 1  	                # number of plots per column in a page
plotW       <- 10                   # plot window width
plotH       <- 7                    # plot window height


#### Set of possible state variable (explanatory variables)

# LSD-names of potential state-defining variables
state_variables<-c("Bda", "Bfail", "Q2u", "dGDP", "r", "dCPI", "U", "mu2avg",
                   "HH1", "HH2", "DefPgdp", "N", "Q2", "PiB", "GDPnom", "Pi1",
                   "Pi2", "NWb", "NW1", "NW2" )

# Nice names for the state-defining variables
state_names<-c("BDA", "BFAIL", "UTIL", "GROWTH",
               "R", "INFL", "U", "MARKUP",
               "HH1", "HH2", "DEF_GDP", "N", "Q2",
               "PIB", "GDP_NOM", "PI1", "PI2", "NWB",
               "NW1", "NW2")


# ==== Create PDF ====

if( repName == "" ) repName <- baseName
if( flag_median ) mcStat <- "median" else mcStat <- "mean"

pdf( paste0( folder, "/", repName, "_irf_plots_", mcStat, ".pdf" ),
     width = plotW, height = plotH )
par( mfrow = c ( plotRows, plotCols ) )


# data reading
noshock <- read.list.lsd( list.files.lsd( folder, paste0( baseName, "1" ) ),
                          c( state_variables, sho, va ),
                          skip = iniDrop, nrows = nKeep )
shock <- read.list.lsd( list.files.lsd( folder, paste0( baseName, "2" ) ),
                        c( state_variables, sho, va ),
                        skip = iniDrop, nrows = nKeep )


x<-seq(1:(per-1))
y<-seq(1:per)

samplemean <- function(x, d) {
  return(mean(x[d], trim = tr))
}

samplemedian <- function(x, d) {
  return(median(x[d]))
}


#---------------- SEED-SPECIFIC IRF --------------------------

no <- si <- imp_resp <- shockG <- shockT <- implen <- impper <- res <- resper <- len <- shockper <- totalmult <-cummult <- list()

mult_data<-cumulate_mult<- data.frame()

con<-conn<-con_med<-conn_med<-list()
int_min_boot <- int_max_boot<-int_min_boot_med <- int_max_boot_med<-vector()


#cycle to compute seed-specific irf and seed-specific C-irf

for (i in 1: length(noshock))
{
  no[[ i ]] <- noshock[[ i ]][,va]
  si[[ i ]] <- shock[[ i ]][,va]
  imp_resp [[ i ]] <- si[[ i ]] - no[[ i ]]
  shockG[[ i ]] <- shock[[ i ]][,sho]

  # search for first shock period
  for ( j in 1 : (length( shockG[[ i ]] ) -1))
    if ( shockG[[ i ]][ j ] != 0 )
      break;
  if ( shockG[[ i ]][ j ] != 0 ) {
    shockT[[ i ]] <- j

    resper[[ i ]] <- imp_resp[[ i ]][ shockT[[ i ]] : ( shockT[[ i ]] + per ) ]

    shockper[[ i ]] <-1                 #if irf are in absolute deviation

    if(flag_perc==1)                    #if irf are in percentage term
#      shockper[[ i ]] <- noshock[[ i ]][,va][ shockT[[ i ]] - 1 ] # really pick Tshock-1?
      shockper[[ i ]] <- noshock[[ i ]][,va][shockT[[ i ]]]

    totalmult[[ i ]] <- vector( )
    cummult[[ i ]] <- vector( )

    imp<-0

    for ( w in 1 : per ) {
      if ( w > 1 )
        imp<- imp + resper[[ i ]][ w ]
      totalmult[[ i ]][ w - 1 ] = resper[[ i ]][ w ] / shockper[[i]][1]
      cummult[[ i ]][ w - 1 ] = imp/shockper[[i]][1]

    }
  } else {
    shockT[[ i ]] <- 0
    implen[[ i ]] <- 0
  }
}


# create dataset with seed-specific irf and C-irf

for (i in 1 : length(totalmult)) {
  mult_data <- rbind(mult_data,totalmult[[ i ]])
  cumulate_mult <- rbind(cumulate_mult,cummult[[ i ]])
}


# remove (evenutal) outliers [very high thrheshold, just to exclude possible "crashes" in the simulations]

sett_percentile<-stack(lapply(mult_data[1:ncol(mult_data)], quantile, prob = 0.75, names = FALSE, na.rm=TRUE))
sett_percentile<-ts(sett_percentile)
vent_percentile<-stack(lapply(mult_data[1:ncol(mult_data)], quantile, prob = 0.25, names = FALSE, na.rm=TRUE))
vent_percentile<-ts(vent_percentile)

out_max <- out_min <-vector()

out_max<-sett_percentile + k*(sett_percentile - vent_percentile)
out_min<-vent_percentile - k*(sett_percentile - vent_percentile)


out_long<- (cumulate_mult[,17] + cumulate_mult[,18] + cumulate_mult[,19] + cumulate_mult[,20]) /4

out_long_tr<-abs(quantile(out_long, 0.75) - quantile(out_long, 0.25))


outliers<-vector()

#remove cases with strong difference on impact and on the cumulated irf at long h
for (i in 1: nrow(mult_data)) {
  if(mult_data[i,1]>out_max[1] | mult_data[i,1]<out_min[1] |
     out_long[i] > (quantile(out_long, 0.75) + (k*out_long_tr) ) |out_long[i] < (quantile(out_long, 0.25) - (k*out_long_tr) ) )
  {
    outliers<-rbind(outliers, i )
  }
}


if(length(outliers)>0)
{
  mult_data<-mult_data[-c(outliers),]
  cumulate_mult<-cumulate_mult[-c(outliers),]
}



#------------- LINEAR IRF (MC average, MC median, CI...) ------------------------------

av_mult<-apply(mult_data,2,mean,trim=tr, na.rm=TRUE)
median_mult<-apply(mult_data,2,median,na.rm=TRUE)

av_cummult<-apply(cumulate_mult,2,mean,trim=tr, na.rm=TRUE)
median_cummult<-apply(cumulate_mult,2,median,na.rm=TRUE)

if(flag_cum==0)
{
  if(flag_median==0)
  {
    for (i in 1: (per-1)) {
      con[[i]]<-boot(data=mult_data[,i], statistic = samplemean, R=rep_boot)
      conn[[i]]<-boot.ci(con[[i]], conf =ci_boot, type="basic")
      int_min_boot<-rbind(int_min_boot, conn[[i]]$basic[4])
      int_max_boot<-rbind(int_max_boot, conn[[i]]$basic[5])
    }

    plot(av_mult, type = "l", ylab = "irf", xlab = "h", main = nam,
         ylim = c(lim_low,lim_up), col=2)

    polygon( c(x, rev(x)), c(int_min_boot, rev(int_max_boot)),  col=gray(0.97), border = NA)
    points(av_mult, type = "l", col=2, lwd=1)
    points(int_max_boot, type = "l", lty=2)
    points(int_min_boot, type = "l", lty=2)
    abline(0,0, lwd=1)

  }

  if(flag_median==1)
  {
    for (i in 1: (per-1)) {
      con_med[[i]]<-boot(data=mult_data[,i], statistic = samplemedian, R=rep_boot)
      conn_med[[i]]<-boot.ci(con_med[[i]], conf =ci_boot, type="basic")
      int_min_boot_med<-rbind(int_min_boot_med, conn_med[[i]]$basic[4])
      int_max_boot_med<-rbind(int_max_boot_med, conn_med[[i]]$basic[5])
    }

    plot(median_mult, type = "l", ylab = "irf", xlab = "h", main = nam,
         ylim = c(lim_low,lim_up), col=2)

    polygon( c(x, rev(x)), c(int_min_boot_med, rev(int_max_boot_med)),  col=gray(0.97), border = NA)
    points(median_mult, type = "l", col=2, lwd=1)
    points(int_max_boot_med, type = "l", lty=2)
    points(int_min_boot_med, type = "l", lty=2)
    abline(0,0, lwd=1)
  }

}


if(flag_cum==1)
{
  if(flag_median==0)
  {
    for (i in 1: (per-1)) {
      con[[i]]<-boot(data=cumulate_mult[,i], statistic = samplemean, R=rep_boot)
      conn[[i]]<-boot.ci(con[[i]], conf =ci_boot, type="basic")
      int_min_boot<-rbind(int_min_boot, conn[[i]]$basic[4])
      int_max_boot<-rbind(int_max_boot, conn[[i]]$basic[5])
    }

    plot(av_cummult, type = "l", ylab = "irf", xlab = "h", main = nam,
         ylim = c(lim_low_cum,lim_up_cum), col=2)

    polygon( c(x, rev(x)), c(int_min_boot, rev(int_max_boot)),  col=gray(0.97), border = NA)
    points(av_cummult, type = "l", col=2, lwd=1)
    points(int_max_boot, type = "l", lty=2)
    points(int_min_boot, type = "l", lty=2)
    abline(0,0, lwd=1)

  }

  if(flag_median==1)
  {
    for (i in 1: (per-1)) {
      con_med[[i]]<-boot(data=cumulate_mult[,i], statistic = samplemedian, R=rep_boot)
      conn_med[[i]]<-boot.ci(con_med[[i]], conf =ci_boot, type="basic")
      int_min_boot_med<-rbind(int_min_boot_med, conn_med[[i]]$basic[4])
      int_max_boot_med<-rbind(int_max_boot_med, conn_med[[i]]$basic[5])
    }

    plot(median_cummult, type = "l", ylab = "irf", xlab = "h", main = nam,
         ylim = c(lim_low_cum,lim_up_cum), col=2)

    polygon( c(x, rev(x)), c(int_min_boot_med, rev(int_max_boot_med)),  col=gray(0.97), border = NA)
    points(median_cummult, type = "l", col=2, lwd=1)
    points(int_max_boot_med, type = "l", lty=2)
    points(int_min_boot_med, type = "l", lty=2)
    abline(0,0, lwd=1)
  }

}


#---------------DATA for STATE-DEPENDENT analyses------------------------------------------

dati<-data.frame()                                                        #data-frame to contain variables for state-dep analysis


#create data-frame with state variables
for (i in 1 : length(noshock)) {
#  dati<-rbind(dati, noshock[[i]][shockT[[i]] - 1,state_variables]) # really pick Tshock-1?
  dati<-rbind(dati, noshock[[i]][shockT[[i]],state_variables])
}

names(dati)<-c(state_names)

#new state variables based on manipulation of other state variables (e.g. ratio between two variables)
dati$N_Q2<-dati$N/dati$Q2
dati$PROF_B<-dati$PIB/dati$GDP
dati$PROF_F<-(dati$PI1 + dati$PI2)/dati$GDP
dati$BANK_W<-dati$NWB/dati$GDP
dati$FIRM_W<-(dati$NW1 + dati$NW2)/dati$GDP


if(length(outliers)>0)
{
  dati<-dati[-c(outliers),]
}

######## Definition of the Metric variable ("dependent" variable used in Random forests algorithms + Wilcoxon tests)

metric<- (cumulate_mult[,17] + cumulate_mult[,18] + cumulate_mult[,19] + cumulate_mult[,20]) /4       #metric to summairze irfs

dati$metric<-metric


#--------------- STATE-DEPENDENT IRF  --------------------------------------------

# selected state variable to be testes

sv<-dati$BDA                                      #state variable 1
sv2<-dati$BANK_W                                  #state variable 2...


##################################

#TREE algorithm to eventually find the best split threshold for the state variable (sv) (based on Metric)

dep<-2                                             #depth of the tree

albe<-ctree(metric~sv, maxdepth=dep)

plot(albe)                                        #look at the tree to chose the splitting treshold

######################################################

threshold<-median(sv)                                 #threshold value to split sample[e,g.  value obtained with the tree alg.]

# data frame containing state-specific irf (state1 and state2)
mult_data_state1<-cumulate_mult_state1<-data.frame()
mult_data_state2<-cumulate_mult_state2<-data.frame()
metric_state1<-metric_state2<-vector()
ind1<-ind2<-1


for (i in 1:nrow(mult_data)) {
  if(sv[i]< threshold)                                                        #rule to split the sample
  {
    mult_data_state1<-rbind(mult_data_state1, mult_data[i,])
    cumulate_mult_state1<-rbind(cumulate_mult_state1, cumulate_mult[i,])
    metric_state1[ind1]<-metric[i]
    ind1<-ind1 + 1
  }
  else
  {
    mult_data_state2<-rbind(mult_data_state2, mult_data[i,])
    cumulate_mult_state2<-rbind(cumulate_mult_state2, cumulate_mult[i,])
    metric_state2[ind2]<-metric[i]
    ind2<-ind2 + 1
  }
}


#-------------IRF state 1 ------------------------------

av_mult_state1<-apply(mult_data_state1,2,mean, trim=tr,na.rm=TRUE)
median_mult_state1<-apply(mult_data_state1,2,median,na.rm=TRUE)

av_cummult_state1<-apply(cumulate_mult_state1,2,mean, trim=tr,na.rm=TRUE)
median_cummult_state1<-apply(cumulate_mult_state1,2,median,na.rm=TRUE)

con_state1<-conn_state1<-con_med_state1<-conn_med_state1<-list()
int_min_boot_state1 <- int_max_boot_state1<-int_min_boot_med_state1 <- int_max_boot_med_state1<-vector()

if(flag_cum==0)
{
  if(flag_median==0)
  {
    for (i in 1: (per-1)) {
      con_state1[[i]]<-boot(data=mult_data_state1[,i], statistic = samplemean, R=rep_boot)
      conn_state1[[i]]<-boot.ci(con_state1[[i]], conf =ci_boot, type="basic")
      int_min_boot_state1<-rbind(int_min_boot_state1, conn_state1[[i]]$basic[4])
      int_max_boot_state1<-rbind(int_max_boot_state1, conn_state1[[i]]$basic[5])
    }

    plot(av_mult_state1, type = "l", ylab = "irf", xlab = "h", main = nam,
         ylim = c(lim_low,lim_up), col=2)

    polygon( c(x, rev(x)), c(int_min_boot_state1, rev(int_max_boot_state1)),  col=gray(0.97), border = NA)
    points(av_mult_state1, type = "l", col=2, lwd=1)
    points(int_max_boot_state1, type = "l", lty=2)
    points(int_min_boot_state1, type = "l", lty=2)
    abline(0,0, lwd=1)

  }

  if(flag_median==1)
  {
    for (i in 1: (per-1)) {
      con_med_state1[[i]]<-boot(data=mult_data_state1[,i], statistic = samplemedian, R=rep_boot)
      conn_med_state1[[i]]<-boot.ci(con_med_state1[[i]], conf =ci_boot, type="basic")
      int_min_boot_med_state1<-rbind(int_min_boot_med_state1, conn_med_state1[[i]]$basic[4])
      int_max_boot_med_state1<-rbind(int_max_boot_med_state1, conn_med_state1[[i]]$basic[5])
    }

    plot(median_mult_state1, type = "l", ylab = "irf", xlab = "h", main = nam,
         ylim = c(lim_low,lim_up), col=2)

    polygon( c(x, rev(x)), c(int_min_boot_med_state1, rev(int_max_boot_med_state1)),  col=gray(0.97), border = NA)
    points(median_mult_state1, type = "l", col=2, lwd=1)
    points(int_max_boot_med_state1, type = "l", lty=2)
    points(int_min_boot_med_state1, type = "l", lty=2)
    abline(0,0, lwd=1)
  }

}


if(flag_cum==1)
{
  if(flag_median==0)
  {
    for (i in 1: (per-1)) {
      con_state1[[i]]<-boot(data=cumulate_mult_state1[,i], statistic = samplemean, R=rep_boot)
      conn_state1[[i]]<-boot.ci(con_state1[[i]], conf =ci_boot, type="basic")
      int_min_boot_state1<-rbind(int_min_boot_state1, conn_state1[[i]]$basic[4])
      int_max_boot_state1<-rbind(int_max_boot_state1, conn_state1[[i]]$basic[5])
    }

    plot(av_cummult_state1, type = "l", ylab = "irf", xlab = "h", main = nam,
         ylim = c(lim_low_cum,lim_up_cum), col=2)

    polygon( c(x, rev(x)), c(int_min_boot_state1, rev(int_max_boot_state1)),  col=gray(0.97), border = NA)
    points(av_cummult_state1, type = "l", col=2, lwd=1)
    points(int_max_boot_state1, type = "l", lty=2)
    points(int_min_boot_state1, type = "l", lty=2)
    abline(0,0, lwd=1)

  }

  if(flag_median==1)
  {
    for (i in 1: (per-1)) {
      con_med_state1[[i]]<-boot(data=cumulate_mult_state1[,i], statistic = samplemedian, R=rep_boot)
      conn_med_state1[[i]]<-boot.ci(con_med_state1[[i]], conf =ci_boot, type="basic")
      int_min_boot_med_state1<-rbind(int_min_boot_med_state1, conn_med_state1[[i]]$basic[4])
      int_max_boot_med_state1<-rbind(int_max_boot_med_state1, conn_med_state1[[i]]$basic[5])
    }

    plot(median_cummult_state1, type = "l", ylab = "irf", xlab = "h", main = nam,
         ylim = c(lim_low_cum,lim_up_cum), col=2)

    polygon( c(x, rev(x)), c(int_min_boot_med_state1, rev(int_max_boot_med_state1)),  col=gray(0.97), border = NA)
    points(median_cummult_state1, type = "l", col=2, lwd=1)
    points(int_max_boot_med_state1, type = "l", lty=2)
    points(int_min_boot_med_state1, type = "l", lty=2)
    abline(0,0, lwd=1)
  }

}



#-------------IRF state 2 ------------------------------

av_mult_state2<-apply(mult_data_state2,2,mean, trim=tr,na.rm=TRUE)
median_mult_state2<-apply(mult_data_state2,2,median,na.rm=TRUE)

av_cummult_state2<-apply(cumulate_mult_state2,2,mean, trim=tr,na.rm=TRUE)
median_cummult_state2<-apply(cumulate_mult_state2,2,median,na.rm=TRUE)

con_state2<-conn_state2<-con_med_state2<-conn_med_state2<-list()
int_min_boot_state2 <- int_max_boot_state2<-int_min_boot_med_state2 <- int_max_boot_med_state2<-vector()

{
  if(flag_median==0)
  {if(flag_cum==0)

    for (i in 1: (per-1)) {
      con_state2[[i]]<-boot(data=mult_data_state2[,i], statistic = samplemean, R=rep_boot)
      conn_state2[[i]]<-boot.ci(con_state2[[i]], conf =ci_boot, type="basic")
      int_min_boot_state2<-rbind(int_min_boot_state2, conn_state2[[i]]$basic[4])
      int_max_boot_state2<-rbind(int_max_boot_state2, conn_state2[[i]]$basic[5])
    }

    plot(av_mult_state2, type = "l", ylab = "irf", xlab = "h", main = nam,
         ylim = c(lim_low,lim_up), col=2)

    polygon( c(x, rev(x)), c(int_min_boot_state2, rev(int_max_boot_state2)),  col=gray(0.97), border = NA)
    points(av_mult_state2, type = "l", col=2, lwd=1)
    points(int_max_boot_state2, type = "l", lty=2)
    points(int_min_boot_state2, type = "l", lty=2)
    abline(0,0, lwd=1)

  }

  if(flag_median==1)
  {
    for (i in 1: (per-1)) {
      con_med_state2[[i]]<-boot(data=mult_data_state2[,i], statistic = samplemedian, R=rep_boot)
      conn_med_state2[[i]]<-boot.ci(con_med_state2[[i]], conf =ci_boot, type="basic")
      int_min_boot_med_state2<-rbind(int_min_boot_med_state2, conn_med_state2[[i]]$basic[4])
      int_max_boot_med_state2<-rbind(int_max_boot_med_state2, conn_med_state2[[i]]$basic[5])
    }

    plot(median_mult_state2, type = "l", ylab = "irf", xlab = "h", main = nam,
         ylim = c(lim_low,lim_up), col=2)

    polygon( c(x, rev(x)), c(int_min_boot_med_state2, rev(int_max_boot_med_state2)),  col=gray(0.97), border = NA)
    points(median_mult_state2, type = "l", col=2, lwd=1)
    points(int_max_boot_med_state2, type = "l", lty=2)
    points(int_min_boot_med_state2, type = "l", lty=2)
    abline(0,0, lwd=1)
  }

}


if(flag_cum==1)
{
  if(flag_median==0)
  {
    for (i in 1: (per-1)) {
      con_state2[[i]]<-boot(data=cumulate_mult_state2[,i], statistic = samplemean, R=rep_boot)
      conn_state2[[i]]<-boot.ci(con_state2[[i]], conf =ci_boot, type="basic")
      int_min_boot_state2<-rbind(int_min_boot_state2, conn_state2[[i]]$basic[4])
      int_max_boot_state2<-rbind(int_max_boot_state2, conn_state2[[i]]$basic[5])
    }

    plot(av_cummult_state2, type = "l", ylab = "irf", xlab = "h", main = nam,
         ylim = c(lim_low_cum,lim_up_cum), col=2)

    polygon( c(x, rev(x)), c(int_min_boot_state2, rev(int_max_boot_state2)),  col=gray(0.97), border = NA)
    points(av_cummult_state2, type = "l", col=2, lwd=1)
    points(int_max_boot_state2, type = "l", lty=2)
    points(int_min_boot_state2, type = "l", lty=2)
    abline(0,0, lwd=1)

  }

  if(flag_median==1)
  {
    for (i in 1: (per-1)) {
      con_med_state2[[i]]<-boot(data=cumulate_mult_state2[,i], statistic = samplemedian, R=rep_boot)
      conn_med_state2[[i]]<-boot.ci(con_med_state2[[i]], conf =ci_boot, type="basic")
      int_min_boot_med_state2<-rbind(int_min_boot_med_state2, conn_med_state2[[i]]$basic[4])
      int_max_boot_med_state2<-rbind(int_max_boot_med_state2, conn_med_state2[[i]]$basic[5])
    }

    plot(median_cummult_state2, type = "l", ylab = "irf", xlab = "h", main = nam,
         ylim = c(lim_low_cum,lim_up_cum), col=2)

    polygon( c(x, rev(x)), c(int_min_boot_med_state2, rev(int_max_boot_med_state2)),  col=gray(0.97), border = NA)
    points(median_cummult_state2, type = "l", col=2, lwd=1)
    points(int_max_boot_med_state2, type = "l", lty=2)
    points(int_min_boot_med_state2, type = "l", lty=2)
    abline(0,0, lwd=1)
  }

}


#statistics and wilcoxon test to test significtivity of the difference across states

av_metric_state1<-mean(metric_state1, trim=tr)
av_metric_state2<-mean(metric_state2, trim=tr)

wt<-wilcox.test(metric_state1, metric_state2)



###graph for comparing state-dependent cumulative irf##########################

leg<-c("low mark-up", "normal/high mark-up")               #legend for the graph

#if no cumulative irf, the code calculate
if(flag_cum==0 & flag_cum_statedep==1){

  con_state1<-conn_state1<- con_state2<-conn_state2<-list()
  int_min_boot_state1 <- int_max_boot_state1<-int_min_boot_state2 <- int_max_boot_state2<-vector()

  for (i in 1: (per-1)) {
    con_state1[[i]]<-boot(data=cumulate_mult_state1[,i], statistic = samplemean, R=rep_boot)
    conn_state1[[i]]<-boot.ci(con_state1[[i]], conf =ci_boot, type="basic")
    int_min_boot_state1<-rbind(int_min_boot_state1, conn_state1[[i]]$basic[4])
    int_max_boot_state1<-rbind(int_max_boot_state1, conn_state1[[i]]$basic[5])
    con_state2[[i]]<-boot(data=cumulate_mult_state2[,i], statistic = samplemean, R=rep_boot)
    conn_state2[[i]]<-boot.ci(con_state2[[i]], conf =ci_boot, type="basic")
    int_min_boot_state2<-rbind(int_min_boot_state2, conn_state2[[i]]$basic[4])
    int_max_boot_state2<-rbind(int_max_boot_state2, conn_state2[[i]]$basic[5])
  }


}

col1<-adjustcolor(col="green", alpha.f = 0.3)
col2<-adjustcolor(col="blue", alpha.f = 0.3)

if(flag_cum_statedep==1)
{
    plot(av_cummult_state1, type = "l", ylab = "C-irf", xlab = "h", main = nam,
     ylim = c(lim_low_cum, lim_up_cum), col=3)

   points(av_cummult_state1, type = "l", col=3, lwd=1)
   polygon( c(x, rev(x)), c(int_min_boot_state1, rev(int_max_boot_state1)),  col=col1, border = NA)
   points(av_cummult_state1, type = "l", col=3, lwd=1)
   points(int_max_boot_state1, type = "l", lty=2, col=3)
   points(int_min_boot_state1, type = "l", lty=2, col=3)

   polygon( c(x, rev(x)), c(int_min_boot_state2, rev(int_max_boot_state2)),  col=col2, border = NA)
   points(av_cummult_state2, type = "l", col=4, lwd=1)
   points(int_max_boot_state2, type = "l", lty=2, col=4)
   points(int_min_boot_state2, type = "l", lty=2, col=4)
   abline(0,0, lwd=1)

   legend("bottomleft" , legend = leg, fill = c(col1, col2))

}



#--------- RANDOM FOREST for DETECTION of STATES --------------------------

#parameters

de<-1                         #depth of the trees
numb<-1000                    #number of the trees
sig<-0.05                     # significativity of the difference
effect<-1                     # 1: C-Irfs are lower in value; 2: C-Irfs are higher in value
nvar<-2                       #number of state-def variable randomly selected to split the sample in a specficic node
nmin<-30L                     #minimum number of observation allowed in a final node
n_states<-8                   #number of most frequent states to focus on


###########################

#dataframe in quantile: Subset of state-defining variable to include in the algoritmh and transform them in quatile
#(the transformation have no effect on the trees, it just simplify the ex-post disctretization)

ddd<-data.frame(metric, rank(dati$BDA)/nrow(dati), rank(dati$PROF_F)/nrow(dati), rank(dati$BANK_W)/nrow(dati),
                rank(dati$UTIL)/nrow(dati),  rank(dati$MARKUP)/nrow(dati), rank(dati$R)/nrow(dati))

names(ddd)<-c("Y","A","B","C","D","E", "G")                            #other variables have to be named as H, I....

form<- Y ~ A + B + C + D + E + G                                         #formula used in the trees



#### usefull objects

pattern<-data.frame(matrix(0, ncol=6, nrow=numb*4))                      #dataframe to store results (i.e. splitting paths = states)

names(pattern)<-c("Var1","Split1","Var2","Split2","Var3","Split3")


indic<-0
cmin<-c("<")
cmaj<-c(">")


############# algorithm to find states ######################

for (rf in 1: numb) {

  #bootrsrapped sample named dat
  bo<-sample(1:nrow(ddd), nrow(ddd), replace=T)                #bootstrapped sample di nome dat
  dat<-data.frame()

  for (p in 1 : length(bo)) {
    dat<-rbind(dat, ddd[bo[p],])
  }


  #fit the tree on the sample dat
  albe1<-ctree(formula = form, data = dat, maxdepth=de, mtry=nvar, alpha=1, minbucket = nmin)


  nodi<-vector()

  nodi<-nodeids(albe1, terminal = TRUE)                                   ## find final nodes

  for (i in 1 : length(nodi)) {                                           # for every final node
    j<-nodi[i]                                                            # node id

    node_data<-as.data.frame(albe1[j]$data)                               #data in the node
    node_ob<-row.names(node_data)                                         #row names of the data in the node

    y_node<-dat[c(node_ob),1]                                            #y values in the node j

    y_no_node<-dat[!(row.names(dat) %in% node_ob),1]                     #y values not in the node j

    av_node<-mean(y_node)                                                #y mean in the node j
    av_no_node<-mean(y_no_node)                                          # y mean outside the node j

    tes<-wilcox.test(y_node, y_no_node)                                  #Wilcoxon test comparing y values in j and not in j
    pv<-tes$p.value                                                      #save pv values for the wilcoxon test


    #if condition to decide whether to consider the splitting path to reach j (economic state of node j) as interesting or not.
    #3 criteria:
    #1) is the sign of the difference the one we are looking for?
    #2) is this difference statistically significant?
    #3) is this diffence due to node j or to what happnes outside node j? Basically, if the sample size of j is high,
    #this difference is not due to j but (with the opposite sign) by what is happening outside j. Indeed, by running the same
    #algortih changing the sign of the difference we are looking for, we exactly recvor this opposite state

    save_node<-FALSE

    if(effect==1)
      save_node<-(av_node<av_no_node & pv<sig & length(y_node)<length(y_no_node))

    if(effect==2)
      save_node<-(av_node>av_no_node & pv<sig & length(y_node)<length(y_no_node))


    if(save_node){
      a<-partykit:::.list.rules.party(albe1,j)                       #find the split path to reach node j

      A_pos<-unlist(gregexpr("A",a))                                #find whether variable A is used in split and in which position
      B_pos<-unlist(gregexpr("B",a))                                #the same for var B...
      C_pos<-unlist(gregexpr("C",a))
      D_pos<-unlist(gregexpr("D",a))
      E_pos<-unlist(gregexpr("E",a))
      G_pos<-unlist(gregexpr("G",a))
      H_pos<-unlist(gregexpr("H",a))
      I_pos<-unlist(gregexpr("I",a))
      J_pos<-unlist(gregexpr("J",a))
      K_pos<-unlist(gregexpr("K",a))

      #delete negative positions which are not really used in the split [gregexpr results in -1 when thevariable is not founded]
      A_pos<-A_pos[A_pos>0]
      B_pos<-B_pos[B_pos>0]
      C_pos<-C_pos[C_pos>0]
      D_pos<-D_pos[D_pos>0]
      E_pos<-E_pos[E_pos>0]
      G_pos<-G_pos[G_pos>0]
      H_pos<-H_pos[H_pos>0]
      I_pos<-I_pos[I_pos>0]
      J_pos<-J_pos[J_pos>0]
      K_pos<-K_pos[K_pos>0]

      #recreating the splitting path to be stored in pattern
      sp<-c(A_pos, B_pos, C_pos, D_pos,E_pos, G_pos, H_pos, I_pos, J_pos, K_pos)
      sp<-sort(sp)

      indic<-indic+1

      for (z in 1 : length(sp)) {

        pattern[indic,(z*2)-1]<-substr(a,sp[z],sp[z])

        if(substr(a,sp[z]+2,sp[z]+2) %in% cmin)
          pattern[indic,(z*2)]<- - as.numeric(substr(a,sp[z]+4,sp[z]+9))

        if(substr(a,sp[z]+2,sp[z]+2) %in% cmaj)
          pattern[indic,(z*2)]<-as.numeric(substr(a,sp[z]+4,sp[z]+8))
      }

    }

  }

}

######### states founded, now discretize them #########

#discretize at decile levels
pattern_rou2<-round(pattern[,2], digits = 1)
pattern_rou4<-round(pattern[,4], digits = 1)
pattern_rou6<-round(pattern[,6], digits = 1)

#dataframe containing discretized states
pattern_rou<-data.frame(pattern[,1],pattern_rou2,pattern[,3],pattern_rou4,pattern[,5],pattern_rou6)

#transform into a vector of "names"
ttt<-vector()
for (m in 1:  nrow(pattern_rou)) {
  ttt[m]<-paste(pattern_rou[m,1], as.character(pattern_rou[m,2]), "|", pattern_rou[m,3], as.character(pattern_rou[m,4]),
               "|", pattern_rou[m,5], as.character(pattern_rou[m,6]), sep = "")
}

resul<-table(ttt)                                                           #aggregate same states togheter

resul_new<-resul[-1]                                                        #eliminate first state which contains all zeros


resul_freq<-round(resul_new/sum(resul_new), digits = 3)                     #relative frequence of states

states_tree<-data.frame(resul_freq)                                         #dataframe with all states and their frequency

names(states_tree)<-c("State", "Frequency")

states_tree<-states_tree[order(states_tree$Frequency, decreasing = TRUE),]

states_tree_most<-states_tree[c(1:n_states),]                                #dataframe with "n_states" more freq states

states_tree_most




#------- SENSITIViTY FOR STATE-DEFINING VARIABLES (Random Forest) -----------------------

#setup

repl<-10000                                 #number of tree
n_s<-1                                      #minimum size of the final node

form_rfs<- metric ~ BDA + GROWTH + MARKUP + UTIL + DEF_GDP + FIRM_W + BANK_W + PROF_F + PROF_B + INFL + R  #formula used in the random forest


##estimation

foresta<-randomForest(formula = form_rfs ,  data = dati, ntree = repl,
                      forest=TRUE, keep.forest = TRUE, do.trace = FALSE, keep.inbag = TRUE,
                      predict.all = TRUE,importance=TRUE, nodesize=n_s)

#plot

impor<-as.data.frame(importance(foresta, type=1))

impor<- cbind(vars=rownames(impor), impor)

impor<-impor[order(impor$`%IncMSE`),]

impor$vars = factor(impor$vars, levels=unique(impor$vars))

barplot(impor$`%IncMSE`, names.arg=impor$vars, ylab = "Mean Decrease Accuracy")


# Close plot file
dev.off( )
