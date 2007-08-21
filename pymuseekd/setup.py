version = "1.0.0"

from distutils.core import setup
from distutils.extension import Extension

setup(name                  = "python-museekd",
      version               = version,
      license               = "GPL",
      description           = "A collection of python tools for museekd",
      author                = "daelstorm",
      author_email          = "daelstorm@gmail.com",
      url                   = "http://museek-plus.org/",
      long_description      = "messages and driver (museek networking bindings), mulog (chat logging), musetup and musetup-gtk (config tools), museekcontrol (commandline client/tool), museekchat (curses chat client)",
      packages              = ['museek'],
      scripts               = ['mulog', 'musetup', 'musetup-gtk', 'museekchat', 'museekcontrol', 'musirc.py'],
      data_files            = [('man/man1', ['mulog.1', 'musetup.1', 'musetup-gtk.1', 'museekcontrol.1'])],

)
