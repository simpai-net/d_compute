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
#include "broad_search.h"

int response_brd_search( brd_response_context *context )
{
	int ret;
	dword ipaddr;
	uint16 port;
	char buf[ MAX_BROADCAST_RESPONSE_CONT_LEN ];
	int buf_len;

	bp_assert( NULL != context );
	
	buf_len = MAX_BROADCAST_RESPONSE_CONT_LEN;
	ret = try_recvfrom( context->brd_sock, &ipaddr, &port, buf, buf_len );
	if( 0 > ret )
	{
		return ret;
	}

	printf( "recved broadcast ip %s, port %u\n", ip2str( ipaddr ), port );

	sprintf_s( buf, MAX_BROADCAST_RESPONSE_CONT_LEN, "%u\r\n", context->work_port );
	buf_len = ( int )strlen( buf );

	ret = try_sendto2( context->brd_sock, ipaddr, port, buf, buf_len );
	if( 0 > ret )
	{
		bp_trace( 1, "send computing server search response failed: %u\n", WSAGetLastError() );
	}

	return 0;
}

dword WINAPI reponse_brd_search_thread( void* param )
{
	int ret;
	brd_response_context *context;
	//dword recv_timeout;	

	bp_assert( NULL != param );

	context = ( brd_response_context* )param;

	bp_assert( INVALID_SOCKET == context->brd_sock );
	bp_assert( NULL != context->stop_notifier );

	ret = create_brdcast_sock( &context->brd_sock, context->local_ipaddr, context->local_port );
	if( 0 > ret )
	{
		goto error;
	}

	//recv_timeout = DELAY_TIME;
	//ret = setsockopt( context->brd_sock, SOL_SOCKET, SO_RCVTIMEO, &recv_timeout, sizeof( recv_timeout ) );
	//if( 0 > ret )
	//{
	//	return 1;
	//}

	while( 1 )
	{
		ret = response_brd_search( context );
		if( 0 > ret )
		{
			bp_trace( 1, "response this somputing server search request failed: %u\n", 
				WSAGetLastError() );
		}

		ret = wait_event_time( context->stop_notifier, DELAY_TIME );
		if( 0 == ret )
		{
			break;
		}
	}

	return 0;
error:
	if( INVALID_SOCKET != context->brd_sock )
	{
		closesocket( context->brd_sock );
		context->brd_sock = INVALID_SOCKET;
	}

	return ret;
}

int init_broad_response_thread( void *param )
{
	brd_response_context *context = ( brd_response_context* )param;

	bp_assert( NULL != context );

	context->stop_notifier = CreateEvent( NULL, TRUE, FALSE, NULL );
	if( NULL == context->stop_notifier )
	{
		return -1;
	}

	context->resp_thread = CreateThread( NULL, 0, reponse_brd_search_thread, ( void* )context, NULL, &context->resp_thread_id );
	if( NULL == context->resp_thread )
	{
		CloseHandle( context->stop_notifier );
		context->stop_notifier = NULL;
		return -1;
	}
	return 0;
}

int recv_responsors_info( int sock, on_responsor_searched onsearched, void *context )
{
	int ret;
	char recv_buf[ MAX_BROADCAST_RESPONSE_CONT_LEN ];
	dword recv_len;
	dword recved;
	responsor_info responsor;

	bp_assert( NULL != onsearched );

	recv_len = MAX_BROADCAST_RESPONSE_CONT_LEN;

	while( 1 )
	{
		ret = try_recvfrom( sock, &responsor.ipaddr, &responsor.port, recv_buf, recv_len );
		if( 0 > ret )
		{
			bp_trace( 5, "broadcast recving failed\n" );
			return ret;
		}
		bp_trace( 5,"broadcast recving success\n" );
		recved = ret;

		ret = readln_from_buf( responsor.recv_buf, MAX_BROADCAST_RESPONSE_CONT_LEN, recv_buf, &recved, FALSE );
		if( 0 > ret )
		{
			continue;
		}

		if( 0 == ret )
		{
			bp_trace( 1, "recv falied response, that length is zero\n" );
			continue;
		}

		ret = onsearched( &responsor, context );
		if( 0 > ret )
		{
			return ret;
		}
	}

	return 0;
}

