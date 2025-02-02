# PEST++
Object Oriented Inverse Modeling Software
<br><br><br>
## Overview
The PEST++ software suite includes several stand-alone tools for model-independent (non-intrusive) computer model parameter estimation and uncertainty analysis.  Codes include:

* ``pestpp``: deterministic GLM parameter estimation using "on-the-fly" subspace reparameterization, effectively reproducing the SVD-Assist methodology of PEST without any user intervention

* ``pestpp-gsa``: Global senitivity analysis using either Morris or Sobol

* ``pestpp-swp``: a generic parallel run utility driven by a CSV file of parameter values

* ``pestpp-opt``: chance-constrainted linear programming

* ``pestpp-ies``: iterative ensemble smoother implementation of GLM.

All members of the software suite can be compiled for PC, MAC, or Linux and have several run managers to support parallelization.  precompiled binaries are available in the "exe" folder.

## Recent Updates

<b> update 5 August 2018 </b>: Official support for ``pestpp-opt``, ``pestpp-ies``, and ``pestpp-swp`` has been moved to [https://github.com/jtwhite79/pestpp](https://github.com/jtwhite79/pestpp).  All inquires regarding these codes should raised in this fork.  Windows binaries compiled with Intel C++ are available in this fork as well.

<b> update 4 July 2018 </b>: PESTPP++ version 4.0.0 has been released to support the newly-developed ``pestpp-ies``. A manuscript documenting ``pestpp-ies`` is available here: [https://www.sciencedirect.com/science/article/pii/S1364815218302676](https://www.sciencedirect.com/science/article/pii/S1364815218302676).  Stay tuned for an actual manual to accompany version 4!

<b> update 2 May 2018 </b>: some refactoring is underway.  ``sweep`` has been renamed ``pestpp-swp`` and ``gsa`` has been renamed ``pestpp-gsa``.  Also, the initial version of the new iterative ensemble smoother is avaiable as ``pestpp-ies``.  The basic ``++`` options needed for fine-grained control of ``pestpp-ies`` are listed below.   

<b> update 09/20/2017</b>: the new optimization under uncertainty tool is ready!  A supporting publication is in the works and should be available soon (a link will be posted once it is accepted).  This new tool uses the same control file/template file/instruction file approach as other PEST(++) applications, so applying this tool to your problem should be seamless.  Optional "++" args for tool are available further done this page.

<b>update 01/25/2017</b>: intel C++ builds are avaiable for mac and for windows.  For mac users, these are statically-linked so they do not require compilers to be installed. 

<b>update 11/25/2016</b>: PEST++ version 3.6 is now available. Some of the many enhancements available in 3.6 include:

* a new approach to implementing regularization. Rather than using the standard pest control file parameters such as ``phimlim``, ``fracphim``, etc, we now offer a single pest++ argument, ``++reg_frac()``, that allows users to specify what fraction  of the composite objective function should be regularization penalty. For example, ``++reg_frac(0.5)`` would result in equal parts data misfit and regularization penalty, which results in the *maximum a posteriori* (MAP) parameter estimate. Using ``++reg_frac()`` will result in substantial speed ups during the lambda calculation process

* a new program for sequential linear programming under uncertainty.  ``pestpp-opt`` is a new executable in the PEST++ suite that uses the standard PEST model independent interface to solve a (sequential) linear programming (LP) problem.  ``pestpp-opt`` relies on the COIN-OR Linear Programming (CLP) solver [https://projects.coin-or.org/Clp]((https://projects.coin-or.org/Clp)).  Also, users have the option to use FOSM-based uncertainty estimation in the evaluation of model-based constraints (such as water levels, stream flows, stream flow depletion, etc) so that a risk-based optimal solution can be found.  See below for the required and optional ``++`` arguments needed to apply ``pestpp-opt``.  Two example problems using the ``pestpp-opt`` tool have been added to the ``benchmarks`` dir.  A publication about this tool is in the works.

* global optimization with differential evolution.  We now have a fully-parallel global solver that implements the differential evolution algorithm (DE) integrated into the pest++ executable.  See below for required and optional ``++`` arguments needed to use the DE solver.

* a new randomization-based SVD solver using the implementation of [https://github.com/ntessore/redsvd-h](https://github.com/ntessore/redsvd-h).  This solver is activated using ``++svd_pack(redsvd)``.  Testing shows it to be very efficient for problems for a wide range of problem sizes, especially with judicious use of ``max_sing``.

* upgrade parameter covariance scaling.  Through the ``++parcov_scale_fac()``, pest++ can now scale the normal matrix (J^tQJ) by a user specified parameter covariance matrix.  If no ``++parcov_filename()`` is provided, pest++ will construct a diagonal parameter covariance matrix from the parameter bounds.  This is a relatively new option and needs more testing, but limited testing to date shows that upgrade vectors resulting from a covariance-scaled normal matrix are more in harmony with expert knowledge.

<b>Update 05/26/2016</b>: PEST++ V3 has been officially released.  It supports a number of really cool features, including global sensitivity analyses, and automatic Bayes linear (first-order, second-moment) parameter and forecast uncertainty estimates.  We also have a utility for fully-parallel parametric sweeps from csv-based parameter files, which is useful for Monte Carlo, design of experiments, surrogate construction, etc.  All of these tools are based on the model-independent communication framework of PEST, so if you have a problem already setup, these tools are ready for you!

<b>Update 10/1/2014</b>: recent stable versions of PEST++ implement dynamic regularization, full restart capabilities, additional options for formulating the normal equations, and an iterative SVD algorithm for very-large problems.  Additionally the YAMR run manager has been improved to use threaded workers so that the master can more easily load balance.  
##Latest Report and Documentation
Welter, D.E., White, J.T., Hunt, R.J., and Doherty, J.E., 2015, Approaches in highly parameterized inversion— PEST++ Version 3, a Parameter ESTimation and uncertainty analysis software suite optimized for large environmental models: U.S. Geological Survey Techniques and Methods, book 7, chap. C12, 54 p., <a ref="http://dx.doi.org/10.3133/tm7C12">http://dx.doi.org/10.3133/tm7C12</a>.


## PEST++ References:

Morris, M.D. 1991. "Factorial Sampling Plans for Preliminary Computational Experiments".  Technometrics 33(2)"161-174

Sobol, I.M. (1993), “Sensitivity Estimates for Nonlinear Mathematical Models,” Mathematical 
Modeling and Computation, 1(4):407-414. 

Welter, D.E., Doherty, J.E., Hunt, R.J., Muffels, C.T., Tonkin, M.J., and Schreüder, W.A., 2012, Approaches in highly parameterized inversion—PEST++, a Parameter ESTimation code optimized for large environmental models: U.S. Geological Survey Techniques and Methods, book 7, section C5, 47 p., available only at <a ref="http://pubs.usgs.gov/tm/tm7c5">http://pubs.usgs.gov/tm/tm7c5</a>.

### Related Links:

* <a ref="http://www.pesthomepage.org">http://www.pesthomepage.org </a>
* <a ref="http://wi.water.usgs.gov/models/pestplusplus/">http://wi.water.usgs.gov/models/pestplusplus</a>
* <a ref="http://wi.water.usgs.gov/models/genie/">http://wi.water.usgs.gov/models/genie/ </a>
* <a ref="https://github.com/jtwhite79/pyemu">https://github.com/jtwhite79/pyemu </a>

## Compiling
The master branch includes a Visual Studio 2015 project, as well as makefiles for linux and mac.

## Testing
The benchmarks/ folder contain several test problems of varying problem size which are used to evaluate the performance of various aspects of the PEST++ algorithm and implementation.  

## Dependencies
Much work has been done to avoid additional external dependencies in PEST++.  As currently designed, the project is fully self-contained and statically linked.  

## pestpp arguments
Here is a (more or less) complete list of ``++`` arguments that can be added to the control file
* ``++overdue_resched_fac(1.2)``:YAMR only, if a run is more than <``overdue_resched_fac``> X average run time, reschedule it on available resources
* ``++overdue_giveup_fac(2.0)``:YAMR only, if a run is more than <``overdue_giveup_fac``> X average run time, mark it as failed
* ``++max_n_super(20)``: maximum number of super parameters to use

* ``++super_eigthres(1.0e-8)`` ratio of max to min singular values used to truncate the singular components when forming the super parameter problem

* ``++n_iter_base(1)``:number of base (full) parameter iterations to complete as part of the on-the-fly combined base-parameter/super-parameter iteration process.  A value of -1 results in calculation of the base jacobian and formation of the super parameter problem without any base parameter upgrades, replicating the behavior of the "svd-assist" methodology of PEST

* ``++n_iter_super(4)``: number of super (reduced dimension) parameter iterations to complete as part of the on-the-fly combined base-parameter/super-parameter iteration process

* ``++svd_pack(propack)``: which SVD solver to use.  valid arguments are ``eigen``(jacobi solution), ``propack``(iterative Lanczos solution) and ``redsvd`` (randomized solution).

* ``++lambdas(0.1,1,10,100,1000)``: the values of lambda to test in the upgrade part of the solution process. Note that this base list is augmented with values bracketing the previous iterations best lambda.  However, if a single value is specified, only one lambda will be used.

* ``++lambda_scale_fac(0.9,0.8,0.7,0.5)``: the values to scale each lambda upgrade vector.  This results in a line search along each upgrade vector direction, so that the number of upgrade vectors = len(lambdas) * len(lambda_scale_fac).  To disable, set = 1.0.

* ``++reg_frac(0.1)``: the portion of the composite phi that will be regularization. If this argument is specified, the ``* regularization`` section of the control file is ignored.  For limited testing, values ranging from 0.05 to 0.25 seem to work well.

* ``++base_jacobian(filename)``: an existing binary jacobian file to use for the first iteration

* ``++parcov(filename)``: an ASCII PEST-style matrix file or uncertainty file to use as the prior parameter covariance matrix in FOSM uncertainty calculations and/or normal matrix scaling.  If not specified and a prior is needed, one is constructed on-the-fly from parameter bounds

* ``++uncertainty(true)``:flag to activate or deactivate FOSM-based parameter and (optionally) forecast uncertainty estimation

* ``++forecasts(fore1,fore2...)``:comma separated list of observations to treat as forecasts in the FOSM-based uncertainty estimation

* ``++iteration_summary(true)``:flag to activate or deactivate writing iteration-based CSV files summarizing parameters (<base_case>.ipar), objective function (<base_case>.iobj) and sensitivities (<base_case>.isen), as well as upgrade summary (<base_case>.upg.csv) and a jacobian parameter-to-run_id mapping (<base_case>.rid). 
* ``++jac_scale(true)``: use PEST-style jacobian scaling. Important, but can be costly because it densifies the normal matrix, making SVD take longer.

* ``++upgrade_augment(true)``: augment the values of lambda to test by including the best lambda from the previous iteration, as well as best lambda * 2.0 and best lambda / 2.0.  If ``true``, then additional lambdas will be included by attempting to extend each upgrade vector along the region of parameter space defined by parameter bounds.  If ``false``, then only the vectors listed in the ``++lambda()`` arg will be tested and no extended upgrade will be included.  

* ``++upgrade_bounds(true)``: use additional tricks and upgrades to deal with upgrades that are going out bounds.  If ``true``, can result in substantial phi improvement, but in some cases can produce NaNs in the upgrade vectors.

* ``++hotstart_resfile(mycase.res)``: use an exising residual file to restart with an existing jacobian to forego the initial, base run and jump straight to upgrade calculations (++base_jacobian arg required).

* ``++max_run_fail(4)``:maximum number of runs that can fail before the run manager emits an error.

* ``++mat_inv(jtqj)``: the form of the normal matrix to use in the solution process. Valid values are "jtqj" and "q1/2j".

* ``++der_forgive(true)``: a flag to tolerate run failures during the derivative calculation process

* ``++parcov_scale_fac(0.01)``: scaling factor to scale the prior parameter covariance matrix by when scaling the normal matrix by the inverse of the prior parameter covariance matrix.  If not specified, no scaling is undertaken; if specified, ``++mat_inv`` must be "jtqj".

* ``++condor_submit_file(pest.sub)``: a HTCondor submit file.  Setting this arg results in use of a specialized version of the YAMR run manager where the ``condor_submit()`` command is issued before the run manager starts, and, once a set of runs are complete, the workers are released and the ``condor_rm()`` command is issued.  This specialized run manager is useful for those sharing an HTCondor pool so that during the upgrade calculation process, all workers are released and during upgrade testing, only the required number workers are queued.  As with all things PEST and PEST++, it is up to the user to make sure the relative paths between the location of the submit file, the control file and the instance of PEST++ are in sync.

### pestpp-swp ``++`` arguments
``sweep`` is a utility to run a parametric sweep for a series of parameter values.  Useful for things like monte carlo, design of experiment, etc. Designed to be used with ``pyemu`` and the python pandas library.  Support pestpp-swp, including input instructions, are available at [https://github.com/jtwhite79/pestpp](https://github.com/jtwhite79/pestpp)

### pestpp-opt ``++`` arguments
``pestpp-opt`` is an implementation of sequential linear programming under uncertainty for the PEST-style model-independent interface.  Support pestpp-opt, including input instructions, are available at [https://github.com/jtwhite79/pestpp](https://github.com/jtwhite79/pestpp)

### pestpp-ies ``++`` arguments
``pestpp-ies`` is an implementation of the iterative ensemble smoother GLM algorithm of Chen and Oliver 2012. So far, this tool has performed very well across a range of problems.  It functions without any additional ``++`` arguments. However, several ``++`` arguments can be used to fine-tune the function of ``pestpp-ies``.  Support pestpp-ies, including input instructions, are available at [https://github.com/jtwhite79/pestpp](https://github.com/jtwhite79/pestpp)


### USGS disclaimer

This software has been approved for release by the U.S. Geological Survey (USGS). Although the software has been subjected to rigorous review, the USGS reserves the right to update the software as needed pursuant to further analysis and review. No warranty, expressed or implied, is made by the USGS or the U.S. Government as to the functionality of the software and related material nor shall the fact of release constitute any such warranty. Furthermore, the software is released on condition that neither the USGS nor the U.S. Government shall be held liable for any damages resulting from its authorized or unauthorized use
