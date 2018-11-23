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
#include "controller.h"
#include "network.h"
#include "broadcast.h"
#include "bp_config.h"
#include "cmd_transmit.h"

int add_test_data( dist_server_ext *ext )
{
	int ret;
	hash_key new_key;
	paral_unit_info *new_unit_info;
	ret = create_new_paral_unit_info( &new_unit_info );
	if( 0 > ret )
	{
		return -1;
	}

	new_unit_info->ipaddr = inet_addr( "192.168.0.156" );
	new_unit_info->port = htons( COMP_SERVER_WORK_PORT );

	new_key = make_hash_key( new_unit_info->ipaddr, new_unit_info->port );
	require_mutex( ext->server_infos.infos_mutex );
	add_hash_item( &ext->server_infos.paral_units_info, new_key, new_unit_info );
	release_mutex( ext->server_infos.infos_mutex );

	ret = create_new_paral_unit_info( &new_unit_info );
	if( 0 > ret )
	{
		return -1;
	}

	new_unit_info->ipaddr = inet_addr( "192.168.0.156" );
	new_unit_info->port = htons( COMP_SERVER_WORK_PORT );

	new_key = make_hash_key( new_unit_info->ipaddr, new_unit_info->port );
	require_mutex( ext->server_infos.infos_mutex );
	add_hash_item( &ext->server_infos.paral_units_info, new_key, new_unit_info );
	release_mutex( ext->server_infos.infos_mutex );
	return 0;
}

int close_dis_server( void *context )
{
	dist_server_ext *ext = ( dist_server_ext* )context;
	bp_assert( NULL != ext );

	stop_broad_search( &ext->searcher );
	CloseHandle( ext->server_infos.infos_mutex );
	stop_response_search( &ext->responsor );
	release_paral_unit_infos( &ext->server_infos, TRUE );
	return 0;
}

int dist_server2paral_unit_proc( io_info *info )
{
	return 0;
}

int dist_server2cli_proc( io_info *info )
{
	int ret = 0;
	dist_server_ext *server_ext;
	paral_command *new_paral_command;
	param_info ret_info;
	free_paral_units *free_units;

	bp_assert( NULL != info );
	bp_assert( NULL != info->work_info );
	bp_assert( NULL != info->work_info->server_ext );

	server_ext = info->work_info->server_ext;

	if( cli_connected == info->cur_process )
	{
new_cmd_recv_again:
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

		free_units = ( free_paral_units* )info->context;

		if( free_units == NULL )
		{
			ret = create_new_free_paral_units( &free_units );
			if( 0 > ret )
			{
				ret_info.param_type = PARAM_TYPE_VAL( PARAM_INT_TYPE );
				ret_info.param_int = -1;
				goto none_free_unit_exist;
			}

			ret = get_free_paral_units( &server_ext->server_infos, free_units );
			if( 0 > ret )
			{
				ret_info.param_type = PARAM_TYPE_VAL( PARAM_INT_TYPE );
				ret_info.param_int = -1;
				goto none_free_unit_exist;
			}

			info->context = free_units;
		}

		ret = distribute_command( new_paral_command, 
			&server_ext->server_infos, 
			free_units,
			&ret_info );

		if( 0 >= ret )
		{
			ret_info.param_type = PARAM_TYPE_VAL( PARAM_INT_TYPE );
			ret_info.param_int = -1;
		}

		when_free_units_communication_done( free_units );

none_free_unit_exist:

		if( NULL != free_units )
		{
			release_free_paral_units( free_units );
			info->context = NULL;
		}

		info->cur_process = cli_request_geted;
		ret = response_command_result( info, &ret_info );
		if( 0 > ret )
		{
			if( AYNC_OPER_INCOMPLETE == ret )
			{
				return 0;
			}
		}

		goto new_cmd_recv_again;		
	}
	else if( cli_request_geted == info->cur_process )
	{
		bp_assert( NULL == info->context );
		info->cur_process = cli_connected;
		goto new_cmd_recv_again;
	}
	else
	{
		bp_assert( FALSE );
	}
	return ret;
}

int computing_final_result_linear( paral_command *command, free_paral_units *units, param_info *final_res )
{
	int ret;
	param_infos res_infos;

	ret = init_param_infos( &res_infos );
	if( 0 > ret )
	{
		return ret;
	}

	ret = get_results_set_from_free_units( units, &res_infos );
	if( ret <= 0 )
	{
		return -1;
	}

	ret = combine_command_result( command, res_infos.infos, res_infos.cur_info_num, final_res );
	return ret;
}

int recv_computed_results( paral_command *command, free_paral_units *units, param_info *ret_info )
{
	int ret;
	dword recved = 0;
	int result_recved;

	result_recved = recv_result_from_free_units( units );
	
	if( 0 >= result_recved )
	{
		return -1;
	}

	ret = computing_final_result_linear( command, units, ret_info );
	if( 0 > ret )
	{
		return ret;
	}
	return result_recved;
}

