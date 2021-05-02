<?php include '../filter_config.php'; ?>
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
        <title>Upload</title>
        <meta http-equiv="Content-Security-Policy" content="default-src 'self'; script-src 'self' 'unsafe-inline'; style-src 'self' 'unsafe-inline'">
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <link rel="icon" href="favicon.ico">
        <link rel="stylesheet" href="w3.css">
        <link rel="stylesheet" href="lwi.css">
        <!-- PHP & JS functions -->
        <?php
            // Check if the form was submitted
            if ( filter_input_array( INPUT_SERVER )[ "REQUEST_METHOD" ] === "POST" ) {
                // Check if file was uploaded without errors
                if ( isset( $_FILES[ "csv" ] ) && $_FILES[ "csv" ][ "error" ] == 0 ) {
                    $allowed = array( "csv" => ".csv" );
                    $filename = $_FILES[ "csv" ][ "name" ];
                    $filetype = $_FILES[ "csv" ][ "type" ];
                    $filesize = $_FILES[ "csv" ][ "size" ];

                    // Verify file extension
                    $ext = pathinfo( $filename, PATHINFO_EXTENSION );

                    if ( ! array_key_exists( $ext, $allowed ) ) {
                        $err1 = "Error";
                        $err2 = "Invalid file format";
                        goto end_err;
                    }

                    // Verify file size - 10kB maximum
                    $maxsize = 10 * 1024;
                    if ( $filesize > $maxsize ) { 
                        $err1 = "Error";
                        $err2 = "File size is larger than the allowed 10kB limit";
                        goto end_err;
                    }

                    // open temporary file and get the content
                    $f = fopen( $_FILES[ "csv" ][ "tmp_name" ], "r" );
                    if ( ! $f ) {
                        $err1 = "Error";
                        $err2 = "Cannot open uploaded file on server";
                        goto end_err;
                    }

                    // discard .csv header line
                    $header = fgetcsv( $f );
                    if ( ! isset( $header[ 0 ] ) || $header[ 0 ] != "Name" || ! isset( $header[ 1 ] ) || $header[ 1 ] != "Value" ) {
                        $err1 = "Error";
                        $err2 = "Invalid CSV file header";
                        goto end_err;
                    }

                    $config = array( );

                    // read each configuration line
                    while ( ! feof( $f ) ) {
                        $line = fgetcsv( $f );
                        if ( ! isset( $line[ 0 ] ) || ! isset( $line[ 1 ] ) ) {
                            continue;
                        }
                        if ( ! is_string( $line[ 0 ] ) || ! is_numeric( $line[ 1 ] ) ) {
                            $err1 = "Error";
                            $err2 = "Invalid CSV file content";
                            goto end_err;
                        }

                        $config[ $line[ 0 ] ] = $line[ 1 ];
                    }

                    // sanitize read values
                    $config_clean = filter_config( $config );
 
                } else {
                    $err1 = "Error";
                    $err2 = "Problem loading configuration file (" . $_FILES[ "csv" ][ "error" ] . ")";
                }
            } else {
                $err1 = "Error";
                $err2 = "Invalid page call";
            }

            end_err:
        ?>
    </head>
    <?php
        if ( isset ( $err1 ) && isset ( $err2 ) ) {
    ?>
            <body>
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
    <?php
            exit;
        }
        echo "<body onload='window.opener.upload_config( " . json_encode( $config_clean ) . " ); setTimeout( function ( ) { window.close( ); }, 3000 )'>\n";
    ?>
        <div class='w3-main w3-display-middle' style='max-width: 600px; margin-left:10px; margin-right:10px'>
            <div class='w3-container w3-card-2 w3-margin-bottom' style='margin-top:10px'>
                <div class='w3-container w3-center'>
                    <h2>Configuration loaded</h2>
                </div>
                <div class='w3-container w3-center' style='margin-top: 30px'>
                    <button onclick='window.close( )' class='w3-button w3-blue w3-padding-large w3-margin-right w3-margin-bottom w3-hover-black'>Close</button>
                </div>
            </div>
        </div>
    </body>
</html>
