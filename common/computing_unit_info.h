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

#ifndef __BALANCE_PARALLEL_COMPUTING_UNIT_INFO_H__
#define __BALANCE_PARALLEL_COMPUTING_UNIT_INFO_H__
#include "broad_search.h"

#define MAX_ONE_CMD_LEN 256
#define PARAL_UNIT_HASH_NUM 256
#define MAX_COMPUTED_RESULT_LEN 4096
#define MAX_ONE_PARAM_LEN 512
#define COMPUTING_SERVER_DISCONN -11
#define DEF_PARAL_UNIT_CMD_LEN ( DEF_CMD_LEN + DEF_PARAMS_LEN )

#define FLUSH_INPUT 0x01
#define FLUSH_OUTPUT 0x02

typedef enum __paral_cmd_proc
{
	paral_inited,
	paral_geted,
	cmd_recving,
	cmd_sending,
	res_recving,
} paral_cmd_proc;

#define unit_allocated 2

typedef struct __paral_unit_info
{
	dword ipaddr;
	uint16 port;
	int peer_sock;

	char *command;
	dword cmd_len;
	dword cmd_buf_len;

	char *result;
	dword res_len;
	dword res_buf_len;

	int paral_proc;

	dword comp_order;

	struct __paral_unit_info *next_unit;
} paral_unit_info;

typedef struct __cluster_info
{
	hash_table paral_units_info;
	HANDLE infos_mutex;
	HANDLE cluster_not_empty;
} cluster_info;

typedef struct __free_paral_units
{
	dlist *free_units;
} free_paral_units;

typedef struct __cluster_and_free_units
{
	free_paral_units *units;
	cluster_info *cluster;
} cluster_and_free_units;

typedef int ( *action_on_paral_unit )( paral_unit_info *info, dword fparam, dword sparam );
typedef int ( *dispatch_params_func )( cluster_info *infos, char *params, dword params_len );
typedef int ( *computing_final_result_func )( paral_command *command, cluster_info *cluster, param_info *final_res );

void when_paral_unit_geted( paral_unit_info *unit_info );
void when_paral_unit_hold( paral_unit_info *unit_info );
void when_paral_unit_connected( paral_unit_info *unit_info );
void when_paral_unit_command_recved( paral_unit_info *unit_info );
void when_paral_unit_cmd_sended( paral_unit_info *unit_info );
void when_paral_unit_result_recved( paral_unit_info *unit_info );
void when_paral_unit_closed( paral_unit_info *unit_info );
void when_paral_unit_holding_release( paral_unit_info *unit_info );
int hold_paral_unit( paral_unit_info *unit_info, paral_unit_info **holded );
void release_paral_unit_holding( paral_unit_info *unit_info );
void when_free_units_communication_done( free_paral_units *units );
int init_free_paral_units( free_paral_units *free_units );
int create_new_free_paral_units( free_paral_units **free_units );
void release_free_paral_units( free_paral_units *units );
int get_free_paral_unit( paral_unit_info *unit_info, dword fparam, dword sparam );
int get_free_paral_units( cluster_info *cluster, free_paral_units *free_units );
int release_free_unit_holding( paral_unit_info *unit_info, dword fparam, dword sparam );
void flush_paral_unit_buf( paral_unit_info *unit_info, dword flag );
int on_paral_unit_searched( responsor_info *responsor, void *context );
void destroy_cluster_value( hash_value *value, dword param );
void release_paral_unit_infos( cluster_info *cluster, int self_free );
int create_new_paral_unit_info( paral_unit_info **info );
void release_paral_unit_info( paral_unit_info *info, int self_free );
int init_cluster_info( cluster_info *cluster );
int connect2paral_unit( paral_unit_info *info );
int recv_paral_unit_result( paral_unit_info *unit_info );
int send_paral_unit_command( paral_unit_info *info );
int dispatch2paral_unit( char *command, paral_unit_info *info );
int get_results_set( paral_unit_info *unit_info, dword fparam, dword sparam );
int send_cmd_and_params( paral_unit_info *unit_info, dword fparam, dword sparam );
int recv_cmd_result( paral_unit_info *unit_info, dword fparam, dword sparam );
int dispatch_cmd_begin( paral_unit_info *unit_info, dword fparam, dword sparam );
int dispatch_cmd_end_term( paral_unit_info *info, dword fparam, dword sparam );
void remove_paral_unit( cluster_info *cluster, paral_unit_info *unit, int self_free );
int action_on_free_units( free_paral_units *units, dword fparam, dword sparam, action_on_paral_unit action );
int action_on_cluster( cluster_info *infos, dword fparam, dword sparam, action_on_paral_unit action );
void release_computing_info( paral_unit_info *info );
int dispatch_cmd_end_sign2free_units( free_paral_units *infos );
int dispatch_cmd_begin2free_units( cluster_info *cluster, free_paral_units *units, char *command );
int recv_result_from_free_units( free_paral_units *units );
int get_results_set_from_free_units( free_paral_units *units, param_infos *res_infos );
int send_command2free_units( free_paral_units *units );
int free_all_free_units_holding( free_paral_units *units );
void release_free_units( free_paral_units *units );
int dist_cmd2paral_units( paral_command *command, cluster_info *infos, free_paral_units *units );
int add_paral_result( paral_unit_info *info, char *line );
void close_paral_unit( paral_unit_info *info );
int dispatch_params2free_units_linear( free_paral_units *units, char *params, dword params_len );

#endif //__BALANCE_PARALLEL_COMPUTING_UNIT_INFO_H__