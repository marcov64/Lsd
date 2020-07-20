package require Tk 8.6
set theme_dir [ file join [ pwd ] [ file dirname [ info script ] ] ]

array set themes {
	aquablue 0.1
	blueelegance 0.1
	sriv 0.5
	waldorf 0.1
}

foreach { theme version } [ array get themes ] {
	package ifneeded ttk::theme::$theme $version \
		[ list source [ file join $theme_dir $theme.tcl ] ]
}
