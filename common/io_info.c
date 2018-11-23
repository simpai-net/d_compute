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
#include "io_info.h"

int response_command_result( io_info *info, param_info *ret_info )
{
	int ret;
	char ret_buf[ 512 ];
	
	if( ret_info->param_type == PARAM_TYPE_VAL( PARAM_INT_TYPE ) )
	{
		sprintf( ret_buf, "%s:%d\r\n\r\n", PARAM_INT_TYPE, ret_info->param_int );
	}
	else if( ret_info->param_type == PARAM_TYPE_VAL( PARAM_UINT_TYPE ) )
	{
		sprintf( ret_buf, "%s:%u\r\n\r\n", PARAM_INT_TYPE, ret_info->param_uint );
	}
	else if ( ret_info->param_type == PARAM_TYPE_VAL( PARAM_FLOAT_TYPE ) )
	{
		sprintf( ret_buf, "%s:%f\r\n\r\n", PARAM_INT_TYPE, ret_info->param_float );
	}
	else if( ret_info->param_type == PARAM_TYPE_VAL( PARAM_DOUBLE_TYPE ) )
	{
		sprintf( ret_buf, "%s:%f\r\n\r\n", PARAM_INT_TYPE, ret_info->param_double );
	}
	else
	{
		bp_assert( FALSE );
	}
 
	ret = new_async_send( info, ret_buf, ( int )strlen( ret_buf ) );

	return ret;
}

int extract_ln_from_buf( char* line, dword buf_len, void *param )
{
	io_info *info;
	bp_assert( NULL != param );

	info = ( io_info* )param;

	return readln_from_buf( line, buf_len, info->recv_buf, &info->recved, TRUE );
}

int extract_ln_from_net( char* line, dword buf_len, void *param )
{
	int ret;
	io_info *info;
	bp_assert( NULL != param );

	info = ( io_info* )param;
	ret = readln( ( int )info->cli_sock, line, buf_len, info->recv_buf, &info->recved, info->recv_buf_len );
	return ret;
}

int get_client_request_from_buf( io_info *info, paral_command **command )
{
	bp_assert( NULL != command );
	bp_assert( NULL != info );
	return get_client_request( info, command, extract_ln_from_buf );
}

int get_client_request_from_net( io_info *info, paral_command **command )
{
	bp_assert( NULL != command );
	bp_assert( NULL != info );
	return get_client_request( info, command, extract_ln_from_net );
}

int get_client_request( io_info *info, paral_command **command, recv_new_line recv_func )
{
	int ret;
	dword error_code;
	char recv_line[ MAX_LINE_LEN ];
	dword recv_time;
	paral_command *new_paral_command;
	
	ret = create_new_command( &new_paral_command );
	if( 0 > ret )
	{
		goto error;
	}

	recv_time = 0;

	while( 1 )
	{
		ret = recv_func( recv_line, MAX_LINE_LEN, ( void* )info );
		if( ret < 0 )
		{
			error_code = WSAGetLastError();
			if( WSA_IO_PENDING == error_code )
			{
				ret = -WSA_IO_PENDING;
			}

			goto error;
		}
		else if( ret == 0 )
		{
			break;
		}
		else
		{
			switch( recv_time )
			{
			case 0:
				ret = add_command( new_paral_command, recv_line );
				recv_time ++;
				break;
			default:
				ret = add_params( new_paral_command, recv_line );
				recv_time ++;
				break;
			}
		}
	}

	ret = add_command_term( new_paral_command );
	if( 0 > ret )
	{
		goto error;
	}
	
	ret = add_param_term( new_paral_command );
	if( 0 > ret )
	{
		goto error;
	}
	*command = new_paral_command;
	return ret;

error:
	if( NULL != new_paral_command )
	{
		destroy_paral_command( new_paral_command );
	}

	*command = NULL;
	return ret;
}

