<?php

/* 
 * Copyright (C) 2017 Marcelo C. Pereira <mcper at unicamp.br>
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

include '../filter_config.php';

$session_id = preg_replace( "/[^\da-z]/i", "", filter_input( INPUT_COOKIE, session_name( ), FILTER_SANITIZE_STRING ) );
$session_short_id = substr( $session_id, -6 );

$lsd_config = "../lwi.lsd";
$confgen_exec = "../lsd_confgen";
$lsd_exec = "../lsd_gnuNW";
$nice_enable = true;
$nice_level = 10;
$max_cores = 4;
$sleep_interval = 5;
$os = "linux";


// adjust Windows executable names
if ( strtoupper( substr( PHP_OS, 0, 3 ) ) === "WIN" ) {
    $os = "windows";
    $confgen_exec .= ".exe";
    $lsd_exec .= ".exe";
    $nice_enable = false;
}
// adjust Mac executable names
if ( strtolower( substr( PHP_OS, 0, 6 ) ) === "darwin" ) {
    $os = "mac";
    $confgen_exec .= "OSX";
    $lsd_exec .= "OSX";
    $nice_enable = true;
}

// Check if the form was submitted
if ( filter_input_array( INPUT_SERVER )[ "REQUEST_METHOD" ] === "POST" ) {

    $config = filter_config( json_decode( filter_input_array( INPUT_POST )[ "x" ], false ) );

    if ( count( $config ) == 0 ) {
        echo "Error: No data to configure simulation";
        return;
    }

    $filename = "../tmp/run-" . $session_short_id . ".csv";

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
    
    // creates the LSD configuration
    if ( file_exists( $confgen_exec ) && file_exists( $lsd_config ) ) {
        $filename_conf = "../tmp/run-" . $session_short_id;
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
    
    // run simulation in LSD
    if ( file_exists( $lsd_exec ) && file_exists( $filename_conf ) ) {
        $filename_log = "../tmp/run-" . $session_short_id . ".log";
        if ( file_exists( $filename_log ) ) {
            unlink( $filename_log );
        }
        
        // execute program as a child process
        $descriptorspec = array (
            0 => array ( "pipe", "r" ),                  // stdin
            1 => array ( "file", $filename_log, "a" ),   // stdout
            2 => array ( "file", $filename_log, "a" )    // stderr
        );
        $command = $lsd_exec . " -t -z -c " . $max_cores . " -f " . $filename_conf . " -o tmp";
        if ( $nice_enable && $os !== "windows" ) {
            $command = "nice -n " . $nice_level . " " . $command;
        }
        if ( $os === "mac" ){
            $command = "exec " . $command;
        }
        $lsd_gnu = proc_open( $command, $descriptorspec, $pipes );
        
        if ( ! $lsd_gnu ) {
            echo "Error: cannot run lsd_gnuNW";
            return;
        }
        
        // make sure the .abort semaphore file doesn't exist
        $filename_abort = "../tmp/run-" . $session_short_id . ".abt";
        if ( file_exists( $filename_abort ) ) {
            unlink( $filename_abort );
        }

        $abort = false;
        while ( $status = proc_get_status( $lsd_gnu )[ "running" ] && ! $abort ) {
            sleep( $sleep_interval );
            // abort if semaphore file is present
            if ( file_exists( $filename_abort ) ) {
                switch ( $os ) {
                    
                    case "windows":
                        $pid = proc_get_status( $lsd_gnu )[ "pid" ];
                        // kill parent and all childs
                        exec( "TASKKILL /F /T /PID " . $pid );
                        break;
                    
                    case "mac":
                        // get the parent pid of the process to kill
                        $parent_pid = proc_get_status( $lsd_gnu )[ "pid" ];
                        // use pgrep to get all the children of this process, and kill them
                        $pids = preg_split( "/\s+/", `pgrep -P $parent_pid` );
                        foreach ( $pids as $pid ) {
                            if ( is_numeric( $pid ) ) {
                                posix_kill( $pid, 9 ); // 9=SIGKILL signal
                            }
                        }
                        // then kill parent
                        posix_kill( $parent_pid, 9 ); // 9=SIGKILL signal
                        break;
                        
                    default:
                        proc_terminate( $lsd_gnu );
                }
                unlink( $filename_abort );
                $abort = true;
            }
        }
        
        // close child process
        fclose( $pipes[ 0 ] );
        fclose( $pipes[ 1 ] );
        fclose( $pipes[ 2 ] );
        proc_close( $lsd_gnu );
        
        // remove LSD configuration and grand total results files
        unlink( $filename_conf );
        $filename_total = "tmp/run-" . $session_short_id . "_" . $seed . "_" . $seed . ".csv";
        if ( file_exists( $filename_total ) ) {
            unlink( $filename_total );
        }
        
        if ( $abort ) {
            echo "Aborted";
            return;
        }
        
        if ( $status[ "exitcode" ] != 0 ) {
            echo "Error: lsd_gnuNW error=" . $status[ "exitcode" ];
            return;
        }
        
        $filename_res = "tmp/run-" . $session_short_id . "_" . $seed . ".csv";
        if ( file_exists( $filename_res ) ) {
            echo number_format( filesize( $filename_res ) / 1024, 1 ) . " kB";
        }

    } else {
        echo "Error: lsd_gnuNW or " . $filename_conf . "not reachable";
    }
}
