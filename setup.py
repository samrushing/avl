# -*- Mode: Python; coding: iso-8859-1 -*-
from __future__ import absolute_import, division, print_function

from distutils.extension import Extension

# Third party libraries.
from Cython.Build import cythonize
# Standard libraries.
from setuptools import Extension, setup

src = ["AVLmodule.c"]

setup(
    name="avl",
    version="2.2.0",
    description="AVL Tree Objects for Python",
    author="Samual M Rushing",
    author_email="[hidden]",
    license="BSD",
    url="https://github.com/samrushing/avl",
    # libraries=[("avl", {"sources": ["lib/avl.c"],
    #                     "include_dirs": ["./lib/"]})],
    ext_modules=cythonize(
        [
            Extension(
                "avl",
                ["avl.pyx", "lib/avl_.c"],
                always_allow_keywords=True,
                include_dirs=["./lib/"],
                extra_compile_args=["-g"],
                extra_link_args=["-g"],
            )
        ],
        compiler_directives={
            "embedsignature": True,
            "profile": True,
            "c_string_type": str,
            "c_string_encoding": "utf-8",
            # "convert_range": True
        },
        gdb_debug=True,
    ),
)
