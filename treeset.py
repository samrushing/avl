# -*- Mode: Python -*-
from __future__ import (absolute_import, division, print_function,
                        unicode_literals)

import avl


# set operations on avl trees.
def intersection(a, b):
    ta = avl.newavl(a)
    result = avl.newavl()
    for x in b:
        if x in ta:
            result.insert(x)
            ta.remove(x)
    return result


def union(a, b):
    result = avl.newavl(a)
    for key in b:
        if key not in a:
            result.insert(key)
    return result


def difference(a, b):
    result = avl.newavl(a)
    for key in b:
        if key in a:
            result.remove(key)
    return result


# an abstract set implementation based on avl trees
class avl_set:
    "accepts an optional list or set as an initializer"

    def __init__(self, set=None):
        if set is None:
            self.items = avl.newavl()
        elif isinstance(set, list):
            self.items = avl.newavl(set[:])
        elif isinstance(set, avl_set):
            self.items = avl.newavl(set.items)
        else:
            self.items = avl.newavl(set)

    def add(self, item):
        if item not in self.items:
            self.items.insert(item)
        else:
            raise ValueError("item already present in set")

    def remove(self, item):
        if item in self.items:
            self.items.remove(item)
        else:
            raise ValueError("item not in set")

    def has_item(self, item):
        return item in self.items

    def __contains__(self, item):
        return item in self.items

    # addition ==  union
    def __add__(self, other):
        return self.__class__(union(self.items, other.items))

    # subtraction == difference
    def __sub__(self, other):
        return self.__class__(difference(self.items, other.items))

    # multiplication == intersection
    def __mul__(self, other):
        return self.__class__(intersection(self.items, other.items))

    # comparison: expresses the <subset> relation, a <= b
    def __cmp__(self, other):
        # is <a> a proper subset of <b> ?
        # -1 == no
        # +1 == yes
        #  0 == they are equal (improper)
        if len(self.items) > len(other.items):
            return -1
        for x in self.items:
            if key not in other.items:
                return -1
        if len(self.items) == len(other.items):
            return 0
        else:
            return 1

    def __repr__(self):
        return '{' + repr(self.items)[1:-1] + '}'
