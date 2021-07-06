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

session_start( );

require '../defaults.php';

$input_config = $config_pref . "lwi-in.csv";
$output_config = $config_pref . "lwi-out.csv";
$config_init = array ( );
$config_out = array ( );
$elem_in_names = array ( );
$elem_out_names = array ( );
$session_short_id = substr( session_id( ), -6 );


// check if configuration files are up to date and update if required
function check_config( ) {
    global $lsd_config, $sa_config, $input_config, $output_config, $limits_exec, $saved_exec;
    
    if ( ! file_exists( $lsd_config ) ) {
        return;
    }
        
    if ( ! file_exists( $output_config ) || ( file_exists( $output_config ) && filemtime( $output_config ) < filemtime( $lsd_config ) ) ) {
        if ( file_exists( $saved_exec ) ) {
            exec( $saved_exec . " -f " . $lsd_config . " -o " . $output_config, $shell_out, $shell_err );
        }
    }

    if ( ! file_exists( $sa_config ) ) {
        return;
    }
        
    if ( ! file_exists( $input_config ) || ( file_exists( $input_config ) && filemtime( $input_config ) < max( filemtime( $lsd_config ), filemtime( $sa_config ) ) ) ) {
        if ( file_exists( $limits_exec ) ) {
            exec( $limits_exec . " -f " . $lsd_config . " -s " . $sa_config . " -o " . $input_config, $shell_out, $shell_err );
        }
    }
}


// read .csv configuration file from server
function read_config( $type_config ) {
    global $input_config, $config_init, $elem_in_names;
    
    $col_names = array ( "Name", "Type", "Lag", "Format", "Value", "Minimum", "Maximum", "Description" );
    $col_idx = array_fill_keys( $col_names, -1 );
    
    if ( ! file_exists( $input_config ) ) {
        die ( "Required input file not found" );
    }
        
    $f = fopen( $input_config, "r" );
    
    // find column indexes to all required names
    $header = fgetcsv( $f );
    $cols = count( $header );
    $keys = count( $col_names );
    $idxs = 0;
    for ( $i = 0; $i < $cols; ++$i ) {
        for ( $j = 0; $j < $keys; ++$j ) {
            if ( $header[ $i ] == $col_names[ $j ] ) {
                $col_idx[ $col_names[ $j ] ] = $i;
                ++$idxs;
                break;
            }
        }
    }
    
    if ( $idxs < $keys ) {
        die ( "Invalid input file contents: idxs=$idxs keys=$keys" );
    }
    
    while ( ! feof( $f ) ) {
        
        $line = fgetcsv( $f );
        if ( $line[ $col_idx[ "Type" ] ] != $type_config ) {
            continue;
        }
        
        $elem = new config_class( );
        foreach ( $col_names as $value ) {
            $elem->$value = $line[ $col_idx[ $value ] ];
        }

        $config_init[ $line[ $col_idx[ "Name" ] ] ] = $elem;
        array_push( $elem_in_names, $line[ $col_idx[ "Name" ] ] );
        
        if ( $line[ $col_idx[ "Format" ] ] == "integer" ) {
            $step = 1;
        } else {
            $delta_up = $line[ $col_idx[ "Maximum" ] ] - $line[ $col_idx[ "Value" ] ];
            $delta_dw = $line[ $col_idx[ "Value" ] ] - $line[ $col_idx[ "Minimum" ] ];
            if ( $delta_up > $delta_dw ) {
                $step = $delta_up / 100;
                if ( $delta_dw > 0 ) {
                //    $step = $delta_dw / round( $delta_dw / $step );
                    $step = ( float ) sprintf( "%.3f", $delta_dw / round( $delta_dw / $step ) );
                }
            } else {
                $step = $delta_dw / 100;
            }
        }
        
        // create one parameter's table line
        echo "<tr>\n";
        echo "<td><b>" . $line[ $col_idx[ "Name" ] ] . "</b></td>\n";
        echo "<td>" . $line[ $col_idx[ "Description" ] ] . "</td>\n";
        echo "<td>" . $line[ $col_idx[ "Format" ] ] . "</td>\n";
        echo "<td>" . $line[ $col_idx[ "Minimum" ] ] . "-" . $line[ $col_idx[ "Maximum" ] ] . "</td>\n";
        echo "<td><input name='" . $line[ $col_idx[ "Name" ] ] . "' id='" . $line[ $col_idx[ "Name" ] ] . "' class='w3-input w3-border' type='number' step='" . $step . "' min='" . $line[ $col_idx[ "Minimum" ] ] . "' max='" . $line[ $col_idx[ "Maximum" ] ] . "' value='" . $line[ $col_idx[ "Value" ] ] . "' required></td>\n";
        echo "<td><button onclick='document.getElementById( '" . $line[ $col_idx[ "Name" ] ] . "' ).value = " . $line[ $col_idx[ "Value" ] ] . "; return false;' class='w3-button w3-blue w3-hover-black'>Reset</button></td>\n";
        echo "</tr>\n";
    }
    
    fclose( $f ); 

    $_SESSION[ "config_init" ] = $config_init;
}


