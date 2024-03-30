/*************************************************************

	LSD 9.0 - January 2024
	written by Marco Valente, Universita' dell'Aquila
	and by Marcelo Pereira, University of Campinas

	Copyright Marco Valente and Marcelo Pereira
	LSD is distributed under the GNU General Public License

	See Readme.txt for copyright information of
	third parties' code used in LSD

 *************************************************************/

/*************************************************************
 COMMONLIB.CPP
 Basic code common between LMM and LSD Browser used in DLL
 or no-window executables. The remaining common code is
 stored in COMMON.CPP.
 *************************************************************/

#include "lib/libLSD.h"				// LSD library classes

#ifdef _LMM_
extern lsd::dlliblinkage lmm_liblnk;
#endif


#ifdef _WIN32
/*************************************************************
 RUN_SYSTEM (Windows)
 executes run command in system without opening
 command-prompt window or activating STL mutexes
 spaces in path/file names are not supported
 *************************************************************/
int lsd::run_system( const char *cmd, simulation *sim, int id )
{
	PROCESS_INFORMATION p_info;
	STARTUPINFO s_info;
	DWORD res;
	LPSTR c_line;

	memset( &s_info, 0, sizeof s_info );
	memset( &p_info, 0, sizeof p_info );
	s_info.cb = sizeof s_info;

	c_line = ( LPSTR ) malloc( strlen( cmd ) + 1 );
	strcpy( c_line, cmd );

	if ( ! CreateProcess( NULL, c_line, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, & s_info, & p_info ) )
	{
		free( c_line );
		return -1;
	}

#ifndef _LMM_
	if ( id >= 0 && sim != NULL && id < ( int ) sim->run_pids.size( ) )
	{

		l_guardT lock( sim->run_pids_lck );
		sim->run_pids[ id ] = p_info.hProcess;
	}
#endif

	WaitForSingleObject( p_info.hProcess, INFINITE );
	GetExitCodeProcess( p_info.hProcess, & res );
	CloseHandle( p_info.hProcess );
	CloseHandle( p_info.hThread );

	free( c_line );
	return res;
}


/*************************************************************
 KILL_SYSTEM (Windows)
 stops a running command in system
 *************************************************************/
int lsd::kill_system( simulation *sim, int id )
{

#ifndef _LMM_
	DWORD res;

	if ( id >= 0 && id < ( int ) sim->run_pids.size( ) &&
		 GetExitCodeProcess( sim->run_pids[ id ], & res ) &&
		 res == STILL_ACTIVE &&
		 ! TerminateProcess( sim->run_pids[ id ], 15 ) )
			return 0;
#endif
	return 1;
}
#else

extern char ** environ;

/*************************************************************
 RUN_SYSTEM (Unix)
 executes run command in system without opening
 command-prompt window or activating STL mutexes
 spaces in path/file names are not supported
 *************************************************************/
int lsd::run_system( const char *cmd, simulation *sim, int id )
{
	char **argv, **envp;
	int res;
	pid_t pid;
	wordexp_t p;

	if ( wordexp( cmd, & p, 0 ) != 0 )
		return -1;

	argv = p.we_wordv;
	envp = environ;

	pid = fork( );
	if ( pid == -1 )
	{
		wordfree( & p );
		return -1;
	}

	if ( pid == 0 )
	{
		execve( argv[ 0 ], argv, envp );
		exit ( errno );
	}
	else
	{
#ifndef _LMM_
		if ( id >= 0 && sim != NULL && id < ( int ) sim->run_pids.size( ) )
		{
			l_guardT lock( sim->run_pids_lck );
			sim->run_pids[ id ] = pid;
		}
#endif
		waitpid( pid, & res, 0 );
		wordfree( & p );

		if ( res == 0 )
			return 0;
		else
			return WEXITSTATUS( res );
	}
}


/*************************************************************
 KILL_SYSTEM (Unix)
 stops a running command in system
 *************************************************************/
