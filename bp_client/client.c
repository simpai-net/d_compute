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
#include "core.h"
#include "common.h"
#include "bp_config.h"
#include "network.h"
#include "controller.h"
#include "bp_client.h"
#include <stdio.h>

#define TEST_COMMAND "math:multiple\r\nI:1286,I:384,U:283\r\n\r\n"

void help()
{
	printf( "balance parallel client usage: \n" );
	printf( "bp_client file\n" );
	printf( "this file store all need to compute command\n" );
}

#define CMD_DELIM_IN_FILE "\\\\"
void prepare_command( const char *command )
{
	int len;
	char *begin;
	char *end;
	char *__command;

	__command = command;
	len = strlen( __command );

	begin = __command;
	for( ; ; )
	{
		end = strstr( begin, CMD_DELIM_IN_FILE );

		if( NULL == end )
		{
			//if( __command[ len - 1 ] != '\n' ||
			//	__command[ len - 2 ] != '\r' )
			//{
			//	strcat( __command, COMMAND_ELEMENT_DELIM );
			//}
			return;
		}
		else
		{
			end[ 0 ] = '\r';
			end[ 1 ] = '\n';
		}
		begin = end + CONST_STR_LEN( CMD_DELIM_IN_FILE );
	}
}

int main(int argc, char* argv[])
{
	int ret;
	int cli_sock;
	client_work_info client_info;
	dword recved;
	paral_unit_info *unit_info;
	int cmd_file;
	dword cmd_file_len;
	char command[ ONE_CMD_BUF_LEN ];
	char bat_file[ MAX_PATH ];
	char *path_sign;
	char user_input;

	if( 2 > argc )
	{
		help();
		return 0;
	}

	path_sign = strrchr( argv[ 1 ], '\\' );
	if( NULL == path_sign )
	{
		ret = get_file_path_in_app_path( argv[ 1 ], bat_file, MAX_PATH );
		if( 0 > ret )
		{
			return 0;
		}
	}
	else
	{
		strcpy_s( bat_file, MAX_PATH, argv[ 1 ] );
	}

	cmd_file = open( bat_file, O_RDONLY );
	if( 0 > cmd_file )
	{
		printf( "this command file can't open\n" );
		return 0;
	}

	lseek( cmd_file, 0, SEEK_SET );
	cmd_file_len = lseek( cmd_file, 0, SEEK_END );
	lseek( cmd_file, 0, SEEK_SET );

	if( ONE_CMD_BUF_LEN < cmd_file_len + CONST_STR_LEN( COMMAND_END_TERM ) )
	{
		printf( "the input command is to long, max long is: %u\n", ONE_CMD_BUF_LEN );
		return 0;
	}

	ret = read( cmd_file, command, cmd_file_len );
	command[ cmd_file_len ] = '\0';
	if( 0 > ret )
	{
		printf( "read the input command in bat file: %s failed\n", bat_file );
		return 0;
	}

	prepare_command( command );

	init_win32sock_lib();
	init_bp_work( 0, 0, NULL );

	ret = init_bp_client( &client_info );
	if( 0 > ret )
	{
		return ret;
	}

	while( 1 )
	{
		ret = get_dist_server_info( &client_info.dist_infos, &unit_info );
		if( 0 > ret )
		{
			bp_trace( 8, "no dist server exist in LAN\n" );
			continue;
		}

		printf( "find one distribute server, that ip is: %s, port is: %u\n", ip2str( unit_info->ipaddr ), unit_info->port );
		bp_trace( 8, "find dist server, it's ip: %s, port: %u\n", ip2str( unit_info->ipaddr ), ( unit_info->port ) );
		
conn_again:
		ret = connect2paral_unit( unit_info );
		if( 0 > ret )
		{
			bp_trace( 8, "connect to dist server falied\n" );
			require_mutex( client_info.dist_infos.infos_mutex );
			remove_paral_unit( &client_info.dist_infos, unit_info, TRUE );
			release_mutex( client_info.dist_infos.infos_mutex );
			continue;
		}
		else
		{
			printf( "distribute server ( ip: %s, port: %u ) connected\n", ip2str( unit_info->ipaddr ), unit_info->port );
			bp_trace( 8, "connect to dist server success\n" );;
		}

send_again:
		ret = dispatch2paral_unit( command, unit_info );
		if( 0 > ret )
		{
			bp_trace( 8, "dipatch command to dist server falied\n" );
			goto error;
		}
		else
		{
			bp_trace( 8, "dipatch command to dist server success\n" );
		}

		ret = dispatch_cmd_end_term( unit_info, NULL, NULL );
		if( 0 > ret )
		{
			bp_trace( 8, "dipatch command to dist server falied\n" );
		}

		ret = send_paral_unit_command( unit_info );
		if( 0 > ret )
		{
			bp_trace( 8, "send command from dist server falied\n" );
			goto error;
		}
		else
		{
			printf( "send command to distribute server( ip: %s, port: %u ) success\n",  ip2str( unit_info->ipaddr ), unit_info->port );
			bp_trace( 8, "send command from dist server success\n");
		}
		
		ret = recv_paral_unit_result( unit_info );
		if( 0 > ret )
		{
			bp_trace( 8, "recv result from dist server falied\n" );
			goto error;
		}
		else
		{
			printf( "computing result is geted: %s\n", unit_info->result );
			bp_trace( 8, "recv result from dist server success : %s\n", unit_info->result );;
		}

		printf( "distribute command agmain ? y\n" );
		user_input = getch();
		if( user_input == 'y' )
		{
			flush_paral_unit_buf( unit_info, FLUSH_INPUT | FLUSH_OUTPUT );
			goto send_again;
		}
		else
		{
			break;
		}
error:
		close_paral_unit( unit_info );
		printf( "distribute command agmain ? y\n" );
		user_input = getch();
		if( user_input == 'y' )
		{
			goto conn_again;
		}
		else
		{
			release_paral_unit_holding( unit_info );
			break;
		}
	}

	return 0;
}
