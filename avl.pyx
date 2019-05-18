# -*- coding: utf-8 -*-

# Copyright (C) 1995-1997 by Sam Rushing <rushing@nightmare.com>
# Copyright (C) 2005 by Germanischer Lloyd AG
# Copyright (C) 2001-2005 by IronPort Systems, Inc.
# Copyright (C) 2019 by Berthold Höllmann
#
#                         All Rights Reserved
#
# Permission to use, copy, modify, and distribute this software and
# its documentation for any purpose and without fee is hereby
# granted, provided that the above copyright notice appear in all
# copies and that both that copyright notice and this permission
# notice appear in supporting documentation, and that the name of Sam
# Rushing not be used in advertising or publicity pertaining to
# distribution of the software without specific, written prior
# permission.
#
# SAM RUSHING DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
# INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
# NO EVENT SHALL SAM RUSHING BE LIABLE FOR ANY SPECIAL, INDIRECT OR
# CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
# OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
# NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
# CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

# cython: language_level=2

from __future__ import (absolute_import, division, print_function,
                        unicode_literals)

import cython

from cpython.int cimport PyInt_Check
from cpython.list cimport (PyList_Sort, PyList_Check, PyList_GetSlice,
                           PyList_Size)
from cpython.object cimport PyObject_RichCompareBool, Py_EQ, Py_LT
from cpython.ref cimport PyObject, Py_DECREF, Py_XDECREF, Py_XINCREF
from cpython.slice cimport PySlice_Check, PySlice_GetIndices
from cpython.unicode cimport PyUnicode_AsASCIIString, PyUnicode_GET_SIZE
from cpython.version cimport PY_MAJOR_VERSION

from libc.string cimport strcpy

cimport avl


__version__ = VERSION


cdef unicode _text(s):
    if type(s) is unicode:
        # Fast path for most common case(s).
        return <unicode>s

    elif PY_MAJOR_VERSION < 3 and isinstance(s, bytes):
        # Only accept byte strings as text input in Python 2.x, not in Py3.
        return (<bytes>s).decode('ascii')

    elif isinstance(s, unicode):
        # We know from the fast path above that 's' can only be a subtype here.
        # An evil cast to <unicode> might still work in some(!) cases,
        # depending on what the further processing does.  To be safe,
        # we can always create a copy instead.
        return unicode(s)

    else:
        raise TypeError("Could not convert to unicode.")


cdef int avl_key_compare_for_python(void * compare_arg, void * a, void * b):
    cdef object self = <object>compare_arg

    if not self.compare_function:
        if PyObject_RichCompareBool(<object>a, <object>b, Py_LT):
            return -1
        elif PyObject_RichCompareBool(<object>a, <object>b, Py_EQ):
            return 0
        return 1
    return self.compare_function(<object>a, <object>b)


