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

#include <Museek/Recoder.hh>
#include <Muhelp/Codec.hh>

using std::string;
using std::wstring;
using std::pair;
using std::map;

static CodecPair new_pair(const std::string& charset) {
	CodecPair r;
	
	r.first = new Codec(charset, "WCHAR_T");
	if(! r.first->valid()) {
		delete r.first;
		r.first = new Codec("ascii", "WCHAR_T");
	}
	
	r.second = new Codec("WCHAR_T", charset);
	if(! r.second->valid()) {
		delete r.second;
		r.second = new Codec("WCHAR_T", "ascii");
	}
	
	return r;
}

static void del_pair(CodecPair& codecs) {
	if(codecs.first)
		delete codecs.first;
	if(codecs.second)
		delete codecs.second;
}

Recoder::Recoder(const std::string& filesystem, const std::string& network)
        : mFilesystem(CodecPair(0,0)), mNetwork(CodecPair(0,0)) {
	
	set_filesystem(filesystem);
	set_network(network);
	
	mUTF8 = new_pair("UTF8");
}

Recoder::~Recoder() {
	del_pair(mFilesystem);
	del_pair(mNetwork);
	del_pair(mUTF8);
	
	map<string, CodecPair>::iterator it = mRooms.begin();
	for(; it != mRooms.end(); ++it)
		del_pair((*it).second);
	
	it = mUsers.begin();
	for(; it != mUsers.end(); ++it)
		del_pair((*it).second);
}

void Recoder::set_filesystem(const string& charset) {
	del_pair(mFilesystem);
	mFilesystem = new_pair(charset);
}

void Recoder::set_network(const string& charset) {
	del_pair(mNetwork);
	mNetwork = new_pair(charset);
}

const CodecPair& Recoder::room(const string& room) const {
	map<string, CodecPair>::const_iterator it = mRooms.find(room);
	if(it != mRooms.end())
		return (*it).second;
	return mNetwork;
}

void Recoder::set_room(const string& room, const string& charset) {
	map<string, CodecPair>::iterator it = mRooms.find(room);
	if(it != mRooms.end())
		del_pair((*it).second);
	
	mRooms[room] = new_pair(charset);
}

void Recoder::unset_room(const string& user) {
	map<string, CodecPair>::iterator it = mRooms.find(user);
	if(it != mRooms.end()) {
		del_pair((*it).second);
		mRooms.erase(it);
	}
}

const CodecPair& Recoder::user(const string& user) const {
	map<string, CodecPair>::const_iterator it = mUsers.find(user);
	if(it != mUsers.end())
		return (*it).second;
	return mNetwork;
}

void Recoder::set_user(const string& user, const string& charset) {
	map<string, CodecPair>::iterator it = mUsers.find(user);
	if(it != mUsers.end())
		del_pair((*it).second);
	
	mUsers[user] = new_pair(charset);
}

void Recoder::unset_user(const string& user) {
	map<string, CodecPair>::iterator it = mUsers.find(user);
	if(it != mUsers.end()) {
		del_pair((*it).second);
		mUsers.erase(it);
	}
}

string Recoder::asciify(const string& text) const {
	string ret;
	string::const_iterator it = text.begin();
	for(; it != text.end(); ++it) {
		if(*it >= 0) // >= 0 because non-ascii will be < 0 (signed char, remember) 
			ret += *it;
		else
			ret += '?';
	}
	return ret;
}

string Recoder::asciify(const wstring& text) const {
	string ret;
	wstring::const_iterator it = text.begin();
	for(; it != text.end(); ++it) {
		if(*it < 128)
			ret += (char)(*it);
		else
			ret += '?';
	}
	return ret;
}

wstring Recoder::wasciify(const string& text) const {
	wstring ret;
	
	string::const_iterator it = text.begin();
	for(; it != text.end(); ++it) {
		if(*it >= 0)	// >= 0 because non-ascii will be < 0 (signed char, remember)
			ret += (wchar_t)(*it);
		else
			ret += '?';
	}
	return ret;
}

string Recoder::fs_to_network(const string& filename) const {
	if(filename.empty())
		return string();
	
	string ret = mNetwork.second->narrow(mFilesystem.first->wide(filename));
	if(ret.empty())
		return asciify(filename);
	return ret;
}

string Recoder::network_to_fs(const string& filename) const {
	if(filename.empty())
		return string();
	
	string ret = mFilesystem.second->narrow(mNetwork.first->wide(filename));
	if(ret.empty())
		return asciify(filename);
	return ret;
}

wstring Recoder::decode_filesystem(const string& text) const {
	if(text.empty())
		return wstring();
	
	wstring ret = mFilesystem.first->wide(text);
	if(ret.empty())
		return wasciify(text);
	return ret;
}

string Recoder::encode_filesystem(const wstring& text) const {
	if(text.empty())
		return string();
	
	string ret = mFilesystem.second->narrow(text);
	if(ret.empty())
		return asciify(text);
	return ret;
}

wstring Recoder::decode_network(const string& text) const {
	if(text.empty())
		return wstring();
	
	wstring ret = mNetwork.first->wide(text);
	if(ret.empty())
		return wasciify(text);
	return ret;
}