#define WAIT_TSECS 10
int lsd::kill_system( simulation *sim, int id )
{
#ifndef _LMM_
	int res, tsecs = 0;

	if ( id >= 0 && id < ( int ) sim->run_pids.size( ) )
	{
		if ( kill( sim->run_pids[ id ], SIGKILL ) == 0 )
		{
			while ( ( res = kill( sim->run_pids[ id ], 0 ) ) == 0 &&
					tsecs++ < WAIT_TSECS )
				msleep( 100 );

			if ( res == 0 )
				return 0;
		}
		else
			if ( errno != ESRCH )
				return 0;
	}
#endif

	return 1;
}

#endif

/*************************************************************
 SET_EXEC
 *************************************************************/
void lsd::set_exec( const char *path, const char *file )
{
	strT exefile, exepath, libfile, libpath, fname;

	exepath = path;
	exefile = file;

	delete [ ] exec_path;
	delete [ ] exec_file;

	exec_path = clean_path( exepath.c_str( ) );	// path of executable file
	exec_file = clean_file( exefile.c_str( ) );	// name of executable file

#ifndef _LMM_
	// try to set dynamic library information
	libpath = exec_path;
	libfile = "lib";
	libfile += exec_file;						// base library name
	if ( libfile.find( '.' ) != strT::npos )// remove Windows extension
		libfile = libfile.substr( 0, libfile.rfind( "." ) );

#ifdef __linux__
	libfile += ".so";
#else
#ifdef __APPLE__
	libfile += ".dylib";
#else
	libfile += ".dll";
#endif
#endif

	// check if lib file is in path
	fname = exec_path;
	fname += "/" + libfile;
	FILE *f = fopen( fname.c_str( ), "r" );
	if ( f != NULL )
		fclose( f );

	if ( f == NULL )							// lib not find
	{
		if ( lib_path != NULL && lib_file != NULL )
			return;								// keep previous lib

		libfile = libpath = "";
	}

	delete [ ] lib_path;
	delete [ ] lib_file;

	lib_path = clean_path( libpath.c_str( ) );
	lib_file = clean_file( libfile.c_str( ) );
#endif
}


/*************************************************************
 LSD_EXIT
 exit LSD
 *************************************************************/
void lsd::lsd_exit( int v )
{
	fflush( stderr );

#ifndef _LMM_
	// stop multi-thread workers, if needed/safe
	for ( auto sim : sims )
		if ( sim->worker_errors( ) == 0 )
			delete [ ] sim->workers;
#endif

	exit( v );
}


/*************************************************************
 EXCEPTION_HANDLER
 handle exceptions and system signals
 *************************************************************/
