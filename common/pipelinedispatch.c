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
#include "computing_unit_info.h"
#include "pipelinedispatch.h"
#include "io_info.h"

int dispatch_pipeline_cmd_begin( paral_unit_info *unit_info, dword fparam, dword sparam )
{
	int ret;
	hash_table *table;

}

typedef struct __paral_unit_record
{
	paral_unit_info *unit_info;
	dword comp_layer;
} paral_unit_record;

int32 allocate_pipeline_comp_unit( cluster_info *cluster )
{
#define DEF_PARAL_COMP_LEVEL 10
	hash_table *table;
	void *item_pos;
	dword wait_alloc_array_len;
	paral_unit_info *unit_info;
	paral_unit_info ***wait_alloc_unit;
	dword *wait_alloc_unit_count;
	dword *wait_alloc_arr_len;
	int32 i;
	int32 j;

	wait_alloc_unit = ( paral_unit_info*** )malloc( sizeof( paral_unit_info** ) * DEF_PARAL_COMP_LEVEL );
	
	if( NULL == wait_alloc_unit )
	{
		return -E_OUTOFMEMORY;
	}

	wait_alloc_unit_count = ( dword* )malloc( sizeof( dword ) * DEF_PARAL_COMP_LEVEL );
	if( NULL == wait_alloc_unit_count )
	{
		free( wait_alloc_unit_count );
		return -E_OUTOFMEMORY;
	}

	wait_alloc_arr_len = ( dword* )malloc( sizeof( dword ) * DEF_PARAL_COMP_LEVEL );
	if( NULL == wait_alloc_arr_len )
	{
		free( wait_alloc_unit_count );
		return -E_OUTOFMEMORY;
	}

	for( i = 0; i < DEF_PARAL_COMP_LEVEL; i ++ )
	{
		wait_alloc_unit[ i ] ( paral_unit_info** )malloc( sizeof( paral_unit_info* ) * DEF_PARAL_COMP_LEVEL );
		if( NULL == wait_alloc_unit[ i ] )
		{
			for( j = 0; j < i; j ++ )
			{
				free( wait_alloc_unit[ j ] );
			}

			free( wait_alloc_unit );
			free( wait_alloc_unit_count );
			return -E_OUTOFMEMORY;
		}

		wait_alloc_unit_count[ i ] = 0;
		wait_alloc_arr_len[ i ] = DEF_PARAL_COMP_LEVEL;
	}

	wait_alloc_array_len = DEF_PARAL_COMP_LEVEL;
	item_pos = NULL;

	table = &cluster->paral_units_info;

	while( 1 )
	{
		item_pos = get_next_item_value( item_pos, table, &( ( hash_value )unit_info ) );
		if( NULL == item_pos )
		{
			bp_trace( 3, "no computing server exist when do action with them\n" );
			release_mutex( cluster->infos_mutex );
			return 0;
		}

		if( unit_info->comp_order > wait_alloc_array_len )
		{
			paral_unit_info ***alloc_unit;
			alloc_unit = ( paral_unit_info** )realloc( sizeof( paral_unit_info** ) * DEF_PARAL_COMP_LEVEL );
			if( NULL == alloc_unit )
			{
				for( i = 0; i < wait_alloc_array_len; i ++ )
				{
					free( wait_alloc_unit[ i ] );
				}

				free( wait_alloc_unit );
				free( wait_alloc_unit_count );
			}

			for( i = wait_alloc_unit_count; i < unit_info->comp_order; i ++ )
			{
				alloc_unit[ i ] = ( paral_unit_info* )malloc( sizeof( paral_unit_info* ) * DEF_PARAL_COMP_LEVEL );
				if( NULL == alloc_unit[ i ] )
				{
					return -E_OUTOFMEMORY;
				}
			}
		}

		if( 0 != wait_alloc_unit_count[ unit_info->comp_order - 1 ] )
		{
			bp_assert( NULL != wait_alloc_unit[ unit_info->comp_order ][ i ] );
			wait_alloc_unit[ unit_info->comp_order - 1 ][ i ]->next_unit = unit_info;
			memmove( wait_alloc_unit[ unit_info->comp_order - 1 ], wait_alloc_unit[ unit_info->comp_order - 1 ] + 1, sizeof( paral_unit_info* ) * wait_alloc_unit_count[ unit_info->comp_order - 1 ] - 1 );
			wait_alloc_unit_count[ unit_info->comp_order - 1 ] -= -1;
		}

		if( wait_alloc_unit_count[ unit_info->comp_order - 1 ] + 1 > wait_alloc_array_len[ unit_info->comp_order - 1 ] )
		{
			unit_info *alloc_info;
			alloc_info = realloc( wait_alloc_unit[ unit_info->comp_order - 1], sizeof( paral_unit_info * ) * wait_alloc_array_len[ unit_info->comp_order ] + 10 );
			if( NULL == alloc_info )
			{
				return -E_OUTOFMEMORY;
			}
			wait_alloc_unit[ unit_info->comp_order - 1 ] = alloc_info;
			wait_alloc_array_len[ unit_info->comp_order - 1 ] = wait_alloc_array_len[ unit_info->comp_order - 1 ] + 10;
		}

		wait_alloc_unit[ unit_info->comp_order][ wait_alloc_unit_count[ unit_info->comp_order - 1 ] = unit_info;
	}
}

int32 sort_pipeline_comp_unit( cluster_info *cluster, dword compute_layer_num )
{
	//very simple sort algrithm.just sort average.

	int32 count;
	hash_table *table;
	void *item_pos;
	table = &cluster->paral_units_info;
	paral_unit_info *unit_info;

	count = 0;
	item_pos = NULL;
	require_mutex( cluster->infos_mutex );

	while( 1 )
	{
		item_pos = get_next_item_value( item_pos, table, &( ( hash_value )unit_info ) );
		if( NULL == item_pos )
		{
			bp_trace( 3, "no computing server exist when do action with them\n" );
			release_mutex( cluster->infos_mutex );
			return 0;
		}

		unit_info->comp_order = count % compute_layer_num;
	}

	release_mutex( cluster->infos_mutex );
	return 0;
}

int32 dist_pipeline_cmd2paral_units( paral_command *command, cluster_info *infos, free_paral_units *units )
{
	int ret;
	unsigned int inited_num;
	unsigned int dispatched_num;
	void *item_pos;
	char __command[ MAX_ONE_CMD_LEN ];

	dispatched_num = 0;
	inited_num = 0;
	item_pos = NULL;

	ret = readln_from_buf( __command, MAX_ONE_CMD_LEN, command->command, &command->cmd_cur_len, FALSE );
	if( ret < 0 )
	{
		return -1;
	}

	inited_num = dispatch_cmd_begin2free_units( infos, units, __command );

	if( inited_num > 0 )
	{
		ret = dispatch_params2free_units_linear( units, command->params, command->param_cur_len );
		if( 0 < ret )
		{
			ret = dispatch_cmd_end_sign2free_units( units );
		}
	}

	return ret;
}

int paral_unit_proc( io_info *info )
{
	int ret;
	if( request_accepted == info->cur_process )
	{
		paral_command *new_paral_command;
		param_info ret_info;

recv_new_cmd_again:
		ret = peek_next_unit_info_end_sign( info->recv_buf, info->recved );
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

		ret = get_next_unit_info( info );
		if( 0 > ret )
		{
			return ret;
		}

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

#define UNIT_INFO_LEN 512
int32 get_next_unit_info( io_info *info )
{
	int32 ret;
	char recv_ln[ UNIT_INFO_LEN ];
	char *recved_buf;
	recved_buf = info->recv_buf;
	ret = readln_from_buf( recv_ln, UNIT_INFO_LEN, recved_buf, &info->recved, TRUE );
	if( ret < 0 )
	{
		return -1;
	}
	io_info->next_unit_ip = inetaddr( recv_ln );

	ret = readln_from_buf( recv_ln, UNIT_INFO_LEN, recved_buf, &info->recved, TRUE );
	if( ret < 0 )
	{
		return -1;
	}
	io_info->next_unit_port = ntohs( atoi( recv_ln ) );
	return 0;
}

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
 
	ret = send_res2next_unit( info, ret_buf, ( int )strlen( ret_buf ) );

	return ret;
}

int32 send_res2next_unit( io_info *info, char *buf, dword len )
{
	int32 ret;
	int32 unit_sock;
	io_info *new_io_info;

	ret = connect2server2( &unit_sock, info->next_unit_ip, info->next_unit_port );
	if( 0 > ret )
	{
		return ret;
	}

	ret = create_new_io_info( &new_io_info, unit_sock, info->work_info );
	if( 0 > ret )
	{
		goto error;
	}

	new_async_send( new_io_info, buf, len );

error:
	if( NULL != new_io_info )
	{
		destroy_io_info( new_io_info );
	}
	return ret;
}