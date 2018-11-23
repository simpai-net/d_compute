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
 
#ifndef __BALANCE_PARALLEL_CORE_H__
#define __BALANCE_PARALLEL_CORE_H__

#ifdef _DLL_IMPL
#define BP_FUNC __declspec( dllexport )
#else
#define BP_FUNC __declspec( dllimport )
#endif

#define BP_API __stdcall

#define MAX_ONE_CALL_PARAMS_LEN 2048
#define LINE_TERM_SIGN "\r\n"
#define COMMAND_ELEMENT_DELIM LINE_TERM_SIGN
#define NULL_LINE_TERM_SIGN ( LINE_TERM_SIGN LINE_TERM_SIGN )
#define COMMAND_END_TERM NULL_LINE_TERM_SIGN


#define PARAM_INT_TYPE "I"
#define PARAM_UINT_TYPE "U"
#define PARAM_FLOAT_TYPE "F"
#define PARAM_DOUBLE_TYPE "D"
#define PARAM_BUF_TYPE "P"

#define PARAM_TYPE_VAL( type ) type[ 0 ]

typedef struct __param_info
{
	unsigned int param_type;
	union
	{
		int param_int;
		unsigned int param_uint;
		float param_float;
		double param_double;
		struct
		{
			void *buf;
			unsigned long buf_len;
		} param_buf;
	};
} param_info;

typedef struct __param_infos
{
	param_info *infos;
	unsigned long cur_info_num;
	unsigned long max_info_num;

} param_infos;

typedef struct __paral_command
{
	unsigned int cmd_max_len;
	unsigned int cmd_cur_len;
	char *command;

	unsigned int param_max_len;
	unsigned int param_cur_len;
	char *params;
} paral_command;

/********************************************************
	one parallel command constructed by these member
	parallel_command
	{
		char command_name[];
		char command_param1[];
		char command_param2[];
	}

	all member of parallel_command end by "\r\n"
*********************************************************/

typedef int ( BP_API *bp_paral_func )( param_info *infos, int info_num, param_info *ret_info );

#define DEF_CMD_LEN 512
#define DEF_PARAMS_LEN 2048
#define MAX_LINE_LEN 2048

void destroy_paral_command( paral_command *command );
int create_new_command( paral_command **command );
int add_command( paral_command *command, char *data );
int add_params( paral_command *command, char *data );
int add_command_term( paral_command *command );
int add_param_term( paral_command *command );
char *get_next_param( char *param, dword param_buf_len, char *params, dword *remain_len );

#endif