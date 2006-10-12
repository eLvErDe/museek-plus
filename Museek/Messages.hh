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

#ifndef __MESSAGES_HH__
#define __MESSAGES_HH__

#include <string>
#include <queue>
#include <vector>
#include <iostream>

typedef unsigned char uchar;

class GenericMessage {
public:
	GenericMessage() {};
	virtual ~GenericMessage() {};
};

#define MAKE virtual std::queue<uchar>& make_network_packet() { pack(get_type());
#define END_MAKE return buffer; };
#define PARSE virtual void parse_network_packet(std::queue<uchar>& data) { buffer = data;
#define END_PARSE };

class NetworkMessage: public GenericMessage {
protected:
	std::queue<uchar> buffer;

public:
	MAKE
	END_MAKE

	PARSE
	END_PARSE

protected:
	void pack(const std::string&, bool=false);
	void pack(const std::vector<uchar>&);
	void pack(uint32);
	void pack(uchar c) { buffer.push(c); }
	void pack(off_t);

	std::string unpack_string();
	std::vector<uchar> unpack_vector();
	std::string unpack_ip();
	uint32 unpack_int();
	off_t unpack_off();
	uchar unpack_char() { 
		uchar c = 0;
		if(! buffer.empty()) {
			c = buffer.front(); 
			buffer.pop(); 
		}  else {
			std::cerr << "WARNING: trying to unpack from an empty buffer" << std::endl;
		}
		return c;
	}

	void compress();
	void decompress();
	
private:
	unsigned char get_type() { return 0; }

};

#define NETWORKMESSAGE(parent, mtype, m_id) \
	class mtype : public parent { \
	private: \
		uint32 get_type() { return m_id; } \
	public:
#define END };

#endif // __MESSAGES_HH__
