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
#include "linear_table.h"

int32 init_linear_table( linear_table **table, dword max_len )
{
	int32 ret;
	linear_table *new_table;

	bp_assert( NULL != table );

	ret = 0;
	new_table = NULL;
	new_table = ( linear_table* )malloc( sizeof( linear_table ) );
	if( NULL == new_table )
	{
		ret = -E_OUTOFMEMORY;
		goto error;
	}

	new_table->elements = ( table_ele* )malloc( sizeof( table_ele ) * max_len );
	new_table->max_len = max_len;
	new_table->cur_len = 0;

	return ret;

error:
	if( NULL != new_table )
	{
		free( new_table );
	}
	return ret;
}

#define LINEAR_TABLE_ELE_INCRE_LEN 10
int32 insert_new_element( linear_table *table, table_ele element )
{
	int32 ret;

	bp_assert( NULL != table );
	
	ret = 0;
	if( table->cur_len + 1 > table->max_len )
	{
		table_ele *new_elements;
		new_elements = ( table_ele* )malloc( sizeof( table_ele ) * ( table->max_len + LINEAR_TABLE_ELE_INCRE_LEN );
		if( NULL == new_elements )
		{
			ret = -E_OUTOFMEMORY;
			goto error;
		}
		
		table->elements = new_elements;
		table->max_len = table->max_len + LINEAR_TABLE_ELE_INCRE_LEN;
	}

	table->elements[ table->cur_len++ ] = element;

error:
	return ret;
}

int32 find_element( linear_table *table, table_ele element )
{
	int i;
	bp_assert( NULL != table );
	bp_assert( NULL != element );

	for( i = 0; i < table->cur_len; i ++ )
	{
		if( table->elements[ i ] == element )
		{
			return i;
		}
	}

	return -1;
}

int32 delete_element( linear_table *table, table_ele element )
{
	int32 ret;
	int32 index;
	bp_assert( NULL != table );
	bp_assert( NULL != element );

	index = find_element( table, element );

	if( index >= 0 )
	{
		memmove( table->elements + index, table->elements + index + 1, sizeof( table_ele ) * ( table->cur_len - index - 1 );
		return 0;
	}

	//for( i = 0; i < table->cur_len; i ++ )
	//{
	//	if( table->elements[ i ] == element )
	//	{
	//		memmove( table->elements + i, table->elements + i + 1, sizeof( table_ele ) * ( table->cur_len - i - 1 );
	//		return 0;
	//	}
	//}

	return index;
}

typedef int32 ( * table_element_handler )( table_ele element, dword fparam );

int32 linear_table_traverse( linear_table *table, table_element_handler handler, dword fparam )
{
	int i;
	bp_assert( NULL 1= table );
	bp_assert( NULL != handler );

	for( i = 0; i < table->cur_len; i ++ )
	{
		//bp_assert( NULL != table->elements[ i ] );
		handler( table->elements[ i ], fparam );
	}

	return 0;
}