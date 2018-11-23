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

#ifndef __BALANCE_PARALLEL_COMMON_H__
#define __BALANCE_PARALLEL_COMMON_H__

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <winerror.h>
#include <stdio.h>
#include <stdlib.h>

#include "bp_config.h"

#ifdef _DEBUG
#include <assert.h>
#define bp_assert( x ) assert( x )
#define bp_trace __bp_trace
#else
#define bp_assert( x )
#define bp_trace 
#endif

typedef unsigned long dword;
typedef unsigned long ulong;
typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;
typedef int int32;
typedef short int16;
typedef char int8, byte;

#include "core.h"
#include "network.h"
#include "dlist.h"

#define ACCEPT_SOCK_INFO_LEVEL 7
#define COMP_PORT_ERROR_LEVEL 2

#define OUTPUT_TO_DEBUGER 0x01
#define OUTPUT_TO_LOG 0x04
#define OUTPUT_TO_CONSOLE 0x02

#define LN_TERM_SIGN COMMAND_ELEMENT_DELIM
#define LN_TERM_SIGN_LEN CONST_STR_LEN( LN_TERM_SIGN )

#define IO_BUF_INC_LEN 128

#define CONST_STR_LEN( s ) ( sizeof( s ) - 1 )
#define MAX_ONCE_TRACE_OUT_LEN ( 1024 )

#define require_mutex( mutex ) wait_disp_obj_time( mutex, INFINITE )
#define require_mutex_time( mutex, timeout ) wait_disp_obj_time( mutex, timeout )
#define wait_event_time( event, timeout ) wait_disp_obj_time( event, timeout )
#define wait_event( event ) wait_disp_obj_time( event, INFINITE )
#define peek_cmd_end( buf, buf_len ) peek_null_line( buf, buf_len )

int init_bp_work( dword trace_mode, 
				 dword trace_level, 
				 char *log_file );
void init_bp_work_default();
void init_locker( dword locker );
void del_locker( dword locker );
void lock( dword locker );
void unlock( dword locker );
int wait_disp_obj_time( HANDLE obj, dword timeout );
void release_mutex( HANDLE mutex );
void signal_event( HANDLE event );
void bp_set_trace_mode( unsigned int mode );
int memsubstr( char* finded, int finded_len, char* buf, int buflen);
int readln_from_buf( char* line, dword max_ln_len, char *buf, dword *remain_buf_len, int is_remove );
int bp_open_log_file( char *log_file_name );
void init_tracer( dword trace_mode, dword trace_level, char *log_file_name );
int get_file_path_in_app_path( const char *file_name, char *file_path, unsigned int maxlen );
void bp_trace_out( char *output );
int realloc_buf_needed( char **buf, int *buf_len, int dest_len );
int add_line_to_buf( char *command, dword cmd_len, char **buf, dword *buf_cur_len, dword *buf_max_len );
int peek_null_line( char *buf, dword buf_len );
int __bp_trace( unsigned int level, char *format, ... );

#endif