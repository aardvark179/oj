// Copyright (c) 2011 Peter Ohler. All rights reserved.
// Licensed under the MIT License. See LICENSE file in the project root for license details.

#include "odd.h"

#include <string.h>
#include "val_stack.h"

static Odd odds = NULL;
static VALUE odd_chain_root = Qundef;
static ID  sec_id;
static ID  sec_fraction_id;
static ID  to_f_id;
static ID  numerator_id;
static ID  denominator_id;
static ID  rational_id;

static void set_class(Odd odd, const char *classname) {
    const char **np;
    ID          *idp;

    odd->classname = classname;
    odd->clen      = strlen(classname);
    ODD_SET_CLASS(odd, rb_const_get(rb_cObject, rb_intern(classname)));
    ODD_SET_CREATE_OBJ(odd, ODD_GET_CLASS(odd));
    odd->create_op = rb_intern("new");
    odd->is_module = (T_MODULE == rb_type(ODD_GET_CLASS(odd)));
    odd->raw       = 0;
    for (np = odd->attr_names, idp = odd->attrs; 0 != *np; np++, idp++) {
        *idp = rb_intern(*np);
    }
    *idp = 0;
}

static VALUE get_datetime_secs(VALUE obj) {
    volatile VALUE rsecs = rb_funcall(obj, sec_id, 0);
    volatile VALUE rfrac = rb_funcall(obj, sec_fraction_id, 0);
    long           sec   = NUM2LONG(rsecs);
    long long      num   = NUM2LL(rb_funcall(rfrac, numerator_id, 0));
    long long      den   = NUM2LL(rb_funcall(rfrac, denominator_id, 0));

    num += sec * den;

    return rb_funcall(rb_cObject, rational_id, 2, rb_ll2inum(num), rb_ll2inum(den));
}

static void print_odd(Odd odd) {
    const char **np;
    int          i;

    printf("  %s {\n", odd->classname);
    printf("    attr_cnt: %d %p\n", odd->attr_cnt, (void *)odd->attr_names);
    printf("    attr_names: %p\n", (void *)*odd->attr_names);
    printf("    attr_names: %c\n", **odd->attr_names);
    for (i = odd->attr_cnt, np = odd->attr_names; 0 < i; i--, np++) {
        printf("    %d %s\n", i, *np);
    }
    printf("  }\n");
}

void print_all_odds(const char *label) {
    Odd odd;
    printf("@ %s {\n", label);
    for (odd = odds; NULL != odd; odd = odd->next) {
        print_odd(odd);
    }
    printf("}\n");
}

static Odd odd_create(void) {
    Odd odd = ALLOC(struct _odd);
    VALUE odd_chain = rb_ary_new2(3);

    memset(odd, 0, sizeof(struct _odd));
    odd->_odd_chain = odd_chain;
    odd->next = odds;
    // Build up the chain that will preserve all the VALUEs.
    if (odds) {
      rb_ary_store(odd->_odd_chain, 2, odds->_odd_chain);
    }
    rb_ary_store(odd_chain_root, 0, odd->_odd_chain);
    odds      = odd;

    return odd;
}

void oj_odd_init(void) {
    Odd          odd;
    const char **np;

    sec_id          = rb_intern("sec");
    sec_fraction_id = rb_intern("sec_fraction");
    to_f_id         = rb_intern("to_f");
    numerator_id    = rb_intern("numerator");
    denominator_id  = rb_intern("denominator");
    rational_id     = rb_intern("Rational");

    odd_chain_root = rb_ary_new();
    rb_gc_register_address(&odd_chain_root);

    // Rational
    odd   = odd_create();
    np    = odd->attr_names;
    *np++ = "numerator";
    *np++ = "denominator";
    *np   = 0;
    set_class(odd, "Rational");
    ODD_SET_CREATE_OBJ(odd, rb_cObject);
    odd->create_op  = rational_id;
    odd->attr_cnt   = 2;

    // Date
    odd   = odd_create();
    np    = odd->attr_names;
    *np++ = "year";
    *np++ = "month";
    *np++ = "day";
    *np++ = "start";
    *np++ = 0;
    set_class(odd, "Date");
    odd->attr_cnt = 4;

    // DateTime
    odd   = odd_create();
    np    = odd->attr_names;
    *np++ = "year";
    *np++ = "month";
    *np++ = "day";
    *np++ = "hour";
    *np++ = "min";
    *np++ = "sec";
    *np++ = "offset";
    *np++ = "start";
    *np++ = 0;
    set_class(odd, "DateTime");
    odd->attr_cnt     = 8;
    odd->attrFuncs[5] = get_datetime_secs;

    // Range
    odd   = odd_create();
    np    = odd->attr_names;
    *np++ = "begin";
    *np++ = "end";
    *np++ = "exclude_end?";
    *np++ = 0;
    set_class(odd, "Range");
    odd->attr_cnt = 3;
}

