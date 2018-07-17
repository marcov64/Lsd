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

"use strict";

// check browser capabilities
function check_html5( ) {
    if ( ! Modernizr.canvas || ! Modernizr.cssanimations ) {
        document.getElementById( "blackout" ).style.display = "block";
        window.alert( "Insufficient HTML5 support\n\nPlease update your browser and try again." );
    }
}


// save current configuration to .csv file on server
function download_config( ) {
    // get the list of configuration values on the page
    var config = read_config( );
    
    // prepare to launch download when file is ready on the server
    var xhttp = new XMLHttpRequest( );
    xhttp.onreadystatechange = function( ) {
        if ( this.readyState === 4 && this.status === 200 ) {
            if ( this.responseText.substring( 0, 5 ) === "Error" ) {
                window.alert( this.responseText );
            } else {
                download( this.responseText );
            }
        }
    };

    // request the server to save configuration to the server disk
    xhttp.open( "POST", "download_config.php", true );
    xhttp.setRequestHeader( "Content-type", "application/x-www-form-urlencoded" );
    xhttp.send( "x=" + JSON.stringify( config ) );        
}


// load input configuration from the just uploaded user file
function upload_config( config ) {
    var elem;
    for ( elem in config ) {
        if ( document.getElementById( elem ) !== null ) {
            document.getElementById( elem ).value = config[ elem ];
        }
    }
}


// reload the default configuration
function reset_config( section ) {
    if ( window.confirm( "Reset configuration to defaults?\n\nAll changed values will be lost." ) ) {
        document.getElementById( section ).reset( );
    }
}


// create lsd configuration file and start simulation execution
function run_sim( ) {
    // check if not already running
    if ( statusID !== null || chronoID !== null ) {
        window.alert( "Simulation is already running\n\nPlease wait until simulation finishes\nor press 'Abort' to stop it." );
        return;
    }
    if ( run_done ) {
        if ( ! window.confirm( "Overwrite results from previous execution?\n\nAll previously produced results will be lost." ) ) {
            return;
        }       
    }
    
    // get the list of configuration values on the page
    var config = read_config( );
    
    var xhttp = new XMLHttpRequest( );
    xhttp.onreadystatechange = function( ) {
        if ( this.responseText.substring( 0, 6 ) === "Error:" ) {
            status_stop( );
            document.getElementById( "status" ).innerHTML = this.responseText;
        } else {
            if ( this.responseText === "Aborted" ) {
                status_stop( );
                document.getElementById( "status" ).innerHTML = "Simulation aborted by user";
                run_done = false;
            } else {
                if ( this.readyState === 1 ) {
                    status_start( "status", "chrono" );
                } else {
                    if ( this.readyState === 4 ) {
                        status_stop( );
                        document.getElementById( "_begin_" ).value = 1;
                        document.getElementById( "_begin_" ).max = config._timeSteps_;
                        document.getElementById( "_end_" ).value = config._timeSteps_;
                        document.getElementById( "_end_" ).max = config._timeSteps_;
                        document.getElementById( "_ready_" ).innerHTML = "Results ready for download";
                        document.getElementById( "_date_" ).innerHTML = format_date( new Date( ) );
                        document.getElementById( "_size_" ).innerHTML = this.responseText;
                        if ( this.status !== 200 ) {
                            document.getElementById( "status" ).innerHTML = "Error: code=" . this.status;
                        }
                    }
                }
            }
        }
    };

    xhttp.open( "POST", "run_sim.php", true );
    xhttp.setRequestHeader( "Content-type", "application/x-www-form-urlencoded" );
    xhttp.send( "x=" + JSON.stringify( config ) );            
}


// abort a running simulation execution
function abort_sim( ) {
    if ( statusID === null && chronoID === null ) {
        return;
    }
    
    // workaround for Mac servers that are unable to notify end of simulation
    if ( abort ) {
        reset_session( "The server is not responding" );
        return;
    }
    
    if ( ! window.confirm( "Abort simulation execution?\n\nAll results produced so far will be lost." ) ) {
        return;
    }
    
    abort = true;
    
    // request the server to create abort file in the server disk
    var xhttp = new XMLHttpRequest( );
    xhttp.open( "GET", "abort_sim.php", true );
    xhttp.send( );            
}


