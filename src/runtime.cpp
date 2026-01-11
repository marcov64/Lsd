/*************************************************************

	LSD 9.0 - January 2026
	written by Marco Valente, Universita' dell'Aquila
	and by Marcelo Pereira, University of Campinas

	Copyright Marco Valente and Marcelo Pereira
	LSD is distributed under the GNU General Public License

	See Readme.txt for copyright information of
	third parties' code used in LSD

 *************************************************************/

/*************************************************************
 RUNTIME.CPP
 Contains initialization and management of run-time plotting

 The main functions contained here are:

 - void prepare_plot( int id_sim, bool da_en )
 Checks is there are LSD variables to plot. If not, returns
 immediately. Otherwise initialize the run time global variables.
 Namely, the vector of the labels for the variables of plot.
 The plot window is initialized according to the id_sim name

 - void lsd::object::assign_plot_vars( int *i, const char *lab,
 bool da_en );
 Create a list of Variables to plot and create the list of labels
 (adding the indexes if necessary) to be used in the plot.

 - void init_plot( int i );
 create the canvas for the plot, the lines, button, labels, etc.

 - void plot_runtime( int *idx, int t, double cur_val,
 double last_val )
 the function used run time to plot the value of variable
 *************************************************************/

#include "LSD.h"

namespace gui
{
	b_vecT plot_prev_draw;
	double plot_y_max;						// runtime plot limits
	double plot_y_min;
	d_vecT plot_prev_val;
	int plot_last_t;
	int plot_num_var;
	i_vecT plot_prev_t;
}


/*************************************************************
 PREPARE_PLOT
 *************************************************************/
void gui::prepare_plot( int id_sim, bool da_en )
{
	char lab[ MAX_ELEM_LENGTH ];

	plot_y_max = plot_y_min = 0;
	plot_last_t = plot_num_var = 0;
	strcpy( lab, "" );
	cmd( "unset -nocomplain tp" );

	sim.root->assign_plot_vars( & plot_num_var, lab, da_en );

	if ( plot_num_var > 0 && add_rt_plot_tab( ".plt", id_sim ) )
	{
		plot_prev_t.assign( plot_num_var, -1 );
		plot_prev_val.assign( plot_num_var, NAN );
		plot_prev_draw.assign( plot_num_var, true );
		init_plot( );
	}
}


/*************************************************************
 ASSIGN_PLOT_VARS
 *************************************************************/
void lsd::object::assign_plot_vars( int *count, const char *lab, bool da_en )
{
	char cur_lab[ MAX_ELEM_LENGTH ];
	int j;

	if ( ! da_en )
	{
		object *cur;

		for ( auto cv = v; cv != NULL; cv = cv->next )
			if ( cv->plot )
			{
				cmd( "lappend tp \"%s%s\"", cv->attr->label, lab );
				++( *count );
			}

		for ( auto cb = b; cb != NULL; cb = cb->next )
		{
			if ( cb->head == NULL )
				continue;

			if ( cb->head->next != NULL )	// multiple instances
				for ( j = 1, cur = cb->head; cur != NULL; cur = BROTHER( cur ), ++j )
				{
					snprintf( cur_lab, MAX_ELEM_LENGTH, "%s#%d", lab, j );
					cur->assign_plot_vars( count, cur_lab, da_en );
				}
			else							// unique instance
				cb->head->assign_plot_vars( count, lab, da_en );
		}
	}
	else
		for ( auto & ca : gui::da.ass_elem )
			if ( ca.plot )
			{
				if ( ca.inst_ini == 1 )
				{
					cmd( "lappend tp \"%s@A%s\"", ca.label.c_str( ), lab );

					if ( gui::da.sav_fct )
						cmd( "lappend tp \"%s@T%s\"", ca.label.c_str( ), lab );

					if ( gui::da.sav_obs )
						cmd( "lappend tp \"%s@O%s\"", ca.label.c_str( ), lab );
				}
				else
					for ( j = 1; j <= ca.inst_ini; ++j )
					{
						snprintf( cur_lab, MAX_ELEM_LENGTH, "%s@A#%d", ca.label.c_str( ), j );
						cmd( "lappend tp \"%s%s\"", cur_lab, lab );

						if ( gui::da.sav_fct )
						{
							snprintf( cur_lab, MAX_ELEM_LENGTH, "%s@T#%d", ca.label.c_str( ), j );
							cmd( "lappend tp \"%s%s\"", cur_lab, lab );
						}

						if ( gui::da.sav_obs )
						{
							snprintf( cur_lab, MAX_ELEM_LENGTH, "%s@O#%d", ca.label.c_str( ), j );
							cmd( "lappend tp \"%s%s\"", cur_lab, lab );
						}
					}

				*count += ca.inst_ini * ( 1 + ( gui::da.sav_fct ? 1 : 0 ) + ( gui::da.sav_obs ? 1 : 0 ) );
			}
}


