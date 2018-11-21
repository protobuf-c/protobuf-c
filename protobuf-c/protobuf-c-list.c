/*
 * Copyright (c) 2018, Naveen Gogineni and the protobuf-c authors.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "protobuf-c-list.h"
#include "protobuf-c.h"

extern ProtobufCAllocator* _allocator;

void* list_add(_List* list, void * value)
{
    _ListNode* node = _allocator->alloc(_allocator->allocator_data, sizeof(_ListNode));
    if(!node) return NULL;
    node->data = value;          
    if(list->first==NULL) 
    {
        list->first = node;
    }
    else 
    {
        list->last->next = node; 
    }       
    node->prev = list->last;
    list->last = node;
    list->length++;
    return node;
}

void* list_remove(_List* list)
{
    void * value = NULL;
    if( list->first == NULL ) 
    {
        return NULL;
    }
    value = list->last->data;
    if( list->last->prev ) 
    {  
        _ListNode* tmp = list->last->prev; 
        _allocator->free(_allocator->allocator_data, list->last);
        tmp->next = NULL;        
        list->last = tmp;
        list->length--;  
    } 
    else 
    {                     
        _allocator->free(_allocator->allocator_data, list->last);
        list->last = NULL;     
        list->first = NULL; 
        list->length = 0; 
    }                       
    return value;
}                           

void** list_flatten(_List* list)
{
    if(list->length == 0 )
    {
        return NULL;
    }
    void ** array = _allocator->alloc(_allocator->allocator_data, 
                                    list->length*sizeof(void *));
    _ListNode* node = list->first;
    int i = 0;
    if( !array ) return NULL;
    while(node)                                    
    {
        array[i++] = node->data;
        node = node->next;
    }
    return array;
}

void list_free(_List* list)
{
    _ListNode* tmp = list->first;
    while(tmp) 
    {
        _ListNode* next = tmp->next;
        _allocator->free(_allocator->allocator_data, tmp->data);  
        _allocator->free(_allocator->allocator_data, tmp);        
        tmp = next;                         
    }                                       
    list->first = list->last = NULL;    
    list->length = 0;                                             
}
