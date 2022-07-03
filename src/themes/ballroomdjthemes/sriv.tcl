# sriv.tcl - Copyright (C) 2004 Pat Thoyts <patthoyts@users.sourceforge.net>
#
# $Id: sriv.tcl,v 1.1.1.1 2005/11/19 23:36:30 jelco Exp $
#
#

namespace eval ttk::theme::sriv {

  package provide ttk::theme::sriv 0.5

  proc LoadImages {imgdir {patterns {*.gif}}} {
    foreach pattern $patterns {
      foreach file [glob -directory $imgdir $pattern] {
        set img [file tail [file rootname $file]]
        if {![info exists images($img)]} {
          set images($img) [image create photo -file $file]
        }
      }
    }
    return [array get images]
  }

  set imgdir [file join [file dirname [info script]] i sriv]
  array set I [LoadImages $imgdir *.gif]

  array set colors {
    -frame    "#a0a0a0"
    -lighter  "#c3c3c3"
    -window     "#e6f3ff"
    -selectbg  "#ffff33"
    -selectfg  "#000000"
    -disabledfg  "#666666"
  }

  ttk::style theme create sriv -settings {
    ttk::style configure . \
        -borderwidth   1 \
        -background   $colors(-frame) \
        -fieldbackground  $colors(-window) \
        -troughcolor  $colors(-lighter) \
        -selectbackground  $colors(-selectbg) \
        -selectforeground  $colors(-selectfg) \
        ;
    ttk::style map . -foreground [list disabled $colors(-disabledfg)]

    ## Buttons.
    #
    ttk::style layout TButton {
      Button.background
      Button.button -children {
          Button.focus -children {
              Button.label
          }
      }
    }

    ttk::style layout Toolbutton {
      Toolbutton.background
      Toolbutton.button -children {
          Toolbutton.focus -children {
              Toolbutton.label
          }
      }
    }

    ttk::style element create button image [list \
        $I(button-n) \
        {pressed !disabled} $I(button-p) \
        {active !selected}  $I(button-h) \
        selected $I(button-s) \
        disabled $I(button-d)] \
        -border {4 4 4 10} -padding {2 2} -sticky news

  ########## red
    ttk::style layout Red.TButton {
      Button.background
      Button.button-red -children {
          Button.focus -children {
              Button.label
          }
      }
    }

    ttk::style element create button-red image [list \
        $I(button-red-n) \
        {pressed !disabled} $I(button-p) \
        {active !selected}  $I(button-h) \
        selected $I(button-s) \
        disabled $I(button-d)] \
        -border {4 4 4 10} -padding {2 2} -sticky news

  ########## yellow
    ttk::style layout Yellow.TButton {
      Button.background
      Button.button-yellow -children {
          Button.focus -children {
              Button.label
          }
      }
    }

    ttk::style element create button-yellow image [list \
        $I(button-yellow-n) \
        {pressed !disabled} $I(button-p) \
        {active !selected}  $I(button-h) \
        selected $I(button-s) \
        disabled $I(button-d)] \
      -border {4 4 4 10} -padding {2 2} -sticky news

  ########## green
    ttk::style layout Green.TButton {
      Button.background
      Button.button-green -children {
          Button.focus -children {
              Button.label
          }
      }
    }

    ttk::style element create button-green image [list \
        $I(button-green-n) \
        {pressed !disabled} $I(button-p) \
        {active !selected}  $I(button-h) \
        selected $I(button-s) \
        disabled $I(button-d)] \
      -border {4 4 4 10} -padding {2 2} -sticky news

   ########## grey
    ttk::style layout Grey.TButton {
      Button.background
      Button.button-grey -children {
          Button.focus -children {
              Button.label
          }
      }
    }

    ttk::style element create button-grey image [list \
        $I(button-grey-n) \
        {pressed !disabled} $I(button-p) \
        {active !selected}  $I(button-h) \
        selected $I(button-s) \
        disabled $I(button-d)] \
      -border {4 4 4 10} -padding {2 2} -sticky news

  ########## violet
    ttk::style layout Violet.TButton {
      Button.background
      Button.button-violet -children {
          Button.focus -children {
              Button.label
          }
      }
    }

    ttk::style element create button-violet image [list \
        $I(button-violet-n) \
        {pressed !disabled} $I(button-p) \
        {active !selected}  $I(button-h) \
        selected $I(button-s) \
        disabled $I(button-d)] \
      -border {4 4 4 10} -padding {2 2} -sticky news

  ########## orange
    ttk::style layout Orange.TButton {
      Button.background
      Button.button-orange -children {
          Button.focus -children {
              Button.label
          }
      }
    }

    ttk::style element create button-orange image [list \
        $I(button-orange-n) \
        {pressed !disabled} $I(button-p) \
        {active !selected}  $I(button-h) \
        selected $I(button-s) \
        disabled $I(button-d)] \
      -border {4 4 4 10} -padding {2 2} -sticky news

    ttk::style configure -padding {3 0}
    ttk::style configure Red.TButton -padding {3 0}
    ttk::style configure Green.TButton -padding {3 0}
    ttk::style configure Yellow.TButton -padding {3 0}
    ttk::style configure Orange.TButton -padding {3 0}
    ttk::style configure Violet.TButton -padding {3 0}
    ttk::style configure Grey.TButton -padding {3 0}
    # ttk::style configure TButton -padding {3 1}

    #style element create button image $I(button-n) \
    #    -map [list pressed $I(button-p)  active $I(button-h)] \
    #    -border 4 -sticky ew

    ttk::style element create Checkbutton.indicator image [list \
          $I(check-nu) \
          {!disabled pressed selected} $I(check-hc) \
          {!disabled active selected} $I(check-hc) \
          {!disabled active} $I(check-hu) \
          {!disabled selected} $I(check-nc) ] \
        -width 24 -sticky w

    ttk::style element create Radiobutton.indicator image [list \
          $I(radio-nu) \
          {!disabled pressed selected} $I(radio-hc) \
          {!disabled active selected} $I(radio-hc) \
          {!disabled active} $I(radio-hu) \
          selected $I(radio-nc) ] \
          -width 24 -sticky w

    # The layout for the menubutton is modified to have a button element
    # drawn on top of the background. This means we can have transparent
    # pixels in the button element. Also, the pixmap has a special
    # region on the right for the arrow. So we draw the indicator as a
    # sibling element to the button, and draw it after (ie on top of) the
    # button image.
    ttk::style layout TMenubutton {
      Menubutton.background
      Menubutton.button -children {
          Menubutton.focus -children {
              Menubutton.padding -children {
                  Menubutton.label -side left -expand true
              }
          }
      }
      Menubutton.indicator -side right
    }

    ttk::style element create Menubutton.button image [list \
        $I(mbut-n) \
        {active !disabled} $I(mbut-a) \
        {pressed !disabled} $I(mbut-a) \
        {disabled}          $I(mbut-d)] \
        -border {4 2 4 2} -padding {2 2 20 2} -sticky news

    ttk::style element create Menubutton.indicator image $I(mbut-arrow-n) \
      -width 8 -sticky w -padding {0 0 18 0}

    ## Toolbar buttons.
    #
    ttk::style element create Toolbutton.button image [list \
        $I(tbar-n) \
        {pressed !disabled} $I(tbar-p) \
        {active !selected}   $I(tbar-a) \
        selected             $I(tbar-p)] \
        -border {2 8 2 2} -padding {2 2} -sticky news

    ## Entry widgets.
    #
    ttk::style configure TEntry \
        -selectborderwidth 1 -padding 0 -insertwidth 2
    ttk::style configure TCombobox \
        -selectborderwidth 1 -padding 0 -insertwidth 2

    ## Notebooks.
    #
# 2019-12-3 these are not always working for some reason
#    ttk::style element create tab image [list \
#        $I(tab-n) \
#        selected $I(tab-p) \
#        active $I(tab-p) \
#        disabled $I(tab-hide-n) ] \
#        -border {4 4 4 2} -height 0

    ## Scrollbars.
    #
    ttk::style layout Vertical.TScrollbar {
      Scrollbar.trough -children {
        Scrollbar.uparrow -side top
        Scrollbar.downarrow -side bottom
        Vertical.Scrollbar.thumb -side top -expand true -sticky ns
      }
    }

    ttk::style layout Horizontal.TScrollbar {
      Scrollbar.trough -children {
        Scrollbar.leftarrow -side left
        Scrollbar.rightarrow -side right
        Horizontal.Scrollbar.thumb -side left -expand true -sticky we
      }
    }

    ttk::style element create Horizontal.Scrollbar.thumb image [list \
        $I(sb-thumb) {pressed !disabled} $I(sb-thumb-p)] \
         -border 4

    ttk::style element create Vertical.Scrollbar.thumb image [list \
        $I(sb-vthumb) {pressed !disabled} $I(sb-vthumb-p)] \
        -border 4

    foreach dir {up down left right} {
      ttk::style element create ${dir}arrow image [list \
          $I(arrow${dir}) \
          disabled $I(arrow${dir}) \
          pressed $I(arrow${dir}-p) \
          active $I(arrow${dir}-h)] \
          -border 1 -sticky {}
    }

    ## Scales.
    #
    ttk::style element create Scale.slider image [list \
        $I(slider) {pressed !disabled} $I(slider-p)]

    ttk::style element create Vertical.Scale.slider image [list \
        $I(vslider) {pressed !disabled} $I(vslider-p)]

    ttk::style element create Horizontal.Progress.bar \
        image $I(sb-thumb) -border 3
    ttk::style element create Vertical.Progress.bar \
        image $I(sb-vthumb) -border 3
  }
}

