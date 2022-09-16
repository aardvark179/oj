// Copyright (c) 2011 Peter Ohler. All rights reserved.
// Licensed under the MIT License. See LICENSE file in the project root for license details.

#ifndef OJ_VAL_STACK_H
#define OJ_VAL_STACK_H

#include <stdint.h>

#include "odd.h"
#include "ruby.h"
#ifdef HAVE_PTHREAD_MUTEX_INIT
#include <pthread.h>
#endif

#define STACK_INC 64

typedef enum {
    NEXT_NONE          = 0,
    NEXT_ARRAY_NEW     = 'a',
    NEXT_ARRAY_ELEMENT = 'e',
    NEXT_ARRAY_COMMA   = ',',
    NEXT_HASH_NEW      = 'h',
    NEXT_HASH_KEY      = 'k',
    NEXT_HASH_COLON    = ':',
    NEXT_HASH_VALUE    = 'v',
    NEXT_HASH_COMMA    = 'n',
} ValNext;

typedef struct _val {
    int _depth;
    const char *   key;
    char           karray[32];
    const char *   classname;
    OddArgs        _odd_args;
    uint16_t       klen;
    uint16_t       clen;
    char           next;  // ValNext
    char           k1;    // first original character in the key
    char           kalloc;
} * Val;

typedef struct _valStack {
    struct _val base[STACK_INC];
    Val         head;  // current stack
    Val         end;   // stack end
    Val         tail;  // pointer to one past last element name on stack
#ifdef HAVE_PTHREAD_MUTEX_INIT
    pthread_mutex_t mutex;
#else
    VALUE mutex;
#endif

} * ValStack;

extern VALUE oj_stack_init(ValStack stack);

inline static int stack_empty(ValStack stack) {
    return (stack->head == stack->tail);
}

inline static void stack_cleanup(ValStack stack) {
    if (stack->base != stack->head) {
        xfree(stack->head);
        stack->head = NULL;
    }
}

extern void stack_push(ValStack stack, VALUE val, ValNext next);

extern void stack_pop(ValStack stack);

extern VALUE val_get_value(Val val);

extern VALUE val_get_key_value(Val val);

extern VALUE val_get_clas(Val val);

inline static OddArgs val_get_odd_args(Val val) {
  return val->_odd_args;
}

extern OddArgs val_clear_odd_args(Val val);

extern VALUE val_set_value(Val val, VALUE v);

extern VALUE val_set_key_value(Val val, VALUE v);

extern VALUE val_set_clas(Val val, VALUE v);

inline static size_t stack_size(ValStack stack) {
    return stack->tail - stack->head;
}

inline static Val stack_peek(ValStack stack) {
    if (stack->head < stack->tail) {
        return stack->tail - 1;
    }
    return 0;
}

inline static Val stack_peek_up(ValStack stack) {
    if (stack->head < stack->tail - 1) {
        return stack->tail - 2;
    }
    return 0;
}

inline static Val stack_prev(ValStack stack) {
    return stack->tail;
}

inline static VALUE stack_head_val(ValStack stack) {
  return val_get_value(stack->head);
}

static inline int val_stack_depth(ValStack stack) {
  return stack->tail - stack->head;
}

extern const char *oj_stack_next_string(ValNext n);

// We declare this here to resolve a circular dependencies between the stack and odd headers.
extern OddArgs oj_odd_alloc_args(Val val, Odd odd);

#endif /* OJ_VAL_STACK_H */
