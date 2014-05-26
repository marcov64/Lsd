# list models returning the list a exploring directory b

proc lst_mdl {} {
global lmod ldir lgroup cgroup

if { [file exists modelinfo.txt]==1 } {

lappend ldir [pwd]
set f [open modelinfo.txt r]
set info [gets $f]
close $f
lappend lmod "$info"
lappend lgroup $cgroup } {}

if { 1==1 } {


set dirs [glob -nocomplain -type d *]


foreach i $dirs {
set flag 0
if { [file isdirectory $i]==1 } {
 cd $i
 if { [file exists groupinfo.txt]==1 } {
  set f [open groupinfo.txt r]
  set info [gets $f]
  close $f
  if {$cgroup != "."} {set cgroup [file join "$cgroup" "$info"] } {set cgroup "$info" }
  set flag 1} {}
  
 lst_mdl
 if { $flag == 1} {
  set cgroup [file dirname $cgroup] } {}
 cd ..
 } {} 
} 

} {}

}

proc chs_mdl {} {
global lmod ldir sd sf d1 d2 f1 f2 lgroup cgroup

set lmod ""
set ldir ""
set sd ""
set sf ""
set d1 ""
set d2 ""
set f1 ""
set f2 ""
set lgroup ""
set cgroup ""

set glabel ""

unset lmod
unset ldir
unset sd
unset sf



lst_mdl


toplevel .l
wm protocol .l WM_DELETE_WINDOW { set choice -1; sblocklmm .l}
wm title .l "Lsd Models"

frame .l.l -relief groove -bd 2
label .l.l.tit -text "List of models" -fg red
scrollbar .l.l.vs -command ".l.l.l yview"
listbox .l.l.l -height 30 -width 70 -yscroll ".l.l.vs set" -selectmode browse
bind .l.l.l <ButtonRelease> {set glabel [lindex $lgroup [.l.l.l curselection]]; .l.l.gl configu -text "$glabel"}
bind .l.l.l <KeyRelease-Up> {set glabel [lindex $lgroup [.l.l.l curselection]]; .l.l.gl configu -text "$glabel"}
bind .l.l.l <KeyRelease-Down> {set glabel [lindex $lgroup [.l.l.l curselection]]; .l.l.gl configu -text "$glabel"}
label .l.l.gt -text "Selected model contained in group:" 
label .l.l.gl -text "$glabel" 


frame .l.t -relief groove -bd 2
frame .l.t.f1 
label .l.t.f1.tit -text "Selected models" -foreground red
frame .l.t.f1.m1 -relief groove -bd 2
label .l.t.f1.m1.l -text "First model"
entry .l.t.f1.m1.d -width 50 -textvariable d1
entry .l.t.f1.m1.f -width 20 -textvariable f1
bind .l.t.f1.m1.f <3> {set tmp [tk_getOpenFile -initialdir $d1]; if { $tmp!=""} {set f1 [file tail $tmp]} {} }
button .l.t.f1.m1.i -text Insert -command {slct; if { [info exists sd]} {set d1 "$sd"; set f1 "$sf"} {}}
pack .l.t.f1.m1.l -anchor nw
pack .l.t.f1.m1.d .l.t.f1.m1.f -expand yes -fill x
pack .l.t.f1.m1.i -anchor n


frame .l.t.f1.m2 -relief groove -bd 2
label .l.t.f1.m2.l -text "Second model"
entry .l.t.f1.m2.d -width 50 -textvariable d2
entry .l.t.f1.m2.f -width 20 -textvariable f2
bind .l.t.f1.m2.f <3> {set tmp [tk_getOpenFile -initialdir $d2]; if { $tmp!=""} {set f2 [file tail $tmp]} {} }
button .l.t.f1.m2.i -text Insert -command {slct; if { [info exists sd]} {set d2 "$sd"; set f2 "$sf"} {}}
pack .l.t.f1.m2.l -anchor nw
pack .l.t.f1.m2.d .l.t.f1.m2.f -expand yes -fill x -anchor nw
pack .l.t.f1.m2.i -anchor n


pack .l.t.f1.tit .l.t.f1.m1 .l.t.f1.m2 -expand yes -fill x -anchor n
pack .l.t.f1 -fill x -anchor n

button .l.t.f1.cmp -text " Compare files " -command {sblocklmm .l; destroy .l; set choice 1}
button .l.t.f1.cnc -text " Cancel " -command {sblocklmm .l; set d1 ""; destroy .l; set choice -1}

pack .l.t.f1.cmp .l.t.f1.cnc

pack .l.l.tit

pack .l.l.vs -side right -fill y
pack .l.l.l -expand yes -fill both 
pack .l.l.gt .l.l.gl -side top -expand yes -fill x -anchor w

pack .l.l .l.t -expand yes -fill both -side left

set j 0
foreach i $lmod {
set k [lindex $lgroup $j]
incr j
.l.l.l insert end "$i" 
}

set w .l; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w

blocklmm .l
}

proc slct {} {
global sd sf ldir
set tmp [.l.l.l curselection]
if { $tmp=="" } {return; set sd ""; set sf ""} {}


set sd [lindex $ldir $tmp]
set sf [file tail [glob [file join $sd *.cpp]]]
}

