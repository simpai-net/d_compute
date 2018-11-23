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
#include "dlist.h"

int add_list_element( dlist *destlist, list_element element )
{
	dlist *newlist;
	bp_assert( NULL != destlist );

	newlist = ( dlist* )malloc( sizeof( dlist ) );
	if( NULL == newlist )
	{
		return -E_OUTOFMEMORY;
	}

	memset( newlist, 0, sizeof( dlist ) );
	newlist->info = element;
	
	newlist->prev = destlist;
	newlist->next = destlist->next;
	if( NULL != destlist->next )
	{
		destlist->next->prev = newlist;
	}
	destlist->next = newlist;
	
	return 0;
}

int del_list_item( dlist *listitem, list_element *element )
{
	bp_assert( NULL != listitem );

	if( NULL != listitem->prev )
	{
		listitem->prev->next = listitem->next;
	}

	if( NULL != listitem->next )
	{
		listitem->next->prev = listitem->prev;
	}

	if( NULL != element )
	{
		*element = listitem->info;
	}
	free( listitem );
	return 0;
}

dlist* find_list_element( dlist *destlist, list_element element )
{
	dlist *listitem;
	bp_assert( NULL != destlist );

	listitem = destlist;
	for( ; NULL != listitem; listitem = listitem->next )
	{
		if( listitem->info == element )
		{
			return listitem;
		}
	}

	return NULL;
}

dlist* get_list_item_ptr( list_element *list_item_member_addr )
{
	dlist *listitem;
	bp_assert( NULL != list_item_member_addr );

	listitem = ( dlist* )list_item_member_addr;

	return listitem;
}

int del_list_element2( dlist *destlist, int index, list_element *element )
{
	int count;
	dlist *listitem;
	bp_assert( NULL != destlist );

	listitem = destlist;
	count = 0;
	for( ; NULL != listitem; listitem = listitem->next )
	{
		if( count == index )
		{
			del_list_item( listitem, element );
			return 0;
		}

		count ++;
	}

	return -1;
}

int del_list_element( dlist *destlist, list_element element )
{
	dlist *listitem;
	list_element tmp_elem;
	listitem = find_list_element( destlist, element );
	if( NULL != listitem )
	{
		del_list_item( listitem, &tmp_elem );
	}
	return 0;
}

int init_list_element( dlist **dest_list )
{
	dlist *new_list;
	new_list = ( dlist* )malloc( sizeof( dlist ) );
	if( NULL == new_list )
	{
		return -E_OUTOFMEMORY;
	}
	memset( new_list, 0, sizeof( dlist ) );
	*dest_list = new_list;
	return 0;
}

int destroy_list( dlist *list_header, destroy_list_element_func des_func )
{
	int ret;
	int failed_count;

	dlist *item;
	dlist *next_item;

	bp_assert( NULL != list_header );
	bp_assert( NULL == list_header->prev && NULL == list_header->info );

	item = list_header;
	failed_count = 0;

	for( ; ; )
	{
		if( NULL == item )
		{
			break;
		}

		next_item = get_next_list( item );
		if( NULL != des_func )
		{
			ret = des_func( item->info );
			if( 0 > ret )
			{
				failed_count ++;
			}
		}

		del_list_item( item, NULL );

		item = next_item;
	}
	
	return -failed_count;
}

dlist* get_next_list( dlist *item )
{
	bp_assert( item );
	bp_assert( item->prev != NULL ? item->prev->next == item : TRUE );
	bp_assert( item->next != NULL ? item->next->prev == item : TRUE );

	return item->next;
}

dlist* get_prev_list( dlist *item )
{
	bp_assert( item );
	bp_assert( item->prev != NULL ? item->prev->next == item : TRUE );
	bp_assert( item->next != NULL ? item->next->prev == item : TRUE );

	return item->prev;
}

dlist* get_next_list_element( dlist *item, list_element *element )
{
	dlist *next_item;

	bp_assert( NULL != item );

	next_item = get_next_list( item );
	
	bp_assert( NULL != item );
	bp_assert( NULL != element );

	if( NULL == next_item )
	{
		*element = NULL;
		return NULL;
	}

	*element = next_item->info;
	return next_item;
}

dlist* get_prev_list_elememt( dlist *item, list_element *element )
{
	dlist *prev_item;
	prev_item = get_prev_list( item );
	
	bp_assert( NULL != item );
	bp_assert( NULL != element );

	if( NULL == prev_item )
	{
		*element = NULL;
		return NULL;
	}

	*element = prev_item->info;
	return prev_item;
}