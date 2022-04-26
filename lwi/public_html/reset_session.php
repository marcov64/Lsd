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

// abort any running simulation before reseting cookie
require "abort_sim.php";

// delete session temporary files
array_map( 'unlink', glob( $output_pref . "*-" . $session_short_id . "*.*" ) );
array_map( 'unlink', glob( $config_pref . "*-" . $session_short_id . "*.*" ) );

// reset the session
session_start( [ 'cookie_lifetime' => 86400, 'cookie_secure' => true, 'cookie_samesite' => "None" ] );
session_regenerate_id( );
