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
#include "network.h"

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"mswsock.lib")

int readln( int socket, char *line, int ln_max_len, char *recvedbuf, unsigned long *buf_begin, unsigned long max_buf_len)
{
	char *ln_term;	
	int ret;
	int end_ln_index;

	unsigned long find_size = 0;
	int term_pos = -1;

	unsigned long readed_len = *buf_begin;
	unsigned long max_ln_len = max_buf_len;

	bp_assert( socket != -1 && buf_begin != NULL);
	bp_assert(max_buf_len != 0);

	ln_term = LN_TERM_SIGN;

	do
	{
		if(readed_len > 0)
		{
			term_pos = memsubstr((char*)ln_term, LN_TERM_SIGN_LEN, (BYTE*)recvedbuf + find_size, readed_len - find_size );
			if(term_pos >= 0)
			{
				term_pos += find_size;
			}

			if(recvedbuf[readed_len - 1] == ln_term[0])
				find_size = readed_len - 1;
			else
				find_size = readed_len;

		}

		if(max_buf_len <= 0 || term_pos < 0)
		{
			if((max_ln_len - readed_len) > 0 )
			{
				
				ret = recv(socket, recvedbuf + readed_len, max_ln_len - readed_len, 0);
				if(ret > 0)
				{
					readed_len += ret;
				}
				else
				{
					*line = 0;
					return ret - 1;
				}
			}
			else
			{
				*line = 0;
				return -1;
			}
		}
	}while(term_pos < 0);

	memcpy(line, recvedbuf, term_pos);
	line[term_pos] = 0;

	end_ln_index = term_pos + LN_TERM_SIGN_LEN;
	readed_len -= end_ln_index;
	memcpy(recvedbuf, recvedbuf + end_ln_index, readed_len);
	*buf_begin= readed_len;
	
	return term_pos;
}

int init_win32sock_lib()
{
	WSADATA wsadata;
	WSAStartup( MAKEWORD( 0x02, 0x02 ), &wsadata );
	
	if( wsadata.wVersion < 0x02 || wsadata.wHighVersion < 0x02 )
		return -1;

	bp_trace( 5, "winsock lib description: %s\n", wsadata.szDescription );
	bp_trace( 5, "winsock lib system status: %s\n", wsadata.szSystemStatus );
	bp_trace( 5, "winsock lib max suport socket number is: %d\n", wsadata.iMaxSockets );
	bp_trace( 5, "winsock lib max udp dg is: %d\n", wsadata.iMaxUdpDg );
	//bp_trace( 5, "winsock lib version description: %s\n", wsadata.lpVendorInfo );
	return 0;
}

void close_win32sock_lib()
{
	WSACleanup();
}

void  __construct_inet_addr( dword ipaddr, uint16 port, struct sockaddr *addr, int addr_len )
{
	struct sockaddr_in *sockaddr;

	bp_assert( NULL != addr );
	bp_assert( sizeof( struct sockaddr_in ) == addr_len );

	sockaddr = ( struct sockaddr_in* )addr;
	memset( sockaddr, 0, sizeof( *sockaddr ) );

	sockaddr->sin_addr.s_addr = ipaddr;
	sockaddr->sin_port = port;
	sockaddr->sin_family = AF_INET;
}

dword inet_addr_conv( char *ipaddr )
{
	dword addr;
	if( NULL == ipaddr || '\0' == *ipaddr )
	{
		addr = INADDR_ANY;
	}
	else
	{
		addr = inet_addr( ipaddr );
	}

	return addr;
}

char* ip2str( dword ipaddr )
{
	struct in_addr addr = { ipaddr };
	return inet_ntoa( addr );
}

void  construct_inet_addr( char* local_ip, uint16 port, struct sockaddr *addr, int addr_len )
{
	dword __ipaddr;
	__ipaddr = inet_addr_conv( local_ip );
	__construct_inet_addr( __ipaddr, htons( port ), addr, addr_len );
}

#define RECONNECT_WAIT_TIME 200

int connect2server( int *sock, char *ipaddr, int16 port )
{
	dword addr;
	addr = inet_addr_conv( ipaddr );
	return connect2server2( sock, addr, htons( port ) );
}

int connect2server2( int *sock, dword ipaddr, uint16 port )
{
	int ret;
	dword err_code;
	int new_sock;
	int try_conn_time = 0;
	struct sockaddr_in addr;

	new_sock = socket( AF_INET, SOCK_STREAM, 0 );
	
	if( INVALID_SOCKET == new_sock )
	{
		*sock = INVALID_SOCKET;
		return -1;
	}

	__construct_inet_addr( ipaddr, port, ( struct sockaddr* )&addr, sizeof( addr ) );

	while( 1 )
	{
		ret = connect( new_sock, ( struct sockaddr* )&addr, sizeof( addr ) );

		if( 0 > ret )
		{
			if( try_conn_time < 5 )
			{
				err_code = GetLastError();
				bp_trace( 1, "connect to server failed error code is %u\n", err_code );
				Sleep( RECONNECT_WAIT_TIME );
				try_conn_time ++;
			}
			else
			{
				break;
			}
		}
		else
		{
			break;
		}
	}

	*sock = new_sock;
	return ret;
}

