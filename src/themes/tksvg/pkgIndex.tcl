#
# Tcl package index file
#
if { $::tcl_platform(platform) eq "unix" } {
	if { $::tcl_platform(os) eq "Darwin" } {
		package ifneeded tksvg 0.7 [ list load [ file join $dir libtksvg0.7.dylib ] tksvg ]
	} elseif { $::tcl_platform(os) eq "Linux" } {
		package ifneeded tksvg 0.7 [ list load [ file join $dir libtksvg0.7.so ] tksvg ]
	}
} elseif { $::tcl_platform(platform) eq "windows" && $::tcl_platform(machine) eq "amd64" } {
	package ifneeded tksvg 0.7 [ list load [ file join $dir tksvg07t.dll ] tksvg ]
}

