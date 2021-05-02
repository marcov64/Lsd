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
        <title>Data</title>
        <meta http-equiv="Content-Security-Policy" content="default-src 'self'; script-src 'self' 'unsafe-inline'; style-src 'self' 'unsafe-inline'">
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <link rel="icon" href="favicon.ico">
        <link rel="stylesheet" href="w3.css">
        <link rel="stylesheet" href="lwi.css">
    </head>
    <body>
        <?php include "../load_res.php"; ?>
        <div class='w3-main' style='margin-left:10px; margin-right:10px'>
            <div class='w3-container w3-card-2 w3-margin-bottom' style='margin-top:10px'>
                <h1 class='w3-xxxlarge w3-text-blue'><b>Data</b></h1>
                <div class='w3-container' style='margin-top: 30px'>
                    <h2 class='w3-xxlarge w3-text-blue'>Time series data</h2>
                    <table class='w3-table w3-striped w3-white'>
                        <col style='width:10%'>
                        <?php
                            // insert column widths
                            $widthp = min( 90 / $num_vars, 10 );
                            foreach ( $sel_vars as $var ) {
                                if ( ! isset ( $series[ $var ] ) ) {
                                    continue;
                                }
                                echo "<col style='width:" . $widthp . "%'>\n";
                            }
                        ?>
                        <thead>
                            <td><em>Time step</em></td>
                            <?php
                                // insert table first row
                                foreach ( $sel_vars as $var ) {
                                    if ( ! isset ( $series[ $var ] ) ) {
                                        continue;
                                    }
                                    echo "<td><em>" . $var . "</em></td>\n";
                                }
                            ?>
                        </thead>
                        <?php
                            // insert the table lines data
                            for ( $i = $first; $i < $last; ++$i ) {
                                echo "<tr>\n";
                                echo   "<td><b>" . ( $i + 1 ) . "</b></td>\n";

                                foreach ( $sel_vars as $var ) {
                                    if ( ! isset ( $series[ $var ] ) ) {
                                        continue;
                                    }

                                    if ( is_numeric( $series[ $var ][ $i ] ) ) {
                                        echo "<td>" . sprintf( "%.5G", $series[ $var ][ $i ] ) . "</td>\n";
                                    } else {
                                        echo "<td>N/A</td>\n";
                                    }
                                }

                                echo "</tr>\n";
                            }
                        ?>
                    </table>
                    <?php
                        // table footnotes
                        $log_msg = $linear ? "" : "<em>Log values</em>.\n";
                        echo "<p>" . $log_msg . "</p>\n";
                    ?>
                </div>
                <div class="w3-container w3-center" style="margin-top: 30px"> 
                    <button onclick='window.close( )' class='w3-button w3-blue w3-padding-large w3-margin-right w3-margin-bottom w3-hover-black'>Close</button>
                </div>
            </div>
        </div>
    </body>
</html>
