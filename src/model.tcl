#*************************************************************
#
#	LSD 9.0 - January 2024
#	written by Marco Valente, Universita' dell'Aquila
#	and by Marcelo Pereira, University of Campinas
#
#	Copyright Marco Valente and Marcelo Pereira
#	LSD is distributed under the GNU General Public License
#
#	See Readme.txt for copyright information of
#	third parties' code used in LSD
#
#*************************************************************

#*************************************************************
# MODEL.TCL
# Tcl scripts used to manage models in LSD.
#
# The script starts from the LSD root and reads the model info or group info files in all
# the descending directories. Users can browse through the model, create new models,
# create new groups, or delete them.
#
# The implementation defines few global lists into which are stored the relevant
# data. As returning value, the proceduce sets the global variable "result"
# to the number of the list chosen and the global lists containing the references
# in the group selected
#*************************************************************

set result 0
set memory 0
set months [ list January February March April May June July August September October November December ]


#************************************************
# SHOWMODEL
#************************************************
proc showmodel { groupdir { modeldir "" } } {
	global lmn lmd ldn lrn lbn result choiceSM lver rootname group group_new group_dir model_group model_dir browser_dir upSymbol groupSymbol lsd_root lsd_example lsd_trash selstate newstate editstate copystate pastestate delstate curpastestate curcopystate memory small_character GROUP_TXT_INFO MODEL_TXT_INFO GROUP_XML_CONFIG MODEL_XML_CONFIG DESCRIPTION colorsTheme darkTheme

	unset -nocomplain lmn lver lmd ldn lrn lbn group

	# lists to hold directory model and group data for each entry
	lappend lmn		; # model/group name
	lappend lver	; # model version (-1 for groups)
	lappend lmd		; # model/group description
	lappend ldn		; # model/group directory
	lappend lrn		; # model/group parent directory
	lappend lbn		; # model/group parent name
	lappend group	; # model = 0 / group = 1 flag

	set browser_dir $groupdir

	if { [ string first "$lsd_root/$lsd_example" [ file normalize $groupdir ] ] == 0 } {
		set example 1
	} else {
		set example 0
	}

	if { [ string first "[ file dirname [ file normalize $group_new ] ]/$lsd_trash" [ file normalize $groupdir ] ] == 0 } {
		set trash 1
	} else {
		set trash 0
	}

	if { [ get_group_setting $groupdir name ] eq $rootname } {
		set root 1
	} else {
		set root 0
	}

	set selstate normal
	set newstate normal
	set editstate normal
	set copystate normal
	set pastestate normal
	set delstate normal

	set curpastestate disabled
	set curcopystate normal

	if { $example } {
		set newstate disabled
		set editstate disabled
		set pastestate disabled
		set delstate disabled
	}

	if { $trash } {
		set selstate disabled
		set newstate disabled
		set editstate disabled
		set pastestate disabled
	}

	if { $root } {
		set copystate disabled
		set pastestate disabled
	}

	if { $memory } {
		set curpastestate $pastestate
	}

	if [ winfo exists .l ] {
		.l.l.l delete 0 end
		.l.t.text conf -state normal
		.l.t.text delete 1.0 end

		.l.m.file entryconf 1 -state $newstate
		.l.m.edit entryconf 0 -state $editstate
		.l.m.edit entryconf 1 -state $curcopystate
		.l.m.edit entryconf 2 -state $curpastestate
		.l.m.edit entryconf 3 -state $delstate

		.l.l.l.m entryconf 1 -state $newstate
		.l.l.l.m entryconf 3 -state $editstate
		.l.l.l.m entryconf 4 -state $curcopystate
		.l.l.l.m entryconf 5 -state $curpastestate
		.l.l.l.m entryconf 6 -state $delstate

		# close tool tip if still showing
		tooltip::hide
	} else {
		newtop .l "LSD Model Browser" { .l.m.file invoke 2 }

		ttk::menu .l.m -tearoff 0

		set m .l.m.file
		ttk::menu $m -tearoff 0
		.l.m add cascade -label File -menu $m -underline 0
		$m add command -label "Select Model/Group" -underline 0 -accelerator Enter -command {
			set result [ .l.l.l curselection ]
			if { [ lindex $group $result ] == 0 } {
				if { $selstate eq "normal" } {
					set model_dir [ lindex $ldn $result ]
					set choiceSM 1
				}
			} else {
				showmodel [ lindex $ldn $result ]
			}
		}

		$m add command -label "New Model/Group..." -underline 0 -state $newstate -accelerator Ins -command {
			set result -1
			set memory 0
			set choiceSM 14
		}

		$m add command -label Quit -underline 0 -accelerator Esc -command {
			set result -1
			set memory 0
			set choiceSM 2
		}

		set m .l.m.edit
		ttk::menu $m -tearoff 0
		.l.m add cascade -label Edit -menu $m -underline 0
		$m add command -label "Edit Name/Description..." -underline 0 -state $editstate -accelerator Ctrl+E -command {
			set result [ .l.l.l curselection ]
			medit $result
		}

		$m add command -label "Copy" -underline 0 -state $curcopystate -accelerator Ctrl+C -command {
			set result [ .l.l.l curselection ]
			mcopy $result
		}

		$m add command -label "Paste" -underline 0 -accelerator Ctrl+V -state $curpastestate -command {
			set result [ .l.l.l curselection ]
			mpaste $result
		}

		$m add command -label "Delete..." -underline 0 -state $delstate -accelerator Del -command {
			set result [ .l.l.l curselection ]
			if { [ lindex $group $result ] != -1 } {
				mdelete $result
			}
		}

		set m .l.m.help
		ttk::menu $m -tearoff 0
		.l.m add cascade -label Help -menu $m -underline 0
		$m add command -label "Help" -underline 0 -accelerator F1 -command {
			LsdHelp modelbrowser.html
		}

		$m add command -label "LSD Documentation" -underline 4 -command {
			LsdHelp LSD_documentation.html
		}

		$m add separator

		$m add command -label "Citing LSD..." -underline 0 -command { LsdCiting $_LSD_DATE_ .l }
		$m add command -label "About LSD..." -underline 0 -command { LsdAbout $_LSD_VERSION_ $_LSD_DATE_ .l }

		.l configure -menu .l.m

		ttk::frame .l.bbar
		ttk::button .l.bbar.new -image newImg -style Toolbutton -command  { .l.m.file invoke 1 }
		ttk::button .l.bbar.edit -image editImg -style Toolbutton -command  { .l.m.edit invoke 0 }
		ttk::button .l.bbar.copy -image copyImg -style Toolbutton -command  { .l.m.edit invoke 1 }
		ttk::button .l.bbar.paste -image pasteImg -style Toolbutton -command { .l.m.edit invoke 2 }
		ttk::button .l.bbar.delete -image deleteImg -style Toolbutton -command { .l.m.edit invoke 3 }
		ttk::button .l.bbar.help -image helpImg -style Toolbutton -command { .l.m.help invoke 0 }

		tooltip::tooltip .l.bbar.new "New Model/Group..."
		tooltip::tooltip .l.bbar.edit "Edit Name/Description..."
		tooltip::tooltip .l.bbar.copy "Copy"
		tooltip::tooltip .l.bbar.paste "Paste"
		tooltip::tooltip .l.bbar.delete "Delete..."
		tooltip::tooltip .l.bbar.help "Help"

		pack .l.bbar.new .l.bbar.edit .l.bbar.copy .l.bbar.paste .l.bbar.delete .l.bbar.help -side left
		pack .l.bbar -padx $::_3 -anchor w -fill x

		ttk::frame .l.l

		ttk::frame .l.l.tit
		ttk::label .l.l.tit.g -text "Current group:"
		ttk::label .l.l.tit.n -style hl.TLabel
		pack .l.l.tit.g .l.l.tit.n -side left

		pack .l.l.tit -pady $::_3 -anchor w

		ttk::scrollbar .l.l.vs -command ".l.l.l yview"
		ttk::listbox .l.l.l -height 15 -width 30 -yscroll ".l.l.vs set" -selectmode browse -dark $darkTheme
		mouse_wheel .l.l.l
		pack .l.l.vs -side right -fill y
		pack .l.l.l -expand yes -fill both

		ttk::menu .l.l.l.m -tearoff 0
		.l.l.l.m  add command -label Select -accelerator Enter -command { .l.m.file invoke 0 }; #entryconfig 0
		.l.l.l.m  add command -label New -accelerator Ins -state $newstate -command { .l.m.file invoke 1 }; #entryconfig 1
		.l.l.l.m  add separator; #entryconfig 2
		.l.l.l.m  add command -label Edit -accelerator Ctrl+E -state $editstate -command { .l.m.edit invoke 0 }; #entryconfig 3
		.l.l.l.m  add command -label Copy -accelerator Ctrl+C -state $curcopystate -command { .l.m.edit invoke 1 }; #entryconfig 4
		.l.l.l.m  add command -label Paste -accelerator Ctrl+V -state $curpastestate -command { .l.m.edit invoke 2 }; #entryconfig 5
		.l.l.l.m  add command -label Delete -accelerator Del -state $delstate -command { .l.m.edit invoke 3 }; #entryconfig 6

		ttk::frame .l.t
		ttk::label .l.t.tit -text Description -anchor center
		pack .l.t.tit -pady $::_3 -expand yes -fill x
		ttk::scrollbar .l.t.yscroll -command ".l.t.text yview"
		ttk::text .l.t.text -wrap word -width 60 -yscrollcommand ".l.t.yscroll set" -entry 0 -dark $darkTheme -style smallFixed.TText
		pack .l.t.yscroll -side right -fill y
		pack .l.t.text -expand yes -fill both
		mouse_wheel .l.t.text

		pack .l.l .l.t -expand yes -fill both -side left

		bind .l <F1> { .l.m.help invoke 0 }
		bind .l <Control-e> { .l.m.edit invoke 0 }
		bind .l <Control-E> { .l.m.edit invoke 0 }
		bind .l <Control-c> { .l.m.edit invoke 1 }
		bind .l <Control-C> { .l.m.edit invoke 1 }
		bind .l <Control-v> { .l.m.edit invoke 2 }
		bind .l <Control-V> { .l.m.edit invoke 2 }
		bind .l <Delete> { .l.m.edit invoke 3 }
		bind .l <Escape> { .l.m.file invoke 2 }
		bind .l <Insert> { .l.m.file invoke 1 }
		bind .l <Return> { .l.m.file invoke 0 }

		bind .l <KeyRelease> {
			if { ( %s & 0x20004 ) != 0 } {
				return
			}
			set kk %K
			if { [ string equal $kk underscore ] || ( [ string length $kk ] == 1 && [ string is alpha -strict $kk ] ) } {
				if [ string equal $kk underscore ] {
					set kk _
				}
				set ll %W
				set ff [ lsearch -start [ expr { [ $ll curselection ] + 1 } ] -nocase [ $ll get 0 end ] "${kk}*" ]
				if { $ff == -1 } {
					set ff [ lsearch -start 0 -nocase [ $ll get 0 end ] "${kk}*" ]
				}
				if { $ff >= 0 } {
					selectinlist $ll $ff
				}
			}

			break
		}

		bind .l <Up> {
			set app [ .l.l.l curselection ]
			.l.t.text conf -state normal
			.l.t.text delete 0.0 end
			.l.t.text insert end [ lindex $lmd $app ]
			.l.t.text conf -state disable
		}

		bind .l <Down> {
			set app [ .l.l.l curselection ]
			.l.t.text conf -state normal
			.l.t.text delete 0.0 end
			.l.t.text insert end [ lindex $lmd $app ]
			.l.t.text conf -state disable
		}

		bind .l <Home> {
			set app 0
			selectinlist .l.l.l $app
			.l.t.text conf -state normal
			.l.t.text delete 0.0 end
			.l.t.text insert end [ lindex $lmd $app ]
			.l.t.text conf -state disable
			break
		}

		bind .l <End> {
			set app end
			selectinlist .l.l.l $app
			.l.t.text conf -state normal
			.l.t.text delete 0.0 end
			.l.t.text insert end [ lindex $lmd $app ]
			.l.t.text conf -state disable
			break
		}

		bind .l.l.l <Double-Button-1> { set dblclk 1; .l.m.file invoke 0 }

		bind .l.l.l <Button-1> {
			set dblclk 0
			after 200
			if { ! $dblclk } {
				set app [ .l.l.l nearest %y ]
				selectinlist .l.l.l $app
				.l.t.text conf -state normal
				.l.t.text delete 0.0 end
				.l.t.text insert end [ lindex $lmd $app ]
				.l.t.text conf -state disable
			}
		}

		bind .l.l.l <Button-2> {
			.l.l.l selection clear 0 end
			.l.l.l selection set [ .l.l.l nearest %y ]
			if { ! [ catch { set name [ selection get ] } ] } {
				if { [ string equal -length [ string length $groupSymbol ] $name $groupSymbol ] || [ string equal -length [ string length $upSymbol ] $name $upSymbol ] } {
					.l.l.l.m entryconf 4 -state disabled
				} else {
					.l.l.l.m entryconf 4 -state $copystate
				}
			} else {
				.l.l.l.m entryconf 4 -state disabled
			}
			if { $memory } {
				.l.l.l.m entryconf 5 -state $pastestate
			} else {
				.l.l.l.m entryconf 5 -state disabled
			}
			tk_popup .l.l.l.m %X %Y
		}

		bind .l.l.l <Button-3> {
			event generate .l.l.l <2> -x %x -y %y
		}

		showtop .l centerW no no yes 0 0 "" no yes
	}

	tooltip::tooltip clear .l.l.l*

	set root_groupdir [ file dirname [ file normalize $group_new ] ]
	if { ! [ file isdirectory $groupdir ] || ( [ string first $root_groupdir [ file normalize $groupdir ] ] != 0 && ! $example && ! $trash ) } {
		set groupdir $root_groupdir
		set groupname $rootname
	} else {
		set groupname [ get_group_setting $groupdir name ]
	}

	.l.l.tit.n conf -text $groupname

	set curdir [ pwd ]
	cd $groupdir

	if { [ file normalize $groupdir ] eq $root_groupdir } {
		# show examples tree that is in main LSD directory
		if { [ file exists "$lsd_root/$lsd_example/$GROUP_TXT_INFO" ] || [ file exists "$lsd_root/$lsd_example/$GROUP_XML_CONFIG" ] } {
			set app [ get_group_setting "$lsd_root/$lsd_example" name ]
			set appd [ get_group_setting "$lsd_root/$lsd_example" description ]
			if { $appd eq "" } {
				set appd "Group: $app\n(description not available)"
			}

			lappend lmn $app
			lappend lver -1
			lappend ldn "$lsd_root/$lsd_example"
			lappend lrn $root_groupdir
			lappend lbn $groupname
			lappend lmd $appd
			lappend group 1
			.l.l.l insert end "$groupSymbol$app"
			.l.l.l itemconf end -fg $colorsTheme(grp)

			tooltip::tooltip .l.l.l -item [ expr { [ .l.l.l index end ] - 1 } ] "[ file nativename $lsd_root/$lsd_example ]"
		}
	} else {
		# show UP icon to move to parent group if not root
		set updir [ file dirname [ pwd ] ]
		set upgroup [ get_group_setting $updir name ]

		lappend lver -1
		lappend lmd "Return to group: $upgroup"
		lappend lrn $groupdir
		lappend lbn $groupname
		lappend lmn $upgroup
		lappend group -1
		.l.l.l insert end $upSymbol

		if { [ file normalize "$lsd_root/$lsd_example" ] eq [ file normalize $groupdir ] } {
			lappend ldn $root_groupdir
		} else {
			lappend ldn [ file dirname $groupdir ]
		}

		tooltip::tooltip .l.l.l -item [ expr { [ .l.l.l index end ] - 1 } ] $upgroup
	}

	set dir [ lsort -dictionary [ glob -nocomplain -type d * ] ]

	# list groups
	foreach i $dir {
		if { ! ( [ file exists "$i/$MODEL_TXT_INFO" ] || [ file exists "$i/$MODEL_XML_CONFIG" ] ) && ( [ file exists "$i/$GROUP_TXT_INFO" ] || [ file exists "$i/$GROUP_XML_CONFIG" ] ) && [ file normalize "$groupdir/$i" ] ne [ file normalize "$lsd_root/$lsd_example" ] } {
			set app [ get_group_setting $i name ]
			set appd [ get_group_setting $i description ]
			if { $appd eq "" } {
				set appd "Group: $app\n(description not available)"
			}

			lappend lmn $app
			lappend lver -1
			lappend ldn "$groupdir/$i"
			lappend lrn $groupdir
			lappend lbn $groupname
			lappend lmd $appd
			lappend group 1
			.l.l.l insert end "$groupSymbol$app"
			.l.l.l itemconf end -fg $colorsTheme(grp)

			tooltip::tooltip .l.l.l -item [ expr { [ .l.l.l index end ] - 1 } ] "[ file nativename $groupdir/$i ]"
		}
	}

	set selpos 0

	# list models
	foreach i $dir {
		if { [ file exists "$i/$MODEL_TXT_INFO" ] || [ file exists "$i/$MODEL_XML_CONFIG" ] } {

			set mn [ get_model_setting $i "model_name" ]
			if { $mn eq "" } {
				set mn [ file tail $i ]
			}

			set ver [ get_model_setting $i "model_version" ]
			if { $ver eq "" } {
				set ver "0.0"
			}

			lappend lmn $mn
			lappend lver $ver
			lappend ldn "$groupdir/$i"
			lappend lrn $groupdir
			lappend lbn $groupname

			if [ file exists "$i/$DESCRIPTION" ] {
				set f [ open "$i/$DESCRIPTION" ]
				lappend lmd "[ read -nonewline $f ]"
				close $f
			} else {
				lappend lmd "Model: $mn\nin directory: [ file nativename $groupdir/$i ]\n(description not available)"
			}

			lappend group 0
			.l.l.l insert end "$mn (v. $ver)"
			.l.l.l itemconf end -fg $colorsTheme(mod)

			if { "$modeldir" ne "" && [ file normalize "$modeldir" ] eq [ file normalize "$groupdir/$i" ] } {
				set selpos [ expr { [ .l.l.l index end ] - 1 } ]
			}

			tooltip::tooltip .l.l.l -item [ expr { [ .l.l.l index end ] - 1 } ] [ file nativename $groupdir/$i ]
		}
	}

	cd $curdir

	.l.t.text insert end [ lindex $lmd 0 ]
	.l.t.text conf -state disable
	selectinlist .l.l.l $selpos
	focus .l.l.l
	update
}


