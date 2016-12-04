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
if [ winfo exists .l ] {
	destroy .l.l .l.t
	set menuOk 1
} { 
	newtop .l "Lsd Model Browser" { .l.m.file invoke 2 }
	set menuOk 0
}

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

bind .l.l.l <1> {.l.l.l selection set [.l.l.l nearest %y]; .l.t.text delete 0.0 end; .l.t.text insert end "[lindex $lmd [.l.l.l nearest %y] ]"}

set curdir [pwd]
if { [ file isdirectory $pippo ] != 1 } {
# recover from invalid folders
 set pippo $RootLsd
}
cd $pippo
if { [string compare $pippo $RootLsd] != 0 } {.l.l.l insert end "<UP>"; lappend lver "-1"; lappend group 1; lappend lmd "Return to group: [file dirname $modelgroup]"; lappend lrn [pwd]; lappend ldn "[file dirname $pippo]"; lappend lmn "<UP>"} {}

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

.l.t.text conf -state normal
.l.t.text insert end "[lindex $lmd 0]"
.l.t.text conf -state disable
.l.l.l selection set 0

cd $curdir

if { ! $menuOk } { # only redraw menu/window if needed

menu .l.m -tearoff 0 -relief groove -bd 2
set m .l.m.file 
menu $m -tearoff 0
.l.m add cascade -label File -menu $m -underline 0
$m add command -label "Select Model" -underline 0 -accelerator Enter -command {set result [.l.l.l curselection]; if { [lindex $group $result] == 1 } { if { [lindex $lmn $result] == "<UP>"} {set modelgroup [file dirname $modelgroup]} {set modelgroup [lindex $lmn $result]}; showmodel [lindex $ldn $result]} { destroytop .l; set choiceSM 1} }
$m add command -label "New Model/Group..." -underline 0  -accelerator Insert -command { destroytop .l; set memory 0; set choiceSM 14} 
$m add command -label Quit -underline 0 -accelerator Escape -command {set result -1; destroytop .l; set memory 0; set choiceSM 2 }

set m .l.m.edit 
menu $m -tearoff 0 -relief groove -bd 2
.l.m add cascade -label Edit -menu $m -underline 0
$m add command -label "Edit Name/Description..." -underline 0 -command {edit [.l.l.l curselection] }
$m add command -label "Copy" -underline 0 -accelerator Ctrl+C -command {copy [.l.l.l curselection] }
if { $memory == 0 } {$m add command -label "Paste" -underline 0 -accelerator Ctrl+V -state disabled -command {paste [.l.l.l curselection] } } {$m add command -label "Paste" -underline 0 -accelerator Ctrl+V -command {paste [.l.l.l curselection] } }
$m add command -label "Delete..." -underline 0 -accelerator Delete -command {delete [.l.l.l curselection] }

set m .l.m.help 
menu $m -tearoff 0 -relief groove -bd 2
.l.m add cascade -label Help -menu $m -underline 0
$m add command -label "Help" -underline 0 -accelerator F1 -command { LsdHelp LMM_help.html#select }
$m add separator
$m add command -label "About Lsd..." -underline 0 -command { tk_messageBox -parent .l -type ok -icon info -title "About Lsd" -message "Version $_LSD_VERSION_ ($_LSD_DATE_)" -detail "Platform: [ string totitle $tcl_platform(platform) ] ($tcl_platform(machine))\nOS: $tcl_platform(os) ($tcl_platform(osVersion))\nTcl/Tk: [ info patch ]" }

.l configure -menu .l.m

# call procedure to adjust geometry and block lmm window, if fresh new window
showtop .l centerS
}

bind .l <F1> {.l.m.help invoke 0}
bind .l <Control-c> {.l.m.edit invoke 1}
bind .l <Control-C> {.l.m.edit invoke 1}
bind .l <Control-v> {.l.m.edit invoke 2}
bind .l <Control-V> {.l.m.edit invoke 2}
bind .l <Delete> {.l.m.edit invoke 3}
bind .l <Escape> {.l.m.file invoke 2}
bind .l <Insert> {.l.m.file invoke 1}
bind .l <Return> {.l.m.file invoke 0}
bind .l.l.l <Double-1> {.l.m.file invoke 0} 

bind .l.l.l <1> {.l.l.l selection clear 0 end; .l.l.l selection set [.l.l.l nearest %y]; set app [.l.l.l curselection]; .l.t.text conf -state normal; .l.t.text delete 0.0 end;  .l.t.text insert end "[lindex $lmd $app ]"; .l.t.text conf -state disable}

bind .l <Up> {if { [.l.l.l curselection] > 0 } {set app [expr [.l.l.l curselection] - 1]; .l.l.l selection clear 0 end; .l.l.l selection set $app; .l.t.text conf -state normal;.l.t.text delete 0.0 end; .l.t.text insert end "[lindex $lmd $app ]"; .l.t.text conf -state disable } {} }

bind .l <Down> {if { [.l.l.l curselection] < [expr [.l.l.l size] - 1] } {set app [expr [.l.l.l curselection] + 1]; .l.l.l selection clear 0 end; .l.l.l selection set $app; .l.t.text conf -state normal;.l.t.text delete 0.0 end;  .l.t.text insert end "[lindex $lmd $app ]"; .l.t.text conf -state disable } {} }

}