string Recoder::encode_network(const wstring& text) const {
	if(text.empty())
		return string();
	string ret = mNetwork.second->narrow(text);
	if(ret.empty())
		return asciify(text);
	return ret;
}

wstring Recoder::decode_room(const string& _room, const string& text) const {
	if(text.empty())
		return wstring();
	
	wstring ret = room(_room).first->wide(text);
	if(ret.empty())
		return wasciify(text);
	return ret;
}

string Recoder::encode_room(const string& _room, const std::wstring& text) const {
	if(text.empty())
		return string();
	string ret = room(_room).second->narrow(text);
	if(ret.empty())
		return asciify(text);
	return ret;
}

wstring Recoder::decode_user(const string& _user, const string& text) const {
	if(text.empty())
		return wstring();
	
	wstring ret = user(_user).first->wide(text);
	if(ret.empty())
		return wasciify(text);
	return ret;
}

string Recoder::encode_user(const string& _user, const std::wstring& text) const {
	if(text.empty())
		return string();
	string ret = user(_user).second->narrow(text);
	if(ret.empty())
		return asciify(text);
	return ret;
}

WStringList Recoder::decode_user_list(const string& _user, const StringList& list) const {
	if(list.empty())
		return WStringList();
	
	WStringList ret;
	
	const CodecPair& codec = user(_user);
	StringList::const_iterator it = list.begin();
	for(; it != list.end(); ++it) {
		wstring recoded = codec.first->wide(*it);
		if(recoded.empty())
			recoded = wasciify(*it);
		ret.push_back(recoded);
	}
	return ret;
}

StringList Recoder::encode_user_list(const string& _user, const WStringList& list) const {
	if(list.empty())
		return StringList();
	
	StringList ret;
	
	const CodecPair& codec = user(_user);
	WStringList::const_iterator it = list.begin();
	for(; it != list.end(); ++it) {
		string recoded = codec.second->narrow(*it);
		if(recoded.empty())
			recoded = asciify(*it);
		ret.push_back(recoded);
	}
	return ret;
}

wstring Recoder::decode_utf8(const string& text) const {
	if(text.empty())
		return wstring();
	
	return mUTF8.first->wide(text);
}

string Recoder::encode_utf8(const wstring& text) const {
	if(text.empty())
		return string();
	
	return mUTF8.second->narrow(text);
}

WFolder Recoder::decode_folder(const CodecPair& codec, const Folder& folder) const {
	WFolder ret;
	Folder::const_iterator it = folder.begin();
	for(; it != folder.end(); ++it) {
		wstring recoded = codec.first->wide((*it).first);
		if(recoded.empty())
			recoded = wasciify((*it).first);
		ret[recoded] = (*it).second;
	}
	return ret;
}

WFolder Recoder::decode_folder(const string& _user, const Folder& folder) const {
	return decode_folder(user(_user), folder);
}

Folder Recoder::utf8_folder(const WFolder& folder) const {
	Folder ret;
	WFolder::const_iterator it = folder.begin();
	for(; it != folder.end(); ++it) {
		string recoded = mUTF8.second->narrow((*it).first);
		ret[recoded] = (*it).second;
	}
	return ret;
}

WShares Recoder::decode_shares(const CodecPair& codec, const Shares& shares) const {
	WShares ret;
	Shares::const_iterator it = shares.begin();
	for(; it != shares.end(); ++it) {
		wstring recoded = codec.first->wide((*it).first);
		if(recoded.empty())
			recoded = wasciify((*it).first);
		ret[recoded] = decode_folder(codec, (*it).second);
	}
	return ret;
}

WShares Recoder::decode_shares(const string& _user, const Shares& shares) const {
	return decode_shares(user(_user), shares);
}

Shares Recoder::utf8_shares(const WShares& shares) const {
	Shares ret;
	WShares::const_iterator it = shares.begin();
	for(; it != shares.end(); ++it) {
		string recoded = mUTF8.second->narrow((*it).first);
		ret[recoded] = utf8_folder((*it).second);
	}
	return ret;
}

WTickers Recoder::decode_tickers(const CodecPair& codec, const Tickers& tickers) const {
	WTickers ret;
	Tickers::const_iterator it = tickers.begin();
	for(; it != tickers.end(); ++it) {
		wstring recoded = codec.first->wide((*it).second);
		if(recoded.empty())
			recoded = wasciify((*it).second);
		ret[(*it).first] = recoded;
	}
	return ret;
}

WTickers Recoder::decode_tickers(const string& _room, const Tickers& tickers) const {
	return decode_tickers(room(_room), tickers);
}

Tickers Recoder::utf8_tickers(const WTickers& tickers) const {
	Tickers ret;
	WTickers::const_iterator it = tickers.begin();
	for(; it != tickers.end(); ++it)
		ret[(*it).first] = mUTF8.second->narrow((*it).second);
	return ret;
}

map<string, Tickers> Recoder::utf8_tickermap(const map<string, WTickers>& tickers) const {
	map<string, Tickers> ret;
	map<string, WTickers>::const_iterator it = tickers.begin();
	for(; it != tickers.end(); ++it)
		ret[(*it).first] = utf8_tickers((*it).second);
	return ret;
}
