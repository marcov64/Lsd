**************************************************************

    LSD 8.0 - May 2022
    written by Marco Valente, Universita' dell'Aquila
    and by Marcelo Pereira, University of Campinas

    Copyright Marco Valente and Marcelo Pereira
    LSD is distributed under the GNU General Public License

**************************************************************

***********
Legal stuff
***********

Copyright (C) 2021 Marcelo C. Pereira <mcper at unicamp.br>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.


******************
Main documentation
******************

LSD Web Interface (LWI) is fully documented for use with LSD models in the LSD documentation. In LSD Model Manager or LSD Browser, open menu Help and choose "LSD Documentation" option.

LWI is NOT installed by the LSD installer. So, after installing LSD, please download the source code at https://github.com/marcov64/LSD and copy the folder named lwi in the downloaded archive (.zip) to the same folder where you installed LSD, making sure the entire subfolder structure below lwi is copied.

It is recommended to first follow the instructions on LSD documentation to setup a test web server (Apache) in your computer, and to configure LWI in it. After that, the Nelson & Winter (N&W) model example should be available if you browse to http://localhost in your computer.

LWI can be also adapted to be used with non-LSD models. More details of how to set LWI in this case is provided in LSD documentation. Before trying to port your non-LSD model to LWI, make sure the basic LWI infrastructure is operating at your web server using the provided model N&W example, and you understand the LWI workflow. We cannot provide further support for LWI usage with non-LSD models.
