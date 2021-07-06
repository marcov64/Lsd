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

require "../defaults.php";

// t distribution for 95% confidence level factor
function t95cl( $df ) {
    $t975 = [ 12.710, 4.303, 3.182, 2.776, 2.571, 2.447, 2.365, 2.306, 2.262, 2.228, 
              2.201, 2.179, 2.160, 2.145, 2.131, 2.120, 2.110, 2.101, 2.093, 2.086, 
              2.080, 2.074, 2.069, 2.064, 2.060, 2.056, 2.052, 2.048, 2.045, 2.042, 
              2.021, 2.000, 1.990, 1.984, 1.962, 1.960 ];   
    
    if ( $df <= 30 ) {
        return $t975[ $df - 1 ];
    } elseif ( $df <= 40 ) {
        return $t975[ 30 ];
    } elseif ( $df <= 60 ) {
        return $t975[ 31 ];
    } elseif ( $df <= 80 ) {
        return $t975[ 32 ];
    } elseif ( $df <= 100 ) {
        return $t975[ 33 ];
    } elseif ( $df <= 1000 ) {
        return $t975[ 34 ];
    } else {
        return $t975[ 35 ];
    }
}

function getcsvheader( $fn, &$vars, &$err ) {
    $f = fopen( $fn, "r" );
    if ( $f == null ) {
        $err = array ( "Error", 
                       "Cannot open results file '" . $fn . "' on server" );
        return 0;
    }

    $vars = fgetcsv( $f );
    $tot_vars = count( $vars );
    fclose( $f );

    if ( ! $vars || $tot_vars === 0 || ! isset( $vars[ 0 ] ) || $vars[ 0 ] === "" ) {
        $err = array ( "Error", 
                       "Invalid results file '" . $fn . "' on server" );
        return 0;
    }
    
    return $tot_vars;
}

function getcsvdata( $fn, $log, &$series, &$err ) {
    
    $f = fopen( $fn, "r" );
    if ( $f == null ) {
        $err = array ( "Error", 
                       "Cannot open results file '" . $fn . "' on server" );
        return 0;
    }

    $vars = fgetcsv( $f );
    $tot_vars = count( $vars );

    if ( ! $vars || feof( $f ) || $tot_vars === 0 ) {
        $err = array ( "Error", 
                       "Invalid results file '" . $fn . "' on server" );
        fclose( $f );
        return 0;
    }
    
    $steps = 0;
    while ( ! feof( $f ) ) {
        
        $line = fgetcsv( $f );

        // ignore blank lines
        if ( ! isset( $line[ 0 ] ) || $line[ 0 ] == "" ) {
            continue;
        }

        // check if all data is present
        if ( count( $line ) < $tot_vars ) {
            $err = array ( "Error", 
                           "Invalid results file '" . $fn . "' on line " . ( $steps + 2 ) );
            fclose( $f );
            return 0;
        }

        // read each column and add to one time series
        for ( $i = 0; $i < $tot_vars; ++$i ) {
            
            if ( ! is_numeric( $line[ $i ] ) && strtoupper( $line[ $i ] ) !== "NA" ) {
                $err = array ( "Error", 
                               "Invalid results file '" . $fn . "' on line " . ( $steps + 2 ) . ", column " . ( $i + 1 ) );
                fclose( $f );
                return 0;
            }

            // treat N/A and log values properly
            $x = $line[ $i ];
            if ( strtoupper( $x ) === "NA" ) {
                $series[ $vars[ $i ] ][ $steps ] = "N/A";
            } elseif ( ! $log ) {
                $series[ $vars[ $i ] ][ $steps ] = floatval( $x );
            } elseif ( floatval( $x ) > 0 ) {
                $series[ $vars[ $i ] ][ $steps ] = log( floatval( $x ) );
            } else {
                $series[ $vars[ $i ] ][ $steps ] = "N/A";
            }
        }

        ++$steps;
    }
   
    fclose( $f );
    return $steps;
}
    
