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

void destroy_paral_command( paral_command *command )
{
	if( NULL != command->command )
	{
		free( command->command );
	}

	if( NULL != command->params )
	{
		free( command->params );
	}

	free( command );
}

int create_new_command( paral_command **command )
{
	paral_command *new_paral_command;

	bp_assert( NULL != command );
	
	new_paral_command = ( paral_command* )malloc( sizeof( *new_paral_command ) );
	if( NULL == new_paral_command )
	{
		*command = NULL;
		return -E_OUTOFMEMORY;
	}

	new_paral_command->command = ( char* )malloc( DEF_CMD_LEN );
	new_paral_command->cmd_cur_len = 0;
	new_paral_command->cmd_max_len = DEF_CMD_LEN;

	new_paral_command->params = ( char* )malloc( DEF_PARAMS_LEN );
	new_paral_command->param_cur_len = 0;
	new_paral_command->param_max_len = DEF_PARAMS_LEN;

	*command = new_paral_command;
	return 0;
}

int add_command( paral_command *command, char *data )
{
	int ret;
	int command_len;
	bp_assert( NULL != command );
	bp_assert( NULL != data );

	command_len = ( int )strlen( data );
	ret = realloc_buf_needed( &command->command, &command->cmd_max_len, command->cmd_cur_len + command_len + LN_TERM_SIGN_LEN );
	if( 0 > ret )
	{
		return ret;
	}

	memcpy( command->command + command->cmd_cur_len, data, command_len );
	command->cmd_cur_len += command_len;
	memcpy( command->command + command->cmd_cur_len, LN_TERM_SIGN, LN_TERM_SIGN_LEN );
	command->cmd_cur_len += LN_TERM_SIGN_LEN;
	return 0;
}

int add_params( paral_command *command, char *data )
{
	int ret;
	int params_len;
	bp_assert( NULL != command );
	bp_assert( NULL != data );

	params_len = ( int )strlen( data );
	ret = realloc_buf_needed( &command->params, &command->param_max_len, command->param_cur_len + params_len + LN_TERM_SIGN_LEN );
	if( 0 > ret )
	{
		return ret;
	}

	memcpy( command->params + command->param_cur_len, data, params_len );
	command->param_cur_len += params_len;
	memcpy( command->params + command->param_cur_len, LN_TERM_SIGN, LN_TERM_SIGN_LEN );
	command->param_cur_len += LN_TERM_SIGN_LEN;
	return 0;
}

int add_command_term( paral_command *command )
{
	int ret;
	bp_assert( NULL != command );

	ret = realloc_buf_needed( &command->command, &command->cmd_max_len, command->cmd_cur_len + LN_TERM_SIGN_LEN );
	if( 0 > ret )
	{
		return ret;
	}

	memcpy( command->command + command->cmd_cur_len, LN_TERM_SIGN, LN_TERM_SIGN_LEN );
	command->cmd_cur_len += LN_TERM_SIGN_LEN;
	return 0;
}

int add_param_term( paral_command *command )
{
	int ret;
	bp_assert( NULL != command );

	ret = realloc_buf_needed( &command->params, &command->param_max_len, command->param_cur_len + LN_TERM_SIGN_LEN );
	if( 0 > ret )
	{
		return ret;
	}

	memcpy( command->params + command->param_cur_len, LN_TERM_SIGN, LN_TERM_SIGN_LEN );
	command->param_cur_len += LN_TERM_SIGN_LEN;
	return 0;
}

char *get_next_param( char *param, dword param_buf_len, char *params, dword *remain_len )
{
	int ret;
	dword __remain_len;

	__remain_len = *remain_len;
	ret = readln_from_buf( param, param_buf_len, params, &__remain_len, FALSE );
	if( 0 > ret )
	{
		bp_trace( 3, "the params format is not correct\n" );
		return ( char* )-1;
	}
	else if( 0 == ret )
	{
		return NULL;
	}
	else
	{
		__remain_len -= ret;
		*remain_len = __remain_len;
		return params + ret;
	}
}