#************************************************
# MCOPY
# Copy a model position for future pastes
#************************************************
proc mcopy i {
	global copylabel copyver copydir copydscr group ldn pastestate memory lmn lver lmd

	if { [ lindex $group $i ] == 0 } {
		set memory 1

		.l.m.edit entryconf 2 -state $pastestate

		set copylabel [ lindex $lmn $i ]
		set copyver [ lindex $lver $i ]
		set copydir [ lindex $ldn $i ]
		set copydscr [ lindex $lmd $i ]
	} else {
		ttk::messageBox -parent .l -title Error -type ok -icon error -message "Cannot copy groups" -detail "Check for existing names and try again."
	}
}


#************************************************
# MDELETE
# Remove a model/group, placing it in a trashbin
#************************************************
proc mdelete i {
	global lrn ldn lmn group group_new group_new lsd_root lsd_example lsd_trash memory model_name model_group GROUP_TXT_INFO GROUP_XML_CONFIG

	if { [ file normalize [ lindex $ldn $i ] ] eq [ file normalize "$lsd_root/$lsd_example" ] || [ file normalize [ lindex $ldn $i ] ] eq [ file normalize $group_new ] } {
		ttk::messageBox -parent .l -title Error -icon error -type ok -message "Cannot delete group" -detail "The group '[ lindex $lmn $i ]' cannot be deleted."
		return
	}

	if { [ lindex $group $i ] == 0 } {
		set item model
	} else {
		set item group
	}

	set trashbin "[ file dirname [ file normalize $group_new ] ]/$lsd_trash"

	if { [ string match -nocase "$trashbin*" [ lindex $ldn $i ] ] } {
		set answer [ ttk::messageBox -parent .l -type yesno -title Confirmation -icon question -default yes -message "Confirm deletion?" -detail "Do you want to delete $item '[ lindex $lmn $i ]' at\n\n[ file nativename [ lindex $ldn $i ] ]" ]
		catch { file delete -force [ lindex $ldn $i ] }
		showmodel [ lindex $lrn $i ]
	} else {
		if { $item eq "model" && $model_name eq [ lindex $lmn $i ] } {
			ttk::messageBox -parent .l -title Error -icon error -type ok -message "Cannot delete model" -detail "The current model '[ lindex $lmn $i ]' at\n\n[ file nativename [ lindex $ldn $i ] ]\n\ncannot be deleted.\n\nPlease close it or choose another model, and try again."
			return
		}

		if { $item eq "group" && $model_group eq [ get_group_setting [ lindex $ldn $i ] name ] } {
			ttk::messageBox -parent .l -title Error -icon error -type ok -message "Cannot delete group" -detail "The group containing the current model '[ get_group_setting [ lindex $ldn $i ] name ]' at\n\n[ file nativename [ lindex $ldn $i ] ]\n\ncannot be deleted.\n\nPlease close current or choose another model in a different group, and try again."
			return
		}

		set answer [ ttk::messageBox -parent .l -type yesno -title Confirmation -icon question -default yes -message "Confirm deletion?" -detail "Do you want to delete $item '[ lindex $lmn $i ]' at\n\n[ file nativename [ lindex $ldn $i ] ]" ]

		set memory 0
		.l.m.edit entryconf 2 -state disabled

		if { $answer eq "yes" } {
			if { ! [ file exists $trashbin ] } {
				file mkdir $trashbin
			}

			if { ! [ file exists "$trashbin/$GROUP_XML_CONFIG" ] } {
				set_group_setting $trashbin name "Deleted Models"
				set_group_setting $trashbin description "Deleted Models: folder containing deleted models.\n\nModels here can be recovered by moving them back to any existing group."
			}

			set name [ string range [ lindex $ldn $i ] [ expr { [ string last / [ lindex $ldn $i ] ] + 1 } ] end ]
			if { [ file exists "$trashbin/$name" ] } {
				if { [ ttk::messageBox -parent .l -type yesno -title Confirmation -icon question -default yes -message "Duplicated deleted model or group" -detail "There is another item named '[ lindex $lmn $i ]' at\n\n[ file nativename [ lindex $ldn $i ] ]\n\nin the deleted models group.\n\nDo you want to proceed and permanently delete the older item?" ] } {
					catch { file delete -force "$trashbin/$name" }
				}
			}
			if { [ catch { file rename -force [ lindex $ldn $i ] "$trashbin/$name" } ] } {
				ttk::messageBox -parent .l -title Error -icon error -type ok -message "Delete error" -detail "Directory\n\n[ file nativename [ lindex $ldn $i ] ]\n\ncannot be deleted now.\n\nYou may try again later."
			}

			showmodel [ lindex $lrn $i ]
		}
	}
}


