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
#include "cmd_transmit.h"
#include "io_info.h"

#define MAX_FUNC_NAME_LEN 256
int command_name2combine_name( const char *command_name, char *function_name, unsigned int maxlen )
{
	char command_func_name[ MAX_FUNC_NAME_LEN ];
	command_name2function_name( command_name, command_func_name, 256 );
	strcat_s( command_func_name, MAX_FUNC_NAME_LEN - strlen( command_func_name ), COMBINE_RES_FUNC_NAME_POSTFIX );

	if( strlen( command_func_name ) + 1 > maxlen )
	{
		return -1;
	}

	strcpy( function_name, command_func_name );
	return 0;
}

int command_name2function_name( const char *command_name, char *function_name, unsigned int maxlen )
{
	char *func_name;
	char *end_func_name;

	func_name = strchr( command_name, CMD_FUNC_DELIM );
	func_name += sizeof( char );
	end_func_name = strstr( command_name, COMMAND_ELEMENT_DELIM );

	if( maxlen < ( uint32 )( end_func_name - func_name ) + 1 )
	{
		return -1;
	}

	memcpy( function_name, func_name, end_func_name - func_name );
	function_name[ end_func_name - func_name ] = '\0';
	return 0;
}

int __combine_command_result( dword func, param_info *results, int info_num, param_info *ret_val )
{
	return ( ( bp_paral_func )( void* )func )( results, info_num, ret_val );
}

int combine_command_result( paral_command *command, param_info *infos, int info_num, param_info *ret_val )
{
	int ret;
	HANDLE parserhandle;
	char combine_func_name[ 256 ];
	void *func;

	load_command_parser( command->command, &parserhandle );
	ret = command_name2combine_name( command->command, combine_func_name, 256 );

	if( 0 > ret )
	{
		return ret;
	}
	func = GetProcAddress( parserhandle, combine_func_name );
	if( NULL == func )
	{
		return -1;
	}

	return __combine_command_result( ( dword )func, infos, info_num, ret_val );
}

void destroy_param_info( param_info *info )
{
	bp_assert( NULL != info );
	return;
}

void destroy_param_infos( param_infos *infos )
{
	int i;
	bp_assert( NULL != infos );

	for( i = 0; i < ( int )infos->cur_info_num; i ++ )
	{
		destroy_param_info( &infos->infos[ i ] );
	}

	free( infos );
}

#define PRE_ALLOC_INFOS_NUM 20
int init_param_infos( param_infos *infos )
{
	bp_assert( NULL != infos );
	infos->infos = ( param_info* )malloc( sizeof( param_info ) * PRE_ALLOC_INFOS_NUM );
	if( NULL == infos->infos )
		return -1;

	infos->cur_info_num = 0;
	infos->max_info_num = PRE_ALLOC_INFOS_NUM;
	return 0;
}

int pre_process_param( char *param_type, char *param_val )
{
	_strupr( param_type );
	_strupr( param_val );
	return 0;
}

char* get_param_info( const char *param, unsigned int param_len, param_info *info )
{
	static char param_val[ 2048 ];
	static char param_type[ 128 ];

	char *val_begin;
	char *val_end;

	char *type_begin;
	char *type_end;
	
	char *next_param;

	type_begin = ( char* )param;
	if( PARAM_DELIM == type_begin[ 0 ] )
	{
		type_begin += 1;
	}

	type_end = strchr( type_begin, TYPE_DELIM );
	if( NULL == type_end )
	{
		return ( char* )-1;
	}
	
	val_begin = type_end + 1;
	val_end = strchr( val_begin, PARAM_DELIM );
	if( NULL == val_end )
	{
		val_end = strstr( val_begin, COMMAND_ELEMENT_DELIM );
		if( NULL == val_end )
		{
			val_end = val_begin + strlen( val_begin );
			next_param = NULL;
		}
		else
		{
			next_param = NULL;
		}
	}
	else
	{
		next_param = val_end + sizeof( char );
	}
	
	memcpy( param_val, val_begin, val_end - val_begin );
	param_val[ val_end - val_begin ] = '\0';
	memcpy( param_type, type_begin, type_end - type_begin );
	param_type[ type_end - type_begin ] = '\0';
	
	pre_process_param( param_type, param_val );
	get_param( param_type, param_val, info );

	return next_param;
}

