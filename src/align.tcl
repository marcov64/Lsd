proc align {w1 w2} {


set a [winfo width $w1]
set b [winfo height $w1]
set c [winfo x $w2]
set d [winfo y $w2]

set e [expr $c - $a]
wm geometry $w1 +$e+$d
}