int search_cluster_info( brd_search_context *context )
{
	//int i;
	int ret;

	bp_assert( NULL != context );

	//for( i = 0; i < SEND_TIME ; i ++ )
	//{
		ret = send_broadcast( context->brd_sock, context->remote_ipaddr, 
			context->remote_port,
			CONST_STR_LRLN_POSFIX( BP_SEARCH_COMP_SERVER_CMD ), 
			CONST_STR_LEN( CONST_STR_LRLN_POSFIX( BP_SEARCH_COMP_SERVER_CMD ) ) );
		
		if( 0 > ret )
		{
			return -1;
		}

	//	Sleep( SEND_DELAY_TIME );
	//}

	ret = recv_responsors_info( context->brd_sock, context->onsearched, context->context );

	return ret;
}

dword WINAPI search_responsor( void* param )
{
	int ret;
	dword recv_timeout;
	brd_search_context *context = ( brd_search_context* )param;

	bp_assert( NULL != param );
	bp_assert( INVALID_SOCKET == context->brd_sock );

	ret = create_brdcast_sock( &context->brd_sock, context->local_ipaddr, context->local_port );
	if( 0 > ret )
	{
		goto error;
	}

	recv_timeout = RECV_TIME;
	setsockopt( context->brd_sock, SOL_SOCKET, SO_RCVTIMEO, ( void* )&recv_timeout, sizeof( recv_timeout ) );

	while( 1 )
	{
		search_cluster_info( context );

		ret = wait_event_time( context->stop_notifier, DELAY_TIME );
		if( 0 == ret )
		{
			break;
		}
	}

error:

	if( INVALID_SOCKET != context->brd_sock )
	{
		closesocket( context->brd_sock );
		context->brd_sock = INVALID_SOCKET;
	}

	return 0;
}

int init_broad_search_thread( void *context )
{
	brd_search_context *__context = ( brd_search_context* )context;

	bp_assert( NULL != __context );

	__context->stop_notifier = CreateEvent( NULL, TRUE, FALSE, NULL );
	if( NULL == __context->stop_notifier )
	{
		bp_assert( FALSE );
		return -1;
	}

	__context->search_thread = CreateThread( NULL, 0, search_responsor, ( void* )__context, NULL, &__context->search_thread_id );
	if( NULL == __context->search_thread )
	{
		CloseHandle( __context->stop_notifier );
		__context->stop_notifier = NULL;
		return -1;
	}

	return 0;
}

void stop_broad_search( void *context )
{
	brd_search_context *__context = ( brd_search_context* )context;
	bp_assert( NULL != __context );

	if( NULL != __context->search_thread )
	{
		bp_assert( NULL != __context->stop_notifier );

		SetEvent( __context->stop_notifier );
		WaitForSingleObject( __context->search_thread, INFINITE );
		CloseHandle( __context->search_thread );
		CloseHandle( __context->stop_notifier );
	}

	if( INVALID_SOCKET != __context->brd_sock )
	{
		closesocket( __context->brd_sock );
	}
}

void stop_response_search( void *context )
{
	brd_response_context *__context = ( brd_response_context* )context;
	bp_assert( NULL != __context );

	if( NULL != __context->resp_thread )
	{
		bp_assert( NULL != __context->stop_notifier );

		SetEvent( __context->stop_notifier );
		WaitForSingleObject( __context->resp_thread, INFINITE );
		CloseHandle( __context->resp_thread );
		CloseHandle( __context->stop_notifier );
	}

	if( INVALID_SOCKET != __context->brd_sock )
	{
		closesocket( __context->brd_sock );
	}
}
