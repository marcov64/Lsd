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

$cookie_id = filter_input( INPUT_COOKIE, session_name( ), FILTER_SANITIZE_FULL_SPECIAL_CHARS );
if ( ! is_null( $cookie_id ) && is_string( $cookie_id ) ) {
    $session_id = preg_replace( "/[^\da-z]/i", "", $cookie_id );
} else {
    $session_id = "NOCOOKIE";
}

$session_short_id = substr( $session_id, -6 );

// Check if the form was submitted
if ( filter_input_array( INPUT_SERVER )[ "REQUEST_METHOD" ] === "POST" ) {

    $config = filter_config( json_decode( filter_input_array( INPUT_POST )[ "x" ], false ) );

    if ( ! is_countable( $config ) || count( $config ) == 0 ) {
        echo "Error: No data to configure simulation";
        return;
    }

    require '../exec_sim.php';
}