int distribute_command( paral_command *command, cluster_info *infos, free_paral_units *units, param_info *ret_info )
{
	int ret;
	int conn_comp_num;

	ret = dist_cmd2paral_units( command, infos, units );
	if( 0 >= ret )
	{
		return ret;
	}

	ret = send_command2free_units( units );
	if( 0 >= ret )
	{
		return ret;
	}

	conn_comp_num = ret;
	bp_trace( 2, "sended command to %d parallel computing servers\n", ret );

	return recv_computed_results( command, units, ret_info );
}

int init_dist_server( void *param )
{
	int ret;
	dist_server_ext *context;

	context = ( dist_server_ext* )param;
	//bp_assert( NULL == context->server_infos.infos_mutex );
	//bp_assert( NULL == context->server_infos.cluster_not_empty );

	context->searcher.local_ipaddr = NULL;
	context->searcher.remote_ipaddr = "255.255.255.255";
	context->searcher.remote_port = BP_COMP_INFO_RECV_PORT;
	context->searcher.onsearched = on_paral_unit_searched;
	context->searcher.context = ( void* )&context->server_infos;

	ret = init_broad_search_thread( &context->searcher );
	if( 0 > ret )
	{
		goto error;
	}

	context->responsor.brd_sock = INVALID_SOCKET;
	context->responsor.local_ipaddr = NULL;
	context->responsor.local_port = BP_DIST_INFO_RECV_PORT;
	context->responsor.work_port = DIST_SERVER_WORK_PORT;

	context->responsor.stop_notifier = CreateEvent( NULL, TRUE, FALSE, NULL );
	if( NULL == context->responsor.stop_notifier )
	{
		goto error;
	}

	ret = init_broad_response_thread( &context->responsor );
	if( 0 > ret )
	{
		goto error;
	}

	return 0;

error:

	if( NULL != context->searcher.search_thread )
	{
		SetEvent( context->searcher.stop_notifier );
		wait_event_time( context->searcher.search_thread, 2000 );
		CloseHandle( context->searcher.search_thread );
		CloseHandle( context->searcher.stop_notifier );
		context->searcher.search_thread = NULL;
		context->searcher.search_thread_id = 0;
		context->searcher.search_thread = NULL;
	}

	if( NULL != context->server_infos.infos_mutex )
	{
		CloseHandle( context->server_infos.infos_mutex );
		context->server_infos.infos_mutex = NULL;
	}

	if( NULL != context->responsor.stop_notifier )
	{
		CloseHandle( context->responsor.stop_notifier );
		context->responsor.stop_notifier = NULL;
	}
	return ret;
}

int start_dist_server( server_work_info **work_info )
{
	int ret;
	int listen_sock;
	dist_server_ext *server_ext;
	server_work_info *new_work_info;
	
	new_work_info = NULL;
	server_ext = ( dist_server_ext* )malloc( sizeof( dist_server_ext ) );
	if( NULL == server_ext )
	{
		return -E_OUTOFMEMORY;
	}

	server_ext->searcher.brd_sock = INVALID_SOCKET;
	server_ext->searcher.search_thread = NULL;
	server_ext->searcher.search_thread_id = 0;
	server_ext->searcher.stop_notifier = NULL;
	server_ext->searcher.context = NULL;

	server_ext->responsor.brd_sock = INVALID_SOCKET;
	server_ext->responsor.local_ipaddr = NULL;
	server_ext->responsor.local_port = 0;
	server_ext->responsor.resp_thread = NULL;
	server_ext->responsor.resp_thread_id = 0;
	server_ext->responsor.stop_notifier = NULL;

	server_ext->server_infos.infos_mutex = NULL;
	server_ext->server_infos.cluster_not_empty = NULL;

	ret = init_cluster_info( &server_ext->server_infos );
	if( 0 > ret )
	{
		return ret;
	}

	ret = create_server_work_info( &new_work_info, 
		dist_server2cli_proc,
		init_dist_server,
		close_dis_server,
		server_ext );
	if( 0 > ret )
	{
		goto error;
	}

	ret = generate_tcp_listen_sock( &listen_sock, NULL, 
		DIST_SERVER_WORK_PORT, TRUE );
	if( 0 > ret )
	{
		goto error;
	}

	ret = init_add_sock2iocp( listen_sock, new_work_info );
	if( 0 > ret )
	{
		goto error;
	}

	*work_info = new_work_info;
	ret = async_accept( listen_sock, new_work_info );
	return ret;

error:
	*work_info = NULL;
	if( NULL != new_work_info )
	{
		destroy_server_work_info( new_work_info );
	}
	return ret;
}

int stop_dist_server( server_work_info *work_info )
{
	destroy_server_work_info( work_info );
	free( work_info->server_ext );
	return 0;
}