/*************************************************************
 ADD_RT_PLOT_TAB
 *************************************************************/
bool gui::add_rt_plot_tab( const char *w, int id_sim )
{
	int i, j, k, cols, rows, dbut, tabs;

	switch ( platform )
	{
		case _WIN_:
			tabs = 12;
			cols = 9;
			rows = 12;
			dbut = 2;
			break;
		case _LIN_:
			tabs = 8;
			cols = 8;
			rows = 11;
			dbut = 3;
			break;
		default:
		case _MAC_:
			tabs = 8;
			cols = 5;
			rows = 11;
			dbut = 2;
			break;
	}

	cmd( "set w %s", w );
	cmd( "set rtptab $w.pad" );

	// abort on weird Tk cases
	if ( id_sim > 1 && ( ! exists_window( "$rtptab" ) || eval_int( "[ $rtptab index end ]" ) == 0 ) )
		return false;

	cmd( "if { ! [ winfo exists $rtptab ] } { \
			newtop $w \"%s%s - LSD Run-time Plots\" \"destroytop $w\" \"\"; \
			wm transient $w .; \
			ttk::notebook $rtptab; \
			pack $rtptab; \
			ttk::notebook::enableTraversal $rtptab; \
			bind $w <F1> { LsdHelp runtime.html }; \
			set rtptab_show 0 \
		}", unsaved_change( ) ? "*" : " ", strlen( sim.conf_name ) > 0 ? sim.conf_name : NO_CONF_NAME );

	set_shortcuts_run( "$w" );

	cmd( "set activeplot $rtptab.tab%d", id_sim );
	cmd( "if [ winfo exists $activeplot ] { \
			if { $activeplot in [ $rtptab  tabs ] } { \
				$rtptab forget $activeplot \
			}; \
			destroy $activeplot \
		}" );
	cmd( "ttk::frame $activeplot" );
	cmd( "pack $activeplot" );

	// add More... tab if too many tabs
	if ( id_sim > tabs )
	{
		if ( ! exists_window( "$rtptab.more" ) )
		{
			cmd( "$rtptab forget 0" );
			cmd( "ttk::frame $rtptab.more" );
			cmd( "pack $rtptab.more" );
			cmd( "$rtptab insert 0 $rtptab.more -text \"More...\" -underline 0" );
		}
		else
			cmd( "$rtptab forget 1" );

		// update the More.. tab list of plots
		cmd( "destroy $rtptab.more.b" );
		cmd( "ttk::frame $rtptab.more.b" );

		if ( id_sim > cols * rows )
			k = id_sim - cols * rows + 1;
		else
			k = 1;

		for ( i = 1; k <= id_sim && i <= rows; ++i )
		{
			cmd( "ttk::frame $rtptab.more.b.l%d", i );

			for ( j = 1; k <= id_sim && j <= cols; ++j )
			{
				cmd( "set b [ expr { $butWid - ( %d ) } ]", k > 99 ? dbut : dbut - 1 );
				cmd( "ttk::button $rtptab.more.b.l%d.b%d -width $b -text \"Run %d\" -command { \
						if { \"$rtptab.tab%d\" ni [ $rtptab tabs ] } { \
							if { [ $rtptab index end ] >= %d } { \
								$rtptab forget 1 \
							}; \
							$rtptab add $rtptab.tab%d -text \"Run %d\" \
						}; \
						$rtptab select $rtptab.tab%d \
					}", i, k, k, k, tabs, k, k, k );
				cmd( "pack $rtptab.more.b.l%d.b%d -side left -padx $_2", i, k );

				++k;
			}

			cmd( "pack $rtptab.more.b.l%d -anchor w -pady $_2", i );
		}

		cmd( "pack $rtptab.more.b -padx $_20 -pady $_20" );
	}

	if ( id_sim < 10 )
		cmd( "$rtptab add $activeplot -text \"Run %d\" -underline 4", id_sim );
	else
		if ( id_sim == 10 )
			cmd( "$rtptab add $activeplot -text \"Run %d\" -underline 5", id_sim );
		else
			cmd( "$rtptab add $activeplot -text \"Run %d\"", id_sim );

	cmd( "$rtptab select $activeplot" );

	return true;
}


