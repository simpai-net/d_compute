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
#include "dlist.h"
#include "network.h"
#include "iocompport.h"
#include "controller.h"

#define COMP_THREAD_NUM( cpu_num ) ( cpu_num * 2 + 2 )

static dword WINAPI comp_port_work_thread( void* param )
{
	int32 ret;
	dword errcode;
	work_thread_context *context = ( work_thread_context* )param;
	HANDLE comp_port = context->comp_port;
	ulong transferred;
	io_info *opera_io_info;
	io_info *file_io_info;
	
	bp_trace( 8, "enter comp_port_work_thread func\n" );
	while( 1 )
	{
		transferred = -1;
		bp_trace( 8, "calling GetQueuedCompletionStatus\n" );
		ret = GetQueuedCompletionStatus( comp_port,
												&transferred,
												( dword* )&file_io_info,
												( LPOVERLAPPED* )&opera_io_info,
												INFINITE );


		//while( 1 )
		//{
		//	Sleep( 1000 );
		//}
		if( FALSE == ret )
		{
			errcode = WSAGetLastError();
			bp_trace( 5, "GetQueuedCompletionStatus failed,error code: %u\n", errcode );
			if( NULL == opera_io_info )
			{
#ifndef ERROR_ABANDONED_WAIT_0
#define ERROR_ABANDONED_WAIT_0 735L
#endif
				//get the completion information failed, and the bound key of the file handle, transferred byte is indetermine
				if( ERROR_ABANDONED_WAIT_0 == errcode )
				{
					bp_trace( 2, "io completion port is closed: 0x%08x\n", comp_port );
					break;
				}
			}
			else
			{
				if( NULL != file_io_info )
				{
					if( RECV_OPERA == file_io_info->operation ||
						SEND_OPERA == file_io_info->operation )
					{
						bp_assert( file_io_info->remote_addr != 0 );
						bp_trace( 5, "one peer socket operation failed, peer ip :%s port :%u\n", ip2str( file_io_info->remote_addr ), file_io_info->remote_port );
					}
				}
				else
				{
					bp_assert( file_io_info == opera_io_info );
					bp_trace( 5, "the listen socket operation failed\n" );
				}
				
				bp_trace( 5, "the bytes transferred is: %u\n", transferred );
			}
			continue;
		}

		bp_trace( 5, "call GetQueuedCompletionStatus successfully\n" );
		bp_assert( NULL != opera_io_info->work_info );

		if( ACCEPT_OPERA == opera_io_info->operation )
		{
			bp_assert( 0 == transferred );

			ret = onaccepted( opera_io_info );
			if( 0 > ret )
			{
				goto error;
			}

			ret = async_accept( ( int )file_io_info->cli_sock, file_io_info->work_info );
			if( 0 > ret )
			{
				goto error;
			}
			continue;
		}
		
		else if( SEND_OPERA == opera_io_info->operation )
		{
			bp_assert( 0 != transferred );

			ret = get_async_result( opera_io_info, FALSE );
			if( 0 > ret )
			{
				bp_assert( FALSE );
				goto error;
			}
			ret = onsended( opera_io_info );
			if( 0 > ret )
			{
				goto error;
			}
			continue;
		}

		else if( RECV_OPERA == opera_io_info->operation )
		{
			if( 0 == transferred )
			{
				//bp_assert( FALSE );
				goto error;
			}

			ret = get_async_result( opera_io_info, FALSE );
			if( 0 > ret )
			{
				if( ret == AYNC_OPER_INCOMPLETE )
				{
					continue;
				}
				bp_assert( FALSE );
				goto error;
			}
			ret = onrecved( opera_io_info );
			if( 0 > ret )
			{
				goto error;
			}

			continue;
		}

		else if( CLOSE_OPERA == opera_io_info->operation )
		{
			destroy_io_info( opera_io_info );
			continue;
		}
error:
		async_close( opera_io_info );
		opera_io_info = NULL;
		file_io_info = NULL;
		continue;
	}

	return 0;
}

void destroy_thread_info( work_thread_context *context )
{
	InterlockedExchange( &context->stop_thread, TRUE );
	WaitForSingleObject( context->thread, INFINITE );
	CloseHandle( context->thread );

	free( context );
}

void destroy_thread_pool( server_work_info *info)
{
	list_element thread_info;
	dlist *thread_item;
	dlist *next_thread_item;
	thread_item = info->thread_list;

	require_mutex( info->thread_list_mutex );

	for( ; ; )
	{
		if( NULL == thread_item )
			break;

		next_thread_item = thread_item->next;
		thread_info  = thread_item->info;

		del_list_item( thread_item, NULL );

		if( NULL != thread_info )
		{
			destroy_thread_info( thread_info );
		}

		thread_item = next_thread_item;
	}

	release_mutex( info->thread_list_mutex );
}

int create_thread_context( work_thread_context **context, int comp_port )
{
	work_thread_context *new_context;

	bp_assert( NULL != context );
	bp_assert( INVALID_HANDLE_VALUE != comp_port );

	new_context = ( work_thread_context* )malloc( sizeof( work_thread_context ) );
	
	if( NULL == new_context )
	{
		*context = NULL;
		return -E_OUTOFMEMORY;
	}

	memset( new_context, 0, sizeof( work_thread_context ) );
	new_context->comp_port = comp_port;
	*context = new_context;
	//context->work_mutex = CreateMutex( NULL, FALSE, NULL );

	return 0;
}

