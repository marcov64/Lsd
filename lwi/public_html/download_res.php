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

$cookie_id = filter_input( INPUT_COOKIE, session_name( ), FILTER_SANITIZE_FULL_SPECIAL_CHARS );
if ( ! is_null( $cookie_id ) && is_string( $cookie_id ) ) {
    $session_id = preg_replace( "/[^\da-z]/i", "", $cookie_id );
} else {
    $session_id = "NOCOOKIE";
}

$session_short_id = substr( $session_id, -6 );

// check if results file exists
$filename_res = $output_pref . "run-" . $session_short_id . "_*.csv";
$filename_res = glob( $filename_res );

if ( is_countable( $filename_res ) && count( $filename_res ) === 1 ) {
    echo $filename_res[ 0 ];
} elseif ( is_countable( $filename_res ) && count( $filename_res ) > 1 ) {
    $filename_zip = $output_pref . "run-" . $session_short_id . ".zip";

    if ( file_exists( $filename_zip ) ) {
        unlink( $filename_zip );
    }

    // adjust Windows executable names
    if ( strtoupper( substr( PHP_OS, 0, 3 ) ) === "WIN" ) {
        $res = exec( "tar -a -c -C " . $output_pref . " -f " . $filename_zip . " " . implode( " ", array_map( basename, $filename_res ) ) );
    } else {
        $res = exec( "zip -j " . $filename_zip . " " . implode( " ", $filename_res ) );
    }

    if ( $res ) {
        echo $filename_zip;
    } else {
        echo "NoFile:";
    }

} else {
    echo "NoFile:";
}
