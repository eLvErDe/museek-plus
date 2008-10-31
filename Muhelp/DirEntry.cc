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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif // HAVE_CONFIG_H

#include <system.h>

#include <Muhelp/DirEntry.hh>
#include <Muhelp/string_ext.hh>

#include <NewNet/nnlog.h>
#include <NewNet/nnpath.h>

using std::string;
using std::map;
using std::queue;

DirEntry::~DirEntry() {
	map<string, DirEntry*>::iterator it = folders.begin();
	for(; it != folders.end(); ++it)
		delete (*it).second;
}

DirEntry* DirEntry::new_folder(const string& path) {
	return new DirEntry(path);
}

DirEntry* DirEntry::new_folder(bool fake) {
	return new DirEntry(fake);
}

void DirEntry::fold(DirEntry* folded) {
	NNLOG("museek.direntry", "fold %s", folded->path.c_str());

	if (! fake) {
		if (folded->folders.find(path) != folded->folders.end())
			return;

		DirEntry* de = folded->new_folder(path);

		Folder::iterator fit = files.begin();
		for(; fit != files.end(); ++fit)
			de->files[(*fit).first] = (*fit).second;

		folded->folders[path] = de;
	}

	map<string, DirEntry*>::iterator dit = folders.begin();
	for(; dit != folders.end(); ++dit)
		(*dit).second->fold(folded);
}

static inline void _pack(queue<unsigned char>& data, uint32 i) {
    for(uint j = 0; j < 4; j++) {
        data.push(i & 0xff);
        i >>= 8;
    }
}

static inline void _pack(queue<unsigned char>& data, uint64 i) {
    for(uint j = 0; j < 8; j++) {
        data.push(i & 0xff);
        i >>= 8;
    }
}

static inline void _pack(queue<unsigned char>& data, string s) {
    _pack(data, (uint32) s.size());
    string::const_iterator it = s.begin();
    for(; it != s.end(); ++it)
        data.push(*it);
}

static inline void _pack_path(queue<unsigned char>& data, string s) {
    _pack(data, (uint32) s.size());
    string::const_iterator it = s.begin();
    for(; it != s.end(); ++it)
        if((*it) == '/')
            data.push('\\');
        else
            data.push(*it);
}

static inline uint32 _unpack_int(queue<unsigned char>& data) {
	uint32 i = 0;
	for(uint j = 0; j < 4; j++) {
		i += data.front() << (j * 8);
		data.pop();
	}
	return i;
}

static inline uint64 _unpack_off(queue<unsigned char>& data) {
	uint64 i = 0;
	for(uint j = 0; j < 8; j++) {
		i += data.front() << (j * 8);
		data.pop();
	}
	return i;
}

static inline string _unpack_str(queue<unsigned char>& data) {
	string r;
	uint32 l = _unpack_int(data);
	r.reserve(l);
	for(uint32 j = 0; j < l; ++j) {
		r += data.front();
		data.pop();
	}
	return r;
}

queue<unsigned char> DirEntry::pack() {
	NNLOG("museek.direntry", "pack");

	queue<unsigned char> data;

	_pack(data, path);
	_pack(data, (uint32)mtime);

	_pack(data, (uint32)folders.size());
	map<string, DirEntry*>::iterator dit = folders.begin();
	for(; dit != folders.end(); ++dit) {
		queue<unsigned char> data2 = (*dit).second->pack();
		while(! data2.empty()) {
			data.push(data2.front());
			data2.pop();
		}
	}

	_pack(data, (uint32)files.size());

	Folder::iterator fit = files.begin();
	for(; fit != files.end(); ++fit) {
		_pack(data, (*fit).first);
		_pack(data, (*fit).second.size);
		_pack(data, (*fit).second.ext);
		_pack(data, (uint32)(*fit).second.attrs.size());
		std::vector<uint32>::iterator ait = (*fit).second.attrs.begin();
		for(; ait != (*fit).second.attrs.end(); ++ait)
			_pack(data, *ait);
	}
	return data;
}

void DirEntry::save(const string& fn) {
	NNLOG("museek.direntry", "save %s", fn.c_str());

	queue<unsigned char> data = pack();
	unsigned char *cdata = new unsigned char[data.size()];

	uint32 i = 0;
	while (! data.empty()) {
		cdata[i++] = data.front();
		data.pop();
	}

	FILE *f = fopen(fn.c_str(), "w");
	if (f != NULL) {
		fwrite(cdata, 1, i, f);
		fclose(f);
	}
	delete [] cdata;
}

void DirEntry::unpack(queue<unsigned char>& data) {
	NNLOG("museek.direntry", "unpack %d", data.size());

	for(map<string, DirEntry*>::iterator it = folders.begin(); it != folders.end(); ++it)
		delete (*it).second;
	folders.clear();
	files.clear();

	path = _unpack_str(data);
	mtime = _unpack_int(data);

	uint32 i = _unpack_int(data);
	for(uint32 j = 0; j < i; ++j) {
		DirEntry* de = new_folder(false);
		de->unpack(data);
		folders[de->path] = de;
	}

	i = _unpack_int(data);
	for(uint32 j = 0; j < i; ++j) {
		FileEntry fe;
		string fn;
		uint32 k;
		fn = _unpack_str(data);
		fe.size = _unpack_off(data);
		fe.ext = _unpack_str(data);
		k = _unpack_int(data);
        for(uint32 l = 0; l < k; ++l)
			fe.attrs.push_back(_unpack_int(data));
		files[fn] = fe;
	}
}

void DirEntry::load(const string& fn) {
	NNLOG("museek.direntry", "load %s", fn.c_str());

	long long size;

	FILE *f = fopen(fn.c_str(), "r");
	if (f == NULL)
		return;

    // obtain file size:
    fseek (f , 0 , SEEK_END);
    size = ftell (f);
    rewind (f);
    
	if (size == 0)
		return;

	unsigned char *cdata = new unsigned char[size];
	if (fread(cdata, 1, size, f) != (uint32)size) {
		delete [] cdata;
		return;
	}

	queue<unsigned char> data;
	for(int i = 0; i < size; i++)
		data.push(cdata[i]);

	delete [] cdata;

	unpack(data);
}

void DirEntry::network_pack(queue<unsigned char>& data) {
	NNLOG("museek.direntry", "network pack <...>");

	_pack(data, (uint32)folders.size());
	map<string, DirEntry*>::iterator dit = folders.begin();
	for(; dit != folders.end(); ++dit) {
		_pack_path(data, (*dit).first);
		_pack(data, (uint32)(*dit).second->files.size());
		Folder::iterator fit = (*dit).second->files.begin();
		for(; fit != (*dit).second->files.end(); ++fit) {
			data.push(1);
			_pack(data, (*fit).first);
			_pack(data, (*fit).second.size);
			_pack(data, (*fit).second.ext);
			_pack(data, (uint32)(*fit).second.attrs.size());
			std::vector<uint32>::iterator ait = (*fit).second.attrs.begin();
			for(uint32 j = 0; ait != (*fit).second.attrs.end(); ++ait) {
				_pack(data, j++);
				_pack(data, *ait);
			}
		}
	}
}

void DirEntry::flatten(Folder& filemap) {
	NNLOG("museek.direntry", "flatten <...>");

	map<string, DirEntry*>::iterator dit = folders.begin();
	for(; dit != folders.end(); ++dit)
		(*dit).second->flatten(filemap);

	Folder::iterator fit = files.begin();
	for(; fit != files.end(); ++fit)
		filemap[str_replace(path, NewNet::Path::separator(), '\\') + "\\" + (*fit).first] = (*fit).second;
}