Odd oj_get_odd(VALUE clas) {
    Odd         odd;
    const char *classname = NULL;

    for (odd = odds; NULL != odd; odd = odd->next) {
      if (clas == ODD_GET_CLASS(odd)) {
            return odd;
        }
        if (odd->is_module) {
            if (NULL == classname) {
                classname = rb_class2name(clas);
            }
            if (0 == strncmp(odd->classname, classname, odd->clen) && ':' == classname[odd->clen]) {
                return odd;
            }
        }
    }
    return NULL;
}

Odd oj_get_oddc(const char *classname, size_t len) {
    Odd odd;

    for (odd = odds; NULL != odd; odd = odd->next) {
        if (len == odd->clen && 0 == strncmp(classname, odd->classname, len)) {
            return odd;
        }
        if (odd->is_module && 0 == strncmp(odd->classname, classname, odd->clen) && ':' == classname[odd->clen]) {
            return odd;
        }
    }
    return NULL;
}

extern OddArgs val_set_odd_args(Val val, VALUE args, OddArgs oa);

OddArgs oj_odd_alloc_args(Val val, Odd odd) {
    OddArgs oa = ALLOC_N(struct _oddArgs, 1);
    VALUE args = rb_ary_new2(MAX_ODD_ARGS);
    oa->_args = args;

    oa->odd = odd;
    return val_set_odd_args(val, args, oa);
}

void oj_odd_free(OddArgs args) {
    xfree(args);
}

int oj_odd_set_arg(OddArgs args, const char *key, size_t klen, VALUE value) {
    const char **np;
    VALUE       *vp;
    int          i;

    for (i = args->odd->attr_cnt, np = args->odd->attr_names, vp = ODD_ARGS_PTR(args); 0 < i; i--, np++, vp++) {
        if (0 == strncmp(key, *np, klen) && '\0' == *((*np) + klen)) {
            *vp = value;
            return 0;
        }
    }
    return -1;
}

void oj_reg_odd(VALUE clas, VALUE create_object, VALUE create_method, int mcnt, VALUE *members, bool raw) {
    Odd          odd;
    const char **np;
    ID          *ap;
    AttrGetFunc *fp;

    odd       = odd_create();
    ODD_SET_CLASS(odd, clas);
    if (NULL == (odd->classname = strdup(rb_class2name(clas)))) {
        rb_raise(rb_eNoMemError, "for class name.");
    }
    odd->clen       = strlen(odd->classname);
    ODD_SET_CREATE_OBJ(odd,  create_object);
    odd->create_op = SYM2ID(create_method);
    odd->attr_cnt  = mcnt;
    odd->is_module = (T_MODULE == rb_type(clas));
    odd->raw       = raw;
    for (ap = odd->attrs, np = odd->attr_names, fp = odd->attrFuncs; 0 < mcnt; mcnt--, ap++, np++, members++, fp++) {
        *fp = 0;
        switch (rb_type(*members)) {
        case T_STRING:
            if (NULL == (*np = strdup(RSTRING_PTR(*members)))) {
                rb_raise(rb_eNoMemError, "for attribute name.");
            }
            break;
        case T_SYMBOL:
            // The symbol can move and invalidate the name so make a copy.
            if (NULL == (*np = strdup(rb_id2name(SYM2ID(*members))))) {
                rb_raise(rb_eNoMemError, "for attribute name.");
            }
            break;
        default: rb_raise(rb_eArgError, "registered member identifiers must be Strings or Symbols."); break;
        }
        *ap = rb_intern(*np);
    }
    *np = 0;
    *ap = 0;
}
