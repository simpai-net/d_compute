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
 

#ifndef __BALANCE_PARALLEL_BROADCAST_H__
#define __BALANCE_PARALLEL_BROADCAST_H__

#define UDP_MAX_SEND_LEN 65535
#define GENERAL_WINSOCK_API_ERROR_LEVEL 3

typedef struct __broadcast_comm
{
	int brd_sock;
	int peer_sock;
	dword peer_ip;
	uint16 peer_port;
} broadcast_comm;

void init_brd_comm_unit( broadcast_comm* comm_unit );
int __create_udp_socket( int *sock, struct sockaddr_in *local_addr );
int create_udp_socket( int *sock, char *local_ip, uint16 local_port );
int create_brdcast_sock( int *sock, char *ipaddr, uint16 port );
int __try_sendto( int socket, struct sockaddr *addr, int addr_len, char *buf, int buflen );
int try_sendto2( int socket, dword ipaddr, int16 port, char *buf, int buflen );
int try_sendto( int socket, char *ipaddr, int16 port, char *buf, int buflen );
void get_inet_addr_info( struct sockaddr_in *addr, int addr_len, dword *ipaddr, uint16 *port );
int __try_recvfrom( int socket, struct sockaddr *addr, int *addr_len, char *buf, int buflen  );
int try_recvfrom( int socket, dword *ipaddr, uint16 *port, char *buf, int buflen );
int send_broadcast( int socket, char *subnet_ip, int16 port, char *buf, uint32 sendlen );
int recv_broadcast( int socket, dword *peer_ipaddr, uint16 *peer_port, char *buf, uint32 recvlen );

#endif

