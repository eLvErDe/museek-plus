# This file was created automatically by SWIG 1.3.27.
# Don't modify this file, modify the SWIG interface instead.

import _mucipherc

# This file is compatible with both classic and new-style classes.
def _swig_setattr_nondynamic(self,class_type,name,value,static=1):
    if (name == "this"):
        if isinstance(value, class_type):
            self.__dict__[name] = value.this
            if hasattr(value,"thisown"): self.__dict__["thisown"] = value.thisown
            del value.thisown
            return
    method = class_type.__swig_setmethods__.get(name,None)
    if method: return method(self,value)
    if (not static) or hasattr(self,name) or (name == "thisown"):
        self.__dict__[name] = value
    else:
        raise AttributeError("You cannot add attributes to %s" % self)

def _swig_setattr(self,class_type,name,value):
    return _swig_setattr_nondynamic(self,class_type,name,value,0)

def _swig_getattr(self,class_type,name):
    method = class_type.__swig_getmethods__.get(name,None)
    if method: return method(self)
    raise AttributeError,name

import types
try:
    _object = types.ObjectType
    _newclass = 1
except AttributeError:
    class _object : pass
    _newclass = 0
del types



malloc_CipherContext = _mucipherc.malloc_CipherContext

free_CipherContext = _mucipherc.free_CipherContext

md5Block = _mucipherc.md5Block

shaBlock = _mucipherc.shaBlock

sha256Block = _mucipherc.sha256Block

cipherKeySHA256 = _mucipherc.cipherKeySHA256

cipherKeyMD5 = _mucipherc.cipherKeyMD5

_blockCipher = _mucipherc._blockCipher

_blockDecipher = _mucipherc._blockDecipher


