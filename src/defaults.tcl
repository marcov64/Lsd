#*************************************************************
#
#	LSD 8.0 - March 2021
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
# DEFAULTS.TCL
# Default values for the Tk windowing system. 
#*************************************************************

# default colors in palette and other elements, according to the theme type (light/dark)
set defcolorsL	{ black red green #d0d000 #fb46bc blue DeepSkyBlue1 RoyalBlue1 PaleTurquoise2 cyan aquamarine DarkSeaGreen1 chartreuse1 OliveDrab khaki3 LightGoldenrod4 sienna1 chocolate4 firebrick3 orange1 salmon3 }
set defcolorsD	{ white tomato green yellow2 pink "steel blue" DeepSkyBlue1 RoyalBlue1 PaleTurquoise2 cyan aquamarine DarkSeaGreen1 chartreuse1 OliveDrab khaki3 LightGoldenrod4 sienna1 chocolate4 firebrick3 orange1 salmon3 }

set hlcolorL	red			; # general highlight color
set hlcolorD	tomato			; # general highlight color
set commcolorL	green4		; # color of comments (light mode)
set commcolorD	DarkOliveGreen3	; # color of comments (dark mode)
set strcolorL	blue4		; # color of strings (light mode)
set strcolorD	RoyalBlue	; # color of strings (dark mode)
set prepcolorL	SaddleBrown	; # color of C preprocessor (light mode)
set prepcolorD	tan1		; # color of C preprocessor (dark mode)
set typecolorL	DarkViolet	; # color of C type (light mode)
set typecolorD	Cyan3		; # color of C type (dark mode)
set kwrdcolorL	purple4		; # color of C keyword (light mode)
set kwrdcolorD	"medium orchid"	; # color of C keyword (dark mode)
set vlsdcolorL	red4		; # color of LSD variable (light mode)
set vlsdcolorD	tomato		; # color of LSD variable (dark mode)
set mlsdcolorL	DodgerBlue4	; # color of LSD macro (light mode)
set mlsdcolorD	SlateGray3	; # color of LSD macro (dark mode)
set parcolorL	black		; # color of parameter label (light mode)
set parcolorD	white		; # color of parameter label (dark mode)
set varcolorL	blue		; # color of variable label (light mode)
set varcolorD	tan1		; # color of variable label (dark mode)
set lvarcolorL	purple		; # color of lagged variable label (light mode)
set lvarcolorD	DarkOliveGreen3	; # color of lagged variable label (dark mode)
set funcolorL	firebrick	; # color of function label (light mode)
set funcolorD	RoyalBlue	; # color of function label (dark mode)
set lfuncolorL	tomato		; # color of lagged function label (light mode)
set lfuncolorD	"medium orchid"	; # color of lagged function label (dark mode)
set objcolorL	red			; # color of object label (light mode)
set objcolorD	tomato		; # color of object label (dark mode)
set grpcolorL	red			; # color of model group label (light mode)
set grpcolorD	tomato		; # color of model group label (dark mode)
set modcolorL	blue		; # color of model label (light mode)
set modcolorD	SlateGray3	; # color of model label (dark mode)

# OS specific minimum window sizes (horizontal & vertical) (must be even numbers)
set hsizeBminMac		520	; # browser window
set vsizeBminMac		600
set hsizeBminLinux		460
set vsizeBminLinux		600
set hsizeBminWindows	420
set vsizeBminWindows	600
set hsizeGminMac		670	; # log window
set vsizeGminMac		360
set hsizeGminLinux		670
set vsizeGminLinux		400
set hsizeGminWindows	620
set vsizeGminWindows	360
set hsizeAminMac		860	; # analysis of results window
set vsizeAminMac		500
set hsizeAminLinux		850
set vsizeAminLinux		530
set hsizeAminWindows	720
set vsizeAminWindows	460
set hsizePminMac		860	; # analysis of results plot window
set vsizePminMac		580
set hsizePminLinux		730
set vsizePminLinux		560
set hsizePminWindows	720
set vsizePminWindows	540
set hsizeDminMac		942	; # debugger/data browse window
set vsizeDminMac		400
set hsizeDminLinux		840
set vsizeDminLinux		400
set hsizeDminWindows	740
set vsizeDminWindows	400

# main windows size (must be even numbers)
set hsizeL		800	; # LMM horizontal size in pixels
set vsizeL		600	; # LMM vertical size in pixels
set hsizeLmin	800	; # LMM minimum horizontal size in pixels
set vsizeLmin	600	; # LMM minimum vertical size in pixels
set hsizeM 		600	; # model structure horizontal size in pixels
set vsizeM		360	; # model structure vertical size in pixels
set hsizeNmin	300	; # objects numbers editor minimum horizontal size in pixels
set vsizeNmin	550	; # objects numbers editor minimum vertical size in pixels
set hsizeImin	300	; # initial values editor minimum horizontal size in pixels
set vsizeImin	400	; # initial values editor minimum vertical size in pixels
set hsizeP 		600	; # default plot (time series) horizontal size in pixels
set vsizeP		300	; # default plot (time series) vertical size in pixels
set hsizePxy	700	; # default plot (XY) horizontal size in pixels
set vsizePxy	400	; # default plot (XY) vertical size in pixels
set hsizeLatMax	1024; # maximum lattice horizontal size in pixels
set vsizeLatMax	1024; # maximum lattice vertical size in pixels
set hsizeLat	420	; # default lattice horizontal size in pixels
set vsizeLat	420	; # default lattice vertical size in pixels
set hsizeR		500	; # horizontal size in pixels
set vsizeR		300	; # vertical size in pixels

# main windows margins
set hmargin		20	; # horizontal right margin from the screen borders
set vmargin		20	; # vertical margins from the screen borders
set bordsize	2	; # width of windows borders
set vmenusize	20	; # height of system menu bar
set tbarsize	60	; # size in pixels of bottom taskbar (exclusion area)
set frPadX		3	; # padding area (X) for frame borders
set frPadY		2	; # padding area (Y) for frame borders

# model structure margins and defaults
set borderM		20	; # open space to border
set nsizeM		20	; # node size (diameter)
set vmarginM	20	; # labels margins to nodes
set vstepM		75	; # vertical absolute step
set hfactM		1.0	; # initial horizontal scaling factor
set vfactM		1.0	; # initial vertical scaling factor
set hfactMmin	0.5	; # minimum horizontal scaling factor
set vfactMmin	0.7	; # minimum vertical scaling factor
set rstepM		0.1	; # relative scaling factor step step
set rfactM		0.3	; # horizontal range exponential factor
set rinitM		750	; # horizontal initial width (4 root sons)
set rincrM		100	; # horizontal width increase step

# runtime windows margins and defaults
set botvsizeR	50	; # bottom part height
set sclhsizeR	75	; # scale width
set cvhmarginR	8	; # horizontal margin for canvas
set botvmarginR	30	; # vertical margin for bottom part
set sclvmarginR	3	; # vertical margin for scale
set hticksR		5	; # number of horizontal run-time plot legend ticks
set vticksR		2	; # number of vertical run-time plot legend ticks
set ticmarginR	5	; # margin for legend ticks
set lablinR		5	; # labels per line
set linlabR		3	; # lines of label
set linvsizeR	18	; # label lines height
set pdigitsR	3	; # default precision digits
set shiftR		20	; # new window shift

# debug/data browse windows margins and defaults
set vspcszD		5	; # line horizontal space size (pixels)
set hnamshD		0.20; # variable name horizontal share (1 column out of 2)
set hvalshD		0.15; # variable value horizontal share (1 column out of 2)
set hupdshD		0.10; # variable last update horizontal share (1 column out of 2)
set hpadshD		0.10; # column pad horizontal share (1 column)

# analysis of results windows margins and defaults
set hmbordsizeP	40	; # minimum width of plot horizontal (left/right) border area (for legends)
set tbordsizeP	5	; # width of plot top border area (for legends)
set bbordsizeP	90	; # width of plot bottom border area (for legends)
set sbordsizeP	0	; # width of plot scroll border area
set htmarginP	4	; # horizontal margin for legends text in plots
set vtmarginP	2	; # vertical margin for legends text in plots
set grayscaleP	0	; # default color mode for plots (0=color)
set gridP		1	; # defaut grid mode (0=no grid)
set linemodeP	1	; # default line mode (1:lines/2:points)
set pdigitsP	3	; # default precision digits
set pointsizeP	1.0	; # default size of points/lines (<1:./=1:x<=2:+/>2:round)
set maxzoomP	4.0	; # maximum zoom magnification factor in plots
set minzoomP	0.5	; # minimum zoom shrinking factor in plots
set parAll	"(all)"	; # string used for selecting all series parents

# lattice defaults
set cscaleLat	1.0	; # default color scale (1:1.0)

# mouse defaults
set sfmwheel	1	; # increase to accelerate mouse wheel and decrease to slow down
set winmwscale	30	; # scroll minimum wheel movement (precision) factor in Windows
set mouseWarp	1	; # set to 0 to disable auto-snapping (ignored in Windows)
	
# internal plot defaults
set fontP		{ Helvetica 9 normal }; # default plot text font/size/mode
set hticksP		3	; # number of horizontal plot legend ticks
set vticksP		3	; # number of vertical plot legend ticks
set avgSmplP	1	; # average Y values when multi-sampling (0=no/1=yes)
set showInitP	0	; # use initial values (t-1) in plots/data (0=no/1=yes)
set smoothP		no	; # options are no, yes or raw
set splstepsP 	3	; # number of spline segments when smoothP is raw

# gnuplot defaults
set gnuplotGrid3D		"60,60,3"
set gnuplotOptions		"set ticslevel 0.0"

# folder/group/object symbols
set upSymbol 			"\u25B2 .."	; # up in tree structure symbol
set groupSymbol			"\u25B6 "	; # group symbol

# OS specific default interface theme
set themeMac			"aqua"		; # native macOS theme
set themeLinux			"awlight"	; # Adwaita light theme
set themeLinuxDark		"awdark"	; # Adwaita dark theme
set themeWindows		"vista"		; # native Windows light theme
set themeWindowsDark	"awblack"	; # Windows dark theme
set darkThemeSuffixes	[ list dark black obscure ]; # words identifying Linux dark themes

# OS specific monospaced font name
set fontMac				"Monaco"			; # "Courier"
set fontLinux			"DejaVu Sans Mono"	; # "Courier"
set fontWindows			"Consolas"			; # "Courier"

# OS specific monospaced font size
set fontSizeMac			14
set fontSizeLinux		11
set fontSizeWindows		11

# OS specific monospaced small font size delta (negative)
set deltaSizeMac		1
set deltaSizeLinux		1
set deltaSizeWindows	2

# OS specific default window button width, spacing and padding
set butWidMac			7 ; # button width in characters
set butWidLinux			8
set butWidWindows		9
set butSpcMac			4 ; # button spacing in pixels
set butSpcLinux			5
set butSpcWindows		5
set butPadMac 			10 ; # padding area (X/Y) for buttons in pixels
set butPadLinux 		10
set butPadWindows 		10

# OS specific button spacing in model structure window
set bhstepMac			27 ; # button horizontal step
set bhstepLinux			35
set bhstepWindows		25
set bvstepMac			26 ; # button vertical step
set bvstepLinux			30
set bvstepWindows		26
set bborderMac			10 ; # additional open space to border
set bborderLinux		0
set bborderWindows		0

# OS specific screen location offset adjustments
set corrXmac			0
set corrYmac			0
set corrXlinux			0
set corrYlinux			-67
set corrXwindows		0
set corrYwindows		0

# OS specific default model executable name
set exeLinux 			"LSD"
set exeMac 				"LSD"
set exeWindows 			"LSD"

# OS specific default system terminal
set sysTermMac			"Terminal"	; # "Terminal", "xterm"
set sysTermLinux		"xterm"		; # "gnome-terminal", "xterm", "uxterm"
set sysTermWindows		"cmd"		;

# OS specific default debugger command
set dbgMac				"lldb"		; # "gdb"
set dbgLinux			"gdb"		;
set dbgWindows			"gdb"		;

# OS specific default browser (open=system default)
set browserMac			"open"		; # "open", "firefox", "safari"
set browserLinux		"firefox"	; # "open", "firefox", "chrome"
set browserWindows		"open"		; # "open", "firefox", "chrome"

# OS specific default wish utility
set wishMac				"wish8.6"
set wishLinux			"wish"
set wishWindows			"wish86.exe"

# OS specific make utility
set makeMac				"make"
set makeLinux			"make"
set makeWinCygwin		"make.exe"
set makeWinMingw		"mingw32-make.exe"

# OS specific default gnuplot terminal (empty string=gnuplot default)
set gnuplotTermMac		""	; # "qt", "x11"
set gnuplotTermLinux	""	; # "qt", "wxt", "x11"
set gnuplotTermWindows	""	; # "wxt", "qt" , "windows"

# default diff application settings
set diffApp				"tkdiff.tcl"; # command line diff application to use
set diffAppType			0			; # type of application (0=tk/1=terminal/2=graphical)
set diffFile1			""			; # option to inform first file name		
set diffFile2			""			; # option to inform second file name		
set diffFile1name		"-L"		; # option for naming first file
set diffFile2name		"-L"		; # option for naming second file
set diffOptions			"-lsd"		; # other options

# known themes table and associated parameters
# theme list elements:		0:plat		1:pkg name				2:full name			3:dark	4:tbph	5:tbpv
set themeTable(alt) 	   	{ all		ttk::theme::alt			"Alt"				0		1	 	1	}
set themeTable(clam) 	   	{ all		ttk::theme::clam		"Clam"				0		2	 	2	}
set themeTable(classic)    	{ all		ttk::theme::classic		"X11 Classic"		0		1	 	1	}
set themeTable(default)    	{ all		ttk::theme::default		"Default"			0		3	 	3	}
set themeTable(aqua) 	   	{ mac		ttk::theme::aqua		"macOS Aqua"		0		0	 	0	}
set themeTable(vista)      	{ windows	ttk::theme::vista		"Windows Vista"		0		4	 	4	}
set themeTable(winnative)  	{ windows	ttk::theme::winnative	"Windows NT"		0		2	 	2	}
set themeTable(awarc)		{ all		awarc		    	 	"Arc"				0		-4	 	-3	}
set themeTable(awblack)    	{ all		awblack    				"Black"				1		2		3	}
set themeTable(awdark)     	{ all		awdark					"Adwaita Dark"		1		2	 	2	}
set themeTable(awlight)    	{ all		awlight					"Adwaita Light"		0		2	 	2	}
set themeTable(awbreeze)   	{ all		awbreeze 			  	"Breeze"			0		-6	 	-3	}
set themeTable(awbreezedark) { all		awbreezedark			"Breeze Dark"		1		-6	 	-3	}
set themeTable(awclearlooks) { all		awclearlooks			"Clearlooks"		0		0	 	0	}
set themeTable(awwinxpblue) { all		awwinxpblue				"XP Blue"			0		2	 	2	}
set themeTable(aquablue)   	{ all		ttk::theme::aquablue 	"Aqua Blue"			0		-7	 	-7	}
set themeTable(equilux)    	{ all		ttk::theme::equilux  	"Equilux"			1		0	 	0	}
set themeTable(keramik_alt)	{ all		ttk::theme::keramik_alt	"Keramik Alt"		0		2	 	2	}
set themeTable(ubuntu)	   	{ all		ttk::theme::ubuntu	 	"Radiance"			0		-10	 	-10	}
set themeTable(scidblue)   	{ all		ttk::theme::scid 		"Scid Blue"			0		-10	 	-10	}
set themeTable(scidgreen)  	{ all		ttk::theme::scid		"Scid Green"		0		-10	 	-10	}
set themeTable(scidgrey)   	{ all		ttk::theme::scid 		"Scid Grey"			0		-10	 	-10	}
set themeTable(scidmint)   	{ all		ttk::theme::scid 		"Scid Mint"			0		-10	 	-10	}
set themeTable(scidpink)   	{ all		ttk::theme::scid 		"Scid Pink"			0		-10	 	-10	}
set themeTable(scidpurple) 	{ all		ttk::theme::scid		"Scid Purple"		0		-10	 	-10	}
set themeTable(scidsand)   	{ all		ttk::theme::scid 		"Scid Sand"			0		-10	 	-10	}
set themeTable(waldorf)	   	{ all		ttk::theme::waldorf	 	"Waldorf"			0		-7	 	-7	}
set themeTable(yaru)	   	{ all		ttk::theme::yaru	 	"Yaru"				0		0	 	0	}

#set themeTable(arc)			{ all		ttk::theme::arc    	 	"Arc (old)"			0		0		0	}
#set themeTable(aquativo)   	{ all		ttk::theme::aquativo 	"Aquativo (broken)"	0		2	 	2	}
#set themeTable(black)      	{ all		ttk::theme::black    	"Black (old)"		1		2	 	2	}
#set themeTable(blue)   	   	{ all		ttk::theme::blue   	 	"Blue (broken)"		0		2	 	2	}
#set themeTable(blueelegance) { all		ttk::theme::blueelegance "Blue Elegance (broken)" 0	3	 	3	}
#set themeTable(breeze)     	{ all		ttk::theme::breeze   	"Breeze (old)"		0		1		1	}
#set themeTable(clearlooks) 	{ all		ttk::theme::clearlooks	"Clearlooks (old)"	0		-7		-7	}
#set themeTable(elegance)   	{ all		ttk::theme::elegance 	"Elegance (broken)"	0		3	 	3	}
#set themeTable(itft1)      	{ all		ttk::theme::itft1    	"ITFT1 (broken)"	0		2	 	2	}
#set themeTable(keramik)    	{ all		ttk::theme::keramik  	"Keramik (broken)"	0		2	 	2	}
#set themeTable(kroc)   	   	{ all		ttk::theme::kroc   	 	"Kroc (broken)"		0		2	 	2	}
#set themeTable(plastik)    	{ all		ttk::theme::plastik  	"Plastik (broken)"	0		0	 	0	}
#set themeTable(smog)       	{ all		ttk::theme::smog     	"Smog (broken)"		0		3	 	3	}
#set themeTable(sriv)       	{ all		ttk::theme::sriv     	"Sriv (broken)"		0		1	 	1	}
#set themeTable(winxpblue)  	{ all		ttk::theme::winxpblue	"XP Blue (old)"		0		2		2	}

# list of all Tk named colors
set allcolors {
	snow {ghost white} {white smoke} gainsboro {floral white}
	{old lace} linen {antique white} {papaya whip} {blanched almond}
	bisque {peach puff} {navajo white} moccasin cornsilk ivory
	{lemon chiffon} seashell honeydew {mint cream} azure {alice blue}
	lavender {lavender blush} {misty rose} white black {dark slate gray}
	{dim gray} {slate gray} {light slate gray} gray {light grey}
	{midnight blue} navy {cornflower blue} {dark slate blue} {slate blue}
	{medium slate blue} {light slate blue} {medium blue} {royal blue}
	blue {dodger blue} {deep sky blue} {sky blue} {light sky blue}
	{steel blue} {light steel blue} {light blue} {powder blue}
	{pale turquoise} {dark turquoise} {medium turquoise} turquoise
	cyan {light cyan} {cadet blue} {medium aquamarine} aquamarine
	{dark green} {dark olive green} {dark sea green} {sea green}
	{medium sea green} {light sea green} {pale green} {spring green}
	{lawn green} green chartreuse {medium spring green} {green yellow}
	{lime green} {yellow green} {forest green} {olive drab} {dark khaki}
	khaki {pale goldenrod} {light goldenrod yellow} {light yellow} yellow
	gold {light goldenrod} goldenrod {dark goldenrod} {rosy brown}
	{indian red} {saddle brown} sienna peru burlywood beige wheat
	{sandy brown} tan chocolate firebrick brown {dark salmon} salmon
	{light salmon} orange {dark orange} coral {light coral} tomato
	{orange red} red {hot pink} {deep pink} pink {light pink}
	{pale violet red} maroon {medium violet red} {violet red}
	magenta violet plum orchid {medium orchid} {dark orchid} {dark violet}
	{blue violet} purple {medium purple} thistle snow2 snow3
	snow4 seashell2 seashell3 seashell4 AntiqueWhite1 AntiqueWhite2
	AntiqueWhite3 AntiqueWhite4 bisque2 bisque3 bisque4 PeachPuff2
	PeachPuff3 PeachPuff4 NavajoWhite2 NavajoWhite3 NavajoWhite4
	LemonChiffon2 LemonChiffon3 LemonChiffon4 cornsilk2 cornsilk3
	cornsilk4 ivory2 ivory3 ivory4 honeydew2 honeydew3 honeydew4
	LavenderBlush2 LavenderBlush3 LavenderBlush4 MistyRose2 MistyRose3
	MistyRose4 azure2 azure3 azure4 SlateBlue1 SlateBlue2 SlateBlue3
	SlateBlue4 RoyalBlue1 RoyalBlue2 RoyalBlue3 RoyalBlue4 blue2 blue4
	DodgerBlue2 DodgerBlue3 DodgerBlue4 SteelBlue1 SteelBlue2
	SteelBlue3 SteelBlue4 DeepSkyBlue2 DeepSkyBlue3 DeepSkyBlue4
	SkyBlue1 SkyBlue2 SkyBlue3 SkyBlue4 LightSkyBlue1 LightSkyBlue2
	LightSkyBlue3 LightSkyBlue4 SlateGray1 SlateGray2 SlateGray3
	SlateGray4 LightSteelBlue1 LightSteelBlue2 LightSteelBlue3
	LightSteelBlue4 LightBlue1 LightBlue2 LightBlue3 LightBlue4
	LightCyan2 LightCyan3 LightCyan4 PaleTurquoise1 PaleTurquoise2
	PaleTurquoise3 PaleTurquoise4 CadetBlue1 CadetBlue2 CadetBlue3
	CadetBlue4 turquoise1 turquoise2 turquoise3 turquoise4 cyan2 cyan3
	cyan4 DarkSlateGray1 DarkSlateGray2 DarkSlateGray3 DarkSlateGray4
	aquamarine2 aquamarine4 DarkSeaGreen1 DarkSeaGreen2 DarkSeaGreen3
	DarkSeaGreen4 SeaGreen1 SeaGreen2 SeaGreen3 PaleGreen1 PaleGreen2
	PaleGreen3 PaleGreen4 SpringGreen2 SpringGreen3 SpringGreen4
	green2 green3 green4 chartreuse2 chartreuse3 chartreuse4
	OliveDrab1 OliveDrab2 OliveDrab4 DarkOliveGreen1 DarkOliveGreen2
	DarkOliveGreen3 DarkOliveGreen4 khaki1 khaki2 khaki3 khaki4
	LightGoldenrod1 LightGoldenrod2 LightGoldenrod3 LightGoldenrod4
	LightYellow2 LightYellow3 LightYellow4 yellow2 yellow3 yellow4
	gold2 gold3 gold4 goldenrod1 goldenrod2 goldenrod3 goldenrod4
	DarkGoldenrod1 DarkGoldenrod2 DarkGoldenrod3 DarkGoldenrod4
	RosyBrown1 RosyBrown2 RosyBrown3 RosyBrown4 IndianRed1 IndianRed2
	IndianRed3 IndianRed4 sienna1 sienna2 sienna3 sienna4 burlywood1
	burlywood2 burlywood3 burlywood4 wheat1 wheat2 wheat3 wheat4 tan1
	tan2 tan4 chocolate1 chocolate2 chocolate3 firebrick1 firebrick2
	firebrick3 firebrick4 brown1 brown2 brown3 brown4 salmon1 salmon2
	salmon3 salmon4 LightSalmon2 LightSalmon3 LightSalmon4 orange2
	orange3 orange4 DarkOrange1 DarkOrange2 DarkOrange3 DarkOrange4
	coral1 coral2 coral3 coral4 tomato2 tomato3 tomato4 OrangeRed2
	OrangeRed3 OrangeRed4 red2 red3 red4 DeepPink2 DeepPink3 DeepPink4
	HotPink1 HotPink2 HotPink3 HotPink4 pink1 pink2 pink3 pink4
	LightPink1 LightPink2 LightPink3 LightPink4 PaleVioletRed1
	PaleVioletRed2 PaleVioletRed3 PaleVioletRed4 maroon1 maroon2
	maroon3 maroon4 VioletRed1 VioletRed2 VioletRed3 VioletRed4
	magenta2 magenta3 magenta4 orchid1 orchid2 orchid3 orchid4 plum1
	plum2 plum3 plum4 MediumOrchid1 MediumOrchid2 MediumOrchid3
	MediumOrchid4 DarkOrchid1 DarkOrchid2 DarkOrchid3 DarkOrchid4
	purple1 purple2 purple3 purple4 MediumPurple1 MediumPurple2
	MediumPurple3 MediumPurple4 thistle1 thistle2 thistle3 thistle4
}