#************************************************
# MEDIT
# Edit the model/group name and description
#************************************************
proc medit i {
	global lrn ldn lmn group lmd result memory small_character darkTheme MODEL_TXT_INFO MODEL_XML_CONFIG DESCRIPTION

	set memory 0
	.l.m.edit entryconf 2 -state disabled

	set result $i

	if { [ lindex $group $i ] == 0 } {
		set item model
	} else {
		set item group
	}

	newtop .l.e "Edit" { .l.e.b.can invoke }

	ttk::frame .l.e.tit
	ttk::label .l.e.tit.l -text "Current $item:"
	ttk::label .l.e.tit.n -text "[ lindex $lmn $i ]" -style hl.TLabel
	pack .l.e.tit.l  .l.e.tit.n -side left -padx $::_2

	ttk::frame .l.e.n
	ttk::label .l.e.n.l -text "Name"
	ttk::entry .l.e.n.n -width 25 -justify center
	.l.e.n.n insert 1 "[ lindex $lmn $i ]"
	pack .l.e.n.l  .l.e.n.n

	ttk::frame .l.e.t
	ttk::label .l.e.t.l -text "Description"
	ttk::frame .l.e.t.t
	ttk::scrollbar .l.e.t.t.yscroll -command ".l.e.t.t.text yview"
	ttk::text .l.e.t.t.text -wrap word -width 60 -height 20 -yscrollcommand ".l.e.t.t.yscroll set" -dark $darkTheme -style smallFixed.TText
	pack .l.e.t.t.yscroll -side right -fill y
	pack .l.e.t.t.text
	mouse_wheel .l.e.t.t.text
	pack .l.e.t.l .l.e.t.t

	pack .l.e.tit .l.e.n .l.e.t -padx $::_5 -pady $::_5

	okcancel .l.e b {
		if { [ lindex $group $result ] == 0 } {
			if { [ file exists "[ lindex $ldn $result ]/$MODEL_TXT_INFO" ] || [ file exists "[ lindex $ldn $result ]/$MODEL_XML_CONFIG" ] } {
				set_model_setting [ lindex $ldn $result ] "model_name" [ .l.e.n.n get ]
			}

			set f [ open "[ lindex $ldn $result ]/$DESCRIPTION" w ]
			puts -nonewline $f [ .l.e.t.t.text get 0.0 end ]
			close $f
		} else {
			set_group_setting [ lindex $ldn $result ] name [ .l.e.n.n get ]
			set_group_setting [ lindex $ldn $result ] description [ .l.e.t.t.text get 0.0 end ]
		}

		if { [ lindex $group $result ] == 0 } {
			set newname [ .l.e.n.n get ]
		}

		destroytop .l.e
		showmodel [ lindex $lrn $result ] [ lindex $ldn $result ]
	} {
		destroytop .l.e
		showmodel [ lindex $lrn $result ] [ lindex $ldn $result ]
	}

	bind .l.e.n.n <Return> {
		focus .l.e.t.t.text
		.l.e.t.t.text mark set insert 1.0
	}

	showtop .l.e
	mousewarpto .l.e.b.ok 0
	.l.e.t.t.text insert end [ lindex $lmd $i ]
	.l.e.n.n selection range 0 end
	focus .l.e.n.n
}