/*************************************************************
 INIT_PLOT
 *************************************************************/
void gui::init_plot( void )
{
	int i;

	if ( ! exists_var( "activeplot" ) || ! exists_window( "$activeplot" ) )
		return;

	cmd( "if { %d > $hsizeR } { set plot_step 1 } { set plot_step [ expr { $hsizeR / %d.0 } ] }", sim.last_t, sim.last_t );

	cmd( "ttk::frame $activeplot.c" );

	// vertical scale values
	cmd( "ttk::canvas $activeplot.c.yscale -width $sclhsizeR -height [ expr { $vsizeR + $sclvmarginR + $botvmarginR } ] -entry 0 -dark $darkTheme" );

	cmd( "$activeplot.c.yscale create text $sclhsizeR [ expr { max( $sclvmarginR, 10 ) } ] -anchor e -justify right -text \"\" -fill $colorsTheme(dfg) -tag ymax" );
	cmd( "$activeplot.c.yscale create text $sclhsizeR [ expr { $sclvmarginR + $vsizeR / 2 } ] -anchor e -justify right -text \"\" -fill $colorsTheme(dfg) -tag medy" );
	cmd( "$activeplot.c.yscale create text $sclhsizeR [ expr { $sclvmarginR + $vsizeR } ] -anchor e -justify right -text \"\" -fill $colorsTheme(dfg) -tag ymin" );

	cmd( "pack $activeplot.c.yscale -side left -anchor nw" );

	// main canvas
	cmd( "ttk::frame $activeplot.c.c  " );
	cmd( "set p $activeplot.c.c.cn" );
	cmd( "ttk::scrollbar $activeplot.c.c.hscroll -orient horiz -command \"$p xview\"" );
	cmd( "ttk::canvas $p -width [ expr { $hsizeR + 2 * $cvhmarginR } ] -height [ expr { $vsizeR + $sclvmarginR + $botvmarginR } ] -scrollregion \"0 0 %d [ expr { $vsizeR + $sclvmarginR + $botvmarginR } ]\" -xscrollcommand \"$activeplot.c.c.hscroll set\" -xscrollincrement 1 -yscrollincrement 1 -dark $darkTheme", sim.last_t );
	cmd( "pack $activeplot.c.c.hscroll -side bottom -expand yes -fill x" );
	cmd( "mouse_wheel $p" );

	// horizontal grid lines
	cmd( "for { set i 0 } { $i <= $vticksR } { incr i } { \
			if { $i > 0 && $i < $vticksR } { \
				set color $colorsTheme(bg) \
			} else { \
				set color $colorsTheme(dfg) \
			}; \
			$p create line [ expr { $cvhmarginR - $ticmarginR } ] [ expr { $sclvmarginR + $vsizeR * $i / $vticksR } ] [ expr { $cvhmarginR } ] [ expr { $sclvmarginR + $vsizeR * $i / $vticksR } ] -fill $colorsTheme(dfg); \
			$p create line [ expr { $cvhmarginR } ] [ expr { $sclvmarginR + $vsizeR * $i / $vticksR } ] [ expr { $cvhmarginR + %d * $plot_step } ] [ expr { $sclvmarginR + $vsizeR * $i / $vticksR } ] -fill $color \
		}", sim.last_t );

	// vertical grid lines
	cmd( "set k [ expr { $vsizeR + $sclvmarginR } ]" );
	cmd( "for { set i 0; set j $cvhmarginR; set u -1 } { $j <= [ expr { $cvhmarginR + %d * $plot_step } ] } { incr i; set j [ expr { $j + $hsizeR / $hticksR } ] } { \
			if { $plot_step > 1 } { \
				set l [ expr { %d * $i / $hticksR } ] \
			} else { \
				set l [ expr { $j - $cvhmarginR } ] \
			}; \
			if { $j > $cvhmarginR && $j < [ expr { $cvhmarginR + %d * $plot_step } ] } { \
				set color $colorsTheme(bg) \
			} else { \
				set color $colorsTheme(dfg) \
			}; \
			$p create line $j $sclvmarginR $j $k -fill $color; \
			$p create line $j $k $j [ expr { $k + $ticmarginR } ] -fill	 $colorsTheme(dfg); \
			if { $l > $u } { \
				$p create text $j [ expr { $k + $ticmarginR } ] -text $l -anchor n -fill $colorsTheme(dfg); \
				set u $l \
			} \
	}	", sim.last_t, sim.last_t, sim.last_t );

	cmd( "pack $p -anchor nw" );
	cmd( "pack $activeplot.c.c -anchor nw" );
	cmd( "pack $activeplot.c -anchor nw" );
	cmd( "$p xview moveto 0" );

	// bottom part
	cmd( "ttk::canvas $activeplot.fond -width [ expr { $sclhsizeR + $hsizeR + 2 * $cvhmarginR } ] -height $botvsizeR -entry 0 -dark $darkTheme" );

	// controls
	cmd( "set scrollB %d", scrollB );
	cmd( "ttk::checkbutton $activeplot.fond.shift -text Scroll -variable scrollB -state disabled -command { set_c_var done_in 8 }" );
	cmd( "if [ string equal $CurPlatform windows ] { \
			set centerB Center; \
			set goWid 7 \
		} elseif [ string equal $CurPlatform linux ] { \
			set centerB Center; \
			set goWid 6 \
		} { \
			set centerB Cen.; \
			set goWid 3 \
		}" );
	cmd( "ttk::button $activeplot.fond.go -width $goWid -text $centerB -state disabled -command { set_c_var done_in 7 }" );

	cmd( "$activeplot.fond create window [ expr { $sclhsizeR / 2 } ] [ expr { $botvsizeR / 4 - 5 } ] -window $activeplot.fond.shift" );
	cmd( "$activeplot.fond create window [ expr { $sclhsizeR / 2 } ] [ expr { 3 * $botvsizeR / 4 - 2 } ] -window $activeplot.fond.go" );

	// labels
	cmd( "set xlabel [ expr { $sclhsizeR + $sclvmarginR } ]" );
	cmd( "set ylabel 0" );
	cmd( "set a 0" );
	cmd( "set b 0" );

	for ( i = 0; i < plot_num_var; ++i )
	{
		cmd( "set lab [ regsub # [ lindex $tp %d ] _ ]", i );
		cmd( "set app [ font measure $fontP $lab ]" );
		cmd( "if { $xlabel + $app + $a > $sclhsizeR + $sclvmarginR + $hsizeR + 2 * $cvhmarginR } { \
				if { $ylabel + $lheightP + $labvpadR <= $linlabR * $lheightP } { \
					set xlabel [ expr { $sclhsizeR + $sclvmarginR } ]; \
					incr ylabel [ expr { $lheightP + $labvpadR } ]; \
					if { $ylabel + $lheightP + $labvpadR > $linlabR * $lheightP } { \
						set a [ expr [ font measure $fontP \"(000 more...)\" ] + $labhpadR ] \
					} \
				} else { \
					set b 1 \
				} \
			}" );

		if ( get_int( "b" ) )
			break;

		cmd( "set it [ $activeplot.fond create text $xlabel $ylabel -font $fontP -anchor nw -text $lab -fill $c%d ]", i < 1100 ? i : 0 );
		cmd( "set xlabel [ expr { $xlabel + $app + $labhpadR } ]" );
		cmd( "set lab [ regsub {#[0-9]+} [ lindex $tp %d ] \"\" ]", i );
		cmd( "set_ttip_descr $activeplot.fond $lab $it 0" );
	}

	if ( i < plot_num_var )
	{
		cmd( "set it [ $activeplot.fond create text $xlabel $ylabel -fill $colorsTheme(fg) -font $fontP -anchor nw -text \"(%d more...)\" ]", plot_num_var - i );
		cmd( "tooltip::tooltip $activeplot.fond -item  $it \"%d series labels not presented\"", plot_num_var - i );
	}

	if ( sim.last_t > get_int( "hsizeR" ) )
	{
		cmd( "$activeplot.fond.go conf -state normal" );
		cmd( "$activeplot.fond.shift conf -state normal" );
		cmd( "tooltip::tooltip $activeplot.fond.go \"Center plot in current time step\"" );
		cmd( "tooltip::tooltip $activeplot.fond.shift \"Automatic scrolling\"" );
	}

	cmd( "pack $activeplot.fond -expand yes -fill both -pady $_7" );
}


