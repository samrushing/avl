# -*- Mode: Python; coding: iso-8859-1 -*-
from __future__ import division, print_function, absolute_import

# Standard libraries.
from setuptools import Extension, setup

src = ['AVLmodule.c']

ext_macros = []
ext_macros = [('DEBUG_AVL', None)]

setup(
    name="avl",
    version="2.1.4",
    description="AVL Tree Objects for Python",
    author='Samual M Rushing',
    author_email="[hidden]",
    license="BSD",
    url='https://github.com/samrushing/avl',
    libraries=[("avl", {"sources": ["lib/avl.c"],
                        "include_dirs": ["./lib/"]})],
    ext_modules=[
        Extension(
            'avl',
            src,
            define_macros=ext_macros,
            include_dirs=["lib"],
        )
    ]
)