int init_io_info( void *info, int sock, server_work_info *work_info )
{
	io_info *new_io_info;
	
	bp_assert( NULL != info );
	bp_assert( NULL != work_info );
	bp_assert( INVALID_SOCKET != sock );

	new_io_info = info;

	new_io_info->io_info_mutex = CreateMutex( NULL, FALSE, NULL );
	if( NULL == new_io_info->io_info_mutex )
	{
		goto error;
	}

	memset( &new_io_info->overlapped, 0, sizeof( new_io_info->overlapped ) ); 
	new_io_info->cur_process = 0;
	new_io_info->local_addr = 0;
	new_io_info->local_port = 0;
	new_io_info->operation = NONE_OPERA;
	new_io_info->cli_sock = sock;
	new_io_info->context = NULL;
	
	new_io_info->recv_buf = ( char* )malloc( DEF_IO_BUF_SIZE );
	if( NULL == new_io_info->recv_buf )
	{
		goto error;
	}
	
	new_io_info->recv_len = 0;
	new_io_info->recved = 0;
	new_io_info->recv_buf_len = DEF_IO_BUF_SIZE;

	new_io_info->send_buf = ( char* )malloc( DEF_IO_BUF_SIZE );
	if( NULL == new_io_info->send_buf )
	{
		goto error;
	}

	new_io_info->send_len = 0;
	new_io_info->sended = 0;
	new_io_info->send_buf_len = DEF_IO_BUF_SIZE;

	new_io_info->work_info = work_info;

	return 0;

error:
	if( NULL != new_io_info->send_buf )
	{
		free( new_io_info->send_buf );
	}

	if( NULL != new_io_info->recv_buf )
	{
		free( new_io_info->recv_buf );
	}

	if( NULL != new_io_info->io_info_mutex )
	{
		CloseHandle( new_io_info->io_info_mutex );
	}

	return -1;
}

int create_new_io_info( io_info **info, int sock, server_work_info *work_info )
{
	int ret;
	io_info *new_io_info;

	new_io_info = ( io_info* )malloc( sizeof( io_info ) );
	if( NULL == new_io_info )
	{
		*info = NULL;
		return -E_OUTOFMEMORY;
	}

	ret = init_io_info( new_io_info, sock, work_info );

	if( 0 > ret )
	{
		destroy_io_info( new_io_info );
		new_io_info = NULL;
	}

	*info = new_io_info;
	return ret;
}


void prepare_async_opera( io_info *info )
{
	memset( &info->overlapped, 0, sizeof( info->overlapped ) );
}

void async_close( io_info *info )
{
	if( INVALID_SOCKET != info->cli_sock )
	{
		info->operation = CLOSE_OPERA;
		closesocket( info->cli_sock );
		info->cli_sock = INVALID_SOCKET;
		return;
	}
	else
	{
		destroy_io_info_element( info );
	}
}

void destroy_io_info_element( io_info *info )
{
	server_work_info *work_info;
	bp_assert( NULL != info );
	bp_assert( NULL != info->work_info );
	
	work_info = info->work_info;

	require_mutex( work_info->io_info_list_mutex );
	del_list_element( work_info->io_info_list, info );
	release_mutex( work_info->io_info_list_mutex );

	destroy_io_info( info );
}

int add_sock2iocp( io_info *info, server_work_info *work_info )
{
	HANDLE comp_port;
	bp_assert( NULL != work_info );
	bp_assert( INVALID_HANDLE_VALUE != work_info->comp_port );
	bp_assert( INVALID_SOCKET != info->cli_sock );

	comp_port = CreateIoCompletionPort( ( HANDLE )info->cli_sock,
		( HANDLE )work_info->comp_port,
		( dword )info,
		0 );		

	if( NULL != comp_port )
	{
		bp_assert( comp_port == work_info->comp_port );
		require_mutex( work_info->io_info_list_mutex );
		add_list_element( work_info->io_info_list, ( list_element )info );
		release_mutex( work_info->io_info_list_mutex );
		return 0;
	}

	destroy_io_info( info );
	return -1;
}

int new_free_async_recv( io_info *info )
{
	int ret;

	info->recv_len = 0;

	ret = realloc_buf_needed( &info->recv_buf, &info->recv_buf_len, info->recved + MAX_FREE_MODE_BUF_LEN );
	if( 0 > ret )
	{
		return ret;
	}

	return async_recv( info, FREE_MODE );
}

int new_async_recv( io_info *info, int recv_len )
{
	int ret;
	
	info->recv_len = recv_len + info->recved;

	ret = realloc_buf_needed( &info->recv_buf, &info->recv_buf_len, info->recv_len );
	if( 0 > ret )
	{
		return ret;
	}

	return async_recv( info, 0 );
}

