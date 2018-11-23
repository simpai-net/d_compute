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

#define GENERAL_WINSOCK_API_ERROR_LEVEL 3

void main()
{
	//enum_all_net_card();
	broadcast_comm brd_comm;
	dword peer_ip;
	uint16 peer_port;
#define SUBNET_BROADCAST_IP INADDR_BROADCAST
	char brd_buf[ 1024 ];

	init_win32sock_lib();

	init_brd_comm_unit( &brd_comm );
	if( 0 > create_brdcast_sock( &brd_comm.brd_sock, NULL, BP_BROADCAST_PORT ) )
		return;
	if( 0 > create_udp_socket( &brd_comm.peer_sock, NULL, BP_BROADCAST_PORT + 1 ) )
		return;

#define SEND_TEST_CONTENT "hello, i broadcast to you\n"
#define PEER_SEND_TEST_CONTENT SEND_TEST_CONTENT " with peer"

	while( 1 )
	{
		//sendbroadcast( brd_comm.brd_sock, "255.255.255.255", BP_BROADCAST_PORT, SEND_TEST_CONTENT, sizeof( SEND_TEST_CONTENT ) );
		//printf( "send brd content is: %s\n", SEND_TEST_CONTENT );
		memset( brd_buf, 0, sizeof( brd_buf ) );
		recv_broadcast( brd_comm.brd_sock, &peer_ip, &peer_port, brd_buf, 1024 );
		printf( "recv brd content is: %s\n", brd_buf );

		//if( peer_ip != brd_comm.peer_ip || peer_port != brd_comm.peer_port )
		//{
		//	brd_comm.peer_ip = peer_ip;
		//	brd_comm.peer_port = peer_port;
		//}

		//if( 0 != peer_ip && 0 != peer_port )
		//{
		//	loop_try_sendto2( brd_comm.peer_sock, peer_ip, peer_port, PEER_SEND_TEST_CONTENT, sizeof( PEER_SEND_TEST_CONTENT ) );
		//	loop_try_recvfrom( brd_comm.peer_sock, &peer_ip, &peer_port, brd_buf, 1024 );
		//}
		Sleep( 10 );
	}

	close_win32sock_lib();
	return;
}
