\name{LSDirf-package}
\alias{LSDirf-package}
\alias{LSDirf}
\docType{package}

\title{
\packageTitle{LSDirf}
}

\description{
\packageDescription{LSDirf}
}

\details{
The Counterfactual Monte Carlo (\strong{CMC}) methodology (see note below) is based on the analysis of samples of seed-specific impulse-response functions (IRF's) and cumulative impulse-response functions (CIRF's) of size N. These samples are highly informative about the effects of a shock affecting a simulation running in LSD, with several statistics of interest that can be computed from them.

In particular, assuming that the mean or the median is chosen as the metric to synthesize information included in these samples, robust IRF and CIRF may be obtained by properly combining the N seed-specific IRF's and CIRF's across the different time horizons. These measures represent the mean/median dynamic effect of a designated shock. Confidence intervals can be obtained by bootstrap, thus allowing the analysis of the uncertainty around these effects.

The CMC methodology allows going beyond the linear effects. Eventual state-dependent effects of the shock can be investigated starting from the IRF and CIRF samples, exploiting the heterogeneity in the simulated system conditions of different runs of the CMC experiment.

In particular, in line with the threshold local projections models adopted in several empirical analyses (e.g., Ramey and Zubairy, 2018), the runs of the CMC experiment can be split into alternative states by comparing the value of one or more \emph{state variable} with a (some) specific threshold(s), computed from the realizations of selected variables' time series in the periods before the shock. As in the case of linear estimates, the confidence intervals around these impulse responses can be constructed via bootstrap, which in this case are also very useful to visually assess the significance of any differences in the impulse responses between alternative states. Several standard statistical tests, such as the t-test or the Mann-Whitney U test, can then be applied to better investigate the significance of state-dependent results.

Such state-dependent analysis can be potentially conducted in two ways. The first is by testing the results against a set of relevant and distinctive system states \emph{known} to the researcher (e.g., Auerbach and Gorodnichenko, 2013). The second takes the alternative approach: instead of testing whether specific states significantly impact the effect of the shock, try to find such states from simulated data. A similar target, for example, is at the heart of the recent literature on \emph{optimal policy}, which goal is to find the optimal allocation of the treatment across heterogeneous units (e.g., Kitagawa and Tetenov, 2018; Athey and Wager, 2021). We offer a data-driven heuristic to this aim that helps discover such states. It is named Random Forest State Identification Algorithm (RFSIA) as it adapts the random forest machine learning technique to our goal.

The main intuition behind the RFSIA is to use a random forest classifier to obtain a set of \emph{meaningful} data stratifications to test for state dependency. More precisely, the idea is to test the state dependency in the final nodes of the regression trees produced by the algorithm and then recombine and rearrange this extremely detailed information to obtain a more general sense of which states have a significant impact on the effect of the shock. In particular, to make the output more understandable and bring out the more evident state patterns, the last step of RFSIA is the quantile discretization of the system states. To this aim, we divide each state variable into deciles and replace the threshold values entering each state, and grouping of similar states.


More details on the methodology, and a comprehensive application to a full LSD simulation model, can be found in Amendola and Pereira (2024).
}

\note{
The \strong{CMC} methodology proposes retrieving the causal effect of shocks by directly exploiting the opportunity to observe the counterfactual in a fully controlled experimental setup. Indeed, counterfactuals emerge naturally in agent-based models if two simulation runs characterized by the same values of the parameters, the same initial conditions, and the same seed of the pseudo-random number generator (PRNG) are considered, and a single shock is introduced in one of them. In these specific circumstances, the non-shocked realization acts as a direct counterfactual for the shocked one, and any difference between the shocked and non-shocked output time series can be directly traced back to the shock.

Building on this insight, the effects of shocks in an agent-based model can be rigorously studied based on the following procedure, which allows collecting a sample of size N of the (dynamic) unit treatment effect:

\enumerate{
  \item Defining a shock generating rule (\eqn{sgr \in \{0, 1\}}), which precisely defines the shock to be tested, that is, which and how model variable(s) is(are) shocked. %Regarding the shock timing, in the following, we assume that the shock appears in a specific simulation period, indexed as \eqn{t_e}.

  \item Running two simulation runs with the same values of parameters, initial conditions, and the PRNG seed but enabling the shock (\eqn{sgr=1}) in just one of them.

  \item Computing the difference in the time series of interest between the two scenarios (counterfactual: \eqn{sgr = 0}; shocked scenario: \eqn{sgr = 1}). In particular, two main metrics can be used to quantify the effects of the shock, namely the impulse-response function (IRF) and the cumulative impulse-response function (CIRF). Depending on the specific variable of interest, impulse response functions can be computed in percentage or absolute terms.

  \item Repeating steps 2 and 3 N times by varying the seed of the PRNG, i.e. running an extensive Monte Carlo experiment.
}

The last step is crucial to robustly evaluate the effect of the shock in an agent-based model. Indeed, in these models, impulse responses are expected to vary by varying the seed. Two explanations for the cross-run variability are possible if the shock size is homogeneous across runs under different seeds. The first one lies in the seed-specific after-shock realizations of the stochastic parts of the model, which is expected to impact the propagation of shocks in the model unpredictably. This implies that the transmission of shocks is inevitably affected by some randomness in most agent-based models. The second reason is instead related to the possibility that, in agent-based models, the effects of shocks may depend on the state of the system, i.e., the propagation of the shocks may be \emph{structurally} amplified or weakened depending on the prevailing conditions. As the state of the system is generally different in each run of the CMC experiment (due to the seed-specific pre-shock realizations of the stochastic parts of the model), this \emph{structural effect} is expected to vary between seeds.

As a consequence of the two cross-run variability channels, the results of a single MC experiment based on a specific seed are not informative enough. Instead, an extensive Monte Carlo experiment in needed, which allows for uncovering the distributional properties of such variability and the study of the effects of shocks in a robust way.
}

\author{
\packageAuthor{LSDirf}

Maintainer: \packageMaintainer{LSDirf}
}

\references{
LSD documentation is available at <\url{https://www.labsimdev.org/}>

The latest LSD binaries and source code can be downloaded at <\url{https://github.com/SantannaKS/Lsd/}>.

Amendola, M., Pereira, M. C. (2024) \emph{Linear and state-dependent impulse responses in agent-based models: a new methodology and an economic application}. SSRN pre-print <doi:10.2139/ssrn.4740360>. Available at <\url{https://www.ssrn.com/abstract=4740360}>.

Athey S., Wager, S. (2021) \emph{Policy learning with observational data}. Econometrica
89(1):133-161

Auerbach A., Gorodnichenko, Y. (2013) \emph{Fiscal multipliers in recession and
expansion}. Fiscal Policy After the Financial Crisis, edited by Alberto Alesina and Francesco Giavazzi. University of Chicago Press

Kitagawa, T., Tetenov, A. (2018) \emph{Who should be treated? empirical welfare maximization methods for treatment choice}. Econometrica
86(2):591-616

Ramey, V., Zubairy, S. (2018) \emph{ Government spending multipliers in good times and in bad: evidence from us historical data}. Journal of political economy 126(2):850-901
}

\seealso{
\link[LSDinterface]{LSDinterface-package}, \link[LSDsensitivity]{LSDsensitivity-package}
}

\keyword{package}
\keyword{models}
\keyword{design}
