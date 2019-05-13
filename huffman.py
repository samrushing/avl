# -*- Mode: Python -*-
from __future__ import (
    division, print_function, absolute_import, unicode_literals)

import avl


def analyze_file(filename):
    f = {}
    with open(filename) as fle:
        data = fle.read()
        for ch in data:
            if ch in f:
                f[ch] += 1
            else:
                f[ch] = 1
    f = f.items()
    return [(x[1], x[0]) for x in f]


def huffman_tree(f):
    t = avl.newavl(f)
    while len(t) > 1:
        w1, c1 = n1 = t[0]
        w2, c2 = n2 = t[1]
        t.remove(n1)
        t.remove(n2)
        t.insert(((w1+w2), (c1, c2)))
    return t[0]


def walk(r, t, s=''):
    if isinstance(t, basestring):
        r.append((s, t))
    else:
        walk(r, t[0], s+'0')
        walk(r, t[1], s+'1')


def huffman_code(t):
    r = []
    walk(r, t[1])
    r = [(len(x[0]), x) for x in r]
    r.sort()
    return [x[1] for x in r]


if __name__ == '__main__':
    import sys
    code = huffman_code(huffman_tree(analyze_file(sys.argv[1])))
    for s, ch in code:
        print('%08s %s' % (repr(ch), s))