// check if log exists and get it
function show_log( ) {
    var xhttp = new XMLHttpRequest( );
    xhttp.onreadystatechange = function( ) {
        if ( this.readyState === 4 && this.status === 200 ) {
            if ( this.responseText !== "" ) {
                var log = window.open( "show_log.html" );
                var resp = JSON.parse( this.responseText );
                var text = "";
                var x;
                for ( x in resp ) {
                    text += resp[ x ];
                }
                log.onload = function( ) { 
                    log.document.getElementById( "log_text" ).innerHTML = text.replace( new RegExp( '\r?\n', 'g' ), '<br/>' );
                };
            } else {
                window.alert( "Simulation not run!\n\nPlease execute the simulation before\ntrying to view the log.");
            }
        }
    };

    // request the server to get log from the server disk (sync mode because of Safari)
    xhttp.open( "GET", "show_log.php", false );
    xhttp.send( );        
}


// download current results .csv file on server, if any
function download_res( ) {
    // prepare to launch download when file is ready on the server
    var xhttp = new XMLHttpRequest( );
    xhttp.onreadystatechange = function( ) {
        if ( this.readyState === 4 && this.status === 200 ) {
            if ( this.responseText === "NoFile:" ) {
                window.alert( "Results file is not available\n\nPlease execute the simulation before using this option." );
            } else {
                download( this.responseText );
            }
        }
    };

    // request the server to get results from the server disk
    xhttp.open( "GET", "download_res.php", true );
    xhttp.send( );        
}


// reset the curreno PHP session
function reset_session( msg ) {
    if ( ! window.confirm( "Reset current session?\n\n" + msg + "." ) ) {
        return;
    }   
        
    var xhttp = new XMLHttpRequest( );
    xhttp.onreadystatechange = function( ) {
        if ( this.readyState === 4 && this.status === 200 ) {
            location.reload( true );
        }
    };

    xhttp.open( "GET", "reset_session.php", true );
    xhttp.send( );            
}


// read the current configuration input values from page
function read_config( ) {
    var config = new Object( );
    
    // get the pre set list of configuration elements on page
    var div_php = document.getElementById( "elem_in_names" );
    var elem_names = JSON.parse( div_php.getAttribute( "data-lwi-in" ) );
    if ( elem_names.length === 0 ) {
        return config;
    }

    // create the object with the configuration (element name and value)
    var x;
    for ( x in elem_names ) {
        config[ elem_names[ x ] ] = document.getElementById( elem_names[ x ] ).value;
    }

    // add the static configuration elements (simulation length and seed)
    config._timeSteps_ = document.getElementById( "_timeSteps_" ).value;
    config._rndSeed_ = document.getElementById( "_rndSeed_" ).value;
    config._numRuns_ = "1";

    return config;
}


// send user message as e-mail and clear form
function send_and_clear( msg, name, email, text ) {
    // get the list of configuration values on the page
    var message = new Object( );
    message.name = document.getElementById( name ).value;
    message.email = document.getElementById( email ).value;
    message.text = document.getElementById( text ).value;
    
    // prepare to launch download when file is ready on the server
    var xhttp = new XMLHttpRequest( );
    xhttp.onreadystatechange = function( ) {
        if ( this.readyState === 4 && this.status === 200 ) {
            if ( this.responseText.substring( 0, 12 ) === "Message sent" ) {
                document.getElementById( msg ).reset( );
            }
            window.alert( this.responseText );
        }
    };

    // request the server to save configuration to the server disk
    xhttp.open( "POST", "contact.php", true );
    xhttp.setRequestHeader( "Content-type", "application/x-www-form-urlencoded" );
    xhttp.send( "x=" + JSON.stringify( message ) );        
}


