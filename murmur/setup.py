#!/usr/bin/python

"""To use this setup script to install Murmur:

        python setup.py install

"""

import sys
import os
import glob

from distutils.core import setup
from distutils.sysconfig import get_python_lib

# Local functions
from setup_inc import *

if __name__ == '__main__' :
    LONG_DESCRIPTION = \
""" Murmur is a PyGTK2 Client for the Museek Daemon for the SoulSeek P2P Network
"""


    setup(name                  = "murmur",
          version               = getVersion(),
          license               = "GPL",
          description           = "PyGTK2 Interface for the SoulSeek Museek Daemon.",
          author                = "daelstorm",
          author_email          = "daelstorm@gmail.com",
          url                   = "http://thegraveyard.org/daelstorm/murmur.php",
          packages              = [ 'pymurmur' ],
          scripts               = [ 'murmur' ],
	  data_files            = [('share/pixmaps/', ['images/murmur-64px.png']), ('share/applications/', ['files/murmur.desktop'])],
	  data_files            = [('man/man1', ['murmur.1'])],
          long_description      = LONG_DESCRIPTION
         )

