/* Museek - Museek's 'core' library
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

#ifndef __INITMESSAGES_HH__
#define __INITMESSAGES_HH__

#include <Museek/Messages.hh>

class InitMessage : public NetworkMessage {};

class PInit : public InitMessage {
public:
	PInit() {};
	PInit(const std::string& _u, const std::string& _t, uint32 _tok) : token(_tok), user(_u), type(_t) { }
	
	unsigned char get_type() { return 1; }
	
	MAKE
		pack(user);
		pack(type);
		pack(token);
	END_MAKE
	
	PARSE
		user = unpack_string();
		type = unpack_string();
		token = unpack_int();
	END_PARSE

	uint32 token;
	std::string user, type;
};

class PPierceFW : public InitMessage {
public:
	PPierceFW() {};
	PPierceFW(uint32 _tok) : token(_tok) { }

	unsigned char get_type() { return 0; }
	
	MAKE
		pack(token);
	END_MAKE

	PARSE
		token = unpack_int();
	END_PARSE

	uint32 token;
};

#endif // __PEERINITMESSAGES_HH__
