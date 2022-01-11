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

require '../defaults.php';

// check if too many instances already running
if ( count( glob( $flag_pref . "run-*.flag" ) ) >= $max_run ) {
    echo "Busy: Cannot execute now, please try later";
    return;
}

$filename = $config_pref . "run-" . $session_short_id . ".csv";

$f = fopen( $filename, "w" );
if ( ! $f ) {
    echo "Error: Cannot write configuration file on server";
    return;
}

fputcsv( $f, array ( "Name", "Value" ) );

$seed = 1;
foreach ( $config as $name => $value ) {
    fputcsv( $f, array ( $name, $value ) );
    if ( $name === "_rndSeed_" ) {
        $seed = $value;
    }
}

fclose( $f );

// creates the LSD configuration, if executable uses it
if ( ! $csv_exec_config ) {
    if ( file_exists( $confgen_exec ) && file_exists( $lsd_config ) ) {
        $filename_conf = $config_pref . "run-" . $session_short_id;
        exec( $confgen_exec . " -f " . $lsd_config . " -c " . $filename . " -o " . $filename_conf, $shell_out, $shell_err );
        unlink( $filename );
        if ( $shell_err != 0 ) {
            echo "Error: lsd_confgen error=" . $shell_err;
            return;
        }
        $filename_conf .= ".lsd";
    } else {
        echo "Error: lsd_confgen or lwi.lsd not reachable";
        return;
    }
} else {
    $filename_conf = $filename;
}

// remove existing output files
array_map( "unlink", glob( $output_pref . "run-" . $session_short_id . "_*.csv" ) );

// kill all sons of unix process, recursively
function kill_recursive( $ppid, $signal ) {
    if ( is_numeric( $ppid ) ) {
        $spids = preg_split( "/\s+/", `pgrep -P $ppid` );
        foreach ( $spids as $spid ) {
            kill_recursive( $spid, $signal );
        }
        posix_kill( $ppid, $signal );
    }
}

// terminate a tree of processes on abort
function proc_tree_terminate( $proc, $signal ) {
    global $os;

    $status = proc_get_status( $proc );

    if ( ! $status[ "running" ] ) {
        return;
    }

    $ppid = $status[ "pid" ];

    if ( $os === "windows" ) {
        if ( $signal === SIGKILL ) {
            exec( "TASKKILL /F /T /PID " . $ppid );
        } else {
            exec( "TASKKILL /T /PID " . $ppid );
        }
    } else {
        kill_recursive( $ppid, $signal );
    }
}

// run simulation in LSD
if ( file_exists( $lsd_exec ) && file_exists( $filename_conf ) ) {
    $filename_log = $output_pref . "run-" . $session_short_id . ".log";
    if ( file_exists( $filename_log ) ) {
        unlink( $filename_log );
    }

    // execute program as a child process
    $descriptorspec = array (
        0 => array ( "pipe", "r" ),                  // stdin
        1 => array ( "file", $filename_log, "a" ),   // stdout
        2 => array ( "file", $filename_log, "a" )    // stderr
    );

    if ( ! $csv_exec_config ) {
        $command = $lsd_exec . " -t -z -p -b -c " . $max_thr_run . ":" . $max_par_run . " -f " . $filename_conf . " -o " . $output_pref;
    } else {
        $command = $lsd_exec . $filename_conf . " " . $output_pref;
    }

    if ( $nice_enable && $os !== "windows" ) {
        $command = "nice -n " . $nice_level . " " . $command;
    }

    if ( $os === "mac" ){
        $command = "exec " . $command;
    }

    $lsdNW = proc_open( $command, $descriptorspec, $pipes );

    if ( ! $lsdNW ) {
        echo "Error: cannot run lsdNW";
        return;
    }

    // flag that task is running
    $filename_flag = $flag_pref . "run-" . $session_short_id . sprintf( "-%02u%02u%02u", getdate( )[ "hours" ], getdate( )[ "minutes" ], getdate( )[ "seconds" ] ) . ".flag";
    touch( $filename_flag );

    // make sure the .abort semaphore file doesn't exist
    $filename_abort = $config_pref . "run-" . $session_short_id . ".abort";
    if ( file_exists( $filename_abort ) ) {
        unlink( $filename_abort );
    }

    $abort = $timeout = false;
    $start = time( );
    while ( $status = proc_get_status( $lsdNW )[ "running" ] && ! $abort && ! $timeout ) {
        sleep( $sleep_interval );

        // abort if semaphore file is present
        if ( file_exists( $filename_abort ) ) {
            proc_tree_terminate( $lsdNW, 15 );
            unlink( $filename_abort );
            $abort = true;
        }

        // abort if running over maximum allowed time
        if ( ( time( ) - $start ) / 60 > $max_time ) {
            $timeout = true;
        }
    }

    // close child process
    fclose( $pipes[ 0 ] );
    fclose( $pipes[ 1 ] );
    fclose( $pipes[ 2 ] );
    proc_close( $lsdNW );

    // remove LSD configuration and grand total results files
    unlink( $filename_flag );
    unlink( $filename_conf );

    if ( $abort ) {
        echo "Aborted: user interruption";
        return;
    } elseif ( $timeout ) {
        echo "Aborted: execution timed out";
        return;
    }

    if ( $status[ "exitcode" ] != 0 ) {
        echo "Aborted: lsdNW error=" . $status[ "exitcode" ];
        return;
    }

    $base_name = $output_pref . "run-" . $session_short_id;
    $filename_res = glob( $base_name . "_*.csv" );

    // creates the MC analysis
    if ( count( $filename_res ) > 1 ) {
        if ( file_exists( $mcstats_exec ) ) {
            exec( $mcstats_exec . " -o " . $base_name . " -f " . implode( " ", $filename_res ), $shell_out, $shell_err );
            if ( $shell_err != 0 ) {
                echo "Aborted: lsd_mcstats error=" . $shell_err;
                return;
            }
        } else {
            echo "Aborted: lsd_mcstats not reachable";
            return;
        }
    }

    echo get_size( $filename_res );
} else {
    echo "Error: lsdNW or " . $filename_conf . "not reachable";
}
