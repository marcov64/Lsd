#*************************************************************
#
#	LSD 8.0 - May 2021
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
# THEME.TCL
# Procedures to manage interface theming.
#
# Available themes are stored in subdirectory src/themes
# and can be customized. New items must be added to the
# themeTable array below.
#*************************************************************

#************************************************
# ISDARKTHEME
# Check for a system dark theme in use
# Redefine standard styles when necessary
# In Linux, only some themes are detected
#************************************************
proc isDarkTheme { } {
	global tcl_platform CurPlatform darkThemeSuffixes winManLinux RootLsd LsdSrc env

	if [ string equal $CurPlatform mac ] {
		update idletasks
		
		if [ tk::unsupported::MacWindowStyle isdark . ] {
			return 1
		}
		
		return 0
		
	} elseif [ string equal $CurPlatform windows ] {
		if { ! [ catch { set AppsUseLightTheme [ registry get HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize AppsUseLightTheme ] } ] } {
			if { ! $AppsUseLightTheme } {
				return 1
			} else {
				return 0
			}
		}
		
		return -1
		
	} elseif [ string equal $CurPlatform linux ] {
		set wm ""
        set theme ""
		set desktop1 ""
		set desktop2 ""
		
		if { [ info exists env(XDG_CURRENT_DESKTOP) ] } {
			set desktop1 [ string tolower $env(XDG_CURRENT_DESKTOP) ]
		}
		
		if { [ info exists env(DESKTOP_SESSION) ] } {
			set desktop2 [ string tolower $env(DESKTOP_SESSION) ]
		} 
		
		foreach m $winManLinux {
			if { [ string first $m $desktop1 ] >= 0 || \
				 [ string first $m $desktop2 ] >= 0 } {
				set wm $m
			} 
		}
		
		if { ( $wm eq "" || $wm eq "gnome" ) && \
			 ! [ catch { exec gsettings get org.gnome.desktop.interface gtk-theme } results ] } {
			set theme $results

		} elseif { ( $wm eq "" || $wm eq "kde" || $wm eq "plasma" ) && \
				   [ file exists "~/.config/kdeglobals" ] } {
			source "$RootLsd/$LsdSrc/ini.tcl" ;	# load config file reader
			if { ! [ catch { set f [ ini::open "~/.config/kdeglobals" ] } ] && \
				 ! [ catch { ini::value $f General Name } results ] } {
				set theme $results
			}
		} elseif { ( $wm eq "" || $wm eq "xfce" ) && \
				   ! [ catch { exec xfconf-query -c xsettings -p /Net/ThemeName } results ] } {
			set theme $results
			
		} elseif { ( $wm eq "" || $wm eq "cinnamon" ) && \
				   ! [ catch { exec gsettings get org.cinnamon.desktop.interface gtk-theme } results ] } {
			set theme $results

		} elseif { ( $wm eq "" || $wm eq "mate" ) && \
				   ! [ catch { exec gsettings get org.mate.interface gtk-theme } results ] } {
			set theme $results

		} elseif { ( $wm eq "" || $wm eq "lxde" ) && \
				   [ file exists "~/.config/openbox/lxde-rc.xml" ] } {
			set f [ open "~/.config/openbox/lxde-rc.xml" ]
			set rc [ string tolower [ read $f ] ]
			close $f

			set thStart [ string first "<theme>" $rc ]
			set thEnd [ string first "</theme>" $rc ]
			
			if { $thStart >= 0 && $thEnd > $thStart } {
				set nameStart [ string first "<name>" $rc $thStart ]
				set nameEnd [ string first "</name>" $rc $thStart ]
				if { $nameStart > $thStart && $nameStart < $thEnd && \
					 $nameEnd > $nameStart + 6 && $nameEnd < $thEnd } {
					catch { set theme [ string range $rc [ expr $nameStart + 6 ] [ expr $nameEnd - 1 ] ] }
				}
			}
		} elseif { ( $wm eq "" || $wm eq "lxqt" ) && \
				   [ file exists "~/.config/lxqt/lxqt.conf" ] } {
			source "$RootLsd/$LsdSrc/ini.tcl" ;	# load config file reader
			if { ! [ catch { set f [ ini::open "~/.config/lxqt/lxqt.conf" ] } ] && \
				 ! [ catch { ini::value $f Qt style } results ] } {
				set theme $results
			}
		}

		if { $theme ne "" } {
			foreach n $darkThemeSuffixes {
				if { [ string first $n [ string tolower $theme ] ] >= 0 } {
					return 1
				}
			}
	   
		    return 0

		} else {
			return -1
		}
	}
}


