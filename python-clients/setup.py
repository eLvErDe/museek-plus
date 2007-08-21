version = "1.0.0"

from distutils.core import setup
from distutils.extension import Extension

setup(name                  = "museek-python-clients",
      version               = version,
      license               = "GPL",
      description           = "A small collection of python clients for museekd",
      author                = "daelstorm",
      author_email          = "daelstorm@gmail.com",
      url                   = "http://museek-plus.org/",
      long_description      = "mulog (chat logging), museekcontrol (commandline client/tool), museekchat (curses chat client), musirc (irc gateway)",
      scripts               = ['mulog', 'museekchat', 'museekcontrol', 'musirc.py'],
      data_files            = [('man/man1', ['mulog.1', 'museekcontrol.1'])],

)