int new_async_send( io_info *info, char *send_data, int data_len )
{
	int ret;
	ret = realloc_buf_needed( &info->send_buf, &info->send_buf_len, info->send_len + data_len );
	if( 0 > ret )
	{
		return ret;
	}

	memcpy( info->send_buf + info->send_len, send_data, data_len );
	info->send_len += data_len;

	return async_send( info );
}

int get_async_result( io_info *info, int wait )
{
	int ret;
	dword transferred;
	dword flags;
	dword errcode;

	bp_assert( NULL != info );

	ret = WSAGetOverlappedResult( info->cli_sock,
		&( info->overlapped ),
		&transferred,
		wait,
		&flags );

	if( FALSE == ret )
	{
		errcode = WSAGetLastError();
		if( WSA_IO_INCOMPLETE == errcode )
		{
			return AYNC_OPER_INCOMPLETE;
		}
		bp_trace( 0, "get overlapped result failed, error code is %u\n", errcode );
		return -1;
	}

	if( SEND_OPERA == info->operation )
	{
		info->sended += transferred;
		return 0;
	}
	else if( RECV_OPERA == info->operation )
	{
		info->recved += transferred;
		return 0;
	}
	else
	{
		bp_assert( FALSE );
		return -1;
	}
}

int async_recv( io_info *info, int flags )
{
	dword flag = 0;
	//dword recved;
	dword errcode;
	int ret;

	bp_assert( NULL != info );
	bp_assert( INVALID_SOCKET != info->cli_sock );

	prepare_async_opera( info );
	info->operation = RECV_OPERA;
	info->wsabuf.buf = info->recv_buf + info->recved;
	if( flags & FREE_MODE )
	{
		bp_assert( info->recv_len <= info->recved );
		info->wsabuf.len = info->recv_buf_len - info->recved;
	}
	else
	{
		bp_assert( info->recv_len > info->recved );
		info->wsabuf.len = info->recv_len - info->recved;
	}
	
	ret = WSARecv( info->cli_sock,
		&( info->wsabuf ),
		1,
		NULL,
		&flag,
		&( info->overlapped ),
		NULL );
	
	if( SOCKET_ERROR == ret )
	{
		errcode = WSAGetLastError();

		if( WSA_IO_PENDING != errcode )
		{
			bp_assert( WSAEINPROGRESS != errcode );
			bp_trace( 1, "WSARecv falied : %d\n", errcode );
			destroy_io_info_element( info );
			return -1;
		}

		return AYNC_OPER_INCOMPLETE;
	}

	ret = get_async_result( info, FALSE );
	if( 0 > ret )
	{
		bp_assert( FALSE );
	}

	return ret;
}

int async_send( io_info *info )
{
	dword flag = 0;
	int ret;
	dword errcode;

	prepare_async_opera( info );
	info->operation = SEND_OPERA;
	info->wsabuf.buf = info->send_buf + info->sended;
	info->wsabuf.len = info->send_len - info->sended;
	
	ret = WSASend( info->cli_sock,
		&( info->wsabuf ),
		1,
		NULL,
		flag,
		&( info->overlapped ),
		NULL );	

	if( SOCKET_ERROR == ret )
	{
		errcode = WSAGetLastError();

		if( WSA_IO_PENDING != errcode )
		{
			bp_assert( WSAEINPROGRESS != errcode );
			bp_trace( 1, "WSASend falied : %d\n", errcode );
			destroy_io_info_element( info );
			return -1;
		}

		return AYNC_OPER_INCOMPLETE;
	}
	
	ret = onsended( info );
	bp_assert( 0 == ret );
	return ret;
}

int async_accept( int listen_sock, server_work_info *work_info  )
{
	dword err_code;
	dword bytes = 0;
	int sock;
	int ret = FALSE;

	io_info *new_io_info = NULL;

	//bp_assert( NULL != work_info );
	bp_assert( INVALID_SOCKET != listen_sock );

	sock = ( int )WSASocket( AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED );

	if( INVALID_SOCKET == sock )
	{
		goto error;
	}

	ret = create_new_io_info( &new_io_info, sock, work_info );
	if( 0 > ret )
	{
		return ret;
	}
	
	new_io_info->operation = ACCEPT_OPERA;
	
    ret = AcceptEx(
		listen_sock,
		new_io_info->cli_sock,
		new_io_info->recv_buf,
		0,
		sizeof( struct sockaddr_in ) + 16,
		sizeof( struct sockaddr_in ) + 16,
		&bytes,
		&new_io_info->overlapped
		);

	if( FALSE == ret )
	{
		err_code = WSAGetLastError();
		if( ERROR_IO_PENDING != err_code )
		{
			bp_trace( 2, "accpet new client connext by AcceptEx() function failed error code is: %u\n", err_code );
			goto error;
		}
	}
	
	return TRUE == ret ? 0 : ( ERROR_IO_PENDING == err_code ? 0 : -1 );

error:
	if( NULL != new_io_info )
	{
		destroy_io_info( new_io_info );
	}

	return -1;
}

