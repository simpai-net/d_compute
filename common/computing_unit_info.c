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
#include "hash.h"
#include "computing_unit_info.h"
#include "cmd_transmit.h"

void when_paral_unit_geted( paral_unit_info *unit_info )
{
	bp_assert( paral_inited == unit_info->paral_proc ); //because the paral_proc will set to paral_inited when init one new paral_unit_info.
}

void when_paral_unit_hold( paral_unit_info *unit_info )
{
	bp_assert( paral_inited == unit_info->paral_proc );
	unit_info->paral_proc = paral_geted;
}

void when_paral_unit_connected( paral_unit_info *unit_info )
{
	bp_assert( paral_geted == unit_info->paral_proc );
	unit_info->paral_proc = cmd_recving;
	//don't need to flush I/O buffer here, because do that already when initilizing and closing of paral_unit_info.
}

void when_paral_unit_command_recved( paral_unit_info *unit_info )
{
	bp_assert( cmd_recving == unit_info->paral_proc );
	unit_info->paral_proc = cmd_sending;
}

void when_paral_unit_cmd_sended( paral_unit_info *unit_info )
{
	bp_assert( NULL != unit_info );
	bp_assert( cmd_sending == unit_info->paral_proc );
	unit_info->paral_proc = res_recving;
}

void when_paral_unit_result_recved( paral_unit_info *unit_info )
{
	bp_assert( NULL != unit_info );
	bp_assert( res_recving == unit_info->paral_proc );
	unit_info->paral_proc = cmd_recving;
}

void when_paral_unit_closed( paral_unit_info *unit_info )
{
	bp_assert( NULL != unit_info );
	unit_info->paral_proc = paral_geted;
	flush_paral_unit_buf( unit_info, FLUSH_INPUT | FLUSH_OUTPUT );
}

void when_paral_unit_holding_release( paral_unit_info *unit_info )
{
	bp_assert( NULL != unit_info );
	bp_assert( paral_geted == unit_info->paral_proc );

	unit_info->paral_proc = paral_inited;
}

int hold_paral_unit( paral_unit_info *unit_info, paral_unit_info **holded )
{
	bp_assert( NULL != unit_info );
	bp_assert( NULL != holded );

	if( unit_info->paral_proc == paral_inited )
	{
		when_paral_unit_hold( unit_info );
		*holded = unit_info;
		return 0;
	}

	*holded = NULL;
	return -1;
}

void release_paral_unit_holding( paral_unit_info *unit_info )
{
	bp_assert( NULL != unit_info );
	if( cmd_recving <= unit_info->paral_proc )
	{
		close_paral_unit( unit_info );
	}

	when_paral_unit_holding_release( unit_info );
}

void when_free_units_communication_done( free_paral_units *units )
{
	dlist *item;
	paral_unit_info *unit;

	item = units->free_units;

	for( ; ; )
	{
		item = get_next_list_element( item, &( ( paral_unit_info* )unit ) );
		if ( NULL == item )
		{
			break;
		}

		release_paral_unit_holding( ( paral_unit_info* )item->info );
	}

	return;
}

int init_free_paral_units( free_paral_units *free_units )
{
	int ret;

	bp_assert( NULL != free_units );
	ret = init_list_element( &free_units->free_units );
	if( 0 > ret )
	{
		return ret;
	}

	//will do some thing here, later.
	return ret;
}

int create_new_free_paral_units( free_paral_units **free_units )
{
	int ret;

	free_paral_units *units;

	bp_assert( NULL != free_units );

	units = ( free_paral_units* )malloc( sizeof( free_paral_units ) );
	if( NULL == units )
	{
		*free_units = NULL;
		return -E_OUTOFMEMORY;
	}

	ret = init_free_paral_units( units );
	if( ret < 0 )
	{
		free( units );
		*free_units = NULL;
		return ret;
	}

	*free_units = units;
	return 0;
}

void release_free_paral_units( free_paral_units *units )
{
	destroy_list( units->free_units, NULL );
	free( units );
}

int get_free_paral_unit( paral_unit_info *unit_info, dword fparam, dword sparam )
{
	int ret;
	paral_unit_info *holded;
	free_paral_units *units;

	bp_assert( NULL != ( void* )fparam );
	
	units = ( free_paral_units* )fparam;
	bp_assert( NULL != units->free_units );

	ret = hold_paral_unit( unit_info, &holded );
	if( 0 > ret )
	{
		return -1;
	}

	ret = add_list_element( units->free_units, ( list_element )unit_info );
	if( 0 > ret )
	{
		return ret;
	}

	return 0;
}