void lsd::exception_handler( int signum, const char *what )
{
	dlliblinkage *liblnk = NULL;
	static char msg1[ MAX_LINE_SIZE ], msg2[ MAX_LINE_SIZE ], msg3[ MAX_LINE_SIZE ];

#ifndef _LMM_
	simulation *sim = NULL;

	if ( sims.size( ) > 0 && sims[ 0 ] != NULL )
	{
		sim = sims[ 0 ];						// handle GUI sim only

		if ( sim->liblnk != NULL )
			liblnk = sim->liblnk;
	}
#else
	liblnk = & ::lmm_liblnk;
#endif

	switch ( signum )
	{
		case SIGINT:
		case SIGTERM:
			if ( liblnk->cmd_backend == NULL )
			{
				snprintf( msg1, MAX_LINE_SIZE, "SIGINT/SIGTERM (%s)", signal_name( signum ) );
				break;
			}
			else
				cmd( "set choice 1" );		// regular quit (checking for save)

			return;
#ifdef SIGWINCH
		case SIGWINCH:
			cmd( "sizetop all" );			// readjust windows size/positions
			cmd( "update" );

			return;
#endif
		case SIGSTL:
			snprintf( msg1, MAX_LINE_SIZE, "SIGSTL (%s)", what != NULL && strlen( what ) > 0 ? what : "STL exception" );
			strcpyn( msg2, "Maybe an invalid math or data operation?\n	Check your standard C++ library calls' arguments", MAX_LINE_SIZE );
			break;

		case SIGMEM:
			snprintf( msg1, MAX_LINE_SIZE, "SIGMEM (%s)", what != NULL && strlen( what ) > 0 ? what : "Out of memory" );
			strcpyn( msg2, "Maybe too many series saved?\n	Try to reduce the number of series saved or the number of cases (time steps)", MAX_LINE_SIZE );
			break;

		case SIGABRT:
			snprintf( msg1, MAX_LINE_SIZE, "SIGABRT (%s)", signal_name( signum ) );
			strcpyn( msg2, "Maybe an invalid call to library or Tcl/Tk?", MAX_LINE_SIZE );
			break;

		case SIGFPE:
			snprintf( msg1, MAX_LINE_SIZE, "SIGFPE (%s)", signal_name( signum ) );
			strcpyn( msg2, "Maybe a division by 0 or similar?", MAX_LINE_SIZE );
		break;

		case SIGILL:
			snprintf( msg1, MAX_LINE_SIZE, "SIGILL (%s)", signal_name( signum ) );
			strcpyn( msg2, "Maybe executing data?", MAX_LINE_SIZE );
		break;

		case SIGSEGV:
			snprintf( msg1, MAX_LINE_SIZE, "SIGSEGV (%s)", signal_name( signum ) );
			strcpyn( msg2, "Maybe an invalid pointer?\n	 Also ensure no group of objects has zero elements.", MAX_LINE_SIZE );
		break;
		default:
			snprintf( msg1, MAX_LINE_SIZE, "Unknown signal (%s)", signal_name( signum ) );
			strcpy( msg2, "" );
	}

	if ( liblnk->cmd_backend != NULL )			// Tcl GUI available?
	{
#ifndef _LMM_
		bool usrExcpt = false;
		for ( auto s : sims )					// search for user exception
			if ( s->user_exception )
				usrExcpt = true;

		if ( ! usrExcpt )
#endif
		{
			strcpyn( msg2, "There is an internal LSD error\n  If error persists, please contact developers", MAX_LINE_SIZE );
			strcpyn( msg3, "LSD will close now...", MAX_LINE_SIZE );
		}
#ifndef _LMM_
		else
		{
			strcpyn( msg3, "Additional information may be obtained running the simulation using the 'Model'/'GDB Debugger' menu option", MAX_LINE_SIZE );

			if ( sim != NULL && sim->quit != 2 )
			{
				if ( ! sim->parallel_mode && sim->fast_mode == 0 &&
					 sim->stack_log != NULL && sim->stack_log->v != NULL &&
					 sim->stack_log->v->label != NULL )
				{
					strcatn( msg3, "\n\nAttempting to open the LSD Debugger.\n\nLSD will close immediately after exiting the Debugger.", MAX_LINE_SIZE );
					sim->plog( "\n\nAn unknown problem was detected while computing the equation \nfor '%s'", sim->stack_log->v->label );
					if ( liblnk->print_stack != NULL )
						liblnk->print_stack( );
				}
				else
				{
					strcatn( msg3, "\n\nPlease disable fast mode and parallel processing to get more information about the error.\n\nLSD will close now.", MAX_LINE_SIZE );
					sim->plog( "\n\nAn unknown problem was detected while executing user's equations code" );
					sim->plog( "\n\nWarning: %s active, cannot open LSD Debugger", sim->parallel_mode ? "parallel preocessing" : "fast mode" );
				}

				sim->quit = 2;
			}
		}
#endif
		cmd( "if { ! [ catch { package present Tk 8.6 } ] && ! [ catch { set tk_ok [ winfo exists . ] } ] && $tk_ok } { \
				catch { ttk::messageBox -parent . -title Error -icon error -type ok -message \"FATAL ERROR\" -detail \"System Signal received:\n\n %s:\n  %s\n\n%s\" } \
				}", msg1, msg2, msg3 );

#ifndef _LMM_
		if ( usrExcpt && sim != NULL )
		{
			if ( ! sim->parallel_mode && sim->fast_mode == 0 &&
				 sim->stack_log != NULL && sim->stack_log->v != NULL &&
				 sim->stack_log->v->label != NULL )
			{
				double useless = -1;
				snprintf( msg3, MAX_LINE_SIZE, "%s (ERROR)", sim->stack_log->v->label );
				if ( liblnk->debugger != NULL )
					( sim->stack_log->v->up->*liblnk->debugger )( NULL, msg3, & useless, false, "" );
			}
		}
		else
#endif
			if ( liblnk->log_tcl_error != NULL )
				liblnk->log_tcl_error( true, "FATAL ERROR", "System Signal received: %s", msg1 );
	}
	else
		fprintf( stderr, "\nFATAL ERROR: System Signal received: %s\n", msg1 );

	lsd_exit( -signum );				// abort program
}