#************************************************
# MPASTE
# Paste a previously copied model/group
#************************************************
proc mpaste i {
	global copydir copyver copylabel copydscr lrn lmn lver lmd choiceSM small_character darkTheme MODEL_TXT_INFO MODEL_XML_CONFIG DESCRIPTION

	set pastedir [ lindex $lrn $i ]

	newtop .l.p "Paste Model" { set choiceSM 2 }

	ttk::frame .l.p.tit

	ttk::frame .l.p.tit.t1
	ttk::label .l.p.tit.t1.l -text "Original model:"
	ttk::label .l.p.tit.t1.n -text $copylabel -style hl.TLabel
	pack .l.p.tit.t1.l  .l.p.tit.t1.n -side left -padx $::_2

	ttk::frame .l.p.tit.t2
	ttk::label .l.p.tit.t2.l -text "Current group:"
	ttk::label .l.p.tit.t2.n -text [ lindex $lrn $i ] -style hl.TLabel
	pack .l.p.tit.t2.l  .l.p.tit.t2.n -side left -padx $::_2

	pack .l.p.tit.t1  .l.p.tit.t2

	ttk::frame .l.p.n
	ttk::label .l.p.n.l -text "New name"
	ttk::entry .l.p.n.n -width 25 -justify center
	.l.p.n.n insert 0 $copylabel
	pack .l.p.n.l  .l.p.n.n

	ttk::frame .l.p.v
	ttk::label .l.p.v.l -text "Version"
	ttk::entry .l.p.v.v -width 10 -justify center
	.l.p.v.v insert 0 $copyver
	pack .l.p.v.l  .l.p.v.v

	ttk::frame .l.p.d
	ttk::label .l.p.d.l -text "New (non-existing) home directory name"
	ttk::entry .l.p.d.d -width 35 -justify center
	.l.p.d.d insert 0 [ file tail $copydir ]
	pack .l.p.d.l  .l.p.d.d

	ttk::frame .l.p.t
	ttk::label .l.p.t.l -text "Model description"

	ttk::frame .l.p.t.t
	ttk::scrollbar .l.p.t.t.yscroll -command ".l.p.t.t.text yview"
	ttk::text .l.p.t.t.text -wrap word -width 60 -height 20 -yscrollcommand ".l.p.t.t.yscroll set" -dark $darkTheme -style smallFixed.TText
	pack .l.p.t.t.yscroll -side right -fill y
	pack .l.p.t.t.text
	mouse_wheel .l.p.t.t.text
	pack .l.p.t.l .l.p.t.t

	pack .l.p.tit .l.p.n .l.p.v .l.p.d .l.p.t -padx $::_5 -pady $::_5

	okcancel .l.p b { set choiceSM 1 } { set choiceSM 2 }

	bind .l.p.n.n <Return> { focus .l.p.v.v; .l.p.v.v selection range 0 end }
	bind .l.p.v.v <Return> { focus .l.p.d.d; .l.p.d.d selection range 0 end }
	bind .l.p.d.d <Return> { focus .l.p.t.t.text; .l.p.t.t.text mark set insert 1.0 }

	showtop .l.p
	mousewarpto .l.p.b.ok 0
	.l.p.t.t.text insert end $copydscr
	.l.p.n.n selection range 0 end
	focus .l.p.n.n

	set newdir ""
	set choiceSM 0
	tkwait variable choiceSM

	if { $choiceSM == 1 } {
		set appd [ .l.p.d.d get ]
		set appv [ .l.p.v.v get ]
		set appl [ .l.p.n.n get ]
		set appdsc [ .l.p.t.t.text get 1.0 end ]

		set confirm [ ttk::messageBox -parent .l.p -type okcancel -icon question -title Confirmation -default ok -message "Confirm copy?" -detail "Every file in directory\n\n[ file nativename $copydir ]\n\nis going to be copied to directory\n\n[ file nativename $pastedir/$appd ]" ]
		if { $confirm == "ok" } {
			if { [ file exists $pastedir/$appd ] } {
				ttk::messageBox -parent .l.p -title Error -icon error -type ok -message "Copy error" -detail "Directory\n\n[ file nativename $pastedir/$appd ]\n\nalready exists.\n\nSpecify a different directory and try again."
			} else {
				file mkdir $pastedir/$appd
				set copylist [ glob -nocomplain "$copydir/*" ]
				foreach a $copylist {
					catch [ file copy -force $a "$pastedir/$appd" ]
				}

				set_model_setting "$pastedir/$appd" "model_name" $appl
				set_model_setting "$pastedir/$appd" "model_version" $appv
				set_model_setting "$pastedir/$appd" "model_date" [ clock format [ clock seconds ] -format "%d %B, %Y" ]

				set f [ open "$pastedir/$appd/$DESCRIPTION" w ]
				puts -nonewline $f "$appdsc"
				close $f

				set newdir $appd
			}
		}
	}

	destroytop .l.p
	set choiceSM 0
	showmodel [ lindex $lrn $i ] $newdir
}