/*
 * be suare the free_units list only one thread own.
 */
int get_free_paral_units( cluster_info *cluster, free_paral_units *free_units )
{
	int ret;
	ret = action_on_cluster( cluster, ( dword )free_units, ( dword )NULL, get_free_paral_unit );
	if( 0 >= ret )
	{
		return ret - 1;
	}

	return ret;
}

int release_free_unit_holding( paral_unit_info *unit_info, dword fparam, dword sparam )
{
	bp_assert( NULL != unit_info );
	release_paral_unit_holding( unit_info );
	return 0;
}

void flush_paral_unit_buf( paral_unit_info *unit_info, dword flag )
{
	if( flag & FLUSH_INPUT )
	{
		unit_info->res_len = 0;
	}

	if( flag & FLUSH_INPUT )
	{
		unit_info->cmd_len = 0;
	}
}

int on_paral_unit_searched( responsor_info *responsor, void *context )
{
	int ret;
	hash_key new_key;
	cluster_info *__context;
	paral_unit_info *new_unit_info = NULL;
	
	bp_assert( NULL != responsor && NULL != context );

	bp_trace( 8, "find dist server, it's ip: %s, port: %u\n", ip2str( responsor->ipaddr ), ntohs( responsor->port ) );
	__context = ( cluster_info* )context;
	ret = check_addr_sanity2( responsor->ipaddr, responsor->port );
	if( 0 > ret )
	{
		bp_trace( 1, "recv sanity response ip %s, port %u\n", ip2str( responsor->ipaddr ), responsor->port );
		return -1;
	}

	ret = create_new_paral_unit_info( &new_unit_info );
	if( 0 > ret )
	{
		goto error;
	}

	responsor->port = atoi( responsor->recv_buf );
	new_unit_info->ipaddr = responsor->ipaddr;
	new_unit_info->port = responsor->port;

	new_key = make_hash_key( responsor->ipaddr, ( dword )responsor->port );
	require_mutex( __context->infos_mutex );
	ret = add_hash_item( &__context->paral_units_info, new_key, new_unit_info );
	release_mutex( __context->infos_mutex );

	if( 0 > ret )
	{
		return ret;
	}

	when_paral_unit_geted( new_unit_info );
	ret = SetEvent( __context->cluster_not_empty );
	bp_assert( TRUE == ret );
	return 0;

error:
	if( NULL != new_unit_info )
	{
		release_paral_unit_info( new_unit_info, TRUE );
	}
	return -1;
}

void destroy_cluster_value( hash_value *value, dword param )
{
	release_paral_unit_info( ( paral_unit_info* )value, param );
}

void release_paral_unit_infos( cluster_info *cluster, int self_free )
{
	int ret;
	hash_table *table;

	bp_assert( NULL != cluster );
	if( NULL != cluster->infos_mutex );
	{
		ret = require_mutex_time( cluster->infos_mutex, 500 );
		if( 0 > ret )
		{
			bp_assert( FALSE );
		}
	}

	table = &cluster->paral_units_info;
	destroy_hash_table( table, destroy_cluster_value, self_free );

	CloseHandle( cluster->cluster_not_empty );
	CloseHandle( cluster->infos_mutex );

	free( cluster );
}

int create_new_paral_unit_info( paral_unit_info **info )
{
	paral_unit_info *new_unit_info;

	bp_assert( NULL != info );
	new_unit_info = ( paral_unit_info* )malloc( sizeof( paral_unit_info ) );
	if( NULL == new_unit_info )
	{
		*info = NULL;
		return -E_OUTOFMEMORY;
	}

	new_unit_info->paral_proc = paral_inited;

	new_unit_info->ipaddr = 0;
	new_unit_info->peer_sock = INVALID_SOCKET;
	new_unit_info->port = 0;

	new_unit_info->result = ( char* )malloc( DEF_PARAMS_LEN );
	if( NULL == new_unit_info->result )
	{
		*info = NULL;
		return -E_OUTOFMEMORY;
	}
	new_unit_info->res_buf_len = DEF_PARAMS_LEN;
	new_unit_info->res_len = 0;

#define DEF_PARAL_UNIT_CMD_LEN DEF_CMD_LEN + DEF_PARAMS_LEN 
	new_unit_info->command = ( char* )malloc( DEF_PARAL_UNIT_CMD_LEN );
	new_unit_info->cmd_buf_len = DEF_PARAL_UNIT_CMD_LEN;
	new_unit_info->cmd_len = 0;
	if( NULL == new_unit_info->command )
	{
		*info = NULL;
		return -E_OUTOFMEMORY;
	}

	*info = new_unit_info;
	return 0;
}

