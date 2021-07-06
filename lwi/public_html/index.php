<?php require '../script.php'; ?>
<!DOCTYPE html>
<!--
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
-->
<html>
    <head>
        <title>LSD Web Interface</title>
        <meta http-equiv="Content-Security-Policy" content="default-src 'self'; script-src 'self' 'unsafe-inline'; style-src 'self' 'unsafe-inline'">
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <link rel="icon" href="favicon.ico">
        <link rel="stylesheet" href="w3.css">
        <link rel="stylesheet" href="lwi.css">
        <!-- PHP & JS functions -->
        <?php 
            check_config( );
        ?>
        <script defer="true" src="modernizr.js"></script>
        <script defer="true" src="script.js"></script>
        <noscript>Error: Javascript scripting is disabled</noscript>
    </head>
    <body onload="check_html5( )">
        <div id="blackout"></div>
        <!-- Sidebar/menu -->
        <nav class="w3-sidebar w3-blue w3-collapse w3-top w3-large w3-padding" style="z-index: 3; width: 210px; font-weight: bold;" id="mySidebar">
            <br>
            <div class="w3-container">
                <!--<img alt="" class="auto-style1" src="lsd.png" />-->
                <img alt="" class="auto-style1" src="lsd.png" style="width:129px; height:89px;" />
                <h3 class="w3-padding-16">
                    
<!-- ### LWI ### ADD THE LONG NAME OF YOUR MODEL HERE -->
                    <b>LSD Model</b>
<!-- ### LWI ### DON'T CHANGE FROM HERE -->
                    
                </h3>
            </div>
            <div class="w3-bar-block">
                <a href="#" onclick="w3_close()" class="w3-bar-item w3-button w3-hover-white">Welcome</a>
                <a href="#configuration" onclick="w3_close()" class="w3-bar-item w3-button w3-hover-white">Configuration</a>
                <a href="#execution" onclick="w3_close()" class="w3-bar-item w3-button w3-hover-white">Execution</a>
                <a href="#analysis" onclick="w3_close()" class="w3-bar-item w3-button w3-hover-white">Analysis</a>
                <a href="#export" onclick="w3_close()" class="w3-bar-item w3-button w3-hover-white">Export</a>
                <a href="#reset" onclick="w3_close()" class="w3-bar-item w3-button w3-hover-white">Reset</a>
                <a href="#help" onclick="w3_close()" class="w3-bar-item w3-button w3-hover-white">Help</a>
                <a href="#contact" onclick="w3_close()" class="w3-bar-item w3-button w3-hover-white">Contact</a>
            </div>
        </nav>

        <!-- Top menu on small screens -->
        <header class="w3-container w3-top w3-hide-large w3-blue w3-xlarge w3-padding">
            <a href="javascript:void(0)" class="w3-button w3-blue w3-margin-right" onclick="w3_open()">☰</a>
            
<!-- ### LWI ### ADD THE SHORT NAME OF YOUR MODEL HERE -->
            <b>LSD Model</b>
<!-- ### LWI ### DON'T CHANGE FROM HERE -->

        </header>

        <!-- Overlay effect when opening sidebar on small screens -->
        <div class="w3-overlay w3-hide-large" onclick="w3_close()" style="cursor: pointer" title="close side menu" id="myOverlay"></div>

        <!-- !PAGE CONTENT! -->
        <div class="w3-main" style="max-width: 1200px; margin-left: 220px; margin-right: 10px">

            <!-- Welcome -->
            <div class="w3-container w3-card-2 w3-margin-bottom" style="margin-top: 10px" id="welcome">
                <div class="w3-hide-large">
                    <h1><br></h1>
                </div>
                <h1 class="w3-xxxlarge w3-text-blue"><b>Welcome</b></h1>
                <div class="w3-container" style="margin-top: 30px">
                    