// plot the selected series
function plot_chart( canv, lab, tval, dat, opt, lim ) {
    // get the data set by PHP
    var label_php = document.getElementById( lab );
    var label = JSON.parse( label_php.getAttribute( "data-labels" ) );
    var t_value_php = document.getElementById( tval );
    var t_value = JSON.parse( t_value_php.getAttribute( "data-t_values" ) );
    var dataset_php = document.getElementById( dat );
    var dataset = JSON.parse( dataset_php.getAttribute( "data-datasets" ) );
    
    // get user options
    var options_php = document.getElementById( opt );
    var auto = JSON.parse( options_php.getAttribute( "data-auto" ) );
    var linear = JSON.parse( options_php.getAttribute( "data-linear" ) );
    var limits_php = document.getElementById( lim );
    var min = Number( JSON.parse( limits_php.getAttribute( "data-min" ) ) );
    var max = Number( JSON.parse( limits_php.getAttribute( "data-max" ) ) );
    
    if ( canv === null || label === null || t_value === null || dataset === null ) {
        return false;
    }
    
    // create the plot data object
    var i, datasets = [ ];
    for ( i in dataset ) {
        var series = {
            label: label[ i ],
            data: dataset[ i ],
            spanGaps: true,
            fill: false,
            borderWidth: 2,
            pointRadius: 0
        };
        datasets.push( series );
    }
    
    // apply different colors
    i = 0;
    var c, len = datasets.length;
    for ( c in colors ) {
        if ( i >= len ) {
            break;
        }
        datasets[ i ].borderColor = colors[ c ];
        datasets[ i ].backgroundColor = colors[ c ];
        ++i;
    }
    
    // create the global options object
    var options = {
        maintainAspectRatio: false,
        scales: {
            yAxes: [ {
                type: linear ? "linear" : "logarithmic",
                ticks: {
                    maxTicksLimit: 6
                },
                gridLines: {
                    display: true,
                    color: colors.lightgray
                }
            } ],
            xAxes: [ {
                ticks: {
                    autoSkipPadding: 100,
                    maxRotation: 0,
                    maxTicksLimit: 10
                },
                gridLines: {
                    display: false,
                    color: colors.lightgray
                }
            } ]
        },
        legend: {
            display: true,
            position: "right",
            labels: {
                boxWidth: 10
            }
        },
        tooltips: {
            mode: "nearest",
            intersect: false,
            position: "nearest"
        },
        chartArea: {
            backgroundColor: colors.white
        }
    };
    
    if ( ! auto ) {
        options.scales.yAxes[ 0 ].ticks.min = min;
        options.scales.yAxes[ 0 ].ticks.max = max;
    }
    
    // control the background color
    Chart.pluginService.register( {
        beforeDraw: function ( chart, easing ) {
            if ( chart.config.options.chartArea && chart.config.options.chartArea.backgroundColor ) {
                var ctx = chart.chart.ctx;
                var chartArea = chart.chartArea;

                ctx.save( );
                ctx.fillStyle = chart.config.options.chartArea.backgroundColor;
                ctx.fillRect( chartArea.left, chartArea.top, chartArea.right - chartArea.left, chartArea.bottom - chartArea.top );
                ctx.restore( );
            }
        }
    } );	
	
    // plot
    var plot = new Chart( canv, {
		type: "line",
        options: options,
        data: {
            labels: t_value,
            datasets: datasets
        }
    } );    
    
    return true;
}


// download an existing file from server
function download( filename ) {
    // create temporary <a> tag to allow download
    var element = document.createElement( "a" );
    element.style.display = "none";
    element.setAttribute( "href", filename );
    element.setAttribute( "download", "" );

    // check HTML5 support and do workaround if not available
    if ( typeof element.download === "undefined" ) {
       window.open( filename );
    } else {
        // create and activate the temporary tag then remove it
        document.body.appendChild( element );
        element.click( );
        document.body.removeChild( element );
    }
}


// update % done and chronometer
var start, timerID, statusID = null, chronoID = null;
var pdone = "0", run_done = false, abort = false;

