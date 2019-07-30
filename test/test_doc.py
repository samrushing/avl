#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""test cases from documentation.
"""
from __future__ import (absolute_import, division, print_function,
                        unicode_literals)

import sys

import pytest

# DNV GL libraries.
import avl

if sys.version_info > (3, ):
    unicode = str

    def cmp(a, b):
        if a < b:
            return -1
        if a == b:
            return 0
        return 1


# Usage:
def test_1():
    t = avl.newavl()
    assert str(t) == "[]"


def test_1_repr():
    t = avl.newavl()
    assert repr(t) == "tree([], None)"


def test_2():
    t = avl.newavl()
    t.insert(50)
    assert str(t) == "[50]"


def test_2_repr():
    t = avl.newavl()
    t.insert(50)
    assert repr(t) == "tree([50], None)"


def test_3():
    t = avl.newavl()
    t.insert(50)
    t.insert(45)
    assert str(t) == "[45, 50]"


def test_4():
    t = avl.newavl()
    t.insert(50)
    t.insert(45)
    t.remove(50)
    assert str(t) == "[45]"


@pytest.fixture
def random_tree():
    return avl.newavl(
        [25, 26, 28, 30, 40, 42, 48, 50, 51, 58, 62, 76, 76, 79, 81, 85, 91,
         91, 91, 99])


def test_5(random_tree):

    assert str(random_tree) == (
        "[25, 26, 28, 30, 40, 42, 48, 50, 51, 58, 62, "
        "76, 76, 79, 81, 85, 91, 91, 91, 99]"
    )


# if you've compiled in the debugging code (-DDEBUG_AVL)
#
# [Note: print_internal_structure() prints directly to stdout,
#   so it doesn't work in pythonwin, but will work with the
#   console-mode python.exe]
# >>> t.print_internal_structure()
#                                        +-[- 88 001]
#                           +-[/ 81 001]-|
#              +-[\ 79 008]-|
#              |            |                         +-[- 76 001]
#              |            |            +-[- 75 002]-|
#              |            |            |            +-[- 74 001]
#              |            +-[- 69 004]-|
#              |                         |            +-[- 57 001]
#              |                         +-[- 55 002]-|
#              |                                      +-[- 53 001]
# +-[- 50 011]-|
#              |                                      +-[- 48 001]
#              |                         +-[/ 44 001]-|
#              |            +-[/ 42 002]-|
#              |            |            +-[- 32 001]
#              +-[- 29 006]-|
#                           |                        +-[- 15 001]
#                           |           +-[- 15 002]-|
#                           |           |            +-[- 12 001]
#                           +-[/ 8 002]-|
#                                       +-[- 3 001]
# [now isn't that pretty?.]
# FYI, the leftmost char in each node represents the 'balance factor'
# for that node, whether the tree is heavy on the left ('\'), right
# ('/'), or not at all ('-').  the middle item is 'repr(key)', and the
# final number is the 'rank' of the node, used internally to quickly
# locate a node of a certain index - it's the number of nodes in the
# left subtree, plus one.
def test_print_tree(random_tree):
    print()
    random_tree.print_internal_structure()


def test_verify(random_tree):
    assert random_tree.verify()


# create a new tree from a list.
# Note: this sorts the list as a side-effect, if you don't
# want that, use a copy (i.e., 'l = avl.from_list(my_list[:])')
def test_6():
    t = avl.newavl([i for i in range(30)])
    assert len(t) == 30


# You can address individual elements in the sequence:
# Note: you cannot assign to the items in the tree, as
# this might change the relative order of the keys.
def test_7():
    t = avl.newavl([i for i in range(30)])
    assert t[25] == 25


def test_7_a():
    t = avl.newavl([i for i in range(30)][::2])
    assert t[12] == 24


def test_8():
    t = avl.newavl([i for i in range(30)])
    assert t[-1] == 29


# You can take slices of the list. The default slice
# operation returns a new avl tree, not a list.
# [If you want a list, use the slice_as_list() method]
def test_9(random_tree):
    assert str(random_tree[3:8]) == "[30, 40, 42, 48, 50]"


def test_10(random_tree):
    assert isinstance(random_tree[3:8], type(avl.newavl()))


@pytest.fixture
def mixed_tree():
    def cmpfun(a, b):
        if isinstance(a, unicode) or isinstance(b, unicode):
            return cmp(str(a), str(b))
        return cmp(a, b)

    t = avl.newavl([i for i in range(10)], cmpfun)
    t.insert("Hey, where will this go?")
    return t


# You can insert any python object:
def test_11(mixed_tree):
    assert str(mixed_tree) == (
        "[0, 1, 2, 3, 4, 5, 6, 7, 8, 9, Hey, where will this go?]"
    )


def test_11_repr(mixed_tree):
    if sys.version_info < (3, ):
        assert repr(mixed_tree)[:86] == (
            "tree([0, 1, 2, 3, 4, 5, 6, 7, 8, 9, u'Hey, where will this "
            "go?'], <function cmpfun at "
        )
    else:
        assert repr(mixed_tree)[:-16] == (
            "tree([0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 'Hey, where will this "
            "go?'], <function mixed_tree.<locals>.cmpfun at "
        )


# is a certain element in the tree?
def test_12(mixed_tree):
    assert mixed_tree.lookup(5) == 5


def test_13(mixed_tree):
    with pytest.raises(KeyError):
        mixed_tree.lookup(34)


def test_14_a(mixed_tree):
    assert mixed_tree.has_key(5)  # noqa


def test_14_b(mixed_tree):
    assert 5 in mixed_tree


def test_15_a(mixed_tree):
    assert not mixed_tree.has_key(34)  # noqa


def test_15_b(mixed_tree):
    assert 34 not in mixed_tree


# lookup is a little tricky, because it returns the first object in the
# tree that compares equal to the key you give.  For builtin objects,
# the builtin compare is used to order the tree, but by using your own
# objects with your own __cmp__ functions, you can do all sorts of
# interesting things.

# *** -------------------- Note --------------------***
# It is important that you not directly change an object stored in an
# avl tree in a way that would change its ordered position.  If you need
# to do this, first remove() the item, change it, then reinsert it!
# This is still very fast, as both operations are O(log n)

# The results of such an illegal modification are unspecified, and are
# likely to crash the program.  The only way to completely avoid this
# problem is to use only immutable types.  This restriction seemed too
# severe to enforce.  So be careful out there!
# *** -------------------- Note --------------------***

# to copy a tree:
def test_copy(mixed_tree):
    t2 = avl.newavl(mixed_tree)
    assert t2 is not mixed_tree
    assert str(mixed_tree) == (
        "[0, 1, 2, 3, 4, 5, 6, 7, 8, 9, Hey, where will this go?]"
    )
    assert str(t2) == (
        "[0, 1, 2, 3, 4, 5, 6, 7, 8, 9, Hey, where will this go?]"
    )
    t2 = None


# to concatenate two trees
def test_concatenate():
    t1 = avl.newavl([i for i in range(5)])
    t2 = avl.newavl([i for i in range(6, 15, 2)])
    assert str(t1 + t2) == "[0, 1, 2, 3, 4, 6, 8, 10, 12, 14]"


# Note: this is the equivalent of creating a
#  copy of t1, and then inserting all the elements
#  of t2 in turn.

# the 'repeat' operation (t1 * 5) is currently undefined
# drop me a line if you need it. (and describe what you think
# it should do)

# ===========================================================================
# release 2.0 (february 1997)
# ===========================================================================
# Major changes to the C library have been applied for this version.
# The comparison function is now a member of the avl_tree structure,
# instead of being passed in with all of the API functions.  An extra
# 'compare_arg' pointer is included to allow attaching 'state' to the
# compare function - this is a bit ugly, but was necessary to cleanly
# interface to python.

# Several new functions are visible from python, including two from
# David Ascher <david_ascher@brown.edu> (at_least() and at_most()).


@pytest.fixture
def t():
    return avl.newavl([1, 2, 3, 4, 5])


# 't.span()' returns a pair of indices into the tree that 'span' the
# given key or key pair.  For example:
def test_span(t):
    assert t.span(4) == (3, 4)


def test_splice(t):
    assert str(t[3:4]) == "[4]"


@pytest.fixture
def random_tree2():
    return avl.tree([864, 394, 776, 911, 430, 41, 265, 988, 523, 497])


def test_span_1(random_tree2):
    assert random_tree2.span(300, 400) == (1, 2)


def test_span_1(random_tree2):
    assert str(random_tree2[1:2]) == "[265]"


def test_span_2(random_tree2):
    assert random_tree2.span(200, 500) == (1, 5)


def test_span_3(random_tree2):
    assert str(random_tree2[1:5]) == "[265, 394, 430, 497]"


# at_least() returns the first object comparing greater to or equal to
# the <key> argument.
def test_at_least(random_tree2):
    assert random_tree2.at_least(400) == 430


# at_most() returns the first object comparing less than or equal to
# the <key> argument.
def test_at_most(random_tree2):
    assert random_tree2.at_most(800) == 776


def test_2_1_3():
    z = avl.newavl(None, lambda x, y: cmp(x[0], y[0]))
    z.insert('hello')
    z.span('h')
