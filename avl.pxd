# -*- coding: utf-8 -*-

# Copyright (C) 1995 by Sam Rushing <rushing@nightmare.com>
# Copyright (C) 2005 by Germanischer Lloyd AG
# Copyright (C) 2001-2005 by IronPort Systems, Inc.
# Copyright (C) 2019 by Berthold Höllmann.
# cython: language_level=2

cdef extern from "avl_.h":

    ctypedef struct avl_node:
        void *key
        avl_node *left
        avl_node *right
        avl_node *parent
        # The lower 2 bits of <rank_and_balance> specify the balance
        # factor: 00==-1, 01==0, 10==+1.
        # The rest of the bits are used for <rank>
        unsigned int rank_and_balance

    cdef unsigned int AVL_GET_RANK(void *  n)

    cdef void AVL_SET_BALANCE(void *, int)

    cdef void AVL_SET_RANK(void *, int)

    ctypedef int (*avl_key_compare_fun_type) (void* compare_arg, void* a, void* b)

    ctypedef int (*avl_free_key_fun_type)    (void * key)
    ctypedef int (*avl_key_printer_fun_type) (char *, void *)

    ctypedef struct avl_tree:
        avl_node * root
        unsigned int                  length
        avl_key_compare_fun_type      compare_fun
        void *                        compare_arg

    cdef avl_tree * avl_new_avl_tree(
        avl_key_compare_fun_type compare_fun, void * compare_arg)

    cdef avl_node * avl_new_avl_node (void * key, avl_node * parent)

    cdef void avl_free_avl_tree (
        avl_tree *            tree,
        avl_free_key_fun_type free_key_fun
    )

    cdef int avl_insert_by_key (
        avl_tree *            ob,
        void *                key,
        unsigned int *        index
    )

    cdef int avl_remove_by_key (
        avl_tree *            tree,
        void *                key,
        avl_free_key_fun_type free_key_fun
    )

    cdef int avl_get_item_by_index (
        avl_tree *            tree,
        unsigned int          index,
        void **               value_address
    )

    cdef int avl_get_item_by_key (
        avl_tree *            tree,
        void *                key,
        void **               value_address
    )

    cdef int avl_get_span_by_key (
        avl_tree *            tree,
        void *                key,
        unsigned int *        low,
        unsigned int *        high
    )

    cdef int avl_get_span_by_two_keys (
        avl_tree *            tree,
        void *                key_a,
        void *                key_b,
        unsigned int *        low,
        unsigned int *        high
    )

    cdef int avl_verify (avl_tree * tree)

    cdef void avl_print_tree (
        avl_tree *            tree,
        avl_key_printer_fun_type key_printer
    )

    cdef avl_node * avl_get_successor (avl_node * node)

    cdef int avl_get_item_by_key_most (
        avl_tree *            tree,
        void *                key,
        void **               value_address
    )

    cdef int avl_get_item_by_key_least (
        avl_tree *            tree,
        void *                key,
        void **               value_address
    )
