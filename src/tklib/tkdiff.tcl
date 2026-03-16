#!/bin/sh
#-*-tcl-*-
# the next line restarts using wish \
exec wish "$0" -- ${1+"$@"}
###############################################################################
#
# TkDiff -- A graphical front-end to diff for Unix and Windows.
# Copyright (C) 1994-2006 by John M. Klassa
# Copyright (C) 1998	  by Bryan Oakley
# Copyright (C) 1999-2001 by AccuRev Inc.
# Copyright (C) 2004	  by Tom Dunne
# Copyright (C) 2004-2025 by Dorothy Robinson ("dorothyr")
# Copyright (C) 2017-2025 by Michael-M ("vampm")
#
# TkDiff Home Page: http://sourceforge.net/p/tkdiff/
#
# Usage:  see "tkdiff -h" or "tkdiff --help"
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.		  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
###############################################################################

#*************************************************************
# Modified by MCP to handle GZip compressed files transparently
#*************************************************************

#### MCP
# Chgd from 8.0 to 8.6 as of V4.3 to ensure support of "Text displaylines"
#package require Tk 8.5-
# Chgd from 8.0 to 8.6 as of V4.3 to ensure support of "Text displaylines" and GZip
package require Tk 8.6
#### MCP

# set a couple o' globals that we might need sooner than later
# N.B> version must be roughly formatted as Maj.Minor[.patch][anythingelse]
#	It is parsed to PROMOTE old Preference data when required
set g(name)	   "TkDiff"
set g(version) "6.0"
set g(debug)	 0	;# Master setting - Initialized (by default) to false

#### MCP
# LSD temporary files to be deleted
set lsd 0
#### MCP

# get this out of the way -- we want to draw the whole user interface
# behind the scenes, then pop up in all of its well-laid-out glory
wm withdraw .

###############################################################################
# Primitive Debugging output (ie. useful values, states, etc.) during operation
# Semi-cryptic output, but helpful (assuming pgmr plants it in useful places)
# and can be enabled by: global var, cmdline flag or hard-coded per instance,
#	and can even be empirically DISABLED (per instance only: a -1 'force' flag)
# EVERY msg is (by default) prefixed with a "Dbg: " header but is overridable,
# and MAY be redirected into a specific output channel (default: stderr)
#
# IMPORTANT: when INVOKING this command (particularly when the 'message'
#		involves variables OR cmd processing), it is more efficeint to quote
#		that ARGUMENT with BRACES, not Dbl-quotes, because of the CONDITIONAL
#		nature of the "Dbg" execution -- those variables OR commands will not
#		be EVALUATED until "Dbg" determines it will ACTUALLY produce output!!
#	This makes a simple DEFAULTED CONTEXT (ie. NO EXPLICIT 'force' flag) MUCH
#	"lighter weight" by not evaluating args it has no intention of USING!!
#
#	Mixing this together with a force-flag of "-1" means that you NEVER really
#	have to "comment out" a Dbg instance (beyond the cost of a NULL invocation)
###############################################################################
proc Dbg {message {force 0} {hdr "Dbg: "} {where stderr}} {
	global g

	if {$force >= 0 && ($g(debug) || $force)} {
		puts $where "$hdr[uplevel 1 set na "\"$message\""]"
	}
}

# Startup phases:
#	IMPORTANT: this is a PRIMARY mechanism by which we act to prevent TK from
#		performing 'callbacks' or other actions at in-opportune times.
#		In short: level-0 begins at script readin. Level-1 should be set when
#			major data upheavals (wipes/(re)builds) are about to take place.
#			Moreover, callbacks themselves should try to separate WATCHING
#			what TK wants to do, from actually ALLOWING TK to initiate such
#			actions by only allowing any data dependent portions once level
#			2 is current.
#		This then makes it possible for the application-code to invoke AT WILL
#		when IT knows the data is ready - **even** when level 2 has not quite
#		(as yet) officially been achieved! (See 'map-resize' as a key example).
#		HOWEVER, because it IS a 'global' flag, it cant be raised until ALL it
#		expects to cover is ready. If there is a need  for finer 'granularity'
#		we can always invent more "Phases", designating which is needed by whom.
#
#	0  : Bare metal, looking for viable cmd args, obtaining 1st SCM content
#	1  : Transitioning to GUI mode and/or DURING major datastruct upheavals
#	2  : Operational mode
set g(startPhase) 0

# FIXME - move to preferences
option add "*TearOff" false 100
option add "*BorderWidth" 1 100

# Determine the windowing system
#	(since there are different ways to do this per past versions of tcl)
if {[catch {tk windowingsystem} w(wSys)]} {
	# (Much older TK versions derive windowingSystem from the platform)
	if {"$::tcl_platform(platform)" == "windows"} {
		set w(wSys) "win32"
	} elseif {"$::tcl_platform(platform)" == "unix"} {
		set w(wSys) "x11"
	} elseif {"$::tcl_platform(platform)" == "macintosh"} {
		set w(wSys) "classic"
	} else { set w(wSys) "x11"
		# N.B> this is NOT Darwin (ie. MacOS X -> Aqua) as
		# 'tk windowingsystem' WONT have 'catch'ed to ARRIVE here
	}
}

# Determine the name of the temporary directory, the rc file name,
# and NULLdev, all of which are platform dependent.
#
# Much MAY likely be overridden by a preference in .tkdiffrc,
# EXCEPT (obviously) when no such file actually exists yet
switch -- $::tcl_platform(platform) {
windows {
		if {[info exists ::env(TEMP)]} {
			# N.B> Backslashes are problematic - lets convert this
			# to a Tcl 'canonical' pathname to just avoid it all.
			# But no worries - 'tmpfile' will convert BACK before use!
			set opts(tmpdir) [join [file split $::env(TEMP)] "/"]
		} { set opts(tmpdir) C:/temp }

		# Reserved filename which is REALLY a NULL device
		set opts(NULLdev) "nul"

		set basercfile "_tkdiff.rc"
		# Native look for toolbar
		set opts(fancyButtons) 1
		set opts(relief) flat
	}
default {
		# MacOS X seemingly sets TMPDIR to something awful like
		#	 /var/folders/uC/uCFr1z6qESSEYkTuOsevX++++yw/-Tmp-/
		# BECAUSE its INHERITED from the "Finder" pgm-launcher -  NOT the USER
		#	Use the system location instead
		if {[info exists ::env(TMPDIR)] && $w(wSys) != "aqua"} {
			set opts(tmpdir) $::env(TMPDIR)
		} { set opts(tmpdir) /tmp }

		# Reserved filename which is REALLY a NULL device (Unix-like platforms)
		set opts(NULLdev) "/dev/null"

		set basercfile ".tkdiffrc"
		# Native look for toolbar
		set opts(fancyButtons) 0
		set opts(relief) raised
	}
}

# Split up and store a VPATH EnvVar (if it exists)
if {[info exists ::env(VPATH)]} {
	set finfo(Vpath) [split $::env(VPATH) ":"]
}

# Where should we start?
#	MacOSX apps want to start in ROOT (thanks Finder) which is obnoxious
if {[pwd] == "/"} {
	if {[info exists ::env(HOME)]} {
	   catch {cd $::env(HOME)}
	}
}

# Try to find a pleasing native look for each platform.
# Fonts.
set sysfont [font actual system]
Dbg "system font: $sysfont" -1

# See what the native menu font is
. configure -menu .native
menu .native
set menufont [lindex [.native configure -font] 3]
destroy .native
Dbg "menufont $menufont"   -1

# Find out what the basic tk Fg/Bg/Font defaults are
label .testlbl -text "LABEL"
set w(bgnd) [lindex [.testlbl cget -background] 0]
set w(fgnd) [lindex [.testlbl cget -foreground] 0]
set labelfont [lindex [.testlbl configure -font] 3]
destroy .testlbl
Dbg "labelfont $labelfont" -1

# and for Text
text .testtext
set textfont [lindex [.testtext configure -font] 3]
destroy .testtext
Dbg "textfont $textfont"   -1

# and for Entry
entry .testent
set w(selcolor) [lindex [.testent configure -selectbackground] 4]
set entryfont [lindex [.testent configure -font] 3]
destroy .testent
Dbg "entryfont $entryfont" -1

# This happens on Windows in tk8.5 (and apparently *nix too)!
# You get {TkDefaultFont} instead of {fixed 12} or whatever
# Then when you add "bold" to it, WHAM - you have a bad spec!
if {[set fs [lindex $textfont 1]] == ""} {
	# Decompose it, to RE-compose it:
	lassign [font actual $textfont] na fm na fs
	set textfont [list $fm $fs]
}
set font [list $textfont]
set bold [list [concat $textfont bold]]
Dbg "::font($font)\n::bold($bold)" -1

option add *Label.font	$labelfont userDefault
option add *Button.font $labelfont userDefault
option add *Menu.font	 $menufont userDefault
option add *Entry.font	$entryfont userDefault

# This makes tk_messageBox use our font.
#	The default tends to be terrible no matter what platform
option add *Dialog.msg.font $labelfont userDefault

# Initialize arrays
#######################
# globals ('g' is general pgm globals, 'w' is more widget-related)
# Defining them upfront mostly just avoids having to test for existance later.
#	Note: MORE KEYNAMES EXIST than just those shown here (eg. name, debug, ...)
#	Note: 'scmS'	is a STATIC list of ALL known SCMs (reverse alpha sorted).
#	Note: 'scmSrch' is a STATIC list of SCMs capable of searches
array set g {
	conflictset		0
	count			0
	destroy			""
	d3Left			{}
	d3Right			{}
	ignore_hevent,1 0
	ignore_hevent,2 0
	is3way			0
	lnumDigits		4
	mapborder		0
	mapheight		0
	mapwidth		0
	mapScrolling	0
	mergefile		""
	mergefileset	0
	returnValue		0
	scmDOsrch		0
	scmPrefer		""
	scmS			{Vpath SVN SCCS RCS PVCS Perforce HG GIT CVS ClearCase BK Accurev}
	scmSrch			{CVS GIT SVN}
	showmerge		0
	statusCurrent	"Standby...."
	statusMrgL		0
	statusMrgR		0
	statusInfo		""
	tempfiles		""
	thumbMinHeight	8
	thumbDeltaY		0
}

# Widgets often go active AS they are being BUILT... potentially
# INVOKING callbacks that EXPECT TO FIND critical variables
# Ensure at least those are PRESENT ahead of time
array set w {
	bLnum			0
	prefD			{alertD 0}
	TypPop1@		-1
	TypPop2@		-1
	TypPopA@		-1
}

set UniQ			0	;# Generic counter for ensuring UNIQUE object names

# reporting options
array set report {
	doSideLeft		0
	doLnumsLeft		1
	doCMrksLeft		1
	doTextLeft		"Full Text"
	doSideRight		1
	doLnumsRight	1
	doCMrksRight	1
	doTextRight		"Full Text"
	filename		"tkdiff.out"
	fnamVetted		0
	BMrptgen		{}
}

# Be advised (regarding the following global array definition):
# Produces On-demand Asynchronous file input via 'exec' machinery
#	 (EXISTENCE of the element 'trigger' WILL activate it)
array set ASYNc {
	out				""
	events			0
}

#	Only those elements that are gauranteed to exist are initialized here.
#	The remainder of the FINFO entries are dynamically added and (occasionally
#	removed) as the user interacts with the tool. There are 3 categories of
#	dynamic information:
#	   #1. entries that describe INPUT parameters:
#			f,*		  filespec describing files/dirs/URLs (w/Glob syntax?)
#			rev,*	  revision value (for a to-be-detected SCM system)
#			FSpec*	  the DE-TILDE'd & DE-Glob'd internal form of f,*
#			scm*	  SCM list (DERIVED from 'f,*'), that detected as valid
#			ulbl,*	  user-label: when provided, overrides "lbl" (see below)
#
#	   #2. entries ACTUALLY used AFTER input has been processed
#			pth,*	  the actual local (possibly temp) file to compare
#			tmp,*	  optional flag denotes "pth" AS a tempfile (& other uses)
#			lbl,*	  displayable label for "pth"
#			pproc,*	  special post processing needed for "pth" (rare)
#
#	   #3. entries VERY similar to #2, but pertaining to ANCESTOR files
#			apth,*	  the actual local (possibly temp) file to compare
#			atmp,*	  optional flag denotes "apth" AS a tempfile (& other uses)
#			albl,*	  displayable label for "apth"
#
#		In each above case, '*' is a monotonic number beginning at 1. Zero
#		is a special case used exclusively for a #1 "Ancestor File" entry.
#		The SAME value, WITHIN its category, describes attributes of a SINGLE
#		object --- However "ulbl" is an exception - its number is USED by
#		category 2, despite being SET by category 1 (reasons are mostly
#		historical, dating from a time when the only values WERE 1 & 2);
#		"ulbl" is NOT expected to see usage beyond that still valid case,
#		although it is NOT specifically prohibitted.
#
#	Items in category #1 represent data ENTERRED by the user; as such they
#	are tied somewhat to the GUI (thereby initialized here), and are
#	(mostly) fixed at being at MOST three each (except MAYBE ulbl). Note that
#	scm,* (as an entry) is mostly for the inquiry/search modes as individually
#	retrieved files will generate their OWN (NOT modifying this global value).
#	Additionally, FSpec* is derived FROM its f,* value but acts as a BARRIER
#	between the syntax a user may ENTER (Tilde/Glob notations), and the
#	equivalent EXPANDED value (FSpec*) used internally (needed to span the
#	semantics of Tcl8.x versus Tcl9.x)
#		SADLY, these needs (internal/external & an unfortunate encoding DEFAULT)
#	only became necessary RECENTLY, (Tcl 9.0) thus one REALITY is that FSpec*
#	values are literally only USED at 'points of entry' and are SUBSEQUENTLY
#	*dumped back* into their external counterparts (f,*) once vetted to exist,
#	and operations can begin!
#
#	>>>>	EDITTING to just USE (FSpec*) EVERYWHERE was deemed TOO costly!
#
#	While Tcl/Tk 9.0 made data ENCODING a much stricter issue, compensation was
#	possible by PRESERVING the default actions of V8.x.x,
#
#		VIA a V9 "-profile tcl8" file config operation to OVERRIDE the defaulted
#			value of 'strict" (and SHOULD have been an 'open' option, but isn't)!
#
#	Items in category #2 (NOT set here) are grouped as adjacently numbered
#	PAIRS, and are files intended, actually, or previously been compared,
#	DERIVED from the items of category #1.
#
#	Items in category #3 (NOT set here) use a DISJOINT monotonic numbering
#	system from 1 to "fPairs" (explained next) AND a 'a'-prefix naming
#
#	Beyond the 'category' entries are:
#	"fCurpair" designates which monotonic PAIR is actively in use (1->fPairs)
#	with "fPairs" itself being the COUNT of how many "fCurpair"s exist.
#
#	"fLfmt" notes the CURRENT format of presenting a multiFile List where
#	(0=menu, 1=dialog); It is used in conjunction with opts(fLMmax) to
#	determine WHEN that format needs to be SWITCHed to one form or the other
#	Intention is to avoid 'unweildy' menus with too many entries.
#
#	Thus "f,1" DIRECTLY implying "pth,1" is true ONLY if "f,1" designates a
#	single file. Likewise for "f,2" -> "pth,2". Input fields designating
#	directories and/or SCM branches (or commits) can generate SEVERAL "pth,N"
#	(and other category 2) entries, each.
#
#	The "lbl,Left" and "lbl,Right" and "title" entries are simply the DISPLAYED
#	label values (set from whatever the ACTIVE pair of "lbl,*" entries are),
#	and are tied directly to the GUI, (providing a cheap update mechanism), and
#	"fRecurs" authorizes recursive descent if the given Fspec(s) is/are Dirs.
#
#	Finally, "Vpath" only EXISTS (as a list of Vnodes) if the EnvVar VPATH did,
#	with "Vpofst" noting how many nodes(0->N) were "TOP pruned" due to the CWD
#
#	IMPORTANT:
#	Certain COMBINATIONS of category 2/3 entries (existance, emptiness) are used
#	to describe various situations (i.e. tmp files, real files, pairs needing
#	comparison but NOT yet fully extracted from a necessary SCM repository;
#	or files NOT editable because they were extracted by an SCM; and even when
#	a 3way diff is to be considered active: ie. EXISTANCE of 'albl,N' element).
#	  EXERCISE CARE Re: ADDING/RENAMING of NEW elements...
#		  Category 2/3 values are essentially considered TRANSIENT and MAY BE
#		  DELETED or reset at times using patterns such as '[aptl]*[0-9]'.
#
#	Be VERY CAUTIOUS when considering CHANGING ANY of these manipulations!
array set finfo {
	title		 {}
	fLfmt		 0
	fPairs		 0
	fCurpair	 1
	fRecurs		 0
	lbl,Left	 "label_of_file_1"
	lbl,Right	 "label_of_file_2"

	f,0			 ""
	rev,0		 ""
	FSpecA		 ""

	f,1			 ""
	ulbl,1		 ""
	rev,1		 ""
	FSpec1		 ""
	scm1		 ""

	f,2			 ""
	ulbl,2		 ""
	rev,2		 ""
	FSpec2		 ""
	scm2		 ""
#### MCP
	lsd,1		 ""
	unzip,1		 ""
	lsd,2		 ""
	unzip,2		 ""
#### MCP
}

# CRITICAL:
#	Color makes a HUGE difference to Tkdiff - lack of it, well, is gonna be
#	REALLY BAD with MANY features virtually UN-USABLE ... BUT it COULD "run".
#
# Historical notes: ##################
# From V4.2 thru V5.0 a truely DISMAL ability to run MONOCHROME was maintained.
#	AS OF V5.1 (circa 2020) we decided that you likely cant BUY a B+W monitor
# anymore, and even if you did, its HIGHLY unlikely you'd be using THIS tool.
##################
#	While we have left inplace the STRUCTURE to PROVIDE such support, AND the
# critical CONTENT for re-engineering the changes to RESTORE that capability,
#
#	POOF - Its now GONE!!		<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
#
# Notably, the "else" to this test for color support WOULD NEED to contain lots
# of local vars, suitably sprinkled throughout the 'driving list' for SETTING
# the defaults. IN ADDITION, to the extent that they would CONFLICT with how
# the COLOR settings are described, those SAME LOCAL VARS would require setting
# within the "if-color" branch (not unlike how the colors themselves are done).
# After, those locals, having replaced the hardcoding, SHOULD be UNSET as well
# ####################################
#
#	BECAUSE of all that, TkDiff **WILL NOW ABORT** if color is unavailable !!
#
# Yet THAT eventuality necessitates some juggling of "procs" to ensure they
# EXIST before someone calls them. SO we need them HERE, NOW, because the
# setting of APPLICATION DEFAULTS takes place at script "READ-IN time"!
#
# Furthermore, because these will ALL now reside ABOVE the demarcation of our
# builtin pgm-flow tracing "region" they will ALWAYS operate silently untracked
# REGARDLESS of WHEN they may be invoked (be that now to ABORT, or later on).
#	Needed is: 'fatal-error' and 'do-exit'
#		Conveniently, BEYOND these two, any FURTHER calls will NOT EMMANATE
#		from either of them because the needed conditions WILL NOT be met, OR
#		they are protected by 'catch'-stmts for their OWN internal reasons !!
# #############################################################################

###############################################################################
###############################################################################
##						Severe Error/Exit handling mechanisms
###############################################################################
###############################################################################
# Throw up a modal error dialog or print a message to stderr.
# In general we print to stderr and exit if the main window hasn't yet been
# created, otherwise put up a dialog and throw an exception.
###############################################################################
proc fatal-error {msg} {
	global g

	if {$g(startPhase)} {
		popmsg $msg "Aborting..."
	} else {
		puts stderr "Error: $msg\n$g(name) Aborted"
	}
	do-exit 2
}

###############################################################################
# Exit with proper code
###############################################################################
proc do-exit {{returncode {}}} {
#### MCP
#	global g w ASYNc
	global g w ASYNc finfo
#### MCP

	# During pgm startup, we MAY have built the status window just to let the
	# user know we are talking to a SCM server that MIGHT experience network
	# latency - so if that window exists (but OTHER windows do not) and we are
	# here, something died and we want to RELEASE that window before we leave.
	if {[info exists w(status)] && ![info exists w(client)]} {
		Dbg "something died (or was killed) ... trying to shutdown"
		catch {wm forget $w(status)}
		# Release any extra event loop (if it is running) so we CAN leave
		set ASYNc(events) 0
		unset -nocomplain ASYNc(trigger) ;# not much point, but it IS correct
	}

#### MCP
	# delete LSD temporary files
	foreach mod { 1 2 } {
		catch { file delete $finfo(lsd,$mod) }
	}
#### MCP
	# we don't particularly care if del-tmp fails.
	catch {del-tmp}

	# Value from latest external execution
	if {$returncode == {}} { set returncode $g(returnValue) }

	# exit with an appropriate return value
	exit $returncode
}

###############################################################################
# OK - FINALLY we can establish the DEFAULTS for running TkDiff	  (@ READ-IN !)
###############################################################################
if {[string first "color" [winfo visual .]] >= 0} {
	# We have color - HURRAY!! (but, let's not go crazy...)
	#
	#	This mass assignment is a NOD to the days of monochrome support
	# It remains more as an example of how to approach reinstating such, and is
	# NOT absolutely needed as the colors could simply be PLANTED where they go
	lassign "Tomato	  PaleGreen	  DodgerBlue   yellow	magenta \
			 Goldenrod1	  Khaki	  gray	 LightSteelBlue	  blue" \
			 Pdel Pins Pchm Polp Padj Pinf Pcur Pdif Pcht Pbyt

# (closebrace) else (openbrace)		<-- WOW Tcl is really pissy about braces!!
#	# Only black and white?? YUCK (It's gonna look/work AWFUL, sorry).
#	lassign {Black White} bLk wHt	;# <-- just shortening the color names
#	lassign "$bLk $bLk $bLk $bLk $bLk $wHt $bLk $wHt $bLk $bLk" \
#			 Pdel Pins Pchm Polp Padj Pinf Pcur Pdif Pcht Pbyt
#
#	These were the specifics of the MONOCHROME support settings:
#
#	textopt	   "-background white -foreground black -font $font"
#	currtag	   "-background $Pcur -foreground white"
#	difftag	   "-background $Pdif -foreground black -font $bold"
#	deltag	   "-background $Pdel -foreground white"
#	instag	   "-background $Pins -foreground white"
#	chgtag	   "-background $Pcht -foreground white"
#	overlaptag "-background $Polp -foreground white"
#	bytetag	   "-underline 1"
#	inlinetag  "-underline 1"
#	mapins	   "$Pins"
#	mapdel	   "$Pdel"
#	mapchg	   "$Pchm"
#	adjcdr	   "$Padj"
#	inform	   "$Pinf"
#
#	The PROBLEM is that 'font'ing, underlining, etc were used to compensate
#	for the LACK of colors in certain instances posing complex substitution
#	situations (different option counts) together, PLUS more INVERSE VIDEO use
#
} else { fatal-error "$g(name) no longer supports Monochrome operation" }

# Establish the DEFAULT option values for numerous application-wide items ...
#	(most are generally customized at runtime and become USER preferences)
#
# Each item is designated by an internal KEY and ALWAYS has a VALUE in 'opts()'
# AND (if a DESCRIPTION was provided) thats kept (via the SAME key) in 'pref()'
# Thus 'opts()' ALWAYS exists, but only MOST are considered as User PREFERENCES
#	(N.B> During ongoing development simply ADD to the table CONTENT as needed,
#	(	  -> do NOT ALTER keys unless remapping OLD ones during UserPref READS)
#	(	> SOME 'opts' have ALREADY been Pre-defaulted: just PRESERVE them here)
# *** IMPORTANT ***
#	Keys matching "[mng][rae][gvn]*" are CARGO Capable (ie $w(wSys) dependant)!
#	Binding to a DIGIT requires the "Key-" descriptor: as <1> would be a BUTTON

#### MCP
#	autoselect		  0		   {Auto-select nearest diff region when scrolling}
#	colorcbs		  0				  {Color change bars to match the diff map}
#	egnBlanks		  0								  {Suppress all Whitespace}
#	filetypes	  { {"Text Files" {*.txt *.tcl}} {"All Files" {*}} }
#	geometry	  "80x30"									 {Text window size}
#	ignoreEmptyLn	  0							{Suppress diffs of empty lines}
#	showlineview	  0					  {Show current line comparison window}
#	showln			  1										{Show line numbers}
#	tabstops		  8												{Tab stops}
#	tagcbs			  0									{Highlight change bars}
#### MCP
foreach {key val desc} [subst {
	adjcdr		  "$Padj"				   {CDR region color during adjustment}
	autocenter		  1				{Automatically center current diff region }
	autoselect		  1		   {Auto-select nearest diff region when scrolling}
	autoSrch		  0					{Auto-search detected SCM when capable}
	bytetag		  "-background $Pbyt -foreground white -underline 1"
									  {Tag options for characters in line view}
	chgtag		  "-background $Pcht"	  {Tag options for changed diff region}
	colorcbs		  1				  {Color change bars to match the diff map}
	currtag		  "-background $Pcur" {Tag options for the current diff region}
	customCode		  {}													 {}
	deltag		  "-background $Pdel -font $bold"
										  {Tag options for deleted diff region}
	diffcmd		  "diff"										 {Diff command}
	difftag		  "-background $Pdif"			 {Tag options for diff regions}
	editor		  ""								{Program for editing files}
	egnCmd		  "diff"											 {Diff cmd}
	egnSrchCmd	  "diff -r"							  {Diff recursive-srch cmd}
	egnCase			  0								{Suppress Case distinction}
	egnBlanks		  1								  {Suppress all Whitespace}
	egn#Blanks		  0							  {Suppress varying Whitespace}
	egn@TabX		  0							{Suppress Tab-based Whitespace}
	egn@EOL			  0								  {Suppress EOL Whitespace}
	egnTabSiz		  0									 {Specify the Tab size}
	egnSCase		  0						   {Suppress Case distinction-srch}
	egnSBlanks		  0							 {Suppress All Whitespace-srch}
	egnS#Blanks		  0						 {Suppress varying Whitespace-srch}
	egnS@TabX		  0					   {Suppress Tab-based Whitespace-srch}
	egnS@EOL		  0							 {Suppress EOL Whitespace-srch}
	egnSTabSiz		  0								{Specify the Tab size-srch}
	egnXcludFil		  0						   {Specify excluded file patterns}
	eopCase		  "-i"							{Suppress Case distinction arg}
	eopBlanks	  "-w"							  {Suppress All Whitespace arg}
	eop#Blanks	  "-b"						  {Suppress varying Whitespace arg}
	eop@TabX	  "-E"						{Suppress Tab-based Whitespace arg}
	eop@EOL		  "-Z"							  {Suppress EOL Whitespace arg}
	eopTabSiz	  " --tabsize"						 {Specify the Tab size arg}
	eopSCase	  "-i"					   {Suppress Case distinction srch-arg}
	eopSBlanks	  "-w"						 {Suppress all Whitespace srch-arg}
	eopS#Blanks	  "-b"					 {Suppress varying Whitespace srch-arg}
	eopS@TabX	  "-E"				   {Suppress Tab-based Whitespace srch-arg}
	eopS@EOL	  "-Z"						 {Suppress EOL Whitespace srch-arg}
	eopSTabSiz	  " --tabsize"					{Specify the Tab size srch-arg}
	eopXcludFil	  " -x"							 {File patterns to exclude arg}
	fancyButtons  $opts(fancyButtons)			{Windows-style toolbar buttons}
	filetypes	  { {"Text Files" {*.txt *.tcl}} {{LSD Equation Files} .cpp} {{LSD Configuration Files} .lsd} {"All Files" {*}} }
									 {Choice of file suffixes for file dialogs}
	fLMmax			  9							  {FileList Menu max-threshold}
	geometry	  "60x30"									 {Text window size}
	genEdit		  "<e>"					 {Invoke an editor on the Current file}
	genFind		  "<Control-f>"			{Request textual search in either file}
	genNxfile	  "<Control-bracketleft>"		{Switch to next available file}
	genPvfile	  "<Control-bracketright>"		{Switch to prev available file}
	genRecalc	  "<Control-r>"			 {Request to re-diff current file pair}
	genXit		  "<Control-q>"					  {Request Immediate tool exit}
	ignoreEmptyLn	  1							{Suppress diffs of empty lines}
	ignoreRegexLnopt  {}						 {RegExp(s) for matching lines}
	ignoreRegexLn	  0				   {Suppress diffs of RegExp-matched lines}
	ignSuprs		  0				  {Utilize Engine suppresions when Diffing}
	inform		  "$Pinf"							  {Informational highlight}
	inlinetag	  "-background $Pchm -font $bold"
							   {Tag options for diff region inline differences}
	inlSuprs	  0							{Designated suppression attributes}
	instag		  "-background $Pins -font $bold"
										 {Tag options for inserted diff region}
	mapchg		  "$Pchm"								{Map color for changes}
	mapdel		  "$Pdel"							  {Map color for deletions}
	mapins		  "$Pins"							  {Map color for additions}
	mapolp		  "$Polp"							 {Map color for collisions}
	mrgLeft		  "<Key-1>"						  {Mark CDR for Leftside Merge}
	mrgLtoR		  "<Key-3>"				   {Mark CDR for Left-then-Right Merge}
	mrgRght		  "<Key-2>"						 {Mark CDR for Rightside Merge}
	mrgRtoL		  "<Key-4>"				   {Mark CDR for Right-then-Left Merge}
	navCntr		  "<c>"						 {Center CDR within Display window}
	navFrst		  "<f>"							{Move CDR to First Diff region}
	navLast		  "<l>"							 {Move CDR to Last Diff region}
	navNext		  "<n>"							 {Move CDR to Next Diff region}
	navPrev		  "<p>"						 {Move CDR to Previous Diff region}
	overlaptag	  "-background $Polp"	  {Tag options for overlap diff region}
	predomMrg		  1								 {Predominant merge choice}
	scmPrefer	   {Auto Auto}				   {Prefer given SCM when detected}
	showcbs			  1										 {Show change bars}
	showinline1		  0					  {Show inline diffs (per-byte method)}
	showinline2		  1					 {Show inline diffs (recursive method)}
	showlineview	  1					  {Show current line comparison window}
	showln			  0										{Show line numbers}
	showmap			  1							  {Show graphical map of diffs}
	syncscroll		  1								   {Synchronize scrollbars}
	tabstops		  4												{Tab stops}
	tagcbs			  1									{Highlight change bars}
	tagln			  0								   {Highlight line numbers}
	tagtext			  1								  {Highlight file contents}
	textopt		  "-background white -foreground black -font $font"
														  {Text widget options}
	tmpdir		  $opts(tmpdir)					  {Directory for scratch files}
	toolbarIcons	  1			   {Use icons instead of labels in the toolbar}
	xcludeFils		  {}					  {Files to exclude when searching}
}] { set opts($key) $val ; if {$desc != {}} {set pref($key) $desc} }
# So much for defaulting ... (as mentioned earlier) "BRIEFLY" is now over!
# Whack the little (global) vars we used to SEED and PROCESS the option values
unset Pdel Pins Pchm Polp Padj Pinf Pcur Pdif Pcht Pbyt key desc val

# Further ensure wrapping is turned off. This might piss off a few people,
# but it would screw up the display ROYALLY to have things wrap
append opts(textopt) " -wrap none"

# Work-around for bad font approximations,
# as suggested by Don Libes (libes@nist.gov).
#### MCP
#catch {tk scaling [expr {100.0 / 72}]}
set refDPI 96
set effDPI [ expr { 72 * [ tk scaling ] } ]
set screenScaling [ expr { $effDPI / $refDPI } ]
catch { tk scaling [ expr { $screenScaling * 100.0 / 72 } ] }
#### MCP


###############################################################################
# <<<< NOTICE >>>>
#
# The following PROCS are STILL ABOVE the "Do Not Track Me" script region !!!
#			These are primarily UTILITY Procs and unworthy of being traced...
#				You will KNOW if one of these fails because they will show up
#			in a Stack Trace on the way to a REAL error, or simply be MISSING
#			because pgm flow never let them be called (and so would not have
#			had the chance to announce themselves anyway...
###############################################################################

###############################################################################
# Our Dialog 'factory' supporting: creation/display/invocation/release
#	Specific actions/args are per each subcmd & existence of 'windowName' (wNM)
############ subcmds ##############
# NONMODAL wNM ?toplevel-args?
# MODAL	   wNM ?toplevel-args?
#				Create/restores Dialog window toplevel; RCode indicates which.
#				1= wNM existed and will be reused; 0= wNM was JUST NOW created:
#				  caller then DEFINEs content (if reqd), followed by any needed
#				  cfg/re-cfg, ultimately terminated with a PROPER 'show' subcmd
#				NONMODAL .vs. MODAL are mutually exclusive subcmds establishing
#				the mode (and expected usage) EACH performing the same tasks.
#			N.B> CHOICE of mode affects HOW to construct AND show the window!!
# dismiss wNM ?savepos?
#				Remove dialog from display. Optionally (savepos==true) retain
#				last known size/position of window to reinstate on next use
#				Default savepos=0. NOTE: 'savepos' is generally only needed
#				if the dialog was intended to be DESTROYED and then rebuilt.
#				A simple 'dismiss' WILL tend to RE-display at its PRIOR loc.
# # # # #  # #
#	The following subcmd causes the dialog to be displayed and is designed to
# be used after POSSIBLE construction and/or configuration.
# N.B>	There are implications to the CONSTRUCTION of a MODAL dialog regarding
#		the GLOBAL variable 'ctrlvar' whose NAME is specifed on the 'show' *IF*
#		it was declared as MODAL.
#			Primary is *it* becomes the target of any "actions" within the
#		dialog that expect to terminate it (successfully or not). The 'val',
#		given ON the 'invoke' should be whatever value you would want in a
#		default sense (Cancel, whatever), in the event the dialog is perhaps
#		closed via the window manager.
#			For a NON-MODAL dialog, ITS "actions" may be whatever is deemed
#		proper, including the 'dismiss' subcmd, or even a "destroy".
#
# show wNM [ctrlvar val] ?focusTarget?
#				Begins dialog processing and WAITs to be dismissed. Note that
#				if the window was defined as MODAL, "ctrlvar val" are REQUIRED.
#				When 'focusTarget' is provided, it should be the element within
#				'wNM' where focus should be initially directed. RCode IS the
#				value of 'ctrlvar' (which itself must be GLOBALLY accessible)
###############################################################################
proc Dialog {cmd wNM args} {
	global	w

	switch -- $cmd {
		NONMODAL -
		MODAL {
			lassign {1 0}  M(MODAL) M(NONMODAL)
			set w(mode$wNM) [expr {$M($cmd) * 2}] ;# To do syntax check @'show'
			if {![winfo exists $wNM]} {
				# N.B> The overall tool class DERIVES from its scriptname
				# (w/a Capitalized 1st letter): MAINTAIN that for any FURTHER
				# windows created else the Wmgr MAY decorate its taskbar wrong
				wm withdraw [toplevel $wNM -class Tkdiff {*}$args]
				if {$w(wSys)=="aqua"} { setAquaDialogStyle $wNM $M($cmd) }
				if {[info exists w(dlgeo,$wNM)]} {
					# We were TOLD (at a prior 'dismiss') to REposition this
					# window at its last known location - make that happen
					scan $w(dlgeo,$wNM) "=%*dx%*d+%d+%d" X Y
					wm geometry $wMN "=+$X+$Y"
					# But because we dont KNOW that such remembering will
					# happen again THIS time, FORGET it, while allowing the
					# dialog to acquire a NEW position AFTER being displayed
					# at this DESIGNATED location (USER is free to move it)
					unset w(dlgeo,$wNM)
					after idle wm geometry $wNM {}
				}
				#  Tell caller window NOW exists, but is UNPOPULATED
				return 0

			# else Tell caller window already exists and READY to be displayed
			} { return 1 }
		}

		show { if {[llength $args] < $w(mode$wNM)} {
									  error "Dialog $wNM: $cmd: missing args" }
			# Put it onscreen
			#	'nowait' is to PREVENT us from WAITING for a MODAL dialog
			#	that is ALREADY visible (not a USUAL situation, but...)
			if {[winfo ismapped $wNM]} {set nowait 1} {wm deiconify $wNM}
			raise $wNM

			if {$w(mode$wNM)} {
				lassign $args ctrlvar val focalpt
				upvar #0 $ctrlvar var

				# Poke ctrlvar if user nukes the window (release tkwait & GRAB)
				set var $val  ;# FORCE named ctrlvar to EXIST with a VALUE
				bind $wNM <Destroy> [list set $ctrlvar $var]

				# Direct focus for the dialog
				if {![string length $focalpt]} { set focalpt $wNM }
				set focalPrev [focus -displayof $wNM]
				focus $focalpt
				if {![info exists nowait]} {catch {tkwait visibility $wNM}}
				catch {grab $wNM}
				#######################################################
				# Begin dialog operation; waiting here until completion
				#######################################################
				tkwait variable $ctrlvar
				catch {focus $focalPrev}
				catch {grab release $wNM}
				return $var
			} { lassign $args focalpt
				if {![string length $focalpt]} { set focalpt $wNM }
				after idle focus $focalpt
			}
		}

		dismiss { lassign [concat $args 0]	retain ;#  Save position?
			# Use CATCH to not CRASH: window MAY have been deleted
			catch { if {$retain} { set w(dlgeo,$wNM) [wm geometry $wNM] }
			catch { wm withdraw $wNM }
			}
		}
	}
}

###############################################################################
# A simple (reusable) 'entrybox' dialog.
###############################################################################
proc Prompt { msg {preload {}} {title {Please provide}}} {
	global w
	set f .prompt
	if {![Dialog MODAL $f -bd 10]} {
		# Window was JUST created (withdrawn) and needs content
		wm title $f $title
		wm transient $f .
		wm	 group	 $f .
		# Don't destroy the window, just hide from view
		wm protocol $f WM_DELETE_WINDOW "Dialog dismiss $f"

		message $f.msg -text $msg -aspect 1000
		entry $f.entry -textvariable w(val$f)
		pack $f.msg $f.entry [set b [frame $f.buttons]] -fill x
		pack $f.entry -pady 5
		button $b.ok -text OK -default active -command "set w(ok$f) 1"
		button $b.cancel -text Cancel		  -command "set w(ok$f) 0"
		pack $b.ok -side left
		pack $b.cancel -side right
		bind $f.entry <Return>		 "$b.ok		invoke ; break"
		bind $f.entry <Escape>		 "$b.cancel invoke ; break"
	} else {
		$f.msg config -text $msg ;# Simply change the prompt msg on reuse
	}
	# Initialize, run, and dismiss
	if {[set w(val$f) $preload] != {}} { $f.entry selection range 0 end }
	$f.entry icursor end
	Dialog show $f w(ok$f) 0 $f.entry
	Dialog dismiss $f

	# Provide STATUS result
	#	N.B> Caller is responsible for retrieving the actual TEXT response
	#		(from the WELL-KNOWN location "$w(val.prompt)") AFTER checking
	#		the returned boolean STATUS of the Dialog (ZERO = user CANCELLED)
	return $w(ok$f)
}

###############################################################################
###############################################################################
##				  BUILTIN Specialized Debugging facilities
###############################################################################
###############################################################################

###############################################################################
# Internal variation (of 'Dbg' - see defn @top of file) specifically geared to
#	the planting of 'trace' stmts which expect to 'append args' to a cmd prefix
#			STRICTLY FOR DEVELOPMENT/INVESTIGATIONAL USAGE
#	example -
#		trace  add	 ?what&name? ?ops? WatcH
#			..... (area of code to watch - particularly for what=variable)
#		trace remove ?what&name? ?ops? WatcH
#
#	BUGS: only implemented for 'variable' traces just now
###############################################################################
proc WatcH {args} {
	if {[set op [lindex $args end]] in "read write unset"} {
		# It was a variable trace, so show its value (unless it was unset)
		if {$op == "unset"} {set value "-na-"} {upvar [lindex $args 0] value}
		Dbg {$args\t\t$value} 1 "WatcH:" stdout
	}
}

###############################################################################
# Modal msg dialog: defaults to error classification/decoration in front of "."
#	Args (except 1st) are optional and are identified by CONTENT, NOT position
#	N.B> Not ALL (-type)s are presently recognized
###############################################################################
proc popmsg {msg args} {
	global g

	# derive args (after establishing defaults)
	lassign {error ok Error} severe type title parent
	foreach item $args {
		if {[string index $item 0] == "."
		&&	[winfo exists $item]}			   { set parent	 "-parent $item"
		} elseif {$item in {error warning info question}} {set severe $item
		} elseif {$item in {ok okcancel yesno yesnocancel}} {set type $item
		} else											  { set title $item }
	}

	# Notify and wait for acknowledgement (default display is in front of ".")
	return [tk_messageBox -message "$msg" -title "$g(name): $title" \
					-type $type -icon $severe {*}$parent]
}

###############################################################################
# INTERNAL stacktrace generator (helps pin down WHERE something got executed)
#	Defaults to tracing stack-levels, but can be asked to do stack-frames
###############################################################################
proc trap-trace {{title "Trace"} {framORlevl "level"}} {
	set str ""
	for {set x [expr [info $framORlevl]-1]} {$x > 0} {incr x -1} {
		append str "$x: [info $framORlevl $x]\n"
	}
	popmsg $str info "$title" ;# pause until developer acknowledges
}

# PREPARE to OverWrite all those defaults (reading the USERS Preferences file)
#	Errors will ultimately be reported. But before doing so, we need to CREATE:
proc define {name value} {
	global w opts
	# Any key coming thru that CONTAINS (read as: PREFIXED) with the value of
	# the CURRENT windowing system must be stripped back to its REAL preference
	# name (and stored as such). This allows anything else (such as some OTHER
	# PLATFORMs setting - aka bindings) to just simply be retained (as CARGO)
	#	N.B> Requires NON-'prefix' USE of cargo TRIGGER strings to be Verboten!
	set opts([string map "$w(wSys) {}" $name]) $value
}
#	This lets the rc file have a slightly more human-friendly interface,
# AND hides our 'cargo' mechanism for w(wSys) DEPENDENT values!
#	Old-style .rc files should still load just fine for now, though it ought to
#	be noted NEW .rc files won't be able to be processed by OLDER TkDiff vrsns
#	BUT - that SHOULDN'T be a problem (who moves backward?)

# Compute Preferences file LOCATION in preparation to attempted READing
#	N.B> TKDIFFRC can hold EITHER a dir or file NAME,
#		with the PRESUMPTION its a FILE if whats NAMED doesnt CURRENTLY exist
#		Same is true of the $HOME location, but NOT for the ROOT location
if {([info exists ::env(TKDIFFRC)] && [set g(rcfile) $::env(TKDIFFRC)]!={})
||	([info exists ::env(HOME)] &&
	 [set g(rcfile) [file join $::env(HOME) $basercfile]] != {})} {
	if {[file isdirectory $g(rcfile)]} {
		set g(rcdir) $g(rcfile)
		# Probe if user supplied a "-P preFname" option on the cmdline
		if {[set g(rcfile) [lindex [regexp -inline -all -- \
									 {(?:\A|\s+)-P\s*(\S+)} $argv] 1]] != {}} {
			set g(rcfile) [file join $g(rcdir) $g(rcfile)]
		} { set g(rcfile) [file join $g(rcdir) $basercfile] }
	}

	# This is terribly anti-social and LIKELY aint 'save'-able ANYWAY!
	# But it ALLOWS us to "soldier on" using JUST the builtin defaults.
	# and anything the user cares to modify them to in their session.
} { set g(rcfile) [file join "/" $basercfile] }

# AND clean up the global namespace (which wasnt checked before trampling)!!
unset -nocomplain basercfile

# GO READ the Pref file (if its there)
if {[file exists $g(rcfile)]} {
	if {[catch {source $g(rcfile)} error]} {
		set startupError [join [list "There was an error in processing your\
		  startup file." "\n$g(name) will still run, but some of your\
		  preferences" "\nmay not be in effect." "\n\nFile: $g(rcfile)" \
		  "\nError: $error"] " "]
	}

###############################################################################
# Preference Morphing
#	(written as if this were a CALLED proc, but STILL all happening @ READ-in)
#
#	RARELY -  an EXISTING preference becomes inconsistent with the evolution
#			of the code. As we cant know in advance what vintage file was just
#	read-in, it may have just installed a SEMANTICALLY OLDER Pref. Its useful
#	to map/adjust (or at least, remove) such older Prefs (when reasonable).
#		Each of these "Morphs" chains forward to the next to allow ANY age of
#	file to be processed UP the evolutionary path: an in-exact science at best!
# N.B>	While ADDing NEW Prefs is never an issue, installing Prefs NEWER than
#		the RUNNING code version is PROBLEMATIC - yet is NOTED; ...NOT aborted!
###############################################################################

# 1st hack (predates files even HAVING the version stamp that WROTE them)
# V3.0 was the first codebase NEEDING this for (~ V2.xx+) files...
#		If user has a 'diffopt' Pref defined (from their rc file),
#		magically convert/merge that to become 'diffcmd'
	if {[info exists opts(diffopt)]} {
		set opts(diffcmd) "diff"	;# The V3.0 original DEFAULT value
		lappend opts(diffcmd) {*}$opts(diffopt)
		unset opts(diffopt)
	}

# 2nd hack	(files become version stamped)
# V4.3.1 Began recording the Codebase version that WROTE the Prefs, but when
# READ back IN, was placed into a bare GLOBAL Var "prefsFileVersion".
#		Luckily, no SEMANTICS ever CHANGED in the codebase since that time,
#		thus nothing needs adjusting beyond the stamp LOCATION itself
#			(maintaining the evolutionary chain)
	if {[info exists "prefsFileVersion"]} {
		set opts(prefsVrsn)	  $prefsFileVersion
		unset prefsFileVersion
	}

# 3rd hack	(PERMANENT location of the 'File Version')
# V5.1 Decided it made more sense to keep the "FV" (fileVrsn) WITH the datums
# and thus base future MORPHs on the versions themselves.
#		Morphing is an UPWARD process, all bets are off if the FV is NEWER!
	if {[info exists opts(prefsVrsn)]
	&&	[regexp {[0-9]+(?:\.[0-9]+)*(?:[ab][0-9]+)?} $opts(prefsVrsn) FV]
	&&	[regexp {[0-9]+(?:\.[0-9]+)*(?:[ab][0-9]+)?}   $g(version)	  CV]
	&&	[package vcompare $FV $CV] <= 0} {

# V5.5 Re-oriented how Diff (as an Engine) is CONFIGd for use in TkDiff
		if {![package vsatisfies $FV "5.5-"]} {
			# 1. No longer presumes "diffcmd" IS (GNU) Diff exclusively. Merges
			# SRCH args into the NEW Srch-cmd (which simply DEFAULTs to "diff")
			# 2. Similarly, suppression categories are now explicit (DO NOT MAP
			# the old 'blackbox' flag set -is not worth it- Just whack'em); the
			# resultant All-Suppressions-OFF default makes USER then RE-config.
			# 3. "ignoreblanks" is now "ignSuprs" (same semantics - new NAME),
			#	 "ignoreRegexLn" and "ignoreEmptyLn" became bit-Based booleans.
			# 4. "diffcmd" is now DERIVED (as opposed to user-SET) thus will be
			# rewritten internally (later), requiring no further action.
			if {[info exists opts(fRecurs)]} {
				lappend opts(egnSrchCmd) {*}$opts(fRecurs)
			}
			if {$opts(ignoreEmptyLn)} { set opts(ignoreEmptyLn) 8 }
			if {$opts(ignoreRegexLn)} { set opts(ignoreRegexLn) 4 }
			if {$opts(ignoreblanks)}  { set opts(ignSuprs)		2 }
			unset -nocomplain  \
				opts(fRecurs) opts(ignoreblanksopt) opts(ignoreblanks)
		}
################################################################################
# FUTURE Morphs should INSERT above HERE as OPEN-ENDED If-'package vsatisfies'
#	code-blocks in INCREASING Version order (when needed)
################################################################################
	} else { popmsg "Your Preference file (V$FV) appears to be
					NEWER
	than the $g(name) V$CV currently running.
		 This MAY NOT operate properly..." warning "Near CRITICAL warning"
	}
}

###############################################################################
#	SCRIPT readin-time TRICK for EXTENDED DYNAMIC debug tracking on DEMAND
#
# Pre-scan command line args to detect/collect ALL debug (-d*) specifications
#	(because we EMBED tracking info INTO designated procs - NOT WRAP THEM!)
# If ANY specs exist - we WRAP the 'proc' LANGUAGE STMT instead to act as a
# SELECTOR of which yet-to-be-read procs to augment with tracking, IN ADDITION
# to its normal task of actually DEFINING every such proc seen - from HERE ON!
#
#N.B> ANY proc NOT TO BE pre-processed should occur BEFORE reaching this code!!
# CRITICAL:
#	"scanning" of EVERY PROC read *DOES AFFECT* autoloaded ones as well:  Thus,
#	'wrapping' here *WILL BE REVERTED* before reaching the END OF THIS SCRIPT!!
#	Also Tcl/Tk V9 changes to default-namespace storage-scoping does NOT 'play'
#	well with this technique and MAY have issues EXPANDING Var values to report
#		JUGGLING the renaming of 'proc' SURROUNDING such namespace READINGS
#	(to UNdo, then REdo) would be A method to EXEMPT them from consideration.
###############################################################################
if {[set DbuG [lsearch -inline -glob -all $argv {-d*}]]!={}} {lappend DbuG --
	rename ::proc ::proc_ ;# RENAME 'proc' stmt to NEW name, then USE to redefn
	proc_ proc {nam arglst body} {
		# Each argline supplied -d spec is COMPOSED of (encoded) idioms:
		#	SIMPLE Dbg activation ->  (d) - Turned on BEFORE Run-time BEGINS
		#	what type of proc?	  ->  regular (dp) or widget (dw)
		#	RE match in/ex-clude? ->  exclude (!) or include () <- implied
		#	 OF a specific naming ->  APPENDED regexp expression
		# N.B> Cmdline ORDER of specs may(?) result in unintended implications
		#		(if so, it is was never anticipated to work in that fashion)
		if {[llength $::DbuG] > 1} { foreach d $::DbuG {
			if {[switch -glob -- $d {
			-dw!?* {expr { [string equal -len 1 $nam "."]
					   &&  [regexp [string range $d 4 end] $nam] ? [break] : 0}}
			-dw?*  {expr { [string equal -len 1 $nam "."]
					   &&  [regexp [string range $d 3 end] $nam]} }
			-dw	   {expr { [string equal -len 1 $nam "."]} }
			-dp!?* {expr {![string equal -len 1 $nam "."]
					   &&  [regexp [string range $d 4 end] $nam] ? [break] : 0}}
			-dp?*  {expr {![string equal -len 1 $nam "."]
					   &&  [regexp [string range $d 3 end] $nam]} }
			-dp	   {expr {![string equal -len 1 $nam "."]} }
			default {if {$d != "--"} {
						# Wasnt EoList sentinel? -- Bad or a plain "-d" arg
						if {$d=="-d" && !$::g(debug)} { set ::g(debug) 1
							## ANNOUNCE us (proves it was turned on)
							Dbg {$::g(name) $::g(version)} 0 "\n\n\nDbg: "
						}
						# Once Dbg turned ON, dont need '-d' IN ::DbuG anymore
						# (N.B> removes duplicates AND syntactically BAD Specs)
						set d [lsearch -exact $::DbuG $d]
						set ::DbuG [lreplace $::DbuG $d $d]
					  }
					  set d 0
				  }}]} then { proc_ $nam $arglst [concat {puts stderr \
					"[string repeat "  " [info level]][info level 0]";} $body]
				return ;# Tracking was EMBEDDED into NAMED proc
		}	}	}
		proc_ $nam $arglst $body ;# <--Does NOT embbed ANYTHING in THIS proc
	}
	# N.B> 'proc_' RUNs ONLY @SCRIPT-READ time (is REVERT'd before READin ends)
}

#### MCP
###############################################################################
# Open file for reading, with GZip decompression if needed
###############################################################################
proc gzopen { name { n "z" } } {
	global finfo

	if { [ catch { open $name } fid ] } {
		fatal-error "Couldn't open file '$name'"
	} else {
		zlib push gunzip $fid
		if { [ catch { read $fid } data ] } {
			chan pop $fid
			set zip 0
		} else {
			close $fid
			set outfile [ tmpfile $n ]
			set out [ open $outfile w ]
			puts -nonewline $out $data
			close $out
			set fid [ open $outfile ]
			set zip 1
		}

		if { $n eq "1" || $n eq "2" } {
			if { $zip } {
				set finfo(unzip,$n) $outfile
			} else {
				set finfo(unzip,$n) $name
			}
		}

		seek $fid 0
		return $fid
	}
}
#### MCP

###############################################################################
###############################################################################
# HERE BEGIN THE PROCS	(any BELOW this line are subject to execution tracking)
###############################################################################
###############################################################################

###############################################################################
# Return the name of a temporary file
# n			- a naming fragment (to help identify where/why it was created)
# forget!=0 - dont 'remember' the filename for the "destroy @ termination list"
###############################################################################
proc tmpfile {n {forget 0}} {
	global g opts UniQ

	set tmpdir [file nativename $opts(tmpdir)]
	set fnam [file join $tmpdir [pid]-$n-[incr UniQ]]
	Dbg {temp file $fnam}
	set access [list RDWR CREAT EXCL TRUNC]
	set perm 0600
	if {[catch {open $fnam $access $perm} fid ]} {
		# something went wrong
		error "Failed creating temporary file: $fid"
	}
	close $fid
	if {!$forget} {lappend g(tempfiles) $fnam}
	return $fnam
}

###############################################################################
# Execute an external command, optionally storing STDOUT into a given filename
# Returns the 3-tuple list "$stdout $stderr $exitcode"
#
# Operation is sensitive to the EXISTANCE (not value) of flag "ASYNc(trigger)"
# to run in ASYNChronous .vs. BLOCKing mode. When running ASYNC, an event loop
# is provided for dispatching tasks encountered WHILE the command is processed
###############################################################################
proc run-command {cmd {out {}}} {
	global ASYNc errorCode

	# Arrange for requested output format (given execution constraints)
	#  N.B> 'fout' targets one of: a channel, a cmd indirection, or a Variable
	if {[info exists ASYNc(trigger)]} {
		# Establish channel for cmd to WRITE into
		if {[set fout $out] != {}} {
			fconfigure [set fout [open $out wb]] -buffering none
			# PREVENT V9.x from throwing 'encoding' errors
			if {$::tcl_version >= 9.0} { fconfigure $fout -profile tcl8 }
		# (-OR- into ALIAS'd global AYSNC-var TO local name)
		} { upvar #0 ASYNc(out) STDout }
	# -OR- redirect'd DIRECTLY into a file (encoding doesnt matter - for now)
	} elseif {[set fout $out] != {}} { set fout "\">$out\"" }

	# Establish default answers
	set STDerr [set STDout ""]
	set exitcode 0
	set cmderr [tmpfile "cmderr" 1] ;# retain filename locally; WE will whack
	#	(N.B> stderr redirection prevents 'catch' from assuming msgs -> errors)

	# But the big difference in ASYNC .vs. BLOCKing is how to deal with STDOUT
	if {[info exists ASYNc(trigger)]} {
		Dbg	 "Cmd running in ASYNC mode"

		# Startup the cmd (so we can attach its stdout to the event loop) ...
		# ...where an (anonymous) handler can snag any/all STDOUT produced, but
		# more importantly WATCHES for an EOF, telling us the cmd has completed
		set cmdout [open "|$cmd \"2>$cmderr\"" rb]
		chan configure $cmdout -blocking 0 -buffering none
		chan event $cmdout readable [list apply {{fin fptr} {
					global ASYNc
					if {$fptr != {}} {
						puts -nonewline $fptr [chan read $fin]
					} else {append ASYNc(out) [chan read $fin]}
					if {[chan eof $fin]} {set ASYNc(events) 0}}} $cmdout $fout]
		# (args shown above are PASSED via these param)	 ->		   fin	  fptr
		set ASYNc(events) 1
		####
		vwait ASYNc(events) ;# wait here until we see EOF from handler above
		####
		chan configure $cmdout -blocking 1 ;# (N.B> to get errorcodes)
		if {[set failed [catch "close $cmdout"]]} {set errCODE $errorCode}
		Dbg "Back from ASYNC cmd: rc($failed)"
		if {$fout != {}} { catch {close $fout} }

	} elseif {[set failed [catch "exec $cmd $fout \"2>$cmderr\"" STDout]]} {
		set errCODE $errorCode ;# Snag this before it can get overwritten
	}

	# Suck out any error messages that MAY have been produced (and whack file)
	catch {
		set hndl [open "$cmderr" r]
		# PREVENT V9.x from throwing 'encoding' errors
		if {$::tcl_version >= 9.0} { fconfigure $hndl -profile tcl8 }
		set STDerr [read $hndl]
		close $hndl
		file delete $cmderr
	}

	if {$failed} {
		switch -- [lindex $errCODE 0] {
		"CHILDSTATUS" {
				set exitcode [lindex $errCODE 2]
			}
		"POSIX" {
				if {$STDerr == ""} {
					set STDerr $STDout
				}
				set exitcode -1
			}
		default {
				set exitcode -1
			}
		}
	}
	#Dbg "runcmd RESULTS($exitcode): out([string length $STDout])\
								err([string length $STDerr]) appropriate ?"
	return [list "$STDout" "$STDerr" "$exitcode"]
}

###############################################################################
# Populate the 'ndx'th finfo FILE via its accompanying finfo 'tmp' SCM command
# Returns descriptive msg(s) if something fails; a NUL string on Success
###############################################################################
proc scm-chkget {ndx} {
	global finfo

	# 'ndx' is a number POSSIBLY prefixed by an 'a' (for ancestor)
	#	adjust the NAMING for 'finfo(xxx)" elements accordingly
	set A "a" ; if {[string index "$ndx" 0] == $A} {
		set ndx [string range $ndx 1 end] } { set A "" }

	if {![info exists finfo(${A}pth,$ndx)]} {
		set finfo(${A}pth,$ndx) "[tmpfile scm$ndx]"
	}
	Dbg {scm-chkget ($ndx) -> '$finfo(${A}tmp,$ndx)': $finfo(${A}pth,$ndx)}

	lassign [run-command "$finfo(${A}tmp,$ndx)" "$finfo(${A}pth,$ndx)"] \
							scmOUT scmERR scmRC

	# Remember to postproccess (if needed) and ...
	if {!$scmRC} {
		if {[info exists finfo(${A}pproc,$ndx)]} {
			$finfo(${A}pproc,$ndx) "$finfo(${A}pth,$ndx)"
		}
		# ... return the erased cmd (DO NOT UNSET) to indicate Success
		return [set finfo(${A}tmp,$ndx) ""]
	}

	# This atrocity originated because CVS refuses to extract the Repo version
	# of a CONFLICTED file - but WITHOUT posting any visible REASON ... WTF? !
	#	So look for this and inject our OWN error msg
	if {[set msg "$scmERR\n$scmOUT"] == "\n" \
	&&	[string match {cvs[ .]*} $finfo(${A}tmp,$ndx)]} {
	  set msg "Is this a CONFLICTed file(?): [lindex $finfo(${A}tmp,$ndx) end]"
	}

	# Send messages back to caller only on failure
	return "$msg" ;# Failed!
}

###############################################################################
# Filter PVCS output files that have CR-CR-LF end-of-lines
###############################################################################
proc filterCRCRLF {file} {
	set outfile [tmpfile CRCRLF]
#### MCP
#	set inp [open $file r]
	set inp [ gzopen $file ]
#### MCP
	set out [open $outfile w]
	fconfigure $inp -translation binary
	fconfigure $out -translation binary
	set CR [format %c 13]
	while {![eof $inp]} {
		set line [gets $inp]
		if {[string length $line] && ![eof $inp]} {
			regsub -all "$CR$CR" $line $CR line
			puts $out $line
		}
	}
	close $inp
	close $out
	file rename -force $outfile $file
}

###############################################################################
# Return the smallest of two values (N.B> args CAN be expressions)
###############################################################################
proc min {a b} {
	return [expr {($a) < ($b) ? [expr ($a)] : [expr ($b)]}]
}

###############################################################################
# Return the largest of two values (N.B> args CAN be expressions)
###############################################################################
proc max {a b} {
	return [expr {($a) > ($b) ? [expr ($a)] : [expr ($b)]}]
}

###############################################################################
# Align (or force set on/off) Info window item visibility
###############################################################################
proc do-show-Info {{which {}}  {force {}}} {
	global g w opts

	if {$force != {}} {
		set opts($which) $force
	}

	# Detect if/when text Info windows should be mapped OR unmapped
	if {$opts(showln) || $opts(showcbs) || $g(is3way)} {
		if {! [winfo ismapped $w(LeftInfo)]} {
			grid $w(LeftInfo)  -row 0 -column 1 -sticky nsew
			grid $w(RightInfo) -row 0 -column 0 -sticky nsew
		}
	} elseif {[winfo ismapped $w(LeftInfo)]} {
		grid forget $w(LeftInfo)
		grid forget $w(RightInfo)
	}

	# The mergeInfo window (for now) is ALWAYS 'on' ...
	# However if we ever create an opt() for the "contrib markers"
	# then simply uncomment this to get it to turn on/off like above
#	if {$opts(showln) || $opts(XXX-contrib-XXX)} {
#		if {! [winfo ismapped $w(mergeInfo)]} {
#			grid $w(mergeInfo) -row 0 -column 0 -sticky nsew
#		}
#	} elseif {[winfo ismapped $w(mergeInfo)]} {
#		grid forget $w(mergeInfo)
#	}

	# In any event SOMETHING changed - ensure we utilize canvas properly
	cfg-line-info
}

###############################################################################
# Transliterate "text-tagging" precedences for Font/Bg/Fg canvas plotting
###############################################################################
proc translit-plot-txtags {twdg} {
	global g opts

	# The neccessity of this routine stems from the USER view being one of
	# setting 'text-tags' for highlighting various meta-data pgm elements,
	# because THAT was the FORMER implementation. Internally we have shifted
	# to a Canvas based technique (to reduce textline aligment issues since
	# version TK8.5), but must NOW cope with the reality of canvas-text NOT
	# providing a 'tag-precedence-stack' mechanism. Emulating a "what-would-
	# have-happened" approach is better than redefining the USER view of the
	# preferences (or auto-magically MAPPING the existing user base).
	#
	#	Technique is to pre-compute how the tagging-specified user input would
	# be precedence-stacked by the pgm so we can setup direct access to "N"
	# composite sets of values as needed when canvas-rendering the meta-data.
	#	Note that TkDiff uses MORE than simple precedence and thus SOME sets
	# might only be UTILIZED by the Left or Right view, or under values of
	# OTHER related option settings -- thus the NAMING of each set is an
	# encoding that 'plot-line-info' intends to access randomly as needed.

	# First establish a BASE precedence layer (just the Text widget settings)
	#	(what you get if NO user tagging was explicitly supplied [unlikely]).
	# For the 3 key display values we support:		  Font Fg Bg
	# plus 2 font-derivative metrics we NEED later:	  Ascent Ascent+Descent
	#	(PLUS a running MAX of certain key-character widths across ALL fonts)
	set	 Fg	 [$twdg cget -foreground]			   ;# foreground
	set	 Bg	 [$twdg cget -background]			   ;# background

	set	 Fnt "[$twdg cget -font]"				   ;# font
	set	 Aft [set Hft [font metrics $Fnt -ascent]] ;# ascent of font
	incr Hft [font metrics $Fnt -descent]		   ;# height of font

	set Dw [font measure $Fnt "8"]				   ;# Digit	 width
	set Cw [font measure $Fnt "+"]				   ;# ChgBar width
	set Sw [font measure $Fnt " "]				   ;# Space	 width
	set Mw [font measure $Fnt "M"]				   ;#  Em	 width

	# Begin the database with a snapshot of the "settings" for what is
	# (effectively) the "textopt" tag layer (plain old file lines)
	lappend DB [set nam t] "{$Fnt} $Aft $Hft $Fg $Bg"

	# Now, OVERLAY in PRECEDENCE ORDER, successive basic tags, recording each
	foreach t {difftag currtag} {

		# Turn each tagging definition into a "look up table"(lut) of its
		# contents, then look for any option names of interest, and process
		# whichever ones are found (similar to above BASE setting derivation)
		append nam [string index $t 0]
		array set lut $opts($t)
		foreach op [array names lut -regexp {\-((f|b)g|(fo[rn]|ba))}] {
			# (allow for abbreviations of the V8.5 option keywords)
			switch -glob -- $op {
			"-for*" -
			"-fg"	{ set Fg $lut($op) }						   ;# fg
			"-b*"	{ set Bg $lut($op) }						   ;# bg
			"-fon*" { set Fnt $lut($op)							   ;# font
					set	 Aft [set Hft [font metrics $Fnt -ascent]] ;# ascent
					incr Hft [font metrics $Fnt -descent]		   ;# height
					set Dw [max $Dw [font measure $Fnt "8"]] ;# maximal Dw
					set Cw [max $Cw [font measure $Fnt "+"]] ;# maximal Cw
					set Sw [max $Sw [font measure $Fnt " "]] ;# maximal Sw
					set Mw [max $Mw [font measure $Fnt "M"]] ;# maximal Mw
					}
			}
		}

		# Append this snapshot of values to the overall database
		lappend DB $nam "{$Fnt} $Aft $Hft $Fg $Bg"
		array unset lut
	}

	# DB entries 't'(text) 'td'(diff) and 'tdc'(curr) now exist IN THAT ORDER
	#
	# Next construct the mutually exclusive variations that are specifically
	# composited by the pgm when adds/chgs/dels are detected in the input files
	# onto EACH of the LAST TWO CATEGORIES. Note that specific Info-only
	# situations (eg. opts(colorcbs), highlighting) are NOT addressed here and
	# is handled during 'plot-line-info' rendering directly.
	foreach t {instag chgtag deltag overlaptag} {

		# Re-establish base settings prior to overlay of EACH mutual tag
		foreach {nam base} [lrange $DB 2 5] {
			lassign $base Fnt Aft Hft Fg Bg

			# Derive new name, then turn each tagging definition into a
			# "look up table"(lut) of its contents, looking for the option
			# names of interest, overlaying values found (same as before)
			#	Note that each new name is a MAPPING into its Chgbar mark
			append nam [string map {i + c ! d - o ?} [string index $t 0]]
			array set lut $opts($t)
			foreach op [array names lut -regexp {\-((f|b)g|(fo[rn]|ba))}] {
				# (again, allow for abbreviations of the V8.5 option keywords)
				switch -glob -- $op {
				"-for*" -
				"-fg"	{ set Fg $lut($op) }						   ;# fg
				"-b*"	{ set Bg $lut($op) }						   ;# bg
				"-fon*" { set Fnt $lut($op)							   ;# font
						set	 Aft [set Hft [font metrics $Fnt -ascent]] ;# ascent
						incr Hft [font metrics $Fnt -descent]		   ;# height
						set Dw [max $Dw [font measure $Fnt "8"]] ;# maximal Dw
						set Cw [max $Cw [font measure $Fnt "+"]] ;# maximal Cw
						set Sw [max $Sw [font measure $Fnt " "]] ;# maximal Sw
						set Mw [max $Mw [font measure $Fnt "M"]] ;# maximal Mw
						}
				}
			}

			# Append this snapshot of value to the overall database
			lappend DB $nam "{$Fnt} $Aft $Hft $Fg $Bg"
			array unset lut ;# throw away all lut tuples for next pass
		}
	}

	# Historical Note (Re: TKDIFF 4.2 and earlier)
	#	The highest precedence tag, "inlinetag", is only designed for (thus
	# overrides) 'chgtag' defined values. However, it is ONLY ever APPLIED to
	# char-ranges within the main L/R-Text widgets. Thus its color/font opts
	# NEVER applied to the actual RENDERING of Info data, despite them having
	# been (in the past) CONFIGURED into the Lnum and CB *Text widgets*. Thus
	# it AFFECTS nothing and as such, this emulation ignores it.

	# Finally, post the data needed by 'cfg-line-info' to compute canvas width
	# AND the complete database of precomputed attrs for 'plot-line-info' with
	# its 11 values: "t, td, td+, td!, td-, td?, tdc, tdc+, tdc!, tdc-, tdc?"
	set g(scrInf,cfg) "$Dw $Cw $Sw $Mw"
	set g(scrInf,tags) $DB
}

###############################################################################
# Resolve present Info window plotting configuration (AFTER any chngd settings)
###############################################################################
proc cfg-line-info {} {
	global g w opts

	# First obtain the maximal Text widget font measurements
	lassign $g(scrInf,cfg) wDig wChg wSpc wEm

	# Then establish an X position for plotting the PRIMARY Info elements such
	# that the maximal line number (if visible) will FIT to its left
	#	Values (mX, tX) for windows (Merge .vs. Text) WILL need to be distinct
	set g(scrInf,mX) [set g(scrInf,tX) \
							 [expr {$opts(showln) ? $wDig*$g(lnumDigits) : 0}]]

	# In a 3way Diff situation, make room for a Textwin "ancestral indicator"
	if {$g(is3way)} { incr g(scrInf,tX) $wEm }

	# MergeInfo always (for now) adds space for ITS (left/right) markers
	#	(but it COULD be done as a pref, by replacing 'true' with some var)
	if {[set sz [expr {( true ? $wChg+$wSpc : 0) + $g(scrInf,mX)}]]} {
		$w(mergeInfo) configure -width [incr sz 3]
		incr g(scrInf,mX) ;# 'slides' padding to 1pxl on left and 2pxl right
	}

	# Add to 'tX' any space needed for Changebars (if visible) which will
	# left-justify to that position defined above. Then INCREASE that amount
	# (+5pxl for padding) and apply it to BOTH Text Info canvases, calling it
	# "scrInf,XX" (for plotting), making the canvas EXACTLY wide enough
	#	(does NOTHING if meta-data visibility options are ALL turned off)
	if {[set sz [expr {($opts(showcbs) ? $wChg+$wSpc : 0) + $g(scrInf,tX)}]]} {
		$w(LeftInfo)  configure -width [incr sz 5]
		$w(RightInfo) configure -width [set g(scrInf,XX) $sz]
		incr g(scrInf,tX) 3;# 'slides' padding to 3pxl on left and 2pxl right
	}
}

###############################################################################
# Plot text widget line numbers and/or contrib markers in adjoining info canvas
###############################################################################
proc plot-merge-info {args} {
	global g w opts

	# Ignore this routine if not needed, havent gotten far enough in processing
	# -OR- its trigger will have zero effect on the displayed content
	if {!$g(showmerge) || $g(startPhase) < 2  \
	||	  ([llength $args] > 0 && [lindex $args 0 1] in $w(benign))} return

	# Initialize:	Empty the canvas
	#	   Identify the line range of the CDR
	#	   Import the 'tag' attr table and make it random access
	#	   Begin with NO current attr group
	$w(mergeInfo) delete all
	lassign [$w(mergeText) tag ranges currtag] sCDR eCDR
	array set attr $g(scrInf,tags)
	set aGRP {}

	# Begin at 1st VISIBLE screen text line, converting its indice->integer
	set Lnum [file rootname [$w(mergeText) index @0,0]]

	# Map/plot Lnums
	# Line numbers here are identical to widget indices. Markers derive
	# from the TAGNAMES used for each line of a given diff REGION.
	#	(PRESUMES the canvas & text widgets are physically aligned!!)
	# Stops when we walk beyond the visible range of the Text widget lines,
	# -OR- we discover the EXTRA "last line" at the bottom of the widget
	set LastLnum [file rootname [$w(mergeText) index end-1lines]]
	while {[llength [set dline [$w(mergeText) dlineinfo $Lnum.0]]] > 0} {
		if {$Lnum == $LastLnum} {break} ;# ignore extra last line

		# Detect/decode any diff(R/L) tag on the line (if it even exists)
		#	(the tag NAME encodes what SIDE the merge contribution came from)
		# N.B. tags report in priority order, thus ZERO should be where to find
		#	EITHER 'diff(R/L)' (each being of lowest prio & mutually exclusive)
		switch [lindex [$w(mergeText) tag names $Lnum.0] 0] {
		diffR { set aNewGRP [expr {$Lnum<$sCDR || $Lnum>=$eCDR ? "td" : "tdc"}]
				set side " >" }
		diffL { set aNewGRP [expr {$Lnum<$sCDR || $Lnum>=$eCDR ? "td" : "tdc"}]
				set side " <" }
		default { set side {} ; set aNewGRP "t"}
		}

		# Instantiate correct 'tag' attribute group (if it changed)
		if {"$aNewGRP" != "$aGRP"} {
			lassign $attr([set aGRP $aNewGRP]) Fnt Asc Hgt Fg Bg
		}

		# We want to plot on the same BASELINE as the text widget, but it
		# must be EMULATED as canvas '-anchor' provides NO SUCH setting.
		lassign $dline na y na na bl ;# extract TxT y and baseline
		incr y $bl	   ;# move y to its baseline then UP by the
		incr y -$Asc   ;# "plot font" ascent (=eff. NE/NW edge)

		# Plot the contributory-side marker (if any)
		if { "$side" != {}} {
			$w(mergeInfo) create text $g(scrInf,mX) $y -anchor nw  \
					-fill $Fg -font $Fnt -text "$side"
		}

		# Plot LineNum if requested
		if {$opts(showln)} {
			$w(mergeInfo) create text $g(scrInf,mX) $y -anchor ne  \
					-fill $Fg -font $Fnt -text "$Lnum"
		}
		incr Lnum
	}
}

###############################################################################
# Plot text widget line numbers and/or change bars in adjoining info canvas
###############################################################################
proc plot-line-info {side args} {
	global g w opts

	# Ignore this routine if we havent gotten far enough into the processing
	#	-OR- everything that might have displayed is turned OFF anyway
	if {$g(startPhase) < 2 \
	||	 ((!$g(is3way)) && (!$opts(showln)) && (!$opts(showcbs)))} return

	# Create session-persistent constants for NOW and FUTURE use
	if {! [info exists g(LR,Left)]} {
		set g(LR,Left)	[list Snum Enum Pad Ofst Cbar]
		set g(LR,Right) [list Snum Enum na na na Pad Ofst Cbar]
	}

	# Only redraw when args are null (meaning we were called by a binding)
	# or when called by the trace and the widget action might potentially
	# change the height of a displayed line.
	if {[llength $args] == 0 || [lindex $args 0 1] ni $w(benign)} {

		# Initialize:	Empty the canvas
		#	   Import the 'tag' attr table and make it random access
		#	   Begin with NO current attr group
		#	   Map the index of the 'current diff' to refer to g(DIFF)
		#	   Presume default first attr-group is a NON hunk-line
		$w(${side}Info) delete all
		array set attr $g(scrInf,tags)
		set aGRP {}
		set gPos [hunk-ndx [hunk-id $g(pos)] DIFF]
		set aNewGRP "t"

		# Begin at 1st VISIBLE screen text line, converting its indice->integer
		set Lnum [file rootname [$w(${side}Text) index @0,0]]

		# Now, (if >1 exists) binary-search for an APPROPRIATE start "scrInf,*"
		# entry to allow mapping 'Lnum' BACK to its ORIGINAL linenumber. We
		# want the CLOSEST item (preferrably ABOVE) the target Lnum value, but
		# BELOW is used when Lnum > last line of the final hunk. When NONE
		# exist (files are identical), the screen numbers ARE the real numbers,
		# so a dummy entry allows the remaining code to function properly.
		if {[set i $g(COUNT)]} {
			# N.B> 'rngeSrch' (unlike hunk-id, et.al) uses ZERO-based indices
			#	so increment the index UNLESS it comes back as the last entry
			if {[set i [rngeSrch DIFF $Lnum "scrInf,"]] != $g(COUNT)} {incr i}
			lassign $g(scrInf,[set hID [hunk-id $i DIFF]]) {*}$g(LR,$side)
		} else {lassign		  { 0 0 0 0 "" 0 0 "" }		   {*}$g(LR,$side) }

		# When a 3way is active, it REQUIRES a per-line 'ancestral' mapping
		#	(so figure out where to START that mapping as well)
		if {$g(is3way)} {
			set anc(max) [llength $g(d3$side)]
			set anc(ndx) [rngeSrch d3$side [expr {$Lnum - $Ofst}]]
			if {$anc(ndx) < $anc(max)} {lassign \
					[lindex $g(d3$side) $anc(ndx)] anc(fst) anc(lst) anc(mrk)
			} else { lassign	   {0 0 " "}	   anc(fst) anc(lst) anc(mrk) }
		}

		# Map/plot Lnums, advancing as needed through any mapping entries.
		# Line number translation consists of USING variables already set but
		# WATCHING for when to ADVANCE to the next sequential mapping entry.
		#	(PRESUMES the canvas & text widgets are physically aligned!!)
		# Stops when we walk beyond the visible range of the Text widget lines,
		# -OR- we discover the EXTRA "last line" at the bottom of the widget
		set LastLnum [file rootname [$w(${side}Text) index end-1lines]]
		while {[llength [set dline [$w(${side}Text) dlineinfo $Lnum.0]]] > 0} {
			if {$Lnum == $LastLnum} {break} ;# ignore extra last line

			# Waterfall test detects phase of WHAT plots WITHIN a hunk boundary
			# and establishes which tag-derived display attribute group to use
			#	(NB. purely Pad'ded lines always skip plotting altogether)
			if {$i > 0 && $Lnum >= $Snum} {
				if {$Lnum > ($Enum - $Pad)} {
					if {$Lnum > $Enum} {
						if {$i < $g(COUNT)} {
							# Step forward to the next hunk mapping
							#	loading the NEXT scrInf,* entry settings
							set hID [hunk-id [incr i] DIFF]
							lassign $g(scrInf,$hID) {*}$g(LR,$side)
							if {[info exists g(overlap$hID)]} {set Cbar "?"}
							# Restart loop if 'Lnum' is NOW INSIDE the params
							# of the newly read-in hunk (to support abutted
							# hunks created by the Split/Combine feature)
							if {$Lnum >= $Snum}	 continue
						# Special fixup needed when FINAL hunk had padding
						} elseif {$Pad} {incr Ofst $Pad; set Pad 0 }
						set CB false ; set aNewGRP "t" ;# Is beyond entry
					} else { incr Lnum ; continue }	   ;# A	 PADDING line
				} else { set CB $opts(showcbs)		   ;# A	 DIFFed	 line
						 set aNewGRP [expr {$i==$gPos ? "tdc$Cbar":"td$Cbar"}]}
			} else {set CB false ; set aNewGRP "t" }   ;# Is before entry

			# Instantiate correct 'tag' attribute group (if it changed)
			if {"$aNewGRP" != "$aGRP"} {
				lassign $attr([set aGRP $aNewGRP]) Fnt Asc Hgt Fg Bg
			}

			# We want to plot on the same BASELINE as the text widget, but it
			# must be EMULATED as canvas '-anchor' provides NO SUCH setting.
			lassign $dline na y na na bl ;# extract TxT y and baseline
			incr y $bl	   ;# move y to its baseline then UP by the
			incr y -$Asc   ;# "plot font" ascent (=eff. NE/NW edge)

			# FINALLY plot THIS Lnum and/or ChgBar per the CURRENT options
			# Do ChgBars 1st (more often skipped), with NW-corner as locpt.
			# Subsequent Linenumber will uses NE-corner at the SAME locpt.
			#	(Annoyingly, canvas text has NO "Bg"-cell - must emulate!)
			# Weird flipping of colors just mimics the way tags were APPLIED
			# when this was all done in a Text widget (as of TkDiff 4.2)
			if {$CB && "$Cbar" != ""} {
				# Highlight Chgbars ? (i.e. colored Bg or Fg)
				if {$opts(tagcbs)} {
					if {$opts(colorcbs)} { switch -- $Cbar {
						"!" -
						"?" { set Cfg [set Cbg $opts(mapchg)] }
						"+" { set Cfg $opts(mapdel) ; set Cbg $opts(mapins) }
						"-" { set Cfg $opts(mapins) ; set Cbg $opts(mapdel) }
						}
					} else { lassign "$Fg $Bg" Cfg Cbg }

					# Make/plot a fontsized ChangeBar "background rect"
					set yy $Hgt
					set Dims [list $g(scrInf,tX) $y $g(scrInf,XX) [incr yy $y]]
					$w(${side}Info) create rect $Dims -fill $Cbg -outline $Cbg
				} else	{ set Cfg $Fg }

				$w(${side}Info) create text $g(scrInf,tX) $y -anchor nw \
					-fill $Cfg -font $Fnt -text " $Cbar"
			}

			if {$opts(showln)} {
				# Highlight LineNum ?
				if {$opts(tagln) && "$Cbar" != ""} {
					# Make/plot a fontsized Lnum "background rect"
					if {$g(is3way)} {
						set xx [lindex $g(scrInf,cfg) 3] ;# ancestral mark ofst
					} { set xx 0 }
					set yy $Hgt
					set Dims [list $g(scrInf,tX) $y [incr xx] [incr yy $y]]
					$w(${side}Info) create rect $Dims -fill $Bg -outline $Bg
				}

				$w(${side}Info) create text $g(scrInf,tX) $y -anchor ne	 \
					-fill $Fg -font $Fnt -text "[expr {$Lnum - $Ofst}]"
			}

			# Insert the 'ancestral' marker if a 3way is in progress
			#	(and we haven't walked off the list of markers altogether)
			if {$g(is3way) && $anc(ndx) < $anc(max) \
			&&	($Lnum - $Ofst) >= $anc(fst) && ($Lnum - $Ofst) <= $anc(lst)} {
				# Markers generated from OTHER side display in inverse video,
				# thus make/plot a fontsized marker "background rect"
				if {[string is upper $anc(mrk)]} {
					set xx [lindex $g(scrInf,cfg) 3] ;# ancestral mark width
					set yy $Hgt
					set Dims [list 1 $y $xx [incr yy $y]]
					$w(${side}Info) create rect $Dims -fill $Fg -outline $Fg

					set Fg3 $Bg; # (which forces us to flip the text color)
				} else { set Fg3 $Fg }

				$w(${side}Info) create text 1 $y -anchor nw	 \
										  -fill $Fg3 -font $Fnt -text $anc(mrk)

				# Step map forward to next triplet (when 'last' has been used)
				if {$anc(lst) == $Lnum - $Ofst} {
					lassign [lindex $g(d3$side) [incr anc(ndx)]] \
							anc(fst) anc(lst) anc(mrk)
				}
			}

			incr Lnum
		}
	}
}

###############################################################################
# Split file containing CVS (or other?) conflict markers into 2 (3?) tmp files
#	 name	Name of input file containing conflict markers
#	 ndx	Highest CURRENT finfo indice (entries added here must be higher)
#	 whose	optional identity Augmentation (eg. the SCM it came from?)
#
# N.B> Its possible a THIRD file (an ancestor) may be seen in the input format
#	(file+marker syntax is as produced by 'diff3 -m Mine [Ancestor] Theirs')
###############################################################################
proc split-conflictfile {name ndx {whose {}}} {
	global g finfo

#### MCP
#	if {[catch {set input [open $name r]}]} {
#		fatal-error "Couldn't open file '$name'"
#	}
	set input [ gzopen $name ]
#### MCP

	# Must derive the SPECIFIC finfo indices we plan to populate
	#	(due to the processing technique being a parallel, NOT sequential one)
	#	ie. 'L'eft	'R'ight	 (and 'A'ncestor when needed)
	set R 1
	set A [expr {[incr R [set L [incr ndx]]] / 2}]

	# Initialize the files/streams/names/flags to start (beyond 1st 4 - empty!)
	#
	# N.B> CANT create finfo(albl,$ndx) until data is SEEN	(it triggers 3way!)
	lassign "7 [open [set finfo(pth,$L)	  [tmpfile cf1]] w]	   \
			   [open [set finfo(pth,$R)	  [tmpfile cf2]] w]	   \
			   [open [set finfo(apth,$A)  [tmpfile cfa]] w]"   \
			out	 f1	 f2	 fa			Re1			finfo(atmp,$A) \
			finfo(lbl,$L) finfo(tmp,$L) finfo(lbl,$R) finfo(tmp,$R)

	# Read/copy input into 'out' files as directed by embedded markers
	while {[gets $input line] >= 0} {
		# The FIRST marker tells us whose marking FORMAT to follow
		if {$Re1 == ""} {
			if {[regexp {^<<<<<<<* +} $line]} {
				# This maps 'diff3-like' merge markers
				set Re1	 {^<<<<<<<* +(.*)}
				set Re2 {^=======*}
				set Re3	 {^>>>>>>>* +(.*)}
				set Re4 {^\|\|\|\|\|\|\|* +(.*)}

			} elseif {[regexp {^>>>>>>>* +} $line]} {
				# This maps ??WHOSE?? markers
				#		   (and why did they invent their OWN?)
				# (***Pls ADD identifying comment!!***)
				set Re1	 {^>>>>>>>* +(.*)}
				set Re2 {^<<<<<<<* +(.*)}
				set Re3	 {^=======*}
				set Re4 {^\|\|\|\|\|\|\|* +(.*)}
			}
		}
		# Dont bother with matching until we find the first marker
		if {$Re1 != ""} {
			if {[regexp $Re1 $line na name]} {
				# First Marker: following data was from SECOND file
				if {$finfo(lbl,$R) == "" && $name != ""} {
					set finfo(lbl,$R)  "[shortNm $name] ($whose Cflct)" }
				set out 2

			} elseif {[regexp $Re2 $line na name]} {
				# Second Marker: following data was from FIRST file
				if {$finfo(lbl,$L) == "" && $name != ""} {
					set finfo(lbl,$L)  "[shortNm $name] ($whose Cflct)" }
				set out 1

			} elseif {[regexp $Re3 $line na name]} {
				# Third Marker: following data is COMMON to ALL files
				if {$finfo(lbl,$L) == "" && $name != ""} {
					set finfo(lbl,$L)  "[shortNm $name] ($whose Cflct)" }
				set out 7

			# FINDING the 4th Marker indicates there WAS an Ancestor!!
			} elseif {[regexp $Re4 $line all name]} {
				# Fourth Marker: following data was from Ancestor file
				if {![info exists finfo(albl,$A)] && "$name" != ""}	  {
					set finfo(albl,$A) "[shortNm $name] ($whose Cflct)" }
				set out 4

			} else {
				if {$out & 1} { puts $f1  $line }
				if {$out & 2} { puts $f2  $line }
				if {$out & 4} { puts $fa  $line }
			}
		} else {
			puts $f1  $line
			puts $f2  $line
			puts $fa  $line
		}
	}
	close $input
	close $f1
	close $f2
	close $fa

	# If for some reason no names were detected, invent SOMETHING ...
	# N.B> Existence of an Ancestor is IMPLICIT within the data
	if {$finfo(lbl,$L) == ""}  {set finfo(lbl,$L)  "theirs ($whose Cflct)"}
	if {$finfo(lbl,$R) == ""}  {set finfo(lbl,$R)	 "ours ($whose Cflct)"}

	# Cleanup & return highest indice used (Ancestors NEVER get counted)
	if {![info exists finfo(albl,$ndx)]} {array unset finfo "a\[pt]*,$A"}
	return $R
}

###############################################################################
# Derive list SrcCodeManagement systems that seem VALID for given dir/file/URL
# (N.B> dir/file candidates that DONT EXIST will SKIP choices that require it)
###############################################################################
proc scm-detect {fn {extra {}}} {

	regsub -all {\$} $fn {\$} fn   ;# (Backslash any '$' ciphers as literal)
	# Use dirname OF argument if it is not a directory already
	# (N.B> isdirectory & dirname are tilde-safe - no thrown errors)
	if {[file isdirectory $fn]} {set dnam $fn} {set dnam [file dirname $fn]}

	# There are basically FOUR 'possibilities' for detection:
	# 1		those determined by the naming of the file (or its directory)
	# 2		those that require some ADJOINING file structure naming
	# 3		those requiring external-executables to be invoked
	# 4		those that depend on existance of certain ENV variables
	#
	### (unknown if a better order exists: one below is purely historical)
	### *My* gut feeling is the precedence described above should be followed
	### (which is NOT completely the case as it exists here) however, as some
	### cases are combo/subsets of others there is plenty of room for debate.
	#
	# In any event, this is now a voting process (former if-else chain) where
	# the user gets to pre-state their choice PROVIDED its an allowed one.
	lappend scms
	if {[file isdirectory [file join $dnam CVS]]}	 { lappend scms CVS }
	if {[is-repo-dir ".svn" $dnam]}					 { lappend scms SVN }
	if {[is-repo-git]}								 { lappend scms GIT }
	if {[string match {*://*} $fn]}					 { lappend scms SVN }
	if {[sccs-is-bk]}								 { lappend scms BK }
	if {[file isdirectory [file join $dnam SCCS]]}	 { lappend scms SCCS }
	if {[file isdirectory [file join $dnam RCS]]}	 { lappend scms RCS }
	if {[file isfile $fn,v]}						 { lappend scms RCS }
	if {[file exists [file join $dnam vcs.cfg]] || \
		[info exists ::env(VCSCFG)]}				 { lappend scms PVCS }
	if {[info exists ::env(P4CLIENT)] || \
		[info exists ::env(P4CONFIG)]}				 { lappend scms Perforce }
	if {[info exists ::env(ACCUREV_BIN)]}			 { lappend scms Accurev }
	if {[info exists ::env(CLEARCASE_ROOT)]}		 { lappend scms ClearCase }
	if {[is-repo-dir ".hg" $dnam]}					 { lappend scms HG }
	if {[is-vpath $dnam]}							 { lappend scms Vpath }

	# We occasionally need to ADD a 'pseudo SCM' to the end of a NONEMPTY list
	if {$extra != "" && [llength $scms]} {lappend scms $extra}
	return $scms
}

###############################################################################
# Scm-detect HELPER Fcn (attempts file normalize on dirname that MAY NOT exist)
###############################################################################
proc is-repo-dir {trgnam dirname} {
	# check for targnam directory in all parent directories
	# (N.B> impossible if dirname itself doesnt exist)
	if {[catch {file normalize $dirname} dirname]} {
		return false
	}
	set prevdir {}
	while {$dirname != $prevdir} {
		set chkDnam [file join $dirname $trgnam]
		if {[file isdirectory $chkDnam]} { return true }
		set prevdir $dirname
		set dirname [file dirname $dirname]
	}
	return false
}

###############################################################################
# Scm Detect HELPER Fcn (runs Git provided command to determine ?CWD? location?
###############################################################################
proc is-repo-git {} {
	return [expr [catch {eval "exec git rev-parse --is-inside-work-tree"}] ==0]
}

###############################################################################
# Scm Detect HELPER Fcn	  Returns:
# =0 if finfo(Vpath) was never loaded from a missing VPATH EnvVar at startup
# =0 if arg is not prefixed by ANY of the VPATH list elements of finfo(Vpath)
# OTHERWISE which element (1->N) matched (note: this is +1 of its index value)
###############################################################################
proc is-vpath {dnam} {
	global	finfo
	# When finfo(Vpath) was never loaded, answer is NO
	if {[info exists finfo(Vpath)]} {
		# But we also need to know if the directory of the targeted filename
		# is fully WITHIN at least ONE of the elements of that VPATH.
		#	N.B> VPATHs are expected to be ABS-paths, so normalize input first
		if {[catch {file normalize $dnam} dnam]} {
			return 0
		}
		foreach vp $finfo(Vpath) {
			if {[incr ndx]
			&& [string equal -length [string length $vp] $vp $dnam]} {
				return $ndx }
		}
	}
	return 0
}

###############################################################################
# Scm Detect HELPER Fcn	  Returns: truth of BitKeeper ?presence?
###############################################################################
proc sccs-is-bk {} {
	set cmd [auto_execok "bk"]
	set result 0
	if {[string length $cmd] > 0} {
		if {![catch {exec bk root} error]} {
			set result 1
		}
	}
	return $result
}

###############################################################################
# Decide which Src Code Managment system is expected to obtain the current file
###############################################################################
proc scm-elect {scms vote} {
	Dbg {Elect Candidates($scms) Vote($vote) for [info level -1]} -1

	# Simply apply the users vote
	#	N.B> This makes it APPEAR that either SCM meta-value (Auto or None)
	#	always results in just TAKING the top entry - the trick is that when
	#	'scms' was setup by 'newDiff' it likely CONTAINS 'None' as a
	#	candidate value, making it electable here, based on the 'vote'
	# This allows the caller to recognize that access was BLOCKed not MISSING
	if {$vote in $scms} {return $vote ;# new democratic way ...user choice
	} else				{return [lindex $scms 0]} ;#ye olde way...1st found
}

###############################################################################
# Obtain a revision of a file:
#	fn		requested file name
#	ndx		index in finfo array to place data ('-ndx' implies Ancestor naming)
#	rev		"" implies SCM will use ITS default (generally 'most recent')
#	Scm		if !Null, which SCM to use (avoids lookup)
#	probe	when true - non-existance is NOT a reason to fail
# Returns 0 (Success) or 1 (Failed + diagnostic messages produced)
###############################################################################
proc get-file-rev {fn ndx {rev ""} {Scm {}} {probe 0}} {
	global g opts finfo tcl_platform

	# First, some simple initializations common to ALL
	regsub -all {\$} $fn {\$} fn   ;# (Ensure any '$' ciphers remain literal)
	set cmdsfx ""		;# To prevent 'exec'-spoofing on Windows platform(?)
	if {$tcl_platform(platform) == "windows"} { set cmdsfx ".exe" }

	# Ancestor files are stored into a slightly adjusted array element name
	#	N.B> 'ndx' AS PASSED *can* be an EXPRESSION (not just a number): resolve!
	if {($ndx) < 0} {
		set A "a"; set ndx [expr {-1 * ($ndx)}]
	} { set A "";  set ndx [expr $ndx] }

	# PRESUME eventual success ... **THEN** ...
	set MSG [set msg {}]
	set stillbad 0
	#	(Dbl-Check file existence - But MUST let URL thru: *IS* no way to Chk)
	if {![string match {*://*} $fn] && ![file exists $fn]} { set stillbad 1 }

	# ... DETECT and FORMULATE the appropriate SCM command to request the file
	#		(if one was not already pre-selected as a parameter)
	#	N.B> The 'None' choice is PRESERVED when it was originally present
	if {"" == $Scm} {
		set Scm [expr {!($ndx & 1)}]			   ;# Get from the correct side
		set ScmVote [lindex $g(scmPrefer) $Scm]	   ;# obtain CURRENT preference
		set Scm [expr {"None" in $finfo(scm[incr Scm])? "None" : ""}]  ;# None?
		set Scm [scm-elect [scm-detect $fn $Scm] $ScmVote]
	}

	# HOWEVER - only SVN will handle a URL; report instead when SCM cant
	#	-UNLESS- its not URL/SVN at all and filename DOESNT EXIST
	if {$Scm == "SVN" || !$stillbad} {
	switch -- $Scm {
	CVS {
		append cmd "cvs" $cmdsfx
		if {[set lbl $rev]!=""} {
			set rev "-r $lbl"}	{ set rev "-r [set lbl "HEAD"]" }
		# For CVS, if it isn't checked out, there is neither a CVS nor RCS
		# directory.  It will however have a ,v suffix just like rcs.
		#	(There is not necessarily a RCS directory for RCS, either...)
		#	(however, if not, then the file will ALWAYS have a ,v suffix.)
		set finfo(${A}lbl,$ndx) "[shortNm $fn] ($Scm $lbl)"
		set finfo(${A}tmp,$ndx) "$cmd update -p $rev \"$fn\""
	}
	SVN {
		append cmd "svn" $cmdsfx
		if {[set lbl $rev]!=""} { set rev "-r $lbl"
		# ??? SVN is WEIRD - has multiple Rev formats but ALLOWS only
		#	HEAD if the FSpec is a URL ?? Expect SVN errors to occur!!
		} elseif {[string match {*://*} "$fn"]} {
			set rev "-r [set lbl "HEAD"]"
		} { set rev "-r [set lbl "BASE"]" }
		# Subversion directly ALLOWS a URL instead of a true filename
		set finfo(${A}lbl,$ndx) "[shortNm $fn] ($Scm $lbl)"
		set finfo(${A}tmp,$ndx) "$cmd cat $rev \"$fn\""
	}
	GIT {
		if {[is-repo-git]} {
			# Only works if you are actually INSIDE the work tree
			append cmd "git" $cmdsfx; # Default revision is the 'stage'
			if {[set lbl $rev]==" "	 ||	 $rev==""} {
				 set lbl "--staged" ; set rev ":"} {set rev "$rev:"}
			set finfo(${A}lbl,$ndx) "[shortNm $fn] ($Scm $lbl)"
			set finfo(${A}tmp,$ndx) \
				"$cmd show \"$rev[exec $cmd rev-parse --show-prefix]$fn\""
		} {set MSG "Please re-start from within a Git work tree."}
	}
	BK {
		append cmd "bk" $cmdsfx
		if {[set lbl $rev]!=""} {
			set rev "-r$lbl"}  { set rev "-r[set lbl "HEAD"]" }
		set finfo(${A}lbl,$ndx) "[shortNm $fn] ($Scm $lbl)"
		set finfo(${A}tmp,$ndx) "$cmd get -p $rev \"$fn\""
	}
	SCCS {
		append cmd "sccs" $cmdsfx
		if {[set lbl $rev]!=""} {
			set rev "-r$lbl"}  { set rev "-r[set lbl "HEAD"]" }
		set finfo(${A}lbl,$ndx) "[shortNm $fn] ($Scm $lbl)"
		set finfo(${A}tmp,$ndx) "$cmd get -p $rev \"$fn\""
	}
	RCS {
		append cmd "co" $cmdsfx
		if {[set lbl $rev]==""} {set lbl "HEAD"}
		set finfo(${A}lbl,$ndx) "[shortNm $fn] ($Scm $lbl)"
		set finfo(${A}tmp,$ndx) "$cmd -p$rev \"$fn\""
	}
	PVCS {
		append cmd "get" $cmdsfx
		if {[set lbl $rev]!=""} {
			set rev "-r$lbl"}  { set rev "-r[set lbl "HEAD"]" }
		set finfo(${A}lbl,$ndx) "[shortNm $fn] ($Scm $lbl)"
		set finfo(${A}tmp,$ndx) "$cmd -p $rev \"$fn\""
		set finfo(${A}pproc,$ndx) "filterCRCRLF"
	}
	Perforce {
		append cmd "p4" $cmdsfx
		if {[set lbl $rev]!=""} {
			set rev "#$lbl"}  { set rev "#[set lbl "HEAD"]" }
		set finfo(${A}lbl,$ndx) "[shortNm $fn] ($Scm $lbl)"
		set finfo(${A}tmp,$ndx) "$cmd print -q \"${fn}$rev\""
	}
	Accurev {
		append cmd "accurev" $cmdsfx
		if {[set lbl $rev]!=""} {
			set rev "-v \"$lbl\""}	{ set rev "-v \"[set lbl "HEAD"]\"" }
		set finfo(${A}lbl,$ndx) "[shortNm $fn] ($Scm $lbl)"
		set finfo(${A}tmp,$ndx) "$cmd cat $rev \"$fn\""
	}
	ClearCase {
		set cmd "cleartool"
		# is this NOT a Windows tool (why no append of .exe?)

		# list given file
		Dbg {exec $cmd ls -s \"$fn\"}
		catch {exec $cmd ls -s \"$fn\"} ctls
		# get the path name to file AND its (present?) revision info
		#	(either CHECKEDOUT or a number)
		if {![regexp {(\S+)/([^/]+)$} $ctls na path checkedout]} {
			set MSG "Couldn't parse ct ls output '$ctls'"
			break
		}

		# Compute the version PRIOR to the one FOUND
		if {$checkedout == "CHECKEDOUT" || $checkedout == 0} {
			if {$checkedout == 0} {
				set path [file dirname $path]
			}
			set pattern "create version \"($path/\[^/\]+)\""
		} else {
			incr checkedout -1
			set pattern "create version \"($path/$checkedout)\""
		}

		# Search history of the file for the determined version on our branch
		Dbg {exec $cmd lshistory -last 50 \"$fn\"}
		catch {exec $cmd lshistory -last 50 \"$fn\"} ctlshistory
		set lines [split $ctlshistory "\n"]
		set prior ""
		foreach line $lines {
			if {[regexp $pattern $line na prior]} {
				# Point DIRECTLY at the requested file
				# However, make it APPEAR like it IS a tmpfile
				#	(so we will deny invoking an editor later)
				set finfo(${A}lbl,$ndx) "[shortNm $fn] ($Scm $prior)"
				set finfo(${A}pth,$ndx) $prior
				set finfo(${A}tmp,$ndx) ""
				break
			}
		}
		if {$prior == ""} {set MSG "Couldn't resolve $fn, gave up..."}
	}
	HG {
		append cmd "hg" $cmdsfx; # Mercurial support
		if {[set lbl $rev]!=""} {
			set rev "-r$lbl"}  { set rev "-r[set lbl "PARENT"]" }
		set finfo(${A}lbl,$ndx) "[shortNm $fn] ($Scm $lbl)"
		set finfo(${A}tmp,$ndx) "$cmd cat $rev \"$fn\""
	}
	Vpath {
		# Has no 'access' cmd: files simply EXIST (or not) in stacked dirs
		# The supplied 'rev' value says which GENERATION of the file to get:
		#	ZERO is topmost (and the default), ONE is PRIOR-to topmost
		#	N.B> finfo(Vpath) was ALREADY top-pruned (by CWD) as needed
		set vp $finfo(Vpofst) ;# But report Vpath depth as per original VPATH
		if {("$rev"!={} && $rev) || $vp} { set lbl "Prior" } {
			set lbl Topmost
			set rev 0
		}

		# Absolute files are PREFIX-matched to identify which node
		if {[string index $fn 0]=="/" && [file exists $fn]} {
		foreach vNod $finfo(Vpath) {
			if {[incr vp] && [string first $vNod/ $fn]} {
				if {$rev} { incr rev -1
				} { set finfo(${A}lbl,$ndx) "[shortNm $fn] (${Scm}(#$vp)-$lbl)"
					set finfo(${A}pth,$ndx) $fn
					if {$vp} { set finfo(${A}tmp,$ndx) "" }
					break
			}	  }
		}} else {
		# Relative files are POSTFIX-joined to identify which node
		foreach vNod $finfo(Vpath) {
			if {[incr vp] && [file exists $vNod/$fn]} {
				if {$rev} { incr rev -1
				} { set finfo(${A}lbl,$ndx) "[shortNm $fn] (${Scm}(#$vp)-$lbl)"
					set finfo(${A}pth,$ndx) [file join $vNod $fn]
					# Make it THINK its a tmpfile if its DOWN the Vpath
					if {$vp > 1} { set finfo(${A}tmp,$ndx) "" }
					break
		}}	}	  }

		if {![info exists finfo(${A}pth,$ndx)]} {
			# Error report depends on whether this was a PROBED request,
			# yet we must STILL pass back a failed-RC if probe is active
			if {$probe} { set stillbad 1 } {
				set msg "$lbl Vpath entry for $fn does not exist"
			}
		}
	}
	None {
		set msg "Did your preferred SCM system ($Scm) block file:\n"
		append msg "	$fn\nfrom its intended SCM repository?"
	}
	default {if {$probe} { set stillbad 1 } {
				# ... but DONT FLAG non-existence, if it was NOT required...
				set msg "File '$fn' is not part of a revision control system"
			}
	}
	}} elseif {stillbad} {
		set msg "File '$fn' does not appear to EXIST?"
	} { set msg "$Scm does not accept URL-based File specifications" }

	# Note label for this file WILL BE overridden (just NOT here, not NOW)
	if {[info exists finfo(ulbl,$ndx)] && $finfo(ulbl,$ndx) != {}} {
		Dbg {  User label: $finfo(ulbl,$ndx) OVERRIDES finfo(lbl,$ndx)}
	}

	# If NO errs (and in 1st pairing) NOW is the time to actually GET the file
	# (even that of an ancestorfile if its required)
	#	N.B> Oddball reason NOT to is if its a Vpath'd file IN the TOP node...
	#		which means its just THERE (and edittable - NO finfo(tmp,*) !!
	if {![string length "$MSG$msg"] && !$stillbad && $ndx < ("$A"=="" ? 3:2)
	&& [info exists finfo(${A}tmp,$ndx)]
	&& [string length $finfo(${A}tmp,$ndx)]} {
			watch-cursor "Accessing $finfo(${A}lbl,$ndx)"
			set MSG [scm-chkget ${A}$ndx]
			restore-cursor
		}

	# Report any errors (THIS GETS slightly INVOLVED):
	#	We need to send back the message(s?) RESPONSIBLE for the RetCode -
	# but the CALLER has the job of sorting out what to DO with them
	if {[string length "$MSG$msg"] || $stillbad} {
		if {"$msg" != ""} { uplevel 1 set msg "{$msg}" }
		if {"$MSG" != ""} { uplevel 1 set MSG "{$MSG}" }
		return 1
	} { return 0 }
}

###############################################################################
# Obtain an ordinary file
#	fn		requested file name
#	ndx		finfo array index to place data ('-' ndx implies Ancestor naming)
#	probe	a probed request ALLOWS non-existence to be considered successfull
# Returns:	0  Success	(WITH NO observable message activity)
#			1  Failed	(PLUS a 'pushed' HARD-error message to the caller)
# (N.B> will reject a URL w/'not exist' IFF asked [2Fspec+0Rev] seems possible)
###############################################################################
proc get-file {fn ndx {probe 0}} {
	global g opts finfo

	# Ancestor files are stored into a slightly adjusted array element name
	#	N.B> 'ndx' AS PASSED *can* be an EXPRESSION (not just a number): resolve!
	if {($ndx) < 0} {
		set A "a"; set ndx [expr {-1 * ($ndx)}]
	} { set A "";  set ndx [expr $ndx]}

	# Verify filename actually exists BEFORE aaking more questions
	#	(NO callers EXPECTED to ask for a default-Rev URL - it simply rejects)
	set MSG {}
	if {![file exists $fn]} {
		# But DO NOT REPORT non-existence if this attempt was ONLY a probe
		if {$probe} { return 1 } { set MSG "File '$fn' does not exist" }
	} elseif {[file isfile $fn] || $fn == $opts(NULLdev)} {
		set finfo(${A}lbl,$ndx) [shortNm [set finfo(${A}pth,$ndx) "$fn"]]
	} else { set MSG "'$fn' exists, but is not a file" }

	# Messaging is PUSHED back to caller (so a BOOLEAN can be returned)
	if {$MSG!={}} {
		uplevel 1 set MSG "{$MSG}"
		return 1
	}
	return 0
}

###############################################################################
# Read the command line (syntactic errors MAY result in usage + termination)
# Returns: =0 incomplete (requires interactive assistance OR SCM search mode)
#		   >0 success (enough info SUPPLIED for at least 1 pairing to exist)
#		   <0 File was found to not exist (also needs interactive assistance)
###############################################################################
proc CmdLn {} {
#### MCP
#	global g opts finfo argv argc
	global g opts finfo argv argc lsd
#### MCP

	# Initialize:						N.B> Ancestor data is NEVER 'counted'
	lassign "[llength $opts(ignoreRegexLnopt)] {}	0	 0	  0	   0	0"\
					 ignRxs				  missing ARGi lbls URLs pths revs
	set D -1 ;# Local-master Dbg switch within this proc

	# Loop through argv, storing revision args in finfo(rev,[12]) and
	# filespec args in finfo(f,[12]). revs and pths are counters.
	# N.B> 'URLs' as a LOCAL variable serves a different purpose in EACH proc
	#		you find it in: here it counts URLs that LACK a SPECIFIC Rev;
	#		Other procs simply use it (locally) for THEIR distinctive purposes.
	while {$ARGi < $argc} {
		Dbg "Examining arg #${ARGi}([set arg [lindex $argv $ARGi]])" 0 "CmdLn: "
		switch -regexp -- $arg {
		"^-h" -
		"^--help" {
				help-concept cline
				exit 0
			}
		"^-d.*" {
				# ::DbuG Specs previoulsy acummulated and/or Dbg activated
			}
		{^-@.*$} {
				# First, de-tangle the option value from the option flag
				# to get just the Ancestor Rev data
				if {[string length $arg] > 2} {
					set rev [string range $arg 2 end]} {
					set rev [lindex $argv [incr ARGi]]
				}
				# First to set this locks it in place, EXCEPT for a URL@Rev
				if {$finfo(rev,0) == ""} {set finfo(rev,0) $rev}
			}
		{^-[vr].*$} {
				# All 'rev' option(s) are ganged together here to share logic:
				# Cant just 'count and store' because it MAY be INTENDED to
				#	backfill a PRE-existing URL Fspec that lacked specific
				#	Rev data (using the pairing-of-args rules); Yet URLS only
				#	count @Rev instances, merges thus just 'happen'

				# First, de-tangle the option value from the option flag
				if {[string length $arg] > 2} {
					set rev [string range $arg 2 end]} {
					set rev [lindex $argv [incr ARGi]]
				}

				# Might THIS Rev PAIR with a PRIOR default-versioned URL?
				#
				#		 + Rev(s) CLAIM to exist (but COULD be URL-paired)
				#	then + 2 Fspecs exist (need to check 1st one)
				if {$pths && [set i [min $revs 2]]
				&&	(($i == 2 && [string match {*://*} $finfo(f,2)]) ||
					 ($i == 1 && [string match {*://*} $finfo(f,1)]))} {
					if {$finfo(rev,$i) != ""} { set i [incr revs] }
				} { set i [incr revs] }
				Dbg "Rev,$i <- ($rev) revs($revs)" $D "CLI\t:"
				# PERMIT up to the 1st two Revs; though errors may COUNT more
				if {$i < 3} {set finfo(rev,$i) $rev}
			}
		"^-L$" {
				set finfo(ulbl,[incr lbls]) [lindex $argv [incr ARGi]]
			}
		"^-L.*" {
				set finfo(ulbl,[incr lbls]) [string range $arg 2 end]
			}
		"^-conflict$" {
				set g(conflictset) 1
			}
		"^-o$" {
				set g(mergefile) [lindex $argv [incr ARGi]]
			}
		"^-o.*" {
				set g(mergefile) [string range $arg 2 end]
			}
		"^-u$"	{
				# Ignore flag generated from "svn diff --diff-cmd=tkdiff"
			}
		"^-B$"	{
				set opts(ignoreEmptyLn) 8
			}
		"^-I$"	{
				lappend opts(ignoreRegexLnopt) [lindex $argv [incr ARGi]]
			}
		"^-I.*"	 {
				lappend opts(ignoreRegexLnopt) [string range $arg 2 end]
			}
		{^-[12]$} {
				set opts(predomMrg) [string range $arg end end]
			}
		"^-psn" {
				# Ignore the Carbon Process Serial Number
				set argv [lreplace $argv $ARGi $ARGi]
				incr argc -1
				incr ARGi
			}
		{^-R$} {
				# Authorize recursion when TWO directories are specified
				set finfo(fRecurs) 1
			}
		"^-P$"	{
				# Step PAST the Preference filename (we grabbed it already)
				#  IFF that is what it was - otherwise it is an actual DIFF arg
				#	(depends on	 Preferences LOCATION being a Dir, or not)
				if {[info exists g(rcdir)]} { incr ARGi } {
					append opts(diffcmd) " " [concat "$arg"]
				}
			}
		"^-P.*"	 {
				# User NAMED (and we already grabbed) the Preference filename
				#  (just swallow it - CANT be the Diff arg; there IS a value!)
			}
#### MCP
		"^-lsd$" {
				set lsd 1
			}
#### MCP
		"^-" {	# Args not otherwise recognized are passed to Diff directly
				#	(WRONG if it was SUPPOSED to grab NEXT item as its VALUE)
				append opts(diffcmd) " " [concat "$arg"]
			}
		{^-a.*$}  -
		default {
				# ALL input fnames come through here (in particular: Ancestor)
				#
				# First, de-tangle any option value from the option flag
				#	(if it even HAD one - only the Ancestor filespec will)
				# establishing slot index and counts, (NEVER count an Ancestor!)
				if {$arg == "-a"} {
					set N 0			  ; set path [lindex $argv [incr ARGi]]
				} elseif {[string match {-a*} $arg]} {
					set N 0			  ; set path [string range $arg 2 end]
				} { set N [incr pths] ; set path $arg }

				# Wow - this is gonna be convoluted - NEED to KNOW if its a URL
				# even though we might never use IT (or its Rev) *IFF* ...
				#		the COUNTS have exceeded the upper-limit of TWO!
				#	ALL just to report a SYNTAX ERROR correctly!

				# Identify if its REALLY a URL (SVN based syntax)
				#	A URL is effectively a Rev, DESPITE any un-specified @Rev!
				#	  (due to implying its own inherent default value)
				if {[string match {*://*} $path]} { Dbg "IS a URL" $D "CLI\t:"
					# If URL has a @Rev attached, will need to STRIP+RELOCATE it
					#	unless its an attempted 2nd Ancestor (-> simply ignored)
					#		(but ensure 'rev' is ALWAYS initialized)
					if {([set rev ""]=="") && ($N || $finfo(f,$N) == "")
					&&	[set at [string last "@\{" $path]] > 0
					|| ([set at [string last  "@"  $path]] > 0 &&
						[string first "/" $path $at] < 0)} {

						# SPLIT them and GRAB EACH individually
						set rev	 [string range $path $at+1 end ]
						set path [string range $path   0  $at-1]
						Dbg "WITH an @Rev($rev)" $D "CLI\t:"
					} { Dbg "with NO @Rev supplied" $D "CLI\t:"}

					# Regardless, count it WAS a URL *UNLESS*
					#	it was an Ancestor (which is NEVER counted)
					if {$N} { incr URLs }

					# Next, deal with Rev COUNTING (ie. 'revs'):
					#	Rev Slot(N) may've been preloaded (-r) earlier thus can
					#	MERGE w/Fspec(N) IFF 'rev' is EMPTY w/NO revs incr
					if {$rev!=""} {
						# Shift POSSIBLE PRE-enterred Rev1 upward
						#	(N.B> *MAY* be moving nothing at all !!)
						if {$N==1} { set info(rev,2) $finfo(rev,1) }
						# SPECIFIED Rev goes into same slot as URL will
						if {$N < 3} { set finfo(rev,$N) "$rev" }
						incr revs
					}
					# Else URL will MERGE with PRIOR "-rRev" (or just BE empty)

					# Sadly - pretend URL exists ... (CANT PROVE otherwise)
					set fexist 1
					Dbg "PLACED AS Rev,$N w/RevCount($revs)" $D "CLI\t:"
				# else its a plain FILE Fspec: But verify tilde+Glob was HANDLED
				#	(ordinarily done by SHELL already - but if was QUOTED? )
				} elseif {$N>2 || [tildChk $path fexist path] || $fexist != 1} {
					Dbg "IS a FILE for (f,$N) marked as non-exist" $D "CLI\t:"
					# HAH - caught you!! Forget the msg, just deal w/existence
					set fexist 0

				# else SAFELY expand any POTENTIAL Glob-syntax into REAL NAME
				} { set path [glob -n $path] }

				# But if FILE (not URL) actually wont exist, TkDiff needs the
				# USER to Repair/Retry again. Arrange to make RetCode NEGATIVE
				if {$N<3 && !$fexist} { set missing "-"
				} { Dbg "for (f,$N)<-($path) w/NO ERROR" $D "CLI\t:" }

				# PERMIT up to the 1st two Fspecs; though more COUNT as errors
				# (N.B> Yes - Ancestor still gets here BECAUSE it isnt counted)
				if {$pths < 3} { set finfo(f,$N) $path
				} { Dbg "Rejected for slot f,$N" $D "CLI\t:" }
			}
		}
		incr ARGi
	}

	# Check for an OVERFLOW of revision and/or file args provided
	#	(Basic command line SYNTAX mistakes made)
	Dbg "Syntax CHK: $pths filespecs($URLs URLs), $revs revisions"
	if {$revs > 2 || $pths > 2} {
		if {$pths > 2} {
			puts stderr "$g(name): Error: specify at most 2 filespecs"
		}
		if {$revs > 2} {
			puts stderr "$g(name): Error: specify at most 2 revisions"
		}
		help-concept cline
		exit 1
	}

	# Underflow is trickier - ZERO Fspecs *may* be legal given an appropriate
	# CWD and compliant SCM. Even ZERO revs can be OK IF the user permitted it
	#
	#	Basically this is all about AVOIDING "newDiff" (IFF requested)
	# when ZERO Fspec args (and POSSIBLY zero Revs) have been provided
	set g(scmDOsrch) 0
	set g(scmPrefer) "$opts(scmPrefer)" ;# <-- Make the default 'active'
	if {!$pths} {
		# The automatic way out is a SINGLE, preferred, searchable SCM, with
		# either given Revs, -OR- the users REQUEST that searching is desired.
		# Otherwise it all loads into the dialog and the user can handle it
		# N.B> do not simplify logic: 'scmDOsrch' is NEEDED by 'assemble-args'

		# First, resolve which SIDE may have a viable SCM (if any)
		set scms [scm-detect "."] ;# (need only 'detect' once w/CWD for both)
		if {[set finfo(scm1) [scm-elect "$scms" [lindex $g(scmPrefer) 0]]] \
							 in $g(scmSrch)} {incr g(scmDOsrch) 1}
		if {[set finfo(scm2) [scm-elect "$scms" [lindex $g(scmPrefer) 1]]] \
							 in $g(scmSrch)} {incr g(scmDOsrch) 2}

		# Finally - check if we now have a DEFINITIVE choice ...
		#	(if both SCMs are the same, it counts as just one)
		if {$g(scmDOsrch) != 3 \
		|| ("$finfo(scm1)" == "$finfo(scm2)" && [incr g(scmDOsrch) -1])} {

			# ... (and the Revs -OR- users OK to just go DO it)
			if {$g(scmDOsrch) && $opts(autoSrch) && !$revs} {
				incr revs ;# go STRAIGHT to processing (no dialog)
			}
		}
		# If revs is ALSO ZERO here, newDiff dialog will be presented next
		#Dbg "DOsrch($g(scmDOsrch)) skipdialog($revs)"
	}

	# Notice and act upon certain imperative settings:
	#	- mark merge file as INITIALLY known (thus triggering the merge window)
	#	- turn on Regex line skipping *if* it was added here (else its a pref)
	if {$g(mergefile) != ""} {set g(mergefileset) 1}
	if {$ignRxs < [llength $opts(ignoreRegexLnopt)]} {set opts(ignoreRegexLn) 4}

	return [expr "${missing}($revs + $pths)"]
}

###############################################################################
# Check provided filename to see if its leading portion can be SHORTENNED.
#	fn:		candidate filename
#	tild:	optional 2nd-choice value to replace HOME value with
#			DEFAULTS to tilde (but SHOULD be set to "" to prohibit 2nd choice)
#				(anything ELSE and it BEST NOT END with a SLASH!!)
#
# Intent is to produce shorter NAMES of filenames in things like:
#			 menu-items, msgs and labels
# RETURNS unchanged filename when modification(s) is not possible
#
#	N.B>	DO NOT USE if the return value COULD find its way to 'exec':
#	   Invoked PGMs (notably Diff) DONT always natively accept 'tilde' names!
###############################################################################
proc shortNm {fn {tild {~}}} {
	global env ;# Should NOT BE MEANS to OBTAIN users HOME location (spoofable)

	# Begin by trying to eliminate the current working directory from the name
	# N.B> +/- 1 ndx game and '/' avoids a potential home-dir SUBSET naming err
	set ndx [string length [set lead [pwd]]]
	if {[string equal -length [incr ndx] "$lead/" "$fn"]} {
		return "[string range "$fn" $ndx end]"

	# IFF that fails to MATCH, then try for replacing a lead $HOME with $tilde
	#	UNLESS: env(HOME) doesnt EXIST	-OR- callers prohibits it (tild=="")
	} elseif {[info exists env(HOME)] && [string length tild]} {
		set ndx [string length [set lead $env(HOME)]]
		if {[string equal -length [incr ndx] "$lead/" "$fn"]} {
			return "$tild[string range "$fn" [incr ndx -1] end]"
		}
	}

	# If NOTHING could be shortened - just return the ORIGINAL filename
	return "$fn"

}

###############################################################################
# Check if provided filename (NOT PATH) is on list of stated EXCLUSIONs
###############################################################################
proc xclude { fn } {
	global opts

	set fn [file tail $fn]
	foreach x $opts(xcludeFils) {
		if {[string match $x $fn]} { return 1 }
	}
	return 0
}

###############################################################################
# Process the arguments, whether from the command line or from the dialog
# Returns: >1 success (= number of files that apparently exist)
#			  (INCLUDES obtaining the first PAIR)
#		   =1 failure (can not continue)
#		   =0 ?successfully? produced nothing to compare... retry?
###############################################################################
proc assemble-args {} {
#### MCP
#	global g opts finfo
	global g opts finfo lsd
#### MCP

	set O([set O(2) 1]) 2;#	 (just a simple meta-pgm 'O'ther identity value)

	# RE-establish how many files and revs we got from the GUI or CmdLn
	#	(An AncestorFile - finfo slot ZERO - is NEVER part of the count)
	# However, a URL must count as BOTH (even w/o a Rev - due to IMPLIED dflts)
	#	N.B> 'URL' here tracks WHICH slot(s) (L=1/R=2) contain a URL Fspec
	lassign {0 0 0} URL revs  pths
	foreach i [array names finfo {f,[12]}] {
		if {$finfo($i) != ""} {
			# This weirdness bumps the Rev cnt when a URL did NOT specify one
			#	(because the ones that DID will be counted shortly)
			# and notes which slot (1, 2, or BOTH=3) a URL was enterred on
			if {[string match {*://*} $finfo($i)]} {
				if {$URL} {set URL 3} {set URL [string index $i end]}
				if {$finfo([string map {f rev} $i]) == ""} {incr revs}
			}
			incr pths
		}
	}
	foreach i [array names finfo {rev,[12]}] {if {$finfo($i)!=""} {incr revs} }

	# Save any current DERIVED values (in case NEWLY produced ones fail)
	set priorVals [array get finfo {[aptl]*[0-9]}]
	array unset finfo {[aptl]*[0-9]}

	Dbg " Recovered $pths filespecs, $revs revisions"

#	The task here is to deal with trying to expand all GIVEN args into PAIRS
#	of things to compare, thus validating *syntactically* what should happen.
#	Note that SEMANTIC correctness (can we actually OBTAIN what is described)
#	will (mostly) occur later.
#		Basic argument ASSUMPTIONs -
#		- when NO SCM is involved, only LOCAL files will participate, possibly
#			aggregated by involving a Directory as one/both of the Filespecs.
#		- when an SCM is needed (because one or more revisions exist), we 1st
#			PRESUME the OTHER FSPEC (if any) refers to a REAL (dir or file)
#			object which MAY be in the sandbox (or not), UNLESS its a URL.
#		- when only a single (or no) revision is provided, then some FILE (in
#			or out of the sandbox) will likely participate unless BOTH are URLs
#		- when TWO revs are given, NO FILES from the sandbox are used (except
#			for possible name generation); revisions ALWAYS create temp files
#			FROM the SCM, even if either were to MATCH that of the sandbox.
#		- finally, if NO ARGS are provided, CERTAIN capable SCM systems MAY
#			generate their OWN list of files AND revisions, PROVIDED the user
#			has authorized such action by SETTING the AutoSrch option.

	# Establish NAMED placeholders for messaging (MSG=BAD msg=recoverable)
	#	NOTE: called fcns KNOW of these names and *may* load them independently
	# Also initialize count of how many IMPLIED files are ultimately derived
	# ...AND a marker that messaging WAS produced and MUST be evaluated
	set Why [set MSG [set msg {}]]
	set Err [set cnt 0]

	# A 'conflict' file is a special animal (1 file representing 1->3 files)!
	if {$g(conflictset)} {
		if {$revs == 0 && $pths == 1} {
			#################################################################
			# tkdiff -conflict FILE			  (N.B> does NOT preclude a 3way)
			#################################################################
			Dbg "Applying Pairing RECIPE: conflict file"

			# Conflict files can come from multiple SCM toolsets, or even a
			#	'diff3 -m Mine [Ancestor] Yours' command. The names entered
			#	into finfo are DERIVED from embedded MARKER lines inside it
			#	(while the CONTENT gets spread out into separate tmpfiles)
			set cnt [split-conflictfile "$finfo(f,1)" $cnt]
		} else {
			set Why "'-conflict' run can specify ONLY 1 filespec (we saw $pths)"
			set Err 1
		}

	# Identify input PATTERN (#Fspecs and #Revs) and ASSEMBLE pairs from it
	} else {
		# DETERMINE the proper SCM(s) to request individual files (when needed)
		foreach N {1 2} {
			#	('shorten' the VARname(s) - conserves coding linespace later)
			set f($N) $finfo(f,$N)	 ;	 set r($N) $finfo(rev,$N)

			# VOTE which SCM to use (pths==0 uses a DIFFERENT Vote strategy)
			if {$N <= $pths} {
				# Yet, must treat NON-existent FILE as a reportable Warning?
				#	(sadly dont KNOW how to do same for URL)
				# But even a non-file MIGHT choose a vaguely accurate SCM
				if {![string match {*://*} $f($N)] && ![file exists $f($N)]} {
					if {$Why == ""} { append Why "File(s) do not exist:" }
					append Why "\n\t'$f($N)"
					set Err 1
				}
				set ScmVote [lindex $g(scmPrefer) $N-1]	   ;# obtain preference
				set Scm($N) [scm-elect [scm-detect $f($N) None] $ScmVote]

			# There CANT be a 2nd SCM if there ISNT a 2nd Fspec
			#	(simply means BOTH Revs - if any - come from the SAME 1st SCM)
			} { set Scm($N) {} }

			# PAY ATTENTION:
			# *User provided* REV data is summarily REMOVED for "Vpath" -
			#		(because no definitive Rev-ID actually exists)
			#	THAT MEANS having to UN-count it (IFF was passed) but ALWAYS
			#	writing it as REQUESTING the *appropriate* "latest" revision
			if {$Scm($N) == "Vpath"
			||	$N == 2 && $Scm(1) == "Vpath" && $Scm(2) == ""} {
				if {$r($N) != ""} { incr revs -1 }
				set r($N) 0 ; # Generally we want the LATEST Vpath version
				#	BUT - if a ONE-Fspec SCM comes up as Vpath, then the LEFT
				# side needs to be an EARLIER version than the one that will
				# show up for the RIGHT side; the TWO-Fspec format MUST NOT!
				# (must be ABLE to compare DISTINCT Fspecs that just *happen*
				# to fall within a common VPATH; even if resolved to SAME file)
				if {$N==2 && $Scm(2) == ""} { set r(1) 1 }
			}
			Dbg "  f,${N}($f($N)) rev,${N}($r($N))" ;# ONLY ever ADJUSTS Vpath revs!
		}

		Dbg " PAIRing SCM info: SCM($Scm(1)/$Scm(2))"

		# UNLESS the input has been deemed UNUSABLE ...
		if {$Err} { set msg $Why

		# ... DERIVE the given input PATTERN into PAIRED file set(s)
		} elseif {$revs <= 2 && $pths == 0} {
			#################################################################
			#  tkdiff		(inquiry or interactive)  (simply NO input given)
			#						 -OR-
			#  tkdiff -rREV							   ($CWD is)  SCM sandbox
			#  tkdiff -rREV1 -rREV2					  (with 1 or 2 revisions)
			#################################################################

			#	Some SCMs can produce their OWN list of files 'known' to be
			# different; POSSIBLY with no input whatsoever. So detect the SCM
			# first, THEN (if it is one) let *it* try. All other cases lead
			# to error msgs (if revs were given).
			# Note that DETECTING the SCM was based on the current PROCESS dir
			# and that 'scmPrefer' used here is DERIVED from the preference
			#	N.B> 'SScm' (Srch-SCM): so named to avoid array/scalar conflict
			if {$g(scmDOsrch)} {
				set SScm [lindex $g(scmPrefer) $g(scmDOsrch)-1]	   ;#VOTE first
				set SScm [scm-elect $finfo(scm$g(scmDOsrch)) $SScm];#then ELECT
			} else {set SScm [concat $g(scmPrefer)]};# <-- it WONT be searchable
			Dbg "Applying Pairing RECIPE: SCM-inquiry ($SScm)"

			switch -glob -- "$SScm" {
			GIT {
				# N.B: An input syntax of '-r ' (or '-r " "') is the Git Index
				if {$opts(autoSrch) || $g(scmDOsrch)} {
					if {[set cnt [inquire-git $revs]] & 1} {
						incr cnt -[set Err 1]
					}
				} else { set Err 1
					set msg "You denied access for $SScm to search for files"
				}
			}

			SVN {
				if {$opts(autoSrch) || $g(scmDOsrch)} {
					# This could take some time, so let user know we are busy
					watch-cursor "Inquiring of SVN for files..."
					if {[set cnt [inquire-svn $revs]] & 1} {
						incr cnt -[set Err 1]
					}
					restore-cursor
				} else { set Err 1
					set msg "You denied access for $SScm to search for files"
				}
			}

			CVS {
				if {$opts(autoSrch) || $g(scmDOsrch)} {
					# This could take some time, so let user know we are busy
					watch-cursor "Inquiring of CVS for files..."
					if {[set cnt [inquire-cvs $revs]] & 1} {
						incr cnt -[set Err 1]
					}
					restore-cursor
				} else { set Err 1
					set msg "You denied access for $SScm to search for files"
				}
			}

			"* *" {
				set msg "no searchable SCM was detected/designated\n"
				if {([lindex $SScm 0]!= ""	   && [lindex $SScm 1]!= "") \
				&&	([lindex $SScm 0]!= "Auto" || [lindex $SScm 1]!= "Auto")} {
					append msg "	  were your SCM settings '$SScm' at fault ?"
				}
				set Err 1
			}

			default {
				if {"$SScm" != "" } {
					set msg "the $SScm SCM system needs at least 1 Fspec given"
				} { set MSG "no SCM was detected for the current directory" }
				set Err 1
			}
			}

		} elseif {$revs < 2 && $pths == 1} {
			#################################################################
			#  tkdiff		FSPEC  (file in, dir at, URL .vs.) SCM sandbox
			#  tkdiff -rREV FSPEC			  with or without a revision)
			#################################################################
			Dbg "Applying Pairing RECIPE: FSpec(1) Revs(0/1)"

			# URL 'side' is determined by which arg was first: Fspec or Rev
			# Any other arg syntax is ALWAYS Left =SCM(@REV) and Right =File
			if {$URL} {
				if {$r($URL) == $r($O($URL))} {
					set MSG "There is NO point in comparing a file to itself"
					set Err 1
				} {
					if {[get-file-rev "$f($O($URL))" $O($URL) "$r($O($URL))" $Scm($O($URL))]} {
						array unset finfo "\[ptl]*,$O($URL)" ; set Err 1
					} elseif {[get-file-rev "$f($URL)" $URL "$r($URL) $Scm($URL)"]} {
						array unset finfo "\[ptl]*,\[12]" ; set Err 1
					} else {incr cnt 2}
				}

			} elseif {[file isdirectory [set P $f(1)]]} {
				# Only in this Dir, or the whole Tree ?
				foreach D [expr {($finfo(fRecurs) && $Scm(1) in $g(scmS))
													 ? [DFSobj Dir $P] : $P } ] {
					foreach P [glob -nocomplain -directory $D -types f *] {
						if {[xclude $P]} { continue }
						# N.B> Uses names IN Dir(s) to PROBE the SCM Sandbox
						if {[get-file-rev "$P" $cnt+1 "$r(1)" $Scm(1) 1]} {
							array unset finfo "\[ptl]*,[expr $cnt+1]" ; set Err 1
						} elseif {[get-file "$P" $cnt+2 1]} {
							array unset finfo "\[ptl]*,[expr $cnt+1]" ; set Err 1
							array unset finfo "\[ptl]*,[expr $cnt+2]"
						} else {incr cnt 2}
					}
				}
			} else {
				if {[get-file-rev "$P" 1 "$r(1)" $Scm(1)]} {
					array unset finfo "\[ptl]*,1" ; set Err 1
				} elseif {[get-file "$P" 2]} {
					array unset finfo "\[ptl]*,\[12]" ; set Err 1
				} else {incr cnt 2}
			}

		} elseif {$revs == 2 && $pths == 1} {
			#################################################################
			#  tkdiff -rREV1 -rREV2 FSPEC	 (file in, dir at) SCM sandbox
			#################################################################
			Dbg "Applying Pairing RECIPE: FSpec(1) Revs(2)"

			if {[file isdirectory [set P $f(1)]]} {
				# Only in this Dir, or the whole Tree ?
				foreach D [expr {($finfo(fRecurs) && $Scm(1) in $g(scmS))
													? [DFSobj Dir $P] : $P } ] {
					foreach P [glob -nocomplain -directory $D -types f *] {
						if {[xclude $P]} { continue }
						# N.B> Uses names IN Dir(s) to PROBE the SCM Sandbox
						if {[get-file-rev "$P" $cnt+1 "$r(1)" $Scm(1) 1]} {
							array unset finfo "\[ptl]*,[expr $cnt+1]" ; set Err 1
						} elseif {[get-file-rev "$P" $cnt+2 "$r(2)" $Scm(2) 1]} {
							array unset finfo "\[ptl]*,[expr $cnt+1]" ; set Err 1
							array unset finfo "\[ptl]*,[expr $cnt+2]"
						} else {incr cnt 2}
					}
				}
			} else {
				if {[get-file-rev "$P" 1 "$r(1)" $Scm(1)]} {
					array unset finfo "\[ptl]*,1" ; set Err 1
				} elseif {[get-file-rev "$P" 2 "$r(2)" $Scm(2)]} {
					array unset finfo "\[ptl]*,\[12]" ; set Err 1
				} else {incr cnt 2}
			}

		} elseif {$revs == 0 && $pths == 2} {
			############################################################
			#  tkdiff FSPEC1 FSPEC2			(dirs, files or mixed)
			############################################################
			Dbg "Applying Pairing RECIPE: FSpec(2) Revs(0)"

			# One, the other, or both may be directories
			# Regardless, the same FILE name must exist in EACH to be paired
			if {[file isdirectory $f(1)] && [file isdirectory $f(2)]} {
				# Should we DO just this one level -or-	 Recursively descend ?
				if {!$finfo(fRecurs) || [string trim $opts(egnSrchCmd)]=={}} {
					foreach P [glob -nocomplain -directory $f(1) -types f *] {
					if {[xclude $P]} { continue }
					#N.B. "file isfile xxx" thankfully WON'T fault OS softlinks
						if {[file isfile \
								   [set F [file join $f(2) [file tail $P]]]]} {
							set finfo(lbl,[incr cnt]) \
											[shortNm [set finfo(pth,$cnt) $P]]
							set finfo(lbl,[incr cnt]) \
											[shortNm [set finfo(pth,$cnt) $F]]
						}
					}
				# Recursive - go for it!
				#	but may need to "normalize" cnt/Err if IT posts a message
				} elseif {[set cnt [inquire-diff]] & 1} {
					incr cnt -[set Err 1]
				}

				# EITHER method MAY result in NO usable input if they simply
				# have no actual COMMON filenames
				if {!$cnt && !$Err} { set Err 1
					set msg "Given directories have NO filenames in common"
				}
			# maybe only ONE was a directory ...
			} elseif {[file isdirectory $f([set i 1])] ||
					  [file isdirectory $f([set i 2])]} {
				 if {[get-file [file join $f($i) [file tail $f($O($i))]] $i]} {
					set MSG "Searched file $f($O($i)) non-existant in: $f($i)"
					array unset finfo "\[ptl]*,$i" ; set Err 1
				} elseif {[get-file "$f($O($i))" $O($i)]} {
					array unset finfo "\[ptl]*,\[12]" ; set Err 1
				} else { incr cnt 2 }
			} else {
				# Otherwise they are just files
				if {[get-file "$f(1)" 1]} {
					array unset finfo "\[ptl]*,1" ; set Err 1
				} elseif {[get-file "$f(2)" 2]} {
					array unset finfo "\[ptl]*,\[12]" ; set Err 1
				} else { incr cnt 2 }
#### MCP
				# Record LSD temporary file
				if { ! $Err } {
					if { $lsd && [ string equal [ file extension $f(1) ] ".tmp" ] } {
						set finfo(lsd,1) $f(1)
					}
					if { $lsd && [ string equal [ file extension $f(2) ] ".tmp" ] } {
						set finfo(lsd,2) $f(2)
					}
				}
#### MCP
			}

		} elseif {$revs > 0 && $pths == 2} {
			#################################################################
			#  tkdiff -rREV1  FSPEC1	(file in, dir at, URL) SCM sandbox
			#	(+)	 [-rREV2] FSPEC2		(same or distinct) SCM sandbox
			#	   (can compare ACROSS a branch/WC boundary or distinct SCMs)
			#################################################################
			Dbg "Applying Pairing RECIPE: FSpec(2) Revs(1/2)"

			if {[file isdirectory $f(1)] && [file isdirectory $f(2)]} {
				# Should we DO just this one level -or-	 Recursively descend ?
				if {!$finfo(fRecurs) || [string trim $opts(egnSrchCmd)]=={}} {
					foreach P [glob -nocomplain -directory $f(1) -types f *] {
						# Use sandbox for name intersection - thus NOT probing!
						if {[xclude $P] || ![file isfile [set F \
								[file join $f(2) [file tail $P]]]]} {continue}

						if {[get-file-rev "$P" $cnt+1 "$r(1)" $Scm(1)]} {
							array unset finfo "\[ptl]*,[expr $cnt+1]"
							set Err 1
						} elseif {[get-file-rev "$F" $cnt+2 "$r(2)" $Scm(2)]} {
							array unset finfo "\[ptl]*,[expr $cnt+1]"
							array unset finfo "\[ptl]*,[expr $cnt+2]"
							set Err 1
						} else { incr cnt 2 }
					}
				# Recursive - go for it!
				#	but may need to "normalize" cnt/Err if IT posts a message
				} elseif {[set cnt [inquire-diff]] & 1} {
					incr cnt -[set Err 1]
				}

				# EITHER method MAY result in NO usable input if they simply
				# have no actual COMMON filenames
				if {!$cnt && !$Err} { set Err 1
					set MSG "Provided Dirs had NO filename in common"
				}
			} elseif {[file isdirectory $f([set i 1])] ||
					  [file isdirectory $f([set i 2])]} {
				set P "[file join $f($i) [file tail $f($O($i))]]"
				if {[get-file-rev "$P" $cnt+1 "$r($i)" $Scm($i)]} {
					array unset finfo "\[ptl]*,[expr $cnt+1]" ; set Err 1
				} elseif {[get-file-rev "$f($O($i))" $cnt+2 "$r($O($i))" $Scm($O($i))]} {
					array unset finfo "\[ptl]*,[expr $cnt+1]" ; set Err 1
					array unset finfo "\[ptl]*,[expr $cnt+2]"
				} else { incr cnt 2 }
			}  else {
				# N.B> for EACH Fspec, we ALLOW an SCM (if permissable)
				if {($URL&1) || $r(1)!=""} {
					if {[get-file-rev "$f(1)" 1 "$r(1)" $Scm(1)]} {
						array unset finfo "\[ptl]*,1" ; set Err 1}
				} elseif {!($URL&1)} {
					if {[get-file "$f(1)" 1]} {
						array unset finfo "\[ptl]*,1" ; set Err 1}
				}

				if { !$Err && (($URL&2) || $r(2)!="")} {
					if {[get-file-rev "$f(2)" 2 "$r(2)" $Scm(2)]} {
						array unset finfo "\[ptl]*,\[12]" ; set Err 1}
				} elseif {!$Err && !($URL&2)} {
					if {[get-file "$f(2)" 2]} {
						array unset finfo "\[ptl]*,\[12]" ; set Err 1}
				}

				if {!$Err} { incr cnt 2 }
			}

		}
	}

	Dbg "Final status: FSpecs($pths) Revs($revs) -> $cnt/2 pairings	 $Err Err"

	# Tell user if ANYTHING went wrong (POSSIBLY superimposing Err INTO cnt)
	if {$Err} {
		if {$MSG!={}} {
			if {[winfo exists .newDiff]} { popmsg "Error: $MSG" } {
				puts stderr "Error: $MSG"
			}
			# Wipeout any GENERATED values (its a loss anyway) restore init val
			array unset finfo {[aptl]*[0-9]}
			array set finfo $priorVals
			set cnt 1

		} elseif {$msg!={}} {
			if {[winfo exists .newDiff]} { popmsg $msg warning "Warning"} {
				puts stderr "Warning: $msg"
			}
			# N.B> when MSG has POSTED this makes NO REAL chgs; BUT if not, it
			#	forces a -1, UNLESS we also found usable files .... for which
			#	it will then operate AS-IF to increment (making 'cnt' ODD).
			# This "odd"ness has no particular special meaning, although it
			# WOULD indicate (to main) that this wasn't a "clean" input parse.
			# At present, 'main' doesn't care and simply uses the found files.
			if {!$cnt} {set cnt -1} { set cnt [expr $cnt|1] }
		}
	}

	# Derive any additional target-related values (preparing to move forward)
	if {$cnt > 1} {
		set finfo(fCurpair) 1
		set finfo(fPairs) [expr {$cnt / 2}]

		if {[set P $finfo(f,0)] != {}} {
			# The USER may only SPECIFY a 3way when its a SINGLE comparison
			# Otherwise we silently erase their attempt
			if {$cnt == 2} {
				# Unlike other files, Ancestors can ONLY come from an SCM when
				# a rev is given (because DEFAULTING it to the most recent
				# check-in defeats its purpose) (?? but what about 'BASE' ??)
				if {[set r0 $finfo(rev,0)] != ""} {
					if {[get-file-rev "$P" -$finfo(fCurpair) "$r0"]} {
						array unset finfo "a\[ptl]*,1"
					}
				} elseif {[get-file "$P" -$finfo(fCurpair)]} {
					array unset finfo "a\[ptl]*,1"
				}
			} else { lassign {} finfo(f,0) finfo(rev,0) }
		}
	}
	# TELL "main" what we want done next...
	#	-1 = Force RESTART
	#	 0 = Simply EXIT
	#	 1 = RESTART (if in graphics mode) or ABORT (when not)
	#	>1 = Process ($cnt/2) file pairs (INTEGER divide ignores ANY +1)
	return $cnt
}

###############################################################################
# Align various label decorations to the CURRENT input file pairing
###############################################################################
proc alignDecor {pairnum} {
	global g w opts finfo

	# Establish if 3way mode is NOW active and what file indices are in use
	set g(is3way) [info exists finfo(albl,$pairnum)]
	Dbg "is3way($g(is3way))"
	set	 ndx(1) [set ndx(2) [expr {$pairnum * 2}]]
	incr ndx(1) -1

#### MCP
#	set finfo(title) \
#		"[file tail $finfo(lbl,$ndx(1))] .vs. [file tail $finfo(lbl,$ndx(2))]"
	if { [ info exists finfo(ulbl,$ndx(1)) ] && $finfo(ulbl,$ndx(1)) != { } } {
		set Llbl $finfo(ulbl,$ndx(1))
	} else {
		set Llbl $finfo(lbl,$ndx(1))
	}
	if { [ info exists finfo(ulbl,$ndx(2)) ] && $finfo(ulbl,$ndx(2)) != { } } {
		set Rlbl $finfo(ulbl,$ndx(2))
	} else {
		set Rlbl $finfo(lbl,$ndx(2))
	}
	set finfo(title) \
		"[ file tail $Llbl ] .vs. [ file tail $Rlbl ]"
#### MCP

	# Set file labels (possibly overridden) and a Tooltip for REAL files
	foreach {LR n} {Left 1 Right 2} {
		if {[info exists finfo(ulbl,$ndx($n))] && $finfo(ulbl,$ndx($n)) !={}} {
			set finfo(lbl,$LR) $finfo(ulbl,$ndx($n))	;# Override lbl display
		} else {set finfo(lbl,$LR) $finfo(lbl,$ndx($n))}

		if {![info exists finfo(tmp,$ndx($n))] \
			&& $finfo(pth,$ndx($n)) != $opts(NULLdev)} {
			#	(N.B> Tip data will ALSO be used by report generation heading)
			set	   g(tooltip,${LR}Label) "{$finfo(pth,$ndx($n))\n"
			append g(tooltip,${LR}Label) \
							"[clock format [file mtime $finfo(pth,$ndx($n))]]}"
		} { set	   g(tooltip,${LR}Label) {}}
		set_tooltips $w(${LR}Label) "$g(tooltip,${LR}Label)"
	}

	# Add/Remove the Ancestor indicator (and its tooltip) as needed
	if {$g(is3way)} {
		grid $w(AncfLabel) -row 0 -column 1
		if {![info exists finfo(atmp,$pairnum)]} {
			set	   tipdata "{$finfo(apth,$pairnum)\n"
			append tipdata "[clock format [file mtime $finfo(apth,$pairnum)]]}"
		} { set	   tipdata "{$finfo(albl,$pairnum)}"}
		set_tooltips $w(AncfLabel) "$tipdata"
	} else {
		set_tooltips $w(AncfLabel) {}
		grid forget $w(AncfLabel)
	}

	# Unlock a preset mergefile name if the CURRENT pairing COULD be arbitrary
	if {$finfo(fPairs) > 1} {set g(mergefileset) 0}

	# Guess the best 'mergefile' name for the CURRENT pairing (if not preset)
	if {! $g(mergefileset)} {
		# If BOTH are tmpfiles, lets go with just the file itself in the CWD...
		if {[info exist finfo(tmp,$ndx(1))]&&[info exist finfo(tmp,$ndx(2))]} {
			set rootname [file rootname [file tail $finfo(pth,$ndx(1))]]
			set suffix [file extension $finfo(pth,$ndx(1))]
		} else {
			# ...or lets pair it to the NON-tempfile location (Left preferred)
			if {[info exists finfo(tmp,$ndx(1))]} {set i 2} {set i 1}
			set rootname [file rootname $finfo(pth,$ndx($i))]
			set suffix [file extension $finfo(pth,$ndx($i))]
		}
		set g(mergefile) [file join [pwd] "${rootname}-merge$suffix"]
	}
	Dbg "MergeFileSet($g(mergefileset)): $g(mergefile)"

	wm title . "$finfo(title) - $g(name) $g(version)"
	return 0
}

###############################################################################
# While not TRULY an 'inquiry' it DOES use Diff to recursively FIND files
###############################################################################
proc inquire-diff {} {
	global opts finfo

	set cnt 0
	set MSG [set msg {}]

	# Will ask Diff which files DIFFER in the given directories (recursively)
	#	N.B> RE creates 3 filename FIELDS out of the TWO (trailing) fnames by
	#	FIRST knowing how many OTHER args preceded it. Besides the OVERALL
	#	match field (#0) returned, the OTHER 3 returned fields will be:
	#		1st unique prefix, common suffix value, 2nd unique prefix
	set cmd [formOpts egnSrchCmd]
	set xtractFNre \
		"^diff(?: +\[^ ]*){[expr [llength $cmd] - 1]} +(.+?)(.*) (.+?)\\2\$"
	lappend cmd $finfo(f,1) $finfo(f,2)

	show-status "Executing {$cmd}"
	lassign [run-command $cmd] dOUT dERR dRC

	# DO NOT RECORD $dRC as g(returnValue) in this context !!
	#	It would be inappropriate as the EXIT code
	if {$dRC < 0 || $dRC > 1 || $dERR != ""} {
		set MSG "Diff FAILED (rc=$dRC):\n$dERR"
	} elseif {$dRC == 0} {
		set msg "Diff reports NO diffs between: $finfo(f,1) $finfo(f,2)"
	}

	# Parses output using a "RegExp 'recognizer'" to accomodate OTHER engines
	# which would (most likely) produce distinctly different syntactic forms
	#	N.B> DO NOT simplify this into a "switch -regexp"... (--> TCL bug!)
	#	Somehow "switch" would PREVENT the $xtractFNre RE from matching
	foreach line [split $dOUT "\n"] {
		switch -glob -- $line {
		{Files *} {
		# GNU Diff -r output syntax (of ALL files): WITH '-q' option:
			set fpath [regexp -inline -- {^Files (.+) and (.+) differ$} $line]

			set finfo(lbl,[incr cnt]) [shortNm [set finfo(pth,$cnt) \
															[lindex $fpath 1]]]
			set finfo(lbl,[incr cnt]) [shortNm [set finfo(pth,$cnt) \
															[lindex $fpath 2]]]
		}
		{diff --git *} {
		# GIT Diff bears a marked resemblance to what "Diff -r" produces, but
		# also MANGLES the filenames slightly. But the BIGGEST pain is it wont
		# honor attempts to EXCLUDE files by name (Claims it can via ":!file",
		# but doesnt WORK, at least NOT in a NON-repo environment)!
			set fpath [regexp -inline -- $xtractFNre $line]

			# Need to carefully REMOVE the ficticious A/B naming prefixes
			foreach {n f} "1 [lindex $fpath 1] 3 [lindex $fpath 3] 0" {
				if {[string match {[ab]/.} $f} {
					lset fpath $n [string range $f 2 end]
				} elseif {[string match {[ab]/} $f} {
					lset fpath $n [string range $f 1 end]
				} { break }
			}

			# Can only acccept BOTH if ABOVE loop properly processed EACH
			if {!$n} {
				set finfo(lbl,[incr cnt]) [shortNm [set finfo(pth,$cnt) \
										  [lindex $fpath 1][lindex $fpath 2]]]
				set finfo(lbl,[incr cnt]) [shortNm [set finfo(pth,$cnt) \
										  [lindex $fpath 3][lindex $fpath 2]]]
			}
		}
		{diff *} {
		# GNU Diff -r output syntax (of NON-BINARY files): WITHOUT '-q' option:
		#	(data parrots the ORIGINAL $cmd except w/ SPECIFIC filenames)
		# N.B> side-benefit: Encountered BINARY files will NOT be picked-up
		#	because Diff CHANGES their wording (to one we DONT look for!)
		#	Thus suppress POSSIBLE "failure" report (based off the Diff Rcode)
			if {$MSG!={}} {set MSG {}}

			# N.B> Stripping out the filenames here requires a Regexp that KNOWS
			#	how many OTHER options were present on the command GENERATING
			#	the output lines being parsed now - (see earlier for its defn)
			set fpath [regexp -inline -- $xtractFNre $line]

			set finfo(lbl,[incr cnt]) [shortNm [set finfo(pth,$cnt) \
										  [lindex $fpath 1][lindex $fpath 2]]]
			set finfo(lbl,[incr cnt]) [shortNm [set finfo(pth,$cnt) \
										  [lindex $fpath 3][lindex $fpath 2]]]
		}
		default { continue }
		}
	}

	# PUSH-return errors (and signal their existence by returning ODD count)
	if {[string length "$MSG$msg"]} {
		if {$MSG!={}} { uplevel 1 set MSG "{$MSG}" }
		if {$msg!={}} { uplevel 1 set msg "{$msg}" }
		incr cnt
	}
	return $cnt
}

###############################################################################
# Request git to supply relevant target argument(s)
###############################################################################
proc inquire-git {revs} {
	global finfo

	set MSG [set msg {}]
	# Git diff requires 0-2 commit-ish "somethings" (hash, HEAD, etc...)
	#
	# As such, we expect those args to come thru as 'revs'; 'pths' would only
	# be useful to LIMIT the list being constructed (if we allowed them).
	#	Git differs from most SCMs in that it has an intermediate "pocket"
	# (called the 'index', or 'stage') BETWEEN the working copy (WC) and a
	# bona-fide "commit" (aka revision). Therefore while the nominal mapping
	# is:
	#	  'revs':
	#		0	= HEAD -> WC
	#		1	= rev -> WC
	#		2	= revA -> revB
	# use of a BLANK rev ("	 ") denotes the Index. Everything else SHOULD be
	# handled by "git rev-parse" (tags/hashes/branches/expressions/etc.)
	#
	#	However, WE are responsible for mapping the BLANK rev to the --staged
	# keyword required to make "git diff" actually access the Index.
	set cmit(2) [set rev(2) ""]
	if {$revs == 0} {
		# Sets up		HEAD	 ->	  WC
		set cmit(1) [set rev(1) "HEAD"]
	} elseif {$revs <= 2} {
		# Sets up	 (R1 or Index)	->	(WC or Index or R2)
		if {"" == [string trim [set cmit(1) [set rev(1) $finfo(rev,1)]]]} {
			set cmit(1) "--staged"
		}
		if {$revs == 2} {
			# Sets up	 R1	  ->   R2	   (but just NOT Index -> Index)!!!
			if {"" == [string trim [set cmit(2) [set rev(2) $finfo(rev,2)]]]} {
				if {"--staged" != $cmit(1)} {set cmit(2) "--staged"} {
					set MSG "BOTH revisions cannot specify the Git Index"
					return 1; # (Would've resulted in Index -> WC)
				}
			}
		}
	}

	# NORMALLY we would only extract the first pairing and simply RECORD
	# the others for later processing...but Git is a local-access SCM, thus
	# latency SHOULD NOT be an issue - just go DO IT ALL right now...
	#
	# Ask Git which files ACTUALLY differ between the given endpoint(s)
	#	(but limit it to those files seemingly modified - NO add/del)
	set cmd "git diff --diff-filter=M --name-only $cmit(1) $cmit(2)"
	lassign [run-command $cmd] gOUT gERR gRC
	if {$gRC != 0 || $gOUT == ""} {
		if {$gRC == 0} {
			set msg "Git Diff claims NO diffs using args: $cmit(1) $cmit(2)"
		} else {set MSG "Git Diff FAILED:\n$gERR"}
		set gitRC 1
	} { set gitRC 0 }

	set git_root [exec git rev-parse --show-toplevel]
	set cnt 0
	foreach file [split $gOUT "\n"] {
		# Ordinarily, 2-Revs would mean no possible WC interaction ...
		#	But if either referred to the Index, then we need to check it
		#
		# Look for an "unmerged" situation (only shows up in the Index)
		# Git-assigned tag #s are:	1:ancestor	2:ours	3:theirs
		#	(N.B> but unknown if they will extract here in that order)
		if {($revs < 2 || $cmit(1) == "--staged" || $cmit(2) == "--staged")	 \
		&& (1< [llength [set xx [split [exec git ls-files -s $file] "\n"]]])} {

			foreach i {3 2 1} {
				# Process each given item (in theirs/ours/ancestor order)
				foreach Gtag $xx { if {$i == [lindex $Gtag end-1]} {break} }

				# Label the pieces and ...
				if {$i > 1} {
					set finfo(pth,[incr cnt]) [set f [tmpfile "git_$i"]]
					set finfo(tmp,$cnt) ""
					set finfo(lbl,$cnt) "[shortNm $file] (GIT Cflct-"
					if {$i == 2} { append finfo(lbl,$cnt) "ours)" } \
					else		 { append finfo(lbl,$cnt) "theirs)" }
				} else {
					set finfo(apth,[expr {$cnt/2}]) [set f [tmpfile "tg_$i"]]
					set finfo(albl,[expr {$cnt/2}]) "Ancestor (GIT Cflct)"
					set finfo(atmp,[expr {$cnt/2}]) ""
				}
				# ... grab the file content using its SHA1 id
				set cmd "git cat-file blob [lindex $Gtag end-2]"
				lassign [run-command $cmd $f] na gERR gRC

				if {!$gRC} { continue }
				# BUT - erase it ALL if ANY of it fails
				set MSG "Git couldn't extract Conflict item($i): $file\n$gERR"
				set gitRC 1	 ; # Let caller know we didn't get EVERYTHING
				if {$i == 1} {array unset finfo "a*,[expr {$cnt/2}]"}
				if {$i <= 2} {array unset finfo "\[ptl]*,$cnt"}
				if {$i <  3} {incr cnt -1}
				if {$i <= 3} {array unset finfo "\[ptl]*,$cnt"; incr cnt -1}
			}
			continue
		}

		# Otherwise its (supposedly) a plain old difference
		foreach i {1 2} {
			incr cnt
			if {$rev($i) != ""} {
				if {" " == [string index "$rev($i)" 0]} {
					set finfo(lbl,$cnt) "[shortNm $file] (Git$cmit($i))"
				} { set finfo(lbl,$cnt) "[shortNm $file] (Git $rev($i))"}

				# Git is a "local-machine" access method (no latency) so doing
				# them ALL right now should not be a burden.
				#	If that proves wrong, THIS is where to fix it.
				set finfo(pth,$cnt) [tmpfile "tkd__[file tail $file]"]
				set finfo(tmp,$cnt) ""
				set cmd "git show $rev($i):$file"

				lassign [run-command $cmd $finfo(pth,$cnt)] na gitERR gRC
				if {$gRC} {
					if [string match "*exists on disk*" $gitERR] {
						#	(the file simply is not *from* the requested 'rev')
						# Maybe it is an uncommitted (yet staged) file ?
						#	Action: do nothing, let the tmp file remain empty.
						# This will end up as looking like 1 big 'add' or 'del'
						# depending on which rev (1 or 2) could not find it.

					} else {
						# Instead, we just let it fall into this catchall,
						# and ensure that the PAIR OF FILES gets skipped...
						# NOT just the one that failed (tmpfils remain unused).
						Dbg "FAILED: 'git show $rev($i):$file':\n$gitERR"
						if {$i == 1} {incr cnt -1; break} {incr cnt -2}
					}
					set gitRC 1
				}
			} else {
				# Just point at the REAL 'working copy' file (allows editting)
				set finfo(lbl,$cnt) "[shortNm $file] (Git--WC)"
				set finfo(pth,$cnt) $git_root/$file
			}
		}
	}

	if {$gitRC} {
		if {$MSG!={}} { uplevel 1 set MSG "{$MSG}" }
		if {$msg!={}} { uplevel 1 set msg "{$msg}" }
		incr cnt
	}
	return $cnt
}

###############################################################################
# Request svn to supply relevant target argument(s)
###############################################################################
proc inquire-svn {revs} {
	global finfo

	set MSG [set msg {}]
	# 'svn diff --summarize' tells us WHAT changed across a range of revisions
	#
	# rev  is what we will tell svn cat to access
	# cmit is how we express the range to 'svn diff'
	set cmit(2) [set rev(2) ""]

	if {$revs == 0} {
		# Sets up		BASE	 ->	  WC
		set cmit(1) [set rev(1) "BASE"]
	} elseif {$revs <= 2} {
		# Sets up	 R1	  ->   (WC or R2)
		set cmit(1) [set rev(1) $finfo(rev,1)]
		if {$revs == 2} {
			# Finish seting up	  R1   ->	R2
			set cmit(2) ":[set rev(2) $finfo(rev,2)]"
		}
	}

	# Ask Svn which items got committed between the given endpoint(s)
	#	do we need/want "--depth files" ???
	# N.B> this might get messy with URL/PEG/date notations!!!
	set cmd "svn diff --summarize -r $cmit(1)$cmit(2)"
	lassign [run-command $cmd] svnOUT svnERR svnRC
	if {$svnRC || $svnOUT == ""} {
		if {$svnRC == 0} {
			set msg "Svn diff claims NO diffs using rev: $cmit(1)$cmit(2)"
		} else {set MSG "Svn diff FAILED:\n$svnERR"}
		set svnRC 1
	}

	# Expected output form should look like lines of:
	#			"flgs	   filename"
	# (indices)	 0-------7 8--------->
	#
	#  where flgs can be:
	#	   D	-deleted
	#	   A	-added
	#	   M	-modified
	#	   xM	-(2nd M) properties modified
	# Note, "svn diff --summarize" unfortunately reports a CONFLICTED file
	# as 'M' as well, so we need to analyze a bit further
	# (because diffing of the embedded 'markers' is not very usefull)

	set cnt 0
	foreach ln [split $svnOUT "\n"] {
		if {[lindex $ln 0] eq "M"} {
			set file [string range $ln 8 end]

			# If *is* a CONFLICTed file, split it up and store the finfo data
			if {$revs < 2 && [string match "C*" [exec svn status -q $file]]} {
				# SVN gives us a couple ways to go:
				#	We COULD split-up the conflicted file, or simply GRAB the 3
				#	files that it stores as "extra" files for us - we will
				#	try the latter (provided all 3 exist), else...
				if {[file exists $file.mine] \
				&&	2 == [llength [set Flist [glob -path $file .r*]]]} {
					# So we have 3 files (2 w/distinct Revs, the EARLIER
					# of which IS the ancestor) - assign them accordingly
					#	First, parse out the two Rev values in the filenames
					set x [string length $file]
					set r0 [string replace [lindex $Flist 0]  0 $x+1 {}]
					set r1 [string replace [lindex $Flist 1]  0 $x+1 {}]

					#	Then attach them ALL into finfo (as Edittable)
					foreach x "$file.r[max $r0 $r1] $file.mine" {
						set finfo(pth,[incr cnt]) "$x"
						set finfo(lbl,$cnt) "[shortNm $x] (SVN Cflct-"
						if {$cnt & 1} { append finfo(lbl,$cnt) "theirs)" } \
						else		  { append finfo(lbl,$cnt)	 "mine)" }
					}
					set finfo(apth,[expr $cnt/2]) $file.r[min $r0 $r1]
					set finfo(albl,[expr $cnt/2]) "[shortNm $x] (SVN Ancestor)"

				} else {set cnt [split-conflictfile $file $cnt SVN]}
				continue
			}

			# otherwise its just plain old difference
			foreach i {1 2} {
				incr cnt
				if {"" != $rev($i)} {
					if {[get-file-rev $file $cnt $rev($i) SVN]} {
						if {$i == 1} {incr cnt -1; break} {incr cnt -2}
						set svnRC 1 ;# remember to PUSH msgs up another level
					}
				} else {
					# Just point at REAL 'working copy' files (allows editting)
					set finfo(lbl,$cnt) "[shortNm $file] (SVN--WC)"
					set finfo(pth,$cnt) $file
				}
			}
		}
	}

	if {$svnRC} {
		if {$MSG!={}} { uplevel 1 set MSG "{$MSG}" }
		if {$msg!={}} { uplevel 1 set msg "{$msg}" }
		incr cnt
	}
	return $cnt
}

###############################################################################
# Request cvs to supply relevant target argument(s)
###############################################################################
proc inquire-cvs {revs} {
	global finfo

	set MSG [set msg {}]
	# 'cvs diff --brief' tells us what changed
	#
	# rev is what we will tell cvs update -p to access
	# cmit is how we express the range to 'cvs diff'
	set cmit(2) [set rev(2) ""]

	if {$revs == 0} {
		# Sets up		BASE	 ->	  WC
		set cmit(1) [set rev(1) "BASE"]
	} elseif {$revs <= 2} {
		# Sets up	 R1	  ->   (WC or R2)
		set cmit(1) [set rev(1) $finfo(rev,1)]
		if {$revs == 2} {
			# Finish seting up	  R1   ->	R2
			set cmit(2) " -r [set rev(2) $finfo(rev,2)]"
		}
	}

	# Ask CVS what changed between the given endpoint(s)
	set outfile [tmpfile "cvsout" 1]
	set cmd "cvs diff -l --brief -r $cmit(1)$cmit(2)"
	lassign [run-command $cmd $outfile] na cvsERR cvsRC
	# cvsRC can be non-zero in many cases,
	#	eg.: if a file doesn't have one of the revs.
	#	Thus it isn't very meaningful or helpfull here; however,
	#	cvsERR should at least contain "cvs diff: Diffing ." regardless.
	#	Yet in the empty case, cvsRC is then zero (rather confusing).
	# Due in part to these issues (and what TCL presumes about errors) we were
	#	thus forced to place the cmd output into a file, to AVOID Tcl replacing
	#	it (in cvsOUT) with its OWN error msgs.
	# SO - we rewrite cvsOUT FROM that file to SEE the case when it's empty,
	# and we will deduce Success (or not) out of the messaging provided
#### MCP
#	set fp [open $outfile r]
	set fp [ gzopen $outfile ]
#### MCP
	set cvsOUT [read $fp]
	close $fp
	file delete $outfile
	set cvsRC 0
	if {[string match {*Diffing*} $cvsERR] } {
		if {$cvsOUT == ""} {
			set msg "CVS diff claims NO diffs using -r $cmit(1)$cmit(2)"
			set cvsRC 1
		}
	}

	# Expected output form will look like:
	# Index: File2.txt
	# ===================================================================
	# RCS file: /home/userid/path-into-repository/File2.txt,v
	# retrieving revision 1.5
	# diff --brief -r1.5 File2.txt
	# Files /var/tmp/cvsdBUe0v and File2.txt differ
	# cvs diff: File3.txt was removed, no comparison available
	# cvs diff: FileAdd.txt is a new entry, no comparison available
	# Index: Ftrunk.txt
	# ===================================================================
	# RCS file: /home/userid/path-into-repository/Ftrunk.txt,v
	# retrieving revision 1.5
	# diff --brief -r1.5 Ftrunk.txt
	# Files /var/tmp/cvs3Wrp6F and Ftrunk.txt differ

	# Note, cvs diff --brief doesn't report a conflicted file differently, and
	# cvs update -p will throw an error (unlike svn cat -p) making identifying
	# such files imperative - we also need its Revision (to locate an ancestor)
	# - UNLESS 2 Revs were provided (as that precludes using ANY WC files)

	set cnt 0
	foreach ln [split $cvsOUT "\n"] {
		if {[string match "Index: *" $ln]} {
			# Grab the filename ...
			set fn [lindex $ln 1]
			set Cflct 0 ;# ... then check for a CONFLICTed file (presumed: NO)
			if {$revs < 2} {
			   # Sadly, CVS does not provide RELATIVE revision references
			   # (cant ask for 'parent' Rev of a file) so are forced to ask for
			   # a 'cvs status' to both check for CONFLICT and GET its revision
			   # THEN simulate a 'parent' conversion ourselves (subtract 0.1)
			   foreach ln [split [exec cvs status $fn] "\n"] {
				  if {!$Cflct && [string match "*Unresolved Conflict" $ln]} {
					 set Cflct 1 } \
				  elseif {$Cflct && [string match "*Working rev*" $ln]} {
					 # We want the last digits (Cf) of its revision value (Cr)
					 set Cflct [lindex [set Cr [split [lindex $ln 2] "."]] end]
					 lset Cr {end} [incr Cflct -1] ;# Compute PARENT Revision!
					 break
				  }
			   }
			}

			# If *is* a CONFLICTed file, split it up and store as finfo data
			#	N.B> "split..." will INCREMENT cnt (which is why we pass it in)
			if {$Cflct} {
				set cnt [split-conflictfile $fn $cnt CVS]

				# OK - When CVS conflicts a file, it ALSO plants the prior
				# data as a hidden file in the WC - THAT is *ALMOST* our
				# ancestor - actually its the ancestor PLUS the users mods
				# (what existed just BEFORE the "update" ran). We COULD go
				# get the REAL ancestor, but this may be good enough ...
				# Grab it if it exists
				if {[file exists [set Afn ".#$fn.[join $Cr "."]"]]} {
					set finfo(apth,[expr {$cnt / 2}]) "$Afn"
					set finfo(albl,[expr {$cnt / 2}]) "[shortNm $Afn] (CVS Cflct)"
				}
				#	(some CVS admins MAY have set up processes to delete it
				#	after a few days wait time - perhaps THAT is when we
				#	should try extracting the REAL one?)
				continue
			}

			# otherwise its a plain old difference
			foreach i {1 2} {
				incr cnt
				if {"" != $rev($i)} {
					if {[get-file-rev $fn $cnt $rev($i) CVS]} {
						if {$i == 1} {incr cnt -1; break} {incr cnt -2}
						set cvsRC 1
					}
				} else {
					# Just point at REAL 'working copy' files (allows editing)
					set finfo(lbl,$cnt) "[shortNm $fn] (CVS--WC)"
					set finfo(pth,$cnt) $fn
				}
			}
		}
	}

	if {$cvsRC} {
		if {$MSG!={}} { uplevel 1 set MSG "{$MSG}" }
		if {$msg!={}} { uplevel 1 set msg "{$msg}" }
		incr cnt
	}
	return $cnt
}

###############################################################################
# Set up the display
###############################################################################
proc create-display {} {
	global g w pref opts tmpopts

	# these are the five major areas of the GUI:
	# menubar - the menubar (duh)
	# toolbar - the toolbar (duh, again)
	# client  - the area with the text widgets and the graphical map
	# status  - a bottom status line
	# merge	  - a separate window monitoring the results of all merge actions

	# 'identify' major frames/windows and store them in a global array
	#
	#	Status window MAY have been pre-built due to excessive network latency
	# If so, re-hide it until we can buildout the remainder of the display ...
	# Otherwise its ALREADY hidden, just build it along with everything else
	if {[set prebuilt [winfo exists .status]]} {
		wm withdraw . } { set w(status) .status }
	set w(client)  .client
	set w(menubar) .menubar
	set w(toolbar) .toolbar
	set w(popupMenu) .popupMenu
	set w(merge) .merge

	# 'identify' other windows that conditionally MAY exist later...
	set w(srch)		.srch
	set w(prefs)	.pref
	set w(scDialog) .scDialog
	set w(mFdiag)	.mFdiag

	# now, simply build all the REQUIRED pieces
	build-toolbar
	build-client
	build-menus
	if {!$prebuilt} { build-status }
	build-merge

#### MCP
#	frame .separator1 -height 2 -borderwidth 2 -relief groove
#	frame .separator2 -height 2 -borderwidth 2 -relief groove
	frame .separator1 -height 2
	frame .separator2 -height 2
#### MCP

	# Create a static list of Text widget actions that, when access'ing, will
	# NOT alter the display height nor count-of all text lines (a plot speedup)
	#	N.B> Its UNDOCUMENTED how this list gets searched, except that each is
	#		a 'string'. Investigation (in TCL srcCode) indicates linear
	#		first-found with length checks on each prior to byte compares.
	#
	#	So BEST order is: HIGHEST LIKELIHOOD EARLIEST; but then ?abbreviations?
	# OUR assessment of order thus presumes fullwords outrank minimal abbrev,
	# which outranks other LEGAL abbrev, but ARRANGED as merged GROUPS based
	# on likelihood of repetitive USAGE in this tool. CONVENIENTLY(?) OUR use
	# of abbreviations mostly falls on words NOT BELONGING in this list anyway.
	# Statistical measurement would be best; perhaps we'll rig one ... someday
	set w(benign) { dlineinfo xview count index mark bbox cget get\
					dump search compare debug peer sync syncpending }

	#	IN ADDITION, we need a 2nd static list: a TxtWidget subcmd-SYNONYM map
	# supporting our "Read Only" strategy (see 'textROfcn' for how this works)
	set w(ROsynonym) {INSERT ins   DELETE del	REPLACE rep	  SEE see}

	# And LASTLY a datum that TRACKS when TK has finished computing lineHeights
	# so syncscroll "gets it right" when the data MAY have been questionable...
	#			normal STATE (& Default) is 3		(see table below)
	set w(SYNCnow) 3

	#	...WHICH IN TURN necessitates a rather CRYPTIC 'state-transition' table
	# to tell 'textROfcn' WHEN 'scrolling' (particularly syncscroll) is ALLOWED
	# by describing HOW 'w(SYNCnow)' should TRANSITION to depict the present
	# status of COMPLYING with and/or REMEMBERING a needed scroll request.
	#	States denoted as NEGATIVE mark WHEN to issue the SYNC-scroll !!
	#
	# EVERY transition records INTO w(SYNCnow) above which is a 3bit-field
	#				  (Rqstd, ReadyRight, ReadyLeft)
	#
	#	Each PAIR(L/R) of ROWs defines how to transit that specific widget
	# in receipt of each DENY/LEFT/RIGHT/RQST notification (in any order)
	# Table Format is:
	#	   ( t w o	 r o w s )	DENY scroll	 MsgGrp 0
	#	   ( o n e	 r o w	 )	LEFT  Ready	 MsgGrp 1
	#	   ( t w o	 r o w s )		unused: (2ndLeft & 1stRight 'Ready's)
	#	   ( o n e	 r o w	 )	RIGHT Ready	 MsgGrp 2
	#	   ( t w o	 r o w s )	RQST scroll	 MsgGrp 3
	#  (N.B> list FORMAT was DICTATED by the SPECIFIC indexing strategy!)
	set w(SYNCtbl) {
		{ { 0  0  2	 2	4  4  6	  0	 0	2  2  4	 4	6}\
		  { 0  1  0	 1	4  5  4	  0	 1	0  1  4	 5	4} }
		{ { 1  1  3	 3 -1  5  6	  1	 1	3  3  5	 5 -3}\
		  {} }
		{ {}\
		  { 2  3  2	 3 -2  5  6	  2	 3	2  3  6 -3	6} }
		{ { 4 -1  6 -3	4 -1 -3	  4 -1	6 -3  4 -1 -3}\
		  { 4  5 -2 -3	4 -3 -2	  4	 5 -2 -3  4 -3 -2} }  }
		#	 syncscroll OFF			  syncscroll  ON
		#		States		 (0 - 6)	 States

	# Now fit all the widgets together and MANAGE it...
	# Note this effectively "declares" the remaining 'pack'er cavity
	# to be just BELOW the client, but ABOVE the status bar.
	#	(EXACTLY where DbgUI will appear if/when sourced in !!)
	# DbgUI is a custom standalone bind-investigation debugging tool
	. configure -menu $w(menubar)
	pack $w(toolbar) -side top -fill x -expand n
	pack .separator1 -side top -fill x -expand n
	pack .separator2 -side top -fill x	  -expand n
	if {!$prebuilt} {
		pack $w(status) -side bottom -fill x -expand n
	}
	pack $w(client)	 -side top -fill both -expand y

	# APPLY the users preferences by calling the proc that gets invoked when
	# the user presses "Apply" from the preferences window. That proc uses a
	# global variable ("tmpopts") which would ordinarily have the NEW values
	# from the dialog.
	#	Since we haven't USED the dialog, populate this array DIRECTLY !
	#	N.B> 'prefapply' GENERALLY looks for CHANGES among 'tmpopts'/'opts'
	#		CAUSING update ACTIONS - precluded by making them *IDENTICAL*.
	# We simply need it to ENSURE these settings are what is "in effect" when
	# it comes to those values affecting "configurable elements"
	array set tmpopts [array get opts]
	prefapply

	# Make sure temporary files get deleted
	#bind . <Destroy> {del-tmp}

	# Next, arrange for line numbers to be redrawn when just about anything
	# happens to ANY of our text widgets programatically.
	#	   This runs much faster than you might think.
	# N.B> Note: traces go to the NON-wrapped widget (xxx'_') to avoid
	#		responding to "fake" subcmnds handled by the WRAPPER directly.
	trace add exec $w(LeftText)_  leave "plot-line-info Left"
	trace add exec $w(RightText)_ leave "plot-line-info Right"
	trace add exec $w(mergeText)_ leave "plot-merge-info"
	bind $w(LeftText)  <Configure> "list plot-line-info Left"
	bind $w(RightText) <Configure> "list plot-line-info Right"
	bind $w(mergeText) <Configure> "list plot-merge-info"

	# Lastly, we make any wheel scrolling over the Info windows work
	#	(even though they themselves dont ACTUALLY scroll - they repaint)
	# 'eval' simply eliminates vars from within the quoted bind-scripts
	#	(???- found no way to just FORWARD the event to the Text widget)
	foreach side {Left Right merge} {
		foreach evt {Button-4 Button-5 Shift-Button-4 Shift-Button-5} {
			eval bind $w(${side}Info) <$evt> \
			   "{event generate $w(${side}Text) <$evt> -when head}"
		}
		foreach evt {MouseWheel Shift-MouseWheel} {
			eval bind $w(${side}Info) <$evt> \
			   "{event generate $w(${side}Text) <$evt> -delta %D -when head}"
		}
	}
	# Watch for the user to toggle scrollbar syncing
	#	(we want to make sure they sync up immediately)
	trace add var opts(syncscroll) write toggleSyncScroll

	# Attach all remaining bindings (mostly User-assigned shortcut HOTkeys)
	# to NEARLY every widget we know !! (plus some 'client' specialty items)
	setBinds {*}[lrange [DFSobj Wdg $w(client)] 1 end] $w(merge) {.}

	# ...and ADVERTISE those HOTkeys into their pertinent MENU entries
	# N.B> DANGEROUS: 'glob-matching' doesnt support "ORed" filtering - so
	#		grab the "gen*, nav*, mrg*" prefs with some extra slop involved
	foreach key [array names pref {[gnm][ear][nvg]*}] {
		if {[info exists w(Accel,$key)]} {
			foreach {mnu idx} $w(Accel,$key) {
				$mnu entryconfigure $idx -accelerator "$opts($key)"
			}
		}
	}

	wm deiconify .
	focus -force $w(acTxWdg)
	update idletasks

	# Need this to make our L/R 'pane'-resize emulation behave logically
	#	(cant have it appealing to its Toplevel for MORE space)
	grid propagate $w(client) false

	# This may appear even stranger:
	#	Now that the main window has been built AND preferences applied, there
	#	is an EXCELLENT chance that its REQUESTED size has been curtailed by
	#	forcing a geometry modification onto it, keeping it from being screen
	#	clipped on its intial display.
	#		It is THOSE dimensions we want the newly built merge window to
	#	center above. So we will use an unusual cmd syntax into centerWindow to
	#	possibly override the main window dimensions, while ALLOWING the actual
	#	w(merge) size to be picked up.
	# But first - we need to let the merge window actually FINISH with its
	# <Configure>ation (or even WE wont get the correct size...)
	# Luckily, 'centerWindow' brackets its action BETWEEN 2 'update' calls...
	scan [winfo geometry .] "%dx%d" W H
	centerWindow $w(merge) "0 0 $W $H"
}

###############################################################################
# Odd recursive little procedure generates a LIST of tree-structured objects
#	(eg. widgetnames -OR- subdirectories) originating and including the SEED
# name and all DEEPER names found from it (N.B> w/Dirs subject to exclusion)
###############################################################################
proc DFSobj { Obj seed } {
	lappend them $seed
	switch $Obj {
	"Dir" { foreach seed [glob -n -d $seed -type d *] { if {![xclude $seed] } {
				lappend them {*}[DFSobj Dir $seed] } }
	}
	"Wdg" { foreach seed [winfo children $seed] {
				lappend them {*}[DFSobj Wdg $seed] }
	}}
	return $them
}

###############################################################################
# When the user changes the "sync scrollbars" option, a trace fires to sync
# the left and right viewports, but only WHEN they've just turned the option ON
###############################################################################
proc toggleSyncScroll {args} {
	global opts

	if {$opts(syncscroll)} { centerCDR }
}

###############################################################################
# show the popup menu, reconfiguring some entries based on where user clicked
#  (notably - over the MAP window becomes somewhat Left/Right ambiguous)
###############################################################################
proc show-popupMenu {X Y} {
	global g w

	set win [winfo containing $X $Y]
	if {$win != $w(mapCanvas)} {

		# Turn this back ON (as it MAY have been turned off last time)
		$w(popupMenu) entryconfigure "Edit*" -state normal

		# Ensure w(acTxWdg) is proper for USE by above entry
		if {[string match {.client.left*} $win]} {
			set w(mPopW) [set w(acTxWdg) $w(LeftText)]
		} else {
			set w(mPopW) [set w(acTxWdg) $w(RightText)]
		}
	} { set w(mPopW) $win
		# Turn this OFF when we are NOT over the Text (or other L/R) windows
		#	(Re: no way to know which SIDE it would apply to)
		$w(popupMenu) entryconfigure "Edit*" -state disabled
	}

	# Cant find what doesnt exist...
	$w(popupMenu) entryconfigure "Find Near*" -state \
									[expr {$g(count) ? "normal" : "disabled"}]

	# Only allow clipboard copy if the primary selection is ours to begin with
	# AND is still PRESENTLY selected (as opposed to being FORMERLY selected)
	if {[selection own] == "$win" && ![catch "$win index sel.first"]} {
		set selstatus "normal"} {set selstatus "disabled"}
	$w(popupMenu) entryconfigure "Copy Selection" -state $selstatus

	# We must GRAB where we ASKED the menu to popup - needed by "Find Nearest"
	# which ordinarily would utilize a mousePt to determine what line was meant
	#	(but which we CANT pass-thru via menu Ops - so we'll fake an "end-run")
	# N.B> (See the "moveNearest" menuitem for where these get re-introduced)
	tk_popup $w(popupMenu) [set w(mPopX) $X] [set w(mPopY) $Y]
}

###############################################################################
# Manipulates the list of filepairs of a multi-file diff
#	(functions even when only ONE pair exists - although its mostly pointless)
###############################################################################
proc multiFile {command args} {
	global g w opts finfo

	# Special 'global' - is logically PRIVATE to this routine
	# N.B> needed because we CANT tie the 'radiobutton' widgets DIRECTLY into
	#	finfo(fCurpair), or we LOSE the KNOWLEDGE if a PRIOR value existed
	#	(due to the widget SETTING its attached VAR BEFORE invoking its CMD)!
	# Think of this as allowing the cmd to VALIDATE it wasn't chosen TWICE.
	global mFactivE

	# Convenience internal names
	set cvs $w(mFdiag).cv
	set vsb $w(mFdiag).sb

	# Default operation is to NOT invoke a Diff (unless determined otherwise)
	set diffit 0
	switch -- $command {
		prev {
			# choose previous file
			if {$finfo(fCurpair) > 1} {
				set diffit [incr finfo(fCurpair) -1]
			}
		}
		next {
			# choose next file
			if {$finfo(fCurpair) < $finfo(fPairs)} {
				set diffit [incr finfo(fCurpair)]
			}
		}
		jump { lassign $args index
			# choose designated file (but disallow RE-selection)
			if {$finfo(fCurpair) != $index} {
				set diffit [set finfo(fCurpair) $index]
			}
		}
		mrkACK { lassign $args index
			multiFile threshld $opts(fLMmax)
			# Mark file listing as SUCCESSFULLY accessed
			if {$finfo(fPairs) <= $opts(fLMmax)} {
				# Remember menuslots (0-3) have "dialog/prev/next/separ"
				$w(multiFileMenu) entryconf [expr {$index + 3}]	   \
										  -activebackg {PaleGreen}
			} { $w(mFlist).b$index config -activebackg {PaleGreen} }
			set mFactivE $index
		}
		mrkNAK { lassign $args index
			multiFile threshld $opts(fLMmax)
			# Mark file listing as UN-SUCCESSFULLY accessed
			if {$finfo(fPairs) <= $opts(fLMmax)} {
				# Remember menuslots (0-3) have "dialog/prev/next/separ"
				$w(multiFileMenu) entryconf [expr {$index + 3}] \
										  -activebackg {Tomato}
			} { $w(mFlist).b$index config -activebackg {Tomato} }
			set mFactivE $index
		}
		empty { lassign $args which
			# Destroy given CATEGORY of entries (if they happen to exist)
			#	(presupposes menu ALWAYS has 'dialog, prev, next' as first 3)
			#	(and YES - it DOES remove the trailing menu separator !!)
			if {$which != "list" && [$w(multiFileMenu) index end] > 2} {
				$w(multiFileMenu) delete 3 end
			}
			if {$which != "menu" &&[winfo exists $w(mFdiag)]} {
				foreach wdg [winfo children $w(mFlist)] { destroy $wdg }
			}
		}
		reload {
			# Empty old entries out first, from either category (if any) ...
			# Reloading DESIRED category thereafter
			multiFile empty "both"
			multiFile load $finfo(fPairs)
		}
		adjsz { lassign $args cnt
			# Establish dialog filelist HEIGHT (to know what can scroll)
			# N.B> invoke as "after idle" so implied 'update' will have run
			set rgn [list 0 0 [winfo	width  $w(mFlist)] \
					   [set i [winfo reqheight $w(mFlist)]]]
			$cvs configure -scrollregion $rgn -yscrollincr [expr $i / $cnt] \
							-height [expr ($i/$cnt) * [min $cnt 5]]
			# (?? above 'height' works initially EXACTLY as desired - unclear
			#	  why it then STOPS (again as desired) after user manually
			#	  chooses to resize window (maybe a 'geometry' override?) ??)
		}
		scroll {lassign $args mvby
			# ONLY DO the requested scroll...
			#	IF	threshold indicates we ARE using the list right now
			#	AND scrollbar is ACTIVE (ie. there IS more to see)
			if {$finfo(fPairs) > $opts(fLMmax) && [winfo ismapped $vsb]} {
				$cvs yview scroll $mvby units
			}
		}
		load { lassign $args cnt
			#	N.B> Cant 'load' what isn't THERE - ensure dialog EXISTs
			#	in case we are ABOUT to populate it!
			if {![winfo exists $w(mFdiag)]
			&& $cnt > $opts(fLMmax)} { multiFile dialog 0 }

			# Append entries that exist NOW (caller told us how many that is)
			# into the TARGETTED location (menu or window) based on threshold
			set i 0
			while {[incr i] <= $cnt} {
				# N.B> see earlier note on global 'mFactivE' vs (fCurpair)
				if {$cnt <= $opts(fLMmax)} {
					if {$i==1} { $w(multiFileMenu) add separator }
					$w(multiFileMenu) add radiobutton -variable mFactivE  \
						 -value $i -label $finfo(lbl,[expr {$i * 2 - 1}]) \
						 -command [list multiFile jump $i]
				} { grid [radiobutton $w(mFlist).b$i  -variable mFactivE  \
						 -value $i -text  $finfo(lbl,[expr {$i * 2 - 1}]) \
						 -command [list multiFile jump $i]] -sticky "w"

					# Unfortunately ALSO must snag Mousewheel events sent
					# to the buttons (because they OVERLAID the canvas)
					if {$w(wSys) == "x11"} {
						bind $w(mFlist).b$i <Button-4> {multiFile scroll -1}
						bind $w(mFlist).b$i <Button-5> {multiFile scroll  1}
					} elseif {$w(wSys) == "aqua"} {
						bind $w(mFlist).b$i <Option-MouseWheel> \
								{multiFile scroll [expr {%D>=0 ? -5:5}]}
					}
					bind $w(mFlist).b$i <MouseWheel> \
						{multiFile scroll [expr {%D>=0 ? -1:1}]} ;# Up/Dwn
				}
			}

			# If content went INTO the list, schedule FINDING its extent...
			# Otherwise its in the menu - Update the Pullright menu LABEL.
			# Regardless, initiating Diff for this case: CALLERS responsibility
			if {$cnt > $opts(fLMmax)} {
				grid $cvs
				after idle multiFile adjsz $cnt
				$w(multiFileMenu) entryconf 0 -label "Choose File..."
				set finfo(fLfmt) 1
			} elseif {[winfo exists $w(mFdiag)]} {
				$w(multiFileMenu) entryconf 0 -label "Reconfig Threshold..."
				set finfo(fLfmt) 0
			}
		}
		threshld { lassign $args thresh
			# New opts(fLMmax) was set, but verify if it "makes a difference"
			if {($finfo(fLfmt)==0 && $finfo(fPairs) <= $thresh)
			||	($finfo(fLfmt)==1 && $finfo(fPairs) >  $thresh)} { return }

			# Populate EXISTING data into the desired target location, but
			# TRANSFER any present STATUS information along with it
			multiFile load [set cnt $finfo(fPairs)]
			set i 0
			while {[incr i] <= $cnt} {
				if {$cnt > $opts(fLMmax)} { set mrkACKNAK \
					[$w(multiFileMenu) entrycget [expr $i+3] -activebackg]
				} { set mrkACKNAK [$w(mFlist).b$i	 cget	 -activebackg] }
				if {$mrkACKNAK in {Tomato PaleGreen}} {
					if {$cnt > $opts(fLMmax)} {
						$w(mFlist).b$i		 config -activebackg $mrkACKNAK
					} { $w(multiFileMenu) entryconf [expr $i+3]	 \
													-activebackg $mrkACKNAK }
				}
			}
			if {$cnt > $opts(fLMmax)} {
				multiFile empty menu ; set finfo(fLfmt) 1
			} { multiFile empty list ; set finfo(fLfmt) 0 }
		}
		set { lassign $args Sfrac Efrac
			# WATCH for making the scrollbar itself VISIBLE, by MONITORING
			# the scrolling feedback values from the canvas-to-scrollbar
			$vsb set $Sfrac $Efrac
			#	(should REALLY only toggle when it makes a difference...but)
			if {$Sfrac>0.0 || $Efrac<1.0} { grid $vsb } { grid remove $vsb }
		}
		dialog { lassign $args showit
			# Reconfig and/or Choose from present list of available multiFiles
			if {![Dialog NONMODAL $w(mFdiag)]} {
				wm title $w(mFdiag) "$g(name) FileList"
				wm transient $w(mFdiag) .
				wm	 group	 $w(mFdiag) .

				# We don't want the window to be deleted, just hidden from view
				wm protocol $w(mFdiag) WM_DELETE_WINDOW \
													{Dialog dismiss $w(mFdiag)}

				# Threshold control area is always present
				#	('scale' wdg is allowed to stretch horiz as needed)
				set ctl [frame $w(mFdiag).ctl -bd 1 -relief solid]
				label $ctl.lbl -text "Prefer menu\nat/below:"
				scale  $ctl.menulim -orient horizontal -from 1 -to 25\
						-var opts(fLMmax) -tick 0 -command {multiFile threshld}
				button $ctl.dismiss -text "Dismiss" \
											-command "Dialog dismiss $w(mFdiag)"
				grid $ctl.lbl $ctl.menulim $ctl.dismiss -sticky ew
				grid columnconfig $ctl 1 -weight 1

				# Now construct a scrollable frame (for widgets of filenames)
				#	   (simply a frame logically INSIDE a canvas)
				#	N.B> DO NOT supply any dimensions to the INNER frame!!
				# But create and connect a vert scrollbar for that canvas
				canvas $cvs -height [expr [winfo reqheight $ctl] *3]  -bd 0 \
								-yscrollcom {multiFile set} -highlightthick 0
				scrollbar $vsb -orient vertical -command "$cvs yview" -bd 0 \
															-highlightthick 0

				# Put the widget-fillable innerframe (mFlist) WITHIN the canvas
				$cvs create window 0 0 -anchor nw -window \
										  [set w(mFlist) [frame $cvs.fl -bd 0]]

				# Put it all together - CTL goes in 1st row but spans 2 cols
				# while CVS + VSB make up second row,
				grid $ctl -columnspan 2 -sticky ew
				grid $cvs $vsb	 -row 1 -sticky nsew
				grid columnconfig $w(mFdiag) 0 -weight 1
				grid rowconfig	  $w(mFdiag) 1 -weight 1

				# Remove the list (initially)... loading content restores it
				# ...but THAT will depend on filecount and Threshold setting
				# and lastly tell canvas to scroll (+/- Y-only) on Wheel events
				grid remove $cvs $vsb
				if {$w(wSys) == "x11"} {
					bind $cvs <Button-4> {multiFile scroll -1} ;# Up
					bind $cvs <Button-5> {multiFile scroll	1} ;# Dwn
				} elseif {$w(wSys) == "aqua"} {
					bind $cvs <Option-MouseWheel> \
						{multiFile scroll [expr {%D>=0 ? -5:5}]} ;# Big Up/Dwn
				}
				bind $cvs <MouseWheel> \
						{multiFile scroll [expr {%D>=0 ? -1:1}]} ;# Up/Dwn
			}
			# Config
			set mFactivE $finfo(fCurpair)
			# Display it (if request was from the User).
			if {$showit} { Dialog show $w(mFdiag) }
		}
	}

	# Certain subcmds (chg of CURRENT file) require Diff to be run
	if {$diffit} { do-diff ; update-display }
}

###############################################################################
# Resize the text windows relative to each other (ie. NET size chg = ZERO)
###############################################################################
proc pane-drag {win x} {

	set relX [expr $x - [winfo rootx $win]]
	set maxX [winfo width $win]
	set frac [expr int((double($relX) / $maxX) * 100)]
	# LIMIT exchange of traded window real estate to the MIDDLE 90%
	set L [set frac [min 95 [max $frac 5]]]
	set R [expr 100 - $frac]
	grid columnconfigure $win 0 -weight $L
	grid columnconfigure $win 2 -weight $R
	#Dbg " new L/R ratio: $L $R"
}

###############################################################################
# build the main client display (text widgets, scrollbars, that sort of fluff)
###############################################################################
proc build-client {} {
	global w opts tk_patchLevel

	frame $w(client) -bd 2 -relief flat

	# set up global variables to reference the widgets, so
	# we don't have to use hardcoded widget paths elsewhere
	# in the code
	#
	# Text	- holds the text of the file
	# Info	- holds meta-data ABOUT 'Text': LineNums, Changebars, etc
	# VSB	- vertical scrollbar
	# HSB	- horizontal scrollbar
	# Label - label to hold the name of the file
	set w(LeftText)	 $w(client).left.text
	set w(LeftInfo)	 $w(client).left.info
	set w(LeftVSB)	 $w(client).left.vsb
	set w(LeftHSB)	 $w(client).left.hsb
	set w(LeftLabel) $w(client).leftlabel

	set w(AncfLabel) $w(client).ancFile

	set w(RightText)  $w(client).right.text
	set w(RightInfo)  $w(client).right.info
	set w(RightVSB)	  $w(client).right.vsb
	set w(RightHSB)	  $w(client).right.hsb
	set w(RightLabel) $w(client).rightlabel

	set w(BottomText) $w(client).bottomtext

	set w(map)		 $w(client).map
	set w(mapCanvas) $w(map).canvas

	# May eventually need this for a 3way diff (see 'alignDecor' for details)
	button $w(AncfLabel) -bd 0 -image ancfImg -command {
		simpleEd open $finfo(apth,$finfo(fCurpair)) ro \
						   fg [$w(mergeText) cget -fg] \
						   bg [$w(mergeText) cget -bg] \
				   title "$finfo(albl,$finfo(fCurpair)) - Ancestor" }

	# We create several widgets twice; once for Left and again for Right
	#
	# First up, the labels...
	#	N.B> DO NOT set a WIDTH size on these, we simply want them to
	#		use whatever space is given to them by virtue of the TxtWdgs
	#		they will be positioned above (ie. in the same grid-mgr COLUMN).
	#	THEY should NOT BE the 'pacing item' for determining the window width!
	Dbg " Assigning labels to headers"
	label $w(LeftLabel)	 -bd 1 -relief flat -textvariable finfo(lbl,Left)
	label $w(RightLabel) -bd 1 -relief flat -textvariable finfo(lbl,Right)

	# These hold the following text widgets and the scrollbars. The reason
	# for the frame is purely for aesthetics. It just looks nicer, IMHO,
	# to "embed" the scrollbars within the text widget
	#	(these won't need to be global)
	set leftFrame  [frame $w(client).left  -bd 1 -relief sunken]
	set rightFrame [frame $w(client).right -bd 1 -relief sunken]


	scrollbar $w(LeftHSB) -borderwidth 1 -orient horizontal -command \
	  [list $w(LeftText) xview]

	scrollbar $w(RightHSB) -borderwidth 1 -orient horizontal -command \
	  [list $w(RightText) xview]

	scrollbar $w(LeftVSB) -borderwidth 1 -orient vertical -command \
	  [list $w(LeftText) yview]

	scrollbar $w(RightVSB) -borderwidth 1 -orient vertical -command \
	  [list $w(RightText) yview]

	# By default, CREATE these at the USERS requested size...
	#	However, will likely be shrunk to keep the INITIAL display onScreen
	if {2 > [scan $opts(geometry) "%dx%d" width height]} {
		popmsg "Invalid geometry setting:\n$opts(geometry)\n	  \
		Reverting to 80x30" "Improper syntax..."
		lassign {80 30} width height
	}
	text $w(LeftText) -padx 0 -wrap none -width $width -height $height \
				-bd 0 -yscrollcommand "vscroll Left" \
					  -xscrollcommand "hscroll-sync 1"

	text $w(RightText) -padx 0 -wrap none -width $width -height $height \
				-bd 0 -yscrollcommand "vscroll Right" \
					  -xscrollcommand "hscroll-sync 2"

	# Technically, we lack the data to configure this properly until both
	# primary files have been loaded into the above text widgets. But we
	# need them right NOW for constructing the overall window layout.
	# Remaining options happen later via "prefapply" and "cfg-line-info"
	canvas $w(LeftInfo)	 -highlightthickness 0
	canvas $w(RightInfo) -highlightthickness 0

	# this widget is the two line display showing the current line, so
	# one can compare character by character if necessary.
	#	N.B> Best when font used is Constant-Width!
	text $w(BottomText) -wrap none -borderwidth 1 -height 2 -width 0

	# this is BASICALLY how we highlight those bytes that are different...
	# the bottom window (lineview) uses a tag to highlight mismatches,
	# so we need to configure that tag as requested
	$w(BottomText) tag configure diff {*}$opts(bytetag)

	# Set up text tags for the 'current diff' (the one chosen by the 'next'
	# and 'prev' buttons) .vs. any ol' diff region.	 All diff regions are
	# given the 'diff' tag initially...
	#	As 'next' and 'prev' are  pressed, to scroll through the differences,
	# one particular diff region is always chosen as the 'current diff', and
	# is set off from the others via the 'curr' tag -- in particular, so its
	# obvious which diff regions in the left and right-hand text widgets align.
	#	N.B> THIS DEFINES THE TAG PRECEDENCE ORDER
	#		Any further downstream code should only ever RE-cfg IN THIS ORDER!
	#		Introspecting to obtain this (ordered) list is the PREFERRED method
	foreach widget [list $w(LeftText) $w(RightText)] {
		$widget configure {*}$opts(textopt)
		foreach t {diff curr del ins chg overlap inline} {
			$widget tag configure ${t}tag {*}$opts(${t}tag)
		}
		$widget tag raise sel ;# Keep this on top
	}

	# build the map...
	#	we want the map to basically resemble a scrollbar, so we'll
	#	steal some information from one of the scrollbars we just created...
	set color [$w(LeftVSB) cget -troughcolor]
	set ht [$w(LeftVSB) cget -highlightthickness]
	set cwidth [expr {[winfo reqwidth $w(LeftVSB)] - ($ht * 2)}]

	# At the widget level, its just a frame holding a canvas...
	frame $w(map) -bd 1 -relief sunken -takefocus 0 -highlightthickness 0

	canvas $w(mapCanvas) -width [expr {$cwidth + 1}] \
	  -yscrollcommand map-resize -background $color -borderwidth 0 \
	  -relief sunken -highlightthickness 0

	# ... but for the REAL map,	 we want an IMAGE we can draw into INSERTED
	# in that canvas, along with  (dummy) linework for a 'scrollbar thumb'
	#	N.B> coords (number of, nor values) dont matter, as 'map-move-thumb'
	#	REWRITES them as a properly positioned, hollow, 3D rectangle (later on)
	set w(mapImg) [image create photo]

	$w(mapCanvas) create image 1 1 -image $w(mapImg) -anchor nw
	$w(mapCanvas) create line 0 0 0 0 -width 1 -tags thumbUL -fill white
	$w(mapCanvas) create line 1 1 1 1 -width 1 -tags thumbLR -fill black
	pack $w(mapCanvas) -side top -fill both -expand y

	# Complete the scrollbar simulation with bindings for interaction
	bind $w(mapCanvas) <ButtonPress-1>	 {handleMapEvent B1-Press	%y}
	bind $w(mapCanvas) <Button1-Motion>	 {handleMapEvent B1-Motion	%y}
	bind $w(mapCanvas) <ButtonRelease-1> {handleMapEvent B1-Release %y}
	bind $w(mapCanvas) <ButtonPress-2>	 {handleMapEvent B2-Press	%y}
	bind $w(mapCanvas) <ButtonRelease-2> {handleMapEvent B2-Release %y}

	# Again, wheel scrolling over the MAP window SHOULD also work
	# - but can only target the THEN w(acTxWdg) text widget
	foreach evt {Button-4 Button-5 Shift-Button-4 Shift-Button-5} {
		eval bind $w(mapCanvas) <$evt> \
		   "{event generate \$w(acTxWdg) <$evt>			  -when head}"
	}
	foreach evt {MouseWheel Shift-MouseWheel} {
		eval bind $w(mapCanvas) <$evt> \
		   "{event generate \$w(acTxWdg) <$evt> -delta %D -when head}"
	}

	# this is a grip for resizing the sides relative to each other.
	button $w(client).grip -borderwidth 3 -relief raised \
	  -cursor sb_h_double_arrow -image resize -takefocus 0
	bind $w(client).grip <B1-Motion> {pane-drag $w(client) %X}

	# use grid to manage the widgets in the left side frame
	grid $w(LeftVSB)  -row 0 -column 0 -sticky ns
	grid $w(LeftInfo) -row 0 -column 1 -sticky nsew
	grid $w(LeftText) -row 0 -column 2 -sticky nsew
	grid $w(LeftHSB)  -row 1 -column 1 -sticky ew -columnspan 2

	grid rowconfigure $leftFrame 0 -weight 1
	grid rowconfigure $leftFrame 1 -weight 0

	grid columnconfigure $leftFrame 0 -weight 0
	grid columnconfigure $leftFrame 1 -weight 0
	grid columnconfigure $leftFrame 2 -weight 1

	# likewise for the right...
	grid $w(RightVSB)  -row 0 -column 4 -sticky ns
	grid $w(RightInfo) -row 0 -column 0 -sticky nsew
	grid $w(RightText) -row 0 -column 1 -sticky nsew
	grid $w(RightHSB)  -row 1 -column 0 -sticky ew -columnspan 2

	grid rowconfigure $rightFrame 0 -weight 1
	grid rowconfigure $rightFrame 1 -weight 0

	grid columnconfigure $rightFrame 0 -weight 0
	grid columnconfigure $rightFrame 1 -weight 1

	# use grid to manage the labels, frames and map. We're going to
	# toss in an extra row just for the benefit of our dummy frame.
	# the intent is that the dummy frame will match the height of
	# the horizontal scrollbars so the map stops at the right place...
	grid $w(LeftLabel)	 -row 0 -column 0 -sticky ew
	grid $w(RightLabel)	 -row 0 -column 2 -sticky ew
	grid $leftFrame		 -row 1 -column 0 -sticky nsew	-rowspan 2
	grid $w(map)		 -row 1 -column 1 -sticky ns
	grid $w(client).grip -row 2 -column 1
	grid $rightFrame	 -row 1 -column 2 -sticky nsew	-rowspan 2
	grid $w(BottomText)	 -row 3 -column 0 -sticky ew -columnspan 4

	grid rowconfigure $w(client) 0 -weight 0
	grid rowconfigure $w(client) 1 -weight 1
	grid rowconfigure $w(client) 2 -weight 0
	grid rowconfigure $w(client) 3 -weight 0

	grid columnconfigure $w(client) {0 2} -weight 100 -uniform a
	grid columnconfigure $w(client) 1 -weight 0

	# Cause the variable w(acTxWdg) to be whichever text widget EVER
	# receives the focus via a mouseclick...
	bind $w(LeftText)  <1> {set w(acTxWdg) $w(LeftText)}
	bind $w(RightText) <1> {set w(acTxWdg) $w(RightText)}
	# N.B> We DEPEND on w(acTxWdg) to ONLY specify ONE of the main Text
	#	   widgets, so PLANT a trace to ensure it ALWAYS STAYS that way
	trace add var w(acTxWdg) write { apply {{ary elem op} {
		upvar $ary gbl
		if {![string match {.client.[lr]*.text} $gbl($elem)]} {
			set gbl($elem) $gbl(PREV$elem)
		}
		set gbl(PREV$elem) $gbl($elem)
	}}}
	# and immediately initialize it ('Right' *IS* arbitrary, but valid)!
	set w(acTxWdg) $w(RightText)

	# Finally, we need to WRAP each of the major text procs ...
	#	(See explanation below this proc for WHY)
	rename $w(RightText)	$w(RightText)_
	proc   $w(RightText)  {cmd args} $::textROfcn
	rename $w(LeftText)		$w(LeftText)_
	proc   $w(LeftText)	  {cmd args}  $::textROfcn
	rename $w(BottomText)	$w(BottomText)_
	proc   $w(BottomText) {cmd args} $::textROfcn

	# ... while (JUST the L/R two) REQUIRE binds related to their POTENTIAL
	# to be scroll-synchronized (owing to an unfortunate, yet necessary, TK
	# processing-performance solution).
	#
	# While tracking was EXPOSED in TK 8.6.5 (via <<WidgetViewSync>>) we will
	# ATTEMPT to emulate it in prior versions (down to our package-require)
	#	N.B> Users TRANSITIONING to TK8.6.5 will do so transparently
	#
	# Besides binding the virtual event, we need to KNOW which TK is running,
	# PLUS a datum per widget (empty) ALL FOR exclusive use in a Pre-TK8.6.5
	# EMULATION designed to DRIVE that same binding!
	set w(TK865) [package vsatisfies $tk_patchLevel 8.6.5-]
	lassign {} w(SYNCbusy$w(LeftText))	w(SYNCbusy$w(RightText))
	bind $w(LeftText)  <<WidgetViewSync>> { %W SYNCnow %d }
	bind $w(RightText) <<WidgetViewSync>> { %W SYNCnow %d }
#		REPLACE above binds with below IF you want to SEE the state transitions
#		that RESULT from receipt of the events. Only uncomment ONE set !!
#	bind $w(LeftText)  <<WidgetViewSync>> {Dbg {\tSYNCnow{[%W SYNCnow %d]}} 1 {}}
#	bind $w(RightText) <<WidgetViewSync>> {Dbg {\tSYNCnow{[%W SYNCnow %d]}} 1 {}}
}

###############################################################################
# This Text-Wdg BODY code will	be used as an 'observer' WRAPPER for EITHER
# the FIRST (or in the case of the L/R widgets, ALL) of these reasons:
#
#	1. To implement a READONLY text widget (WITHOUT using "-state disabled")
#		- BLOCKS all attempts to modify content (via CONVENTIONAL subcmds)
#		+	BUT implements ALTERNATIVE synonyms for use by OUR OWN code
#
#	The REMAINING items are EXCLUSIVELY peculiar to the Primary L/R Widgets:
#	2. To watch insert-cursor repositions and update the line comparison window
#	3. To implement SAFE synchro-scrolling (AND when active) CDR autoselection
#	4. To detect and initiate calculating INLINE diff tagging (when VISIBLE)
#
# N.B> #3 needs temporary deferral if TK hasnt yet fully updated line heights
#	   yet; but #4 is PARTLY responsible for CAUSING the DELAY in handling #3
#	   AND (prior to TK8.6.5) there is no OFFICIAL way to quantify the delay!
#
# PRIOR to TK8.6.5, DETERMINING when TK is "busy" can only be approximated;
# SOLUTION:
#	Created 'fake' widget subcmd SYNCbusy (+ its triggering mechanism) to form
#	a 'POLL mechanism' that SIMULATES (as best it can) WHEN the TK8.6.5 virtual
#	event <<WidgetViewSync>> SHOULD have occurred. Consider this as a BACKPORT
#	of the TK8.6.5 approach which AUTOMATICALLY disengages if USING >=TK8.6.5
###############################################################################

set textROfcn {
	global g w opts

	set This "[lindex [info level 0] 0]"				;# OUR proc name
	if {![set ThisID [string match {*left*} $This]]} {	;#	(and matching ID)
		set ThisID 2 }
	set Othr [string map {right left left right} $This] ;# COMPANION proc name
	set Real "${This}_" ;# N.B> trailing underscore!!	 # SHADOWED proc name
	# N.B> (Not presently NEEDED, but "${Othr}_" WILL access OTHER REAL widget)
	set D -1 ;# Special Dbg "GROUP" force/enable/disable flag (1/0/-1 resp.)

	lassign $args a1 a2 a3 a4 ;# (to inspect upto first 4 args OF cmd)
	set result {}

	# Read-Only support:
	# Content modifications are DISALLOWED unless done with OUR "synonyms"...
	#	(nothing more than simple capitalizations of the 'proper' subcmd)
	# Intent is to prevent modifications from unsanctioned sources (eg. User)
	#	In simpler terms: your basic Read Only Text Widget
	#		('see' is included because it causes IMPLICIT scrolling, which
	#		 must block when it CO-EXISTS with actions that we ALWAYS block)
	# To MAP synonyms, a static list "w(ROsynonym)" was created ONCE, earlier.
	#
	# Beyond the 'Read-Only' synonyms, other 'FAKE' commands exist for special
	# purposes - yet, ALL such fakes are similarly CAPITALIZED to avoid
	# confusion with actual widget subcommands.
	#	N.B> HOWEVER - NO fake OR synonym is EVER permitted to be ABREVIATED
	switch -glob $cmd {
		ALLOW { if {$a1 == "see" && $a2 in {1 0}} {
				# Fictitious command "ALLOW"s widget to permit 1 'see' to WORK
					set w(see$Real) $a2} {
					set w(see$Real) 0}
				# "ALLOW' is SPECIFIED on key bindings that PRECEED Class bind
				# scripts having EMBEDDED "see" actions. BOTH bindings fire in
				# SEQUENCE, thus only the ones WE 'ALLOW'ed can execute "see"
		}
		see {	if {[info exists w(see$Real)] && $w(see$Real)} {
				# CONDITIONALLY permits operation (for certain Class Bindings
				#	notably those providing keyboard insert-point navigation)
					set w(see$Real) 0
					set result [$Real $cmd {*}$args]

				# ... else "see" IS BLOCKED (NO scrolling w/o our cooperation)
				# These generally occur as 'follow-on' actions attempting to
				# ensure visual feedback when TYPING (which *we* BLOCKED)!
				} { Dbg "$Real: ${cmd}(?): DENIED:	ReadOnly" }
			# ({sniff...} Perfect place for a C-language fall-thru case!)
		}
		ins* -
		del* -
		r* {   Dbg "$Real: $cmd: DENIED:  ReadOnly" ;#(<==abbrev 'r'eplace)
					# BLOCKs actions (NOTHING changes w/o our cooperation)
		########  This ENDS the essence of providing READONLY access. #########

		#	EVERYTHING BELOW is ALL 'special case' code peculiar to TkDiff
		# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
		}

		t* { if {!$w(TK865) && $opts(showinline2)} { ;# (<==abbrev 't'ag) \
			 EMULATES triggering virtual event when using an OLDER TK
				# Monitor (real) tag manipulation subcmnds
				switch -glob "$a1$a2" {
					a*inlinetag -
					re*inlinetag -
					del*vL* -
					con*vL* {
						# catches "add/rem inlinetag" -OR- "conf/del vL<hid>"
						#			?? WHY NO 'remove vL*' ??
						#	ANY of which THEORETICALLY may change text heights
						# TECHNICALLY others exist, but THESE matter most to US
						# and the BACKPORT need not(?) worry about the others
						#
						# *BUT* - dont FLOOD the event queue with these (they
						# tend to occur in flurries) - Only need the FIRST that
						# would actually MODIFY our current state
						if {$w(SYNCnow) & $ThisID} {
							event gen $This <<WidgetViewSync>> -data 0
						}
					}
				}
			}
			# Nonetheless, ALWAYS pass them directly to the REAL widget
			set result [$Real $cmd {*}$args]
		}
		SYNCnow {
			# The <<WidgetViewSync>> virtual event handler (ALL TK versions)
			#
			#	Again, a 'FAKE' subcommand, intended to TRACK scroll usage
			# until BOTH (or either) Text widgets stop "adjusting" themselves
			# so ALIGNMENT can be UNEQUIVOCALLY obtained.
			#
			# The Virtual Event should be BOUND as to simply PASS its value:
			#			bind $Wdg <<WidgetViewSync>> {%W SYNCnow %d}
			#
			# N.B> OLDER TK uses tag/SYNCbusy code to TRIGGER event creation
			#	   NEWER TK simply HAS the event builtin and sends by itself
			#		  TK8.6.5 protocol says we will see a %d=0 BEFORE %d=1
			#
			# Translate args into our state-table indexing strategy
			#	(N.B> we took a small liberty in that our emulation passes the
			#		COUNT of SYNCbusy attempts instead of "1" for THIS Dbg)
			Dbg {<<WVSync>> [expr {"$a1"=={} ? "RQST" : $a1}] Side($ThisID)} $D
			if {"$a1" == ""}		{ set a1 3 } elseif {$a1} {set a1 $ThisID}
			if {$opts(syncscroll)}	{ set a2 7 }			  {set a2 0 }
			# Then lookup the NEW state, executing a RQST'd scroll as needed
			if {[set w(SYNCnow) \
				[lindex $w(SYNCtbl) "$a1 ${ThisID}-1 $w(SYNCnow)+$a2"]] < 0} {
				### State Table ###	 Msg	 Row		 Column			YES

				set w(SYNCnow) [expr abs($w(SYNCnow))]; # Strip the RQST bit
				Dbg {\tSYNCnow FIRING as ($w(SYNCnow))} $D {}

				# FORCE the "alignment" scroll (at wherever we ARE just now)
				#	This is actually a SAFE scroll because we are satisfying a
				#	RQST that JUST THIS MOMENT showed as READY. Thus it WILL BE
				#	READY when being processed, and CANT raise a RQST itself.
				#	  - INCLUDING its possible syncscroll partner.
			 #	$This	 yview [$Real index @0,0]			  ;#  ORIG	way
			 #	$Othr	 YVIEW moveto [lindex [$Real yview] 0];#  ALT?	way
				${Othr}_ yview moveto [lindex [$Real yview] 0];# DIRECT way
			}

			# Return the PRESENT state value (in case anyone is watching)
			#	N.B> as this GENERALLY runs by a bind script that mayn't CARE,
			#		the only place it IS viewable is from that RQST requestor
			#		BUT-  alternate (commented-out) bind scripts ARE available
			return $w(SYNCnow)
		}
		SYNCbusy {
			# (DEFINED for PRE TK8.6.5) emulates WHEN to send <<W..V..Sync>>=1
			#	Polls widget for APPARENT quiescence of computations based on
			# LACK of change in "Wdg yview" results. Wants CONSECUTIVE match
			# results(3) to qualify. Its the CALLERS responsibility to AVOID
			# invoking when using TK 8.6.5 (or greater).
			#
			lassign "[$Real yview] $a2 $a3 $a4 0 0 0" F1 F2 a2 a3 a4
			incr a4 ;# (N.B> $a4 CAN be eliminated: only counts POLLs needed)
			if {$F1==$a2 && $F2==$a3} {
				if {[incr a1]==3} {
					event gen $This <<WidgetViewSync>> -data $a4 -when head
					set w($cmd$This) {}
				} { set w($cmd$This) [after 333 $This $cmd $a1 $F1 $F2 $a4]}
			} {		set w($cmd$This) [after 333 $This $cmd	1  $F1 $F2 $a4]}
			Dbg "SYNCbusy Wdg.mtchd.try($ThisID.$a1.$a4)\n	Prv\t$a2 $a3\n \
															 Cur\t$F1 $F2"	$D
		}

		YVIEW {
			# PREVENTs a recursion-firestorm when opts(syncscroll) is true.
			# Invoked FROM the COMPANION widget to PERFORM a "dual scroll"
			#	   (or anytime we only want THIS SIDE to take action)
			# N.B> BEHAVES like a synonym, but RETAINS its "fake" cmd name
			#		to let THIS widget still go thru visibility checks
				set result [$Real yview {*}$args]
		}

		default {	set cmd [string map $w(ROsynonym) $cmd]
				# Only RECOGNIZED synonyms or ACTUAL cmds are ever PERFORMED !
				# Synthetic ('FAKES') must be addressed ABOVE, not exec'd here
				set result [$Real $cmd {*}$args]
		}
	}

	# The rest of this code applies ONLY to the L/R main display windows:
	#	(N.B> A Text widget NAMING vulnerability is exposed here)
	if {[string match {.client.[rl]*.text} $This]} {

		# If the Line comparison window is visible AND the window insertion
		# cursor MOVED to a NEW line, then update the comparison display
		if {$opts(showlineview)
			&& $cmd == "mark" && $a1 == "set" && $a2 == "insert"
			&& $w(bLnum) != [set i [file rootname [$Real index insert]]]} {

			# Remember the screenline number for next time, then update
			set w(bLnum) $i
			set left [$w(LeftText)_	 get $i.0 $i.0lineend]
			set rght [$w(RightText)_ get $i.0 $i.0lineend]
			$w(BottomText) REPLACE 1.0 end "< $left\n> $rght"

			# find characters that are different, and hilite/tag them
			if {$left != $rght} {
				set c 2; # N.B> compensate for OUR "< " or "> " prefixes
				foreach l [split $left {}]	  r [split $rght {}] {
					if {[string compare $l $r] != 0} {
						$w(BottomText) tag add diff 1.$c "1.$c+1c"
						$w(BottomText) tag add diff 2.$c "2.$c+1c"
					}
					incr c
				}
				# but do not draw attention to either of the 'NL' chars
				$w(BottomText) tag remove diff "1.0 lineend"
				$w(BottomText) tag remove diff "2.0 lineend"
			}
		}

		# If the view has just CHANGED, its time for VISIBILITY-based tasks
		#	(autoselect and/or INLINE scheduling) and/or syncscroll
		#
		# N.B> Beware the subtlty of this test - it tacitly applies to all
		#	'yview' subcmnds (EXCEPT the introspection one having NO ARGS);
		#
		#		But ALSO permits 'Y* anything' (a synthetic YVIEW command) OR
		#	"see" to 'slip thru', providing ACCESS to the following visibility
		#	tasks EXCEPT the ability to notify the $Othr widget (-> syncscroll)
		if {([string match {[Yy]*} $cmd] || $cmd=="see")  && $a1 != {}} {

			# First - if the scroll we JUST DID happenned during an UNSAFE
			# data period, then we want to ASK that a SECOND scroll be
			# PROVIDED when the data finally BECOMES stable.
			#	N.B> choosing the Dbg variant simply REPORTs this new state
			if {!($w(SYNCnow) & $ThisID)} {
								$This SYNCnow		  ;# ONLY uncomment	 ONE!!
			#	Dbg {\tSYNCnow([$This SYNCnow])} 1 {} ;# ONLY uncomment	 ONE!!
			}

			# Next find what PHYSICAL line range (inclusive) is visible
			#	N.B> 10000 ensures we get the LAST-line despite window resizes
			set TpLn [file rootname [$Real index @0,0]]
			set BtLn [file rootname [$Real index @0,10000]]

			# When BOTH syncscroll and autoselect are ACTIVE, choose a NEW CDR
			if {$opts(syncscroll)} {
				if {$opts(autoselect) && $g(count) > 0 && $g(startPhase)>1} {
					# If probe point (Lnum at middle of window) yeilds a region
					# other than the CDR, AND some portion IS visible right NOW
					# it becomes the new CDR
					if {[set i [find-diff [expr ($TpLn+$BtLn)/2]]]!=$g(pos)} {
						lassign $g(scrInf,[hunk-id $i]) S E
						if {![set k [hCLIP $S $E $TpLn $BtLn]] || $k/4!=$k%4} {
							move $i 0 0 ;#N.B> (3rd arg 0) IGNORE repositioning
						}
					}
				}

				# CRITCAL design concept: Distinction between yview/YVIEW:
				#	YVIEW will NOT induce the "Othr" side to "talk back"
				#	Firestorm would OTHERWISE occur if a yview LEFTscroll was
				#	allowed to CAUSE a yview RIGHTscroll, reversing infinitely
				if {$cmd=="yview"} { $Othr YVIEW {*}$args }
			}

			# PERFORMANCE hook:
			#	Calc of INLINE diff markings, sadly, CAN be compute-intensive
			#
			# Accordingly, only DO them if KNOWN will be ONSCREEN (which both
			# limits AND distributes the time spent). Thankfully, once done, it
			# need not be calculated AGAIN until applicable UserPrefs change
			# (which sadly destroys ALL Calcs done to date); OR Split/Combine
			# manufactures a previously unknow hunk that is NOW "in-view"
			set INLsched 0
			if {$opts(showinline2) && $g(count) > 0 && $g(startPhase)>1} {
				lassign "[find-diff $TpLn] [find-diff $BtLn]" i j
				while {$i <= $j} {
					lassign $g(scrInf,[set hID [hunk-id $i]]) S E
					if {[string match "*c*" $hID]
					&& ![info exists g(inline,$hID)]
					&& (![set k [hCLIP $S $E $TpLn $BtLn]] || $k/4 != $k%4)} {
						# Sentinel "empty list" simultaneously prevents FURTHER
						# scheduling while it exists; yet REMOVING it serves to
						# CANCEL any "already being-processed" requests
						set g(inline,$hID) [list]
						after idle inline-hunk ratcliff $hID sched $TpLn $BtLn 0
						# Indicate we scheduled one or more items
						#	(helps determine when SYNCbusy NEEDED, just below)
						incr INLsched
						Dbg "SCHEDULED inline $hID" $D
					}
					incr i
				}
			}

			# (PRE TK8.6.5): scrolls performed WHILE lineHeights ARE in flux or
			# WILL BE from any JUST Scheduled INLINING, need to detect when its
			# again safe (via POLLing) to perform one final ALIGNMENT scroll.
			#	Scrolling BOTH sides (eg. syncscroll) can be even MORE critical
			# This initiates such POLLing to EMULATE a TK8.6.5 builtin feature.
			if {!$w(TK865) && (!($w(SYNCnow) & $ThisID) || $INLsched)
			&&	 $w(SYNCbusy$This) == ""} {
				# N.B> when queued, are placed BEHIND any inlines SCHEDULED
					set w(SYNCbusy$This) [after idle $This SYNCbusy 0]
				if {$opts(syncscroll)} {
					set w(SYNCbusy$Othr) [after idle $Othr SYNCbusy 0]
				}
			}
		}
	}
	return $result
}

###############################################################################
# Perform inline data re-computation and/or re-tagging across ALL hunks
###############################################################################
proc compute-inlines {optNam {flush 1}} {
	global g w opts

	# By default, remove ALL inline tags/data (so new ones MAY be added),
	#	(skipped when specifically TOLD that none presently exist)
	if {$flush} {
		$w(LeftText)  tag remove inlinetag 1.0 end
		$w(RightText) tag remove inlinetag 1.0 end
		array unset g "inline,*"
		# N.B> Unclear if all this CAUSES Vscroll to trigger on its own, which
		#	MIGHT then reschedule a Ratcliff inlining, thereby restoring the
		#	CORRECT view automatically - or if TK delays it -OR- causes it to
		#	trigger when Vscroll is temporarily blocked. But it is the reason
		#	WHY an extra 'vscroll tickle' was added into tail of "prefapply"!
		# UNCERTAIN if above untagging/unset cmd SEQUENCE might play a role
		Dbg "W I P E D inlines (ALL)"
	}

	# Compute inline data per requested algorithm style
	#
	# N.B> Ratcliff has become too compute-intensive to do ALL AT ONE time
	#		(once we added semantic suppression categories).
	#	Just leave them uncomputed and let $w(acTxWdg) scrolling do AS NEEDED.
	if {$optNam != "showinline2"} {
		foreach hID $g(diff) {
			# Remember: only chg-type hunks can EVER have inline diffs
			#	(N.B> deskews when turning 'OFF' to clean-up after flush)
			#	OTHERWISE generate NEW inline DATA, retag AND deskew
			if {[string match "*c*" "$hID"]} {
				if {$optNam == "off"} {de-skew-hunk $hID} {
					inline-hunk byte $hID 0
				}
			}
		}
		# But KICK scroll to *LOOK* for Ratcliff (when ON); otherwise is a NOOP
	} { $w(acTxWdg) SEE @1,1 }
}

###############################################################################
# Compute, mark and deskew the inline-differences for a given SINGLE HUNK
# IMPORTANT:
#	Depending on both style and effective SIZE of hunk, this proc MAY BE
#	re-scheduled to work its way thru large Ratcliff-based hunks in segments.
#	The GUI stays operational, but may feel slightly SLUGGISH until completion.
###############################################################################
proc inline-hunk {style hID args} {
	global	g w

	set D -1 ;# Dbg "GROUP" force/enable/disable flag (1/0/-1 resp.)

	# First, protect against the hunk DISAPPEARING during CHAINED processing,
	# - OR if an INPROGRESS calc must restart because its attributes changed.
	# Otherwise establish default initial values
	if {![info exists g(scrInf,$hID)]
	||	([lindex $args end] && ![info exists g(inline,$hID)])} {
		Dbg "  Inline ABORTED" $D ; return
	} {
		# Default calcs ALL lines (First->Lim), displaying when 'Lim' reached
		#	(AFTER getting last UN-padded Lnum, to assess active size of hunk)
		lassign $g(scrInf,$hID) Ls Le P(1) na na P(2)
		set Lim [set Last [expr {$P(1) ? $Le-$P(1) : $Le-$P(2)}]]
		lassign "0 $Ls 0 5" First Dsply Wrap heuristic
		# N.B.> 'First': remains as an INDEX until calcs begin; a LNUM after
		#		'heuristic': must be MANUALLY tuned for GUI responsiveness
	}

	# Ratcliff algorithm CAN require CHAINED processing to maintain UI response
	#	So limit calcs to a NEXT (or final) HUERISTIC-based PORTION of the data
	#	N.B> But arrange to get VISIBLE tagging done BEFORE most OFFSCREEN ones
	#		   by MAYBE splitting hunk into two BLOCKS (tail-first, then head)
	#		   (makes us look more responsive than we really are)
	#
	# Key items being juggled below (in passed order) are:
	#	Cmd	  - which BLOCK (head/tail) we are processing per chained iteration
	#	Wrap  - (!0) OFFSET where we STARTED the 'tail' portion (IFF done first)
	#	Lim	  - endpoint (inclusive) LNUM of current PORTION
	#	Dsply - LNUM where we should begin Tagging (endpt is heuristic based)
	#	Last  - hard endpoint (inclusive) LNUM where current 'Cmd' terminates
	#	First - OFFSET into hunk where CURRENT calculations should begin
	# This basically "orients" the data to acheive a visual speed illusion
	# Will sequence as: sched -> tail* -> (wrap to) head* -> done
	#		  OR		sched ->					head* -> done
	# Where '*' means distinct (REPEATING) heuristic-derived Calcs + Tagging
	#
	# RECOVER the 6 values from PRIOR chained-portion and adjust as necessary
	if {$style == "ratcliff"} { switch [lindex $args 0] {
		sched { lassign $args Cmd Wmin Wmax
			# N.B> 'sched' is the PRIMARY entry point (from scroll tracking)
			Dbg {: $hID} $D "SCHEDULING"
			# Derive which strategy for the PORTION that gets displayed first
			# Test looks for hunk SPANNING window top edge: ('tail' strategy)
			#	CRITICAL:  Vars are CURRENTLY set as for the 'head' strategy!
			if {$Ls < $Wmin && $Le <= $Wmax && $Le >= $Wmin} {
				set First [expr $Wmin-$Ls] ;# adjust to begin @1st visible line
				# ALSO sets Wrap as eventual 'First' AFTER 'wrapping' to 'head'
				lassign "tail $First $Wmin $Last" Cmd Wrap Dsply
			} { set Cmd "head" }
			# (mnemonics: WrapIdx  Lim	Dsply@	 Last@	 1stIdx
			Dbg {: $Cmd Wi$Wrap Lm$Lim D@$Dsply L@$Last Fi$First} $D "PERFORM as"
		}
		tail {	lassign $args Cmd Wrap Lim Dsply tLast First
			# Watch for when to "wrap" to the 'head' approach (and cancel Wrap)
			if {$First + $Ls > $Lim} {
				lassign "head  0	$Last	$Ls		0" \
						 Cmd  Wrap	 Lim   Dsply  First
				Dbg {: $Cmd $Wrap $Lim $Dsply $Last $First} $D "REwritten as"

			# otherwise simply MAINTAIN 'tail' viewpoint for affected Vars
			} { set Last $tLast }
		}
		head {	lassign $args Cmd Wrap Lim Dsply Last First
			# Just keep USING values which were passed till done
		}}

		# Finally, APPLY the heuristic to keep the UI responsive throughout
		if {$First + $Ls + $heuristic < $Lim} {
		  set Lim [expr $First + $Ls - 1 + $heuristic]
		}
	} else { Dbg "hID($hID) is @ SCREEN line #$Ls (for Lnum correlation)" $D }
	# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

	# Perform the CURRENT inline difference calculation (for EITHER style)
	#	N.B> Note subtle transform of 'First' from Hunk offset into LineNum
	for {set i [incr First $Ls]} {$i <= $Lim} {incr i} {
		inline-$style $hID [expr $i - $Ls]	 \
			   [$w(LeftText) get $i.0 $i.end] [$w(RightText) get $i.0 $i.end]
		Dbg {Calc'd Lnum $i (Ofst [expr $i-$Ls]) until Lim ($Lim)} $D
	}

	# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
	if {$style=="ratcliff"} {
		# Place an EARLY SUBSET of tags onscreen NOW ?
		Dbg {?DISPLAY?: i($i) Lim($Lim) Last($Last) Dsply($Dsply)} $D
		if {$i < $Last && $i > $Dsply + ($heuristic * 2)} {
			remark-inline $hID $Dsply [incr Dsply [expr ($heuristic * 3) - 1]]
			incr Dsply ; # Further-adjust for FUTURE display start positions

		# Otherwise just flush whatever remains when endpoint is hit
		} elseif {$i > $Last} { remark-inline $hID $Dsply $Last
		} { Dbg "		  : NOT DONE" $D }

		# Reschedule any REMAINING portion into a CHAINED timeslot (as needed)
		# N.B> Sends current (w/reset Lim) values (NXT iteration will readjust)
		if {$i <= $Last || $Wrap} { after 25 inline-hunk $style $hID \
								$Cmd $Wrap $Last $Dsply $Last [expr $i-$Ls]
		} { Dbg "  Inline completed" $D }
	} { remark-inline $hID }
}

###############################################################################
# Functionality: Inline diffs
# Athr: Michael D. Beynon : mdb - beynon@yahoo.com
# Date: 04/08/2003 : mdb - Added inline character diffs.
#		04/16/2003 : mdb - Rewrote longest-common-substring to be faster.
#						 - Added byte-by-byte algorithm.
#		08Oct2017  : mpm - Simplified byte-by-byte alg.
#						 - Revised generated output data format (both alg.)
#		12Jun2018  : mpm - Rewrote lcs-string (again) to be even faster.
#
# The recursive version is derived from the Ratcliff/Obershelp pattern
# recognition algorithm (Dr Dobbs July 1988), where we search for a longest
# common substring between two strings. This match is used as an anchor,
# around which we recursively do the same for the two left and two right
# remaining substrings (omitting the anchor).
# This precisely determines the location of the intraline tags.
#
#		05Sep2021  : mpm - Redesigned to allow semantic-matching within the LCS
#		19Mar2022  : mpm - Re-engineer to ADD mistakenly NONhandled "-E" option
#
# (L)ongest (C)ommon (S)ubstring WITH (v)arying (b)lank/(c)ase Support
#	Emulates the following optional Diff MATCH-SUPPRESSION options:
#	-i	Case insensitivity					(Cign)
#	-w	Any WhtSpc at all					(Wign)
#	-b	Paired WhtSpc of different lengths	(Wcnt)
#	-E	Paired WhtSpc ending @COMMON Column (Wtab)
#	-Z	WhtSpc at E-O-Line					(Weol)	(but see initialization)
#
### N.B> C R I T I C A L: caller at LIBERTY to INTERCHANGE all (1<->2) AT WILL!
proc LCSvbc {ops
		s1 of1 ln1 SP1 M1eol LCSB1 LCSE1	s2 of2 ln2 SP2 M2eol LCSB2 LCSE2} {
	upvar $LCSB1 LcsB1 $LCSE1 LcsE1		;# LONGER  exemplar str (eg. haystack)
	upvar $LCSB2 LcsB2 $LCSE2 LcsE2		;# SHORTER exemplar str (eg.  needle )

	# Initializations	(N.B> Bit(1)==Weol was handled by 'inline-ratcliff')
	lassign "[expr $of1+$ln1] [expr $of2+$ln2] 0	16	 8	  4	   2" \
					M1				M2		   best Cign Wign Wcnt Wtab

	# Basis of loops is to proceed similar to that of "strstr(haystack,needle)",
	# but looking for ANY/EVERY POSSIBLE SUBSTR within both strings to "match",
	# PLUS the rules for matching are OPTIONALLY more semantic than "identical".
	#	Loop structure: 1st steps through s1 (once); 2nd thru s2 (repetitively);
	#					3rd simply "matches" next avail char (or meta-group)
	#		 'Continue's indicate EXTENDING the current match; 'break' is can't.
	#		 Outer 2 loops abort when remaining bytes incapable of a NEW 'best'
	#		 'SkpN's REMOVE one entire MATCH-element PREFIX (often 1 char)
	for {lassign "$of1 $of1" S1 E1} {
			$E1 < ($M1-($ln2<$best ? $best-$ln2:0))} {
					set E1 [incr S1 [expr ($skp1 ?$skp1:1)]]} {

		for {lassign "$of2 $of2 0" S2 E2 skp1}	{
				$E2 < ($M2-($ln2<$best ? 0:$best))} {
					set E2 [incr S2 [expr ($skp2 ?$skp2:1)]]} {

			for {lassign "$S1 0" E1 skp2}  {
					$E1 < $M1 && $E2 < $M2} {
								  set skp2 [expr ($skp2 ? $skp2 : $E2-$S2)]
								  set skp1 [expr ($skp1 ? $skp1 : $E1-$S1)]}  {

				# We build a LCS one matched ITEM (often a single char) at a
				# time, *BUT* Tcl is NOT very adept at "aggregate expressions"
				# - so we PRE-DERIVED certain key values needed. Go GRAB them
				# (as SPARINGLY as possible), starting w/actual CHARs + if its
				# WhtSpc (<0) and/or its EFFECTIVE Column position, EXPRESSED
				# as MAGNITUDE only - (BECAUSE WhtSpc is coded AS NEGATIVE!).
				set C1 [string index $s1 $E1] ; set C1sp [lindex $SP1 $E1]
				set C2 [string index $s2 $E2] ; set C2sp [lindex $SP2 $E2]

				# WhtSpc	(INSIGNIFICANT or NON-EXISTENT)	 #######
				#	aka - Identical matching and/or
				#		Capitalization		meaningless (diffopt: -i) #####
				if {($C1sp>0 && $C2sp>0) || !($ops&14)} {
					for {set i 0} {$E1<$M1 && $E2<$M2 &&
						("$C1"=="$C2" || ($ops&$Cign && [string tolower "$C1"]
									 == [string tolower "$C2"]))} { incr i } {
							set C1 [string index $s1 [incr E1]]
							set C1sp [lindex $SP1 $E1]
							set C2 [string index $s2 [incr E2]]
							set C2sp [lindex $SP2 $E2]
					 }
					 # CONTINUE presuming we collect ANTHING contiguously...
					 if {$i} {
						 # When WhtSpc suppression is active but STOPS ON one,
						 # 'backing-up' 1 position (IFF WAS a WhtSpc MATCH)
						 # makes NEXT CYCLE simpler to parse AFTER we continue.
						if {$E1<$M1 && $E2<$M2 && ($ops&14)
						&&	(($C1sp<0) ^ ($C2sp<0)) && [lindex $SP1 $E1-1]<0} {
							incr E1 -1; incr E2 -1
						}
						continue
					 }

				# WhtSpc									###############
				#		totally				meaningless (diffopt: -w) -OR-
				#		count Varying		meaningless (diffopt: -b)
		} elseif {($ops&$Wign			&& (($C1sp<0) || ($C2sp<0)))
					  || ($ops&$Wcnt && $C1sp<0 && (($C1sp<0) == ($C2sp<0)))} {
					while {$E1 < $M1 && $C1sp<0} {
											set C1sp [lindex $SP1 [incr E1]] }
					while {$E2 < $M2 && $C2sp<0} {
											set C2sp [lindex $SP2 [incr E2]] }
					continue;

				#		Columnar aligned	meaningless (diffopt: -E) #####
				} elseif {$ops&$Wtab && $C1sp<0 && $C2sp<0} {
					# First PROBE where EACH WhtSpc ENDS ...
					for {set j [set i 0]} \
							{$E1+$i<$M1 && [lindex $SP1 $E1+$i]<0} \
															{incr i} {set j $i}
					for {set k [set i 0]} \
							{$E2+$i<$M2 && [lindex $SP2 $E2+$i]<0} \
															{incr i} {set k $i}
					# ... and if thats the SAME COLUMN - it MATCHED
					if {[lindex $SP1 $j] == [lindex $SP2 $k]} {
						incr E1 $j ; incr E2 $k ; continue }

				#		any @EOL			meaningless (diffopt: -Z) #####
				#							(PRE-handled at initialization)
				}
				break;	# <<==--	ITEM did  *N O T*  Match ##############
			}

			# Record best seen (based on len of consumed bytes from BOTH strs)
			#	(position values noted are ABSOLUTE within each ORIGINAL string
			#	 representing the byte where marking turns ON and then, OFF)
			if {[set i [expr $E1-$S1]] + [set j [expr $E2-$S2]] > $best} {
				set LcsE1 [expr [set LcsB1 $S1] + $i]
				set LcsE2 [expr [set LcsB2 $S2] + $j]
				set best  [expr [set skp1  $i]	+ $j]
				# Altering skp1 AVOIDS time spent on ONLY(?) fail attempts...
				break	;# ... BREAKING here allows that to take effect NOW
			}
		}
	}
	return $best
}

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Recursive Ratcliff/Obershelp DFS walk mechanism
#	N.B> UNLIKE 'LCSvbc' above, these are THE REAL (1<->2) based variable sets
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
proc ROwalk {hID pairID ops	 s1 off1 len1 SP1 M1eol	 s2 off2 len2 SP2 M2eol}  {
	global g

	# Bgn[12]/End[12] demarcate where LCS was FOUND in EACH string
	#	(N.B> due to semantic 'ops', they might NOT be a common length)
	lassign {0 0 0 0 0} Bgn1 End1 Bgn2 End2 cnt

	if {$len1>0 && $len2>0} {
		# LCS Alg wants the LONGER string as its first argset,
		#			(eg. XXX(haystack, needle)
		#	   (while not (strictly) commutative, it *IS* deterministic
		#	   and WILL tend to execute quicker when configured this way)
		if {$len2 < $len1} {
			set cnt [LCSvbc $ops $s1 $off1 $len1 $SP1 $M1eol Bgn1 End1 \
								 $s2 $off2 $len2 $SP2 $M2eol Bgn2 End2]
		} else {
			set cnt [LCSvbc $ops $s2 $off2 $len2 $SP2 $M2eol Bgn2 End2 \
								 $s1 $off1 $len1 $SP1 $M1eol Bgn1 End1]
		}

		# A ZERO indicates no 'LCS' AT ALL between s1/s2; otherise do the left
		# AND right substring-pairs lying to EACH side of the LCS (if present)
		if {$cnt > 0} {
			if {$Bgn1 > $off1 || $Bgn2 > $off2} {
				# left
				incr cnt [ROwalk $hID $pairID $ops \
							$s1 $off1 [expr $Bgn1-$off1] $SP1 $M1eol\
							$s2 $off2 [expr $Bgn2-$off2] $SP2 $M2eol]

			}
			if {$End1<$off1+$len1 || $End2<$off2+$len2} {
				# right
				incr cnt [ROwalk $hID $pairID $ops \
							$s1 $End1 [expr $off1+$len1-$End1] $SP1 $M1eol\
							$s2 $End2 [expr $off2+$len2-$End2] $SP2 $M2eol]
			}
		}
	}

	# When one or both strings do not subdivide, the REMAINDER becomes marked
	# Yet option "-w"(8) can STILL affect WHICH chars to NOT mark

	# RIGHT-side markings:
	if {$len2>0 && ($len1<1 || !$cnt)} {
		# Continue processing further for "-w" (aka Wign)
		if {($ops & 8)} {
			for {incr len2 $off2} {[set mark $off2] < $len2} {} {
				if {[string is space [string index $s2 $off2]]} {
					incr off2 ; continue
				}
				while {($off2 < $len2) &&
					![string is space [string index $s2 $off2]]} {
						incr off2}
				lappend g(inline,$hID) r $pairID $mark $off2
			}
		} elseif {$len2} {lappend g(inline,$hID) r \
											  $pairID $off2 [expr $off2+$len2]}
	}

	# LEFT-side markings:
	if {$len1>0 && ($len2<1 || !$cnt)} {
		# Continue processing further for "-w" (aka Wign)
		if {($ops & 8)} {
			for {incr len1 $off1} {[set mark $off1] < $len1} {} {
				if {[string is space [string index $s1 $off1]]} {
					incr off1 ; continue
				}
				while {$off1 < $len1 &&
					![string is space [string index $s1 $off1]]}	{incr off1}
				lappend g(inline,$hID) l $pairID $mark $off1
			}
		} elseif {$len1} {lappend g(inline,$hID) l \
											  $pairID $off1 [expr $off1+$len1]}
	}

	return $cnt
}

###############################################################################
# Gateway to the Ratcliff/Obershelp Alg. for marking inline differences
# Returns COUNT of "matched" chars - if ==L1+L2 then lines considered identical
###############################################################################
proc inline-ratcliff {hID pairID s1 s2} {
	global g opts

	if {![set L1 [string length $s1]] || ![set L2 [string length $s2]] } {
		return 0
	}

	# To AVOID excessive repetition of certain derivations (WhtSpc/Columns) we
	# do them ONCE here and PROVIDE them for usage later.
	# Create PARALLEL-indexed lists that denote what COLUMN number each char
	# represents AND if that char *is* WhtSpc (negative==YES) or not.
	set Tsz $opts(tabstops)
	set col 0

	# Do ONCE for the first string ...
	foreach ch [split $s1 {}] {
		if {[string is space $ch]} {
			lappend SP1 -[incr col [expr ("$ch"=="\t"? $Tsz-($col%$Tsz):1)]]
		} { lappend SP1	 [incr col] }
	}

	# ... and again for the other string
	set col 0
	foreach ch [split $s2 {}] {
		if {[string is space $ch]} {
			lappend SP2 -[incr col [expr ("$ch"=="\t"? $Tsz-($col%$Tsz):1)]]
		} { lappend SP2	 [incr col] }
	}

	# Dispense with the ENTIRE "-Z" suppression category IMMEDIATELY (if reqd)
	#	(remember to collect the count of 'matched' chars)
	set col 0
	if {$opts(inlSuprs)&1} {
		while {$L1 && [lindex $SP1 $L1-1]<0} {incr L1 -1 ; incr col}
		while {$L2 && [lindex $SP2 $L2-1]<0} {incr L2 -1 ; incr col}
	}

	# THEN begin the Algorithm PASSING all the info it needs
	# N.B> Return code is a count of matched bytes
	incr col [ROwalk $hID $pairID $opts(inlSuprs) \
									$s1 0 $L1 $SP1 $L1		$s2 0 $L2 $SP2 $L2]
	return $col
}

###############################################################################
# Gateway to the (rather simplistic) "Byte" Alg. for marking inline differences
# Returns NON-zero count of total items marked
###############################################################################
proc inline-byte {hID pairID s1 s2} {
	global g

	if {![set len1 [string length $s1]] || ![set len2 [string length $s2]] } {
		return 0
	}

	set lenmin [min $len1 $len2]
	set cnt 0
	set size 0
	for {set i 0} {$i <= $lenmin} {incr i} {
		if {[string index $s1 $i] == [string index $s2 $i]} {
			# start/continue a NON-diff region
			if {$size} {
				# which ENDS a diff region
				lappend g(inline,$hID)	r $pairID [expr $i-$size] $i
				lappend g(inline,$hID)	l $pairID [expr $i-$size] $i
				set size 0
				incr cnt
			}
		} else { incr size }
	}
	if {$size} {
		# ended in a diff region
		lappend g(inline,$hID)	r $pairID [expr $i-$size] $len2
		lappend g(inline,$hID)	l $pairID [expr $i-$size] $len1
		incr cnt
	}
	return $cnt
}

###############################################################################
# create (if necessary) and show the find dialog
###############################################################################
proc srch-text {} {
	global g w

	if {![Dialog NONMODAL $w(srch)]} {
		wm title $w(srch) "$g(name) Find"
		wm transient $w(srch) .
		wm	 group	 $w(srch) .

		# we don't want the window to be deleted, just hidden from view
		wm protocol $w(srch) WM_DELETE_WINDOW {Dialog dismiss $w(srch)}

#### MCP
#		frame $w(srch).content -bd 2 -relief groove
		frame $w(srch).content
#### MCP
		pack $w(srch).content -side top -fill both -expand y -padx 0 -pady 5

		frame $w(srch).buttons
		pack $w(srch).buttons -side bottom -fill x -expand n

		set wrapd [message $w(srch).buttons.msg -aspect 1000]
		button $w(srch).buttons.doit -text "Find Next" -command "srchit $wrapd"
		button $w(srch).buttons.dismiss -text "Dismiss" \
			-command  { Dialog dismiss $w(srch)
						$w(acTxWdg) yview [$w(acTxWdg) index @0,0] }
		pack $w(srch).buttons.dismiss -side right -pady 5 -padx 0
		pack $w(srch).buttons.doit -side right -pady 5 -padx 1
		pack $w(srch).buttons.msg -side left -expand 1 -pady 5 -padx 1

		set ff $w(srch).content.findFrame
		frame $ff -height 100 -bd 2 -relief flat
		pack $ff -side top -fill x -expand n -padx 0 -pady 5

		label $ff.label -text "Find what:" -underline 2

		entry $ff.entry -textvariable g(findString)

		checkbutton $ff.searchCase -text "Ignore Case" -indicatoron true \
			-variable g(findIgnoreCase) -offvalue "" -onvalue "-nocase"

		checkbutton $ff.keepSyncd -text "Stay Sync'd" -indicatoron true \
			-variable g(staySyncd) -offvalue 0 -onvalue 1

		grid $ff.label		-row 0 -column 0 -sticky e	 -rowspan 2
		grid $ff.entry		-row 0 -column 1 -sticky ew	 -rowspan 2
		grid $ff.searchCase -row 0 -column 2 -sticky nw
		grid $ff.keepSyncd	-row 1 -column 2 -sticky sw
		grid columnconfigure $ff 0 -weight 0
		grid columnconfigure $ff 1 -weight 1
		grid columnconfigure $ff 2 -weight 0

		# we need this in other places...
		set w(findEntry) $ff.entry

		bind $ff.entry <Return> "srchit $wrapd"

		set of $w(srch).content.optionsFrame
		frame $of -bd 2 -relief flat
		pack $of -side top -fill y -expand y -padx 10 -pady 10

		label $of.directionLabel -text "Search Direction:" -anchor e
		radiobutton $of.directionForward  -text	 "Down" -indicatoron true \
			-variable g(findDirection)	  -value "-forward"
		radiobutton $of.directionBackward -text	 "Up" -indicatoron true \
			-variable g(findDirection)	  -value "-backward"


		label $of.windowLabel -text "Window:" -anchor e
		radiobutton $of.windowLeft	  -text "Left"	 -indicatoron true \
			-variable w(acTxWdg) -value $w(LeftText)
		radiobutton $of.windowRight	  -text "Right" -indicatoron true \
			-variable w(acTxWdg) -value $w(RightText)


		label $of.searchLabel -text "Search Type:" -anchor e
		radiobutton $of.searchExact	 -text	"Exact"	  -indicatoron true \
			-variable g(findType)	 -value "-exact"
		radiobutton $of.searchRegexp -text	"Regexp"  -indicatoron true \
			-variable g(findType)	 -value "-regexp"

		grid $of.directionLabel	   -row 1 -column 0 -sticky w
		grid $of.directionForward  -row 1 -column 1 -sticky w
		grid $of.directionBackward -row 1 -column 2 -sticky w

		grid $of.windowLabel -row 0 -column 0 -sticky w
		grid $of.windowLeft	 -row 0 -column 1 -sticky w
		grid $of.windowRight -row 0 -column 2 -sticky w

		grid $of.searchLabel  -row 2 -column 0 -sticky w
		grid $of.searchExact  -row 2 -column 1 -sticky w
		grid $of.searchRegexp -row 2 -column 2 -sticky w

		grid columnconfigure $of {0 1} -weight 0
		grid columnconfigure $of   2   -weight 1

		set g(findType)		 "-exact"
		set g(findDirection) "-forward"
		set g(findIgnoreCase) "-nocase"
		set g(lastSearch)	 ""

		# TENTATIVELY try for Text window (PROVIDED it HAS the focus)
		#	(N.B> remember its TRACED to only PERMIT setting BY a TextWdg)
		set w(acTxWdg) [focus]

		# On creation, flop it centerred (then let user put it anywhere)
		centerWindow $w(srch)
	}
	# Only config on (re)display is to ensure message starts empty
	$w(srch).buttons.msg configure -text {}

	# Put it onscreen (NON MODAL)
	Dialog show $w(srch) $w(findEntry)
}


###############################################################################
# do the "Edit->Copy" functionality, by copying the current selection
# to the clipboard
###############################################################################
proc do-copy {} {
	clipboard clear -displayof .
	# figure out which window has the selection...
	catch {
		clipboard append [selection get -displayof .]
	}
}

###############################################################################
# search for the text in the find dialog
###############################################################################
proc srchit {wrapWdg} {
	global g w

	if {[set win $w(acTxWdg)] == "$w(LeftText)"} {
		set Otherwin $w(RightText)
	} { set Otherwin  $w(LeftText) }

	if {$g(lastSearch) != ""} {
		if {$g(findDirection) == "-forward"} {
			set start [$win index "insert +1c"]
		} { set start insert }
	} { set start 1.0 }

	# Eval needed as findIgnoreCase may turn into an EMPTY (eg. NON) Arg
	set result [eval $win search $g(findDirection) $g(findType) \
		 $g(findIgnoreCase) -- $g(findString) $start]

	if {[string length $result] > 0} {
		$wrapWdg configure -text {}
		# if this is a regular expression search, get the whole line and try
		# to figure out exactly what matched; otherwise we know we must
		# have matched the whole string...
		if {$g(findType) == "-regexp"} {
			set line [$win get $result "$result lineend"]
			if {$g(findIgnoreCase)} {
				regexp -nocase -- $g(findString) $line matchVar
			} { regexp -- $g(findString) $line matchVar }
			set length [string length $matchVar]
		} { set length [string length $g(findString)] }

		set g(lastSearch) $result
		$win mark set insert $result
		$win tag remove sel 1.0 end
		$win tag add sel $result "$result + ${length}c"
		$win SEE $result
		if {$g(staySyncd)} { $Otherwin YVIEW [$win index @0,0] }
		focus $win

	} {	 bell ;# MAY be SILENT? (thats Annoying); Visually hint we found NADA!
		$wrapWdg configure -text {Nothing found} -bg Tomato
		after 1250 $wrapWdg config -bg $w(bgnd) -text {{Next attempt restarts}}
		set g(lastSearch) {}
	}
}

###############################################################################
# Build the menu bar AND the popup menu (Do AFTER client has been built)
###############################################################################
proc build-menus {} {
	global g w opts finfo

	# We are building TWO distinct menu TREEs here: popUp and menuBar
	#	Generate THEM and then we can let the factory build them all out
	# N.B> menubar is strange in that it LACKS a native way to indicate it HAS
	#	   focus - hence the elaborate 'takefocus'-proc to provide SOME visual
	#	   feedback (the activebackg) that you've TAB'd to the right place!!
	#	   The 'bind' simply exists to extinguish the feedback on a next TAB
	#	   (but DEPENDS on the TK IMPLEMENTATION of that named virtual event)
	set pM [menu $w(popupMenu)]
	set mB [menu $w(menubar) -activebackg $w(selcolor) -takefocus \
						{apply {wdg { return [expr {[$wdg activate 0]=={}}] }}}]
	bind $mB <<TraverseOut>> { %W activate none }

	# Export NAMING of cascaded nodes (factory uses the short local synonyms)
	#	N.B> do NOT pre-CREATE such menus - Factory will do that
	set w(fileMenu)		 [set fM  $w(menubar).file]
	set w(multiFileMenu) [set mFM $fM.multi]
	set w(viewMenu)		 [set vM  $w(menubar).view]
	set w(helpMenu)		 [set hM  $w(menubar).help]
	set w(editMenu)		 [set eM  $w(menubar).edit]
	set w(mergeMenu)	 [set gM  $w(menubar).merge]
	set w(markMenu)		 [set mM  $w(menubar).marks]

	# Data specification for driving the MENU building Factory
	#############################################################
	#	Menu	 Type	  Label { positional type-specific args }
	#								 Ul: underline
	#								 Mu: menu
	#	Items enclosed in [ ]		 Cm: command
	#	   are optional as			 Ac: accelerator-link
	#		  depicted				 Vr: state-variable
	#								 Ov: ON-value
	#								 Tp: Tooltip text
	# # #	#	#	#	#	#	#	#	#	#	#	# # #
	#	.abc	separator	 {}	 {						}
	#	.def	cascade		 Lb	 { Ul  Mu			 [Tp] }
	#	.mno	command		 Lb	 { Ul  Cm  [Ac]		 [Tp] }
	#	.xyz	checkbutton	 Lb	 { Ul [Cm] [Ac] Vr Ov Tp  }
	foreach {Mu Ty Lb Ul} [subst -nocommands {
	$pM comm "First Diff"	  {0 {move first}	   navFrst}
	$pM comm "Previous Diff"  {0 {move -1}		   navPrev}
	$pM comm "Center Current Diff"	{0 {centerCDR} navCntr}
	$pM comm "Next Diff"	  {0 {move	1}		   navNext}
	$pM comm "Last Diff"	  {0 {move	last}	   navLast}
	$pM separator {}		  {}
	$pM comm "Find Nearest Diff" \
					 {13 {moveNearest \$w(mPopW) menu \$w(mPopX) \$w(mPopY)}}
	$pM separator {}		  {}
	$pM comm "Find..."		  {1 {srch-text} genFind}
	$pM comm "Edit"			  {0 {do-edit \$w(mPopX) \$w(mPopY)}   genEdit}
	$pM separator {}		  {}
	$pM comm "Copy Selection" {5 {do-copy}			}

	$mB casc "File"	 {0 $fM}
	  $fM comm "New..."	   {0 "do-new-diff" {}
						   "Select new input parameters and compute a new Diff"}
	  $fM casc "File List" {5 $mFM
	   "Choose a different file pair from those derived from the present input"}
		$mFM comm "Reconfig Threshold..." {0 {multiFile dialog 1} {}
						  "Adjust where filelist is displayed (menu or dialog)"}
		$mFM comm "Previous File" {0 {multiFile prev} genPvfile
												"Choose the previous file pair"}
		$mFM comm "Next File" {1 {multiFile next} genNxfile
													"Choose the next file pair"}
	  $fM separator {} {}
	  $fM comm "Recompute Diffs" {0 {reCalcD user}	genRecalc
						"Recompute all difference regions for the current file"}
	  $fM separator {} {}
	  $fM comm "Write Report..." {0 {rpt-gen popup} {}
			"Configure and produce Diff textual content and statistical output"}
	  $fM separator {} {}
	  $fM comm "Exit" {1 {do-exit} genXit	   "Immediately terminate $g(name)"}

   $mB casc "Edit"	{0 $eM}
	  $eM comm "Copy" {0 {do-copy} {}
							"Copy the currently selected text to the clipboard"}
	  $eM separator {} {}
	  $eM comm "Find..." {0 {srch-text} genFind
					"Pop up a dialog to search for a string within either file"}
	  $eM separator {} {}
	  $eM comm "Ignore CDR" {0 {ignore-hunk} {}
				 "Suppress the CDR to no longer be seen as a Difference region"}
	  $eM comm "Split..." {0 {splcmbDlg 0} {}
								   "Split the current diff at specified bounds"}
	  $eM comm "Combine..." {2 {splcmbDlg 1} {}
					"Combine the current diff region with ADJACENT neighbor(s)"}
	  $eM separator {} {}
	  $eM comm "Edit File 1" {10 \
		  {do-edit [winfo rootx $w(LeftText)].0 [winfo rooty $w(LeftText)].0} {}
									  "Launch an editor for the left side File"}
	  $eM comm "Edit File 2" {10 \
		{do-edit [winfo rootx $w(RightText)].0 [winfo rooty $w(RightText)].0} {}
									 "Launch an editor for the right side File"}
	  $eM separator {} {}
	  $eM comm "Preferences..." {0 {customize} {}
										"Pop up a window to customize $g(name)"}

	$mB casc "View"	 {0 $vM}
	  $vM checkb "Utilize Suppressions" {0
								  {reCalcD ignSuprs RevAlgn} {} opts(ignSuprs) 2
						   "If set, applys suppression options during the Diff"}
	  $vM checkb "Ignore Blank Lines" {7
						{reCalcD ignoreEmptyLn RevAlgn} {} opts(ignoreEmptyLn) 8
							"If set, suppress empty lines from becoming a Diff"}
	  $vM checkb "Ignore RE-matched Lines" {7
						{reCalcD ignoreRegexLn RevAlgn} {} opts(ignoreRegexLn) 4
			 "If set, suppress Diffs from lines matching Regular Expression(s)"}
	  $vM separator {} {}
	  $vM checkb "Show Line Numbers"  {3 {do-show-Info showln} {} opts(showln) 1
					  "If set, show line numbers beside each line of each file"}
	  $vM checkb "Show Change Bars" {6 {do-show-Info showcbs} {} opts(showcbs) 1
				 "If set, show the changebar column for each line of each file"}
	  $vM checkb "Show Diff Map" {10 {do-show-map} {} opts(showmap) 1
		"If set, display the graphical 'Diff Map' in the center of the display"}
	  $vM checkb "Auto Center" \
				  {0 {if {\$opts(autocenter)} {centerCDR}} {} opts(autocenter) 1
		 "If set, moving to another diff region centers the diff on the screen"}
	  $vM checkb "Auto Select" {6 {} {} opts(autoselect) 1
		"If set, automatically selects the nearest diff region while scrolling"}
	  $vM checkb "Show Line Comparison Window" \
								  {21 {do-show-lineview} {} opts(showlineview) 1
					 "If set, display the window with byte-by-byte differences"}
	  $vM checkb "Show Inline Comparison (byte)" \
						 {26 {do-show-inline showinline1} {} opts(showinline1) 1
							  "If set, display inline byte-by-byte differences"}
	  $vM checkb "Show Inline Comparison (recursive)" \
						 {31 {do-show-inline showinline2} {} opts(showinline2) 1
						   "If set, display inline recursive based differences"}
	  $vM separator {} {}
	  $vM checkb "Synchronize Scrollbars" {0 {} {} opts(syncscroll) 1
					 "If set, scrolling either window will scroll both windows"}
	  $vM separator {} {}
	  $vM comm "First Diff" {0 {move first} navFrst
												   "Go to the first difference"}
	  $vM comm "Previous Diff" {0 {move -1} navPrev
				  "Go to the diff region just prior to the current diff region"}
	  $vM comm "Center Current Diff" {0 {centerCDR} navCntr
							"Center the display around the current diff region"}
	  $vM comm "Next Diff" {0 {move 1} navNext
					 "Go to the diff region just after the current diff region"}
	  $vM comm "Last Diff" {0 {move last} navLast	"Go to the last difference"}

	$mB casc "Mark"	 {3 $mM}
	  $mM comm "Bookmark Current Diff" {0 {bkmark creat} {}
						  "Create a Bookmark for the current difference region"}
	  $mM comm "Clear Current Bookmark" {0 {bkmark erase} {}
						 "Clear the Bookmark for the current difference region"}

	$mB cascade "Merge" {0 $gM}
	  $gM checkb "Show Merge Window" {9 {do-show-merge 1} {} g(showmerge) 1
						   "Pops up a window showing the current merge results"}
	  $gM comm "Write Merge File..." {6 {merge-write-file} {}
			 "Write the merge file to disk AFTER confirming the filename first"}

	$mB cascade "Help"	{0 $hM}
	  $hM comm "On Concepts+Syntax" {3 {help-concept gui} {}
									  "Show help on the command line arguments"}
	  $hM comm "On GUI" {3 {help-GUI} {}
						 "Show help on how to use the Graphical User Interface"}
	  $hM comm "On Preferences" {3 {help-prefs} {}
								   "Show help on the user-settable preferences"}
	  $hM separator {} {}
	  $hM comm "About $g(name)" {0 {about-TkD} {}
									  "Show information about this application"}
	  $hM comm "About Wish" {6 {about-wish} {}
						 "Show information about the TK Windowing-Shell (Wish)"}
	  $hM comm "About Diff" {6 {about-diff} {}
									   "Show information about the diff-engine"}
	}] {
		# THIS is the MENU factory (which processes the above list)
		#	N.B> for those items HAVING accelerators, THEY will be attached
		# LATER - all we do NOW is DEFINE to which menuItems they should GO
		switch -glob $Ty {
		ca* { lassign $Ul Ul mU Tp
			$Mu add cascade -label $Lb -underline $Ul -menu [menu $mU]
			if {$Tp!={}} { set g(tooltip,$Lb) "$Tp" }
		}
		co* { lassign $Ul Ul Cm Ac Tp
			$Mu add command -label $Lb -underline $Ul -command $Cm
			if {$Ac!={}} { lappend w(Accel,$Ac) $Mu [$Mu index end] }
			if {$Tp!={}} { set g(tooltip,$Lb) "$Tp" }
		}
		ch* { lassign $Ul Ul Cm Ac Vr Ov Tp
			$Mu add checkbutton -label $Lb -underline $Ul -vari $Vr -onval $Ov
			if {$Ac!={}} { lappend w(Accel,$Ac) $Mu [$Mu index end] }
			if {$Cm!={}} { $Mu entryconfig [$Mu index end] -command $Cm }
			set g(tooltip,$Lb) "$Tp"
		}
		s*	{ $Mu add separator }
		}
	}

	### Silly extra things easier to do from OUTSIDE the factory...
	#
	# Alternate tooltip (for when TkDiff REMOVES the "..." from the label)
	set "g(tooltip,Write Merge File)" \
			"Write the merge file to disk USING the command line specified name"

	# It is not readily apparent that the first 3 View Menu items will FORCE
	# a Diff to occur - try to (subtley) warn the user of this with a HILITE
	verify alertD pushtomenu $vM

	# And this simply ISN'T a user-modifiable binding (as expected by factory)
	$pM entryconfigure "Find Near*" -accelerator "Dbl-Click"

	# Establish the bindings to provide menuItem Tooltips
	foreach m "$fM $mFM $eM $vM $mM $gM $hM" {
		bind $m <<MenuSelect>> {showTooltip menu %W}
	}
}

###############################################################################
# Show explanation of an item (menu/toolbutton) in the status bar at the bottom
# PRIMARILY used only for menu items:
#	Still works for buttons PROVIDED 'set_tooltips' was NOT CALLED for a popup
#		(for us, thats the Bookmark toolbuttons)
###############################################################################
proc showTooltip {type wdg} {
	global g

	switch -- $type {
	menu {
			if {[catch {$wdg entrycget active -label} label]} {
				set label ""
			}
			if {[info exists g(tooltip,$label)]} {
				set g(statusInfo) $g(tooltip,$label)
			} else {
				set g(statusInfo) $label
			}
		}
	button {
			if {[info exists g(tooltip,$wdg)]} {
				set g(statusInfo) $g(tooltip,$wdg)
			} else {
				set g(statusInfo) ""
			}
		}
	}
	update idletasks
}

###############################################################################
# Build the toolbar, in text and/or image mode
###############################################################################
proc build-toolbar {} {
	global w g opts

	# Create the toolbar AND the dynamic (reusable) Bookmark popup menu
	set w(bkmenu) [menu [set tb [frame $w(toolbar) -bd 0]].bkmenu]

	# Remember: ORDER OF CONSTRUCTION prescribes focus-Tabbing sequence ...
	#  (so, get any non-focusable yet needed separators/labels out of the way)
	foreach nam { 1 2 3 4 5 6 } { toolsep $tb.sep$nam }
#### MCP
#	set w(navLbl) [label $tb.navLbl -pady 0 -bd 2 -relief groove -text "Diff:"]
#	set w(mrgLbl) [label $tb.mrgLbl -pady 0 -bd 2 -relief groove -text "Merge:"]
#	set w(bkmLbl) [label $tb.bkmLbl -pady 0 -bd 2 -relief groove -text "BkMark:"]
	set w(navLbl) [label $tb.navLbl -pady 0 -text "Diff:"]
	set w(mrgLbl) [label $tb.mrgLbl -pady 0 -text "Merge:"]
	set w(bkmLbl) [label $tb.bkmLbl -pady 0 -text "BkMark:"]
#### MCP

	# The combo box
	set w(combo) $tb.combo
	::combobox::combobox $w(combo) -bd 1 -editable 0 -width 20 -command moveTo
	#  (do these NOW (no point in kludging up 'setBind'); FIXES focus-Tabbing)
		bind $w(combo) <<PrevWindow>> "[bind all <<PrevWindow>>] ; break"
		bind $w(combo) <<NextWindow>> "[bind all <<NextWindow>>] ; break"

	# Next, the simple BUTTONS (table driven enforces visual/naming uniformity)
	#	Using a "factory" approach cuts down on the code verbosity somewhat.
	# (N.B> See "ANNOYANCE" below for details on 'TW' field)
	foreach {nam  txt TW cmd  tip} {
		rediff	 "Rediff"	  40 {reCalcD user}
							 {"Recompute and redisplay ALL difference regions"}
		ignCDR	 "Ignore CDR" 76 {ignore-hunk}	 {"Ignore Current diff region"}
		splitCDR "Split..."	  44 {splcmbDlg 0}
									  {"Split Diff region at specified bounds"}
		cmbinCDR "Combine..." 76 {splcmbDlg 1}
							  {"Combine Diff region with ADJACENT neighbor(s)"}
		find	 "Find..."	  44 {srch-text}
									 {"Search for a string within either file"}
		firstCDR "First"	  34 {move first}	  {"Move to First Diff region"}
		lastCDR	 "Last"		  34 {move last}	   {"Move to Last Diff region"}
		prevCDR	 "Prev"		  34 {move -1}	  {"Move to Preceding Diff region"}
		nextCDR	 "Next"		  34 {move	1}	  {"Move to Following Diff region"}
		ctrCDR	 "Center"	  46 {centerCDR}	 {"Center Current Diff region"}
		bkmSet	 "Set"		  26 {bkmark creat}			  {"Bookmark this CDR"}
		bkmRls	 "Clear"	  34 {bkmark erase}		{"Clear this CDR bookmark"}
	  } {
		set_tooltips [set	 w(${nam}_im) \
			[toolbutton	 $tb.${nam}_im -image ${nam}Img	   -command $cmd]] $tip

		# TK ANNOYANCE: '-width -1' CLAIMS to produce a minimal btn WIDTH ...
		#
		#	Instead it does EXACTLY the same as not specifying ANY at all: just
		#	USES an 'average width' TIMES the #-of-Lbl-chars which wastes TONS
		#	of space - Even MORE SO w/proportional fonts!
		#
		# We will CONSTRAIN it OURSELF by injecting an extra frame WE control
		# (mildly ugly approach, but SHOULD work multi-platform: +/- ?fonts?)
		# (We DO the same for -height, but thats just a constant 22 pixels)
		set_tooltips [set	 w(${nam}_tx) \
			[toolbutton [frame $tb.${nam}_tx -width $TW -height 22 -bd 0].btn \
												-text $txt -command $cmd]] $tip
	}

	# The remaining items (managing of EXISTING bookmarks) dont need Tooltips
	#	(next two lines forms a ?'mini-widget'?: a scrollable frame of widgets)
	#	(with the FOLLOWING two being its 'scroller btns')
	set w(bkmCvs) [canvas $tb.bCvs -height 22 -xscrollcom {bkmark set}	   \
													-bd 0 -highlightthick 0]
	set w(bkmSF)  [frame  $w(bkmCvs).f -bd 0]
	#	N.B> NEVER supply any dimensions to w(bkmSF)!! (details in 'bkmark')

	set w(bkmSL)  [button $tb.bSL -image arroWl -command {bkmark scroll -1}]
	set w(bkmSR)  [button $tb.bSR -image arroWr -command {bkmark scroll	 1}]

	# Finally - INSERT our widget-fillable frame INSIDE the scrollable canvas
	$w(bkmCvs) create window 0 0 -anchor nw -window $w(bkmSF)

	# Last, do the RADIO buttons	 (N.B> are NOT part of focus-Tabbing)
	# A 2nd Factory is easier as each requires a specific extra term ('val').
	#	Focus-Tabbing is DISALLOWED (too easy to accidently toggle them that
	#	way and NOT notice it) so they have no effect on the focus sequence
	#	Besides - hotkeys exist for them anyway - so keyboard remains viable
	# N.B> Somehow UNAFFECTED by text-style oversizing (???) Whatever....
	foreach {nam val txt cmd tip} {
		mrgC1	1 "L"  {do-merge-choice 1}
									{"select the diff on the left for merging"}
		mrgC2	2 "R"  {do-merge-choice 2}
								   {"select the diff on the right for merging"}
		mrgC12 12 "LR" {do-merge-choice 12}
						 {"select the diff on the left then right for merging"}
		mrgC21 21 "RL" {do-merge-choice 21}
						 {"select the diff on the right then left for merging"}
	  } {
		set_tooltips [set	 w(${nam}_im) \
			[radiobutton $tb.${nam}_im -variable g(toggle) -takefocus 0 \
				-image ${nam}Img -indicatoron 0 -selectcolor $w(selcolor) \
				-value $val -command $cmd]] $tip

		set_tooltips [set	 w(${nam}_tx) \
			[radiobutton $tb.${nam}_tx -variable g(toggle) -takefocus 0 \
				-text  $txt		 -indicatoron 1 \
				-value $val -command $cmd]] $tip
	}

	# Assemble each piece WHERE it belongs,
	#	choosing the Txt/Img variations as needed,
	cfg-toolbar true
}

###############################################################################
# By default, (Re-)Populate Toolbar w/preferred button styling (IFF misaligned)
#	(BUT when init==1): COMPOSE & MAP the Toolbar widgets where they belong
###############################################################################
proc cfg-toolbar {{init 0}} {
	global w opts

	# (shorten some NEEDED variables, and provide a meta-pgming translation)
	lassign "$w(toolbar) $opts(toolbarIcons) _tx _im"	tb I btn(0) btn(1)

	# Generally, we only need to SWAP OUT certain toolbar items in response
	# to the user toggling a preference (txt .vs. iconic buttons) BECAUSE
	# the 'grid' by default REMEMBERS all the items we ever put into it ...

	if {$init} {
		# ...BUT - if this IS the VERY FIRST invocation we must 'grid' it ALL
		#	(plus make any final 1-TIME adjustments to various specific items)

		#  "grid" makes this marginally harder because its ugly to config items
		# one-by-one AND get everything in ONE row (unlike "pack"), but we NEED
		# "grid" because it "remembers" where an item WAS if we simply UNMAP it.
		#	Worse is we need CERTAIN items (toolbuttons) to be "stacked" into the
		# SAME grid-CELL so we can TOGGLE which variant of each IS mapped.
		#
		# So this may look weird as code - but dont argue with success!
		# First, list it all out (in left-to-right order): the ENTIRE Toolbar !
		#	- BUT (for now) ONLY as their "image" identities... THEN "grid" it,
		#	AND THEN "grid" it AGAIN (after switching to their "text" forms)
		#
		# This should result in 'grid' KNOWING all of them, AND stacking BOTH
		# versions (_tx & _im) of any such toolbuttons INTO the SAME grid-CELL.
		set theRow	[list $tb.combo $tb.sep1 $tb.rediff_im	  $tb.ignCDR_im	  \
			$tb.splitCDR_im $tb.cmbinCDR_im	 $tb.find_im	  $tb.sep2		  \
			$tb.mrgLbl		$tb.mrgC12_im	 $tb.mrgC1_im	  $tb.mrgC2_im	  \
			$tb.mrgC21_im	$tb.sep3		 $tb.navLbl		  $tb.firstCDR_im \
			$tb.lastCDR_im	$tb.prevCDR_im	 $tb.nextCDR_im	  $tb.sep4		  \
			$tb.ctrCDR_im	$tb.sep5		 $tb.bkmLbl		  $tb.bkmSet_im	  \
			$tb.bkmRls_im	$tb.sep6		 $tb.bSL $tb.bCvs $tb.bSR]

		grid {*}$theRow -padx 0 -sticky w
		# N.B> Phooey! The SIMPLE DIRECT way out didn't (QUITE yet) work.....
		#	(see last line of this code-block: was FORMERLY the NEXT line!)
		#
		# It's NOT that it won't DO the OVERLAY, its that when mentioning a
		# KNOWN slave, it fails to 'increment' the COL for it as we imagined...
		#		*BUT* ... there's ANOTHER way (via a "set theory" operation):
		#	1st REMOVE all the common items, THEN let them be added a 2ND time
		#	to ensure any new 'OTHER' items FALL into their CORRECT columns!
		grid forget {*}[regsub -all {[^ ]+_im} $theRow {}]
		#
		# NOW we can RE-add everything AGAIN with it ALL being managed properly
		grid {*}[string map {_im _tx} $theRow] -row 0 -padx 0 -sticky w

		# Certain items (walk ALL the slaves) need just a bit more configuring
		#	(including some tiny bits of UN-configuring from just above)
		#	N.B> '-sticky' (like others) REWRITES its value - NOT merges!
		foreach item [grid slaves $tb] {
			if {[string match {*_im}	  $item]}	{ grid $item -pady 2	} \
			elseif {[string match {*_tx}  $item]}	{ grid $item -pady 2
				# Time to put the REAL textbtn inside its frame - if it IS one!
				#	(just dont LET it GRAB more space than we've ALLOWED it)
				if {[winfo exists $item.btn]} { pack propagate $item false
									pack $item.btn -fill both -expand yes } } \
			elseif {[string match {*.bCvs} $item]}	{ grid $item -sticky ew } \
			elseif {[string match  {*sep?} $item]}	{
									 grid $item -padx 2 -pady 2 -sticky nsw } \
			elseif {[string match  {*.bS?} $item]}	{
							  $item config -repeatdelay 200 -repeatinterval 300
				if {[string match	  {*R} $item]}	{
											 grid $item -padx 0 -sticky	 e	} \
				else					   { grid $item -padx 0 }			}
		}

		#	Sadly, theres no "grid $slave -column" to ASK "What col is it IN?",
		# instead yank it from the WHOLE slave-attr LIST (& mark it 'Stretchy')
		grid columnconfig $tb [lindex [grid info $tb.bCvs] 3] -weight 1

		# Initially, HIDE the Bkmark scroll-btns (they'll come/go dynamically)
		grid remove $tb.bSL $tb.bSR
		# N.B> 'falling-thru' to below WILL UNMAP the UNDESIRED button forms
	}

	# Verify we have the DESIRED toolbtn FORM (txt/img) set as visible/hidden
	# N.B> There MAY be nothing to DO (if the Icon toggle hasnt been CHANGED)
	#	(spin over ALL items partly because 'winfo' doesnt offer glob-specs)
	foreach item [winfo children $tb] {
		if {[string match {*_[it][mx]} $item]
		&& ([winfo ismapped $item] || $init)
		&& ![string match "*$btn($I)" $item] } {
			# out with the old, in with the new
			grid remove $item
			grid [string map {_tx {} _im {}} $item]$btn($I)
		}
	}
	# Pgmr: Useful in identifying toolbar spacing/occupancy/stacking issues!!
	# foreach item [grid slaves $tb] {Dbg "[grid info $item]\t<-- $item" true}

	# Ensure 'relief' on ALL toolbutton items AGREE with the CURRENT setting
	if {$opts(relief)=="flat" && $I}   { set newB 0 }  { set newB 1 }

	# BUT, Radiobuttons IGNORE relief settings if they have an image, so make
	# THEIR borderwidth = 0 if the CURRENT RELIEF is intended to be flat
	foreach wdg [concat [info comm $tb.*$btn($I)] [info comm $tb.cvs.f.*]] {
		if {[string match {*.mrgC[12]*_im} $wdg]} {
			$wdg configure -relief $opts(relief) -bd $newB -selectc $w(selcolor)
		} { $wdg configure -relief $opts(relief) }
	}

	# Changing Icon<->Txt buttons MAY affect size of the Bookmark Scroll region
	after idle bkmark adjSz -1
}

###############################################################################
# Construct the status window (a place for hints and/or SHORT messaging)
###############################################################################
proc build-status {} {
	global g w

	frame $w(status) -bd 0

	set w(statusLabel) $w(status).label
	set w(statusCurrent) $w(status).current
	set w(statusMrgL) $w(status).mrgL
	set w(statusMrgR) $w(status).mrgR

	# MacOS has a resize handle in the bottom right which will sit on top of
	# whatever is placed there. So, we add a little bit of whitespace there.
	# It's harmless, so let's just do it on all of the platforms.
	label $w(status).blank -image nullImg -width 16 -bd 1 -relief sunken

	label $w(statusCurrent) -textvariable g(statusCurrent) -anchor e \
	  -width 14 -borderwidth 1 -relief sunken -padx 4 -pady 2
	label $w(statusMrgL) -textvariable g(statusMrgL) -anchor e \
				-borderwidth 1 -compound right -image mrgC1Img -relief sunken
	label $w(statusMrgR) -textvariable g(statusMrgR) -anchor e \
				-borderwidth 1 -compound left  -image mrgC2Img -relief sunken

	label $w(statusLabel) -textvariable g(statusInfo) -anchor w -width 1 \
	  -borderwidth 1 -relief sunken -pady 2
	pack $w(status).blank -side right -fill y

	pack $w(statusCurrent) -side right -fill y -expand n
	pack $w(statusMrgR) -side right -fill y -expand n
	pack $w(statusMrgL) -side right -fill y -expand n

	pack $w(statusLabel) -side left -fill both -expand y
}

###############################################################################
# handles simulated-scroll events over the map
#  Provides 3 modes:
# B1-click (over trough) pages, B1-motion (over thumb) drags, or B2-click jumps
# Once a button is down, the mode locks and mouse X-location becomes irrelevant
###############################################################################
proc handleMapEvent {event y} {
	global g w opts

	switch -- $event {
	B1-Press {
			if {! $g(mapScrolling)} {
				set ty1 [lindex $g(thumbBbox) 1]
				set ty2 [lindex $g(thumbBbox) 3]
				if {$y >= $ty1 && $y <= $ty2} {
					# this captures the negative delta between the mouse press
					# and the top of the thumbbox. It's used so when we scroll
					# by moving the mouse, we can keep this distance constant.
					#  (this is how all scrollbars work, and what is expected)
					set g(thumbDeltaY) [expr -1 * ($y - $ty1 - 2)]
						 set g(mapScrolling) 3
				} else { set g(mapScrolling) 1 }
				# Either way, mode is set and other mouse events are locked out
			}
		}
	B2-Press {
			# Set mode and lock out other mouse events
			if {! $g(mapScrolling)} { set g(mapScrolling) 2 }
	}
	B2-Release -
	B1-Motion {
			if {$g(mapScrolling) & 2} {
				if {$g(mapScrolling) == 3} {
					incr y $g(thumbDeltaY)
				}

				# Show text corresponding to map location
				$w(acTxWdg) yview moveto [expr $y.0 / $g(mapheight).0]

				# Release our mouse event lock (B2-click completed)
				if {$g(mapScrolling) == 2} { set g(mapScrolling) 0 }
			}
		}
	B1-Release {
			show-status ""
			if {$g(mapScrolling) & 1} {
				set ty1 [lindex $g(thumbBbox) 1]
				set ty2 [lindex $g(thumbBbox) 3]
				# if we release over the trough (*not* over the thumb)
				#	just scroll by the 'size' of the thumb (eg. 1 page); ...
				# otherwise we must have been dragging the thumb and we're done
				if {$y < $ty1 || $y > $ty2} {
					# (when "syncscroll" is set, the other window will follow)
					$w(acTxWdg) yview scroll [expr {$y < $ty1 ? -1 : 1}] pages
				}

				# Release our mouse event lock (B1 click/drag completed)
				set g(mapScrolling) 0
			}
		}
	}
}

# makes a toolbar "separator"
proc toolsep {w} {
	label $w -image [image create photo] -highlightthickness 0 -bd 1 -width 0 \
	  -relief groove
	return $w
}

proc toolbutton {w args} {
	global opts

	# create the button
	# Dflts for '-bd' AND '-pady' =1 (generally whats wanted anyway; don't set)
	button $w {*}$args

	# add minimal tooltip-like support
	bind $w <Enter> [list toolbtnEvent <Enter> %W]
	bind $w <Leave> [list toolbtnEvent <Leave> %W]
	bind $w <FocusIn> [list toolbtnEvent <FocusIn> %W]
	bind $w <FocusOut> [list toolbtnEvent <FocusOut> %W]

	$w configure -relief $opts(relief)

	return $w
}

# handle events in our fancy toolbuttons...
proc toolbtnEvent {event w {isToolbutton 1}} {
	global g opts

	switch -- $event {
	"<FocusIn>" -
	"<Enter>" {
			showTooltip button $w
			if {$opts(fancyButtons) && $isToolbutton && [$w cget -state] == \
			  "normal"} {
				$w configure -relief raised
			}
		}
	"<FocusOut>" -
	"<Leave>" {
			set g(statusInfo) ""
			if {$opts(fancyButtons) && $isToolbutton} {
				$w configure -relief flat
			}
		}
	}
}

###############################################################################
# move the map thumb to correspond w/current shown text (just like a scrollbar)
###############################################################################
proc map-move-thumb {y1 y2} {
	global g w

	# Scale the thumb height (subject to a minumum size big enough to 'grab')
	set thumbheight [max $g(thumbMinHeight) \
									[expr {round(($y2-$y1) * $g(mapheight))}]]

	# L/R edge positions (-3 so right edge remains INSIDE our border)
	set x1 0
	if {[info exists g(mapwidth)]} {
		set x2 [expr {$g(mapwidth) - 3}]
	} {set x2 0}

	# B/T edge positions (-2 so bottom edge remains INSIDE our border)
	#	but ensure top edge wont exceed the top of the map itself
	# N.B> Computationally, we want to POSITION the thumb by its CENTER
	#	such that when the thumbheight hits minimum, the EXCESS spreads to
	#	BOTH edges (unless either edge encroaches its window limit).
	set y1 [max 0 [expr {int((($y1 + $y2)/2)*$g(mapheight))-($thumbheight/2)}]]
	set y2 [expr {$y1 + $thumbheight}]
	if {$y2 > $g(mapheight)} {
		set y2 [expr {$g(mapheight) - 2 }]
		set y1 [expr {$y2 - $thumbheight}]
	}

	# extra offset values for upcomming drawing trick
	set dx1 [expr {$x1 + 1}]
	set dy1 [expr {$y1 + 1}]

	# Draw two L-shapes (1 light, 1 dark) aligned for a 3d appearance
	$w(mapCanvas) coords thumbUL  $x1 $y2 $x1 $y1 $x2 $y1
	$w(mapCanvas) coords thumbLR $dx1 $y2 $x2 $y2 $x2 $dy1

	# Record bounding box (for use in event handler, eg., dragging, etc)
	set g(thumbBbox) [list $x1 $y1 $x2 $y2]
}

###############################################################################
# Attach bindings needed by each provided widget (per naming conventions)
###############################################################################
proc setBinds {args} {
	global opts

	# Assign bindings based primarily on the "role" each widget plays in the UI
	# PRIMARY are: - the major Text windows
	#			   - their matching Info Canvas
	#			   - toplevels that house the primary appl displays (no dialogs)
	# however, several MINOR windows (scrollbars, labels) having a L/R "sense";
	# EVEN the DiffMap is adequate for supporting a majority of the POPup menu.
	# All of these are valid as a bindtag (btag) so do whichever was passed-in,
	# although SOME will just be quietly IGNORED ("grip", "Ancestor file").
	#	OUR DESIRE is to permit "input command access" from most ANYWHERE.
	foreach btag $args {

		if {[string match {.client.*} $btag]} {
			# Skip client elements we dont care about:
			#		ancFile, grip, frames
			if {[string match {.client.[ag]*} $btag]
			|| [winfo class $btag]=="Frame"} { continue }

			if {[string match {*.info} $btag]} {
			# The Info widget is actually a Canvas PAIRED with a Text widget
			# In our configuration, they share equivalent Y-coords
			# (w/acceptable X) and can thus OPERATE as-if the event happened
			# OVER that Text widget. Info widgets must pretend their dblclick
			# occurred within the COMPANION "*.text" widget instead (because
			# a canvas doesn't HAVE any "line indices" to locate (but the
			# widget relative coords STILL work because of physical alignment)
				set companion [string replace $btag end-3 end "text"]
				bind $btag <Double-1> \
					[subst {moveNearest $companion xy %x %y ; break}]
				bind $btag $opts(genEdit) { do-edit %X %Y }

			} elseif {[string match {*.text} $btag]} {
			# OUR Text wdgs are (intentionally) READONLY, yet DO take keystrokes
			# First some 'built-in' specific "features" for the L/R Text windows
				bind $btag <Double-1>	  {moveNearest %W  xy	%x %y ; break}
				bind $btag <Return>		  {moveNearest %W mark insert ; break}
				bind $btag $opts(genEdit) { do-edit %X %Y }

				# Next, we want 'focus-Tabbing' to work properly everywhere ...
				#	But the default Text CLASS bindings would presume that
				#	a typed <Tab> or <Shift-Tab> should be "insert"ed instead
				#
				# So we'll reach AROUND the Text Class to the "all" bindtag where
				# 'focus-Tabbing lives, install them here, AND "break" so the
				# class rules NEVER see it. This LEAVES the Text Class unmodified
				# and thus fully functional for uses OUTSIDE the ones setup here.
				bind $btag <<PrevWindow>> "[bind all <<PrevWindow>>] ; break"
				bind $btag <<NextWindow>> "[bind all <<NextWindow>>] ; break"

				# ALL ABOUT cursor-based SCROLLING:
				# These following (assidiously named) 20 bindings are all about
				# POSITIONING the insert cursor (and MAYBE forming a selection)
				# via the keyboard. But each EXPECTS to scroll the window AS
				# NEEDED, for visual feedback (and user confirmation) which is
				# EXACTLY the same reason TYPING does (so the users SEES what
				# they are doing). Thus scrolling is tied up with TYPING as
				# well as OTHER uses.
				#	Yet our R/O widget BLOCKs TYPING - thus ALSO needs to BLOCK
				# the visualization aid ($widget see insert) it issues, because
				# one CANT POSSIBLY 'see' what HASNT been ALLOWED to HAPPEN!
				#	This becomes easier to understand when realizing that OTHER
				# forms of scrolling (scrollbar, mouse) DO NOT update the
				# insert cursor!
				# Thus were cursor-based scrolling NOT blocked, a simple key
				# touch would then cause a RADICAL scrollBACK to wherever the
				# insert cursor was last left, to ENTER a key that WE, in turn,
				# WONT LET OCCUR! And the user then asking
				#					"what the .... just happened?"
				#	Sadly, there are "precedence" issues around trying to
				# OVERRIDE the TYPING binding (<Key-Any>) WITHOUT just
				# "removing" it, which we CANT do, as we have OTHER instances
				# that truely NEED it to exist.
				#	A (classic) solution MIGHT be to CLONE the Class bindings,
				# hack them up and use the RESULT as the Class for the R/O
				# widgets. That felt dangerous at best (interactions are both
				# subtle AND widespread).
				#	OUR "solution" was to provide a ficticious widget cmd:
				#				'ALLOW'
				# bound to EXACTLY THE SAME BINDINGS as all the PERMITTED
				# movements (general positioning and selection creation) but
				# NOT to typing; binding them ALL to the widget tag (ie. just
				# AHEAD of the Class invocations but WITHOUT a 'break'). Each
				# call *ALLOW*s 'see' to NOT BE BLOCKED for 1 invocation only!
				# Thus as BOTH bindings WILL FIRE, we are SIMPLY PERMITTING
				# the Class actions to function, while BLOCKING others, ALL
				# without making ANY modification to the Class bindings!
				#	The only thing we MIGHT get *wrong* is failing to "ALLOW"
				# yet one more binding (or one too many); which is INFINITELY
				# easier to fix than hacking even FURTHER on a CLONED class
				# (endlessly?).

			# These are ALL targetted on the Arrowkeys with Mods= S/C/SC
			#  (should also note that literals always WIN against virtuals in event
			#  precedence battles ...  IE.: which are you choosing to 'Give up' ?
				# These TWENTY bindings RESTORES Key-based cursor movement (as
				# pertains to SCROLLING) in support of simple movement AND the
				# creation of "selections". Each works by issuing a FAKE widget
				# cmd to OUR 'ROfcn' PERMITTING the (implicitly following) Text
				# Class-binding 'see' request to operate normally. Think of it
				# as "arming" the Class binding to function as designed.
				#
				#	Distinction is that Class rules that ATTEMPT Ins/Del/Repl OPS
				# must also NOT have THEIR requests to 'see' permitted, thereby
				# ENSURING a **COMPLETE BLOCKAGE** of any modification effects!
				foreach A {"" Select} {
					foreach B {Next Prev} {
						foreach C {Char Line Word Para} {
							bind $btag <<$A$B$C>>	{%W ALLOW see 1}
						}
					}
					set B "Line"
					foreach C {Start End} {
							bind $btag <<$A$B$C>>	{%W ALLOW see 1}
					}
				}

			# That leaves the DiffMap+Labels, (and NOT-clickable ScrlBars) ...
			} elseif {![string match {.client.*sb}	  $btag]} {
				# ScrlBars shouldnt Dbl-click at ALL (because they will MOVE),
				# but Labels will simulate the click ASIF it was IN Textwin
				#	(because labels dont have '@x,y' addressable lines)
				if {[string match {.client.*label}	  $btag]} {
					set companion [string replace $btag end-4 end ".text"]
					bind $btag <Double-1> \
						[subst {moveNearest $companion	  xy  %x %y ; break}]
				# We let DiffMap move, because it treats the XY as a POSITION
				# within the WHOLE of the content, not an analog of the Textpos
				} {bind $btag <Double-1>  {moveNearest %W xy  %x %y ; break}}
			}

			#	...(plus the fall-thru of everything above) which ALL needs
			# the binding to provide the Popup menu (so its just EVERYWHERE)
			bind $btag <3> {show-popupMenu %X %Y}

		# Anything ELSE has to be one of the two 'toplevel' frames we use...
		# Each is assigned ALL of the GLOBALLY defined bindings (because they
		# will act REGARDLESS of which 'contained' widget holds the focus).
		# This works because EVERY widget includes ITS toplevel as a bindtag
		#	**BUT** requires the CHOICE of the bound-keys to not BE subject
		#			to exclusivity (ie. anyone using a 'break')
		} else {
			bind $btag $opts(navCntr)	   { centerCDR }
			bind $btag $opts(navNext)	   { move  1 }
			bind $btag $opts(navPrev)	   { move -1 }
			bind $btag $opts(navFrst)	   { move first }
			bind $btag $opts(navLast)	   { move last }
			bind $btag $opts(genFind)	   { srch-text }
			bind $btag $opts(genNxfile)	   { multiFile next }
			bind $btag $opts(genPvfile)	   { multiFile prev }
			bind $btag $opts(genXit)	   { do-exit }
			bind $btag $opts(genRecalc)	   { reCalcD user }
			bind $btag $opts(mrgLeft)	   { do-merge-choice 1	}
			bind $btag $opts(mrgRght)	   { do-merge-choice 2	}
			bind $btag $opts(mrgLtoR)	   { do-merge-choice 12 }
			bind $btag $opts(mrgRtoL)	   { do-merge-choice 21 }
		}
	}
}

###############################################################################
# Bookmark handler: User toolbar-btns that Jump-Move to PARTICULAR diff regions
#
#	Has a wealth of SUBCMDs (w/indiv arg fmts), including SOME to simply avoid
#	the need for yet MORE support fcns to encapsulate involved data derivations
#	-- makes the REMAINDER easier to read; Think: data-accessors w/better names
#	Thus:
#						  Note: 'slv' is a GRID MANAGED window (eg: .win.a.b.c)
#								'{mn  mx}' is a floating point value PAIR
#								most OTHER values are simply integers
#
# Primary-level (user) capabilities:
#	subcmd		  arg
#	=======	   ========
#	jump		 hNDX	   what a bookmark executes (jump to region # hNDX)
#	creat		?hNDX?	   creates	a bookmark (dflt = CDR)
#	erase		?hNDX?	   destroys a bookmark (dflt = CDR)
#	scroll		+/-num	   scroll the displaylist of bookmarks (num positions)
#	denote		 hNDX	   permit a user-specified identifier for chosen BMark
#	rptgen		 hNDX	   toggle inclusion for report generation purposes
#	eraseall			   destroys ALL bookmarks
#
# Internal-level functors:
#	mpop	  {hNDX x y}   request popup-menu for designated hunk @ X,Y
#	set			{mn mx}	   FEEDBACK from scrolled-widget stating current view
#	<hunk-id>	 hNDX	   to reconfig when hunk-ndx/hunk-id relation chgs
#	adjSz		  col	   modify config AFTER ins/del of $col by eventloop
#
# Private primitives:	   (post-event-loop data accessors) to OBTAIN:
#	winCol		  slv	   grid column within master where $slv exists
#	viewSz		  slv	   Max VIEWABLE extent of provided-slaves MASTER
###############################################################################
proc bkmark {cmd args} {
	global g w report

	# (apologies for our PRESUMPTION that 'grid info' produces a FIXED sequence)
	#  Documentation only states indices 0 & 1 of ("-in $mstr -column $N -xx...)
	#  Grid provides no OTHER means of NEEDED introspection
	switch -glob -- $cmd {
		winCol {
			# -> the grid-column index a MANAGED window object occupies
			return [lindex [grid info $args] 3]
		}
		viewSz { lassign [grid info $args] na mstr na col
			# -> pixel width of VIEWABLE $mstr area of given MANAGED slv
			return [lindex [grid bbox $mstr $col 0] 2]
		}
		adjSz { lassign $args col
			# Things to account for with the addition/loss of a Bmark button
			#	(can involve grid configuration AND/OR scrolling considerations)
			# N.B> when called with "col<0", only re-analyzes for being resized
			# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
			# As a rule, you MUST NOT USE any of the above 'primitives' UNTIL
			#	giving the eventloop a chance to spin AGAINST the changed item!
			# THATS why THIS subcmd is BEST invoked by "after idle"!

			# Make certain the scrolling canvas TRACKS where the last btn ENDS
			#	(but also SNAG just that now NEW width value for further use)
			$w(bkmCvs) configure -scrollregion [concat 0 0					  \
										  [set i [winfo reqwidth  $w(bkmSF)]] \
												 [winfo reqheight $w(bkmSF)]]

			# Check if NOW is the time to toggle BOTH scroller btns visibility
			if {[winfo ismapped $w(toolbar).bSL]} {
				if {$i < ([bkmark viewSz $w(bkmCvs)]						  \
								  + (2 * [winfo reqwidth $w(toolbar).bSL])) } {
					grid remove $w(toolbar).bSL $w(toolbar).bSR
				}
			} elseif {$i > [bkmark viewSz $w(bkmCvs)]} {
					grid		$w(toolbar).bSL $w(toolbar).bSR
			}

			# When 'col' is INTERIOR to the known grid, there is NOW a "gap" in
			# the COL numbering (because a btn WAS destroyed). Close that up
			# by shifting grid	column assignments DOWNWARD of any ABOVE 'col'
			#	(just like the DISPLAY did when it slid the graphics over)
			if {$col >= 0 && $col < [lindex [grid size $w(bkmSF)] 0] - 1} {
				foreach wdg [grid slaves $w(bkmSF)] {
					if {$col < [set j [bkmark winCol $wdg]]} {
						grid $wdg -column [incr j -1]
					}
				}
			}
		}
		jump { set hNDX $args
			# What the button actually does... GO there
			move $hNDX 0 1
		}
		scroll {
			# For now -'units' is 1 button-widget size at a time
			#	(unsure if something smaller ?1/4 btn? and FASTER is better)
			$w(bkmCvs) xview $cmd $args units
		}
		set { lassign $args Sfrac Efrac
			# Simply use scrolling feedback to (en/dis)able indiv scroller btns
			# - ensures auto-repeat STOPS firing when max travel is reached!
			#	(uses 'catch' to map all the fractional junk to 'normal')
			set L(0.0) [set R(1.0) disabled] ; # <- keyed values that matter

			foreach {wdg val} "$w(bkmSL) $Sfrac $w(bkmSR) $Efrac" {
				if {[catch "set need \$[string index $wdg end]($val)"]} {
					set need normal
				}
				if {"$need"!= [$wdg cget -state]} {$wdg configure -state $need}
			}
		}
		creat { if {$args=={}} {set hNDX $g(pos)} {lassign $args hNDX}
			# Make a whole new bookmark
			if {! [winfo exists [set wdg $w(bkmSF).mark[hunk-id $hNDX]]]} {

				# Major graphic trick - Bookmark is both an image AND text...
				#	Image is a graphics MASK with strategic transparent holes.
				#	Text lays ON TOP of it. Size is CLAMPED per the image such
				#	that Bgnd color ONLY shows thru holes, where not overlayed
				#	by the text. Allows us to differentiate via color if/when
				#	we so choose (likely in the future)
				toolbutton [frame $wdg -width [image width bkmImg]		-bd 0 \
								  -height	[image height bkmImg] ].btn -bd 0 \
							 -compound center -image bkmImg -text "\[$hNDX\]" \
							 -default disabled -command "bkmark jump $hNDX"	  \
																 -bg \#40d040
				# Locking the frame size HALTS button from appealing for MORE
				# (excessive) text space, and only THEN can we shove the button
				pack propagate $wdg false	   ; # inside its EXACT sized jail!
				pack $wdg.btn

				# Doing Tooltip THIS way wont PopUp: simply reports as 'Status'
				#	N.B> future possibility:
				#		Should we WANT a 2nd category of bookmark, we only need
				#		to change its -bg color, and perhaps call 'set_tooltip'
				#		to OVERRIDE the 'statusbar binding' with a 'popup' one
				set g(tooltip,$wdg.btn) "\[$hNDX]: Jump to this diff region"

				# EACH Bmark takes the 'NEXT' avail column	(but see cmd=erase)
				if {![set col [llength [grid slaves $w(bkmSF)]]]
				&& [winfo reqwidth $w(bkmSF)] == 1} {
					#	Apologies about weird if-test, but ONLY want this ONCE
					#	per SESSION on the VERY FIRST Bmark! We are thus USING
					#	the idea that TK originally creates the FRAME the btn
					#	was JUST created into as 1x1, UNTIL the event-loop can
					#	ACTUALLY get a chance to INSERT it (coming shortly).
					#		Note that should the user ever REMOVE the LAST btn,
					#	the FRAME actually RETAINS its THEN size (and does NOT
					#	return to the original 1x1) PRESERVING our 1-time only.
					#
					# Now we want to set up for a full btns-worth of scrolling
					# We need the ACTUAL gridded size so emulate the '-padx 1'
					$w(bkmCvs) configure -xscrollincrement					  \
											[expr {[winfo reqwidth $wdg] + 2}]
				}
				grid $wdg -row 0 -column $col -padx 1

				# Finish-up by attaching the dynamic menu hook, then assessing
				# its addition AFTER implied events (object resizings) complete
				bind $wdg.btn <Button-3> "bkmark mpop $hNDX %X %Y"
				after idle bkmark adjSz $col
			}
			update-display
		}
		erase { if {$args=={}} {set hNDX $g(pos)} {lassign $args hNDX}
			# Destroy the given bookmark
			set hID [hunk-id $hNDX]
			if {[winfo exists [set wdg $w(bkmSF).mark$hID]]} {
				if {$hID in $report(BMrptgen)} { set report(bkmYN) 0
					# Must not remain in the report content
					bkmark rptgen $hNDX
				}
				unset -nocomplain g(tooltip,$wdg.btn)
				# It makes a difference what grid COL this Bmark occupied...
				set col [bkmark winCol $wdg]
				bind $wdg.btn <Button-3> {}
				destroy $wdg
				# Finish-up AFTER implied events (object resizings) are handled
				after idle bkmark adjSz $col
			}
			update-display
		}
		eraseall {
			# Destroy ALL EXISTING bookmark(s)
			set bookmarks [info commands $w(bkmSF).mark*]
			# N.B> not to worry about "grid column' numbering -> goes to zero
			if {[llength $bookmarks] > 0} {
				foreach wdg $bookmarks {
					# Silly, but we need the frame to then KILL both IT + BTN
					if {[string match *.btn $wdg]} { continue }
					unset -nocomplain g(tooltip,$wdg.btn)
					bind $wdg.btn <Button-3> {}
					destroy $wdg
				}
				set report(BMrptgen) [list]
				after idle bkmark adjSz 0
			}
			update-display
		}
		"[0-9]*[acd]*[0-9]" { lassign $args hNDX
			# Re-config diffnum <-> hunk-id relationship (ie. Re-number hunk)
			if {[winfo exists [set wdg $w(bkmSF).mark$cmd]]} {
				$wdg.btn config -text "\[$hNDX]" -bd 1 -pady 1				  \
												   -command "bkmark jump $hNDX"
				set g(tooltip,$wdg.btn) \
								   [regsub {[0-9]+} $g(tooltip,$wdg.btn) $hNDX]
				bind $wdg.btn <Button-3> "bkmark mpop $wdg $hNDX %X %Y"
			}
		}
		denote { lassign $args hNDX
			# Ask for, then modify, the tooltip text per the users input
			set tID "tooltip,$w(bkmSF).mark[hunk-id $hNDX].btn"
			set i [string first ":" $g($tID)]
			set curVal [string range $g($tID) [expr $i+1] end]
			if [Prompt "Your annotation for Diff-region \[$hNDX]:" $curVal] {
				if {$curVal=={} && [string index $w(val.prompt) 0] != " "} {
					set w(val.prompt) " $w(val.prompt)"
				}
				set g($tID) [string replace $g($tID) $i end ":$w(val.prompt)"]
			}
		}
		rptgen { lassign $args hNDX
			# TOGGLE the participation of hNDX in a "Bkmark" report generation
			if {$hNDX != $report(bkmYN)} {
				# ALREADY present: must remove it
				set i [lsearch -exact $report(BMrptgen) [hunk-id $hNDX]]
				set report(BMrptgen) [lreplace $report(BMrptgen) $i $i]
			} else { lappend report(BMrptgen) [hunk-id $hNDX] }
		}
		mpop { lassign $args hNDX x y
			# Empty, Config, then popup, the BMark menu for the specific button
			$w(bkmenu) delete 0 end
			$w(bkmenu) add command \
							   -label "annotate" -command "bkmark denote $hNDX"
			set report(bkmYN) \
					  [expr {[hunk-id $hNDX] in $report(BMrptgen) ? $hNDX : 0}]
			$w(bkmenu) add checkbutton -variable report(bkmYN) -onvalue $hNDX \
							  -label "in-report" -command "bkmark rptgen $hNDX"
			tk_popup $w(bkmenu) $x $y
		}
	}
}

###############################################################################
# Customize the display (among other things).
#
# N.B> Editting within the 'Behavior' category REQUIRES the use of a local GRAB
# which, in turn, is used to BLOCK access to focus-Tabbing into major controls
# of the dialog (Categories, Save, and Help buttons) PREVENTING keyboard invokes
# of such controls. That same "grab aversion" CAN ALSO be caused by usage of a
# "combobox" (which MAY do a GLOBAL grab) in whichever category is then active.
###############################################################################
proc customize {} {
	global g w pref opts tmpopts

	# FIXME: It takes 2 tries to map this dialog on the Mac
	if {![Dialog NONMODAL $w(prefs)]} {
		wm title $w(prefs) "$g(name) Preferences   ([file tail $g(rcfile)])"
		wm transient $w(prefs) .
		wm	 group	 $w(prefs) .
		# we don't want the window to be deleted, just hidden from view
		wm protocol $w(prefs) WM_DELETE_WINDOW {Dialog dismiss $w(prefs)}

		# the button frame...
		#	N.B> Unusual 'takefocus' prevents the Behavior tab from letting
		#	the keyboard Tab-traverse INTO these buttons until AFTER a local
		#	grab is no longer in-force ON that particular tab-page!
		frame $w(prefs).btns -bd 0
		button $w(prefs).btns.dismiss -width 8 -text "Dismiss" \
								   -command {prefdismiss $w(prefs)}
		button $w(prefs).btns.apply	  -width 8 -text "Apply"   \
					   -command {prefapply $w(prefs).btns.apply}
		button $w(prefs).btns.save	  -width 8 -text "Save"	 \
		  -command {prefsave $w(prefs).btns.save} -takefocus \
					  {apply {wdg { return [expr {[grab current $wdg]=={}}] }}}
		button $w(prefs).btns.help	  -width 8 -text "Help"	 \
		  -command {help-prefs}					  -takefocus \
					  {apply {wdg { return [expr {[grab current $wdg]=={}}] }}}

		pack $w(prefs).btns			-side bottom -fill x
		pack $w(prefs).btns.dismiss -side right -padx 10 -pady 5
		pack $w(prefs).btns.help	-side right -padx 10 -pady 5
		pack $w(prefs).btns.save	-side right -padx 1	 -pady 5
		pack $w(prefs).btns.apply	-side right -padx 1	 -pady 5

		# a series of radiobuttons to act as a poor mans notebook tab
		frame $w(prefs).notebook -bd 0
		pack $w(prefs).notebook -side top -fill x -pady 4

		# The relief makes these work, so we don't need to use the selcolor
		# Radiobuttons without indicators look rather sucky on MacOSX,
		# so we'll tweak the style for that platform
		#	These are also subject to non-Tab-traversal when grab is active
		set indicatoron [expr {$w(wSys) == "aqua"}]
		foreach page {General Display Behavior Appearance Engine} {
			set frame $w(prefs).f$page
			set rb $w(prefs).notebook.f$page
			radiobutton $rb -command "setPrefPage $frame" -selectcolor $w(bgnd)\
			  -variable g(prefPage) -value $frame -height 2 -text $page \
			  -indicatoron $indicatoron -borderwidth 1 -takefocus \
					  {apply {wdg { return [expr {[grab current $wdg]=={}}] }}}

			pack $rb -side left

#### MCP
#			frame $frame -bd 2 -relief groove -width 400 -height 300
			frame $frame -width 400 -height 300
#### MCP
		}

		# This Pref is supportted internally,
		# yet won't give the user a way to directly edit (right now, anyway).
		# Still, we need to ensure tmpopts knows about it
		set tmpopts(customCode) $opts(customCode)

		# General
		# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
		set frame $w(prefs).fGeneral
		set row 0
		foreach key {diffcmd xcludeFils tmpdir editor ignoreRegexLnopt
														filetypes geometry } {
			label $frame.l$row -text "$pref($key): " -anchor w
			set tmpopts($key) $opts($key)
			if {$key == "ignoreRegexLnopt" || $key == "filetypes"} {
				::combobox::combobox $frame.e$row -listvar tmpopts($key) \
							-width 50 -command "editLstPref $key"
			} elseif {$key=="diffcmd"} {
				entry [set W $frame.e$row] -width 50 -bd 2 -relief sunken	  \
								  -textvariable tmpopts($key) -state readonly \
								  -validate key -vcmd "verify alertD $key $W 1"
				# Added to FIND key->widget (when needed later)
				# N.B> 'egnOpt' is meaningless but simplifies parsing later
				dict set w(prefD) $key "0 egnOpt $W {}"

				# We also want a SPECIAL binding for when the mouse HOVERS to
				# show us the CURRENT value, in those circumstances where it
				# is warning of being overwritten....
				bind $W <Enter> "if {\$tmpopts($key)!=\$opts($key)} {\
						   show-status \"CURRENT Diff command: \$opts($key)\"}"
				bind $W <Leave> "if {\$tmpopts($key)!=\$opts($key)} {\
													 show-status {} }"
			} else {
				entry $frame.e$row -width 50 -bd 2 -relief sunken \
												  -textvariable tmpopts($key)
			}

			# Declare "-state" on some of these as slaved to other controls
			if {[string match {*opt} $key]} {
				linkState $frame.e$row tmpopts([string range $key 0 end-3])
			}

			grid $frame.l$row -row $row -column 0 -sticky w -padx 5 -pady 2
			grid $frame.e$row -row $row -column 1 -sticky ew -padx 5 -pady 2

			incr row
		}
		# Some of these labels are long, we'll need font measure
		set Fnt [$frame.l[expr {$row-1}] cget -font]

		# this is just for filler...
		label $frame.filler -text {}
		grid $frame.filler -row $row
		incr row

		# Option fields
		# Note that the order of the list is used to determine the layout.
		# So, if adding something to the list pay attention to what it affects.
		#
		# Remaining layout is a 2-col, row-major order (ie. cols vary fastest)
		#		an 'x' means an empty column; a '-' means an empty row
		# (Note: rows must be fully filled - even if that means a trailing 'x')
		set col 0
		foreach key { ignSuprs autocenter - ignoreEmptyLn autoselect -
				ignoreRegexLn autoSrch syncscroll scmPrefer - predomMrg x} {

			if {$key != "x"} {
				if {$key == "-"} {
					frame $frame.f${row} -bd 0 -height 4
					grid $frame.f${row} -row $row -column 0 -padx 20 -pady 4 \
													 -columnspan 2 -sticky nsew
					set col 1 ;# forces NEXT column to zero and increments row
				} else {
					set tmpopts($key) $opts($key)
					if {"$key" == "scmPrefer"} {
						set f [frame $frame.c${row}$col -bd 0]
						label $f.l -text "$pref($key): " -anchor w
						pack  $f.l -side left
						# Hmm, annoying - we need two of these, but want to
						# treat the value as the list of BOTH -
						#	this'll take some work
						# FIXME: the spinbox widget is cut off on the Mac,
						# even with -width increased
						foreach {val} {0 1} {
							 # set it to reassemble values when EITHER changes
							 spinbox $f.s$val -width 7 -repeatinterval 400 \
									 -values [list None {*}$g(scmS) Auto]  \
									 -command "apply {{ndx v} {
									  global tmpopts
									  lset tmpopts($key) \$ndx \$v
									 }} $val %s" -state readonly
							 eval $f.s$val set [lindex $tmpopts($key) $val]
							 pack $f.s$val -side top
						}
					} elseif {"$key" == "predomMrg"} {
						set f [frame $frame.c${row}$col -bd 0]
						label $f.l -text "$pref($key): " -anchor w
						pack  $f.l -side left
						foreach {nam val} {Left 1 Right 2} {
							 radiobutton $f.r$val -text $nam -value $val  \
													-variable tmpopts($key)
							 pack $f.r$val -side left
						}
					} else {
						set W [checkbutton $frame.c${row}$col -indicatoron 1  \
								   -text "$pref($key)" -variable tmpopts($key)]

						# Each 'ign(ore...)' toggle CAN cause a RedoDiff case
						# Alert via 'inform' color hiliting to that widget.
						#	Also re-encode the ONvalues as NON-OVERLAPPED bits
						#	attaching the check-cmd & record in the dictionary
						# N.B> weird 'switch' VALUE-test means DONT DO w/B==0
						#	  (B==1 is RESERVED(key==diffcmd) not handled here)
						if {[switch -glob $key {
							ign*E*Ln { set B 8 }
							ign*R*Ln { set B 4 }
							ignS*	 { set B 2 }
							default	 { set B 0 }
						}]} { $W config -command "verify alertD $key $W $B"	  \
																	-onvalue $B
							  # Added to FIND key->widget (when needed later)
							  # N.B> 'egnOpt' is meaningless but helps parsing
							  dict set w(prefD) $key "0 egnOpt $W {}"
						}
					}
					grid $frame.c${row}$col -row $row -column $col -sticky w  \
																		-padx 5
				}
			}
			if {![set col [expr {$col ? 0 : 1}]]} { incr row }
		}

		# The bottom row and right col should stretch to take up any extra room
		grid columnconfigure $frame 0 -weight 0
		grid columnconfigure $frame 1 -weight 1
		grid rowconfigure $frame $row -weight 1

		# pack this window for a brief moment, and compute the window
		# size. We'll do this for each "page" and find the largest
		# size to be the size of the dialog
		pack $frame -side right -fill both -expand y
		update idletasks
		set maxwidth [winfo reqwidth $w(prefs)]
		set maxheight [winfo reqheight $w(prefs)]
		pack forget $frame

		# Appearance
		# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
		set frame $w(prefs).fAppearance
		set row 0
		foreach key {textopt difftag deltag instag chgtag currtag bytetag
														inlinetag overlaptag} {
			set tmpopts($key) $opts($key)
			label $frame.l$row -text "$pref($key): " -anchor w
			entry $frame.e$row -textvariable tmpopts($key) -bd 2 -relief sunken

			grid $frame.l$row -row $row -column 0 -sticky w	 -padx 5 -pady 2
			grid $frame.e$row -row $row -column 1 -sticky ew -padx 5 -pady 2

			incr row
		}

		# tabstops are placed after a little extra whitespace, since it is
		# slightly different than all of the other options (ie: it's not
		# a list of widget options)
		frame $frame.sep$row -bd 0 -height 4
		grid $frame.sep$row -row $row -column 0 -stick ew -columnspan 2		\
															  -padx 5 -pady 2
		incr row

		set key "tabstops"
		set tmpopts($key) $opts($key)
		label $frame.l$row -text "$pref($key):" -anchor w
		entry $frame.e$row -textvariable tmpopts($key) -width 3 \
			-bd 2 -relief sunken -vcmd {verify integer %P %S} \
				 -validate key -invcmd "verify revert %W $key %s"
		grid $frame.l$row -row $row -column 0 -sticky w -padx 5 -pady 2
		grid $frame.e$row -row $row -column 1 -sticky w -padx 5 -pady 2

		incr row
		# Option fields
		# Note that the order of the list is used to determine the layout.
		# So, if adding something to the list pay attention to what it affects.
		#
		# Remaining layout is a 2-col, row-major order (ie. cols vary fastest)
		#		an 'x' means an empty column; a '-' means an empty row
		# (Note: rows must be fully filled - even if that means a trailing 'x')
		set col 0
		foreach key {inform adjcdr mapins mapchg mapdel mapolp} {

			if {$key != "x"} {
				if {$key == "-"} {
					frame $frame.f${row} -bd 0 -height 4
					grid $frame.f${row} -row $row -column 0 -padx 20 -pady 4 \
													 -columnspan 2 -sticky nsew
					set col 1 ;# forces NEXT column to zero and increments row
				} else {
				   # button 'active' bg shows color as contrasted w/Txt fg
				   set tmpopts($key) $opts($key)
				   set b $frame.b${row}$col
				   button $b -text "$pref($key)" -activebackgr $tmpopts($key) \
									-activeforeground [$w(LeftText) cget -fg] \
					   -command [expr {"$key"!="inform" ? "clrpick $b $key" : \
					  "clrpick $b $key;verify alertD pushtomenu $w(viewMenu)"}]

				   grid $b -row $row -column $col -sticky ew -padx 5 -pady 2
				}
			}
			if {![set col [expr {$col ? 0 : 1}]]} { incr row }
		}

		# The bottom row and right col should stretch to take up any extra room
		grid columnconfigure $frame 0 -weight 0
		grid columnconfigure $frame 1 -weight 1
		grid rowconfigure $frame $row -weight 1

		pack $frame -side right -fill both -expand y
		update idletasks
		set maxwidth [max $maxwidth [winfo reqwidth $w(prefs)]]
		set maxheight [max $maxheight [winfo reqheight $w(prefs)]]
		pack forget $frame

		# Display
		# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
		set frame $w(prefs).fDisplay

		# Option fields
		# Note that the order of the list is used to determine the layout.
		# So, if adding something to the list pay attention to what it affects.
		#
		# Remaining layout is a 2-col, row-major order (ie. cols vary fastest)
		#		an 'x' means an empty column; a '-' means an empty row
		# (Note: rows must be fully filled - even if that means a trailing 'x')
		set row 0
		set col 0
		foreach key { toolbarIcons fancyButtons - showln tagln - showcbs tagcbs
					  - showmap colorcbs - tagtext showinline1 x showinline2
					  showlineview inlSuprs - x fLMmax } {
			if {$key != "x"} {
				if {$key == "-"} {
					frame $frame.f${row} -bd 0 -height 4
					grid $frame.f${row} -row $row -column 0 -padx 20 -pady 4 \
												   -columnspan 2 -sticky nsew
					set col 1 ;# forces NEXT column to zero and increments row
				} else {
					set tmpopts($key) $opts($key)
					if {$key=="fLMmax"} {
						scale $frame.c${row}$col   -orient horizontal -tick 0 \
								-from 1 -to 25 -var tmpopts($key)			  \
								-label $pref($key) -length [font measure $Fnt " $pref($key) "]
						# N.B> '-length' mitigates stupid label truncation!
					} elseif {$key=="inlSuprs"} {
						# We need several buttons COMBINING into a single value
						#	this'll take some work
						set f [frame $frame.c${row}$col -bd 0]
						label $f.l -text "	 w/$pref($key): " -anchor w
						pack  $f.l -side top
						# We need five distinct buttons, but want to
						# treat the value as a composition of ALL -
						foreach {typ val} {
								  Case 16 Blanks 8 #Blanks 4 @TabX 2 @EOL 1} {
							checkbutton $f.b$val -text $typ -indicatoron 1	  \
										 -onvalue $val	-variable w($key$val) \
							-command "set tmpopts($key) \[pickSuprs $key $val]"
							 set w($key$val) [expr {$tmpopts($key) & $val}]
							 pack $f.b$val -side left
						}
					} else {
						checkbutton $frame.c${row}$col	 -indicatoron 1		  \
									-text "$pref($key)" -variable tmpopts($key)
					}

					 # Manage each widget EXCEPT 'fancybuttons' on MacOS 'Aqua'
					if {$key != "fancyButtons" || $w(wSys) != "aqua"} {
						grid $frame.c${row}$col -row $row -column $col -padx 5\
																	-sticky w
					}
				}
			}
			if {![set col [expr {$col ? 0 : 1}]]} { incr row }
		}

		# add validation to ensure only one of the showinline* options is set
		trace add var tmpopts(showinline1) write "monitor-inline $f"
		trace add var tmpopts(showinline2) write "monitor-inline $f"

		# The bottom row and right col should stretch to take up any extra room
		grid columnconfigure $frame 0 -weight 0
		grid columnconfigure $frame 1 -weight 1
		grid rowconfigure $frame $row -weight 1

		pack $frame -side right -fill both -expand y
		update idletasks
		set maxwidth [max $maxwidth [winfo reqwidth $w(prefs)]]
		set maxheight [max $maxheight [winfo reqheight $w(prefs)]]
		pack forget $frame

		# Behavior	(aka bindings)
		# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
		set frame $w(prefs).fBehavior

		# Option fields
		# Note that the order of the list is used to determine the layout.
		# So, if adding something to the list pay attention to what it affects.
		#
		# Remaining layout is a 2-col, row-major order (ie. cols vary fastest)
		#		an 'x' means an empty column; a '-' means an empty row
		# (Note: rows must be fully filled - even if that means a trailing 'x')
		set row 0
		set col 0
		foreach key {Navigation navFrst x navLast x navNext x navPrev x navCntr
					- Merge\ Choice mrgLeft x mrgRght x mrgLtoR x mrgRtoL
					- Generic	 genEdit x genFind x genNxfile x genPvfile
					x			 genRecalc x genXit }  {
			if {$key != "x"} {
				if {$key == "-"} {
					frame $frame.f${row} -bd 0 -height 10
					grid $frame.f${row} -row $row -column 0 -padx 20 -pady 4 \
												   -columnspan 2 -sticky nsew
					set col 1 ;# forces NEXT column to zero and increments row
				} else {
					set b $frame.b${row}$col
					if {$col} {
						set tmpopts($key) $opts($key)
						label $b -text $pref($key) -takefocus 1 -relief sunken\
														  -highlightthickness 1
						bind $b <Enter>			  "getKey view %W $key"
						bind $b <ButtonRelease-1> "getKey prep %W $key"
						bind $b <Leave>			  "getKey rlse %W $key"
					} else { label $b -text $key -width 20 }
					grid $b -row $row -column $col -sticky ew -padx 5 -pady 2
				}
			}
			if {![set col [expr {$col ? 0 : 1}]]} { incr row }
		}

		# The bottom row and right col should stretch to take up any extra room
		grid columnconfigure $frame 0 -weight 0
		grid columnconfigure $frame 1 -weight 1
		grid rowconfigure $frame $row -weight 1

		pack $frame -side right -fill both -expand y
		update idletasks
		set maxwidth  [max $maxwidth  [winfo reqwidth  $w(prefs)]]
		set maxheight [max $maxheight [winfo reqheight $w(prefs)]]
		pack forget $frame


		# Engine configuration
		# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
		set frame $w(prefs).fEngine

		set row 0
		# Break page into two subframes for thw TWO engine commands we need
		# Then insert an entrybox for each into their first row
		#	We will then populate any additional options by looping over each
		set	   LF1 [labelframe $frame.lf1 -text $pref(egnCmd)]
		set	   LF2 [labelframe $frame.lf2 -text $pref(egnSrchCmd)]
		entry $LF1.e$row -relief sunken -bd 2 -textvar tmpopts(egnCmd) \
						 -validate key -vcmd "verify egnCfg $LF2 %P egnSrchCmd"
		entry $LF2.e$row -relief sunken -bd 2 -textvar tmpopts(egnSrchCmd) \
						 -validate key -vcmd "verify egnCfg $LF2 %P egnCmd"
		grid  $LF1.e$row -row $row -column 1 -columnspan 2 -sticky ew
		grid  $LF2.e$row -row $row -column 1 -columnspan 2 -sticky ew

		# Option fields
		# Note that the order of the list is used to determine the layout.
		# So, if adding something to the list pay attention to what it affects.
		#
		# Remaining layout is a 2-col, row-major order (ie. cols vary fastest)
		#		an 'x' means an empty column; a '-' means an empty row
		# (Note: rows must be fully filled - even if that means a trailing 'x')

		# (First 5 "egn*" Wdgs get a UNIQUE BIT-val 'onval' derived from $pwr2)
		foreach LF "$LF1 $LF2" {pwr2 row col keys} {
							 16 1 0 {- egnCase eopCase egnBlanks eopBlanks
									 egn#Blanks eop#Blanks egn@TabX eop@TabX
									 egn@EOL eop@EOL egnTabSiz eopTabSiz}
							 16 1 0 {- egnSCase eopSCase egnSBlanks eopSBlanks
									 egnS#Blanks eopS#Blanks egnS@TabX eopS@TabX
									 egnS@EOL eopS@EOL egnSTabSiz eopSTabSiz
									 egnXcludFil eopXcludFil}} {
			foreach key $keys  {
				if {$key != "x"} {
					if {$key == "-"} {
						frame $LF.f${row} -bd 0 -height 4
						grid  $LF.f${row} -row $row -column 0 -columnspan 2 \
												-padx 20 -pady 4 -sticky nsew
						set col 1 ;# forces NEXT col to zero and increments row
					} else {
						set tmpopts($key) $opts($key)
						# There are TWO SETS (Diff/Srch) of suppressions:
						# create a unique PREFIX (per set) to group their
						# values collectively
						#
						## N.B> BUNCHES of structure in the KEY NAMES:	beware!
						if {[string match {???S*} $key]
						||	[string match {???XcludFil} $key]} {
							set PfX "egnSOpt"}	{set PfX "egnOpt"}

						if {[string match {eop*} $key]} {
							# This gets weird with a 'hidden' 3rd col despite
							# the LAYOUT pretending there is only TWO
							#	(because we need a LABEL for the entry widget)
							set V [string match {*[SF]i[zl]} $key]
							label $LF.c${row}$col	-text "via cmd flag:"
							entry $LF.e${row}$col	-relief sunken	  -bd 2	  \
									-vcmd "verify $PfX $LF.c${row}0 %P %S $V" \
										   -invcmd "verify revert %W $key %s" \
										   -validate key -textvar tmpopts($key)
							grid $LF.e${row}$col -row $row -column 2 -sticky ew

						} elseif {$pwr2} {
							# The FIRST 5 checkboxes have specific $pwr2 ONvals
							set W [checkbutton $LF.c${row}$col -indicatoron 1 \
									 -text $pref($key) -variable w($PfX$pwr2) \
							   -onvalue $pwr2 -command "pickSuprs $PfX $pwr2" ]

							# Init widget var FIRST, afterward always REFLECT
							# the value OUTWARD back to the actual key setting
							set w($PfX$pwr2) $tmpopts($key)
							trace add var w($PfX$pwr2) write "apply {{k a i o}\
										 {set ::tmpopts(\$k) \$::w(\$i)}} $key"

							# Provide access to wdg AND var for later:
							#	BECAUSE w($PfX$pwr2) is whats TIED to widget!
							#
							# Also the 'egnOpt' set of widgets must watch for
							# ALERTS indicating "diffcmd" has been ALTERRED!
							if {$PfX=="egnOpt"} {
								$W config -command [concat [$W cget -command] \
								  ";set tmpopts(diffcmd) \[formOpts egnCmd];" \
												"verify alertD $key $W $pwr2"]
								dict set w(prefD) $key "$pwr2 $PfX $W"
							}

							# Decrement PowerOf2 for nxt time: 16->8->4->2->1->0
							set pwr2 [expr {$pwr2/2}]

						} { set W [checkbutton $LF.c${row}$col -indicatoron 1 \
									 -text $pref($key) -variable tmpopts($key)]

							# Despite not being $pwr2-based, it STILL belongs
							# to its $PfX group, one of which needs more....
							if {$PfX=="egnOpt"} {
								$W config -command \
									"set tmpopts(diffcmd) \[formOpts egnCmd] ;\
													  verify alertD $key $W 0"

								# Provide access to wdg AND var for later:
								#	(bit==0 says use "$key" - NOT "$PfX")
								dict set w(prefD) $key "0 $PfX $W"
							}
						}

						# Add logical predecessor link (ie. UpTree) to items
						# that can be turned OFF (supports dynamic hiliting)
						if {$PfX=="egnOpt" && [string match {egn*} $key]} {
							dict lappend w(prefD) $key ignSuprs
						}

						grid $LF.c${row}$col -row $row -column $col -sticky w \
																	-padx {5 0}
					}
				}
				if {![set col [expr {$col ? 0 : 1}]]} { incr row }
			}
		}

		# Collect ALL the keys, for which 'ignSuprs' was set as its UpTree link
		# and append THEM onto the value for 'ignSuprs' (eg. DownTree links)
		dict lappend w(prefD) "ignSuprs" \
						 [dict keys [dict filter $w(prefD) value {* ignSuprs}]]

		# Establish page layout compliance w/optional widgets BEFORE we measure
		$LF2.e0 validate

		# Assemble the labelframes into the page
		grid $LF1 -row 0 -column 0 -padx 8 -pady 8 -sticky ew
		grid $LF2 -row 1 -column 0 -padx 8 -pady 8 -sticky ew

		# Stretch last column (horiz) of labelframes to use extra space from...
		grid columnconfigure $LF1	2 -weight 1
		grid columnconfigure $LF2	2 -weight 1

		# ... whatever space THEY get passed from the surrounding page frame
		grid columnconfigure $frame 0 -weight 1

		pack $frame -side right -fill both -expand y
		update idletasks
		set maxwidth  [max $maxwidth  [winfo reqwidth  $w(prefs)]]
		set maxheight [max $maxheight [winfo reqheight $w(prefs)]]
		pack forget $frame

		# # # # # # # Assemble all the page/Tabs and Display one  # # # # # # #
		setPrefPage [set g(prefPage) $w(prefs).fGeneral]

		# compute a reasonable FIRST location for the window...
		centerWindow $w(prefs) "$maxwidth $maxheight"
	}

	# Config Dialog RE-display: BEGIN with NO alert (USUALLY agrees at start)
	# THEN reinstate alert (via validation) if it was DEFERRED from last time
	if {[info exists g(deferD)]} {
		foreach key $g(deferD) {
			switch -glob $key {
			ign*E*Ln { set B 8 }
			ign*R*Ln { set B 4 }
			ignS*	 { set B 2 }
			diffcmd	 { set B 1 }
			default	 { set B 0 }
			}
			verify alertD $key [lindex [dict get $w(prefD) $key] 2] $B
		}
		unset g(deferD)
		Dbg "REINSTATED hilite: alertD STILL [dict get $w(prefD) alertD]" 1
	} { dict set w(prefD) alertD 0 }

	# Too helpful to whack - can FORCE simply by PREFIXing to it: 1$g(debug)
	if {[set prefDbg $g(debug)]} {
		Dbg "PREF DICT:" $prefDbg
		foreach {key data} $w(prefD) { Dbg "$key {$data}" $prefDbg "\t" }
	}

	# FINALLY  - display it!
	Dialog show $w(prefs)
}

###############################################################################
# Hotkey-edit event handler to display/capture/decode/establish global hotkeys
#	N.B> $wdg is EITHER the 'label' or an 'entry' depending on HOW FAR the
#	edit process has progressed - pay attention to binds, focus, and grabs
#	NOTE: does NOT "apply" the new binding - that happens during 'prefapply'
###############################################################################
proc getKey {cmd wdg key args} {
	global w tmpopts opts pref

	switch $cmd {
		view	{
			# Simply SHOW the user what the binding currently is
			$wdg config -text $tmpopts($key)
			set w(savLblBg) [$wdg cget -background]
		}
		prep	{
			$wdg config -bg $opts(inform)
			# N.B> magic numbers to AVOID responding to Modifier keys (we hope)
			#	(see Tcl manpage 'keysyms' to decode values)
			bind $wdg <Key> "if {(%N<65505 || %N>65518) && (%N!=65407)} {
													getKey edit %W $key %s %K}"
			focus $wdg ;# Make sure next keystroke is SEEN by above binding
		}
		rlse	{
			# Perhaps <FocusOut> (maybe?) needs to come here as well as <Leave>
			# (ie. when we unmap it during 'edit' - or does that happen anyway?)
			$wdg config -text $pref($key) -bg $w(savLblBg)
			bind $wdg <Key> {}
		}
		edit	{
			# Hide the label widget and pop an entry widget to take its place
			#		(LOADING said entry widget with the VERY NEXT KEYSTROKE)
			#	Entry widget NAME simply appends 'E' to the Label it replaces
			set tmpopts($key) [keyMods {*}$args]
			entry ${wdg}E -bg $opts(inform) -textvar tmpopts($key)
			grid  ${wdg}E {*}[grid info $wdg]
			focus ${wdg}E		;# Trade focus to newly created widget
			grid remove $wdg	;# THEN Unmap (but dont forget) Label
			#		( N.B> this 'deactivates' its attached <Leave> binding)
			update idletask
			# Now ensure "editting" is TERMINATED if user tries to go elsewhere
			bind  ${wdg}E <Return>	 "getKey insert	 %W $key {*}$args"
			bind  ${wdg}E <Escape>	 "getKey cancel	 %W $key"
			bind  ${wdg}E <FocusOut> "getKey chkfocs %W $key {*}$args"
			bind  ${wdg}E <Button-1> "getKey chkcncl %W $key %X %Y"
			# Buttonpress is tricky - COULD depend on WHERE it occurs ...
			#	thus grab the pointer to make sure WE get to evaluate it
			#	(should be SAFE -because- current toplevel is NOT MODAL)
			grab ${wdg}E
		}
		chkfocs {
			Dbg {$cmd $key as $tmpopts($key) via $wdg}
			#	N.B> Only ${wdg}E widgets should call this subcommand
			# While not as critical for "Click-to-Type", "Focus-follows-Mouse"
			# could prematurely trigger a focus-loss in mid-edit which would
			# (due to trying to support focus-Tabbing AS an 'accept/insert'),
			# be MISTAKEN as such. Thus TRY to differentiate (if possible).
			#
			#	(TK ?? Un-documented: [focus] reports EMPTY even when mouse
			#	is moved OUT (in FfM mode) of the current Toplevel YET stays
			#	WITHIN the "." Toplevel; But WILL report a value for a NEW
			#	current Toplevel (such as the popup Help dialog): strange?!!
			#		However, as the Help dialog is then FROZEN by the grab,
			#	it becomes useless, thus we now prevent it from popping up
			#	(which obviates any need to check for and deal with it NOW)
			#
			# Whatever - so if a Focus-Tab has occurred, we SHOULD see a new
			# window reported and we can therefore ACCEPT the pending edit
			if {[set win [focus -displayof [winfo toplevel $wdg]]] != {}} {
				Dbg {  focus went to $win}
				# Presumption is user just used a [Tab] to complete the edit
				getKey insert $wdg $key {*}$args
			}
		}
		chkcncl {
			Dbg {$cmd $key as $tmpopts($key) via $wdg}
			# Decide if user is simply MOVING the entry insertcursor or DOING
			# something un/related IMPLYING we should CANCEL (or ACCEPT?) it.
			#	N.B> Only ${wdg}E widgets should call this subcommand
			#	SKIP: "Save" (is pointless); "Help" would be hung by the grab
			#	N.B> ${wdg}E MAY no longer exist (if cancelled by Dismiss here)
			if {[winfo exists $wdg]
			&&	"$wdg" != "[set win [winfo containing {*}$args]]"} {
				# Somewhere else ... must we cancel?
				if {"$win" == "$w(prefs).btns.dismiss"} {
					Dbg {  Was $win - need to cancel}
					getKey cancel $wdg $key
					$win invoke
					return -code break
				} elseif {"$win" == "$w(prefs).btns.apply"} {
					# sortof makes sense, if you think of it as a "shortcut"
					Dbg {  Was $win - need to accept}
					getKey insert $wdg $key
					$win invoke
					return -code break
				}
				# Hmmm, we COULD notice a click on a DIFFERENT bind target and
				# cancel/switch (or accept/switch) to IT, but lets NOT for now
				#	FYI - method: "event generate" TO the new widget window,
				#		but after doing WHAT (accept/cancel)?
			}
			# Everything else should be fine. If we are still within the prefs
			# toplevel, the grab is in effect (pretty sure that applies to our
			# main windows as well) so NOTHING should happen as it wont ever be
			# DELIVERED anywhere else. However, if its outside the TOOL, we
			# WONT KNOW it, because the grab was NOT global...so we simply
			# pause and wait for the user to get back to us.
		}
		cancel -
		insert {
			Dbg {$cmd $key as $tmpopts($key) via $wdg (being destroyed)}
			# N.B> Only ${wdg}E widgets should call these subcommands
			if {$cmd != "cancel"} {
				# Re-instate angle-brackets (even if user TRIED to add them)
				#	(Note: also prevents specifying virtual events)
				set tmpopts($key) "<[string map {< {}	> {}} $tmpopts($key)]>"
			} else { set tmpopts($key) $opts($key) }

			# Whack the entry widget and RESTORE the LABEL widget back in place
			#	(Derives original Label widget name FROM the given Entry widget)
			# Grab is implicitly released as its target ($wdg) is killed
			destroy $wdg
			grid [string replace $wdg end end]
			update idletask
		}
	}
}

###############################################################################
# Presumptious little routine to decode a Keypress into ALL its contributors
###############################################################################
proc keyMods {state key} {
	global w
	# List of 'power-of-2' bit masks is used to recognize the modifers:
	# N.B> Certain bit patterns (Alt) were found as platform specific (win32)?
	foreach {bit nam}  {   131072 "Alt"
			128	 "Mod5"	   64	 "Mod4"
			32	 "Mod3"	   16	 "Mod2"
			8	 "Mod1"	   4  "Control"
			2	 "Lock"	   1	"Shift" } {
		if {$state & $bit} { lappend modifiers $nam }
	}
	# MacOS doesn't seem to LIKE having 'Key' as a modifier - skip it... UNLESS
	# its a single DIGIT which WOULD be MISTAKEN as a Button; instead of a Key!
	if {$w(wSys) != "aqua" || [string match {[0-9]} $key]} {
		lappend modifiers Key $key } {
		lappend modifiers	  $key
	}

	# Platforms apparently define "preferred" names for certain modifiers
	# Some of this is platform derived, others (Aqua) was manpage derived
	set map(win32) [list "Mod1" "Num"					  "Mod3" "Scroll"]
	set map(aqua)  [list "Mod1" "Command"  "Mod2" "Option"				 ]
	set map(x11)   [list "Mod1" "Alt"					  "Mod3" "Scroll"]

	return [string map $map($w(wSys)) [join $modifiers "-"]]
}

###############################################################################
# Formulate/return the DESIGNATED Cmd w/all ACTIVE options (not incldg Files)
###############################################################################
proc formOpts {cmd} {
	global	tmpopts opts

	# Each command OPERATES from ITS distinct context despite SHARED keys
	# N.B> When ADDing keys to the Engine page, need to put them HERE as well
	set keys {}
	switch $cmd {
		egnSrchCmd { upvar 0 opts ctx
			if {[lindex $opts($cmd) 0] ne [lindex $opts(egnCmd) 0]} {
				lappend keys egnSCase eopSCase egnSBlanks eopSBlanks	\
							egnS#Blanks eopS#Blanks egnS@TabX eop@STabX \
							egnS@EOL eopS@EOL egnSTabSiz eopSTabSiz
			} else {
				# If names match then SHARE the keys/vals FROM the "egnCmd"
				lappend keys egnCase eopCase egnBlanks eopBlanks egn#Blanks \
							eop#Blanks egn@TabX eop@TabX egn@EOL eop@EOL	\
							egnTabSiz eopTabSiz
			}
			lappend keys   egnXcludFil eopXcludFil
		}
		egnCmd { upvar	0 tmpopts ctx
			if {$ctx(ignSuprs)} {
			lappend keys   egnCase eopCase egnBlanks eopBlanks egn#Blanks \
							eop#Blanks egn@TabX eop@TabX egn@EOL eop@EOL  \
							egnTabSiz eopTabSiz
			}
		}
	}

	# Certain keys need VALUES (and ANY might be config'd as "unused")
	set cmd $ctx($cmd)
	foreach {key opt} $keys {
		if {!$ctx($key)} { continue }

		# Keys w/VALUES have to worry about missing data and/or formatting
		if {[string match {*[FS]i[lz]} $key]} {
			set fmt [string match { *} $ctx($opt)] ;# seperate Val from Key?
			switch $key$fmt {
				egnSTabSiz0 -
				egnTabSiz0 { lappend cmd $ctx($opt)$ctx(tabstops) }
				egnSTabSiz1 -
				egnTabSiz1 {
					lappend cmd [string range $ctx($opt) 1 end] $ctx(tabstops)
				}
				egnXcludFil0 -
				egnXcludFil1 {
					# This is specified multiple times (once per pattern)
					foreach val $ctx(xcludeFils) {
						if {$fmt} {
							lappend cmd [string range $ctx($opt) 1 end] $val
						} { lappend cmd $ctx($opt)$val }
					}
				}
			}

		# Otherwise its JUST the optionflag all by itself
		} { lappend cmd $ctx($opt) }
	}
	return $cmd
}

###############################################################################
# Manipulating SUPPRESSION state transitions during Prefs editting, needs to
# keep 5 widget-tracked (and potentially) 1 'combined') bit values in concert
#	AND enforce the cascaded precedence arrangement of LEGAL combinations
# (N.B> Oh yeah, there are also THREE distinct SETS of 5 widgets - stay sharp!)
###############################################################################
proc pickSuprs {key bit} {
	global w tmpopts opts

	# The (fundamentally boolean) values $w(${key}N) have a powers-of-two
	# NAMING and 'ON-value's connotation as each DESCRIBEs a specific BIT

	# Bit(16) is permitted to freely toggle, irrespective of any others, as
	# may bits (1 or 2) PROVIDED the higher PRECEDENCE bits (4 or 8) ARE zero.
	# Collectively, this establishes a 3 Tier "8->4->(2 or 1)" precedence
	# ranking among ONLY the lower 4 (out of 5) bits
	#
	# Based on what Bit just CHANGED, clear any from the WRONG rankings...
	switch $bit {
	1	-
	2	{	lassign {0 0} w(${key}8) w(${key}4) }
	4	{	if {($w(${key}8) & 8)} { set w(${key}8) 0
			} { lassign {0 0} w(${key}2) w(${key}1) }
		}
	8	{	lassign {0 0 0}	  w(${key}4) w(${key}2) w(${key}1) }
	}

	# ...then simply REBUILD the combined value (returning the result).
	return [expr $w(${key}16)+$w(${key}8)+$w(${key}4)+$w(${key}2)+$w(${key}1)]
}

###############################################################################
# A collection of (various) entry widget per-keystroke data content validators
#	(most used by Preferences - but 'occupancy' is EXCLUSIVELY for NewDiff use)
# PLUS subsequent logic fragments that deal with interactions RELATED to them.
###############################################################################
proc verify {subcmd args} {
	global w tmpopts opts

	# Establish a DEFAULT return status
	set result 1
	#	(individual subcmds -SPECIFICALLY real VALIDATORS- override as needed)

	switch -- $subcmd {

		revert { lassign $args W key oldV
			# REAL Validator: (but always returns TRUE)
			# Erase keystroke from widget display (by RELOADING its -textvar)
			# N.B> expected to be invoked via "entry -invcmd ..." - MUST NOT be
			#	done until AFTER returning: '-validate none' WOULD become set!
			after idle "$W config -validate key ; set tmpopts($key) \"$oldV\""
		}

		egnSOpt -
		egnOpt { lassign $args W newV chr V
			# REAL Validator (the options themselves):
			# Only blanks are DISALLOWED (unless its the PERMITTED pad-flag)
			set result [expr {$chr!=" " || [string first " " $newV $V] <0}]
			# Passed checkbox widget is DISABLED if value is fewer than 2 chars
			if {[string length $newV] - ([string index $newV 0]==" ") < 2} {
				# Also DE-hilite widget (unneeded for egnSOpt, but harmless)
				$W deselect	 ;	verify hlit $W 1 1	;  alignState $W 0
			} {										   alignState $W 1 }
		}

		egnCfg { lassign $args LFR Vc1 vc2
			# MONITORING Validator: (returned status always TRUE)
			# Checks 1 or BOTH conditions (based on WHICH widget called):
			# - When editting the egnCmd, must reflect into "diffcmd"
			#	(AND trigger the Alert, it in turn, causes).
			if {$vc2 == "egnSrchCmd"} {
				after idle {
					set tmpopts([set Key "diffcmd"]) [formOpts egnCmd]
					verify alertD $Key [lindex [dict get $w(prefD) $Key] 2] 1
					}
			}
			# - Yet ALWAYS watches BOTH engine commands for 1st-word agreement
			# tripped by changes in 1st WORD in EITHER of the 2 Engine cmds.
			# First decide which WAY (manage -or- remove) the Option rows ...
			#	... then go DO it (basically one row at a time)
			# (kinda presumptious about names and layout), Eh...feeling lazy.
			if {[lindex $Vc1 0] ne [lindex $tmpopts($vc2) 0]} {
				foreach row {2 3 4 5 6 7} {
				  # Doing this when UNNECESSARY has no detrimental effect
				  grid $LFR.c${row}0  $LFR.c${row}1	 $LFR.e${row}1 }
			} { foreach row {2 3 4 5 6 7} {
				  # (loop lets a PREV UNmanaged state "do nothing" gracefully)
				  foreach Wdg [grid slaves $LFR -row $row] {grid remove $Wdg} }
			}
		}

		hlit { lassign $args W newV oldV
			# REAL Validator:
			# Rather simple - hilite 'alert' status of given Wdg WHEN provided
			# values DIFFER (else DE-hilite) - BUT respect and/or deal with
			# the potentially varying STATE of that widget (disabled-> no chg!)
			set result [expr {"$oldV"!="$newV"}]
			#	N.B> HOWEVER - 'state' MAY depend both on method of invocation
			#		(Keyboard/mouse) AND the defn of widget (entry/checkbutton)
			#		with regard to WHICH attribute should RECEIVE the hilite
			switch [$W cget -state] {
				readonly {
					# ENTRY widget:
					# ('disabled' color IS the 'normal BG' for a readonly)
					$W config -readonlybackground [expr {$result ? \
							 $tmpopts(inform) : [$W cget -disabledbackground]}]
				}
				active -
				normal {
					# CHECKBUTTON: 'active' occurs for mouse hover (thus click)
					# 'normal' covers most everything else regardless of Widget
					$W config -background [expr {$result ? \
												  $tmpopts(inform) : $w(bgnd)}]
				}
				default {
					# Only expected when W-state: 'disabled' (rare possibility)
					set result 0
				}
			}
		}

		alertD { lassign $args key W bit
			# MONITORING Validator: (returned status always TRUE)
			# LOTS OF WORK for mostly GUI-fluff to WARN user that the Pref edit
			# LEADS to EXECUTING Diff (which wipes any EXISTING temporal work).
			#	Hilites Apply-Btn if ANY contributors exist AND the 'W'dg
			#	itself if IT is one of those RESPONSIBLE (there can be several)
			# Implies to user both IF and WHY the Diff will occur when APPLIED
			# and affords the opportunity to NOT CAUSE such changes casually
			switch -glob $key {
				reset {
					#	Used when Apply/Dismiss-ing dialog to "start fresh" for
					# NEXT time (in lieu of destroying/rebuilding the Dialog).
					# Forcibly DE-HILITEs most everything! Does NOT ALTER ANY
					# state variables (except during a DISMISS of INLINE items)
					# N.B> Dismiss/Apply ITSELF assigns the ACTUAL Pref value
					dict for {K data} $w(prefD) { lassign $data bit PfX Wdg
						if {[string match {[ie]gn*} $K] && $PfX=="egnOpt"} {
							# Force EVERY alert hilite OFF.
							verify hlit $Wdg 0 0

						# Snag 'diffcmd' widget as it passes by (used shortly)
						} elseif {$K=="diffcmd"} { set W $Wdg }
					}
					verify hlit $W $tmpopts(diffcmd) $opts(diffcmd)

					# Marginally related: these 'hidden' widgetVars need to
					# piggyback on any RESET being performed as well.
					#	(has no effect when performing APPLY, as ALREADY set)
					foreach bit {16 8 4 2 1} {
						set w(inlSuprs$bit) [expr {$opts(inlSuprs) & $bit}]
					}

					# Last, dont forget to de-hilite the Apply button itself
					#	N.B> Associated alert-contrib Bits RESETs at RE-display
					verify hlit $w(prefs).btns.apply 0 0
					return $result
				}
				ign*R*Lnopt -
				ign*R*Ln {
					# Hilite depends on TWO (related) keys,
					# and what values EACH will ultimately have -
					#	Whats awkward is BOTH trying to HILIT the SAME widget
					# (the toggle).
					#	If any LIST items EXIST and the toggle CHANGED, - OR -
					# is PRESENTLY on and the ITEMS were changed, then HILIT;
					# Otherwise HILITE is either forced OFF *or* left unchanged
					if {[string match {*opt} $key]} {
						# (ONLY invoked when a physical LIST change occurred)
						if {$tmpopts(ignoreRegexLn)} { set OnOff \
									[verify hlit $W $tmpopts($key) $opts($key)]
						} { return $result }

						# This handles whenever the TOGGLE is flipped
					} elseif {[llength $tmpopts(${key}opt)]} { set OnOff \
									[verify hlit $W $tmpopts($key) $opts($key)]
					} { set OnOff 0 }
				}
				ign*E*Ln {
					set OnOff [verify hlit $W $tmpopts($key) $opts($key)]
				}
				pushtomenu {
					#  (really just a util fcn tangentally related to "alertD")
					# The first 3 View->menuitems need to USE the HILITE color
					# to ALSO warn of an impending Diff. Simply PUSH that color
					# into them should that color HAPPEN to become changed.
					foreach item {0 1 2} {
						$W entryconfig $item -activebackground $opts(inform)
					}
					return $result
				}
				diffcmd -
				ignSuprs {
					# When executing via "diffcmd", SOMEONE already changed it;
					# otherwise "ignSuprs" was toggled (ON-or-OFF) thus it must
					# be recomputed (which will come thru here a SECOND time).
					if {$key=="ignSuprs"} {
						set tmpopts(diffcmd) [formOpts egnCmd]
					}

					set OnOff [verify hlit $W $tmpopts(diffcmd) $opts(diffcmd)]

					# However, if "ignSuprs" was just TOGGLED Off, then it
					# BLOCKED any Engine-based CHANGES and could thus BECOME
					# the NEW reason for a different 'diffcmd'. Re-evaluate if
					# any Engine elements should therefore DE-hilite by passing
					# a FICTICIOUS keyname (which is BASICALLY a simulation of
					# a 'fall-thru' switch-case which TCL doesn't support!)
					if {$key=="ignSuprs"} { verify alertD egnOpt }
				}
				default {
					# These are specific Engine pref toggles tweaked On/Off
					# - determine if that results in a CHANGE in the alert
					# status (particularly when being turned OFF!).
					#	Must assess ALL the POSSIBLE reasons, reflecting EACH
					# status as they MAY BE inter-related!
					dict for {K data} $w(prefD) { lassign $data bit PfX Wdg PrD
						if {[string match {egn*} $K] && $PfX=="egnOpt"} {
							# Next align any alert hilites (either ON or OFF).
							# FORCE an OFF when predecessor (PrD) is ALSO OFF.
							#	N.B> 'bit' chooses WHERE TO FIND values needed
							if {$bit} { incr AnyToggled \
								[verify hlit $Wdg [expr {$tmpopts($PrD) ? \
									   $opts($K) : $w($PfX$bit)}] $w($PfX$bit)]
							} {			incr AnyToggled \
								[verify hlit $Wdg [expr {$tmpopts($PrD) ? \
									   $opts($K) : $tmpopts($K)}] $tmpopts($K)]
							}

						# Snag 'ignSuprs' widget as it passes by (need below)
						} elseif {$K=="ignSuprs"} { set W $Wdg }
					}

					# If ALL above verifys are OFF, RE-verify $PrD widget *VIA*
					# "diffcmd" (NOT "ignSuprs"!!!) so that ALL it DOES is
					# the 'verify hlit' (but CONTINUING UpTree) - BUT USE
					# the 'bit' value for ignSuprs so it removes any Apply-btn
					# hilite IFF DE-hiliting "ignSuprs" was ALSO accomplished.
					if {!$AnyToggled} { verify alertD diffcmd $W 2 }
					return $result
				}
			}

			# Weird boolean expression SETS a specific BIT per the OnOff value
			# Odder YET - 'dict update' CAUSES "isApplied" to BECOME a localVar
			# to CONTAIN that updated value - which THEN hilites the Apply btn!
			dict update w(prefD) alertD isApplied \
				"set isApplied \[expr {(\$isApplied & ~$bit)+($bit * $OnOff)}]"
			verify hlit $w(prefs).btns.apply 0 $isApplied
		}

		integer { lassign $args newV digit
			# REAL Validator:
			# Fairly simple -
			#	IF proposed value is still an integer - accept - else dont.
			#
			# N.B> allows SURROUNDING blanks UNLESS 'digit' (%S) was PROVIDED
			#	   (Note: w/o '-strict' completely EMPTY is considered VALID)
			if {$digit=={} || [string match {[0-9]} $digit]} {
				set result [string is integer $newV]

			} elseif {$digit!={} && $digit==" "} {
				set result 0 ;# caller disallows any 'padding' blanks

			} { set result [string is integer $newV] }
		}

		occupancy { lassign $args  Wdg newV
			# MONITORING Validator: (returned status always TRUE)
			# Designed for the newDiff 'revision' fields
			#	Disables the LABEL (W) when the ENTRY value (newV) is empty ...
			#	(a GUI feedback trick to discern an EMPTY .vs. BLANK field)
			#
			#	BUT also monitors the main PAIR of Rev entry-widgets to warn
			#	user about using entry widget #2 w/o using #1
			#		(because 'assemble-args' would then READ #2 as-if #1)

			# (grab instance number of widget - not applicable if none exists)
			if {[string is digit [set ndx [string index $Wdg end]]]} {
				set Other([set Other(2) 1]) 2	;#	(meta-pgm identity value)

				# Next, derive meta-addressable names for PAIRED entry widget
				append W($ndx) [file rootname $Wdg] \
											 [string map {l e} [file ext $Wdg]]
				set W($Other($ndx)) [string repl $W($ndx) end end $Other($ndx)]

				# finally adjust the bkgnd of #2 if it is USED when #1 is empty
				if {($ndx==2 && "$newV" != "" && ![$W(1) index end])  \
				||	($ndx==1 && "$newV" == "" &&  [$W(2) index end])} {
					$W(2) configure -bg		Tomato
				} { $W(2) configure -bg [$W(1) cget -bg] }
			}

			# Simple length check determines STATE of the (provided) label-wdg
			$Wdg configure -state [expr {[string length $newV] ?
													  "normal" : "disabled"}]
		}
	}

	# Each REAL validator has its OWN rules for acceptance, just return result
	return $result
}

###############################################################################
# Align status of passed widget(s) to agree with passed var($index) bool value
#	N.B> 3 variants - "...Tr..." interfaces with traced array variables (which
#	   'linkState' CREATEs by naming the Controll-ED WIDGET & Controll-ING Var)
#		 Basic version allows a LIST of widgets to be supplied for alignment
###############################################################################
proc linkState {Wdg Var} {
	uplevel trace add variable $Var write "{alignTrState $Wdg}"
}

proc alignTrState {widget name elem op} {
	upvar $name var
	alignState $widget $var($elem)
}

proc alignState {wdgLst boolean} {
	foreach W $wdgLst {
		$W configure -state [expr {$boolean ? "normal" : "disabled"}] }
}

###############################################################################
# Specialized color-picker invoked by button (feedback to specific button -bg)
###############################################################################
proc clrpick {wdg key} {
	global pref tmpopts

	set color [tk_chooseColor -initialcolor [$wdg cget -activebackground] \
					  -parent [file rootname $wdg] -title "Choose $pref($key)"]
	if {"$color" != ""} {
		$wdg configure -activebackground [set tmpopts($key) $color]
	}
}

###############################################################################
# Manage user interaction with any pref represented via a 'list of values'
###############################################################################
proc editLstPref {key args} {
	global pref tmpopts

	# Empty values simply have no effect and are ignored
	#	(we sortof use it as feedback that we "accepted" the add/delete)
	foreach {wdg value} $args {
		if {![string length "[string trim "$value"]"]} {return}
	}

	# Ugh - the combobox widget apparently has a *global* GRAB in progress ...
	#	 So we CANT really popup modal dialogs for confirmations, etc.
	#	 Instead, we will ENCAPSULATE the notices/feedback/actions to occur
	#	 *after* this callback (and combobox) are DONE (and the grab is gone)
	#
	# N.B> "subst + backslashing" is needed to resolve & embed LOCAL vars

	# Confirm requests to DELETE from the list
	if {[set ndx [lsearch -exact $tmpopts($key) "$value"]] >= 0} {
		after idle [subst {
				if {{ok} == \[tk_messageBox -type okcancel -icon question	\
				-title {Please Confirm} -parent [file rootname $wdg]		\
				-message "Remove this entry from the\n'$pref($key)' list ?" \
													   -default cancel ]}	\
				{ set tmpopts($key) \[lreplace \$tmpopts($key) $ndx $ndx];	\
							 editLstFeedback $wdg $key {	R e m o v e d} }
		}]
	} else {
		# Possibly validate the FORM of the specific entry before ADDING it
		if {"$key" == "filetypes" && [llength "$value"] != 2} {
			after idle [subst {
					tk_messageBox -type ok -title {Syntax error} -icon info \
					-parent [file rootname $wdg]  -detail {(not added)}		\
					-message "Format should be '{filetype label} .extension'"
			}]
		} else {
			after idle [subst { lappend tmpopts($key) {$value} ; \
								   editLstFeedback $wdg $key {	  A d d e d}
			}]
		}
	}
}

###############################################################################
# Pure unadulterated GUI fluff (lets user KNOW their edit was accepted)
###############################################################################
proc editLstFeedback {wdg key msg} {
	global w
	# Pretend to enter a new value (but dont let the command fire) ... then
	# 1250ms later, clear with an EMPTY value (and LET it fire w/no effect)
	$wdg configure -commandstate disabled
	$wdg configure -value "$msg"
	after 1250 "$wdg configure -commandstate normal -value {}"

	# Admittedly strange place to put this, but from a SEQUENCE point-of-view
	# it snags that the edit HAS OCCURRED which warrants FURTHER feedback.
	#	Other oddity is redirecting such feedback into a RELATED widget
	if {$key=="ignoreRegexLnopt"} { verify alertD $key [lindex \
						  [dict get $w(prefD) [string range $key 0 end-3]] 2] 4
	}
}

###############################################################################
# Emulate SEMI-radio-button behavior: only 1 can be 'on', BUT BOTH may be 'off'
#	Simultaneously adjust state of subordinate attribute widgets as well
###############################################################################
proc monitor-inline {W name index op} {
	global tmpopts

	switch $index {
	"showinline2" {
			alignState "$W.l $W.b16 $W.b8 $W.b4 $W.b2 $W.b1" $tmpopts($index)
			if {$tmpopts($index)} { set tmpopts(showinline1) 0 }
		}
	"showinline1" {
			if {$tmpopts($index)} { set tmpopts(showinline2) 0 }
		}
	}
}

###############################################################################
# Finalize packing the Preferences dialog for the largest "tab" overlay
# and designate which to actually display
###############################################################################
proc setPrefPage {which} {
	global w

	pack forget $w(prefs).fGeneral
	pack forget $w(prefs).fAppearance
	pack forget $w(prefs).fDisplay
	pack forget $w(prefs).fBehavior
	pack forget $w(prefs).fEngine
	pack $which -side right -fill both -expand y
}

###############################################################################
# Quickly spin through all the prefs and look exclusively for any that WERE
# editted, yet NEVER applied. Request permission to remove them and base the
# decision to erase them AND dismissal of the dialog on that answer
###############################################################################
proc prefdismiss {prefwin} {
	global g pref opts tmpopts

	# Anything here that was UN-"Apply"-ed ?
	#	(that WASN'T specifically DEFERed)
	foreach key [array names pref] {
		if {"$tmpopts($key)" ne "$opts($key)"
		&& (![info exists g(deferD)] || $key ni $g(deferD))} {
			if {![info exists YN]} {
				set YN [popmsg "You made UN-APPLIED edits !\n\n	  Remove them?"\
								"Please confirm" question yesno $prefwin]
				if {$YN == "no"} { return }
			}
			set tmpopts($key) "$opts($key)"
			Dbg "Dismiss: Un-setting $key"
		}
	}
	if {![info exists YN] || $YN == "yes"} {
		# Make certain any internal Vars are re-aligned w/what WILL exist
		# and ensure nothing remains hilited, then UNMAP the Dialog
		verify alertD reset
		Dialog dismiss $prefwin
	}
}

###############################################################################
# Apply customized preferences - $WdG is only provided when invoked from dialog
#	and is the ID of the 'Apply' button that caused it to become invoked
# Expects $opts() holds CURRENT settings; $tmpopts() ALL (possibly CHGD) values
###############################################################################
proc prefapply {{WdG {}}} {
	global g w pref opts tmpopts

	set feedback green ;# Presumed 'status' of updating prefs (AS A WHOLE)
	set prevgrid [wm grid .]
	# Geom-manager 'propagation' is generally OFF within w(client) to force any
	# sizing changes (particularly subtle ones such as font adjustments caused
	# by Text tagging) to "trade" among its OWN widgets, instead of "Appealing"
	# for more space from the toplevel. Its ALSO critical to our EMULATION of a
	# 'pane window' relation between the L/R Txtwins (SANS a widget): it too
	# needs a hard-stop confined area to work properly. YET, we come through
	# this 're-cfg' code not ONLY when the USER asks us to, but ALSO during the
	# INITIAL startup BEFORE we have ANY IDEA how big anything SHOULD be, and
	# thus appealling TO the Toplevel is actually a NECESSITY...
	#
	#	SO we USE the fact that AT startup, propagation is NOT YET "OFF"
	#	(it will BECOME SO after we return from that SPECIFIC startup call)
	#
	#	HOWEVER - the behavior we WANT is to prevent the INITIAL startup from
	#	producing a window LARGER than the current screen - even if we have to
	#	LIMIT the users "preferred" opts(geometry) value regarding Txtwin sizes
	#
	# Afterward, the Toplevel will be modifiable ONLY by the USER, yet it
	# should ALWAYS operate in a 'gridded' resize-mode. This gets insidious
	# when considering that some 'prefs' might control the visibility of client
	# elements which could ALTER the amount of Txtwin real-estate, and thus
	# WOULD modify the "grid defn" for that Toplevel. But there is a CATCH:
	#
	#	TYING a Txtwin *into* its Toplevel (via the Txtwdg "-setgrid 1" option)
	#	ONLY WORKS PROPERLY if IT is the ONLY widget to absorb any resizing!
	#
	# It turns out we have one Toplevel that DOES (merge) and one NOT (client)!
	# Yet there is a STOOPID reason we still want to use "-setgrid 1" in BOTH
	# cases - and that is the Txtwdg calculates its OWN IDEA of what the grid
	# INCREMENT size (in pixels) is for the loaded font! Something we can only
	# approximate by "measuring an entire alphabet" and divide by 26 - which
	# works - but is clearly "English based" unlike all user-provided files to
	# be displayed. SO WE ARE BANKING on TK to "get it right" by LETTING it
	# THINK (for a moment) that just ONE of the L/R (client) Txtwdgs will be
	# TIED to the Toplevel **JUST** so we can get that Font analysis performed.
	# Then we will sever the connection, but USE the computed pixel value, and
	# install NEW Toplevel 'gridding' parameters ourselves! What a PITA!!
	#	N.B> Despite being described here, this all MOSTLY happens @ the end.

	if {! [file isdirectory $tmpopts(tmpdir)]} {
		popmsg "Invalid temporary directory:\n$tmpopts(tmpdir)\nReverted ..." \
				$w(prefs)
		set tmpopts(tmpdir) $opts(tmpdir)
		set feedback red
	}

	# (Possibly) rebalance the Txtwin(s):
	#	Effectively CANCELs any EXISTING L/R 'pane' adjustments,
	#	and RESULTs in re-centering the DiffMap (if displayed)
	# N.B> Subtle impact: Should cause L/R Txtwdg WIDTHs to become IDENTICAL!
	grid columnconfigure $w(client) {0 2} -weight 100 -uniform a

	# This may look contrived (see discussion above for explanation) ...
	#	... the point is that IF it fails for ONE, it LIKELY fails for ALL,
	#		so there is little point to "fail and reset" over-and-over
	foreach wdg {Left merge Right Bottom} toplnk {1 1 0 0} {

		# Should this Txtwdg LINK to its Toplevel (for gridded resizes)?
		if {$toplnk} {set toplnk "-setgrid 1"} {set toplnk ""}

		# N.B> ensure 'toplnk' has every chance of becoming set (when active):
		#	Even *if* USERS input is invalid (to preserve later code semantics)
		if {[catch "$w(${wdg}Text)	 configure $toplnk $tmpopts(textopt)"]} {
			popmsg "Invalid text widget setting:\n\n'$tmpopts(textopt)'" \
					$w(prefs)

			# Error recovery - restore PRIOR settings (+ DONT do any further!)
			$w(${wdg}Text)	 configure {*}$opts(textopt)
			set tmpopts(textopt) $opts(textopt)
			set feedback red
			break
		}
	}

	# Make certain the (now established) Text FG/BG colors are PUSHed into
	# attrs needed to visibly see their focus-highlight border, and the BG
	# of their adjoining Info windows (just get seed values from CURRENT win)
	set fg [$w(acTxWdg) cget -foreground]
	set bg [$w(acTxWdg) cget -background]
	foreach txtwin {Left Right merge} {
	   $w(${txtwin}Text) configure -highlightb $bg -highlightc $fg
	   $w(${txtwin}Info) configure -background $bg
	}

	#NOTE: This loop is basically "testing" each NEW tag setting for syntactic
	#	   validity (as well as 'installing' them). H O W E V E R ...
	#	   it is IMPERATIVE they PROCESS (and thus remain) in PRECEDENCE order
	#	   already ESTABLISHED @creation time
	foreach tag [lsearch -all -inline [$w(acTxWdg) tag names] "*tag"] {
		foreach win [list $w(LeftText) $w(RightText)] {
			if {[catch "$win tag configure $tag $tmpopts($tag)"]} {
				popmsg "Invalid settings for \"$pref($tag)\":\n\
				\n'$tmpopts($tag)' is not a valid option string\nReverted..." \
				$w(prefs)
				# if one fails, restore its prior 'good' setting
				eval "$win tag configure $tag $opts($tag)"
				set tmpopts($tag) $opts($tag)
				set feedback red
			}
		}
	}

	# Same for the (only) tag for the line-comparison widget ...
	if {[catch "$w(BottomText) tag configure diff $tmpopts(bytetag)"]} {
		popmsg "Invalid settings for \"$pref(bytetag)\":\n\
		'$tmpopts(bytetag)' is not a valid option string.\nReverted..." \
				$w(prefs)
		# Again, if it fails, restore the prior 'good' setting
		eval "$w(BottomText) tag configure diff $opts(bytetag)"
		set tmpopts(bytetag) $opts(bytetag)
		set feedback red
	}

	# ... but if that tag contained a FONT request, we want to elevate that
	# font to the entire widget (lest it obscure the windows basic purpose)
	# INCLUDING the possible need to re-cfg the window height to match
	if {[set bHiFont [$w(BottomText) tag cget diff -font]] ne ""} {
		if {$bHiFont ne [$w(BottomText) cget -font]} {
			$w(BottomText) configure -font "$bHiFont" -height 2
		}
	}

	# tabstops require a little extra work. We need to figure out the width of
	# an "avg char" in the widget's font, multiplying by the tab stop "count".
	# We tried using an "m", but "0" appears to work better.
	set cwidth [font measure [$w(acTxWdg) cget -font] "0"]
	set tabstops [expr {$cwidth * $tmpopts(tabstops)}]
	$w(LeftText)   configure -tabs $tabstops
	$w(RightText)  configure -tabs $tabstops
	$w(mergeText)  configure -tabs $tabstops

	# But, for the bottom text widget, the tabstop is adjusted to take into
	# consideration the two bytes PREFIXED to each line (ie: "< " or "> ").
	$w(BottomText) configure -tabs \
		[list [expr {$tabstops+($cwidth*2)}] [expr {2*$tabstops+($cwidth*2)}]]

	# Set remaining 'opts' to the values from 'tmpopts'
	#	N.B> any ERRORS to this point have all been REVERTED to prior values
	# PAY ATTENTION:
	#	Most options represent "data state" values and can simply be 'set',
	#	that INCLUDES those already processed (above) which WILL be recorded;
	#	but some are TRANSITION (or 'edge') triggered and thus must notice
	#	when they are being CHANGED, more so than JUST their final value.
	#
	#		With such 'edge' options, SEQUENCE *does* make a difference,
	#	such as the 'ignore...' group, which could force a REdiff and thus
	#	influence OTHER settings, such as skipping tasks which ultimately get
	#	redone anyway (such as inline-diff processing, which ITSELF has its
	#	own sequence issue [unwinding 2 NEARLY mutually exclusive values]).
	#		We also want to avoid the time it can take to RE-tag everything
	#	(via a call to 'remark-diffs') if we dont need to - so we have to
	#	watch for CHANGES among the options that *could* have altered tags.
	#
	# BUT WE CANT assess *all* of that until we've seen ALL the settings
	#	(or worse, write code to handle each COMBINATION that might occur)
	#
	# SO - we 'pre-arrange' those settings having their OWN issues into a
	# sub-order we can depend on (to write the logic ONE way), and then post
	# flag values we can assess AFTERWARD to enforce the larger precedence
	# issues - thus avoiding the "excess" work alluded to above.
	#
	#	(N.B.: when the startup coding invokes 'prefapply', it just COPIES
	#	'opts' into 'tmpopts' first - as such, transitions will NEVER exist.)

	# First we need an 'inversion' primitive to access meta-state values ...
	set OTHER(showinline1) showinline2
	set OTHER(showinline2) showinline1

	# ... NEXT, preload any keys needing their OWN precedence order ...
	# (Does anyone appreciate all this work for 'auto-Diff'ing?)
	#
	#	(Reason: must see ANY key that MIGHT trip an 'error' BEFORE we look at
	#			 whether the Diff cmd changed. EG- ANY of the hotkey bindings!!
	#			 (Would need to DEFER a redo-Diff execution if errors detected)
	lappend keys genEdit genFind genNxfile genPvfile genRecalc genXit navFrst \
				   navLast navNext navPrev mrgLeft mrgRght mrgRtoL mrgLtoR \
				   geometry diffcmd

	#	(Reason: chgd content of an '...opt' field that IS [and WILL] remain
	#			 in use, OR turning the entire category ON/OFF for a redo-Diff)
	lappend keys ignoreEmptyLn ignoreRegexLnopt ignoreRegexLn

	#	(Reason: switching among inline algorithms, INCLUDING simple ON or OFF)
	lappend keys showinline1 showinline2

	#	(followed by EVERY PREF defined - BUT each mostly PROCESSES once)
	#	N.B> ensures we dont MISS any (as has happened before... )
	lappend keys {*}[array names pref]

	# ... finally, init the flags we need to derive - and then GET TO IT!!
	set remap [set remark 0]		 ;# defaulted as: do NOT remap or remark
	set inlActn {}					 ;#	 NOR 'compute-inlines' or force a Diff
	if {[info exists  g(deferD)]} {
		set redoDiff $g(deferD)	   ;#	 UNLESS that rediff was PENDING!!!
		unset g(deferD)
	} { set redoDiff {} }

	foreach key $keys {
		# What (if anything) is transitioning ?
		if {$tmpopts($key) ne $opts($key) && $key ni $redoDiff} {
			switch $key {
			"geometry" {
				# More of a syntax-chk validation than a transition issue
				if {2 > [scan $tmpopts(geometry) "%dx%d" na na]} {
					popmsg "Invalid geometry:\n$tmpopts(geometry)\n		 \
							Reverted..." $w(prefs) "Improper syntax..."
					set tmpopts(geometry) $opts(geometry)
					set feedback red
				}
			}
			"diffcmd" -
			"ignoreEmptyLn" {
				# EITHER a NEW Diff command was formatted, OR tertiary output
				# needs to be RE-analyzed for ignoring empty-line hunks.
				#	N.B> The fact that this IS DIFFERENT is sufficeint
				if {$key ni $redoDiff} { lappend redoDiff $key }
			}
			"ignoreRegexLnopt" {
				# Here we catch changes made in the "...opt" field while the
				# toggle REMAINS in a (non-transitional) 'ON' state...
				set key2 [string range $key 0 end-3]
				if {$tmpopts($key2) && $opts($key2) && $key2 ni $redoDiff} {
					lappend redoDiff $key2
				}
			}
			"ignoreRegexLn" {
				# Turning this 'ON' requires REFERING to a non-empty opt list
				#	(N.B> depends on "...opt" being processed FIRST)
				if {$tmpopts($key)} {
					if {![llength $opts(${key}opt)]} { set tmpopts($key) 0
					} elseif {$key ni $redoDiff} { lappend redoDiff $key }

				#but turning 'OFF': gauranteed (couldn't have been ON w/o data)
				}  elseif {$key ni $redoDiff} { lappend redoDiff $key }
			}
			"egnCase" -
			"egnBlanks" -
			"egn#Blanks" -
			"egn@TabX" -
			"egn@EOL" -
			"egnTabSiz" {
				# All these are dependent on the EVENTUAL state of "ignSuprs"
				#	When ON, each of these is a contributor to "diffcmd" and
				# thus subject to DEFERRAL if an error occurs.
				# Otherwise, simply accepting the toggle is sufficient
				#	(N.B> MUST be processed AFTER having seen "diffcmd")
				if {$tmpopts(ignSuprs) && "diffcmd" in $redoDiff} {
					lappend redoDiff $key
				}
			}
			"showinline1" -
			"showinline2" {
				# (meta-logic here only APPEARS convoluted)
				# Basically has only 3 possibilities:
				#
				# ... a DOUBLE transition: MUST select the eventual 'ON'
				if {"$tmpopts($OTHER($key))" ne "$opts($OTHER($key))"} {
					if {$tmpopts($key)} {
						#	THIS opt *is* the 'ON', but must then PRESET
						# the other OFF (to eliminate the 2nd transition)
						set opts($OTHER($key)) 0
					}
					set inlActn "compute-inlines $key"

				# ... a single OFF -> ON transition
				#	  -OR- the ALLOWED 2nd transition from prior DOUBLE)
				#	  (N.B> but passes an explicit NOFLUSH flag for the former)
				} elseif {$tmpopts($key)} {
					set inlActn "compute-inlines $key [expr {"$inlActn" != {}}]"

				# ... a single ON -> OFF transition
				} else { set inlActn "compute-inlines off" }
			}

			"inlSuprs" {
				# (N.B> processes AFTER 'showinline' above)
				# This only needs to DO something if:
				#	1. its value CHANGED (obviously)
				#	2. the RESULT (from above) of 'showinline2' is true
				#	3. and 'inlActn' was NOT already set (from above)
				# Otherwise, just recording the changed value is fine
				if {$inlActn == {} && $opts(showinline2)} {
					set inlActn "compute-inlines showinline2"
				}
			}

			"genEdit" -
			"genFind" -
			"genNxfile" -
			"genPvfile" -
			"genRecalc" -
			"genXit"  -
			"navFrst" -
			"navLast" -
			"navNext" -
			"navPrev" -
			"mrgLeft" -
			"mrgRght" -
			"mrgRtoL" -
			"mrgLtoR" {
				# Pass the EXISTING bindScript TO the NEW keystroke defn
				#	(ALL global shortcuts APPLY to the 'toplevel' widgets)
				if {[catch "bind . $tmpopts($key) {[bind . $opts($key)]}" E]} {
					popmsg "Bind failed: Preference '$key':\n$E\nBind Ignored" \
							$w(prefs)
					bind . $tmpopts($key) {} ;# Failed- need to remove try?
					set tmpopts($key) $opts($key) ;# RETAIN old keystroke!!
					set feedback red
				} {
					# Success! Push to the other toplevel, & erase old hotkey
					#	N.B> problematic if reassigning SAME hotkey TWICE (seq?)
					Dbg {$key binding swapped: $opts($key) to $tmpopts($key)}
					bind $w(merge) $tmpopts($key) "[bind $w(merge) $opts($key)]"
					bind .merge $opts($key) {}
					bind .		$opts($key) {}
					# Also update any MENUs that are advertising the hotkey!
					if {[info exists w(Accel,$key)]} {
						foreach {mnu idx} $w(Accel,$key) {
							$mnu entryconfigure $idx -accelerator "$opts($key)"
						}
					}
				}
			}

			"mapchg" -
			"mapdel" -
			"mapins" -
			"mapolp" {set remap 1}

			"chgtag"	 -
			"currtag"	 -
			"deltag"	 -
			"difftag"	 -
			"inlinetag"	 -
			"instag"	 -
			"overlaptag" -
			"tagtext"	 -
			"textopt"	 {set remark 1}
			}

			if {$feedback=="green" || $key ni $redoDiff} {
				set opts($key) $tmpopts($key)
				Dbg "ACCEPTED $key" 0 "Apply: "
			} { Dbg "DEFERRED $key" 0 "Apply: "}
		}
	}

	# interpret this binary toggle into its true value
	set opts(relief) [expr {$opts(fancyButtons) ? "flat" : "raised"}]

	# Need to TRANSLITERATE the USER input form of "Text tags" that deal with
	# the display attrs of Text, LineNumbers and/or ChangeBars, and INSTEAD
	# compute a derivation into data lists [g(scrInf,tags) and g(scrInf,cfg)]
	# that can emulate (via a canvas) what WAS FORMERLY implemented (TkDiff 4.2
	# and earlier) as individual Text widgets. This all comes together in
	# 'plot-line-info' which renders the EQUIVALENT Info data format as before,
	# but WITHOUT the potential line-skewing introduced by TK V8.5 enhancements
	translit-plot-txtags $w(acTxWdg) ;# L/R Text attrs identical: grab one

	# Walk down our DERIVED precedence-list flags and find out what needs doing
	#	(which is nothing if its all being handled by forcing a whole new Diff)
	# N.B> if any prior ERRORS occurred then THOSE ITEMS reverted to UNCHANGED,
	#	and are UNABLE to trigger any derivative changes, thus do NOT restrict
	#	performing such actions (may as well do what we KNOW needs doing)
	if {![llength $redoDiff]} {
		# (what about any altered tag SETTINGs ?)
		if {$remark} {
			eval $inlActn ;# MAYBE recompute inlines (so they CAN be tagged ?)
			remark-diffs
			show-status ""
		# (or how about ONLY an altered inline algorithm or on/off state ?)
		} elseif {"$inlActn" != ""} {
			eval $inlActn ;# recompute the inlines
		}
		# chgd map colors
		if {$remap > 0 && $g(startPhase) > 1} { map-draw }
	}

	# Align, (show or hide) various data (Lnums, Cbars, etc.), and we are done
	cfg-toolbar
	do-show-Info
	do-show-map
	do-show-lineview
	multiFile threshld $opts(fLMmax)
	###########################################################################
	### OK - thats it for getting preferences in place	-  - -
	######
	#		BUT we MAY need to look at HOW LARGE the tool window might become
	#	IF we follow ONLY the prefs. We wish to PREVENT the INITIAL tool window
	#	from EXCEEDING the screensize by TREATING the 'geometry' pref more as a
	#	'upper-bound' than as an explicit requirement.
	#
	# IN ADDITION, despite having earlier connected the Left wdg FOR gridding,
	#	ALWAYS *dis-connect* it from its Toplevel AGAIN after the prefs setup
	#	REGARDLESS because its technically WRONG (only works for 1 widget --
	#	WE have TWO)!! Nevertheless, we need to USE it to PRESEVE any potential
	#	RESIZE the user MAY have performed in the interim.
	lassign [concat [wm grid .] $prevgrid]									  \
						GW(1) GH(1) GcW(1) GcH(1)	 GW(0) GH(0) GcW(0) GcH(0)
	Dbg {PROPOSED wm grid is WxH($GW(1)x$GH(1)) of WxH($GcW(1)x$GcH(1))pxls\n\
		\twhile PREVsz was WxH($GW(0)x$GH(0))		($GcW(0)x$GcH(0))}
	if {$WdG == {}} {
		# ON STARTUP--- (one time per session only)
		# Turn the SCREEN pixel size into an equiv number of Fontbased grids,
		# divided by 2 (for each TxtWdg), and (ugh) FUDGE its companions sizes.
		#	(Apologies for the 'magic "-18"' in this equation - how it came to
		#	BE is lost to history - my best guess is it represents *A* means
		#	of accounting for what USED to be multiple vertical widgets w/fixed
		#	widths, that '-setgrid' is NOT measuring, but ARE a portion of what
		#	gridded-resizing is expected to manage (expressed in grid-incr(s).)
		# Ultimately 'maxw' is the LARGEST #of chars (ie. grid cells), that if
		# configured to BOTH Txtwdgs, results in NO screen-clip of the WINDOW
		# Similarly 'maxh' is the equivalent height value, again in grid units
		set maxw [expr {(([winfo vrootwidth .] / $GcW(1)) / 2) - 18}]
		set maxh [expr {([winfo vrootheight .]			  -
				 ($opts(showlineview) ? [winfo reqheight $w(BottomText)] : 0) -
						 [winfo reqheight $w(menubar)]	  -
						 [winfo reqheight $w(toolbar)]	  -
						 [winfo reqheight $w(status)])	  / $GcH(1)}]

		# N.B> 1st-time execution @create-time: make sure REQSTD geometry
		#	ISNT itself causing the INITIAL window to EXCEED the screen size!
		#	User can always MANUALLY resize LATER if they so choose.
		scan $opts(geometry) "%dx%d" width height
		set GW(1) [min $maxw $width]
		set GH(1) [min $maxh $height]
		Dbg {Trim BOTH L/R to NEW computed width($GW(1)) - and detach gridding}

		$w(LeftText)  configure -height $GH(1) -width $GW(1)  -setgrid 0
		$w(RightText) configure -height $GH(1) -width $GW(1)

		# Double it: one for each L/R TxtWdgs and CHOOSE it for final settings
		#	N.B> *this* is the portion that "-setgrid 1" DOESN'T understand!!
		incr GW(1) $GW([set i 1])
	} else {
		# Make sure we DISCONNECT the Toplevel from the widget, but PRESERVE
		# any GRIDDED size the USER may have MANUALLY adjusted the window to
		#	This gets a little hairy if the NEW prefs has CHANGED the CellSz
		#	and MAY yet be ODDER if the window manager has alterred the window,
		#	as it appears (on X11 anyway) to 'reserve' space for the menubar
		#	by TRIMMING the grid width-count of cells to not invade it.
		# Thus recompute the Width COUNT to express the same POSITION but with
		# utilizing the ?NEW? Cell size Width
		if {$GcW(0) != $GcW(1)} {
			set GW(0) [expr {( $GcW(0) * $GW(0) ) / $GcW(1)}]
		}
		$w(LeftText) configure -setgrid [set i 0]

		set hold [$WdG cget -activebackground]	;# Let user know if it worked
		$WdG configure -activebackground $feedback ; $WdG flash
		$WdG configure -activebackground $hold
	}
	update idletasks  ;# update all this (we BELIEVE *up* the geom mgr chain)
	wm grid . $GW($i) $GH($i) $GcW(1) $GcH(1)	;# Make grid deal w/L&R wdgs
	Dbg {  NEW	 wm grid is ($GW($i) X $GH($i))	  ($GcW(1) x $GcH(1))}

	# Force a whole new Diff if user changed ANY of the result semantics
	# PROVIDED we passed thru re-configuration unscathed; OTHERWISE ...
	#	remember we NEED to, but ONLY GENTLY remind User to fix their mistakes
	if {[llength $redoDiff]} {
		if {$feedback == "green"} {verify alertD reset; reCalcD $redoDiff} {
			popmsg "Due to previous errors, a detected need for re-invoking\
			Diff has been deferred.\n\n	   Respecify any items that were\
			'Reverted' and 'Apply' them again"	   warning		$w(prefs) \
			"Diff request deferral..."
			set g(deferD) $redoDiff ;# Remember this is PENDING!!
		}
	} elseif {$inlActn != {} && $opts(showinline2)} {
		# Must nudge L/R Text widget to re-evaluate visibility of INLINES
		# N.B> asking to see the Window *2nd* line is a virtual guarantee that
		#	*NOTHING* will scroll - but the REQUEST gains us a Visability scan
		after idle after 0 $w(acTxWdg) SEE @0,0+1line
	}
}

###############################################################################
# Save customization changes.
###############################################################################
proc prefsave {wdg} {
	global g w pref opts

	# Make a backup (if present) then open the NEW Preference File
	if {[file exists $g(rcfile)]} {file rename -force $g(rcfile) "$g(rcfile)~"}
	set fid [open $g(rcfile) w]

	# Declare WHEN tkdiff wrote this file (and what version was used)
	puts $fid "# This file was generated by $g(name)"
	puts $fid "# [clock format [clock seconds]]"
	# NOT a preference, per se - but is used when MORPHing outdated Prefs
	puts $fid "define prefsVrsn {$g(version)}\n"

	# Now, put ALL of the preferences in the file
	#	(with one small wrinkle - CERTAIN prefs have platform dependant values)
	# When we encounter one of THOSE, make sure we prepend the CURRENT platform
	#	to its key, and grab ANY EXISTING others that MAY have been stored as
	#	'cargo' data on readin, and WRITE THOSE BACK OUT as well!
	#
	# Otherwise its just a plain old preference and out it goes
	# N.B> A platform prefix WILL perturb the alpha-order key list - Ah well...
	foreach key [lsort [array names pref]] {
		regsub "\n" $pref($key) "\n# " comment
		puts $fid "# $comment"

		# Watch for any of our TRIGGER key PREFIXes: "nav", "mrg" or "gen"
		#	N.B> DANGER - misnomer: we cheated and match PERMUTATIONS thereof
		# Ensure it emits using the PRESENT windowing system 'extra' PREFIX;
		#	and additionally CHECK for (and output) any POSSIBLY ASSOCIATED
		#	"cargo" values pertaining TO that SAME basic key
		if {[string match "\[nmg]\[are]\[vgn][string range $key 3 end]" $key]} {
			foreach {wSys} "aqua win32 x11" {
				if {[info exists opts($wSys$key)]} {
					puts $fid "define $wSys$key {$opts($wSys$key)}"
				}
			}
			# Hint: the "current" system is always AFTER any cargo --
			#	(sneakily IDENTIFIES what platform WROTE the preference file)
			puts $fid "define $w(wSys)$key {$opts($key)}\n"
		}  else {	puts $fid "define $key {$opts($key)}\n" }
	}

	# ... and now any custom code
	puts $fid "# custom code"
	puts $fid "# Put any custom code you want to be executed in the"
	puts $fid "# following block. This code will be automatically executed"
	puts $fid "# after the GUI has been set up but before the diff is "
	puts $fid "# performed. Use this code to customize the interface if"
	puts $fid "# you so desire."
	puts $fid "#  "
	puts $fid "# Even though you can't (as of version 3.09) edit this "
	puts $fid "# code via the preferences dialog, it will be automatically"
	puts $fid "# saved and restored if you do a SAVE from that dialog.\n"
	puts $fid "# Unless you REALLY know what you are doing, it is probably"
	puts $fid "# wise to leave this unmodified.\n"

	puts $fid "define customCode {\n[string trim $opts(customCode) \n]\n}\n"

	close $fid

	if {$::tcl_platform(platform) == "windows"} {
		file attribute $g(rcfile) -hidden 1
	}

	# Let user know SOMETHING happened
	set hold [$wdg cget -activebackground]
	$wdg configure -activebackground green
	$wdg flash
	$wdg configure -activebackground $hold
}

###############################################################################
# Text has scrolled horizontally, update scrollbars and synchronize windows
###############################################################################
proc hscroll-sync {id args} {
	global g w opts

	# If ignore_hevent is true, we've already taken care of scrolling.
	# We're only interested in the first event.
	if {$g(ignore_hevent,$id)} { return }

	# Scrollbar sizes
	lassign [$w(LeftText)  xview] start size1 ; set size1 [expr $size1-$start]
	lassign [$w(RightText) xview] start size2 ; set size2 [expr $size2-$start]

	if {$opts(syncscroll) || $id == 1} {
		set start [lindex $args 0]
		if {$id != 1} {
			set start [expr {$start * $size2 / $size1}]
		}
		$w(LeftHSB) set $start [expr {$start + $size1}]
		$w(LeftText) xview moveto $start
		set g(ignore_hevent,1) 1
	}

	if {$opts(syncscroll) || $id == 2} {
		set start [lindex $args 0]
		if {$id != 2} {
			set start [expr {$start * $size1 / $size2}]
		}
		$w(RightHSB) set $start [expr {$start + $size2}]
		$w(RightText) xview moveto $start
		set g(ignore_hevent,2) 1
	}

	# Force all the event handlers for the view alterations above to trigger,
	# having locked out the recursive (redundant) events using ignore_hevent.
	update idletasks

	# Restore to normal
	set g(ignore_hevent,1) 0
	set g(ignore_hevent,2) 0
}

###############################################################################
# Main Text widget has scrolled vertically, update scrollbar(s)
###############################################################################
proc vscroll {id y0 y1} {
	global w

	# Update ACTUAL scrollbar
	$w(${id}VSB) set $y0 $y1

	# And MAYBE the MAP one if this HAPPENS to be the active window
	if {$w(acTxWdg) == $w(${id}Text)} { map-move-thumb $y0 $y1 }
}

###############################################################################
# Draw a miniature map of the diff regions
###############################################################################
proc map-draw {} {
	global g w opts

	# There are TWO reasons we might not be able to properly draw (as yet):
	#	1. The TK geometry manager might not have gotten around to making
	#	   its decision about how big our window is supposed to be; or
	#
	#	2. The application may not have progressed far enough to HAVE the
	#	   data to plot anything useful quite yet
	#
	# Unfortunately we can only check the TK reason now (because if we test
	# for the other condition (a flag) here, we impose a restriction on the
	# application to RAISE g(startPhase) AHEAD of making its OWN call -
	# even when it KNOWS the data is perfectly ready.
	# N.B> startPhase is used globally to NOT WASTE TIME doing TK things that
	#		wont be correct because the application isnt quite ready yet.

	if {$g(mapheight) && $g(mapwidth)} {
		# We add some transparent stuff to make the map fill the canvas
		# in order to receive mouse events at the very bottom.
		$w(mapImg) blank
		$w(mapImg) put \#000 -to 0 $g(mapheight) $g(mapwidth) $g(mapheight)
	} else {return}

	# A Text widget ALWAYS contains a blank line at the end - thus
	# (in normal cases) it tends to LOOK like it has TWO; Yet, if
	# the input data LACKED a <NL> this ratio could blowup...
	# So protect it by providing a floor value of 1.0
	set lines [max [expr {double([$w(acTxWdg) index end]) - 2}] 1.0]
	set factor [expr {$g(mapheight) / $lines}]

	# Paint color stripes per type of every hunk
	foreach hID $g(diff) {
		lassign $g(scrInf,$hID) S E na na C1 na na C2

		set y [expr {int(($S - 1) * $factor) + $g(mapborder)}]
		set size [expr {round(($E - $S + 1) * $factor)}]
		if {$size < 1} {
			set size 1
		}
		switch -- "[append C1 $C2]" {
		"-"	 { set color $opts(mapdel) }
		"+"	 { set color $opts(mapins) }
		"!!" { set color [expr {[info exists g(overlap$hID)] ? \
						 $opts(mapolp) : $opts(mapchg)}]
			 }
		}

		$w(mapImg) put $color -to 0 $y $g(mapwidth) [expr {$y + $size}]
	}

	# replot the 'thumb' on top
	# implicitly handles a shift in position (if being called by map-resize)
	map-move-thumb {*}[$w(acTxWdg) yview]
}

###############################################################################
# Resize map to fit window size
###############################################################################
proc map-resize {args} {
	global g w opts

	# We need to keep its size up-to-date, starting with its height
	# First account for spacing items surrounding the map
	set	 g(mapborder) [$w(map) cget -borderwidth]
	incr g(mapborder) [$w(map) cget -highlightthickness]

	# This can be touchy - we are racing against the TK bkgnd task that can be
	#	cfg'ing the vertical scrolling (which calls us - at least twice -
	#							because of EACH of the Left/Right scrollbars)
	#	HOWEVER -- these FIRST call(s) might have PRECEDEd the geometry
	#	manager stretching w(map) to its proper size causing it to still be
	#	AT its 1x1 initial size which would then FAIL as we try to compute the
	#	INTERIOR size we can plot within!
	#
	#	THUS - simply watch the current map size until its viably LARGE enough

	# Reduce the effective height by any frame border elements (top AND bottom)
	# And, when that height is not stupidly short, record both width & height
	if {[set height [expr {[winfo height $w(map)]-($g(mapborder) *2)}]] > 10} {
		set g(mapheight)	$height
		set g(mapwidth)		[winfo width $w(map)]
	}

	# When we are in startPhase 1, we likely dont HAVE the data NEEDED to DRAW
	#	So limit this proc to just TRACKING the size changes; it will be
	#	explicitly drawn (from 'mark-diffs') when the data is ready
	if {$g(startPhase) > 1} { map-draw }
}

###############################################################################
# Toggle showing the line comparison window
###############################################################################
proc do-show-lineview {{showLineview {}}} {
	global w opts

	if {$showLineview != {}} {
		set opts(showlineview) $showLineview
	}

	if {$opts(showlineview)} {
		# (re-)Manage BottomText, then tickle to update SOMEWHERE reasonable
		grid $w(BottomText)
		$w(acTxWdg) mark set insert insert
	} else {
		grid remove $w(BottomText)
	}
}

###############################################################################
# Toggle showing inline comparison
###############################################################################
proc do-show-inline {which {truefalse {}}} {
	global opts

	# translation tbl TO mutually-disjoint option
	set other(showinline1) showinline2
	set other(showinline2) showinline1

	if {$truefalse != {}} {
		set opts($which) $truefalse
	}

	set flush 1
	# mutually disjoint options
	#	Turn requested option ON ?
	if {$opts($which)} {
		#	Yes, but was OTHER option already ON ?
		if {$opts($other($which))} {
			#	Yes - so mark IT as OFF, but the flush remains
			set opts($other($which)) 0
		} {
			# otherwise it is just turning this ON, thus no flush reqd
			set flush 0
		}
	} elseif {!$opts($other($which))} {
		# No, turn requested option OFF ('other' is already OFF)
		set which off ;# and dont generate more - but FLUSH remains
	}
	# POSSIBLY recompute but ALWAYS retags (even if only removal)
	compute-inlines $which $flush
}

###############################################################################
# Toggle showing map or not
###############################################################################
proc do-show-map {{showMap {}}} {
	global w opts

	if {$showMap != {}} {
		set opts(showmap) $showMap
	}

	if {$opts(showmap)} {
		grid $w(map) -row 1 -column 1 -stick ns
	} else {
		grid forget $w(map)
	}
}

###############################################################################
# Find and return the "diff INDEX" nearest to SCREENLINE $line.
###############################################################################
proc find-diff {line} {
	global g

	# Binary search $line as either WITHIN, or PRECEEDING the index returned
	#	N.B> $i is a REAL (0-based) list index - NOT a (1-based) Diff index;
	#	... UNLESS $line was BEYOND the last known hunk definition (and is thus
	#	*THE* proper Diff index of that last hunk).
	if {[set i [rngeSrch diff $line "scrInf,"]] != $g(count)} {

		# So it all comes down to this:
		#	If INSIDE the hunk (or it PRECEDED the FIRST hunk) - simply convert
		#	$i to its equiv(+1) Diff index; -OR- decide which is CLOSER:
		#	the prior ENDpt or the found STARTpt, adjusting $i to whichever WHILE
		#	ensuring its logical conversion to its "Diff index" value
		set S [lindex $g(scrInf,[lindex $g(diff) $i]) 0]
		set E [lindex $g(scrInf,[hunk-id [max 1 $i]]) 1]
		if {($S	 <=	 $line) || !$i || ($S - $line < $line - $E)} { incr i }
	}
	return $i
}

###############################################################################
# Calculate number of lines in diff region
# hID			 Diff hunk identifier
# version	(1, 2, 12, 21) left and/or right window version
###############################################################################
proc diff-size {hID version} {
	global g

	lassign $g(scrInf,$hID) S E P(1) na na P(2)

	switch -- $version {
	1  -
	2  { set lines [expr {$E - $S - $P($version) + 1}] }
	12 -
	21 { set lines [expr {$E - $S - $P(1) + $E - $S - $P(2) + 2}] }
	}
	return $lines
}

###############################################################################
# Toggle showing merge preview dialog or not
###############################################################################
proc do-show-merge {{showMerge ""}} {
	global g w

	if {$showMerge != ""} {
		set g(showmerge) $showMerge
	}

	# Re-cfg buttons to hint at state of intended Merge FILENAME (when visible)
	if {$g(showmerge)} {
		if {$g(mergefileset)} {
			$w(mergeWriteAndExit) configure -text "Save & Exit"
			$w(mergeWrite) configure -text "Save"
		} else {
			$w(mergeWriteAndExit) configure -text "Save & Exit..."
			$w(mergeWrite) configure -text "Save..."
		}
		if {![winfo ismapped $w(merge)]} {
			Dialog show $w(merge) $w(mergeText)
			merge-center ;# (centers the CDR - not the window)
		}
	} elseif {[winfo ismapped $w(merge)]} { Dialog dismiss $w(merge) }
}

###############################################################################
# Create Merge preview dialog
###############################################################################
proc build-merge {} {
	global g w opts

	if {![Dialog  NONMODAL	$w(merge)]} {
		wm	 title [set win $w(merge)] "$g(name) Merge Preview"
		wm	 group	   $win .
		wm transient   $win .
		wm protocol	   $win WM_DELETE_WINDOW {do-show-merge 0}

		frame $win.bottom
		frame $win.top -bd 1 -relief sunken

		# Certain widgets will need external handles, remainder are local
		set w(mergeInfo)		 $win.top.info
		set w(mergeText)		 $win.top.text
		set w(mergeVSB)			 $win.top.vsb
		set w(mergeHSB)			 $win.top.hsb
		set w(mergeWrite)		 $win.bottom.mergeWrite
		set w(mergeWriteAndExit) $win.bottom.mergeWriteAndExit

		# Window and scrollbars
		scrollbar $w(mergeHSB) -orient horizont -com [list $w(mergeText) xview]
		scrollbar $w(mergeVSB) -orient vertical -com [list $w(mergeText) yview]
		text $w(mergeText) -bd 0 -takefocus 1			\
				-yscrollcommand [list $w(mergeVSB) set] \
				-xscrollcommand [list $w(mergeHSB) set]

		canvas $w(mergeInfo) -highlightthickness 0

		pack $win.bottom -side bottom -fill x
		pack $win.top -side top -fill both -expand yes -ipadx 5 -ipady 10
		grid $w(mergeInfo) -row 0 -column 0 -sticky nsew
		grid $w(mergeText) -row 0 -column 1 -sticky nsew
		grid $w(mergeVSB)  -row 0 -column 2 -sticky ns
		grid $w(mergeHSB)  -row 1 -column 0 -sticky ew -columnspan 2

		grid rowconfigure $win.top 0 -weight 1
		grid rowconfigure $win.top 1 -weight 0

		grid columnconfigure $win.top {0 2} -weight 0
		grid columnconfigure $win.top	1	-weight 1

		# buttons
		button $win.bottom.mRecenter -width 8 -text "ReCenter" -underline 0 \
			-command merge-center

		button $win.bottom.mDismiss -width 8 -text "Dismiss" -underline 0 \
			-command "do-show-merge 0"

		button $win.bottom.mExit -width 8 -text "Exit $g(name)" -underline 0 \
			-command {do-exit}

		# These last two buttons NAMES are later re-cfg'd with "..." appended
		# when g(mergefileset)==0 to signify a file browser popup will occur
		# (provided the merge window itself is actually visible)
		button $w(mergeWrite) -width 8 -text "Save" -underline 0 \
			-command {merge-write-file}

		button $w(mergeWriteAndExit) -width 8 -text "Save & Exit" -underline 8 \
				-command {merge-write-file 1 }

		pack $win.bottom.mDismiss  -side right -pady 5 -padx 10
		pack $win.bottom.mRecenter -side right -pady 5 -padx 1
		pack $w(mergeWrite)		   -side right -pady 5 -padx 1 -ipadx 1
		pack $w(mergeWriteAndExit) -side right -pady 5 -padx 1 -ipadx 1
		pack $win.bottom.mExit	   -side right -pady 5 -padx 1

		# Insert tag defs (in precedence order)
		# N.B> This matters to 'plot-merge-info':
		#	 we NEED 'diffR' or 'diffL' as lowest precedence TAGS
		#	 (whichever applies to the diff line in question).
		#	 Its an encoding trick noting which SIDE contrib'ed a diff line.
		$w(mergeText)	  configure			  {*}$opts(textopt)
		$w(mergeText) tag configure {diffL}	  {*}$opts(difftag)
		$w(mergeText) tag configure {diffR}	  {*}$opts(difftag)
		$w(mergeText) tag configure {currtag} {*}$opts(currtag)
		$w(mergeText) tag raise sel ;# Keep this on top

		# adjust the tabstops (see similar code in prefapply WHY we use "0")
		set cwidth [font measure [$w(mergeText) cget -font] "0"]
		set tabstops [expr {$cwidth * $opts(tabstops)}]
		$w(mergeText) configure -tabs $tabstops

		# Lastly, this text window ALSO needs to be READONLY, so we WRAP it
		rename $w(mergeText) $w(mergeText)_
		proc $w(mergeText) {cmd args} $::textROfcn
	}
	# N.B> cfg'ing and 'show'ing the dialog is up to 'do-show-merge'
}

###############################################################################
# Write merge preview to file (after optionally confirming filename)
###############################################################################
proc merge-write-file {{andExit 0}} {
	global g w opts
	Dbg {-> ([expr {$g(mergefileset) ? "into" : "confirming" }] $g(mergefile))}

	if {!$g(mergefileset)} {
		# Uncertain of wanting 'nativename' .vs. 'normalize' here...
		#	(each supposedly yields an absolute name)
		set path [file nativename $g(mergefile)]
		# Regardless, next SPLIT that into dir & file, and pass as PIECES ...
		# otherwise any/all user "directory browsing" will be IGNORED simply
		# because the '-initialfile' was passed as an absolute path!!
		set path [tk_getSaveFile -filetypes $opts(filetypes) \
			  -initialdir  [file dirname $path] \
			  -initialfile [file   tail	 $path] -defaultextension "" \
			  -parent [expr {[winfo ismap $w(merge)]? $w(merge) : $w(client)}]]

		if {[string length $path] > 0} {
			set g(mergefile) $path
		} else return ;# file browser cancelled out - DO NOT WRITE or EXIT
	 }

	# Actually write merge output to the given filename
	set hndl [open "$g(mergefile)" w]
	# PREVENT V9.x from throwing 'encoding' errors (does it matter for write?)
	if {$::tcl_version >= 9.0} { fconfigure $hndl -profile tcl8 }
	set txt [$w(mergeText) get 1.0 end-1lines]
	puts -nonewline $hndl $txt
	close $hndl

	if {$andExit} do-exit
}

###############################################################################
# Add a mark where each diff begins and tag each region so they are visible.
# Default case ONLY WORKS when pre-loaded text is the original (Left) version.
# Optional arg allows adding/removing (ie. editting) hunk identifiers later on
###############################################################################
proc merge-add-marks {{hIDS {}}} {
	global g w

	# Mark ALL lines first, so inserting choices won't mess up line numbers.
	#	N.B> WHEN hIDS is supplied, it MUST be homogeneous: ALL or NONE can
	#	pre-exist. And, when they dont exist, ascending order is REQUIRED.
	if {"$hIDS" != {}} {
		if {"mark[lindex $hIDS 0]" in [$w(mergeText) mark names]} {
			# Exists - so remove it (and every MERGE thing pertaining to it)
			foreach hID "$hIDS" {
				# CRITICAL: Put the merge text content BACK to a "Left" view !
				#  Then eliminate the mark AND choice (caller zaps the rest)
				merge-select-version $hID $g(merge$hID) 1
				$w(mergeText) mark unset mark$hID
				unset g(merge$hID)
			}
			return

		} else {
			# NEW hID - Find WHERE to plant each new MARK
			#	Apologies for the convoluted logic here, but we need a PRIOR
			# hunk location as an anchor (if there is one.) If NOT, then NO
			# numbers need adjusting; But if there IS, the rule of "Left only"
			# view DOES NOT APPLY to that FIRST anchor. Each planted MARK then
			# BECOMES the new anchor as we loop and is ALWAYS in "Left view"
			set prvHid {}
			foreach hID "$hIDS" {
				# Identify the 1st closest PRIOR hunk INDEX (if unknown)
				if {$prvHid == {}} {
					if {[set i [hunk-ndx $hID]] > 1} {incr i -1}
				}
				# If not YET known, produce prvHid and verify it really IS a
				# "PRIOR" hunk, setting 'i' to ITS merge-choice value if yes
				if {$prvHid != "" || ( "[set prvHid [hunk-id $i]]" != "$hID" \
									 && [set i $g(merge$prvHid)])} {
					# Now determine WHERE that anchor starts in 'mergeText',
					# ADDing its CURRENT SIZE (minus 1), plus the STARTING
					# position of the NEW hunk
					set S [expr {int([$w(mergeText) index mark$prvHid]) \
										 +	[diff-size $prvHid $i] - 1	\
										 +	[lindex $g(scrInf,$hID) 0]} ]
					# Using SCREEN numbering is OK because when we arrange
					# to subtract the screen END Lnum of the PRIOR hunk ...
					set O [lindex $g(scrInf,$prvHid) 1]
					# ... it will all convert to the NEW hunk location
				} else { lassign $g(scrInf,$hID) S na na O }

				# Set the NEW mark (and eventually fall thru to tagging)
				$w(mergeText) mark set mark$hID [incr S -$O].0
				$w(mergeText) mark gravity mark$hID left
				set prvHid $hID ;# This becomes the NEXT anchor (as we loop)
				set i 1			;# and (by defn) is ALWAYS in a "Left" view
			}
		}
	} else { ;# Do the entire Text (MUST BE in PURE LEFT context!!)
		foreach hID [set hIDS $g(diff)] {
			lassign $g(scrInf,$hID) S na na O

			$w(mergeText) mark set mark$hID [incr S -$O].0
			$w(mergeText) mark gravity mark$hID left
		}
	}

	# ... finally, select per merge CHOICES and TAG the regions for each
	set currdiff [hunk-id $g(pos)]
	foreach hID $hIDS {

		# Tag and/or Insert designated Left or Right window text versions
		# N.B.: works PROVIDED the merge hID range is IN a "Left copy" state
		if {$g(merge$hID) == 1} {
			# (But dont do a Left 'a'-type hunk - it's not visible)
			if {![string match "*a*" "$hID"]} {
				add-tag $w(mergeText) diffL {} mark$hID "+[diff-size $hID 1]"
			}
		} else { merge-select-version $hID 1 $g(merge$hID) }

		# Also attach "currtag" if/when correct hunk encountered
		if {"$hID" == "$currdiff"} {
			add-tag $w(mergeText) currtag {} \
									 mark$hID "+[diff-size $hID $g(merge$hID)]"
		}
	}
}

###############################################################################
# Remove/Re-Add hunk content to the merge window
# hID				diff hunk identifier
# oldversion   (1, 2, 12, 21) previous merge choice
# newversion   (1, 2, 12, 21) new merge choice
###############################################################################
proc merge-select-version {hID oldversion newversion} {
	global g w

	if {[set tot [diff-size $hID $oldversion]]} {
		$w(mergeText) DELETE mark$hID "mark${hID}+${tot}lines"
	}

	# Start of hunk in screen coordinates
	set S [lindex $g(scrInf,$hID) 0]

	# Get the text to insert directly from window
	switch -- $newversion {
		1 {
			if {[set tot [set i [diff-size $hID 1]]]} {
				lappend txt [$w(LeftText)  get $S.0 $S.0+${i}lines] diffL
			} else {return}
		}
		2 {
			if {[set tot [set i [diff-size $hID 2]]]} {
				lappend txt [$w(RightText) get $S.0 $S.0+${i}lines] diffR
			} else {return}
		}
		12 {
			if {[set tot [set i [diff-size $hID 1]]]} {
				lappend txt [$w(LeftText)  get $S.0 $S.0+${i}lines] diffL
			}
			if {[set tot [diff-size $hID 2]]} {
				lappend txt [$w(RightText) get $S.0 $S.0+${i}lines] diffR
				incr tot $i
			}
		}
		21 {
			if {[set tot [set i [diff-size $hID 2]]]} {
				lappend txt [$w(RightText) get $S.0 $S.0+${i}lines] diffR
			}
			if {[set i [diff-size $hID 1]]} {
				lappend txt [$w(LeftText)  get $S.0 $S.0+${i}lines] diffL
				incr tot $i
			}
		}
	}

	# Normally (prior to Combine/Split) mark$hID would ALWAYS have been the
	# sole Left-'gravitized' Text mark (attached to the newline ending the
	# NON-hunk line PRECEEDING the hunk start edge) at any ONE Text position.
	#	But since then, MULTIPLE marks (referring to optionally merge-able
	# abutted hunks) CAN COINCIDE, possibly only for a moment (between the
	# deletion and add done in this proc), thus causing them to cluster to the
	# front of ALL the possibilities - despite the need for SOME of those
	# choices to logically FOLLOW the insertion being made (to maintain linear
	# order).
	#	Thus we must analyze EVERY insertion for such clustering and POSSIBLY
	# adjust the gravities of SOME to ensure the hunk ordering linearity
	# imposed by g(diff) remains intact...
	set pos [hunk-ndx $hID]
	set regravitize {}
	foreach {na markID na} [$w(mergeText) dump -mark mark$hID] {
		if {[hunk-ndx [string range $markID 4 end]] > $pos} {
			$w(mergeText) mark gravity $markID right
			lappend regravitize $markID
		}
	}

	# NOW insert AND tag it (txt holds PAIRS of textlines AND assoc tag)
	$w(mergeText) INSERT mark$hID {*}$txt
	if {"$hID" == "[hunk-id $g(pos)]"} {
		add-tag $w(mergeText) currtag {} mark$hID "+$tot"
	}

	# ... Nevertheless, we always LEAVE all gravities as 'Left' AFTER the
	# insertion, just so we need not guess (or ask) the next time around.
	foreach {markID} $regravitize { $w(mergeText) mark gravity $markID left }
}

###############################################################################
# Center the merge region in the merge window
###############################################################################
proc merge-center {} {
	global g w

	# bail if there are no diffs
	if {$g(count) == 0} { return }

	# Size of diff in lines of text
	set hID [hunk-id $g(pos)]
	set difflines [diff-size $hID $g(merge$hID)]

	# Window height in percent
	set yview [$w(mergeText) yview]
	set ywindow [expr {[lindex $yview 1] - [lindex $yview 0]}]

	# First line of diff and total number of lines in window
	set firstline [$w(mergeText) index mark$hID]
	set totallines [$w(mergeText) index end]

	if {($difflines / $totallines) < $ywindow} {
		# Diff fits in window, center it
		$w(mergeText) yview moveto [expr {($firstline + $difflines / 2) / \
		  $totallines - ($ywindow / 2.0)}]
	} else {
		# Diff too big, show top part
		$w(mergeText) yview moveto [expr {($firstline - 1) / $totallines}]
	}
}

###############################################################################
# Update the merge preview window with the designated (1,2,12,21) merge choice
###############################################################################
proc do-merge-choice {newversion} {
	global g w opts

	set hID [hunk-id $g(pos)]
	switch $g(merge$hID) { 1 {incr g(statusMrgL) -1} 2 {incr g(statusMrgR) -1}}
	merge-select-version $hID $g(merge$hID) $newversion
	switch [set g(merge$hID) $newversion] {
								1 {incr g(statusMrgL)} 2 {incr g(statusMrgR)} }

	# Must ask user (when this is a collision) if their choice CLEARed it
	if {[info exists g(overlap$hID)]} {
		after idle [subst -nocommands {
				if {{yes} == [tk_messageBox -type yesno -icon question	\
				-title {Please Confirm} -parent $w(client) -default no	\
				-message "Did this choice RESOLVE the collision ?" ]}	\
					 { unset g(overlap$hID)
					   set-dtags $hID currtag overlaptag
					   if {$g(startPhase) > 1} { map-draw }
					 }
		}]
	}

	if {$g(showmerge) && $opts(autocenter)} {
		merge-center
	}
	set g(toggle) $newversion
}

###############################################################################
# Extract the start and end lines for file1 and file2 from the diff header
# passed in "line".
###############################################################################
proc extract {line} {
	# the line darn well better be of the form <range><op><range>, where op is
	# one of "a","c" or "d" (possibly in EITHER case). range will either be a
	# single number or two numbers separated by a comma.

	# is this a cool regular expression, or what? :-)
	regexp -nocase {([0-9]*)(,([0-9]*))?([acd])([0-9]*)(,([0-9]*))?} $line \
	  matchvar s1 x e1 op s2 x e2

	if {[info exists s1] && [info exists s2]} {

		if {"$e1" == ""} { set e1 $s1 }
		if {"$e2" == ""} { set e2 $s2 }

		return [list $s1 $e1 $s2 $e2 $op]
	} else {
		fatal-error "Could not parse following output line from diff:\n$line"
	}

}

###############################################################################
# Add a tag to a region (of chars on a given line -OR- of lines themselves).
###############################################################################
proc add-tag {wgt tag line start end} {
	global g

	if {"$line" eq {}} {
		# interpret OUR shorthand notation allowed for line tagging
		#	(args passed are INTEGERS - convert to INDICE syntax)
		if {[string match \[0-9\]* "$start"]} {append start ".0"}
		if {[string match \[0-9\]* "$end"]} {append end ".0"}
		# 'end' may begin with JUST a plus/minus value
		#	(+/-)xxx   becomes	 "start (+/-)xxx lines"
		#	   xxx	   becomes	 "xxx +1 lines"
		set end [expr {[string match \[-+\]* "$end"] \
						? "$start${end}lines" : "$end+1lines"}]
		$wgt tag add $tag $start $end			   ;# the lines themselves
	} else {
		$wgt tag add $tag $line.$start $line.$end  ;# chars ON $line
	}
}

###############################################################################
# Change the tags for the GIVEN diff region to appear as the CDR.
# 'hID' is the region hunk identifier (from the g(diff) list)
# If 'oldtag' is present, first remove it from the region
# If 'setpos' is non-zero, make sure the region becomes visible.
# Returns the diff hunk identifier UNLESS the Given range was INVALID, then ""
###############################################################################
proc set-dtags {hID newtag {oldtag ""} {setpos 0}} {
	global g w opts

	# Figure out which lines we need to address...
	if {![info exists g(scrInf,$hID)]} {
		# This may seem an ODD place for this to be but it IS correct
		#	If the REASON we can't find the designated hID is because there is
		# NONE TO BE FOUND (zero diffs) its POSSIBLE we just did a newDiff,
		# having reloaded all of the Text widgets and their CONTENTS.
		#	We needed to DELAY till here so g(startPhase) could be reset to
		# allow INFO plot actions to occur. They then fire AS we scroll to 1.0
		if {!$g(count)} {
			$w(LeftText)  SEE 1.0
			$w(RightText) SEE 1.0
			if {$g(showmerge)} {$w(mergeText) SEE 1.0}
		}
		return ""
	}
	lassign $g(scrInf,$hID) S E na na cL na na cR

	# Remove old tag
	if {"$oldtag" != ""} {
		$w(LeftText)  tag remove $oldtag $S.0 $E.0+1lines
		$w(RightText) tag remove $oldtag $S.0 $E.0+1lines

		# Of tags to remove, only "currtag" makes sense for the Merge window
		if {"$oldtag" == "currtag"} { catch {
			set lines [diff-size $hID $g(merge$hID)]
			$w(mergeText) tag remove $oldtag mark$hID "mark$hID+${lines}lines"}
		}
	}

	# Map chgbar marker(s) into applicable tag definition (danger: cL modified)
	switch -- [append cL $cR] {
	"-"	 { set coltag deltag }
	"+"	 { set coltag instag }
	"!!" { set coltag [expr {[info exists g(overlap$hID)] ? \
											 "overlaptag" : "chgtag" }]
		 }
	}

	# Add new tag
	if {$opts(tagtext)} {
		add-tag $w(LeftText)  $newtag {} $S $E
		add-tag $w(RightText) $newtag {} $S $E
		add-tag $w(RightText) $coltag {} $S $E
	}

	if {[set full [diff-size $hID $g(merge$hID)]]} {
		# Merge must map 'difftag' into SIDE-SPECIFIC equivalent tags
		if {"$newtag" == "difftag"} {
			# We'll use meta-programming to unwind and map the encoding
			# so create the transforms we need to access the pieces
			set sideTag([set side2(21) [set side1(12) 1]]) "diffL"
			set sideTag([set side2(12) [set side1(21) 2]]) "diffR"

			if {$g(merge$hID) < 10} {
				# Its a single side and occupies the 'full' length ...
				lappend tags $sideTag($g(merge$hID)) mark$hID $full
			} else {
				# ... or its  2 sides that SUMS to the full length (beware of 0)
				if {[set first [diff-size $hID $side1($g(merge$hID))]]} {
					lappend tags $sideTag($side1($g(merge$hID))) mark$hID $first
				} else {
					lappend tags $sideTag($side2($g(merge$hID))) mark$hID $first
				}
				# Append the 2nd piece (if needed)
				if {$first && $first != $full} {
					lappend tags $sideTag($side2($g(merge$hID))) \
								  mark$hID+${first}lines [expr {$full - $first}]
				}
			}
		} else {lappend tags $newtag mark$hID $full}
		foreach {tag where lines} "$tags" {
			add-tag $w(mergeText) $tag {} $where "+$lines"
		}
	}

	# Move the view on both text widgets so that the new region is visible.
	if {$setpos} {
		if {$opts(autocenter)} { centerCDR } else {
			$w(LeftText)  SEE $S.0
			$w(RightText) SEE $S.0
			$w(LeftText)  mark set insert $S.0
			$w(RightText) mark set insert $S.0

			if {$g(showmerge)} { $w(mergeText) SEE mark$hID }
		}
	}

	return $hID
}

###############################################################################
# moves to the diff nearest the insertion cursor or the mouse click,
# depending on $mode (which can be either "menu", "xy" or "mark") AND window
###############################################################################
proc moveNearest {window mode args} {
	global g w

	set isDiffMap [string match {*.map.canvas} $window]

	switch -- $mode {
	"menu" -
	"xy" {	lassign $args x y
			if {"$mode"=="menu"} {
				# Convert Menu ROOT coords to window
				incr x -[winfo rootx $window]
				incr y -[winfo rooty $window]
			}
			if {$isDiffMap} {
				# Diffmap represents the ENTIRE file - SCALE the coord
				# to fabricate the equiv Text index location
				set index [expr ($y.0 / $g(mapheight).0) \
								 * [$w(acTxWdg) index "end -1lines linestart"]]
			} { set index [$window index @$x,$y] }
	}
	"mark" {	set index [$window index [lindex $args 0]] }
	}

	move [find-diff [file rootname $index]] 0 1
}

###############################################################################
# this is called to decode a combobox entry into which hunk to jump to
###############################################################################
proc moveTo {window value} {
	global g w

	# we know that the value is prefixed by the number/index of
	# the diff the user wants. So, just grab that out of the string
	regexp {([0-9]+) *:} $value matchVar index
	move $index 0 1
}

###############################################################################
# Move the "current" diff indicator (i.e. go to a different diff region:
# If "relative" is 0 go to the GIVEN diff number; else treat as increment (+/-)
# Also accepts keywords "first" and "last"
###############################################################################
proc move {value {relative 1} {setpos 1}} {
	global g w

	if {$value == "first"} {
		set value 1
		set relative 0
	}
	if {$value == "last"} {
		set value $g(count)
		set relative 0
	}

	# Remove old 'curr' tag
	set-dtags [hunk-id $g(pos)] difftag currtag

	# Bump 'pos' (one way or the other).
	if {$relative} {
		set g(pos) [expr {$g(pos) + $value}]
	} else {
		set g(pos) $value
	}

	# Range limit REQUESTED 'pos' into "1 - MAX"
	set g(pos) [min [max $g(pos) 1] $g(count)]

	# Set new 'curr' tag
	#	N.B> if 'hunk-id' produces an UNKNOWABLE id (ie. "") due to ZERO hunks
	#		set-dtags does NOTHING except to jump L/R/merge windows to line 1.0
	#		irrespective of the value for $setpos
	set g(currdiff) [set-dtags [hunk-id $g(pos)] currtag "" $setpos]

	# update the buttons, etc.
	update-display

}

###############################################################################
# Align the availability of UI elements to the tools CURRENT context conditions
###############################################################################
proc update-display {} {
	global g w opts finfo

	#Dbg "	startPhase $g(startPhase)"
	if {!$g(startPhase)} return

	# The coding approach here is somewhat unusual:
	#	It's organized as sequential LAYERS of decisions instead of a single
	#	TREE of chained tests to arrive at each items proper '-state' setting.
	#
	#	To limit "flickering" of widgets, that choice of LAYER is critical.
	#
	#	Its best to try avoiding toggling the same widget from multiple layers,
	#	particularly as "else" clauses, only to nearly ALWAYS redo it at a
	#	LOWER layer. Think about the frequency that each layer-test is most
	#	likely to branch during general operation of the tool.
	#
	#	This works (and results in fewer code lines) - but its confusing to
	#	assess WHERE (which layer) any given widget BELONGS at and if it
	#	NEEDS to be repeated at MULTIPLE levels

	##### First layer - Does the tool have enough input to attempt a diff ?
	if {$g(startPhase) < 2} {
		# disable darn near everything

		foreach b [list rediff ignCDR splitCDR cmbinCDR find \
				   prevCDR firstCDR nextCDR lastCDR ctrCDR \
				   mrgC1 mrgC2 mrgC12 mrgC21] {
			$w(${b}_im) configure -state disabled
			$w(${b}_tx) configure -state disabled
		}
		foreach menu [list $w(popupMenu) $w(viewMenu)] {
			$menu entryconfigure "Previous*" -state disabled
			$menu entryconfigure "First*" -state disabled
			$menu entryconfigure "Next*" -state disabled
			$menu entryconfigure "Last*" -state disabled
			$menu entryconfigure "Center*" -state disabled
		}
		$w(popupMenu) entryconfigure "Find..." -state disabled
		$w(popupMenu) entryconfigure "Find Nearest*" -state disabled
		$w(popupMenu) entryconfigure "Edit*" -state disabled

		$w(editMenu) entryconfigure "Find*" -state disabled
		$w(editMenu) entryconfigure "Edit File 1" -state disabled
		$w(editMenu) entryconfigure "Edit File 2" -state disabled

		$w(fileMenu) entryconfigure "File List" -state disabled
		$w(fileMenu) entryconfigure "Write*" -state disabled
		$w(fileMenu) entryconfigure "Recompute*" -state disabled

		$w(mergeMenu) entryconfigure "Show*" -state disabled
		$w(mergeMenu) entryconfigure "Write*" -state disabled -label \
		 [expr {$g(mergefileset) ? "Write Merge File" : "Write Merge File..."}]

		$w(markMenu) entryconfigure "Bookm*" -state disabled
		$w(markMenu) entryconfigure "Clear*" -state disabled

	} else {
		# these are generally enabled, assuming we have (or about to re-)
		# run a proper DIFF of a couple of files
		foreach b [list rediff find prevCDR firstCDR nextCDR lastCDR \
				   ctrCDR mrgC1 mrgC2 mrgC12 mrgC21] {
			$w(${b}_im) configure -state normal
			$w(${b}_tx) configure -state normal
		}

		$w(popupMenu) entryconfigure "Find..." -state normal
		$w(popupMenu) entryconfigure "Find Nearest*" -state normal
		$w(popupMenu) entryconfigure "Edit*" -state normal

		$w(editMenu) entryconfigure "Find*" -state normal
		$w(editMenu) entryconfigure "Edit File 1" -state normal
		$w(editMenu) entryconfigure "Edit File 2" -state normal

		if {$finfo(fPairs) > 1} {
			$w(fileMenu) entryconfigure "File List" -state normal
		} else {
			$w(fileMenu) entryconfigure "File List" -state disabled
		}
		$w(fileMenu) entryconfigure "Write*" -state normal
		$w(fileMenu) entryconfigure "Recompute*" -state normal

		$w(mergeMenu) entryconfigure "Show*" -state normal
		$w(mergeMenu) entryconfigure "Write*" -state normal -label \
		 [expr {$g(mergefileset) ? "Write Merge File" : "Write Merge File..."}]

		# Hmmm.... on my Mac the combobox flashes if we don't add this
		# check. Is this a bug in AquaTk, or in my combobox... :-|
		if {[$w(combo) cget -state] != "normal"} {
			$w(combo) configure -state normal
		}
	}

	# update the status line AND if any RE-match data exists
	set g(statusCurrent) "$g(pos) of $g(count)"
	set g(statusInfo) ""
	$w(viewMenu) entryconfigure "Ignore RE*" -state \
		[expr {[llength $opts(ignoreRegexLnopt)] ? "normal":"disabled"}]

	##### Second layer - Do any diffs exist ? <implies a REAL g(pos)>
	#
	# Update the combobox, merge choices, and hunk centering.
	if {$g(count)} {
		# update the combobox. We don't want its command to fire, so
		# we'll disable it temporarily
		$w(combo) configure -commandstate "disabled"
		set i [expr {$g(pos) - 1}]
		$w(combo) configure -value [lindex [$w(combo) list get 0 end] $i]
		$w(combo) selection clear
		$w(combo) configure -commandstate "normal"

		# Merge choices and hunk centering
		foreach buttonpref {im tx} {
			$w(ignCDR_$buttonpref) configure -state normal
			$w(ctrCDR_$buttonpref) configure -state normal
			$w(mrgC1_$buttonpref) configure -state normal
			$w(mrgC2_$buttonpref) configure -state normal
			$w(mrgC12_$buttonpref) configure -state normal
			$w(mrgC21_$buttonpref) configure -state normal
		}
		$w(mrgLbl) configure -state normal

		$w(popupMenu) entryconfigure "Center*" -state normal
		$w(viewMenu) entryconfigure "Center*" -state normal
		$w(editMenu) entryconfigure "Ignore*" -state normal

	} else {
		# Note: this is essentially for the "No-Diffs-found" case
		#	and effectively suggests that Layer 4 will do NOTHING!
		foreach b [list ignCDR splitCDR cmbinCDR ctrCDR bkmRls \
			bkmSet mrgC1 mrgC2 mrgC12 mrgC21] {
			$w(${b}_im) configure -state disabled
			$w(${b}_tx) configure -state disabled
		}
		$w(mrgLbl) configure -state disabled

		$w(popupMenu) entryconfigure "Center*" -state disabled
		$w(viewMenu) entryconfigure "Center*" -state disabled
		$w(editMenu) entryconfigure "Ignore*" -state disabled
		$w(editMenu) entryconfigure "Split*" -state disabled
		$w(editMenu) entryconfigure "Combine*" -state disabled
		$w(markMenu) entryconfigure "Bookm*" -state disabled
		$w(markMenu) entryconfigure "Clear*" -state disabled
	}

	##### Third layer - is CDR at (or beyond) edges of its valid range ?
	#	(N.B> also applies to the legitimate "No Diffs Found" situation)
	#
	# Update navigation items
	if {$g(pos) <= 1} {
		foreach buttonpref {im tx} {
			$w(prevCDR_$buttonpref) configure -state disabled
			$w(firstCDR_$buttonpref) configure -state disabled
		}

		$w(popupMenu) entryconfigure "Previous*" -state disabled
		$w(popupMenu) entryconfigure "First*" -state disabled
		$w(viewMenu) entryconfigure "Previous*" -state disabled
		$w(viewMenu) entryconfigure "First*" -state disabled

	} else {   ;# can transition lower
		foreach buttonpref {im tx} {
			$w(prevCDR_$buttonpref) configure -state normal
			$w(firstCDR_$buttonpref) configure -state normal
		}
		$w(popupMenu) entryconfigure "Previous*" -state normal
		$w(popupMenu) entryconfigure "First*" -state normal
		$w(viewMenu) entryconfigure "Previous*" -state normal
		$w(viewMenu) entryconfigure "First*" -state normal
	}

	if {$g(pos) >= $g(count)} {
		foreach buttonpref {im tx} {
			$w(nextCDR_$buttonpref) configure -state disabled
			$w(lastCDR_$buttonpref) configure -state disabled
		}
		$w(popupMenu) entryconfigure "Next*" -state disabled
		$w(popupMenu) entryconfigure "Last*" -state disabled
		$w(viewMenu) entryconfigure "Next*" -state disabled
		$w(viewMenu) entryconfigure "Last*" -state disabled
	} else {   ;# can transition higher
		foreach buttonpref {im tx} {
			$w(nextCDR_$buttonpref) configure -state normal
			$w(lastCDR_$buttonpref) configure -state normal
		}
		$w(popupMenu) entryconfigure "Next*" -state normal
		$w(popupMenu) entryconfigure "Last*" -state normal
		$w(viewMenu) entryconfigure "Next*" -state normal
		$w(viewMenu) entryconfigure "Last*" -state normal
	}

	##### Fourth layer - is the specific CDR encumbered in some way
	# (thus g(pos) MUST have a legitimate value)
	#
	# Update availability of bookmarking and Split/Combine actions
	# AND the specific merge-choice selected
	if {$g(count) > 0} {

		# Show which merge option is current for this CDR
		set g(toggle) $g(merge[set hID [hunk-id $g(pos)]])

		# Bookmark (S)et and (C)lear items depend on the CDR marker
		# existance and are ALWAYS in opposite states to each other
		if {[winfo exists $w(bkmSF).mark$hID]} \
			{ set tmp {C S} }	{ set tmp {S C} }
		lassign {normal disabled} {*}$tmp
		foreach buttonpref {im tx} {
			$w(bkmRls_$buttonpref) configure -state $C
			$w(bkmSet_$buttonpref) configure -state $S
		}

		$w(markMenu) entryconfigure "Clear*" -state $C
		$w(markMenu) entryconfigure "Bookm*" -state $S

		# (S)plit/(C)ombine each have specific condition checks
		set S [expr {[splcmb-chk split $g(pos)] ? "normal" : "disabled"}]
		set C [expr {[splcmb-chk cmbin $g(pos)] ? "normal" : "disabled"}]
		foreach buttonpref {im tx} {
				$w(splitCDR_$buttonpref) configure -state $S
				$w(cmbinCDR_$buttonpref) configure -state $C
			}

		$w(editMenu) entryconfigure "Split*" -state $S
		$w(editMenu) entryconfigure "Combine*" -state $C
	}
}

###############################################################################
# Center entire CDR (or top line if cant fit) in each window (NO CDR? line 1.0)
###############################################################################
proc centerCDR {} {
	global g w

	if {[info exists g(scrInf,[hunk-id $g(pos)])]} {
		lassign $g(scrInf,[hunk-id $g(pos)]) S E

		# Window requested height in pixels
		set opix [winfo reqheight $w(acTxWdg)]
		# Window requested lines
		set olin [$w(acTxWdg) cget -height]
		# Current window height in pixels
		set npix [winfo height $w(acTxWdg)]

		# Visible lines
		set winlines [expr {$npix * $olin / $opix}]
		# Lines in diff
		set diffsize [expr {$E - $S + 1}]

		# Move insert markers to CDR first line
		$w(LeftText)  mark set insert $S.0
		$w(RightText) mark set insert $S.0

		# Center (if possible)
		if {$diffsize < $winlines} {
			set h [expr {($winlines - $diffsize) / 2}]
		}  { set h 2 }
	} else { set h 0 ; set S 1; # There IS no CDR
	}
		# Use YVIEW (to ignore syncscroll) but ALWAYS DO both windows anyway
		$w(LeftText)  YVIEW "[max 0 $S-$h].0"
		$w(RightText) YVIEW "[max 0 $S-$h].0"

	if {$g(showmerge)} { merge-center }
}

###############################################################################
# Wipe the slate clean...
###############################################################################
proc wipe {} {
	global g

	# Short circuit useless traces and key indexing lists
	if {$g(startPhase)} {set g(startPhase) 1}
	set g(COUNT)  [set g(count) 0]
	set g(DIFF)	  [set g(diff) ""]
	set g(d3Left) [set g(d3Right) {}]
	set g(pos) 0
	set g(currdiff) ""

	# N.B: It is critical that hID-related datums, particularly those that use
	# their EXISTANCE as the basis for internal decision making, be REMOVED
	# when attempting a "start over" to avoid seemingly random errors.
	# NOTE: finfo is managed specifically by 'assemble-args' DO NOT TOUCH
	array unset g {scrInf,[0-9]*}
	array unset g {overlap[0-9]*}
	array unset g {merge[0-9]*}
	array unset g {inline,*}
}

###############################################################################
# Wipe all data and all windows
###############################################################################
proc wipe-window {} {
	global g w

	wipe

	# Deleting text 'removes' any/all tag INSTANCES (w/o 'deleting' the defns)
	# yet LEAVES any marks: Wipe those out as well (but grab the list first...)
	#	N.B> mergeText must be processed first: (it HAS the needed mark names)!
	foreach wdg {mergeText LeftText RightText} {
		$w($wdg) DELETE 1.0 end

		if {"$wdg" == "mergeText"} {
			$w($wdg) mark unset {*}[set taglst [$w($wdg) mark names]]

			# ... now REWRITE taglst to refer ONLY to (derived) 'vL*' names
			for {set i 0} {$i<[llength $taglst]} {incr i} {
				if {[string match "mark*c*" [set nm [lindex $taglst $i]]]} {
				 set taglst [lreplace $taglst $i $i [string map {mark vL} $nm]]
				# (but PURGE elements that dont derive, readjusting the index)
				} else { set taglst [lreplace $taglst $i $i]; incr i -1; }
			}
		# but ALSO purge (now useless) 'vL*' tag defns from L/R Text widgets
		} else { $w($wdg) tag delete $taglst }
	}

	# No one uses this - what was it for? should we just Whack it?	\
	if {[string length $g(destroy)] > 0} {							\
		eval $g(destroy)											\
		set g(destroy) ""											\
	}

	$w(combo) list delete 0 end

	bkmark eraseall
}

###############################################################################
# Search an ascending sorted list of lower/upper bound pairs for a given value.
#  [**> LIST MUST EXIST AS A NAMED ARRAY ELEMENT OF THE GLOBAL ('g') SPACE <**]
#
# Returns the index that either CONTAINS it, or FOLLOWS it; -OR-
# the original list LENGTH (i.e. an invalid index), indicating 'Exceeds range'
#
#	N.B> as long as the bounds info is in the 1st two elements of the item
#		 being searched, additional fields may be stored in the same 'record'.
###############################################################################
proc rngeSrch {rnge val {indirect {}}} {
	global g

	# Until TcL V8.(6?).? arrives, there is NO "lsearch -bisect -command"
	#	(so this code is our own customized 'tuple binary-search' instead)

	# If 'rnge' contains (what amounts to) array INDICES to yet ANOTHER
	# table of values, then 'indirect' can be used to specify the PREFIX
	# name of where to indirectly access those ACTUAL range values
	if {$indirect != {}} {
		set ithItem {$g($indirect[lindex $g($rnge) $i])}
	}  {set ithItem			 {[lindex $g($rnge) $i]} }

	# Dont bother if 'rnge' is empty or 'val' exceeds its largest value
	set max [llength $g($rnge)]
	if {([set HI [set i [incr max -1]]] >= [set LO 0])	  \
	&&			   ($val <= [lindex [subst $ithItem] 1])} {

		# Pick the FIRST midpoint and extract its values
		set i [expr {($LO + $HI)/2}]
		lassign [lindex [subst $ithItem]] low hgh

		# Repetitively narrow the boundaries until we find it
		#	N.B> (extra expression ENSURES boundary ALWAYS moves)
		while {$HI > $LO} {
			if {$val > $hgh} {set LO [expr {$LO==$i ? $i+1 : $i}]} {
			if {$val < $low} {set HI [expr {$HI==$i ? $i-1 : $i}]} {
			break}} ;# Wow - a lucky HIT - stop NOW!!

			# Pick NEW midpoint and try again
			set i [expr {($LO + $HI)/2}]
			lassign [lindex [subst $ithItem]] low hgh
		}
	} else {return [incr max]}
	return $i
}

###############################################################################
# Specialized range-check machinery to find ancestor collisions (by mark-diffs)
# Return an encoded 'category' of Ancestor mark(s) found in the requested range
# Categories are: 0	 -> None
#				  1	 -> Additive
#				  2	 -> Deletive
#				  3	 -> Both
#
#	N.B> optional arg is an initially unknown VarName (in callers stackframe)
#	to permit CHAINED accesses. It avoids searching for the correct 'anc' range
#	as is done on the FIRST such access by storing its LAST USED 'anc' index
#	to simply resume from that point (not unlike a co-routine or iterator)
###############################################################################
proc chk-ancRnge {anclst S E {prev {}}} {
	global g

	if {![set result [llength $g($anclst)]]} {return 0}
	if {$prev != {}} {upvar $prev  ndx} ;# (Remember where NEXT call starts)

	# Do we skip 'binary searching' for the first ancestor range?
	if {![info exists ndx]} {
		# No...but if searching yields 'Exceeds known ranges' THATs an answer,
		# yet needs DECREMENTing (to a valid value) to be CACHEd (if it will)
		if {$result == [set ndx [rngeSrch $anclst $S]]} {incr ndx -1}
	}

	# Get values of first ancestor range to check
	#	(args S & E are expected to BE in min/max order)
	lassign "$S $E 0 [lindex $g($anclst) $ndx]" s(0) e(0) result s(1) e(1) mrk

	# Check ancestral ranges until found (or is known it CANT be found)
	while {$s(1) <= $e(0)} {
		# choose i'th segment as leftmost (and j as other - i.e. 0/1)
		set j [expr {[set i [expr {$s(0) > $s(1)}]] == 0}]

		# Look for range intersections and record category
		if {$s(0) == $s(1) || $e($i) >= $s($j)} {
			set result [expr { $result | ([string is lower $mrk] ? 1 : 2)}]
		}

		# Step to next ancestor range (if one actually exists)
		if {$ndx < [llength $g($anclst)] - 1} {
			lassign [lindex $g($anclst) [incr ndx]] s(1) e(1) mrk
		} else {break}
	}
	# If result is true, then  $s($j)  and	[min $e($i) $e($j)]
	# IS the OVERLAP BOUNDS (I think)
	# (could maybe be returned as an optional upvar'ed "list" ?)
	return $result
}

###############################################################################
# Interactively cause the CDR to be treated as suppressed from now on
###############################################################################
proc ignore-hunk {} {
	global	g w

	# Ensure g(pos) will remain within the eventual g(diff)
	if {[set i $g(pos)] == [llength $g(diff)]} {incr i -1}
	set hID $g(currdiff)

	# Then re-categorize the current hunk
	if {![mark-diffs [list $hID [string map {a A c C d D} $hID]]]} {
		set g(pos) $i
		$w(combo) configure -commandstate disabled
		$w(combo) configure -value {}
		$w(combo) configure -commandstate normal
		after idle {show-status "Files now APPEAR as identical"}
	} { move $i 0 1 }
	update-display
}

###############################################################################
# Mark difference regions and build up the combobox
#	N.B> Be very AWARE of when/why g(diff) .vs. g(DIFF) is used!!!
###############################################################################
proc mark-diffs {{rmvrpl {}}} {
	global g w opts

	set wdg(1) $w(LeftText)
	set wdg(2) $w(RightText)
	set O([set O(2) 1]) 2	;#	(just a simple meta-pgm 'O'ther identity value)
	lassign {0 0 0 0 0 0 0 0} \
	  g(COUNT) g(count) g(statusMrgL) g(statusMrgR) delta(1) delta(2) boxW boxC

	# Distinguishing between EDITTING .vs. LOADING of the global diff hunk
	# list is defined by the OPTIONAL "(r)e(m)o(v)e and (r)e(pl)ace" argument
	if {$rmvrpl != {}} {
		set Lpad [set Rpad {}]		;# (tmps for scheduling Pad-line removal)
		set hack {}					;# Dont need the responsiveness hack ...
		set g(startPhase) 1			;# ... but RE-suspend "plot-line-info"
		$w(combo) list delete 0 end ;# ComboBox will simply be RE-loaded

		# Next do the REMOVEs of hunks from the diff list FIRST (including all
		# that depends on them) ... REPLACing with the NEW hunks before return.
		#	This mostly works because the entries being removed occupy the
		#	SAME SINGLE contiguous run as the entries taking their place.
		# Happily, Tcl ALLOWS modifying a list ACTIVELY being processed (due
		# to its incessant CLONING of things DURING their modification)!!
		#
		# N.B> We DO NOT re-evaluate "suppression" rules when editting diffs!
		#	 It would interfere with the ability to RE-edit later...sorry!
		set i 0
		foreach d $g(diff) {
			if {[set ndx [lsearch -exact $rmvrpl $d]] >= 0} {
				if {![info exists inject]} {
					set i [set inject [lsearch -exact $g(diff) $d]];#1st delete
				}

				# Only ONE side ever has Pad lines - remove them
				lassign $g(scrInf,$d) na E Pl na na Pr
				set S [incr E] ;# (shift range downward for Widget addressing)
				if {$Pl} {lappend Lpad [incr S -$Pl].0 $E.0} ;# Left  Padding
				if {$Pr} {lappend Rpad [incr S -$Pr].0 $E.0} ;# Right Padding

				$w(LeftText)  tag delete vL$d ;# Left Vertical-Linearity
				$w(RightText) tag delete vL$d ;#Right Vertical-Linearity

				bkmark erase  [incr i]	  ;# Eliminate bookmark (if any)
				merge-add-marks [list $d]	;# ... and its Merge data

				unset -nocomplain g(inline,$d) ;# inline diffs
				unset -nocomplain g(overlap$d) ;# 3way diff collision
				unset g(scrInf,$d) ;# line numbering information
				Dbg "  KILLED  inline $d"

				# Now that everything is gone, remove the hID from $rmvrpl
				set rmvrpl	[lreplace $rmvrpl $ndx $ndx]
			} elseif {$i>0} { break } ;# early out once contiguous block found
		}

		# We MUST have deleted SOMETHING by now... ?
		if {$i} {
			# Must eliminate Padding all at once to avoid shifting the indices
			if {[llength $Lpad]} {$w(LeftText)	DELETE {*}$Lpad}
			if {[llength $Rpad]} {$w(RightText) DELETE {*}$Rpad}

			# ... finally overlay the NEW hIDs ... REPLACING what was deleted
			# N.B> 'i' begins as an "index+1" position against g(diff) ...
			#	afterward, 'inject' refers to 1st NEW index in g(DIFF)
			set j [lsearch -exact $g(DIFF) [lindex $g(diff) $inject]];#map 1st,
			# N.B.: $rmvrpl COULD right now be a 1 element list of an IGNORED
			#	hunk that MUST be added to g(DIFF), but **NOT** to g(diff)!!
			#	(as fabricated by the 'ignore CDR' user action)
			if {[llength $rmvrpl] !=1 || [string match {*[acd]*} {*}$rmvrpl]} {
				set g(diff) [lreplace $g(diff) $inject [incr i -1] {*}$rmvrpl]
			} { set g(diff) [lreplace $g(diff) $inject [incr i -1]]}
			set i [expr {$i - $inject + $j}] ;# readjust i to last mapped index
			set g(DIFF) [lreplace $g(DIFF) [set inject $j] $i {*}$rmvrpl]
		}
	} else {
		# Ain't this clever? We want to update the display as soon as we've
		# marked enough diffs to fill the display so the user will have the
		# impression we're fast. But, to prevent it from slowing us down too
		# much, put this code in a variable and delete it AFTER it fires once
		set hack {
			# for now, just pick a number out of thin air. Ideally
			# we'd compute the number of lines that are visible and
			# use that, but I'm too lazy today...
			if {$g(count) > 25} {
				update idletasks
				set hack {}	   ;# once fired, dont bother doing it again
			}
		}
	}

	# Compute minimal spacing to format the combobox entry numbering
	set fmtW [string length "[llength "$g(diff)"]"]

	# Walk through each diff hunk DERIVING global data for eventual use
	foreach d $g(DIFF) {

		# If its Info ALREADY exists, we are obviously in EDIT mode, needing
		# primarily to keep the 'delta(*)'s updated AND (re)add into comboBox
		if {[info exists g(scrInf,$d)]} {

			# Get most of what we know of this hunk ...
			# ... derive its type and determine if we count it as a REAL hunk
			lassign $g(scrInf,$d) S E Pl na Cl Pr Or Cr
			if {[string is lower [set type [expr {"$Cl$Cr"=="" ? "I":"i"}]]]} {
				incr g(count)
			}
			incr g(COUNT) ;# It ALWAYS counts in the superset list

			# However, ALL existing hunks BEYOND the injected entries require
			# certain minor realignments:
			#	a) "scrInf,*" Ofst fields MUST be rewritten to the NEW deltas
			#	   likewise the S & E fields must adjust to the new delta SUM
			#	b) if REAL (and bookmarked?), it will need a renumbered label
			if {$inject < $g(COUNT)} {
				set S [expr {$delta(2) - $Or + $S}]
				set E [expr {$delta(2) - $Or + $E}]
				set g(scrInf,$d) \
							  [list $S $E $Pl $delta(1) $Cl $Pr $delta(2) $Cr]
				if {"$type" == "i"} {bkmark $d $g(count)}
			}
			incr delta(1) $Pl  ;# Keep the deltas CURRENT for EVERY hunk
			incr delta(2) $Pr

		} elseif { [set result [extract $d]] != ""} {
		# Otherwise, its a NEW hunk needing to be processed
			lassign $result s(1) e(1) s(2) e(2) type

			# Count it ... but only NON-suppressed hunks count as REAL
			incr g(COUNT)
			if {[string is lower $type]} {
				incr g(count)

				# In addition, before ALTERING any of those start/end numbers,
				# check for an active 3way diff and whether this hunk collides
				# any ancestral changes together. Moreover, also establish its
				# 'default' L/R merge choice (Ancestral over User preferred)
				if {$g(is3way)} {
					set g(merge$d) [set i [set j 0]]; # begin as unknown
					switch -- $type {
					"a" {	set i [chk-ancRnge d3Right $s(2) $e(2) RaNDX] }

					"c" {	set j [chk-ancRnge d3Left  $s(1) $e(1) LaNDX]
							set i [chk-ancRnge d3Right $s(2) $e(2) RaNDX]
						}
					"d" {	set j [chk-ancRnge d3Left  $s(1) $e(1) LaNDX] }
						}
					# ACTUAL choice is based on a 4x4 table of possibilities
					#	1st row handles 'Del's ;  1st col handles 'Add's ;
					#	and the rest applies to 'Chg's; Negative is a COLLISION
					# N.B> LOGICAL chances of SOME lower-right values occurring
					#	is LOWER than the apparent overwhelming -2's suggests!
					if {[set g(merge$d) [lindex {{ 0  1	 2 -2}
												 { 2 -2	 2 -2}
												 { 1  1 -2 -2}
												 {-2 -2 -2 -2}} $i $j]] < 0} {
						set g(overlap$d) 1; # Collision
						set g(merge$d)	 2; # Dflt Choice is 'Right' because
						# its implied by the original (L/R) file arrangement
					}
					if {!$g(merge$d)} {set g(merge$d) $opts(predomMrg)}
				} else { set g(merge$d) $opts(predomMrg) } ;# when NO 3way at all
			}

			# Now REmap s(1),e(1) s(2),e(2) to refer to SCREEN linenumbers
			# First, compute the RAW Left and Right linecounts
			set siz(1) [expr {$e(1) - $s(1)}]
			set siz(2) [expr {$e(2) - $s(2)}]

			# Then adjust BOTH starts, accounting for ALL PRIOR hunk padding
			#	(these then become this hunks starting SCREEN linenumbers)
			incr s(1) $delta(1)
			incr s(2) $delta(2)

			# Next, based on what TYPE of diff it is, decide WHICH widget:
			#	- gets any (and how much) blankline padding (via setting "i")
			#	- gets what type-associated ChangegBar character
			# N.B. Note that the RAW s($i) on "a,d"-types is 1-less initially
			# because it refers to a line number BEFORE the line that (by
			# virtue of the add/delete) does not actually exist on that side.
			# Uppercase types are hunks to be IGNORED (they get Padded only)
			set pad(1) [set pad(2) 0]
			set cbar(1) [set cbar(2) ""]
			switch -- $type {
			"A" -
			"a" {	 ;# an 'add' pads to the LEFT widget
					set pad([set i 1]) [incr siz(2)]
					incr s(1) ;# (RAW lnum was the one BEFORE the add)
					set cbar(2) [expr {$type == "a" ? "+" : ""}]
				}
			"D" -
			"d" {	;# a 'delete' pads to the RIGHT widget
					set pad([set i 2]) [incr siz(1)]
					incr s(2) ;# (RAW lnum was the one BEFORE the delete)
					set cbar(1) [expr {$type == "d" ? "-" : ""}]
				}
			"C" -
			"c" {	;# a 'change' pads to the SHORTER widget
					set i [expr {$siz(1) < $siz(2) ? 1 : 2}]
					set pad($i) [expr {abs([incr siz(1)] - [incr siz(2)])}]
					set cbar(2) [set cbar(1) [expr {$type == "c" ? "!" : ""}]]
				}
			}

			# Now, compute the END line numbers to THEIR screen values...
			incr siz($i) $pad($i)
			set e(2) [expr {$s(2) + $siz(2) - 1}]

			# IMPORTANT: if you've done the math (and logic), "e(1)" MUST EQUAL
			# "e(2)" when all is complete. But we still need the UNpadded value
			# as well -- so UNTIL THE NEXT ITERATION:
			#		  e(2) will hold the PADDED end value and
			#		  e(1) the UNpadded one.
			# Watch CAREFULLY where each gets used!!!
			# Moreover, s(1) will LIKELY be utilized as an INITIALIZED temp
			set e(1) [expr {$e(2) - $pad($i)}]

			# SAVE all this SCREEN ADJUSTED data for mapping various operations
			# later on throughout the tool
			#	N.B!!	  s(1),e(1) == s(2),e(2)  so only one set is recorded
			set g(scrInf,$d) [list $s(2) $e(2) \
				$pad(1) $delta(1) $cbar(1) $pad(2) $delta(2) $cbar(2) ]

			# Accumulate any newly computed padding for the NEXT iteration
			incr delta($i) $pad($i)

			# FINALLY, we can ACTUALLY pad the widget into compliance (if reqd),
			# THEN plant the vL* TAG on BOTH 'final' LINE.0 of EACH side of hunk
			# for re-config WHEN any "inline diff" prefs (and/or data) changes.
			# N.B> APPEND (by INSERT 'end+1indice') incase final LINE.0 *IS* NL
			if {$pad($i) > 0} {
				$wdg($i) INSERT $e(1).end+1indice [string repeat "\n" $pad($i)]
			}
			# (of course, only REAL "chg" hunks EVER need skew compensation)
			if {"$type" == "c"} {
				# The vL* TAG ensures each L/R hunk pairing remains the same
				# PHYSICAL height in BOTH Text widgets, PROVIDING a L/R
				# alignment of MOST lines, by diminishing the scrolling skew
				# introduced by TK Vrsn(>= 8.5) "display .vs. logical" lines.
				#	Begin by CREATING the TAG in BOTH widgets (w/NO instances)
				$w(LeftText)  tag configure vL$d -spacing3 0
				$w(RightText) tag configure vL$d -spacing3 0

				# If/When active, generate inline diff data for this hunk
				#	Note: this is ONLY the data - NOT a display change (yet)
				# N.B> DONT DO THIS for the Ratcliff Algorithm case:
				#			its WAY too expensive computationally!!
				if {$opts(showinline1)} {
					while {$s(1) <= $e(1)} {
						inline-byte $d [expr {$s(1) - $s(2)}]	  \
							[$w(LeftText)  get $s(1).0 $s(1).end] \
							[$w(RightText) get $s(1).0 $s(1).end]
						incr s(1) ;# (warned you this value could be trashed)
					}
				}

				# N.B> TK implementation issue:
				#	Ordinarily we would WAIT (until "remark-diffs") to PLACE
				#	these tags; but PLACING -or- CONFIGURing it triggers some
				#	ugly asnychronous updating that can take measuarble time
				#	to complete - giving rise to POSSIBLY reading "old" data.
				# To combat this, we PLACE the tag (in its 'do nothing' state)
				# NOW (so its 'updating' makes NO DIFFERENCE) and we'll deal
				# with RE-CONFIGURING it later (if and when needed)
				#
				# FURTHER, place them on FIRST char of each SIDES 'last-line'
				# to keep it from colliding with Pad line insertion!!
				# These are on DIFFERENT LINES when PADding actually occurred!
				add-tag	  $wdg($i)	 vL$d $e(1) 0 1
				add-tag $wdg($O($i)) vL$d $e(2) 0 1
			}
		}

		# Append entry into combobox (and hilight when its a 3way collision)
		if {[string is lower "$type"]} {

			# Also update the status window merge counters
			switch $g(merge$d) { 1 { incr g(statusMrgL) } 2 { incr g(statusMrgR) } }

			$w(combo) list insert end \
							 "[set item [format "%*d: %s" $fmtW $g(count) $d]]"
			if {[info exists g(overlap$d)]} {
				$w(combo) list itemconf end -background $opts(mapolp)
			}
			# measure its width, remembering the LONGEST entry seen ...
			#	This is a bit of a pain: the width is SPECIFIED by charCOUNT
			# of an AVERAGE (aka '0') character, but WE have a space+colon and
			# UP TO 2 commas, ALL of which are THINNER in a proportional font!
			# So we track ('boxC') those pesky commas along with the charCOUNT
			set boxW [max $boxW [string length $item]]
			if	{$boxC < 2} { if  {[string match {*,*,*} $item]} {set boxC 2} \
			  elseif {$boxC < 1 && [string match  {*,*}	 $item]} {set boxC 1}
			}
			eval $hack ;# ... and TRY to update display ASAP
		}
	}
	# Beyond here, MOST other tool functions are based on g(diff) and g(count)
	#	[big exception is "line numbering" code that uses g(DIFF) and g(COUNT)]

	# Shrinkwrap combobox TO its data (avoiding both clipping AND excess space)
	#	(ie. REDUCED by a pixel-calc'ed value of EQUIVALENT 'avg' chars)
	if {$g(count)} {
		set i [font measure [set f [$w(combo) cget -font]] "0"]
		set j [font measure	 $f	 ": [string repeat "," $boxC]"]
		$w(combo) configure -width [expr $boxW -((([incr boxC 2]*$i) -$j) /$i)]
	}

	# Ensure that any NEWLY CREATED diff regions are 'mark'ed in the MERGE
	# window (so they can be tagged in the next step -- note that 'rmvrpl' here
	# either HAS the list of ONLY the additions, or is EMPTY which flags the
	# procedure to mark EVERY diff (unless it was an SINGLE suppress request)
	if {$g(count)
	&& ([llength $rmvrpl]!=1 || [string match {*[acd]*} [lindex $rmvrpl 0]])} {
		merge-add-marks $rmvrpl
	}

	# Lastly, ensure the MAP reflects the CURRENT diffs and go (re-)TAG it all
	map-draw
	remark-diffs
	return $g(count)
}

###############################################################################
# Remark difference regions...
###############################################################################
proc remark-diffs {} {
	global g w pref opts

	if {$g(statusInfo) == ""} {show-status "Re-Marking differences..."}

	# First, reconfigure ALL tags (based on the current options) ...
	#
	# IMPORTANT - this loop DEFINES the ENTIRE tag PRECEDENCE throughout TkDiff
	#	(and MUST AGREE with/for the 'translit-plot-txtags' emulation coding!!)
	# (N.B> we abbreviate tag names here simply to fit in 80 columns)
	foreach tag {diff curr del ins chg overlap inline sel} {

		# By cycling each AMONG the widgets, we give them TIME to update things
		#	(N.B> PARTICULARLY Y-pixel-height related things!!)
		foreach win [list $w(LeftText) $w(RightText) $w(mergeText)] {
			if { "$tag" == "sel"} {
				# When we SEE the "sel" tag specified (which MUST be LAST), then
				# its TIME to RAISE any "vL" tags ABOVE everything BUT "sel"
				foreach tag [$win tag names] {
					if {[string match "vL*" $tag]} { $win tag raise $tag }
				}
				$win tag raise [set tag sel]

			} else { $win tag remove ${tag}tag 1.0 end

				# Catch provides an error check against bad userpref settings
				# that MAY have been editted BY HAND directly in the users file
				 if {($win != $w(mergeText) || $tag == "curr")
				 && [catch "$win tag configure ${tag}tag $opts(${tag}tag)" bad]} {
					popmsg "Invalid settings for \"$pref(${tag}tag)\":\
					\n\n'$opts(${tag}tag)' is not a valid option string:\n$bad\
					\n\nPlease repair this preference as soon as possible"

				# Yet 'difftag' cfgs into mergeText as TWO names: diffR & diffL,
				# but as ITSELF in the main Text windows (despite the same attrs)
				#  - a coding trick so merge knows which SIDE provided the line!
				} elseif {$win == $w(mergeText) && "$tag" == "diff"} {
					# ('difftag' SETTINGS were already validity checked by now)
					$win tag configure ${tag}R {*}$opts(${tag}tag)
					$win tag configure ${tag}L {*}$opts(${tag}tag)
				}
			}
		}
	}

	# Now, reapply the tags applicable to all the diff regions
	foreach hID $g(diff) {
		# First do the difftag (plus derivatives: ins/del/chg/overlap) ...
		set-dtags $hID difftag

		# ... and reinstate any needed inline Tagging (inclds. de-skew)
		if {($opts(showinline1) || $opts(showinline2))
		&& [string match "*c*" "$hID"]} { remark-inline $hID }
	}

	# Turn "plot-line-info" processing back ON if it was OFF
	if {$g(startPhase) == 1} {incr g(startPhase)}

	# finally, re-establish the current diff
	set g(currdiff) [set-dtags [hunk-id $g(pos)] currtag]
}

###############################################################################
# Update Skew correction (Re possibly LARGER 'inline' FONT usage) on given hunk
###############################################################################
proc de-skew-hunk {hID} {
	global g w

	# Get current skew value (either Left or Right) for DESIGNATED hunk
	#	(so it can be SUBTRACTED from our measurement)
	lassign $g(scrInf,$hID) s1 e1
	set lsz -[set Lsz [[set wL $w(LeftText)]  tag cget vL$hID -spacing3]]
	set rsz -[set Rsz [[set wR $w(RightText)] tag cget vL$hID -spacing3]]

	# Want measurements to be RE-ANALYZED but without any PRIOR SKEW included
	#	(best AFTER any deferred processing has completed ... sortof ... )
	update idletasks
	#		In truth, TK (@8.6.3) updates its Y-position CACHE in asynchronous
	#		chunks of about 250 lines per each go, *NOT* tied into "idletasks",
	#		ANYTIME something SUGGESTS it might change (and spacing3 *is* one)!
	#			Our best chance to AVOID reading "old-data" is to LIMIT the
	#		NUMBER of such asynch-"ripples" we propagate, AND at least TRY to
	#		space them out w/other processing (to give each the TIME req'd).
	# So START by asking each widget what it THINKS the SKEW *should* be ...
	#	(causing 1 ?foreshortened? ripple in each widget)
	incr lsz [$wL count -update -ypixels $s1.0 $e1.0+1lines]
	incr rsz [$wR count -update -ypixels $s1.0 $e1.0+1lines]

	# Now config shorter side IF NEEDED to make left/right screen heights agree
	#	(deal with MINOR possibility that skew MIGHT JUMP to other side)
	#		AT MOST - causes one ADDITIONAL ripple in each side (per hID) ...
	#		...but OFTEN only ONE side - if so forget the UNAFFECTED wdg name
	if {$rsz > $lsz} {
		if {$Rsz} {$wR tag configure vl$hID -spacing3 0} {set wR {}}
		if {$Lsz+$lsz} {
			$wL tag config vL$hID -spacing3 [expr $rsz-$lsz]}
		#	Dbg {  Skew $hID -> LEFT [expr $rsz-$lsz] L($lsz) R($rsz)}

	} elseif {$lsz > $rsz} {
		if {$Lsz} {$wL tag configure vl$hID -spacing3 0} {set wL {}}
		if {$Rsz+$rsz} {
			$wR tag config vL$hID -spacing3 [expr $lsz-$rsz]}
		#	Dbg {  Skew $hID -> RGHT [expr $lsz-$rsz] L($lsz) R($rsz)}

	} elseif {$Rsz || $Lsz} {
		# Because we measured IGNORING skew, ONE of these MAY have a value
		# when INLINing is being turned OFF (and thus NEEDS to be ZEROed)
		if {$Rsz} {$wR tag configure vl$hID -spacing3 0} {set wR {}}
		if {$Lsz} {$wL tag configure vl$hID -spacing3 0} {set wL {}}
		#	Dbg {  Skew $hID -> [expr $lsz-$rsz] L($lsz) R($rsz) was zeroed}

	# TAKING this return basically implies NOTHING needed changing
	#	Thus we can AVOID any further ripple processing by the TK internals
	} else {return}

	# N.B.	Ripples: the TK implementation of pixel-height management involves
	#  an async 'catch-up', obstensibly to maintain UI responsiveness. Sadly,
	#  their method can seemingly be short-circuited (unknown WHY) leaving the
	#  computation of the overall physical height (across ALL lines) improperly
	#  PROPAGATED (i.e. USING older cached-values) when calc'ing the FRACTIONS
	#  needed to talk to the scrollbars AND establish the TxtWdg YView.
	#  This in turn causes any sync'd scrolling to misrepresent (it DEPENDS on
	#  both L/R windows USING the SAME fractions) what each window should show
	#  even though the actual SETTINGS were already PROVIDED w/correct values!
	#		What we OBSERVED was that if EACH hunk was subsequently made to
	#  DISPLAY, something in TK NOTICED the "still incorrect cached values" and
	#  CORRECTED them, but for ONLY those ACTUALLY DISPLAYED. In short, if one
	#  SCROLLED thru every hunk, everything slowly came back into alignment.
	#
	#	As of Tk8.6.5 a new subcmd (sync) is POSSIBLE, but our approach MIGHT
	#	still be faster given we target smaller SPECIFIC indice ranges.
	#		(The "sync" implem. has a bit more of a GLOBAL WIDGET effect)
	# BONUS:  using "count -update -ypixels" also lets us STAY with TK V8.5
	#
	# SO - (assuming we actually changed SOMETHING...) we FORCE TK to rescan
	# our changes by RE-STARTING its background task which SHOULD yeild both
	# SIDES of the hunk FINALLY being cached CORRECTLY (and w/ALL skew GONE)
	if {$wL != {}} {$wL count -update -ypixels $s1.0 $e1.0+1lines}
	if {$wR != {}} {$wR count -update -ypixels $s1.0 $e1.0+1lines}

	# Tk8.6.3 *BUG*: the "legacy" Txtwdg implementation would OCCASIONALLY
	# invalidate internal BTree ptrs when the data size was very small. SEEMED
	# to involve the deferred processing somehow, and will hopefully be gone
	# if (or when) the Txtwdg impl is redone (see http://core.tcl.tk/ TIP #466)
	#
	# Calling "idletasks" TWICE (here and earlier) STOPPED the observed SEGV.
	update idletasks
}

###############################################################################
# Add inline tags for a given SINGLE hunk to BOTH Text widgets
# Does entire hunk UNLESS a specific Begin OR End Lnum (inclusive) is provided
###############################################################################
proc remark-inline {hID {BgnL 0} {EndL 0}} {
	global g w

	# N.B> Oddly enough, it is legitimately POSSIBLE that ABSOLUTELY IDENTICAL
	# linepairs can be 'inline-diff'ed resulting in NO output list of ranges!
	# Distinction is ENTIRELY about how Diff chose to describe the hunk...
	#	eg.:	 1c1,2
	#				   |  abc  |	 |	abc	 |	   <--- compared identical
	#				   |	   |	 |	d e	 |	   <--- skips (left is empty)
	#				   |  xyz  |	 |	xyz	 |		  (thus ZERO results)
	#	versus:	 1a2
	#				   |  abc  |	 |	abc	 |
	#				   |	   |	 |	d e	 |	   (only 'c' types DO inlines)
	#				   |  xyz  |	 |	xyz	 |
	#
	#	   (Diff output can be quite capricious at times!!)
	# Accordingly, variable MAY legitimately NOT EXIST - or be EMPTY!
	if {[info exists g(inline,$hID)] && [llength $g(inline,$hID)]} {
		set wdg(l) "LeftText"
		set wdg(r) "RightText"

		# Presumes 'inlinetag' was ALREADY removed from BOTH Text widgets
		# N.B> 'Bgn/End' DEFAULTS to 'S/E' resp. if not overriden when called
		lassign $g(scrInf,$hID) S E
		if {!$BgnL} { set BgnL $S}
		if {!$EndL} { set EndL $E}

		# N.B> note subtle oscillation between NDX and Lnum as loop proceeds
		foreach {side lndx Scol Ecol} $g(inline,$hID) {
			if {[incr lndx $S] <= $EndL} {
				if {$lndx >= $BgnL} {
					add-tag $w($wdg($side)) inlinetag $lndx $Scol $Ecol
				}
			}
		}
	}
	# Line heights MAY have changed (eg. TAG fonts) - compensate
	#	(Other tags are applied in L/R pairs, implicitly maintaining entropy)
	de-skew-hunk $hID
}

###############################################################################
# Post some SHORT informational text.
#	Behavior DEPENDS slightly on overall state (initial startup .vs. running)
# where target LABEL widget should be GROWN to accomodate the info msg passed.
###############################################################################
proc show-status {message} {
	global g w

	# Grow (pre-built ONLY) status widget to accept posting message
	if {!$g(startPhase) && [winfo exist .status]
	&&	[$w(statusLabel) cget -width] < [set grow [string length $message]]} {
		$w(statusLabel) config -width [min 70 $grow]
	}
	set g(statusInfo) $message
	update idletasks
}

###############################################################################
# A limited Cohen-Sutherland line CLIP Alg classifier (only does 1 dimension)
# thus its name: "half clip" ; Zero return means total INCLUSION
###############################################################################
proc hCLIP {s e mn mx} {
	return [expr ($e<$mn)*8 + ($e>$mx)*4 + ($s<$mn)*2 + ($s>$mx)]
	# Essentially a binary-packed PAIR of 2bit-wide values, thus has values
	# from 0 to 15. When in use, only roughly HALF (9) are LOGICALLY possible
}

###############################################################################
# Compute differences (start from the beginning, basically).
###############################################################################
proc rediff {} {
	global g w opts finfo

	# Read the files into their respective widgets
	# and derive the overall line number magnitude.
	set g(lnumDigits) 0
	set	 i [set j [expr {[set pairnum $finfo(fCurpair)] * 2}]]
	incr i -1
	set Statmsg [set msg {}];# Assume this is all gonna work ...
	foreach {LR ndx} [list Left $i Right $j] {
		# When finfo(pth,X) is NOT set yet, its a SCM file that
		# has not yet been obtained -- go get it
		show-status "reading $finfo(lbl,$ndx) ..."
		if {![info exists finfo(pth,$ndx)]} {
			# if it fails: finfo(pth,$ndx) will LIKELY be an empty tmpfile
			if {"" != [set msg [scm-chkget $ndx]]} {popmsg "$msg"}
		}
#### MCP
#		if {[catch {set hndl [open "$finfo(pth,$ndx)" r]}]} {
#			fatal-error "Failed to open file: $finfo(pth,$ndx)"
#		} else {fconfigure $hndl -translation \
#			[expr {"$::tcl_platform(platform)" == "windows" ? "crlf" : "lf"}]}
		set hndl [ gzopen "$finfo(pth,$ndx)" $ndx ]
		fconfigure $hndl -translation \
			[expr {"$::tcl_platform(platform)" == "windows" ? "crlf" : "lf"}]
#### MCP
		# PREVENT V9.x from throwing 'encoding' errors
		if {$::tcl_version >= 9.0} { fconfigure $hndl -profile tcl8 }
		$w(${LR}Text) REPLACE 1.0 end [read $hndl]
		# Must also replace the merge window contents (w/Left contents)
		if {$LR == "Left"} {
			seek $hndl 0 start ;# Rewind the Left file
			catch { $w(mergeText) mark unset [$w(mergeText) mark names] }
			$w(mergeText) REPLACE 1.0 end [read $hndl]
			if {![regexp {\.0$} [$w(mergeText) index "end-1lines lineend"]]} {
				$w(mergeText) INSERT end "\n"
			}
		}
		close $hndl
		set lines [file rootname [$w(${LR}Text) index end-1lines]]
		set g(lnumDigits) [max [string length "$lines"] $g(lnumDigits)]
	}
	# Provide feedback on this filepair being successfully accessed (or not)...
	# Decorate all the visuals per this set of files...and then finally push
	# g(lnumDigits) AND is3way to reconfig width of Info widgets (do-show-Info)
	if {$msg=={}} { multiFile mrkACK $pairnum } { multiFile mrkNAK $pairnum }
	alignDecor $pairnum
	do-show-Info

	# Diff the two files and store the summary lines into 'g(diff)'
	set cmd $opts(diffcmd)
#### MCP
#	lappend cmd $finfo(pth,$i) $finfo(pth,$j)
	if { $i <= 2 && $j <= 2 } {
		lappend cmd $finfo(unzip,$i) $finfo(unzip,$j)
	} else {
		lappend cmd $finfo(pth,$i) $finfo(pth,$j)
	}
#### MCP

	show-status "Executing {$cmd}"
	lassign [run-command "$cmd"] diffOUT diffERR diffRC
	set g(returnValue) $diffRC ;# Record REAL RC

	# Now, when that exit code *IS* 0 there are NO differences; when
	# its a 1 there *ARE* differences (but *PERHAPS* not what you expect)
	#	Any OTHER exit code simply means trouble
	if {$diffRC < 0 || $diffRC > 1 || $diffERR != ""} {
		popmsg "diff failed:$diffRC:\n$diffERR\n\
				[string range $diffOUT 0 75] ... (partial)"
		# Simulate 'identical' going forward: (avoids any further issues)
		set Statmsg ">> NO ACTION TAKEN << due to errors"
		set diffOUT {}
		set diffRC 0
		set lines {};	# Simulate 'identical' to avoid issues
	} elseif {"[set lines [split $diffOUT "\n"]]" != "" && $diffRC} {
		# Historical note: OLDER 'diff' vrsns USED to produce:
		#	"Binary files ..(names).. are different" ON stdout WITH RC=1
		#	Newer ones do similar, BUT w/RC=2 (when non-text files are used)
		# At least VERIFY 1st line LOOKS (mostly) like a 'Normal' diff header
		if {![string match {[0-9]*[acd][0-9]*} [lindex $lines 0]]} {
			# Hmmm. Try converting (in place) FROM "Unified" format ...
			deUnify lines
			if {![string match {[0-9]*[acd][0-9]*} [lindex $lines 0]]} {
				# ... Still NO?
			popmsg "diff failed:$diffRC: Unrecognized diff format:
			[string range $diffOUT 0 75] ... (partial)"
			set Statmsg ">> NO COMPARISON POSSIBLE << due to errors"
			set lines {};	# Again, simulate 'identical' to avoid issues
			set diffRC 0

		# Close enough - (at least protects us from unexpected formats)
		#	Cheap trick: extra trailing sentinel will flush NEXT loop
			} { lappend lines "0" }
		}	  { lappend lines "0" }
	}

	# Collect all lines containing diff hunk headers
	#	 N.B> Critical Concept- There are TWO lists of headers:
	#	'g(DIFF)' is the superset and includes EVERY reported hunk
	#	'g(diff)' is POTENTIALLY a subset, but is USED by MOST OF THE TOOL
	#
	# The distinction comes from options the user MAY have used to suppress
	# certain kinds of hunks (blanklines, REmatched) which WE MUST PROCESS and
	# NOT pass to Diff (it would HIDE places where widget padding is needed).
	#	Our technique is to UPPERCASE the headers for hunks being suppressed,
	# but then ALSO restrict such headers to the 'g(DIFF)' list.
	#
	#	When the options are NOT used, both lists are identical - (but beware
	# of LATENT bugs being CAUSED by keying some downstream feature to the
	# WRONG list!!). Otherwise, THIS code simply APPLYS the suppression options
	# and forms BOTH lists, in a "state machine" style of parsing. Both headers
	# AND diff content lines must be read, as the rules for "suppression" need
	# EVERY line of the hunk to be QUALIFIED before ignoring is possible.
	#	Generally, it is Text widget "Padding" and "Line numbering" tasks that
	# require the use of 'g(DIFF)'; everything(?) else should use 'g(diff)'.
	set hID [set g(DIFF) [set g(diff) {}]]
	foreach line $lines {
		switch -glob [string index $line 0] {
			"-" {continue}
			"[0-9]" {if {$opts(ignoreEmptyLn) \
				 || ($opts(ignoreRegexLn) && $opts(ignoreRegexLnopt) != "")} {
					if {[string length $hID]} {
						if {[string match {*[acd]*} $hID]} {
							lappend g(diff) $hID
						}
						lappend g(DIFF) $hID
					}
					# Presume it WILL suppress (re-activating at each hunk)
					set hID [string toupper [lindex $line 0]]
					if {[set Esuppress $opts(ignoreEmptyLn)]} {
						# Ignoring EMPTY lines CAN depend on BLANK suppression
						# (when active) as MOST can make ONLY WhtSpc evaporate
						if {$opts(ignSuprs) && ($opts(egnBlanks) +
										 $opts(egn#Blanks) + $opts(egn@EOL))} {
							set Eexpn {^..[[:space:]]*$};# any of "-wbZ" used
						} { set Eexpn {^..$}}			;#	   otherwise
					}
					set Rsuppress [llength $opts(ignoreRegexLnopt)]
				} elseif {[string length $line]-1} {
					lappend g(diff) [lindex $line 0]
					lappend g(DIFF) [lindex $line 0]
					set hID {}
				}
			}
			"[<>]" {if {![string match {*[ACD]*} $hID]} {continue}
				# Verify this lines data against the reasons for suppression
				if {$Esuppress} {
					if {![regexp $Eexpn $line]} {set Esuppress 0}
				}
				if {$Rsuppress} {
					set Rsuppress [llength $opts(ignoreRegexLnopt)]
					# (if ANY expn matches, then the suppression remains valid)
					foreach Iexpn $opts(ignoreRegexLnopt) {
						if {![regexp $Iexpn [string range $line 2 end]]} {
							incr Rsuppress -1} {break}
					}
				}
				# Cancel the presumption of suppression if the reason is gone
				if {!$Esuppress && !$Rsuppress} {set hID [string tolower $hID]}
			}
		}
	}
	Dbg {DIFF([llength $g(DIFF)]) .vs. diff([llength $g(diff)])}

	if {$diffRC && $g(is3way)} {
		# Make sure we HAVE the ancestorfile (go get it if not)
		if {![info exists finfo(apth,$pairnum)]} {
			# if it fails: finfo(apth,$pairnum) may LIKELY be an empty tmpfile,
			#	making the Ancestrals APPEAR as 1 BIG Add (on each side)
			if {"" != [set msg [scm-chkget "a$pairnum"]]} {popmsg "$msg"}
		}
		set g(d3Left) [set g(d3Right) ""]; # ERASE the 2 global output lists

		# SO - the WHOLE IDEA here is that based on a common ancestor, lines can
		# ONLY be ADDed or DELeted (a CHG is some of both). ADDed lines always
		# SURVIVE into the target, but a DEL line 'survives' only when the OTHER
		# side FAILS to ALSO delete it.
		#
		#	   >> We want the LOCATION of EVERY survivor (per its side) <<
		#
		#	Thus NON intersects simply get recorded, although to WHICH SIDE
		# depends on its ADD/DEL status. Adds go to the side of occurence, Dels
		# obviously can only be marked on the OPPOSING side for those portions
		# that were NOT deleted from BOTH ... but COLLISIONS are tricky!

		#########################################
		#	We've kinda "folded" this processing INTO itself because the
		# mapping technique TRUELY requires reading BOTH Diff streams
		# simultaneously in a "data directed" fashion. But to conserve both
		# time & code, we've reorganized it to do everything as a 3 pass
		# hybrid state-machine, reading first one, THEN the other stream
		# (during which MOST of the processing will occur), with a final
		# 'flush' pass to finish mapping any PENDING items from the 1st pass.
		#
		# N.B: 'i','j' ARE available for tmp usage once loop starts (a Tcl-ism)
		#	(the trailing ZERO sentinel forces the FINAL "3rd iteration" flush)
		foreach {NDX LR} [list $i Left $j Right 0] {
			if ($NDX) {
				# In 3-way merge - we diff EACH file "from" the ancestor
				set cmd $opts(diffcmd)
				lappend cmd $finfo(apth,$pairnum) $finfo(pth,$NDX)

				show-status "Executing {$cmd}"
				lassign [run-command "$cmd"] diffOUT diffERR RC;#NOT diffRC

				if {$RC < 0 || $RC > 1 || $diffERR != ""} {
					popmsg "Ancestor/$LR diff failed:$RC:\n$diffERR\n\
							[string range $diffOUT 0 75] ... (partial)"
					set diffRC 0; # Indicate we CANT do the 3way
					break
				} elseif {"[set lines [split $diffOUT "\n"]]" != "" && $RC} {
					if {![string match {[0-9]*[acd][0-9]*} [lindex $lines 0]]} {
						# Once again, MAYBE its actually "Unified" format...
						deUnify lines ; # TRY to convert it (in place)
					}
					if {![string match {[0-9]*[acd][0-9]*} [lindex $lines 0]]} {
						popmsg "Ancestor/$LR diff: Improper diff format:\n\n\
							[string range $diffOUT 0 75] ...\n (partial output)"
						set diffRC 0; # Again, we CANT do the 3way
						break
					}
				} elseif {!$RC} {
						popmsg "Ancestor/$LR diff: Inconsistent data:\n\n\
						Ancestor must NOT be identical to $LR side file"
						set diffRC 0; # Yet again, we CANT do the 3way
						break
				}

			# (3rd pass? ... FAKE our way INSIDE next loop (to flush Left side)
			} else {set lines [list { }]}

			foreach line $lines {
				# Spin until we detect a hunk header line
				if {$NDX && ![string match {[0-9]*} $line]} { continue }

				# 1st pass (Left): accumulate ALL the headers as a list ...
				if {$LR == "Left"} {
					lappend d3Left(hnks) "[extract $line]"

					# ... but also detect that first entry (& init onetime vars)
					#	  while ALSO tracking the max size of the list we build
					if {[set maxLh [llength $d3Left(hnks)]] == 1} {

						upvar 0	 d3Right CH; # Presume Right is initial CUR Hunk
						set CH(nxt) "break"
						set CH(swap) "d3Left"
						set CH(self) "d3Right"
						lassign {0 0 ? 0} CH(s) CH(e) CH(m) CH(cv);# init stage

						upvar 0	 d3$LR	MH ; # Presume Left is initial MAP Hunk
						set MH(nxt) {
							set d3Left(hnk) "[lindex $d3Left(hnks) [incr ndx]]"}
						set MH(swap) "d3Right"
						set MH(self) "d3Left"
						lassign {0 0 ? 0} MH(s) MH(e) MH(m) MH(cv);# init stage

						# Last, "prime" the upcoming while loop w/1st Map Hunk
						set ndx -1 ; eval $MH(nxt)
						lassign "$MH(hnk)" MH(s1) MH(e1) MH(s2) MH(e2) MH(typ)
					}
					# Dbg puts -nonewline "\n<$LR>$maxLh line($line)"
					continue

				# 2nd pass (Right): simply parse this one line (a 1-entry list)
				} elseif {$NDX} { set CH(hnk) "[extract $line]"
				# Dbg puts -nonewline "\n<$LR> line($line)"

				# 3rd pass (Left): Flip one FINAL time & map remaining(?) Lefts
				#
				# N.B.: CH & MH are aliases to the LOCAL d3Left/d3Right arrays
				#	(our data structure). Each ALWAYS refers to only ONE of the
				#	arrays, but are OFTEN exchanged so the coding can adhere to
				#	the notion that the CURRENT hunk (CH) is the one BEING
				#	mapped and (MH), the MAPPING hunk (from the other array),
				#	supplys the critical data needed to do so.
				} else {
						# Dbg puts -nonewline " *** FINAL CH MH swap ***"
						upvar 0 $MH(swap) MH   $CH(swap) CH}

				# Process BOTH diff headers IN TANDEM to create the Markings
				#
				#	Odd control test is yet ANOTHER "sneak path" to permit
				#	this (at first a) conventional "while" to LATER operate (if
				#	needed) as a NON-loop (triggering code is just BEYOND loop)
				#	The technique is a variation on "Dynamic Programming"
				while {$ndx < $maxLh || $ndx > $maxLh} {
					lassign $CH(hnk) CH(s1) CH(e1) CH(s2) CH(e2) CH(typ)

					# EARLIER hunk (in Ancestor nums) is expected to BE the one
					# mapped (CH); SHOULD we exchange (L<->R) to achieve that ?
					#	(*ONLY* if we are in NEITHER "flush" mode [per if-tst])
					if {$NDX && $ndx < $maxLh && $CH(s1) > $MH(s1)} {
						# Dbg puts -nonewline " *** Semantic CH MH swap ***"
						upvar 0 $MH(swap) MH  $CH(swap) CH }

					# Chk the DEL portion for collisions with EITHER another
					# DEL (from the opposite side) -OR- an ADD in the SAME side
					# (we designate these as Del->Del or Del->Add respectively)
					if {$CH(typ) != "a"} {
						# Dbg puts -nonewline "\n\tDel\t\tNDX($NDX) \
						CHhunk($CH(hnk)) ndx:maxLh($ndx<>$maxLh) CH($CH(self))"
						set E [expr {$CH(e1) - $CH(s1)}] ;# get span of Cur del

						# Next, compute its MAPPED startpt PRESUMING the
						# maphunk(MH) is BEYOND us in ancestral ordering ...
						set S [expr {$CH(s1) + ($MH(s2) -($MH(typ) != "d"))
											 - ($MH(s1) -($MH(typ) != "a"))}]

						# ...BUT, if that presumption was wrong, further adjust
						# the mapping by the distance spanned of that maphunk
						if {$CH(s1) > $MH(s1)} {
							incr S [expr {$MH(s1) - $MH(e1) -($MH(typ) != "a") \
										+ $MH(e2) - $MH(s2) +($MH(typ) != "d")}]
						}

						# Finally, we can finish the mapping by both setting
						# its endpt, and the needed "conversion factor" (CV)
						# that permits us recovering its ancestral numbering.
						incr E $S
						set CV [expr $CH(s1) - $S]

				#########################################
				# NOW the FUN ... manufacturing SEQUENTIAL marker data
				#
				#	Deletions pose an issue as you cant really mark a line that
				# isnt there. BUT, the line MIGHT exist on the other side (L/R)
				# IFF it so happens that it was never deleted by any of the
				# "other side" hunks!
				#	So what we need now is to COMPARE the deletions from BOTH
				# sides, eliminating those they have in common, and manufacture
				# markers for the rest. The trick is we have to compare using
				# ANCESTRAL values, but POST the marks in (L/R) numbers (hence
				# the CV value!).
				#	Marks are simply an ascending LIST of inclusive line num
				# pairings (and a displayable UPPERCASED marker char) that
				# describes lines within that range as "NOT deleted" as they
				# WERE by the opposing side. Mark numbers are side-specific!
				#
				# Ready?
				# We find collisions by comparing the MOST RECENT range posted,
				# against that which we are ABOUT to post. This clearly means
				# we MAY have to BOTH edit the PRIOR posting in addition to the
				# one in progress (be it an Add or a Del). ALL possibilities
				# exist: do nothing, eliminate both or either or NEITHER, even
				# splitting 1 into 2, and inserting the 3rd inbetween.
				#	To evaluate which, we perform a modified version (a single
				# dimension) of a Cohen Sutherland CLIP Alg. (hCLIP) to quickly
				# identify 1 of 9 possible situations, and then simply switch
				# among those possibilities. Using the following mnemonics:
				#		arg1	 S: Segment startpt		ALWAYS the curr Segmnt
				#		arg2	 E: Segment endpt
				#		arg3	RS: Range startpt		ALWAYS the prior Mark
				#		arg4	RE: Range endpt
				#
				# The 9 hCLIP-encoded proc result specifys these relationships:
				#	0	: S >RS	 and  E <RE
				#	2	: S<=RS	 and  E <RE
				#	4	: S >RS	 and  E>=RE
				#	5	: S =RE	 and  E>=RE
				#	6	: S<=RS	 and  E>=RE
				#	10	: S<=RS	 and  E =RS
				#	7	: (special case RS=RE): S =RS  and	E >RE
				#	14	: (special case RS=RE): S <RS  and	E =RE
				#	15	: (special case RS=RE): S =RS  and	E =RE
				#########################################

						# Note that BECAUSE Dels only post to the opposite side
						# LOOK for that "prior" Del post ON our own CUR side
						if {$CH(s) && [string is upper $CH(m)]
						  && (![set i [hCLIP $S+$CV $E+$CV	  $CH(s)+$CH(cv) \
									  $CH(e)+$CH(cv)]] || ($i%4) != ($i/4))} {

							#########################################
							# So why CLIP again? Its tough to explain easily,
							# but doing so *and* ADJUSTING the rangepts INWARD
							# yields DISTINCTIVE cases involving EITHER rangept
							#		(as outlined in the table above)
							#
							#	ABOUT THE FOLLOWING "if (![switch ...])":
							#	  In all the juggling to ensure the data stays
							#	sequential, AND the need to occasionally erase
							#	a prior Mark WITHOUT posting anything to take
							#	its place (and TCL not permitting "fall-thru"
							#	cases), we chose to UTILIZE the return value of
							#	"switch" (being the last statement value from
							#	whatever case was executed) to handle a tricky
							#	situation where we absolutely MUST UNFLUSH the
							#	LAST mark from the actual list, to repopulate
							#	the "staged" area WHICH MUST HAVE A VALUE AT
							#	ALL TIMES (except when it WILL be overwritten)
							#########################################
							if {![switch [set i [hCLIP $S+$CV $E+$CV		  \
										 $CH(s)+$CH(cv)+1  $CH(e)+$CH(cv)-1]] {
							"0"	 -
							"15" {	if {!$i} {
									# @0 flush: s/S-1 (then stage) E+1/e
										lappend g($CH(self))				  \
											   "$CH(s) [expr $S+$CV-1-$CH(cv)] \
												$CH(m) $CH(cv)"
										set CH(s) [expr $E+$CV+1-$CH(cv)]
									}
									set S 0;# Either way, Del Seg is eliminated

									expr {!$i}; # return code for UNFLUSH (T/F)
							}
							"2"	 -
							"10" -
							"14" {	if {[set j [expr $CH(s)+$CH(cv)-1-$CV]]<$S}\
																	   {set j 0}
									if {$i != 14} {;# @2 & @10 edit into: E+1/e
										set CH(s) [expr $E+$CV+1-$CH(cv)]
									} elseif {$j} {set CH(s) 0}

									if {$j} { set E $j } else { set S $j }
									# (UNFLUSH return code is implicitly coded)
							}
							"4"	 -
							"5"	 -
							"7"	 {	if {[set j [expr $CH(e)+$CH(cv)+1-$CV]]>$E}\
																	   {set j 0}
									if {$i != 7} {;# @4 & @5 edit into: s/S-1
										set CH(e) [expr $S+$CV-1-$CH(cv)]
									# @4 & @5:
									} elseif {$j} {set CH(s) 0}

									 set S $j
									# (UNFLUSH return code is implicitly coded)
							}
							"6"	 {	if {[set j [expr $CH(s)+$CH(cv)-1-$CV]]<$S}\
																	   {set j 0}
									if {[set i [expr $CH(e)+$CH(cv)+1-$CV]]>$E}\
																	   {set i 0}
									# After measuring Seg overhangs,
									# handle having both, either or neither
									if {$i && $j} {
										lassign "$S $j						   \
										[string toupper "$CH(typ)"] $CV"	   \
														CH(s) CH(e) CH(m) CH(cv)
										set S $i
									} else { set S [incr i $j] }
									# (UNFLUSH return code is implicitly coded)
							}
							}]} then {;# UNFLUSH:
								#	  REMOVE end of list and reinstall as staged
								lassign [lindex $g($CH(self)) end]			   \
													   CH(s) CH(e)] CH(m) CH(cv)
								if {"$CH(s)" == ""} {set CH(s) 0};# <- paranoia
								set g($CH(self)) [lreplace g($CH(self)) end end]
							}

						# YET, if that "prior other side" post was REALLY an Add
						# then the COMPARISON must be directed TO that other
						# side to check for a Del->Add collision (but THIS time
						# using ONLY (L/R) values as Adds simply DO NOT HAVE a
						# legitimate Ancestral value).
						} elseif {$MH(s) && [string is lower $MH(m)]
						 && (![set i [hCLIP $S $E  $MH(s) $MH(e)]]	  ||	  \
														   ($i%4) != ($i/4))} {

							# Not nearly as difficult as the above Del->Del
							# Just apply offset needed to get beyond the Add
							set i [expr $MH(e) - $S + 1]
							incr S	$i
							incr E	$i
							incr CV -$i;# maintain reference to Ancestral lines
						}

						# Post current Del Mark (if it hasn't been editted away)
						# by 1st FLUSHING the prior STAGED mark ... (if any)
						#	This "staging area" allows us to more easily edit
						# any individual element without ALWAYS having to "POP"
						# and "re-stage" an entire entry as we try to modify it.
						if {$S} {
							# Flush any presently staged item to its list...
							if {$MH(s)} {lappend g($MH(self))				 \
												  "$MH(s) $MH(e) $MH(m) $MH(cv)"
							}

							# ... then stage (ie. POST) the CUR Del for editting
							lassign "$S $E [string toupper $CH(typ)] $CV" \
														MH(s) MH(e) MH(m) MH(cv)
						}
					}

					# SIMILARLY, check (SAME CURhunk) for an Add component that
					# maybe collides with an EXISTING Del (posted from the
					# OTHER side, now on this side, designated as a "Add->Del")
					if {"[set TYP "$CH(typ)"]" != "d"} {
						# Dbg puts -nonewline "\n\tAdd\t\tNDX($NDX) \
						CHhunk($CH(hnk)) ndx:maxLh($ndx<>$maxLh) CH($CH(self))"
						# By defn, Add values are NEVER Ancestral, thus have
						# no need to ever view their comparisons as anything
						# more than the (L/R) values they ALL posess.
						set S $CH(s2)
						set E $CH(e2)
						set CV 0;	   # Thus its best if CV remains ZERO

						# Look for a Del collision on same side as the CUR Add
						if {$CH(s) && [string is upper $CH(m)]
						  && (![set i [hCLIP $S $E $CH(s) $CH(e)]]	   ||	  \
														   ($i%4) != ($i/4))} {
				#########################################
				# There are TWO forms of Add->Del collisions: "push" and "split"
				# push: is when the Add PRECEDES the Del (but still collides).
				#		Add takes precedence, so Del is "pushed" to be after it.
				#
				# split: The converse causes the Del to be "split" AT the Add
				#		start, with the remainder of the Del "pushed" after it.
				#
				# EITHER WAY, we end up with a MINIMUM of 2 postings (max 3),
				# but the FINAL 2 are always in 'Add', 'Del' sequence -
				#	   which is **NOT THE PRESENT ORDER** ...
				########################################

							# ... THUS - we FULLY PRE-swap them NOW!!
							# (which may make reading the entirety of the
							# switch block feel a bit backward - BE AWARE)
							lassign \
							"$S $E $TYP $CV $CH(s) $CH(e) $CH(m) $CH(cv)" \
							CH(s)  CH(e)  CH(m)	 CH(cv)		S  E  TYP  CV
							# NOW: CH(*) is the Add....others are the Del

							# Once again, re-CLIP Add Seg TO an INLINED range
							#	(reasons explained under Del->Del processing)
							switch [hCLIP $CH(s) $CH(e) $S+1 $E-1]	{
							"0"	 -
							"4"	 -
							"5"	 {; # Above cases ALL SPLIT the Del to 2 segs
								  # Flush the LEAD Del portion into the list,
								  lappend g($CH(self))						\
												"$S [incr CH(s) -1] $TYP $CV"

								  # Next, calc span of Add Seg, begin to update
								  # start of 2nd Del fragment, and repair CH(s)
								  set i [expr [set S $CH(e)] - [incr CH(s)] +1]

								  # lastly, finish edit to trailing Del startpt
								  incr S
							}
							default {; # Remaining cases ALL perform a "push"
								  # Calc span of Add to push Del past it
								  set i [expr $CH(e) - $CH(s) + 1]
								  incr S $i
							}}
							# EVERY case then exits thru this common code
							#	(which is completing a half-done 'push')
							incr E $i  ;# update endpt
							incr CV -$i;# and maintain Ancestral link
						}

						# Adds are NEVER "editted", so NO "if S==0" is needed.
						# Just flush the presently staged Add to the list...
						if {$CH(s)} {lappend g($CH(self))				 \
												"$CH(s) $CH(e) $CH(m) $CH(cv)"}

						# ... then stage REMAINING Del Segment for the future
						lassign "$S $E $TYP $CV"   CH(s) CH(e) CH(m) CH(cv)
					}

					# Curr hunk is now mapped ... "increment" to get NEXT one
					#	PROVIDED the Left side is NOT in "flush" mode -OR-
					#	the Right side is STILL flushing...
					if {$ndx < $maxLh} { eval $CH(nxt) } else break
				}
				# Dbl-check WHY we are getting a new RIGHT hunk; if its because
				# there are NO MORE "Lefts", we are in a "flush Right" state:
				#	Activate the "sneak path" to allow EACH NEXT Right INSIDE
				#	the above "while" loop for a single pass
				if {$ndx == $maxLh} {incr ndx}
			}
		}

		# Everything has been mapped, simply FLUSH the final staged values
		if {$diffRC} {
			lappend g($CH(self)) "$CH(s) $CH(e) $CH(m) $CH(cv)"
			lappend g($MH(self)) "$MH(s) $MH(e) $MH(m) $MH(cv)"
			# Dbg puts ""	N.B> Uncommenting ALL 'Dbg puts' lines will produce
			# a TRACE of the Ancestral processing flow (must remove 'Dbg' as
			# well as Dbg doesn't ACCEPT the "-nonewline" flag: for now anyway)
		}
	}

	if {!$diffRC && $g(is3way)} {
		# Apparently we EXPECTED to do a 3way, but errors have prevented it!
		#	DROP the 3way state, but ALLOW the 2way to continue (if it can)
		array unset finfo "a\[ptl]*,$pairnum"; # FORGET about Ancestor request
		alignDecor $pairnum ;				# then RESET state/Decor from 3->2
		do-show-Info ;					# including room for Ancestral Markers
		popmsg "Attempted 3way Diff was cancelled\n	   (due to prior errors)" \
				warning "Unable to Comply"
	}
	Dbg {Left  ancestral datums: [llength $g(d3Left)]}
	Dbg {Right ancestral datums: [llength $g(d3Right)]}

	# Mark up the two text widgets and go to the first diff (if there is one).
	# Otherwise BLANK the combobox (in case it has old data from a PRIOR diff)
	if {$diffRC} {show-status "Marking differences..."}

	if {![mark-diffs]} {
		$w(combo) configure -commandstate disabled
		$w(combo) configure -value {}
		$w(combo) configure -commandstate normal
		if {"$Statmsg" == ""} {
			# Unless something FORCED us to not perform/complete the
			# diff, this HAS to be the reason there just ARE zero hunks!
			# BUT - dont lie - if there are SUPPRESSED hunks
			# then it really only APPEARS this way
			if {$g(COUNT) > $g(count)} {
				set Statmsg "Files now APPEAR as identical"
			} { set Statmsg "Files are identical" }
		}
		eval after idle {show-status "{$Statmsg}"}
	} else { move first 0 1 }
	# NEEDED update-display should be handled by caller
}

###############################################################################
# Convert "Unified" hunk format into "Normal" format   (if that is what exists)
# (Caveat: MAY alternately BE in "Normal" format: just w/leading COMMENT lines)
###############################################################################
proc deUnify { Var } {
	upvar $Var var

	# REWRITE the data as Diff "Normal" IFF currently "Unified"
	set Uhdr -1 ; # Ignore EVERY line until 1st 'Unified' hunk header found
	set Nhdr -1 ; # Ignore EVERY line until 1st 'Normal'  hunk header found
	set typ	 {} ; # Ignore context lines until a (+/-) marked line found

	foreach ln $var {

		# Diff "Normal" fmt is ordinarily IDENTIFY'd by its header on LINE#1
		#		But we needed to allow 'prelude' lines (for debugging reasons)
		#			-- NOT UNLIKE Unified 'file-header' lines --
		#		SO if THATs what this IS, just swallow THOSE and REWRITE
		#		everything else VERBATIM (incl. this FIRST 'Normal' header)
		# (N.B> logic test simply locks IN-or-OUT which REWRITE is being done!)
		if {$Uhdr < 0 && ($Nhdr > 0
		||	[string match {[0-9]*[acd][0-9]*} [lindex $ln 0]])} {
			# Rewrite line list: ONCE!	('foreach' keeps READING the original)
			# Also stops flushing ALL 'Normal' file-prelude lines
			if {$Nhdr < 0} { set Nhdr [llength [set var [list]]] }
			lappend var $ln
			continue
		}

		# Find a Unified hunk header (if found 1st, will LOCKOUT above code)
		if {[string match {@@ * * @@*} $ln]} {

			# Extract line numbers/counts per this 'Unified' hunk header
			regexp {@@ -([0-9]*)(,([0-9]*))?(?: \+)([0-9]*)(,([0-9]*))?.*} $ln\
					  matchvar L na Lc R na Rc
			if {$Lc=={}} { set Lc 1 } ; if {$Rc=={}} { set Rc 1 }

			# Rewrite line list: ONCE!	('foreach' keeps READING the original)
			# Also stops flushing ALL 'Unified' file-header lines
			if {$Uhdr < 0} { set Uhdr [llength [set var [list]]] }

		} { # Skip 'context'/'identical' lines (but decrement from counts)
			#	(or just categorically skip ALL the fileheader nonsense)
			if {[set mrk [string index $ln 0]]==" " && $Uhdr >= 0} {
				# But if any REAL lines INTERVENED, make THOSE a 'Normal' hunk
				if {$typ != {}} {
					set Rsiz [expr [llength $var] - $Uhdr - $ofst]
					set					 hdr	  [expr {$typ=="a" ? $L-1:$L}]
					if {$ofst>1} {append hdr ","  [expr $L+$ofst-1]}
								  append hdr $typ [expr {$typ=="d" ? $R-1:$R}]
					if {$Rsiz>1} {append hdr ","  [expr $R+$Rsiz-1]}

					set var [linsert $var $Uhdr $hdr]
					incr L $ofst ; incr R $Rsiz ; set typ {}
				}
				incr L ; incr Lc -1 ; incr R ; incr Rc -1 ; continue
			} elseif {$Uhdr < 0} { continue }

			# Found a marked line to post; Perhaps begin a NEW 'Normal' hunk ?
			if {$typ=={}} { set Uhdr [llength $var] ; set ofst 0 }

			# Assign WHERE the (+/-) line goes (and detect overall hunk type)
			switch -- "$mrk" {
			"+" {	if {$typ=="c" || $typ=="d"} { set typ "c" } { set typ "a" }
					lappend var [string repl $ln 0 0 ">"]
					incr Rc -1
			}
			"-" {	if {$typ=="c" || $typ=="a"} { set typ "c" } { set typ "d" }
					set var [linsert $var $Uhdr+$ofst [string repl $ln 0 0 "<"]]
					incr Lc -1
					incr ofst
			}}

			# If both 'Unified' counts hit zero HERE, install final 'Normal'
			#	header (only NEEDED when current (+/-) line has NO trailing
			#	context BECAUSE it was the LAST PHYSICAL LINE of ENTIRE FILE)
			if {!($Lc+$Rc) && $typ!={}} {
				# Assemble VERY LAST "Normal" hdr
				set Rsiz [expr [llength $var] - $Uhdr - $ofst]
				set					 hdr	  [expr {$typ=="a" ? $L-1:$L}]
				if {$ofst>1} {append hdr ","  [expr $L+$ofst-1]}
							  append hdr $typ [expr {$typ=="d" ? $R-1:$R}]
				if {$Rsiz>1} {append hdr ","  [expr $R+$Rsiz-1]}

				# (can ignore any POST-hdr numbering fixups - unneeded)
				set var [linsert $var $Uhdr $hdr"]
				incr L $ofst ; incr R $Rsiz ; set typ {}
			}
		}
	}
}

###############################################################################
# Set the X cursor to "watch" for a window and all of its descendants.
#
# An optional msg 'WHY' will post to the status area (when BOTH exist); if the
# '.status' window DOESN'T exist yet, one **MAY** be temporarily built, and the
# reason posted there, PROVIDED it takes longer than a specifiable delay(in ms)
# to REACH the code that can cancel the need (ie- the message is only IMPORTANT
# if the GUI isn't inplace yet AND the action we elected to be BUSY about takes
# randomly longer than someone can withstand waiting for feedback:
#	Prime example:	hung networks or simply unpredictable latency.
###############################################################################
proc watch-cursor {{WHY {}} {delay 1250}} {
	global g w ASYNc

	# Cant 'busy' out windows that arent't there yet ...
	if {[winfo exists w(client)]} {
		. configure -cursor watch
		$w(client) configure -cursor watch
		$w(toolbar) configure -cursor watch
		$w(menubar) configure -cursor watch
		if {$WHY != {}} {show-status "$WHY"}
		update idletasks

	# ... but if we gave a REASON WHY - someone should see THAT reasonably soon
	} elseif {$WHY != {}} {
		# Thus we want to REQUEST a status window be built after a short delay;
		#	HOWEVER if we can complete the "busy" task and get back in time to
		# CANCEL the request, the user need NEVER see it -BUT- that means
		# changing to ASYNC processing for any external tasks we might spawn or
		# we will NEVER see the timer fire (it needs a running event loop).
		#	This temp Status window IS removed @pgm exit (on failures), OR as
		# soon as we replace it with the main GUI (eg. success). Once built,
		# future Busy/Unbusy pairs simply USE whatever status window exists.
		if {![winfo exists .status]} {
			#	So post the timer, SAVING its ID in a global whose EXISTENCE
			# will be used as a flag, so 'run-command' operates in ASYNC mode.
			# (N.B> but only the first one to get here can actually post)
			if {![info exists ASYNc(trigger)]} {
				set ASYNc(trigger) [after $delay need-status "{$WHY}"]
				Dbg "Posted ASYNc(trigger)($ASYNc(trigger))"
			}
		} else { show-status "$WHY" }
		update idletasks
	}
}

###############################################################################
# Give the user SOMETHING to look at while they wait
#
# N.B> if processing hangs, clicking the window 'exit' decoration will kill pgm
###############################################################################
proc need-status {WHY} {
	global g w

	set w(status) .status
	build-status
	pack $w(status) -side bottom -fill x -expand n
	wm deiconify .
	# N.B> Protocol only works IFF event loop is RUNNING (to see 'after' event)
	show-status $WHY
	update idletasks
}

###############################################################################
# Restore the X cursor for a window and all of its descendants.
###############################################################################
proc restore-cursor {} {
	global w ASYNc

	if {[winfo exists w(client)]} {
		. configure -cursor {}
		$w(client) configure -cursor {}
		$w(toolbar) configure -cursor {}
		$w(menubar) configure -cursor {}
		show-status ""
		update idletasks
	} elseif {[info exists ASYNc(trigger)]} {
		# If got here in time ... cancel setting up the status window
		# Regardless, reset to synchronous IO -- future attempts will either
		# REMAIN synchronous (w/existing Status window), or retrigger ASYNc
		if {![winfo exists .status]} {
			after cancel $ASYNc(trigger)
			Dbg "Cancelled ASYNc(trigger)($ASYNc(trigger))"
		}
		unset ASYNc(trigger)
	}
}

###############################################################################
# Check if error was thrown by us or unexpected
###############################################################################
proc check-error {result output} {
	global errorInfo

	if {$result && $output != "Fatal"} {
		error $result $errorInfo
	}
}

###############################################################################
# Recalc current diff (and optionally ALIGN) the semantic [preference] reason
# Attempt to return to the same diff region, numerically speaking.
#	(used to force a redo when a changed semantic could AFFECT the Diff result)
###############################################################################
proc reCalcD {reason {RevAlgn {}}} {
	global g tmpopts opts finfo

	# Optionally permits REVERSE-aligning the preference setting
	#	N.B> used in odd situation where invoked by MENU instead of Pref Dialog
	#		Prevents DIALOG from REACTING to anything CHANGED....
	#
	#		...doing tmpopts/opts assignments in the REVERSE ORDER nullifies
	#		many NORMAL "validation" operations by effectively triggering on
	#		the TRAILING EDGE (when making them IDENTICAL) instead of the
	#		LEADING EDGE (when they first DIVERGE). Its all about which Var is
	#		PHYSICALLY being watched (ie. tmpopts within the Dialog).
	if {$RevAlgn eq "RevAlgn"}	{
		set tmpopts($reason) $opts($reason)
		if {$reason=="ignSuprs"} {
			set tmpopts(diffcmd) [set opts(diffcmd) [formOpts egnCmd]]
		}
	}

	# Just go DO the Diff
	set	 ndx(1) [set ndx(2) [expr {$finfo(fCurpair) * 2}]]
	incr ndx(1) -1
	#N.B> Silently ignored IFF subject data is missing (SCM untried or failed)
	if {$finfo(pth,$ndx(1)) != {} && $finfo(pth,$ndx(2)) != {}} {
		Dbg "Forcing recompute via $reason pref change"
		set current $g(pos)
		do-diff
		move $current 0 1
		centerCDR
	}
}

###############################################################################
# Wipe most everything (data plus widget content), then kick off a rediff
###############################################################################
proc do-diff {} {
	global g tmpopts opts

	wipe-window

	watch-cursor
	update idletasks

	# If we are HERE and there is a DEFERRED Diff pending, its clearly time
	# to STOP deferring - in fact, that deferral REASON should now become
	# "Apply"d (as the user was SUPPOSED to have done before now). This ensures
	# the RESULTS of the Diff AGREE with the settings CURRENTLY provided.
	if {[info exists g(deferD)]} {
		foreach key $g(deferD) { set opts($key) $tmpopts($key) }
		unset g(deferD) ;# Then Cancel any PENDING redo (see prefapply)
	}

	set result [catch { rediff } output]

	check-error $result $output

	if {$g(mergefileset)} { do-show-merge 1 }
	restore-cursor
}

###############################################################################
# start a new diff from the popup dialog
###############################################################################
proc do-new-diff {} {
	global g finfo

	# Unlock the PRESENT mergefile settings (but leave name for now), then ...
	# Pop up the dialog to collect the args and form them together
	# into a command - bailing out if dialog cancels or args is malformed
	set g(mergefileset) 0
	if {![newDiff] || ![assemble-args]} return

	# make new file args available then do the diff
	multiFile reload
	do-diff

	move first 1 1

	update-display
}

###############################################################
# Convert from hunk-index
#	a 1-based monotonic difference position (called a hunk)
# to hunk-id
#	a diff-encoded (nnn[acd]mmm) descriptive format
###############################################################
proc hunk-id { ndx {lst diff}} {
	global g

	# lst:	  'DIFF'  (superset diffs) has ALL hunks (inclds IGNORED)
	#		  'diff'   (subset diffs)  has only REAL hunks
	# Both lists expect to NOT have a *dummy* index-0 element
	lindex $g($lst) [incr ndx -1]
}

###############################################################
# Convert from hunk-id
#	a diff-encoded (nnn[acd]mmm) descriptive format
# to hunk-ndx
#	a 1-based monotonic difference position (called a hunk)
###############################################################
proc hunk-ndx { id {lst diff}} {
	global g

	# lst:	  'DIFF'  (superset diffs) has ALL hunks (inclds IGNORED)
	#		  'diff'   (subset diffs)  has only REAL hunks
	# Both lists expect to NOT have a *dummy* index-0 element
	expr { 1 + [lsearch -exact $g($lst) $id] }
}

###############################################################################
# Get things going...
###############################################################################
proc main {} {
	global g w opts finfo ASYNc startupError errorInfo tk_version

	wm protocol . WM_DELETE_WINDOW do-exit
	wm title . "$g(name) $g(version)"
	wm iconphoto . -default deltaGif

	if {$w(wSys) == "x11"} {
		# All this nonsense is necessary to use an icon bitmap that's
		# not in a separate file.
		toplevel .icw
		if {[string first "color" [winfo visual .]] >= 0} {
			label .icw.l -image deltaGif
		} else {
			label .icw.l -image delta48
		}

		pack .icw.l
		bind .icw <Button-1> "wm deiconify ."
		wm iconwindow . .icw

		if {![get_gtk_params]} {
			Dbg "Default x11 (fallback) options established"
			set hlbg "#4a6984"
			set hlfg "#ffffff"
			option add *Menu.selectColor $w(fgnd)
			option add *Checkbutton.selectColor ""
			option add *Radiobutton.selectColor ""
		}
	}
	if {($w(wSys) == "aqua") && ($tk_version < 8.6)} { get_aqua_params }

	# Ordinarily the CWD points at/under the TOPMOST Vpath node (IFF they exist)
	# But *IFF* located deeper, use that position to TOP-prune the lead elements
	#	(allows user to review "versions" earlier than the most recent two
	#	 WITHOUT having to explicitly modify the VPATH EnvVar itself)
	if {[info exists finfo(Vpath)]} {
		if {[set finfo(Vpofst) [is-vpath "."]]} {
			set finfo(Vpath) [lrange $finfo(Vpath) [incr finfo(Vpofst) -1] end]

			# Further, adjust EACH Vpath NODE to designate itself as within any
			# SUBDIRs implied by CWD being farther than the stated VPATH edge
			set len [string length [lindex $finfo(Vpath) [set i 0]]]
			if {[set subdir [string range [pwd] $len end]] != {}} {
				foreach node $finfo(Vpath) {
					lset finfo(Vpath) $i "${node}$subdir" ; incr i
				}
			}
		}
		Dbg {Current (pruned($finfo(Vpofst))) EFFECTIVE\
										Vpath\n\t[join $finfo(Vpath) "\n\t"]}
	}

	#	Begin by interpolating command args
	#
	#		'CmdLn' may EXIT if args are INCORRECT/INVALID, or pass
	#	control to 'newDiff' if simply missing; EITHER of which will,
	#	in turn, invoke 'assemble-args' to OBTAIN the first (or only)
	#	pairing of actual files to DIFF. If MULTIPLE pairs resulted from
	#	that proc, SUBSEQUENT pairings will be chooseable via the GUI.
	#	Insufficient pairings results in a "Retry" or an "Abort" based on
	#	having USED 'newDiff' or 'CmdLn' respectively, AND further
	#	conditioned on having encountered a HARD error (.vs. a warning).
	#
	# N.B> newDiff must KNOW if CLIENT windows exist: provide NAME for checking
	set w(client)  .client ;# (to be built shortly by 'create-display' below)

	if {[CmdLn] > 0 || [newDiff]} {
		while {[set rcod [assemble-args]] < 2} {
			# REMOVING this clause provides NewDiff as a FAILED-CmdLn retry)
			if {$rcod > 0 && ![winfo exists .newDiff]} {
							fatal-error "Insufficient usable input"}
			if {![newDiff]} {do-exit}
		}
	} else {do-exit}

	# The ONLY WAY this exists is if 'assemble-args' was forced
	# to warn about delayed SCM access time - get rid of it
	#	(and any lingering ASYNC processing condition)
	if {[info exists w(status)]} {
		wm forget $w(status)
		unset -nocomplain ASYNc(trigger)
		Dbg "ASYNC mode has been dropped"
	}

	set g(startPhase) 1

	create-display

	# Evaluate any custom code the user MAY have provided
	if { [string trim $opts(customCode)] != {}} {
		Dbg "Custom code IS in use...beware"
		if {[catch [list uplevel \#0 $opts(customCode)] error]} {
			set startupError "Error in custom code: \n\n$error"
		}
	}

	# Populate the multiFile list with any OTHER file pairs-in-waiting
	multiFile reload

	# Finally DRAIN anything still pending in the eventloop
	update

	do-diff

	update-display

	# kick the HORIZONTAL scroll in the main windows
	# (VERTICAL already happenned by now, which only MAY have induced this)
	hscroll-sync 1 0
	hscroll-sync 2 0

	wm deiconify .
	update idletasks

	if {[info exists startupError]} {
		popmsg	$startupError warning "Error in Startup File"
	}
}

###############################################################################
# Erase tmp files (if necessary) and destroy the application.
###############################################################################
proc del-tmp {} {
	global g

	foreach f $g(tempfiles) {
		file delete $f
	}
}

###############################################################################
# Put up a window with formatted text
###############################################################################
proc do-text-info {win title text} {
	global g w opts

	if {![Dialog NONMODAL $win]} {
		wm title $win "$g(name) Help - $title"
		wm transient $win .
		wm	 group	 $win .
		# we could leave this off (its what TK would do anyway) but...
		wm protocol $win WM_DELETE_WINDOW "destroy $win"

		set width 64
		set height 32

		# grid the button BEFORE its sibling frame
		#	(thus making it LOWER/LATER in the stacking/clipping order)
		# N.B> Note that the Dismiss button is NOT using a 'ctrlvar' setting
		#	(see explanantion at Bottom of this proc)
		grid [button $win.done -text Dismiss -command "destroy $win"] \
									-row 1 -column 0 -sticky se -pady 5 -padx 5
		grid [frame $win.f -bd 2 -relief sunken] -row 0 -column 0 -sticky news
		grid rowconfigure	 $win 0 -weight 1
		grid columnconfigure $win 0 -weight 1

		text $win.f.title -height 2 -width 50 -wrap word -bg white -fg black \
			-highlightthickness 0 -bd 0

		# Convenience Help info
		#	HOWEVER - this Dialog instance MAY be being built BEFORE the MAIN
		#	client windows FROM WHICH we want to steal its Fg/Bg colors...
		# Perhaps there's a more elegant way to DO this, but create a DUMMY
		# window we can suck the client PREFs into, to then steal from IT!!
		if {![winfo exists $w(client)]} {
			set w(acTxWdg) [text $win.dummy {*}$opts(textopt)]
		}
		text $win.f.text -setgrid 1 -width $width -height $height -wrap word \
			-yscroll [list $win.f.vsb set] -padx 20 -highlightthickness 0 \
			-bd 0 -bg [$w(acTxWdg) cget -bg] -fg [$w(acTxWdg) cget -fg]

		# (just remember to cleanup the dummy window afterward)
		# N.B> w(acTxWdg) *WILL BE* the subject of a trace ... just not YET!!!
		if {![winfo exists $w(client)]} { destroy $w(acTxWdg) }

		scrollbar $win.f.vsb -orient vertical -command "$win.f.text yview" -bd 1

		pack $win.f.vsb	  -side right -fill y	 -expand n
		pack $win.f.title -side top	  -fill x	 -expand n
		pack $win.f.text  -side left  -fill both -expand y

		if {$g(debug)} {
			# Silly idea - writing the raw help text out for printing ...
			#	(make the button hide in plain sight); ??convert to manpage??
			button $win.write -text {} -relief flat -takefocus 0 -command \
				"set pth \[tk_getSaveFile -parent $win.f.text -initialdir {.}]
				 if {\$pth != {}} {
					 puts \[set pth \[open \$pth w]] {<ttl>$title</ttl>\n$text}
				 close \$pth}"
			grid $win.write -row 1 -column 0 -sticky sw -pady 5 -padx 5
		}

		put-text $win.f.title "<ttl>$title</ttl>"
		put-text $win.f.text $text
		$win.f.title configure -state disabled
		$win.f.text	 configure -state disabled
		update idletasks

		# Only how big - NOT where to put it!
		wm geometry $win ${width}x${height}
	}
	Dialog show $win
}

###############################################################################
# centers window 'win' over parent
###############################################################################
proc centerWindow {win {size {}} {persist 0}} {

	update
	set parent .

	# Last two optional args are permitted to be in EITHER order
	#	AND can be distinguished by their CONTENT - not just their sequence
	#	(Rearrange them if they were specifed in backward order)
	if {[llength $size] == 1} {
		set x $persist ; set persist $size ; set size $x
	}

	# What follows here has to do with WHEN the centering was being requested
	# AND what data might NOT be particularly reliable (such as the dimensions
	# of  the window BEING centered -OR- the parent being centered ON:
	#	'size'	= empty (normal) says: use the data of the windows themselves
	#			= 2 values: use these AS the size of the window TO center
	#			= 4 values: as for =2, but use 3&4 as the PARENT dimensions
	# Note that if WxH of 'win' has been GIVEN as (0 0) (from either syntax)
	# that is the same as requesting that the window itself provide its size

	if {[llength $size] > 1} {
		lassign [concat $size  0 0] wWidth wHeight pWidth pHeight
	}
	if {[llength $size] < 4} {
		set pWidth	[winfo reqwidth	 $parent]
		set pHeight [winfo reqheight $parent]
	}
	if {[llength $size] < 2 || ($wWidth==0 && $wHeight==0)} {
		set wWidth	[winfo reqwidth	 $win]
		set wHeight [winfo reqheight $win]
	}
	Dbg {centering(${wWidth}x$wHeight) onto parent(${pWidth}x$pHeight)}

	# get on with the centering
	set pX [winfo rootx $parent]
	set pY [winfo rooty $parent]

	set centerX [expr {$pX +($pWidth  / 2)}]
	set centerY [expr {$pY +($pHeight / 2)}]

	set x [expr {$centerX -($wWidth	 / 2)}]
	set y [expr {$centerY -($wHeight / 2)}]

	# Can NEVER set WxH in PIXELS if window is GRIDDED !! Only the location
	if {[llength $size] > 0 && [wm grid $win]=={}} {
		wm geometry $win [set ctr "=${wWidth}x${wHeight}+${x}+${y}"]
	} { wm geometry $win [set ctr "=+${x}+${y}"] }
	Dbg {Centering has targeted at $ctr above parent @($pX,$pY)}

	# OK - if I understand this correctly, the geometry request will not ONLY
	# make the window respond as asked, but will be RETAINED by the window
	# even THROUGH a 'withdraw'/'deiconify' cycle, thereby REINSTATING that
	# position - even if the user happenned to drag the window elsewhere AFTER
	# it had been first displayed as 'centered' (often as a convenience).
	#	But OUR moving the window involves a <Configure> event, which is WHY:
	update
	# has been provided (to LET that event happen). Thus after that completes
	# and presuming we were not told (via 'persist') to LEAVE it that way,
	# schedule a near future request to REMOVE that geometry setting, allowing
	# the window to revert to TRACKing any interactive USER-MADE adjustments.
	if {!$persist} { after idle wm geometry $win {} }
}

###############################################################################
# Tcl V8->V9 has different semantics concerning TILDE as 1st letter of filename
#	V8	treats	~ as $HOME	(throwing [undocumented] error if $env(HOME) unset)
#				~xxx as USER xxx's $HOME (throwing error if user is unknown)
#		all perfomed IMPLICITLY by 'glob' and *MOST* 'file' commands
#		(notably NOT for 'file exists' despite ~xxx expansion being attempted)
#		Apparent Logic is dependent on IFF file must exist to perform operation
#		Thus 'isdirectory' and 'isfile' also APPEAR as error-free
#
#	V9	treats	~ as a conventional character everywhere BY DEFAULT, BUT -
#			provides an explicit 'file tildeexpand' cmd to essentialy MAP $name
#			returning it (unchanged when no lead tilde is present) but throw
#			errors just as V8 did (although V9 DOCUMENTS *both* reasons)
#
#	This Proc isolates the V8/V9 transition, and supports BOTH semantics
#	but with a PREFERENCE of tilde-notation over that of tilde-as-a-lead-char
#
# N.B>	Despite GLOB use internally (implicit for V8, explicit for V9), ACTUAL
#		NAMES are never returned (via canon); only a tilde-notation FREE form
#		of the original input name (glob-syntax and all). HOWEVER 'result' WILL
#		contain the COUNT of names that WILL BE produced (0->N) when the caller
#		passes 'canon' INTO a V8/V9 glob which WILL NOT fault!
#
# For TkDiff, TOO MANY names (>1) is pointless and is LOGICALLY the same as (0).
###############################################################################
proc tildChk {fnam result canon} {
	global	tcl_version
	upvar $result resptr $canon canptr	 ;# point OUR locals AT callers Vars

	# INITIALIZATIONS:
	set D  -1 ;# Local master-Dbg authorizer (in case parse issues arise later)
	# Presume V8 will NOT find a literal-tilde usage (until proven otherwise)
	set V8plainT 0
	# Isolate first fragment from ANY subsequent GLOB syntax
	set f1 [lindex [set frags [file split $fnam]] 0]
	Dbg {SPLIT frags($frags) INITIALIZATION} $D "tildChk: "

	# CALLER is responsible for which returned values (RetCode/result/canon)
	# are of logical importance within ITS specific CALLING situation
	#	Original V8 check produced 3 possible 'result' value sets (w/o errs):
	#		0		- fnam doesnt exist							w/RetCod=0
	#		1		- fnam	does  exist							w/RetCod=0
	#		FailMsg - tilde conversion failed (no such user)	w/Retcod=1
	#	V9 adds an ADDITIONAL output datum (canon) to those sets:
	#		0		- defaulted as fnam CLONE					w/Retcod=1
	#		1		- canonical form of converted fnam			w/Retcod=0
	#		FailMsg - tilde conversion failed (no user/home)	w/Retcod=1
	#
	# User EXPECTED to prefix initial fnam with './' to PREVENT tildeexpand,
	# although code ATTEMPTS such evaluation if tildeexpand was IMPOSSIBLE

	if {($tcl_version >= 9.0) && ("~" == [string index $fnam 0])} {
		# Perform tilde expansion on JUST 1st frag, OR (if that fails)
		#	a DIRECT filename search, again on only that 1st fragment
		if {![set Err [catch {file tildeexpand $f1} newf1]]
		||	 [llength [set nms [glob -n $f1]]]} {
			# If Err non-zero HERE, there IS a CWD-relative LITERAL tilde name
			#	(of which there MUST be only a SINGLE name via the V9 GLOB)
			# TWO possible ways to success:
			if {$Err && [llength $nms] == 1} {
				# reassemble fnam w/FOUND literal tilde as 1st frag, THEN ...
				set fnam [file join $nms {*}[lrange $frags 1 end]]
			} elseif {!$Err} {
				# reassemble fnam using tilde EXPANDED 1st fragment, THEN ...
				set fnam [file join $newf1 {*}[lrange $frags 1 end]]
			} else {
				# failed: expansion was IMPOSSIBLE (AND found no LITERAL tilde)
				set resptr $newf1;	# shove msg where it belongs
				set canptr $fnam;	# has NO canonical form BEYOND itself: clone
				return $Err;		# tell caller theres a DEFINITE problem
			}
			# ...EXIT via GLOB code below as it'll GLOB from PROPER (HOME/CWD)
			# (N.B> catch is superfluous as V9 GLOB has no reason to fault)
		} else {
			# failed: expansion was IMPOSSIBLE (AND found no LITERAL tilde)
			set resptr $newf1;	# shove msg where it belongs
			set canptr $fnam;	# has NO canonical form BEYOND itself: clone
			return $Err;		# tell caller theres a DEFINITE problem
		}
	# replicate default V9 semantic tildeexpand behavior as PREPARATION for V8
	#		(eg. PRESUME it has NO canonical form BEYOND itself: clone it)
	#	N.B> BUT return 'EXISTS' & '!Err' when *NO* INPUT name	was provided!!
	} elseif {[set resptr [expr {[string index [set canptr $fnam] 0] == {}}]]} {
		return 0 ;# an EMPTY filename MUSTN'T infer the CWD: as next GLOB would!
	} elseif {$tcl_version >= 9.0} { set newf1 [glob -n $f1]
		Dbg {Default V9 1st Frag newf1($newf1) to later update canon} \
		$D " + + + : "
	}

	# (N.B> V8/V9 BOTH rely on GLOB to provide filesystem EXISTENCE of result)
	#	 yet REQUIREs catch as V8 glob w/tilde can FAIL (on '~' or '~userid')
	# However, V8 THEN gets a 2nd try by SUBVERTING any lead-tilde (if present)
	# (N.B> V9 CANT RAISE an error as its GLOB wont treat tildes special)
	if {![set Err [catch {llength [glob -n $fnam ]} resptr]]
	|| ($tcl_version < 9.0 && [set V8plainT [llength [glob -n "./$fnam"]]])} {
		# (above checks for POSSIBILITY of a V8 plain-tilde filesystem NAME

		if {$tcl_version < 9.0} {
			if {$Err}  {set newf1 [glob -n "./$f1"]}  {set newf1 [glob -n $f1]}
			Dbg {Default V8 1st Frag newf1($newf1) to later update canon} \
			$D " + + + : "
		}

		# INFER the WORKING tilde-notation is PREFERd over a literal-tilde NAME
		#	'V8plainT' is ONLY true when (V8 Err==1 + resptr w/msg) - allowing
		#			   overriding BOTH in FAVOR of a FOUND literal-tilde NAME!
		if {$V8plainT} { set Err 0 ; set resptr $V8plainT }
	}

	# Update 'canon':
	#	NORMALLY when 'resptr' is false, 'canon' could just REMAIN the default
	#	CLONE of 'fnam' -- but that could MISS the CONVERTED (!Err) tilde,
	#	simply because of GLOB notation ultimately failing to match anything!
	if {!$Err} {
		# Provide at least the GOOD TILDE so caller doesn't TRIP over it ...
		set canptr [file join $newf1 {*}[lrange $frags 1 end]]
		Dbg {REWROTE result($resptr) CANON($canptr) w/NEW 1st frag($newf1)} \
		$D "tildChk: "
	}
	return $Err
}

###############################################################################
# The "New Diff" dialog
# In order to be able to enter only one filename if it's a revision-controlled
# file, the dialog now simply collects the arguments and sends them through the
# SAME argument analyzer used by the command line parser.
###############################################################################
proc newDiff {} {
	global g w finfo opts pref

	# Snapshot the current state of the primary INPUT variables into a global
	# location (so that the 'Cancel' button callback can find/restore them)
	#	N.B> only preserves FINFO data (sadly, OTHER chgs will remain active)
	set g(NDpriorVals) [array get finfo {[fr]*[0-2]}]

	# Special case: on a SUBSEQUENT invocation, verify that the dialog IS
	#				designated AS 'transient' (which it would NOT have been
	#				if it was the FIRST window to be created. This MUST be
	#				done while the window is "withdrawn" BEFORE redisplaying.
	if {[winfo exists [set w(newDiff) .newDiff]]
	&& [wm transient $w(newDiff)] == ""} { wm transient $w(newDiff) . }

	if {![Dialog MODAL $w(newDiff)]} {
		wm title	$w(newDiff) "New Diff		 $g(name) $g(version)"
		wm group	$w(newDiff) .
		wm protocol $w(newDiff) WM_DELETE_WINDOW	{ set w(NewDok) 0 }

		# CAN't start as the FIRST window on Windows if it's made 'transient'
		#	N.B> This is the CAUSE of the above "Special case" adjustment
		if {[winfo exists .client]} {
			wm transient $w(newDiff) .
		}

		set fSpec [frame $w(newDiff).fSpec -borderwidth 2 -relief groove]

		# N.B> Widget name derivation is constrained by various callbacks...
		#	   - 'newDiffBrowse' uses the LAST letter of their pathname to
		#		 implement a 'shared directory path' protocol among the TWO
		#		 main entry widgets 'e1' and 'e2'; a NON-digit last-char
		#		 AVOIDS the protocol, but WILL work for the Ancestor usage
		#	   - TWO main label widgets 'l1' and 'l2' reflect when associated
		#		 ENTRY widget refers to a Non URL/existant-filesystem entity
		#	   - Revision labels reflect when their ENTRY is non-null
		#	   - SCM lists dynamically derive from the Entry values, and then
		#		 further determines and SETs searchability.
		#	Thus 'scm-updat' callback knows WAY TO MUCH about almost everything
		#	Be Carefull!!
		label $fSpec.l1 -text "FSpec 1:"
		entry $fSpec.e1 -vcmd {scm-updat scm1 %W %P [string length %s]} \
													   -textvariable finfo(f,1)
		entry $fSpec.er1 -textvariable finfo(rev,1) \
					  -vcmd "verify occupancy [label $fSpec.lr1 -text "-r"] %P"

		label $fSpec.l2 -text "FSpec 2:"
		entry $fSpec.e2 -vcmd {scm-updat scm2 %W %P [string length %s]} \
													   -textvariable finfo(f,2)
		entry $fSpec.er2 -textvariable finfo(rev,2) -validate key \
					 -vcmd "verify occupancy [label $fSpec.lr2 -text "-r"] %P"
		$fSpec.er1 configure -validate key;# allow validation AFTER .er2 exists
		$fSpec.er1 validate
		$fSpec.er2 validate

		label $fSpec.lA -text "Ancestor:"
		entry $fSpec.eA -vcmd {scm-updat ancstr %W %P [string length %s]} \
													   -textvariable finfo(f,0)
		entry $fSpec.erA -textvariable finfo(rev,0) -validate key \
					 -vcmd "verify occupancy [label $fSpec.lrA -text "-r"] %P"
		$fSpec.erA validate

		set mrgopt [frame $fSpec.f4] ;# pre-pack all this
		  label $mrgopt.l4 -text "$pref(predomMrg):" -anchor w
		  radiobutton $mrgopt.r1 -variable opts(predomMrg) -text Left  -value 1
		  radiobutton $mrgopt.r2 -variable opts(predomMrg) -text Right -value 2
		  pack $mrgopt.l4 $mrgopt.r1 $mrgopt.r2 -side left -pady 6

		# Now create the SCM comboboxes + 'labels' (tying them to the entry box)
		::combobox::combobox $fSpec.scm1 -editable 0 -listvar finfo(scm1) \
											-width 10 -command "scm-updat srch"
		::combobox::combobox $fSpec.scm2 -editable 0 -listvar finfo(scm2) \
											-width 10 -command "scm-updat srch"
		checkbutton $fSpec.scm1lbl -offrelief flat -onvalue 1 \
									  -command "scm-updat set $fSpec.scm1lbl 1"
		checkbutton $fSpec.scm2lbl -offrelief flat -onvalue 2 \
									  -command "scm-updat set $fSpec.scm2lbl 2"
		$fSpec.e1 configure -validate key
		$fSpec.e2 configure -validate key
		$fSpec.eA configure -validate key

		# We need the Browser buttons to fit the COMBINED height of BOTH the
		# filename entry and revision fields, so pre-pack it into a subframe
		set Brws1 [labelframe $fSpec.fB1 -text "Browse..."]
			button $Brws1.bf -borderwidth 1 -highlightthickness 1 -image \
				   txtfImg -command [list newDiffBrowse	  "File"	$fSpec.e1]
			button $Brws1.bd -borderwidth 1 -highlightthickness 1 -image \
				   fldrImg -command [list newDiffBrowse "Directory" $fSpec.e1]
			pack $Brws1.bf -padx {7 0} -pady {0 2} -side left
			pack $Brws1.bd -padx {0 7} -pady {0 2} -side right
			set_tooltips $Brws1.bf {"to a file"}
			set_tooltips $Brws1.bd {"to a directory"}

		set Brws2 [labelframe $fSpec.fB2 -text "Browse..."]
			button $Brws2.bf -borderwidth 1 -highlightthickness 1 -image \
				   txtfImg -command [list newDiffBrowse	  "File"	$fSpec.e2]
			button $Brws2.bd -borderwidth 1 -highlightthickness 1 -image \
				   fldrImg -command [list newDiffBrowse "Directory" $fSpec.e2]
			pack $Brws2.bf -padx {7 0} -pady {0 2} -side left
			pack $Brws2.bd -padx {0 7} -pady {0 2} -side right
			set_tooltips $Brws2.bf {"to a file"}
			set_tooltips $Brws2.bd {"to a directory"}

		set Brws3 [labelframe $fSpec.fB3 -text "Browse..."]
			button $Brws3.bf -borderwidth 1 -highlightthickness 1 \
					 -image txtfImg \
					 -command [list newDiffBrowse "File" $fSpec.eA "Ancestor"]
			pack $Brws3.bf -side top
			set_tooltips $Brws3.bf {"to a file"}

		checkbutton $fSpec.b -variable finfo(fRecurs) -text Directory\nRecurse\
								-selectcolor $opts(inform) -indicator 0

		# we'll use the grid geometry manager to get things lined up right...
		grid $fSpec.l1							  -sticky e	   -row 0 -column 0
		grid $fSpec.e1	-columnspan 4 -pady 4	  -sticky nsew -row 0 -column 1
		grid $fSpec.scm1lbl						  -sticky e	   -row 1 -column 0
		grid $fSpec.scm1						  -sticky e	   -row 1 -column 1
		grid $fSpec.lr1		   -padx {5 0}					   -row 1 -column 2
		grid $fSpec.er1										   -row 1 -column 3
		grid $Brws1 -rowspan 2 -padx 4 -pady 4	  -sticky nsew -row 0 -column 5

		grid $fSpec.l2							  -sticky e	   -row 2 -column 0
		grid $fSpec.e2	-columnspan 4 -pady {8 4} -sticky nsew -row 2 -column 1
		grid $fSpec.scm2lbl						  -sticky e	   -row 3 -column 0
		grid $fSpec.scm2						  -sticky e	   -row 3 -column 1
		grid $fSpec.lr2		   -padx {5 0}					   -row 3 -column 2
		grid $fSpec.er2										   -row 3 -column 3
		grid $Brws2 -rowspan 2 -padx 4 -pady 4	  -sticky nsew -row 2 -column 5

		# N.B> Padding Ancestor label reserves spacing for scm(N)lbl checkboxes
		grid $fSpec.lA	-padx {12 0}			  -sticky e	   -row 4 -column 0
		grid $fSpec.eA	-columnspan 4 -pady {8 4} -sticky nsew -row 4 -column 1
		grid $fSpec.lrA										   -row 5 -column 2
		grid $fSpec.erA										   -row 5 -column 3
		grid $Brws3 -rowspan 2 -padx 4 -pady 4	  -sticky nsew -row 4 -column 5

		grid $fSpec.f4	 -columnspan 4 -pady 4	  -sticky w	   -row 6 -column 0
		grid $fSpec.b		   -padx 4 -pady 4	  -sticky nsew -row 6 -column 5
		grid remove $fSpec.b ;# Start with Recurse button NOT displayed

		grid columnconfigure $fSpec	 1 -weight 1
		grid columnconfigure $fSpec	 4 -weight 4

		# Container for additional datums we rarely need
#### MCP
#		set options [frame $w(newDiff).options -bd 2 -relief groove]
		set options [frame $w(newDiff).options]
#### MCP
		button $options.more -text "More" -command "newDiffHdn $options opn"

		checkbutton $options.cflct -text "input is Conflict format" \
													-var g(conflictset)
		label $options.ml -text "Merge Output"
		entry $options.me -textvariable g(mergefile)
		label $options.l1l -text "Label for File 1"
		entry $options.l1e -textvariable finfo(ulbl,1)
		label $options.l2l -text "Label for File 2"
		entry $options.l2e -textvariable finfo(ulbl,2)

		grid $options.more -column 0 -row 0 -sticky nw
		grid columnconfigure $options -0 -weight 0

		# Container for local Help (also hidden until requested)
		#	(activation will be from a PRIMARY Dialog button)
#### MCP
#		set hlp [frame $w(newDiff).hlp -bd 2 -relief groove]
		set hlp [frame $w(newDiff).hlp]
#### MCP

		#	HOWEVER - this help subwindow MAY be being built BEFORE the MAIN
		#	client windows FROM WHICH we plan to steal its Fg/Bg colors...
		# Perhaps there's a more elegant way to DO this, but create a DUMMY
		# window we can sieve the client PREFs thru, so we can steal from IT!!
		if {![winfo exists $w(client)]} {
			set w(acTxWdg) [text $hlp.dummy {*}$opts(textopt)]
		}

		text $hlp.txt -setgrid 1 -wrap word -highlightthickness 0 -height 10 \
					-bg [$w(acTxWdg) cget -bg] -fg [$w(acTxWdg) cget -fg]	 \
			-yscroll [list [set hlp.vsb [scrollbar $hlp.vsb -orient vertical \
					-command "$hlp.txt yview" -bd 1]] set] -bd 0

		# (just remember to cleanup the dummy window afterward)
		# N.B>	w(acTxWdg) *WILL BE* the subject of a trace... just not YET !!
		if {![winfo exists $w(client)]} { destroy $w(acTxWdg) }

		# Form-up the help sub-frame with its textWin + scrollbar
		# then FILL the window with the pertinent Help info (& disable it)
		pack $hlp.vsb -side right -fill	 y	 -expand 0 -padx {0 2} -pady {0 2}
		pack $hlp.txt -side left  -fill both -expand 1 -padx {2 0} -pady {0 2}
		grid columnconfigure $hlp -0 -weight 0
		put-text $hlp.txt [help-GUI newDiff]
		$hlp.txt tag configure margn -lmargin1 5 -lmargin2 5 -rmargin 5
		$hlp.txt tag add margn 1.0 end
		$hlp.txt configure -state disabled

		# Now the primary buttons for this dialog...
		set btns [frame $w(newDiff).buttons]

		button $btns.help -width 5 -text "Help" -command "newDiffHdn $hlp opH"
		button $btns.cancel -text "Cancel" -width 5 -default normal -command {
			if {! [winfo exists .client]} {do-exit}
			array set finfo $g(NDpriorVals) ;# restore start values
			newDiffHdn $w(newDiff).hlp clH
			set w(NewDok) 0
		}
#### MCP
#		button $btns.ok -text "Ok" -width 5 -default active -command \{
		button $btns.ok -text "OK" -width 5 -default active -command {
#### MCP
			# SYNTAX BARRIER (blocks tilde+Glob for Tcl8->9 Chgd semantics)
			lassign "$finfo(FSpec1) $finfo(FSpec2) $finfo(FSpecA)" \
					   finfo(f,1)	  finfo(f,2)	  finfo(f,0)
			Dbg "OK FSpec1($finfo(f,1)) FSpec2($finfo(f,2)) FSpecA($finfo(f,0))"
			# CANT claim its a conflictfile if ISNT a file at all
			if {![file isfile $finfo(f,1)]} {set g(conflictset) 0}

			# Because of the call-semantics for 'assemble-args', ALL internal
			# Fspecs (f,0 f,1 & f,2) MUST NOT leave here w/VERSIONED URLs ...
			#	(vrsn DATA was ALREADY diverted properly(?) elsewhere)
			#
			# BUG(?) SVN '@syntax+MEANING': is NOT what TkDiff BELIEVES it is
			# See 15Jun25 Dev-notes or read SVN DOCs about "PEG Revision"
			#	(yet MAY NOT be truly PERTINENT as TkDiff only wants 2 files!!)
			foreach x "f,0 f,1 f,2" {
				#	(A URL, w/@-syntax (date-OR-other), w/NO trailing slashs)
				if {[string match {*://*}		$finfo($x)]			 \
				&& ( [set at [string last "@\{" $finfo($x)]]	> 0	 \
				 || ([set at [string last  "@"	$finfo($x)]]	> 0	 \
					 &&		 [string first "/"	$finfo($x) $at] < 0))} {
					# REMOVE the @-syntax from this Fspec
					set finfo($x) [string range $finfo($x) 0 $at-1]
				}
			}
			newDiffHdn $w(newDiff).hlp clH
			set w(NewDok) 1
		}
		pack $btns.ok $btns.help $btns.cancel \
										-side left -fill none -expand y -pady 4

		# pack this crud in...(btns FIRST so resize wont clip them)
		pack $btns -side bottom -fill x -expand n
		pack $fSpec -side top -fill both -ipady 2 -ipadx 20 -padx 5 -pady 5

		pack $options -side top -fill both -ipady 5 -ipadx 5 -padx 5 -pady 5

		bind $w(newDiff) <Return> [list $btns.ok invoke]
		bind $w(newDiff) <Escape> [list $btns.cancel invoke]
	}

	# initialize dialog
	set g(scmDOsrch) 0					;# Begin from a non-search SCM state
	set g(scmPrefer) "$opts(scmPrefer)" ;# and w/default SCM preferences
	lassign {} FSpec1 FSpec2 FSpecA		;# ReStart as EMPTY (SAFE- ?Unneeded?)
	$w(newDiff).fSpec.e1 validate
	$w(newDiff).fSpec.e2 validate
	$w(newDiff).fSpec.eA validate
	set detectMrgFilChg $g(mergefile)

	######
	Dialog show $w(newDiff) w(NewDok) 0 $w(newDiff).fSpec.e1;# MODAL: wait here
	######

	Dialog dismiss $w(newDiff)
	# Only lock-in Mergefile if user CHANGED and ACCEPTED it
	if {$w(NewDok) && $g(mergefile) != "" } {
		set g(mergefileset) [expr {$g(mergefile) != $detectMrgFilChg}]
	}
	return $w(NewDok)
}

###############################################################################
# Specialized handler for dynamic SCM state/choice transitions within NewDiff
#	N.B> (knows ALOT about the NAMING/RELATIONS of the widgets it manipulates)
# Args: subcmd	- requested check/adjustment to be processed
#		wdg		- widget making the request
#		val		- proposed new value for widget (or value being monitored)
#		limit	- optional value needed (eg. prev length, special flag, etc.)
###############################################################################
proc scm-updat {subcmd wdg val {limit 0}} {
	global g w opts finfo

	# Init and/or invent some useful meta-pgming naming conversions
	set D  -1 ;# Local master-Dbg authorizer (in case parse issues arise later)
	set Other([set Other(2) 1]) 2	;#	(a simple meta-pgm identity value)
	if {[string is digit [set ndx [string index $wdg end]]]} {
		# grab the instance number of widget (if any); then use it
		# to create meta-addressable names for PAIRED widgets
		set W($ndx) $wdg
		set W($Other($ndx)) [string replace $wdg end end $Other($ndx)]
	}

	# Overall this implements a 'ripple effect' starting from an 'entry'
	# validation to load a 'combobox' list of detected SCMs (whose value MAY
	# then be "searchable") to a checkbutton that ASKs for such searching.
	# Also each widget along the way can initiate its OWN ripple independently
	#	N.B> ANCESTOR uses MUCH of this (skipping combobox-related parts)
	switch -glob $subcmd {
		"ancstr"   -
		"scm\[12]" {
			# N.B> 'ndx' has values of 1,2 or A -- just	 be careful
			if {"$ndx" != "A"} {
				set scmbox [file rootname $wdg].$subcmd ;# matching SCM widget
			} ; set	 lWdg  [file rootname $wdg].l$ndx	;# matching LBL widget

			# Always need to alert user about USING the 2nd entry widget
			# when the 1st is still empty -> highlight 2nd widget bkgnd.
			#	(N.B> a complete NO-OP when $ndx is NOT 1 or 2)
			if {("$ndx"=="2" && "$val" != "" && ![$W(1) index end]) \
			||	("$ndx"=="1" && "$val" == "" &&	 [$W(2) index end])} {
					 $W(2) configure -bg Tomato
			} elseif {$ndx in {1 2}} { $W(2) configure -bg [$W(1) cget -bg] }

			# (An exotic bit of actual state-machine name PARSING happens here)
			#	1. Leading tilde IMPLYS a file (over possibility of a URL)
			#	2. URLs become recognized once we can see ':/' in WIDGET-val
			#	3. Glob notation will be useful in filenames ONLY (not URLs)
			#
			# Thus, following only operates WHEN its BELIEVED to be a FILENAME
			#	(N.B> when presence of ':/' has not yet been seen)
			#
			# BASICALLY, a FAIL'd Tilde notation IMPLYs a NONexistant fname BUT
			# when FOLLOWED by SLASH, NON-existance is then PERPETUAL! A FAIL'd
			# tilde warrants a msg and a Red hilite, w/EACH a reason to REJECT
			# the (perpetuating) SLASH; other chars should provisionally accept
			# w/yellow hilite, but NO hilite at ALL when $val PHYSICALLY exists!

			# (Clumsy V9 compat?) coding to GET a NL-prefix Dbg (commented-out)
			#	(V9 claims mismanaged quotes where V8 was just fine)
			if {[string index $val 0] != ""} { Dbg "" $D "" }
			Dbg {VAL($val) LIMIT($limit)} $D "" ;# NO-prefix instead of below!!
			#		[expr {[string index $val 0] != {} ? "\n" : ""}]

			if {[string index $val 0] == "~" || ![string match {*:/*} $val]} {
				set accept true ;# PRESUMEs added char in 'val' is acceptable
				set BkSpc [expr {[string length $val] < $limit}];# BkSpc'd ?

				# (N.B> tilde FAULTS if $env(HOME) UNSET or userID INVALID ...
				#  FURTHER: 'val' MAY no longer reflect EXACT widget content!)
				# THUS - save a copy (orig) BEFORE it can be overwritten!
				if {[set Err [tildChk [set orig $val] fexist val]]} {
					# MUST report a LAST slash - it 'perp'etuates the failure
					#	BUT - if NO SLASH exists, perp OBVIOUSLY tilde itself!
					if {[set i [string first "/" $val 1]] > 0 || ![set i 0]} {
						Dbg {updat($subcmd) BkSpc($BkSpc) Err($Err)\
							TypPop$ndx@($w(TypPop$ndx@)) val($val)->BAD\
							FSpec${ndx}($val)}						$D "Pop:\t"
						if {$w(TypPop$ndx@) < $i} { popmsg \
							"$fexist" ok [file rootname $wdg] "Erase/Re-enter ?"
						}
						set w(TypPop$ndx@) $i
					}
					set fexist 0; # failed conversion IMPLYS non-existance
				} { set w(TypPop$ndx@) -1 }

				# MAY have thrown err, but SOME PATH-fragment does not exist!
				#	(could be the FIRST one if tildChk FAILED to convert)
				# HOWEVER - we NEED a position in ORIGINALLY specified context
				# thus we must walk THOSE fragments: NOT what tildChk returned!
				if {$fexist != 1} {
					# warn user (lbl-hilite), $val is NOT (yet?) a real entity,
					# Acceptance based on if/where a perpetuating SLASH IS...
					if {!$Err && !$fexist} {
						set LSlash [set i -1] ;# index of NEXT SLASH in PATH
						while {[set i [string first "/" $orig [incr i]]] >= 0} {
							set OV [string range $orig 0 $i]
							Dbg {i($i) LSlash($LSlash):([string range $orig \
											0 $LSlash]) orig($orig) OV($OV)} $D
							# (N.B> glob WILL NOT crash)
							if {[llength [glob -n $OV]]} {
								set LSlash $i
							} { break }
						}

						#	 Color Establishes SEVERITY of non-existence
						# Am simply in limbo - (yellow?) - hint we want more
						$lWdg configure -bg $opts(inform)
					} elseif {!$fexist} { set LSlash $limit ; # A tiny LIE:
					#				(no SLASH exist?): Is IMPLIED by E-O-Line
						$lWdg configure -bg Tomato
						# EMPHASIZE (red) tilde-Err -or- (yellow) too many exist
					} { $lWdg configure -bg $opts(inform) }

					# BUT will REJECT(0) only when a Slash was TYPED
					#	(not backspaced TO) BEYOND where non-exist became known
					# N.B> value 'i' DEPENDS on $Err: references NOT equivalent!
					if {!$fexist && !$BkSpc \
					&& "/" == [string index $orig [set i [$wdg index insert]]] \
					&& (($Err && $i >= $w(TypPop$ndx@)) || $i > $LSlash)} {
						set accept false }

					if {!$fexist} { Dbg {updat($subcmd) BkSpc($BkSpc)\
						Err($Err) fexist($fexist) LSlash($LSlash)\
						TypPop$ndx@($w(TypPop$ndx@)) val($val)->BAD\
						Accept($accept)}							$D "Res:\t"
					}

					# Yet STILL must tell SCM to UNSET (Re: fname is UNKNOWN)
					if {$subcmd != "ancstr" && [llength $finfo($subcmd)]} {
						 after idle $scmbox configure -value \
												   "{[set finfo($subcmd) { }]}"
					}
					# N.B> User CAN ALWAYS backspace/re-enter to correct
					return $accept ; # Rejects when Slash perpetuates non-exist
				}
			}

			# Have a URL or REAL file entit(ies?) (ie. ?sequence-of? fragments)
			#	(ensure typing-monitor hilit turns OFF)
			$lWdg configure -bg $w(bgnd)
			Dbg {updat($subcmd) BkSpc($BkSpc) Err($Err) fexist($fexist)\
						TypPop$ndx@($w(TypPop$ndx@)) val($val)->LEGIT} $D "\t"

			# HOWEVER:
			#	'val' is either a URL or FILE entities (w/POSSIBLE Glob syntax)
			# OR empty. Provided such syntax WILL result in a SINGLE entity,
			# REPLACE any GLOB syntax by REWRITING 'val' with its expansion
			if {$val != "" && ![string match {*://*} $val] \
			&&	$fexist == 1}	  { set val [glob -n $val] }

			# EntryBox contents EXISTS (or is empty) - recheck SCM choice status?
			#	(N.B> Simply NOT when/where/how Ancestor finds its SCM)
			if {$subcmd != "ancstr"} {
				set finfo($subcmd) [scm-detect $val None]
				# SCMBOX command fires (by setting its value), further rippling
				# to set 'srch' state, only AFTER this validation callback ends
				set vote [lindex $opts(scmPrefer) $ndx-1] ; after idle \
				$scmbox configure -value "{[scm-elect $finfo($subcmd) $vote]}"
			}

			# Finally, if that CONTENT happens to be a Rev-carrying URL we
			# need to lockout the USER from ITS Revision-widget. Sadly, TCL
			# wont *do* a "switch fallthru" case, so we will RECURSE to it!
			return [scm-updat lckR $wdg $val]	;# (Always returns TRUE)
		}
		"lckR" {
			set rWdg [file rootname $wdg].er$ndx ;# (gonna need Rev wdg)

			# Update PROPOSED FSpec value (to take effect when we LEAVE dialog)
			#	(N.B> 'val' SHOULD no longer contain ANY Glob syntax !!)
			set finfo(FSpec$ndx) $val
			Dbg {updat($subcmd) BkSpc(_) Err(_) TypPop$ndx@($w(TypPop$ndx@))\
							val($val)->LEGIT FSpec${ndx}($val)}	   $D "Lock:\t"

			# If the present Fspec value appears to be a URL of the
			#	(admittedly, for NOW) Subversion syntactic variety...
			if {[string match {*://*} $val]} {
				# ...and CHOSE to specify a REVISION suffix ...
				if {[set at [string last "@\{" $path]] > 0	  \
				|| ([set at [string last "@"  $path]] > 0 && \
					[string first "/" $path $at] < 0} {
					# ... PUSH that revision value into ITS widget and
					# PREVENT any user manipulation via that widget (LOCK it)
					#	(N.B> cant DO in SAME config call - disable wins!)
					set [$rWdg cget -textvar] [string range $path $at+1 end]
					$rWdg configure -state disabled
				} { $rWdg configure -state normal }
			# else UNLOCK (and empty) it if WAS locked by above previously
			} elseif {[$rWdg cget -state]=="disabled"} {
				set [$rWdg cget -textvariable] "" ;# zap old URL-provided Rev
				$rWdg configure -state normal
			}
			return true ;# validation ALWAYS true: is (mostly) monitoring
		}
		"srch" {
			# (generally REACHED by cmd associated w/modifying SCMBOX widget)
			lset g(scmPrefer) $ndx-1 "$val" ;# Keep working global up-to-date
			# Cant search when ANY Fspec exists or a non-detected SCM
			if {[string trim $finfo(f,1)] != "" \
			||	[string trim $finfo(f,2)] != ""} {
				set wdg [string range $wdg 0 end-1]
				foreach lbl "1lbl 2lbl" {
					$wdg$lbl configure -text "	 SCM :" -indicatoron 0
					$wdg$lbl deselect
				}

				# Only allow following ONCE, regardless of ACTUAL side touched
				# (N.B> this is NOT the 'limit' value seen in OTHER subcmds)
				if {$limit} { return }

				# If both Fspecs happen to be Directories (OR when the FIRST is
				# a Dir and the other is EMPTY), allow user to choose between a
				# recursive or single-level candidate srch evaluation - UNLESS
				# the Engine opts to support Dir/Dir dont actually exist
				set recursW [file rootname $wdg].b
				if {[file isdir $finfo(f,1)] &&
				   ([file isdir $finfo(f,2)] || ( "" == $finfo(f,2)
									&& [lindex $g(scmPrefer) 0] in $g(scmS)))} {
					if {![winfo ismapped $recursW]} { grid		  $recursW }
				} { if { [winfo ismapped $recursW]} { grid remove $recursW } }
				alignState $recursW [expr {[string trim $opts(egnSrchCmd)]!={}}]

			# Otherwise srch is determined PER CHOSEN SCM for each Fspec
			} else {
				if {$val in $g(scmSrch)} {
					${wdg}lbl configure -text  "Search?" -indicatoron 1
				} else {
					# Chosen SCM cant handle searching - remove option
					${wdg}lbl configure -text "	  SCM :" -indicatoron 0
					${wdg}lbl deselect
				}
				# MAY need to reactivate OTHER side (if Fspec JUST went empty)
				#	(N.B> appending 'limit' flag (1) stops firestorm recursion)
				if {!$limit} { scm-updat srch $W($Other($ndx)) \
											  [$W($Other($ndx)) cget -value] 1
				}
			}
		}
		"set" {
			# If allowed, USER chooses whether to SEARCH the SCM for candidates
			if {[$wdg cget -indicatoron]} {
				# 'val' here is which SIDE was toggled - merge its NEW VALUE
				# adjusting the other side to establish the radioBtn-like value
				if {($val & $g(scmDOsrch))} {
					incr	 g(scmDOsrch) -$val ;# Turn choice OFF
				} else { set g(scmDOsrch)  $val ;# Turn choice ON ... but
					[file rootname $wdg].scm$Other($val)lbl deselect;
					# N.B>	Only ONE choice can be ON (but BOTH can be OFF)
				}
			# Un-toggle (ie. ignore THIS invocation) if indicator was NOT shown
			#	(means they clicked on it when it wasn't "armed" to accept)
			} else	{$wdg deselect}
		}
	}
}

###############################################################################
# Expand/contract window to access lesser-used features of the NewDiff dialog
###############################################################################
proc newDiffHdn { W opncls } {
	global g w finfo

	switch $opncls {
	"opn" {
	grid $W.cflct -row 0 -column 2 -sticky w
	grid $W.ml	  -row 1 -column 1 -sticky e
	grid $W.me	  -row 1 -column 2 -sticky nsew -pady 4 -padx {0 4}
	grid $W.l1l	  -row 2 -column 1 -sticky e
	grid $W.l1e	  -row 2 -column 2 -sticky nsew -pady 4 -padx {0 4}
	grid $W.l2l	  -row 3 -column 1 -sticky e
	grid $W.l2e	  -row 3 -column 2 -sticky nsew -pady 4 -padx {0 4}
	grid columnconfigure $W 2 -weight 1

		$W.more configure -text "Less" -command "newDiffHdn $W cls"
	update
	}
	"cls" {
	grid remove $W.cflct $W.ml $W.me $W.l1l $W.l1e $W.l2l $W.l2e

	# Zap everything as we close (yes, the last 3 get "")
	lassign {0 0} g(conflictset) g(mergefileset) g(mergefile) \
									finfo(ulbl,1) finfo(ulbl,2)

		$W.more configure -text "More" -command "newDiffHdn $W opn"
	}
	"opH" {
		pack $W -side bottom -fill both -padx 5 -expand 1
		set btn [string map {hlp buttons} $W]
		$btn.help configure -text "Less\nHelp" -command "newDiffHdn $W clH"
	}
	"clH" {
		pack forget $W
		set btn [string map {hlp buttons} $W]
		$btn.help configure -text "Help" -command "newDiffHdn $W opH"
	}}
}

###############################################################################
# File/Directory browser for the "New Diff" dialog
###############################################################################
proc newDiffBrowse {type widget {title {}}} {
	global w opts

	# Uses TARGET widget name to locate OTHER widget field (expects a 1 or 2)
	if {[string is digit [set n [string index $widget end]]]} {
	set widgroot [string range $widget 0 end-1]
	set other([set other(2) 1]) 2
	} else { set n {} }

	# Start from what is IN the target already
	#	Basically we want each item to START browsing from where
	#	the most recent request left off; that means (in order):
	#	   - the directory of where it is already
	#	   - the directory of where the OTHER entry is (widgets 1 & 2 only)
	#	   - the current working directory
	#	Note that the PRIOR use of EITHER item CAN itself BE a directory
	if {[set entrystuff [$widget get]] != ""} {
		if {![file isdirectory [set initdir $entrystuff]]} {
			set initfil [file tail $initdir]
			set initdir [file dirname $initdir]
		} else {set initfil {}}

	} elseif {$n!={} && [set entrystuff [${widgroot}$other($n) get]] != ""} {
		if {![file isdirectory [set initdir $entrystuff]]} {
			set initfil [file tail $initdir]
			set initdir [file dirname $initdir]
		} else {set initfil {}}

	} else { set initdir [pwd]; set initfil {} }
	Dbg {NewDbrowse: initdir($initdir) initfil($initfil)}

	# What KIND of entry are we browsing to find ?
	switch -glob $type {
	"D*" { set chosen [tk_chooseDirectory -title "$type ${n}${title}" \
					-parent $w(newDiff) -initialdir	 $initdir]
			# ?BUG? Undocumented Behavior (at the very least) -
			#	When NO EFFECTIVE manipulation occurs: the dialog 'OK'
			#	button returns "initdir" ... but 'Cancel' returns ""
			# In keeping with TRYING to 'shorten Fnames', we will use
			# the CWD when 'initdir' (or the user) happens to steer there
			if {$chosen==[pwd]} { set chosen "." }
		}
	"F*" { set chosen [tk_getOpenFile -title "$type ${n}$title" \
					-parent $w(newDiff) -initialdir	 $initdir	\
					-initialfile $initfil -filetypes $opts(filetypes)]
		}
	}

	# Send back what we got (inserted only when it was successful)
	if {[string length $chosen] > 0} {
		$widget delete 0 end
		$widget insert 0 [shortNm $chosen]
		$widget selection range 0 end
		$widget xview end
		focus $widget
	} else { after idle {raise $w(newDiff)} }
	return $chosen
}

###############################################################################
# Split or Combine dialog (modal): adjust CDR bounds & forms EQUIVALENT diff(s)
###############################################################################
proc splcmbDlg {Combine} {
	global g w opts splcmb

	# (If first time invoked) ... Construct the Dialog window itself)
	if {![Dialog MODAL $w(scDialog)]} {
		wm	 title	 $w(scDialog) "Adjust Diff Bounds"
		wm transient $w(scDialog) .
		wm	 group	 $w(scDialog) .
		wm resizable $w(scDialog) 0 0
		wm protocol	 $w(scDialog) WM_DELETE_WINDOW {$w(scDialog).cncl invoke}

		# Encode the addressable slots/labels for loading into a 5x3 grid:
		set row(u)	[set col(l) 0]	   ;# Upper row	 (or)  Left col-pair(0&1)
		set row(l)				2	   ;# Lower row
		set col(r)				3	   ;# Right col-pair(3&4)
		set lbl(l)	"Left Side"		   ;# (both SIDE labels go in row 1)
		set lbl(r)	"Right Side"	   ;#
		set lbl(lu) "Upper Edge"	   ;# (both EDGE labels go in col 2)
		set lbl(ll) "Lower Edge"	   ;#
		#	(Button columns are designed as VERTICALLY-OPPOSED pairings)
		lassign { 0 0  1 1	3 3	 4 4}  col(luu) col(lld)   col(lud) col(llu) \
									   col(rud) col(rlu)   col(ruu) col(rld)
		# Now start building the dialog
		label $w(scDialog).msg ;# Message content will be 'cfg'ed later
		pack $w(scDialog).msg -side top -padx 4 -pady 4

		# Populate the 5x3 grid (logically 3x3, but outer cols span 2 each)
		frame [set BtnFr $w(scDialog).btn] -relief groove -padx 4 -pady 4
		foreach LR {l r} {		;# Left Right	 SIDE > collectively forms
		  foreach UL {u l} {	;# Upper Lower	 EDGE > widget names & args
			foreach DU {d u} {	;# Down Up		 BUTN >	 to "splcmb-adj"
			  set nm "."
			  button ${BtnFr}[append nm $LR $UL $DU] -image arroW$DU	\
								  -repeatdelay 750	-repeatinterval 400 \
								  -command	[list splcmb-adj $LR $UL $DU]
			  grid ${BtnFr}$nm -row $row($UL) -column $col(${LR}${UL}$DU)
			}
			if {[info exists lbl(${LR}$UL)]} {					;# Edge label
			  label ${BtnFr}[set nm .lB${LR}$UL] -text "$lbl(${LR}$UL)"
			  grid	${BtnFr}$nm -row $row($UL) -column 2
			}
		  }
		  label ${BtnFr}[set nm .lB$LR] -text "$lbl($LR)"		;# Side label
		  grid ${BtnFr}$nm -row 1 -column $col($LR) -columnspan 2
		}
		pack $BtnFr -side top -padx 4 -pady 4

		# Set up to signal 'tkwait ::scDialogRet' when user has completed task
		button $w(scDialog).done -command {set w(scDialogRet) 1} ;# -text later
		button $w(scDialog).cncl -command {set w(scDialogRet) 0} -text "Cancel"
		pack $w(scDialog).done $w(scDialog).cncl -pady 4 -side left -expand yes

		# Ensure dialog can be RAISED during its modal-grab if becomes hidden
		#	(Should put this definition elsewhere; someplace more general)
		bind modalDialog <ButtonPress> {wm deiconify %W ; raise %W}
		bindtags $w(scDialog) [linsert [bindtags $w(scDialog)] 0 modalDialog]
	} { set BtnFr $w(scDialog).btn }

	# # # # # # # # # # # # # # # # # # # # #
	# Re-configure Dialog contents for PRESENT usage
	#	Some settings WILL depend on whether mode is "Split" .vs. "Combine"
	if {$Combine} {
		$w(scDialog).done configure -text "Combine"
		$w(scDialog).msg configure -text \
							 "Use buttons to EXPAND the current diff region"
		lassign {disable normal} inward outward ;# cmbin btns init state
	} else {
		$w(scDialog).done configure -text "Split"
		$w(scDialog).msg configure -text \
							 "Use buttons to REDUCE the current diff region"
		lassign {normal disable} inward outward ;# split btns init state
	}
	foreach {b} {luu ruu lld rld} {$BtnFr.$b configure -state $outward}
	foreach {b} {lud rud llu rlu} {$BtnFr.$b configure -state $inward}

	# Identify the target CDR, its Line info (and extract its type)
	lassign $g(scrInf,[set hID [hunk-id $g(pos)]]) S E Pl Ol na Pr Or
	regexp {[0-9,]*([acd])[0-9,]*} $hID na CDRtyp

	# # # # # # # # # # # # # # # # # # # # #
	# Next, establish the 'working set' of data (global splcmb array entries)
	# Start fresh by flushing any old data and recording the CDR info and ID
	unset -nocomplain splcmb
	set splcmb(rnge) [list [list $S $E $Pl $Ol $Pr $Or $hID]]

	# Also initialize the 'Pad'-lines "jump" table for EACH side
	#	A jump table records pairs of line numbers that correspond to the top
	#	and bottom of a contiguous run of "Pad" lines IN a splcmb(rnge) entry.
	#	Used later in "splcmb-adj" to *jump* past those lines when editting.
	set splcmb(jl) [set splcmb(jr) {}]
	if {$Pl} {set splcmb(jl) [list [expr {$E-$Pl+1}] $E]}
	if {$Pr} {set splcmb(jr) [list [expr {$E-$Pr+1}] $E]}

	# Hmm, 'Combine' requires a lttle more work -
	if {$Combine} {
		# Must RE-derive the ORIGINAL bounds of which this CDR is a SUBSET
		#	(create some temps to work with)
		set minpos [set maxpos $g(pos)]
		set nS $S
		set nE $E

		# Now try to EXTEND those values OUTWARD as far as they can go
		while {$minpos > 1} {
			set nhID [hunk-id [incr minpos -1]]

			if {$nS == [lindex $g(scrInf,$nhID) 1] + 1} {
				# Subsume this hunk (it abuts the CDR leading edge)
				lassign $g(scrInf,$nhID) nS tE tPl tOl na tPr tOr
				set splcmb(rnge) [linsert $splcmb(rnge) 0 \
									 [list $nS $tE $tPl $tOl $tPr $tOr $nhID]]
				if {$tPl} {set splcmb(jl) [linsert $splcmb(jl) 0 \
												  [expr {$tE-$tPl+1}] $tE]}
				if {$tPr} {set splcmb(jr) [linsert $splcmb(jr) 0 \
												  [expr {$tE-$tPr+1}] $tE]}
			} else { break }
		}

		while {$maxpos < $g(count)} {
			set nhID [hunk-id [incr maxpos]]

			if {$nE == [lindex $g(scrInf,$nhID) 0] - 1} {
				# Subsume this hunk (it abuts the CDR trailing edge)
				lassign $g(scrInf,$nhID) tS nE tPl tOl na tPr tOr
				lappend splcmb(rnge)  [list $tS $nE $tPl $tOl $tPr $tOr $nhID]
				if {$tPl} {lappend splcmb(jl) [expr {$nE-$tPl+1}] $nE}
				if {$tPr} {lappend splcmb(jr) [expr {$nE-$tPr+1}] $nE}
			} else { break }
		}
		# splcmb(rnge) now has an ORDERED list of possibly involved hunks;
		# and ORDERED splcmb(jr/jl) lists - ie. ALL its "jump table" info.
		# ALSO 'nS' and 'nE' now have the OUTERMOST encompassing EDGE values
	} else { set nS $S ; set nE $E }

	# Further adjust the "Combine"-mode buttons if CDR is AT either edge of
	#	   the rnge (ie. already sitting at an exterior limit) ...
	# -OR- further adjust the "Split"-mode buttons if its an "a/d"-type CDR to
	#	   disallow adjustment to the ALL "Pad" lines side	(its pointless).
	set btns {} ;# Note: default is that NEITHER adjustment will be required
	if {$Combine} {
		if {$S == $nS} {set btns {luu ruu} }
		if {$E == $nE} {set btns {lld rld} }
	} elseif {"$CDRtyp" == "a" || "$CDRtyp" == "d"} {
		if {$Pl} {set btns {lud llu} }
		if {$Pr} {set btns {rud rlu} }
	}
	if {[llength $btns]} {foreach b $btns {$BtnFr.$b configure -state disable}}

	# Construct the (user modifiable) 'working set' of the PRESENT CDR edges
	#	(semantic indices refer to	 SIDE and EDGE	 pairings)
	# Note that the RELATIONSHIP of these MOVABLE edges to the 'hard limit'
	# EDGES (defined next) WILL DEPEND on the "Split" .vs. "Combine" mode
	lassign "$S $E $S $E" splcmb(lu) splcmb(ll) splcmb(ru) splcmb(rl)
	incr splcmb(ll)	 ;# Txt-wdg require 'lower' edge specs be 1 lower
	incr splcmb(rl)

	# Next, a (static) set of 'hard limits' semantically BRACKETING the edges
	#	This semantic is an ['i'nner/'o'uter -plus- 'u'pper/'l'ower] concept
	#	describing where any given BTN (and its implied EDGE) is HEADING to.
	#		NOTE: this will LATER REQUIRE a mode-specific reverse-mapping
	#	conversion (in "splcmb-adj") that can mirror the distinct edge VALUE
	#	REARRANGEMENT being done here (would've been easy if Tcl had pointers!)
	lassign "$nS $S $E $nE" splcmb(ou) splcmb(iu) splcmb(il) splcmb(ol)
	incr splcmb(ol)	 ;# As before, lower bnds must be BELOW range (for Txt-wdg)
	incr splcmb(il)

	# Last config step - setup for the Txt-wdg tagging, ensuring visibility...
	foreach wdg "$w(LeftText) $w(RightText)" {
		$wdg  tag configure scCDR -background $opts(adjcdr)
		$wdg  tag configure scADD -background $opts(mapins)
		$wdg  tag configure scCHG -background $opts(mapchg)
		$wdg  tag configure scDEL -background $opts(mapdel)
		$wdg  tag configure scPSH			  -bgstipple gray50
		$wdg  SEE $S.0 ;# N.B> grab will BLOCK scrolling: becomes OUR problem
	}
	# ... and now 'paint' the CURRENT (starting) state for the user.
	#	(Note: this ALSO *creates* datums describing the Split/Combine STATE)
	splcmb-Feedback $Combine

	# # # # # # # # # # # # # # # # # # # # #
	# FINALLY ... Display and Invoke the actual Dialog
	Dialog show $w(scDialog) w(scDialogRet) 0
	#
	# # # # # # # # # # # # # # # # # # # # #
	# waits here for the user to do their thing ... (tick, tick, tick)
	# # # # # # # # # # # # # # # # # # # # #
	#
	# Continue processing, beginning with taking down the Dialog itself
	Dialog dismiss $w(scDialog)

	# ELIMINATE all Dialog-overlaid-tagging in the Text widgets
	foreach wdg "$w(LeftText) $w(RightText)" {
		$wdg tag delete scADD scDEL scCHG scCDR scPSH
	}

	#splcmb-chk data ;# Formatted DEBUG output

	# And BAIL-OUT if user Cancelled  -OR-	made no ACTUAL changes
	#	(each movable edge is AT its original STARTING position)
	if {!$w(scDialogRet) || \
		( ($splcmb(lu)==$splcmb(ru) && $splcmb(lu)==$S)	  && \
		  ($splcmb(ll)==$splcmb(rl) && $splcmb(ll)==$E+1) )} {
		# HOWEVER - partial operation MAY have moved the scrolling
		#	RESTORE alignment if that mode is active
		if {$opts(autocenter)+$opts(syncscroll)} { centerCDR }
		return
	}

	# # # # # # # # # # # # # # # # # # # # #
	# Interpret/process the users interaction
	#

	# Factor-out/realign the minor inconsistencies between Split and Combine
	if {$Combine} {
		# Among the hIDs within 'splcmb(rnge)', ignore ALL that the user has
		# chosen to NOT coalesce any portion of BACK within the CDR boundary
		#	(Remember: to discount the implicit +1 of lower EDGE values)
		foreach {tS tE na tOl na tOr thID} [join $splcmb(rnge)] {
			if {($splcmb(lu)   > $tE && $splcmb(ru)	  > $tE) \
			||	($splcmb(ll)-1 < $tS && $splcmb(rl)-1 < $tS)} {continue}

			lappend rnge $thID

			# Realign Numbering to FIRST involved hunk (to init LN(l/r) below)
			if {[llength $rnge]==1}	 { lassign "$tS $tOl $tOr" S Ol Or }

			# Rewrite (promote) the CDR type UNLESS they will ALL agree
			if {"$CDRtyp" != "c" && "$thID" != \
						   [regexp -inline "\[0-9,]+$CDRtyp\[0-9,]+" $thID]} {
				set CDRtyp "c"
			}
		}
	} else { set rnge [list $hID] } ;# However, Split only involves the CDR

	# Neither mode should EVER evaluate the 'Pad'-side of a "NON-chg" CDR
	if {"$CDRtyp" == "a"} {set splcmb(l2) 0}
	if {"$CDRtyp" == "d"} {set splcmb(r2) 0}

	# (L)ine (N)umbering begins with values just PRIOR to first INVOLVED hunk
	set LN(l) [expr {$S -$Ol -1}]
	set LN(r) [expr {$S -$Or -1}]

	# At the moment, 'rnge' is a list of the INVOLVED hIDs (to be deleted).
	# Grab its count, to use later in ensuring g(pos) REMAINS a legal value
	# when the hunks being deleted HAPPEN to be at the high end of g(diff).
	set minpos [llength $rnge]

	# Walk each region - forming any NEW "hID"s (into 'rnge') as we go
	foreach rgn {1 2 3} { set NEWid {}
		# Skip entire region if BOTH sides empty ...
		if {!$splcmb(l$rgn) && !$splcmb(r$rgn)} { continue }

		# ... otherwise process BOTH halves to construct the SINGLE new hID
		# using a technique that roughly parallels what "mark-diffs" would do
		#	Step through the (D)datum item (bounds and type) for each side
		foreach LR {l r} {
			if {$splcmb(${LR}$rgn)} {
				foreach "bgn($LR) end($LR) typ" $splcmb(${LR}${rgn}D) {
					# factor out encompassed jump entries (if any)
					set i 0
					foreach {n1 n2} $splcmb(j$LR) {
						if {$bgn($LR) <= $n1 && $n2 <= $end($LR)} {
							set i [expr {$i + $n2 - $n1 + 1}]
						}
					}
					# THEN compute number of LOGICAL lines, and MAP the type
					set sz($LR) [expr {$end($LR) - $bgn($LR) - $i}]
					set t [string map "CDR $CDRtyp	ADD a  DEL d  CHG c" $typ]
					switch $t {
						"a" {	 append NEWid $LN(l) a [incr LN(r)]
							 if {$sz($LR)} {
								 append NEWid "," [incr LN(r) $sz($LR)]
							 }
						}
						"d" {	 append NEWid [incr LN(l)]
							 if {$sz($LR)} {
								 append NEWid "," [incr LN(l) $sz($LR)]
							 }
								 append NEWid d $LN(r)
						}
						"c" {if {"$LR" == "r" } {
									 append NEWid [incr LN(l)]
								 if {$bgn(l) != $end(l)} {
									 append NEWid "," [incr LN(l) $sz(l)]
								 }
									 append NEWid c [incr LN(r)]
								 if {$bgn(r) != $end(r)} {
									 append NEWid "," [incr LN(r) $sz(r)]
								 }
							 }
						}
					}
				}
			}
		}
		lappend rnge $NEWid
	}
	# Combine will likely REMOVE more hunks than it ADDS.
	# Ensure g(pos) REMAINS within its eventual bounds; preferably unchanged
	#	(minpos was earlier set to the number of hunks being removed)
	set minpos [expr {(-2 * $minpos) + [llength $rnge] + [llength $g(diff)]}]
	set g(pos) [min $minpos $g(pos)]

	# Remove and Replace the designated HIDs (Note: does NO scrolling!), but
	#	because g(pos) WAS precomputed it WILL BE properly tagged as CDR
	# N.B> CANT EVER reduce to ZERO diffs - no need to test RetCod
	mark-diffs $rnge

	# Cleanup any alignment requirements and the general display state
	if {$opts(autocenter)+$opts(syncscroll)} { centerCDR }
	update-display
}

###############################################################################
# Split/Combine dialog button callback: perform edge movement (and update UI)
###############################################################################
proc splcmb-adj {side edge btn} {
	global w splcmb

	# Only PERMITTED actions can invoke us, so NO CHECKs are EVER reqd
	#	(buttons are enabled/disabled as needed per invocation)
	# N.B> Args not only describe the action, but also the INVOKING widget
	Dbg "\n	   Btn HIT: Side<$side>	 Edge<$edge>  Btn<$btn>"

	# Invent some static translations to provide "symbolic meta-programming".
	# Many are basically just 'inverse mappings' indexed by an EDGE or a BTN,
	# (or a +/- 'btn' move defn). "push" (a <Split-only> predicate) says WHEN
	# colocated edges MUST move together and is indexed by an EDGE plus a BTN
	lassign { 1 1 0 0	  1	 -1		r l	  l u	d u }					  \
			push(ud) push(lu) push(uu) push(ld)			 mvEg(d) mvEg(u)  \
			otherS(l) otherS(r)	  otherE(u) otherE(l)	otherB(u) otherB(d)

	# Recover the semantic MODE we are operating under (because we can't PASS
	# its value from a widget cmd), then use it to create a CONTEXT-SPECIFIC
	# mapping from Edge/Btn specs to the 'LIMit edge' each is APPROACHING
	# N.B> The Combine-mode mapping is	NECESSARILY DIFFERENT  than Split-mode
	#		Edge - Btn		  "Combine"			 "Split"
	#	   Upperedge-Up	  -> Outer-Upper	-> Outer-Upper
	#	   Upperedge-Down -> Inner-Upper	-> Outer-Lower
	#	   Loweredge-Up	  -> Inner-Lower	-> Outer-Upper
	#	   Loweredge-Down -> Outer-Lower	-> Outer-Lower
	if {[set CS [expr {[llength $splcmb(rnge)] - 1}]]} {
			  set CSmap {uu ou	  ud iu	   lu il	ld ol}
	} else {  set CSmap {uu ou	  ud ol	   lu ou	ld ol} }

	# OK - Extract/categorize the CURRENT edge location values
	# THEN actually MOVE the designated edge ...
	#	(HOWEVER when in <Split-mode>):	 IFF both edges WERE coincident, also
	#	conceptually PUSH (really drag) the OPPOSING edge along as well ...
	#	UNLESS the movement logically SEPARATEs the edges (ie. stops pushing)
	set aLIM $splcmb([string map $CSmap ${edge}$btn])	   ;# (a)pproached LIM
	set bLIM $splcmb([string map $CSmap ${edge}$otherB($btn)]) ;# (b)ehind LIM
	set oldE $splcmb(${side}$edge)			   ;# Edge ABOUT to move
	set oppE $splcmb(${side}$otherE($edge))	   ;# (opp)osed Edge <Split only>

	#	MOVE the EDGE  !!
	set newE [incr splcmb(${side}$edge) $mvEg($btn)]

	# Special condition (mostly meaningful for Combine):
	#	If moved edge WAS sitting *on* the "Opposite" LIM, its possibly ALSO
	#	a jump entry - so PRETEND we just moved THERE and let jumping fix it.
	# HOWEVER - This is really all about ensuring we NEVER "jump BACKWARD" by
	# accidentally STARTing from the "wrong direction" half of a jump tuple.
	#	(because *that* causes an endless-loop toggling jump condition)
	if {($oldE == $bLIM && [set i [lsearch $splcmb(j$side) $oldE]] >= 0) \
	&&	(($i & 1  && $mvEg($btn) < 0) || (!($i & 1) && $mvEg($btn) > 0))} {
		set newE $oldE}

	set i 0
	# Check if the move TRIGGERS a "jump": jumping moves to the "other end"
	# of the jump tuple (which MUST be in the direction we are moving) ...
	# and THEN moves the edge AGAIN (by 1) UNLESS doing so would exceed the
	# approaching limit. Barring that, each successful jump forces a new pass,
	# looking for an ABUTTED jump, until no more exist (or 'aLIM' is found)
	#	N.B> A Split NEVER has abutted entries - Combine may have several
	while {$i < [llength $splcmb(j$side)]} {
		set i 0 ;# (start a new pass - ends @ aLIM or when NO jump is found)
		foreach jmp $splcmb(j$side) {
			if {$jmp == $newE} {
				set newE [lindex $splcmb(j$side) [expr {$i & 1 ? $i-1 : $i+1}]]
				if {$newE == $aLIM} { set i [llength $splcmb(j$side)]
						 set splcmb(${side}$edge) $newE
				} else { set splcmb(${side}$edge) [incr newE $mvEg($btn)]
					if {$newE == $aLIM} { set i [llength $splcmb(j$side)] }
				}
				break
			}
			incr i
		}
	}
	# Also check if moving was "push"ing the opposing edge with it (Split only)
	if {$oldE == $oppE && $push(${edge}$btn)} {
		set oppE [set splcmb(${side}$otherE($edge)) $newE] }

	# Now the FUN - First, readjust which buttons will NOW be available ...
	set Bwdg $w(scDialog).btn.$side		;# (just conserving src-code typing)

	# First two rules apply to EITHER Combine OR Split (for moving edge)
	#	Exitting the 'behind' limit(activate  OTHER Button); and
	#	Entering the 'ahead'  limit(deactivate THIS Button)
	if {$oldE==$bLIM} { ${Bwdg}${edge}$otherB($btn) configure -state normal	 }
	if {$newE==$aLIM} { ${Bwdg}${edge}$btn			configure -state disabled}

	# The remainder applies ONLY to SPLIT (and is caused by PUSHING)
	#	Push other edge INTO 'approach' limit (deactivate other edge+SAME btn)
	#	Push other edge OUT of 'behind' limit (activate OTHER edge AND button)
	if {!$CS && $push(${edge}$btn) && $oppE==$newE} {
		if {$oppE == $aLIM} {
			${Bwdg}$otherE($edge)$btn				configure -state disabled}
		if {$oldE == $bLIM} {
			${Bwdg}$otherE($edge)$otherB($btn)		configure -state normal	 }
	}

	# ... THEN add visual user feedback of what this boundary move MEANT
	splcmb-Feedback $CS

	# ADJUST Text VIEW (in the side just changed) so we see what happenned
	#	(N.B> user is UNABLE to scroll for themselves ... grab is in force)
	#
	# Conceptually, we test for whether oldE is visible (but must CALC (=i)
	# an "equivalent" oldE to eliminate any potential "jump" usage earlier)
	# Ultimately we choose the faked oldE, or a horizoned-version of newE
	# and tell the widget to FORCE that line to be visible - EITHER yeilds
	# a view that keeps the moved subregion boundary edge ONSCREEN.
	#	(Only adjusts the widget whose 'side' actually moved if scroll!=synced)
	set side [string map {l LeftText r RightText} $side]
	set i $newE
	# If our 'faked' oldE IS ALREADY onscreen, maintain the "horizoned" view
	if {[$w($side) bbox [incr i $mvEg($otherB($btn))].0]!={}} {
		incr mvEg(u) -1			   ;# (modify into a BALANCED horizon envelope)
		incr newE $mvEg($btn)	   ;# ensure NEW edge is INSIDE that horizon
	# else PUTTING that oldE location ONscreen will CREATE the "horizoned" view
	} { set newE $i }
	$w($side) SEE $newE.0
}

###############################################################################
# Interpret, display and produce a data mapping of the CURRENT moved-edge state
###############################################################################
proc splcmb-Feedback {Combine} {
	global g w splcmb

	# Begin by UNtagging all Split/Combine highlighting from affected area
	foreach wdg "$w(LeftText) $w(RightText)" {
		foreach tag {scCDR scADD scDEL scCHG scPSH} {
			$wdg tag remove $tag $splcmb(ou).0 $splcmb(ol).0
		}
	}

	# Then put back what belongs based on CURRENT boundary conditions
	#	For Combine, compute the current EFFECTIVE Outer (U/L) bounds;
	#	Split ALREADY knows those bounds - just copy to the local vars
	if {$Combine} {
		# Begin by FINDING the outer (u/l) edges of the INVOLVED hIDs
		#	(remember to discount the +1 of the lower edges when comparing)
		set upper [set lower 0]
		foreach hunk $splcmb(rnge) {
			lassign $hunk S E na na na na hID
			if {($splcmb(lu)   > $E && $splcmb(ru)	 > $E) \
			||	($splcmb(ll)-1 < $S && $splcmb(rl)-1 < $S)} {continue}

			# extract type
			regexp {[0-9,]*([acd])[0-9,]*} $hID na type

			# Retain JUST the first and last edge values (and its diff-type)
			if {!$upper}	 {set upper $S; set typ(u) $type}
			if {$E > $upper} {set lower $E; set typ(l) $type; incr lower}
		}
	} else { lassign "$splcmb(ou) $splcmb(ol)" upper lower }

	# Now, arrange ALL edges (working and limits) as 3 top-to-btm
	# sub-regions, noting which HAS any content (per sub-region, per side).
	foreach LR {l r} {
		lassign {0 1 0} splcmb(${LR}1) splcmb(${LR}2) splcmb(${LR}3)
		set splcmb(${LR}1)	[expr \
				   {[set t(1$LR) $upper] < [set b(1$LR) $splcmb(${LR}u)]}]

		set splcmb(${LR}2)	[expr \
		  {[set t(2$LR) $splcmb(${LR}u)] < [set b(2$LR) $splcmb(${LR}l)]}]

		set splcmb(${LR}3)	[expr \
				   {[set t(3$LR) $splcmb(${LR}l)] < [set b(3$LR) $lower]}]

		# Dbg [join [list \
		"<${LR}1>$splcmb(${LR}1)  <t1$LR>$t(1$LR)  <b1$LR>$b(1$LR)"	  \
		"<${LR}2>$splcmb(${LR}2)  <t2$LR>$t(2$LR)  <b2$LR>$b(2$LR)"	  \
		"<${LR}3>$splcmb(${LR}3)  <t3$LR>$t(3$LR)  <b3$LR>$b(3$LR)"] "\n	 "]
	}

	# Then "paint" (tag) the occupied sub-regions in appropriate MAP colors
	# based on the LOGICALLY IMPLIED DIFFERENCE of each sub-region pairing
	# ALSO RECORD (via L/R sub-region 'D'atums) WHICH lines + type was set
	#
	# N.B> DECREMENTing 'bottom' values IN-BETWEEN its widget use and the
	#	subsequent recording produces a PURE "screen Lnum" data viewpoint
	#
	# Note: The only distinction reqd for 'Combine' is to PREVENT treating
	# the 'Pad'-only half of region 1&3 'a/d'-type hunks AS data (by turning
	# the 'occupied' flag OFF ... *AFTER* highlighting for user feedback)
	foreach rgn {1 2 3} {
		if {$splcmb(r$rgn) && $splcmb(l$rgn)} {
			if {$rgn == 2} {	   set tag scCDR
			if {! $splcmb(r$rgn)} {set tag scDEL}
			if {! $splcmb(l$rgn)} {set tag scADD}
			} else				  {set tag scCHG}
			$w(LeftText)  tag add  $tag	 $t(${rgn}l).0 $b(${rgn}l).0
			$w(RightText) tag add  $tag	 $t(${rgn}r).0 $b(${rgn}r).0
			incr b(${rgn}l) -1
			incr b(${rgn}r) -1
			set splcmb(l${rgn}D) \
					 "$t(${rgn}l) $b(${rgn}l) [string range $tag 2 4]"
			set splcmb(r${rgn}D) \
					 "$t(${rgn}r) $b(${rgn}r) [string range $tag 2 4]"

		} elseif {$splcmb(r$rgn)} {
			$w(RightText) tag add scADD $t(${rgn}r).0 $b(${rgn}r).0
			if {!$Combine && $rgn==2} {
				$w(LeftText) tag add scPSH $t(2l).0 [expr $b(2l)+1].0
			}
			incr b(${rgn}r) -1
			if {$Combine && (($rgn==1 && "$typ(u)"=="d") \
			|| ($rgn==3 && "$typ(l)"=="d"))} {set splcmb(r$rgn) 0}
			set splcmb(r${rgn}D) "$t(${rgn}r) $b(${rgn}r) ADD"

		} elseif {$splcmb(l$rgn)} {
			$w(LeftText)  tag add scDEL $t(${rgn}l).0 $b(${rgn}l).0
			if {!$Combine && $rgn==2} {
				$w(RightText) tag add scPSH $t(2r).0 [expr $b(2r)+1].0
			}
			incr b(${rgn}l) -1
			if {$Combine && (($rgn==1 && "$typ(l)"=="a") \
			|| ($rgn==3 && "$typ(l)"=="a"))} {set splcmb(l$rgn) 0}
			set splcmb(l${rgn}D) "$t(${rgn}l) $b(${rgn}l) DEL"

		}
	}
}

###############################################################################
# Primarily code that advises (1|0) on eligibility of hunk for Split/Combine...
# ...but also provides a formatted STDOUT data-dump for debugging purposes
###############################################################################
proc splcmb-chk {what {pos 0}} {
	global g splcmb

	switch -exact -- $what {
		"split" {
			# Is dependant on there being MORE than 1 line on EITHER side
			# N.B> this PREVENTS splitting ANY one-line hunk (incl. "chg"-type)
			if {$pos <= $g(count) && $g(count) > 0} {
				lassign $g(scrInf,[hunk-id $pos]) S E Pl na na Pr
				return [expr {($E - $S) || ($Pl + $Pr > 1)}]
			}
		}

		"cmbin" {
			# Is dependant on there being some hunk ABUTTED either above/below
			if {$pos <= $g(count) && $g(count) > 1} {

				# Grab edge values of the target CDR at 'pos'
				lassign $g(scrInf,[hunk-id $pos]) S E

				# Validate and check BELOW target first, then ABOVE - and exit ASAP
				if {[incr pos -1]} {
					if {($S - 1 == [lindex $g(scrInf,[hunk-id $pos]) 1])} {return 1}
				}
				if {[incr pos 2] <= $g(count)} {
					if {($E + 1 == [lindex $g(scrInf,[hunk-id $pos]) 0])} {return 1}
				}
			}
		}

		"data" {
			if {"$pos" != "0"} { puts "***** $pos" } ;# <-- simply a dump identifier
			# This is a DRAMATICALLY more READABLE output format!!!
			puts " EDGES : <l>$splcmb(lu) $splcmb(ll)	 <r>$splcmb(ru) $splcmb(rl)"
			puts " AMONG :"
			foreach {S E Pl Ol Pr Or hID} [join $splcmb(rnge)] {
				puts "[format "\t%d	 %d	   P=%d,%d	  O=%d,%d	 %s" \
								 $S	 $E		$Pl $Pr	   $Ol $Or	$hID]"
			}
			puts "\nou $splcmb(ou)"
			foreach side {l r} {
				foreach rgn {1 2 3} {
					if {$splcmb(${side}$rgn)} {
						puts "\t${side}$rgn $splcmb(${side}$rgn)\t${side}${rgn}D\
														  $splcmb(${side}${rgn}D)"
					} else {puts "\t${side}$rgn $splcmb(${side}$rgn)"}
				}
				if {"$splcmb(j$side)" != {}} {puts "\t\tj$side	$splcmb(j$side)"}
				if {"$side" == "l"} { if {[llength $splcmb(rnge)] > 1} {
					puts "iu $splcmb(iu)\n\t(CDR)\nil $splcmb(il)" }  { puts "" }
				}
			}
			puts "ol+ $splcmb(ol)\n"
		}
	}
	return 0
}

###############################################################################
# All the code to implement the report writing dialog.
#	N.B> the ONLY "public" subcmd is 'popup'; all others are for INTERNAL usage
###############################################################################
proc rpt-gen {subcmd args} {
	global g w opts finfo report

	set w(reportPopup) .reportPopup

	# N.B> we COULD have 'passed' these around, but this was actually clearer
	#
	#	Need the number of SCREEN lines that exist (either side will do)
	#	(and "F" is simply a static list of FIELD NAMEs we read hunk data into)
	set		maxlns [file rootname [$w(acTxWdg) index end-1lines]]
	lappend F	   S E P(Left) O(Left) C(Left) P(Right) O(Right) C(Right)

	switch -- $subcmd {
	popup {
		# Put the dialog up on screen
		if {![Dialog MODAL $w(reportPopup)]} {
			wm title	 $w(reportPopup) "$g(name) - Generate Report"
			wm group	 $w(reportPopup) .
			wm transient $w(reportPopup) .
			wm protocol	 $w(reportPopup) WM_DELETE_WINDOW {rpt-gen dismiss}

			# Populate content ...
			#	and perform a ONE-TIME centering ...
			rpt-gen build
			centerWindow $w(reportPopup)
		}
		# Configure it for this usage
		unset -nocomplain report(stats)
		rpt-gen update
		set report(filename) [file join [pwd] $report(filename)]

		# The following does NOT return until the *Dialog* is completed
		Dialog show $w(reportPopup) w(status$w(reportPopup)) 0

		# Whether we WROTE or NOT we are done, take down the dialog
		#	(and Reset the filename validity to 'needs check' for next time)
		Dialog dismiss $w(reportPopup)
		set report(fnamVetted) 0
	}
	save -
	dismiss {
		# RELEASING the 'Dialog show' depends on the asking subcmd
		# AND if a writing REQUEST was actually permitted/successful
		if {$subcmd eq "save"} {
			if {![set rc [rpt-gen write]]} {
				# DO NOT release the Dialog -
				#	Let user pick a new filename or CHOOSE to bail out!
				return
			}
		} { set rc 0 }
		set w(status$w(reportPopup)) $rc
	}
	update {
		# Align all GUI elements with current settings

		lassign {disabled disabled} state(Left) state(Right)
		if {$report(doSideLeft)}  { set state(Left)	 "normal" }
		if {$report(doSideRight)} { set state(Right) "normal" }

		foreach side {Left Right} {
			foreach item {lnums cmrks text} {
				$w(reportPopup).cFrm.$item$side configure -state $state($side)
			}
		}
		# Compute the (minimally formatted) stats, posting it TO the dialog,
		# AND also HOLD onto it for report output (ONCE per dialog usage)
		if {![info exists report(stats)] || ![string length $report(stats)]}  {
			$w(reportPopup).msg configure									  \
				  -text [join [set report(stats) [rpt-gen stats $maxlns]] "\n"]
		}

		# Lastly, decide if a 'Bookmark"-style report choice is permitted
		set bkmOK [expr {[llength $report(BMrptgen)] ? "normal" : "disabled"}]
		foreach side {Left Right} {
			$w(reportTextMnu$side) entryconfigure "B*" -state $bkmOK
		}
	}
	stats {
		# Develop some simple statistical data (for REAL hunks ONLY)
		lassign { 0 0 0 0 "" 0 0 "" }  {*}$F
		set aCnt [set dCnt [set cCnt [set modLft 0]]]
		set aTot [set dTot [set cTot [set modRgt 0]]]
		foreach hID $g(diff) {
			lassign $g(scrInf,$hID) {*}$F

			switch -- "[append C(Left) $C(Right)]" {
			"+"	 { incr aCnt ; incr aTot $P(Left)  ; incr modRgt $P(Left) }
			"-"	 { incr dCnt ; incr dTot $P(Right) ; incr modLft $P(Right)}
			"!!" { incr cCnt ; incr cTot [expr {$P(Left) - $P(Right)}]
				   incr modLft	   [expr {$E - $S - $P(Left)  + 1}]
				   incr modRgt	   [expr {$E - $S - $P(Right) + 1}]		  }
			}
		}

		# ... next compute what we can from them ...
		#	(Note: maxlns derived from a WIDGET: has an EXTRA empty line)
		set sz(Left)  [expr {$maxlns - 1 - $O(Left)	 - $P(Left)	 }]
		set sz(Right) [expr {$maxlns - 1 - $O(Right) - $P(Right) }]
		set pctLft [expr {double($modLft*100) /double($sz(Left)) }]
		set pctRgt [expr {double($modRgt*100) /double($sz(Right))}]
		set totsz  [expr {		 $sz(Left)	  +		   $sz(Right)}]
		set totmod [expr {		 $modLft	  +		   $modRgt	 }]
		set totpct [expr {		 $pctLft	  +		   $pctRgt	 }]
		set effpct [expr {double($totmod*100) / double($totsz)	 }]

		# ... then format our findings (kinda NEEDs a MONO font) and ...
		lappend out "Number of diffs: $g(count)\n"
		set fmt "%6d regions were %s: %d(net) modified lines"
		lappend out [format "$fmt"	 $dCnt "deleted" $dTot]
		lappend out [format "$fmt"	 $aCnt " added " $aTot]
		lappend out [format "$fmt\n" $cCnt "changed" $cTot]
		set fmt "%6d %s lines were affected: %4.4g %% of %6d"
		lappend out [format "$fmt" $modLft "Left " $pctLft $sz(Left) ]
		lappend out [format "$fmt" $modRgt "Right" $pctRgt $sz(Right)]
		set fmt "%6d %s lines were involved: %4.4g %% or %6.4g %%"
		lappend out [format "$fmt" $totmod "Total" $totpct $effpct]

		# send it all back to the caller
		return $out
	}
	browse {
		set path [tk_getSaveFile -parent $w(reportPopup) \
			-filetypes $opts(filetypes) \
			-initialdir	 [file dirname $report(filename)] \
			-initialfile [file tail	   $report(filename)]]

		if {[string length $path] > 0} {
			set report(filename) $path
			set report(fnamVetted) 1
		}
	}
	write {
		if {!$report(fnamVetted)} {
			# Either this was just a default-generated name, or its a name that
			# was PLAYED with AFTER having BEEN Vetted - either way force user
			# to confirm and/or alter the name before we trash something.
			rpt-gen browse
			if {!$report(fnamVetted)} { return 0 }
		}

		# Apparently we are good to go - reset for next time and just DO it
		set report(fnamVetted) 0
		set handle [open $report(filename) w]

		puts $handle "$g(name) $g(version) report\t\t\
										[clock format [clock seconds]]"

		# Mention the file name(s) ... BOTH unless exactly one is OFF
		set not([set not(Right) Left]) Right
		foreach {side} {Left Right} {
			if {$report(doSide$side) || !$report(doSide$not($side))} {
				# (N.B> 'alignDecor' left this cookie just for us - pick it up)
				if {$g(tooltip,${side}Label)!={}} {
					set mtime [string range	 $g(tooltip,${side}Label)		  \
						  [string first "\n" $g(tooltip,${side}Label)]+1 end-1]
				} { set mtime "(@ today)" }
				# Yeah I know the padding seems strange - but it lines
				# up things (because L/R are 4/5 chars in length, resp.)
				puts $handle "	$side\tfile :  $finfo(lbl,$side)	$mtime"
			}
		}

		# Stats have already been Computed and Formatted, just include them
		#	(BUT remember to adapt its NL & spacing relative to the report)
		puts $handle "\n[join $report(stats) "\n  "]\n\n"

		# Translate the GUI setting regarding the DESIRED output format into
		# something a bit EASIER to use (if not understand -> boolean logic)
		#	Fairly simple - Should output be limited to:
		#		2	   ALL	 'D'iff 'R'egions
		#		1	SPECIFIC 'DR's (those bookmarked)
		#		0	no DR-related restrictions whatsoever
		switch -glob $report(doText$side) {
			"Diff*" { set DR 2 }
			"Book*" { set DR 1 }
			default { set DR 0 }
		}

		# Pre-Load FIRST PHYSICAL hunk (if any - IGNOREs MAY still exist)
		#	("H", "skpH" & "pfxH" just track the hunk 'ndx' for use later)
		#
		# IMPORTANT: Note we are walking through g(DIFF) not g(diff) !!
		# N.B. code DETECTs & INTERPOLATEs further hunks AS lines advance
		if {$g(COUNT) > [set i [set skpH [set pfxH [set H 0]]]]} {
			lassign $g(scrInf,[set hID [hunk-id [incr H] DIFF]]) {*}$F
			if {[info exists g(overlap$hID)]}	  {
				set C(Left) [set C(Right) "?"]} \
			elseif {"$C(Left)$C(Right)" == ""}	  {
				incr skpH ;# must account for an 'ignored' hunk
			}
			# A 'S'ignificant 'D'iff 'R'egion is one that can LIMIT the output
			# based on a confluence of chosen MODE and the specific region
			set SDR [expr {($DR > 1) || ($DR && $hID in $report(BMrptgen))}]
		} else { lassign { 0 0 0 0 "" 0 0 "" 0 }  {*}$F SDR}

		# Now produce the requested categories of data (if any)
		if {(!$report(doSideRight) && !$report(doSideLeft))} {set maxlns 0}
		while {[incr i] < $maxlns} {

			set out(Left) [set out(Right) ""]
			foreach side {Left Right} {
				if {!$report(doSide$side)} {continue}

				# Waterfall test detects phase of WHERE "$i" falls IN hunk,
				# thus what SHOULD be displayed (if not 'off' by request)
				#
				# N.B> DESPITE coding as loop - this RARELY ever needs to!
				#	It exists ENTIRELY because there is NO 'goto' in Tcl;
				# thus a 'continue' is the ONLY way to RE-start this code !
				while {true} {

				if {$H > 0 && $i >= $S} {
					if {$i > ($E - $P($side))} {
						if {$i > $E} {
							if {$H < $g(COUNT)} {
								# Step forward to the NEXT hunk mapping
								set hID [hunk-id [incr H] DIFF]
								lassign $g(scrInf,$hID) {*}$F
								if {[info exists g(overlap$hID)]}  {
										   set C(Left) [set C(Right) "?"]} \
								elseif {"$C(Left)$C(Right)" == ""} {
									incr skpH ;# account for 'ignored' hunks
								}
								# Establish 'significance' of this NEW region
								set SDR [expr {($DR > 1) \
									|| ($DR && $hID in $report(BMrptgen))}]

								# WHY IS THERE NO goto IN THIS LANGUAGE!!!
								#
								# RESTART waterfall: 'i' MIGHT now be INSIDE
								# newly read-in hunk (supports abutted hunk
								# defs as created by Split/Combine feature)
								continue
								## (Poor PGMRS is the problem - NOT goto !)

							} elseif {$P($side)} {
							  # Fixup trailing Lnums when FINAL hunk padded
								   incr O($side) $P($side); set P($side) 0}
									set LN 1;set CB 0	;# Is beyond hunk
						} else	  { set LN	[set CB 0]} ;# A PADDING line
					} else		  { set LN	[set CB 1]} ;# A  DIFF	 line
				} else			  { set LN 1;set CB 0 } ;# Is before hunk

				break ;# if we reach here, we need NOT go back around!!
				}

				# "Diffs Only" or "Bookmarked" acts as a filter, blocking ALL
				# output UNTIL we are INSIDE a diff region. Else it does NADA!
				#	The derivation of a 'S'ignificant 'D'iff 'R'egion comes
				#	from both the Text mode chosen and the CURRENT region and
				#	was determined earlier as we encountered each region
				if {$DR} {
					# If line is OUTSIDE of ANY hunk, SKIP (due to DR mode)
					if {($LN ^ $CB)} {						continue
					# Watch for the 1st line of ANY Diff (it needs counting)
					} elseif {$pfxH < ($H - $skpH)} {
						# if 'significant' - produce a label AND count it
						#	(but count REGARDLESS to keep # correct)
						if {$SDR} {
							puts $handle  "\nDiff #[incr pfxH] ($hID):"
						} { incr pfxH ;						continue }
					# But suppress ANY Diff line that is NOT significant
					} elseif {!$SDR}					  { continue }
				}

				if {$report(doLnums$side)} {
					if {$LN} { append out($side) \
					   [format "%*d " $g(lnumDigits) [expr {$i-$O($side)}]]
					} else {continue}
					# N.B> LN==0 implys a PAD line (No CMrk/Text can exist)
					#	Thus no need to append ANYTHING more to this line !!
				}

				if {$report(doCMrks$side)} {
					append out($side) [string range \
								  [expr {$CB ? "$C($side)  " : "  "}] 0 1]
				}

				if {"$report(doText$side)" != " (no text) "} {
					append out($side) [string trimright \
							   [$w(${side}Text) get "$i.0" "$i.0 lineend"]]
				}
			}

			if {$report(doSideLeft) == 1 && $report(doSideRight) == 1} {
				set output [format "%-90s%-90s" "$out(Left)" "$out(Right)"]

			} elseif {$report(doSideRight) == 1} {
				set output "$out(Right)"

			} elseif {$report(doSideLeft) == 1} {
				set output "$out(Left)"

			}
			set output "[string trimright "$output"]"
			if {[string length "$output"]} { puts $handle "$output" }
		}
		close $handle
		return 1
	}
	build {
		# The major guts goes inside the "client Frame" (cFrm)
		#	except for buttons (so we can hold onto them during resizing)
#### MCP
#		set cf [frame $w(reportPopup).cFrm -bd 2 -relief groove]
		set cf [frame $w(reportPopup).cFrm]
#### MCP
		set bf [frame $w(reportPopup).bFrm -bd 0]
		pack $bf -side bottom -fill x	 -expand n
		pack $cf -side bottom -fill both -expand y -padx 5 -pady 5
		# Apologies, but this REALLY NEEDS a Mono-spaced font!!!
		pack [message $w(reportPopup).msg -aspect 500						 \
										  -font {"Courier" 11 italic}] -pady 5
		# buttons...
		pack [button $bf.cancel -text "Cancel" -underline 0 -width 6 \
					   -command {rpt-gen dismiss}] -side right -padx 5 -pady 5
		pack [button $bf.save -text "Save" -underline 0 -width 6 \
								 -command {rpt-gen save}]  -side right -pady 5

		# client area.
		#	Treat this as a 5-col area, so we can basically spread any
		#	expansion SPACING among the EMPTY columns
		set col(Left) 1
		set col(Right) 3
		foreach side {Left Right} {
			set pickS [checkbutton $cf.pickS$side -command {rpt-gen update}]
			set lnums [checkbutton $cf.lnums$side]
			set cmrks [checkbutton $cf.cmrks$side]
			set mnu [tk_optionMenu [set txt $cf.text$side] report(doText$side)\
					 "Full Text"   "Diffs Only"	  "Bookmarked"	 " (no text) "]

			$pickS configure -text "$side Side"		-var report(doSide$side)
			$lnums configure -text "Line Numbers"	-var report(doLnums$side)
			$cmrks configure -text "Change Markers" -var report(doCMrks$side)

			# we need this MENU from the above for config ops in subcmd 'updat'
			set w(reportTextMnu$side) $mnu

			grid $pickS	 -row 0 -column $col($side) -sticky w
			grid $lnums	 -row 1 -column $col($side) -sticky w -padx {10 0}
			grid $cmrks	 -row 2 -column $col($side) -sticky w -padx {10 0}
			grid $txt	 -row 3 -column $col($side) -sticky w -padx {10 0}
		}

		# the entry, label and button for the filename will get
		# stuffed into a "file frame" (fFrm) for convenience...
		frame $cf.fFrm -bd 0
		grid  $cf.fFrm -row 4 -column 0 -columnspan 5 -sticky ew -padx {0 5}

		label  $cf.fFrm.l -text "File:"
		entry  $cf.fFrm.e -textvar report(filename) -width 30 -validate key \
								-vcmd {set report(fnamVetted) 0; return true}
		button $cf.fFrm.b -text "Browse..." -command {rpt-gen browse} \
			-highlightthickness 0 -bd 1 -pady 0

		pack $cf.fFrm.b -side right -pady 4 -anchor se -padx 2
		pack $cf.fFrm.l -side left	-pady 4 -anchor sw -padx 2
		pack $cf.fFrm.e -side left	-pady 4 -fill x -expand y

		grid rowconfigure	 $cf {0 1 2 3} -weight 0
		grid columnconfigure $cf  {0 2 4}  -weight 1 -uniform a
	}
	}
}

###############################################################################
# Open one of the diff'd files in an editor - IF PERMITTED
#	Fundamentally depends on w(acTxWdg) to designate which file
#
# Always attempts to use FOCUS to SHIFT the active window first (which only
# works if that window IS one of the Text windows), then OVERRIDES it
# with the window containing (optionally provided) X,Y ROOT coordinates
#	(implying it came from a mouse-based binding and NOT a keyboard one).
# We accept ANY window having a L/R attribute to perform the override (as
# they are the only ones even HAVING the binding needed to MAKE the call).
#	IN ANY EVENT - w(acTxWdg) is adjusted and then directs the file to grab.
###############################################################################
proc do-edit { {X {}} {Y {}} } {
	global g w opts finfo

	# IF ROOT coordinates were provided, we use them to PICK the Window
	# being TARGETTED as 'active' (with a fallback to current FOCUS)
	if {$X!={} && $Y!={}} {
		set win [list [focus] [winfo containing $X $Y]]
	} { set win [list [focus]] }

	# Attempt to assign w(acTxWdg) reasonably
	# First is FOCUS (to establish the fallback),
	# then OVERRIDE with the mouse coordinates (IFF they were provided)
	foreach win $win {
		foreach side {Left Right} {
			foreach item {Text Info VSB HSB Label} {
				# Only LEGITIMATE windows are permitted to TRY
				if {$win == $w($side$item)}	 {
					set w(acTxWdg) $w(${side}Text)
					break
				}
			}
		}
	}

	# Locate the correct filename
	set ndx [expr {$finfo(fCurpair) * 2}]
	if {$w(acTxWdg) == $w(LeftText)} {incr ndx -1}

	if {![info exists finfo(tmp,$ndx)]} {
		# Got the file - GET the line number
		set file "$finfo(pth,$ndx)"
		if {$g(count)} {
			lassign $g(scrInf,$g(currdiff)) line na na O(1) na na O(0)
			incr line -$O([expr {int($ndx & 1)}])
		} else {set line 1} ;# have to pick something if no CDR exists

		if {[string length [string trim $opts(editor)]] == 0} {
			simpleEd open "$file" $line
		} elseif {[regexp "\\\$file" "$opts(editor)"] == 1} {
			eval set cmdline \"$opts(editor) &\"
			Dbg "exec $cmdline"
			eval exec $cmdline
		} else {
			Dbg "exec $opts(editor) \"{$file}\" &"
			eval exec $opts(editor) "{$file}" &
		}
	} else { popmsg "This file is not editable" warning "Dis-allowed" }
}

##########################################################################
# A simple editor, from Bryan Oakley.
# 22Jun2018	 mpm: now accepts (opt.) line number to display (dflt = 1)
# 04Aug2018	 mpm: additional keywords/parsing added for open subcmd
#			 mpm: now provides line numbering (in adjoining subwindow)
##########################################################################
proc simpleEd {command args} {
	global textfont

	switch -- $command {
	open {
			# Ingest required args (and establish default options):
			#	filename
			if {[set argn [llength $args]]} {
				set filename [lindex $args [set count 0]]
				set line 1
				set title  "$filename - Simple Editor"
				set FG {}
				set BG {}
			} {error "simpleEd open ?filename?: reqd arg missing"}

			# ... then see if others were provided (in any order)
			#	[Lnum] ['fg' color] ['bg' color] ['title' xxxx] ['ro']
			while {[incr count] < $argn} {
				switch -glob [set arg [lindex $args $count]] {
				"\[0-9]" { set line $arg }
				"f*"	 { lappend FG -fg [lindex $args [incr count]] }
				"b*"	 { lappend BG -bg [lindex $args [incr count]] }
				"t*"	 { set title	  [lindex $args [incr count]] }
				"ro"	 { set RO	 [list configure -state disabled] }
				}
			}

			set w .editor
			set count 0
			while {[winfo exists ${w}$count]} {
				incr count 1
			}
			set w ${w}$count

			toplevel $w -borderwidth 2 -relief sunken
			wm title $w $title
			wm group $w .

			menu $w.menubar
			$w configure -menu $w.menubar
			$w.menubar add cascade -label "File" -menu $w.menubar.fileMenu

			menu $w.menubar.fileMenu

			if {![info exists RO]} {
				$w.menubar.fileMenu add command -label "Save" \
				  -underline 1 -command [list simpleEd save $filename $w]
				$w.menubar.fileMenu add command -label "Save As..." \
				  -underline 1 -command [list simpleEd saveAs $filename $w]
				$w.menubar.fileMenu add separator
			}
			$w.menubar.fileMenu add command -label "Exit" -underline 1 \
			  -command [list simpleEd exit $w]

			if {![info exists RO]} {
				$w.menubar add cascade -label "Edit" -menu $w.menubar.editMenu

				menu $w.menubar.editMenu

				$w.menubar.editMenu add command -label "Cut" -command \
				  [list event  generate $w.text <<Cut>>]
				$w.menubar.editMenu add command -label "Copy" -command \
				  [list event generate $w.text <<Copy>>]
				$w.menubar.editMenu add command -label "Paste" -command \
				  [list event generate $w.text <<Paste>>]
			}

			text $w.text -wrap none -xscrollcommand [list $w.hsb set] \
			  -yscrollcommand [list $w.vsb set] -borderwidth 0 \
			  -font $textfont {*}$FG {*}$BG
			scrollbar $w.vsb -orient vertical -command [list $w.text yview]
			scrollbar $w.hsb -orient horizontal -command [list $w.text xview]

			# Derive needed info to fabricate/utilize a line numbering canvas
			set Aft [font metrics $textfont -ascent]   ;# Ascent of font
			set Dw	[font measure $textfont "8"]	   ;# Digit width
			set Fg	[$w.text cget -fg]		  ;# Same foreground & background
			canvas $w.cnvs -highlightthickness 0 -bg [$w.text cget -bg]

			grid $w.cnvs -row 0 -column 0 -sticky nsew
			grid $w.text -row 0 -column 1 -sticky nsew
			grid $w.vsb -row 0 -column 2 -sticky ns
			grid $w.hsb -row 1 -column 1 -sticky ew

			grid columnconfigure $w 0 -weight 0
			grid columnconfigure $w 1 -weight 1
			grid columnconfigure $w 2 -weight 0
			grid rowconfigure $w 0 -weight 1
			grid rowconfigure $w 1 -weight 0

#### MCP
#			set fd [open $filename]
			set fd [ gzopen $filename ]
#### MCP
			# PREVENT V9.x from throwing 'encoding' errors
			if {$::tcl_version >= 9.0} { fconfigure $fd -profile tcl8 }
			$w.text insert 1.0 [read $fd]
			close $fd

			set lenDigits [string length [$w.text index end]]
			$w.cnvs configure -width [set X [expr {int($lenDigits-2)*$Dw+3}]]
			# N.B> tracing on the Vert-Scrlbar trips on window resizes too
			trace add exec $w.vsb leave [list apply "{Fg Asc X args} {
				$w.cnvs delete all
				set Lnum \[file rootname \[$w.text index @0,0]]
				set LastLnum \[file rootname \[$w.text index end-1lines]]
				while {\[llength \[set dl \[$w.text dlineinfo \$Lnum.0]]]>0} {
					if {\$Lnum == \$LastLnum} {break} ;# ignore extra last line
					lassign \$dl na y na na bl
					incr y \$bl
					incr y -\$Asc
					$w.cnvs create text \$X \$y -anchor ne -font \"$textfont\" \
							  -fill \$Fg -text \$Lnum
					incr Lnum
				}
				update idletasks
			}" $Fg $Aft [incr X -2]]
			$w.text see $line.0 ;# N.B> done AFTER the trace setup to tickle it
			if {[info exists RO]} {$w.text {*}$RO }
		}
	save {
			set filename [lindex $args 0]
			set w [lindex $args 1]
			set fd [open $filename w]
			puts $fd [$w.text get 1.0 "end-1c"]
			close $fd
		}
	saveAs {
			set filename [lindex $args 0]
			set w [lindex $args 1]
			set filename [tk_getSaveFile -filetypes $opts(filetypes) \
										 -initialfile [file tail $filename] \
										 -initialdir [file dirname $filename]]
			if {$filename != ""} {
				simpleEd save $filename $w
			}
		}
	exit {
			set w [lindex $args 0]
			destroy $w
		}
	}
}
# end of simpleEd

# tooltips version 0.1
# Paul Boyer
# Science Applications International Corp.
#
# MODIFIED (for TkDiff)
# 31Jul2018	 mpm: NO-OP/UN-bind(s) if setting to an empty description
##############################
# set_tooltips gets a button's name and the tooltip string as arguments
# and creates the proper bindings for entering and leaving the button
# Serves as the external interface to the feature
#	(helper procs carry a "i"nternal "T"ool "T"ip naming prefix)
##############################
proc set_tooltips {widget tiptxt} {
	global g

	if {$tiptxt == {}} {
		bind $widget <Enter> {}
		bind $widget <Leave> {}
		bind $widget <Button-1> {}
		return
	}

	if {![info exists g(tooltip_id)]} {set g(tooltip_id) "Initialized"}
	bind $widget <Enter> "
		catch { after 500 { iTT_PopUp %W $tiptxt } }  g(tooltip_id)
		"
	bind $widget <Leave>	{iTT_PopDown}
	bind $widget <Button-1> {iTT_PopDown}
}

##############################
# internal_tooltips_PopDown is used to de-activate the tooltip window
##############################
proc iTT_PopDown {} {
	global g

	after cancel $g(tooltip_id)
	catch {destroy .tooltips_wind}
}

##############################
# internal_tooltips_PopUp is used to activate the tooltip window
#
# MODIFIED (for TkDiff)
# 20Oct2020	 mpm: TK issue? (lack of multi-monitor screen(x/y) ORIGIN)
#				  (also tighten'd minor extraneous border, etc. pixels)
##############################
proc iTT_PopUp {wid tiptxt} {
	global g w opts

	# get rid of other existing tooltips
	catch {destroy .tooltips_wind}

	toplevel .tooltips_wind -class ToolTip -highlightthickness 0 -bd 0
	set size_changed 0

	# get the cursor position
	set X [winfo pointerx $wid]
	set Y [winfo pointery $wid]

	# add a slight offset to make tooltips fall below cursor
	set Y [expr {$Y + 20}]

	# Now pop up the new widgetLabel
	wm overrideredirect .tooltips_wind 1
	wm geometry .tooltips_wind +$X+$Y
	label .tooltips_wind.l -text $tiptxt -border 1 -relief raised \
	  -background $opts(inform) -foreground $w(fgnd)
	pack .tooltips_wind.l

	# make invisible
	wm withdraw .tooltips_wind
	update idletasks

	# Dont let the ToolTip get CLIPPED by the screen edge
	#	N.B> would have PREFERRED "winfo screen[x|y] $wid" but doesn't exist!
	#	*mpm* (TK missing-feature bug?: can FAIL on 2nd,3rd...etc monitors)
	#	Need to know SOME extent (aka display bounds) cursor is WITHIN !!
	lassign [iTT_scrnEdges $wid] na na RgtLoc BotLoc

	# adjust for bottom of screen
	if {($Y + [winfo reqheight .tooltips_wind]) > $BotLoc } {
		# ?? shouldn't the 25 REALLY be 2 times the original offset of 20 ??
		set Y [expr {$BotLoc - [winfo reqheight .tooltips_wind] - 25}]
		set size_changed 1
	}
	# adjust for right border of screen
	if {($X + [winfo reqwidth .tooltips_wind]) > $RgtLoc } {
		set X [expr {$RgtLoc - [winfo reqwidth .tooltips_wind]}]
		set size_changed 1
	}
	# reset position
	if {$size_changed == 1} {
		wm geometry .tooltips_wind +$X+$Y
	}
	# make visible
	wm deiconify .tooltips_wind
	raise .tooltips_wind ;# Reqd MacOS (no harm anybody else): AFTER deiconify!

	# make tooltip disappear after 5 sec
	set g(tooltip_id) [after 5000 { iTT_PopDown }]
}

###############################################################################
# internal_tooltips_scrnEdges is used to avoid clipping the tooltip window
#
# TK-BUG/oversight/failure:
#	In a multi-screen configuration, TK fails to provide a means to LOCATE the
# screen edges. Historically (aka: single screen) the location was IMPLIED to
# be at (0,0), thus only the width/height were ever provided. However, it is
# UNCLEAR the PROPER (W,H) is being returned when requested, as the POSITION of
# the requestor can appear to be OUTSIDE that returned (0,0)-based (W,H).
#	This proc compensates (poorly) by USING the (W,H) when it "appears" to be
# valid, and substitutes the dimensions of its "Toplevel" when not. As such it
# must be possible to ASK for the position/size of a REALIZED toplevel (not
# one still under construction, or incompletely modified). Be careful!
#
#	Arg:  the same as "winfo screen(width/height)": an 'exemplar' window
#
#	Returns: list of 4 (position) values "minX minY maxX maxY" of edges to USE
###############################################################################
proc iTT_scrnEdges { win } {

	lassign "[winfo vrootx	$win] [winfo vrooty	 $win]
			 [winfo rootx	$win] [winfo rooty	 $win]
			 [winfo width	$win] [winfo height	 $win]				  0 0
			 [winfo screenw $win] [winfo screenh $win]" vx vy x y w h X Y W H

# DBG shows everything known; PROVING TK cant tell us where the edges are of any
# ONE monitor is when the Display is configured in a multi-monitor arrangement !!
#	 Dbg "vx($vx) vy($vy) x($x) y($y) w($w) h($h) X($X) Y($Y) W($W) H($H)\
									.WxH([winfo screenw .] [winfo screenh .])"

	# If the LOCATION of the exemplar window is WITHIN the TK-provided screen
	# width/height, then use IT; otherwise fallback to using the Toplevel it
	# belongs to (best option - lousy though it is)!
	#	('inclusion' test LOOKS for any aspect of non-inclusion: 0==inside)
	if {($x+$vx+$w<$X) ||($x+$vx+$w>$X+$W) ||($x+$vx<$X) ||($x+$vx>$X+$W)
	||	($y+$vy+$h<$Y) ||($y+$vy+$h>$Y+$H) ||($y+$vy<$Y) ||($y+$vy>$Y+$H)} {
		set TL [winfo toplevel	$win]
		lassign "[winfo rootx $TL] [winfo rooty	 $TL]
				 [winfo width $TL] [winfo height $TL]" X Y W H
		incr W $X
		incr H $Y
	}
	return "$X $Y $W $H"
}

proc get_gtk_params { } {
	global w

	if {! [llength [auto_execok xrdb]]} { return 0 }
	set pipe [open "|xrdb -q" r]

	while {[gets $pipe ln] > -1} {
		switch -glob -- $ln {
			{\*Toplevel.background:*}	{ set bg	 [lindex $ln 1] }
			{\*Toplevel.foreground:*}	{ set fg	 [lindex $ln 1] }
			{\*Text.background:*}		{ set textbg [lindex $ln 1] }
			{\*Text.foreground:*}		{ set textfg [lindex $ln 1] }
			{\*Text.selectBackground:*} { set hlbg	 [lindex $ln 1] }
			{\*Text.selectForeground:*} { set hlfg	 [lindex $ln 1] }
		}
	}
	close $pipe

	if {! [info exists bg] || ! [info exists fg]} { return 0 }
	set w(selcolor) $hlbg
	option add *Entry.Background $textbg
	option add *Entry.Foreground $textfg
	option add *Entry.selectBackground $hlbg
	option add *Entry.selectForeground $hlfg
	option add *Entry.readonlyBackground $bg
	option add *Listbox.background $textbg
	option add *Listbox.selectBackground $hlbg
	option add *Listbox.selectForeground $hlfg
	option add *Text.Background $textbg
	option add *Text.Foreground $textfg
	option add *Text.selectBackground $hlbg
	option add *Text.selectForeground $hlfg
	# Menu checkboxes
	option add *Menu.selectColor $fg
	option add *Checkbutton.selectColor ""
	option add *Radiobutton.selectColor ""

	Dbg "Gtk default visual options established"
	return 1
}

###############################################################################
# Mac platform-specific display stuff
#	Note: The 'Dbg' used to be a DBoxProc setting (which is/was ALSO Modal)
#		When we added $modal to be able to CHOOSE, we had to DROP its use
#		(which was FINE as it also slipped into "no-longer-recognized")
#	SO - we either tell Aqua about MODAL windows or we tell it nothing -
#	Tclers Wiki reference page "Aqua Toplevels" has broken refs or we would
#	have tried to pick an explicit NON-modal style keyword (if there is one)
###############################################################################
proc setAquaDialogStyle {toplev modal {err {}}} { if { !$modal ||
	[catch {tk::unsupported::MacWindowStyle style $toplev moveableModal} err]} {
		Dbg "if modal($modal) then MacWindowStyle moveableModal failed? {$err}"
	}
}

proc get_aqua_params {} {
	global w

	# No longer invoked as of Tk8.6
	# since it now uses the OS's lignt/dark themes appropriately

	# Keep everything from being blinding white
	option add *Frame.background		   #ebebeb userDefault
	option add *Label.background		   #ebebeb userDefault
	option add *Checkbutton.Background	   #ebebeb userDefault
	option add *Radiobutton.Background	   #ebebeb userDefault
	option add *Message.Background		   #ebebeb userDefault

	# or else there are little white boxes around the button "pill"
	option add *Button.highlightBackground #ebebeb userDefault
	option add *Entry.highlightBackground  #ebebeb userDefault
	Dbg "Default AQUA visual options established"
}

###############################################################################
# Report the version of wish
###############################################################################
proc about-wish {} {
	global tk_patchLevel

	set version $tk_patchLevel
	set whichwish [info nameofexecutable]

	set about_string "$whichwish\n\nTk version	$version"

	popmsg $about_string info "About Wish"
}

###############################################################################
# Report the version of diff
###############################################################################
proc about-diff {} {

	set whichdiff [auto_execok diff]
	if {[llength $whichdiff]} {
		set whichdiff [join $whichdiff]
		set cmdline "diff -v"
		catch {eval "exec $cmdline"} output
		set message "$whichdiff\n$output"
	} else { set message "diff was not found in your path!" }

	popmsg $message info "About Diff"
}

###############################################################################
# Throw up an "about" window.
###############################################################################
proc about-TkD {} {
	global g

	set title "About $g(name)"
	set text {
<hdr>$g(name) $g(version)</hdr>

<itl>$g(name)</itl> is a Tcl/Tk front-end to <itl>diff</itl>\
	  for Unix-like & Windows platforms, and is originally
Copyright (C) 1994-2006 by John M. Klassa, among others:
Copyright (C) 1998		by Bryan Oakley
Copyright (C) 1999-2001 by AccuRev Inc.
Copyright (C) 2004		by Tom Dunne
Copyright (C) 2004-2025 by Dorothy Robinson ("dorothyr")
Copyright (C) 2017-2025 by Michael-M ("vampm")

Many of the toolbar icons were created by Dean S. Jones and used with his\
	  permission. These icons have the following Copyright:

Copyright(C) 1998 by Dean S. Jones
dean@gallant.com
http://www.gallant.com/icons.htm
http://www.javalobby.org/jfa/projects/icons/

Others not coverred by the above are the work of Michael-M

<bld>This program is free software; you can redistribute it and/or modify it\
	  under the terms of the GNU General Public License as published by the\
	  Free Software Foundation; either version 2 of the License, or (at your\
	  option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT\
	  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or\
	  FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU General Public License\
	  for more details.

You should have received a copy of the GNU General Public License along with\
	  this program; if not, write to the Free Software Foundation, Inc., 59\
	  Temple Place, Suite 330, Boston, MA 02111-1307 USA</bld>
	}

	set text [subst -nobackslashes -nocommands $text]
	do-text-info .about $title $text
}

###############################################################################
# Throw up a "command line usage and concepts" window.
###############################################################################
proc help-concept {mode} {
	global g pref

	set usage {
	$g(name) $g(version) may be started in any of the following forms(1-4):
		(Note that a FSPEC is either a file, directory or a Subversion-
		 style URL; optional parameters are documented here in [brackets])

	(1) Interactive selection of files to compare:
		tkdiff

	(2) Plain files:
		tkdiff FSPEC1 FSPEC2

	(3) Plain file containing conflict markers:
		tkdiff -conflict FILE

	(4) Source control:
			(any of: AccuRev, BitKeeper, ClearCase, CVS, Git, Mercurial,
	  Perforce, PVCS, RCS, SCCS, Subversion, Vpath)

		tkdiff	-rREV1 [-rREV2]	 FSPEC1 [FSPEC2]
		tkdiff [-rREV1 [-rREV2]]			(Search: CVS, Git or Subversion)
		tkdiff OLD-URL[@OLDREV] NEW-URL[@NEWREV]  (Subversion)

	Additional optional parameters:
		-a ANCESTORFILE
		-@ REV		  (of Ancestorfile - if coming from Source control)
		-o MERGEOUTPUTFILE
		-L LEFT_FILE_LABEL [-L RIGHT_FILE_LABEL]
		-P PreferenceFilename		  (only when configured)
		-I RegularExpression		   (ignore matched-lines)
		-B						(ignore empty-lines)
		-R						(recursive Directory Tree searching)
		-1,-2				   (preferred default merge side)
		-d						(debugging output)
	}

	set text {
<hdr>Description</hdr>
			  (any references to <btn>GUI</btn> elements will be as shown here)

$g(name) on its surface, appears as a tool to display and draw ones attention\
	  to modifications that have apparently occurred over some period of time\
	  by comparing two "states" of what began as an ORIGINAL text file. These\
	  modifications can be thought of as "differences" between any two data\
	  "states" of that file. But $g(name) as a TOOL, is about more than just\
	  presenting this picture of a files evolution over time. As a TOOL, it\
	  also is designed to help create yet MORE variations of a file, by\
	  selectively CHOOSING to "keep" some differences (or 'diff's), while\
	  simultaneously NOT "keeping" others. This is called "Merging" and is\
	  actually $g(name)s primary purpose.

Classically speaking, a 'diff' is a <itl>directed</itl> comparison of two text\
	  files that describes what would need to be changed to convert the first\
	  such file content into the second. $g(name) therefore groups its\
	  parameters as specified into a "Left" and "Right" pairing based on their\
	  <itl>repetition</itl> on the command line. Thus the first\
	  <cmp>FSPEC</cmp> encountered is <bld>usually</bld> the "Left" and the\
	  next would be the "Right". Revision specifications work similarly.
	  However, $g(name) often <itl>infers</itl> an argument (be it\
	  'Filespec' or 'Revision') to satisfy the need for TWO items to compare.\
	  Some inferences are simple, such as when one <cmp>FSPEC</cmp> is a\
	  FILE, and the other is a DIRECTORY; it infers the same NAMED file from\
	  the directory as the FILE that was already specified. Similarly, if no\
	  second <cmp>FSPEC</cmp> is provided, $g(name) will attempt to access\
	  a <bld>S</bld>ource <bld>C</bld>ode <bld>M</bld>anagement system (SCM:\
	  see below) to provide the missing item, but in this particular case, it\
	  will <itl>ALSO</itl> force such item to be the "Left", or first,\
	  element of the comparison. Beyond this <itl>pairing</itl> convention,\
	  each parameter is independent of others on the command line. Ultimately,\
	  all "Left" args are collectively used to specify the item(s) to compare\
	  to item(s) collectively formed by "Right" args. Unfortunately, this\
	  "pairing" technique can be up-ended somewhat when using <bld>URL</bld>s\
	  because of their ability to specify not only a FSPEC, but also a REV\
	  simultaneously. $g(name) treats a FSPEC at a slightly higher\
	  precedence than a REV when parsing the arguments and it absolutely will\
	  <bld>not</bld> <itl>fracture</itl> a URL@REV which may make it\
	  challenging to anticipate which entity specified will end up as "Left".\
	  This can be mitigated somewhat by not <itl>using</itl> the "@rev" syntax\
	  offerred by a URL, and simply specifying it individually, however,\
	  formulating the command line parameters is, as always, up to you.

In the first form(1), $g(name) will <itl>almost always</itl> present a dialog\
	  to allow you to choose the items to diff (subject only to a preference\
	  setting '<itl>$pref(autoSrch)</itl>' described in the <btn>Help</btn>\
	  menu "<btn>On Preferences</btn>" page).
	  This dialog, known as the <btn>New...</btn> Diff dialog, provides an\
	  interactive means of specifying the majority of command line parameters.\
	  Because the command line <bld>appears</bld> to offer such things as\
	  "tilde" and/or "glob" notations, AND furthermore owing to the extremely\
	  interactive nature of this Dialog, it warrants its <bld>own</bld>\
	  detailed description which can be found within the <btn>Help</btn> menu\
	  topic <btn>On GUI</btn> under the heading\
	  "<bld>The New Diff Dialog</bld>". For now, just know it not only provides\
	  for simple parameter data entry, but constantly analyzes what such data\
	  <itl>implys</itl> to the remainder of $g(name) INCLUDING which of the\
	  four forms shown above will be honored!

In the second form(2), either or both <cmp>FSPEC</cmp>s may be to a local\
	  file or directory, or symbolic links to such. When a directory is\
	  involved, only its contained FILES sharing a common name will be paired\
	  together, one from each originally given <cmp>FSPEC</cmp>. Note that\
	  this <itl>CAN</itl> produce multiple pairs of files to be Diff'ed (if\
	  both were directories). $g(name) remembers all of them, and permits\
	  switching among them later.
	  Generally, only the files of the <itl>given</itl> directory are\
	  considered as possible candidates, depending on the value of the\
	  <itl>other</itl> <cmp>FSPEC</cmp>: when that is a <cmp>FILE</cmp>,\
	  only a single comparison will result. <itl>Yet</itl>, when trying to\
	  use a URL in this form (versioned or not), be advised that the OTHER\
	  filespec will <bld>not</bld> be accepted as a <itl>directory</itl> unless\
	  such directory is KNOWN to Subversion, (ie. the Working Copy).\
	  See form(4) below for more details.
	  However, when both <cmp>FSPEC</cmp>s (or a singleton, which would\
	  technically be a form(4) occurrence), is/are <bld>real</bld> directories,\
	  another possibility occurs, as to whether said directory(s) should be\
	  searched <itl>recursively</itl> or not, producing a TREE of FILE\
	  candidates. From the command line, this distinction is answered by the\
	  presence (or lack thereof) of the option '<bld>-R</bld>', but is also\
	  <itl>dependent</itl> on a viable preference setting\
	  ("<itl>$pref(egnSrchCmd)</itl>") having been established. See the\
	  <btn>Help</btn> topic <btn>On Preferences</btn> for further details.

In the third form(3), a single <cmp>FILE</cmp> containing "conflict markers"\
	  will be split into two (or three) temporary files and used as ordinary\
	  input by $g(name). Such files can be generated by external tools such as\
	  "<cmp>merge</cmp>", "<cmp>cvs</cmp>", "<cmp>vmrg</cmp>", or even\
	  "<cmp>diff3 -m</cmp>" and perhaps others. Note that the\
	  '<bld>-conflict</bld>' flag is also available on the dialog (via the\
	  hidden grouping), but the corresponding <cmp>FSPEC1</cmp>\
	  <itl>must</itl> then also <bld>be</bld> an actual <cmp>FILE</cmp>, or\
	  the setting will be ignored. Note that if the conflict file CONTAINS\
	  appropriate internal "markers" that indicate a THIRD file was involved,\
	  $g(name) <itl>will</itl> configure itself to process in what is called,\
	  <bld>3-Way Diff mode</bld>, using the third derived file as an Ancestor.

The fourth form(4) is conditional on $g(name) being able to detect a viable SCM\
	  system (see below). It can further depend on the presence (or perceived\
	  NEED) of a Revision value - to <itl>distinguish</itl> it from a form(2)\
	  request! However, make note that if it <itl>DOES</itl>, find a SCM, it\
	  <bld>may</bld> effectively override the first form as described earlier\
	  (i.e. interactive startup). Presently only "CVS", "Git" or "Subversion"\
	  SCM systems will behave this way, when invoked with no arguments as is\
	  suggested in this form(4) as syntactically possible. The determining\
	  factor (as to accessing the interactive dialog or not) is controlled by\
	  the preference setting '<itl>$pref(autoSrch)</itl>', more fully\
	  described in the <btn>Help</btn> topic <btn>On Preferences</btn> page.
	  Remember that besides the explicit specification of <cmp>REV</cmp>\
	  arguments, $g(name) interprets a LACK of TWO <cmp>Fspec</cmp>s as an\
	  <itl>implicit</itl> request for additional assistance from an SCM. Under\
	  certain circumstances, this presumption might <itl>not</itl> be what was\
	  INTENDED, leading to complaints of "insufficient input", where the REAL\
	  issue is that $g(name) has detected a <itl>potential</itl> SCM which\
	  then FAILED to locate a tracked file, where what was DESIRED was to\
	  treat the <cmp>FSPEC</cmp>s as just FILES (or DIRECTORIES) which in the\
	  command line circumstance, can only be accomplished by adjusting the\
	  setting "<itl>$pref(scmPrefer)</itl>" to tell $g(name) to ignore the\
	  detected SCM, OR using the <btn>New...</btn> dialog to prefer the pseudo\
	  SCM setting "None". See the section "Source Code Management" below\
	  concerning further details on how SCMs are detected.

<hdr>User Preferences</hdr>
Practically everything in $g(name) is customizable and as such, those settings\
	  are paramount to making the most of what can be done with its usage.\
	  While $g(name) comes reasonably configured out of the box (specifically\
	  for the "Diff" engine), personalization to your own taste, or even to a\
	  given Projects expectations are all reasons why just ONE set of\
	  customizations may be insufficient.
	  You might wish to review the details (in the <btn>Help</btn> topic\
	  <btn>On Preferences</btn> page) of how to adjust your runtime environment\
	  to take advantage of the capability to have MORE than just a\
	  <bld>single</bld> personal Preference file. Note that as delivered,\
	  $g(name) does <bld>not</bld> provide this feature, nor its companion\
	  command line option ("<bld>-P</bld>"), until the specific configuration\
	  action is taken by the user, as described on that page.

<hdr>Source Code Management</hdr>
In all the SCM forms, $g(name) will detect which SCM system(s) are possible.\
	  This detection supports RCS, CVS and SCCS by looking for a directory\
	  with the same name, although RCS can also be detected via its ",v" file\
	  naming suffix convention. It detects and supports PVCS by looking for a\
	  vcs.cfg file. It detects and supports AccuRev, Perforce, ClearCase and\
	  Vpath by looking for the environment variables named ACCUREV_BIN,\
	  P4CLIENT, CLEARCASE_ROOT and VPATH respectively. It detects Git by\
	  looking for a .git directory, but will only work when started from within\
	  a Git work-tree. Similarly, Subversion looks for a .svn directory, except\
	  when using URLs, expecting any <cmp>FSPEC</cmp> to reside within a\
	  recognized "<bld>W</bld>orking <bld>C</bld>opy" (WC). Mercurial is\
	  supported by looking for a directory named ".hg" in the\
	  <cmp>FSPEC</cmp> directory or any of its ancestor directories, which\
	  is also how .svn and (for the most part) .git are searched.
	  It is important to recognize that several detections are based on the\
	  provided <cmp>FSPEC</cmp>(s), or alternately the "<bld>C</bld>urrent\
	  <bld>W</bld>orking <bld>D</bld>irectory" (CWD) where $g(name) was\
	  invoked, and at MOST times, BOTH! Often this can necessitate invoking\
	  $g(name) from <itl>within</itl> the "Sandbox" (a synonym for "WC")\
	  which are the actual files and directories that the specific SCM is\
	  actively tracking. This implicit use of the CWD is often instrumental in\
	  making a given SCM interaction not only detectable, but functional.
	  On the other hand, often depending on the specific SCM involved, this\
	  tendency to detect an SCM from its FSPEC pathname, or the CWD can IMPLY a\
	  SCM where one was not WANTED. This is why the <itl>$pref(scmPrefer)</itl>\
	  preference provides a special value of "<bld>None</bld>"; effectively\
	  preventing the detection when it was not wanted in the first place. Just\
	  know that this flag is available as <itl>either</itl> an actual\
	  preference (used every time) or as a ONE-TIME override (via its selection\
	  from the <btn>New...</btn> dialog). In practice, the dialog approach is\
	  often the simpler mechanism to use.

<cmp>REV1</cmp> and <cmp>REV2</cmp>, when given, must be a valid revision value\
	  for <cmp>FSPEC</cmp>. When the SCM system (RCS, CVS, etc.) is detected\
	  (see above), but no revision number is given, <cmp>FSPEC</cmp> is\
	  compared with the "<itl>default</itl>" revision (as defined by the\
	  specific SCM); often the most recently checked in. Again, multiple\
	  pairings may still be possible, if <cmp>FSPEC</cmp> was specified as\
	  a directory; where each would then attempt to use the <bld>same</bld>\
	  revision. For some SCMs, (those that expect every file to have its OWN\
	  revision, eg. SCCS or RCS) this can be problematic, unless the given\
	  revision format were to be something interprettable by that SCM as\
	  universally applicable, such as a "date".

Revision values are generally peculiar to a specific SCM. For example, a Git\
	  <cmp>REV</cmp> (see manpage for  git-rev-parse) offers several unusual\
	  variations:
	 <cmp>FILE</cmp>				 [compare with <cmp>HEAD</cmp> by default]
  -r <cmp>HEAD</cmp> <cmp>FILE</cmp>		[compare with <cmp>HEAD</cmp>]
  -r <cmp>HEAD^</cmp> <cmp>FILE</cmp>	   [compare with parent of <cmp>HEAD</cmp>]
  -r <cmp>HEAD~5</cmp> <cmp>FILE</cmp>	  [compare with 5th parent of <cmp>HEAD</cmp>]
  -r <cmp>HEAD~20</cmp> -r <cmp>HEAD^</cmp> <cmp>FILE</cmp>	  [compare 20th parent and parent of <cmp>HEAD</cmp>]
  -r 29329e <cmp>FILE</cmp>	   [compare with commit 29329e (full/partial SHA1)]
  -r v1.2.3 <cmp>FILE</cmp>		 [compare with tag (UNTESTED)]
$g(name) does not, itself, do anything with the value other than pass it along.

Because there are two <cmp>FSPEC</cmp>s there can also be potentially two\
	  distinct SCM systems. Although most people will only ever need to\
	  deal with one SCM system for a given situation, there <itl>IS</itl>\
	  an unusual arrangement possible that offers a distinct advantage.
	  For example, presume you use Subversion, or any other network-based\
	  repository, but will be unable to assure a viable network connection\
	  for some stretch of time. One solution would be to create a RCS\
	  subdirectory and post local modifications to it (avoiding the network)\
	  until such time as you can once again contact the server, at which point\
	  you can reinstate whichever RCS version you wish to send to Subversion\
	  using $g(name) to confirm exactly which changes you want current.
	  Even with just a single SCM system, you may have multiple WCs,\
	  representing perhaps different branches of the same code, and wish to\
	  fabricate a merged file as a hybrid of the two versions. $g(name) can\
	  address BOTH simultaneously, thus allowing the hybrid to be constructed.

<hdr>Special SCM circumstances</hdr>
While most SCM systems are generally some form of database with some method of\
	  designating unique "revision" identifiers, one, "Vpath", is really nothing\
	  more than a structured set of directories whose names are given as a list\
	  within their similarly named environment variable (VPATH). As such, its\
	  revisions are <itl>implied</itl> by the linear position each directory\
	  occupys. Each such directory in the list represents an <bld>earlier</bld>\
	  revision of whatever files it contains, thus when $g(name) needs an\
	  additional 'version' of some <cmp>Fspec</cmp> to create a pairing, it\
	  searches down the list finding the first-matched filename as the needed\
	  "predecessor" file. Some people like to think of this as a 3-dimensional\
	  filesystem, but its most important properties are that the topmost\
	  directory (under which $g(name) is invoked) is what other SCMs would\
	  consider its Sandbox, and EACH directory need only have the files that\
	  are unique (ie. changed). Thus each directory represents a "changeset"\
	  across all the files it specifies collectively. This particular form of\
	  <cmp>REV</cmp>s are simply inherent in the "Vpath" implementation, and\
	  are <itl>never</itl> actually specifed as arguments. However, the CWD\
	  can interact with a stated VPATH in two important ways:
	  1. when the CWD matches a Vpath node OTHER than the topmost; and
	  2. when the CWD specifies a Subdirectory of the matched Vpath node.
In the first case, any Nodes SKIPPED are treated as-if they were never\
	  mentioned at all. This enables reaching DEEPER into the version history\
	  by "top-pruning" the latest nodes. In the second, it establishes the\
	  named Subdirectory as the subset of versioned files of interest. Fspecs\
	  specified as outside BOTH adjusted containments are treated as NOT\
	  within the VPATH, and thus ineligible to be "found" as a pairing, despite\
	  being readily available as just a "plain file" Fspec.

Additionally, some SCM systems provide abilities that can identify which of\
	  their files are <itl>different</itl> given <itl>only</itl> some set of\
	  desired revisions (including defaulted). $g(name), using its capacity\
	  for accepting multiple pairings, will attempt to access those utilities\
	  it knows of to obtain such as an input source. We call this the "search"\
	  or "inquiry" mode. SCM systems lacking this ability will simply reject\
	  any provided Revision arguments as inadequate with an error message.
	  Note that given the <bld>lack</bld> of a <cmp>FSPEC</cmp> in this\
	  instance, such SCM utilities basically expect the CWD (of, in this case,\
	  $g(name)) to <bld>be</bld> within the "WC" of the files they manage.\
	  Thus the directory where you invoke $g(name) again plays a role, but\
	  was likely instrumental in getting that SCM to be detected in the first\
	  place, so should not be an issue.
	  Accordingly, if <itl>fewer</itl> than two revisions are given\
	  <bld>and</bld> the SCM can accomodate it, inferred revisions will be\
	  supplied (generally the latest or HEAD, or similar). However, note that\
	  the <bld>Git</bld> SCM has an unusual arrangement in that an intermediate\
	  UNNAMED revision (referred to as the 'stage' or 'index') sits between\
	  the working copy and the last commit; $g(name) will allow you to specify\
	  this quasi-revision using a revision value of " " (blank) either on the\
	  command line or in the GUI dialog. Remember that on the command line this\
	  will require quoting (to be parsed correctly), the simplest being "-r	 ".\
	  For the dialog, the field <bld>label</bld> for the revision will be\
	  dimmed when it is EMPTY, as it would otherwise be difficult to actually\
	  SEE a legally enterred blank.
	  Finally, when "inquiry" mode is active, it is entirely possible that one\
	  such detected difference <itl>might</itl> be a "conflict"ed file, as is\
	  often created when a prior SCM merge request was not fully completed.\
	  $g(name) will accept it as such, even when it is one of multiple files\
	  being provided, and automatically switch into "conflict" or even "3-Way"\
	  mode when later processing that particular entry, as needed.

One last point about SCM systems - as described earlier, it should be apparent\
	  that owing to the many varied ways any one system <itl>might</itl> be\
	  detected, $g(name) needs to try ALL of them, albeit in some unseen order.\
	  Because $g(name) has existed for decades, often longer than many of the\
	  SCM systems themselves, that <bld>ORDER</bld> is more a case of historical\
	  randomness we mustn't change (to preserve existing usage), than of a\
	  well-reasoned rational choice.
	  As delivered, $g(name) defaults the preference\
	  "<itl>$pref(scmPrefer)</itl>" to the value "Auto Auto", which <itl>will\
	  choose</itl> the <bld>first</bld> such SCM to sucessfully be detected for\
	  each of the two respective Text windows. You may need to tune one or\
	  both settings to the SCM you <itl>Prefer</itl> per each window if more\
	  than one system is detectable in the environment you run within, and the\
	  one you want, doesn't happen to come out as "first".\
	  For the record, this internal precedence ORDER is presently:
	  CVS
	  SVN
	  GIT
	  BK
	  SCCS
	  RCS
	  PVCS
	  Perforce
	  Accurev
	  Clearcase
	  HG
	  Vpath
with only <itl>detected</itl> systems participating. This admittedly arbitrary\
	  looking order MAY have had a reason when each was added to $g(name), but\
	  if so, the explanation was never noted, and is thus lost to history.

<hdr>A quick word about quoting</hdr>
Most command environments, a Unix/Linux Shell for example, offer multiple\
	  means of quoting (such as single or double quote characters). As a\
	  <itl>general</itl> rule, any $g(name) option flag that takes a value\
	  (such as a <cmp>REV</cmp> or others) may be specified as directly\
	  prefixed to that value, or separated by "white space" (blanks, tabs,\
	  etc.). However you must <itl>not</itl> try to <bld>pack</bld> multiple\
	  $g(name) <bld>flags</bld> into a single parameter as they will not be\
	  recognized as such by $g(name), and would thus likely be passed directly\
	  to "Diff" or whatever other differencing engine has been configured,\
	  as is. See the section "The Diff engine" below for further specific rules.

<hdr>Limitations of URLs</hdr>
Besides Subversion being the only SCM to "define" the usage (and syntax) of\
	  URLs for accessing the remote repository, there are other issues their\
	  existance causes to the general semantics of $g(name) insofar as their\
	  tacit use as a FSPEC. First is their ability to additionally specify\
	  a revision. $g(name) will ensure that the revision STAYS with the URL,\
	  even if it means jumbling the apparent command line order of arguments\
	  and what that might mean to which entity ends up as 'Left' .vs. 'Right'.\
	  Separating these aspects ON THE COMMAND LINE, may make predictability\
	  of what ends up where easier for experienced users. Lastly, because\
	  $g(name) has no ability (at present) to determine exactly WHAT the URL\
	  may point at, trying to treat a URL as naming a directory (to take\
	  advantage of pairs generation, etc.), EVEN IF IT ACTUALLY DOES, will\
	  not be honored as $g(name) expects the URL to name a FILE! Perhaps this\
	  may be addressed in the future, but for now - thems the rules!
	  One final point about the Subversion URL: while there are multiple forms\
	  of Revisions (an ever increasing integer value, various keywords (HEAD,\
	  BASE, PREVIOUS and COMMITTED) and even a DATE format, only HEAD appears\
	  to be <bld>valid</bld> when using a URL! Accordingly, while filename\
	  based access will <itl>default</itl> to using <bld>BASE</bld>, when a\
	  URL is used, that <itl>default</itl> will become <bld>HEAD</bld>! Expect\
	  to see access failures using other Revison forms WITH a URL specification!

<hdr>Requesting a 3-Way diff</hdr>
A "3-Way" diff is most often used for merging a file that different people\
	  may have worked on both <itl>independently</itl> and\
	  <itl>simultaneously</itl>, <bld>back</bld> into a single file. Just as\
	  files for comparison are designated with some combination of a FSPEC\
	  (and possible REV value), an ANCESTORFILE may be specified as a\
	  <itl>third</itl> file using the "<bld>-a</bld>" option to designate it.
	  To be useful, this file should be a version that <itl>closely</itl>\
	  predates BOTH versions being compared/merged. If using an SCM to track\
	  past versions, also specifying the "<bld>-@</bld>" option will provide\
	  the necessary REV value to obtain the proper file. Note however, that\
	  should $g(name) detect that the Ancestor file happens to be\
	  <bld>identical</bld> to <itl>EITHER</itl> of the other two, "3-Way"\
	  mode will be discontinued and instead treated as a simple two-file\
	  comparison. A notification of this happening <itl>will</itl> be provided.

<hdr>Additional hints</hdr>
With regard to inferred SCM revision fields, invoking $g(name) with no viable\
	  arguments at all <itl>MAY</itl> result in <itl>either</itl> an SCM\
	  trying to supply such args <bld>OR</bld> presenting the interactive\
	  dialog. However, when an SCM <itl>is</itl> detected and searched, but\
	  results in not <itl>finding</itl> any files to compare, often only a\
	  termination message will be produced. This is even MORE likely if\
	  $g(name) was started WITH ARGUMENTS from the command line; whereas if it\
	  was initiated via the interactive <btn>New...</btn> dialog, $g(name)\
	  will issue the message and then TRY to return to the dialog to perhaps\
	  <itl>adjust</itl> the settings to values that DO function.

It is <itl>NOT</itl> recommended to <bld>specify</bld> an\
	  <cmp>ANCESTORFILE</cmp>, <cmp>MERGEOUTPUTFILE</cmp> or more than two\
	  "<cmp>-L File-label</cmp>" options when using any form that will resolve\
	  to more than a <itl>single</itl> diff pair (i.e. generally when a\
	  directory <cmp>FSPEC</cmp> is paired against anything but a\
	  <cmp>FSPEC</cmp> that is a single <cmp>FILE</cmp>). It will likely\
	  produce <itl>undesired</itl> results, an example of which is outlined as\
	  follows:
	  When the merge output filename is not specified, $g(name) will present a\
	  dialog to allow you to choose a name for that file when attempting to\
	  write it. This is actually the simplest method of operation. If you\
	  <itl>do</itl> choose to provide a name (via the command line\
	  <itl>or</itl> the <btn>New...</btn> Diff dialog window) $g(name) will\
	  <bld>try</bld> to honor it. But there is a strong possibility you may\
	  be asked to reconfirm that name <itl>OR</itl> be presented with an\
	  entirely new name when you attempt to write to it. This generally occurs\
	  when $g(name) detects that multiple file pairs are in use, which would\
	  result in cross associating the single given merge output name to an\
	  indeterminate file pairing. Thus $g(name) then reverts to "suggesting"\
	  its own name. Of course, you may <itl>at that point</itl> then choose\
	  whatever filename you wish.
	  In a similar fashion, many of the "Additional optional parameters"\
	  shown are intended for use when $g(name) is invoked to process a\
	  <itl>SINGLE</itl> file pair, as was its original historical heritage.

As a further note regarding the $g(name) "<itl>suggested</itl>" merge file\
	  output names, be advised that $g(name) will try to fabricate a name that\
	  derives from the filename used in the Left window, unless that file\
	  itself derives from an SCM system, in which case it will try to choose\
	  its name from that of the Right window. When BOTH windows represent SCM\
	  files, it will aim for the current directory that $g(name) was invoked\
	  from, but the default name chosen would then be based from a fairly\
	  cryptic tempfile name which almost certainly will need renaming.\
	  Regardless of the <itl>default</itl> name presented, you may, of\
	  course, place the output in any filename you designate.

The remaining options perform the following services:
	  Both <cmp>-B</cmp> and <cmp>-I RegularExpression</cmp> are intended to\
	  suppress differences from EMPTY or RE-matched lines respectively, and\
	  you may specify the "<cmp>-I</cmp>" option more than once. Each operates\
	  as described by the GNU Diff documentation, but are part of $g(name)\
	  itself and NEITHER is passed to the Diff engine (thus making them usable\
	  by <itl>any</itl> such engine).
	  The mutually exclusive options <cmp>-1</cmp> and <cmp>-2</cmp> allow\
	  one to suggest to $g(name) which side (Left or Right, respectively),\
	  should be chosen <itl>during initial read-in</itl> as the contributing\
	  merge side for any diff region for which $g(name) cannot discern a reason\
	  (such as in a 3way ancestor-file situation) to choose one versus the\
	  other. Oftentimes the intent of the merge (back porting, etc.) and\
	  the order of files on the command line can dictate which file should\
	  be treated as the contributing "source" for the eventual merge outputs,\
	  such that the <bld>majority</bld> of merge choices will be "pre-selected"\
	  for you. In fact, it is possible to <itl>toggle</itl> this value\
	  <itl>after-the-fact</itl> if you find that the left-to-right order of the\
	  files did <bld>not</bld> turn out as you expected; simply invoke\
	  <btn>New...</btn> Diff, flip the setting and allow $g(name) to redo that\
	  difference computation, then proceed to make the fewer number of needed\
	  merge choice assignments. Debug output (<cmp>-d</cmp>), while not really\
	  meant for the average user, is simply mentioned here for completeness\
	  sake. When used, it produces somewhat cryptic textual output via the\
	  STDERR output stream showing significant status and/or mileposts with\
	  varying degrees of usefullness to those familiar with $g(name) internals.

<hdr>Network latency</hdr>
$g(name) does not, itself, require network access to run. However, certain\
	  SCM systems are based on such technology and can thus introduce delays\
	  in the processing performed by $g(name). In fact, a network\
	  <itl>outage</itl> could even hang $g(name) while waiting for a response.
	  To help combat that possibility,\
	  <itl>particularly at tool startup</itl>, $g(name) <itl>may</itl> alert\
	  you that such a delay appears to be occurring. A popup status panel, in\
	  advance of the main $g(name) display, will present messages of\
	  activities occurring. As long as new activities continue to occur\
	  (perhaps every few seconds), no action is needed on your part.
	  However, be advised that should the messages stall and you attempt to\
	  <bld>dismiss</bld> this panel, you are, in fact, requesting that\
	  $g(name) <itl>ABORT</itl> completely.
	  This feedback mechanism is intended to simply provide you with interim\
	  updates until sufficient information can be obtained to present the main\
	  display. Under normal conditions, no such messages are needed nor\
	  produced, and $g(name) <itl>will remove</itl> the status display itself\
	  when the main display is ready.

<hdr>The Diff engine (and more quoting)</hdr>
Although $g(name) was designed as a frontend to the classic, UNIX derived,\
	  "diff" command, there is no specific reason some other utility meeting\
	  its input/output and invocation requirements cannot be used. Because of\
	  this, $g(name) can be configured to interoperate with other differencing\
	  engines, some having perhaps more advanced (or desireable) detection\
	  methods. Prior to $g(name) V5.5, the primary requirement for such engines\
	  was it must produce what GNU Diff called "<bld>normal</bld>" diff output\
	  format (NOT "Unified" NOR "Context"). This is <bld>NO LONGER TRUE</bld>.\
	  $g(name) now accepts <bld>Unified</bld> format automatically with no\
	  further user action required ("Context" remains unsupported).
	  With regard to telling the Engine what optional features it should\
	  perform, $g(name) always communicated these via "comand line options"\
	  and still does, although it has become more formalized via an Engine\
	  configuration section added to, and described in, the\
	  <btn>On Preferences</btn> topic of <btn>Help</btn>. This should be\
	  consulted for details when deciding to use an Engine OTHER than the\
	  default UNIX-derivative "DIFF" command.
	  Because this "option flag" method is ALSO how one invokes $g(name), it\
	  is POSSIBLE for options intended for the Engine to be supplied via the\
	  $g(name) command line as a simple way to augment the operation of the\
	  current $g(name) session. The only requirements are it needs to be a flag\
	  that $g(name) does not, itself, recognize AND must parse as a SINGLE-WORD!
	  Accordingly, any option flag having a leading dash that is <itl>not</itl>\
	  recognized as a $g(name) option is passed <itl>virtually untouched</itl>\
	  to your Diff engine of choice. This "pass through" feature permits you to\
	  <itl>temporarily</itl> alter the way the Diff engine is called,\
	  <itl>for the entire session</itl>, without resorting to a change in your\
	  preferences file or settings. However it also means that to use, for\
	  example, the "-d" option for GNU diff (which <itl>is</itl> a $g(name)\
	  option), you would need to pass it using its long-form equivalent of\
	  "--minimal" (to avoid the mis-interpretation).
	  Just be aware, that should you make preference changes that ALTER the\
	  use of ANY flags currently being passed from $g(name) TO the Engine,\
	  this special "pass thru" option will be <bld>removed</bld> and no longer\
	  in force, UNLESS specifically re-enterred via the preferences dialog.\
	  The best way to think of this, is to believe that specifying a random\
	  option from the $g(name) command line is EQUIVALENT to having added it\
	  to the current session VIA the preference dialog - and thus any FURTHER\
	  preference changes (related to the options PASSED to Diff) is simply\
	  responding to your new changes (which never knew the temporary option\
	  was ever present).
	  BUT as a related issue, trying to pass <itl>any</itl> option that\
	  requires a <bld>value</bld> <itl>MAY</itl> necessitate an unusual form\
	  of quoting to preserve syntactically required "white space" characters.\
	  As noted earlier, if the value portion has NO blanks and is permitted to\
	  be physically attached to its option flag, no special action is required.
	  But if the value itself requires a "space" -or- must be SEPARATED from\
	  its option flag by one (to satisfy the parsing rules of the Diff engine),\
	  then you should pass the <itl>entire construct</itl> within double quote\
	  characters, and perhaps even <itl>doubly</itl> so. As an example,\
	  GNU diff has an option whose syntax is:
	  <bld>-I, --ignore-matching-lines=RE</bld>
Admittedly, this option <itl>is</itl> a $g(name) recognized option (at least\
	  via the "-I" flag) and thus would not be passed on to the engine, but it\
	  can illustrate the issues involved. Thus if something similar\
	  <itl>WERE</itl> to be passed to tell Diff (in this example) to not\
	  consider any line that starts with a octathorp (#) followed by a space,\
	  you might think to specify this to $g(name) as <itl>either</itl> :
	  "--ignore-matching-lines=^#  "   (or better "-I^#	 ")
or
	  <bld>"</bld>-I  <bld>\"</bld>^#  <bld>\" "</bld>	  (or "-I  {^#	}")
Note in each case, the quoting of BOTH the flag and its value\
	  <itl>together</itl>. But particularly, in the second form, note the extra\
	  quoting (done with escaped double quotes or "brace" characters as shown)\
	  surrounding the value as well; this demonstrates how to pass a flag that\
	  MUST be separated from the value it passes. Be aware that <itl>single\
	  quotes will NOT work</itl> here. The "brace" character is simply the\
	  lexical mechanism used by the internals of $g(name) to quote its content\
	  where potential "substitutions" are to be avoided. Without this 'extra'\
	  quoting as shown, $g(name) <itl>would pass</itl> the option, but the\
	  resulting Diff would <itl>MISS</itl> the trailing blank of the value.
	  The reason is primarily that $g(name) <itl>has no true syntactic\
	  understanding</itl> of nearly ANY flags being passed to the engine, nor\
	  if they might legitimately require a value, or need said value to be\
	  separated from its flag while still <itl>preserving</itl> any embedded\
	  blanks. Yet, certain flags ARE understood (notably the classic Engine\
	  options that deal with difference suppressions) and are provided for by\
	  the <btn>Engine</btn> configuration section of the preference settings,\
	  to address these specific syntactic concerns. Recommended reading!

	  IMPORTANT NOTE - from a $g(name) perspective, this example GNU Diff\
	  option should <bld>never</bld> be passed to <bld>ANY</bld> diff engine\
	  ... it was designed to make the <itl>direct output of Diff itself</itl>\
	  more meaningful to a <itl>human</itl> by simply suppressing what is,\
	  in reality, actual differences. Besides, the <itl>functionality</itl>\
	  can be PROVIDED by $g(name) itself (VIA the recognized "-I" flag).\
	  $g(name) <bld>will fail badly</bld> if you were to perhaps\
	  <itl>sneak</itl> THIS option to Diff (using its long-form flag name).\
	  You have been warned.
	}

	set usage [subst -nobackslashes -nocommands $usage]
	if {$mode == "cline"} { puts $usage
	} { append gui "<hdr>Command line</hdr>" $usage [subst -nobac -nocom $text]
		do-text-info .usage "$g(name) Concepts + Syntax" $gui
	}
}

###############################################################################
# Throw up a help window for the GUI.
###############################################################################
proc help-GUI {{mode {}}} {
	global g pref

	set DOLLAR {$} ;# TCL quoting trick embeds a literal dollar sign into text
	set title "How to use the $g(name) GUI"
	set topic1 {
	  (Disclaimer: Historically, $g(name) had provided support [meager though\
	  it was] for Monochrome displays. As of release V5.1, such support has\
	  been withdrawn entirely. While it is <itl>possible</itl> to re-instate\
	  it, the ready availability and/or cost of Color monitors makes doing so\
	  unlikely).

<hdr>Layout</hdr>
The top row contains the <btn>File</btn>, <btn>Edit</btn>, <btn>View</btn>,\
	  <btn>Mark</btn>, <btn>Merge</btn> and <btn>Help</btn> menus. Note that\
	  on some platforms, common practice is to relocate this menubar to a\
	  specific screen location, often the top of the display. This happens\
	  automatically on such platforms, and is not peculiar to, nor caused by\
	  $g(name). The second row is a toolbar having diff-region management,\
	  search, navigation and merge selection tools, each represented by either\
	  a symbolic image or just text to convey their specialty. Below that are\
	  labels which identify the contents for each of the two text windows that\
	  follow just below them. Note that these labels will <itl>also</itl>\
	  produce a "tooltip" popup (a brief description) showing the ACTUAL\
	  filename and its modification time when hovering over it with the mouse,\
	  <itl>provided</itl> it is not a tempfile (such as extracted from a SCM).
	  In addition, if an <cmp>ANCESTORFILE</cmp> was specified at startup, a\
	  third label (a small graphic denoting a text file labelled "A") will\
	  appear between the other two labels. It also will display a tooltip\
	  indicating its underlying name, and <itl>possibly</itl> its modification\
	  time, the latter based (again) on the file <itl>not</itl> having been\
	  extracted from an SCM or not. But in reality it is also a\
	  <bld>button</bld> that when pressed, will popup a <itl>display only</itl>\
	  presentation of that Ancestor file, for those who simply <itl>have</itl>\
	  to be able to see it.

To the left of EACH Text window is another (optional) narrow window of the\
	  same height, known as the INFO window, in which additional data\
	  ABOUT the individual text lines can be displayed, such as its line\
	  number and various markers or highlighting that $g(name) will provide\
	  based on yet further settings and/or its analysis. Toggling the\
	  <btn>$pref(showln)</btn> menu item of the <btn>View</btn> Menu controls\
	  if this window is present, as does changing its setting as a Preference.

The left-most text window displays the contents of <cmp>FILE1</cmp>, the most\
	  recently checked-in revision, <cmp>REV</cmp> or <cmp>REV1</cmp>,\
	  respectively (as per the startup options described in\
	  the "On Concepts+Syntax" help). The right-most window displays the\
	  contents of <cmp>FILE2</cmp>, <cmp>FILE</cmp> or <cmp>REV2</cmp>,\
	  respectively. Clicking the right mouse button over either of\
	  these windows will give you a context sensitive menu with actions that\
	  will act on the window you clicked over. For example, if you click\
	  right over the right hand window and select "Edit", the file displayed\
	  on the right hand side would be loaded into a text editor. Outboard of\
	  BOTH Text+INFO windows are conventional vertical scrollbars, with\
	  horizontal ones located below their respective Text windows, which\
	  can be operated independently or synchronously at the users preference.\
	  Between the two text windows is an <itl>optional</itl> display known as\
	  the "Diff Map" which serves not only as a graphical overview of\
	  <itl>where</itl> <bld>ALL</bld> the difference regions exist across the\
	  entire file, it will also behave as a conventional vertical scrollbar.\
	  The <btn>View</btn> menu item "<btn>Show Diff Map</btn>" specifies if\
	  this display element is actually shown onscreen or not, as you choose.\
	  And finally, located between the two horizontal scrollbars, is what is\
	  called a "<btn>grip</btn>" which can be dragged horizontally with the\
	  mouse to re-distribute the relative screen space allocated among the\
	  Left and Right windows themeselves.

Following all these <itl>MAY</itl> be an optional two line window called the\
	  "Line Comparison" window. This will show the "current line" from each of\
	  the Left and Right text windows, one on top of the other. This\
	  "current line" is defined as the line that shows the blinking "insertion"\
	  cursor, which can be set by merely clicking on any line in either text\
	  display and/or "driven about" utilizing numerous keyboard accelerators.\
	  The entire window may be hidden (or requested) by the <btn>View</btn>\
	  menu item "<btn>Show Line Comparison Window</btn>" being chosen, or not,\
	  as desired.

At the bottom of the main display is the Status bar, where tool activity and/or\
	  informational messages of various sorts will display from time to time.\
	  At the far right edge of this bar, are two dedicated displays; the first\
	  is a stylized count of the number of merge choices that presently select\
	  either the Left or Right version of each Diff Region. These values will\
	  be modifed as choices are made within the tool. As such, it serves to\
	  suggest how much MERGE work is at potential risk of loss, should a NEW\
	  execution of "Diff" take place.
	  The second is a COUNT of the regions that presently exist (and if any,\
	  which region is "current" - ie. the "<bld>CDR</bld>"). The SUM of the\
	  Left/Right merge choices at times <itl>may not</itl> equal the TOTAL of\
	  all regions, as some regions COULD be set to select BOTH sides, which is\
	  NOT counted. The merge counts are additionally a reminder of which side\
	  is providing the bulk of the merge selections, which might signal the\
	  need to toggle the "<itl>$pref(predomMrg)</itl>" preference, BEFORE\
	  expending the time to review and make individual selections.
	}
	set topic2 {
<hdr>The New Diff Dialog</hdr>

Beyond the main display window, there exists a Dialog window describing the\
	  input parameters $g(name) was provided, either via the command line,\
	  or by having this very window PRESENTED when $g(name) senses it lacks\
	  sufficient (or perhaps, unsuitable) input parameters. The Dialog itself\
	  is <bld>highly</bld> interactive where nearly every user manipulation\
	  results in the Dialog displaying or hiding elements, recalculating\
	  pertinent values, and even monitoring keystrokes and providing highlight\
	  <itl>feedback</itl> as to whether an in-progress file specification being\
	  typed is acceptable.
	  Because of its desire to <bld>emulate</bld> all inherent abilities of\
	  the (historical UNIX) command line, it also provides for such things as\
	  "tilde" and/or "glob" notation; basically typing shortcuts for specifying\
	  file-system based names, to avoid the drudgery of typing the WHOLE\
	  literal path. There are 3 defined '<bld>tilde</bld>' shortcuts that ALL\
	  work ONLY when they are the FIRST character of a file path:
			1. a SINGLE tilde (~) is replaced by the users HOME (aka login)\
	  directory
			2. a tilde-minus PAIR yeilds the most recent PRIOR current\
	  directory visited, and
			3. a tilde-word group considers <bld>word</bld> to be a UserID and\
	  supplies <itl>its</itl> HOME directory.
	  '<bld>Glob</bld>', on the other hand, is a PATTERN-match syntax where\
	  you can <itl>describe</itl> portions of the desired name, rather than\
	  TYPING them literally. Notably, an asterisk (*) stands for any number of\
	  characters (except a forward-slash), while a question mark (?) represents\
	  any <itl>single</itl> character. There are others such as\
	  <bld>[bracket]</bld> sets for a <itl>single</itl> SPECIFIED char from the\
	  enumerated set, but the trait they ALL share is they only combine (with\
	  other literal characters) <bld>within</bld> any single fragment of an\
	  overall path name, delimited by forward slashes.
	  Note that these "features" are a natural consequence of that (original)\
	  command line environment (commonly called '<itl>the SHELL</itl>') itself,\
	  but are so ingrained that NOT providing them via the Dialog would be\
	  perceived as a penalty.
	  Sadly, attempting to USE "Glob"ing WILL NEVER work when combined into a\
	  URL syntactic form, NOR can you "Glob" a UserID - but it wouldn't have\
	  worked from the SHELL command line either!\
	  Yet, the SHELL ability to expand (so-named) Environment Variables\
	  (eg. ${DOLLAR}HOME/.../...) has NOT been replicated for the Dialog.

While the command line uses repetition exclusively to distinguish "Left" and\
	  "Right" parameter instances, the dialog obviously expects such values to\
	  be <itl>filled</itl> in a similar "Left first" order; as a reminder,\
	  entering a value (such as a <cmp>FSPEC</cmp> or <cmp>REV</cmp>) into\
	  the "second position" while the "first" remains <itl>empty</itl> will\
	  result in a red warning highlight of the (possibly) mis-positioned value;\
	  you may, of course, enter values in any order you wish: it is, after all\
	  <itl>only</itl> a reminder, which will disappear when both are populated.
	  However, simply <itl>because</itl> you entered the value into the second\
	  position, will not ensure that it LOGICALLY <bld>remains</bld> there if\
	  you fail to populate <itl>ANYTHING</itl> into the first position. In such\
	  a case, $g(name) will interpret the given value as <itl>belonging</itl>\
	  to the <bld>first</bld> position and will apply its command line\
	  "pairing" rules accordingly. The same applies to Revision data enterred,\
	  although not when BOTH <cmp>FSPEC</cmp> fields have data present.
	  Another distinction of the dialog is its presentation and adjustability\
	  of the SCM that will be used <itl>if and when</itl> the command syntax\
	  requires one. Where the command line operates purely by preference\
	  settings, the dialog allows you to adjust the final interpretation\
	  within the bounds of all <itl>presently entered</itl> parameters, which\
	  is to say the dialog will continually readjust as values are enterred\
	  or removed.

In fact, even the simple act of <itl>TYPING</itl> into either FSpec (1 or 2)\
	  or Ancestor fields may cause an immediate highlight to occur on that\
	  fields LABEL! The <itl>intention</itl> of this highlight is to help you\
	  recognize <bld>when</bld> the name as currently shown, represents an\
	  ACTUAL filesystem-named entity, be that a FILE, DIRECTORY, or (to a limited\
	  degree), a URL!
	  The color of the highlight is patterned on a conventional stoplight\
	  paradigm, where YELLOW (really, whatever color was defined by the toolwide\
	  preference "<itl>$pref(inform)</itl>") signifies the name is NOT (yet)\
	  recognized as <bld>real</bld>: generally, you need to keep typing.\
	  <bld>But</bld> if it should be <itl>Red</itl>, continued typing MAY\
	  not help, as it signifies what is ALREADY entered can easily\
	  <bld>NOT</bld> produce a real entity name! You are thus reminded (via a\
	  popup message) to PERHAPS <itl>backspace</itl> and try again.
	  In general, a RED highlight means the problem stems from a\
	  <bld>tilde</bld> related issue (such as typing something that is NOT YET\
	  the intended UserID. You <itl>can</itl> continue typing to complete the\
	  name, as long as you <bld>don't</bld> type a fragment-delimiting\
	  <bld>slash</bld> character BEFORE completion. Doing so will result in the\
	  popup message re-appearing <itl>AND THE SLASH WILL NOT BE ACCEPTED</itl>!
	  This "<bld>hard</bld>" error is a recognition that IF the slash were\
	  allowed, the file name typed thus far can NEVER be found or used by\
	  $g(name), <itl>regardless</itl> of what might be typed afterward.
	  But other situations exist. For instance, because $g(name) permits 'Glob'\
	  syntax (described earlier), its possible you entered something that could\
	  actually match <itl>MULTIPLE</itl> <bld>real</bld> items AT THAT fragment\
	  level in the name, which is considered mildly <bld>illegal</bld>: your\
	  <itl>GOAL</itl> here is entering something that resolves to a SINGLE item\
	  name ONLY! Accordingly, trying to enter a forward-slash at such a point\
	  would <itl>again</itl> be treated as a <bld>hard</bld> error - but\
	  because its NOT tilde related, the highlight would only be YELLOW. Note\
	  however, that continued typing where earlier Glob-fragments MAY have been\
	  overly loose in their specification (eg. a single asterisk), the nature\
	  of evaluating an entire PATH of further fragments, can quickly narrow the\
	  scope of that earlier fragment DOWN to being a SINGLE possibility,\
	  provided the later fragments become more and more distinctive! And there\
	  is yet even MORE to consider.

In addition to both "Tilde" and/or "Glob" syntax, there is the extremely\
	  twisted notion of a <bld>REAL</bld> filename that BEGINS (within SOME\
	  directory fragment) with an <itl>ACTUAL</itl> <bld>literal tilde</bld>!
	  $g(name) <itl>WILL PERMIT</itl> direct access to such a name, from that\
	  current fragment, <bld>provided</bld> it does NOT actually\
	  <bld>duplicate</bld> an ACTUAL Userid (from which it would be\
	  indestinguishable)! $g(name) will <itl>ALWAYS</itl> prefer the "Tilde"\
	  interpretation (if it works) over that of a "plain old tilde-filename"!
	  There are various reasons for this choice, but the most compelling one is\
	  that OUTSIDE of $g(name), the SHELL environment would invariably make the\
	  same choice, making it exceptionally difficult to MANIPULATE such files\
	  until one realizes that it can be SUBVERTED by prefacing the name with a\
	  DOT-SLASH prefix (./) thereby causing the "tilde" to no longer\
	  <itl>BE</itl> the <bld>first</bld> character <itl>of the PATH</itl>.
	  While this SAME work-around IS VIABLE for $g(name) as well, in practice,\
	  remembering to ALWAYS protect such a filename becomes tedious, and thus\
	  $g(name) while allowing such names, would highly NOT recommended their\
	  use - and while $g(name) will often get it right, YOUR environment\
	  OUTSIDE of $g(name) can easily get it WRONG, ALWAYS believing it\
	  represents SOMEONES HOME directory. So why mention all this?
	  Because you may be a user who OPERATES from a command line! And while\
	  this Dialog will watch over what you type AS YOU TYPE IT, the command\
	  line and the SHELL running at that point, will DELIVER such names fully\
	  formed (admittedly having handled both Tilde and Glob itself, <itl>when\
	  it can</itl>); some may LEAVE unexpandble names intact and simply pass\
	  them <bld>as-is</bld>. But if $g(name) sees the name is\
	  <bld>unusable</bld>, IT will deliver you directly to this Dialog to FIX\
	  the problem, without you having TYPED anything! Thus where the dialog\
	  would have prevented you from compounding your errors with extra\
	  (useless) path fragments, the <bld>problem</bld> could NOW be as far\
	  back as a leading tilde. ONE WAY to dodge this issue <itl>could</itl> be\
	  to prefix the tilde with the dot-slash (./) as already described; but\
	  NOW that you are sitting IN the Dialog, your best action might be to\
	  just Backspace until the highlight DISAPPEARS -or- just USE one of the\
	  BROWSER options to rewrite the whole filename by navigating to and\
	  PICKING a replacement (as will be described shortly).

	  Lastly, there is no '<bld>green</bld>' in this semi-streetlight analogy;\
	  in its place, is merely NO highlight at all. This is what you are aiming\
	  for when typing, as trying to <btn>OK</btn> the Dialog with\
	  "<bld>invalid</bld>" names (as suggested by the highlights) will most\
	  often result in SOMETHING-<itl>not-found</itl> messages elsewhere in\
	  $g(name), and a good chance of finding yourself BACK in this Dialog!

	  One final detail, involving URLs, is that until the 'protocol'-delimiter\
	  (<bld>://</bld>) can be seen, $g(name) will THINK its watching a file\
	  name being typed with its YELLOW highlight just asking for more - but\
	  once seen, the highlight will <itl>disappear, SUGGESTING</itl> it thus\
	  '<itl>exists</itl>' - but that cant <bld>actually</bld> be determined:\
	  you simply <itl>MUST</itl> finish entering it correctly.

But even ALL THAT is not the end of the Dialogs perception of what you wish to\
	  accomplish. A heretofore <itl>hidden</itl> button, <btn>Recurse\
	  Directory</btn> can appear allowing you to choose if the currently\
	  enterred FileSpecs, where both (or only the first, with the second empty)\
	  are directories to specify if it/they should be searched\
	  <itl>recursively</itl> or not, producing a TREE of FILE candidates.\
	  Pressing it <itl>toggles</itl> it on (or off) with highlighting for\
	  feedback.
	  Note that if the equivalent command line option (<bld>-R</bld>) was\
	  ALREADY given, the button may appear as <itl>already activated</itl>.\
	  Yet, one last point - if the button displays as <bld>disabled</bld>\
	  (regardless of showing as <itl>activated</itl>) it will not function,\
	  and is simply reminding you that the NEEDED preference setting\
	  ("<itl>$pref(egnSrchCmd)</itl>") has not yet been established.

Still, one of the clear advantages of the dialog, besides the instantaneous\
	  reaction to individual argument adjustments, is the ability to "Browse"\
	  to files or directories, although typing always remains valid.\
	  In contrast to the preference-controlled 'automatic' search mode of the\
	  command line (see <itl>$pref(autoSrch)</itl> setting in the\
	  "<btn>On Preferences</btn>" <btn>Help</btn> page), requesting that mode\
	  via the dialog is handled via a checkbox, that will only be presented\
	  when conditions indicate it is possible.
	  Please note that <itl>most</itl> of the "Additional optional parameters"\
	  <itl>are available</itl> from the dialog, but are initially hidden from\
	  view, as they are often not applicable except in special cases. If you\
	  need to set them, press <btn>More</btn> to view them <bld>but</bld>\
	  <bld>do not</bld> re-"hide" them (by pressing <btn>Less</btn>) before\
	  clicking the <btn>OK</btn> button on the dialog as hiding them\
	  <itl>ALSO</itl> causes them ALL to become completely <bld>unset</bld>.\
	  Lastly, those items not provided for within the dialog ('Ignore...'\
	  settings, etc.), or really <itl>ANY</itl> of the\
	  "Additional optional parameters" listed may still be provided on the\
	  command line without forfeit of invoking the dialog.
	}
	set topic3 {
<hdr>Recognizing and Selecting the CDR</hdr>

All difference regions (DRs) are <itl>typically</itl> highlighted to set them\
	  apart from the surrounding text, unless the "<btn>$pref(tagtext)</btn>"\
	  preference has been deselected. The <itl>current difference region</itl>,\
	  or <bld>CDR</bld>, is further set apart in the Left text window so that\
	  it can be correlated to its partner in the other (that is, the CDR on\
	  the left matches the CDR on the right). This "correlation" is most easily\
	  seen by requesting that the CDR be "centered" in both text windows,\
	  either on demand: using either the popup menu, toolbar button, or keyboard\
	  accelerator hotkey; or by choosing an applicable user preference such as\
	  "<btn>$pref(autocenter)</btn>", which when paired with\
	  "<btn>$pref(syncscroll)</btn>", will cause both Left and Right CDRs to\
	  nearly <bld>always</bld> be aligned as well. You can read more about\
	  these and other preference settings in the <btn>Help</btn> menu topic\
	  "<btn>On Preferences</btn>".

The CDR can be chosen in a sequential manner by means of the <btn>Next</btn>\
	  and <btn>Previous</btn> buttons. Similarly, the <btn>First</btn> and\
	  <btn>Last</btn> buttons allow you to quickly navigate to the\
	  first or last CDR, respectively. For random access to the DRs, use the\
	  dropdown listbox in the toolbar or the diff map, described below.

By clicking right over MOST windows and using the popup menu you can select\
	  <btn>Find Nearest Diff</btn> to find the diff region nearest the point\
	  where you right-clicked, or simply double-click either <itl>on</itl> or\
	  <itl>near</itl> an existing DR, as a shortcut to the same result.
	  As double-clicking over the Text window can ALSO be interpretted as a\
	  request to "select" a word in that window, you may perform the\
	  double-click over the INFO window (if displayed) NEXT to the line(s)\
	  that comprise the equivalent lines of its text window to AVOID the\
	  selection process. HOWEVER, note that using <btn>Find Nearest Diff</btn>\
	  from either the popup-menu OR by double-clicking when clicked OVER the\
	  "Diff Map" is <itl>interpretted</itl> differently BECAUSE the content\
	  of the Map represents the <bld>entire file</bld> and not just the lines\
	  that appear to be adjacent on either side of it. It is THAT position\
	  in the <bld>file</bld> where the search for the "nearest" DR will begin.

For keyboard-centric power users, be advised that causing $g(name) to nominate\
	  a new CDR will cause the text display "insert" cursor to immediately\
	  jump to that CDRs first line. Accordingly, you might benefit from making\
	  some minor adjustments to the "<itl>Text widget options</itl>" preference\
	  setting (specifically adding "-insertbackground <itl>color</itl>" and\
	  "-insertwidth <itl>numberOfPixels</itl>") with appropriate values to make\
	  it easier to <bld>see</bld> the location of this critical piece of many\
	  keyboard-based operations.

<hdr>Operations</hdr>

1. From the <btn>File</btn> menu:

The <btn>New...</btn> item displays a dialog where you may choose two files\
	  to compare. Selecting "OK" from that dialog will diff the two files. Be\
	  advised that this is the same dialog as may appear when $g(name) is\
	  started with no command line parameters given, and its described\
	  behavior there is the same as invoking it from this context (see the\
	  help topic "On Concepts+Syntax" for specific details).
	  Next, the <btn>File List</btn> item will only be active when the current\
	  $g(name) command parameters yeilds more than a single pairing of files\
	  to compare; pressing it produces a submenu that gives access to the list\
	  of the other available comparisons. Depending on how lengthy that list\
	  might be, it may be shown <bld>directly</bld> in the menu itself, up to\
	  an <itl>adjustable</itl> maximum of 25 names. When that is the case,\
	  choosing one re-initializes the display by performing a Diff to the file\
	  pair thus selected. The names themselves are only the "Left"-side files,\
	  but they represent <itl>both</itl> of the paired files being compared.\
	  Note that <itl>after</itl> choosing an item, the background of that item\
	  will henceforth be red or green when the mouse hovers over that item,\
	  based on whether that pairing was successfully read into $g(name). When\
	  no color is shown, that item has NOT yet been accessed.\
	  List items <itl>may</itl> require noticeable time to load\
	  if the files each represents requires network access to be processed;\
	  however, once loaded, subsequent reloading is entirely a local task.
	  When the submenu does <bld>NOT</bld> display the files, the FIRST entry\
	  of the menu will read "<btn>Choose File...</btn>" which, in turn, causes\
	  a popup window to be displayed containing the list. The operation of the\
	  list is the same regardless of which (menu or popup) is in use. Remaining\
	  items on the submenu simply select either the <btn>Previous File</btn> or\
	  <btn>Next File</btn> relative to whatever file was current. However,\
	  there is an additional control on the popup that permits you to\
	  <itl>adjust</itl> the threshold value that $g(name) should use in\
	  determining which form of the list to provide. As this threshold is also\
	  a user preference ("$pref(fLMmax)"), more details are available in the\
	  help category "On Preferences", including recommended uses.
	  For the case where files <itl>ARE</itl> shown in the submenu, the\
	  <btn>Choose File...</btn> item becomes renamed to\
	  <btn>Reconfig Threshold...</btn> allowing you to switch to using the popup\
	  list at your discretion. Note that because the list presentation depends\
	  on the <bld>current</bld> number of files, performing a <btn>New...</btn>\
	  Diff, may fall above or below your <itl>present</itl> threshold.
	  The <btn>Recompute Diffs</btn> item recomputes the differences between\
	  the two files whose names appear above each of the two text display\
	  windows. The <btn>Write Report...</btn> item lets you create various text\
	  report files that can contain information content of your choosing from\
	  the text windows(s). In addition, simply visiting the dialog to compose\
	  your report will provided detailed statistics on the breadth and\
	  complexity of the differences between the current file pair, which will\
	  automatically be included in any such report created. You may, of course,\
	  choose to not produce <itl>ANY</itl> report, and simply view the\
	  statistics. For more information about the reports themselves, see the\
	  section "Report Generation" presented later. Lastly, the <btn>Exit</btn>\
	  item terminates $g(name).

2. From the <btn>Edit</btn> menu:
			(be advised that many items of this menu are ALSO included as
			 toolbar buttons, further described under section <bld>8</bld> below.
			 It is recommend you read BOTH)

<btn>Copy</btn> copies the currently selected text to the system clipboard.\
	  <btn>Find</btn> pops up a dialog to let you search either text window\
	  for a specified text string, but see additional details in section #8\
	  below. <btn>Ignore CDR</btn> allows you to designate the\
	  <itl>present</itl> CDR as no longer being of any concern whatsoever to\
	  $g(name). It can be used for those situations where it would be\
	  otherwise <itl>dangerous</itl> to attempt to automatically\
	  ignore the Diff region by some <bld>rule</bld> (such as implied by the\
	  <btn>Ignore RE-matched Lines</btn> mechanism of the <btn>View</btn> menu\
	  described shortly). <btn>Split...</btn> and <btn>Combine...</btn> pops\
	  up a dialog that allows you to rearrange the CONTENT of the CDR to\
	  isolate specific lines, facilitating specific merge file generation\
	  goals. It should be noted however, that these last three operations\
	  ("Split", "Combine", and "Ignore") are LOCAL to $g(name) itself; meaning\
	  that each represents work performed by you to <itl>adjust</itl> the\
	  <bld>result</bld> of the most recent Diff, generally in pursuit of some\
	  "Merge" goal. Such work is <itl>ONLY VALID</itl> until such time as a\
	  mergefile is written out (to lock the work in place) or a subsequent\
	  Diff is run <itl>by any means</itl> to effectively <bld>cancel</bld>\
	  such work! See the upcoming sub-heading <bld>Merging</bld> for further\
	  info. <btn>Edit File 1</btn> and <btn>Edit File 2</btn> launch an editor\
	  on the files displayed in the left- and right-hand panes.\
	  <btn>Preferences</btn> pops up a dialog box from which display\
	  (and other) options can be changed and saved.

3. From the <btn>View</btn> menu:

This menu is organized into a few sections, the first of which deals with\
	  how the output from the diff engine can be tuned or interpretted.\
	  <btn>Utilize Suppressions</btn> toggles whether certain user preference\
	  defined options should (or not) be used when invoking Diff. Both of\
	  <btn>Ignore Blank Lines</btn> and <btn>Ignore RE-matched Lines</btn>\
	  in turn, toggle an ability to suppress (basically NOT notice or\
	  highlight) any difference region identified by the engine that is\
	  <itl>exclusively</itl> comprised of the indicated category. Lines that\
	  otherwise seem to match, but have been "grouped" by Diff into a larger\
	  difference region are <bld>NEVER</bld> suppressed.
	  IMPORTANT: toggling <itl>any</itl> of these settings will cause $g(name)\
	  to <itl>immediately</itl> re-invoke the diff engine so as to provide\
	  the requested interpretation. This <bld>will cause the loss</bld> of\
	  any merge work that may have been in progress at that time. For this\
	  reason, when the keyboard or mouse is positioned to select any of them,\
	  they will be highlighted to remind you of this impending loss.
	  In the second section are items controlling what information gets\
	  displayed within the tool itself. Both <btn>$pref(showln)</btn> and\
	  <btn>$pref(showcbs)</btn> toggle the display of line numbers and\
	  markers (respectively) in the text displays. <btn>Show Diff Map</btn>\
	  toggles the display of the diff map (see below) on or off.\
	  The <btn>Show Line Comparison Window</btn> item toggles the display of a\
	  literal two line over/under "line comparison" window near the bottom of\
	  the display. As an alternative to that, the two mutually exclusive items\
	  <btn>Show Inline Comparison (byte)</btn> or\
	  <btn>Show Inline Comparison (recursive)</btn> will display the specific\
	  interline differences as configurable highlighting directly\
	  <itl>within</itl> the Left and Right text displays themselves. You may\
	  choose any combination, at any time, as suits your comprehension needs.
	  HOWEVER - be advised the (so-called) 'recursive' method is particularly\
	  compute-intensive, which is at odds with being an <itl>interactive</itl>\
	  tool; you may notice a slight hesitation as these appear on screen. This\
	  is normal, and wiill complete often within a single or more second(s). Yet\
	  owing to the nature of how such interactive response is provided, its\
	  POSSIBLE the display may MISS a specific hilite situation if you happen\
	  to be SCROLLING when it decides the calculation is required. <bld>DO NOT\
	  PANIC</bld> - simply toggle the <btn>View</btn> menu item\
	  <btn>Show Inline Comparison (recursive)</btn> OFF and back ON, and the\
	  missing Hilites will appear.
	  The third section addresses automatic processing that can be performed\
	  as other interactions in $g(name) take place.\
	  If <btn>Synchronize Scrollbars</btn> is on, the Left and Right\
	  text windows are synchronized i.e. scrolling one of the windows scrolls\
	  the other. If <btn>Auto Center</btn> is on, jumping (by whatever means)\
	  to a new CDR centers that new CDR automatically. <btn>Auto Select</btn>\
	  will attempt to designate the diff region currently closest to the\
	  middle of a scrolled Left/Right text window <itl>AS</itl> the new CDR;\
	  however, only when <btn>$pref(syncscroll)</btn> is also ON.\
	  Furthermore, if the window is in the process of being <itl>manually</itl>\
	  scrolled (via a mousewheel or driving the insert-cursor about the screen\
	  by way of keyboard actions), <btn>Auto Select</btn> will continue to\
	  operate, yet <btn>Auto Center</btn> (if active) will be temporarily\
	  suppressed, to avoid fighting over what (or who) should control scrolling.
	  The fourth (and final) section basically reiterates simple navigation\
	  actions available elsewhere (toolbar, popup menu) for moving among the\
	  various diff regions.

4. From the <btn>Mark</btn> menu:

The <btn>Bookmark Current Diff</btn> creates a new toolbar button that will\
	  jump to the current diff region. The <btn>Clear Current Diff Mark</btn>\
	  will remove the toolbar mark button associated with the current diff\
	  region, if one exists. When created, each is labelled with the index of\
	  the present CDR (as depicted in the first tool on the toolbar - see\
	  below in subsection #8). Be advised that during <btn>Split</btn> or\
	  <btn>Combine</btn>\operations (described shortly), or <itl>ANY</itl>\
	  operation that would recompute one or more DRs, these "bookmark" buttons\
	  may automatically be cleared, but <bld>only when</bld> they are\
	  <bld>directly</bld> involved.

5. From the <btn>Merge</btn> menu:

The <btn>Show Merge Window</btn> item pops up a window with the current\
	  merged version of the two files. This will be described further in a\
	  later section called "Merge Preview" below.\
	  The <btn>Write Merge File</btn> item (or possibly the\
	  <btn>Write Merge File...</btn>) will allow you to save the contents of\
	  that window to a file.
	  Pay special attention to the existance of those three trailing dots when\
	  electing to write the Merge File (either here <itl>OR</itl> from the\
	  buttons on the dialog itself) - if they are <bld>NOT</bld> present,\
	  it means $g(name) <itl>already knows</itl> what filename to produce,\
	  (i.e. from the command line) and you will <bld>not</bld> be given a\
	  chance to confirm or alter that name.

6. From the <btn>Help</btn> menu:

The <btn>About $g(name)</btn> item displays copyright and author\
	  information. The <btn>On GUI</btn> item generates this window. The\
	  <btn>On Concepts+Syntax</btn> item displays help on the $g(name) command\
	  line options and syntax, but also includes discussions on topics\
	  related to tool startup such as initiating a particular run-mode or\
	  interactively supplying the command arguments. Lastly, user-settable\
	  preferences help is provided via the <btn>On Preferences</btn> item.

7. From the <btn>Popup</btn> menu:

The Popup menu is generally available over the majority of individual windows\
	  comprising the "Left/Right" aspects of the overall display, activated by\
	  the, so-called "menu" button of the mouse. Each of the operations it\
	  provides is available elsewhere, although it can be convenient to have\
	  the mechanisms "close by" during operation. It is a "context sensitive"\
	  menu in that not every operation is ALWAYS available, depending on WHERE\
	  the original popup request was made. Yet each item still performs the\
	  same functionality as the other locations (toolbar, keyboard, mouse)\
	  would have. This functionality consists of all the toolbar navigation\
	  choices (First, Last, Next, Previous, Center) CDRs, plus a selection of\
	  others such as Find, Edit and "Copy Selection" (to the clipboard), along\
	  with the earlier discussed "Find Nearest Diff". Just dont expect to ask\
	  to "Edit" one of the files, when popped-up over the Diff Map... you NEED\
	  the context (where you popped-up) to hint at which SIDE, thus which file,\
	  is being requested!

8. From the toolbar:

(Be advised that in these explanations, the button descriptions refer to the\
	  <itl>textual</itl> name <bld>ON</bld> that button as would be seen when\
	  the user preference to "<itl>$pref(toolbarIcons)</itl>" is unset.)

The first tool is a dropdown list of all of the differences in a standard\
	  diff-type format (prefixed with a simple consecutive index number).\
	  You may use this list to go directly to <itl>any</itl> diff\
	  region. Further navigation tools will be described in due turn.\
	  Proceeding left-to-right, the next tool, <btn>Rediff</btn>, simply\
	  re-computes the diff of the CURRENT two files from scratch as if it was\
	  a new Diff. This could be appropriate if you have invoked an editor on\
	  either file since starting and now wish to see the net effects of your\
	  editting; just recall that <itl>doing so</itl> will cause the loss of\
	  pending <itl>interactive</itl> work such as merge choices, Splits, and\
	  so forth.	 The next tool <btn>Ignore</btn> will cause the CDR to no\
	  longer be treated <itl>AS</itl> a DR; it is a interactive method not\
	  unlike those that perform a similar service based on command line flags\
	  that match either a empty or regular-expression defined line. The next\
	  two tools, <btn>Split</btn> and <btn>Combine</btn>, each provide\
	  complimentary abilities to adjust the boundaries of the CDR. The reasons\
	  for doing this are further explained in the section below on Merging.
	  The remaining tools on the toolbar consist of the <btn>Find</btn> tool\
	  for searching the text for a given word or phrase. Because of its\
	  ability to search EITHER Text window independently, it will IGNORE\
	  any setting of "<itl>$pref(syncscroll)</itl>" while active, and instead\
	  use the <btn>Stay Sync'd</btn> toggle ON the search panel itself. When\
	  subsequently dismissed, synchronization will revert back to the prior\
	  established value; further, any scroll operations performed DIRECTLY on\
	  the windows themselves will continue to be governed by that same master\
	  setting.
	  Following this are, in order, groupings of tools dealing with merge\
	  choice selections, navigation, and lastly a Bookmarking facility for\
	  remembering specific diff positions so that jumping among them does\
	  not require memorization of somewhat meaningless numbers. These, among\
	  other topics, will now be further detailed.

	  (Editors note: The next <itl>physical group</itl> of tools ("Merge:")
	  will be deferred until after the others, as it is predicated on
	  understanding practically <itl>ALL</itl> of the others and how they may
	  interact - besides being a complex topic in its own right).

<hdr>Navigation tools</hdr>

	  Adjacent to the label <bld>Diff:</bld>, the <btn>Next</btn> and\
	  <btn>Prev</btn> buttons take you to the "next" and "previous" DR,\
	  respectively; just as the <btn>First</btn> and <btn>Last</btn> buttons\
	  take you to the "first" and "last" DR. These actions will <itl>also</itl>\
	  affect the Merge Window (when displayed). The <btn>Center</btn> button\
	  centers the CDRs in their respective text windows. You can also set\
	  <btn>Auto Center</btn> in <btn>Preferences</btn> (or via the\
	  <btn>View</btn> menu) to do this automatically for you as you navigate\
	  through the diff regions. Dont forget that the dropdown list (the first\
	  tool on the toolbar) <itl>ALSO</itl> provides movement to <itl>any</itl>\
	  DR, as well. Even Bookmarks (explained shortly) can do the same.

<hdr>Keyboard Navigation</hdr>

When $g(name) has the current keyboard focus, you may also use the following\
	  (<bld>global default</bld>) keyboard shortcut keys:
<cmp>
	^[	   (Ctrl-Bracketleft)  Load NEXT file pair
	^]	   (Ctrl-Bracketright) Load PREV file pair
	c	   Center current diff
	f	   First diff
	l	   Last diff
	n	   Next diff
	p	   Previous diff
	e	   Load a text editor with the 'current' file
	^f	   (Ctrl-f) Find some specified piece of text
	^r	   (Ctrl-r) Recompute Diffs of current file pair
	^q	   (Ctrl-q) Exit $g(name) immediately
	1	   Elect Left as the CDR Merge Choice
	2	   Elect Right as the CDR Merge Choice
	3	   Elect Left-then-Right as the CDR Merge Choice
	4	   Elect Right-then-Left as the CDR Merge Choice
</cmp>
There are, of course, other keyboard operations that apply as well, such as\
	  platform specific keys to invoke a "button", but all such keys (including\
	  ours) each require a concept known as "keyboard focus" to be properly\
	  located on the object which is intended to respond. The windowing toolkit\
	  (Tcl/Tk in our case) defines most of these, as it does the means of\
	  <itl>assigning</itl> such focus (pressing 'Tab' or 'Shift-Tab') to switch\
	  among such items capable of responding. But while the\
	  "keystroke-to-action" relations (aka. bindings) defined by Tcl/Tk are\
	  nearly <itl>always</itl> targetted at a given type of "widget", those of\
	  $g(name) are more "global" in nature, mostly only requiring the current\
	  focus to be <itl>somewhere</itl> within the "main" window.
	  That is not to say that ambiguous situations are impossible. Most notable\
	  of these is the "<bld>e</bld>" (edit) Hotkey: assuming the current focus\
	  is associated with <itl>anything</itl> having a suitable "Left" or\
	  "Right" connotation (such as a text display or scrollbar) that will\
	  dictate which file is accessed. Failing that, $g(name) will look at where\
	  the Mouse currently rests and retry using its position (such as when the\
	  focus is on a toolbutton). For this reason, it is generally safest to use\
	  the popup menu to invoke the editor, ensuring the desired file is loaded.\
	  However, note $g(name) <itl>DOES</itl> add one "extra" builtin hotkey\
	  which is:<cmp>
	Return	  Make the closest diff range become the CDR
</cmp> where closest means the range closest to the 'insert cursor' in the text\
	  window <itl>having</itl> the <bld>active</bld> keyboard focus only.

It is important to emphasize that these are only the <itl>DEFAULT</itl> hotkeys\
	  as defined by $g(name) before any customizations or applied preferences.\
	  Prior to Version 5.1, these values (well, most of them) were\
	  <bld>hard-coded</bld> <itl>and</itl> non-customizable. That is\
	  <itl>NO LONGER THE SITUATION</itl>! But, due to concerns over accidental\
	  loss of in-progress work from a simple keypress, the "Control"-modifier\
	  has been added to the historical bindings of "r" and "q", <itl>AS</itl>\
	  defaults; you can, after all, <itl>now</itl> re-instate or alter them as\
	  you see fit. See the section entitled "Behavior" under the\
	  <btn>Help</btn> topic <btn>On Preferences</btn> for the details on\
	  choosing your own settings.
	  In addition, the cursor keys, Home, End, PageUp and PageDown work as\
	  expected, affecting the view in whichever Text window has the focus.\
	  Note that, as expected, if <btn>$pref(syncscroll)</btn> is set in\
	  <btn>Preferences</btn>, and the keyboard actions imply scrolling, both\
	  will scroll simultaneously, despite these keys only affecting the insert\
	  cursor of the presently focussed window.

<hdr>Scrolling</hdr>

To scroll the text widgets independently, make sure\
	  <btn>$pref(syncscroll)</btn> in <btn>Preferences</btn> is off. If it is\
	  on, scrolling either text widget scrolls the other. Scrolling will not\
	  change the current diff region (CDR) in this condition, nor will it cause\
	  the Merge Window (if displayed) to scroll. A Mouse scroll-wheel is also\
	  recognized for scrolling vertically, or, if the <cmp>Shift</cmp> key\
	  is simultaneously pressed, horizontally, as well.

<hdr>Book Marks</hdr>

Located adjacent to the label <bld>BkMark:</bld>, you can set "bookmarks" that\
	  identify specific diff regions, primarily for easier navigation.\
	  To do this, click on the <btn>Set</btn> bookmark button when the desired\
	  DR is currently the CDR. It will create a <bld>new</bld> toolbar button\
	  that will jump back to this specific diff region when pressed.
	  To clear a diff mark, first <bld>make that DR the CDR</bld>, then click\
	  on the <btn>Clear</btn> bookmark button. Each is labelled with the\
	  sequence number of the DR it represents. Note however, that because\
	  <btn>Split</btn> or <btn>Combine</btn> can <itl>both</itl> manufacture or\
	  destroy <itl>specific</itl> DRs, it can become necessary for $g(name) to\
	  "Clear" a given bookmark. The same can be said for <btn>Ignore</btn>.\
	  Only those specific markers involved are affected; however, any marker\
	  carrying a label <itl>beyond</itl> any addition or contraction of DRs\
	  <bld>will</bld> always have their labels 'adjusted' accordingly to\
	  maintain their originally designated region association.
	  The actual Bookmark buttons themselves, will appear in whatever remaining\
	  space exists on the righthand side of the toolbar. However, should you\
	  create more than the available space can handle, $g(name) will provide a\
	  pair of "scroller" buttons to enable you to create as many as needed, yet\
	  still be able to access them as required. These "scrollers" will\
	  auto-repeat when pressed-and-held with the mouse, deactivating when the\
	  designated end-of-the-list is reached, and disappearing altogether when\
	  no longer needed.
	  Bookmarks are created, and are maintained, in the <bld>order</bld> you\
	  choose to manufacture them. Thus those created earlier will be nearer the\
	  Left edge of all bookmarks available. As mentioned earlier, each is\
	  labelled with the DR sequence number it represents, but if it is\
	  important to have a <itl>more-permanent</itl> "identity" for the given\
	  DR, you can right-click the bookmark and select <btn>annotate</btn> from\
	  its popup menu to assign a short description of your choosing. This\
	  naming, like the default one internally assigned, will be displayed in\
	  the Status bar whenever the mouse hovers over that bookmark.\
	  Note however, that despite naming a bookmark, the bookmark is still\
	  susceptible to being deleted.
	  The other item on the bookmark-button menu, is a toggle,\
	  <btn>in report</btn>, that can be used to designate the DR to participate\
	  (or not) in a specific <btn>Write Report...</btn> feature that allows you\
	  to include only those DRs that have been "tagged" for inclusion. In this\
	  fashion, you can document only the specific changes you choose to\
	  identify, perhaps to illustrate some particular issue in addressing how\
	  best to resolve it.

<hdr>Report Generation</hdr>

$g(name) can output various textual forms of the same data as viewed in the\
	  main display windows. At the present time, this does <itl>not</itl>\
	  include the many various forms of "highlighting" rendered by the tool\
	  directly onscreen. Nonetheless, the data can be assembled in several\
	  different combinations from the full text of BOTH (or either) side(s);\
	  or only the "difference regions" (again for either side), and even just\
	  <itl>SELECTED</itl> DRs (courtesy of the "Bookmark" menu options). When\
	  presented with the dialog to make your choices, you will also be shown\
	  statistics on the magnitude of the DRs in various breakdowns. This same\
	  set of data will form part of the Heading within the report, which also\
	  includes the file names (with applicable modification timestamps where\
	  possible) and, of course, the date the report is generated. It is even\
	  permitted to get just the header information with NONE of the actual\
	  file content as the output.
	  While an output filename is provided by default, you may retype it OR\
	  use the <btn>Browse...</btn> to specify a replacement. Note however that\
	  regardless of your simply "taking" the default or typing (or RE-typing)\
	  a replacement, $g(name) will verify that the name provided is "safe" to\
	  write to, meaning that if it refers to an <bld>existing</bld> file,\
	  you will be given the opportunity to change or confirm it via the\
	  provided file browser window.

<hdr>Diff Map</hdr>

The diff map is a graphic index of where all the diff regions exist. It is\
	  shown in the middle of the main window if <btn>Show Diff Map</btn> on\
	  the <btn>View</btn> menu is on. The map is a miniature of the file's\
	  DRs from top to bottom. Each DR is rendered as a patch of color;\
	  initially Delete as red, Insert as green and Change as blue and in the\
	  case of a 3-way merge, overlap regions, called "collisions" are marked\
	  in yellow. These colors are simply the defaults provided by $g(name),\
	  and can be adjusted via the <btn>Preferences...</btn> item in the\
	  <btn>Edit</btn> menu, to perhaps compensate for better contrast or\
	  spectrum adjustments given other objects onscreen with your particular\
	  monitor (or simply personal taste).
	  The height of each patch corresponds to the relative size of the diff\
	  region. A transparent "thumb" lets you interact with the map as if it\
	  were a scrollbar, and Mouse scroll-wheel actions are fully supported,\
	  but will be <itl>directed</itl> to whichever of the two text windows is\
	  holding the current input focus, if the windows are not synchronized.\
	  All diff regions are drawn on the map even if too thin to ordinarily be\
	  visible. For large files with small nearby diff regions, this may result\
	  in patches overwriting each other, due to scaling issues.

<hdr>Merge Preview</hdr>

To see an ongoing preview of the file that would be written by\
	  <btn>Write Merge File</btn>, select <btn>Show Merge Window</btn> in the\
	  <btn>Merge</btn> menu. A separate window will be shown containing the\
	  preview. It is updated as you select merge choices, and provides markers\
	  that remind you as to which side (Left/Right) is presently contributing\
	  its region into the result. Note that when viewing a choice such as the\
	  Left-side of an "add"-type, or the Right-side of a "del"-type CDR,\
	  there is <itl>nothing</itl> to actually display. Additionally, the\
	  Preview window is responsive to the current <btn>$pref(showln)</btn>\
	  preference setting. It is also synchronized with the other text widgets\
	  when <btn>Synchronize Scrollbars</btn> is on, at least as far as actions\
	  that <itl>change</itl> the CDR, however it <bld>does not</bld> actually\
	  <itl>scroll</itl> in unison with the other windows, primarily because as\
	  a representation of the eventual Merge file, it does NOT HAVE any of the\
	  <itl>padding</itl> lines which accounts for a substantial amount of the\
	  vertical spacing being scrolled by the other windows.

<hdr>Merging</hdr>

To merge the two files, go through the difference regions (via <btn>Next</btn>,\
	  <btn>Prev</btn> or whatever other means you prefer) and select\
	  <btn>L</btn> (for "Left") or <btn>R</btn> (for "Right"), located\
	  adjacent to the toolbar <bld>Merge:</bld> label, assigning which side\
	  should be used for each. Alternately, the "<bld>1</bld>" & "<bld>2</bld>"\
	  (default) hotkeys will do the same, respectively. The initial selections\
	  (after invoking Diff) will have already been established by a user\
	  preference and/or whether a 3way (involving an ancestor file) was\
	  performed (explained further in the section "<bld>3way merging</bld>"\
	  below).
	  Selecting <btn>L</btn> means that the the left-most file's version of\
	  the difference will be used in creating the final result; choosing\
	  <btn>R</btn> means that the right-most file's difference is used. Each\
	  choice is recorded, and can be changed arbitrarily many times.\
	  If you need pieces from BOTH the Left AND Right versions you may choose\
	  the <btn>LR</btn> or <btn>RL</btn> (Left-then-Right or\
	  Right-then-Left, respectively) choices instead, <itl>but then</itl> you\
	  must remember to eventually edit the merged result <bld>AFTER</bld> you\
	  commit it to disk. This might be useful, for example, if <itl>both</itl>\
	  variations should exist with additional wording, or in the case of source\
	  coding, a conditional inclusion macro, surrounding the entire result. To\
	  commit the final, merged result to disk, choose\
	  <btn>Write Merge File</btn> from the <btn>Merge</btn> menu, or one of the\
	  <btn>Save</btn> buttons provided on the dialog (if it is displayed).\
	  Remember that each of these items may be labelled with a trailing "..."\
	  if $g(name) is <itl>uncertain</itl> of what the target filename should\
	  be, thereby providing a file browser dialog to either specify and/or\
	  confirm the name.

<hdr>Merging - in more detail</hdr>

Oftentimes, you may find that the "Diff" engine has packed several lines\
	  worth of differences into a large chunk, simply because it never found a\
	  <itl>common</itl> line that BOTH files could agree was the SAME in both\
	  files. Yet only a <itl>SINGLE</itl> defined difference region (a CDR)\
	  can have its Left or Right side chosen for merging at any one time.\
	  As a side note, "context" and "unified" diff output formats tend\
	  to exacerbate this problem, and is part of the reason we dont generally\
	  like them as a data format, although we <itl>DO</itl> allow them,\
	  automatically deconstructing them into the equivalent "normal" format.\
	  Nevertheless, this is the "problem" that <btn>Split</btn> or\
	  <btn>Combine</btn> are intended to address. Using these tools, you will\
	  be permitted to repartition the exact lines that should be treated as\
	  a distinct difference region. In each case, you start from some specific\
	  CDR, and then either break it apart into smaller pieces ("Split") or\
	  reassemble it ("Combine") at line boundaries of your choice.
	  A dialog window is provided to oversee the movement of the CDR boundary\
	  edges, with feedback provided in the Text windows. You need only to\
	  click on arrows to adjust either or both edges in the Left or Right\
	  text window displays until satisfied that the <itl>NEW</itl> CDR\
	  describes the change content you wish to convey. Be aware these arrow\
	  buttons will <itl>automatically</itl> advance if you press and hold\
	  instead of clicking, making it easier to adjust a large expanse.
	  Note that only <itl>legal</itl> edge motions are ever permitted,\
	  and the buttons will automatically deactivate as necesssary. Most\
	  actions will make a visible change in highlighting as seen in the main\
	  text windows. One specific highlight is worthy of extra explanation,\
	  however. When performing the movement of an edge during a "Split"\
	  operation and that edge begins "<itl>pushing</itl>" against its opposing\
	  edge, movement will STILL occur. But because the highlighting that\
	  <bld>was</bld> representing the middle region of the three, will have\
	  <itl>NOW</itl> been squeezed shut, that highlighting will then be\
	  represented by a different shading of the LOWER of the TWO edges that\
	  sit on opposing sides of the demarcation of the two remaining regions.\
	  Backing off one step, in <itl>either</itl> direction however, will\
	  "re-open" that center region and revert its associated highlighting.
	  As the dialog is in complete control of the text windows at this point,\
	  it will also control scrolling the window as necessary to keep the edge\
	  being moved visible. When a moving edge gets within <itl>two</itl>\
	  lines of the top or bottom of the window, the window will be scrolled\
	  to maintain that visibility. This can be disorienting when the DR is\
	  exceptionally large. Because of this FORCED individual scrolling of\
	  the windows, even in the presence of options to request\
	  "<itl>$pref(syncscroll)</itl>" or "<itl>$pref(autocenter)</itl>",\
	  BOTH <btn>Split</btn> and <btn>Combine</btn> will automatically\
	  RESTORE the view per those settings when they terminate, regardless\
	  if that is a successful or cancelled operation.
	  Once accepted, $g(name) will treat the new difference region exactly\
	  the same as any other, despite the fact that it appears run together\
	  with other adjoining regions, having NO common line to separate them.\
	  The power of this is that two modifications, having NOTHING to do\
	  with each other beyond proximity, can thus be merged (or not)\
	  <itl>INDIVIDUALLY</itl> as needed. Given that many version control\
	  systems prefer that only those lines pertinent to a specific logical\
	  change reside in a given 'patch', these features allows the user to\
	  surgically distinguish one <itl>logical</itl> change from another.
	  Note that ONLY a previously Split region, can ever be Combined,\
	  <bld>provided</bld> you do <itl>NOT</itl> choose to <btn>Ignore CDR</btn>\
	  some portion of it in the interim. Note further that $g(name) will\
	  always assign each line of the original CDR into an appropriate region\
	  (creating and/or removing existing regions as necessary), and\
	  <itl>automatically</itl> assigning its type (add/change/delete).
	  If you have difficulty envisioning which edges to move to accomplish\
	  a specific goal, think of the edges as defining 3 individual regions\
	  per side of data: Above-the-CDR, the NEW CDR, and Below-the-CDR. Then\
	  remember that changes always flow from the left side to the right. Thus\
	  when a Left side region has a zero size, the corresponding Right side\
	  region is being "added". Conversely, if a right-side region describes\
	  zero lines, the left-side region describes a "delete". Regions that BOTH\
	  have lines are simply "changes".
	  Note that only REAL lines (those having Line numbers, when shown) are\
	  ever counted toward the occupancy of the regions. Padding lines\
	  (displayed to align CDRs on screen) mean nothing despite their being\
	  highlighted as part of a CDR, and will be <itl>stepped over</itl> as\
	  edges are moved. Finally, remember that any changes <itl>YOU</itl> might\
	  make to any CDR content is transitory, and only exists within $g(name)\
	  until the next time any "Diff" is invoked, even a <btn>Rediff</btn>. This\
	  suggests that before beginning any merge work, you shoud ensure that all\
	  settings or menu choices that adjust or interpret the Diff results\
	  (predominate side, ignored blanks/lines), or worse, those that might\
	  <itl>trigger</itl> a new Diff invocation if they are <itl>changed</itl>,\
	  have all been configured appropriately. ALL interactive merge work\
	  (including Split/Combine and Ignore) is transitory until the merge file\
	  is actually written out, and <bld>can not</bld> be automatically\
	  recovered; <bld>only</bld> reconstructed!

<hdr>3way Merging</hdr>

A 3way merge, as the name suggests, involves a third file that is expected\
	  to have been an earlier <itl>common</itl> version to <itl>both</itl>\
	  files presently being compared. Providing this <bld>ancestor</bld> file\
	  will cause an icon to appear <itl>between</itl> the normal Left/Right\
	  file labels on the display (indicating the mode is in force and\
	  permitting viewing access if absolutely necessary) and thereby\
	  allow $g(name) to look backward in time, to address the unique issue of\
	  intentionally diverged <itl>independent</itl> modifications (the Left\
	  and Right files) being merged back together into a single output file.
	  Specifically, $g(name) wants to identify the modifications that\
	  <itl>created</itl> the Left and Right variants, with the intention of\
	  preserving <bld>ALL</bld> such changes (both sides) into the final\
	  result, as automatically as possible. Thus, among the Left/Right diffs\
	  being shown by $g(name), certain lines may, or may not, have been\
	  modified during their creation from the ancestor. We call these\
	  <bld>ancestral</bld> artifacts, and $g(name) will annotate such lines\
	  using markers to the left of the line numbers (if displayed), denoting\
	  what kind of modification (add, chg, del) had previously occurred. Note\
	  that <itl>ancestral deletions</itl> technically no longer\
	  <itl>exist</itl> in their respective Left/Right files, and thus were\
	  effectively and implicitly embedded into those files at that time.
	  HOWEVER, given the notion that "merging" is supposed to be the proper\
	  <itl>inclusion</itl> of <bld>BOTH</bld> sets of changes, that would\
	  mean Deletions <itl>must also</itl> be fully included. Accordingly\
	  if only ONE side were to delete a specific line, the <itl>failure</itl>\
	  of the OTHER side to do the same, suggests that the decision to remove\
	  the line in one version and not the other is questionable, and is no\
	  different than the case of Adding a line in one version and not the\
	  other. <itl>Because</itl> of this, $g(name) WILL PROVIDE an ancestral\
	  artifact on lines that were <bld>NOT</bld> deleted as they were by the\
	  opposing version; to remind the user that choosing one side versus the\
	  other when merging is not always just a "simple" choice.
	  To distinguish a "Deletion" artifact from an Additive one, $g(name) will\
	  display such items in <itl>inverse</itl> video AND as Capitalized. Just\
	  remember that the inverse video is signalling that the ancestral\
	  Deletion - <itl>from the other side</itl> - decided the line SHOULD BE\
	  (and has been) removed, while the marked side says the line should\
	  remain - <bld>NOT</bld> that you the user should CHOOSE the side with\
	  the inverse mark to CAUSE the line to be deleted (which it would NOT do).
	  Generally, when ancestral markers show up in ONLY the Left (or Right)\
	  windows, $g(name) simply responds by choosing that side as the initial\
	  merge choice for that region (except, obviously, for inverse Deletion\
	  marks). When <itl>BOTH</itl> sides show markers <itl>of the same\
	  type</itl> (regarding it being Capitalized .vs. NOT Capitalized) $g(name)\
	  selects the "Right" side as the merge choice, but also declares the\
	  region as a <bld>collision</bld> which requires user assistance to solve,\
	  highlighting it appropriately to draw it to your attention.
	  As a further reminder, it will also highlight within the dropdown list\
	  of diff regions on the toolbar, which can thus be used to quickly locate\
	  these problematic areas, simply by scrolling throught the list looking\
	  for the highlighted items.
	  Despite all automatic attempts to choose the proper merge choice,\
	  $g(name) does not and can not, itself, <itl>resolve</itl> arbitrary\
	  collisions. However, as it turns out, the <btn>Split</btn> tool, by\
	  repartitioning the region into distinct smaller regions, can often be\
	  used to <bld>resolve</bld> what we call <itl>simple</itl> collisions by\
	  ensuring only one side of each split portion carries markers from\
	  a single side (if possible). At such time, $g(name) will re-assign the\
	  affected merge choices appropriately, possibly eliminating the entire\
	  collision altogether.
	  Because of this ability to remove a collision through direct user\
	  interaction using <btn>Split</btn>, $g(name) will <itl>also presume</itl>\
	  that independently choosing any <itl>manually</itl> selected merge\
	  choice, when dealing with a collision region is trying to\
	  accomplish the same goal, and <bld>will remove</bld> the primary\
	  indications (highlighting) of the collision, <itl>provided</itl> you\
	  agree via a popup question. Yet note however, that the responsibility in\
	  that case, is yours; $g(name) has no additional means to actually\
	  determine if the collision was truely resolved. Note that "resolved"\
	  regions are only ever <itl>de-highlighted</itl> from the Left and Right\
	  windows; the toolbar diff region dropdown list ALWAYS retains which\
	  regions were formerly collisions <itl>unless</itl> the region was fully\
	  resolved via the <btn>Split</btn> tool.
	  Finally, remember that like all "adjustments" done after having run a\
	  Diff, all of it is <itl>entirely</itl> transitory until the Merge output\
	  file is generated, or another Diff is invoked, by any means.

<hdr>Original Author</hdr>
John M. Klassa

<hdr>Comments</hdr>
Questions and comments may be sent to the TkDiff mailing list at
	  tkdiff-discuss@lists.sourceforge.net.
Or directly into the Discussion forum at
	  https://sourceforge.net/p/tkdiff/discussion
	}

	#  Just hand newDiff Dialog only its PIECE of the larger description ??
	if {$mode=="newDiff"} { return [subst -nobackslashes -nocommands $topic2] }

	do-text-info .help $title \
			[subst -nobackslashes -nocommands [append $topic1 $topic2 $topic3]]
}

###############################################################################
# display help on the preferences
###############################################################################
proc help-prefs {} {
	global g pref

	set title "$g(name) Preferences"
	set text {
<hdr>Overview</hdr>

Preferences control almost everything within $g(name): colors, fonts, what to\
	  highlight, algorithmic strategies, and even what information should be\
	  displayed (or not). $g(name) has a complete set of builtin values for\
	  it all, a builtin editor to modify them for the current session,\
	  <bld>and</bld> a way to PRESERVE the current values as a <bld>set</bld>\
	  into a file for use in <itl>future</itl> sessions:
		  The Preference file.
This file is automatically searched and (when found) loaded, every time\
	  $g(name) is invoked; and indeed even <itl>WHERE</itl> $g(name) will\
	  <itl>look</itl> for it is subject to your control.

Preferences are located (by default) in your home directory (identified by the\
	  environment variable <cmp>HOME</cmp>.) If this variable is not set, the\
	  platform-specific variant of "/" (the system ROOT directory) will be\
	  attempted, although its more than likely you will be unable to SAVE\
	  <itl>anything</itl> to that location based entirely on write permissions.
	  If you are on a Windows platform the default file NAME will\
	  be "<cmp>_tkdiff.rc</cmp>" and will be given the attribute "hidden".\
	  For any other platform this name is "<cmp>.tkdiffrc</cmp>".
	  You may override the name AND location of this file by setting the\
	  environment variable <cmp>TKDIFFRC</cmp> to whatever filepath you wish.\
	  But thats not the end of your choices. While the description thus far\
	  has depicted the location of what is <itl>PRESUMED</itl> to be a file,\
	  and represents the <bld>default</bld> actions $g(name) will take on\
	  startup, the situation can be altered if that targeted location is,\
	  in fact, a <itl>Directory</itl> (except in the system ROOT variant).

In this alternate case, $g(name) will look WITHIN that directory for the\
	  <bld>same</bld> base-name used to originally find the directory\
	  <bld>UNLESS</bld> a different name was supplied on the command line via\
	  the "<bld>-P</bld> <itl>filename</itl>" option. This permits you to have\
	  more than just the SINGLE set of preferences (YOUR default), should you\
	  need different settings when it comes to which SCM system to use, or the\
	  TAB-size for the files being examined, etc., etc. Note however, $g(name)\
	  WILL NOT create this target directory itself. That is YOUR task to\
	  configure. Without it, all you will have is a single Preference file,\
	  and the command line option (<bld>-P</bld> <itl>filename</itl>) will\
	  become ignored <itl>for this purpose</itl>; yet remember that options\
	  given on the command line <bld>NOT</bld> RECOGNIZED as belonging to\
	  $g(name) are <itl>automatically</itl> thought to belong to Diff itself.
	  IMPORTANT: This can easily lead to invalid syntactic commands, and failed\
	  operations - in particular, any attempt to pass a PreferenceFilename when\
	  a directory is <bld>NOT</bld> in use will certainly fail!
	  Lastly, please note that the "<itl>filename</itl>" in this context is\
	  NOT permitted to contain "spaces", regardless of being quoted or NOT on\
	  the command line.

As hinted earlier, you may view, edit, locally apply and even save preferences\
	  (into whichever file was designated per above) from a provided dialog\
	  accessible via the <btn>Edit</btn> menu <btn>Preferences...</btn> item;\
	  note that the dialog titlebar will specify the individual Preference\
	  filename that will be subject to updating should you <btn>Save</btn> any\
	  changes.
	  It is necessary to actually <btn>Apply</btn> any changes\
	  <itl>before</itl> attempting to <btn>Save</btn> them, as saving to the\
	  preferences file will <bld>only save</bld> the <itl>current</itl>\
	  setting values, and not those that may have been editted, but not yet\
	  applied. Should any individual setting be deemed unworkable, its\
	  <bld>prior</bld> value will most often be reverted AND a popup message\
	  produced. <btn>Dismiss</btn>, besides removing the dialog will also\
	  attempt to <itl>CANCEL</itl> any edits made after the most recent\
	  <btn>Apply</btn>. You will be asked for confirmation to proceed in this\
	  case, to give you the opportunity to <btn>Apply</btn> them before\
	  <bld>losing</bld> them completely.
	  There is one small side-effect of <btn>Apply</btn>ing the preferences,\
	  and that is an unavoidable "re-balancing" of the <btn>Grip</btn> that is\
	  used for apportioning horizontal space among the Left and Right windows.
	  Be aware though, that <itl>certain</itl> preferences, when subsequently\
	  <btn>Apply</btn>ed, will cause $g(name) to immediately re-invoke "Diff",\
	  which will <bld>destroy</bld> any <itl>UN-SAVED</itl> interactive work\
	  (specifically any merge choices, "Ignore"d or "Split/Combine"d CDRs)\
	  to their initial states.

The following descriptions will, among other things, identify which of them\
	  can exhibit this behavior; yet the characteristic each has in common\
	  is that they somehow involve either the "<itl>formulation</itl>" of the\
	  Diff command itself, or the "<itl>interpretation</itl>" of its\
	  <bld>results</bld> (eg. blank-handling, ignores, etc.). To remind you\
	  of the potential penalty involved, whenever a preference modification is\
	  made that would <bld>lead</bld> to an inevitable Diff INVOCATION,\
	  both individual items AND the <btn>Apply</btn> button will become\
	  highlighted. You may, of course, proceed at your own risk. However, as a\
	  safety net, should an <btn>Apply</btn> happen to encounter a "reverted"\
	  setting due to problems and ALSO note the need to <btn>Rediff</btn>,\
	  $g(name) will <bld>suspend</bld> that need and instead allow you to\
	  <itl>repair</itl> the failed settings. Besides a popup message to that\
	  effect, the <btn>Apply</btn> button itself will flash RED (briefly) upon\
	  detecting ANY errors (for which you will have already been notified).\
	  Conversely, if applying all settings has been successful, the button\
	  flash will be GREEN, in addition to any <itl>possibly</itl> visible\
	  changes you may observe in the main windows from your new settings.

Preferences are organized onscreen into FIVE categories: General, Display,\
	  Behavior, Appearance and Engine. Yet, in the resulting file, they are\
	  kept in alphabetical order of the preference identifier key. But EACH\
	  will have the same descriptive labels (on screen, in this Help, or as a\
	  comment in the file). For the purposes here, they will be presented in\
	  their onscreen grouping and order.

<hdr>General</hdr>

<bld>$pref(diffcmd)</bld>

This is the REAL command that will be run to create a diff of any two files.\
	  It is NOT a modifiable preference, at least not directly; It simply\
	  displays HOW the command is presently configured, to which $g(name) will\
	  append some pair of files. However, its "<itl>content</itl>" will respond\
	  to modifications made to OTHER pertinent preferences, including several\
	  that may reside in other sections of the dialog. As originally delivered,\
	  this will typically be "diff"; yet other differencing engines, providing\
	  other algorithms are possible.
As this IS the prototype command $g(name) intends to invoke, when any implied\
	  modification occurs from manipulating OTHER preferences, the NEW value\
	  will be immediately highlighted, indicating that <btn>Apply</btn> WILL\
	  cause the newly configured command to be invoked. Should you need to\
	  review what the PRESENT value was BEFORE it was changed an hilighted,\
	  hovering the mouse over the item displays that value in the Status bar.

<bld>$pref(xcludeFils)</bld>

Whenever $g(name) is looking for file pairings within the file system, it first\
	  has to form candidate names, generally because the arguments it is\
	  working with are directories. This preference names one or more filename\
	  <itl>patterns</itl> which should NOT be considered for matching to produce\
	  a pairing. Simply <itl>seperate</itl> each pattern with at least\
	  one space, which implies the pattern itself CANNOT contain one. Note that\
	  it doesn't matter if the pattern matches an actual FILE or any other NAMED\
	  entity (eg. a directory) during the search - it is immediately skipped.
	  Provides some level of control over what files $g(name) SHOULD find.

<bld>$pref(tmpdir)</bld>

The name of a directory for files that are temporarily created while $g(name)\
	  is running. This value is initially obtained from a somewhat platform\
	  dependent environment variable: Windows uses TEMP; and others, TMPDIR.\
	  However, the MacOS program-launcher ("Finder"), being a system-level\
	  tool, generally sets the TMPDIR variable with its OWN path (NOT a value\
	  suitable for the user/tool being launched); thus $g(name) initially will\
	  simply default to "/tmp" on that platform. You, of course, may override.

<bld>$pref(editor)</bld>

The name of an external editor program to use when editing a file (ie: when\
	  you select "Edit" from the popup menu). If this value is empty, a\
	  simple editor built in to $g(name) will be used, and will be positioned\
	  such that the current diff is visible. Windows users might want to set\
	  this to "notepad". Unix users may want to set this to
	  "xterm -e vi"	  or perhaps "gnuclient".
When run, the name of the file to edit will be appended as the last argument\
	  on the command line. Alternately, if the supplied value contains the\
	  string "\$file" (without the quotes), it's treated as a complete\
	  external command line, allowing any additional legal syntax, where the\
	  following parameters can be used:
	  \$file: the file of the window you invoked upon
	  \$line: the starting line of the current diff
For example, in the case of NEdit or Emacs you could use
	  "nc -line \$line \$file"	 and
	  "emacs +\$line \$file"   respectively.
Or for VI, perhaps something like
	  "xterm -e vi +:set\\\\ nu +\$line \$file"	  which opens VI in a separate\
	  Xterm window, loads the file at the designated CDR line AND causes line\
	  numbering within VI to be turned "on".

<bld>$pref(ignoreRegexLnopt)</bld>

An editable dropdown list of Regular Expressions that are used to identify text\
	  lines that should be ignored/suppressed (when possible, and activated)\
	  thereby eliminating them from being displayed/highlighted <itl>AS</itl>\
	  real Diff regions. But you must be <bld>very cautious</bld> when forming\
	  such Regular Expressions, so as to NOT IDENTIFY a line that might have\
	  <itl>OTHER legitimate</itl> differences on it.
	  Initially, the item will display nothing except its dropdown arrow.\
	  To view the existing list, simply click the dropdown arrow, and scroll\
	  thru the resulting list. Clicking on an <bld>entry</bld> of that list,\
	  is a request to <bld>delete</bld> it, but you will be asked for\
	  confirmation first, which you may decline.
	  However, declining conveniently PLACES that entry into the originally\
	  empty dropdown entry box, where you may then <bld>edit</bld> it by\
	  <itl>first</itl> clicking on it (to remove the selection highlight) and\
	  then using the keyboard to traverse about the entry (arrows, backspace,\
	  retyping) until satisfied, whereupon pressing [Return] will\
	  <bld>add</bld> it <itl>as a new value</itl> (<bld>not</bld> as an edit\
	  to the previous entry). Note that <itl>shifting</itl> the current focus\
	  <itl>away</itl> via a mouse click elsewhere, or pressing [Tab],\
	  <bld>also counts</bld> as a [Return], confirming your edit completion.\
	  Obviously if this happens prematurely, you only need delete it and\
	  try again.
	  If instead you simply start typing first, either AFTER a declined\
	  deletion, or from the initial empty display state, you will directly\
	  <bld>add</bld> whatever is typed after pressing [Return].
	  Nevertheless, confirmation of each "add" or "delete" will be flashed\
	  momentarily whenever the list is actually modifed (and the entry will\
	  then be returned to its empty-looking initial state).
	  If the entire item is shown <itl>disabled</itl>, it can be accessed by\
	  toggling the "<itl>$pref(ignoreRegexLn)</itl>" option described\
	  (shortly) below.
	  However, note that IF that option *remains* "set", any changes made to\
	  this table qualifies as a REASON to re-invoke a new Diff, and thus will\
	  result in the warning highlight, and the concommitant concerns about\
	  in-progress work.

<bld>$pref(filetypes)</bld>

Another editable dropdown list, consisting this time of file suffixes you may\
	  wish to use as filters in the various file open and save dialogs\
	  throughout the tool. Editting procedures are as described immediately\
	  above, except that the format is that of two "words" separated by white\
	  space. The first word is used as a label, and if it contains spacing,\
	  should be enclosed in <bld>{braces}</bld>. The second is a file-glob\
	  pattern depicting applicable file extension you wish to see. Thus entries\
	  like "All *" or "{Text Files} *.txt" or even "{C Files} *.[cChH]" should\
	  all be self explanatory. For sanitys sake, it is best to keep the\
	  labels short!

<bld>$pref(geometry)</bld>

This defines the <itl>default</itl> size, in characters, of the two main text\
	  windows. The format must be <cmp>WIDTHxHEIGHT</cmp>. For example, "80x40".
	  However, note that while $g(name) will TRY to honor this request, if it\
	  would result in the overall tool attempting to display as LARGER than the\
	  screen size of your monitor, the actual values used may be trimmed back\
	  to fit. While various realities (this setting, Font size, your Monitor\
	  resolution) may all affect the <bld>initial</bld> tool display, once\
	  completed, you are free to resize the tool as you desire, including\
	  making it <itl>larger</itl> than your screen; although doing so may make\
	  general operations more difficult.

<bld>$pref(ignSuprs)</bld>

If <bld>set</bld>, then whichever of the Engine-defined Suppressions are\
	  presently marked "active" are ADDED into the prototype "$pref(diffcmd)",\
	  obviously inducing that command to be changed, and (again) raising the\
	  issue of an impending "Diff" invocation and its warning highlight. This\
	  preference serves simply as a ON/OFF toggle to the suppression categories\
	  as a group, eliminating the need to toggle several items to accomplish\
	  the same result. Turning OFF all suppressions is an important step when\
	  making final merge choices, if INDENTATION in the final merged file is\
	  of any importance to you.

<bld>$pref(autocenter)</bld>

If <bld>set</bld>, whenever a new diff region becomes the CDR (for example,\
	  when pressing the <btn>Next</btn> or <btn>Prev</btn> buttons), the\
	  diff region will be automatically centered on the screen.
If <bld>unset</bld>, no automatic centering will occur. However, the setting\
	  <itl>may also be ignored</itl> in the unique situation where\
	  "autoselection" (described shortly) is <bld>also set</bld> and the\
	  display is already being PHYSICALLY scrolled. Stated differently:\
	  auto-centering will not "fight" the user over who gets to position the\
	  text window content.

<bld>$pref(ignoreEmptyLn)</bld>

If <bld>set</bld>, then $g(name) will not count, nor highlight, any region\
	  that is exclusively comprised of empty (or possibly white space filled\
	  lines if the above "<itl>$pref(ignSuprs)</itl>" is active)\
	  whenever a diff is executed. This essentially mimics a feature of the\
	  original Diff program, but is performed entirely within $g(name).
If <bld>unset</bld>, no special significance is attached to blank/empty lines\
	  and $g(name) will report the regions as Diff reports them.
Note if you press <bld>Apply</bld> AFTER changing this setting (either to\
	  <bld>set</bld> <itl>or</itl> <bld>unset</bld>), it will trigger an\
	  immediate "<btn>Rediff</btn>" which <bld>WILL DISCARD</bld>\
	  any transitory activity not yet finalized. As such, it is yet another\
	  instance of the warning highlight that precedes such action.
Also note that when you choose to ignore empty lines, you are implicitly\
	  saying that those affected lines <itl>will be retained</itl> in any\
	  merged output <bld>exactly</bld> as they appearred originally in the\
	  Left-hand text window. Note that for $g(name) to permit the DR to\
	  <bld>be ignored</bld>, <itl>every line</itl> must be classified as such\
	  <itl>regardless</itl> of the specific reason (ie. based on being BLANK,\
	  or as a result of matching any of the "<itl>$pref(ignoreRegexLnopt)</itl>"\
	  described earlier, provided THEY are active).

<bld>$pref(autoselect)</bld>

If <bld>set</bld>, automatically select the visible diff region nearest to the\
	  middle of the text window when scrolling.
If <bld>unset</bld>, the current diff region will not change during scrolling.
This only takes effect if "<itl>$pref(syncscroll)</itl>" is <bld>set</bld>,\
	  thus can be thought of as a "modifier" for that setting.

<bld>$pref(ignoreRegexLn)</bld>

If <bld>set</bld>, then the above "<itl>$pref(ignoreRegexLnopt)</itl>" will\
	  participate whenever a diff is executed. It also permits that option\
	  to be editted.
If <bld>unset</bld>, that same option will <itl>not</itl> participate\
	  in any invoked diff and is also disabled from being modified.
You may toggle this setting simply to gain editting access to the\
	  "<itl>$pref(ignoreRegexLnopt)</itl>", but if you press <bld>Apply</bld>\
	  BEFORE toggling BACK to the original value (be it <bld>set</bld>\
	  <itl>or</itl> <bld>unset</bld>), it will trigger an immediate\
	  "<btn>Rediff</btn>" which <bld>WILL DISCARD</bld> any\
	  transitory activity not yet finalized.
Conversely, if you <bld>set</bld> this, but the list of REs is empty at the\
	  time of the "Apply", this setting will simply revert to <bld>unset</bld>\
	  without error.
Again, note that when you choose to ignore matched lines, you are implicitly\
	  saying that those affected lines <itl>will be retained</itl> in any\
	  merged output <bld>exactly</bld> as they appeared originally in the Left\
	  text window. Also, as before, for $g(name) to permit the DR to\
	  <bld>be ignored</bld>, <itl>every line</itl> must be classified as such\
	  <itl>regardless</itl> of the specific reason (ie. based on being BLANK,\
	  or as a result of matching any ONE expression).

<bld>$pref(autoSrch)</bld>

When <bld>set</bld>, $g(name) will automatically initiate, at any tool startup\
	 that does <itl>NOT</itl> provide a <cmp>FSPEC</cmp>, an attempt to\
	 query a detected, preferred and capable SCM for files that it claims have\
	 differences. While this capability is always available at startup when at\
	 least <itl>one</itl> <cmp>REV</cmp> is provided, this setting\
	 <bld>overrides</bld> the normal behavior of $g(name) to produce the\
	 <btn>New...</btn> Diff dialog, when <itl>zero</itl> arguments are provided.
	 Note that for most capable SCMs to be detected in this fashion, the\
	 <bld>C</bld>urrent <bld>W</bld>orking <bld>D</bld>irectory (CWD) for\
	 $g(name) needs to be inside the actual "Working Copy" (WC) set of files\
	 the particular SCM controls.
	 When the choice is <bld>unset</bld>, normal dialog behavior is restored.\
	 This setting is mostly a convenience for users that find themselves\
	 actively persuing merge resolutions in a SCM-controlled environment on a\
	 day-to-day basis.

<bld>$pref(syncscroll)</bld>

If <bld>set</bld>, scrolling either text window will result in both windows\
	  scrolling.
If <bld>unset</bld>, the windows will scroll independent of each other.
Note that this setting has only a <itl>limited effect</itl> on the Merge\
	  Preview window contents, in that <itl>changes</itl> of the CDR will\
	  "jump" scroll, but direct interactive scrolling will not (see the\
	  <btn>Help</btn> topic "<btn>On GUI</btn>" for more details).
	  In addition, there are various functions within $g(name) where it is\
	  either impossible, or impractical to maintain strict adherence to this\
	  setting. For features that <bld>require</bld> the windows to become\
	  "fractured", $g(name) will strive to reinstate the requested synchronous\
	  behavior as quickly as permitted as those operations are completed.\
	  this can OCCASIONALLY cause the windows to be "over eager" to comply,\
	  based on certain combinations of other related settings, such as\
	  "<itl>$pref(autoselect)</itl>" and "<itl>$pref(autocenter)</itl>".\
	  These do not tend to produce errors, quite so much as confusion.

<bld>$pref(scmPrefer)</bld>

This setting is actually a pair of values, describing which SCM you prefer to\
	  utilize for EACH of the two possible sides of the comparison. Initially\
	  both values default to "Auto" which produces the classical $g(name)\
	  behavior of the "first-detected" SCM possible (based on an internal\
	  precedence list of known SCM systems). By choosing a <itl>specific</itl>\
	  SCM system, you are effectively <itl>overriding</itl> that internal list\
	  <bld>provided</bld> the chosen value is <itl>still detectable</itl>.\
	  If not, then the behavior reverts to the classical norm whenever that\
	  <itl>side</itl> requires the use of an SCM.
	  There is also a possible value of 'None', if you believe that\
	  <itl>NO</itl> SCM should be involved for that side, but be aware that\
	  such a setting may interfere with the ability to query an otherwise\
	  capable SCM from providing candidates to be compared. $g(name) will try\
	  to inform you should this seem to occur, although there are other\
	  reasons, besides this, for that possibility. Note however, that because\
	  of subtle differences between the two ways of starting the tool: the\
	  command line or the dialog, 'None' is generally <itl>ignored</itl> for\
	  command line uses, while it will behave as a <itl>override</itl> value\
	  on the dialog, until the dialog is actually accepted.

<bld>$pref(predomMrg)</bld>

This setting decides, for those cases where no specific reason (such as an\
	  implied choice from a 3way ancestor diff) exists, which of the two sides\
	  <bld>Left</bld> or <bld>Right</bld>, should be <itl>initialized</itl>\
	  as contributing its portion of the changed lines to the eventual merge\
	  result.
	  Determining how best to toggle this setting involves not only the order\
	  of files as provided initially, but also on the specific goals\
	  envisioned by the user for the merge as a whole. For example, if\
	  back-porting some specific capability, it might be best to select the\
	  side of the older file, and then only interactively merge the needed\
	  individual regions from the newer one.
	  This option most often comes into play when a Diff is invoked,\
	  although it will also apply when <btn>Split</btn> or <btn>Combine</btn>\
	  is used and there was no <itl>other</itl> reason to choose a side,\
	  as every region must ultimately posess <itl>SOME</itl> setting prior to\
	  being displayed.
	  As a reminder of the oftentimes lopsided contributions from ONE side\
	  versus the other, a status display of how MANY merges favor the Left\
	  versus the Right is maintained near the lower right corner of the display.

<hdr>Display</hdr>

<bld>$pref(toolbarIcons)</bld>

If <bld>set</bld>, the toolbar buttons will use icons instead of text labels.
If <bld>unset</bld>, the toolbar buttons will use text labels instead of icons.
Be advised that the toolbar can be a crowded place, and that generally speaking\
	  the icon-style buttons take less space, and provide Tooltip popup\
	  descriptions in the event you can't recall what any individual graphic\
	  <itl>means</itl>.

<bld>$pref(fancyButtons)</bld>

If <bld>set</bld>, toolbar buttons will mimic the visual behavior of typical\
	  Microsoft Windows applications. Buttons will initially be flat until the\
	  cursor moves over them, at which time they will be raised.
If <bld>unset</bld>, toolbar buttons will always appear raised.
This feature is not supported in MacOSX.

<bld>$pref(showln)</bld>

If <bld>set</bld>, line numbers are displayed alongside each line of each file.
If <bld>unset</bld>, no line numbers will appear.

<bld>$pref(tagln)</bld>

If <bld>set</bld>, line numbers are highlighted with the options defined in\
	   the Appearance section of the preferences.
If <bld>unset</bld>, line numbers won\'t be highlighted.

<bld>$pref(showcbs)</bld>

If <bld>set</bld>, change bars are displayed alongside each diff region line\
	  of each file.
If <bld>unset</bld>, no change bars will appear.
The exact form of such change-bars are controlled by further preferences,\
	  described next.

<bld>$pref(tagcbs)</bld>

If <bld>set</bld>, change indicators will be highlighted. The highlighting\
	  itself is the subject of yet another preference\
	  "<itl>$pref(colorcbs)</itl>" described shortly.
If <bld>unset</bld> change indicators are simply displayed as encoded textual\
	  markers: a "+" for lines that exist in only one file; a "-" for lines\
	  that are missing from only one file, and "!" for lines that differ\
	  between the two files.

<bld>$pref(showmap)</bld>

If <bld>set</bld>, a colorized, graphical "diff map" will be displayed between\
	  the two files, showing regions that have changed. By default, Red is\
	  used to show deleted lines, Green for added lines, Blue for changed\
	  lines, and Yellow for overlapping lines during a 3-way merge. Note that\
	  any of these colors are themselves "preferences" and thus, changeable\
	  (See entries under section <bld>Appearance</bld> below).
If <bld>unset</bld>, the diff map will not be shown.

<bld>$pref(colorcbs)</bld>

If <bld>set</bld> the change bars will appear as solid colored bars\
	  that match the colors used in the diff map.
If <bld>unset</bld>, IN ADDITION to just the color bars, the change bars will\
	  display a "+" for lines that exist in only one file, a "-" for lines\
	  that are missing from only one file, and "!" for lines that differ\
	  between the two files. Due to color-on-color layering, the "!" markers\
	  may visually disappear in this situation from BOTH using the same color.

<bld>$pref(tagtext)</bld>

If <bld>set</bld>, the file contents will be highlighted with the options\
	   defined in the Appearance section of the preferences.
If <bld>unset</bld>, the file contents won't be highlighted.
Note - failure to generally highlight the text may make some functions of
	   $g(name) problematic, but the choice remains yours.

<bld>$pref(showinline1)</bld>

If <bld>set</bld>, show inline diffs in the main window. This is useful to\
	  see what the actual diffs are within a large diff region.
If <bld>unset</bld>, the inline diffs are neither computed nor shown. This\
	  is the simpler method, where byte-by-byte comparisons\
	  are used. This inline diff <itl>never</itl> honors\
	  any "<itl>$pref(ignSuprs)</itl>" value, regardless of that\
	  option being enabled; but see the following TWO preference (below) for\
	  a method to sidestep this limitation.

<bld>$pref(showinline2)</bld>

If <bld>set</bld>, show inline diffs in the main window. This is useful to see\
	  what the actual diffs are within a large diff region.
If <bld>unset</bld>, the inline diffs are neither computed nor shown. This\
	  approach is more complex, but should give more pleasing\
	  results for source code and written text files.  This is the\
	  Ratcliff/Obershelp pattern matching algorithm which recursively\
	  finds the largest common substring, and recursively repeats on the left\
	  and right remainders. When no more <itl>common</itl> substrings can be\
	  FOUND, whatever remains ARE the actual differences.

<bld>$pref(inlSuprs)</bld>

This is a collection of five distinct settings that collectively control what\
	  is NOT highlighted as <itl>different</itl> when the more complex inline\
	  mode (<itl>$pref(showinline2)</itl>) described above is used.\
	  You may <bld>set</bld> or <bld>unset</bld> any or none of them as best\
	  suits your needs, yet is itself only active (or even modifiable) when\
	  the "<itl>$pref(showinline2)</itl>" preference is marked active. Further\
	  recognize that some categories are actually subsets of one another and\
	  turning ON one, can result in OTHER PRECEDENCE items being turned OFF.
Each of the five is a <bld>suppression</bld> category that just happens to\
	  <bld>parallel</bld> the same five Suppression values that can be\
	  configured for the Diff engine.\
	  Thus if you manually <itl>ALIGN</itl> these INDIVIDUAL settings to\
	  agree, you will SEE exactly why Diff chose the line as different.
	  However, there are valid reasons to not always maintain this alignment,\
	  depending on what you are trying to understand at the moment. Many choose\
	  to perform Diffs as if BLANKS (or even character-case) do not matter. Yet\
	  one might still wish to SEE where such occurences are. The converse is\
	  also true: Diffing for EXACT matching, but VIEWING to ignore whitespace,\
	  or Capitalizations can be enlightening. The choice (and its alignment) is\
	  yours. The five suppressions are (in general precedence order):
	  <btn>Case</btn> - Capitalizations (GNU DIFF option <bld>-i</bld>)
	  <btn>Blanks</btn> - ALL Whitespace (GNU DIFF option <bld>-w</bld>)
	  <btn>#Blanks</btn> - AMOUNT of Whitespace (GNU DIFF option <bld>-b</bld>)
	  <btn>@TabX</btn> - Columnar Whitespace (GNU DIFF option <bld>-E</bld>)
	  <btn>@EOL</btn> - Whitespace at End-of-Line (GNU DIFF option <bld>-Z</bld>)
The initial (default) setting is to suppress none of these individual display\
	  categories. However, note that the "@TabX" category involves the USE of\
	  yet another preference (<itl>$pref(tabstops)</itl>), to compute the\
	  intended result of paired Whitespace that (after TAB expansion) reaches\
	  an IDENTICAL column in EACH file simultaneously, to qualify as ignored.

<bld>$pref(showlineview)</bld>

If <bld>set</bld>, display the window near the bottom of the display that\
	  shows the "current" line from <itl>each</itl> file, one above the other.\
	  Clicking on any specific line, or manuevering the text 'insert' cursor\
	  via the keyboard, in either text window selects which line to contribute.\
	  This window is most useful to do a visual byte-by-byte comparison of a\
	  line that has changed; by default, the display begins with text rendered\
	  the same as in the main display, with mismatched bytes marked with\
	  underlines, and a blue background and white foreground, but other\
	  approaches include configuring with a "constant width" font\
	  (via <itl>$pref(bytetag)</itl>) such as "Courier" to more easily spot\
	  the differences, and/or perhaps a different foreground color.
If <bld>unset</bld>, the window will not be shown.

<bld>$pref(fLMmax)</bld>

This setting controls how $g(name) presents the list of potential file pairs\
	  when more than a single pair was derived from the input parameters. As\
	  the label on the control suggests, file counts <itl>less-than (or equal\
	  to)</itl> the value selected will be made available via the menu\
	  directly. Conversely, if the file count is <itl>greater</itl>, a popup\
	  window will be used. The primary reason for this choice is that a menu\
	  can become unweildy when it contains more than a handful of items.
	  In addition, some users may prefer the ability to <itl>see</itl> the\
	  filelist at all times, via the popup window, which may be accomplished\
	  by setting the threshold to its minimum value. $g(name) defaults this\
	  value to 9 initially, but only permits a fixed maximum of 25.

<hdr>Behavior</hdr>

It is said that people who spend large blocks of time using any given tool\
	  often become what is termed "power users". As such they become so\
	  familiar with the sequence of operations, that they prefer to keep\
	  their hands on the keyboard and NOT the mouse. Nearly all operations\
	  in a tool have some keyboard equivalent to allow them to be invoked.\
	  These are called "keyboard accelerators", or simply, <bld>hotkeys</bld>.\
	  As of $g(name) V5.1, it is now possible for the user to specify what\
	  specific keys are preferred for tool functions previously provided\
	  <itl>almost exclusively</itl> via "menus" or "buttons". Those other\
	  mechanisms continue to exist, but $g(name) now allows the "power users"\
	  to select their own.

	  Each of the following items operates in exactly the same fashion. Only\
	  the task each performs is unique. Thus the following describes how to\
	  review and/or change each key-combination for any of these features.

	  Simply hovering the mouse over any given item will cause the item\
	  display to switch from its brief "task description" to showing the\
	  individual key-combination that will invoke said function.

	  To actually <itl>change</itl> that value, clicking the specific item will\
	  "<bld>arm</bld>" it to accept the\
	  <bld>very next keyboard interaction</bld> to be made. Be advised:\
	  <itl>despite an item holding the current focus highlight, you may NOT\
	  use</itl> <cmp>spacebar</cmp> (or any other platform-specific\
	  keyboard-click equivalent) <itl>to perform that click</itl>. To alert\
	  you that this very critical step has been primed, the background of the\
	  item will be changed to the "inform" (ie. the ToolTip) color. At this\
	  juncture, if you move the mouse to LEAVE the item, the edit will be\
	  cancelled. If instead you actually press some key-combination\
	  (for example: Shift Z), that combination will\
	  <bld>temporarily replace</bld> whatever combination was previously\
	  present. It will ALSO visually SHIFT that displayed value from being\
	  centered to being "left-justified" <bld>still</bld> with the "inform"\
	  background color.

	  You are now in a "textual-modify" phase, where it may be advantageous to\
	  adjust the value to something either more, or less, specific (ie. adding\
	  or removing various "modifier" keys, such as Shift, or Control, Numlock\
	  or the mostly useless descriptor "Key" - which must <bld>never</bld> be\
	  removed if the remaining value is a <bld>digit</bld>). The mouse is no\
	  longer required to stay within the item bounds at this time, but will\
	  only function in a restricted manner. You may use it to set the\
	  insert-cursor position for performing edits (just as any normal entry\
	  field). It is ALSO permitted to access <itl>certain</itl> Dialog buttons\
	  (specifically the <btn>Apply</btn> or <btn>Dismiss</btn> ones), which\
	  <bld>will</bld> finalize the edit in a manner consistent with each,\
	  as a shortcut.

	  Beyond that shortcut, only one of two choices remain; either you press\
	  the "Escape" key (to cancel the <bld>entire</bld> edit sequence),\
	  or press "Return" to confirm that the sequence is as you intended it.\
	  Either finalizes the hotkey definition process, removes the special\
	  background "inform" color, and returns you to normal operation.
	  As a convenience to the "power users", pressing [Tab] (or [Shift-Tab])\
	  will be treated as an implicit "Return". Clicking either of the two\
	  Dialog buttons (via mouse OR keyboard actions) mentioned earlier will\
	  complete the editting phase with the appropriate "cancel" or "accept"\
	  action (eg. <btn>Dismiss</btn> as a "cancel") while also immediately\
	  performing their normal task.

	  Note that while MOST keys on a keyboard CAN be specified, including\
	  keypads and function keys, SOME may have been usurped by the Operating\
	  System or Window Manager and will never even be delivered to $g(name).\
	  Many OTHERS could have some generally defined meaning with Tcl/Tk;\
	  particularly within the various textual widgets. The good news there is\
	  that because nearly all of the $g(name) widgets operate in a\
	  "display only" mode within the main tool windows (where these hotkeys\
	  exist), there is a low chance of cross connecting your choice of hotkeys\
	  with those of the widgets themselves. Just be aware that anything\
	  <itl>YOU</itl> choose that has not been incapacitated (by virtue of that\
	  "display only" widget status) <itl>would operate</itl>\
	  <bld>in addition</bld> to (and after) anything Tcl/Tk might use the same\
	  keys for, unless the Tcl/Tk definition chooses to block further actions.

	  Our recommendation, is to keep it simple and preferrably unique. Most\
	  typing keys would be available as there are few places in $g(name) where\
	  typing is possible. If you wish to create a "mode-based" family of\
	  hotkeys (eg. lots of people like the idea of using the "arrow" keys in\
	  place of the default "merge choice" keys), then perhaps pairing them\
	  with the use of "NumLock" would allow that use when desired, without\
	  sacrificing the normal arrow usage (such as moving the text insert\
	  cursor about), NOR requiring a second key (Shift, Control, etc) to be\
	  simultaneously held down.

	  We specifically require the use of the mouse in the "arming" operation,\
	  to limit mis-struck keys, at in-opportune times. Safety for the less-than\
	  "power-user" community overrides the minor inconvenience to the real\
	  ones among you, who will likely use this capability <itl>once</itl> and\
	  then, never adjust it again!

<bld>$pref(navFrst)</bld>
	  This hotkey is defaulted to "f"

<bld>$pref(navLast)</bld>
	  This hotkey is defaulted to "l"

<bld>$pref(navNext)</bld>
	  This hotkey is defaulted to "n"

<bld>$pref(navPrev)</bld>
	  This hotkey is defaulted to "p"

<bld>$pref(navCntr)</bld>
	  This hotkey is defaulted to "c"

<bld>$pref(mrgLeft)</bld>
	  This hotkey is defaulted to "Key-1"

<bld>$pref(mrgRght)</bld>
	  This hotkey is defaulted to "Key-2"

<bld>$pref(mrgLtoR)</bld>
	  This hotkey is defaulted to "Key-3"

<bld>$pref(mrgRtoL)</bld>
	  This hotkey is defaulted to "Key-4"

<bld>$pref(genEdit)</bld>
	  This hotkey is defaulted to "e"

<bld>$pref(genFind)</bld>
	  This hotkey is defaulted to "Control-f"

<bld>$pref(genNxfile)</bld>
	  This hotkey is defaulted to "Control-["

<bld>$pref(genPvfile)</bld>
	  This hotkey is defaulted to "Control-]"

<bld>$pref(genRecalc)</bld>
	  This hotkey is defaulted to "Control-r"

<bld>$pref(genXit)</bld>
	  This hotkey is defaulted to "Control-q"

The final two settings are a deviation from historical $g(name) (which never\
	  required the Control-Key modifier) as it was considered safer to require\
	  TWO fingers on the keyboard before wiping out all in-progress transitory\
	  work, for which there is no recovery, beyond reconstruction.

One other small point should be made about ALL of the Behavior settings -\
	  technically, each <bld>is platform dependent</bld>! Each platform has\
	  unique names, particularly for some of the "modifier" keys. $g(name)\
	  therefore will prefix the specific windowing system ID to each of the\
	  preference identifiers when storing and retrieving the values to the\
	  preferences file. While this was not originally intended to be a\
	  "feature", it does exhibit the unusual ability for a single preference\
	  file to be directly USABLE on multiple platforms, WITHOUT data\
	  collisions, and presuming you have a multi-platform situation all using\
	  a <itl>single</itl> preference file.
	  It also means that each platform will require its OWN specialization\
	  work, as regards which specific keys are being configured to do what.

<hdr>Appearance</hdr>

As the majority of the $g(name) content is textual, the presentation of such\
	  information is at the heart of most of the tools features. Controlling\
	  and tuning that presentation is therefore key to obtaining the best\
	  experience in using it, but only when it matches the users expectations\
	  of proper degrees of emphasis - which is highly subjective. Tcl/Tk has\
	  a remarkably rich set of attributes that can be applied, and even\
	  layered atop each other. While $g(name) makes use of many of these\
	  attributes itself, it still makes sense to allow the user to make many\
	  of these <itl>adjustments</itl> on their own.
	  While $g(name) must necessarily "OWN" the organization of the various\
	  display layers, to maintain functionality, describing such layering is\
	  important when trying to determine what "other" attributes may help the\
	  user obtain their most useful presentation. Just be aware that $g(name)\
	  utilizes <bld>several</bld> of these attributes itself, and might not\
	  operate correctly should some be arbitrarily introduced. As a general\
	  rule, colors, fonts and sizes are likely candidates for customization;\
	  with the syntax rules for their specification dictated by Tcl/Tk.

<bld>$pref(textopt)</bld>

This is a list of Tk text widget options that are applied to each of the two\
	  text windows in the main display, and the Merge Preview. If you have Tk\
	  installed on your machine (and you should) these will be documented in\
	  the "Text.n" manual page. These settings constitute the "base" layer of\
	  attributes which will be seen, unless some "higher layer" of attributes\
	  is designated as being ABOVE them. The remaining settings will describe\
	  when such a "change in layer" takes place.

<bld>$pref(difftag)</bld>

This is a list of Tk text widget tag options that are applied to all diff\
	  regions. These options have a higher priority than those for just plain\
	  text. Use this option to make diff regions stand out from regular text.

<bld>$pref(deltag)</bld>

This is a list of Tk text widget tag options that are applied to regions that\
	  have been deleted. These options have a higher priority than those for\
	  all diff regions.

<bld>$pref(instag)</bld>

This is a list of Tk text widget tag options that are applied to regions that\
	  have been inserted. These options have a higher priority than those for\
	  all diff regions.

<bld>$pref(chgtag)</bld>

This is a list of Tk text widget tag options that are applied to regions that\
	  have been changed. These options have a higher priority than those for\
	  all diff regions.

<bld>$pref(currtag)</bld>

This is a list of Tk text widget tag options that are applied to the current\
	  diff region. So, for example, if you set the forground for all diff\
	  regions to be black and set the foreground for this option to be blue,\
	  these current diff region settings (eg. foreground color) will be used.\
	  These tags have a higher priority than those for all diff regions, AND\
	  a higher priority than the change, inserted and deleted diff regions,\
	  but ONLY in the LEFT text window. In the RIGHT text window, these\
	  settings fall BELOW the individual change-category ones described.

<bld>$pref(inlinetag)</bld>

This is a list of Tk text widget tag options that are applied to differences\
	  within lines in a diff region. These tags have a higher priority than\
	  those for all diff regions, and a higher priority than the change,\
	  inserted and deleted diff regions, AND the current region.

<bld>$pref(bytetag)</bld>

This is a list of Tk text widget tag options that are applied to individual\
	  differing characters in the line view. These options <bld>do not</bld>\
	  affect the main text displays. Note that if a font specification is also\
	  included, that font will be used for <itl>ALL</itl> the characters, not\
	  just the differing ones. If you remove all settings, text will appear as\
	  it does in the main displays, with <bld>NO</bld> difference highlighting\
	  at all. Think of it as a completely independent "layering" stack.

<bld>$pref(tabstops)</bld>

This defines the number of characters for each tabstop in the main display\
	  windows. Be aware that with the ability to specify fonts, not only of\
	  the basic text display layer, but of layered individual character ranges\
	  (as happens with "inline-diff" highlighting), the mere presence of a\
	  [Tab] will not generally cause pieces of text to "align" as might\
	  ordinarily be expected. This problem gets worse when considering the use\
	  of "so called" proportional fonts. Nevertheless, the default is 8.
	  Further note that ONE of the difference suppression categories (@TabX),\
	  both the display AND the Engine instances, will utilize this value when\
	  evaluating if that suppression will be applied. As such, it is MORE than\
	  just a means to spacing of its surrounding text.


<bld>The remaining Appearance items</bld>
are all formerly internal color settings that have now been made\
	  accessible for customization. Each takes the form of a button, when\
	  hovered over by the mouse, displays the current color each uses.
	  Pressing that button will popup a color chooser dialog to make\
	  adjustments for the items (described) that the setting covers.

<bld>$pref(inform)</bld>
	  is used primarily in the production of popup ToolTip window backgrounds\
	  to help draw your attention to it (before it disappears). Other uses\
	  include a role in the editting sequence of a global hotkey definition to\
	  again draw your attention to the "<itl>sensitive state</itl>" where just\
	  <bld>touching</bld> practically ANY key on the keyboard will result in\
	  advancing the definition procedure. It also serves as the Highlight that\
	  warns of an impending "Diff" when modifying critical preferences -or-\
	  selecting certain specific items from the <btn>View</btn> menu, AND\
	  also serves as a indicator of 'existence' when entering input filenames\
	  interactively via the <btn>File</btn>-><btn>New...</btn> Dialog.\
	  The default is "Goldenrod1".

<bld>$pref(adjcdr)</bld>
	  is used exclusively by the <btn>Split</btn> or <btn>Combine</btn>\
	  features to highlight (in the text windows) the bounds of the CDR as\
	  it is being adjusted. The default is "magenta".

<bld>$pref(mapins)</bld>
	  is used by the "<bld>Diff Map</bld>" as well as Split/Combine text\
	  window feedback and potentially the highlighting of Line numbers or\
	  Changebars (if requested), to indicate something being "added". The\
	  default is "Pale Green".

<bld>$pref(mapchg)</bld>
	  is used by the "<bld>Diff Map</bld>" as well as Split/Combine text\
	  window feedback and potentially the highlighting of Line numbers or\
	  Changebars (if requested), to indicate something being "changed". The\
	  default is "Dodger Blue".

<bld>$pref(mapdel)</bld>
	  is used by the "<bld>Diff Map</bld>" as well as Split/Combine text\
	  window feedback and potentially the highlighting of Line numbers or\
	  Changebars (if requested), to indicate something being "deleted". The\
	  default is "Tomato".

<bld>$pref(mapolp)</bld>
	  is used by the "<bld>Diff Map</bld>" as well as Split/Combine text\
	  window feedback and potentially the highlighting of Line numbers or\
	  Changebars (if requested), to indicate a COLLISION between diff\
	  regions during a 3way diff. Classically this color had actually\
	  been hardcoded, let alone defaulted to "yellow".

<hdr>Engine</hdr>

As often mentioned, $g(name) was originally designed as a graphic wrapper\
	  to the UNIX-based "Diff" utility. As such, that tool served as the target\
	  of what was POSSIBLE, and thus what would comprise the general set of\
	  preferences to be parameterized. Yet several other "work-alike" Engines,\
	  some with exceptional capabilities have come to exist since.
	  The following preferences, unlike most others, are intended to DEFINE the\
	  interface to a <bld>generic</bld> external differencing tool that\
	  $g(name) invokes when it needs to compare file pairs. It is divided into\
	  two nearly identical descriptor groups, corresponding to the two basic\
	  services $g(name) requires of its external engine:
	  1. a command to generate the actual Diff hunks themselves; and
	  2. a command to recursively FIND pairs of files HAVING differences.

As delivered, these commands are, in fact, invocations of the SAME underlying\
	  tool (eg. Diff), though they need NOT be. Yet, in practice, many engines\
	  simply require an extra flag option (or two) to accomplish the\
	  recursive service named above. As each of these represents an\
	  <itl>external</itl> command each is identified by the FIRST WORD of\
	  their values. It is expected that such commands will be "found" by the\
	  operating system using this "name"-word alone (typically by searching\
	  for it wherever other such commands would be found, for instance via\
	  the users <cmp>PATH</cmp> environment variable).

	  Associated with each command is a group of option flags that indicate\
	  specific features $g(name) is prepared to utilize <bld>IFF</bld> the\
	  engine itself HAS such a capability. Each option is presented as a\
	  text field where the LITERAL flag for the described option is SPECIFIED,\
	  together with a checkbox toggle item to designate if that option should\
	  be USED, when $g(name) requests either of the two services.
	  However - when BOTH services have IDENTICAL "names" (ie. First word),\
	  then only ONE SET of associated options is presented for configuration\
	  (as they are actually the SAME underlying tool). Just recognize that the\
	  options of "<itl>$pref(egnCmd)</itl>" <bld>will be shared</bld> by the\
	  "<itl>$pref(egnSrchCmd)</itl>" service when invoked.
	  Conversely, if that first "name"-word of the two commands differ, then\
	  EACH command will provide its OWN group of options, suitable for\
	  configuring, what is therefore DISTINCT external commands.

	  From a configuration standpoint, these settings all operate the same\
	  way. Each is actually a PAIR of settings: a toggle (to say please USE\
	  this setting) and a text-field to DEFINE the literal flag it requires.\
	  Most flags are NOT designed to accept a "value", although if such a\
	  value can be CONCATENATED to the flag, it would work. For those few\
	  flags that EXPECT to have a value (identified below when possible) there\
	  is an special encoding that can be given if it is REQUIRED that the flag\
	  be seperated from its value: simply PREFIX the flag with a single blank\
	  and $g(name) will then format the flag AND value as desired.

<bld>$pref(egnCmd)</bld>

This is quite simply the "Diff" command to be used. It is the SOURCE of what\
	  is eventually used to form the "<itl>$pref(diffcmd)</itl>" preference.\
	  It is HERE that one could supply any "extra" option flags if, with the\
	  intent of causing a NEW prototype Diff command to be formed, thereby\
	  CAUSING any <itl>temporary pass-thru</itl> flags from the original\
	  command line to be eliminated.

	  Subordinate to this basic command, are the following KNOWN option flags:

	  <bld>$pref(egnCase)</bld>
	  <bld>$pref(eopCase)</bld>		(default <bld>-i</bld>)
	  Ignores differences related to capitalizations.

	  <bld>$pref(egnBlanks)</bld>
	  <bld>$pref(eopBlanks)</bld>	  (default <bld>-w</bld>)
	  Ignores differences relating to ANY form of Whitespace.

	  <bld>$pref(egn#Blanks)</bld>
	  <bld>$pref(eop#Blanks)</bld>	   (default <bld>-b</bld>)
	  Ignores differences related to Whitespace of non-identical length.

	  <bld>$pref(egn@TabX)</bld>
	  <bld>$pref(eop@TabX)</bld>	 (default <bld>-E</bld>)
	  Ignores differences related to Tab-expansion to a COMMON location.

	  <bld>$pref(egn@EOL)</bld>
	  <bld>$pref(eop@EOL)</bld>		(default <bld>-Z</bld>)
	  Ignores differences related to Whitespace found at an end-of-line.

	  <bld>$pref(egnTabSiz)</bld>
	  <bld>$pref(eopTabSiz)</bld>	  (default <bld>--tabsize</bld>)
	  This defines the option to inform Diff of the WIDTH of a single [Tab].
	  The actual VALUE passed is defined by the "<itl>$pref(tabstops)</itl>"
	  preference, within the <btn>Appearance</btn> tabbed section.


<bld>$pref(egnSrchCmd)</bld>

The command (with likely additional flags) $g(name) uses to search a given\
	  pair of directory <bld>trees</bld> recursively to locate all pairs of\
	  resultant files that contain differences. Each pair will be expected to\
	  be named identically as the search proceeds, except for the starting\
	  directories.

	  $g(name) choosing to USE this service originates from either the command\
	  line (or the interactive dialog), and will only be effective when\
	  <bld>both</bld> given inputs <itl>ARE</itl> directories, AND the proper\
	  recursion-authorization option has been supplied (via dialog OR cmdline).
	  However, in terms of its <itl>PRECEDENCE</itl>, using a directory pairing\
	  to recursively locate files only applies when <bld>no SCM</bld> is\
	  involved. This <itl>MAY</itl> result in complaints of\
	  "<itl>no files found</itl>" in certain circumstances; yet IF the SCM\
	  access were to be DISABLED (this <bld>IS</bld> why the value "None"\
	  exists as a choice in the "<itl>$pref(scmPrefer)</itl>" setting),\
	  the recursive search WOULD then work.

	  The default value provided (diff <bld>-r</bld>) is somewhat better than\
	  the former default (which also included the <bld>-q</bld> flag). Both\
	  are reasonable, but not always ideal. First they <itl>presume</itl>\
	  the use of a GNU-like Diff Engine (to be understood to mean) "recursive"\
	  and "quiet" respectively. But the use of the "quiet" option implies how\
	  and what is eventually reported by the search. Using it, <itl>ALL</itl>\
	  like-named files are reported, <itl>even when they do not appear to be\
	  Text files</itl>. This means your resulting file list can contain files\
	  that are KNOWN to be different, despite not being files that can be\
	  readily compared or visually read, let alone reviewed, merged, etc.\
	  Leaving the "quiet" option OUT, effectively results in Diff\
	  <itl>SUPPRESSING</itl> any files that are considered (by the GNU Diff\
	  Engine, at least) to be <bld>binary</bld> (eg. non-text) from being\
	  returned as candidates.
	  While this may be a better result, it is possibly still not perfect.\
	  There are instances, of seemingly "Text-like" files, for which $g(name)\
	  might still be inappropriate. A Cscope database, for one, comes to mind.

	  Thus you may wish to <bld>use</bld> yet <itl>more</itl> options here\
	  (such as the Gnu Diff <bld>-x</bld> or <bld>-X</bld> options) that tell\
	  the Engine to <bld>ignore</bld> specified filename patterns when\
	  searching for file pairs. The first of these is provided as a possible\
	  (and value-passing) built-in configuration option (shown below). The\
	  values it would pass come from the "<itl>$pref(xcludeFils)</itl>"\
	  preference defined under the <btn>General</btn> tab, which is the\
	  <bld>same</bld> list of excluded file patterns used by any\
	  NON-recursive searches done bu $g(name).

	  <bld>$pref(egnXcludFil)</bld>
	  <bld>$pref(eopXcludFil)</bld>		(default <bld>-x</bld>)
	  This defines a REPEATING option to inform Diff what filename patterns
	  to ignore. The VALUEs passed are defined by the preference
	  "<itl>$pref(xcludeFils)</itl>", from the <btn>General</btn> section.

	  As mentioned earlier, should the first word of the two command names\
	  DIFFER, an entire SECOND SET of suppression flag definition and use\
	  toggles will be made available for configuration. For completeness\
	  sake, we note their names here, but each is defaulted and operates\
	  EXACTLY as described earlier, but EXCLUSIVELY for recursive searching:

	  <bld>$pref(egnSCase)</bld>
	  <bld>$pref(eopSCase)</bld>	 (default <bld>-i</bld>)
	  Ignores differences related to capitalizations.

	  <bld>$pref(egnSBlanks)</bld>
	  <bld>$pref(eopSBlanks)</bld>	   (default <bld>-w</bld>)
	  Ignores differences relating to ANY form of Whitespace.

	  <bld>$pref(egnS#Blanks)</bld>
	  <bld>$pref(eopS#Blanks)</bld>		(default <bld>-b</bld>)
	  Ignores differences related to Whitespace of non-identical length.

	  <bld>$pref(egnS@TabX)</bld>
	  <bld>$pref(eopS@TabX)</bld>	  (default <bld>-E</bld>)
	  Ignores differences related to Tab-expansion to a COMMON location.

	  <bld>$pref(egnS@EOL)</bld>
	  <bld>$pref(eopS@EOL)</bld>	 (default <bld>-Z</bld>)
	  Ignores differences related to Whitespace found at an end-of-line.

	  <bld>$pref(egnSTabSiz)</bld>
	  <bld>$pref(eopSTabSiz)</bld>	   (default <bld>--tabsize</bld>)
	  This defines the option to inform Diff of the WIDTH of a single [Tab].

<bld>Configuration notes</bld>

	  You should review the documentation for your configured Engine to\
	  determine if other available options might assist in "getting the desired\
	  results". $g(name) does not, itself, interpret ANY of these option flags\
	  and simply sends them to the Engine when requesting either service.\
	  HOWEVER - because the handling of the results <itl>returned</itl> by the\
	  Engine drives what then happens, it <bld>is possible</bld> that YOUR\
	  chosen Engine might not "report back" its results in a syntax\
	  intelligible to $g(name), which presently understands both the "normal"\
	  AND "Unified" formats. If such is the case, contact us for assistance.

	  But if you are the adventurous sort, or cant locate a viable copy of\
	  "Diff" for your platform, (or wish to try a DIFFERENT algorithm), you\
	  could configure something like:

	  "$pref(egnCmd)" : <bld>git diff</bld>
		"$pref(egnCase)" : <bld>-i</bld>
		"$pref(egnBlanks)" : <bld>-w</bld>
		"$pref(egn#Blanks)" : <bld>-b</bld>
		"$pref(egn@TabX)" :		(leave empty - Git non-support)
		"$pref(egn@EOL)" : <bld>--ignore-space-at-eol</bld>
		"$pref(egnTabSiz)" :	(again leave this empty - Git non-support)


	  "$pref(egnSrchCmd)" : <bld>git diff --diff-format=M</bld>
		"$pref(egnXcludFil)" : (empty: dont try ":!" - didnt work)

	  We are neither GIT experts, nor actually ADVOCATING its usage, or\
	  suggesting that the above would be the ONLY way to configure for its use.\
	  This is nothing more than an EXAMPLE of configuring a DIFFERENT engine\
	  OTHER than the expected default. Git is both formidable AND ever\
	  changing; but it WAS POSSIBLE to configure it effectively.

	  Remember that if two distinct commands are chosen for the two service\
	  routines, then a secondary set of "suppression" options will be presented\
	  for configuration for the recursive search service. BOTH sets are\
	  initially defined and defaulted per the original Diff Engine. Be aware\
	  any "extra" options ADDED to the "<itl>$pref(egnCmd)</itl>" preference\
	  should likely be added to the "<itl>$pref(egnSrchCmd)</itl>" preference\
	  to ensure SEARCHES that locate files will actually FIND the files that\
	  the other service is expected to report differences within.

<hdr>Custom Settings</hdr>

There is an additional setting built-in to the Preferences file called\
	 <bld>customCode</bld> (together with a comment about not using it) that\
	 nevertheless has some simple uses. The big advantage is that, like each\
	 other setting described above, the contents of this setting <itl>IS</itl>\
	 retained automatically when modified by the $g(name) Preferences Dialog.
	 However, it can only be <itl>set</itl> or <itl>modified</itl> externally\
	 via a text editor. Still, occasionally there have been customizations of\
	 the GUI that many users found helpful that are often difficult (if not\
	 impossible) to specify correctly using other means. Although there are\
	 fewer at the moment (per the newer 'color' buttons described above), we\
	 offer up the following (still valid) possibilit(y/ies) as suggestions:

	1. Highlighting the current Merge Choice (when in Icon mode) -
This item, typically required the use of XResources in the past to do\
	correctly, but the following is much simpler:
<cmp>
				set w(selcolor) orange
				cfg-toolbar
</cmp>
makes it easier to see which of the four icons is presently "selected", as\
	the default is generally only a greyed background shading of the\
	unselected state. Note that the command "set" and name "w(selcolor)"\
	must be exactly as shown (using parenthesis). More importantly, the\
	<cmp>cfg-toolbar</cmp> function call is REQUIRED for it to have effect!

CAVEAT: Doing more than this requires intimate knowledge of the internal\
	code, and, as such, could be subject to future elimination or even\
	promotion to a full fledged REAL 'Preference' setting. But for now,\
	it works. Moreover, the admonishment to not misuse this facility still\
	applies, as it is exceedingly easy to disrupt normal program operation.
	}

	# since we have embedded references to the preference labels in
	# the text, we need to perform substitutions. Because of this, if
	# you edit the above text, be sure to properly escape any dollar
	# signs that are not meant to be treated as a variable reference

	do-text-info .help-prefs $title [subst -nocommands $text]
}

######################################################################
#
# text formatting routines derived from Klondike
# Reproduced here with permission from their author.
#
# Copyright (C) 1993,1994 by John Heidemann <johnh@ficus.cs.ucla.edu>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#	 notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#	 notice, this list of conditions and the following disclaimer in the
#	 documentation and/or other materials provided with the distribution.
# 3. The name of John Heidemann may not be used to endorse or promote products
#	 derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY JOHN HEIDEMANN ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL JOHN HEIDEMANN BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#
######################################################################
proc put-text {tw txt} {

	$tw configure -font {Fixed 12}

	$tw configure -font -*-Times-Medium-R-Normal-*-14-*

	$tw tag configure bld -font -*-Times-Bold-R-Normal-*-14-*
	$tw tag configure cmp -font -*-Courier-Medium-R-Normal-*-12-*
	$tw tag configure hdr -font -*-Helvetica-Bold-R-Normal-*-16-* -underline 1
	$tw tag configure itl -font -*-Times-Medium-I-Normal-*-14-*
	$tw tag configure ttl -font -*-Helvetica-Bold-R-Normal-*-18-*
	#$tw tag configure h3 -font -*-Helvetica-Bold-R-Normal-*-14-*
	$tw tag configure btn -foreground white -background grey


	$tw mark set insert 0.0

	set t $txt

	while {[regexp -indices {<([^@>]*)>} $t match inds] == 1} {

		set start [lindex $inds 0]
		set end [lindex $inds 1]
		set keyword [string range $t $start $end]

		set oldend [$tw index end]

		$tw insert end [string range $t 0 [expr {$start - 2}]]

		purge-all-tags $tw $oldend insert

		if {[string range $keyword 0 0] == "/"} {
			set keyword [string trimleft $keyword "/"]
			if {[info exists tags($keyword)] == 0} {
				error "end tag $keyword without beginning"
			}
			$tw tag add $keyword $tags($keyword) insert
			unset tags($keyword)
		} else {
			if {[info exists tags($keyword)] == 1} {
				error "nesting of begin tag $keyword"
			}
			set tags($keyword) [$tw index insert]
		}

		set t [string range $t [expr {$end + 2}] end]
	}

	set oldend [$tw index end]
	$tw insert end $t
	purge-all-tags $tw $oldend insert
}

proc purge-all-tags {w start end} {
	foreach tag [$w tag names $start] {
		$w tag remove $tag $start $end
	}
}

##############################################################################
# Given 'namespace' changes of TclTk V9.0
# is SAFEST to HALT any further Dbg injections HERE
#	(if they were ever ACTIVATED in the first place)
##############################################################################
if {[string length [info commands "proc_"]]} {
	rename "proc" "" ; rename "proc_" "proc" }

# Copyright (c) 1998-2003, Bryan Oakley
# All Rights Reserved
#
# Bryan Oakley
# oakley@bardo.clearlight.com
#
# combobox v2.3 August 16, 2003
#
# MODIFIED (for TkDiff)
# 31Jul2018	 mpm: (<-tagged) added support for 'list itemconfigure' subcommand
# 25Oct2020	 mpm: (<-tagged) added hack for multiple-monitor issue (TK bug?)
#
# a combobox / dropdown listbox (pick your favorite name) widget
# written in pure tcl
#
# this code is freely distributable without restriction, but is
# provided as-is with no warranty expressed or implied.
#
# thanks to the following people who provided beta test support or
# patches to the code (in no particular order):
#
# Scott Beasley		Alexandre Ferrieux		Todd Helfter
# Matt Gushee		Laurent Duperval		John Jackson
# Fred Rapp			Christopher Nelson
# Eric Galluzzo		Jean-Francois Moine		Oliver Bienert
#
# A special thanks to Martin M. Hunt who provided several good ideas,
# and always with a patch to implement them. Jean-Francois Moine,
# Todd Helfter and John Jackson were also kind enough to send in some
# code patches.
#
# ... and many others over the years.

# ITSELF requires Tk 8.0-	  (but TkDiff already has that covered)
package provide combobox 2.3

namespace eval ::combobox {

	# this is the public interface
	namespace export combobox

	# these contain references to available options
	variable widgetOptions

	# these contain references to available commands and subcommands
	variable widgetCommands
	variable scanCommands
	variable listCommands
}

# ::combobox::combobox --
#
#	  This is the command that gets exported. It creates a new
#	  combobox widget.
#
# Arguments:
#
#	  w		   path of new widget to create
#	  args	   additional option/value pairs (eg: -background white, etc.)
#
# Results:
#
#	  It creates the widget and sets up all of the default bindings
#
# Returns:
#
#	  The name of the newly created widget

proc ::combobox::combobox {w args} {
	variable widgetOptions
	variable widgetCommands
	variable scanCommands
	variable listCommands

	# perform a one time initialization
	if {![info exists widgetOptions]} {
		Init
	}

	# build it...
	eval Build $w $args

	# set some bindings...
	SetBindings $w

	# and we are done!
	return $w
}

# ::combobox::Init --
#
#	  Initialize the namespace variables. This should only be called
#	  once, immediately prior to creating the first instance of the
#	  widget
#
# Arguments:
#
#	 none
#
# Results:
#
#	  All state variables are set to their default values; all of
#	  the option database entries will exist.
#
# Returns:
#
#	  empty string

proc ::combobox::Init {} {
	variable widgetOptions
	variable widgetCommands
	variable scanCommands
	variable listCommands
	variable defaultEntryCursor

	array set widgetOptions [list \
		-background			 {background		  Background} \
		-bd					 -borderwidth \
		-bg					 -background \
		-borderwidth		 {borderWidth		  BorderWidth} \
		-buttonbackground	 {buttonBackground	  Background} \
		-command			 {command			  Command} \
		-commandstate		 {commandState		  State} \
		-cursor				 {cursor			  Cursor} \
		-disabledbackground	 {disabledBackground  DisabledBackground} \
		-disabledforeground	 {disabledForeground  DisabledForeground} \
		-dropdownwidth		 {dropdownWidth		  DropdownWidth} \
		-editable			 {editable			  Editable} \
		-elementborderwidth	 {elementBorderWidth  BorderWidth} \
		-fg					 -foreground \
		-font				 {font				  Font} \
		-foreground			 {foreground		  Foreground} \
		-height				 {height			  Height} \
		-highlightbackground {highlightBackground HighlightBackground} \
		-highlightcolor		 {highlightColor	  HighlightColor} \
		-highlightthickness	 {highlightThickness  HighlightThickness} \
		-image				 {image				  Image} \
		-listvar			 {listVariable		  Variable} \
		-maxheight			 {maxHeight			  Height} \
		-opencommand		 {opencommand		  Command} \
		-relief				 {relief			  Relief} \
		-selectbackground	 {selectBackground	  Foreground} \
		-selectborderwidth	 {selectBorderWidth	  BorderWidth} \
		-selectforeground	 {selectForeground	  Background} \
		-state				 {state				  State} \
		-takefocus			 {takeFocus			  TakeFocus} \
		-textvariable		 {textVariable		  Variable} \
		-value				 {value				  Value} \
		-width				 {width				  Width} \
		-xscrollcommand		 {xScrollCommand	  ScrollCommand} \
	]

	set widgetCommands [list \
		bbox	  cget	   configure	curselection \
		delete	  get	   icursor		index		 \
		insert	  list	   scan			selection	 \
		xview	  select   toggle		open		 \
		close	 subwidget	\
	]

	set listCommands [list \
		delete		 get	  \
		index		 insert	  itemconfigure	   size \
	]  ;# mpm - added itemconfigure

	set scanCommands [list mark dragto]

	# why check for the Tk package? This lets us be sourced into
	# an interpreter that doesn't have Tk loaded, such as the slave
	# interpreter used by pkg_mkIndex. In theory it should have no
	# side effects when run
	if {[lsearch -exact [package names] "Tk"] != -1} {

		##################################################################
		#- this initializes the option database. Kinda gross, but it works
		#- (I think).
		##################################################################

		# the image used for the button...
		if {$::tcl_platform(platform) == "windows"} {
			image create bitmap ::combobox::bimage -data {
				#define down_arrow_width 12
				#define down_arrow_height 12
				static char down_arrow_bits[] = {
					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
					0xfc,0xf1,0xf8,0xf0,0x70,0xf0,0x20,0xf0,
					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00;
				}
			}
		} else {
			image create bitmap ::combobox::bimage -data  {
				#define down_arrow_width 15
				#define down_arrow_height 15
				static char down_arrow_bits[] = {
					0x00,0x80,0x00,0x80,0x00,0x80,0x00,0x80,
					0x00,0x80,0xf8,0x8f,0xf0,0x87,0xe0,0x83,
					0xc0,0x81,0x80,0x80,0x00,0x80,0x00,0x80,
					0x00,0x80,0x00,0x80,0x00,0x80
				}
			}
		}

		# compute a widget name we can use to create a temporary widget
		set tmpWidget ".__tmp__"
		set count 0
		while {[winfo exists $tmpWidget] == 1} {
			set tmpWidget ".__tmp__$count"
			incr count
		}

		# get the scrollbar width. Because we try to be clever and draw our
		# own button instead of using a tk widget, we need to know what size
		# button to create. This little hack tells us the width of a scroll
		# bar.
		#
		# NB: we need to be sure and pick a window	that doesn't already
		# exist...
		scrollbar $tmpWidget
		set sb_width [winfo reqwidth $tmpWidget]
		set bbg [$tmpWidget cget -background]
		destroy $tmpWidget

		# steal options from the entry widget
		# we want darn near all options, so we'll go ahead and do
		# them all. No harm done in adding the one or two that we
		# don't use.
		entry $tmpWidget
		foreach foo [$tmpWidget configure] {
			# the cursor option is special, so we'll save it in
			# a special way
			if {[lindex $foo 0] == "-cursor"} {
				set defaultEntryCursor [lindex $foo 4]
			}
			if {[llength $foo] == 5} {
				set option [lindex $foo 1]
				set value [lindex $foo 4]
				option add *Combobox.$option $value widgetDefault

				# these options also apply to the dropdown listbox
				if {[string compare $option "foreground"] == 0 \
						|| [string compare $option "background"] == 0 \
						|| [string compare $option "font"] == 0} {
					option add *Combobox*ComboboxListbox.$option $value \
							widgetDefault
				}
			}
		}
		destroy $tmpWidget

		# these are unique to us...
		option add *Combobox.elementBorderWidth	 1		widgetDefault
		option add *Combobox.buttonBackground	 $bbg	widgetDefault
		option add *Combobox.dropdownWidth		 {}		widgetDefault
		option add *Combobox.openCommand		 {}		widgetDefault
		option add *Combobox.cursor				 {}		widgetDefault
		option add *Combobox.commandState		 normal widgetDefault
		option add *Combobox.editable			 1		widgetDefault
		option add *Combobox.maxHeight			 10		widgetDefault
		option add *Combobox.height				 0
	}

	# set class bindings
	SetClassBindings
}

# ::combobox::SetClassBindings --
#
#	 Sets up the default bindings for the widget class
#
#	 this proc exists since it's The Right Thing To Do, but
#	 I haven't had the time to figure out how to do all the
#	 binding stuff on a class level. The main problem is that
#	 the entry widget must have focus for the insertion cursor
#	 to be visible. So, I either have to have the entry widget
#	 have the Combobox bindtag, or do some fancy juggling of
#	 events or some such. What a pain.
#
# Arguments:
#
#	 none
#
# Returns:
#
#	 empty string

proc ::combobox::SetClassBindings {} {

	# make sure we clean up after ourselves...
	bind Combobox <Destroy> [list ::combobox::DestroyHandler %W]

	# this will (hopefully) close (and lose the grab on) the
	# listbox if the user clicks anywhere outside of it. Note
	# that on Windows, you can click on some other app and
	# the listbox will still be there, because tcl won't see
	# that button click
	set this {[::combobox::convert %W -W]}
	bind Combobox <Any-ButtonPress>	  "$this close"
	bind Combobox <Any-ButtonRelease> "$this close"

	# this helps (but doesn't fully solve) focus issues. The general
	# idea is, whenever the frame gets focus it gets passed on to
	# the entry widget
	bind Combobox <FocusIn> {::combobox::tkTabToWindow \
								 [::combobox::convert %W -W].entry}

	# this closes the listbox if we get hidden
	bind Combobox <Unmap> {[::combobox::convert %W -W] close}

	return ""
}

# ::combobox::SetBindings --
#
#	 here's where we do most of the binding foo. I think there's probably
#	 a few bindings I ought to add that I just haven't thought
#	 about...
#
#	 I'm not convinced these are the proper bindings. Ideally all
#	 bindings should be on "Combobox", but because of my juggling of
#	 bindtags I'm not convinced thats what I want to do. But, it all
#	 seems to work, its just not as robust as it could be.
#
# Arguments:
#
#	 w	  widget pathname
#
# Returns:
#
#	 empty string

proc ::combobox::SetBindings {w} {
	upvar ::combobox::${w}::widgets	 widgets
	upvar ::combobox::${w}::options	 options

	# juggle the bindtags. The basic idea here is to associate the
	# widget name with the entry widget, so if a user does a bind
	# on the combobox it will get handled properly since it is
	# the entry widget that has keyboard focus.
	bindtags $widgets(entry) \
			[concat $widgets(this) [bindtags $widgets(entry)]]

	bindtags $widgets(button) \
			[concat $widgets(this) [bindtags $widgets(button)]]

	# override the default bindings for tab and shift-tab. The
	# focus procs take a widget as their only parameter and we
	# want to make sure the right window gets used (for shift-
	# tab we want it to appear as if the event was generated
	# on the frame rather than the entry.
	bind $widgets(entry) <Tab> \
			"::combobox::tkTabToWindow \[tk_focusNext $widgets(entry)\]; break"
	bind $widgets(entry) <Shift-Tab> \
			"::combobox::tkTabToWindow \[tk_focusPrev $widgets(this)\]; break"

	# this makes our "button" (which is actually a label)
	# do the right thing
	bind $widgets(button) <ButtonPress-1> [list $widgets(this) toggle]

	# this lets the autoscan of the listbox work, even if they
	# move the cursor over the entry widget.
	bind $widgets(entry) <B1-Enter> "break"

	bind $widgets(listbox) <ButtonRelease-1> \
		"::combobox::Select [list $widgets(this)] \
		 \[$widgets(listbox) nearest %y\]; break"

	bind $widgets(vsb) <ButtonPress-1>	 {continue}
	bind $widgets(vsb) <ButtonRelease-1> {continue}

	bind $widgets(listbox) <Any-Motion> {
		%W selection clear 0 end
		%W activate @%x,%y
		%W selection anchor @%x,%y
		%W selection set @%x,%y @%x,%y
		# need to do a yview if the cursor goes off the top
		# or bottom of the window... (or do we?)
	}

	# these events need to be passed from the entry widget
	# to the listbox, or otherwise need some sort of special
	# handling.
	foreach event [list <Up> <Down> <Tab> <Return> <Escape> \
			<Next> <Prior> <Double-1> <1> <Any-KeyPress> \
			<FocusIn> <FocusOut>] {
		bind $widgets(entry) $event \
			[list ::combobox::HandleEvent $widgets(this) $event]
	}

	# like the other events, <MouseWheel> needs to be passed from
	# the entry widget to the listbox. However, in this case we
	# need to add an additional parameter
	catch {
		bind $widgets(entry) <MouseWheel> \
			[list ::combobox::HandleEvent $widgets(this) <MouseWheel> %D]
	}
}

# ::combobox::Build --
#
#	 This does all of the work necessary to create the basic
#	 combobox.
#
# Arguments:
#
#	 w		  widget name
#	 args	  additional option/value pairs
#
# Results:
#
#	 Creates a new widget with the given name. Also creates a new
#	 namespace patterened after the widget name, as a child namespace
#	 to ::combobox
#
# Returns:
#
#	 the name of the widget

proc ::combobox::Build {w args } {
	variable widgetOptions

	if {[winfo exists $w]} {
		error "window name \"$w\" already exists"
	}

	# create the namespace for this instance, and define a few
	# variables
	namespace eval ::combobox::$w {

		variable ignoreTrace 0
		variable oldFocus	 {}
		variable oldGrab	 {}
		variable oldValue	 {}
		variable options
		variable this
		variable widgets

		set widgets(foo) foo  ;# coerce into an array
		set options(foo) foo  ;# coerce into an array

		unset widgets(foo)
		unset options(foo)
	}

	# import the widgets and options arrays into this proc so
	# we don't have to use fully qualified names, which is a
	# pain.
	upvar ::combobox::${w}::widgets widgets
	upvar ::combobox::${w}::options options

	# this is our widget -- a frame of class Combobox. Naturally,
	# it will contain other widgets. We create it here because
	# we need it in order to set some default options.
	set widgets(this)	[frame	$w -class Combobox -takefocus 0]
	set widgets(entry)	[entry	$w.entry -takefocus 1]
	set widgets(button) [label	$w.button -takefocus 0]

	# this defines all of the default options. We get the
	# values from the option database. Note that if an array
	# value is a list of length one it is an alias to another
	# option, so we just ignore it
	foreach name [array names widgetOptions] {
		if {[llength $widgetOptions($name)] == 1} continue

		set optName	 [lindex $widgetOptions($name) 0]
		set optClass [lindex $widgetOptions($name) 1]

		set value [option get $w $optName $optClass]
		set options($name) $value
	}

	# a couple options aren't available in earlier versions of
	# tcl, so we'll set them to sane values. For that matter, if
	# they exist but are empty, set them to sane values.
	if {[string length $options(-disabledforeground)] == 0} {
		set options(-disabledforeground) $options(-foreground)
	}
	if {[string length $options(-disabledbackground)] == 0} {
		set options(-disabledbackground) $options(-background)
	}

	# if -value is set to null, we'll remove it from our
	# local array. The assumption is, if the user sets it from
	# the option database, they will set it to something other
	# than null (since it's impossible to determine the difference
	# between a null value and no value at all).
	if {[info exists options(-value)] \
			&& [string length $options(-value)] == 0} {
		unset options(-value)
	}

	# we will later rename the frame's widget proc to be our
	# own custom widget proc. We need to keep track of this
	# new name, so we'll define and store it here...
	set widgets(frame) ::combobox::${w}::$w

	# gotta do this sooner or later. Might as well do it now
	pack $widgets(button) -side right -fill y	 -expand no
	pack $widgets(entry)  -side left  -fill both -expand yes

	# I should probably do this in a catch, but for now it's
	# good enough... What it does, obviously, is put all of
	# the option/values pairs into an array. Make them easier
	# to handle later on...
	array set options $args

	# Next, the dropdown list (built offscreen) ...
	# which also requires some extra window management foo
	wm withdraw		[set widgets(dropdown) [toplevel  $w.top]]
	wm overrideredirect $widgets(dropdown) 1
	wm transient		$widgets(dropdown) [winfo toplevel $w]
	wm group			$widgets(dropdown) [winfo  parent  $w]
	wm resizable		$widgets(dropdown) 0 0

	# The listbox and scrollbar go INSIDE that window,
	# ... but we only manage the vsb (later on) AS NEEDED
	set	  widgets(listbox) [listbox	  $w.top.list]
	set	  widgets(vsb)	   [scrollbar $w.top.vsb]
	pack $widgets(listbox) -side left -fill both -expand y

	# now fine tune the widgets based on the options (and a few
	# arbitrary values...)

	# NB: we are going to use the frame to handle the relief
	# of the widget as a whole, so the entry widget will be
	# flat. This makes the button which drops down the list
	# to appear "inside" the entry widget.

	$widgets(vsb) configure \
			-borderwidth 1 \
			-command "$widgets(listbox) yview" \
			-highlightthickness 0

	$widgets(button) configure \
			-background $options(-buttonbackground) \
			-highlightthickness 0 \
			-borderwidth $options(-elementborderwidth) \
			-relief raised \
			-width [expr {[winfo reqwidth $widgets(vsb)] - 2}]

	$widgets(entry) configure \
			-borderwidth 0 \
			-relief flat \
			-highlightthickness 0

	$widgets(dropdown) configure \
			-borderwidth $options(-elementborderwidth) \
			-relief sunken

	$widgets(listbox) configure \
			-selectmode browse \
			-background [$widgets(entry) cget -bg] \
			-yscrollcommand "$widgets(vsb) set" \
			-exportselection false \
			-borderwidth 0


#	 trace add variable ::combobox::${w}::entryTextVariable write \
#		 [list ::combobox::EntryTrace $w]

	# this moves the original frame widget proc into our
	# namespace and gives it a handy name
	rename ::$w $widgets(frame)

	# Finally, create our widget proc. Obviously (?) it goes in
	# the global namespace. All combobox widgets will actually
	# share the same widget proc to cut down on the amount of
	# bloat.
	proc ::$w {command args} \
		"eval ::combobox::WidgetProc $w \$command \$args"


	# ok, the thing exists... let's do a bit more INSTANCE configuration.
	if {[catch "::combobox::Configure \
					[list $widgets(this)] [array get options]" error]} {
		catch {destroy $w}
		error "internal error: $error"
	}

	return ""
}

# ::combobox::HandleEvent --
#
#	 this proc handles events from the entry widget that we want
#	 handled specially (typically, to allow navigation of the list
#	 even though the focus is in the entry widget)
#
# Arguments:
#
#	 w		 widget pathname
#	 event	 a string representing the event (not necessarily an
#			 actual event)
#	 args	 additional arguments required by particular events

proc ::combobox::HandleEvent {w event args} {
	upvar ::combobox::${w}::widgets	 widgets
	upvar ::combobox::${w}::options	 options
	upvar ::combobox::${w}::oldValue oldValue

	# for all of these events, if we have a special action we'll
	# do that and do a "return -code break" to keep additional
	# bindings from firing. Otherwise we'll let the event fall
	# on through.
	switch $event {

		"<MouseWheel>" {
			if {[winfo ismapped $widgets(dropdown)]} {
				set D [lindex $args 0]
				# the '120' number in the following expression has
				# it's genesis in the tk bind manpage, which suggests
				# that the smallest value of %D for mousewheel events
				# will be 120. The intent is to scroll one line at a time.
				$widgets(listbox) yview scroll [expr {-($D/120)}] units
			}
		}

		"<Any-KeyPress>" {
			# if the widget is editable, clear the selection.
			# this makes it more obvious what will happen if the
			# user presses <Return> (and helps our code know what
			# to do if the user presses return)
			if {$options(-editable)} {
				$widgets(listbox) see 0
				$widgets(listbox) selection clear 0 end
				$widgets(listbox) selection anchor 0
				$widgets(listbox) activate 0
			}
		}

		"<FocusIn>" {
			set oldValue [$widgets(entry) get]
		}

		"<FocusOut>" {
			if {![winfo ismapped $widgets(dropdown)]} {
				# did the value change?
				set newValue [$widgets(entry) get]
				if {$oldValue != $newValue} {
					CallCommand $widgets(this) $newValue
				}
			}
		}

		"<1>" {
			set editable [::combobox::GetBoolean $options(-editable)]
			if {!$editable} {
				if {[winfo ismapped $widgets(dropdown)]} {
					$widgets(this) close
					return -code break;

				} else {
					if {$options(-state) != "disabled"} {
						$widgets(this) open
						return -code break;
					}
				}
			}
		}

		"<Double-1>" {
			if {$options(-state) != "disabled"} {
				$widgets(this) toggle
				return -code break;
			}
		}

		"<Tab>" {
			if {[winfo ismapped $widgets(dropdown)]} {
				::combobox::Find $widgets(this) 0
				return -code break;
			} else {
				::combobox::SetValue $widgets(this) [$widgets(this) get]
			}
		}

		"<Escape>" {
#			$widgets(entry) delete 0 end
#			$widgets(entry) insert 0 $oldValue
			if {[winfo ismapped $widgets(dropdown)]} {
				$widgets(this) close
				return -code break;
			}
		}

		"<Return>" {
			# did the value change?
			set newValue [$widgets(entry) get]
			if {$oldValue != $newValue} {
				CallCommand $widgets(this) $newValue
			}

			if {[winfo ismapped $widgets(dropdown)]} {
				::combobox::Select $widgets(this) \
						[$widgets(listbox) curselection]
				return -code break;
			}
		}

		"<Next>" {
			$widgets(listbox) yview scroll 1 pages
			set index [$widgets(listbox) index @0,0]
			$widgets(listbox) see $index
			$widgets(listbox) activate $index
			$widgets(listbox) selection clear 0 end
			$widgets(listbox) selection anchor $index
			$widgets(listbox) selection set $index
		}

		"<Prior>" {
			$widgets(listbox) yview scroll -1 pages
			set index [$widgets(listbox) index @0,0]
			$widgets(listbox) activate $index
			$widgets(listbox) see $index
			$widgets(listbox) selection clear 0 end
			$widgets(listbox) selection anchor $index
			$widgets(listbox) selection set $index
		}

		"<Down>" {
			if {[winfo ismapped $widgets(dropdown)]} {
				::combobox::tkListboxUpDown $widgets(listbox) 1
				return -code break;

			} else {
				if {$options(-state) != "disabled"} {
					$widgets(this) open
					return -code break;
				}
			}
		}

		"<Up>" {
			if {[winfo ismapped $widgets(dropdown)]} {
				::combobox::tkListboxUpDown $widgets(listbox) -1
				return -code break;

			} else {
				if {$options(-state) != "disabled"} {
					$widgets(this) open
					return -code break;
				}
			}
		}
	}

	return ""
}

# ::combobox::DestroyHandler {w} --
#
#	 Cleans up after a combobox widget is destroyed
#
# Arguments:
#
#	 w	  widget pathname
#
# Results:
#
#	 The namespace that was created for the widget is deleted,
#	 and the widget proc is removed.

proc ::combobox::DestroyHandler {w} {

	catch {
		# if the widget actually being destroyed is of class Combobox,
		# remove the namespace and associated proc.
		if {[string compare [winfo class $w] "Combobox"] == 0} {
			# delete the namespace and the proc which represents
			# our widget
			namespace delete ::combobox::$w
			rename $w {}
		}
	}
	return ""
}

# ::combobox::Find
#
#	 finds something in the listbox that matches the pattern in the
#	 entry widget and selects it
#
#	 N.B. I'm not convinced this is working the way it ought to. It
#	 works, but is the behavior what is expected? I've also got a gut
#	 feeling that there's a better way to do this, but I'm too lazy to
#	 figure it out...
#
# Arguments:
#
#	 w		widget pathname
#	 exact	boolean; if true an exact match is desired
#
# Returns:
#
#	 Empty string

proc ::combobox::Find {w {exact 0}} {
	upvar ::combobox::${w}::widgets widgets
	upvar ::combobox::${w}::options options

	## *sigh* this logic is rather gross and convoluted. Surely
	## there is a more simple, straight-forward way to implement
	## all this. As the saying goes, I lack the time to make it
	## shorter...

	# use what is already in the entry widget as a pattern
	set pattern [$widgets(entry) get]

	if {[string length $pattern] == 0} {
		# clear the current selection
		$widgets(listbox) see 0
		$widgets(listbox) selection clear 0 end
		$widgets(listbox) selection anchor 0
		$widgets(listbox) activate 0
		return
	}

	# we're going to be searching this list...
	set list [$widgets(listbox) get 0 end]

	# if we are doing an exact match, try to find,
	# well, an exact match
	set exactMatch -1
	if {$exact} {
		set exactMatch [lsearch -exact $list $pattern]
	}

	# search for it. We'll try to be clever and not only
	# search for a match for what they typed, but a match for
	# something close to what they typed. We'll keep removing one
	# character at a time from the pattern until we find a match
	# of some sort.
	set index -1
	while {$index == -1 && [string length $pattern]} {
		set index [lsearch -glob $list "$pattern*"]
		if {$index == -1} {
			regsub {.$} $pattern {} pattern
		}
	}

	# this is the item that most closely matches...
	set thisItem [lindex $list $index]

	# did we find a match? If so, do some additional munging...
	if {$index != -1} {

		# we need to find the part of the first item that is
		# unique WRT the second... I know there's probably a
		# simpler way to do this...

		set nextIndex [expr {$index + 1}]
		set nextItem [lindex $list $nextIndex]

		# we don't really need to do much if the next
		# item doesn't match our pattern...
		if {[string match $pattern* $nextItem]} {
			# ok, the next item matches our pattern, too
			# now the trick is to find the first character
			# where they *don't* match...
			set marker [string length $pattern]
			while {$marker <= [string length $pattern]} {
				set a [string index $thisItem $marker]
				set b [string index $nextItem $marker]
				if {[string compare $a $b] == 0} {
					append pattern $a
					incr marker
				} else {
					break
				}
			}
		} else {
			set marker [string length $pattern]
		}

	} else {
		set marker end
		set index 0
	}

	# ok, we know the pattern and what part is unique;
	# update the entry widget and listbox appropriately
	if {$exact && $exactMatch == -1} {
		# this means we didn't find an exact match
		$widgets(listbox) selection clear 0 end
		$widgets(listbox) see $index

	} elseif {!$exact}	{
		# this means we found something, but it isn't an exact
		# match. If we find something that *is* an exact match we
		# don't need to do the following, since it would merely
		# be replacing the data in the entry widget with itself
		set oldstate [$widgets(entry) cget -state]
		$widgets(entry) configure -state normal
		$widgets(entry) delete 0 end
		$widgets(entry) insert end $thisItem
		$widgets(entry) selection clear
		$widgets(entry) selection range $marker end
		$widgets(listbox) activate $index
		$widgets(listbox) selection clear 0 end
		$widgets(listbox) selection anchor $index
		$widgets(listbox) selection set $index
		$widgets(listbox) see $index
		$widgets(entry) configure -state $oldstate
	}
}

# ::combobox::Select --
#
#	 selects an item from the list and sets the value of the combobox
#	 to that value
#
# Arguments:
#
#	 w		widget pathname
#	 index	listbox index of item to be selected
#
# Returns:
#
#	 empty string

proc ::combobox::Select {w index} {
	upvar ::combobox::${w}::widgets widgets
	upvar ::combobox::${w}::options options

	# the catch is because I'm sloppy -- presumably, the only time
	# an error will be caught is if there is no selection.
	if {![catch {set data [$widgets(listbox) get [lindex $index 0]]}]} {
		::combobox::SetValue $widgets(this) $data

		$widgets(listbox) selection clear 0 end
		$widgets(listbox) selection anchor $index
		$widgets(listbox) selection set $index

	}
	$widgets(entry) selection range 0 end
	$widgets(entry) icursor end

	$widgets(this) close

	return ""
}

# ::combobox::HandleScrollbar --
#
#	 causes the scrollbar of the dropdown list to appear or disappear
#	 based on the contents of the dropdown listbox
#
# Arguments:
#
#	 w		 widget pathname
#	 action	 the action to perform on the scrollbar
#
# Returns:
#
#	 an empty string

proc ::combobox::HandleScrollbar {w {action "unknown"}} {
	upvar ::combobox::${w}::widgets widgets
	upvar ::combobox::${w}::options options

	if {$options(-height) == 0} {
		set hlimit $options(-maxheight)
	} else {
		set hlimit $options(-height)
	}

	switch $action {
		"grow" {
			if {$hlimit > 0 && [$widgets(listbox) size] > $hlimit} {
				pack forget $widgets(listbox)
				pack $widgets(vsb) -side right -fill y -expand n
				pack $widgets(listbox) -side left -fill both -expand y
			}
		}

		"shrink" {
			if {$hlimit > 0 && [$widgets(listbox) size] <= $hlimit} {
				pack forget $widgets(vsb)
			}
		}

		"crop" {
			# this means the window was cropped and we definitely
			# need a scrollbar no matter what the user wants
			pack forget $widgets(listbox)
			pack $widgets(vsb) -side right -fill y -expand n
			pack $widgets(listbox) -side left -fill both -expand y
		}

		default {
			if {$hlimit > 0 && [$widgets(listbox) size] > $hlimit} {
				pack forget $widgets(listbox)
				pack $widgets(vsb) -side right -fill y -expand n
				pack $widgets(listbox) -side left -fill both -expand y
			} else {
				pack forget $widgets(vsb)
			}
		}
	}

	return ""
}

# ::combobox::ComputeGeometry --
#
#	 computes the geometry of the dropdown list based on the size of the
#	 combobox...
#
# Arguments:
#
#	 w	   widget pathname
#
# Returns:
#
#	 the desired geometry of the listbox

proc ::combobox::ComputeGeometry {w} {
	upvar ::combobox::${w}::widgets widgets
	upvar ::combobox::${w}::options options

	if {$options(-height) == 0 && $options(-maxheight) != "0"} {
		# if this is the case, count the items and see if
		# it exceeds our maxheight. If so, set the listbox
		# size to maxheight...
		set nitems [$widgets(listbox) size]
		if {$nitems > $options(-maxheight)} {
			# tweak the height of the listbox
			$widgets(listbox) configure -height $options(-maxheight)
		} else {
			# un-tweak the height of the listbox
			$widgets(listbox) configure -height 0
		}
		update idletasks
	}

	# compute height and width of the dropdown list
	set bd [$widgets(dropdown) cget -borderwidth]
	set height [expr {[winfo reqheight $widgets(dropdown)] + $bd + $bd}]
	if {[string length $options(-dropdownwidth)] == 0
	||	$options(-dropdownwidth) == 0} {
		set width [winfo width $widgets(this)]
	} else {
		set m [font measure [$widgets(listbox) cget -font] "m"]
		set width [expr {$options(-dropdownwidth) * $m}]
	}

	# (Sadly, for Windows users the following measurements won't take into
	# consideration the height of the taskbar, but don't blame me -- there
	# isn't any way to detect it or figure out its dimensions. The same is
	# likely true of any window manager with some magic windows glued to the
	# top or bottom of the screen)

	# Figure out where to place it on the screen, trying to take into account
	# we MAY be running under some virtual window manager
	#	(but lets use REALLY-short varnames, 'cause it gets a little involved)
	# N.B> ALL width/height values are POSITIVE MAGNITUDES only (not coords)
	lassign "[winfo	  vrootx	 $widgets(this)]
			 [winfo	  vrooty	 $widgets(this)]
			 [winfo	   rootx	 $widgets(this)]
			 [winfo	   rooty	 $widgets(this)] $width $height 0 0
			 [winfo screenwidth	 $widgets(this)]
			 [winfo screenheight $widgets(this)]" vx vy x y w h X Y W H

	########### mpm
	#TK-BUG?	Detected: Oct2020 TK8.6.3(+)	 Multiple display-monitor hack
	###
	#	While the screen DIMENSIONs *may* be correct, they are UN-TETHERED when
	#	related to anything BUT a "main" display SCREEN...(eg. a 2nd monitor)
	#	(ie. where WxH is NOT anchored at [0,0], be that actually or virtually)
	#	The BUG: there exists NO MEANS of obtaining THAT screens ORIGIN coord!
	#
	# So, IFF the widget is OUTSIDE the 0-based screen dimension AT THE OUTSET,
	# then RESET the X,Y,W,H values to its containing Toplevel before going on
	if {($y+$vy+$h < $Y) || ($y+$vy+$h > $Y+$H)
	||	($y+$vy	   < $Y) || ($y+$vy	   > $Y+$H)} {
		set TL [winfo toplevel $widgets(this)]
		lassign "[winfo rootx $TL] [winfo rooty	 $TL]
				 [winfo width $TL] [winfo height $TL]" X Y W H
	}

	# The x coordinate is simply the rootx of our widget, adjusted for
	# the virtual window. We won't worry about whether the window will
	# be offscreen to the left or right -- we want the illusion that it
	# is part of the entry widget, so if part of the entry widget is off-
	# screen, so will the list. If you want to change the behavior,
	# simply change the "if{0}" statement... (AND update this comment!)
	incr x $vx
	if {0} {
		# Keep it inboard of the defined limits (when possible)
		if {($x + $w) > ($X + $W)} { set x [expr {$X + $W - $w}] }
		if {	$x	  < $X} { set x $X }
	}

	# The y coordinate begins as the rooty plus vrooty offset plus
	# the height of the static part of the widget plus 1 for a
	# tiny bit of visual separation...
	set y_below [expr {$y + $vy + [winfo reqheight $widgets(this)] + 1}]

	# But check if it will FIT (at its present size)...
	if {($y_below + $h) >= ($Y + $H)} {
		# No? OK - Fine. So pop it UP above the entry widget instead.
		set y_above [expr {$y + $vy - $h - 1}]

		# But (again) check if it fits THERE (at its present size)...
		if {$y_above < $Y} {
			# How annoying!! This means it extended beyond our "screen"
			# Now we'll try to be REALLY clever and either pop it UP or
			# DOWN, depending on WHICH way gives us the biggest list,
			# TRIMMING THE LIST to fit and forcing the use of a scrollbar

			if {($y+$vy) > ($Y + ($H / 2))} {
				# we are in the LOWER half of the "screen" -- pop it UP.
				# Y will be its upper-bound; that parts easy. The HEIGHT
				# becomes its DISTANCE TO the y coordinate of our widget,
				# minus a pixel for some visual separation.
				set h [expr {$y + $vy - $Y - 1}]
				set y $Y
			} else {
				# we are in the UPPER half of the "screen" -- pop it DOWN
				# while trimming its HEIGHT to the lower boundary
				set h [expr {$y_below - ($Y + $H)}]
				set y $y_below
			}
			HandleScrollbar $widgets(this) crop

		} else	{ set y $y_above }
	} else		{ set y $y_below }

	# FINALLY return the resultant geometry
	return [format "=%dx%d+%d+%d" $w $h $x $y]
}

# ::combobox::DoInternalWidgetCommand --
#
#	 perform an internal widget command, then mung any error results
#	 to look like it came from our megawidget. A lot of work just to
#	 give the illusion that our megawidget is an atomic widget
#
# Arguments:
#
#	 w			 widget pathname
#	 subwidget	 pathname of the subwidget
#	 command	 subwidget command to be executed
#	 args		 arguments to the command
#
# Returns:
#
#	 The result of the subwidget command, or an error

proc ::combobox::DoInternalWidgetCommand {w subwidget command args} {
	upvar ::combobox::${w}::widgets widgets
	upvar ::combobox::${w}::options options

	set subcommand $command
	set command [concat $widgets($subwidget) $command $args]

	if {[catch $command result]} {
		# replace the subwidget name with the megawidget name
		regsub $widgets($subwidget) $result $widgets(this) result

		# replace specific instances of the subwidget command
		# with our megawidget command
		switch $subwidget,$subcommand {
			listbox,index  {regsub "index"	$result "list index"  result}
			listbox,insert {regsub "insert" $result "list insert" result}
			listbox,delete {regsub "delete" $result "list delete" result}
			listbox,get	   {regsub "get"	$result "list get"	  result}
			listbox,size   {regsub "size"	$result "list size"	  result}
			listbox,itemconfigure	{ ;# mpm: added entire switch clause
			  regsub "itemconfigure" $result "list itemconfigure" result}
		}
		error $result

	} else {
		return $result
	}
}


# ::combobox::WidgetProc --
#
#	 This gets uses as the widgetproc for an combobox widget.
#	 Notice where the widget is created and you'll see that the
#	 actual widget proc merely evals this proc with all of the
#	 arguments intact.
#
#	 Note that some widget commands are defined "inline" (ie:
#	 within this proc), and some do most of their work in
#	 separate procs. This is merely because sometimes it was
#	 easier to do it one way or the other.
#
# Arguments:
#
#	 w		   widget pathname
#	 command   widget subcommand
#	 args	   additional arguments; varies with the subcommand
#
# Results:
#
#	 Performs the requested widget command

proc ::combobox::WidgetProc {w command args} {
	upvar ::combobox::${w}::widgets widgets
	upvar ::combobox::${w}::options options
	upvar ::combobox::${w}::oldFocus oldFocus
	upvar ::combobox::${w}::oldGrab oldGrab

	set command [::combobox::Canonize $w command $command]

	# this is just shorthand notation...
	set doWidgetCommand \
			[list ::combobox::DoInternalWidgetCommand $widgets(this)]

	if {$command == "list"} {
		# ok, the next argument is a list command; we'll
		# rip it from args and append it to command to
		# create a unique internal command
		#
		# NB: because of the sloppy way we are doing this,
		# we'll also let the user enter our secret command
		# directly (eg: list-insert, list-delete , etc), but we
		# won't document that fact (mpm: bugfix - was missing Canonize)
		set command "list-[::combobox::Canonize \
								 $w {list command} [lindex $args 0]]"
		set args [lrange $args 1 end]
	}

	set result ""

	# many of these commands are just synonyms for specific
	# commands in one of the subwidgets. We'll get them out
	# of the way first, then do the custom commands.
	switch $command {
		bbox -
		delete -
		get -
		icursor -
		index -
		insert -
		scan -
		selection -
		xview {
			set result [eval $doWidgetCommand entry $command $args]
		}
		list-get	{set result [eval $doWidgetCommand listbox get $args]}
		list-index	{set result [eval $doWidgetCommand listbox index $args]}
		list-size	{set result [eval $doWidgetCommand listbox size $args]}
		list-itemconfigure	 { ;# mpm - added entire switch clause
			   set result [eval $doWidgetCommand listbox itemconfigure $args]}

		select {
			if {[llength $args] == 1} {
				set index [lindex $args 0]
				set result [Select $widgets(this) $index]
			} else {
				error "usage: $w select index"
			}
		}

		subwidget {
			set knownWidgets [list button entry listbox dropdown vsb]
			if {[llength $args] == 0} {
				return $knownWidgets
			}

			set name [lindex $args 0]
			if {[lsearch $knownWidgets $name] != -1} {
				set result $widgets($name)
			} else {
				error "unknown subwidget $name"
			}
		}

		curselection {
			set result [eval $doWidgetCommand listbox curselection]
		}

		list-insert {
			eval $doWidgetCommand listbox insert $args
			set result [HandleScrollbar $w "grow"]
		}

		list-delete {
			eval $doWidgetCommand listbox delete $args
			set result [HandleScrollbar $w "shrink"]
		}

		toggle {
			# ignore this command if the widget is disabled...
			if {$options(-state) == "disabled"} return

			# pops down the list if it is not, hides it
			# if it is...
			if {[winfo ismapped $widgets(dropdown)]} {
				set result [$widgets(this) close]
			} else {
				set result [$widgets(this) open]
			}
		}

		open {

			# if this is an editable combobox, the focus should
			# be set to the entry widget
			if {$options(-editable)} {
				focus $widgets(entry)
				$widgets(entry) select range 0 end
				$widgets(entry) icursor end
			}

			# if we are disabled, we won't allow this to happen
			if {$options(-state) == "disabled"} {
				return 0
			}

			# if there is a -opencommand, execute it now
			if {[string length $options(-opencommand)] > 0} {
				# hmmm... should I do a catch, or just let the normal
				# error handling handle any errors? For now, the latter...
				uplevel \#0 $options(-opencommand)
			}

			# compute the geometry of the window to pop up, and set
			# it, and force the window manager to take notice
			# (even if it is not presently visible).
			#
			# this isn't strictly necessary if the window is already
			# mapped, but we'll go ahead and set the geometry here
			# since its harmless and *may* actually reset the geometry
			# to something better in some weird case.
			set geometry [::combobox::ComputeGeometry $widgets(this)]
			wm geometry $widgets(dropdown) $geometry
			update idletasks

			# if we are already open, there's nothing else to do
			if {[winfo ismapped $widgets(dropdown)]} {
				return 0
			}

			# save the widget that currently has the focus; we'll restore
			# the focus there when we're done
			set oldFocus [focus]

			# ok, tweak the visual appearance of things and
			# make the list pop up
			$widgets(button) configure -relief sunken
			wm deiconify $widgets(dropdown)
			update idletasks
			raise $widgets(dropdown)

			# force focus to the entry widget so we can handle keypress
			# events for traversal
			focus -force $widgets(entry)

			# select something by default, but only if its an
			# exact match...
			::combobox::Find $widgets(this) 1

			# save the current grab state for the display containing
			# this widget. We'll restore it when we close the dropdown
			# list
			set status "none"
			set grab [grab current $widgets(this)]
			if {$grab != ""} {set status [grab status $grab]}
			set oldGrab [list $grab $status]
			unset grab status

			# *gasp* do a global grab!!! Mom always told me not to
			# do things like this, but sometimes a man's gotta do
			# what a man's gotta do.
			grab -global $widgets(this)

			# fake the listbox into thinking it has focus. This is
			# necessary to get scanning initialized properly in the
			# listbox.
			event generate $widgets(listbox) <B1-Enter>

			return 1
		}

		close {
			# if we are already closed, don't do anything...
			if {![winfo ismapped $widgets(dropdown)]} {
				return 0
			}

			# restore the focus and grab, but ignore any errors...
			# we're going to be paranoid and release the grab before
			# trying to set any other grab because we really really
			# really want to make sure the grab is released.
			catch {focus $oldFocus} result
			catch {grab release $widgets(this)}
			catch {
				set status [lindex $oldGrab 1]
				if {$status == "global"} {
					grab -global [lindex $oldGrab 0]
				} elseif {$status == "local"} {
					grab [lindex $oldGrab 0]
				}
				unset status
			}

			# hides the listbox
			$widgets(button) configure -relief raised
			wm withdraw $widgets(dropdown)

			# select the data in the entry widget. Not sure
			# why, other than observation seems to suggest that's
			# what windows widgets do.
			set editable [::combobox::GetBoolean $options(-editable)]
			if {$editable} {
				$widgets(entry) selection range 0 end
				$widgets(button) configure -relief raised
			}


			# magic tcl stuff (see tk.tcl in the distribution
			# lib directory)
			::combobox::tkCancelRepeat

			return 1
		}

		cget {
			if {[llength $args] != 1} {
				error "wrong # args: should be $w cget option"
			}
			set opt [::combobox::Canonize $w option [lindex $args 0]]

			if {$opt == "-value"} {
				set result [$widgets(entry) get]
			} else {
				set result $options($opt)
			}
		}

		configure {
			set result [eval ::combobox::Configure {$w} $args]
		}

		default {
			error "bad option \"$command\""
		}
	}

	return $result
}

# ::combobox::Configure --
#
#	 Implements the "configure" widget subcommand
#
# Arguments:
#
#	 w		widget pathname
#	 args	zero or more option/value pairs (or a single option)
#
# Results:
#
#	 Performs typcial "configure" type requests on the widget

proc ::combobox::Configure {w args} {
	variable widgetOptions
	variable defaultEntryCursor

	upvar ::combobox::${w}::widgets widgets
	upvar ::combobox::${w}::options options

	if {[llength $args] == 0} {
		# hmmm. User must be wanting all configuration information
		# note that if the value of an array element is of length
		# one it is an alias, which needs to be handled slightly
		# differently
		set results {}
		foreach opt [lsort [array names widgetOptions]] {
			if {[llength $widgetOptions($opt)] == 1} {
				set alias $widgetOptions($opt)
				set optName $widgetOptions($alias)
				lappend results [list $opt $optName]
			} else {
				set optName	 [lindex $widgetOptions($opt) 0]
				set optClass [lindex $widgetOptions($opt) 1]
				set default [option get $w $optName $optClass]
				if {[info exists options($opt)]} {
					lappend results [list $opt $optName $optClass \
							$default $options($opt)]
				} else {
					lappend results [list $opt $optName $optClass \
							$default ""]
				}
			}
		}

		return $results
	}

	# one argument means we are looking for configuration
	# information on a single option
	if {[llength $args] == 1} {
		set opt [::combobox::Canonize $w option [lindex $args 0]]

		set optName	 [lindex $widgetOptions($opt) 0]
		set optClass [lindex $widgetOptions($opt) 1]
		set default [option get $w $optName $optClass]
		set results [list $opt $optName $optClass \
				$default $options($opt)]
		return $results
	}

	# if we have an odd number of values, bail.
	if {[expr {[llength $args]%2}] == 1} {
		# hmmm. An odd number of elements in args
		error "value for \"[lindex $args end]\" missing"
	}

	# Great. An even number of options. Let's make sure they
	# are all valid before we do anything. Note that Canonize
	# will generate an error if it finds a bogus option; otherwise
	# it returns the canonical option name
	foreach {name value} $args {
		set name [::combobox::Canonize $w option $name]
		set opts($name) $value
	}

	# process all of the configuration options
	# some (actually, most) options require us to
	# do something, like change the attributes of
	# a widget or two. Here's where we do that...
	#
	# note that the handling of disabledforeground and
	# disabledbackground is a little wonky. First, we have
	# to deal with backwards compatibility (ie: tk 8.3 and below
	# didn't have such options for the entry widget), and
	# we have to deal with the fact we might want to disable
	# the entry widget but use the normal foreground/background
	# for when the combobox is not disabled, but not editable either.

	set updateVisual 0
	foreach option [array names opts] {
		set newValue $opts($option)
		if {[info exists options($option)]} {
			set oldValue $options($option)
		}

		switch -- $option {
			-buttonbackground {
				$widgets(button) configure -background $newValue
			}
			-background {
				set updateVisual 1
				set options($option) $newValue
			}

			-borderwidth {
				$widgets(frame) configure -borderwidth $newValue
				set options($option) $newValue
			}

			-command {
				# nothing else to do...
				set options($option) $newValue
			}

			-commandstate {
				# do some value checking...
				if {$newValue != "normal" && $newValue != "disabled"} {
					set options($option) $oldValue
					set message "bad state value \"$newValue\";"
					append message " must be normal or disabled"
					error $message
				}
				set options($option) $newValue
			}

			-cursor {
				$widgets(frame) configure -cursor $newValue
				$widgets(entry) configure -cursor $newValue
				$widgets(listbox) configure -cursor $newValue
				set options($option) $newValue
			}

			-disabledforeground {
				set updateVisual 1
				set options($option) $newValue
			}

			-disabledbackground {
				set updateVisual 1
				set options($option) $newValue
			}

			-dropdownwidth {
				set options($option) $newValue
			}

			-editable {
				set updateVisual 1
				if {$newValue} {
					# it's editable...
					$widgets(entry) configure -state normal \
							-cursor $defaultEntryCursor
				} else {
					$widgets(entry) configure -state disabled \
							-cursor $options(-cursor)
				}
				set options($option) $newValue
			}

			-elementborderwidth {
				$widgets(button) configure -borderwidth $newValue
				$widgets(vsb) configure -borderwidth $newValue
				$widgets(dropdown) configure -borderwidth $newValue
				set options($option) $newValue
			}

			-font {
				$widgets(entry) configure -font $newValue
				$widgets(listbox) configure -font $newValue
				set options($option) $newValue
			}

			-foreground {
				set updateVisual 1
				set options($option) $newValue
			}

			-height {
				$widgets(listbox) configure -height $newValue
				HandleScrollbar $w
				set options($option) $newValue
			}

			-highlightbackground {
				$widgets(frame) configure -highlightbackground $newValue
				set options($option) $newValue
			}

			-highlightcolor {
				$widgets(frame) configure -highlightcolor $newValue
				set options($option) $newValue
			}

			-highlightthickness {
				$widgets(frame) configure -highlightthickness $newValue
				set options($option) $newValue
			}

			-image {
				if {[string length $newValue] > 0} {
				#	puts "old button width: [$widgets(button) cget -width]"
					$widgets(button) configure \
						-image $newValue \
						-width [expr {[image width $newValue] + 2}]
				#	puts "new button width: [$widgets(button) cget -width]"

				} else {
					$widgets(button) configure -image ::combobox::bimage
				}
				set options($option) $newValue
			}

			-listvar {
				if {[catch {$widgets(listbox) cget -listvar}]} {
					return -code error \
						"-listvar not supported with this version of tk"
				}
				$widgets(listbox) configure -listvar $newValue
				set options($option) $newValue
			}

			-maxheight {
				# ComputeGeometry may dork with the actual height
				# of the listbox, so let's undork it
				$widgets(listbox) configure -height $options(-height)
				HandleScrollbar $w
				set options($option) $newValue
			}

			-opencommand {
				# nothing else to do...
				set options($option) $newValue
			}

			-relief {
				$widgets(frame) configure -relief $newValue
				set options($option) $newValue
			}

			-selectbackground {
				$widgets(entry) configure -selectbackground $newValue
				$widgets(listbox) configure -selectbackground $newValue
				set options($option) $newValue
			}

			-selectborderwidth {
				$widgets(entry) configure -selectborderwidth $newValue
				$widgets(listbox) configure -selectborderwidth $newValue
				set options($option) $newValue
			}

			-selectforeground {
				$widgets(entry) configure -selectforeground $newValue
				$widgets(listbox) configure -selectforeground $newValue
				set options($option) $newValue
			}

			-state {
				if {$newValue == "normal"} {
					set updateVisual 1
					# it's enabled

					set editable [::combobox::GetBoolean \
							$options(-editable)]
					if {$editable} {
						$widgets(entry) configure -state normal
						$widgets(entry) configure -takefocus 1
					}

					# note that $widgets(button) is actually a label,
					# not a button. And being able to disable labels
					# wasn't possible until tk 8.3. (makes me wonder
					# why I chose to use a label, but that answer is
					# lost to antiquity)
					if {[info patchlevel] >= 8.3} {
						$widgets(button) configure -state normal
					}

				} elseif {$newValue == "disabled"}	{
					set updateVisual 1
					# it's disabled
					$widgets(entry) configure -state disabled
					$widgets(entry) configure -takefocus 0
					# note that $widgets(button) is actually a label,
					# not a button. And being able to disable labels
					# wasn't possible until tk 8.3. (makes me wonder
					# why I chose to use a label, but that answer is
					# lost to antiquity)
					if {$::tcl_version >= 8.3} {
						$widgets(button) configure -state disabled
					}

				} else {
					set options($option) $oldValue
					set message "bad state value \"$newValue\";"
					append message " must be normal or disabled"
					error $message
				}

				set options($option) $newValue
			}

			-takefocus {
				$widgets(entry) configure -takefocus $newValue
				set options($option) $newValue
			}

			-textvariable {
				$widgets(entry) configure -textvariable $newValue
				set options($option) $newValue
			}

			-value {
				::combobox::SetValue $widgets(this) $newValue
				set options($option) $newValue
			}

			-width {
				$widgets(entry) configure -width $newValue
				$widgets(listbox) configure -width $newValue
				set options($option) $newValue
			}

			-xscrollcommand {
				$widgets(entry) configure -xscrollcommand $newValue
				set options($option) $newValue
			}
		}

		if {$updateVisual} {UpdateVisualAttributes $w}
	}
}

# ::combobox::UpdateVisualAttributes --
#
# sets the visual attributes (foreground, background mostly)
# based on the current state of the widget (normal/disabled,
# editable/non-editable)
#
# why a proc for such a simple thing? Well, in addition to the
# various states of the widget, we also have to consider the
# version of tk being used -- versions from 8.4 and beyond have
# the notion of disabled foreground/background options for various
# widgets. All of the permutations can get nasty, so we encapsulate
# it all in one spot.
#
# note also that we don't handle all visual attributes here; just
# the ones that depend on the state of the widget. The rest are
# handled on a case by case basis
#
# Arguments:
#	 w	 widget pathname
#
# Returns:
#	 empty string

proc ::combobox::UpdateVisualAttributes {w} {

	upvar ::combobox::${w}::widgets		widgets
	upvar ::combobox::${w}::options		options

	if {$options(-state) == "normal"} {

		set foreground $options(-foreground)
		set background $options(-background)

	} elseif {$options(-state) == "disabled"} {

		set foreground $options(-disabledforeground)
		set background $options(-disabledbackground)
	}

	$widgets(entry)	  configure -foreground $foreground -background $background
	$widgets(listbox) configure -foreground $foreground -background $background
	$widgets(button)  configure -foreground $foreground
	$widgets(vsb)	  configure -background $background -troughcolor $background
	$widgets(frame)	  configure -background $background

	# we need to set the disabled colors in case our widget is disabled.
	# We could actually check for disabled-ness, but we also need to
	# check whether we're enabled but not editable, in which case the
	# entry widget is disabled but we still want the enabled colors. It's
	# easier just to set everything and be done with it.

	if {$::tcl_version >= 8.4} {
		$widgets(entry) configure \
			-disabledforeground $foreground \
			-disabledbackground $background
		$widgets(button)  configure -disabledforeground $foreground
		$widgets(listbox) configure -disabledforeground $foreground
	}
}

# ::combobox::SetValue --
#
#	 sets the value of the combobox and calls the -command,
#	 if defined
#
# Arguments:
#
#	 w			widget pathname
#	 newValue	the new value of the combobox
#
# Returns
#
#	 Empty string

proc ::combobox::SetValue {w newValue} {

	upvar ::combobox::${w}::widgets		widgets
	upvar ::combobox::${w}::options		options
	upvar ::combobox::${w}::ignoreTrace ignoreTrace
	upvar ::combobox::${w}::oldValue	oldValue

	if {[info exists options(-textvariable)] \
			&& [string length $options(-textvariable)] > 0} {
		set variable ::$options(-textvariable)
		set $variable $newValue
	} else {
		set oldstate [$widgets(entry) cget -state]
		$widgets(entry) configure -state normal
		$widgets(entry) delete 0 end
		$widgets(entry) insert 0 $newValue
		$widgets(entry) configure -state $oldstate
	}

	# set our internal textvariable; this will cause any public
	# textvariable (ie: defined by the user) to be updated as
	# well
#	 set ::combobox::${w}::entryTextVariable $newValue

	# redefine our concept of the "old value". Do it before running
	# any associated command so we can be sure it happens even
	# if the command somehow fails.
	set oldValue $newValue


	# call the associated command. The proc will handle whether or
	# not to actually call it, and with what args
	CallCommand $w $newValue

	return ""
}

# ::combobox::CallCommand --
#
#	calls the associated command, if any, appending the new
#	value to the command to be called.
#
# Arguments:
#
#	 w		   widget pathname
#	 newValue  the new value of the combobox
#
# Returns
#
#	 empty string

proc ::combobox::CallCommand {w newValue} {
	upvar ::combobox::${w}::widgets widgets
	upvar ::combobox::${w}::options options

	# call the associated command, if defined and -commandstate is
	# set to "normal"
	if {$options(-commandstate) == "normal" && \
			[string length $options(-command)] > 0} {
		set args [list $widgets(this) $newValue]
		uplevel \#0 $options(-command) $args
	}
}


# ::combobox::GetBoolean --
#
#	  returns the value of a (presumably) boolean string (ie: it should
#	  do the right thing if the string is "yes", "no", "true", 1, etc
#
# Arguments:
#
#	  value		  value to be converted
#	  errorValue  a default value to be returned in case of an error
#
# Returns:
#
#	  a 1 or zero, or the value of errorValue if the string isn't
#	  a proper boolean value

proc ::combobox::GetBoolean {value {errorValue 1}} {
	if {[catch {expr {([string trim $value])?1:0}} res]} {
		return $errorValue
	} else {
		return $res
	}
}

# ::combobox::convert --
#
#	  public routine to convert %x, %y and %W binding substitutions.
#	  Given an x, y and or %W value relative to a given widget, this
#	  routine will convert the values to be relative to the combobox
#	  widget. For example, it could be used in a binding like this:
#
#	  bind .combobox <blah> {doSomething [::combobox::convert %W -x %x]}
#
#	  Note that this procedure is *not* exported, but is intended for
#	  public use. It is not exported because the name could easily
#	  clash with existing commands.
#
# Arguments:
#
#	  w		a widget path; typically the actual result of a %W
#			substitution in a binding. It should be either a
#			combobox widget or one of its subwidgets
#
#	  args	should one or more of the following arguments or
#			pairs of arguments:
#
#			-x <x>		will convert the value <x>; typically <x> will
#						be the result of a %x substitution
#			-y <y>		will convert the value <y>; typically <y> will
#						be the result of a %y substitution
#			-W (or -w)	will return the name of the combobox widget
#						which is the parent of $w
#
# Returns:
#
#	  a list of the requested values. For example, a single -w will
#	  result in a list of one items, the name of the combobox widget.
#	  Supplying "-x 10 -y 20 -W" (in any order) will return a list of
#	  three values: the converted x and y values, and the name of
#	  the combobox widget.

proc ::combobox::convert {w args} {
	set result {}
	if {![winfo exists $w]} {
		error "window \"$w\" doesn't exist"
	}

	while {[llength $args] > 0} {
		set option [lindex $args 0]
		set args [lrange $args 1 end]

		switch -exact -- $option {
			-x {
				set value [lindex $args 0]
				set args [lrange $args 1 end]
				set win $w
				while {[winfo class $win] != "Combobox"} {
					incr value [winfo x $win]
					set win [winfo parent $win]
					if {$win == "."} break
				}
				lappend result $value
			}

			-y {
				set value [lindex $args 0]
				set args [lrange $args 1 end]
				set win $w
				while {[winfo class $win] != "Combobox"} {
					incr value [winfo y $win]
					set win [winfo parent $win]
					if {$win == "."} break
				}
				lappend result $value
			}

			-w -
			-W {
				set win $w
				while {[winfo class $win] != "Combobox"} {
					set win [winfo parent $win]
					if {$win == "."} break;
				}
				lappend result $win
			}
		}
	}
	return $result
}

# ::combobox::Canonize --
#
#	 takes a (possibly abbreviated) option or command name and either
#	 returns the canonical name or an error
#
# Arguments:
#
#	 w		  widget pathname
#	 object	  type of object to canonize; must be one of "command",
#			  "option", "scan command" or "list command"
#	 opt	  the option (or command) to be canonized
#
# Returns:
#
#	 Returns either the canonical form of an option or command,
#	 or raises an error if the option or command is unknown or
#	 ambiguous.

proc ::combobox::Canonize {w object opt} {
	variable widgetOptions
	variable columnOptions
	variable widgetCommands
	variable listCommands
	variable scanCommands

	switch $object {
		command {
			if {[lsearch -exact $widgetCommands $opt] >= 0} {
				return $opt
			}

			# command names aren't stored in an array, and there
			# isn't a way to get all the matches in a list, so
			# we'll stuff the commands in a temporary array so
			# we can use [array names]
			set list $widgetCommands
			foreach element $list {
				set tmp($element) ""
			}
			set matches [array names tmp ${opt}*]
		}

		{list command} {
			if {[lsearch -exact $listCommands $opt] >= 0} {
				return $opt
			}

			# command names aren't stored in an array, and there
			# isn't a way to get all the matches in a list, so
			# we'll stuff the commands in a temporary array so
			# we can use [array names]
			set list $listCommands
			foreach element $list {
				set tmp($element) ""
			}
			set matches [array names tmp ${opt}*]
		}

		{scan command} {
			if {[lsearch -exact $scanCommands $opt] >= 0} {
				return $opt
			}

			# command names aren't stored in an array, and there
			# isn't a way to get all the matches in a list, so
			# we'll stuff the commands in a temporary array so
			# we can use [array names]
			set list $scanCommands
			foreach element $list {
				set tmp($element) ""
			}
			set matches [array names tmp ${opt}*]
		}

		option {
			if {[info exists widgetOptions($opt)] \
					&& [llength $widgetOptions($opt)] == 2} {
				return $opt
			}
			set list [array names widgetOptions]
			set matches [array names widgetOptions ${opt}*]
		}
	}

	if {[llength $matches] == 0} {
		set choices [HumanizeList $list]
		error "unknown $object \"$opt\"; must be one of $choices"

	} elseif {[llength $matches] == 1} {
		set opt [lindex $matches 0]

		# deal with option aliases
		switch $object {
			option {
				set opt [lindex $matches 0]
				if {[llength $widgetOptions($opt)] == 1} {
					set opt $widgetOptions($opt)
				}
			}
		}

		return $opt

	} else {
		set choices [HumanizeList $list]
		error "ambiguous $object \"$opt\"; must be one of $choices"
	}
}

# ::combobox::HumanizeList --
#
#	 Returns a human-readable form of a list by separating items
#	 by columns, but separating the last two elements with "or"
#	 (eg: foo, bar or baz)
#
# Arguments:
#
#	 list	 a valid tcl list
#
# Results:
#
#	 A string which as all of the elements joined with ", " or
#	 the word " or "

proc ::combobox::HumanizeList {list} {

	if {[llength $list] == 1} {
		return [lindex $list 0]
	} else {
		set list [lsort $list]
		set secondToLast [expr {[llength $list] -2}]
		set most [lrange $list 0 $secondToLast]
		set last [lindex $list end]

		return "[join $most {, }] or $last"
	}
}

# This is some backwards-compatibility code to handle TIP 44
# (http://purl.org/tcl/tip/44.html). For all private tk commands
# used by this widget, we'll make duplicates of the procs in the
# combobox namespace.
#
# I'm not entirely convinced this is the right thing to do. I probably
# shouldn't even be using the private commands. Then again, maybe the
# private commands really should be public. Oh well; it works so it
# must be OK...
foreach command {TabToWindow CancelRepeat ListboxUpDown} {
	if {[llength [info commands ::combobox::tk$command]] == 1} break;

	set tmp [info commands tk$command]
	set proc ::combobox::tk$command
	if {[llength [info commands tk$command]] == 1} {
		set command [namespace which [lindex $tmp 0]]
		proc $proc {args} "uplevel $command \$args"
	} else {
		if {[llength [info commands ::tk::$command]] == 1} {
			proc $proc {args} "uplevel ::tk::$command \$args"
		}
	}
}

# end of combobox.tcl

######################################################################
# icon image data.
######################################################################
image create bitmap delta48 -data {
  #define delta48_width 48
  #define delta48_height 48
  static char delta48_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x80, 0x13, 0x00, 0x00,
  0x00, 0x00, 0xc0, 0x10, 0x00, 0x00, 0x00, 0x00, 0x40, 0x08, 0x00, 0x00,
  0x00, 0x00, 0x20, 0x08, 0x00, 0x00, 0x00, 0x00, 0x30, 0x0c, 0x00, 0x00,
  0x00, 0x00, 0x10, 0x04, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x0e, 0x00, 0x00,
  0x00, 0x00, 0x04, 0x1b, 0x00, 0x00, 0x00, 0x00, 0x06, 0x1b, 0x00, 0x00,
  0x00, 0x00, 0x02, 0x33, 0x00, 0x00, 0x00, 0x00, 0x03, 0x2e, 0x00, 0x00,
  0x00, 0x00, 0x11, 0x6c, 0x00, 0x00, 0x00, 0x00, 0x11, 0x68, 0x00, 0x00,
  0x00, 0x80, 0x10, 0xc8, 0x00, 0x00, 0x00, 0x80, 0x10, 0xa8, 0x01, 0x00,
  0x00, 0x80, 0x08, 0x08, 0x01, 0x00, 0x00, 0x80, 0x08, 0xac, 0x03, 0x00,
  0x00, 0x80, 0x09, 0x06, 0x02, 0x00, 0x00, 0xc0, 0x09, 0xaa, 0x06, 0x00,
  0x00, 0x40, 0x09, 0x01, 0x04, 0x00, 0x00, 0xe0, 0x93, 0xae, 0x0a, 0x00,
  0x00, 0x30, 0x92, 0x06, 0x18, 0x00, 0x00, 0xb0, 0x92, 0xad, 0x1a, 0x00,
  0x00, 0x18, 0x53, 0x04, 0x30, 0x00, 0x00, 0xa8, 0x11, 0xac, 0x2a, 0x00,
  0x00, 0x0c, 0x12, 0x04, 0x60, 0x00, 0x00, 0xac, 0x12, 0xac, 0x6a, 0x00,
  0x00, 0x02, 0x14, 0x04, 0x80, 0x00, 0x00, 0xab, 0x0a, 0xae, 0xaa, 0x01,
  0x00, 0x01, 0x28, 0x02, 0x00, 0x01, 0x80, 0xab, 0x3a, 0xaf, 0xaa, 0x03,
  0x80, 0x00, 0x70, 0x0c, 0x00, 0x02, 0xc0, 0xaa, 0x5a, 0xa8, 0xaa, 0x06,
  0x40, 0x00, 0xa0, 0x08, 0x00, 0x0c, 0xa0, 0xaa, 0xea, 0xac, 0xaa, 0x0a,
  0x30, 0x00, 0x80, 0x05, 0x00, 0x18, 0xb0, 0xaa, 0xaa, 0xab, 0xaa, 0x1a,
  0x08, 0x00, 0x00, 0x04, 0x00, 0x30, 0xfc, 0xff, 0xff, 0xbe, 0xff, 0x7f,
  0xfc, 0xff, 0xff, 0xbd, 0xff, 0x7f, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  }
}

image create photo deltaGif -format gif -data {
R0lGODlhMAAwAOf/AAUughEzYAQ5lgg3oh43YBQ4jQ46nyo1eBc9oyw7hA1CrQxDpxlAnyQ9nik/
exxApkE+QiNBlCVAmiBArQxJmhRGqxRItyVEqyhHlCZHpzRKXh5KsCpHrzpIeiBOkypLki9JnhVQ
tCNMswlTxD5HmS1MrSRPrz9LhylQnS1OqChPtiBSvz5Nki1Sui9Rvwtfr0hRhSdWw0RSiyZXvlBS
X1RPeyBazS5XzGROeSxZxzRco1ZUhVFWki9dxDBexR1j1Slhzk5bj1BamkRdrlZbi1ZcgGtXfyFn
02FgUi5mzVNgmlVhkERjuSVr0U9kkFNkmF1ijEVmtkxktT9ovWZigzNszDVtuR9x3VJnoF1nhSlx
0D1sxxJ442dmjDJyxG9ncnBliFVqsEdtw2lmmTFy2Xpjj6FiDIRkZkByx6VlAjB40atkB19vo2tt
k6hnBoJsXkR4x0p3wEx2xTN9z6ppCmVxrYZrfK9oDlh1unJxklR4si2B2WB0vKVsJ61sDoFwij2A
2oxxa5VucT+CyDiE0DWGy1GAt7JwFEmDuEeDxDWJ24t2hXt4rLVzGDqKz1KDzEiHwnF9ukCMxZx5
WEGL10WLzLl2GzyPzlyGxFiHy5d7dEqNwZp8art4J4Z/nEyN1WCJu1qLxJKAj0iSzFWO0MB8I5SC
hFKRzLp+McF9LFCTx2SOzF6RzpeGjr+CLlyUyWWTypOJoZWMp8aHNKKOi7SPZ8uMOKqRfamRg8aP
R86PQraUd7qUitKTRs+WRtCXSL6ad7aciLqaoLGfodSbUsCehrKfrr6ej7uikdCgYcahftifVcmi
c8Skc8ehlsmjecCli9uhWMqmid2kYdqnYsGpqsWsjuGoZcGul8ypqt6rZdKsidisd96sbdSuhNyu
c9evf9SwkuOxcdiyjs21rde2hOW0eue0dN62hum2b+G3gei2fOO4fOO5g+u4eNS9m9/BoPPAhvDB
me/Bn+/CpvHFiPTGkPXFtvnJmv/Ko/7KsPvNqv7Lt/7Opf/PwP/siyH+EUNyZWF0ZWQgd2l0aCBH
SU1QACH5BAEKAP8ALAAAAAAwADAAAAj+AP8JHEiwoEGDbA4qXMiwYSQxWHA0nEjRIJMtaDA9MVKx
48QoWyitWiXDo0mFTOQAYgXrEYyTMAnKIaUI1ikvO2LGjIKJEilWcIb80XmSEZ5XlE7BCaOpD1GP
U9Bk+pSUT7Ffh55WrBOHpaJPidgES+YqjdaJYUKxIrXn1KhNbZgRa3S2oR5Wrx6RcQvq2Ddqs+jU
VdgG1KlTigCNQhTEExi5ncwMRnhK1SkulEZJGjRIx65uugRPHvhHj9tRVxxdIqQmCYoz36KlMjv6
HxVDqkYRSs36SI4Igcx1w1r7HxRIo0bNOTJHC5AbMSJwMncOsJvaTiSNqtTkR5XnOWL+fAC2bp24
uaPfLJF0qRCQJEDCx2ghg1n56oEnT1IiiTUQGzbEIKALRnBTHjviLNPJYHTUEkQhWiRhQw7QxdCD
EMewU94652jTi2haNcJMF1bEN4KAM/QgBTbtbHhfNGVplcYs3bSiQ3gnztBCCnagg46LHA6X1VON
ECMONEIIaEELM5RQAy+05OENkPhdpxMdtmhzTjuiZBCDAi20EAENX3xQwDNAmodeTGukEs057Lwz
DAgrKKCCChmYkIQKEqBJJTW23BHTIb90c446svDhQwsGbKDCBpBuEIEyaZoXTSm0eTQjNeeUJ4oJ
M6xgwJ2PQoqCn1R66MdJRYrT6Tr+yGBw5wMcRHrBBi80UymH07gCIkV0zMLphuBQkYEKD1xQ6wUX
VICBfZV2+AtdFaVhyTLiAAnNCSpcYACzFVzAAAHQRgtYpg35oYuWQLZziwfeMtssBUgYuKuallCU
hpuvAonOIgwMUMHAGWhgzb33aWPLrwoRauiu27BgwAMVPAAABLfgMg7C51xqpUJrCNtvmuGwMIEB
FQMQgAMHBMOxh0Mq1CrC5eSxwcQKPGBCCB1IgzCH1MR4EJbsIizNEgxM/MDSWWz8czdrFpRGKdj+
XN42UDDwwMQMmKKO1fgx/A8dvRRt9TZjPLBAA0WAY3V5CVqCrhulvPn2gdaEIQCKCbjczaE2uqw6
UJHnxFPP4Ygnrng99lQDAg/wLC454u70ShsduriDTz+cd+75553vY0wK1ewD+umc4xNPNHRhmQ4+
+vAj++y01077PWUQkY0/tvfOjz75xDMXKtfEg4888ySv/PLMM+/MDljEIgw59DTPvDzy2HNOLnUZ
I0gHBSQQS3Hkl29+bQEBADs=
}

image create photo findImg -format gif -data {
R0lGODdhFAAUAPf/AAAAAIAAAACAAICAAAAAgIAAgACAgMDAwMDcwKbK8P/w1P/isf/Ujv/G
a/+4SP+qJf+qANySALl6AJZiAHNKAFAyAP/j1P/Hsf+rjv+Pa/9zSP9XJf9VANxJALk9AJYx
AHMlAFAZAP/U1P+xsf+Ojv9ra/9ISP8lJf4AANwAALkAAJYAAHMAAFAAAP/U4/+xx/+Oq/9r
j/9Ic/8lV/8AVdwASbkAPZYAMXMAJVAAGf/U8P+x4v+O1P9rxv9IuP8lqv8AqtwAkrkAepYA
YnMASlAAMv/U//+x//+O//9r//9I//8l//4A/twA3LkAuZYAlnMAc1AAUPDU/+Kx/9SO/8Zr
/7hI/6ol/6oA/5IA3HoAuWIAlkoAczIAUOPU/8ex/6uO/49r/3NI/1cl/1UA/0kA3D0AuTEA
liUAcxkAUNTU/7Gx/46O/2tr/0hI/yUl/wAA/gAA3AAAuQAAlgAAcwAAUNTj/7HH/46r/2uP
/0hz/yVX/wBV/wBJ3AA9uQAxlgAlcwAZUNTw/7Hi/47U/2vG/0i4/yWq/wCq/wCS3AB6uQBi
lgBKcwAyUNT//7H//47//2v//0j//yX//wD+/gDc3AC5uQCWlgBzcwBQUNT/8LH/4o7/1Gv/
xkj/uCX/qgD/qgDckgC5egCWYgBzSgBQMtT/47H/x47/q2v/j0j/cyX/VwD/VQDcSQC5PQCW
MQBzJQBQGdT/1LH/sY7/jmv/a0j/SCX/JQD+AADcAAC5AACWAABzAABQAOP/1Mf/sav/jo//
a3P/SFf/JVX/AEncAD25ADGWACVzABlQAPD/1OL/sdT/jsb/a7j/SKr/Jar/AJLcAHq5AGKW
AEpzADJQAP//1P//sf//jv//a///SP//Jf7+ANzcALm5AJaWAHNzAFBQAPLy8ubm5tra2s7O
zsLCwra2tqqqqp6enpKSkoaGhnp6em5ubmJiYlZWVkpKSj4+PjIyMiYmJhoaGg4ODv/78KCg
pICAgP8AAAD/AP//AAAA//8A/wD//////yH5BAEAAAEALAAAAAAUABQAQAjUAAMIHEiwoEF3
AOQpXMiQIQB3ARC6a6fO3buHAiVWfAcPYwB1AN6pa/fQnUkAIy+qEwiy3bp07DqaPPmS3TqS
Kz/SA8ATQDyB8XoCoJczI4B2F+VBjCjvocyBCNOVS9cxAE+rUqliRHhznbunEY96dbl15kyC
Zs8OrDgzJ1uTRVnSYzcO5M8AQeu6I0oQ5DukAOAJlglPJVR5gBMifNjUqTyoAM6NK1f1auTJ
YDuuOxdTKM/NneGFHVkRLEKKE0GeFGzRdODWMhd7Xipb6FKDuAsGBAA7
}

image create photo ctrCDRImg -format gif -data {
R0lGODlhFAAUAPcAAAAAAIAAAACAAICAAAAAgIAAgACAgMDAwMDcwKbK8P/w1P/isf/Ujv/G
a/+4SP+qJf+qANySALl6AJZiAHNKAFAyAP/j1P/Hsf+rjv+Pa/9zSP9XJf9VANxJALk9AJYx
AHMlAFAZAP/U1P+xsf+Ojv9ra/9ISP8lJf4AANwAALkAAJYAAHMAAFAAAP/U4/+xx/+Oq/9r
j/9Ic/8lV/8AVdwASbkAPZYAMXMAJVAAGf/U8P+x4v+O1P9rxv9IuP8lqv8AqtwAkrkAepYA
YnMASlAAMv/U//+x//+O//9r//9I//8l//4A/twA3LkAuZYAlnMAc1AAUPDU/+Kx/9SO/8Zr
/7hI/6ol/6oA/5IA3HoAuWIAlkoAczIAUOPU/8ex/6uO/49r/3NI/1cl/1UA/0kA3D0AuTEA
liUAcxkAUNTU/7Gx/46O/2tr/0hI/yUl/wAA/gAA3AAAuQAAlgAAcwAAUNTj/7HH/46r/2uP
/0hz/yVX/wBV/wBJ3AA9uQAxlgAlcwAZUNTw/7Hi/47U/2vG/0i4/yWq/wCq/wCS3AB6uQBi
lgBKcwAyUNT//7H//47//2v//0j//yX//wD+/gDc3AC5uQCWlgBzcwBQUNT/8LH/4o7/1Gv/
xkj/uCX/qgD/qgDckgC5egCWYgBzSgBQMtT/47H/x47/q2v/j0j/cyX/VwD/VQDcSQC5PQCW
MQBzJQBQGdT/1LH/sY7/jmv/a0j/SCX/JQD+AADcAAC5AACWAABzAABQAOP/1Mf/sav/jo//
a3P/SFf/JVX/AEncAD25ADGWACVzABlQAPD/1OL/sdT/jsb/a7j/SKr/Jar/AJLcAHq5AGKW
AEpzADJQAP//1P//sf//jv//a///SP//Jf7+ANzcALm5AJaWAHNzAFBQAPLy8ubm5tra2s7O
zsLCwra2tqqqqp6enpKSkoaGhnp6em5ubmJiYlZWVkpKSj4+PjIyMiYmJhoaGg4ODv/78KCg
pICAgP8AAAD/AP//AAAA//8A/wD//////yH5BAEAAAEALAAAAAAUABQAAAiUAAMIHBjAHYCD
ANwRHHjOncOHBgkRSgjRYUOEGAEYMpQRoUMA/8SJFGdwY0JyKFFSBGCuZcuSHN25bLmyo0aO
Nj+GJAkg0caNiU6q/DjToE9DQWW6rNkxUdCcBneONHhy5FCDM106zErzo82vB3XuTEm27Equ
aJd6BQsVpFSRZcmeTYuWKduM7hpW3Lv33MK/gAUGBAA7
}

image create photo firstCDRImg -format gif -data {
R0lGODlhFAAUAPcAAAAAAIAAAACAAICAAAAAgIAAgACAgMDAwMDcwKbK8P/w1P/isf/Ujv/G
a/+4SP+qJf+qANySALl6AJZiAHNKAFAyAP/j1P/Hsf+rjv+Pa/9zSP9XJf9VANxJALk9AJYx
AHMlAFAZAP/U1P+xsf+Ojv9ra/9ISP8lJf4AANwAALkAAJYAAHMAAFAAAP/U4/+xx/+Oq/9r
j/9Ic/8lV/8AVdwASbkAPZYAMXMAJVAAGf/U8P+x4v+O1P9rxv9IuP8lqv8AqtwAkrkAepYA
YnMASlAAMv/U//+x//+O//9r//9I//8l//4A/twA3LkAuZYAlnMAc1AAUPDU/+Kx/9SO/8Zr
/7hI/6ol/6oA/5IA3HoAuWIAlkoAczIAUOPU/8ex/6uO/49r/3NI/1cl/1UA/0kA3D0AuTEA
liUAcxkAUNTU/7Gx/46O/2tr/0hI/yUl/wAA/gAA3AAAuQAAlgAAcwAAUNTj/7HH/46r/2uP
/0hz/yVX/wBV/wBJ3AA9uQAxlgAlcwAZUNTw/7Hi/47U/2vG/0i4/yWq/wCq/wCS3AB6uQBi
lgBKcwAyUNT//7H//47//2v//0j//yX//wD+/gDc3AC5uQCWlgBzcwBQUNT/8LH/4o7/1Gv/
xkj/uCX/qgD/qgDckgC5egCWYgBzSgBQMtT/47H/x47/q2v/j0j/cyX/VwD/VQDcSQC5PQCW
MQBzJQBQGdT/1LH/sY7/jmv/a0j/SCX/JQD+AADcAAC5AACWAABzAABQAOP/1Mf/sav/jo//
a3P/SFf/JVX/AEncAD25ADGWACVzABlQAPD/1OL/sdT/jsb/a7j/SKr/Jar/AJLcAHq5AGKW
AEpzADJQAP//1P//sf//jv//a///SP//Jf7+ANzcALm5AJaWAHNzAFBQAPLy8ubm5tra2s7O
zsLCwra2tqqqqp6enpKSkoaGhnp6em5ubmJiYlZWVkpKSj4+PjIyMiYmJhoaGg4ODv/78KCg
pICAgP8AAAD/AP//AAAA//8A/wD//////yH5BAEAAAEALAAAAAAUABQAAAiUAAMIdFevoMGD
Bd0JXBig3j9ChAxJnDixHkOBDilqlGjxIkGEIBVevHjOnbtzI1MKLAkAwEmVJN0BIKTIJUqY
AVgS+neo5kuVOv9J7Gkzpc5BFIn+XHg06SGlN1fKbDlTYiKqRRmWNFnV0FWTS7XqtGoz6six
XrMClRkxbdizbMm+jQngUKK7ao1OxTo3JliTZgUGBAA7
}

image create photo prevCDRImg -format gif -data {
R0lGODdhFAAUAPf/AAAAAIAAAACAAICAAAAAgIAAgACAgMDAwMDcwKbK8P/w1P/isf/Ujv/G
a/+4SP+qJf+qANySALl6AJZiAHNKAFAyAP/j1P/Hsf+rjv+Pa/9zSP9XJf9VANxJALk9AJYx
AHMlAFAZAP/U1P+xsf+Ojv9ra/9ISP8lJf4AANwAALkAAJYAAHMAAFAAAP/U4/+xx/+Oq/9r
j/9Ic/8lV/8AVdwASbkAPZYAMXMAJVAAGf/U8P+x4v+O1P9rxv9IuP8lqv8AqtwAkrkAepYA
YnMASlAAMv/U//+x//+O//9r//9I//8l//4A/twA3LkAuZYAlnMAc1AAUPDU/+Kx/9SO/8Zr
/7hI/6ol/6oA/5IA3HoAuWIAlkoAczIAUOPU/8ex/6uO/49r/3NI/1cl/1UA/0kA3D0AuTEA
liUAcxkAUNTU/7Gx/46O/2tr/0hI/yUl/wAA/gAA3AAAuQAAlgAAcwAAUNTj/7HH/46r/2uP
/0hz/yVX/wBV/wBJ3AA9uQAxlgAlcwAZUNTw/7Hi/47U/2vG/0i4/yWq/wCq/wCS3AB6uQBi
lgBKcwAyUNT//7H//47//2v//0j//yX//wD+/gDc3AC5uQCWlgBzcwBQUNT/8LH/4o7/1Gv/
xkj/uCX/qgD/qgDckgC5egCWYgBzSgBQMtT/47H/x47/q2v/j0j/cyX/VwD/VQDcSQC5PQCW
MQBzJQBQGdT/1LH/sY7/jmv/a0j/SCX/JQD+AADcAAC5AACWAABzAABQAOP/1Mf/sav/jo//
a3P/SFf/JVX/AEncAD25ADGWACVzABlQAPD/1OL/sdT/jsb/a7j/SKr/Jar/AJLcAHq5AGKW
AEpzADJQAP//1P//sf//jv//a///SP//Jf7+ANzcALm5AJaWAHNzAFBQAPLy8ubm5tra2s7O
zsLCwra2tqqqqp6enpKSkoaGhnp6em5ubmJiYlZWVkpKSj4+PjIyMiYmJhoaGg4ODv/78KCg
pICAgP8AAAD/AP//AAAA//8A/wD//////yH5BAEAAAEALAAAAAAUABQAQAiGAAMIHCjwnDt3
5wgqLHjQHQBChgwlAtAw4cIABh9GnIjwIsOH/yIeUkTR4sWMECWW9DgQJcmOJx0SGhRR5KGR
Kxei3JjT406VMH06BECUaFCWGXsilfkP51GCKGnWdGryY9GUE4s+xfiT47mqCrsq1SmT51ao
ZYGCDevwUKK3Y8k2PLg2IAA7
}

image create photo nextCDRImg -format gif -data {
R0lGODdhFAAUAPf/AAAAAIAAAACAAICAAAAAgIAAgACAgMDAwMDcwKbK8P/w1P/isf/Ujv/G
a/+4SP+qJf+qANySALl6AJZiAHNKAFAyAP/j1P/Hsf+rjv+Pa/9zSP9XJf9VANxJALk9AJYx
AHMlAFAZAP/U1P+xsf+Ojv9ra/9ISP8lJf4AANwAALkAAJYAAHMAAFAAAP/U4/+xx/+Oq/9r
j/9Ic/8lV/8AVdwASbkAPZYAMXMAJVAAGf/U8P+x4v+O1P9rxv9IuP8lqv8AqtwAkrkAepYA
YnMASlAAMv/U//+x//+O//9r//9I//8l//4A/twA3LkAuZYAlnMAc1AAUPDU/+Kx/9SO/8Zr
/7hI/6ol/6oA/5IA3HoAuWIAlkoAczIAUOPU/8ex/6uO/49r/3NI/1cl/1UA/0kA3D0AuTEA
liUAcxkAUNTU/7Gx/46O/2tr/0hI/yUl/wAA/gAA3AAAuQAAlgAAcwAAUNTj/7HH/46r/2uP
/0hz/yVX/wBV/wBJ3AA9uQAxlgAlcwAZUNTw/7Hi/47U/2vG/0i4/yWq/wCq/wCS3AB6uQBi
lgBKcwAyUNT//7H//47//2v//0j//yX//wD+/gDc3AC5uQCWlgBzcwBQUNT/8LH/4o7/1Gv/
xkj/uCX/qgD/qgDckgC5egCWYgBzSgBQMtT/47H/x47/q2v/j0j/cyX/VwD/VQDcSQC5PQCW
MQBzJQBQGdT/1LH/sY7/jmv/a0j/SCX/JQD+AADcAAC5AACWAABzAABQAOP/1Mf/sav/jo//
a3P/SFf/JVX/AEncAD25ADGWACVzABlQAPD/1OL/sdT/jsb/a7j/SKr/Jar/AJLcAHq5AGKW
AEpzADJQAP//1P//sf//jv//a///SP//Jf7+ANzcALm5AJaWAHNzAFBQAPLy8ubm5tra2s7O
zsLCwra2tqqqqp6enpKSkoaGhnp6em5ubmJiYlZWVkpKSj4+PjIyMiYmJhoaGg4ODv/78KCg
pICAgP8AAAD/AP//AAAA//8A/wD//////yH5BAEAAAEALAAAAAAUABQAQAiGAAMIHHjOncGD
5wYqVFgQACFDhhIBcJdwIUN3DgsdUjSxokWBDR9G7PixIYCTIiWeJGmx4T9ChA6x/BggJESJ
FGnWtDmSoseLGSFC3DizJMaiNE2uRLrQ5U2mQFNCJYhRak6dPHH+vGjQ4VOETasWEmrokFmO
V6OOLYt2a1iHbXWGTbswIAA7
}

image create photo lastCDRImg -format gif -data {
R0lGODlhFAAUAPcAAAAAAIAAAACAAICAAAAAgIAAgACAgMDAwMDcwKbK8P/w1P/isf/Ujv/G
a/+4SP+qJf+qANySALl6AJZiAHNKAFAyAP/j1P/Hsf+rjv+Pa/9zSP9XJf9VANxJALk9AJYx
AHMlAFAZAP/U1P+xsf+Ojv9ra/9ISP8lJf4AANwAALkAAJYAAHMAAFAAAP/U4/+xx/+Oq/9r
j/9Ic/8lV/8AVdwASbkAPZYAMXMAJVAAGf/U8P+x4v+O1P9rxv9IuP8lqv8AqtwAkrkAepYA
YnMASlAAMv/U//+x//+O//9r//9I//8l//4A/twA3LkAuZYAlnMAc1AAUPDU/+Kx/9SO/8Zr
/7hI/6ol/6oA/5IA3HoAuWIAlkoAczIAUOPU/8ex/6uO/49r/3NI/1cl/1UA/0kA3D0AuTEA
liUAcxkAUNTU/7Gx/46O/2tr/0hI/yUl/wAA/gAA3AAAuQAAlgAAcwAAUNTj/7HH/46r/2uP
/0hz/yVX/wBV/wBJ3AA9uQAxlgAlcwAZUNTw/7Hi/47U/2vG/0i4/yWq/wCq/wCS3AB6uQBi
lgBKcwAyUNT//7H//47//2v//0j//yX//wD+/gDc3AC5uQCWlgBzcwBQUNT/8LH/4o7/1Gv/
xkj/uCX/qgD/qgDckgC5egCWYgBzSgBQMtT/47H/x47/q2v/j0j/cyX/VwD/VQDcSQC5PQCW
MQBzJQBQGdT/1LH/sY7/jmv/a0j/SCX/JQD+AADcAAC5AACWAABzAABQAOP/1Mf/sav/jo//
a3P/SFf/JVX/AEncAD25ADGWACVzABlQAPD/1OL/sdT/jsb/a7j/SKr/Jar/AJLcAHq5AGKW
AEpzADJQAP//1P//sf//jv//a///SP//Jf7+ANzcALm5AJaWAHNzAFBQAPLy8ubm5tra2s7O
zsLCwra2tqqqqp6enpKSkoaGhnp6em5ubmJiYlZWVkpKSj4+PjIyMiYmJhoaGg4ODv/78KCg
pICAgP8AAAD/AP//AAAA//8A/wD//////yH5BAEAAAEALAAAAAAUABQAAAiTAAMIHHjOncGD
5wYqVFgQgMOH7hIuZOgOwD9ChA4BiDiRokVDhhJtlNgxQENCIEVyLGmyIsqQI1meO5lyJEmK
BgG8VGnwZsuHOmtCvHmyEEiQh5IqiumRkNGjh5auXFgUqVSfTQtFZSrT5VWWHrmCFVhwakl3
9dKqXZvW3cR6F18enVvv7b+5eEHWXYiWrV+3AgMCADs=
}

image create photo rediffImg -format gif -data {
R0lGODdhFAAUAPf/AAAAAIAAAACAAICAAAAAgIAAgACAgMDAwMDcwKbK8P/w1P/isf/Ujv/G
a/+4SP+qJf+qANySALl6AJZiAHNKAFAyAP/j1P/Hsf+rjv+Pa/9zSP9XJf9VANxJALk9AJYx
AHMlAFAZAP/U1P+xsf+Ojv9ra/9ISP8lJf4AANwAALkAAJYAAHMAAFAAAP/U4/+xx/+Oq/9r
j/9Ic/8lV/8AVdwASbkAPZYAMXMAJVAAGf/U8P+x4v+O1P9rxv9IuP8lqv8AqtwAkrkAepYA
YnMASlAAMv/U//+x//+O//9r//9I//8l//4A/twA3LkAuZYAlnMAc1AAUPDU/+Kx/9SO/8Zr
/7hI/6ol/6oA/5IA3HoAuWIAlkoAczIAUOPU/8ex/6uO/49r/3NI/1cl/1UA/0kA3D0AuTEA
liUAcxkAUNTU/7Gx/46O/2tr/0hI/yUl/wAA/gAA3AAAuQAAlgAAcwAAUNTj/7HH/46r/2uP
/0hz/yVX/wBV/wBJ3AA9uQAxlgAlcwAZUNTw/7Hi/47U/2vG/0i4/yWq/wCq/wCS3AB6uQBi
lgBKcwAyUNT//7H//47//2v//0j//yX//wD+/gDc3AC5uQCWlgBzcwBQUNT/8LH/4o7/1Gv/
xkj/uCX/qgD/qgDckgC5egCWYgBzSgBQMtT/47H/x47/q2v/j0j/cyX/VwD/VQDcSQCrPQCW
MQBzJQBQGdT/1LH/sY7/jmv/a0j/SCX/JQD+AADcAAC5AACWAABzAABQAOP/1Mf/sav/jo//
a3P/SFf/JVX/AEncAD25ADGWACVzABlQAPD/1OL/sdT/jsb/a7j/SKr/Jar/AJLcAHq5AGKW
AEpzADJQAP//1P//sf//jv//a///SP//Jf7+ANzcALm5AJaWAHNzAFBQAPLy8ubm5tra2s7O
zsLCwra2tqqqqp6enpKSkoaGhnp6em5ubmJiYlZWVkpKSj4+PjIyMiYmJhoaGg4ODv/78KCg
pICAgP8AAAD/AP//AAAA//8A/wD//////yH5BAEAAAEALAAAAAAUABQAQAicAAMIHEiwoMF0
7AD0euVKl8OHrhjqAgDvnDsAGDOmG2jR3TmDIAVaxFiRoMJXKF/1ypgR5UqPIWOCTIfQnc2b
ABpS/Bgg3cmUQIOqBHBxIUpYADYKLEqUp8ynUKMatFgy5LmrWEdOrDoQIcuvrnSWPJfQqFCg
YhPCAtqrrduUL8/9fIWUJs2LQ2EGmFt34MWmBNPdvKlUquEAAQEAOw==
}

image create photo ignCDRImg -format gif -data {
R0lGODlhFgAWAKIAANnZ2d0AAJ6enmJiYgAAAAC5AACWMQBQACH5BAEAAAAALAAAAAAWABYA
AANwCLrc/jBKF8JcgtU6xSBDtlmRR2QCMZZfVhjGBj6mUrzxBoAZ9tmwXKWg4ClqAFzssHkV
Q8gkLHAAMHHEnSD62lyDhiLqBxAOwc9ebRRQhnchhoeNTlNW5QXB2Bi1MHx9OgApH38RHA09
F4yNjo8MCQA7
}

image create photo bkmSetImg -format gif -data {
R0lGODlhFAAUAPcAAAAAAIAAAACAAICAAAAAgIAAgACAgMDAwMDcwKbK8P/w1Pjisd/UjtHJ
a8O4SL2qJcWqAK+SAJN6AGJiAEpKADIyAP/j1P/Hsf+rjv+Pa/9zSP9XJf9VANxJALk9AJYx
AHMlAFAZAP/U1P+xsf+Ojv9ra/9ISP8lJf4AANwAALkAAJYAAHMAAFAAAP/U4/+xx/+Oq/9r
j/9Ic/8lV/8AVdwASbkAPZYAMXMAJVAAGf/U8P+x4v+O1P9rxv9IuP8lqv8AqtwAkrkAepYA
YnMASlAAMv/U//+x//+O//9r//9I//8l//4A/twA3LkAuZYAlnMAc1AAUPDU/+Kx/9SO/8Zr
/7hI/6ol/6oA/5IA3HoAuWIAlkoAczIAUOPU/8ex/6uO/49r/3NI/1cl/1UA/0kA3D0AuTEA
liUAcxkAUNTU/7Gx/46O/2tr/0hI/yUl/wAA/gAA3AAAuQAAlgAAcwAAUNTj/7HH/46r/2uP
/0hz/yVX/wBV/wBJ3AA9uQAxlgAlcwAZUNTw/7Hi/47U/2vG/0i4/yWq/wCq/wCS3AB6uQBi
lgBKcwAyUNT//7H//47//2v//0j//yX//wD+/gDc3AC5uQCWlgBzcwBQUNT/8LH/4o7/1Gv/
xkj/uCX/qgD/qgDckgC5egCWYgBzSgBQMtT/47H/x47/q2v/j0j/cyX/VwD/VQDcSQC5PQCW
MQBzJQBQGdT/1LH/sY7/jmv/a0j/SCX/JQD+AADcAAC5AACWAABzAABQAOP/1Mf/sav/jo//
a3P/SFf/JVX/AEncAD25ADGWACVzABlQAPD/1OL/sdT/jsb/a7j/SKr/Jar/AJLcAHq5AGKW
AEpzADJQAP//1P//sf//jv//a///SP//Jf7+ANzcALm5AJaWAHNzAFBQAPLy8ubm5tra2s7O
zsLCwra2tqqqqp6enpKSkoaGhnp6em5ubmJiYlZWVkpKSj4+PjIyMiYmJhoaGg4ODv/78KCg
pICAgP8AAAD/AP//AAAA//8A/wD//////yH5BAEAAAEALAAAAAAUABQAAAiZAAMIHEhQoLqD
CAsqFAigIQB3Dd0tNKjOXSxXrmABWBABgLqCByECuAir5EYJHimKvOgqFqxXrzZ2lBhgJUaY
LV/GOpkSIqybOF3ClPlQIEShMF/lfLVzAcqPRhsKXRqTY1GCFaUy1ckTKkiRGhtapTkxa82u
ExUSJZs2qtOUbQ2ujTsQ4luvbdXNpRtA712+UeEC7ou3YEAAADt=
}

image create photo bkmRlsImg -format gif -data {
R0lGODlhFAAUAPcAAAAAAIAAAACAAICAAAAAgIAAgACAgMDAwMDcwKbK8P/w1Pjisd/UjtHJ
a8O4SL2qJcWqAK+SAJN6AGJiAEpKADIyAP/j1P/Hsf+rjv+Pa/9zSP9XJf9VANxJALk9AJYx
AHMlAFAZAP/U1P+xsf+Ojv9ra/9ISP8lJf4AANwAALkAAJYAAHMAAFAAAP/U4/+xx/+Oq/9r
j/9Ic/8lV/8AVdwASbkAPZYAMXMAJVAAGf/U8P+x4v+O1P9rxv9IuP8lqv8AqtwAkrkAepYA
YnMASlAAMv/U//+x//+O//9r//9I//8l//4A/twA3LkAuZYAlnMAc1AAUPDU/+Kx/9SO/8Zr
/7hI/6ol/6oA/5IA3HoAuWIAlkoAczIAUOPU/8ex/6uO/49r/3NI/1cl/1UA/0kA3D0AuTEA
liUAcxkAUNTU/7Gx/46O/2tr/0hI/yUl/wAA/gAA3AAAuQAAlgAAcwAAUNTj/7HH/46r/2uP
/0hz/yVX/wBV/wBJ3AA9uQAxlgAlcwAZUNTw/7Hi/47U/2vG/0i4/yWq/wCq/wCS3AB6uQBi
lgBKcwAyUNT//7H//47//2v//0j//yX//wD+/gDc3AC5uQCWlgBzcwBQUNT/8LH/4o7/1Gv/
xkj/uCX/qgD/qgDckgC5egCWYgBzSgBQMtT/47H/x47/q2v/j0j/cyX/VwD/VQDcSQC5PQCW
MQBzJQBQGdT/1LH/sY7/jmv/a0j/SCX/JQD+AADcAAC5AACWAABzAABQAOP/1Mf/sav/jo//
a3P/SFf/JVX/AEncAD25ADGWACVzABlQAPD/1OL/sdT/jsb/a7j/SKr/Jar/AJLcAHq5AGKW
AEpzADJQAP//1P//sf//jv//a///SP//Jf7+ANzcALm5AJaWAHNzAFBQAPLy8ubm5tra2s7O
zsLCwra2tqqqqp6enpKSkoaGhnp6em5ubmJiYlZWVkpKSj4+PjIyMiYmJhoaGg4ODv/78KCg
pICAgP8AAAD/AP//AAAA//8A/wD//////yH5BAEAAAEALAAAAAAUABQAAAiwAAMIHEhQoLqD
CAsCWKhwIbyFANwNXBiD4UF3sVw9rLhQXQCKNTguzLgxZMePMWqo5OgqVkmVNwAIXHhDpUl3
7gCkhMkwJ02bHHfWiCkzQM5YP1cKJepRoM+kNoculEhQXc6cNW3GzNm0oFWdUSviLDgRbFST
RRsuzYpWrVaoHMsujYgVKMOPUYkCWPCQbY2iP/UuiACgr9S0NDvulQBAXd+7ZYv6bPowLdmB
By8LDAgAOw==
}

image create photo mrgC1Img -format gif -data {
R0lGODdhFAAUAPf/AAAAAIAAAACAAICAAAAAgIAAgACAgMDAwMDcwKbK8P/w1P/isf/Ujv/G
a/+4SP+qJf+qANySALl6AJZiAHNKAFAyAP/j1P/Hsf+rjv+Pa/9zSP9XJf9VANxJALk9AJYx
AHMlAFAZAP/U1P+xsf+Ojv9ra/9ISP8lJf4AANwAALkAAJYAAHMAAFAAAP/U4/+xx/+Oq/9r
j/9Ic/8lV/8AVdwASbkAPZYAMXMAJVAAGf/U8P+x4v+O1P9rxv9IuP8lqv8AqtwAkrkAepYA
YnMASlAAMv/U//+x//+O//9r//9I//8l//4A/twA3LkAuZYAlnMAc1AAUPDU/+Kx/9SO/8Zr
/7hI/6ol/6oA/5IA3HoAuWIAlkoAczIAUOPU/8ex/6uO/49r/3NI/1cl/1UA/0kA3D0AuTEA
liUAcxkAUNTU/7Gx/46O/2tr/0hI/yUl/wAA/gAA3AAAuQAAlgAAcwAAUNTj/7HH/46r/2uP
/0hz/yVX/wBV/wBJ3AA9uQAxlgAlcwAZUNTw/7Hi/47U/2vG/0i4/yWq/wCq/wCS3AB6uQBi
lgBKcwAyUNT//7H//47//2v//0j//yX//wD+/gDc3AC5uQCWlgBzcwBQUNT/8LH/4o7/1Gv/
xkj/uCX/qgD/qgDckgC5egCWYgBzSgBQMtT/47H/x47/q2v/j0j/cyX/VwD/VQDcSQC5PQCW
MQBzJQBQGdT/1LH/sY7/jmv/a0j/SCX/JQD+AADcAAC5AACWAABzAABQAOP/1Mf/sav/jo//
a3P/SFf/JVX/AEncAD25ADGWACVzABlQAPD/1OL/sdT/jsb/a7j/SKr/Jar/AJLcAHq5AGKW
AEpzADJQAP//1P//sf//jv//a///SP//Jf7+ANzcALm5AJaWAHNzAFBQAPLy8ubm5tra2s7O
zsLCwra2tqqqqp6enpKSkoaGhnp6em5ubmJiYlZWVkpKSj4+PjIyMiYmJhoaGg4ODv/78KCg
pICAgP8AAAD/AP//AAAA//8A/wD//////yH5BAEAAAEALAAAAAAUABQAQAiIAAMIHEiwYMFz
7gAQ+meoIaGHECEeAuDuoDt35wxqFIgQAMWMGzkmVHRooseTKD1WPAgy5MCOhAZRvEizJsaR
hxrq3LkzEcWXIz+eG0qUqMujSJMixJg0AEyhRYuKVDjIUMqrMxUy5MnVkM+bAEgaOpSorNmz
X6eSnGmzZkunCT825fh2btKAADt=
}

image create photo mrgC2Img -format gif -data {
R0lGODdhFAAUAPf/AAAAAIAAAACAAICAAAAAgIAAgACAgMDAwMDcwKbK8P/w1P/isf/Ujv/G
a/+4SP+qJf+qANySALl6AJZiAHNKAFAyAP/j1P/Hsf+rjv+Pa/9zSP9XJf9VANxJALk9AJYx
AHMlAFAZAP/U1P+xsf+Ojv9ra/9ISP8lJf4AANwAALkAAJYAAHMAAFAAAP/U4/+xx/+Oq/9r
j/9Ic/8lV/8AVdwASbkAPZYAMXMAJVAAGf/U8P+x4v+O1P9rxv9IuP8lqv8AqtwAkrkAepYA
YnMASlAAMv/U//+x//+O//9r//9I//8l//4A/twA3LkAuZYAlnMAc1AAUPDU/+Kx/9SO/8Zr
/7hI/6ol/6oA/5IA3HoAuWIAlkoAczIAUOPU/8ex/6uO/49r/3NI/1cl/1UA/0kA3D0AuTEA
liUAcxkAUNTU/7Gx/46O/2tr/0hI/yUl/wAA/gAA3AAAuQAAlgAAcwAAUNTj/7HH/46r/2uP
/0hz/yVX/wBV/wBJ3AA9uQAxlgAlcwAZUNTw/7Hi/47U/2vG/0i4/yWq/wCq/wCS3AB6uQBi
lgBKcwAyUNT//7H//47//2v//0j//yX//wD+/gDc3AC5uQCWlgBzcwBQUNT/8LH/4o7/1Gv/
xkj/uCX/qgD/qgDckgC5egCWYgBzSgBQMtT/47H/x47/q2v/j0j/cyX/VwD/VQDcSQC5PQCW
MQBzJQBQGdT/1LH/sY7/jmv/a0j/SCX/JQD+AADcAAC5AACWAABzAABQAOP/1Mf/sav/jo//
a3P/SFf/JVX/AEncAD25ADGWACVzABlQAPD/1OL/sdT/jsb/a7j/SKr/Jar/AJLcAHq5AGKW
AEpzADJQAP//1P//sf//jv//a///SP//Jf7+ANzcALm5AJaWAHNzAFBQAPLy8ubm5tra2s7O
zsLCwra2tqqqqp6enpKSkoaGhnp6em5ubmJiYlZWVkpKSj4+PjIyMiYmJhoaGg4ODv/78KCg
pICAgP8AAAD/AP//AAAA//8A/wD//////yH5BAEAAAEALAAAAAAUABQAQAiNAAMIHEiwYEF3
AP79GzSIkMOHhAwZKkQIgLtzBguec3cxo8eNACxiHIgwpMmTIQ8dUiTSo8aRBDdynEkTIcWW
ARBGlMizJ8+VFgOcG0q0KEKWHV0qXcp0qUyYA4tKBVkxaU6UWAFMrIoR4SCfYCXe5AjgUKKz
aNMeMgT0osyaNMsihfqxpNWmQ5s2DQgAOw==
}

image create photo mrgC12Img -format gif -data {
R0lGODlhFAAUAPMHAAAAAAB6uQCS3CWq/0i4/47U/7Hi/////729vQAAAAAAAAAAAAAAAAAAAAAA
AAAAACH5BAEAAAgALAAAAAAUABQAAAT+ECGEECgAIYQQggghhBBCCIFiAEQIIYQQQgghhCACxRAA
AAAAAAABAAghUA4hpBRYSimllAEQAuVAQgghhBBCCCECAoRAGIQQQgghkBBCiAAIIRAGgUMIIYQQ
QggBEEQIgTAGAAAAACAAAACEEEIgDAARQgghhBBCCCGIEAIBIIQQQghBhBBCCCGEEEIIIgQKQAgh
hBBCECGEEEIImAIQggghAAAAAAAAAATEFIAQQmCUUmAppZRCCDkFIAQREIQQQgghhBBIyCkAISAI
IYRAQgghhJARAEIACiGEEEIIIQYZMACEEAAAAAAAgACAMQJACCGEEEQIIYQQAiMAhCAPQgghhBBC
CCEEQQAIIYQiADs=
}

image create photo mrgC21Img -format gif -data {
R0lGODlhFAAUAPMHAAAAAAB6uQCS3CWq/0i4/47U/7Hi/////729vQAAAAAAAAAAAAAAAAAAAAAA
AAAAACH5BAEAAAgALAAAAAAUABQAAAT+ECGEEEIIIYRAgQAhhBBCCCGEEEQIIWAKQAghBCAAAAAA
AACAmAIBQgiBUUoppRRYCiHkFIAQAoJAQgghhBBCCDkFAoSAIIQQQgghkBBCRgAIASGEgEIIIYQY
ZASAEEQAAAAAAAAAMOAIACGEEEIIIQQRQgiMABBCCCGIEEIIIYQQCABBhBBCCCEECkAIIoQQQggh
hBBCEBQDEEIIIYQQggghhEAxBAAAAAQAAAAAQgiUQyAhpZRSSillAAQRKIcQQgghhBBICBEAIRAG
IYRAQgghhBAiAEIIgjDIEEIIIYQQUAiAEEIgjAEAgAAAAAAAACGEEARhAIQQQgghhCAPQgghhEAA
CCEEEUIIIYQiADs=
}

image create photo splitCDRImg -format gif -data {
R0lGODlhFgAWALMAANnZ2ba2tkpKSp6enmJiYgAAAAC5AACWMQBQAP//////////////////
/////////yH5BAEAAAAALAAAAAAWABYAAASKEMhJaRAD41G7DEQhipjXBYWhqoVgWmBxzEjB
vUAQG/NRuy9diNercXTIJGHYOxR+gcFyOhURfYUQYTAYeUdXI4Cbk63O4Wyl22z3bB22uw2v
oHyIvL5pUFO6X158cGQ6XIeHIoNaR0lJXDI9fT84hpFFdUFRl1hAlTGYN5+cTp44Ul8lOBMZ
rRsRADs=
}

image create photo cmbinCDRImg -format gif -data {
R0lGODlhFgAWAKIAANnZ2ba2tkpKSgAAAJ6engC5AACWMQBQACH5BAEAAAAALAAAAAAWABYA
AAOACLrcEGKQ4OqCowxBbcOFYUgeA4riUCqneGwm8QUZ+spXhCtE7cK5wUgw6YV+u0ckNGg2
C8ehaSmCWqM3hhHF7ZK0wq54lFQODq6DuvvqXHpoZ5Or4XwiL2KgR9+4WT1JfCh1fw9lATR9
dit7YVVAjRFcLytvYVmWLJN+mpcTAAkAOw==
}

image create photo fldrImg -format gif -data {
R0lGODlhFAAWAKIAANnZ2QAAAP/MmZlmMzMzM////////////yH5BAEAAAAALAAAAAAUABYA
AANUCLrc/tCFSWdUQeitQ8xcWFnYEG6miAlD67Yn64Hx2RJTXQ84raO83C8U9A1vwiGqpwQy
m5oilCVlWU3YKwsHCLy+YAK3Ky6bzzjCYsSuqC/w+CMBADs=
}

image create photo txtfImg -format gif -data {
R0lGODlhFAAWAKIAANnZ2TMzM////wAAAJmZmf///////////yH5BAEAAAAALAAAAAAUABYA
AANYGLq88BAEQaudIb5pO88R11UiuI3XBXFD61JDEM8nCrtujbcW4RODmq3yC0puuxcFKBwS
jaykUsA8OntQpPTZvFZF2un3iu1ul1kyuuv8Bn7wuE8WkdqNCQA7
}

image create photo ancfImg -format gif -data {
R0lGODlhFAAUAJEAANnZ2QAAAD8/P////yH5BAEAAAAALAAAAAAUABQAAAJKRI6ZwB0N4Xsy
WkpZttp57igdaCgYiVQGuAiAcEaHtsUNjNUjXfYMPFqUZp8MMaTaXDLAFUcYRB2dyovrZSMl
r9yX1yVoDk3kRwEAOw==
}

image create photo bkmImg -format gif -data {
R0lGODlhLgAWAJEAANnZ2czMzD8/P////yH5BAEAAAAALAAAAAAuABYAAAJyjI+py20CI3S0
JgFyFrZXrHHeyECbhKbqipYAqMXyTNPiYtb6Pt9KzgvqWgcMTIiMwUS5YzK5fISe1FdIuqk+
QZPAEgAOi8fkMtmIQJnX7HIX14633z+53f0qsfZ89ctHEuglBijo4VRo+KGi2Oj4eFAAADs=
}

image create photo nullImg

image create bitmap resize -data {
	#define resize_width 14
	#define resize_height 11
	static char resize_bits[] = {
		0x20, 0x01, 0x30, 0x03, 0x38, 0x07, 0x3c, 0x0f, 0x3e, 0x1f, 0x3f, 0x3f,
		0x3e, 0x1f, 0x3c, 0x0f, 0x38, 0x07, 0x30, 0x03, 0x20, 0x01
	}
}

# Despite the common naming the U/D arrows are LARGER than the L/R ones
#	(the former is used on a Dialog, the latter inside the toolbar)

image create bitmap arroWu -data {
	#define arroWu_width 29
	#define arroWu_height 15
	static unsigned char arroWu_bits[] = {
		0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x40,0x00,0x00,
		0x00,0xe0,0x00,0x00, 0x00,0xf0,0x01,0x00, 0x00,0xf8,0x03,0x00,
		0x00,0xfc,0x07,0x00, 0x00,0xfe,0x0f,0x00, 0x00,0xff,0x1f,0x00,
		0x80,0xff,0x3f,0x00, 0xc0,0xff,0x7f,0x00, 0xe0,0xff,0xff,0x00,
		0xf0,0xff,0xff,0x01, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00
	}
}

image create bitmap arroWd -data {
	#define arroWd_width 29
	#define arroWd_height 15
	static unsigned char arroWd_bits[] = {
		0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0xf0,0xff,0xff,0x01,
		0xe0,0xff,0xff,0x00, 0xc0,0xff,0x7f,0x00, 0x80,0xff,0x3f,0x00,
		0x00,0xff,0x1f,0x00, 0x00,0xfe,0x0f,0x00, 0x00,0xfc,0x07,0x00,
		0x00,0xf8,0x03,0x00, 0x00,0xf0,0x01,0x00, 0x00,0xe0,0x00,0x00,
		0x00,0x40,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00
	}
}

image create bitmap arroWl -data {
	#define arroWl_width 10
	#define arroWl_height 21
	static unsigned char arroWl_bits[] = {
		0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x01, 0x80,0x01,0xc0,0x01,
		0xe0,0x01,0xf0,0x01, 0xf8,0x01,0xfc,0x01, 0xfe,0x01,0xfc,0x01,
		0xf8,0x01,0xf0,0x01, 0xe0,0x01,0xc0,0x01, 0x80,0x01,0x00,0x01,
		0x00,0x00,0x00,0x00, 0x00,0x00
	}
}

image create bitmap arroWr -data {
	#define arroWr_width 10
	#define arroWr_height 21
	static unsigned char arroWr_bits[] = {
		0x00,0x00,0x00,0x00, 0x00,0x00,0x02,0x00, 0x06,0x00,0x0e,0x00,
		0x1e,0x00,0x3e,0x00, 0x7e,0x00,0xfe,0x00, 0xfe,0x01,0xfe,0x00,
		0x7e,0x00,0x3e,0x00, 0x1e,0x00,0x0e,0x00, 0x06,0x00,0x02,0x00,
		0x00,0x00,0x00,0x00, 0x00,0x00
	}
}

###############################################################################

# run the main proc
main
