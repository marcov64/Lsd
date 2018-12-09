<?php
    $session_id = preg_replace( "/[^\da-z]/i", "", filter_input( INPUT_COOKIE, session_name( ), FILTER_SANITIZE_STRING ) );
    $session_short_id = substr( $session_id, -6 );
?>
<!DOCTYPE html>
<!--
Copyright (C) 2017 Marcelo C. Pereira <mcper at unicamp.br>

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
        <link rel="icon" href="lsd_favicon.ico">
        <link rel="stylesheet" href="w3.css">
        <link rel="stylesheet" href="lwi.css">
        <!-- load the chart library -->
        <script src="Chart.min.js"></script>
        <script src="script.js"></script>
        <noscript>Error: Javascript scripting is disabled</noscript>
    </head>
    <body onload="if ( ! plot_chart( '_chart_', '_labels_', '_t_values_', '_datasets_', '_options_', '_limits_' ) ) {
                        window.alert( 'Invalid plot data\n\nPlease check your data selection and try again.' );
                        window.close( );
                    }">
        <?php 
            include "../load_res.php"; 
            
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
            $t_values = array( );
            $datasets = array ( );
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
                    ++$k;
                }
            }
            echo "<div id='_t_values_' data-t_values='" . json_encode( $t_values ) . "'></div>\n";
            echo "<div id='_datasets_' data-datasets='" . json_encode( $datasets ) . "'></div>\n";
            echo "<div id='_options_' data-linear='" . json_encode( $linear ) . "' data-auto='" . json_encode( $auto ) . "'></div>\n";
            if ( $auto ) {
                echo "<div id='_limits_' data-min='0' data-max='1'></div>\n";
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
                        $scale_msg = $auto ? "" : "  Vertical range: from " . $min . " to " . $max . ".";
                        $log_msg = $linear ? "" : "  <em>Log scale</em>.";
                        echo "<p>" . $t_msg . $scale_msg . $log_msg . "</p>\n";
                    ?>
                </div>
                <div class="w3-container w3-center" style="margin-top: 30px"> 
                    <button onclick='window.close( )' class='w3-button w3-blue w3-padding-large w3-margin-right w3-margin-bottom w3-hover-black'>Close</button>
                </div>
            </div>
        </div>
    </body>
</html>
