<?php

/*
 * Copyright (C) 2021 Marcelo C. Pereira <mcper at unicamp.br>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// server execution settings
$nice_enable = true;            // enable lower priority for simulation run
$nice_level = 10;               // priority level (unix) used to run simulation
$max_run = 3;                   // maximum simultaneous simulations on server
$max_par_run = 4;               // maximum parallel executions per simulation
$max_thr_run = 1;               // maximum threads to use per execution
$max_time = 30;                 // maximum execution time per simulation (minutes)
$sleep_interval = 1;            // status update interval (seconds)
$csv_exec_config = false;       // executable reads .csv config. (instead .lsd)

// contact e-mail addresses
$lwi_admin = "lwi_admin@localhost";     // email for contact messages about model
$subject = "LWI user feedback";         // contact message subject text
$reply_to = "webmaster@localhost";      // web server administrator

// other settings
$NA = "NA";             // string used for "not available" values in .csv files

// writeable locations (web server must be able to write in those)
$output_pref = "tmp/";          // prefix to output results files
$config_pref = "../tmp/";       // prefix to intermediate configuration files
$flag_pref = "../tmp/";         // prefix to execution flag files (server control)

// LSD file names (relative to index.php path)
$lsd_exec = "../lsdNW";             // LSD getlimits executable (no extension)
$lsd_config = "../lwi.lsd";         // LSD model configuration file to use
$sa_config = "../lwi.sa";           // LSD model parameter-range file to use
$limits_exec = "../lsd_getlimits";  // LSD getlimits executable (no extension)
$saved_exec = "../lsd_getsaved";    // LSD getsaved executable (no extension)
$confgen_exec = "../lsd_confgen";   // LSD confgen executable (no extension)
$mcstats_exec = "../lsd_mcstats";   // LSD mcstats executable (no extension)

// os-specific settings
if ( strtoupper( substr( PHP_OS, 0, 3 ) ) === "WIN" ) {
    $os = "windows";
    $lsd_exec .= ".exe";
    $limits_exec .= ".exe";
    $saved_exec .= ".exe";
    $confgen_exec .= ".exe";
    $mcstats_exec .= ".exe";
    $nice_enable = false;
} elseif  ( strtolower( substr( PHP_OS, 0, 6 ) ) === "darwin" ) {
    $os = "mac";
#    $lsd_exec .= "OSX";
    $limits_exec .= "OSX";
    $saved_exec .= "OSX";
    $confgen_exec .= "OSX";
    $mcstats_exec .= "OSX";
} else {
    $os = "linux";
}

