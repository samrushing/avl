# -*- Mode: Python; coding: iso-8859-1 -*-
from __future__ import absolute_import, division, print_function

from distutils.core import Extension, setup

# Third party libraries.
from Cython.Build import cythonize

VERSION = "2.2.1"

setup(
    name="avl",
    version=VERSION,
    description="AVL Tree Objects for Python",
    author="Samual M Rushing",
    author_email="[hidden]",
    license="BSD",
    url="https://github.com/samrushing/avl",
    libraries=[("avl", {"sources": ["avl.c"]})],
    ext_modules=cythonize(
        [
            Extension(
                "avl",
                ["avl_module.pyx"],
                include_dirs=["./lib/"],
                # extra_compile_args=["-g"],
                # extra_link_args=["-g"],
            )
        ],
        compiler_directives={
            "embedsignature": True,
            # "profile": True,
            "c_string_type": "str",
            "c_string_encoding": "utf-8",
        },
        compile_time_env={"VERSION": VERSION},
        gdb_debug=True,
    ),
)
