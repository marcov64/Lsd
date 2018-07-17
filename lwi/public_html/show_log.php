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

$session_id = preg_replace( "/[^\da-z]/i", "", filter_input( INPUT_COOKIE, session_name( ), FILTER_SANITIZE_STRING ) );
$session_short_id = substr( $session_id, -6 );

$filename_log = "../tmp/run-" . $session_short_id . ".log";

if ( file_exists( $filename_log ) ) {
    echo json_encode( file( $filename_log ) );    
}
