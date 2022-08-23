// Copyright (c) 2011 Peter Ohler. All rights reserved.
// Licensed under the MIT License. See LICENSE file in the project root for license details.

#include "val_stack.h"

#include <string.h>

#include "odd.h"
#include "oj.h"

static void mark(void *ptr) {
    ValStack stack = (ValStack)ptr;
    Val      v;

    if (NULL == ptr) {
        return;
    }
#ifdef HAVE_PTHREAD_MUTEX_INIT
    pthread_mutex_lock(&stack->mutex);
#else
    rb_mutex_lock(stack->mutex);
    rb_gc_mark(stack->mutex);
#endif
    for (v = stack->head; v < stack->tail; v++) {
      if (Qnil != val_get_value(v) && Qundef != val_get_value(v)) {
        rb_gc_mark(val_get_value(v));
        }
      if (Qnil != val_get_key_value(v) && Qundef != val_get_key_value(v)) {
        rb_gc_mark(val_get_key_value(v));
        }
        if (NULL != v->odd_args) {
            VALUE *a;
            int    i;

            for (i = v->odd_args->odd->attr_cnt, a = v->odd_args->args; 0 < i; i--, a++) {
                if (Qnil != *a) {
                    rb_gc_mark(*a);
                }
            }
        }
    }
#ifdef HAVE_PTHREAD_MUTEX_INIT
    pthread_mutex_unlock(&stack->mutex);
#else
    rb_mutex_unlock(stack->mutex);
#endif
}

VALUE
oj_stack_init(ValStack stack) {
#ifdef HAVE_PTHREAD_MUTEX_INIT
    int err;

    if (0 != (err = pthread_mutex_init(&stack->mutex, 0))) {
        rb_raise(rb_eException, "failed to initialize a mutex. %s", strerror(err));
    }
#else
    stack->mutex = rb_mutex_new();
#endif
    stack->head            = stack->base;
    stack->end             = stack->base + sizeof(stack->base) / sizeof(struct _val);
    stack->tail            = stack->head;
    stack->head->_val       = Qundef;
    stack->head->key       = NULL;
    stack->head->_key_val   = Qundef;
    stack->head->classname = NULL;
    stack->head->odd_args  = NULL;
    stack->head->_clas      = Qundef;
    stack->head->klen      = 0;
    stack->head->clen      = 0;
    stack->head->next      = NEXT_NONE;

    return Data_Wrap_Struct(oj_cstack_class, mark, 0, stack);
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