#define PARAMS_LEN_INC_NUM 10
int realloc_params_buf( param_infos *infos, unsigned int info_num )
{
	unsigned int alloc_info_num;
	void *realloc_buf;

	if( infos->max_info_num >= info_num )
	{
		return 0;
	}

	alloc_info_num = ( ( info_num + PARAMS_LEN_INC_NUM - 1 ) / PARAMS_LEN_INC_NUM ) * PARAMS_LEN_INC_NUM;

	if( NULL == infos->infos )
	{
		realloc_buf = malloc( alloc_info_num * sizeof( param_info ) );
	}
	else
	{
		realloc_buf = realloc( infos->infos, alloc_info_num * sizeof( param_info ) );
	}

	if( NULL == realloc_buf )
	{
		return -1;
	}

	infos->infos = ( param_info* ) realloc_buf;
	infos->max_info_num = alloc_info_num;
	return 0;
}

int add_param_infos( const char *params, param_infos *infos )
{
	int ret;
	char *next_param;
	int info_num;

	bp_assert( NULL != params );
	info_num = 0;
	next_param = ( char* )params;

	for( ; ; )
	{
		ret = realloc_params_buf( infos, infos->cur_info_num + info_num + 1 );
		if( 0 > ret )
		{
			bp_assert( FALSE );
			return ret;
		}
		next_param = get_param_info( next_param, infos->cur_info_num + info_num, &infos->infos[ infos->cur_info_num + info_num ] );
		if( ( char* )-1 == next_param )
		{
			break;
		}
		else
		{
			info_num ++;
			if( NULL == next_param )
			{
				break;
			}
		}
	}

	infos->cur_info_num += info_num;
	return info_num;
}

int load_command_parser( char *command_name, HANDLE *handle )
{
	char parser_path[ MAX_PATH ];
	char parser_name[ MAX_PATH ];
	HANDLE parserhandle;
	command_name2module_name( command_name, parser_name, MAX_PATH );
	get_file_path_in_app_path( parser_name, parser_path, MAX_PATH );

	parserhandle = LoadLibraryA( parser_path );

	if( NULL == parserhandle )
	{
		*handle = NULL;
		return -1;
	}
	*handle = parserhandle;
	return 0;
}

int get_param( char *param_type, char *param_val, param_info *info )
{
	if( 0 == strcmp( param_type, PARAM_INT_TYPE ) )
	{
		info->param_type = PARAM_TYPE_VAL( PARAM_INT_TYPE );
		info->param_int = atoi( param_val );
	}
	else if ( 0 == strcmp( param_type, PARAM_UINT_TYPE ) )
	{
		info->param_type = PARAM_TYPE_VAL( PARAM_UINT_TYPE );
		info->param_uint = ( unsigned int )atoi( param_val );
	}
	else if ( 0 == strcmp( param_type, PARAM_FLOAT_TYPE ) )
	{
		info->param_type = PARAM_TYPE_VAL( PARAM_FLOAT_TYPE );
		info->param_float = ( float )atof( param_val );
	}
	else if ( 0 == strcmp( param_type, PARAM_DOUBLE_TYPE ) )
	{
		info->param_type = PARAM_TYPE_VAL( PARAM_DOUBLE_TYPE );
		info->param_double = ( double )atof( param_val );
	}
	else
	{
		bp_assert( FALSE );
		return -1;
	}
	return 0;
}

int command_name2module_name( const char *command_name, char *parser_name, unsigned int maxlen )
{
	char *mod_name_end;
	mod_name_end = strchr( command_name, CMD_FUNC_DELIM );

	if( maxlen < ( uint32 )( mod_name_end - command_name + 1 ) )
	{
		return -1;
	}

	memcpy( parser_name, command_name, mod_name_end - command_name );
	parser_name[ mod_name_end - command_name ] = '\0';
	return 0;
}

