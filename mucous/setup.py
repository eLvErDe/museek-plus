#!/usr/bin/python

"""To use this setup script to install Mucous:

        python setup.py install

"""

import sys
import os
import glob

from distutils.core import setup
from distutils.sysconfig import get_python_lib


if __name__ == '__main__' :
    LONG_DESCRIPTION = \
""" Mucous is a Curses interface for the Museek Daemon for the SoulSeek P2P Network
"""

    from pymucous.utils import Version

    setup(name                  = "mucous",
          version               = Version,
          license               = "GPL",
          description           = "Curses Interface for the SoulSeek Museek Daemon.",
          author                = "daelstorm",
          author_email          = "daelstorm@gmail.com",
          url                   = "http://www.museek-plus.org/wiki/mucous",
          packages              = [ 'museek', 'pymucous' ],
          scripts               = [ 'mucous' ],
	  data_files            = [('man/man1', ['mucous.1'])],
          long_description      = LONG_DESCRIPTION
         )

