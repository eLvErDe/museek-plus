version = "0.0.1"

from distutils.core import setup
from distutils.extension import Extension

setup(name                  = "pymucipher",
      version               = version,
      license               = "GPL",
      description           = "Mucipher for python",
      author                = "Hyriand",
      author_email          = "hyriand@thegraveyard.org",
      url                   = "http://museek-plus.org/",
      long_description      = "PyMucipher (encryption for museek python clients) stand-alone package",
      py_modules            = ['mucipher', 'mucipherc'],
      # Workaround for swig/c++/distutils bug in python2.4
      # http://bugs.python.org/issue1479255
      # http://mail.python.org/pipermail/distutils-sig/2005-November/005387.html
      options               = {'build_ext':{'swig_opts':'-c++'}}, 
      ext_modules           = [ 
          Extension("_mucipherc", ["../aes.cpp", "../hexdigest.cpp", "../md5.cpp", "../sha.cpp", "../sha256.cpp", "../wraphelp.cpp", "mucipher.i"], swig_opts=["-c++"])
      ],
)