#************************************************
# UPDATETHEME
# Check the current system theme and
# adjust the default and current theme if
# necessary. Only change current theme
# if default theme is selected
#************************************************
proc updateTheme { } {
	global CurPlatform DefaultTheme lsdTheme themeMac themeLinux \
		   themeLinuxDark themeWindows themeWindowsDark
	
	if [ string equal $CurPlatform mac ] {
		set DefaultTheme $themeMac

	} elseif [ string equal $CurPlatform windows ] {
		set dark [ isDarkTheme ]
		if { $dark == 1 } {
			set DefaultTheme $themeWindowsDark
			if { ! [ info exists lsdTheme ] || $lsdTheme eq $themeWindows } {
				set lsdTheme $themeWindowsDark
			}
		} elseif { $dark == 0 } {
			set DefaultTheme $themeWindows
			if { ! [ info exists lsdTheme ] || $lsdTheme eq $themeWindowsDark } {
				set lsdTheme $themeWindows
			}
		} else {
			set DefaultTheme $themeWindows
			if { ! [ info exists lsdTheme ] } {
				set lsdTheme $themeWindows
			}
		}
	} elseif [ string equal $CurPlatform linux ] {
		set dark [ isDarkTheme ]
		if { $dark == 1 } {
			set DefaultTheme $themeLinuxDark
			if { ! [ info exists lsdTheme ] || $lsdTheme eq $themeLinux } {
				set lsdTheme $themeLinuxDark
			}
		} elseif { $dark == 0 } {
			set DefaultTheme $themeLinux
			if { ! [ info exists lsdTheme ] || $lsdTheme eq $themeLinuxDark } {
				set lsdTheme $themeLinux
			}
		} else {
			set DefaultTheme $themeLinux
			if { ! [ info exists lsdTheme ] } {
				set lsdTheme $themeLinux
			}
		}
	}
}


#************************************************
# SETSTYLES
# Create common used derived ttk styles
# Redefine standard styles when necessary
#************************************************
proc setstyles { } {
	global lsdTheme themeTable colorsTheme fonttype dim_character small_character
	
	# toolbutton widget styles
	if { [ array names themeTable -exact $lsdTheme ] != "" } {
		set tbpadh [ lindex $themeTable($lsdTheme) 4 ]
		set tbpadv [ lindex $themeTable($lsdTheme) 5 ]
	} else {
		set tbpadh 3
		set tbpadv 3
	}
	ttk::style configure Toolbutton -anchor center -padding "$tbpadh $tbpadv"
	ttk::style configure bold.Toolbutton \
		-font [ font create -weight bold ]
	ttk::style configure hlBold.Toolbutton -anchor w -foreground $colorsTheme(hl) \
		-font [ font create -size $small_character -weight bold ]
	ttk::style map hlBold.Toolbutton -foreground [ list disabled $colorsTheme(hl) ] \
		-background [ list disabled [ ttk::style lookup Toolbutton -background !disabled ] ] \
		-font [ list disabled [ font create -size $small_character ] ] 
	
	# button widget styles
	ttk::style configure TButton -anchor center
	ttk::style configure center.TButton -width -1
	ttk::style configure small.TButton -padding 0 \
		-font [ font create -size $small_character ]

	# entry widget styles
	ttk::style configure TEntry -justify center

	# text widget styles (manually managed in ttk::text below)
	ttk::style configure TText \
		-font TkTextFont
	ttk::style configure boldSmallProp.TText \
		-font [ font create -family TkDefaultFont -size $small_character -weight bold ]
	ttk::style configure fixed.TText \
		-font [ font create -family "$fonttype" -size $dim_character ]
	ttk::style configure smallFixed.TText \
		-font [ font create -family "$fonttype" -size $small_character ]
	
	# label widget styles
	ttk::style configure TLabel -anchor center
	ttk::style configure hl.TLabel -foreground $colorsTheme(hl)
	ttk::style configure sel.TLabel -foreground $colorsTheme(sfg) \
		-background $colorsTheme(sbg)
	ttk::style configure selHl.TLabel -foreground $colorsTheme(hl) \
		-background $colorsTheme(sbg)
	ttk::style configure bold.TLabel \
		-font [ font create -weight bold ]
	ttk::style configure hlBold.TLabel -foreground $colorsTheme(hl) \
		-font [ font create -weight bold ]
	ttk::style configure boldSmall.TLabel \
		-font [ font create -size $small_character -weight bold ]
	ttk::style configure hlBoldSmall.TLabel -foreground $colorsTheme(hl) \
		-font [ font create -size $small_character -weight bold ]
	ttk::style configure graySmall.TLabel -foreground gray \
		-font [ font create -size $small_character ]
}