void release_paral_unit_info( paral_unit_info *info, int self_free )
{
	bp_assert( NULL != info );
	bp_assert( paral_inited == info->paral_proc );

	if( INVALID_SOCKET != info->peer_sock )
	{
		shutdown( info->peer_sock, SD_BOTH );
		closesocket( info->peer_sock );
	}

	if( NULL != info->command )
	{
		free( info->command );
	}

	if( NULL != info->result )
	{
		free( info->result );
	}

	if( self_free )
	{
		free( info );
	}
}

int init_cluster_info( cluster_info *cluster )
{
	int ret;
	bp_assert( NULL != cluster );

	ret = -1;
	cluster->cluster_not_empty = CreateEvent( NULL, TRUE, FALSE, NULL );
	if( NULL == cluster->cluster_not_empty )
	{
		goto error;
	}

	cluster->infos_mutex = CreateMutex( NULL, FALSE, NULL );
	if( NULL == cluster->infos_mutex )
	{
		goto error;
	}

	ret = init_hash_table( &cluster->paral_units_info, PARAL_UNIT_HASH_NUM );
	if( 0 > ret )
	{
		goto error;
	}

	return 0;

error:
	if( hash_table_inited( &cluster->paral_units_info ) )
	{
		destroy_hash_table( &cluster->paral_units_info, destroy_cluster_value, TRUE );
	}

	if( NULL != cluster->infos_mutex )
	{
		CloseHandle( cluster->infos_mutex );
	}

	if( NULL != cluster->cluster_not_empty )
	{
		CloseHandle( cluster->cluster_not_empty );
	}

	return ret;
}

int connect2paral_unit( paral_unit_info *info )
{
	int ret;
	int cli_socket;

	if( INVALID_SOCKET != info->peer_sock )
	{
		bp_trace( 5, "communicate to computing server ( ip: %s, port: %u )", 
			ip2str( info->ipaddr ), info->port );
		goto __return;
	}

	bp_assert( paral_geted == info->paral_proc );

	ret = connect2server2( &cli_socket, htonl( info->ipaddr ), htons( info->port ) );
	if( 0 > ret )
		return ret;

	info->peer_sock = cli_socket;
	when_paral_unit_connected( info );

__return:

	return 0;
}

int recv_paral_unit_result( paral_unit_info *unit_info )
{
	int ret;
	char recv_buf[ MAX_COMPUTED_RESULT_LEN ];
	dword recved;

	bp_assert( INVALID_SOCKET != unit_info->peer_sock );
	bp_assert( NULL != unit_info->result && 
		0 != unit_info->res_buf_len );
	bp_assert( res_recving == unit_info->paral_proc );

	recved = 0;

	while( 1 )
	{
		ret = readln( unit_info->peer_sock, unit_info->result + unit_info->res_len, unit_info->res_buf_len - unit_info->res_len, recv_buf, &recved, MAX_COMPUTED_RESULT_LEN );
		if( 0 > ret )
		{
			break;
		}
		else if ( 0 == ret )
		{
			when_paral_unit_result_recved( unit_info );
			break;
		}

		unit_info->res_len += ret;
	}

	return ret;
}

int send_paral_unit_command( paral_unit_info *info )
{
	int ret;

	bp_assert( NULL != info );
	bp_assert( cmd_sending == info->paral_proc );

	ret = loop_try_send( info->peer_sock, info->command, info->cmd_len );
	if( 0 > ret )
	{
		close_paral_unit( info );
		return ret;
	}

	when_paral_unit_cmd_sended( info );
	return 0;
}

