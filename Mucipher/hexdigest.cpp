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

#include "mucipher.h"

static char hextab[] = "0123456789abcdef";

void hexDigest(unsigned char *digest, int length, char* digestOut) {
	int i;
	for(i = 0; i < length; i++) {
		digestOut[i*2] = hextab[digest[i] >> 4];
		digestOut[i*2 + 1] = hextab[digest[i] & 0x0f];
	}
	digestOut[i*2] = '\0';
}