#************************************************
# SETCOLOR
# Set the color(s) of a widget element
#************************************************
proc setcolor { w args } {
	global colorsTheme
	array set options $args
	
	foreach { element color } [ array get options ] {
		if { [ array names colorsTheme -exact $color ] != "" && \
			 ! [ catch { winfo rgb . $colorsTheme($color) } ] } {
			$w configure $element $colorsTheme($color)
		}
	}
}


#************************************************
# SETTAB
# Adjust tab size according to font type and size
#************************************************
proc settab { w size style } {
	set font [ ttk::style lookup $style -font ]
	set tabwidth "[ expr { $size * [ font measure "$font" 0 ] } ] left"
	$w conf -font "$font" -tabs $tabwidth -tabstyle wordprocessor
}


#************************************************
# TTK::TEXT
# Create a ttk-like themed text widget
#************************************************
proc ttk::text { w args } {
	array set options [ concat { -entry 1 -dark 0 -style "" } $args ]
	set entry $options(-entry)
	set dark $options(-dark)
	set style $options(-style)
	array unset options "-entry"
	array unset options "-dark"
	array unset options "-style"
	
	::text $w {*}[ array get options ]
	
	$w configure -relief flat -highlightthickness 1

	if { $style != "" } {
		catch { $w configure -font [ ttk::style lookup $style -font ] }
	}
	
	if { $entry } {
		$w configure -undo 1 -highlightthickness 1
		if { $dark } {
			setcolor $w -background dbg -foreground fg
		} else {
			setcolor $w -background ebg -foreground efg
		}
	} else {
		$w configure -highlightthickness 0 -cursor "" -insertofftime 1 -insertontime 0
		if { $dark } {
			setcolor $w -background bg -foreground fg
		} else {
			setcolor $w -background bg -foreground efg
		}
	}
	
	setcolor $w -selectbackground sbg \
				-selectforeground sfg \
				-inactiveselectbackground isbg \
				-highlightcolor hc \
				-highlightbackground bg \
				-insertbackground fg
}


#************************************************
# TTK::LISTBOX
# Create a ttk-like themed listbox widget
#************************************************
proc ttk::listbox { w args } {
	array set options [ concat { -dark 0 } $args ]
	set dark $options(-dark)
	array unset options "-dark"

	::listbox $w {*}[ array get options ]

	$w configure -relief flat -borderwidth 0
	
	if { $dark } {
		setcolor $w -background dbg
	} else {
		setcolor $w -background ebg
	}
	
	setcolor $w -foreground fg \
				-selectbackground sbg \
				-selectforeground sfg \
				-disabledforeground dfg \
				-highlightcolor hc \
				-highlightbackground bg
}


#************************************************
# TTK::MENU
# Create a ttk-like themed menu widget
#************************************************
proc ttk::menu { w args } {
	::menu $w {*}$args

	$w configure -relief flat -activeborderwidth 0

	setcolor $w -background bg \
				-foreground fg \
				-activebackground sbg \
				-activeforeground sfg \
				-disabledforeground dfg \
				-selectcolor efg 
}


#************************************************
# TTK::CANVAS
# Create a ttk-like themed canvas widget
#************************************************
proc ttk::canvas { w args } {
	array set options [ concat { -entry 1 -dark 0 } $args ]
	set entry $options(-entry)
	set dark $options(-dark)
	array unset options "-entry"
	array unset options "-dark"

	::canvas $w {*}[ array get options ]

	$w configure -relief flat -borderwidth 0 -highlightthickness 0
	
	if { $entry } {
		if { $dark } {
			setcolor $w -background dbg
		} else {
			setcolor $w -background ebg
		}
	} else {
		if { $dark } {
			setcolor $w -background bg
		} else {
			setcolor $w -background bg
		}
	}
	
	setcolor $w -selectbackground sbg \
				-selectforeground sfg \
				-highlightcolor hc \
				-highlightbackground bg \
				-insertbackground fg
}


#************************************************
# TTK::MESSAGEBOX
# Create a ttk-like themed message box
#************************************************

# current status of message box
set ttk::msgBoxName ""
set ttk::msgBoxValue ""

