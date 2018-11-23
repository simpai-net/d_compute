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

#ifndef __BALANCE_PARALLEL_IOCOMPPORT_H__
#define __BALANCE_PARALLEL_IOCOMPPORT_H__

#include "io_info.h"

#define IPV4_ADDR_STR_LEN 16

typedef struct __work_thread_context
{
	dword thread_id;
	int stop_thread;
	HANDLE thread;
	HANDLE comp_port;
} work_thread_context;

void destroy_thread_info( work_thread_context *context );
void destroy_thread_pool( server_work_info *info);
int create_thread_context( work_thread_context **context, int comp_port );
int init_server_work_info( server_work_info *info, 
						  server_proc_func proc_func,
						  init_server_func init_func,
						  release_server_func release_func, 
						  void *server_ext);
void release_server_work_info( server_work_info *info );
void destroy_server_work_info( server_work_info *info );
int create_server_work_info( server_work_info **info, 
							server_proc_func proc_func,
							init_server_func init_func,
							release_server_func release_func,
							void *server_ext );
int init_add_sock2iocp( int sock, server_work_info *work_info );

#endif //__BALANCE_PARALLEL_IOCOMPPORT_H__