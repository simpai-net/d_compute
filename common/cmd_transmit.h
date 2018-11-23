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

#ifndef __BALANCE_PARALLEL_H__
#define __BALANCE_PARALLEL_H__

#define CMD_FUNC_DELIM ':'
#define TYPE_DELIM ':'
#define PARAM_DELIM ','
#define COMBINE_RES_FUNC_NAME_POSTFIX "_combine"

#define REMAIN_LEN( begin_p, buflen, cur_p ) ( begin_p + buflen - cur_p )

int command_name2combine_name( const char *command_name, char *function_name, unsigned int maxlen );
int command_name2function_name( const char *command_name, char *function_name, unsigned int maxlen );
int combine_command_result( paral_command *command, param_info *infos, int info_num, param_info *ret_val );
void destroy_param_info( param_info *info );
void destroy_param_infos( param_infos *infos );
int init_param_infos( param_infos *infos );
int pre_process_param( char *param_type, char *param_val );
char* get_param_info( const char *param, unsigned int param_len, param_info *info );
int realloc_params_buf( param_infos *infos, unsigned int info_num );
int add_param_infos( const char *params, param_infos *infos );
int load_command_parser( char *command_name, HANDLE *handle );
int get_param( char *param_type, char *param_val, param_info *info );
int command_name2module_name( const char *command_name, char *parser_name, unsigned int maxlen );
void dynamic_call_func( void *func, param_infos *infos );
int execute_command( paral_command *command, param_info *ret_val );

#endif //__BALANCE_PARALLEL_H__