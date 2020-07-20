#*************************************************************
#
#	LSD 7.3 - December 2020
#	written by Marco Valente, Universita' dell'Aquila
#	and by Marcelo Pereira, University of Campinas
#
#	Copyright Marco Valente and Marcelo Pereira
#	LSD is distributed under the GNU General Public License
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
# In Linux, only GTK themes are detected
#************************************************
proc isDarkTheme { platform darkSuffixes } {
	if [ string equal $platform mac ] {
		if { ! [ string equal [ info patchlevel ] 8.6.9 ] } {
			update
			return tk::unsupported::MacWindowStyle isdark .
		}
	} elseif [ string equal $platform linux ] {
		catch { exec gsettings get org.gnome.desktop.interface gtk-theme } results
		foreach namePart $darkSuffixes {
			if { [ string first [ string tolower $results ] $namePart ] >= 0 } {
				return 1
			}
		}
	} elseif [ string equal $platform windows ] {
		package require registry
		if { ! [ catch { set AppsUseLightTheme [ registry get HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize AppsUseLightTheme ] } ] } {
			if { ! $AppsUseLightTheme } {
				return 1
			}
		}
	}
	return 0
}


#************************************************
# UPDATETHEME
# Check the current system theme and
# adjust the default and current theme if
# necessary. Only change current theme
# if default theme is selected
#************************************************
proc updateTheme { } {
	global CurPlatform DefaultTheme lsdTheme darkThemeSuffixes themeMac \
		   themeLinux themeLinuxDark themeWindows themeWindowsDark
	
	if [ string equal $CurPlatform mac ] {
		set DefaultTheme $themeMac
	} elseif [ string equal $CurPlatform linux ] {
		if [ isDarkTheme $CurPlatform $darkThemeSuffixes ] {
			set DefaultTheme $themeLinuxDark
			if [ string equal $lsdTheme $themeLinux ] {
				set lsdTheme $themeLinuxDark
			}
		} else {
			set DefaultTheme $themeLinux
			if [ string equal $lsdTheme $themeLinuxDark ] {
				set lsdTheme $themeLinux
			}
		}
	} elseif [ string equal $CurPlatform windows ] {
		if [ isDarkTheme $CurPlatform $darkThemeSuffixes ] {
			set DefaultTheme $themeWindowsDark
			if [ string equal $lsdTheme $themeWindows ] {
				set lsdTheme $themeWindowsDark
			}
		} else {
			set DefaultTheme $themeWindows
			if [ string equal $lsdTheme $themeWindowsDark ] {
				set lsdTheme $themeWindows
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
	global lsdTheme themeTable small_character
	
	if { [ array names themeTable -exact $lsdTheme ] != "" } {
		set tbpad [ lindex $themeTable($lsdTheme) 4 ]
	} else {
		set tbpad 3
	}
	ttk::style configure Toolbutton -padding "$tbpad $tbpad" 
	
	ttk::style configure small.TText -font [ font create -size $small_character ]
	
	ttk::style configure red.TLabel -foreground red
	ttk::style configure bold.TLabel -font [ font create -weight bold ]
	ttk::style configure boldSmall.TLabel -font [ font create -size $small_character -weight bold ]
	ttk::style configure redBoldSmall.TLabel -foreground red -font [ font create -size $small_character -weight bold ]
	ttk::style configure graySmall.TLabel -foreground gray -font [ font create -size $small_character ]
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
# TTK::TEXT
# Create a ttk-like themed text widget
#************************************************
proc ttk::text { w args } {
	array set options [ concat { -entry 0 -dark 0 -style "" } $args ]
	set entry $options(-entry)
	set dark $options(-dark)
	set style $options(-style)
	array unset options "-entry"
	array unset options "-dark"
	array unset options "-style"
	
	::text $w {*}[ array get options ]
	
	$w configure -relief flat -borderwidth 0
	
	if [ string equal -style small.TText ] {
		$w configure -font [ ttk::style lookup small.TText -font ]
	}
	
	if { $entry } {
		if { $dark } {
			setcolor $w -background dbg -foreground fg
		} else {
			setcolor $w -background ebg -foreground efg
		}
	} elseif { $dark } {
		setcolor $w -background bg -foreground fg
	} else {
		setcolor $w -background bg -foreground efg
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
		setcolor $w -background bg
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

	$w configure -relief flat -borderwidth 0

	setcolor $w -background bg \
				-foreground fg \
				-activebackground sbg \
				-activeforeground sfg \
				-disabledforeground dfg \
				-selectcolor ebg 
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
		return [ tk_messageBox {*}$args ]
	}
	
	array set options [ concat { -default "" -detail "" -icon info -message "" -parent . -title "" -type ok } $args ]

	set name [ concat .msgBox_ [ expr int( rand( ) * 10000 ) ] ]
	
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
						set name ${parameter}.${name}
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
	
	newtop $name $title "ttk::messageBox_exit $default" $parent

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
		}
		ok {
			ok $name bottom { ttk::messageBox_exit ok }
		} 
		okcancel {
			okcancel $name bottom { ttk::messageBox_exit ok } { ttk::messageBox_exit cancel }
		} 
		retrycancel {
			retrycancel $name bottom { ttk::messageBox_exit retry } { ttk::messageBox_exit cancel }
		} 
		yesno {
			yesno $name bottom { ttk::messageBox_exit yes } { ttk::messageBox_exit no }
		} 
		yesnocancel {
			yesnocancel $name bottom { ttk::messageBox_exit yes } { ttk::messageBox_exit no } { ttk::messageBox_exit cancel }
		}
	}
	
	pack $name.bottom -side right
	
	showtop $name centerW
	focus  $name.bottom.$default
	
	vwait ttk::msgBoxValue
	return $ttk::msgBoxValue
}
