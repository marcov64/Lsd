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

// check data integrity and load it if ok
if ( filter_input_array( INPUT_SERVER )[ "REQUEST_METHOD" ] === "POST" ) {

    // get selected variables plus options
    $config = filter_input_array( INPUT_POST );
    $server = filter_input_array( INPUT_SERVER );
    $full_name = filter_var( $server[ "SCRIPT_NAME" ], FILTER_SANITIZE_URL );
    $from_slash = strrchr( $full_name, "/" );
    if ( $from_slash ) {
        $script = substr( $from_slash, 1 );
    } else {
        $script = $full_name;
    }

    if ( count( $config ) == 0 ) {
        $err1 = "Error";
        $err2 = "No data to produce simulation statistics";
        goto end_err;
    }

    // get results file name
    $filename_results = "tmp/run-" . $session_short_id . "_*.csv";

    if ( ! ( $results = glob( $filename_results ) ) ) {
        $err1 = "No simulation results available";
        $err2 = "Please execute the simulation before using this option";
        goto end_err;
    }

    // count selected variables
    $num_vars = 0;
    $sel_vars = array ( );
    foreach ( $config as $var => $value ) {
        $var = filter_var( $var, FILTER_SANITIZE_STRING );
        $value = filter_var( $value, FILTER_SANITIZE_STRING );
        if ( ! strncmp( $var, "_out-", 5 ) && $value == "on" ) {
            ++$num_vars;
            $sel_vars[ ] = substr( $var, 5 );
        }
    }
    if ( $num_vars == 0 && $script !== "show_stats.php" ) {
        $err1 = "No time series selected";
        $err2 = "Please select at least one series and try again";
        goto end_err;
    }
    if ( $num_vars > 15 ) {
        $err1 = "Too many time series selected";
        $err2 = "Please select up to 15 series and try again";
        goto end_err;
    }

    // read the .csv results file
    $f = fopen( $results[ 0 ], "r" );
    if ( $f == null ) {
        $err1 = "Error";
        $err2 = "Cannot open results file '" . $results[ 0 ] . "' on server";
        goto end_err;
    }

    // get .csv header line
    $vars = fgetcsv( $f );
    $tot_vars = count( $vars );
    if ( feof( $f ) || ! isset( $vars[ 0 ] ) || $vars[ 0 ] == "" ) {
        $err1 = "Error";
        $err2 = "Invalid results file '" . $results[ 0 ] . "' on server";
        goto end_err;
    }

    // use all series if none was selected
    if ( $num_vars == 0 ) {
        $num_vars = $tot_vars;
        $sel_vars = $vars;
    }

    // sanitize user-set options
    $config[ "_linear_" ] = filter_var( $config[ "_linear_" ], FILTER_SANITIZE_NUMBER_INT );
    $linear = ( $config[ "_linear_" ] == "1" ) ? true : false;
    $config[ "_auto_" ] = filter_var( $config[ "_auto_" ], FILTER_SANITIZE_NUMBER_INT );
    $auto = ( $config[ "_auto_" ] == "1" ) ? true : false;
    if ( ! $auto ) {
        $min = filter_var( $config[ "_min_" ], FILTER_SANITIZE_NUMBER_INT );
        $max = max( filter_var( $config[ "_max_" ], FILTER_SANITIZE_NUMBER_INT ), $config[ "_min_" ] );
    }

    // read each configuration line
    $series = array( );
    $steps = 0;
    while ( ! feof( $f ) ) {
        $line = fgetcsv( $f );

        // ignore blank lines
        if ( ! isset( $line[ 0 ] ) || $line[ 0 ] == "" ) {
            continue;
        }

        // check if all data is present
        if ( count( $line ) < $tot_vars ) {
            $err1 = "Error";
            $err2 = "Invalid results file '" . $results[ 0 ] . "' on line " . ( $steps + 2 );
            goto end_err;
        }

        // read each column and add to one time series
        for ( $i = 0; $i < $tot_vars; ++$i ) {
            if ( ! is_numeric( $line[ $i ] ) && $line[ $i ] !== "NA" ) {
                $err1 = "Error";
                $err2 = "Invalid results file '" . $results[ 0 ] . "' on line " . ( $steps + 2 ) . ", column " . ( $i + 1 );
                goto end_err;
            }

            // treat N/A and log values properly
            $x = $line[ $i ];
            if ( $x === "NA" ) {
                $series[ $vars[ $i ] ][ $steps ] = "N/A";
            } elseif ( $linear || $script === "show_plot.php" ) {
                $series[ $vars[ $i ] ][ $steps ] = floatval( $x );
            } elseif ( floatval( $x ) > 0 ) {
                $series[ $vars[ $i ] ][ $steps ] = log( floatval( $x ) );
            } else {
                $series[ $vars[ $i ] ][ $steps ] = "N/A";
            }
        }

        ++$steps;
    }

    // finish sanitizing user-set options
    $config[ "_begin_" ] = min( max( filter_var( $config[ "_begin_" ], FILTER_SANITIZE_NUMBER_INT ), 1 ), 1000 );
    $config[ "_end_" ] = min( max( filter_var( $config[ "_end_" ], FILTER_SANITIZE_NUMBER_INT ), $config[ "_begin_" ] ), 1000 );
    $first = $config[ "_begin_" ] - 1;
    $last = max( min( $config[ "_end_" ], $steps ), $first + 1 );
    $all = ( $first > 1 || $last < $steps ) ? false : true;

    fclose( $f );

    // compute descriptive statistics
    if ( $script === "show_stats.php" ) {
        
        $stats = array ( );
        foreach( $series as $var => $ts ) {
            $n = $sum = $sqsum = 0;
            $min = INF;
            $max = -INF;
            for ( $i = $first; $i < $last; ++$i ) {
                $x = $ts[ $i ];
                if ( $x == "N/A" ) {
                    continue;
                }
                ++$n;
                $sum += $x;
                $sqsum += $x * $x;
                if ( $x < $min ) {
                    $min = $x;
                }
                if ( $x > $max ) {
                    $max = $x;
                }
            }

            if ( $n == 0 ) {
                $stats[ $var ][ "Mean" ] = "N/A";  
                $stats[ $var ][ "SD" ] = "N/A";  
                $stats[ $var ][ "Min" ] = "N/A";  
                $stats[ $var ][ "Max" ] = "N/A";  
            } else {
                $stats[ $var ][ "Mean" ] = $sum / $n;  
                $stats[ $var ][ "SD" ] = sqrt( max( $sqsum / $n - ( $sum / $n ) * ( $sum / $n ), 0 ) );  
                $stats[ $var ][ "Min" ] = $min;  
                $stats[ $var ][ "Max" ] = $max;  
            }
            $stats[ $var ][ "Obs" ] = $n;
        }
    }

} else {
    $err1 = "Error";
    $err2 = "Invalid page call";
}

end_err:

if ( isset ( $err1 ) && isset ( $err2 ) ) {
?>
            <div class='w3-main w3-display-middle' style='max-width: 600px; margin-left:10px; margin-right:10px'>
                <div class='w3-container w3-card-2 w3-margin-bottom' style='margin-top:10px'>
                    <div class='w3-container w3-center'>
                    <?php 
                        echo "<h2>" . $err1 . "</h2></br>\n";
                        echo "<p>" . $err2 . "</p>\n";
                    ?>
                    </div>
                    <div class='w3-container w3-center' style='margin-top: 30px'>
                        <button onclick='window.close( )' class='w3-button w3-blue w3-padding-large w3-margin-right w3-margin-bottom w3-hover-black'>Close</button>
                    </div>
                </div>
            </div>
        </body>
    </html>
<?php
    exit;
}