function status_start( id_stat, id_chrono ) {
    start = new Date( );
    statusID = document.getElementById( id_stat );
    chronoID = document.getElementById( id_chrono );
    if ( statusID !== null && chronoID !== null ) {
        pdone = "0";
        run_done = false;
        abort = false;
        timerID = window.setTimeout( status, 1000 );
    }
}

function status_stop( ) {
    if ( statusID !== null && chronoID !== null ) {
        window.clearTimeout( timerID );
        statusID.innerHTML = "Simulation run complete";
        run_done = true;
        abort = false;
        statusID = null;
        chronoID = null;
    }
}

function status( ) {
    if ( statusID === null || chronoID === null ) {
        return;
    }
    
    if ( abort ) {
        statusID.innerHTML = "Aborting ...";
        return;
    }
    
    var diff = new Date( Date.now( ) - start ); 
    var sec = diff.getUTCSeconds( );
    var min = diff.getUTCMinutes( );
    var hr = diff.getUTCHours( );
    
    // check status only each 5 seconds
    if ( ( sec + 1 ) % 5 === 0 ) {
        var xhttp = new XMLHttpRequest( );
        xhttp.timeout = 3000;
        xhttp.onreadystatechange = function( ) {
            if ( this.readyState === 4 && this.status === 200 ) {
                if ( this.responseText !== "" ) {
                    pdone = this.responseText;
                }
            }
        };
        xhttp.open( "GET", "stat_sim.php", true );
        xhttp.send( );
    }

    // nice format digits
    if ( min < 10 ) {
        min = "0" + min;
    }
    if ( sec < 10 ) {
        sec = "0" + sec;
    }
    
    statusID.innerHTML = "Simulation running ... (" + pdone + "% done)";
    chronoID.innerHTML = hr + "h" + min + "min" + sec + "s";
    timerID = window.setTimeout( status, 1000 );
}


// format date and time
function format_date( date ) {
    var monthNames = [ "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" ];

    var year = date.getFullYear( );
    var monthIndex = date.getMonth( );
    var day = date.getDate( );
    var hour = date.getHours( );
    var min = date.getMinutes( );

    // nice format digits
    if ( min < 10 ) {
        min = "0" + min;
    }
    if ( hour < 10 ) {
        hour = "0" + hour;
    }
    if ( day < 10 ) {
        day = "0" + day;
    }
    year = year.toString().substr( 2, 2 );
    
    return day + " " + monthNames[ monthIndex ] + " " + year + ",  " + hour + ":" + min;
}


// scripts to open and close sidebar
function w3_open( ) {
    document.getElementById( "mySidebar" ).style.display = "block";
    document.getElementById( "myOverlay" ).style.display = "block";
}


function w3_close( ) {
    document.getElementById( "mySidebar" ).style.display = "none";
    document.getElementById( "myOverlay" ).style.display = "none";
}


// named colors for Chart.js
var colors = {
    black: "rgba( 0, 0, 0, 1 )",
    red: "rgba( 255, 0, 0, 1 )",
    green: "rgba( 0, 128, 0, 1 )",
    gold: "rgba( 255, 215, 0, 1 )",
    fuchsia: "rgba( 255, 0, 255, 1 )",
    blue: "rgba( 0, 0, 255, 1 )",
    DeepSkyBlue: "rgba( 0, 191, 255, 1 )",
    gray: "rgba( 128, 128, 128, 1 )",
    brown: "rgba( 165, 42, 42, 1 )",
    cyan: "rgba( 0, 255, 255, 1 )",
    darksalmon: "rgba( 233, 150, 122, 1 )",
    DarkSeaGreen: "rgba( 143, 188, 143, 1 )",
    chartreuse: "rgba( 127, 255, 0, 1 )",
    indigo: "rgba( 75, 0, 130, 1 )",
    khaki: "rgba( 240, 230, 140, 1 )",
    siena: "rgba( 160, 82, 45, 1 )",
    lightgray: "rgba( 211, 211, 211, 1 )",
    white: "rgba( 255, 255, 255, 1 )"
};
