/* Mucipher - Cryptograhic library for Museek
 *
 * Copyright (C) 2003-2004 Hyriand <hyriand@thegraveyard.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

%module mucipherc

%{
#include <system.h>
#include <Mucipher/mucipher.h>
%}

%include "cstring.i"
%include "cmalloc.i"

%cstring_chunk_output(unsigned char *md5Hash, 16)
%cstring_chunk_output(unsigned char *shaHash, 20)
%cstring_chunk_output(unsigned char *sha256Hash, 32)

%apply (char *STRING, int LENGTH) { (unsigned char *str, int len) };
%apply (char *STRING, int LENGTH) { (char *str, int len) };
	
%malloc(CipherContext)
%free(CipherContext)

extern void md5Block(unsigned char *str, int len, unsigned char *md5Hash);
extern void shaBlock(unsigned char *str, int len, unsigned char *shaHash);
extern void sha256Block(unsigned char *str, int len, unsigned char *sha256Hash);

extern void cipherKeySHA256(CipherContext *ctx, char *str, int len);
extern void cipherKeyMD5(CipherContext *ctx, char *str, int len);

%cstring_output_allocate_size(char **s, int *slen, free(*$1));
void _blockCipher(CipherContext *ctx, char *str, int len, char **s, int *slen);
void _blockDecipher(CipherContext *ctx, char *str, int len, char **s, int *slen);
