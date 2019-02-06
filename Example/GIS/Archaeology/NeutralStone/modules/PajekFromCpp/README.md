# PajekFromCpp

Complete rework 10/2018

**Version 0.1** (as of 01/2019)

Stand alone c++ source code to create pajek .net and .paj (including timeline feature for PajekToScgAnim) files from any simulation in c/c++. For the documentation (how to) see the PajekFromCpp_macro.h file. The macros are intended as API, but in general you only need to include PajekFromCpp.cpp.

The additional file "CreateDir.h" provides capbabilities to create directories on linux and windows machines.
See https://stackoverflow.com/a/29828907/3895476 for more information.

The code allows to generate .net (static views) and .paj (dynamic views) output from any simulation model in c++.

This code is generated and tested for the use with LSD by Marco Valente (http://www.labsimdev.org/Joomla_1-3/, https://github.com/marcov64/Lsd), a c++ based framework for agent-based modeling. It is not a substitute to the network functionality implemented by Marcelo Pereira in current snapshots of LSD. It is only ment to be used to export network-data from a simulation in such a manner that it may be analysed and visualised with pajek or other software that can make use of the .paj or .net format.  The intention is to provide full flexibility and all the options that pajek by Andrej Mrvar (http://mrvar.fdv.uni-lj.si/pajek/) .paj (and .net) format provides. In addition, the .paj files created may be used with PajekToSvgAnim by Darko Brvar: - PajekToSvgAnim (by Darko Brvar): See http://mrvar.fdv.uni-lj.si/pajek/ in the section "Supporting programs".

### Usage

See the PajekFromCpp_test.cpp file for a use-case example. Take a look at PajekFromCpp_macro.h for the relevant commands and description thereof.

*If somebody produces a better test model, please let me know so I can add it instead!*

### Misc
The code is provided "as is". You may reach out to the author via E-Mail at: [frederik.schaff@rub.de](frederik.schaff@rub.de)

For further information on Pajek see also:

Mrvar, Andrej; Batagelj, Vladimir (2016): Analysis and visualization of large
networks with program package Pajek. In: Complex Adaptive Systems Modeling 4(1),
 S. 1-8. DOI: 10.1186/s40294-016-0017-8.

Nooy, Wouter de; Mrvar, Andrej; Batagelj, Vladimir (2011): Exploratory social
network analysis with Pajek. Rev. and expanded 2nd ed. England, New York:
Cambridge University Press (Structural analysis in the social sciences, 34).

### License  

The code is distributed under the MIT license, see the attached license file.