/*************************************************************
 HANDLE_SIGNALS
 *************************************************************/
void lsd::handle_signals( void ( * handler )( int signum ) )
{
	for ( int i = 0; i < REG_SIG_NUM; ++i )
		signal( signals[ i ], handler );
}


/*************************************************************
 SIGNAL_HANDLER
 handle critical system signals
 *************************************************************/
void lsd::signal_handler( int signum )
{
	exception_handler( signum, NULL );
}


/*************************************************************
 SIGNAL_NAME
 *************************************************************/
const char *lsd::signal_name( int signum )
{
	int i;
	for ( i = 0; i < REG_SIG_NUM && signals[ i ] != signum; ++i );
	if ( i == REG_SIG_NUM )
		return "Unknow exception";
	return signal_names[ i ];
}


/*************************************************************
 MSLEEP
 stop execution for a given period
 *************************************************************/
void lsd::msleep( unsigned msec )
{
	if ( msec <= 0 )
		return;

#ifdef _WIN32
	Sleep( msec );
#else
	usleep( msec * 1000 );
#endif
	return;
}


/*************************************************************
 _MSLEEP_
 stop execution for a given period in equations
 *************************************************************/
void lsd::equation::_msleep_( unsigned msec )
{
	msleep( msec );
}


/*************************************************************
 CLEAN_FILE
 remove any path prefixes to filename, if present
 *************************************************************/
char *lsd::clean_file( const char *filename )
{
	char *name, *newname;

	if ( filename == NULL )
		return NULL;

	if ( strchr( filename, '/' ) != NULL )
		name = strrchr( filename, '/' ) + 1;
	else
		if ( strchr( filename, '\\' ) != NULL )
			name = strrchr( filename, '\\' ) + 1;
		else
			name = ( char * ) filename;

	newname = new char [ strlen( name ) + 1 ];
	strcpyn( newname, name, strlen( name ) + 1 );

	return newname;
}


/*************************************************************
 CLEAN_PATH
 remove cygwin/MSYS path prefixes, if present,
 and replace \ with /
 *************************************************************/
char *lsd::clean_path( const char *filepath )
{
	int i, drvpos, pathpos;
	char *newpath, oldpath[ strlen( filepath ) + 1 ];
	const int npref = 5;
	const char *pref[ npref ] = { "/cygdrive/", "/c/", "/d/", "/e/", "/f/" };

	if ( filepath == NULL )
		return NULL;

	strcpy( oldpath, filepath );

	for ( i = 0; i < npref && strncmp( oldpath, pref[ i ], strlen( pref[ i ] ) ); ++i );

	if ( i < npref )
	{
		char temp[ strlen( oldpath ) + 1 ];
		strcpy( temp, "" );

		if ( i == 0 )	// Cygwin
		{
			drvpos = strlen( pref[ i ] );		// drive letter position
			pathpos = drvpos + 1;				// path start
		}
		else			// MSYS
		{
			drvpos = 1;							// drive letter position
			pathpos = 2;						// path start
		}

		temp[ 0 ] = toupper( oldpath[ drvpos ] );// copy drive letter
		temp[ 1 ] = ':';						// insert ':' drive separator
		strcpyn( temp + 2, oldpath + pathpos, strlen( oldpath ) - 1 );
		strcpyn( oldpath, temp, strlen( oldpath ) + 1 );
	}

	for ( i = 0; i < ( int ) strlen( oldpath ); ++i )
		if ( oldpath[ i ] == '\\' )				// replace \ with /
			oldpath[ i ] = '/';

	newpath = new char [ strlen( oldpath ) + 1 ];
	strcpyn( newpath, oldpath, strlen( oldpath ) + 1 );

	return newpath;
}


