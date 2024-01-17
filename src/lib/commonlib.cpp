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

#include "libLSD.h"


#ifdef _WIN32

/****************************************************
 RUN_SYSTEM (Windows)
 executes run command in system without opening
 command-prompt window or activating STL mutexes
 spaces in path/file names are not supported
 ****************************************************/
int run_system( const char *cmd, int id )
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

#ifndef _NP_

	if ( id >= 0 && id < ( int ) run_pids.size( ) )
	{

		lock_guard < mutex > lock( lock_run_pids );
		run_pids[ id ] = p_info.hProcess;
	}

#endif

	WaitForSingleObject( p_info.hProcess, INFINITE );
	GetExitCodeProcess( p_info.hProcess, & res );
	CloseHandle( p_info.hProcess );
	CloseHandle( p_info.hThread );

	free( c_line );
	return res;
}


/****************************************************
 KILL_SYSTEM (Windows)
 stops a running command in system
 ****************************************************/
int kill_system( int id )
{

#ifndef _NP_

	DWORD res;

	if ( id >= 0 && id < ( int ) run_pids.size( ) &&
		 GetExitCodeProcess( run_pids[ id ], & res ) &&
		 res == STILL_ACTIVE &&
		 ! TerminateProcess( run_pids[ id ], 15 ) )
			return 0;

#endif

	return 1;
}

#else

extern char ** environ;

/****************************************************
 RUN_SYSTEM (Unix)
 executes run command in system without opening
 command-prompt window or activating STL mutexes
 spaces in path/file names are not supported
 ****************************************************/
int run_system( const char *cmd, int id )
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

