/* museekd - The Museek daemon
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

#ifndef __MUSEEKDTYPES_HH__
#define __MUSEEKDTYPES_HH__

#define EM_CHAT		1 << 0
#define EM_PRIVATE	1 << 1
#define EM_TRANSFERS	1 << 2
#define EM_USERINFO	1 << 3
#define EM_USERSHARES	1 << 4
#define EM_INTERESTS	1 << 5
#define EM_CONFIG	1 << 6

#define ST_GLOBAL	1 << 0
#define ST_BUDDY	1 << 1
#define ST_ROOM		1 << 2

struct _PrivateMessage {
	uint32 ticket, timestamp;
	std::string user;
	std::wstring message;
};

typedef struct _PrivateMessage PrivateMessage;

#endif // __MUSEEKDTYPES_HH__
