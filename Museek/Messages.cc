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

#include <Museek/mu.hh>

#include <Museek/Messages.hh>

#define MULOG_DOMAIN "Museek.NM"
#include <Muhelp/Mulog.hh>

#include <zlib.h>

void NetworkMessage::pack(const std::string& str, bool trslash) {
	CT("pack %s, %d", str.c_str(), trslash);
	
	pack((uint32)str.size());
	if (! trslash)
		for (uint32 i = 0; i < str.size(); i++)
			pack((uchar)str[i]);
	else
		for (uint32 i = 0; i < str.size(); i++)
			if(str[i] == '/')
				pack((uchar)'\\');
			else
				pack((uchar)str[i]);
}

void NetworkMessage::pack(const std::vector<uchar>& d) {
	CT("pack <...> (%d)", d.size());
	
	pack((uint32)d.size());
	std::vector<uchar>::const_iterator it = d.begin();
	for(; it != d.end(); ++it)
		pack(*it);
}

void NetworkMessage::pack(uint32 i) {
	CT("pack %u", i);
	
	for(uint j = 0; j < 4; ++j) {
		pack((uchar)(i & 0xff));
		i = i >> 8;
	}
}

void NetworkMessage::pack(off_t i) {
	CT("pack %lli", (long long int) i);
	
	for(uint j = 0; j < 8; ++j) {
		pack((uchar)(i & 0xff));
		i = i >> 8;
	}
}

uint32 NetworkMessage::unpack_int() {
	CT("unpack_int");
	
	if (buffer.size() < 4)
		return 0;
	uint32 l = 0;
	for(uint j = 0; j < 4; j++)
		l += unpack_char() << (j * 8);
	return l;
}

off_t NetworkMessage::unpack_off() {
	CT("unpack_off");
	
	if (buffer.size() < 8)
		return 0;
	off_t l = 0;
	for(uint j = 0; j < 8; j++)
		l += unpack_char() << (j * 8);
	return l;
}

std::string NetworkMessage::unpack_string() {
	CT("unpack_string");
	
	std::string x;
	
	if (buffer.size() < 4)
		return x;
	
	uint32 len = unpack_int();
	if (buffer.size() < len)
		return x;
	
	for(uint32 i = 0; i < len; i++)
		x += unpack_char();
	
	return x;
}

std::string NetworkMessage::unpack_ip() {
	CT("unpack_ip");
	
	uchar _ip[16];
	unsigned char part_1, part_2, part_3, part_4;
	part_4 = unpack_char();
	part_3 = unpack_char();
	part_2 = unpack_char();
	part_1 = unpack_char();
	snprintf((char *)_ip, 16, "%u.%u.%u.%u", part_1, part_2, part_3, part_4);
	return std::string((const char *)_ip);
}

std::vector<uchar> NetworkMessage::unpack_vector() {
	CT("unpack_vector");
	
	std::vector<uchar> vec;
	if (buffer.size() < 4)
		return vec;
	uint32 len = unpack_int();
	if (buffer.size() < len)
		return vec;
	for(uint32 i = 0; i < len; i++)
		vec.push_back(unpack_char());
	return vec;
}

void NetworkMessage::compress() {
	CT("compress");
	
	uint32 _mtype = unpack_int();

	uLong outbuf_len = (int)(buffer.size() * 1.1 + 12.0), i = 0;
	uchar *inbuf = new uchar[buffer.size()],
	              *outbuf = new uchar[outbuf_len];

	while(! buffer.empty())
		inbuf[i++] = unpack_char();

	if (::compress((Bytef *)outbuf, &outbuf_len, (Bytef *)inbuf, i) == Z_OK) {
		pack(_mtype);
		for(i = 0; i < outbuf_len; i++)
			pack(outbuf[i]);
	}

	delete [] outbuf;
	delete [] inbuf;
}

#define DEFAULTALLOC 1000000
void NetworkMessage::decompress() {
	CT("decompress");
	
	z_stream zst;
	uchar *inbuf, *outbuf;
	zst.zalloc = (alloc_func)NULL;
	zst.zfree = (free_func)NULL;

	zst.avail_in = buffer.size();
	inbuf = new uchar[zst.avail_in];
	zst.next_in = (Bytef*)inbuf;

	uLong i = 0;
	while(! buffer.empty())
		inbuf[i++] = unpack_char();

	zst.avail_out = DEFAULTALLOC;
	outbuf = new uchar[DEFAULTALLOC];
	zst.next_out = (Bytef*)outbuf;

	int err = inflateInit(&zst);
	if (err != Z_OK) {
		delete [] inbuf;
		delete [] outbuf;
		if (err != Z_MEM_ERROR)
			inflateEnd(&zst);
		DEBUG("decompression error");
		return;
	}

	do {
		err = inflate(&zst, Z_FINISH);
		switch(err) {
		case Z_STREAM_END:
			break;
		case Z_BUF_ERROR:
			if (zst.avail_out > 0) {
				inflateEnd(&zst);
				delete [] inbuf;
				delete [] outbuf;
				DEBUG("decompression error");
				return;
			}
		case Z_OK:
			i = 0;
			while(i < (DEFAULTALLOC - zst.avail_out))
				buffer.push(outbuf[i++]);
			zst.avail_out = DEFAULTALLOC;
			zst.next_out = (Bytef*)outbuf;
			break;
		default:
			DEBUG("decompression error");
			inflateEnd(&zst);
			delete [] inbuf;
			delete [] outbuf;
			return;
		}
	} while (err != Z_STREAM_END);
	i = 0;
	while (i < (DEFAULTALLOC - zst.avail_out))
		buffer.push(outbuf[i++]);
	delete [] inbuf;
	delete [] outbuf;
	inflateEnd(&zst);
}
#undef DEFAULTALLOC
