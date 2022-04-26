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

$cookie_id = filter_input( INPUT_COOKIE, session_name( ), FILTER_SANITIZE_FULL_SPECIAL_CHARS );
if ( ! is_null( $cookie_id ) && is_string( $cookie_id ) ) {
    $session_id = preg_replace( "/[^\da-z]/i", "", $cookie_id );
} else {
    $session_id = "NOCOOKIE";
}

$session_short_id = substr( $session_id, -6 );

require '../defaults.php';

$filename_flag = glob( $flag_pref . "run-" . $session_short_id . "-*" . ".flag" );

if ( is_countable( $filename_flag ) && count( $filename_flag ) > 0 && file_exists( $filename_flag[ 0 ] ) ) {
    $filename_abort = $config_pref . "run-" . $session_short_id . ".abort";
    touch( $filename_abort );
}
