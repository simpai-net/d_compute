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
#include "iocompport.h"
#include "network.h"
#include "bp_config.h"
#include "computing_server.h"
#include "broadcast.h"
#include "cmd_transmit.h"
#include "io_info.h"

int parallel_node_work( io_info *info )
{
	int ret;
	if( request_accepted == info->cur_process )
	{
		paral_command *new_paral_command;
		param_info ret_info;

recv_new_cmd_again:
		ret = peek_cmd_end( info->recv_buf, info->recved );
		if( 0 > ret )
		{
			ret = new_free_async_recv( info );
			if( 0 > ret )
			{
				if( AYNC_OPER_INCOMPLETE == ret )
				{
					return 0;
				}
				return ret;
			}

			return ret;
		}

		ret = get_client_request_from_buf( info, &new_paral_command );
		if( 0 > ret )
		{
			return ret;
		}

		ret = execute_command( new_paral_command, &ret_info );
		if( 0 > ret )
		{
			return ret;
		}

		ret = response_command_result( info, &ret_info );
		if( 0 > ret )
		{
			return ret;
		}
		goto recv_new_cmd_again;
	}

	return 0;
}

int close_parallel_node( void *context )
{
	paral_unit_ext *ext = ( paral_unit_ext* )context;
	bp_assert( NULL != ext );

	if( NULL != ext->responsor.resp_thread )
	{
		SetEvent( ext->responsor.stop_notifier );
		WaitForSingleObject( ext->responsor.resp_thread, INFINITE );
		CloseHandle( ext->responsor.resp_thread );
	};
	return 0;
}

int init_parallel_node( void *context )
{
	paral_unit_ext *ext = ( paral_unit_ext* )context;
	bp_assert( NULL != ext );

	ext->responsor.brd_sock = INVALID_SOCKET;
	ext->responsor.resp_thread = NULL;
	ext->responsor.resp_thread_id = 0;
	ext->responsor.stop_notifier = NULL;
	ext->responsor.local_ipaddr = NULL;
	ext->responsor.local_port = BP_COMP_INFO_RECV_PORT;
	ext->responsor.work_port = COMP_SERVER_WORK_PORT;
	return init_broad_response_thread( &ext->responsor );
}

int start_parallel_node( server_work_info **work_info )
{
	int ret;
	int listen_sock;
	server_work_info *new_work_info;
	paral_unit_ext *server_ext;

	new_work_info = NULL;
	server_ext = ( paral_unit_ext* )malloc( sizeof( paral_unit_ext ) );
	if( NULL == server_ext )
	{
		return -E_OUTOFMEMORY;
	}

	ret = create_server_work_info( &new_work_info, 
		parallel_node_work,
		init_parallel_node,
		close_parallel_node,
		server_ext );
	if( 0 > ret )
	{
		goto error;
	}

	ret = generate_tcp_listen_sock( &listen_sock, NULL, 
		COMP_SERVER_WORK_PORT, TRUE );
	if( 0 > ret )
	{
		goto error;
	}

	ret = init_add_sock2iocp( listen_sock, new_work_info );
	if( 0 > ret )
	{
		goto error;
	}

	ret = async_accept( listen_sock, new_work_info );
	if( 0 > ret )
	{
		goto error;
	}

	*work_info = new_work_info;
	return 0;

error:
	*work_info = NULL;
	if( NULL != new_work_info )
	{
		destroy_server_work_info( new_work_info );
	}
	return ret;
}

void stop_parallel_node( server_work_info *work_info )
{
	destroy_server_work_info( work_info );
}