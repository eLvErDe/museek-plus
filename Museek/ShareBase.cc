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

#include <Museek/ShareBase.hh>
#include <Museek/Museek.hh>
#include <Museek/Recoder.hh>

#define MULOG_DOMAIN "Museek.SH"
#include <Muhelp/Mulog.hh>
#include <Muhelp/string_ext.hh>

#include <zlib.h>

#include <string>
#include <map>
#include <vector>
#include <algorithm>

using std::string;
using std::wstring;
using std::map;
using std::vector;
using std::pair;

#include <iostream>

ShareBase::ShareBase(Museek* museek) : mMuseek(museek), mNumFolders(0), mNumFiles(0) {
	CT("ShareBase");
}

void ShareBase::load(const string& db, const string& type) {
	CT("load %s", db.c_str());
	
	mShares.load(db);
	update(type);
}

void ShareBase::update(const string& type) {
	CT("update");
	
	recode();
	update_flat();
	update_compressed();
	update_word_maps();
	
	mNumFolders = mRecoded.folders.size();
	mNumFiles = mFlat.size();
	
	DEBUG("updated shares db, %i folders, %i files", mNumFolders, mNumFiles);
	
	if(mMuseek->logged_in())
		if  (type  != "buddy")
			mMuseek->server_shared_folders_files(mNumFolders, mNumFiles);
}

void ShareBase::recode() {
	CT("recode");
	
	mRecoded.folders.clear();
	
	std::map<std::string, DirEntry*>::iterator it = mShares.folders.begin();
	for(; it != mShares.folders.end(); ++it) {
		std::string _redir = mMuseek->recoder()->fs_to_network((*it).first);
		if(_redir.empty()) {
			DEBUG("WARNING: Couldn't transcode '%s' to network encoding", (*it).first.c_str());
			continue;
		}

		Folder::iterator fit = (*it).second->files.begin();
		Folder _refolder;
		for(; fit != (*it).second->files.end(); ++fit) {
			std::string _refn = mMuseek->recoder()->fs_to_network((*fit).first);
			if(_refn.empty()) {
				DEBUG("WARNING: Couldn't transcode '%s' to network encoding", (*fit).first.c_str());
				continue;
			}
			_refolder[_refn] = (*fit).second;
		}
		
		DirEntry* de = new DirEntry(_redir);
		de->files = _refolder;
		mRecoded.folders[_redir] = de;
	}
}

void ShareBase::update_flat() {
	CT("update_flat");
	
	mFlat.clear();
	mRecoded.flatten(mFlat);
}

void ShareBase::update_compressed() {
	CT("update_compressed");
	
	mCompressed.clear();
	
	std::queue<unsigned char> data;
	mRecoded.network_pack(data);
	
	uLong outbuf_len = (int)(data.size() * 1.1 + 12), i = 0;
	
	char *inbuf = new char[data.size()],
	     *outbuf = new char[outbuf_len];
	
	while(! data.empty()) {
		inbuf[i++] = data.front();
		data.pop();
	}

	if (compress((Bytef *)outbuf, &outbuf_len, (Bytef *)inbuf, i) == Z_OK) {
		for(uint i = 0; i < outbuf_len; i++)
			mCompressed.push_back(outbuf[i]);
	} else
		DEBUG("compression error");

	delete [] outbuf;
	delete [] inbuf;
}

bool ShareBase::is_shared(const string& path) const {
	return mFlat.find(path) != mFlat.end();
}

/* this is a bit hairy... */
inline wchar_t mutate(wchar_t c, bool special = false) {
	if(c >= 'A' && c <= 'Z')
		return c | 32;
	
	switch(c) {
		case '/':
		case ' ':
		case ';':
		case ':':
		case '\'':
		case '\\':
		case ']':
		case '[':
		case '{':
		case '}':
		case '<':
		case '>':
		case ',':
		case '.':
		case '!':
		case '@':
		case '#':
		case '$':
		case '%':
		case '^':
		case '&':
		case '(':
		case ')':
		case '_':
		case '+':
		case '=':
		case '~':
		case '`':
		case '"':
			return ' ';
		case '-':
		case '*':
			if(! special)
				return ' ';
		default:
			return c;
	}
}