/*************************************************************
 CMD_GUI
 *************************************************************/
void lsd::cmd( const char *cm, ... )
{
	static va_list argptr;

	va_start( argptr, cm );

#ifndef _LMM_
	if ( sims.size( ) > 0 && sims[ 0 ] != NULL && sims[ 0 ]->liblnk != NULL )
		sims[ 0 ]->liblnk->cmd_backend( cm, argptr );
#else
	if ( lmm_liblnk.cmd_backend != NULL )
		::lmm_liblnk.cmd_backend( cm, argptr );
#endif

	va_end( argptr );
}


/*************************************************************
 VALID_LABEL
 *************************************************************/
bool lsd::valid_label( const char *lab )
{
	return std::regex_match( lab, std::regex( "^[a-zA-Z_][a-zA-Z0-9_]*$" ) );
}


/*************************************************************
 VALID_XML_STRING
 *************************************************************/
bool lsd::valid_xml_string( const char *lab )
{
	return std::regex_match( lab, std::regex( "[^&<>\"']*" ) );
}


/*************************************************************
 STRCATN
 Concatenate strings respecting total size of first one
 *************************************************************/
char *lsd::strcatn( char *d, const char *s, size_t dSz )
{
	if ( dSz <= 0 || d == NULL || strlen( d ) >= dSz - 1 || s == NULL || strlen( s ) == 0 )
		return d;

	return strncat( d, s, dSz - strlen( d ) - 1 );
}


/*************************************************************
 STRCLN
 trim whitespace from the beginning/end of string
 and convert line ends to LF only (unix-like)
 *************************************************************/
int lsd::strcln( char *out, const char *str, int outSz )
{
	char buf[ strlen( str ) + 1 ];
	strlf( buf, str, strlen( str ) + 1 );
	return strtrim( out, buf, outSz );
}


/*************************************************************
 STRCPYN
 Copy string respecting total size of destination
 one
 *************************************************************/
char *lsd::strcpyn( char *d, const char *s, size_t dSz )
{
	if ( dSz <= 0 || d == NULL || s == NULL )
		return d;

	if ( strlen( s ) > dSz - 1 )
	{
		strncpy( d, s, dSz - 1 );
		d[ dSz - 1 ] = '\0';
	}
	else
		strcpy( d, s );

	return d;
}


/*************************************************************
 STRDECDATA
 decode string from a XML CDATA value
 out and in strings can be the same
 if out is NULL, space is allocated to the result,
 which MUST be deallocated by the caller
 *************************************************************/
char *lsd::strdecdata( char *out, const char *in, int outSz )
{
	strT buf = in;
	int pos = -3;

	if ( out != NULL && outSz <= 0 )
		return NULL;

	while ( ( pos = buf.find( "]]\x7f>", pos + 3 ) ) != ( int ) strT::npos )
		buf.erase( pos + 2, 1 );		// remove DEL (0x7f) character

	if ( out == NULL )
	{
		outSz = outSz > 0 ? outSz : buf.length( ) + 1;
		out = new char [ outSz ];
	}

	return strcpyn( out, buf.c_str( ), outSz );
}


/*************************************************************
 STRENCDATA
 encode string for a XML CDATA value
 out and in strings can be the same
 if out is NULL, space is allocated to the result,
 which MUST be deallocated by the caller
 *************************************************************/
char *lsd::strencdata( char *out, const char *in, int outSz )
{
	strT buf = in;
	int pos = -4;

	if ( out != NULL && outSz <= 0 )
		return NULL;

	while ( ( pos = buf.find( "]]>", pos + 4 ) ) != ( int ) strT::npos )
		buf.insert( pos + 2, "\x7f" );		// insert DEL (0x7f) character

	if ( out == NULL )
	{
		outSz = outSz > 0 ? outSz : buf.length( ) + 1;
		out = new char [ outSz ];
	}

	return strcpyn( out, buf.c_str( ), outSz );
}


