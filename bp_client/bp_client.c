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
#include "bp_config.h"
#include "network.h"
#include "controller.h"
#include "bp_client.h"
#include "hash.h"
#include "computing_unit_info.h"

int init_bp_client( client_work_info *client_info )
{
	int ret;
	bp_assert( NULL != client_info );

	ret = init_cluster_info( &client_info->dist_infos );
	if( 0 > ret )
	{
		return ret;
	}

	client_info->searcher.local_ipaddr = NULL;
	client_info->searcher.local_port = BP_CLIENT_BROADCAST_PORT;
	client_info->searcher.search_thread = NULL;
	client_info->searcher.search_thread_id = 0;
	client_info->searcher.onsearched = on_paral_unit_searched;
	client_info->searcher.stop_notifier = NULL;
	client_info->searcher.remote_ipaddr = BROADCAST_IP_ADDRESS;
	client_info->searcher.remote_port = BP_DIST_INFO_RECV_PORT;
	client_info->searcher.brd_sock = INVALID_SOCKET;
	client_info->searcher.context = ( void * )&client_info->dist_infos;

	ret = init_broad_search_thread( ( void* )&client_info->searcher );
	if( 0 > ret )
	{
		release_paral_unit_infos( &client_info->dist_infos, TRUE );
	}
	return ret;
}

void stop_bp_client( client_work_info *client_info )
{
	stop_broad_search( client_info );
}

int get_dist_server_info( cluster_info *cluster, paral_unit_info **finded_info )
{
	int ret;
	hash_table *table;
	paral_unit_info *unit_info;
	void *item_pos;
	bp_assert( NULL != cluster && NULL != finded_info );
	bp_assert( NULL != cluster->cluster_not_empty );

	table = &cluster->paral_units_info;
	
	ret = wait_event_time( cluster->cluster_not_empty, 6000 );
	if( 0 > ret )
	{
		return ret;
	}

	item_pos = NULL;

	require_mutex( cluster->infos_mutex );
	item_pos = get_next_item_value( item_pos, table, &( ( hash_value )unit_info ) );
	
	for( ; ; )
	{
		if( item_pos == NULL )
		{
			release_mutex( cluster->infos_mutex );
			return -1;
		}

		ret = hold_paral_unit( unit_info, finded_info );
		if( 0 == ret )
		{
			release_mutex( cluster->infos_mutex );
			return 0;
		}

		item_pos = get_next_item_value( item_pos, table, &( ( hash_value )unit_info ) );
	}

	release_mutex( cluster->infos_mutex );
	return 0;
}



