#
# Tcl package index file
#
# Note sqlite*3* init specifically
#
package ifneeded sqlite3 3.25.3 \
    [list load [file join $dir sqlite3.25.3.dll] Sqlite3]