// add hidden configuration data for JS
function write_config( ) {
    global $elem_in_names;
    
    session_write_close( );
    
    echo "<div id='elem_in_names' data-lwi-in='" . json_encode( $elem_in_names ) . "'></div>\n";
}


class output_class {
    public $Name = "", $Type = "", $Object = "", $Description = "";
}

// read .csv saved variables file from server
function read_saved( ) {
    global $output_config, $config_out, $elem_out_names;
    
    $col_names = array ( "Name", "Type", "Object", "Description" );
    $col_idx = array_fill_keys( $col_names, -1 );
          
    if ( ! file_exists( $output_config ) ) {
        die ( "Required output file not found" );
    }
        
    $f = fopen( $output_config, "r" );
    
    // find column indexes to all required names
    $header = fgetcsv( $f );
    $cols = count( $header );
    $keys = count( $col_names );
    $idxs = 0;
    for ( $i = 0; $i < $cols; ++$i ) {
        for ( $j = 0; $j < $keys; ++$j ) {
            if ( $header[ $i ] == $col_names[ $j ] ) {
                $col_idx[ $col_names[ $j ] ] = $i;
                ++$idxs;
                break;
            }
        }
    }
    
    if ( $idxs < $keys ) {
        die ( "Invalid output file contents: idxs=$idxs keys=$keys" );
    }
    
    while ( ! feof( $f ) ) {
        
        $line = fgetcsv( $f );
        if ( $line[ $col_idx[ "Name" ] ] == "" ) {
            continue;
        }
        
        $elem = new output_class( );
        foreach ( $col_names as $value ) {
            $elem->$value = $line[ $col_idx[ $value ] ];
        }

        $config_out[ $line[ $col_idx[ "Name" ] ] ] = $elem;
        array_push( $elem_out_names, $line[ $col_idx[ "Name" ] ] );
        
        // create one output's table line
        echo "<tr>\n";
        echo "<td><b>" . $line[ $col_idx[ "Name" ] ] . "</b></td>\n";
        echo "<td>" . $line[ $col_idx[ "Description" ] ] . "</td>\n";
        echo "<td><input name='_out-" . $line[ $col_idx[ "Name" ] ] . "' id='_out-" . $line[ $col_idx[ "Name" ] ] . "' class='w3-check' type='checkbox'></td>\n";
        echo "</tr>\n";
    }
    
    fclose( $f );
    
    echo "<div id='elem_out_names' data-lwi-out='" . json_encode( $elem_out_names ) . "'></div>\n";
}


$filename_res = false;

// check previously succesfull simulation execution results
function check_results( $session_short_id ) {
    global $filename_res, $output_pref;

    // get results file name
    $filename_res = glob( $output_pref . "run-" . $session_short_id . "_*.csv" );
    
    if ( $filename_res ) {
        $filename_res = $filename_res[ 0 ];
        echo "Ready for download";
    } else {
        echo "Simulation not run";
    }
}
                       

// check date/time creation of a previously found simulation results
function results_date_time( ) {
    global $filename_res;

    if ( $filename_res && file_exists( $filename_res[ 0 ] ) ) {
        echo date( "d M Y,  H:i T", filectime( $filename_res[ 0 ] ) );
    } else {
        echo "-";
    }
}


// check size of a previously found simulation results
function results_size( ) {
    global $filename_res;
    echo get_size( $filename_res );
}
