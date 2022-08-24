// Copyright (c) 2011 Peter Ohler. All rights reserved.
// Licensed under the MIT License. See LICENSE file in the project root for license details.

#include "val_stack.h"

#include <string.h>

#include "odd.h"
#include "oj.h"

VALUE
oj_stack_init(ValStack stack) {
    VALUE chain_ary = rb_ary_new2(5);
#ifdef HAVE_PTHREAD_MUTEX_INIT
    int err;

    if (0 != (err = pthread_mutex_init(&stack->mutex, 0))) {
        rb_raise(rb_eException, "failed to initialize a mutex. %s", strerror(err));
    }
#else
    VALUE mutex = rb_mutex_new()
    stack->mutex = mutex;
#endif
    stack->head            = stack->base;
    stack->end             = stack->base + sizeof(stack->base) / sizeof(struct _val);
    stack->tail            = stack->head;
    stack->head->_chain_ary = chain_ary;
    stack->head->_val       = Qundef;
    stack->head->key       = NULL;
    stack->head->_key_val   = Qundef;
    stack->head->classname = NULL;
    stack->head->_odd_args  = NULL;
    stack->head->_clas      = Qundef;
    stack->head->klen      = 0;
    stack->head->clen      = 0;
    stack->head->next      = NEXT_NONE;

    VALUE res = Data_Wrap_Struct(oj_cstack_class, 0, 0, stack);
    rb_iv_set(res, "stack_chain", chain_ary);
#ifndef HAVE_PTHREAD_MUTEX_INIT
    rb_iv_set(res, "stack_mutex", mutex);
#endif
    return res;
}

const char *oj_stack_next_string(ValNext n) {
    switch (n) {
    case NEXT_ARRAY_NEW: return "array element or close";
    case NEXT_ARRAY_ELEMENT: return "array element";
    case NEXT_ARRAY_COMMA: return "comma";
    case NEXT_HASH_NEW: return "hash pair or close";
    case NEXT_HASH_KEY: return "hash key";
    case NEXT_HASH_COLON: return "colon";
    case NEXT_HASH_VALUE: return "hash value";
    case NEXT_HASH_COMMA: return "comma";
    case NEXT_NONE: break;
    default: break;
    }
    return "nothing";
}
