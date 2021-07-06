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

include '../defaults.php';

if ( filter_input_array( INPUT_SERVER )[ "REQUEST_METHOD" ] === "POST" ) {

    // get message data
    $message = json_decode( filter_input_array( INPUT_POST )[ "x" ], false );
    if ( isset ( $message ) ) {
        if ( isset ( $message->name ) ) {
            $name = filter_var( $message->name, FILTER_SANITIZE_STRING );
            if ( isset ( $message->email ) ) {
                $email = filter_var( $message->email, FILTER_SANITIZE_EMAIL );
                if ( isset ( $message->text ) ) {
                    $text = wordwrap( filter_var( $message->text, FILTER_SANITIZE_STRING ), 70, "\r\n" );
                } else {
                    echo "Invalid message text\n\nPlease try adding a valid text and try again";
                    return;
                }
            } else {
                echo "Invalid e-mail\n\nPlease try inserting a valid e-mail and try again";
                return;
            }
        } else {
            echo "Invalid name\n\nPlease try inserting a valid name and try again";
            return;
        }
    } else {
        echo "Invalid message\n\nPlease check your message and try again";
        return;
    }

    // Preferences for Subject field
    $subject_preferences = array(
        "input-charset" => "utf-8",
        "output-charset" => "utf-8",
        "line-length" => 76,
        "line-break-chars" => "\r\n"
    );

    // Mail header
    $header = "Content-type: text/html; charset=utf-8\r\n";
    $header .= "From: " . $name . " <" . $email . ">\r\n";
    $header .= "Reply-To: " . $reply_to . "\r\n";
    $header .= "MIME-Version: 1.0\r\n";
    $header .= "Content-Transfer-Encoding: 8bit\r\n";
    $header .= "Date: " . date( "r (T)" ) . "\r\n";
    $header .= "X-Mailer: PHP/" . phpversion( ) . "\r\n";
    $header .= iconv_mime_encode( "Subject", $subject, $subject_preferences );

    // Send mail
    if ( mail( $lwi_admin , $subject, $text, $header ) ) {
        echo "Message sent\n\nThanks for your feedback";
    } else {    
        echo "Message not sent\n\nSorry, there was a problem sending your message";
    }

} else {
    echo "Error\n\nInvalid page call";
}