/*************************************************************
 STRLF
 replace CR-LF pairs with LF only on C string
 *************************************************************/
int lsd::strlf( char *out, const char *str, int outSz )
{
	int i, j;

	for ( i = j = 0; str[ j ] != '\0' && i < outSz - 1; ++j )
		if ( str[ j ] == '\r' )
		{
			if ( str[ j + 1 ] != '\n' )
				out[ i++ ] = '\n';
		}
		else
			out[ i++ ] = str[ j ];

	out[ i ] = '\0';

	return i;
}


/*************************************************************
 STRWRDS
 count words in a string
 *************************************************************/
int lsd::strwrds( const char *s )
{
	char lst = '\0';
	int i = 0, wrd = 0;

	if ( s == NULL )
		return 0;

	while ( isspace( s[ i ] ) )
		++i;

	if ( s[ i ] == '\0' )
		return 0;

	for ( ; s[ i ] != '\0'; lst = s[ i++ ] )
		if ( isspace( s[ i ] ) && ! isspace( lst ) )
			wrd++;

	if ( isspace( lst ) )
		return wrd;

	return wrd + 1;
}


/*************************************************************
 STRWRAP
 insert line breaks in string to wrap text at given width
 based on code from ulf.astrom@gmail.com
 *************************************************************/
int lsd::strwrap( char *out, const char *str, int outSz, int wid )
{
	int i, lines, tlen, len, pos, close_word, open_word;

	if ( str == NULL )
		return 0;

	tlen = strlen( str );

	if ( tlen == 0 || wid <= 0 || outSz <= 0 )
		return 0;

	lines = pos = 0;
	while ( pos < tlen && outSz > 0 )
	{
		if ( str[ pos ] == '\n' )
		{
			len = 0;
			goto end_line;
		}

		if ( pos + wid > tlen )
			wid = tlen - pos;

		len = wid;
		close_word = 0;
		while ( str[ pos + len + close_word ] != '\0' && ! isspace( str[ pos + len + close_word ] ) )
			++close_word;

		open_word = 0;
		while ( str[ pos + len ] != '\0' && ! isspace( str[ pos + len ] ) )
		{
			--len;
			++open_word;

			if ( open_word + close_word > wid * 0.8 )
			{
				len = wid;
				break;
			}
		}

		for ( i = 0; i < len; ++i )
		{
			if ( str[ pos + i ] == '\n' )
			{
				len = i;
				break;
			}
		}

		end_line:

		++lines;

		if ( len > outSz - 1 )
			len = outSz - 1;

		if ( len > 0 )
		{
			strncpy( out, str + pos, len );
			out += len;
			outSz -= len;
		}

		out[ 0 ] = '\n';
		++out;
		--outSz;

		if ( len == wid )
			--len;

		pos += len + 1;
	}

	*( out - 1 ) = '\0';

	return lines;
}


/*************************************************************
 STRUPR
 convert string to upper case
 *************************************************************/
char *lsd::strupr( char *s )
{
	char *p;

	if ( s == NULL )
		return NULL;

	for ( p = s ; strlen( p ) != 0; ++p )
		*p = ( char ) toupper( ( int ) *p );

	return s;
}


/*************************************************************
 STRTOD
 split a C string into a double float,
 controlling for conversion errors, producing
 inv as result in this case
 *************************************************************/
double lsd::strtod( const char *in, char** endptr, double inv )
{
	double d;;

	errno = 0;				// detect invalid values
	if ( strlen( in ) == 0 )
		d = inv;
	else
		d = ::strtod( in, endptr );

	if ( errno != 0 )
	{
		if ( d == 0. )
			d = inv;
		else
			if ( d == HUGE_VAL )
				d = DBL_MAX;
			else
				if ( d == - HUGE_VAL )
					d = - DBL_MAX;
#ifndef _LMM_
	if ( sims.size( ) == 1 && sims[ 0 ] != NULL )
		sims[ 0 ]->plog( "\nWarning: invalid double float (%s), adjusted to %g", in, d );
#endif
	}

	return d;
}


/*************************************************************
 STRTODSPLIT
 split a C string into a vector of double floats
 using sep as the separator character, controlling
 for conversion errors, producing inv as result
 in this case
 *************************************************************/
