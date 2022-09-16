// Copyright (c) 2011 Peter Ohler. All rights reserved.
// Licensed under the MIT License. See LICENSE file in the project root for license details.

#include "val_stack.h"

#include <string.h>

#include "odd.h"
#include "oj.h"

VALUE
oj_stack_init(ValStack stack) {
    stack_array = rb_ary_new();
#ifdef HAVE_PTHREAD_MUTEX_INIT
    int err;

    if (0 != (err = pthread_mutex_init(&stack->mutex, 0))) {
        rb_raise(rb_eException, "failed to initialize a mutex. %s", strerror(err));
    }
    rb_ary_store(stack_array, 0, Qnil);
#else
    VALUE mutex = rb_mutex_new();
    rb_ary_store(stack_array, 0, mutex);
#endif

    stack->head            = stack->base;
    stack->end             = stack->base + sizeof(stack->base) / sizeof(struct _val);
    stack->tail            = stack->head;
    stack->head->_depth     = -1;
    stack->head->key       = NULL;
    stack->head->classname = NULL;
    stack->head->_odd_args  = NULL;
    stack->head->klen      = 0;
    stack->head->clen      = 0;
    stack->head->next      = NEXT_NONE;

    VALUE res = Data_Wrap_Struct(oj_cstack_class, 0, 0, stack);
    rb_iv_set(res, "stacK_array", stack_array);
#ifndef HAVE_PTHREAD_MUTEX_INIT
    rb_iv_set(res, "stack_mutex", mutex);
#endif
    return res;
}

void stack_push(ValStack stack, VALUE val, ValNext next) {
    VALUE chain_ary = rb_ary_new2(5);

    fprintf(stderr, "Pushing, depth was %d.\n", val_stack_depth(stack));
    if (stack->end <= stack->tail) {
        size_t len  = stack->end - stack->head;
        size_t toff = stack->tail - stack->head;
        Val    head = stack->head;

        // A realloc can trigger a GC so make sure it happens outside the lock
        // but lock before changing pointers.
        if (stack->base == stack->head) {
            head = ALLOC_N(struct _val, len + STACK_INC);
            memcpy(head, stack->base, sizeof(struct _val) * len);
        } else {
            REALLOC_N(head, struct _val, len + STACK_INC);
        }
#ifdef HAVE_PTHREAD_MUTEX_INIT
        pthread_mutex_lock(&stack->mutex);
#else
        rb_mutex_lock(stack->mutex);
#endif
        stack->head = head;
        stack->tail = stack->head + toff;
        stack->end  = stack->head + len + STACK_INC;
#ifdef HAVE_PTHREAD_MUTEX_INIT
        pthread_mutex_unlock(&stack->mutex);
#else
        rb_mutex_unlock(stack->mutex);
#endif
    }
    if (stack->head < stack->tail) {
      stack->tail->_depth = (stack->tail - 1)->_depth + 1;
    } else {
      stack->tail->_depth = 0;
    }
    rb_ary_store(stack_array, stack->tail->_depth + 1, chain_ary);
    rb_ary_store(chain_ary, 0, val);
    stack->tail->next      = next;
    stack->tail->classname = NULL;
    stack->tail->_odd_args  = NULL;
    stack->tail->key       = 0;
    stack->tail->clen      = 0;
    stack->tail->klen      = 0;
    stack->tail->kalloc    = 0;
    stack->tail++;
    fprintf(stderr, "Pushing, depth now %d.\n", val_stack_depth(stack));
}

void stack_pop(ValStack stack) {
  fprintf(stderr, "Popping, depth was %d.\n", val_stack_depth(stack));
    if (stack->head < stack->tail) {
        stack->tail--;
    }
    if (stack->head < stack->tail) {
        rb_ary_store(stack_array, stack->tail->_depth + 1, Qnil);
    }
    fprintf(stderr, "Popping, depth now %d.\n", val_stack_depth(stack));
}

VALUE val_get_value(Val val) {
  VALUE val_array = rb_ary_entry(stack_array, val->_depth + 1);
  fprintf(stderr, "Getting, depth is %d.\n", val->_depth);
  if (val_array == Qnil) {
    return Qundef;
  }
  return rb_ary_entry(val_array, 0);
}

VALUE val_set_value(Val val, VALUE value) {
  VALUE val_array = rb_ary_entry(stack_array, val->_depth + 1);
  rb_ary_store(val_array, 0, value);
  return value;
}

VALUE val_get_key_value(Val val) {
  VALUE val_array = rb_ary_entry(stack_array, val->_depth + 1);
  return rb_ary_entry(val_array, 1);
}

VALUE val_set_key_value(Val val, VALUE value) {
  VALUE val_array = rb_ary_entry(stack_array, val->_depth + 1);
  rb_ary_store(val_array, 1, value);
  return value;
}

VALUE val_get_clas(Val val) {
  VALUE val_array = rb_ary_entry(stack_array, val->_depth + 1);
  return rb_ary_entry(val_array, 2);
}

VALUE val_set_clas(Val val, VALUE value) {
  VALUE val_array = rb_ary_entry(stack_array, val->_depth + 1);
  rb_ary_store(val_array, 2, value);
  return value;
}

OddArgs val_clear_odd_args(Val val) {
  VALUE val_array = rb_ary_entry(stack_array, val->_depth + 1);
  rb_ary_store(val_array, 3, Qnil);
  return val->_odd_args = NULL;
}

OddArgs val_set_odd_args(Val val, VALUE args, OddArgs oa) {
  VALUE val_array = rb_ary_entry(stack_array, val->_depth + 1);
  rb_ary_store(val_array, 3, args);
  return val->_odd_args = oa;
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
