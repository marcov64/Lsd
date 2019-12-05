#*************************************************************
#
#	LSD 7.2 - December 2019
#	written by Marco Valente, Universita' dell'Aquila
#	and by Marcelo Pereira, University of Campinas
#
#	Copyright Marco Valente and Marcelo Pereira
#	LSD is distributed under the GNU General Public License
#	
#*************************************************************

#*************************************************************
# DEFAULTS.TCL
# Default values for the Tk windowing system. 
#*************************************************************

package require Tk

# default colors in palette and other elements
set defcolors	{ black red green #d0d000 #fb46bc blue DeepSkyBlue1 grey40 PaleTurquoise2 cyan aquamarine DarkSeaGreen1 chartreuse1 OliveDrab khaki3 LightGoldenrod4 sienna1 chocolate4 firebrick3 orange1 salmon3 }

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
set axcolorP	gray25		; # color of plot axis
set grcolorP	gray75		; # color of plot grid

# main windows size and margins (must be even numbers)
set hmargin		20	; # horizontal right margin from the screen borders
set vmargin		20	; # vertical margins from the screen borders
set bordsize	2	; # width of windows borders
set tbarsize	55	; # size in pixels of bottom taskbar (exclusion area) - Windows 7+ = 82
set hsizeL		800	; # LMM horizontal size in pixels
set vsizeL		600	; # LMM vertical size in pixels
set hsizeLmin	620	; # LMM minimum horizontal size in pixels
set vsizeLmin	300	; # LMM minimum vertical size in pixels
set hsizeB 		400	; # browser horizontal size in pixels
set vsizeB		620	; # browser vertical size in pixels
set hsizeM 		600	; # model structure horizontal size in pixels
set vsizeM		400	; # model structure vertical size in pixels
set hsizeI 		800	; # initial values editor horizontal size in pixels
set vsizeI		600	; # initial values editor vertical size in pixels
set hsizeN 		350	; # objects numbers editor horizontal size in pixels
set vsizeN		550	; # objects numbers editor vertical size in pixels

# plot windows size and margins
set hsizeP 		600	; # default plot (time series) horizontal size in pixels
set vsizeP		300	; # default plot (time series) vertical size in pixels
set hmbordsizeP	40	; # minimum width of plot horizontal (left/right) border area (for legends)
set tbordsizeP	5	; # width of plot top border area (for legends)
set bbordsizeP	90	; # width of plot bottom border area (for legends)
set sbordsizeP	0	; # width of plot scroll border area
set htmarginP	4	; # horizontal margin for legends text in plots
set vtmarginP	2	; # vertical margin for legends text in plots
set hsizePxy	640	; # default plot (XY) horizontal size in pixels
set vsizePxy	450	; # default plot (XY) vertical size in pixels

# internal plot options
set fontP		{ Helvetica 9 normal }; # default plot text font/size/mode
set hticksP		3	; # number of horizontal plot legend ticks
set vticksP		3	; # number of vertical plot legend ticks
set avgSmplP	1	; # average Y values when multi-sampling (0=no/1:yes)
set smoothP		no	; # options are no, yes or raw
set splstepsP 	3	; # number of spline segments when smoothP is raw

# lattice options (maximum & default, color scale)
set hsizeLatMax	1024; # maximum lattice horizontal size in pixels
set vsizeLatMax	1024; # maximum lattice vertical size in pixels
set hsizeLat	400	; # default lattice horizontal size in pixels
set vsizeLat	400	; # default lattice vertical size in pixels
set cscaleLat	1.0	; # default color scale (1:1.0)

# plot windows defaults
set grayscaleP	0	; # default color mode for plots (0=color)
set gridP		0	; # defaut grid mode (0=no grid)
set linemodeP	1	; # default line mode (1:lines/2:points)
set pdigitsP	3	; # default precision digits
set pointsizeP	1.0	; # default size of points (<1:./<2:x/<3:+/>=3:round)
set maxzoomP	4.0	; # maximum zoom magnification factor in plots
set minzoomP	0.5	; # minimum zoom shrinking factor in plots

# analysis window sizes (must be even numbers)
set daCwidLinux		28	; # series lists width (Linux)
set daCwidMac		28	; # Mac
set daCwidWindows	36	; # Windows

# model structure window defaults
set borderM		20		; # open space to border
set nsizeM		20		; # node size (diameter)
set vmarginM	20		; # labels margins to nodes
set vstepM		75		; # vertical absolute step
set hfactM		1.0		; # initial horizontal scaling factor
set vfactM		1.0		; # initial vertical scaling factor
set hfactMmin	0.5		; # minimum horizontal scaling factor
set vfactMmin	0.7		; # minimum vertical scaling factor
set rstepM		0.1		; # relative scaling factor step step
set rfactM		0.3		; # horizontal range exponential factor
set rinitM		750		; # horizontal initial width (4 root sons)
set rincrM		100		; # horizontal width increase step
set bsizeMwin	2		; # button size (Windows/Mac 8.6.9)
set bsizeMlin	1		; # button size (Linux/Mac 8.6.10+)
set bhstepMwin	25		; # button horizontal step (Windows/Mac 8.6.9)
set bhstepMlin	35		; # button horizontal step (Linux)
set bhstepMmac	45		; # button horizontal step (Mac 8.6.10+)
set bvstepMwin	26		; # button vertical step (Windows/Mac)
set bvstepMlin	30		; # button vertical step (Linux)
set borderMmac	10		; # additional open space to border (Mac 8.6.10+)
set ncolorM 	white	; # node color
set ncolorMsel 	blue	; # selected node color
set tcolorM 	red		; # node name color
set lcolorM 	gray	; # line color

# mouse wheel scroll factor (sensitivity)
set sfmwheel	1	; # increase to accelerate mouse wheel and decrease to slow down
set winmwscale	30	; # scroll minimum wheel movement (precision) factor in Windows

# folder/group/object symbols
set upSymbol 	"\u25B2 .."	; # up in tree structure symbol
set groupSymbol	"\u25B6 "	; # group symbol

# OS specific monospaced font name
set fontMac		"Monaco"			; # "Courier"
set fontLinux	"DejaVu Sans Mono"	; # "Courier"
set fontWindows	"Consolas"			; # "Courier"

# OS specific monospaced font size
set fontSizeMac		14
set fontSizeLinux	11
set fontSizeWindows	11

# OS specific monospaced small font size delta (negative)
set deltaSizeMac		1
set deltaSizeLinux		1
set deltaSizeWindows	2

# OS specific default window buttons widths
set butLinux	7
set butMac		5
set butMacTk869	8
set butWindows	9

# OS specific screen location offset adjustments
set corrXmac	0
set corrYmac	0
set corrXlinux	0
set corrYlinux	-55
set corrXwindows 0
set corrYwindows 0

# OS specific default model executable name
set exeLinux "lsd"
set exeMacPkg "LSD"
set exeMacOSX "lsd"
set exeWin32 "lsd"
set exeWin64 "lsd"

# OS specific default system terminal
set sysTermMac		"Terminal"	; # "Terminal", "xterm"
set sysTermLinux	"xterm"		; # "gnome-terminal", "xterm", "uxterm"
set sysTermWindows	"cmd"		;

# OS specific default debugger command
set dbgMac			"lldb"		; # "gdb"
set dbgLinux		"gdb"		;
set dbgWindows		"gdb"		;

# OS specific default browser (open=system default)
set browserMac		"open"		; # "open", "firefox", "safari"
set browserLinux	"firefox"	; # "open", "firefox", "chrome"
set browserWindows	"open"		; # "open", "firefox", "chrome"

# OS specific default wish utility
set wishMacTk85		"wish8.5"
set wishMacTk86		"wish8.6"
set wishLinux		"wish"
set wishWinTk85		"wish85.exe"
set wishWinTk86		"wish86.exe"

# OS specific make utility
set makeMac			"make"
set makeLinux		"make"
set makeWin32		"make.exe"
set makeWin64cyg	"make.exe"
set makeWin64mgw	"mingw32-make.exe"

# OS specific default gnuplot terminal (empty string=gnuplot default)
set gnuplotTermMac		""	; # "qt", "x11"
set gnuplotTermLinux	""	; # "qt", "wxt", "x11"
set gnuplotTermWindows	""	; # "wxt", "qt" , "windows"

# default gnuplot options
set gnuplotGrid3D	"60,60,3"				;
set gnuplotOptions	"set ticslevel 0.0"		;
