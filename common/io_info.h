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
 
#ifndef __BALANCE_PARALLEL_IO_INFO_H__
#define __BALANCE_PARALLEL_IO_INFO_H__

#define DEF_IO_BUF_SIZE 4096
#define MAX_FREE_MODE_BUF_LEN 2048
#define io_accept_state 1
#define FREE_MODE 0x01
#define AYNC_OPER_INCOMPLETE -11

typedef int ( *server_proc_func )( struct __io_info *info );
typedef int ( *init_server_func )( void *context);
typedef int ( *release_server_func )( void *context );

typedef int ( *recv_new_line )( char* line, dword buf_len, void *param );

typedef enum __opera_type
{
	NONE_OPERA=0,
	RECV_OPERA,
	SEND_OPERA,
	ACCEPT_OPERA,
	CLOSE_OPERA,
	OPERA_TYPE_END
} opera_type;

typedef struct __server_work_info
{
	int inited;
	int listen_sock;
	int comp_port;

	dlist *thread_list;
	dlist *io_info_list;

	HANDLE thread_list_mutex;
	HANDLE io_info_list_mutex;

	server_proc_func cur_proc_func;
	init_server_func init_func;
	release_server_func release_func;
	void *server_ext;
} server_work_info;

typedef struct __io_info
{
	WSAOVERLAPPED overlapped;
	opera_type operation;

	SOCKET cli_sock;

	dword local_addr;
	dword remote_addr;

	uint16 remote_port;
	uint16 local_port;

	HANDLE io_info_mutex;
	WSABUF wsabuf;

	char *recv_buf;
	dword recv_len;
	dword recved;
	dword recv_buf_len;

	char *send_buf;
	dword send_len;
	dword sended;
	dword send_buf_len;

	dword cur_process;

	void *context;

	server_work_info *work_info;
} io_info;

int response_command_result( io_info *info, param_info *ret_info );
int extract_ln_from_buf( char* line, dword buf_len, void *param );
int extract_ln_from_net( char* line, dword buf_len, void *param );
int get_client_request_from_buf( io_info *info, paral_command **command );
int get_client_request_from_net( io_info *info, paral_command **command );
int get_client_request( io_info *info, paral_command **command, recv_new_line recv_func );
int init_io_info( void *info, int sock, server_work_info *work_info );
int create_new_io_info( io_info **info, int sock, server_work_info *work_info );
void prepare_async_opera( io_info *info );
void async_close( io_info *info );
void destroy_io_info_element( io_info *info );
int add_sock2iocp( io_info *info, server_work_info *work_info );
int new_free_async_recv( io_info *info );
int new_async_recv( io_info *info, int recv_len );
int new_async_send( io_info *info, char *send_data, int data_len );
int get_async_result( io_info *info, int wait );
int async_recv( io_info *info, int flags );
int async_send( io_info *info );
int async_accept( int listen_sock, server_work_info *work_info  );
int onrecved( io_info *cur_io_info );
int onsended( io_info *cur_io_info );
int onaccepted( io_info *cur_io_info );
void destroy_io_info( io_info *info );
void destroy_all_io_info( server_work_info *work_info );

#endif //__BALANCE_PARALLEL_IO_INFO_H__