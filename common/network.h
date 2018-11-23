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

#ifndef __BALANCE_PARALLEL_NETWORK_H__
#define __BALANCE_PARALLEL_NETWORK_H__
#include <winsock2.h>
#include <mswsock.h>

int readln( int socket, char *line, int ln_max_len, char *recvedbuf, unsigned long *buf_begin, unsigned long max_buf_len);
int init_win32sock_lib();
void close_win32sock_lib();
void  __construct_inet_addr( dword ipaddr, uint16 port, struct sockaddr *addr, int addr_len );
dword inet_addr_conv( char *ipaddr );
char* ip2str( dword ipaddr );
void  construct_inet_addr( char* local_ip, uint16 port, struct sockaddr *addr, int addr_len );
int connect2server( int *sock, char *ipaddr, int16 port );
int connect2server2( int *sock, dword ipaddr, uint16 port );
int sendln( int sock, char *buf, int buflen );
int loop_try_send( int socket, char *buf, int buflen );
int loop_try_recv( int socket, char *buf, int buflen );
int check_addr_sanity( char *ipaddr, unsigned short port );
int check_addr_sanity2( dword ipaddr, unsigned short port );
int generate_tcp_listen_sock( int *sock, char *ipaddr, uint16 port, int flag );
int gracefully_close( int sock, char *buf, int buf_len );
int sock_io_ctl( int sock, long cmd, ulong *value );
int sock_io_ctl_ex( int sock, 
				   dword cmd, 
				   void *input, 
				   dword input_len, 
				   void *output,
				   dword output_len
				   );
int get_broadcast_addr( int sock, struct sockaddr *addr, dword addr_len );

#endif //__BALANCE_PARALLEL_NETWORK_H__