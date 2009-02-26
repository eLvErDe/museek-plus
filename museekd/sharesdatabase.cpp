/* Museek - Museek's 'core' library
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif // HAVE_CONFIG_H
#include "mutypes.h"
#include "sharesdatabase.h"
#include "museekd.h"
#include "codesetmanager.h"
#include "servermanager.h"
#include <Muhelp/string_ext.hh>
#include <zlib.h>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <NewNet/nnpath.h>

using std::string;
using std::wstring;
using std::map;
using std::vector;

#include <iostream>

Museek::SharesDatabase::SharesDatabase(Museekd* museekd) : mMuseekd(museekd), mNumFolders(0), mNumFiles(0) {
}

void Museek::SharesDatabase::load(const string& db, bool add) {
 	NNLOG("museekd.shares.debug", "loading share database %s", db.c_str());
	mShares.load(db);
	update(add);
}

void Museek::SharesDatabase::update(bool add) {
	recode(add);
	update_flat();
	update_compressed();
	update_word_maps();

	mNumFolders = mRecoded.folders.size();
	mNumFiles = mFlat.size();

	NNLOG("museekd.shares.debug", "Updated shares, mNumFolders=%i, mNumFiles=%i", mNumFolders, mNumFiles);

    mMuseekd->sendSharedNumber();
}

void Museek::SharesDatabase::recode(bool add) {
	if (!add)
        mRecoded.folders.clear();

	std::map<std::string, DirEntry*>::iterator it = mShares.folders.begin();
	for(; it != mShares.folders.end(); ++it) {
        std::string _redir = mMuseekd->codeset()->fromFSToNet((*it).first);
		if(_redir.empty()) {
 			NNLOG("museekd.shares.warn", "Couldn't transcode '%s' to network encoding", (*it).first.c_str());
			continue;
		}

		Folder::iterator fit = (*it).second->files.begin();
		Folder _refolder;
		for(; fit != (*it).second->files.end(); ++fit) {
            std::string _refn = mMuseekd->codeset()->fromFSToNet((*fit).first);
			if(_refn.empty()) {
 				NNLOG("museekd.shares.warn", "Couldn't transcode '%s' to network encoding", (*fit).first.c_str());
				continue;
			}
			_refolder[_refn] = (*fit).second;
		}

		DirEntry* de = new DirEntry(_redir);
		de->files = _refolder;
		mRecoded.folders[_redir] = de;
	}
}

void Museek::SharesDatabase::update_flat() {
	mFlat.clear();
	mRecoded.flatten(mFlat);
}

void Museek::SharesDatabase::update_compressed() {
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
 		NNLOG("museekd.shares.warn", "compression error");

	delete [] outbuf;
	delete [] inbuf;
}

/**
 * The given path should be encoded with net encoding. Separator should be the network one (backslash).
 */
bool Museek::SharesDatabase::is_shared(const string& path) const {
	return mFlat.find(path) != mFlat.end();
}

/**
 * The given path should be encoded with net encoding. Separator should be the network one (backslash).
 * Do a case insensitive search in the base for a path corresponding to the given one.
 */