void dynamic_call_func( void *func, param_infos *infos )
{
	int ret_val;
	int info_size;
	int param_offset;
	int param_type_offset;
	int info_num;

	info_num = infos->cur_info_num;
	info_size = sizeof( infos->infos[ 0 ] );
	param_offset = ( int32 )&(( ( param_info* )0 )->param_int );
	param_type_offset = ( int32 )&(( ( param_info* )0 )->param_type );

	__asm
	{
		mov			ecx,dword ptr [info_num];
		dec			ecx
push_params:
		mov         eax,dword ptr [infos];
		lea         ebx,dword ptr [eax];
		mov			eax,dword ptr [info_size];
		mul			ecx;
		lea         ebx,dword ptr [ebx + eax];
		mov			eax,dword ptr [param_offset];
		lea			ebx,dword ptr [ebx + eax];
		mov			ebx,dword ptr [ebx];
		push        ebx;
		dec			ecx;
		and			ecx,ecx;
		jnz			push_params;

		mov         eax,dword ptr [infos.infos];
		mov			ebx,dword ptr [info_size];
		lea         ecx,dword ptr [eax];
		mov			ebx,dword ptr [param_offset];
		lea			ecx,dword ptr [ecx + ebx];
		mov			ecx,dword ptr [ecx];
		push        ecx; 
		call        dword ptr [func]; 
		mov         dword ptr [ret_val],eax;
		mov			eax,dword ptr[info_size];
		add         esp,8;

	}
}

int execute_command( paral_command *command, param_info *ret_val )
{
	int ret;
	HANDLE parserhandle;
	char command_func_name[ 256 ];
	char combine_func_name[ 256 ];
	char one_command_params[ 2046];
	char *remain_params;
	int exec_count;
	param_infos infos;
	param_infos res_infos;
	param_info res_info;
	bp_paral_func func;
	bp_paral_func res_func;

	remain_params = command->params;

	ret = load_command_parser( command->command, &parserhandle );
	if( 0 > ret )
	{
		return ret;
	}

	ret = command_name2function_name( command->command, command_func_name, 256 );
	if( 0 > ret )
	{
		return ret;
	}
	
	ret = command_name2combine_name( command->command, combine_func_name, 256 );
	if( 0 > ret )
	{
		return ret;
	}

	res_func = ( bp_paral_func )GetProcAddress( parserhandle, combine_func_name );
	if( NULL == res_func )
	{
		return -1;
	}

	func = ( bp_paral_func )GetProcAddress( parserhandle, command_func_name );
	if( NULL == res_func )
	{
		return -1;
	}

	exec_count = 0;

	ret = init_param_infos( &infos );
	if( 0 > ret )
	{
		return ret;
	}

	ret = init_param_infos( &res_infos );
	if( 0 > ret )
	{
		return ret;
	}

#define INFO_SPACE_LEN 12
	while( 1 )
	{
		ret = readln_from_buf( one_command_params, 2046, remain_params, &command->param_cur_len, FALSE );

		if( 0 > ret )
		{
			return -exec_count;
		}

		if( 0 == ret )
		{
			break;;
		}

		remain_params += ret;
		ret = add_param_infos( one_command_params, &infos );
		if( 0 >= ret )
		{
			bp_trace( 2, "add param info failed: %s\n", one_command_params );
			return ret;
		}

		ret = func( infos.infos, infos.cur_info_num, &res_info );
		if( 0 > ret )
		{
			return -exec_count;
		}

		exec_count ++;

		realloc_params_buf( &res_infos, exec_count );
		memcpy( &res_infos.infos[ exec_count - 1], &res_info, sizeof( res_info ) );
		res_infos.cur_info_num ++;
	}

	res_func( res_infos.infos, res_infos.cur_info_num, &res_info );
	*ret_val = res_info;
	return exec_count;
}