#ifndef _NP_

		if ( id >= 0 && id < ( int ) run_pids.size( ) )
		{
			lock_guard < mutex > lock( lock_run_pids );
			run_pids[ id ] = pid;
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


/****************************************************
 KILL_SYSTEM (Unix)
 stops a running command in system
 ****************************************************/
#define WAIT_TSECS 10
int kill_system( int id )
{

#ifndef _NP_

	int res, tsecs = 0;

	if ( id >= 0 && id < ( int ) run_pids.size( ) )
	{
		if ( kill( run_pids[ id ], SIGKILL ) == 0 )
		{
			while ( ( res = kill( run_pids[ id ], 0 ) ) == 0 &&
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

/****************************************************
 LSD_EXIT
 exit LSD
 ****************************************************/
void lsd_exit( int v )
{
	fflush( stderr );

#ifndef _NP_

	// stop multi-thread workers, if needed/safe
	if ( worker_errors( ) == 0 )
		delete [ ] workers;

#endif

	exit( v );
}


/****************************************************
 EXCEPTION_HANDLER
 handle exceptions and system signals
 ****************************************************/
void exception_handler( int signum, const char *what )
{
	static char msg1[ MAX_LINE_SIZE ], msg2[ MAX_LINE_SIZE ], msg3[ MAX_LINE_SIZE ];

	switch ( signum )
	{
		case SIGINT:
		case SIGTERM:
			if ( liblnk.cmd_backend == NULL )
			{
				snprintf( msg1, MAX_LINE_SIZE, "SIGINT/SIGTERM (%s)", signal_name( signum ) );
				break;
			}
			else
				cmd_gui( "set choice 1" );		// regular quit (checking for save)

			return;
#ifdef SIGWINCH
		case SIGWINCH:
			cmd_gui( "sizetop all" );			// readjust windows size/positions
			cmd_gui( "update" );

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

	if ( liblnk.cmd_backend != NULL )			// Tcl GUI available?
	{
#ifndef _LMM_
		if ( ! user_exception )
#endif
		{
			strcpyn( msg2, "There is an internal LSD error\n  If error persists, please contact developers", MAX_LINE_SIZE );
			strcpyn( msg3, "LSD will close now...", MAX_LINE_SIZE );
		}
#ifndef _LMM_
		else
		{
			strcpyn( msg3, "Additional information may be obtained running the simulation using the 'Model'/'GDB Debugger' menu option", MAX_LINE_SIZE );
			if ( quit != 2 )
			{
				if ( ! parallel_mode && fast_mode == 0 && stack_log != NULL &&
					 stack_log->vs != NULL && stack_log->vs->label != NULL )
				{
					strcatn( msg3, "\n\nAttempting to open the LSD Debugger.\n\nLSD will close immediately after exiting the Debugger.", MAX_LINE_SIZE );
					plog( "\n\nAn unknown problem was detected while computing the equation \nfor '%s'", stack_log->vs->label );
					if ( liblnk.print_stack != NULL )
						liblnk.print_stack( );
				}
				else
				{
					strcatn( msg3, "\n\nPlease disable fast mode and parallel processing to get more information about the error.\n\nLSD will close now.", MAX_LINE_SIZE );
					plog( "\n\nAn unknown problem was detected while executing user's equations code" );
					plog( "\n\nWarning: %s active, cannot open LSD Debugger", parallel_mode ? "parallel preocessing" : "fast mode" );
				}

				quit = 2;
			}
		}
#endif

		cmd_gui( "if { ! [ catch { package present Tk 8.6 } ] && ! [ catch { set tk_ok [ winfo exists . ] } ] && $tk_ok } { \
				catch { ttk::messageBox -parent . -title Error -icon error -type ok -message \"FATAL ERROR\" -detail \"System Signal received:\n\n %s:\n  %s\n\n%s\" } \
				}", msg1, msg2, msg3 );

#ifndef _LMM_
		if ( user_exception )
		{
			if ( ! parallel_mode && fast_mode == 0 && stack_log != NULL &&
				 stack_log->vs != NULL && stack_log->vs->label != NULL )
			{
				double useless = -1;
				snprintf( msg3, MAX_LINE_SIZE, "%s (ERROR)", stack_log->vs->label );
				if ( liblnk.deb != NULL )
					liblnk.deb( stack_log->vs->up, NULL, msg3, & useless, false, "" );
			}
		}
		else
#endif
			if ( liblnk.log_tcl_error != NULL )
				liblnk.log_tcl_error( true, "FATAL ERROR", "System Signal received: %s", msg1 );
	}
	else
		fprintf( stderr, "\nFATAL ERROR: System Signal received: %s\n", msg1 );

	lsd_exit( -signum );				// abort program
}


/****************************************************
 HANDLE_SIGNALS
 ****************************************************/
void handle_signals( void ( * handler )( int signum ) )
{
	for ( int i = 0; i < REG_SIG_NUM; ++i )
		signal( signals[ i ], handler );
}


/****************************************************
 SIGNAL_HANDLER
 handle critical system signals
 ****************************************************/
void signal_handler( int signum )
{
	exception_handler( signum, NULL );
}


/****************************************************
 SIGNAL_NAME
 ****************************************************/
const char *signal_name( int signum )
{
	int i;
	for ( i = 0; i < REG_SIG_NUM && signals[ i ] != signum; ++i );
	if ( i == REG_SIG_NUM )
		return "Unknow exception";
	return signal_names[ i ];
}


/****************************************************
 MSLEEP
 stop execution for a given period
 ****************************************************/
void msleep( unsigned msec )
{
#ifdef _WIN32
	Sleep( msec );
#else
	usleep( msec * 1000 );
#endif
	return;
}


/****************************************************
 CLEAN_FILE
 remove any path prefixes to filename, if present
 ****************************************************/
char *clean_file( const char *filename )
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


/****************************************************
 CLEAN_PATH
 remove cygwin/MSYS path prefixes, if present, and replace \ with /
 ****************************************************/
char *clean_path( const char *filepath )
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
			drvpos = strlen( pref[ i ] );				// drive letter position
			pathpos = drvpos + 1;						// path start
		}
		else			// MSYS
		{
			drvpos = 1;									// drive letter position
			pathpos = 2;								// path start
		}

		temp[ 0 ] = toupper( oldpath[ drvpos ] );		// copy drive letter
		temp[ 1 ] = ':';								// insert ':' drive separator
		strcpyn( temp + 2, oldpath + pathpos, strlen( oldpath ) - 1 );
		strcpyn( oldpath, temp, strlen( oldpath ) + 1 );
	}

	for ( i = 0; i < ( int ) strlen( oldpath ); ++i )
		if ( oldpath[ i ] == '\\' )						// replace \ with /
			oldpath[ i ] = '/';

	newpath = new char [ strlen( oldpath ) + 1 ];
	strcpyn( newpath, oldpath, strlen( oldpath ) + 1 );

	return newpath;
}


/****************************************************
 CMD_GUI
 ****************************************************/
void cmd_gui( const char *cm, ... )
{
	static va_list argptr;

	va_start( argptr, cm );

	if ( liblnk.cmd_backend != NULL )
		liblnk.cmd_backend( cm, argptr );

	va_end( argptr );
}


/****************************************************
 VALID_LABEL
 ****************************************************/
bool valid_label( const char *lab )
{
	return regex_match( lab, regex( "^[a-zA-Z_][a-zA-Z0-9_]*$" ) );
}


/****************************************************
 VALID_XML_STRING
 ****************************************************/
bool valid_xml_string( const char *lab )
{
	return regex_match( lab, regex( "[^&<>\"']*" ) );
}


/****************************************************
 STRCATN
 Concatenate strings respecting total size of first one
 ****************************************************/
char *strcatn( char *d, const char *s, size_t dSz )
{
	if ( dSz <= 0 || d == NULL || strlen( d ) >= dSz - 1 || s == NULL || strlen( s ) == 0 )
		return d;

	return strncat( d, s, dSz - strlen( d ) - 1 );
}


/***************************************************
 STRCLN
 trim whitespace from the beginning/end of string
 and convert line ends to LF only (unix-like)
 ***************************************************/
int strcln( char *out, const char *str, int outSz )
{
	char buf[ strlen( str ) + 1 ];
	strlf( buf, str, strlen( str ) + 1 );
	return strtrim( out, buf, outSz );
}


/****************************************************
 STRCPYN
 Copy string respecting total size of destination one
 ****************************************************/
char *strcpyn( char *d, const char *s, size_t dSz )
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


/***************************************************
 STRDECDATA
 decode string from a XML CDATA value
 out and in strings can be the same
 if out is NULL, space is allocated to the result,
 which MUST be deallocated by the caller
 ***************************************************/
char *strdecdata( char *out, const char *in, int outSz )
{
	string buf = in;
	int pos = -3;

	if ( out != NULL && outSz <= 0 )
		return NULL;

	while ( ( pos = buf.find( "]]\x7f>", pos + 3 ) ) != ( int ) string::npos )
		buf.erase( pos + 2, 1 );		// remove DEL (0x7f) character

	if ( out == NULL )
	{
		outSz = outSz > 0 ? outSz : buf.length( ) + 1;
		out = new char [ outSz ];
	}

	return strcpyn( out, buf.c_str( ), outSz );
}


/***************************************************
 STRLF
 replace CR-LF pairs with LF only on C string
 ***************************************************/
int strlf( char *out, const char *str, int outSz )
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


/***************************************************
 STRTOD
 split a C string into a double float,
 controlling for conversion errors, producing
 inv as result in this case
***************************************************/
double strtod( const char *in, char** endptr, double inv )
{
	double d;;

	errno = 0;				// detect invalid values
	if ( strlen( in ) == 0 )
		d = inv;
	else
		d = strtod( in, endptr );

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
		plog( "\nWarning: invalid double float (%s), adjusted to %g", in, d );
#endif
	}

	return d;
}


/***************************************************
 STRTODSPLIT
 split a C string into a vector of double floats
 using sep as the separator character, controlling
 for conversion errors, producing inv as result
 in this case
***************************************************/
vector < double > strtodsplit( const char *in, char sep, double inv )
{
	string buf;
	stringstream ss( in );
	vector < double > out;

	while ( getline( ss, buf, sep ) )
		out.push_back( strtod( buf.c_str( ), NULL, inv ) );

	return out;
}


/***************************************************
 STRTOL
 split a C string into a long integer,
 controlling for conversion errors, producing
 inv as result in this case
***************************************************/
long strtol( const char *in, char** endptr, int base, long inv )
{
	long l;

	errno = 0;				// detect invalid values
	if ( strlen( in ) == 0 )
		l = inv;
	else
		l = strtol( in, endptr, base );

	if ( errno != 0 )
	{
		if ( l == 0 )
			l = inv;
#ifndef _LMM_
		plog( "\nWarning: invalid long integer (%s), adjusted to %d", in, l );
#endif
	}

	return l;
}


/***************************************************
 STRTOLSPLIT
 split a C string into a vector of long integers
 using sep as the separator character, controlling
 for conversion errors, producing inv as result
 in this case
***************************************************/
vector < long > strtolsplit( const char *in, char sep, long inv )
{
	string buf;
	stringstream ss( in );
	vector < long > out;

	while ( getline( ss, buf, sep ) )
		out.push_back( strtol( buf.c_str( ), NULL, 10, inv ) );

	return out;
}


/***************************************************
 STRTOSTRSPLIT
 split a C string into a vector of strings using
 sep as the separator character
***************************************************/
vector < string > strtostrsplit( const char *in, char sep, bool remQuotes )
{
	string buf;
	stringstream ss( in );
	vector < string > out;

	while ( getline( ss, buf, sep ) )
	{
		if ( remQuotes )
			buf.erase( remove( buf.begin( ), buf.end( ), '\"' ), buf.end( ) );

		out.push_back( buf );
	}

	return out;
}


/***************************************************
 STRTRIM
 trim whitespace from the beginning/end of string
 ***************************************************/
int strtrim( char *out, const char *str, int outSz )
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


/***************************************************
 STRTRIMIN
 trim whitespace from the beginning/end of string,
 and also remove duplicated whitespace inside
 ***************************************************/
int strtrimin( char *out, const char *str, int outSz )
{
	string buf, in = str;

	unique_copy( in.begin( ), in.end( ), back_insert_iterator < string > ( buf ),
				 [ ] ( char a, char b ) { return isspace( a ) && isspace( b ); } );

	return strtrim( out, buf.c_str( ), outSz );
}


/***************************************************
 STRWSP
 check for a string of just whitespace
 ***************************************************/
bool strwsp( const char *str )
{
	if ( str == NULL )
		return true;

	while ( isspace( ( unsigned char ) *str ) )
		++str;

	if ( *str == '\0' )			// all spaces?
		return true;

	return false;
}