std::string Museek::SharesDatabase::find_shared_nocase(const std::string& path) const {
    Folder::const_iterator it;
    for (it = mFlat.begin(); it != mFlat.end(); it++) {
        if (tolower(it->first) == path)
            return it->first;
    }
    return std::string();
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

void Museek::SharesDatabase::update_word_maps() {
	mCharMap.clear();

	// Generate the search map (used for searching)
	Folder::iterator fit = mFlat.begin();
	for(; fit != mFlat.end(); ++fit) {
		string entry = mMuseekd->codeset()->fromNet((*fit).first), word;

		string::const_iterator sit = entry.begin();
		for(; sit != entry.end(); ++sit) {
			wchar_t c = mutate(*sit);
			if(c == ' ') {
				if(! word.empty()) {
					mCharMap[word[0]][word][(*fit).first] = (*fit).second;
				}
				word = string();
			} else
				word += c;
		}

		if(! word.empty()) {
			mCharMap[word[0]][word][(*fit).first] = (*fit).second;
		}
	}
}

inline Folder Museek::SharesDatabase::fetch(const std::string& word) const {
	map<wchar_t, Shares>::const_iterator it1 = mCharMap.find(word[0]);
	if(it1 != mCharMap.end()) {
		Shares::const_iterator it2 = (*it1).second.find(word);
		if(it2 != (*it1).second.end())
			return (*it2).second;
	}
	return Folder();
}

/* this is the best I can do I think... */
void Museek::SharesDatabase::search(const string& _query, Folder& result) {
 	NNLOG("museekd.shares.debug", "sharesdatabase search %s", _query.c_str());

	string query = _query;

	size_t results = 0;

	string word;

	if(query.empty())
		return;

	/* add a space to make sure we also get the last word */
	query += (wchar_t)' ';

	vector<Folder* > q_in; // foobar
	StringList q_out; // -foobar
	StringList q_part; // *foobar "foo bar"
	bool quoted = false, was_quoted = false;

	/* breaks up the query into in-groups, out-files and terms */
	string::iterator sit = query.begin();
	for(; sit != query.end(); ++sit) {
		if(*sit == '"') {
			quoted = ! quoted;
			was_quoted = true;
			continue;
		}

		wchar_t c = mutate(*sit, quoted ? false : word.empty());
		if(! quoted && c == ' ') {
			wchar_t firstC = word[0];
			if(was_quoted || firstC == '*') {
			    if (firstC == '*')
                    word = word.substr(1);
				q_part.push_back(word);
			}
			else if(firstC == '-') {
			    if (word.size() > 0)
                    q_out.push_back(mMuseekd->codeset()->toNet(string(word.data() + 1, word.size() - 1)));
			}
			else {
				/* find files that match this word */
				Folder* t = &mCharMap[word[0]][word];
				if(! t->empty())
					q_in.push_back(t);
				else
					return;
			}
			was_quoted = false;
			word = string();
			continue;
		}
		word += c;
	}

	if(q_in.empty() && q_part.empty())
		return;

    else if (!q_in.empty()) {
        Folder* base = q_in[0];
        vector<Folder* >::const_iterator first = q_in.begin()++;
        Folder::const_iterator it = base->begin();
        for(; it != base->end(); ++it) {
            // Did we already found this result?
            if(result.find((*it).first) != result.end())
                continue;

            // Don't add results that contains forbidden words
            if(! q_out.empty()) {
                string lowr = tolower((*it).first);
                StringList::const_iterator oit = q_out.begin();
                for(; oit != q_out.end(); ++oit)
                    if(lowr.find(*oit) != string::npos)
                        break;
                if(oit != q_out.end())
                    continue;
            }

            // If we arrive here, we can add the file to the results if it matches every keyword
            vector<Folder* >::const_iterator ref = first;
            for(; ref != q_in.end(); ++ref)
                if((*ref)->find((*it).first) == (*ref)->end())
                    break;

            if(ref == q_in.end()) {
                // Ok, it matches every keyword, but does it match every phrase?
                StringList::const_iterator partit = q_part.begin();
                for(; partit != q_part.end(); ++partit)
                    if (tolower((*it).first).find(tolower(*partit)) == std::string::npos)
                        break;

                if(partit == q_part.end()) {
                    result[(*it).first] = (*it).second;
                    ++results;
                }
            }

            // Don't send more than 500 results
            if(results >= 500)
                return;
        }
    }
    else {
        // We're only searching phrases (*foobar "foo bar"), search in flat list
        Folder::iterator fit = mFlat.begin();
        StringList::const_iterator wit;
        for(; fit != mFlat.end(); ++fit) {
            string entry = tolower(mMuseekd->codeset()->fromNet((*fit).first));
            bool notFound = false;

            for (wit = q_part.begin(); wit != q_part.end(); wit++) {
                size_t posFound = entry.find(tolower(*wit));
                if (posFound == std::string::npos) {
                    notFound = true;
                    break;
                }
            }

            if (!notFound) {
                result[(*fit).first] = (*fit).second;
                ++results;
            }

            // Don't send more than 500 results
            if(results >= 500)
                return;
        }
    }
}

/**
 * The given path should be encoded with net encoding. Separator should be the network one (backslash).
 */
Shares Museek::SharesDatabase::folder_contents(const std::string& _f) {
	Shares r_map;

	if(_f.empty())
		return r_map;

	std::string q = _f;
	if(q[q.size()-1] == '\\')
		q = q.substr(0, q.size()-1);

	if(q.empty())
		return r_map;

	std::map<std::string, DirEntry*>::iterator it = mRecoded.folders.begin();
	for(; it != mRecoded.folders.end(); ++it) {
		if(((*it).first == q) || ((*it).first.substr(0, q.size()+1) == q + '\\'))
			{
			r_map[(*it).first ] = (*it).second->files;
			}
	}

	return r_map;
}
