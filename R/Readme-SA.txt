The LSDsensitivity R package provides tools to analyze simulated experiments from LSD.

The package offers two sensitivity analysis (SA) methods: Morris Elementary Effects (EE) and Sobol Variance Decomposition (SVD). 

EE employs a simple one-factor-at-a-time (OAT) SA and is usually applied as an initial screening method while selecting relevant factors to a SVD global SA. EE requires an appropriate set of sample points (the DoE) which can be generated in LSD when "EE Sampling" is selected in the "Data" menu. Please make sure to take note of the DoE parameters used for the sampling, as they will be required for the configuration of the R analysis script.

Due to its high computational cost, SVD is performed over a meta-model fitted from the experiment data produced by the LSD original model. The meta-model can be fitted using different sampling strategies offered by LSD, being "NOLH Sampling" (Near Orthogonal Latin Hypercube) usually the most efficient. Additionally to the set of samples used to fit the meta-model, it is recommended to also generate another set for the (external) validation of the meta-model.

The package offers two meta-modeling (MM) methods: Kriging and polynomial (first or second order with/without interactions). Kriging is offered under five different variance kernels (Matern 5/2, Matern3/2, Gaussian, exponential and power exponential) and two trend models (constant or first order polynomial) to choose, including auto-selection to the best fitting alternative. Polynomial meta-models of first or second order, with or without interactions, with auto-selection are also offered. Kriging is the recommended option in most cases.

LSD 7.0+ default installation provides example scripts using the LSDsensitivity package. LSD can be downloaded at https://github.com/marcov64/Lsd.

Complete data preparation and analysis steps: 

Below are the minimum required steps to perform SA on a working LSD model using NOLH sampling, Kriging MM and SVD. The changes to perform an EE or to use a polynomial MM are also indicated, as options.

1. Define the parameters/initial values to be explored in the SA, their max/min ranges and the result variables over which the SA is to be done

2. In LMM create a no-window (command prompt) version of your model by selecting menu "Model"/"Create 'No Window' Version"

3. In LSD Browser make sure that all parameters/initial values are set with the correct "calibration"/default values (menu "Data"/"Initial Values"), the required result variables are being saved (menu "Model"/Change Element..."/"Save" checkbox/"Ok" or "Save" in the right mouse button context menu) and the number of MC runs for each SA sample (point) is defined (menu "Run"/"Simulation Settings"/"Number of simulation runs" field, typically set to 10)

4. Save your setup in a baseline .lsd configuration file (menu "File"/"Save As..."), preferably in a new folder inside your current model configuration folder (you can create a new folder while in the "Save As..." dialog box)

5. (Re)load your baseline .lsd configuration if it is not already loaded (menu "File"/"Load...")

6. Choose the ranges (max/min) for each parameter/initial value in your SA exploration space by using the "Sensitivity Analysis" button in the menu "Model"/"Change Element..." window or the same option in the context menu (mouse right-button click on the parameter/variable name in the "Variables & Parameters" listbox)

7. After choosing all ranges, save your exploration space definition as a .sa sensitivity analysis file using the same base name and folder as your .lsd baseline configuration (menu "File", "Save Sensitivity...")

8. With both the created .lsd and .sa files loaded (use menu "File"/"Load..." and "File"/"Load Sensitivity..." if required), select "Data"/"Sensitivity Analysis"/"NOLH Sampling..." and accept the defaults (several new .lsd files will be created in your baseline configuration folder, those are the sample points for the meta-model estimation)

8a. To perform Elementary Effects (EE) analysis instead of Sobol Variance Decomposition, in the step below select "Data"/"Sensitivity Analysis"/"EE Sampling..." instead (NOLH sampling cannot be used for EE)

8b. If a polynomial meta-model (MM) is being estimated, sometimes it is preferred to use "Data"/"Sensitivity Analysis"/"MC Range Sampling..." despite not required

9. Immediately after the previous step, select menu "Data"/"Sensitivity Analysis"/"MC Range Sampling..." and accept the defaults (to create the external validation sample, more .lsd files will be created for the additional sampling points)

9a. EE analysis does not uses external validation, so skip this step for EE

10. Immediately after the previous step select menu "Run"/"Create/Run Parallel Batch", accept using the just created configuration, adjust the number of cores only if going to run in another machine (8 in a modern PC, 20 in a basic server), and decide if you want to start the (time-consuming) processing right now or later (in the current or in another machine)

11. If running later in the same machine, you just have to execute the created script file (.bat or .sh) inside the folder your baseline .lsd file was created

12. If running in another machine, you have to copy the entire model folder and subfolders to the new machine (the remaining LSD folders are not required), recompile LSD for the new platform if required and execute the script file (.bat or .sh) in the same folder as your baseline .lsd file

13. Open R (or RStudio) and check you have the following packages installed and download them if required (if you install LSDsensitivity from CRAN or another online repository, and not from a file, all other dependency packages should be automatically downloaded):

LSDsensitivity, LSDinterface, abind, tseries, car, minqa, nloptr, Rcpp, RcppEigen, lme4, SparseM, MatrixModels, pbkrtest, quantreg, DiceKriging, kSamples, SuppDists, randtoolbox, rngWELL, rgenoud, sensitivity, xts, TTR, quadprog, zoo, quantmod

14. Open the "kriging-sobol-SA.R" example script (included in your LSD installation folder) in RStudio or your text editor

14.a For EE analysis, open "elementary-effects-SA.R" instead

14.b For the use of a polynomial MM for the SVD analysis, open "poly-sobol-SA.R" instead

15. Adjust the vector "lsdVars" to contain all the LSD saved variables you want to use in your analysis (do not include saved but unused variables, for performance reasons), replacing the dummies "varX" 

16. Adjust the vector "logVars" to contain all LSD variables (included in "lsdVars") that require to have the log value used in the analysis (let the vector empty, i.e. "c( )", if no log variable is required)

17. Include in the vector "newVars" any new variable (not included in "lsdVars") that has to be added to the dataset (let the vector empty, i.e. "c( )", if no new variable is required)

18. Adapt the "eval.vars" function to compute any new variable included in "newVars" (use the commented example as a reference)

19. Adjust the arguments to the function "read.doe.lsd" for the relative folder of LSD data files (default is same as R working directory), the data files base name (the file name chosen for the baseline configuration in step 4 without the .lsd suffix) and the name of the variable to be used as reference for the sensitivity analysis (you have to run the script multiple times if there is more than one)

20. Save the modified script, renaming if necessary, and run it in R (or click the "Source" button in RScript), redirecting output to a file first if required
