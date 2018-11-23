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
 
#ifndef __BALANCE_PARALLEL_HASH_H__
#define __BALANCE_PARALLEL_HASH_H__

#define HASH_KEY_EXISTED -11
typedef struct __long_int
{
	dword low_part;
	dword high_part;
} long_int;

typedef long_int hash_key;
typedef void *hash_value;
#define HASH_NULL_VALUE NULL
typedef struct __hash_item
{
	hash_key key;
	hash_value value;

	struct __hash_item *next_item;
	struct __hash_item *next_link;

} hash_item;

typedef struct __hash_table
{
#define first_hash_item header->next_item
	hash_item *header;
	hash_item *items;
	dword size;
} hash_table;

typedef int ( *hash_val_comp )( hash_val, hash_val );
typedef void ( *destroy_hash_value )( hash_value *value, dword param );

#define hash_table_inited( table ) ( NULL != ( table )->header )
#define compare_key( key1, key2 ) ( key1.high_part == key2.high_part && key1.low_part == key2.low_part )

dword modulo_hash( hash_key key, dword table_size );
int hash_item_is_empty( hash_item *item );
int find_item_in_list( hash_item *list, hash_key key, hash_item **prev_item, hash_item **item );
int locate_hash_item( hash_table *table, hash_key key, hash_item **item );
int add_hash_item( hash_table *table, hash_key key, hash_value val );
hash_item *find_prev_item( hash_table *table, hash_item *item );
int del_hash_item( hash_table *table, hash_key key, hash_value *value );
int get_hash_value( hash_table *table, hash_key key, hash_value *value );
int init_hash_table( hash_table *table, dword size );
hash_item* get_next_hash_item( hash_item *item );
void destroy_hash_table( hash_table *table, destroy_hash_value des_func, dword param );
hash_key make_hash_key( dword higher, dword lower );
void* get_next_item_value( void *pos_record, hash_table *table, hash_value *value );
int hash_is_empty( hash_table *table );

#endif //__BALANCE_PARALLEL_HASH_H__