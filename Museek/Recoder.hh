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

#ifndef __RECODER_HH__
#define __RECODER_HH__

#include <string>
#include <map>

class Codec;

typedef std::pair<Codec*, Codec*> CodecPair;

class Recoder {
public:
	Recoder(const std::string& filesystem, const std::string& network);
	virtual ~Recoder();
	
	// Local filesystem codec pair
	virtual void set_filesystem(const std::string& charset);
	
	// Default network codec pair
	virtual void set_network(const std::string& charset);
	
	// Room specific codec pair
	virtual void set_room(const std::string& room, const std::string& charset);
	virtual void unset_room(const std::string& room);
	
	// User specific codec pair
	virtual void set_user(const std::string& user, const std::string& charset);
	virtual void unset_user(const std::string& user);
	
	// Asciification in case translation fails
	std::string asciify(const std::string& text) const;
	std::string asciify(const std::wstring& text) const;
	std::wstring wasciify(const std::string& text) const;
	
	// Transcode fs <-> network;
	virtual std::string fs_to_network(const std::string& text) const;
	virtual std::string network_to_fs(const std::string& text) const;
	
	// Filesystem <-> unicode
	virtual std::wstring decode_filesystem(const std::string& text) const;
	virtual std::string encode_filesystem(const std::wstring& text) const;
	
	// Network <-> unicode
	virtual std::wstring decode_network(const std::string& text) const;
	virtual std::string encode_network(const std::wstring& text) const;
	
	// Room text <-> unicode
	virtual std::wstring decode_room(const std::string& room, const std::string& text) const;
	virtual std::string encode_room(const std::string& room, const std::wstring& text) const;
	
	// User text <-> unicode
	virtual std::wstring decode_user(const std::string& user, const std::string& text) const;
	virtual std::string encode_user(const std::string& user, const std::wstring& text) const;
	virtual WStringList decode_user_list(const std::string& user, const StringList& list) const;
	virtual StringList encode_user_list(const std::string& user, const WStringList& list) const;
	
	// UTF8 <-> unicode
	virtual std::wstring decode_utf8(const std::string& text) const;
	virtual std::string encode_utf8(const std::wstring& text) const;
	
	// Transcode folder
	virtual WFolder decode_folder(const std::string& user, const Folder& folder) const;
	virtual Folder utf8_folder(const WFolder& folder) const;
	
	// Decode shares
	virtual WShares decode_shares(const std::string& user, const Shares& shares) const;
	virtual Shares utf8_shares(const WShares& shares) const;
	
	// Decode tickers
	virtual WTickers decode_tickers(const std::string& room, const Tickers& tickers) const;
	virtual Tickers utf8_tickers(const WTickers& tickers) const;
	virtual std::map<std::string, Tickers> utf8_tickermap(const std::map<std::string, WTickers>& tickers) const;
	
private:
	inline const CodecPair& filesystem() const { return mFilesystem; }
	inline const CodecPair& network() const { return mNetwork; }
	const CodecPair& room(const std::string& room) const;
	const CodecPair& user(const std::string& user) const;
	WFolder decode_folder(const CodecPair& codec, const Folder& folder) const;
	WShares decode_shares(const CodecPair& codec, const Shares& shares) const;
	WTickers decode_tickers(const CodecPair& codec, const Tickers& tickers) const;
	
	CodecPair mFilesystem, mNetwork, mUTF8;
	std::map<std::string, CodecPair> mRooms;
	std::map<std::string, CodecPair> mUsers;
};

#endif // __RECODER_HH__
