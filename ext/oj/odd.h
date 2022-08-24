// Copyright (c) 2011 Peter Ohler. All rights reserved.
// Licensed under the MIT License. See LICENSE file in the project root for license details.

#ifndef OJ_ODD_H
#define OJ_ODD_H

#include <stdbool.h>

#include "ruby.h"

#define MAX_ODD_ARGS 10

typedef VALUE (*AttrGetFunc)(VALUE obj);

typedef struct _odd {
    struct _odd *next;
    const char * classname;
    size_t       clen;
    VALUE        _odd_chain;
    VALUE        _clas;  // Ruby class or module
    VALUE        _create_obj;
    ID           create_op;
    int          attr_cnt;
    bool         is_module;
    bool         raw;
    const char * attr_names[MAX_ODD_ARGS];  // NULL terminated attr names
    ID           attrs[MAX_ODD_ARGS];       // 0 terminated attr IDs
    AttrGetFunc  attrFuncs[MAX_ODD_ARGS];
} * Odd;

typedef struct _oddArgs {
    Odd   odd;
    VALUE _args[MAX_ODD_ARGS];
} * OddArgs;

#define ODD_ARGS_PTR(odd) odd->_args
#define ODD_GET_CLASS(odd) odd->_clas
#define ODD_GET_CREATE_OBJ(odd) odd->_create_obj
#define ODD_SET_CLASS(odd, clas) (odd_set_class(odd, clas))
#define ODD_SET_CREATE_OBJ(odd, create_obj) (odd_set_create_obj(odd, create_obj))

static inline VALUE odd_set_class(Odd odd, VALUE clas) {
  rb_ary_store(odd->_odd_chain, 1, clas);
  return odd->_clas = clas;
}

static inline VALUE odd_set_create_obj(Odd odd, VALUE create_obj) {
  rb_ary_store(odd->_odd_chain, 2, create_obj);
  return odd->_create_obj = create_obj;
}

extern void    oj_odd_init(void);
extern Odd     oj_get_odd(VALUE clas);
extern Odd     oj_get_oddc(const char *classname, size_t len);
extern OddArgs oj_odd_alloc_args(Odd odd);
extern void    oj_odd_free(OddArgs args);
extern int     oj_odd_set_arg(OddArgs args, const char *key, size_t klen, VALUE value);
extern void    oj_reg_odd(VALUE clas, VALUE create_object, VALUE create_method, int mcnt, VALUE *members, bool raw);

#endif /* OJ_ODD_H */
