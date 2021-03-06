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
 
#include <tchar.h>
#include "common.h"
#include "iocompport.h"

int _tmain(int argc, _TCHAR* argv[])
{
	int i;
	int ret;
	io_info *test_io_info;

	init_bp_work(0, 0, NULL);

	for( i = 0; i < 100; i++ )
	{
		ret = create_new_io_info( &test_io_info, 23, 44521 );
		bp_trace( 1, "create_new_io_info\n" );
		destroy_io_info( test_io_info );
	}

	for( i = 0; i < 100; i++ )
	{
		ret = create_new_io_info( &test_io_info, 23, 44521 );
		bp_trace( 1, "create_new_io_info\n" );
		destroy_io_info( test_io_info );
	}
	return 0;
}

