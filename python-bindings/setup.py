version = "1.0.0"

from distutils.core import setup
from distutils.extension import Extension

setup(name                  = "museek-python-bindings",
      version               = version,
      license               = "GPL",
      description           = "Python bindings for museek clients",
      author                = "daelstorm",
      author_email          = "daelstorm@gmail.com",
      url                   = "http://museek-plus.org/",
      long_description      = "messages and driver (museek networking bindings)",
      packages              = ['museek'],
)
