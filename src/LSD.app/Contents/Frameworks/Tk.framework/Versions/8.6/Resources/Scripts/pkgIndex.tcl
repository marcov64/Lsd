if {[catch {package present Tcl 8.6.0}]} return
package ifneeded Tk 8.6.7 [list load [file normalize [file join $dir .. .. Tk]] Tk]
