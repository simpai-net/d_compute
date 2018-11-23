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

#ifndef __BANLANCE_PARALLEL_DLIST_H__
#define __BANLANCE_PARALLEL_DLIST_H__

typedef void* list_element;

typedef struct __dlist
{
	list_element info;
	struct __dlist *prev;
	struct __dlist *next;
} dlist;

#include "common.h"
#include "dlist.h"

typedef int ( *destroy_list_element_func )( list_element );

int add_list_element( dlist *destlist, list_element element );
int del_list_item( dlist *listitem, list_element *element );
dlist* find_list_element( dlist *destlist, list_element element );
dlist* get_list_item_ptr( list_element *list_item_member_addr );
int del_list_element2( dlist *destlist, int index, list_element *element );
int del_list_element( dlist *destlist, list_element element );
int init_list_element( dlist **dest_list );
int destroy_list( dlist *list_header, destroy_list_element_func des_func );
dlist* get_next_list( dlist *item );
dlist* get_prev_list( dlist *item );
dlist* get_next_list_element( dlist *item, list_element *element );
dlist* get_prev_list_elememt( dlist *item, list_element *element );

#endif