# Mucipher - Cryptograhic library for Museek
#
# Copyright (C) 2003-2004 Hyriand <hyriand@thegraveyard.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

import mucipherc

class CipherContext:
	def __init__(self, key, alg = "SHA256"):
		self.__context__ = mucipherc.malloc_CipherContext()
		
		if alg not in ["SHA256", "MD5"]:
			raise RuntimeError, "Invalid key generation algorithm"
		
		if alg == "MD5":
			mucipherc.cipherKeyMD5(self.__context__, key)
		else:
			mucipherc.cipherKeySHA256(self.__context__, key)
	
	def __call__(self):
		return self.__context__
	
	def __del__(self):
		import mucipherc
		mucipherc.free_CipherContext(self.__context__)

class Cipher:
	def __init__(self, key, alg = "SHA256"):
		self.ctx_encode = CipherContext(key, alg)
		self.ctx_decode = CipherContext(key, alg)
		
	def cipher(self, data):
		return mucipherc._blockCipher(self.ctx_encode(), data)
	
	def decipher(self, data):
		return mucipherc._blockDecipher(self.ctx_decode(), data)

class shaBlock:
	hextab = "0123456789abcdef"
	hasher = mucipherc.shaBlock
	
	def __init__(self, text):
		self.text = text
	
	def __str__(self):
		return self.hasher(self.text)
	
	def hexdigest(self):
		ret = ""
		temp = str(self)
		for i in temp:
			ret = ret + self.hextab[ord(i) >> 4] + self.hextab[ord(i) & 0x0f]
		return ret
	
	def __repr__(self):
		return self.hexdigest()

class md5Block(shaBlock):
	hasher = mucipherc.md5Block

class sha256Block(shaBlock):
	hasher = mucipherc.sha256Block
