/* Muhelp - Helper library for Museek
 *
 * Copyright (C) 2003-2004 Hyriand <hyriand@thegraveyard.org>
 * Copyright 2008 little blue poney <lbponey@users.sourceforge.net>
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

#include <system.h>

#include <Muhelp/Codec.hh>

#include <iconv.h>
#include <string>
#include <iostream>

using std::string;
using std::wstring;

Codec::Codec(const string& from, const string& to) {
	mCD = iconv_open(to.c_str(), from.c_str());
}

Codec::~Codec() {
	if(mCD != (iconv_t)-1)
		iconv_close(mCD);
}

string Codec::convert(const string& text) {
	if(text.empty())
		return string();
	
	string ret;
	
	if(mCD == (iconv_t)-1)
		return ret;
	
	size_t buf_len = text.size() * 4 + 4;
	char* out_buf = new char[buf_len];
	
	size_t r, in_left, out_left;
	
	while(1) {
		in_left = text.size();
		out_left = buf_len;
		const char* pinbuf = text.data();
		char* pout_buf = out_buf;
		
		r = iconv(mCD, (ICONV_IN)(&pinbuf), &in_left, (char**)(&pout_buf), &out_left);
		if(r == (size_t)-1 && errno == E2BIG) {
			std::cerr << "TOO BIG" << std::endl;
			delete [] out_buf;
			buf_len *= 2;
			out_buf = new char[buf_len];
			continue;
		}
		
		break;
	}
	
	if(r != (size_t)-1 && in_left == 0)
		ret = string(out_buf, buf_len - out_left);
	
	delete [] out_buf;
	
	return ret;
}

string Codec::convert(const string& text, const string& from, const string& to) {
	if(text.empty())
		return string();
	
	Codec codec(from, to);
	return codec.convert(text);
}


wstring Codec::wide(const string& text) {
	if(text.empty())
		return wstring();
	
	string r = convert(text);
	return wstring((wchar_t*)r.data(), r.size() / sizeof(wchar_t));
}

wstring Codec::wide(const string& text, const string& from) {
	if(text.empty())
		return wstring();
		
	Codec codec(from, "WCHAR_T");
	return codec.wide(text);
}

string Codec::narrow(const wstring& text) {
	if(text.empty())
		return string();
	
	string s = string((char*)text.data(), text.size() * sizeof(wchar_t));
	return convert(s);
}

string Codec::narrow(const wstring& text, const string& to) {
	if(text.empty())
		return string();
	
	Codec codec("WCHAR_T", to);
	return codec.narrow(text);
}
