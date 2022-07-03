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

include '../defaults.php';

$cookie_id = filter_input( INPUT_COOKIE, session_name( ), FILTER_SANITIZE_FULL_SPECIAL_CHARS );
if ( ! is_null( $cookie_id ) && is_string( $cookie_id ) ) {
    $session_id = preg_replace( "/[^\da-z]/i", "", $cookie_id );
} else {
    $session_id = "NOCOOKIE";
}

$session_short_id = substr( $session_id, -6 );

$filename_log = $output_pref . "run-" . $session_short_id . ".log";
$stat = 0;

if ( file_exists( $filename_log ) ) {
    $log = file( $filename_log );

    for ( $i = count( $log ) - 1; $i >= 0; --$i ) {
        if ( strstr( $log[ $i ], "0%" ) ) {
            $last = strrpos( $log[ $i ], "%" );
            for ( $j = $last; $j > 0 && $log[ $i ][ $j ] != "."; --$j );
            if ( $j > 0 ) {
                ++$j;
                $stat = substr( $log[ $i ], $j, $last - $j );
                break;
            }
        }
    }
}

echo $stat;
