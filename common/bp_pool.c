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
#include "bp_pool.h"


#define BP_POOL_INCRE_LEN 30
int32 init_bp_pool( bp_pool **pool )
{
	int ret;
	bp_pool *new_pool;

	bp_assert( NULL != pool );

	ret = 0;

	new_pool = ( bp_pool* )malloc( sizeof( bp_pool ) );

	if( NULL == new_pool )
	{
		ret = -E_OUTOFMEMORY;
		goto error;
	}

	new_pool->elements = ( pool_element* )malloc( sizeof( pool_element ) * BP_POOL_INCRE_LEN );

	if( NULL == new_pool->elements )
	{
		ret = -E_OUTOFMEMORY;
		goto error;
	}

	new_pool->len = 0;
	new_pool->max_len = BP_POOL_INCRE_LEN;

	*pool = new_pool;
error:
	return ret;
}

#define realloc_element_if_needed( buf, new_buf, max_len, new_len, ele_size, ret_val ) \
	ret_val = 0; \	
	if( ( new_len ) > ( max_len ) ) \
	{ \
		new_buf = realloc( ( buf ), ( new_len ) * ele_size ); \
		if( NULL != ( new_buf ) ) \
		{ \
			ret_val = -E_OUTOFMEMORY; \
			buf = new_buf; \
			( max_len ) = ( new_len ); \
		} \
	} \


byte* find_one_size_pool_element( bp_pool *pool, dword size )
{
	int32 i;
	int32 size_exist;
	dword time_waited;
	pool_element *cur_ele;

	bp_assert( NULL != pool );
	bp_assert( 0 != size );

	time_waited = 0;
	while( 1 )
	{
		for( i = 0; i < pool->len; i ++ )
		{
			cur_ele = pool->elements[ i ];

			if( size == cur_ele->buf_len )
			{
				size_exist = TRUE;
				if( FALSE == cur_ele->used )
				{
					cur_ele->used = TRUE;
					return cur_ele->buff;
				}
			}
		}

		if( FALSE == size_exist )
		{
			pool_element *new_eles;
			realloc_element_if_needed( pool->elements, new_eles, pool->max_len, pool->len + 1, sizeof( pool_element ), ret );

			if( 0 > ret )
			{
				goto error;
			}

			pool->elements[ pool->len ].buff = malloc( sizeof( byte ) * size );
			if( NULL == pool->elements[ pool->len ].buff )
			{
				ret = -E_OUTOFMEMORY;
				goto error;
			}

			pool->elements[ pool->len ].used = TRUE;
			pool->elements[ pool->len ].buf_len = size;
			return pool->elements[ pool->len ].buff;
		}
		else
		{
			if( time_waited >= wait_time )
			{
				return NULL;
			}
#define POOL_WAIT_TIME 300
#ifdef _WIN32
#define bp_sleep( ms ) Sleep( ms )
#else
#define bp_sleep( us ) sleep( us * 1000 )
#endif
			bp_sleep( POOL_WAIT_TIME );
			time_waited += POOL_WAIT_TIME;
		}
	}

error:
	return NULL;
}

int32 release_pool_element( bp_pool *pool, byte *buf )
{
	int32 i;
	bp_assert( NULL != pool );
	bp_assert( NULL != buf );

	for( i = 0; i < pool->len; i ++ )
	{
		bp_assert( pool->elements[ i ] != NULL );

		if( pool->elements[ i ].buff == buf )
		{
			pool->elements[ i ].used = FALSE;
			return 0;
		}
	}

	return -1;
}

void destroy_pool( bp_pool *pool )
{
	int32 i;

	bp_assert( NULL != pool );

	for( i = 0; i < pool->len; i ++ )
	{
		bp_assert( NULL != pool->elements[ i ].buff );
		free( pool->elements[ i ] );
	}

	free( pool->elements );

	free( pool );
}
