# ****************************************************
# ****************************************************
# LSD 7.0 - August 2015
# written by Marco Valente
# Universita' dell'Aquila
# 
# Copyright Marco Valente
# Lsd is distributed according to the GNU Public License
# 
# Comments and bug reports to marco.valente@univaq.it
# ****************************************************


# ****************************************************
# Default values for the Tk windowing system. 
#
# ****************************************************

package require Tk

# main windows size and margins (must be even numbers)
set hmargin		20	; # horizontal right margin from the screen borders
set vmargin		20	; # vertical margins from the screen borders
set bordsize	2	; # width of windows borders
set tbarsize	55	; # size in pixels of bottom taskbar (exclusion area) - Windows 7+ = 82
set hsizeL		800	; # LMM horizontal size in pixels
set vsizeL		600	; # LMM vertical size in pixels
set hsizeLmin	600	; # LMM minimum horizontal size in pixels
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

# line smoothing for series line plot
set smoothP		no	; # options are no, yes or raw
set splstepsP 	3	; # number of spline segments when smoothP is raw

# other plot windows defaults
set grayscaleP	0	; # default color mode for plots (0=color)
set gridP		0	; # defaut grid mode (0=no grid)
set fontP		{ Times 10 normal }; # default plot text font/size/mode
set linemodeP	1	; # default line mode (1:lines/2:points)
set pdigitsP	4	; #default precision digits
set hticksP		3	; # number of horizontal plot legend ticks
set vticksP		3	; # number of vertical plot legend ticks
set pointsizeP	1.0	; # default size of points (<1:./<2:x/<3:+/>=3:round)
set maxzoomP	4.0	; # maximum zoom magnification factor in plots
set minzoomP	0.5	; # minimum zoom shrinking factor in plots

# default colors in palette and other elements
set defcolors	{ black red green #d0d000 #fb46bc blue DeepSkyBlue1 grey40 PaleTurquoise2 cyan aquamarine DarkSeaGreen1 chartreuse1 OliveDrab khaki3 LightGoldenrod4 sienna1 chocolate4 firebrick3 orange1 salmon3 }
set axcolorP	gray25	; # color of plot axis
set grcolorP	gray75	; # color of plot grid

# mouse wheel scroll factor (sensitivity)
set sfmwheel	1	; # increase to accelerate mouse wheel and decrease to slow down
set winmwscale	30	; # scroll minimum wheel movement (precision) factor in Windows

# OS specific screen location offset adjustments
set corrXmac	0
set corrYmac	0
set corrXlinux	0
set corrYlinux	-55
set corrXwindows 0
set corrYwindows 0

proc defaults_loaded { } { }
