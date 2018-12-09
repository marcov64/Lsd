#*************************************************************
# DBLCLICK.TCL
# Enhancements to double-click behavior in text widgets.
#
# package doubleclick
#
# (c) Wolf-Dieter Busch
#
# license: OLL (One Line Licence):
# Use it, change it, but do not blame me.
#
# changes behaviour of mouse <Double-1> as follows:
# <Double-1> on word char selects word characters only
# <Double-1> on other char selects non-space characters
# <Double-1> on opening brace selects to matching counterpart
# <Double-1> on opening paren or brace or double quote does the same
#
# changed binding on tag Text and event <Double-1>
# changed procedure: ::tk::TextSelectTo
# changed procedure: ::tk::TextNextPos
# new procedure: ::tk::TextCharAtXYescaped
# new procedure: tcl_findClosingBrace
#*************************************************************

package require Tk 8.5

proc ::tk::selectToClosingChar {w x y} {
  set i0 [$w index @$x,$y]
  set transList [list \u007b \u007d\
                   \" \" ' ' „ “ ‚ ‘  “ ” ‘ ’\
                   \u00bb \u00ab \u00ab \u00bb\
                   \u203a \u2039 \u2039 \u203a\
                   \u005b \u005d < > \u0028 \u0029]
  set c0 [$w get $i0]
  set selectTo {{w i0 i1} {
      if {[$w tag ranges sel] eq ""} then {
        $w tag add sel $i0 $i1
        $w mark set insert $i0
      } else {
        $w tag add sel sel.first $i1
        $w mark set insert $i1
      }
    }}
  if {![dict exists $transList $c0]} then {
    return false
  }
  set c1 [dict get $transList $c0]
  if {$c0 ni [list \[ \( \{ \" <]} then {
    # Quotes - non-nestable
    set i1 [$w search $c1 $i0+1chars end]
    if {$i1 eq ""} then {
      return false
    }
    # $w tag add sel $i0 $i1+1chars
    # $w mark set insert $i0
    apply $selectTo $w $i0 $i1+1chars
    return true
  } elseif {$c0 eq "<"} then {
    # HTML tags?
    set i1 [$w search > $i0+1chars end]
    if {$i1 eq ""} then {
      # no closing char > - not an HTML tag
      return false
    }
    if {[$w search / $i0 $i1] ne ""} then {
      # closing or empty tag - non-nestable
      apply $selectTo $w $i0 $i1+1chars
      return true
    } else {
      # opening tag - nestable
      set txt [string trim [$w get $i0+1chars $i1]]
      set name [lindex [split $txt] 0]
      set open <\\s*$name\[^>\]*>
      set close <\\s*/\\s*$name\\s*>
      set i1 $i0
      while true {
        set i1 [$w search -regexp $close $i1 end]
        if {$i1 eq ""} then {
          return false
        }
        set i1 [$w index [$w search > $i1 end]+1chars]
        set txt [$w get $i0 $i1]
        set txt [string map [list \{ " " \} " " \" " "] $txt]
        regsub -all $open $txt \{ txt
        regsub -all $close $txt \} txt
        if {[info complete $txt]} then {
          apply $selectTo $w $i0 $i1
          return true
        }
      }
    }
    return false
  } else {
    # braces, brackets - nestable
    if {$c0 in [list \{ \"]} then {
      set map {}
    } else {
      set map [list \{ " " \} " " \" " " $c0 \{ $c1 \}]
    }
    set i1 $i0
    while true {
      set i1 [$w search $c1 $i1+1chars end]
      if {$i1 eq ""} then {
        return false
      }
      if {[info complete\
             [string map $map\
                [$w get $i0 $i1+1chars]]]} then {
        apply $selectTo $w $i0 $i1+1chars
        return true
      }
    }
  }
}

proc setDoubleclickBinding textWidget {
  bind $textWidget <Double-Button-1> {
    set ::tk::Priv(selectMode) word
    if {[::tk::selectToClosingChar %W %x %y]} then break
  }
  bind $textWidget <Shift-Button-1> {
    if {$::tk::Priv(selectMode) ne "word"} then continue
    if {[::tk::selectToClosingChar %W %x %y]} then break
  }
  bind $textWidget <B1-Motion> {
    if {$::tk::Priv(selectMode) ne "word"} then continue
    if {[::tk::selectToClosingChar %W %x %y]} then break
  }
}