proc ttk::messageBox { args } {
	global CurPlatform
	
	if [ string equal $CurPlatform mac ] {
		array set options $args
		if { "-parent" in [ array names options ] && $options(-parent) == "" } {
			array unset options "-parent"
		}
		
		return [ tk_messageBox {*}[ array get options ] ]
	}
	
	array set options [ concat { -default "" -detail "" -icon info -message "" -parent . -title "" -type ok } $args ]

	set name [ string cat .msgBox_ [ expr { int( rand( ) * 10000 ) } ] ]
	
	foreach { option parameter } [ array get options ] {
		switch $option {
			-default {
				if { ! ( $parameter in { ok yes no cancel ignore abort retry } ) } {
					set options(-default) ""
				}				
			}
			-detail { } 
			-icon {
				if { ! ( $parameter in { error info question warning } ) } {
					set options(-icon) info
				}				
			} 
			-message { } 
			-parent {
				if { $parameter != "." } {
					if [ winfo exists $parameter ] {
						set name ${parameter}${name}
					}
				}
			} 
			-title { } 
			-type {
				if { ! ( $parameter in { abortretryignore ok okcancel retrycancel yesno yesnocancel } ) } {
					set options(-type) ok
				}				
			} 
			default {
				error "Unrecognized option"
			}
		}
	}
	
	if { $options(-default) == "" } {
		switch $options(-type) {
			abortretryignore {
				set options(-default) abort
			} 
			ok {
				set options(-default) ok
			} 
			yesno {
				set options(-default) no
			} 
			okcancel -
			retrycancel -
			yesnocancel {
				set options(-default) cancel
			}
		}
	}

	return [ ttk::messageBox_draw $name $options(-icon) $options(-title) $options(-parent) \
								  $options(-message) $options(-detail) $options(-type) \
								  $options(-default) ]
}

proc ttk::messageBox_exit { value } {
	global ttk::msgBoxName ttk::msgBoxValue
	destroytop $ttk::msgBoxName
	set ttk::msgBoxName ""
	set ttk::msgBoxValue $value
}

proc ttk::messageBox_draw { name icon title parent message detail type default } {
	global ttk::msgBoxName ttk::msgBoxValue errorDlgImg infoDlgImg questDlgImg warnDlgImg
	
	destroytop $ttk::msgBoxName
	set ttk::msgBoxName $name
	
	newtop $name $title "" $parent
	
	ttk::frame $name.top
	
	ttk::frame $name.top.icon
		
	switch $icon {
		error {
			ttk::label $name.top.icon.label -image errorDlgImg
		}
		info {
			ttk::label $name.top.icon.label -image infoDlgImg
		}
		question {
			ttk::label $name.top.icon.label -image questDlgImg
		}
		warning {
			ttk::label $name.top.icon.label -image warnDlgImg
		}
		default {
			ttk::label $name.top.icon.label -image infoDlgImg
		}
	}
	
	pack $name.top.icon.label -pady 5 -anchor nw
	
	ttk::frame $name.top.text
	ttk::label $name.top.text.message -wraplength 305 \
				-justify left -text $message -style boldSmall.TLabel
	ttk::label $name.top.text.details -wraplength 305  \
				-justify left -text $detail
	pack $name.top.text.message \
		 $name.top.text.details -pady 5 -anchor nw
		 
	pack $name.top.icon $name.top.text \
		 -padx 5 -side left -anchor nw
	pack $name.top -padx 15 -pady 15
	
	switch $type {
		abortretryignore {
			abortretryignore  $name bottom { ttk::messageBox_exit abort } { ttk::messageBox_exit retry } { ttk::messageBox_exit ignore }
			set close abort
		}
		ok {
			ok $name bottom { ttk::messageBox_exit ok }
			set close ok
		} 
		okcancel {
			okcancel $name bottom { ttk::messageBox_exit ok } { ttk::messageBox_exit cancel }
			set close cancel
		} 
		retrycancel {
			retrycancel $name bottom { ttk::messageBox_exit retry } { ttk::messageBox_exit cancel }
			set close cancel
		} 
		yesno {
			yesno $name bottom { ttk::messageBox_exit yes } { ttk::messageBox_exit no }
			set close no
		} 
		yesnocancel {
			yesnocancel $name bottom { ttk::messageBox_exit yes } { ttk::messageBox_exit no } { ttk::messageBox_exit cancel }
			set close cancel
		}
	}
	
	pack $name.bottom -side right
	
	wm protocol $name WM_DELETE_WINDOW "ttk::messageBox_exit $close"
	
	if { $parent == "" } {
		showtop $name centerS no no no 0 0 "" no yes
	} else {
		showtop $name centerW
	}
	
	
	if [ winfo exists $name.bottom.$default ] {
		$name.bottom.$default configure -default active
		mousewarpto $name.bottom.$default
	}
	
	if [ string equal $icon error ] {
		bell
	}
	
	vwait ttk::msgBoxValue
	return $ttk::msgBoxValue
}