<!-- ### LWI ### ADD YOUR WELCOME MESSAGE HERE -->
                    <p>This is LSD Web Interface (LWI). LWI makes a basic subset of LSD tools available to web browser users, allowing the configuration, execution, and analysis of results produced by LSD models. LWI user front-end is compatible with any computer platform equipped with a modern HTML5 browser.</p>
                    <p>If you are interested in learning more about LSD or download a full copy of it, please check the <a href="http://www.labsimdev.org" title="LSD" target="_blank" class="w3-hover-opacity">LSD website</a>.</p>
                    <p>LWI requires that users are familiar with the model being simulated. The user is supposed to have some knowledge of the model structure and its main parameters and variables to fully profit from LWI.</p>
                    <p>LWI is organized in several interaction cards, available below. The user can scroll across the cards or use the Navigation panel (if available) to reach the cards directly. When initiating a new LWI session, the Navigation Panel is presented together with the Welcome card. In devices with a narrow screen, like smartphones, the user has to click in the “☰” symbol to open the Navigation panel. Help buttons in all cards leads the user to the associated content on the help card.</p>
<!-- ### LWI ### DON'T CHANGE FROM HERE -->
                    
                </div>
            </div>

            <!-- Configuration -->
            <div class="w3-container w3-card-2 w3-margin-bottom" id="configuration">
                <h1 class="w3-xxxlarge w3-text-blue"><b>Configuration</b></h1>
                <form id="config">
                    <div class="w3-container" style="margin-top: 30px">
                        <h2 class="w3-xxlarge w3-text-blue">Parameters</h2>
                        <table class="w3-table w3-striped w3-white">
                            <col style='min-width:15%'>
                            <col style='max-width:50%'>
                            <col style='width:10%'>
                            <col style='width:10%'>
                            <col style='width:12%'>
                            <col style='width:10%'>
                            <thead>
                                <td><em>Parameter</em></td>
                                <td><em>Description</em></td>
                                <td><em>Type</em></td>
                                <td><em>Range</em></td>
                                <td><em>Value</em></td>
                            </thead>
                            <?php read_config( "parameter" ); ?>
                        </table>
                    </div>
                    <div class="w3-container" style="margin-top: 30px">
                        <h2 class="w3-xxlarge w3-text-blue">Simulation settings</h2>
                        <table class="w3-table w3-striped w3-white">
                            <col style='min-width:15%'>
                            <col style='max-width:50%'>
                            <col style='width:10%'>
                            <col style='width:10%'>
                            <col style='width:12%'>
                            <col style='width:10%'>
                            <thead>
                                <td><em>Parameter</em></td>
                                <td><em>Description</em></td>
                                <td><em>Type</em></td>
                                <td><em>Range</em></td>
                                <td><em>Value</em></td>
                            </thead>
                            <tr>
                                <td><b>timeSteps</b></td>
                                <td>Number of time steps to perform the simulation</td>
                                <td>integer</td>
                                <td>10-10000</td>
                                <td>
                                    <input name="timeSteps" id="_timeSteps_" class="w3-input w3-border" type="number" min="10" max="10000" value="100" required></td>
                                <td>
                                    <button onclick="document.getElementById( '_timeSteps_' ).value = '100'; return false;" class="w3-button w3-blue w3-hover-black">Reset</button></td>
                            </tr>
                            <tr>
                                <td><b>numRuns</b></td>
                                <td>Number of times to repeat the simulation (Monte Carlo experiment)</td>
                                <td>integer</td>
                                <td>1-100</td>
                                <td>
                                    <input name="numRuns" id="_numRuns_" class="w3-input w3-border" type="number" min="1" max="100" value="1" required></td>
                                <td>
                                    <button onclick="document.getElementById( '_numRuns_' ).value = '1'; return false;" class="w3-button w3-blue w3-hover-black">Reset</button></td>
                            </tr>
                            <tr>
                                <td><b>rndSeed</b></td>
                                <td>First seed to be used to initialize the pseudorandom number generator</td>
                                <td>integer</td>
                                <td>1-</td>
                                <td>
                                    <input name="rndSeed" id="_rndSeed_" class="w3-input w3-border" type="number" min="1" max="" value="1" required></td>
                                <td>
                                    <button onclick="document.getElementById( '_rndSeed_' ).value = '1'; return false;" class="w3-button w3-blue w3-hover-black">Reset</button></td>
                            </tr>
                        </table>
                    </div>
                    <?php write_config( ); ?>
                </form>
                <div class="w3-container w3-center" style="margin-top: 30px">
                    <button onclick="download_config( ); return false;" class="w3-button w3-blue w3-padding-large w3-margin-right w3-margin-bottom w3-hover-black">Download</button>
                    <button onclick="window.open( 'upload_config.html' ); return false;" class="w3-button w3-blue w3-padding-large w3-margin-right w3-margin-bottom w3-hover-black">Upload</button>
                    <button onclick="reset_config( 'config' ); return false;" class="w3-button w3-blue w3-padding-large w3-margin-right w3-margin-bottom w3-hover-black">Reset All</button>
                    <a href="#help-configuration">
                        <button class="w3-button w3-blue w3-padding-large w3-margin-right w3-margin-bottom w3-hover-black">Help</button>
                    </a>
                </div>
            </div>

            <!-- Execution -->
            <div class="w3-container w3-card-2 w3-margin-bottom" id="execution">
                <h1 class="w3-xxxlarge w3-text-blue"><b>Execution</b></h1>
                <div class="w3-container" style="margin-top: 50px">
                    <div class="w3-row-padding w3-grayscale">
                        <div class="w3-col m6 w3-margin-bottom">
                            <div class="w3-light-grey">
                                <div class="w3-container">
                                    <h3 class="w3-opacity">Status</h3>
                                    <p id="status">Simulation not started</p>
                               </div>
                            </div>
                        </div>
                        <div class="w3-col m6 w3-margin-bottom">
                            <div class="w3-light-grey">
                                <div class="w3-container">
                                    <h3 class="w3-opacity">Elapsed time</h3>
                                    <p id="chrono">0h00min00s</p>
                                </div>
                            </div>
                        </div>
                    </div>
                </div>
                <div class="w3-container w3-center" style="margin-top: 30px">
                    <button onclick="run_sim( ); return false;" class="w3-button w3-blue w3-padding-large w3-margin-right w3-margin-bottom w3-hover-black">Start</button>
                    <button onclick="abort_sim( ); return false;" class="w3-button w3-blue w3-padding-large w3-margin-right w3-margin-bottom w3-hover-black">Abort</button>
                    <button onclick="show_log( ); return false;" class="w3-button w3-blue w3-padding-large w3-margin-right w3-margin-bottom w3-hover-black">Log</button>
                    <a href="#help-execution">
                        <button class="w3-button w3-blue w3-padding-large w3-margin-right w3-margin-bottom w3-hover-black">Help</button>
                    </a>
                </div>
            </div>

            <!-- Analysis -->
            <div class="w3-container w3-card-2 w3-margin-bottom" id="analysis">
                <form action="" name="analysis" method="post" target="_blank">
                    <h1 class="w3-xxxlarge w3-text-blue"><b>Analysis</b></h1>
                    <div class="w3-container" style="margin-top: 30px">
                        <h2 class="w3-xxlarge w3-text-blue">Available time series</h2>
                        <table class="w3-table w3-striped w3-white">
                            <thead>
                                <td><em>Variable</em></td>
                                <td><em>Description</em></td>
                                <td><em>Selected</em></td>
                            </thead>
                            <?php read_saved( ); ?>
                        </table>
                    </div>
                    <div class="w3-container" style="margin-top: 50px">
                        <div class="w3-row-padding w3-grayscale">
                            <div class="w3-col m5 w3-margin-bottom">
                                <div class="w3-row m5 w3-margin-bottom">
                                    <div class="w3-light-grey">
                                        <div class="w3-container">
                                            <h3 class="w3-opacity">Time step range</h3>
                                            <div class="w3-container">
                                                <div class="w3-section">
                                                    <label>Begin</label>
                                                    <input class="w3-input w3-border" type="number" name="_begin_" id="_begin_" min="1" max="1000" value="1" required>
                                                </div>
                                                <div class="w3-section">
                                                    <label>End</label>
                                                    <input class="w3-input w3-border" type="number" name="_end_" id="_end_" min="1" max="1000" value="1000" required>
                                                </div>
                                            </div>
                                        </div>
                                    </div>
                                </div>
                                 <div class="w3-row m5 w3-margin-bottom">
                                   <div class="w3-light-grey">
                                        <div class="w3-container">
                                            <h3 class="w3-opacity">MC plot bands</h3>
                                            <div class="w3-container">
                                                <div class="w3-section">
                                                    <input class="w3-check" type="checkbox" name="_CI_" id="_CI_" value="1" checked>
                                                    <label>Confidence</label>
                                                </div>
                                                <div class="w3-section">
                                                    <input class="w3-check" type="checkbox" name="_MM_" id="_MM_" value="1">
                                                    <label>Max-min</label>
                                                </div>
                                            </div>
                                        </div>
                                    </div>
                                </div>
                            </div>
                            <div class="w3-col m6 w3-margin-bottom">
                                <div class="w3-light-grey">
                                    <div class="w3-container">
                                        <h3 class="w3-opacity">Scale options</h3>
                                        <div class="w3-col m5">
                                            <div class="w3-container">
                                                <p>
                                                    <input onclick="document.getElementById( '_min_' ).disabled = true; document.getElementById( '_max_' ).disabled = true" class="w3-radio" type="radio" name="_auto_" id="_auto_" value="1" checked>
                                                    <label>Auto</label>
                                                </p>
                                                <p>
                                                    <input onclick="document.getElementById( '_min_' ).disabled = false; document.getElementById( '_max_' ).disabled = false" class="w3-radio" type="radio" name="_auto_" id="_auto_" value="0">
                                                    <label>Manual</label>
                                                </p>
                                            </div>
                                        </div>
                                        <div class="w3-col m5">
                                            <div class="w3-container">
                                                <p>
                                                    <input class="w3-radio" type="radio" name="_linear_" id="_linear_" value="1" checked>
                                                    <label>Linear</label>
                                                </p>
                                                <p>
                                                    <input class="w3-radio" type="radio" name="_linear_" id="_linear_" value="0">
                                                    <label>Log</label>
                                                </p>
                                            </div>
                                        </div>
                                        <div class="w3-margin-bottom">
                                            <div class="w3-container">
                                                <div class="w3-section">
                                                    <label>Minimum</label>
                                                    <input class="w3-input w3-border" name="_min_" id="_min_" type="number" value="0.0" step="0.01" required disabled="">
                                                </div>
                                                <div class="w3-section">
                                                    <label>Maximum</label>
                                                    <input class="w3-input w3-border" name="_max_" id="_max_" type="number" value="1.0" step="0.01" required disabled>
                                                </div>
                                            </div>
                                        </div>
                                    </div>
                                </div>
                            </div>
                        </div>
                        <div class="w3-container w3-center" style="margin-top: 30px">
                            <input type="submit" OnClick="document.forms[ 'analysis' ].action = 'show_stats.php'" class="w3-button w3-blue w3-padding-large w3-margin-right w3-margin-bottom w3-hover-black" value="Statistics">
                            <input type="submit" OnClick="document.forms[ 'analysis' ].action = 'show_table.php'" class="w3-button w3-blue w3-padding-large w3-margin-right w3-margin-bottom w3-hover-black" value="Data">
                            <input type="submit" OnClick="document.forms[ 'analysis' ].action = 'show_plot.php'" class="w3-button w3-blue w3-padding-large w3-margin-right w3-margin-bottom w3-hover-black" value="Plot">
                            <a href="#help-analysis">
                                <button type="button" class="w3-button w3-blue w3-padding-large w3-margin-right w3-margin-bottom w3-hover-black">Help</button>
                            </a>
                        </div>
                    </div>
                </form>
            </div>

            <!-- Export -->
            <div class="w3-container w3-card-2 w3-margin-bottom" id="export">
                <h1 class="w3-xxxlarge w3-text-blue"><b>Export</b></h1>
                <div class="w3-container" style="margin-top: 50px">
                    <div class="w3-row-padding w3-grayscale">
                        <div class="w3-col m4 w3-margin-bottom">
                            <div class="w3-light-grey">
                                <div class="w3-container">
                                    <h3 class="w3-opacity">Results status</h3>
                                    <p id="_ready_">
                                        <?php check_results( $session_short_id ); ?>
                                    </p>
                                </div>
                            </div>
                        </div>
                        <div class="w3-col m4 w3-margin-bottom">
                            <div class="w3-light-grey">
                                <div class="w3-container">
                                    <h3 class="w3-opacity">Date & time</h3>
                                    <p id="_date_">
                                        <?php results_date_time( ); ?>
                                    </p>
                                </div>
                            </div>
                        </div>
                        <div class="w3-col m4 w3-margin-bottom">
                            <div class="w3-light-grey">
                                <div class="w3-container">
                                    <h3 class="w3-opacity">Size</h3>
                                    <p id="_size_">
                                        <?php results_size( ); ?>
                                    </p>
                                </div>
                            </div>
                        </div>
                    </div>
                </div>
                <div class="w3-container w3-center" style="margin-top: 30px">
                    <button onclick="download_res( ); return false;" class="w3-button w3-blue w3-padding-large w3-margin-right w3-margin-bottom w3-hover-black">Download</button>
                    <a href="#help-export">
                        <button class="w3-button w3-blue w3-padding-large w3-margin-right w3-margin-bottom w3-hover-black">Help</button>
                    </a>
                </div>
            </div>

            <!-- Reset Session -->
            <div class="w3-container w3-card-2 w3-margin-bottom" id="reset">
                <h1 class="w3-xxxlarge w3-text-blue"><b>Reset</b></h1>
                <p>Press the button below to reset your session. Please note that all unsaved configuration or execution results will be irreversibly lost.</p>
                <div class="w3-container w3-center" style="margin-top: 30px">
                    <button onclick="reset_session( 'All non-saved configurations and results will be lost' ); return false;" class="w3-button w3-blue w3-padding-large w3-margin-right w3-margin-bottom w3-hover-black">Reset Session</button>
                    <a href="#help-reset">
                        <button class="w3-button w3-blue w3-padding-large w3-margin-right w3-margin-bottom w3-hover-black">Help</button>
                    </a>
                </div>
            </div>

            <!-- Help -->
            <div class="w3-container w3-card-2 w3-margin-bottom" id="help">
                <h1 class="w3-xxxlarge w3-text-blue"><b>Help</b></h1>

                <h2 class="w3-xxlarge w3-text-blue" id="help-configuration">Configuration section</h2>
                <p>The Configuration section allows the user to control many of its parameters, initial conditions and general simulation settings. From now on, each set of values to all the required parameters, initial conditions and simulation settings is defined as one model configuration. Every time the user initiates a new LSD web interface (LWI) session, the default model configuration is automatically loaded. From the Configuration section, the user can change any value, load a saved configuration or save the current configuration to her computer.</p>
                <p>Configuration values can be of three types: real (floating point) numbers, integer numbers and discrete options (like yes/no). All values have predefined ranges (maximum and minimums or a set of options) and the user is not allowed to input values outside such ranges (an error message is produced).</p>
                <p>Please note that not all combinations of configuration values may be adequate for the proper model operation. If unexpected results are obtained after changing several values at once, please try to restore some of them to the default configuration (by pressing the Default button besides any changed value) and execute the simulation again.</p>
                <h3 class="w3-xlarge w3-text-blue">Model parameters</h3>
                <p>Model parameters are predefined and fixed values that are used to compute the model equations. For instance, the user can change parameters to modify simulated agent’s behavioural rules or the applicable institutional rules.</p>
                <h3 class="w3-xlarge w3-text-blue">Initial conditions</h3>
                <p>Model initial conditions are the values assumed for variables which lagged values are required by the computation of model equations when the simulation starts.</p>
                <h3 class="w3-xlarge w3-text-blue">Simulation settings</h3>
                <p>Simulation settings are values defining the operation of the simulation. <strong>timeSteps</strong> control the simulation time span. <strong>numRuns</strong> define how many times the simulation is run (repeated), which allows the analysis of the results as a Monte Carlo experiment.  <strong>rndSeed</strong> controls the initialization of the pseudo-random number generator (PRNG). When multiple simulation runs are set, the PRNG seed is increased by one each time.</p>
                <h3 class="w3-xlarge w3-text-blue">Saving the current configuration</h3>
                <p>At any moment while in the Configuration section, the user has the option to save the current configuration. The configuration is always saved to the user’s computer disk/storage as it is not possible to save it in the LWI server. It is important to regularly save any changes made to the default configuration, as any unsaved information is irreversibly lost whenever the user session expires or is reset (by clicking on <strong>Reset Session</strong> in Reset section).</p>
                <p>To save the current configuration, simply click on <strong>Download</strong> in Configuration section. A dialog window will open, asking for the name and the destination folder of the configuration file. Configuration files have CSV (comma-separated values) format and “.csv” extension.</p>
                <h3 class="w3-xlarge w3-text-blue">Loading an existing configuration</h3>
                <p>An existing configuration file, previously saved using the <strong>Download</strong> option, can be loaded at any time, by clicking on <strong>Upload</strong> in Configuration section. A dialog window will ask for the file containing the saved configuration, assuming the default “.csv” extension.</p>
                <h3 class="w3-xlarge w3-text-blue">Resetting all configuration values to the defaults</h3>
                <p>Clicking <strong>Reset All</strong> replaces the current configuration values with the default ones. Be careful to use it if you have configuration changes to be saved, as the existing values are lost. If required, click on <strong>Download</strong> before using this option.</p>
                
                <h2 class="w3-xxlarge w3-text-blue" id="help-execution">Execution section</h2>
                <p>The Execution section controls the execution of the simulation model in the LWI server. Model execution can take from a few seconds to several minutes, according to the selected configuration, in particular the chosen numbers of simulation time steps and runs.</p>
                <h3 class="w3-xlarge w3-text-blue">Starting a simulation run</h3>
                <p>As soon as the user finishes the configuration of the model in the Configuration section, it is possible to start the execution by clicking on <strong>Start</strong> in the Execution section.</p>
                <h3 class="w3-xlarge w3-text-blue">Stopping an unfinished simulation run</h3>
                <p>At any time the user may interrupt the simulation by clicking in <strong>Abort</strong> in the Execution section. Stopping the execution before the Status box shows “Simulation completed” aborts the simulation and no data is saved for analysis.</p>
                <h3 class="w3-xlarge w3-text-blue">Checking the status of a running simulation</h3>
                <p>While the execution is going on, “Simulation running” is show in the Status box in the Execution section, together with the elapsed time. User can execute only one simulation instance at a time.</p>
                <h3 class="w3-xlarge w3-text-blue">Viewing the simulation log</h3>
                <p>If there is any error preventing the simulation execution to complete, the Status box will show “Simulation failed” and the execution will be aborted and no data is saved for analysis. User can click on <strong>Log</strong> in Execution section to check the causes of the failure.</p>

                <h2 class="w3-xxlarge w3-text-blue" id="help-analysis">Analysis section</h2>
                <p>After the execution of the simulation model, when status box in the Execution section shows “Simulation completed”, the produced simulation time series will be available in the Analysis section.</p>
                <h3 class="w3-xlarge w3-text-blue">The time series list</h3>
                <p>The user can select one or more series in time series list to be used when using the command buttons. Clicking once selects the time series and an additional click deselects it.</p>
                <h3 class="w3-xlarge w3-text-blue">The time steps range selector</h3>
                <p>By default the commands in the Analysis section operate over all simulation time steps, from <em>t = 1</em> to the number of time steps defined in the Configuration section. The user has the option to restrict the range of time steps to use in the analysis, by changing the default values in the Selected Time Steps box.</p>
                <h3 class="w3-xlarge w3-text-blue">The scale options</h3>
                <p>The Scale box provide configuration to the scaling of the vertical axis of time series plots. The <strong>Auto</strong> option uses auto scaling to show the entire data range present in the series. The <strong>Manual</strong> option allows the user to select the minimum and maximum values for the vertical axis. The <strong>Log</strong> option uses a logarithmic scale (natural base) for the vertical plot axis, descriptive statistics and data table, instead of a linear one (<strong>Linear</strong> option).</p>
                <h3 class="w3-xlarge w3-text-blue">The Monte Carlo (MC) plot bands</h3>
                <p>The MC-band options provide extra information for multi-run simulation plots. MC-bands are only available when <strong>numRuns</strong>, in Configuration section, is set to more than one run (at least 10 runs are recommended for sensible results). The <strong>Confidence</strong> option add a standard 95% confidence interval band to each series selected to plot. The <strong>Max-min</strong> option a band from the minimum to the maximum values obtained in the set of simulation runs.</p>
                <h3 class="w3-xlarge w3-text-blue">Showing descriptive statistics</h3>
                <p>Clicking on <strong>Statistics</strong> in Analysis section creates a new browser window showing some descriptive statistics for the selected time series, including the mean, the standard deviation, the minimum and maximum values, the number of observations, and the Monte Carlo standard error (when the number of simulation runs is larger than one). If <strong>Log</strong> is selected, log values (natural base) are used in computations.</p>
                <h3 class="w3-xlarge w3-text-blue">Creating data tables</h3>
                <p>Clicking on <strong>Data</strong> in Analysis section creates a new browser window containing a table with the selected time series in the columns and the time step values in the rows. If <strong>Log</strong> is selected, log values (natural base) are presented. At least one and up to 15 series can be selected at a time. However, multiple data windows may be open at any time.</p>
                <h3 class="w3-xlarge w3-text-blue">Plotting time series</h3>
                <p>Clicking on <strong>Plot</strong> in Analysis section creates a new browser window with the selected time series plots. The horizontal axis represents the simulation time steps and the vertical axis, the selected series values. At least one and up to 15 series can be selected at a time. However, multiple data windows may be open at any time.</p>

                <h2 class="w3-xxlarge w3-text-blue" id="help-export">Export section</h2>
                <p>After an LWI simulation is successfully executed, the user can download the entire results data as (compressed) text file(s) in CSV (comma-separated values) format. CSV-formatted files can be easily imported in any numerical analysis software, like spreadsheets or statistical packages. File is compressed (zip format) only when there are results from more than one simulation run.</p>
                <h3 class="w3-xlarge w3-text-blue">Saving the current configuration</h3>
                <p>As soon as simulation execution is finished, the results data file can be downloaded to the user’s computer. It is not possible to permanently save simulation results in the LWI server, so it is important to save relevant results before they are irreversibly lost whenever the user session expires or is reset (by clicking on <strong>Reset Session</strong> in Reset section).</p>
                <p>To download any available results data, simply click on <strong>Download</strong> in Export section. A dialog window will open, asking for the name and the destination folder of the data file(s). Results files have the “.csv” extension by default and may be grouped inside a single compressed file with “.zip” extension.</p>
                <h3 class="w3-xlarge w3-text-blue">The LWI data export format</h3>
                <p>LWI results data files may be zip compressed to allow downloading multiple files. Zip files can be easily decompressed in any platform.</p>
                <p>Uncompressed CSV files are comma-separated text files. Columns contain single variables while lines represent variables values at each time step. However, the first line has a special meaning, being the first time step values located in the second line and so on. The first line contains the columns headers with the names of the variables in each column.</p>
 
                <h2 class="w3-xxlarge w3-text-blue" id="help-reset">Reset section</h2>
                <p>The <strong>Reset Session</strong> button discards any changes made by the user, including configurations, running simulations or results data, and initiates a new LWI session. All configuration values entered by the user is lost. Any executing simulation is aborted and the results, lost.</p>

            </div>

            <!-- Contact -->
            <div class="w3-container w3-card-2 w3-margin-bottom" id="contact">
                <h1 class="w3-xxxlarge w3-text-blue"><b>Contact</b></h1>
                <p>Do you want to ask a question or inform a problem? Please send us a message.</p>
                <form id="message" action="contact.php" method="post" target="_blank">
                    <div class="w3-section">
                        <label>Name</label>
                        <input class="w3-input w3-border" type="text" name="name" id="_name_" required>
                    </div>
                    <div class="w3-section">
                        <label>Email</label>
                        <input class="w3-input w3-border" type="email" name="email" id="_email_" required>
                    </div>
                    <div class="w3-section">
                        <label>Message</label>
                        <textarea class="w3-input w3-border" type="text" name="text" id="_text_" required rows="5"></textarea>
                    </div>
                    <button type="button" onclick="send_and_clear( 'message', '_name_', '_email_', '_text_' ); return false;" class="w3-button w3-block w3-padding-large w3-blue w3-margin-bottom">Send Message</button>
                </form>
            </div>

            <!-- End page content -->
        </div>

        <!-- W3.CSS Container -->
        <div class="w3-main w3-light-grey w3-padding-16" style="max-width: 1220px; margin-left: 210px; padding-right: 30px; padding-left: 30px">