/*************************************************************
 PLOT_RUNTIME
 *************************************************************/
void gui::plot_runtime( int *idx, int t, double cur_val, double last_val )
{
	if ( idx == NULL )
		idx = & cur_plt_var;

	if ( ! exists_var( "activeplot" ) || ! exists_window( "$activeplot.c.c.cn" ) || plot_num_var == 0 || *idx >= plot_num_var || *idx > 1000 )
		return;

	if ( std::isfinite( last_val ) )
	{
		plot_prev_t[ *idx ] = t - 1;
		plot_prev_val[ *idx ] = last_val;
	}

	if ( plot_prev_t[ *idx ] == t - 1 )
		draw_plot( idx, t, cur_val );
	else
		plot_prev_draw[ *idx ] = false;

	plot_prev_t[ *idx ] = t;
	plot_prev_val[ *idx ] = cur_val;
	++( *idx );
}


/*************************************************************
 DRAW_PLOT
 *************************************************************/
void gui::draw_plot( int *idx, int t, double cur_val, bool point )
{
	bool relabel = false;
	double value, scale, zero_lim, yhi, ylo, ymed;
	int p_digits = get_int( "pdigitsR" );

	if ( ! std::isfinite( cur_val ) || ! std::isfinite( plot_prev_val[ *idx ] ) )
	{
		plot_prev_draw[ *idx ] = false;
		return;
	}
	else
		plot_prev_draw[ *idx ] = true;

	yhi = std::max( cur_val, plot_prev_val[ *idx ] );
	ylo = std::min( cur_val, plot_prev_val[ *idx ] );

	if ( plot_y_max <= plot_y_min || std::fabs( plot_y_max - plot_y_min ) < MARG )	// very initial setting
	{
		if ( yhi > 0 )
			plot_y_max = sim.round_digits( yhi * ( 1 + MARG ), p_digits );
		else
			plot_y_max = sim.round_digits( yhi * ( 1 - MARG ), p_digits );

		plot_y_min = sim.round_digits( ylo, p_digits );
		relabel = true;
	}

	if ( yhi >= plot_y_max )
	{
		value = yhi * ( yhi > 0 ? 1 + MARG_CONST : 1 - MARG_CONST );
		value = sim.round_digits( value, p_digits );

		if ( value == plot_y_min )
			plot_y_min -= std::fabs( plot_y_min ) * MARG;

		if ( value == plot_y_min )	// case all are zero?
			plot_y_min -= MARG;

		scale = ( plot_y_max - plot_y_min ) / ( value - plot_y_min );
		plot_y_max = value;
		relabel = true;
		cmd( "$activeplot.c.c.cn scale punto 0 $vsizeR 1 %lf", scale  < 0.01 ? 0.01 : scale );
	}

	if ( ylo <= plot_y_min )
	{
		value = ylo * ( ylo > 0 ? 1 - MARG_CONST : 1 + MARG_CONST );
		value = std::min( value, plot_y_min - ( plot_y_max - plot_y_min ) / get_int( "vsizeR" ) );
		value = sim.round_digits( value, p_digits );

		if ( value == plot_y_max )
			plot_y_max += std::fabs( plot_y_max ) * MARG;

		if ( value == plot_y_max )	// case all are zero?
			plot_y_max += MARG;

		scale = ( plot_y_max - plot_y_min ) / ( plot_y_max - value );
		plot_y_min = value;
		relabel = true;
		cmd( "$activeplot.c.c.cn scale punto 0 0 1 %lf", scale  < 0.01 ? 0.01 : scale );
	}

	if ( plot_y_max <= plot_y_min || std::fabs( plot_y_max - plot_y_min ) < MARG )
	{
		if ( plot_y_max != 0 )
			plot_y_max += std::fabs( plot_y_max ) * MARG;
		else
			plot_y_max += MARG;
	}

	if ( relabel )
	{
		ymed = sim.round_digits( ( plot_y_max - plot_y_min ) / 2 + plot_y_min, p_digits );
		zero_lim = ( plot_y_max - plot_y_min ) * MARG;

		cmd( "$activeplot.c.yscale itemconf plot_y_max -text %.*g", p_digits, fabs( plot_y_max ) < zero_lim ? 0 : plot_y_max );
		cmd( "$activeplot.c.yscale itemconf medy -text %.*g", p_digits, fabs( ymed ) < zero_lim ? 0 : ymed );
		cmd( "$activeplot.c.yscale itemconf plot_y_min -text %.*g", p_digits, fabs( plot_y_min ) < zero_lim ? 0 : plot_y_min );
	}

	cmd( "set x1 [ expr { floor( $cvhmarginR + %d * $plot_step ) } ]", t );
	cmd( "set y1 [ expr { floor( $sclvmarginR + ( $vsizeR - ( ( %lf - %lf ) / ( %lf - %lf ) ) * $vsizeR ) ) } ]", cur_val, plot_y_min, plot_y_max, plot_y_min );

	if ( point )
	{
		cmd( "$activeplot.c.c.cn create line [ expr { $x1 + 2 } ] [ expr { $y1 + 2 } ] [ expr { $x1 - 3 } ] [ expr { $y1 - 3 } ] -tag punto -fill $c%d", *idx );
		cmd( "$activeplot.c.c.cn create line [ expr { $x1 + 2 } ] [ expr { $y1 - 2 } ] [ expr { $x1 - 3 } ] [ expr { $y1 + 3 } ] -tag punto -fill $c%d", *idx );
	}
	else
	{
		cmd( "set x2 [ expr { floor( $cvhmarginR + %d * $plot_step ) } ]", t - 1 );
		cmd( "set y2 [ expr { floor( $sclvmarginR + ( $vsizeR - ( ( %lf - %lf ) / ( %lf - %lf ) ) * $vsizeR ) ) } ]", plot_prev_val[ *idx ], plot_y_min, plot_y_max, plot_y_min );
		cmd( "$activeplot.c.c.cn create line $x2 $y2 $x1 $y1 -tag punto -fill $c%d", *idx );
	}
}


