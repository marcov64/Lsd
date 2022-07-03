<?php
    $cookie_id = filter_input( INPUT_COOKIE, session_name( ), FILTER_SANITIZE_FULL_SPECIAL_CHARS );
    if ( ! is_null( $cookie_id ) && is_string( $cookie_id ) ) {
        $session_id = preg_replace( "/[^\da-z]/i", "", $cookie_id );
    } else {
        $session_id = "NOCOOKIE";
    }

    $session_short_id = substr( $session_id, -6 );
?>
<!DOCTYPE html>
<!--
Copyright (C) 2021 Marcelo C. Pereira <mcper at unicamp.br>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
-->
<html>
    <head>
        <title>Plot</title>
        <meta http-equiv="Content-Security-Policy" content="default-src 'self'; script-src 'self' 'unsafe-inline'; style-src 'self' 'unsafe-inline'">
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <link rel="icon" href="favicon.ico">
        <link rel="stylesheet" href="w3.css">
        <link rel="stylesheet" href="lwi.css">
        <!-- load the chart library -->
        <script src="chart.min.js"></script>
        <script src="script.js"></script>
        <noscript>Error: Javascript scripting is disabled</noscript>
    </head>
    <body onload="if ( ! plot_chart( '_chart_', '_labels_', '_t_values_', '_datasets_', '_options_', '_limits_' ) ) {
                        window.alert( 'Invalid plot data\n\nPlease check your data selection and try again.' );
                        window.close( );
                    }">
        <?php
            require "../load_res.php";

            // set plot labels
            $labels = array ( );
            foreach ( $sel_vars as $var ) {
                if ( ! isset ( $series[ $var ] ) ) {
                    continue;
                }

                $labels[ ] = $var;
            }

            echo "<div id='_labels_' data-labels='" . json_encode( $labels ) . "'></div>\n";

            // set data matrix on the specified time window
            $t_values = $datasets = $datasets_lo = $datasets_hi = $datasets_min = $datasets_max = array ( );
            for ( $i = $first, $j = 0; $i < $last; ++$i, ++$j ) {

                $t_values[ $j ] = $i + 1;
                $k = 0;
                foreach ( $sel_vars as $var ) {

                    if ( ! isset ( $series[ $var ] ) ) {
                        continue;
                    }

                    if ( is_numeric( $series[ $var ][ $i ] ) ) {
                        $datasets[ $k ][ $j ] = sprintf( "%.4G", $series[ $var ][ $i ] );
                    } else {
                        $datasets[ $k ][ $j ] = null;
                    }

                    if ( $ci && $mc_runs > 1 ) {
                        if ( is_numeric( $series_lo[ $var ][ $i ] ) &&
                             is_numeric( $series_hi[ $var ][ $i ] ) ) {
                            $datasets_lo[ $k ][ $j ] = sprintf( "%.4G", $series_lo[ $var ][ $i ] );
                            $datasets_hi[ $k ][ $j ] = sprintf( "%.4G", $series_hi[ $var ][ $i ] );
                        } else {
                            $datasets_lo[ $k ][ $j ] = $datasets_hi[ $k ][ $j ] = null;
                        }
                    }

                    if ( $mm && $mc_runs > 1 ) {
                        if ( is_numeric( $series_min[ $var ][ $i ] ) &&
                             is_numeric( $series_max[ $var ][ $i ] ) ) {
                            $datasets_min[ $k ][ $j ] = sprintf( "%.4G", $series_min[ $var ][ $i ] );
                            $datasets_max[ $k ][ $j ] = sprintf( "%.4G", $series_max[ $var ][ $i ] );
                        } else {
                            $datasets_min[ $k ][ $j ] = $datasets_max[ $k ][ $j ] = null;
                        }
                    }

                    ++$k;
                }
            }
            echo "<div id='_t_values_' data-t_values='" . json_encode( $t_values ) . "'></div>\n";
            echo "<div id='_datasets_' data-datasets='" . json_encode( $datasets ) .
                                    "' data-datasets_lo='" . json_encode( $datasets_lo ) .
                                    "' data-datasets_hi='" . json_encode( $datasets_hi ) .
                                    "' data-datasets_min='" . json_encode( $datasets_min ) .
                                    "' data-datasets_max='" . json_encode( $datasets_max ) . "'></div>\n";
            echo "<div id='_options_' data-linear='" . json_encode( $linear ) .
                                   "' data-auto='" . json_encode( $auto ) .
                                   "' data-ci='" . json_encode( $ci ) .
                                   "' data-mm='" . json_encode( $mm ) . "'></div>\n";
            if ( $auto ) {
                echo "<div id='_limits_' data-min='0.0' data-max='1.0'></div>\n";
            } else {
                echo "<div id='_limits_' data-min='" . json_encode( $min ) . "' data-max='" . json_encode( $max ) . "'></div>\n";
            }
        ?>
        <div class='w3-main w3-display-middle w3-white' style='margin-left:10px; margin-right:10px'>
            <div class='w3-container w3-card-2 w3-margin-bottom' style='margin-top:10px'>
                <div class='w3-container w3-center' style='margin-top: 30px'>
                    <div class="chart-container" style="position:relative; margin:auto; height:60vh; width:80vw; background-color: #fff">
                        <canvas id="_chart_" style="background-color: #fff"></canvas>
                    </div>
                    <?php
                        // plot footnotes
                        $t_msg = $all ? "" : "Time step range: from " . ( $first + 1 ) . " to " . $last . ".";
                        $ci_msg = ( $ci && $mc_runs > 1 ) ? "  MC 95% confidence interval in dark shadow." : "";
                        $mm_msg = ( $mm && $mc_runs > 1 ) ? "  MC Min/Max values in light shadow." : "";
                        $scale_msg = $auto ? "" : "  Vertical range: from " . $min . " to " . $max . ".";
                        $log_msg = $linear ? "" : "  <em>Log scale</em>.";
                        echo "<p>" . $t_msg . $ci_msg . $mm_msg . $scale_msg . $log_msg . "</p>\n";
                    ?>
                </div>
                <div class="w3-container w3-center" style="margin-top: 30px">
                    <button onclick='window.close( )' class='w3-button w3-blue w3-padding-large w3-margin-right w3-margin-bottom w3-hover-black'>Close</button>
                </div>
            </div>
        </div>
    </body>
</html>
