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
#include "broadcast.h"

void init_brd_comm_unit( broadcast_comm* comm_unit )
{
	bp_assert( NULL != comm_unit );
	memset( comm_unit, 0, sizeof( *comm_unit ) );
}

int __create_udp_socket( int *sock, struct sockaddr_in *local_addr )
{
	int ret;
	int new_sock;

	new_sock = ( int )socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if( INVALID_SOCKET == new_sock )
	{
		goto error;
	}

	ret = bind( new_sock, ( struct sockaddr* )local_addr, sizeof( *local_addr ) );

	if( SOCKET_ERROR == ret )
	{
		bp_trace( GENERAL_WINSOCK_API_ERROR_LEVEL, "bind() error %i",GetLastError() );
		goto error;
	}

	*sock = new_sock;
	return *sock;

error:
	
	if( INVALID_SOCKET != new_sock )
	{
		closesocket( new_sock );
	}

	*sock = INVALID_SOCKET;
	return *sock;
}

int create_udp_socket( int *sock, char *local_ip, uint16 local_port )
{
	struct sockaddr_in local_addr;
	bp_assert( NULL != sock );

	construct_inet_addr( local_ip, local_port, ( struct sockaddr* )&local_addr, sizeof( local_addr ) );
	return __create_udp_socket( sock, &local_addr );
}

int create_brdcast_sock( int *sock, char *ipaddr, uint16 port )
{
	int ret;
	int new_sock;
	char sw_brdcast;

	sw_brdcast = 1;

	ret = create_udp_socket( &new_sock, ipaddr, port );
	
	if( 0 > ret )
	{
		return ret;
	}

	//switch on the broadcast option of this new socket
	ret = setsockopt( new_sock, SOL_SOCKET, SO_BROADCAST, ( char * )&sw_brdcast, sizeof( sw_brdcast ) );

	if ( SOCKET_ERROR == ret )
	{
		bp_trace( GENERAL_WINSOCK_API_ERROR_LEVEL, "setsockopt() error %i",GetLastError() );
		goto error;
	}

	*sock = new_sock;
	return *sock;

error:
	
	if( INVALID_SOCKET != new_sock )
	{
		closesocket( new_sock );
	}

	*sock = INVALID_SOCKET;
	return *sock;
}

int __try_sendto( int socket, struct sockaddr *addr, int addr_len, char *buf, int buflen )
{
	int ret;

	bp_assert( INVALID_SOCKET != socket );


	ret = sendto( socket, 
		buf, 
		buflen, 
		MSG_DONTROUTE, 
		( SOCKADDR * )addr, 
		addr_len );

	if( ret <= 0 )
	{
		return ret - 1;
	}

	return ret;
}

int try_sendto2( int socket, dword ipaddr, int16 port, char *buf, int buflen )
{
	struct sockaddr_in dest_addr;

	bp_assert( INVALID_SOCKET != socket );

	__construct_inet_addr( htonl( ipaddr ), htons( port ), ( struct sockaddr* )&dest_addr, sizeof( dest_addr ) );
	return __try_sendto( socket, ( struct sockaddr* )&dest_addr, sizeof( dest_addr ), buf, buflen );
}

int try_sendto( int socket, char *ipaddr, int16 port, char *buf, int buflen )
{
	struct sockaddr_in dest_addr;

	bp_assert( INVALID_SOCKET != socket );

	construct_inet_addr( ipaddr, port, ( struct sockaddr* )&dest_addr, sizeof( dest_addr ) );
	return __try_sendto( socket, ( struct sockaddr* )&dest_addr, sizeof( dest_addr ), buf, buflen );
}

void get_inet_addr_info( struct sockaddr_in *addr, int addr_len, dword *ipaddr, uint16 *port )
{
	bp_assert( NULL != ipaddr && sizeof( struct sockaddr_in ) == addr_len );
	bp_assert( NULL != ipaddr && NULL != port );

	*ipaddr = ntohl( addr->sin_addr.s_addr );
	*port = ntohs( addr->sin_port );
}

int __try_recvfrom( int socket, struct sockaddr *addr, int *addr_len, char *buf, int buflen  )
{
	int ret;
	struct sockaddr_in *addrin;

	addrin = ( struct sockaddr_in* )addr;

	bp_assert( INVALID_SOCKET != socket );

	ret = recvfrom( socket, 
		buf, 
		buflen, 
		0, 
		( SOCKADDR * )addr, 
		addr_len );

	if( ret <= 0 )
	{
		return ret - 1;
	}

	return ret;
}

int try_recvfrom( int socket, dword *ipaddr, uint16 *port, char *buf, int buflen )
{
	int ret;
	struct sockaddr_in addr;
	int addr_len;

	bp_assert( NULL != ipaddr && NULL != port );

	memset( &addr, 0, sizeof( addr ) );
	addr_len = sizeof( addr );

	ret = __try_recvfrom( socket, ( struct sockaddr* )&addr, &addr_len, buf, buflen );

	get_inet_addr_info( &addr, addr_len, ipaddr, port );

	return ret;
}

int send_broadcast( int socket, char *subnet_ip, int16 port, char *buf, uint32 sendlen )
{
	int ret;

	bp_assert( INVALID_SOCKET != socket );

	ret = try_sendto( socket, subnet_ip, port, buf, sendlen );

	return ret;
}

int recv_broadcast( int socket, dword *peer_ipaddr, uint16 *peer_port, char *buf, uint32 recvlen )
{
	int ret;

	bp_assert( INVALID_SOCKET != socket );

	ret = try_recvfrom( socket, peer_ipaddr, peer_port, buf, recvlen );

	//socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	return ret;
}