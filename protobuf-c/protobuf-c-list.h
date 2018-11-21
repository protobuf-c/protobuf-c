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
#ifndef PROTOBUF_C_LIST_H
#define PROTOBUF_C_LIST_H

/* an item in a linked list */
typedef struct _ListNode
{
    struct _ListNode* prev;
    struct _ListNode* next;
    void* data;
} _ListNode;

/* a List object */
typedef struct
{
    _ListNode * first;
    _ListNode * last;
    int length;
} _List;

#define LIST_INITIALIZER { NULL, NULL, 0}
#define DEFINE_LIST(name) static _List name = LIST_INITIALIZER;

/* add a value to list */
void* list_add(_List* list, void * value);

/* dequeue an item from list and return data pointer */
void* list_remove(_List* list);

/* flatten the list to an array of data */
void** list_flatten(_List* list);

/* deallocate all memory from list */
void list_free(_List* list);

#endif /* PROTOBUF_C_LIST_H */