void ShareBase::update_word_maps() {
	CT("update_wordmaps");
	
	mCharMap.clear();
	
	// Generate the search map (used for searching)
	Folder::iterator fit = mFlat.begin();
	for(; fit != mFlat.end(); ++fit) {
		wstring entry = mMuseek->recoder()->decode_network((*fit).first),
		        word;
		
		wstring::const_iterator sit = entry.begin();
		for(; sit != entry.end(); ++sit) {
			wchar_t c = mutate(*sit);
			if(c == ' ') {
				if(! word.empty()) {
					mCharMap[word[0]][word][(*fit).first] = (*fit).second;
				}
				word = wstring();
			} else
				word += c;
		}
		
		if(! word.empty()) {
			mCharMap[word[0]][word][(*fit).first] = (*fit).second;
		}
	}
}

inline map<string, FileEntry> ShareBase::fetch(const std::wstring& word) const {
	map<wchar_t, map<wstring, map<string, FileEntry> > >::const_iterator it1 = mCharMap.find(word[0]);
	if(it1 != mCharMap.end()) {
		map<wstring, map<string, FileEntry> >::const_iterator it2 = (*it1).second.find(word);
		if(it2 != (*it1).second.end())
			return (*it2).second;
	}
	return map<string, FileEntry>();
}

/* this is the best I can do I think... */
void ShareBase::search(const wstring& _query, Folder& result) {
	CT("search %s", _query.c_str());
	
	wstring query = _query;
	
	size_t results = 0;
	
	wstring word;
	
	if(query.empty())
		return;
	
	/* add a space to make sure we also get the last word */
	query += (wchar_t)' ';
	
	vector<map<string, FileEntry>* > q_in;
	StringList q_out;
	vector<wstring> q_part;
	bool quoted = false, was_quoted = false;
	
	/* breaks up the query into in-groups, out-files and terms */
	wstring::iterator sit = query.begin();
	for(; sit != query.end(); ++sit) {
		if(*sit == '"') {
			quoted = ! quoted;
			was_quoted = true;
			continue;
		}
		
		wchar_t c = mutate(*sit, quoted ? false : word.empty());
		if(! quoted && c == ' ') {
			wchar_t c2 = word[0];
			if(was_quoted || c2 == '*')
				q_part.push_back(word);
			else if(c2 == '-') {
				q_out.push_back(mMuseek->recoder()->encode_network(wstring(word.data() + 1, word.size() - 1)));
			} else {
				/* find files that match this word */
				map<string, FileEntry>* t = &mCharMap[word[0]][word];
				if(! t->empty())
					q_in.push_back(t);
				else
					return;
			}
			was_quoted = false;
			word = wstring();
			continue;
		}
		word += c;
	}
	
	if(q_in.empty())
		return;
		
	map<string, FileEntry>* base = q_in[0];
	vector<map<string, FileEntry>* >::const_iterator first = q_in.begin()++;
	map<string, FileEntry>::const_iterator it = base->begin();
	for(; it != base->end(); ++it) {
		if(result.find((*it).first) != result.end())
			continue;
		
		if(! q_out.empty()) {
			string lowr = tolower((*it).first);
			StringList::const_iterator oit = q_out.begin();
			for(; oit != q_out.end(); ++oit)
				if(lowr.find(*oit) != string::npos)
					break;
			if(oit != q_out.end())
				continue;
		}
		
		vector<map<string, FileEntry>* >::const_iterator ref = first;
		for(; ref != q_in.end(); ++ref)
			if((*ref)->find((*it).first) == (*ref)->end())
				break;
				
		if(ref == q_in.end()) {
			result[(*it).first] = (*it).second;
			++results;
		}
		
		if(results >= 500)
			return;
	}
	
	return;
}

Shares ShareBase::folder_contents(const std::string& _f) {
	CT("folder_contents %s", _f.c_str());
	
	Shares r_map;
	
	if(_f.empty())
		return r_map;
	
	std::string q = str_replace(_f, '\\', '/');
	if(q[q.size()-1] == '/')
		q = q.substr(0, q.size()-1);

	if(q.empty())
		return r_map;
	
	std::map<std::string, DirEntry*>::iterator it = mRecoded.folders.begin();
	for(; it != mRecoded.folders.end(); ++it) {
		
		if((*it).first == q || (*it).first.substr(0, q.size()+1) == q + "/" || (*it).first.substr(0, q.size()) == q )
			{
			r_map[(*it).first ] = (*it).second->files;
			}
	}
	
	return r_map;
}
