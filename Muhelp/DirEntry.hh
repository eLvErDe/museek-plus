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

#ifndef __DIRENTRY_HH__
#define __DIRENTRY_HH__

#include <queue>
#include <string>
#include <vector>
#include <map>

#include <museekd/mutypes.h>

class DirEntry {
public:
	DirEntry(bool _f = true) { fake = _f; mtime = 0; };
	DirEntry(const std::string& p) : path(p) { fake = false; mtime = 0; };
	virtual ~DirEntry();

	virtual DirEntry* new_folder(bool fake);
	virtual DirEntry* new_folder(const std::string& path);

	void fold(DirEntry* folded);
	void network_pack(std::queue<unsigned char>&);
	void flatten(Folder&);

	void save(const std::string&);
	void load(const std::string&);

	std::string path;
	std::map<std::string, DirEntry*> folders;
	Folder files;

protected:
	std::queue<unsigned char> pack();
	void unpack(std::queue<unsigned char>&);

	bool fake;
	time_t mtime;
};

#endif // __DIRENTRY_HH__
