# -*- Mode: Python; coding: iso-8859-1 -*-

from distutils.core import setup, Extension

src = ['AVLmodule.c']

ext_macros = []
#ext_macros = ['DEBUG_AVL']

setup (
    name         = "avl",
    version      = "2.1.3",
    description  = "AVL Tree Objects for Python",
    author       = 'Samual M Rushing',
    author_email = "[hidden]",
    license      = "BSD",
    url          = 'https://github.com/samrushing/avl',
    libraries    = [("avl", {"sources": ["avl.c"]})],
    ext_modules  = [
        Extension (
            'avl',
            src,
            define_macros=ext_macros,
            library_dirs=['.'],
            libraries=['avl']
        )
    ]
)