d_vecT lsd::strtodsplit( const char *in, char sep, double inv )
{
	d_vecT out;
	strT buf;
	std::stringstream ss( in );

	while ( getline( ss, buf, sep ) )
		out.push_back( strtod( buf.c_str( ), NULL, inv ) );

	return out;
}


/*************************************************************
 STRTOL
 split a C string into a long integer,
 controlling for conversion errors, producing
 inv as result in this case
 *************************************************************/
long lsd::strtol( const char *in, char** endptr, int base, long inv )
{
	long l;

	errno = 0;				// detect invalid values
	if ( strlen( in ) == 0 )
		l = inv;
	else
		l = ::strtol( in, endptr, base );

	if ( errno != 0 )
	{
		if ( l == 0 )
			l = inv;
#ifndef _LMM_
		if ( sims.size( ) == 1 && sims[ 0 ] != NULL )
			sims[ 0 ]->plog( "\nWarning: invalid long integer (%s), adjusted to %d", in, l );
#endif
	}

	return l;
}


/*************************************************************
 STRTOLSPLIT
 split a C string into a vector of long integers
 using sep as the separator character, controlling
 for conversion errors, producing inv as result
 in this case
 *************************************************************/
l_vecT lsd::strtolsplit( const char *in, char sep, long inv )
{
	strT buf;
	std::stringstream ss( in );
	l_vecT out;

	while ( getline( ss, buf, sep ) )
		out.push_back( strtol( buf.c_str( ), NULL, 10, inv ) );

	return out;
}


/*************************************************************
 STRTOSTRSPLIT
 split a C string into a vector of strings using
 sep as the separator character
 *************************************************************/
str_vecT lsd::strtostrsplit( const char *in, char sep, bool remQuotes )
{
	strT buf;
	std::stringstream ss( in );
	str_vecT out;

	while ( getline( ss, buf, sep ) )
	{
		if ( remQuotes )
			buf.erase( remove( buf.begin( ), buf.end( ), '\"' ), buf.end( ) );

		out.push_back( buf );
	}

	return out;
}


/*************************************************************
 TO_STRING
 convert double to string, allowing for sprintf
 pattern format
 *************************************************************/
strT lsd::to_string( const char *fmt, double val )
{
	char buf[ 100 + 1 ];
	strT res;

	if ( snprintf( buf, 100, fmt, val ) < 0 )
		strcpy( buf, "" );

	return res = buf;
}


/*************************************************************
 STRTRIM
 trim whitespace from the beginning/end of string
 *************************************************************/
int lsd::strtrim( char *out, const char *str, int outSz )
{
	char *end;
	int size;

	if ( str == NULL || outSz <= 0 )
		return 0;

	while ( isspace( ( unsigned char ) *str ) )
		++str;

	if ( *str == '\0' )			// all spaces?
	{
		out[ 0 ] = '\0';
		return 1;
	}

	end = ( char * ) str + strlen( str ) - 1;
	while ( end > str && isspace( ( unsigned char ) *end ) )
		--end;
	++end;

	size = ( end - str ) < outSz ? ( end - str ) : outSz - 1;

	memcpy( out, str, size );
	out[ size ] = '\0';

	return size;
}


/*************************************************************
 STRTRIMIN
 trim whitespace from the beginning/end of string,
 and also remove duplicated whitespace inside
 *************************************************************/
int lsd::strtrimin( char *out, const char *str, int outSz )
{
	strT buf, in = str;

	unique_copy( in.begin( ), in.end( ), std::back_insert_iterator < strT > ( buf ),
				 [ ] ( char a, char b ) { return isspace( a ) && isspace( b ); } );

	return strtrim( out, buf.c_str( ), outSz );
}


/*************************************************************
 STRWSP
 check for a string of just whitespace
 *************************************************************/
bool lsd::strwsp( const char *str )
{
	if ( str == NULL )
		return true;

	while ( isspace( ( unsigned char ) *str ) )
		++str;

	if ( *str == '\0' )			// all spaces?
		return true;

	return false;
}
