\name{LSDirf-package}
\alias{LSDirf}
\docType{package}

\title{
\packageTitle{LSDirf}
}

\description{
\packageDescription{LSDirf}
}

\details{
The Counterfactual Monte Carlo (\strong{CMC}) methodology (see note below) is based on the analysis of samples of seed-specific impulse-response functions (IRF's) and cumulative impulse-response functions (CIRF's) of size N. These samples are extremely informative about the effects of a shock affecting a simulation running in LSD, with several statistics of interest that can be computed from them.

In particular, assuming that the mean or the median is chosen as the metric to synthesize information included in these samples, robust IRF and CIRF may be obtained by combining the N seed-specific IRF's and CIRF's across the different time horizons. These measures offer a representation of the mean/median dynamic effect of a designated shock. Confidence intervals around these mean/median effects can be then obtained by bootstrap, thus allowing the analysis of the uncertainty around these effects.

However, the CMC methodology allows going beyond the linear effects. Eventual state-dependent effects of the shock can, indeed, be investigated starting from the IRF and CIRF samples, exploiting the heterogeneity in the simulated system conditions of different runs of the CMC experiment. As in the case of linear estimates, the confidence intervals around these impulse responses can be constructed via bootstrap, which in this case are also very useful to visually assess the significance of any differences in the impulse responses between alternative states. Several standard statistical tests, such as the t-test or the Mann-Whitney U test, can be then applied to better investigate the significance of state-dependent results.

Summing up, state-dependency can be studied in a robust way through an intuitive procedure that only requires defining a rule to catch the state of interest and then splitting the CMC sample accordingly. The resulting subsamples provide indeed all the information we need to evaluate the effects of the shock in the alternative states of the simulated system. In line with threshold state-dependent empirical analyses (e.g. Auerbach and Gorodnichenko, 2013; Ramey and Zubairy, 2018), the states are identified by comparing the value of one or more state variables with specific thresholds, computing the former as realizations of selected time series in the period(s) before the shock.

The CMC methodology evaluates state-dependency on the assumption that the simulation state in question is known to the researcher. In many cases, however, it may be interesting to adopt the opposite approach, that is: instead of testing whether specific states significantly impact the effect of the shock, try to find such states from simulated data. A similar target, for example, is at the heart of the recent literature on “Optimal Policy”, which goal is to find the optimal allocation of the treatment across heterogeneous units (e.g. Kitagawa and Tetenov, 2018; Athey and Wager, 2020). To this aim, we offer a data-driven heuristic helpful for discovering such states. It is named Random Forest State Identification Algorithm (RFSIA) as it adapts the random forest machine learning technique to our goal.

The main intuition behind the RFSIA is the exploitation of the random forest technique to obtain a series of “meaningful” stratifications of the data on which to test state-dependency. More precisely, the idea is to test the state-dependency in the final nodes of the regression trees and then recombine and rearrange this extremely detailed information to obtain a more general sense of which states have a significant impact on the effect of the shock. However, such state information is extremely detailed and, at times, difficult to interpret. Therefore, to make the output more understandable and bring out the more evident state patterns, the last step of RFSIA is the quantile discretization of the economic states. To this aim, we divide each state variable into deciles and replace the threshold values entering each state, and grouping of similar states.
}

\note{
The \strong{CMC} methodology proposes retrieving the causal effect of shocks by exploiting the opportunity to directly observe the counterfactual in a fully controlled experimental setup. Indeed, counterfactuals emerge naturally in agent-based models if two simulation runs characterized by the same values of the parameters, the same initial conditions and the same seed of the pseudo-random number generator (PRNG) are considered, and a single shock is introduced in one of them1. In these specific circumstances, in particular, the simulation without shock acts as a direct counterfactual for the shocked one, and any difference between shocked and not shocked time series can be directly traced back to the shock. By proposing an analogy with the policy evaluation literature, this experimental setup allows us to observe something similar to the concept of the unit treatment effect, i.e. a very informative metric that cannot be directly observed in empirical analyses since, in reality, each unit is either treated or untreated at any specific time.

Building on this insight, the effects of shocks in agent-based models can be studied through the following procedure:

\enumerate{
  \item Defining a shock (impulse) generating rule (\code{SGR}), which precisely defines the shock to be tested that is: which parameter(s) is(are) shocked and the magnitude, length, timing, and persistence of the shock. Regarding the timing, in the following, we assume that the shock appears in a specific simulation period.

  \item Running two simulation runs with the same values of parameters, initial conditions and seed of the PRNG, and activating the shock generating rule in just one of them (\code{SGR = 1}).

  \item Computing the difference in the time series of interest between the two scenarios (counterfactual: \code{SGR = 0}; shocked scenario: \code{SGR = 1}). In particular, two main metrics can be used to quantify the effects of the shock, namely the impulse-response function (IRF) and the cumulative impulse response function (CIRF). Depending on whether we are interested in percentage or absolute effects - a choice that generally depends on the specific variable of interest.

  \item Repeating steps 2 and 3 N times by varying the seed of the PRNG, i.e. running an extensive Monte Carlo experiment.
}

The last step is crucial to robustly evaluate the effect of the shock in an agent-based model. Indeed, in these models, impulse responses are expected to vary by varying the seed. Two reasons may explain this across-seed variability. The first one lies in the “seed-specific after-shock realizations of the stochastic parts of the model”, which is expected to unpredictably impact the propagation of shocks in our synthetic economic system. In agent-based models, therefore, the transmission of shocks is inevitably affected by some randomness. The second reason is instead related to the already emphasized possibility that, in agent-based models, the effects of shocks may depend on the state of the economy, i.e. the propagation of the shocks may be “structurally” amplified or weakened depending on economic conditions. As the state of the economy is generally different in each run of the CMC experiment (due to the “seed-specific pre-shock realizations of the stochastic parts of the model”), this “structural effect” is expected to vary between seeds.

As a consequence of both channels, the results of a single experiment, based on a specific seed, is not enough informative. Instead, what is needed is an extensive Monte Carlo experiment, which allows us to uncover the distributional properties of this variability and study the effects of shocks in a sound and robust way.
}

\author{
\packageAuthor{LSDirf}

Maintainer: \packageMaintainer{LSDirf}
}

\references{
LSD documentation is available at \url{https://www.labsimdev.org/}

The latest LSD binaries and source code can be downloaded at \url{https://github.com/marcov64/Lsd/}.

Athey [INITIALS], Wager [INITIALS] (2020) \emph{[PAPER TITLE]}. [JOURNAL]
[VOL]([ISSUE]):[PAGINI]-[PAGEND]

Auerbach [INITIALS], Gorodnichenko [INITIALS] (2013) \emph{[PAPER TITLE]}. [JOURNAL]
[VOL]([ISSUE]):[PAGINI]-[PAGEND]

Kitagawa [INITIALS], Tetenov [INITIALS] (2018) \emph{[PAPER TITLE]}. [JOURNAL]
[VOL]([ISSUE]):[PAGINI]-[PAGEND]

Ramey [INITIALS], Zubairy [INITIALS] (2018) \emph{[PAPER TITLE]}. [JOURNAL]
[VOL]([ISSUE]):[PAGINI]-[PAGEND]
}

\seealso{
\link[LSDinterface]{LSDinterface-package}, \link[LSDsensitivity]{LSDsensitivity-package}
}

\keyword{package}
\keyword{models}
\keyword{design}