int onrecved( io_info *cur_io_info )
{
	int ret = 0;
	server_work_info *work_info;
	bp_assert( NULL != cur_io_info->work_info );
	work_info = cur_io_info->work_info;

	bp_assert( NULL != work_info->cur_proc_func );
	
	if( cur_io_info->recved >= cur_io_info->recv_len )
		ret = work_info->cur_proc_func( cur_io_info );
	return ret;
}

int onsended( io_info *cur_io_info )
{
	int ret;
	server_work_info *work_info;
	bp_assert( NULL != cur_io_info->work_info );
	work_info = cur_io_info->work_info;

	bp_assert( NULL != work_info->cur_proc_func );
	if( cur_io_info->sended >= cur_io_info->send_len )
		ret = work_info->cur_proc_func( cur_io_info );
	return ret;
}

int onaccepted( io_info *cur_io_info )
{
	int ret;
	struct sockaddr_in *local_addr;
	struct sockaddr_in *remote_addr;
	server_work_info *work_info;

	int32 local_addr_len;
	int32 remote_addr_len;

	local_addr = NULL;
	remote_addr = NULL;

	GetAcceptExSockaddrs(
		cur_io_info->recv_buf,
		0,
		sizeof( struct sockaddr_in ) + 16,
		sizeof( struct sockaddr_in ) + 16,
		( struct sockaddr** )&local_addr,
		&local_addr_len,
		( struct sockaddr** )&remote_addr,
		&remote_addr_len
		);

	bp_assert( 0 != local_addr->sin_port && 
		0 != remote_addr->sin_port );

	bp_trace( ACCEPT_SOCK_INFO_LEVEL, "accept new client socket: %u\n local ip: %s local port: %u\n peer ip: %s peer port: %u\n", 
		cur_io_info->cli_sock , 
		inet_ntoa( local_addr->sin_addr ), 
		ntohs( local_addr->sin_port ),
		inet_ntoa( remote_addr->sin_addr ), 
		ntohs( remote_addr->sin_port ) );

	require_mutex( cur_io_info->io_info_mutex );
	cur_io_info->local_addr = ntohl( local_addr->sin_addr.S_un.S_addr );
	cur_io_info->remote_addr = ntohl( remote_addr->sin_addr.S_un.S_addr );
	cur_io_info->local_port = ntohs( local_addr->sin_port );
	cur_io_info->remote_port = ntohs( remote_addr->sin_port );
	
	add_sock2iocp( cur_io_info, cur_io_info->work_info );

	release_mutex( cur_io_info->io_info_mutex );

	bp_assert( NULL != cur_io_info->work_info );
	work_info = cur_io_info->work_info;
	
	cur_io_info->cur_process = io_accept_state;
	bp_assert( NULL != work_info->cur_proc_func );
	ret = work_info->cur_proc_func( cur_io_info );

	return ret;
}

void destroy_io_info( io_info *info )
{
	bp_assert( NULL != info );
	if( NULL != info->io_info_mutex )
	{
		CloseHandle( info->io_info_mutex );
	}

	if( INVALID_SOCKET != info->cli_sock )
	{
		closesocket( info->cli_sock );
	}

	if( NULL != info->recv_buf )
	{
		free( info->recv_buf );
	}

	if( NULL != info->send_buf )
	{
		free( info->send_buf );
	}

	free( info );
}

void destroy_all_io_info( server_work_info *work_info )
{
	list_element info;
	dlist *io_info_item;
	dlist *next_io_info_item;

	bp_assert( NULL != work_info );
	io_info_item = work_info->io_info_list;;
	
	for( ; ; )
	{
		if( NULL == io_info_item )
			break;

		next_io_info_item = io_info_item->next;
		info = io_info_item->info;
		del_list_item( io_info_item, NULL );
		destroy_io_info( info );
	}
}
