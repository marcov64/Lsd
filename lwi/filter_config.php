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

class config_class {
    public $Name = "", $Type = "", $Lag = "", $Format = "", $Value = "", $Minimum = "", $Maximum = "", $Description = "";
}

// validate a configuration array
function filter_config( $config ) {
    
    // open and close session as fast as possible
    //session_start( [ 'cookie_lifetime' => 86400, 'cookie_secure' => true, 'cookie_samesite' => "None" ] );
    //$config_init = $_SESSION[ "config_init" ];
    //session_write_close( );

    // dirty solution to avoid always blocking behavior
    if ( ! session_readonly( ) ) {
        // workaround of workaround for Mac servers
        session_start( [ 'cookie_lifetime' => 86400, 'cookie_secure' => true, 'cookie_samesite' => "None" ] );
        session_write_close( );
    }
    $config_init = $_SESSION[ "config_init" ];

    $config_clean = array( );
    foreach ( $config as $name => $value ) {
        
        $name_clean = filter_var( $name, FILTER_SANITIZE_STRING );
        if ( ! $name_clean ) {
            continue;
        }
        
        // handle meta-parameters and regular elements
        if ( $name_clean == "_timeSteps_" ) {
            $value_clean = min( max( filter_var( $value, FILTER_SANITIZE_NUMBER_INT ), 10 ), 10000 );
        } elseif ( $name_clean == "_rndSeed_" ) {
            $value_clean = max( filter_var( $value, FILTER_SANITIZE_NUMBER_INT ), 1 );
        } elseif ( $name_clean == "_numRuns_" ) {
            $value_clean = min( max( filter_var( $value, FILTER_SANITIZE_NUMBER_INT ), 1 ), 100 );
        } else {
            if ( ! isset ( $config_init[ $name_clean ] ) ) {
                continue;
            }

            $format = $config_init[ $name_clean ]->Format;
            $max = $config_init[ $name_clean ]->Maximum;
            $min = $config_init[ $name_clean ]->Minimum;
            if ( $format == "integer" ) {
                $value_clean = filter_var( $value, FILTER_SANITIZE_NUMBER_INT );
            } else {
                $value_clean = filter_var( $value, FILTER_SANITIZE_NUMBER_FLOAT, FILTER_FLAG_ALLOW_FRACTION );
            }
            
            if ( ! $value_clean && $value_clean !== "0" ) {
                continue;
            }
            
            $value_clean = min( max( $value_clean, $min ), $max );
        }
        $config_clean[ $name_clean ] = $value_clean;
    }

    return $config_clean;
}


// dirty way read session data without locking files
// this is necessary as session_write_close( ) doesn't really unlock
// doesn't work on Mac servers because 'session_save_path( )' doesn't work
function session_readonly( )
{
    $session_id = preg_replace( "/[^\da-z]/i", "", filter_input( INPUT_COOKIE, session_name( ), FILTER_SANITIZE_STRING ) );
    $session_file = session_save_path( ) . "/sess_" . $session_id;
    
    if ( file_exists( $session_file ) ) {
        $session_data = file_get_contents( $session_file );

        $return_data = array( );
        $offset = 0;
        while ( $offset < strlen( $session_data ) ) {
            if ( ! strstr( substr( $session_data, $offset), "|" ) ) {
                break;
            }
            $pos = strpos( $session_data, "|", $offset );
            $num = $pos - $offset;
            $varname = substr( $session_data, $offset, $num );
            $offset += $num + 1;
            $data = unserialize( substr( $session_data, $offset ) );
            $return_data[ $varname ] = $data;
            $offset += strlen( serialize( $data ) );
        }
        $_SESSION = $return_data;
        
        return true;
    }
    
    return false;
}

// set error handler for ignoring warnings - typically due to session management problems
function error_handler( $errno, $errstr ) {
    if ( $errno === E_WARNING || $errno === E_NOTICE ) {
        return;
    }
    
    echo "<b>Error:</b> [$errno] $errstr<br>";
    echo "Ending Script";
    die( );
}

set_error_handler( "error_handler" );

// get size of a set of files
function get_size( $files ) {
    if ( $files != false && count( $files ) > 0 ) {
        $size = 0;
        $num = 0;
        foreach ( $files as $f ) {
            if ( file_exists( $f ) ) {
                $size += filesize( $f );
                ++$num;
            }
        }
        
        if ( $size > 0 ) {
            if ( $num > 1 ) {
                return number_format( $size / 1024, 1 ) . " kB  [" . $num . " file(s)]";
            } else {
                return number_format( $size / 1024, 1 ) . " kB";
            }
        } else {
            return "-";
        }
    } else {
        return "-";
    }
}
