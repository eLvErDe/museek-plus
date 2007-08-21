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
      ext_modules           = [ 
          Extension("_mucipherc", ["aes.c", "hexdigest.c", "md5.c", "sha.c", "sha256.c", "wraphelp.c", "mucipher.i"])
      ],
)
