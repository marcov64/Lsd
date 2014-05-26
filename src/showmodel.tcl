####################################################
# Tcl script used to browse in the Lsd models 
# the script starts from the Lsd root and reads
# the modelinfo.txt or groupinfo.txt in all
# the descending directories.
# Users can browse through the model, create new models,
# create new groups, or delete them
#
# The implementation defines few global lists into which are stored the relevant
# data. As returning value, the proceduce sets the global variable "result"
# to the number of the list chosen and the global lists containing the references
# in the group selected


lappend lmn
lappend lver
lappend lmd
lappend ldn
lappend lrn
set modelgroup "Lsd"
lappend group
set result 0
set memory 0

proc showmodel pippo {
global lmn lmd ldn lrn group result choiceSM lver modelgroup RootLsd memory



set choiceSM 1
unset lmn
lappend lmn
unset lver
lappend lver
unset lmd
lappend lmd
unset ldn
lappend ldn
unset lrn
lappend lrn

unset group
lappend group

set choiceSM 0
toplevel .l
wm protocol .l WM_DELETE_WINDOW { }
wm title .l "Lsd Models"

frame .l.l -relief groove -bd 2
#if { [string compare "$modelgroup" "."] == 0 } {set modelgroup "(no group)"} {}
label .l.l.tit -text "Group: $modelgroup" -fg red
scrollbar .l.l.vs -command ".l.l.l yview"
listbox .l.l.l -height 12 -width 30 -yscroll ".l.l.vs set" -selectmode single

frame .l.t -relief groove -bd 2
label .l.t.tit -text Description -foreground red
pack .l.t.tit -expand yes -fill x
scrollbar .l.t.yscroll -command ".l.t.text yview"
text .l.t.text -wrap word -font {Times 12 normal} -width 60 -relief sunken -yscrollcommand ".l.t.yscroll set" -state disable

pack .l.t.yscroll -side right -fill y
pack .l.t.text -expand yes -fill both



pack .l.l.tit
pack .l.l.vs -side right -fill y
pack .l.l.l -expand yes -fill both
pack .l.l .l.t -expand yes -fill both -side left


set w .l; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w
bind .l.l.l <1> {.l.l.l selection set [.l.l.l nearest %y]; .l.t.text delete 0.0 end; .l.t.text insert end "[lindex $lmd [.l.l.l nearest %y] ]"}


set curdir [pwd]
cd $pippo
set dir [glob *]

foreach i $dir {
 set testdir "$i"

 if { [file isdirectory $testdir ] == 1 } {
   if { [file exists $i/modelinfo.txt]==1} {
     set f [open $i/modelinfo.txt r]
     set app1 "[gets $f]"
     set app2 "[gets $f]"
     lappend lmn "$app1"
     lappend lver "$app2"
     lappend ldn "$pippo/$i"
     lappend lrn "[pwd]"
     close $f
   if { [file exists $i/description.txt]==1}  {set f [open $i/description.txt]; lappend lmd "[read -nonewline $f]"; close $f} {lappend lmd "Model: $app1\nin directory: $pippo/$i\n(description not available)"}
     lappend group "0"
     .l.l.l insert end "$app1 (ver. $app2)"
     
    } { if { [file exists $i/groupinfo.txt]==1} {
      set f [open $i/groupinfo.txt r]
      set app "[gets $f]"
      close $f
#      if { [string compare $modelgroup "(no group)"] == 0} {lappend lmn "$app"} {lappend lmn "$modelgroup/$app"}
      lappend lmn "$modelgroup/$app"
      lappend lver "-1"
      lappend ldn "$pippo/$i"
      lappend lrn "[pwd]"
      if { [file exists $i/description.txt]==1}  {set f [open $i/description.txt]; lappend lmd "[read -nonewline $f]"; close $f} {lappend lmd "Group directory: $app\n(description not available)"}
      lappend group "1"
      .l.l.l insert end "GROUP: $app"
      
     } {}
    
   
   }
   } {}
   
 }


if { [string compare $pippo $RootLsd] != 0 } {.l.l.l insert end "<UP>"; lappend lver "-1"; lappend group 1; lappend lmd "Return to group: [file dirname $modelgroup]"; lappend lrn [pwd]; lappend ldn "[file dirname $pippo]"; lappend lmn "<UP>"} {}
.l.t.text conf -state normal
.l.t.text insert end "[lindex $lmd 0]"
.l.t.text conf -state disable

.l.l.l selection set 0

cd $curdir
bind .f.t.t <Enter> {focus -force .l};  wm transient .l .; focus -force .l;
#.f.t.t conf -state disabled


menu .l.m -tearoff 0 -relief groove -bd 12
set m .l.m.file 
menu $m -tearoff 0 -relief raised
.l.m add cascade -label File -menu $m -underline 0
$m add command -label Select -underline 0 -accelerator Enter -command {set result [.l.l.l curselection]; if { [lindex $group $result] == 1 } { if { [lindex $lmn $result] == "<UP>"} {set modelgroup [file dirname $modelgroup]} {set modelgroup [lindex $lmn $result]};bind .f.t.t <Enter> {}; .f.t.t conf -state normal; destroy .l; focus -force .f.t.t;  showmodel [lindex $ldn $result]} { bind .f.t.t <Enter> {}; .f.t.t conf -state normal; destroy .l; focus -force .f.t.t;  set choiceSM 1} }

$m add command -label Abort -underline 0 -accelerator Escape -command {set result -1;bind .f.t.t <Enter> {}; .f.t.t conf -state normal; destroy .l; focus -force .f.t.t; set choiceSM 2 }


set m .l.m.edit 
menu $m -tearoff 0 -relief raised -bd 2
.l.m add cascade -label Edit -menu $m -underline 0

$m add command -label "New Model/Group" -underline 0 -command {bind .f.t.t <Enter> {}; .f.t.t conf -state normal; destroy .l; focus -force .f.t.t;  set choiceSM 14} 
$m add command -label "Delete" -underline 0 -accelerator Delete -command {delete [.l.l.l curselection] }
$m add command -label "Copy" -underline 0 -command {copy [.l.l.l curselection] }
if { $memory == 0 } {$m add command -label "Paste" -underline 0 -state disabled -command {paste [.l.l.l curselection] } } {$m add command -label "Paste" -underline 0 -command {paste [.l.l.l curselection] } }
$m add command -label "Edit name/description" -underline 0 -command {edit [.l.l.l curselection] }


set m .l.m.help 
menu $m -tearoff 0 -relief raised
.l.m add cascade -label Help -menu $m -underline 0
$m add command -label "Help" -underline 0 -accelerator F1 -command {LsdHelp LMM_help.html#select}

bind .l <F1> {.l.m.help invoke 0}
bind .l <Delete> {.l.m.edit invoke 1}
bind .l <Escape> {.l.m.file invoke 1}
bind .l <Return> {.l.m.file invoke 0}
bind .l.l.l <Double-1> {.l.m.file invoke 0} 

bind .l.l.l <1> {.l.l.l selection clear 0 end; .l.l.l selection set [.l.l.l nearest %y]; set app [.l.l.l curselection]; .l.t.text conf -state normal; .l.t.text delete 0.0 end;  .l.t.text insert end "[lindex $lmd $app ]"; .l.t.text conf -state disable}

bind .l <Up> {if { [.l.l.l curselection] > 0 } {set app [expr [.l.l.l curselection] - 1]; .l.l.l selection clear 0 end; .l.l.l selection set $app; .l.t.text conf -state normal;.l.t.text delete 0.0 end; .l.t.text insert end "[lindex $lmd $app ]"; .l.t.text conf -state disable } {} }

bind .l <Down> {if { [.l.l.l curselection] < [expr [.l.l.l size] - 1] } {set app [expr [.l.l.l curselection] + 1]; .l.l.l selection clear 0 end; .l.l.l selection set $app; .l.t.text conf -state normal;.l.t.text delete 0.0 end;  .l.t.text insert end "[lindex $lmd $app ]"; .l.t.text conf -state disable } {} }




.l configure -menu .l.m

}





#################################
# copy a model position for future pastes
################################
proc copy i {
global copylabel copyver copydir copydscr group ldn memory lmn lver lmd
if { [lindex $group $i] == 1  } {tk_messageBox -title Error -type ok -icon error -message "Cannot copy groups" } {

set memory 1

.l.m.edit entryconf 3 -state normal

set copylabel [lindex $lmn $i]
set copyver [lindex $lver $i]
set copydir [lindex $ldn $i]
set copydscr [lindex $lmd $i]
}
}


#################################
# Remove a model/group, placing it in a trashbin
################################
proc delete i {
global lrn ldn lmn group RootLsd

if { [lindex $lmn $i] == "<UP>" } {return } {}


if { [lindex $group $i] == 1} {set item group} {set item model}
set answer [tk_messageBox -type yesno -title Remove -icon question -message "Do you want to remove $item\n[lindex $lmn $i]\n(dir [lindex $ldn $i])?"]

if { $answer == "yes" } {
 file rename -force [lindex $ldn $i] $RootLsd/trashbin
 destroy .l
 showmodel [lindex $lrn $i]} { }
}

#################################
# Edit the model/group name and description
################################
proc edit i {
global lrn ldn lmn group lmd result

destroy .l
toplevel .l

wm title .l "Edit"
if { [lindex $group $i] == 1} {set item group} {set item model}
label .l.ln -text "Insert new label for the $item [lindex $lmn $i]" -anchor w
set app "[lindex $lmn $i]"

entry .l.n -width 30
.l.n insert 1 "[file tail [lindex $lmn $i]]"
label .l.ld -text "Insert a new description" -anchor w
frame .l.t
scrollbar .l.t.yscroll -command ".l.t.text yview"
text .l.t.text -wrap word -width 60 -relief sunken -yscrollcommand ".l.t.yscroll set"

pack .l.ln .l.n .l.ld -fill x
pack .l.t.yscroll -side right -fill y
pack .l.t.text -expand yes -fill both
pack .l.t
frame .l.b
button .l.b.ok -text Ok -command {set app "[.l.n get]"; if { [lindex $group $result] == 1} {set f [open [lindex $ldn $result]/groupinfo.txt w]} {set f [open [lindex $ldn $result]/modelinfo.txt w]}; puts -nonewline $f "$app"; close $f; set f [open [lindex $ldn $result]/description.txt w]; puts -nonewline $f [.l.t.text get 0.0 end]; close $f; destroy .l; showmodel [lindex $lrn $result]}
button .l.b.esc -text Cancel -command {destroy .l; showmodel [lindex $lrn $result]}
pack .l.b.ok .l.b.esc -side left
pack .l.b

.l.t.text insert end "[lindex $lmd $i]"
set result $i
focus -force .l.n
.l.n selection range 0 end
bind .l.n <Return> {focus -force .l.t.text; .l.t.text mark set insert 1.0}
bind .l <Escape> {.l.b.esc invoke}
bind .l.t.text <Control-e> {.l.b.ok invoke}

set w .l; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w
}



#################################
# Paste a previously copied model/group
################################
proc paste i {
global copydir copyver copylabel copydscr lrn memory modelgroup lmn lver lmd choiceSM

set pastedir [lindex $lrn $i]

# with the line below works!
#puts here




destroy .l
toplevel .l

update
wm title .l "Paste Model"


label .l.ln -text "Insert a label for the copy of model: $copylabel" -anchor w
entry .l.n -width 30
.l.n insert 0 "$copylabel"

label .l.lv -text "Insert a new version number for the new model" -anchor w
entry .l.v -width 30
.l.v insert 0 "$copyver"

label .l.ld -text "Insert a directory name for the new model to be created in [lindex $lrn $i]" -anchor w
entry .l.d -width 30
.l.d insert 0 "[file tail $copydir]"

label .l.ldsc -text "Insert a description for the new model" -anchor w
frame .l.t
scrollbar .l.t.yscroll -command ".l.t.text yview"
text .l.t.text -wrap word -width 60 -relief sunken -yscrollcommand ".l.t.yscroll set"
.l.t.text insert end "$copydscr"

pack .l.ln .l.n .l.lv .l.v .l.ld .l.d .l.ldsc -fill x

pack .l.t.yscroll -side right -fill y
pack .l.t.text -expand yes -fill both
pack .l.t
frame .l.b

button .l.b.ok -text Ok -command {set choiceSM 1}
button .l.b.esc -text Cancel -command {set choiceSM 2}
pack .l.b.ok .l.b.esc -side left
pack .l.b


set w .l; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w
raise .l
focus -force .l.n
bind .l.n <Return> {focus -force .l.v}
bind .l.v <Return> {focus -force .l.d}
bind .l.d <Return> {focus -force .l.b.ok}
bind .l.b.ok <Return> {.l.b.ok invoke}
bind .l <Escape> {.l.b.esc invoke}

tkwait variable choiceSM

if { $choiceSM == 2 } { } {

  set appd [.l.d get]
  set appv [.l.v get]
  set appl [.l.n get]
  set appdsc "[.l.t.text get 1.0 end]"
  
  set confirm [tk_messageBox -type yesno -icon info -title Confirm -message "Every file in dir.:\n$copydir\n is going to be copied in dir.:\n$pastedir/$appd"]
  if { $confirm == "yes" } {
    set app [file exists $pastedir/$appd]
    if { $app == 1} {tk_messageBox -title Error -icon error -type ok -message "Directory $pastedir/$appd already exists. Specify a different directory." } {
       #viable directory name 
       file mkdir $pastedir/$appd
       set copylist [glob $copydir/*]
#       tk_messageBox -title Here1 -type ok -message "Arrived here."
       foreach a $copylist {catch [file copy -force $a $pastedir/$appd]}
#       tk_messageBox -title Here2 -type ok -message "Arrived here, too."
       set f [open $pastedir/$appd/description.txt w]
       puts -nonewline $f "$appdsc"
       close $f
#       tk_messageBox -title Here3 -type ok -message "Arrived here 3."       
       set f [open $pastedir/$appd/modelinfo.txt w]
       puts $f "$appl"
       puts $f "$appv"
       set frmt "%d %B, %Y"
       puts $f "[clock format [clock seconds] -format "$frmt"]"
       close $f
     } 
  
} {}
}
destroy .l
set choiceSM 0
# tk_messageBox -title Here4 -type ok -message "Arrived here 4."
showmodel [lindex $lrn $i]

}

#set RootLsd [pwd]; frame .f; frame .f.t; text .f.t.t; destroy .l; proc showmodel {} {}; source src/showmodel.tcl; showmodel [pwd]
#destroy .l; proc showmodel {} {}; source src/showmodel.tcl; showmodel [pwd]
