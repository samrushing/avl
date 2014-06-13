# -*- Mode: Python; coding: iso-8859-1 -*-
"""
distutil setup script
"""

from distutils.core import setup, Extension

# set file generation umask, so that group gets write permissions on
# generated files
import os
os.umask(2)

src     = ['AVLmodule.c']

ext_macros = []
#ext_macros = ['DEBUG_AVL']

setup (name         = "avl",
       version      = "2.1.3",
       description  = "AVL Tree Objects for Python",
       author       = 'Samual M Rushing',
       author_email = "[hidden]",
       maintainer   = "Berthold Höllmann",
       maintainer_email = "hoel@gl-group.com",
       license      = "BSD",
       url          = 'http://www.nightmare.com/squirl/python-ext/avl/',
       libraries    = [("avl", {"sources": ["avl.c"]})],
       ext_modules  = [Extension('avl', src,
                                 define_macros=ext_macros)])

# Local Variables:;
# compile-command:"python setup.py build";
# End:;