/*************************************************************
 RESET_PLOT
 *************************************************************/
void gui::reset_plot( void )
{
	cmd( "if { [ info exists activeplot ] && [ winfo exists $activeplot ] } { \
			$activeplot.fond.go conf -state disabled; \
			$activeplot.fond.shift conf -state disabled; \
			tooltip::tooltip clear $activeplot.fond.go; \
			tooltip::tooltip clear $activeplot.fond.shift; \
			if { %d || ! [ winfo ismapped $activeplot ] || ! [ info exists rtptab_show ] || ! $rtptab_show } { \
				destroytop [ winfo toplevel $activeplot ] \
			} else { \
				$rtptab select $activeplot; \
				deiconifytop $activeplot \
			}; \
			update \
		}", sim.fast ? 1 : 0 );
}


/*************************************************************
 ENABLE_PLOT
 *************************************************************/
void gui::enable_plot( void )
{
	cmd( "if { [ info exists activeplot ] && [ winfo exists $activeplot ] } { \
			$rtptab select $activeplot; \
			if { [ info exists rtptab_show ] && ! $rtptab_show } { \
				set rtptab_show 1; \
				showtop [ winfo toplevel $activeplot ] \
			} else { \
				deiconifytop $activeplot \
			}; \
			focustop .log; \
			update \
		}" );
}