int dispatch2paral_unit( char *command, paral_unit_info *info )
{
	int ret;
	int cmd_len;

	bp_assert( NULL != command );
	bp_assert( NULL != info );
	bp_assert( INVALID_SOCKET != info->peer_sock );
	bp_assert( cmd_recving == info->paral_proc );

	cmd_len = ( int )strlen( command );

	ret = add_line_to_buf( command, cmd_len, &info->command, &info->cmd_len, &info->cmd_buf_len );
	if( 0 > ret )
	{
		info->cmd_len = 0;
		return ret;
	}

	return ret;
}

int get_results_set( paral_unit_info *unit_info, dword fparam, dword sparam )
{
	int ret;
	param_infos *res_infos;

	bp_assert( NULL != unit_info );
	bp_assert( NULL != fparam );
	res_infos = ( param_infos* )fparam;

	ret = add_param_infos( unit_info->result, res_infos );
	if( 0 > ret )
	{
		bp_assert( FALSE );
	}

	return ret;
}

int send_cmd_and_params( paral_unit_info *unit_info, dword fparam, dword sparam )
{
	int ret;
	bp_assert( NULL != unit_info );
	bp_assert( NULL != unit_info->ipaddr &&
		NULL != unit_info->port );
	bp_assert( NULL != unit_info->command &&
		0 < unit_info->cmd_len );

	if( INVALID_SOCKET == unit_info->peer_sock )
	{
		return COMPUTING_SERVER_DISCONN;
	}

	ret = loop_try_send( unit_info->peer_sock, unit_info->command, unit_info->cmd_len );

	if( ( dword )ret < unit_info->cmd_len )
	{
		close_paral_unit( unit_info );
		bp_trace( 1, "send line to computing server failed %u\n", WSAGetLastError() );
	}
	else
	{
		when_paral_unit_cmd_sended( unit_info );
	}

	return 0;
}

int recv_cmd_result( paral_unit_info *unit_info, dword fparam, dword sparam )
{
	int ret;
	if( INVALID_SOCKET == unit_info->peer_sock )
	{
		return COMPUTING_SERVER_DISCONN;
	}

	ret = recv_paral_unit_result( unit_info );
	if( 0 > ret )
	{
		bp_trace( 1, "get result from computing server failed\n ip: %s, port: %u \n",
			ip2str( unit_info->ipaddr ), unit_info->port );

		close_paral_unit( unit_info );
	}

	return 0;
}

/* matrix command format:
	MATRIX:handler_name:function_name\r\nparams\r\nparams\r\n...\r\n\r\n
/* pipeline command format:
	PIPELINE:handler_name\r\nfunction_name:params\r\nfunction_name:params\r\n...\r\n\r\n
*/

int dispatch_cmd_begin( paral_unit_info *unit_info, dword fparam, dword sparam )
{
	int ret;
	cluster_and_free_units *cluster_and_units;
	char *command;

	bp_assert( NULL != unit_info );
	cluster_and_units = ( cluster_and_free_units* )( void* )fparam;
	bp_assert( NULL != cluster_and_units->cluster );
	bp_assert( NULL != cluster_and_units->units );
	
	command = ( char* )( void* )sparam;
	bp_assert( NULL != command );

	ret = connect2paral_unit( unit_info );
	if( ret < 0 )
	{
		bp_trace( 1, "connect to computing server ( ip:%s, port:%u ) failed %u\n", 
			ip2str( unit_info->ipaddr ), 
			unit_info->port, WSAGetLastError() );

		del_list_element( cluster_and_units->units, ( list_element )( void* )unit_info );
		remove_paral_unit( cluster_and_units->cluster, unit_info, TRUE );
		return ret;
	}

	bp_assert( INVALID_SOCKET != unit_info->peer_sock );

	ret = dispatch2paral_unit( command, unit_info );
	if( 0 > ret )
	{
		bp_trace( 1, "dispatch command to computing unit ( ip: %s, port: %u ) failed\n", 
			ip2str( unit_info->ipaddr ), unit_info->port );

		close_paral_unit( unit_info );
		return ret;
	}
	return 0;
}