#################################
# copy a model position for future pastes
################################
proc copy i {
global copylabel copyver copydir copydscr group ldn memory lmn lver lmd
if { [lindex $group $i] == 1  } {tk_messageBox -parent .l -title Error -type ok -icon error -message "Cannot copy groups" -detail "Check for existing names and try again" } {

set memory 1

.l.m.edit entryconf 2 -state normal

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
global lrn ldn lmn group RootLsd memory

set memory 0
.l.m.edit entryconf 2 -state disabled

if { [lindex $lmn $i] == "<UP>" } {return } {}

if { [lindex $group $i] == 1} {set item group} {set item model}

if { [ string match -nocase $RootLsd/trashbin* [ lindex $ldn $i ] ] } {
 set answer [tk_messageBox -parent .l -type yesno -title Confirmation -icon question -default yes -message "Confirm deletion?" -detail "Do you want to delete $item\n[lindex $lmn $i]\n(dir [lindex $ldn $i])?"]
 file delete -force [ lindex $ldn $i ]
 showmodel [lindex $lrn $i]
} {
set answer [tk_messageBox -parent .l -type yesno -title Confirmation -icon question -default yes -message "Confirm deletion?" -detail "Do you want to delete $item\n[lindex $lmn $i]\n(dir [lindex $ldn $i])?"]

if { $answer == "yes" } {
 set modelDir [ string range [ lindex $ldn $i ] 0 [ expr [ string last / [ lindex $ldn $i ] ] - 1 ] ] 

 file mkdir $RootLsd/trashbin
 set f [open $RootLsd/trashbin/groupinfo.txt w]
 puts $f "Deleted Models"
 close $f
 set f [open $RootLsd/trashbin/description.txt w]
 puts $f "Folder containing deleted models.\n"
 close $f
 set modelName [ string range [ lindex $ldn $i ] [ expr [ string last / [ lindex $ldn $i ] ] + 1 ] end ]
 if { [ file exists $RootLsd/trashbin/$modelName ] } {
  file delete -force $RootLsd/trashbin/$modelName
 }
 file rename -force [lindex $ldn $i] $RootLsd/trashbin/$modelName
 showmodel [lindex $lrn $i]
 } {}
 }
}


#################################
# Edit the model/group name and description
################################
proc edit i {
global lrn ldn lmn group lmd result memory

set memory 0
.l.m.edit entryconf 2 -state disabled

newtop .l.e "Edit" { .l.e.b.esc invoke }

if { [lindex $group $i] == 1} {set item group} {set item model}
label .l.e.ln -text "Insert new label for the $item [lindex $lmn $i]"
set app "[lindex $lmn $i]"

entry .l.e.n -width 30
.l.e.n insert 1 "[file tail [lindex $lmn $i]]"
label .l.e.ld -text "Insert a new description"
frame .l.e.t
scrollbar .l.e.t.yscroll -command ".l.e.t.text yview"
text .l.e.t.text -wrap word -width 60 -relief sunken -yscrollcommand ".l.e.t.yscroll set"

pack .l.e.ln .l.e.n .l.e.ld -fill x
pack .l.e.t.yscroll -side right -fill y
pack .l.e.t.text -expand yes -fill both
pack .l.e.t
frame .l.e.b
button .l.e.b.ok -width -9 -text Ok -command {set app "[.l.e.n get]"; if { [lindex $group $result] == 1} {set f [open [lindex $ldn $result]/groupinfo.txt w]} {set f [open [lindex $ldn $result]/modelinfo.txt w]}; puts -nonewline $f "$app"; close $f; set f [open [lindex $ldn $result]/description.txt w]; puts -nonewline $f [.l.e.t.text get 0.0 end]; close $f; destroytop .l.e; showmodel [lindex $lrn $result]}
button .l.e.b.esc -width -9 -text Cancel -command {destroytop .l.e; showmodel [lindex $lrn $result]}
pack .l.e.b.ok .l.e.b.esc -padx 10 -pady 10 -side left
pack .l.e.b

.l.e.t.text insert end "[lindex $lmd $i]"
set result $i
focus -force .l.e.n
.l.e.n selection range 0 end
bind .l.e.n <Return> {focus -force .l.e.t.text; .l.e.t.text mark set insert 1.0}
bind .l.e <Escape> {.l.e.b.esc invoke}
bind .l.e.t.text <Control-e> {.l.e.b.ok invoke}

showtop .l.e centerS
}


#################################
# Paste a previously copied model/group
################################
proc paste i {
global copydir copyver copylabel copydscr lrn modelgroup lmn lver lmd choiceSM

set pastedir [lindex $lrn $i]

newtop .l.p "Paste Model" { .l.b.esc invoke }

update


label .l.p.ln -text "Insert a label for the copy of model: $copylabel"
entry .l.p.n -width 30
.l.p.n insert 0 "$copylabel"

label .l.p.lv -text "Insert a new version number for the new model"
entry .l.p.v -width 30
.l.p.v insert 0 "$copyver"

label .l.p.ld -text "Insert a directory name for the new model to be created in [lindex $lrn $i]"
entry .l.p.d -width 30
.l.p.d insert 0 "[file tail $copydir]"

label .l.p.ldsc -text "Insert a description for the new model"

frame .l.p.t
scrollbar .l.p.t.yscroll -command ".l.p.t.text yview"
text .l.p.t.text -wrap word -width 60 -relief sunken -yscrollcommand ".l.p.t.yscroll set"
.l.p.t.text insert end "$copydscr"

pack .l.p.ln .l.p.n .l.p.lv .l.p.v .l.p.ld .l.p.d .l.p.ldsc -fill x

pack .l.p.t.yscroll -side right -fill y
pack .l.p.t.text -expand yes -fill both
pack .l.p.t
frame .l.p.b

button .l.p.b.ok -width -9 -text Ok -command {set choiceSM 1}
button .l.p.b.esc -width -9 -text Cancel -command {set choiceSM 2}
pack .l.p.b.ok .l.p.b.esc -padx 10 -pady 10 -side left
pack .l.p.b

showtop .l.p centerS

focus -force .l.p.n
bind .l.p.n <Return> {focus -force .l.p.v}
bind .l.p.v <Return> {focus -force .l.p.d}
bind .l.p.d <Return> {focus -force .l.p.b.ok}
bind .l.p.b.ok <Return> {.l.p.b.ok invoke}
bind .l.p <Escape> {.l.p.b.esc invoke}

tkwait variable choiceSM

if { $choiceSM == 2 } { } {

  set appd [.l.p.d get]
  set appv [.l.p.v get]
  set appl [.l.p.n get]
  set appdsc "[.l.p.t.text get 1.0 end]"
  
  set confirm [tk_messageBox -parent .l.p -type okcancel -icon question -title Confirmation -default ok -message "Confirm copy?" -detail "Every file in dir.:\n$copydir\n is going to be copied in dir.:\n$pastedir/$appd"]
  if { $confirm == "ok" } {
    set app [file exists $pastedir/$appd]
    if { $app == 1} {tk_messageBox -parent .l.p -title Error -icon error -type ok -message "Copy error" -detail "Directory $pastedir/$appd already exists.\nSpecify a different directory." } {
       #viable directory name 
       file mkdir $pastedir/$appd
       set copylist [glob $copydir/*]
       foreach a $copylist {catch [file copy -force $a $pastedir/$appd]}
       set f [open $pastedir/$appd/description.txt w]
       puts -nonewline $f "$appdsc"
       close $f
       set f [open $pastedir/$appd/modelinfo.txt w]
       puts $f "$appl"
       puts $f "$appv"
       set frmt "%d %B, %Y"
       puts $f "[clock format [clock seconds] -format "$frmt"]"
       close $f
     } 
  
} {}
}
destroytop .l.p
set choiceSM 0
showmodel [lindex $lrn $i]

}