/*************************************************************
 DISABLE_PLOT
 *************************************************************/
void gui::disable_plot( void )
{
	cmd( "if { [ info exists activeplot ] && [ winfo exists $activeplot ] } { \
			wm withdraw [ winfo toplevel $activeplot ]; \
			update \
		}" );
}


/*************************************************************
 CENTER_PLOT
 *************************************************************/
void gui::center_plot( void )
{
	cmd( "if { [ info exists activeplot ] && [ winfo exists $activeplot ] && %d > [ expr { $hsizeR / 2 } ] } { \
			set newpos [ expr { %lf - $hsizeR / 2 / %lf } ]; \
			$activeplot.c.c.cn xview moveto $newpos; \
			update idletasks \
		}", sim.t, sim.t / ( double ) sim.last_t, ( double ) sim.last_t );
}


/*************************************************************
 SCROLL_PLOT
 *************************************************************/
void gui::scroll_plot( void )
{
	// plot a point for isolated values
	for ( auto i = 0; i < ( int ) plot_prev_t.size( ); ++i )
		if ( ! plot_prev_draw[ i ] && plot_prev_t[ i ] == sim.t )
			draw_plot( & i, sim.t, plot_prev_val[ i ], true );

	if ( scrollB )
		cmd( "if { [ info exists activeplot ] && [ winfo exists $activeplot ] && [ winfo ismapped $activeplot ] && %d > [ expr { $hsizeR * 0.8 } ] } { \
				$activeplot.c.c.cn xview scroll %d units \
			}", sim.t, sim.t - plot_last_t );

	plot_last_t = sim.t;
}
