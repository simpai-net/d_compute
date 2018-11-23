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
 

#ifndef __BALANCE_PARALLEL_BP_CLIENT_H__
#define __BALANCE_PARALLEL_BP_CLIENT_H__
#include "broad_search.h"

#define ONE_CMD_BUF_LEN ( MAX_ONE_CMD_LEN + MAX_ONE_CALL_PARAMS_LEN )
typedef struct __client_work_info
{
	brd_search_context searcher;
	cluster_info dist_infos;
} client_work_info;

int init_bp_client( client_work_info *client_info );
void stop_bp_client( client_work_info *client_info );
int get_dist_server_info( cluster_info *cluster, paral_unit_info **finded_info );
#endif //__BALANCE_PARALLEL_BP_CLIENT_H__