int sendln( int sock, char *buf, int buflen )
{
	int ret;

	if( 0 < buflen )
	{
		ret = loop_try_send( sock, buf, buflen );
		if( ret < buflen )
			return -1;
	}

	ret = loop_try_send( sock, COMMAND_ELEMENT_DELIM, CONST_STR_LEN( COMMAND_ELEMENT_DELIM ) );
	if( ret < CONST_STR_LEN( COMMAND_ELEMENT_DELIM ) )
		return -1;

	return 0;
}

int loop_try_send( int socket, char *buf, int buflen )
{
	int sended;
	int allsend;
	int ret;

	bp_assert( INVALID_SOCKET != socket );
	bp_assert( 0 < buflen );

	allsend = buflen;
	sended = 0;
	
	while( 1 )
	{
		if( sended == allsend )
			break;

		ret = send( socket, buf + sended, allsend - sended, 0 );
		if( ret <= 0 )
		{
			return sended;
		}

		sended += ret;
	}

	return sended;
}

int loop_try_recv( int socket, char *buf, int buflen )
{
	int recved;
	int allrecv;
	int ret;

	bp_assert( INVALID_SOCKET != socket );
	bp_assert( 0 < buflen );

	allrecv = buflen;
	recved = 0;
	
	while( 1 )
	{
		if( recved == allrecv )
			break;

		ret = recv( socket, buf + recved, allrecv - recved, 0 );
		if( ret <= 0 )
		{
			return recved;
		}

		recved += ret;
	}

	return recved;
}

int check_addr_sanity( char *ipaddr, unsigned short port )
{
	return 0;
}

int check_addr_sanity2( dword ipaddr, unsigned short port )
{
	return 0;
}

int generate_tcp_listen_sock( int *sock, char *ipaddr, uint16 port, int flag )
{
	int ret = 0;
	dword __flag;
	int __listen_sock;
	struct sockaddr_in listenaddr;

	if( TRUE == flag )
	{
		__flag = WSA_FLAG_OVERLAPPED;
	}
	else
	{
		__flag = 0;
	}

	__listen_sock = WSASocket( AF_INET,SOCK_STREAM, 0, NULL, 0, __flag );

	if( INVALID_SOCKET == __listen_sock )		//·ÖÅäÊ§°Ü
	{
		bp_trace( 0, "Create Socket faild with Error: %d", GetLastError() );
		goto error;
	}

	construct_inet_addr( ipaddr, port, ( struct sockaddr* )&listenaddr, sizeof( listenaddr ) );

	ret = bind( __listen_sock,
		( struct sockaddr* )&listenaddr,
		sizeof( listenaddr ) );

	if( 0 > ret )
	{
		bp_trace( 0, "bind Socket faild with Error: %d", GetLastError() );
		goto error;
	}

#define LISTEN_QUEUE_LEN 512
	ret = listen( __listen_sock, LISTEN_QUEUE_LEN );
	if( 0 > ret )
	{
		bp_trace( 0, "listen Socket faild with Error: %d", GetLastError() );
		goto error;
	}

	*sock = __listen_sock;
	return 0;

error:
	if( INVALID_SOCKET != __listen_sock )
	{
		closesocket( __listen_sock );
	}

	*sock = INVALID_SOCKET;
	return ret;
}

int gracefully_close( int sock, char *buf, int buf_len )
{
	int ret;
	int recved;
	char *recv_buf;
	int max_len;
	char __buf[ 128 ];

	ret = shutdown( sock, SD_SEND );
	if( 0 > ret )
	{
		closesocket( sock );
		return ret;
	}

	if( NULL != buf )
	{
		bp_assert( 0 < buf_len );
		recv_buf = buf;
		max_len = buf_len;
	}
	else
	{
		recv_buf = __buf;
		max_len = 128;
	}

	recved = 0;
	for( ; ; )
	{
		if( recved == max_len )
		{
			closesocket( sock );
			return -1;
		}

		ret = recv( sock, recv_buf + recved, max_len - recved, 0 );
		if( 0 > ret )
		{
			closesocket( sock );
			return ret;
		}
		else if( 0 == ret )
		{
			closesocket( sock );
			return 0;
		}
		
		recved += ret;
	}
}

int sock_io_ctl( int sock, long cmd, ulong *value )
{
	return ioctlsocket( sock, cmd, value );
}

int sock_io_ctl_ex( int sock, 
				   dword cmd, 
				   void *input, 
				   dword input_len, 
				   void *output,
				   dword output_len
				   )
{
	dword bytes_returned;
	return WSAIoctl( sock, cmd, input, input_len, output, output_len, &bytes_returned, NULL, NULL );
}

int get_broadcast_addr( int sock, struct sockaddr *addr, dword addr_len )
{
	return sock_io_ctl_ex( sock, SIO_GET_BROADCAST_ADDRESS, NULL, 0, ( void* )addr, addr_len );
}
