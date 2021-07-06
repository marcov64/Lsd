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

require '../filter_config.php';
require '../defaults.php';

$session_id = preg_replace( "/[^\da-z]/i", "", filter_input( INPUT_COOKIE, session_name( ), FILTER_SANITIZE_STRING ) );
$session_short_id = substr( $session_id, -6 );

// Check if the form was submitted
if ( filter_input_array( INPUT_SERVER )[ "REQUEST_METHOD" ] === "POST" ) {
    // save configuration to an individual user file for later download
    $config = filter_config( json_decode( filter_input_array( INPUT_POST )[ "x" ], false ) );

    if ( count( $config ) == 0 ) {
        echo "Error\n\nNo data to save.";
        return;
    }

    // download file name for this session
    $filename_base = $output_pref . "config-" . $session_short_id . "-";
    $filename_down = $filename_base . sprintf( "%02u%02u%02u%02u%02u", getdate( )[ "hours" ], getdate( )[ "minutes" ], getdate( )[ "mday" ], getdate( )[ "mon" ], getdate( )[ "year" ] - 2000 ) . ".csv";
    
    // delete previous file in same session
    if ( $old = glob( $filename_base . "*" ) ) {
        foreach ( $old as $file ) {
            unlink( $file );
        }
    }

    // create and fill the file to download
    $f = fopen( $filename_down, "w" );
    if ( ! $f ) {
        echo "Error\n\nCannot write configuration file on server.";
        return;
    }

    fputcsv( $f, array ( "Name", "Value" ) );

    foreach ( $config as $name => $value ) {
        fputcsv( $f, array ( $name, $value ) );
    }

    fclose( $f );

    echo $filename_down;
}
