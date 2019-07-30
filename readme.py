#! /usr/bin/env python
# -*- coding: utf-8 -*-
from __future__ import (absolute_import, division, print_function,
                        unicode_literals)

# Standard libraries.
import random

import avl

t = avl.newavl()
print(t)  # []
t.insert(50)
print(t)  # [50]
t.insert(45)
print(t)  # [45, 50]
t.remove(50)
print(t)  # [45]

t = avl.newavl()
for x in range(20):
    t.insert(random.randint(0, 100))
print(t)
# [3, 8, 12, 15, 15, 29, 32, 42, 44, 48, 50, 53, 55, 57, 69, 74, 75, 76, 79,
#  81, 88]
# if you've compiled in the debugging code (-DDEBUG_AVL)
#
# [Note: print_internal_structure() prints directly to stdout,
#     so it doesn't work in pythonwin, but will work with the
#  console-mode python.exe]

t.print_internal_structure()
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

# create a new tree from a list.
# Note: this sorts the list as a side-effect, if you don't
# want that, use a copy (i.e., 'l = avl.from_list(my_list[:])')

t = avl.newavl(range(30))
len(t)
# 30
# >>>

# You can address individual elements in the sequence:
# Note: you cannot assign to the items in the tree, as
# this might change the relative order of the keys.

t[25]
# 25
t[-1]
# 30

# You can take slices of the list. The default slice
# operation returns a new avl tree, not a list.
# [If you want a list, use the slice_as_list() method]

random_tree = avl.newavl([random.randint(0, 100) for _ in range(10)])
random_tree[3:8]
# [45, 87, 29, 73, 12]
type(_)
# <type 'AVL tree'>

# You can insert any python object:

t = avl.newavl(range(10))
t.insert('Hey, where will this go?')
t
# [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 'Hey, where will this go?']
# >>>

# is a certain element in the tree?

t.lookup(5)
# 5
try:
    t.lookup(34)
except KeyError:
    pass
else:
    raise
# Traceback (innermost last):
#   File "<stdin>", line 1, in ?
# KeyError: 34
t.has_key(5)  # noqa
# better
# 1
5 in t
t.has_key(34)  # noqa
# 0
# >>>
# better
34 in t

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
t1 = avl.tree([5, 8, 9])
t2 = avl.newavl(t)

# to concatenate two trees
t3 = t1 + t2
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

# 't.span()' returns a pair of indices into the tree that 'span' the
# given key or key pair.  For example:

t = avl.tree([1, 2, 3, 4, 5])
t
# [1,2,3,4,5]
t.span(4)
# (3,4)
t[3:4]
# [4]
t = avl.newavl([random.randint(0, 1000) for _ in range(10)])
# >>> t
# [15, 34, 371, 536, 659, 691, 714, 754, 847, 936]
# >>> t.span(200,500)
# (2, 3)
# >>> t[2:3]
# [371]
# >>> t.span(300,700)
# (2, 6)
# >>> t[2:6]
# [371, 536, 659, 691]
# >>>

# at_least() returns the first object comparing greater to or equal to
# the <key> argument.

t.at_least(400)
# 536

# at_most() returns the first object comparing less than or equal to
# the <key> argument.
t.at_most(800)
# 754

# ===========================================================================
# release 2.1.0 (Jun 2005)
# ===========================================================================

# Berthold Höllmann finally spurs me into doing the long-awaited merge.

# ===========================================================================
# release 2.1.1 (Jun 2005)
# ===========================================================================

# Lots of fixes/changes merged from various sources.

# Only one change in functionality:
#   insert_by_key() now returns the index of insertion.

# Thanks To:
#   Berthold Höllmann (modernization, 64-bit fixes, setup.py)
#   Charlie Kemp (another setup.py)
#   Paul Cameron (subtle bug when removing non-existent key )

# And From IronPort:
#   Martin Baker (made slicing match Python's, other changes)
#   Eric Huss (refcount touch-ups, lots of other work)
#   Sam Rushing

# ===========================================================================
# release 2.1.2 (Aug 2005)
# ===========================================================================

# Fixed a bug reported by Kenneth Duda:

z = avl.newavl(None, lambda x, y: cmp(x[0], y[0]))
z.insert('hello')
z.span('h')

# exceptions.SystemError: 'NULL object passed to Py_BuildValue'

# # ===========================================================================
# # release 2.1.3 (Sep 2005)
# # ===========================================================================

# Win32 compilation issue reported by Berthold Höllmann:

#   AVLmodule.c(855) : error C2099: initializer is not a constant