$err = array ( );

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
        $err = array ( "Error", 
                       "No data to produce simulation statistics" );
        goto end_err;
    }

    // sanitize user-set options
    $config[ "_begin_" ] = min( max( filter_var( $config[ "_begin_" ], FILTER_SANITIZE_NUMBER_INT ), 1 ), 10000 );
    $config[ "_end_" ] = min( max( filter_var( $config[ "_end_" ], FILTER_SANITIZE_NUMBER_INT ), $config[ "_begin_" ] ), 10000 );
    $config[ "_linear_" ] = filter_var( $config[ "_linear_" ], FILTER_SANITIZE_NUMBER_INT );
    $config[ "_auto_" ] = filter_var( $config[ "_auto_" ], FILTER_SANITIZE_NUMBER_INT );
    
    if ( isset ( $config[ "_CI_" ] ) ) {
        $config[ "_CI_" ] = filter_var( $config[ "_CI_" ], FILTER_SANITIZE_NUMBER_INT );
    } else {
        $config[ "_CI_" ] = "0";
    }
    
    if ( isset ( $config[ "_MM_" ] ) ) {
        $config[ "_MM_" ] = filter_var( $config[ "_MM_" ], FILTER_SANITIZE_NUMBER_INT );
    } else {
        $config[ "_MM_" ] = "0";        
    }
    
    $first = $config[ "_begin_" ] - 1;
    $ci = ( $config[ "_CI_" ] == "1" ) ? true : false;
    $mm = ( $config[ "_MM_" ] == "1" ) ? true : false;
    $linear = ( $config[ "_linear_" ] == "1" ) ? true : false;
    $log = ! $linear && $script !== "show_plot.php";
    $auto = ( $config[ "_auto_" ] == "1" ) ? true : false;
    if ( ! $auto ) {
        $min = filter_var( $config[ "_min_" ], FILTER_VALIDATE_FLOAT, FILTER_NULL_ON_FAILURE );
        $max = filter_var( $config[ "_max_" ], FILTER_VALIDATE_FLOAT, FILTER_NULL_ON_FAILURE );
        if ( $min === null || $max === null || $max <= $min || ( $log && $min <= 0 ) ) {
            $auto = true;
        }
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
        $err = array ( "No time series selected", 
                       "Please select at least one series and try again" );
        goto end_err;
    }
    
    if ( $num_vars > 15 ) {
        $err = array ( "Too many time series selected", 
                       "Please select up to 15 series and try again" );
        goto end_err;
    }

    // get results file name
    $results = glob( $output_pref . "run-" . $session_short_id . "_*.csv" );

    if ( ! $results || count( $results ) === 0 ) {
        $err = array ( "No simulation results available", 
                       "Please execute the simulation before using this option" );
        goto end_err;
    }

    if ( count( $results ) > 1 ) {
        $mc_runs = 0;
        foreach ( $results as $fn ) {
            if ( preg_match( "~^.+_[0-9]+\.csv~", $fn ) ) {
                ++$mc_runs;
            }
        }
        
        $results = glob( $output_pref . "run-" . $session_short_id . "_mean.csv" );
        $res_se = glob( $output_pref . "run-" . $session_short_id . "_se.csv" );
        $res_max = glob( $output_pref . "run-" . $session_short_id . "_max.csv" );
        $res_min = glob( $output_pref . "run-" . $session_short_id . "_min.csv" );
        
        if ( ! $results || count( $results ) !== 1 ||
             ! $res_se || count( $res_se ) !== 1 || 
             ! $res_max || count( $res_max ) !== 1 || 
             ! $res_min || count( $res_min ) !== 1 ) {
            $err = array ( "Incomplete simulation results available", 
                           "Please try to execute the simulation again" );
            goto end_err;
        }
    } else {
        $mc_runs = 1;
    }
    
    // read (mean) results file
    $vars = array ( );    
    $tot_vars = getcsvheader( $results[ 0 ], $vars, $err );
    if ( $tot_vars === 0 ) {
        goto end_err;
    }

    $series = array ( );
    $steps = getcsvdata( $results[ 0 ], $log, $series, $err );
    if ( $steps === 0 ) {
        goto end_err;
    }
    
    // read other MC results files
    if ( isset ( $res_se ) && isset ( $res_max ) && isset ( $res_min ) ) {
        $vars_se = $vars_max = $vars_min = array ( );
        getcsvheader( $res_se[ 0 ], $vars_se, $err );
        getcsvheader( $res_max[ 0 ], $vars_max, $err );
        getcsvheader( $res_min[ 0 ], $vars_min, $err );
        
        if ( $vars_se !== $vars || $vars_max !== $vars || $vars_min !== $vars ) {
            $err = array ( "Inconsistent MC simulation results (vars)", 
                           "Please try to execute the simulation again" );
            goto end_err;
        }        
        
        $series_se = $series_max = $series_min = array ( );
        $steps_se = getcsvdata( $res_se[ 0 ], $log, $series_se, $err );
        $steps_max = getcsvdata( $res_max[ 0 ], $log, $series_max, $err );
        $steps_min = getcsvdata( $res_min[ 0 ], $log, $series_min, $err );
         
        if ( $steps_se !== $steps || $steps_max !== $steps || $steps_min !== $steps ) {
            $err = array ( "Inconsistent MC simulation results (steps)", 
                           "Please try to execute the simulation again" );
            goto end_err;
        }
    }
    
    // finish sanitizing user-set options
    $last = max( min( $config[ "_end_" ], $steps ), $first + 1 );
    $all = ( $first > 1 || $last < $steps ) ? false : true;

    // use all series if none was selected
    if ( $num_vars === 0 ) {
        $num_vars = $tot_vars;
        $sel_vars = $vars;
    }

    // compute descriptive statistics
    if ( $script === "show_stats.php" ) {
        
        $stats = array ( );
        foreach( $series as $var => $ts ) {
            $n = $sum = $sqsum = 0;
            $mi = INF;
            $ma = -INF;
            for ( $i = $first; $i < $last; ++$i ) {
                $x = $ts[ $i ];
                if ( $x == "N/A" ) {
                    continue;
                }
                ++$n;
                $sum += $x;
                $sqsum += $x * $x;
                if ( $x < $mi ) {
                    $mi = $x;
                }
                if ( $x > $ma ) {
                    $ma = $x;
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
                $stats[ $var ][ "Min" ] = $mi;  
                $stats[ $var ][ "Max" ] = $ma;  
            }
            
            $stats[ $var ][ "Obs" ] = $n * $mc_runs;
            
            if ( ! isset ( $series_se ) ) {
                $stats[ $var ][ "SE" ] = "N/A";
            }
        }
        
        if ( isset ( $series_se ) ) {
            foreach( $series_se as $var => $ts ) {
                $n = $sum = 0;
                for ( $i = $first; $i < $last; ++$i ) {
                    $x = $ts[ $i ];
                    if ( $x == "N/A" ) {
                        continue;
                    }
                    ++$n;
                    $sum += $x;
                }

                if ( $n == 0 ) {
                   $stats[ $var ][ "SE" ] = "N/A";  
                } else {
                   $stats[ $var ][ "SE" ] = $sum / $n;
                }
           }
        }
    }

    // compute confidence bands
    if ( $script === "show_plot.php" && isset ( $series_se ) ) {
        
        $series_lo = $series_hi = array ( );
        foreach( $series as $var => $ts ) {
            $series_lo[ $var ] = $series_hi[ $var ] = array ( );
            for ( $i = 0; $i < $steps; ++$i ) {
                $ci_range = t95cl( $mc_runs - 1 ) * $series_se[ $var ][ $i ];
                $series_lo[ $var ][ $i ] = $ts[ $i ] - $ci_range;
                $series_hi[ $var ][ $i ] = $ts[ $i ] + $ci_range;
            }
        }
    }
        
} else {
    $err = array ( "Error", "Invalid page call" );
}

end_err:

if ( count( $err ) === 2 ) {
?>
            <div class='w3-main w3-display-middle' style='max-width: 600px; margin-left:10px; margin-right:10px'>
                <div class='w3-container w3-card-2 w3-margin-bottom' style='margin-top:10px'>
                    <div class='w3-container w3-center'>
                    <?php 
                        echo "<h2>" . $err[ 0 ] . "</h2></br>\n";
                        echo "<p>" . $err[ 1 ] . "</p>\n";
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