int dispatch_cmd_end_term( paral_unit_info *info, dword fparam, dword sparam )
{
	int ret;

	if( INVALID_SOCKET == info->peer_sock )
	{
		return COMPUTING_SERVER_DISCONN;
	}

	bp_assert( cmd_recving == info->paral_proc );

	ret = realloc_buf_needed( &info->command, &info->cmd_buf_len, info->cmd_len + CONST_STR_LEN( COMMAND_ELEMENT_DELIM ) );
	if( 0 > ret )
	{
		close_paral_unit( info );
		return ret;
	}
	memcpy( info->command + info->cmd_len, COMMAND_ELEMENT_DELIM, CONST_STR_LEN( COMMAND_ELEMENT_DELIM ) );
	info->cmd_len += CONST_STR_LEN( COMMAND_ELEMENT_DELIM );

	when_paral_unit_command_recved( info );
	return 0;
}

void remove_paral_unit( cluster_info *cluster, paral_unit_info *unit, int self_free )
{
	int ret;
	int hash_ret;
	hash_key key;

	key = make_hash_key( unit->ipaddr, unit->port );
	require_mutex( cluster->infos_mutex );
	hash_ret = del_hash_item( &cluster->paral_units_info, key, NULL );
	bp_assert( 0 == hash_ret );
	ret = hash_is_empty( &cluster->paral_units_info );
	release_mutex( cluster->infos_mutex );
	if( 0 == ret )
	{
		ret = ResetEvent( cluster->cluster_not_empty );
		bp_assert( TRUE == ret );
	}
	release_paral_unit_info( unit, self_free );
}

/*
 * be suare the free_units list only one thread own,too.
 */
int action_on_free_units( free_paral_units *units, dword fparam, dword sparam, action_on_paral_unit action )
{
	int ret;
	paral_unit_info *unit_info;
	uint32 success_num;
	dlist *item;
	dlist *next_item;

	success_num = 0;
	
	item = units->free_units;

	for( ; ; )
	{
		item = get_next_list_element( item, &( ( list_element )unit_info ) );

		if( NULL == item )
			break;

		unit_info = ( paral_unit_info* )item->info;

		bp_assert( NULL != unit_info );
		bp_assert( NULL != unit_info->ipaddr &&
			NULL != unit_info->port );

		ret = action( unit_info, fparam, sparam );
		if( 0 > ret )
		{
			bp_trace( 1, "do action on computing server ( ip: %s, port: %u )failed\n", 
				ip2str( unit_info->ipaddr ), 
				unit_info->port );
		}
		else
		{
			success_num ++;
		}
	}

	return success_num;
}

int action_on_cluster( cluster_info *infos, dword fparam, dword sparam, action_on_paral_unit action )
{
	int ret;
	paral_unit_info *unit_info;
	void *item_pos;
	hash_table *table;
	unsigned int success_num;

	table = &infos->paral_units_info;

	success_num = 0;

	item_pos = NULL;
	require_mutex( infos->infos_mutex );
	item_pos = get_next_item_value( item_pos, table, &( ( hash_value )unit_info ) );
	if( NULL == item_pos )
	{
		bp_trace( 3, "no computing server exist when do action with them\n" );
		release_mutex( infos->infos_mutex );
		return 0;
	}
	
	for( ; ; )
	{
		if( NULL == item_pos )
			break;
		
		bp_assert( NULL != unit_info );
		bp_assert( NULL != unit_info->ipaddr &&
			NULL != unit_info->port );

		ret = action( unit_info, fparam, sparam );
		if( 0 > ret )
		{
			bp_trace( 1, "do action on computing server ( ip: %s, port: %u )\n", 
				ip2str( unit_info->ipaddr ), 
				unit_info->port );
		}
		else
		{
			success_num ++;
		}

		item_pos = get_next_item_value( item_pos, table, &( ( hash_value )unit_info ) );
	}

	release_mutex( infos->infos_mutex );
	return success_num;
}

void release_computing_info( paral_unit_info *info )
{
	if( INVALID_SOCKET != info->peer_sock )
	{
		closesocket( info->peer_sock );
		info->peer_sock = INVALID_SOCKET;
	}
}

int dispatch_cmd_end_sign2free_units( free_paral_units *infos )
{
	return action_on_free_units( infos, ( dword )NULL, 0, dispatch_cmd_end_term );
}

int dispatch_cmd_begin2free_units( cluster_info *cluster, free_paral_units *units, char *command )
{
	cluster_and_free_units cluster_and_units;
	cluster_and_units.cluster = cluster;
	cluster_and_units.units = units;

	return action_on_free_units( units, ( dword )&cluster_and_units, ( dword )command, dispatch_cmd_begin );
}