int init_server_work_info( server_work_info *info, 
						  server_proc_func proc_func,
						  init_server_func init_func,
						  release_server_func release_func, 
						  void *server_ext)
{
	int i;
	int ret;
	int work_thread_num;
	SYSTEM_INFO sys_Info;

	bp_assert( NULL != info );
	bp_assert( NULL != proc_func );

	if( FALSE != info->inited )
		return -1;

	GetSystemInfo( &sys_Info );
	work_thread_num = COMP_THREAD_NUM( sys_Info.dwNumberOfProcessors );

	info->cur_proc_func = proc_func;
	info->init_func = init_func;
	info->release_func = release_func;
	info->server_ext = server_ext;

	info->comp_port = ( int )CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, 0, work_thread_num );
	if( INVALID_HANDLE_VALUE == info->comp_port )
	{
		bp_trace( COMP_PORT_ERROR_LEVEL, "CreateIoCompletionPort faild with Error: %d", GetLastError() );
		return -1;
	}

	ret = init_list_element( &info->thread_list );
	if( 0 > ret )
	{
		return -E_OUTOFMEMORY;
	}

	ret = init_list_element( &info->io_info_list );
	if( 0 > ret )
	{
		return -E_OUTOFMEMORY;
	}

	info->thread_list_mutex = CreateMutex( NULL, FALSE, NULL );
	if( NULL == info->thread_list_mutex )
	{
		return -1;
	}

	info->io_info_list_mutex = CreateMutex( NULL, FALSE, NULL );
	if( NULL == info->io_info_list_mutex )
	{
		return -1;
	}
	work_thread_num = 1;

	for( i = 0; i < work_thread_num; i++ )
	{
		work_thread_context *thread_context;

		ret = create_thread_context( &thread_context, info->comp_port );
		if( 0 > ret )
		{
			goto error;
		}

		thread_context->thread = CreateThread( NULL, 0, comp_port_work_thread, ( void * ) thread_context, 0/*CREATE_SUSPENDED*/, &thread_context->thread_id );

		if( NULL == thread_context->thread )
		{
			bp_trace( 3, "Create Server Work Thread faild with Error: %d", WSAGetLastError() );
			goto error;
		}

		ret = add_list_element( info->thread_list, ( list_element )thread_context );
		if( 0 > ret )
		{
			return ret;
		}
	}

	info->inited = TRUE;
	return 0;

error:
	release_server_work_info( info );
	return -1;
}

void release_server_work_info( server_work_info *info )
{
	bp_assert( NULL != info );

	if( NULL != info->thread_list )
	{
		destroy_thread_pool( info );
		info->thread_list = NULL;
	}

	if( NULL != info->thread_list_mutex )
		CloseHandle( info->thread_list_mutex );

	if( NULL != info->io_info_list )
	{
		destroy_all_io_info( info );
		info->io_info_list = NULL;
	}

	if( NULL != info->io_info_list_mutex )
		CloseHandle( info->io_info_list_mutex );

	if( INVALID_HANDLE_VALUE != info->comp_port )
	{
		CloseHandle( ( HANDLE )info->comp_port );
	}

	if( NULL != info->release_func )
	{
		info->release_func( info->server_ext );
	}
	info->inited = FALSE;
}

void destroy_server_work_info( server_work_info *info )
{
	release_server_work_info( info );
	free( info );
}

int create_server_work_info( server_work_info **info, 
							server_proc_func proc_func,
							init_server_func init_func,
							release_server_func release_func,
							void *server_ext )
{
	int ret;
	server_work_info *new_info = NULL;

	if( NULL != init_func )
	{
		ret = init_func( server_ext );
		if( 0 > ret )
		{
			return ret;
		}
	}

	new_info = ( server_work_info* )malloc( sizeof( server_work_info ) );

	if( NULL == new_info )
	{
		return -E_OUTOFMEMORY;
	}

	new_info->comp_port = INVALID_HANDLE_VALUE;
	new_info->cur_proc_func = NULL;
	new_info->io_info_list = NULL;
	new_info->io_info_list_mutex = INVALID_HANDLE_VALUE;
	new_info->listen_sock = INVALID_SOCKET;
	new_info->thread_list = NULL;
	new_info->io_info_list_mutex = INVALID_HANDLE_VALUE;
	new_info->server_ext = NULL;
	new_info->init_func = NULL;
	new_info->release_func = NULL;
	new_info->inited = FALSE;

	ret = init_server_work_info( new_info, proc_func, init_func, release_func, server_ext );
	if( 0 > ret )
	{
		free( new_info );
		*info = NULL;
		return ret;
	}

	*info = new_info;
	return ret;
}

int init_add_sock2iocp( int sock, server_work_info *work_info )
{
	int ret;
	io_info *new_io_info = NULL;
	ret = create_new_io_info( &new_io_info, sock, work_info );
	if( 0 > ret )
	{
		goto error;
	}

	new_io_info->cli_sock = sock;
	return add_sock2iocp( new_io_info, work_info );

error:
	if( NULL != new_io_info )
	{
		destroy_io_info( new_io_info );
	}
	return ret;
}