<!-- ### LWI ### ADD YOUR COPYRIGHT LINK HERE -->
            <p class="w3-left">Copyright © 2021 <a href="https://www.labsimdev.org" title="LSD Website" target="_blank" class="w3-hover-opacity">LSD Team</a></p>
<!-- ### LWI ### DON'T CHANGE FROM HERE -->

            <p class="w3-right">Powered by 
                <a href="https://www.chartjs.org/" title="Chart.js" target="_blank" class="w3-hover-opacity">Chart.js</a>, 
                <a href="https://www.w3schools.com/w3css/default.asp" title="W3.CSS" target="_blank" class="w3-hover-opacity">w3.css</a>, 
                <a href="https://www.labsimdev.org" title="LSD" target="_blank" class="w3-hover-opacity">LSD</a>
            </p>
            <p style="margin-top: 70px"><strong>Disclaimer</strong>: The information and views set out in this software are those of the author(s) and do not necessarily reflect the official opinion of the LSD Team. The LSD Team does not guarantee the accuracy of the data provided. Neither LSD Team nor any person acting on the Team’s behalf may be held responsible for the use which may be made of the information contained therein.</p>
            
<!-- ### LWI ### ADD YOUR COPYRIGHT AND WARRANTY MESSAGES HERE -->
            <p><strong>Copyright Notice</strong>: This Model is property of the respective authors. The Model is protected by copyright laws, trademark and design rights. Any unauthorized use of the Model will be considered a violation of the authors’ intellectual property rights. The Model may not be copied, distributed, published or used in any way, in whole or in part, without prior written agreement from the authors, except as otherwise allowed by the agreed Conditions of Use.</p>
            <p><strong>No Warranties</strong>: this site, the associated model and its results are provided “as is” without any express or implied warranty of any kind including warranties of non-infringement of intellectual property or fitness for any particular purpose. In no event shall the authors or its suppliers and partners be liable for any damages whatsoever (including, without limitation, damages for loss of profits or loss of information) arising out of the use of or inability to use the information provided on this site, even if the authors or its suppliers and partners have been advised of the possibility of such damages. Because some jurisdictions prohibit the exclusion or limitation of liability for incidental or consequential damages, some of the above limitations may not apply to you.</p>
<!-- ### LWI ### DON'T CHANGE FROM HERE -->
        
        </div>
    </body>
</html>
