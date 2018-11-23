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

#ifndef __BALANCE_PARALLEL_BROAD_SEARCH_H__
#define __BALANCE_PARALLEL_BROAD_SEARCH_H__

#include "broadcast.h"

#define BP_SEARCH_COMP_SERVER_CMD "IS_COMP_SERVER"
#define CONST_STR_LRLN_POSFIX( c_str ) ( BP_SEARCH_COMP_SERVER_CMD COMMAND_ELEMENT_DELIM )
#define BP_SEARCH_COMP_SERVER_TIME 3

#define RESP_SEND_PROC_TIME 1000
#define RESP_SEND_DELAY_TIME 200
#define RESP_SEND_TIME ( RESP_SEND_PROC_TIME / RESP_SEND_DELAY_TIME )
#define SEND_PROCEDURE_TIME 5000
#define SEND_DELAY_TIME 50
#define SEND_TIME ( SEND_PROCEDURE_TIME / SEND_DELAY_TIME )
#define DELAY_TIME 2500
#define DECV_TIME_ADD_TIME 1000
#define RECV_TIME ( SEND_PROCEDURE_TIME + DELAY_TIME + DECV_TIME_ADD_TIME )

#define MAX_BROADCAST_RESPONSE_CONT_LEN 256

typedef struct __responsor_info
{
	char recv_buf[ MAX_BROADCAST_RESPONSE_CONT_LEN ];
	dword ipaddr;
	uint16 port;
} responsor_info;

typedef int ( *on_responsor_searched )( responsor_info *info, void *context );

typedef struct __brd_search_context
{
	HANDLE search_thread;
	dword search_thread_id;
	on_responsor_searched onsearched;
	HANDLE stop_notifier;
	char *local_ipaddr;
	uint16 local_port;
	char *remote_ipaddr;
	uint16 remote_port;
	int brd_sock;
	void *context;
} brd_search_context;

typedef struct __brd_response_context
{
	int brd_sock;
	char *local_ipaddr;
	uint16 local_port;
	uint16 work_port;
	HANDLE stop_notifier;
	HANDLE resp_thread;
	dword resp_thread_id;
} brd_response_context;

int response_brd_search( brd_response_context *context );
dword WINAPI reponse_brd_search_thread( void* param );
int init_broad_response_thread( void *param );
int recv_responsors_info( int sock, on_responsor_searched onsearched, void *context );
int search_cluster_info( brd_search_context *context );
dword WINAPI search_responsor( void* param );
int init_broad_search_thread( void *context );
void stop_broad_search( void *context );
void stop_response_search( void *context );

#endif //__BALANCE_PARALLEL_BROAD_SEARCH_H__