cdef int tree_from_list(object list,
                        avl.avl_node * parent, avl.avl_node ** address,
                        unsigned int low, unsigned int high):
    cdef unsigned int midway = ((high - low) // 2) + low
    cdef int left_height, right_height
    cdef avl_node * new_node
    if low == high:
        address[0] = NULL
        return 1
    else:
        item = list[midway]

        new_node = avl.avl_new_avl_node (<void*>item, parent)
        if not new_node:
            return -1
        address[0] = new_node
        Py_XINCREF(< PyObject*>item)
        AVL_SET_RANK(new_node, (midway-low)+1)
        left_height = tree_from_list(
            list, new_node, &(new_node.left), low, midway)
        if left_height < 0:
            return -1
        right_height = tree_from_list(
            list, new_node, &(new_node.right), midway+1, high)
        if right_height < 0:
            return -1
        if left_height > right_height:
            AVL_SET_BALANCE(new_node, -1)
            return left_height + 1
        elif right_height > left_height:
            AVL_SET_BALANCE(new_node, +1)
            return right_height + 1
        AVL_SET_BALANCE(new_node, 0)
        return left_height + 1


# remove_by_key's free_key_fun callback
cdef int avl_tree_key_free_fun(void * key):
    Py_DECREF(<object>key)
    return 0


cdef int tree_from_tree(avl_node ** node,
                        avl_node * parent,
                        avl_node ** address,
                        unsigned int low,
                        unsigned int high):
    cdef unsigned int midway = ((high - low) // 2) + low
    cdef object item
    cdef avl_node * new_node
    cdef long int left_height, right_height

    if low == high:
        address[0] = NULL
        return 1
    else:
        new_node = avl.avl_new_avl_node (NULL, parent)
        if not new_node:
            return -1
        address[0] = new_node
        avl.AVL_SET_RANK(new_node, (midway-low)+1)
        left_height = tree_from_tree(
            node, new_node, &(new_node[0].left), low, midway)
        if left_height < 0:
            return -1

        item = <object>node[0][0].key
        Py_XINCREF(< PyObject*>item)
        new_node[0].key = <void*> item
        node[0] = avl.avl_get_successor(node[0])

        right_height = tree_from_tree(
            node, new_node, &(new_node[0].right), midway+1, high)
        if right_height < 0:
            return -1
        if left_height > right_height:
            avl.AVL_SET_BALANCE(new_node, -1)
            return left_height + 1
        elif right_height > left_height:
            avl.AVL_SET_BALANCE(new_node, +1)
            return right_height + 1
        else:
            AVL_SET_BALANCE(new_node, 0)
            return left_height + 1


cdef int avl_copy_avl_node(avl_node * source_node,
                           avl_node * dest_parent,
                           avl_node ** dest_node) except *:
    cdef avl_node * new_node
    new_node = avl.avl_new_avl_node(
        source_node[0].key, dest_parent)
    if not new_node:
        raise MemoryError("Cannot allocate node")
    Py_XINCREF (<PyObject*>new_node[0].key)
    new_node[0].rank_and_balance = source_node[0].rank_and_balance
    if source_node[0].left:
        if avl_copy_avl_node(source_node[0].left,
                             new_node,
                             &(new_node[0].left)):
            return -1
    else:
        new_node[0].left = NULL
    if source_node[0].right:
        if avl_copy_avl_node(source_node[0].right,
                             new_node,
                             &(new_node[0].right)):
            return -1
    else:
        new_node[0].right = NULL
    dest_node[0] = new_node
    return 0


cdef void avl_copy_avl_tree(tree source, tree dest) except *:
    if source.tree[0].length:
        if avl_copy_avl_node(
                source.tree[0].root[0].right,
                dest.tree[0].root,
                &(dest.tree[0].root[0].right)):
            avl.avl_free_avl_tree(source.tree, avl_tree_key_free_fun)
            raise Exception(
                "something went amiss whilst building the tree!")
    dest.tree[0].length = source.tree[0].length


cdef int avl_tree_key_printer(char * buffer, void * key):
    cdef object repr_string
    cdef int length

    repr_string = _text(repr(<object>key))
    if repr_string:
        try:
            strcpy(buffer, PyUnicode_AsASCIIString(repr_string))
        except Exception as ex:
            print("As ASCII: ", ex.message, repr_string)
        try:
            length = PyUnicode_GET_SIZE(repr_string)
        except Exception as ex:
            print("get size: ", ex.message, repr_string)
        return length
    else:
        strcpy(buffer, "<couldn't print node>")
        return 21


cdef class tree:
    cdef avl.avl_tree * tree
    cdef avl.avl_node * node_cache
    cdef Py_ssize_t cache_index
    cpdef readonly object compare_function

    def __cinit__(self, args=None, object compare_function=None):
        cdef object tmp_list

        cdef Py_ssize_t low = 0, length
        self.tree = avl_new_avl_tree(avl_key_compare_for_python, <void*>self)
        if not self.tree:
            raise MemoryError("Cannot allocate tree")
        self.tree[0].root = avl.avl_new_avl_node(NULL, <avl.avl_node*>NULL)

        self.node_cache = NULL
        self.cache_index = 0
        self.compare_function = compare_function

        if args is None:
            pass
        elif PyList_Check(args):
            length = PyList_Size(args)
            tmp_list = PyList_GetSlice(args, low, length)
            if PyList_Sort(tmp_list) == -1:
                avl.avl_free_avl_tree(self.tree, avl_tree_key_free_fun)
                raise
            if (tree_from_list(
                    tmp_list,
                    self.tree[0].root,
                    &(self.tree[0].root[0].right),
                    0,
                    length) < 0):
                avl.avl_free_avl_tree(self.tree, avl_tree_key_free_fun)
                raise Exception(
                    "something went amiss whilst building the tree!")
            self.tree[0].length = length
        elif isinstance(args, tree):
            self.compare_function = (<tree>args).compare_function
            avl_copy_avl_tree(args, self)
        else:
            raise TypeError("unsupported argument {}".format(args))

    def __dealloc__(self):
        avl.avl_free_avl_tree(self.tree, avl_tree_key_free_fun)

    def __str__(self):
        cdef object s = "["
        cdef object comma = ", "
        cdef avl_node * node
        cdef Py_ssize_t i

        if self.tree[0].length == 0:
            return "[]"

        # find leftmost node
        node = self.tree[0].root[0].right
        while node[0].left != NULL:
            node = node[0].left

        for i in range(self.tree[0].length):
            if i > 0:
                s += comma
            s += str(<object>node[0].key)
            node = avl_get_successor(node)

        s += "]"
        return s

    def __repr__(self):
        cdef object s
        cdef object comma
        cdef avl_node * node
        cdef Py_ssize_t i

        if not self.tree[0].length:
            return "tree([], {!r})".format(self.compare_function)

        s = "tree(["
        comma = ", "
        # find leftmost node
        node = self.tree[0].root[0].right
        while (node[0].left):
            node = node[0].left

        for i in range(self.tree[0].length):
            if i > 0:
                s += comma
            s += repr(<object>node[0].key)
            node = avl_get_successor(node)

        s += "], {!r})".format(self.compare_function)
        return s

    def __len__(self):
        return <int>self.tree[0].length

    def __getitem__(self, object arg):
        cdef void * value
        cdef Py_ssize_t index
        cdef Py_ssize_t i
        cdef unsigned int m
        cdef Py_ssize_t ilow, ihigh, step
        cdef avl_node * node

        # Python takes care of negative indices for us, so if
        # i is negative, that is an error.
        if PyInt_Check(arg):
            i = arg
            # range-check the index
            if i < 0:
                index = self.tree[0].length + i
            else:
                index = <unsigned int>i
            if (index >= self.tree[0].length):
                raise IndexError("tree index out of range (too large)")
            if (((i < 0) and (-i > self.tree[0].length))):
                raise IndexError("tree index out of range (too small)")

            # get the python object, and store in <value>

            # index cache
            if self.node_cache and index == (self.cache_index + 1):
                self.node_cache = avl_get_successor(self.node_cache)
                self.cache_index += 1
                value = self.node_cache[0].key
            if (avl_get_item_by_index(self.tree, index, &value) != 0):
                raise Exception("error while accessing item")
            return <object>value
        elif PySlice_Check(arg):
            if (PySlice_GetIndices(
                    arg, self.tree[0].length, &ilow, &ihigh, &step) < 0):
                raise IndexError("invalid slice")
            if step != 1:
                raise IndexError("slice with step not supported")

            # We are attempting to match Python slicing on list objects,
            # which is incredibly lenient. Basically, it is impossible to
            # get an exception.

            # By the time we are called, the Python internals have
            # already added the length of self to ilow and ihigh if they are
            # negative. However, the values can still be negative or too large.
            if ilow < 0:
                ilow = 0
            if ihigh < 0:
                ihigh = 0
            if ihigh > self.tree[0].length:
                ihigh = self.tree[0].length

            new_tree = tree(self.compare_function)

            # return empty tree in this degenerate case:
            if ihigh <= ilow:
                return new_tree

            # locate node <ilow>
            node = self.tree[0].root[0].right
            m = ilow + 1
            while True:
                if m < avl.AVL_GET_RANK(node):
                    node = node[0].left
                elif m > avl.AVL_GET_RANK(node):
                    m -= avl.AVL_GET_RANK(node)
                    node = node[0].right
                else:
                    break

            if tree_from_tree(
                    &node,
                    new_tree.tree[0].root,
                    &(new_tree.tree[0].root[0].right),
                    0,
                    ihigh - ilow) < 0:
                raise Exception(
                    "something went amiss whilst building the tree!")

            new_tree.tree[0].length = ihigh - ilow

            return new_tree

        raise ValueError("index is neiter int nor slice")

    def __add__(self, tree other):
        cdef tree self_copy = tree(self.compare_function)
        cdef unsigned int other_node_counter = other.tree[0].length
        cdef avl_node * other_node
        cdef unsigned int ignore

        avl_copy_avl_tree(self, self_copy)
        if not self_copy:
            raise MemoryError()

        if other_node_counter:
            # find the leftmost node of other
            other_node = other.tree[0].root[0].right
            while other_node[0].left:
                other_node = other_node[0].left

            # iterate over the items in other, inserting
            # them into self_copy
            while other_node_counter:
                other_node_counter -= 1
                Py_XINCREF(< PyObject*>other_node[0].key)
                if avl_insert_by_key(
                        self_copy.tree,
                        <void*>other_node[0].key,
                        &ignore):
                    del(self_copy)
                    raise Exception("concatiation failed")
                other_node = avl_get_successor(other_node)
        return self_copy

    cpdef insert(self, val):
        "Insert an item into the tree"
        cdef unsigned int index = 0
        Py_XINCREF(<PyObject*>val)
        if (avl.avl_insert_by_key(self.tree, <void*>val, &index) != 0):
            Py_DECREF(val)
            raise Exception("error while inserting item")
        else:
            self.node_cache = NULL
            return index

    cpdef remove(self, val):
        "Remove an item from the tree"
        if (avl.avl_remove_by_key(
                self.tree, <void*>val, avl_tree_key_free_fun) != 0):
            raise Exception("error while removing item")
        else:
            self.node_cache = NULL
        return None

    cpdef object lookup(self, key):
        "Return the first object comparing equal to the <key> argument"
        cdef PyObject * return_value
        cdef int result

        if self.tree[0].length:
            result = avl.avl_get_item_by_key(
                self.tree, <void*>key, <void**>cython.address(return_value))
            if result == 0:
                # success
                Py_XINCREF (<PyObject*>return_value)
                return <object>return_value
        raise KeyError(key)

    cpdef bint has_key(self, object key):
        cdef PyObject * return_value
        "Does the tree contain an item comparing equal to <key>?"
        if self.tree[0].length:
            result = avl_get_item_by_key(
                self.tree, <void*>key, <void**>cython.address(return_value))
            if result == 0:
                # success
                return True
        return False

    cpdef tuple span(self, low_key, high_key=None):
        """t.span (key) => (low, high)
Returns a pair of indices (low, high) that span the range of <key>"""

        cdef unsigned int low, high
        cdef int result

        if not self.tree[0].length:
            return (0, 0)
        # only one key was specified
        if high_key is None:
            result = avl_get_span_by_key(
                self.tree,
                <void*>low_key,
                &low,
                &high)
            if result == 0:
                # success
                return (low, high)
            else:
                raise Exception("error while locating key span")
        # they specified two keys
        result = avl_get_span_by_two_keys(
            self.tree,
            <void*>low_key,
            <void*>high_key,
            &low,
            &high)
        if result == 0:
            # success
            return (low, high)
        else:
            raise Exception("error while locating key span")

    cpdef at_least(self, key_val):
        """Return the first object comparing greater to or equal to the <key> 
argument"""
        cdef PyObject * return_value
        cdef int result

        if self.tree[0].length:
            result = avl_get_item_by_key_least(
                self.tree,
                <void*>key_val,
                <void**>&return_value)
            if (result == 0):
                # success
                Py_XINCREF(return_value)
                return <object>return_value
        raise KeyError(key_val)

    cpdef at_most(self, key_val):
        """Return the first object comparing less than or equal to the <key>
argument"""
        cdef PyObject * return_value
        cdef int result

        if self.tree[0].length:
            result = avl_get_item_by_key_most(
                self.tree,
                <void*>key_val,
                <void**>&return_value)
            if result == 0:
                # success
                Py_XINCREF(return_value)
                return <object>return_value
        raise KeyError(key_val)

    cpdef bint verify(self):
        """Verify the internal structure of the AVL tree (testing only)"""
        return avl_verify(self.tree) == 0

    cpdef print_internal_structure(self):
        avl_print_tree(self.tree, avl_tree_key_printer)


def newavl(arg=None, compare_function=None):
    """With no arguments, returns a new and empty tree.
Given a list, it will return a new tree containing the elements
  of the list, and will sort the list as a side-effect
Given a tree, will return a copy of the original tree
An optional second argument is a key-comparison function"""
    return tree(arg, compare_function)
