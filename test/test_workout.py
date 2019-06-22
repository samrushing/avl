# -*- Mode: Python -*-
# give the avl module a good workout.
from __future__ import (absolute_import, division, print_function,
                        unicode_literals)

import random
# Standard libraries.
import sys
import time

import pytest

import avl


class timer:
    def __init__(self):
        self.start = time.time()

    def end(self):
        print("%f seconds" % (time.time() - self.start))


def generate_test_numbers(n=10000):
    return [random.randint(0, 1000000) for x in range(n)]


def fill_up(tree, nums):
    print("filling up...")
    t = timer()
    for num in nums:
        tree.insert(num)
    t.end()


def random_indices_tree(length):
    t = avl.newavl()
    # build a 'list' of indices
    [t.insert(num) for num in range(length)]
    t2 = avl.newavl()
    for x in range(length):
        i = random.choice(t)
        t2.insert((x, i))
        t.remove(i)
    # return the tree as a list
    return [x[1] for x in t2]


def random_indices_list(length):
    choices = range(length)
    result = range(length)
    for i in range(length):
        n = random.choice(choices)
        result[i] = n
        choices.remove(n)
    return result


# we're careful to avoid timing the random module.


def empty(tree):
    values = [tree[i] for i in random_indices_tree(len(tree))]
    print("emptying the tree...")
    t = timer()
    for value in values:
        tree.remove(value)
    t.end()


def random_slice(length):
    left = random.randint(0, length)
    right = random.randint(0, length)
    if left > right:
        return (right, left)
    return (left, right)


def slice_test(tree, num_slices=100):
    print("computing {} slices...".format(num_slices))
    l = len(tree)
    slices = [random_slice(l) for i in range(num_slices)]
    print("slicing %d times..." % num_slices)
    t = timer()
    for left, right in slices:
        slice = tree[left:right]
    t.end()


def do_test(n):
    tree = avl.newavl()
    print("generating random numbers...")
    t = timer()
    nums = generate_test_numbers(n)
    t.end()
    fill_up(tree, nums)
    slice_test(tree)
    empty(tree)


# def test_workout():
#     # print(sys.argv)
#     # if len(sys.argv) > 1:
#     #     iterations = int(sys.argv[1])
#     #     if len(sys.argv) > 2:
#     #         test_size = int(sys.argv[2])
#     #     else:
#     #         test_size = 10000
#     # else:
#     iterations = 100
#     test_size = 10000
#     for i in range(iterations):
#         print("test %d" % i)
#         do_test(test_size)


@pytest.mark.benchmark(
    min_rounds=1000,
    warmup=False
)
def test_timeit(benchmark):
    @benchmark
    def result():
        do_test(10000)