int recv_result_from_free_units( free_paral_units *units )
{
	return action_on_free_units( units, ( dword )NULL, 0, recv_cmd_result );
}

int get_results_set_from_free_units( free_paral_units *units, param_infos *res_infos )
{
	return action_on_free_units( units, ( dword )( void* )res_infos, 0, get_results_set );
}

int send_command2free_units( free_paral_units *units )
{
	return action_on_free_units( units, ( dword )NULL, 0, send_cmd_and_params );
}

int free_all_free_units_holding( free_paral_units *units )
{
	return action_on_free_units( units, ( dword )NULL, 0, release_free_unit_holding );
}

void release_free_units( free_paral_units *units )
{
	free_all_free_units_holding( units );
	free( units );
}

int dist_cmd2paral_units( paral_command *command, cluster_info *infos, free_paral_units *units )
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

int add_paral_result( paral_unit_info *info, char *line )
{
	int ret;
	bp_assert( NULL != info );

	ret = realloc_buf_needed( &info->result, &info->res_buf_len, ( dword )strlen( line ) + info->res_len );
	if( 0 > ret )
		return ret;

	strcpy_s( info->result + info->res_len, info->res_buf_len - info->res_len, line );
	info->res_len = ( dword )strlen( line );
	memcpy( info->result + info->res_len, COMMAND_ELEMENT_DELIM, CONST_STR_LEN( COMMAND_ELEMENT_DELIM ) );
	info->res_len += CONST_STR_LEN( COMMAND_ELEMENT_DELIM );
	return 0;
}

void close_paral_unit( paral_unit_info *info )
{
	if( INVALID_SOCKET != info->peer_sock )
	{
		closesocket( info->peer_sock );
		info->peer_sock = INVALID_SOCKET;
	}
	bp_trace( 5, "close this paral unit, ( ip: %s, port: %u that current process is %u\n", 
		ip2str( info->ipaddr ),
		info->port,
		info->paral_proc );

	when_paral_unit_closed( info );
}

int dispatch_params2free_units_linear( free_paral_units *units, char *params, dword params_len )
{
	int ret;
	char *next_param;
	char param[ MAX_ONE_PARAM_LEN ];
	dword remain_len;
	paral_unit_info *unit_info;
	int dispatched;
	dlist *item;

	item = get_next_list_element( units->free_units, &( ( list_element )unit_info ) );
	
	bp_assert( NULL != item );

	dispatched = 0;
	next_param = params;
	remain_len = params_len;
	for( ; ; )
	{
		next_param = get_next_param( param, MAX_ONE_PARAM_LEN, next_param, &remain_len );
		if( NULL == next_param )
		{
			break;
		}
		else if( ( char* )-1 == next_param )
		{
			bp_trace( 1, "the recved data info from server is fault format\n" );
		}

		for( ; ; )
		{
			if( NULL == item )
			{
				bp_trace( 3, "enum all parallel computing server again\n" );

				item = get_next_list_element( units->free_units, &( ( list_element )unit_info ) );
				bp_assert( NULL != item );

				bp_assert( NULL != unit_info );

				for( ; ; )
				{
					if( INVALID_SOCKET == unit_info->peer_sock )
					{
						item = get_next_list_element( item, &( ( list_element )unit_info ) );
						if( NULL == item )
						{
							return -1;
						}
					}
					else
					{
						break;
					}
				}
			}


			bp_assert( NULL != unit_info );
			bp_assert( 0 != unit_info->ipaddr &&
				0 != unit_info->port );

			if( INVALID_SOCKET == unit_info->peer_sock )
			{
				item = get_next_list_element( item, &( ( list_element )unit_info ) );
				continue;
			}

			ret = dispatch2paral_unit( param, unit_info );
			if( 0 > ret )
			{
				bp_trace( 1, "dispatch command to computing unit ( ip: %s, port: %u ) failed\n", 
					ip2str( unit_info->ipaddr ), unit_info->port );

				close_paral_unit( unit_info );
				item = get_next_list_element( item, &( ( list_element )unit_info ) );
				continue;
			}
			else
			{
				dispatched ++;
				item = get_next_list_element( item, &( ( list_element )unit_info ) );
				break;
			}
		}
	}
	
	return dispatched;
}
