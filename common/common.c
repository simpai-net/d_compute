/*
 * @author: JiJie, simple_ai@outlook.com
 * This file is part of d_compute.
 * 
 * ----- The MIT License (MIT) ----- 
 * Copyright (c) 2009, JiJie
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "common.h"

dword g_trace_level = 1;
dword g_output_mode = OUTPUT_TO_DEBUGER | OUTPUT_TO_CONSOLE;
int g_log_file = -1;
#define LOCKER_NUM 3
#define DEBUGER_LOCK_INDEX 0
#define CONSOLE_LOCK_INDEX 1
#define LOG_FILE_LOCK_INDEX 2
CRITICAL_SECTION g_lockers[ LOCKER_NUM ];

int __bp_trace( unsigned int level, char *format, ... )
{
	va_list args;
	int trace_out_len;

	char trace_out[ MAX_ONCE_TRACE_OUT_LEN ];
	
	if( g_trace_level < level )
		return 0;

	va_start( args, format );

	trace_out_len = vsprintf( trace_out, format, args );
	va_end( args );

	bp_trace_out( trace_out );
	return trace_out_len;
}

int init_bp_work( dword trace_mode, 
				 dword trace_level, 
				 char *log_file )
{
	init_tracer( trace_mode, trace_level, log_file );
	return 0;
}

void init_bp_work_default()
{
	init_tracer( OUTPUT_TO_DEBUGER | OUTPUT_TO_CONSOLE, 10, "bp_trace.log" );
}

void init_locker( dword locker )
{
	bp_assert( LOCKER_NUM > locker );
	InitializeCriticalSection( &g_lockers[ locker ] );
}

void del_locker( dword locker )
{
	bp_assert( LOCKER_NUM > locker );
	DeleteCriticalSection( &g_lockers[ locker ] );
}

void lock( dword locker )
{
	bp_assert( LOCKER_NUM > locker );
	EnterCriticalSection( &g_lockers[ locker ] );
}

void unlock( dword locker )
{
	bp_assert( LOCKER_NUM > locker );
	LeaveCriticalSection( &g_lockers[ locker ] );
}

int wait_disp_obj_time( HANDLE obj, dword timeout )
{
	dword ret;
	bp_assert( NULL != obj );
	ret = WaitForSingleObject( obj, timeout );
	if( ret == WAIT_OBJECT_0 )
		return 0;
	else
		return -1;
}

void release_mutex( HANDLE mutex )
{
	ReleaseMutex( mutex );
}

void signal_event( HANDLE event )
{
	SetEvent( event );
}

void bp_set_trace_mode( unsigned int mode )
{
#ifdef _WIN32
	InterlockedExchange( &g_output_mode, mode );
#endif
}

int memsubstr( char* finded, int finded_len, char* buf, int buflen)
{
	int i;
	int j;
	char* __buf = ( char* )buf;
	int end_index = buflen - finded_len + 1;
	char isfinded;

	if( buflen < finded_len )
		return -1;

	for( i = 0; i < end_index; i ++)
	{
		isfinded = TRUE;
		for( j = 0; j < finded_len; j++ )
		{
			if( finded[j] != __buf[i + j] )
			{
				isfinded = FALSE;
			}
		}

		if(isfinded)
			return i;
	}
	return -1;
}

int readln_from_buf( char* line, dword max_ln_len, char *buf, dword *remain_buf_len, int is_remove )
{
	int term_pos;
	char *ln_term;
	dword remain;

	bp_assert( NULL != remain_buf_len );
	ln_term = LN_TERM_SIGN;
	remain = *remain_buf_len;

	if( 0 == remain )
	{
		return 0;
	}
			
	term_pos = memsubstr((char*)ln_term, LN_TERM_SIGN_LEN, (BYTE*)buf, remain );

	if( 0 > term_pos )
	{
		return -1;
	}

	if( ( uint32 )( term_pos + 1 ) > max_ln_len )
	{
		return -1;
	}

	memcpy( line, buf, term_pos );
	line[ term_pos ] = '\0';

	if( TRUE == is_remove )
	{
		int ln_end_pos;
		ln_end_pos = term_pos + LN_TERM_SIGN_LEN;
		remain -= ln_end_pos;
		memmove( buf, buf + ln_end_pos, remain );
		*remain_buf_len = remain;
	}

	return term_pos;
}

int bp_open_log_file( char *log_file_name )
{
	int g_log_file;
	char *dir_delim_pos;
	char log_file_path[ MAX_PATH ];
	
	bp_assert( NULL != log_file_name );

	dir_delim_pos = strchr( log_file_name, '\\' );
	if( NULL == dir_delim_pos )
	{
		get_file_path_in_app_path( log_file_name, log_file_path, MAX_PATH );
	}
	else
	{
		strcpy_s( log_file_path, MAX_PATH, log_file_name );
	}

	g_log_file = open( log_file_path, O_RDWR | O_CREAT | O_TRUNC );
	
	return g_log_file;
}

void init_tracer( dword trace_mode, dword trace_level, char *log_file_name )
{
	int ret;

	g_output_mode = 0;
	if( trace_mode & OUTPUT_TO_CONSOLE )
	{
		HANDLE __stdout;
		__stdout = GetStdHandle( STD_OUTPUT_HANDLE );
		if( NULL == __stdout )
		{
			ret = AllocConsole();
		}

		if( NULL != stdout || TRUE == ret )
		{
			g_output_mode |= OUTPUT_TO_CONSOLE;
			init_locker( CONSOLE_LOCK_INDEX );
		}
	}

	if( trace_mode & OUTPUT_TO_LOG )
	{
		bp_assert( NULL != log_file_name );
		ret = bp_open_log_file( log_file_name );
		
		if( 0 > ret )
		{
			OutputDebugStringA( "open log file failed\n" );
		}
		else
		{
			g_output_mode |= OUTPUT_TO_LOG;
			init_locker( LOG_FILE_LOCK_INDEX );
		}
	}

	if( trace_mode & OUTPUT_TO_DEBUGER )
	{
		g_output_mode |= OUTPUT_TO_DEBUGER;
		init_locker( DEBUGER_LOCK_INDEX );
	}

	if( 0 == g_output_mode )
	{
		g_output_mode |= OUTPUT_TO_DEBUGER;
		init_locker( DEBUGER_LOCK_INDEX );
	}

	g_trace_level = trace_level;
}

int get_file_path_in_app_path( const char *file_name, char *file_path, unsigned int maxlen )
{
	int i;
	int module_name_len;
	char module_path[ MAX_PATH ];

	GetModuleFileNameA( NULL, module_path, MAX_PATH );
	module_name_len = ( int )strlen( module_path );
	
	for( i = module_name_len - 1; i >= 0; i-- )
	{
		if( module_path[ i ] == '\\' )
		{
			break;
		}
	}

	strcpy_s( module_path + i + 1, MAX_PATH - i - 1, file_name );

	if( maxlen < strlen( module_path ) + 1 )
	{
		return -1;
	}

	strcpy_s( file_path, maxlen, module_path );

	return 0;
}

#define MAX_ONCE_TRACE_OUT_LEN ( 1024 )
void bp_trace_out( char *output )
{
	if( g_output_mode & OUTPUT_TO_DEBUGER )
	{
		lock( DEBUGER_LOCK_INDEX );
		OutputDebugStringA( output );
		unlock( DEBUGER_LOCK_INDEX );
	}
	
	if( g_output_mode & OUTPUT_TO_LOG )
	{
		lock( LOG_FILE_LOCK_INDEX );
		write( g_log_file, output, ( uint32 )strlen( output ) );
		unlock( LOG_FILE_LOCK_INDEX );
	}

	if( g_output_mode & OUTPUT_TO_CONSOLE )
	{
		dword bytes_written;
		lock( CONSOLE_LOCK_INDEX );
		WriteConsoleA( GetStdHandle( STD_OUTPUT_HANDLE ),
			output, ( DWORD )strlen( output ), 
			&bytes_written, NULL); 
		unlock( CONSOLE_LOCK_INDEX );
	}
}

int realloc_buf_needed( char **buf, int *buf_len, int dest_len )
{
	dword new_buf_len;
	char *new_buf;

	bp_assert( NULL != buf && NULL != buf_len );

	if( dest_len > *buf_len )
	{
		new_buf_len = dest_len + IO_BUF_INC_LEN;
		if( NULL == *buf )
		{
			new_buf = malloc( new_buf_len );
		}
		else
		{
			new_buf = realloc( *buf, new_buf_len );
		}

		if( NULL == new_buf )
		{
			return -E_OUTOFMEMORY;
		}

		*buf = new_buf;
		*buf_len = new_buf_len;
	}
	return 0;
}

int add_line_to_buf( char *command, dword cmd_len, char **buf, dword *buf_cur_len, dword *buf_max_len )
{
	int ret;
	dword cur_len;
	char *__buf;

	bp_assert( NULL != command &&
		NULL != buf &&
		NULL != buf_cur_len &&
		NULL != buf_max_len );

	cur_len = *buf_cur_len;

	if( cur_len + cmd_len > *buf_max_len );
	{
		ret = realloc_buf_needed( buf, buf_max_len, cur_len + cmd_len + CONST_STR_LEN( COMMAND_ELEMENT_DELIM ) );
		if( 0 > ret )
		{
			return ret;
		}
	}

	__buf = *buf;
	memcpy( __buf + cur_len, command, cmd_len );
	cur_len += cmd_len;
	memcpy( __buf + cur_len, COMMAND_ELEMENT_DELIM, CONST_STR_LEN( COMMAND_ELEMENT_DELIM ) );
	cur_len += CONST_STR_LEN( COMMAND_ELEMENT_DELIM );

	*buf_cur_len = cur_len;
	return 0;
}

int peek_null_line( char *buf, dword buf_len )
{
	int ret;
	ret = memsubstr( NULL_LINE_TERM_SIGN, CONST_STR_LEN( NULL_LINE_TERM_SIGN ), 
		buf, buf_len );
	return ret;
}
