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
 
#ifndef __BALANCE_PARALLEL_CONTROLLER_H__
#define __BALANCE_PARALLEL_CONTROLLER_H__

#include "dlist.h"
#include "core.h"
#include "iocompport.h"
#include "hash.h"
#include "broad_search.h"
#include "computing_unit_info.h"

enum dist_servver2cli_action
{
	cli_none_exist,
	cli_connected,
	cli_request_geted
};

typedef struct __dist_server_ext
{
	brd_search_context searcher;
	brd_response_context responsor;
	cluster_info server_infos;
} dist_server_ext;

int add_test_data( dist_server_ext *ext );
int close_dis_server( void *context );
int dist_server2paral_unit_proc( io_info *info );
int dist_server2cli_proc( io_info *info );
int computing_final_result_linear( paral_command *command, free_paral_units *units, param_info *final_res );
int recv_computed_results( paral_command *command, free_paral_units *units, param_info *ret_info );
int distribute_command( paral_command *command, cluster_info *infos, free_paral_units *units, param_info *ret_info );
int init_dist_server( void *param );
int start_dist_server( server_work_info **work_info );
int stop_dist_server( server_work_info *work_info );

#endif